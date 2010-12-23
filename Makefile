all	:	cap

CFLAGS = -g -Wall -Werror -std=gnu99 -pedantic

cap	: main.o
	gcc $(CFLAGS) -o cap main.o -lxine

clean:
	-rm cap *.o
