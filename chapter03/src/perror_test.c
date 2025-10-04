#include "common.h"
int main(int argc, char const *argv[])
{
    /* code */
    /*
    FILE *fopen(const char *filename, const char *mode);
    打开一个文件，返回一个文件指针，如果失败，返回NULL，并设置errno
    参数：
        filename：文件名
        mode：打开方式
    返回值：
        FILE *：文件指针
    打开方式：
        "r"：只读
        "w"：只写
        "a"：追加
        "r+"：读写
        "w+"：读写
    */
    fopen("test.txt", "r");
    perror("perror test");
    return 0;
}