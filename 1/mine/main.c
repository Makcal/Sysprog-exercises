#include <stddef.h>
#include <stdio.h>
#include <err.h>

int main(int argc, char **) {
    if (argc <= 2) {
        fprintf(stderr, "Not enough arguments!\n");
        return 1;
    }
}

