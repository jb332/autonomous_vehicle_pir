/*
 * Created by jb on 26/04/2020.
 */

#ifndef SIMULATION_WINDOW_H
#define SIMULATION_WINDOW_H

#include <glib-2.0/glib-object.h>
#include <gtk/gtk.h>
#include "simulation_point.h"
#include "simulation_vehicle.h"
#include "simulation_circuit.h"

G_BEGIN_DECLS

#define SIMULATION_TYPE_WINDOW simulation_window_get_type()
G_DECLARE_FINAL_TYPE(SimulationWindow, simulation_window, SIMULATION, WINDOW, GtkWindow)


void
simulation_window_convert_coordinates(SimulationWindow * self,
                                      SimulationPoint *  point,
                                      int *              x_pixels,
                                      int *              y_pixels,
                                      int                width,
                                      int                height);

void
simulation_window_get_vehicle_and_circuit(SimulationWindow * self, SimulationVehicle ** vehicle, SimulationCircuit ** circuit);

SimulationWindow *
simulation_window_new(int width, int height, SimulationVehicle * vehicle, SimulationPoint * scale, SimulationCircuit * circuit);


G_END_DECLS

#endif /* SIMULATION_WINDOW_H */

