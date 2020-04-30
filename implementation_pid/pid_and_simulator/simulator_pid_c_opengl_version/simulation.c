#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "types.h"
#include "functions.h"
#include "draw.h"
#include "pid.h"

void simulate_move(Vehicle * vehicle, int simu_period, float rand_degrees, float max_speed, float max_steer) {
    float delta_t = (float)simu_period / 1000.0;
    float delta_d = vehicle->speed * (float)delta_t;
    float rand_minus_1_plus_1 = 2.0 * (float)rand() / (float)RAND_MAX - 1;
    float rand_ratio = rand_degrees / 360.0;
    float speed_ratio = vehicle->speed / max_speed;

    float new_angle = (vehicle->angle + limit_steer(vehicle->steer, max_steer) * delta_t) * (1.0 + rand_ratio * rand_minus_1_plus_1 * speed_ratio);
    new_angle = fmod(new_angle, 360.0);
    Point new_position = compute_new_position(vehicle->position, delta_d, vehicle->angle);

    vehicle->angle = new_angle;
    vehicle->position = new_position;
}

int main(int argc, char * argv[]) {
    srand(42);

    /* Begin Common Parameters */
    Point stop_points[4] = {
            {30.0,  20.0},
            {30.0,  80.0},
            {120.0, 80.0},
            {120.0, 20.0}
    };

    int n_aux_points = 7;
    /* End Common Parameters */

    /* Begin Simulator Parameters */
    int width = 1200;           /* window width in pixels */
    int height = 800;           /* window height in pixels */
    int simu_period = 20;       /* period between each iteration in ms (delta_t) */
    float rand_degrees = 1.0;   /* maximum random deviation (in degrees) in the new angle calculation from steer and previous angle (angle is multiplied by : 1 + rand_degrees / 360 * random_minus1_plus1 * speed / max_speed) */
    float max_speed = 5.0;      /* maximum speed value (speed <= max_speed) */
    float max_steer = 45.0;     /* maximum steer value (-max_steer <= steer <= max_steer) */
    /* End Simulator Parameters */

    bool clockwise;
    if(argc < 2) {
        clockwise = true;
    } else {
        if(strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--reversed") == 0) {
            clockwise = false;
        } else {
            fprintf(stderr, "Error : You must either provide no argument and the vehicle is gonna turn clockwise or provide one argument : \"-r\" or \"--reversed\", and in that case it will turn in the other way\n");
            exit(-1);
        }
    }

    Vehicle vehicle = {
            {
                clockwise ? 30.0 : 75.0,
                clockwise ? 50.0 : 20.0
            },
            clockwise ? 0.0 : 90.0,
            0.0,
            0.0
    };

    Circuit circuit = make_circuit(n_aux_points, clockwise, stop_points);

    /* Start PID thread */
    void * args[2];
    args[0] = (Vehicle *)&vehicle;
    args[1] = (Circuit *)&circuit;
    pthread_t thread_pid;
    pthread_create(&thread_pid, NULL, pid_loop, (void *)args);

    /* OpenGL */
    if(!glfwInit()) {
        pthread_cancel(thread_pid);
        fprintf(stderr, "Error : failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(width, height, "Autonomous Vehicle Simulator", NULL, NULL);
    if(window == NULL) {
        pthread_cancel(thread_pid);
        fprintf(stderr, "Error : failed to open a GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        clock_t start_time = clock();

        /* update width and height */
        glfwGetFramebufferSize(window, &width, &height);

        /* clear color buffer */
        glClear(GL_COLOR_BUFFER_BIT);

        /* simulation iteration */
        simulate_move(&vehicle, simu_period, rand_degrees, max_speed, max_steer);

        draw(width, height, &vehicle, &circuit, (Point){150.0, 100.0});

        glfwSwapBuffers(window);
        glfwPollEvents();

        /* wait for the remaining period time */
        clock_t stop_time = clock();
        double elapsed_time = (double)(stop_time - start_time) / CLOCKS_PER_SEC;
        double wait_time_ns = simu_period * 1000000.0 - elapsed_time * 1000000000.0;
        struct timespec t = {
            wait_time_ns / 1000000000.0,
            fmod(wait_time_ns, 1000000000.0)
        };
        nanosleep(&t, NULL);
    }

    for(int i=0; i<4; i++) {
        free(circuit.aux_points[i]);
    }
    glfwTerminate();
    pthread_cancel(thread_pid);
    return 0;
}
