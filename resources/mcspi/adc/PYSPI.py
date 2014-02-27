import struct
import time

class SPI_SETUP():

    @staticmethod    
    def setSYSCONFIG(CLOCKACTIVITY = 0x3, SIDLEMODE = 0x2, AUTOIDLE = 0x1):
        """
        sysconfig register setup
        """
        vCLOCKACTIVITY       = CLOCKACTIVITY << 8  # 0x3 ocp and functional clocks maintained
        vSIDLEMODE           = SIDLEMODE << 3      # 0x1 idle request ignored
        vAUTOIDLE            = AUTOIDLE               # 0x1 automatic ocp clock strategy is applied
    
        SYSCONFIG = 0x0000| vCLOCKACTIVITY | vSIDLEMODE | vAUTOIDLE
        print "SYSCONFIG "+ hex(SYSCONFIG) +"\n"

        return SYSCONFIG
    
    
    @staticmethod    
    def setXFERLEVEL(WCNT = 0x0, AFL = 1, AEL = 1 ):
        """
        Used to configure value to write to MCSPI_XFERLEVEL register
        units of AFL and AEL are in bytes
        """
        vWCNT                = WCNT << 16 # word count for 
        vAFL                 = (AFL - 1) << 8  #
        vAEL                 = (AEL - 1)       #
    
        XFER = 0x00000000| vWCNT | vAFL |vAEL
        print "XFERLEVEL " +  hex(XFER) +"\n"
        return XFER
    
    @staticmethod    
    def setIRQENABLE(EOWKE = 0x0, RX3_FULL_ENABLE = 0x0, TX3_UNDERFLOW_ENABLE = 0x0, TX3_EMPTY_ENABLE = 0x0,
    RX2_FULL_ENABLE = 0x0, TX2_UNDERFLOW_ENABLE = 0x0, TX2_EMPTY_ENABLE = 0x0,
    RX1_FULL_ENABLE = 0x0, TX1_UNDERFLOW_ENABLE = 0x0, TX1_EMPTY_ENABLE = 0x0,
    RX0_OVERFLOW_ENABLE = 0x0, RX0_FULL_ENABLE = 0x0, TX0_UNDERFLOW_ENABLE = 0x0, TX0_EMPTY_ENABLE = 0x0):
        """
        helper function that sets the interupts and outputs the value for the register
        There is an EWOK in the register!!
        """
    
        vEOWKE                   = EOWKE << 17
        vRX3_FULL_ENABLE         = RX3_FULL_ENABLE << 14
        vTX3_UNDERFLOW_ENABLE    = TX3_UNDERFLOW_ENABLE << 13
        vTX3_EMPTY_ENABLE        = TX3_EMPTY_ENABLE << 12
        vRX2_FULL_ENABLE         = RX2_FULL_ENABLE << 10
        vTX2_UNDERFLOW_ENABLE    = TX2_UNDERFLOW_ENABLE << 9
        vTX2_EMPTY_ENABLE        = TX2_EMPTY_ENABLE << 8
        vRX1_FULL_ENABLE         = RX1_FULL_ENABLE << 6
        vTX1_UNDERFLOW_ENABLE    = TX1_UNDERFLOW_ENABLE << 5
        vTX1_EMPTY_ENABLE        = TX1_EMPTY_ENABLE << 4
        vRX0_OVERFLOWENABLE      = RX0_OVERFLOW_ENABLE << 3
        vRX0_FULL_ENABLE         = RX0_FULL_ENABLE << 2
        vTX0_UNDERFLOW_ENABLE    = TX0_UNDERFLOW_ENABLE << 1
        vTX0_EMPTY_ENABLE        = TX0_EMPTY_ENABLE
        
        IRQENABLE = 0x00000000 | vEOWKE | vRX3_FULL_ENABLE| vTX3_UNDERFLOW_ENABLE|vTX3_EMPTY_ENABLE|vRX2_FULL_ENABLE|vTX2_UNDERFLOW_ENABLE|vTX2_EMPTY_ENABLE|vRX1_FULL_ENABLE|vTX1_UNDERFLOW_ENABLE|vTX1_EMPTY_ENABLE |vRX0_OVERFLOWENABLE | vRX0_FULL_ENABLE | vTX0_UNDERFLOW_ENABLE | vTX0_EMPTY_ENABLE
        
        print "IRQENABLE " +  hex(IRQENABLE)  +"\n"

        return IRQENABLE
    
    
    @staticmethod    
    def setSYST(SPIENDIR = 0x0, SPIDATDIR1 = 0x0, SPIDATDIR0 = 0x1, SPICLK = 0x0, SPIDAT_1 = 0x0, SPIDAT_0 = 0x0, SPIEN_1 = 0x0, SPIEN_0 = 0x0):
        """
        Helper function to setup System Test - only useful if operating in SYSTEM TEST mode
        """
        # SSB  ?
        vSPIENDIR                = SPIENDIR << 10     	# output in master mode
        vSPIDATDIR1              = SPIDATDIR1 << 9      # 0x0 output
        vSPIDATDIR0              = SPIDATDIR0 << 8      # 0x1 input
        vSPICLK                  = SPICLK << 6      		# driven low
        vSPIDAT_1                = SPIDAT_1 << 5
        vSPIDAT_0                = SPIDAT_0 << 4      	# driven low if output
        vSPIEN_1                 = SPIEN_1 << 1      		# driven low
        vSPIEN_0                 = SPIEN_0             	# driven low
        SYST = 0x00000000 | vSPIENDIR | vSPIDATDIR1 | vSPIDATDIR0 | vSPICLK | vSPIDAT_1 |vSPIDAT_0 | vSPIEN_1 | vSPIEN_0
        
        print "SYST " +  hex(SYST)  +"\n"

        return SYST
    
    @staticmethod    
    def setMODCONTROL(FDAA = 0x1, MOA = 0x0, INITDLY  = 0x0, SYSTEM_TEST = 0x0, MS = 0x0, PIN34  = 0x0, SINGLE  = 0x0):
        """
        Helper function that returns value for MODCONTROL register
    
        """
        vFDAA                   = (FDAA << 8)      		# 0x0 FIFO manged by CSPI_tx and rx registers
        vMOA                    = (MOA << 7)      		# multiple word access disabled
        vINITDLY                = (INITDLY << 4)      # no intial delay
        vSYSTEM_TEST            = (SYSTEM_TEST << 3)  # Functional mode
        vMS                     = (MS << 2)         	# This module is a master
        vPIN34                  = (PIN34 << 1)        # 0x0 SPIEN is used as chip select 
        vSINGLE                 = (SINGLE)            # 0x0 more than one channel will be used in master mode
        MODCONTROL = 0x00000000 | vFDAA| vMOA | vINITDLY | vSYSTEM_TEST | vMS | vPIN34 | vSINGLE
    
        print "MODCONTROL " + hex(MODCONTROL)  +"\n"

        return MODCONTROL
    
    @staticmethod    
    def setCH_CONF(CLKG = 0x0, FFER = 0x1, FFEW = 0x0, TCS = 0x0, SBPOL = 0x0, SBE = 0x0, SPIENSLV = 0x0, FORCE = 0x0, TURBO = 0x0, IS = 0x1, DPE1 = 0x1, DPE0 = 0x0, DMAR = 0x1, DMAW = 0x1, TRM = 0x0, WL = 0xF, EPOL = 0x1, CLKD = 0x1, POL = 0x1, PHA = 0x0):
        """
        Helper Function to set up and return a configuration for SPI channel
        !!come helper come!!
        """
    
        vCLKG                   = (CLKG << 29)       # 0x0 clock divider granularity power of 2
        vFFER                   = (FFER << 28)       # FIFO enabled for recieve, 0x0 not used
        vFFEW                   = (FFEW << 27)       # FIFO enabled for transmit, 0x0 not used
        vTCS                    = (TCS << 25)        # 0.5 clock cycle delay 
        vSBPOL                  = (SBPOL << 24)      # start bit held to zero
        vSBE                    = (SBE << 23)        # start bit enable  , 0x0 default set by WL
        vSPIENSLV               = (SPIENSLV << 21)   # spi select signal detection on ch 0
        vFORCE                  = (FORCE << 20)      # manual assertion to keep SPIEN active between SPI words
				vTURBO                  = (TURBO << 19)      # 0x0 turbo is deactivated 
				vIS                     = (IS << 18)         # Input select SPIDAT1 selected for reception
        vDPE1                   = (DPE1 << 17)       # 0x1 no Transmission enable for data line 1
        vDPE0                   = (DPE0 << 16)       # data line zero selected for transmission
        vDMAR                   = (DMAR << 15)       # DMA read request is enable
        vDMAW                   = (DMAW << 14)       # DMA write request is enable
        vTRM                    = (TRM << 12)        # Transmit only   
        vWL                     = (WL << 7)          # 16-bit for ADC
        vEPOL                   = (EPOL << 6)        # spien is held low during active state
        vCLKD                   = (CLKD << 2)        # 0x1 for DAC 24 MHz, 0x2 for ADC 16MHz
        vPOL                    = (POL << 1)         # SPI clock is held low during ative state
        vPHA                    = (PHA)              # data latched on odd numbered edges of SPICLK
        CH_CONF = 0x00000000 | vCLKG | vFFER | vFFEW | vTCS | vSBPOL | vSBE | vSPIENSLV| vFORCE | vTURBO | vIS | vDPE1 | vDPE0 | vDMAR | vDMAW | vTRM | vWL | vEPOL | vCLKD | vPOL | vPHA
    
        print "CH_CONF " + hex(CH_CONF)  +"\n"

        return CH_CONF

class Reg_Helper():   
    
    def getReg(self,address, mapped, length=32):
        """ Returns unpacked 16 or 32 bit register value starting from address. """
        if (length == 32):
            return struct.unpack("<L", mapped[address:address+4])[0]
        elif (length == 16):
            return struct.unpack("<H", mapped[address:address+2])[0]
        else:
            raise ValueError("Invalid register length: %i - must be 16 or 32" % length)
    
    
    def setReg(self,address, mapped, new_value, length=32):
        """ Sets 16 or 32 bits at given address to given value. """
        if (length == 32):
            mapped[address:address+4] = struct.pack("<L", new_value)
        elif (length == 16):
            mapped[address:address+2] = struct.pack("<H", new_value)
        else:
            raise ValueError("Invalid register length: %i - must be 16 or 32" % length)
    
    def printValue(self,register):
        print hex(register)
        #print"byte 1=" +str(bin(register & 0x000000ff))
        #print"byte 2=" +str(bin((register & 0x0000ff00) >> 8))
        #print"byte 3=" +str(bin((register & 0x00ff0000) >> 16))
        #print"byte 4=" +str(bin((register & 0xff000000) >> 24)) + "\n"
        print "byte4|byte3|byte2|byte1"
        print str(bin((register & 0xff000000) >> 24)) +"|"+ str(bin((register & 0x00ff0000) >> 16)) +"|"+ str(bin((register & 0x0000ff00) >> 8)) +"|"+ str(bin(register & 0x000000ff)) +"\n"

    
    def setAndCheckReg(self,address, mapped, new_value, name = "Reg"):
        print"value written: " + hex(new_value)
        self.setReg(address,mapped, new_value)
        value = self.getReg(address, mapped)
        print"register value of " + name +":"
        self.printValue(value)
    
        
    def checkValue(self, address, mapped, bit = 0, value = 1, name = "Reg"):
        reg = self.getReg(address, mapped)
        flag = value << bit
        print"check value of resgister" + name +":"
        self.printValue(reg)
        if ((reg & flag) == flag):
            return True
        else:
            return False
    
    
    def waitTillSet(self, address, mapped, bit = 0, value =1, name = "Reg", maxNum = 10):
        checkAgain = True
        count = 0
        check = True
        while((check == True) and (count < maxNum)):
            result = self.checkValue(address, mapped, bit, value, name)
            if (result == True):
                check = False
            elif(result == False): 
                time.sleep(0.000001)
                count += 1
        print count

    def grabAndSet(self, address, mapped, bit = 1, value = 0x1, name = "Reg"):
        sys = self.getReg(address, mapped)
        print"register value of" + name + ":\n"
        self.printValue(sys)
        reset = value << bit
        sysReset = sys | reset
        self.setReg(address, mapped, sysReset)


