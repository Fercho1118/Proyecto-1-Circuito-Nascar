/*
 * Reglas de comportamiento de los vehiculos.
 *
 * Este modulo concentra lo que decide como se mueve la flota cuadro a cuadro:
 * cuanto puede correr cada vehiculo segun lo cerrada que este la curva, que
 * pasa cuando toca el muro y que pasa cuando dos vehiculos se encuentran.
 */
#ifndef PHYSICS_H
#define PHYSICS_H

#include "car.h"

/* Rapidez maxima con la que se puede recorrer la pista en ese punto sin que
 * la fuerza lateral saque al vehiculo de su trayectoria. Es grande en las
 * rectas y pequena en las curvas cerradas. */
float physics_speed_limit(float angle, float lane);

/* Avanza la simulacion completa dt segundos. */
void physics_step(Car *cars, int n, float dt);

#endif /* PHYSICS_H */
