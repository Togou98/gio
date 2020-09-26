package gio

import (
	"fmt"
	"net"
	"time"
)

const DEFAULTLOOPCYCLE = 20
const DEFAULTBASEEVENTSSIZE = 0xF

func init() {

}

type Server struct {
	Data       func(c Conn, in []byte) (out []byte, i interface{})
	OnConnect  func(c Conn)
	RoutineNum int
	LoopCycle  int
}
type Conn interface {
	Context() interface{}
	RemoteAddr() net.Addr
	SetContext(i interface{})
	Wake()
	ShutdownFd(half bool)
}

func AddServant(srv *Server, addrs ...string) error {
	//var lns []*listener
	var lns []*listener
	defer func() {
		for _, ln := range lns {
			ln.Close()
		}
	}()
	for _, addr := range addrs {

		ln, err := buildListener(addr)
		if err != nil {
			panic(err)
		}
		setNonblock(ln.fd)
		lns = append(lns, ln)
	}
	fmt.Printf("Server ListenAt %v\n", addrs)
	return serve(srv, lns)
}

const (
	consoleCyan = "[GIO]\033[1;36m<%s>\033[0m%s\n"
	//debugCyan  ="\\033[1;36m[DEBUG]\\033[0m"
	//infoGreen = "\\033[1;36m[DEBUG]\\033[0m"
	//warnPink = "\\033[1;35m[WARN]\\033[0m"
	//errorRed = "\\033[0;31m[ERROR]\\033[0m"
)

func now2str(f string) string {
	return time.Now().Format(f)
}
func ConSole(str string) {
	fmt.Printf(consoleCyan, now2str("03:04:06"), str)
}
