/*
-------------------------------------------------------------------------

	This file is part of the BoneClamp Data Conversion and Processing System
	Copyright (C) 2013 BoneClamp

-------------------------------------------------------------------------

	Filename: boneclamp.c

	To the extent possible under law, the author(s) have dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.

	You should have received a copy of the CC Public Domain Dedication along with
	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
*/

#include "boneclamp.h"

void rbInit(RingBuffer *rb, int size) {
    //put rb into appropriate memory locations
    rb->size = RB_UTILS_SIZE + USERSPACE_OFFSET;
    rb->start = RB_UTILS_START + USERSPACE_OFFSET;
    rb->count = RB_UTILS_COUNT + USERSPACE_OFFSET;
    rb->count = RB_UTILS_END + USERSPACE_OFFSET;
    rb->elems = RB_DATA + USERSPACE_OFFSET;
    rb->elems_end = RB_DATA_END_ADDRESS + USERSPACE_OFFSET;

    //intialize cb parameter/pointer values
    rb->*size = size;
    rb->*start = 0;
    rb->*count = 0;
    rb->*end = rb->elems + count*sizeof(data);
    rb->*elems = 0; //is this necessary? will initialize with data!
    rb->*elems_end = (RB_DATA + USERSPACE_OFFSET) + size*sizeof(data);
}

void rbFree(RingBuffer *rb) {
    free(rb->elems);
}

int rbIsFull(RingBuffer *rb) {
    return rb->count == rb->size;
}

int rbIsEmpty(RingBuffer *rb) {
    return rb->count == 0;
}

/*
//irrelevant for acquisition but might come in useful for transmission
//do we want to write integers or data structs (if relevant)?
void rbWrite(RingBuffer *rb, int data) {
    int end = (rb->*start + rb->*count) % rb->*size; //element index
    int endAddress = rb->*elems + end*sizeof(int);//element address
    //rb->*end = (rb->*start + rb->*count) % rb->*size;
    *endAddress = data;
    if (rb->*count == rb->*size)
        rb->*start = (rb->*start + 1) % rb->*size; //overwrite full
    else
        ++rb->*count;
}
*/

//read from the ring buffer
void rbRead(RingBuffer *rb, data *datapoint) {
    datapoint = rb->*start;
    rb->*start = (rb->*start + 1) % rb->*size;
    --rb->*count;
}

//Probably don't need this
/*bc_data bc_data_init()  {
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

    return bc_data;
}
*/

