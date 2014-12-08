2014.12.7
# Puggle Data Frame Specification

The PuggleBoard uses two types of data-frames. One frame is appropirate if an Intan amplifier chip is present. The other is appropriate when only the onboard ADC and DAC are being used.

## FRAME A - INTAN CHIP PRESENT

### Header
32-bit header: magic number equal to 0xC6911999 that can be used to check for bus data synchrony

### Time stamp
32-bit timestamp: sample counter that is incremented once per sample

### Intan data
16-bit intan result 0
16-bit intan result 1
16-bit intan result 2
16-bit intan result 3
16-bit intan result 4
16-bit intan result 5
16-bit intan result 6
16-bit intan result 7
16-bit intan result 8
16-bit intan result 9
16-bit intan result 10
16-bit intan result 11
16-bit intan result 12
16-bit intan result 13
16-bit intan result 14
16-bit intan result 15
16-bit intan result 16
16-bit intan result 17
16-bit intan result 18
16-bit intan result 19
16-bit intan result 20
16-bit intan result 21
16-bit intan result 22
16-bit intan result 23
16-bit intan result 24
16-bit intan result 25
16-bit intan result 26
16-bit intan result 27
16-bit intan result 28
16-bit intan result 29
16-bit intan result 30
16-bit intan result 31
16-bit intan result 32
16-bit intan result 33
16-bit intan result 34

### Onboard ADC
16-bit onboard ADC result 0
16-bit onboard ADC result 1
16-bit onboard ADC result 2
16-bit onboard ADC result 3

-----

## FRAME B - NO INTAN CHIP

### Header
32-bit header: magic number equal to 0xC6911999 that can be used to check for bus data synchrony

### Time stamp
32-bit timestamp: sample counter that is incremented once per sample

### Onboard ADC
16-bit onboard ADC result 0
16-bit onboard ADC result 1
16-bit onboard ADC result 2
16-bit onboard ADC result 3


