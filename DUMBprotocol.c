//
// Created by alanfl on 12/3/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef struct message {
    struct message * next;
    char * message_body;
} message;

typedef struct message_box {
    struct message_box * next;
    pthread_t owner; // Clients are abstracted as threads
    message * head;
    char * name;
} message_box;

char * create_box(char * name);

char * delete_box(char * name);

char * open_box(char * name);

char * close_box(char * name);

char * add_message(char * name, char * message);

char * get_next_message(char * name);

char * dump_box();

