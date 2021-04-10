#!/usr/bin/python

######## constants ########

import cv2
import numpy as np

from constants import SIMULATION_BOUNDS

######## code ########

class c_camera(object):
	def __init__(self):
		for camera in range(2):
			capture= cv2.VideoCapture(camera)
			if capture is not None:
				if capture.isOpened():
					capture.set(3, SIMULATION_BOUNDS[0])
					capture.set(4, SIMULATION_BOUNDS[1])
					self.capture= capture
					return
		self.capture= None
		print("No camera")

	def read_frame(self):
		if self.capture is not None:
			good, frame= self.capture.read()
			if good:
				return frame
			print("Bad frame")
		return None

	def process_frame(self, frame):
		if frame is not None:
			flip= cv2.flip(frame, 1)
			blur= cv2.GaussianBlur(flip, (3, 3), 0)
			edges= cv2.Canny(blur, 100, 200)
			edges_thick = cv2.blur(edges, (5, 5))
			return edges_thick.T
		return np.zeros([SIMULATION_BOUNDS[0], SIMULATION_BOUNDS[1]])
