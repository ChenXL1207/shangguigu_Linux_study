#include "common.h"

#define THREAD_COUNT 2
#define ADD_COUNT 50000  // 每个线程累加的次数

// 全局共享变量
int sum = 0;
pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
    线程函数：对全局变量sum进行累加
    参数：
        @param arg: 线程参数（这里不使用）
    返回：
        @return: NULL
*/
void *add_thread_func(void *arg) {
    int thread_id = *(int *)arg;
    // 1. 锁定互斥锁：累加之前获取互斥锁，保证同一时间只有一个线程可以累加
    pthread_mutex_lock(&sum_mutex);

    printf("线程 %d 开始执行，将进行 %d 次累加操作\n", thread_id, ADD_COUNT);
    
    for (int i = 0; i < ADD_COUNT; i++) {
        sum++;  // 对共享变量进行累加
    }
    // 2. 解锁互斥锁：累加完成后释放互斥锁，允许其他线程进行累加
    pthread_mutex_unlock(&sum_mutex);
    printf("线程 %d 执行完毕\n", thread_id);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    
    printf("=== 多线程累加示例 ===\n");
    printf("创建 %d 个线程，每个线程对 sum 进行 %d 次累加\n", THREAD_COUNT, ADD_COUNT);
    printf("理论上最终结果应该是: %d\n", THREAD_COUNT * ADD_COUNT);
    printf("初始 sum = %d\n\n", sum);
    
    // 创建线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;  // 线程ID从1开始
        if (pthread_create(&threads[i], NULL, add_thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return -1;
        }
    }
    
    // 等待所有线程结束
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return -1;
        }
    }
    
    // 打印最终结果
    printf("\n=== 执行结果 ===\n");
    printf("最终 sum = %d\n", sum);
    printf("期望结果 = %d\n", THREAD_COUNT * ADD_COUNT);
    printf("差值 = %d\n", (THREAD_COUNT * ADD_COUNT) - sum);
    
    if (sum == THREAD_COUNT * ADD_COUNT) {
        printf("✓ 结果正确！没有发生竞态条件\n");
    } else {
        printf("✗ 结果不正确！发生了竞态条件\n");
        printf("  这是因为多个线程同时访问共享变量sum时没有同步机制\n");
    }
    
    return 0;
}
