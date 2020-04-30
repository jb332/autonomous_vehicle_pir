//
// Created by jb on 30/04/2020.
//

#ifndef OPENGL_DRAW_H
#define OPENGL_DRAW_H

#include "types.h"

void draw_line(float x1, float y1, float x2, float y2, int width);

void draw_circle(float centre_x, float centre_y, float radius);

void draw(int width, int height, Vehicle * vehicle, Circuit * circuit, Point scale);

#endif //OPENGL_DRAW_H
