#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <sys/mman.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Structure representing metadata for each memory block.
// Used to track allocation status and link blocks together in a bin.
typedef struct header {
    void* ptr_iniziale;     // Starting address of the block
    void* ptr_finale;       // Ending address of the block (inclusive)
    struct header* ptr_next;// Pointer to the next block in the bin (linked list)
    void* breakingpoint;
    bool is_free;           // Flag indicating whether the block is available
} header_t;

// Structure representing a memory pool.
// Each bin holds a linked list of blocks of a specific size.
// Acts as a mini-cache to optimize repeated allocations of small blocks.
typedef struct {
    header_t* bin_8;   // Bin for 8-byte blocks
    header_t* bin_16;  // Bin for 16-byte blocks
    header_t* bin_32;  // Bin for 32-byte blocks
} memory_pool;

// Global variables shared across allocator modules
extern int INIZIO;             // Flag to check if the pool has been initialized
extern memory_pool* memory;    // Pointer to the memory pool structure

// Structure used for dynamic dispatch of allocation functions.
// Each entry maps a block size to its corresponding allocator function.
// Enables flexible selection of allocation strategy at runtime.
typedef struct {
    const size_t size_block;   // Size of each block in the bin
    void* (*allocator_memory)(size_t size, const int number_block, const int blocchi_allocare);
    // Function pointer to the allocator for this bin
} comand_for_bin;


typedef struct {
    const size_t size_block;   // Size of each block in the bin
    int (*deallocate_block)(void* ptr,size_t size,const int blocchi_allocare);
    
} comand_for_deallo;


int deallocate_block(void* ptr,size_t size,const int blocchi_allocare);


// Allocator fun   
// Allocates a series of blocks of the given size and links them together.
// Parameters:
// - size: size of each block
// - number_block: index of the bin (used to select bin_8, bin_16, etc.)
// - blocchi_allocare: number of blocks to allocate
void* allocate_bin(size_t size, const int number_block, const int blocchi_allocare);

// Main allocator interface.
// Called by the user to request memory of a given size.
// Internally dispatches to the correct bin or fallback allocator.
void* allocator(size_t size);

// Fallback allocator for arbitrary sizes.
// Uses mmap to allocate memory directly when no bin is suitable.
void* allocate_memory(size_t size);

// Allocates memory for the bin metadata (array of header_t).
// Used during bin initialization.
void* allocate_memory_for_bin(size_t size);

// Initializes the memory pool structure.
// Allocates space for the bins and sets up initial state.
int allocate_memory_pool();

// Deallocator function.
// Frees a previously allocated block.
// Note: Implementation may vary depending on bin or direct allocation.
int deallocator(void* ptr, size_t size);
int deallocate_everything();
int deallocate_bin(header_t* bin,size_t size);
#endif
