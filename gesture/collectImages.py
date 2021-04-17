import cv2
import os
import time

#uuid will help make sure all images in folder will have a unique name
import uuid

#Folder/Path to Folder where images will be stored
IMAGES_PATH = 'collectedImages/'

#List of gestures to train the model on 
labels = ['longhorn', 'peace', 'circle', 'palm', 'fist']

#Number of images to capture for each gesture
NUMBER_IMAGES = 15


for label in labels:
    try:
        os.makedirs(os.path.join(IMAGES_PATH, label))
    except FileExistsError:
        print(label + ' folder already created')
    cap = cv2.VideoCapture(0)
    print('Collecting images for {}'.format(label))
    for imgnum in range(NUMBER_IMAGES):
        ret, frame = cap.read()    
        imgname = os.path.join(IMAGES_PATH, label,label+'.'+'{}.jpg'.format(str(uuid.uuid1())))
        cv2.imwrite(imgname, frame)
        cv2.imshow('frame', frame)
        time.sleep(2)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
