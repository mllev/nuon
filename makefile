CC=gcc

all: build

build:
	$(CC) src/main.c src/nuon.c src/picoev_kqueue.c -o bin/nuon

clean:
	rm -rf bin/nuon