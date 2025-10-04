/*
    消息队列：生产者消费者模型
*/

#include "common.h"

int main(int argc, char const *argv[])
{
    // 创建消息队列
    struct mq_attr attr;

    // 有用的参数，表示消息队列的容量
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    // 被忽略的消息，在队列创建中暂时用不到
    attr.mq_flags = 0;
    attr.mq_curmsgs = 0;

    char *mq_name = "/product_consumer_mq_test";
    mqd_t mqdes = mq_open(mq_name,O_CREAT | O_RDWR,0666,&attr);

    if(mqdes == (mqd_t)-1){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 生产者：不断接收控制台中的数据，发送到消息队列
    char write_buf[100];
    struct timespec time_info;

    printf("生产者启动，请输入消息（按Ctrl+D退出）：\n");
    
    // 死循环：不断接收控制台中的数据，发送到消息队列, 直到收到EOF信号（ctrl + d）
    while(1){
        // 清空缓冲区
        memset(write_buf,0,sizeof(write_buf));

        ssize_t ret = read(STDIN_FILENO,write_buf,sizeof(write_buf));
        clock_gettime(CLOCK_REALTIME,&time_info);
        time_info.tv_sec += 5;

        if(ret == -1){
            perror("read");
            continue;
        }else if(ret == 0){
            // ctrl + d 退出,关闭控制台输入
            printf("ctrl + d 退出,关闭控制台输入\n");
            // 发送EOF信号到消息队列
            if(mq_timedsend(mqdes,"EOF",3,5,&time_info) == -1){
                perror("mq_timedsend");
            }
            break;
        }
        
        // 发送数据到消息队列（正常情况）
        if(mq_timedsend(mqdes,write_buf,ret,5,&time_info) == -1){
            perror("mq_timedsend");
        }else{
            printf("生产者：发送数据到消息队列：%s",write_buf);
        }
    }

    // 关闭消息队列
    mq_close(mqdes);

    // 删除消息队列
    mq_unlink(mq_name);
    return 0;
}