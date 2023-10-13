CC = gcc
CFLAGS = -o

SRCS = master.c slave.c
EXES = $(SRCS:.c=.out)
.SUFFIXES: .c .out

.c.out:
	$(CC) $(CFLAGS) $@ $<

all: $(EXES)

clean:
	rm -f $(EXES) 

