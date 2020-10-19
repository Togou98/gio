#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstring>
using namespace std;
sockaddr* parseAddr(string addr);
int createListenSocket(string);
int setNonblock(int);
class Conn;
Conn* acceptNewConn(int);
template<typename T>
void GC(T ptr){
    free(ptr);
    ptr = 0;
}
int createListenSocket(string addr){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd < 0){
        cerr<<"Socket Create Error"<<endl;
        return -1;
    }
    auto sockAddr = parseAddr(addr);
   if (bind(fd,sockAddr,sizeof(struct sockaddr_in)) < 0){
       cerr<<"Socket Bind Error"<<endl;
       return -2;
   };
   int t = 1;
   if (setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&t,sizeof(t)) < 0){
       cerr<<"Socket SetReusePort Error"<<endl;
       return -3;
   }
   if( listen(fd,0x400) > 0){
       cerr<<"Socket Listen Error"<<endl;
       return -4;
   } 
   cerr<<"Socket Listen At "<<addr<<endl;
    return fd;
}

sockaddr* parseAddr(string addr){
    auto pos = addr.find(":");
    string ip = addr.substr(0,pos);
    string port = addr.substr(pos+1,addr.size());
    cerr<<"IP: "<<ip<<" Port:" <<port<<endl;
    uint16_t _p =  htons(atoi(port.c_str()));

    struct sockaddr_in localAddr;
    void *paddr = (void*)malloc(sizeof(sockaddr_in));
    if (paddr == nullptr){
        cerr<<"Malloc sockAddr_in Error"<<endl;
        return nullptr;
    } 
    memset(&localAddr,0,sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port =  _p;
    localAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    memcpy(paddr,&localAddr,sizeof(sockaddr_in));
    return (sockaddr*)paddr;

    // dont forget delete
}
int setNonblock(int fd){
    int Flag = fcntl(fd,F_GETFL,0);
   return  fcntl(fd,F_SETFL,Flag | O_NONBLOCK);
}



class Conn{
    public:
    Conn(){};
    ~Conn(){
        close(fd);
        GC(remoteAddr);
        GC(this);
    }
    void parseIpPort();
    void setNonBlock();
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
void Conn::parseIpPort(){
    struct sockaddr_in cliaddr;
     memset(&cliaddr, 0, sizeof(cliaddr));
     socklen_t nl=sizeof(cliaddr);
     getpeername(fd,(struct sockaddr*)&cliaddr,&nl);
     ip  = inet_ntoa(cliaddr.sin_addr);
     port = ntohs(cliaddr.sin_port); 
}
void Conn::setNonBlock(){
    ::setNonblock(fd);
}

Conn *acceptNewConn(int fd){
    socklen_t  len = 0;
    struct sockaddr *addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
    int nfd = accept(fd,addr,0);
    if (nfd < 0){
        if(errno == EAGAIN){
            GC(addr);
            return nullptr;
        }
        if(errno == EINTR){

        }
        cerr<<"Accept NewFd Error"<<endl;
        GC(addr);
        return nullptr;
    }else{
        Conn* c = new Conn;
        c->fd = nfd;
        c->remoteAddr = addr;
        c->parseIpPort();
        return c;
    }
}