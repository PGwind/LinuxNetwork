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

void read_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[BUFSIZ] = {0};
    int len = read(fd, buf, sizeof(buf));

    printf("what = %s, read from write: %s\n", 
            what & EV_READ ? "read 满足":"read 不满足", buf);
    sleep(1);

    return ;
}

int main(int argc, char *argv[])
{
    // 创建 fifo
    unlink("testfifo");
    mkfifo("testfifo", 0644);

    // 打开fifo的读端
    int fd = open("testfifo", O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        sys_err("open err");
    }

    // 创建 event_base
    struct event_base *base = event_base_new();

    //  创建 事件对象
    struct event *ev = NULL;
    ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

    // 添加事件到 event_base 上
    event_add(ev, NULL);

    // 启动循环
    event_base_dispatch(base);          // while(1)


    // 销毁 event_base
    event_base_free(base);

}
