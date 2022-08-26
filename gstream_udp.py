#!/usr/bin/env python

import cv2
print(cv2.__version__)

# Uncommenting this would allow to check if your opencv build has GSTREAMER support 
#print(cv2.getBuildInformation())


cap = cv2.VideoCapture("v4l2src ! videoconvert ! x264enc ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=BGR ! queue ! appsink drop=1", cv2.CAP_GSTREAMER)
#cap = cv2.VideoCapture("udpsrc ! application/x-rtp,media=video,encoding-name=H264 ! queue ! rtpjitterbuffer latency=500 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=BGR ! queue ! appsink drop=1", cv2.CAP_GSTREAMER)

# For NVIDIA using NVMM memory 
#cap = cv2.VideoCapture("udpsrc port=5000 ! application/x-rtp,media=video,encoding-name=H264 ! queue ! rtpjitterbuffer latency=500 ! rtph264depay ! h264parse ! nvv4l2decoder ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw,format=BGR ! queue ! appsink drop=1", cv2.CAP_GSTREAMER)

width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
#fps = cap.get(cv2.CAP_PROP_FPS) #doesn't work with python in my case so forcing below...you may have to adjust for your case
fps = 30

if not cap.isOpened():
   print('Failed to open camera')
   exit

print('Source opened, framing %dx%d@%d' % (width,height,fps))


writer = cv2.VideoWriter("appsrc ! video/x-raw,format=BGR ! queue ! videoconvert ! x264enc insert-vui=1 ! h264parse ! rtph264pay ! udpsink port=5001 host=127.0.0.1 auto-multicast=0", cv2.CAP_GSTREAMER, 0, float(fps), (int(width),int(height))) 

# For NVIDIA using NVMM memory 
#writer = cv2.VideoWriter("appsrc ! video/x-raw,format=BGR ! queue ! videoconvert ! video/x-raw,format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=1 insert-vui=1 ! h264parse ! rtph264pay ! udpsink port=5001", cv2.CAP_GSTREAMER, 0, float(fps), (int(width),int(height))) 

if not writer.isOpened():
   print('Failed to open writer')
   cap.release()
   exit


while True:
    ret_val, img = cap.read();
    if not ret_val:
        break

    writer.write(img);
    cv2.waitKey(1)

writer.release()
cap.release()