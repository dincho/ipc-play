#Compiler binary
CC=gcc
#Compiler flags: all warnings, c99 standart
CFLAGS=-Wall -std=c99

all: writer reader

writer: writer.c
	$(CC) $(CFLAGS) writer.c -o writer

reader: reader.c
	$(CC) $(CFLAGS) reader.c -o reader

clean:
	rm -rf *o writer reader