import cv2
import pyfakewebcam
import time
# 取得するデータソース（Webカメラ）を選択
cap = cv2.VideoCapture(0)

# 最初のフレームから画像のサイズを取得
ret, frame = cap.read()
camera = pyfakewebcam.FakeWebcam('/dev/video2', frame.shape[1], frame.shape[0])
print(frame.shape[1],frame.shape[0])
while True:
    # 各フレームの画像を取得
    ret, frame = cap.read()

    # ここで何らかのエフェクトをかける

    # 色空間を変更
    # RGB順にする
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 画像を仮想カメラに流す
    camera.schedule_frame(frame)
    cv2.imshow("wind",frame)
    # 画像をスクリーンに表示しなくなったので，次のフレーム（30fps）まで待機する
    time.sleep(0.033)

# 終了処理
cap.release()
