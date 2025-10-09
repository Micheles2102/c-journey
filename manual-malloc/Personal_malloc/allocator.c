#include "allocator.h"

// Global memory pool pointer.
// This structure holds bins for different block sizes.
memory_pool* memory;

// Number of blocks to allocate per bin.
// These values define the initial capacity of each bin.
int blocchi_8 = 4;
int blocchi_16 = 3;
int blocchi_32 = 2;

// Initialization flag.
// Used to determine whether the memory pool has already been created.
int INIZIO = 1;

// Allocates the memory pool structure using mmap.
// This pool will hold pointers to bins for different block sizes.
int allocate_memory_pool() {
    memory = mmap(NULL, sizeof(memory_pool), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("Memory pool allocation error");
        return -1;
    }
    INIZIO = 0; // Mark pool as initialized
    return 1;
}

// Fallback allocator for arbitrary sizes.
// Allocates a single block of memory using mmap.
// Used when no bin is suitable or for internal allocations.
void* allocate_memory(size_t size) {
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("Memory allocation error");
        return NULL;
    }
    return ptr; // Return pointer to the allocated memory
}

// Allocates and initializes a bin of fixed-size blocks.
// Parameters:
// - size: size of each block
// - number_block: index of the bin (0 for bin_8, 1 for bin_16, 2 for bin_32)
// - blocchi_allocare: number of blocks to allocate in the bin
void* allocate_bin(size_t size, const int number_block, const int blocchi_allocare) {
    void* breakingpoint = NULL;
    header_t* bin_attuale = NULL;

    // Ensure the memory pool has been initialized
    if (memory == NULL) {
        perror("Memory is not allocated");
        exit(-1);
    }

    // Select the appropriate bin based on number_block
    bin_attuale = (number_block == 0) ? memory->bin_8 :
                  (number_block == 1) ? memory->bin_16 :
                                        memory->bin_32;

    // If the bin is not yet allocated, initialize it
    if (!bin_attuale) {
        // Allocate space for the array of headers
        bin_attuale = allocate_memory(blocchi_allocare * sizeof(header_t));
        if (!bin_attuale) {
            perror("Memory allocation bin error");
            exit(-2);
        } else {
            // Update the corresponding bin pointer in the memory pool
            if (number_block == 0) memory->bin_8 = bin_attuale;
            else if (number_block == 1) memory->bin_16 = bin_attuale;
            else memory->bin_32 = bin_attuale;
        }

        // Allocate the actual memory blocks for the bin
        bin_attuale->ptr_iniziale = allocate_memory(sizeof(char) * blocchi_allocare * size);
        if (!bin_attuale->ptr_iniziale) {
            perror("Memory allocation blocks error");
            exit(-3);
        }

        // Calculate the breaking point (last byte of the last block)
        breakingpoint = (void*)((char*)bin_attuale->ptr_iniziale + (blocchi_allocare * size - 1));

        // Initialize each block in the bin
        for (int i = 0; i < blocchi_allocare; i++) {
            bin_attuale[i].is_free = true;
            bin_attuale[i].ptr_finale = (void*)((char*)bin_attuale[i].ptr_iniziale + (size - 1));

            // If this is the last block, terminate the linked list
            if ((char*)bin_attuale[i].ptr_finale == (char*)breakingpoint) {
                bin_attuale[i].ptr_next = NULL;
                break;
            }

            // Link to the next block and set its starting address
            bin_attuale[i].ptr_next = &bin_attuale[i + 1];
            bin_attuale[i + 1].ptr_iniziale = (void*)((char*)bin_attuale[i].ptr_iniziale + size);
        }
    }

    // Traverse the bin to find a free block
    header_t* tmp = bin_attuale;
    while (tmp->is_free == false) {
        if (!tmp->ptr_next) {
            perror("Unable to allocate: all blocks are occupied. Please free some memory.");
            return NULL;
        }
        tmp = tmp->ptr_next;
    }

    // Mark the block as used and return its starting address
    tmp->is_free = false;
    return tmp->ptr_iniziale;
}
