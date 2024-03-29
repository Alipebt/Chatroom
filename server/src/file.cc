#include "net_wrap.h"
#include "serverClass.h"

struct file_wrap
{
    long size;
    char name[1024];
};

void Server::recv_file(int clie_fd, string gorp, string ID)
{

    struct file_wrap fw;

    char r[BUFSIZ];
    char rf[BUFSIZ];
    string path;
    string gets, puts;
    Value putv, member;
    Reader rd;
    FastWriter w;

    while (true)
    {
        bzero(r, sizeof(r));
        if (Net::Read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "正在获取文件信息" << endl;
            break;
        }
    }
    if (strcmp(r, "fail") == 0)
    {
        return;
    }
    memcpy(&fw, r, sizeof(fw));
    cout << fw.size << "=======" << fw.name << endl;

    if (gorp == "g")
    {
        path = PATHG + ID + "/" + fw.name;
    }
    else if (gorp == "p")
    {
        path = PATHP + ID + "/" + fd_ID[clie_fd] + "/" + fw.name;
    }

    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);

    if (fd == NULL)
    {
        perror("无法加载文件");
        exit(1);
    }

    long ret, ret2;
    long sum = 0;
    long sum2 = 0;
    while (true)
    {
        bzero(rf, sizeof(rf));
        if ((ret = Net::Read(clie_fd, rf, sizeof(rf))) > 0)
        {

            ret2 = write(fd, rf, ret);
            if (ret2 > 0)
            {
                sum2 += ret2;
            }
            cout << sum2 << endl;

            if (sum2 >= fw.size)
            {
                cout << "BREAK" << endl;
                break;
            }
        }
    }

    leveldb::Status s = NMdb->Get(leveldb::ReadOptions(), ID, &gets);
    rd.parse(gets, putv);
    member["sender"] = fd_ID[clie_fd];
    member["opt"] = "newfm";
    member["fname"] = fw.name;
    putv.append(member);
    puts = w.write(putv);
    leveldb::Status s2 = NMdb->Put(leveldb::WriteOptions(), ID, puts);

    return;
}
void Server::send_file(int clie_fd, string gorp, string ID)
{
    struct file_wrap fw;
    struct stat statbuf;
    char r[BUFSIZ];
    char *filename;
    string path, name;
    char sendbuf[BUFSIZ]; // bufsiz
    while (true)
    {
        while (true)
        {
            bzero(r, sizeof(r));
            if (Net::Read(clie_fd, r, sizeof(r)) > 0)
            {
                break;
            }
        }
        name = r;
        if (gorp == "g")
        {
            path = PATHG + ID + "/" + name;
        }
        else if (gorp == "p")
        {
            path = PATHP + fd_ID[clie_fd] + "/" + ID + "/" + name;
        }

        if (stat(path.c_str(), &statbuf) == -1)
        {
            cout << "无效的路径" << endl;
            Net::Write(clie_fd, "fail", 4);
            break;
        }

        fw.size = statbuf.st_size;
        filename = (char *)basename(path.c_str());
        strcpy(fw.name, filename);

        Net::Write(clie_fd, &fw, sizeof(fw));

        int fp = open(path.c_str(), O_RDONLY);

        long ret, retw;
        long sum = 0;
        while (true)
        {
            if ((ret = Net::Read(fp, sendbuf, BUFSIZ)) > 0)
            {

                retw = write(clie_fd, sendbuf, ret);
                if (retw > 0)
                {
                    sum += retw;
                }
                cout << sum << endl;
                if (ret > retw)
                {
                    cout << "重设偏移" << endl;
                    lseek(fp, sum, SEEK_SET);
                }

                if (sum >= fw.size)
                {
                    cout << "BREAK" << endl;
                    break;
                }
                bzero(sendbuf, BUFSIZ);
            }
        }

        Net::Close(fp);

        break;
    }

    return;
}

void Server::file_menu(int clie_fd, string opt)
{
    char gorp[BUFSIZ];
    string gorps;
    char ID[BUFSIZ];
    string IDs;
    string gets;
    Value getv;
    string member;
    Reader rd;

    bool success = false;

    while (true)
    {
        bzero(gorp, sizeof(gorp));
        if (Net::Read(clie_fd, gorp, sizeof(gorp)) > 0)
        {
            break;
        }
    }
    cout << gorp << endl;
    gorps = gorp;
    while (true)
    {
        bzero(ID, sizeof(ID));
        if (Net::Read(clie_fd, ID, sizeof(ID)) > 0)
        {
            break;
        }
    }
    cout << ID << endl;
    IDs = ID;

    if (strcmp(gorp, "g") == 0)
    {
        leveldb::Status s = Gdb->Get(leveldb::ReadOptions(), ID, &gets);
        rd.parse(gets, getv);
        if (s.ok())
        {
            for (int i = 0; i < (int)getv["member"].size(); i++)
            {
                member = getv["member"][i].asString();
                if (member == fd_ID[clie_fd])
                {
                    success = true;
                    Net::Write(clie_fd, "success", 7);
                    if (opt == "send")
                    {
                        send_file(clie_fd, gorps, IDs);
                    }
                    else if (opt == "recv")
                    {
                        recv_file(clie_fd, gorp, IDs);
                    }
                }
            }
        }

        if (!success)
        {
            Net::Write(clie_fd, "fail", 4);
        }
    }
    else if (strcmp(gorp, "p") == 0)
    {
        leveldb::Status s = Fdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &gets);
        rd.parse(gets, getv);
        for (int i = 0; i < (int)getv.size(); i++)
        {
            member = getv[i]["sender"].asString();
            if (member == ID && getv[i]["opt"] == BE_FRIENDS)
            {
                success = true;
                Net::Write(clie_fd, "success", 7);
                if (opt == "send")
                {
                    send_file(clie_fd, gorps, IDs);
                }
                else if (opt == "recv")
                {
                    recv_file(clie_fd, gorp, IDs);
                }
            }
        }

        if (!success)
        {
            cout << "send   fail" << endl;
            Net::Write(clie_fd, "fail", 4);
        }
    }
    else
    {
        Net::Write(clie_fd, EXIT, sizeof(EXIT));
    }
}

void Server::cout_file(int clie_fd)
{
    cout << "进入发文件" << endl;
    string gets;
    Value getv, member;
    Reader rd;
    FastWriter w;
    string send;
    char r[BUFSIZ];
    leveldb::Status s = NMdb->Get(leveldb::ReadOptions(), fd_ID[clie_fd], &gets);
    rd.parse(gets, getv);
    for (int i = 0; i < (int)getv.size(); i++)
    {

        member = getv[i];
        send = w.write(member);
        cout << "发送文件信息" << send << endl;

        Net::Write(clie_fd, send.c_str(), send.length());

        while (true)
        {
            if (Net::Read(clie_fd, r, sizeof(r)) > 0 && strcmp(r, ACCEPT) == 0)
            {
                cout << "受到" << r << endl;
                break;
            }
        }
    }
    cout << "结束" << endl;
    Net::Write(clie_fd, "END", sizeof("END"));

    return;
}