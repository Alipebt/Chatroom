#include <pthread.h>
#include "net_wrap.h"
#include "serverClass.h"



int main()
{
    Server server(SERV_PORT, "127.0.0.1");
    server.run();
}