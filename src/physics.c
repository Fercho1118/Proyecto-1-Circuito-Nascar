#include "physics.h"
#include "config.h"
#include "track.h"

#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

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

/* ---- Regla 3: velocidad en curvas -------------------------------------- */

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

/* ---- Regla 2: colision contra el muro ---------------------------------- */

/* Los muros son los bordes del asfalto, y el vehiculo los toca cuando su
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

/* ---- Regla 1: colision entre vehiculos --------------------------------- */

/* Lo que un vehiculo decide despues de mirar a todos los demas. Se guarda
 * aparte del vehiculo mismo porque la exploracion se hace sobre el estado del
 * cuadro anterior: si cada vehiculo se modificara mientras los demas todavia
 * lo estan leyendo, el resultado dependeria del orden del recorrido. */
typedef struct {
    float bump;   /* cambio inmediato de rapidez por un golpe */
    float brake;  /* frenado sostenido por venir alcanzando a otro */
    float steer;  /* hacia donde conviene abrirse para rebasar */
    float push;   /* empuje lateral para deshacer un encimamiento */
    int   hit;    /* hubo contacto en este cuadro */
} Reaction;

static Reaction reactions[MAX_CARS];

/* Separacion sobre el asfalto entre dos vehiculos, en pixeles y con signo.
 * Es positiva cuando el segundo va adelante del primero. Se obtiene del
 * angulo que los separa, llevado al rango corto para que dar la vuelta
 * completa no cuente como estar lejos, y convertido a distancia con la escala
 * local de la pista. */
static float gap_ahead(const Car *a, const Car *b)
{
    const float TWO_PI = 2.0f * (float)M_PI;

    float da = b->angle - a->angle;
    if (da >  (float)M_PI) da -= TWO_PI;
    if (da < -(float)M_PI) da += TWO_PI;

    float scale = 0.5f * (track_scale(a->angle) + track_scale(b->angle));
    return da * scale;
}

/* Revisa a un vehiculo contra todos los demas y acumula en r lo que le toca
 * hacer. Solo lee el estado de la flota; nada de lo que escribe sale de r,
 * que pertenece unicamente a este vehiculo. */
static void scan_neighbours(const Car *cars, int n, int i, Reaction *r)
{
    const Car *self = &cars[i];

    r->bump = r->brake = r->steer = r->push = 0.0f;
    r->hit = 0;

    for (int j = 0; j < n; ++j) {
        if (j == i) continue;

        const Car *other = &cars[j];

        float ds = gap_ahead(self, other);
        if (fabsf(ds) > LOOKAHEAD) continue;

        float dl = self->lane - other->lane;

        /* Contacto: las dos carrocerias se traslapan a lo largo y a lo ancho. */
        if (fabsf(ds) < CAR_LEN && fabsf(dl) < CAR_WID) {
            float rel = self->speed - other->speed;

            /* El que viene mas rapido por detras cede rapidez y el de
             * adelante la recibe. La expresion es antisimetrica, asi que lo
             * que uno pierde es lo que el otro gana y la flota no se acelera
             * ni se frena en conjunto por efecto de los golpes. */
            if ((ds > 0.0f && rel > 0.0f) || (ds < 0.0f && rel < 0.0f))
                r->bump -= rel * BUMP_TRANSFER;

            /* Dos vehiculos no pueden ocupar el mismo lugar, asi que se
             * separan de lado con una fuerza que crece con el traslape. Si
             * van exactamente al mismo carril el desempate se hace por indice
             * para que la separacion no dependa del orden del recorrido. */
            float side = (dl != 0.0f) ? ((dl > 0.0f) ? 1.0f : -1.0f)
                                      : ((i < j) ? -1.0f : 1.0f);
            r->push += side * (CAR_WID - fabsf(dl)) * SEPARATION_GAIN;
            r->hit = 1;
            continue;
        }

        /* Sin contacto todavia, pero viene alcanzando a uno mas lento que le
         * tapa el paso: frena en proporcion a lo cerca que esta y busca un
         * hueco hacia el lado con mas espacio libre. */
        if (ds > 0.0f && fabsf(dl) < CAR_WID * 1.15f &&
            other->speed < self->speed) {

            float closeness = 1.0f - ds / LOOKAHEAD;
            float closing   = self->speed - other->speed;

            r->brake += closing * closeness * FOLLOW_BRAKE;

            /* Se abre hacia el lado en el que ya viene descentrado respecto
             * del otro; si van igual de alineados, escoge el lado de la pista
             * donde le queda mas asfalto. */
            float side;
            if (dl != 0.0f)          side = (dl > 0.0f) ? 1.0f : -1.0f;
            else if (self->lane > 0.0f) side = -1.0f;
            else                        side =  1.0f;

            r->steer += side * OVERTAKE_STEER * closeness;
        }
    }

    /* Un vehiculo puede estar tocando a muchos a la vez, y aunque cada
     * contacto aporte poco la suma llega a ser enorme. Acotarla es lo que
     * impide que un amontonamiento se realimente y desborde la simulacion. */
    if (r->bump  >  BUMP_MAX)         r->bump  =  BUMP_MAX;
    if (r->bump  < -BUMP_MAX)         r->bump  = -BUMP_MAX;
    if (r->push  >  PUSH_MAX)         r->push  =  PUSH_MAX;
    if (r->push  < -PUSH_MAX)         r->push  = -PUSH_MAX;
    if (r->brake >  FOLLOW_BRAKE_MAX) r->brake =  FOLLOW_BRAKE_MAX;
}

/* Traslada al vehiculo lo que decidio despues de mirar a sus vecinos. */
static void apply_reaction(Car *car, const Reaction *r, float dt)
{
    car->speed += r->bump;
    car->speed -= r->brake * dt;
    if (car->speed < MIN_SPEED) car->speed = MIN_SPEED;
    if (car->speed > MAX_SPEED) car->speed = MAX_SPEED;

    car->lane_vel += r->push * dt;

    /* El carril buscado se mide desde donde el vehiculo esta ahora, y nunca
     * apunta fuera del asfalto. */
    float goal = car->lane + r->steer;
    if (goal >  LANE_LIMIT) goal =  LANE_LIMIT;
    if (goal < -LANE_LIMIT) goal = -LANE_LIMIT;
    car->lane_goal = goal;

    if (r->hit) car->impact_flash = IMPACT_FLASH_TIME;
}

/* ---- Paso de simulacion ------------------------------------------------ */

/* Interruptor entre la version secuencial y la paralela. */
static int g_parallel = 1;

void physics_set_parallel(int enabled)
{
    g_parallel = enabled ? 1 : 0;
}

int physics_is_parallel(void)
{
    return g_parallel;
}

void physics_set_threads(int threads)
{
#ifdef _OPENMP
    if (threads > 0) omp_set_num_threads(threads);
#else
    (void)threads;
#endif
}

int physics_thread_count(void)
{
#ifdef _OPENMP
    if (!g_parallel) return 1;

    int used = 1;
    #pragma omp parallel
    {
        #pragma omp master
        used = omp_get_num_threads();
    }
    return used;
#else
    return 1;
#endif
}

/* El paso de simulacion esta partido en tres pasadas y cada una se reparte
 * entre los hilos por separado. La barrera implicita al final de cada pasada
 * es lo que garantiza el orden: nadie empieza a decidir hasta que todos
 * terminaron de mirar, y nadie avanza hasta que todos terminaron de decidir.
 *
 * En las tres, la iteracion i escribe unicamente en la posicion i de su
 * arreglo, asi que dos hilos nunca tocan el mismo dato y no hace falta ningun
 * candado. El reparto es estatico porque el costo de cada iteracion es
 * practicamente el mismo: la exploracion recorre siempre a toda la flota.
 *
 * El dibujo se queda fuera de todo esto y corre en un solo hilo, porque SDL
 * no admite que varios hilos usen el mismo renderer.
 *
 * La clausula if de cada directiva es la que permite tener las dos versiones
 * del algoritmo en un mismo binario. Con el reparto apagado, OpenMP no llega a
 * formar el equipo de trabajo y las tres pasadas se recorren de corrido en el
 * hilo que las invoco, que es exactamente la version secuencial. */
static double last_step_ms = 0.0;

double physics_last_ms(void)
{
    return last_step_ms;
}

void physics_step(Car *cars, int n, float dt)
{
    Uint64 t0 = SDL_GetPerformanceCounter();

    /* Pasada 1. La parte pesada: cada vehiculo mira a todos los demas, lo que
     * hace que el trabajo crezca con el cuadrado del tamano de la flota. Solo
     * lee el estado de los vehiculos y escribe en su propia reaccion. */
    #pragma omp parallel for if (g_parallel) schedule(static)
    for (int i = 0; i < n; ++i)
        scan_neighbours(cars, n, i, &reactions[i]);

    /* Pasada 2. Cada vehiculo aplica lo que decidio y ajusta su rapidez y su
     * direccion, sin volver a consultar a los demas. */
    #pragma omp parallel for if (g_parallel) schedule(static)
    for (int i = 0; i < n; ++i) {
        apply_reaction(&cars[i], &reactions[i], dt);
        speed_control(&cars[i], dt);
        steering(&cars[i], dt);
    }

    /* Pasada 3. Recien aqui los vehiculos se mueven y se resuelve el contacto
     * con el muro. */
    #pragma omp parallel for if (g_parallel) schedule(static)
    for (int i = 0; i < n; ++i) {
        car_update(&cars[i], dt);
        wall_collision(&cars[i]);
    }

    Uint64 t1 = SDL_GetPerformanceCounter();
    last_step_ms = 1000.0 * (double)(t1 - t0) /
                   (double)SDL_GetPerformanceFrequency();
}
