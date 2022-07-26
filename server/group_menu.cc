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
    string opt = "NULL";

    Value getv;
    string get;
    string send;
    string member;
    FastWriter w;
    Reader rd;

    string ID = fd_ID[clie_fd];

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }
    cout << "R " << r << endl;

    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), r, &get);
    rd.parse(get, getv);
    cout << "============1 " << ID << endl;
    for (int i = 0; i < (int)getv["member"].size(); i++)
    {
        cout << "============2" << endl;
        member = getv["member"][i].asString();

        if (ID == member)
        {
            cout << "============3" << endl;
            opt = "member";
            Net::Write(clie_fd, opt.c_str(), opt.length());
            break;
        }
    }
    if (opt != "member")
    {
        cout << "============4" << endl;
        for (int i = 0; i < (int)getv["application"].size(); i++)
        {
            cout << "============5" << endl;
            member = getv["application"][i].asString();

            if (ID == member)
            {
                cout << "============6" << endl;
                opt = "application";
                Net::Write(clie_fd, opt.c_str(), opt.length());
                break;
            }
        }
    }

    if (s.ok() && opt == "NULL")
    {
        Net::Write(clie_fd, "success", 7);
        rd.parse(get, getv);
        getv["application"].append(fd_ID[clie_fd]);
        send = w.write(getv);
        leveldb::Status s2 = Gdb->Put(leveldb::WriteOptions(), r, send);
    }
    else if (opt != "member" && opt != "application")
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

        getgID = it->key().ToString();
        leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), getgID, &get);
        rd.parse(get, getv);
        member = getv["member"];

        for (int i = 0; i < (int)member.size(); i++)
        {

            if (member[i] == fd_ID[clie_fd])
            {

                Net::Write(clie_fd, getgID.c_str(), getgID.length());
                in_group = true;

                while (true)
                {
                    bzero(r, sizeof(r));
                    if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (!in_group)
    {

        Net::Write(clie_fd, "NULL", 4);
    }
    else
    {

        Net::Write(clie_fd, "END", 3);
    }

    cout << "退出查看群" << endl;
    return;
}

void Server::man_addgroup(int clie_fd, string gID)
{
    FastWriter w;
    string gets;
    string send;
    string member;
    Value getv;
    Value deletev;
    Reader rd;

    bool id_in = false;
    bool have_id = true;

    char r[BUFSIZ];
    char r2[BUFSIZ];
    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), gID, &gets);
    rd.parse(gets, getv);

    for (int i = 1; i < (int)getv["application"].size(); i++)
    {
        send = getv["application"][i].asString();
        Net::Write(clie_fd, send.c_str(), send.length());
        while (true)
        {
            bzero(r, sizeof(r));
            if (read(clie_fd, r, sizeof(r)) > 0)
            {
                break;
            }
        }
    }

    if (getv["application"].size() <= 1)
    {
        have_id = false;
        Net::Write(clie_fd, "NULL", 4);
    }
    else
    {
        Net::Write(clie_fd, "END", 3);
    }

    while (have_id)
    {
        //接收成员ID
        while (true)
        {
            bzero(r, sizeof(r));
            if (read(clie_fd, r, sizeof(r)) > 0)
            {
                cout << "R " << r << endl;
                break;
            }
        }
        cout << "size: " << (int)getv["application"].size() << endl;
        for (int i = 1; i < (int)getv["application"].size(); i++)
        {
            member = getv["application"][i].asString();
            cout << "member: " << member << endl;
            if (strcmp(r, member.c_str()) == 0)
            {
                id_in = true;
                break;
            }
        }

        if (strcmp(r, "q") == 0)
        {
            Net::Write(clie_fd, "exit", 4);
            break;
        }
        else if (!id_in)
        {
            Net::Write(clie_fd, "NULL", 4);
        }
        else
        {
            Net::Write(clie_fd, "success", 7);
        }

        //接收操作
        if (id_in)
        {
            while (true)
            {
                bzero(r2, sizeof(r2));
                if (read(clie_fd, r2, sizeof(r2)) > 0)
                {
                    break;
                }
            }

            if (strcmp(r2, "y") == 0)
            {
                Net::Write(clie_fd, "agree", 5);
                for (int i = 0; i < (int)getv["application"].size(); i++)
                {
                    member = getv["application"][i].asString();
                    if (strcmp(r, member.c_str()) == 0)
                    {
                        getv["application"].removeIndex(i, &deletev);
                        getv["member"].append(r);

                        send = w.write(getv);
                        cout << "sendy:" << send << endl;
                        leveldb::Status s2 = Gdb->Put(leveldb::WriteOptions(), gID, send);

                        break;
                    }
                }
            }
            else if (strcmp(r2, "n") == 0)
            {
                Net::Write(clie_fd, "refuse", 6);
                for (int i = 0; i < (int)getv["application"].size(); i++)
                {
                    member = getv["application"][i].asString();
                    if (strcmp(r, member.c_str()) == 0)
                    {
                        getv["application"].removeIndex(i, &deletev);

                        send = w.write(getv);
                        cout << "sendn:" << send << endl;
                        leveldb::Status s2 = Gdb->Put(leveldb::WriteOptions(), gID, send);

                        break;
                    }
                }
            }
            else if (strcmp(r2, "q") == 0)
            {
                Net::Write(clie_fd, "quit", 4);
            }
            else
            {
                Net::Write(clie_fd, "fail", 4);
            }
        }
    }

    return;
}

void Server::manage_menu(int clie_fd)
{
    cout << "进入判断职位" << endl;
    Value getv;

    string gets;
    Reader rd;

    string judge = "fail";
    string ID;

    char r[BUFSIZ];
    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R: " << r << endl;
            break;
        }
    }
    ID = r;

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
    cout << "结束判断职位" << judge << endl;

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
                man_addgroup(clie_fd, ID);
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
            manage_menu(clie_fd);
        }
        else if (strcmp(r, EXIT) == 0)
        {
            break;
        }
    }

    cout << "退出群菜单" << endl;
    return;
}