/**
 * @file device.c
 *
 * @brief Implements simulated devices.
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-12-01
 */

#include <types.h>
#include <assert.h>

#include <task.h>
#include <sched.h>
#include <device.h>
#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>

#define NULL 0

/**
 * @brief Fake device maintainence structure.
 * Since our tasks are periodic, we can represent 
 * tasks with logical devices. 
 * These logical devices should be signalled periodically 
 * so that you can instantiate a new job every time period.
 * Devices are signaled by calling dev_update 
 * on every timer interrupt. In dev_update check if it is 
 * time to create a tasks new job. If so, make the task runnable.
 * There is a wait queue for every device which contains the tcbs of
 * all tasks waiting on the device event to occur.
 */

struct dev
{
	tcb_t* sleep_queue;
	unsigned long   next_match;
};
typedef struct dev dev_t;

/* devices will be periodically signaled at the following frequencies */
const unsigned long dev_freq[NUM_DEVICES] = {100, 200, 500, 50};
static dev_t devices[NUM_DEVICES];

/**
 * @brief Initialize the sleep queues and match values for all devices.
 */
void dev_init(void)
{
	int i;
   	for (i = 0; i < NUM_DEVICES; i++) {
   		dev_t new_device;
   		new_device.sleep_queue = NULL;
   		new_device.next_match = dev_freq[i];
   		devices[i] = new_device;
   	}
}


/**
 * @brief Puts a task to sleep on the sleep queue until the next
 * event is signalled for the device.
 *
 * @param dev  Device number.
 */
void dev_wait(unsigned int dev)
{
	if (dev >= NUM_DEVICES) {
		printf("Awwww you done fucked up\n");
		return;
	}
	tcb_t *sleep_task = devices[dev].sleep_queue;

	// find the current running task
	uint8_t current_priority = get_cur_prio();
	tcb_t *current_task = runqueue_remove(current_priority);

	// edge case: if the current task would be highest priority for 
	// this device, just add it to the front of the queue
	if (sleep_task == NULL || sleep_task->cur_prio > current_priority) {
		devices[dev].sleep_queue = current_task;
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
}


/**
 * @brief Signals the occurrence of an event on all applicable devices. 
 * This function should be called on timer interrupts to determine that 
 * the interrupt corresponds to the event frequency of a device. If the 
 * interrupt corresponded to the interrupt frequency of a device, this 
 * function should ensure that the task is made ready to run 
 */
void dev_update(unsigned long millis)
{
	int i;
	for (i = 0; i < NUM_DEVICES; i++) {
		// if it's time to check this device
		if (devices[i].next_match <= millis) {
			// add all of the devices tasks to the runqueue
			tcb_t *task = devices[i].sleep_queue;
			while (task != NULL) {
				runqueue_add(task, task->cur_prio);
				tcb_t *next = task->sleep_queue;
				task->sleep_queue = NULL;
				task = next;
			}
			devices[i].sleep_queue = NULL;
			// reset the next match
			devices[i].next_match += dev_freq[i];
		}
	}	
}

