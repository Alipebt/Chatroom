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

            cout << " 请输入你的用户名\n>";
            cin >> name;
            if (name.length() > 20 || name.length() < 3)
            {
                cout << "name长度应在3~20" << endl;
                Net::Write(clie_fd, "fail", 4); //结束服务器调用sign_up函数
                continue;
            }

            if (pass1 == pass2)
            {
                Value info;
                info["ID"] = ID;
                info["pass"] = pass1;
                info["name"] = name;

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
    string in, s;
    char r[BUFSIZ];
    Value match;
    FastWriter w;
    match["sender"] = ID;

    while (true)
    {
        bzero(r, sizeof(r));

        cout << "+------------------+" << endl;
        cout << "|     ChatRoom     |" << endl;
        cout << "+------------------+" << endl;
        cout << "|                  |" << endl;
        cout << "|    1:私聊        |" << endl;
        cout << "|    2:群聊        |" << endl;
        cout << "|    3:好友管理    |" << endl;
        cout << "|    0:注销        |" << endl;
        cout << "|                  |" << endl;
        cout << "+------------------+" << endl;

        cin >> in;

        Net::Write(clie_fd, in.c_str(), in.length());
        if (in == PRIVATE)
        {
            //给服务器发送请求

            cout << " 请输入对方ID\n>";
            cin >> in;
            match["recver"] = in;

            s = w.write(match);

            Net::Write(clie_fd, s.c_str(), s.length());
            while (true)
            {
                bzero(r, sizeof(r));
                if (read(clie_fd, r, sizeof(r)) > 0)
                {
                    if (strcmp(r, "success") == 0)
                    {
                        cout << "已与" << in << "连接:" << endl;
                        thread send(thread_send, clie_fd);
                        thread recv(thread_recv, clie_fd);

                        send.join();
                        recv.join();
                    }
                    else if (strcmp(r, "fail") == 0)
                    {
                        cout << "无用户" << in << endl;
                    }
                    else
                    {
                        cout << "连接失败" << endl;
                    }
                    break;
                }
            }
        }
        else if (in == PUBLIC)
        {
        }
        else if (in == FRIENDS_MENU)
        {
            friends_menu(clie_fd, ID);
        }
        else if (in == SIGN_OUT)
        {

            break;
        }
    }

    return;
}

void Client::friends_menu(int clie_fd, string ID)
{
    string in, s;
    Value friends;

    FastWriter w;
    char r[BUFSIZ];
    friends["sender"] = ID;

    while (true)
    {

        cout << "+------------------+" << endl;
        cout << "|     ChatRoom     |" << endl;
        cout << "+------------------+" << endl;
        cout << "|                  |" << endl;
        cout << "|    1:添加好友    |" << endl;
        cout << "|    2:删除好友    |" << endl;
        cout << "|    3:查看好友    |" << endl;
        cout << "|    4:好友请求    |" << endl;
        cout << "|    5:屏蔽好友    |" << endl;
        cout << "|    0:退出界面    |" << endl;
        cout << "|                  |" << endl;
        cout << "+------------------+" << endl;

        cin >> in;
        Net::Write(clie_fd, in.c_str(), in.length());

        if (in == ADD_FRIEND)
        {
            cout << " 请输入对方ID\n>";
            cin >> in;

            friends["recver"] = in;
            friends["opt"] = ADD_FRIEND;

            s = w.write(friends);

            Net::Write(clie_fd, s.c_str(), s.length());
            cout << "发送好友请求" << s << endl;
            while (true)
            {
                bzero(r, sizeof(r));
                if (read(clie_fd, r, sizeof(r)) > 0)
                {
                    if (strcmp(r, "success") == 0)
                    {
                        cout << "成功发送请求" << endl;
                    }
                    else if (strcmp(r, "befriends") == 0)
                    {
                        cout << "你们以及是好友了" << endl;
                    }
                    else if (strcmp(r, "fail") == 0)
                    {

                        cout << "无此用户" << endl;
                    }
                    break;
                }
            }
        }
        else if (in == VIEW_FRIENDS)
        {
            thread_friends(clie_fd, BE_FRIENDS);
        }
        else if (in == MAS_FRIEND)
        {
            thread_friends(clie_fd, MAS_FRIEND);

            while (true)
            {
                cout << "请输入所要操作的对象的ID (取消操作:q)" << endl;
                cin >> in;
                if (in == "q")
                {
                    Net::Write(clie_fd, in.c_str(), in.length());
                    break;
                }
                friends["recver"] = in;
                cout << " ( y/n/q )" << endl;
                cin >> in;

                if (in == "q")
                {
                    continue;
                }
                else if (in == "y")
                {
                    friends["opt"] = ADD_FRIEND;
                }
                else if (in == "n")
                {
                    friends["opt"] = DEL_FRIEND;
                }
                else
                {
                    cout << "无此操作" << endl;
                    continue;
                }

                s = w.write(friends);
                Net::Write(clie_fd, s.c_str(), s.length());

                if (read(clie_fd, r, sizeof(r)) > 0)
                {
                    if (strcmp(r, "befriends") == 0)
                    {
                        cout << "你与" << in << "已是好友" << endl;
                    }
                    else if (strcmp(r, "fail") == 0)
                    {
                        cout << "无此请求" << endl;
                    }
                }
            }
        }
        else if (in == EXIT)
        {
            cout << "退出" << endl;
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

void Client::thread_send(int clie_fd)
{
    cout << "客户端发送线程开启" << endl;
    char s[BUFSIZ];
    while (true)
    {
        cin >> s;
        // write返回顺利写入字节，若==0或<0则可能对端关闭
        //不用Write以防客户端报错退出
        int ret = write(clie_fd, s, strlen(s));

        if (strcmp(s, ROOM_EXIT) == 0 || ret <= 0)
        {
            break;
        }
        bzero(s, sizeof(s));
    }
    cout << "客户端发送线程关闭" << endl;
    return;
}
void Client::thread_recv(int clie_fd)
{
    cout << "客户端接收线程开启" << endl;
    Reader rd;
    Value recv;

    char r[BUFSIZ];
    while (true)
    {
        //同write
        int ret = read(clie_fd, r, sizeof(r));
        if (ret <= 0)
        {
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
            cout << "客户端接收异常" << endl;
            break;
        }

        rd.parse(r, recv);

        if (recv["massage"].asString() == ROOM_EXIT /*|| recv["massage"].asString() == EXIT*/)
        {
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
            cout << "客户端已收到关闭请求" << endl;
            break;
        }

        if (recv["massage"].asString() != ACCEPT)
        {
            cout << LIGHT_BLUE << "[" << recv["sender"].asString() << "]:" << recv["massage"].asString() << NONE << endl;
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
        }
        bzero(r, sizeof(r));
    }
    cout << "客户端接收线程关闭" << endl;
    return;
}

void Client::thread_friends(int clie_fd, string opt)
{
    // BE_FRIENDS  查看好友
    // MAS_FRIEND 管理好友请求

    char r[BUFSIZ];
    Value recv;
    Value number;
    Reader rd;

    if (opt == MAS_FRIEND)
    {
        cout << "好友申请" << endl;
        while (true)
        {
            if (read(clie_fd, r, sizeof(r)) > 0)
            {
                if (strcmp(r, "NULL") == 0)
                {
                    cout << "无好友请求" << endl;
                    Net::Write(clie_fd, ACCEPT, sizeof(ACCEPT));
                }
                else if (strcmp(r, "END") == 0)
                {
                    break;
                }
                else
                {
                    rd.parse(r, recv);
                    cout << "ID: " << recv["sender"].asString() << endl;
                    Net::Write(clie_fd, ACCEPT, sizeof(ACCEPT));
                }
                bzero(r, sizeof(r));
            }
        }
    }
    else if (opt == BE_FRIENDS)
    {
        cout << "好友:" << endl;
        while (true)
        {
            if (read(clie_fd, r, sizeof(r)) > 0)
            {
                if (strcmp(r, "END") == 0)
                {
                    break;
                }
                if (strcmp(r, "NULL") == 0)
                {
                    cout << "无好友" << endl;
                }
                else
                {
                    rd.parse(r, recv);
                    cout << "ID:" << recv["sender"].asString() << endl;
                }

                Net::Write(clie_fd, ACCEPT, sizeof(ACCEPT));
                bzero(r, sizeof(r));
            }
        }
    }

    return;
}