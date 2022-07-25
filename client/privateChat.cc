#include "clientClass.h"
#include "net_wrap.h"

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
                thread recv(thread_recv, clie_fd);

                send.join();
                recv.join();
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
                cout << "连接失败" << endl;
            }
            break;
        }
        bzero(r, sizeof(r));
    }
}