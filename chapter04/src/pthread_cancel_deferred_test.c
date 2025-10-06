#include "common.h"

// 点函数延迟取消，需要配合pthread_testcancel函数使用

void *thread_func(void *arg){
    printf("thread start running...\n");
    // 默认取消类型是延迟，异步取消
    printf("working...\n");
    sleep(1);
    // 人为调用取消点函数，在取消点之前，线程会执行取消点函数，之后线程会执行取消点之后的代码
    pthread_testcancel();
    printf("after pthread_testcancel...\n");
    return NULL;
}

int main(int argc, char *argv[]){
    // 创建线程
    pthread_t pid;
    pthread_create(&pid,NULL,thread_func,NULL);

    // 取消子线程
    if(pthread_cancel(pid) != 0){
        perror("pthread_cancel error");
        exit(1);
    }

    void *res;
    // pthread_cancel只是发送一个取消信号，不会立即取消线程，线程会在下一个取消点取消
    pthread_join(pid,&res);
    
    if(res == PTHREAD_CANCELED){
        printf("thread canceled...\n");
    }else{
        printf("thread not canceled exit code = %ld\n",(long)res);
    }
    
    return 0;
}