//
// Created by jb on 29/04/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif

#include "functions.h"

/****** common ******/
float compute_distance(Point point1, Point point2) {
    return sqrt(
            pow(point2.x - point1.x, 2.0) +
            pow(point2.y - point1.y, 2.0)
    );
}

float modulo_angle_param(float angle, float mod) {
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

float modulo_angle_deg(float angle) {
    return modulo_angle_param(angle, 360.0);
}

float modulo_angle_rad(float angle) {
    return modulo_angle_param(angle, 2.0 * M_PI);
}

float compute_angle_difference(float angle1, float angle2) {
    float delta_angle = modulo_angle_deg(angle1) -
                        modulo_angle_deg(angle2);

    if(delta_angle > 180.0) {
        delta_angle -= 360.0;
    } else if(delta_angle < -180.0) {
        delta_angle += 360.0;
    }
    return delta_angle;
}

/****** simulation ******/
void convert_coordinates(Point coord, Point scale, int width, int height, int * x_pixels, int * y_pixels) {
    *x_pixels = (int)((float)width * coord.x / scale.x);
    *y_pixels = (int)((float)height * (scale.y - coord.y) / scale.y);
}

float limit_steer(float steer, float max_abs) {
    if(steer < -max_abs) {
        return -max_abs;
    } else if(steer > max_abs) {
        return max_abs;
    } else {
        return steer;
    }
}

Point compute_new_position(Point former_position, float distance, float angle) {
    float angle_rad = angle * 2.0 * M_PI / 360.0;
    return (Point) {
            former_position.x + distance * sin(angle_rad),
            former_position.y + distance * cos(angle_rad)
    };
}

/****** pid regulator ******/
float compute_direction(Point point1, Point point2) {
    if(point1.x == point2.y) {
        if(point1.y < point2.y) {
            return 0.0;
        } else if(point1.y > point2.y) {
            return 180.0;
        } else {
            fprintf(stderr, "Error : Could not compute direction. Provided coordinates are equal\n");
            exit(EXIT_FAILURE);
        }
    } else {
        float a = (point2.y - point1.y) / (point2.x - point1.x);
        float angle;
        if(point1.x < point2.x) {
            angle = M_PI_2 - atan(a);
        } else {
            angle = 3.0 * M_PI_2 - atan(a);
        }
        return angle * 360.0 / (2.0 * M_PI); //rad to deg
    }
}

float get_aimed_direction(Point position, Point destination) {
    return compute_direction(position, destination);
}

float get_aimed_speed(Point position, Point destination, float stop_distance, float max_speed) {
    float distance_to_target = compute_distance(position, destination);
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

float compute_direction_error(float measured_direction, float aimed_direction) {
    return compute_angle_difference(aimed_direction, measured_direction);
}
