#include <exports.h>
#include <bits/errno.h>
#include <bits/fileno.h>
#include <bits/swi.h>

#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/interrupt.h>
#include <arm/timer.h>

#include "globals.h"
#include "swi_handler.h"
#include "irq_handler.h"
#include "user_setup.h"
#include "exit_handler.h"

uint32_t global_data;
unsigned long base_time;

int swi_instr_1;
int swi_instr_2;
int irq_instr_1;
int irq_instr_2;

int *irqTop;

typedef enum {FALSE, TRUE} bool;

#define NULL 0

// 0xe59ff014 (LDR pc, [pc, 0x14]) --> 0x014 through masking
#define SWI_VECT_ADDR 0x08
#define IRQ_VECT_ADDR 0x18
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

#define ICPR_ADDR 0x40d00010
#define ICMR_ADDR 0x40d00004
#define ICLR_ADDR 0x40d00008
#define ASSERT_26 0x02000000

#define OSMR0_ADDR 0x40a00000
#define OIER_ADDR 0x40a0001c
#define OSCR0_ADDR 0x40a00010
#define OSSR_ADDR 0x40a00014
#define CLOCK_TO_MILLI 3250000000

/* Checks the Vector Table at vector for an appropriate address. */
bool check_vector(int* vector) {
    int vector_instr = *(vector);

    // Check if the offset is negative.
    if ((vector_instr & LDR_SIGN_MASK) == 0) {
        return FALSE;
    }

    // Check that the instruction is a (LDR pc, [pc, 0x000])
    if ((vector_instr & 0xFFFFF000) != LDR_PC_PC_INSTR) {
        return FALSE;
    }

    return TRUE;
}

//wires in handler at address from offset in vector
//returns the address overwritten
int* wire_in (int vector, int swi_true) {
    int jmp_offset = (*((int *) vector))&(0xFFF);

    // &S_Handler" in Jump Table.
    int *handler_addr = *(int **)(vector + PC_OFFSET + jmp_offset);

    // Save original Uboot handler instructions.
    if (swi_true) {
        swi_instr_1 = *handler_addr;
        swi_instr_2 = *(handler_addr + 1);  
    }
    else {
        irq_instr_1 = *handler_addr;
        irq_instr_2 = *(handler_addr + 1);
    }

    // Wire in our own: LDR pc, [pc, #-4] = 0xe51ff004
    *handler_addr = 0xe51ff004;
    *(handler_addr + 1) = swi_true ? (int) &swi_handler : (int) &irq_handler; // New pointer to handler.
    return handler_addr;
}

int kmain(int argc, char** argv, uint32_t table)
{

	app_startup(); /* Note that app_startup() sets all uninitialized and */ 
			/* zero global variables to zero. Make sure to consider */
			/* any implications on code executed before this. */

    //variable declarations
    int *swi_handler_addr;
    int *irq_handler_addr;

    //get the start time of the kernel
    base_time = get_timer(0);

	global_data = table;

    //check irq and swi vector instructions
	if (check_vector((int *)SWI_VECT_ADDR) == FALSE) {
        return BAD_CODE;
    }
    printf("swi vector is good\n");
    if (check_vector((int *)IRQ_VECT_ADDR) == FALSE) {
        return BAD_CODE;
    }
    printf("irq vector is good\n");

    /** Wire in the SWI and IRQ handlers. **/
    // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
    swi_handler_addr = wire_in(SWI_VECT_ADDR,1);
    printf("SWI handler wired in\n");
    irq_handler_addr = wire_in(IRQ_VECT_ADDR,0);
    printf("IRQ handler wired in\n");

    // Copy argc and argv to user stack in the right order.
    int *spTop = ((int *) USER_STACK_TOP) - 1;
    int i = 0;
    for (i = argc-1; i >= 0; i--) {
        *spTop = (int)argv[i];
        spTop--;
    }
    *spTop = argc;

    printf("user stack is set up\n");

    //set up irq stack
    irqTop = malloc(4*32);
    if (irqTop == NULL) {
        printf("mallocing irq stack failed\n");
        return 0xe3303;
    } 

    irqTop = irqTop+31;

    /** Jump to user program. **/
    int usr_prog_status = user_setup(spTop);

    printf("returned from user program\n");

    /** Restore SWI Handler. **/
    *swi_handler_addr = swi_instr_1;
    *(swi_handler_addr + 1) = swi_instr_2;

    //restore IRQ handler
    *irq_handler_addr = irq_instr_1;
    *(irq_handler_addr + 1) = irq_instr_2;

    //free irq stack
    free(irqTop);

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
    unsigned i;
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

    unsigned i = 0;
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

//handler for time_swi, trigger interrupt
unsigned long timer_handler() {
    

    return get_timer(base_time);
}

//handler for sleep_swi, triggers interrupt
void sleep_handler(unsigned long millis) {

    //pointers to IC registers we need
    volatile int* ICMR = (volatile int *) ICMR_ADDR;
    volatile int* ICLR = (volatile int *) ICLR_ADDR;

    //pointers to OS timer registers
    volatile int* OIER = (volatile int *) OIER_ADDR;
    volatile int* OSSR = (volatile int *) OSSR_ADDR;
    //volatile int* OSMR0 = (volatile int *) OSMR0_ADDR;

    *ICLR = (*ICLR & 0); //enable IRQ
    *ICMR = (*ICMR & ASSERT_26); //mask all interrupts besides Match Timer 0
    *OIER = (*OIER & 0xFFFFFFF1); //allow matches to trigger interrupts
    *OSSR = (*OSSR & 0xFFFFFFF1); //hopefully this triggers an interrupt
    //*OSMR0 = *((volatile int *) (millis * CLOCK_TO_MILLI)
}

/* C_SWI_Handler uses SWI number to call the appropriate function. */
int C_SWI_Handler(int swiNum, int *regs) {
    int count = 0;
    unsigned long ret;
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
        case TIME_SWI:
            ret = timer_handler();
            count = (int)ret;
            break;
        case SLEEP_SWI:
            printf("got sleep swi\n");
            sleep_handler((unsigned long)regs[0]);
            break;
        default:
            printf("Error in ref C_SWI_Handler: Invalid SWI number.");
            exit_handler(BAD_CODE); // never returns
    }

    return count;
}

/* C_IRQ_Handler uses IRQ number to perform appropriate action */
int C_IRQ_Handler(int irqNum) {
    //volatile int* OSMR0 = (volatile int *) OSMR0_ADDR;
    printf("I like the number %x\n",irqNum);
    return 0xa00a;
}
