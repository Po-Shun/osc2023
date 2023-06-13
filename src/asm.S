#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define THREAD_CPU_CONTEXT 0
.global from_el1_to_el0
from_el1_to_el0:
    //enable irq interrupt bit7 irq interrupt mask set to 0
    mov x2, 0x340
    // TODO WHY 0x3c0
    msr spsr_el1, x2
    // elr_el1 -> program start addr
    msr elr_el1, x0
    // sp_el0 -> usr prog stack addr
    msr sp_el0, x1
    eret

.global core_timer_enable
core_timer_enable:
  mov x0, 1
  msr cntp_ctl_el0, x0 // enable
  // Clock frequency. Indicates the system counter clock frequency, in Hz.
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0 // set expired time
  mov x0, 2
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

core_timer_handler:
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0

.global two_sec_interrupt
two_sec_interrupt:
    mrs x0, cntfrq_el0
    mov x1, 2
    mul x0, x0, x1
    msr cntp_tval_el0, x0
    ret

.global set_timer_expire
set_timer_expire:
    msr cntp_tval_el0, x0
    ret

.global context_switch
context_switch:
  mov x9, x0
  mov x10, sp
  mrs x11, spsr_el1
  mrs x12, elr_el1
  mrs x13, esr_el1
  mrs x14, sp_el0
  stp x19, x20, [x9, 16 * 0]
  stp x21, x22, [x9, 16 * 1]
  stp x23, x24, [x9, 16 * 2]
  stp x25, x26, [x9, 16 * 3]
  stp x27, x28, [x9, 16 * 4]
  stp fp, lr, [x9, 16 * 5]
  stp x10, x11, [x9, 16 * 6]
  stp x12, x13, [x9, 16 * 7]
  str x14, [x9, 16 * 8]

  mov x9, x1
  ldp x19, x20, [x9, 16 * 0]
  ldp x21, x22, [x9, 16 * 1]
  ldp x23, x24, [x9, 16 * 2]
  ldp x25, x26, [x9, 16 * 3]
  ldp x27, x28, [x9, 16 * 4]
  ldp fp, lr, [x9, 16 * 5]
  ldp x10, x11, [x9, 16 * 6]
  ldp x12, x13, [x9, 16 * 7]
  ldr x14, [x9, 16 * 8]
  mov sp, x10
  msr spsr_el1, x11
  msr elr_el1, x12
  msr esr_el1, x13
  msr sp_el0, x14
  ret

.global context_switch1 
context_switch1:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9
    ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret