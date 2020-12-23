/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_FLASH.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined SIZE_OF_NVRAM
    #define SIZE_OF_NVRAM 0
#endif

#if defined SPI_SW_UPLOAD || defined SPI_FLASH_FAT || (defined SPI_FILE_SYSTEM && defined FLASH_FILE_SYSTEM)
    #define SPI_FLASH_ENABLED 
#endif
#if (defined SPI_EEPROM_FILE_SYSTEM && defined FLASH_FILE_SYSTEM)
    #define SPI_EEPROM_ENABLED 
#endif



/* =================================================================== */
/*                             constants                               */
/* =================================================================== */


/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

#if (defined FLASH_ROUTINES || defined ACTIVE_FILE_SYSTEM  || defined USE_PARAMETER_BLOCK || defined INTERNAL_USER_FILES) && defined _WINDOWS
    int iFetchingInternalMemory = _ACCESS_NOT_IN_CODE;
#endif


#if defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK
// This routine runs from SRAM - the reason why the pointer is passed is to avoid the routine taking it from a const value in FLASH, which is then not code location independent
//
    #if defined _WINDOWS || defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS
        #if defined _WINDOWS && defined USE_SECTION_PROGRAMMING
            static unsigned char ucFlexRam[FLEXRAM_SIZE] = {0};
            #undef FLEXRAM_START_ADDRESS
            #define FLEXRAM_START_ADDRESS  ucFlexRam
        #endif
    #endif
#endif

#if defined _WINDOWS                                                     // flex spi interface simulation
static const FLEXSPI_LUT_ENTRY *ptrFlexSPI_LUT[2] = {0};
static unsigned long ulValidRxFifoEntries[2] = {0};
static unsigned long ulValidTxFifoEntries[2] = {0};
static unsigned long ulTotalRxDataCounter[2] = {0};
static unsigned long ulTotalTxDataCounter[2] = {0};
static unsigned char *ptrRxFifo[2] = {0};
static unsigned char *ptrTxFifo[2] = {0};

static int fnSimFlexSPI(int iBus)
{
    static int iLUT_state[2] = {0};
    static const FLEXSPI_LUT_ENTRY *ptrActiveFlexSPI_Entry[2] = {0};
    static unsigned short usLength[2] = {0};
    static unsigned long ulCmdID[2] = {0};
    unsigned char ucCmd;
    unsigned char ucOp;
    unsigned char ucPads;
    iMX_FLEX_SPI *ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI_BLOCK;
    #if FLEX_SPI_INTERFACES > 1
    if (iBus != 0) {
        ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI2_BLOCK;
    }
    #endif
    if ((ptrFlexSPI->IPCMD & FLEX_SPI_IPCMD_TRG) != 0) {                 // command has been started
        const FLEXSPI_LUT_ENTRY *ptr_this_LUT = ptrFlexSPI_LUT[iBus];
        ptrFlexSPI->IPCMD &= ~(FLEX_SPI_IPCMD_TRG);
        usLength[iBus] = (ptrFlexSPI->IPCR1 & FLEX_SPI_IPCR1_IDATSZ_MASK);
        ulCmdID[iBus] = ((ptrFlexSPI->IPCR1 & FLEX_SPI_IPCR1_ISEQID_MASK) >> FLEX_SPI_IPCR1_ISEQID_SHIFT);
        while ((ptr_this_LUT->ulReference & 0x0f) != ulCmdID[iBus]) {
            ptr_this_LUT++;
            if (ptr_this_LUT->ulReference == NOR_CMD_LUT_SEQ_IDX_END_OF_TABLE) {
                _EXCEPTION("Invalid LUT entry reference!!");
            }
        }
        ptrActiveFlexSPI_Entry[iBus] = ptr_this_LUT;
        iLUT_state[iBus] = 0;
    }
    switch (iLUT_state[iBus]++) {
    case 0:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_OPERAND0) >> 0);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_OPCODE0) >> FLEX_SPI_LUT_OPCODE0_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_NUM_PADS0) >> FLEX_SPI_LUT_NUM_PADS0_SHIFT);
        break;
    case 1:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_OPERAND1) >> FLEX_SPI_LUT_OPERAND1_SHIFT);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_OPCODE1) >> FLEX_SPI_LUT_OPCODE1_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[0] & FLEX_SPI_LUT_NUM_PADS1) >> FLEX_SPI_LUT_NUM_PADS1_SHIFT);
        break;
    case 2:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_OPERAND0) >> 0);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_OPCODE0) >> FLEX_SPI_LUT_OPCODE0_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_NUM_PADS0) >> FLEX_SPI_LUT_NUM_PADS0_SHIFT);
        break;
    case 3:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_OPERAND1) >> FLEX_SPI_LUT_OPERAND1_SHIFT);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_OPCODE1) >> FLEX_SPI_LUT_OPCODE1_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[1] & FLEX_SPI_LUT_NUM_PADS1) >> FLEX_SPI_LUT_NUM_PADS1_SHIFT);
        break;
    case 4:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_OPERAND0) >> 0);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_OPCODE0) >> FLEX_SPI_LUT_OPCODE0_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_NUM_PADS0) >> FLEX_SPI_LUT_NUM_PADS0_SHIFT);
        break;
    case 5:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_OPERAND1) >> FLEX_SPI_LUT_OPERAND1_SHIFT);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_OPCODE1) >> FLEX_SPI_LUT_OPCODE1_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[2] & FLEX_SPI_LUT_NUM_PADS1) >> FLEX_SPI_LUT_NUM_PADS1_SHIFT);
        break;
    case 6:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_OPERAND0) >> 0);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_OPCODE0) >> FLEX_SPI_LUT_OPCODE0_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_NUM_PADS0) >> FLEX_SPI_LUT_NUM_PADS0_SHIFT);
        break;
    case 7:
        ucCmd = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_OPERAND1) >> FLEX_SPI_LUT_OPERAND1_SHIFT);
        ucOp = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_OPCODE1) >> FLEX_SPI_LUT_OPCODE1_SHIFT);
        ucPads = (unsigned char)((ptrActiveFlexSPI_Entry[iBus]->ulEntrySequence[3] & FLEX_SPI_LUT_NUM_PADS1) >> FLEX_SPI_LUT_NUM_PADS1_SHIFT);
        break;
    case 10:
        {
            unsigned long ulTxWatermark = ((((ptrFlexSPI->IPTXFCR & FLEX_SPI_IPTXFCR_TXWMRK_MASK) >> FLEX_SPI_IPTXFCR_TXWMRK_SHIFT) + 1) * 8);
            unsigned long ulFifoContent = ((ptrFlexSPI->IPTXFSTS & FLEX_SPI_IPTXFSTS_FILL_MASK) * 8);
            ptrTxFifo[iBus] = (unsigned char *)&ptrFlexSPI->TFDR0;
            while (usLength[iBus] != 0) {
                if (ulFifoContent < ulTxWatermark) {
                    usLength[iBus]--;
                    fnSimSPI_Flash(W25Q_WRITE, *ptrTxFifo[iBus]++);      // transmit data from tx fifo
                    ++ulValidTxFifoEntries[iBus];
                    ++ulTotalTxDataCounter[iBus];
                    ulFifoContent++;
                    ptrFlexSPI->IPTXFSTS = (((ulTotalTxDataCounter[iBus] / 8) << 16) | ((ulValidTxFifoEntries[iBus] + 7) / 8));
                }
                else {
                    iLUT_state[iBus] = 10;
                    return 1;
                }
            }
            iLUT_state[iBus] = 0;
            ptrFlexSPI->INTR |= (FLEX_SPI_INTR_IPTXWE);
            FlexSPI_CS = 1;
            fnSimSPI_Flash(W25Q_CHECK_SS, 0);                            // negate CS - simulate the SPI FLASH device
        }
        return 1;
    case 11:
        {
            unsigned long ulRxWatermark = ((((ptrFlexSPI->IPRXFCR & FLEX_SPI_IPRXFCR_RXWMRK_MASK) >> FLEX_SPI_IPRXFCR_RXWMRK_SHIFT) + 1) * 8);
            unsigned long ulFifoContent = ((ptrFlexSPI->IPRXFSTS & FLEX_SPI_IPRXFSTS_FILL_MASK) * 8);
            ptrRxFifo[iBus] = (unsigned char *)&ptrFlexSPI->RFDR0;
            while (usLength[iBus] != 0) {
                ptrFlexSPI->INTR |= FLEX_SPI_INTR_IPRXWA;
                if (ulFifoContent < ulRxWatermark) {
                    usLength[iBus]--;
                    *ptrRxFifo[iBus]++ = fnSimSPI_Flash(W25Q_READ, 0);   // fill the rx fifo with data
                    ++ulValidRxFifoEntries[iBus];
                    ++ulTotalRxDataCounter[iBus];
                    ulFifoContent++;
                    ptrFlexSPI->IPRXFSTS = (((ulTotalRxDataCounter[iBus] /8) << 16) | ((ulValidRxFifoEntries[iBus] + 7)/8));
                }
                else {
                    iLUT_state[iBus] = 11;
                    return 1;
                }
            }
            FlexSPI_CS = 1;
            fnSimSPI_Flash(W25Q_CHECK_SS, 0);                            // negate CS - simulate the SPI FLASH device
        }
        return 1;
    default:
        iLUT_state[iBus] = 0;
        break;
    }

    switch (ucOp) {
    case FLEXSPI_Command_STOP:                                           // stop execution, deassert CS
        FlexSPI_CS = 1;
        fnSimSPI_Flash(W25Q_CHECK_SS, 0);                                // negate CS - simulate the SPI FLASH device
        iLUT_state[iBus] = 0;
        break;
    case FLEXSPI_Command_DDR:                                            // transmit command code to flash, using DDR mode
    case FLEXSPI_Command_SDR:                                            // transmit command code to flash, using SDR mode
        FlexSPI_CS = 0;
        fnSimSPI_Flash(W25Q_CHECK_SS, 0);                                // assert CS - simulate the SPI FLASH device
        fnSimSPI_Flash(W25Q_WRITE, ucCmd);                               // send command byte        
        break;
    case FLEXSPI_Command_WRITE_DDR:                                      // transmit data to flash, using DDR mode
    case FLEXSPI_Command_WRITE_SDR:                                      // transmit data to flash, using SDR mode
        iLUT_state[iBus] = 10;
        return 1;                                                        // do nothing just yet but we will transmit the data from the tx fifo once it is available
    case FLEXSPI_Command_RADDR_DDR:                                      // transmit row address to flash, using DDR mode
    case FLEXSPI_Command_RADDR_SDR:                                      // transmit row address to flash, using SDR mode
        while (ucCmd >= 8) {
            ucCmd -= 8;
            switch (ucCmd) {
            case 24:
    #if defined SPI_FLASH_S26KL
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> (ucCmd + 3))); // send address
    #else
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> ucCmd)); // send address
    #endif
                break;
            case 16:
    #if defined SPI_FLASH_S26KL
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> (ucCmd + 3))); // send address
    #else
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> ucCmd)); // send address
    #endif
                break;
            case 8:
    #if defined SPI_FLASH_S26KL
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> (ucCmd + 3))); // send address
    #else
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> ucCmd)); // send address
    #endif
                break;
            case 0:
    #if defined SPI_FLASH_S26KL
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> 3)); // send address
    #else
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> ucCmd)); // send address
    #endif
                break;
            default:
                _EXCEPTION("Unexpected row address length!");
                break;
            }
        }
        return 0;
    case FLEXSPI_Command_CADDR_DDR:                                      // transmit column address to flash, using DDR mode
    case FLEXSPI_Command_CADDR_SDR:                                      // transmit column address to flash, using SDR mode
        while (ucCmd >= 8) {
            ucCmd -= 8;
    #if defined SPI_FLASH_S26KL
            if (ucCmd > 0) {
                fnSimSPI_Flash(W25Q_WRITE, 0);                           // dummy byte
            }
            else {
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 & 0x07)); // send column address (3 bits are valid)
            }
    #else
            fnSimSPI_Flash(W25Q_WRITE, (unsigned char)(ptrFlexSPI->IPCR0 >> ucCmd)); // send column address
    #endif
        }
        return 0;
    case FLEXSPI_Command_DUMMY_SDR:                                      // leave data lines undriven by FlexSPI controller
    case FLEXSPI_Command_DUMMY_DDR:                                      // leave data lines undriven by FlexSPI controller
        return 0;
    case FLEXSPI_Command_READ_DDR:                                       // receive read data from flash, using DDR mode
    case FLEXSPI_Command_READ_SDR:                                       // receive read data from flash, using SDR mode
        {
            unsigned long ulRxWatermark = ((((ptrFlexSPI->IPRXFCR & FLEX_SPI_IPRXFCR_RXWMRK_MASK) >> FLEX_SPI_IPRXFCR_RXWMRK_SHIFT) + 1) * 8);
            unsigned long ulFifoContent = ((ptrFlexSPI->IPRXFSTS & FLEX_SPI_IPRXFSTS_FILL_MASK) * 8);
            while (usLength[iBus] != 0) {
                ptrFlexSPI->INTR |= FLEX_SPI_INTR_IPRXWA;
                if (ulFifoContent < ulRxWatermark) {
                    usLength[iBus]--;
                    *ptrRxFifo[iBus]++ = fnSimSPI_Flash(W25Q_READ, 0);   // fill the rx fifo with data
                    ++ulValidRxFifoEntries[iBus];
                    ++ulTotalRxDataCounter[iBus];
                    ulFifoContent++;
                    ptrFlexSPI->IPRXFSTS = (((ulTotalRxDataCounter[iBus] / 8) << 16) | ((ulValidRxFifoEntries[iBus] + 7)/8));
                }
                else {
                    iLUT_state[iBus] = 11;
                    return 1;
                }
            }
            FlexSPI_CS = 1;
            fnSimSPI_Flash(W25Q_CHECK_SS, 0);                            // negate CS - simulate the SPI FLASH device
            return 1;
        }
        break;
    case FLEXSPI_Command_LEARN_SDR:                                      // receive read data or preamble bit from flash, SDR mode
    case FLEXSPI_Command_DATSZ_SDR:                                      // transmit read/program data size (byte) to flash, SDR mode
    case FLEXSPI_Command_DUMMY_RWDS_SDR:                                 // leave data lines undriven by FlexSPI controller, dummy cycles decided by RWDS
    case FLEXSPI_Command_JUMP_ON_CS:                                     // stop execution, deassert CS and save operand[7:0] as the instruction start pointer for next sequence
    case FLEXSPI_Command_MODE1_DDR:                                      // transmit 1-bit mode bits to flash, using DDR mode
    case FLEXSPI_Command_MODE2_DDR:                                      // transmit 2-bit mode bits to flash, using DDR mode
    case FLEXSPI_Command_MODE4_DDR:                                      // transmit 4-bit Mode bits to flash, using DDR mode
    case FLEXSPI_Command_MODE8_DDR:                                      // transmit 8-bit Mode bits to flash, using DDR mode
    case FLEXSPI_Command_LEARN_DDR:                                      // receive read data or preamble bit from flash, DDR mode
    case FLEXSPI_Command_DATSZ_DDR:                                      // transmit read/program data size (byte) to flash, DDR mode
    case FLEXSPI_Command_DUMMY_RWDS_DDR:                                 // leave data lines undriven by FlexSPI controller, dummy cycles decided by RWDS
    case FLEXSPI_Command_MODE1_SDR:                                      // transmit 1-bit mode bits to flash, using SDR mode
    case FLEXSPI_Command_MODE2_SDR:                                      // transmit 2-bit mode bits to flash, using SDR mode
    case FLEXSPI_Command_MODE4_SDR:                                      // transmit 4-bit mode bits to flash, using SDR mode
    case FLEXSPI_Command_MODE8_SDR:                                      // transmit 8-bit mode bits to flash, using SDR mode
        _EXCEPTION("To do if used!!");
        break;
    default:
        _EXCEPTION("Invalid command!!");
        break;
    }
    return 0;
}
#endif

static void fnFlexSPI_Command(int iBus, int iID, unsigned long ulLength, unsigned char *ptrDataIn, unsigned char *ptrDataOut, unsigned long ulOffset)
{
#if defined SPI_FLASH_S26KL
    int iNumber = (iID >> 24);                                           // extract optional sequence number
#endif
#if !defined iMX_BOOTLOADER
    unsigned long ulMaxOffset = FLEXSPI_FLASH_SIZE;
#endif
    iMX_FLEX_SPI *ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI_BLOCK;
#if FLEX_SPI_INTERFACES > 1
    if (iBus != 0) {
        ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI2_BLOCK;
    #if !defined iMX_BOOTLOADER && defined FLEXSPI2_FLASH_SIZE
        ulMaxOffset = FLEXSPI2_FLASH_SIZE;
    #endif
    }
#endif
    ptrFlexSPI->FLSHA1CR2 |= FLEX_SPI_FLSHCR2_CLRINSTRPTR;
    WRITE_ONE_TO_CLEAR(ptrFlexSPI->INTR, (FLEX_SPI_INTR_IPCMDDONE | FLEX_SPI_INTR_IPCMDGE | FLEX_SPI_INTR_AHBCMDGE | FLEX_SPI_INTR_IPCMDERR | FLEX_SPI_INTR_AHBCMDERR | FLEX_SPI_INTR_IPRXWA | /*FLEX_SPI_INTR_IPTXWE |*/ FLEX_SPI_INTR_AHBBUSTIMEOUT | FLEX_SPI_INTR_SEQTIMEOUT)); // clear error flags
#if !defined iMX_BOOTLOADER
    if (ulOffset >= ulMaxOffset) {
        fnDebugMsg("Bad offset!");
        return;
    }
#endif
    ptrFlexSPI->IPCR0 = ulOffset;
    ptrFlexSPI->IPTXFCR |= FLEX_SPI_IPTXFCR_CLRIPTXF;                    // reset read/write pointers in IP transmit fifo
    ptrFlexSPI->IPRXFCR |= FLEX_SPI_IPRXFCR_CLRIPRXF;                    // reset read/write pointers in IP receive fifo
#if defined _WINDOWS
    ulValidRxFifoEntries[iBus] = 0;
    ulTotalRxDataCounter[iBus] = 0;
    ptrFlexSPI->IPRXFSTS = 0;
    ptrRxFifo[iBus] = (unsigned char *)&ptrFlexSPI->RFDR0;
    ulValidTxFifoEntries[iBus] = 0;
    ulTotalTxDataCounter[iBus] = 0;
    ptrFlexSPI->IPTXFSTS = 0;
    ptrTxFifo[iBus] = (unsigned char *)ptrFlexSPI->TFDR0;
#endif
#if defined SPI_FLASH_S26KL
    ptrFlexSPI->IPCR1 = (((iID << FLEX_SPI_IPCR1_ISEQID_SHIFT) & FLEX_SPI_IPCR1_ISEQID_MASK) | ((iNumber << FLEX_SPI_IPCR1_ISEQNUM_SHIFT) & FLEX_SPI_IPCR1_ISEQNUM_MASK) | ulLength);
#else
    ptrFlexSPI->IPCR1 = (((iID << FLEX_SPI_IPCR1_ISEQID_SHIFT) & FLEX_SPI_IPCR1_ISEQID_MASK) | ulLength);
#endif
    ptrFlexSPI->IPCMD = FLEX_SPI_IPCMD_TRG;                              // manually start the transfer
#if defined _WINDOWS
    #if defined SPI_FLASH_S26KL
    do {
    #endif
        if (fnSimFlexSPI(iBus) == 0) {                                   // simulate executing a step in an entry
            if (fnSimFlexSPI(iBus) == 0) {                               // simulate executing a step in an entry
                if (fnSimFlexSPI(iBus) == 0) {                           // simulate executing a step in an entry
                    if (fnSimFlexSPI(iBus) == 0) {                       // simulate executing a step in an entry
                        if (fnSimFlexSPI(iBus) == 0) {                   // simulate executing a step in an entry
                            if (fnSimFlexSPI(iBus) == 0) {               // simulate executing a step in an entry
                                if (fnSimFlexSPI(iBus) == 0) {           // simulate executing a step in an entry
                                    fnSimFlexSPI(iBus);                  // simulate executing a step in an entry
                                }
                            }
                        }
                    }
                }
            }
        }
    #if defined SPI_FLASH_S26KL
        if (iNumber != 0) {
            ptrFlexSPI->IPCMD = FLEX_SPI_IPCMD_TRG;                      // manually start the following transfer (only requird by simulation and not HW)
            iNumber--;
            ptrFlexSPI->IPCR1 = (((++iID << FLEX_SPI_IPCR1_ISEQID_SHIFT) & FLEX_SPI_IPCR1_ISEQID_MASK) | ((iNumber << FLEX_SPI_IPCR1_ISEQNUM_SHIFT) & FLEX_SPI_IPCR1_ISEQNUM_MASK) | ulLength);
        }
        else {
            break;
        }
    } while (1 != 0);
    #endif
#endif
    if (ptrDataIn != 0) {
        unsigned long ulRxWatermark = ((((ptrFlexSPI->IPRXFCR & FLEX_SPI_IPRXFCR_RXWMRK_MASK) >> FLEX_SPI_IPRXFCR_RXWMRK_SHIFT) + 1) * 8);
        while (ulLength != 0) {
            if (ulLength > ulRxWatermark) {
                _WAIT_REGISTER_FALSE(ptrFlexSPI->INTR, FLEX_SPI_INTR_IPRXWA); // wait until the rx fifo has filled
                uMemcpy(ptrDataIn, (const void *)&ptrFlexSPI->RFDR0, ulRxWatermark);
                ptrDataIn += ulRxWatermark;
                ulLength -= ulRxWatermark;
            }
            else {
                while (ulLength > ((ptrFlexSPI->IPRXFSTS & FLEX_SPI_IPRXFSTS_FILL_MASK) * 8)) {} // wait until the requested amount of data has been received
                uMemcpy(ptrDataIn, (const void *)&ptrFlexSPI->RFDR0, ulLength);
                break;
            }
            WRITE_ONE_TO_CLEAR(ptrFlexSPI->INTR, (FLEX_SPI_INTR_IPRXWA));// pop data
#if defined _WINDOWS
            ptrFlexSPI->IPRXFSTS = 0;
            fnSimFlexSPI(iBus);                                          // simulate
#endif
        }
    }
    else if (ptrDataOut != 0) {
        unsigned long ulTxWatermark = ((((ptrFlexSPI->IPTXFCR & FLEX_SPI_IPTXFCR_TXWMRK_MASK) >> FLEX_SPI_IPTXFCR_TXWMRK_SHIFT) + 1) * 8);
        while (ulLength != 0) {
            _WAIT_REGISTER_FALSE(ptrFlexSPI->INTR, FLEX_SPI_INTR_IPTXWE);// wait until there is space in the tx fifo
            if (ulLength > ulTxWatermark) {
                uMemcpy((void *)&ptrFlexSPI->TFDR0, ptrDataOut, ulTxWatermark);
                WRITE_ONE_TO_CLEAR(ptrFlexSPI->INTR, (FLEX_SPI_INTR_IPTXWE)); // push data
#if defined _WINDOWS
                ptrFlexSPI->IPTXFSTS = 0;
                fnSimFlexSPI(iBus);                                      // simulate
#endif
                ulLength -= ulTxWatermark;
                ptrDataOut += ulTxWatermark;
            }
            else {
                uMemcpy((void *)&ptrFlexSPI->TFDR0, ptrDataOut, ulLength);
                break;
            }
        }
        WRITE_ONE_TO_CLEAR(ptrFlexSPI->INTR, (FLEX_SPI_INTR_IPTXWE));    // push data
#if defined _WINDOWS
        ptrFlexSPI->IPTXFSTS = 0;
        fnSimFlexSPI(iBus);                                              // simulate
#endif
    }
    while ((ptrFlexSPI->STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle
}


#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
static void fnPerformJedecReset(void)
{
    int i;
    #define FLEX_SPI_SS0     PORT3_BIT6
    #define FLEX_SPI_SCLK    PORT3_BIT7
    #define FLEX_SPI_DATA00  PORT3_BIT8

    // Configure the three pins involved in the JEDEC reset as GPIOs outputs initially driving SS0 and DATA00 high and SCLK low
    //
    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(3, (FLEX_SPI_SS0 | FLEX_SPI_SCLK | FLEX_SPI_DATA00), (FLEX_SPI_SS0 | FLEX_SPI_DATA00), (PORT_SRE_SLOW | PORT_DSE_MID));
    // Perform the reset sequence by toggling CS low four times with alternating values of DATA00 (SCLK can be either high or low but must remain stable)
    //
    for (i = 0; i < 4; i++) {                                            // generate 4 clocks
        _CLEARBITS(3, FLEX_SPI_SS0);                                     // set SS0 low
        _TOGGLE_PORT(3, FLEX_SPI_DATA00);                                // toggle the state of DATA00
        _SETBITS(3, FLEX_SPI_SS0);                                       // set SS0 high
    }

    // Reconfigure the SS0, SCLK and DATA00 lines to peripheral functions
    //
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_SS0_B,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO3-06 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO3-07 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO3-08 - alt function 1

    fnDelayLoop(120);                                                    // delay while the flash settles
}
#endif

static void fnInitSpiFlash(int iBus)
{
    POWER_UP_ATOMIC(6, FLEXSPI_CLOCKS);                                  // enable clocks to FlexSPI
#if defined MIMXRT1024 || defined MIMXRT1064
    if (iBus != 0) {                                                     // internal bus
    #if !defined MIMXRT1024                                              // 1024 pinout is not yet known
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_02 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE); // FLEXSPI2_A_DATA0
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_03 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE); // FLEXSPI2_A_DATA1
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_02 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE); // FLEXSPI2_A_DATA2
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_10 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE); // FLEXSPI2_A_DATA3
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_08 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE); // FLEXSPI2_A_SCLK
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_06 = (IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE);  //FLEXSPI2_A_SS0_B
        IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_09 = (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE);// FLEXSPI2_A_DQS
    #endif

        FLEX_SPI2_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_ARDFEN | FLEX_SPI_MCR0_ATDFEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_SWRESET); // perform a reset of the module
        _WAIT_REGISTER_TRUE(FLEX_SPI2_MCR0, FLEX_SPI_MCR0_SWRESET);      // wait for reset to complete

        FLEX_SPI2_MCR0 = (FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_DQS_LOOPBACK | FLEX_SPI_MCR0_MDIS);
        FLEX_SPI2_MCR1 = (FLEX_SPI_MCR1_AHBBUSWAIT_MASK | FLEX_SPI_MCR1_SEQWAIT_MASK);
        FLEX_SPI2_MCR2 = (FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
        FLEX_SPI2_AHBCR = (FLEX_SPI_AHBCR_READADDROPT | FLEX_SPI_AHBCR_PREFETCHEN | FLEX_SPI_AHBCR_BUFFERABLEEN | FLEX_SPI_AHBCR_CACHABLEEN);
        FLEX_SPI2_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
        FLEX_SPI2_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
        FLEX_SPI2_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
        FLEX_SPI2_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
        FLEX_SPI2_IPTXFCR = 0;                                           // configure IP fifo watermarks
        FLEX_SPI2_IPRXFCR = 0;
        FLEX_SPI2_FLSHA1CR0 = (0);                                       // reset the flash size on all ports
        FLEX_SPI2_FLSHA2CR0 = (0);
        FLEX_SPI2_FLSHB1CR0 = (0);
        FLEX_SPI2_FLSHB2CR0 = (0);

        while ((FLEX_SPI2_STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle

        FLEX_SPI2_FLSHA1CR0 = (FLEXSPI2_FLASH_SIZE / 1024);              // flash size in KBytes

        FLEX_SPI2_FLSHA1CR1 = (0x20063);                                 // flash parameters
      //FLEX_SPI2_FLSHA1CR2 = (2);                                       // operation items
        FLEX_SPI2_FLSHA1CR2 = (0);                                       // operation items
        FLEX_SPI2_DLLACR = (FLEX_SPI_DLLnCR_OVRDEN);                     // configure DLL
        FLEX_SPI2_FLSHCR4 = FLEX_SPI_FLSHCR4_WMOPT1;                     // write mask configuration

        FLEX_SPI2_MCR0 = FLEX_SPI_MCR0_RESERVED_ONE | (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_DQS_LOOPBACK); // exit stop mode
        return;
    }
#endif
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_05, FLEXSPI_A_DQS,    (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DQS    on GPIO3-05 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_SS0_B,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO3-06 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO3-07 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_04, FLEXSPI_B_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_B_SCLK   on GPIO3-04 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO3-08 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_09, FLEXSPI_A_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA01 on GPIO3-09 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO3-10 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_11, FLEXSPI_A_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA03 on GPIO3-11 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_03, FLEXSPI_B_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_B_DATA00 on GPIO3-08 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_02, FLEXSPI_B_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_B_DATA01 on GPIO3-09 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_01, FLEXSPI_B_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_B_DATA02 on GPIO3-10 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_00, FLEXSPI_B_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_B_DATA03 on GPIO3-11 - alt function 1

    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(3, (PORT3_BIT10), (PORT3_BIT10), (PORT_SRE_SLOW | PORT_DSE_MID)); // FLEXSPI_A_DATA02 set to '1' to hold WP pin high in SPI mode (temp)

    fnPerformJedecReset();                                           // perform a jedec reset

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_ARDFEN | FLEX_SPI_MCR0_ATDFEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_SWRESET); // perform a reset of the module
    _WAIT_REGISTER_TRUE(FLEX_SPI_MCR0, FLEX_SPI_MCR0_SWRESET);       // wait for reset to complete

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_COMBINATION_EN | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_STROBE | FLEX_SPI_MCR0_MDIS) ;
    FLEX_SPI_MCR1 = 0xffffffff;
    FLEX_SPI_MCR2 = (FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    FLEX_SPI_AHBCR = FLEX_SPI_AHBCR_PREFETCHEN;
    FLEX_SPI_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT);
    FLEX_SPI_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT);
    FLEX_SPI_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT);
    FLEX_SPI_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT);
    FLEX_SPI_IPRXFCR = 0;
    FLEX_SPI_IPTXFCR = 0;
    FLEX_SPI_FLSHA1CR0 = 0;
    FLEX_SPI_FLSHA2CR0 = 0;
    FLEX_SPI_FLSHB1CR0 = 0;
    FLEX_SPI_FLSHB2CR0 = 0;

    while ((FLEX_SPI_STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle

    FLEX_SPI_FLSHA1CR0 = (FLEXSPI_FLASH_SIZE / 1024);                    // flash size in kBytes
    FLEX_SPI_FLSHA1CR1 = 0x00050044;
    FLEX_SPI_FLSHA1CR2 = 0;
    #if FLEXSPI_CLK_ROOT_FREQUENCY >= 100000000
    FLEX_SPI_DLLACR = 0x00000079;
    #else
    FLEX_SPI_DLLACR = 0x00001d00;
    #endif
    FLEX_SPI_FLSHCR4 = FLEX_SPI_FLSHCR4_WMOPT1;
    FLEX_SPI_MCR0 &= ~(FLEX_SPI_MCR0_MDIS);
#elif defined MIMXRT1010 || defined MIMXRT1015 || defined MIMXRT1020 || defined MIMXRT1024 || defined MIMXRT1060 || defined MIMXRT1064 || defined TEENSY_4_0 || defined TEENSY_4_1 || defined MIMXRT1170 // test vehicles
    // Initialise pins to the QSPI flash device
    //
    #if defined TEENSY_4_0 || defined TEENSY_4_1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_SS0_B, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO3-06 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO3-07 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO3-08 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_09, FLEXSPI_A_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA01 on GPIO3-09 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO3-10 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_11, FLEXSPI_A_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA03 on GPIO3-11 - alt function 1
    #elif defined MIMXRT1060 || defined MIMXRT1064 || defined MIMXRT1170
        #if !defined APPLICATION_REQUIRES_GPIO_SD_B1_05
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_05, FLEXSPI_A_DQS,    (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DQS    on GPIO3-05 - alt function 1
        #endif
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_SS0_B,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO3-06 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO3-07 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO3-08 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_09, FLEXSPI_A_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA01 on GPIO3-09 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO3-10 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_11, FLEXSPI_A_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA03 on GPIO3-11 - alt function 1
    #elif defined MIMXRT1010
        #if !defined APPLICATION_REQUIRES_GPIO_SD_12
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_12, FLEXSPI_A_DQS,    (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DQS    on GPIO2-12 - alt function 0
        #endif
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_11, FLEXSPI_A_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA03 on GPIO2-11 - alt function 0
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_10, FLEXSPI_A_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO2-10 - alt function 0
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_09, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO2-09 - alt function 0
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_08, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO2-08 - alt function 0
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_07, FLEXSPI_A_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA01 on GPIO2-07 - alt function 0
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_06, FLEXSPI_A_SS0_B,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO2-06 - alt function 0
    #else
        #if !defined APPLICATION_REQUIRES_GPIO_SD_B1_05
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_05, FLEXSPI_A_DQS,    (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DQS    on GPIO3-25 - alt function 1
        #endif
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_DATA03, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA03 on GPIO3-26 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK,   (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SCLK   on GPIO3-27 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA00 on GPIO3-28 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_09, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO3-29 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA01, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA01 on GPIO3-30 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_11, FLEXSPI_A_SS0_B,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_SS0_B  on GPIO3-31 - alt function 1
    #endif
    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_ARDFEN | FLEX_SPI_MCR0_ATDFEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_SWRESET); // perform a reset of the module
    _WAIT_REGISTER_TRUE(FLEX_SPI_MCR0, FLEX_SPI_MCR0_SWRESET);           // wait for reset to complete

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_MDIS);
    FLEX_SPI_MCR1 = (FLEX_SPI_MCR1_AHBBUSWAIT_MASK | FLEX_SPI_MCR1_SEQWAIT_MASK);
    #if defined MIMXRT1010 || defined MIMXRT1064
    FLEX_SPI_MCR2 = (FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    #else
    FLEX_SPI_MCR2 = (FLEX_SPI_MCR2_SAMEDEVICEEN | FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    #endif
    FLEX_SPI_AHBCR = (FLEX_SPI_AHBCR_READADDROPT | FLEX_SPI_AHBCR_PREFETCHEN | FLEX_SPI_AHBCR_BUFFERABLEEN | FLEX_SPI_AHBCR_CACHABLEEN);
    FLEX_SPI_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_IPTXFCR = 0;                                                // configure IP fifo watermarks
    FLEX_SPI_IPRXFCR = 0;
    FLEX_SPI_FLSHA1CR0 = (0);                                            // reset the flash size on all ports
    FLEX_SPI_FLSHA2CR0 = (0);
    FLEX_SPI_FLSHB1CR0 = (0);
    FLEX_SPI_FLSHB2CR0 = (0);

    while ((FLEX_SPI_STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle

    FLEX_SPI_FLSHA1CR0 = (FLEXSPI_FLASH_SIZE / 1024);                    // flash size in KBytes

    FLEX_SPI_FLSHA1CR1 = (0x20063);                                      // flash parameters
    #if defined MIMXRT1024 || defined MIMXRT1064
    FLEX_SPI_FLSHA1CR2 = (2);                                            // operation items
    #else
    FLEX_SPI_FLSHA1CR2 = (0);                                            // operation items
    #endif
    FLEX_SPI_DLLACR = (FLEX_SPI_DLLnCR_OVRDEN);                          // configure DLL
    FLEX_SPI_FLSHCR4 = FLEX_SPI_FLSHCR4_WMOPT1;                          // write mask configuration
    #if (!defined MIMXRT1010 && defined APPLICATION_REQUIRES_GPIO_SD_B1_05) || (defined MIMXRT1010 && defined APPLICATION_REQUIRES_GPIO_SD_12)
    FLEX_SPI_MCR0 = FLEX_SPI_MCR0_RESERVED_ONE | (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK); // exit stop mode (max. 60MHz)
    #else
    FLEX_SPI_MCR0 = FLEX_SPI_MCR0_RESERVED_ONE | (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_RXCLKSRC_DQS_LOOPBACK); // exit stop mode (full-speed)
    #endif
#elif defined MIMXRT1050                                                 // test vehicle
    // Initialise pins to the Hyper flash device
    //
    IOMUXC_FLEXSPI_B_DATA3_SELECT_INPUT = IOMUXC_FLEXSPI_B_DATA3_SELECT_INPUT_GPIO_SD_B1_00_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_00, FLEXSPI_B_DATA03,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIB_DATA03 on GPIO3-00 - alt function 1
    IOMUXC_FLEXSPI_B_DATA2_SELECT_INPUT = IOMUXC_FLEXSPI_B_DATA2_SELECT_INPUT_GPIO_SD_B1_01_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_01, FLEXSPI_B_DATA02,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIB_DATA02 on GPIO3-01 - alt function 1
    IOMUXC_FLEXSPI_B_DATA1_SELECT_INPUT = IOMUXC_FLEXSPI_B_DATA1_SELECT_INPUT_GPIO_SD_B1_02_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_02, FLEXSPI_B_DATA01,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIB_DATA01 on GPIO3-02 - alt function 1
    IOMUXC_FLEXSPI_B_DATA0_SELECT_INPUT = IOMUXC_FLEXSPI_B_DATA0_SELECT_INPUT_GPIO_SD_B1_03_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_03, FLEXSPI_B_DATA00,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIB_DATA00 on GPIO3-03 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_04, FLEXSPI_B_SCLK,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select  on GPIO3-04 - alt function 1        
    IOMUXC_FLEXSPI_A_DQS_SELECT_INPUT = IOMUXC_FLEXSPI_A_DQS_SELECT_INPUT_GPIO_SD_B1_05_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_05, FLEXSPI_A_DQS,   (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUE |IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_DQS on GPIO3-05 - alt function 1
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_06, FLEXSPI_A_SS0_B, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_SS0_B on GPIO3-06 - alt function 1
    IOMUXC_FLEXSPI_A_SCK_SELECT_INPUT = IOMUXC_FLEXSPI_A_SCK_SELECT_INPUT_GPIO_SD_B1_07_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_07, FLEXSPI_A_SCLK,  (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_SCLK on GPIO3-07 - alt function 1
    IOMUXC_FLEXSPI_A_DATA0_SELECT_INPUT = IOMUXC_FLEXSPI_A_DATA0_SELECT_INPUT_GPIO_SD_B1_08_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_08, FLEXSPI_A_DATA00,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_DATA00 on GPIO3-08 - alt function 1
    IOMUXC_FLEXSPI_A_DATA1_SELECT_INPUT = IOMUXC_FLEXSPI_A_DATA1_SELECT_INPUT_GPIO_SD_B1_09_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_09, FLEXSPI_A_DATA01,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_DATA01 on GPIO3-09 - alt function 1
    IOMUXC_FLEXSPI_A_DATA2_SELECT_INPUT = IOMUXC_FLEXSPI_A_DATA2_SELECT_INPUT_GPIO_SD_B1_10_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA02,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_DATA02 on GPIO3-10 - alt function 1
    IOMUXC_FLEXSPI_A_DATA3_SELECT_INPUT = IOMUXC_FLEXSPI_A_DATA3_SELECT_INPUT_GPIO_SD_B1_11_ALT1;
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_11, FLEXSPI_A_DATA03,(IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPIA_DATA03 on GPIO3-11 - alt function 1

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_COMBINATION_EN | FLEX_SPI_MCR0_RXCLKSRC_STROBE | FLEX_SPI_MCR0_SWRESET); // perform a reset of the module
    _WAIT_REGISTER_TRUE(FLEX_SPI_MCR0, FLEX_SPI_MCR0_SWRESET);           // wait for reset to complete

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_COMBINATION_EN | FLEX_SPI_MCR0_RXCLKSRC_STROBE | FLEX_SPI_MCR0_DOZEEN | FLEX_SPI_MCR0_MDIS); // disable for the following configuration
    FLEX_SPI_MCR1 = (FLEX_SPI_MCR1_AHBBUSWAIT_MASK | FLEX_SPI_MCR1_SEQWAIT_MASK);
    FLEX_SPI_MCR2 = (FLEX_SPI_MCR2_SCKBDIFFOPT | FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    FLEX_SPI_AHBCR = (FLEX_SPI_AHBCR_READADDROPT | FLEX_SPI_AHBCR_PREFETCHEN | FLEX_SPI_AHBCR_BUFFERABLEEN | FLEX_SPI_AHBCR_CACHABLEEN);
    FLEX_SPI_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_IPTXFCR = 0;                                                // configure IP fifo watermarks
    FLEX_SPI_IPRXFCR = 0;
    FLEX_SPI_FLSHA1CR0 = (0);                                            // reset the flash size on all ports
    FLEX_SPI_FLSHA2CR0 = (0);
    FLEX_SPI_FLSHB1CR0 = (0);
    FLEX_SPI_FLSHB2CR0 = (0);

    while ((FLEX_SPI_STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle

    FLEX_SPI_FLSHA1CR0 = (FLEXSPI_FLASH_SIZE / 1024);                    // flash size in KBytes

    FLEX_SPI_FLSHA1CR1 = (0x00021c03);                                   // flash parameters (interval 2, column address size 3, word addressable, hold time 0, chip select setup 3.5 serial root clock cycles)
    FLEX_SPI_FLSHA1CR2 = (0x00140100);                                   // operation items (read triggered command number 0, read index 0) (write triggered command number 0, write index 1)
    FLEX_SPI_DLLACR    = (0x00001d00);                                   // configure DLL
    FLEX_SPI_FLSHCR4   = FLEX_SPI_FLSHCR4_WMOPT1;                        // write mask configuration

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_COMBINATION_EN | FLEX_SPI_MCR0_RXCLKSRC_STROBE | FLEX_SPI_MCR0_DOZEEN); // exit stop mode
#endif
#if defined ERR011377
    // After configuring DLL and the lock status bit is set, the data may be wrong if read/write
    // immediately from FLEXSPI based external flash due to timing issue
    // Workaround: Add delay time (100 NOP) after the DLL lock status is set
    //
    if ((FLEX_SPI_DLLACR & FLEX_SPI_DLLnCR_DLLEN) != 0) {
        volatile int i= 0;
        // Wait until sample clocks have locked
        //
        while ((FLEX_SPI_STS2 & (FLEX_SPI_STS2_ASLVLOCK | FLEX_SPI_STS2_AREFLOCK | FLEX_SPI_STS2_BSLVLOCK | FLEX_SPI_STS2_BREFLOCK)) != (FLEX_SPI_STS2_ASLVLOCK | FLEX_SPI_STS2_AREFLOCK | FLEX_SPI_STS2_BSLVLOCK | FLEX_SPI_STS2_BREFLOCK)) {}
        // Add additional (at least) 100 NOP delay
        //
        while (i++ < (100 * 2)) {
            i++;
        }
    }
#endif
#if defined _iMX && (defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART)
    _CONFIG_PERIPHERAL_LOOPBACK(GPIO_SD_B1_10, FLEXSPI_A_DATA02, (IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_6 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select FLEXSPI_A_DATA02 on GPIO3-10 - alt function 1
#endif
}

static void fnEnterLUT(const FLEXSPI_LUT_ENTRY *ptrTable, int iBus)
{
    int iLUT = 0;
    volatile unsigned long *ptrLUT;
    iMX_FLEX_SPI *ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI_BLOCK;
#if FLEX_SPI_INTERFACES > 1
    if (iBus != 0) {
        ptrFlexSPI = (iMX_FLEX_SPI *)FLEX_SPI2_BLOCK;
    }
#endif
#if defined _WINDOWS
    ptrFlexSPI_LUT[iBus] = ptrTable;
#endif
    // Update look-up table
    //
    while ((ptrFlexSPI->STS0 & (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) != (FLEX_SPI_STS0_ARBIDLE | FLEX_SPI_STS0_SEQIDLE)) {} // wait until the bus is idle

    ptrFlexSPI->LUTKEY = FLEX_SPI_LUTKEY_KEY;
    ptrFlexSPI->LUTCR = FLEX_SPI_LUTCR_UNLOCK;
    uMemset((void *)&ptrFlexSPI->LUT0, 0x00, (FLEX_SPI_LUT_ENTRIES * sizeof(unsigned long))); // ensure all non-used entries are clear
    while (ptrTable[iLUT].ulReference != NOR_CMD_LUT_SEQ_IDX_END_OF_TABLE) {
        ptrLUT = &ptrFlexSPI->LUT0;
        ptrLUT += ((ptrTable[iLUT].ulReference & 0x0f) * 4);             // move to the LUT entry
#if defined _WINDOWS
        if (*ptrLUT != 0) {
            _EXCEPTION("Overlapping LUT indexes being used!!!!");
        }
#endif
        *ptrLUT++ = ptrTable[iLUT].ulEntrySequence[0];                   // enter the sequence(s)
        *ptrLUT = ptrTable[iLUT].ulEntrySequence[1];
#if defined BOOT_HYPER_FLASH
        *(++ptrLUT) = ptrTable[iLUT].ulEntrySequence[2];
        *(++ptrLUT) = ptrTable[iLUT].ulEntrySequence[3];
#endif
        iLUT++;
    }

    ptrFlexSPI->LUTKEY = FLEX_SPI_LUTKEY_KEY;
    ptrFlexSPI->LUTCR = FLEX_SPI_LUTCR_LOCK;

    ptrFlexSPI->MCR0 |= FLEX_SPI_MCR0_SWRESET;                           // perform a reset of the module
    _WAIT_REGISTER_TRUE(ptrFlexSPI->MCR0, FLEX_SPI_MCR0_SWRESET);        // wait for reset to complete
}

#if !defined iMX_BOOTLOADER
extern void fnDisplaySPI(void)
{
    int iFlashType = fnSPI_Flash_available();
    fnDebugMsg("SPI Flash: ");
    switch (iFlashType) {
    #if defined SPI_FLASH_AT25SF
    case AT25SF128A:
        fnDebugMsg("adesto 16MByte AT25SF128A");
        break;
    #endif
    #if defined SPI_FLASH_IS25
    case IS25LP_WP_32:
        fnDebugMsg("ISSI 4MByte IS25LP032A");                            // 32MBit / 4MegByte - IS25LP032A
        break;
    case IS25LP_WP_64:
        fnDebugMsg("ISSI 8MByte IS25LP032A");                            // 64MBit / 8MegByte - IS25LP064A
        break;
    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    case IS25LP_WP_256D:
        fnDebugMsg("ISSI 32MByte IS25LP256D");                           // 64MBit / 8MegByte - IS25LP064A
        break;
    #endif
    #endif
    #if defined SPI_FLASH_ATXP
    case EcoXiP_ATXP032:                                                 // 32MBit / 4MegByte - ATXP032
        fnDebugMsg("adesto 4MByte EcoXiP ATXP032");
        break;
    case EcoXiP_ATXP064:                                                 // 64MBit / 8MegByte - ATXP064
        fnDebugMsg("adesto 8MByte EcoXiP ATXP064");
        break;
    case EcoXiP_ATXP128:                                                 // 128MBit / 16MegByte - ATXP128
        fnDebugMsg("adesto 16MByte EcoXiP ATXP128");
        break;
    #endif
    #if defined SPI_FLASH_S26KL
    case S26KL512:                                                       // 512MBit / 64MegByte - S26KL512
        fnDebugMsg("Cypress 64MByte S26KL512");
        break;
    #endif
    #if defined SPI_FLASH_W25Q
    case W25Q16:                                                         // 16MBit / 2MegByte - W25Q16
        fnDebugMsg("Winbond 2MByte W25Q16");
        break;
    case W25Q32:                                                         // 32MBit / 4MegByte - W25Q32
        fnDebugMsg("Winbond 4MByte W25Q32");
        break;
    case W25Q64:                                                         // 64MBit / 8MegByte - W25Q64
        fnDebugMsg("Winbond 8MByte W25Q64");
        break;
    #endif
    default:
        fnDebugMsg("NOT FOUND!!");
        break;
    }
    fnDebugMsg("\r\n");
}
#endif


#if defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK
// This command is used to erase the complete FLASH contents, including the security area
// Since the complete FLASH will be empty the code will no longer run afterwards
//
extern int fnMassEraseFlash(void)
{
    return 0;
}
#endif


#if defined FLASH_ROUTINES || defined ACTIVE_FILE_SYSTEM || defined USE_PARAMETER_BLOCK
    #if defined SPI_EEPROM_ENABLED
// This routine reads data from the defined device into a buffer. The access details inform of the length to be read (already limited to maximum possible length for the device)
// as well as the address in the specific device
//
static void fnReadSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer)
{
    fnSPI_EEPROM_command(EEPROM_READ_DATA_BYTES, ptrAccessDetails->ulOffset, _EXTENDED_CS ptrBuffer, ptrAccessDetails->BlockLength);
}

// This routine writes data from a buffer to an area in SPI Flash (the caller has already defined the data to a particular area and device)
//
static void fnWriteSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer)
{
    MAX_FILE_LENGTH Length = ptrAccessDetails->BlockLength;
    unsigned long Destination = ptrAccessDetails->ulOffset;
    unsigned short usDataLength;
    unsigned short usPageNumber = (unsigned short)(Destination/SPI_FLASH_PAGE_LENGTH); // the page the address is in
    unsigned short usPageOffset = (unsigned short)(Destination - (usPageNumber * SPI_FLASH_PAGE_LENGTH)); // offset in the page
    while (Length != 0) {
        usDataLength = (unsigned short)Length;
        if (usDataLength > (SPI_EEPROM_PAGE_LENGTH - usPageOffset)) {
            usDataLength = (SPI_EEPROM_PAGE_LENGTH - usPageOffset);
        }
        fnSPI_EEPROM_command(EEPROM_WRITE_ENABLE, 0, _EXTENDED_CS 0, 0); // write enable
        fnSPI_EEPROM_command(PAGE_PROG, ((usPageNumber * SPI_EEPROM_PAGE_LENGTH)) | usPageOffset, _EXTENDED_CS ptrBuffer, usDataLength); // copy new content
        Length -= usDataLength;
        ptrBuffer += usDataLength;
        usPageNumber++;
        usPageOffset = 0;
    }
    fnSPI_EEPROM_command(EEPROM_WRITE_DISABLE, 0, _EXTENDED_CS 0, 0); // write disable (this will generally not be necessary since it is automatically set after each successful write operation=
}

    #endif

    #if defined SPI_FLASH_ENABLED && !defined iMX_BOOTLOADER             // if SPI memory driver required
// Include generic SPI flash routines
// static void fnReadSPI(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static void fnWriteSPI(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static MAX_FILE_LENGTH fnDeleteSPI(ACCESS_DETAILS *ptrAccessDetails);
// static void fnReadSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static void fnWriteSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
//
        #include "../SPI_Memory/spi_flash_interface.h"
    #endif

    #if !defined ONLY_INTERNAL_FLASH_STORAGE
// Search for the memory type that the starting address is in, return the type and restrict the largest length that can be read,written, erased from that location
//
extern unsigned char fnGetStorageType(unsigned char *memory_pointer, ACCESS_DETAILS *ptrAccessDetails)
{
    unsigned long ulMaximumLength;
    STORAGE_AREA_ENTRY *ptrStorageList = UserStorageListPtr;
    #if defined _WINDOWS                                                 // this is only used when reading with the simulator
    if (iFetchingInternalMemory != _ACCESS_NOT_IN_CODE) {                // if the access is to program memory when simulating
        iFetchingInternalMemory = _ACCESS_NOT_IN_CODE;                   // reset single-shot program access
        return _STORAGE_PROGRAM;
    }
    #endif
    while (ptrStorageList != 0) {                                        // while there are storage areas defined
        if ((memory_pointer >= ptrStorageList->ptrMemoryStart) && (memory_pointer <= ptrStorageList->ptrMemoryEnd)) { // determine in which memory the access starts in
            if (ptrAccessDetails == 0) {
                return ptrStorageList->ucStorageType;
            }
            ptrAccessDetails->ucDeviceNumber = 0;
            if (ptrStorageList->ucDeviceCount > 1) {                     // if storage area is made up of multiple devices (assumed to be all the same size)
                unsigned long ulDeviceRangeLength = ((unsigned long)((ptrStorageList->ptrMemoryEnd - ptrStorageList->ptrMemoryStart) + 1)/ptrStorageList->ucDeviceCount);
                unsigned char *ptrStart = ptrStorageList->ptrMemoryStart;
                unsigned char *ptrEnd = (ptrStorageList->ptrMemoryStart + ulDeviceRangeLength);                
                while (ptrEnd <= memory_pointer) {
                    ptrStart += ulDeviceRangeLength;
                    ptrEnd += ulDeviceRangeLength;
                    ptrAccessDetails->ucDeviceNumber++;                  // the device number that the access is in
                }
                ulMaximumLength = (unsigned long)(ptrEnd - memory_pointer);
                ptrAccessDetails->ulOffset = (unsigned long)(memory_pointer - ptrStart); // offset from start of memory
            }
            else {
                ulMaximumLength = (unsigned long)(ptrStorageList->ptrMemoryEnd - memory_pointer);
                ulMaximumLength++;                                       // compensate the fact that the end pointer is to the end of the last used address (see reason in STORAGE_AREA_ENTRY definition)
                ptrAccessDetails->ulOffset = (unsigned long)(memory_pointer - ptrStorageList->ptrMemoryStart); // offset from start of memory
            }
            if (ulMaximumLength < ptrAccessDetails->BlockLength) {
                ptrAccessDetails->BlockLength = (MAX_FILE_LENGTH)ulMaximumLength; // limit the length to the present storage device
            }
    #if defined _iMX && (defined MIMXRT1024 || defined MIMXRT1064) && defined USE_EXTERNAL_QSPI_FLASH // special case!!
            if (ptrStorageList->ptrMemoryStart == (unsigned char *)(FLEXSPI2_FLASH_BASE)) {
                if (ptrStorageList->ptrMemoryEnd == (unsigned char *)(FLEXSPI2_FLASH_BASE + (FLEXSPI2_FLASH_SIZE - 1))) {
                    ptrAccessDetails->ucDeviceNumber = 1;
                }
            }
    #endif
            return ptrStorageList->ucStorageType;                        // the storage area type
        }
        ptrStorageList = (STORAGE_AREA_ENTRY *)(ptrStorageList->ptrNext);// move to next storage area
    };
    _EXCEPTION("Attempted access to unsupported storage area!!!!");
    return _STORAGE_INVALID;                                             // attempted access outside of the defined storage areas
}
    #endif

// Erase FLASH sector(s). The pointer can be anywhere in the sector to be erased.
// If the length signifies multiple sectors, each one necessary is erased.
//
extern int fnEraseFlashSector(unsigned char *ptrSector, MAX_FILE_LENGTH Length)
{
#if (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0))
    unsigned short _FLASH_GRANULARITY = FLASH_GRANULARITY;               // assume program flash granularity
#else
    #define _FLASH_GRANULARITY FLASH_GRANULARITY
#endif
    #if defined MANAGED_FILES
    MAX_FILE_LENGTH OriginalLength = Length;
    #endif
    #if !defined ONLY_INTERNAL_FLASH_STORAGE
    ACCESS_DETAILS AccessDetails;
    do {
        AccessDetails.BlockLength = Length;
        switch (fnGetStorageType(ptrSector, &AccessDetails)) {           // get the storage type based on the memory location and also return the largest amount of data that can be read from a single device
        case _STORAGE_INTERNAL_FLASH:
            _EXCEPTION("iMX RT has no internal Flash!!");
            break;
        #if defined SPI_FLASH_ENABLED
        case _STORAGE_SPI_FLASH:
            {
                MAX_FILE_LENGTH PageBoundaryOffset = (MAX_FILE_LENGTH)(((CAST_POINTER_ARITHMETIC)(AccessDetails.ulOffset))%SPI_FLASH_PAGE_LENGTH);
                Length += PageBoundaryOffset;                            // include length back to start of page
                ptrSector -= PageBoundaryOffset;                         // set to page boundary
                AccessDetails.BlockLength = fnDeleteSPI(&AccessDetails); // delete page/block in SPI flash
            }
            break;
        #endif
        #if defined SPI_EEPROM_ENABLED
        case _STORAGE_SPI_EEPROM:
        {
            unsigned char ucErased[SPI_EEPROM_PAGE_LENGTH];
            MAX_FILE_LENGTH PageBoundaryOffset = (MAX_FILE_LENGTH)(((CAST_POINTER_ARITHMETIC)(AccessDetails.ulOffset))%SPI_EEPROM_PAGE_LENGTH);
            Length += PageBoundaryOffset;                                // include length back to start of page
            ptrSector -= PageBoundaryOffset;                             // set to page boundary
            uMemset(ucErased, 0xff, SPI_EEPROM_PAGE_LENGTH);             // buffer with 0xff
            fnWriteSPI(&AccessDetails, ucErased);                        // delete page in SPI eeprom (by programming to 0xff)
            AccessDetails.BlockLength = SPI_EEPROM_PAGE_LENGTH;
        }
            break;
        #endif
        #if defined I2C_EEPROM_FILE_SYSTEM
        case _STORAGE_I2C_EEPROM:
            {
                CAST_POINTER_ARITHMETIC align_length = (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXTENSION_FILE_SIZE - 1)));
                Length += align_length;
                AccessDetails.BlockLength += align_length;
                ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXTENSION_FILE_SIZE - 1)); // set to sector boundary
                if (fnDeleteI2C_eeprom(&AccessDetails) < 0) {            // delete data in I2C EEPROM
                    return -1;
                }
            }
            break;
        #endif
        #if defined EXT_FLASH_FILE_SYSTEM
        case _STORAGE_PARALLEL_FLASH:
            Length += (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXT_FLASH_PAGE_LENGTH - 1)));
            ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXT_FLASH_PAGE_LENGTH - 1)); // set to sector boundary
            AccessDetails.BlockLength = fnDeleteParallelFlash(&AccessDetails); // delete page/block in parallel flash 
            if (AccessDetails.BlockLength == 0) {
                return -1;                                               // error
            }
            break;
        #endif
        default:
            _EXCEPTION("Invalid storage type");
            return -1;                                                   // invalid
        }
        ptrSector += AccessDetails.BlockLength;
        if (Length <= AccessDetails.BlockLength) {                       // if completed
        #if defined MANAGED_FILES
            if (OriginalLength == 0) {                                   // if a single page erase was called, return the page size of the storage area
	            return AccessDetails.BlockLength;
	        }
        #endif
            break;
        }
        Length -= AccessDetails.BlockLength;
    } FOREVER_LOOP();
    #else                                                                // case when only internal Flash is available
    _EXCEPTION("iMX RT has no internal Flash!!");
    #endif
    return 0;                                                            // success
}


extern int fnWriteBytesFlash(unsigned char *ucDestination, unsigned char *ucData, MAX_FILE_LENGTH Length)
{
    #if !defined ONLY_INTERNAL_FLASH_STORAGE
    ACCESS_DETAILS AccessDetails;
    AccessDetails.BlockLength = Length;
    FOREVER_LOOP() {                                                     // allow zero length write to ensure that open flash buffer can be closed
        switch (fnGetStorageType(ucDestination, &AccessDetails)) {       // get the storage type based on the memory location and also return the largest amount of data that can be read written to a single device
        case _STORAGE_INTERNAL_FLASH:
            _EXCEPTION("iMX RT has no internal flash!");
            break;
        #if defined SPI_FLASH_ENABLED
        case _STORAGE_SPI_FLASH:
            fnWriteSPI(&AccessDetails, ucData);
            break;
        #endif
        #if defined SPI_EEPROM_ENABLED
        case _STORAGE_SPI_EEPROM:
            fnWriteSPI_EEPROM(&AccessDetails, ucData);
            break;
        #endif
        #if defined I2C_EEPROM_FILE_SYSTEM
        case _STORAGE_I2C_EEPROM:
            fnWriteI2C_eeprom(&AccessDetails, ucData);
            break;
        #endif
        #if defined EXT_FLASH_FILE_SYSTEM
        case _STORAGE_PARALLEL_FLASH:
            if (fnWriteParallelFlash(&AccessDetails, ucData) != 0) {
                return 1;                                                // error
            }
            break;
        #endif
        default:
            _EXCEPTION("Invalid storage type");
            return 1;                                                    // invalid
        }
        if (Length == AccessDetails.BlockLength) {                       // if write is complete
            return 0;
        }
        Length -= AccessDetails.BlockLength;                             // decrease the amount of data remaining
        ucDestination += AccessDetails.BlockLength;                      // increase the destination address accordingly
        ucData += AccessDetails.BlockLength;                             // increase the buffer address accordingly
        AccessDetails.BlockLength = Length;                              // synchronise individual accesses to remaining length
    }
  //return 0;
    #else
    return 0;
    #endif
}

extern void fnGetParsFile(unsigned char *ParLocation, unsigned char *ptrValue, MAX_FILE_LENGTH Size)
{
    #if defined _WINDOWS
    if (iFetchingInternalMemory != _ACCESS_NOT_IN_CODE) {
        uMemcpy(ptrValue, ParLocation, Size);
        return;
    }
    #endif
    #if !defined NO_FLASH_SUPPORT
        #if !defined ONLY_INTERNAL_FLASH_STORAGE
    {
        ACCESS_DETAILS AccessDetails;
        while (Size != 0) {
            AccessDetails.BlockLength = Size;                            // synchronise individual accesses to remaining length
            switch (fnGetStorageType(ParLocation, &AccessDetails)) {     // get the storage type based on the memory location and also return the largest amount of data that can be read from a single device
        #if !defined _WINDOWS
            case _STORAGE_INVALID:                                       // read from unknown area is allowed on the HW so that SRAM (for example) could be read without explicity declaring it
        #endif
            case _STORAGE_INTERNAL_FLASH:
        #if defined _WINDOWS && (defined MIMXRT1024 || defined MIMXRT1064)
                ParLocation -= 0x10000000;                               // temporarily take the content from external QSPI flash
        #endif
                uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), AccessDetails.BlockLength); // read from QSPI FLASH with no access restrictions so we can simply copy the data
                break;
                #if defined SPI_FLASH_ENABLED
            case _STORAGE_SPI_FLASH:
        #if defined iMX_BOOTLOADER                                       // the "Bare-Minimum" boot loader works in XiP mode and can read the QSPI flash diectly
                uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), AccessDetails.BlockLength); // read from QSPI FLASH with no access restrictions so we can simply copy the data
        #else
                fnReadSPI(&AccessDetails, ptrValue);                     // read from the SPI device
        #endif
                break;
                #endif
                #if defined SPI_EEPROM_ENABLED
            case _STORAGE_SPI_EEPROM:
                fnReadSPI_EEPROM(&AccessDetails, ptrValue);              // read from the SPI eeprom device
                break;
                #endif
                #if defined I2C_EEPROM_FILE_SYSTEM
            case _STORAGE_I2C_EEPROM:
                fnReadI2C_eeprom(&AccessDetails, ptrValue);              // read the data from the external device
                break;
                #endif
                #if defined EXT_FLASH_FILE_SYSTEM
            case _STORAGE_PARALLEL_FLASH:
                uMemcpy(ptrValue, fnGetExtFlashAdd(ParLocation), AccessDetails.BlockLength); // read from external parallel FLASH with no access restrictions so we can simply copy the data
                break;
                #endif
            case _STORAGE_PROGRAM:                                       // internal and directly accessible
                uMemcpy(ptrValue, ParLocation, Size);                            
                return;
            default:
                _EXCEPTION("Invalid storage type");
                return;                                                  // invalid
            }
            if (Size == AccessDetails.BlockLength) {                     // if read is complete
                return;
            }
            Size -= AccessDetails.BlockLength;                           // decrease the amount of data remaining
            ParLocation += AccessDetails.BlockLength;                    // increase the destination address accordingly
            ptrValue += AccessDetails.BlockLength;                       // increase the buffer address accordingly
        }
    }
        #else
            #if defined FLEXFLASH_DATA                                   // {82} if working with FlashNMV in data flash mode
    if (ParLocation >= (unsigned char *)(SIZE_OF_FLASH - SIZE_OF_FLEXFLASH)) { // is the access a read from FlexNVM
        ParLocation += (FLEXNVM_START_ADDRESS - (SIZE_OF_FLASH - SIZE_OF_FLEXFLASH)); // convert to FlexNVM physical address
    }
            #elif (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
    if (ParLocation >= (unsigned char *)(SIZE_OF_FLASH)) { // is the access a read from EEPROM
        ParLocation += (KE_EEPROM_START_ADDRESS - (SIZE_OF_FLASH)); // convert to EEPROM physical address
    }
            #endif
    uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), Size);                 // directly copy memory since this must be a pointer to code (embedded file)
        #endif
    #endif
}
#endif


#if defined USE_PARAMETER_BLOCK

#define PARAMETER_BLOCK_1 (unsigned char *)(PARAMETER_BLOCK_START)
#define PARAMETER_BLOCK_2 (unsigned char *)(PARAMETER_BLOCK_START + PARAMETER_BLOCK_SIZE)


#define PARAMETER_STATUS_SIZE   2


extern unsigned char fnGetValidPars(unsigned char ucValid)
{
    unsigned char ucValidUse[PARAMETER_STATUS_SIZE];
    unsigned char ucCompare;

    fnGetParsFile((unsigned char *)PARAMETER_BLOCK_1, (unsigned char *)ucValidUse, sizeof(ucValidUse)); // the first 2 long word locations (4 with fast devices) define the validity of the block. 0x55 0x55 means it is fully valid. 0x55 0xff means it contains temporary unvalidated data

    if (PRESENT_TEMP_BLOCK != ucValid) {
        ucCompare = 0x55;                                                // we are looking for validated pattern
    }
    else {
        ucCompare = 0xff;                                                // we are looking for temporary pattern
    }

    if ((ucValidUse[0] == 0x55) && (ucValidUse[PARAMETER_STATUS_SIZE/2] == ucCompare)) {
        return 1;                                                        // first block matched request
    }
    #if defined USE_PAR_SWAP_BLOCK
    else {
        fnGetParsFile((unsigned char *)PARAMETER_BLOCK_2, (unsigned char *)ucValidUse, sizeof(ucValidUse));
        if ((ucValidUse[0] == 0x55) && (ucValidUse[PARAMETER_STATUS_SIZE/2] == ucCompare)) {
            return 2;                                                    // second block matched request
        }
    }
    #endif
    return 0;                                                            // all other cases indicate the searched block doesn't exist
}

// Retrieve parameter from the defined block
//
extern int fnGetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned char *ptrPar = PARAMETER_BLOCK_1;
    #if defined USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar += (PARAMETER_BLOCK_2 - PARAMETER_BLOCK_1);
    }
    #endif

    ptrPar += PARAMETER_STATUS_SIZE;                                     // first parameter starts after validation information
    #if defined PARAMETER_NO_ALIGNMENT
    ptrPar = (unsigned long *)((unsigned char *)ptrPar + usParameterReference); // move to the location of the parameter using a pointer to bytes
    fnGetParsFile((unsigned char *)ptrPar, ucValue, usLength);
    #else
    ptrPar += usParameterReference;                                      // move to the long word location where the parameter value is stored

    fnGetParsFile((unsigned char *)ptrPar, ucValue, usLength);           // retrieve the data
    #endif
    return 0;
}


// Delete all sectors contained in a parameter block
//
static int fnDeleteParBlock(unsigned char *ptrAddInPar)
{
    return (fnEraseFlashSector(ptrAddInPar, PARAMETER_BLOCK_SIZE));
}


// Write parameters to the defined block
//
extern int fnSetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned char *ptrPar = PARAMETER_BLOCK_1, *ptrStart;
    unsigned char ucValid = 0x55;
    int iBlockUse = 0;
    unsigned char ucValidCheck;

    if ((TEMP_PARS & ucValidBlock) != 0) {
        ucValidBlock &= ~TEMP_PARS;
        iBlockUse = 1;
    }

    if ((ucValidBlock & BLOCK_INVALID) != 0) {                           // no valid parameter blocks have been found so we can use the first for saving the data to
        fnDeleteParBlock((unsigned char *)PARAMETER_BLOCK_1);            // we delete the parameter block to be sure it is fresh on first use
    #if defined USE_PAR_SWAP_BLOCK
        fnDeleteParBlock((unsigned char *)PARAMETER_BLOCK_2);            // same for swap block
    #endif
        ucValidBlock = 1;
    }

    #if defined USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar = PARAMETER_BLOCK_2;
    }
    #endif
    ptrStart = ptrPar;
    ptrPar += PARAMETER_STATUS_SIZE;                                     // first parameter starts after validation information
    #if defined PARAMETER_NO_ALIGNMENT
    ptrPar = (unsigned long *)((unsigned char *)ptrPar + usParameterReference); // move to the location of the parameter using a pointer to bytes
    #else
    ptrPar += usParameterReference;                                      // move to the location where the parameter value is stored
    #endif

    if (usLength == 0) {                                                 // command rather than normal write
        if (iBlockUse != 0) {                                            // if we have a new set waiting, set it to use state
            iBlockUse = 0;
    #if defined USE_PAR_SWAP_BLOCK
            if (ptrStart == PARAMETER_BLOCK_1) {
                ptrPar = PARAMETER_BLOCK_2;
            }
            else {
                ptrPar = PARAMETER_BLOCK_1;
            }
            fnDeleteParBlock((unsigned char *)ptrPar);
    #endif
        }
        else {                                                           // we must delete the block
            fnDeleteParBlock((unsigned char *)ptrStart);
            if (usParameterReference != 0) {
                return 0;                                                // we don't validate it again
            }
        }
    }
    else {
    #if defined PARAMETER_NO_ALIGNMENT
        #if defined _WINDOWS                                             // when simulating the write size to internal flash is checked since each write is expected to be a multiple of the flash write size
        if (ptrPar < (unsigned long *)SIZE_OF_FLASH) {                   // parameter memory is in the internal flash
            if ((usLength % FLASH_ROW_SIZE) != 0) {                      // check write block size which must always be a multiple of the flash write size
                _EXCEPTION("When using the option PARAMETER_NO_ALIGNMENT all parameter block writes must be of the correct modularity size");
            }
            if (((CAST_POINTER_ARITHMETIC)ptrPar & (FLASH_ROW_SIZE - 1)) != 0) { // check the alignment of the write
                _EXCEPTION("When using the option PARAMETER_NO_ALIGNMENT all parameter block writes must be aligned to a flash long word/phrase");
            }
        }
        #endif
        fnWriteBytesFlash((unsigned char *)ptrPar, ucValue, usLength);   // it is expected that the user writes blocks of parameter data that is aligned and packed to respect the characteristics of the flash memory being used
    #else
        fnWriteBytesFlash((unsigned char *)ptrPar, ucValue, usLength); // write all parameters to FLASH
    #endif
    }
    fnGetParsFile((unsigned char *)ptrStart, (unsigned char *)&ucValidCheck, sizeof(ucValidCheck));
    if (ucValidCheck != ucValid) {
        ucValidCheck = ucValid;
        fnWriteBytesFlash((unsigned char *)ptrStart, (unsigned char *)&ucValidCheck, sizeof(ucValidCheck)); // validate data
    }
    if (iBlockUse == 0) {
        fnGetParsFile((unsigned char *)(ptrStart + (PARAMETER_STATUS_SIZE/2)), (unsigned char *)&ucValidCheck, sizeof(ucValidCheck));
        if (ucValidCheck != ucValid) {
            ucValidCheck = ucValid;
            fnWriteBytesFlash((unsigned char *)(ptrStart + (PARAMETER_STATUS_SIZE/2)), (unsigned char *)&ucValidCheck, sizeof(ucValidCheck)); // use data
        }
    }
    return 0;
}
#endif

#if defined SUPPORT_OCOTP
#define OCOTP_TIMING_WAIT_NS        150                                  // wait time should be not less than 150ns
#define OCOTP_TIMING_RELEX_NS       100                                  // relax time should be not less than 100ns
#define OCOTP_TIMING_PROGRAM_NS     10000                                // program time should be range 9000ns .. 11000ns
#define OCOTP_TIMING_READ_NS        40                                   // read time should be less than 40ns

// Wait time should be higher than OCOTP_TIMING_WAIT_NS
//
#define OCOTP_WAIT_TIMING     ((((OCOTP_TIMING_WAIT_NS * (IPG_CLK_ROOT_FREQUENCY/1000000) + 1000) / 1000 - 1) << HW_OCOTP_TIMING_WAIT_SHIFT) & HW_OCOTP_TIMING_WAIT_MASK)

// Relax time should be higher than OCOTP_TIMING_RELEX_NS
//
#define OCOTP_RELAX_TIMING    ((((OCOTP_TIMING_RELEX_NS * (IPG_CLK_ROOT_FREQUENCY/1000000) + 1000) / 1000 - 1) << HW_OCOTP_TIMING_RELAX_SHIFT) & HW_OCOTP_TIMING_RELAX_MASK)

// Strobe_prog time should be close to OCOTP_TIMING_PROGRAM_NS, only add half of 1000
//
#define OCOTP_STROBE_PROGRAM_TIMING   (((OCOTP_TIMING_PROGRAM_NS * (IPG_CLK_ROOT_FREQUENCY/1000000) + 500) / 1000 + 2 * ((OCOTP_RELAX_TIMING >> HW_OCOTP_TIMING_RELAX_SHIFT) + 1) - 1) & HW_OCOTP_TIMING_STROBE_PROG_MASK)

// Strobe_read time should be higher than OCOTP_TIMING_READ_NS
//
#define OCOTP_STROBE_READ_TIMING ((((OCOTP_TIMING_READ_NS * (IPG_CLK_ROOT_FREQUENCY/1000000) + 1000) / 1000 + 2 * ((OCOTP_RELAX_TIMING >> HW_OCOTP_TIMING_RELAX_SHIFT) + 1) - 1) << HW_OCOTP_TIMING_STROBE_READ_SHIFT) & HW_OCOTP_TIMING_STROBE_READ_MASK)

static void fnPrepareOTP_operation(void)
{
    POWER_UP_ATOMIC(2, OCOTP_CTRL_CLOCK);                                // ensure that the module is clocked
    _WAIT_REGISTER_TRUE(HW_OCOTP_CTRL, HW_OCOTP_CTRL_BUSY);              // wait until previous operations have completed
    _CLEAR_REGISTER(HW_OCOTP_CTRL, HW_OCOTP_CTRL_ERROR);                 // ensure any error condition flag is cleared
    HW_OCOTP_TIMING = (OCOTP_WAIT_TIMING | OCOTP_RELAX_TIMING | OCOTP_STROBE_PROGRAM_TIMING | OCOTP_STROBE_READ_TIMING); // set appropriate STROBE_READ and RELAX timings
    _WAIT_REGISTER_TRUE(HW_OCOTP_CTRL, HW_OCOTP_CTRL_BUSY);              // wait until operation has completed
}

static void fnReloadShadowRegisters(void)
{
    fnPrepareOTP_operation();
    _SET_REGISTER(HW_OCOTP_CTRL, HW_OCOTP_CTRL_RELOAD_SHADOWS);          // trigger a reload of the shadow registers
    UPDATE_SHADIW_REGISTERS();                                           // simulate update of shadow registers
    _WAIT_REGISTER_TRUE(HW_OCOTP_CTRL, (HW_OCOTP_CTRL_BUSY | HW_OCOTP_CTRL_RELOAD_SHADOWS)); // wait until operation has completed and the reload flag has been negated
    POWER_DOWN_ATOMIC(2, OCOTP_CTRL_CLOCK);
}

extern void OCOTP_ReadFuseShadowRegister(unsigned long ulAddress, unsigned long *ptr_ulData, int iNumber)
{
    do {
        fnPrepareOTP_operation();
        _CLEAR_REGISTER(HW_OCOTP_CTRL, HW_OCOTP_CTRL_ADDR_MASK);         // ensure address field is empty
        _SET_REGISTER(HW_OCOTP_CTRL, (ulAddress++ & HW_OCOTP_CTRL_ADDR_MASK)); // set register address
        HW_OCOTP_READ_CTRL = HW_OCOTP_READ_CTRL_READ_FUSE;               // initiate OTP read
        _WAIT_REGISTER_TRUE(HW_OCOTP_CTRL, HW_OCOTP_CTRL_BUSY);          // wait until operation has completed
        *ptr_ulData++ = HW_OCOTP_READ_FUSE_DATA;                         // read the value
    } while (iNumber-- > 1);                                             // allow reading multiple contiguous registers
    POWER_DOWN_ATOMIC(2, OCOTP_CTRL_CLOCK);
}

extern int OCOTP_WriteFuseShadowRegister(unsigned long ulAddress, unsigned long *ulData, int iNumber)
{
    CCM_CGPR |= (CCM_CGPR_EFUSE_PROG_SUPPLY_GATE | CCM_CGPR_FIXED_1);    // allow eFUSE programming
    ulAddress -= 0x400;
    ulAddress /= (4 * sizeof(unsigned long));
    if ((ulAddress + (iNumber - 1)) > HW_OCOTP_CTRL_ADDR_MASK) {         // check that a valid programming range is being used
        return -1;
    }
    do {
        fnPrepareOTP_operation();
        _CLEAR_REGISTER(HW_OCOTP_CTRL, (HW_OCOTP_CTRL_ADDR_MASK | HW_OCOTP_CTRL_WR_UNLOCK_MASK)); // ensure address field is empty
        _SET_REGISTER(HW_OCOTP_CTRL, (HW_OCOTP_CTRL_WR_UNLOCK | (ulAddress++ & HW_OCOTP_CTRL_ADDR_MASK))); // set register address and unlock key
        HW_OCOTP_DATA = *ulData++;                                       // write the value to be programmed
        PROGRAM_OTP();                                                   // simulate programming
        _WAIT_REGISTER_TRUE(HW_OCOTP_CTRL, HW_OCOTP_CTRL_BUSY);          // wait until operation has completed
        if ((HW_OCOTP_CTRL & HW_OCOTP_CTRL_ERROR) != 0) {                // write failed
            _CLEAR_REGISTER(HW_OCOTP_CTRL, (HW_OCOTP_CTRL_ERROR));       // clear the error
            CCM_CGPR = ((CCM_CGPR & ~CCM_CGPR_EFUSE_PROG_SUPPLY_GATE) | CCM_CGPR_FIXED_1); // disable eFUSE programming
            POWER_DOWN_ATOMIC(2, OCOTP_CTRL_CLOCK);
            return -1;
        }
    } while (iNumber-- > 1);                                             // allow programming multiple contiguous registers
    CCM_CGPR = ((CCM_CGPR & ~CCM_CGPR_EFUSE_PROG_SUPPLY_GATE) | CCM_CGPR_FIXED_1); // disable eFUSE programming
    fnReloadShadowRegisters();                                           // resynchronise with the shadow registers
    return 0;                                                            // successful
}
#endif
