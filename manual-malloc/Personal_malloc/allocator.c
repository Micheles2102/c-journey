#include "allocator.h"


void* allocate_memory(size_t size){
    void* ptr=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(ptr==MAP_FAILED){
        perror("Memory allocation error");
        return NULL;
    }
    return ptr; //puntatore al primo elemento allocato
}

void* allocate_memory_for_bin(size_t size){ //sono uno contiiguo all'altro (come indirizzo virtuale)
    void* ptr=mmap(NULL,size*sizeof(header_t),PROT_READ|PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(ptr==MAP_FAILED){
        perror("Memory allocation error\n");
        return NULL;
    }
    return ptr; 
}