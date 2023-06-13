#ifndef TASK_H
#define TASK_H
#include "task_base.h"
void create_idle();
int Thread(unsigned long func);
extern void ret_from_fork();
TASK* create_fork_task(unsigned long func);
int user_thread(unsigned long func);
#endif