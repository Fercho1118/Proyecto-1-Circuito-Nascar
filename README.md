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

La gracia no está en que los autos se muevan en círculo, sino en que cada uno
se comporte como un piloto independiente. Los autos aceleran en las rectas,
frenan antes de entrar a las curvas, se estorban entre sí, se adelantan y de
vez en cuando terminan raspando el muro. Como cada vehículo toma sus propias
decisiones cuadro a cuadro y hay que revisar cómo interactúa con los demás, la
simulación crece rápido con la cantidad de autos en pista, y ahí es donde entra
la paralelización con OpenMP.

## Las tres funciones que simula la carrera

El comportamiento de la carrera se construye sobre tres reglas:

### 1. Control de velocidad en curvas

Un auto no puede tomar una curva a cualquier velocidad: el agarre de las
llantas limita cuánta fuerza lateral aguanta antes de irse de frente. Mientras
más cerrada la curva, más despacio hay que ir. Cada auto revisa la pista que
tiene por delante y decide cuándo empezar a frenar para llegar a la curva a una
velocidad que pueda sostener, y cuándo volver a acelerar al salir hacia la
recta. Los pilotos no son idénticos: unos son más conservadores y otros entran
más apurados de lo que la pista aguanta.

### 2. Colisión de vehículo contra el muro

La pista está delimitada por un muro interno y uno externo. Un auto que entra a
la curva más rápido de lo que su agarre permite se abre hacia afuera y termina
golpeando el muro. El choque le cuesta velocidad y lo rebota de vuelta hacia la
pista, así que un error en la curva se paga con posiciones perdidas.

### 3. Colisión entre vehículos

Con varios autos peleando la misma línea, los toques son inevitables. Cuando
dos vehículos se traslapan se detecta el contacto y se resuelve según cómo
chocaron: un roce costado con costado los abre de carril, mientras que un toque
de trompa contra cola frena al de atrás y empuja al de adelante.

## Primera entrega

Esta entrega establece la base gráfica del proyecto y define las herramientas
con las que se va a trabajar. Todo está en un solo archivo, [main.c](main.c):

- **Ventana gráfica.** Se eligió **SDL2** como API. La ventana se crea en
  `main()` con un renderer acelerado por hardware y sincronizado al refresco de
  la pantalla.
- **Elementos en memoria.** El arreglo `cars[MAX_CARS]` guarda el estado de los
  vehículos que se dibujan, junto con `num_cars`, que indica cuántas posiciones
  están en uso. Cada `Car` guarda su posición sobre el óvalo, su velocidad, y
  las coordenadas y orientación con las que se dibuja.
- **Función de renderizado.** `draw_car()` dibuja un vehículo como un
  rectángulo rojo orientado en su dirección de avance. La pista se dibuja
  aparte con `draw_track()`, como un anillo de asfalto entre dos elipses.
- **Función de actualización.** `update_car()` calcula la siguiente ubicación
  del vehículo: avanza su ángulo sobre el óvalo según el tiempo transcurrido
  desde el cuadro anterior y de ahí obtiene su posición en pantalla y hacia
  dónde apunta.

Por ahora corre un solo auto rojo sobre la pista. Las tres funciones descritas
arriba y la paralelización con OpenMP corresponden a las siguientes entregas;
el Makefile ya deja el compilador configurado para OpenMP.

## Dependencias

- Un compilador de C (`clang` o `gcc`)
- **SDL2** — la librería gráfica
- **libomp** — el runtime de OpenMP

En macOS, con [Homebrew](https://brew.sh):

```sh
brew install sdl2 libomp
```

En Linux (Debian/Ubuntu):

```sh
sudo apt install build-essential libsdl2-dev libomp-dev
```

## Cómo correrlo

```sh
cd Proyecto-1-Circuito-Nascar
make
./nascar
```

`ESC` o `Q` cierra la ventana. Para recompilar desde cero:

```sh
make clean && make
```
