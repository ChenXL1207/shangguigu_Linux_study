#include "common.h"

/*
    pipe:在内核空间开辟一个缓冲区（位于用户区和内核区），用于进程间通信（半双工，单向），
    本质是一个先进先出的队列，两个进程都可以读写，读写进程使用文件描述符表示

    pipefd：读写文件描述符数组
        pipefd[0]：读端文件描述符
        pipefd[1]：写端文件描述符
    return：
        成功：0
        失败：-1
*/

/*
    相关宏定义
    #define EXIT_FAILURE 1
    #define EXIT_SUCCESS 0

    #define STDIN_FILENO 0
    #define STDOUT_FILENO 1
    #define STDERR_FILENO 2
*/

int main(int argc, char const *argv[])
{
    pid_t cpid;
    int pipefd[2];

    // 将程序传递进来的第一个命令行参数，通过管道传递给第二个命令行参数
    if(argc != 2){
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 创建管道
    if(pipe(pipefd) == -1){
        perror("new pipe error");
        exit(EXIT_FAILURE);
    }

    cpid = fork();

    if(cpid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }else if(cpid == 0){
        // 子进程,读取管道的数据，打印到控制台
        close(pipefd[1]);
        char buffer[1024];
        int bytes_read = read(pipefd[0],buffer,sizeof(buffer));
        buffer[bytes_read] = '\0'; // 确保字符串以null结尾
        
        char message[2048]; // 增加缓冲区大小
        snprintf(message,sizeof(message),"Child process read from pipe: %s, my pid is %d\n",buffer,getpid());
        write(STDOUT_FILENO,message,strlen(message));
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }else{
        // 父进程,写入管道数据，传递给子进程
        close(pipefd[0]);
        // 将数据写入到管道中
        write(pipefd[1],argv[1],strlen(argv[1]));
        // 等待子进程结束
        waitpid(cpid,NULL,0);
        // 关闭写端
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }
    return 0;
}