/*
 * Created by jb on 29/04/2020.
 */

#include <stdbool.h>
#include <time.h>

#include "pid.h"
#include "types.h"
#include "functions.h"

void * pid_loop(void * args) {
    Vehicle * vehicle = ((Vehicle **)args)[0];
    Circuit * circuit = ((Circuit **)args)[1];

    /* Parameters Begin */
    int pid_period = 50;
    float max_speed = 5.0;
    float stop_distance = 1.0;
    float kp = 3.0;
    float ki = 0.1;
    float kd = 10.0;
    /* Parameters End */

    float previous_error_direction = 0.0;
    float total_error_direction = 0.0;

    int currently_targeted_stop_point = (int)(circuit->clockwise); /* from 0 to 3 (bottom left, top left, top right, bottom right) */
    int currently_targeted_aux_point = 0; /* from 0 to (n_aux_points - 1) */
    Point destination = circuit->aux_points[currently_targeted_stop_point][currently_targeted_aux_point];

    /* pid loop */
    while(true) {
        struct timespec t = {0, pid_period * 1000000};
        nanosleep(&t, NULL);

        /* direction */
        float aimed_direction = get_aimed_direction(vehicle->position, destination);
        float error_direction = compute_direction_error(vehicle->angle, aimed_direction);
        total_error_direction += error_direction;

        float steer =
                kp * error_direction +
                ki * total_error_direction +
                kd * (error_direction - previous_error_direction);

        float speed = get_aimed_speed(vehicle->position, destination, stop_distance, max_speed);

        /* destination change */
        float distance_to_target = compute_distance(vehicle->position, destination);
        if(distance_to_target < stop_distance) {
            currently_targeted_aux_point++;
            if(currently_targeted_aux_point == circuit->n_aux_points) {
                currently_targeted_aux_point = 0;
                currently_targeted_stop_point += 1;
                if(currently_targeted_stop_point == 4) {
                    currently_targeted_stop_point = 0;
                }
            }
            destination = circuit->aux_points[currently_targeted_stop_point][currently_targeted_aux_point];
        }

        previous_error_direction = error_direction;

        vehicle->speed = speed;
        vehicle->steer = steer;
    }
    return NULL;
}