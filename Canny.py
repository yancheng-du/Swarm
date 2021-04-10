# OpenCV program to perform Edge detection in real time
# import libraries of python OpenCV
# where its functionality resides
import cv2

# np is an alias pointing to numpy library
import numpy as np

# ~ms resolution timer
from timeit import default_timer as timer

# capture frames from a camera
for camera in range(2):
	cap = cv2.VideoCapture(camera)
	if cap.isOpened():
		break
if cap is None or not cap.isOpened():
	sys.exit("no camera")

# total time reading and number of times read
read_total = 0
read_count = 0

# loop runs if capturing has been initialized
while (1):
	read_start = timer()
	# reads frames from a camera
	ret, frame = cap.read()
	read_total += timer() - read_start
	read_count += 1

	blur = cv2.GaussianBlur(frame, (7,7),0)
	#fgmask = fgbg.apply(blur)
	# converting BGR to HSV
	#hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

	# define range of red color in HSV
	#lower_red = np.array([30, 150, 50])
	#upper_red = np.array([255, 255, 180])

	# create a red HSV colour boundary and
	# threshold HSV image
	#mask = cv2.inRange(hsv, lower_red, upper_red)

	# Bitwise-AND mask and original image
	#res = cv2.bitwise_and(frame, frame, mask=mask)

	# Display an original image
	cv2.imshow('Original', blur)

	# finds edges in the input image image and
	# marks them in the output map edges
	edges = cv2.Canny(blur, 200, 200)
	#print(edges.shape)
	#print(np.count_nonzero(edges))
	# Display edges in a frame
	cv2.imshow('Edges', edges)

	# Wait for Esc key to stop
	k = cv2.waitKey(5)&0xFF
	if k==27:
		break

print("read average: ", 1000*read_total//read_count, "ms", sep="")

# Close the window
cap.release()

# De-allocate any associated memory usage
cv2.destroyAllWindows()