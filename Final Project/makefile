CC = gcc
CFLAGS = -Wall -pedantic-errors -std=gnu99 -g
LIBS = -lm -lrt -pthread
DEPS = client.h helper.h server.h servant.h
CLIENT_OBJ = client.o helper.o
SERVER_OBJ = server.o helper.o
SERVANT_OBJ = servant.o helper.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

all: client server servant

client: $(CLIENT_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

server: $(SERVER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
servant: $(SERVANT_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
clean:
	rm -f *.o client helper server servant