#include "common.h"
#include <sys/mman.h>
/*
    共享内存是进程间通信的一种方式，它可以让多个进程看到同一块内存区域。
    shm_open 创建共享内存
    shm_unlink 删除共享内存
*/

/*
    int shm_open(const char *name, int oflag, mode_t mode);
    @name: 共享内存的名称
    @oflag: 共享内存的标志
    @mode: 共享内存的权限
    @return: 共享内存的文件描述符
    @desc: 创建共享内存

    int oflag:
        O_CREAT: 创建共享内存
        O_RDWR: 读写权限
        O_RDONLY: 只读权限
        O_WRONLY: 只写权限
        O_APPEND: 追加权限
        O_TRUNC: 截断权限
    mode_t mode:(mask,0666)
        S_IRUSR: 用户读权限
        S_IWUSR: 用户写权限
        S_IRGRP: 组读权限
        S_IWGRP: 组写权限
        S_IROTH: 其他读权限
        S_IWOTH: 其他写权限
*/  

/*
    int shm_unlink(const char *name);
    @name: 共享内存的名称
    @return: 0 成功，-1 失败
    @desc: 删除共享内存
*/

/*
    truncate 截断文件
    int truncate(const char *path, off_t length);
    @path: 文件路径
    @length: 截断长度
    @return: 0 成功，-1 失败
    @desc: 截断文件
*/

/*
    ftruncate 截断文件
    int ftruncate(int fd, off_t length);
    @fd: 文件描述符
    @length: 截断长度
    @return: 0 成功，-1 失败
    @desc: 截断文件
*/

/*
    mmap 映射文件
    void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
    @addr: 映射地址,通常设置为NULL
    @length: 映射长度，设置truncate的长度
    @prot: 映射权限,PROT_READ,PROT_WRITE,PROT_READ|PROT_WRITE
    @flags: 映射标志，MAP_SHARED,MAP_PRIVATE，通常为MAP_SHARED
    @fd: 文件描述符，通常为shm_open的返回值
    @offset: 映射偏移量，通常设置为0
    @return: 映射地址，失败返回MAP_FAILED
*/

/*
    munmap 解除映射
    int munmap(void *addr, size_t length);
    @addr: 映射地址
    @length: 映射长度
    @return: 0 成功，-1 失败
    @desc: 解除映射
*/

int main(int argc, char const *argv[])
{
    // 1. 创建一个共享内存
    char shm_name[100] = {0};
    int fd;
    char *shm_addr;
    pid_t pid;

    sprintf(shm_name, "/shm_letter_%d", getpid());
    fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // 2. 截断共享内存,设置共享内存的大小
    ftruncate(fd,1024);

    // 3. 映射共享内存
    shm_addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shm_addr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // 映射完成之后关闭fd连接
    close(fd);

    // 4. 使用内存映射实现进程间通信
    pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        // 子进程
        // 写入共享内存
        strcpy(shm_addr, "Hello, Shared Memory!");
        printf("%d 写入共享内存: %s\n", getpid(), shm_addr);
    }
    else{
        // 父进程
        waitpid(pid, NULL, 0);
        // 读取共享内存
        printf("%d 共享内存内容: %s\n", getpid(), shm_addr);
    }

    // 5. 映射完成之后解除映射
    int ret = munmap(shm_addr, 1024);
    if(ret == -1){
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // 6. 释放共享内存对象
    shm_unlink(shm_name);
}