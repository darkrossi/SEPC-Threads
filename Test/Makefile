CC=gcc -g

all: main allocateur mutex

main: main.c
	$(CC) -o main main.c -lpthread
	./main

allocateur: allocateur.c
	$(CC) -o allocateur allocateur.c -lpthread

mutex: mutex.c
	$(CC) -o mutex mutex.c -lpthread
	./mutex		

clean:
	rm main allocateur