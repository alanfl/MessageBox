//
// Created by alanfl and jmagnes362 on 12/3/19.
//

#include "network.h"
#include "messagebox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define BLOCK_SIZE 4096

void log_message(char * message);

void * create_session(void * fd);

void log_message(char * message) {
    time_t now;
    time(&now);

    // Something other than ok, log it
    if(strncmp(message, "OK", 2) != 0) {
        // Some kind of error, log as an error
        if(strncmp(message, "ER:", 3) == 0) {
            fprintf(stderr, "%lu %.19s %s\n", (unsigned long) pthread_self(), ctime(&now), message);
        }
        // Some kind of msg:, log it
        else if(strncmp(message, "msg:", 4) == 0) {
            fprintf(stdout, "%lu %.19s %s\n", (unsigned long) pthread_self(), ctime(&now), message + 4);
        }
        // Something else, log it
        else {
            fprintf(stdout, "%lu %.19s %s\n", (unsigned long) pthread_self(), ctime(&now), message);
        }
    }
}

void * create_session(void * fd) {
    log_message("msg:connected");
    int session_fd = *(int *) fd;
    char * handshake;

    // Initiate handshake from client
    handshake = receive_message(session_fd);
    if(strcmp(handshake, "HELLO") != 0) {
        // Something went wrong with the handshake, log it and clear
        log_message("ER:BAD HANDSHAKE");
        free(handshake);
        close(session_fd);
        return NULL;
    }

    // Good handshake, confirm client ready
    send_message(session_fd, "HELLO DUMBv0 ready!");
    log_message("msg:HELLO");   // Log connection

    // Only allow a user one active box at a time
    char active_box[64];
    char * response;

    // Begin processing commands from client
    while(1) {
        // Await command
        char *command = receive_message(session_fd);
        log_message(command);

        // E.1 GDBYE
        if (strlen(command) == 0 || strcmp(command, "GDBYE") == 0) {
            log_message("msg:disconnected");
            break;
        }
            // E.2 CREAT
        else if (strncmp(command, "CREAT", 5) == 0) {
            response = create_box(command + 6); // 6 from start should be the box name (hopefully)
        }
            // E.6 DELBX
        else if (strncmp(command, "DELBX", 5) == 0) {
            response = delete_box(command + 6);
        }
            // E.3 OPNBX
        else if (strncmp(command, "OPNBX", 5) == 0) {
            response = open_box(command + 6);

            // Prevent opening multiple boxes by a single context
            if (strncmp(response, "OK", 2) == 0) {
                // If there exists an active box, and that active box isn't the same as the requested box
                if (strlen(active_box) > 0 && strcmp(active_box, command + 6) != 0) {
                    // Close the active box then
                    close_box(active_box);
                }
                // Mark active box as empty, make active box requested box
                memset(active_box, 0, sizeof(active_box));
                strcpy(active_box, command + 6);
            }
        }
            // E.7 CLSBX
        else if (strncmp(command, "CLSBX", 5) == 0) {
            response = close_box(command + 6);
        }
            // E.5 PUTMG
        else if (strncmp(command, "PUTMG", 5) == 0) {
            size_t index = 0;
            size_t command_size = strlen(command);

            // Iterate until ! to calculate offset
            while (index < command_size - 1 && command[index++] != '!');
            // Extract message
            char * message = command + index;
            // Ensure proper seperation
            while (index < command_size - 1 && command[index++] != '!');
            command[index - 1] = 0;

            int message_size = atoi(message);
            message = command + index;

            // Expecting the message length to be the same as the computed size
            if (strlen(message) == message_size) {
                // Insert message to the open box
                response = insert_message(active_box, message);
            } else {
                response = calloc(64, 1);
                // Something went wrong, WHAT? is fitting here
                sprintf(response, "ER:WHAT?");
            }
        }
            // E.4 NXTMG
        else if (strncmp(command, "NXTMG", 5) == 0) {
            response = get_next_message(active_box);
        }
            // Anything else is a WHAT?
        else {
            response = calloc(64, 1);
            sprintf(response, "ER:WHAT?");
        }

        log_message(response);
        send_message(session_fd, response);

        free(response);
        free(command);
    }

    // On exit, close the active box
    if(strlen(active_box) > 0) {
        close_box(active_box);
    }

    close(session_fd);
    return NULL;
}


int main(int argc, char** argv) {
	//check args
	if(argc != 2) {
	    printf("Error: Incorrect number of arguments.\n");
	    printf("Usage: DUMBserver port\n");
	    exit(0);
	}

	int port = atoi(argv[1]);

	if(port < 4096 || port > 65535) {
	    printf("Error: Invalid port number.\n");
	    printf("Port number must be greater than 4096 and less than 65535.\n");
	    exit(0);
	}

	int sock_fd, session_fd, length;
	struct sockaddr_in server_addr, client_addr;

	// Create the socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1){
		printf("Error: failed to create socket.\n");
		exit(0);
	}

	// Setup the IP and the port
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	// Bind the socket to the ip and the specified port
	if((bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) != 0) {
        printf("Error: could not bind socket.\n");
        exit(0);
	}

	// Start listening for connections
	if ((listen(sock_fd, 5)) != 0) {
	    printf("Error: could not listen for incoming connections.\n");
	    exit(0);
	}

	printf("Listening on port: %d, accepting connections from clients.\n", port);

	// Start accepting clients
	// Start new thread for each client
	while(1) {
        length = sizeof(client_addr);

        // Assign a file descriptor to the incoming connection
        session_fd = accept(sock_fd, (struct sockaddr *) &client_addr, (socklen_t *) &length);

        // Check for error
        if (session_fd < 0) {
            printf("Error: could not accept client.\n");

            // Undefined behavior here, will just exit for now.
            exit(0);
        }

        // Now, start a new thread to handle the newly created session
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, create_session, (void *) &session_fd) != 0) {
            close(session_fd);
        }
    }

	// Close the socket
	close(sock_fd);
}