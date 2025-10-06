// 写饥饿测试
#include "common.h"
#include <unistd.h>  // 添加usleep函数支持

#define THREAD_WRITER_COUNT 2
#define THREAD_READER_COUNT 10  // 增加读线程数量

void *writer_thread_func(void *arg);
void *reader_thread_func(void *arg);

// // 静态初始化读写锁，好处：不用手动销毁读写锁，可以保证读写锁的初始化是线程安全的
// static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER; // 静态初始化读写锁
// // 动态初始化读写锁，好处：可以手动销毁读写锁，可以保证读写锁的初始化是线程安全的
pthread_rwlock_t rwlock; // 动态初始化读写锁

int shared_data = 0;

int main(int argc, char *argv[]){
    pthread_rwlockattr_t rwlock_attr;
    
    // 初始化读写锁属性
    pthread_rwlockattr_init(&rwlock_attr);
    
    // 设置读写锁属性：优先考虑写者，避免写饥饿
    int ret = pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
    if (ret != 0) {
        printf("设置读写锁属性失败: %s\n", strerror(ret));
        return -1;
    }
    
    // 使用属性初始化读写锁
    pthread_rwlock_init(&rwlock, &rwlock_attr);

    pthread_t threads_writer[THREAD_WRITER_COUNT];
    pthread_t threads_reader[THREAD_READER_COUNT];

    int thread_id_writer[THREAD_WRITER_COUNT];
    // 初始化写线程ID
    for(ssize_t i = 0; i < THREAD_WRITER_COUNT; i++){
        thread_id_writer[i] = i+1;
    }
    // 初始化读线程ID
    int thread_id_reader[THREAD_READER_COUNT];
    for(ssize_t i = 0; i < THREAD_READER_COUNT; i++){
        thread_id_reader[i] = i+1;
    }
    // 先创建读线程，让读操作先开始
    printf("=== 写饥饿测试开始（使用PTHREAD_RWLOCK_PREFER_WRITER_NP属性） ===\n");
    printf("创建 %d 个读线程，%d 个写线程\n", THREAD_READER_COUNT, THREAD_WRITER_COUNT);
    printf("✓ 已设置写者优先属性，观察写饥饿是否得到缓解\n\n");
    
    // 创建读线程
    for(int i = 0; i < THREAD_READER_COUNT; i++) {
        pthread_create(&threads_reader[i],NULL,reader_thread_func,&thread_id_reader[i]);
    }

    // 延时后创建写线程，让读线程先获得读锁
    sleep(2);

    // 创建写线程
    pthread_create(&threads_writer[0],NULL,writer_thread_func,&thread_id_writer[0]);
    pthread_create(&threads_writer[1],NULL,writer_thread_func,&thread_id_writer[1]);

    // 等待所有线程结束
    for(ssize_t i = 0; i < THREAD_WRITER_COUNT; i++){
        pthread_join(threads_writer[i],NULL);
    }
    for(ssize_t i = 0; i < THREAD_READER_COUNT; i++){
        pthread_join(threads_reader[i],NULL);
    }

    // 销毁读写锁和属性
    pthread_rwlock_destroy(&rwlock);
    pthread_rwlockattr_destroy(&rwlock_attr);
    
    printf("\n=== 程序执行完毕 ===\n");
    printf("最终数据值: %d\n", shared_data);
    return 0;
}

void *writer_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    printf("写线程 %d 开始尝试获取写锁...\n", thread_id);
    
    for(int i = 0; i < 5; i++) {  // 减少写操作次数，但增加延时
        printf("写线程 %d 尝试第 %d 次写入...\n", thread_id, i+1);
        
        pthread_rwlock_wrlock(&rwlock);  // 写锁
        shared_data++;
        printf("✓ 写线程 %d 成功写入数据: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);  // 解锁
        
        // 增加写线程延时，让饥饿现象更明显
        usleep(500000);  // 500ms延时
    }
    
    printf("写线程 %d 执行完毕\n", thread_id);
    return NULL;
}

void *reader_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 10; i++) {  // 增加读操作次数
        // 读写锁中的读是可以并发的，即多个读线程可以同时读取共享资源
        pthread_rwlock_rdlock(&rwlock);
        printf("读线程 %d 读取数据: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        // 减少读线程延时，让读操作更频繁
        usleep(100000);  // 100ms延时（原来200ms）
    }
    
    printf("读线程 %d 执行完毕\n", thread_id);
    return NULL;
}