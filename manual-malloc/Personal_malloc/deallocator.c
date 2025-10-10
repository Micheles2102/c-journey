#include "allocator.h"
//reminder
//adjust part with exit in other .c code

int deallocate_block(void* ptr,size_t size,const int number_block){
    header_t*  bin_attuale=NULL;
    // Select the appropriate bin based on number_block
    bin_attuale = (number_block == 0) ? memory->bin_8 :
                  (number_block == 1) ? memory->bin_16 :
                                        memory->bin_32;
    if (!bin_attuale) {
        fprintf(stderr, "Bin has not been initialized\n");
        return -1;
    }
    while(bin_attuale && bin_attuale->ptr_iniziale!=ptr){
        bin_attuale=bin_attuale->ptr_next;
    }
    if(bin_attuale->ptr_finale==bin_attuale->breakingpoint){
        perror("error not block found"); 
        return -1;
    }
    bin_attuale->is_free=1;
    if(munmap(bin_attuale->ptr_iniziale,size)==-1){
        perror("Error during munmap()");
        if(deallocate_everything()==0){
            perror("all dealloc. for safe");    
        }
        
        return -1;
    }
    return 1;
}


int deallocate_bin(header_t* bin,size_t size){
    while(bin && bin->ptr_finale!=bin->breakingpoint){
        if(munmap(bin->ptr_iniziale,size)!=0){
            perror("Error during munmap()");
            return -1;
            break;
        }
        bin=bin->ptr_next;
    }
    if(munmap(bin,sizeof(header_t))!=0){
        perror("error delete bin");
        return -1;
    }
    return 1;
}

int deallocate_everything(){
    int block=8;
    header_t* bins[] = { memory->bin_8, memory->bin_16, memory->bin_32 };
    for(int i=0;i<3;i++){
        if(!bins[i]){
            continue;
        }
        if(deallocate_bin(bins[i],sizeof(char)*block)!=0)
            return -1;
        block=block*2;
    }
    if(munmap(memory,sizeof(memory_pool))==-1){
        perror("error delete the memory pool");
        return -1;
    }

    return 1;
}