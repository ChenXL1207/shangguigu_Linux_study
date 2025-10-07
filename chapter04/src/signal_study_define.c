/*
    信号量
    基本概念：信号量本质是一个非负整数，用于进程间同步，主要两种目的：互斥和同步
    互斥：用于保护共享资源，确保同一时间只有一个进程可以访问共享资源
    同步：用于进程间的同步，确保进程按照一定的顺序执行

    信号量操作：
    1. 初始化信号量：sem_init
    2. 等待信号量：sem_wait
    3. 发送信号量：sem_post
    4. 销毁信号量：sem_destroy

    理解：信号量是用于进程间同步的，用于保护共享资源，确保同一时间只有一个进程可以访问共享资源

    sem_init: 初始化信号量
    int sem_init(sem_t *sem, int pshared, unsigned int value);
    参数：
        @param sem: 信号量
        @param pshared: 共享方式,0表示线程间共享，1表示进程间共享
        @param value: 信号量初始值,1表示信号量初始值为1
    返回：
        @return: 成功返回0，失败返回-1
    
    sem_wait: 等待信号量
    int sem_wait(sem_t *sem);
    参数：
        @param sem: 信号量
    返回：
        @return: 成功返回0，失败返回-1
    
    sem_post: 发送信号量
    int sem_post(sem_t *sem);
    参数：
        @param sem: 信号量
    返回：
        @return: 成功返回0，失败返回-1
    
    sem_destroy: 销毁信号量
    int sem_destroy(sem_t *sem);
    参数：
        @param sem: 信号量
    返回：
        @return: 成功返回0，失败返回-1
*/

#include "common.h"
#include <semaphore.h>
#define THREAD_COUNT 10000
int shared_data = 0;
sem_t unamed_sem;

void *plusOne(void *arg){
    sem_wait(&unamed_sem); // 等待信号量
    int tmp = shared_data + 1;
    shared_data = tmp;
    sem_post(&unamed_sem); // 释放信号量
    return NULL;
}

int main(int argc, char const *argv[])
{
    // 初始化信号量，初始值为1（互斥信号量）
    sem_init(&unamed_sem, 0, 1);
    
    printf("=== 信号量互斥示例 ===\n");
    printf("创建 %d 个线程，每个线程对共享数据加1\n", THREAD_COUNT);
    
    pthread_t threads[THREAD_COUNT];
    for(ssize_t i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threads[i],NULL,plusOne,NULL);
    }
    for(ssize_t i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i],NULL);
    }
    
    printf("最终结果: shared_data = %d\n", shared_data);
    printf("预期结果: %d\n", THREAD_COUNT);
    
    // 销毁信号量
    sem_destroy(&unamed_sem);
    return 0;
}