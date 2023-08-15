#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "wrap.h"

#define SERV_ADDR "serv.socket"

int main()
{
    int lfd, cfd, len, size, i;
    struct sockaddr_un serv_addr, cli_addr;
    char buf[4096];

    lfd = Socket(AF_UNIX, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SERV_ADDR);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(serv_addr.sun_path);

    unlink(SERV_ADDR);      // 确保bind之前serv.sock文件不存在,bind会创建该文件
    Bind(lfd, (struct sockaddr *)&serv_addr, len);   // 参3不能是sizeof(servaddr)

    Listen(lfd, 20);

    printf("Accept ...\n");
    while (1) {
        len = sizeof(cli_addr);    // AF_UNIX大小+108B

        cfd = Accept(lfd, (struct sockaddr *)&cli_addr, (socklen_t *)&len);

        len -= offsetof(struct sockaddr_un, sun_path);    // 得到文件名的长度
        cli_addr.sun_path[len] = '\0';                   // 确保打印时,没有乱码出现

        printf("client bind filename %s\n", cli_addr.sun_path);

        while ((size = read(cfd, buf, sizeof(buf))) > 0) {
            for (i = 0; i < size; i++)
                buf[i] = toupper(buf[i]);
            write(cfd, buf, size);
        }
        close(cfd);
    }
    close(lfd);

    return 0;

}
