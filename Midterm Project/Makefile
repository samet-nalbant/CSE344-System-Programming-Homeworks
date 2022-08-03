compile: link
	gcc client.o -o client -lm -pthread
	gcc serverY.o -o serverY -lm -pthread -lrt
	gcc serverZ.o -o serverZ -lm -pthread -lrt
link:
	gcc -c client.c
	gcc -c serverZ.c
	gcc -c serverY.c
clean:
	rm *.o
	rm client
	rm serverY
	rm serverZ
