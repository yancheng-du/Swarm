#!/usr/bin/python

import numpy as np
import pygame

from constants import *

######## code ########

class c_life(object):
	def __init__(self):
		self.board= np.zeros((SIMULATION_BOUNDS[1], SIMULATION_BOUNDS[0]))>0

	def update(self, frame):
		self.board|= frame>0
		count= sum(np.roll(np.roll(self.board, i, 0), j, 1) \
			for i in (-1, 0, 1) for j in (-1, 0, 1) if (i != 0 or j != 0))
		self.board= (count==3)|(self.board&(count==2))

	def draw(self, display):
		surface= pygame.pixelcopy.make_surface(255*self.board.T)
		display.blit(surface, (0, 0))
