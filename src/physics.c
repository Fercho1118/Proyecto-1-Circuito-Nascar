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

void physics_step(Car *cars, int n, float dt)
{
    for (int i = 0; i < n; ++i) {
        speed_control(&cars[i], dt);
        car_update(&cars[i], dt);
    }
}
