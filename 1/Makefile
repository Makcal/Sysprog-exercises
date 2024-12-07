GCC_FLAGS = -Wextra -Werror -Wall -Wno-gnu-folding-constant -O2

all: build/sort

build/solution: build/libcoro.o build/solution.o
	cd build; \
	  gcc $(GCC_FLAGS) libcoro.o solution.o -o solution

build/sort: build/libcoro.o build/sort.o
	cd build; \
	  gcc $(GCC_FLAGS) libcoro.o sort.o -o sort

build/libcoro.o: libcoro.c
	gcc $(GCC_FLAGS) libcoro.c -c -o build/libcoro.o

build/solution.o: solution.c
	gcc $(GCC_FLAGS) solution.c -c -o build/solution.o

build/sort.o: sort.c
	gcc $(GCC_FLAGS) sort.c -c -o build/sort.o

clean:
	rm build/*

