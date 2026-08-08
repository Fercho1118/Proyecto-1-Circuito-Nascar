/*
 * Circuito de Nascar - screensaver
 * Proyecto 1, Computacion Paralela (UVG)
 *
 * Abre la ventana de SDL2 donde corre el screensaver y sostiene el ciclo
 * principal: atiende los eventos del usuario, actualiza la posicion de los
 * vehiculos y repinta la escena en cada cuadro.
 */

#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define WIN_W       1000
#define WIN_H       650
#define TRACK_CX    (WIN_W / 2.0f)
#define TRACK_CY    (WIN_H / 2.0f)
#define TRACK_RX    340.0f   /* radio horizontal de la linea central */
#define TRACK_RY    200.0f   /* radio vertical de la linea central   */

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
#define MAX_CARS 16
static Car cars[MAX_CARS];
static int num_cars = 1;

/* ---- Actualizacion ----------------------------------------------------- */

/* Calcula la siguiente ubicacion del auto. Avanza su angulo segun el tiempo
 * transcurrido y de ahi obtiene su posicion en pantalla y su orientacion. */
static void update_car(Car *car, float dt)
{
    car->angle += car->speed * dt;
    if (car->angle > 2.0f * (float)M_PI) car->angle -= 2.0f * (float)M_PI;

    car->x = TRACK_CX + TRACK_RX * cosf(car->angle);
    car->y = TRACK_CY + TRACK_RY * sinf(car->angle);

    /* La tangente del ovalo da la direccion en la que va el auto. */
    car->heading = atan2f(TRACK_RY * cosf(car->angle),
                          -TRACK_RX * sinf(car->angle));
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
        if (dt > 0.05f) dt = 0.05f;

        for (int i = 0; i < num_cars; ++i)
            update_car(&cars[i], dt);

        /* El color de fondo representa el cesped y borra el cuadro anterior
         * antes de dibujar los elementos de la escena. */
        SDL_SetRenderDrawColor(ren, 28, 96, 52, 255);
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
