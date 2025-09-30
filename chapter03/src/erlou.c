#include "common.h"
int main(int argc, char const *argv[])
{
    /* code */
    if (argc < 2){
        printf("parameter not enough\n");
        return 1;
    }
    printf("This process id is %d,my father process id is %d,start exec %s\n",getpid(),getppid(),argv[1]);
    
    return 0;
}
