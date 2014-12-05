/** @file ub_test.c
 * 
 * @brief The UB Test for basic schedulability
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

//#define DEBUG 0

#include <sched.h>
#include <task.h>
#ifdef DEBUG
#include <exports.h>
#endif

// precalculated from wolfram alpha
static const float bounds[20] = 
	{1.000000, 0.828427, 0.779763, 0.756828, 
	0.743492, 0.734772, 0.728627, 0.724062, 
	0.720538, 0.717735, 0.715452, 0.713557, 
	0.711959, 0.710593, 0.709412, 0.708381, 
	0.707472, 0.706666, 0.705946, 0.705298}; 

/**
 * @brief Perform UB Test and reorder the task list.
 *
 * The task list at the end of this method will be sorted in order is priority
 * -- from highest priority (lowest priority number) to lowest priority
 * (highest priority number).
 *
 * @param tasks  An array of task pointers containing the task set to schedule.
 * @param num_tasks  The number of tasks in the array.
 *
 * @return 0  The test failed.
 * @return 1  Test succeeded.  The tasks are now in order.
 */
int assign_schedule(task_t** tasks, size_t num_tasks)
{

	float sum = 0.0F;
	task_t* swap;
	unsigned i, j;

	for (i = 0; i < num_tasks; i++) {
		sum += ((float) (tasks[i]->C)) / ((float) (tasks[i]->T));
	}

	if (num_tasks > 20 || sum > bounds[num_tasks - 1]) {
		return 0;
	}

	// sort the tasks array
  	for (i = 0; i < num_tasks - 1; i++) {
    	for (j = 0; j < num_tasks - 1; j++) {
    		if (tasks[j]->T > tasks[j + 1]->T) {
    			swap = tasks[j];
    			tasks[j] = tasks[j + 1];
    			tasks[j + 1] = swap;
         	}
    	}
  	}

	return 1;	
}
	


