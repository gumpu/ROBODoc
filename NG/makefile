
robodoc : robodoc.o
	gcc -L /home/frans/usr/local/lib robodoc.o -llua -lm

robodoc.o : robodoc.c
	gcc -O2 -Wall -I /home/frans/usr/local/include -c robodoc.c -o robodoc.o

clean :
	-rm robodoc.o robodoc
