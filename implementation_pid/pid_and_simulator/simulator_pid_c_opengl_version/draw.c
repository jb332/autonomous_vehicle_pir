//
// Created by jb on 30/04/2020.
//

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif
#include "draw.h"
#include "functions.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void draw_line(float x1, float y1, float x2, float y2, int width) {
    GLfloat lineVertices[] = {
            x1, y1, 0.0,
            x2, y2, 0.0
    };
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glPushAttrib(GL_LINE_BIT);
    glLineWidth(width);
    glLineStipple(1, 0x00FF);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT,0, lineVertices);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopAttrib();

    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

void draw_disk(float centre_x, float centre_y, float radius) {
    int num_segments = 42; // number of segments used to draw the circle

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glBegin(GL_POLYGON);
    for(int i = 0; i < num_segments; i++)
    {
        float angle = 2.0 * M_PI * (float)i / (float)num_segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex2f(x + centre_x, y + centre_y);
    }
    glEnd();

    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);
}

void draw(int width, int height, Vehicle * vehicle, Circuit * circuit, Point scale) {
    glViewport(0.0f, 0.0f, width, height); // specifies the part of the window to which OpenGL will draw (in pixels), convert from normalised to pixels
    glMatrixMode(GL_PROJECTION); // projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
    glLoadIdentity(); // replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrpho and glRotate cumulate, basically puts us at (0, 0, 0)

    /*
    GLfloat matrix[16] = {
            1.0,    0.0,    0.0,    0.0,
            0.0,    1.0,    0.0,    0.0,
            0.0,    0.0,    1.0,    0.0,
            0.0,    0.0,    0.0,    1.0
    };
    */
    glOrtho(0, width, height, 0, 0, 1); // essentially set coordinate system
    //glOrtho(0, width, height, 0, 0, 1); // essentially set coordinate system
    glMatrixMode(GL_MODELVIEW); // (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world

    glLoadIdentity();
    //glLoadMatrixf(matrix);

    int coef_point_size = 4;
    int coef_rect_size = 5;

    //set the window background color to white
    glClearColor(255.0, 255.0, 255.0, 255.0);

    /* drawing stop points */
    glColor3d(255, 0, 0);
    float stop_point_size = (float)coef_point_size * fmin((float)width, (float)height) / 400.0;
    for(int i=0; i<4; i++) {
        int circ_x, circ_y;
        convert_coordinates(circuit->stop_points[i], scale, width, height, &circ_x, &circ_y);
        draw_disk(circ_x, circ_y, (float)stop_point_size);
    }

    /* drawing auxiliary points */
    glColor3d(0, 0, 255);
    float aux_point_size = stop_point_size / 2.0;
    for(int i=0; i<4; i++) {
        for(int j=0; j<circuit->n_aux_points; j++) {
            int circ_x, circ_y;
            convert_coordinates(circuit->aux_points[i][j], scale, width, height, &circ_x, &circ_y);
            draw_disk(circ_x, circ_y, (float)aux_point_size);
        }
    }

    /* drawing the vehicle */
    glColor3d(0, 0, 0);
    float rect_width = (float)(coef_rect_size * fmin(width, height) / 200);
    float rect_height = rect_width * 2.0;
    float rect_semi_diag = sqrt(pow(rect_width, 2.0) + pow(rect_height, 2.0)) / 2.0;

    float rect_angle_top_right = atan(rect_width / rect_height);
    float rect_angle_bottom_right = M_PI - rect_angle_top_right;
    float rect_angle_bottom_left = -rect_angle_bottom_right;
    float rect_angle_top_left = -rect_angle_top_right;

    int rect_x, rect_y;
    convert_coordinates(vehicle->position, scale, width, height, &rect_x, &rect_y);
    float rect_angle = (float)(vehicle->angle) * 2.0 * M_PI / 360.0; /* deg to rad */

    float rect_x1 = rect_x + sin(modulo_angle_rad(rect_angle - rect_angle_top_right)) * rect_semi_diag;
    float rect_y1 = rect_y - cos(modulo_angle_rad(rect_angle - rect_angle_top_right)) * rect_semi_diag;
    float rect_x2 = rect_x + sin(modulo_angle_rad(rect_angle - rect_angle_bottom_right)) * rect_semi_diag;
    float rect_y2 = rect_y - cos(modulo_angle_rad(rect_angle - rect_angle_bottom_right)) * rect_semi_diag;
    float rect_x3 = rect_x + sin(modulo_angle_rad(rect_angle - rect_angle_bottom_left)) * rect_semi_diag;
    float rect_y3 = rect_y - cos(modulo_angle_rad(rect_angle - rect_angle_bottom_left)) * rect_semi_diag;
    float rect_x4 = rect_x + sin(modulo_angle_rad(rect_angle - rect_angle_top_left)) * rect_semi_diag;
    float rect_y4 = rect_y - cos(modulo_angle_rad(rect_angle - rect_angle_top_left)) * rect_semi_diag;
    float rect_x5 = rect_x + sin(rect_angle) * rect_semi_diag * 5.0 / 4.0;
    float rect_y5 = rect_y - cos(rect_angle) * rect_semi_diag * 5.0 / 4.0;

    draw_line(rect_x1, rect_y1, rect_x2, rect_y2, 2);
    draw_line(rect_x2, rect_y2, rect_x3, rect_y3, 2);
    draw_line(rect_x3, rect_y3, rect_x4, rect_y4, 2);
    draw_line(rect_x4, rect_y4, rect_x5, rect_y5, 2);
    draw_line(rect_x5, rect_y5, rect_x1, rect_y1, 2);
}