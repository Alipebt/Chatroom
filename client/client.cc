#include "clientClass.h"
#include "net_wrap.h"

int main()
{
    string ip;
    cout << "输入ip" << endl;
    cin >> ip;
    Client client(SERV_PORT, ip);
    client.run();
}