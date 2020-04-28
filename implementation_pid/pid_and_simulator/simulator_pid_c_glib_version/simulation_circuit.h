/*
 * Created by jb on 26/04/2020.
 */

#ifndef SIMULATION_CIRCUIT_H
#define SIMULATION_CIRCUIT_H

#include <glib-2.0/glib-object.h>
#include "simulation_point.h"

G_BEGIN_DECLS

#define SIMULATION_TYPE_CIRCUIT simulation_circuit_get_type()
G_DECLARE_FINAL_TYPE(SimulationCircuit, simulation_circuit, SIMULATION, CIRCUIT, GObject)

SimulationCircuit *
simulation_circuit_new(int n_aux_points,
                       gboolean clockwise,
                       SimulationPoint * stop_points[]);

void
simulation_circuit_get_points_and_params(SimulationCircuit *  self,
                                         int *                n_aux_points,
                                         gboolean *           clockwise,
                                         SimulationPoint ***  stop_points,
                                         SimulationPoint **** aux_points);

char *
simulation_circuit_to_string(SimulationCircuit * self);

G_END_DECLS

#endif /* SIMULATION_CIRCUIT_H */

