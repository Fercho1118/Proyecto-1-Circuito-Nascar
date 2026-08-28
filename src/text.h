/*
 * Texto de mapa de bits.
 *
 * SDL por si solo no dibuja texto y traer SDL_ttf obligaria a distribuir una
 * fuente junto con el programa. Como lo unico que hace falta son unas cuantas
 * etiquetas y numeros, el modulo lleva su propia fuente de 5 por 7 pixeles y
 * dibuja cada caracter como un conjunto de rectangulos.
 */
#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>

/* Alto y ancho de un caracter en pixeles de la fuente, antes de escalar. */
#define GLYPH_W 5
#define GLYPH_H 7

/* Dibuja una cadena con la esquina superior izquierda en (x, y). scale repite
 * cada pixel de la fuente ese numero de veces, asi que el alto final del texto
 * es GLYPH_H * scale. Solo reconoce letras mayusculas, digitos y unos pocos
 * signos; lo demas se dibuja como un espacio. */
void text_draw(SDL_Renderer *ren, int x, int y, int scale,
               SDL_Color color, const char *s);

/* Ancho en pixeles que ocuparia la cadena al dibujarla con esa escala. */
int text_width(const char *s, int scale);

#endif /* TEXT_H */
