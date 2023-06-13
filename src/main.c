#include "uart.h"
#include "command.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "allocate.h"
#include "fdt.h"
#include "timer.h"
#include "memory.h"
#include "scheduler.h"
#include "task.h"
#include "cpio.h"
#include "syscall.h"
#define RECV_LEN 100
#define USER_STACK_TOP 0x40000
static char recv_buf[RECV_LEN] = {0};
static int shared = 1;
void timer_print1_callback(char* msg){
  unsigned int sec;
  volatile unsigned int cntpct_el0;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cntpct_el0));
  volatile unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (cntfrq_el0));
  
  sec = cntpct_el0 / cntfrq_el0;
  uart_puts("Second after boot: ");
  uart_hex(sec);
  uart_puts("\n\r");
  uart_puts(msg);
}

void foo() {
//  printf("FOO start: %d\n", task_queue->cur->self->pid);
  for(int i = 0; i < 10; ++i) {
      // printf("TASK_ID: %d, I: %d\n", task_queue->cur->self->pid, i);
      // delay(1000000);
      select_task();
  }
  // printf("FOO STOP: %d\n", task_queue->cur->self->pid);
  mark_task_as_stop(task_queue->cur);
  // printf("FOO state: %d, %d\n", task_queue->cur->self->pid, task_queue->cur->self->state);
  while(1){
    asm volatile("nop");
  }
}


void user_foo() {
 
   printf("User thread id: %d\n", getpid());

    volatile unsigned int __attribute__((aligned(16))) mailbox[7];
    mailbox[0] = 7 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_BOARD_REVISION;
    mailbox[3] = 4;
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;
    mailbox[6] = END_TAG;
    // printf("SSS: %x\n", mailbox);
    mbox_call(0x8, mailbox);
    printf("Board Revision:\t\t%x\n", mailbox[5]);

    int pid = fork();
    if (pid == 0) {
        printf("Child says hello!\n");
        while(1) {
            // printf("Please don't kill me :(\n");
            shared++;
        }
    } else if (pid > 0) {
        printf("Parent says, \"My child has pid %d\"\n", pid);
        printf("Shared? %d\n", shared);
        for(int i = 0; i< 10; i++){
          delay(10000000);
        }
        printf("Kill my own child :(\n");
        kill(pid);
        delay(10000000);
        printf("shared %d\n", shared);
    }

    //char buf[4] = {0};
    //uart_read(buf, 3);
    //uart_write(buf, 3);

    exit();

}


void video_player(){
  
  exec("syscall.img", 0);
}
void shell(){
  enable_preempt();
  while(1){
    uart_getline(recv_buf, RECV_LEN);
    if(strcmp(recv_buf, "ls") == 0){
      uart_puts("\r");
      ls_cmd();
    }
    else if(strcmp(recv_buf, "hello") == 0){
      uart_puts("\rHELLO WORLD\n\r");
    }
    else if(strcmp(recv_buf, "async") == 0){
      uart_puts("\r");
      async_test();
    }
    else if(strcmp(recv_buf, "timer") == 0){
      uart_puts("\n");
      time_multiplex_test();
    }
    else if(strcmp(recv_buf, "reboot") == 0){
      uart_puts("\rReboot\n\r");
      reset(100);
    }
    else if(startwith(recv_buf, "set \"") == 0){
      uart_puts("CMD: settimeout\n\r");
      int msg_start = find_in_str(recv_buf, '"');
      int msg_end = msg_start + 1 + find_in_str(&recv_buf[msg_start + 1], '"');
      int int_start = msg_end + 2;
      int int_end = find_in_str(recv_buf, '\0') - 1;
      unsigned int size = msg_end - msg_start + 3;
      size += (size % 16);
      char*  msg = (char*) simple_malloc(32);
      msg_start ++;
      msg_end --;
      for(int i = 0; i < (msg_end - msg_start + 2); i++){
        if(i != (msg_end - msg_start + 1))
          *(msg + i) = recv_buf[msg_start + i];
        else{
          *(msg + i) = '\n';
          *(msg + i + 1) = '\r';
        }

      }
      //extract timout value
      unsigned long timeout = 0;
      for(int i=int_start; i <= int_end; i++){
        if(recv_buf[i] >= '0' && recv_buf[i] <= '9'){
          timeout *= 10;
          timeout += (recv_buf[i] - '0');
        }
        else{
          uart_puts("Timeout number error\n\r");
          continue;
        }
      }
      timeout = get_cpu_freq() * timeout;
      add_timer(timer_print1_callback, msg, timeout);
    }
    else if(strcmp(recv_buf, "page_allocate") == 0){
      printf("testcase page allocate start\r\n");
      void* temp = page_malloc(8);
      void* temp2 = page_malloc(8);
      page_free(temp);
      void* temp3 = page_malloc(8);
      void* temp4 = page_malloc(8);
      page_free(temp2);
      temp = page_malloc(4096* 3);
      page_free(temp3);
      page_free(temp4);
      page_free(temp);
      printf("testcase page allocate end\r\n");
    }
    else if(strcmp(recv_buf, "chunk_allocate") == 0){
      printf("testcase chunk allocate start\r\n");
      void* temp = malloc(8);
      void* temp2 = malloc(256);
      free(temp);
      free(temp2);
      printf("testcase chunk allocate end\n\r");
    }
    else if(strcmp(recv_buf, "test1") == 0){
      printf("test_start\n");
      for(int i = 0; i < 10; i++){
        Thread(&foo);
      }
      printf("test_end\n");
    }
    else if(strcmp(recv_buf, "test2") == 0){
      printf("test_start\n");
      user_thread(&user_foo);
      // user_foo();
      printf("test_end\n");
    }
    else if(strcmp(recv_buf, "test") == 0){
      printf("test_start\n");
      unsigned long long tmp;
      asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
      tmp |= 1;
      asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
      user_thread(&video_player);
      printf("test_end\n");
    }
    else if(strcmp(recv_buf, "help") == 0){
      uart_puts("help:\t\tlist available command\n\r");
      uart_puts("ls:\t\tlist initramfs files\n\r");
      uart_puts("cat:\t\tshow the file content\n\r");
      uart_puts("hello:\t\tprint \"hello world\"\n\r");
      uart_puts("exec:\t\tececute the user program\n\r");
      uart_puts("async:\t\ttestcase for async uart\n\r");
      uart_puts("timer:\t\ttestcase for timer multiplexing\n\r");
      uart_puts("page_allocate:\t\ttestcae for page allocator\n\r");
      uart_puts("reboot:\t\treboot rpi\n\r");
    }
    else {
      uart_puts("\rUknown cmd\n\r");
    }
  }
}


void main(){
  unsigned long DTB_BASE;
  asm volatile("mov %0, x20"
              :"=r"(DTB_BASE)
              );
  // set up serial console
    uart_init();
    fdt_traverse((fdt_header*)DTB_BASE, initramfs_callback);
    // fdt_traverse((fdt_header*)DTB_BASE, initramfs_end_callback);
    fdt_reserve_memory((fdt_header*) DTB_BASE);
    initramfs_reserve_memory((fdt_header*) DTB_BASE);

    // asm volatile("msr DAIFClr, 0xf");
    uart_puts("CPIO_BASE: ");
    uart_hex(CPIO_BASE);
    uart_puts("\n\r");
    
    // say hello
    uart_puts("\r\n\t\tWelcome NYCU OSC 2023!\r\n");
    // uart_puts("Hardware Info:\r\n\t");
    // unsigned int revision, base, size;
    // get_board_revision(&revision);
    // uart_puts("Board Revision: ");
    // uart_hex(revision);
    // uart_puts("\r\n\tARM MEMORY BASE: ");
    // get_arm_memory(&base, &size);
    // uart_hex(base);
    // uart_puts("\r\n\tARM MEMORY SIZE: ");
    // uart_hex(size);
    // uart_puts("\r\n");
    init_memory_reserve();
    init_task_queue();
    Thread(&idle);
    // for(int i = 0; i < 10; i++){
    //     Thread(&foo);
    // }
    // Thread(&shell);
    unsigned long long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    user_thread(&video_player);
    core_timer_enable();
    // shell();
}