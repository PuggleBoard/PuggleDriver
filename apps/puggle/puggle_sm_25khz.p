//   puggle_sm.p - PUGGLE'S SPI STATE MACHINE
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

#define ONBOARD_CHS     r3
#define INTAN_CHS       r4
#define INTAN_AUX_CHS   r5
#define INTAN_DATA1     r6
#define INTAN_DATA2     r7
//#define ADC_CH3         r6
//#define ADC_CH4         r7
//#define DAC_CH1         r8
//#define DAC_CH2         r9
//#define DAC_CH3         r10
//#define DAC_CH4         r11
//#define ADC_INIT        r12
#define CYCLES          r13
//#define INIT_CYCLES     r14
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

// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
MOV       r0, 0x00000000
MOV       r1, CTBIR_1
ST32      r0, r1

// Set counter for number of blocks copied
MOV CYCLES, 0

// Setup memory addresses
MOV ADDR_PRU_SHARED, PRU_SHARED_ADDR
MOV ADDR_PRU1_DRAM, OWN_DRAM_ADDR
MOV CONTROLS, COMMANDS_ADDR

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

// Intialize Intan SPI bus
CLR INTAN_SCLK
SET INTAN_CS1
SET INTAN_CS2
CLR INTAN_MOSI

// Onboard ADC initialization (four initialization cycles to get start channel at 1)
ADC_XFER_WORD ADC_DATA, 0xe7ff
ADC_XFER_WORD ADC_DATA, 0xc000
ADC_XFER_WORD ADC_DATA, 0xc000
ADC_XFER_WORD ADC_DATA, 0xc000

// Intan chip(s) intialization. 64 8 bit registers. First 17 must be configured.
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x80DE //R0
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8102 //R1 (25 kHz -> 825 kS/sec, so buffer bias = 2) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8204 //R2 (25 kHz -> 825 kS/sec, so mux bias = 4) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8302 //R3 (Temp sensors off; digout in HiZ) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x840C //R4 (Weak MISO is enabled; twoscomp enabled; absmode off; DSPen enabled; DSP cuttoff = 11 = 1.94 Hz, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8500 //R5 (Impedance check is off) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8600 //R6 (Impedance check DAC is off) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8700 //R7 (Impedance check amplifier is off) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8811 //R8 (offchip RH1 off; RH1 DAC1 = 17, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8900 //R9 (ADC aux1 off, MAKE CONFIGURABLE; RH1 DAC2 = 0, MAKE CONFIGURABLE ) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8A10 //R10 (offchip RH2 off; RH2 DAC1 = 16, MAKE CONFIGURABLE ) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8B00 //R11 (ADC aux2 off, MAKE CONFIGURABLE;RH2 DAC2 = 0, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8C11 //R8 (RH1 DAC1 = 17, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8D00 //R9 (RH1 DAC2 = 0, MAKE CONFIGURABLE ) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8E1C //R10 (offchip RL off; RL DAC1 = 16, MAKE CONFIGURABLE ) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8F06 //R11 (ADC aux3 off, MAKE CONFIGURABLE; RL DAC3 = 0; RL DAC2 = 6, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x8CFF //R11 (Channels 0-7 powered on, MAKE CONFIGURABLE) 
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x90FF //R11 (Channels 8-15 powered on, MAKE CONFIGURABLE)
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x91FF //R11 (Channels 16-23 powered on, MAKE CONFIGURABLE)
INTAN_XFER_WORD INTAN_DATA1, INTAN_DATA2, 0x92FF //R11 (Channels 23-31 powered on, MAKE CONFIGURABLE)

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
  //ADD ADDR_PRU1_DRAM, ADDR_PRU1_DRAM, 2

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
 
// Configure Channel Number
QBEQ CH_1, CONTROLS.b1, 1
QBEQ CH_2, CONTROLS.b1, 3
QBEQ CH_3, CONTROLS.b1, 7
QBEQ CH_4, CONTROLS.b1, 15

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
    //MOV CHAN_NUM, 0
    JMP START_ACQ

START_ACQ:

    // Collect ADC sample
    ADC_XFER_INV_WORD ADC_DATA, ADC_TX

    // Collect DAC sample
    DAC_XFER_WORD DAC_DATA, DAC_TX
  
    // Increment channel number if necessary
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
