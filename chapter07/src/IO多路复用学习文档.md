# IO 多路复用学习文档

## 目录

1. [什么是 IO 多路复用](#什么是-io-多路复用)
2. [为什么需要 IO 多路复用](#为什么需要-io-多路复用)
3. [IO 模型对比](#io-模型对比)
4. [select 系统调用](#select-系统调用)
5. [poll 系统调用](#poll-系统调用)
6. [epoll 系统调用](#epoll-系统调用)
7. [三种方式对比](#三种方式对比)
8. [实际应用示例](#实际应用示例)
9. [最佳实践](#最佳实践)
10. [常见问题](#常见问题)

## 什么是 IO 多路复用

**IO 多路复用（I/O Multiplexing）** 是一种机制，允许一个进程同时监视多个文件描述符（通常是网络套接字），当其中任何一个文件描述符就绪（可读、可写或出现异常）时，进程就能立即感知并处理。

### 核心概念

- **多路**：多个文件描述符（套接字）
- **复用**：一个进程/线程处理多个连接
- **事件驱动**：基于事件的就绪通知机制

### 基本思想

传统的阻塞 IO 模型中，一个进程只能处理一个连接。如果要处理多个连接，需要创建多个进程或线程，这会消耗大量系统资源。

IO 多路复用允许一个进程同时监视多个文件描述符，当某个文件描述符就绪时，进程可以立即处理它，然后继续监视其他文件描述符。

## 为什么需要 IO 多路复用

### 传统方式的局限

#### 1. 多进程方式

```c
// 为每个客户端创建一个进程
while (1) {
    int clientfd = accept(serverfd, ...);
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程处理客户端
        handle_client(clientfd);
        exit(0);
    }
}
```

**缺点：**
- 进程创建和切换开销大
- 内存消耗大（每个进程独立地址空间）
- 进程间通信复杂
- 受系统进程数限制

#### 2. 多线程方式

```c
// 为每个客户端创建一个线程
while (1) {
    int clientfd = accept(serverfd, ...);
    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, &clientfd);
}
```

**缺点：**
- 线程创建和切换开销
- 内存消耗（每个线程有独立的栈空间）
- 线程同步问题
- 受系统线程数限制

#### 3. 非阻塞 IO + 轮询

```c
// 非阻塞 IO + 忙等待
while (1) {
    for (int i = 0; i < max_clients; i++) {
        if (FD_ISSET(clients[i], &readfds)) {
            // 处理客户端
        }
    }
}
```

**缺点：**
- CPU 占用率高（忙等待）
- 响应延迟（轮询间隔）
- 资源浪费

### IO 多路复用的优势

1. **高效**：一个进程/线程处理多个连接
2. **资源节约**：不需要为每个连接创建进程/线程
3. **响应及时**：基于事件通知，无需轮询
4. **可扩展**：可以处理大量并发连接

## IO 模型对比

### 1. 阻塞 IO（Blocking IO）

```c
// 阻塞等待数据到达
char buffer[1024];
int n = read(fd, buffer, sizeof(buffer));  // 阻塞
```

**特点：**
- 进程阻塞直到数据到达
- 简单易用
- 一个进程只能处理一个连接

### 2. 非阻塞 IO（Non-blocking IO）

```c
// 设置非阻塞
fcntl(fd, F_SETFL, O_NONBLOCK);

// 循环检查
while (1) {
    int n = read(fd, buffer, sizeof(buffer));
    if (n > 0) {
        // 处理数据
        break;
    }
    // 继续其他工作
}
```

**特点：**
- 不阻塞，立即返回
- 需要轮询检查
- CPU 占用率高

### 3. IO 多路复用（IO Multiplexing）

```c
// 使用 select/poll/epoll 监视多个文件描述符
select(maxfd + 1, &readfds, NULL, NULL, NULL);
```

**特点：**
- 一个进程监视多个文件描述符
- 基于事件通知
- 高效处理大量连接

### 4. 异步 IO（Asynchronous IO）

```c
// 异步 IO（Linux 中的 aio）
aio_read(&aiocb);
// 立即返回，数据准备好后通过信号或回调通知
```

**特点：**
- 完全异步
- 需要操作系统支持
- 实现复杂

## select 系统调用

### 函数原型

```c
#include <sys/select.h>

int select(int nfds, 
           fd_set *readfds, 
           fd_set *writefds, 
           fd_set *exceptfds, 
           struct timeval *timeout);
```

### 参数说明

- **nfds**：最大的文件描述符值 + 1
- **readfds**：可读文件描述符集合
- **writefds**：可写文件描述符集合
- **exceptfds**：异常文件描述符集合
- **timeout**：超时时间（NULL 表示永久等待）

### 返回值

- **成功**：返回就绪的文件描述符数量
- **失败**：返回 -1，设置 errno
- **超时**：返回 0

### 相关宏

```c
// 清空集合
void FD_ZERO(fd_set *set);

// 添加文件描述符到集合
void FD_SET(int fd, fd_set *set);

// 从集合中移除文件描述符
void FD_CLR(int fd, fd_set *set);

// 检查文件描述符是否在集合中
int FD_ISSET(int fd, fd_set *set);
```

### select 使用示例

```c
#include "common.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverfd, clientfd;
    int client_sockets[MAX_CLIENTS] = {0};
    fd_set readfds;
    int max_sd, activity;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // 创建服务器套接字
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 设置地址重用
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 监听
    listen(serverfd, 5);
    printf("服务器监听端口 8888\n");

    while (1) {
        // 清空文件描述符集合
        FD_ZERO(&readfds);

        // 添加服务器套接字
        FD_SET(serverfd, &readfds);
        max_sd = serverfd;

        // 添加客户端套接字
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        // 等待活动
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select");
            continue;
        }

        // 检查新的连接
        if (FD_ISSET(serverfd, &readfds)) {
            socklen_t client_len = sizeof(client_addr);
            clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_len);

            // 添加到客户端数组
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = clientfd;
                    printf("新客户端连接: %s:%d\n", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port));
                    break;
                }
            }
        }

        // 检查客户端数据
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // 客户端断开
                    printf("客户端断开连接\n");
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("收到消息: %s", buffer);
                    send(sd, "收到\n", 5, 0);
                }
            }
        }
    }

    close(serverfd);
    return 0;
}
```

### select 的缺点

1. **文件描述符数量限制**：通常限制为 1024（FD_SETSIZE）
2. **性能问题**：需要复制文件描述符集合到内核
3. **每次调用都需要重新设置**：需要重新设置文件描述符集合
4. **线性扫描**：需要遍历所有文件描述符

## poll 系统调用

### 函数原型

```c
#include <poll.h>

int poll(struct pollfd *fds, 
         nfds_t nfds, 
         int timeout);
```

### 参数说明

- **fds**：pollfd 结构体数组
- **nfds**：数组元素个数
- **timeout**：超时时间（毫秒，-1 表示永久等待）

### pollfd 结构体

```c
struct pollfd {
    int fd;         // 文件描述符
    short events;   // 要监视的事件
    short revents;  // 返回的事件
};
```

### 事件标志

- **POLLIN**：可读
- **POLLOUT**：可写
- **POLLERR**：错误
- **POLLHUP**：挂起
- **POLLNVAL**：无效文件描述符

### poll 使用示例

```c
#include "common.h"
#include <poll.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverfd, clientfd;
    struct pollfd fds[MAX_CLIENTS + 1];
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int nfds = 1;  // 当前文件描述符数量

    // 创建服务器套接字
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 监听
    listen(serverfd, 5);
    printf("服务器监听端口 8888\n");

    // 初始化 pollfd 数组
    fds[0].fd = serverfd;
    fds[0].events = POLLIN;
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
        fds[i].fd = -1;
    }

    while (1) {
        // 等待事件
        int activity = poll(fds, nfds, -1);

        if (activity < 0) {
            perror("poll");
            continue;
        }

        // 检查服务器套接字（新连接）
        if (fds[0].revents & POLLIN) {
            socklen_t client_len = sizeof(client_addr);
            clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_len);

            // 添加到 pollfd 数组
            for (int i = 1; i < MAX_CLIENTS + 1; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = clientfd;
                    fds[i].events = POLLIN;
                    if (i >= nfds) {
                        nfds = i + 1;
                    }
                    printf("新客户端连接: %s:%d\n", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port));
                    break;
                }
            }
        }

        // 检查客户端套接字
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) {
                continue;
            }

            if (fds[i].revents & POLLIN) {
                int valread = read(fds[i].fd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // 客户端断开
                    printf("客户端断开连接\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                } else {
                    buffer[valread] = '\0';
                    printf("收到消息: %s", buffer);
                    send(fds[i].fd, "收到\n", 5, 0);
                }
            }
        }
    }

    close(serverfd);
    return 0;
}
```

### poll 的优点

1. **无文件描述符数量限制**：理论上无限制（受系统资源限制）
2. **更灵活的接口**：使用结构体数组，更易管理
3. **更高效**：不需要每次重新设置文件描述符集合

### poll 的缺点

1. **仍然需要遍历**：需要遍历所有文件描述符
2. **性能问题**：大量文件描述符时性能下降
3. **每次调用都需要复制**：需要复制 pollfd 数组到内核

## epoll 系统调用

epoll 是 Linux 特有的高性能 IO 多路复用机制，适用于处理大量并发连接。

### epoll 的三个系统调用

#### 1. epoll_create / epoll_create1

```c
#include <sys/epoll.h>

int epoll_create(int size);
int epoll_create1(int flags);
```

**功能**：创建一个 epoll 实例

**参数**：
- **size**：提示内核要监视的文件描述符数量（Linux 2.6.8 后忽略）
- **flags**：EPOLL_CLOEXEC 等标志

**返回值**：epoll 文件描述符

#### 2. epoll_ctl

```c
int epoll_ctl(int epfd, 
              int op, 
              int fd, 
              struct epoll_event *event);
```

**功能**：控制 epoll 实例，添加、修改或删除文件描述符

**参数**：
- **epfd**：epoll 文件描述符
- **op**：操作类型
  - **EPOLL_CTL_ADD**：添加文件描述符
  - **EPOLL_CTL_MOD**：修改文件描述符
  - **EPOLL_CTL_DEL**：删除文件描述符
- **fd**：要操作的文件描述符
- **event**：事件结构体

#### 3. epoll_wait

```c
int epoll_wait(int epfd, 
               struct epoll_event *events, 
               int maxevents, 
               int timeout);
```

**功能**：等待事件发生

**参数**：
- **epfd**：epoll 文件描述符
- **events**：返回的事件数组
- **maxevents**：事件数组大小
- **timeout**：超时时间（毫秒，-1 表示永久等待）

### epoll_event 结构体

```c
struct epoll_event {
    uint32_t events;    // 事件类型
    epoll_data_t data;  // 用户数据
};

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
```

### 事件标志

- **EPOLLIN**：可读
- **EPOLLOUT**：可写
- **EPOLLERR**：错误
- **EPOLLHUP**：挂起
- **EPOLLET**：边缘触发模式（默认水平触发）
- **EPOLLONESHOT**：一次性事件
- **EPOLLRDHUP**：对端关闭连接

### epoll 使用示例

```c
#include "common.h"
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverfd, clientfd;
    int epollfd;
    struct epoll_event event, events[MAX_EVENTS];
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // 创建服务器套接字
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 监听
    listen(serverfd, 5);
    printf("服务器监听端口 8888\n");

    // 创建 epoll 实例
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 添加服务器套接字到 epoll
    event.events = EPOLLIN;
    event.data.fd = serverfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // 等待事件
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            continue;
        }

        // 处理事件
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == serverfd) {
                // 新连接
                socklen_t client_len = sizeof(client_addr);
                clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_len);

                // 设置为非阻塞（可选）
                int flags = fcntl(clientfd, F_GETFL, 0);
                fcntl(clientfd, F_SETFL, flags | O_NONBLOCK);

                // 添加客户端到 epoll
                event.events = EPOLLIN | EPOLLET;  // 边缘触发
                event.data.fd = clientfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) == -1) {
                    perror("epoll_ctl");
                    close(clientfd);
                } else {
                    printf("新客户端连接: %s:%d\n", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port));
                }
            } else {
                // 客户端数据
                int fd = events[i].data.fd;
                int valread = read(fd, buffer, BUFFER_SIZE);
                if (valread <= 0) {
                    // 客户端断开
                    printf("客户端断开连接\n");
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    buffer[valread] = '\0';
                    printf("收到消息: %s", buffer);
                    send(fd, "收到\n", 5, 0);
                }
            }
        }
    }

    close(epollfd);
    close(serverfd);
    return 0;
}
```

### epoll 的工作模式

#### 1. 水平触发（Level Triggered, LT）

- **默认模式**
- 只要文件描述符就绪，就会持续通知
- 类似 select/poll 的行为
- 编程简单，但可能效率较低

#### 2. 边缘触发（Edge Triggered, ET）

- **需要设置 EPOLLET 标志**
- 只在状态变化时通知一次
- 需要一次性读取所有数据
- 效率更高，但编程复杂

### epoll 的优势

1. **高性能**：不需要遍历所有文件描述符
2. **无数量限制**：理论上无限制
3. **高效的事件通知**：只返回就绪的文件描述符
4. **边缘触发模式**：更高的性能
5. **内核优化**：Linux 内核专门优化

## 三种方式对比

| 特性 | select | poll | epoll |
|------|--------|------|-------|
| **文件描述符数量** | 有限制（通常1024） | 无限制 | 无限制 |
| **性能** | O(n) 线性扫描 | O(n) 线性扫描 | O(1) 事件通知 |
| **内核实现** | 轮询 | 轮询 | 回调 |
| **跨平台** | 是 | 是 | Linux 特有 |
| **使用复杂度** | 中等 | 中等 | 较高 |
| **内存拷贝** | 每次调用都拷贝 | 每次调用都拷贝 | 内核维护 |
| **适用场景** | 少量连接 | 中等连接 | 大量连接 |

### 性能对比

- **少量连接（< 100）**：三种方式性能相近
- **中等连接（100-1000）**：poll 和 epoll 性能相近
- **大量连接（> 1000）**：epoll 性能明显优于 select 和 poll

## 实际应用示例

### 使用 epoll 的完整服务器示例

```c
#include "common.h"
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

typedef struct {
    int fd;
    char buffer[BUFFER_SIZE];
    size_t len;
} client_data_t;

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int serverfd, epollfd;
    struct epoll_event event, events[MAX_EVENTS];
    struct sockaddr_in server_addr;
    client_data_t *clients = calloc(MAX_EVENTS, sizeof(client_data_t));

    // 创建服务器套接字
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(serverfd);

    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 监听
    listen(serverfd, 128);
    printf("服务器监听端口 8888\n");

    // 创建 epoll 实例
    epollfd = epoll_create1(0);

    // 添加服务器套接字
    event.events = EPOLLIN;
    event.data.fd = serverfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event);

    while (1) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == serverfd) {
                // 处理新连接
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_len);
                    
                    if (clientfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;  // 没有更多连接
                        }
                        perror("accept");
                        continue;
                    }

                    set_nonblocking(clientfd);
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = clientfd;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);

                    printf("新客户端连接: %s:%d\n", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port));
                }
            } else {
                // 处理客户端数据
                int fd = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                
                while (1) {
                    ssize_t count = read(fd, buffer, BUFFER_SIZE);
                    if (count < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;  // 数据读取完毕
                        }
                        // 错误，关闭连接
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    } else if (count == 0) {
                        // 客户端关闭
                        printf("客户端断开连接\n");
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    } else {
                        // 处理数据
                        buffer[count] = '\0';
                        printf("收到消息: %s", buffer);
                        send(fd, "收到\n", 5, 0);
                    }
                }
            }
        }
    }

    close(epollfd);
    close(serverfd);
    free(clients);
    return 0;
}
```

## 最佳实践

### 1. 选择合适的方式

- **少量连接（< 100）**：使用 select 或 poll
- **中等连接（100-1000）**：使用 poll 或 epoll
- **大量连接（> 1000）**：使用 epoll
- **跨平台**：使用 select 或 poll
- **Linux 专用**：使用 epoll

### 2. 非阻塞 IO

在使用 epoll 的边缘触发模式时，建议使用非阻塞 IO：

```c
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
```

### 3. 边缘触发模式注意事项

- 必须一次性读取所有数据
- 使用非阻塞 IO
- 需要循环读取直到 EAGAIN

### 4. 错误处理

- 检查所有系统调用的返回值
- 处理 EINTR 错误（被信号打断）
- 处理 EAGAIN/EWOULDBLOCK（非阻塞 IO）

### 5. 资源管理

- 及时关闭不需要的文件描述符
- 从 epoll 中移除已关闭的文件描述符
- 避免文件描述符泄漏

## 常见问题

### Q1: select、poll、epoll 的区别是什么？

**A:** 
- **select**：有文件描述符数量限制，需要每次重新设置
- **poll**：无数量限制，使用结构体数组
- **epoll**：Linux 特有，性能最好，适合大量连接

### Q2: 什么时候使用边缘触发模式？

**A:** 
- 需要最高性能时
- 能够保证一次性读取所有数据
- 使用非阻塞 IO

### Q3: epoll 为什么性能更好？

**A:**
- 不需要遍历所有文件描述符
- 内核维护就绪列表
- 只返回就绪的文件描述符
- 支持边缘触发模式

### Q4: 如何处理大量并发连接？

**A:**
- 使用 epoll
- 使用边缘触发模式
- 使用非阻塞 IO
- 使用线程池处理业务逻辑

### Q5: select 的文件描述符限制如何解决？

**A:**
- 使用 poll（无限制）
- 使用 epoll（Linux）
- 修改 FD_SETSIZE（不推荐）

### Q6: 边缘触发和水平触发如何选择？

**A:**
- **水平触发**：编程简单，适合大多数场景
- **边缘触发**：性能更高，需要仔细处理

## 总结

IO 多路复用是 Linux 网络编程中的重要技术，可以高效地处理大量并发连接。选择合适的 IO 多路复用方式需要根据实际需求：

- **select**：简单易用，适合少量连接
- **poll**：无数量限制，适合中等连接
- **epoll**：高性能，适合大量连接（Linux）

掌握 IO 多路复用技术，可以编写出高性能的网络服务器程序。

## 参考资料

- `man select` - select 系统调用手册
- `man poll` - poll 系统调用手册
- `man epoll` - epoll 系统调用手册
- `man fcntl` - 文件控制函数手册
- 《UNIX 网络编程》- W. Richard Stevens
- 《Linux 高性能服务器编程》- 游双

