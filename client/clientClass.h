#ifndef _CLIENT_CLASS_H_
#define _CLIENT_CLASS_H_

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
    void run();                               //启动客户端服务
    void sign_in_up(int clie_fd);             //登录注册界面
    void main_menu(int clie_fd, string ID);   //主菜单
    void group_menu(int clie_fd, string ID);  //群聊菜单
    void manage_menu(int clie_fd, string ID); //管理群

    void privateChat(int clie_fd, string ID);  //私聊
    void friends_menu(int clie_fd, string ID); //好友管理
    void create_group(int clie_fd, string ID); //创建群聊
    void add_group(int clie_fd);               //申请入群
    void quit_group(int clie_fd);              //退出群
    void view_group(int clie_fd);              //查看群
    void man_addgroup(int clie_fd);            //入群请求
    void man_view(int clie_fd);                //查看群成员
    void man_addmanager(int clie_fd);          //添加管理员
    void man_delmanager(int clie_fd);          //取消管理员
    void man_delmember(int clie_fd);           //踢出成员
    void man_delgroup(int clie_fd);            //解散群

    static void thread_send(int clie_fd);                //发送线程
    static void thread_recv(int clie_fd);                //接收线程
    static void thread_friends(int clie_fd, string opt); //好友管理线程
};

#endif
