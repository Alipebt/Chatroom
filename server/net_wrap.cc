#include "net_wrap.h"
#include "serverClass.h"

/***********************************************************************

                            封装错误处理函数

************************************************************************/

void Net::perr_exit(const char *str)
{
    perror(str);
    exit(1);
}

int Net::Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int n;
    n = accept(sockfd, addr, addrlen);
    if (n < 0)
    {
        perr_exit("accept失败");
    }
    return n;
}

int Net::Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int n;
    n = bind(sockfd, addr, addrlen);
    if (n < 0)
    {
        perr_exit("bind失败");
    }
    return n;
}

int Net::Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int n;
    n = connect(sockfd, addr, addrlen);
    if (n < 0)
    {
        perr_exit("connect失败");
    }
    return n;
}

int Net::Listen(int sockfd, int backlog)
{
    int n;
    n = listen(sockfd, backlog);
    if (n < 0)
    {
        perr_exit("listen失败");
    }
    return n;
}

int Net::Socket(int domain, int type, int protocol)
{
    int n;
    n = socket(domain, type, protocol);
    if (n < 0)
    {
        perr_exit("socket失败");
    }
    return n;
}

ssize_t Net::Read(int fd, void *buf, size_t count)
{
    int n;
    n = read(fd, buf, count);
    if (n == 0)
    {
        // perr_exit("read失败");
        close(fd);
        close(Server::fd_bor[fd]);
        cout << "关闭" << endl;
    }
    // else if (n == 0)
    // {
    //     Net::Close(fd);
    //     Net::Close(Server::fd_bor[fd]);
    // }
    return n;
}

ssize_t Net::Write(int fd, const void *buf, size_t count)
{
    int n;
    n = write(fd, buf, count);
    if (n < 0)
    {
        // int mainfd;
        perr_exit("write失败");
        // for (int i = 0; i < Server::fd_bor.size(); i++)
        // {
        //     if (Server::fd_bor[i] == fd)
        //     {
        //         mainfd = i;
        //     }
        // }
        // close(fd); // newfm
        // close(mainfd);
        // cout << "关闭" << endl;
        // close(fd);
        // close(Server::fd_bor[fd]);
    }
    return n;
}

int Net::Close(int fd)
{
    int n;
    n = close(fd);
    if (n < 0)
    {
        perr_exit("close失败");
    }
    return n;
}
/*
EPOLL
*/
int Net::Epoll_create(int size)
{
    int n;
    n = epoll_create(size);
    if (n < 0)
    {
        perr_exit("epoll_create失败");
    }
    return n;
}
int Net::Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    int n;
    n = epoll_wait(epfd, events, maxevents, timeout);
    if (n == -1)
    {
        perr_exit("epoll_wait失败");
    }
    return n;
}
int Net::Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    int n;
    n = epoll_ctl(epfd, op, fd, event);
    if (n == -1)
    {
        perr_exit("epoll_ctl失败");
    }
    return n;
}
