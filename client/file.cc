#include "clientClass.h"
#include "net_wrap.h"

struct file_wrap
{
    long size;
    char name[1024];
};

void Client::send_file(int clie_fd)
{
    struct file_wrap fw;
    struct stat statbuf;
    char *filename;
    string path;
    char sendbuf[BUFSIZ];

    while (true)
    {
        cout << " 请输入要发送的文件的路径\n>";
        cin >> path;
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

        // cout << "文件:" << fw.name << "开始发送" << endl;
        // sendfile(clie_fd, fp, 0, statbuf.st_size);
        // cout << "文件:" << fw.name << "发送成功" << endl;
        long ret, retw;
        long sum = 0;
        while (true)
        {
            if ((ret = read(fp, sendbuf, BUFSIZ)) > 0)
            {

                retw = write(clie_fd, sendbuf, ret);
                sum += retw;
                cout << sum << endl;

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

void Client::recv_file(int clie_fd)
{
    struct file_wrap fw;

    char r[BUFSIZ];
    char rf[BUFSIZ];
    char path[BUFSIZ];

    string filename;
    char recvpath[BUFSIZ];

    cout << " 请输入要保存的文件名\n>";
    cin >> filename;
    cout << " 请输入要保存的路径\n>";
    cin >> recvpath;

    Net::Write(clie_fd, filename.c_str(), filename.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, BUFSIZ) > 0)
        {
            break;
        }
    }
    if (strcmp(r, "fail") == 0)
    {
        cout << "无此文件" << endl;
        return;
    }

    memcpy(&fw, r, sizeof(fw));
    cout << fw.size << "=======" << fw.name << endl;

    sprintf(path, "%s%s", recvpath, fw.name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == NULL)
    {
        perror("无效的路径");

        return;
    }

    long ret;
    long sum = 0;
    while (true)
    {
        // sleep(0.05);
        bzero(rf, sizeof(rf));
        if ((ret = read(clie_fd, rf, sizeof(rf))) >= 0)
        {

            sum += ret;
            // cout << sum << endl;
            write(fd, rf, ret);
            if (sum >= fw.size)
            {
                cout << "BREAK" << endl;
                break;
            }
        }
    }
    cout << path << endl;
    close(fd);

    return;
}

void Client::file_menu(int clie_fd, string opt)
{
    string path;
    string in;
    char r[BUFSIZ];

    if (opt == "recv")
    {
        cout << "接收文件" << endl;
    }
    else if (opt == "send")
    {
        cout << "发送文件" << endl;
    }
    cout << " 群组文件(g)/个人文件(p)\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());
    cout << " 请输入操作对象ID\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R  " << r << endl;
            break;
        }
    }

    if (strcmp(r, "success") == 0)
    {
        if (opt == "recv")
        {
            recv_file(clie_fd);
        }
        else if (opt == "send")
        {
            send_file(clie_fd);
        }
    }
    else if (strcmp(r, "fail") == 0)
    {
        cout << "你无此权限" << endl;
    }
    else
    {
        cout << "取消操作" << endl;
    }

    return;
}

void Client::cout_file(int clie_fd)
{
    char r[BUFSIZ];
    Value getv;
    Reader rd;
    while (true)
    {

        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            if (strcmp(r, "END") == 0)
            {
                break;
            }
            rd.parse(r, getv);
            cout << "来自" << getv["sender"].asString() << "的文件：" << getv["fname"] << endl;

            Net::Write(clie_fd, ACCEPT, sizeof(ACCEPT));
        }
    }

    return;
}