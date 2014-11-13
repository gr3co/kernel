/** @file splat.c
 *
 * @brief Displays a spinning cursor.
 *
 * Links to libc.
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
	char positions[] = {'|', '/', '-', '\\'};
	unsigned index = 0;
	while (1) {
		printf("\r%c", positions[index]);
		index = (index + 1) % 4;
		sleep(200);
	}
	return 0;
}
