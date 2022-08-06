#include "serverClass.h"
#include "net_wrap.h"

void Server::thread_worknm(int new_fd, int idfd)
{
    string ID = "0";
    string send, newsend;
    string gets, newgets;
    Value getv, member, newgetv;
    Reader rd;
    FastWriter w;
    Value deleteValue;
    char r[BUFSIZ];

    int time = 0;

    while (ID == "0")
    {
        ID = fd_ID[idfd];

        while (ID != "0")
        {
            ID = fd_ID[idfd];

            // cout << ".";
            leveldb::Status s = NMdb->Get(leveldb::ReadOptions(), ID, &gets);
            rd.parse(gets, getv);
            for (int i = 0; i < (int)getv.size(); i++)
            {
                member = getv[i];
                send = w.write(member);
                cout << "发送文件信息" << send << endl;

                Net::Write(new_fd, send.c_str(), send.length());

                leveldb::Status sss = NMdb->Get(leveldb::ReadOptions(), ID, &newgets);
                rd.parse(newgets, newgetv);
                newgetv.removeIndex(0, &deleteValue);
                newsend = w.write(newgetv);
                // pthread_mutex_lock(&fd_mutex[clie_fd]); //加锁
                NMdb->Put(leveldb::WriteOptions(), ID, newsend);
            }
        }
    }

    return;
}
