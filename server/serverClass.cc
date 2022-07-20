#include "serverClass.h"
#include "net_wrap.h"

/*****************************************************************

                            LEVELDB

 ***************************************************************/

leveldb::DB *IPdb;
leveldb::Options IPopt;

leveldb::DB *Mdb;
leveldb::Options Mopt;

void check_status(leveldb::Status status)
{
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }
}

/****************************************************************

 *                      MUTEX

 ***************************************************************/
pthread_mutex_t mutex;
/******************************************************************
 *
                            SERVER

******************************************************************/

vector<bool> Server::fd_arr(1000, false); //初始化vector
vector<bool> Server::fd_in(1000, false);
vector<string> Server::fd_ID(1000, "0");

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
/**
 *
 *
 *   登录，注册操作
 *
 *
 */
bool Server::sign_in(int clie_fd)
{
    bool is_success = false;

    string ID, pass;
    Reader rd;
    Value Jsinfo, JsinfoDB;

    char info[BUFSIZ];

    while (true)
    {
        if ((read(clie_fd, info, sizeof(info))) > 0)
        {
            cout << "[客户端] " << clie_fd << " : " << info << endl;
            break;
        }
    }

    if (rd.parse(info, Jsinfo))
    {
        ID = Jsinfo["ID"].asString();
        pass = Jsinfo["pass"].asString();

        //遍历ID
        leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (ID == it->key().ToString() && rd.parse(it->value().ToString(), JsinfoDB))
            {
                if (rd.parse(it->value().ToString(), JsinfoDB))
                {
                    if (pass == JsinfoDB["pass"].asString())
                    {
                        //登录成功

                        Write(clie_fd, "success", 7);
                        fd_ID[clie_fd] = ID;
                        is_success = true;
                    }
                }
                //(测试用)
                string outpass;
                leveldb::Status status = IPdb->Get(leveldb::ReadOptions(), ID, &outpass);
                check_status(status);
                cout << "[数据库] " << ID << " : " << outpass << endl;
            }
        }

        // ID密码不匹配
        if (!is_success)
        {
            Write(clie_fd, "fail", 4);

            is_success = false;
        }
    }
    return is_success;
}

void Server::sign_up(int clie_fd)
{

    string ID;
    Reader rd;
    Value Jsinfo;

    char info[BUFSIZ];
    bool id_is_used = false;

    while (true)
    {
        if ((read(clie_fd, info, sizeof(info))) > 0)
        {
            cout << " [客户端]" << clie_fd << ":" << info << endl;
            break;
        }
    }

    if (rd.parse(info, Jsinfo))
    {
        ID = Jsinfo["ID"].asString();

        //遍历ID
        leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());
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
            leveldb::Status status = IPdb->Put(leveldb::WriteOptions(), ID, info);
            check_status(status);

            //(测试用)
            string outpass;
            status = IPdb->Get(leveldb::ReadOptions(), ID, &outpass);
            check_status(status);
            cout << "[数据库]" << ID << " : " << outpass << endl;
        }
    }
    return;
}
/**
 *
 *  登录后操作
 *
 */
void Server::match_with(int clie_fd)
{
    Reader rd;
    Value match;
    string recverID;

    char r[BUFSIZ];

    bool is_match = false;

    while (true)
    {
        if ((read(clie_fd, r, sizeof(r))) > 0)
        {
            cout << " [客户端]" << clie_fd << ":" << r << endl;
            break;
        }
    }

    if (rd.parse(r, match))
    {
        recverID = match["recver"].asString();
        //查找所连接的ID是否存在
        leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (recverID == it->key().ToString())
            {
                Write(clie_fd, "success", 7);
                cout << clie_fd << "与" << recverID << "匹配成功" << endl;
                is_match = true;

                thread send(thread_send, clie_fd, recverID);
                thread recv(thread_recv, clie_fd, recverID);

                send.join();
                recv.join();
                cout << "已退出连接" << endl;
                break;
            }
        }
    }
    else
    {
        cout << "解析失败" << endl;
    }
    if (!is_match)
    {
        Write(clie_fd, "fail", 4);
        cout << clie_fd << "与" << recverID << "匹配失败" << endl;
    }
}

void Server::thread_recv(int clie_fd, string recverID)
{
    Value all_massage;
    Value match_massage;
    match_massage["sender"] = fd_ID[clie_fd];

    Reader rd;
    FastWriter w;

    string send_to_db;
    string oldmassage;

    cout << "sender:" << fd_ID[clie_fd] << endl;
    cout << "recver:" << recverID << endl;

    char r[BUFSIZ];
    while (true)
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            pthread_mutex_lock(&mutex); //加锁

            //取到之前的记录
            leveldb::Status status = Mdb->Get(leveldb::ReadOptions(), recverID, &oldmassage);
            if (status.ok())
            {
                rd.parse(oldmassage, all_massage);
            }

            cout << "massage:" << r << endl;
            match_massage["massage"] = r;
            //加入新记录
            all_massage.append(match_massage);

            send_to_db = w.write(all_massage);
            //发送到数据库
            Mdb->Put(leveldb::WriteOptions(), recverID, send_to_db);

            pthread_mutex_unlock(&mutex); //解锁

            //(测试用)
            string outpass;
            Mdb->Get(leveldb::ReadOptions(), recverID, &outpass);
            cout << "////[数据库]" << recverID << " : " << outpass << endl;
        }
        bzero(r, sizeof(r));
    }
}

void Server::thread_send(int clie_fd, string senderID) //注意：此时sender与recver应交换
{
    Reader rd;
    FastWriter w;
    Value recv_from_db;
    Value deleteValue;

    string recverID = fd_ID[clie_fd];
    string send_to_db;

    char r[BUFSIZ];

    while (true)
    {
        leveldb::Iterator *it = Mdb->NewIterator(leveldb::ReadOptions());

        for (it->SeekToFirst(); it->Valid(); it->Next())
        {

            if (recverID == it->key().ToString())
            {
                rd.parse(it->value().ToString(), recv_from_db);
                break;
            }
        }

        for (int i = 0; i < recv_from_db.size(); i++)
        {
            Value number = recv_from_db[i];

            if (number["sender"].asString() == senderID)
            {
                string send = w.write(number);
                Write(clie_fd, send.c_str(), send.length());

                recv_from_db.removeIndex(i, &deleteValue);
                send_to_db = w.write(recv_from_db);

                pthread_mutex_lock(&mutex); //加锁
                Mdb->Put(leveldb::WriteOptions(), recverID, send_to_db);
                pthread_mutex_unlock(&mutex);

                i--;

                string outpass;
                leveldb::Status sta = Mdb->Get(leveldb::ReadOptions(), recverID, &outpass);
                check_status(sta);
                cout << "[数据库] " << recverID << " : " << outpass << endl;
            }
            sleep(0.05);
        }
        sleep(0.05);
    }
}
/*

     执行读写(EPOLL ET非阻塞,轮询)

 */
void Server::thread_work(int clie_fd)
{
    char r[BUFSIZ];
    //登录
    while (fd_in[clie_fd] == false && read(clie_fd, r, sizeof(r)) > 0)
    {

        if (strcmp(r, EXIT) == 0)
        {
            cout << "----" << clie_fd << "已退出----" << endl;
            Close(clie_fd);

            fd_in[clie_fd] == false;

            break;
        }
        else if (strcmp(r, SIGN_UP) == 0)
        {
            Server::sign_up(clie_fd);
        }
        else if (strcmp(r, SIGN_IN) == 0)
        {
            fd_in[clie_fd] = Server::sign_in(clie_fd);
            break;
        }
        bzero(r, sizeof(r));
    }

    bzero(r, sizeof(r));
    //主页面
    while (fd_in[clie_fd] == true && read(clie_fd, r, sizeof(r)) > 0)
    {
        if (strcmp(r, PRIVATE) == 0)
        {
            match_with(clie_fd);
        }
        else if (strcmp(r, PUBLIC) == 0)
        {
        }
        else if (strcmp(r, SIGN_OUT) == 0)
        {
            fd_in[clie_fd] = false;
            fd_ID.erase(fd_ID.begin() + clie_fd - 1); //删除第clie_fd的元素
            break;
        }
        bzero(r, sizeof(r));
    }
    return;
}

void Server::run()
{
    //互斥锁
    pthread_mutex_init(&mutex, NULL);
    //数据库
    IPopt.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(IPopt, "/tmp/serverdata/infoDB", &IPdb);
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }

    Mopt.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(Mopt, "/tmp/serverdata/massage", &Mdb);
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }

    //正向遍历        (测试用)
    leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());

    cout << "开始正向遍历:" << endl;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "key: " << it->key().ToString() << " value: " << it->value().ToString() << endl;
    }
    cout << "-------------------" << endl;

    //网络连接

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