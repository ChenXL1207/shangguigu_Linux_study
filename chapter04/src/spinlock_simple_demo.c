// 自旋锁简单示例 - 理解基本概念
#include "common.h"
#include <unistd.h>

#define THREAD_COUNT 3
#define WORK_COUNT 5

// 全局共享变量
int shared_data = 0;

// 自旋锁
pthread_spinlock_t spinlock;

void *worker_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    printf("线程 %d 开始工作\n", thread_id);
    
    for (int i = 0; i < WORK_COUNT; i++) {
        printf("线程 %d 尝试获取自旋锁...\n", thread_id);
        
        // 获取自旋锁 - 如果锁被占用，线程会"自旋"等待
        pthread_spin_lock(&spinlock);
        
        printf("✓ 线程 %d 获得自旋锁，开始临界区操作\n", thread_id);
        
        // 临界区：模拟一些工作
        int old_value = shared_data;
        usleep(100000);  // 100ms延时，模拟工作
        shared_data = old_value + 1;
        
        printf("✓ 线程 %d 完成工作，数据更新为: %d\n", thread_id, shared_data);
        
        // 释放自旋锁
        pthread_spin_unlock(&spinlock);
        
        printf("线程 %d 释放自旋锁\n", thread_id);
        
        // 线程休息一下
        usleep(200000);  // 200ms延时
    }
    
    printf("线程 %d 完成所有工作\n", thread_id);
    return NULL;
}

int main() {
    printf("=== 自旋锁基本概念演示 ===\n");
    printf("自旋锁特点:\n");
    printf("1. 线程等待锁时不会睡眠，而是不断检查锁状态\n");
    printf("2. 适用于锁持有时间很短的场景\n");
    printf("3. 在多核CPU上效果更好\n");
    printf("4. 会消耗CPU资源（因为不断检查）\n\n");
    
    // 初始化自旋锁
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    
    // 初始化线程ID
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
    }
    
    printf("创建 %d 个线程，每个线程执行 %d 次操作\n", THREAD_COUNT, WORK_COUNT);
    printf("初始数据: %d\n\n", shared_data);
    
    // 创建线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== 执行结果 ===\n");
    printf("最终数据: %d\n", shared_data);
    printf("期望结果: %d\n", THREAD_COUNT * WORK_COUNT);
    
    if (shared_data == THREAD_COUNT * WORK_COUNT) {
        printf("✓ 数据一致性正确！\n");
    } else {
        printf("✗ 数据不一致！\n");
    }
    
    // 销毁自旋锁
    pthread_spin_destroy(&spinlock);
    
    printf("\n=== 自旋锁 vs 互斥锁 ===\n");
    printf("自旋锁: 忙等待，不睡眠，适合短时间持有锁\n");
    printf("互斥锁: 睡眠等待，适合长时间持有锁\n");
    
    return 0;
}
