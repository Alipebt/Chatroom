#include "serverClass.h"
#include "net_wrap.h"

/******************************************************************
 *
                            SERVER

******************************************************************/

vector<bool> Server::fd_arr(1000, false); //初始化vector
vector<bool> Server::fd_pthread(1000, false);
vector<bool> Server::fd_in(1000, false);
vector<bool> Server::fd_new(1000, false);
vector<string> Server::fd_ID(1000, "0");
vector<int> Server::fd_bor(1000, 0);

int Server::imas = 0;

pthread_mutex_t Server::mutex;
vector<pthread_mutex_t> Server::fd_mutex(1000, mutex);

leveldb::Options Server::opt;
leveldb::DB *Server::IPdb;
leveldb::DB *Server::Mdb;
leveldb::DB *Server::Fdb;
leveldb::DB *Server::Gdb;
leveldb::DB *Server::GMdb;
leveldb::DB *Server::NMdb;

//构造函数传入接口与ip
Server::Server(int port, string ip) : server_port(port), server_ip(ip){};

//服务器销毁前先关闭红黑树上的套接字
Server::~Server()
{
    // for(用于迭代的变量:迭代的范围)

    for (auto clie_fd : fd_arr)
    {
        if (fd_arr[clie_fd])
        {
            Net::Close(clie_fd);
        }
    }

    Net::Close(link_fd);
}

void Server::check_status(leveldb::Status status)
{
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }
}

/*

     执行读写(EPOLL ET非阻塞,轮询)

 */

void Server::thread_work(int clie_fd)
{
    bool exit = false;
    // bool main_menu_in = false;
    //菜单
    while (true)
    {
        exit = sign_menu(clie_fd);

        if (exit)
        {
            fd_pthread[clie_fd] = false;

            pthread_exit((void *)"客户端关闭");
        }
        //主页面
    }

    return;
}

void Server::run()
{

    //数据库
    opt.create_if_missing = true;

    leveldb::Status s1 = leveldb::DB::Open(opt, "/tmp/serverdata/infoDB", &IPdb);
    if (!s1.ok())
    {
        cerr << s1.ToString() << endl;
    }

    leveldb::Status s2 = leveldb::DB::Open(opt, "/tmp/serverdata/massage", &Mdb);
    if (!s2.ok())
    {
        cerr << s2.ToString() << endl;
    }

    leveldb::Status s3 = leveldb::DB::Open(opt, "/tmp/serverdata/friends", &Fdb);
    if (!s3.ok())
    {
        cerr << s3.ToString() << endl;
    }

    leveldb::Status s4 = leveldb::DB::Open(opt, "/tmp/serverdata/group", &Gdb);
    if (!s4.ok())
    {
        cerr << s4.ToString() << endl;
    }

    leveldb::Status s5 = leveldb::DB::Open(opt, "/tmp/serverdata/groupmassage", &GMdb);
    if (!s5.ok())
    {
        cerr << s5.ToString() << endl;
    }
    leveldb::Status s6 = leveldb::DB::Open(opt, "/tmp/serverdata/newmassage", &NMdb);
    if (!s6.ok())
    {
        cerr << s6.ToString() << endl;
    }

    //网络连接

    int link_fd, clie_fd;
    string send_fd;

    link_fd = Net::Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr, clie_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    socklen_t clie_addr_len;
    clie_addr_len = sizeof(clie_addr);

    int opt = 1;
    setsockopt(link_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

    Net::Bind(link_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    Net::Listen(link_fd, 128);

    int ep_fd;
    ep_fd = Net::Epoll_create(EPOLL_SIZE);

    struct epoll_event tep, ep[EPOLL_SIZE];
    tep.events = EPOLLIN | EPOLLOUT; // EPOLLET
    tep.data.fd = link_fd;

    Net::Epoll_ctl(ep_fd, EPOLL_CTL_ADD, link_fd, &tep);

    int newmas = 1;

    while (true)
    {
        int n = Net::Epoll_wait(ep_fd, ep, EPOLL_SIZE, 0);
        for (int i = 0; i < n; i++)
        {
            if (!(ep[i].events & EPOLLIN))
            {
                continue;
            }
            if (ep[i].data.fd == link_fd)
            {

                clie_fd = Net::Accept(link_fd, (struct sockaddr *)&clie_addr, &clie_addr_len);

                int keep_alive = 1;
                int keep_idle = 3;
                int keep_interval = 1;
                int keep_count = 10;
                if (setsockopt(clie_fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)))
                {
                    perror("Error setsockopt(SO_KEEPALIVE) failed");
                    exit(1);
                }
                if (setsockopt(clie_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle)))
                {
                    perror("Error setsockopt(TCP_KEEPIDLE) failed");
                    exit(1);
                }
                if (setsockopt(clie_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)))
                {
                    perror("Error setsockopt(TCP_KEEPINTVL) failed");
                    exit(1);
                }
                if (setsockopt(clie_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)))
                {
                    perror("Error setsockopt(TCP_KEEPCNT) failed");
                    exit(1);
                }

                int flag = fcntl(clie_fd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(clie_fd, F_SETFL, flag);

                tep.data.fd = clie_fd;
                Net::Epoll_ctl(ep_fd, EPOLL_CTL_ADD, clie_fd, &tep);

                cout << "----" << clie_fd << "已连接----" << endl;
                fd_arr[clie_fd] = true;

                if (newmas % 2 == 0)
                {
                    fd_new[clie_fd] = true;
                }
                newmas++;
                cout << "$" << clie_fd << fd_new[clie_fd] << endl;
            }
            else if (!fd_pthread[ep[i].data.fd] && fd_arr[clie_fd] && !fd_new[ep[i].data.fd])
            {
                int sockfd = ep[i].data.fd;

                thread chile_t(Server::thread_work, sockfd);
                cout << "为" << sockfd << "创建线程" << endl;
                fd_pthread[sockfd] = true;

                if (fd_new[ep[i + 1].data.fd] && fd_arr[ep[i].data.fd])
                {
                    int sockfdnew = ep[i + 1].data.fd;
                    fd_bor[sockfd] = sockfdnew; // bor

                    thread chile_tnm(Server::thread_worknm, sockfdnew, sockfd);

                    cout << "为" << sockfdnew << "创建新消息" << endl;
                    fd_pthread[sockfdnew] = true;

                    chile_tnm.detach();
                }

                chile_t.detach();
            }
        }
    }
    Net::Close(link_fd);
    Net::Close(ep_fd);

    return;
}