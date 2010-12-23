all	:	cap
debug : cap-debug

CFLAGS = -Wall -Werror -std=gnu99 -pedantic

cap	: main.o
	gcc $(CFLAGS) -o cap main.o -lxine

cap-debug	:	main.o
	gcc $(CFLAGS) -g -DDEBUG -o cap main.c -lxine

clean:
	-rm cap *.o
