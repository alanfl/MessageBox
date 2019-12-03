FLAGS= -Wall -Werror -fsanitize=address -std=c99
CC=gcc
LIBS = -lm -lpthread

all: DUMBserver.o DUMBclient.o
	$(CC) $(FLAGS) -o $@ $^
	
