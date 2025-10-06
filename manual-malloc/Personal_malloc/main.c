#include "allocator.h"
#include <stdio.h>

int main(){

    int *data=allocator(sizeof(int*)*5);

    for(int i=0;i<8;i++){
        data[i]=i;
        printf("Il valore di data e' : %d\n",data[i]);
    }

    deallocator(data);

    return 0;
}