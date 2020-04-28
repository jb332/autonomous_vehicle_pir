/*
 * Created by jb on 26/04/2020.
 */

#include "simulation_vehicle.h"

struct _SimulationVehicle {
    GObject           parent_instance;
    SimulationPoint * position;
    float             angle;
    float             speed;
    float             steer;
    GtkDrawingArea *  darea;
};

G_DEFINE_TYPE(SimulationVehicle, simulation_vehicle, G_TYPE_OBJECT)

/****** properties handling ******/
enum {
    PROP_POS = 1,
    PROP_ANG,
    PROP_SPEED,
    PROP_STEER,
    PROP_DAREA,
    N_PROPERTIES
};

static GParamSpec * properties[N_PROPERTIES];

static void
simulation_vehicle_get_property(GObject    * object,
                                guint        prop_id,
                                GValue     * value,
                                GParamSpec * pspec)
{
    SimulationVehicle * self = (SimulationVehicle *)object;

    switch (prop_id) {
        case PROP_POS:
            g_value_set_object(value, self->position);
            break;
        case PROP_ANG:
            g_value_set_float(value, self->angle);
            break;
        case PROP_SPEED:
            g_value_set_float(value, self->speed);
            break;
        case PROP_STEER:
            g_value_set_float(value, self->steer);
            break;
        case PROP_DAREA:
            g_value_set_object(value, self->darea);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
simulation_vehicle_set_property(GObject      * object,
                                guint          prop_id,
                                const GValue * value,
                                GParamSpec   * pspec)
{
    SimulationVehicle * self = (SimulationVehicle *)object;

    switch (prop_id) {
        case PROP_POS:
            if(G_IS_OBJECT(self->position)) {
                g_object_unref(self->position);
            }
            self->position = g_value_get_object(value);
            break;
        case PROP_ANG:
            self->angle = g_value_get_float(value);
            break;
        case PROP_SPEED:
            self->speed = g_value_get_float(value);
            break;
        case PROP_STEER:
            self->steer = g_value_get_float(value);
            break;
        case PROP_DAREA:
            if(G_IS_OBJECT(self->darea)) {
                g_object_unref(self->darea);
            }
            self->darea = g_value_get_object(value);
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

/****** destructor (called when no reference points to the object) ******/
static void
simulation_vehicle_finalize(GObject * object)
{
    SimulationVehicle * self = SIMULATION_VEHICLE(object);
    g_object_unref(self->position);
    G_OBJECT_CLASS (simulation_vehicle_parent_class)->finalize(object);
}
/* Note : There exists another method used in the destruction process : "dispose()"
 * It is called before each time a reference to the object is destroyed including when the last reference is destroyed
 * In that particular case, it is called before the "finalize()" method.
 * We have no need to override it here
 */

/****** init functions *******/
static void
simulation_vehicle_class_init (SimulationVehicleClass * klass)
{
    GObjectClass * object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = simulation_vehicle_get_property;
    object_class->set_property = simulation_vehicle_set_property;
    object_class->finalize = simulation_vehicle_finalize;

    properties[PROP_POS] = g_param_spec_object(
        "position",
        "position",
        "current vehicle position",
        SIMULATION_TYPE_POINT,
        G_PARAM_READWRITE
    );

    properties[PROP_ANG] = g_param_spec_float(
        "angle",
        "angle",
        "current vehicle angle (clockwise from north)",
        0.0, /* => */ G_MAXFLOAT,
        0.0,
        G_PARAM_READWRITE
    );

    properties[PROP_SPEED] = g_param_spec_float(
        "speed",
        "speed",
        "current speed command aimed by the vehicle",
        0.0, /* => */ G_MAXFLOAT,
        0.0,
        G_PARAM_READWRITE
    );

    properties[PROP_STEER] = g_param_spec_float(
        "steer",
        "steer",
        "current steer command aimed by the vehicle",
        -G_MAXFLOAT, /* => */ G_MAXFLOAT,
        0.0,
        G_PARAM_READWRITE
    );

    properties[PROP_DAREA] = g_param_spec_object(
        "darea",
        "darea",
        "drawing area, needed to trigger a draw event",
        GTK_TYPE_DRAWING_AREA,
        G_PARAM_READWRITE
    );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        properties
    );
}

static void
simulation_vehicle_init(SimulationVehicle * self)
{}

/****** convenience constructor ******/
SimulationVehicle *
simulation_vehicle_new(SimulationPoint * position, float angle)
{
    /* position object should not be freed after creation,
     * because ownership is transfered to this object
     * since we do not pass "g_object_ref(position)" as an
     * argument of "g_object_new()" here, but only "position"
     * note : "g_object_ref()" increases the reference count.
     * And, since an object is destroyed (via "finalize()") when
     * its reference count reaches 0, to destroy an object
     * it is necessary to call "g_object_unref(object)" or
     * "g_clear_object(&object)" that also clears the object reference,
     * on each object reference
     *
     * example 1 :
     *
     * SimulationPoint * a = g_object_new(SIMULATION_TYPE_POINT, "x", 2, "y", 3, NULL);
     * SimulationPoint * b = a;
     * a = NULL;
     * g_clear_object(&b); <=> g_object_unref(b); b = NULL;
     *
     *
     * example 2 :
     *
     * SimulationPoint * a = g_object_new(SIMULATION_TYPE_POINT, "x", 2, "y", 3, NULL);
     * SimulationPoint * b = g_object_ref(a);
     * g_clear_object(&a); <=> g_object_unref(a); a = NULL;
     * g_clear_object(&b); <=> g_object_unref(b); b = NULL;
     *
     */
    return g_object_new(SIMULATION_TYPE_VEHICLE, "position", position, "angle", angle, NULL);
    /* g_object_new calls in the order :
     *    - object_class->constructor() : not overriden
     *    - instance_init(), simulation_vehicle_init() here : empty
     *    - object_class->constructed() : not overriden
     * and it then set the properties passed as parameters
     */
}

/****** convenience methods to access properties ******/
void
simulation_vehicle_get_sensors(SimulationVehicle * self, SimulationPoint ** position, float * angle)
{
    g_object_get(G_OBJECT(self), "position", position, "angle", angle, NULL);
}

void
simulation_vehicle_set_sensors(SimulationVehicle * self, SimulationPoint * position, float angle)
{
    g_object_set(G_OBJECT(self), "position", position, "angle", angle, NULL);
}

void
simulation_vehicle_get_commands(SimulationVehicle * self, float * speed, float * steer)
{
    g_object_get(G_OBJECT(self), "speed", speed, "steer", steer, NULL);
}

void
simulation_vehicle_set_commands(SimulationVehicle * self, float speed, float steer)
{
    g_object_set(G_OBJECT(self), "speed", speed, "steer", steer, NULL);
}

void
simulation_vehicle_trigger_draw_event(SimulationVehicle * self)
{
    GtkDrawingArea * darea;
    g_object_get(G_OBJECT(self), "darea", &darea, NULL);
    gtk_widget_queue_draw(GTK_WIDGET(darea));
}

void
simulation_vehicle_set_drawing_area(SimulationVehicle * self, GtkDrawingArea * darea)
{
    g_object_set(G_OBJECT(self), "darea", darea, NULL);
}

/****** methods ******/
char *
simulation_vehicle_to_string(SimulationVehicle * self)
{
    char * pattern = "position = %s\nangle = %.2f\nspeed = %.2f\nsteer = %.2f\n";
    size_t needed_size = 1 + g_snprintf(NULL, 0, pattern, simulation_point_to_string(self->position), self->angle, self->speed, self->steer);
		char * str = g_malloc(needed_size * sizeof(char));
		g_snprintf(str, needed_size * sizeof(char), pattern, simulation_point_to_string(self->position), self->angle, self->speed, self->steer);
		return str;
}

