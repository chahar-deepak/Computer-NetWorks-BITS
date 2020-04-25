#include "multiChannel.h"

void die ( char * s) {
    perror(s) ;
    exit(1) ;
}

int main () {


    struct sockaddr_in si_other1 , si_other2 ;

    si_other1.sin_family = AF_INET ;
    si_other1.sin_addr.s_addr = inet_addr("127.0.0.1") ;
    si_other1.sin_port = htons(PORT) ;

    si_other2.sin_family = AF_INET ;
    si_other2.sin_addr.s_addr = inet_addr("127.0.0.1") ;
    si_other2.sin_port = htons(PORT) ;

    // socket creation
    int s1 , s2 ;
    if ( (s1 = socket( AF_INET , SOCK_STREAM , 0 )) == -1 ) {
        die("socket_1") ;
    }
    if ( (s2 = socket ( AF_INET , SOCK_STREAM , 0 )) == -1 ) {
        close(s1) ;
        die("socket_2") ;
    }
    
    // attempting Connection
    if ( connect( s1 , (struct sockaddr*) &si_other1 , sizeof(si_other1) ) < 0){
        close (s1) ;
        close (s2) ;
        die("connect1") ;
    }
    if ( connect( s2 , (struct sockaddr*) &si_other2 , sizeof(si_other2) ) < 0){
        close(s1) ;
        close(s2) ;
        die("connect2") ;
    }
    printf("Connection DONE\n") ;

    //*************************************************************************************************************
    //*************************************************************************************************************
    
    
    
    int offset =  0 ;
    FILE *fp = fopen("SourceFile.txt" ,"rb") ;
    if (fp == NULL )
        die("file") ;
    fseek(fp , offset , SEEK_SET ) ;



    fd_set readfds ;
    int maxSD , activity , sentBytes ;
    maxSD = s1>s2?s1:s2 ;

    // transmission begins boizz 
    packet pk1, pk2 ,r1,r2 , repk ; 
    int nread ;
    unsigned long long int seq = 0 , ack1 = -1 , ack2 = -1 ;


    struct timeval timer ;
        timer.tv_sec = 0 ;
        timer.tv_usec = 0 ;


    // send 1 packet *****************************************
    pk1.channelID = 0 ;
    pk1.isData = 1 ;
    pk1.seq_no = seq ;
    
    nread = fread ( &(pk1.data) , 1 , SIZE , fp ) ;
        seq += nread ;
        pk1.data[nread] = '\0' ;
        pk1.size = nread ;
    ack1 = pk1.seq_no ;

    if( nread == -1 )
        die("fread error") ;

    if(pk1.size != SIZE ) {
        
        // send it and exit
        while ( 1 ) {
            printf("Sending seq: %llu via %d : |%s|\n",pk1.seq_no , pk1.channelID , pk1.data ) ;
            send ( s1 , &pk1 , sizeof(pk1) , 0 ) ;

            FD_ZERO(&readfds) ;
            FD_SET(s1 , &readfds ) ;

            timer.tv_sec = OUTTIME ;
            timer.tv_usec = 0 ;

            activity = select ( s1+1 , &readfds , NULL , NULL , &timer ) ;

            if (timer.tv_sec == 0 && timer.tv_usec == 0 ) {
                timer.tv_sec = OUTTIME ;
                timer.tv_usec = 0 ;
            }
            else {

                fclose ( fp ) ;
                close ( s1 ) ;
                sleep(2) ;
                return 0 ;
            }
            
        }
    }
    else {
        timer.tv_sec = OUTTIME ;
        timer.tv_usec = 0 ;

        printf("Sending seq: %llu via %d : |%s|\n",pk1.seq_no , pk1.channelID , pk1.data ) ;
        send ( s1 , &pk1 , sizeof(pk1) , 0 ) ;

    }

    // send 2 packet *****************************************
    pk2.channelID = 1 ;
    pk2.isData = 1 ;
    pk2.seq_no = seq ;

    nread = fread ( &(pk2.data) , 1 , SIZE , fp ) ;
        seq += nread ;
        pk2.data[nread] = '\0' ;
        pk2.size = nread ;


    if ( nread == -1 )
        die ( "fread error") ;
    
    ack2 = pk2.seq_no ;
    printf("Sending seq: %llu via %d : |%s|\n",pk2.seq_no , pk2.channelID , pk2.data ) ;
    send ( s2 , &pk2 , sizeof(pk2) , 0 ) ;


    if ( pk2.size != SIZE ) {
        // send these 2 and exit
        FD_ZERO(&readfds) ;
        FD_SET(s1 , &readfds) ;
        FD_SET(s2 , &readfds) ;

        maxSD = s1>s2 ? s1 : s2 ;
        activity = select ( maxSD+1 , &readfds , NULL , NULL , &timer ) ;

        if ( timer.tv_sec == 0 && timer.tv_usec == 0 ) {
            timer.tv_sec = OUTTIME ;
            timer.tv_usec = 0 ;

            repk = (pk1.seq_no < pk2.seq_no) ? pk1 : pk2 ;
            if( send( repk.channelID?s2:s1 , &repk , sizeof(repk) , 0 ) == -1 ) die("send repk error") ;
            

        }
        if ( FD_ISSET(s1 , &readfds) ) {

            if (recv(s1 , &r1 , sizeof(r1) , 0) == -1 ) die("recv") ;

            printf("r1.seq_no : %llu",r1.seq_no) ;
            if ( r1.seq_no == ack1 ){
                printf("resetting ACK\n") ;
                ack1 = -1 ;
            }

        }
        if( FD_ISSET(s2 , &readfds) ) {
            if (recv(s2 , &r2 , sizeof(r2) , 0) == -1) die("recv") ;

            printf("r2.seq_no : %llu",r2.seq_no) ;
            if (r2.seq_no == ack2 ){
                printf("resetting ACK\n") ;
                ack2 = -1 ;
            }
        }
        
        
        if(ack1 == -1 && ack2 == -1) {
            fclose(fp) ;
            close(s1) ;
            close(s2) ;
            sleep(2) ;
            return 0 ;
        }
        

    }
    else {
        ;
    }

    // more packets ****************************************************************************************************
    // more packets ****************************************************************************************************

    while ( 1 ) {
        FD_ZERO(&readfds) ;
        FD_SET(s1 , &readfds) ;
        FD_SET(s2 , &readfds) ;

        if ( (activity = select(maxSD+1 , &readfds , NULL , NULL , &timer )) < 0) perror ("select") ;
    
        if ( timer.tv_sec == 0 && timer.tv_usec == 0 ) {
            timer.tv_sec = OUTTIME ;
            timer.tv_usec = 0 ;

            repk = (pk1.seq_no < pk2.seq_no) ? pk1 : pk2 ;
            printf("Resending seq: %llu via %d : |%s|\n",repk.seq_no , repk.channelID , repk.data ) ;
            if( send( repk.channelID?s2:s1 , &repk , sizeof(repk) , 0 ) == -1 ) die("send repk error") ;
            
        }
        if ( FD_ISSET(s1 , &readfds) ) {
            
            if ( recv(s1 , &r1 , sizeof(r1) , 0 ) == -1 ) die("recv error") ;
            printf("ACK for %llu via %d\n" , r1.seq_no , r1.channelID ) ;


            if (r1.seq_no == pk1.seq_no) 
                ack1 = -1 ;
            
            
            nread = fread ( &(pk1.data) , 1 , SIZE , fp ) ;
                pk1.seq_no = seq ;
                seq += nread ;
                pk1.data[nread] = '\0' ;
                pk1.size = nread ;

            printf("Sending seq: %llu via %d : |%s|\n",pk1.seq_no , pk1.channelID , pk1.data ) ;
            if ( send(s1 , &pk1 , sizeof(pk1) , 0) == -1 ) die("send error") ;

            ack1 = pk1.seq_no ;

            if ( nread != SIZE ) {
                // endgame : need to get ACKS of both
                goto endgame;
                
            }
            else {
                // keep looping
                ;
            }
        }
        if ( FD_ISSET(s2 , &readfds) ) {
            if ( recv(s2 , &r2 , sizeof(r2) , 0 ) == -1 ) die("recv error") ;
            

            printf("ACK for %llu via %d\n" , r2.seq_no , r2.channelID ) ;
            if (r2.seq_no == pk2.seq_no)
                ack2 = -1 ;
            
            
            nread = fread ( &(pk2.data) , 1 , SIZE , fp ) ;
                pk2.seq_no = seq ;
                seq += nread ;
                pk2.data[nread] = '\0' ;
                pk2.size = nread ;

            
            printf("Sending seq: %llu via %d : |%s|\n",pk2.seq_no , pk2.channelID , pk2.data ) ;
            if ( send(s2 , &pk2 , sizeof(pk2) , 0) == -1 ) die("send error") ;

            ack2 = pk2.seq_no ;

            if ( nread != SIZE ) {
                // endgame : need to get ACKS of both
                goto endgame;
                
            }
            else {
                // keep looping
                ;
            }



        }
            // printf("Unhandled ELSE\n") ;
        
    }

    endgame :;
    printf("WE ARE IN ENDGAME now\n") ;

    while ( 1 ) {
        FD_ZERO(&readfds) ;
        FD_SET(s1 , &readfds) ;
        FD_SET(s2 , &readfds) ;

        if ( (activity = select(maxSD+1 , &readfds , NULL , NULL , &timer )) < 0) perror ("select") ;

        if( FD_ISSET(s1 , &readfds) ) {
            if ( recv(s1 , &r1 , sizeof(r1) , 0 ) == -1 ) die("recv error") ;
            printf("ACK for %llu via %d\n" , r1.seq_no , r1.channelID ) ;

            ack1 = -1 ;
            // printf("Got ACK1\n") ;
        }
        if( FD_ISSET(s2 , &readfds) ) {

            if ( recv(s2 , &r2 , sizeof(r2) , 0 ) == -1 ) die("recv error") ;
            printf("ACK for %llu via %d\n" , r2.seq_no , r2.channelID ) ;
            
            ack2 = -1 ;
            // printf("Got ACK2\n") ;
        }

        if( ack1==-1 && ack2==-1 ){
            break ;
        }

        if( timer.tv_sec == 0 && timer.tv_usec == 0 ) {
            timer.tv_sec = OUTTIME ;
            timer.tv_usec = 0 ;

            if( ack1 == -1 ) {
                repk = pk2 ;
            } 
            else if( ack2 == -1 ){
                repk = pk1 ;
            }
            else if ( ack1 != -1 && ack2 != -1 ) {
                repk = (pk1.seq_no < pk2.seq_no) ? pk1 : pk2 ;
            }
            else {
                printf("Unhandled ELSE\n") ;
            }

            printf("Resending seq: %llu via %d : |%s|\n",repk.seq_no , repk.channelID , repk.data ) ;
            if( send( repk.channelID?s2:s1 , &repk , sizeof(repk) , 0 ) == -1 ) die("send repk error") ;

        }
        

    }


    
    



    fclose(fp) ;
    close ( s1 ) ;
    close ( s2 ) ;
    printf("DONE Closing\n") ;
    sleep(2) ;
    return 0 ;
}