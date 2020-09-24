package gio

import "fmt"

type Buf struct{
	buf []byte
	rdx int
	wdx int
	hdx int
	ln int
}
func NewBuf(ln int)*Buf{
	buf := []byte{}
	rdx := 0
	hdx := 0
	wdx := rdx
	return &Buf{buf,rdx,wdx,hdx,ln}
}
func(b *Buf)Cap()int{
	return cap(b.buf)
}
func(b *Buf)Len()int{
	return b.wdx - b.rdx
}

func(b *Buf)In(data []byte){
	b.wdx += len(data)
	b.buf = append(b.buf,data...)
}
func(b *Buf)Out(ln int)[]byte {
	out := make([]byte,ln)
	movedLn := copy(out,b.buf[b.rdx:b.wdx])
	b.rdx += movedLn
	if b.rdx == b.wdx{
		b.rdx,b.wdx = 0,0
		b.buf = b.buf[:b.rdx]
	}
	return out
}
func(b *Buf)String(){
	fmt.Sprintf("%v",b.buf[:b.Len()])
}