#include "track.h"
#include "config.h"

#include <math.h>

/* El trazo se recorre por longitud de arco. Empezando en el extremo derecho de
 * la recta inferior, el circuito se divide en cuatro tramos consecutivos:
 *
 *     0                  recta inferior, de derecha a izquierda
 *     TRACK_STRAIGHT     curva izquierda, media vuelta
 *     + PI * TRACK_R     recta superior, de izquierda a derecha
 *     + TRACK_STRAIGHT   curva derecha, media vuelta
 *
 * Hacia afuera de esa cuenta el resto del programa sigue ubicando a los
 * vehiculos con un angulo entre cero y una vuelta completa. Ese angulo no es
 * un angulo geometrico sino la fraccion del circuito ya recorrida, y se
 * traduce a longitud de arco de forma proporcional. Mantener esa interfaz es
 * lo que permite cambiar la forma de la pista sin tocar la fisica, el dibujo
 * ni el avance de los vehiculos. */

#define ARC_TURN   ((float)M_PI * TRACK_R)
#define ARC_TOTAL  (2.0f * TRACK_STRAIGHT + 2.0f * ARC_TURN)

/* Centros de las dos curvas. */
#define TURN_L_CX (TRACK_CX - TRACK_STRAIGHT * 0.5f)
#define TURN_R_CX (TRACK_CX + TRACK_STRAIGHT * 0.5f)

/* Desplazamiento que hace que el angulo de un cuarto de vuelta, donde el resto
 * del programa coloca la linea de meta y la parrilla de salida, caiga a la
 * mitad de la recta inferior. */
#define ARC_OFFSET (TRACK_STRAIGHT * 0.5f - ARC_TOTAL * 0.25f)

float track_wrap(float angle)
{
    const float TWO_PI = 2.0f * (float)M_PI;
    angle = fmodf(angle, TWO_PI);
    if (angle < 0.0f) angle += TWO_PI;
    return angle;
}

/* Longitud de arco recorrida que corresponde a ese angulo. */
static float arc_of(float angle)
{
    float s = track_wrap(angle) * (ARC_TOTAL / (2.0f * (float)M_PI));

    s += ARC_OFFSET;
    s = fmodf(s, ARC_TOTAL);
    if (s < 0.0f) s += ARC_TOTAL;
    return s;
}

/* Resuelve un punto del circuito: entrega su posicion sobre la linea central y
 * el vector unitario que apunta en el sentido de avance. */
static void resolve(float s, Vec2 *pos, Vec2 *dir)
{
    if (s < TRACK_STRAIGHT) {
        /* Recta inferior. Se recorre de derecha a izquierda. */
        pos->x = TRACK_CX + TRACK_STRAIGHT * 0.5f - s;
        pos->y = TRACK_CY + TRACK_R;
        dir->x = -1.0f;
        dir->y =  0.0f;
        return;
    }
    s -= TRACK_STRAIGHT;

    if (s < ARC_TURN) {
        /* Curva izquierda. El angulo arranca en el extremo inferior y avanza
         * media vuelta hasta el superior, pasando por el punto mas a la
         * izquierda del circuito. */
        float a = s / TRACK_R;
        pos->x = TURN_L_CX - TRACK_R * sinf(a);
        pos->y = TRACK_CY  + TRACK_R * cosf(a);
        dir->x = -cosf(a);
        dir->y = -sinf(a);
        return;
    }
    s -= ARC_TURN;

    if (s < TRACK_STRAIGHT) {
        /* Recta superior. Se recorre de izquierda a derecha. */
        pos->x = TRACK_CX - TRACK_STRAIGHT * 0.5f + s;
        pos->y = TRACK_CY - TRACK_R;
        dir->x = 1.0f;
        dir->y = 0.0f;
        return;
    }
    s -= TRACK_STRAIGHT;

    /* Curva derecha, del extremo superior al inferior. */
    float a = s / TRACK_R;
    pos->x = TURN_R_CX + TRACK_R * sinf(a);
    pos->y = TRACK_CY  - TRACK_R * cosf(a);
    dir->x = cosf(a);
    dir->y = sinf(a);
}

Vec2 track_point(float angle, float lane)
{
    Vec2 pos, dir;
    resolve(arc_of(angle), &pos, &dir);

    /* La normal se obtiene girando el sentido de avance un cuarto de vuelta, y
     * queda apuntando hacia afuera del circuito. Como el sentido de avance ya
     * viene unitario, no hace falta normalizarla. */
    Vec2 p;
    p.x = pos.x + dir.y * lane;
    p.y = pos.y - dir.x * lane;
    return p;
}

float track_heading(float angle)
{
    Vec2 pos, dir;
    resolve(arc_of(angle), &pos, &dir);
    return atan2f(dir.y, dir.x);
}

float track_scale(float angle)
{
    /* El angulo se reparte de forma pareja sobre la longitud del circuito, asi
     * que la conversion entre uno y otra es la misma en todo el trazo. */
    (void)angle;
    return ARC_TOTAL / (2.0f * (float)M_PI);
}

float track_radius(float angle)
{
    float s = arc_of(angle);

    /* En las rectas la trayectoria no gira. Se devuelve un radio enorme en
     * lugar de infinito para que el limite de rapidez que sale de el quede muy
     * por encima de cualquier velocidad alcanzable y no imponga freno alguno,
     * sin arriesgar una division entre cero mas adelante. */
    if (s < TRACK_STRAIGHT) return STRAIGHT_RADIUS;

    s -= TRACK_STRAIGHT;
    if (s < ARC_TURN) return TRACK_R;

    s -= ARC_TURN;
    if (s < TRACK_STRAIGHT) return STRAIGHT_RADIUS;

    return TRACK_R;
}
