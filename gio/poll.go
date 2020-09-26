package gio

import (
	"fmt"
	"net"
	"sync/atomic"
	"syscall"

	reuseport "github.com/kavu/go_reuseport"
)
const DEFAULTLOOPCYCLE = 20
const DEFAULTBASEEVENTSSIZE = 0x400
//epoll 结构封装
type Action int
const R = syscall.EPOLLIN
const W = syscall.EPOLLOUT
const ET = 0x80000000 // -syscall.EPOLLET
const (
	NONE Action = iota
	READ
	WRITE
)
type Poll struct {
	epfd        int
	wfd         int
	evsize      int
	isListePoll bool
	closeCh     chan struct{}
}

//创建epoll 对象实例
func newPoll() *Poll {
	p := new(Poll)
	epfd, err := syscall.EpollCreate1(0)
	if err != nil {
		panic(err)
	}
	p.epfd = epfd
	r0, _, e0 := syscall.Syscall(syscall.SYS_EVENTFD2, 0, 0, 0)
	if e0 != 0 {
		syscall.Close(epfd)
		panic(err)
	}
	p.evsize = DEFAULTBASEEVENTSSIZE
	p.wfd = int(r0)
	p.closeCh = make(chan struct{})
	p.AddR(p.wfd, false)
	return p
}
func listenPoll() *Poll {
	p := new(Poll)
	epfd, err := syscall.EpollCreate1(0)
	if err != nil {
		panic(err)
	}
	p.epfd = epfd
	p.evsize = 2
	p.wfd = -1
	p.closeCh = make(chan struct{})
	return p
}

//注册Fd 监听可读事件

func (p *Poll) Close() error {
	err := syscall.Close(p.wfd)
	if err != nil {
		return err
	}
	return syscall.Close(p.epfd)
}


func (p *Poll) AddR(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: R | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: R,
	})
}
func (p *Poll) AddRW(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: R | W | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: R | W,
	})
}
func (p *Poll) AddW(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: W | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_ADD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: W,
	})
}
func (p *Poll) ModW(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: W | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: W,
	})
}
func (p *Poll) ModRW(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: R | W | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: R | W,
	})
}
func (p *Poll) ModR(fd int, et bool) error {
	if et {
		return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
			Fd:     int32(fd),
			Events: R | ET,
		})
	}
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_MOD, fd, &syscall.EpollEvent{
		Fd:     int32(fd),
		Events: R,
	})
}
func (p *Poll) Del(fd int) error {
	return syscall.EpollCtl(p.epfd, syscall.EPOLL_CTL_DEL, fd, nil)
}
func avoidListenEpollRecvNewConn(s *servant, P *poller) chan *conn {
	if P.poll.isListePoll {
		return nil
	} else {
		return s.acceptchan
	}
}
func (p *Poll) WaitFn(s *servant, P *poller, callback func(fd int, Act Action) error) error {
	for {
		select {
		case c := <-avoidListenEpollRecvNewConn(s, P):
			{
				P.fd2conns[c.fd] = c
				P.poll.AddR(c.fd, true)
				c.poller = P
				atomic.AddInt32(&P.count, 1)
				if s.srv.OnConnect != nil {
					s.srv.OnConnect(c)
				}
				//ConSole(fmt.Sprintf("Work Poll<%d> recv New Conn :%d \n", P.index, c.fd))
			}
		case <-s.closeCh:
			{
				return fmt.Errorf("Closed Signal")
			}
		default:
			{
				readyEvBuf := make([]syscall.EpollEvent, p.evsize)
				ok, err := syscall.EpollWait(p.epfd, readyEvBuf, s.srv.LoopCycle)
				if err != nil && err != syscall.EINTR {
					return err
				}
				for i := 0; i < ok && ok > 0; i++ { // new client
					if fd := int(readyEvBuf[i].Fd); fd != p.wfd {
						if p.isListePoll {
							go acceptHandle(s, fd) //里面循环accept套接字
						} else {
							var err error
							if readyEvBuf[i].Events&syscall.EPOLLIN > 0 {
								err = callback(fd, READ)
							} else if readyEvBuf[i].Events&syscall.EPOLLOUT > 0 {
								err = callback(fd, WRITE)
							}
							if err != nil {
								return err
							}
						}
					} else {
						var b [8]byte
						syscall.Read(fd, b[:])
					}
				}
				if ok == p.evsize {
					p.evsize = 2 * p.evsize
				}
			}
		}
	}
}

func Sockaddr2Addr(sa syscall.Sockaddr) net.Addr { //sockaddr 转标准库网络地址
	var Addr net.Addr
	switch sa := sa.(type) {
	case *syscall.SockaddrInet4:
		Addr = &net.TCPAddr{
			IP:   append([]byte{}, sa.Addr[:]...),
			Port: sa.Port,
		}
	case *syscall.SockaddrInet6:
		var zone string
		if sa.ZoneId != 0 {
			if ifidx, err := net.InterfaceByIndex(int(sa.ZoneId)); err != nil {
				zone = ifidx.Name
			}
		}
		if zone == "" && sa.ZoneId != 0 {
			panic("Temparory Unsupport IPV6")
		}
		Addr = &net.TCPAddr{
			IP:   append([]byte{}, sa.Addr[:]...),
			Port: sa.Port,
			Zone: zone,
		}
	case *syscall.SockaddrUnix:
		Addr = &net.UnixAddr{Net: "unix", Name: sa.Name}
	}
	return Addr
}
func SetKeepAlive(fd, secs int) error {
	if err := syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_KEEPALIVE, 1); err != nil {
		return err
	}
	if err := syscall.SetsockoptInt(fd, syscall.IPPROTO_TCP, syscall.TCP_KEEPINTVL, secs); err != nil {
		return err
	}
	return syscall.SetsockoptInt(fd, syscall.IPPROTO_TCP, syscall.TCP_KEEPIDLE, secs)
}
func ReusePortListen(network, addr string) (net.Listener, error) {
	return reuseport.Listen(network, addr)
}
