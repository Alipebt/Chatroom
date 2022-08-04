#include <pthread.h>
#include "net_wrap.h"
#include "serverClass.h"

int main()
{
    Server server(SERV_PORT, "192.168.30.163");
    server.run();
}