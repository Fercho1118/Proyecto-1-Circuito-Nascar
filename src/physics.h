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

/* Fija cuantos hilos usa la simulacion. Un valor menor que uno deja el que
 * OpenMP haya elegido segun la maquina. Sin OpenMP la llamada no hace nada. */
void physics_set_threads(int threads);

/* Cuantos hilos esta usando de verdad la simulacion. Devuelve uno cuando el
 * programa se compilo sin OpenMP. */
int physics_thread_count(void);

/* Milisegundos que tardo el ultimo paso de simulacion. Es la medida que
 * interesa para comparar configuraciones, porque deja fuera el dibujo y la
 * espera por la sincronizacion vertical. */
double physics_last_ms(void);

#endif /* PHYSICS_H */
