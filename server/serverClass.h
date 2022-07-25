#ifndef _SERVER_CLASS_H_
#define _SERVER_CLASS_H_

#include <iostream>
#include <vector>
#include "leveldb/db.h"

#define SIGN_IN "1" //登入
#define SIGN_UP "2" //注册
#define EXIT "0"    //退出

#define PRIVATE "1"      //私聊
#define GROUP "2"        //群菜单
#define FRIENDS_MENU "3" //好友管理
#define SIGN_OUT "0"     //登出

#define ADD_FRIEND "1"   //加好友
#define DEL_FRIEND "2"   //删好友
#define VIEW_FRIENDS "3" //查看好友
#define MAS_FRIEND "4"   //好友请求
#define IGN_FRIEND "5"   //屏蔽好友
#define BE_FRIENDS "0"   //成为好友

#define JOIN_GROUP "1"   //群聊
#define ADD_GROUP "2"    //加入群聊
#define QUIT_GROUP "3"   //退出群聊
#define CREATE_GROUP "4" //创建群聊
#define VIEW_GROUP "5"   //查看群
#define MAN_GROUP "6"    //管理群

#define MAN_ADDGROUP "1"    //加群申请
#define MAN_VIEW "2"        //查看群信息
#define MAN_ADDMANAGER "3"  //添加管理
#define MAN_QUITMANAGER "4" //取消管理
#define MAN_QUITMEMBER "5"  //踢出成员
#define MAN_DELGROUP "6"    //解散该群

#define ACCEPT "_ACCEPT_" //收到

#define ROOM_EXIT "_exit" //退出

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
    static leveldb::DB *Gdb;     //群
    static leveldb::DB *GMdb;    //群消息

public:
    Server(int port, string ip);
    ~Server();

    void run();
    static bool sign_menu(int clie_fd);
    static void main_menu(int clie_fd);
    static void friends_menu(int clie_fd);
    static void group_menu(int clie_fd);

    static bool sign_in(int clie_fd);                    //登录判断
    static void sign_up(int clie_fd);                    //注册判断
    static void match_with(int clie_fd);                 //连接判断
    static void add_friend(int clie_fd, char r[BUFSIZ]); //加好友
    static void mas_friend(int clie_fd);                 //管理好友请求
    static void cout_friend(int clie_fd, string opt);    //输出
    static void create_group(int clie_fd);               //创建群聊
    static void add_group(int clie_fd);                  //加入群聊
    static void quit_group(int clie_fd);                 //退出群
    static void view_group(int clie_fd);                 //查看群
    static void manage_menu(int clie_fd);                //管理群

    static void thread_work(int clie_fd);                  //服务器线程工作
    static void thread_recv(int clie_fd, string recverID); //接收消息至数据库
    static void thread_send(int clie_fd, string recverID); //发送数据库消息

    static void check_status(leveldb::Status status);
};

#endif