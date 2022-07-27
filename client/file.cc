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
            continue;
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
        long ret;
        long sum = 0;
        while (true)
        {
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
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }
    memcpy(&fw, r, sizeof(fw));
    cout << fw.size << "=======" << fw.name << endl;

    sprintf(path, "%s%s", recvpath, fw.name);

    cout << path << endl;

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);

    if (fd == NULL)
    {
        perror("无法加载文件");
        exit(1);
    }

    // while (true)
    // {
    //     cout << ".";
    //     bzero(rf, sizeof(rf));
    //     if (read(clie_fd, rf, sizeof(rf)) > 0)
    //     {
    //         cout << ".";
    //         write(fd, rf, sizeof(rf));
    //         sum += RECVBUF;
    //         if (sum >= fw.size)
    //         {
    //             cout << "break;" << endl;
    //             break;
    //         }
    //     }
    // }
    long ret;
    long sum = 0;
    while (true)
    {
        // sleep(0.05);
        bzero(rf, sizeof(rf));
        if ((ret = read(clie_fd, rf, sizeof(rf))) > 0)
        {

            sum += ret;
            cout << sum << endl;
            write(fd, rf, ret);
            if (sum >= fw.size)
            {
                cout << "BREAK" << endl;
                break;
            }
        }
    }

    return;
}