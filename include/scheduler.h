#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task_base.h"





struct TASK_NODE{
  TASK* self;
  struct TASK_NODE* prev;
  struct TASK_NODE* next;
};

struct TASK_QUEUE{
  struct TASK_NODE head;
  struct TASK_NODE tail;
  struct TASK_NODE* cur;
  int total_tasks;
};

typedef struct TASK_NODE TASK_NODE;
typedef struct TASK_QUEUE TASK_QUEUE;

extern struct TASK_QUEUE* task_queue;
extern int* preemptable;
void init_task_queue();

void task_push(TASK* task);

//todo
void task_pop(TASK_NODE* task);

void select_task();

extern void context_switch(TASK* cur, TASK* next);
void start_scheduler();

void idle();
void enable_preempt();
void disable_preempt();
void mark_task_as_stop(TASK_NODE* task_node);



#endif