/*
 * Geometria de la pista.
 *
 * La pista es un ovalo y cada punto sobre ella se identifica con dos valores:
 * el angulo recorrido sobre la linea central y un desplazamiento lateral que
 * indica que tan hacia afuera o hacia adentro del centro esta el punto. Este
 * modulo traduce ese par de coordenadas a la pantalla y expone las medidas
 * locales de la curva que el resto del programa necesita.
 */
#ifndef TRACK_H
#define TRACK_H

typedef struct {
    float x, y;
} Vec2;

/* Ajusta el trazo al tamano de la ventana. El circuito conserva su forma y se
 * escala de manera uniforme para caber con holgura. Debe llamarse antes de
 * usar cualquier otra funcion del modulo; mientras no se llame, el trazo tiene
 * las medidas que corresponden a la ventana por omision. */
void track_configure(int win_w, int win_h);

/* Centro geometrico del circuito, en coordenadas de pantalla. */
Vec2 track_center(void);

/* Devuelve el angulo equivalente dentro del rango [0, 2*pi). */
float track_wrap(float angle);

/* Convierte una posicion de pista a coordenadas de pantalla. Un lane positivo
 * aleja el punto del centro del ovalo y uno negativo lo acerca. */
Vec2 track_point(float angle, float lane);

/* Orientacion de la pista en ese angulo, es decir hacia donde avanza un
 * vehiculo que la recorre. */
float track_heading(float angle);

/* Cuantos pixeles de recorrido corresponden a un radian en ese punto. Sirve
 * para convertir entre velocidad lineal y velocidad angular, ya que el ovalo
 * avanza mas rapido en las rectas que en las curvas para un mismo angulo. */
float track_scale(float angle);

/* Radio de curvatura local. Es grande en las rectas y pequeno en las curvas
 * cerradas, y determina que tan rapido se puede tomar cada tramo. */
float track_radius(float angle);

#endif /* TRACK_H */
