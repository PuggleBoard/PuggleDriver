.origin 0
.entrypoint START
#include "puggle.hp"

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

// Configure MCSPI0 channel 0 - RHD2132

// Disable channel 0
MOV addr, MCSPI0_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

// Configure channel 0
MOV addr, MCSPI0_CH0CONF     
MOV val, INTAN_CH0_CONF
SBBO val, addr, 0, 4

// Set XFER Level
MOV addr, MCSPI0_XFERLEVEL
MOV val, ADC_XFER
SBBO val, addr, 0, 4

// Configure RHD2132 registers
CONFIGURE:
CALL ENABLE_CH0

// Write R0 Config
MOV addr, MCSPI0_TX0
MOV val, 0x80FE 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R1 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8102
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R2 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8204
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R3 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8300
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R4 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8450
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R5 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8500
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R6 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8600
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R7 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8700
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R8 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8800
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R9 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8900
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R10 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8A00
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R11 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8B00
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R12 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8C00 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R13 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8D00 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R14 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8EFF 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R15 Config
MOV addr, MCSPI0_TX0
MOV val, 0x8FFF 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R16 Config
MOV addr, MCSPI0_TX0
MOV val, 0x90FF 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Write R17 Config
MOV addr, MCSPI0_TX0
MOV val, 0x91FF 
SBBO val, addr, 0, 4

CALL ENABLE_CH0

// Disable channel 0
MOV addr, MCSPI0_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

RUN_AQ:

// Read I from Register 40
CALL ENABLE_CH0

MOV addr, MCSPI0_TX0
MOV val, READ_I
SBBO val, addr, 0, 4

delay

// Read N from Register 41
CALL ENABLE_CH0

MOV addr, MCSPI0_TX0
MOV val, READ_N
SBBO val, addr, 0, 4

delay

// Read T from Register 42
CALL ENABLE_CH0

MOV addr, MCSPI0_TX0
MOV val, READ_T
SBBO val, addr, 0, 4

delay

// Read A from Register 43
CALL ENABLE_CH0

MOV addr, MCSPI0_TX0
MOV val, READ_A
SBBO val, addr, 0, 4

delay

// Read N from Register 44
CALL ENABLE_CH0

MOV addr, MCSPI0_TX0
MOV val, READ_N2
SBBO val, addr, 0, 4

delay

// Disable channel 0
MOV addr, MCSPI0_CH0CTRL
MOV val, DIS_CH
SBBO val, addr, 0 ,4

delay

JMP EXIT

//JMP RUN_AQ

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

EXIT:
#ifdef AM33XX
//Send notification to Host for program completion
MOV R31.b0, PRU0_ARM_INTERRUPT+16
#else
MOV R31.b0, PRU0_ARM_INTERRUPT
#endif

HALT
