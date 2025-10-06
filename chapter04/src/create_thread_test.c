/*
    线程创建和同步
    线程定义：
        线程是进程中的一个执行单元，是进程中的一个实体，是CPU调度和分配的基本单位，是比进程更小的能独立运行的基本单位。
    
    线程创建函数：
    int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
        @param thread: 线程ID
        @param attr: 线程属性，默认设置为NULL
        @param start_routine: 线程函数，线程启动后执行的函数
        @param arg: 线程函数参数，线程函数需要传递的参数
        @return: 0成功，-1失败

    线程等待函数：
    int pthread_join(pthread_t thread, void **retval);
        @param thread: 要等待的线程ID
        @param retval: 用于接收线程返回值的指针，如果不需要返回值可以设置为NULL
        @return: 0成功，非0失败
        功能：阻塞调用线程，直到指定的线程结束。类似于进程中的wait()函数
        注意：每个线程只能被join一次，多次join会导致未定义行为

        pthread_t: 线程ID,属性一般设置为long int
*/    

#include "common.h"

#define BUF_LEN 1024

void *thread1_func(void *arg);
void *thread2_func(void *arg);

char *buf;

// 程序实现创建两个线程
// 线程1：不断接收控制台中的数据，发送到buf中
// 线程2：不断从buf中读取数据，打印到控制台

int main(int argc, char *argv[]){
    // 初始化buf
    buf = (char *)malloc(BUF_LEN); // 动态分配内存
    memset(buf,0,BUF_LEN); // 初始化buf
    // 声明线程id
    pthread_t pid_input;
    pthread_t pid_output;

    // 创建线程
    // 创建读线程
    pthread_create(&pid_input,NULL,thread1_func,NULL);
    // 创建写线程
    pthread_create(&pid_output,NULL,thread2_func,NULL);

    // 主线程等待子线程结束
    // pthread_join会阻塞主线程，直到指定的子线程执行完毕
    // 这里等待两个线程都结束，确保程序不会在主线程退出时强制终止子线程
    pthread_join(pid_input,NULL);  // 等待输入线程结束
    pthread_join(pid_output,NULL); // 等待输出线程结束

    // 释放buf
    free(buf);
    return 0;
}

// 读线程需要实现的代码逻辑
void *thread1_func(void *arg){
    int i = 0;
    while(1){
        char c = fgetc(stdin);
        if(c && c != '\n'){
            buf[i++] = c;
        }
        // 缓冲区索引溢出
        if(i >= BUF_LEN){
            i = 0;
        }
    }
}

// 写线程需要实现的代码逻辑
void *thread2_func(void *arg){
    int i = 0;
    while(1){
        if(buf[i]){
            // 读取一个字节写出控制台，之后换行
            fputc(buf[i],stdout);
            fputc('\n',stdout);
            buf[i++] = '\0';
            if(i >= BUF_LEN){
                i = 0;
            }
        }else{
            // 如果当前没有写出数据，则睡眠1秒
            sleep(1);
        }
    }
}