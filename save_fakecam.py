import cv2
import time
import numpy as np
video = cv2.VideoCapture("/dev/video2") # 引数がファイルパス

frame_rate = video.get(cv2.CAP_PROP_FPS)
width = int(video.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(video.get(cv2.CAP_PROP_FRAME_HEIGHT))
size = (width, height)
#video.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"H264"))
print(frame_rate)
fmt = cv2.VideoWriter_fourcc(*'mp4v')#'h', '2', '6', '4') # ファイル形式(ここではmp4v)
writer = cv2.VideoWriter('./savetest_5sec_.mp4', fmt, frame_rate, size) # ライター作成
start = time.time()
duration = 0
while (duration<1.0):
  ret, frame = video.read()
  writer.write(frame)
  duration = time.time() - start
print(frame.shape)
print("record finish")
writer.release()
video.release()
cv2.destroyAllWindows()

print("---end---")
