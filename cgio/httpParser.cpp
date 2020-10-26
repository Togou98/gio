#include "httpParser.h"
#include <algorithm>

 string _RNRN = "\r\n\r\n";
 const int _RNRNLEN =  4;
Http::Http(){}
Http::~Http(){}


Http& Http::Parse(string data){
    this->rawStr.append(data);
    return *this;
}

void Http::parseHttp() { 
    if(this->rawStr.empty()) return;
    size_t headBodyGap = rawStr.find(_RNRN,0);
    if(headBodyGap  < 0 ) return;
    string headText = rawStr.substr(0,headBodyGap);
    string bodyText = rawStr.substr(headBodyGap + _RNRNLEN);

}




///////////////////////////////for test




int main(){
    Http h;
    h.Parse(str);
    
}