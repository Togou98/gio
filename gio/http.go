package gio

import (
	"bytes"
	"strconv"
	"time"
)

const VER = "HTTP/1.1"
var rn = [2]byte{'\r','\n'}
var rn_rn = [4]byte{'\r','\n','\r','\n'}
const RNRN = "\r\n\r\n"
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
	ReqMeth    Method
	StatusMsg      string
	Code           int
	Ver            string
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
		Content_Type:   "",
		Path:           "",
		Header:         make(map[string]string),
		Body:           bytes.NewBuffer([]byte{}),
	}
}

type HttpProccesor struct {
	netStream []byte
	handle    map [string]func(r *Response)
}
func(h *HttpProccesor)HandleFunc(path string,fn func(r *Response)){
	h.handle[path] = fn
}

func NewHttpProcessor() *HttpProccesor {
	return &HttpProccesor{[]byte{},make(map[string]func(r *Response))}
}


func (h *HttpProccesor) GenResponse(data []byte) *Response {
	h.netStream = append(h.netStream,data...)
	if bytes.HasPrefix(h.netStream, []byte(GET.String())) || bytes.HasPrefix(h.netStream, []byte(POST.String())) && bytes.Contains(h.netStream, HTTPEND[:]) {
		res :=  h.ParseRequest().genarateResponse()
		if fn,ok := h.handle[res.Path];ok && res.Code != 500{
			fn(res)
			return res
		}else{
			return Response404(res.Path)
		}
	}
	return nil
}
func(h *HttpProccesor)ParseRequest()*Request{
	req := NewRequest()
	firstLineIdx := 	bytes.Index(h.netStream,rn[:])
	endIdx := bytes.Index(h.netStream, rn_rn[:])
	headerBuf := h.netStream[firstLineIdx+2 : endIdx]
	parseFistLine(h.netStream[:firstLineIdx], req)
	headers := bytes.Split(headerBuf, rn[:])
	parseHeader(headers, req)
	if req.Content_Length > 0 {
		req.Body = h.netStream[endIdx+4 : endIdx+4+req.Content_Length]
	}
	h.netStream = h.netStream[:0]
	return req
}

func Response404(path string)*Response{
	response := NewResponse()
	response.Body.WriteString("<h1><center><b>404 Pages Not Found</b></center></h1><br><center><h3>Gio/0.0.1</h3></center>")
	response.Code = 404
	response.StatusMsg =  "ERR"
	//don't need set path
	return response
}
func Response500(path string)*Response{
	respone := NewResponse()
	respone.Body.WriteString("<h1><center><b>Server Internel Error</b></center></h1>")
	respone.Code = 500
	respone.StatusMsg = "ERR"
	respone.Path = path
	return respone
}
func(request *Request) genarateResponse()*Response{
	if request != nil {
		res := NewResponse()
		res.ReqDatas = request.Body
		res.Path = request.Path
		res.Header["Date"] = time.Now().Format(time.RFC822)
		if request.Keep_Alive {
			res.Header["Connection"] = "keep-alive"
		}
		res.Header["Server"] = "Gio/0.0.1"
		return res
	}else {return Response500(request.Path)}
}


func (r *Response) Bytes() []byte {
	out := bytes.NewBuffer([]byte{})
	out.WriteString(r.Ver)
	out.WriteByte(' ')
	out.WriteString(strconv.Itoa(r.Code))
	out.WriteByte(' ')
	out.WriteString(r.StatusMsg)
	out.Write(rn[:])
	if r.Content_Type != "" {
		out.WriteString(r.Content_Type)
		out.Write(rn[:])
	}
	dataln := r.Body.Len()
	out.WriteString("Content-Length: " + strconv.Itoa(dataln))
	out.Write(rn[:])
	for k, v := range r.Header {
		out.WriteString(k + ": " + v)
		out.Write(rn[:])
	}
	out.Write(rn[:])
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