#include "common.h"
#include <sys/epoll.h>  // epoll 系统调用
#include <fcntl.h>      // fcntl 函数

#define PORT 8888
#define MAX_EVENTS 10
#define BUF_SIZE 1024

int main() {
    int server_fd, client_fd, epoll_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    struct epoll_event ev, events[MAX_EVENTS];
    char buf[BUF_SIZE];

    // 1. 创建服务器套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    handle_error("socket", server_fd);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int res = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    handle_error("bind", res);

    res = listen(server_fd, 10);
    handle_error("listen", res);

    // 2. 创建epoll实例
    epoll_fd = epoll_create1(0);
    handle_error("epoll_create1", epoll_fd);

    // 3. 添加监听套接字到epoll
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    handle_error("epoll_ctl add listen", res);

    printf("epoll服务器启动, 监听端口: %d\n", PORT);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        handle_error("epoll_wait", nfds);

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                // 有新客户端连接
                client_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                printf("新客户端连接: %s:%d, fd=%d\n",
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port),
                    client_fd);

                // 设置非阻塞
                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                // 添加客户端fd到epoll
                ev.events = EPOLLIN | EPOLLET; // 边缘触发
                ev.data.fd = client_fd;
                res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                if (res < 0) {
                    perror("epoll_ctl add client");
                    close(client_fd);
                }
            } else {
                // 客户端有数据到来
                int client = events[i].data.fd;
                while (1) {
                    ssize_t len = recv(client, buf, sizeof(buf) - 1, 0);
                    if (len == 0) {
                        printf("客户端关闭: fd=%d\n", client);
                        close(client);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client, NULL);
                        break;
                    } else if (len < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 已经读取完
                            break;
                        } else {
                            perror("recv");
                            close(client);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client, NULL);
                            break;
                        }
                    } else {
                        buf[len] = '\0';
                        printf("收到fd=%d数据: %s\n", client, buf);
                        // 回显
                        send(client, buf, len, 0);
                    }
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
