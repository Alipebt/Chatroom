#include "serverClass.h"
#include "net_wrap.h"
/**
 *
 *  登录后操作
 *
 */

void Server::thread_recv(int clie_fd, string recverID)
{
    cout << "服务器接收线程开启" << endl;
    Value all_massage_sender;
    Value all_massage_recver;
    Value match_massage;
    match_massage["sender"] = fd_ID[clie_fd];

    Reader rd;
    FastWriter w;

    string send_to_db;
    string oldmassage;

    string outpass;

    leveldb::Status status_sender;
    leveldb::Status status_recver;

    cout << "sender:" << fd_ID[clie_fd] << endl;
    cout << "recver:" << recverID << endl;

    char r[BUFSIZ];
    while (true)
    {
        // pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁

        if (read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) != 0)
        {
            cout << "massage:" << r << endl;
            match_massage["massage"] = r;
            //写给sender
            //取到之前的记录
            status_sender = Mdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &oldmassage);
            if (status_sender.ok())
            {
                rd.parse(oldmassage, all_massage_sender);
            }

            //加入新记录
            all_massage_sender.append(match_massage);

            send_to_db = w.write(all_massage_sender);
            //发送到数据库
            Mdb->Put(leveldb::WriteOptions(), fd_ID[clie_fd], send_to_db);

            //(测试用)

            Mdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &outpass);
            cout << "////[数据库]" << clie_fd << " : " << fd_ID[clie_fd] << " : " << outpass << endl;

            if (strcmp(r, ROOM_EXIT) == 0)
            {
                // pthread_mutex_unlock(&fd_mutex[clie_fd]); //解锁
                break;
            }

            //写给recver
            //取到之前的记录
            status_recver = Mdb->Get(leveldb::ReadOptions(), recverID, &oldmassage);
            if (status_recver.ok())
            {
                rd.parse(oldmassage, all_massage_recver);
            }

            //加入新记录
            all_massage_recver.append(match_massage);

            send_to_db = w.write(all_massage_recver);
            //发送到数据库
            Mdb->Put(leveldb::WriteOptions(), recverID, send_to_db);

            // xinxiaoxi
            string gets;
            Value putv, member;
            string puts;
            leveldb::Status s = NMdb->Get(leveldb::ReadOptions(), recverID, &gets);
            rd.parse(gets, putv);
            member["sender"] = fd_ID[clie_fd];
            member["opt"] = "newmas";
            putv.append(member);
            puts = w.write(putv);
            leveldb::Status s2 = NMdb->Put(leveldb::WriteOptions(), recverID, puts);
        }

        // pthread_mutex_unlock(&fd_mutex[clie_fd]); //解锁

        bzero(r, sizeof(r));
    }
    cout << "服务器接收线程关闭" << endl;
    return;
}

void Server::thread_send(int clie_fd, string senderID) //注意：此时sender与recver应交换
{
    cout << "服务器发送线程开启" << endl;
    Reader rd;
    FastWriter w;
    Value recv_from_db;
    Value deleteValue;

    Value member;

    string recverID = fd_ID[clie_fd];
    string send_to_db;
    string send;

    // char r[BUFSIZ];
    int i = 0;
    bool is_first_open = true;

    leveldb::Status status;
    string oldmassage;
    Value all_massage;

    string gets, getsi;
    Value getv, getv2, memberi, getvi;

    string newgets, newsend;
    Value newgetv;

    leveldb::Status statuc3 = IPdb->Get(leveldb::ReadOptions(), recverID, &getsi);
    if (rd.parse(getsi, getvi))
    {

        for (int j = 0; j < (int)getvi["ignore"].size(); j++)
        {

            memberi = getvi["ignore"][j].asString();

            if (memberi == senderID)
            {

                cout << memberi << senderID << endl;
                ignore = true;
                break;
            }
        }
    }

    // //历史消息
    // leveldb::Status s = Mdb->Get(leveldb::ReadOptions(), recverID, &gets);
    // cout << gets << endl;
    // Net::Write(clie_fd, gets.c_str(), gets.length());
    // rd.parse(gets, getv);
    // i = (int)getv.size();

    //私聊消息

    while (true)
    {
        sleep(0.05);
        // // shanchuxinxiaoxi
        // leveldb::Status sss = NMdb->Get(leveldb::ReadOptions(), recverID, &newgets);
        // rd.parse(newgets, newgetv);
        // for (int i = 0; i < (int)newgetv.size(); i++)
        // {
        //     if (newgetv[i]["sender"] == senderID)
        //     {
        //         newgetv.removeIndex(i, &deleteValue);
        //         newsend = w.write(newgetv);

        //         imas--;

        //         // pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁
        //         NMdb->Put(leveldb::WriteOptions(), recverID, newsend);
        //         i--;
        //     }
        // }
        // ////////////////

        leveldb::Status s = Mdb->Get(leveldb::ReadOptions(), recverID, &gets);
        rd.parse(gets, getv);

        if (is_first_open && i >= (int)getv.size())
        {
            is_first_open = false;
        }

        for (; i < (int)getv.size(); i++)
        {
            sleep(0.05);
            // if (is_first_open)
            // {
            //     is_first_open = false;
            //     continue;
            // }
            member = getv[i];

            if (member["sender"].asString() == senderID || member["sender"].asString() == fd_ID[clie_fd])
            {

                if (member["massage"].asString() != ROOM_EXIT)
                {

                    if (!is_first_open && member["sender"].asString() == fd_ID[clie_fd])
                    {

                        continue;
                    }
                }

                if (memberi == member["sender"].asString())
                {
                    cout << "ignore" << endl;
                    continue;
                }
                send = w.write(member);
                cout << "xSENDx" << send << endl;
                Net::Write(clie_fd, send.c_str(), send.length());

                if (member["massage"].asString() == ROOM_EXIT && member["sender"].asString() == fd_ID[clie_fd])
                {

                    /**/
                    status = Mdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &oldmassage);
                    if (status.ok())
                    {
                        rd.parse(oldmassage, all_massage);
                    }

                    //删除记录

                    all_massage.removeIndex(i, &deleteValue);

                    send_to_db = w.write(all_massage);
                    //发送到数据库
                    Mdb->Put(leveldb::WriteOptions(), fd_ID[clie_fd], send_to_db);
                    // ceshi
                    string outpass;
                    Mdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &outpass);
                    cout << "[Exit]" << fd_ID[clie_fd] << " : " << outpass << endl;

                    cout << "服务器发送线程关闭" << endl;
                    is_first_open = true;
                    return;
                }
            }

            if (recv_from_db.size() > 50) //数据库最多存50条消息
            {
                recv_from_db.removeIndex(0, &deleteValue);
                send_to_db = w.write(recv_from_db);

                // pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁
                Mdb->Put(leveldb::WriteOptions(), recverID, send_to_db);
                // pthread_mutex_unlock(&fd_mutex[clie_fd]);

                i--;
            }

            sleep(0.1);
        }
        sleep(0.1);
    }
    cout << "服务器发送线程关闭" << endl;
    is_first_open = true;
    return;
}

void Server::match_with(int clie_fd)
{
    cout << "进入match_with" << endl;
    Reader rd;
    Value match;
    Value recv_from_db;
    Value member;

    bool is_friend = false;
    bool ignore = false;

    string recverID;
    string senderID;

    string buf;

    // string gets;
    // string members;
    // Value getv;

    char r[BUFSIZ];

    while (true)
    {
        bzero(r, sizeof(r));
        if ((read(clie_fd, r, sizeof(r))) > 0)
        {
            cout << " [客户端]" << clie_fd << ":" << r << endl;
            break;
        }
    }

    if (rd.parse(r, match))
    {
        recverID = match["recver"].asString();
        senderID = match["sender"].asString();

        leveldb::Status status1 = IPdb->Get(leveldb::ReadOptions(), recverID, &buf);
        if (status1.ok())
        {

            leveldb::Status status2 = Fdb->Get(leveldb::ReadOptions(), senderID, &buf);

            if (status2.ok())
            {
                rd.parse(buf, recv_from_db);

                for (int i = 0; i < (int)recv_from_db.size(); i++)
                {
                    member = recv_from_db[i];
                    cout << "###" << member["sender"] << member["recver"] << member["opt"] << endl;
                    if (member["sender"] == recverID && member["recver"] == senderID && member["opt"] == BE_FRIENDS)
                    {

                        is_friend = true;
                        break;
                    }
                }
                if (is_friend)
                {

                    cout << clie_fd << "与" << recverID << "匹配成功" << endl;

                    Net::Write(clie_fd, "success", 7);
                    thread send(thread_send, clie_fd, recverID);
                    thread recv(thread_recv, clie_fd, recverID);

                    send.join();
                    recv.join();

                    cout << "已退出连接" << endl;
                }
                else
                {
                    Net::Write(clie_fd, "refuse", 6);
                }
            }
            else
            {
                Net::Write(clie_fd, "refuse", 6);
            }
        }
        else
        {
            Net::Write(clie_fd, "NULL", 4);
        }
    }
    else
    {
        Net::Write(clie_fd, "fail", 4);
        cout << clie_fd << "与" << recverID << "匹配失败" << endl;
    }

    return;
}