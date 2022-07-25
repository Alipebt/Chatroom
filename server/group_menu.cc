#include "net_wrap.h"
#include "serverClass.h"

void Server::create_group(int clie_fd)
{
    cout << "开始创建群" << endl;
    Value newgroup;
    Value group;
    Value put;

    string ngID;
    string get;
    Reader rd;

    bool id_used = false;

    char r[BUFSIZ];
    while (true)
    {
        bzero(r, sizeof(r));
        if ((read(clie_fd, r, sizeof(r))) > 0)
        {
            break;
        }
    }
    rd.parse(r, newgroup);

    ngID = newgroup["ID"].asString();

    leveldb::Status status1 = Gdb->Get(leveldb::ReadOptions(), ngID, &get);
    if (status1.ok())
    {
        Net::Write(clie_fd, "fail", 4);
        id_used = true;
    }

    if (!id_used)
    {
        Net::Write(clie_fd, "success", 7);
        leveldb::Status status2 = Gdb->Put(leveldb::WriteOptions(), ngID, r);
        check_status(status2);
    }

    cout << "结束创建群" << endl;
    return;
}

void Server::add_group(int clie_fd)
{
    cout << "开始加群" << endl;
    char r[BUFSIZ];

    Value getv;
    string get;
    Reader rd;

    while (true)
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }

    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), r, &get);
    if (s.ok())
    {
        Net::Write(clie_fd, "success", 7);
        rd.parse(get, getv);
        getv["application"].append(fd_ID[clie_fd]);
    }
    else
    {
        Net::Write(clie_fd, "fail", 4);
    }
    cout << "结束加群" << endl;
    return;
}

void Server::quit_group(int clie_fd)
{
    cout << "进入退出群" << endl;
    char r[BUFSIZ];

    Value member;
    Value getv;
    Reader rd;
    FastWriter w;
    Value deletes;

    string get;
    string send;

    bool success = false;

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R" << r << endl;
            break;
        }
    }

    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), r, &get);
    if (s.ok())
    {

        rd.parse(get, getv);
        member = getv["member"];
        for (int i = 0; i < (int)getv.size(); i++)
        {

            if (member[i] == fd_ID[clie_fd])
            {
                if (getv["master"] != fd_ID[clie_fd])
                {
                    Net::Write(clie_fd, "success", 7);
                    getv["member"].removeIndex(i, &deletes);

                    send = w.write(getv);
                    leveldb::Status s1 = Gdb->Put(leveldb::WriteOptions(), r, send);

                    success = true;
                }
                else
                {
                    Net::Write(clie_fd, "refuse", 6);
                }
            }
        }

        if (!success)
        {
            Net::Write(clie_fd, "fail", 4);
        }
    }
    else
    {
        Net::Write(clie_fd, "fail", 4);
    }

    cout << "结束退出群" << endl;
    return;
}

void Server::view_group(int clie_fd)
{
    cout << "进入查看群" << endl;
    Value getv;
    Value member;
    Reader rd;

    string ID;
    string getgID;
    string get;

    bool in_group = false;

    ID = fd_ID[clie_fd];
    char r[BUFSIZ];
    //遍历ID
    leveldb::Iterator *it = Gdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "==========1" << endl;
        getgID = it->key().ToString();
        leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), getgID, &get);
        rd.parse(get, getv);
        member = getv["member"];

        for (int i = 0; i < (int)member.size(); i++)
        {
            cout << "==========5" << endl;
            if (member[i] == fd_ID[clie_fd])
            {
                cout << "==========2" << endl;
                Net::Write(clie_fd, getgID.c_str(), getgID.length());
                in_group = true;
                break;

                while (true)
                {
                    if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
                    {
                        break;
                    }
                }
            }
        }
        cout << "==========6" << endl;
    }

    if (!in_group)
    {
        cout << "==========3" << endl;
        Net::Write(clie_fd, "NULL", 4);
    }
    else
    {
        cout << "==========4" << endl;
        Net::Write(clie_fd, "END", 3);
    }

    cout << "退出查看群" << endl;
    return;
}

void Server::manage_menu(int clie_fd)
{
    cout << "进入判断职位" << endl;
    Value getv;

    string gets;
    Reader rd;

    string judge = "fail";

    char r[BUFSIZ];
    while (true)
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }

    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), r, &gets);
    if (s.ok())
    {
        rd.parse(gets, getv);
        if (getv["master"] == fd_ID[clie_fd])
        {
            judge = "master";
            Net::Write(clie_fd, judge.c_str(), judge.length());
        }
        else
        {
            for (int i = 0; i < (int)getv["manager"].size(); i++)
            {
                if (getv["manager"][i] == fd_ID[clie_fd])
                {
                    judge = "manager";
                    Net::Write(clie_fd, judge.c_str(), judge.length());
                    break;
                }
            }

            if (judge != "manager")
            {
                for (int i = 0; i < (int)getv["member"].size(); i++)
                {
                    if (getv["member"][i] == fd_ID[clie_fd])
                    {
                        judge = "member";
                        Net::Write(clie_fd, judge.c_str(), judge.length());
                        break;
                    }
                }
            }
        }

        if (judge == "fail")
        {
            Net::Write(clie_fd, judge.c_str(), judge.length());
        }
    }
    else
    {
        Net::Write(clie_fd, judge.c_str(), judge.length());
    }
    cout << "结束判断职位" << endl;

    //进入管理
    cout << "进入管理" << endl;

    if (judge == "master")
    {
        while (true)
        {
            while (true)
            {
                bzero(r, sizeof(r));
                if (read(clie_fd, r, sizeof(r)) > 0)
                {
                    break;
                }
            }

            if (strcmp(r, MAN_ADDGROUP) == 0)
            {
            }
            else if (strcmp(r, MAN_VIEW) == 0)
            {
            }
            else if (strcmp(r, MAN_ADDMANAGER) == 0)
            {
            }
            else if (strcmp(r, MAN_QUITMANAGER) == 0)
            {
            }
            else if (strcmp(r, MAN_QUITMEMBER) == 0)
            {
            }
            else if (strcmp(r, MAN_DELGROUP) == 0)
            {
            }
            else if (strcmp(r, EXIT) == 0)
            {
                break;
            }
        }
    }
    else if (judge == "manager")
    {
    }
    else if (judge == "member")
    {
    }

    cout << "退出管理" << endl;
    return;
}

void Server::group_menu(int clie_fd)
{
    cout << "进入群菜单" << endl;
    char r[BUFSIZ];
    while (true)
    {
        while (true)
        {
            bzero(r, sizeof(r));
            if ((read(clie_fd, r, sizeof(r))) > 0)
            {
                break;
            }
        }

        if (strcmp(r, JOIN_GROUP) == 0)
        {
        }
        else if (strcmp(r, ADD_GROUP) == 0)
        {
            add_group(clie_fd);
        }
        else if (strcmp(r, QUIT_GROUP) == 0)
        {
            quit_group(clie_fd);
        }
        else if (strcmp(r, CREATE_GROUP) == 0)
        {
            create_group(clie_fd);
        }
        else if (strcmp(r, VIEW_GROUP) == 0)
        {
            view_group(clie_fd);
        }
        else if (strcmp(r, MAN_GROUP) == 0)
        {
        }
        else if (strcmp(r, EXIT) == 0)
        {
            break;
        }
    }

    cout << "退出群菜单" << endl;
    return;
}