#!/usr/bin/python

######## imports ########

import random
import math
import pygame
from pygame.locals import *

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
FRAMES_PER_SECOND= 60
DT= 1.0/FRAMES_PER_SECOND

QUIT_MODE= 0
BEE_MODE= 1

SIMULATION_BOUNDS= (720, 1280)

BEE_SIZE= 2.0
BEE_COLOR= YELLOW_COLOR
BEE_TURN= 2*math.pi
BEE_STOP_TIME= (0.1, 0.4)
BEE_MOVE_TIME= (0.2, 0.8)
BEE_SPEED= 120.0
BEE_INITIAL_COUNT= 1000

######## vector2d ########

class c_vector2d(object):
	__slots__= ("x", "y")
	
	def __init__(self, other= None, facing= None, magnitude= None):
		if other!=None:
			self.x= other[0]
			self.y= other[1]
		elif facing!=None or magnitude!=None:
			if facing==None:
				facing= random.uniform(0.0, 2*math.pi)
			self.x= math.cos(facing)
			self.y= math.sin(facing)
			if magnitude!=None:
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
	
	def __div__(self, other):
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
	
	def __idiv__(self, other):
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

######## display helpers ########

def display_point(position):
	return (int(position[0]+0.5), int(SIMULATION_BOUNDS[1]-position[1]+0.5))

def display_points(positions):
	result= []
	for position in positions:
		result.append(display_point(position))
	return result

def display_size(size):
	return int(size+0.5)

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
		pygame.draw.circle(surface, self.color, display_point(self.linear_position), display_size(self.size))

######## bee ########

class c_bee(c_entity):
	def __init__(self):
		position= random.uniform(0.0, SIMULATION_BOUNDS[0]+SIMULATION_BOUNDS[1]+4*BEE_SIZE)
		if (position<SIMULATION_BOUNDS[0]+2*BEE_SIZE):
			linear_position= (position-BEE_SIZE, -BEE_SIZE)
		else:
			linear_position= (-BEE_SIZE, position-SIMULATION_BOUNDS[0]-2*BEE_SIZE-BEE_SIZE)
		c_entity.__init__(self, BEE_SIZE, BEE_COLOR, linear_position, c_vector2d(), random.uniform(0.0, 2*math.pi), 0)
		self.moving= False
		self.timer= 0.0
	
	def update(self):
		self.timer-= DT
		if self.timer<0:
			self.moving= not self.moving
			self.timer= random.uniform(*BEE_MOVE_TIME) if self.moving else random.uniform(*BEE_STOP_TIME)
			if self.moving:
				self.angular_velocity= random.uniform(-BEE_TURN, BEE_TURN)
			else:
				self.linear_velocity= c_vector2d()
				self.angular_velocity= random.uniform(-BEE_TURN, BEE_TURN)
		if self.moving:
			self.linear_velocity= c_vector2d(facing= self.angular_position, magnitude= BEE_SPEED)
		c_entity.update(self)
	
######## swarm ########

class c_swarm(object):
	def __init__(self):
		pygame.init()
		self.display= pygame.display.set_mode(SIMULATION_BOUNDS)
		pygame.display.set_caption('Swarm')
		pygame.time.set_timer(TIMER_EVENT, MILLISECONDS_PER_SECOND//FRAMES_PER_SECOND)
		self.mode= BEE_MODE
		self.desired_bee_count= BEE_INITIAL_COUNT
		self.bees= []
	
	def main(self):
		while self.mode!=QUIT_MODE:
			event= pygame.event.wait()			
			if event.type==TIMER_EVENT:
				pygame.display.flip()
				self.update()
				self.draw()
			elif event.type==KEYDOWN and event.key==K_ESCAPE:
				self.mode= QUIT_MODE
			elif event.type==QUIT:
				self.mode= QUIT_MODE
	
	def update(self):
		if self.mode==BEE_MODE:
			if len(self.bees)<self.desired_bee_count:
				self.bees.append(c_bee())
			elif len(self.bees)>self.desired_bee_count:
				self.bees.pop()
			for bee in self.bees:
				bee.update()
	
	def draw(self):
		self.display.fill(BLACK_COLOR)
		if self.mode==BEE_MODE:
			for bee in self.bees:
				bee.draw(self.display)

######## glue code ########

if __name__ == '__main__':
	game= c_swarm()
	game.main()
