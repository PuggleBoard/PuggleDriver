/* 
 * File:   main.c
 * Author: Jonathan Newman
 *
 * Created on September 1, 2014, 6:21 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
//#include <dg_stream.h>

/*
 * 
 */
int main(int argc, char* argv[]) {

    bool isCaseInsensitive = false;
    int opt;
    enum { SERVER_MODE, CLIENT_MODE} mode = SERVER_MODE;
    char *optargs[2] = {" "," "};

    // Non optional arguments
    while ((opt = getopt(argc, argv, "sc")) != -1) 
    {
        switch (opt) 
        {
        case 's': 
            mode = SERVER_MODE; 
            break;
        case 'c': 
            mode = CLIENT_MODE; 
            break;
        default:
            fprintf(stderr, "Usage: %s [-sc] [hostname message]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    // Optional arguments
    if (optind < argc) 
    {
        int optind0 = optind;
        while (optind < argc)
        {
            optargs[optind-optind0] = argv[optind];
            printf("option %d is: %s\n", optind, optargs[optind-optind0]);
            optind++;
        }
    }
    
    
    // Now optind (declared extern int by <unistd.h>) is the index of the first 
    // non-option argument. If it is >= argc, there were no non-option arguments.
    
    switch (mode)
    {
        case SERVER_MODE:
        {
            if (argc != 4) 
            {
		fprintf(stderr,"usage: dg_stream -s hostname message\n");
		exit(1);
            }
        
        server_start(optargs);
        break;       
        }
        case CLIENT_MODE:
        { 
            client_start();
            break;       
        }
        default:
            fprintf(stderr, "Usage: %s [-sc] [hostname message]\n", argv[0]);
            exit(EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}

