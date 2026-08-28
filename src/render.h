/*
 * Dibujo de la escena.
 *
 * SDL solo sabe rellenar rectangulos alineados a los ejes, asi que todas las
 * figuras curvas o giradas de este modulo se arman como mallas de triangulos
 * y se envian a la GPU con SDL_RenderGeometry.
 */
#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

/* Borra el cuadro anterior pintando el fondo de cesped. */
void render_background(SDL_Renderer *ren);

/* Dibuja el asfalto, los bordes de la pista y la linea de meta. */
void render_track(SDL_Renderer *ren);

/* Dibuja todos los vehiculos que estan en uso. */
void render_cars(SDL_Renderer *ren);

/* Dibuja el panel con las medidas de la corrida: cuantos vehiculos hay en
 * pista, con cuantos hilos se esta simulando, a cuantos cuadros por segundo
 * va el screensaver y cuanto tarda un paso de simulacion. */
void render_hud(SDL_Renderer *ren, float fps, double sim_ms);

#endif /* RENDER_H */
