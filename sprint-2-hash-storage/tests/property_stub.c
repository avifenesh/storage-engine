#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Property-test scaffold for your hash engine.
// TODO: replace printf/returns with real asserts once implementation exists.

static unsigned rnd(void) { return (unsigned)rand(); }

int main(int argc, char **argv)
{
    int trials = (argc > 1) ? atoi(argv[1]) : 1000;
    srand((unsigned)time(NULL));
    printf("[hash property stub] running %d trials...\n", trials);
    // TODO: generate key/value pairs, mirror into an oracle map, call your
    // hash_put/hash_get/hash_delete, and compare results.
    printf("[hash property stub] TODO: enable assertions after implementing hash engine\n");
    return 0; // keep passing until you implement
}

