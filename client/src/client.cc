#include "clientClass.h"
#include "net_wrap.h"

int main()
{
    Client client(SERV_PORT, "192.168.30.161");
    client.run();
}