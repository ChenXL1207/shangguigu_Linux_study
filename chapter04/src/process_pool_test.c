#include "common.h"

/*
    GThreadPool: 线程池
    结构体：
        typedef struct GThreadPool{
            GFunction func; // 线程函数
            gpointer user_data; // 用户数据
            gint max_threads; // 线程池中线程的最大数量
            gboolean exclusive; // 是否独占线程
            GError **error; // 错误信息
        }GThreadPool;

    g_thread_pool_num: 线程池中线程的数量
    函数：create_thread_pool(int g_thread_pool_num)
        功能：创建线程池
        参数：
            @param g_thread_pool_num: 线程池中线程的数量
        返回：
            @return: 0成功，-1失败
    
    g_thread_pool_push: 将任务推送到线程池
    函数：g_thread_pool_push(GThreadPool *pool, gpointer data)
        功能：将任务推送到线程池
        参数：
            @param pool: 线程池
            @param data: 用户数据
        返回：
            @return: 0成功，-1失败

    g_thread_pool_free: 释放线程池
    函数：g_thread_pool_free(GThreadPool *pool, gboolean immediate, gboolean wait)
        功能：释放线程池
        参数：
            @param pool: 线程池
            @param immediate: 是否立即释放
            @param wait: 是否等待线程池中的线程完成
        返回：
            @return: 0成功，-1失败


*/

void task_func(gpointer data,gpointer user_data){
    int task_num = *(int *)data;
    printf("线程%ld: 任务%d开始执行......\n", pthread_self(), task_num);
    sleep(1); // 固定执行时间，便于观察线程复用
    printf("线程%ld: 任务%d执行完成\n", pthread_self(), task_num);
    free(data);
}

int main(int argc, char *argv[]){
    printf("=== 线程池演示：展示线程复用和任务队列 ===\n");
    
    // 创建线程池 - 只有3个线程，但会有更多任务
    /*
        g_thread_pool_new(
            GFunction func, // 线程函数
            gpointer user_data, // 用户数据
            gint max_threads, // 线程池中线程的最大数量
            gboolean exclusive, // 是否独占线程
            GError **error // 错误信息
        )
    */
    GThreadPool *pool = g_thread_pool_new(task_func,NULL,3,FALSE,NULL);
    
    printf("创建了3个线程的线程池\n");
    printf("将提交10个任务，观察线程复用...\n\n");
    
    // 将更多任务推送到线程池 - 10个任务，但只有3个线程
    for(int i = 1; i <= 10; i++){
        int *tmp = malloc(sizeof(int));
        *tmp = i;
        /*
            g_thread_pool_push(
                GThreadPool *pool, // 线程池
                gpointer data, // 用户数据
                GError **error // 错误信息
            )
        */
        g_thread_pool_push(pool,tmp,NULL); // 将任务推送到线程池
        printf("提交任务%d到线程池\n", i);
        usleep(100000); // 稍微延迟，便于观察任务提交顺序
    }
    
    printf("\n所有任务已提交，等待执行完成...\n");
    
    // 释放线程池
    /*
        g_thread_pool_free(
            GThreadPool *pool, // 线程池
            gboolean immediate, // 是否立即释放
            gboolean wait // 是否等待线程池中的线程完成
        )
    */
    g_thread_pool_free(pool,FALSE,TRUE);
    printf("\n线程池释放完成,所有任务均已执行完成\n");
    printf("观察上面的输出，可以看到同一个线程ID处理了多个任务\n");
    return 0;
}