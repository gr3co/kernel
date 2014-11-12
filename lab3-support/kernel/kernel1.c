/*
 * kernel.c: Kernel main (entry) function
 *
 * Author: Group Member 1 <email address>
 *         Group Member 2 <email address>
 * Date:   The current time & date
 */

#include <exports.h>
#include <bits/errno.h>
#include <bits/fileno.h>
#include <bits/swi.h>

#include "globals.h"
#include "swi_handler.h"
#include "user_setup.h"
#include "exit_handler.h"

typedef enum {FALSE, TRUE} bool;

// 0xe59ff014 (LDR pc, [pc, 0x14]) --> 0x014 through masking
#define SWI_VECT_ADDR 0x08
#define PC_OFFSET 0x08

// Cannot write to this address. kernel.bin loaded here. Stack grows down.
#define USER_STACK_TOP 0xa3000000

// (LDR pc, [pc, 0x000]) 0xe59ff000 --> 0x000 through masking
#define LDR_PC_PC_INSTR 0xe59ff000
#define LDR_SIGN_MASK 0x00800000

#define BAD_CODE 0x0badc0de

#define SFROM_START 0x00000000
#define SFROM_END 0x00ffffff
#define SDRAM_START 0xa0000000
#define SDRAM_END 0xa3ffffff

/* Checks the SWI Vector Table. */
bool check_swi_vector() {
    int swi_vector_instr = *((int *)SWI_VECT_ADDR);

    // Check if the offset is negative.
    if ((swi_vector_instr & LDR_SIGN_MASK) == 0) {
        return FALSE;
    }

    // Check that the instruction is a (LDR pc, [pc, 0x000])
    if ((swi_vector_instr & 0xFFFFF000) != LDR_PC_PC_INSTR) {
        return FALSE;
    }

    return TRUE;
}

int main(int argc, char *argv[]) {
    if (check_swi_vector() == FALSE) {
        return BAD_CODE;
    }

    /** Wire in the SWI handler. **/
    // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
    int jmp_offset = (*((int *) SWI_VECT_ADDR))&(0xFFF);

    // &S_Handler" in Jump Table.
    int *swi_handler_addr = *(int **)(SWI_VECT_ADDR + PC_OFFSET + jmp_offset);

    // Save original Uboot SWI handler instructions.
    int swi_instr_1 = *swi_handler_addr;
    int swi_instr_2 = *(swi_handler_addr + 1);

    // Wire in our own: LDR pc, [pc, #-4] = 0xe51ff004
    *swi_handler_addr = 0xe51ff004;
    *(swi_handler_addr + 1) = (int) &swi_handler; // New swi handler.

    // Copy argc and argv to user stack in the right order.
    int *spTop = ((int *) USER_STACK_TOP) - 1;
    int i = 0;
    for (i = argc-1; i >= 0; i--) {
        *spTop = (int)argv[i];
        spTop--;
    }
    *spTop = argc;


    /** Jump to user program. **/
    int usr_prog_status = user_setup(spTop);


    /** Restore SWI Handler. **/
    *swi_handler_addr = swi_instr_1;
    *(swi_handler_addr + 1) = swi_instr_2;

	return usr_prog_status;
}


/* Verifies that the buffer is entirely in valid memory. */
int check_mem(char *buf, int count, unsigned start, unsigned end) {
    unsigned start_buf = (unsigned) buf;
    unsigned end_buf = (unsigned)(buf + count);

    if ( (start_buf < start) || (start_buf > end) ) {
        return FALSE;
    }
    if ( (end_buf < start) || (end_buf > end) ) {
        return FALSE;
    }
    // Overflow case.
    if ( start_buf >= end_buf ) {
        return FALSE;
    }

    return TRUE;
}

// write function to replace the system's write function
ssize_t write_handler(int fd, const void *buf, size_t count) {

    // Check for invalid memory range or file descriptors
    if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == FALSE &&
        check_mem((char *) buf, (int) count, SFROM_START, SFROM_END) == FALSE) {
        exit_handler(-EFAULT);
    } else if (fd != STDOUT_FILENO) {
        exit_handler(-EBADF);
    }

    char *buffer = (char *) buf;
    int i;
    char read_char;
    for (i = 0; i < count; i++) {
        // put character into buffer and putc
        read_char = buffer[i];
        putc(read_char);
    }
    return i;
}


// read function to replace the system's read function
ssize_t read_handler(int fd, void *buf, size_t count) {
    // Check for invalid memory range or file descriptors
    if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == FALSE) {
        exit_handler(-EFAULT);
    } else if (fd != STDIN_FILENO) {
        exit_handler(-EBADF);
    }

    int i = 0;
    char *buffer = (char *) buf;
    char read_char;

    while (i < count) {
        read_char = getc();

        if (read_char == 4) { //EOT character
            return i;
        } else if (((read_char == 8) || (read_char == 127))) { // backspace or DEL character
            buffer[i] = 0; // '\0' character
            if(i > 0) {
                i--;
                puts("\b \b");
            }
        } else if ((read_char == 10) || (read_char == 13)) { // '\n' newline or '\r' carriage return character
            buffer[i] = '\n';
            putc('\n');
            return (i+1);
        } else {
            // put character into buffer and putc
            buffer[i] = read_char;
            i++;
            putc(read_char);
        }
    }

    return i;
}

/* C_SWI_Handler uses SWI number to call the appropriate function. */
int C_SWI_Handler(int swiNum, int *regs) {
    int count = 0;
    switch (swiNum) {
        // ssize_t read(int fd, void *buf, size_t count);
        case READ_SWI:
            count = read_handler(regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        // ssize_t write(int fd, const void *buf, size_t count);
        case WRITE_SWI:
            count = write_handler((int) regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        // void exit(int status);
        case EXIT_SWI:
            exit_handler((int) regs[0]); // never returns
            break;
        default:
            printf("Error in ref C_SWI_Handler: Invalid SWI number.");
            exit_handler(BAD_CODE); // never returns
    }

    return count;
}


