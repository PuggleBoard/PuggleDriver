#include "boneclamp.h"

bc_data bc_data_init()  {
    bc_data bc;

    //Memory mapping
    bc.resolution = RESOLUTION + USERSPACE_OFFSET;
    bc.sampling_freq = SAMPLING_FREQ + USERSPACE_OFFSET;
    bc.sampling_period = SAMPLING_PERIOD + USERSPACE_OFFSET;
    bc.number_of_channels = NUMBER_OF_CHANNELS + USERSPACE_OFFSET;
    bc.voltage_limit = VOLTAGE_LIMIT + USERSPACE_OFFSET;

    bc.channel_1 = CHANNEL_1 + USERSPACE_OFFSET;
    bc.channel_2 = CHANNEL_2 + USERSPACE_OFFSET;
    bc.channel_3 = CHANNEL_3 + USERSPACE_OFFSET;
    bc.channel_4 = CHANNEL_4 + USERSPACE_OFFSET;
    bc.channel_5 = CHANNEL_5 + USERSPACE_OFFSET;
    bc.channel_6 = CHANNEL_6 + USERSPACE_OFFSET;
    bc.channel_7 = CHANNEL_7 + USERSPACE_OFFSET;
    bc.channel_8 = CHANNEL_8 + USERSPACE_OFFSET;
    bc.channel_9 = CHANNEL_9 + USERSACE_OFFSET;
    bc.channel_10 = CHANNEL_10 + USERSPACE_OFFSET;
    bc.channel_11 = CHANNEL_11 + USERSPACE_OFFSET;
    bc.channel_12 = CHANNEL_12 + USERSPACE_OFFSET;
    bc.channel_13 = CHANNEL_13 + USERSPACE_OFFSET;
    bc.channel_14 = CHANNEL_14 + USERSPACE_OFFSET;
    bc.channel_15 = CHANNEL_15 + USERSPACE_OFFSET;
    bc.channel_16 = CHANNEL_16 + USERSPACE_OFFSET;
    

    //Default value assignment
    *bc.resolution = DEF_RESOLUTION;
    *bc.sampling_freq = DEF_SAMPLING_FREQ;
    *bc.sampling_period = DEF_SAMPLING_PERIOD;
    *bc.number_of_channels = DEF_NUMBER_OF_CHANNELS;
    *bc.voltage_limit = DEF_VOLTAGE_LIMIT;

    //Set all channels to 0
    *bc.channel_1 = 0;
    *bc.channel_2 = 0;
    *bc.channel_3 = 0;
    *bc.channel_4 = 0;
    *bc.channel_5 = 0;
    *bc.channel_6 = 0;
    *bc.channel_7 = 0;
    *bc.channel_8 = 0;
    *bc.channel_9 = 0;
    *bc.channel_10 = 0;
    *bc.channel_11 = 0;
    *bc.channel_12 = 0;
    *bc.channel_13 = 0;
    *bc.channel_14 = 0;
    *bc.channel_15 = 0;
    *bc.channel_16 = 0;

    return bc;
}

