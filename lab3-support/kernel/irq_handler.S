@irq handler

	.extern C_IRQ_Handler
	.extern irqTop

    .file   "irq_handler.S"
    .text

    .global irq_handler

irq_handler:

	sub     sp, sp, #4
    stmfd   sp!, {r0-r12, lr}

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @set stack pointer to heap allocated irq stack
	ldr 	r2, =irqTop
	ldr 	r2, [r2] 	@r2 -> irqTop

	str 	sp, [r2]	@irqTop = oldSp
	mov		sp, r2 		@sp -> irqTop
	

    @for now, pass in arbitary constant to be printed in C_IRQ_Handler
    @mov r0, #0xbb

    bl      C_IRQ_Handler

    @restore old stack
    ldr r2, [sp]		@r2 = oldSP
    mov sp, r2 			@sp = oldSp

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}

    @ remove space used to store spsr
    add     sp, sp, #4
    movs 	pc, lr
