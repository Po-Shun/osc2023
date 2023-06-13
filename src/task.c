#include "task.h"
#include "memory.h"
#include "uart.h"
#include "scheduler.h"
#include "timer.h"

static int id = 0;
int Thread(unsigned long func){
  disable_preempt();
  TASK* task;
  task = (TASK*)malloc(sizeof(TASK));
  printf("CREATE: %x\n", task);
  task->pid = id++; 
  task->state = TASK_RUNNING;
  task->page = page_malloc(MEM_PAGE_SIZE);
  task->context.lr = (unsigned long) func;
  task->context.sp = (unsigned long)task->page + MEM_PAGE_SIZE;

  task_push(task);
  enable_preempt();
  return 0;

}

void move_to_el0(){
  disable_irq();

  printf("MOVE TO EL0\n");
  // struct el0_regs* reg = (struct el0_regs*)(task_queue->cur->self->page + (MEM_PAGE_SIZE) - sizeof(struct el0_regs));
  unsigned long spsr_el1 = task_queue->cur->self->context.spsr_el1;
  unsigned long elr_el1 = task_queue->cur->self->context.elr_el1;
  task_queue->cur->self->user_stack = page_malloc(4*MEM_PAGE_SIZE);
  unsigned long spel0 = task_queue->cur->self->user_stack + (4 * MEM_PAGE_SIZE);
  printf("SPEL9: %x\n", spel0);
  // task_queue->cur->self->context.spsr_el1 = 0;
  // task_queue->cur->self->context.elr_el1 = 0;
  printf("spsr_el1 %x, elr_el1 %x\n", spsr_el1, elr_el1 );
  asm volatile( 
                "msr spsr_el1, %0\n\t"
                "msr elr_el1, %1\n\t"
                "msr sp_el0, %2\n\t"
                "mov sp, %3\n\t" // clear kernel stack
                "eret"
               ::
                "r" (spsr_el1),
                "r" (elr_el1),
                // "r" (task_queue->cur->self->page + MEM_PAGE_SIZE));
                "r" (spel0),
                "r" (task_queue->cur->self->page + MEM_PAGE_SIZE));
}

int user_thread(unsigned long func){
  disable_preempt();
  TASK* task = (TASK*)malloc(sizeof(TASK));
  task->pid = id ++;
  task->state = TASK_RUNNING;
  task->page = page_malloc(MEM_PAGE_SIZE);
  task->context.lr = &move_to_el0;
  // task->context.sp = (unsigned long)task->page + (MEM_PAGE_SIZE) - sizeof(struct el0_regs);
  task->context.sp = (unsigned long)task->page + (MEM_PAGE_SIZE);
  unsigned long spsr_el1 = 0x340;
  unsigned long esr_el1 = func;
  // unsigned long spel0 = (unsigned long)task->page + MEM_PAGE_SIZE - sizeof(struct el0_regs);
  task->context.elr_el1 = func;
  task->context.spsr_el1 = spsr_el1;
  // struct el0_regs* regs = (struct el0_regs*)(task->page +(MEM_PAGE_SIZE)- sizeof(struct el0_regs));
  // regs->spsr_el1 = spsr_el1;
  // regs->elr_el1 = func;
  printf("USER thread: %x\n", task->page);
  printf("spsr_el1 %x, elr_el1 %x\n", spsr_el1, func);
  task_push(task);
  enable_preempt();
  return 0;
}

TASK* create_fork_task(unsigned long func){
  TASK* task;
  task = (TASK*) malloc(sizeof(TASK));
  task->pid = id++;
  task->state = TASK_RUNNING;
  task->page = page_malloc(MEM_PAGE_SIZE);
  task->user_stack = page_malloc(4*MEM_PAGE_SIZE);
  task->context.lr = (unsigned long) func;
  task->context.sp = task->page + MEM_PAGE_SIZE;
  task_push(task);
  return task;
}