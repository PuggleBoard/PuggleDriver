#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "maps.h"

//Default acquisition properties
//(MUST FIND DEF_SAMPLING PERIOD!!!!)
#define DEF_RESOLUTION          16
#define DEF_SAMPLING_FREQ       25000
#define DEF_SAMPLING_PERIOD     42      //DUMMY VALUE!!
#define DEF_NUMBER_OF_CHANNELS  16
#define DEF_VOLTAGE_LIMIT       5

typedef struct bc_data  {
    unsigned short *resolution;
    unsigned short *sampling_freq;
    unsigned short *voltage_limit;
    unsigned short *number_of_channels;
    unsigned int *sampling_period;

    unsigned int *channel_1;
    unsigned int *channel_2;
    unsigned int *channel_3;
    unsigned int *channel_4;
    unsigned int *channel_5;
    unsigned int *channel_6;
    unsigned int *channel_7;
    unsigned int *channel_8;
    unsigned int *channel_9;
    unsigned int *channel_10;
    unsigned int *channel_11;
    unsigned int *channel_12;
    unsigned int *channel_13;
    unsigned int *channel_14;
    unsigned int *channel_15;
    unsigned int *channel_16;   
}

void bc_data_init();

