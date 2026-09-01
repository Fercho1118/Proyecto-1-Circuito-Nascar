#include "bench.h"
#include "car.h"
#include "config.h"
#include "physics.h"

#include <SDL.h>
#include <math.h>
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

/* Resumen estadistico de las repeticiones de una misma configuracion. */
typedef struct {
    double mean;   /* promedio de los tiempos, en segundos */
    double stdev;  /* dispersion alrededor del promedio */
    double best;   /* la corrida mas rapida */
} Stats;

/* Mide una configuracion varias veces y resume los resultados.
 *
 * El promedio es la medida que se reporta, la dispersion dice que tan
 * confiable es ese promedio y la mejor corrida sirve de referencia de lo que
 * la maquina alcanza cuando nada mas la interrumpe. */
static Stats measure(int parallel, int threads, int num_vehicles, int frames,
                     unsigned int seed, int repeats)
{
    double times[MAX_BENCH_REPEATS];
    double sum = 0.0;

    Stats st;
    st.best = 0.0;

    for (int r = 0; r < repeats; ++r) {
        times[r] = run_once(parallel, threads, num_vehicles, frames, seed);
        sum += times[r];
        if (r == 0 || times[r] < st.best) st.best = times[r];
    }

    st.mean = sum / (double)repeats;

    /* Dispersion sobre la muestra. Con una sola corrida no hay dispersion que
     * calcular y se reporta cero. */
    double acc = 0.0;
    for (int r = 0; r < repeats; ++r) {
        double d = times[r] - st.mean;
        acc += d * d;
    }
    st.stdev = (repeats > 1) ? sqrt(acc / (double)(repeats - 1)) : 0.0;

    return st;
}

int bench_run(int num_vehicles, int frames, unsigned int seed, int repeats)
{
    if (num_vehicles < 1)        num_vehicles = 1;
    if (num_vehicles > MAX_CARS) num_vehicles = MAX_CARS;
    if (frames < 1)                   frames = 1;
    if (repeats < 1)                  repeats = 1;
    if (repeats > MAX_BENCH_REPEATS)  repeats = MAX_BENCH_REPEATS;

    if (SDL_Init(SDL_INIT_TIMER) != 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* Cuantos hilos ofrece la maquina, para no probar configuraciones que
     * solo repartirian el mismo trabajo entre hilos que no existen. */
    physics_set_threads(0);
    int available = physics_thread_count();

    printf("Vehiculos: %d   Cuadros por corrida: %d   Repeticiones: %d"
           "   Hilos disponibles: %d\n\n",
           num_vehicles, frames, repeats, available);
    printf(" Version      Hilos   Media (s)   Desv (s)    Mejor (s)"
           "   ms/cuadro   Aceleracion   Eficiencia\n");
    printf(" ----------   -----   ---------   ---------   ---------"
           "   ---------   -----------   ----------\n");

    /* La referencia es la version secuencial, no la paralela con un solo hilo.
     * Esa distincion importa: la corrida de un hilo sigue pasando por el
     * armado del equipo de trabajo de OpenMP, asi que compararse contra ella
     * escondaria el costo de introducir el paralelismo. */
    Stats base = measure(0, 1, num_vehicles, frames, seed, repeats);
    printf(" %-10s   %5s   %9.4f   %9.4f   %9.4f   %9.3f   %11s   %10s\n",
           "secuencial", "-", base.mean, base.stdev, base.best,
           1000.0 * base.mean / (double)frames, "-", "-");

    for (int k = 0; k < THREAD_STEPS_LEN; ++k) {
        int t = THREAD_STEPS[k];
        if (t > available) break;

        Stats st = measure(1, t, num_vehicles, frames, seed, repeats);
        double ms = 1000.0 * st.mean / (double)frames;

        /* La aceleracion compara los promedios contra la version secuencial y
         * la eficiencia dice que tanto de esa ganancia se sostiene por hilo. */
        double speedup = (st.mean > 0.0) ? base.mean / st.mean : 0.0;
        double eff     = speedup / (double)t * 100.0;

        printf(" %-10s   %5d   %9.4f   %9.4f   %9.4f   %9.3f   %10.2fx"
               "   %8.0f%%\n",
               "paralela", t, st.mean, st.stdev, st.best, ms, speedup, eff);
    }

    SDL_Quit();
    return 0;
}
