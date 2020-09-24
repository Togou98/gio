package main

import (
	"./gio"
	//"fmt"
	//"io/ioutil"
	"flag"
)
var t *int
func init(){
	t = flag.Int("t",1,"Use -t <int> to Set WorkRoutineNum\n")
	flag.Parse()
}
func main() {
	test()
}

func test() {
	gio.SetFdLimit(0xffff)
	srv := new(gio.Server)
	srv.LoopCycle = 10
	srv.RoutineNum = *t
	srv.PreContext = func(c gio.Conn) {
		c.SetContext(gio.NewHttpProcessor())
	}
	srv.Data = func(c gio.Conn, in []byte) (out []byte, i interface{}) {
		if p,ok := c.Context().(*gio.HttpProccesor);ok{
			rsp := p.ProcessData(in)
			if rsp != nil{
				rsp.Body = []byte("Hello Gio")
				out = rsp.Bytes()
				//c.ShutdownFd(true)
			}
		}
		return
	}
	gio.AddServant(srv, "tcp://0.0.0.0:8080")
}
