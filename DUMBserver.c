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

int create_server(char * port);
int get_type(char * input);
int receive_command(int sockfd);
void unknown_command(int sockfd);
void GDBYE_command(int sockfd, char * command);
void CREAT_command(int sockfd, char * command);
void OPNBX_command(int sockfd, char * command);
void NXTMG_command(int sockfd, char * command);					
void PUTMG_command(int sockfd, char * command);
void DELBX_command(int sockfd, char * command);
void CLSBX_command(int sockfd, char * command);

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

	int sockfd, client;

	//this will create a socket that is binded and listening
	sockfd = create_server(port);
	if(sockfd == -1){
		printf("Failed to create server.\n");
		return 0;
	}
	
	struct sockaddr_storage client_addr;
	socklen_t addr_len = sizeof(client_addr);

	//loop to accept each connection
	//make a new thread for each client
	while(client = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len)){
		if(client == -1){
			printf("accept error\n");
		}else{
			printf("Successfully connected!\n");
		}
		
		char hello[10];
		bzero(hello, 10);		
		recv(client, hello, 10, 0);		
		if(strcmp(hello, "HELLO")==0){
			char response[] = "HELLO DUMBv0 ready!";
			send(client, response, sizeof(response), 0);
			//make a new thread
		}else{
			char response[] = "ER:WHAT?";
			send(client, response, sizeof(response), 0);
			//do not make a new thread
		}

		//infinite loop that receives commands
		while(1){
			int type = receive_command(client);
			printf("%d\n", type);			
			if(type == 0){
				//GDBYE received, end loop and exit thread
				break;
			}		
		}
		
		//exit thread

	}
	

	//Things to remember for implementation:
	//1)put a lock around create and delete
	//2)put a nonblocking lock around an open call


	
	//for each new thread
	//int open_box = 0; //indicates if there is a box currently open
	//char box_name [25]; //stores name of open box	

    	return 0;
}

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
		printf("socket error\n");
	}

	//bind socket
	int status = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(status != 0){
		printf("bind error\n");
		sockfd = -1;
	}

	//listen
	int max = 10; //max number of sockets stored in a queue for accept
	status = listen(sockfd, max);
	if(status != 0){
		printf("listen error\n");
		sockfd = -1;
	}
	
	printf("Now listening\n");
	freeaddrinfo(res);

	return sockfd;
}

int receive_command(int sockfd){
	
	//for now:
	int max = 4096, type;
	char buff[max];
	bzero(buff, max);			
	recv(sockfd, buff, max, 0);//receive command
	type = get_type(buff); //find type of command

	//testing
	//printf("%s\n", buff);
	//printf("%d\n", type);

	//deal with command
	switch(type){
		case -1: unknown_command(sockfd);
			break;
		case 0: GDBYE_command(sockfd, buff);
			break;
		case 1: CREAT_command(sockfd, buff);
			break;
		case 2: OPNBX_command(sockfd, buff);
			break;
		case 3: NXTMG_command(sockfd, buff);
			break;
		case 4: PUTMG_command(sockfd, buff);
			break;
		case 5: DELBX_command(sockfd, buff);
			break;
		case 6: CLSBX_command(sockfd, buff);
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

void unknown_command(int sockfd){
	char buff[] = "ER:WHAT?";
	send(sockfd, buff, sizeof(buff), 0);
}

void GDBYE_command(int sockfd, char * command){
	//verify command is in correct format
	if(strcmp(command, "GDBYE")==0){
		close(sockfd);
	}else{
		unknown_command(sockfd);
	}	
}

void CREAT_command(int sockfd, char * command){
	if(valid_name(command)==-1){
		unknown_command(sockfd);
	}else{
		//call create box funtion
		//send "OK!" on success

		int test = 0;
		if(test==1){
			char buff[] = "OK!";
			send(sockfd, buff, sizeof(buff), 0);

		//or "ER:EXIST" if box with that name exists
		}else{
			char buff[] = "ER:EXIST";
			send(sockfd, buff, sizeof(buff), 0);
		}
	}
}

void OPNBX_command(int sockfd, char * command){

}

void NXTMG_command(int sockfd, char * command){

}
					
void PUTMG_command(int sockfd, char * command){

}

void DELBX_command(int sockfd, char * command){

}

void CLSBX_command(int sockfd, char * command){

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

