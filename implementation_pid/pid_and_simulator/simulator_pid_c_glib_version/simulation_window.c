/*
 * Created by jb on 26/04/2020.
 */

#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265359
    #define M_PI_2 (M_PI/2.0)
#endif
#include "simulation_window.h"
#include "simulation_functions.h"

struct _SimulationWindow {
    GtkWindow           parent_instance;
    int                 width;
    int                 height;
    SimulationVehicle * vehicle;
    SimulationPoint *   scale;
    SimulationCircuit * circuit;
};

G_DEFINE_TYPE(SimulationWindow, simulation_window, GTK_TYPE_WINDOW)

/***************************************************************/
enum {
    PROP_VEHICLE = 1,
    PROP_SCALE,
    PROP_CIRCUIT,
    N_PROPERTIES
};

static GParamSpec * properties[N_PROPERTIES];

static void
simulation_window_get_property(GObject    * object,
                               guint        prop_id,
                               GValue     * value,
                               GParamSpec * pspec)
{
    SimulationWindow * self = (SimulationWindow *)object;

    switch (prop_id) {
        case PROP_VEHICLE:
            g_value_set_object(value, self->vehicle);
            break;
        case PROP_SCALE:
            g_value_set_object(value, self->scale);
            break;
        case PROP_CIRCUIT:
            g_value_set_object(value, self->circuit);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
simulation_window_set_property(GObject      * object,
                               guint          prop_id,
                               const GValue * value,
                               GParamSpec   * pspec)
{
    SimulationWindow * self = (SimulationWindow *)object;

    switch (prop_id) {
        case PROP_VEHICLE:
            if(G_IS_OBJECT(self->vehicle)) {
                g_object_unref(self->vehicle);
            }
            self->vehicle = g_value_get_object(value);
            break;
        case PROP_SCALE:
            if(G_IS_OBJECT(self->scale)) {
                g_object_unref(self->scale);
            }
            self->scale = g_value_get_object(value);
            break;
        case PROP_CIRCUIT:
            if(G_IS_OBJECT(self->circuit)) {
                g_object_unref(self->circuit);
            }
            self->circuit = g_value_get_object(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

/****** destructor (called when no reference points to the object) ******/
static void
simulation_window_finalize(GObject * object)
{
    SimulationWindow * self = SIMULATION_WINDOW(object);
    g_object_unref(self->vehicle);
    g_object_unref(self->scale);
    g_object_unref(self->circuit);
    G_OBJECT_CLASS (simulation_window_parent_class)->finalize(object);
}

/****** init functions *******/
static void
simulation_window_class_init (SimulationWindowClass * klass)
{
    GObjectClass * object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = simulation_window_get_property;
    object_class->set_property = simulation_window_set_property;
    object_class->finalize = simulation_window_finalize;

    properties[PROP_VEHICLE] = g_param_spec_object(
        "vehicle",
        "vehicle",
        "vehicle",
        SIMULATION_TYPE_VEHICLE,
        G_PARAM_READWRITE
    );

    properties[PROP_SCALE] = g_param_spec_object(
        "scale",
        "scale",
        "max coordinates",
        SIMULATION_TYPE_POINT,
        G_PARAM_READWRITE
    );

    properties[PROP_CIRCUIT] = g_param_spec_object(
        "circuit",
        "circuit",
        "circuit",
        SIMULATION_TYPE_CIRCUIT,
        G_PARAM_READWRITE
    );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        properties
    );
}

static void
simulation_window_init(SimulationWindow * self)
{}

/****** exit handler ******/
static void
simulation_window_exit_handler(gpointer * data)
{
    SimulationWindow * self = SIMULATION_WINDOW(data);
    g_clear_object(&self);
    gtk_main_quit();
}

/****** methods ******/
void
simulation_window_convert_coordinates(SimulationWindow * self,
                                      SimulationPoint *  point,
                                      int *              x_pixels,
                                      int *              y_pixels,
                                      int                width,
                                      int                height)
{
    float x, y, x_max, y_max;
    simulation_point_get_coordinates(point, &x, &y);
    simulation_point_get_coordinates(self->scale, &x_max, &y_max);
    *x_pixels = (int)((float)width * x / x_max);
    *y_pixels = (int)((float)height * (y_max - y) / y_max);
}

/****** draw callback ******/
gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    SimulationWindow * self = SIMULATION_WINDOW(data);

    int n_aux_points;
    gboolean clockwise;
    SimulationPoint ** stop_points;
    SimulationPoint *** aux_points;
    simulation_circuit_get_points_and_params(self->circuit, &n_aux_points, &clockwise, &stop_points, &aux_points);

    SimulationPoint * position;
    float angle;
    simulation_vehicle_get_sensors(self->vehicle, &position, &angle);

    int coef_point_size = 4;
    int coef_rect_size = 5;

    int width, height;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);


    cairo_set_source_rgb(cr, 255.0, 255.0, 255.0);
    cairo_rectangle(cr, 0.0, 0.0, (double)width, (double)height);
    cairo_fill(cr);

    cairo_set_line_width(cr, 2.0);

    /* drawing stop points */
    double stop_point_size = (double)coef_point_size * fmin((double)width, (double)height) / 400.0;
    cairo_set_source_rgb(cr, 255.0, 0.0, 0.0);
    for(int i=0; i<4; i++) {
        int circ_x, circ_y;
        simulation_window_convert_coordinates(self, stop_points[i], &circ_x, &circ_y, width, height);
        cairo_arc(cr, (double)circ_x, (double)circ_y, stop_point_size, 0.0, 2.0 * M_PI);
        cairo_fill(cr);
    }

    /* drawing auxiliary points */
    double aux_points_size = stop_point_size / 2.0;
    cairo_set_source_rgb(cr, 0.0, 0.0, 255.0);
    for(int i=0; i<4; i++) {
        for(int j=0; j<n_aux_points; j++) {
            int circ_x, circ_y;
            simulation_window_convert_coordinates(self, aux_points[i][j], &circ_x, &circ_y, width, height);
            cairo_arc(cr, (double)circ_x, (double)circ_y, aux_points_size, 0.0, 2.0 * M_PI);
            cairo_fill(cr);
        }
    }

    /* drawing the vehicle */
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    double rect_width = (double)(coef_rect_size * fmin(width, height) / 200);
    double rect_height = rect_width * 2.0;
    double rect_semi_diag = sqrt(pow(rect_width, 2.0) + pow(rect_height, 2.0)) / 2.0;

    double rect_angle_top_right = atan(rect_width / rect_height);
    double rect_angle_bottom_right = M_PI - rect_angle_top_right;
    double rect_angle_bottom_left = -rect_angle_bottom_right;
    double rect_angle_top_left = -rect_angle_top_right;

    int rect_x, rect_y;
    simulation_window_convert_coordinates(self, position, &rect_x, &rect_y, width, height);
    double rect_angle = (double)angle * 2.0 * M_PI / 360.0; /* deg to rad */

    double rect_x1 = rect_x + sin(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_top_right)) * rect_semi_diag;
    double rect_y1 = rect_y - cos(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_top_right)) * rect_semi_diag;
    double rect_x2 = rect_x + sin(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_bottom_right)) * rect_semi_diag;
    double rect_y2 = rect_y - cos(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_bottom_right)) * rect_semi_diag;
    double rect_x3 = rect_x + sin(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_bottom_left)) * rect_semi_diag;
    double rect_y3 = rect_y - cos(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_bottom_left)) * rect_semi_diag;
    double rect_x4 = rect_x + sin(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_top_left)) * rect_semi_diag;
    double rect_y4 = rect_y - cos(simulation_functions_modulo_angle_rad(rect_angle - rect_angle_top_left)) * rect_semi_diag;
    double rect_x5 = rect_x + sin(rect_angle) * rect_semi_diag * 5.0 / 4.0;
    double rect_y5 = rect_y - cos(rect_angle) * rect_semi_diag * 5.0 / 4.0;

    cairo_move_to(cr, rect_x1, rect_y1);
    cairo_line_to(cr, rect_x2, rect_y2);
    cairo_line_to(cr, rect_x3, rect_y3);
    cairo_line_to(cr, rect_x4, rect_y4);
    cairo_line_to(cr, rect_x5, rect_y5);
    cairo_line_to(cr, rect_x1, rect_y1);
    cairo_stroke(cr);

    return TRUE; /* event ok */
}

/****** convenience methods to access properties ******/
void
simulation_window_get_vehicle_and_circuit(SimulationWindow * self, SimulationVehicle ** vehicle, SimulationCircuit ** circuit)
{
    g_object_get(self, "vehicle", vehicle, "circuit", circuit, NULL);
}

/****** convenience constructor ******/
SimulationWindow *
simulation_window_new(int width, int height, SimulationVehicle * vehicle, SimulationPoint * scale, SimulationCircuit * circuit)
{
    SimulationWindow * self = g_object_new(SIMULATION_TYPE_WINDOW, "vehicle", vehicle, "scale", scale, "circuit", circuit, NULL);
    gtk_window_set_title(GTK_WINDOW(self), "Autonomous Vehicle Simulator");
    gtk_window_resize(GTK_WINDOW(self), width, height);
    gtk_window_set_position(GTK_WINDOW(self), GTK_WIN_POS_CENTER);
    g_signal_connect(G_OBJECT(self), "delete-event", G_CALLBACK(simulation_window_exit_handler), (gpointer)self);
    GtkDrawingArea * darea = GTK_DRAWING_AREA(gtk_drawing_area_new());
    simulation_vehicle_set_drawing_area(vehicle, darea);
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(draw_callback), self);
    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(darea));
    gtk_widget_show_all(GTK_WIDGET(self));
    return self;
}
