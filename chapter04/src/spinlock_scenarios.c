// 自旋锁适用场景演示
#include "common.h"
#include <unistd.h>
#include <time.h>

#define THREAD_COUNT 4

// 场景1: 短时间临界区 - 适合自旋锁
int short_critical_data = 0;
pthread_spinlock_t short_spinlock;

// 场景2: 长时间临界区 - 不适合自旋锁
int long_critical_data = 0;
pthread_spinlock_t long_spinlock;

void *short_critical_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    for (int i = 0; i < 1000; i++) {
        pthread_spin_lock(&short_spinlock);
        
        // 短时间临界区：只是简单的赋值操作
        short_critical_data++;
        
        pthread_spin_unlock(&short_spinlock);
    }
    
    printf("短临界区线程 %d 完成\n", thread_id);
    return NULL;
}

void *long_critical_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    for (int i = 0; i < 100; i++) {
        pthread_spin_lock(&long_spinlock);
        
        // 长时间临界区：模拟复杂计算
        printf("线程 %d 开始长时间计算...\n", thread_id);
        usleep(10000);  // 10ms延时，模拟复杂计算
        long_critical_data++;
        printf("线程 %d 完成计算，数据: %d\n", thread_id, long_critical_data);
        
        pthread_spin_unlock(&long_spinlock);
        
        // 线程休息
        usleep(5000);  // 5ms休息
    }
    
    printf("长临界区线程 %d 完成\n", thread_id);
    return NULL;
}

void test_short_critical() {
    printf("\n=== 测试1: 短时间临界区（适合自旋锁）===\n");
    
    pthread_spin_init(&short_spinlock, PTHREAD_PROCESS_PRIVATE);
    short_critical_data = 0;
    
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, short_critical_worker, &thread_ids[i]);
    }
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("短临界区结果: 数据=%d, 耗时=%.6f秒\n", short_critical_data, duration);
    
    pthread_spin_destroy(&short_spinlock);
}

void test_long_critical() {
    printf("\n=== 测试2: 长时间临界区（不适合自旋锁）===\n");
    printf("注意观察: 其他线程会长时间自旋等待，浪费CPU\n");
    
    pthread_spin_init(&long_spinlock, PTHREAD_PROCESS_PRIVATE);
    long_critical_data = 0;
    
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, long_critical_worker, &thread_ids[i]);
    }
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("长临界区结果: 数据=%d, 耗时=%.6f秒\n", long_critical_data, duration);
    
    pthread_spin_destroy(&long_spinlock);
}

int main() {
    printf("=== 自旋锁适用场景演示 ===\n");
    printf("自旋锁适用场景:\n");
    printf("✓ 临界区执行时间很短（微秒级别）\n");
    printf("✓ 多核CPU环境\n");
    printf("✓ 锁竞争不激烈\n");
    printf("✗ 临界区执行时间长\n");
    printf("✗ 单核CPU环境\n");
    printf("✗ 锁竞争激烈\n\n");
    
    // 测试短临界区
    test_short_critical();
    
    // 测试长临界区
    test_long_critical();
    
    printf("\n=== 总结 ===\n");
    printf("1. 短临界区: 自旋锁效率高，线程切换开销小\n");
    printf("2. 长临界区: 自旋锁效率低，其他线程浪费CPU自旋\n");
    printf("3. 选择原则: 临界区时间 < 线程切换时间时使用自旋锁\n");
    
    return 0;
}


