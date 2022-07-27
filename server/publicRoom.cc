#include "serverClass.h"
#include "net_wrap.h"

void Server::thread_recv_pub(int clie_fd, string gID)
{
    cout << "进入接收线程" << endl;
    Value sendv;
    string gets, send;
    Value getv;
    Reader rd;
    FastWriter w;
    sendv["sender"] = fd_ID[clie_fd];
    char r[BUFSIZ];

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R:  " << r << endl;
            sendv["massage"] = r;

            leveldb::Status s = GMdb->Get(leveldb::ReadOptions(), gID, &gets);
            rd.parse(gets, getv);
            getv.append(sendv);

            send = w.write(getv);
            leveldb::Status s2 = GMdb->Put(leveldb::WriteOptions(), gID, send);

            if (strcmp(r, ROOM_EXIT) == 0)
            {
                break;
            }
        }
    }
    cout << "结束接收线程" << endl;

    return;
}

void Server::thread_send_pub(int clie_fd, string gID)
{
    cout << "进入发送线程" << endl;
    string gets, gets2;
    Reader rd;
    Value getv, getv2, member, delv;
    FastWriter w;
    string send;

    int i = 0;
    bool is_first_open = true;

    while (true)
    {
        sleep(0.05);
        // pthread_mutex_lock(&fd_mutex[clie_fd]);
        leveldb::Status s = GMdb->Get(leveldb::ReadOptions(), gID, &gets);
        rd.parse(gets, getv);

        if (is_first_open && i >= (int)getv.size())
        {
            is_first_open = false;
        }

        if (i > (int)getv.size())
        {
            i = (int)getv.size();
        }
        for (; i < (int)getv.size(); i++)
        {
            sleep(0.05);
            member = getv[i];

            if (member["massage"].asString() != ROOM_EXIT)
            {

                if (!is_first_open && member["sender"].asString() == fd_ID[clie_fd])
                {

                    continue;
                }
            }

            send = w.write(member);
            Net::Write(clie_fd, send.c_str(), send.length());

            if (member["massage"].asString() == ROOM_EXIT && member["sender"].asString() == fd_ID[clie_fd])
            {

                getv.removeIndex(i, &delv);
                send = w.write(getv);

                leveldb::Status s2 = GMdb->Put(leveldb::WriteOptions(), gID, send);
                cout << "SEND:  " << send << endl;

                i--;

                cout << "结束发送线程" << endl;
                is_first_open = true;
                return;
            }

            sleep(0.05);
        }

        if (getv.size() > 50) //数据库最多存50条消息
        {
            // cout << "0000000000007" << endl;
            getv.removeIndex(0, &delv);
            send = w.write(getv);

            leveldb::Status s3 = GMdb->Put(leveldb::WriteOptions(), gID, send);

            i--;
        }
    }
    // pthread_mutex_unlock(&fd_mutex[clie_fd]);
    cout << "结束发送线程" << endl;
    is_first_open = true;
    return;
}

void Server::match_with_pub(int clie_fd)
{
    char r[BUFSIZ];
    string gID;
    string gets;
    string member;
    Value getv;
    Reader rd;

    bool is_member = false;

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }
    gID = r;
    leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), gID, &gets);
    rd.parse(gets, getv);
    if (s.ok())
    {
        for (int i = 0; i < (int)getv["member"].size(); i++)
        {
            member = getv["member"][i].asString();
            if (member == fd_ID[clie_fd])
            {
                is_member = true;
            }
        }

        if (!is_member)
        {
            Net::Write(clie_fd, "fail", 4);
        }
    }
    else
    {
        Net::Write(clie_fd, "fail", 4);
    }

    if (is_member)
    {
        Net::Write(clie_fd, "success", 7);
        thread send(thread_send_pub, clie_fd, gID);
        thread recv(thread_recv_pub, clie_fd, gID);

        send.join();
        recv.join();
    }

    return;
}