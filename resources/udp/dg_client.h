/* 
 * File:   dg_client.h
 * Author: Jonathan Newman
 *
 * Created on September 1, 2014, 6:04 PM
 */

#ifndef DG_CLIENT_H
#define	DG_CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    // Constants
    #define PORT "4950"
    #define MAXBUFLEN 100
    
    // Prototypes
    void *getinaddr(void);
    int start(void);
    int bindsocket(void);
    int receive(void);
    

#ifdef	__cplusplus
}
#endif

#endif	/* DG_CLIENT_H */

