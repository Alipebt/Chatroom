#include "net_wrap.h"
#include "clientClass.h"

/******************************************************************
 *
                            CLIENT

******************************************************************/

//构造函数传入接口和ip
Client::Client(int port, string ip) : server_port(port), server_ip(ip) {}

//客户端关闭前关闭其与服务器连接的套接字
Client::~Client()
{
    Close(clie_fd);
}

void Client::sign_in_up(int clie_fd)
{
    string in;
    string s;
    char r[BUFSIZ];
    string ID, pass1, pass2;

    while (true)
    {
        cout << "+------------------+" << endl;
        cout << "|     ChatRoom     |" << endl;
        cout << "+------------------+" << endl;
        cout << "|                  |" << endl;
        cout << "|      1:登录      |" << endl;
        cout << "|      2:注册      |" << endl;
        cout << "|      0:退出      |" << endl;
        cout << "|                  |" << endl;
        cout << "+------------------+" << endl;

        cin >> in;

        Write(clie_fd, in.c_str(), in.length());

        if (in == SIGN_IN)
        {
            Value info;

            cout << " 请输入你的ID\n>";
            cin >> ID;
            if (ID.length() > 20 || ID.length() < 3)
            {
                cout << "\nID长度应在3~20" << endl;
                Write(clie_fd, "fail", 4); //结束服务器调用sign_in函数
                continue;
            }
            pass1 = getpass(" 请输入你的密码\n>");
            if (pass1.length() > 20 || pass1.length() < 3)
            {
                cout << "\n密码长度应在3~20" << endl;
                Write(clie_fd, "fail", 4); //结束服务器调用sign_in函数
                continue;
            }

            info["ID"] = ID;
            info["pass"] = pass1;
            FastWriter w;
            s = w.write(info);
        }
        else if (in == SIGN_UP)
        {
            cout << " 请输入你的ID\n>";
            cin >> ID;
            if (ID.length() > 20 || ID.length() < 3)
            {
                cout << "\nID长度应在3~20" << endl;
                Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                continue;
            }
            pass1 = getpass(" 请输入你的密码\n>");
            if (pass1.length() > 20 || pass1.length() < 3)
            {
                cout << "\n密码长度应在3~20" << endl;
                Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                continue;
            }
            pass2 = getpass(" 请再次输入你的密码\n>");

            if (pass1 == pass2)
            {
                Value info;
                info["ID"] = ID;
                info["pass"] = pass1;

                //有格式序列化
                FastWriter w;
                s = w.write(info);

                Write(clie_fd, s.c_str(), s.length());

                while (true)
                {
                    if ((read(clie_fd, r, sizeof(r))) > 0)
                    {
                        cout << " 收到服务器消息\n>" << r << endl;
                        if (strcmp(r, "fail") == 0)
                        {
                            cout << ">ID已被占用" << endl;
                        }
                        else if (strcmp(r, "success") == 0)
                        {
                            cout << ">注册成功" << endl;
                        }
                        break;
                    }
                }
            }
            else
            {
                Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                cout << "两次密码不一致" << endl;
            }
        }
        else if (in == EXIT)
        {
            cout << "程序已退出" << endl;
            exit(1);
        }
        bzero(r, sizeof(r));
    }
}

void Client::run()
{

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    clie_fd = Socket(AF_INET, SOCK_STREAM, 0);

    Connect(clie_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    sign_in_up(clie_fd);

    thread send_t(thread_send, clie_fd), recv_t(thread_recv, clie_fd);

    send_t.join();
    cout << "发送线程已关闭" << endl;
    recv_t.join();
    cout << "接收线程已关闭" << endl;

    return;
}

void Client::thread_send(int clie_fd)
{

    char s[BUFSIZ];
    while (true)
    {
        cin >> s;

        // write返回顺利写入字节，若==0或<0则可能对端关闭
        //不用Write以防客户端报错退出
        int ret = write(clie_fd, s, strlen(s));
        if (strcmp(s, EXIT) == 0 || ret <= 0)
        {
            break;
        }
        bzero(s, sizeof(s));
    }
    return;
}
void Client::thread_recv(int clie_fd)
{
    char r[BUFSIZ];
    while (true)
    {
        //同write
        int ret = read(clie_fd, r, sizeof(r));
        if (ret <= 0)
        {
            break;
        }
        bzero(r, sizeof(r));
        // cout << "收到服务器发来的信息：" << recv << endl;
    }
}
