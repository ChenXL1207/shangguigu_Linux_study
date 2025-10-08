#include "common.h"
#include <sys/wait.h>
#include <semaphore.h>

/**
 * 主函数 - 演示有名信号量实现进程间同步
 * @param argc int 命令行参数个数
 * @param argv char const*[] 命令行参数数组
 * @return int 程序退出状态码
 * 
 * 功能：演示使用有名信号量实现父子进程同步
 * 流程：创建有名信号量 -> 创建子进程 -> 进程同步 -> 清理资源
 */
int main(int argc, char const *argv[])
{
    // 有名信号量的名称（必须以斜杠开头）
    char *sem_name = "/named_sem_bin_process";
    
    // 创建有名信号量
    // sem_open(name, oflag, mode, value)
    // name: 信号量名称（必须以/开头）
    // oflag: O_CREAT|O_RDWR 创建并读写模式
    // mode: 0666 权限设置（读写权限）
    // value: 1 信号量初始值（二进制信号量，0或1）
    sem_t *sem_value = sem_open(sem_name, O_CREAT | O_RDWR, 0666, 1);
    
    // 检查信号量创建是否成功
    if (sem_value == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    
    // 创建子进程
    // fork() 返回值：
    // < 0: 创建失败
    // = 0: 子进程
    // > 0: 父进程，返回值为子进程PID
    int pid = fork();
    
    if (pid > 0) {
        // 父进程
        printf("父进程开始执行...\n");
        
        // 等待信号量（sem-1），如果sem=0则阻塞等待
        // sem_wait(sem): 信号量值-1，如果值为0则阻塞
        if (sem_wait(sem_value) == -1) {
            perror("sem_wait");
            exit(1);
        }
        
        printf("父进程获得信号量，开始临界区操作\n");
        
        // 模拟临界区操作
        sleep(2);
        printf("父进程完成临界区操作\n");
        
        // 释放信号量（sem+1），唤醒等待的进程
        // sem_post(sem): 信号量值+1，如果有进程在等待则唤醒一个
        if (sem_post(sem_value) == -1) {
            perror("sem_post");
            exit(1);
        }
        
        printf("父进程释放信号量\n");
        
        // 等待子进程执行完毕
        // waitpid(pid, status, options)
        // pid: 要等待的子进程ID
        // status: 存储子进程退出状态的指针（NULL表示不关心）
        // options: 等待选项（0表示阻塞等待）
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid");
            exit(1);
        }
        
        printf("父进程执行完毕\n");
        
        // 关闭信号量
        // sem_close(sem): 关闭信号量，释放进程内的信号量引用
        if (sem_close(sem_value) == -1) {
            perror("sem_close");
            exit(1);
        }
        
        // 删除有名信号量
        // sem_unlink(name): 删除系统中的有名信号量
        // 注意：只有当所有进程都关闭了信号量后，unlink才会真正删除
        if (sem_unlink(sem_name) == -1) {
            perror("sem_unlink");
            exit(1);
        }
        
        printf("父进程清理完毕\n");
        
    } else if (pid == 0) {
        // 子进程
        printf("子进程开始执行...\n");
        
        // 等待信号量（sem-1），如果sem=0则阻塞等待
        if (sem_wait(sem_value) == -1) {
            perror("sem_wait");
            exit(1);
        }
        
        printf("子进程获得信号量，开始临界区操作\n");
        
        // 模拟临界区操作
        sleep(1);
        printf("子进程完成临界区操作\n");
        
        // 释放信号量（sem+1），唤醒等待的进程
        if (sem_post(sem_value) == -1) {
            perror("sem_post");
            exit(1);
        }
        
        printf("子进程释放信号量\n");
        printf("子进程执行完毕\n");
        
        // 关闭信号量
        if (sem_close(sem_value) == -1) {
            perror("sem_close");
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
