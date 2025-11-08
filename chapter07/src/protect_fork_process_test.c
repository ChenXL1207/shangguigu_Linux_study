/*
守护进程测试
相关系统调用函数
1. setsid()
pid_t setsid(void);
@brief 创建一个新的会话
@param 无
@return 成功返回会话ID,失败返回-1
2. umask()
mode_t umask(mode_t mask);
@brief 设置文件权限掩码
@param 权限掩码
@return 成功返回0,失败返回-1
3. chdir()
int chdir(const char *path);
@brief 改变当前工作目录
@param 路径
@return 成功返回0,失败返回-1
4. openlog()
void openlog(const char *ident, int logopt, int facility);
@brief 打开日志文件
@param 日志文件名
@param 日志选项
@param 日志设施
@return 成功返回0,失败返回-1
5. syslog()
void syslog(int priority, const char *format, ...);
@brief 发送日志消息
@param 优先级
@param 格式化字符串
@param 可变参数
@return 无
6. closelog()
void closelog(void);
@brief 关闭日志文件
@param 无
@return 无
7. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    return 0;
}