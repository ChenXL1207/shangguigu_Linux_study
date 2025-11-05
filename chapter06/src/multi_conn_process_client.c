#include "common.h"

/*
 * 多线程客户端程序
 * 功能：连接到多线程服务器，支持同时发送和接收消息
 */

// 从服务器接收数据的线程函数
void *read_from_server(void *arg) {
    int client_socketfd = *(int *)arg;
    char *read_buf = (char *)malloc(1024);
    ssize_t count = 0;
    
    if (!read_buf) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    memset(read_buf, 0, 1024);
    
    // 循环接收服务器数据
    while ((count = recv(client_socketfd, read_buf, 1024, 0)) > 0) {
        printf("接收到服务器消息: %s", read_buf);
        memset(read_buf, 0, 1024);
    }
    
    // recv返回0表示服务器关闭连接
    if (count == 0) {
        printf("服务器关闭连接\n");
    } else {
        perror("recv");
    }
    
    free(read_buf);
    pthread_exit(NULL);
}

// 向服务器发送数据的线程函数
void *write_to_server(void *arg) {
    int client_socketfd = *(int *)arg;
    char *write_buf = (char *)malloc(1024);
    ssize_t count = 0;
    
    if (!write_buf) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    // 从标准输入读取数据并发送给服务器
    while (fgets(write_buf, 1024, stdin) != NULL) {
        count = send(client_socketfd, write_buf, strlen(write_buf), 0);
        
        if (count > 0) {
            printf("发送数据: %s", write_buf);
        } else if (count == 0) {
            printf("连接已关闭\n");
            break;
        } else {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
    
    // 当输入结束（Ctrl+D）时，关闭写端
    printf("输入结束，关闭写端\n");
    shutdown(client_socketfd, SHUT_WR);
    
    free(write_buf);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int client_socketfd;
    int connect_result;
    pthread_t pid_read;
    pthread_t pid_write;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    // 初始化服务器地址和客户端地址
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    
    // 填写服务器地址
    server_addr.sin_family = AF_INET;  // 地址族, AF_INET --> IPv4
    // 方法一：server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // 方法二：inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);  // 连接到本地服务器
    server_addr.sin_port = htons(6666);  // 端口号
    
    // 客户端网络编程流程
    // 1. 创建套接字
    client_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    handle_error("socket", client_socketfd);
    
    // 2. 绑定客户端地址（可选，通常客户端不需要显式绑定）
    // 如果需要指定客户端端口，可以取消下面的注释
    /*
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(8888);  // 客户端端口
    int bind_result = bind(client_socketfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (bind_result < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    */
    
    // 3. 连接服务器
    connect_result = connect(client_socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_result < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    printf("已连接到服务器 %s:%d，套接字描述符: %d\n", 
           inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), client_socketfd);
    
    // 4. 创建读线程，用于接收服务器消息
    if (pthread_create(&pid_read, NULL, read_from_server, &client_socketfd) != 0) {
        perror("pthread_create read");
        exit(EXIT_FAILURE);
    }
    
    // 5. 创建写线程，用于向服务器发送消息
    if (pthread_create(&pid_write, NULL, write_to_server, &client_socketfd) != 0) {
        perror("pthread_create write");
        exit(EXIT_FAILURE);
    }
    
    // 6. 主线程等待子线程结束
    pthread_join(pid_read, NULL);
    pthread_join(pid_write, NULL);
    
    // 7. 释放资源
    printf("释放资源\n");
    close(client_socketfd);
    
    return 0;
}
