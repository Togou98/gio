#ifndef __T_SOCKET_H
#define __T_SOCKET_H
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "poll.h"
using namespace std;
sockaddr* parseAddr(string addr);
int createListenSocket(string);
int setNonblock(int);
#endif