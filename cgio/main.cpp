#include "socket.h"

int main()
{
    int lfd = createListenSocket("127.0.0.1:8080");
    Conn *c = acceptNewConn(lfd);

    if (c)
    {
        handle(c);
    }
}
