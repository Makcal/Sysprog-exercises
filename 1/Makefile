GCC_FLAGS = -Wextra -Werror -Wall -Wno-gnu-folding-constant -O2

all: build/main


reference: build/reference/main

build/reference/main: build/reference/libcoro.o build/reference/main.o
	cd build/reference; \
	  gcc $(GCC_FLAGS) libcoro.o main.o -o main

build/reference/libcoro.o: reference/libcoro.c
	gcc $(GCC_FLAGS) reference/libcoro.c -c -o build/reference/libcoro.o

build/reference/main.o: reference/main.c
	gcc $(GCC_FLAGS) reference/main.c -c -o build/reference/main.o


mine: build/mine/main

build/mine/main: build/mine/main.o
	cd build/mine; \
	  gcc $(GCC_FLAGS) main.o -o main

build/mine/main.o: mine/main.c
	gcc $(GCC_FLAGS) mine/main.c -c -o build/mine/main.o

clean:
	cd build; \
		rm -rf *; \
		mkdir mine reference

