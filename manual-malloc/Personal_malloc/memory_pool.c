#include "allocator.h"

void* allocator(size_t size){
   if(size<=8){
        void* breakingpoint=NULL;
        if(!memory.bin_8){
            memory.bin_8=allocate_memory_for_bin(4);
            memory.bin_8->ptr_iniziale=allocate_memory(32);//voglio che crea 4 blocchi da 8
             //mi setto un breaking point
            breakingpoint=memory.bin_8->ptr_iniziale+31;

            for(int i=0;i<4;i++){
                memory.bin_8[i].is_free=1;
                memory.bin_8[i].ptr_finale=memory.bin_8[i].ptr_iniziale+7;
                memory.bin_8[i].ptr_next=&memory.bin_8[i]+1;
                if(memory.bin_8[i].ptr_finale+1==breakingpoint){
                    break;
                }
                memory.bin_8[i].ptr_iniziale=memory.bin_8[i].ptr_finale+1;
            }
        }

        //fino a quando vede che non ci sono blochci disponibili
        while(memory.bin_8->is_free==0){
            //controlla se il successivo è il breaking point
            if(memory.bin_8->ptr_next != breakingpoint)
                //se non lo è vai al successivo
                memory.bin_8=memory.bin_8->ptr_next;
            //se dovesse esserlo
            perror("Impossibile creare nuova memoria,tutti i blocchi sono occupati; liberare qualcosa");
            return NULL;
        }
        memory.bin_8->is_free=0;
        return memory.bin_8->ptr_iniziale;
   }
   if(size<32 && size>8){
        void* breakingpoint=NULL;
        if(!memory.bin_16){
            memory.bin_16=allocate_memory_for_bin(4);
            memory.bin_16->ptr_iniziale=allocate_memory(96);//voglio che crea 3 blocchi da 32
             //mi setto un breaking point
            breakingpoint=memory.bin_16->ptr_iniziale+95;

            for(int i=0;i<4;i++){
                memory.bin_16[i].is_free=1;
                memory.bin_16[i].ptr_finale=memory.bin_16[i].ptr_iniziale+7;
                memory.bin_16[i].ptr_next=&memory.bin_16[i]+1;
                if(memory.bin_16[i].ptr_finale+1==breakingpoint){
                    break;
                }
                memory.bin_16[i].ptr_iniziale=memory.bin_16[i].ptr_finale+1;
            }
        }

        //fino a quando vede che non ci sono blochci disponibili
        while(memory.bin_16->is_free==0){
            //controlla se il successivo è il breaking point
            if(memory.bin_16->ptr_next != breakingpoint)
                //se non lo è vai al successivo
                memory.bin_16=memory.bin_16->ptr_next;
            //se dovesse esserlo
            perror("Impossibile creare nuova memoria,tutti i blocchi sono occupati; liberare qualcosa\n");
            return NULL;
        }
        memory.bin_16->is_free=0;
        return memory.bin_16->ptr_iniziale;
   }
}