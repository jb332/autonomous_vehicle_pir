#ifndef SIMULATION_FUNCTIONS_H
#define SIMULATION_FUNCTIONS_H
#include "simulation_point.h"

/****** common ******/
double
simulation_functions_modulo_angle_param(double angle, double mod);

double
simulation_functions_modulo_angle_deg(double angle);

double
simulation_functions_modulo_angle_rad(double angle);

float
simulation_functions_compute_angle_difference(float angle1, float angle2);


/****** simulation ******/
float
simulation_functions_limit_steer(float steer, float max_abs);

SimulationPoint *
simulation_functions_compute_new_position(SimulationPoint * former_position, float distance, float angle);


/****** pid regulator ******/
/* compute the angle formed by two points from the north */
float
simulation_functions_compute_direction(SimulationPoint * point1, SimulationPoint * point2);

/* get aimed direction depending on the current and target positions */
float
simulation_functions_get_aimed_direction(SimulationPoint * position, SimulationPoint * destination);

/* get aimed speed depending on the distance to target */
float
simulation_functions_get_aimed_speed(SimulationPoint * position, SimulationPoint * destination, float stop_distance, float max_speed);

/* basically a difference with modulos */
float
simulation_functions_compute_direction_error(float measured_direction, float aimed_direction);

#endif /* SIMULATION_FUNCTIONS_H */

