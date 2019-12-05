//
// Created by alanfl and jmagnes362 on 12/3/19.
//

#ifndef ASSIGNMENT3_DUMBPROTOCOL_H
#define ASSIGNMENT3_DUMBPROTOCOL_H

char * create_box(char * name);

char * delete_box(char * name);

char * open_box(char * name);

char * close_box(char * name);

char * add_message(char * name, char * message);

char * get_next_message(char * name);

char * dump_box();

#endif //ASSIGNMENT3_DUMBPROTOCOL_H
