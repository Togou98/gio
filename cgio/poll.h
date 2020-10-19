#include <sys/epoll.h>

#include <map>
#include <atomic>
#include "socket.h"
class Poller{
public:   
    Poller();
    Poller(int lfd);
    ~Poller();
    void loop();
    void mod(int,int);
    void add(int,int);
    void del(int,int);
    int listenFd;
    int epfd;
    // void(*pf)() pHandleFunc;
    map<int,Conn*> conns;
    atomic_int count;
};

Poller::Poller(int lfd):epfd(epoll_create1(0)),listenFd(lfd),conns(map<int,Conn*>()){
   count = 0;
   this->add(lfd,EPOLLIN | EPOLLET);
}
void Poller::mod(int fd,int op){
   struct epoll_event ev;
   ev.data.fd =fd;
   ev.events = op;
   epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
}
void Poller::add(int fd,int op){
   struct epoll_event ev;
   ev.data.fd =fd;
   ev.events = op;
   epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
}

void Poller::del(int fd,int op){
   epoll_ctl(epfd,EPOLL_CTL_MOD,fd,nullptr);
}



