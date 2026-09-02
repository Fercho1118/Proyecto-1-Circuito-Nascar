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

/* Tamano por omision de la ventana. Se puede cambiar por linea de comandos
 * dentro de las cotas de abajo; el minimo respeta el tamano de canvas que pide
 * el enunciado del proyecto. */
#define DEFAULT_WIN_W 1000
#define DEFAULT_WIN_H 650
#define MIN_WIN_W     640
#define MIN_WIN_H     480
#define MAX_WIN_W     7680
#define MAX_WIN_H     4320

/* ---- Pista ------------------------------------------------------------- */

/* La pista sigue el trazo de un ovalo de Nascar: dos rectas paralelas unidas
 * por dos curvas semicirculares, no una elipse. La diferencia importa, porque
 * en una elipse la curvatura cambia en todo momento y no existe ningun tramo
 * verdaderamente recto, mientras que en un ovalo real la curva es constante
 * dentro de cada giro y desaparece por completo en las rectas.
 *
 * TRACK_R es el radio de las dos curvas y TRACK_STRAIGHT el largo de cada
 * recta, ambos medidos sobre la linea central, para el tamano de ventana por
 * omision. Con otra ventana el trazo se escala en la misma proporcion para
 * conservar la forma. */
#define TRACK_R        190.0f
#define TRACK_STRAIGHT 320.0f
#define TRACK_W        130.0f

/* Radio que se reporta en las rectas. No es infinito para no arriesgar una
 * division entre cero, pero es lo bastante grande como para que el limite de
 * rapidez que sale de el nunca llegue a aplicarse. */
#define STRAIGHT_RADIUS 1.0e6f

/* Ancho de los bordes a rayas que delimitan el asfalto. */
#define KERB_W 8.0f

/* Muestras usadas para aproximar el ovalo al dibujarlo. Mas muestras dan un
 * contorno mas suave a cambio de mas triangulos por cuadro. */
#define TRACK_SAMPLES 240

/* ---- Vehiculos --------------------------------------------------------- */

#define CAR_LEN  34.0f
#define CAR_WID  18.0f

/* Cota superior de la flota. El arreglo de vehiculos se reserva con este
 * tamano, de modo que la cantidad real se elige al arrancar sin necesidad de
 * pedir memoria durante la simulacion. */
#define MAX_CARS     2048
#define DEFAULT_CARS 48

/* Cotas de los demas valores que se pueden dar por linea de comandos. Sirven
 * para rechazar entradas absurdas antes de que lleguen a usarse. */
#define MAX_THREADS          64
#define MAX_SEED             2147483647
#define DEFAULT_SEED         2026
#define MAX_BENCH_FRAMES     100000
#define DEFAULT_BENCH_FRAMES 300

/* Repeticiones de cada configuracion en la medicion. Una sola corrida no dice
 * mucho: el sistema operativo reparte el procesador entre otros procesos y dos
 * corridas iguales rara vez tardan lo mismo. Repetir y reportar promedio y
 * dispersion es lo que permite afirmar que una diferencia es real. */
#define MAX_BENCH_REPEATS     1000
#define DEFAULT_BENCH_REPEATS 10

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
 * curva: mientras mas alta, mas agarre y mas velocidad en las curvas. El valor
 * esta ajustado al radio de las curvas del ovalo para que el limite quede por
 * debajo de la rapidez de crucero y frenar antes de entrar tenga sentido. */
#define MAX_LATERAL_ACC 330.0f

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

/* Duracion del destello con el que se marca un golpe, en segundos, y cuanto
 * llega a aclarar la carroceria. El tope evita que una flota en contacto
 * permanente termine dibujada por completo de blanco. */
#define IMPACT_FLASH_TIME 0.28f
#define FLASH_MAX         0.65f

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

/* Piso y techo de rapidez. El piso evita que un vehiculo golpeado quede
 * detenido en pista y el techo acota lo que puede ganar en un encontronazo. */
#define MIN_SPEED  70.0f
#define MAX_SPEED  (CRUISE_SPEED * 1.7f)

/* Cotas de lo que un solo cuadro puede cambiarle a un vehiculo por efecto de
 * sus vecinos.
 *
 * Cada contacto por separado aporta poco, pero en un amontonamiento un mismo
 * vehiculo puede estar tocando a muchos a la vez y la suma se dispara. Sin
 * estas cotas ese exceso lo lanza mas rapido todavia contra los siguientes, la
 * realimentacion se multiplica cuadro a cuadro y la simulacion termina
 * desbordando. Acotar la reaccion mantiene el amontonamiento incomodo, que es
 * lo que se quiere, sin que se vuelva inestable. */
#define BUMP_MAX         120.0f
#define PUSH_MAX         900.0f
#define FOLLOW_BRAKE_MAX 900.0f

/* ---- Simulacion -------------------------------------------------------- */

/* Tope del paso de tiempo. Evita saltos grandes cuando la ventana se congela
 * un momento y la simulacion recibe un intervalo enorme de golpe. */
#define MAX_DT 0.05f

#endif /* CONFIG_H */
