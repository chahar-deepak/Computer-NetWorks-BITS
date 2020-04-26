#include "packet.h"


void die (char *s) {
    perror(s) ;
    exit(1) ;
}

int main() {

    struct sockaddr_in si_relay1 , si_relay2 , si_me , si_other ;
    int meLen = sizeof(si_me) , rlen1 = sizeof(si_relay1) , rlen2 = sizeof(si_relay2) , olen = sizeof(si_other) ;

    packet pksend , pkrecv ;
    strcpy ( pksend.data , "|HULK is bulk|\0") ;

    int s = socket ( AF_INET , SOCK_DGRAM , 0 ) ;
    if ( s < 0 ) die("ERROR socket") ;


    si_me.sin_family = AF_INET ;
    si_me.sin_port = htons(CLIENT_PORT) ;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY) ;

    if ( bind(s , (struct sockaddr *)&si_me , meLen ) < 0 ) die ("ERROR binding socket") ;


    si_relay1.sin_family = AF_INET ;
    si_relay1.sin_addr.s_addr = inet_addr("127.0.0.1") ;
    si_relay1.sin_port = htons(RELAY_PORT) ;

    
    // printf("Sending : %llu & %s\n" , pksend.seq_no ,pksend.data ) ;
    // if ( sendto( s , &pksend , sizeof(pksend) , 0 , (struct sockaddr *) &si_relay1 , rlen1 ) == -1 ) die ("ERROR in sending") ;
    // if ( recvfrom ( s , &pkrecv , sizeof(pkrecv) , 0 , (struct sockaddr *)&si_other , &olen) < 0) die("ERROR in recieving") ;

    // printf ("ACK? : %s\n" , pkrecv.data ) ;


    for ( int i =0 ; i<5 ; i++ ) {
        pksend.seq_no = i ;

        printf("Sending : %llu & %s\n" , pksend.seq_no ,pksend.data ) ;
        if ( sendto( s , &pksend , sizeof(pksend) , 0 , (struct sockaddr *) &si_relay1 , rlen1 ) == -1 ) die ("ERROR in sending") ;

    }



    close ( s ) ;
    return 0 ;


}