# UDP 开发函数

## 概述

UDP 协议接收和发送数据不再使用 `send` 和 `recv` 方法，这两个方法一般用于 TCP 通信。UDP 通信使用 `sendto` 和 `recvfrom` 方法。

## TCP vs UDP 通信方式对比

### TCP 通信
- **连接方式**：面向连接的协议，需要先建立连接（connect）
- **发送函数**：`send()` / `write()`
- **接收函数**：`recv()` / `read()`
- **特点**：每次通信都通过已建立的连接，不需要指定目标地址

### UDP 通信
- **连接方式**：无连接的协议，不需要建立连接
- **发送函数**：`sendto()` - 需要指定目标地址
- **接收函数**：`recvfrom()` - 可以获取发送方地址
- **特点**：每次通信都需要指定目标地址，可以实现一对一、一对多、多对一、多对多通信

## UDP 核心函数

### 1. sendto() - 发送数据

#### 函数原型
```c
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

#### 参数说明
- **sockfd**：UDP 套接字文件描述符
- **buf**：要发送的数据缓冲区
- **len**：要发送的数据长度（字节数）
- **flags**：标志位，通常设为 0
  - `0`：默认方式
  - `MSG_DONTWAIT`：非阻塞发送
  - `MSG_NOSIGNAL`：不发送 SIGPIPE 信号
- **dest_addr**：目标地址结构体指针（`struct sockaddr_in` 或 `struct sockaddr_in6`）
- **addrlen**：目标地址结构体的长度（`sizeof(struct sockaddr_in)`）

#### 返回值
- **成功**：返回实际发送的字节数
- **失败**：返回 -1，并设置 `errno`

#### 使用示例
```c
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8888);
inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

char *msg = "Hello UDP Server";
ssize_t send_len = sendto(sockfd, msg, strlen(msg), 0,
                          (struct sockaddr *)&server_addr, sizeof(server_addr));
if (send_len < 0) {
    perror("sendto");
}
```

### 2. recvfrom() - 接收数据

#### 函数原型
```c
#include <sys/socket.h>

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

#### 参数说明
- **sockfd**：UDP 套接字文件描述符
- **buf**：接收数据的缓冲区
- **len**：缓冲区的大小（字节数）
- **flags**：标志位，通常设为 0
  - `0`：默认方式
  - `MSG_DONTWAIT`：非阻塞接收
  - `MSG_PEEK`：查看数据但不从缓冲区移除
- **src_addr**：用于保存发送方地址的结构体指针（可以为 NULL）
- **addrlen**：输入输出参数
  - **输入**：`src_addr` 缓冲区的大小
  - **输出**：实际接收到的地址长度

#### 返回值
- **成功**：返回实际接收的字节数
- **失败**：返回 -1，并设置 `errno`
- **连接关闭**：返回 0（UDP 通常不会返回 0，除非套接字被关闭）

#### 使用示例
```c
char recv_buf[1024];
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);

ssize_t recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf) - 1, 0,
                            (struct sockaddr *)&client_addr, &client_len);
if (recv_len > 0) {
    recv_buf[recv_len] = '\0';  // 添加字符串结束符
    printf("收到来自 %s:%d 的消息: %s\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           recv_buf);
} else if (recv_len < 0) {
    perror("recvfrom");
}
```

## UDP 编程流程

### 服务器端流程
1. 创建套接字：`socket(AF_INET, SOCK_DGRAM, 0)`
2. 绑定地址：`bind(sockfd, &server_addr, sizeof(server_addr))`
3. 接收数据：`recvfrom()` - 可获取客户端地址
4. 发送数据：`sendto()` - 使用接收到的客户端地址
5. 关闭套接字：`close(sockfd)`

### 客户端流程
1. 创建套接字：`socket(AF_INET, SOCK_DGRAM, 0)`
2. （可选）绑定地址：`bind()` - 通常客户端不需要绑定
3. 发送数据：`sendto()` - 指定服务器地址
4. 接收数据：`recvfrom()` - 可获取服务器地址
5. 关闭套接字：`close(sockfd)`

## 完整示例代码

### UDP 服务器示例
```c
#include "common.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("UDP 服务器启动，监听端口 8888\n");

    char recv_buf[1024];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        ssize_t recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf) - 1, 0,
                                    (struct sockaddr *)&client_addr, &client_len);
        if (recv_len > 0) {
            recv_buf[recv_len] = '\0';
            printf("收到来自 %s:%d 的消息: %s\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port),
                   recv_buf);

            // 回复消息
            const char *reply = "收到消息";
            sendto(sockfd, reply, strlen(reply), 0,
                   (struct sockaddr *)&client_addr, client_len);
        }
    }

    close(sockfd);
    return 0;
}
```

### UDP 客户端示例
```c
#include "common.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    char send_buf[1024];
    char recv_buf[1024];

    while (fgets(send_buf, sizeof(send_buf), stdin) != NULL) {
        // 发送数据
        ssize_t send_len = sendto(sockfd, send_buf, strlen(send_buf), 0,
                                  (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (send_len < 0) {
            perror("sendto");
            continue;
        }

        // 接收回复
        struct sockaddr_in reply_addr;
        socklen_t reply_len = sizeof(reply_addr);
        ssize_t recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf) - 1, 0,
                                    (struct sockaddr *)&reply_addr, &reply_len);
        if (recv_len > 0) {
            recv_buf[recv_len] = '\0';
            printf("服务器回复: %s\n", recv_buf);
        }
    }

    close(sockfd);
    return 0;
}
```

## 注意事项

1. **数据包大小**：UDP 数据包最大理论大小为 65507 字节（65535 - 8 字节 UDP 头 - 20 字节 IP 头），但实际建议不超过 1472 字节（1500 字节 MTU - 20 字节 IP 头 - 8 字节 UDP 头）

2. **数据丢失**：UDP 不保证数据可靠传输，数据可能丢失、重复或乱序

3. **地址结构**：`recvfrom()` 和 `sendto()` 中的地址结构体长度参数需要正确处理

4. **阻塞与非阻塞**：默认情况下，`recvfrom()` 会阻塞等待数据到达，可以使用 `MSG_DONTWAIT` 标志或设置套接字为非阻塞模式

5. **广播和多播**：UDP 支持广播和多播通信，可以实现一对多通信

6. **错误处理**：需要检查 `sendto()` 和 `recvfrom()` 的返回值，并根据 `errno` 进行适当的错误处理

## 相关函数

- `socket()` - 创建套接字
- `bind()` - 绑定地址
- `close()` - 关闭套接字
- `setsockopt()` - 设置套接字选项（如广播、多播等）
- `getsockopt()` - 获取套接字选项
