#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 9999

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main()
{
    int client_fd;          // 客户端套接字
    int conter = 10;
    int ret, bytesRead;     // 返回值
    char buf[BUFSIZ];

    struct sockaddr_in server_addr;              // 服务器地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);  // Presentation to Network 字符串->二进制
    
    // 创建套接字
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
        sys_err("socket error");
    
    // 建立连接
    ret = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
        sys_err("connect error");

    // 读写
    while (--conter) {
        write(client_fd, "hello\n", 6);                // 向socket写数据
        bytesRead = read(client_fd, buf, sizeof(buf));  // 读取服务器处理完后的数据
        write(STDOUT_FILENO, buf, bytesRead);           // 打印到终端
        sleep(1);
    }

    // 关闭
    close(client_fd);

    return 0;
}
