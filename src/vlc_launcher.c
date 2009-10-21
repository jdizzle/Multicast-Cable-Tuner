#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "vlc_launcher.h"



/*
 * information required to launch vlc
 */
static char* vlc_command = NULL;
static char* vlc_vdev	 = NULL;
static char* vlc_adev	 = NULL;


static pid_t	vlc_pid	= 0; //current vlc pid


#define VLC_DEBUG(l, m, args...) do {if(debug_level >= l) printf("VLC %s:%d - " m "\n", __FILE__, __LINE__, ##args); }while(0)
static int debug_level = 0;

static const char* convert_vcodec(enum vcodec v) {
    switch(v) {
	case MP4V:
	    return "mp4v";
	case MP2V:
	    return "mp2v";
    }
}

static const char* convert_acodec(enum acodec a) {
    switch(a) {
	case MP3:
	    return "mp3";
    }
}


static void child_died(int s) {
    VLC_DEBUG(4, "child terminated callback called");
    if(vlc_pid) {
	VLC_DEBUG(1, "vlc process died unexpectedly");
	vlc_pid = 0;
    }
}

int vlc_kill() {
    if(vlc_pid) {
	int r;

	VLC_DEBUG(2, "killing spawned vlc process %d", vlc_pid);
	r = kill(vlc_pid, SIGTERM);
	if(r == -1) {
	    VLC_DEBUG(1, "failed to kill spawned process");
	    return -1;
	}

	vlc_pid = 0;
	waitpid(0, NULL, 0);

    } else {
	VLC_DEBUG(3, "no vlc process to kill");
    }

    return 0;
}

/*
 * fork the vlc process and return it's pid
 */
int vlc_launch(enum vcodec v, enum acodec a, struct in_addr dst, __u16 port) {
    char *vlcargs[7];
    char vbuf[500];
    char isbuf[500];
    char tcbuf[500];
    int pid, r;
    
    

    snprintf(vbuf, sizeof(vbuf), "v4l2://%s", vlc_vdev);
    snprintf(isbuf, sizeof(isbuf), ":input-slave=oss://%s", vlc_adev);
    snprintf(tcbuf, sizeof(tcbuf), "#transcode{vcodec=%s,acodec=%s}:standard{access=udp,dst=%s:%d}", convert_vcodec(v), convert_acodec(a), inet_ntoa(dst), port);

    //build the exec aguments array
    vlcargs[0] = vlc_command;
    vlcargs[1] = "-Idummy"; //no UI spawn
    vlcargs[2] = vbuf;
    vlcargs[3] = isbuf;
    vlcargs[4] = "--sout";
    vlcargs[5] = tcbuf;
    vlcargs[6] = NULL;

    if(debug_level > 2) {
	VLC_DEBUG(3, "args are as follows:");
	char** c;
	for(c = vlcargs; *c; c++) {
	    VLC_DEBUG(3, "'%s'", *c);
	}
    }
	    
    
    pid = fork();
    if(pid == 0) {
	if(debug_level < 5) {
	    VLC_DEBUG(1, "closing standard out and standard error");
	    fclose(stdout);
	    fclose(stderr);
	}
	//	extern char** environ;
	//	environ = NULL;
	execvp(vlcargs[0], vlcargs);
	perror("error execing vlc");
	exit(1);
    } 
    vlc_pid = pid;
    VLC_DEBUG(3, "Got pid %d", pid);
    return pid;
}


int vlc_init(char* command, char* vdev, char* adev) {
    char* c = getenv("VLC_DEBUG");
    if(c)
	debug_level = atoi(c);
    VLC_DEBUG(1, "set debug level to %d", debug_level);

    VLC_DEBUG(4, "using command %s, vdev %s, adev %s", command, vdev, adev);
    vlc_command = strdup(command);
    vlc_vdev	= strdup(vdev);
    vlc_adev	= strdup(adev);

    if(!vlc_command || !vlc_vdev || !vlc_adev) {
	VLC_DEBUG(1, "failed to allocate memory for the strings!");
	return -1;
    }


    if(signal(SIGCHLD, child_died) == SIG_ERR) {
	VLC_DEBUG(1, "failed to register signal handler");
	return SIG_ERR;
    }

    return 0;
    
}

