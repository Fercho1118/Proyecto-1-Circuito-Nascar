#include "car.h"
#include "config.h"
#include "track.h"

#include <math.h>

Car cars[MAX_CARS];
int num_cars = DEFAULT_CARS;

void car_sync_screen(Car *car)
{
    Vec2 p = track_point(car->angle, car->lane);
    car->x = p.x;
    car->y = p.y;
    car->heading = track_heading(car->angle);
}

void car_update(Car *car, float dt)
{
    /* El desplazamiento lateral avanza con su propia rapidez. Las reglas de
     * choque y de adelanto son las que la modifican. */
    car->lane += car->lane_vel * dt;

    if (car->impact_flash > 0.0f) {
        car->impact_flash -= dt;
        if (car->impact_flash < 0.0f) car->impact_flash = 0.0f;
    }

    /* La rapidez esta en pixeles por segundo, pero la posicion se guarda como
     * un angulo. track_scale convierte una de esas unidades en la otra, y es
     * lo que hace que el vehiculo mantenga el mismo ritmo sobre el asfalto
     * aunque el ovalo cubra mas recorrido por radian en las rectas. */
    float d_angle = (car->speed / track_scale(car->angle)) * dt;
    car->angle = track_wrap(car->angle + d_angle);

    car_sync_screen(car);
}

/* Reparte los colores sobre el circulo cromatico para que dos vehiculos
 * vecinos en el arreglo nunca queden del mismo tono. La saturacion alta y el
 * brillo parejo mantienen la paleta legible sobre el asfalto oscuro. */
static SDL_Color car_color(int i)
{
    /* El paso irracional evita que la secuencia de tonos se repita en ciclos
     * cortos cuando la flota es grande. */
    float h = fmodf((float)i * 0.61803399f, 1.0f) * 6.0f;
    int   sector = (int)h;
    float f = h - (float)sector;

    float v = 0.94f, p = 0.22f;
    float q = v * (1.0f - f * 0.78f);
    float t = v * (1.0f - (1.0f - f) * 0.78f);
    float r, g, b;

    switch (sector) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    SDL_Color c = { (Uint8)(r * 255.0f), (Uint8)(g * 255.0f),
                    (Uint8)(b * 255.0f), 255 };
    return c;
}

void car_init_field(int n)
{
    if (n < 1)        n = 1;
    if (n > MAX_CARS) n = MAX_CARS;
    num_cars = n;

    /* La flota se ordena en filas de NUM_LANES vehiculos. La separacion entre
     * filas es la que deja un hueco comodo detras de cada auto, pero si hay
     * tantas filas que no caben en una vuelta se reparten uniformemente para
     * que ninguna quede montada sobre otra. */
    int   rows = (n + NUM_LANES - 1) / NUM_LANES;
    float scale = track_scale(0.0f);
    float gap = (CAR_LEN + 24.0f) / scale;
    float even = 2.0f * (float)M_PI / (float)rows;
    if (gap > even) gap = even;

    for (int i = 0; i < n; ++i) {
        Car *c = &cars[i];

        int row  = i / NUM_LANES;
        int lane = i % NUM_LANES;

        /* La salida arranca en la linea de meta y se extiende hacia atras. */
        c->angle = track_wrap((float)M_PI * 0.5f - (float)row * gap);
        c->lane      = ((float)lane - (float)(NUM_LANES - 1) * 0.5f) * LANE_STEP;
        c->lane_goal = c->lane;
        c->lane_vel  = 0.0f;
        c->impact_flash = 0.0f;

        /* Cada vehiculo recibe una rapidez de crucero ligeramente distinta,
         * repartida de forma pareja sobre el rango permitido. */
        float t = (n > 1) ? (float)i / (float)(n - 1) : 0.5f;
        c->cruise = CRUISE_SPEED * (1.0f - CRUISE_SPREAD * 0.5f
                                        + CRUISE_SPREAD * t);
        c->speed  = c->cruise;
        c->color  = car_color(i);

        car_sync_screen(c);
    }
}
