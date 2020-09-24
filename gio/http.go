package gio

import (
	"bytes"
	"strconv"
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
	Data       *bytes.Buffer
	IsComplete bool
	Meth       Method
	Path       string
	Ver        string
	Host       string
	Body       []byte
	Header     map[string]string
}
type Response struct {
	ReqDatas       *bytes.Buffer
	StatusMsg      string
	Code           int
	Ver            string
	Content_Length int
	Content_Type   string
	Path           string
	Header         map[string]string
	Body           []byte
}

func NewRequest() *Request {
	return &Request{
		bytes.NewBuffer([]byte{}),
		false,
		GET,
		"",
		VER,
		"",
		[]byte{},
		make(map[string]string),
	}
}
func NewResponse() *Response {
	return &Response{
		ReqDatas:       bytes.NewBuffer([]byte{}),
		StatusMsg:      "OK",
		Code:           200,
		Ver:            VER,
		Content_Length: 0,
		Content_Type:   "",
		Path:           "",
		Header:         make(map[string]string),
		Body:           []byte{},
	}
}

type HttpProccesor struct {
	Req *Request
}

func NewHttpProcessor() *HttpProccesor {
	return &HttpProccesor{Req: NewRequest()}
}
func (h *HttpProccesor) ProcessData(data []byte) *Response {
	h.Req.Data.Write(data)
	ln := h.Req.Data.Len()
	end := h.Req.Data.Bytes()[ln-4 : ln]
	if !bytes.Equal(end, HTTPEND[:]) {
		return nil
	} else {
		h.Req.IsComplete = true
		h.Req.Proc()
		res := NewResponse()
		res.Path = h.Req.Path
		if len(h.Req.Body) > 0 {
			res.ReqDatas.Write(h.Req.Body)
		}
		h.Req.Clear()
		return res
	}
	return nil
}
func (r *Request) Proc() bool {
	lines := bytes.Split(r.Data.Bytes(), DELIMByte[:])
	//ln := len(lines)
	//if lines[]
	parseFistLine(lines[0], r)
	if r.Meth == GET {
		lines = lines[1 : len(lines)-2]
		parseGETOtherLine(lines, r)

	} else if r.Meth == POST {

	}
	return r.IsComplete
}
func (r *Request) Clear() {
	r = NewRequest()
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
	dataln := len(r.Body)
	out.WriteString("Content-Length: " + strconv.Itoa(dataln))
	out.Write(DELIMByte[:])
	for k, v := range r.Header {
		out.WriteString(k + ": " + v)
		out.Write(DELIMByte[:])
	}
	out.Write(DELIMByte[:])
	out.Write(r.Body)
	return out.Bytes()
}

func parseFistLine(b []byte, r *Request) {
	fLine := bytes.SplitN(b, SPACEByte[:], 3)
	if string(fLine[0]) == GET.String() {
		r.Meth = GET
	}
	r.Path = string(fLine[1])
	//Ver
	return
}
func parseGETOtherLine(b [][]byte, r *Request) {
	for _, v := range b {
		if len(v) <= 0 {
			break
		}
		//fmt.Println(string(v))
		v = bytes.TrimSpace(v)
		kv := bytes.SplitN(v, COLON[:], 2)
		key := string(kv[0])
		value := string(kv[1])
		r.Header[key] = value
	}
	r.Host = r.Header["Host"]
}
func parsePOSTOtherLine(b [][]byte, r *Request) {

}
