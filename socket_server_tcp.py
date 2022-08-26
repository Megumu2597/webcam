import socket
import numpy as np
import cv2,time
fps=7
size=(1280,720)
fourcc=cv2.VideoWriter_fourcc(*'MJPG')
video=cv2.VideoWriter('outputMJPG.avi',fourcc,fps,size)


#soc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDPソケットオブジェクト作成
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #TCPソケットオブジェクト作成

#soc.connect(("10.231.249.146", 1025))#サーバー側のipと使用するポート(ポートはサーバーと同じにする。)
s.bind(("127.0.0.1", 1025))    # サーバー側PCのipと使用するポート
#soc.connect(("192.168.1.215", 1025))    # サーバー側PCのipと使用するポート
print("接続待機中")  
s.listen(1)# 接続要求を待機,サーバー有効化
soc, addr = s.accept()          # 要求が来るまでブロック
print(str(addr)+"と接続完了")
buf=b''
while(1):
    data = soc.recv(207360)#引数は下記注意点参照
    #print(len(data))
    
    buf=buf+data
    if len(buf)>2764800:
        tmp = buf[2764800:]
        buf = buf[0:2764800]
        buf = np.fromstring(buf,dtype=np.uint8)#バイトデータ→ndarray変換
        #print(data.shape)
        buf = np.reshape(buf,(720,1280,3))#形状復元(これがないと一次元行列になってしまう。)　reshapeの第二引数の(480,640,3)は引数は送られてくる画像の形状

        cv2.imshow("w",buf)
        video.write(buf)
        buf=tmp

    #time.sleep(0.001)

    if cv2.waitKey(1) & 0xFF ==ord('q'):
        break
#video.release()

cv2.destroyAllWindows() # 作成したウィンドウを破棄   video.release()
