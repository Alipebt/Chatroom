#ifndef _CLIENT_CLASS_H_
#define _CLIENT_CLASS_H_

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

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
#define SHINE "\033[5m"      //闪烁
#define DASH "\033[9m"       // 中间一道横线
#define QUICKSHINE "\033[6m" //快闪
#define FANXIAN "\033[7m"    //反显
#define XIAOYIN "\033[8m"    // 消隐，消失隐藏

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
    void run();                                //启动客户端服务
    void sign_in_up(int clie_fd);              //登录注册界面
    void main_menu(int clie_fd, string ID);    //主菜单
    void privateChat(int clie_fd, string ID);  //私聊
    void friends_menu(int clie_fd, string ID); //好友管理

    static void thread_send(int clie_fd);                //发送线程
    static void thread_recv(int clie_fd);                //接收线程
    static void thread_friends(int clie_fd, string opt); //好友管理线程
};

#endif
