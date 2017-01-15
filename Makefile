CFLAGS=-O3
CC=gcc
EXEC=alloc_mem

help:
	@echo "You must specify one of the following:" 
	@echo "   32bit         -Linux 32 bit binary"
	@echo "   64bit         -Linux 64 bit binary"
	@echo "   clean         -cleanup"

32bit:
	$(CC) -m32 $(CFLAGS) -o $(EXEC) alloc_mem.c

64bit:
	$(CC) $(CFLAGS) -o $(EXEC) alloc_mem.c


clean:
	rm core $(EXEC) *.o 

