@turn on irqs

	.file   "irq_enable.S"
	.text
	.global irq_enable

irq_enable:
	stmfd 	sp!, {r12}
	mrs     r12, cpsr
	bic     r12, r12, #0x080
	msr     cpsr, r12
	ldmfd	sp!, {r12}
	mov 	pc, lr

