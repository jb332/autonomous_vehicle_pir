#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "types.h"
#include "draw.h"
#include "pid.h"
#include "simulator.h"


int main(int argc, char * argv[]) {
    /* Begin Common Parameters */
    int x_left   = 30.0;
    int x_right  = 120.0;
    int y_bottom = 20.0;
    int y_top    = 80.0;

    int n_aux_points = 7;
    /* End Common Parameters */

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

    Point stop_points[4] = {
            {x_left,  y_bottom},
            {x_left,  y_top},
            {x_right, y_top},
            {x_right, y_bottom}
    };

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
    pthread_mutex_t mutex_draw_event;
    pthread_mutex_t mutex_shutdown;
    pthread_mutex_init(&mutex_sensors, NULL);
    pthread_mutex_init(&mutex_commands, NULL);
    pthread_mutex_init(&mutex_draw_event, NULL);
    pthread_mutex_init(&mutex_shutdown, NULL);

    /* Conditions */
    pthread_cond_t draw_event;
    pthread_cond_init(&draw_event, NULL);

    /* Shutdown boolean */
    bool shutdown = false;

    /* Start PID loop (thread 1) */
    void * args_pid[6];
    args_pid[0] = &vehicle;
    args_pid[1] = &circuit;
    args_pid[2] = &mutex_sensors;
    args_pid[3] = &mutex_commands;
    args_pid[4] = &mutex_shutdown;
    args_pid[5] = &shutdown;
    pthread_t thread_pid;
    pthread_create(&thread_pid, NULL, pid_loop, args_pid);

    /* Start Simulator loop (thread 2) */
    void * args_simu[7];
    args_simu[0] = &vehicle;
    args_simu[1] = &mutex_sensors;
    args_simu[2] = &mutex_commands;
    args_simu[3] = &mutex_draw_event;
    args_simu[4] = &mutex_shutdown;
    args_simu[5] = &draw_event;
    args_simu[6] = &shutdown;
    pthread_t thread_simulator;
    pthread_create(&thread_simulator, NULL, simulator_loop, args_simu);

    /* Start Draw loop (main thread) */
    int status = draw_loop(&vehicle, &circuit, &mutex_sensors, &mutex_draw_event, &draw_event);

    /* Shutdown */
    pthread_mutex_lock(&mutex_shutdown);
    shutdown = true;
    pthread_mutex_unlock(&mutex_shutdown);
    pthread_join(thread_pid, NULL);
    pthread_join(thread_simulator, NULL);

    pthread_mutex_destroy(&mutex_sensors);
    pthread_mutex_destroy(&mutex_commands);
    pthread_mutex_destroy(&mutex_draw_event);
    pthread_mutex_destroy(&mutex_shutdown);
    pthread_cond_destroy(&draw_event);
    for(int i=0; i<4; i++) {
        free(circuit.aux_points[i]);
    }

    return status;
}
