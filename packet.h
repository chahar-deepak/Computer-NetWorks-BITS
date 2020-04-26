#ifndef _PACKET
#define _PACKET

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>


#define PACKET_SIZE 100
#define RELAY_PORT 5555
#define CLIENT_PORT 5556
#define SERVER_PORT 5557
#define DROP_RATE 30
#define N 4

typedef struct _pac {
    long long int seq_no ;
    int size ;
    int isLast ;
    char data[PACKET_SIZE+1] ;
} packet ;


#endif