/** @file proc.c
 * 
 * @brief Implementation of `process' syscalls
 *
 * @author Mike Kasick <mkasick@andrew.cmu.edu>
 * @date   Sun, 14 Oct 2007 00:07:38 -0400
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-12
 */

#include <exports.h>
#include <bits/errno.h>
#include <config.h>
#include <kernel.h>
#include <syscall.h>
#include <sched.h>

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>
#include <device.h>

int kernel_task_create(task_t* tasks, size_t num_tasks)
{

	disable_interrupts();

	unsigned i;
	task_t current_task;

	if(num_tasks > OS_MAX_TASKS) {
		return -EINVAL;
	}

	if (!valid_addr(tasks,num_tasks*sizeof(task_t), USR_START_ADDR, USR_END_ADDR+1)) {
		printf("tasks is allocated at a bad address\n");
		return -EFAULT;
	}

	for (i = 0; i < num_tasks; i++) {
		current_task = tasks[i];
		//check if the user stack contains valid memory addresses
		if (!valid_addr(current_task.stack_pos, OS_KSTACK_SIZE, USR_START_ADDR, USR_END_ADDR)) {
			printf("user inputted an invalid stack address\n");
			return -EFAULT;
		}
		//check schedubility
		//*sorted_task[i] = &tasks[i];
	}

	task_t *ptrs[OS_MAX_TASKS];
	for (i = 0; i < num_tasks; i++) {
		ptrs[i] = &tasks[i];
	}

	if (assign_schedule(ptrs, num_tasks) == 0) {
		printf("Tasks aren't schedulable");
		return -ESCHED;
	}

  	// global variable
  	task_created = 1;

  	allocate_tasks(ptrs, num_tasks);
  	dispatch_nosave();

  	return -1;
}

int kernel_event_wait(unsigned int dev)
{
	if (dev >= 4) {
		printf("invalid device number\n");
		return -EINVAL;
	}
	disable_interrupts();
  	dev_wait(dev);
  	enable_interrupts();
  	return 0;	
}

/* An invalid syscall causes the kernel to exit. */
void invalid_syscall(unsigned int call_num)
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
