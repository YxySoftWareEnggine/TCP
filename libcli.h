#ifndef LIBCLI_H
#define LIBCLI_H
typedef struct CLIDEV{
   int (*start)(char* sevip,int sevport,int (*handler)(char *buf,int size));
   int (*stop)();
   int (*send)(char* buf,int size);
} cli_t;
extern cli_t clidev;
#endif