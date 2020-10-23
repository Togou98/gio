#include "server.h"
void Run();
int main()
{
    
    Run();
}

void Run()
{
    try
    {
        Server *s = new Server("127.0.0.1:8080");
        cout << "Thread Number " << s->threadNum << endl;
        s->threadNum = 1;
        s->loopInterval = 2000;
        s->Data = [](Conn* c,char *in)->char*{
           return in;
        };
        s->run();
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
        return;
    }
};



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