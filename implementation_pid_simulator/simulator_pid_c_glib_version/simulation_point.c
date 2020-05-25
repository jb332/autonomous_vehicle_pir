/*
 * Created by jb on 25/04/2020.
 */

#include <math.h>
#include "simulation_point.h"

struct _SimulationPoint {
    GObject parent_instance;
    float   x;
    float   y;
};

G_DEFINE_TYPE(SimulationPoint, simulation_point, G_TYPE_OBJECT)

/****** properties handling ******/
enum {
    PROP_X = 1,
    PROP_Y,
    N_PROPERTIES
};

static GParamSpec * properties[N_PROPERTIES];

static void
simulation_point_get_property(GObject    * object,
                              guint        prop_id,
                              GValue     * value,
                              GParamSpec * pspec)
{
    SimulationPoint * self = (SimulationPoint *)object;

    switch (prop_id) {
        case PROP_X:
            g_value_set_float(value, self->x);
            break;
        case PROP_Y:
            g_value_set_float(value, self->y);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
simulation_point_set_property(GObject      * object,
                              guint          prop_id,
                              const GValue * value,
                              GParamSpec   * pspec)
{
    SimulationPoint * self = (SimulationPoint *)object;

    switch (prop_id) {
        case PROP_X:
            self->x = g_value_get_float(value);
            break;
        case PROP_Y:
            self->y = g_value_get_float(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

/****** init functions ******/
static void
simulation_point_class_init (SimulationPointClass * klass)
{
    GObjectClass * object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = simulation_point_get_property;
    object_class->set_property = simulation_point_set_property;

    properties[PROP_X] = g_param_spec_float(
        "x",
        "x",
        "x coordinate",
        0.0, /* => */ G_MAXFLOAT,
        0.0,
        G_PARAM_READWRITE
    );

    properties[PROP_Y] = g_param_spec_float(
        "y",
        "y",
        "y coordinate",
        0.0, /* => */ G_MAXFLOAT,
        0.0,
        G_PARAM_READWRITE
    );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        properties
    );
}

static void
simulation_point_init(SimulationPoint * self)
{}

/****** convenience constructor ******/
SimulationPoint *
simulation_point_new(float x, float y)
{
    return g_object_new(SIMULATION_TYPE_POINT, "x", x, "y", y, NULL);
}

/****** convenience methods to access properties ******/
void
simulation_point_get_coordinates(SimulationPoint * self, float * x, float * y)
{
    g_object_get(G_OBJECT(self), "x", x, "y", y, NULL);
}

/****** methods ******/
float
simulation_point_distance_to(SimulationPoint * self, SimulationPoint * dest)
{
    return sqrt(
        pow(dest->x - self->x, 2.0) +
        pow(dest->y - self->y, 2.0)
    );
}

char *
simulation_point_to_string(SimulationPoint * self)
{
    size_t needed_size = 1 + g_snprintf(NULL, 0, "(%.2f, %.2f)", self->x, self->y);
		char * str = g_malloc(needed_size * sizeof(char));
		g_snprintf(str, needed_size * sizeof(char), "(%.2f, %.2f)", self->x, self->y);
		return str;
}

