all: master slave

master: master.c
	gcc -o master.out master.c

slave: slave.c
	gcc -o slave.out slave.c

clean:
	rm -f *.o master.out slave.out
	

