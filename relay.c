#include "packet.h"

void addDelay () {
    int delay = rand()%2001 ;
    printf("> Delay of %d micro-seconds\n",delay) ;
    usleep(delay) ;
}

void die (char *s) {
    perror(s) ;
    exit(1) ;
}

int main (int argc , char  *argv[]) {
    if (argc != 2) {
        printf("Incorrect Number of Arguments : ./relay.c {0,1}\n") ;
        return 0 ;
    }

    struct sockaddr_in si_me, si_client , si_server , si_current ;
    int serverLen = sizeof(si_server) , clientLen = sizeof(si_client) , currentLen = sizeof(si_current) ;

    packet rpk ;

    int s = socket ( AF_INET , SOCK_DGRAM , 0 ) ;
        if( s == -1 ) die("ERROR socket") ;
    
    

    si_me.sin_family = AF_INET ;
    si_me.sin_port = htons(RELAY_PORT) ;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY) ;

    si_server.sin_family = AF_INET ;
    si_server.sin_port = htons(SERVER_PORT) ;
    si_server.sin_addr.s_addr = inet_addr("127.0.0.1") ;

    si_client.sin_family = AF_INET ;
    si_client.sin_port = htons(CLIENT_PORT) ;
    si_client.sin_addr.s_addr = inet_addr("127.0.0.1") ;


    if ( bind( s, (struct sockaddr *) &si_me , sizeof(si_me)) == -1 ) die("ERROR bind") ;


    printf("Relay is ready\n") ;
    while( 1 ) {
        
        if( recvfrom ( s , &rpk , sizeof(rpk) , 0 , (struct sockaddr *) &si_current , &currentLen ) == -1 ) die("ERROR recvfrom") ;

        printf("Recieved packet from %s:%d\n" , inet_ntoa(si_current.sin_addr) , ntohs(si_current.sin_port) ) ;

        pid_t pid ;
        
        if ( (pid = fork()) < 0 ) die("Fork failed") ;

        else if ( pid == 0 ) {
            // child process
            srand (time(0) ^ (getpid()<<16) ) ;


            // printf("I am Child\n") ;
            if ( ntohs(si_current.sin_port) == SERVER_PORT ) { // no delay just relay
                printf("Server->Client\n") ;
                if( sendto( s , &rpk , sizeof(rpk) , 0 , (struct sockaddr *) &si_client , clientLen ) == -1 ) die("ERROR Sending to client") ;
            }
            else if ( ntohs(si_current.sin_port) == CLIENT_PORT ){ // client 

                if ( rand()%100 < DROP_RATE ) {
                    printf("----DROP : %llu\n" , rpk.seq_no) ;
                }
                else {
                    printf("Client->Server\n") ;
                    addDelay() ;
                    if( sendto( s , &rpk , sizeof(rpk) , 0 , (struct sockaddr *) &si_server , serverLen ) == -1 ) die("ERROR Sending to server") ;

                }

            }

            return 0 ;
        }

        else {
            // parent process 
            // printf("I am parent Process\n") ;
            ;
        }



        

    }

    close(s) ;
    sleep ( 2 ) ;
    return 0 ;
}
