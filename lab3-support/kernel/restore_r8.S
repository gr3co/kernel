@restore r8 to global_data

	.extern global_data
	.file "restore_r8.S"
	.text
	.global restore_r8

restore_r8:
	ldr r8, =global_data
	ldr r8, [r8]
	mov pc, lr


