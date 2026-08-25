#include "physics.h"
#include "config.h"
#include "track.h"

#include <math.h>

float physics_speed_limit(float angle, float lane)
{
    /* Un vehiculo que gira describe un arco de radio R. Para no derrapar, la
     * aceleracion lateral v^2/R tiene que quedar por debajo de lo que agarra
     * el neumatico, de donde sale v = raiz(a_max * R).
     *
     * El carril corre el radio: por fuera de la linea central la curva es mas
     * abierta y admite mas rapidez, y por dentro es mas cerrada y admite
     * menos, que es la razon por la que la trazada exterior conviene en las
     * curvas cerradas aunque el recorrido sea mas largo. */
    float radius = track_radius(angle) + lane;
    if (radius < 1.0f) radius = 1.0f;

    return sqrtf(MAX_LATERAL_ACC * radius);
}

/* Ajusta la rapidez del vehiculo hacia la que le conviene llevar.
 *
 * No basta con mirar el punto donde va: si el vehiculo solo frenara al entrar
 * a la curva ya llegaria demasiado rapido. Por eso tambien consulta el limite
 * un tramo mas adelante y se queda con el menor de los dos, que es lo que
 * hace que empiece a frenar antes de la curva y vuelva a acelerar al salir. */
static void speed_control(Car *car, float dt)
{
    float here  = physics_speed_limit(car->angle, car->lane);

    float ahead_angle = track_wrap(car->angle +
                                   PREVIEW_DIST / track_scale(car->angle));
    float ahead = physics_speed_limit(ahead_angle, car->lane);

    float limit  = (here < ahead) ? here : ahead;
    float target = (car->cruise < limit) ? car->cruise : limit;

    /* Un motor acelera mucho mas despacio de lo que frena un juego de frenos,
     * asi que las dos cotas son distintas. */
    if (car->speed < target) {
        car->speed += ACCEL_MAX * dt;
        if (car->speed > target) car->speed = target;
    } else {
        car->speed -= BRAKE_MAX * dt;
        if (car->speed < target) car->speed = target;
    }
}

/* Lleva al vehiculo hacia el carril que quiere mantener.
 *
 * La correccion es proporcional a lo lejos que esta de ese carril y se
 * amortigua con la rapidez lateral que ya trae, de modo que el vehiculo se
 * acomoda sin quedar oscilando. A eso se suma el empuje hacia afuera cuando
 * va mas rapido de lo que la curva permite: ese exceso es el que abre la
 * trayectoria y termina llevando al vehiculo contra el muro. */
static void steering(Car *car, float dt)
{
    float pull = (car->lane_goal - car->lane) * STEER_GAIN;
    float damp = car->lane_vel * STEER_DAMP;

    float slide = 0.0f;
    float limit = physics_speed_limit(car->angle, car->lane);
    if (car->speed > limit)
        slide = (car->speed - limit) * SLIDE_GAIN;

    car->lane_vel += (pull - damp + slide) * dt;

    if (car->lane_vel >  LANE_VEL_MAX) car->lane_vel =  LANE_VEL_MAX;
    if (car->lane_vel < -LANE_VEL_MAX) car->lane_vel = -LANE_VEL_MAX;
}

/* Segunda regla: choque contra el muro.
 *
 * Los muros son los bordes del asfalto, y el vehiculo los toca cuando su
 * desplazamiento lateral supera lo que permite el ancho de la pista. El
 * vehiculo no los atraviesa: se le devuelve al limite, su rapidez lateral se
 * invierte conservando solo una parte, que es el rebote, y pierde parte de la
 * rapidez de avance por el roce contra el muro. */
static void wall_collision(Car *car)
{
    float side = 0.0f;

    if (car->lane >  LANE_LIMIT) side =  1.0f;
    if (car->lane < -LANE_LIMIT) side = -1.0f;
    if (side == 0.0f) return;

    car->lane = side * LANE_LIMIT;

    /* Solo rebota si venia acercandose al muro. Sin esa condicion un vehiculo
     * pegado al borde quedaria rebotando contra el indefinidamente. */
    if (car->lane_vel * side > 0.0f)
        car->lane_vel = -car->lane_vel * WALL_RESTITUTION;

    car->speed *= WALL_SPEED_KEEP;

    /* Tras el golpe el vehiculo busca volver hacia el interior de la pista. */
    car->lane_goal = side * (LANE_LIMIT - LANE_STEP);
    car->impact_flash = IMPACT_FLASH_TIME;
}

void physics_step(Car *cars, int n, float dt)
{
    for (int i = 0; i < n; ++i) {
        speed_control(&cars[i], dt);
        steering(&cars[i], dt);
    }

    /* La integracion va en una pasada aparte para que todas las decisiones se
     * tomen sobre el mismo estado y ningun vehiculo reaccione a posiciones ya
     * modificadas dentro del mismo cuadro. */
    for (int i = 0; i < n; ++i) {
        car_update(&cars[i], dt);
        wall_collision(&cars[i]);
    }
}
