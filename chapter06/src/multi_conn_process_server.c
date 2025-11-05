#include "common.h"

void *read_from_write_thread(void *arg){
    // 从客户端读取数据并打印到控制台
    int clientfd = *(int *)arg;

    char *read_buf = NULL;
    char *write_buf = NULL;
    ssize_t recv_count = 0, send_count = 0;
    
    // 分配内存
    read_buf = (char *)malloc(1024);
    write_buf = (char *)malloc(1024);
    
    if(!read_buf){
        printf("分配读缓冲区失败\n");
        close(clientfd);
        perror("malloc");
        return NULL;
    }
    if(!write_buf){
        printf("分配写缓冲区失败\n");
        close(clientfd);
        perror("malloc");
        free(read_buf);
        return NULL;
    }
    
    memset(read_buf, 0, 1024);
    memset(write_buf, 0, 1024);
    
    while((recv_count = recv(clientfd, read_buf, 1024, 0)) > 0){
        printf("收到客户端%d的消息:%s\n",clientfd,read_buf);
        // 把收到的信息写到缓冲，回复收到
        memset(write_buf, 0, 1024);
        strcpy(write_buf, "收到\n");
        send_count = send(clientfd,write_buf,strlen(write_buf),0);
        if(send_count > 0){
            printf("回复收到%ld字节\n",send_count);
        }else{
            handle_error("send",send_count);
            break;
        }
    }
    
    if(recv_count == 0){
        printf("客户端%d断开连接\n",clientfd);
    }else if(recv_count < 0){
        printf("读取数据失败\n");
    }
    // 当客户端输入ctrl+d时，客户端断开连接
    close(clientfd);
    free(read_buf);
    free(write_buf);
    return NULL;
}
/*
SIGCHLD信号处理函数，用于回收子进程资源，避免僵尸进程
参数：
    signum：信号编号
*/
void zombie_handler(int signum){
    pid_t pid;
    int status;
    // 一个SIGCHLD信号可能对应多个子进程退出
    // 使用while循环回收所有退出的子进程，避免僵尸进程
    while((pid = waitpid(-1,&status,WNOHANG)) > 0){//WNOHANG：非阻塞模式
        if(WIFEXITED(status)){
            printf("子进程%d退出,退出码%d\n",pid,WEXITSTATUS(status));
        }else if(WIFSIGNALED(status)){
            printf("子进程%d被信号%d杀死\n",pid,WTERMSIG(status));
        }else{
            printf("子进程%d退出,退出状态%d\n",pid,status);
        }
        fflush(stdout);//刷新缓冲区，保证输出内容立即显示
    }
}


int main(int argc, char const *argv[])
{
    int socketfd,clientfd;

    // 使用 sigaction 注册 SIGCHLD，并开启 SA_RESTART 避免被打断的系统调用返回 EINTR，拓展方法
    // struct sigaction sa;
    // memset(&sa, 0, sizeof(sa));
    // sa.sa_handler = zombie_handler;
    // sigemptyset(&sa.sa_mask); // 清空信号屏蔽字
    // sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // SA_RESTART：被打断的系统调用返回 EINTR，SA_NOCLDSTOP：不停止子进程
    // sigaction(SIGCHLD, &sa, NULL); // 注册 SIGCHLD 信号处理函数,NULL：不设置信号处理函数,sa：信号处理函数
    signal(SIGCHLD,zombie_handler);
    struct sockaddr_in server_addr,client_addr;//服务器地址和客户端地址
    // 清空服务器地址和客户端地址
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    // 填写服务器地址
    server_addr.sin_family = AF_INET;//地址族,AF_INET-->IPv4
    // 填写IP地址，本地地址：0.0.0.0    
    // 方法一：server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 填写IP地址
    // 方法二：inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 将字符串转换为网络字节序的IP地址
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 填写IP地址,INADDR_ANY-->0.0.0.0
    // 填写端口号，htons(10000)-->10000
    server_addr.sin_port = htons(6666);

    // 网络编程流程
    // 1.创建套接字socket
    socketfd = socket(AF_INET,SOCK_STREAM,0);//创建套接字,AF_INET-->IPv4,SOCK_STREAM-->流式套接字,0-->默认协议
    handle_error("socket",socketfd);

    // 设置套接字选项，允许端口重用（解决 TIME_WAIT 状态导致的绑定失败问题）// 拓展知识点
    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2.绑定地址 bind
    int bind_result = bind(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("bind",bind_result);

    // 3.监听 listen
    int listen_result = listen(socketfd,10);
    handle_error("listen",listen_result);

    // 4.接受连接 accept,需要接受多个客户端连接
    socklen_t client_len = sizeof(client_addr);
    while(1){
        clientfd = accept(socketfd,(struct sockaddr *)&client_addr,&client_len);
        if(clientfd < 0){
            if(errno == EINTR){
                // 被信号打断，重试
                continue;
            }
            handle_error("accept",clientfd);
        }
        // 和每一个客户端使用一个进程交互 把客户端发送的信息打印到控制台 回复收到
        pid_t pid = fork();
        if(pid == 0){
            // 子进程
            printf("与客户端%s:%d连接成功,建立连接套接字%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),clientfd);
            close(socketfd);
            // 从客户端读取数据并打印到控制台
            read_from_write_thread(&clientfd);
            // 关闭clientfd
            close(clientfd);
            _exit(0);
        }else{
            // 父进程
            close(clientfd);
        }
    }
    printf("服务器退出\n");
    close(socketfd);
    return 0;
}