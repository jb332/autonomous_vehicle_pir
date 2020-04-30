/*
 * Created by jb on 29/04/2020.
 */

#ifndef SIMULATION_TYPES_H
#define SIMULATION_TYPES_H

#include <stdbool.h>

/*****************
 ***** Point *****
 *****************/
typedef struct {
    float x;
    float y;
} Point;


/*******************
 ***** Vehicle *****
 *******************/
typedef struct {
    Point position;
    float angle;
    float speed;
    float steer;
} Vehicle;


/*******************
 ***** Circuit *****
 *******************/
typedef struct {
    int n_aux_points;
    bool clockwise;
    Point stop_points[4];
    Point * aux_points[4];
} Circuit;

/* pseudo-constructor for the circuit type */
Circuit make_circuit(int n_aux_points, bool clockwise, Point stop_points[]);


#endif /* SIMULATION_TYPES_H */
