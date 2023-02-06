CC       = gcc
CFLAGS   = -Wall

all: tokenring

tokenring: tokenring.c
	$(CC) $(CFLAGS) -o tokenring tokenring.c

clean:
	rm -f tokenring

pipes:
	rm -f /tmp/pipe*