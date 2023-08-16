#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <event2/event.h>
#include <sys/stat.h>

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

void write_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[] = "hello libevent";

    int len = write(fd, buf, strlen(buf)+1);

    sleep(1);

    return ;
}

int main(int argc, char *argv[])
{

    // 打开fifo的读端
    int fd = open("testfifo", O_WRONLY | O_NONBLOCK);
    if (fd == -1)
    {
        sys_err("open err");
    }

    // 创建 event_base
    struct event_base *base = event_base_new();

    //  创建 事件对象
    struct event *ev = NULL;
    ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);

    // 添加事件到 event_base 上
    event_add(ev, NULL);

    // 启动循环
    event_base_dispatch(base);          // while(1)


    // 销毁 event_base
    event_base_free(base);

}
