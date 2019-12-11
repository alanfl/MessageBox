//
// Created by alanfl and jmagnes362 on 12/10/19.
//

#ifndef ASSIGNMENT3_MESSAGEBOX_H
#define ASSIGNMENT3_MESSAGEBOX_H

// Usage: Appends a new box of box_name at the end
// of the linked list of boxes
char * create_box(char * box_name);

// Usage: Deletes box with box_name from the linked list
char * delete_box(char * box_name);

// Usage: Sets the box's current user to the thread id that called
// this command if possible
// If box is already open, send an error msg and move on
char * open_box(char * box_name);

// Usage: sets the box's current user to none if possible
char * close_box(char * box_name);

// Usage: Adds a message node to the end of the linked list
// for the specified message box
char * insert_message(char * box_name, char * message);

// Usage: Retrieves the next message (head of the linked list),
// prints the content, and then moves the head of the LL to the next node
char * get_next_message(char * box_name);

#endif //ASSIGNMENT3_MESSAGEBOX_H
