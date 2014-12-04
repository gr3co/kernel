@ restore_r8.S: restores r8 to the u-boot code
@
@ Author: Stephen Greco <sgreco@andrew.cmu.edu>
@ Author: Ramsey Natour <rnatour@andrew.cmu.edu>
@ Date:   11/12/2014 20:00

	.extern global_data
	.file "restore_r8.S"
	.text
	.global restore_r8

restore_r8:
	ldr r8, =global_data
	ldr r8, [r8]
	mov pc, lr


