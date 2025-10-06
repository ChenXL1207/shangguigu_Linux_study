/*
 * Linux signal test
    Linux系统中有多种信号，每种信号都用一个唯一的整数值来表示，常见的信号包括如下
    1. SIGINT(2): 中断信号，通常由用户通过键盘输入^C来发送
    2. SIGKILL(9): 强制终止信号，通常由用户通过键盘输入^C来发送
    3. SIGTERM(15): 终止信号，通常由用户通过键盘输入^C来发送
    4. SIGSEGV(11): 段错误信号，通常由程序非法访问内存时发送
    5. SIGALRM(14): 定时器信号，通常由程序设置定时器时发送
    6. SIGUSR1(10): 用户自定义信号，通常由程序发送
    7. SIGUSR2(12): 用户自定义信号，通常由程序发送
    8. SIGCHLD(17): 子进程状态改变信号，通常由子进程状态改变时发送
    9. SIGCONT(19): 继续执行信号，通常由程序继续执行时发送
    10. SIGSTOP(19): 停止执行信号，通常由程序停止执行时发送
*/

/*
    注册信号处理函数逻辑
    // 信号处理函数声明
    typedef void (*signal_handler_t)(int);
    @param signum 信号编号
    @param handler 信号处理函数
    @return 返回值为SIG_ERR表示注册失败，否则返回注册前的信号处理函数
    signal_handler_t signal(int signum, signal_handler_t handler);
*/

#include "common.h"

void signal_handler(int signum){
    printf("I am signal handler, my signum is %d\n",signum);
    exit(signum);
}

int main(int argc, char *argv[])
{
    if(signal(SIGINT, signal_handler) == SIG_ERR){
        perror("signal");
        return 1;
    }

    while(1){
        sleep(1);
        printf("I am main process, my pid is %d\n",getpid());
    }
    return 0;
}