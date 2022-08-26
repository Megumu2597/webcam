import cv2
import numpy as np
import subprocess as sp
import shlex
from sys import platform
#https://stackoverflow.com/questions/71907857/how-to-receive-byte-stream-by-using-gstreamer-with-python-subprocess-module-or-g

width = 714
height = 420

input_file_name = 'udp://127.0.0.1:1025'  # For testing, use MP4 input file instead of RTSP input.

# Build MP4 synthetic input video file for testing:
sp.run(shlex.split(f'ffmpeg -y -f lavfi -i testsrc=size={width}x{height}:rate=25:duration=100 -vcodec libx264 -pix_fmt yuv420p {input_file_name}'))

if platform == "win32":
    # stdout_file_name = "con:"
    # gstreamer_exe = 'c:/gstreamer/1.0/msvc_x86_64/bin/gst-launch-1.0.exe'
    raise Exception('win32 system is not supported')
else:
    stdout_file_name = "/dev/stdout"
    gstreamer_exe = 'gst-launch-1.0'

# https://stackoverflow.com/questions/29794053/streaming-mp4-video-file-on-gstreamer
p = sp.Popen(shlex.split(f'{gstreamer_exe} --quiet filesrc location={input_file_name} ! qtdemux ! video/x-h264 ! avdec_h264 ! videoconvert ! capsfilter caps="video/x-raw, format=BGR" ! filesink location={stdout_file_name}'), stdout=sp.PIPE)

while True:
    raw_image = p.stdout.read(width * height * 3)

    if len(raw_image) < width*height*3:
        break

    image = np.frombuffer(raw_image, dtype='uint8').reshape((height, width, 3))
    cv2.imshow('image', image)
    key = cv2.waitKey(1)sd