#include "common.h"
/*
    fifo 有名管道
    1. 有名管道是linux系统中的一种特殊文件，用于进程间通信
    2. 有名管道是一种半双工的通信方式，即数据只能单向流动

    mkfifo 创建有名管道
    int mkfifo(const char *pathname, mode_t mode)
    @pathname: 管道文件名
    @mode: 管道文件的权限
    @return: 成功返回0，失败返回-1
*/

int main(int argc, char *argv[]){
    char *fifo_pipe_path = "/tmp/fifo_pipe";
    int fd;

    // if(mkfifo(fifo_pipe_path, 0666) != 0){
    //     perror("mkfifo");
    //     exit(EXIT_FAILURE);
    // }

    // 对有名管道进行读写操作
    fd = open(fifo_pipe_path,O_RDONLY);

    if(fd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 对有名管道进行读操作
    char buffer[1024];
    ssize_t read_num;
    
    printf("等待管道数据...\n");
    
    while(1){
        // 读取管道数据，写到控制台
        read_num = read(fd, buffer, sizeof(buffer));
        if(read_num > 0){
            printf("接收到数据: ");
            write(STDOUT_FILENO, buffer, read_num);
            printf(" (长度: %zd)\n", read_num);
        }else if(read_num == 0){
            printf("管道文件被关闭\n");
            break;
        }else{
            perror("read");
            exit(EXIT_FAILURE);
        }
    }
    close(fd);
    // 释放管道文件
    if(unlink(fifo_pipe_path)==-1){
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}