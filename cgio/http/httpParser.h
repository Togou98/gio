#include <string>
#include <cstring>
#include <map>
#include <functional>
#include <sstream>
#include <unistd.h>
using namespace std;
class Response;
class Request;
class HttpStruct;

class Http
{
public:
    Http();
    ~Http();
    string rawStr;
    Http &Parse(string);
    Http& parseHttp();
    Request *Req;
    Response *Res;
    void GET();
    void POST();
    map<string,void(*)(const Request*,Response*)> phfMap;
    string doResponse();
private:
};

struct Request
{
    Request(string, string);
    void parseLine();
    void parseOtherLine(string);
    void parseHead();
    void parseBody();
    void parseKeyValue(string);
    void specialFilter();
    string rawHeader;
    string rawBody;
    ///////////////////
    string Method;
    string Path;
    string Version;
    map<string, string> Header;
    bool Keep_Alive;
    int Content_Length = 0;
    bool Done;
    string Host;
};
struct Response
{   Response();
    Response(Request *);
    string Page404();
    string Page500();
    void WriteString(string);
    string String();
    map<string, string> Header;
    int ContentLength()const;
    bool Keep_Alive;
    int Code;
    string Message;
private:
    string PayLoad;

};
