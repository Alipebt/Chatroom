#include "net_wrap.h"
#include "serverClass.h"

void Server::main_menu(int clie_fd)
{

    char r[BUFSIZ];

    while (fd_in[clie_fd])
    {
        if (read(clie_fd, r, sizeof(r)) > 0)
        {
            cout << "进入main_menu的选项判断" << r << endl;

            if (strcmp(r, PRIVATE) == 0)
            {
                match_with(clie_fd);
                cout << "已返回主菜单" << endl;
            }
            else if (strcmp(r, PUBLIC) == 0)
            {
                cout << "错误2" << endl;
            }
            else if (strcmp(r, FRIENDS_MENU) == 0)
            {
                cout << "错误3" << endl;
            }
            else if (strcmp(r, SIGN_OUT) == 0)
            {
                cout << "执行exit" << endl;

                fd_in[clie_fd] = false;
                fd_ID[clie_fd] = "0";

                break;
            }
            else
            {
                cout << "错误else" << endl;
            }
        }
        bzero(r, sizeof(r));
    }
    cout << "退出main_menu" << endl;
    return;
}