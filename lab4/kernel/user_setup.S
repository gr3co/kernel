@ user_setup.S: helper function written in asm called from kernel
@
@ Author: Ramsey Natour <rnatour@andrew.cmu.edu>
@ Date:   November 10, 2014
@
@ Hint:
@   CPSR:
@       N|Z|C|V| [27:9] |I|F|T|M[4:0]
@   I -> 1
@   F -> 1
@   T -> T
@   CPSR_SVC = 0x10011 -> CPSR_USER = 0x10000

    .extern user_setup_stack_ptr

    .file   "user_setup.S"
    .text

    .global user_setup
    
user_setup:
    @ Save gp registers, lr, and cpsr so that we can return to kernel main later.
    stmfd   sp!, {r1-r12, lr}
    mrs     r2, cpsr
    stmfd   sp!, {r2}

    @ change mode to USER
    mrs     r2, cpsr
    orr     r2, r2, #0x040  @ Mask the FIQ bit.
    bic     r2, r2, #0x080  @ Unmask the IRQ bit.
    bic     r2, r2, #0x003  @ Change mode bits to user.
    msr     cpsr, r2

    @ set up user stack from spTop passed in from kernel.c
    mov     sp, r0

    @load global_data back into r8
    @ldr     r8, =global_data
    @ldr     r8, [r8]

    @ go to user program
    ldr     pc, =0xa0000000

    @@@ exit syscall essentially returns through here!
