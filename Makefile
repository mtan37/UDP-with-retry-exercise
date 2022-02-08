CC = gcc
CFLAGS=-Wall
OBJ_SERVER = server.o udp_communication.o 
OBJ_CLIENT = client.o udp_communication.o 
all: server client
%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o
	rm server
	rm client
