#!/usr/bin/python

#Don't know if this is actually necessary, but thought it would be consistent with the other modes
#Just spits back out camera-processing as a mode

######## imports ########
import numpy as np
import pygame
import cv2

######## code ########


class c_passthrough():
    def __init__(self):
        self.frame = 0

    def update(self, frame):
        self.frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.frame = cv2.transpose(self.frame)

    def draw(self, display):
        surface = pygame.surfarray.make_surface(self.frame)
        display.blit(surface, (0,0))