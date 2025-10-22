#include "common.h"
/*
socket函数
    作用：创建一个套接字
    参数：
        domain：地址族
        type：套接字类型
        protocol：协议
    返回值：成功返回套接字描述符，失败返回-1
bind函数
    作用：绑定地址
    参数：
        sockfd：套接字描述符
        addr：地址
        addrlen：地址长度
    返回值：成功返回0，失败返回-1
listen函数
    作用：监听套接字
    参数：
        sockfd：套接字描述符
        backlog： backlog：连接队列的最大长度
    返回值：成功返回0，失败返回-1
accept函数
    作用：接受连接
    参数：
        sockfd：套接字描述符
        addr：地址
        addrlen：地址长度
    返回值：成功返回套接字描述符，失败返回-1
*/

void *read_from_server(void *arg){
    // 使用recv接受客户端发送的数据，打印控制台
    char * read_buf = NULL;
    int client_socketfd = *(int *)arg;
    read_buf = malloc(sizeof(char)*1024);
    ssize_t count = 0;
    if(!read_buf){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    // 接收数据
    // 只要能接受数据，正常使用recv函数，直到recv函数返回0，表示客户端关闭连接
   while((count = recv(client_socketfd,read_buf,1024,0))){
    if(count > 0){
        printf("接收到数据: %s",read_buf);
        fputs(read_buf,stdout);
    }else if(count == 0){
        printf("客户端关闭连接\n");
    }else{
        perror("recv");
        exit(EXIT_FAILURE);
    }
    }
    free(read_buf);
    pthread_exit(NULL);
}

void *write_to_server(void *arg){
    char * write_buf = NULL;
    int client_socketfd = *(int *)arg;
    write_buf = malloc(sizeof(char)*1024);
    ssize_t count = 0;

    if(!write_buf){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    while(fgets(write_buf,1024,stdin) != NULL){
        // 发送数据
        count = send(client_socketfd,write_buf,strlen(write_buf),0);
        if(count > 0){
            printf("发送数据: %s",write_buf);
        }else if(count == 0){
            printf("客户端关闭连接\n");
        }else{
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
    printf("客户端关闭连接,不再写入，关闭连接\n");
    shutdown(client_socketfd,SHUT_WR);
    free(write_buf);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int client_socketfd;
    int bind_result;
    int connect_result;
    pthread_t pid_read;
    pthread_t pid_write;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    // 初始化服务器地址
    memset(&server_addr,0,sizeof(server_addr));
    // 初始化客户端地址
    memset(&client_addr,0,sizeof(client_addr));

    // 填写服务器地址
    server_addr.sin_family = AF_INET; // 地址族,AF_INET-->IPv4
    // 方法一：server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 填写IP地址
    // 方法二：inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 将字符串转换为网络字节序的IP地址
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); // 连接到本地服务器
    // 填写端口号，htons(6666)-->6666
    server_addr.sin_port = htons(6666); // 填写端口号

    // 客户端网络编程
    // 1.创建套接字
    client_socketfd = socket(AF_INET,SOCK_STREAM,0);
    handle_error("socket",client_socketfd);

    // 2. 绑定地址bind
    bind_result = bind(client_socketfd,(struct sockaddr *)&client_addr,sizeof(client_addr));
    if(bind_result < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 3.连接服务器
    connect_result = connect(client_socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(connect_result < 0){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    printf("与服务器%s %d建立链接 文件描述符是%d\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),client_socketfd);
    // 创建子线程用于收消息
    pthread_create(&pid_read,NULL,read_from_server,&client_socketfd);
    // 创建子线程用于发消息
    pthread_create(&pid_write,NULL,write_to_server,&client_socketfd);
    // 主线程等待子线程结束
    pthread_join(pid_read,NULL);
    pthread_join(pid_write,NULL);
    // 关闭套接字
    printf("释放资源\n");
    close(client_socketfd);
    return 0;
}

// inet_ntoa函数
// 作用：将网络字节序的IP地址转换为字符串
// 参数：
//     in：网络字节序的IP地址
// 返回值：字符串
// 示例：
//     inet_ntoa(192.168.1.1)-->"192.168.1.1"
//     inet_ntoa(10.0.0.1)-->"10.0.0.1"
//     inet_ntoa(127.0.0.1)-->"127.0.0.1"
//     inet_ntoa(0.0.0.0)-->"0.0.0.0"
//     inet_ntoa(255.255.255.255)-->"255.255.255.255"
//     inet_ntoa(192.168.1.1)-->"192.168.1.1"

// ntohs函数
// 作用：将网络字节序的端口号转换为主机字节序的端口号
// 参数：
//     net：网络字节序的端口号
// 返回值：主机字节序的端口号
// 示例：
//     ntohs(10000)-->10000
//     ntohs(10001)-->10001
//     ntohs(10002)-->10002
//     ntohs(10003)-->10003
//     ntohs(10004)-->10004
//     ntohs(10005)-->10005