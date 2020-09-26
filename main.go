package main

import (
	"./gio"
	"flag"
	"fmt"
)

var t int

func init() {
	//t = flag.Int("t", 1, "Use -t <int> to Set WorkRoutineNum\n")
	flag.Parse()
}
func main() {
	test()
}

func test() {
	//buf ,_:= ioutil.ReadFile("plmmzj.mp4")
	//fmt.Println("File Size ",len(buf))
	gio.SetFdLimit(0xffff)
	srv := new(gio.Server)
	srv.LoopCycle = 10
	srv.RoutineNum = t
	srv.OnConnect = func(c gio.Conn) {
		httpHandle := gio.NewHttpProcessor()
		httpHandle.HandleFunc("/", func(r *gio.Response) {
			r.Body.WriteString(welcome)
			r.Content_Type = "text/plain; charset=UTF-8"
		})
		httpHandle.HandleFunc("/post",func(r *gio.Response){
			r.Body.WriteString(welcome)
			fmt.Println(r.ReqDatas)
		})
		c.SetContext(httpHandle)
	}
	srv.Data = func(c gio.Conn, in []byte) (out []byte) {
		if p, ok := c.Context().(*gio.HttpProccesor); ok {
			if res := p.GenResponse(in);res != nil{
				out = res.Bytes()
			}
		}
		return
	}
	gio.Run(srv, "0.0.0.0:8080")
}

const welcome = `<h1><b><center>Hello Gio</center></b></h1>`
