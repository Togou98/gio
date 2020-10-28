#ifndef __POLL__
#define __POLL__
#include <map>
#include <atomic>
#include <functional>
#include "server.h"
#include "httpParser.h"
using namespace std;
class Conn;
class Server;
class Poller
{
public:
   Poller();
   Poller(int, int);
   ~Poller();
   void Wait(Server *, function<int(Conn *, int)>);
   void mod(int, int);
   void add(int, int);
   void del(int, int);
   int who() const;
   std::map<int, Conn *> conns;
   int evsize = 2;
   int index;
   atomic_int count;

private:
   int epfd;
   void handleNewConn(Server *s, int fd);
   int listenFd;
};
class Conn
{
public:
   Conn();
   ~Conn();
   Parser *Ctx;
   void parseIpPort(const struct sockaddr_in &);
   void setNonBlock();
   Poller *father = nullptr;
   int fd;
   string ip;
   int port;
   string in;  // 接收数据
   string out; //发送缓冲
   int outpos; //写位置
   int cnt;
};

template <typename T>
void GC(T ptr)
{
   if (ptr)
   {
      free(ptr);
      ptr = nullptr;
   }
}
template <typename T>
void CPPGC(T heapptr)
{
   if (heapptr)
   {
      delete heapptr;
      heapptr = nullptr;
   }
}
#endif