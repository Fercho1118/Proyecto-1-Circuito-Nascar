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
static double run_once(int parallel, int threads, int num_vehicles, int frames,
                       unsigned int seed)
{
    physics_set_parallel(parallel);
    physics_set_threads(threads);
    car_init_field(num_vehicles, seed);

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

int bench_run(int num_vehicles, int frames, unsigned int seed)
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
    printf(" Version      Hilos   Tiempo (s)   ms/cuadro   Aceleracion"
           "   Eficiencia\n");
    printf(" ----------   -----   ----------   ---------   -----------"
           "   ----------\n");

    /* La referencia es la version secuencial, no la paralela con un solo hilo.
     * Esa distincion importa: la corrida de un hilo sigue pasando por el
     * armado del equipo de trabajo de OpenMP, asi que compararse contra ella
     * escondaria el costo de introducir el paralelismo. */
    double base = run_once(0, 1, num_vehicles, frames, seed);
    printf(" %-10s   %5s   %10.3f   %9.3f   %10s   %10s\n",
           "secuencial", "-", base, 1000.0 * base / (double)frames, "-", "-");

    for (int k = 0; k < THREAD_STEPS_LEN; ++k) {
        int t = THREAD_STEPS[k];
        if (t > available) break;

        double secs = run_once(1, t, num_vehicles, frames, seed);
        double ms   = 1000.0 * secs / (double)frames;

        /* La aceleracion compara contra la version secuencial y la eficiencia
         * dice que tanto de esa ganancia se sostiene por hilo. */
        double speedup = (secs > 0.0) ? base / secs : 0.0;
        double eff     = speedup / (double)t * 100.0;

        printf(" %-10s   %5d   %10.3f   %9.3f   %10.2fx   %8.0f%%\n",
               "paralela", t, secs, ms, speedup, eff);
    }

    SDL_Quit();
    return 0;
}
