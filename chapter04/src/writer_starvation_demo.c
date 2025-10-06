#include "common.h"
#include <unistd.h>

// 写饥饿演示 - 用简单例子说明
pthread_rwlock_t rwlock;
int data = 0;

void *reader_func(void *arg) {
    int id = *(int*)arg;
    
    for(int i = 0; i < 3; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("读者%d 正在读书，当前数据: %d\n", id, data);
        sleep(1);  // 模拟读书时间
        pthread_rwlock_unlock(&rwlock);
        sleep(1);  // 休息一下
    }
    
    printf("读者%d 离开图书馆\n", id);
    return NULL;
}

void *writer_func(void *arg) {
    int id = *(int*)arg;
    
    printf("管理员%d 想要整理书架...\n", id);
    
    pthread_rwlock_wrlock(&rwlock);  // 等待所有读者离开
    printf("✓ 管理员%d 终于可以整理书架了！数据更新为: %d\n", id, ++data);
    sleep(2);  // 模拟整理时间
    pthread_rwlock_unlock(&rwlock);
    
    printf("管理员%d 完成工作\n", id);
    return NULL;
}

int main() {
    pthread_rwlock_init(&rwlock, NULL);
    
    printf("=== 写饥饿演示：图书馆场景 ===\n");
    printf("读者可以同时进入，但管理员必须等所有读者离开\n\n");
    
    pthread_t readers[5];
    pthread_t writers[2];
    int reader_ids[5] = {1,2,3,4,5};
    int writer_ids[2] = {1,2};
    
    // 先创建读者（模拟图书馆很受欢迎）
    for(int i = 0; i < 5; i++) {
        pthread_create(&readers[i], NULL, reader_func, &reader_ids[i]);
        sleep(1);  // 读者陆续进入
    }
    
    sleep(2);  // 让读者先开始读书
    
    // 管理员想要工作
    pthread_create(&writers[0], NULL, writer_func, &writer_ids[0]);
    pthread_create(&writers[1], NULL, writer_func, &writer_ids[1]);
    
    // 等待所有线程结束
    for(int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }
    for(int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }
    
    pthread_rwlock_destroy(&rwlock);
    return 0;
}

