CC=gcc

all: build

build: 
	$(CC) src/nuon.c src/query.c src/token.c src/parser.c -o bin/nuon

clean:
	rm -rf bin/nuon