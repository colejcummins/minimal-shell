CC = gcc
CFLAGS = -o mush -Wall -pedantic -lm

all : 
	$(CC)  $(CFLAGS) mush.c parseline.c parseline.h

clean :
	rm *.o $(MAIN) core
