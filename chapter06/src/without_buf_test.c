#include "common.h"

int main(int argc, char *argv[]) {
    FILE *fp = fopen("test.txt", "w"); // 打开文件
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    // 方法二：修改刷写模式
    // if(setvbuf(fp, NULL, _IONBF, 0) != 0){
    // 测试行缓冲，遇到换行符或者写满才能写出
    if(setvbuf(fp,NULL,_IOLBF,0) != 0){
        perror("setvbuf");
    }

    // 文件写入是全缓冲，等待缓冲区满或者遇到换行符才会写入文件
    fprintf(fp, "Hello, World!\n"); // 写入数据
    // 此时是没有写入数据的，原因：stdio 对文件流默认为全缓冲，未遇到换行符且缓冲未满；同时未调用 fflush/fclose 刷新；随后调用 execve 会直接替换进程镜像，不会执行退出处理与缓冲刷新。
    // 方法一：可以使用手动刷写
    // fflush(fp);


    char *args[] = {"/usr/bin/ping","-c","10","www.baidu.com",NULL};
    char *envp[] = {NULL};
    execve(args[0], args, envp);

    // 如果跳转失败
    perror("execve");
    return 0;
}