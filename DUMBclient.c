//
// Created by alanfl and jmagnes362 on 12/3/19.
//

#include "DUMBprotocol.h"
#include <stdio.h>
#include <string.h>

int get_type(char * input);
void help_command();
void quit_command();
void create_command();
void open_command();
void next_command();
void put_command();
void delete_command();
void close_command();

int main(int argc, char** argv) {
	
	//check args

	//connect to server here using argv

	//for now, assume it was successful
	printf("HELLO DUMBv0 ready!\n");

	//wait for user commands until told to stop	
	while(1){
		char input[10];
		scanf("%s", input);
		int type = get_type(input);

		switch(type){
			case -1: printf("That is not a command, for a command list enter 'help'.\n");
				break;
			case 0: help_command();
				break;
			case 1: quit_command();
				break;
			case 2: create_command();
				break;
			case 3: open_command();
				break;
			case 4: next_command();
				break;
			case 5: put_command();
				break;
			case 6: delete_command();
				break;
			case 7: close_command();
				break;
		}

		//after disconnecting from the server close
		if(type == 1){
			break;
		}
	}

	return 0;
}



/*
quit		(which causes: E.1 GDBYE)
create		(which causes: E.2 CREAT)
delete		(which causes: E.6 DELBX)			
open		(which causes: E.3 OPNBX)
close		(which causes: E.7 CLSBX)
next		(which causes: E.4 NXTMG)
put		(which causes: E.5 PUTMG)
help		return 0
unknown		return -1
*/


//returns a number corresponding to the command
int get_type(char * input){
	if(strcmp(input, "quit")==0){
		return 1;
	}else if(strcmp(input, "create")==0){
		return 2;
	}else if(strcmp(input, "delete")==0){
		return 6;
	}else if(strcmp(input, "open")==0){
		return 3;
	}else if(strcmp(input, "close")==0){
		return 7;
	}else if(strcmp(input, "next")==0){
		return 4;
	}else if(strcmp(input, "put")==0){
		return 5;
	}else if(strcmp(input, "help")==0){
		return 0;
	}else{
		return -1;
	}
}

//print all the available commands for the user
void help_command(){
	printf("Commands are: 'quit', 'create', 'delete', 'open', 'close', 'next', 'put'\n");
}

void quit_command(){
	//send quit command
	//if receive a response report an error
	//otherwise successful and program will end

}

void create_command(){
	printf("Okay, what is the name of the message box?\nCreate: ");
	char input [50];
	scanf("%s", input);

	//send to server
	//receive reply

	int temp = 0;

	//if successful
	if(temp == 0){
		printf("Success! Message box '%s' has been created.\n", input);
	
	//EXIST returned
	}else if(temp == 1){
		printf("Error. Message box '%s' already exists.\n", input);

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}

}


void open_command(){
	printf("Okay, open which message box?\nOpen: ");
	char input [50];
	scanf("%s", input);
	
	int temp = 0;

	//if successful
	if(temp == 0){
		printf("Success! Message box '%s' is now open.\n", input);

	//NEXST returned
	}else if(temp == 1){
		printf("Error. Message box '%s' does not exist.\n", input);

	//OPEND returned
	}else if(temp == 2){
		printf("Error. Message box '%s' is in use by another user.\n", input);

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}		

}

void next_command(){
	printf("Okay, getting message.\n");
	
	//send to server
	//receive reply

	int temp = 0;

	//if successful
	if(temp == 0){
		//OK!arg0!msg returned
		//print the message

	//EMPTY returned
	}else if(temp == 1){
		printf("Error. No messages left in the box.\n");

	//NOOPN returned
	}else if(temp == 2){
		printf("Error. There is no message box open.\n");

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}	
}

//////////////////////////////////////////////////////////////////
//NEEDS TO BE FIXED
//does not scan more than one word
/////////////////////////////////////////////////////////////////

void put_command(){
	printf("Okay, insert message\nPut: ");
	char input [100];
	scanf("%s", input);
	printf("%s\n", input);

	//send to server
	//receive reply

	
	int temp = 0;

	//if successful
	if(temp == 0){
		printf("Success! Message added.\n");

	//NOOPN returned
	}else if(temp == 2){
		printf("Error. There is no message box open.\n");

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}	
}

void delete_command(){
	printf("Okay, delete which message box?\nDelete: ");
	char input [50];
	scanf("%s", input);
	
	//send to server
	//receive reply

	int temp = 0;

	//if successful
	if(temp == 0){
		printf("Success! Message box '%s' has been deleted.\n", input);

	//NEXST returned
	}else if(temp == 1){
		printf("Error. Message box '%s' does not exist.\n", input);

	//OPEND returned
	}else if(temp == 2){
		printf("Error. Message box '%s' is open.\n", input);

	//NOTMT returned
	}else if(temp == 3){
		printf("Error. Message box '%s' still has messages.\n", input);

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}	

}

void close_command(){
	printf("Okay, close which message box?\nClose: ");
	char input [50];
	scanf("%s", input);

	//send to server
	//receive reply
	
	int temp = 0;

	//if successful
	if(temp == 0){
		printf("Success! Message box '%s' is now closed.\n", input);

	//NOOPN returned
	}else if(temp == 1){
		printf("Error. Message box '%s' is not open.\n", input);

	//WHAT? returned
	}else{
		printf("Error. Command was unsuccessful, please try again.\n");
	}		

}


