#include "server.h"
#include "poll.h"
#include "socket.h"
#include "httpParser.h"
#include <bits/stdc++.h>
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
    {   string IPORT =  "127.0.0.1:8080";
        Server *s = new Server(IPORT);
        s->loopInterval = 1000;  //wait 间隔
        s->threadNum = 1;   //线程数 不配就为CPU核数
        cout<<"Http Server ListenAnd Serving @"<<IPORT<<"Use ["<<s->threadNum<<"]Threads With "<<s->loopInterval <<"ms"<<endl;
        httpInit();
        s->PreSet = [](Conn *c) {
            auto h = new Http;
            h->PATH("/get", [](const Request *Req, Response *Res) {
                Res->WriteString("For Test Server");
                Res->Code = 200;
                Res->Message = "OK";
                return;
            });
            h->PATH("/upload", [](const Request *Req, Response *Res) {
                Res->WriteString(Http::pages);
                Res->Code = 200;
                Res->Message = "OK";
            });
            h->PATH("/uploadfiles", [](const Request *Req, Response *Res) {
                Http::str2file("./"+Req->Filename,Req->Data);
                Res->WriteString(Req->Filename + " Upload Success!");
                Res->Code = 200;
                Res->Message = "OK";
                cout << Req->Filename << endl;
            });
            c->Ctx = h;
        };
        s->Data = [](Conn *c, string in) -> string {
            string out = ((Http *)(c->Ctx))->Parse(in).parseHttp().doResponse();
            if(!out.empty()) return out;
            else return "";
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
