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
    Net::Close(clie_fd);
}

void Client::sign_in_up(int clie_fd)
{
    string in;
    string s;
    char r[BUFSIZ];
    string ID, pass1, pass2, name;

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

        Net::Write(clie_fd, in.c_str(), in.length());
        cout << "发送" << in << endl;

        if (in == SIGN_IN)
        {
            Value info;

            cout << " 请输入你的ID\n>";
            cin >> ID;
            if (ID.length() > 20 || ID.length() < 3)
            {
                cout << "\nID长度应在3~20" << endl;
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_in函数
                continue;
            }
            pass1 = getpass(" 请输入你的密码\n>");
            if (pass1.length() > 20 || pass1.length() < 3)
            {
                cout << "\n密码长度应在3~20" << endl;
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_in函数
                continue;
            }

            info["ID"] = ID;
            info["pass"] = pass1;
            FastWriter w;
            s = w.write(info);

            Net::Write(clie_fd, s.c_str(), s.length());
            cout << "发送" << s << endl;
            while (true)
            {

                if ((read(clie_fd, r, sizeof(r))) > 0)
                {
                    cout << " 收到服务器消息\n>" << r << endl;

                    if (strcmp(r, "success") == 0)
                    {
                        cout << " [登录成功]" << endl;
                        main_menu(clie_fd, ID);
                    }
                    else if (strcmp(r, "fail") == 0)
                    {
                        cout << " [ID或密码错误]" << endl;
                    }
                    else
                    {
                        cout << " [登录失败]" << endl;
                    }
                    break;
                }
            }
        }
        else if (in == SIGN_UP)
        {
            cout << " 请输入你的ID\n>";
            cin >> ID;
            if (ID.length() > 20 || ID.length() < 3)
            {
                cout << "\nID长度应在3~20" << endl;
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                continue;
            }
            pass1 = getpass(" 请输入你的密码\n>");
            if (pass1.length() > 20 || pass1.length() < 3)
            {
                cout << "\n密码长度应在3~20" << endl;
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                continue;
            }
            pass2 = getpass(" 请再次输入你的密码\n>");

            if (pass1 == pass2)
            {
                Value info;
                info["ID"] = ID;
                info["pass"] = pass1;
                // info["name"] = name;

                //有格式序列化
                FastWriter w;
                s = w.write(info);

                Net::Write(clie_fd, s.c_str(), s.length());
                cout << s << endl;

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
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
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

void Client::main_menu(int clie_fd, string ID)
{
    string in;
    char r[BUFSIZ];

    while (true)
    {
        bzero(r, sizeof(r));

        cout << "+------------------+" << endl;
        cout << "|     ChatRoom     |" << endl;
        cout << "+------------------+" << endl;
        cout << "|                  |" << endl;
        cout << "|    1:私聊        |" << endl;
        cout << "|    2:群菜单      |" << endl;
        cout << "|    3:好友管理    |" << endl;
        cout << "|    4.发送文件    |" << endl;
        cout << "|    5.接收文件    |" << endl;
        cout << "|    0:注销        |" << endl;
        cout << "|                  |" << endl;
        cout << "+------------------+" << endl;

        cin >> in;

        Net::Write(clie_fd, in.c_str(), in.length());
        if (in == PRIVATE)
        {
            privateChat(clie_fd, ID);
        }
        else if (in == GROUP)
        {
            group_menu(clie_fd, ID);
        }
        else if (in == FRIENDS_MENU)
        {
            friends_menu(clie_fd, ID);
        }
        else if (in == "4")
        {
            // send_file(clie_fd);
            file_menu(clie_fd, "send");
        }
        else if (in == "5")
        {
            // recv_file(clie_fd);
            file_menu(clie_fd, "recv");
        }
        else if (in == SIGN_OUT)
        {

            break;
        }
    }

    return;
}

void Client::run()
{

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    clie_fd = Net::Socket(AF_INET, SOCK_STREAM, 0);

    Net::Connect(clie_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    sign_in_up(clie_fd);

    return;
}
