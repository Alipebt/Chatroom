#ifndef _CLIENT_CLASS_H_
#define _CLIENT_CLASS_H_

#define SIGN_IN "1"
#define SIGN_UP "2"
#define EXIT "0"

#define PRIVATE "1"
#define PUBLIC "2"
#define ADD_FRIEND "3"
#define SIGN_OUT "0"

#include <iostream>

using namespace std;

class Client
{

private:
    int server_port;  //服务器端口
    string server_ip; //服务器ip
    int clie_fd;      //与服务器建立连接的套接字描述符
public:
    Client(int port, string ip);
    ~Client();
    void run();                             //启动客户端服务
    void sign_in_up(int clie_fd);           //登录注册界面
    void main_menu(int clie_fd, string ID); //主菜单
    static void thread_send(int clie_fd);   //发送线程
    static void thread_recv(int clie_fd);   //接收线程
};

#endif
