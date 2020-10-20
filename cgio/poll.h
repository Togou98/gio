#pragma once
#include <sys/epoll.h>
#include <functional>
#include <map>
#include <atomic>
#include <utility>
#include "server.h"
#include "socket.h"
// class Server;
class Conn{
    public:
    Conn(){};
    ~Conn(){
        father = nullptr;
        close(fd);
        GC(remoteAddr);
        GC(this);
    }
    void parseIpPort();
    void setNonBlock();
    Poller *father = nullptr; 
    int fd;
    struct sockaddr* remoteAddr;
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
   Poller(int lfd);
   ~Poller();
   void Wait(Server*, function<int(Conn *c,int op)>);
   void mod(int, int);
   void add(int, int);
   void del(int, int);
   friend class Server;
private:
   void handleNewConn(const int fd);
   int listenFd;
   int epfd;
   int evsize = 2;
   map<int, Conn*> conns;
   atomic_int count;
};

Poller::Poller(int lfd) : epfd(epoll_create1(0)), listenFd(lfd), conns(map<int, Conn *>())
{
   count = 0;
   this->add(lfd, EPOLLIN | EPOLLET);
}
void Poller::mod(int fd, int op)
{
   struct epoll_event ev;
   ev.data.fd = fd;
   ev.events = op;
   epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}
void Poller::add(int fd, int op)
{
   struct epoll_event ev;
   ev.data.fd = fd;
   ev.events = op;
   epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void Poller::del(int fd, int op)
{
   epoll_ctl(epfd, EPOLL_CTL_MOD, fd, nullptr);
}

void Poller::Wait(Server *s, function<int(Conn *c,int op)> itfn)
{
   while (true)
   {
      struct epoll_event *events = (struct epoll_event *)malloc(2 * sizeof(epoll_event));
      int ready = epoll_wait(epfd, events, evsize, s->loopInterval);
      if (ready < 0 && errno != EINTR)
      {
         //epoll err;
         continue;
         GC(events);
      }
      for (int i = 0; i < ready; i++)
      {
         int fd = events[i].data.fd;
         auto ev = events[i].events;
         if (fd == listenFd)
         {
            handleNewConn(fd);
         }else if( ev | EPOLLIN ){//datain
            itfn(conns[fd],1);
         }else if (ev | EPOLLOUT){
            itfn(conns[fd],2);
         }
      }
      if(ready == this->evsize && this->evsize <= 4096){
         this->evsize =  (this->evsize << 1);
      }
      GC(events);
   }
}
void Poller::handleNewConn(const int fd)
{
   while (true)
   {
      socklen_t len = 0;
      struct sockaddr *addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
      int nfd = accept(this->listenFd, addr, 0);
      if (nfd < 0)
      {
         GC(addr);
         if (errno == EAGAIN) return;
         if (errno == EINTR) continue;
      } else {
         Conn* c = new Conn;
         c->fd = nfd;
         c->remoteAddr = addr;
         c->parseIpPort();
         c->setNonBlock();
         c->father = this;
         this->count++;
         this->conns[c->fd] = c;
         this->add(c->fd,EPOLLIN|EPOLLOUT|EPOLLET);
      }
   }
}