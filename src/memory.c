#include "memory.h"
#include "uart.h"

typedef struct framearray_entry framearray_entry;
static framearray_entry frame_array[TOTAL_FRAME];
static struct memory_pool mpools[MAX_POOL_NUM] = {{0, 0, {0}, 0, {0}, {0}, {0}}};
static int frame_list[4];
static int max_size = 0;
static unsigned int reserve_pos = 0;
static unsigned int reserve_section[8][2] = {{0,0}};
extern char _end;
char* kernel_end = &_end;

int pow(int base, int pow) {
    int ret = 1;
    for (int i=0; i<pow; i++) {
        ret *= base;
    }
    return ret;
}

unsigned int get_required_order(unsigned int size){
   unsigned int ret = 0;
   unsigned int cmp = MEM_PAGE_SIZE;
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
    if(frame_list[t] >= 0) break;
    t ++;
  }
  return t;
}

void* page_malloc(unsigned int size){
  printf("-----------Page malloc-------------------\n");
  if(size > max_size){
    printf("[ERROR] Requested size exceed the maximum size of block %d \n\r", (int) max_size);
    return 0;
  }
  
  unsigned int order = get_required_order(size);
  printf("[DEBUG] Require order: %d\r\n", order);
  int free_order = get_free_frame_form_fram_list(order);
  printf("[DEBUG] Free order: %d\r\n", free_order);
  while(free_order != order){
    printf("[DEBUG] frame list %d\n", frame_list[free_order]);
    framearray_entry*  split_left = &frame_array[frame_list[free_order]];
    if(split_left->next != 0) {
      frame_list[free_order] = split_left->next->index;
      frame_array[frame_list[free_order]].prev = 0;
    }
    else frame_list[free_order] = -1;
    if (frame_list[free_order] != -1)
      printf("[DEBUG] Split at order %d, head is 0x%x now.\n", free_order, frame_list[free_order]);
    else
      printf("[DEBUG] Split at order %d, head is Null now.\n", free_order);

    unsigned int middle_offset = pow(2, split_left->size - 1);
    // split total 4kb * 2 ^ middle_offset 
    framearray_entry* split_right = &frame_array[split_left->index + middle_offset];

    // re-initialize the left block
    split_left->size --;
    split_left->val = ALLOCABLE;
    // the first element for order size - 1
    split_left->prev = 0;
    split_left->next = split_right;

    split_right->size = split_left->size;
    split_right->val = ALLOCABLE;
    split_right->prev = split_left;
    split_right->next = 0;

    free_order --;
    frame_list[free_order] = split_left->index;
  }


  int ret_index = frame_list[order];
  framearray_entry* temp = frame_array[ret_index].next;
  if(temp != 0){
    printf("T\n");
    frame_list[order] = temp->index;
    temp->prev = 0;
  }
  else{
    printf("F\n");
    frame_list[order] = -1;
  }
  

  framearray_entry* ret = &frame_array[ret_index];

  ret->val = OCCUIPITED;
  ret->prev = 0;
  ret->next = 0;
  for (int i=0; i <= MAX_BLOCK_SIZE_ORDER; i++) {
    if (frame_list[i] != -1)
      printf("[DEBUG] Head of order %d has frame array index %d.\n",i,frame_list[i]);
    else
      printf("[DEBUG] Head of order %d has frame array index null.\n",i);
  }
  return (void*)MEM_START+(MEM_PAGE_SIZE*ret->index);

}

void page_free(void* address){
  unsigned int page_num = ((unsigned int)address- MEM_START) / MEM_PAGE_SIZE;
  framearray_entry* target = &frame_array[page_num];
  printf("---------page free start----------\n");
  printf("[DEBUG] Now freeing address 0x%x with frame index %d.\n", address, (int)page_num);
  for(int i = target->size; i <= MAX_BLOCK_SIZE_ORDER; i++){
    // find the buddy 
    unsigned int buddy = page_num ^ pow(2, i);
    framearray_entry* frame_buddy = &frame_array[buddy];
    printf("[DEBUG] Index %d at order %d, buddy %d at order %d state %d.\n", 
                (int)page_num, (int)i, (int)buddy, frame_buddy->size, frame_buddy->val);
    if(i <= MAX_BLOCK_SIZE_ORDER - 1 && frame_buddy->val == ALLOCABLE && i == frame_buddy->size){
      printf("[DEBUG] Merging from order %d. Frame indices %d, %d.\n", i, (int)buddy, (int)page_num);
      if(frame_buddy->prev != 0){
        frame_buddy->prev->next = frame_buddy->next;
      }
      else{
        // frame_buddy has no prev means it is in the head of frame_list 
        if(frame_buddy->next != 0){
          frame_list[frame_buddy->size] = frame_buddy->next->index;
        }
        else{
          frame_list[frame_buddy->size] = -1;
        }
      }

      if(frame_buddy->next != 0){
        frame_buddy->next->prev = frame_buddy->prev;
      }

      frame_buddy->prev = 0;
      frame_buddy->next = 0;
      frame_buddy->val = CONTINIOUS;
      frame_buddy->size = -1;
      target->val = CONTINIOUS;

      if(frame_buddy->index < target->index){
        page_num = frame_buddy->index;
        target = frame_buddy;
      }

      for (int i=0; i <= MAX_BLOCK_SIZE_ORDER; i++) {
        if (frame_list[i] != -1)
          printf("[DEBUG] Head of order %d has frame array index %d.\n",i,frame_list[i]);
        else
          printf("[DEBUG] Head of order %d has frame array index null.\n",i);
      }
      printf("[DEBUG] Frame index of next merge target is %d.\n", (int)page_num);


    }
    else{
      target->size = i;
      target->val = ALLOCABLE;
      target->prev = 0;
      target->next = 0;
      printf("[DEBUG] frame list %d\n", frame_list[i]);
      printf("[DEBUG] target next %d\n", target->next->index);
      if(frame_list[i] != -1){
        target->next = &frame_array[frame_list[i]];
        frame_array[frame_list[i]].prev = target;
      }
      frame_list[i] = target->index;
      printf("[DEBUG] Frame index %d pushed to frame list of order %d\n", target->index, i);
      break;
    }
  }

  for (int i=0; i <= MAX_BLOCK_SIZE_ORDER; i++) {
    if (frame_list[i] != -1)
        printf("[DEBUG] Head of order %d has frame array index %d.\n",i,frame_list[i]);
    else
        printf("[DEBUG] Head of order %d has frame array index null.\n",i);
  }

}

int find_fit_chunk_size(unsigned int size){
  unsigned int fit_size = 0;
  if(size <= 16) fit_size = 16;
  else{
    int temp = size % 16;
    if(temp != 0) fit_size = (size /16 + 1) * 16;
    else fit_size = size;
  } 
  printf("[DEBUG] fit_size %d\n", fit_size);
  return fit_size;
}

void init_pool(struct memory_pool* pool, unsigned int size){
  pool->chunk_size = size;
  pool->chunk_pre_page = MEM_PAGE_SIZE / size;
  pool->page_used = 0;
  for(int i = 0; i < MAX_POOL_PAGE; i++){
    pool->chunk_allocate[i] = 0;
    pool->chunk_pos[i] = 0;
    pool->page_ptr[i] = 0;
    pool->free_chunk[i] = 0;
  }
}

int find_fit_pool_idx(unsigned int size){
  unsigned int fit_size = find_fit_chunk_size(size);

  if(fit_size >= MEM_PAGE_SIZE){
    printf("[ERROR] Request chunk size larger equal than page size\n");
    return -1;
  }

  for(int i = 0; i < MAX_POOL_NUM; i ++){
    if(mpools[i].chunk_size == fit_size) return i;
    else if(mpools[i].chunk_size == 0){
      init_pool(&mpools[i], fit_size);
      return i;
    }
  }
  return -1;
}

int find_free_chunk(struct memory_pool* pool){
  int ret = -1;
  for(int i = 0; i < MAX_POOL_PAGE; i++){
    if(pool->free_chunk[i] != 0){
      ret = i;
      break;
    }
  }
  return ret;
}

void* malloc(unsigned int size){
  printf("----------mallloc start------------\n");
  int pool_idx = find_fit_pool_idx(size);
  printf("[DEBUG] fit pool idx %d\n", pool_idx);

  struct memory_pool* target_pool = &mpools[pool_idx];
  int free_chunk_page = find_free_chunk(target_pool);
  if(free_chunk_page != -1){
    void* ret = (void*)target_pool->free_chunk[free_chunk_page];
    target_pool->chunk_allocate[free_chunk_page] ++;
    struct chunk* old_chunk = target_pool->free_chunk[free_chunk_page];
    target_pool->free_chunk[free_chunk_page] = target_pool->free_chunk[free_chunk_page]->next;
    old_chunk->next = 0;
  }

  if(target_pool->page_used >= MAX_POOL_PAGE && target_pool->chunk_allocate[target_pool->page_used - 1] >= target_pool->chunk_pre_page){
    printf("[DEBUG] reach pool maximum chunk\n");
    return 0;
  }

  // the allocated page are all full, request new page to allocate the chunk 
  if((target_pool->page_used > 0 && target_pool->chunk_allocate[target_pool->page_used - 1] >= target_pool->chunk_pre_page) || (target_pool->page_used == 0 && target_pool->chunk_allocate[0] == 0)){
    target_pool->page_ptr[target_pool->page_used] = page_malloc(MEM_PAGE_SIZE);
    printf("[DEBUG] Allocate new page for reqested chunk\n");
    target_pool->page_used ++;
    target_pool->chunk_pos[target_pool->page_used - 1] = 0; 
  }
  void *ret = target_pool->page_ptr[target_pool->page_used - 1] + target_pool->chunk_size * target_pool->chunk_pos[target_pool->page_used - 1];
  target_pool->chunk_pos[target_pool->page_used - 1] ++;
  target_pool->chunk_allocate[target_pool->page_used - 1] ++;

  printf("[DEGUG] allocate new chunk finish, pos %x\n", ret);
  return ret;
}

void free(void* address){
  printf("----------free start----------\n");
  void* prefix_addr0 = (void*)((unsigned long long)address);
  printf("[DEBUG] prefix_addr %x\n", prefix_addr0);
  void* prefix_addr = (void*)((unsigned long long)address & 0xfffff000);
  printf("[DEBUG] prefix_addr %x\n", prefix_addr);
  for(int i = 0; i < MAX_POOL_NUM; i++){
    for(int j = 0; j < mpools[i].page_used; j++){
      void *prefix_base_addr = (void*)((unsigned long long )mpools[i].page_ptr[j] & 0xfffff000);
      if(prefix_addr == prefix_base_addr){
        printf("[DEBUG] free chunk form pool idx %d\n", i);
        struct memory_pool* target_pool = &mpools[i];
        target_pool->chunk_allocate[j] --;
        int flag = target_pool->chunk_allocate[j];
        printf("[DEBUG] flag %d\n", flag);
        if(flag != 0){
          struct chunk* old_free_chunk = target_pool->free_chunk[j];
          target_pool->free_chunk[j] = (struct chunk*) address;
          target_pool->free_chunk[j]->next = old_free_chunk;
          target_pool->free_chunk[j]->page_id = j;
        }
        else{
          page_free(target_pool->page_ptr[j]);
          for(int pos = j + 1; pos < target_pool->page_used; pos ++){
            printf("[DEBUG] rebase pages\n");
            target_pool->chunk_allocate[pos - 1] = target_pool->chunk_allocate[pos]; 
            target_pool->chunk_pos[pos - 1]  = target_pool->chunk_pos[pos];
            target_pool->page_ptr[pos - 1] = target_pool->page_ptr[pos];
            target_pool->free_chunk[pos - 1] = target_pool->free_chunk[pos];
          }
          target_pool->page_used --;
          if(target_pool->page_used == 0){
            printf("[DEBUG] free unused mpool\n");
            target_pool->chunk_size = 0;
          }


        }
        return;
      }
    }
  }
  printf("[DEBUG] couldn't find the target\n");
}

unsigned int find_page_frame_index(void* pos){
  unsigned int ret = (int)((unsigned int)pos) / MEM_PAGE_SIZE;
  printf("IDX: 0x%x\n", ret);
  return ret;
}
void memory_reserve(void* start, void* end){
  reserve_section[reserve_pos][0] = find_page_frame_index(start);
  reserve_section[reserve_pos][1] = find_page_frame_index(end);
  reserve_pos ++;
}

void init_memory_reserve(){
  int n_frame_block = pow(2, MAX_BLOCK_SIZE_ORDER);
  // reserve kernel image 
  memory_reserve(0x0, (void*)kernel_end);

  //initialize frame list
  for(int i = 0; i < 3; i++){
    frame_list[i] = -1;
  }

  for(int i = 0; i < TOTAL_FRAME; i++){
    frame_array[i].index = i;
    frame_array[i].size = 0;
    frame_array[i].val = ALLOCABLE;
    frame_array[i].prev = 0;
    frame_array[i].next = 0;
  }

  // mark corresponding frames as reserved 
  for(int i = 0; i < reserve_pos; i++){
    int start = reserve_section[i][0];
    int end = reserve_section[i][1];
    for(int i = start; i <= end; i++){
      frame_array[i].val = RESERVED;
    }
  }

  for(unsigned int n = 0; n < TOTAL_FRAME; n++){
    framearray_entry* target = &frame_array[n];
    unsigned int idx = n;

    if(target->val == RESERVED || target->val == CONTINIOUS) continue;

    for(int i = target->size; i <= MAX_BLOCK_SIZE_ORDER; i++){
      unsigned int buddy = idx ^ pow(2, i);
      framearray_entry* frame_buddy = &frame_array[buddy];

      if( i < MAX_BLOCK_SIZE_ORDER && frame_buddy->val == ALLOCABLE && i == frame_buddy->size){
        if(frame_buddy->prev != 0){
          frame_buddy->prev->next = frame_buddy->next;
        }
        else{
          if(frame_buddy->next != 0){
            frame_list[frame_buddy->size] = frame_buddy->next->index;
          }
          else{
            frame_list[frame_buddy->size] = -1;
          }
        }

        if(frame_buddy->next != 0){
          frame_buddy->next->prev = frame_buddy->prev;
        }

        frame_buddy->prev = 0;
        frame_buddy->next = 0;
        frame_buddy->val = CONTINIOUS;
        frame_buddy->size = -1;
        target->val = CONTINIOUS;

        if(frame_buddy->index < target->index){
          idx = frame_buddy->index;
          target = frame_buddy;
        }
      }
      else{
        target->size = i;
        target->val = ALLOCABLE;
        target->prev = 0;
        target->next = 0;
        if(frame_list[i] != -1){
          target->next = &frame_array[frame_list[i]];
          frame_array[frame_list[i]].prev = target;
        }
        frame_list[i] = target->index;
        break;
      }
    }
  }

  for (int i=0; i <= MAX_BLOCK_SIZE_ORDER; i++) {
    if (frame_list[i] != -1)
        printf("[DEBUG] Head of order %d has frame array index %d.\n",i,frame_list[i]);
    else
        printf("[DEBUG] Head of order %d has frame array index null.\n",i);
  }
  max_size = MEM_PAGE_SIZE * pow(2, MAX_BLOCK_SIZE_ORDER - 1);

}

void init_memory(){
  
  // initialize the frame array
  int n_frame_block = pow(2, MAX_BLOCK_SIZE_ORDER);
  for(int i = 0; i < 3; i++){
    frame_list[i] = -1;
  }
  for(int i = 0; i < TOTAL_FRAME; i++){
    if(i % n_frame_block == 0){
      frame_array[i].index = i;
      frame_array[i].size = MAX_BLOCK_SIZE_ORDER;
      frame_array[i].val = ALLOCABLE;
      if(i == 0){
        frame_array[i].prev = 0;
      }
      else{
        frame_array[i].prev = &frame_array[i - n_frame_block];
      }
      if(i == (TOTAL_FRAME - n_frame_block)){
        frame_array[i].next = 0;
      }
      else{
        frame_array[i].next = &frame_array[i + n_frame_block];
      }
    }
    else{
      frame_array[i].index = i;
      frame_array[i].size = -1;
      frame_array[i].val = CONTINIOUS;
      frame_array[i].prev = 0;
      frame_array[i].next = 0;
    }
  }
  frame_list[MAX_BLOCK_SIZE_ORDER] = 0;
  max_size = MEM_PAGE_SIZE * pow(2, MAX_BLOCK_SIZE_ORDER - 1);
  printf("------init memory finish------\n");
}