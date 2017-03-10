#include <stdio.h>
#include <stdlib.h>

int add (int a, int b);

int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int r = add(1, 2);
    printf("r = %d\n", r);
}
