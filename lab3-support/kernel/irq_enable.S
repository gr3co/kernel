@ irq_enable.S: enables irqs
@
@ Author: Stephen Greco <sgreco@andrew.cmu.edu>
@ Author: Ramsey Natour <rnatour@andrew.cmu.edu>
@ Date:   11/12/2014 20:00


	.file   "irq_enable.S"
	.text
	.global irq_enable

irq_enable:
	@ store r12 so we can use it
	stmfd 	sp!, {r12}

	@ turn off the bit corresponding to irq_enable
	mrs     r12, cpsr
	bic     r12, r12, #0x080
	msr     cpsr, r12

	@ restore r12
	ldmfd	sp!, {r12}
	mov 	pc, lr

