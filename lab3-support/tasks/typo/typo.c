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
		start = time();
		n = read(STDIN_FILENO,buf,BUFSIZE);
		end = time();
		write(STDOUT_FILENO,buf,n);
		printf("%d.%ds\n",(end - start)/1000, ((end - start) % 1000) / 100);
	}
	
	//never gets here
	return 0;
}
