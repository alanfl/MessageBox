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
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int create_server(char * port);
void * thread_driver(void * input);
int get_type(char * input);
int receive_command(int sockfd, char * address);
void unknown_command(int sockfd, char * address);
void GDBYE_command(int sockfd, char * command, char * address);
void CREAT_command(int sockfd, char * command, char * address);
void OPNBX_command(int sockfd, char * command, char * address);
void NXTMG_command(int sockfd, char * command, char * address);			
void PUTMG_command(int sockfd, char * command, char * address);
void DELBX_command(int sockfd, char * command, char * address);
void CLSBX_command(int sockfd, char * command, char * address);
void report_event(char * address, char * event);
void report_error(char * address, char * error);

struct arguments{
	int sockfd;
	char * addr;
};

int main(int argc, char** argv) {
	//check args
	if(argc > 2){
		printf("Error: Too many arguments\n");
		return 0;
	}else if(argc < 2){
		printf("Error: Not enough arguments\n");
		return 0;
	}	
	
	//set port var
	char * port = argv[1];
	int portnumber = atoi(port);
	if(portnumber < 4000){
		printf("Port number is too small.\n");
		return 0;
	}

	int sockfd, client;

	//this will create a socket that is binded and listening
	sockfd = create_server(port);
	if(sockfd == -1){
		printf("Failed to create server.\n");
		return 0;
	}
	
	struct sockaddr_in client_addr;
	int len = sizeof(client_addr);

	pthread_t thread;

	//loop to accept each connection
	//make a new thread for each client
	while(1){
		client = accept(sockfd, (struct sockaddr*)&client_addr, &len);
		if(client == -1){
			printf("accept() failed\n");//remove
			continue;//there was an error, move onto next connection
		}

		//successfully made a connection, output connection info
		//create a new thread and loop back to accept()

		//find the time for output purposes
		char curr_time[12];
		time_t now = time(NULL);
		struct tm * tptr = localtime(&now);
		strftime(curr_time, 12, "%H%M %d %b", tptr);	
		
		//obtain IP address in a string to pass to 
		//the thread for server output
		char * addr = malloc(sizeof(char) * 20);
		bzero(addr, sizeof(addr));
		addr = inet_ntoa(client_addr.sin_addr);

		//send connection to stdout
		printf("%s %s connected\n", curr_time, addr);
		
		//create the struct that will be sent to the thread
		struct arguments * args = malloc(sizeof(struct arguments));
		args->sockfd = client;
		//malloc args->addr and copy the address into it
		args->addr = malloc(strlen(addr)+1);
		strcpy(args->addr, addr);

		//create a new thread
		pthread_create(&thread, NULL, thread_driver, (void*)args);

		//detach thread so it can exit on its own
		pthread_detach(thread);

	}
	
	//Things to remember for implementation:
	//1)put a lock around create and delete
	//2)put a nonblocking lock around an open call
	
    	return 0;
}

//creates a socket, binds it, and sets it to listen for connections
//returns -1 if there is any error and ends the program
int create_server(char * port){
	//create addrinfo structs for getaddrinfo
	struct addrinfo hints, *res;
	hints.ai_flags = AI_PASSIVE; //allows socket to bind and accept	
	hints.ai_family = AF_INET; //only ipv4
	hints.ai_socktype = SOCK_STREAM;
	
	getaddrinfo(NULL, port, &hints, &res);

	//create socket
	int sockfd; 
	sockfd = socket(res->ai_family, SOCK_STREAM, 0);
	if(sockfd == -1){
		return sockfd;
	}

	//bind socket
	int status = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(status != 0){
		return -1;
	}

	//listen
	int max = 10; //max number of sockets stored in a queue for accept
	status = listen(sockfd, max);
	if(status != 0){
		return -1;
	}
	
	freeaddrinfo(res);
	return sockfd;
}

void * thread_driver(void * input){
	//get the file descriptor and the IP address
	struct arguments * args = (struct arguments *)input;
	int client = args->sockfd;
	char * address = args->addr;	
	
	char hello[10];
	bzero(hello, 10);		
	recv(client, hello, 10, 0);		
	if(strcmp(hello, "HELLO")==0){
		char response[] = "HELLO DUMBv0 ready!";
		send(client, response, sizeof(response), 0);
		report_event(address, "HELLO");
	}else{
		//received something other than HELLO, exit early
		char response[] = "ER:WHAT?";
		send(client, response, sizeof(response), 0);
		report_error(address, "ER:WHAT?");
		close(client);
		pthread_exit(NULL);
	}

	//infinite loop that receives commands
	while(1){
		int type = receive_command(client, address);
		//printf("%d\n", type);			
		if(type == 0){
			//user disconnected, end loop and exit thread
			report_event(address, "disconnected");
			break;
		}		
	}

	//user has disconnected, exit thread	
	//free args
	free(args->addr);
	free(args);
	pthread_exit(NULL);
}

//function that receives, interprets, and returns commands
int receive_command(int sockfd, char * address){
	
	int max = 4096, type;
	char buff[max];
	bzero(buff, max);	
	//receive command
	int test = recv(sockfd, buff, max, 0);		
	if(test == 0 || test == -1){
		//user has disconnected or there was an error
		//patch to the flaw of user disconnecting
		close(sockfd);
		return 0;
	}

	type = get_type(buff); //find type of command


//debug
////////////////////////////////////////////////////////////////////
	//??? I have no idea why but unless this is here, 
	//after the first disconnect get_type fails
	char tstr[max];
////////////////////////////////////////////////////////////////////	
	
	//deal with command
	switch(type){
		case -1: unknown_command(sockfd, address);
			break;
		case 0: GDBYE_command(sockfd, buff, address);
			break;
		case 1: CREAT_command(sockfd, buff, address);
			break;
		case 2: OPNBX_command(sockfd, buff, address);
			break;
		case 3: NXTMG_command(sockfd, buff, address);
			break;
		case 4: PUTMG_command(sockfd, buff, address);
			break;
		case 5: DELBX_command(sockfd, buff, address);
			break;
		case 6: CLSBX_command(sockfd, buff, address);
			break;
	}

	return type;
}


//returns an int corresponding to the command
int get_type(char * input){
	char comp[5];
	int i;
	for(i = 0; i < 5; i++){
		comp[i] = input[i];
	}

	//do not need to check for HELLO command
	if(strcmp(comp, "GDBYE")==0){
		return 0;
	}else if(strcmp(comp, "CREAT")==0){
		return 1;
	}else if(strcmp(comp, "OPNBX")==0){
		return 2;
	}else if(strcmp(comp, "NXTMG")==0){
		return 3;
	}else if(strcmp(comp, "PUTMG")==0){
		return 4;
	}else if(strcmp(comp, "DELBX")==0){
		return 5;
	}else if(strcmp(comp, "CLSBX")==0){
		return 6;
	}else{
		return -1; //unknown command
	}
}

void unknown_command(int sockfd, char * address){
	char buff[] = "ER:WHAT?";
	send(sockfd, buff, sizeof(buff), 0);
	report_error(address, "ER:WHAT?");
}

void GDBYE_command(int sockfd, char * command, char * address){
	report_event(address, "GDBYE");	
	//verify command is in correct format
	if(strcmp(command, "GDBYE")==0){
		close(sockfd);//closes client socket

///////////////////////////////////////////////////////////////
		//make sure any open box is closed
	}else{
		unknown_command(sockfd, address);
	}	
}

void CREAT_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "CREAT");	

	if(valid_name(command)==-1){
		unknown_command(sockfd, address);
	}else{
		//blocking mutex around create
		//call create box funtion
		//unlock mutex
		

		//send "OK!" on success
		int test = 0;
		if(test==1){
			char buff[] = "OK!";
			send(sockfd, buff, sizeof(buff), 0);

		//or "ER:EXIST" if box with that name exists
		}else{
			char buff[] = "ER:EXIST";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:EXIST");
		}
	}
}

void OPNBX_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "OPNBX");	
	
	if(valid_name(command)==-1){
		unknown_command(sockfd, address);
	}else{
		//call open box funtion
		//mutexes will be involved here
				
		//return outcome of box


		//send "OK!" on success
		int test = 2;
		if(test==1){
			char buff[] = "OK!";
			send(sockfd, buff, sizeof(buff), 0);

		//"ER:NEXST" if box with that name does not exist
		}else if(test==2){
			char buff[] = "ER:NEXST";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:NEXST");

		//"ER:OPEND" if box is already open
		}else if(test==3){
			char buff[] = "ER:OPEND";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:OPEND");
		}
	}
}

void NXTMG_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "NXTMG");	
		
	//if any additional args, send WHAT?	
	if(strlen(command) > 5){
		unknown_command(sockfd, address);
	}else{

		//call get next message
		//turn char * with message into output format
		
		//send "OK!arg0!msg" on success
		int test = 1;
		if(test==1){
			char buff[] = "OK!5!Hello";//temp
			send(sockfd, buff, sizeof(buff), 0);

		//"ER:EMPTY" if box with that name does not exist
		}else if(test==2){
			char buff[] = "ER:EMPTY";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:EMPTY");

		//"ER:NOOPN" if box is already open
		}else if(test==3){
			char buff[] = "ER:NOOPN";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:NOOPN");
		}
	}
}
				
void PUTMG_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "PUTMG");	

	//call put message function


	int test = 1;
	//successful
	if(test==1){
		char buff[] = "OK!";
		send(sockfd, buff, sizeof(buff), 0);

	//"ER:NOOPN" if there is no box open
	}else if(test==2){
		char buff[] = "ER:NOOPN";
		send(sockfd, buff, sizeof(buff), 0);
		report_error(address, "ER:NOOPN");

	//"ER:WHAT?" if format is incorrect
	}else if(test==3){
		char buff[] = "ER:WHAT?";
		send(sockfd, buff, sizeof(buff), 0);
		report_error(address, "ER:WHAT?");
	}
}

void DELBX_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "DELBX");	
	
	if(valid_name(command)==-1){
		unknown_command(sockfd, address);
	}else{
		//call delete box funtion
		//mutexes will be involved here

		//send "OK!" on success
		int test = 2;
		if(test==1){
			char buff[] = "OK!";
			send(sockfd, buff, sizeof(buff), 0);

		//"ER:NEXST" if box with that name does not exist
		}else if(test==2){
			char buff[] = "ER:NEXST";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:NEXST");

		//"ER:OPEND" if box is already open
		}else if(test==3){
			char buff[] = "ER:OPEND";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:OPEND");
		
		//"ER:NOTMT" if box is already open
		}else if(test==4){
			char buff[] = "ER:NOTMT";
			send(sockfd, buff, sizeof(buff), 0);
			report_error(address, "ER:NOTMT");
		}
	}

}

void CLSBX_command(int sockfd, char * command, char * address){
	//output received command regardless of success or failure
	report_event(address, "CLSBX");	

	//chek if valid command	
	if(valid_name(command)==-1){
		unknown_command(sockfd, address);
	}else{
		//call close box funtion
		
	

		//send "OK!" on success
		int test = 2;
		if(test==1){
			char buff[] = "OK!";
			send(sockfd, buff, sizeof(buff), 0);

		//"ER:NOOPN" if box is not open
		}else if(test==2){
			char buff[] = "ER:NOOPN";
			send(sockfd, buff, sizeof(buff), 0);
			report_event(address, "ER:NOOPN");	
		}
	}
}


//makes sure that the name of the box is in correct format
//returns -1 if there is an issue, otherwise returns 1
int valid_name(char * command){
	//if longer than 25 char or shorter than 5, it is not a valid name
	//first 6 characters will be "XXXXX "
	if(strlen(command) > 31 || strlen(command) < 11){
		return -1;
	}
	//make sure there is a space after the command
	if(isspace(command[5])==0){
		return -1;
	}
	//make sure first character is a letter
	if(isalpha(command[6])==0){
		return -1;
	}

	return 1;
}

//print all errors to stdout
void report_event(char * address, char * event){
	//find current time
	char curr_time[12];
	time_t now = time(NULL);
	struct tm * tptr = localtime(&now);
	strftime(curr_time, 12, "%H%M %d %b", tptr);
	printf("%s %s %s\n", curr_time, address, event);
}

//print all errors to stderr
void report_error(char * address, char * error){
	//find current time
	char curr_time[12];
	time_t now = time(NULL);
	struct tm * tptr = localtime(&now);
	strftime(curr_time, 12, "%H%M %d %b", tptr);
	fprintf(stderr, "%s %s %s\n", curr_time, address, error);
}
