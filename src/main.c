/*
 * Circuito de Nascar - screensaver
 * Proyecto 1, Computacion Paralela (UVG)
 *
 * Abre la ventana de SDL2 donde corre el screensaver y sostiene el ciclo
 * principal: atiende los eventos del usuario, actualiza la posicion de los
 * vehiculos y dibuja la pista con los vehiculos encima en cada cuadro.
 */

#include "config.h"
#include "track.h"

#include <SDL.h>
#include <math.h>
#include <stdio.h>

/* ---- Elemento a renderizar -------------------------------------------- */

/* Un vehiculo. Su posicion real es el angulo sobre el ovalo; x, y y heading
 * se derivan de ese angulo y solo sirven para dibujarlo. */
typedef struct {
    float angle;    /* posicion sobre el ovalo, en radianes */
    float speed;    /* radianes por segundo */
    float x, y;     /* posicion en pantalla, derivada de angle */
    float heading;  /* hacia donde apunta el auto */
} Car;

/* Arreglo con los vehiculos que se actualizan y se dibujan en cada cuadro.
 * num_cars indica cuantas posiciones del arreglo estan en uso. */
static Car cars[MAX_CARS];
static int num_cars = 1;

/* ---- Actualizacion ----------------------------------------------------- */

/* Calcula la siguiente ubicacion del auto. Avanza su angulo segun el tiempo
 * transcurrido y le pide al modulo de pista la posicion y la orientacion que
 * corresponden a ese angulo. */
static void update_car(Car *car, float dt)
{
    car->angle = track_wrap(car->angle + car->speed * dt);

    Vec2 p = track_point(car->angle, 0.0f);
    car->x = p.x;
    car->y = p.y;
    car->heading = track_heading(car->angle);
}

/* ---- Dibujo ------------------------------------------------------------ */

/* Dibuja un rectangulo relleno girado un angulo. SDL solo rellena rectangulos
 * alineados a los ejes, asi que la figura se arma con dos triangulos cuyos
 * vertices se rotan a mano alrededor del centro. */
static void fill_rotated_rect(SDL_Renderer *ren, float cx, float cy,
                              float half_len, float half_wid,
                              float angle, SDL_Color color)
{
    float ca = cosf(angle), sa = sinf(angle);
    float ox[4] = { -half_len,  half_len, half_len, -half_len };
    float oy[4] = { -half_wid, -half_wid, half_wid,  half_wid };

    SDL_Vertex v[4];
    for (int i = 0; i < 4; ++i) {
        v[i].position.x  = cx + ox[i] * ca - oy[i] * sa;
        v[i].position.y  = cy + ox[i] * sa + oy[i] * ca;
        v[i].color       = color;
        v[i].tex_coord.x = 0.0f;
        v[i].tex_coord.y = 0.0f;
    }
    const int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(ren, NULL, v, 4, idx, 6);
}

/* Dibuja la pista como un anillo de asfalto. Recorre el ovalo tomando muestras
 * a intervalos regulares y en cada una pide los dos bordes del asfalto; los
 * tramos entre muestras consecutivas se rellenan con triangulos. */
static void draw_track(SDL_Renderer *ren)
{
    const int SAMPLES = TRACK_SAMPLES;
    const SDL_Color asphalt = { 48, 48, 54, 255 };

    SDL_Vertex verts[SAMPLES * 2];
    int        idx[SAMPLES * 6];

    for (int i = 0; i < SAMPLES; ++i) {
        float t = 2.0f * (float)M_PI * (float)i / (float)SAMPLES;

        /* Los bordes se obtienen desplazando la linea central medio ancho de
         * pista hacia cada lado, siguiendo la normal del ovalo. */
        Vec2 inner = track_point(t, -TRACK_W * 0.5f);
        Vec2 outer = track_point(t,  TRACK_W * 0.5f);

        verts[i * 2 + 0].position.x = inner.x;
        verts[i * 2 + 0].position.y = inner.y;
        verts[i * 2 + 1].position.x = outer.x;
        verts[i * 2 + 1].position.y = outer.y;

        for (int k = 0; k < 2; ++k) {
            verts[i * 2 + k].color       = asphalt;
            verts[i * 2 + k].tex_coord.x = 0.0f;
            verts[i * 2 + k].tex_coord.y = 0.0f;
        }
    }

    /* Dos triangulos por tramo. El modulo hace que el ultimo tramo se una con
     * el primero y el anillo quede cerrado. */
    for (int i = 0; i < SAMPLES; ++i) {
        int a = i * 2, b = a + 1;
        int c = ((i + 1) % SAMPLES) * 2, d = c + 1;
        idx[i * 6 + 0] = a; idx[i * 6 + 1] = b; idx[i * 6 + 2] = d;
        idx[i * 6 + 3] = a; idx[i * 6 + 4] = d; idx[i * 6 + 5] = c;
    }

    SDL_RenderGeometry(ren, NULL, verts, SAMPLES * 2, idx, SAMPLES * 6);
}

/* Dibuja un auto como un rectangulo rojo orientado en su direccion de avance. */
static void draw_car(SDL_Renderer *ren, const Car *car)
{
    const SDL_Color red = { 214, 40, 40, 255 };
    fill_rotated_rect(ren, car->x, car->y,
                      CAR_LEN * 0.5f, CAR_WID * 0.5f, car->heading, red);
}

/* ---- Programa ---------------------------------------------------------- */

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Circuito de Nascar",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!win) {
        printf("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* El renderer acelerado delega el dibujo a la GPU y la sincronizacion
     * vertical limita el ciclo a la tasa de refresco del monitor. */
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
                                           SDL_RENDERER_ACCELERATED |
                                           SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        printf("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    /* El auto arranca en la parte de abajo del ovalo. La llamada con dt cero
     * no lo mueve: solo calcula su posicion inicial en pantalla. */
    cars[0].angle   = (float)M_PI * 0.5f;
    cars[0].speed   = 0.9f;
    update_car(&cars[0], 0.0f);

    /* El contador de alto rendimiento mide cuanto dura cada cuadro, de modo
     * que el auto avanza a la misma velocidad sin importar los fps. */
    Uint64 prev = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();
    int running = 1;

    while (running) {
        /* La cola de eventos se vacia en cada cuadro. El screensaver termina
         * al cerrar la ventana o al presionar ESC o Q. */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN &&
                (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))
                running = 0;
        }

        /* Segundos transcurridos desde el cuadro anterior. El tope evita un
         * salto grande cuando la ventana se queda congelada un momento. */
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now - prev) / freq);
        prev = now;
        if (dt > MAX_DT) dt = MAX_DT;

        for (int i = 0; i < num_cars; ++i)
            update_car(&cars[i], dt);

        /* El cuadro se arma de atras hacia adelante: primero el cesped que
         * borra lo anterior, encima la pista y al final los vehiculos. */
        SDL_SetRenderDrawColor(ren, 28, 96, 52, 255);
        SDL_RenderClear(ren);
        draw_track(ren);
        for (int i = 0; i < num_cars; ++i)
            draw_car(ren, &cars[i]);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
