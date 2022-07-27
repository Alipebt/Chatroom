#include "net_wrap.h"
#include "serverClass.h"

void Server::main_menu(int clie_fd)
{
    /////////////////////////////////////////////////
    //正向遍历        (测试用)
    leveldb::Iterator *it = IPdb->NewIterator(leveldb::ReadOptions());

    cout << "-------------------" << endl;
    cout << "ID_PASS:" << endl;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "key: " << it->key().ToString() << " value: " << it->value().ToString() << endl;
    }
    cout << "-------------------" << endl;

    it = Fdb->NewIterator(leveldb::ReadOptions());
    cout << "FRIENDS:" << endl;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "key: " << it->key().ToString() << " value: " << it->value().ToString() << endl;
    }
    cout << "-------------------" << endl;

    it = Gdb->NewIterator(leveldb::ReadOptions());
    cout << "GROUP:" << endl;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << "key: " << it->key().ToString() << " value: " << it->value().ToString() << endl;
    }
    cout << "-------------------" << endl;

    ///////////////////////////////////////////////
    char r[BUFSIZ];

    while (fd_in[clie_fd])
    {

        if (read(clie_fd, r, sizeof(r)) > 0)
        {

            if (strcmp(r, PRIVATE) == 0)
            {
                match_with(clie_fd);
            }
            else if (strcmp(r, GROUP) == 0)
            {
                group_menu(clie_fd);
            }
            else if (strcmp(r, FRIENDS_MENU) == 0)
            {
                friends_menu(clie_fd);
            }
            else if (strcmp(r, SIGN_OUT) == 0)
            {
                cout << "执行exit" << endl;

                fd_in[clie_fd] = false;
                fd_ID[clie_fd] = "0";

                break;
            }
            else if (strcmp(r, "4") == 0)
            {
                recv_file(clie_fd);
            }
            else if (strcmp(r, "5") == 0)
            {
                send_file(clie_fd);
            }
            else
            {
                cout << "错误else" << endl;
            }

            cout << "已返回主菜单" << endl;
        }
        bzero(r, sizeof(r));
    }
    cout << "退出main_menu" << endl;
    return;
}