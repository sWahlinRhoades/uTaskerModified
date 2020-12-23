/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      spi_flash_MX25L.h - Macronix
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    09.03.2019 Make generic for any processor
    22.03.2019 Add 32MByte part (with 4-byte mode)

*/

#if defined SPI_FLASH_MX25L

#if defined _SPI_DEFINES
    #if defined _STM32
        #define MANUAL_FLASH_CS_CONTROL
        #define ASSERT_CS_LINE(cs_line) __ASSERT_CS(cs_line)
        #define NEGATE_CS_LINE(cs_line) __NEGATE_CS(cs_line)
        #define FLUSH_SPI_FIFO_AND_FLAGS()
        #define WRITE_SPI_CMD0(ucCommand) SSPDR_X = ucCommand
        #define WRITE_SPI_CMD0_LAST(ucData) SSPDR_X = ucData
        #define READ_SPI_FLASH_DATA()     (unsigned char)SSPDR_X
        #define CLEAR_RECEPTION_FLAG()
        #if defined _WINDOWS
            #define WAIT_TRANSFER_END()   while ((SSPSR_X & SPISR_TXE) == 0) { SSPSR_X |= SPISR_TXE;} \
                                          while ((SSPSR_X & SPISR_BSY) != 0) {SSPSR_X &= ~SPISR_BSY;}
        #else
            #define WAIT_TRANSFER_END()   while ((SSPSR_X & SPISR_TXE) == 0) {} \
                                          while ((SSPSR_X & SPISR_BSY) != 0) {}
        #endif
        #define WAIT_SPI_RECEPTION_END()  WAIT_TRANSFER_END()
        #if !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
        #if !defined SPI_TX_BYTE
            #define SPI_TX_BYTE           SSPDR_X                        // for simulator
        #endif
        #if !defined SPI_RX_BYTE
            #define SPI_RX_BYTE           SSPDR_X                        // for simulator
        #endif
    #elif defined _KINETIS
        #if !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
    #endif
    #if defined SPI_FLASH_MULTIPLE_CHIPS
        #define __EXTENDED_CS     iChipSelect,
        static unsigned char fnCheckMX25L(int iChipSelect);
        static const STORAGE_AREA_ENTRY spi_flash_storage_MX25L = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            SPI_FLASH_DEVICE_COUNT                                       // multiple devices
        };
    #else
        #define __EXTENDED_CS
        static unsigned char fnCheckMX25L(void);
        static const STORAGE_AREA_ENTRY spi_flash_storage_MX25L = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START),                          // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            0                                                            // not multiple devices
        };
    #endif
#endif


    // This code is inserted to detect the presence of the SPI FLASH device(s). If the first device is not detected the SPI interface is disabled.
    // If there are multiple devices, each will be recorded.
    //
#if defined _CHECK_SPI_CHIPS
    #if defined SPI_FLASH_MULTIPLE_CHIPS
    ucSPI_FLASH_Type[0] = fnCheckMX25L(0);                               // flag whether the first SPI FLASH device is connected
    #else
    ucSPI_FLASH_Type[0] = fnCheckMX25L();                                // flag whether the SPI FLASH device is connected
    #endif
    if (ucSPI_FLASH_Type[0] < MX25L1645) {                               // we expect at least this part to be available
    #if !defined SPI_FLASH_SECOND_SOURCE_MODE
        POWER_DOWN_SPI_FLASH_INTERFACE();                                // power down SPI 
    #endif
    }
    else {
    #if defined SPI_FLASH_MULTIPLE_CHIPS                                 // check for further devices
        int i = 0;
        while (++i < SPI_FLASH_DEVICE_COUNT) {
            ucSPI_FLASH_Type[i] = fnCheckMX25L(i);
        }
    #endif
        UserStorageListPtr = (STORAGE_AREA_ENTRY *)&spi_flash_storage_MX25L; // insert spi flash as storage medium
    #if defined SPI_FLASH_SECOND_SOURCE_MODE
        fnSPI_command = fnSPI_command_MX25L;
    #endif
    }
#endif




#if defined _SPI_FLASH_INTERFACE
// This is the main interface code to the MX25L SPI FLASH device

/* =================================================================== */
/*                             MX25L driver                            */
/* =================================================================== */

#define READ_STATUS_REGISTER_1   0x05
  #define STATUS_WIP             0x01                                    // WIP (write in progress)
  #define STATUS_WEL             0x02                                    // WEL (write enable latch)
  #define STATUS_BP0             0x04                                    // block protection 0
  #define STATUS_BP1             0x08                                    // block protection 1
  #define STATUS_BP2             0x10                                    // block protection 2
  #define STATUS_BP3             0x20                                    // block protection 3
  #define STATUS_QUAD_ENABLE     0x40                                    // quad enable
  #define STATUS_SRWD            0x80                                    // status register write protect
#define WRITE_ENABLE             0x06
#define WRITE_DISABLE            0x04
#define ENABLE_WRITE_STATUS_REG  0x50
#define WRITE_STATUS_REGISTER    0x01

#define SET_BURST_WITH_WRAP      0x77
#define SET_BLOCK_PTR_PROTECTION 0x39                                    // not S25FL116K

#define PAGE_PROG                0x02
#define SECTOR_ERASE             0x20
#if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    #define HALF_BLOCK_ERASE     0x52
#endif
#define BLOCK_ERASE              0xd8
#define CHIP_ERASE               0x60                                    // also 0xc7

#define ERASE_PROGRAM_SUSPEND    0x75
#define ERASE_PROGRAM_RESUME     0x7a

#define READ_DATA_BYTES          0x03
#define FAST_READ                0x0b
#define FAST_READ_DUAL_OUTPUT    0x3b
#define FAST_READ_QUAD_OUTPUT    0x6b
#define FAST_READ_DUAL_IO        0xbb
#define FAST_READ_QUAD_IO        0xeb
#define CONTINUOUS_READ_RESET    0xff

#define SOFTWARE_RESET_ENABLE    0x66
#define SOFTWARE_RESET           0x99

#define DEEP_POWER_DOWN          0xb9
#define RELEASE_POWER_DOWN       0xab

#define DEVICE_ID                0x90
#define READ_JEDEC_ID            0x9f

#define READ_SECURITY_REGISTERS  0x48
#define ERASE_SECURITY_REGISTERS 0x44
#define PROG_SECURITY_REGISTERS  0x42

#if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    #define EN4B_MODE            0xb7                                    // enter 4 byte mode in order to be able to address more that 16MBytes
    #define EX4B_MODE            0xe9
#endif


#define MANUFACTURER_ID_MACRONIX 0xc2                                    // Macronix's manufacturer ID
#define SPI_FLASH_DEVICE_TYPE    0x20

#define DEVICE_ID_DATA_MX25L1645  0x14                                   // 16MBit / 2MegByte
#define DEVICE_ID_DATA_MX25L1606  0x15                                   // 16MBit / 2MegByte
#define DEVICE_ID_DATA_MX25L3245  0x16                                   // 32MBit / 4MegByte
#define DEVICE_ID_DATA_MX25L6445  0x17                                   // 64MBit / 8MegByte
#define DEVICE_ID_DATA_MX25L12845 0x18                                   // 128MBit / 16MegByte
#define DEVICE_ID_DATA_MX25L25635 0x19                                   // 256MBit / 32MegByte

#define DANGER_ERASING           0x80000000
#define DANGER_PROGRAMMING       0x40000000
#define WARNING_SUSPENDED        0x20000000


// SPI FLASH hardware interface
//
    #if defined SPI_FLASH_MULTIPLE_CHIPS
static void fnSPI_command_MX25L(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #else
static void fnSPI_command_MX25L(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
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
    int iEraseCommand = 0;
    MAX_FILE_LENGTH dummyWrites  = 0;
    MAX_FILE_LENGTH dataWrites   = 0;
    MAX_FILE_LENGTH discardCount = 0;

    FLUSH_SPI_FIFO_AND_FLAGS();                                          // ensure that the FIFOs are empty and the status flags are reset before starting

    if ((SPI_FLASH_Danger[iChipSelect] & (DANGER_PROGRAMMING | DANGER_ERASING)) != 0) { // check whether the chip is ready to work - if not, wait until the present internal operation has completed or suspend it
        volatile unsigned char ucStatus;
    #if defined SUPPORT_ERASE_SUSPEND
        unsigned long ulInitialState = SPI_FLASH_Danger[iChipSelect];    // backup the original state
    #endif
        SPI_FLASH_Danger[iChipSelect] = 0;                               // device will no longer be busy after continuing
        do {
            fnSPI_command_MX25L(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS &ucStatus, 1); // read busy status register
    #if defined SUPPORT_ERASE_SUSPEND
            if ((ucStatus & STATUS_BUSY) && (ulInitialState & DANGER_ERASING)) { // busy erasing a sector (we can temporarily interrupt if it is a read or program command for a different sector)
                if ((PAGE_PROG == ucCommand) || (READ_DATA_BYTES == ucCommand)) { // allow programming and read commands to interrupt erasures in progress
                    if ((ulPageNumberOffset & ~0xfff) != (SPI_FLASH_Danger[iChipSelect] & ~(DANGER_PROGRAMMING | DANGER_ERASING | 0xfff))) { // as long as the sector is a different one
                        fnSPI_command_MX25L(ERASE_PROGRAM_SUSPEND, 0, _EXTENDED_CS 0, 0); // command a suspend so that data can be read from or written to in any other sector (it takes 20us before the resume state is set, during which the busy state will continue to exist)
                        ulInitialState &= ~(DANGER_PROGRAMMING | DANGER_ERASING);
                        SPI_FLASH_Danger[iChipSelect] = (WARNING_SUSPENDED | ulInitialState); // mark that there is an suspended operation
                    }
                }
            }
    #endif
        } while ((ucStatus & STATUS_WIP) != 0);                          // until no longer busy
    }

    #if defined SET_SPI_FLASH_MODE
    SET_SPI_FLASH_MODE();
    #endif

    #if !defined DSPI_SPI || defined MANUAL_FLASH_CS_CONTROL
    ASSERT_CS_LINE(ulChipSelectLine);                                    // assert the chip select line
    #endif

    switch (ucCommand) {
    case SECTOR_ERASE:                                                   // 4k sector to be deleted (50ms required
    #if defined HALF_BLOCK_ERASE
    case HALF_BLOCK_ERASE:                                               // 32k half-block to be deleted (150ms required)
    #endif
    case BLOCK_ERASE:                                                    // 64k block to be deleted (500ms required)
        iEraseCommand = 1;
    case PAGE_PROG:                                                      // 1 to 256 bytes to be programmed to a page (700us required for 256 bytes)
    #if defined SUPPORT_ERASE_SUSPEND
        if ((SPI_FLASH_Danger[iChipSelect] & WARNING_SUSPENDED) == 0) {
            SPI_FLASH_Danger[iChipSelect] = ((DANGER_PROGRAMMING << iEraseCommand) | ulPageNumberOffset); // a write/erase will be started so we need to poll the status before next command
        }
    #else
        SPI_FLASH_Danger[iChipSelect] = DANGER_PROGRAMMING;              // mark that there is an operation in progress that will require time to complete
    #endif
        dataWrites = discardCount = DataLength;
    case DEVICE_ID:
    case READ_DATA_BYTES:                                                // 50MHz read - first setting the address and then reading the defined amount of data bytes
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
    #endif
        if ((DataLength != 0) || (iEraseCommand != 0)) {
    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)                             // 4 byte addressing
            WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset >> 24));   // write parameters
        #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
        #endif
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
    #endif
            WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset >> 16));   // write parameters
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
    #endif
            WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset >> 8));    // send page number offset
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
            CLEAR_RECEPTION_FLAG();                                      // clear the receive flag
            discardCount += 1;
    #else
            discardCount += 4;
    #endif
            if (DataLength == 0) {
                WRITE_SPI_CMD0_LAST((unsigned char)(ulPageNumberOffset));// send page number offset
            }
            else {
                WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset));     // send page number offset
            }
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
            if (dataWrites == 0) {
                dummyWrites = DataLength;                                // since we are reading we perform only dummy writes
            }
        }
        else {                                                           // no address is required in this case since we are in an AAI sequence
    #if defined SST25_A_VERSION
            WRITE_SPI_CMD0_LAST(*ucData);                                // always a single byte for A types
            discardCount = 2;
    #else
            WRITE_SPI_CMD0(*ucData++);                                   // always a write pair performed
        #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
        #endif
        #if (SPI_FLASH_FIFO_DEPTH < 2)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
            CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        #endif
            WRITE_SPI_CMD0_LAST(*ucData);
            discardCount = 3;
    #endif
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
        }
        break;

    case READ_JEDEC_ID:                                                  // read identification sequence (0x9f)
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
    #else
        discardCount = 1;
    #endif
        dummyWrites = 2;
        WRITE_SPI_CMD0(0xff);                                            // ensure transmit FIFO has more than one byte in it
        break;

    case READ_STATUS_REGISTER_1:                                         // read single byte from status register (0x05)
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
    #else 
        discardCount = 1;
    #endif
        WRITE_SPI_CMD0_LAST(0xff);                                       // ensure transmit FIFO has more than one byte in it
        break;

    #if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
    case EN4B_MODE:                                                      // enable 4 byte mode
    #endif
    case ERASE_PROGRAM_SUSPEND:
    case CHIP_ERASE:                                                     // command without further data
    case WRITE_DISABLE:
    case WRITE_ENABLE:
    case ERASE_PROGRAM_RESUME:
    case ENABLE_WRITE_STATUS_REG:
        WRITE_SPI_CMD0_LAST(ucCommand);                                  // write single command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
        discardCount = 1;
        break;

    case WRITE_STATUS_REGISTER:
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if (SPI_FLASH_FIFO_DEPTH < 2)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        discardCount = 1;
    #else
        discardCount = 2;
    #endif
        WRITE_SPI_CMD0_LAST(ucCommand);                                  // write new value
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
        DataLength = 0;
        break;
    }

    while (DataLength != 0) {
        WAIT_SPI_RECEPTION_END();                                        // wait until at least one byte is in the receive FIFO
        if (discardCount != 0) {
            (void)READ_SPI_FLASH_DATA();                                 // discard
            discardCount--;
        }
        else {
    #if defined _WINDOWS
            SPI_RX_BYTE = fnSimSPI_Flash(S25FL1_K_READ, (unsigned char)SPI_TX_BYTE); // simulate the SPI FLASH device
    #endif
            *ucData++ = READ_SPI_FLASH_DATA();                           // read the byte from the receive FIFO and save to the application buffer
            DataLength--;
        }
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        if (dummyWrites != 0) {                                          // dummy writes
            if (dummyWrites > 1) {
                WRITE_SPI_CMD0(0xff);                                    // write dummy
            }
            else {
                WRITE_SPI_CMD0_LAST(0xff);                               // write final dummy
            }
            dummyWrites--;
        }
        if (dataWrites != 0) {                                           // data writes
            if (dataWrites > 1) {
                WRITE_SPI_CMD0(*ucData++);                               // write data byte
            }
            else {
                WRITE_SPI_CMD0_LAST(*ucData++);                          // write final data byte
            }
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
            dataWrites--;
            DataLength--;
        }
    }
    while (discardCount != 0) {
        WAIT_SPI_RECEPTION_END();                                        // wait until at least one byte is in the receive FIFO
        (void)READ_SPI_FLASH_DATA();                                     // discard
        CLEAR_RECEPTION_FLAG();                                          // clear the receive flag
        discardCount--;
    }
    #if (!defined DSPI_SPI) || defined MANUAL_FLASH_CS_CONTROL
    NEGATE_CS_LINE(ulChipSelectLine);                                    // negate the chip select line
    #endif
    #if defined _WINDOWS
        #if defined DSPI_SPI && !defined MANUAL_FLASH_CS_CONTROL
    if ((SPI_TX_BYTE & SPI_PUSHR_EOQ) != 0) {                            // check that the CS has been negated
        SPI_TX_BYTE &= ~(ulChipSelectLine);
    }
        #endif
    fnSimSPI_Flash(S25FL1_K_CHECK_SS, 0);                                // simulate the SPI FLASH device
    #endif
    #if defined REMOVE_SPI_FLASH_MODE
    REMOVE_SPI_FLASH_MODE();
    #endif
}


// Check whether a known SPI FLASH device can be detected - called only once on start up
//
#if defined SPI_FLASH_MULTIPLE_CHIPS
static unsigned char fnCheckMX25L(int iChipSelect)
#else
static unsigned char fnCheckMX25L(void)
#endif
{
    volatile unsigned char ucID[3];
    unsigned char ucReturnType = NO_SPI_FLASH_AVAILABLE;

    #if defined SPI_FLASH_MULTIPLE_CHIPS
    if (iChipSelect == 0) {                                              // only on first device check
        fnDelayLoop(1000);                                               // 1ms start up delay to ensure SPI FLASH ready for erase/writes
    }
    #else
    fnDelayLoop(1000);                                                   // 1ms start up delay to ensure SPI FLASH ready for erase/writes
    #endif
    do {
        fnSPI_command_MX25L(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS ucID, 1); // read status register 1 to check that the device is not busy due to a reset during erasure
        if (ucID[0] == 0xff) {                                           // the status register will never contain 0xff and so this means that there is no device connected
            break;
        }
    } while ((ucID[0] & STATUS_WIP) != 0);                               // allow operation in progress to complete before reading the chip type
    fnSPI_command_MX25L(READ_JEDEC_ID, 0, __EXTENDED_CS ucID, sizeof(ucID));
    if ((ucID[0] == MANUFACTURER_ID_MACRONIX) && (ucID[1] == SPI_FLASH_DEVICE_TYPE)) { // Macronix memory part recognised
        switch (ucID[2]) {
        case DEVICE_ID_DATA_MX25L1645:                                   // 16Mbit/2Meg byte
            ucReturnType = MX25L1645;
            break;
        case DEVICE_ID_DATA_MX25L1606:                                   // 16Mbit/2Meg byte
            ucReturnType = MX25L1606;
            break;
        case DEVICE_ID_DATA_MX25L3245:                                   // 32Mbit/4Meg byte
            ucReturnType = MX25L3245;
            break;
        case DEVICE_ID_DATA_MX25L6445:                                   // 64Mbit/8Meg byte
            ucReturnType = MX25L6445;
            break;
        case DEVICE_ID_DATA_MX25L12845:                                  // 128Mbit/16Meg byte
            ucReturnType = MX25L12845;
            break;
        case DEVICE_ID_DATA_MX25L25635:
#if SPI_FLASH_SIZE >= (32 * 1024 * 1024)
            fnSPI_command_MX25L(EN4B_MODE, 0, __EXTENDED_CS ucID, 0);    // switch to 4 byte mode so that more that >16MByte can be addressed
#endif
            ucReturnType = MX25L25635;
            break;
        default:
            return NO_SPI_FLASH_AVAILABLE;                               // no device detected
        }
    }
    return ucReturnType;
}
#endif
#endif
