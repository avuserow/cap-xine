all	:	cap

CFLAGS = -g -Wall -Werror

cap	: main.o
	gcc $(CFLAGS) -o cap main.o -lxine

clean:
	-rm cap *.o
