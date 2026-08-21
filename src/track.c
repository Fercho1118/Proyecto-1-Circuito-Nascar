#include "track.h"
#include "config.h"

#include <math.h>

/* La linea central es la elipse (cx + rx*cos t, cy + ry*sin t). Casi todo lo
 * que sigue sale de derivar esa expresion respecto de t. */

float track_wrap(float angle)
{
    const float TWO_PI = 2.0f * (float)M_PI;
    angle = fmodf(angle, TWO_PI);
    if (angle < 0.0f) angle += TWO_PI;
    return angle;
}

/* Vector tangente sin normalizar: la derivada de la linea central. */
static void tangent(float angle, float *tx, float *ty)
{
    *tx = -TRACK_RX * sinf(angle);
    *ty =  TRACK_RY * cosf(angle);
}

Vec2 track_point(float angle, float lane)
{
    float ca = cosf(angle), sa = sinf(angle);

    /* La normal apunta hacia afuera del ovalo. Se obtiene girando la tangente
     * un cuarto de vuelta y normalizando el resultado. */
    float nx = TRACK_RY * ca;
    float ny = TRACK_RX * sa;
    float len = sqrtf(nx * nx + ny * ny);
    if (len > 0.0f) { nx /= len; ny /= len; }

    Vec2 p;
    p.x = TRACK_CX + TRACK_RX * ca + nx * lane;
    p.y = TRACK_CY + TRACK_RY * sa + ny * lane;
    return p;
}

float track_heading(float angle)
{
    float tx, ty;
    tangent(angle, &tx, &ty);
    return atan2f(ty, tx);
}

float track_scale(float angle)
{
    float tx, ty;
    tangent(angle, &tx, &ty);
    return sqrtf(tx * tx + ty * ty);
}

float track_radius(float angle)
{
    /* Para una elipse el radio de curvatura es |T|^3 dividido entre el
     * producto de los radios. */
    float s = track_scale(angle);
    return (s * s * s) / (TRACK_RX * TRACK_RY);
}
