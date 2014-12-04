/**
 * @file mutex.c
 *
 * @brief Implements mutices.
 *
 * @author Harry Q Bovik < PUT YOUR NAMES HERE
 *
 * 
 * @date  
 */

//#define DEBUG_MUTEX

#include <lock.h>
#include <task.h>
#include <sched.h>
#include <bits/errno.h>
#include <arm/psr.h>
#include <arm/exception.h>
#ifdef DEBUG_MUTEX
#include <exports.h> // temp
#endif

#define NULL 0

mutex_t gtMutex[OS_NUM_MUTEX];
static int next_mutex_available;

void mutex_init()
{
	int i;
	for (i = 0; i < OS_NUM_MUTEX; i++) {
		gtMutex[i].bAvailable = FALSE;
		gtMutex[i].pHolding_Tcb = NULL;
		gtMutex[i].bLock = 0;
		gtMutex[i].pSleep_queue = NULL;
	}
	next_mutex_available = 0;
}

int mutex_create(void)
{
	if (next_mutex_available >= OS_NUM_MUTEX) {
		printf("No mutexes remaining\n");
		return;
	}
	gtMutex[next_mutex_available++].bAvailable = TRUE;
}

int mutex_lock(int mutex)
{
	if (mutex >= OS_NUM_MUTEX) {
		printf("Invalid mutex number\n");
		return -EINVAL;
	}
	if (gtMutex[mutex].bAvailable == FALSE) {
		printf("Mutex unavailable\n");
		return -EINVAL;
	}
	if (gtMutex[mutex].bLock == 1 
		&& gtMutex[mutex].pHolding_Tcb == get_cur_tcb()) {
		printf("Mutex already locked by this task\n");
		return -EDEADLOCK;
	}
	// Mutex currently available
	if (gtMutex[mutex].bLock == 0) {
		gtMutex[mutex].bLock = 1;
		gtMutex[mutex].pHolding_Tcb = get_cur_tcb();

		// Do something to the tcb's priority too
		
		return 0;
	// Add the current task to the mutex sleep queue
	} else {
		uint8_t current_priority = get_cur_prio();
		tcb_t *current_task = get_cur_tcb();
		tcb_t *sleep_task = gtMutex[mutex].pSleep_queue;
		if (sleep_task == NULL || sleep_task->cur_prio > current_priority) {
			gtMutex[mutex].pSleep_queue = current_task;
			current_task->pSleep_queue = sleep_task;
		} else {
			// look for where the current task would fit into the queue
			while (sleep_task->sleep_queue != NULL 
				&& sleep_task->sleep_queue->cur_prio < current_priority) {
				sleep_task = sleep_task->sleep_queue;
			}
			// and insert the task
			tcb_t *next = sleep_task->sleep_queue;
			sleep_task->sleep_queue = current_task;
			current_task->sleep_queue = next;
		}
	}
}

int mutex_unlock(int mutex  __attribute__((unused)))
{
	return 1; // fix this to return the correct value
}

