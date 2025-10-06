/*
    线程终止
    （1） pthread_exit
    主要由三类方法：
      1. 线程函数执行return语句
      2. 线程函数内部调用执行pthread_exit(推荐使用)
      3. 其他线程调用pthread_cancel函数终止线程

    pthread_exit函数：
    void pthread_exit(void *retval);
        @param retval: 线程退出时返回的值，一般设置为NULL
        @return: 无
        功能：线程退出函数，线程退出后，线程占用的资源会被释放

    pthread_join函数：(相当于等待结果回收)
    int pthread_join(pthread_t thread, void **retval);
        @param thread: 要等待的线程ID
        @param retval: 用于接收线程返回值的指针，如果不需要返回值可以设置为NULL
        @return: 0成功，非0失败
        功能：阻塞调用线程，直到指定的线程结束。类似于进程中的wait()函数
        注意：每个线程只能被join一次，多次join会导致未定义行为

    phread_detach函数：（相当于自动回收）
    int pthread_detach(pthread_t thread);
        @param thread: 要分离的线程ID
        @return: 0成功，非0失败
        功能：分离线程，线程退出后，线程占用的资源会被释放
        注意：每个线程只能被detach一次，多次detach会导致未定义行为

    pthread_cancel函数：
    int pthread_cancel(pthread_t thread);
        @param thread: 要取消的线程ID
        @return: 0成功，非0失败
        功能：取消线程，线程退出后，线程占用的资源会被释放
        注意：每个线程只能被cancel一次，多次cancel会导致未定义行为

*/

#include "common.h"

void *thread_func(void *arg){
    printf("子线程开始运行...\n");
    sleep(2);
    printf("子线程即将退出...\n");
    // 使用pthread_exit退出线程
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    printf("主线程开始运行...\n");
    
    // 创建线程
    pthread_t pid;
    pthread_create(&pid, NULL, thread_func, NULL);
    
    // 等待子线程结束
    pthread_join(pid, NULL);
    
    printf("主线程结束运行...\n");
    return 0;
}