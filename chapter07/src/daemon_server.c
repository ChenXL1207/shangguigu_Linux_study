#include "common.h"


#define DAEMON_NAME "daemon_server"
#define PID_FILE "/tmp/daemon_server.pid"
#define LOG_FILE "/tmp/daemon_server.log"
#define PORT 8888

static int keep_running = 1; // 是否继续运行
static int daemon_mode = 0;  // 0: 前台模式, 1: 守护进程模式

// 日志写入函数（支持前台和后台模式）
void write_log(const char *message) {
    time_t now = time(NULL); // 获取当前时间
    char *time_str = ctime(&now); // 将时间转换为字符串
    time_str[strlen(time_str) - 1] = '\0'; // 移除换行符
    
    char log_msg[2048]; // 日志消息
    snprintf(log_msg, sizeof(log_msg), "[%s] %s", time_str, message);
    
    if (daemon_mode) {
        // 守护进程模式：只写入日志文件
        FILE *log_fp = fopen(LOG_FILE, "a");
        if (log_fp) {
            fprintf(log_fp, "%s\n", log_msg);
            fclose(log_fp);
        }
    } else {
        // 前台模式：同时输出到控制台和日志文件
        printf("%s\n", log_msg);
        fflush(stdout);
        
        FILE *log_fp = fopen(LOG_FILE, "a");
        if (log_fp) {
            fprintf(log_fp, "%s\n", log_msg);
            fclose(log_fp);
        }
    }
}

// 控制台输出函数（仅前台模式使用）
void console_print(const char *message) {
    if (!daemon_mode) {
        printf("%s\n", message);
        fflush(stdout);
    }
}

// 信号处理函数
void signal_handler(int signum) {
    switch(signum) {
        case SIGTERM: // 终止信号
        case SIGINT: // 中断信号
            keep_running = 0;
            write_log("收到退出信号，准备退出...");
            break;
        case SIGHUP: // 重新加载配置信号
            write_log("收到SIGHUP信号，重新加载配置...");
            break;
        default:
            break; // 其他信号不处理
    }
}

// 创建守护进程
int create_daemon() {
    pid_t pid; // 子进程ID

    // 1. 第一次fork，创建子进程
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid > 0) {
        // 父进程退出，让子进程成为孤儿进程，被init进程收养
        exit(EXIT_SUCCESS);
    }

    // 2. 创建新的会话，脱离控制终端
    if (setsid() < 0) {
        perror("setsid");
        return -1;
    }

    // 3. 第二次fork，确保不是会话组长，无法重新获取控制终端
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid > 0) {
        // 父进程退出
        exit(EXIT_SUCCESS);
    }

    // 4. 改变工作目录到根目录
    if (chdir("/") < 0) {
        perror("chdir");
        return -1;
    }

    // 5. 设置文件权限掩码
    umask(0);

    // 6. 关闭文件描述符
    // 关闭标准输入、输出、错误输出
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 重新打开标准输入、输出、错误输出到 /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    return 0;
}

// 写入PID文件
int write_pid_file() {
    FILE *fp = fopen(PID_FILE, "w");
    if (fp == NULL) {
        write_log("无法创建PID文件");
        return -1;
    }
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
    return 0;
}

// 删除PID文件
void remove_pid_file() {
    unlink(PID_FILE);
}

// 处理客户端连接
void handle_client(int clientfd, struct sockaddr_in *client_addr) {
    char buffer[1024];
    ssize_t recv_len;

    memset(buffer, 0, sizeof(buffer));
    recv_len = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
    
    if (recv_len > 0) {
        buffer[recv_len] = '\0';
        
        // 前台模式：显示客户端信息
        if (!daemon_mode) {
            printf(">>> 客户端 [%s:%d] 发送消息: %s\n", 
                   inet_ntoa(client_addr->sin_addr), 
                   ntohs(client_addr->sin_port), 
                   buffer);
        }
        
        // 记录日志
        char log_msg[2048];
        snprintf(log_msg, sizeof(log_msg), "收到客户端 [%s:%d] 消息: %s", 
                 inet_ntoa(client_addr->sin_addr), 
                 ntohs(client_addr->sin_port), 
                 buffer);
        log_msg[sizeof(log_msg) - 1] = '\0';
        write_log(log_msg);

        // 回复消息
        const char *reply = "收到消息\n";
        ssize_t sent = send(clientfd, reply, strlen(reply), 0);
        if (!daemon_mode && sent > 0) {
            printf("<<< 回复客户端: %s", reply);
        }
    }
    
    close(clientfd);
}

// 显示使用帮助
void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("选项:\n");
    printf("  -d, --daemon    以守护进程模式运行（后台运行）\n");
    printf("  -p, --port PORT 指定端口号（默认: %d）\n", PORT);
    printf("  -h, --help      显示此帮助信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s              # 前台模式运行，显示交互信息\n", program_name);
    printf("  %s -d           # 守护进程模式运行\n", program_name);
    printf("  %s -p 9999      # 前台模式，使用端口9999\n", program_name);
    printf("  %s -d -p 9999   # 守护进程模式，使用端口9999\n", program_name);
}

int main(int argc, char const *argv[])
{
    int socketfd, clientfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int port = PORT;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
            daemon_mode = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[++i]);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "错误: 端口号必须在1-65535之间\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr, "错误: -p 选项需要端口号参数\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "错误: 未知选项 %s\n", argv[i]);
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // 注册信号处理函数
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);

    if (daemon_mode) {
        // 守护进程模式
        printf("正在启动守护进程...\n");
        
        // 创建守护进程
        if (create_daemon() < 0) {
            fprintf(stderr, "创建守护进程失败\n");
            exit(EXIT_FAILURE);
        }

        // 写入PID文件
        if (write_pid_file() < 0) {
            exit(EXIT_FAILURE);
        }

        write_log("守护进程启动成功");
    } else {
        // 前台模式
        printf("========================================\n");
        printf("  守护进程服务器 (前台模式)\n");
        printf("========================================\n");
        printf("端口: %d\n", port);
        printf("PID: %d\n", getpid());
        printf("日志文件: %s\n", LOG_FILE);
        printf("按 Ctrl+C 退出\n");
        printf("========================================\n\n");
        
        write_log("服务器启动（前台模式）");
    }

    // 初始化服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // 创建套接字
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        if (daemon_mode) {
            write_log("创建套接字失败");
            remove_pid_file();
        } else {
            perror("socket");
        }
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项，允许端口重用
    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        if (daemon_mode) {
            write_log("绑定地址失败");
        } else {
            perror("bind");
            printf("错误: 绑定端口 %d 失败，可能端口已被占用\n", port);
        }
        close(socketfd);
        if (daemon_mode) {
            remove_pid_file();
        }
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(socketfd, 10) < 0) {
        if (daemon_mode) {
            write_log("监听失败");
        } else {
            perror("listen");
        }
        close(socketfd);
        if (daemon_mode) {
            remove_pid_file();
        }
        exit(EXIT_FAILURE);
    }

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "服务器监听端口 %d", port);
    write_log(log_msg);
    
    if (!daemon_mode) {
        printf("✓ 服务器已启动，等待客户端连接...\n\n");
    }

    // 主循环：接受客户端连接
    client_len = sizeof(client_addr);
    while (keep_running) {
        clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
        if (clientfd < 0) {
            if (errno == EINTR) {
                // 被信号打断，继续
                continue;
            }
            if (!daemon_mode) {
                perror("accept");
            } else {
                write_log("接受连接失败");
            }
            continue;
        }

        // 记录客户端连接
        if (!daemon_mode) {
            printf("\n[新连接] 客户端 %s:%d 已连接\n", 
                   inet_ntoa(client_addr.sin_addr), 
                   ntohs(client_addr.sin_port));
        }
        
        snprintf(log_msg, sizeof(log_msg), "客户端连接: %s:%d", 
                 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        write_log(log_msg);

        // 处理客户端请求（简单处理，实际可以使用多进程或多线程）
        handle_client(clientfd, &client_addr);
        
        if (!daemon_mode) {
            printf("[连接关闭] 客户端 %s:%d 已断开\n\n", 
                   inet_ntoa(client_addr.sin_addr), 
                   ntohs(client_addr.sin_port));
        }
    }

    // 清理资源
    if (daemon_mode) {
        write_log("守护进程退出");
        remove_pid_file();
    } else {
        printf("\n服务器正在关闭...\n");
        write_log("服务器退出（前台模式）");
    }
    
    close(socketfd);

    if (!daemon_mode) {
        printf("服务器已关闭\n");
    }

    return 0;
}

