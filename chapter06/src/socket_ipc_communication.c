#include "common.h"
#define SERVER_PATH "unix_domain.socket"
#define SERVER_MODE 1
#define CLIENT_MODE 2
#define BUFFER_SIZE 1024

static struct sockaddr_un socket_addr;
static char *buffer = malloc(sizeof(char) * BUFFER_SIZE);
static int mode = 0;

int main(int argc, char const *argv[])
{
    // 设计结构
    // 通过main方法传参的方式，启动不同的客户端
    // 如果不添加参数，则启动服务器
    if(argc == 1 || strcmp(argv[1],"server") == 0){
        // 启动服务器
        mode = SERVER_MODE;
    }else if(strcmp(argv[1],"client") == 0){
        // 启动客户端
        mode = CLIENT_MODE;
    }else{
        printf("Usage: %s [server|client]\n",argv[0]);
        exit(1);
    }

    // 创建scoket ipc通信
    memset(&socket_addr,0,sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;// 使用UNIX域套接字,本地通信,不需要IP地址,只需要文件路径,不使用IP地址，使用文件路径
    strcpy(socket_addr.sun_path,SERVER_PATH);
    int socketfd = socket(AF_UNIX,SOCK_STREAM,0);// 创建套接字,使用UNIX域套接字,本地通信,不需要IP地址,只需要文件路径,不使用IP地址，使用文件路径
    handle_error("socket",socketfd);
    
    switch(mode){
        case SERVER_MODE:
            server_func(socketfd);
            break;
        case CLIENT_MODE:
            client_func(socketfd);
            break;
        default:
            printf("Usage: %s [server|client]\n",argv[0]);
            exit(1);
    }
    close(socketfd);
    free(buffer);
    return 0;
}

void server_func(int socketfd)
{
    // 启动服务器,服务端接受消息，收到EOF则结束通信
    printf("启动服务器\n");
    // 1.绑定套接字
    int result = bind(socketfd,(struct sockaddr *)&socket_addr,sizeof(socket_addr));
    handle_error("bind",result);
    // 2.监听套接字
    result = listen(socketfd,5);// 监听套接字,最多5个客户端连接
    handle_error("listen",result);
    // 3.接受客户端连接
    socketlen_t client_addr_len = sizeof(socket_addr);
    int client_fd = accept(socketfd,(struct sockaddr *)&socket_addr,&client_addr_len);// 接受客户端连接,NULL表示不使用客户端地址,NULL表示不使用客户端地址长度
    handle_error("accept",client_fd);
    
    printf("客户端%s:%d连接成功\n",inet_ntoa(socket_addr.sun_addr),ntohs(socket_addr.sun_port));

    do{
        // 4.接收客户端消息
        memset(buffer,0,BUFFER_SIZE);
        int result = recv(client_fd,buffer,BUFFER_SIZE,0);// 接收客户端消息,buffer-->接收消息的缓冲区,BUFFER_SIZE-->接收消息的最大长度,0-->默认标志
        handle_error("recv",result);
        if(strcmp(buffer,"EOF") != 0){
            printf("接收到客户端消息: %s\n",buffer);
            strcpy(buffer,"OK");
            // 5.发送消息给客户端
            result = send(client_fd,buffer,strlen(buffer),0);// 发送消息给客户端,buffer-->发送消息的缓冲区,strlen(buffer)-->发送消息的长度,0-->默认标志
            handle_error("send",result);
        }else{
            printf("客户端发送了EOF,结束通信\n");
            break;
        }
    }while(1);
}

void client_func(int socketfd)
{
    // 启动客户端,客户端发送消息，发送EOF则结束通信
    printf("启动客户端\n");
}