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
    // char path[BUFSIZ];
    string path;

    Value putv, member;
    string puts, gets;
    FastWriter w;
    Reader rd;

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }
    if (strcmp(r, "fail") == 0)
    {
        return;
    }
    memcpy(&fw, r, sizeof(fw));
    cout << fw.size << "=======" << fw.name << endl;

    // sprintf(path, "/tmp/serverdata/file/%s", fw.name); //
    if (gorp == "g")
    {
        path = PATHG + ID + "/" + fw.name;
    }
    else if (gorp == "p")
    {
        path = PATHP + ID + "/" + fd_ID[clie_fd] + "/" + fw.name;
    }

    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    // FILE *fp = fopen(r, "wb"); //

    if (fd == NULL)
    {
        perror("无法加载文件");
        exit(1);
    }

    long ret;
    long sum = 0;
    while (true)
    {
        bzero(rf, sizeof(rf));
        if ((ret = read(clie_fd, rf, sizeof(rf))) > 0)
        {
            sum += ret;
            cout << sum << endl;

            write(fd, rf, ret);

            if (sum >= fw.size /*< BUFSIZ*/)
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
    char sendbuf[BUFSIZ];

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

        // int fp = open(path.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
        int fp = open(path.c_str(), O_RDONLY);

        // cout << "文件:" << fw.name << "开始发送" << statbuf.st_size << endl;
        // sendfile(clie_fd, fp, 0, statbuf.st_size);
        // cout << "文件:" << fw.name << "发送成功" << endl;
        long ret;
        long sum = 0;
        while (true)
        {
            sleep(0.1);
            if ((ret = read(fp, sendbuf, BUFSIZ)) > 0)
            {
                sum += ret;
                cout << sum << endl;
                write(clie_fd, sendbuf, ret);

                if (ret < BUFSIZ)
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
        if (read(clie_fd, gorp, sizeof(gorp)) > 0)
        {
            break;
        }
    }
    cout << gorp << endl;
    gorps = gorp;
    while (true)
    {
        bzero(ID, sizeof(ID));
        if (read(clie_fd, ID, sizeof(ID)) > 0)
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