package main

import (
	"./gio"
	"flag"
)

var t *int

func init() {
	t = flag.Int("t", 1, "Use -t <int> to Set WorkRoutineNum\n")
	flag.Parse()
}
func main() {
	test()
}

func test() {
	gio.SetFdLimit(0xffff)
	srv := new(gio.Server)
	srv.LoopCycle = 20
	srv.RoutineNum = 0
	srv.PreContext = func(c gio.Conn) {
		c.SetContext(gio.NewHttpProcessor())
	}
	srv.Data = func(c gio.Conn, in []byte) (out []byte, i interface{}) {
		if p, ok := c.Context().(*gio.HttpProccesor); ok {
			p.ParseData(in)
			if res := p.GenResponse();res != nil{
				res.Body.WriteString(welcome)
				out = res.Bytes()
			}
		}
		return
	}
	gio.AddServant(srv, "0.0.0.0:8080")
}

const welcome = `
<h1><b><center>Hello Gio</center></b></h1>
`
