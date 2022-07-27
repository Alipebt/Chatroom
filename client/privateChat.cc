#include "clientClass.h"
#include "net_wrap.h"

void Client::thread_send(int clie_fd)
{
    cout << "客户端发送线程开启" << endl;
    char s[BUFSIZ];
    while (true)
    {
        cin >> s;
        // write返回顺利写入字节，若==0或<0则可能对端关闭
        //不用Write以防客户端报错退出
        int ret = write(clie_fd, s, strlen(s));

        if (strcmp(s, ROOM_EXIT) == 0 || ret <= 0)
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

    char r[BUFSIZ];
    while (true)
    {
        //同write
        int ret = read(clie_fd, r, sizeof(r));
        if (ret <= 0)
        {
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
            cout << "客户端接收异常" << endl;
            break;
        }

        rd.parse(r, recv);

        if (recv["massage"].asString() == ROOM_EXIT /*|| recv["massage"].asString() == EXIT*/)
        {
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
            cout << "客户端已收到关闭请求" << endl;
            break;
        }

        if (recv["massage"].asString() != ACCEPT)
        {
            if (recv["sender"].asString() != ID)
            {
                cout << LIGHT_BLUE << "               [" << recv["sender"].asString() << "]:" << recv["massage"].asString() << NONE << endl;
            }
            else
            {
                cout << recv["massage"].asString() << endl;
            }
            Net::Write(clie_fd, ACCEPT, strlen(ACCEPT));
        }
        bzero(r, sizeof(r));
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

        if (read(clie_fd, r, sizeof(r)) > 0)
        {
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
        bzero(r, sizeof(r));
    }
}