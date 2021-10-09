#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libsev.h"
#define RECVSIZE 4096
int (*sev_recv)(int fd,char *buf,int size);
int sev_fd;
pthread_t accept_pid;
int sev_state;
void* recv_pthread(void* arg){
   int fd=*(int*)arg;
   printf("----recv start fd=%d  pid=%d\n",fd,pthread_self());
   char *buf=(char*)malloc(RECVSIZE);
   fd_set rfds;
   struct timeval t_out;
   while(sev_state){
       FD_ZERO(&rfds);
       FD_SET(fd,&rfds);
       t_out.tv_sec=0;
       t_out.tv_usec=200000;
       int err=select(fd+1,&rfds,NULL,NULL,&t_out);
       if (err==-1) break;  //设备出现问题
       else if (err==0) continue; //超时
       //接收数据
       int size=recv(fd,buf,RECVSIZE,0);
       if (size<=0) break; //网络断开
       //将数据发送到UI
       sev_recv(fd,buf,size);
    }
    //释放内存
    free(buf);
    //关闭客户端描述符
    close(fd);
    printf("----recv stop fd=%d\n",fd);
}
void* accept_pthread(void*arg){
    while(1){
       struct sockaddr_in cliaddr;
       int clilen;
       int cli_fd=accept(sev_fd,(struct sockaddr*)&cliaddr,&clilen);
       if (cli_fd==-1) continue;
       //-----------------------
       pthread_t pid;
       pthread_create(&pid,NULL,recv_pthread,&cli_fd);
    }
}
int sev_start(int sevport,int (*handler)(int fd,char *buf,int size)){
    if (sev_state==1) return -1;
    sev_recv=handler;
    //创建套接字
    sev_fd=socket(AF_INET,SOCK_STREAM,0);
    if (sev_fd==-1) goto SOCKERR;
    //绑定
    struct sockaddr_in sevaddr;
    sevaddr.sin_family=AF_INET;
    sevaddr.sin_port=htons(sevport);
    sevaddr.sin_addr.s_addr=INADDR_ANY;
    int err=bind(sev_fd,(struct sockaddr*)&sevaddr,sizeof(sevaddr));
    if (err==-1) goto BINDERR;
    //启动侦听
    err=listen(sev_fd,SOMAXCONN);
    if (err==-1) goto BINDERR;
    //启动等待连接线程
    sev_state=1;
    pthread_create(&accept_pid,NULL,accept_pthread,NULL);
    return 0;
BINDERR:
    close(sev_fd);
SOCKERR:
    printf("----- err=%s\n",strerror(errno));
    return -1;
}
int sev_stop(){
    if (sev_state==0) return -1;
    //关闭所有的客户端线程
    sev_state=0;
    usleep(200000);
    //关闭等待连接线程
    pthread_cancel(accept_pid);
    pthread_join(accept_pid,NULL);
    close(sev_fd);
    return 0;
}

int sev_send(int fd,char *buf,int size){
    if (sev_state==0) return -1;
    return send(fd,buf,size,0);
}

sev_t sevdev={
   sev_start,
   sev_stop,
   sev_send
};