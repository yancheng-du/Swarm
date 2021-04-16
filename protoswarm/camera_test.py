#!/usr/bin/python

import cv2
import sys
from timeit import default_timer as timer

from camera import c_camera

######## code ########

if __name__=='__main__':
	camera= c_camera()
	width= int(camera.capture.get(cv2.CAP_PROP_FRAME_WIDTH))
	height= int(camera.capture.get(cv2.CAP_PROP_FRAME_HEIGHT))
	fps= camera.capture.get(cv2.CAP_PROP_FPS)
	print(width, height, fps)
	if len(sys.argv)==1:
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
			cv2.imshow("Single-threaded Camera", frame)
		print("read: %.1fms"%(1000*read_total/read_count), sep="")
		print("process: %.1fms"%(1000*process_total/process_count), sep="")
	else:
		get_total= 0
		get_count= 0
		camera.start()
		while cv2.waitKey(33)!=27:
			start= timer()
			frame= camera.get_frame()
			get_total+= timer()-start
			get_count+= 1
			cv2.imshow("Multithreaded Camera", frame)
		print("get: %.1fms"%(1000*get_total/get_count), sep="")
		camera.stop()
		camera.join()
	cv2.destroyAllWindows()
	camera.capture.release()
