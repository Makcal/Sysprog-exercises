GCC_FLAGS = -Wextra -Werror -Wall -Wno-gnu-folding-constant -O2

all: mine


reference: build/reference/main

build/reference/main: build/reference/libcoro.o build/reference/main.o
	cd build/reference; \
	  gcc $(GCC_FLAGS) libcoro.o main.o -o main

build/reference/libcoro.o: reference/libcoro.c
	gcc $(GCC_FLAGS) reference/libcoro.c -c -o build/reference/libcoro.o

build/reference/main.o: reference/main.c
	gcc $(GCC_FLAGS) reference/main.c -c -o build/reference/main.o


mine: build/mine/main

build/mine/main: build/mine/main.o build/mine/libcoro.o build/heap_help.o
	cd build/mine; \
	  gcc $(GCC_FLAGS) ../heap_help.o libcoro.o main.o -o main

build/mine/main.o: mine/main.c
	gcc $(GCC_FLAGS) mine/main.c -c -o build/mine/main.o

build/mine/libcoro.o: mine/libcoro.c
	gcc $(GCC_FLAGS) mine/libcoro.c -c -o build/mine/libcoro.o


build/heap_help.o: ../utils/heap_help/heap_help.c
	gcc $(GCC_FLAGS) ../utils/heap_help/heap_help.c -c -o build/heap_help.o


clean:
	cd build; \
		rm -rf *; \
		mkdir mine reference

