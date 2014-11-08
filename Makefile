
## Created by Anjuta

CC = gcc
CFLAGS = -g -Wall
OBJECTS = server.o
INCFLAGS = 
LDFLAGS = -Wl,-rpath,/usr/local/lib
LIBS = 

## all: Server

Server: $(OBJECTS)
	$(CC) -o Server $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:	.c .cc .C .cpp .o

.c.o :
	$(CC) -o $@ -c $(CFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o

.PHONY: all
.PHONY: count
.PHONY: clean
