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

int create_server(char * port);
int get_type(char * input);

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
		
		char buff[4096];
		bzero(buff, sizeof(buff));
		recv(client, buff, sizeof(buff), 0);
		if(strcmp(buff, "HELLO")==0){
			char response[] = "HELLO DUMBv0 ready!";
			send(client, response, sizeof(response), 0);
		}else{
			char response[] = "SOME ERROR";
			send(client, response, sizeof(response), 0);
		}

		//insert infinite loop that receives commands
			
	}





	//for each new thread
	int open_box = 0; //indicates if there is a box currently open
	char box_name [25]; //stores name of open box	
	
	//run a loop for the thread to receive commands until told to stop
	while(1){

		//receive and interpret command
		//char * input = recv(); //store message from client
		char * input = "HELLO"; //temp input without connection

		//copy command to determine its type while leaving out args
		char * substr;
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
		2)put a nonblocking lock around an open call
		*/
	
	}

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


