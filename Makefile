include .env

CC?=gcc
CFLAGS_DEBUG?=-g -pedantic -std=c99 -Wall -Wextra -Wmissing-prototypes -Wold-style-definition
CFLAGS_RELEASE?=-O2 -std=c99
CFLAGS?=${CFLAGS_DEBUG}
PLATFORM?=Debug

SRC:=$(wildcard src/*.c src/**/*.c src/**/**/*.c src/**/**/**/*.c src/**/**/**/**/*.c)
OBJ:=$(patsubst src/%.c, src/%.o, $(SRC))

.PHONY=build
build:
	mkdir -p bin
	mkdir -p bin/${PLATFORM}
	${foreach file, ${SRC}, ${CC} ${CFLAGS} -c ${file} -o ${patsubst src/%.c, src/%.o, ${file}} &&} echo
	${CC} ${CFLAGS} ${OBJ} -o bin/${PLATFORM}/main ${RPATH}

.PHONY=debug
debug:
	make build
	
.PHONY=release
release:
	make build CFLAGS="${CFLAGS_RELEASE}" PLATFORM="Release"

.PHONY=clean
clean:
	rm -f ${OBJ}
	rm -f bin/Debug/main
	rm -f bin/Release/main

.PHONY=valgrind
valgrind:
	valgrind --leak-check=full bin/Debug/main &> valgrind.txt
