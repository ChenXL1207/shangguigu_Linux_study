#include "common.h"

void *red_thread_func(void *arg);
void *white_thread_func(void *arg);

// 定义结构体，接收线程结果
typedef struct thread_result{
    char *p; // 接收线程结果
    int len; // 接收线程结果长度
}thread_result;

// 问题存在：如果一个进程的多个线程都获取到标准的输入输出，会造成冲突的问题，导致只有一个线程能够正常工作

int main(int argc, char *argv[]){
    // 案例： chenxl回复红玫瑰与白玫瑰
    // 创建两个线程，一个线程回复红玫瑰，一个线程回复白玫瑰
    // 线程1：回复红玫瑰
    // 线程2：回复白玫瑰
    // 线程1和线程2都回复完毕后，主线程打印结果
    // 线程1和线程2都使用pthread_exit函数退出
    // 主线程使用pthread_join函数等待线程1和线程2退出
    // 主线程使用pthread_exit函数退出

    pthread_t pid_red;
    pthread_t pid_white;

    char red_code = 'r';
    char white_code = 'w';

    thread_result *result_red = NULL;
    thread_result *result_white = NULL;

    // 创建红玫瑰线程
    pthread_create(&pid_red,NULL,red_thread_func,&red_code);
    // 创建白玫瑰线程
    pthread_create(&pid_white,NULL,white_thread_func,&white_code);

    // 打印结果
    pthread_join(pid_red,(void **)&result_red);
    pthread_join(pid_white,(void **)&result_white);

    printf("红玫瑰结果: %s\n",result_red->p);
    printf("白玫瑰结果: %s\n",result_white->p);

    free(result_red);
    free(result_white);
    return 0;
}

void *red_thread_func(void *arg){
    // 初始化结构体
    thread_result *result = (thread_result *)malloc(sizeof(thread_result));
    // 解析传递的参数
    char code = *(char *)arg;
    // 声明读取消息的字符串
    char *ans = malloc(1024);
    while(1){
        fgets(ans,1024,stdin);
        if(ans[0] == code){
            // 接收到回复消息
            free(ans);
            printf("红玫瑰回复完毕,离开...\n");
            char *resp = "红玫瑰收到回复，准备离开...\n";
            // 为result->p分配内存
            result->p = (char *)malloc(strlen(resp) + 1);
            strcpy(result->p,resp);
            result->len = strlen(resp);
            pthread_exit((void *)result);
        }else{
            printf("红玫瑰继续等待...\n");
        }
    }
}

void *white_thread_func(void *arg){
    // 初始化结构体
    thread_result *result = (thread_result *)malloc(sizeof(thread_result));
    // 解析传递的参数
    char code = *(char *)arg;
    // 声明读取消息的字符串
    char *ans = malloc(1024);
    while(1){
        fgets(ans,1024,stdin);
        if(ans[0] == code){
            // 接收到回复消息
            free(ans);
            printf("白玫瑰回复完毕,离开...\n");
            char *resp = "白玫瑰收到回复，准备离开...\n";
            // 为result->p分配内存
            result->p = (char *)malloc(strlen(resp) + 1);
            strcpy(result->p,resp);
            result->len = strlen(resp);
            pthread_exit((void *)result);
        }else{
            printf("白玫瑰继续等待...\n");
        }
    }
}