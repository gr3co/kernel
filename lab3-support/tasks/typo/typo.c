/** @file typo.c
 *
 * @brief Echos characters back with timing data.
 *
 * Links to libc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/fileno.h>
#include <unistd.h>

#define BUFSIZE 64

int main(int argc, char** argv)
{
	char prompt[] = "> ";
	char buf[BUFSIZE];
	unsigned start, end;
	int n;

	printf("reads first 64 chars\n");
	while (1) {
		printf(prompt);

		// start the timer as soon as the prompt is printed
		start = time();

		n = read(STDIN_FILENO,buf,BUFSIZE);

		// stop the timer when the user presses enter
		end = time();
		write(STDOUT_FILENO,buf,n);

		// really hacky way of printing time instead of converting to float lol
		printf("%d.%ds\n",(end - start)/1000, ((end - start) % 1000) / 100);
	}
	
	//never gets here
	return 0;
}
