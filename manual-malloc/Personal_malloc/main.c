#include "allocator.h"
#include <stdio.h>

int main(){

    int *data=allocator(sizeof(int)*5);

    for(int i=0;i<5;i++){
        data[i]=i;
        printf("Il valore di data e' : %d\n",data[i]);
    }
    printf("data points to: %p\n", (void*)data);

    int test=1;
    if((test=deallocator(data,sizeof(int)*5))!=1){
        perror("Error during free");
        return -1;
    }
    printf("il valore di test è %d",test);
    data = NULL;

    printf("è andato tutto bene");
    return 0;
}