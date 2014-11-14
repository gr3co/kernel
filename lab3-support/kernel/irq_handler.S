@irq handler

	.extern C_IRQ_Handler
	.extern irqTop
    .extern stackPtr

    .file   "irq_handler.S"
    .text

    .global irq_handler
    .extern global_data
    .extern irqTop

irq_handler:

    ldr     sp, =irqTop
    ldr     sp, [sp]

	sub     sp, sp, #4
    stmfd   sp!, {r0-r12, lr}

    ldr r8, =global_data
    ldr r8, [r8]

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @set stack pointer to heap allocated irq stack
	@ldr 	r2, =stackPtr
    @str     sp, [r2]
    @ldr     r2, =irqTop
    @ldr     sp, [r2]

    @ldr 	r2, [r2] 	@r2 -> irqTop

	@str 	sp, [r2]	@irqTop = oldSp
	@mov		sp, r2 		@sp -> irqTop
	

    @for now, pass in arbitary constant to be printed in C_IRQ_Handler
    @mov r0, #0xbb

    bl      C_IRQ_Handler

    @restore old stack
    @ldr r2, [sp]		@r2 = oldSP
    @mov sp, r2 			@sp = oldSp
    @ldr     r2, =stackPtr
    @ldr     sp, [r2]

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}

    @ remove space used to store spsr
    add     sp, sp, #4
    movs 	pc, lr
