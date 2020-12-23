/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      spi_flash_S26KL.h - cypress
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    This file contains SPI FLASH specific code for all chips that are supported.
    It is declared as a header so that projects do not need to specify that it is not to be compiled.
    Its goal is to improve overall readability of the hardware interface.

    **********************************************************************/


#if defined SPI_FLASH_S26KL

#if defined _SPI_DEFINES


#if !defined iMX_BOOTLOADER
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    static void fnSPI_command_S26KL(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #else
    static void fnSPI_command_S26KL(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength);
    #endif
#endif

#define CHECK_SPI_FLASH_BUSY     0x00                                    // pseudo request to see whether device is ready
#define READ_STATUS_REGISTER_1   0x01                                    // all commands are pseudo requests to make the interface compatible with SPI types
    #define STATUS_ESTAT         0x0100                                  // sector erase status bit
    #define STATUS_SLSB          0x0200                                  // sector lock status bit
    #define STATUS_PSSB          0x0400                                  // program suspend status bit
    #define STATUS_WBASB         0x0800                                  // write buffer abort status bit
    #define STATUS_PSB           0x1000                                  // program status bit
    #define STATUS_ESB           0x2000                                  // erase status bit
    #define STATUS_ESSB          0x4000                                  // erase suspend status bit
    #define STATUS_READY         0x8000                                  // device ready bit
    #define STATUS_CRCSSB        0x0001                                  // CRC suspend status bit
#define READ_JEDEC_ID            0x02
#define READ_DATA_BYTES          0x03
#define PAGE_PROG                0x04
#define BLOCK_ERASE              0x05


#define CFI_ENTER                       0x98
#define RESET_ASO_EXIT                  0xf0                             // or 0xff
#define CFI_COMMAND_ADDRESS             (0x555 << 1)
#define STATUS_REG__COMMAND_ADDRESS     (0x555 << 1)
#define STATUS_REGISTER_READ            0x70

#define MANUFACTURER_ID_CYPRESS  0x01                                    // Cypress manufacturer's ID

// Unique ID's
//
#define DEVICE_ID_1_DATA_CYPRESS_FLASH_512M  0x7e                        // 512MBit / 64MegByte - S26KL512

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
        #define POWER_DOWN_SPI_FLASH_INTERFACE()
    #endif
    #if defined SPI_FLASH_MULTIPLE_CHIPS
        #define __EXTENDED_CS     iChipSelect,
        static unsigned char fnCheckS26KLXXX(int iChipSelect);
        static const STORAGE_AREA_ENTRY spi_flash_storage_S26KL = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            SPI_FLASH_DEVICE_COUNT                                       // multiple devices
        };
    #else
        #define __EXTENDED_CS
        static unsigned char fnCheckS26KLXXX(void);
        static const STORAGE_AREA_ENTRY spi_flash_storage_S26KL = {
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
    ucSPI_FLASH_Type[0] = fnCheckS26KLXXX(0);                            // flag whether the first SPI FLASH device is connected
    #else
    ucSPI_FLASH_Type[0] = fnCheckS26KLXXX();                             // flag whether the SPI FLASH device is connected
    #endif
    if (ucSPI_FLASH_Type[0] < S26KL512) {                                // we expect at least this part to be available
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
        UserStorageListPtr = (STORAGE_AREA_ENTRY *)&spi_flash_storage_S26KL; // insert spi flash as storage medium
    #if defined SPI_FLASH_SECOND_SOURCE_MODE
        fnSPI_command = fnSPI_command_is25;
    #endif
    }
#endif



#if defined _SPI_FLASH_INTERFACE
// This is the main interface code to the ISSI SPI FLASH device
//
/* =================================================================== */
/*                            S26KL driver                             */
/* =================================================================== */

#if defined _iMX

#define HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA(transaction, address, data) \
    FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (transaction)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00), \
    FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, ((unsigned char)(address >> 11))) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, ((unsigned char)(address >> 3))), \
    FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, ((unsigned char)(address & 0x7))), \
    FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, ((unsigned char)(data >> 8))) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, ((unsigned char)(data)))


static const FLEXSPI_LUT_ENTRY tableLUT_config[] = {                     // look-up table used only during configuration
    {   // Read data
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA,
        {
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_READ | HYPTER_PROTOCOL_CA0_LINEAR_BURST)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_RADDR_DDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_CADDR_DDR, ADDRESS_LENGTH_16BITS) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_READ_DDR, 0x04),
        0,0
        },
    },
    {   // Write data
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEDATA,
        {
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_LINEAR_BURST)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_RADDR_DDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_CADDR_DDR, ADDRESS_LENGTH_16BITS) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_WRITE_DDR, 0x02),
        0,0
        },
    },
    {   // Read status
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_READSTATUS,
        {
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x05),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x70),
        },
    },
    {   // Read status - follow on 1
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_READSTATUS_1,
        {
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_READ | HYPTER_PROTOCOL_CA0_LINEAR_BURST)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_RADDR_DDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_CADDR_DDR, ADDRESS_LENGTH_16BITS) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DUMMY_RWDS_DDR, 0x0b),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_READ_DDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // Write enable
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEENABLE,
        {                                                                // unlock command 1 - write data 0x00aa to address 0x555
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x00000555, 0x00aa),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x05),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa)*/
        },
    },
    {   // Write enable - follow on 1
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEENABLE_1,
        {                                                                // unlock command 2 - write data 0x0055 to address 0x2aa
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x000002aa, 0x0055),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x55),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x02),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x55)*/
        },
    },
    {   // Erase sector
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_ERASESECTOR,
        {                                                                // sector erase (1) - write data 0x0080 to address 0x555 (proceeded by write enable sequence)
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x00000555, 0x0080),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x05),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x80)*/
        },
    },
    {   // Erase sector - follow on 1
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_ERASESECTOR_1,
        {                                                                // sector erase (2) - write data 0x00aa to address 0x555 (proceeded by sector erase 1)
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x00000555, 0x00aa),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x05),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa)*/
        },
    },
    {   // Erase sector - follow on 2
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_ERASESECTOR_2,
        {                                                                // sector erase (3) - write data 0x0055 to address 0x2aa (proceeded by sector erase 2)
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x000002aa, 0x0055),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x55),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x02),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x55),*/
        },
    },
    {   // Erase sector - follow on 3
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_ERASESECTOR_3,
        {                                                                // sector erase (4) - write data 0x0030 to address in sector to be erased (proceeded by sector erase 3)
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_RADDR_DDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_CADDR_DDR, ADDRESS_LENGTH_16BITS) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x30) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
        0,
        },
    },
    {   // Write page
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_PAGEPROGRAM,
        {                                                                // word program - write data 0x00a0 to address 0x555 (proceeded by write enable sequence)
        HYPERFLASH_COMMAND_ADDRESS_ASSIGNMENT_DATA((HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM), 0x00000555, 0x00a0),
      /*FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x00),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xaa),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0x05),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, 0x00) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_DDR, 0xa0)*/
        },
    },
    {   // Write page - follow on 1
        //
        HYPERFLASH_CMD_LUT_SEQ_IDX_PAGEPROGRAM_1,
        {                                                                // program address followed by program word value
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_DDR, (HYPTER_PROTOCOL_CA0_WRITE | HYPTER_PROTOCOL_CA0_TARGET_MEM)) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_RADDR_DDR, ADDRESS_LENGTH_24BITS),
        FLEXSPI_LUT_8PAD_SEQ_1(FLEXSPI_Command_CADDR_DDR, ADDRESS_LENGTH_16BITS) | FLEXSPI_LUT_8PAD_SEQ_2(FLEXSPI_Command_WRITE_DDR, 0x80),
        0,0
        },
    },
    {   // End of LUT
        //
        NOR_CMD_LUT_SEQ_IDX_END_OF_TABLE,
        {
            0,0,0,0
        },
    },
};

#define tableLUT_main  tableLUT_config

#endif

#if !defined iMX_BOOTLOADER
// SPI FLASH hardware interface
//
    #if defined SPI_FLASH_MULTIPLE_CHIPS
static void fnSPI_command_S26KL(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #else
static void fnSPI_command_S26KL(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
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
        #define WAIT_LIMIT_STEP  1000
        volatile unsigned short usStatus;
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
            fnSPI_command_S26KL(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS (volatile unsigned char *)&usStatus, sizeof(usStatus)); // read busy status register
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
        } while ((usStatus & STATUS_READY) == 0);                        // until no longer busy
    }
    #if defined MANAGED_FILES
    else if (ucCommand == CHECK_SPI_FLASH_BUSY) {                        // pseudo command used to check device's status
        return;                                                          // the device is not busy so return immediately
    }
    #endif

#if defined _iMX
    switch (ucCommand) {
    case READ_DATA_BYTES:
        {
            unsigned char ulReadBuffer[2];
            MAX_FILE_LENGTH LengthToRead;
            if (DataLength == 0) {
                return;
            }
            if ((ulPageNumberOffset & 0x1) != 0) {                       // if reading from an odd address
                fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA, 2, ulReadBuffer, 0, (ulPageNumberOffset - 1)); // read two bytes from the even address
                *ucData++ = ulReadBuffer[1];                             // put the second byte into the user buffer
                if (--DataLength == 0) {
                    return;                                              // single odd address read complete
                }
                ulPageNumberOffset++;
            }
            // Reads from here are even address aligned
            //
            LengthToRead = (DataLength & ~1);                            // even length to read
            DataLength &= 1;                                             // remaining byte
            if (LengthToRead != 0) {                                     // if more that a single byte remaining
                fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA, LengthToRead, (unsigned char *)ucData, 0, ulPageNumberOffset); // read an even length of bytes
                ulPageNumberOffset += LengthToRead;
                ucData += LengthToRead;
            }
            if (DataLength != 0) {                                       // if one additional byte is to be read
                fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA, 1, (unsigned char *)ucData, 0, ulPageNumberOffset); // read an even length of bytes
            }
            return;
        }
    case READ_STATUS_REGISTER_1:
        {
            static const unsigned char ucCommand[2] = { 0x00, STATUS_REGISTER_READ };
            fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEDATA, sizeof(ucCommand), 0, (unsigned char *)ucCommand, STATUS_REG__COMMAND_ADDRESS); // enter CFI (common flash interface)
            fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA, DataLength, (unsigned char *)ucData, 0, 0); // read the status register
        }
        return;
    case PAGE_PROG:
        {
            unsigned long ulIntermediateBuffer = 0;
            unsigned char ucIntemediateWriteBuffer[SPI_FLASH_PAGE_LENGTH];
            int iStartIndex = (ulPageNumberOffset % SPI_FLASH_PAGE_LENGTH);
            unsigned long ulWriteDataLength = DataLength;
            fnFlexSPI_Command(0, (HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEENABLE | WRITEENABLE_SEQUENCE_LENGTH), 0, 0, 0, 0);
            if ((ulPageNumberOffset & 0x1) != 0) {                       // first byte write is to an uneven address
                ucIntemediateWriteBuffer[iStartIndex - 1] = 0xff;        // prime 0xff so that it doesn't change data when programmed
                ulPageNumberOffset--;                                    // program to one address lower
                ulWriteDataLength++;                                     // program one extra byte
                ulIntermediateBuffer = 1;                                // the intermediate buffer needs to be used
            }
            if (((ulPageNumberOffset + ulWriteDataLength) & 0x1) != 0) { // final byte write is to an uneven address
                ucIntemediateWriteBuffer[iStartIndex + DataLength] = 0xff; // prime 0xff so that it doesn't change data when programmed
                ulWriteDataLength++;                                     // program one extra byte
                ulIntermediateBuffer |= 2;                               // the intermediate buffer needs to be used
            }
            if (ulIntermediateBuffer != 0) {
                uMemcpy(&ucIntemediateWriteBuffer[iStartIndex], (const void *)ucData, DataLength); // copy the user data to the intermediate buffer
                ucData = &ucIntemediateWriteBuffer[iStartIndex];
                if ((ulIntermediateBuffer & 0x1) != 0) {
                    ucData--;
                }
            }
            fnFlexSPI_Command(0, (HYPERFLASH_CMD_LUT_SEQ_IDX_PAGEPROGRAM | WRITEDATA_SEQUENCE_LENGTH), ulWriteDataLength, 0, (unsigned char *)ucData, ulPageNumberOffset);
        }
        break;
    case BLOCK_ERASE:                                                    // block erase - 256k
        fnFlexSPI_Command(0, (HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEENABLE | WRITEENABLE_SEQUENCE_LENGTH), 0, 0, 0, ulPageNumberOffset);
        fnFlexSPI_Command(0, (HYPERFLASH_CMD_LUT_SEQ_IDX_ERASESECTOR | ERASESECTOR_SEQUENCE_LENGTH), 0, 0, 0, ulPageNumberOffset); // delete block
        break;
    case READ_JEDEC_ID:
        {
            unsigned char ucCommand[2] = {0x00, CFI_ENTER};
            fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEDATA, sizeof(ucCommand), 0, ucCommand, CFI_COMMAND_ADDRESS); // enter CFI (common flash interface)
            fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_READDATA, DataLength, (unsigned char *)ucData, 0, 0); // read the ID
            ucCommand[1] = RESET_ASO_EXIT;
            fnFlexSPI_Command(0, HYPERFLASH_CMD_LUT_SEQ_IDX_WRITEDATA, sizeof(ucCommand), 0, ucCommand, 0); // exit CFI
        }
        return;
    default:
        _EXCEPTION("Unsupported command!");
        break;
    }
    SPI_FLASH_Danger[iChipSelect] = 1;                                   // before the next access is made the status register's busy flag needs to be checked and waited on until cleared
#else
    // SPI mode doesn't exist for this device
    //
#endif
}

// Check whether a known SPI FLASH device can be detected - called only once on start up
//
#if defined SPI_FLASH_MULTIPLE_CHIPS
static unsigned char fnCheckS26KLXXX(int iChipSelect)
#else
static unsigned char fnCheckS26KLXXX(void)
#endif
{
    volatile unsigned char ucID[32];
    unsigned char ucReturnType = NO_SPI_FLASH_AVAILABLE;
#if defined _iMX
    unsigned short usStatusRegister;
    fnInitSpiFlash(0);
    fnEnterLUT(tableLUT_config, 0);                                      // enter the look-up table used during initialisation
    #if !defined BOOT_HYPER_FLASH
    fnFlexSPI_Command(0, NOR_CMD_LUT_SEQ_IDX_EXITQPI, 0, 0, 0, 0);       // use single line mode for initial monitoring
    #endif
#else
    fnDelayLoop(10000);                                                  // the SPI Flash requires maximum 10ms after power has been applied until it can be written
#endif
    fnSPI_command_S26KL(READ_JEDEC_ID, 0, __EXTENDED_CS ucID, sizeof(ucID));
    if (ucID[1] == MANUFACTURER_ID_CYPRESS) {                            // ISSI memory part recognised
        switch (ucID[3]) {
        case DEVICE_ID_1_DATA_CYPRESS_FLASH_512M:                        // 512MBit / 64MegByte - S26KL512
            ucReturnType = S26KL512;
            break;
        default:                                                         // possibly a larger part but we don't accept it
            return NO_SPI_FLASH_AVAILABLE;
        }
    }
    else {
        return NO_SPI_FLASH_AVAILABLE;
    }
#if defined _iMX && defined WRITE_PROTECT_SPI_FLASH_BOOT_AREA
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
    fnSPI_command_S26KL(READ_STATUS_REGISTER_1, 0, (unsigned char *)&usStatusRegister, sizeof(usStatusRegister)); // read the status register
#endif
  //fnEnterLUT(tableLUT_main, 0);                                        // enter the operating look-up table
    return ucReturnType;
}
#endif
#endif
#endif
