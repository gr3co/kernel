/** @file countdown.c
 *
 * @brief Counts down from some number
 *
 * Links to libc.
 *
 * @author Stephen Greco <sgreco@andrew.cmu.edu>
 * @date   2014-11-12
 */
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	// start at 10 seconds
	unsigned start = 10;

	while (start > 0) {
		// print the current remaining time
		printf("\r%d  ", start--);
		// wait a second before printing again
		sleep(1000);
	}

	// oh no you've run out of time!
	printf("\rBOOM!!!\n");
	return 0;
}
