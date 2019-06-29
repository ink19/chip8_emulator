CC=gcc
CFLAGS+= -std=c99 -g
LDFLAGS+= -lSDL2

all: main

sdl_test: sdl_test.c

main: main.o chip8_funcs.o

clear:
	rm -rf *.o main sdl_test