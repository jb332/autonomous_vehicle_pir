/*
 * Created by jb on 29/04/2020.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "simulator.h"
#include "types.h"
#include "functions.h"


static void simulate_move(Vehicle *         vehicle_ptr,
                          int               simu_period,
                          float             rand_degrees,
                          float             max_speed,
                          float             max_steer,
                          pthread_mutex_t * mutex_sensors_ptr,
                          pthread_mutex_t * mutex_commands_ptr,
                          bool              steer_dependant_alea)
{
    pthread_mutex_lock(mutex_sensors_ptr);
    Point position = vehicle_ptr->position;
    float angle = vehicle_ptr->angle;
    pthread_mutex_unlock(mutex_sensors_ptr);

    pthread_mutex_lock(mutex_commands_ptr);
    float speed = vehicle_ptr->speed;
    float steer = vehicle_ptr->steer;
    pthread_mutex_unlock(mutex_commands_ptr);

    float delta_t = (float)simu_period / 1000.0;
    float delta_d = speed * (float)delta_t;
    float rand_minus_1_plus_1 = 2.0 * (float)rand() / (float)RAND_MAX - 1.0;
    float speed_ratio = speed / max_speed;

    float new_angle;
    if(steer_dependant_alea) {
        float rand_ratio = rand_degrees / 360.0;
        new_angle = (angle + limit_steer(steer, max_steer) * delta_t) * (1.0 + rand_ratio * rand_minus_1_plus_1 * speed_ratio);
    } else {
        new_angle = angle + limit_steer(steer, max_steer) * delta_t +
                          rand_degrees * rand_minus_1_plus_1 * speed_ratio;
    }

    new_angle = fmod(new_angle, 360.0);
    Point new_position = compute_new_position(position, delta_d, angle);

    pthread_mutex_lock(mutex_sensors_ptr);
    vehicle_ptr->angle = new_angle;
    vehicle_ptr->position = new_position;
    pthread_mutex_unlock(mutex_sensors_ptr);
}

void * simulator_loop(void * args) {
    srand(42);

    /* Begin Simulator Parameters */
    int simu_period = 20;       /* period between each iteration in ms (delta_t), note : going below 20 is useless because opengl does not follow */
    float rand_degrees = 1.0;   /* maximum random deviation (in degrees) in the new angle calculation from steer and previous angle (angle is multiplied by : 1 + rand_degrees / 360 * random_minus1_plus1 * speed / max_speed) */
    float max_speed = 5.0;      /* maximum speed value (speed <= max_speed) */
    float max_steer = 45.0;     /* maximum steer value (-max_steer <= steer <= max_steer) */
    /* End Simulator Parameters */

    Vehicle * vehicle_ptr = ((Vehicle **)args)[0];
    pthread_mutex_t * mutex_sensors_ptr = ((pthread_mutex_t **)args)[1];
    pthread_mutex_t * mutex_commands_ptr = ((pthread_mutex_t **)args)[2];
    pthread_mutex_t * mutex_draw_event_ptr = ((pthread_mutex_t **)args)[3];
    pthread_mutex_t * mutex_shutdown_ptr = ((pthread_mutex_t **)args)[4];
    pthread_cond_t * draw_event_ptr = ((pthread_cond_t **)args)[5];
    bool * shutdown_ptr = ((bool **)args)[6];
    
    /* simulation loop */
    bool finished = false;
    while(!finished) {
        clock_t period_begin_time = clock();

        /* simulation iteration */
        simulate_move(vehicle_ptr, simu_period, rand_degrees, max_speed, max_steer, mutex_sensors_ptr, mutex_commands_ptr, false);

        /* trigger draw event */
        pthread_mutex_lock(mutex_draw_event_ptr);
        pthread_cond_signal(draw_event_ptr);
        pthread_mutex_unlock(mutex_draw_event_ptr);

        /* check for shutdown order */
        pthread_mutex_lock(mutex_shutdown_ptr);
        if(*shutdown_ptr) {
            finished = true;
        }
        pthread_mutex_unlock(mutex_shutdown_ptr);

        /* wait for the remaining period time */
        if(!finished) {
            wait_remaining_period(period_begin_time, simu_period);
        }
    }

    return NULL;
}

