package gio

import (
	"bytes"
	"strconv"
	"time"
)

const BasicGET = "GET / HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n"
const BasicPost = "POST / HTTP/1.1\r\nHost: 192.168.1.1\r\nContent-Length: 7\r\n\r\nNOTHING"
const COMMONGET = `GET / HTTP/1.1
Host: start.firefoxchina.cn
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:81.0) Gecko/20100101 Firefox/81.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: zh-CN,en-US;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Cookie: Hm_lvt_dd4738b5fb302cb062ef19107df5d2e4=1600491670,1600593966,1600777335,1600958927; uid=rBADnV9jZlQXGgP8HUIyAg==; Hm_lpvt_dd4738b5fb302cb062ef19107df5d2e4=1600958937
Upgrade-Insecure-Requests: 1
Pragma: no-cache
Cache-Control: no-cache`
const CURLPOST = `POST / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:82.0) Gecko/20100101 Firefox/82.0
Accept: */*
Accept-Language: zh-CN,en-US;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Content-Length: 8
Origin: https://getman.cn
Connection: keep-alive
Pragma: no-cache
Cache-Control: no-cache

sdagsgsa`
const VER = "HTTP/1.1"

var DELIMByte = [2]byte{'\r', '\n'}
var SPACEByte = [1]byte{' '}
var COLON = [1]byte{':'}
var HTTPEND = [4]byte{'\r', '\n', '\r', '\n'}

type Method int

const (
	GET Method = iota
	POST
)

func (m Method) String() string {
	return []string{"GET", "POST"}[m]
}

type Request struct {
	Data           []byte
	Meth           Method
	Path           string
	Ver            string
	Host           string
	Body           []byte
	Content_Length int
	Keep_Alive     bool
	Header         map[string]string
}
type Response struct {
	ReqDatas       []byte
	StatusMsg      string
	Code           int
	Ver            string
	Content_Length int
	Content_Type   string
	Path           string
	Header         map[string]string
	Body           *bytes.Buffer
}

func NewRequest() *Request {
	return &Request{
		[]byte{},
		GET,
		"",
		VER,
		"",
		[]byte{},
		0,
		false,
		make(map[string]string),
	}
}
func NewResponse() *Response {
	return &Response{
		ReqDatas:       nil,
		StatusMsg:      "OK",
		Code:           200,
		Ver:            VER,
		Content_Length: 0,
		Content_Type:   "",
		Path:           "",
		Header:         make(map[string]string),
		Body:           bytes.NewBuffer([]byte{}),
	}
}

type HttpProccesor struct {
	DataStream []byte
}

func NewHttpProcessor() *HttpProccesor {
	return &HttpProccesor{[]byte{}}
}

func (h *HttpProccesor) ParseData(data []byte) {
	h.DataStream = append(h.DataStream, data...)
}
func (h *HttpProccesor) GenResponse() *Response {
	if bytes.HasPrefix(h.DataStream, []byte(GET.String())) || bytes.HasPrefix(h.DataStream, []byte(POST.String())) && bytes.Contains(h.DataStream, HTTPEND[:]) {
		req := parseRequest(h.DataStream)
		if req != nil {
			res := NewResponse()
			res.ReqDatas = req.Body
			res.Path = req.Path
			res.Header["Date"] = time.Now().Format(time.RFC822)
			if req.Keep_Alive {
				res.Header["Connection"] = "keep-alive"
			}
			return res
		}
	}
	return nil
}
func parseRequest(data []byte) *Request {
	req := NewRequest()
	firstLineIdx := bytes.Index(data, DELIMByte[:])
	endIdx := bytes.Index(data, HTTPEND[:])
	headerBuf := data[firstLineIdx+2 : endIdx]
	parseFistLine(data[:firstLineIdx], req)
	headers := bytes.Split(headerBuf, DELIMByte[:])
	parseHeader(headers, req)
	if req.Content_Length > 0 {
		req.Body = data[endIdx+4 : endIdx+4+req.Content_Length]
		data = data[endIdx+4+req.Content_Length:]
	}
	return req
}

func (r *Response) Bytes() []byte {
	out := bytes.NewBuffer([]byte{})
	out.WriteString(r.Ver)
	out.WriteByte(' ')
	out.WriteString(strconv.Itoa(r.Code))
	out.WriteByte(' ')
	out.WriteString(r.StatusMsg)
	out.Write(DELIMByte[:])
	if r.Content_Type != "" {
		out.WriteString(r.Content_Type)
		out.Write(DELIMByte[:])
	}
	dataln := r.Body.Len()
	out.WriteString("Content-Length: " + strconv.Itoa(dataln))
	out.Write(DELIMByte[:])
	for k, v := range r.Header {
		out.WriteString(k + ": " + v)
		out.Write(DELIMByte[:])
	}
	out.Write(DELIMByte[:])
	out.Write(r.Body.Bytes())
	return out.Bytes()
}

func parseFistLine(b []byte, r *Request) {
	fLine := bytes.SplitN(b, SPACEByte[:], 3)
	if string(fLine[0]) == GET.String() {
		r.Meth = GET
	} else if string(fLine[0]) == POST.String() {
		r.Meth = POST
	}
	r.Path = string(fLine[1])
	//Ver
	return
}
func parseHeader(b [][]byte, r *Request) {
	for _, v := range b {
		if len(v) <= 0 {
			break
		}
		kv := bytes.SplitN(v, COLON[:], 2)
		kv[1] = bytes.TrimLeft(kv[1], " ")
		key := string(kv[0])
		value := string(kv[1])
		r.Header[key] = value
	}
	r.Host = r.Header["Host"]
	if ctxLn, ok := r.Header["Content-Length"]; ok {
		ln, err := strconv.Atoi(ctxLn)
		if err != nil {
			return
		}
		r.Content_Length = ln
	}
	if alive, ok := r.Header["Connection"]; ok {
		if ok {
			if alive == "keep-alive" || alive == "Keep-Alive" {
				r.Keep_Alive = true
			}
		}
	}
}
