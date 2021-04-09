#!/usr/bin/python

######## imports ########

import random
import math
import pygame
from pygame.locals import *
import numpy as np

import cv2
import sys

######## constants ########

BLACK_COLOR= (0, 0, 0)
RED_COLOR= (255, 0, 0)
GREEN_COLOR= (0, 255, 0)
BLUE_COLOR= (0, 0, 255)
YELLOW_COLOR= (255, 255, 0)
MAGENTA_COLOR= (255, 0, 255)
CYAN_COLOR= (0, 255, 255)
WHITE_COLOR= (255, 255, 255)

MILLISECONDS_PER_SECOND= 1000

TIMER_EVENT= USEREVENT
FRAMES_PER_SECOND= 30
DT= 1.0/FRAMES_PER_SECOND

QUIT_MODE= 0
BEE_MODE= 1

SIMULATION_BOUNDS= (1280, 720)

BEE_SIZE= 2.0
BEE_COLOR= YELLOW_COLOR
BEE_CHANGE_TIME= (0.2, 0.8)
BEE_TURN_RATE= (-math.pi, math.pi)
BEE_SPEED= 150.0
BEE_INITIAL_COUNT= 1200
BEE_COUNT_RATE= 300

#Constants only for circle
NECTAR_NUM= 2000
NECTAR_CIRCLE_SIZE= 300

NECTAR_BOX_SIZE= 15
NECTAR_BOX_STRIDE= 3
NECTAR_EDGE_SIZE= 3
#define the attraction matrix
ATTRACTION_MATRIX= np.zeros((2*NECTAR_BOX_SIZE+1, 2*NECTAR_BOX_SIZE+1,2))
for y in range(ATTRACTION_MATRIX.shape[0]):
	for x in range(ATTRACTION_MATRIX.shape[1]):
		a, b= NECTAR_BOX_SIZE-x, NECTAR_BOX_SIZE-y
		if np.linalg.norm((a,b))>=NECTAR_BOX_SIZE:
			ATTRACTION_MATRIX[x, y]= (0,0)
		elif -NECTAR_EDGE_SIZE <a<NECTAR_EDGE_SIZE and -NECTAR_EDGE_SIZE<b<NECTAR_EDGE_SIZE:
			ATTRACTION_MATRIX[x, y]= (np.inf, np.inf)
		else:
			a,b= NECTAR_BOX_SIZE-x, NECTAR_BOX_SIZE-y
			ATTRACTION_MATRIX[x,y]= (a, b)

######## vector2d ########

class c_vector2d(object):
	__slots__= ("x", "y")

	def __init__(self, other= None, facing= None, magnitude= None):
		if other is not None:
			self.x= other[0]
			self.y= other[1]
		elif facing is not None or magnitude is not None:
			if facing is None:
				facing= random.uniform(0.0, 2*math.pi)
			self.x= math.cos(facing)
			self.y= math.sin(facing)
			if magnitude is not None:
				self*= magnitude
		else:
			self.x= 0
			self.y= 0

	def __len__(self):
		return 2

	def __getitem__(self, index):
		return (self.x, self.y)[index]

	def __neg__(self):
		return c_vector2d(-self.x, -self.y)

	def __add__(self, other):
		return c_vector2d((self.x+other[0], self.y+other[1]))

	def __radd__(self, other):
		return c_vector2d((other[0]+self.x, other[1]+self.y))

	def __sub__(self, other):
		return c_vector2d((self.x-other[0], self.y-other[1]))

	def __rsub__(self, other):
		return c_vector2d((other[0]-self.x, other[1]-self.y))

	def __mul__(self, other):
		return c_vector2d((self.x*other, self.y*other))

	def __rmul__(self, other):
		return c_vector2d((other*self.x, other*self.y))

	def __truediv__(self, other):
		return c_vector2d((self.x/other, self.y/other))

	def __iadd__(self, other):
		self.x+= other[0]
		self.y+= other[1]
		return self

	def __isub__(self, other):
		self.x-= other[0]
		self.y-= other[1]
		return self

	def __imul__(self, other):
		self.x*= other
		self.y*= other
		return self

	def __itruediv__(self, other):
		self.x/= other
		self.y/= other
		return self

	def magnitude(self):
		return math.sqrt(self.x*self.x+self.y*self.y)

	def normalized(self):
		return self/self.magnitude()

	def pinned(self, limit):
		if self.magnitude()>limit:
			return limit*self.normalized()
		else:
			return self

	def rotated(self, angle):
		cosine= math.cos(angle)
		sine= math.sin(angle)
		return c_vector2d((self.x*cosine-self.y*sine, self.y*cosine+self.x*sine))

######## entity ########

class c_entity(object):
	def __init__(self, size, color, linear_position, linear_velocity, angular_position, angular_velocity):
		self.size= size
		self.color= color
		self.linear_position= c_vector2d(linear_position)
		self.linear_velocity= c_vector2d(linear_velocity)
		self.angular_position= angular_position
		self.angular_velocity= angular_velocity

	def update(self):
		self.linear_position+= self.linear_velocity*DT
		while self.linear_position.x<-self.size:
			self.linear_position.x+= SIMULATION_BOUNDS[0]+2*self.size
		while self.linear_position.x>SIMULATION_BOUNDS[0]+self.size:
			self.linear_position.x-= SIMULATION_BOUNDS[0]+2*self.size
		while self.linear_position.y<-self.size:
			self.linear_position.y+= SIMULATION_BOUNDS[1]+2*self.size
		while self.linear_position.y>SIMULATION_BOUNDS[1]+self.size:
			self.linear_position.y-= SIMULATION_BOUNDS[1]+2*self.size
		self.angular_position+= self.angular_velocity*DT
		while self.angular_position<0.0:
			self.angular_position+= 2*math.pi
		while self.angular_position>=2*math.pi:
			self.angular_position-= 2*math.pi

	def draw(self, surface):
		pygame.draw.circle(surface, self.color, self.linear_position, self.size)

######## bee ########

class c_bee(c_entity):
	def __init__(self):
		position= random.uniform(0.0, SIMULATION_BOUNDS[0]+SIMULATION_BOUNDS[1]+4*BEE_SIZE)
		if position<SIMULATION_BOUNDS[0]+2*BEE_SIZE:
			linear_position= (position-BEE_SIZE, -BEE_SIZE)
		else:
			linear_position= (-BEE_SIZE, position-SIMULATION_BOUNDS[0]-2*BEE_SIZE-BEE_SIZE)
		c_entity.__init__(self, BEE_SIZE, BEE_COLOR, linear_position, c_vector2d(), random.uniform(0.0, 2*math.pi), 0)
		self.timer= 0.0
		self.nectar= False

	def update(self, vectar_map):
		pos= self.readField(vectar_map)
		# 0,0 means no nectar near by
		#if speed[0]==0 and speed[1]==0:
		if vectar_map[pos[0],pos[1]]== 0:
			if self.timer<=0.0:
				self.timer= random.uniform(*BEE_CHANGE_TIME)
				self.angular_velocity= random.uniform(*BEE_TURN_RATE)
			else:
				self.timer-= DT
			self.linear_velocity= c_vector2d(facing=self.angular_position, magnitude=BEE_SPEED)
		#inf,inf means its on the nectar
		else: #speed[0]==np.inf and speed[1]==np.inf:
			self.linear_velocity= c_vector2d()
			vectar_map[pos[0],pos[1]]=0
		#else:
		#	theta= np.arctan2(speed[1],speed[0])
		#	self.linear_velocity= c_vector2d(facing=theta, magnitude=BEE_SPEED)
		c_entity.update(self)

	def readField(self, vector_map):
		map_x= int(self.linear_position[0])
		map_y= int(self.linear_position[1])
		if map_x<0:
			map_x= 0
		if map_x>=SIMULATION_BOUNDS[0]:
			map_x= SIMULATION_BOUNDS[0]-1
		if map_y<0:
			map_y= 0
		if map_y>=SIMULATION_BOUNDS[1]:
			map_y= SIMULATION_BOUNDS[1]-1
		return [map_x, map_y]

####### nectar ########

class c_nectar(c_entity):
	def __init__(self, size, color, position):
		c_entity.__init__(self, size, color, position, c_vector2d(), random.uniform(0.0, 2*math.pi), 0)

######## swarm ########

class c_swarm(object):
	def __init__(self, see_nectar= False):
		self.desired_bee_count= BEE_INITIAL_COUNT
		self.bees= []
		self.vect_map= np.zeros([SIMULATION_BOUNDS[0], SIMULATION_BOUNDS[1],2])
		self.see_nectar= see_nectar
		self.nectar= []

	#add nectar with a field described by ATTRACTION_MATRIX
	def addNectar(self, x, y, stride):
		#only add within certain range to avoid index out of bounds
		if NECTAR_BOX_SIZE<x<SIMULATION_BOUNDS[0]-NECTAR_BOX_SIZE and NECTAR_BOX_SIZE<y<SIMULATION_BOUNDS[1]-NECTAR_BOX_SIZE:
			if self.see_nectar:
				nect= c_nectar(2, WHITE_COLOR, [x, y])
				self.nectar.append(nect)
			#use a stride to avoid adding vector field for every single nectar
			if stride%NECTAR_BOX_STRIDE==0:
				self.vect_map[
					max(x-NECTAR_BOX_SIZE, 0) : min(SIMULATION_BOUNDS[0]+1, x+NECTAR_BOX_SIZE+1), \
					max(y-NECTAR_BOX_SIZE, 0) : min(SIMULATION_BOUNDS[1]+1, y+NECTAR_BOX_SIZE+1) \
					]+= ATTRACTION_MATRIX[0:2*NECTAR_BOX_SIZE+1, 0:2*NECTAR_BOX_SIZE+1]

	#add nectar without field, faster performance
	def addNectarNoField(self, x ,y):
		if 0<=x<SIMULATION_BOUNDS[0] and 0<=y<SIMULATION_BOUNDS[1]:
			if self.see_nectar:
				nect= c_nectar(2, WHITE_COLOR, [x, y])
				self.nectar.append(nect)
			self.vect_map[
				max(x-NECTAR_EDGE_SIZE, 0): min(SIMULATION_BOUNDS[0]+1, x+NECTAR_EDGE_SIZE+1), \
				max(y-NECTAR_EDGE_SIZE,0): min(SIMULATION_BOUNDS[1]+1, y+NECTAR_EDGE_SIZE+1) \
				]= (np.inf, np.inf)
			#self.vect_map[x,y]= (np.inf, np.inf)

	# use video camera and edge detection to place nectars
	def captureNectar(self, frame):
		if frame is not None:
			flip= cv2.flip(frame, 1)
			blur= cv2.GaussianBlur(flip, (3, 3), 0)
			edges= cv2.Canny(blur, 100, 200)
			edges_thick = cv2.blur(edges, (5,5))
			#x_idx,y_idx= np.nonzero(edges)
			#print(x_idx.shape)
			#self.nectar= []
			#for i in range(len(x_idx)):
				#self.addNectar(y_idx[i], x_idx[i], i)
				#self.addNectarNoField(y_idx[i], x_idx[i])
			#set the vector map directly to the output of canny to increase performance
			self.vect_map= edges_thick.T
		else:
			self.vect_map= np.zeros([SIMULATION_BOUNDS[0], SIMULATION_BOUNDS[1]])

	def update(self, frame):
		self.captureNectar(frame)
		for i in range(BEE_COUNT_RATE//FRAMES_PER_SECOND):
			if len(self.bees)<self.desired_bee_count:
				self.bees.append(c_bee())
			elif len(self.bees)>self.desired_bee_count:
				self.bees.pop()
		for bee in self.bees:
			bee.update(self.vect_map)

	def draw(self, display):
		display.fill(BLACK_COLOR)
		if self.see_nectar:
			for nect in self.nectar:
				nect.draw(display)
		for bee in self.bees:
			bee.draw(display)

######## main ########

def initialize_pygame():
	pygame.init()
	display= pygame.display.set_mode(SIMULATION_BOUNDS)
	pygame.display.set_caption('Swarm')
	pygame.time.set_timer(TIMER_EVENT, MILLISECONDS_PER_SECOND//FRAMES_PER_SECOND)
	return display

def initialize_camera():
	for camera in range(2):
		cap= cv2.VideoCapture(camera)
		if cap is not None:
			if cap.isOpened():
				cap.set(3, SIMULATION_BOUNDS[0])
				cap.set(4, SIMULATION_BOUNDS[1])
				return cap
	print("No camera", file=sys.stderr)
	return None

def read_camera_frame(cap):
	if cap is not None:
		good, frame= cap.read()
		if good:
			return frame
		print("Bad frame", file=sys.stderr)
	return None

if __name__=='__main__':
	display= initialize_pygame()
	cap= initialize_camera()
	mode= BEE_MODE
	swarm= c_swarm(see_nectar=False)
	while mode!=QUIT_MODE:
		tick= False
		events= [pygame.event.wait()]
		events+= pygame.event.get()
		for event in events:
			if event.type==TIMER_EVENT:
				tick= True
			elif event.type==KEYDOWN and event.key==K_ESCAPE:
				mode= QUIT_MODE
			elif event.type==QUIT:
				mode= QUIT_MODE
		if tick:
			pygame.display.flip()
			swarm.update(read_camera_frame(cap))
			swarm.draw(display)
