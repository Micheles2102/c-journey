//DEPRECATED
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // Provides access to sbrk() for manual heap management

// Define a 16-byte alignment block to ensure proper memory alignment
typedef char ALIGN[16];

// Header structure for each memory block
// Contains metadata: size, free flag, and pointer to the next block
union header {
    struct {
        size_t size;           // Size of the allocated block
        unsigned is_free;      // Flag indicating if the block is free (1) or in use (0)
        union header *next;    // Pointer to the next block in the linked list
    } s;
    ALIGN stub;               // Ensures proper alignment of the header
};
typedef union header header_t;

// Global pointers to the start and end of the memory block list
header_t *head, *tail;

// Mutex to ensure thread-safe access to the allocator
pthread_mutex_t global_malloc_lock;

/**
 * get_free_block: searches the linked list for a free block
 * that is large enough to satisfy the requested size.
 * Returns a pointer to the block if found, otherwise NULL.
 */
header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while(curr) {
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}

/**
 * my_malloc: custom implementation of malloc using sbrk().
 * Allocates memory by either reusing a free block or extending the heap.
 * Returns a pointer to usable memory, or NULL on failure.
 */
void *my_malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;

    if (!size)
        return NULL;

    pthread_mutex_lock(&global_malloc_lock);

    // Try to reuse an existing free block
    header = get_free_block(size);
    if (header) {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1); // Return pointer to memory after the header
    }

    // Allocate new memory via sbrk
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    // Initialize the new block's header
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    // Update the linked list
    if (!head)
        head = header;
    if (tail)
        tail->s.next = header;
    tail = header;

    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

/**
 * my_free: custom implementation of free.
 * Marks the block as free, and if it's at the end of the heap,
 * releases it using sbrk to shrink the program break.
 */
int my_free(void *block)
{
    header_t *header, *tmp;
    void *programbreak;

    if (!block)
        return -1;

    pthread_mutex_lock(&global_malloc_lock);

    // Retrieve the header associated with the block
    header = (header_t*)block - 1;

    // Get current program break
    programbreak = sbrk(0);

    // If the block is at the end of the heap, shrink the heap
    if ((char*)header + sizeof(header_t) + header->s.size == programbreak) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            tmp = head;
            while (tmp) {
                if(tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        if (sbrk(0 - sizeof(header_t) - header->s.size) == (void*)-1) {
            perror("sbrk shrink failed");
            pthread_mutex_unlock(&global_malloc_lock);
            return -1;
        }
    }

    // Otherwise, mark the block as free
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
    return 1;
}

/**
 * main: demonstrates usage of the custom allocator.
 * Allocates an array of integers, initializes it, prints values,
 * and then frees the memory.
 */
int main(){
    // Initialize the mutex for thread-safe allocation
    if (pthread_mutex_init(&global_malloc_lock, NULL) != 0) {
        perror("Mutex initialization failed");
        return -1;
    }

    // Allocate memory for 10 integers
    int* data = my_malloc(sizeof(int) * 10);
    if(!data){
        perror("Allocation Error");
        return -1;
    }

    // Initialize and print the array
    for(int i = 0; i < 10; i++){
        data[i] = i;
        printf("Value of data[%d] is %d\n", i, data[i]);
    }

    // Free the allocated memory
    int freed = my_free(data);
    if(freed < 0){
        perror("Free failed");
        return -1;
    }

    // Destroy the mutex before exiting
    pthread_mutex_destroy(&global_malloc_lock);
}