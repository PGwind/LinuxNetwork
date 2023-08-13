#include "wrap.h"
#include <poll.h>

#define MAXLINE 80
#define SERV_PORT 9999
#define OPEN_MAX 1024

int main()
{
    int i, j, maxi, listenfd, connfd, sockfd;
    int nready;                                 // 监听poll返回值，记录满足监听事件的fd个数
    ssize_t n;

    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t cli_len;

    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cli_addr, serv_addr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    Listen(listenfd, 128);

    client[0].fd = listenfd;                    // 要监听的第一个文件描述符，存入client[0]
    client[0].events = POLLIN;                 // listenfd 监听普通读事件

    for (i = 1; i< OPEN_MAX; i++)
        client[i].fd = -1;                      // 用 -1 初始化client[]里剩下的元素，0也是文件描述符，不能用

    maxi = 0;                                   // client[] 数组有效元素中最大元素下标

    for ( ; ; ) {
        nready = poll(client, maxi+1, -1);      // 阻塞监听是否有客户端链接请求

        if (client[0].revents & POLLIN) {       // listenfd 有读事件就绪
            cli_len = sizeof(cli_addr);
            connfd = Accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);      // Accept 不阻塞
            printf("Received from %s at PORT %d\n",
                    inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
                    ntohs(cli_addr.sin_port));

            for (i = 1; i < OPEN_MAX; i++)
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }

            if (i == OPEN_MAX)                  // 到达客户端数量上限
                perr_exit("too many clients");

            client[i].events = POLLIN;          // 设置刚刚返回的connfd，监控读事件

            if (i > maxi)
                maxi = i;                       // 更新下标
            if (--nready <= 0)                  // 没有就绪事件，继续回到poll阻塞
                continue;
        }

        for (i = 1; i <= maxi; i++) {           // 前面if没满足，说明没有listenfd满足，检测client[]，看是哪个connfd就绪
            if ((sockfd = client[i].fd) < 0)
                continue;

            if (client[i].revents & POLLIN) {
                if ((n = Read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {                  // 收到RST标志
                        printf("client[%d] aborted connection\n", i);
                        Close(sockfd);
                        client[i].fd = -1;                      // poll中不监控该文件描述符
                    } else 
                        perr_exit("read error");
                }  else if (n == 0) {                        // 说明客户端先关闭连接
                    printf("client[%d] closed connection\n", i);
                    Close(sockfd);
                    client[i].fd = -1;
                } else {
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    Writen(sockfd, buf, n);
                    Writen(STDOUT_FILENO, buf, n);
                }
                if (--nready <= 0)
                    break;

            }
        }
    }

    return 0;
}
