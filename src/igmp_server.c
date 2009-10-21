
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>


#include <linux/mroute.h>
#include <linux/igmp.h>
#include <linux/if.h>



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "igmp_server.h"

#define VIF_INDEX 1	//only need one index as I only have one interface.  This number is arbitrary

static int igmp_socket = 0;
static int multicast_version = 1;

#define IGMP_DEBUG(l, m, args...) do { if(debug_level >= l) printf("IGMP %s:%d - " m "\n", __FILE__, __LINE__, ##args); }while(0)
static int debug_level = 1;






/**
 *  Will return 0 on success, error on anything else
 */
int igmp_server_init(char* intf) {
    struct in_addr addr;
    struct ip_mreq mreq;
    struct ifreq ifr;
    struct vifctl vif;
    char *c;
    int r;

    c = getenv("IGMP_DEBUG");
    if(c)
	debug_level = atoi(c);
    IGMP_DEBUG(3, "debug level %d", debug_level);

    igmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if(igmp_socket <= 0) {
	IGMP_DEBUG(1, "failed to create socket");
	return 1;
    }
    
    mreq.imr_multiaddr.s_addr = IGMP_ALL_ROUTER;
    mreq.imr_interface.s_addr = INADDR_ANY;
    r = setsockopt(igmp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if(r) {
	IGMP_DEBUG(1, "failed to join multicast group");
	return r;
    }

    
    r = setsockopt(igmp_socket, IPPROTO_IP, MRT_INIT, &multicast_version, sizeof(int));
    if(r) {
	IGMP_DEBUG(1, "failed to initialize multicast router");
	return r;
    }

    IGMP_DEBUG(3, "quering address for interface %s", intf);
    strncpy(ifr.ifr_ifrn.ifrn_name, intf, IFNAMSIZ);
    r = ioctl(igmp_socket, SIOCGIFADDR, &ifr);
    if(r == -1) {
	IGMP_DEBUG(1, "failed to ioctl for address");
	return 1;
    }

    addr = ((struct sockaddr_in*)&(ifr.ifr_ifru.ifru_addr))->sin_addr;
    IGMP_DEBUG(3, "got address %s", inet_ntoa(addr));
    
    vif.vifc_vifi = VIF_INDEX;
    vif.vifc_flags = 0;
    vif.vifc_threshold = 1;
    vif.vifc_rate_limit = 0;
    vif.vifc_lcl_addr = addr;
    vif.vifc_rmt_addr.s_addr = 0;
    r = setsockopt(igmp_socket, IPPROTO_IP, MRT_ADD_VIF, &vif, sizeof(vif));
    if(r) {
	IGMP_DEBUG(1, "failed to add the virtual address %s", inet_ntoa(addr));
	return r;
    }

    return 0;

}


struct cb_le {
    u_int8_t type;
    union {
	u_int8_t val;
	short    meta;
    } code;
    igmp_cb cb;
    struct cb_le* next;
};

static struct cb_le* cb_head = NULL; //list of callbacks

/**
 * Register a callback for the particular type and code igmp messages
 */
void igmp_register_cb(u_int8_t type, short code, igmp_cb cb) {
    struct cb_le* le = (struct cb_le*)malloc_nofail(sizeof(struct cb_le));
    
    IGMP_DEBUG(5, "Registering callback at %p", cb);
    le->type = type;
    le->code.meta = code;
    le->cb = cb;
    le->next = cb_head;
    cb_head = le;
}


/**
 * receive an igmp packet and call appropriately
 */
int igmp_server_recv() {
    char buf[4096];
    struct iphdr* ip_hdr = (struct iphdr*)buf;
    struct igmphdr* igmp_hdr;
    struct cb_le* le;
    struct in_addr grpaddr, srcaddr;
    int r;

    IGMP_DEBUG(4, "waiting for packet");
    r = recv(igmp_socket, buf, sizeof(buf), 0);
    if(r < 0) {
	IGMP_DEBUG(1, "error on recieving packet");
	return r;
    }
    
    srcaddr.s_addr = ip_hdr->saddr;
    IGMP_DEBUG(5, "Packet from %s, proto: %d", inet_ntoa(srcaddr), ip_hdr->protocol);

    igmp_hdr = (struct igmphdr*)(buf + (ip_hdr->ihl *4)); //get the actual igmp header
    grpaddr.s_addr = igmp_hdr->group;
    IGMP_DEBUG(3, "Received igmp packet with type:%x, code:%x, group:%s", igmp_hdr->type, igmp_hdr->code, inet_ntoa(grpaddr));


    for(le = cb_head; le; le = le->next) {
	if(igmp_hdr->type == le->type) {
	    if(igmp_hdr->code == le->code.val || (le->code.meta == IGMP_CODE_ANY)) {
		IGMP_DEBUG(3, "Matched callback at %p", le->cb);
		(le->cb)(igmp_hdr->type, igmp_hdr->code, grpaddr, srcaddr);
	    }
	}
    }

    return 0;

}
	
int igmp_server_run() {
    int r;
    do {
	r = igmp_server_recv();
    } while(!r);

    IGMP_DEBUG(1, "server receive failed");
    return r;
}
