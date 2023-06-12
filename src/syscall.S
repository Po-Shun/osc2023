#include "syscall.h"

.global getpid
getpid:
  mov w8, #SYSCALL_GETPID
  svc #0
  ret

.global uart_read
uart_read:
  mov w8, #SYSCALL_UART_READ
  svc #0
  ret

.global uart_write
uart_write:
  mov w8, #SYSCALL_UART_WRITE
  svc #0
  ret

.global exec
exec:
  mov w8, #SYSCALL_EXEC
  svc #0
  ret

.global fork
fork:
  mov w8, #SYSCALL_FORK
  svc #0
  ret

.global exit
exit:
  mov w8, #SYSCALL_EXIT
  svc #0
  ret

.global mbox_call
mbox_call:
  mov w8, #SYSCALL_MBOX_CALL
  svc #0
  ret

.global kill
kill:
  mov w8, #SYSCALL_KILL
  svc #0
  ret