compile: link
	gcc hw3unnamed.o -o hw3unnamed -lm -pthread -Wall -lrt
	gcc hw3named.o -o hw3named -lm -pthread -Wall -lrt
link:
	gcc -c hw3unnamed.c -pthread
	gcc -c hw3named.c -pthread
clean:
	rm *.o
	rm hw3unnamed
	rm hw3named
