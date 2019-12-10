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

int create_socket(char * host, char * port);
int get_type(char * input);
void help_command();
int quit_command();
void create_command();
void open_command();
void next_command();
void put_command();
void delete_command();
void close_command();
void send_message(int sockfd, int type, char input[]);
int get_command(int count);

int main(int argc, char** argv) {
	//check args
	if(argc > 3){
		printf("Error: Too many arguments\n");
		return 0;
	}else if(argc < 3){
		printf("Error: Not enough arguments\n");
		return 0;
	}	

	//set host and port variables
	char * host = argv[1];
	char * port = argv[2];

	int i, sockfd;
	//loop that goes three times attempting to connect to server
	//if it fails three times, report error and close client program
	for(i = 0; i < 3; i++){
		sockfd = create_socket(host, port);
		if(sockfd != -1){
			break;

		//failed all three times
		}else if(i == 3){
			printf("Error. Failed to connect to server.\n");		
			return 0;
		}	
	}
	

	char hello[] = "HELLO";
	send(sockfd, hello, sizeof(hello), 0);
		
	char buffer[20];
	bzero(buffer, sizeof(buffer));
	recv(sockfd, buffer, sizeof(buffer), 0);
	if(strcmp(buffer, "HELLO DUMBv0 ready!")==0){
		printf("Success! Connection established.\n");
	}else{
		printf("Error. Connection terminated.\n");
		return 0;
	}

	int cmdcount = 0;
	//wait for user commands until told to stop	
	while(1){
		int type = get_command(cmdcount);
		cmdcount++;

		//char input[10];
		//scanf("%s", input); //cannot detect a space in input
		//int type = get_type(input);
		int test = 0;

		switch(type){
			case -1: printf("That is not a command, for a command list enter 'help'.\n");
				break;
			case 0: help_command();
				break;
			case 1: test = quit_command(sockfd);
				break;
			case 2: create_command(sockfd);
				break;
			case 3: open_command(sockfd);
				break;
			case 4: next_command(sockfd);
				break;
			case 5: put_command(sockfd);
				break;
			case 6: delete_command(sockfd);
				break;
			case 7: close_command(sockfd);
				break;
		}

		//after disconnecting from the server close
		if(test == 1){
			break;
		}
	}

	return 0;
}

//function that creates the socket and attempts to connect to the server
//returns socket file descriptor if successful and -1 if unsuccessful
int create_socket(char * host, char * port){
	
	//create addrinfo structs for getaddrinfo
	struct addrinfo hints, *res;
	hints.ai_family = AF_INET; //only ipv4
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(host, port, &hints, &res);

	//create socket
	int sockfd, status; 
	sockfd = socket(res->ai_family, SOCK_STREAM, 0);

	//connect to server
	status = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if(status == -1){
		sockfd = -1;
	}

	freeaddrinfo(res);
	return sockfd;
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

int quit_command(int sockfd){
	//send quit command
	//if receive a response report an error
	//otherwise successful and program will end
	char buff[] = "GDBYE";
	send(sockfd, buff, sizeof(buff), 0);
	bzero(buff, sizeof(buff));
	int test = recv(sockfd, buff, sizeof(buff), 0);
	
	
	//check if there is no response
	if(test!=0){
		printf("Error disconnecting.\n");
		return -1;
		//keep running because you are still connected
	}else{
		printf("Success! You have been disconnected from the server.\n");
		close(sockfd);
		return 1;
	}
}

void create_command(int sockfd){
	printf("Okay, what is the name of the message box?\nCreate: ");
	
	int max = 4096;
	char input[max], message[max], temp;
	//clear last newline/enter
	scanf("%c", &temp);
	scanf("%[^\n]", input);
	
	//function that sends the correct command
	send_message(sockfd, 1, input);

	//clear buffer to receive new message
	bzero(message, max);
	recv(sockfd, message, max, 0);

	//if successful
	if(strcmp(message, "OK!")==0){
		printf("Success! Message box '%s' has been created.\n", input);
	
	//EXIST returned
	}else if(strcmp(message, "ER:EXIST")==0){
		printf("Error. Message box '%s' already exists.\n", input);

	//WHAT? returned
	}else if(strcmp(message, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	}
}


void open_command(int sockfd){
	printf("Okay, open which message box?\nOpen: ");
	
	int max = 4096;
	char input[max], message[max], temp;
	//clear last newline/enter
	scanf("%c", &temp);
	scanf("%[^\n]", input);
	
	//function that sends the correct command
	send_message(sockfd, 2, input);

	//clear buffer to receive new message	
	bzero(message, max);
	recv(sockfd, message, max, 0);
	
	//if successful
	if(strcmp(message, "OK!")==0){
		printf("Success! Message box '%s' is now open.\n", input);

	//NEXST returned
	}else if(strcmp(message, "ER:NEXST")==0){
		printf("Error. Message box '%s' does not exist.\n", input);

	//OPEND returned
	}else if(strcmp(message, "ER:OPEND")==0){
		printf("Error. Message box '%s' is in use by another user.\n", input);

	//WHAT? returned
	}else if(strcmp(message, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	}		
}


void next_command(int sockfd){
	printf("Okay, getting message.\n");

	//interact with server	
	char msg[] = "NXTMG";
	send(sockfd, msg, sizeof(msg), 0);

	int max = 4096;
	char buff[max];
	bzero(buff, sizeof(buff));
	recv(sockfd, buff, sizeof(buff), 0);

	//this is more complicated than the others 
	//have to interpret response

	//EMPTY returned
	if(strcmp(buff, "ER:EMPTY")==0){
		printf("Error. No messages left in the box.\n");

	//NOOPN returned
	}else if(strcmp(buff, "ER:NOOPN")==0){
		printf("Error. There is no message box open.\n");

	//WHAT? returned
	}else if(strcmp(buff, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	
	//OK!	
	}else{
		//OK!arg0!msg returned
		int i, size, index;
		char length [6];
		//loop to get length of message
		for(i = 3; i < 9; i++){
			if(buff[i] == '!'){
				length[i-3] = '\0';
				break;
			}
			length[i-3] = buff[i];
		}
		//convert string to int
		index = i + 1; //first index of message
		size = atoi(length);
		
		char response[size+1];//+1 for '\0'
		for(i = 0; i < size;i++){
			response[i] = buff[i + index];
		}
		response[size] = '\0';	
		printf("%s\n", response);
	}	
}

void put_command(int sockfd){
	printf("Okay, insert message\nPut: ");
	int max = 4096;	
	char input [max], message[max], temp;
	scanf("%c", &temp);
	scanf("%[^\n]", input);
	
	//build message string
	strcpy(message, "PUTMG!");
	int size = strlen(input);//find length of message
	char length[6]; 
	sprintf(length, "%d", size);//convert length to string
	strcat(message, length);
	strcat(message, "!");
	strcat(message, input);
	
	//send to server
	send(sockfd, message, sizeof(message), 0);
	//refresh buffer	
	bzero(message, sizeof(message));
	//receive reply
	recv(sockfd, message, sizeof(message), 0);

	//if successful
	if(strcmp(message, "OK!")==0){
		printf("Success! Message added.\n");

	//NOOPN returned
	}else if(strcmp(message, "ER:NOOPN")==0){
		printf("Error. There is no message box open.\n");

	//WHAT? returned
	}else if(strcmp(message, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	}	
}

void delete_command(int sockfd){
	printf("Okay, delete which message box?\nDelete: ");
	
	int max = 4096;
	char input[max], message[max], temp;
	//clear last newline/enter
	scanf("%c", &temp);
	scanf("%[^\n]", input);
	
	//function that sends the correct command
	send_message(sockfd, 3, input);
	
	//clear buffer to receive new message
	bzero(message, max);
	recv(sockfd, message, max, 0);

	//if successful
	if(strcmp(message, "OK!")==0){
		printf("Success! Message box '%s' has been deleted.\n", input);

	//NEXST returned
	}else if(strcmp(message, "ER:NEXST")==0){
		printf("Error. Message box '%s' does not exist.\n", input);

	//OPEND returned
	}else if(strcmp(message, "ER:OPEND")==0){
		printf("Error. Message box '%s' is open.\n", input);

	//NOTMT returned
	}else if(strcmp(message, "ER:NOTMT")==0){
		printf("Error. Message box '%s' still has messages.\n", input);

	//WHAT? returned
	}else if(strcmp(message, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	}	
}

void close_command(int sockfd){
	printf("Okay, close which message box?\nClose: ");
	
	int max = 4096;
	char input[max], message[max], temp;
	//clear last newline/enter
	scanf("%c", &temp);
	scanf("%[^\n]", input);
	
	//function that sends the correct command
	send_message(sockfd, 4, input);
	
	//clear buffer to receive new message
	bzero(message, max);
	recv(sockfd, message, max, 0);

	//if successful
	if(strcmp(message, "OK!")==0){
		printf("Success! Message box '%s' is now closed.\n", input);

	//NOOPN returned
	}else if(strcmp(message, "ER:NOOPN")==0){
		printf("Error. Message box '%s' is not open.\n", input);

	//WHAT? returned
	}else if(strcmp(message, "ER:WHAT?")==0){
		printf("Error. Command was unsuccessful, please try again.\n");
	}		
}

//sends the correctly formatted message for create, open, delete, and close
//					       1      2      3          4
void send_message(int sockfd, int type, char input[]){
	int max = 4096;
	char message[max];
	bzero(message, sizeof(message));

	//edit message based on the type
	switch(type){
		case 1: strcpy(message, "CREAT ");
			break;
		case 2: strcpy(message, "OPNBX ");
			break;
		case 3: strcpy(message, "DELBX ");
			break;
		case 4: strcpy(message, "CLSBX ");
			break;
	}
	
	strcat(message, input);
	
	printf("Sent: %s\n", message);

	//interact with server	
	send(sockfd, message, max, 0);	
}

int get_command(count){
	int max = 4096;
	char input[max];
	bzero(input, sizeof(input));
	if(count > 0){
		char temp;
		scanf("%c", &temp);
	}

	//char input[max], temp;
	//bzero(input, sizeof(input));
	//scanf("%c", &temp);
	scanf("%[^\n]", input);
	int type = get_type(input);
	return type;
}
