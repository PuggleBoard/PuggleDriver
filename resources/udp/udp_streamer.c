/* 
*
* streamer.c -- a datagram 'server'
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include "udp_stream.h"
#include "fifo_defs.h"

// Start listening to messages on PORT
int streamer_start(char *servargs[]) 
{
    
    int sockfd;                         // Socket file descriptor
    int fifo;                           // FIFO file descritor for getting data from BB's RAM
    struct addrinfo hints, *servinfo;   
    int rv;
    float frame[NCHAN*BUFSAMP] = {};
    int framesize = sizeof(frame);
    unsigned long framecount = 0; 
    
    // Timing stuff
    time_t now;
    time_t then;
    time(&now);
    struct tm beg = *localtime(&now);
    double dtsec;

    // Clear hints and fill out relevant fields
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;            // Unspecified
    hints.ai_socktype = SOCK_DGRAM;         // UDP

    // Get address info
    if ((rv = getaddrinfo(servargs[0], PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Bind to to socket
    if ((rv = streamer_bindsocket(&sockfd, servinfo)) != 0)
    {
        fprintf(stderr, "bindsocket: failed to bind socket\n");
        return rv;
    } 
    
    printf("streamer: sockfd = %d\n", sockfd);
    
    // Open FIFO
    streamer_openfifo(&fifo);
    
    // Send message
    while(1)
    {
        // Pull data fram RAWFIFO
        steamer_getframe(frame, NCHAN*BUFSAMP);
         
         
        if ((rv = streamer_sendframe(&sockfd, frame, framesize, servinfo)) == -1)
        {
            fprintf(stderr, "sendto: failed to send frame\n");
            exit(1);
        } 
        
        dtsec = difftime(time(&then), now);
        framecount++;
        if ((framecount % 100000) == 0)
        {
            printf("streamer: sent %lu frames in %f seconds\n", framecount,dtsec);
        }
    }
    
   
    
    return 0;

}

// Bind socket
int streamer_bindsocket(int *sfd, struct addrinfo *si)
{
    // Bind call return status
    int stat;
    struct addrinfo *p;

    // Loop through all possible sockets given our addinfo and bind 
    // the first one we can
    for (p = si; p != NULL; p->ai_next)
    {
        if ((*sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("streamer: socket");
            continue;
        }

        break;
    }
    
    if (p == NULL) 
    {
        fprintf(stderr, "streamer: failed to bind socket\n");
        return 2;
    }

    // Success
    printf("streamer: bound to port %s\n",PORT);
    return stat;
}

// Send message
int streamer_send(int *sfd, char *servargs[], struct addrinfo *si)
{
    
    struct sockaddr_storage their_addr;
    int numbytes;
    char s[INET6_ADDRSTRLEN];           // Use INET6 length since this will work for IPv4 too.
    
    
    if ((numbytes = sendto(*sfd, servargs[1], strlen(servargs[1]), 0, si->ai_addr, si->ai_addrlen)) == -1) 
    {
	perror("streamer: sendto");
	return numbytes;
    }

    printf("streamer: sent %d bytes to %s\n", numbytes, servargs[1]);

    return 0;
}

// Serialize floating point value
int streamer_sendframe(int *sfd, float *data_frame, int framesize, struct addrinfo *si)
{
    int numbytes;
    
    if ((numbytes = sendto(*sfd, data_frame, framesize, 0, si->ai_addr, si->ai_addrlen)) == -1)
    {
        perror("streamer: sendto");
        return numbytes;
    }
}

int streamer_openfifo(int *fifo) {
    
    //  Create FIFO over which data will be received
    if( mkfifo( RAWFIFO, 0666) < 0 ) {
        if (errno != EEXIST) {
            perror( "streamer: mkfifo");
            exit( EXIT_FAILURE);
        }
    }
    
    // Open the FIFO
    fifo = open( "PUGGLE-STREAM", O_RDWR );
    if( fifo < 0 ) {
        perror( "streamer: open FIFO" );
        quit( EXIT_FAILURE );
    }
    
    // Success
    printf("streamer: RAWFIFO open, listening for incoming data.");
    return 0;
    
}

int steamer_getframe(int *fifo, float *data_frame, float count)
{
//    // For now we are just going to create a data frame
//    count--;
//    while(count >= 0)
//    {
//        data_frame[count] = (float)(count);
//        count--;
//    }
    
    int numbytes;
    
    // Pull data from the fifo buffer
    if (numbytes = read(fifo, &data_frame, count) < 0) {
        perror("streamer: fifo read");
	return numbytes;
    };
    
    
    return 0;
    
}

int streamer_close(int *sfd, struct addrinfo *si)
{
     // Free up the servinfo struct and close the socket
    freeaddrinfo(si);
    close(*sfd);
   
}

//void configure(frame_config *fc)
//{
//    fc.
//    
//}

