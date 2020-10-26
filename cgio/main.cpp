#include "server.h"
#include "poll.h"
#include "socket.h"
#include<bits/stdc++.h>
void toserver(){
    const string echoback  ="' Echo";
    try
    {
        Server *s = new Server("127.0.0.1:8080");
        cout << "Thread Number " << s->threadNum << endl;
        s->threadNum = 1;
        s->loopInterval = 1000;
        s->PreSet = [](Conn *c){
            c->Ctx = nullptr;
        };
        s->Data = [echoback](Conn* c,string in)->string{
           
           in += echoback;
            return in;
        };
        s->Run();
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
        return;
    }
}
int main()
{
    toserver();
}


/*
int *ptr = (int*)malloc(sizeof(4));
    *ptr  = 1024;
    if(ptr != nullptr){
    GC(ptr);
    GC(ptr);
    }
    cout <<*ptr<<endl;
    int z = 1;
*/