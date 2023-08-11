#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 9527

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main()
{
    int sockfd = 0, client_fd = 0;     // 服务端和客户端 套接字文字描述符
    
    int ret;                        //  检测返回值
    int bytesRead;                   // 读到字节数
    char buf[BUFSIZ];               // BUFSIZ：_IO_BUFSIZ  大小4096
    char client_IP[1024];

    // 设置服务器地址结构
    struct sockaddr_in server_addr, client_addr;      
    socklen_t client_addr_len;                          //客户端套接字地址结构长度  

    server_addr.sin_family = AF_INET;                   // 设置了地址族为IPv4
    server_addr.sin_port  = htons(SERVER_PORT);         // 设置了服务器的端口号
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 设置服务器的IP地址

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        sys_err("socket error");

    // 绑定套接字
    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) 
        sys_err("bind error");

    // 设置服务器监听上限
    ret = listen(sockfd, 128);
    if (ret == -1)
        sys_err("listen error");

    // 阻塞监听客户端连接
    client_addr_len = sizeof(client_addr);

    client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);    // client_addr 是传出参数，c_a_l 是传入参数
    if (client_fd == -1)
        sys_err("accept error");
    printf("client ip:%s port:%d\n",
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
            ntohs(client_addr.sin_port));           // 打印客户端ip+端口
    
    while (1) {
        bytesRead = read(client_fd, buf, sizeof(buf));    // 读取客户端数据
        write(STDOUT_FILENO, buf, bytesRead);       // 输出到终端

        for (int i = 0; i < bytesRead; i++)
            buf[i] = toupper(buf[i]);       // 小写->大写

        write(client_fd, buf, bytesRead);           // 输出到客户端
    }

    close(sockfd);
    close(client_fd);

}
