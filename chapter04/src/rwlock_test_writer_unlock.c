// 写操作不加锁测试 - 演示数据竞争风险
#include "common.h"
#include <unistd.h>  // 添加usleep函数支持

#define THREAD_WRITER_COUNT 2
#define THREAD_READER_COUNT 6

void *writer_thread_func(void *arg);
void *reader_thread_func(void *arg);

// // 静态初始化读写锁，好处：不用手动销毁读写锁，可以保证读写锁的初始化是线程安全的
// static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER; // 静态初始化读写锁
// // 动态初始化读写锁，好处：可以手动销毁读写锁，可以保证读写锁的初始化是线程安全的
pthread_rwlock_t rwlock; // 动态初始化读写锁

int shared_data = 0;

int main(int argc, char *argv[]){
    // 初始化读写锁
    pthread_rwlock_init(&rwlock,NULL);

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
    // 创建写线程
    pthread_create(&threads_writer[0],NULL,writer_thread_func,&thread_id_writer[0]);
    pthread_create(&threads_writer[1],NULL,writer_thread_func,&thread_id_writer[1]);

    // 短暂延时后立即创建读线程，让读写同时进行
    sleep(1);

    // 创建读线程
    pthread_create(&threads_reader[0],NULL,reader_thread_func,&thread_id_reader[0]);
    pthread_create(&threads_reader[1],NULL,reader_thread_func,&thread_id_reader[1]);
    pthread_create(&threads_reader[2],NULL,reader_thread_func,&thread_id_reader[2]);
    pthread_create(&threads_reader[3],NULL,reader_thread_func,&thread_id_reader[3]);
    pthread_create(&threads_reader[4],NULL,reader_thread_func,&thread_id_reader[4]);
    pthread_create(&threads_reader[5],NULL,reader_thread_func,&thread_id_reader[5]);

    // 等待所有线程结束
    for(ssize_t i = 0; i < THREAD_WRITER_COUNT; i++){
        pthread_join(threads_writer[i],NULL);
    }
    for(ssize_t i = 0; i < THREAD_READER_COUNT; i++){
        pthread_join(threads_reader[i],NULL);
    }

    // 销毁读写锁
    pthread_rwlock_destroy(&rwlock);
    return 0;
}

void *writer_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 10; i++) {
        // pthread_rwlock_wrlock(&rwlock);  // 写锁被注释，故意制造数据竞争
        shared_data++;
        printf("线程 %d 写入数据: %d\n",thread_id,shared_data);
        // pthread_rwlock_unlock(&rwlock);  // 解锁被注释
        
        // 添加小延时，让竞争更明显
        usleep(100000);  // 100ms延时
    }
    
    return NULL;
}

void *reader_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 5; i++) {
        // 读写锁中的读是可以并发的，即多个读线程可以同时读取共享资源
        pthread_rwlock_rdlock(&rwlock);
        printf("线程 %d 读取数据: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        // 添加小延时
        usleep(200000);  // 200ms延时
    }
    
    return NULL;
}