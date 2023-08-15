#include "wrap.h"
#include <strings.h>

#define MAXLINE 8192
#define SERV_PORT 9999

struct s_info {
    struct sockaddr_in cli_addr;
    int connfd;
};

void *do_work(void *arg)                        // 定义一个结构体，将地址结构跟cfd捆绑
{
    int n, i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];                  // #define INET_ADDRSTRLEN 16

    pthread_detach(pthread_self());          // 子线程分离，防止产生僵尸线程

    while (1) {
        n = Read(ts->connfd, buf, MAXLINE);     // 读客户端
        if (n == 0) {
            printf("the client %d closed...\n", ts->connfd);
            break;
        }
        printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &(*ts).cli_addr.sin_addr, str, sizeof(str)),
                ntohs((*ts).cli_addr.sin_port));    // 打印客户端信息

        for (i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);           // 小写->大写

        Write(STDOUT_FILENO, buf, n);           // 打印到屏幕
        Write(ts->connfd, buf, n);              // 回写到客户端
    }
    Close(ts->connfd);

    return (void *)0;                           // 这个位置与 pthread_exit(0) 作用一样
}


int main()
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_addr_len;
    int listenfd, connfd;
    pthread_t tid;

    struct s_info ts[256];      // 创建结构体数组
    int i = 0, ret;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));       // 地址结构清零
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));    // 绑定

    Listen(listenfd, 128);                                              // 设置同一时刻连接服务器的上限数

    printf("Accepting client connect...\n");

    while (1) {
        cli_addr_len = sizeof(cli_addr);
        connfd = Accept(listenfd, (struct sockaddr *)&cli_addr, &cli_addr_len);     // 阻塞监听客户端请求
        ts[i].cli_addr = cli_addr;
        ts[i].connfd = connfd;

        ret = pthread_create(&tid, NULL, do_work, (void *)&ts[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
            return 1;
        }

        i++;
    }

    return 0;
}
