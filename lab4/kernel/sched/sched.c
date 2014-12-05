/** @file sched.c
 * 
 * @brief Top level implementation of the scheduler.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

#include <types.h>
#include <assert.h>

#include <kernel.h>
#include <config.h>
#include "sched_i.h"

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>

#define NULL 0 

uint32_t idle_stack[8];

tcb_t system_tcb[OS_MAX_TASKS]; /*allocate memory for system TCBs */

void sched_init(task_t* main_task  __attribute__((unused)))
{
	// refuse to implement
}

/**
 * @brief This is the idle task that the system runs when no other task is runnable
 */
 
static void idle(void* ptr __attribute__((unused)))
{
	 enable_interrupts();
	 while(1);
}

static tcb_t* create_tcb(task_t task, uint8_t priority) {
	
	// set up the tcb
	system_tcb[priority].native_prio = priority;
	system_tcb[priority].cur_prio = priority;
	system_tcb[priority].holds_lock = FALSE;
	system_tcb[priority].sleep_queue = NULL;

		// set up the context
	sched_context_t *context = &(system_tcb[priority].context);
	context->r4 = (uint32_t) task.lambda;
	context->r5 = (uint32_t) task.data;
	context->r6 = (uint32_t) task.stack_pos;
	context->sp = (void*) &(system_tcb[priority].kstack_high);
	context->lr = (void*) &launch_task;

	// add it to the runqueue and the system tcb list (the indices should always match), runlist is for redundancacy
	runqueue_add(&system_tcb[priority], priority);

	return &system_tcb[priority];

}

/**
 * @brief Allocate user-stacks and initializes the kernel contexts of the
 * given threads.
 *
 * This function assumes that:
 * - num_tasks < number of tasks allowed on the system.
 * - the tasks have already been deemed schedulable and have been appropriately
 *   scheduled.  In particular, this means that the task list is sorted in order
 *   of priority -- higher priority tasks come first.
 *
 * @param tasks  A list of scheduled task descriptors.
 * @param size   The number of tasks is the list.
 */
void allocate_tasks(task_t** tasks, size_t num_tasks)
{
	runqueue_init();
		
	unsigned i;
	for (i = 0; i < num_tasks - 1; i++) {
		create_tcb(tasks[0][i], i);
	}

	task_t idle_task;
	idle_task.lambda = idle;
<<<<<<< HEAD
	idle_task.C = IDLE_PRIO;
	idle_task.stack_pos = (void*)0xa3000000;
=======
	idle_task.C = 63;
	idle_task.stack_pos = (void*)&idle_stack;
>>>>>>> 86e9d41c9e042894a1cf4781a08c5daa497be37c
	idle_task.data = NULL;
	tcb_t* idle_tcb = create_tcb(idle_task, 63);

	enable_interrupts();

	dispatch_init(idle_tcb);
}

