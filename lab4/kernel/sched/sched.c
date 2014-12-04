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

static void create_tcb(task_t task, uint8_t priority) {
	
	tcb_t result;
	
	// set up the context
	sched_context_t context;
	context.sp = (void*)task.stack_pos;
	context.lr = (void*)task.lambda;
	context.r5 = (uint32_t)task.data;

	// set up the tcb
	result.native_prio = priority;
	result.cur_prio = priority;
	result.context = context;

	// add it to the runqueue and the system tcb list (the indices should always match), runlist is for redundancacy
	runqueue_add(&result, priority);
	system_tcb[priority] = result;

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
	
	task_t idle_task;
	idle_task.lambda = idle;
	idle_task.C = 63;
	idle_task.stack_pos = (void*)0xa3000000;
	idle_task.data = NULL;
	create_tcb(idle_task, 63);
	
	unsigned i;
	for (i = 0; i < num_tasks; i++) {
		create_tcb(tasks[0][i], i);
	}

	dispatch_nosave();
}

