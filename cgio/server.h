#ifndef _SERVER_H_
#define _SERVER_H_
#include <vector>
#include <memory>
#include <thread>
#include "poll.h"
#include "socket.h"
#include "signal.h"
#include <mutex>
using namespace std;
using error = void*;
class Poller;
class Conn;
class Server{
public:
    Server();
    Server(string);
    ~Server();
    void run();
    std::function<char*(Conn*,char*)> Data;
    std::function<void(Conn*,void*)> PreSet;
    int threadNum;
    int loopInterval;
    int lsfd;
private:
    vector<shared_ptr<Poller>> pollers;
};
 error ReadFunc(Server *s,shared_ptr<Poller> p,Conn* c);
 error WriteFunc(Server *s,shared_ptr<Poller> p,Conn* c);
#endif