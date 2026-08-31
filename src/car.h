/*
 * Estado y avance de los vehiculos.
 *
 * Un vehiculo no guarda su posicion como un par de coordenadas sino como una
 * posicion de pista: el angulo que lleva recorrido sobre el ovalo y el carril
 * en el que va. Las coordenadas de pantalla se derivan de ese par y solo se
 * usan para dibujar, lo que permite razonar sobre adelantos y distancias
 * comparando angulos en lugar de resolver geometria.
 */
#ifndef CAR_H
#define CAR_H

#include <SDL.h>

typedef struct {
    float angle;    /* posicion sobre el ovalo, en radianes */
    float lane;      /* desplazamiento lateral respecto a la linea central */
    float lane_vel;  /* rapidez con la que se desplaza de lado, en px/s */
    float lane_goal; /* carril que el vehiculo esta tratando de mantener */
    float speed;    /* rapidez sobre el asfalto, en pixeles por segundo */
    float cruise;   /* rapidez que el vehiculo busca cuando nada lo limita */

    float x, y;     /* posicion en pantalla, derivada de angle y lane */
    float heading;  /* orientacion con la que se dibuja el vehiculo */

    /* Segundos que le quedan al destello con el que se marca un golpe. Se
     * usa solo para dibujar y no influye en el movimiento. */
    float impact_flash;

    SDL_Color color;
} Car;

/* Arreglo con los vehiculos que se actualizan y se dibujan en cada cuadro.
 * num_cars indica cuantas posiciones del arreglo estan en uso. */
extern Car cars[];
extern int num_cars;

/* Coloca n vehiculos sobre la pista y deja el resto del arreglo sin usar. La
 * semilla decide los colores y las diferencias de ritmo entre ellos: con la
 * misma semilla la parrilla sale identica, lo que hace repetibles tanto la
 * escena como las mediciones. */
void car_init_field(int n, unsigned int seed);

/* Recalcula x, y y heading a partir de la posicion de pista del vehiculo. */
void car_sync_screen(Car *car);

/* Calcula la siguiente ubicacion del vehiculo avanzandolo sobre la pista
 * durante dt segundos. */
void car_update(Car *car, float dt);

#endif /* CAR_H */
