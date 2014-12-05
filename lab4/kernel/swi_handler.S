@ swi_handler.S: SWI Handler
@
@ Author: Ramsey Natour <rnatour@andrew.cmu.edu>
@ Date:   November 10, 2014
@
@ Hint: To change the SWI handler in class, we need to accommodate a return
@   value from the C_SWI_Handler.

    .extern C_SWI_Handler

    .file   "swi_handler.S"
    .text

    .global swi_handler
    .extern global_data
    
swi_handler:

    @ recover user register and save
    stmfd    sp, {sp, lr}^
    add      sp, sp, #-8

    sub     sp, sp, #4
    stmfd   sp!, {r0-r12, lr}
    @add     sp, sp, #-14*4

    ldr r8, =global_data
    ldr r8, [r8]

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    mrs     r2, cpsr
    bic     r2, r2, #0x080  @ Unmask the IRQ bit.
    msr     cpsr, r2


    @ Set Param 1: extract SWI and get SWI no. by bit-masking
    ldr     r0, [lr, #-4]
    bic     r0, r0, #0xff000000

    @ Set Param 2: pointer to parameters on stack
    mov     r1, sp

    @ go to handler
    bl      C_SWI_Handler

    mrs     r2, cpsr
    orr     r2, r2, #0x080  @ Unmask the IRQ bit.
    msr     cpsr, r2

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}
    @add     sp, sp, #14*4

    @ remove space used to store spsr
    add     sp, sp, #4

    @ restores user registers
    ldmfd    sp, {sp, lr}^
    add      sp, sp, #8

    movs    pc, lr
