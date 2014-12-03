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

int task_create(task_t* tasks, size_t num_tasks)
{
	unsigned int i = 0, j=0;
	//unsigned long t[num_tasks];
	//task_t * sorted_task[num_tasks];
	task_t current_task;

	if(num_tasks > OS_MAX_TASKS) {
		return -EINVAL;
	}

	if (!valid_addr(tasks,num_tasks*sizeof(task_t), USR_START_ADDR, USR_END_ADDR+1)) {
		printf("tasks is allocated at a bad address\n");
		return -EFAULT;
	}

	for (; i < num_tasks; i++) {
		current_task = tasks[i];
		//check if the user stack contains valid memory addresses
		if (!valid_addr(current_task.stack_pos, OS_KSTACK_SIZE, USR_START_ADDR, USR_END_ADDR)) {
			printf("user inputted an invalid stack address\n");
			return -EFAULT;
		}
		//check schedubility
		//*sorted_task[i] = &tasks[i];
	}
	task_t swap;

	//create sorted task list
	for (i = 0 ; i < ( num_tasks - 1 ); i++) {
    	for (j = 0 ; j < num_tasks - i - 1; j++) {
      		if (tasks[j].T > tasks[j+1].T) {
        		swap = tasks[j];
        		tasks[j] = tasks[j+1];
        		tasks[j+1] = swap;
      		}
    	}
  	}

  	allocate_tasks(&tasks, num_tasks);

  	return -1;
}

int event_wait(unsigned int dev)
{
	if (dev >= 4) {
		printf("invalid device number\n");
		return -EINVAL;
	}
	dev_wait(dev);
  	return 0;	
}

/* An invalid syscall causes the kernel to exit. */
void invalid_syscall(unsigned int call_num)
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
