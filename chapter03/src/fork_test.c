#include "common.h"

int main(int argc, char const *argv[])
{
    // 使用fork创建子线程
    // 调用fork之前，代码都在父进程种运行
    printf("ChenXL:befor fork(father process),id:%d\n",getpid());

    // 使用fork创建子线程
    /*
        extern __pid_t fork (void) __THROWNL;
        return : -1 失败
                 0 子进程
                 >0 父进程，返回值是子进程ID
    */
    pid_t pid = fork(); //一定要设置变量接收返回值
    // printf("ChenXL:after fork(id:%d)\n",getpid());
    // printf("Fork result:%d\n",pid);

    if(pid < 0){
        printf("old process fork new process faild\n");
        exit(1);
    }else if (pid == 0){
        // 执行单独子进程代码（子进程）
        printf("I am child process, my id is %d\n",getpid());
    }else{
        // 执行父进程代码（父进程）
        printf("I am father process, my id is %d, my child process id is %d\n",getpid(),pid);
    }
    

    return 0;
}
