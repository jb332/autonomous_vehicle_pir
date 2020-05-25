/*
 * Created by jb on 29/04/2020.
 */

#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "pid.h"
#include "types.h"
#include "functions.h"


void * pid_loop(void * args) {
    Vehicle * vehicle_ptr = ((Vehicle **)args)[0];
    Circuit * circuit_ptr = ((Circuit **)args)[1];
    pthread_mutex_t * mutex_sensors_ptr = ((pthread_mutex_t **)args)[2];
    pthread_mutex_t * mutex_commands_ptr = ((pthread_mutex_t **)args)[3];
    pthread_mutex_t * mutex_shutdown_ptr = ((pthread_mutex_t **)args)[4];
    bool * shutdown_ptr = ((bool **)args)[5];

    /* Parameters PID Begin */
    int pid_period = 50;
    float max_speed = 5.0;
    float stop_distance = 1.0;
    float kp = 3.0;
    float ki = 0.1;
    float kd = 10.0;
    /* Parameters PID End */

    float previous_error_direction = 0.0;
    float total_error_direction = 0.0;

    int currently_targeted_stop_point = (int)(circuit_ptr->clockwise); /* from 0 to 3 (bottom left, top left, top right, bottom right) */
    int currently_targeted_aux_point = 0; /* from 0 to (n_aux_points - 1) */
    Point destination = circuit_ptr->aux_points[currently_targeted_stop_point][currently_targeted_aux_point];

    /* pid loop */
    bool finished = false;
    while(!finished) {
        clock_t period_begin_time = clock();

        /* direction */
	pthread_mutex_lock(mutex_sensors_ptr);
	Point position = vehicle_ptr->position;
	float angle = vehicle_ptr->angle;
	pthread_mutex_unlock(mutex_sensors_ptr);

        float aimed_direction = get_aimed_direction(position, destination);
        float error_direction = compute_direction_error(angle, aimed_direction);
        total_error_direction += error_direction;

        float steer =
                kp * error_direction +
                ki * total_error_direction +
                kd * (error_direction - previous_error_direction);

        float speed = get_aimed_speed(position, destination, stop_distance, max_speed);

        /* destination change */
        float distance_to_target = compute_distance(position, destination);
        if(distance_to_target < stop_distance) {
            currently_targeted_aux_point++;
            if(currently_targeted_aux_point == circuit_ptr->n_aux_points) {
                currently_targeted_aux_point = 0;
                currently_targeted_stop_point += 1;
                if(currently_targeted_stop_point == 4) {
                    currently_targeted_stop_point = 0;
                }
            }
            destination = circuit_ptr->aux_points[currently_targeted_stop_point][currently_targeted_aux_point];
        }

        previous_error_direction = error_direction;

	pthread_mutex_lock(mutex_commands_ptr);
        vehicle_ptr->speed = speed;
        vehicle_ptr->steer = steer;
	pthread_mutex_unlock(mutex_commands_ptr);

        /* check for shutdown order */
        pthread_mutex_lock(mutex_shutdown_ptr);
        if(*shutdown_ptr) {
            finished = true;
        }
        pthread_mutex_unlock(mutex_shutdown_ptr);

        /* wait for the remaining period time */
        if(!finished) {
            wait_remaining_period(period_begin_time, pid_period);
        }
    }

    return NULL;
}

