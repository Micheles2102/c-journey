#include "allocator.h"
#include <stdio.h>

/* //CHECK DONE
gcc  allocator.c memory_pool.c allocator.h deallocator.c main.c -o test
strace -e mmap,munmap ./test
valgrind --leak-check=full ./test */

int main(){
    int size=2;
    int *data=allocator(sizeof(int)*size);

    for(int i=0;i<size;i++){
        data[i]=i;
        printf("Data value is' : %d\n",data[i]);
    }
    printf("data points to: %p\n", (void*)data);

    int test=1;
    if((test=deallocator(data,sizeof(int)*size))!=1){
        perror("Error during free");
        return -1;
    }
    printf("Test: %d",test);
    data = NULL;

    return 0;
}