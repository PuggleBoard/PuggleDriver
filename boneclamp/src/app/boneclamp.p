//
//-------------------------------------------------------------------------
//
//	This file is part of the BoneClamp Data Conversion and Processing System
//	Copyright (C) 2013 BoneClamp
//
//-------------------------------------------------------------------------
//
//	Filename: boneclamp.p
//
//	To the extent possible under law, the author(s) have dedicated all copyright
//	and related and neighboring rights to this software to the public domain
//	worldwide. This software is distributed without any warranty.
//
//	You should have received a copy of the CC Public Domain Dedication along with
//	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
//

#include "maps.h"

#define PRU0_ARM_INTERRUPT 35

#define WAIT_LOOPS      10
#define DATA_SIZE       0x0008
#define DATA_CHANNEL    7

.origin 0

//set ARM such that PRU can write to GPIO
LBCO r0, C4, 4, 4
CLR r0, r0, 4
SBCO r0, C4, 4, 4

//some other inits
MOV r1, 0   //counter
MOV r2, 0   //counter
MOV r20, RB_UTILS_SIZE
MOV r21, RB_UTILS_START
MOV r22, RB_UTILS_COUNT
MOV r23, RB_UTILS_END
MOV r24, RB_DATA
MOV r25, RB_DATA_END_ADDRESS
LBBO r14, r24, 0, 4
LBBO r15, r25, 0, 4

//generate data stream (eventually acquire datastream from SPI)
DATASTREAM:
    //delay
    ADD r1, r1, 1
    QBLT DATASTREAM, r1, WAIT_LOOPS
    //increment data value
    ADD r2, r2, 1
    //reset delay counter and push to buffer
    MOV r1, 0
    QBA RINGBUFFER

//push to ring buffer
RINGBUFFER:
    //load end pointer
    LBBO r13, r23, 0, 4
    //load element count from buffer
    LBBO r12, r22, 0, 4
    //write data point to buffer
    SBBO r2, r13, 0, &r1
    //write channel number to buffer
    SBBO r2, r13, 4, DATA_CHANNEL
    //increment end pointer and element count
    ADD r13, r13, DATA_SIZE
    ADD r12, r12, 1
    //if necessary, wrap around
    QBGE AFTER_RESET, r13, r15
    MOV r13, r24
    SUB r12, r12, 1
    AFTER_RESET:
    //put end pointer back in shared memory
    SBBO r13, r23, 0, 4
    //generate next datapoint
    QBGE FINISH, r2, 20
    QBA DATASTREAM

FINISH:
    //tell host that program is complete
    MOV R31.b0, PRU0_ARM_INTERRUPT
    HALT
