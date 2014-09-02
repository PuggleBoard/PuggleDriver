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
#include "dg_client.h"

// Start listening to messages on PORT
int server_start(char *servargs[]) 
{
    
    int sockfd; // Socket file descriptor
    struct addrinfo hints, *servinfo;
    int rv;

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
    
    // Send message
    if ((rv = server_send(&sockfd, servargs, servinfo)) == -1)
    {
        fprintf(stderr, "sendto: failed to send message\n");
        exit(1);
    } 
    
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
    struct addrinfo *p = si;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len = sizeof(their_addr);
    int numbytes;
    char s[INET6_ADDRSTRLEN];           // Use INET6 length since this will work for IPv4 too.
    
    
    if ((numbytes = sendto(*sfd, servargs[1], strlen(servargs[1]), 0, p->ai_addr, p->ai_addrlen)) == -1) 
    {
	perror("server: sendto");
	return numbytes;
    }

    printf("server: sent %d bytes to %s\n", numbytes, servargs[1]);

    return 0;
}

// Serialize floating point value
int server_sendframe(int nchan, int nsamp, float **data_frame)
{

    if ((numbytes = sendto(*sfd, data_frame, nchan*nsamp*4, 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("server: sendto");
        return numbytes;
    }
}

