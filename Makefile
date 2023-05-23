# Makefile for CS50 nuggets
#
# Selena Han, May 22 2023

S = support
L = libcs50

CC = gcc
MAKE = make


# uncomment the following to turn on server logging to stdout
# TESTING=-DSERVERTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I$S -I$L
OBJS = server.o client.o
LIBS = -lm -lncurses
LLIBS = $S/support.a $L/libcs50-given.a 

.PHONY: all clean

all: 
	cd $L; cp $L-given.a $L.a
	make -C support
	make server

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all


# building the executable
server: $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) server.o $(LLIBS) $(LIBS) -o $@

server.o: $S/grid.h $S/log.h $S/message.h $L/hashtable.h $L/file.h $L/mem.h

client: $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) client.o $(LLIBS) $(LIBS) -o $@

client.o: client.c $L/hashtable.h $L/file.h $L/mem.h


# client: $(OBJS) $(LLIBS)
# 	$(CC) $(CFLAGS) client.o $(LLIBS) $(LIBS) -o $@

# client.o: $S/grid.h $S/log.h $S/message.h $L/mem.h


clean:
	rm -rf *.dSYM
	rm -f *~ *.o
	rm -f server
	rm -f client
	rm -f core
	make -C libcs50 clean
	make -C support clean