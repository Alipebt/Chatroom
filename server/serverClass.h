#ifndef _SERVER_CLASS_H_
#define _SERVER_CLASS_H_

#include <iostream>
#include <vector>

#define SIGN_IN "1"
#define SIGN_UP "2"
#define EXIT "0"

#define PRIVATE "1"
#define PUBLIC "2"
#define SIGN_OUT "0"

using namespace std;

class Server
{
private:
    int server_port;             //服务器端口号
    int link_fd;                 //与客户端建立连接的套接字描述符
    string server_ip;            //服务器ip
    static vector<bool> fd_arr;  //保存所有套接字描述符
    static vector<string> fd_ID; //保存套接字对应的ID
    static vector<bool> fd_in;   //保存套接字描述符登录状态

public:
    Server(int port, string ip);
    ~Server();

    void run();

    static bool sign_in(int clie_fd);    //登录判断
    static void sign_up(int clie_fd);    //注册判断
    static void match_with(int clie_fd); //连接判断

    static void thread_work(int clie_fd);                  //服务器线程工作
    static void thread_recv(int clie_fd, string recverID); //接收消息至数据库
    static void thread_send(int clie_fd, string recverID); //发送数据库消息
};
#endif