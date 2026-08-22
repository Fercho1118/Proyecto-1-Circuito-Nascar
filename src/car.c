#include "car.h"
#include "config.h"
#include "track.h"

#include <math.h>

Car cars[MAX_CARS];
int num_cars = 1;

void car_sync_screen(Car *car)
{
    Vec2 p = track_point(car->angle, car->lane);
    car->x = p.x;
    car->y = p.y;
    car->heading = track_heading(car->angle);
}

void car_update(Car *car, float dt)
{
    /* La rapidez esta en pixeles por segundo, pero la posicion se guarda como
     * un angulo. track_scale convierte una de esas unidades en la otra, y es
     * lo que hace que el vehiculo mantenga el mismo ritmo sobre el asfalto
     * aunque el ovalo cubra mas recorrido por radian en las rectas. */
    float d_angle = (car->speed / track_scale(car->angle)) * dt;
    car->angle = track_wrap(car->angle + d_angle);

    car_sync_screen(car);
}

void car_init_field(int n)
{
    if (n < 1)        n = 1;
    if (n > MAX_CARS) n = MAX_CARS;
    num_cars = n;

    for (int i = 0; i < n; ++i) {
        Car *c = &cars[i];

        /* La salida se reparte a lo largo del ovalo empezando por la linea de
         * meta, que esta en la parte de abajo de la pista. */
        c->angle = track_wrap((float)M_PI * 0.5f - (float)i * 0.18f);
        c->lane  = 0.0f;
        c->speed = CRUISE_SPEED;
        c->color = (SDL_Color){ 214, 40, 40, 255 };

        car_sync_screen(c);
    }
}
