.PHONY: build

TARGET=prg/hello.cob

run: build
	./main $(TARGET)

debug: build
	gdb --args ./main $(TARGET)

build:
	gcc -g3 src/*.c -o main

