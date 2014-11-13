/** @file splat.c
 *
 * @brief Displays a spinning cursor.
 *
 * Links to libc.
 */
#include <stdlib.h>
#include <unistd.h>
#include <bits/fileno.h>

int main(int argc, char** argv) {
	char positions[] = {'|', '/', '-', '\\'};
	unsigned index = 0;
	char buf[] = {'\r', '|'};
	write(STDOUT_FILENO, buf, 2);
	while (1) {
		sleep(200);
		index = (index + 1) % 4;
		buf[1] = positions[index];
		write(STDOUT_FILENO, buf, 2);
	}
	return 0;
}
