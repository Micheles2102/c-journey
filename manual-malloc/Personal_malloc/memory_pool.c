#include "allocator.h"

// Reminder: using sizeof(char) even when not strictly necessary to emphasize byte-level operations

// Block allocation strategy:
// [Block 0] → [Block 1] → [Block 2] → [Block 3] → NULL
//   0x1000     0x1008     0x1010     0x1018
// Each block is linked to the next via a header_t structure

// Array of function pointer structs used to optimize allocation based on runtime size requirements.
// NOTE: In this version, all bins use the same allocator function (allocate_bin), 
// but this structure allows future customization per bin (e.g., different strategies for 8 vs 32 bytes).
comand_for_bin alloc_pool[] = {
    {.size_block = sizeof(char) * 8,  .allocator_memory = allocate_bin},
    {.size_block = sizeof(char) * 16, .allocator_memory = allocate_bin},
    {.size_block = sizeof(char) * 32, .allocator_memory = allocate_bin},
    {.size_block = 0} // Sentinel to mark end of array
};

comand_for_deallo dealloc_pool[] = {
    {.size_block = sizeof(char) * 8,  .deallocate_block = deallocate_block},
    {.size_block = sizeof(char) * 16, .deallocate_block = deallocate_block},
    {.size_block = sizeof(char) * 32, .deallocate_block = deallocate_block},
    {.size_block = 0} // Sentinel to mark end of array
};

// Main allocator function called by the user.
// It initializes the memory pool if needed, selects the appropriate bin based on size,
// and dispatches the allocation to the correct function via function pointer.
void* allocator(size_t size) {
    // Initialize memory pool on first call
    if (INIZIO != 0) {
        if (allocate_memory_pool() == -1) {
            perror("Memory pool error");
            exit(-3); // Exit if pool allocation fails
        }
    }

    void* check = NULL;
    int i = 0;

    // Guard clause: reject allocations larger than the largest bin size
    if (size > sizeof(char) * 32) {
        perror("Impossible allocation for this size");
        exit(-4);
    }

    // Dispatch loop: find the first bin that can satisfy the requested size
    // The condition ensures that the allocator_memory function is called only if size fits the bin
    // (4 - i) is used to determine how many blocks to allocate for that bin
    do {
        check = (size <= alloc_pool[i].size_block)
            ? alloc_pool[i].allocator_memory(alloc_pool[i].size_block, i, (4 - i))
            : NULL;
        i++;
    } while (!check); // Repeat until a valid allocation is returned

    return check; // Return pointer to allocated memory
}


int deallocator(void* ptr,size_t size){
    int check=0;
    int i=0;
    do {
        check = (size <= dealloc_pool[i].size_block)
            ? dealloc_pool[i].deallocate_block(ptr,dealloc_pool[i].size_block,i)
            : 0;
        i++;
    } while (!check); // Repeat until a valid allocation is returned

    return check;
}