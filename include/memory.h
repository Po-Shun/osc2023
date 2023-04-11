#ifndef _MEMORY_H
#define _MEMORY_H

#define MEM_PAGE_SIZE 4096
#define MEM_START 0x10000000
#define MEM_END 0x20000000
#define TOTAL_FRAME (MEM_END - MEM_START) / MEM_PAGE_SIZE
#define MAX_BLOCK_SIZE_ORDER 3 


// definition for value for frame entry
#define ALLOCABLE 1
#define OCCUIPITED 2
#define CONTINIOUS 3

struct framerray_entry{
  unsigned int index;
  int size;
  int val;
};


void init_memory();
void* malloc(unsigned int);
#endif