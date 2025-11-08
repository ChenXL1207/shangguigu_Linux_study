#include <sys/socket.h> // 套接字
#include <sys/types.h> // 类型定义
#include <sys/wait.h> // 进程等待
#include <netinet/in.h> // 网络地址族
#include <arpa/inet.h> // 地址转换
#include <unistd.h> // 标准输入输出
#include <errno.h> // 错误号
#include <string.h> // 字符串操作
#include <stdio.h> // 标准输入输出
#include <stdlib.h> // 标准库
#include <pthread.h> // 线程
#include <signal.h> // 信号处理
#include <time.h>      // time, ctime
#include <fcntl.h>     // open
#include <sys/stat.h>  // umask

#define handle_error(cmd,result) \
    if(result<0){ \
        perror(cmd); \
        exit(EXIT_FAILURE); \
    }
