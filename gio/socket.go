package gio

import (
	"fmt"
	"net"
	"os"
	"strconv"
	"strings"
	"syscall"
)

const LISTENBACKLOG = 4096

func bindAndListen(Addr string) (fd int, err error) {
	ipAndPort := strings.SplitN(Addr, string(COLON[:]), 2)
	if len(ipAndPort) != 2 {
		return -1, fmt.Errorf("Parse %s Error", Addr)
	}
	ip := net.ParseIP(ipAndPort[0])
	port, err := strconv.Atoi(ipAndPort[1])
	if err != nil {
		return -1, fmt.Errorf("Port :%s Error", port)
	}

	fd, err = syscall.Socket(syscall.AF_INET, syscall.SOCK_STREAM, syscall.IPPROTO_IP)
	if err != nil {
		return -1, err
	}
	err = setReusePort(fd)
	if err != nil {
		return fd, err
	}
	addr := &syscall.SockaddrInet4{Port: port, Addr: [4]byte{ip[0], ip[1], ip[2], ip[3]}}
	err = syscall.Bind(fd, addr)
	if err != nil {
		return
	}
	err = syscall.Listen(fd, LISTENBACKLOG)
	if err != nil {
		return
	}
	return fd, nil
}

type listener struct {
	addr, network string //default TCP
	fd            int
}

func (l *listener) Close() error {
	return syscall.Close(l.fd)
}
func buildListener(Addr string) (*listener, error) {
	var err error
	l := new(listener)
	l.addr, l.network = Addr, "tcp"
	l.fd, err = bindAndListen(Addr)
	if err != nil {
		return nil, err
	}
	return l, nil
}
func setNonblock(fd int) error {
	return syscall.SetNonblock(fd, true)

}
func setReusePort(fd int) error {
	// 0xf so_reuseport
	return syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, 0xf, 1)
}
func SetFdLimit(n int) {
	l := syscall.Rlimit{}
	l.Cur = uint64(n)
	syscall.Setrlimit(syscall.RLIMIT_NOFILE, &l)
	err := syscall.Getrlimit(syscall.RLIMIT_NOFILE, &l)
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}
	fmt.Printf("CurFdLimit: %d | MaxFdLimit: %d\n", l.Cur, l.Max)
}
