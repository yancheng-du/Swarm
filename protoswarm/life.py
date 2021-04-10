#!/usr/bin/python

######## code ########

class c_life(object):
	def __init__(self):
		pass

	def update(self, frame):
		pass

	def draw(self, display):
		center= (SIMULATION_BOUNDS[0]//2, SIMULATION_BOUNDS[1]//2)
		radius= math.min(SIMULATION_BOUNDS[0], SIMULATION_BOUNDS[1])//2
		pygame.draw.circle(display, WHITE_COLOR, center, radius)
