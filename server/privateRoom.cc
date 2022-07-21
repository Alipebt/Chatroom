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
        pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁

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

            /*//(测试用)

            Mdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &outpass);
            cout << "////[数据库]" << fd_ID[clie_fd] << " : " << outpass << endl;
            */

            if (strcmp(r, ROOM_EXIT) == 0)
            {
                pthread_mutex_unlock(&fd_mutex[clie_fd]); //解锁
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
        }

        pthread_mutex_unlock(&fd_mutex[clie_fd]); //解锁

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

    Value number;

    string recverID = fd_ID[clie_fd];
    string send_to_db;
    string send;

    char r[BUFSIZ];
    int i = 0;
    bool is_first_open = true;

    leveldb::Status status;
    string oldmassage;
    Value all_massage;

    while (true)
    {

        leveldb::Iterator *it = Mdb->NewIterator(leveldb::ReadOptions());

        for (it->SeekToFirst(); it->Valid(); it->Next())
        {

            if (recverID == it->key().ToString())
            {
                rd.parse(it->value().ToString(), recv_from_db);
                break;
            }
        }

        if (is_first_open && i >= recv_from_db.size() - 1)
        {
            is_first_open = false;
        }

        for (; i < recv_from_db.size(); i++)
        {
            number = recv_from_db[i];

            if (number["sender"].asString() == senderID || number["sender"].asString() == fd_ID[clie_fd])
            {

                // cout << "1------------------" << endl;

                if (number["massage"].asString() != ROOM_EXIT)
                {
                    if (!is_first_open && number["sender"].asString() == fd_ID[clie_fd])
                    {
                        continue;
                    }
                }
                send = w.write(number);
                pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁
                Net::Write(clie_fd, send.c_str(), send.length());

                while (true)
                {
                    read(clie_fd, r, sizeof(r));
                    // cout << "2------------------" << endl;
                    if (strcmp(r, ACCEPT) == 0)
                    {

                        pthread_mutex_unlock(&fd_mutex[clie_fd]); //解锁
                        break;
                    }
                    bzero(r, sizeof(r));
                }

                if (number["massage"].asString() == ROOM_EXIT && number["sender"].asString() == fd_ID[clie_fd])
                {
                    // cout << "3------------------" << endl;
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
                    return;
                }
            }

            if (recv_from_db.size() > 50) //数据库最多存50条消息
            {
                recv_from_db.removeIndex(0, &deleteValue);
                send_to_db = w.write(recv_from_db);

                pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁
                Mdb->Put(leveldb::WriteOptions(), recverID, send_to_db);
                pthread_mutex_unlock(&fd_mutex[clie_fd]);

                i--;
            }

            sleep(0.1);
        }
        sleep(0.1);
    }
    cout << "服务器发送线程关闭" << endl;
    return;
}

void Server::match_with(int clie_fd)
{
    cout << "进入match_with" << endl;
    Reader rd;
    Value match;
    string recverID;

    char r[BUFSIZ];

    bool is_match = false;

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
        //查找所连接的ID是否存在
        leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            if (recverID == it->key().ToString())
            {
                Net::Write(clie_fd, "success", 7);
                cout << clie_fd << "与" << recverID << "匹配成功" << endl;
                is_match = true;

                thread send(thread_send, clie_fd, recverID);
                thread recv(thread_recv, clie_fd, recverID);

                send.join();
                recv.join();
                cout << "已退出连接" << endl;
                break;
            }
        }
    }
    else
    {
        cout << "解析失败" << endl;
    }
    if (!is_match)
    {
        Net::Write(clie_fd, "fail", 4);
        cout << clie_fd << "与" << recverID << "匹配失败" << endl;
    }
    return;
}