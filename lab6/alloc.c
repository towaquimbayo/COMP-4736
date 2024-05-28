#include "alloc.h"

typedef struct Node {
  int size; // size of the block of memory
  void *ptr; // pointer to the start of the block of memory
  struct Node *next; // pointer to the next block of memory
} Node;

Node *free_list = NULL; // linked list of free blocks of memory
Node *node_list = NULL; // linked list of blocks of memory in use

/**
 * Initialize the memory manager by alloacing a 4KB page from the OS.
 * Initialize any other data structures as required.
 * Return 0 if initialization is successful, 1 otherwise.
 * @return 0 if initialization is successful, 1 otherwise.
 */
int init_alloc() {
  if (free_list != NULL || node_list != NULL) {
    printf("Memory manager has already been initialized\n");
    return 1;
  }

  // Initialize free_list and node_list as empty linked lists
  void *node_memory = mmap(NULL, sizeof(Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (node_memory == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  free_list = (Node *) node_memory;
  node_list = (Node *) node_memory;

  // Allocate a 4KB page from the OS using mmap
  void *page_memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (page_memory == MAP_FAILED) {
    perror("mmap");
    return 1;
  }
  
  // Initialize free_list with the 4KB page
  free_list->size = PAGESIZE;
  free_list->ptr = page_memory;
  free_list->next = NULL;

  printf("Initialized memory manager with %d bytes at %p\n", PAGESIZE, free_list->ptr);
  return 0;
}

/**
 * Cleanup any data structures used by the memory manager.
 * Free any memory allocated from the OS.
 * Return 0 if cleanup is successful, 1 otherwise.
 */
int cleanup() {
  if (free_list == NULL || node_list == NULL) {
    printf("Memory manager has not been initialized\n");
    return 1;
  }

  // Free the free_list
  if (munmap(free_list, sizeof(Node)) == -1) {
    perror("munmap");
    return 1;
  }

  // Free the node_list
  if (munmap(node_list, sizeof(Node)) == -1) {
    perror("munmap");
    return 1;
  }
  
  free_list = NULL;
  node_list = NULL;
  printf("Memory manager has been cleaned up\n");
  return 0;
}

/**
 * Allocate a block of memory of given size and returns a
 * char* pointing to the start of the block of memory allocated,
 * or NULL if the allocation fails.
 * @param size Number of bytes to allocate.
 */
char *alloc(int size) {
  // Check if the size is valid
  if (size <= 0 || size > PAGESIZE || size % MINALLOC != 0) {
    printf("Invalid size of memory to allocate\n");
    return NULL;
  }

  // Check if the memory manager has been initialized
  if (free_list == NULL || node_list == NULL) {
    printf("Memory manager has not been initialized\n");
    return NULL;
  }

  // Check if there is a block of memory in free_list that is large enough
  printf("Current free list size: %d bytes at %p\n", free_list->size, free_list->ptr);
  if (size > free_list->size) {
    printf("No block of memory is large enough\n");
    return NULL;
  }

  void *node_memory = mmap(NULL, sizeof(Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (node_memory == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }

  // Create a new node to store the block of memory
  Node *new_node = (Node *) node_memory;
  new_node->size = size;
  new_node->next = NULL;

  // Add the block of memory to node_list
  Node *node = new_node;
  node->ptr = free_list->ptr;

  // Update free_list to point to the next block of memory
  free_list->size -= size;
  free_list->ptr += size;

  // Add the block of memory to node_list
  node->next = node_list;
  node_list = node;

  printf("Allocated %d bytes at %p\n", size, node->ptr);
  return (char *) node->ptr;
}

/**
 * Deallocate a block of memory that was previously allocated using
 * alloc(). If ptr is NULL, no operation is performed.
 * @param ptr Pointer to the block of memory to deallocate.
 */
void dealloc(char *ptr) {
  if (ptr == NULL) { // Check if ptr is NULL
    printf("Pointer is NULL\n");
    return;
  }

  // Check if the memory manager has been initialized
  if (free_list == NULL || node_list == NULL) {
    printf("Memory manager has not been initialized\n");
    return;
  }

  Node *current = node_list;
  Node *previous = NULL;

  // Check if ptr is a valid pointer to a block of memory
  while (current != NULL && current->ptr != ptr) {
    previous = current;
    current = current->next;
  }

  // Check if ptr is a valid pointer to a block of memory
  if (current == NULL || current->ptr != ptr) {
    printf("Invalid pointer\n");
    return;
  }

  // Deallocate the block of memory from node_list
  if (previous == NULL) node_list = current->next;
  else previous->next = current->next;

  // Add the block of memory back to the free_list
  current->next = free_list;
  free_list = current;

  printf("Deallocated %d bytes at %p\n", current->size, current->ptr);

  // Sort free_list by the address of the block of memory
  current = free_list;
  while (current != NULL && current->next != NULL) {
    if (current->ptr > current->next->ptr) {
      Node *temp = current->next;
      current->next = current->next->next;
      temp->next = current;
      free_list = temp;
    } else current = current->next;
  }

  // Merge adjacent blocks of memory in free_list
  current = free_list;
  while (current != NULL && current->next != NULL) {
    if (current->ptr + current->size == current->next->ptr) {
      current->size += current->next->size;
      current->next = current->next->next;
    } else current = current->next;
  }
  printf("Merged %d bytes at %p\n", free_list->size, free_list->ptr);
}
