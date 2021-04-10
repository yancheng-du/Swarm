#!/usr/bin/python

######## imports ########

import pygame
from pygame.locals import *

from camera import c_camera
from constants import *
from bees import c_bees

######## constants ########

TIMER_EVENT= USEREVENT
MILLISECONDS_PER_SECOND= 1000

QUIT_MODE= 0
BEE_MODE= 1

######## code ########

def initialize_pygame():
	pygame.init()
	display= pygame.display.set_mode(SIMULATION_BOUNDS, pygame.FULLSCREEN | pygame.HWSURFACE | pygame.DOUBLEBUF | pygame.HWACCEL)
	pygame.display.set_caption('Protoswarm')
	pygame.time.set_timer(TIMER_EVENT, MILLISECONDS_PER_SECOND//FRAMES_PER_SECOND)
	return display

if __name__=='__main__':
	display= initialize_pygame()
	camera= c_camera()
	clock = pygame.time.Clock()
	mode= BEE_MODE
	swarm= c_bees(see_nectar=False)
	while mode!=QUIT_MODE:
		tick= False
		events= [pygame.event.wait()]
		events+= pygame.event.get()
		for event in events:
			if event.type==TIMER_EVENT:
				tick= True
			elif event.type==KEYDOWN:
				if event.key==K_ESCAPE:
					mode= QUIT_MODE
			elif event.type==QUIT:
				mode= QUIT_MODE
		if tick:
			clock.tick(FRAMES_PER_SECOND)
			fps_counter = pygame.font.SysFont('Comic Sans MS', 30)
			fps_surface = fps_counter.render(str(int(clock.get_fps())), False, (220, 0, 0))
			display.blit(fps_surface, (0,0))
			pygame.display.flip()
			swarm.update(camera.process_frame(camera.read_frame()))
			swarm.draw(display)
