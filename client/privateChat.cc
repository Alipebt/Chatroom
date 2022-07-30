#include "clientClass.h"
#include "net_wrap.h"

void Client::thread_send(int clie_fd)
{
    cout << "客户端发送线程开启" << endl;
    char s[BUFSIZ];
    string del;
    while (true)
    {
        // cin >> s;
        cin.getline(s, BUFSIZ);
        // write返回顺利写入字节，若<0则可能对端关闭
        //不用Write以防客户端报错退出
        int ret = write(clie_fd, s, strlen(s));

        if (strcmp(s, ROOM_EXIT) == 0 || ret < 0)
        {
            break;
        }

        bzero(s, sizeof(s));
    }
    cout << "客户端发送线程关闭" << endl;
    return;
}
void Client::thread_recv(int clie_fd, string ID)
{
    cout << "客户端接收线程开启" << endl;
    Reader rd;
    Value recv;

    Value getv, member;

    char r[BUFSIZ];

    bool first = true;

    // //历史消息
    // while (first)
    // {
    //     if (read(clie_fd, r, sizeof(r)) > 0)
    //     {
    //         cout << "xRx" << r << endl;
    //         if (rd.parse(r, getv) && getv.size() != 1)
    //         {
    //             cout << "!!!!!" << endl;
    //             if (getv["massage"].asString() == "_exit")
    //             {
    //                 cout << "客户端接收线程关闭" << endl;
    //                 return;
    //             }
    //             for (int i = 0; i < (int)getv.size(); i++)
    //             {
    //                 member = getv[i];
    //                 if (member["sender"].asString() != ID)
    //                 {
    //                     cout << LIGHT_BLUE << "               [" << member["sender"].asString() << "]:" << member["massage"].asString() << NONE << endl;
    //                 }
    //                 else
    //                 {
    //                     cout << ":" << member["massage"].asString() << endl;
    //                 }
    //             }
    //         }
    //         first = false;
    //         break;
    //     }
    // }

    //私聊消息

    while (true)
    {
        //同write
        if (read_line(clie_fd, r, sizeof(r)) > 0)
        {

            if (rd.parse(r, recv))
            {

                if (recv["massage"].asString() == ROOM_EXIT)
                {

                    cout << "客户端已收到关闭请求" << endl;
                    break;
                }

                if (recv["sender"].asString() != ID)
                {
                    cout << LIGHT_BLUE << "               [" << recv["sender"].asString() << "]:" << recv["massage"].asString() << NONE << endl;
                }
                else
                {
                    cout << recv["massage"].asString() << endl;
                }

                bzero(r, sizeof(r));
            }
        }
    }
    cout << "客户端接收线程关闭" << endl;
    return;
}

void Client::privateChat(int clie_fd, string ID)
{
    string in, s;
    char r[BUFSIZ];
    Value match;
    FastWriter w;
    match["sender"] = ID;
    //给服务器发送请求

    cout << " 请输入对方ID\n>";
    cin >> in;
    match["recver"] = in;

    s = w.write(match);

    Net::Write(clie_fd, s.c_str(), s.length());
    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "R" << r << endl;
            if (strcmp(r, "success") == 0)
            {
                cout << "已与" << in << "连接:" << endl;
                thread send(thread_send, clie_fd);
                thread recv(thread_recv, clie_fd, ID);

                send.join();
                recv.join();
            }
            else if (strcmp(r, "ignore") == 0)
            {
                cout << "你已屏蔽该好友" << endl;
            }
            else if (strcmp(r, "NULL") == 0)
            {
                cout << "无用户" << in << endl;
            }
            else if (strcmp(r, "refuse") == 0)
            {
                cout << "用户" << in << "不是你的好友" << endl;
            }
            else
            {
                cout << "无用户" << in << endl;
            }
            break;
        }
    }
}

ssize_t Client::read_peek(int sockfd, void *buf, size_t len)
{
    while (1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret == -1)
        {
            if (errno == EINTR) //出现中断
                continue;
        }
        return ret;
    }
}

ssize_t Client::readn(int fd, void *buf, size_t count)
{
    int left = count; //剩余的字节数
    char *ptr = (char *)buf;
    while (left > 0)
    {
        int readBytes = read(fd, ptr, left);
        if (readBytes < 0) // read函数小于0有两种情况：1中断;2出错
        {
            if (errno == EINTR) //读被中断
            {
                continue;
            }
            else
                return -1;
        }
        if (readBytes == 0) //读到了EOF
        {
            //对方关闭了呀
            printf("peer close\n");
            break;
        }
        left -= readBytes;
        ptr += readBytes;
    }
    return count - left;
}

ssize_t Client::read_line(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nRead = 0;
    int left = maxline; //剩下的字节数
    char *pbuf = (char *)buf;
    int count = 0;
    while (1)
    {
        ret = read_peek(sockfd, pbuf, left); //从socket缓冲区中读取指定长度的内容，但并不删除
        if (ret < 0)
        {
            return ret;
        }
        nRead = ret;
        for (int i = 0; i < nRead; ++i) //看看读取出来的数据中是否有换行符\n
        {
            if (pbuf[i] == '\n') //如果有换行符
            {
                cout << "读到换行" << endl;
                ret = readn(sockfd, pbuf, i + 1); //读取一行
                if (ret != i + 1)                 //一定会读到i+1个字符，否则是读取出错
                {
                    exit(EXIT_FAILURE);
                }
                return ret + count;
            }
        }
        //如果窥探的数据中并没有换行符
        //把这段没有换行符\n的内容读取出来
        ret = readn(sockfd, pbuf, nRead);
        if (ret != nRead)
        {
            exit(EXIT_FAILURE);
        }
        pbuf += nRead;
        left -= nRead;
        count += nRead;
    }

    return -1;
}
