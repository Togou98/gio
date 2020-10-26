#include "httpParser.h"
#include <algorithm>
#include <iostream>
string _RNRN = "\r\n\r\n";
string _RN = "\r\n";
string SPACE = " ";
const int _RNRNLEN = 4;
const int _RNLEN = 2;
Http::Http() {}
Http::~Http() {}

Http &Http::Parse(string data)
{
    this->rawStr.append(data);
    return *this;
}

void Http::parseHttp()
{
    cout << rawStr << endl;
    if (this->rawStr.empty())
        return;
    size_t headBodyGap = rawStr.find(_RNRN);
    if (headBodyGap >= rawStr.npos)
        return;
    string headText = rawStr.substr(0, headBodyGap);
    string bodyText = rawStr.substr(headBodyGap + _RNRNLEN);
    Req = new Request(headText, bodyText);
    Req->parseHead();
    Req->parseBody();
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
        cout << "First Line :" << f << endl;
        Method = f.substr(0, f.find(SPACE));
        int secondSpacePos = f.substr(Method.size() + 1, f.size()).find(SPACE);
        Path = f.substr(Method.size() + 1, secondSpacePos);
        Version = f.substr(secondSpacePos + Path.size(), f.size());
    }
    parseOtherLine(rawHeader.substr(pos + _RNLEN, rawHeader.size()));
}
void Request::parseOtherLine(string s)
{
    int beg = 0;
    while (true)
    {
        int nextRnPos = s.find(_RN, beg);
        if (nextRnPos == s.npos)
            break;
        string thisline = s.substr(beg, nextRnPos);
        beg = nextRnPos;
    }
}
void Request::parseKeyValue(string kv){
    
}
void Request::parseBody()
{
    if (rawBody.empty())
        return;
}

Request::Request(string h, string b) : rawHeader(h), rawBody(b) {}

///////////////////////////////for test

int main()
{
    Http h;
    h.Parse(str).parseHttp();
}