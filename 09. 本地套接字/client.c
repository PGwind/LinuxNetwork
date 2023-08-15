#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>

#include "wrap.h"

#define SERV_ADDR "serv.socket"
#define CLIE_ADDR "clie.socket"

int main(void)
{
    int  cfd, len;
    struct sockaddr_un serv_addr, cli_addr;
    char buf[4096];

    cfd = Socket(AF_UNIX, SOCK_STREAM, 0);

    bzero(&cli_addr, sizeof(cli_addr));
    cli_addr.sun_family = AF_UNIX;
    strcpy(cli_addr.sun_path,CLIE_ADDR);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(cli_addr.sun_path);     /* 计算客户端地址结构有效长度 */

    unlink(CLIE_ADDR);
    Bind(cfd, (struct sockaddr *)&cli_addr, len);                                 /* 客户端也需要bind, 不能依赖自动绑定*/


    bzero(&serv_addr, sizeof(serv_addr));                                          /* 构造server 地址 */
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SERV_ADDR);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(serv_addr.sun_path);   /* 计算服务器端地址结构有效长度 */

    Connect(cfd, (struct sockaddr *)&serv_addr, len);

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        write(cfd, buf, strlen(buf));
        len = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);
    }

    close(cfd);

    return 0;
}

