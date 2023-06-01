CC=g++
CFLAGS=-g -I.
DEPS = Socket.h ConectaCuatro.h
OBJ = Socket.o ConectaCuatro.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: CC_Server CC_Client

CC_Server: $(OBJ) ConectaCuatro_Server.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

CC_Client: $(OBJ) ConectaCuatro_Client.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o CC_Server CC_Client