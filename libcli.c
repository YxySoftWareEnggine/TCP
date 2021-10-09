#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libcli.h"
#define RECVSIZE 4096
int (*cli_recv)(char *buf,int size);
int cli_fd;
pthread_t recv_pid;
int cli_state;
void* recv_pthread(void* arg){
   char *buf=(char*)malloc(RECVSIZE);
   fd_set rfds;
   struct timeval t_out;
   while(cli_state){
       FD_ZERO(&rfds);
       FD_SET(cli_fd,&rfds);
       t_out.tv_sec=0;
       t_out.tv_usec=200000;
       int err=select(cli_fd+1,&rfds,NULL,NULL,&t_out);
       if (err==-1) break;  //设备出现问题
       else if (err==0) continue; //超时
       //接收数据
       int size=recv(cli_fd,buf,RECVSIZE,0);
       if (size<=0) break; //网络断开
       //将数据发送到UI
       cli_recv(buf,size);
    }
    //释放内存
    free(buf);
    //关闭客户端描述符
    close(cli_fd);
}
int cli_start(char* sevip,int sevport,int (*handler)(char *buf,int size)){
    if (cli_state==1) return -1;
    cli_recv=handler;
    //创建套接字
    cli_fd=socket(AF_INET,SOCK_STREAM,0);
    if (cli_fd==-1) goto SOCKERR;
    //绑定
    struct sockaddr_in cliaddr;
    cliaddr.sin_family=AF_INET;
    cliaddr.sin_port=0; //自动分配端口
    cliaddr.sin_addr.s_addr=INADDR_ANY; //绑定所有ip
    int err=bind(cli_fd,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
    if (err==-1) goto BINDERR;
    //连接服务器
    struct sockaddr_in sevaddr;
    sevaddr.sin_family=AF_INET;
    sevaddr.sin_port=htons(sevport);
    sevaddr.sin_addr.s_addr=inet_addr(sevip);
    err=connect(cli_fd,(struct sockaddr*)&sevaddr,sizeof(sevaddr));
    if (err==-1) goto BINDERR;
    //启动等待连接线程
    cli_state=1;
    pthread_create(&recv_pid,NULL,recv_pthread,NULL);
    return 0;
BINDERR:
    close(cli_fd);
SOCKERR:
    printf("----- err=%s\n",strerror(errno));
    return -1;
}

int cli_stop(){
    if (cli_state==0) return -1;
    //关闭所有的客户端线程
    cli_state=0;
    pthread_join(recv_pid,NULL);
    return 0;
}

int cli_send(char *buf,int size){
    if (cli_state==0) return -1;
    return send(cli_fd,buf,size,0);
}

cli_t clidev={
   cli_start,
   cli_stop,
   cli_send
};