#include "wrap.h"
#include <strings.h>

#define SRV_PORT 9999

void catch_child(int signum)        // 等待并处理已经退出的子进程
{
    while (waitpid(0, NULL, WNOHANG) > 0);
    return ;
}

int main()
{
    int lfd, cfd;
    pid_t pid;
    int ret, i;
    char buf[BUFSIZ];
    struct sockaddr_in srv_addr, clt_addr;
    socklen_t clt_addr_len;

    // memset(&srv_addr, 0, sizeof(srv_addr));         // 将地址结构清零
    bzero(&srv_addr, sizeof(srv_addr));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SRV_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);

    Bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));      // 绑定

    Listen(lfd, 128);                                               // 设置同一时刻连接服务器的上限数

    clt_addr_len = sizeof(clt_addr);

    while (1) {
        cfd = Accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len); // 阻塞监听客户端请求

        pid = fork();
        if (pid < 0)
            perr_exit("fork error");
        else if (pid == 0) {
            Close(lfd);
            break;
        } else {
            struct sigaction act;                   // 设置一个信号处理程序来处理子进程的退出信号
            
            act.sa_handler = catch_child;           // 信号处理函数
            sigemptyset(&act.sa_mask);              // 清空信号屏蔽集合
            act.sa_flags = 0;                       // 设置信号处理的标志

            ret = sigaction(SIGCHLD, &act, NULL);   // 子进程终止或暂停时发送给父进程的信号
            if (ret != 0)
                perr_exit("sigaction error");

            Close(cfd);
            continue;
        }
    }

    if (pid == 0) {                              // 子线程
        for (;;) {
            ret = Read(cfd, buf, sizeof(buf));   // 读客户端
            if (ret == 0) {
                Close(cfd);                     // 结束关闭 client
                exit(1);
            }

            for (i = 0; i < ret; i++)
                buf[i] = toupper(buf[i]);       // 小写 -> 大写

            Write(cfd, buf, ret);               // 回写到客户端
            Write(STDOUT_FILENO, buf, ret);     // 打印到屏幕
        }
    }

    return 0;
}
