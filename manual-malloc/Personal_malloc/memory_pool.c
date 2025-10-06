#include "allocator.h"

void* allocator(size_t size){
   if(size<=8){
        void* breakingpoint=NULL;
        if(!memory.bin_8){
            if(!(memory.bin_8=allocate_memory_for_bin(4))){
                perror("Memory allocation bin error");
                exit(-1);
            }

            if(!(memory.bin_8->ptr_iniziale=allocate_memory(4*8))){
                //voglio che crea 4 blocchi da 8
                perror("Memory allocation blocks error");
                exit(-2);
            }
            
            //mi setto un breaking point
            //siccome sto lavorando su bye con void* caso (void*)((char*) in modo tale da potere modificare spostandomi e lo ricasto a void*
            breakingpoint=(void*)((char*)memory.bin_8->ptr_iniziale+31);

            for(int i=0;i<4;i++){
                memory.bin_8[i].is_free=1;
                memory.bin_8[i].ptr_finale=(void*)((char*)memory.bin_8[i].ptr_iniziale+7);
                //quando arriva ad avere i=4 non farà l'ultimo codice a riga 20 perchp si interrompe qui
                if((void*)((char*)memory.bin_8[i].ptr_finale+1)==breakingpoint){
                    memory.bin_8[i].ptr_next=NULL;
                    break;
                }
                memory.bin_8[i].ptr_next=&memory.bin_8[i+1];
                memory.bin_8[i+1].ptr_iniziale=(void*)((char*)memory.bin_8[i].ptr_iniziale+8);
            }
        }

        //fino a quando vede che non ci sono blocchi disponibili
        header_t* tmp=memory.bin_8;
        while(tmp->is_free==0){
            //controlla se il successivo è il breaking point(dunque NULL)
            if(!tmp->ptr_next){
                //se dovesse esserlo
                perror("Impossibile creare nuova memoria,tutti i blocchi sono occupati; liberare qualcosa");
                return NULL;
            }
            //se non lo è vai al successivo
            tmp=tmp->ptr_next;
            
        }
        tmp->is_free=0;
        return tmp->ptr_iniziale;
   }
   if(size<=16 && size>8){
        void* breakingpoint=NULL;
        if(!memory.bin_16){
            if(!(memory.bin_16=allocate_memory_for_bin(3))){
                perror("Memory allocation bin error");
                exit(-1);
            }

            if(!(memory.bin_16->ptr_iniziale=allocate_memory(3 * 16))){
                // voglio che crei 3 blocchi da 16 byte ciascuno

                perror("Memory allocation blocks error");
                exit(-2);
            }
            //mi setto un breaking point
            breakingpoint=(void*)((char*)memory.bin_16->ptr_iniziale+47);

            for(int i=0;i<3;i++){
                memory.bin_16[i].is_free=1;
                memory.bin_16[i].ptr_finale=(void*)((char*)memory.bin_16[i].ptr_iniziale+15);
                //quando arriva ad avere i=4 non farà l'ultimo codice a riga 20 perchp si interrompe qui
                if((void*)((char*)memory.bin_16[i].ptr_finale+1)==breakingpoint){
                    memory.bin_16[i].ptr_next=NULL;
                    break;
                }
                memory.bin_16[i].ptr_next=&memory.bin_16[i+1];
                memory.bin_16[i+1].ptr_iniziale=(void*)((char*)memory.bin_16[i].ptr_iniziale+16);
            }
        }

        //fino a quando vede che non ci sono blochci disponibili
        header_t* tmp=memory.bin_16;
        while(tmp->is_free==0){
            //controlla se il successivo è il breaking point(dunque NULL)
            if(!tmp->ptr_next){
                //se dovesse esserlo
                perror("Impossibile creare nuova memoria,tutti i blocchi sono occupati; liberare qualcosa");
                return NULL;
            }
            //se non lo è vai al successivo
            tmp=tmp->ptr_next;
            
        }
        tmp->is_free=0;
        return tmp->ptr_iniziale;
   }
   return NULL; 
}