package gio
import (
//	"container/list"
	"time"
)

type TimeWheel struct {
	interval time.Duration
	tick     <-chan time.Time
	curPos   int
	maxPos   int
	callback func(interface{})
	lst []*list.List
}
func(t *TimeWheel)start(){
	t.tick = time.Tick(t.interval)
	for _ := range t.tick{
		//1s
	}
}
type Task struct {
	delay time.Duration
	circle int
}


