#include "common.h"

/**
 * 主函数 - 演示进程间信号量同步
 * @param argc int 命令行参数个数
 * @param argv const char*[] 命令行参数数组
 * @return int 程序退出状态码
 * 
 * 功能：演示使用共享内存中的信号量实现父子进程同步
 * 流程：创建共享内存 -> 映射信号量 -> 创建子进程 -> 进程同步 -> 清理资源
 */
int main(int argc,const char * argv[]){
    // 共享内存名称
    char *shm_name = "unamed_sem_shm";
    
    // 创建共享内存对象
    // shm_open(name, oflag, mode)
    // name: 共享内存对象名称
    // oflag: O_CREAT|O_RDWR 创建并读写模式
    // mode: 0666 权限设置（读写权限）
    int fd = shm_open(shm_name,O_CREAT|O_RDWR,0666);
    
    // 调整共享内存大小
    // ftruncate(fd, length)
    // fd: 文件描述符
    // length: 新的文件大小（字节）
    ftruncate(fd,sizeof(sem_t));
    
    // 将共享内存映射到进程地址空间
    // mmap(addr, length, prot, flags, fd, offset)
    // addr: 映射地址（NULL表示系统自动选择）
    // length: 映射长度
    // prot: PROT_READ|PROT_WRITE 读写权限
    // flags: MAP_SHARED 共享映射
    // fd: 文件描述符
    // offset: 偏移量（0表示从头开始）
    sem_t *sem = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

    // 初始化信号量
    // sem_init(sem, pshared, value)
    // sem: 信号量指针
    // pshared: 1表示进程间共享（非0）
    // value: 0 初始值，表示没有资源可用
    sem_init(sem,1,0);

    // 创建子进程
    // fork() 返回值：
    // < 0: 创建失败
    // = 0: 子进程
    // > 0: 父进程，返回值为子进程PID
    pid_t pid = fork();

    if(pid<0){
        // fork失败
        perror("fork");
    }else if(pid == 0){
        // 子进程
        sleep(1);  // 睡眠1秒，确保父进程先执行到sem_wait
        printf("this is son process\n");
        
        // 子进程释放信号量（sem+1），唤醒等待的父进程
        // sem_post(sem): 信号量值+1，如果有进程在等待则唤醒一个
        sem_post(sem);
    }else{
        // 父进程
        // 父进程等待信号量（sem-1），如果sem=0则阻塞等待
        // sem_wait(sem): 信号量值-1，如果值为0则阻塞
        sem_wait(sem);
        printf("this is parent process\n");
        
        // 等待子进程结束
        // waitpid(pid, status, options)
        // pid: 要等待的子进程ID
        // status: 存储子进程退出状态的指针（NULL表示不关心）
        // options: 等待选项（0表示阻塞等待）
        waitpid(pid,NULL,0);
    }

    // 回收资源
    if(pid>0){
        // 只有父进程负责销毁信号量
        // sem_destroy(sem): 销毁信号量
        if(sem_destroy(sem) == -1){
            perror("sem_destroy");
        }
    }

    // 取消内存映射
    // munmap(addr, length)
    // addr: 映射的起始地址
    // length: 映射的长度
    if(munmap(sem,sizeof(sem)) == -1){
        perror("munmap");
    }
    
    // 关闭文件描述符
    // close(fd): 关闭文件描述符
    if(close(fd) == -1){
        perror("close");
    }
    
    if(pid>0){
        // 只有父进程负责删除共享内存对象
        // shm_unlink(name): 删除共享内存对象
        if(shm_unlink(shm_name) == -1){
            perror("shm_unlink");
        }
    }
    return 0;
}