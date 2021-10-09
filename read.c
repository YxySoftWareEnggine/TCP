#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcli.h"

int recvdata(char* buf,int size){
    printf("--- size=%d,buf=%s\n",size,buf);
}

int main(int argc,char **argv){
    int err= clidev.start(argv[1],atoi(argv[2]),recvdata);
    if (err==-1) return -1;
    char buf[256];
    while(1){
       scanf("%s",buf);
       clidev.send(buf,strlen(buf));
       if (strcmp(buf,"q")==0) break;
    }
    clidev.stop();
}