#include "packet.h"


void die (char *s) {
    perror(s) ;
    exit(1) ;
}

int main() {

    struct sockaddr_in si_me , si_other ;
    int slen = sizeof(si_other) ;

    packet pkrecv ;
    packet pkack ;
    
    strcpy( pkack.data , "isACK\0") ;

    int s = socket( AF_INET , SOCK_DGRAM , 0 ) ;
    if ( s < 0 ) die("ERROR socket creation") ;

    si_me.sin_family = AF_INET ;
    si_me.sin_port = htons(SERVER_PORT) ;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY) ;

    if ( bind(s , (struct sockaddr *) &si_me  , slen) < 0 ) die("ERROR binding") ;


    printf("Standby to Recieve Data\n") ;
    while ( 1 ) {
        
        if ( recvfrom(s , &pkrecv , sizeof(pkrecv) , 0 , (struct sockaddr *) &si_other  , &slen ) < 0 ) die ("ERROR in listening\n") ;

        printf("> Recieved : %llu & |%s| from %s:%d\n" , pkrecv.seq_no ,pkrecv.data  , inet_ntoa(si_other.sin_addr) , ntohs(si_other.sin_port) ) ;

        if ( sendto(s ,&pkack , sizeof(pkack) , 0 , (struct sockaddr *) &si_other , slen ) < 0 ) die("ERROR sending falied\n") ;
    }

    close ( s ) ;
    return 0 ;

}