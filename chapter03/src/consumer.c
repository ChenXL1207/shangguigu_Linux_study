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

    // 消费者：不断接收消息队列中的数据
    char read_buf[100];

    printf("消费者启动，等待生产者发送消息...\n");
    
    // 死循环：不断接收消息队列中的数据, 直到收到EOF信号
    while(1){
        // 清空缓冲区
        memset(read_buf,0,sizeof(read_buf));
        
        // 使用阻塞式接收，不设置超时，这样会一直等待直到有消息
        ssize_t ret = mq_receive(mqdes,read_buf,sizeof(read_buf),NULL);
        if(ret == -1){
            perror("mq_receive");
            break;
        }
        
        // 判断当前数据是否是EOF信号
        if(strcmp(read_buf,"EOF") == 0 || read_buf[0] == EOF){
            printf("收到EOF信号，退出程序\n");
            break;
        }
        
        // 正常情况：打印消息队列中的数据
        printf("消费者：接收消息队列（生产者）中的数据：%s",read_buf);
    }

    // 关闭消息队列
    mq_close(mqdes);

    // 删除消息队列
    mq_unlink(mq_name);
    return 0;
}