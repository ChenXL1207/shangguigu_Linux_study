#include "common.h"
/*
    sockaddr_in结构体：用于IPv4地址的结构体
    struct sockaddr_in {
        sa_family_t sin_family; // 地址族,AF_INET-->IPv4
        in_port_t sin_port; // 端口号,htons(8888)-->8888
        struct in_addr sin_addr; // IP地址,INADDR_ANY-->0.0.0.0
        unsigned char sin_zero[8]; // 填充字段，用于兼容sockaddr结构体
    };
    in_addr结构体：用于IPv4地址的结构体
    struct in_addr {
        in_addr_t s_addr; // IP地址,INADDR_ANY-->0.0.0.0
    };
    in_addr_t类型：用于IPv4地址的结构体
    typedef unsigned long in_addr_t;
    in_addr_t类型：用于IPv4地址的结构体
*/
int main(int argc, char const *argv[]){
    // 第一步：创建TCP套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // 创建TCP套接字,AF_INET-->IPv4,SOCK_STREAM-->TCP,0-->默认协议
    handle_error("socket", server_fd);

    // 第二步：绑定地址
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    memset(&server_addr,0,server_addr_len);
    server_addr.sin_family = AF_INET; // 地址族,AF_INET-->IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 填写IP地址,INADDR_ANY-->0.0.0.0
    server_addr.sin_port = htons(8888); // 填写端口号,htons(8888)-->8888
    int bind_result = bind(server_fd,(struct sockaddr *)&server_addr,server_addr_len);
    handle_error("bind",bind_result);

    // 第三步：监听套接字
    int listen_result = listen(server_fd,10); // 监听套接字,10-->连接队列的最大长度
    handle_error("listen",listen_result);
    printf("服务器启动，监听端口: 8888\n");
    printf("等待客户端连接...\n");

    // 第四步：循环接受客户端连接
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        memset(&client_addr,0,client_addr_len);
        
        // 接受客户端连接,server_fd-->服务器套接字,client_addr-->客户端地址,client_addr_len-->客户端地址长度
        // 注意第三个参数是地址长度的指针，与bind函数不同
        int client_fd = accept(server_fd,(struct sockaddr *)&client_addr,&client_addr_len); 
        handle_error("accept",client_fd);

        printf("客户端连接: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 第五步：循环读取客户端数据并回显
        char buffer[1024];
        while(1) {
            memset(buffer,0,sizeof(buffer));
            // 使用 recv 读取数据
            int recv_len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if(recv_len > 0) {
                // 接收到数据，显示并回显
                printf("收到客户端数据[%s:%d]: %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                
                // 回显数据
                int send_result = send(client_fd, buffer, recv_len, 0);
                if(send_result < 0) {
                    perror("send");
                    break;
                }
            } else if(recv_len == 0) {
                // 客户端关闭连接
                printf("客户端[%s:%d] 关闭连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                break;
            } else {
                // 读取错误
                perror("recv");
                break;
            }
        }

        // 第七步：关闭客户端套接字
        close(client_fd);
        printf("客户端[%s:%d] 连接已关闭\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("等待下一个客户端连接...\n");
    }

    // 关闭服务器套接字（正常情况下不会执行到这里）
    close(server_fd);
    return 0;
}