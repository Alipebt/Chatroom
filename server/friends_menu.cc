#include "net_wrap.h"
#include "serverClass.h"

void Server::add_friend(int clie_fd, char *re)
{
    cout << "进入加好友界面" << endl;
    Value friends;
    Value recv_from_db;
    Value recv_from_db2;
    Value allv;
    string recv_s;

    Value member;
    Value member2;
    Value member3;
    Value deleteValue;

    FastWriter w;
    Reader rd;

    string s, sr;

    string recver;
    string sender;

    string res = re;
    char r[BUFSIZ];

    strcpy(r, res.c_str());

    bool id_used = false;
    bool mas = false;

    if (strcmp(r, "NULL") == 0)
    {
        while (true)
        {
            if ((read(clie_fd, r, sizeof(r))) > 0)
            {
                break;
            }
        }
    }
    else
    {
        mas = true;
    }

    if (rd.parse(r, friends))
    {

        recver = friends["recver"].asString();
        sender = friends["sender"].asString();
        if (friends["opt"].asString() != "NULL")
        {

            //遍历ID
            leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                if (recver == it->key().ToString())
                {
                    id_used = true;
                    break;
                }
            }

            if (id_used)
            {

                //发送者
                leveldb::Status status3 = Fdb->Get(leveldb::ReadOptions(), sender, &recv_s);

                rd.parse(recv_s, recv_from_db);

                for (int i = 0; i < (int)recv_from_db.size(); i++)
                {

                    member = recv_from_db[i];
                    if (member["sender"] == recver)
                    {

                        if (member["opt"] == ADD_FRIEND)
                        {

                            //对方也发了请求
                            member["opt"] = BE_FRIENDS;
                            recv_from_db.removeIndex(i, &deleteValue);

                            recv_from_db.append(member);
                            s = w.write(recv_from_db);
                            cout << "S1: " << s << endl;

                            leveldb::Status status = Fdb->Put(leveldb::WriteOptions(), sender, s);

                            //修改对方的好友
                            leveldb::Status status2 = Fdb->Get(leveldb::ReadOptions(), recver, &sr);
                            rd.parse(sr, recv_from_db2);
                            cout << "sr: " << sr << "#" << (int)recv_from_db2.size() << endl;
                            for (int j = 0; j < (int)recv_from_db2.size(); j++)
                            {
                                member2 = recv_from_db2[j];
                                if (member2["sender"] == sender && member2["recver"] == recver)
                                {
                                    //修改对方的好友列表
                                    member2["opt"] = BE_FRIENDS;
                                    recv_from_db2.removeIndex(j, &deleteValue);

                                    recv_from_db2.append(member2);
                                    s = w.write(recv_from_db2);
                                    cout << "S2: " << s << endl;

                                    leveldb::Status status = Fdb->Put(leveldb::WriteOptions(), recver, s);
                                }
                            }
                            if (!(int)recv_from_db2.size())
                            {
                                member2["opt"] = BE_FRIENDS;
                                member2["sender"] = sender;
                                member2["recver"] = recver;

                                recv_from_db2.append(member2);
                                s = w.write(recv_from_db2);
                                cout << "S3 :" << s << endl;

                                leveldb::Status status = Fdb->Put(leveldb::WriteOptions(), recver, s);
                            }
                            Net::Write(clie_fd, "befriends", 9);
                        }
                        else if (member["opt"] == BE_FRIENDS)
                        {

                            //已经是好友
                            Net::Write(clie_fd, "befriends", 9);
                        }
                    }
                    else if (!mas)
                    {
                        //不是好友

                        leveldb::Status status2 = Fdb->Get(leveldb::ReadOptions(), recver, &sr);
                        rd.parse(sr, recv_from_db2);
                        rd.parse(r, member3);

                        recv_from_db2.append(member3);
                        s = w.write(recv_from_db2);
                        leveldb::Status status = Fdb->Put(leveldb::WriteOptions(), recver, s);
                        check_status(status);

                        Net::Write(clie_fd, "success", 7);
                    }
                    else
                    {

                        Net::Write(clie_fd, "fail", 7);
                    }
                }

                if (!(int)recv_from_db.size())
                {

                    Net::Write(clie_fd, "success", 7);
                    rd.parse(r, member3);
                    allv.append(member3);
                    s = w.write(allv);
                    leveldb::Status status = Fdb->Put(leveldb::WriteOptions(), recver, s);
                    check_status(status);
                }
            }
            else
            {
                //无此用户
                Net::Write(clie_fd, "fail", 4);
            }
        }
    }
    return;
}

void Server::cout_friend(int clie_fd, string opt)
{
    Value recv_from_db;
    Value member;
    string recv;
    string s;

    FastWriter w;
    Reader rd;

    char r[BUFSIZ];

    leveldb::Status status = Fdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &recv);
    rd.parse(recv, recv_from_db);

    for (int i = 0; i < (int)recv_from_db.size(); i++)
    {

        member = recv_from_db[i];
        s = w.write(member);

        if (member["recver"] == fd_ID[clie_fd])
        {

            if (member["opt"] == ADD_FRIEND && opt == MAS_FRIEND)
            {
                Net::Write(clie_fd, s.c_str(), s.length());
                while (true)
                {
                    bzero(r, sizeof(r));
                    if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
                    {
                        break;
                    }
                }
            }
            else if (member["opt"] == BE_FRIENDS && opt == BE_FRIENDS)
            {

                Net::Write(clie_fd, s.c_str(), s.length());
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
        else
        {

            Net::Write(clie_fd, "NULL", 4);
            while (true)
            {
                if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
                {
                    break;
                }
            }
        }
    }
    if (!(int)recv_from_db.size())
    {

        Net::Write(clie_fd, "NULL", 4);
        while (true)
        {
            if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
            {
                break;
            }
        }
    }

    Net::Write(clie_fd, "END", 3);

    return;
}

void Server::mas_friend(int clie_fd)
{
    cout << "进入操作请求" << endl;
    char r[BUFSIZ];
    char s[BUFSIZ];
    Value recv;

    FastWriter w;
    Reader rd;

    string sender, recver, send;

    while (true)
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R: " << r << endl;
            if (strcmp(r, "q") == 0)
            {
                break;
            }
            rd.parse(r, recv);
            if (recv["opt"] == ADD_FRIEND)
            {
                send = w.write(recv);
                strcpy(s, send.c_str());
                add_friend(clie_fd, s);
                cout << "退出加好友页面" << endl;
            }
            else if (recv["opt"] == DEL_FRIEND)
            {
            }
        }
        bzero(r, sizeof(r));
    }
}

void Server::friends_menu(int clie_fd)
{
    cout << "进入好友管理页面" << endl;
    char r[BUFSIZ];
    while (true)
    {

        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r) > 0))
        {
            if (strcmp(r, ADD_FRIEND) == 0)
            {
                add_friend(clie_fd, "NULL");
                cout << "结束添加好友" << endl;
            }
            else if (strcmp(r, DEL_FRIEND) == 0)
            {
            }
            else if (strcmp(r, VIEW_FRIENDS) == 0)
            {
                cout_friend(clie_fd, BE_FRIENDS);
            }
            else if (strcmp(r, MAS_FRIEND) == 0)
            {
                cout_friend(clie_fd, MAS_FRIEND);
                cout << "退出输出列表" << endl;
                mas_friend(clie_fd);
                cout << "结束查看列表" << endl;
            }
            else if (strcmp(r, IGN_FRIEND) == 0)
            {
            }
            else if (strcmp(r, EXIT) == 0)
            {
                break;
            }
        }
    }
    return;
}