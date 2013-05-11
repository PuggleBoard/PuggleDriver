.origin 0
.entrypoint START

#include "../sage_pru_p2cape.hp"

#define SCLK        HSPD_SPI_SCLK
#define MOSI        HSPD_SPI_MOSI
#define MISO        HSPD_SPI_MISO
#define CS          HSPD_SPI_CS

#define CUR_OFFSET      r16

#define RUN_FLAG        r17
#define ADDR_PRURAM		r18

#define NUM_SAMPLES     r19
#define NUM_PAGES       r20

#define CUR_SAMPLE		r21
#define CUR_PAGE        r22

#define SPI_TX			r23
#define SPI_RX			r24
#define SPI_CUR_BIT		r25

#define PRU1_CUR_PAGE   r26

#define ADDR_CUR_PAGE   r27

#define ADDR_SHARED     r28

#define XFER_CHUNK_SIZE  16

.macro SPI_XFER_WORD
.mparam RSPI_RX

    CLR CS

    MOV SPI_CUR_BIT, 16 // Read 16 bits total

    delay 40

SPI_XFER_WORD_LOOP:

//    Set MOSI to the current bit of SPI_TX
//    QBBS MOSI_HIGH, SPI_TX, SPI_CUR_BIT
//    CLR MOSI
//    JMP MOSI_DONE
//    MOSI_HIGH:
//    NOP
//    SET MOSI
//    MOSI_DONE:

    SET SCLK

    // Set b0 of R11 to MISO
    QBBC MISO_LOW, MISO
    OR RSPI_RX, RSPI_RX, 1
    JMP MISO_DONE
    // r11.b0 is already zero
    MISO_LOW:
    NOP
    NOP
    MISO_DONE:
    // Shift left R11 one bit
    LSL RSPI_RX, RSPI_RX, 1

    delay 15

    CLR SCLK

    delay 15

    SUB SPI_CUR_BIT, SPI_CUR_BIT, 1
    QBNE SPI_XFER_WORD_LOOP, SPI_CUR_BIT, 0

    delay 5

    SET CS

    delay 40

.endm

.macro SPI_XFER_DWORD
.mparam RSPI_RX
XOR RSPI_RX, RSPI_RX, RSPI_RX
SPI_XFER_WORD RSPI_RX
SPI_XFER_WORD RSPI_RX
LSR RSPI_RX, RSPI_RX, 1
.endm


START:

INIT:

    // Clear the register file
    ZERO 0, 64

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0100.  This will make C28 point to 0x00010000 (PRU shared RAM).
    MOV     r0, 0x000000100
    MOV     r1, CTPPR_0
    ST32    r0, r1

    // Set initial pin values
    SET CS
    CLR SCLK
    CLR MOSI

    // Init the high speed gain control
    HSPD_GAIN_INIT
    // Select channel 1
    CLR HSPD_SPI_MOSI

    // Set the high speed gain
    HSPD_SET_GAIN 0x66

    // Set the buffer size
    MOV NUM_SAMPLES, LOCAL_BUFFER_PAGE_SIZE

    // Save the address to write the current page for PRU1
    MOV PRU1_CUR_PAGE, LOCAL_BUFFER_INFO_OFFSET

    // Load the address of PRU0 RAM into ADDR_PRURAM
    MOV ADDR_PRURAM, MEM_PRU_DATA0_BASE

    // Init ADDR_CUR_PAGE to MEM_PRU_SHARED_BASE + LOCAL_BUFFER_INFO_OFFSET
    MOV r0, LOCAL_BUFFER_INFO_OFFSET
    MOV ADDR_CUR_PAGE, MEM_PRU_SHARED_BASE
    ADD ADDR_CUR_PAGE, ADDR_CUR_PAGE, r0

    MOV ADDR_SHARED, 0x00010000

    delay 32000 // 800 instructions per clock cycle

BUFFLOOP:

    // Reset current sample counter
    MOV CUR_OFFSET, 0

    // Reset current page
    MOV CUR_PAGE, 0


PAGELOOP:

    // Save the current page to the PRU1
    SBBO CUR_PAGE, ADDR_CUR_PAGE, 0, 2

    // Reset the sample counter
    MOV CUR_SAMPLE, 0

RUNLOOP:

	// Transfer 32 bytes via SPI

    SPI_XFER_DWORD r0
    SPI_XFER_DWORD r1
    SPI_XFER_DWORD r2
    SPI_XFER_DWORD r3

    // Save SPI result into CONST_PRU_SHAREDRAM+CUR_OFFSET
    SBBO r0, ADDR_SHARED, CUR_OFFSET, XFER_CHUNK_SIZE

	// Increment CUR_SAMPLE
    ADD CUR_SAMPLE, CUR_SAMPLE, XFER_CHUNK_SIZE
    ADD CUR_OFFSET, CUR_OFFSET, XFER_CHUNK_SIZE

    // Have we hit th buffer size?  If not, keep sampling
    // Go to RUNLOOP if CUR_SAMPLE < NUM_SAMPLES
    QBGT RUNLOOP, CUR_SAMPLE, NUM_SAMPLES

    ADD CUR_PAGE, CUR_PAGE, 1

    // Goto PAGELOOP if CUR_PAGE < LOCAL_BUFFER_NUM_PAGES
    QBGT PAGELOOP, CUR_PAGE, LOCAL_BUFFER_NUM_PAGES

	// Is the run flag still set?
	LBBO RUN_FLAG, ADDR_PRURAM, OFFSET(HostParamsPRUMemory.RunFlag), 4
    QBNE BUFFLOOP, RUN_FLAG, 0

	// Runloop is off, so QUIT
	JMP EXIT

EXIT:

    // Tell the host program we're DONE
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    HALT

