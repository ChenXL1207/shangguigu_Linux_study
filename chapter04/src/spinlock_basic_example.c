// 自旋锁基础示例
#include "common.h"
#include <unistd.h>
#include <time.h>

#define THREAD_COUNT 4
#define WORK_COUNT 1000000  // 每个线程的工作量

// 全局共享变量
int shared_counter = 0;

// 自旋锁
pthread_spinlock_t spinlock;

// 互斥锁（用于对比）
pthread_mutex_t mutex;

void *spinlock_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    printf("自旋锁线程 %d 开始工作\n", thread_id);
    
    for (int i = 0; i < WORK_COUNT; i++) {
        // 获取自旋锁
        pthread_spin_lock(&spinlock);
        
        // 临界区：对共享变量进行操作
        shared_counter++;
        
        // 释放自旋锁
        pthread_spin_unlock(&spinlock);
    }
    
    printf("自旋锁线程 %d 完成工作\n", thread_id);
    return NULL;
}

void *mutex_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    printf("互斥锁线程 %d 开始工作\n", thread_id);
    
    for (int i = 0; i < WORK_COUNT; i++) {
        // 获取互斥锁
        pthread_mutex_lock(&mutex);
        
        // 临界区：对共享变量进行操作
        shared_counter++;
        
        // 释放互斥锁
        pthread_mutex_unlock(&mutex);
    }
    
    printf("互斥锁线程 %d 完成工作\n", thread_id);
    return NULL;
}

// 测试自旋锁性能
void test_spinlock_performance() {
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    struct timespec start, end;
    double duration;
    
    printf("\n=== 自旋锁性能测试 ===\n");
    
    // 初始化自旋锁
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    
    // 重置计数器
    shared_counter = 0;
    
    // 记录开始时间
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // 创建线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, spinlock_worker, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 记录结束时间
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // 计算耗时
    duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("自旋锁测试结果:\n");
    printf("  线程数: %d\n", THREAD_COUNT);
    printf("  每线程工作量: %d\n", WORK_COUNT);
    printf("  总工作量: %d\n", THREAD_COUNT * WORK_COUNT);
    printf("  实际计数: %d\n", shared_counter);
    printf("  耗时: %.6f 秒\n", duration);
    printf("  吞吐量: %.0f 操作/秒\n", (THREAD_COUNT * WORK_COUNT) / duration);
    
    // 销毁自旋锁
    pthread_spin_destroy(&spinlock);
}

// 测试互斥锁性能
void test_mutex_performance() {
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    struct timespec start, end;
    double duration;
    
    printf("\n=== 互斥锁性能测试 ===\n");
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);
    
    // 重置计数器
    shared_counter = 0;
    
    // 记录开始时间
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // 创建线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, mutex_worker, &thread_ids[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 记录结束时间
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // 计算耗时
    duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("互斥锁测试结果:\n");
    printf("  线程数: %d\n", THREAD_COUNT);
    printf("  每线程工作量: %d\n", WORK_COUNT);
    printf("  总工作量: %d\n", THREAD_COUNT * WORK_COUNT);
    printf("  实际计数: %d\n", shared_counter);
    printf("  耗时: %.6f 秒\n", duration);
    printf("  吞吐量: %.0f 操作/秒\n", (THREAD_COUNT * WORK_COUNT) / duration);
    
    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);
}

int main() {
    printf("=== 自旋锁 vs 互斥锁 性能对比测试 ===\n");
    printf("测试场景: %d 个线程，每个线程执行 %d 次加锁/解锁操作\n", THREAD_COUNT, WORK_COUNT);
    
    // 测试自旋锁性能
    test_spinlock_performance();
    
    // 测试互斥锁性能
    test_mutex_performance();
    
    printf("\n=== 自旋锁特点说明 ===\n");
    printf("1. 自旋锁: 线程在等待锁时不会睡眠，而是不断检查锁的状态\n");
    printf("2. 互斥锁: 线程在等待锁时会睡眠，让出CPU给其他线程\n");
    printf("3. 自旋锁适用: 锁持有时间短，多核CPU环境\n");
    printf("4. 互斥锁适用: 锁持有时间长，单核或多核都适用\n");
    
    return 0;
}
