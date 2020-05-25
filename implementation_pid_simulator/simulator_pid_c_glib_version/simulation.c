#include <stdio.h>
#include <glib-2.0/glib-object.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "simulation_point.h"
#include "simulation_vehicle.h"
#include "simulation_circuit.h"
#include "simulation_window.h"
#include "simulation_functions.h"

/* struct to pass parameters to the simulation loop thread */
typedef struct
{
    SimulationVehicle * vehicle;
    int simu_period;
    float rand_degrees;
    float max_speed;
    float max_steer;
} simulation_loop_params_t;

/* struct to pass parameters to the pid loop thread */
typedef struct
{
    SimulationVehicle * vehicle;
    SimulationCircuit * circuit;
} pid_loop_params_t;


void
iterate(SimulationVehicle * vehicle, int simu_period, float rand_degrees, float max_speed, float max_steer, GRand * rand_generator)
{
    SimulationPoint * position;
    float angle;
    simulation_vehicle_get_sensors(vehicle, &position, &angle);

    float speed, steer;
    simulation_vehicle_get_commands(vehicle, &speed, &steer);

    float delta_t = (float)simu_period / 1000.0;
    float delta_d = speed * (float)delta_t;
    float rand_minus_1_plus_1 = 2.0 * (float)g_rand_double(rand_generator) - 1.0;
    float rand_ratio = rand_degrees / 360.0;
    float speed_ratio = speed / max_speed;

    float new_angle = (angle + simulation_functions_limit_steer(steer, max_steer) * delta_t) * (1.0 + rand_ratio * rand_minus_1_plus_1 * speed_ratio);
    new_angle = simulation_functions_modulo_angle_deg(new_angle);
    SimulationPoint * new_position = simulation_functions_compute_new_position(position, delta_d, angle);
    /*
    float new_angle = angle + 1;
    float x_pos, y_pos;
    simulation_point_get_coordinates(position, &x_pos, &y_pos);
    SimulationPoint * new_position = simulation_point_new(x_pos, y_pos + 0.1);
    */

    simulation_vehicle_set_sensors(vehicle, new_position, new_angle);

    simulation_vehicle_trigger_draw_event(vehicle);
}

gpointer
simulation_loop(gpointer data)
{
    simulation_loop_params_t * params_ptr = (simulation_loop_params_t *)data;
    GRand * rand_generator = g_rand_new_with_seed(42);
    while(TRUE) {
        g_usleep(params_ptr->simu_period * 1000);
        iterate(params_ptr->vehicle, params_ptr->simu_period, params_ptr->rand_degrees, params_ptr->max_speed, params_ptr->max_steer, rand_generator);
    }
    return NULL;
}

void
main_simulator(SimulationVehicle * vehicle, SimulationCircuit * circuit)
{
    /* Parameters begin */
    int width = 1200; /* window width in pixels */
    int height = 800; /* window height in pixels */
    SimulationPoint * scale = simulation_point_new(150.0, 100.0); /* max size of the map in meters for both coordinates, used for scaling */
    int simu_period = 10; /* period between each iteration in ms (delta_t) */
    float rand_degrees = 1.0; /* maximum random deviation (in degrees) in the new angle calculation from steer and previous angle (angle is multiplied by : 1 + rand_degrees / 360 * random_minus1_plus1 * speed / max_speed) */
    float max_speed = 5.0; /* maximum speed value (speed <= max_speed) */
    float max_steer = 45.0; /* maximum steer value (-max_steer <= steer <= max_steer) */
    /* Parameters end */

    /* gtk initialization */
    gtk_init(0, NULL);

    /* create the window object used by the graphical library */
    simulation_window_new(width, height, vehicle, scale, circuit);

    /* create a thread for the simulation_loop() function */
    simulation_loop_params_t params = {
        vehicle,
        simu_period,
        rand_degrees,
        max_speed,
        max_steer
    };
    g_thread_new("simulation_loop_thread", (GThreadFunc)simulation_loop, (gpointer)&params);

    /* launches the graphical simulator application (start gtk loop) */
    gtk_main();
}

gpointer
main_pid_loop(gpointer * data)
{
    pid_loop_params_t * params_ptr = (pid_loop_params_t *)data;
    /* Parameters Begin */
    int pid_period = 50;
    float max_speed = 5.0;
    float stop_distance = 1.0;
    float kp = 3.0;
    float ki = 0.1;
    float kd = 10.0;
    /* Parameters End */

    int n_aux_points;
    gboolean clockwise;
    SimulationPoint ** stop_points;
    SimulationPoint *** aux_points;
    simulation_circuit_get_points_and_params(params_ptr->circuit, &n_aux_points, &clockwise, &stop_points, &aux_points);

    float previous_error_direction = 0.0;
    float total_error_direction = 0.0;

    int currently_targeted_stop_point = (int)clockwise; /* from 0 to 3 (bottom left, top left, top right, bottom right) */
    int currently_targeted_aux_point = 0; /* from 0 to (n_aux_points - 1) */
    SimulationPoint * destination = aux_points[currently_targeted_stop_point][currently_targeted_aux_point];

    /* pid loop */
    while(TRUE) {
        g_usleep(pid_period * 1000);

        SimulationPoint * position;
        float current_direction;
        simulation_vehicle_get_sensors(params_ptr->vehicle, &position, &current_direction);

        /* direction */
        float aimed_direction = simulation_functions_get_aimed_direction(position, destination);
        float error_direction = simulation_functions_compute_direction_error(current_direction, aimed_direction);
        total_error_direction += error_direction;

        float steer =
            kp * error_direction +
            ki * total_error_direction +
            kd * (error_direction - previous_error_direction);

        float speed = simulation_functions_get_aimed_speed(position, destination, stop_distance, max_speed);

        /* destination change */
        float distance_to_target = simulation_point_distance_to(position, destination);
        if(distance_to_target < stop_distance) {
            currently_targeted_aux_point++;
            if(currently_targeted_aux_point == n_aux_points) {
                currently_targeted_aux_point = 0;
                currently_targeted_stop_point += 1;
                if(currently_targeted_stop_point == 4) {
                    currently_targeted_stop_point = 0;
                }
            }
            destination = aux_points[currently_targeted_stop_point][currently_targeted_aux_point];
        }

        previous_error_direction = error_direction;

        simulation_vehicle_set_commands(params_ptr->vehicle, speed, steer);
    }

    return NULL;
}

int
main(int argc, char *argv[])
{
    gboolean clockwise;
    if(argc < 2) {
        clockwise = TRUE;
    } else {
        if(g_strcmp0(argv[1], "-r") == 0 || g_strcmp0(argv[1], "--reversed") == 0) {
            clockwise = FALSE;
        } else {
            g_fprintf(stderr, "Error : You must either provide no argument and the vehicle is gonna turn clockwise or provide one argument : \"-r\" or \"--reversed\", and in that case it will turn in the other way\n");
            exit(EXIT_FAILURE);
        }
    }
    int n_aux_points = 7;
    SimulationPoint ** stop_points = g_malloc(4 * sizeof(SimulationPoint *));
    stop_points[0] = simulation_point_new(30.0 , 20.0);
    stop_points[1] = simulation_point_new(30.0 , 80.0);
    stop_points[2] = simulation_point_new(120.0, 80.0);
    stop_points[3] = simulation_point_new(120.0, 20.0);
    SimulationCircuit * circuit = simulation_circuit_new(n_aux_points, clockwise, stop_points);

    SimulationPoint * position;
    float angle;
    if(clockwise) {
        position = simulation_point_new(30.0, 50.0);
        angle = 0.0;
    } else {
        position = simulation_point_new(75.0, 20.0);
        angle = 90.0;
    }
    SimulationVehicle * vehicle = simulation_vehicle_new(position, angle);

    pid_loop_params_t params = {
        vehicle,
        circuit
    };
    g_thread_new("pid_loop_thread", (GThreadFunc)main_pid_loop, (gpointer)&params);

    main_simulator(vehicle, circuit);

    return EXIT_SUCCESS;
}

