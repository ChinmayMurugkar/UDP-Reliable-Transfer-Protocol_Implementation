all: urft-server urft-client 

urft-server: server.o
	gcc server.o -o urft-server

urft-client: client.o
	gcc client.o -o urft-client

server.o: server.c
	gcc -c server.c

client.o: client.c
	gcc -c client.c

clean:
	rm -rf *.o *.out urft-client urft-server 
