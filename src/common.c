#include <stdlib.h>
#include <stdio.h>

#include "common.h"


void * _malloc_nofail(size_t s, char* file, int line) {
    void *p = malloc(s);
    if(!p) {
	fprintf(stderr, "Failed to allocate memory at %s:%d\n", file, line);
	abort();
    }
}


int address_difference(in_addr_t a, in_addr_t b) {
    return (ntohl(a) - ntohl(b));
}

in_addr_t add_to_address(in_addr_t a, int amount) {
    return htonl(ntohl(a) + amount);
}
	
