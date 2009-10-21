#include <netinet/in.h>

//debug routine
#define INFO(m, args...) printf("INFO %s:%d " m, __FILE__, __LINE__, ##args)

#define malloc_nofail(s) _malloc_nofail(s, __FILE__, __LINE__)

int address_difference(in_addr_t a, in_addr_t b);

in_addr_t add_to_address(in_addr_t a, int amount);
