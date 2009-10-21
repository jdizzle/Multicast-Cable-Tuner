#include <netinet/in.h>
#include <linux/types.h>


enum vcodec {
    MP4V,
    MP2V
};


enum acodec {
    MP3
};



int vlc_kill();
int vlc_launch(enum vcodec, enum acodec, struct in_addr, __u16);
int vlc_init(char*, char* , char*);
