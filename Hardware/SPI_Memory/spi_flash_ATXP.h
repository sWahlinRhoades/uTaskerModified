/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      spi_flash_ATXP.h - adesto EcoXiP
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    This file contains SPI FLASH specific code for all chips that are supported.
    It is declared as a header so that projects do not need to specify that it is not to be compiled.
    Its goal is to improve overall readability of the hardware interface.

    **********************************************************************/


#if defined SPI_FLASH_ATXP

#if defined _SPI_DEFINES

#if !defined iMX_BOOTLOADER
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    static void fnSPI_command_ATXP(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #else
    static void fnSPI_command_ATXP(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #endif
#endif

#define CHECK_SPI_FLASH_BUSY     0x00                                    // pseudo request to see whether device is ready

#define WRITE_ENABLE             0x06                                    // enable writes
#define WRITE_DISABLE            0x04
#define READ_STATUS_REGISTER_1   0x05                                    // read status reuster byte 1
    #define STATUS_BUSY          0x01                                    // device busy (read-only)
    #define STATUS_WEL           0x02                                    // write enable latch status (read-only)
    #define STATUS_SWP0          0x04                                    // software protection status 0 (read-only)
    #define STATUS_SWP1          0x08                                    // software protection status 1 (read-only)
    #define STATUS_SWP_UNPROTECTED    0x00
    #define STATUS_SWP_SOME_PROTECTED (STATUS_SWP0)
    #define STATUS_SWP_ALL_PROTECTED  (STATUS_SWP1 | STATUS_SWP0)
    #define STATUS_UDPDS         0x10                                    // ultra-deep power-down status (read-only)
    #define STATUS_EPE           0x20                                    // program/erase error (read-only)
    #define STATUS_DPDS          0x40                                    // deep power-down status (read-only)
    #define STATUS_SPRL          0x80                                    // sector protect registers locked
#define READ_STATUS_CONTROL_REGISTERS   0x65
#define WRITE_STATUS_CONTROL_REGISTERS  0x71
#define ENTER_QSP                0x35
#define EXIT_QSP                 0xf5
#define WRITE_STATUS_REGISTER    0x01
#define PAGE_PROG                0x02                                    // byte/page program 1..256 bytes
#define QUAD_PROGRAM_PAGE        0x32
#define SUB_SECTOR_ERASE         0x20                                    // sector erase - 4k (name for compatibility)
#define HALF_BLOCK_ERASE         0x52                                    // half block erase - 32k
#define BLOCK_ERASE              0xd8                                    // block erase - 64k
#define CHIP_ERASE               0x60
#define ERASE_SUSPEND            0x75
#define ERASE_RESUME             0x7a
#define _POWER_DOWN              0xb9
#define _RELEASE_POWER_DOWN      0xab
#define CONT_READ_RESET          0xff
#define MANUFACTURER_ID          0x90
#define MANUFACTURER_ID_DUAL_I_O 0x92
#define MANUFACTURER_ID_QUAD_I_O 0x94
#define READ_JEDEC_ID            0x9f
#define READ_UNIQUE_ID           0x4b
#define READ_DATA_BYTES          0x03
#define FAST_READ                0x0b                                    
#define FAST_READ_DUAL_OUTPUT    0x3b
#define FAST_READ_DUAL_I_O       0xbb
#define FAST_READ_QUAD_OUTPUT    0x6b
#define FAST_READ_QUAD_I_O       0xeb
#define WORD_READ_QUAD_I_O       0xe7
#define OCTAL_WORD_READ_QUAD_I_O 0xe3
#define RETURN_TO_SPI_MODE       0xff
#define _UNPROTECT_SECTOR        0x39


// Number of dummy cycles for SPI/quad/octal modes
//
#define ECOXIP_READ_SPI_DUMMY_CYCLES         8
#define ECOXIP_READ_NON_SPI_DUMMY_CYCLES     22
#define CTRL_REG_BYTE3_VAL                   (((ECOXIP_READ_NON_SPI_DUMMY_CYCLES - 8) >> 1) | 0x10)

#if defined SPI_FLASH_ATXP128
    #define MANUFACTURER_ID_ADESTO   0x1f                                // adesto manufacturer's ID
#else
    #define MANUFACTURER_ID_ADESTO   0x43                                // adesto manufacturer's ID
#endif

// Unique ID's
//
#define DEVICE_ID_1_DATA_ADESTO_FLASH_32M     0xa7                         // 32MBit / 4MegByte - ATXP032
#define DEVICE_ID_1_DATA_ADESTO_FLASH_64M     0xa8                         // 64MBit / 8MegByte - ATXP064
#define DEVICE_ID_1_DATA_ADESTO_FLASH_128M    0xa9                         // 128MBit / 16MegByte - ATXP128

#if !defined iMX_BOOTLOADER
    #if defined _STM32
        #define MANUAL_FLASH_CS_CONTROL
        #define ASSERT_CS_LINE(cs_line)      __ASSERT_CS(cs_line)
        #define NEGATE_CS_LINE(cs_line)      __NEGATE_CS(cs_line)
        #define FLUSH_SPI_FIFO_AND_FLAGS()
        #define WRITE_SPI_CMD0(ucCommand)    SSPDR_X = ucCommand
        #define WRITE_SPI_CMD0_LAST(ucData)  SSPDR_X = ucData
        #define READ_SPI_FLASH_DATA()        (unsigned char)SSPDR_X
        #define CLEAR_RECEPTION_FLAG()
        #if defined _WINDOWS
            #define WAIT_TRANSFER_END()      while ((SSPSR_X & SPISR_TXE) == 0) { SSPSR_X |= SPISR_TXE;} \
                                             while (SSPSR_X & SPISR_BSY) {SSPSR_X &= ~SPISR_BSY;}
        #else
            #define WAIT_TRANSFER_END()      while ((SSPSR_X & SPISR_TXE) == 0) {} \
                                             while (SSPSR_X & SPISR_BSY) {}
        #endif
        #define WAIT_SPI_RECEPTION_END()     WAIT_TRANSFER_END()
        #if !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
    #elif defined _KINETIS
        #if !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
    #elif defined _iMX
        #define SPI_FLASH_START  (FLEXSPI_FLASH_BASE)
        #define FLUSH_SPI_FIFO_AND_FLAGS()
        #define POWER_DOWN_SPI_FLASH_INTERFACE()
    #endif
    #if defined SPI_FLASH_MULTIPLE_CHIPS
        #define __EXTENDED_CS     iChipSelect,
        static unsigned char fnCheckIS25XXX(int iChipSelect);
        static const STORAGE_AREA_ENTRY spi_flash_storage_ATXP = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            SPI_FLASH_DEVICE_COUNT                                       // multiple devices
        };
    #else
        #define __EXTENDED_CS
        static unsigned char fnCheckATXPXXX(void);
        static const STORAGE_AREA_ENTRY spi_flash_storage_ATXP = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            0                                                            // not multiple devices
        };
    #endif
#endif
#endif


    // This code is inserted to detect the presence of the SPI FLASH device(s). If the first device is not detected the SPI interface is disabled.
    // - if there are multiple devices, each will be recorded
    //
#if defined _CHECK_SPI_CHIPS
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    ucSPI_FLASH_Type[0] = fnCheckATXPXXX(0);                             // flag whether the first SPI FLASH device is connected
    #else
    ucSPI_FLASH_Type[0] = fnCheckATXPXXX();                              // flag whether the SPI FLASH device is connected
    #endif
    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    if (ucSPI_FLASH_Type[0] < EcoXiP_ATXP128)
    #elif SPI_FLASH_SIZE >= (8 * 1024 * 1024)
    if (ucSPI_FLASH_Type[0] < EcoXiP_ATXP064)
    #else
    if (ucSPI_FLASH_Type[0] < EcoXiP_ATXP032)
    #endif
    {                                                                    // we expect at least this part to be available
    #if !defined SPI_FLASH_SECOND_SOURCE_MODE
        POWER_DOWN_SPI_FLASH_INTERFACE();                                // power down SPI 
    #endif
    }
    else {
    #if defined SPI_FLASH_MULTIPLE_CHIPS                                 // check for further devices
        int i = 0;
        while (++i < SPI_FLASH_DEVICE_COUNT) {
            ucSPI_FLASH_Type[i] = fnCheckATXPXXX(i);
        }
    #endif
        UserStorageListPtr = (STORAGE_AREA_ENTRY *)&spi_flash_storage_ATXP; // insert spi flash as storage medium
    #if defined SPI_FLASH_SECOND_SOURCE_MODE
        fnSPI_command = fnSPI_command_ATXP;
    #endif
    }
#endif



#if defined _SPI_FLASH_INTERFACE
// This is the main interface code to the ISSI SPI FLASH device
//
/* =================================================================== */
/*                             ATXP driver                             */
/* =================================================================== */

#if defined _iMX
static const FLEXSPI_LUT_ENTRY tableLUT_config[] = {                     // look-up table used only during configuration
    {   // Fast read - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_FAST,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, ECOXIP_READ_SPI_DUMMY_CYCLES) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x80),
        },
    },
    {   // Read status
        NOR_CMD_LUT_SEQ_IDX_READSTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x01),
        0,
        },
    },
    {   // Write enable
        NOR_CMD_LUT_SEQ_IDX_WRITEENABLE,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_ENABLE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },  
    {   // Read status/control register
        NOR_CMD_LUT_SEQ_IDX_READ_STATUS_CONTROL_REGS,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_CONTROL_REGISTERS) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_8BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 8) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x80),
        },
    },
    {   // Write status/control register (this specific sequence will write 2 bytes to status/control regs 2-3)
        NOR_CMD_LUT_SEQ_IDX_WRITE_STATUS_CONTROL_REGS,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_STATUS_CONTROL_REGISTERS) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_SDR, 0x02),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_WRITE_SDR, 0x02) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        },
    },
    {   // Read ID
        NOR_CMD_LUT_SEQ_IDX_READID,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_JEDEC_ID) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x0c),
        0,
        },
    },
    {   // Unprotect sectors
        NOR_CMD_LUT_SEQ_IDX_UNPROTECT_SECTORS,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, _UNPROTECT_SECTOR) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        0,
        },
    },
    {   // Exit octal mode
        NOR_CMD_LUT_SEQ_IDX_EXITQPI,
        {
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_SDR, RETURN_TO_SPI_MODE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // End of LUT
        NOR_CMD_LUT_SEQ_IDX_END_OF_TABLE,
        {
            0,0
        },
    },
};

static const FLEXSPI_LUT_ENTRY tableLUT_main[] = {                       // look-up table used during main operation
    {   // Fast read - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_FAST,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, ECOXIP_READ_SPI_DUMMY_CYCLES) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x80),
        },
    },
    {   // Read status
        NOR_CMD_LUT_SEQ_IDX_READSTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x01),
        0,
        },
    },
    {   // Write enable
        NOR_CMD_LUT_SEQ_IDX_WRITEENABLE,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_ENABLE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // Unprotect sectors
        NOR_CMD_LUT_SEQ_IDX_UNPROTECT_SECTORS,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, _UNPROTECT_SECTOR) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        0,
        },
    },
    {   // Write disable
        NOR_CMD_LUT_SEQ_IDX_WRITEDISABLE,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_DISABLE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // Erase sector - 4k
        NOR_CMD_LUT_SEQ_IDX_ERASESECTOR,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, SUB_SECTOR_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        0,
        },
    },
    {   // Erase half-block - 32k
        NOR_CMD_LUT_SEQ_IDX_ERASE_HALF_BLOCK,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, HALF_BLOCK_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        0,
        },
    },
    {   // Erase block - 64k
        NOR_CMD_LUT_SEQ_IDX_ERASE_BLOCK,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, BLOCK_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        0,
        },
    },
    {   // Page program - single mode
        NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, PAGE_PROG) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_32BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_WRITE_SDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        },
    },
    {   // End of LUT
        NOR_CMD_LUT_SEQ_IDX_END_OF_TABLE,
        {
            0,0
        },
    },
};

#endif

#if !defined iMX_BOOTLOADER
// SPI FLASH hardware interface
//
    #if defined SPI_FLASH_MULTIPLE_CHIPS
static void fnSPI_command_ATXP(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #else
static void fnSPI_command_ATXP(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #endif
{
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    unsigned long ulChipSelectLine = ulChipSelect[iChipSelect];
    #define ulChipSelectLineSim 0xffffffff
    #else
    #define ulChipSelectLine    CS0_LINE
    #define ulChipSelectLineSim CS0_LINE
    #define iChipSelect 0
    #endif
    #if !defined _iMX
    int iRead = 0;
    int iErase = 0;
    unsigned char ucTxCount = 0;
    unsigned char ucCommandBuffer[3];
    #endif

    FLUSH_SPI_FIFO_AND_FLAGS();                                          // ensure that the FIFOs are empty and the status flags are reset before starting

    if (SPI_FLASH_Danger[iChipSelect] != 0) {                            // check whether the chip is ready to work, if not wait
        #define WAIT_LIMIT_STEP  1000
        volatile unsigned char ucStatus;
        volatile int iWaitCounter = 0;
        volatile int iWaitMax = WAIT_LIMIT_STEP;
        SPI_FLASH_Danger[iChipSelect] = 0;                               // device will no longer be busy after continuing
        do {
            iWaitCounter++;                                              // count the number of times the busy status has been polled
            if (iWaitCounter >= iWaitMax) {                              // if the device has been busy for a long period of time we retrigger the watchdog
                fnRetriggerWatchdog();
                iWaitCounter = 0;
                iWaitMax *= 2;                                           // increase the delay exponentially until the next watchdog trigger takes place so that the watchdog will fire in case it is blocked for a long time
            }
            fnSPI_command_ATXP(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS &ucStatus, 1); // read busy status register
    #if defined MANAGED_FILES
            if (ucCommand == CHECK_SPI_FLASH_BUSY) {                     // pseudo request to see whether device is ready
                if ((ucStatus & STATUS_BUSY) == 0) {
                    return;                                              // the device is no longer busy
                }
                else if (--(*ucData) == 0) {
                    SPI_FLASH_Danger[iChipSelect] = 1;                   // put the busy bit back
                    return;                                              // the maximum number of status requests has expired so quit
                }
            }
    #endif
        } while ((ucStatus & STATUS_BUSY) != 0);                         // until no longer busy
    }
    #if defined MANAGED_FILES
    else if (ucCommand == CHECK_SPI_FLASH_BUSY) {                        // pseudo command used to check device's status
        return;                                                          // the device is not busy so return immediately
    }
    #endif

#if defined _iMX
    switch (ucCommand) {
    case READ_DATA_BYTES:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READ_FAST, DataLength, (unsigned char *)ucData, 0, ulPageNumberOffset);
        return;
    case WRITE_ENABLE:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0);
        return;
    case WRITE_DISABLE:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEDISABLE, 0, 0, 0, 0);
        return;
    case READ_STATUS_REGISTER_1:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, (unsigned char *)ucData, 0, 0);
        return;
    case PAGE_PROG:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE, DataLength, 0, (unsigned char *)ucData, ulPageNumberOffset);
        break;
    case _UNPROTECT_SECTOR:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_UNPROTECT_SECTORS, 0, 0, 0, ulPageNumberOffset);
        return;
    case SUB_SECTOR_ERASE:                                               // sector erase - 4k (name for compatibility)
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_ERASESECTOR, 0, 0, 0, ulPageNumberOffset); // delete sector
        break;
    case HALF_BLOCK_ERASE:                                               // half block erase - 32k
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_ERASE_HALF_BLOCK, 0, 0, 0, ulPageNumberOffset); // delete half block
        break;
    case BLOCK_ERASE:                                                    // block erase - 64k
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_ERASE_BLOCK, 0, 0, 0, ulPageNumberOffset); // delete block
        break;
    case READ_JEDEC_ID:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READID, DataLength, (unsigned char *)ucData, 0, 0);
        return;
    case WRITE_STATUS_REGISTER:
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG, 1, 0, (unsigned char *)ucData, 0);
        break;
    default:
        _EXCEPTION("Unsupported command!");
        break;
    }
    SPI_FLASH_Danger[iChipSelect] = 1;                                   // before the next access is made the status register's busy flag needs to be checked and waited on until cleared
#else
    SET_SPI_FLASH_MODE();

    #if !defined DSPI_SPI || defined MANUAL_FLASH_CS_CONTROL             // control chip select line when no automation is available or when specifically preferred
    ASSERT_CS_LINE(ulChipSelectLine);                                    // assert the chip select line
    #endif

    switch (ucCommand) {
    case BLOCK_ERASE:                                                    // 64k block erase
    case HALF_BLOCK_ERASE:                                               // 32k half-block
    case SUB_SECTOR_ERASE:                                               // 4k sector
        iErase = 1;
    case PAGE_PROG:
        SPI_FLASH_Danger[iChipSelect] = 1;                               // a write/erase will be started so we need to poll the status before next command
    case READ_DATA_BYTES:                                                // 25MHz read - first setting the address and then reading the defined amount of data bytes
        WRITE_SPI_CMD0(ucCommand);                                       // send command
    #if defined _WINDOWS
        fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);          // simulate the SPI FLASH device
    #endif
    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
        ucCommandBuffer[0] = (unsigned char)(ulPageNumberOffset >> 24);  // define the address to be read, written or erased
        ucCommandBuffer[1] = (unsigned char)(ulPageNumberOffset >> 16);
        ucCommandBuffer[2] = (unsigned char)(ulPageNumberOffset >> 8);
        ucCommandBuffer[3] = (unsigned char)(ulPageNumberOffset);
    #else
        ucCommandBuffer[0] = (unsigned char)(ulPageNumberOffset >> 16);  // define the address to be read, written or erased
        ucCommandBuffer[1] = (unsigned char)(ulPageNumberOffset >> 8);
        ucCommandBuffer[2] = (unsigned char)(ulPageNumberOffset);
    #endif
        while (ucTxCount < sizeof(ucCommandBuffer)) {                    // complete the command sequence
            WAIT_SPI_RECEPTION_END();                                    // wait until at least one byte is in the receive FIFO
            (void)READ_SPI_FLASH_DATA();                                 // the rx data is not interesting here
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
            if ((ucTxCount == (sizeof(ucCommandBuffer) - 1)) && (iErase != 0)) { // erase doesn't have further data after the address
                WRITE_SPI_CMD0_LAST(ucCommandBuffer[ucTxCount++]);       // send address with no further data to follow
            }
            else {
                WRITE_SPI_CMD0(ucCommandBuffer[ucTxCount++]);            // send address
            }
        #if defined _WINDOWS
            fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
        #endif
        }
        break;

    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    case ENTER_4_BYTE_MODE:
    #endif
    case WRITE_DISABLE:
    case WRITE_ENABLE:
        WRITE_SPI_CMD0_LAST(ucCommand);                                  // send command
    #if defined _WINDOWS
        fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);          // simulate the SPI FLASH device
    #endif
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
    #if !defined DSPI_SPI || defined MANUAL_FLASH_CS_CONTROL             // control chip select line when no automation is available or when specifically preferred
        NEGATE_CS_LINE(ulChipSelectLine);                                // negate the chip select line
    #endif
    #if defined _WINDOWS
        #if defined DSPI_SPI
        if ((SPI_TX_BYTE & SPI_PUSHR_EOQ) != 0) {                        // check that the CS has been negated
            SPI_TX_BYTE &= ~(ulChipSelectLine);
        }
        #endif
        fnSimSPI_Flash(W25Q_CHECK_SS, 0);                                // simulate the SPI FLASH device
    #endif
        REMOVE_SPI_FLASH_MODE();
        return;

    case READ_JEDEC_ID:
    case READ_STATUS_REGISTER_1:                                         // read single byte from status register
        WRITE_SPI_CMD0(ucCommand);                                       // send command
        iRead = 1;
    #if defined _WINDOWS
        fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);          // simulate the SPI FLASH device
    #endif
        ucTxCount = sizeof(ucCommandBuffer);                             // no additional address to be written
        break;

    case WRITE_STATUS_REGISTER:
        WRITE_SPI_CMD0(ucCommand);                                       // send command
    #if defined _WINDOWS
        fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);          // simulate the SPI FLASH device
    #endif
    #if !defined DSPI_SPI
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
    #endif
    default:
        break;
    }

    if ((iRead != 0) || (READ_DATA_BYTES == ucCommand)) {
        WAIT_SPI_RECEPTION_END();                                        // wait until tx byte has been sent and rx byte has been completely received
        (void)READ_SPI_FLASH_DATA();                                     // the rx data is not interesting here
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        while (DataLength-- != 0) {                                      // while data bytes to be read
            if (DataLength == 0) {                                       // final byte
                WRITE_SPI_CMD0_LAST(0xff);                               // send idle line (final byte)
            }
            else {
                WRITE_SPI_CMD0(0xff);                                    // send idle line
            }
    #if defined _WINDOWS
            SPI_RX_BYTE = fnSimSPI_Flash(W25Q_READ, 0);                  // simulate the SPI FLASH device
    #endif
            WAIT_SPI_RECEPTION_END();                                    // wait until tx byte has been sent and rx byte has been completely received
            *ucData++ = READ_SPI_FLASH_DATA();
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
        }
    }
    else {
        WAIT_SPI_RECEPTION_END();                                        // wait until tx byte has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        while (DataLength-- != 0) {                                      // while data bytes to be written
            if (DataLength == 0) {                                       // final byte
                WRITE_SPI_CMD0_LAST(*ucData++);                          // send data (final byte)
    #if defined _WINDOWS
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
            }
            else {
                WRITE_SPI_CMD0(*ucData++);                               // send data
    #if defined _WINDOWS
                fnSimSPI_Flash(W25Q_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
            }
            WAIT_SPI_RECEPTION_END();                                    // wait until tx byte has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
        }
    }

    #if !defined DSPI_SPI || defined MANUAL_FLASH_CS_CONTROL             // control chip select line when no automation is available or when specifically preferred
    NEGATE_CS_LINE(ulChipSelectLine);                                    // negate the chip select line
    #endif
    #if defined _WINDOWS
        #if defined DSPI_SPI
    if ((SPI_TX_BYTE & SPI_PUSHR_EOQ) != 0) {                            // check that the CS has been negated
        SPI_TX_BYTE &= ~(ulChipSelectLine);
    }
        #endif
    fnSimSPI_Flash(W25Q_CHECK_SS, 0);                                    // simulate the SPI FLASH device
    #endif
    REMOVE_SPI_FLASH_MODE();
#endif
}

// Check whether a known SPI FLASH device can be detected - called only once on start up
//
#if defined SPI_FLASH_MULTIPLE_CHIPS
static unsigned char fnCheckATXPXXX(int iChipSelect)
#else
static unsigned char fnCheckATXPXXX(void)
#endif
{
#if defined SPI_FLASH_ATXP128
    volatile unsigned char ucID[3];
    #define MANUFACTURER_INDEX  0
#else
    volatile unsigned char ucID[10];
    #define MANUFACTURER_INDEX  7
#endif
    unsigned char ucReturnType = NO_SPI_FLASH_AVAILABLE;
#if defined _iMX
    unsigned char ucStatusRegister;
    fnInitSpiFlash(0);
    fnEnterLUT(tableLUT_config, 0);                                      // enter the look-up table used during initialisation
#else
    fnDelayLoop(10000);                                                  // the SPI Flash requires maximum 10ms after power has been applied until it can be written
#endif
    fnSPI_command_ATXP(READ_JEDEC_ID, 0, __EXTENDED_CS ucID, sizeof(ucID));
    if (ucID[MANUFACTURER_INDEX] == MANUFACTURER_ID_ADESTO) {            // adesto memory part recognised
        switch (ucID[MANUFACTURER_INDEX + 1]) {
        case DEVICE_ID_1_DATA_ADESTO_FLASH_32M:                          // 32MBit / 4MegByte - ATXP032
            ucReturnType = EcoXiP_ATXP032;
            break;
        case DEVICE_ID_1_DATA_ADESTO_FLASH_64M:                          // 64MBit / 8MegByte - ATXP064
            ucReturnType = EcoXiP_ATXP064;
            break;
        case DEVICE_ID_1_DATA_ADESTO_FLASH_128M:                         // 128MBit / 16MegByte - ATXP128
            ucReturnType = EcoXiP_ATXP128;
            break;
        default:                                                         // possibly a larger part but we don't accept it
            return NO_SPI_FLASH_AVAILABLE;
        }
    }
    else {
        return NO_SPI_FLASH_AVAILABLE;
    }
#if defined _iMX && defined WRITE_PROTECT_SPI_FLASH_BOOT_AREA_
    // Ensure that the boot block is write protected by default
    //
    fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READFUNCTION, 1, &ucStatusRegister, 0, 0); // read the function register
    if ((ucStatusRegister & FUNCTION_TBS) == 0) {
        ucStatusRegister = FUNCTION_TBS;                                 // write to bottom configuration in order to protect the boot loader area
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0); // enable writes
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEFUNCTION, 1, 0, &ucStatusRegister, 0); // save the value (one-time-programmable and cannot be changed back again)
    }
    do {
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
    } while ((ucStatusRegister & STATUS_BUSY) != 0);                     // repeat if busy
    if ((ucStatusRegister & (STATUS_QE | STATUS_BP0 | STATUS_BP1 | STATUS_BP2 | STATUS_BP3)) != (STATUS_QE | STATUS_BP0)) { // check quad mode and write protecion configuration
        ucStatusRegister |= (STATUS_QE | STATUS_BP0);
        ucStatusRegister &= ~(STATUS_BP1 | STATUS_BP2 | STATUS_BP3);     // quad mode configured with write protection on the boot block
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0); // enable writes
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG, 1, 0, &ucStatusRegister, 0); // save the value
        do {
            fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
        } while ((ucStatusRegister & STATUS_BUSY) != 0);                 // repeat until no longer busy
    }
#else
    fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
    if ((ucStatusRegister & STATUS_SWP_ALL_PROTECTED) != 0) {            // if protected (this device powers up with all sectors protected)
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0); // enable writes
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_UNPROTECT_SECTORS, 0, 0, 0, 0); // unprotect sectors
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
    }
    #if 0
    if ((ucStatusRegister & STATUS_QE) == 0) {
        ucStatusRegister |= STATUS_QE;
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0); // enable writes
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG, 1, 0, &ucStatusRegister, 0); // save the value
        do {
            fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
        } while ((ucStatusRegister & STATUS_BUSY) != 0);                 // repeat until no longer busy
    }
    #endif
#endif
    fnEnterLUT(tableLUT_main, 0);                                        // enter the operating look-up table
    return ucReturnType;
}
#endif
#endif
#endif
