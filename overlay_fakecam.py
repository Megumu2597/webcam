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

def fig2img(fig):
    """Convert a Matplotlib figure to a PIL Image and return it"""
    buf = io.BytesIO()
    fig.savefig(buf)
    buf.seek(0)
    img = Image.open(buf)
    return img

def qimage_to_cv(qimage):
    w, h, d = qimage.size().width(), qimage.size().height(), qimage.depth()
    bytes_ = qimage.bits().asstring(w * h * d // 8)
    #print(type(qimage))
    arr = np.frombuffer(bytes_, dtype=np.uint8).reshape((h, w, d // 8))
    #arr = Image.open(io.BytesIO(bytes_))
    return arr

#画像にグラフを入れる関数

def graph_qt(img,array,camera_height,camera_width,fps,graph_flag,graph_img,curve,p):

    if graph_flag: #i%round(fps)==0:
        left = np.arange(1,1+2*10) #横軸
        curve.setData(left,array)
        exporter = pg.exporters.ImageExporter(p.plotItem)
        fig=exporter.export(toBytes=True)
        graph_img = qimage_to_cv(fig)
        graph_img = Image.fromarray(graph_img)
    #print("graph_img.size:   ",graph_img.width,graph_img.height) ちゃんと216,144でした.
    graph_img = graph_img.resize((216, 144))#graph_img.width//2の結果,半分にしたい
    position = (0,camera_height-graph_img.height+10)    #枠の分ちょっと大きいので10とか0.9で調整
    img.paste(graph_img,position,graph_img)
    return img, graph_img

def overlay():    
    write=False
    #gasデータ読み取り
    dir = '/home/ubuntu/webcam'               # 動画が保存されているディレクトリ
    #path = 'A5A3.mp4'   # ファイル名
    #path = 'DJI_20220216104709_0001_W.MP4'
    #step = 1                    # 動画内で処理するフレーム間隔（1より大きい整数だと動画が間引かれる）
    plogPath = 'A5A3.log'
    logLines = []
    datetime_format = '%Y/%m/%d  %H:%M:%S.%f'
    past_sec = 2 #ガス何秒分みるか
    baseTime = datetime.datetime.strptime('2022/02/16 14:59:45.282', datetime_format)
    past_10_gas=np.array([0]*10*past_sec) #ガスn秒分,10hz
    with open(os.path.join(*[dir, plogPath])) as f:
        for line in f:
            if len(line) > 0 :
                logObj = json.loads(line)
                logObj['gasdetection']['time_obj'] = datetime.datetime.strptime(logObj['gasdetection']['time'], datetime_format)
                #logObj['gasdetection']['time_obj'] = logObj['gasdetection']['time_obj'] + datetime.timedelta(seconds=-4)
                #A3A5
                logObj['gasdetection']['time_obj'] = logObj['gasdetection']['time_obj'] + datetime.timedelta(seconds=4)
                #A5A3
                #logObj['gasdetection']['time_obj'] = logObj['gasdetection']['time_obj'] + datetime.timedelta(seconds=0)
                logLines.append(logObj['gasdetection'])


    # 取得するデータソース（Webカメラ）を選択
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        sys.exit()
    width = cap.get(cv2.CAP_PROP_FRAME_WIDTH) #640
    height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT) #480
    fps = cap.get(cv2.CAP_PROP_FPS)
    camera_width = int(width)     #カメラ幅
    camera_height = int(height)    #カメラ高さ
    centerX = camera_width // 2   #中心(横)
    centerY = camera_height // 2  #中心(縦)
    
    #for videowriterのためniyaru .
    if (write):
        size = (width, height)
        cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"H264"))
        fmt = cv2.VideoWriter_fourcc('h', '2', '6', '4') # ファイル形式(ここではmp4v)
        writer = cv2.VideoWriter(dir+'temp_overlay.h264', fmt, fps, size)

    last_time=-100
    video_start_time = time.time()
    graph_flag = True
    counter = 0
    graph_img = -1#適当に入れとく

    #QTGRAPHを作る
    p = pg.plot()
    curve= p.plot(pen ='g', symbol ='x', symbolPen ='g', symbolBrush = 0.2, name ='green')
    p.setXRange(1, 21)
    p.setYRange(0, 400)
    p.showGrid(x = True, y = True)
    y=200*np.ones(20)
    p.plot(y,pen=pg.mkPen('b', width=5))#'r')
    p.win.hide() #プロットのウィンドウを非表示にする
    #p.setBackground("#FFFFFF00")
    
    out = cv2.VideoWriter(
        #'appsrc ! videoconvert ! videoscale ! video/x-raw,format=I420 ! v4l2sink device=/dev/video2',
        'appsrc ! videoconvert ! x264enc bframes=0 key-int-max=45 bitrate=500 tune=zerolatency ! video/x-h264 ! v4l2sink device=/dev/video2',
        #'v4l2src ! videoconvert ! videoscale ! video/x-raw,format=I420 ! x264enc bframes=0 key-int-max=45 bitrate=500 tune=zerolatency ! video/x-h264,stream-format=avc,alignment=au ! v4l2sink device=/dev/video2',
        0,           # 出力形式。今回は0で。
        30,          # FPS
        (1280, 960),  # 出力画像サイズ
        True,        # カラー画像フラグ
    )

    # 最初のフレームから画像のサイズを取得
    camera = pyfakewebcam.FakeWebcam('/dev/video2', camera_width, camera_height) #frame.shape[1], frame.shape[0])
    print("start overlay loop")
    while True:
        past_time=time.time()
        # 各フレームの画像を取得
        ret, frame = cap.read()
        video_time = time.time() - video_start_time
        #ガス受け取り
        min = int(video_time / 60)
        sec = int(video_time % 60)
        msec = int(video_time * 100) % 100
        s_min = str(min).zfill(2)
        s_sec = str(sec).zfill(2)
        s_msec = str(msec).zfill(2)
        s_time = s_min + ':' + s_sec + ':' + s_msec
        millisec = int (video_time * 1000)

        frameTime = baseTime + datetime.timedelta(milliseconds=millisec)
        lastLog = logLines[0]
        for logLine in logLines :
            if (logLine['time_obj'] > frameTime) :
                break
            lastLog = logLine
        past_10_gas = np.append(past_10_gas,lastLog['ppm-m'])[1:]
        #各種描画
        #十字
        cv2.line(frame, (0, centerY), (camera_width, centerY), (175, 175, 175), thickness=1)
        cv2.line(frame, (centerX, 0), (centerX, camera_height), (175, 175, 175), thickness=1)
        ##ターゲット
        #cv2.rectangle(frame,(centerX - targetSquareWidth // 2, centerY - targetSquareHeight // 2),(centerX + targetSquareWidth // 2,centerY + targetSquareHeight // 2),(175,175,175),1)
        #左上の枠
        #cv2.rectangle(frame,(0, 0),(60,60),(175,175,175),1)
        ##右下の枠
        #cv2.rectangle(frame,(camera_width - 120, camera_height - 60),(camera_width,camera_height),(175,175,175),1)
        # #test20211128
        # #waypoint表示 
        #label = "WP:" + str(current_aircraft_waypoint)
        #frame = cv2.putText(frame,label,(0, camera_height-10-20-35), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2, cv2.LINE_AA)
        #frame = cv2.putText(frame,label,(0, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 1, cv2.LINE_AA)    #test20220404
        # #test20211128

        #グラフの描画　毎秒更新するためにgraph_flag
        graph_flag = True if (int(video_time) - last_time) > 1 else False
        img = Image.fromarray(frame)
        img, graph_img = graph_qt(img,past_10_gas,camera_height,camera_width,round(fps),graph_flag,graph_img,curve,p)
        frame = np.array(img)
        if graph_flag:
            last_time=int(video_time)
        graph_flag = False 
        #add end

        # 画像表示
        #cv2.imshow('frame', frame)
        
        # ここで何らかのエフェクトをかける
        #例: 色空間をRGB順に変更してみる なんか青いのでこれ必要.
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # 画像を仮想カメラに流す
        camera.schedule_frame(frame)
        #out.write(frame)
        if (write):
            writer.write(frame)
        #fpsの計測
        now=time.time()
        delta = now-past_time
        #print(frame.tobytes())
        # 画像をスクリーンに表示しなくなったので，次のフレーム（30fps）まで待機する
        #time.sleep(0.015)
        sleep_time=1.0 / 30.1 - delta  
        if(sleep_time>0):
            time.sleep(sleep_time)
        else:
            time.sleep(0.00001)
        #print("fps:",1.0/(time.time()-past_time)) #29.9くらい

    # 終了処理
    cap.release()


if __name__ == '__main__':
    overlay()