CC=gcc

all: build

build: 
	$(CC) src/nuon.c src/map.c src/graph.c src/query.c src/token.c src/parser.c src/exec.c -o bin/nuon

clean:
	rm -rf bin/nuon