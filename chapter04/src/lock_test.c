/*
    互斥锁：
        保证同一时刻只有一个线程可以执行临界区的代码
        pthread_mutex_t
            typdef union{
                char __size[__SIZEOF_PTHREAD_MUTEX_T];  //互斥锁的大小
                long int __align;                       //互斥锁的对齐方式
            }pthread_mutex_t;

        pthread_mutex_init: 初始化互斥锁
            int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
            @param mutex: 互斥锁
            @param attr: 互斥锁属性
            @return: 0成功，-1失败
        
        pthread_mutex_lock: 锁定互斥锁
            int pthread_mutex_lock(pthread_mutex_t *mutex);
            @param mutex: 互斥锁
            @return: 0成功，-1失败

        pthread_mutex_trylock: 尝试锁定互斥锁
            int pthread_mutex_trylock(pthread_mutex_t *mutex);
            @param mutex: 互斥锁
            @return: 0成功，-1失败

        pthread_mutex_unlock: 解锁互斥锁
            int pthread_mutex_unlock(pthread_mutex_t *mutex);
            @param mutex: 互斥锁
            @return: 0成功，-1失败

        pthread_mutex_destroy: 销毁互斥锁
            int pthread_mutex_destroy(pthread_mutex_t *mutex);
            @param mutex: 互斥锁
            @return: 0成功，-1失败

        使用流程：
            1. 初始化互斥锁
            2. 锁定互斥锁
            3. 解锁互斥锁
            4. 销毁互斥锁
        
    读写锁：
        允许多个线程同时读取共享资源，但只有一个线程可以写入共享资源
        pthread_rwlock_t
            typedef union{
                char __size[__SIZEOF_PTHREAD_RWLOCK_T];
                long int __align;
            }pthread_rwlock_t;

        pthread_rwlock_init: 初始化读写锁
            int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
            @param rwlock: 读写锁
            @param attr: 读写锁属性
            @return: 0成功，-1失败
        
        pthread_rwlock_rdlock: 锁定读写锁
            int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
            @param rwlock: 读写锁
            @return: 0成功，-1失败
        
        pthread_rwlock_wrlock: 锁定读写锁
            int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
            @param rwlock: 读写锁
            @return: 0成功，-1失败
        
        pthread_rwlock_unlock: 解锁读写锁
            int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
            @param rwlock: 读写锁
            @return: 0成功，-1失败
        
        pthread_rwlock_destroy: 销毁读写锁
            int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
            @param rwlock: 读写锁
            @return: 0成功，-1失败
        
    自旋锁：
        在获取锁之前，线程在循环中忙等待，适用于锁持有时间非常短的场景
*/

#include "common.h"

int main(int argc, char *argv[]){
    /* code */
    return 0;
}