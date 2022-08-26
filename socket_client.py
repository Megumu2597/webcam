import socket
import numpy as np
import cv2
import time
     
soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)#ソケットオブジェクト作成

#soc.connect(("127.0.0.1", 1025))    # サーバー側PCのipと使用するポート
soc.bind(("127.0.0.1", 1025))    # サーバー側PCのipと使用するポート
#soc.bind(("192.168.1.215", 1025))    # サーバー側PCのipと使用するポート

print("接続待機中")  

#soc.listen(1)                     # 接続要求を待機

#soc, addr = soc.accept()          # 要求が来るまでブロック

print("と接続完了")  
#cap2 = cv2.VideoCapture(1)
#cap3 = cv2.VideoCapture(2)
#cap4 = cv2.VideoCapture(3)
cap1 = cv2.VideoCapture(0)
cap1.set(cv2.CAP_PROP_FRAME_WIDTH,1280)
cap1.set(cv2.CAP_PROP_FRAME_HEIGHT,720)
fourcc = cv2.VideoWriter_fourcc(*'XVID')
cap1.set(cv2.CAP_PROP_FOURCC,cv2.VideoWriter_fourcc('H','2','6','4'))
#cap1.set(cv2.CAP_PROP_FOURCC,cv2.VideoWriter_fourcc('m','p','4','v'))
    
while (True):
    #time.sleep(0.5)            #フリーズするなら#を外す。
    ret1, frame1 = cap1.read()
    img2 = cv2.resize(frame1, dsize=(1280,720)) # 1280*720*3=2764800
    
    #img2=frame1   

    #print(img2.shape)
    img2 = img2.tostring()        #numpy行列からバイトデータに変換
    #for i in range(60):
    #    soc.send(img2[i*61440:(i+1)*61440])              # ソケットにデータを送信
    soc.send(img2)
    #k = cv2.waitKey(1)         #↖ 
    ##if k== 13 :                #←　ENTERキーで終了
    #    break                  #↙

cam.releace()     