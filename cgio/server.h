#ifndef __SERVER__
#define __SERVER__
#include <string>
#include <vector>
#include <memory>
#include "server.h"
#include <signal.h>
#include "poll.h"
#include "socket.h"
using error = void *;
class Server;
class Poller;
class Conn;
error ReadFunc(Server *s, std::shared_ptr<Poller> p, Conn *c);
error WriteFunc(Server *s, std::shared_ptr<Poller> p, Conn *c);
error CloseFunc(Server *s, std::shared_ptr<Poller> p, Conn *c);
class Server{
public:
    Server();
    Server(std::string addr);
    ~Server();
    void Run();
    std::function<std::string(Conn *, std::string)> Data;
    std::function<void(Conn *)> PreSet;
    int threadNum;
    int loopInterval;
    int lsfd;
private:
    std::vector<std::shared_ptr<Poller>> pollers;
};
#endif