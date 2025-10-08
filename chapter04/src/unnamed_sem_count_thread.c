#include "common.h"

// 全局信号量指针
sem_t *full;    // 满信号量，表示有数据可读
sem_t *empty;   // 空信号量，表示有空间可写

// 共享数据
int shared_num; // 生产者消费者之间共享的数据

/**
 * 生成随机数
 * @return int 返回一个随机整数
 */
int rand_num(){
    srand(time(NULL));  // 使用当前时间作为随机数种子
    return rand();      // 返回随机数
}

/**
 * 生产者线程函数
 * @param arg void* 线程参数（未使用）
 * @return void* 线程返回值（NULL）
 * 
 * 功能：生产者线程负责生产数据并放入共享缓冲区
 * 流程：等待空信号量 -> 生产数据 -> 释放满信号量
 */
void *producer_func(void *arg){
    for(ssize_t i = 0 ; i < 5; i++)
    {
        // 等待空信号量（empty-1），如果empty=0则阻塞等待
        // empty信号量表示缓冲区中可用的空间数量
        sem_wait(empty);
        
        printf("\n 第%ld轮数据传输\n",i+1);
        sleep(1);  // 模拟生产数据的时间
        
        // 生产数据：生成随机数并放入共享变量
        shared_num = rand_num();
        
        // 释放满信号量（full+1），通知消费者有数据可读
        // full信号量表示缓冲区中可读的数据数量
        sem_post(full);
    }
    return NULL;
}

/**
 * 消费者线程函数
 * @param arg void* 线程参数（未使用）
 * @return void* 线程返回值（NULL）
 * 
 * 功能：消费者线程负责从共享缓冲区读取数据
 * 流程：等待满信号量 -> 消费数据 -> 释放空信号量
 */
void *consumer_func(void *arg){
    for(ssize_t i = 0 ; i < 5; i++)
    {
        // 等待满信号量（full-1），如果full=0则阻塞等待
        // full信号量表示缓冲区中可读的数据数量
        sem_wait(full);
        
        printf("第%ld轮消费者开始读取数据,",i+1);
        sleep(1);  // 模拟消费数据的时间
        
        // 消费数据：读取共享变量中的数据
        printf("接收的数据为：%d",shared_num);
        
        // 释放空信号量（empty+1），通知生产者有空间可写
        // empty信号量表示缓冲区中可用的空间数量
        sem_post(empty);
    }
    return NULL;
}

/**
 * 主函数
 * @param argc int 命令行参数个数
 * @param argv char const*[] 命令行参数数组
 * @return int 程序退出状态码
 * 
 * 功能：演示使用信号量实现的生产者-消费者模式
 * 流程：初始化信号量 -> 创建线程 -> 等待线程完成 -> 清理资源
 */
int main(int argc,char const *argv[])
{
    // 为信号量分配内存空间
    full = malloc(sizeof(sem_t));    // 满信号量
    empty = malloc(sizeof(sem_t));   // 空信号量

    // 初始化信号量
    // sem_init(sem, pshared, value)
    // sem: 信号量指针
    // pshared: 0表示线程间共享，非0表示进程间共享
    // value: 信号量的初始值
    sem_init(empty,0,1);  // empty初始值为1，表示有1个空位可写
    sem_init(full,0,0);   // full初始值为0，表示没有数据可读

    // 创建生产者消费者线程
    pthread_t producer_id,consumer_id;  // 线程ID
    
    // pthread_create(thread, attr, start_routine, arg)
    // thread: 指向线程ID的指针
    // attr: 线程属性（NULL表示使用默认属性）
    // start_routine: 线程要执行的函数
    // arg: 传递给线程函数的参数
    pthread_create(&producer_id,NULL,producer_func,NULL);
    pthread_create(&consumer_id,NULL,consumer_func,NULL);

    // 等待所有线程全部执行完成
    // pthread_join(thread, retval)
    // thread: 要等待的线程ID
    // retval: 存储线程返回值的指针（NULL表示不关心返回值）
    pthread_join(producer_id,NULL);
    pthread_join(consumer_id,NULL);

    // 销毁信号量，释放资源
    // sem_destroy(sem): 销毁信号量
    sem_destroy(empty);
    sem_destroy(full);

    return 0;
}