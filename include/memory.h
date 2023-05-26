#ifndef _MEMORY_H
#define _MEMORY_H

#define MEM_PAGE_SIZE 4096
#define MEM_START 0x10000000
#define MEM_END 0x20000000
#define TOTAL_FRAME (MEM_END - MEM_START) / MEM_PAGE_SIZE
#define MAX_BLOCK_SIZE_ORDER 3 


#define MAX_POOL_NUM 8
#define MAX_POOL_PAGE 8

// definition for value for frame entry
#define ALLOCABLE 1
#define OCCUIPITED 2
#define CONTINIOUS 3

struct framearray_entry{
  unsigned int index;
  struct framearray_entry *prev;
  struct framearray_entry *next;
  int size;
  int val;
};

struct chunk{
  unsigned int page_id;
  struct chunk* next;
};

struct memory_pool{
  unsigned int chunk_size;
  unsigned int page_used;
  unsigned int chunk_allocate[MAX_POOL_PAGE];
  unsigned int chunk_pre_page;
  unsigned int chunk_pos[MAX_POOL_PAGE];
  void* page_ptr[MAX_POOL_PAGE];
  struct chunk* free_chunk[MAX_POOL_PAGE];
};

void init_memory();
void* page_malloc(unsigned int);
void page_free(void* address);

void* malloc(unsigned int);
void free(void* address);

#endif