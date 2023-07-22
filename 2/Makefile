GCC_FLAGS = -Wextra -Werror -Wall -Wno-gnu-folding-constant -g
FILES = solution.c pipe_list.c command.c utils.c parser.c evaluator.c

all: solution.c pipe_list.c command.c utils.c parser.c evaluator.c
	gcc $(GCC_FLAGS) $(FILES)

leaks:
	gcc -ldl -rdynamic ../utils/heap_help/heap_help.c $(FILES)

clean:
	rm a.out
