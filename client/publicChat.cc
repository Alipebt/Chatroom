#include "clientClass.h"
#include "net_wrap.h"

void Client::thread_send_pub(int clie_fd)
{
    cout << "进入发送线程" << endl;
    string in;
    while (true)
    {
        cin >> in;
        write(clie_fd, in.c_str(), in.length());

        if (in == ROOM_EXIT)
        {
            break;
        }
    }
    cout << "结束发送线程" << endl;
    return;
}

void Client::thread_recv_pub(int clie_fd, string ID)
{
    cout << "进入接收线程" << endl;
    char r[BUFSIZ];
    Reader rd;
    Value recv;
    while (true)
    {
        bzero(r, sizeof(r));
        if (read_line(clie_fd, r, sizeof(r)) > 0)
        {
            rd.parse(r, recv);
            if (recv["massage"].asString() == ROOM_EXIT && recv["sender"].asString() == ID)
            {
                break;
            }

            if (recv["massage"].asString() != ROOM_EXIT)
            {
                if (recv["sender"].asString() != ID)
                {
                    cout << LIGHT_BLUE << "               [" << recv["sender"].asString() << "]:" << recv["massage"].asString() << NONE << endl;
                }
                else
                {
                    cout << recv["massage"].asString() << endl;
                }
            }
        }
    }
    cout << "结束接收线程" << endl;

    return;
}

void Client::publicChat(int clie_fd, string ID)
{
    string in;
    char r[BUFSIZ];

    cout << " 请输入你要进入的群ID\n>";
    cin >> in;
    Net::Write(clie_fd, in.c_str(), in.length());

    while (true)
    {
        bzero(r, sizeof(r));
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            break;
        }
    }

    if (strcmp(r, "success") == 0)
    {
        cout << "已进入群" << in << endl;
        thread send(thread_send_pub, clie_fd);
        thread recv(thread_recv_pub, clie_fd, ID);

        send.join();
        recv.join();
    }
    else if (strcmp(r, "refuse") == 0)
    {
        cout << "你不是群" << in << "的成员" << endl;
    }
    else
    {
        cout << "你不是群成员" << endl;
    }

    return;
}