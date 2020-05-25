#include <glib/gprintf.h>
#include <stdio.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif
#include "simulation_functions.h"

/****** common ******/
double
simulation_functions_modulo_angle_param(double angle, double mod) {
    if(angle >= mod) {
        do {
            angle -= mod;
        } while(angle >= mod);
    } else if(angle < 0.0) {
        do {
            angle += mod;
        } while(angle < 0.0);
    }
    return angle;
}

double
simulation_functions_modulo_angle_deg(double angle)
{
    return simulation_functions_modulo_angle_param(angle, 360.0);
}

double
simulation_functions_modulo_angle_rad(double angle)
{
    return simulation_functions_modulo_angle_param(angle, 2.0 * M_PI);
}

float
simulation_functions_compute_angle_difference(float angle1, float angle2)
{
    float delta_angle = simulation_functions_modulo_angle_deg(angle1) -
                        simulation_functions_modulo_angle_deg(angle2);

    if(delta_angle > 180.0) {
        delta_angle -= 360.0;
    } else if(delta_angle < -180.0) {
        delta_angle += 360.0;
    }
    return delta_angle;
}

/****** simulation ******/
float
simulation_functions_limit_steer(float steer, float max_abs)
{
    if(steer < -max_abs) {
        return -max_abs;
    } else if(steer > max_abs) {
        return max_abs;
    } else {
        return steer;
    }
}

SimulationPoint *
simulation_functions_compute_new_position(SimulationPoint * former_position, float distance, float angle)
{
    float x, y;
    simulation_point_get_coordinates (former_position, &x, &y);
    float delta_x = distance * sin(angle * 2.0 * M_PI / 360.0);
    float delta_y = distance * cos(angle * 2.0 * M_PI / 360.0);
    return simulation_point_new(
        x + delta_x,
        y + delta_y
    );
}

/****** pid regulator ******/
float
simulation_functions_compute_direction(SimulationPoint * point1, SimulationPoint * point2)
{
    float x1, y1, x2, y2;
    simulation_point_get_coordinates (point1, &x1, &y1);
    simulation_point_get_coordinates (point2, &x2, &y2);
    if(x1 == x2) {
        if(y1 < y2) {
            return 0.0;
        } else if(y1 > y2) {
            return 180.0;
        } else {
            g_fprintf(stderr, "Error : Could not compute direction. Provided coordinates are equal\n");
            exit(EXIT_FAILURE);
        }
    } else {
        float a = (y2 - y1) / (x2 - x1);
        float angle;
        if(x1 < x2) {
            angle = M_PI_2 - atan(a);
        } else {
            angle = 3.0 * M_PI_2 - atan(a);
        }
        return angle * 360.0 / (2.0 * M_PI);
    }
}

float
simulation_functions_get_aimed_direction(SimulationPoint * position, SimulationPoint * destination)
{
    return simulation_functions_compute_direction(position, destination);
}

float
simulation_functions_get_aimed_speed(SimulationPoint * position, SimulationPoint * destination, float stop_distance, float max_speed)
{
    float distance_to_target = simulation_point_distance_to(position, destination);
    if(distance_to_target < stop_distance) {
        float aimed_speed = distance_to_target * 0.5 - 0.5;
        if(aimed_speed < 0.0) {
            return 0.0;
        } else {
            return aimed_speed;
        }
    } else {
        return max_speed;
    }
}

float
simulation_functions_compute_direction_error(float measured_direction, float aimed_direction)
{
    return simulation_functions_compute_angle_difference (aimed_direction, measured_direction);
}
