#include "common.h"
#include <sys/wait.h>
#include <semaphore.h>

// 有名信号量名称
#define EMPTY_SEM_NAME "/named_sem_empty"  // 空信号量，表示有空间可写
#define FULL_SEM_NAME  "/named_sem_full"   // 满信号量，表示有数据可读

// 共享数据（使用全局变量模拟，实际应用中可能需要共享内存）
int shared_data = 0;

/**
 * 生产者进程函数
 * @param empty_sem sem_t* 空信号量指针
 * @param full_sem sem_t* 满信号量指针
 * 
 * 功能：生产者进程负责生产数据
 * 流程：等待空信号量 -> 生产数据 -> 释放满信号量
 */
void producer_process(sem_t *empty_sem, sem_t *full_sem) {
    for (int i = 0; i < 5; i++) {
        printf("生产者：准备生产第%d个数据\n", i + 1);
        
        // 等待空信号量（empty-1），如果empty=0则阻塞等待
        // empty信号量表示缓冲区中可用的空间数量
        if (sem_wait(empty_sem) == -1) {
            perror("producer sem_wait empty");
            exit(1);
        }
        
        printf("生产者：获得空信号量，开始生产数据\n");
        
        // 生产数据：生成随机数
        srand(time(NULL) + i);
        shared_data = rand() % 1000;
        
        // 模拟生产时间
        sleep(1);
        
        printf("生产者：生产完成，数据为 %d\n", shared_data);
        
        // 释放满信号量（full+1），通知消费者有数据可读
        // full信号量表示缓冲区中可读的数据数量
        if (sem_post(full_sem) == -1) {
            perror("producer sem_post full");
            exit(1);
        }
        
        printf("生产者：释放满信号量\n");
    }
}

/**
 * 消费者进程函数
 * @param empty_sem sem_t* 空信号量指针
 * @param full_sem sem_t* 满信号量指针
 * 
 * 功能：消费者进程负责消费数据
 * 流程：等待满信号量 -> 消费数据 -> 释放空信号量
 */
void consumer_process(sem_t *empty_sem, sem_t *full_sem) {
    for (int i = 0; i < 5; i++) {
        printf("消费者：准备消费第%d个数据\n", i + 1);
        
        // 等待满信号量（full-1），如果full=0则阻塞等待
        // full信号量表示缓冲区中可读的数据数量
        if (sem_wait(full_sem) == -1) {
            perror("consumer sem_wait full");
            exit(1);
        }
        
        printf("消费者：获得满信号量，开始消费数据\n");
        
        // 消费数据：读取共享数据
        int consumed_data = shared_data;
        
        // 模拟消费时间
        sleep(1);
        
        printf("消费者：消费完成，数据为 %d\n", consumed_data);
        
        // 释放空信号量（empty+1），通知生产者有空间可写
        // empty信号量表示缓冲区中可用的空间数量
        if (sem_post(empty_sem) == -1) {
            perror("consumer sem_post empty");
            exit(1);
        }
        
        printf("消费者：释放空信号量\n");
    }
}

/**
 * 主函数 - 演示有名信号量实现生产者-消费者模式
 * @param argc int 命令行参数个数
 * @param argv char const*[] 命令行参数数组
 * @return int 程序退出状态码
 * 
 * 功能：演示使用有名信号量实现生产者-消费者模式
 * 流程：创建有名信号量 -> 创建子进程 -> 生产者消费者同步 -> 清理资源
 */
int main(int argc, char const *argv[])
{
    // 创建有名信号量
    // sem_open(name, oflag, mode, value)
    // name: 信号量名称（必须以/开头）
    // oflag: O_CREAT|O_RDWR 创建并读写模式
    // mode: 0666 权限设置（读写权限）
    // value: 信号量初始值
    
    // 创建空信号量，初始值为1（表示有1个空位可写）
    sem_t *empty_sem = sem_open(EMPTY_SEM_NAME, O_CREAT | O_RDWR, 0666, 1);
    if (empty_sem == SEM_FAILED) {
        perror("sem_open empty");
        exit(1);
    }
    
    // 创建满信号量，初始值为0（表示没有数据可读）
    sem_t *full_sem = sem_open(FULL_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (full_sem == SEM_FAILED) {
        perror("sem_open full");
        exit(1);
    }
    
    printf("有名信号量创建成功\n");
    
    // 创建子进程
    // fork() 返回值：
    // < 0: 创建失败
    // = 0: 子进程
    // > 0: 父进程，返回值为子进程PID
    int pid = fork();
    
    if (pid > 0) {
        // 父进程 - 作为生产者
        printf("父进程作为生产者开始执行...\n");
        producer_process(empty_sem, full_sem);
        
        // 等待子进程执行完毕
        // waitpid(pid, status, options)
        // pid: 要等待的子进程ID
        // status: 存储子进程退出状态的指针（NULL表示不关心）
        // options: 等待选项（0表示阻塞等待）
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid");
            exit(1);
        }
        
        printf("父进程（生产者）执行完毕\n");
        
        // 关闭信号量
        // sem_close(sem): 关闭信号量，释放进程内的信号量引用
        if (sem_close(empty_sem) == -1) {
            perror("sem_close empty");
            exit(1);
        }
        if (sem_close(full_sem) == -1) {
            perror("sem_close full");
            exit(1);
        }
        
        // 删除有名信号量
        // sem_unlink(name): 删除系统中的有名信号量
        // 注意：只有当所有进程都关闭了信号量后，unlink才会真正删除
        if (sem_unlink(EMPTY_SEM_NAME) == -1) {
            perror("sem_unlink empty");
            exit(1);
        }
        if (sem_unlink(FULL_SEM_NAME) == -1) {
            perror("sem_unlink full");
            exit(1);
        }
        
        printf("父进程清理完毕\n");
        
    } else if (pid == 0) {
        // 子进程 - 作为消费者
        printf("子进程作为消费者开始执行...\n");
        consumer_process(empty_sem, full_sem);
        
        printf("子进程（消费者）执行完毕\n");
        
        // 关闭信号量
        if (sem_close(empty_sem) == -1) {
            perror("sem_close empty");
            exit(1);
        }
        if (sem_close(full_sem) == -1) {
            perror("sem_close full");
            exit(1);
        }
        
        // 子进程退出
        exit(0);
        
    } else {
        // fork失败
        perror("fork");
        exit(1);
    }
    
    return 0;
}
