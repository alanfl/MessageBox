FLAGS= -Wall -Werror -fsanitize=address -std=c99
CC=gcc

all: server client

server: DUMBserver.o message.o
	$(CC) $(FLAGS) -o DUMBserver $^

client: DUMBclient.o
    $(CC) $(FLAGS) -o DUMBclient $^

%.o: %.c %.h
    	$(CC) $(FLAGS) -c $<

.PHONY: clean

clean:
    rm -f *.o *.i *.bc *.s


