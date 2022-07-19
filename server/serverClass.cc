#include "serverClass.h"
#include "net_wrap.h"

/*****************************************************************

                            LEVELDB

 ***************************************************************/

leveldb::DB *db;
leveldb::Options options;

void check_status(leveldb::Status status)
{
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }
}
/******************************************************************
 *
                            SERVER

******************************************************************/

vector<bool> Server::fd_arr(1000, false); //初始化vector

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
            Close(clie_fd);
        }
    }

    Close(link_fd);
}

bool Server::sign_up(int clie_fd)
{
    bool is_success = false;

    string ID;
    Reader rd;
    Value Jsinfo;

    char info[BUFSIZ];
    bool id_is_used = false;

    while (true)
    {
        if ((read(clie_fd, info, sizeof(info))) > 0)
        {
            break;
        }
    }

    if (rd.parse(info, Jsinfo))
    {
        ID = Jsinfo["ID"].asString();

        //遍历ID
        leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (ID == it->key().ToString())
            {
                Write(clie_fd, "fail", 4);
                id_is_used = true;
                break;
            }
        }

        // ID未被使用
        if (!id_is_used)
        {
            Write(clie_fd, "success", 7);
            leveldb::Status status = db->Put(leveldb::WriteOptions(), ID, info);
            check_status(status);

            string outpass;
            status = db->Get(leveldb::ReadOptions(), ID, &outpass);
            check_status(status);
            cout << "[数据库]" << ID << " : " << outpass << endl;

            is_success = true;
        }
    }
    return is_success;
}

void Server::thread_work(int clie_fd)
{
    // bool is_success = false;

    int ret;
    char r[BUFSIZ];

    /*
    执行读写(非阻塞,轮询)
    */
    while ((ret = read(clie_fd, r, sizeof(r))) > 0)
    {
        cout << clie_fd << " : " << r << endl;

        if (strcmp(r, EXIT) == 0)
        {
            cout << "----" << clie_fd << "已退出----" << endl;
            Close(clie_fd);

            fd_arr[clie_fd] == false;

            break;
        }
        else if (strcmp(r, SIGN_UP) == 0)
        {
            Server::sign_up(clie_fd);
        }
        bzero(r, sizeof(r));
    }
    return;
}

void Server::run()
{
    //数据库
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/serverdata/infoDB", &db);
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }

    //正向遍历
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());

    cout << "开始正向遍历:" << endl;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "key: " << it->key().ToString() << " value: " << it->value().ToString() << endl;
    }
    cout << "________________" << endl;

    //网络连接

    Server::fd_arr.resize(1000, false);

    int link_fd, clie_fd;
    link_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr, clie_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t clie_addr_len;
    clie_addr_len = sizeof(clie_addr);

    int opt = 1;
    setsockopt(link_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

    Bind(link_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    Listen(link_fd, 128);

    int ep_fd;
    ep_fd = Epoll_create(EPOLL_SIZE);

    struct epoll_event tep, ep[EPOLL_SIZE];
    tep.events = EPOLLIN | EPOLLOUT; // EPOLLET
    tep.data.fd = link_fd;

    Epoll_ctl(ep_fd, EPOLL_CTL_ADD, link_fd, &tep);

    while (true)
    {
        int n = Epoll_wait(ep_fd, ep, EPOLL_SIZE, 0);
        for (int i = 0; i < n; i++)
        {
            if (!(ep[i].events & EPOLLIN))
            {
                continue;
            }
            if (ep[i].data.fd == link_fd)
            {
                clie_fd = Accept(link_fd, (struct sockaddr *)&clie_addr, &clie_addr_len);

                int flag = fcntl(clie_fd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(clie_fd, F_SETFL, flag);

                tep.data.fd = clie_fd;
                Epoll_ctl(ep_fd, EPOLL_CTL_ADD, clie_fd, &tep);

                cout << "----" << clie_fd << "已连接----" << endl;
                fd_arr[clie_fd] = true;
            }
            else
            {

                int sockfd = ep[i].data.fd;

                thread chile_t(Server::thread_work, sockfd);
                chile_t.detach();
            }
        }
    }
    Close(link_fd);
    Close(ep_fd);

    return;
}