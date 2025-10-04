#include "common.h"

int num = 0;

/*
    进程之间的内存是隔离的，得使用进程通信，方法如下：
    1. 管道（匿名管道和命名管道）
    2. 消息队列
    3. 共享内存
    4. 信号量
    5. 套接字（Unix Domain Socket IPC）
*/

int main(int argc, char const *argv[])
{
    int subprocess_status;
    /* code */
    pid_t pid = fork();
    if(pid < 0){
        perror("fork");
        exit(1);
    }else if(pid == 0){
        // 子进程
        num = 1;
        printf("I am child process, my pid is %d, my parent pid is %d, my num is %d\n",getpid(),getppid(),num);
    }else{
        waitpid(pid,&subprocess_status,0);
        // 父进程
        printf("I am parent process, my pid is %d, my child pid is %d, my num is %d\n",getpid(),pid,num);
    }
    return 0;
}