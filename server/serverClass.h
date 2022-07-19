#ifndef _SERVER_CLASS_H_
#define _SERVER_CLASS_H_

#include <iostream>
#include <vector>

#define SIGN_IN "1"
#define SIGN_UP "2"
#define EXIT "0"

using namespace std;

class Server
{
private:
    int server_port;            //服务器端口号
    int link_fd;                //与客户端建立连接的套接字描述符
    string server_ip;           //服务器ip
    static vector<bool> fd_arr; //保存所有套接字描述符
public:
    Server(int port, string ip);
    ~Server();

    void run();
    static bool sign_up(int clie_fd);
    static void thread_work(int clie_fd);
};
#endif