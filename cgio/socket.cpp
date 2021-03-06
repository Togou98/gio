#include "socket.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
int createListenSocket(string addr)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        cerr << "Socket Create Error" << endl;
        return -1;
    }
    auto sockAddr = parseAddr(addr);
    int t = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &t, sizeof(t)) < 0)
    {
        cerr << "Socket SetReusePort Error" << endl;
        return -2;
    }
    if (bind(fd, sockAddr, sizeof(struct sockaddr_in)) < 0)
    {
        cerr << "Socket Bind Error" << endl;
        GC(sockAddr);
        return -3;
    };

    if (listen(fd, 0x400) > 0)
    {
        cerr << "Socket Listen Error" << endl;
        return -4;
    }
    setNonblock(fd);
    return fd;
}

sockaddr *parseAddr(string addr)
{
    auto pos = addr.find(":");
    string ip = addr.substr(0, pos);
    string port = addr.substr(pos + 1, addr.size());
    uint16_t _p = htons(atoi(port.c_str()));

    struct sockaddr_in localAddr;
    void *paddr = (void *)malloc(sizeof(sockaddr_in));
    if (paddr == nullptr)
    {
        cerr << "Malloc sockAddr_in Error" << endl;
        return nullptr;
    }
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = _p;
    localAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    memcpy(paddr, &localAddr, sizeof(sockaddr_in));
    return (sockaddr *)paddr;

    // dont forget delete
}
int setNonblock(int fd)
{
    int Flag = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, Flag | O_NONBLOCK);
}
