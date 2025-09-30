#include "common.h"
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    /* code */
    // fork 之前
    int subprocess_status;

    pid_t pid = fork();
    if(pid < 0){
        perror("fork error");
        exit(1);
    }else if(pid == 0){
        // child
        char *args[] = {
            "/usr/bin/ping",
            "-c",
            "10",
            "www.baidu.com",
            NULL
        };
        char *envp[] = {NULL};
        printf("start execve\n");
        int ExeRet = execve(args[0], args, envp);
        if(ExeRet < 0){
            perror("execve error");
            exit(1);
        }
        printf("end execve\n");
    }else{
        // 父进程
        printf("parent process id is %d, child pid process = %d\n", getpid(),pid);
        /*
            waitpid - 阻塞等待子进程退出，并回收资源
            参数1：pid - 等待的子进程id
                -1：等待任意子进程退出
                >0：等待指定pid的子进程退出
                0：等待与调用进程同一进程组的子进程退出
                <-1：等待进程组id等于参数绝对值的所有子进程退出
            参数2：status - 用于存储子进程的退出状态信息
            参数3：options - 选项，一般传0
        作用：1.阻塞等待子进程退出 2.回收子进程资源
        进程退出的两种方式：
            1.正常退出 exit/_exit/return
            2.异常退出：收到信号导致进程终止
        进程退出的状态信息存储在status中，需要使用宏来解析
            1.正常退出：WIFEXITED(status) - 判断是否正常退出
                        WEXITSTATUS(status) - 获取正常退出的返回值
            2.异常退出：WIFSIGNALED(status) - 判断是否异常退出
                        WTERMSIG(status) - 获取导致异常退出的信号编号
        资源回收：子进程退出后，内核会保留子进程的部分信息，称为僵尸进程
                通过wait/waitpid函数可以回收子进程资源，避免僵尸进程
        备注：如果不回收子进程资源，子进程会变成僵尸进程，占用系统资源
            僵尸进程会一直存在，直到父进程调用wait/waitpid回收资源
            如果父进程先于子进程退出，子进程会被init进程收养，init进程会负责回收资源
        进程状态：
            1.运行态(Running) - 进程正在CPU上运行
            2.就绪态(Ready) - 进程准备好运行，等待CPU调度
            3.阻塞态(Blocked) - 进程等待某个事件发生，如I/O操作完成
            4.僵尸态(Zombie) - 进程已经退出，但父进程尚未回收资源
        进程状态转换：
            1.运行态 -> 就绪态 - 时间片用完，CPU调度其他进程
            2.就绪态 -> 运行态 - CPU调度该进程运行
            3.运行态 -> 阻塞态 - 进程等待I/O等事件
            4.阻塞态 -> 就绪态 - 等待事件完成，进程准备好运行
            5.运行态 -> 僵尸态 - 进程调用exit等退出
            6.僵尸态 -> 资源回收 - 父进程调用wait/waitpid回收资源
        进程调度：
            1.时间片轮转(Round Robin) - 每个进程分配固定时间片，时间片用完切换到下一个进程
            2.优先级调度(Priority Scheduling) - 根据进程优先级调度，高优先级进程优先运行
            3.多级反馈队列(Multilevel Queue) - 多个队列，不同优先级队列，进程根据行为动态调整优先级
        */
        waitpid(pid, &subprocess_status, 0);
    }
    printf("old process wait new process end\n");
    return 0;
}
