extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
}
#include <iostream>
// g++ -o recvh264 recv_h264_udp.cpp 
int main()
{
 int sock;
 struct sockaddr_in addr;

 char buf[2048];

 sock = socket(AF_INET, SOCK_DGRAM, 0);

 addr.sin_family = AF_INET;
 addr.sin_port = htons(1025);
 addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");

 bind(sock, (struct sockaddr *)&addr, sizeof(addr));
while(1){
 memset(buf, 0, sizeof(buf));
 recv(sock, buf, sizeof(buf), 0);

 printf("%s\n", buf);
}
 close(sock);

 return 0;
}
