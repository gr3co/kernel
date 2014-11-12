@ exit_handler.S: function written in asm called from kernel
@
@ Author: Harry Q Bovik <hqbovik@andrew.cmu.edu>
@ Date:   Tue, 23 Oct 2007 11:20:33 -0400
@
@ Hint: Use our global stack pointer to help us return to kernel main through
@   user_setup().

    .extern user_setup_stack_ptr

    .file   "exit_handler.S"
    .text

    .global exit_handler
exit_handler:
    @ return value is in r0

    @ set global return value to exit status (in r0)
    ldr     r1, =user_setup_stack_ptr
    ldr     sp, [r1]

    @ Restore cpsr, gp registers, and lr.
    ldmfd   sp!, {r1}
    msr     cpsr, r1
    ldmfd   sp!, {r1-r12, lr}

    @ Go back into kernel main.
    mov     pc, lr
