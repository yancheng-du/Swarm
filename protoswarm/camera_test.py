#!/usr/bin/python

import cv2
from timeit import default_timer as timer

from camera import c_camera

######## code ########

if __name__=='__main__':
	camera= c_camera()
	width= int(camera.capture.get(cv2.CAP_PROP_FRAME_WIDTH))
	height= int(camera.capture.get(cv2.CAP_PROP_FRAME_HEIGHT))
	fps= camera.capture.get(cv2.CAP_PROP_FPS)
	print(width, height, fps)
	read_total= 0
	read_count= 0
	process_total= 0
	process_count= 0
	while cv2.waitKey(1)!=27:
		start= timer()
		frame= camera.read_frame()
		read_total+= timer()-start
		read_count+= 1
		start= timer()
		frame= camera.process_frame(frame)
		process_total+= timer()-start
		process_count+= 1
		cv2.imshow("Camera", frame)
	print("read: %.1fms"%(1000*read_total/read_count), sep="")
	print("process: %.1fms"%(1000*process_total/process_count), sep="")
	cv2.destroyAllWindows()
	camera.capture.release()
