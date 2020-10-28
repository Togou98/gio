#include "httpParser.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "poll.h"
string _RNRN = "\r\n\r\n";
string _RN = "\r\n";
string SPACE = " ";
string COLON = ":";
string FROMDATASEPRATER = "--";
string BOUNDARY = "boundary";
string HTTP_1_1 = "HTTP/1.1";
const string EMPTYSTRING = "";
const string SERVER = "GIO/0.0.1";
const int _RNRNLEN = 4;
const int _RNLEN = 2;

Http::Http(){
}
Http::~Http()
{   
    //  CPPGC(Req);
    //  CPPGC(Res);
    // cout<<"HTTP Des Called"<<endl;
}

string Http::parse(const string& data)
{
    if (data.empty())
        return EMPTYSTRING;
    if(rawStr.empty() && Req){
        Thisfree();
    }
    if (!Req)
    {   
        rawStr += data;
        size_t headBodyGap = rawStr.find(_RNRN);
        if (headBodyGap > rawStr.length() || headBodyGap < 0)
            return EMPTYSTRING;
        string headText = rawStr.substr(0, headBodyGap);
        string bodyText = rawStr.substr(headBodyGap + _RNRNLEN, rawStr.length());
        // Req = new Request(headText, bodyText);
        Req = make_shared<Request>(headText,bodyText);
        Req->parseHead();
    }
    else
    {
        cout<<Req->rawBody<<endl;
        rawStr += data;
        Req->rawBody.append(data);
    }
    if (Req->Done)
    {
        if (Req->rawBody.length() > 0 && Req->Content_Length > 0)
        {
            Req->parseBody();
        }
        // Res = new Response(Req);
        Res = make_shared<Response>(Req);
        return doResponse();
    }else return EMPTYSTRING;
}
void Request::parseHead()
{
    int pos = rawHeader.find(_RN);
    if (pos != rawHeader.npos && Method.empty() && Path.empty())
    {
        string f = rawHeader.substr(0, pos);
        Method = f.substr(0, f.find(SPACE));
        string next = f.substr(Method.length() + 1, f.length());
        Path = next.substr(0, next.find(SPACE));
        Version = HTTP_1_1;
    }
    parseOtherLine(rawHeader.substr(pos + _RNLEN, rawHeader.size()));
}

void Request::parseOtherLine(string s)
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
    try
    {
        int colonPos = kv.find(COLON);
        if (colonPos == kv.npos)
            return;
        string k = kv.substr(0, colonPos);
        string v = kv.substr(colonPos + 2, kv.size()); // +2  + 1
        Header[k] = v;
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
    }
}
void Request::specialFilter()
{
    auto itCL = Header.find("Content-Length");
    if (!rawBody.empty()  ||itCL != Header.end())
    {
        Content_Length = atoi(itCL->second.c_str());
        if (rawBody.length() >= Content_Length)
        {
            Done = true;
        }
        else
        {
            Done = false;
        }
    } else Done = true;
    auto itConn = Header.find("Connection");
    if (itConn != Header.end())
    {
        if (itConn->second == "keep-alive" || itConn->second == "Keep-Alive"){
            Keep_Alive = true;
        }
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
    if (!Done)
        return;
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
Response::Response(Request *Req) : Keep_Alive(Req->Keep_Alive)
{
    
    Header["Server"] = SERVER;
}
string Http::doResponse()
{
    if (!Req || !Req->Done || !Res)
    {
        return EMPTYSTRING;
    }
    string Ret;
    auto HandleFuncIt = phfMap.find(Req->Path);
    if (HandleFuncIt == phfMap.end())
    {
        Ret = Res->Page404(Req->Path);
    }
    else
    {
        HandleFuncIt->second(Req, Res);
        Ret = Res->String();
    }
   Thisfree();
    return Ret;
}
void Http::Thisfree(){
    rawStr.clear();
    // if(Req) delete Req;
    // if(Res) delete Res;
    // Req = nullptr;
    // Res = nullptr;
}
Request::~Request()
{
}
Response::~Response()
{
}

void Http::PATH(string path, void (*handle)(shared_ptr<Request> Req,shared_ptr<Response> Res))
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
    if (Keep_Alive)
        Header["Connection"] = "keep-alive";
    else Header["Connection"] = "close";
    for (const auto &head : Header)
    {
        oss << head.first << ": " << head.second << _RN;
    }
    oss << "Content-Length"
        << ": " << ContentLength() << _RNRN;
    oss << PayLoad;
    return oss.str();
}
string Response::Page404(string path)
{
    Code = 404;
    Message = "ERR";
    WriteString("404 Page Not found At " + path);
    return String();
}
string Response::Page500()
{
    return "";
}
Request::Request(string h, string b) : rawHeader(h), rawBody(b) {}
string Parser::parse(const string&){
    return EMPTYSTRING;
}
Parser::~Parser(){}
string Http::pages = "";
void Http::str2file(const string &path, const string &str)
{
    std::ofstream out(path, std::ios_base::out | std::ios_base::trunc);
    stringstream ss;
    ss << str;
    out << ss.str();
    return out.close();
}
