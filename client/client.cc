#include "clientClass.h"
#include "net_wrap.h"

int main()
{
    Client client(SERV_PORT, "192.168.1.160");
    client.run();
}