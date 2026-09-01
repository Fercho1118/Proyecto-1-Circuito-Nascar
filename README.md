# Circuito de Nascar — Screensaver

Proyecto 1 · Computación Paralela · Universidad del Valle de Guatemala

**Integrantes**

| Nombre | Carné |
|---|---|
| Fernando Rueda | 23748 |
| Jorge Luis Felpe Aguilar | 23195 |
| Fernando Hernández | 23645 |

## La idea

Un screensaver que muestra una carrera de Nascar vista desde arriba, con la
estética de la película *Cars*: un óvalo de asfalto rodeado de césped y un
pelotón de autos de colores dando vueltas sin parar.

El trazo sigue la forma de un óvalo de Nascar real, que no es una elipse sino
dos rectas paralelas unidas por dos curvas semicirculares. La diferencia no es
cosmética: en una elipse la curvatura cambia en todo momento y no existe ningún
tramo verdaderamente recto, mientras que en un óvalo real la curva es constante
dentro de cada giro y desaparece por completo en las rectas. Eso es lo que hace
que aparezca un punto de frenado definido antes de entrar a cada curva.

La gracia no está en que los autos se muevan en círculo, sino en que cada uno
se comporte como un piloto independiente. Los autos aceleran en las rectas,
frenan antes de entrar a las curvas, se estorban entre sí, se adelantan y de
vez en cuando terminan raspando el muro. Como cada vehículo toma sus propias
decisiones cuadro a cuadro y hay que revisar cómo interactúa con los demás, la
simulación crece rápido con la cantidad de autos en pista, y ahí es donde entra
la paralelización con OpenMP.

## Las tres funciones que simula la carrera

### 1. Control de velocidad en curvas

Un vehículo que gira describe un arco de radio `R`, y la aceleración lateral
que eso genera es `v²/R`. Si esa aceleración supera lo que agarra el neumático,
el auto se sale de su trayectoria. Despejando queda la rapidez máxima a la que
se puede tomar cada punto de la pista:

```
v_max = raíz(a_lateral_max · R)
```

En el trazo hay solo dos situaciones. En las rectas la trayectoria no gira, el
radio es infinito y la fórmula no impone ningún límite: el auto corre a su
velocidad de crucero. En las curvas el radio es constante e igual al de la
semicircunferencia, así que el límite es el mismo en todo el giro. El carril
también cuenta, porque la trazada exterior describe una curva más abierta y
admite más velocidad que la interior, que es la razón por la que conviene
abrirse en la curva aunque el recorrido sea más largo.

Además el auto no mira solo el punto donde está: consulta el límite un tramo más
adelante y se queda con el menor de los dos. Como el límite cae de golpe al
entrar a la curva, eso produce un punto de frenado bien definido sobre la recta,
igual que en una carrera real, y el auto vuelve a acelerar al salir. El frenado
es más agresivo que la aceleración, como en un auto real.

### 2. Colisión de vehículo contra el muro

Los muros son los bordes del asfalto. Un auto los toca cuando su desplazamiento
lateral supera lo que permite el ancho de la pista, y cuando eso pasa no los
atraviesa: se le devuelve al límite, su velocidad lateral se invierte
conservando solo una parte —eso es el rebote— y pierde velocidad de avance por
el roce. Después de golpear, el auto se reorienta hacia el interior de la pista.

Esta regla queda ligada a la anterior: cuando un auto entra a una curva más
rápido de lo que el radio permite, ese exceso lo empuja hacia afuera, le abre la
trayectoria y es lo que termina llevándolo contra el muro exterior.

### 3. Colisión entre vehículos

Es la única regla que obliga a mirar **pares** de autos en lugar de uno a la vez,
y por lo tanto la que carga el trabajo computacional.

Cada auto recorre a los demás y mide la separación sobre el asfalto. Si las dos
carrocerías se traslapan hay contacto: el que viene más rápido por detrás cede
velocidad y el de adelante la recibe, y ambos se separan de lado con una fuerza
que crece con el traslape. La transferencia es antisimétrica, así que lo que uno
pierde es lo que el otro gana y el pelotón no se acelera ni se frena en conjunto
por efecto de los golpes.

Si todavía no hay contacto pero un auto viene alcanzando a otro más lento que le
tapa el paso, frena en proporción a lo cerca que está y se abre buscando el hueco
por el lado donde le queda más asfalto. De ahí salen los adelantamientos.

La reacción a los vecinos está acotada por cuadro. Cada contacto por separado
aporta poco, pero en un amontonamiento un mismo auto puede estar tocando a
muchos a la vez y la suma se dispara: sin esa cota, el exceso lo lanza más
rápido todavía contra los siguientes, la realimentación se multiplica cuadro a
cuadro y la simulación termina desbordando. Con la cota el amontonamiento sigue
siendo incómodo, que es lo que se quiere, pero se mantiene estable incluso con
más autos de los que caben en la pista.

## Dependencias

El proyecto necesita dos cosas, ambas instalables con Homebrew en macOS:

```sh
brew install sdl2 libomp
```

- **SDL2** — la ventana, el renderer acelerado y los eventos de teclado.
- **libomp** — la implementación de OpenMP. El `clang` que trae macOS no la
  incluye, así que se toma de Homebrew. El Makefile la detecta solo; si no está
  instalada, el proyecto compila igual y corre en versión secuencial.

En Linux basta con `sudo apt install libsdl2-dev` y `gcc`, que ya trae OpenMP.

## Cómo correrlo

```sh
git clone https://github.com/Fercho1118/Proyecto-1-Circuito-Nascar.git
cd Proyecto-1-Circuito-Nascar
make
./nascar
```

El screensaver se cierra con `ESC`, con `Q` o cerrando la ventana.

### Opciones

Todos los parámetros se leen de la línea de comandos; no hay valores fijados en
el código que haya que recompilar para cambiar.

| Opción | Qué hace | Rango | Por omisión |
|---|---|---|---|
| `-n`, `--autos N` | vehículos en pista | 1 – 2048 | 48 |
| `-t`, `--hilos N` | hilos de simulación | 1 – 64 | los que elija OpenMP |
| `-w`, `--ancho N` | ancho de la ventana | 640 – 7680 | 1000 |
| `-a`, `--alto N` | alto de la ventana | 480 – 4320 | 650 |
| `-s`, `--semilla N` | semilla de la parrilla | 0 – 2147483647 | 2026 |
| `--seq` | corre la versión secuencial | — | apagado |
| `--bench` | mide el rendimiento, sin ventana | — | apagado |
| `--cuadros N` | cuadros por corrida de medición | 1 – 100000 | 300 |
| `--repite N` | repeticiones de cada configuración | 1 – 1000 | 10 |
| `-h`, `--ayuda` | muestra la ayuda | — | — |

```sh
./nascar -n 300               # trescientos vehículos
./nascar -n 300 -t 4          # forzando cuatro hilos
./nascar -n 300 -s 7          # otra parrilla, otros colores
./nascar -w 1600 -a 900       # en una ventana más grande
./nascar -n 300 --seq         # versión secuencial
```

El panel de la esquina muestra cuántos autos hay en pista, con cuántos hilos se
está simulando, los cuadros por segundo y cuánto tarda un paso de simulación.

### Programación defensiva

Todo argumento pasa por validación antes de usarse. La conversión se hace con
`strtol` y se verifica que no haya sobrado texto, de modo que `12abc` o `hola`
no se acepten como números —cosa que `atoi` sí haría, devolviendo cero en
silencio—, que el valor esté dentro de su rango, y que las opciones que llevan
valor efectivamente lo traigan. Cualquier problema se reporta diciendo cuál fue
y el programa termina con código distinto de cero.

```
$ ./nascar -n abc
Error: el valor de -n debe ser un numero entero, y se recibio "abc".

$ ./nascar -n 99999
Error: el valor de -n debe estar entre 1 y 2048, y se recibio "99999".

$ ./nascar --pista 5
Error: opcion desconocida "--pista".
```

## Versión secuencial y versión paralela

El proyecto produce dos ejecutables a partir de los mismos fuentes:

```sh
make        # nascar      — versión paralela, con OpenMP
make seq    # nascar-seq  — versión secuencial, compilada sin OpenMP
make both   # ambos
```

En `nascar-seq` las directivas `#pragma omp` quedan como comentarios para el
compilador y el binario ni siquiera depende de la librería de hilos.

Además, el binario paralelo trae la bandera `--seq`, que apaga el reparto
mediante la cláusula `if` de las directivas: OpenMP no llega a formar el equipo
de trabajo y las tres pasadas se recorren de corrido en el hilo que las invocó.

Esa distinción importa para medir. Correr la versión paralela con un solo hilo
**no** es lo mismo que la versión secuencial: sigue pagando el armado del equipo
de trabajo. Por eso la medición toma como referencia la versión secuencial y
deja la fila de un hilo como dato aparte, precisamente para poder ver cuánto
cuesta introducir el paralelismo por sí mismo. En la práctica ese costo resulta
ser cerca del 1%.

## Medición de rendimiento

Con `--bench` el programa no abre ventana: corre solo la simulación y repite
cada configuración varias veces.

```sh
./nascar --bench -n 1200 --cuadros 150 --repite 10
```

Dejar el dibujo fuera es lo que hace útiles los números. Con ventana, la
sincronización vertical amarra el ciclo a la tasa de refresco del monitor y
esconde cualquier diferencia entre configuraciones.

Cada configuración se mide diez veces porque una sola corrida no distingue una
mejora real del ruido de la máquina: el sistema operativo reparte el procesador
entre otros procesos y dos corridas iguales rara vez tardan lo mismo. Se reporta
el promedio, su desviación y la corrida más rápida.

Resultado en una MacBook Pro con Apple Silicon (14 hilos disponibles):

```
Vehiculos: 1200   Cuadros por corrida: 150   Repeticiones: 10   Hilos disponibles: 14

 Version      Hilos   Media (s)   Desv (s)    Mejor (s)   ms/cuadro   Aceleracion   Eficiencia
 ----------   -----   ---------   ---------   ---------   ---------   -----------   ----------
 secuencial       -      0.4034      0.0027      0.4000       2.689             -            -
 paralela         1      0.4075      0.0044      0.4006       2.717         0.99x          99%
 paralela         2      0.2116      0.0015      0.2103       1.411         1.91x          95%
 paralela         4      0.1145      0.0011      0.1137       0.763         3.52x          88%
 paralela         6      0.0826      0.0005      0.0817       0.551         4.88x          81%
 paralela         8      0.0707      0.0011      0.0687       0.471         5.70x          71%
 paralela        12      0.0761      0.0005      0.0756       0.507         5.30x          44%
```

Tres cosas que se leen ahí:

La desviación es de menos del 1% del promedio en todas las filas, así que las
diferencias entre configuraciones son reales y no ruido.

La escalabilidad se sostiene bien hasta 8 hilos y se estanca después. Eso
coincide con la arquitectura del procesador: los núcleos adicionales son de
eficiencia, mucho más lentos que los de rendimiento, así que agregarlos ya no
aporta trabajo útil al mismo ritmo. Con 12 hilos el tiempo incluso empeora.

La fila de un hilo tarda apenas 1% más que la versión secuencial, lo que dice
que el costo de introducir OpenMP es despreciable frente a lo que se gana.

## Qué se paraleliza

El paso de simulación está partido en tres pasadas y cada una se reparte entre
los hilos por separado:

1. **Exploración de vecinos** — cada auto mira a todos los demás. Es `O(n²)` y
   es la que carga el trabajo; con 1500 autos son más de dos millones de
   comparaciones por cuadro.
2. **Decisión** — cada auto aplica lo que resolvió y ajusta velocidad y
   dirección. Es `O(n)`.
3. **Integración** — recién aquí los autos se mueven y se resuelve el contacto
   con el muro. Es `O(n)`.

No hace falta ningún candado. En las tres pasadas la iteración `i` escribe
únicamente en la posición `i` de su arreglo, así que dos hilos nunca tocan el
mismo dato. Lo que cada auto decide en la primera pasada se guarda en un arreglo
aparte y se aplica hasta la segunda: por eso todos deciden sobre la misma foto
del cuadro anterior y el resultado **no depende del orden en que se recorra el
pelotón**. Correr con 1, 2, 4 u 8 hilos produce exactamente el mismo estado.

La barrera implícita al final de cada pasada es la que mantiene el orden entre
mirar, decidir y avanzar.

El dibujo se queda deliberadamente en un solo hilo, porque SDL no admite que
varios hilos usen el mismo renderer.

## Estructura del proyecto

```
src/
  main.c        ciclo principal, ventana y eventos
  config.h      constantes y cotas de validación
  options.h/.c  lectura y validación de los argumentos
  track.h/.c    geometría del óvalo: posición, orientación y curvatura
  car.h/.c      estado de los vehículos y su avance sobre la pista
  physics.h/.c  las tres reglas y el paso de simulación paralelo
  render.h/.c   dibujo de la pista, los vehículos y el panel
  text.h/.c     fuente de mapa de bits para el panel
  bench.h/.c    medición de rendimiento sin ventana
Makefile
```

Un vehículo no guarda su posición como un par de coordenadas sino como una
posición de pista: cuánto lleva recorrido del circuito y en qué carril va. Las
coordenadas de pantalla se derivan de ese par y solo se usan para dibujar. Eso
es lo que permite razonar sobre adelantamientos y distancias comparando dos
números, en lugar de resolver geometría en cada cuadro.

Ese avance se expresa como un ángulo entre cero y una vuelta completa, aunque el
trazo ya no sea una elipse: no es un ángulo geométrico sino la fracción del
circuito ya recorrida. Mantener esa interfaz es lo que permitió cambiar la forma
de la pista tocando únicamente `track.c`, sin modificar la física, el dibujo ni
el avance de los vehículos.
