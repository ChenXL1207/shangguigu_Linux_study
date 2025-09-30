#include "common.h"
int main(int argc, char const *argv[])
{
    /* code */
    //使用标准库函数创建子线程
    /*
        extern int system (const char *__command) __wur;
        使用linux命令创建子线程
        return : 成功返回0，失败返回失败编号

        ps -ef 查看线程
    */
    int sysResult = system("ping -c 10 www.baidu.com");
    if(sysResult != 0){
        perror("system");
        exit(1);
    }
    return 0;
}
