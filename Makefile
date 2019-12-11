FLAGS=-Wall -Werror -g -fsanitize=address
CC=gcc
# CC=cc

all: server client

server: DUMBserver.o messagebox.o network.o
	$(CC) $(FLAGS) -o DUMBserver $^

client: DUMBclient.o network.o
	$(CC) $(FLAGS) -o DUMBclient $^

test: test.o multi-thread.o messagebox.o
	$(CC) $(FLAGS) -o $@ $^

%.o: %.c %.h network.h messagebox.h
	$(CC) $(FLAGS) -c $<

.PHONY: clean

clean:
	rm -f *.o *.i *.bc *.s DUMBclient DUMBserver