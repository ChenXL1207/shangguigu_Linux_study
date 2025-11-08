#include "common.h"

#define PORT 8888
#define SERVER_IP "127.0.0.1"

int main(int argc, char const *argv[])
{
    int socketfd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    ssize_t send_len, recv_len;

    // 初始化服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 创建套接字
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 连接服务器
    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    printf("已连接到守护进程服务器 %s:%d\n", SERVER_IP, PORT);
    printf("输入 'quit' 退出，输入 'EOF' 退出\n");

    // 循环发送和接收消息
    while (1) {
        printf("请输入消息: ");
        fflush(stdout);

        // 从标准输入读取
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        // 移除换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        // 检查退出命令
        if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "EOF") == 0) {
            printf("退出客户端\n");
            break;
        }

        if (len == 0) {
            continue;
        }

        // 发送消息
        send_len = send(socketfd, buffer, len, 0);
        if (send_len < 0) {
            perror("send");
            break;
        }
        printf("发送: %s (%ld 字节)\n", buffer, send_len);

        // 接收回复
        memset(buffer, 0, sizeof(buffer));
        recv_len = recv(socketfd, buffer, sizeof(buffer) - 1, 0);
        if (recv_len < 0) {
            perror("recv");
            break;
        } else if (recv_len == 0) {
            printf("服务器关闭连接\n");
            break;
        } else {
            buffer[recv_len] = '\0';
            printf("接收: %s", buffer);
        }
    }

    close(socketfd);
    return 0;
}

