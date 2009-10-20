#include <netinet/in.h>
#include <linux/types.h>


enum vcodec {
    MP4V
};


enum acodec {
    MP3
};


void vlc_init();

int vlc_launch(enum vcodec, enum acodec, struct in_addr, __u16);
