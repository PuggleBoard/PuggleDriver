//
//	 -------------------------------------------------------------------------
//
//	 This file is part of the Puggle Data Conversion and Processing System
//	 Copyright (C) 2013 Puggle
//
//	 -------------------------------------------------------------------------
//
//	Created in 2013 by: Yogi Patel <yapatel@gatech.edu>
//  Written by: Yogi Patel <yapatel@gatech.edu>
//              Jon Newman <jnewman6@gatech.edu>
//
//	To the extent possible under law, the author(s) have dedicated all copyright
//	and related and neighboring rights to this software to the public domain
//	worldwide. This software is distributed without any warranty.
//
//	You should have received a copy of the CC Public Domain Dedication along with
//	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
//

.origin 0
.entrypoint INIT

#include "puggle.hp"

#define CHAN_NUM        r3
#define ADC_CH1         r4
#define ADC_CH2         r5
#define ADC_CH3         r6
#define ADC_CH4         r7
#define DAC_CH1         r8
#define DAC_CH2         r9
#define DAC_CH3         r10
#define DAC_CH4         r11
#define ADC_INIT        r12
#define CYCLES     r13
#define INIT_CYCLES     r14
#define CONTROLS        r20
#define CUR_PAGE        r21
#define DAC_TX          r22
#define ADC_TX          r23
#define ADDR_PRU_SHARED r24
#define ADDR_PRU1_DRAM  r25
#define DAC_DATA        r27
#define ADC_DATA        r28

INIT:
// Enable OCP
LBCO  r0, CONST_PRUCFG, 4, 4
CLR   r0, r0, 4
SBCO  r0, CONST_PRUCFG, 4, 4

// Configure pointer register for PRU1 by setting c28_pointer[15:0] 0x00010000 (PRU shared RAM)
MOV   r0, 0x00000100
MOV   r1, CTPPR_1_0
ST32  r0, r1

// Configure pointer register for PRU1 by setting c31_pointer[15:0] to 0x80001000 (DDR memory)
MOV   r0, 0x00100000
MOV   r1, CTPPR_1_1
ST32  r0, r1

// Set counter for number of blocks copied
MOV CYCLES, 0

// Setup memory addresses
MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR
MOV ADDR_PRU1_DRAM, OWN_DRAM_ADDR

// Configure ADC/DAC channels
MOV DAC_CH1.b2, 0x31
MOV DAC_CH2.b2, 0x32
MOV DAC_CH3.b2, 0x34
MOV DAC_CH4.b2, 0x38
MOV ADC_CH1.w0, 0x0000
MOV ADC_CH2.w0, 0x1000
MOV ADC_CH3.w0, 0x2000
MOV ADC_CH4.w0, 0x3000

// Initialize onboard AIO SPI bus
SET DAC_CS
SET ADC_CS
SET SCLK
CLR MOSI

// Set initialization parameters
MOV CHAN_NUM, 1

// Run four initialization cycles (to get channels in phase)
MOV ADC_INIT, 0xe7ff
ADC_XFER_WORD ADC_DATA, ADC_INIT
MOV ADC_INIT, 0xc000
ADC_XFER_WORD ADC_DATA, ADC_INIT
ADC_XFER_WORD ADC_DATA, ADC_INIT
ADC_XFER_WORD ADC_DATA, ADC_INIT
 
// Move in config from PRU shared
LBCO CONTROLS, CONST_PRU_SHAREDRAM, 0, 4

// Wait until told to start
WBS CONTROLS.t0

// Start acquisition
SET_CHANNEL:
  
  // Trigger conversion using CONVST
  CLR CNV

  // Update config flags
  LBCO CONTROLS, CONST_PRU_SHAREDRAM, 0, 4
  
  // Copy data from PRU1 DRAM into each channel
  LBBO DAC_CH1.w0, ADDR_PRU1_DRAM, 0, 2
  LBBO DAC_CH2.w0, ADDR_PRU1_DRAM, 0, 2
  LBBO DAC_CH3.w0, ADDR_PRU1_DRAM, 0, 2
  LBBO DAC_CH4.w0, ADDR_PRU1_DRAM, 0, 2

  // Disable CONVST
  SET CNV

  // ~1600 ns delay for the conversion
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayFourty
  delayTwenty

  // Copy acquired 2 bytes to shared memory
  SBBO ADC_DATA.w0, ADDR_PRU_SHARED, 0, 2

  // Increment addresses by 2 bytes
  ADD ADDR_PRU_SHARED, ADDR_PRU_SHARED, 2
  ADD ADDR_PRU1_DRAM, ADDR_PRU1_DRAM, 2

  // Clear ADC_DATA for next round
  MOV ADC_DATA, 0

  // Incrememnt Counter
  ADD CYCLES, CYCLES, 1

  // Check PRU shared address
  QBLE CONTINUE, CYCLES, 200

    // Reset PRU Shared memory address
    MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR
    MOV ADDR_PRU1_DRAM, OWN_DRAM_ADDR
    MOV CYCLES, 0
    JMP MEM_DONE

  // Ensure timing consistency with RESET_ADDR label
  CONTINUE:
    NOP
    NOP
    NOP

MEM_DONE:
 
// Configure DAC Channel Number
QBEQ CH_1, CHAN_NUM, 1
QBEQ CH_2, CHAN_NUM, 2 
QBEQ CH_3, CHAN_NUM, 3 
QBEQ CH_4, CHAN_NUM, 4

CH_1:
    MOV DAC_TX, DAC_CH1
    MOV ADC_TX, ADC_CH1
    NOP
    NOP
    NOP
    NOP
    JMP START_ACQ
CH_2:
    MOV DAC_TX, DAC_CH2
    MOV ADC_TX, ADC_CH3
    NOP
    NOP
    NOP
    JMP START_ACQ
CH_3:
    MOV DAC_TX, DAC_CH3
    MOV ADC_TX, ADC_CH4
    NOP
    NOP
    JMP START_ACQ
CH_4:
    MOV DAC_TX, DAC_CH4
    MOV ADC_TX, ADC_CH1
    MOV CHAN_NUM, 0
    JMP START_ACQ


START_ACQ:

    // Collect ADC sample
    ADC_XFER_INV_WORD ADC_DATA, ADC_TX

    // Collect DAC sample
    DAC_XFER_WORD DAC_DATA, DAC_TX
  
    // Increment channel number
    //ADD CHAN_NUM, CHAN_NUM, 1

    // Check run/stop
    QBBS INIT_RESET, CONTROLS.t0
    JMP EXIT
    
    // Need to do this mini jump to get to reset
    // because there are too many instructions 
    // between the Check run/stop QBBS and
    // the SET_CHANNEL label
    INIT_RESET:
        JMP SET_CHANNEL

EXIT:
  // Copy acquired 2 bytes to shared memory
  SBBO ADC_DATA.w0, ADDR_PRU_SHARED, 0, 2
  
  // Increment address by 2 bytes
  ADD ADDR_PRU_SHARED, ADDR_PRU_SHARED, 2

  MOV r31.b0, PRU1_ARM_INTERRUPT+16
  HALT
