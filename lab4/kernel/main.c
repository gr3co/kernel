/** @file main.c
 *
 * @brief kernel main
 *
 * @author 
 *	   
 *	   
 * @date   
 */
 
#include <kernel.h>
#include <task.h>
#include <sched.h>
#include <device.h>
#include <assert.h>
#include <syscall.h>
#include <types.h>
#include <lock.h>

#include <bits/swi.h>

#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/interrupt.h>
#include <arm/timer.h>
#include <arm/reg.h>

#include "handler.h"
#include "user_setup.h"

unsigned swi_instr_1;
unsigned swi_instr_2;
unsigned irq_instr_1;
unsigned irq_instr_2;

//keeps track of system time in milliseconds
volatile unsigned current_time;

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

#define CLOCK_TO_10_MILLI OSTMR_FREQ / 100

uint32_t global_data;
bool_e task_created;

extern void irq_wrapper(void);
extern int swi_handler(int);
//extern void irq_enable(void);


//set up lab4 data structures

/* Checks the Vector Table at vector for an appropriate address. */
bool_e check_vector(int* vector) {
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
    *(handler_addr + 1) = swi_true ? (int) &swi_handler : (int) &irq_wrapper; // New pointer to handler.
    return handler_addr;
}

int kmain(int argc __attribute__((unused)), 
    char** argv __attribute__((unused)), 
    uint32_t table)
{

	app_startup();
	global_data = table;
	
	int *swi_handler_addr;
    int *irq_handler_addr; 

    if (check_vector((int *)IRQ_VECT_ADDR) == FALSE) {
        return BAD_CODE;
    }
    
    swi_handler_addr = wire_in(SWI_VECT_ADDR,1);
    irq_handler_addr = wire_in(IRQ_VECT_ADDR,0);

    //initialize global time variable
    current_time = 0;
    //enable OSTIMER 0 match interrupts
    reg_set(INT_ICMR_ADDR, (1<<26));
    //set all interrupts to IRQ
    reg_write(INT_ICLR_ADDR,0);
    //enable channel 0 on the OS Timer
    reg_write(OSTMR_OIER_ADDR,OSTMR_OIER_E0);
    //reset the clock
    reg_write(OSTMR_OSCR_ADDR, 0);
    // put 10 millis + current time in OSMR
    reg_write(OSTMR_OSMR_ADDR(0), (unsigned)CLOCK_TO_10_MILLI);
    //enable IRQ's
    enable_interrupts();

    // tasks haven't been created yet
    task_created = 0;

    user_setup((unsigned*) USER_STACK_TOP);

	assert(0);        /* should never get here */
}

/* C_SWI_Handler uses SWI number to call the appropriate function. */
int C_SWI_Handler(int swi_num, int *regs) {
    int count = 0;
    switch (swi_num) {
        // ssize_t read(int fd, void *buf, size_t count);
        case READ_SWI:
            count = read_syscall(regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        // ssize_t write(int fd, const void *buf, size_t count);
        case WRITE_SWI:
            count = write_syscall((int) regs[0], (void *) regs[1], (size_t) regs[2]);
            break;
        case TIME_SWI:
            count = (int) time_syscall();
            break;
        case SLEEP_SWI:
            sleep_syscall ((unsigned) regs[0]);
            break;
        case CREATE_SWI:
            count = task_create((task_t*) regs[0], (size_t) regs[1]);
            break;
        case MUTEX_CREATE:
            count = mutex_create();
            break;
        case MUTEX_LOCK:
            count = mutex_lock((int) regs[0]);
            break;
        case MUTEX_UNLOCK:
            count = mutex_unlock((int) regs[0]);
            break;
        case EVENT_WAIT:
            count = event_wait((int) regs[0]);
            break;
        default:
            invalid_syscall(swi_num);
            break;
    }

    return count;
}
