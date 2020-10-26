#include <string>
#include <cstring>
using namespace std;




class Http{
public:
    Http();
    ~Http();
    string rawStr;
   Http& Parse(string);
   HttpStruct* data();
private:
    void parseHttp();
    HttpStruct* headerAndBody;
};

struct HttpStruct{
    Request Req;
    Response Res;
};
struct Request{

};
struct Response{

};





























string str = R"delimiter(
    POST / HTTP/1.1
Host: localhost:3000
Connection: keep-alive
Content-Length: 184
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
Origin: http://127.0.0.1:5500
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryf1C8sbvyAvdrAvhD
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.105 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Sec-Fetch-Site: cross-site
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: iframe
Referer: http://127.0.0.1:5500/server/html.html
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7

------WebKitFormBoundaryf1C8sbvyAvdrAvhD
Content-Disposition: form-data; name="upload"; filename="a.txt"
Content-Type: text/plain

abc
------WebKitFormBoundaryf1C8sbvyAvdrAvhD--

)delimiter";