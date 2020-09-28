package gio

type Buf struct{
	buf []byte
	r int
	w int
}
func Buffer(size int)*Buf{
	buf :=make([]byte,size)
	buf = buf[:0]
	r := 0
	w := r
	return &Buf{buf,r,w}
}
func(b *Buf)Len()int{
	return b.w - b.r
}
func(b *Buf)Cap()int{
	return cap(b.buf)
}
func(b *Buf)Write(data []byte)(int,error){
	if b.Cap() - b.w > len(data) && b.r > len(data){
		copy(b.buf[:b.w - b.r],b.buf[b.r:b.w])
		b.r -= len(data)
		b.w -= len(data)
	}
	if b.Cap() < b.w + len(data){
		b.buf = append(b.buf,data...)
	}else{
		b.buf = append(b.buf[:b.w],data...)
	}
	b.w += len(data)
	return len(data),nil
}
func(b *Buf)Read(data []byte)(int,error){
	tmp := b.buf[b.r:b.w]
	data = tmp
	b.r += len(tmp)
	return len(data),nil
}
