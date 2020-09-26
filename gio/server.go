package gio

import (
	"fmt"
	"net"
	"runtime"
	"sync"
	"sync/atomic"
	"syscall"
	"unsafe"
)

type poller struct {
	index    int
	poll     *Poll
	fd2conns map[int]*conn
	count    int32
}
type conn struct {
	fd int //socket fd
	//out *bytes.Buffer//可写数据
	out         []byte
	ctx         interface{}
	in          []byte //读到的数据
	sa          syscall.Sockaddr
	remote      net.Addr
	local       net.Addr
	poller      *poller
	closeStatus bool
}

func (c *conn) Context() interface{} { return c.ctx }
func (c *conn) Wake() {
	var n uint64 = 1
	syscall.Write(c.poller.poll.wfd, ((*[8]byte)(unsafe.Pointer((&n)))[:]))
}
func (c *conn) RemoteAddr() net.Addr     { return c.remote }
func (c *conn) SetContext(i interface{}) { c.ctx = i }
func (c *conn) ShutdownFd(half bool) {
	if half {
		shutDownFd(nil, c.poller, c, fmt.Errorf("Graceful Close"))
	} else {
		c.closeStatus = true
	}
}

type servant struct {
	srv        *Server
	pollers    []*poller
	wg         *sync.WaitGroup
	closeCh    chan struct{} //关闭通道
	closeln    chan struct{}
	acceptchan chan *conn
	accepts    int32
	lns        []*listener
}

func serve(srv *Server, lns []*listener) error {
	if srv.RoutineNum == 0 {
		srv.RoutineNum = runtime.NumCPU()
	}
	if srv.LoopCycle == 0 {
		srv.LoopCycle = DEFAULTLOOPCYCLE
	}
	routineNum := srv.RoutineNum
	s := new(servant)
	s.closeCh = make(chan struct{})
	s.acceptchan = make(chan *conn, 4096)
	s.closeln = make(chan struct{})
	s.wg = new(sync.WaitGroup)
	s.lns = lns
	s.srv = srv
	//	defer waitforshutdonw() wg.wati()
	for i := 0; i < routineNum+1; i++ { // + 1 第一个是监听epoll线程
		var p *poller
		if i == 0 {
			p = createListenPoller(lns)
		} else {
			p = &poller{
				index:    i,
				poll:     newPoll(),
				fd2conns: make(map[int]*conn),
			}
		}
		s.pollers = append(s.pollers, p)
	}
	for _, p := range s.pollers {
		go work(s, p)
		s.wg.Add(1)
	}
	fmt.Printf("%d WorkerRoutine On Watingfor I/O Events", routineNum+1)
	return func() error {
		s.waitForShutdonw()
		for _, p := range s.pollers {
			p.poll.closeCh <- struct{}{}
			close(p.poll.closeCh)
		}
		s.wg.Wait()
		for _, p := range s.pollers {
			for _, c := range p.fd2conns {
				closeProc(s, p, c, nil)
			}
			p.poll.Close()
		}
		return nil
	}()
}
func work(s *servant, p *poller) {
	defer func() {
		s.shutDown()
		s.wg.Done()
	}()
	itfn := func(fd int, Act Action) error {
		c := p.fd2conns[fd]
		switch {
		case c == nil:
			{
				fmt.Println("No conn found")
			}
		case Act == WRITE && len(c.out) > 0:
			{
				return sendProc(s, p, c)
			}
		default:
			return recvProc(s, p, c)
		}
		return nil
	}
	p.poll.WaitFn(s, p, itfn)
}

func recvProc(s *servant, p *poller, c *conn) error {
	for {
		buf := make([]byte, 0xFFFF)
		n, err := syscall.Read(c.fd, buf)
		if n == 0 || err != nil {
			if err == syscall.EAGAIN {
				break
			}
			return closeProc(s, p, c, err)
		}
		if n > 0 {
			c.in = append(c.in, buf[:n]...)
		}
		if s.srv.Data != nil {
			out, _ := s.srv.Data(c, c.in)
			c.in = nil
			if len(out) >= 0 {
				c.out = append(c.out, out...)
			} else if c.closeStatus {
				return closeProc(s, p, c, fmt.Errorf("%s Point Closed", c.remote.String()))
			}
		}
	}
	if len(c.out) != 0 {
		p.poll.ModRW(c.fd, true)
	}
	return nil
}
func sendProc(s *servant, p *poller, c *conn) error {
	for {
		if len(c.out) > 0 {
			n, err := syscall.Write(c.fd, c.out)

			if n <= 0 || err != nil {
				if err == syscall.EAGAIN {
					break
				}
				return closeProc(s, p, c, err)
			}
			if n == len(c.out) {
				c.out = c.out[:0]
				break
			} else {
				c.out = c.out[n:]
			}
		} else {
			break
		}
	}
	if len(c.out) == 0 {
		p.poll.ModR(c.fd, true)
	}
	if c.closeStatus == true {
		return closeProc(s, p, c, fmt.Errorf("Delay Closed"))
	}
	return nil
}
func closeProc(s *servant, p *poller, c *conn, err error) error {
	atomic.AddInt32(&p.count, -1)
	delete(p.fd2conns, c.fd)
	p.poll.Del(c.fd)
	syscall.Close(c.fd)
	return nil
}
func shutDownFd(s *servant, p *poller, c *conn, err error) error {
	syscall.Shutdown(c.fd, syscall.SHUT_RD)
	return nil
}
func acceptHandle(s *servant, fd int) error {
	for _, ln := range s.lns {
		//no roundroubin
		if ln.fd == fd {
			for {
				nfd, sa, err := syscall.Accept(fd)
				if err != nil {
					if err == syscall.EAGAIN {
						return nil
					}
					return err
				}
				if err := syscall.SetNonblock(nfd, true); err != nil {
					return err
				}
				//if err := syscall.SetsockoptLinger(nfd,syscall.SOL_SOCKET,syscall.SO_LINGER,&syscall.Linger{Onoff: int32(0)});err != nil{
				//	return err
				//}
				c := &conn{fd: nfd, sa: sa, in: nil, out: []byte{}}
				c.remote = Sockaddr2Addr(sa)
				atomic.AddInt32(&s.accepts, 1)
				s.acceptchan <- c
			}
		}
	}
	return nil
}

func createListenPoller(lns []*listener) *poller {
	poller := &poller{
		index:    0,
		poll:     listenPoll(),
		fd2conns: make(map[int]*conn),
		count:    int32(len(lns)),
	}
	for _, ln := range lns {
		poller.poll.AddR(ln.fd, true)
		poller.fd2conns[ln.fd] = &conn{fd: ln.fd, poller: poller}
	}
	poller.poll.isListePoll = true
	return poller
}

func (s *servant) waitForShutdonw() {
	<-s.closeCh
	close(s.closeCh)
}
func (s *servant) shutDown() {
	s.closeCh <- struct{}{}
}
