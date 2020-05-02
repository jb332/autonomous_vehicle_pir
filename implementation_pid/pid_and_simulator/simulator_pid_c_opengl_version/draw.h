/*
 * Created by jb on 30/04/2020.
 */

#ifndef DRAW_H
#define DRAW_H

#include "types.h"

void draw(int width, int height, Vehicle * vehicle_ptr, Circuit * circuit_ptr, Point scale, pthread_mutex_t * mutex_sensors_ptr);

#endif /* DRAW_H */
