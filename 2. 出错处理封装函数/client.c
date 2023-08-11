#include "wrap.h"

#define SERVER_PORT 9527

int main()
{
    int client_fd;          // 客户端套接字
    int conter = 10;
    int bytesRead;     // 返回值
    char buf[BUFSIZ];

    struct sockaddr_in server_addr;              // 服务器地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);  // Presentation to Network 字符串->二进制
    
    // 创建套接字
    client_fd = Socket(AF_INET, SOCK_STREAM, 0);
    
    // 建立连接
    Connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 读写
    while (--conter) {
        Write(client_fd, "hello\n", 6);                // 向socket写数据
        bytesRead = Read(client_fd, buf, sizeof(buf));  // 读取服务器处理完后的数据
        Write(STDOUT_FILENO, buf, bytesRead);           // 打印到终端
        sleep(1);
    }

    // 关闭
    close(client_fd);

    return 0;
}
