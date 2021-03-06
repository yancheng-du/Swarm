#!/usr/bin/python

######## imports ########

import pygame
from pygame.locals import *

from camera import c_camera
from constants import *
from bees import c_bees
from snow import c_snow
from life import c_life
from passthrough import c_passthrough

######## constants ########

DISPLAY_TITLE= 'Protoswarm'

FONT_NAME= 'Comic Sans MS'
FONT_SIZE= 30

FPS_COLOR= (220, 0, 0)
FPS_POSITION= (0, 0)

TIMER_EVENT= USEREVENT
MILLISECONDS_PER_SECOND= 1000
QUIT_MODE= -1

######## code ########

def initialize_pygame():
	pygame.init()
	display= pygame.display.set_mode(SIMULATION_BOUNDS, 0)
	pygame.display.set_caption(DISPLAY_TITLE)
	pygame.time.set_timer(TIMER_EVENT, MILLISECONDS_PER_SECOND//FRAMES_PER_SECOND)
	return display

if __name__=='__main__':
	display= initialize_pygame()
	camera= c_camera()
	clock= pygame.time.Clock()
	show_fps= False
	font= pygame.font.SysFont(FONT_NAME, FONT_SIZE)
	mode= 0
	swarm= [c_bees(), c_snow(), c_life(), c_passthrough()]
	camera.start()
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
				elif event.key==K_LCTRL or event.key==K_RCTRL:
					show_fps= not show_fps
				elif event.key==K_LALT or event.key==K_RALT:
					mode= (mode+1)%len(swarm)
			elif event.type==QUIT:
				mode= QUIT_MODE
		if tick:
			clock.tick()
			if show_fps:
				display.blit(font.render(str(int(clock.get_fps())), False, FPS_COLOR), FPS_POSITION)
			pygame.display.flip()
			swarm[mode].update(camera.get_frame())
			swarm[mode].draw(display)
	camera.stop()
	camera.join()
