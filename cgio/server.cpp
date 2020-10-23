#include "server.h"
using SP = shared_ptr<Poller>;
void PollerThreadEntrance(Server *s, SP);
Server::Server() : threadNum(thread::hardware_concurrency()), loopInterval(1000), lsfd(createListenSocket("127.0.0.1:8080"))
{
    if (lsfd < 0)
    {
        cerr << "Server Create Error In lsfd" << endl;
        exit(-1);
    }
}
Server::Server(string addr) : threadNum(thread::hardware_concurrency()), loopInterval(1000), lsfd(createListenSocket(addr))
{
    if (lsfd < 0)
    {
        cerr << "Server Create Error In lsfd" << endl;
        exit(-1);
    }
}

void Server::run()
{
    if (threadNum <= 0)
    {
        threadNum = 1;
    }
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    for (int i = 0; i < threadNum; i++)
    {
        auto sPollPtr = shared_ptr<Poller>(new Poller(lsfd, i));
        pollers.push_back(sPollPtr);
    }
    vector<std::thread> vThread;

    for (int i = 0; i < threadNum; i++)
    {
        thread t(PollerThreadEntrance, this, pollers[i]);
        vThread.push_back(std::move(t));
    }
    for (auto &t : vThread)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

Server::~Server()
{
    for (auto p : pollers)
    {
        p->~Poller();
    }
}

void PollerThreadEntrance(Server *s, SP p)
{
    try
    {
        cout << "I am thread " << p->who() << endl;
        std::function<int(Conn *, int)> func = [s,p](Conn *c, int op) {
            if (op == 1)
            {
                //read;
                ReadFunc(s,p,c);
            }
            else if (op == 2)
            {
                //write
            }
            return 0;
        };
        p->Wait(s, func);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
};
error ReadFunc(Server *s, shared_ptr<Poller> p, Conn *c)
{
    while(true)
    {
         char buf[4096];
        int got = recv(c->fd, buf, 4096, 0);
        if (got <= 0)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            if (got == 0)
            {
                //close
                return nullptr;
            }
        }
        else
        {
            cout<<"Got Data |"<<buf<<endl;
            if(s->Data){
               auto out = s->Data(c,buf);
               int len = strlen(out);
               if(len > 0){
                   p->mod(c->fd,EPOLLOUT|EPOLLET);
               }
            }
        }
    }

    return nullptr;
}