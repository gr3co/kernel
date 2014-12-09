/** @file ctx_switch.c
 * 
 * @brief C wrappers around assembly context switch routines.
 *
 *@author Stephen Greco <sgreco@andrew.cmu.edu>
 * @author Ramsey Natour <rnatour@andrew.cmu.edu>
 * @date   2014-12-4 
 */
 

#include <types.h>
#include <assert.h>

#include <config.h>
#include <kernel.h>
#include "sched_i.h"
#include <sched.h>
#include <arm/exception.h>
#ifdef DEBUG_MUTEX
#include <exports.h>
#endif

static tcb_t* cur_tcb; /* use this if needed */
void* tcb_kernel_stack;


/**
 * @brief Initialize the current TCB and priority.
 *
 * Set the initialization thread's priority to IDLE so that anything
 * will preempt it when dispatching the first task.
 */
void dispatch_init(tcb_t* idle)
{
	cur_tcb = idle;
}


/**
 * @brief Context switch to the highest priority task while saving off the 
 * current task state.
 *
 * This function needs to be externally synchronized.
 * We could be switching from the idle task.  The priority searcher has been tuned
 * to return IDLE_PRIO for a completely empty run_queue case.
 */
void dispatch_save(void)
{
	uint8_t highest = highest_prio();

	// don't bother if we're not changing anything
	if (cur_tcb->cur_prio <= highest)
    	return;

    tcb_t *next_tcb = runqueue_remove(highest_prio());
  	runqueue_add(cur_tcb, cur_tcb->cur_prio);

	tcb_t* old = cur_tcb;
	cur_tcb = next_tcb;

	//printf("switching with save from task %d\n",highest);
	ctx_switch_full(&cur_tcb->context, &old->context);
}

/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * don't save the current task state.
 *
 * There is always an idle task to switch to.
 */
void dispatch_nosave(void)
{
	uint8_t highest = highest_prio();
	cur_tcb = runqueue_remove(highest);
	//printf("switching without save from task %d\n",highest);
	ctx_switch_half(&cur_tcb->context);
}


/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * and save the current task but don't mark is runnable.
 *
 * There is always an idle task to switch to.
 */
void dispatch_sleep(void)
{
	uint8_t highest = highest_prio();
	tcb_t *next = runqueue_remove(highest);

	// we don't want to remove the idle task
	if (highest == 63)
    	runqueue_add(next, next->cur_prio);	

	tcb_t* old = cur_tcb;
	cur_tcb = next;
	//printf("putting task %d to sleep\n",highest);
	ctx_switch_full(&cur_tcb->context, &old->context);
}

/**
 * @brief Returns the priority value of the current task.
 */
uint8_t get_cur_prio(void)
{
	return cur_tcb->cur_prio;
}

/**
 * @brief Returns the TCB of the current task.
 */
tcb_t* get_cur_tcb(void)
{
	return cur_tcb;
}
