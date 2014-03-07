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

	// Configure MCSPI0 channel 0 - ADC

	// Disable channel 0
	MOV addr, MCSPI0_CH0CTRL
	MOV val, DIS_CH
	SBBO val, addr, 0 ,4

	// Configure channel 0
	MOV addr, MCSPI0_CH0CONF     
	MOV val, ADC_CH0_CONF
	SBBO val, addr, 0, 4

	// Set XFER Level
	MOV addr, MCSPI0_XFERLEVEL
	MOV val, ADC_XFER
	SBBO val, addr, 0, 4

	CONFIGURE:
	CALL ENABLE_CH0

	// Write ADC configuration to SPI_TX0
	MOV addr, MCSPI0_TX0
	MOV val, 0x0 //ADC_CONFIG
	SBBO val, addr, 0, 4

	// Disable channel 0
	MOV addr, MCSPI0_CH0CTRL
	MOV val, DIS_CH
	SBBO val, addr, 0 ,4

	delay

	RUN_AQ:

	CALL ENABLE_CH0

	// Write ADC configuration to SPI_TX0
	MOV addr, MCSPI0_TX0
	MOV val, 0x0 //READ_ADC_CONFIG
	SBBO val, addr, 0, 4

	// Disable channel 0
	MOV addr, MCSPI0_CH0CTRL
	MOV val, DIS_CH
	SBBO val, addr, 0 ,4

	delay

	JMP RUN_AQ

	CHECKTX0:
		MOV addr, MCSPI0_CH0STAT
		LBBO val, addr, 0, 4
		QBBC CHECKTX0, val.t1
		JMP r18.w0

	ENABLE_CH0:
		// Enable Channel 0
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
