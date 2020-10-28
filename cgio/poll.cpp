#include <sys/epoll.h>
#include <utility>
#include <unistd.h>
#include <string>
#include "server.h"
#include "socket.h"
#include "poll.h"
#include "httpParser.h"
Poller::Poller(int lfd,int idx) : epfd(epoll_create1(0)), listenFd(lfd), conns(map<int, Conn *>())
{
   index = idx;
   count = 0;
   this->add(lfd, EPOLLIN | EPOLLET);
}
Poller::~Poller(){
        close(this->epfd);
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

void Conn::setNonBlock(){
    setNonblock(fd);
}
int Poller::who() const{
      return index;
}
void Conn::parseIpPort(const struct sockaddr_in& addr){
    port = ntohs(addr.sin_port);
    ip = string(inet_ntoa(addr.sin_addr));
}
void Poller::handleNewConn(Server *s,int fd)
{
   while (true){
      struct sockaddr_in caddr = {0}; //为应用层获取客户端的IP和端口号
      socklen_t len = sizeof(struct sockaddr_in);
      
      int nfd = accept(this->listenFd, (struct sockaddr*)&caddr, &len);
      if (nfd < 0)
      {
         if (errno == EAGAIN){ errno = 0 ;return;} 
         if (errno == EINTR) {errno = 0;continue;}
      } else {
         Conn* c = new Conn;
         c->fd = nfd;
         c->father = this;
         c->outpos  = 0;
         c->parseIpPort(caddr);
         c->setNonBlock();
         this->count++;
         this->conns[c->fd] = c;
         this->add(c->fd,EPOLLIN|EPOLLET);
         if(s->PreSet){
            s->PreSet(c);
         }
         cout<<"Thread["<<who()<<"] Capture New Cli From "<<c->ip <<":"<<c->port<<endl;
      }
   }
}

void Poller::Wait(Server* s,std::function<int(Conn* c,int op)> fn){
   while (true)
   {
      struct epoll_event events[evsize];
      int ready = epoll_wait(epfd, events, evsize, s->loopInterval);
      if (ready < 0)
      {

         if (errno == EINTR){
            errno = 0;
            continue;
            GC(events);
         }
      }
      if(ready <= 0){
        continue;
      }
      for (int i = 0; i < ready; i++)
      {
         int fd = events[i].data.fd;
         auto ev = events[i].events;
         if (fd == listenFd)
         {  
            handleNewConn(s,fd);
            continue;
         }
         if(ev & EPOLLIN ){
            fn(conns[fd],1);
         }else if( ev & EPOLLOUT){
            fn(conns[fd],2);
         }
      }
      if(ready >= this->evsize && this->evsize <= 4096){
         this->evsize =  (this->evsize << 1);
      }
   }
}
Conn::Conn(){
   this->Ctx = nullptr;
   this->father = nullptr;
   this->fd = -1;
}

Conn::~Conn(){
   cnt = 0;
   father = nullptr;
   fd = -1;
   in.clear();
   out.clear();
   if(Ctx) delete Ctx;
}