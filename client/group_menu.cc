#include "clientClass.h"
#include "net_wrap.h"

void Client::create_group(int clie_fd, string ID)
{
    string in, s;
    Value group;
    FastWriter w;
    char r[BUFSIZ];

    group["master"] = ID;
    group["member"].append(ID);
    group["application"].append("");
    cout << " 请输入群ID\n>";
    cin >> in;
    group["ID"] = in;
    cout << " 请输入群名\n>";
    cin >> in;
    group["name"] = in;

    s = w.write(group);
    Net::Write(clie_fd, s.c_str(), s.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }

    cout << "R " << r << endl;

    if (strcmp(r, "success") == 0)
    {
        cout << "创建成功" << endl;
    }
    else if (strcmp(r, "fail") == 0)
    {
        cout << "该群ID已被占用" << endl;
    }
    else
    {
        cout << "错误" << endl;
    }

    return;
}

void Client::add_group(int clie_fd)
{
    char r[BUFSIZ];

    string in;
    cout << " 请输入想要加入的群聊\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R: " << r << endl;
            break;
        }
    }

    if (strcmp(r, "success") == 0)
    {
        cout << "申请成功" << endl;
    }
    else if (strcmp(r, "fail") == 0)
    {
        cout << "该群不存在" << endl;
    }
    else if (strcmp(r, "member") == 0)
    {
        cout << "你已是该群成员" << endl;
    }
    else if (strcmp(r, "application") == 0)
    {
        cout << "不可重复发送申请" << endl;
    }

    return;
}

void Client::quit_group(int clie_fd)
{
    string in;
    char r[BUFSIZ];

    bzero(r, sizeof(r));
    cout << " 请输入要退出的群\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());

    while (true)
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }

    if (strcmp(r, "success") == 0)
    {
        cout << "你已退出群" << in << endl;
    }
    else if (strcmp(r, "fail") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(r, "refuse") == 0)
    {
        cout << "群主不可退群,可选择解散该群" << endl;
    }
}

void Client::view_group(int clie_fd)
{
    char r[BUFSIZ];

    cout << "你加入的群:" << endl;
    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            if (strcmp(r, "NULL") == 0)
            {
                cout << "未加入任何群" << endl;
                break;
            }
            else if (strcmp(r, "END") == 0)
            {
                break;
            }
            else
            {
                cout << r << endl;
                Net::Write(clie_fd, ACCEPT, sizeof(ACCEPT));
            }
        }
        }
}

void Client::man_addgroup(int clie_fd)
{
    char r[BUFSIZ];
    string in;
    bool id_in = true;
    bool no_id = true;

    cout << "申请列表：" << endl;
    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            if (strcmp(r, "NULL") == 0)
            {
                cout << "无申请" << endl;
                break;
            }
            else if (strcmp(r, "END") == 0)
            {
                break;
            }
            else
            {
                no_id = false;
                cout << r << endl;
                Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
            }
        }
    }

    while (!no_id)
    {
        cout << " 请输入你要操作的ID(取消q)\n> ";
        cin >> in;

        Net::Write(clie_fd, in.c_str(), in.length());

        while (true)
        {
            bzero(r, sizeof(r));
            if (read(clie_fd, r, sizeof(r)) > 0)
            {
                cout << "R " << r << endl;
                break;
            }
        }

        if (strcmp(r, "exit") == 0)
        {
            break;
        }
        else if (strcmp(r, "NULL") == 0)
        {
            id_in = false;
            cout << "无该成员" << endl;
        }
        else if (strcmp(r, "success") == 0)
        {
        }

        if (id_in)
        {

            cout << " ( y/n/q )" << endl;
            cin >> in;

            Net::Write(clie_fd, in.c_str(), in.length());

            while (true)
            {
                bzero(r, sizeof(r));
                if (read(clie_fd, r, sizeof(r)) > 0)
                {
                    break;
                }
            }

            if (strcmp(r, "agree") == 0)
            {
                cout << "已同意" << endl;
            }
            else if (strcmp(r, "refuse") == 0)
            {
                cout << "已拒绝" << endl;
            }
            else if (strcmp(r, "quit") == 0)
            {
                cout << "已取消操作" << endl;
            }
            else
            {
                cout << "无此操作" << endl;
            }
        }
    }

    return;
}

void Client::manage_menu(int clie_fd, string ID)
{
    string in;
    char r[BUFSIZ];

    cout << " 输入需要管理的群\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R: " << r << endl;
            break;
        }
    }

    if (strcmp(r, "master") == 0)
    {
        cout << "群：" << in << "\t" << r << endl;
        while (true)
        {
            cout << "+------------------+" << endl;
            cout << "|     ChatRoom     |" << endl;
            cout << "+------------------+" << endl;
            cout << "|                  |" << endl;
            cout << "|    1:入群请求   |" << endl;
            cout << "|    2:查看成员   |" << endl;
            cout << "|    3:添加管理   |" << endl;
            cout << "|    4:取消管理   |" << endl;
            cout << "|    5:踢出成员   |" << endl;
            cout << "|    6:解散该群   |" << endl;
            cout << "|    0:退出界面   |" << endl;
            cout << "|                  |" << endl;
            cout << "+------------------+" << endl;

            cin >> in;
            Net::Write(clie_fd, in.c_str(), in.length());

            if (in == MAN_ADDGROUP)
            {
                man_addgroup(clie_fd);
            }
            else if (in == MAN_VIEW)
            {
            }
            else if (in == MAN_ADDMANAGER)
            {
            }
            else if (in == MAN_QUITMANAGER)
            {
            }
            else if (in == MAN_QUITMEMBER)
            {
            }
            else if (in == MAN_DELGROUP)
            {
            }
            else if (in == EXIT)
            {
                break;
            }
        }
    }
    else if (strcmp(r, "manager") == 0)
    {
        cout << "群：" << in << "\t" << r << endl;
    }
    else if (strcmp(r, "member") == 0)
    {
        cout << "群：" << in << "\t" << r << endl;
    }
    else
    {
        cout << "你不是该群成员" << endl;
    }

    return;
}

void Client::group_menu(int clie_fd, string ID)
{
    string in;

    while (true)
    {
        cout << "+------------------+" << endl;
        cout << "|     ChatRoom     |" << endl;
        cout << "+------------------+" << endl;
        cout << "|                  |" << endl;
        cout << "| 1:进入群聊       |" << endl;
        cout << "| 2:申请入群       |" << endl;
        cout << "| 3:申请退群       |" << endl;
        cout << "| 4:创建群聊       |" << endl;
        cout << "| 5:查看已加入的群 |" << endl;
        cout << "| 6:管理群聊       |" << endl;
        cout << "| 0:退出页面       |" << endl;
        cout << "|                  |" << endl;
        cout << "+------------------+" << endl;

        cin >> in;
        Net::Write(clie_fd, in.c_str(), in.length());

        if (in == JOIN_GROUP)
        {
        }
        else if (in == ADD_GROUP)
        {
            add_group(clie_fd);
        }
        else if (in == QUIT_GROUP)
        {
            quit_group(clie_fd);
        }
        else if (in == CREATE_GROUP)
        {
            create_group(clie_fd, ID);
        }
        else if (in == VIEW_GROUP)
        {
            view_group(clie_fd);
        }
        else if (in == MAN_GROUP)
        {
            manage_menu(clie_fd, ID);
        }
        else if (in == EXIT)
        {
            break;
        }
    }
}