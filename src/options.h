/*
 * Lectura y validacion de los argumentos de linea de comandos.
 *
 * Todo lo que el programa acepta desde afuera pasa por aqui, de modo que el
 * resto del codigo trabaje siempre con valores ya revisados. Un argumento mal
 * escrito o fuera de rango no se corrige en silencio: se reporta con un
 * mensaje que dice cual fue el problema y el programa termina, en lugar de
 * arrancar con una configuracion que el usuario no pidio.
 */
#ifndef OPTIONS_H
#define OPTIONS_H

/* Resultado de leer los argumentos. */
typedef enum {
    OPTIONS_OK    = 0,  /* la configuracion quedo lista para usarse */
    OPTIONS_ERROR = 1,  /* hubo un argumento invalido, ya se reporto */
    OPTIONS_HELP  = 2   /* se pidio la ayuda; no hay nada mas que hacer */
} OptionsResult;

typedef struct {
    int num_cars;      /* vehiculos a poner en pista */
    int threads;       /* hilos de simulacion; cero deja decidir a OpenMP */
    int bench;         /* correr la medicion en lugar del screensaver */
    int bench_frames;  /* cuadros que dura cada corrida de la medicion */
    int win_w, win_h;  /* tamano de la ventana en pixeles */
    unsigned int seed; /* semilla de la parrilla: colores y ritmos */
} Options;

/* Lee los argumentos sobre una configuracion que ya trae los valores por
 * omision. Devuelve OPTIONS_OK si todo estaba bien. */
OptionsResult options_parse(int argc, char **argv, Options *opt);

/* Imprime como se usa el programa. */
void options_usage(const char *program);

#endif /* OPTIONS_H */
