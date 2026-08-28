#include "bench.h"
#include "car.h"
#include "config.h"
#include "physics.h"

#include <SDL.h>
#include <stdio.h>

/* Configuraciones de hilos que se prueban. Se descartan las que pasen de lo
 * que la maquina ofrece. */
static const int THREAD_STEPS[] = { 1, 2, 4, 6, 8, 12, 16 };
static const int THREAD_STEPS_LEN =
    (int)(sizeof(THREAD_STEPS) / sizeof(THREAD_STEPS[0]));

/* Corre la simulacion con la cantidad de hilos indicada y devuelve cuantos
 * segundos tardo. Cada corrida vuelve a armar la parrilla, de modo que todas
 * hagan exactamente el mismo trabajo. */
static double run_once(int threads, int num_vehicles, int frames)
{
    physics_set_threads(threads);
    car_init_field(num_vehicles);

    /* Unos cuantos cuadros de calentamiento, para que la medicion no cargue
     * con el costo de levantar los hilos ni con la memoria todavia fria. */
    for (int f = 0; f < 30; ++f)
        physics_step(cars, num_cars, 1.0f / 60.0f);

    Uint64 t0 = SDL_GetPerformanceCounter();
    for (int f = 0; f < frames; ++f)
        physics_step(cars, num_cars, 1.0f / 60.0f);
    Uint64 t1 = SDL_GetPerformanceCounter();

    return (double)(t1 - t0) / (double)SDL_GetPerformanceFrequency();
}

int bench_run(int num_vehicles, int frames)
{
    if (num_vehicles < 1)        num_vehicles = 1;
    if (num_vehicles > MAX_CARS) num_vehicles = MAX_CARS;
    if (frames < 1)              frames = 1;

    if (SDL_Init(SDL_INIT_TIMER) != 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* Cuantos hilos ofrece la maquina, para no probar configuraciones que
     * solo repartirian el mismo trabajo entre hilos que no existen. */
    physics_set_threads(0);
    int available = physics_thread_count();

    printf("Vehiculos: %d   Cuadros: %d   Hilos disponibles: %d\n\n",
           num_vehicles, frames, available);
    printf(" Hilos   Tiempo (s)   ms/cuadro   Aceleracion   Eficiencia\n");
    printf(" -----   ----------   ---------   -----------   ----------\n");

    double base = 0.0;

    for (int k = 0; k < THREAD_STEPS_LEN; ++k) {
        int t = THREAD_STEPS[k];
        if (t > available) break;

        double secs = run_once(t, num_vehicles, frames);
        double ms   = 1000.0 * secs / (double)frames;

        if (k == 0) base = secs;

        /* La aceleracion compara contra la corrida de un solo hilo y la
         * eficiencia dice que tanto de esa ganancia se sostiene por hilo. */
        double speedup = (secs > 0.0) ? base / secs : 0.0;
        double eff     = speedup / (double)t * 100.0;

        printf(" %5d   %10.3f   %9.3f   %10.2fx   %8.0f%%\n",
               t, secs, ms, speedup, eff);
    }

    SDL_Quit();
    return 0;
}
