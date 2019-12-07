//
// Created by alanfl and jmagnes362 on 12/3/19.
//

#include "DUMBprotocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netdb.h>

int main(int argc, char** argv) {

	//create head of message box linked list
	message_box * head = malloc(sizeof(message_box *));
	head->next = NULL;
	head->head = NULL;
	head->name = "head";

	//create socket

	//bind

	//listen

	//accept

	//upon accept(), create a new thread for the client



	//for each new thread
	int open_box = 0; //indicates if there is a box currently open
	char box_name [25]; //stores name of open box	
	
	//run a loop for the thread to receive commands until told to stop
	while(1){

		//receive and interpret command
		//char * input = recv(); //store message from client
		char * input = "HELLO"; //temp input without connection

		//copy command to determine its type while leaving out args
		char substr [5];
		substr = strncpy(substr, input, 5);
		int type = get_type(substr);
		
		//perform the command

		if(type == -1){ //unknown command
			//send "WHAT?" to client
		
		}else if(type == 0){ //HELLO
			// send "HELLO DUMBv0 ready!" to client

		}else if(type == 1){ //GDBYE
			//if there is an open box, close
			if(open_box){
				//close_box(box_name);
			}

			//end loop and thread
		}
		//implement CREAT through CLSBX


		/*
		Things to remember for implementation:
		1)put a lock around create and delete
		2)put a trylock around an open call to prevent blocking
		*/
	
	}

    	return 0;
}


//returns an int corresponding to the command
int get_type(char * input){
	if(strcmp(input, "HELLO")==0){
		return 0;
	}else if(strcmp(input, "GDBYE")==0){
		return 1;
	}else if(strcmp(input, "CREAT")==0){
		return 2;
	}else if(strcmp(input, "OPNBX")==0){
		return 3;
	}else if(strcmp(input, "NXTMG")==0){
		return 4;
	}else if(strcmp(input, "PUTMG")==0){
		return 5;
	}else if(strcmp(input, "DELBX")==0){
		return 6;
	}else if(strcmp(input, "CLSBX")==0){
		return 7;
	}else{
		return -1; //unknown command
	}
}


