// 使用pthread_rwlockattr_t解决写饥饿问题
#include "common.h"
#include <unistd.h>

#define THREAD_WRITER_COUNT 2
#define THREAD_READER_COUNT 10

void *writer_thread_func(void *arg);
void *reader_thread_func(void *arg);

pthread_rwlock_t rwlock;
int shared_data = 0;

int main(int argc, char *argv[]){
    pthread_rwlockattr_t rwlock_attr;
    
    printf("=== 使用pthread_rwlockattr_t解决写饥饿问题 ===\n");
    
    // 初始化读写锁属性
    pthread_rwlockattr_init(&rwlock_attr);
    
    // 设置读写锁属性：优先考虑写者
    // PTHREAD_RWLOCK_PREFER_WRITER_NP: 当有写者等待时，新的读者会被阻塞
    int ret = pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
    if (ret != 0) {
        printf("设置读写锁属性失败: %s\n", strerror(ret));
        return -1;
    }
    
    printf("✓ 已设置读写锁属性: PTHREAD_RWLOCK_PREFER_WRITER_NP\n");
    printf("✓ 当有写者等待时，新的读者会被阻塞，避免写饥饿\n\n");
    
    // 使用属性初始化读写锁
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
    
    // 先创建读线程
    printf("创建 %d 个读线程...\n", THREAD_READER_COUNT);
    for(int i = 0; i < THREAD_READER_COUNT; i++) {
        pthread_create(&threads_reader[i],NULL,reader_thread_func,&thread_id_reader[i]);
    }

    // 延时后创建写线程
    sleep(2);
    printf("创建 %d 个写线程...\n", THREAD_WRITER_COUNT);
    pthread_create(&threads_writer[0],NULL,writer_thread_func,&thread_id_writer[0]);
    pthread_create(&threads_writer[1],NULL,writer_thread_func,&thread_id_writer[1]);

    // 等待所有线程结束
    for(int i = 0; i < THREAD_WRITER_COUNT; i++){
        pthread_join(threads_writer[i],NULL);
    }
    for(int i = 0; i < THREAD_READER_COUNT; i++){
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
    
    for(int i = 0; i < 3; i++) {
        printf("写线程 %d 尝试第 %d 次写入...\n", thread_id, i+1);
        
        pthread_rwlock_wrlock(&rwlock);
        shared_data++;
        printf("✓ 写线程 %d 成功写入数据: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        usleep(300000);  // 300ms延时
    }
    
    printf("写线程 %d 执行完毕\n", thread_id);
    return NULL;
}

void *reader_thread_func(void *arg){
    int thread_id = *(int *)arg;
    
    for(int i = 0; i < 5; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("读线程 %d 读取数据: %d\n",thread_id,shared_data);
        pthread_rwlock_unlock(&rwlock);
        
        usleep(200000);  // 200ms延时
    }
    
    printf("读线程 %d 执行完毕\n", thread_id);
    return NULL;
}


