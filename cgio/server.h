#include <vector>
#include "poll.h"
#include <memory>
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

Server::Server():threadNum(1),loopInterval(20),lsfd(createListenSocket("127.0.0.1:8080")){
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



void PollerThreadEntrance();