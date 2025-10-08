#include "allocator.h"
//idea dell'allocazione dei blocchi:
// [Block 0] → [Block 1] → [Block 2] → [Block 3] → NULL
//   0x1000     0x1008     0x1010     0x1018

void* allocator(size_t size){
   if(INIZIO!=0){
        if(allocate_memory_pool()==-1){
            perror("Memory pool error");
            exit(-3);
        }
   }
   if(size<=8){
        void* breakingpoint=NULL;
        if(!memory->bin_8){
            if(!(memory->bin_8=allocate_memory_for_bin(blocchi_8*sizeof(header_t)))){
                perror("Memory allocation bin error");
                exit(-1);
            }

            if(!(memory->bin_8->ptr_iniziale=allocate_memory(sizeof(char)*blocchi_8*8))){
                //voglio che crea 4 blocchi da 8
                perror("Memory allocation blocks error");
                exit(-2);
            }
            
            //mi setto un breaking point
            //siccome sto lavorando su bye con void* caso (void*)((char*) in modo tale da potere modificare spostandomi e lo ricasto a void*
            breakingpoint=(void*)((char*)memory->bin_8->ptr_iniziale+31);

            for(int i=0;i<blocchi_8;i++){
                memory->bin_8[i].is_free=1;
                memory->bin_8[i].ptr_finale=(void*)((char*)memory->bin_8[i].ptr_iniziale+7);
                //quando arriva ad avere i=4 non farà l'ultimo codice a riga 20 perchp si interrompe qui
                if((void*)((char*)memory->bin_8[i].ptr_finale+1)==breakingpoint){
                    memory->bin_8[i].ptr_next=NULL;
                    break;
                }
                memory->bin_8[i].ptr_next=&memory->bin_8[i+1];
                memory->bin_8[i+1].ptr_iniziale=(void*)((char*)memory->bin_8[i].ptr_iniziale+8);
            }
        }

        //fino a quando vede che non ci sono blocchi disponibili
        header_t* tmp=memory->bin_8;
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
        if(!memory->bin_16){
            if(!(memory->bin_16=allocate_memory_for_bin(blocchi_16*sizeof(header_t)))){
                perror("Memory allocation bin error");
                exit(-1);
            }

            if(!(memory->bin_16->ptr_iniziale=allocate_memory(sizeof(char)*blocchi_16 * 16))){
                // voglio che crei 3 blocchi da 16 byte ciascuno

                perror("Memory allocation blocks error");
                exit(-2);
            }
            //mi setto un breaking point
            breakingpoint=(void*)((char*)memory->bin_16->ptr_iniziale+47);

            for(int i=0;i<3;i++){
                memory->bin_16[i].is_free=1;
                memory->bin_16[i].ptr_finale=(void*)((char*)memory->bin_16[i].ptr_iniziale+15);
                //quando arriva ad avere i=4 non farà l'ultimo codice a riga 20 perchp si interrompe qui
                if((void*)((char*)memory->bin_16[i].ptr_finale+1)==breakingpoint){
                    memory->bin_16[i].ptr_next=NULL;
                    break;
                }
                memory->bin_16[i].ptr_next=&memory->bin_16[i+1];
                memory->bin_16[i+1].ptr_iniziale=(void*)((char*)memory->bin_16[i].ptr_iniziale+16);
            }
        }

        //fino a quando vede che non ci sono blochci disponibili
        header_t* tmp=memory->bin_16;
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
   if(size<=32 && size>16){
        void* breakingpoint=NULL;
        if(!memory->bin_32){
            if(!(memory->bin_32=allocate_memory_for_bin(blocchi_32*sizeof(header_t)))){
                perror("Memory allocation bin error");
                exit(-1);
            }

            if(!(memory->bin_32->ptr_iniziale=allocate_memory(sizeof(char)*blocchi_32 * 32))){
                // voglio che crei 3 blocchi da 16 byte ciascuno

                perror("Memory allocation blocks error");
                exit(-2);
            }
            //mi setto un breaking point
            breakingpoint=(void*)((char*)memory->bin_32->ptr_iniziale+63);

            for(int i=0;i<blocchi_32;i++){
                memory->bin_32[i].is_free=1;
                memory->bin_32[i].ptr_finale=(void*)((char*)memory->bin_32[i].ptr_iniziale+15);
                //quando arriva ad avere i=4 non farà l'ultimo codice a riga 20 perchp si interrompe qui
                if((void*)((char*)memory->bin_32[i].ptr_finale+1)==breakingpoint){
                    memory->bin_32[i].ptr_next=NULL;
                    break;
                }
                memory->bin_32[i].ptr_next=&memory->bin_32[i+1];
                memory->bin_32[i+1].ptr_iniziale=(void*)((char*)memory->bin_32[i].ptr_iniziale+16);
            }
        }

        //fino a quando vede che non ci sono blochci disponibili
        header_t* tmp=memory->bin_32;
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