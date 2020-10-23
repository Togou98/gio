#include "poll.h"

Poller::Poller(int lfd,int idx) : epfd(epoll_create1(0)), listenFd(lfd), conns(map<int, Conn *>())
{
   index = idx;
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

void Conn::setNonBlock(){
    setNonblock(fd);
}
void Conn::parseIpPort(const struct sockaddr_in& addr){
    port = ntohs(addr.sin_port);
    ip = string(inet_ntoa(addr.sin_addr));
}
void Poller::handleNewConn(const int fd)
{
   while (true){
      struct sockaddr_in caddr = {0}; //为应用层获取客户端的IP和端口号
      socklen_t len = sizeof(struct sockaddr_in);
      
      int nfd = accept(this->listenFd, (struct sockaddr*)&caddr, &len);
      if (nfd < 0)
      {
         if (errno == EAGAIN) return;
         if (errno == EINTR) continue;
      } else {
         Conn* c = new Conn;
         c->fd = nfd;
         c->father = this;
         c->parseIpPort(caddr);
         c->setNonBlock();
         this->count++;
         this->conns[c->fd] = c;
         this->add(c->fd,EPOLLIN|EPOLLOUT|EPOLLET);
      }
   }
}

void Poller::Wait(Server* s,std::function<int(Conn* c,int op)> fn){
   cout<<"Thread "<< who()<< " Step IN wait" <<endl;
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
      if(ready <= 0){
        cout<<"Thread ["<<who()<<"] Timeout "<<endl;
        continue;
      }
      for (int i = 0; i < ready; i++)
      {
         int fd = events[i].data.fd;
         auto ev = events[i].events;
         if (fd == listenFd)
         {  
            cout<<"May Be New Conn Comein" <<endl;
            handleNewConn(fd);
         }else if( ev | EPOLLIN ){//datain
            fn(conns[fd],1);
         }else if (ev | EPOLLOUT){
            fn(conns[fd],2);
         }
      }
      if(ready == this->evsize && this->evsize <= 4096){
         this->evsize =  (this->evsize << 1);
      }
      GC(events);
   }
}
Poller::~Poller(){

}