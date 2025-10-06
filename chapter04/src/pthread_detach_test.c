#include "common.h"


void *thread_func(void *arg){
    printf("thread start running...\n");
    sleep(1);
    printf("thread finished running...\n");
    return NULL;
}

int main(int argc, char *argv[]){
    // 创建线程
    pthread_t pid;
    pthread_create(&pid,NULL,thread_func,NULL);

    // 使用pthread_detach函数分离线程，线程完成之后完成回收相关资源工作
    pthread_detach(pid);

    // 主线程完成时间需要比子线程的运行时间要慢一点
    printf("main thread continues running...\n");
    sleep(3);//因为主线程不会在pthread_join函数中等待子线程完成。如果父进程先于子进程退出，子进程会变成僵尸进程
    printf("main thread finished running...\n");
    
    return 0;
}