//
// Created by alanfl and jmagnes362 on 12/10/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef struct message {
    struct message * next;  // For linked list
    char * content;         // Actual text in the message
} message_t;

typedef struct message_box {
    struct message_box * next;  // For linked list
    message_t * message_head;     // Message linked list
    message_t * message_tail;     // QOL purposes
    pthread_t user;             // User who has control of the box
    char * name;                // Name of the box
} message_box_t;

char * create_box(char * box_name);
char * delete_box(char * box_name);
char * open_box(char * box_name);
char * close_box(char * box_name);
char * insert_message(char * box_name, char * message);
char * get_next_message(char * box_name);
int is_valid_name(char * box_name);
char * generate_response(char * response);   // Easily craft a response for the client

message_box_t * BOX_HEAD;     // Pointer for list of boxes

pthread_mutex_t box_lock = PTHREAD_MUTEX_INITIALIZER; // need mutex lock for data manipulation

// Returns either the matching box or the last box
message_box_t * find_box(char * name) {
    message_box_t * curr;
    message_box_t * prev = NULL;

    // Iterate the list
    curr = BOX_HEAD;
    while(curr != NULL) {
        if( strcmp(curr->name, name) == 0) {    // Matching names
            return curr;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    return prev;
}

char * create_box(char * box_name) {
    if(is_valid_name(box_name) == 0) {
        return generate_response("ER:WHAT?");   // Malformed message, respond with ER:WHAT?
    }

    char * response;
    pthread_mutex_lock(&box_lock);  // Critical section, block all other code until this box is added
    message_box_t * found_box = find_box(box_name);

    if(found_box != NULL && strcmp(found_box->name, box_name) == 0) {   // Redundant check but do it again
        // Box already exists, do not recreate and response should be an error
        response = generate_response("ER:EXIST");
    } else {
        // No match found, good to create a box of name box_name
        message_box_t * box = (message_box_t *) malloc(sizeof(message_box_t));
        box->next = NULL;
        box->message_head = NULL;
        box->message_tail = NULL;
        box->user = (pthread_t)NULL;
        // Copy name into attribute
        box->name = (char *) malloc(strlen(box_name) + 1);
        strcpy(box->name, box_name);

        // Now insert this box into list
        if(BOX_HEAD == NULL) {  // This is the first message box
            BOX_HEAD = box;
        } else {
            // Otherwise, insert box at the tail of the list
            found_box->next = box;
        }

        response = generate_response("OK!"); // All is well, return OK!
    }

    pthread_mutex_unlock(&box_lock);
    return response;
}

char * delete_box(char * box_name) {
    if(is_valid_name(box_name) == 0) {
        return generate_response("ER:WHAT?");   // Malformed message, respond with ER:WHAT?
    }

    char * response;
    pthread_mutex_lock(&box_lock);  // Critical section, block all other code until this box is removed

    message_box_t * found_box = find_box(box_name);

    // No box found or redundant match was found to be erroneous
    if(found_box == NULL || strcmp(found_box->name, box_name) != 0) {
        response = generate_response("ER:NEXST"); // No match for box_name
    } else if(found_box->user != (pthread_t)NULL) {
        // Box is still open, cannot delete
        response = generate_response("ER:OPEND");
    } else if(found_box->message_head != NULL) {
        // Box is not empty, cannot delete
        response = generate_response("ER:NOTMT");
    } else {
        // Okay to delete

        message_box_t * prev = BOX_HEAD;
        // Found box is the head, must move head
        if(found_box == BOX_HEAD) {
            BOX_HEAD = found_box->next;
        }
        // Found box is not the head, must rearrange pointers
        else {
            while(prev->next != found_box) {
                prev = prev->next;
            }
            prev->next = found_box->next;
        }

        free(found_box->name);
        free(found_box);
        response = generate_response("OK!");
    }

    pthread_mutex_unlock(&box_lock);
    return response;
}

char * open_box(char * box_name) {

    if(is_valid_name(box_name) == 0) {
        return generate_response("ER:WHAT?");   // Malformed message, respond with ER:WHAT?
    }

    char * response;
    pthread_t thread_id = pthread_self();

    // START CRITICAL SECTION
    pthread_mutex_lock(&box_lock);
    message_box_t * found_box = find_box(box_name);

    if(found_box == NULL || strcmp(found_box->name, box_name) != 0) {   // Redundant check but do it again
        // Box does not exist, response should be an error
        response = generate_response("ER:NEXST");
    } else if (found_box->user != 0 && found_box->user != thread_id) {
        // This box is already open, and not by the calling context
        // TODO unsure about repeated opens, should probably also be an ER:OPEND
        response = generate_response("ER:OPEND");
    } else {
        found_box->user = thread_id;

        // TODO what if a user opens the box and then exits the thread or disconnects?

        // TODO can a client open multiple boxes at the same time?

        // TODO maybe cache a pointer to that specific box in local

        response = generate_response("OK!");
    }

    pthread_mutex_unlock(&box_lock);
    // END CRITICAL SECTION
    return response;
}

char * close_box(char * box_name) {
    if(is_valid_name(box_name) == 0) {
        return generate_response("ER:WHAT?");   // Malformed message, respond with ER:WHAT?
    }

    char * response;
    pthread_t thread_id = pthread_self();

    // START CRITICAL SECTION
    pthread_mutex_lock(&box_lock);
    message_box_t * found_box = find_box(box_name);

    if(found_box == NULL || strcmp(found_box->name, box_name) != 0) {   // Redundant check but do it again
        // A found box does not exist, response should be an error
        response = generate_response("ER:NOOPN");
    }
    // Box was not opened by this context, return error
    else if (found_box->user != thread_id) {
        response = generate_response("ER:NOOPN");
    }
    // Okay to close box
    else {
        found_box->user = 0;
        response = generate_response("OK!");
    }

    pthread_mutex_unlock(&box_lock);
    // END CRITICAL SECTION

    return response;
}

char * insert_message(char * box_name, char * message) {
    pthread_t thread_id = pthread_self();
    char * response = NULL;

    // START CRITICAL SECTION
    pthread_mutex_lock(&box_lock);

    message_box_t * found_box = find_box(box_name);

    if(found_box == NULL || strcmp(found_box->name, box_name) != 0) {
        // Box was not found, or redundant check caught an error
        response = generate_response("ER:NOOPN");
    } else if (found_box->user != thread_id) {
        // Box was found, but the opening context is not the calling context
        // Deny and send response
        response = generate_response("ER:NOOPN");
    }

    pthread_mutex_unlock(&box_lock);
    // END CRITICAL SECTION

    // No need to lock here because only one user will
    // ever interact with a particular box at a time

    if(response == NULL) {
        // Create a new message node
        message_t * new_message = (message_t *) malloc(sizeof(message_t));
        new_message->next = NULL;
        new_message->content = (char *) malloc(strlen(message) + 1);
       strcpy(new_message->content, message);

        // Must malloc for "response" in this case, since it's being stored
        response = malloc(20);
        sprintf(response, "%s%lu", "OK!", strlen(message));

        // Now insert message node in the linked list tied to this box
        if(found_box->message_head == NULL) {   // First message in the box, set head equal to it
            found_box->message_head = found_box->message_tail = new_message;
        }
        // Insert at the end otherwise, and rearrange pointers
        else {
            found_box->message_tail->next = new_message;
            found_box->message_tail = new_message;
        }
	
    }

    return response;
}

char * get_next_message(char * box_name) {
    pthread_t thread_id = pthread_self();
    char * response = NULL;

    // BEGIN CRITICAL SECTION
    pthread_mutex_lock(&box_lock);

    message_box_t * found_box = find_box(box_name);

    if(found_box == NULL || strcmp(found_box->name, box_name) != 0) {
        // Box couldn't be found, deny and send response
        response = generate_response("ER:NOOPN");
        return response;
    }
    // Calling context is not the opening context, deny and send response
    else if (found_box->user != thread_id) {
        response = generate_response("ER:NOOPN");
    }

    pthread_mutex_unlock(&box_lock);
    // END CRITICAL SECTION

    // Only one user interacting on a message box at a time
    if(response == NULL) {
        if(found_box->message_head == NULL) {
            // Box has no messages in it
            response = generate_response("ER:EMPTY");
        }
        // Okay to fetch the next message
        else {
            message_t * next_message = found_box->message_head;
            found_box->message_head = next_message->next;

            // This was the only message, tail must now be null
            if(found_box->message_head == NULL) {
                found_box->message_tail = NULL;
            }

            // Allocate for response at size plus what else is needed
            // This is for, protocol response, then length of message, then message itself
            response = (char *) calloc(strlen(next_message->content) + 64, 1);

            // Print into buffer in expected format
            sprintf(response, "%s!%lu!%s", "OK", strlen(next_message->content), next_message->content);

            // Free unused space
            free(next_message->content);
            free(next_message);
        }
    }

    return response;
}

// Usage: private function to validate box names
int is_valid_name(char * name) {
    int valid = 1;
    if( isalpha((int) ((char) name[0])) == 0 || // Name cannot be empty
        strlen(name) < 5                     || // Name must be at least 5 characters
        strlen(name) > 25                       // Name cannot exceed 25 characters
    ) {
        valid = 0;
    }
    return valid;
}

char * generate_response(char * response) {
    char * resp = (char *) malloc(strlen(response) + 1);    // Null terminated
    strcpy(resp, response);
    return resp;
}
