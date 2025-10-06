#include "common.h"

#define THREAD_COUNT 20000

/*
    thread_create: 线程创建
    参数：
        @param thread: 线程ID
        @param attr: 线程属性，默认设置为NULL
        @param start_routine: 线程函数，线程启动后执行的函数
        @param arg: 线程函数需要传递的参数
        @return: 0成功，-1失败
*/

// 初始化互斥锁
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// 线程方法
void *add_thread_func(void *arg){
    // 转化传入参数
    int *p = (int *)arg;
    // 1. 锁定互斥锁：累加之前获取互斥锁，保证同一时间只有一个线程可以累加
    pthread_mutex_lock(&counter_mutex);
    // 累加1
    (*p)++;
    // 2. 解锁互斥锁：累加完成后释放互斥锁，允许其他线程进行累加
    pthread_mutex_unlock(&counter_mutex);
    return NULL;
}

int main(int argc, char *argv[]){
    /* code */
    pthread_t threads[THREAD_COUNT];
    int result = 0;
    for(ssize_t i = 0; i < THREAD_COUNT; i++){
        // 创建线程功能是给传入的参数累加1
        pthread_create(&threads[i],NULL,add_thread_func,&result);
    }

    // 等待所有线程结束
    for(ssize_t i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i],NULL);
    }

    // 打印结果
    printf("result = %d\n",result);
    return 0;
}