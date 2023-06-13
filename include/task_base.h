#ifndef TASK_BASE_H
#define TASK_BASE_H


#define TASK_TYPE_THREAD 0x01
#define TASK_TYPE_FORK 0x02

#define TASK_RUNNING 0x00
#define TASK_STOP 0x01
#define TASK_BOUND -1

struct cpu_context{
    // unsigned long x19;
    // unsigned long x20;
    // unsigned long x21;
    // unsigned long x22;
    // unsigned long x23;
    // unsigned long x24;
    // unsigned long x25;
    // unsigned long x26;
    // unsigned long x27;
    // unsigned long x28;
    // unsigned long fp; 
    // unsigned long lr; 
    // unsigned long sp;
    
    unsigned long long x19;
    unsigned long long x20;
    unsigned long long x21;
    unsigned long long x22;
    unsigned long long x23;
    unsigned long long x24;
    unsigned long long x25;
    unsigned long long x26;
    unsigned long long x27;
    unsigned long long x28;
    unsigned long long fp;
    unsigned long long lr;
    unsigned long long sp;
    unsigned long long spsr_el1;
    unsigned long long elr_el1;
    unsigned long long esr_el1;
    unsigned long long sp_el0;
};

struct el0_regs{
  unsigned long long gen_reg[31];
  unsigned long long elr_el1;
  unsigned long long spsr_el1;
  unsigned long long padding;
};

typedef struct cpu_context cpu_context;
struct TASK
{
  struct cpu_context context;
  unsigned int type;
  unsigned int pid;
  void* page;
  void* user_stack;
  void* user_page;
  int state;
};
typedef struct TASK TASK;

// typedef struct reg_state {
//     unsigned long regs[31];
//     unsigned long sp;
//     unsigned long lr;
//     // unsigned long pstate;
// } reg_state;

#endif