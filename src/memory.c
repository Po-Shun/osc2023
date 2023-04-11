#include "memory.h"
#include "uart.h"

typedef struct framerray_entry framerray_entry;
static framerray_entry frame_array[TOTAL_FRAME];
static int frame_list[3];
static int max_size = 0;

int pow(int base, int pow) {
    int ret = 1;
    for (int i=0; i<pow; i++) {
        ret *= base;
    }
    return ret;
}

int get_required_order(unsigned int size){
   int ret;
   int cmp = MEM_PAGE_SIZE;
   while(1){
      if(cmp >= size) break;
      cmp *= 2;
      ret ++;
   }
   return ret;
}

int get_free_frame_form_fram_list(int order){
  int t = order;
  while(1){
    if(frame_list[t] != 0) break;
    t ++;
  }
  return t;
}

void* malloc(unsigned int size){
  if(size > max_size){
    printf("[ERROR] Requested size exceed the maximum size of block %d \n\r", (int) max_size);
    return 0;
  }
  
  int order = get_required_order(size);

  int free_idx = get_free_frame_form_fram_list(order);
  

}


void init_memory(){
  // initialize the frame array
  int n_frame_block = pow(2, MAX_BLOCK_SIZE_ORDER);

  for(int i = 0; i < TOTAL_FRAME; i++){
    if(i % n_frame_block == 0){
      frame_array[i].index = i;
      frame_array[i].size = MAX_BLOCK_SIZE_ORDER;
      frame_array[i].val = ALLOCABLE;
    }
    else{
      frame_array[i].index = i;
      frame_array[i].size = -1;
      frame_array[i].val = CONTINIOUS;
    }
  }
  frame_list[MAX_BLOCK_SIZE_ORDER - 1] = 0;
  max_size = MEM_PAGE_SIZE * pow(2, MAX_BLOCK_SIZE_ORDER - 1);
}