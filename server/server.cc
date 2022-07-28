#include <pthread.h>
#include "net_wrap.h"
#include "serverClass.h"

int main()
{
    string ip;
    cout << "输入ip" << endl;
    cin >> ip;
    Server server(SERV_PORT, ip);
    server.run();
}