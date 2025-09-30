#include "common.h"

int main(int argc, char const *argv[])
{
    /* code */
    // fork 之前（父进程）
    int fd = open("io.txt",O_CREAT | O_WRONLY | O_APPEND,0644);
    if(fd == -1){
        perror("open");
        exit(1);
    }
    char buffer[1024]; //缓冲区存放写出数据

    pid_t pid = fork();
    if(pid < 0){
        perror("fork");
        exit(1);
    }else if(pid == 0){
        // 子进程
        strcpy(buffer,"this child process\n");

    }else{
        // 父进程
        sleep(1); //让父进程睡眠1秒，确保子进程先运行
        strcpy(buffer,"this father process\n");
    }

    // 父子进程都要执行的代码
    ssize_t bytes_writes = write(fd,buffer,strlen(buffer));
    if(bytes_writes == -1){
        perror("write");
        close(fd);
        exit(1);
    }

    close(fd);//写入完毕，关闭文件

    if(pid == 0){
        printf("child process write successfully\n");
    }else{
        printf("father process write successfully\n");
    }
    return 0;
}
