#include "scheduler.h"
#include "memory.h"
#include "uart.h"
#include "timer.h"

// static struct TASK_QUEUE task_queue;

static TASK_QUEUE ready_queue;
static int Preemptable = 1;
TASK_QUEUE* task_queue = &ready_queue;
int* preemptable = &Preemptable; 

void enable_preempt() { Preemptable = 1; }

void disable_preempt() { Preemptable = 0; }

void init_task_queue(){
  task_queue->head.prev = 0;
  task_queue->head.next = &task_queue->tail;
  task_queue->head.self->state = TASK_BOUND;
  task_queue->tail.prev = &task_queue->head;
  task_queue->tail.next = 0;
  task_queue->tail.self->state = TASK_BOUND;

  task_queue->total_tasks = 0;
  task_queue->cur = &task_queue->head;

}

void task_push(TASK* task){
  TASK_NODE* new_task = (TASK_NODE*)(malloc(sizeof(TASK_NODE)) + MEM_PAGE_SIZE);
  new_task->self = task;
  task_queue->tail.prev->next = new_task;
  new_task->prev = task_queue->tail.prev;
  task_queue->tail.prev = new_task;
  new_task->next = &task_queue->tail;
  // task_queue->head.next->prev = new_task;
  // task_queue->head.next = new_task;
  task_queue->total_tasks ++;
  // TASK_NODE* a = task_queue->head.next;
  // printf("LIST ALL ITEM\n");
  // printf("HEAD : %x, tail : %x\n", &task_queue->head, &task_queue->tail);
  // while(a != &task_queue->tail){
  //   printf("TASK_ID: %d\n", a->self->pid);
  //   printf("POS: %x\n", a->self);
  //   a = a->next;
  // }
  // printf("TASK #: %d\n", task_queue->total_tasks); 
}

void mark_task_as_stop(TASK_NODE* task_node){
  disable_preempt();
  TASK_NODE* prev = task_node->prev;
  TASK_NODE* next = task_node->next;
  prev->next = next;
  next->prev = prev;
  prev = task_queue->tail.prev;
  next = &task_queue->tail;
  prev->next = task_node;
  next->prev = task_node;
  task_node->next = next;
  task_node->prev = prev;
  task_node->self->state = TASK_STOP;
  enable_preempt();
}

void task_pop(TASK_NODE* task_node){
  disable_preempt();
  printf("POP TASK: %d\n", task_node->self->pid);
  TASK_NODE* prev = task_node->prev;
  TASK_NODE* next = task_node->next;
  prev->next = next;
  next->prev = prev;
  page_free(task_node->self->page);
  free(task_node->self);
  enable_preempt();
}

void killZombies(){
  // printf("TASK #: %x\n",task_queue->total_tasks);
  disable_preempt();
  while(1){
    if(task_queue->tail.prev != &task_queue->head && task_queue->tail.prev->self->state == TASK_STOP){
      // printf("FIND POP TASK %d\n",task_queue->cur->self->pid);
      task_pop(task_queue->tail.prev);
      task_queue->total_tasks --;
    }
    else{
      break;
    }
  }
  enable_preempt();
}

void idle(){
  printf("IDLE THREAD START\n");
  core_timer_enable();
  while(1){
    killZombies();
    select_task();
  }
}
static int count = 0;
void select_task(){
  // printf("ENTER\n");
  // if(task_queue->cur == &task_queue->head){
  //   printf("SELECT FIRST JOB\n");
  // }else {
  //   printf("SELECT_TASK_START current : %d\n", task_queue->cur->self->pid);
  // }
  disable_preempt();
  int flag = 0;
  TASK_NODE* cur = task_queue->cur;
  TASK_NODE* select_job = 0;
  // printf("CUR: %x\n", cur->self);
  while(cur != task_queue->cur || flag != 1){
    if(flag == 0) flag =1;
    cur = cur->next;
    if(cur == &task_queue->tail){
      cur = task_queue->head.next; 
    }
    if(cur != &task_queue->head && cur != 0 && cur->self->state == TASK_RUNNING){
      // printf("SELECT A JOB %x\n", cur->self);
      select_job = cur;
      break;
    }
  }
  // printf("AFTER SELECT\n");
  if(select_job != 0){
    // printf("Context start\n");
    // if(task_queue->cur == &task_queue->head){
    //   printf("PREV ID NONE\n");
    // }
    // else{
    //   printf("PREV ID: %d\n", task_queue->cur->self->pid);
    // }
    // printf("SWITHC to id : %d\n", select_job->self->pid);
    // printf("%x, %x\n", task_queue->cur->self, select_job->self->context.lr);
    // context_switch(task_queue->cur->self, select_job->self);
    TASK* old;
    if(task_queue->cur == &task_queue->head){
      old = 0;
    }
    else{
      old = task_queue->cur->self;
    }
    task_queue->cur = select_job;
    // printf("%x, %x, %x, %d\n", old->pid, task_queue->cur->self->pid, taskqemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s_queue->cur->self->context.lr, count ++);
    context_switch(old, task_queue->cur->self);
  }
  enable_preempt();

}