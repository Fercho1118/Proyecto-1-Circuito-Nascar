#include "render.h"
#include "car.h"
#include "config.h"
#include "track.h"

#include <math.h>

/* Paleta de la escena. */
static const SDL_Color COL_GRASS   = {  28,  96,  52, 255 };
static const SDL_Color COL_INFIELD = {  22,  78,  43, 255 };
static const SDL_Color COL_ASPHALT = {  48,  48,  54, 255 };
static const SDL_Color COL_KERB_A  = { 198,  40,  40, 255 };
static const SDL_Color COL_KERB_B  = { 236, 236, 236, 255 };
static const SDL_Color COL_WHITE   = { 240, 240, 240, 255 };
static const SDL_Color COL_BLACK   = {  24,  24,  26, 255 };

static SDL_Vertex vertex_at(float x, float y, SDL_Color c)
{
    SDL_Vertex v;
    v.position.x  = x;
    v.position.y  = y;
    v.color       = c;
    v.tex_coord.x = 0.0f;
    v.tex_coord.y = 0.0f;
    return v;
}

/* Dibuja un anillo que sigue el ovalo entre dos desplazamientos laterales.
 * Cada tramo se emite como un cuadrilatero independiente, lo que permite
 * alternar el color por tramo para formar los bordes a rayas. Con stripe en
 * cero el anillo queda de un solo color. */
static void fill_ring(SDL_Renderer *ren, float lane_in, float lane_out,
                      SDL_Color a, SDL_Color b, int stripe)
{
    const int S = TRACK_SAMPLES;
    SDL_Vertex verts[TRACK_SAMPLES * 4];
    int        idx[TRACK_SAMPLES * 6];

    for (int i = 0; i < S; ++i) {
        float t0 = 2.0f * (float)M_PI * (float)i       / (float)S;
        float t1 = 2.0f * (float)M_PI * (float)(i + 1) / (float)S;

        SDL_Color c = a;
        if (stripe > 0 && ((i / stripe) % 2) != 0) c = b;

        Vec2 p0 = track_point(t0, lane_in);
        Vec2 p1 = track_point(t0, lane_out);
        Vec2 p2 = track_point(t1, lane_out);
        Vec2 p3 = track_point(t1, lane_in);

        int v = i * 4;
        verts[v + 0] = vertex_at(p0.x, p0.y, c);
        verts[v + 1] = vertex_at(p1.x, p1.y, c);
        verts[v + 2] = vertex_at(p2.x, p2.y, c);
        verts[v + 3] = vertex_at(p3.x, p3.y, c);

        idx[i * 6 + 0] = v;     idx[i * 6 + 1] = v + 1; idx[i * 6 + 2] = v + 2;
        idx[i * 6 + 3] = v;     idx[i * 6 + 4] = v + 2; idx[i * 6 + 5] = v + 3;
    }

    SDL_RenderGeometry(ren, NULL, verts, S * 4, idx, S * 6);
}

/* Rellena el terreno encerrado por la pista con un abanico de triangulos que
 * parten del centro del ovalo. */
static void fill_infield(SDL_Renderer *ren, float lane)
{
    const int S = TRACK_SAMPLES;
    SDL_Vertex verts[TRACK_SAMPLES + 1];
    int        idx[TRACK_SAMPLES * 3];

    verts[0] = vertex_at(TRACK_CX, TRACK_CY, COL_INFIELD);
    for (int i = 0; i < S; ++i) {
        float t = 2.0f * (float)M_PI * (float)i / (float)S;
        Vec2 p = track_point(t, lane);
        verts[i + 1] = vertex_at(p.x, p.y, COL_INFIELD);

        idx[i * 3 + 0] = 0;
        idx[i * 3 + 1] = i + 1;
        idx[i * 3 + 2] = (i + 1) % S + 1;
    }

    SDL_RenderGeometry(ren, NULL, verts, S + 1, idx, S * 3);
}

/* Dibuja la linea de meta como un damero de dos filas que cruza el ancho de
 * la pista en la parte de abajo del ovalo. */
static void draw_finish_line(SDL_Renderer *ren)
{
    const int COLS = 8;
    const int ROWS = 2;
    const float a0 = (float)M_PI * 0.5f;
    /* El ancho angular se elige para que el damero se vea cuadrado en la
     * zona donde cruza, que es una recta del ovalo. */
    const float da = (CAR_LEN * 0.9f) / track_scale(a0);

    SDL_Vertex verts[COLS * ROWS * 4];
    int        idx[COLS * ROWS * 6];
    int        n = 0, m = 0;

    for (int r = 0; r < ROWS; ++r) {
        float ta = a0 + da * (float)r / (float)ROWS;
        float tb = a0 + da * (float)(r + 1) / (float)ROWS;

        for (int c = 0; c < COLS; ++c) {
            float li = -TRACK_W * 0.5f + TRACK_W * (float)c / (float)COLS;
            float lo = -TRACK_W * 0.5f + TRACK_W * (float)(c + 1) / (float)COLS;

            SDL_Color col = ((r + c) % 2 == 0) ? COL_WHITE : COL_BLACK;

            Vec2 p0 = track_point(ta, li);
            Vec2 p1 = track_point(ta, lo);
            Vec2 p2 = track_point(tb, lo);
            Vec2 p3 = track_point(tb, li);

            verts[n + 0] = vertex_at(p0.x, p0.y, col);
            verts[n + 1] = vertex_at(p1.x, p1.y, col);
            verts[n + 2] = vertex_at(p2.x, p2.y, col);
            verts[n + 3] = vertex_at(p3.x, p3.y, col);

            idx[m + 0] = n;     idx[m + 1] = n + 1; idx[m + 2] = n + 2;
            idx[m + 3] = n;     idx[m + 4] = n + 2; idx[m + 5] = n + 3;
            n += 4; m += 6;
        }
    }

    SDL_RenderGeometry(ren, NULL, verts, n, idx, m);
}

/* Dibuja un rectangulo relleno girado un angulo. Los cuatro vertices se rotan
 * a mano alrededor del centro y se unen con dos triangulos. */
static void fill_rotated_rect(SDL_Renderer *ren, float cx, float cy,
                              float half_len, float half_wid,
                              float angle, SDL_Color color)
{
    float ca = cosf(angle), sa = sinf(angle);
    float ox[4] = { -half_len,  half_len, half_len, -half_len };
    float oy[4] = { -half_wid, -half_wid, half_wid,  half_wid };

    SDL_Vertex v[4];
    for (int i = 0; i < 4; ++i)
        v[i] = vertex_at(cx + ox[i] * ca - oy[i] * sa,
                         cy + ox[i] * sa + oy[i] * ca, color);

    const int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(ren, NULL, v, 4, idx, 6);
}

void render_background(SDL_Renderer *ren)
{
    SDL_SetRenderDrawColor(ren, COL_GRASS.r, COL_GRASS.g, COL_GRASS.b, 255);
    SDL_RenderClear(ren);
}

void render_track(SDL_Renderer *ren)
{
    const float half = TRACK_W * 0.5f;

    fill_ring(ren, -half, half, COL_ASPHALT, COL_ASPHALT, 0);
    fill_infield(ren, -half - KERB_W);

    /* Los bordes a rayas van pegados al asfalto por fuera y por dentro. */
    fill_ring(ren, -half - KERB_W, -half, COL_KERB_A, COL_KERB_B, 6);
    fill_ring(ren,  half, half + KERB_W, COL_KERB_A, COL_KERB_B, 6);

    draw_finish_line(ren);
}

void render_cars(SDL_Renderer *ren)
{
    for (int i = 0; i < num_cars; ++i) {
        const Car *c = &cars[i];

        /* La carroceria lleva el color del vehiculo y encima va un techo mas
         * oscuro, que da la sensacion de volumen y ayuda a distinguir hacia
         * donde apunta el auto. */
        SDL_Color roof = { (Uint8)(c->color.r / 3), (Uint8)(c->color.g / 3),
                           (Uint8)(c->color.b / 3), 255 };

        /* Un golpe reciente aclara la carroceria hacia el blanco, con lo que
         * los choques se alcanzan a ver aunque duren pocos cuadros. */
        SDL_Color body = c->color;
        if (c->impact_flash > 0.0f) {
            float k = c->impact_flash / IMPACT_FLASH_TIME;
            body.r = (Uint8)(c->color.r + (255 - c->color.r) * k);
            body.g = (Uint8)(c->color.g + (255 - c->color.g) * k);
            body.b = (Uint8)(c->color.b + (255 - c->color.b) * k);
        }

        fill_rotated_rect(ren, c->x, c->y, CAR_LEN * 0.5f, CAR_WID * 0.5f,
                          c->heading, body);
        fill_rotated_rect(ren, c->x, c->y, CAR_LEN * 0.22f, CAR_WID * 0.32f,
                          c->heading, roof);
    }
}
