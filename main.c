/*
 * Circuito de Nascar - screensaver
 * Proyecto 1, Computacion Paralela (UVG)
 *
 * Abre la ventana de SDL2 donde corre el screensaver y sostiene el ciclo
 * principal: atiende los eventos del usuario y repinta la escena en cada
 * cuadro.
 */

#include <SDL.h>
#include <stdio.h>

#define WIN_W 1000
#define WIN_H 650

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
