#include "common.h"

// 多线程TCP客户端：同时支持发送和接收

void *send_thread(void *arg) {
    int client_fd = *(int *)arg;
    char send_buf[1024];

    while (1) {
        // 从标准输入读取一行
        if (fgets(send_buf, sizeof(send_buf), stdin) == NULL) {
            break;
        }
        int len = strlen(send_buf);
        if (len == 0) continue;
        int send_result = send(client_fd, send_buf, len, 0);
        if (send_result <= 0) {
            perror("send");
            break;
        }
    }
    // 关闭写，通知服务器和接收线程
    shutdown(client_fd, SHUT_WR);
    return NULL;
}

void *recv_thread(void *arg) {
    int client_fd = *(int *)arg;
    char recv_buf[1024];
    while (1) {
        memset(recv_buf, 0, sizeof(recv_buf));
        int recv_len = recv(client_fd, recv_buf, sizeof(recv_buf) - 1, 0);
        if (recv_len > 0) {
            printf("服务器: %s", recv_buf);
            fflush(stdout);
        } else if (recv_len == 0) {
            // 服务器关闭
            printf("\n连接已关闭\n");
            break;
        } else {
            perror("recv");
            break;
        }
    }
    // 关闭读
    shutdown(client_fd, SHUT_RD);
    return NULL;
}

int main(int argc, char const *argv[]){
    // TCP客户端编程:
    // 目标1：实现多线程处理，支持同时发送和接收消息
    // 目标2：实现多客户端连接，为每个客户端创建线程

    // 支持参数指定数量的并发客户端，如果不指定默认1个
    int client_count = 1;
    if (argc >= 2) {
        client_count = atoi(argv[1]);
        if (client_count <= 0) client_count = 1;
    }

    // 定义客户端线程主函数
    void* client_worker(void* arg) {
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        handle_error("socket", client_fd);

        struct sockaddr_in server_addr;
        socklen_t server_addr_len = sizeof(server_addr);
        memset(&server_addr, 0, server_addr_len);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(8888);
        int connect_result = connect(client_fd,(struct sockaddr *)&server_addr,server_addr_len);
        handle_error("connect", connect_result);

        printf("客户端线程[%lu] 已连接到服务器 127.0.0.1:8888\n", pthread_self());
        printf("请输入消息（输入 quit 退出）:\n");

        pthread_t tid_send, tid_recv;
        pthread_create(&tid_send, NULL, send_thread, &client_fd);
        pthread_create(&tid_recv, NULL, recv_thread, &client_fd);

        pthread_join(tid_send, NULL);
        pthread_join(tid_recv, NULL);

        close(client_fd);
        return NULL;
    }

    // 启动多个客户端线程
    pthread_t *client_threads = malloc(sizeof(pthread_t) * client_count);
    for (int i = 0; i < client_count; ++i) {
        pthread_create(&client_threads[i], NULL, client_worker, NULL);
    }
    for (int i = 0; i < client_count; ++i) {
        pthread_join(client_threads[i], NULL);
    }
    free(client_threads);


    // 第二步：连接服务器
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    memset(&server_addr,0,server_addr_len);
    server_addr.sin_family = AF_INET; // 地址族,AF_INET-->IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 填写IP地址,INADDR_ANY-->0.0.0.0
    server_addr.sin_port = htons(8888); // 填写端口号,htons(8888)-->8888
    int connect_result = connect(client_fd,(struct sockaddr *)&server_addr,server_addr_len);
    handle_error("connect", connect_result);
    printf("已连接到服务器 127.0.0.1:8888\n");
    printf("请输入消息（输入 quit 退出）:\n");

    // 启动发送和接收线程
    pthread_t tid_send, tid_recv;
    pthread_create(&tid_send, NULL, send_thread, &client_fd);
    pthread_create(&tid_recv, NULL, recv_thread, &client_fd);

    // 等待线程结束
    pthread_join(tid_send, NULL);
    pthread_join(tid_recv, NULL);

    close(client_fd);
    return 0;
}