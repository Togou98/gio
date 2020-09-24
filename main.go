package main

import (
	"./gio"
	"fmt"
	"io/ioutil"
)

func main() {
	test()
	//testBuf()
}

func test() {
	buf, _ := ioutil.ReadFile("./main")
	// buf = buf[:250000]
	fmt.Println("File size ", len(buf))
	srv := new(gio.Server)
	srv.LoopCycle = 10
	srv.RoutineNum = 1
	gio.SetLogLevel(gio.CONSOLEVEL)
	srv.PreContext = func(c gio.Conn) {
		c.SetContext(gio.NewHttpProcessor())
	}
	srv.Data = func(c gio.Conn, in []byte) (out []byte, i interface{}) {
		if p,ok := c.Context().(*gio.HttpProccesor);ok{
			rsp := p.ProcessData(in)
			if rsp != nil{
				rsp.Body = []byte("Hello Gio")
				out = rsp.Bytes()
				// c.ShutdownFd(true)
			}
		}
		return
	}
	gio.AddServant(srv, "tcp://0.0.0.0:8080")
}
