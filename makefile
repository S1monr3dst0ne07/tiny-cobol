.PHONY: build

run: build
	./main prg/fib.cob

debug: build
	gdb --args ./main prg/fib.cob

build:
	gcc -g3 src/*.c -o main

