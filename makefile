all: client server

clean:
	rm client.exe
	rm server.exe
	
client: client.c
	gcc -std=c99 -Wall -g -pedantic -DENDEBUG -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -o client client.c
server: server.c
	gcc -std=c99 -Wall -g -pedantic -DENDEBUG -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -o server server.c