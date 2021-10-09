#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libsev.h"

int recvdata(int fd,char* buf,int size){
    printf("--- size=%d,buf=%s\n",size,buf);
    sevdev.send(fd,"ok",3);
}

int main(int argc,char **argv){
    int err= sevdev.start(atoi(argv[1]),recvdata);
    if (err==-1) return -1;
    char buf[256];
    while(1){
       scanf("%s",buf);
       if (strcmp(buf,"q")==0) break;
    }
    sevdev.stop();
}