/** @file crt0.S
 *
 * @brief Execution startup routine
 *
 * @author Ramsey Natour <rnatour@andrew.cmu.edu>
 * @date   November 9, 2014
 */

#include <asm.h>
#include <bits/swi.h>

	.file "crt0.S"

FUNC(_start)
    ldr r0, [sp]   @ give argc
    add r1, sp, #4 @ give argv
    bl main
    swi EXIT_SWI
