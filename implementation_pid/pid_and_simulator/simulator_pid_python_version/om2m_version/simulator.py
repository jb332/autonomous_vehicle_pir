from gi import require_version
require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib
from threading import Thread, RLock
from time import sleep
import math
import random
import sys
import request_om2m
import http.server
from http.server import BaseHTTPRequestHandler
import json

###############
### Monitor ###
###############

class S(BaseHTTPRequestHandler):

    def __init__(self, request, client, server):
        BaseHTTPRequestHandler.__init__(self, request, client, server)
        self.vehicle = None
        self.lock = None

    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(self):
        self._set_response()
        self.wfile.write("GET request for {}".format(self.path).encode('utf-8'))

    def do_POST(self):
        # print("post")
        content_length = int(self.headers['Content-Length'])
        # print(self.headers)
        post_data = self.rfile.read(content_length)  # <--- Gets the data itself
        # print(post_data)
        if self.headers['Content-Type'].find('application/json') != -1:
            data_jason = json.loads(post_data.decode('utf-8'))
            # print(post_data.decode('utf-8'))
            # print(data_jason)
            if content_length > 100:
                cin = data_jason['m2m:sgn']['m2m:nev']['m2m:rep']['m2m:cin']
                label = cin.get('lbl', -1)
                con = cin.get('con', -1)
                # print(con)
                if "pid" in label:
                    # print("iflabel")
                    con_jason = json.loads(con)
                    speed = con_jason['speed']
                    steer = con_jason['steer']
                    with self.lock:
                        self.vehicle.speed = speed
                        self.vehicle.steer = steer
        self._set_response()
        self.wfile.write("POST request for {}".format(self.path).encode('utf-8'))




def main_monitor(port, vehicle, lock):
    print("run_monitor")
    server_address = ("", port)
    server = http.server.HTTPServer
    handler = S
    handler.vehicle = vehicle
    handler.lock = lock
    print("Serveur actif sur le port :", port)
    httpd = server(server_address, handler)
    httpd.serve_forever()

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
    def __init__(self, window_size, vehicle, scale, circuit, coefs, sensors_lock):
        super(SimuWindow, self).__init__()

        vehicle.update_draw = self.queue_draw
        self.vehicle = vehicle
        self.scale = scale
        self.circuit = circuit
        self.coefs = coefs
        self.sensors_lock = sensors_lock

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

        with self.sensors_lock:
            position = self.vehicle.position
            angle = self.vehicle.angle

        rect_x, rect_y = self.convert_coord(position, width, height)
        rect_angle = math.radians(angle)

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
    vehicle, delta_t, rand_degrees, max_speed, max_steer, sensors_lock, commands_lock, nameAE, i = args_tuple

    with sensors_lock:
        position = vehicle.position
        angle = vehicle.angle

    with commands_lock:
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

    """
    README !!!
    il faut envoyer une requête http sur un cse avec la position et l'angle du véhicule au lieu de simplement écrire dans l'objet véhicule
    ensuite, un callback qu'il faudra ajouter et lier au serveur qui sourscrit à OM2M, se chargera de mettre à jour l'objet véhicule (réutiliser le code ci-dessous du coup),
    tant pour les capteurs lorsque ce programme enverra des requêtes, que pour les commandes envoyées par le pid
    on peut aussi envisager d'ignorer les requetes reçues qui concernent les capteurs et de directement mettre à jour l'objet bien-sûr, mais ça complique inutilement
    le programme je trouve

    with sensors_lock:
        vehicle.position = new_position
        vehicle.angle = new_angle
    
    """

    #envoi requête HTTP ici
    data = '"{ \
                   \\"appID\\": \\"app_'+nameAE+'\\", \
                   \\"category\\": \\"app_value\\", \
                   \\"x\\": ' + str(new_position.x) + ', \
                   \\"y\\": ' + str(new_position.y) + ', \
                    \\"angle\\": ' + str(new_angle) + ' \
                   }" '
    url = "http://localhost:8080/~/in-cse/in-name/"+nameAE+"/DATA"
    request_om2m.createContentInstance("admin:admin", url, data, "coordonate")

    vehicle.update_draw() #appelle la fonction de dessin pour redessiner le véhicule lorsque sa position et/ou son angle ont changé


# loop that periodically adds the iterate() function to gtk queued tasks
def simu_loop(vehicle, simu_period, rand_degrees, max_speed, max_steer, sensors_lock, commands_lock, nameAE):
    random.seed(42)
    i = 1
    while True:
        sleep(simu_period)
        GLib.idle_add(iterate, (vehicle, simu_period, rand_degrees, max_speed, max_steer, sensors_lock, commands_lock, nameAE, i))
        i += 1


# simulation process
def main_simulator(vehicle, circuit, sensors_lock, commands_lock, nameAE):
    print("main_simu")
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
    SimuWindow(window_size, vehicle, scale, circuit, coefs, sensors_lock)

    # create a thread for the simu_loop() function
    Thread(target=simu_loop, args=(vehicle, simu_period, rand_degrees, max_speed, max_steer, sensors_lock, commands_lock, nameAE), daemon=True).start()
    # launches the graphical simulator application
    Gtk.main()


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

    #om2m
    port = 1800
    nameAE = "NavSensorGPS"
    Thread(target=main_monitor, args=(port, vehicle, sensors_lock), daemon=True).start()
    request_om2m.init_om2m(nameAE, port)

    main_simulator(vehicle, circuit, sensors_lock, commands_lock, nameAE)


if __name__ == "__main__":
    main()
