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
#define TRACK_W  70.0f

/* Ancho de los bordes a rayas que delimitan el asfalto. */
#define KERB_W 8.0f

/* Muestras usadas para aproximar el ovalo al dibujarlo. Mas muestras dan un
 * contorno mas suave a cambio de mas triangulos por cuadro. */
#define TRACK_SAMPLES 180

/* ---- Vehiculos --------------------------------------------------------- */

#define CAR_LEN  34.0f
#define CAR_WID  18.0f
#define MAX_CARS 16

/* Rapidez a la que un vehiculo recorre el asfalto cuando nada lo limita, en
 * pixeles por segundo. */
#define CRUISE_SPEED 320.0f

/* ---- Simulacion -------------------------------------------------------- */

/* Tope del paso de tiempo. Evita saltos grandes cuando la ventana se congela
 * un momento y la simulacion recibe un intervalo enorme de golpe. */
#define MAX_DT 0.05f

#endif /* CONFIG_H */
