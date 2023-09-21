all: master slave

master: master.c
	gcc -o master master.c

slave: slave.c
	gcc -o slave slave.c

