#ifndef _NET_WRAP_H_
#define _NET_WRAP_H_

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <thread>
#include <algorithm>
#include "json/json.h"

#include <vector>

using namespace Json;
using namespace std;

#define EPOLL_SIZE 500 // epoll的大小
#define SERV_PORT 5000 //端口号

class Net
{
public:
    static void perr_exit(const char *s);
    static int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    static int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    static int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    static int Listen(int sockfd, int backlog);
    static int Socket(int domain, int type, int protocol);
    static ssize_t Read(int fd, void *buf, size_t count);
    static ssize_t Write(int fd, const void *buf, size_t count);
    static int Close(int fd);

    static int Epoll_create(int size);
    static int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    static int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
};

#endif