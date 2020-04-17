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

class Vehicle:
    def __init__(self, position, angle):
        self.position = position
        self.angle = angle
        self.speed = 0
        self.steer = 0
        self.update_draw = None


#################
### Simulator ###
#################

# SimuWindow inherits from Gtk.Window
class SimuWindow(Gtk.Window):
    # simulation window constructor
    def __init__(self, window_size, vehicle, scale, points, coefs):
        super(SimuWindow, self).__init__()

        vehicle.update_draw = self.queue_draw
        self.vehicle = vehicle
        self.scale = scale
        self.points = points
        self.coefs = coefs

        self.darea = Gtk.DrawingArea()
        self.darea.connect("draw", self.on_draw)
        self.add(self.darea)

        self.set_title("Autonomous Vehicle Simulator")
        width, height = window_size
        self.resize(width, height)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.connect("delete-event", Gtk.main_quit)
        self.show_all()

    # convert coordinates on the given scale to pixel coordinates on the window
    def convert_coord(self, position, width, height):
        x, y = position
        x_max, y_max = self.scale
        return (
            width * x / x_max,
            height * (y_max - y) / y_max
        )

    # drawing function, called on various gtk events like window resizing and during each iteration
    def on_draw(self, wid, cr):
        width, height = self.get_size()

        cr.set_source_rgb(255, 255, 255)
        cr.rectangle(0, 0, width, height)
        cr.fill()

        cr.set_source_rgb(0, 0, 0)
        cr.set_line_width(2)

        # drawing stop points
        point_size = self.coefs["point_size"] * min(width, height) / 400
        for coord in self.points:
            circ_x, circ_y = self.convert_coord(coord, width, height)
            cr.arc(circ_x, circ_y, point_size, 0, 2 * math.pi)
            cr.fill()

        # drawing the vehicle
        rect_width = self.coefs["rect_size"] * min(width, height) / 200
        rect_height = rect_width * 2
        rect_semi_diag = math.sqrt(rect_width * rect_width + rect_height * rect_height) / 2

        rect_angle_top_right = math.atan(rect_width / rect_height)
        rect_angle_bottom_right = math.pi - rect_angle_top_right
        rect_angle_bottom_left = -rect_angle_bottom_right
        rect_angle_top_left = -rect_angle_top_right

        rect_x, rect_y = self.convert_coord(self.vehicle.position, width, height)
        rect_angle = math.radians(self.vehicle.angle)

        rect_x1 = rect_x + math.sin(rect_angle - rect_angle_top_right) * rect_semi_diag
        rect_y1 = rect_y - math.cos(rect_angle - rect_angle_top_right) * rect_semi_diag
        rect_x2 = rect_x + math.sin(rect_angle - rect_angle_bottom_right) * rect_semi_diag
        rect_y2 = rect_y - math.cos(rect_angle - rect_angle_bottom_right) * rect_semi_diag
        rect_x3 = rect_x + math.sin(rect_angle - rect_angle_bottom_left) * rect_semi_diag
        rect_y3 = rect_y - math.cos(rect_angle - rect_angle_bottom_left) * rect_semi_diag
        rect_x4 = rect_x + math.sin(rect_angle - rect_angle_top_left) * rect_semi_diag
        rect_y4 = rect_y - math.cos(rect_angle - rect_angle_top_left) * rect_semi_diag
        rect_x5 = rect_x + math.sin(rect_angle) * rect_semi_diag * 5 / 4
        rect_y5 = rect_y - math.cos(rect_angle) * rect_semi_diag * 5 / 4

        cr.move_to(rect_x1, rect_y1)
        cr.line_to(rect_x2, rect_y2)
        cr.line_to(rect_x3, rect_y3)
        cr.line_to(rect_x4, rect_y4)
        cr.line_to(rect_x5, rect_y5)
        cr.line_to(rect_x1, rect_y1)
        cr.stroke()


# compute the new position from the former one, the distance between them and the angle
def compute_new_position(former_position, distance, angle):
    x, y = former_position
    delta_x = distance * math.sin(math.radians(angle))
    delta_y = distance * math.cos(math.radians(angle))
    return (
        x + delta_x,
        y + delta_y
    )


# iteration
def iterate(args_tuple):
    vehicle, delta_t, rand_portion, coef_mul_speed, i = args_tuple

    position = vehicle.position
    angle = vehicle.angle

    speed = vehicle.speed
    steer = vehicle.steer

    delta_d = speed * coef_mul_speed * delta_t
    rand_minus_1_plus_1 = 2 * random.random() - 1

    new_angle = angle + steer * delta_t * (1 + rand_portion * rand_minus_1_plus_1)
    new_position = compute_new_position(position, delta_d, angle)

    vehicle.position = new_position
    vehicle.angle = new_angle

    vehicle.update_draw()


# loop that periodically adds the iterate() function to gtk queued tasks
def simu_loop(vehicle, simu_period, rand_portion, coef_mul_speed):
    random.seed(42)
    i = 1
    while True:
        sleep(simu_period)
        GLib.idle_add(iterate, (vehicle, simu_period, rand_portion, coef_mul_speed, i))
        i += 1


# simulation process
def main_simulator(vehicle):
    # Parameters Begin
    window_size = (1200, 800)   # size of the window in pixels
    scale = (1500, 1000)        # max size of the map in meters for both coordinates, used for scaling
    points = [                  # coordinates of the points (autonomous vehicle stops)
        (300, 200),
        (300, 800),
        (1200, 200),
        (1200, 800)
    ]
    coefs = {                   # coefficients used to adjust the size of graphical elements (they are automatically resized when the window is resized)
        "point_size": 4,
        "rect_size": 5
    }
    simu_period = 0.01          # period between each iteration (delta_t)
    rand_portion = 0.1          # random percentage is the new angle calculation from steer and previous angle
    coef_mul_speed = 20         # multiplier coefficient for speed
    # Parameters End

    # create the window object used by the graphical library
    SimuWindow(window_size, vehicle, scale, points, coefs)

    # create a thread for the simu_loop() function
    Thread(target=simu_loop, args=(vehicle, simu_period, rand_portion, coef_mul_speed), daemon=True).start()
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
def get_aimed_direction(x_pos, y_pos, x_dest, y_dest):
    return compute_direction(x_pos, y_pos, x_dest, y_dest)


# calculate the distance between two points in a cartesian coordinate system
def compute_distance(x1, y1, x2, y2):
    return math.sqrt((y2 - y1) ** 2 + (x2 - x1) ** 2)


# get aimed speed depending on the distance to target
def get_aimed_speed(x_pos, y_pos, x_dest, y_dest, stop_distance, max_speed):
    distance_to_target = compute_distance(x_pos, y_pos, x_dest, y_dest)
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
    error_direction = aimed_direction - measured_direction
    if error_direction < 0:
        error_direction += 360
    elif error_direction > 360:
        error_direction -= 360

    return error_direction


# pid process
def main_pid_loop(vehicle, destination):
    # Parameters Begin
    pid_period = 0.05
    max_speed = 4
    stop_distance = 8
    k = K(1, 1, 1)  # steer pid coefs (kp, ki, kd)  TO ADJUST
    # Parameters End

    previous_error_direction = 0
    total_error_direction = 0

    # pid loop
    # http://www.ferdinandpiette.com/blog/2011/08/implementer-un-pid-sans-faire-de-calculs/
    while True:
        sleep(pid_period)

        current_direction = vehicle.angle
        x_pos, y_pos = vehicle.position
        x_dest, y_dest = destination

        # direction
        aimed_direction = get_aimed_direction(x_pos, y_pos, x_dest, y_dest)
        error_direction = compute_direction_error(current_direction, aimed_direction)
        total_error_direction += error_direction

        steer = \
            k.p * error_direction + \
            k.i * total_error_direction + \
            k.d * (error_direction - previous_error_direction)

        previous_error_direction = error_direction

        speed = get_aimed_speed(x_pos, y_pos, x_dest, y_dest, stop_distance, max_speed)

        vehicle.speed = speed
        vehicle.steer = steer

        print("speed = " + str(speed) + "\t\t" + "steer = " + str(steer) + "\n")


############
### Main ###
############

def main():

    position = (300, 200)
    angle = 0
    vehicle = Vehicle(position, angle)

    if len(sys.argv) < 2:
        destination = (1200, 800)
    else:
        destination = (
            float(sys.argv[1]),
            float(sys.argv[2])
        )

    Thread(target=main_pid_loop, args=(vehicle, destination,), daemon=True).start()
    main_simulator(vehicle)


if __name__ == "__main__":
    main()






























    # garbage

    """
    I tried to use pipes, but pipes are bad. So, for now I use multithreading. All those problems should go with OM2M HTTP requests

    commands_in, commands_out = os.pipe()
    sensors_in, sensors_out = os.pipe()
    if os.fork() == 0:
        os.close(sensors_out)
        os.close(commands_in)
        #child__pid_loop(sensors_in, commands_out)
    else:
        os.close(sensors_in)
        os.close(commands_out)
        parent__simulation_window(sensors_out, commands_in)
    """


    """
    lines = commands.readlines()
    if len(lines) == 0 or lines[-1].split("\t\t")[0] == "i":
        print("Warning : no command found in file. Applying default command i.e. speed = 0 and steer = 0")
        speed = 0.0
        steer = 0.0
    else:
        commands_lst = lines[-1].split("\t\t")
        speed = float(commands_lst[0])
        steer = float(commands_lst[1])

    print("speed = " + str(speed) + "\t\tsteer = " + str(steer))
    """