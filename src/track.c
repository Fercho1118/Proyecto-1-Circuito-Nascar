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

/* Medidas del trazo. Dejan de ser constantes de compilacion porque el tamano
 * de la ventana se elige al arrancar, pero arrancan con los valores que
 * corresponden a la ventana por omision para que el modulo sea utilizable aun
 * si nadie llama a track_configure. */
static float g_cx       = DEFAULT_WIN_W / 2.0f;
static float g_cy       = DEFAULT_WIN_H / 2.0f;
static float g_turn_r   = TRACK_R;
static float g_straight = TRACK_STRAIGHT;

/* Valores derivados de los anteriores. Se recalculan una sola vez al
 * configurar, en lugar de rehacerlos en cada consulta. */
static float g_arc_turn   = (float)M_PI * TRACK_R;
static float g_arc_total  = 2.0f * TRACK_STRAIGHT + 2.0f * (float)M_PI * TRACK_R;
static float g_arc_offset = 0.0f;
static float g_scale      = 0.0f;

/* Rehace los valores derivados a partir del radio y del largo de las rectas. */
static void recompute(void)
{
    g_arc_turn  = (float)M_PI * g_turn_r;
    g_arc_total = 2.0f * g_straight + 2.0f * g_arc_turn;

    /* Desplazamiento que hace que el angulo de un cuarto de vuelta, donde el
     * resto del programa coloca la linea de meta y la parrilla de salida,
     * caiga a la mitad de la recta inferior. */
    g_arc_offset = g_straight * 0.5f - g_arc_total * 0.25f;

    g_scale = g_arc_total / (2.0f * (float)M_PI);
}

void track_configure(int win_w, int win_h)
{
    g_cx = (float)win_w * 0.5f;
    g_cy = (float)win_h * 0.5f;

    /* El trazo se escala de forma uniforme segun el eje que quede mas
     * apretado, con lo que conserva su forma en cualquier ventana. */
    float sx = (float)win_w / (float)DEFAULT_WIN_W;
    float sy = (float)win_h / (float)DEFAULT_WIN_H;
    float s  = (sx < sy) ? sx : sy;

    g_turn_r   = TRACK_R * s;
    g_straight = TRACK_STRAIGHT * s;

    recompute();
}

Vec2 track_center(void)
{
    Vec2 c = { g_cx, g_cy };
    return c;
}

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
    if (g_arc_offset == 0.0f && g_scale == 0.0f) recompute();

    float s = track_wrap(angle) * g_scale;

    s += g_arc_offset;
    s = fmodf(s, g_arc_total);
    if (s < 0.0f) s += g_arc_total;
    return s;
}

/* Resuelve un punto del circuito: entrega su posicion sobre la linea central y
 * el vector unitario que apunta en el sentido de avance. */
static void resolve(float s, Vec2 *pos, Vec2 *dir)
{
    if (s < g_straight) {
        /* Recta inferior. Se recorre de derecha a izquierda. */
        pos->x = g_cx + g_straight * 0.5f - s;
        pos->y = g_cy + g_turn_r;
        dir->x = -1.0f;
        dir->y =  0.0f;
        return;
    }
    s -= g_straight;

    if (s < g_arc_turn) {
        /* Curva izquierda. El angulo arranca en el extremo inferior y avanza
         * media vuelta hasta el superior, pasando por el punto mas a la
         * izquierda del circuito. */
        float a = s / g_turn_r;
        pos->x = (g_cx - g_straight * 0.5f) - g_turn_r * sinf(a);
        pos->y = g_cy + g_turn_r * cosf(a);
        dir->x = -cosf(a);
        dir->y = -sinf(a);
        return;
    }
    s -= g_arc_turn;

    if (s < g_straight) {
        /* Recta superior. Se recorre de izquierda a derecha. */
        pos->x = g_cx - g_straight * 0.5f + s;
        pos->y = g_cy - g_turn_r;
        dir->x = 1.0f;
        dir->y = 0.0f;
        return;
    }
    s -= g_straight;

    /* Curva derecha, del extremo superior al inferior. */
    float a = s / g_turn_r;
    pos->x = (g_cx + g_straight * 0.5f) + g_turn_r * sinf(a);
    pos->y = g_cy - g_turn_r * cosf(a);
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
    return g_scale;
}

float track_radius(float angle)
{
    float s = arc_of(angle);

    /* En las rectas la trayectoria no gira. Se devuelve un radio enorme en
     * lugar de infinito para que el limite de rapidez que sale de el quede muy
     * por encima de cualquier velocidad alcanzable y no imponga freno alguno,
     * sin arriesgar una division entre cero mas adelante. */
    if (s < g_straight) return STRAIGHT_RADIUS;

    s -= g_straight;
    if (s < g_arc_turn) return g_turn_r;

    s -= g_arc_turn;
    if (s < g_straight) return STRAIGHT_RADIUS;

    return g_turn_r;
}
