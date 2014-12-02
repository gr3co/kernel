/** @file crt0.S
 *
 * @brief Execution startup routine
 *
 * @author Mike Kasick <mkasick@andrew.cmu.edu>
 * @date   Sun, 07 Oct 2007 01:51:29 -0400
 */

#include <asm.h>

	.file "crt0.S"

FUNC(_start)
	ldr r0, [sp]   @ give argc
    add r1, sp, #4 @ give argv
    bl main

loop:
	mov r0, #0
	mov r0, #0
    b loop
