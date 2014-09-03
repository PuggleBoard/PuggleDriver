/* 
 * File:   dg_server.h
 * Author: jon
 *
 * Created on September 1, 2014, 6:17 PM
 */



#ifndef DG_STREAM_H
#define	DG_STREAM_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <sys/socket.h>
    
    /* Constants */
    #define PORT "4950"
    #define MAXBUFLEN 100000
    #define NCHAN 32
    #define BUFSAMP 100
    
    /* Type prototypes*/
    struct addrinfo;
    
    typedef struct {
        int sample_freq_hz;
        int frame_freq_hz;
        int bytes_per_sample;
        int bytes_per_channel_per_frame;
        int bytes_per_frame;
        int samples_per_channel_per_frame;
        int samples_per_frame;
        int channel_count;
        int digital_zero;
        int socket_buffer_bytes;
    } frame_config;
    
    /* Prototypes */
    int configure();
    int streamer_bindsocket(int *sfd, struct addrinfo *si);
    int streamer_send(int *sfd, char *servargs[], struct addrinfo *si);
    int streamer_sendconfig(int *sfd, char *servargs[], struct addrinfo *si);
    int streamer_sendframe(int *sfd, float data_frame[], int framesize, struct addrinfo *si);
    int steamer_getframe(float data_frame[], int count);
    int streamer_start(char *servargs[]);
    int streamer_close(int *sfd, struct addrinfo *si);
    
    int receiver_bindsocket(int *sfd, struct addrinfo *si);
    int receiver_receive(int *sfd);
    int receiver_receiveframe(int *sfd);
    int receiver_start(void);
    int receiver_close(void);
    
    
    /* Common methods */

#ifdef	__cplusplus
}
#endif

#endif	/* DG_SERVER_H */

