OBJECT_MODE := 32
CC=gcc
#CFLAGS=-I. 
CFLAGS=-I. -ggdb -Wall 

all:
	$(CC) $(CFLAGS) testdir_fork.c testdir_fio.c testdir.c fstouch.c -o fstouch 
help:
	@echo "make {help|all|install}"
install: 
	cp -v fstouch /ssg/bin/fstouch 
