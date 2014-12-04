@ irq_handler.S: handle IRQ's when they're called
@
@ Author: Stephen Greco <sgreco@andrew.cmu.edu>
@ Author: Ramsey Natour <rnatour@andrew.cmu.edu>
@ Date:   11/12/2014 20:00

	.extern C_IRQ_Handler
	.extern irqTop
    .extern stackPtr

    .file   "irq_handler.S"
    .text

    .global irq_handler
    .extern global_data
    .extern irqTop

irq_handler:

    @ make sure stack is where we want it to be
    ldr     sp, =irqTop
    ldr     sp, [sp]

    @ make space for spsr
	sub     sp, sp, #4

    @ stash away our current registers
    stmfd   sp!, {r0-r12, lr}

    @ restore r8 every time we go to irq mode
    ldr r8, =global_data
    ldr r8, [r8]

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @ jump to C code
    bl      C_IRQ_Handler

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}

    @ remove space used to store spsr
    add     sp, sp, #4
    movs 	pc, lr
