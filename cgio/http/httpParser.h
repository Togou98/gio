#include <string>
#include <cstring>
using namespace std;

class HttpStruct;
class Response;
class Request;

class Http
{
public:
    Http();
    ~Http();
    string rawStr;
    Http &Parse(string);
    Request *Req;
    Response *Res;
    void parseHttp();


private:
};

struct Request
{
    Request(string,string);
    void parseLine();
    void parseOtherLine(string);
    void parseHead();
    void parseBody();
    void parseKeyValue(string);
    string rawHeader;
    string rawBody;
///////////////////
    string Method;
    string Path;
    string Version;
};
struct Response
{
    Response(string);
};

string str = "GET /Post HTTP/1.1\r\nHost: 100.100.1\r\nContent-Length: 184\r\n\r\nDATATEMPLATE";