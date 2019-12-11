//
// Created by alanfl and jmagnes362 on 12/3/19.
//

#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BLOCK_SIZE 4096

// Usage: Translates an error message from the server into a readable error msg
void translate_error(char * error, char * input);

// Usage: Prints all the available commands to the terminal
void print_commands();

// Utility: Gets input from the user terminal
char * get_input();

// Usage: Acquires handshake from server and begins listening to user input
void create_session(int session_fd);

int main(int argc, char** argv) {
	//check args
	if(argc != 3) {
        printf("Error: incorrect number of arguments.\n");
        printf("Usage: DUMBclient host_ip port_number\n");
        exit(0);
    }

	// Assign variables
	char * host = argv[1];
	int port = atoi(argv[2]);

	int sock_fd, session_fd;
	struct sockaddr_in server_addr;

	// Create socket and check errors
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
	    printf("Error: could not create socket.\n");
	    exit(0);
	}

	// Prep for connection
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons((unsigned short) port);

	// Connect to the server socket
	if(connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0) {
	    // Error connecting
	    printf("Error: connection to the server failed.\n");
	    exit(0);
	}

	// Begin communicating with the server
	create_session(sock_fd);

	// Close the socket when finished
	close(sock_fd);
}

void create_session(int session_fd) {
    // Define reading buffer
    size_t buffer_size = BLOCK_SIZE;
    char * buffer = (char *) calloc(buffer_size, sizeof(char));
    char * response;

    // Initiate with protocol handshake
    send_message(session_fd, "HELLO");
    response = receive_message(session_fd);

    if(strcmp(response, "HELLO DUMBv0 ready!") != 0) {
        printf("Error: Could not connect to server.\n");
        if(response) free(response);
        if(buffer) free(buffer);
        return;
    }

    printf("Success! Connected to server.\n");
    if(response) free (response);
    response = NULL;

    print_commands();

    char * input; // User input
    while(1) {
        printf("> ");
        input = get_input();

        // quit
        if(strcmp(input, "quit") == 0) {
            send_message(session_fd, "GDBYE");
            response = receive_message(session_fd);

            if(strlen(response) == 0) {
                printf("Disconnected from the server.\n");
                if(response) free(response);
                if(buffer) free(buffer);
                return;
            } else {
                printf("Error disconnecting from the server.\n");
            }
        }
        // create
        else if(strcmp(input, "create") == 0) {
            free(input);
            printf("Okay, what is the name of the new message box?\n");
            printf("create:> ");
            input = get_input();
            memset(buffer, 0, buffer_size);
            sprintf(buffer, "CREAT %s", input);

            send_message(session_fd, buffer);
            response = receive_message(session_fd);

            // Successful creation
            if(strcmp(response, "OK!") == 0) {
                printf("Message box '%s' was successfully created.\n", input);
            }
            // Error, translate and print
            else {
                translate_error(response, input);
            }
        }
        // delete
        else if(strcmp(input, "delete") == 0) {
            free(input);
            printf("Okay, which box do you want to delete?\n");
            printf("delete:> ");
            input = get_input();
            memset(buffer, 0, buffer_size);
            sprintf(buffer, "DELBX %s", input);

            send_message(session_fd, buffer);
            response = receive_message(session_fd);

            // Delete was successful
            if(strcmp(response, "OK!") == 0) {
                printf("Message box '%s' was successfully deleted.\n", input);
            }
            // Error
            else {
                translate_error(response, input);
            }
        }
        // open
        else if(strcmp(input, "open") == 0) {
            free(input);
            printf("Okay, which box do you want to open?\n");
            printf("open:> ");
            input = get_input();
            memset(buffer, 0, buffer_size);
            sprintf(buffer, "OPNBX %s", input);

            send_message(session_fd, buffer);
            response = receive_message(session_fd);

            // Open successful
            if(strcmp(response, "OK!") == 0) {
                printf("Message box '%s' was successfully opened.\n", input);
            }
            // Error opening
            else {
                translate_error(response, input);
            }
        }
        // close
        else if(strcmp(input, "close") == 0) {
            free(input);
            printf("Okay, which box do you want to close?\n");
            printf("close:> ");
            input = get_input();
            memset(buffer, 0, buffer_size);
            sprintf(buffer, "CLSBX %s", input);

            send_message(session_fd, buffer);
            response = receive_message(session_fd);

            // Open successful
            if (strcmp(response, "OK!") == 0) {
                printf("Message box '%s' was successfully closed.\n", input);
            }
                // Error opening
            else {
                translate_error(response, input);
            }
        }
        // put
        else if(strcmp(input, "put") == 0) {
            free(input);
            printf("Okay, what is your message?\n");
            printf("put:> ");
            input = get_input();

            // Message is too long for the buffer and message
            // Must reallocate
            if (buffer_size < strlen(input) + 64) {
                buffer_size = strlen(input) + 64;
                char *new_buffer = (char *) realloc(buffer, buffer_size);
                buffer = new_buffer;
            }

            memset(buffer, 0, buffer_size);
            sprintf(buffer, "PUTMG!%lu!%s", strlen(input), input);

            // Expected response from the server
            char expected[32];
            sprintf(expected, "OK!%lu", strlen(input));

            send_message(session_fd, buffer);
            response = receive_message(session_fd);

            if (strcmp(response, expected) == 0) {
                printf("Message: '%s' was added to the message box.\n", input);
            } else {
                translate_error(response, NULL);
            }
        }
        // next
        else if(strcmp(input, "next") == 0) {

            send_message(session_fd, "NXTMG");
            response = receive_message(session_fd);

            if(strncmp(response, "OK", 2) == 0) {
                int index = 0;
                size_t response_length = strlen(response);

                while(index < response_length-1 && response[index++] != '!');
                char * message = response + index;
                while(index < response_length-1 && response[index++] != '!');
                response[index-1] = 0;

                int message_size = atoi(message);
                message = response + index;

                if(strlen(message) == message_size) {
                    printf("Success! Message: '%s'\n", message);
                } else {
                    printf("Error: message size and message conflict.\n");
                }
            } else {
                translate_error(response, NULL);
            }
        }
	// help
        else if(strcmp(input, "help") == 0) {
	    print_commands();

	}
        // all others are unknown
        else {
            printf("Unknown command: '%s', valid commands are:\n", input);
            print_commands();
        }

        if(input) {
            free(input);
            input = NULL;
        }

        if(response) {
            free(response);
            response = NULL;
        }
    }

    free(buffer);
}

void translate_error(char * error, char * input) {
    if(strcmp(error, "ER:NEXST") == 0) {
        printf("Error: message box '%s' does not exist.\n", input);
    } else if(strcmp(error, "ER:EXIST") == 0) {
        printf("Error: message box '%s' already exists.\n", input);
    } else if(strcmp(error, "ER:NOOPN") == 0) {
        if(input != NULL)
            printf("Error: message box '%s' is not open yet.\n", input);
        else
            printf("Error: no message box is open yet.\n");
    } else if(strcmp(error, "ER:EMPTY") == 0) {
        printf("Error: the active message box has no messages.\n");
    } else if(strcmp(error, "ER:NOTMT") == 0) {
        printf("Error: message box '%s' is not empty.\n", input);
    } else if(strcmp(error, "ER:WHAT?") == 0) {
        if(input != NULL) {
            if(isalpha((int) ((char) input[0])) == 0)
                printf("Error: message box name '%s' does not start with a letter.\n", input);
            else if(strlen(input) < 5)
                printf("Error: message box name '%s' does not have at least 5 letters.\n", input);
            else if(strlen(input) > 25)
                printf("Error: message box name '%s' does not have fewer than 25 letters.\n", input);
        } else {
            printf("Error: invalid DUMB protocol command.\n");
        }
    } else {
        printf("%s\n", error);
    }
}

char * get_input() {
    size_t input_size = BLOCK_SIZE;
    char * input;

    input = (char*)calloc(input_size, sizeof(char));

    size_t index = 0;
    while((input[index++] = getchar()) != '\n') {
        if(index == input_size - 1) {
            input_size += BLOCK_SIZE;
            char * new_input = (char *) realloc(input, input_size);

            memset(new_input + index, 0, input_size-index);
            input = new_input;
        }
    }

    input[index - 1] = 0; // Trim newline
    return input;
}

void print_commands() {
    printf("1.   quit: quit the program.\n");
    printf("2. create: create a new message box.\n");
    printf("3.   open: open an existing message box.\n");
    printf("4.    put: put a new message into the active message box.\n");
    printf("5.   next: read next message from the active message box.\n");
    printf("6.  close: close active message box.\n");
    printf("7. delete: delete a message box.\n");
}
