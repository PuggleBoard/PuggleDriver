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

//#include "boneclamp.h"
#pragma once

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

#define DEF_RB_SIZE             32

//for handling buffer output in userspace
typedef struct data {
    int datap;
    int channelNumber;
} data;

//ring buffer structure
typedef struct RingBuffer {
    unsigned int size;
    void *start;
    void *count;
    void *end;
    void *elems;//start of elements
    void *elems_end;//end of elements
} RingBuffer;

void rbInit(RingBuffer *rb, unsigned int size) {
    //put rb into appropriate memory locations
    //rb->size = (unsigned int *)calloc(1, sizeof(int));
    //rb->size = (unsigned int *)calloc(1, sizeof(int));
    //rb->count = (unsigned int *)calloc(1, sizeof(int));
    //rb->end = (unsigned int *)calloc(1, sizeof(int));
    //rb->elems = (data *)calloc(1, sizeof(data));

    rb->size = (unsigned int *) (RB_UTILS_SIZE + USERSPACE_OFFSET);
    rb->start = (void *) (RB_UTILS_START + USERSPACE_OFFSET);
    //rb->count = (void *) (RB_UTILS_COUNT + USERSPACE_OFFSET);
    rb->end = (void *) (RB_UTILS_END + USERSPACE_OFFSET);
    rb->elems = (void *) (RB_DATA + USERSPACE_OFFSET);
    rb->elems_end = (void *) RB_DATA_END_ADDRESS + USERSPACE_OFFSET;

    //intialize rb parameter/pointer values
    data empty;
    empty.datap = 0;
    empty.channelNumber = 0;

    //unsigned int a = 1;

    //*(rb->size) = size;
    *(rb->start) = (void *) RB_DATA;
    //*(rb->count) = 0;
    *(rb->end) = 0; 
    *(rb->elems) = empty; //is this necessary? will initialize with data!
    //*(rb->elems_end) = (RB_DATA + USERSPACE_OFFSET) + size*sizeof(data);
}

void rbFree(RingBuffer *rb) {
    free(rb->elems);
    free(rb->size);
    free(rb->start);
    free(rb->count);
    free(rb->end);
    free(rb);
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
    datapoint = (data *) RB_UTILS_START + *(rb->start)*sizeof(data);
    *(rb->start) = (*(rb->start) + sizeof(data)) % *(rb->size);
    --*(rb->count);
}

