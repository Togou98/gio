#include "httpParser.h"
#include <algorithm>
#include <iostream>
#include <fstream>

string _RNRN = "\r\n\r\n";
string _RN = "\r\n";
string SPACE = " ";
string COLON = ":";
string FROMDATASEPRATER = "--";
string BOUNDARY = "boundary";
const int _RNRNLEN = 4;
const int _RNLEN = 2;
Http::Http()
{
}
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
    if(!Req){
        if (headBodyGap >= rawStr.length() || headBodyGap <= 0) return *this;
          string headText = rawStr.substr(0, headBodyGap + _RNLEN);
        string bodyText = rawStr.substr(headBodyGap + _RNRNLEN);
         if(!Req) Req = new Request(headText, bodyText);
    }else{
        Req->rawBody.append(rawStr);
    }
    Req->parseHead();
    if(Req->Done){
    Req->parseBody();
    Res = new Response(Req);
    }
    return *this;
}
void Request::parseHead()
{
    parseLine();
}
void Request::parseLine()
{
    int pos = rawHeader.find(_RN);
    if (pos != rawHeader.npos && Method.empty() && Path.empty())
    {
        string f = rawHeader.substr(0, pos);
        Method = f.substr(0, f.find(SPACE));
        int secondSpacePos = f.substr(Method.size() + 1, f.size()).find(SPACE);
        Path = f.substr(Method.size() + 1, secondSpacePos);
        Version = f.substr(secondSpacePos + Path.size(), f.size());
    }
    parseOtherLine(rawHeader.substr(pos + _RNLEN, rawHeader.size()));
}

void Request::parseOtherLine(string s)
{
    try{

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
    }catch(exception& e){
        cerr<< e.what()<<endl;
        return;
    }

}
void Request::parseKeyValue(string kv)
{   
    try{
    int colonPos = kv.find(COLON);
    if (colonPos == kv.npos)
        return;
    string k = kv.substr(0, colonPos);
    string v = kv.substr(colonPos + 2, kv.size()); // +2  + 1
    Header[k] = v;
    }catch(exception& e){
        cerr<<e.what()<<endl;
    }
}
void Request::specialFilter()
{
    auto itCL = Header.find("Content-Length");
    if (itCL != Header.end())
    {
        Content_Length = atoi(itCL->second.c_str());
        if(rawBody.length() >= Content_Length){
            Done = true;
        }else{
            Done =false;
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
    if(rawBody.empty()) return;
    if(!Done) return;
    auto CttypeIt = Header.find("Content-Type");
    if (CttypeIt == Header.end())
        return;
    int boundPos = CttypeIt->second.find(BOUNDARY);
    if (boundPos < 0 || boundPos > CttypeIt->second.size())
        return; //error???
    string boundary = CttypeIt->second.substr(boundPos, CttypeIt->second.length());
    boundary = boundary.substr(BOUNDARY.size() + 1, boundary.size());
    parseTrueBody(boundary + FROMDATASEPRATER);
}
void Request::parseTrueBody(string boundary)
{
    string divBound = rawBody.substr(boundary.size() + _RNLEN, rawBody.length() - (boundary.length() + _RNRNLEN));
    divBound = divBound.substr(0, divBound.length() - (boundary.length() + _RNRNLEN));
    string fname = "filename";
    int Fpos = divBound.find(fname);
    divBound = divBound.substr(Fpos, divBound.length());
    Data = divBound.substr(divBound.find(_RNRN) + _RNRNLEN, divBound.length());
    divBound = divBound.substr(fname.length() + 2, divBound.find("Content-Type"));
    Filename = divBound.substr(0, divBound.find("\"\r\n"));
}
Request::Request(string h, string b) : rawHeader(h), rawBody(b) {}
Response::Response(Request *Req)
{
    if (Req->Keep_Alive)
    {
        Keep_Alive = true;
        Header["Connection"] = "keep-alive";
    }
}
string Http::doResponse()
{
    if(Req == nullptr) return "";
    if(!Req->Done) return "";
    string Ret;
    auto HandleFuncIt = phfMap.find(Req->Path);
    if (HandleFuncIt == phfMap.end())
    {
        Ret = Res->Page404(Req->Path);
    }else{
        HandleFuncIt->second(Req, Res);
        Ret = Res->String();
    }
    
    rawStr.clear();
    delete Req;
    Req = nullptr;
    delete Res;
    Res = nullptr;
    return Ret;
}
void Http::PATH(string path, void (*handle)(const Request *, Response *))
{
    phfMap[path] = handle;
}
void Response::WriteString(string content)
{
    PayLoad.append(content);
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
string Response::Page404(string path)
{
    Code = 404;
    Message = "ERR";
    WriteString("404 Page Not found At "+ path );
    return String();
}
string Response::Page500()
{
    return "";
}
string Http::pages = "";
void Http::str2file(const string& path,const string& str){
    std::ofstream out(path,std::ios_base::out | std::ios_base::trunc);
    stringstream ss;
    ss << str;
    out << ss.str();
    return out.close(); 
}
///////////////////////////////for test
// string str = "GET /post HTTP/1.1\r\nHost: 10000000\r\nConnection: keep-alive\r\nContent-Length: 12\r\n\r\nTEMPLATEDATA";
// string str2 = "POST /get HTTP/1.1\r\n\r\n";
// string testupload = "POST /upload HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nContent-Length: 210\r\nContent-Type: multipart/form-data; boundary=----WebKitFormBoundaryFsz9N2QtCXfoY3cq\r\n\r\n------WebKitFormBoundaryFsz9N2QtCXfoY3cq\r\nContent-Disposition: form-data; name=\"upload\"; filename=\"Helloworld\"\r\nContent-Type: application/octet-stream\r\n\r\nHelloworld\r\n------WebKitFormBoundaryFsz9N2QtCXfoY3cq--\r\n";
