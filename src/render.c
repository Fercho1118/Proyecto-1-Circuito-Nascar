#include "render.h"
#include "car.h"
#include "config.h"
#include "physics.h"
#include "text.h"
#include "track.h"

#include <math.h>
#include <stdio.h>

/* Paleta de la escena. */
static const SDL_Color COL_GRASS   = {  28,  96,  52, 255 };
static const SDL_Color COL_INFIELD = {  22,  78,  43, 255 };
static const SDL_Color COL_ASPHALT = {  48,  48,  54, 255 };
static const SDL_Color COL_KERB_A  = { 198,  40,  40, 255 };
static const SDL_Color COL_KERB_B  = { 236, 236, 236, 255 };
static const SDL_Color COL_WHITE   = { 240, 240, 240, 255 };
static const SDL_Color COL_BLACK   = {  24,  24,  26, 255 };
static const SDL_Color COL_SOMBRA  = {   0,   0,   0,  90 };
static const SDL_Color COL_VIDRIO  = { 176, 200, 216, 255 };

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

    Vec2 mid = track_center();
    verts[0] = vertex_at(mid.x, mid.y, COL_INFIELD);
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

/* Agrega al lote un rectangulo girado, descrito en coordenadas propias del
 * vehiculo: el eje x apunta hacia adelante y el eje y hacia su costado. El
 * desplazamiento se aplica ya en pantalla, que es lo que permite separar la
 * sombra de la carroceria sin girarla dos veces. */
static void push_quad(SDL_Vertex *v, int *idx, int *nv, int *ni,
                      float cx, float cy, float ca, float sa,
                      float x0, float y0, float x1, float y1,
                      float off_x, float off_y, SDL_Color col)
{
    const float lx[4] = { x0, x1, x1, x0 };
    const float ly[4] = { y0, y0, y1, y1 };
    int base = *nv;

    for (int k = 0; k < 4; ++k) {
        v[base + k].position.x  = cx + lx[k] * ca - ly[k] * sa + off_x;
        v[base + k].position.y  = cy + lx[k] * sa + ly[k] * ca + off_y;
        v[base + k].color       = col;
        v[base + k].tex_coord.x = 0.0f;
        v[base + k].tex_coord.y = 0.0f;
    }
    *nv = base + 4;

    const int orden[6] = { 0, 1, 2, 0, 2, 3 };
    for (int k = 0; k < 6; ++k) idx[*ni + k] = base + orden[k];
    *ni += 6;
}

/* Dibuja toda la flota.
 *
 * Cada vehiculo aporta cuatro piezas: la sombra que lo despega del asfalto, la
 * carroceria con su color, el techo oscuro y el parabrisas. Las cuatro se
 * acumulan en un solo par de arreglos y se envian a la GPU con una unica
 * llamada, en lugar de una por pieza: con flotas grandes la diferencia entre
 * miles de llamadas y una sola es considerable. Los arreglos son estaticos
 * para no reservarlos en cada cuadro. */
void render_cars(SDL_Renderer *ren)
{
    static SDL_Vertex verts[MAX_CARS * 16];
    static int        idx[MAX_CARS * 24];
    int nv = 0, ni = 0;

    const float HL = CAR_LEN * 0.5f, HW = CAR_WID * 0.5f;

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < num_cars; ++i) {
        const Car *c = &cars[i];
        float ca = cosf(c->heading), sa = sinf(c->heading);

        /* Un golpe reciente aclara la carroceria, pero solo hasta cierto
         * punto: si el destello llegara al blanco puro, una flota amontonada
         * en contacto permanente se dibujaria entera de blanco y se perderian
         * los colores. */
        SDL_Color body = c->color;
        if (c->impact_flash > 0.0f) {
            float k = (c->impact_flash / IMPACT_FLASH_TIME) * FLASH_MAX;
            body.r = (Uint8)(c->color.r + (255 - c->color.r) * k);
            body.g = (Uint8)(c->color.g + (255 - c->color.g) * k);
            body.b = (Uint8)(c->color.b + (255 - c->color.b) * k);
        }

        SDL_Color roof = { (Uint8)(body.r / 3), (Uint8)(body.g / 3),
                           (Uint8)(body.b / 3), 255 };

        push_quad(verts, idx, &nv, &ni, c->x, c->y, ca, sa,
                  -HL, -HW, HL, HW, 2.5f, 3.0f, COL_SOMBRA);
        push_quad(verts, idx, &nv, &ni, c->x, c->y, ca, sa,
                  -HL, -HW, HL, HW, 0.0f, 0.0f, body);
        push_quad(verts, idx, &nv, &ni, c->x, c->y, ca, sa,
                  -HL * 0.62f, -HW * 0.66f, HL * 0.16f, HW * 0.66f,
                  0.0f, 0.0f, roof);
        push_quad(verts, idx, &nv, &ni, c->x, c->y, ca, sa,
                  HL * 0.16f, -HW * 0.54f, HL * 0.52f, HW * 0.54f,
                  0.0f, 0.0f, COL_VIDRIO);
    }

    if (nv > 0) SDL_RenderGeometry(ren, NULL, verts, nv, idx, ni);
}

void render_hud(SDL_Renderer *ren, float fps, double sim_ms)
{
    const int scale = 2;
    const int pad   = 10;
    const int line  = GLYPH_H * scale + 6;
    const int rows  = 4;

    char buf[64];

    /* Fondo semitransparente, para que el texto se lea igual de bien sobre el
     * cesped que sobre el asfalto. */
    SDL_Rect panel = { pad, pad, 210, rows * line + pad };
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
    SDL_RenderFillRect(ren, &panel);

    int y = pad + 6;

    snprintf(buf, sizeof(buf), "VEHICULOS: %d", num_cars);
    text_draw(ren, pad + 8, y, scale, COL_WHITE, buf);
    y += line;

    snprintf(buf, sizeof(buf), "HILOS: %d", physics_thread_count());
    text_draw(ren, pad + 8, y, scale, COL_WHITE, buf);
    y += line;

    snprintf(buf, sizeof(buf), "FPS: %d", (int)(fps + 0.5f));
    text_draw(ren, pad + 8, y, scale, COL_WHITE, buf);
    y += line;

    /* El tiempo de simulacion se muestra con dos decimales porque con flotas
     * pequenas baja de un milisegundo. */
    snprintf(buf, sizeof(buf), "FISICA: %d.%02d MS",
             (int)sim_ms, (int)((sim_ms - (int)sim_ms) * 100.0));
    text_draw(ren, pad + 8, y, scale, COL_WHITE, buf);
}
