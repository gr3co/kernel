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
 #include <exports.h>
/*#ifdef DEBUG_MUTEX
 // temp
#endif
*/
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
		//printf("No mutexes remaining\n");
		return -ENOMEM;
	}
	gtMutex[next_mutex_available].bAvailable = TRUE;
	return next_mutex_available++;
}

int mutex_lock(int mutex)
{
	if (mutex >= OS_NUM_MUTEX) {
		//printf("Invalid mutex number\n");
		return -EINVAL;
	}
	if (gtMutex[mutex].bAvailable == FALSE) {
		//printf("Mutex unavailable\n");
		return -EINVAL;
	}
	tcb_t *current_task = get_cur_tcb();
	if (gtMutex[mutex].bLock == 1 
		&& gtMutex[mutex].pHolding_Tcb == current_task) {
		//printf("Mutex already locked by this task\n");
		return -EDEADLOCK;
	}
	// Mutex currently available
	if (gtMutex[mutex].bLock == 0) {
		gtMutex[mutex].bLock = 1;
		gtMutex[mutex].pHolding_Tcb = current_task;

		current_task->holds_lock += 1;
		// bump up priority to 0 so this task will always run 
		if (HLP) current_task->cur_prio = 0;

	// Add the current task to the mutex sleep queue
	} else {
		uint8_t current_priority = get_cur_prio();
		//printf("adding to sleep queu\n");
		//current_task = /*runqueue_remove(current_priority)*/get_cur_tcb;
		tcb_t *sleep_task = gtMutex[mutex].pSleep_queue;
		if (sleep_task == NULL || sleep_task->cur_prio > current_priority) {
			gtMutex[mutex].pSleep_queue = current_task;
			current_task->sleep_queue = sleep_task;
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
		dispatch_sleep();
	}
	return 0;
}

int mutex_unlock(int mutex)
{
	if (mutex >= OS_NUM_MUTEX) {
		//printf("Invalid mutex number\n");
		return -EINVAL;
	}
	if (gtMutex[mutex].bAvailable == FALSE) {
		//printf("Mutex unavailable\n");
		return -EINVAL;
	}
	tcb_t *current_task = get_cur_tcb();
	if (gtMutex[mutex].pHolding_Tcb != current_task) {
		//printf("Mutex already locked by this task\n");
		return -EPERM;
	}
	current_task->cur_prio = current_task->native_prio;
	current_task->holds_lock -= 1;

	gtMutex[mutex].pHolding_Tcb = gtMutex[mutex].pSleep_queue;
	if (gtMutex[mutex].pHolding_Tcb == NULL) {
		gtMutex[mutex].bLock = 0;
	} else {
		gtMutex[mutex].pSleep_queue = gtMutex[mutex].pHolding_Tcb->sleep_queue;
		gtMutex[mutex].pHolding_Tcb->sleep_queue = NULL;
		gtMutex[mutex].pHolding_Tcb->cur_prio = 0;
		runqueue_add(gtMutex[mutex].pHolding_Tcb, 0);
	}

	return 0;
}

