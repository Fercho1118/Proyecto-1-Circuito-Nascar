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

/* ---- Comportamiento ---------------------------------------------------- */

/* Aceleracion lateral que aguanta un vehiculo antes de derrapar, en pixeles
 * por segundo al cuadrado. Es lo que fija que tan rapido se puede tomar cada
 * curva: mientras mas alta, mas agarre y mas velocidad en las curvas. */
#define MAX_LATERAL_ACC 620.0f

/* Cotas de aceleracion y de frenado sobre el eje de avance. */
#define ACCEL_MAX 260.0f
#define BRAKE_MAX 540.0f

/* Distancia que el vehiculo mira hacia adelante para anticipar una curva y
 * empezar a frenar antes de llegar a ella. */
#define PREVIEW_DIST 150.0f

/* Direccion. El vehiculo corrige hacia el carril que quiere mantener con una
 * fuerza proporcional a lo lejos que esta de el, y el amortiguamiento evita
 * que se pase de largo y quede oscilando de un lado a otro. */
#define STEER_GAIN 7.0f
#define STEER_DAMP 3.4f
#define LANE_VEL_MAX 110.0f

/* Que tanto empuja hacia afuera el exceso de rapidez en una curva. Es lo que
 * hace que un vehiculo que entra demasiado fuerte se abra y termine contra
 * el muro exterior. */
#define SLIDE_GAIN 3.2f

/* Choque contra el muro. Se conserva parte de la rapidez lateral en sentido
 * contrario, que es el rebote, y se pierde parte de la rapidez de avance. */
#define WALL_RESTITUTION 0.38f
#define WALL_SPEED_KEEP  0.80f

/* Duracion del destello con el que se marca un golpe, en segundos. */
#define IMPACT_FLASH_TIME 0.28f

/* Interaccion entre vehiculos. LOOKAHEAD es la distancia a la que un vehiculo
 * empieza a tomar en cuenta al que lleva adelante para frenar o intentar
 * rebasarlo. */
#define LOOKAHEAD 115.0f

/* Fraccion de la diferencia de rapidez que se transfiere en un golpe entre
 * dos vehiculos. Con uno el choque seria perfectamente elastico. */
#define BUMP_TRANSFER 0.55f

/* Fuerza con la que dos vehiculos encimados se separan de lado. */
#define SEPARATION_GAIN 9.0f

/* Cuanto se abre un vehiculo para intentar un rebase y con que intensidad
 * frena mientras no consigue el hueco. */
#define OVERTAKE_STEER 28.0f
#define FOLLOW_BRAKE   2.4f

/* Piso de rapidez, para que un vehiculo golpeado no quede detenido en pista. */
#define MIN_SPEED 70.0f

/* ---- Simulacion -------------------------------------------------------- */

/* Tope del paso de tiempo. Evita saltos grandes cuando la ventana se congela
 * un momento y la simulacion recibe un intervalo enorme de golpe. */
#define MAX_DT 0.05f

#endif /* CONFIG_H */
