#include "common.h"

int main(int argc, char const *argv[])
{
    // 使用UDP协议完成客户端与服务端的通信
    // 使用EOF结束通信
    int socketfd,temp_result;
    struct sockaddr_in server_addr,client_addr;//服务器地址和客户端地址
    char *buffer = malloc(sizeof(char) * 1024);

    // 清空服务器地址和客户端地址
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    // 填写服务器地址
    server_addr.sin_family = AF_INET;//地址族,AF_INET-->IPv4
    // 填写IP地址，本地地址：0.0.0.0    
    // 方法一：server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 填写IP地址
    // 方法二：inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 将字符串转换为网络字节序的IP地址
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 填写IP地址,INADDR_ANY-->0.0.0.0
    // 填写端口号，htons(10000)-->10000
    server_addr.sin_port = htons(6666);

    // 使用UDP协议完成客户端与服务端的通信,编程流程
    // 1.创建套接字socket
    socketfd = socket(AF_INET,SOCK_DGRAM,0); //创建套接字,AF_INET-->IPv4,SOCK_DGRAM-->UDP,0-->默认协议
    handle_error("socket",socketfd);    
    // 2.绑定地址 bind
    int bind_result = bind(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("bind",bind_result);

    do{
        // 3.接收数据 recvfrom
        memset(buffer,0,1024);
        // 计算客户端地址长度
        socklen_t client_addr_len = sizeof(client_addr);
        temp_result = recvfrom(socketfd,buffer,1024,0,(struct sockaddr *)&client_addr,&client_addr_len);//接收数据,buffer-->接收数据的缓冲区,1024-->接收数据的最大长度,0-->默认标志,client_addr-->客户端地址,client_addr_len-->客户端地址长度
        if(temp_result > 0){
            buffer[temp_result] = '\0';
            if(strcmp(buffer,"EOF") != 0){
                printf("接收到客户端%s:%d的数据: %s\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),buffer);
                strcpy(buffer,"OK");
                // 4.发送数据 sendto
                temp_result = sendto(socketfd,buffer,strlen(buffer),0,(struct sockaddr *)&client_addr,client_addr_len);//发送数据,buffer-->发送数据的缓冲区,strlen(buffer)-->发送数据的实际长度,0-->默认标志,client_addr-->客户端地址,client_addr_len-->客户端地址长度
                if(temp_result < 0){
                    handle_error("sendto",temp_result);
                }
            }else{
                printf("客户端%s:%d发送了EOF,结束通信\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                break;
            }
        }else{
            printf("接收数据失败\n");
            break;
        }
    }while(1);

    // 4.关闭套接字 close
    close(socketfd);
    free(buffer);

    return 0;
}