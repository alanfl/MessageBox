//
// Created by alanfl and jmagnes362 on 12/10/19.
//

#ifndef ASSIGNMENT3_NETWORK_H
#define ASSIGNMENT3_NETWORK_H

// Usage: writes a message to specified file descriptor
int send_message(int fd, char * message);

// Usage: reads a message from specified file descriptor
char * receive_message(int fd);

#endif //ASSIGNMENT3_NETWORK_H
