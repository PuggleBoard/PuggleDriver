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
    
    /* Prototypes */
    int start(void);
    int bindsocket(void);
    int send(void);
    int receive(void);
    
    /* Common methods */
    
    // Get sockaddr, IPv4 or IPv6
    void *get_in_addr(struct sockaddr *sa)
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

