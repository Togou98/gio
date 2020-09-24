package gio

import (
	"bytes"
	"fmt"
	"strconv"
	"strings"
	"time"
)

const DELIMTEROFHEADBODY = `\r\n\r\n`
const HTTPLINEEND = `\r\n`
const SPACE = ` `
const HTTP1_1  HTTPVERSION = iota
type HTTPVERSION  int

type Method int
const (
	GET Method = iota
	HEAD
	POST
)
func (h HTTPVERSION)String()string{
	return []string{`HTTP/1.1`}[h]
}
func(m Method)String()string{
	return []string{`GET`,`HEAD`,`POST`}[m]
}

const COLON = `:`
type Request struct{
	Method Method
	Path string
	Ver HTTPVERSION
	Host string
	Others map[string]string
	Cookies string
	Content_Length int
	KeepAlive bool
	Body *bytes.Buffer
}
type Response struct{
	StatusMessage string
	StatusCode int
	Ver HTTPVERSION
	Content_Length int64
	Others map[string]string
	Body *bytes.Buffer
}
func ParseRequest(data string)(r *Request){
	if strings.HasPrefix(data,GET.String()) ||strings.HasPrefix(data,HEAD.String())||strings.HasPrefix(data,POST.String()){

	}else {
		return
	}
	headAndBody := strings.SplitN(data,DELIMTEROFHEADBODY,2)

	if len(headAndBody) != 2 {
		return
	}
	r = parseHeader(headAndBody[0])
	if len(headAndBody[1]) > 0 && r.Method == POST{ //不是post不用解析数据了
		r.Body = bytes.NewBuffer([]byte{})
		r.Body.WriteString(headAndBody[1])
		r.Content_Length = r.Body.Len()
		r.Others["Content-Length"] = strconv.Itoa(r.Content_Length)
	}
	return
}
func parseHeader(header string)*Request { //head data
	lines := strings.Split(header,`\n`)
	r := parseFirstLine(lines[0])
	for i:=1;i<len(lines) && r != nil;i++{
		parseOtherLine(r,lines[i])
	}
	r.Host = r.Others["Host"]
	return r
}
func parseFirstLine(line string)*Request {
	threePart := strings.SplitN(line,SPACE,3)
	if len(threePart) != 3{
		return nil
	}
	h := new(Request)
	if threePart[0] == GET.String(){
		h.Method = GET
	}else if threePart[0] == HEAD.String(){
		h.Method = HEAD
	}else if threePart[0] == POST.String(){
		h.Method = POST
	}
	h.Path = threePart[1]
	if threePart[2] == HTTP1_1.String(){
		h.Ver = HTTP1_1
	}
	h.Others = make(map[string]string)
	return h
}
func parseOtherLine(r *Request,line string)*Request {
	line = strings.TrimSpace(line)
	filedAndValue := strings.SplitN(line,COLON,2)
	if len(filedAndValue) !=2 {
		return r
	}
	r.Others[filedAndValue[0]] = filedAndValue[1]
	return r
}

func BuildResponseFromRequest(r *Request)*Response {
	if r == nil{
		return nil
	}
	method := r.Method
	path := r.Path
	if method == GET {
		return easyGet(path)
	}else if method == POST{
		return new(Response)
	}
	return nil
}
func easyGet(path string)*Response{
	rsp := &Response{
		StatusMessage: `OK`,
		StatusCode:    200,
		Body:          bytes.NewBuffer([]byte{}),
		Others: make(map[string]string),
	}
	//rsp.Body.buf.WriteString(fmt.Sprintf("Get Path :%s",path))
	return rsp
}
func easyPost(path string)*Response{
	return nil
}
func(r *Response)string()*bytes.Buffer{
	data := []byte{}
	buf := bytes.NewBuffer(data)
	buf.WriteString(r.Ver.String())
	buf.WriteString(SPACE)
	buf.Write(strconv.AppendInt(buf.Bytes(),int64(r.StatusCode),10))
	buf.WriteString(SPACE)
	buf.WriteString(r.StatusMessage)
	buf.WriteString(HTTPLINEEND)
	buf.WriteString(fmt.Sprintf("%s:%s","Date",time.Now().Format(time.UnixDate)))
	buf.WriteString(HTTPLINEEND)
	return buf
}
func(r *Response)Write(b []byte)(int,error){
	if r == nil{
		return 0,fmt.Errorf("Empty Response Pointer")
	}
	n,err := r.Body.Write(b)
	r.Content_Length  += int64(n)
	return n,err
}
func(r *Response)String()string{
	if r == nil{
		return PAGE500
	}
	buf := r.string()
	if r.Body.Len() > 0{
		r.Others["Content-Length"] = strconv.Itoa(r.Body.Len())
		r.Content_Length = int64(r.Body.Len())
	}
	for k,v := range r.Others{
		line := k + COLON + v + HTTPLINEEND
		buf.WriteString(line)
	}
	buf.WriteString(HTTPLINEEND)
	buf.WriteString(r.Body.String())
	return buf.String()
}

const BasicChormeRequest =`GET /324dsafasd HTTP/1.1
Host: localhost:8080
Connection: close
Cache-Control: max-age=0
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Accept-Language: zh-CN,zh;q=0.9

`
const PAGE500 = `HTTP/1.1 404 ERR
Cache-Control: max-age=604800
Content-Length: 45

<h1><center>500 Internal Error!</center></h1>`
const PAGE404 = `HTTP/1.1 404 ERR
Cache-Control: max-age=604800
Content-Length: 44

<h1><center>404 PageNot Found!</center></h1>`
////////////////////////////////////////
//func main(){
//	test()
//}
//func test(){
//	r := ParseRequest(BasicChormeRequest)
//	rsp := BuildResponseFromRequest(r)
//	fmt.Println(rsp)
//
//}