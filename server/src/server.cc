#include <pthread.h>
#include "net_wrap.h"
#include "serverClass.h"

int main()
{
    Server server(SERV_PORT, "192.168.0.104");
    server.run();
}