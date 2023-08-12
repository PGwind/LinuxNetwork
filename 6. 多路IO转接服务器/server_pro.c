#include "wrap.h"
#include <ctype.h>

#define SERV_PORT 9999

int main()
{
    int i, j, n, maxi;

    int nready, client[FD_SETSIZE];             // 自定义数组client，防止遍历1024个文件描述符  FD_SETSIZE 默认为1024
    int maxfd, listenfd, connfd, sockfd;
    char buf[BUFSIZ], str[INET_ADDRSTRLEN];     // #define INET_ADDRSTRLEN 16

    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;
    fd_set rset, allset;                                        // 定义 读集合、备份集合

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);                         // 套接字

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 端口复用

    bzero(&serv_addr, sizeof(serv_addr));                               // 清零

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));  // 绑定
    
    Listen(listenfd, 128);                                              // 设置监听上限


    maxfd = listenfd;                                           // 最大文件描述符
    
    maxi = -1;                                                  // 将来用作client[]的下标，初始值指向0个元素之前下标位置   
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;                                         // client[] 初始化 -1

    FD_ZERO(&allset);                                           // 清空 监听集合
    FD_SET(listenfd, &allset);                                  // 将待监听fd添加到监听集合

    while (1) {
        rset = allset;                                          // 备份
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);      // select 监听文件描述符
        if (nready < 0)
            perr_exit("select error");

        if (FD_ISSET(listenfd, &rset)) {
            clie_addr_len = sizeof(clie_addr);
            connfd = Accept(listenfd, (struct sockaddr *)&clie_addr, &clie_addr_len);       // Accept不会阻塞
            printf("Received from %s at PORT %d\n",
                    inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),
                    ntohs(clie_addr.sin_port));
            
            for (i = 0; i < FD_SETSIZE; i++) 
                if (client[i] < 0) {                                // 找到 client[] 中没有使用的位置
                    client[i] = connfd;                             // 保存 accept 返回的文件描述符到 client[] 里
                    break;
                }

            if (i == FD_SETSIZE) {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            FD_SET(connfd, &allset);        // connfd 添加至监控描述符集合allset

            if (maxfd < connfd)
                maxfd = connfd;

            if (i > maxi)
                maxi = i;                  // 保证 maxi 存的总是 client[] 最后一个元素下标

            if (--nready == 0)              // 说明select 只返回一个 listenfd，后续的 for 不需要执行
                continue;
        }

        // 检测哪个 clients 有数据就绪
        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset)) {
                if ((n = Read(sockfd, buf, sizeof(buf))) == 0) {    // 当 client 关闭链接时，服务器端也关闭对应链接
                    Close(sockfd);                                 
                    FD_CLR(sockfd, &allset);                        // 解除 select 对此文件描述符的监控
                    client[i] = -1;
                } else if (n > 0) {
                    for (j = 0; j <  n; j++)
                        buf[j] = toupper(buf[j]);
                    Write(sockfd, buf, n);
                    Write(STDOUT_FILENO, buf, n);
                } else if (n == -1)
                    perr_exit("read error");

                if (--nready == 0)                                    // 说明只剩下 listenfd 
                    break;
            }
        }
    }

    Close(listenfd);

    return 0;
}
