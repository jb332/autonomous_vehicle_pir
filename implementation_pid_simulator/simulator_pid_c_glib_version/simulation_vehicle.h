/*
 * Created by jb on 26/04/2020.
 */

#ifndef SIMULATION_VEHICLE_H
#define SIMULATION_VEHICLE_H

#include <glib-2.0/glib-object.h>
#include <gtk/gtk.h>
#include "simulation_point.h"

G_BEGIN_DECLS

#define SIMULATION_TYPE_VEHICLE simulation_vehicle_get_type()
G_DECLARE_FINAL_TYPE(SimulationVehicle, simulation_vehicle, SIMULATION, VEHICLE, GObject)

SimulationVehicle *
simulation_vehicle_new(SimulationPoint * position, float angle);

void
simulation_vehicle_get_sensors(SimulationVehicle * self, SimulationPoint ** position, float * angle);

void
simulation_vehicle_set_sensors(SimulationVehicle * self, SimulationPoint * position, float angle);

void
simulation_vehicle_get_commands(SimulationVehicle * self, float * speed, float * steer);

void
simulation_vehicle_set_commands(SimulationVehicle * self, float speed, float steer);

void
simulation_vehicle_trigger_draw_event(SimulationVehicle * self);

void
simulation_vehicle_set_drawing_area(SimulationVehicle * self, GtkDrawingArea * darea);

char *
simulation_vehicle_to_string(SimulationVehicle * self);

G_END_DECLS

#endif /* SIMULATION_VEHICLE_H */

