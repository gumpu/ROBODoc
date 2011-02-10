CFLAGS = -ansi -pedantic -O2 -Wall -I /home/frans/usr/local/include 
LFLAGS = -L /home/frans/usr/local/lib -llua -ldl -lm

robodoc : robodoc.o
	gcc lpeg-0.10.1/lpeg.o robodoc.o -o robodoc $(LFLAGS)

robodoc.o : robodoc.c
	gcc $(CFLAGS) -c robodoc.c -o robodoc.o

clean :
	-rm robodoc.o robodoc
