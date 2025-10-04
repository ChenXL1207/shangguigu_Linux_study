#include "common.h"

/*
    mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
    @name: 消息队列名称
    @oflag: 打开方式 O_CREAT,O_RDWR,O_RDONLY
    @mode: 权限 0666
    @attr: 消息队列属性 通常设置为NULL
    @return: 消息队列描述符，失败返回-1
    @desc: 打开一个消息队列，如果消息队列不存在，则创建一个消息队列
    
    typdef int mqd_t; 用来记录消息队列描述符，实际上是int类型
*/

/*
    int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
    @mqdes: 消息队列描述符
    @msg_ptr: 消息指针
    @msg_len: 消息长度
    @msg_prio: 消息优先级
    @return: 成功返回0，失败返回-1
    @desc: 发送一个消息到消息队列

    int mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
    @mqdes: 消息队列描述符
    @msg_ptr: 消息指针
    @msg_len: 消息长度
    @msg_prio: 消息优先级
    @return: 成功返回0，失败返回-1
    @desc: 接收一个消息从消息队列

    int mq_close(mqd_t mqdes);
    @mqdes: 消息队列描述符
    @return: 成功返回0，失败返回-1
    @desc: 关闭一个消息队列

    int mq_unlink(const char *name);
    @name: 消息队列名称
    @return: 成功返回0，失败返回-1
    @desc: 删除一个消息队列

    int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
    @mqdes: 消息队列描述符
    @attr: 消息队列属性
    @return: 成功返回0，失败返回-1
    @desc: 获取一个消息队列的属性

    int mq_setattr(mqd_t mqdes, struct mq_attr *attr, struct mq_attr *oldattr);
    @mqdes: 消息队列描述符
    @attr: 消息队列属性
    @oldattr: 旧的消息队列属性
    @return: 成功返回0，失败返回-1
    @desc: 设置一个消息队列的属性

    int mq_notify(mqd_t mqdes, const sigset_t *sigset);
    @mqdes: 消息队列描述符
    @sigset: 信号集
    @return: 成功返回0，失败返回-1
    @desc: 设置一个消息队列的信号集

    int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abs_timeout);
    @mqdes: 消息队列描述符
    @msg_ptr: 消息指针
    @msg_len: 消息长度
    @msg_prio: 消息优先级
    @abs_timeout: 绝对时间
    @return: 成功返回0，失败返回-1
    @desc: 发送一个消息到消息队列，如果消息队列满了，则等待一段时间
    
    mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
    @name: 消息队列名称
    @oflag: 打开方式 O_CREAT,O_RDWR,O_RDONLY
    @mode: 权限 0666
    @attr: 消息队列属性 通常设置为NULL
    @return: 消息队列描述符，失败返回-1
    @desc: 打开一个消息队列，如果消息队列不存在，则创建一个消息队列

    attr.mq_maxmsg: 消息队列的容量
    attr.mq_msgsize: 消息的最大长度
    attr.mq_flags: 消息队列的标志
    attr.mq_curmsgs: 当前消息队列中的消息数
*/

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

    char *mq_name = "/father_son_mq_test";
    mqd_t mqdes = mq_open(mq_name,O_CREAT | O_RDWR,0666,&attr);

    if(mqdes == (mqd_t)-1){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 创建父子进程
    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }else if(pid == 0){
        // 子进程 等待接收消息队列中的消息
        char recv_buf[100]; // 对应attr.mq_msgsize
        struct timespec time_info;
        
        for(int i = 0; i < 10; i++){
            // 清空接收缓冲区
            memset(recv_buf,0,sizeof(recv_buf));

            clock_gettime(CLOCK_REALTIME,&time_info);
            time_info.tv_sec += 5;

            // 设置接收时间
            int ret = mq_timedreceive(mqdes,recv_buf,sizeof(recv_buf),NULL,&time_info);
            if(ret == -1){
                perror("mq_timedreceive");
            }else{
                printf("子进程(pid=%d) 第%d次接收到消息: %s\n", getpid(), i+1, recv_buf);
            }
        }
    }
    else{
        // 父进程 发送消息到消息队列
        char send_buf[100]; // 对应attr.mq_msgsize
        struct timespec time_info;
        
        for(int i = 0; i < 10; i++){
            // 清空发送缓冲区
            memset(send_buf,0,sizeof(send_buf));
            sprintf(send_buf,"Hello from father process! (pid=%d, round=%d)", getpid(), i+1);

            // 设置发送时间(获取当前具体时间)
            clock_gettime(CLOCK_REALTIME,&time_info);
            time_info.tv_sec += 5;

            int ret = mq_timedsend(mqdes,send_buf,strlen(send_buf),0,&time_info);
            if(ret == -1){
                perror("mq_timedsend");
            }else{
                printf("父进程(pid=%d) 第%d次发送消息: %s\n", getpid(), i+1, send_buf);
            }
            
            sleep(1);
        }
        
        // 等待子进程完成
        waitpid(pid,NULL,0);
    }

    // 释放消息队列，不管是父进程还是子进程，都要释放消息队列
    mq_close(mqdes);
    // 删除消息队列
    if(pid>0){
        mq_unlink(mq_name);
    }
    
}
