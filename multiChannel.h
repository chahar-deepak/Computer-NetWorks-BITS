#ifndef _multi
#define _multi


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 8888 
#define SIZE 2
#define PDR 60
#define BUFSIZE 2
#define OUTTIME 1

typedef struct _packet {
    int size ;
    unsigned long long int seq_no ;
    int isData ;
    int channelID ;
    char data[SIZE] ;
} packet ;



#endif