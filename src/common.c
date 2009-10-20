
#include <stdlib.h>
#include <stdio.h>


void * _malloc_nofail(size_t s, char* file, int line) {
    void *p = malloc(s);
    if(!p) {
	fprintf(stderr, "Failed to allocate memory at %s:%d\n", file, line);
	abort();
    }
}
	
