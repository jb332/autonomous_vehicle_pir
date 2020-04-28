from gi import require_version
require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib
from threading import Thread
from time import sleep
import math
import random
import sys


##############
### Common ###
##############

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def distance_to(self, dest):
        return math.sqrt((dest.x - self.x) ** 2 + (dest.y - self.y) ** 2)

    def __repr__(self):
        return "({}, {})".format(self.x, self.y)


class Circuit:
    """
    This class represents the points of the map on which the car turns
    """

    def __init__(self, stop_points, n_aux_points, clockwise):
        radius = 8

        self.stop_points = stop_points
        self.n_aux_points = n_aux_points
        self.clockwise = clockwise

        """
        Calculate for each corner a list of n+1 points which round the edge
        The edges are in the clockwise order starting with bottom-left, they are reversed if clockwise is false
        """
        bot_l = stop_points[0]
        top_l = stop_points[1]
        top_r = stop_points[2]
        bot_r = stop_points[3]

        pi2 = math.pi / 2
        self.aux_points = [[], [], [], []]
        centres = [
            Point(bot_l.x + radius, bot_l.y + radius),
            Point(top_l.x + radius, top_l.y - radius),
            Point(top_r.x - radius, top_r.y - radius),
            Point(bot_r.x - radius, bot_r.y + radius)
        ]

        radius += 1  # increased radius
        for k in range(4):
            aux_points_index_range = range(n_aux_points)
            if k % 2 == int(not clockwise):  # bottom left and top right
                aux_points_index_range = reversed(aux_points_index_range)
            for j in aux_points_index_range:
                x = radius * math.cos((float(j) / (n_aux_points - 1)) * pi2)
                y = radius * math.sin((float(j) / (n_aux_points - 1)) * pi2)
                if k < 2:  # left
                    x *= -1
                if k == 0 or k == 3:  # bottom
                    y *= -1
                self.aux_points[k].append(
                    Point(centres[k].x + x, centres[k].y + y)
                )

        if not clockwise:
            #self.stop_points.reverse()
            self.aux_points.reverse()


class Vehicle:
    def __init__(self, position, angle):
        self.position = position
        self.angle = angle
        self.speed = 0
        self.steer = 0
        self.update_draw = None


def compute_angle_difference(angle1, angle2):
    delta_angle = angle1 % 360 - angle2 % 360
    if delta_angle > 180:
        delta_angle -= 360
    elif delta_angle < -180:
        delta_angle += 360
    return delta_angle


#################
### Simulator ###
#################

# SimuWindow inherits from Gtk.Window
class SimuWindow(Gtk.Window):
    # simulation window constructor
    def __init__(self, window_size, vehicle, scale, circuit, coefs):
        super(SimuWindow, self).__init__()

        vehicle.update_draw = self.queue_draw
        self.vehicle = vehicle
        self.scale = scale
        self.circuit = circuit
        self.coefs = coefs

        darea = Gtk.DrawingArea()
        darea.connect("draw", self.on_draw)
        self.add(darea)

        self.set_title("Autonomous Vehicle Simulator")
        width, height = window_size
        self.resize(width, height)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.connect("delete-event", Gtk.main_quit)
        self.show_all()

    # convert coordinates on the given scale to pixel coordinates on the window
    def convert_coord(self, point, width, height):
        x, y = point.x, point.y
        x_max, y_max = self.scale
        return (
            width * x / x_max,
            height * (y_max - y) / y_max
        )

    # drawing function, called on various gtk events like window resizing and during each iteration
    def on_draw(self, wid, cr):
        width, height = self.get_size()

        cr.set_source_rgb(255, 255, 255)  # draw background in white
        cr.rectangle(0, 0, width, height)
        cr.fill()

        cr.set_line_width(2)

        # drawing stop points
        stop_point_size = self.coefs["point_size"] * min(width, height) / 400
        cr.set_source_rgb(255, 0, 0)  # draw stop points in red
        for stop_point in self.circuit.stop_points:
            circ_x, circ_y = self.convert_coord(stop_point, width, height)
            cr.arc(circ_x, circ_y, stop_point_size, 0, 2 * math.pi)
            cr.fill()

        # drawing auxiliary points
        aux_point_size = stop_point_size / 2
        cr.set_source_rgb(0, 0, 255)  # draw auxiliary points in blue
        for stop_point_index in range(4):
            for aux_point in self.circuit.aux_points[stop_point_index]:
                pt_x, pt_y = self.convert_coord(aux_point, width, height)
                cr.arc(pt_x, pt_y, aux_point_size, 0, 2 * math.pi)
                cr.fill()

        # drawing the vehicle
        cr.set_source_rgb(0, 0, 0)  # back to black
        rect_width = self.coefs["rect_size"] * min(width, height) / 200
        rect_height = rect_width * 2
        rect_semi_diag = math.sqrt(rect_width * rect_width + rect_height * rect_height) / 2

        rect_angle_top_right = math.atan(rect_width / rect_height)
        rect_angle_bottom_right = math.pi - rect_angle_top_right
        rect_angle_bottom_left = -rect_angle_bottom_right
        rect_angle_top_left = -rect_angle_top_right

        rect_x, rect_y = self.convert_coord(self.vehicle.position, width, height)
        rect_angle = math.radians(self.vehicle.angle)

        rect_x1 = rect_x + math.sin((rect_angle - rect_angle_top_right) % (2 * math.pi)) * rect_semi_diag
        rect_y1 = rect_y - math.cos((rect_angle - rect_angle_top_right) % (2 * math.pi)) * rect_semi_diag
        rect_x2 = rect_x + math.sin((rect_angle - rect_angle_bottom_right) % (2 * math.pi)) * rect_semi_diag
        rect_y2 = rect_y - math.cos((rect_angle - rect_angle_bottom_right) % (2 * math.pi)) * rect_semi_diag
        rect_x3 = rect_x + math.sin((rect_angle - rect_angle_bottom_left) % (2 * math.pi)) * rect_semi_diag
        rect_y3 = rect_y - math.cos((rect_angle - rect_angle_bottom_left) % (2 * math.pi)) * rect_semi_diag
        rect_x4 = rect_x + math.sin((rect_angle - rect_angle_top_left) % (2 * math.pi)) * rect_semi_diag
        rect_y4 = rect_y - math.cos((rect_angle - rect_angle_top_left) % (2 * math.pi)) * rect_semi_diag
        rect_x5 = rect_x + math.sin(rect_angle) * rect_semi_diag * 5 / 4
        rect_y5 = rect_y - math.cos(rect_angle) * rect_semi_diag * 5 / 4

        cr.move_to(rect_x1, rect_y1)
        cr.line_to(rect_x2, rect_y2)
        cr.line_to(rect_x3, rect_y3)
        cr.line_to(rect_x4, rect_y4)
        cr.line_to(rect_x5, rect_y5)
        cr.line_to(rect_x1, rect_y1)
        cr.stroke()


def limit_steer(steer, max_abs):
    if steer < -max_abs:
        return -max_abs
    elif steer > max_abs:
        return max_abs
    else:
        return steer


# compute the new position from the former one, the distance between them and the angle
def compute_new_position(former_position, distance, angle):
    x, y = former_position.x, former_position.y
    delta_x = distance * math.sin(math.radians(angle))
    delta_y = distance * math.cos(math.radians(angle))
    return Point(
        x + delta_x,
        y + delta_y
    )


# iteration
def iterate(args_tuple):
    vehicle, delta_t, rand_degrees, max_speed, max_steer, i = args_tuple

    position = vehicle.position
    angle = vehicle.angle

    speed = vehicle.speed
    steer = vehicle.steer

    delta_d = speed * delta_t
    rand_minus_1_plus_1 = 2 * random.random() - 1
    rand_ratio = rand_degrees / 360
    speed_ratio = speed / max_speed

    new_angle = (angle + limit_steer(steer, max_steer) * delta_t) * (
            1 + rand_ratio * rand_minus_1_plus_1 * speed_ratio) % 360
    new_position = compute_new_position(position, delta_d, angle)

    """
    print(
        "\nSIMULATION\t\tangle difference = " + str(round(compute_angle_difference(new_angle, angle), 2)) + "" + "\n\n")
    """

    vehicle.position = new_position
    vehicle.angle = new_angle

    vehicle.update_draw()


# loop that periodically adds the iterate() function to gtk queued tasks
def simu_loop(vehicle, simu_period, rand_degrees, max_speed, max_steer):
    random.seed(42)
    i = 1
    while True:
        sleep(simu_period)
        GLib.idle_add(iterate, (vehicle, simu_period, rand_degrees, max_speed, max_steer, i))
        i += 1


# simulation process
def main_simulator(vehicle, circuit):
    # Parameters Begin
    window_size = (1200, 800)  # size of the window in pixels
    scale = (150, 100)  # max size of the map in meters for both coordinates, used for scaling
    coefs = {
        # coefficients used to adjust the size of graphical elements (they are automatically resized when the window is resized)
        "point_size": 4,
        "rect_size": 5
    }
    simu_period = 0.01  # period between each iteration (delta_t)
    rand_degrees = 1  # maximum random deviation (in degrees) in the new angle calculation from steer and previous angle (angle is multiplied by : 1 + rand_degrees / 360 * random_minus1_plus1 * speed / max_speed)
    max_speed = 5  # maximum speed value (speed <= max_speed)
    max_steer = 45  # maximum steer value (-max_steer <= steer <= max_steer)
    # Parameters End

    # create the window object used by the graphical library
    SimuWindow(window_size, vehicle, scale, circuit, coefs)

    # create a thread for the simu_loop() function
    Thread(target=simu_loop, args=(vehicle, simu_period, rand_degrees, max_speed, max_steer), daemon=True).start()
    # launches the graphical simulator application
    Gtk.main()


################
### PID loop ###
################

# PID coefficients
class K:
    def __init__(self, kp, ki, kd):
        self.p = kp
        self.i = ki
        self.d = kd


# compute the angle formed by two points from the north
def compute_direction(x1, y1, x2, y2):
    if x1 == x2:
        if y1 < y2:
            return 0.0
        elif y1 > y2:
            return 180.0
        else:
            raise Exception("Error : Could not compute direction. Provided coordinates are equal")
    else:
        a = (y2 - y1) / (x2 - x1)
        if x1 < x2:
            angle = math.pi / 2 - math.atan(a)
        else:
            angle = 3 * math.pi / 2 - math.atan(a)
        return math.degrees(angle)


# get aimed direction depending on the current and target positions
def get_aimed_direction(position, destination):
    return compute_direction(position.x, position.y, destination.x, destination.y)


# get aimed speed depending on the distance to target
def get_aimed_speed(position, destination, stop_distance, max_speed):
    distance_to_target = position.distance_to(destination)
    if distance_to_target < stop_distance:
        aimed_speed = distance_to_target * 0.5 - 0.5
        if aimed_speed < 0:
            return 0
        else:
            return aimed_speed
    else:
        return max_speed


# basically a difference with modulos
def compute_direction_error(measured_direction, aimed_direction):
    return compute_angle_difference(aimed_direction, measured_direction)


# pid process
def main_pid_loop(vehicle, circuit):
    # Parameters Begin
    pid_period = 0.05
    max_speed = 5
    stop_distance = 1
    k = K(3, 0.1, 10)  # steer pid coefs (kp, ki, kd)
    # Parameters End

    previous_error_direction = 0
    total_error_direction = 0

    currently_targeted_stop_point = int(circuit.clockwise)  # from 0 to 3 (bottom left, top left, top right, bottom right)
    currently_targeted_aux_point = 0  # from 0 to (n_aux_points - 1)
    destination = circuit.aux_points[currently_targeted_stop_point][currently_targeted_aux_point]
    # print(destination)

    # pid loop
    # http://www.ferdinandpiette.com/blog/2011/08/implementer-un-pid-sans-faire-de-calculs/
    while True:
        sleep(pid_period)

        current_direction = vehicle.angle
        position = vehicle.position

        # direction
        aimed_direction = get_aimed_direction(position, destination)
        error_direction = compute_direction_error(current_direction, aimed_direction)
        total_error_direction += error_direction

        steer = \
            k.p * error_direction + \
            k.i * total_error_direction + \
            k.d * (error_direction - previous_error_direction)

        speed = get_aimed_speed(position, destination, stop_distance, max_speed)

        # destination change
        distance_to_target = position.distance_to(destination)
        if distance_to_target < stop_distance:
            currently_targeted_aux_point += 1
            if currently_targeted_aux_point == circuit.n_aux_points:
                currently_targeted_aux_point = 0
                currently_targeted_stop_point += 1
                if currently_targeted_stop_point == 4:
                    currently_targeted_stop_point = 0

            destination = circuit.aux_points[currently_targeted_stop_point][currently_targeted_aux_point]
            # print(destination)

        """
        print("speed = " + str(round(speed, 2)) + "\t\t" + "kp = " + str(
            round(k.p * error_direction, 2)) + "\t\t" + "ki = " + str(
            round(k.i * total_error_direction, 2)) + "\t\t" + "kd = " + str(
            round(k.d * (error_direction - previous_error_direction), 2)) + "\t\t" + "steer = " + str(
            round(steer, 2)) + "\n")
        """

        previous_error_direction = error_direction

        vehicle.speed = speed
        vehicle.steer = steer


############
### Main ###
############

def main():
    # circuit
    if len(sys.argv) < 2:
        clockwise = True  # indicate which way the vehicle is gonna turn in the circuit
    else:
        if sys.argv[1] == "-r" or sys.argv[1] == "--reversed":
            clockwise = False
        else:
            raise Exception("You must either provide no argument and the vehicle is gonna turn clockwise or provide "
                            "one argument : \"-r\" or \"--reversed\", and in that case it will turn in the other way")

    n_aux_points = 7  # number of auxiliary points added at each stop point
    circuit = Circuit(  # coordinates of the points (autonomous vehicle stops)
        [
            Point(30, 20),
            Point(30, 80),
            Point(120, 80),
            Point(120, 20)
        ],
        n_aux_points,
        clockwise
    )

    # vehicle
    if clockwise:
        position = Point(30, 50)
        angle = 0
    else:
        position = Point(75, 20)
        angle = 90
    vehicle = Vehicle(position, angle)

    Thread(target=main_pid_loop, args=(vehicle, circuit), daemon=True).start()
    main_simulator(vehicle, circuit)


if __name__ == "__main__":
    main()
