#include "clientClass.h"
#include "net_wrap.h"
void Client::thread_newmas(int new_fd, string ID)
{
    Value getv;
    string gets;
    Reader rd;
    cout << "进入newmas" << endl;
    char r[BUFSIZ];
    cout << "ID" << ID << endl;
    while (true)
    {
        bzero(r, sizeof(r));
        if (read_line(new_fd, r, sizeof(r)) > 0)
        {
            cout << " " << endl;
            rd.parse(r, getv);
            cout << "#" << getv["opt"].asString() << "#" << getv["sender"].asString();
            cout << " " << endl;
            // if (getv["opt"].asString() == "newfm")
            // {
            //     cout << "有来自" << getv["sender"].asString() << "的文件";
            // }
            // if (getv["opt"].asString() == "newmas")
            // {
            //     cout << "有来自" << getv["sender"].asString() << "的消息";
            // }
        }
    }
}