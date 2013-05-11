/*
-------------------------------------------------------------------------

	This file is part of the BoneClamp Data Conversion and Processing System
	Copyright (C) 2013 BoneClamp

-------------------------------------------------------------------------

	Filename: something.c

	To the extent possible under law, the author(s) have dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.

	You should have received a copy of the CC Public Domain Dedication along with
	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
*/

#include "boneclamp.h"
#include "maps.h"
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM             0
#define PRUSS0_SHAREDRAM    4
#define AM33XX

static int mem_fd;
static void *sharedMem;

static unsigned int *sharedMem_rbint;

int main()  {
    unsigned int ret;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    printf("\n--Test of ring buffer functionality--\n");

    printf("Initializing PRU...");
    prussdrv_init();
    printf("\t\t\t\tDone\n");

    ret = prussdrv_open(PRU_EVTOUT_0);
    printf("Checking prussdrv_open()...");
    if (ret) {
        printf("prussdrv_open failed!\n");
        printf("Aborting application!\n");
        return (ret);
    }
    printf("\t\t\tDone\n");

    
    printf("Initializing INTC...");
    prussdrv_pruintc_init(&pruss_intc_initdata);
    printf("\t\t\t\tDone\n");
    

    //map PRU shared RAM to userspace
    printf("Mapping PRU memory to userspace...");
    prussdrv_map_prumem(PRUSS0_SHAREDRAM, &sharedMem);
    printf("\t\t\tDone\n");

    //create and initialize ring buffer
    printf("Initializing ring buffer in PRU shared memory...");
    sharedMem_rbint = (unsigned int *) sharedMem;
    rbInit(sharedMem_rbint, DEF_RB_SIZE);
    printf("\t\t\t\tDone\n");

    //printf("Loading test program...\n");
    //prussdrv_exec_program(PRU_NUM, "./boneclamp.bin");
    prussdrv_exec_program(PRU_NUM, "./test.bin");

    /* read stuff from ring buffer */
    /*
    data *currentDatapoint = calloc(1, sizeof(data));
    while(1) {
        if (!rbIsEmpty(buffer)) {
            rbRead(buffer, currentDatapoint);
            printf("Channel: %i Value: %i", currentDatapoint->channelNumber, currentDatapoint->datap);
            if (currentDatapoint->datap == 20) {
                break;
            }
        }
        else
            printf("Buffer currently empty");
    }
    */

    //free(currentDatapoint);
    //rbFree(sharedMem_rb);

    //wait until PRU program is completed
    printf("Test complete. Do the results make sense?\n");
    prussdrv_pru_wait_event(PRU_EVTOUT_0);
    prussdrv_pru_clear_event(PRU_EVTOUT_0);

    //shut down the pru
    printf("Shutting down the PRU...");
    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit();
    //close(mem_fd);
    printf("\t\t\t\tDone\n");

	return 0;
}

