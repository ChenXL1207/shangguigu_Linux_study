#include "common.h"
#include <sys/un.h>

#define SERVER_PATH "unix_domain.socket"
#define SERVER_MODE 1
#define CLIENT_MODE 2
#define BUFFER_SIZE 1024

static struct sockaddr_un socket_addr;
static char *buffer = NULL;
static int mode = 0;
static int socketfd = -1;

// 前向声明
void server_func(int socketfd);
void client_func(int socketfd);
void cleanup_socket(void);
void signal_handler(int sig);

int main(int argc, char const *argv[])
{
    // 初始化缓冲区
    buffer = malloc(sizeof(char) * BUFFER_SIZE);
    if(buffer == NULL){
        perror("malloc");
        exit(1);
    }

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

    // 如果是服务器模式，先清理可能存在的旧 socket 文件
    if(mode == SERVER_MODE){
        unlink(SERVER_PATH);
    }

    // 注册信号处理函数，确保程序退出时清理资源
    signal(SIGINT, signal_handler);  // 中断信号，按下Ctrl+C时触发
    signal(SIGTERM, signal_handler); // 终止信号，使用kill命令时触发
    signal(SIGPIPE, SIG_IGN);       // 忽略 SIGPIPE 信号（当客户端断开时，服务器发送数据会触发 SIGPIPE）
    // atexit函数注册一个函数，当程序正常退出时，会自动调用该函数
    atexit(cleanup_socket); // 退出时清理资源，程序正常退出时触发

    // 创建scoket ipc通信
    memset(&socket_addr,0,sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;// 使用UNIX域套接字,本地通信,不需要IP地址,只需要文件路径,不使用IP地址，使用文件路径
    strcpy(socket_addr.sun_path,SERVER_PATH);
    socketfd = socket(AF_UNIX,SOCK_STREAM,0);// 创建套接字,使用UNIX域套接字,本地通信,不需要IP地址,只需要文件路径,不使用IP地址，使用文件路径
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
    // 资源清理由 cleanup_socket 函数处理（通过 atexit 注册）
    return 0;
}

void server_func(int socketfd)
{
    int client_fd = -1;
    
    // 启动服务器,服务端接受消息，收到EOF则结束通信
    printf("启动服务器\n");
    // 1.绑定套接字
    int result = bind(socketfd,(struct sockaddr *)&socket_addr,sizeof(socket_addr));
    if(result < 0){
        perror("bind");
        cleanup_socket();
        exit(EXIT_FAILURE);
    }
    // 2.监听套接字
    result = listen(socketfd,5);// 监听套接字,最多5个客户端连接
    if(result < 0){
        perror("listen");
        cleanup_socket();
        exit(EXIT_FAILURE);
    }
    // 3.接受客户端连接
    socklen_t client_addr_len = sizeof(socket_addr);
    client_fd = accept(socketfd,(struct sockaddr *)&socket_addr,&client_addr_len);// 接受客户端连接,NULL表示不使用客户端地址,NULL表示不使用客户端地址长度
    if(client_fd < 0){
        perror("accept");
        cleanup_socket();
        exit(EXIT_FAILURE);
    }
    
    printf("客户端连接成功，路径: %s\n",socket_addr.sun_path);

    do{
        // 4.接收客户端消息
        memset(buffer,0,BUFFER_SIZE);
        ssize_t recv_result = recv(client_fd,buffer,BUFFER_SIZE,0);// 接收客户端消息,buffer-->接收消息的缓冲区,BUFFER_SIZE-->接收消息的最大长度,0-->默认标志
        
        // 检查接收结果
        if(recv_result < 0){
            // 真正的错误
            perror("recv");
            break;
        }else if(recv_result == 0){
            // 客户端断开连接（正常情况）
            printf("客户端断开连接\n");
            break;
        }
        
        // 处理接收到的消息
        if(strcmp(buffer,"EOF") != 0){
            printf("接收到客户端消息: %s\n",buffer);
            strcpy(buffer,"OK");
            // 5.发送消息给客户端
            ssize_t send_result = send(client_fd,buffer,strlen(buffer),0);// 发送消息给客户端,buffer-->发送消息的缓冲区,strlen(buffer)-->发送消息的长度,0-->默认标志
            if(send_result < 0){
                perror("send");
                break;
            }
        }else{
            printf("客户端发送了EOF,结束通信\n");
            break;
        }
    }while(1);
    
    // 清理客户端连接
    if(client_fd >= 0){
        close(client_fd);
    }
    
    // 服务器关闭时删除 socket 文件（确保在所有退出路径上都能清理）
    unlink(SERVER_PATH);
}

void client_func(int socketfd)
{
    int msgLen = 0;
    // 启动客户端,客户端发送消息，发送EOF则结束通信
    if(connect(socketfd,(struct sockaddr *)&socket_addr,sizeof(socket_addr)) == -1){
        handle_error("connect",-1);
    }
    printf("连接服务器成功\n");
    do{
        msgLen = read(STDIN_FILENO,buffer,BUFFER_SIZE);
        if(send(socketfd,buffer,msgLen,0) == -1){
            handle_error("send",-1);
        }
        memset(buffer,0,BUFFER_SIZE);
        if(recv(socketfd,buffer,BUFFER_SIZE,0) == -1){//接收服务器消息,buffer-->接收消息的缓冲区,BUFFER_SIZE-->接收消息的最大长度,0-->默认标志
            handle_error("recv",-1);
        }
        printf("接收到服务器消息: %s\n",buffer);
        if(strcmp(buffer,"EOF") == 0){
            break;
        }
    }while(1);
}

// 清理 socket 文件的函数
void cleanup_socket(void)
{
    // 确保在服务器模式下删除 socket 文件
    if(mode == SERVER_MODE){
        unlink(SERVER_PATH);  // 即使文件不存在也不会报错
    }
    if(socketfd >= 0){
        close(socketfd);
        socketfd = -1;  // 标记已关闭
    }
    // 注意：buffer 在 main 函数中分配，由 atexit 清理
    // 这里不释放 buffer，避免重复释放
}

// 信号处理函数
void signal_handler(int sig)
{
    cleanup_socket();
    exit(0);
}