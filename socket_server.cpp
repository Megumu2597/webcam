#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    int sd;
    int acc_sd;
    struct sockaddr_in addr;
 
    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in from_addr;
 
    char data[2764800];
    
   // 受信バッファの初期化
    memset(data, 0, sizeof(data));
 
    // IPv4 TCP のソケットを作成
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
 
    // 待ち受けるIPとポート番号を設定
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1025);
    addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
 
    // バインドする
    if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
 
    // パケット受信待ち状態とする
    // 待ちうけキューを１０としている
    if(listen(sd, 10) < 0) {
        perror("listen");
        return -1;
    }
 
    // クライアントからコネクト要求が来るまで停止する
    // 以降、サーバ側は acc_sd を使ってパケットの送受信を行う
    if((acc_sd = accept(sd, (struct sockaddr *)&from_addr, &sin_size)) < 0) {
        perror("accept");
        return -1;
    }

    cv::Mat img;
    img = cv::Mat::zeros(720 , 1280, CV_8UC1);    
    int imgSize = img.total() * img.elemSize();
    uchar *iptr = img.data;
    int bytes = 0;
    int key;
    //make img continuos
    if ( ! img.isContinuous() ) { 
          img = img.clone();
    }
    cv::VideoWriter writer; // 動画ファイルを書き出すためのオブジェクトを宣言する
    int    width=1280, height=720, fourcc; // 作成する動画ファイルの設定
    fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v'); // ビデオフォーマットの指定( ISO MPEG-4 / .mp4)
	double fps=30.0;
    writer.open("CloneVideo.mp4", fourcc, fps, cv::Size(width, height));

    while(1){
        // パケット受信。パケットが到着するまでブロック
        //if(recv(acc_sd, data, sizeof(data), 0) < 0) {
        if(recv(acc_sd, iptr, sizeof(iptr), 0) < 0) {
            perror("recv");
            return -1;
        }
        // 受信データの出力
        printf("%lu\n",sizeof(iptr));
        //printf("%lu\n",sizeof(data));
        //cv処理
        cv::imshow("showing", img);
        writer << img;// 画像 image を動画ファイルへ書き出す
		if (cv::waitKey(1) == 'q') break; //qを押すと終了

    }
    
    // パケット送受信用ソケットのクローズ
    //pclose(acc_sd);
 
    // 接続要求待ち受け用ソケットをクローズ
    //pclose(sd);
 
 
    return 0;
}
//gcc -o server socket_server.c 
//g++ -o ppserver socket_server.cpp `pkg-config --cflags opencv4` `pkg-config --libs opencv4`