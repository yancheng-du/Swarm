#!/usr/bin/python

import cv2
import numpy as np

from constants import *

######## code ########

class c_camera(object):
	def __init__(self):
		for camera in range(2):
			capture= cv2.VideoCapture(camera)
			if capture is not None:
				if capture.isOpened():
					capture.set(cv2.CAP_PROP_FRAME_WIDTH, SIMULATION_BOUNDS[0])
					capture.set(cv2.CAP_PROP_FRAME_HEIGHT, SIMULATION_BOUNDS[1])
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
			cv2.flip(frame, 1, frame)
			cv2.GaussianBlur(frame, (3, 3), 0, frame)
			frame= cv2.Canny(frame, 100, 200)
			cv2.blur(frame, (5, 5), frame)
			return frame
		return np.zeros((SIMULATION_BOUNDS[1], SIMULATION_BOUNDS[0]))
