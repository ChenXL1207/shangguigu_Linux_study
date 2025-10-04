#include "common.h"
int main(int argc, char const *argv[])
{
    /* code */
    // 跳转之前
    char *name = "chenxl";
    printf("before execve,name:%s,my process id is %d \n",name,getpid());

    pid_t pid  = fork();

    if(pid < 0){
        perror("fork");
        exit(1);
    }else if(pid == 0){
        // 子进程
        char *new_name = "ergou";
        char *args[] = {
            // "/home/xuanliang-chen/shangguigu_Linux/chapter03/build/erlou.out",
            "/home/chenx/Linux_study_shangguigu/linux_shangguigu/chapter03/build/erlou_block.out",
            new_name,
            NULL
        };
        char *envp[] = {
            NULL
        };
        int exRes = execve(args[0],args,envp);
        if(exRes == -1){
            perror("execve");
            exit(1);
        }
    }else{
        // 父进程
        printf("I am father process, my id is %d, my child process id is %d\n",getpid(),pid);
    }

    return 0;
}
