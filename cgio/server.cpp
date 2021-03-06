#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include "server.h"
using namespace std;
const int RECVBUFSIZE = 0x2000;
const size_t SIZEZERO = 0;
void PollerThreadEntrance(Server *, int);

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

void Server::Run()
{
    if (threadNum <= 0)
    {
        threadNum = 1;
    }
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    vector<std::thread> vThread;

    for (int i = 0; i < threadNum; i++)
    {
        thread t(PollerThreadEntrance, this, i);
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
}
error CloseFunc(Server *s, shared_ptr<Poller> p, Conn *c)
{
    cout << "[" << c->ip << ":" << c->port << "] Closed Connection " << endl;
    p->del(c->fd, 0);
    ::close(c->fd);
    p->conns.erase(c->fd);
    p->count--;
    CPPGC(c);
    return nullptr;
}
error ReadFunc(Server *s, shared_ptr<Poller> p, Conn *c)
{
    char buf[RECVBUFSIZE] = "\0"; //8192
    while (true)
    {
        int got = recv(c->fd, buf, RECVBUFSIZE, 0);
        if (got <= 0)
        {
            if (errno == EAGAIN)
            {
                errno = 0;
                break;
            }
            if (errno == EINTR)
            {
                errno = 0;
                continue;
            }
            if (got == 0)
            {
                return ::CloseFunc(s, p, c);
            }
        }
        else
        {
            c->in.append(buf, got); //每次发送数据后 将C in 清空
            if (s->Data)
            {
                string out = s->Data(c, c->in);
                if (out.size() > SIZEZERO)
                {
                    c->out.append(out);
                    p->mod(c->fd, EPOLLOUT | EPOLLET);
                }
            }
            if (got >= RECVBUFSIZE)
            {
                continue;
            }
        }
    }
    return nullptr;
}
error WriteFunc(Server *s, shared_ptr<Poller> p, Conn *c)
{
    while (true)
    {
        size_t sendLen = c->out.size() - c->outpos;
        if (sendLen <= 0)
        {
            return nullptr;
            p->mod(c->fd, EPOLLIN | EPOLLET);
        }
        const char *sendBegin = c->out.c_str() + c->outpos;
        int ok = send(c->fd, (void *)sendBegin, sendLen, 0);
        if (ok <= 0)
        {
            if (errno == EAGAIN)
            {
                errno = 0;
                break;
            }
            if (errno == EINTR)
            {
                errno = 0;
                continue;
            }
            if (ok == 0)
            {
                return ::CloseFunc(s, p, c);
            }
        }
        else
        {
            c->outpos += ok;
            if (c->outpos >= c->out.size())
            {
                c->out.clear();
                c->in.clear();
                c->outpos = 0;
                p->mod(c->fd, EPOLLIN | EPOLLOUT | EPOLLET);
            }
            else
            {
                p->mod(c->fd, EPOLLOUT | EPOLLET);
            }
        }
    }
    return nullptr;
}

void PollerThreadEntrance(Server *s, int i)
{
    try
    {
        thread_local auto PollPtr = shared_ptr<Poller>(new Poller(s->lsfd, i));
        s->pollers.push_back(PollPtr);

        PollPtr->Wait(s, [&](Conn *c, int op) -> int {
            if (op == 1)
                ReadFunc(s, PollPtr, c);
            else if (op == 2)
                WriteFunc(s, PollPtr, c);
            return 0;
        });
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
};
