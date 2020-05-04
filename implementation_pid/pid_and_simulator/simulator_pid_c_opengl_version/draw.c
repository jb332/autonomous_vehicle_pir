/*
 * Created by jb on 30/04/2020.
 */

#include <stdio.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif
#include "draw.h"
#include "functions.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


static void draw_line(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

static void plot_point(float x, float y) {
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

static void plot_points(float centre_x, float centre_y, float x, float y) {
    for(int i = y; i >= x; i--) {
        plot_point(centre_x+x, centre_y+i);
        plot_point(centre_x-x, centre_y+i);
        plot_point(centre_x+x, centre_y-i);
        plot_point(centre_x-x, centre_y-i);
        plot_point(centre_x+i, centre_y+x);
        plot_point(centre_x-i, centre_y+x);
        plot_point(centre_x+i, centre_y-x);
        plot_point(centre_x-i, centre_y-x);
    }
}

/* Bresenham's circle algorithm modified to draw a disk */
/* learn more : "https://fr.slideshare.net/saikrishnatanguturu/computer-graphics-ver10" */
static void draw_disk_mid_point(float centre_x, float centre_y, float radius) {
    float x, y, p;
    x = 0.0;
    y = radius;
    p = 3.0 - 2.0 * radius;
    /*
     * demonstration :
     * pi = ((xi + 1)^2 + yi^2 - r^2) + ((xi + 1)^2 + (yi - 1)^2 - r^2)
     * p0 = ((x0 + 1)^2 + y0^2 - r^2) + ((x0 + 1)^2 + (y0 - 1)^2 - r^2)
     *    = ((0 + 1)^2 + r^2 - r^2) + ((0 + 1)^2 + (r - 1)^2 - r^2)
     *    = 1 + 1 - 2r + 1 = 3 - 2r
     */
    plot_points(centre_x, centre_y, x, y);

    while(x < y) {
        if (p >= 0.0) {
            /* the pixel below is closer to the circle */
            p += 4.0 * (x - y) + 10.0;
            /*
             * demonstration :
             * p(i+1) = ((x(i+1) + 1)^2 + y(i+1)^2 - r^2) + ((x(i+1) + 1)^2 + (y(i+1) - 1)^2 - r^2)
             *        = ((xi + 2)^2 + (yi - 1)^2 - r^2) + ((xi + 2)^2 + (yi - 2)^2 - r^2)
             *        = (((xi + 1)^2 + 2(xi + 1) + 1) + (yi^2 - 2yi + 1) - r^2) + (((xi + 1)^2 + 2(xi + 1) + 1) + ((yi - 1)^2 - 2(yi - 1) + 1) - r^2)
             *        = ((xi + 1)^2 + yi^2 - r^2) + ((xi + 1)^2 + (yi - 1)^2 - r^2)    +    2(xi + 1) + 1 - 2yi + 1 + 2(xi + 1) + 1 - 2(yi - 1) + 1
             *        = pi + 2xi + 2 + 1 - 2yi + 1 + 2xi + 2 + 1 - 2yi + 2 + 1
             *        = pi + 4(xi - yi) + 10
             */
            y--;
        } else {
            /* the pixel above is closer to the circle */
            p += 4.0 * x + 6.0;
            /*
             * demonstration :
             * p(i+1) = ((x(i+1) + 1)^2 + y(i+1)^2 - r^2) + ((x(i+1) + 1)^2 + (y(i+1) - 1)^2 - r^2)
             *        = ((xi + 2)^2 + yi^2 - r^2) + ((xi + 2)^2 + (yi - 1)^2 - r^2)
             *        = (((xi + 1)^2 + 2(xi + 1) + 1) + yi^2 - r^2) + (((xi + 1)^2 + 2(xi + 1) + 1) + (yi - 1)^2 - r^2)
             *        = ((xi + 1)^2 + yi^2 - r^2) + ((xi + 1)^2 + (yi - 1)^2 - r^2)    +    2(xi + 1) + 1 + 2(xi + 1) + 1
             *        = pi + 4xi + 6
             */
        }
        x++;
        plot_points(centre_x, centre_y, x , y);
    }
}

static void draw_disk_segments(float centre_x, float centre_y, float radius) {
    int num_segments = 42; /* number of segments used to draw the circle */

    int formerPolygonMode;
    glGetIntegerv(GL_POLYGON_MODE, &formerPolygonMode);
    glPolygonMode(GL_FRONT, GL_FILL);
    glBegin(GL_POLYGON);
    for(int i = 0; i < num_segments; i++) {
        float angle = 2.0 * M_PI * (float)i / (float)num_segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex2f(x + centre_x, y + centre_y);
    }
    glEnd();
    glPolygonMode(GL_FRONT, formerPolygonMode);
}

static void draw_disk(float centre_x, float centre_y, float radius, bool segments_mode) {
    if(segments_mode) {
        draw_disk_segments(centre_x, centre_y, radius);
    } else {
        draw_disk_mid_point(centre_x, centre_y, radius);
    }
}

static void activate_antialiasing() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

static void draw(int               width,
                 int               height,
                 int               coef_point_size,
                 int               coef_rect_size,
                 Vehicle *         vehicle_ptr,
                 Circuit *         circuit_ptr,
                 Point             scale,
                 pthread_mutex_t * mutex_sensors_ptr)
{
    static int previous_width = -1;
    static int previous_height = -1;
    if(width != previous_width || height != previous_height) {
        glViewport(0.0, 0.0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        previous_width = width;
        previous_height = height;
    }

    /* set the window background color to white */
    glClearColor(255.0, 255.0, 255.0, 255.0);

    /* drawing stop points */
    glColor3d(255, 0, 0);
    float stop_point_size = (float)coef_point_size * fmin((float)width, (float)height) / 400.0;
    for(int i = 0; i < 4; i++) {
        int circ_x, circ_y;
        convert_coordinates(circuit_ptr->stop_points[i], scale, width, height, &circ_x, &circ_y);
        draw_disk(circ_x, circ_y, (float) stop_point_size, true);
    }

    /* drawing auxiliary points */
    glColor3d(0, 0, 255);
    float aux_point_size = stop_point_size / 2.0;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < circuit_ptr->n_aux_points; j++) {
            int circ_x, circ_y;
            convert_coordinates(circuit_ptr->aux_points[i][j], scale, width, height, &circ_x, &circ_y);
            draw_disk(circ_x, circ_y, (float) aux_point_size, true);
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

    pthread_mutex_lock(mutex_sensors_ptr);
    Point position = vehicle_ptr->position;
    float angle = vehicle_ptr->angle;
    pthread_mutex_unlock(mutex_sensors_ptr);

    convert_coordinates(position, scale, width, height, &rect_x, &rect_y);
    float rect_angle = (float)(angle) * 2.0 * M_PI / 360.0; /* deg to rad */

    float rect_x1 = rect_x + sin(fmod(rect_angle - rect_angle_top_right, 2.0 * M_PI)) * rect_semi_diag;
    float rect_y1 = rect_y - cos(fmod(rect_angle - rect_angle_top_right, 2.0 * M_PI)) * rect_semi_diag;
    float rect_x2 = rect_x + sin(fmod(rect_angle - rect_angle_bottom_right, 2.0 * M_PI)) * rect_semi_diag;
    float rect_y2 = rect_y - cos(fmod(rect_angle - rect_angle_bottom_right, 2.0 * M_PI)) * rect_semi_diag;
    float rect_x3 = rect_x + sin(fmod(rect_angle - rect_angle_bottom_left, 2.0 * M_PI)) * rect_semi_diag;
    float rect_y3 = rect_y - cos(fmod(rect_angle - rect_angle_bottom_left, 2.0 * M_PI)) * rect_semi_diag;
    float rect_x4 = rect_x + sin(fmod(rect_angle - rect_angle_top_left, 2.0 * M_PI)) * rect_semi_diag;
    float rect_y4 = rect_y - cos(fmod(rect_angle - rect_angle_top_left, 2.0 * M_PI)) * rect_semi_diag;
    float rect_x5 = rect_x + sin(rect_angle) * rect_semi_diag * 5.0 / 4.0;
    float rect_y5 = rect_y - cos(rect_angle) * rect_semi_diag * 5.0 / 4.0;

    glLineWidth(2.0);
    draw_line(rect_x1, rect_y1, rect_x2, rect_y2);
    draw_line(rect_x2, rect_y2, rect_x3, rect_y3);
    draw_line(rect_x3, rect_y3, rect_x4, rect_y4);
    draw_line(rect_x4, rect_y4, rect_x5, rect_y5);
    draw_line(rect_x5, rect_y5, rect_x1, rect_y1);
    glLineWidth(1.0);
}

int draw_loop(Vehicle *         vehicle_ptr,
              Circuit *         circuit_ptr,
              pthread_mutex_t * mutex_sensors_ptr,
              pthread_mutex_t * mutex_draw_event_ptr,
              pthread_cond_t *  draw_event_ptr)
{
    /* Begin Graphical Parameters */
    int width = 1200;             /* window width in pixels */
    int height = 800;             /* window height in pixels */

    Point scale = {150.0, 100.0}; /* scale (max coordinates) */

    int coef_point_size = 4;      /* objects size depends on the window size, but */
    int coef_rect_size = 5;       /* you can apply multiplier coefficients here */
    /* End Graphical Parameters */

    /* OpenGL */
    if(!glfwInit()) {
        fprintf(stderr, "Error : failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(width, height, "Autonomous Vehicle Simulator", NULL, NULL);
    if(window == NULL) {
        fprintf(stderr, "Error : failed to open a GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    /* does not work on any configuration ! */
    activate_antialiasing();

    while(!glfwWindowShouldClose(window)) {
        /* wait for draw event */
        pthread_mutex_lock(mutex_draw_event_ptr);
        pthread_cond_wait(draw_event_ptr, mutex_draw_event_ptr);
        pthread_mutex_unlock(mutex_draw_event_ptr);

        /* update width and height */
        glfwGetFramebufferSize(window, &width, &height);

        /* clear color buffer */
        glClear(GL_COLOR_BUFFER_BIT);

        draw(width, height, coef_point_size, coef_rect_size, vehicle_ptr, circuit_ptr, scale, mutex_sensors_ptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
