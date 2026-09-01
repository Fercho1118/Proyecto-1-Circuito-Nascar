/*
 * Medicion de rendimiento sin ventana.
 *
 * Corre la simulacion sola, sin dibujar nada, repitiendo la misma cantidad de
 * cuadros con distinta cantidad de hilos. Cada configuracion se mide varias
 * veces y se reporta el promedio junto con su dispersion, porque una sola
 * corrida no distingue una mejora real del ruido de la maquina. Al no abrir ventana no interviene la
 * sincronizacion vertical, que de otro modo limitaria todo a la tasa de
 * refresco del monitor y escondaria cualquier diferencia.
 */
#ifndef BENCH_H
#define BENCH_H

/* Corre la medicion e imprime la tabla de resultados. Devuelve cero si todo
 * salio bien. */
int bench_run(int num_vehicles, int frames, unsigned int seed, int repeats);

#endif /* BENCH_H */
