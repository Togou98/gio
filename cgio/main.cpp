#include "server.h"
#include "poll.h"
#include "socket.h"
#include "httpParser.h"
#include <bits/stdc++.h>
#include "stdlib.h"

void toserver()
{
    try
    {   
    int cnt =0;
        static string IPORT =  "127.0.0.1:8080";
        Server *s = new Server(IPORT);
        s->loopInterval = 20;  //wait 间隔
        s->threadNum = 1;   //线程数 不配就为CPU核数
        cout<<"Http Server ListenAnd Serving @"<<IPORT<<" Use ["<<s->threadNum<<"] Threads With "<<s->loopInterval <<" ms"<<endl;
        Http::init();
        s->PreSet = [](Conn *c) {
            Http* p = new Http;
            p->PATH("/get", [](const Request* Req,Response* Res) {
                Res->WriteString("For Test Server");
                Res->Code = 200;
                Res->Message = "OK";
            });
            p->PATH("/upload", [](const Request* Req,Response* Res) {
                Res->WriteString(Http::fMap["upload.html"]);
                Res->Code = 200;
                Res->Message = "OK";
            });
            p->PATH("/uploadfiles", [](const Request* Req,Response* Res) {
                Http::str2file("./"+Req->Filename,Req->Data);
                string UPLOADOKBEG = "<p><a href=\"" + IPORT + "/\"";
                string UPLOADOKEND = "Upload Success!</a></p>";
                Res->WriteString(UPLOADOKBEG + Req->Filename +"\">" + Req->Filename + UPLOADOKEND );
                Res->Code = 200;
                Res->Message = "OK";
                Http::fMap[Req->Filename] = Http::file2str(Req->Filename);
                cout << Req->Filename << endl;
            });
            c->Ctx = p;
        };
        s->Data = [](Conn *c, const string& in) -> string {
            c->cnt++;
            cout<<c->port<< " "<<c->cnt <<" Times in called"<<endl;
            string out = c->Ctx->parse(in);
            int t = 0x10;
            return out;
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
