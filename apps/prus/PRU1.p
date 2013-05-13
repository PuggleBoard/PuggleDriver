//
//	 -------------------------------------------------------------------------
//
//	 This file is part of the BoneClamp Data Conversion and Processing System
//	 Copyright (C) 2013 BoneClamp
//
//	 -------------------------------------------------------------------------
//
//	Written in 2013 by: Yogi Patel <yapatel@gatech.edu>
//
//	To the extent possible under law, the author(s) have dedicated all copyright
//	and related and neighboring rights to this software to the public domain
//	worldwide. This software is distributed without any warranty.
//
//	You should have received a copy of the CC Public Domain Dedication along with
//	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
//

.origin 0
.entrypoint START

#include "PRU_cape.hp"

#define ADDR_PRURAM      r16
#define ADDR_DDR         r17
#define ADDR_DDR_PARAMS  r18
#define CUR_SAMPLE       r19
#define CUR_PRU0_PAGE    r20.w0
#define CUR_PRU1_PAGE    r20.w2
#define CUR_PAGE_OFFSET  r21
#define NUM_PAGES        r22.w0
#define PAGE_SIZE        r22.w2
#define ADDR_CUR_PAGE    r23
#define ADDR_SHARED      r24
#define CUR_DDR_OFFSET   r25
#define NUM_DDR_PAGES    r26
#define CUR_DDR_PAGE        r27
#define TOTAL_PAGES_WRITTEN r28
#define XFER_CHUNK_SIZE  32 // Transfer 32 bytes at a time

START:
// Enable OCP master port
LBCO    r0, CONST_PRUCFG, 4, 4
CLR     r0, r0, 4
SBCO    r0, CONST_PRUCFG, 4, 4

// Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
// field to 0x0100.  This will make C28 point to 0x00010000 (PRU shared RAM).
//  MOV     r0, 0x000000100
//  MOV     r1, CTPPR_0
//  ST32    r0, r1

// Load the address of PRU0 RAM into ADDR_PRURAM
MOV ADDR_PRURAM, MEM_PRU_DATA1_BASE

MOV CUR_PRU0_PAGE, 0

MOV NUM_PAGES, LOCAL_BUFFER_NUM_PAGES
MOV PAGE_SIZE, LOCAL_BUFFER_PAGE_SIZE

LOAD_CONSTS:

// Init ADDR_CUR_PAGE to MEM_PRU_SHARED_BASE + LOCAL_BUFFER_INFO_OFFSET
MOV r0, LOCAL_BUFFER_INFO_OFFSET
MOV ADDR_CUR_PAGE, MEM_PRU_SHARED_BASE
ADD ADDR_CUR_PAGE, ADDR_CUR_PAGE, r0

// Get the DRAM base address from pru_mem[0] and put it into R5
LBBO  ADDR_DDR, ADDR_PRURAM, OFFSET(HostParamsPRUMemory.DDRBaseAddress), 4

LBBO  ADDR_DDR_PARAMS, ADDR_PRURAM, OFFSET(HostParamsPRUMemory.DDRSampleBytesAvailable), 4
ADD ADDR_DDR_PARAMS, ADDR_DDR_PARAMS, ADDR_DDR

LBBO  NUM_DDR_PAGES, ADDR_PRURAM, OFFSET(HostParamsPRUMemory.DDRPagesAvailable), 4

// Get the # of samples we have available and put it into R7
// LBBO    DDR_BUFFER_SIZE, ADDR_PRURAM, 4, 4

// MOV     ARM_CUR_SAMPLE, ADDR_DDR
// ADD     ARM_CUR_SAMPLE, ARM_CUR_SAMPLE, NUM_SAMPLES
// ADD     ARM_CUR_SAMPLE, ARM_CUR_SAMPLE, NUM_SAMPLES

MOV ADDR_SHARED, 0x00010000

MOV TOTAL_PAGES_WRITTEN, 0

DDR_PAGE_START:

MOV CUR_DDR_PAGE, 0
MOV CUR_DDR_OFFSET, 0

PAGE_START:

MOV CUR_PRU1_PAGE, 0
MOV CUR_PAGE_OFFSET, 0

XFER_LOOP:

// Check to see if the host is still running
LBBO r0, ADDR_PRURAM, OFFSET(HostParamsPRUMemory.RunFlag), 4
QBEQ EXIT, r0, 0

// TODO: NOP a bit so not to saturate the PRU BUS ?

// Load the PRU0 current page
LBBO CUR_PRU0_PAGE, ADDR_CUR_PAGE, 0, 2

// Loop if they are equal (nothing to transfer)
QBEQ XFER_LOOP, CUR_PRU1_PAGE, CUR_PRU0_PAGE

MOV CUR_SAMPLE, 0

XFER_PAGE:

// TODO: Transfer the page with DMA

///// Transfer
// Load 4 bytes @ CUR_PAGE_OFFSET into R0/R1
// LBCO r0, CONST_PRU_SHAREDRAM, CUR_PAGE_OFFSET, 8

LBBO r0, ADDR_SHARED, CUR_PAGE_OFFSET, XFER_CHUNK_SIZE

// Save 8 bytes from R0/R1 into ADDR_DDR+CUR_PAGE_OFFSET
SBBO r0, ADDR_DDR, CUR_DDR_OFFSET, XFER_CHUNK_SIZE

///// Incremement counters
ADD CUR_SAMPLE, CUR_SAMPLE, XFER_CHUNK_SIZE
ADD CUR_PAGE_OFFSET, CUR_PAGE_OFFSET, XFER_CHUNK_SIZE
ADD CUR_DDR_OFFSET, CUR_DDR_OFFSET, XFER_CHUNK_SIZE

// delay 100, r15

// Keep looping while CUR_SAMPLE < PAGE_SIZE
QBGT XFER_PAGE, CUR_SAMPLE, PAGE_SIZE

// Reset the sample counter
MOV CUR_SAMPLE, 0

// Write r27/r28 to ddr params
SBBO CUR_DDR_PAGE, ADDR_DDR_PARAMS, OFFSET(PRUParamsDDRMemory.LastWrittenDDRPage), 8

// Incremement the page #
ADD CUR_PRU1_PAGE, CUR_PRU1_PAGE, 1
ADD CUR_DDR_PAGE, CUR_DDR_PAGE, 1
ADD TOTAL_PAGES_WRITTEN, TOTAL_PAGES_WRITTEN, 1

// Jump to XFER_LOOP if CUR_PRU1_PAGE < NUM_PAGES
QBGT XFER_LOOP, CUR_PRU1_PAGE, NUM_PAGES

// Jump to XFER_LOOP if CUR_PRU1_PAGE < NUM_PAGES
QBGT PAGE_START, CUR_DDR_PAGE, NUM_DDR_PAGES

// Flip back to page 0
JMP DDR_PAGE_START

EXIT:

// Tell the host program we are DONE
MOV r31.b0, PRU1_ARM_INTERRUPT+16
HALT
