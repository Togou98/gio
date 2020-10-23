template<typename T>
void GC(T ptr){
    free(ptr);
    ptr = 0;
}