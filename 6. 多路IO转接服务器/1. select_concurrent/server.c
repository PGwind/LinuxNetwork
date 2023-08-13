#include "wrap.h"
#include <ctype.h>

#define SERV_PORT 9999

int main()
{
    int i, j, n, nready;

    int maxfd = 0;
    int listenfd, connfd;
    char buf[BUFSIZ];

    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);                         // 套接字

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 端口复用

    bzero(&serv_addr, sizeof(serv_addr));                               // 清零

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));  // 绑定
    
    Listen(listenfd, 128);                                              // 设置监听上限

    fd_set rset, allset;                                        // 定义 读集合、备份集合

    maxfd = listenfd;                                           // 最大文件描述符
        
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
            
            FD_SET(connfd, &allset);        // connfd 添加至监控描述符集合allset

            if (maxfd < connfd)
                maxfd = connfd;

            if ( nready == 1)              // 说明select 只返回一个 listenfd，后续的 for 不需要执行
                continue;
        }

        // 检测哪个 clients 有数据就绪
        for (i = listenfd + 1; i <= maxfd; i++) {             // 处理满足读事件的 fd
            if (FD_ISSET(i, &rset)) {
                if ((n = Read(i, buf, sizeof(buf))) == 0) {   // 当 client 关闭链接时，服务器端也关闭对应链接
                    Close(i);                                 
                    FD_CLR(i, &allset);                       // 解除 select 对此文件描述符的监控
                } else if (n > 0) {
                    for (j = 0; j <  n; j++)
                        buf[j] = toupper(buf[j]);
                    Write(i, buf, n);
                    Write(STDOUT_FILENO, buf, n);
                } else if (n == -1)
                    perr_exit("read error");
            }
        }
    }

    Close(listenfd);

    return 0;
}
