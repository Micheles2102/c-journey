#ifndef ALLOCATOR_H
#define ALLOCATOR_H


#include <sys/mman.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>



typedef struct{
    void* ptr_iniziale; // puntatore iniziale di quel blocco 
    void* ptr_finale; //puntatoore finale di quel blocco ptr_iniziale+7; 7 perchè ptr_iniziale è incluso
    header_t* ptr_next; // mi serve per creare una lista tra i vari header
    bool is_free;
}header_t;

//memory pool per avere una mini cache di blocchi più grandi allocati per quella dimensione
typedef struct 
{
    header_t* bin_8;
    header_t* bin_16;
    header_t* bin_24;
    header_t* bin_32;
}memory_pool;

extern int INIZIO;
extern int blocchi_8;
extern int blocchi_16;
extern int blocchi_32;
extern memory_pool* memory;

//questa funziona crea/gestisce la memory pool
void* allocator(size_t size);
// questo per allocare memoria nel caso in cui non ci siano blocchi
void* allocate_memory(size_t size);
//funzione per allocare le struct dei bin
void* allocate_memory_for_bin(size_t size);

void* allocate_memory_for_pool(size_t size);

//funzione per allocare la pool
int allocate_memory_pool();
// questa non cambierà
int deallocate_memory(void* ptr,size_t size);


#endif ALLOCATOR_H