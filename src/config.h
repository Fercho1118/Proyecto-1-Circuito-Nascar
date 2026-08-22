/*
 * Parametros de configuracion del screensaver.
 *
 * Concentra en un solo lugar las medidas de la ventana, la geometria de la
 * pista y el tamano de los vehiculos, de modo que ajustar la escena no obligue
 * a buscar constantes repartidas entre los modulos.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* ---- Ventana ----------------------------------------------------------- */

#define WIN_W 1000
#define WIN_H 650

/* ---- Pista ------------------------------------------------------------- */

/* La pista es un ovalo centrado en la ventana. Los radios describen la linea
 * central y el ancho se reparte a ambos lados de ella. */
#define TRACK_CX (WIN_W / 2.0f)
#define TRACK_CY (WIN_H / 2.0f)
#define TRACK_RX 340.0f
#define TRACK_RY 200.0f
#define TRACK_W  130.0f

/* Ancho de los bordes a rayas que delimitan el asfalto. */
#define KERB_W 8.0f

/* Muestras usadas para aproximar el ovalo al dibujarlo. Mas muestras dan un
 * contorno mas suave a cambio de mas triangulos por cuadro. */
#define TRACK_SAMPLES 180

/* ---- Vehiculos --------------------------------------------------------- */

#define CAR_LEN  34.0f
#define CAR_WID  18.0f

/* Cota superior de la flota. El arreglo de vehiculos se reserva con este
 * tamano, de modo que la cantidad real se elige al arrancar sin necesidad de
 * pedir memoria durante la simulacion. */
#define MAX_CARS     2048
#define DEFAULT_CARS 48

/* Carriles disponibles. Se reparten simetricamente alrededor de la linea
 * central, separados por LANE_STEP pixeles. */
#define NUM_LANES 3
#define LANE_STEP 37.0f

/* Desplazamiento lateral maximo antes de tocar el muro. Descuenta medio ancho
 * de vehiculo para que el contacto ocurra con la carroceria y no con el eje. */
#define LANE_LIMIT (TRACK_W * 0.5f - CAR_WID * 0.5f)

/* Rapidez a la que un vehiculo recorre el asfalto cuando nada lo limita, en
 * pixeles por segundo. Cada vehiculo recibe una variacion sobre este valor
 * para que la flota no avance en bloque y se produzcan alcances. */
#define CRUISE_SPEED  320.0f
#define CRUISE_SPREAD 0.22f

/* ---- Simulacion -------------------------------------------------------- */

/* Tope del paso de tiempo. Evita saltos grandes cuando la ventana se congela
 * un momento y la simulacion recibe un intervalo enorme de golpe. */
#define MAX_DT 0.05f

#endif /* CONFIG_H */
