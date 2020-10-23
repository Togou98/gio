#ifndef _POLLER_
#define _POLLER_
#include <sys/epoll.h>
#include <functional>
#include <map>
#include <atomic>
#include <utility>
#include "server.h"
#include "socket.h"
#include "util.h"
class Server;
class Poller;
class Conn{
    public:
    Conn(){};
    ~Conn(){
        father = nullptr;
        close(fd);
        GC(this);
    }
    void parseIpPort(const struct sockaddr_in&);
    void setNonBlock();
    Poller *father = nullptr; 
    int fd;
    string ip;
    string port;
    char *in;
    int insize;
    char *out;
    int outsize;
    //
};
class Poller
{
public:
   Poller();
   Poller(int,int);
   ~Poller();
    void Wait(Server*,function<int(Conn*,int)>);
   void mod(int, int);
   void add(int, int);
   void del(int, int);
   int who() const{
      return index;
   };
private:
   void handleNewConn(const int fd);
   int listenFd;
   int epfd;
   int evsize = 2;
   int index;
   map<int, Conn*> conns;
   atomic_int count;
};
#endif