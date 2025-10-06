#include "allocator.h"

int deallocate_memory(void* ptr,size_t size){
    int tmp=munmap(ptr,size);
    if(tmp==-1){
        perror("Error during deallocation");
        return tmp;
    }
    return 1;
}