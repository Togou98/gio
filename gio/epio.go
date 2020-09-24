package gio

import (
	"net"
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
	//ConSole(fmt.Sprintf("%d Address is On listening\n",len(lns)))
	return serve(srv,lns)
}