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
        size_t sample_freq_hz;
        size_t frame_freq_hz;
        size_t bytes_per_sample;
        size_t bytes_per_channel_per_frame;
        size_t bytes_per_frame;
        size_t samples_per_channel_per_frame;
        size_t samples_per_frame;
        size_t channel_count;
        size_t digital_zero;
        size_t socket_buffer_bytes;
    } frame_config;
    
    /* Prototypes */
    int configure();
    int streamer_bindsocket(int *sfd, struct addrinfo *si);
    int streamer_send(int *sfd, char *servargs[], struct addrinfo *si);
    int streamer_sendconfig(int *sfd, char *servargs[], struct addrinfo *si);
    int streamer_sendframe(int *sfd, float data_frame[], int framesize, struct addrinfo *si);
    int steamer_getframe(int *fifo, float data_frame[], int count);
    int streamer_start(char *servargs[]);
    int streamer_close(int *sfd, struct addrinfo *si);
    
    int receiver_bindsocket(int *sfd, struct addrinfo *si);
    int receiver_receive(int *sfd, unsigned long *bytecount);
    int receiver_receiveframe(int *sfd, unsigned long *bytecount);
    int receiver_start(void);
    int receiver_close(void);
    
    
    /* Common methods */
    //void exit(void);

#ifdef	__cplusplus
}
#endif

#endif	/* DG_SERVER_H */

