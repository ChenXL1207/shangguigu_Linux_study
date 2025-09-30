#include "common.h"

int main(int argc, char const *argv[])
{
    // 跳转之前
    char *name = "chenxl";
    printf("before execve,name:%s,my process id is %d, my father process id is %d \n", name, getpid(),getppid());

    // 执行跳转
    /*
        int execve (const char *__path, char *const __argv[], char *const __envp[]) __THROWNL __nonnull ((1, 2));
        参数1：要跳转的程序路径,可执行文件路径
        参数2：给跳转程序传递的参数数组
        参数3：给跳转程序传递的环境变量数组
        return : 成功不返回，失败返回-1并设置errno

        argv 数组要求：
            1.第一个元素是程序名称 --> 程序的固定路径  （一定要传入的）
            2. 执行程序需要传入的参数 （不定长）
            2.最后一个元素必须是NULL

        envp 数组要求：
            1.传入的环境变量 （不定长） key = value
            2.最后一个元素必须是NULL

        跳转前后只有进程号保留，其他变量均删除
    */

    char *args_input[] = {
        "/home/xuanliang-chen/shangguigu_Linux/chapter03/build/erlou.out", // 可执行文件路径
        name,
        NULL};

    char *envp_input[] = {
        "PATH=/home/xuanliang-chen/shangguigu_Linux/chapter03/src/erlou.c",
        NULL
    };

    int re = execve(args_input[0], args_input, envp_input);

    if (re == -1)
    {
        perror("execve");
        exit(1);
    }

    // 此处的代码没有意义，因为已经跳转了程序
    return 0;
}
