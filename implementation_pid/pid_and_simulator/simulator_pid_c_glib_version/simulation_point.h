/*
 * Created by jb on 25/04/2020.
 */

#ifndef SIMULATION_POINT_H
#define SIMULATION_POINT_H

#include <glib-2.0/glib-object.h>

G_BEGIN_DECLS

#define SIMULATION_TYPE_POINT simulation_point_get_type()
G_DECLARE_FINAL_TYPE(SimulationPoint, simulation_point, SIMULATION, POINT, GObject)

SimulationPoint *
simulation_point_new(float x, float y);

void
simulation_point_get_coordinates(SimulationPoint * self, float * x, float * y);

float
simulation_point_distance_to(SimulationPoint * self, SimulationPoint * dest);

char *
simulation_point_to_string(SimulationPoint * self);

G_END_DECLS

#endif /* SIMULATION_POINT_H */

