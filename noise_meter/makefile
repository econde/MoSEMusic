main: main.o buff.o values.o process.o filter.o client.o
	gcc -Wall -g -o main main.o buff.o process.o filter.o values.o client.o -lasound -lm -ljansson

main.o: main.c buff.h values.h process.h filter.h client.h
	gcc -g -c main.c 

buff.o: buff.c buff.h
	gcc -g -c buff.c

values.o: values.c values.h
	gcc -g -c values.c

process.o: process.c process.h buff.h values.h 
	gcc -g -c process.c

filter.o: filter.c filter.h buff.h
	gcc -g -c filter.c

client.o: client.c client.h
	gcc -g -c client.c
	
clean:
	rm main *.o
