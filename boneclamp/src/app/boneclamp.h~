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
#include <sys/mman.h>
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

static int mem_fd;
//static void *rbMem, *sharedMem;

static void *rbBaseAddress, *rbStart, *rbEnd, *rbData;

static int bufferSize;

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

void rbInit(unsigned int *sharedMem, unsigned int size);
//void rbFree(RingBuffer *rb);
//int rbIsFull(RingBuffer *rb);
//int rbIsEmpty(RingBuffer *rb);
//void rbRead(RingBuffer *rb, data *datapoint);


void rbInit(unsigned int *sharedMem, unsigned int size) {   
    bufferSize = size;

    //point to pointers (sitting in shared RAM) to ring buffer elements
    rbBaseAddress = (unsigned long *) (RB_BASEADDR + USERSPACE_OFFSET); 
    rbStart = (unsigned long *) (rbBaseAddress + RB_UTILS_START_OFFSET);
    rbEnd = (unsigned long *) (rbBaseAddress + RB_UTILS_END_OFFSET);
    rbData = (unsigned long *) (rbBaseAddress + RB_DATA_OFFSET);
    printf("\nBuffer at %p\n", rbBaseAddress);
    printf("Buffer start at %p\n", &sharedMem[1]);
    //*(unsigned long *) rbStart = 0x4a318080;
    sharedMem[1] = 0x00000000;
    printf("At start: %x\n", sharedMem[0]);
    printf("At start: %x\n", sharedMem[1]);
    printf("At start: %x\n", sharedMem[2]);


    //memcpy(rbStart, rbData, sizeof(long));
    //*(unsigned long *) rbStart = rbBaseAddress + RB_DATA_OFFSET;

}
/*
void rbFree(RingBuffer *rb) {
    free(rb->elems);
    free(rb->size);
    free(rb->start);
    free(rb->count);
    free(rb->end);
    free(rb);
}
*/
/*
int rbIsFull(RingBuffer *rb) {
    return rb->count == rb->size;
}

int rbIsEmpty(RingBuffer *rb) {
    return rb->count == 0;
}
*/
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
/*
void rbRead(RingBuffer *rb, data *datapoint) {
    datapoint = (data *) RB_UTILS_START + *(rb->start)*sizeof(data);
    *(rb->start) = (*(rb->start) + sizeof(data)) % *(rb->size);
    --*(rb->count);
}
*/


