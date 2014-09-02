/* 
*
* dg_server.c -- a datagram 'server'
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "dg_stream.h"

// Start listening to messages on PORT
int server_start(char *servargs[]) 
{
    
    int sockfd; // Socket file descriptor
    struct addrinfo hints, *servinfo;
    int rv;
    float frame[NCHAN*BUFSAMP] = {};
    int framesize = sizeof(frame);

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
    if ((rv = server_bindsocket(&sockfd, servinfo)) != 0)
    {
        fprintf(stderr, "bindsocket: failed to bind socket\n");
        return rv;
    } 
    
    printf("server: sockfd = %d\n", sockfd);
    
    // Get data frame
    server_getframe(frame, NCHAN*BUFSAMP);
    
    // Send message
    if ((rv = server_sendframe(&sockfd, frame, framesize, servinfo)) == -1)
    {
        fprintf(stderr, "sendto: failed to send message\n");
        exit(1);
    } 
    printf("server: sent %d frames\n", rv);
    
    // Free up the servinfo struct now that we are bound
    freeaddrinfo(servinfo);
    close(sockfd);
    
    return 0;

}

// Bind with error checking
int server_bindsocket(int *sfd, struct addrinfo *si)
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
            perror("server: socket");
            continue;
        }

        break;
        
    }
    
    if (p == NULL) 
    {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }

    // Success
    printf("server: bound to port %s\n",PORT);
    return stat;
}

// Send message
int server_send(int *sfd, char *servargs[], struct addrinfo *si)
{
    
    struct sockaddr_storage their_addr;
    //char buf[MAXBUFLEN];
    //socklen_t addr_len = sizeof(their_addr);
    int numbytes;
    char s[INET6_ADDRSTRLEN];           // Use INET6 length since this will work for IPv4 too.
    
    
    if ((numbytes = sendto(*sfd, servargs[1], strlen(servargs[1]), 0, si->ai_addr, si->ai_addrlen)) == -1) 
    {
	perror("server: sendto");
	return numbytes;
    }

    printf("server: sent %d bytes to %s\n", numbytes, servargs[1]);

    return 0;
}

// Serialize floating point value
int server_sendframe(int *sfd, float *data_frame, int framesize, struct addrinfo *si)
{
    int numbytes;
    
    if ((numbytes = sendto(*sfd, data_frame, framesize, 0, si->ai_addr, si->ai_addrlen)) == -1)
    {
        perror("server: sendto");
        return numbytes;
    }
}

int server_getframe(float *data_frame, int count)
{
    // For now we are just going to create a data frame
    count--;
    while(count >= 0)
    {
        data_frame[count] = (float)(count);
        count--;
    }
    
    return 0;
    
}

//void server_configure(frame_config *fc)
//{
//    fc.
//    
//}

