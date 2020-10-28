#include "server.h"
#include "poll.h"
#include "socket.h"
#include "httpParser.h"
#include <bits/stdc++.h>
#include "stdlib.h"
void httpInit()
{
    if (Http::pages.empty())
    {
        ifstream in("./upload.html");
        stringstream ss;
        if (in.is_open())
        {
            ss << in.rdbuf();
            Http::pages = ss.str();
        }
        in.close();
    }
}
void toserver()
{
    try
    {   
    int cnt =0;
        string IPORT =  "127.0.0.1:8080";
        Server *s = new Server(IPORT);
        s->loopInterval = 20;  //wait 间隔
        s->threadNum = 1;   //线程数 不配就为CPU核数
        cout<<"Http Server ListenAnd Serving @"<<IPORT<<" Use ["<<s->threadNum<<"] Threads With "<<s->loopInterval <<" ms"<<endl;
        httpInit();
        s->PreSet = [](Conn *c) {
            Http* p = new Http;
            p->PATH("/get", [](shared_ptr<Request> Req,shared_ptr<Response> Res) {
                Res->WriteString("For Test Server");
                Res->Code = 200;
                Res->Message = "OK";
            });
            p->PATH("/upload", [](shared_ptr<Request> Req,shared_ptr<Response> Res) {
                Res->WriteString(Http::pages);
                Res->Code = 200;
                Res->Message = "OK";
            });
            p->PATH("/uploadfiles", [](shared_ptr<Request> Req,shared_ptr<Response> Res) {
                Http::str2file("./"+Req->Filename,Req->Data);
                Res->WriteString(Req->Filename + " Upload Success!");
                Res->Code = 200;
                Res->Message = "OK";
                cout << Req->Filename << endl;
            });
            c->Ctx = p;
        };
        s->Data = [](Conn *c, const string& in) -> string {
            c->cnt++;
            cout<<c->port<< " "<<c->cnt <<" Times in called"<<endl;
            return c->Ctx->parse(in);
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
