#ifndef LIBSEV_H
#define LIBSEV_H
typedef struct SEVDEV{
   int (*start)(int sevport,int (*handler)(int fd,char *buf,int size));
   int (*stop)();
   int (*send)(int fd,char* buf,int size);
} sev_t;
extern sev_t sevdev;
#endif