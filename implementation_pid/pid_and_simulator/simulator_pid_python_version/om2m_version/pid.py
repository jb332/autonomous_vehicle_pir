from gi import require_version
require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib
from threading import Thread, RLock
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
def main_pid_loop(vehicle, circuit, sensors_lock, commands_lock):
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

        with sensors_lock:
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

        """
        README !!!!!
        il faut envoyer une requête http sur un cse avec la vitesse et l'angle de braquage calculés au lieu de simplement écrire dans l'objet véhicule
        ensuite, un callback qu'il faudra ajouter et lier au serveur qui sourscrit à OM2M, se chargera de mettre à jour l'objet véhicule (réutiliser le code ci-dessous du coup),
        tant pour les capteurs lorsque le simulateur enverra des requêtes, que pour les commandes envoyées par ce programme
        on peut aussi envisager d'ignorer les requetes reçues qui concernent les commandes et de directement mettre à jour l'objet bien-sûr, mais ça complique inutilement
        le programme je trouve
        
        with commands_lock:
            vehicle.speed = speed
            vehicle.steer = steer
        """

        #envoi requête HTTP ici

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

    # mutex
    sensors_lock = RLock()
    commands_lock = RLock()

    main_pid_loop(vehicle, circuit, sensors_lock, commands_lock)


if __name__ == "__main__":
    main()