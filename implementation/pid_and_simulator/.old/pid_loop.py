#!/usr/bin/env python
#import rospy
#from robucity.msg import car_command
import sys, select, termios, tty
import math
from time import sleep


#########################
### ROS communication ###
#########################

#Used to send commands to the vehicle
#pub = rospy.Publisher('/robucity/car_command', car_command, queue_size = 1)

#Used to retrieve sensor data like coordinates or speed
#rospy.Subscriber("/robucity/status", vehicle_status, treatSensorData)

#r = rospy.Rate(100)


#####################
### Parameters to ###
### set manually  ###
#####################

maxSpeed = 4 #meters per second
stopDistance = 8 #meters

speedKp = 1 # TO ADJUST
speedKi = 1
speedKd = 1

steerKp = 1
steerKi = 1
steerKd = 1


###################
### Sensor data ###
###################

latPos, lonPos = (0, 0) # from -180 to 180
currentSpeed = 0
currentDirection = 0 # idem, equivalent to full North direction


########################
### Functions to get ###
###    aimed speed   ###
########################

#computes the distance bewteen to points on earth
#Because earth is not flat
def haversineFormulaRad(lat1, lon1, lat2, lon2):
	R = 6371e3 # earth radius
	dLat = lat2 - lat1
	dLon = lon2 - lon1
	a = math.sin(dLat/2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dLon/2)**2
	c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
	return R * c


def haversineFormula(lat1, lon1, lat2, lon2):
	return math.degrees(haversineFormulaRad(math.radians(lat1), math.radians(lon1), math.radians(lat2), math.radians(lon2)))

def getAimedSpeed(latPos, lonPos, latDest, lonDest):
	distanceToTarget = haversineFormula(latPos, lonPos, latDest, lonDest)
	if distanceToTarget < stopDistance:
		aimedSpeed = distanceToTarget * 0.5 - 0.5
		if aimedSpeed < 0:
			return 0
		else:
			return aimedSpeed
	else:
		return maxSpeed


#######################
###  Functions to   ###
###    get aimed    ###
### direction angle ###
#######################

#To understand bearing angle : https://fr.wikipedia.org/wiki/Relevement
#Just in case we decide to participate in the Paris-Dakar
def computeBearingRad(lat1, lon1, lat2, lon2):
	dLon = lon2-lon1
	x = math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(dLon)
	y = math.sin(dLon) * math.cos(lat2)
	return math.atan2(y, x)

def computeBearing(lat1, lon1, lat2, lon2):
	return math.degrees(computeBearingRad(math.radians(lat1), math.radians(lon1), math.radians(lat2), math.radians(lon2)))

def getAimedDirectionAngle(latPos, lonPos, latDest, lonDest):
	return computeBearing(latPos, lonPos, latDest, lonDest)


###########################
### Function to command ###
###     the shuttle     ###
###########################

def sendInstructions(status, speed, steer, pub):
	twist = car_command()
	twist.enable = status
	twist.target_speed = speed
	twist.target_steer = steer
	#pub.publish(twist)

def printInstructions(speed, steer):
	print(speed)
	print(steer)

############################
### Callback function to ###
###  update sensor data  ###
############################

#en supposant que data est un dictionnaire tout gentil, ce qu'il n'est bien-sur pas
#c'est juste que j'ai pas le format donc voila quoi ^^
def treatSensorData(data):
	global speed, steer, x, y, currentDirection
	speed = data["speed"] # la vitesse du vehicule
	latPos, lonPos = data["position"] # les coordonnées du vehicule (latitude et longitude)
	currentDirection = data["bearing"] # supposons que le GPS est tres gentil et qu'il renvoie le relevement ou bearing/hearing angle c'est-a-dire un angle en degrés entre -180 et 180° indiquant la direction qu'a prise le vehicule avec le Nord comme degré 0 et dans le sens indirect (la tu peux utiliser ce mot Denis)
# PS : meme Google est contre toi (clockwise) : https://developers.google.com/maps/documentation/javascript/reference/geometry?hl=fr#spherical.computeHeading


###############################
### Arguments : destination ###
###############################

#print(calculateBearing(float(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4])))

latDest = float(sys.argv[1])
lonDest = float(sys.argv[2])


####################
### Control loop ###
####################

def computeDirectionError(measuredDirection, aimedDirection):
	errorDirection = aimedDirection - measuredDirection
	if errorDirection < -180:
		errorDirection += 360
	elif errorDirection > 180:
		errorDirection -= 360
	
	return errorDirection


previousErrorDirection = 0
totalErrorDirection = 0

previousSpeed = 0
previousErrorSpeed = 0
totalErrorSpeed = 0

#boucle d'asservissement avec PID
#http://www.ferdinandpiette.com/blog/2011/08/implementer-un-pid-sans-faire-de-calculs/
while True:
	#r.sleep()
	sleep(0.05)

	#direction
	aimedDirection = getAimedDirectionAngle(latPos, lonPos, latDest, lonDest)
	measuredDirection = currentDirection
	errorDirection = computeDirectionError(measuredDirection, aimedDirection) #basically a difference with modulos
	totalErrorDirection += errorDirection

	newSteer = steerKp * errorDirection + steerKi * totalErrorDirection + steerKd * (errorDirection - previousErrorDirection)
	
	previousErrorDirection = errorDirection


	#speed
	aimedSpeed = getAimedSpeed(latPos, lonPos, latDest, lonDest)
	measuredSpeed = currentSpeed
	errorSpeed = aimedSpeed - measuredSpeed
	totalErrorSpeed += errorSpeed

	#IMPORTANT : added previous value to pid since it is a speed, not an acceleration
	newSpeed = previousSpeed + speedKp * errorSpeed + speedKi * totalErrorSpeed + speedKd * (errorSpeed - previousErrorSpeed)

	previousSpeed = newSpeed
	previousErrorSpeed = errorSpeed


	#sendInstructions(1, newSpeed, newSteer, pub)
	printInstructions(newSpeed, newSteer)
