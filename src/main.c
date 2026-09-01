/*
 * Circuito de Nascar - screensaver
 * Proyecto 1, Computacion Paralela (UVG)
 *
 * Abre la ventana de SDL2 donde corre el screensaver y sostiene el ciclo
 * principal: atiende los eventos del usuario, actualiza la posicion de los
 * vehiculos y le pide al modulo de dibujo que arme el cuadro.
 */

#include "config.h"
#include "bench.h"
#include "car.h"
#include "options.h"
#include "track.h"
#include "physics.h"
#include "render.h"
#include "text.h"

#include <SDL.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    /* Los argumentos se leen y se validan antes de tocar nada. Si algo viene
     * mal, el programa lo reporta y termina en lugar de arrancar con una
     * configuracion que el usuario no pidio. */
    Options opt;
    OptionsResult parsed = options_parse(argc, argv, &opt);

    if (parsed == OPTIONS_HELP)  return 0;
    if (parsed != OPTIONS_OK)    return 1;

    /* Con --bench el programa no abre ventana: corre solo la simulacion y
     * mide como escala al repartirla entre distinta cantidad de hilos. */
    if (opt.bench)
        return bench_run(opt.num_cars, opt.bench_frames, opt.seed,
                         opt.bench_repeats);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Circuito de Nascar",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       opt.win_w, opt.win_h,
                                       SDL_WINDOW_SHOWN);
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

    /* El trazo se ajusta al tamano de la ventana antes de colocar la flota,
     * porque la parrilla se reparte sobre la longitud del circuito. */
    track_configure(opt.win_w, opt.win_h);

    /* La bandera --seq deja fuera el reparto entre hilos, con lo que el mismo
     * binario corre la version secuencial del algoritmo. */
    physics_set_parallel(!opt.sequential);
    physics_set_threads(opt.threads);
    car_init_field(opt.num_cars, opt.seed);
    printf("Vehiculos en pista: %d   version: %s   hilos: %d\n",
           num_cars,
           physics_is_parallel() ? "paralela" : "secuencial",
           physics_thread_count());

    /* El contador de alto rendimiento mide cuanto dura cada cuadro, de modo
     * que los vehiculos avanzan a la misma velocidad sin importar los fps. */
    Uint64 prev = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();
    int running = 1;

    /* Los cuadros por segundo se promedian con un filtro que da mas peso a lo
     * reciente, para que el numero del panel no salte en cada cuadro. */
    float fps = 60.0f;

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

        if (dt > 0.0f)
            fps += ((1.0f / dt) - fps) * 0.05f;

        physics_step(cars, num_cars, dt);

        /* El cuadro se arma de atras hacia adelante: primero el fondo que
         * borra lo anterior, encima la pista y al final los vehiculos. */
        render_background(ren);
        render_track(ren);
        render_cars(ren);
        render_hud(ren, fps, physics_last_ms());
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
