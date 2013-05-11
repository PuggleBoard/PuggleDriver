.setcallreg r28.w0
.origin 0
.entrypoint START

#include "../sage_pru.hp"

#define DELAY_TIME  40
#define DELAY   delay_ms DELAY_TIME

// Loop the whole thing 100 times
MOV r1, 4
START:

    CALL ALL_OFF
    SET SPI0_SCLK
    DELAY

    CALL ALL_OFF
    SET SPI0_MOSI
    DELAY

    CALL ALL_OFF
    SET SPI0_CS
    DELAY

    CALL ALL_OFF
    SET SPI1_SCLK
    DELAY
    
    CALL ALL_OFF
    SET SPI1_MOSI
    DELAY
    
    CALL ALL_OFF
    SET SPI1_CS
    DELAY

    SUB r1, r1, 1
    QBNE START, r1, 0
    
// Tell the host program we're DONE    
MOV R31.b0, PRU0_ARM_INTERRUPT+16
HALT

ALL_OFF:

    CLR SPI0_SCLK
    CLR SPI0_MOSI
    CLR SPI0_MISO
    CLR SPI0_CS
    
    CLR SPI1_SCLK
    CLR SPI1_MOSI
    CLR SPI1_MISO
    CLR SPI1_CS
    
    RET