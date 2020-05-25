/*
 * Created by jb on 29/04/2020.
 */

#include <stdlib.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif

#include "types.h"


Circuit make_circuit(int n_aux_points, bool clockwise, Point stop_points[]) {
    Circuit circuit;
    circuit.n_aux_points = n_aux_points;
    circuit.clockwise = clockwise;

    for(int i = 0; i < 4; i++) {
        circuit.stop_points[i] = stop_points[
                i           * (int)clockwise +
                (4 - 1 - i) * (int)(!clockwise)
        ];
    }

    for(int i = 0; i < 4; i++) {
        circuit.aux_points[i] = malloc(n_aux_points * sizeof(Point));
    }

    int radius = 8;

    Point centres[4];
    for(int i = 0; i < 4; i++) {
        centres[i] = (Point) {
                stop_points[i].x + (float) ((1 - 2 * (i > 1)) * radius),           /* x + radius if left,   x - radius if right */
                stop_points[i].y + (float) ((1 - 2 * (i == 1 || i == 2)) * radius) /* y + radius if bottom, y - radius if top */
        };
    }

    radius++; /* increased radius */
    for(int i = 0; i < 4; i++) {
        bool reversed = i % 2 == (int)(!clockwise);
        for(int j=0; j<n_aux_points; j++) {
            float angle = (float)j / (float)(n_aux_points - 1) * M_PI_2;
            float x = (float)radius * (float)cos(angle);
            float y = (float)radius * (float)sin(angle);
            if(i < 2) { /* left */
                x *= -1;
            }
            if(i == 0 || i == 3) { /* bottom */
                y *= -1;
            }
            circuit.aux_points[
                    i       * (int)clockwise +  /* normal */
                    (4-1-i) * (int)(!clockwise) /* reversed */
            ][
                    j                  * !reversed +
                    (n_aux_points-1-j) * reversed
            ] = (Point) {
                    centres[i].x + x,
                    centres[i].y + y
            };
        }
    }

    return circuit;
}

void free_circuit(Circuit * circuit_ptr) {
    for(int i = 0; i < 4; i++) {
        free(circuit_ptr->aux_points[i]);
    }
}
