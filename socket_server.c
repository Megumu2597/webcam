#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    while(1){
        // パケット受信。パケットが到着するまでブロック
        if(recv(acc_sd, data, sizeof(data), 0) < 0) {
            perror("recv");
            return -1;
        }
        // 受信データの出力
        printf("%d\n",strlen(data));
    }
    
    // パケット送受信用ソケットのクローズ
    close(acc_sd);
 
    // 接続要求待ち受け用ソケットをクローズ
    close(sd);
 
 
    return 0;
}
//gcc -o server socket_server.c 