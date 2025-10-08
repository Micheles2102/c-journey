#include "allocator.h"
memory_pool* memory;
int blocchi_8=4;
int blocchi_16=3;
int blocchi_32=2;

int allocate_memory_pool(){
    memory=mmap(NULL,sizeof(memory_pool),PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(memory==MAP_FAILED){
        perror("Memory pool allocation error");
        return -1;
    }
    INIZIO=0;
    return 1;
}
void* allocate_memory(size_t size){
    void* ptr=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(ptr==MAP_FAILED){
        perror("Memory allocation error");
        return NULL;
    }
    return ptr; //puntatore al primo elemento allocato
}

void* allocate_memory_for_bin(size_t size){ //sono uno contiiguo all'altro (come indirizzo virtuale)
    void* ptr=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(ptr==MAP_FAILED){
        perror("Memory allocation error\n");
        return NULL;
    }
    return ptr; 
}