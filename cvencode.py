import cv2
import pyfakewebcam
from PIL import Image, ImageFont, ImageDraw
import numpy as np
import os
import sys
import datetime
import json
#import matplotlib.pyplot as plt
import time
import io
import time
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph.exporters
import numpy as np
import base64


def main():
    cap = cv2.VideoCapture("/dev/video0")
    if not cap.isOpened():
        print("exit")
        sys.exit()
    width = cap.get(cv2.CAP_PROP_FRAME_WIDTH) #640
    height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT) #480
    fps = cap.get(cv2.CAP_PROP_FPS)
    camera_width = int(width)     #カメラ幅
    camera_height = int(height)    #カメラ高さ
    centerX = camera_width // 2   #中心(横)
    centerY = camera_height // 2  #中心(縦)
    print(camera_width,camera_height,"camera wh")
    print("start overlay loop")
    filename = "cvencode.h264"
    debug = False
    #h264のjpegにして
    i = 0
    while True:
        ret, frame = cap.read()
        if debug:
            cv2.imwrite(filename,frame)
        else:
            i += 1
            ext = os.path.splitext(filename)
            print(ext)#リストになってる.
            frame = cv2.resize(frame, (640, 480))
            result, buf = cv2.imencode(".jpg", frame, None)
            image = base64.b64encode(buf)
            print(image[10])
            if result:
                with open(filename, mode='w+b') as f:
                    np.frombuffer(image, dtype = 'int16').tofile(f)
                print("success")
                if i>60:
                    return True
            else:
                print("failure")
                return False
            

if __name__ == '__main__':
    main()