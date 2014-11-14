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
	unsigned start = 10;
	while (start > 0) {
		printf("\r%d  ", start--);
		sleep(1000);
	}
	printf("\rBOOM!!!\n");
	return 0;
}
