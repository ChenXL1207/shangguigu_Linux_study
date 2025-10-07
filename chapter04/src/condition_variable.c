#include "common.h"

/*
    条件变量（Condition Variable）：
    先前基础知识：
        关键字restrict：
            用于告诉编译器，指针是访问对象的唯一方式，避免编译器优化
    
    pthread_cond_wait: 等待条件变量(暂时释放互斥锁，等待条件变量满足)
        int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
        @brief: 等待条件变量，当条件变量满足时，线程会继续执行
        @param cond: 条件变量
        @param mutex: 互斥锁
        @return: 0成功，-1失败

    pthread_cond_signal: 发送信号(唤醒一个等待条件变量的线程)
        int pthread_cond_signal(pthread_cond_t *cond);
        @brief: 发送信号，当条件变量满足时，线程会继续执行
        @param cond: 条件变量
        @return: 0成功，-1失败

    pthread_cond_broadcast: 广播信号(唤醒所有等待条件变量的线程)
        int pthread_cond_broadcast(pthread_cond_t *cond);
        @brief: 广播信号，当条件变量满足时，线程会继续执行
        @param cond: 条件变量
        @return: 0成功，-1失败

    pthread_cond_timedwait: 等待条件变量(等待时间到达)
        int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
        @brief: 等待条件变量，当条件变量满足时，线程会继续执行
        @param cond: 条件变量
        @param mutex: 互斥锁
        @param abstime: 等待时间
        @return: 0成功，-1失败

    操作流程：
        1. 初始化（pthread_cond_init）：创建并初始化条件变量
        2. 等待条件变量（pthread_cond_wait）：等待条件变量满足
        3. 定时等待条件变量（pthread_cond_timedwait）：等待条件变量满足，等待时间到达
        4. 发送信号（pthread_cond_signal）：发送信号，唤醒一个等待条件变量的线程
        5. 广播信号（pthread_cond_broadcast）：广播信号，唤醒所有等待条件变量的线程
        6. 销毁条件变量（pthread_cond_destroy）：销毁条件变量

    静态初始化变量：PTHREAD_COND_INITIALIZER
    动态初始化变量：pthread_cond_init
    销毁条件变量：pthread_cond_destroy
*/

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int count = 0;
int should_exit = 0; // 控制程序退出的标志

// 静态初始化互斥锁
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 初始化（pthread_cond_init）：创建并初始化条件变量
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 函数声明
void *producer_thread_func(void *arg);
void *consumer_thread_func(void *arg);

int main(int argc, char const *argv[])
{
    printf("=== 条件变量生产者-消费者示例程序启动 ===\n");
    printf("缓冲区大小: %d\n", BUFFER_SIZE);
    printf("程序将运行10秒后自动退出\n\n");
    
    // 创建两个线程，一个向buffer中添加数据，一个从buffer中取出数据
    pthread_t producer_thread;
    pthread_t consumer_thread;

    // 创建生产者线程
    pthread_create(&producer_thread,NULL,producer_thread_func,NULL);
    // 创建消费者线程
    pthread_create(&consumer_thread,NULL,consumer_thread_func,NULL);

    // 主线程等待10秒
    sleep(10);
    should_exit = 1; // 设置退出标志
    
    // 发送信号唤醒可能等待的线程
    pthread_cond_broadcast(&cond);

    // 主线程需要挂起，等待生产者和消费者线程完成
    pthread_join(producer_thread,NULL);
    pthread_join(consumer_thread,NULL);

    // 销毁条件变量
    pthread_cond_destroy(&cond);
    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);

    return 0;
}
/*
    生产者线程：不断向buffer中添加数据
    参数：
        @param arg: 线程参数（这里不使用）
    返回：
        @return: NULL
*/
void *producer_thread_func(void *arg){
    int item = 0;
    while(!should_exit){
        pthread_mutex_lock(&mutex); // 锁定互斥锁
        while(count == BUFFER_SIZE && !should_exit){
            pthread_cond_wait(&cond,&mutex); // 等待条件变量，暂停生产，等待条件变量满足
        }
        if(should_exit) {
            pthread_mutex_unlock(&mutex); // 解锁互斥锁
            break;
        }

        buffer[count++] = item++; // 添加数据
        printf("生产者: 生产了物品 %d，当前缓冲区中有 %d 个物品\n", item-1, count);
        pthread_cond_signal(&cond); // 发送信号，唤醒消费者线程
        pthread_mutex_unlock(&mutex); // 解锁互斥锁
        sleep(0.5); // 睡眠0.5秒
    }
    printf("生产者线程退出\n");
    return NULL;
}
/*
    消费者线程：不断从buffer中取出数据
    参数：
        @param arg: 线程参数（这里不使用）
    返回：
        @return: NULL
*/
void *consumer_thread_func(void *arg){
    while(!should_exit){
        pthread_mutex_lock(&mutex); // 锁定互斥锁
        while(count == 0 && !should_exit){
            pthread_cond_wait(&cond,&mutex); // 等待条件变量
        }
        if(should_exit) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        int consumed_item = buffer[--count]; // 取出数据
        printf("消费者: 消费了物品 %d，当前缓冲区中有 %d 个物品\n", consumed_item, count);
        pthread_cond_signal(&cond); // 发送信号
        pthread_mutex_unlock(&mutex); // 解锁互斥锁
        sleep(1); // 睡眠1秒
    }
    printf("消费者线程退出\n");
    return NULL;
}