#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "igmp_server.h"
#include "vlc_launcher.h"

/*
  the current streaming guys
*/
int 			current_pid 	= 0;
struct in_addr		current_group	= {0};
struct in_addr	 	current_source 	= {0};


void usage(char* arg0) {
    printf("Usage: %s address\n", arg0);
    printf(" address: the address of the interface to listen for multicast traffic on\n");
}

void joined(__u8 type, __u8 code, struct in_addr group, struct in_addr source) {
    int pid;
    printf("Group %s joined by %s\n", inet_ntoa(group), inet_ntoa(source));
    if(!current_pid) {
	printf("spawing new video process\n");
	pid = vlc_launch(MP4V, MP3, group, 1234);
	if(pid == -1) {	
	    perror("failed to fork");
	    exit(1);
	}
	current_pid = pid;
	current_group = group;
	current_source = source;
    }
    
}

void left(u_int8_t type, u_int8_t code, struct in_addr group, struct in_addr source) {
    int r;
    printf("Group %s left by %s\n", inet_ntoa(group), inet_ntoa(source));
    if(current_pid && source.s_addr == current_source.s_addr && group.s_addr == current_group.s_addr) {
	printf("killing process\n");
	r = kill(current_pid, SIGKILL);
	if(r) {
	    perror("failed to kill process");
	    exit(1);
	}
	current_pid = 0;
    }
}

int main(int argc, char** args) {
    char b[4096];
    int r;

    if(argc != 2) {
	usage(args[0]);
	exit(1);
    }
	

    vlc_init();
    r = igmp_server_init(args[1]);
    if(r) {
	perror("igmp server");
	return 1;
    }

    r = setuid(501);
    if(r) {
	perror("setuid");
    }

    igmp_register_cb(IGMP_HOST_MEMBERSHIP_REPORT, IGMP_CODE_ANY, joined);
    igmp_register_cb(IGMPV2_HOST_MEMBERSHIP_REPORT, IGMP_CODE_ANY, joined);
    igmp_register_cb(IGMPV3_HOST_MEMBERSHIP_REPORT, IGMP_CODE_ANY, joined);

    igmp_register_cb(IGMP_HOST_LEAVE_MESSAGE, IGMP_CODE_ANY, left);


    while(1) {
	r = igmp_server_recv(b, sizeof(b));
	if(r) {
	    perror("reading from server");
	    return 1;
	}
    }

    return 0;

}
