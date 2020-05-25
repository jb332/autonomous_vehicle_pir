/*
 * Created by jb on 26/04/2020.
 */

#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif
#include "simulation_circuit.h"
#define SIMULATION_CIRCUIT_STR_SIZE 1000

struct _SimulationCircuit {
    GObject             parent_instance;
    int                 n_aux_points;
    gboolean            clockwise;
    SimulationPoint **  stop_points;
    SimulationPoint *** aux_points;
};

G_DEFINE_TYPE(SimulationCircuit, simulation_circuit, G_TYPE_OBJECT)

/***************************************************************/
enum {
    PROP_N_AUX_PTS = 1,
    PROP_CLKWISE,
    PROP_STOP_PTS,
    PROP_AUX_PTS,
    N_PROPERTIES
};

static GParamSpec * properties[N_PROPERTIES];

static void
simulation_circuit_get_property(GObject    * object,
                                guint        prop_id,
                                GValue     * value,
                                GParamSpec * pspec)
{
    SimulationCircuit * self = (SimulationCircuit *)object;

    switch (prop_id) {
        case PROP_N_AUX_PTS:
            g_value_set_int(value, self->n_aux_points);
            break;
        case PROP_CLKWISE:
            g_value_set_boolean(value, self->clockwise);
            break;
        case PROP_STOP_PTS:
            g_value_set_pointer(value, self->stop_points);
            break;
        case PROP_AUX_PTS:
            g_value_set_pointer(value, self->aux_points);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
simulation_circuit_set_property(GObject      * object,
                                guint          prop_id,
                                const GValue * value,
                                GParamSpec   * pspec)
{
    SimulationCircuit * self = (SimulationCircuit *)object;

    switch (prop_id) {
        case PROP_N_AUX_PTS:
            self->n_aux_points = g_value_get_int(value);
            break;
        case PROP_CLKWISE:
            self->clockwise = g_value_get_boolean(value);
            break;
        case PROP_STOP_PTS:
            self->stop_points = g_value_get_pointer(value);
            break;
        case PROP_AUX_PTS:
            self->aux_points = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

/****** destructor (called when no reference points to the object) ******/
static void
simulation_circuit_finalize(GObject * object)
{
    SimulationCircuit * self = SIMULATION_CIRCUIT(object);
    for(int i=0; i<4; i++) {
        /* clearing stop points */
        g_object_unref(self->stop_points[i]);
        /* clearing auxiliary points */
        for(int j=0; j<self->n_aux_points; j++) {
            g_object_unref(self->aux_points[i][j]);
        }
        /* clearing references to auxialiary points */
        g_free(self->aux_points[i]);
    }
    G_OBJECT_CLASS (simulation_circuit_parent_class)->finalize(object);
}

/****** init functions *******/
static void
simulation_circuit_class_init (SimulationCircuitClass * klass)
{
    GObjectClass * object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = simulation_circuit_get_property;
    object_class->set_property = simulation_circuit_set_property;
    object_class->finalize = simulation_circuit_finalize;

    properties[PROP_N_AUX_PTS] = g_param_spec_int(
        "n_aux_points",
        "n_aux_points",
        "number of auxiliary points for each stop point",
        0, /* => */ G_MAXINT,
        0,
        G_PARAM_READWRITE
    );

    properties[PROP_CLKWISE] = g_param_spec_boolean(
        "clockwise",
        "clockwise",
        "tells if the vehicle should turn around the circuit clockwise",
        FALSE,
        G_PARAM_READWRITE
    );

    properties[PROP_STOP_PTS] = g_param_spec_pointer(
        "stop_points",
        "stop_points",
        "stop points array",
        G_PARAM_READWRITE
    );

    properties[PROP_AUX_PTS] = g_param_spec_pointer(
        "aux_points",
        "aux_points",
        "array of auxiliary points arrays",
        G_PARAM_READWRITE
    );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        properties
    );
}

static void
simulation_circuit_init(SimulationCircuit * self)
{}

/****** convenience constructor ******/
SimulationCircuit *
simulation_circuit_new(int n_aux_points,
                       gboolean clockwise,
                       SimulationPoint * stop_points[])
{
    SimulationPoint *** aux_points = g_malloc(4 * sizeof(SimulationPoint **));

    for(int i=0; i<4; i++) {
        aux_points[i] = g_malloc(n_aux_points * sizeof(SimulationPoint *));
    }

    int radius = 8;

    SimulationPoint * centres[4];
    for(int i=0; i<4; i++) {
        float x, y;
        simulation_point_get_coordinates(stop_points[i], &x, &y);
        centres[i] = simulation_point_new(
            x + (float)((1 - 2 * (i>1)) * radius),         /* x + radius if left,   x - radius if right */
            y + (float)((1 - 2 * (i==1 || i==2)) * radius) /* y + radius if bottom, y - radius if top */
        );
    }

    radius++; /* increased radius */
    for(int i=0; i<4; i++) {
        gboolean reversed = i % 2 == (int)(!clockwise);
        for(int j=0; j<n_aux_points; j++) {
            double angle = (double)j / (double)(n_aux_points - 1) * M_PI_2;
            float x = (float)radius * (float)cos(angle);
            float y = (float)radius * (float)sin(angle);
            if(i < 2) { /* left */
                x *= -1;
            }
            if(i == 0 || i == 3) { /* bottom */
                y *= -1;
            }
            float centre_x, centre_y;
            simulation_point_get_coordinates(centres[i], &centre_x, &centre_y);
            aux_points[
                i       * (int)clockwise +  /* normal */
                (4-1-i) * (int)(!clockwise) /* reversed */
            ][
                j                  * !reversed +
                (n_aux_points-1-j) * reversed
            ] = simulation_point_new(centre_x + x, centre_y + y);
        }
    }

    if(!clockwise) { //reverse stop_points array
        for (int i = 0; i < 4 / 2; i++) {
            SimulationPoint * aux = stop_points[i];
            stop_points[i] = stop_points[4 - 1 - i];
            stop_points[4 - 1 - i] = aux;
            /*
            SimulationPoint ** aux2 = aux_points[i];
            aux_points[i] = aux_points[4 - 1 - i];
            aux_points[4 - 1 - i] = aux2;
            */
        }
    }

    return g_object_new(SIMULATION_TYPE_CIRCUIT, "n_aux_points", n_aux_points, "clockwise", clockwise, "stop_points", stop_points, "aux_points", aux_points, NULL);
}

/****** convenience methods to access properties ******/
void
simulation_circuit_get_points_and_params(SimulationCircuit *  self,
                                         int *                n_aux_points,
                                         gboolean *           clockwise,
                                         SimulationPoint ***  stop_points,
                                         SimulationPoint **** aux_points)
{
    g_object_get(G_OBJECT(self), "n_aux_points", n_aux_points, "clockwise", clockwise, "stop_points", stop_points, "aux_points", aux_points, NULL);
}

/****** methods ******/
/* BE CAREFUL : IT WORKS ONLY FOR 3 AUXILIARY POINTS */
char *
simulation_circuit_to_string(SimulationCircuit * self)
{
		char * res = g_malloc(SIMULATION_CIRCUIT_STR_SIZE * sizeof(char));
    char * pattern = "n_aux_points = %d\nclockwise = %d\nstop_points = [\n\t%s\n\t%s\n\t%s\n\t%s\n]\naux_points = [\n\t[%s, %s, %s]\n\t[%s, %s, %s]\n\t[%s, %s, %s]\n\t[%s, %s, %s]\n]\n";

    char * str[16];
    str[0]  = simulation_point_to_string(self->stop_points[0]);
    str[1]  = simulation_point_to_string(self->stop_points[1]);
    str[2]  = simulation_point_to_string(self->stop_points[2]);
    str[3]  = simulation_point_to_string(self->stop_points[3]);
    str[4]  = simulation_point_to_string(self->aux_points[0][0]);
    str[5]  = simulation_point_to_string(self->aux_points[0][1]);
    str[6]  = simulation_point_to_string(self->aux_points[0][2]);
    str[7]  = simulation_point_to_string(self->aux_points[1][0]);
    str[8]  = simulation_point_to_string(self->aux_points[1][1]);
    str[9]  = simulation_point_to_string(self->aux_points[1][2]);
    str[10] = simulation_point_to_string(self->aux_points[2][0]);
    str[11] = simulation_point_to_string(self->aux_points[2][1]);
    str[12] = simulation_point_to_string(self->aux_points[2][2]);
    str[13] = simulation_point_to_string(self->aux_points[3][0]);
    str[14] = simulation_point_to_string(self->aux_points[3][1]);
    str[15] = simulation_point_to_string(self->aux_points[3][2]);

		g_snprintf(
        res,
        SIMULATION_CIRCUIT_STR_SIZE * sizeof(char),
        pattern,
        self->n_aux_points,
        self->clockwise,
        str[0],
        str[1],
        str[2],
        str[3],
        str[4],
        str[5],
        str[6],
        str[7],
        str[8],
        str[9],
        str[10],
        str[11],
        str[12],
        str[13],
        str[14],
        str[15]
    );

    for(int i=0; i< 16; i++) {
        g_free(str[i]);
    }

		return res;
}

