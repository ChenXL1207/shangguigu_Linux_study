// 对比不同pthread_rwlockattr_t属性的效果
#include "common.h"
#include <unistd.h>

#define THREAD_WRITER_COUNT 2
#define THREAD_READER_COUNT 8

void *writer_thread_func(void *arg);
void *reader_thread_func(void *arg);

int shared_data = 0;
pthread_rwlock_t rwlock;  // 全局读写锁变量

// 测试不同的读写锁属性
void test_rwlock_attribute(int attr_kind, const char* attr_name) {
    pthread_rwlockattr_t rwlock_attr;
    
    printf("\n=== 测试属性: %s ===\n", attr_name);
    
    // 初始化属性
    pthread_rwlockattr_init(&rwlock_attr);
    pthread_rwlockattr_setkind_np(&rwlock_attr, attr_kind);
    
    // 初始化读写锁
    pthread_rwlock_init(&rwlock, &rwlock_attr);
    
    pthread_t threads_writer[THREAD_WRITER_COUNT];
    pthread_t threads_reader[THREAD_READER_COUNT];
    
    int thread_id_writer[THREAD_WRITER_COUNT];
    int thread_id_reader[THREAD_READER_COUNT];
    
    // 初始化线程ID
    for(int i = 0; i < THREAD_WRITER_COUNT; i++){
        thread_id_writer[i] = i+1;
    }
    for(int i = 0; i < THREAD_READER_COUNT; i++){
        thread_id_reader[i] = i+1;
    }
    
    // 重置共享数据
    shared_data = 0;
    
    // 创建读线程
    for(int i = 0; i < THREAD_READER_COUNT; i++) {
        pthread_create(&threads_reader[i],NULL,reader_thread_func,&thread_id_reader[i]);
    }
    
    sleep(1);  // 让读线程先开始
    
    // 创建写线程
    pthread_create(&threads_writer[0],NULL,writer_thread_func,&thread_id_writer[0]);
    pthread_create(&threads_writer[1],NULL,writer_thread_func,&thread_id_writer[1]);
    
    // 等待所有线程结束
    for(int i = 0; i < THREAD_WRITER_COUNT; i++){
        pthread_join(threads_writer[i],NULL);
    }
    for(int i = 0; i < THREAD_READER_COUNT; i++){
        pthread_join(threads_reader[i],NULL);
    }
    
    printf("最终数据值: %d\n", shared_data);
    
    // 清理
    pthread_rwlock_destroy(&rwlock);
    pthread_rwlockattr_destroy(&rwlock_attr);
}

int main(int argc, char *argv[]){
    printf("=== pthread_rwlockattr_t 属性对比测试 ===\n");
    printf("测试不同读写锁属性对写饥饿的影响\n");
    
    // 测试1: 默认属性（读者优先）
    test_rwlock_attribute(PTHREAD_RWLOCK_PREFER_READER_NP, 
                         "PTHREAD_RWLOCK_PREFER_READER_NP (读者优先)");
    
    sleep(2);
    
    // 测试2: 写者优先
    test_rwlock_attribute(PTHREAD_RWLOCK_PREFER_WRITER_NP, 
                         "PTHREAD_RWLOCK_PREFER_WRITER_NP (写者优先)");
    
    sleep(2);
    
    // 测试3: 写者优先非递归
    test_rwlock_attribute(PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP, 
                         "PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP (写者优先非递归)");
    
    printf("\n=== 测试完成 ===\n");
    printf("观察不同属性下写线程的执行情况\n");
    
    return 0;
}

void *writer_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 2; i++) {
        printf("写线程 %d 尝试写入...\n", thread_id);
        
        pthread_rwlock_wrlock(&rwlock);
        shared_data++;
        printf("✓ 写线程 %d 写入成功: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        usleep(500000);  // 500ms延时
    }
    
    return NULL;
}

void *reader_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 3; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("读线程 %d 读取: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        usleep(300000);  // 300ms延时
    }
    
    return NULL;
}
