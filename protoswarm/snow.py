#!/usr/bin/python

import numpy as np
import pygame

from constants import *

######## constants ########

SNOWFLAKE_SIZE= (1, 3)
SNOWFLAKE_COLOR= WHITE_COLOR
SNOWFLAKE_HORIZONTAL_SPEED= (0, 4)
SNOWFLAKE_VERTICAL_SPEED= (50, 10)

SNOW_BACKGROUND_COLOR= BLACK_COLOR

SNOWFLAKE_COUNT= 20000

######## code ########

class c_snow(object):
	def __init__(self):
		self.snow_times= np.zeros(SNOWFLAKE_COUNT)
		self.snow_sizes= np.random.randint(*SNOWFLAKE_SIZE, SNOWFLAKE_COUNT)
		self.snow_positions= np.column_stack((np.random.uniform(0, SIMULATION_BOUNDS[0], SNOWFLAKE_COUNT), np.random.uniform(0, SIMULATION_BOUNDS[1], SNOWFLAKE_COUNT)))
		self.snow_velocities= np.column_stack((np.random.normal(*SNOWFLAKE_HORIZONTAL_SPEED, SNOWFLAKE_COUNT), np.random.normal(*SNOWFLAKE_VERTICAL_SPEED, SNOWFLAKE_COUNT)))

	def update(self, frame):
		self.snow_positions+= self.snow_velocities*DT
		self.snow_positions%= SIMULATION_BOUNDS
		#self.snow_positions[self.snow_positions[..., 0]-self.snow_sizes<0

	def draw(self, display):
		display.fill(SNOW_BACKGROUND_COLOR)
		for size, position in zip(self.snow_sizes, self.snow_positions):
			pygame.draw.circle(display, SNOWFLAKE_COLOR, position, size)
