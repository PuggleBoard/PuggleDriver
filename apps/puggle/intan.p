.origin 0
.entrypoint START
#include "puggle.hp"

//////////////////////////////////////
START:
// Enable OCP master port
LBCO r0, C4, 4, 4
CLR r0, r0, 4
SBCO r0, C4, 4, 4

// Enable CLKSPIREF and CLK
MOV addr, CM_PER_SPI0_CLK_CTRL
MOV val, CM_PER_SPI0_CLK_EN
SBBO val, addr, 0, 4

// Reset SPI
MOV addr, MCSPI0_SYSCONFIG
LBBO val, addr, 0, 4
SET val.t1
SBBO val, addr, 0, 4

//////////////////////////////////////
// Validate reset
CHECKRESET:
MOV addr, MCSPI0_SYSSTATUS
LBBO val, addr, 0, 4
QBBC CHECKRESET, val.t0

// Configure MODULCTRL
MOV addr, MCSPI0_MODULCTRL
MOV val, MODCONTROL
SBBO val, addr , 0, 4

// Configure SYSCONFIG
MOV  addr, MCSPI0_SYSCONFIG
MOV  val, ADC_SYSCONFIG
SBBO val, addr, 0, 4

// Reset interrupt status bits by writing all ones
MOV addr, MCSPI0_IRQSTATUS
MOV val, RESET_IRQ_STAT 
SBBO val, addr, 0, 4

// Enable interupts
MOV addr, MCSPI0_IRQENABLE
MOV val, ADC_IRQENABLE
SBBO val, addr, 0, 4

//////////////////////////////////////
// Configure MCSPI0 channel 0 - RHD2132

// Disable channel 0
CALL DISABLE_CH0

// Configure channel 0
MOV addr, MCSPI0_CH0CONF     
MOV val, INTAN_CH0_CONF
SBBO val, addr, 0, 4

// Set XFER Level
MOV addr, MCSPI0_XFERLEVEL
MOV val, ADC_XFER
SBBO val, addr, 0, 4

//////////////////////////////////////
// ******************** CONFIGURE INTAN REGISTERS ********************

delay

CONFIGURE:

// Write R0 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x80DE 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R1 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8102
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R2 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8204
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R3 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8302
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R4 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x845F
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R5 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8500
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R6 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8600
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R7 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8700
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R8 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8811
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R9 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8980
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R10 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8A10
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R11 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8B80
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R12 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8C10 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R13 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8DDC 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R14 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8EFF 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R15 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x8FFF 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R16 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x90FF 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delayTwenty

// Write R17 Config
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, 0x91FF 
SBBO val, addr, 0, 4
CALL DISABLE_CH0

//////////////////////////////////////
// ******************** BEGIN ACQUISITION ********************

delay

// Clear convert command
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, CONVERT_ZERO
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delay

RUN_AQ:

// Read data in autocycle mode
CALL ENABLE_CH0
MOV addr, MCSPI0_TX0
MOV val, AUTO_ACQUIRE
SBBO val, addr, 0, 4
CALL DISABLE_CH0

delay

JMP RUN_AQ

//////////////////////////////////////
// Check to make sure data is ready
CHECKTX0:
MOV addr, MCSPI0_CH0STAT
LBBO val, addr, 0, 4
QBBC CHECKTX0, val.t1
JMP r18.w0

// Enable Channel 0
ENABLE_CH0:
MOV addr, MCSPI0_CH0CTRL
MOV val, EN_CH
SBBO val, addr, 0, 4
JAL r18.w0, CHECKTX0
RET

// DISABLE Channel 0
DISABLE_CH0:
MOV addr, MCSPI0_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4
delay
RET

EXIT:
#ifdef AM33XX
//Send notification to Host for program completion
MOV R31.b0, PRU0_ARM_INTERRUPT+16
#else
MOV R31.b0, PRU0_ARM_INTERRUPT
#endif

HALT
