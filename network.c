//
// Created by alanfl and jmagnes362 on 12/10/19.
//

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCK_SIZE 4096

int send_message(int fd, char * message) {
    int total_written = 0;
    int message_size = strlen(message) + 1; // Null terminated;

    // For tracking empty writes and preventing a hang as a result
    int repeat = 0;

    // Continue writing until the entire message has been written
    while(total_written < message_size) {
        int written = write(fd, message + total_written, message_size);

        // Error while writing, return an error
        if (written < 0) {
            return -1;
        }

        // Track writes to prevent hanging
        repeat++;

        // If there are many cases where write fails, i.e. 1000 times, exit
        if(written == 0 && repeat > 1000) {
            return -1;
        }

        total_written += written;
    }

    return total_written;
}

char * receive_message(int fd) {
    char * buffer = (char *) calloc (BLOCK_SIZE + 1, sizeof(char));

    memset(buffer, 1, BLOCK_SIZE + 1);

    // Tracking read errors
    int repeat = 0;
    int total_read = 0;

    // Hacky, but a do while doesn't seem to work
    while(1) {
        int read_size = read(fd, buffer + total_read, BLOCK_SIZE);

        // Error when reading
        if(read_size < 0) {
            buffer[0] = 0;
            return buffer;
        }

        // Prevent hanging on empty reads continuously
        repeat++;
        if (read_size == 0 && repeat > 1000) {
            buffer[0] = 0;
            return buffer;
        }

        // Empty message, return the empty buffer
        total_read += read_size;
        if(total_read == 0) {
            buffer[0] = 0;
            return buffer;
        }

        // Null terminator was found, whole message was read
        if(buffer[total_read-1] == 0) {
            break;
        }

        // We have not received the whole message, but our buffer is full
        // Reallocate and continue
        if(read_size == BLOCK_SIZE) {
            char * new_buffer = (char *) realloc(buffer, total_read+BLOCK_SIZE);
            memset(buffer + total_read, 1, BLOCK_SIZE + 1);
            buffer = new_buffer;
        }
    }

    return buffer;
}