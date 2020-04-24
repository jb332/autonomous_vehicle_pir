import json
import simulation
import math



class Point:

	def __init__(self, x, y):
		self.x = x
		self.y = y

	def dist(dest):
		return math.sqrt((self.x - dest.x)**2+(self.y - dest.y)**2)
	def __repr__(self):
		return("({}, {})".format(self.x, self.y))


class Circuit:

	"""
	This class represents the map on which the car turn
	"""

	def __init__(self):
		self.botL = Point(0, 0)
		self.botR = Point(300, 0)
		self.topL = Point(0, 20)
		self.topR = Point(300, 20)

	def generateEdge(self, n):

		"""
		Return for each corner a list of n+1 points which round the edge
		The edges are in the order : bottom-left, bottom-right, top-left, top-right

		"""
		rayon = 5
		pi2 = math.pi / 2 
		resultat = [[], [], [], []]
		centres= [Point(self.botL.x + rayon, self.botL.y + rayon), Point(self.botR.x - rayon, self.botR.y + rayon), Point(self.topL.x + rayon, self.topL.y - rayon), Point(self.topR.x - rayon, self.topR.y - rayon )]
		rayon = 6
		for j in range(n+1):
			x = rayon * math.cos((float(j)/n) * pi2)
			y = rayon * math.sin((float(j)/n)*pi2)
			for k in range(4):
				ysign = -1 if k < 2 else 1
				resultat[k].append(Point(centres[k].x + x*(-1)**(k+1), centres[k].y +y*ysign))
		return resultat





stations = [Point(280, 0), Point(155, 0), Point(30, 0), Point(60, 20)]



def getPos():
	return Point(0, 0)


def nextStop():

	return {
		"station" : 0 ,
		"nb" : 2
	}

def nextMove():
	currentPos = getPos()
	dest = stations[nextStop().station]


	


