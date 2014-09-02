/* 
 * File:   dg_server.h
 * Author: jon
 *
 * Created on September 1, 2014, 6:17 PM
 */

#include <sys/socket.h>

#ifndef DG_STREAM_H
#define	DG_STREAM_H

#ifdef	__cplusplus
extern "C" {
#endif

    /* Constants */
    #define PORT "4950"
    #define MAXBUFLEN 100

    /* Type prototypes*/
    struct addrinfo;
    
    /* Prototypes */
    int server_bindsocket(int *sfd, struct addrinfo *si);
    int server_send(int *sfd, char *servargs[], struct addrinfo *si);
    int server_sendconfig(int *sfd, char *servargs[], struct addrinfo *si);
    //int server_serializeframe(float **data_frame);
    int server_sendframe(int *sfd, float **data_frame, struct addrinfo *si);
    int server_start(char *servargs[]);
    int server_close(void);
    
    int client_bindsocket(int *sfd, struct addrinfo *si);
    int client_receive(int *sfd);
    int client_start(void);
    int client_close(void);
    
    
    /* Common methods */
    
    // Get sockaddr, IPv4 or IPv6
    void *getinaddr(struct sockaddr *sa)
    {

        // See if this is IPv4
        if (sa->sa_family == AF_INET) 
        {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        // Else get the IPv6 address
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }



#ifdef	__cplusplus
}
#endif

#endif	/* DG_SERVER_H */

