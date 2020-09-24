package gio

import (
	"fmt"
	"net"
	"os"
	"syscall"
	"time"
)

func init(){

}
type Server struct {
	Data func(c Conn,in []byte)(out []byte,i interface{})
	PreContext func(c Conn)
	Init func()
	RoutineNum int
	LoopCycle int
}
type Conn interface {
	Context() interface{}
	RemoteAddr() net.Addr
	SetContext(i interface{})
	Wake()
	ShutdownFd(half bool)
}
func AddServant(srv *Server,addrs ...string)error{
	var lns []*listener
	defer func(){
		for _,ln := range lns{
			ln.close()
		}
	}()
	for _,addr := range addrs{
		var err error
		var ln listener
		ln.network,ln.addr = parseAddr(addr)
		ln.ln ,err = net.Listen(ln.network,ln.addr)
		if err != nil{
			return err
		}
		ln.lnaddr = ln.ln.Addr()
		if err := ln.setReusePortAndNonBlock(); err != nil{
			return err
		}
		lns = append(lns,&ln)
	}
	fmt.Printf("Server ListenAt %v\n",addrs)
	return serve(srv,lns)
}
func SetFdLimit(n int){
	l := syscall.Rlimit{}
	l.Cur = uint64(n)
	syscall.Setrlimit(syscall.RLIMIT_NOFILE, &l)
	err := syscall.Getrlimit(syscall.RLIMIT_NOFILE,&l)
	if err != nil{
		fmt.Println(err)
		os.Exit(-1)
	}
	fmt.Printf("CurFdLimit: %d | MaxFdLimit: %d\n",l.Cur,l.Max)
}
const(
	consoleCyan = "\033[1;36m<%s>\033[0m%s\n"
	//debugCyan  ="\\033[1;36m[DEBUG]\\033[0m"
	//infoGreen = "\\033[1;36m[DEBUG]\\033[0m"
	//warnPink = "\\033[1;35m[WARN]\\033[0m"
	//errorRed = "\\033[0;31m[ERROR]\\033[0m"
)
func now2str(f string)string{
	return time.Now().Format(f)
}
func ConSole(str string){

		fmt.Printf(consoleCyan,now2str("03:04:06"),str)
}
