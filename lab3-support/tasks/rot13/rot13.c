/*
 * rot13.c: ROT13 cipher test application
 *
 * Authors: Group Member 1 <email address>
 *          Group Member 2 <email address>
 * Date:    The current time & date
 */

#define BUFFER_SIZE 256

#include <unistd.h>
#include <bits/fileno.h>

#define MAX_BUFFER 1024

/* String length excluding null character. */
int string_length(char* yo) {
    int i = 0;
    while (i < MAX_BUFFER && yo[i] != 0) {
        i++;
    }
    return i;
}

void echo(int argc, char** argv) {
    /* Put your code here */
    int i = 0;
    for (i = 0; i < argc; i++) {
        int len = string_length(argv[i]);
        write(STDOUT_FILENO, argv[i], len);
        write(STDOUT_FILENO, "\n", 1);
    }
}


int main(int argc, char** argv) {
    ssize_t read_count;
    char read_buf[BUFFER_SIZE];
    char write_buf[BUFFER_SIZE];

    /* Part 2 requirement. */
    echo(argc, argv);

    //char* usage_message = "Welcome to rot13. You can exit with no input.\n\0";
    //int message_len = string_length(usage_message);
    //write(STDOUT_FILENO, (const void *) usage_message, message_len);

    while (1) {
        /* Read a chunk of size BUFFER_SIZE from STDIN */
        read_count = read(STDIN_FILENO, read_buf, BUFFER_SIZE);

        // Error, return 1 -> exit with status 1
        if(read_count <= 0) { return 1; }

        // No input -> just carriage return character, terminate with 0 (success).
        if(read_count == 1) { return 0; }

        /* ROT13 the input buffer and store the result in write_buf */
        int i = read_count - 1;
        while( i >= 0) {
            char c = read_buf[i];
            if (c >= 'a' && c <= 'm') write_buf[i] = c+13;
            else if (c >= 'A' && c <= 'M') write_buf[i] = c+13;
            else if (c >= 'n' && c <= 'z') write_buf[i] = c-13;
            else if (c >= 'N' && c <= 'Z') write_buf[i] = c-13;
            else {
                write_buf[i] = c;
            }
            i--;
        }

        // If error on write, exit with status 1
        if (write(STDOUT_FILENO, write_buf, read_count) < 0) {
            return 1;
        }
    }

    return 0;
}
