#!/usr/bin/python

#Don't know if this is actually necessary, but thought it would be consistent with the other modes
#Just spits back out camera-processing as a mode

######## imports ########
import numpy as np
import pygame

######## code ########


class c_canny():
    def __init__(self):
        self.frame = 0

    def update(self, frame):
        self.frame = np.rot90(frame)
        self.frame = np.flip(self.frame, 0)

    def draw(self, display):
        surface = pygame.surfarray.make_surface(self.frame)
        display.blit(surface, (0,0))