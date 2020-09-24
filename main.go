package main

import (
	"./gio"
	"bytes"
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
	srv.Data = func(c gio.Conn, in []byte) (out []byte, i interface{}) {
		if bytes.Contains(in, []byte(`\r\n\r\n`)) {
			out = []byte("HEllo MY GIo")
			c.ShutdownFd(true)
		}
		i = nil
		return
	}
	gio.AddServant(srv, "tcp://0.0.0.0:8080")
}
