#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "types.h"
#include "functions.h"
#include "draw.h"
#include "pid.h"
#include "simulator.h"

int main(int argc, char * argv[]) {
    /* Begin Common Parameters */
    Point stop_points[4] = {
            {30.0,  20.0},
            {30.0,  80.0},
            {120.0, 80.0},
            {120.0, 20.0}
    };
    int n_aux_points = 7;
    /* End Common Parameters */

    /* Begin Graphical Parameters */
    int width = 1200;           /* window width in pixels */
    int height = 800;           /* window height in pixels */
    /* End Graphical Parameters */

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

    /* Mutex */
    pthread_mutex_t mutex_sensors;
    pthread_mutex_t mutex_commands;
    pthread_mutex_init(&mutex_sensors, NULL);
    pthread_mutex_init(&mutex_commands, NULL);

    /* Start PID thread */
    void * args_pid[4];
    args_pid[0] = &vehicle;
    args_pid[1] = &circuit;
    args_pid[2] = &mutex_sensors;
    args_pid[3] = &mutex_commands;
    pthread_t thread_pid;
    pthread_create(&thread_pid, NULL, pid_loop, args_pid);

    /* Start Simulator thread */
    void * args_simu[3];
    args_simu[0] = &vehicle;
    args_simu[1] = &mutex_sensors;
    args_simu[2] = &mutex_commands;
    pthread_t thread_simulator;
    pthread_create(&thread_simulator, NULL, simulator_loop, args_simu);

    /* OpenGL */
    if(!glfwInit()) {
        fprintf(stderr, "Error : failed to initialize GLFW\n");
        pthread_cancel(thread_pid);
        pthread_cancel(thread_simulator);
        pthread_mutex_destroy(&mutex_sensors);
        pthread_mutex_destroy(&mutex_commands);
        for(int i=0; i<4; i++) {
            free(circuit.aux_points[i]);
        }
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(width, height, "Autonomous Vehicle Simulator", NULL, NULL);
    if(window == NULL) {
        fprintf(stderr, "Error : failed to open a GLFW window\n");
        pthread_cancel(thread_pid);
        pthread_cancel(thread_simulator);
        pthread_mutex_destroy(&mutex_sensors);
        pthread_mutex_destroy(&mutex_commands);

        for(int i=0; i<4; i++) {
            free(circuit.aux_points[i]);
        }
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        /* update width and height */
        glfwGetFramebufferSize(window, &width, &height);

        /* clear color buffer */
        glClear(GL_COLOR_BUFFER_BIT);

        draw(width, height, &vehicle, &circuit, (Point){150.0, 100.0}, &mutex_sensors);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    pthread_cancel(thread_pid);
    pthread_cancel(thread_simulator);
    pthread_mutex_destroy(&mutex_sensors);
    pthread_mutex_destroy(&mutex_commands);
    for(int i=0; i<4; i++) {
        free(circuit.aux_points[i]);
    }
    glfwTerminate();
    return 0;
}
