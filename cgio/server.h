#ifndef _SERVER_
#define _SERVER_
#include <vector>
#include "poll.h"
#include <memory>
class Server;
class Server{
public:
    Server();
    void run();
    int threadNum;
    int loopInterval;
    int lsfd;
private:
    vector<shared_ptr<Poller>> pollers;
};
using SP = shared_ptr<Poller>;

Server::Server():threadNum(1),loopInterval(1000),lsfd(createListenSocket("127.0.0.1:8080")){
    if(lsfd < 0 ){
        cerr<<"Server Create Error In lsfd"<<endl;
        exit(-1);
    }
}

void Server::run(){
    for(int i = 0 ; i < threadNum;i++){
        auto sPollPtr   = shared_ptr<Poller>(new Poller(lsfd));
        pollers.push_back(sPollPtr);
    }
    
}



void PollerThreadEntrance(const Server* s,SP p);
#endif