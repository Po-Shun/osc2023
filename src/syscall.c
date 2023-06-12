#include "syscall.h"
#include "scheduler.h"
#include "mailbox.h"
#include "memory.h"
#include "uart.h"
#include "fdt.h"
#include "timer.h"
#include "cpio.h"
#include "task.h"
#include "string.h"

int sys_getpid(){
  return task_queue->cur->self->pid;
}

unsigned int sys_uart_read(char buf[], unsigned int size){
  for(unsigned int i = 0; i < size; i++){
    buf[i] = uart_getc();
  }
  return size;
}

unsigned int sys_uart_write(const char buf[], unsigned int size){
  for(unsigned int i = 0; i < size; i++){
    uart_send(buf[i]);
  }
  return size;
}

int sys_exec(const char* name, char *const argv[]){
  struct cpio_newc_header *pos = (struct cpio_noewc_header*) CPIO_BASE;
  char* current = (char*) CPIO_BASE;
  int filesize = 0;
  while(1){
    pos = (struct cpio_newc_header*)current;
    int namesize = hexstr_to_uint(pos->c_namesize, 8);
    filesize = hexstr_to_uint(pos->c_filesize, 8);
    current += sizeof(struct cpio_newc_header);
    printf("%s\n", current);
    if(strcmp(current, "TRAILER!!!") == 0) {
      uart_puts("\r");
      uart_puts("File Not Found\n\r");
      break;
    }
    int load_flag = 0;
    if(strcmp(current, name) == 0){
      load_flag = 1;
    }
    current += namesize;
    int total_size = current - (char*) pos;
    int reminder = total_size % 4;
    if(reminder != 0){
      current  += (4 - reminder);
    }
    if(load_flag == 1){
      break;
    }
    current += filesize;
    total_size = current - (char*) pos;
    reminder = total_size % 4;
    if(reminder != 0){
      current += (4 - reminder);
    }
  }

  task_queue->cur->self->user_page = page_malloc((unsigned int)filesize);
  // 2 ^ order 
  unsigned int order = (1 << get_required_order(filesize));
  asm volatile("msr sp_el0, %0" :: "r"(task_queue->cur->self->user_stack + (order * MEM_PAGE_SIZE)));
  // copy the executed binary into memory
  for(int i = 0 ; i < filesize; i++){
    ((char*)task_queue->cur->self->user_page)[i] = ((char*)current)[i];
  }

  // reset trapframe 
  struct el0_regs* regs = (task_queue->cur->self->page + ( MEM_PAGE_SIZE) - sizeof(struct el0_regs));
  for(int i = 0; i < 31; i++){
    regs->gen_reg[i] = 0;
  }
  regs->elr_el1 = (unsigned long)task_queue->cur->self->user_page;
  printf("start exec, %x\n", regs->elr_el1);
  return 0;
}

void forkprocedure(){
  enable_preempt();

  // printf("-------GO in fork pro\n");
  struct el0_regs* trap_frame = (struct el0_regs*)(task_queue->cur->self->page + (MEM_PAGE_SIZE) - sizeof(struct el0_regs));
  trap_frame->gen_reg[0] = 0;
  // printf("[FORK PRO] trap_frame %x\n", trap_frame);
  // for(int i = 0; i < 31; i++){
  //   printf("%x ", trap_frame->gen_reg[i]);
  // }
  // printf("%x ", trap_frame->elr_el1);
  // printf("%x\n", trap_frame->spsr_el1);
  disable_irq();
  asm volatile("mov sp, %0\n\t"
               "b exit_kernel" :: "r"(trap_frame));
}

// todo 
int sys_fork(){
  disable_preempt();
  // printf("*****FORK stark\n");
  TASK* task = create_fork_task(&forkprocedure);
  unsigned long long spel0, elr_el1;
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  // printf("ELR_EL1: %x\n", elr_el1);
  task->context.spsr_el1 = 0x340;
  task->context.elr_el1 = elr_el1;
  task->context.sp_el0 = task->user_stack + (4*MEM_PAGE_SIZE);
  // task->context.lr = elr_el1;
  task->context.sp = (unsigned long)task->page +  (MEM_PAGE_SIZE) - sizeof(struct el0_regs);
  // task->context.sp_el0 = (unsigned long)task->page +  (4 * MEM_PAGE_SIZE) - sizeof(struct el0_regs);
  // task->context.sp = (unsigned long)task->page + sp_offset;

  struct el0_regs* cur = (struct el0_regs*)(task_queue->cur->self->page +  (MEM_PAGE_SIZE) - sizeof(struct el0_regs));
  // for(int i = 0; i < 31; i++){
  //   printf("%x ", cur->gen_reg[i]);
  // }
  // printf("%x ", cur->elr_el1);
  // printf("%x\n", cur->spsr_el1);
  struct el0_regs* new = (struct el0_regs*)(task->page + (MEM_PAGE_SIZE) - sizeof(struct el0_regs));
  memcpy(new, cur, sizeof(struct el0_regs));
  // printf("NEW %x\n", new);
  // for(int i = 0; i < 31; i++){
  //   printf("%x ", new->gen_reg[i]);
  // }
  // printf("%x ", new->elr_el1);
  // printf("%x\n", new->spsr_el1);
  // printf("FORK ELR_EL1: %x, prece: %x, elr_el1: %x\n", new->elr_el1, &forkprocedure, elr_el1);
  // printf("CUR : %x\n", cur);
  enable_preempt();
  return task->pid;
}

void sys_exit(int status){
  printf("PID %d EXIT\n", task_queue->cur->self->pid);
  task_queue->cur->self->state = TASK_STOP;
  select_task();
  while(1) asm volatile("nop");
}


int sys_mbox_call(unsigned char ch, unsigned int *mbox){
  unsigned long r = ((unsigned long)mbox & 0xFFFFFFF0) | (ch & 0xF);
  while ((*MAILBOX_STATUS) & MAILBOX_FULL){
    asm volatile("nop");
  }
  *MAILBOX_WRITE = r;
  while (1) {
    while ((*MAILBOX_STATUS) & MAILBOX_EMPTY){
      asm volatile("nop");
    }
    if (*MAILBOX_READ == r) {
      return mbox[1] == REQUEST_SUCCEED;
    }
  }
}

void sys_kill(int pid){
  printf("KILL START\n");
  TASK_NODE* t = task_queue->head.next;

  while(t != &task_queue->tail){
    if(t->self->pid == pid){
      mark_task_as_stop(t);
      break;
    }
    t = t->next;
  }
}

void* const sys_call_table[] = {
  sys_getpid,
  sys_uart_read,
  sys_uart_write,
  sys_exec,
  sys_fork,
  sys_exit,
  sys_mbox_call,
  sys_kill
};
