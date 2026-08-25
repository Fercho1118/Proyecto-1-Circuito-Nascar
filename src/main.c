/*
 * Circuito de Nascar - screensaver
 * Proyecto 1, Computacion Paralela (UVG)
 *
 * Abre la ventana de SDL2 donde corre el screensaver y sostiene el ciclo
 * principal: atiende los eventos del usuario, actualiza la posicion de los
 * vehiculos y le pide al modulo de dibujo que arme el cuadro.
 */

#include "config.h"
#include "car.h"
#include "physics.h"
#include "render.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    /* El unico argumento opcional es la cantidad de vehiculos, lo que permite
     * probar la escena con flotas de distinto tamano sin recompilar. */
    int wanted = (argc > 1) ? atoi(argv[1]) : DEFAULT_CARS;

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

    car_init_field(wanted);
    printf("Vehiculos en pista: %d\n", num_cars);

    /* El contador de alto rendimiento mide cuanto dura cada cuadro, de modo
     * que los vehiculos avanzan a la misma velocidad sin importar los fps. */
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

        physics_step(cars, num_cars, dt);

        /* El cuadro se arma de atras hacia adelante: primero el fondo que
         * borra lo anterior, encima la pista y al final los vehiculos. */
        render_background(ren);
        render_track(ren);
        render_cars(ren);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
