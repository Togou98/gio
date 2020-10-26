#include "httpParser.h"
#include <algorithm>
#include <iostream>
string _RNRN = "\r\n\r\n";
string _RN = "\r\n";
string SPACE = " ";
string COLON = ":";
const int _RNRNLEN = 4;
const int _RNLEN = 2;
Http::Http() {}
Http::~Http()
{
    delete Req;
    delete Res;
}

Http &Http::Parse(string data)
{
    this->rawStr.append(data);
    return *this;
}

Http &Http::parseHttp()
{
    if (rawStr.empty())
        return *this;
    size_t headBodyGap = rawStr.find(_RNRN);
    if (headBodyGap >= rawStr.npos)
        return *this;
    string headText = rawStr.substr(0, headBodyGap + _RNLEN);
    string bodyText = rawStr.substr(headBodyGap + _RNRNLEN);
    Req = new Request(headText, bodyText);
    Req->parseHead();
    Req->parseBody();
    Res = new Response(Req);
    return *this;
}
void Request::parseHead()
{
    parseLine();
}
void Request::parseLine()
{
    int pos = rawHeader.find(_RN);
    if (pos != rawHeader.npos)
    {
        string f = rawHeader.substr(0, pos);
        Method = f.substr(0, f.find(SPACE));
        int secondSpacePos = f.substr(Method.size() + 1, f.size()).find(SPACE);
        Path = f.substr(Method.size() + 1, secondSpacePos);
        Version = f.substr(secondSpacePos + Path.size(), f.size());
    }
    parseOtherLine(rawHeader.substr(pos + _RNLEN, rawHeader.size()));
}



// string strlens = "GET /Post HTTP/1.1\r\nHost: 100.100.1\r\nContent-Length: 184\r\nConnections: keep-alive\r\nboundery: -----xxxxxx\r\n\r\nDATATEMPLATE";
void Request::parseOtherLine(string s)
// {   transform(s.begin(),s.end(),str.begin(),::tolower);
{
    while (true)
    {
        int next = s.find(_RN);
        if (next >= 0 && next < s.size())
        {
            parseKeyValue(s.substr(0, next));
            s = s.substr(next + 2, s.size());
        }
        else
        {
            break;
        }
    }
    specialFilter();
}
void Request::parseKeyValue(string kv)
{
    int colonPos = kv.find(COLON);
    if (colonPos == kv.npos)
        return;
    string k = kv.substr(0, colonPos);
    string v = kv.substr(colonPos + 2, kv.size()); // +2  + 1
    Header[k] = v;
}
void Request::specialFilter()
{
    auto itCL = Header.find("Content-Length");
    if (itCL != Header.end())
    {
        Content_Length = atoi(itCL->second.c_str());
        if (Content_Length >= rawBody.length())
        {
            Done = true;
        }
    }
    else
    {
        Done = true;
    }
    auto itConn = Header.find("Connection");
    if (itConn != Header.end())
    {
        if (itConn->second == "keep-alive")
            Keep_Alive = !Keep_Alive;
    }
    auto itHost = Header.find("Host");
    if (itHost != Header.end())
    {
        Host = itHost->second;
    }
}
void Request::parseBody()
{
    if (rawBody.empty())
        return;
}

Request::Request(string h, string b) : rawHeader(h), rawBody(b) {}
Response::Response(Request *Req)
{
    if (Req->Keep_Alive){
        Keep_Alive = true;
    Header["Connection"] = "keep-alive";
    }
}
string Http::doResponse()
{   
    if (Req && !Req->Done)
        return "";
    auto HandleFuncIt = phfMap.find(Req->Path);
    if (HandleFuncIt == phfMap.end())
    {
        return Res->Page404();
    }
    else
    {
        HandleFuncIt->second(Req, Res);
        rawStr.clear();
        delete Req;
        Req = nullptr;
        string Ret = Res->String();
        delete Res;
        Res = nullptr;
        return Ret;
    }
    // if (Req->Method == "GET")
    // {
    //     auto HandleFuncIt = getHandles.find(Req->Path);
    //     if (HandleFuncIt == getHandles.end())
    //         return Response::Page404();
    //     else
    //     {
    //         HandleFuncIt->second(Req, Res);
    //         return doResponse();
    //     }
    // }
    // else if (Req->Method == "POST")
    // {
    //     auto HandleFuncIt = postHandles.find(Req->Path);
    //     if (HandleFuncIt == postHandles.end())
    //         return Response::Page404();
    //     else
    //     {
    //         HandleFuncIt->second(Req, Res);
    //         return doResponse();
    //     }
    // }
}
void Response::WriteString(string content)
{
    PayLoad += content;
}
int Response::ContentLength() const
{
    return PayLoad.length();
}
string Response::String()
{
    ostringstream oss;
    oss << "HTTP/1.1 " << Code << " " << Message << _RN;
    for (const auto &head : Header)
    {
        oss << head.first << ": " << head.second << _RN;
    }
    oss << "Content-Length"
        << ": " << PayLoad.size() << _RNRN;
    oss << PayLoad;
    return oss.str();
}
string Response::Page404()
{
    return "";
}
string Response::Page500()
{
    return "";
}
///////////////////////////////for test

string str = "GET /post HTTP/1.1\r\nHost: 10000000\r\nConnection: keep-alive\r\nContent-Length: 12\r\n\r\nTEMPLATEDATA";
string str2 = "POST /get HTTP/1.1\r\n\r\n";
int main()
{
    Http h;
    h.phfMap["/get"] = [](const Request *Req, Response *Res) {
        Res->WriteString("Func get");
        Res->Code = 200;
        Res->Message = "OK";
        return;
    };
    for(int i =0; i < 1000000;i++){
    cout <<"["<<i<<"] "<< h.Parse(str2).parseHttp().doResponse()<< endl;
    }
}