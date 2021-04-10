#!/usr/bin/python

import numpy as np
import random

from constants import *
from entity import *

######## constants ########

BEE_SIZE= 2.0
BEE_COLOR= YELLOW_COLOR
BEE_CHANGE_TIME= (0.2, 0.8)
BEE_TURN_RATE= (-math.pi, math.pi)
BEE_SPEED= 150.0
BEE_INITIAL_COUNT= 1200
BEE_COUNT_RATE= 300

######## code ########

class c_bee(c_entity):
	def __init__(self):
		edge_position= random.uniform(0.0, SIMULATION_BOUNDS[0]+SIMULATION_BOUNDS[1]+4*BEE_SIZE)
		if edge_position<SIMULATION_BOUNDS[0]+2*BEE_SIZE:
			linear_position= (edge_position-BEE_SIZE, -BEE_SIZE)
		else:
			linear_position= (-BEE_SIZE, edge_position-SIMULATION_BOUNDS[0]-2*BEE_SIZE-BEE_SIZE)
		c_entity.__init__(self, BEE_SIZE, BEE_COLOR, linear_position, c_vector2d(), random.uniform(0.0, 2*math.pi), 0)
		self.timer= 0.0

	def update(self, frame):
		(x, y)= np.clip((int(self.linear_position.x), int(self.linear_position.y)), 0, (SIMULATION_BOUNDS[0]-1, SIMULATION_BOUNDS[1]-1))
		if frame[x, y]==0:
			if self.timer<=0.0:
				self.timer= random.uniform(*BEE_CHANGE_TIME)
				self.angular_velocity= random.uniform(*BEE_TURN_RATE)
			else:
				self.timer-= DT
			self.linear_velocity= c_vector2d(facing=self.angular_position, magnitude=BEE_SPEED)
		else:
			self.linear_velocity= c_vector2d()
		c_entity.update(self)

class c_bees(object):
	def __init__(self, see_nectar= False):
		self.desired_bee_count= BEE_INITIAL_COUNT
		self.bees= []
		self.see_nectar= see_nectar

	def update(self, frame):
		for i in range(BEE_COUNT_RATE//FRAMES_PER_SECOND):
			if len(self.bees)<self.desired_bee_count:
				self.bees.append(c_bee())
			elif len(self.bees)>self.desired_bee_count:
				self.bees.pop()
		for bee in self.bees:
			bee.update(frame)

	def draw(self, display):
		display.fill(BLACK_COLOR)
		if self.see_nectar:
			pass
		for bee in self.bees:
			bee.draw(display)
