#!/usr/bin/python

import math
import pygame

from constants import *

######## code ########

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

class c_entity(object):
	def __init__(self, size, color, linear_position, linear_velocity, angular_position, angular_velocity):
		assert(size>0)
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
