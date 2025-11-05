#include "common.h"

int main(int argc, char const *argv[])
{
    // 使用UDP协议完成客户端与服务端的通信
    // 使用EOF结束通信
    int socketfd,temp_result;
    struct sockaddr_in server_addr;//服务器地址
    char *buffer = malloc(sizeof(char) * 1024);

    // 清空服务器地址
    memset(&server_addr,0,sizeof(server_addr));

    // 填写服务器地址
    server_addr.sin_family = AF_INET;//地址族,AF_INET-->IPv4
    // 填写IP地址，客户端需要指定服务器地址
    // 方法一：server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 填写IP地址
    // 方法二：inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 将字符串转换为网络字节序的IP地址
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 填写服务器IP地址
    // 填写端口号，htons(10000)-->10000
    server_addr.sin_port = htons(6666);

    // 使用UDP协议完成客户端与服务端的通信,编程流程
    // 1.创建套接字socket
    socketfd = socket(AF_INET,SOCK_DGRAM,0); //创建套接字,AF_INET-->IPv4,SOCK_DGRAM-->UDP,0-->默认协议
    handle_error("socket",socketfd);    

    // 2.绑定地址 bind 注意：客户端不需要绑定地址
    // int bind_result = bind(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    // handle_error("bind",bind_result);

    do{
        printf("请输入要发送的数据: ");
        fflush(stdout);
        // 从控制台读取数据
        int buflen = read(STDIN_FILENO,buffer,1024);
        if(buflen > 0){
            // 移除换行符
            if(buffer[buflen-1] == '\n'){
                buffer[buflen-1] = '\0';
                buflen--;
            }
            buffer[buflen] = '\0';
            if(strcmp(buffer,"EOF") == 0){
                break;
            }
            // 发送实际数据长度，不是整个缓冲区大小
            temp_result = sendto(socketfd,buffer,buflen,0,(struct sockaddr *)&server_addr,sizeof(server_addr));
            handle_error("sendto",temp_result);
            printf("发送数据: %s\n",buffer);
        }else{
            printf("读取数据失败\n");
            break;
        }
        // 清空缓冲区，用于接受数据，避免上一次数据的影响
        memset(buffer,0,1024);

        // 接收数据
        socklen_t server_addr_len = sizeof(server_addr);
        temp_result = recvfrom(socketfd,buffer,1024,0,(struct sockaddr *)&server_addr,&server_addr_len);
        if(temp_result > 0){
            buffer[temp_result] = '\0';
            printf("接收到服务器%s:%d的数据: %s\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),buffer);
            if(strcmp(buffer,"EOF") == 0){
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