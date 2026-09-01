#include "options.h"
#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void options_usage(const char *program)
{
    printf("Screensaver de un circuito de Nascar.\n\n");
    printf("Uso: %s [opciones]\n\n", program);
    printf("Opciones:\n");
    printf("  -n, --autos N     vehiculos en pista, entre 1 y %d"
           " (por omision %d)\n", MAX_CARS, DEFAULT_CARS);
    printf("  -t, --hilos N     hilos de simulacion, entre 1 y %d"
           " (por omision los que elija OpenMP)\n", MAX_THREADS);
    printf("      --bench       mide el rendimiento sin abrir ventana\n");
    printf("      --cuadros N   cuadros por corrida de la medicion, entre 1"
           " y %d (por omision %d)\n", MAX_BENCH_FRAMES, DEFAULT_BENCH_FRAMES);
    printf("  -w, --ancho N     ancho de la ventana, entre %d y %d"
           " (por omision %d)\n", MIN_WIN_W, MAX_WIN_W, DEFAULT_WIN_W);
    printf("  -a, --alto N      alto de la ventana, entre %d y %d"
           " (por omision %d)\n", MIN_WIN_H, MAX_WIN_H, DEFAULT_WIN_H);
    printf("  -s, --semilla N   semilla de la parrilla, entre 0 y %d"
           " (por omision %d)\n", MAX_SEED, DEFAULT_SEED);
    printf("      --repite N    repeticiones de cada configuracion medida,"
           " entre 1 y %d (por omision %d)\n",
           MAX_BENCH_REPEATS, DEFAULT_BENCH_REPEATS);
    printf("      --seq         corre la version secuencial, sin reparto"
           " entre hilos\n");
    printf("  -h, --ayuda       muestra esta ayuda\n\n");
    printf("Ejemplos:\n");
    printf("  %s                       arranca con la configuracion"
           " por omision\n", program);
    printf("  %s -n 300                trescientos vehiculos\n", program);
    printf("  %s -n 300 -t 4           trescientos vehiculos con cuatro"
           " hilos\n", program);
    printf("  %s -n 300 -s 7           otra parrilla, con otros colores\n",
           program);
    printf("  %s -n 300 --seq          version secuencial\n", program);
    printf("  %s --bench -n 1500       mide el rendimiento\n", program);
}

/* Convierte el texto de un argumento a numero entero.
 *
 * Rechaza lo que no sea un entero completo, de modo que algo como "12abc" o
 * "hola" no se acepte como si fuera un numero: strtol deja en end el primer
 * caracter que no pudo leer, y si ahi no viene el fin de la cadena es que
 * sobraba texto. Tambien rechaza lo que se salga del rango permitido, para que
 * el valor no llegue recortado en silencio a quien lo use. */
static int read_int(const char *text, const char *flag,
                    long low, long high, int *out)
{
    if (text == NULL || text[0] == '\0') {
        fprintf(stderr, "Error: %s necesita un valor.\n", flag);
        return 0;
    }

    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);

    if (end == text || *end != '\0') {
        fprintf(stderr, "Error: el valor de %s debe ser un numero entero,"
                        " y se recibio \"%s\".\n", flag, text);
        return 0;
    }

    if (errno == ERANGE || value < low || value > high) {
        fprintf(stderr, "Error: el valor de %s debe estar entre %ld y %ld,"
                        " y se recibio \"%s\".\n", flag, low, high, text);
        return 0;
    }

    *out = (int)value;
    return 1;
}

/* Indica si el argumento corresponde a la opcion, en su forma corta o larga.
 * La forma corta puede venir vacia para las opciones que solo tienen larga. */
static int matches(const char *arg, const char *shortf, const char *longf)
{
    if (shortf != NULL && strcmp(arg, shortf) == 0) return 1;
    return strcmp(arg, longf) == 0;
}

OptionsResult options_parse(int argc, char **argv, Options *opt)
{
    const char *program = (argc > 0) ? argv[0] : "nascar";

    /* Valores por omision. Cualquiera de ellos se puede sobrescribir, pero el
     * programa siempre arranca con una configuracion valida. */
    opt->num_cars     = DEFAULT_CARS;
    opt->threads      = 0;
    opt->sequential   = 0;
    opt->bench        = 0;
    opt->bench_frames  = DEFAULT_BENCH_FRAMES;
    opt->bench_repeats = DEFAULT_BENCH_REPEATS;
    opt->win_w        = DEFAULT_WIN_W;
    opt->win_h        = DEFAULT_WIN_H;
    opt->seed         = DEFAULT_SEED;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (matches(arg, "-h", "--ayuda") || strcmp(arg, "--help") == 0) {
            options_usage(program);
            return OPTIONS_HELP;
        }

        if (strcmp(arg, "--seq") == 0 ||
            strcmp(arg, "--secuencial") == 0) {
            opt->sequential = 1;
            continue;
        }

        if (strcmp(arg, "--bench") == 0) {
            opt->bench = 1;
            continue;
        }

        /* Las opciones que llevan valor lo toman del siguiente argumento, asi
         * que primero hay que confirmar que ese argumento exista. */
        if (matches(arg, "-n", "--autos")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, 1, MAX_CARS, &opt->num_cars))
                return OPTIONS_ERROR;
            continue;
        }

        if (matches(arg, "-t", "--hilos")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, 1, MAX_THREADS, &opt->threads))
                return OPTIONS_ERROR;
            continue;
        }

        if (matches(arg, "-w", "--ancho")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, MIN_WIN_W, MAX_WIN_W, &opt->win_w))
                return OPTIONS_ERROR;
            continue;
        }

        if (matches(arg, "-a", "--alto")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, MIN_WIN_H, MAX_WIN_H, &opt->win_h))
                return OPTIONS_ERROR;
            continue;
        }

        if (matches(arg, "-s", "--semilla")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            int value;
            if (!read_int(argv[++i], arg, 0, MAX_SEED, &value))
                return OPTIONS_ERROR;
            opt->seed = (unsigned int)value;
            continue;
        }

        if (matches(arg, NULL, "--repite")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, 1, MAX_BENCH_REPEATS,
                          &opt->bench_repeats))
                return OPTIONS_ERROR;
            continue;
        }

        if (matches(arg, NULL, "--cuadros")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: %s necesita un valor.\n", arg);
                return OPTIONS_ERROR;
            }
            if (!read_int(argv[++i], arg, 1, MAX_BENCH_FRAMES,
                          &opt->bench_frames))
                return OPTIONS_ERROR;
            continue;
        }

        fprintf(stderr, "Error: opcion desconocida \"%s\".\n\n", arg);
        options_usage(program);
        return OPTIONS_ERROR;
    }

    return OPTIONS_OK;
}
