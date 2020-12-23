/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      spi_flash_IS25LP.h - ISSI
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    This file contains SPI FLASH specific code for all chips that are supported.
    It is declared as a header so that projects do not need to specify that it is not to be compiled.
    Its goal is to improve overall readability of the hardware interface.
    15.02.2020 Add FlexSPI Flash mode for iMX RT

    **********************************************************************/


#if defined SPI_FLASH_IS25                                               // the complete content is excluded when this flash device is not used

#if defined _SPI_DEFINES

#if !defined iMX_BOOTLOADER || ((defined MIMXRT1024 || defined MIMXRT1064) && defined USE_EXTERNAL_QSPI_FLASH)
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    static void fnSPI_command_is25(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #else
    static void fnSPI_command_is25(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #endif
#endif
#define CHECK_SPI_FLASH_BUSY     0x00                                    // pseudo request to see whether device is ready

#define WRITE_ENABLE             0x06
#define WRITE_DISABLE            0x04
#define READ_STATUS_REGISTER_1   0x05
    #define STATUS_BUSY          0x01                                    // write in progress flag
    #define STATUS_WEL           0x02                                    // write enable latch
    #define STATUS_BP0           0x04                                    // block protection settings
    #define STATUS_BP1           0x08
    #define STATUS_BP2           0x10
    #define STATUS_BP3           0x20
    #define STATUS_QE            0x40                                    // non-volatile quad-output function
    #define STATUS_SRWD          0x80                                    // non-volatile status register write disable
#define READ_FUNCTION_REGISTER   0x48
    #define FUNCTION_TBS         0x02                                    // top/bottom selection (OTP)
    #define FUNCTION_PSUS        0x04                                    // program suspend bit (read-only)
    #define FUNCTION_ESUS        0x08                                    // erase suspend bit (read-only)
    #define FUNCTION_IR_LOCK_0   0x10                                    // lock information row 0 (OTP)
    #define FUNCTION_IR_LOCK_1   0x20                                    // lock information row 1 (OTP)
    #define FUNCTION_IR_LOCK_2   0x40                                    // lock information row 2 (OTP)
    #define FUNCTION_IR_LOCK_3   0x80                                    // lock information row 3 (OTP)
#define WRITE_FUNCTION_REGISTER  0x42
#define READ_PARAMETERS          0xc0
#define ENTER_QSP                0x35
#define EXIT_QSP                 0xf5
#define WRITE_STATUS_REGISTER    0x01
#define PAGE_PROG                0x02
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
#if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    #define ENTER_4_BYTE_MODE    0xb7
    #define EXIT_4_BYTE_MODE     0xe9
#endif

#define MANUFACTURER_ID_ISSI     0x9d                                    // ISSI manufacturer's ID
#define MANUFACTURER_ID_ADESTO   0x1f                                    // Adesto manufacturer's ID

// Unique ID's
//
#define DEVICE_ID_1_DATA_ISSI_FLASH_32M     0x16                         // 32MBit / 4MegByte - IS25LP032A
#define DEVICE_ID_1_DATA_ISSI_FLASH_64M     0x17                         // 64MBit / 8MegByte - IS25LP064A
#define DEVICE_ID_1_DATA_ISSI_FLASH_256M    0x19                         // 256MBit / 32MegByte - IS25LP256D/IS25WP256D
#define DEVICE_ID_1_DATA_ADESTO_FLASH_128M  0x01                         // 128MBit / 16MegByte - AT25SF128A

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
        #if defined iMX_RT1024 || defined iMX_RT1064
            #define SPI_FLASH_START  (FLEXSPI1_FLASH_BASE)
        #else
            #define SPI_FLASH_START  (FLEXSPI_FLASH_BASE)
        #endif
        #define POWER_DOWN_SPI_FLASH_INTERFACE()
    #endif
    #if defined SPI_FLASH_MULTIPLE_CHIPS
        #define __EXTENDED_CS     iChipSelect,
        static unsigned char fnCheckIS25XXX(int iChipSelect);
        static const STORAGE_AREA_ENTRY spi_flash_storage_IS25 = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            SPI_FLASH_DEVICE_COUNT                                       // multiple devices
        };
    #else
        #define __EXTENDED_CS
        static unsigned char fnCheckIS25XXX(void);
        static const STORAGE_AREA_ENTRY spi_flash_storage_IS25 = {
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
    ucSPI_FLASH_Type[0] = fnCheckIS25XXX(0);                             // flag whether the first SPI FLASH device is connected
    #else
    ucSPI_FLASH_Type[0] = fnCheckIS25XXX();                              // flag whether the SPI FLASH device is connected
    #endif
    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    if (ucSPI_FLASH_Type[0] < IS25LP_WP_256D)
    #elif SPI_FLASH_SIZE >= (8 * 1024 * 1024)
    if (ucSPI_FLASH_Type[0] < IS25LP_WP_64)
    #else
    if (ucSPI_FLASH_Type[0] < IS25LP_WP_32)
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
            ucSPI_FLASH_Type[i] = fnCheckIS25XXX(i);
        }
    #endif
        UserStorageListPtr = (STORAGE_AREA_ENTRY *)&spi_flash_storage_IS25; // insert spi flash as storage medium
    #if defined SPI_FLASH_SECOND_SOURCE_MODE
        fnSPI_command = fnSPI_command_is25;
    #endif
    }
#endif

#if defined _SPI_FLASH_INTERFACE
// This is the main interface code to the ISSI SPI FLASH device
//
/* =================================================================== */
/*                             IS25 driver                             */
/* =================================================================== */
#if defined _iMX
static const FLEXSPI_LUT_ENTRY tableLUT_config_IS25LP[] = {              // look-up table used only during configuration
    {   // Fast read quad mode - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ_QUAD_I_O) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 0x06) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        },
    },
    {   // Exit QPI mode
        NOR_CMD_LUT_SEQ_IDX_EXITQPI,
        {
        FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_SDR, EXIT_QSP) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // Read ID
        NOR_CMD_LUT_SEQ_IDX_READID,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_JEDEC_ID) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        0,
        },
    },
    {   // Read function register
        NOR_CMD_LUT_SEQ_IDX_READFUNCTION,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_FUNCTION_REGISTER) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        0,
        },
    },
    {   // Write function register
        NOR_CMD_LUT_SEQ_IDX_WRITEFUNCTION,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_FUNCTION_REGISTER) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_WRITE_SDR, 0x04),
        0,
        },
    },
    {   // Read status register
        NOR_CMD_LUT_SEQ_IDX_READSTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
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
    {   // Write status register
        NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_STATUS_REGISTER) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_WRITE_SDR, 0x04),
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

static const FLEXSPI_LUT_ENTRY tableLUT_main_IS25LP[] = {                // look-up table used during main operation
    {   // Fast read quad mode - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ_QUAD_I_O) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 0x06) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        },
    },
    {   // Normal read mode - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_NORMAL,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_DATA_BYTES) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_READ_SDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        },
    },
    {   // Enter QPI mode
        NOR_CMD_LUT_SEQ_IDX_ENTERQPI,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, ENTER_QSP) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    /*
    {   // Fast read mode - SDR
        NOR_CMD_LUT_SEQ_IDX_READ_FAST,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 0x08) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        },
    },*/
    {   // Read parameters
        NOR_CMD_LUT_SEQ_IDX_READSTATUS,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_PARAMETERS) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
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
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, SUB_SECTOR_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        0,
        },
    },
    {   // Erase half-block - 32k
        NOR_CMD_LUT_SEQ_IDX_ERASE_HALF_BLOCK,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, HALF_BLOCK_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        0,
        },
    },
    {   // Erase block - 64k
        NOR_CMD_LUT_SEQ_IDX_ERASE_BLOCK,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, BLOCK_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        0,
        },
    },
    {   // Page program - single mode
        NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, PAGE_PROG) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_WRITE_SDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        },
    },
    /*
    {   // Page program - quad mode
        NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, QUAD_PROGRAM_PAGE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_WRITE_SDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        },
    },*/
    {   // Write status register
        NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_STATUS_REGISTER) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_WRITE_SDR, 0x04),
        0,
        },
    },
    {   // Read status register
        NOR_CMD_LUT_SEQ_IDX_READSTATUSREG,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        0,
        },
    },
    /*
    {   // Read status register in quad mode
        NOR_CMD_LUT_SEQ_IDX_READSTATUSREG_QUAD,
        {
        FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
        0,
        },
    },*/
    {   // Erase whole chip
        NOR_CMD_LUT_SEQ_IDX_ERASECHIP,
        {
        FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, CHIP_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
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
#endif

#if !defined iMX_BOOTLOADER || ((defined MIMXRT1024 || defined MIMXRT1064) && defined USE_EXTERNAL_QSPI_FLASH)
// SPI FLASH hardware interface
//
    #if defined SPI_FLASH_MULTIPLE_CHIPS
static void fnSPI_command_is25(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #else
static void fnSPI_command_is25(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
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
        #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    unsigned char ucCommandBuffer[4];
        #else
    unsigned char ucCommandBuffer[3];
        #endif
    FLUSH_SPI_FIFO_AND_FLAGS();                                          // ensure that the FIFOs are empty and the status flags are reset before starting
    #endif
    if (SPI_FLASH_Danger[iChipSelect] != 0) {                            // check whether the chip is ready to work, if not wait
        volatile unsigned char ucStatus;
        SPI_FLASH_Danger[iChipSelect] = 0;                               // device will no longer be busy after continuing
        do {
            fnSPI_command_is25(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS &ucStatus, 1); // read busy status register
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
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READ_NORMAL, DataLength, (unsigned char *)ucData, 0, ulPageNumberOffset);
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
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE/*NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD*/, DataLength, 0, (unsigned char *)ucData, ulPageNumberOffset);
        break;
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
static unsigned char fnCheckIS25XXX(int iChipSelect)
#else
static unsigned char fnCheckIS25XXX(void)
#endif
{
    volatile unsigned char ucID[3];
    unsigned char ucReturnType = NO_SPI_FLASH_AVAILABLE;
#if defined _iMX
    unsigned char ucStatusRegister;
    fnInitSpiFlash(0);
    fnEnterLUT(tableLUT_config_IS25LP, 0);                               // enter the look-up table used during initialisation
    #if !defined BOOT_HYPER_FLASH
    fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_EXITQPI, 0, 0, 0, 0);       // use single line mode for initial monitoring
    #endif
#else
    fnDelayLoop(10000);                                                  // the SPI Flash requires maximum 10ms after power has been applied until it can be written
#endif
    fnSPI_command_is25(READ_JEDEC_ID, 0, __EXTENDED_CS ucID, sizeof(ucID)); // read the SPI flash identity
    if (ucID[0] == MANUFACTURER_ID_ISSI) {                               // ISSI memory part recognised
        switch (ucID[2]) {
        case DEVICE_ID_1_DATA_ISSI_FLASH_32M:                            // 32MBit / 4MegByte - IS25LP032A
            ucReturnType = IS25LP_WP_32;
            break;
        case DEVICE_ID_1_DATA_ISSI_FLASH_64M:                            // 64MBit / 8MegByte - IS25LP064A
            ucReturnType = IS25LP_WP_64;
            break;
#if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
        case DEVICE_ID_1_DATA_ISSI_FLASH_256M:
            fnSPI_command_is25(ENTER_4_BYTE_MODE, 0, __EXTENDED_CS ucID, 0); // switch to 4 byte mode so that more that 16MByte can be addressed
            return IS25LP_WP_256D;
#endif
        default:                                                         // possibly a larger part but we don't accept it
            return NO_SPI_FLASH_AVAILABLE;
        }
    }
    else {
        return NO_SPI_FLASH_AVAILABLE;
    }
#if defined _iMX
    #if defined WRITE_PROTECT_SPI_FLASH_BOOT_AREA
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
    if ((ucStatusRegister & STATUS_QE) == 0) {                           // if the non-volatile quad-output function is not enabled
        ucStatusRegister |= STATUS_QE;                                   // enable the quad-output function
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, 0, 0, 0); // enable writes
        fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG, 1, 0, &ucStatusRegister, 0); // save the new value
        do {
            fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_READSTATUSREG, 1, &ucStatusRegister, 0, 0); // read the status register
        } while ((ucStatusRegister & STATUS_BUSY) != 0);                 // repeat until no longer busy
    }
    #endif
    fnEnterLUT(tableLUT_main_IS25LP, 0);                                 // enter the operating look-up table
#endif
    return ucReturnType;
}
#endif
#endif
#endif
