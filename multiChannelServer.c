#include "multiChannel.h"

void die ( char * s) {
    perror(s) ;
    exit(1) ;
}


int main () {

    struct sockaddr_in si_other1 ,si_other2 , si_me ;
    
    // my address info
    si_me.sin_family = AF_INET ;
    si_me.sin_addr.s_addr = inet_addr("127.0.0.1") ;
    si_me.sin_port = htons(PORT) ;

    //initializing socket
    int s = socket(AF_INET , SOCK_STREAM , 0 ) ;
    if ( s == -1 )
        die("socket") ;

    printf( "Listening socket ID : %d\n" , s) ;

    // setting up reuse
    int reuse = 1;
    if ( setsockopt( s , SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        close(s) ;
        die("setsockopt(SO_REUSEADDR) failed") ;
    }

    // binding socket to structure
    if ( bind( s , (struct sockaddr *) &si_me , sizeof(si_me) ) < 0 )
        die ("bind") ;
    if ( listen( s , 2) < 0 )
        die("listen") ;
    
    printf("listening...\n") ;

    // accepting both connections
    int s1 , s1_len = sizeof(si_other1) ;
    if (  (s1 = accept( s , (struct sockaddr *) &si_other1 , &s1_len)) == -1 ){
        close(s) ;
        die("accept1") ;
    }
    int s2 , s2_len = sizeof(si_other2) ;
    if (  (s2 = accept( s , (struct sockaddr *) &si_other2 , &s2_len)) == -1 ){
        close(s) ;
        close(s1) ;
        die("accept2") ;
    }

    printf("BOTH Connections established\n") ;

    // ******************************************************************************************************************
    // ******************************************************************************************************************


    // file handling
    FILE * fp = fopen("DestinationFile.txt","w") ;

    fd_set readfds ;
    int maxSD , activity , rcvdBytes ;
    maxSD = s ;
    if(maxSD < s1 )
        maxSD = s1 ;
    if(maxSD < s2 )
        maxSD = s2 ;

    // ****************************************************************
    packet buffer[BUFSIZE] ;
    for ( int i=0 ; i<BUFSIZE ; i++ ) {
        buffer[i].seq_no = -1 ;
    }
    int bufferCount = 0 ;
    // ****************************************************************

    packet forACK1 , forACK2 ;
        forACK1.isData = 0 ;
        forACK2.isData = 0 ;
    packet pk1 , pk2 ;
    long long int waiting_for = 0 ;

    printf("Initializing Data Import\n\n\n") ;


    while ( 1 ) {

        FD_ZERO(&readfds) ;
        FD_SET(s1 , &readfds) ;
        FD_SET(s2 , &readfds) ;

        
        if ( (activity = select(maxSD+1 , &readfds , NULL , NULL , NULL ) < 0) )    perror("Error in select") ;
        
        
        if ( FD_ISSET(s1 , &readfds) ) {


            // memset(&pk1 , '\0' , sizeof(pk1) ) ;
            rcvdBytes = recv(s1 , &pk1 , sizeof(pk1) , 0 ) ;
            if(rcvdBytes == -1) {
                printf("Error in recv\n") ;
                break ;
            }


            if ( rand()%100  < PDR ) {
                printf("> Dropping Packet ( %d , %d , %llu , %d ) : |%s|\n" , pk1.channelID , pk1.isData , pk1.seq_no , pk1.size , pk1.data ) ;
            }
            else {

                printf("> ( %d , %d , %llu , %d ) : |%s|\n" , pk1.channelID , pk1.isData , pk1.seq_no , pk1.size , pk1.data ) ;


                if( waiting_for == pk1.seq_no ){

                    // send ACK back
                    forACK1.channelID = pk1.channelID ;
                    forACK1.seq_no = pk1.seq_no ;

                    if ( send(s1 , &forACK1 , sizeof(forACK1) , 0 ) == -1 )   die("ACK send Failed\n") ;


                    printf("Outputting to File seq_no : %llu\n",pk1.seq_no) ;
                    fprintf(fp,"%s",pk1.data ) ;
                    waiting_for += pk1.size ;
                    
                    
                    // check buffer for further data and place that in file
                    int reduceBy = 0 ;
                    for ( int i = 0 ; i<bufferCount ; i++ ) {
                        printf("--Current Waiting_for : %llu\n",waiting_for) ;
                        for( int j =0 ; j<BUFSIZE ; j++ ) {

                            if(buffer[j].seq_no == waiting_for) {
                                printf("Outputting to File seq_no : %llu\n",buffer[j].seq_no) ;
                                waiting_for += buffer[j].size ;
                                fprintf(fp,"%s",buffer[j].data ) ;

                                
                                buffer[j].seq_no = -1 ;
                                reduceBy++ ;
                                break ;
                            }
                        }
                    }
                    bufferCount -= reduceBy ;
                    printf("BufferCount on packet arrival : %d\n" , bufferCount) ;
                }
                else{
                    // buffer is full then donot send ack 
                    
                    
                    if(bufferCount == BUFSIZE) {
                        printf("Buffer Full seq_no : %llu\n", pk1.seq_no);
                    }
                    // store && send ack with seq_no
                    else {

                        // send ACK back
                        forACK1.channelID = pk1.channelID ;
                        forACK1.seq_no = pk1.seq_no ;

                        if ( send(s1 , &forACK1 , sizeof(forACK1) , 0 ) == -1 )   die("ACK send Failed\n") ;


                        printf("\t--------adding %llu to buffer\n",pk1.seq_no) ;
                        int ii ;
                        for ( ii= 0 ; ii<BUFSIZE ; ii++ ) {
                            if( buffer[ii].seq_no == -1 ) {
                                buffer[ii] = pk1 ;
                                bufferCount++ ;
                                break ;
                            }
                        }
                        

                        
                    }

                    printf("----BUFFER STATUS : ") ;
                    for ( int i=0 ; i<BUFSIZE ; i++ )
                        if(buffer[i].seq_no != -1)
                            printf("%llu , ",buffer[i].seq_no) ;
                    printf("|\n") ;
                    
                }

                // printf("Handled seq_no :%llu\n",pk1.seq_no) ;

                if( pk1.size != SIZE && (waiting_for == (pk1.seq_no + pk1.size)) ) {
                    break ;
                }

            }
            
        }
        if ( FD_ISSET(s2 , &readfds) ) {


            // memset(&pk2 , '\0' , sizeof(pk2) ) ;
            rcvdBytes = recv(s2 , &pk2 , sizeof(pk2) , 0 ) ;
            if(rcvdBytes == -1) {
                printf("Error in recv\n") ;
                break ;
            }


            if ( rand()%100 < PDR ) {
                printf("> Dropping Packet ( %d , %d , %llu , %d ) : |%s|\n" , pk2.channelID , pk2.isData , pk2.seq_no , pk2.size , pk2.data ) ;
            }
            else {

                printf("> ( %d , %d , %llu , %d ) : |%s|\n" , pk2.channelID , pk2.isData , pk2.seq_no , pk2.size , pk2.data ) ;



                if( waiting_for == pk2.seq_no ){

                    // send ACK back
                    forACK2.channelID = pk2.channelID ;
                    forACK2.seq_no = pk2.seq_no ;

                    if ( send(s2 , &forACK2 , sizeof(forACK2) , 0 ) == -1 )   die("ACK send Failed\n") ;




                    printf("Outputting to File seq_no : %llu\n",pk2.seq_no) ;
                    fprintf(fp,"%s",pk2.data ) ;
                    waiting_for += pk2.size ;
                    
                    
                    
                    
                    // check buffer for further data and place that in file
                    int reduceBy = 0 ;
                    for ( int i = 0 ; i<bufferCount ; i++ ) {
                        printf("--Current Waiting_for : %llu\n",waiting_for) ;
                        for( int j =0 ; j<BUFSIZE ; j++ ) {
                            if(buffer[j].seq_no == waiting_for) {
                                printf("Outputting to File seq_no : %llu\n",buffer[j].seq_no) ;
                                waiting_for += buffer[j].size ;
                                fprintf(fp,"%s",buffer[j].data ) ;



                                buffer[j].seq_no = -1 ;
                                reduceBy++ ;
                                break ;
                            }
                        }
                    }
                    bufferCount -= reduceBy ;
                    printf("BufferCount on packet arrival : %d\n" , bufferCount) ;
                }
                else{
                    // buffer is full then donot send ack 
                    if(bufferCount == BUFSIZE) {
                        printf("Buffer Full seq_no : %llu\n", pk1.seq_no);;
                    }
                    // store && send ack with seq_no
                    else {

                        // send ACK back
                        forACK2.channelID = pk2.channelID ;
                        forACK2.seq_no = pk2.seq_no ;

                        if ( send(s2 , &forACK2 , sizeof(forACK2) , 0 ) == -1 )   die("ACK send Failed\n") ;


                        printf("\t--------adding %llu to buffer\n",pk2.seq_no) ;
                        int ii ;
                        for ( ii= 0 ; ii<BUFSIZE ; ii++ ) {
                            if( buffer[ii].seq_no == -1 ) {
                                buffer[ii] = pk2 ;
                                bufferCount++ ;
                                break ;
                            }
                        }
                        printf("--------now Buffer Count : %d\n",bufferCount) ;
                    }

                    printf("----BUFFER STATUS : ") ;
                    for ( int i=0 ; i<BUFSIZE ; i++ )
                        if(buffer[i].seq_no != -1)
                            printf("%llu , ",buffer[i].seq_no) ;
                    printf("|\n") ;


                }


                if( pk2.size != SIZE && (waiting_for == (pk2.seq_no + pk2.size)) ) {
                    break ;
                }


            }

        }

    }


    for ( int i = 0 ; i<bufferCount ; i++ ) {
        for( int j =0 ; j<BUFSIZE ; j++ ) {
            if(buffer[j].seq_no == waiting_for) {
                printf("Outputting to File seq_no : %llu\n",buffer[j].seq_no) ;
                waiting_for += buffer[j].size ;
                fprintf(fp,"%s",buffer[j].data ) ;



                buffer[j].seq_no = -1 ;
                break ;
            }
        }
    }

    
    fclose(fp) ;
    close (s1) ;
    close (s2) ;
    close (s) ;
    sleep(2) ;
    return 0 ;
}