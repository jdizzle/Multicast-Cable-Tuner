#include <netinet/in.h>
#include <linux/igmp.h>

//initializing igmp socket connection
int igmp_server_init(char*);

//begin receiving the packet
int igmp_server_recv();

//callback format for igmp queries
typedef void (*igmp_cb)(__u8 type, __u8 code, struct in_addr group, struct in_addr source);

//register a callback for particular igmp messages
#define IGMP_CODE_ANY (-1)
void igmp_register_cb(u_int8_t, short code, igmp_cb);

//loop receiving igmp packets, calling the appropriate callbacks 
int igmp_server_run();





