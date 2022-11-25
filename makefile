all: server client1 client2

server:
	gcc server.c -o server

client1:
	gcc client1.c -o client1

client2:
	gcc client2.c -o client2

clean:
	rm -f server client1 client2
