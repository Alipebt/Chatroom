#include "clientClass.h"
#include "net_wrap.h"

int main()
{
    Client client(SERV_PORT, "127.0.0.1");
    client.run();
}