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

    if(mkfifo(fifo_pipe_path, 0666) != 0){
        perror("mkfifo");
        if(errno == EEXIST){
            printf("管道文件已存在\n");
            exit(EXIT_FAILURE);
        }
    }

    // 对有名管道进行读写操作
    fd = open(fifo_pipe_path,O_RDWR);

    if(fd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 对有名管道进行写操作
    char buffer[1024];
    ssize_t read_num;
    
    printf("请输入数据（输入 'quit' 退出）:\n");
    
    while(1){
        // 读取控制台数据，写到管道中
        read_num = read(STDIN_FILENO, buffer, sizeof(buffer));
        if(read_num > 0){
            // 检查是否是退出命令
            buffer[read_num] = '\0'; // 添加字符串结束符
            if(strncmp(buffer, "quit\n", 5) == 0){
                printf("退出写入程序\n");
                break;
            }
            
            write(fd, buffer, read_num);
            printf("写入管道成功，数据长度: %zd\n", read_num);
        }else if(read_num == 0){
            printf("标准输入被关闭\n");
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