/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      spi_flash_s25fl1-k.h - Spansion
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2019
    *********************************************************************
    22.12.2015 Correction for KL interface                               {1}
    22.12.2015 Remove write enable outside of this file for KL/KE chip select compatibility {2}
    09.03.2019 Make generic for any processor

*/

#if defined SPI_FLASH_S25FL1_K

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
                                          while (SSPSR_X & SPISR_BSY) {SSPSR_X &= ~SPISR_BSY;}
        #else
            #define WAIT_TRANSFER_END()   while ((SSPSR_X & SPISR_TXE) == 0) {} \
                                          while (SSPSR_X & SPISR_BSY) {}
        #endif
        #define WAIT_SPI_RECEPTION_END() WAIT_TRANSFER_END()
        #if !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
    #elif defined _KINETIS
        #if defined KINETIS_KL
            #define SPI_FLASH_FIFO_DEPTH     1
        #elif !defined SPI_FLASH_FIFO_DEPTH
            #define SPI_FLASH_FIFO_DEPTH     1                           // if no fifo depth is specified we assume that it is 1
        #endif
    #endif
    #if defined SPI_FLASH_MULTIPLE_CHIPS
        #define __EXTENDED_CS     iChipSelect,
        static unsigned char fnCheckS25FL1_K(int iChipSelect);
        static const STORAGE_AREA_ENTRY spi_flash_storage = {
            (void *)&default_flash,                                      // link to internal flash
            (unsigned char *)(SPI_FLASH_START,                           // spi flash area start
            (unsigned char *)(SPI_FLASH_START + (SPI_DATA_FLASH_SIZE - 1)), // size of SPI flash area
            _STORAGE_SPI_FLASH,                                          // type
            SPI_FLASH_DEVICE_COUNT                                       // multiple devices
        };
    #else
        #define __EXTENDED_CS
        static unsigned char fnCheckS25FL1_K(void);
        static const STORAGE_AREA_ENTRY spi_flash_storage = {
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
#if defined _CHECK_SPI_CHIPS
        #if defined SPI_FLASH_MULTIPLE_CHIPS
    ucSPI_FLASH_Type[0] = fnCheckS25FL1_K(0);                            // flag whether the first SPI FLASH device is connected
        #else
    ucSPI_FLASH_Type[0] = fnCheckS25FL1_K();                             // flag whether the SPI FLASH device is connected
        #endif
    if (ucSPI_FLASH_Type[0] < S25FL116K) {                               // we expect at least this part to be available
        POWER_DOWN_SPI_FLASH_INTERFACE();                                // power down SPI 
    }
    else {
        #if defined SPI_FLASH_MULTIPLE_CHIPS                             // check for further devices
        int i = 0;
        while (++i < SPI_FLASH_DEVICE_COUNT) {
            ucSPI_FLASH_Type[i] = fnCheckS25FL1_K(i);
        }
        #endif
        UserStorageListPtr = (STORAGE_AREA_ENTRY *)&spi_flash_storage;   // insert spi flash as storage medium
    }
#endif




#if defined _SPI_FLASH_INTERFACE
// This is the main interface code to the S25FL1-K SPI FLASH device

/* =================================================================== */
/*                           S25FL1-K driver                           */
/* =================================================================== */

#define READ_STATUS_REGISTER_1   0x05
  #define STATUS_BUSY            0x01
  #define STATUS_WEL             0x02
  #define STATUS_BP0             0x04
  #define STATUS_BP1             0x08
  #define STATUS_BP2             0x10
  #define STATUS_TB              0x20
  #define STATUS_SEC             0x40
  #define STATUS_SRP0            0x80
#define READ_STATUS_REGISTER_2   0x35
  #define STATUS2_SUS            0x80
#define READ_STATUS_REGISTER_3   0x33
#define WRITE_ENABLE             0x06
#define WRITE_DISABLE            0x04
#define ENABLE_WRITE_STATUS_REG  0x50
#define WRITE_STATUS_REGISTER    0x01

#define SET_BURST_WITH_WRAP      0x77
#define SET_BLOCK_PTR_PROTECTION 0x39                                    // not S25FL116K

#define PAGE_PROG                0x02
#define SECTOR_ERASE             0x20
#define BLOCK_ERASE              0xd8
#define CHIP_ERASE               0xc7                                    // also 0x60

#define ERASE_PROGRAM_SUSPEND    0x75
#define ERASE_PROGRAM_RESUME     0x7a

#define READ_DATA_BYTES          0x03
#define FAST_READ                0x0B
#define FAST_READ_DUAL_OUTPUT    0x3B
#define FAST_READ_QUAD_OUTPUT    0x6B
#define FAST_READ_DUAL_IO        0xbB
#define FAST_READ_QUAD_IO        0xeB
#define CONTINUOUS_READ_RESET    0xff

#define SOFTWARE_RESET_ENABLE    0x66
#define SOFTWARE_RESET           0x99

#define DEEP_POWER_DOWN          0xb9
#define RELEASE_POWER_DOWN       0xab

#define DEVICE_ID                0x90
#define READ_JEDEC_ID            0x9f
#define READ_UNIQUE_ID           0x5a

#define READ_SECURITY_REGISTERS  0x48
#define ERASE_SECURITY_REGISTERS 0x44
#define PROG_SECURITY_REGISTERS  0x42


#define MANUFACTURER_ID_SPANSION 0x01                                    // Spansion's manufacturer ID
#define SPI_FLASH_DEVICE_TYPE    0x40

#define DEVICE_ID_DATA_S25FL116K 0x14                                    // 16MBit / 2MegByte
#define DEVICE_ID_DATA_S25FL132K 0x16                                    // 32MBit / 4MegByte
#define DEVICE_ID_DATA_S25FL164K 0x17                                    // 64MBit / 8MegByte

#define DANGER_ERASING           0x80000000
#define DANGER_PROGRAMMING       0x40000000
#define WARNING_SUSPENDED        0x20000000


// SPI FLASH hardware interface
//
    #if defined SPI_FLASH_MULTIPLE_CHIPS
static void fnSPI_command(unsigned char ucCommand, unsigned long ulPageNumberOffset, int iChipSelect, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
    #else
static void fnSPI_command(unsigned char ucCommand, unsigned long ulPageNumberOffset, volatile unsigned char *ucData, MAX_FILE_LENGTH DataLength)
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

    if (SPI_FLASH_Danger[iChipSelect] & (DANGER_PROGRAMMING | DANGER_ERASING)) { // check whether the chip is ready to work - if not, wait until the present internal operation has completed or suspend it
        volatile unsigned char ucStatus;
    #if defined SUPPORT_ERASE_SUSPEND
        unsigned long ulInitialState = SPI_FLASH_Danger[iChipSelect];    // backup the original state
    #endif
        SPI_FLASH_Danger[iChipSelect] = 0;                               // device will no longer be busy after continuing
        do {
            fnSPI_command(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS &ucStatus, 1); // read busy status register
    #if defined SUPPORT_ERASE_SUSPEND
            if ((ucStatus & STATUS_BUSY) && (ulInitialState & DANGER_ERASING)) { // busy erasing a sector (we can temporarily interrupt if it is a read or program command for a different sector)
                if ((PAGE_PROG == ucCommand) || (READ_DATA_BYTES == ucCommand)) { // allow programming and read commands to interrupt erasures in progress
                    if ((ulPageNumberOffset & ~0xfff) != (SPI_FLASH_Danger[iChipSelect] & ~(DANGER_PROGRAMMING | DANGER_ERASING | 0xfff))) { // as long as the sector is a different one
                        fnSPI_command(ERASE_PROGRAM_SUSPEND, 0, _EXTENDED_CS 0, 0); // command a suspend so that data can be read from or written to in any other sector (it takes 20us before the resume state is set, during which the busy state will continue to exist)
                        ulInitialState &= ~(DANGER_PROGRAMMING | DANGER_ERASING);
                        SPI_FLASH_Danger[iChipSelect] = (WARNING_SUSPENDED | ulInitialState); // mark that there is an suspended operation
                    }
                }
            }
    #endif
        } while ((ucStatus & STATUS_BUSY) != 0);                         // until no longer busy
    }

    #if defined SET_SPI_FLASH_MODE
    SET_SPI_FLASH_MODE();
    #endif

    #if defined KINETIS_KL || defined MANUAL_FLASH_CS_CONTROL
    ASSERT_CS_LINE(ulChipSelectLine);                                    // assert the chip select line
    #endif

    switch (ucCommand) {
    case SECTOR_ERASE:                                                   // 4k sector to be deleted (50ms required)
    case BLOCK_ERASE:                                                    // 64k block to be deleted (500ms required)
        iEraseCommand = 1;
    case PAGE_PROG:                                                      // 1 to 256 bytes to be programmed to a page (700us required for 256 bytes)
      //fnSPI_command(WRITE_ENABLE, 0, _EXTENDED_CS 0, 0);               // {2} first execute a write enable so that the erase/program operation can be executed
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
    #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
    #endif
        if ((DataLength != 0) || (iEraseCommand != 0)) {
            WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset >> 16));   // write parameters
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
	#if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
    #endif
            WRITE_SPI_CMD0((unsigned char)(ulPageNumberOffset >> 8));    // send page number offset
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
    #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
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
        #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
            WAIT_SPI_RECEPTION_END();                                    // wait until the command has been sent
            (void)READ_SPI_FLASH_DATA();                                 // discard the received byte
        #endif
            WRITE_SPI_CMD0_LAST(*ucData);
            discardCount = 3;
    #endif
    #if defined _WINDOWS
            fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);  // simulate the SPI FLASH device
    #endif
        }
        break;

    case READ_JEDEC_ID:
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
    #else
        discardCount = 1;
    #endif
        dummyWrites = 2;
        WRITE_SPI_CMD0(0xff);                                            // ensure transmit FIFO has more than one byte in it
        break;

    case READ_STATUS_REGISTER_2:
    case READ_STATUS_REGISTER_3:
    case READ_STATUS_REGISTER_1:                                         // read single byte from status register
        WRITE_SPI_CMD0(ucCommand);                                       // write command byte
    #if defined _WINDOWS
        fnSimSPI_Flash(S25FL1_K_WRITE, (unsigned char)SPI_TX_BYTE);      // simulate the SPI FLASH device
    #endif
    #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
    #else                                                                // {1}
        discardCount = 1;
    #endif
        WRITE_SPI_CMD0_LAST(0xff);                                       // ensure transmit FIFO has more than one byte in it
        break;

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
    #if defined SPI_FLASH_FIFO_DEPTH && (defined SPI_FLASH_FIFO_DEPTH == 1)
        WAIT_SPI_RECEPTION_END();                                        // wait until the command has been sent
        (void)READ_SPI_FLASH_DATA();                                     // discard the received byte
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
        	*ucData++ = READ_SPI_FLASH_DATA();                               // discard
        	*ucData = READ_SPI_FLASH_DATA();
        	*ucData-- = READ_SPI_FLASH_DATA();
        	*ucData = READ_SPI_FLASH_DATA();
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
    #if defined KINETIS_KL || defined MANUAL_FLASH_CS_CONTROL
    NEGATE_CS_LINE(ulChipSelectLine);                                    // negate the chip select line
    #endif
    #if defined _WINDOWS
        #if defined _KINETIS && !defined KINETIS_KL
    if (SPI_TX_BYTE & SPI_PUSHR_EOQ) {                                   // check that the CS has been negated
        SPI_TX_BYTE &= ~(ulChipSelectLine);
    }
        #endif
    fnSimSPI_Flash(S25FL1_K_CHECK_SS, 0);                                // simulate the SPI FLASH device
    #endif
    #if defined REMOVE_SPI_FLASH_MODE
    REMOVE_SPI_FLASH_MODE();
    #endif
    #if defined SUPPORT_ERASE_SUSPEND
    if ((SPI_FLASH_Danger[iChipSelect] & WARNING_SUSPENDED) && ((PAGE_PROG == ucCommand) || (READ_DATA_BYTES == ucCommand))) { // if the present operation caused an erase suspend to take place
        unsigned char ucStatus;
        SPI_FLASH_Danger[iChipSelect] &= ~(DANGER_PROGRAMMING | WARNING_SUSPENDED); // programming will terminated here and the suspend stte will be exited
        if (PAGE_PROG == ucCommand) {                                    // if the suspend was due to a write (rather than a read)
            do {
                fnSPI_command(READ_STATUS_REGISTER_1, 0, _EXTENDED_CS &ucStatus, 1); // wait until the present programming terminates so that the resume can be executed
            } while (ucStatus & STATUS_BUSY);                            // wait until no longer busy
        }
        fnSPI_command(READ_STATUS_REGISTER_2, 0, _EXTENDED_CS &ucStatus, 1); // check the suspend state to ensure that the suspended operation hasn't completed
        if (ucStatus & STATUS2_SUS) {                                    // if the device is indeed in the suspended state
            fnSPI_command(ERASE_PROGRAM_RESUME, 0, _EXTENDED_CS 0, 0);   // command a resume of the interrupted erase operation
            SPI_FLASH_Danger[iChipSelect] |= DANGER_ERASING;             // erasure in progress again
            // Warning: a suspend should not be commanded again within 20us of this resume
            //
            fnDelayLoop(20);                                             // this delay ensures that there is no risk involved
        }
//_TOGGLE_PORT(D, (PORTD_BIT0));
    }
    #endif
}



// Check whether a known SPI FLASH device can be detected - called only once on start up
//
#if defined SPI_FLASH_MULTIPLE_CHIPS
static unsigned char fnCheckS25FL1_K(int iChipSelect)
#else
static unsigned char fnCheckS25FL1_K(void)
#endif
{
    volatile unsigned char ucID[3];
    unsigned char ucReturnType = NO_SPI_FLASH_AVAILABLE;

    #if defined SPI_FLASH_MULTIPLE_CHIPS
    if (iChipSelect == 0) {                                              // only on first device check
        fnDelayLoop(12000);                                              // 12ms start up delay to ensure SPI FLASH ready for erase/writes
    }
    #else
    fnDelayLoop(12000);                                                  // 12ms start up delay to ensure SPI FLASH ready for erase/writes
    #endif
    do {
        fnSPI_command(READ_STATUS_REGISTER_1, 0, __EXTENDED_CS ucID, 1); // read status register 1 to check that the device is not busy due to a reset during erasure
        if (ucID[0] == 0xff) {                                           // the status register will never contain 0xff and so this means that there is no device connected
            break;
        }
    } while ((ucID[0] & STATUS_BUSY) != 0);                              // allow operation in progress to complete before reading the chip type
    fnSPI_command(READ_JEDEC_ID, 0, __EXTENDED_CS ucID, sizeof(ucID));

    //@modify our part id is 01 02 15 4D
    if((ucID[0]==MANUFACTURER_ID_SPANSION) && (ucID[1]==0x2) && (ucID[2]==0x15))
    {
    	ucReturnType = S25FL164K;//DEVICE_ID_DATA_S25FL132K; //S25FL164K is for the release part
    }else
    if ((ucID[0] == MANUFACTURER_ID_SPANSION) && (ucID[1] == SPI_FLASH_DEVICE_TYPE)) { // Spansion memory part recognised
        switch (ucID[2]) {
        case DEVICE_ID_DATA_S25FL116K:                                   // 16Mbit/2Meg byte
            ucReturnType = S25FL116K;
            break;
        case DEVICE_ID_DATA_S25FL132K:                                   // 32Mbit/4Meg byte
            ucReturnType = S25FL132K;
            break;
        case DEVICE_ID_DATA_S25FL164K:                                   // 64Mbit/8Meg byte
            ucReturnType = S25FL164K;
            break;
        default:
            return NO_SPI_FLASH_AVAILABLE;                               // no device detected
        }
    }
    #if defined SUPPORT_ERASE_SUSPEND
    fnSPI_command(READ_STATUS_REGISTER_2, 0, _EXTENDED_CS ucID, 1);      // read status register 2
    if (ucID[0] & STATUS2_SUS) {                                         // if we detect that the device is in a suspended state it means that there was a reset after putting it to this mode
        fnSPI_command(ERASE_PROGRAM_RESUME, 0, _EXTENDED_CS 0, 0);       // command a resume
        do {
            fnSPI_command(READ_STATUS_REGISTER_1, 0, _EXTENDED_CS ucID, 1); // read status register 1 to check that the device is not busy due to a reset during erasure
        } while (ucID[0] & STATUS_BUSY);                                 // allow suspended operation to complete before continuing    
    }
    #endif
    return ucReturnType;
}
#endif

#endif
