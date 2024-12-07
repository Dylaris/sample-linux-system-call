all: server client

server: server.c
	gcc -std=c99 -Wall -Wextra $< -o $@
client: client.c
	gcc -std=c99 -Wall -Wextra $< -o $@

clean: 
	rm server client server-fifo

.PHONY: all clean
