/*
 * Created by jb on 29/04/2020.
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"


/****** common ******/
float compute_distance(Point point1, Point point2);

float compute_angle_difference(float angle1, float angle2);

void wait_remaining_period(clock_t start_time, int period_in_ms);


/****** simulation ******/
void convert_coordinates(Point coord, Point scale, int width, int height, int * x_pixels, int * y_pixels);

float limit_steer(float steer, float max_abs);

Point compute_new_position(Point former_position, float distance, float angle);


/****** pid regulator ******/
float compute_direction(Point point1, Point point2);

float get_aimed_direction(Point position, Point destination);

float get_aimed_speed(Point position, Point destination, float stop_distance, float max_speed);

float compute_direction_error(float measured_direction, float aimed_direction);


#endif /* FUNCTIONS_H */
