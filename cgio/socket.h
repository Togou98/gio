#ifndef __T_SOCKET_H
#define __T_SOCKET_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "poll.h"
using namespace std;
sockaddr* parseAddr(string addr);
int createListenSocket(string);
int setNonblock(int);
#endif