/** @file run_queue.c
 * 
 * @brief Run queue maintainence routines.
 *
 * @author Stephen Greco <sgreco@andrew.cmu.edu>
 * @author Ramsey Natour <rnatour@andrew.cmu.edu>
 * @date   2014-12-4 
 */

#include <types.h>
#include <assert.h>
#include <task.h>
#include <arm/exception.h>

#include <kernel.h>
#include <sched.h>
#include "sched_i.h"
#include <exports.h>

#define NULL 0

//create runq queue data structure
//tcb_t * run_queue[OS_MAX_TASKS]; 

static tcb_t* run_list[OS_MAX_TASKS]; //run_list points to elements in system_tcb

/* A high bit in this bitmap means that the task whose priority is
 * equal to the bit number of the high bit is runnable.
 */
static uint8_t run_bits[OS_MAX_TASKS/8];

/* This is a trie structure.  Tasks are grouped in groups of 8.  If any task
 * in a particular group is runnable, the corresponding group flag is set.
 * Since we can only have 64 possible tasks, a single byte can represent the
 * run bits of all 8 groups.
 */
static uint8_t group_run_bits;

/* This unmap table finds the bit position of the lowest bit in a given byte
 * Useful for doing reverse lookup.
 */
static uint8_t prio_unmap_table[] =
{

0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/**
 * @brief Clears the run-queues and sets them all to empty.
 */
void runqueue_init(void)
{
	int i = 0;
	for (;i < OS_MAX_TASKS;i++) run_list[i] = NULL;
	i = 0;
	group_run_bits = 0;
	for (;i < OS_MAX_TASKS/8;i++) run_bits[i] = 0;
}

/**
 * @brief Adds the thread identified by the given TCB to the runqueue at
 * a given priority.
 *
 * The native priority of the thread need not be the specified priority.  The
 * only requirement is that the run queue for that priority is empty.  This
 * function needs to be externally synchronized.
 */
void runqueue_add(tcb_t* tcb, uint8_t prio)
{
	run_list[prio] = tcb;
	group_run_bits |= (1 << (prio >> 3));
	run_bits[prio>>3] |= (1 << (prio & 7));
	//printf("add: %#x\n", run_bits[0]);
}


/**
 * @brief Empty the run queue of the given priority.
 *
 * @return  The tcb at enqueued at the given priority.
 *
 * This function needs to be externally synchronized.
 */
tcb_t* runqueue_remove(uint8_t prio)
{

	tcb_t * new = run_list[prio];

	run_list[prio] = NULL;
	run_bits[prio>>3] &= ~(1 << (prio & 7));
	if (!run_bits[prio >> 3]) group_run_bits &= ~(1 << (prio >> 3)); 
	//printf("remove: %#x\n", run_bits[0]);
	return new; 	
}

/**
 * @brief This function examines the run bits and the run queue and returns the
 * priority of the runnable task with the highest priority (lower number).
 */
uint8_t highest_prio(void)
{
	int y, x;
	y = prio_unmap_table[group_run_bits];
	x = prio_unmap_table[run_bits[y]];
	return (y << 3) + x;	
}
