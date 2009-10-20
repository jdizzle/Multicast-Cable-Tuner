
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include "vlc_launcher.h"

//the command template for vlc
#define PATH_TO_VLC "/root/vlc/vlc"

static char* vlc_args[] = {"vlc", "v4l2:///dev/video0", ":input-slave=oss:////dev/dsp2", "--sout", NULL, NULL};
static char** vlc_transcode_arg = NULL;


#define VLC_DEBUG(l, m, args...) do {if(debug_level >= l) printf("VLC %s:%d - " m "\n", __FILE__, __LINE__, ##args); }while(0)
static int debug_level = 0;


void vlc_init() {
    char* c = getenv("VLC_DEBUG");
    if(c)
	debug_level = atoi(c);
    VLC_DEBUG(1, "set debug level to %d", debug_level);

    vlc_transcode_arg = &vlc_args[sizeof(vlc_args)/sizeof(*vlc_args) - 2]; //make this pointer the spare spot in the args array
}

static const char* convert_vcodec(enum vcodec v) {
    switch(v) {
	case MP4V:
	    return "mp4v";
    }
}

static const char* convert_acodec(enum acodec a) {
    switch(a) {
	case MP3:
	    return "mp3";
    }
}


/**
 * fork the vlc process and return it's pid
 */
int vlc_launch(enum vcodec v, enum acodec a, struct in_addr dst, __u16 port) {
    char cbuf[8000];
    int pid, r;
    
    snprintf(cbuf, sizeof(cbuf), "#transcode{vcodec=%s,acodec=%s}:standard{access=udp,dst=%s:%d}", convert_vcodec(v), convert_acodec(a), inet_ntoa(dst), port);

    VLC_DEBUG(3, "transcode became: %s", cbuf);

    *vlc_transcode_arg = cbuf;

    if(debug_level > 2) {
	VLC_DEBUG(3, "args are as follows:");
	char** c;
	for(c = vlc_args; *c; c++) {
	    VLC_DEBUG(3, "%s", *c);
	}
    }
	    
    
    pid = fork();
    if(pid == 0) {
	if(debug_level < 2) {
	    VLC_DEBUG(1, "closing standard out and standard error");
	    fclose(stdout);
	    fclose(stderr);
	}
	execv(PATH_TO_VLC, vlc_args);
	perror("error execing");
	exit(1);
    } 
    VLC_DEBUG(3, "Got pid %d", pid);
    return pid;
}
