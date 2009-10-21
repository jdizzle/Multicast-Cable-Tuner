#include <netinet/in.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "channel.h"
#include "igmp_server.h"
#include "vlc_launcher.h"



void usage(char* arg0) {
    printf("Usage: %s [options]\n", arg0);
    printf(" -v dev : set the tuner device (default /dev/video0)\n");
    printf(" -a dev : set the audio device (default /dev/dsp1)\n");
    printf(" -b intf: set interface to listen for multicast traffic on (default eth0)\n"); 
    printf(" -m addr: set the base multicast address (address of channel 0) (default 224.0.100.0)\n");
    printf(" -c cmd : set the path to the vlc command (default 'vlc')\n");
    printf(" -h     : print this message\n");
}


int main(int argc, char** argv) {
    int r, c;

    /*
     * defaults
     */
    char* vdev = "/dev/video0";
    char* adev = "/dev/dsp1";
    char* intf = "eth0";
    char* cmd  = "vlc";
    struct in_addr mbase = {htonl(0xe0006400)}; //default multicast base is 224.0.100.0

    while((c = getopt(argc, argv, "v:a:b:m:c:h")) != -1)
    {
	switch(c) {
	    case 'v':
		vdev = optarg;
		break;
	    case 'a':
		adev = optarg;
		break;
	    case 'b':
		intf = optarg;
		break;
	    case 'm':
		mbase.s_addr = inet_addr(optarg);
		break;
	    case 'c':
		cmd = optarg;
		break;
	    case 'h':
		usage(argv[0]);
		exit(0);
		break;
	}
    }

    

    r = vlc_init(cmd, vdev, adev);
    if(r) {
	perror("vlc");
	return 1;
    }

    r = igmp_server_init(intf);
    if(r) {
	perror("igmp server");
	return 1;
    }

    r = channel_init(mbase, vdev);
    if(r) {
	perror("channel control");
	return 1;
    }
			 

    r = setuid(-2); //drop privileges to "nobody"
    //    r = setuid(501); //XXX hardcode for now
    if(r) {
	perror("setuid");
	return 1;
    }



    r = igmp_server_run();
    if(r) {
	perror("error running igmp listener");
	return 1;
    }

    return 0;
}
