all: srr

srr : srr.o srr_memory.o disk.o
	gcc srr.o srr_memory.o disk.o -o srr `pkg-config fuse --cflags --libs`
	
srr.o :srr.c
	gcc -c srr.c -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`	

mount:
	./srr -f /home/shreya/Desktop/pro2
	
unmount :
	sudo umount /home/shreya/Desktop/pro2	
		
memory_func.o : srr_memory.c
	gcc -c srr_memory.c
	
disk.o : disk.c
	gcc -c disk.c	
clean :
	rm -rf *.o	