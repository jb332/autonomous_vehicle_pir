/*
 * Created by jb on 30/04/2020.
 */

#ifndef DRAW_H
#define DRAW_H

#include <pthread.h>
#include "types.h"


int draw_loop(Vehicle *         vehicle_ptr,
              Circuit *         circuit_ptr,
              pthread_mutex_t * mutex_sensors_ptr,
              pthread_mutex_t * mutex_draw_event_ptr,
              pthread_cond_t *  draw_event_ptr);

#endif /* DRAW_H */
