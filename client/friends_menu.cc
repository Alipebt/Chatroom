#include "clientClass.h"
#include "net_wrap.h"

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