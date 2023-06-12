#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_GETPID        0
#define SYSCALL_UART_READ     1
#define SYSCALL_UART_WRITE    2
#define SYSCALL_EXEC          3
#define SYSCALL_FORK          4
#define SYSCALL_EXIT          5
#define SYSCALL_MBOX_CALL     6
#define SYSCALL_KILL          7


#ifndef __ASSEMBLER__
int sys_getpid();
unsigned int sys_uart_read(char buf[], unsigned int size);
unsigned int sys_uart_write(const char buf[], unsigned int size);
int sys_exec(const char* name, char *const argv[]);
int sys_fork();
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);
#endif

#endif