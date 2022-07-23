#ifndef _SERVER_CLASS_H_
#define _SERVER_CLASS_H_

#include <iostream>
#include <vector>
#include "leveldb/db.h"

#define SIGN_IN "1"
#define SIGN_UP "2"
#define EXIT "0"

#define PRIVATE "1"
#define PUBLIC "2"
#define FRIENDS_MENU "3"
#define SIGN_OUT "0"

#define ADD_FRIEND "1"
#define DEL_FRIEND "2"
#define VIEW_FRIENDS "3"
#define MAS_FRIEND "4"
#define IGN_FRIEND "5"
#define BE_FRIENDS "0"

#define ACCEPT "_ACCEPT_"

#define ROOM_EXIT "_exit"

using namespace std;

class Server
{
private:
    int server_port;                //服务器端口号
    int link_fd;                    //与客户端建立连接的套接字描述符
    string server_ip;               //服务器ip
    static vector<bool> fd_arr;     //保存所有套接字描述符的连接状态
    static vector<bool> fd_pthread; //套接字描述符的线程
    static vector<string> fd_ID;    //保存套接字对应的ID
    static vector<bool> fd_in;      //保存套接字描述符登录状态

    static vector<pthread_mutex_t> fd_mutex; //锁
    static pthread_mutex_t mutex;

    static leveldb::Options opt; // opt
    static leveldb::DB *IPdb;    // ID与pass
    static leveldb::DB *Mdb;     // massage
    static leveldb::DB *Fdb;     // 好友请求

public:
    Server(int port, string ip);
    ~Server();

    void run();
    static bool sign_menu(int clie_fd);
    static void main_menu(int clie_fd);
    static void friends_menu(int clie_fd);

    static bool sign_in(int clie_fd);                    //登录判断
    static void sign_up(int clie_fd);                    //注册判断
    static void match_with(int clie_fd);                 //连接判断
    static void add_friend(int clie_fd, char r[BUFSIZ]); //加好友
    static void mas_friend(int clie_fd);                 //管理好友请求
    static void cout_friend(int clie_fd, string opt);    //输出

    static void thread_work(int clie_fd);                  //服务器线程工作
    static void thread_recv(int clie_fd, string recverID); //接收消息至数据库
    static void thread_send(int clie_fd, string recverID); //发送数据库消息

    static void check_status(leveldb::Status status);
};

#endif