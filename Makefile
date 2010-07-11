CC = gcc
LD = gcc
RM = rm -f
CFLAGS = -g -Wall
SRC = rwt.c ben.c
OBJ = $(SRC:=.o)

.PHONY: all build clean test rebuild retest
all: build

build: rwt

rwt : $(OBJ)
	$(LD) -o $@ $^

%.c.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean :
	$(RM) rwt $(OBJ)

test : rwt
	./rwt test.torrent -a 'new_tracker_omg' -p '*' -wf out.torrent
	./rwt out.torrent -p '*'

rebuild: | clean build
retest: | rebuild test
