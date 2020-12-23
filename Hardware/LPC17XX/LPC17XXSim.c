/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:   	    LPC17XXSim.c
    Project: 	Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2013
    *********************************************************************
    25.10.2011 Add LPC1788
    06.10.2012 Add SUPPORT_TOUCH_SCREEN                                  {1}

*/  
                          

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"

#ifdef _LPC17XX


/**************************************************************************************************/
/********************************* NXP LPC17XX ****************************************************/
/**************************************************************************************************/

// A copy of the LPC17XX peripheral registers for simulation purposes
//
extern LPC17XX_PERIPH  ucLPC17xx = {0};

#ifdef LPC1788                                                           // temporary
    #define PCLKSEL0             0
    #define PCLKSEL1             0
    #define PCLK_TIMER0_CCLK_1   1
    #define PCLK_TIMER0_CCLK_2   2
    #define PCLK_TIMER0_CCLK_4   0
    #define PCLK_TIMER0_CCLK_8   3
    #define PCLK_TIMER1_CCLK_1   1
    #define PCLK_TIMER1_CCLK_2   2
    #define PCLK_TIMER1_CCLK_4   0
    #define PCLK_TIMER1_CCLK_8   3
    #define PCLK_TIMER2_CCLK_1   1
    #define PCLK_TIMER2_CCLK_2   2
    #define PCLK_TIMER2_CCLK_4   0
    #define PCLK_TIMER2_CCLK_8   3
    #define PCLK_TIMER3_CCLK_1   1
    #define PCLK_TIMER3_CCLK_2   2
    #define PCLK_TIMER3_CCLK_4   0
    #define PCLK_TIMER3_CCLK_8   3
    #define PCLK_WDT_CCLK_1      1
    #define PCLK_WDT_CCLK_2      2
    #define PCLK_WDT_CCLK_4      0
    #define PCLK_WDT_CCLK_8      3
#endif

unsigned long vector_ram[(sizeof(VECTOR_TABLE))/sizeof(unsigned long)]; // long word aligned

static unsigned long ulPort_in_0, ulPort_in_1, ulPort_in_2, ulPort_in_3, ulPort_in_4, ulPort_in_5;

static unsigned char ucPortFunctions[PORTS_AVAILABLE][PORT_WIDTH] = {0};

static int iFlagRefresh = 0;

#ifdef USB_INTERFACE                                                     // {14}
    static unsigned long ulEndpointInt = 0;    
    static int iRxDataBank[NUMBER_OF_USB_ENDPOINTS] = {0};
    static unsigned long ulTxDataBank[NUMBER_OF_USB_ENDPOINTS] = {0};
    static int iTxDataToggle[NUMBER_OF_USB_ENDPOINTS] = {0};
    static unsigned char ucUSB_address = DEV_EN;                         // USB device address defaults to 0x00 and enabled after reset
#endif

// Initialisation of all non-zero registers in the device
//
static void fnSetDevice(unsigned long *port_inits)
{
    extern void fnEnterHW_table(void *hw_table);
#ifdef LPC1788
    Matrix_Arb = 0x0000000d;
  //PBOOST = BOOST;                                                      // power boost is enabled by default according to the user's manual but this doesn't seem to be the case
    PCLKSEL = 0x00000004;                                                // PCLK is divided by 2 by default
    PCONP = (PCI2C2 | PCI2C1 | PCGPIO | PCRTC | PCI2C0 | PCUART1 | PCUART0 | PCTIM0 | PCTIM1);
    CCLKCFG = 0x00000001;

    IOCON_P0_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_01 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_02 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_03 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_04 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_05 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_06 = IOCON_TYPE_D_DEFAULT;

    IOCON_P0_07 = IOCON_TYPE_W_DEFAULT;
    IOCON_P0_08 = IOCON_TYPE_W_DEFAULT;
    IOCON_P0_09 = IOCON_TYPE_W_DEFAULT;

    IOCON_P0_10 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_11 = IOCON_TYPE_D_DEFAULT;

    IOCON_P0_12 = IOCON_TYPE_A_DEFAULT;
    IOCON_P0_13 = IOCON_TYPE_A_DEFAULT;

    IOCON_P0_14 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_15 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_16 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_17 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_18 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_19 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_20 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_21 = IOCON_TYPE_D_DEFAULT;
    IOCON_P0_22 = IOCON_TYPE_D_DEFAULT;

    IOCON_P0_23 = IOCON_TYPE_A_DEFAULT;
    IOCON_P0_24 = IOCON_TYPE_A_DEFAULT;
    IOCON_P0_25 = IOCON_TYPE_A_DEFAULT;
    IOCON_P0_26 = IOCON_TYPE_A_DEFAULT;

    IOCON_P1_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_01 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_02 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_03 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_04 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_05 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_06 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_07 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_08 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_09 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_10 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_11 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_12 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_13 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_14 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_15 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_16 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_17 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_18 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_19 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_20 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_21 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_22 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_23 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_24 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_25 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_26 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_27 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_28 = IOCON_TYPE_D_DEFAULT;
    IOCON_P1_29 = IOCON_TYPE_D_DEFAULT;

    IOCON_P1_30 = IOCON_TYPE_A_DEFAULT;
    IOCON_P1_31 = IOCON_TYPE_A_DEFAULT;

    IOCON_P2_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_01 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_02 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_03 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_04 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_05 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_06 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_07 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_08 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_09 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_10 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_11 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_12 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_13 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_14 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_15 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_16 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_17 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_18 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_19 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_20 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_21 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_22 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_23 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_24 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_25 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_26 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_27 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_28 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_29 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_30 = IOCON_TYPE_D_DEFAULT;
    IOCON_P2_31 = IOCON_TYPE_D_DEFAULT;

    IOCON_P3_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_01 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_02 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_03 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_04 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_05 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_06 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_07 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_08 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_09 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_10 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_11 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_12 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_13 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_14 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_15 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_16 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_17 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_18 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_19 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_20 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_21 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_22 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_23 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_24 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_25 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_26 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_27 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_28 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_29 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_30 = IOCON_TYPE_D_DEFAULT;
    IOCON_P3_31 = IOCON_TYPE_D_DEFAULT;

    IOCON_P4_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_01 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_02 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_03 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_04 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_05 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_06 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_07 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_08 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_09 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_10 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_11 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_12 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_13 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_14 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_15 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_16 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_17 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_18 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_19 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_20 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_21 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_22 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_23 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_24 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_25 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_26 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_27 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_28 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_29 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_30 = IOCON_TYPE_D_DEFAULT;
    IOCON_P4_31 = IOCON_TYPE_D_DEFAULT;

    IOCON_P5_00 = IOCON_TYPE_D_DEFAULT;
    IOCON_P5_01 = IOCON_TYPE_D_DEFAULT;

    IOCON_P5_04 = IOCON_TYPE_D_DEFAULT;
#else
    PCONP = (PCSPI | PCRTC | PCI2C0 | PCUART1 | PCUART0 | PCTIM1 | PCTIM0);
    IRCTRIM = 0xa0;
#endif
    RSID = POR;

	I2C0STAT = I2C_IDLE_STATE;                                           // I2C Controller
	I2C1STAT = I2C_IDLE_STATE;
	I2C2STAT = I2C_IDLE_STATE;
	I2C0SCLH = 0x04;
	I2C1SCLH = 0x04;
	I2C2SCLH = 0x04;
	I2C0SCLL = 0x04;
	I2C1SCLL = 0x04;
	I2C2SCLL = 0x04;

	FLASHCFG = (FLASHCFG_FIXED | FLASHTIM_6_CLOCKS);

    FIO0PIN = ulPort_in_0 = *port_inits++;                               // set port inputs to default states
    FIO1PIN = ulPort_in_1 = *port_inits++;
    FIO2PIN = ulPort_in_2 = *port_inits++;
    FIO3PIN = ulPort_in_3 = *port_inits++;
    FIO4PIN = ulPort_in_4 = *port_inits++;
#ifdef LPC1788
    FIO5PIN = ulPort_in_5 = *port_inits;
#endif

    RTC_AUX = 0x8;

    WDTC = 0xff;                                                         // watchdog
    WDTV = 0xff;
    WDFEED = 0x12345678;                                                 // read only register - value used by simulator

    CLRT = 0x370f;                                                       // Ethernet Controller
    MAXF = 0x600;
    MAC1 = (SOFT_RESET);
    EMAC_MODULE_ID = OLD_MAC_MODULE_ID;

    U0IIR = U1IIR = U2IIR = U3IIR = 0x01;                                // UART
    U0LSR = U1LSR = U2LSR = U3LSR = 0x60;
    U0TER = U1TER = U2TER = U3TER = 0x80;
    U0FDR = U1FDR = U2FDR = U3FDR = 0x10;

    RICOMPVAL = 0xffffffff;                                              // RIT
    RICTRL    = (RITENBR | RITEN);

    USBIntSt = 0x80000000;                                               // USB device
    USBDevIntSt = 0x00000010;
    USBReEp = 0x00000003;
    USBMaxPSize = 0x00000008;

    fnEnterHW_table(ucPortFunctions);
}


unsigned char ucFLASH[SIZE_OF_FLASH];

extern void fnInitialiseDevice(void *port_inits)
{
    uMemset(ucFLASH, 0xff, sizeof(ucFLASH));                             // we start with deleted FLASH memory contents
    fnPrimeFileSystem();                                                 // the project can then set parameters or files as required
    fnSetDevice((unsigned long *)port_inits);                            // set device registers to startup condition (if not zerod)
}

extern void fnDeleteFlashSector(unsigned char *ptrSector)
{
	int i = FLASH_GRANULARITY;

	while (i--) {
		*ptrSector++ = 0xff;
	}
}


extern unsigned char *fnGetEEPROM(unsigned short usOffset);

extern unsigned char *fnGetFileSystemStart(int iMemory)
{
#if defined SPI_FILE_SYSTEM && !defined FLASH_FILE_SYSTEM
    return (fnGetEEPROM(uFILE_START));
#elif defined (SAVE_COMPLETE_FLASH)
    return (&ucFLASH[0]);
#else
    return (&ucFLASH[uFILE_START-FLASH_START_ADDRESS]);
#endif
}


extern unsigned long fnGetFlashSize(int iMemory)
{
#if defined SPI_FILE_SYSTEM && !defined FLASH_FILE_SYSTEM
    return (fnGetEEPROMSize());
#elif defined (SAVE_COMPLETE_FLASH)
    return (sizeof(ucFLASH));
#else
    return (FILE_SYSTEM_SIZE);
#endif
}

static int iSectorsSelected[30] = {0};

static unsigned long fnGetSector(unsigned long FlashAddress)
{
    FlashAddress -= FLASH_START_ADDRESS;
    if (FlashAddress < FLASH_SECTOR_16_OFFSET) {                         // one of the first small sectors
        return (FlashAddress/FLASH_GRANULARITY_SMALL);
    }
    else {                                                               // one of the large sectors
        FlashAddress -= FLASH_SECTOR_16_OFFSET;
        return ((FlashAddress/FLASH_GRANULARITY_LARGE) + 16);
    }
}

static unsigned char *fnGetSectorAdd(unsigned long ulStartSector)
{
    if (ulStartSector < 16) {                                            // one of the first small sectors
        return (unsigned char *)(FLASH_START_ADDRESS + FLASH_SECTOR_0_OFFSET + (ulStartSector * FLASH_GRANULARITY_SMALL));
    }
    else {                                                               // one of the large sectors
        return (unsigned char *)(FLASH_START_ADDRESS + FLASH_SECTOR_16_OFFSET + ((ulStartSector - 16) * FLASH_GRANULARITY_LARGE));
    }
}

static unsigned long fnGetSectorLength(unsigned long ulStartSector)
{
    if (ulStartSector < 16) {                                            // one of the first small sectors
        return (FLASH_GRANULARITY_SMALL);
    }
    else {                                                               // one of the middle large sectors
        return (FLASH_GRANULARITY_LARGE);
    }
}

extern void fnSimNXPFlash(unsigned long commands[], unsigned long results[])
{
    int iStartSector;
    int iEndSector;
    switch (commands[0]) {
    case FLASH_PREPARE_SECTOR_TO_WRITE:
        iStartSector = commands[1];
        iEndSector   = commands[2];
        results[0] = INVALID_SECTOR;
        while (iStartSector <= iEndSector) {
            iSectorsSelected[iStartSector] = 1;                          // mark that the sector has been prepared for the next operation            
            iStartSector++;
            results[0] = CMD_SUCCESS;
        }
        break;
    case FLASH_COPY_RAM_TO_FLASH:
        {
            unsigned char *ptrDestination = (unsigned char *)commands[1];
            unsigned char *ptrSource      = (unsigned char *)commands[2];
            unsigned long ulBytesToWrite  = commands[3];
            unsigned long ulSector;
            if (commands[4] != (MASTER_CLOCK/1000)) {
                 _EXCEPTION("Bad clock speed !!");
            }
            if ((CAST_POINTER_ARITHMETIC)ptrSource & 0x03) {
                results[0] = SRC_ADDR_ERROR;
                break;
            }
            if ((CAST_POINTER_ARITHMETIC)ptrDestination & 0x03) {
                results[0] = DST_ADDR_ERROR;
                break;
            }
            ulSector = fnGetSector((unsigned long)ptrDestination);
            if (ulSector >= 30) {
                results[0] = DST_ADDR_NOT_MAPPED;
                break;
            }
            if (iSectorsSelected[ulSector] == 0) {
                results[0] = SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION;
                break;                
            }
            ptrDestination = fnGetFlashAdd(ptrDestination);
            if ((ptrDestination < ucFLASH) || (ptrDestination >= &ucFLASH[SIZE_OF_FLASH])) {
                results[0] = DST_ADDR_NOT_MAPPED;
                break;
            }
            if ((ulBytesToWrite != 256) && (ulBytesToWrite != 512) && (ulBytesToWrite != 1024) && (ulBytesToWrite != 4096)) {
                results[0] = COUNT_ERROR;
                break;
            }
            while (ulBytesToWrite >= FLASH_LINE_SIZE) {                  // program to FLASH (change only bits from 1 to 0) a 16 byte line can not change individual bits
                int iLineLength = 0;
                int iOldLineBlank = 1;
                unsigned char ucOldLine[FLASH_LINE_SIZE];
                unsigned char ucNewLine[FLASH_LINE_SIZE];
                uMemcpy(ucNewLine, ptrSource, FLASH_LINE_SIZE);          // make a copy of old and new FLASH data
                uMemcpy(ucOldLine, ptrDestination, FLASH_LINE_SIZE);
                while (iLineLength < FLASH_LINE_SIZE) {
                    if (ucOldLine[iLineLength++] != 0xff) {
                        iOldLineBlank = 0;
                    }
                }
                if (iOldLineBlank == 0) {                                // check that we only write 1 to non-blank lines
                    iLineLength = 0;
                    while (iLineLength < FLASH_LINE_SIZE) {
                        if (ucNewLine[iLineLength++] != 0xff) {
                            _EXCEPTION("Writing non '1' to a non-blank line !!");
                        }
                    }
                }
                else {
                    uMemcpy(ptrDestination, ptrSource, FLASH_LINE_SIZE); // write the FLASH line - we have checked that only one write to a line tries to change bits to '0'
                }
                ulBytesToWrite -= FLASH_LINE_SIZE;
                ptrDestination += FLASH_LINE_SIZE;
                ptrSource += FLASH_LINE_SIZE;
            }         
            uMemset(iSectorsSelected, 0, sizeof(iSectorsSelected));
            results[0] = CMD_SUCCESS;
        }
        break;
    case FLASH_ERASE_SECTORS:
        {
            unsigned long ulStartSector = commands[1];
            unsigned long ulEndSector   = commands[2];
            if (commands[3] != (MASTER_CLOCK/1000)) {
                 _EXCEPTION("Bad clock speed !!");
            }
            if (ulStartSector >= 30) {
                results[0] = INVALID_SECTOR;
                break;
            }
            while (ulStartSector <= ulEndSector) {
                if (iSectorsSelected[ulStartSector] == 0) {
                    results[0] = SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION;
                    break; 
                }
                uMemset(fnGetFlashAdd(fnGetSectorAdd(ulStartSector)), 0xff, fnGetSectorLength(ulStartSector));
                ulStartSector++;
            }
            uMemset(iSectorsSelected, 0, sizeof(iSectorsSelected));
            results[0] = CMD_SUCCESS;
        }
        break;
    case FLASH_BLANK_CHECK_SECTORS:
        {
            unsigned long ulStartSector = commands[1];
            unsigned long ulEndSector   = commands[2];

            if (ulStartSector >= 30) {
                results[0] = INVALID_SECTOR;
                break;
            }
            while (ulStartSector <= ulEndSector) {
                unsigned char *PtrSectorAddress = fnGetSectorAdd(ulStartSector);
                unsigned long ulSectorSize = fnGetSectorLength(ulStartSector);
                while (ulSectorSize--) {
                    if (*PtrSectorAddress != 0xff) {
                        results[0] = SECTOR_NOT_BLANK;
                        results[1] = (unsigned long)(PtrSectorAddress - fnGetSectorAdd(ulStartSector)); // offset
                        results[2] = *(unsigned long *)((CAST_POINTER_ARITHMETIC)PtrSectorAddress & ~0x3);
                        break; 
                    }
                    PtrSectorAddress++;
                }               
                ulStartSector++;
            }
            results[0] = CMD_SUCCESS;
        }
        break;
    case FLASH_READ_PART_ID:
        results[1] = PART_IDENTIFICATION_NUMBER;                         // part identification number
        results[0] = CMD_SUCCESS;
        break;
    case FLASH_READ_BOOT_CODE_VERSION:
        results[1] = 0x00000101;
        results[0] = CMD_SUCCESS;
        break;
    case FLASH_COMPARE:                                                  // not supported becasue we do not simulate RAM
        results[0] = CMD_SUCCESS;
        break;
    case FLASH_REVOKE_ISP:
        break;
    default:
        _EXCEPTION("Invalid IAP command !!");
        break;
    }
}

// Transform physical access address to address in simulated FLASH
//
extern unsigned char *fnGetFlashAdd(unsigned char *ucAdd)
{
    unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)FLASH_START_ADDRESS;
    return ucAdd + sim_add;
}

extern unsigned char *fnPutFlashAdd(unsigned char *ucAdd)
{
    unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)START_OF_FLASH;
    return ucAdd - sim_add;
}

// Periodic tick (on timer 1) - dummy since timer simulation used now
//
extern void RealTimeInterrupt(void)
{
}

// Get the present state of a port - note that devices with Fast GPIO only simulate these
//
extern unsigned long fnGetPresentPortState(int portNr)
{
    portNr--;
    switch (portNr) {
    case _PORT0:
    return ((FIO0DIR & FIO0SET) | (~FIO0DIR & ulPort_in_0));
    case _PORT1:
    return ((FIO1DIR & FIO1SET) | (~FIO1DIR & ulPort_in_1));  
    case _PORT2:
    return ((FIO2DIR & FIO2SET) | (~FIO2DIR & ulPort_in_2));  
    case _PORT3:
    return ((FIO3DIR & FIO3SET) | (~FIO3DIR & ulPort_in_3));  
    case _PORT4:
    return ((FIO4DIR & FIO4SET) | (~FIO4DIR & ulPort_in_4));
#ifdef LPC1788
    case _PORT5:
    return ((FIO5DIR & FIO5SET) | (~FIO5DIR & ulPort_in_5));
#endif
    default:
        return 0;
    }
}

// Get the present state of a port direction - note that devices with Fast GPIO only simulate these
//
extern unsigned long fnGetPresentPortDir(int portNr)
{
    portNr--;
    switch (portNr) {
    case _PORT0:
    return (FIO0DIR);
    case _PORT1:
    return (FIO1DIR);
    case _PORT2:
    return (FIO2DIR);
    case _PORT3:
    return (FIO3DIR);
    case _PORT4:
    return (FIO4DIR);
#ifdef LPC1788
    case _PORT5:
    return (FIO5DIR);
#endif
    default:
        return 0;
    }
}

static unsigned short fnIsPer(unsigned long ucPinSelValue)
{
    unsigned long ulCheck = 0x00000003;
    unsigned short usPeripheral = 0;
    unsigned short usPeripheralBit = 0x0001;
    while (usPeripheralBit != 0) {
        if ((ucPinSelValue & ulCheck) != 0) {
            usPeripheral |= usPeripheralBit;                             // port has a peripheral function
        }
        usPeripheralBit <<= 1;
        ulCheck <<= 2;
    }
    return usPeripheral;
}

extern unsigned long fnGetPresentPortPeriph(int portNr)
{
    unsigned long ulPeripherals = 0;
#ifdef LPC1788
    unsigned long *ptrPortPins = IOCON_P0_ADD + ((portNr - 1) * (0x80/sizeof(unsigned long)));
    unsigned long ulBit = 0x00000001;
    int i;
    for (i = 0; i < 32; i++) {
        if (*ptrPortPins++ & IOCON_FUNCTION_MASK) {
            ulPeripherals |= ulBit;
        }
        ulBit <<= 1;
    }
    return (ulPeripherals);
#else   
    switch (--portNr) {
    case _PORT0:
        ulPeripherals = fnIsPer(PINSEL1);
        ulPeripherals <<= 16;
        ulPeripherals |= fnIsPer(PINSEL0);
        return (ulPeripherals);
    case _PORT1:
        ulPeripherals = fnIsPer(PINSEL3);
        ulPeripherals <<= 16;
        ulPeripherals |= fnIsPer(PINSEL2);
        return (ulPeripherals);
    case _PORT2:
        ulPeripherals = fnIsPer(PINSEL5);
        ulPeripherals <<= 16;
        ulPeripherals |= fnIsPer(PINSEL4);
        return (ulPeripherals);
    case _PORT3:
        ulPeripherals = fnIsPer(PINSEL7);
        ulPeripherals <<= 16;
        ulPeripherals |= fnIsPer(PINSEL6);
        return (ulPeripherals);
    case _PORT4:
        ulPeripherals = fnIsPer(PINSEL9);
        ulPeripherals <<= 16;
        ulPeripherals |= fnIsPer(PINSEL8);
        return (ulPeripherals);
    default:
        return 0;
    }
#endif
}


static void fnUpdatePeripheralFunction(int iPort, unsigned long ulPortFunction)
{
    unsigned long ulPortBit = 0x00000001;
    int i = 0;
#ifdef LPC1788
    unsigned long *ptrPortPins = IOCON_P0_ADD + (iPort * (0x80/sizeof(unsigned long)));
    while (ulPortFunction != 0) {
        if (ulPortFunction & ulPortBit) {
            ucPortFunctions[iPort][i] = (unsigned char)(*ptrPortPins & IOCON_FUNCTION_MASK);
            ulPortFunction &= ~ulPortBit;
        }
        i++;
        ulPortBit <<= 1;
        ptrPortPins++;
    }
#else
    unsigned long *ptrPinSel = (unsigned long *)PIN_CONNECT_BLOCK;
    unsigned short usShift = 0;

    ptrPinSel += (2*iPort);

    while (ulPortFunction != 0) {
        if (ulPortFunction & ulPortBit) {
            unsigned char ucFunction = (unsigned char)(0x03 & (*ptrPinSel >> usShift));
            ucPortFunctions[iPort][i] = ucFunction;
            ulPortFunction &= ~ulPortBit;
        }
        i++;
        ulPortBit <<= 1;
        usShift += 2;
        if (usShift >= 32) {
            usShift = 0;
            ptrPinSel++;
        }
    }
#endif
}





// See whether there has been a port change which should be displayed
//
extern int fnPortChanges(int iForce)
{
    static unsigned long ulPortDir0 = 0, ulPortDir1 = 0, ulPortDir2 = 0, ulPortDir3 = 0, ulPortDir4 = 0, ulPortDir5 = 0;
    static unsigned long ulPortVal0 = 0, ulPortVal1 = 0, ulPortVal2 = 0, ulPortVal3 = 0, ulPortVal4 = 0, ulPortVal5 = 0;
    static unsigned long ulPortFunction0 = 0, ulPortFunction1 = 0, ulPortFunction2 = 0, ulPortFunction3 = 0, ulPortFunction4 = 0, ulPortFunction5 = 0;
    unsigned long ulNewValue;
    unsigned long ulNewPortPer;
    int iRtn = iFlagRefresh;
    iFlagRefresh = 0;

    ulNewValue = fnGetPresentPortState(_PORT0 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT0 + 1);
    if ((ulPortDir0 != FIO0DIR) || (ulNewValue != ulPortVal0) || (ulNewPortPer != ulPortFunction0)) {
        ulPortDir0 = FIO0DIR;
        ulPortVal0 = ulNewValue;
        ulPortFunction0 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT0, ulPortFunction0);
        iRtn |= PORT_CHANGE;
    }
    ulNewValue = fnGetPresentPortState(_PORT1 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT1 + 1);
    if ((ulPortDir1 != FIO1DIR) || (ulNewValue != ulPortVal1) || (ulNewPortPer != ulPortFunction1)) {
        ulPortDir1 = FIO1DIR;
        ulPortVal1 = ulNewValue;
        ulPortFunction1 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT1, ulPortFunction1);
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT2 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT2 + 1);
    if ((ulPortDir2 != FIO2DIR) || (ulNewValue != ulPortVal2) || (ulNewPortPer != ulPortFunction2)) {
        ulPortDir2 = FIO2DIR;
        ulPortVal2 = ulNewValue;
        ulPortFunction2 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT2, ulPortFunction2);
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT3 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT3 + 1);
    if ((ulPortDir3 != FIO3DIR) || (ulNewValue != ulPortVal3) || (ulNewPortPer != ulPortFunction3)) {
        ulPortDir3 = FIO3DIR;
        ulPortVal3 = ulNewValue;
        ulPortFunction3 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT3, ulPortFunction3);
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT4 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT4 + 1);
    if ((ulPortDir4 != FIO4DIR) || (ulNewValue != ulPortVal4) || (ulNewPortPer != ulPortFunction4)) {
        ulPortDir4 = FIO4DIR;
        ulPortVal4 = ulNewValue;
        ulPortFunction4 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT4, ulPortFunction4);
        iRtn |= PORT_CHANGE;
    }
#ifdef LPC1788
    ulNewValue = fnGetPresentPortState(_PORT5 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT5 + 1);
    if ((ulPortDir5 != FIO5DIR) || (ulNewValue != ulPortVal5) || (ulNewPortPer != ulPortFunction5)) {
        ulPortDir5 = FIO5DIR;
        ulPortVal5 = ulNewValue;
        ulPortFunction5 = ulNewPortPer;
        fnUpdatePeripheralFunction(_PORT5, ulPortFunction5);
        iRtn |= PORT_CHANGE;
    }
#endif
    return iRtn;
}

// Handle port interrupts on input changes
//
static void fnPortInterrupt(int iPort, unsigned long ulChangedBit)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
    switch (iPort) {
    case _PORT0:
        if (ulChangedBit & IO0IntEnR & ulPort_in_0) {                    // rising edge on this rising-edge enabled bit
            IO0IntStatR |= (ulChangedBit & IO0IntEnR & ulPort_in_0);     // signal that an interrupt is pending
            IOIntStatus |= P0int;
        }
        if (ulChangedBit & IO0IntEnF & ~ulPort_in_0) {                   // falling edge on this falling-edge enabled bit
            IO0IntStatF |= (ulChangedBit & IO0IntEnF & ~ulPort_in_0);    // signal that an interrupt is pending
            IOIntStatus |= P0int;
        }        
        break;
    case _PORT2:
        if (ulChangedBit & IO2IntEnR & ulPort_in_2) {                    // rising edge on this rising-edge enabled bit
            IO2IntStatR |= (ulChangedBit & IO2IntEnR & ulPort_in_2);     // signal that an interrupt is pending
            IOIntStatus |= P2int;
        }
        if (ulChangedBit & IO2IntEnF & ~ulPort_in_2) {                   // falling edge on this falling-edge enabled bit
            IO2IntStatF |= (ulChangedBit & IO2IntEnF & ~ulPort_in_2);    // signal that an interrupt is pending
            IOIntStatus |= P2int;
        }
#ifndef LPC1788
        if (ulChangedBit & PORT2_BIT10) {                                // handle external interrupts on EINT0
            if ((PINSEL4 & PINSEL4_P2_10_RESET) == PINSEL4_P2_10_EINT0) {// configured as EINT0 pin
                if ((ulPort_in_2 & PORT2_BIT10) && (EXTPOLAR & EXTMODE & EXTPOLAR0)) { // positive edge sensitive and change detected
                    EXTINT |= EINT0;
                }
                else if ((!(ulPort_in_2 & PORT2_BIT10)) && (EXTMODE & EXTMODE0) && (!(EXTPOLAR & EXTPOLAR0))) {// negative edge sensitive and change detected
                    EXTINT |= EINT0;
                }
            }
            if (EXTINT & EINT0) {
                IRQ0_31_SPR |= (1 << irq_EINT0_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_EINT0_ID)) {                 // if edge interrupt interrupt is not disabled
                    ptrVect->processor_interrupts.irq_EINT0();           // call the interrupt handler
                }
            }
        }
        if (ulChangedBit & PORT2_BIT11) {                                // handle external interrupts on EINT1
            if ((PINSEL4 & PINSEL4_P2_11_RESET) == PINSEL4_P2_11_EINT1) {// configured as EINT1 pin
                if ((ulPort_in_2 & PORT2_BIT11) && (EXTPOLAR & EXTMODE & EXTPOLAR1)) { // positive edge sensitive and change detected
                    EXTINT |= EINT1;
                }
                else if ((!(ulPort_in_2 & PORT2_BIT11)) && (EXTMODE & EXTMODE1) && (!(EXTPOLAR & EXTPOLAR1))) {// negative edge sensitive and change detected
                    EXTINT |= EINT1;
                }
            }
            if (EXTINT & EINT1) {
                IRQ0_31_SPR |= (1 << irq_EINT1_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_EINT1_ID)) {                 // if edge interrupt interrupt is not disabled
                    ptrVect->processor_interrupts.irq_EINT1();           // call the interrupt handler
                }
            }
        }
        if (ulChangedBit & PORT2_BIT12) {                                // handle external interrupts on EINT2
            if ((PINSEL4 & PINSEL4_P2_12_RESET) == PINSEL4_P2_12_EINT2) {// configured as EINT2 pin
                if ((ulPort_in_2 & PORT2_BIT12) && (EXTPOLAR & EXTMODE & EXTPOLAR2)) { // positive edge sensitive and change detected
                    EXTINT |= EINT2;
                }
                else if ((!(ulPort_in_2 & PORT2_BIT12)) && (EXTMODE & EXTMODE2) && (!(EXTPOLAR & EXTPOLAR2))) {// negative edge sensitive and change detected
                    EXTINT |= EINT2;
                }
            }
            if (EXTINT & EINT2) {
                IRQ0_31_SPR |= (1 << irq_EINT2_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_EINT2_ID)) {                 // if edge interrupt interrupt is not disabled
                    ptrVect->processor_interrupts.irq_EINT2();           // call the interrupt handler
                }
            }
        }
        if (ulChangedBit & PORT2_BIT13) {                                // handle external interrupts on EINT3
            if ((PINSEL4 & PINSEL4_P2_13_RESET) == PINSEL4_P2_13_EINT3) {// configured as EINT3 pin
                if ((ulPort_in_2 & PORT2_BIT13) && (EXTPOLAR & EXTMODE & EXTPOLAR3)) { // positive edge sensitive and change detected
                    EXTINT |= EINT3;
                }
                else if ((!(ulPort_in_2 & PORT2_BIT13)) && (EXTMODE & EXTMODE3) && (!(EXTPOLAR & EXTPOLAR3))) {// negative edge sensitive and change detected
                    EXTINT |= EINT3;
                }
            }
        }
#endif
        break;
    }
    if ( 
        (EXTINT & EINT3) ||                                              // external interrupt 3 shares an interrupt with the GPO ports
        (IOIntStatus & (P0int | P2int))) {
        IRQ0_31_SPR |= (1 << irq_EINT3_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_EINT3_ID)) {                         // if edge interrupt interrupt is not disabled
            ptrVect->processor_interrupts.irq_EINT3();                   // call the interrupt handler
        }
    }
}

// Simulate setting, clearing or toggling of an input pin
//
extern void fnSimulateInputChange(unsigned char ucPort, unsigned char ucPortBit, int iChange)
{
    unsigned long ulBit = (0x80000000 >> ucPortBit);
    switch (ucPort) {
    case _PORT0:
        if (~FIO0DIR & ulBit) {
            unsigned long ulOriginal_port_state = ulPort_in_0;
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_0 ^= ulBit;
                FIO0PIN &= ~ulBit;
                FIO0PIN |= (ulPort_in_0 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_0 |= ulBit;
                FIO0PIN |= ulBit;
            }
            else {
                ulPort_in_0 &= ~ulBit;
                FIO0PIN &= ~ulBit;
            }
            if (ulPort_in_0 != ulOriginal_port_state) {
                fnPortInterrupt(_PORT0, ulBit);
            }
        }
        break;
    case _PORT1:
        if (~FIO1DIR & ulBit) {
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_1 ^= ulBit;
                FIO1PIN &= ~ulBit;
                FIO1PIN |= (ulPort_in_1 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_1 |= ulBit;
                FIO1PIN |= ulBit;
            }
            else {
                ulPort_in_1 &= ~ulBit;
                FIO1PIN &= ~ulBit;
            }
        }
        break;
    case _PORT2:
        {
            unsigned long ulOriginal_port_state = ulPort_in_2;
            if (!(~FIO2DIR & ulBit)) {                                   // accept input change if an input or an external interrupt
#ifndef LPC1788
                if ((ulBit == PORT2_BIT10) && ((PINSEL4 & PINSEL4_P2_10_RESET) == PINSEL4_P2_10_EINT0)) {
                }
                else if ((ulBit == PORT2_BIT11) && ((PINSEL4 & PINSEL4_P2_11_RESET) == PINSEL4_P2_11_EINT1)) {
                }
                else if ((ulBit == PORT2_BIT12) && ((PINSEL4 & PINSEL4_P2_12_RESET) == PINSEL4_P2_12_EINT2)) {
                }
                else if ((ulBit == PORT2_BIT13) && ((PINSEL4 & PINSEL4_P2_13_RESET) == PINSEL4_P2_13_EINT3)) {
                }
                else {
                    break;
                }
#endif
            }
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_2 ^= ulBit;
                FIO2PIN &= ~ulBit;
                FIO2PIN |= (ulPort_in_2 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_2 |= ulBit;
                FIO2PIN |= ulBit;
            }
            else {
                ulPort_in_2 &= ~ulBit;
                FIO2PIN &= ~ulBit;
            }
            if (ulPort_in_2 != ulOriginal_port_state) {
                fnPortInterrupt(_PORT2, ulBit);
            }
        }
        break;
    case _PORT3:
        if (~FIO3DIR & ulBit) {
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_3 ^= ulBit;
                FIO3PIN &= ~ulBit;
                FIO3PIN |= (ulPort_in_3 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_3 |= ulBit;
                FIO3PIN |= ulBit;
            }
            else {
                ulPort_in_3 &= ~ulBit;
                FIO3PIN &= ~ulBit;
            }
        }
        break;
    case _PORT4:
        if (~FIO4DIR & ulBit) {
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_4 ^= ulBit;
                FIO4PIN &= ~ulBit;
                FIO4PIN |= (ulPort_in_4 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_4 |= ulBit;
                FIO4PIN |= ulBit;
            }
            else {
                ulPort_in_4 &= ~ulBit;
                FIO4PIN &= ~ulBit;
            }
        }
        break;
#ifdef LPC1788
    case _PORT5:
        if (~FIO5DIR & ulBit) {
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_5 ^= ulBit;
                FIO5PIN &= ~ulBit;
                FIO5PIN |= (ulPort_in_5 & ulBit);
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_5 |= ulBit;
                FIO5PIN |= ulBit;
            }
            else {
                ulPort_in_5 &= ~ulBit;
                FIO5PIN &= ~ulBit;
            }
        }
        break;
#endif
    }
}

// Update ports based on present register settings
//
extern void fnSimPorts(void)
{
    static unsigned long ulOutput0 = 0, ulOutput1 = 0, ulOutput2 = 0, ulOutput3 = 0, ulOutput4 = 0, ulOutput5 = 0;
    unsigned long ulNewState;

    ulNewState = ulOutput0 & (~(FIO0CLR & ~FIO0MASK));
    FIO0CLR = 0;
    ulNewState |= (FIO0SET & ~ulOutput0 & ~FIO0MASK);
    FIO0SET = ulOutput0 = ulNewState;
    FIO0PIN = FIO0DIR & FIO0SET | (~FIO0DIR & ulPort_in_0);
    ulNewState = ulOutput1 & (~(FIO1CLR & ~FIO1MASK));
    FIO1CLR = 0;
    ulNewState |= (FIO1SET & ~ulOutput1 & ~FIO1MASK);
    FIO1SET = ulOutput1 = ulNewState;
    FIO1PIN = FIO1DIR & FIO1SET | (~FIO1DIR & ulPort_in_1);

    ulNewState = ulOutput2 & (~(FIO2CLR & ~FIO2MASK));
    FIO2CLR = 0;
    ulNewState |= (FIO2SET & ~ulOutput2 & ~FIO2MASK);
    FIO2SET = ulOutput2 = ulNewState;
    FIO2PIN = FIO2DIR & FIO2SET | (~FIO2DIR & ulPort_in_2);
    FIO2CLR = 0;

    ulNewState = ulOutput3 & (~(FIO3CLR & ~FIO3MASK));
    FIO3CLR = 0;
    ulNewState |= (FIO3SET & ~ulOutput3 & ~FIO3MASK);
    FIO3SET = ulOutput3 = ulNewState;
    FIO3PIN = FIO3DIR & FIO3SET | (~FIO3DIR & ulPort_in_3);

    ulNewState = ulOutput4 & (~(FIO4CLR & ~FIO4MASK));
    FIO4CLR = 0;
    ulNewState |= (FIO4SET & ~ulOutput4 & ~FIO4MASK);
    FIO4SET = ulOutput4 = ulNewState;
    FIO4PIN = FIO4DIR & FIO4SET | (~FIO4DIR & ulPort_in_4);

#ifdef LPC1788
    ulNewState = ulOutput5 & (~(FIO5CLR & ~FIO5MASK));
    FIO5CLR = 0;
    ulNewState |= (FIO5SET & ~ulOutput5 & ~FIO5MASK);
    FIO5SET = ulOutput5 = ulNewState;
    FIO5PIN = FIO5DIR & FIO5SET | (~FIO5DIR & ulPort_in_5);
#endif

#if CHIP_HAS_UARTS > 0 && defined RTS_0_PIN                              // RTS outputs
    if (RTS_0_PORT_DDR & RTS_0_PIN) {                                    // user defined RTS pin configured as output
        static unsigned char ucRTS0 = 1;
        if (RTS_0_PORT & RTS_0_PIN) {
            if (ucRTS0 == 0) {
                ucRTS0 = 1;
                fnConfigSimSCI(0, NEGATE_RTS_COM_0, 0);                  // negate control line
            }
        }
        else {
            if (ucRTS0 != 0) {
                ucRTS0 = 0;
                fnConfigSimSCI(0, ASSERT_RTS_COM_0, 0);                  // assert control line
            }
        }
    }
#endif
#if CHIP_HAS_UARTS > 1 && defined RTS_1_PIN                              // RTS outputs
    if (RTS_1_PORT_DDR & RTS_1_PIN) {                                    // user defined RTS pin configured as output
        static unsigned char ucRTS1 = 1;
        if (RTS_1_PORT & RTS_1_PIN) {
            if (ucRTS1 == 0) {
                ucRTS1 = 1;
                fnConfigSimSCI(1, NEGATE_RTS_COM_1, 0);                  // negate control line
            }
        }
        else {
            if (ucRTS1 != 0) {
                ucRTS1 = 0;
                fnConfigSimSCI(1, ASSERT_RTS_COM_1, 0);                  // assert control line
            }
        }
    }
#endif
#if CHIP_HAS_UARTS > 2 && defined RTS_2_PIN                              // RTS outputs
    if (RTS_2_PORT_DDR & RTS_2_PIN) {                                    // user defined RTS pin configured as output
        static unsigned char ucRTS2 = 1;
        if (RTS_2_PORT & RTS_2_PIN) {
            if (ucRTS2 == 0) {
                ucRTS2 = 1;
                fnConfigSimSCI(2, NEGATE_RTS_COM_2, 0);                  // negate control line
            }
        }
        else {
            if (ucRTS2 != 0) {
                ucRTS2 = 0;
                fnConfigSimSCI(2, ASSERT_RTS_COM_2, 0);                  // assert control line
            }
        }
    }
#endif
#if CHIP_HAS_UARTS > 3 && defined RTS_3_PIN                              // RTS outputs
    if (RTS_3_PORT_DDR & RTS_3_PIN) {                                    // user defined RTS pin configured as output
        static unsigned char ucRTS3 = 1;
        if (RTS_3_PORT & RTS_3_PIN) {
            if (ucRTS3 == 0) {
                ucRTS3 = 1;
                fnConfigSimSCI(3, NEGATE_RTS_COM_3, 0);                  // negate control line
            }
        }
        else {
            if (ucRTS3 != 0) {
                ucRTS3 = 0;
                fnConfigSimSCI(3, ASSERT_RTS_COM_3, 0);                  // assert control line
            }
        }
    }
#endif
}


// Simulate the reception of serial data by inserting bytes into the input buffer and calling interrupts
//
extern void fnSimulateSerialIn(int iPort, unsigned char *ptrDebugIn, unsigned short usLen)
{
#ifdef SERIAL_INTERFACE
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));

    switch (iPort) {
    case 0:
        if (U0FCR & FIFO_ENABLE) {
            U0IIR |= UART_FIFO_ENABLED;
        }
        else {
            return;
        }
        while (usLen--) {
            U0LSR |= UART_RDR;                                           // line status
            U0IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
            U0IIR |= UART_RDA_INT_PENDING;                               // set interrupt cause
	        U0RBR_THR = *ptrDebugIn++;                                   // put each character into the receive holding register
            IRQ0_31_SPR |= (1 << irq_UART0_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_UART0_ID)) {                     // if UART 0 interrupt is not disabled
                if (U0IER & UART_RDA_INT) {                              // if receive interrupt is enabled
                    ptrVect->processor_interrupts.irq_UART0();           // call the interrupt handler
                }
            }
        }
        break;
    case 1:
        if (U1FCR & FIFO_ENABLE) {
            U1IIR |= UART_FIFO_ENABLED;
        }
        else {
            return;
        }
        while (usLen--) {
            U1LSR |= UART_RDR;                                           // line status
            U1IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
            U1IIR |= UART_RDA_INT_PENDING;                               // set interrupt cause
	        U1RBR_THR = *ptrDebugIn++;                                   // put each character into the receive holding register
            IRQ0_31_SPR |= (1 << irq_UART1_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_UART1_ID)) {                     // if UART 1 interrupt is not disabled
                if (U1IER & UART_RDA_INT) {                              // if receive interrupt is enabled
                    ptrVect->processor_interrupts.irq_UART1();           // call the interrupt handler
                }
            }
        }
        break;
    case 2:
        if (U2FCR & FIFO_ENABLE) {
            U2IIR |= UART_FIFO_ENABLED;
        }
        else {
            return;
        }
        while (usLen--) {
            U2LSR |= UART_RDR;                                           // line status
            U2IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
            U2IIR |= UART_RDA_INT_PENDING;                               // set interrupt cause
	        U2RBR_THR = *ptrDebugIn++;                                   // put each character into the receive holding register
            IRQ0_31_SPR |= (1 << irq_UART2_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_UART2_ID)) {                     // if UART 2 interrupt is not disabled
                if (U2IER & UART_RDA_INT) {                              // if receive interrupt is enabled
                    ptrVect->processor_interrupts.irq_UART2();           // call the interrupt handler
                }
            }
        }
        break;
    case 3:
        if (U3FCR & FIFO_ENABLE) {
            U3IIR |= UART_FIFO_ENABLED;
        }
        else {
            return;
        }
        while (usLen--) {
            U3LSR |= UART_RDR;                                           // line status
            U3IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
            U3IIR |= UART_RDA_INT_PENDING;                               // set interrupt cause
	        U3RBR_THR = *ptrDebugIn++;                                   // put each character into the receive holding register
            IRQ0_31_SPR |= (1 << irq_UART3_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_UART3_ID)) {                     // if UART 3 interrupt is not disabled
                if (U3IER & UART_RDA_INT) {                              // if receive interrupt is enabled
                    ptrVect->processor_interrupts.irq_UART3();           // call the interrupt handler
                }
            }
        }
        break;
    #ifdef LPC1788
    case 4:
        if (U4FCR & FIFO_ENABLE) {
            U4IIR |= UART_FIFO_ENABLED;
        }
        else {
            return;
        }
        while (usLen--) {
            U4LSR |= UART_RDR;                                           // line status
            U4IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
            U4IIR |= UART_RDA_INT_PENDING;                               // set interrupt cause
	        U4RBR_THR = *ptrDebugIn++;                                   // put each character into the receive holding register
            IRQ32_63_SPR |= (1 << (irq_UART4_ID - 32));
            IRQ32_63_CPR = IRQ32_63_SPR;
            if (IRQ32_63_SER & (1 << (irq_UART4_ID - 32))) {             // if UART 4 interrupt is not disabled
                if (U4IER & UART_RDA_INT) {                              // if receive interrupt is enabled
                    ptrVect->processor_interrupts.irq_UART4();           // call the interrupt handler
                }
            }
        }
        break;
    #endif
    }
#endif
}


static void fnUART_Tx_int(int iChannel)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
    switch (iChannel) {
    case 0:
        U0LSR |= UART_THRE;                                              // line status
        U0IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
        U0IIR |= UART_THRE_INT_PENDING;                                  // set interrupt cause
        IRQ0_31_SPR |= (1 << irq_UART0_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_UART0_ID)) {                         // if UART 0 interrupt is not disabled
            if (U0IER & UART_THRE_INT) {                                 // if transmit interrupt is enabled
                ptrVect->processor_interrupts.irq_UART0();               // call the interrupt handler
            }
        } 
        break;
    case 1:
        U1LSR |= UART_THRE;                                              // line status
        U1IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
        U1IIR |= UART_THRE_INT_PENDING;                                  // set interrupt cause
        IRQ0_31_SPR |= (1 << irq_UART1_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_UART1_ID)) {                         // if UART 1 interrupt is not disabled
            if (U1IER & UART_THRE_INT) {                                 // if transmit interrupt is enabled
                ptrVect->processor_interrupts.irq_UART1();               // call the interrupt handler
            }
        } 
        break;
    case 2:
        U2LSR |= UART_THRE;                                              // line status
        U2IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
        U2IIR |= UART_THRE_INT_PENDING;                                  // set interrupt cause
        IRQ0_31_SPR |= (1 << irq_UART2_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_UART2_ID)) {                         // if UART 2 interrupt is not disabled
            if (U2IER & UART_THRE_INT) {                                 // if transmit interrupt is enabled
                ptrVect->processor_interrupts.irq_UART2();               // call the interrupt handler
            }
        } 
        break;
    case 3:
        U3LSR |= UART_THRE;                                              // line status
        U3IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
        U3IIR |= UART_THRE_INT_PENDING;                                  // set interrupt cause
        IRQ0_31_SPR |= (1 << irq_UART3_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_UART3_ID)) {                         // if UART 3 interrupt is not disabled
            if (U3IER & UART_THRE_INT) {                                 // if transmit interrupt is enabled
                ptrVect->processor_interrupts.irq_UART3();               // call the interrupt handler
            }
        } 
        break;
    #ifdef LPC1788
    case 4:
        U4LSR |= UART_THRE;                                              // line status
        U4IIR &= ~(UART_NO_INT_PENDING | UART_INT_MASK);
        U4IIR |= UART_THRE_INT_PENDING;                                  // set interrupt cause
        IRQ32_63_SPR |= (1 << (irq_UART4_ID - 32));
        IRQ32_63_CPR = IRQ32_63_SPR;
        if (IRQ32_63_SER & (1 << (irq_UART4_ID - 32))) {                 // if UART 4 interrupt is not disabled
            if (U4IER & UART_THRE_INT) {                                 // if transmit interrupt is enabled
                ptrVect->processor_interrupts.irq_UART4();               // call the interrupt handler
            }
        } 
        break;
    #endif
    }
}

#ifdef IIC_INTERFACE
static void fnI2C_int(int iChannel)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
    switch (iChannel) {
    case 0:
        if (I2C0STAT != I2C_IDLE_STATE) {                                // if a state generating an interrupt
            IRQ0_31_SPR |= (1 << irq_I2C0_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_I2C0_ID)) {                      // if I2C interrupt is not disabled
                ptrVect->processor_interrupts.irq_I2C0();                // call the interrupt handler
            } 
        }
        break;
    case 1:
        if (I2C1STAT != I2C_IDLE_STATE) {                                // if a state generating an interrupt
            IRQ0_31_SPR |= (1 << irq_I2C1_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_I2C1_ID)) {                      // if I2C interrupt is not disabled
                ptrVect->processor_interrupts.irq_I2C1();                // call the interrupt handler
            } 
        }
        break;
    case 2:
        if (I2C2STAT != I2C_IDLE_STATE) {                                // if a state generating an interrupt
            IRQ0_31_SPR |= (1 << irq_I2C2_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_I2C2_ID)) {                      // if I2C interrupt is not disabled
                ptrVect->processor_interrupts.irq_I2C2();                // call the interrupt handler
            } 
        }
        break;
    }
}
#endif

// Process simulated interrupts
//
extern unsigned long fnSimInts(char *argv[])
{
#ifdef SERIAL_INTERFACE
    extern unsigned char ucTxLast[NUMBER_SERIAL];
#endif
    unsigned long ulNewActions = 0;
    int *ptrCnt;

    if ((iInts & CHANNEL_0_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART0];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_0_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_0_SERIAL_INT;                          // interrupt has been handled
#ifdef SERIAL_INTERFACE
	            fnLogTx0(ucTxLast[0]);
                ulNewActions |= SEND_COM_0;
                fnUART_Tx_int(0);
#endif
            }
        }
	}

    if ((iInts & CHANNEL_1_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART1];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_1_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_1_SERIAL_INT;                          // interrupt has been handled
#ifdef SERIAL_INTERFACE
	            fnLogTx1(ucTxLast[1]);
                ulNewActions |= SEND_COM_1;
                fnUART_Tx_int(1);
#endif
            }
        }
	}

    if ((iInts & CHANNEL_2_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART2];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_2_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_2_SERIAL_INT;                          // interrupt has been handled
#ifdef SERIAL_INTERFACE
	            fnLogTx2(ucTxLast[2]);
                ulNewActions |= SEND_COM_2;
                fnUART_Tx_int(2);
#endif
            }
        }
	}

    if ((iInts & CHANNEL_3_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART3];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_3_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_3_SERIAL_INT;                          // interrupt has been handled
#ifdef SERIAL_INTERFACE
	            fnLogTx3(ucTxLast[3]);
                ulNewActions |= SEND_COM_3;
                fnUART_Tx_int(3);
#endif
            }
        }
	}

#ifdef LPC1788
    if ((iInts & CHANNEL_4_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART4];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_4_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_4_SERIAL_INT;                          // interrupt has been handled
    #ifdef SERIAL_INTERFACE
	            fnLogTx4(ucTxLast[4]);
                ulNewActions |= SEND_COM_4;
                fnUART_Tx_int(4);
    #endif
            }
        }
	}
#endif

    if ((iInts & IIC_INT0) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C0];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= IIC_INT0;                                      // enough IIC interupts handled in this tick period
            }
            else {
		        iInts &= ~IIC_INT0;                                      // interrupt has been handled
#ifdef IIC_INTERFACE
                fnI2C_int(0);
#endif
            }
        }
	}

    if ((iInts & IIC_INT1) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C1];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= IIC_INT1;                                      // enough IIC interupts handled in this tick period
            }
            else {
		        iInts &= ~IIC_INT1;                                      // interrupt has been handled
#ifdef IIC_INTERFACE
                fnI2C_int(1);
#endif
            }
        }
	}

    if ((iInts & IIC_INT2) && (argv)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C2];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= IIC_INT2;                                      // enough IIC interupts handled in this tick period
            }
            else {
		        iInts &= ~IIC_INT2;                                      // interrupt has been handled
#ifdef IIC_INTERFACE
                fnI2C_int(2);
#endif
            }
        }
	}
#ifdef USB_INTERFACE                                                     // {14}
    if (iInts & USB_INT) {
        int iEndpoint = 0;
        iInts &= ~USB_INT;
        while ((iEndpoint < NUMBER_OF_USB_ENDPOINTS) && (ulEndpointInt != 0)) {
            if (ulEndpointInt & (1 << iEndpoint)) {
                ulEndpointInt &= ~(1 << iEndpoint);
                fnCheckUSBOut(0, iEndpoint);
            }
            iEndpoint++;
        }
    }
#endif
    return ulNewActions;
}

extern void fnSimulateModemChange(int iPort, unsigned long ulNewState, unsigned long ulOldState)
{
// Note that the modem status bits are according to the MS specifications as passed by GetCommModemStatus().
// To avoid including MS headers, the bits are defined here - it is not expected that they will ever change...
#define MS_CTS_ON  0x0010
#define MS_DSR_ON  0x0020
#define MS_RING_ON 0x0040
#define MS_RLSD_ON 0x0080                                                // carrier detect
#ifdef SUPPORT_HW_FLOW
    unsigned long ulChange = (ulNewState ^ ulOldState);
    switch (iPort) {
    case 0:
    #ifdef CTS_0_PIN
        if (ulChange & MS_CTS_ON) {                                      // CTS change - set the state in the corresponding register and generate interrupt, if enabled
            unsigned long ulPortBit = CTS_0_PIN;
            unsigned char ucInput = 31;
            while (!(ulPortBit & 0x01)) {
                ucInput--;
                ulPortBit >>= 1;
            }
            if (ulNewState & MS_CTS_ON) {                                // CTS has just been activated
                fnSimulateInputChange(CTS_0_PORT, ucInput, CLEAR_INPUT);
            }
            else {                                                       // CTS has just been deactivated
                fnSimulateInputChange(CTS_0_PORT, ucInput, SET_INPUT);
            }
        }
    #endif
        break;
    case 1:
    #ifdef CTS_1_PIN
        if (ulChange & MS_CTS_ON) {                                      // CTS change - set the state in the corresponding register and generate interrupt, if enabled
            unsigned char ucPortBit = CTS_1_PIN;
            unsigned char ucInput = 7;
            while (!(ucPortBit & 0x01)) {
                ucInput--;
                ucPortBit >>= 1;
            }
            if (ulNewState & MS_CTS_ON) {                                // CTS has just been activated
                fnSimulateInputChange(CTS_1_PORT, ucInput, CLEAR_INPUT);
            }
            else {                                                       // CTS has just been deactivated
                fnSimulateInputChange(CTS_1_PORT, ucInput, SET_INPUT);
            }
        }
        break;
    #endif
    #ifdef CTS_2_PIN
    case 2:
        if (ulChange & MS_CTS_ON) {                                      // CTS change - set the state in the corresponding register and generate interrupt, if enabled
            unsigned char ucPortBit = CTS_2_PIN;
            unsigned char ucInput = 7;
            while (!(ucPortBit & 0x01)) {
                ucInput--;
                ucPortBit >>= 1;
            }
            if (ulNewState & MS_CTS_ON) {                                // CTS has just been activated
                fnSimulateInputChange(CTS_2_PORT, ucInput, CLEAR_INPUT);
            }
            else {                                                       // CTS has just been deactivated
                fnSimulateInputChange(CTS_2_PORT, ucInput, SET_INPUT);
            }
        }
        break;
    #endif
    #ifdef CTS_3_PIN
    case 3:
        if (ulChange & MS_CTS_ON) {                                      // CTS change - set the state in the corresponding register and generate interrupt, if enabled
            unsigned char ucPortBit = CTS_3_PIN;
            unsigned char ucInput = 7;
            while (!(ucPortBit & 0x01)) {
                ucInput--;
                ucPortBit >>= 1;
            }
            if (ulNewState & MS_CTS_ON) {                                // CTS has just been activated
                fnSimulateInputChange(CTS_3_PORT, ucInput, CLEAR_INPUT);
            }
            else {                                                       // CTS has just been deactivated
                fnSimulateInputChange(CTS_3_PORT, ucInput, SET_INPUT);
            }
        }
        break;
    #endif
    }
#endif
}

// UART Break detection simulation
//
extern void fnSimulateBreak(int iPort)
{
}

// Process simulated DMA
//
extern unsigned long fnSimDMA(char *argv[])
{
    return 0;
}


extern void fnSimulateLinkUp(void)
{
#ifdef ETH_INTERFACE
    #ifdef PHY_INTERRUPT
    unsigned char ucBit = 31;
    unsigned long ulBit = PHY_INT_PIN;
    while (ulBit > 1) {
        ulBit >>= 1;
        ucBit--;
    }
    fnSimulateInputChange(PHY_INT_PORT, ucBit, CLEAR_INPUT);
    #endif
    fnUpdateIPConfig();                                                  // update display in simulator
#endif
}

#ifdef SUPPORT_RTC
// Synchronise the internal RTC to the PC time when simulating
//
extern void fnInitInternalRTC(char *argv[])
{
    unsigned short *ptrShort = (unsigned short *)*argv;

    RTC_YEAR = *ptrShort++;
    RTC_MONTH = *ptrShort++;
    RTC_DOW = *ptrShort++;
    RTC_DOM = *ptrShort++;
    RTC_HOUR = *ptrShort++;
    RTC_MIN = *ptrShort++;
    RTC_SEC = *ptrShort++;
    RTC_DOY = ((RTC_MONTH * 30) + RTC_DOM);                              // approximate day of year

    RTC_CTIME0 = (RTC_SEC | (RTC_MIN << 8) | (RTC_HOUR << 16) | (RTC_DOW << 24));
    RTC_CTIME1 = (RTC_DOM | (RTC_MONTH << 8) | (RTC_YEAR << 16));
    RTC_CTIME2 = RTC_DOY;

    RTC_CCR = (/*CCR_CLKSRC_32K | */CCR_CLKEN);                              // enable the RTC (this assumes that it was previously enabled before last power down and running from 32kHz clock, which is the general case)
}
#endif


// We can simulate timers during a tick here if required
//
extern int fnSimTimers(void)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
    if (SYSTICK_CSR & SYSTICK_ENABLE) {                                  // SysTick is enabled
        unsigned long ulTickCount = 0;
        if (SYSTICK_CSR & SYSTICK_CORE_CLOCK) {
            ulTickCount = (TICK_RESOLUTION * (MASTER_CLOCK/1000));       // count per tick period from internal clock
        }
        if ((SYSTICK_CURRENT+1) > ulTickCount) {
            SYSTICK_CURRENT -= ulTickCount;
        }
        else {
            SYSTICK_CURRENT = SYSTICK_RELOAD;
            if (SYSTICK_CSR & SYSTICK_TICKINT) {                         // if interrupt enabled
                INT_CONT_STATE_REG |= PENDSTSET;
                if (!(ucLPC17xx.CORTEX_M3_REGS.ulPRIMASK & INTERRUPT_MASKED)) { // if interrupt have been enabled, call interrupt handler
                    ptrVect->ptrSysTick();
                }
            }
        }
    }


    // Timer 0..3 simulation. These assume no prescaler and only match on internal match channel 0
    //
    if (T0TCR & COUNTER_ENABLE) {                                        // ensure enabled
        unsigned long ulTickCount = 0;
        switch (PCLKSEL0 & PCLK_TIMER0_CCLK_8) {
        case PCLK_TIMER0_CCLK_1:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(1*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER0_CCLK_2:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(2*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER0_CCLK_4:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(4*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER0_CCLK_8:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(8*1000)); // clock pulses in tick period
            break;
        }
        if ((T0TC + ulTickCount) >= T0MR0) {                             // match with register 0
            if (T0MCR & MR0R) {                                          // reset on match
                T0TC = 0;
            }
            if (T0MCR & MR0S) {                                          // stop on match
                T0TCR = 0;
            }
            if (T0MCR & MR0I) {                                          // interrupt programmed on match
                IRQ0_31_SPR |= (1 << irq_Timer0_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_Timer0_ID)) {                // if timer 0 interrupt is not disabled
                    ptrVect->processor_interrupts.irq_Timer0();          // call the interrupt handler
                }
            }
        }
        else {
            T0TC += ulTickCount;
        }
    }

    if (T1TCR & COUNTER_ENABLE) {                                        // ensure enabled
        unsigned long ulTickCount = 0;
        switch (PCLKSEL0 & PCLK_TIMER1_CCLK_8) {
        case PCLK_TIMER1_CCLK_1:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(1*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER1_CCLK_2:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(2*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER1_CCLK_4:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(4*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER1_CCLK_8:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(8*1000)); // clock pulses in tick period
            break;
        }
        if ((T1TC + ulTickCount) >= T1MR0) {                             // match with register 0
            if (T1MCR & MR0R) {                                          // reset on match
                T1TC = 0;
            }
            if (T1MCR & MR0S) {                                          // stop on match
                T1TCR = 0;
            }
            if (T1MCR & MR0I) {                                          // interrupt programmed on match
                IRQ0_31_SPR |= (1 << irq_Timer1_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_Timer1_ID)) {                // if timer 1 interrupt is not disabled
                    ptrVect->processor_interrupts.irq_Timer1();          // call the interrupt handler
                }
            }
        }
        else {
            T1TC += ulTickCount;
        }
    }

    if (T2TCR & COUNTER_ENABLE) {                                        // ensure enabled
        unsigned long ulTickCount = 0;
        switch (PCLKSEL1 & PCLK_TIMER2_CCLK_8) {
        case PCLK_TIMER2_CCLK_1:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(1*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER2_CCLK_2:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(2*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER2_CCLK_4:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(4*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER2_CCLK_8:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(8*1000)); // clock pulses in tick period
            break;
        }
        if ((T2TC + ulTickCount) >= T2MR0) {                             // match with register 0
            if (T2MCR & MR0R) {                                          // reset on match
                T2TC = 0;
            }
            if (T2MCR & MR0S) {                                          // stop on match
                T2TCR = 0;
            }
            if (T2MCR & MR0I) {                                          // interrupt programmed on match
                IRQ0_31_SPR |= (1 << irq_Timer2_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_Timer2_ID)) {                // if timer 2 interrupt is not disabled
                    ptrVect->processor_interrupts.irq_Timer2();          // call the interrupt handler
                }
            }
        }
        else {
            T2TC += ulTickCount;
        }
    }

    if (T3TCR & COUNTER_ENABLE) {                                        // ensure enabled
        unsigned long ulTickCount = 0;
        switch (PCLKSEL1 & PCLK_TIMER2_CCLK_8) {
        case PCLK_TIMER3_CCLK_1:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(1*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER3_CCLK_2:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(2*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER3_CCLK_4:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(4*1000)); // clock pulses in tick period
            break;
        case PCLK_TIMER3_CCLK_8:
            ulTickCount = (unsigned long)((float)((float)TICK_RESOLUTION*(float)PCLK)/(float)(8*1000)); // clock pulses in tick period
            break;
        }
        if ((T3TC + ulTickCount) >= T3MR0) {                             // match with register 0
            if (T3MCR & MR0R) {                                          // reset on match
                T3TC = 0;
            }
            if (T3MCR & MR0S) {                                          // stop on match
                T3TCR = 0;
            }
            if (T3MCR & MR0I) {                                          // interrupt programmed on match
                IRQ0_31_SPR |= (1 << irq_Timer3_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_Timer3_ID)) {                // if timer 3 interrupt is not disabled
                    ptrVect->processor_interrupts.irq_Timer3();          // call the interrupt handler
                }
            }
        }
        else {
            T3TC += ulTickCount;
        }
    }
    if (WDMOD & WDEN) {
        if (WDFEED == 0x55) {                                            // recognised as retrigger
            WDTV = WDTC;                                                 // reload watchdog counter since retriggered
            WDFEED = 0x12345678;
        }
        else {
            unsigned long ulTickCount;
#if defined LPC1788
            ulTickCount = ((500000/1000)*TICK_RESOLUTION);               // watchdog timer runs from dedicated internal 500kHz oscillator (+/-30% tolerance)
#else
            switch (WDCLKSEL) {
            case WD_CLOCK_INT_RC:                                        // not supported
            case WD_CLOCK_RTC:
                WDTV = 0;
                break;
            case WD_CLOCK_PCLK:
                switch (PCLKSEL0 & PCLK_WDT_CCLK_8) {
                    case PCLK_WDT_CCLK_1:
                        ulTickCount = ((PCLK/1000)*TICK_RESOLUTION);
                        break;
                    case PCLK_WDT_CCLK_2:
                        ulTickCount = ((PCLK/2000)*TICK_RESOLUTION);
                        break;
                    case PCLK_WDT_CCLK_4:
                        ulTickCount = ((PCLK/4000)*TICK_RESOLUTION);
                        break;
                    case PCLK_WDT_CCLK_8:
                        ulTickCount = ((PCLK/8000)*TICK_RESOLUTION);
                        break;
                }
                if (WDTV >= ulTickCount) {
                    WDTV -= ulTickCount;
                }
                else {
                    WDTV = 0;
                }
                break;
            }
#endif
            if (WDTV == 0) {
                if (WDRESET & WDMOD) {                                   // programmed to generate reset on timeout
                    return RESET_CARD_WATCHDOG;                          // signal watchdog reset
                }
            }
        }
    }
#ifdef SUPPORT_RTC
    if (RTC_CCR & CCR_CLKEN) {                                           // if RTC is enabled
      //if (RTC_CCR & CCR_CLKSRC_32K) {                                  // if clocked from 32kHz
            RTC_CTC += (32768/(1000/TICK_RESOLUTION));                   // the count per TICK based on 32kHz source
      //}
      //else {
      //    RTC_CTC += ((PCLK/(RTC_PREINT + 1))/(1000/TICK_RESOLUTION)); // the count per TICK
      //}
        if (RTC_CTC >= 32768) {                                          // one second count
            RTC_CTC = 0;
            (RTC_SEC)++;
            if (RTC_CIIR & CIIR_IMSEC) {                                 // interrupt on each second increment
                RTC_ILR |= RTCCIF;                                       // set the increment interrupt flag
            }
            if (RTC_SEC >= 60) {
                RTC_SEC = 0;
                (RTC_MIN)++;
                if (RTC_CIIR & CIIR_IMMIN) {                             // interrupt on each minute increment
                    RTC_ILR |= RTCCIF;                                   // set the increment interrupt flag
                }
                if (RTC_MIN >= 60) {
                    RTC_MIN = 0;
                    (RTC_HOUR)++;
                    if (RTC_CIIR & CIIR_IMHOUR) {                        // interrupt on each hour increment
                        RTC_ILR |= RTCCIF;                               // set the increment interrupt flag
                    }
                    if (RTC_HOUR >= 24) {
                        RTC_HOUR = 0;
                        (RTC_DOW)++;
                        if (RTC_CIIR & (CIIR_IMDOW | CIIR_IMDOW | CIIR_IMDOY)) { // interrupt on each day of week, month or year increment
                            RTC_ILR |= RTCCIF;                           // set the increment interrupt flag
                        }
                        if (RTC_DOW >= 7) {
                            RTC_DOW = 0;
                        }
                        (RTC_DOY)++;
                        (RTC_DOM)++;
                        if (RTC_DOM >= 30) {                             // approximate
                            RTC_DOM = 0;
                            (RTC_MONTH)++;
                            if (RTC_CIIR & CIIR_IMMON) {                 // interrupt on each month increment
                                RTC_ILR |= RTCCIF;                       // set the increment interrupt flag
                            }
                            if (RTC_MONTH >= 12) {
                                RTC_MONTH = 0;
                                (RTC_YEAR)++;
                                if (RTC_CIIR & CIIR_IMYEAR) {            // interrupt on each year increment
                                    RTC_ILR |= RTCCIF;                   // set the increment interrupt flag
                                }
                            }
                        }
                    }
                }
            }
            RTC_CTIME0 = (RTC_SEC | (RTC_MIN << 13) | (RTC_HOUR << 20) | (RTC_DOW << 26));
            RTC_CTIME1 = (RTC_DOM | (RTC_MONTH << 11) | (RTC_YEAR << 27));
            RTC_CTIME2 = RTC_DOY;

            // Check for alarm match
            //
            if (RTC_AMR != (AMRSEC | AMRMIN | AMRHOUR | AMRDOM | AMRDOW | AMRDOY | AMRMON | AMRYEAR)) { // if alarms are not masked
                if ((RTC_AMR & AMRSEC) || (RTC_SEC == RTC_ALSEC)) {      // seconds match or are masked
                    if ((RTC_AMR & AMRMIN) || (RTC_MIN == RTC_ALMIN)) {  // minutes match or are masked
                        if ((RTC_AMR & AMRHOUR) || (RTC_HOUR == RTC_ALHOUR)) { // hours match or are masked
                            if ((RTC_AMR & AMRDOM) || (RTC_DOM == RTC_ALDOM)) { // day of month matches or is masked
                                if ((RTC_AMR & AMRDOW) || (RTC_DOW == AMRDOW)) { // day of week matches or is masked
                                    if ((RTC_AMR & AMRDOY) || (RTC_DOY == AMRDOY)) { // day of year matches or is masked
                                        if ((RTC_AMR & AMRMON) || (RTC_MONTH == AMRMON)) { // month matches or is masked
                                            if ((RTC_AMR & AMRYEAR) || (RTC_YEAR == AMRYEAR)) { // year matches or is masked
                                                RTC_ILR |= RTCALF;       // flag alarm match
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (RTC_ILR != 0) {
                IRQ0_31_SPR |= (1 << irq_RTC_ID);
                IRQ0_31_CPR = IRQ0_31_SPR;
                if (IRQ0_31_SER & (1 << irq_RTC_ID)) {                   // if RTC interrupt is not disabled
                    ptrVect->processor_interrupts.irq_RTC();             // call the interrupt handler
                }
            }
        }
    }
#endif
#ifdef SUPPORT_RIT
    if ((PCONP & PCRIT) && (RICTRL & RITEN)) {                           // repetitive interrupt timer enabled
        unsigned long ulCount = ((PCLK/1000)*TICK_RESOLUTION);           // count values in one period
        if (((RICOUNTER + ulCount) > RICOMPVAL) || ((RICOUNTER + ulCount) < RICOUNTER)) {
            if (RICTRL & RITENCLR) {
                RICOUNTER = 0;                                           // reset on match
            }
            RICTRL |= RITINT;                                            // set interrupt flag
            IRQ0_31_SPR |= (1 << irq_RITINT_ID);
            IRQ0_31_CPR = IRQ0_31_SPR;
            if (IRQ0_31_SER & (1 << irq_RITINT_ID)) {                    // if RIT interrupt is not disabled
                ptrVect->processor_interrupts.irq_RITINT();              // call the interrupt handler
            }
        }
        else {
            RICOUNTER += ulCount;
        }
    }
#endif
    return 0;
}


unsigned char *fnGetSimTxBufferAdd(void)
{
#ifdef ETH_INTERFACE
    return (fnGetTxBufferAdd(0));
#else
    return 0;
#endif
}


extern int fnSimulateEthernetIn(unsigned char *ucData, unsigned short usLen, int iForce)
{
#ifdef ETH_INTERFACE                                                     // we feed frames in promiscuous mode and filter them according to their details
    extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen);
    LPC17XX_RX_BD *ptrRxBd;
    LPC17XX_RX_STATUS *ptrRxStatus;

    if ((iForce == 0) && (fnCheckEthernetMode(ucData, usLen) == 0)) {
        return 0;                                                        // if we are not in promiscuous mode and the MAC address is not defined to be received we ignore the frame
    }

    ptrRxBd = (LPC17XX_RX_BD *)EMAC_RxDescriptor;
    ptrRxBd += EMAC_RxProduceIndex;

    ptrRxStatus = (LPC17XX_RX_STATUS *)EMAC_RxStatus;
    ptrRxStatus += EMAC_RxConsumeIndex;                          

    uMemcpy((unsigned char *)ptrRxBd->ptrBlock, ucData, usLen);          // copy received data to the input buffer
    ptrRxStatus->ulStatusInfo = (usLen-1);                               // the frame length

    if (ptrRxBd->bd_details & RX_LAN_INTERRUPT_ON_DONE) {
        EMAC_IntStatus |= EMAC_RxDoneInt;                                // set interrupt flag
        IRQ0_31_SPR |= (1 << irq_Ethernet_Controller_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_Ethernet_Controller_ID)) {           // if Ethernet interrupt is not disabled
            if (EMAC_IntEnable & EMAC_RxDoneInt) {                       // if the interrupt is enabled
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
                ptrVect->processor_interrupts.irq_Ethernet_Controller(); // call the interrupt handler
            }
        }
    }

    if (EMAC_RxProduceIndex >= EMAC_RxDescriptorNumber) {
        EMAC_RxProduceIndex = 0;
    }
    else {
        (EMAC_RxProduceIndex)++;
    }
    return 1;
#else
    return 0;
#endif
}


#ifdef SUPPORT_KEY_SCAN

#if KEY_COLUMNS == 0
    #define _NON_MATRIX
    #undef KEY_COLUMNS
    #define KEY_COLUMNS  VIRTUAL_KEY_COLUMNS
    #undef KEY_ROWS
    #define KEY_ROWS VIRTUAL_KEY_ROWS
#endif

static unsigned char fnMapPortBit(unsigned long ulRealBit)
{
    unsigned long ulBit = 0x80000000;
    unsigned char ucRef = 0;

    while (ulBit) {
        if (ulRealBit & ulBit) {
            break;
        }
        ulBit >>= 1;
        ucRef++;
    }
    return ucRef;
}

int iKeyPadInputs[KEY_COLUMNS][KEY_ROWS] = {0};

extern void fnSimulateKeyChange(int *intTable)
{
#if defined _NON_MATRIX
    int iCol, iRow;
    int iChange;
    for (iCol = 0; iCol < KEY_COLUMNS; iCol++) {
        for (iRow = 0; iRow < KEY_ROWS; iRow++) {
            iChange = iKeyPadInputs[iCol][iRow];                         // original value
            iKeyPadInputs[iCol][iRow] = *intTable++;                     // new value
            if (iChange != iKeyPadInputs[iCol][iRow]) {
    #if defined KEY_POLARITY_POSITIVE
                if (iChange)
    #else
                if (iChange)                                             // generally a key press is a '0' 
    #endif
                {
                    iChange = SET_INPUT;
                }
                else {
                    iChange = CLEAR_INPUT;
                }
                switch ((iCol * KEY_ROWS) + iRow) {                      // the specific input
    #if defined KEY_1_PORT_REF
                case 0:                                                  // first key
                    fnSimulateInputChange(KEY_1_PORT_REF, fnMapPortBit(KEY_1_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_2_PORT_REF
                case 1:                                                  // second key
                    fnSimulateInputChange(KEY_2_PORT_REF, fnMapPortBit(KEY_2_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_3_PORT_REF
                case 2:                                                  // third key
                    fnSimulateInputChange(KEY_3_PORT_REF, fnMapPortBit(KEY_3_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_4_PORT_REF
                case 3:                                                  // fourth key
                    fnSimulateInputChange(KEY_4_PORT_REF, fnMapPortBit(KEY_4_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_5_PORT_REF
                case 4:                                                  // fifth key
                    fnSimulateInputChange(KEY_5_PORT_REF, fnMapPortBit(KEY_5_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_6_PORT_REF
                case 5:                                                  // sixth key
                    fnSimulateInputChange(KEY_6_PORT_REF, fnMapPortBit(KEY_6_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_7_PORT_REF
                case 6:                                                  // seventh key
                    fnSimulateInputChange(KEY_7_PORT_REF, fnMapPortBit(KEY_7_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_8_PORT_REF
                case 7:                                                  // eighth key
                    fnSimulateInputChange(KEY_8_PORT_REF, fnMapPortBit(KEY_8_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_9_PORT_REF
                case 8:                                                  // ninth key
                    fnSimulateInputChange(KEY_9_PORT_REF, fnMapPortBit(KEY_9_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_10_PORT_REF
                case 9:                                                  // tenth key
                    fnSimulateInputChange(KEY_10_PORT_REF, fnMapPortBit(KEY_10_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_11_PORT_REF
                case 10:                                                 // eleventh key
                    fnSimulateInputChange(KEY_11_PORT_REF, fnMapPortBit(KEY_11_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_12_PORT_REF
                case 11:                                                 // twelf key
                    fnSimulateInputChange(KEY_12_PORT_REF, fnMapPortBit(KEY_12_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_13_PORT_REF
                case 12:                                                 // thirteenth key
                    fnSimulateInputChange(KEY_13_PORT_REF, fnMapPortBit(KEY_13_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_14_PORT_REF
                case 13:                                                 // fourteenth key
                    fnSimulateInputChange(KEY_14_PORT_REF, fnMapPortBit(KEY_14_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_15_PORT_REF
                case 14:                                                 // fifteenth key
                    fnSimulateInputChange(KEY_15_PORT_REF, fnMapPortBit(KEY_15_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_16_PORT_REF
                case 15:                                                 // sixteenth key
                    fnSimulateInputChange(KEY_16_PORT_REF, fnMapPortBit(KEY_16_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_17_PORT_REF
                case 16:                                                 // seventeenth key
                    fnSimulateInputChange(KEY_17_PORT_REF, fnMapPortBit(KEY_17_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_18_PORT_REF
                case 17:                                                 // eighteenth key
                    fnSimulateInputChange(KEY_18_PORT_REF, fnMapPortBit(KEY_18_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_19_PORT_REF
                case 18:                                                 // nineteenth key
                    fnSimulateInputChange(KEY_19_PORT_REF, fnMapPortBit(KEY_19_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_20_PORT_REF
                case 19:                                                 // twentieth key
                    fnSimulateInputChange(KEY_20_PORT_REF, fnMapPortBit(KEY_20_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_21_PORT_REF
                case 20:                                                 // twenty first key
                    fnSimulateInputChange(KEY_21_PORT_REF, fnMapPortBit(KEY_21_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_22_PORT_REF
                case 21:                                                 // twenty second key
                    fnSimulateInputChange(KEY_22_PORT_REF, fnMapPortBit(KEY_22_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_23_PORT_REF
                case 22:                                                 // twenty third key
                    fnSimulateInputChange(KEY_23_PORT_REF, fnMapPortBit(KEY_23_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_24_PORT_REF
                case 23:                                                 // twenty fourth key
                    fnSimulateInputChange(KEY_24_PORT_REF, fnMapPortBit(KEY_24_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_25_PORT_REF
                case 24:                                                 // twenty fifth key
                    fnSimulateInputChange(KEY_25_PORT_REF, fnMapPortBit(KEY_25_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_26_PORT_REF
                case 25:                                                 // twenty sixth key
                    fnSimulateInputChange(KEY_26_PORT_REF, fnMapPortBit(KEY_26_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_27_PORT_REF
                case 26:                                                  // twenty seventh key
                    fnSimulateInputChange(KEY_27_PORT_REF, fnMapPortBit(KEY_27_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_28_PORT_REF
                case 27:                                                  // twenty eighth key
                    fnSimulateInputChange(KEY_28_PORT_REF, fnMapPortBit(KEY_28_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_29_PORT_REF
                case 28:                                                  // twenty ninth key
                    fnSimulateInputChange(KEY_29_PORT_REF, fnMapPortBit(KEY_29_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_30_PORT_REF
                case 29:                                                  // thirtieth key
                    fnSimulateInputChange(KEY_30_PORT_REF, fnMapPortBit(KEY_30_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_31_PORT_REF
                case 30:                                                  // thirty first key
                    fnSimulateInputChange(KEY_31_PORT_REF, fnMapPortBit(KEY_31_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_32_PORT_REF
                case 31:                                                  // thirty second key
                    fnSimulateInputChange(KEY_32_PORT_REF, fnMapPortBit(KEY_32_PORT_PIN), iChange);
                    break;
    #endif
                }
            }
        }
    }
#else
    memcpy(iKeyPadInputs, intTable, sizeof(iKeyPadInputs));              // copy key pad state to local set
#endif
}

static int fnColumnLow(int iColumnOutput)
{
#if defined KEY_COLUMNS && !defined _NON_MATRIX
    switch (iColumnOutput) {
    case 0:
        return (KEY_COL_OUT_1 & ~KEY_COL_OUT_PORT_1 & KEY_COL_OUT_DDR_1);// if column 1 is being driven low
    #if KEY_COLUMNS > 1
    case 1:
        return (KEY_COL_OUT_2 & ~KEY_COL_OUT_PORT_2 & KEY_COL_OUT_DDR_2);// if column 2 is being driven low
    #endif
    #if KEY_COLUMNS > 2
    case 2:
        return (KEY_COL_OUT_3 & ~KEY_COL_OUT_PORT_3 & KEY_COL_OUT_DDR_3);// if column 3 is being driven low
    #endif
    #if KEY_COLUMNS > 3
    case 3:
        return (KEY_COL_OUT_4 & ~KEY_COL_OUT_PORT_4 & KEY_COL_OUT_DDR_4);// if column 4 is being driven low
    #endif
    #if KEY_COLUMNS > 4
    case 4:
        return (KEY_COL_OUT_5 & ~KEY_COL_OUT_PORT_5 & KEY_COL_OUT_DDR_5);// if column 5 is being driven low
    #endif
    #if KEY_COLUMNS > 5
    case 5:
        return (KEY_COL_OUT_6 & ~KEY_COL_OUT_PORT_6 & KEY_COL_OUT_DDR_6);// if column 6 is being driven low
    #endif
    #if KEY_COLUMNS > 6
    case 6:
        return (KEY_COL_OUT_7 & ~KEY_COL_OUT_PORT_7 & KEY_COL_OUT_DDR_7);// if column 7 is being driven low
    #endif
    #if KEY_COLUMNS > 7
    case 7:
        return (KEY_COL_OUT_8 & ~KEY_COL_OUT_PORT_8 & KEY_COL_OUT_DDR_8);// if column 8 is being driven low
    #endif
    }
#endif
    return 0;
}

static void fnSetRowInput(int iRowInput, int iState)
{
    int iChange;

    if (iState) {
        iChange = CLEAR_INPUT;
    }
    else {
        iChange = SET_INPUT;
    }

#if !defined _NON_MATRIX
    switch (iRowInput) {
    case 0:
    #ifdef KEY_ROWS
        fnSimulateInputChange(KEY_ROW_IN_PORT_1_REF, fnMapPortBit(KEY_ROW_IN_1), iChange);
    #endif
        break;
    case 1:
    #if KEY_ROWS > 1
        fnSimulateInputChange(KEY_ROW_IN_PORT_2_REF, fnMapPortBit(KEY_ROW_IN_2), iChange);
    #endif
        break;
    case 2:
    #if KEY_ROWS > 2
        fnSimulateInputChange(KEY_ROW_IN_PORT_3_REF, fnMapPortBit(KEY_ROW_IN_3), iChange);
    #endif
        break;
    case 3:
    #if KEY_ROWS > 3
        fnSimulateInputChange(KEY_ROW_IN_PORT_4_REF, fnMapPortBit(KEY_ROW_IN_4), iChange);
    #endif
        break;
    case 4:
    #if KEY_ROWS > 4
        fnSimulateInputChange(KEY_ROW_IN_PORT_5_REF, fnMapPortBit(KEY_ROW_IN_5), iChange);
    #endif
        break;
    case 5:
    #if KEY_ROWS > 5
        fnSimulateInputChange(KEY_ROW_IN_PORT_6_REF, fnMapPortBit(KEY_ROW_IN_6), iChange);
    #endif
        break;
    case 6:
    #if KEY_ROWS > 6
        fnSimulateInputChange(KEY_ROW_IN_PORT_7_REF, fnMapPortBit(KEY_ROW_IN_7), iChange);
    #endif
        break;
    case 7:
    #if KEY_ROWS > 7
        fnSimulateInputChange(KEY_ROW_IN_PORT_8_REF, fnMapPortBit(KEY_ROW_IN_8), iChange);
    #endif
        break;
    }
#endif
}

// This routine updates the ports to respect the present setting of a connected matrix key pad
//
extern void fnSimMatrixKB(void)
{
    int i, j;
    iFlagRefresh = fnPortChanges(1);                                     // synchronise with present settings                                                                         
    for (i = 0; i < KEY_COLUMNS; i++) {                                  // check whether a column control is being driven low. If this is the case, any pressed ones in the column are seen at the row input
        if (fnColumnLow(i)) {
            for (j = 0; j < KEY_ROWS; j++) {
                fnSetRowInput(j, iKeyPadInputs[i][j]);
            }
        }
    }
}
#endif

#ifdef USB_INTERFACE                                                     // {14}

// Handle data sent by USB                                         
//
extern void fnSimUSB(int iType, int iEndpoint, USB_HW *ptrUSB_HW)
{
    extern void fnChangeUSBState(int iNewState);
    switch (iType) {
    case USB_SIM_RESET:
        {
            int x;
            ucUSB_address = DEV_EN;
            fnChangeUSBState(0);
            for (x = 0; x < NUMBER_OF_USB_ENDPOINTS; x++) {
                fnLogUSB(x, 0, (unsigned char *)0xffffffff, 0);          // log reset on each endpoint
            }
        }
        break;
    case USB_SIM_TX:                                                     // a frame transmission has just been started
//        if (((TXPKTRDY | EPEDS) & *ptrUSB_HW->ptr_ulUSB_BDControl) != (TXPKTRDY | EPEDS)) { // if the ownership has not been passed to the USB controller ignore it
//          return;
//      }
        iInts |= USB_INT;                                                // flag that the interrupt should be handled
        ulEndpointInt |= (1 << iEndpoint);                               // flag endpoint
        break;
    case USB_SIM_ENUMERATED:                                             // flag that we have completed emumeration
        fnChangeUSBState(1);
        break;
    case USB_SIM_STALL:
        fnLogUSB(iEndpoint, 1, (unsigned char *)0xffffffff, 0);          // log stall
        break;
    case USB_SIM_SUSPEND:
        fnChangeUSBState(0);
        break;
    }
}

static unsigned char ucEndpointFifoIn0[64];                               // LPC endpoint rx FIFOs - control
static unsigned char ucEndpointFifoIn1[64];                               // isochronous
static unsigned char ucEndpointFifoIn2[2][64];                            // bulk - double buffered
static unsigned char ucEndpointFifoIn3[2][1023];                          // isochronous - double buffered
static unsigned char ucEndpointFifoIn4[64];                               // interrupt
static unsigned char ucEndpointFifoIn5[2][64];                            // bulk - double buffered
static unsigned char ucEndpointFifoIn6[2][1023];                          // isochronous - double buffered
static unsigned char ucEndpointFifoIn7[64];                               // interrupt
static unsigned char ucEndpointFifoIn8[2][64];                            // bulk - double buffered
static unsigned char ucEndpointFifoIn9[9][1023];                          // isochronous - double buffered
static unsigned char ucEndpointFifoIn10[64];                              // interrupt
static unsigned char ucEndpointFifoIn11[2][64];                           // bulk - double buffered
static unsigned char ucEndpointFifoIn12[9][1023];                         // isochronous - double buffered
static unsigned char ucEndpointFifoIn13[64];                              // interrupt
static unsigned char ucEndpointFifoIn14[2][64];                           // bulk - double buffered
static unsigned char ucEndpointFifoIn15[2][64];                           // bulk - double buffered

static unsigned short usEndpoint_in_index[16] = {0};
static unsigned char ucEndpoint_in_buffer_put[16] = {0};
static unsigned char ucEndpoint_in_buffer_get[16] = {0};
static unsigned long ulEndpoint_in_valid[16] = {0};

static unsigned char ucEndpointFifoOut0[64];                             // LPC endpoint rx FIFOs - control
static unsigned char ucEndpointFifoOut1[64];                             // isochronous
static unsigned char ucEndpointFifoOut2[2][64];                          // bulk - double buffered
static unsigned char ucEndpointFifoOut3[2][1023];                        // isochronous - double buffered
static unsigned char ucEndpointFifoOut4[64];                             // interrupt
static unsigned char ucEndpointFifoOut5[2][64];                          // bulk - double buffered
static unsigned char ucEndpointFifoOut6[2][1023];                        // isochronous - double buffered
static unsigned char ucEndpointFifoOut7[64];                             // interrupt
static unsigned char ucEndpointFifoOut8[2][64];                          // bulk - double buffered
static unsigned char ucEndpointFifoOut9[2][1023];                        // isochronous - double buffered
static unsigned char ucEndpointFifoOut10[64];                            // interrupt
static unsigned char ucEndpointFifoOut11[2][64];                         // bulk - double buffered
static unsigned char ucEndpointFifoOut12[2][1023];                       // isochronous - double buffered
static unsigned char ucEndpointFifoOut13[64];                            // interrupt
static unsigned char ucEndpointFifoOut14[2][64];                         // bulk - double buffered
static unsigned char ucEndpointFifoOut15[2][64];                         // bulk - double buffered

static int iEndpointProperties[16] = {                                   // double-buffered property
    0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1
};

static unsigned long endpoint_enabled = 0;

static unsigned char ucDeviceStatus = 0;
static unsigned char ucEndpointStatus[2*16] = {0};


extern void fnCheckUSBOut(int iDevice, int iEndpoint)
{
    extern int iGetGetBank(int iEndpoint);

    int iBank = 0;
    switch (iEndpoint) {
    case 0:                                                              // endpoint 0
        while (ulEndpoint_in_valid[0]) {                                 // IN buffer valid
            unsigned short usUSBLength;
            usUSBLength = (unsigned short)ulEndpoint_in_valid[0];        // extract the length of data content
            ulEndpoint_in_valid[0] = 0;
            iTxDataToggle[0] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(0, usUSBLength, ucEndpointFifoOut0, iTxDataToggle[0]);
            USBEpIntSt  |= 0x00000002;                                   // mark that this endpoint has a pending interrupt
            if (USBEpIntSt & 0x00000002) {                               // if the interrupt is enabled set the device interrupt state
                if (USBEpIntPri & 0x00000002) {
                    USBDevIntSt |= DEVICE_STATUS_EP_FAST;
                }
                else {
                    USBDevIntSt |= DEVICE_STATUS_EP_SLOW;
                }
            }
            ucEndpointStatus[1] &= ~(USB_EP_BUFFER_1_FULL | USB_EP_FULL_EMPTY); // tx buffer free
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt            
        }
        /*if (UDP_CSR0 & FORCESTALL) {
            UDP_CSR0 |= STALLSENT;                                       // stall acknowledged
            UDP_ISR = EP0INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }*/
        break;
/*
    case 1:                                                              // endpoint 1
        if (!(UDP_CSR1 & EPEDS)) {
            return;                                                      // endpoint not enabled so ignore
        }
        while (UDP_CSR1 & TXPKTRDY) {                                    // transmission bit set
            unsigned short usUSBLength;
            iBank = iGetGetBank(1);
            memcpy((unsigned char *)&usUSBLength, ucEndpointFifoOut1[iBank], sizeof(unsigned short)); // extract the length of data content
            iTxDataToggle[1] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(1, usUSBLength, &ucEndpointFifoOut1[iBank][2], iTxDataToggle[1]);
            if (UDP_IMR & EP1INT) {
                UDP_ISR |= EP1INT;
            }
            UDP_CSR1 &= ~TXPKTRDY;                                       // transmission complete
            UDP_CSR1 |= TXCOMP;                                          // mark that the transmission has been successfully acknowledged            
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt
        }
        if (UDP_CSR1 & FORCESTALL) {
            UDP_CSR1 |= STALLSENT;                                       // stall acknowledged
            UDP_ISR = EP1INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }
        break;
    case 2:                                                              // endpoint 2
        if (!(UDP_CSR2 & EPEDS)) {
            return;                                                      // endpoint not enabled so ignore
        }
        while (UDP_CSR2 & TXPKTRDY) {                                    // transmission bit set
            unsigned short usUSBLength;
            iBank = iGetGetBank(2);
            memcpy((unsigned char *)&usUSBLength, ucEndpointFifoOut2[iBank], sizeof(unsigned short)); // extract the length of data content
            iTxDataToggle[2] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(2, usUSBLength, &ucEndpointFifoOut2[iBank][2], iTxDataToggle[2]);
            if (UDP_IMR & EP2INT) {
                UDP_ISR |= EP2INT;
            }
            UDP_CSR2 &= ~TXPKTRDY;                                       // transmission complete
            UDP_CSR2 |= TXCOMP;                                          // mark that the transmission has been successfully acknowledged            
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt
        }
        if (UDP_CSR2 & FORCESTALL) {
            UDP_CSR2 |= STALLSENT;                                       // stall acnowledged
            UDP_ISR = EP2INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }
        break;
    case 3:                                                              // endpoint 3
        if (!(UDP_CSR3 & EPEDS)) {
            return;                                                      // endpoint not enabled so ignore
        }
        while (UDP_CSR3 & TXPKTRDY) {                                    // transmission bit set
            unsigned short usUSBLength;
            memcpy((unsigned char *)&usUSBLength, ucEndpointFifoOut3, sizeof(unsigned short)); // extract the length of data content
            iTxDataToggle[3] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(3, usUSBLength, &ucEndpointFifoOut3[2], iTxDataToggle[3]);
            if (UDP_IMR & EP3INT) {
                UDP_ISR |= EP3INT;
            }
            UDP_CSR3 &= ~TXPKTRDY;                                       // transmission complete
            UDP_CSR3 |= TXCOMP;                                          // mark that the transmission has been successfully acknowledged            
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt
        }
        if (UDP_CSR3 & FORCESTALL) {
            UDP_CSR3 |= STALLSENT;                                       // stall acnowledged
            UDP_ISR = EP0INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }
        break;
    case 4:                                                              // endpoint 4
        if (!(UDP_CSR4 & EPEDS)) {
            return;                                                      // endpoint not enabled so ignore
        }
        while (UDP_CSR4 & TXPKTRDY) {                                    // transmission bit set
            unsigned short usUSBLength;
            iBank = iGetGetBank(4);
            memcpy((unsigned char *)&usUSBLength, ucEndpointFifoOut4[iBank], sizeof(unsigned short)); // extract the length of data content
            iTxDataToggle[4] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(4, usUSBLength, &ucEndpointFifoOut4[iBank][2], iTxDataToggle[4]);
            if (UDP_IMR & EP4INT) {
                UDP_ISR |= EP4INT;
            }
            UDP_CSR4 &= ~TXPKTRDY;                                       // transmission complete
            UDP_CSR4 |= TXCOMP;                                          // mark that the transmission has been successfully acknowledged            
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt
        }
        if (UDP_CSR4 & FORCESTALL) {
            UDP_CSR4 |= STALLSENT;                                       // stall acnowledged
            UDP_ISR = EP0INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }
        break;*/
    case 5:                                                              // endpoint 5
        while (ulEndpoint_in_valid[5]) {                                 // IN buffer valid
            unsigned short usUSBLength;
            usUSBLength = (unsigned short)ulEndpoint_in_valid[5];        // extract the length of data content
            ulEndpoint_in_valid[5] = 0;
            iTxDataToggle[5] ^= 1;                                       // automatic data toggle on transmission since it is automatically controlled by the chip
            fnLogUSB(5, usUSBLength, ucEndpointFifoOut5[0], iTxDataToggle[5]);
            USBEpIntSt  |= 0x00000800;                                   // mark that this endpoint has a pending interrupt
            if (USBEpIntSt & 0x00000800) {                               // if the interrupt is enabled set the device interrupt state
                if (USBEpIntPri & 0x00000800) {
                    USBDevIntSt |= DEVICE_STATUS_EP_FAST;
                }
                else {
                    USBDevIntSt |= DEVICE_STATUS_EP_SLOW;
                }
            }
            ucEndpointStatus[11] &= ~(USB_EP_BUFFER_1_FULL | USB_EP_FULL_EMPTY); // tx buffer free
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_IN_SUCCESS);     // generate tx interrupt            
        }
        /*if (UDP_CSR0 & FORCESTALL) {
            UDP_CSR0 |= STALLSENT;                                       // stall acknowledged
            UDP_ISR = EP0INT;
            fnSimulateUSB(iDevice, iEndpoint, 0, 0, USB_STALL_SUCCESS);  // generate stall interrupt
        }*/
        break;
    }
}



extern unsigned long fnUSB_command_interpreter(unsigned long ulCommand, unsigned long ulData)
{
    switch (ulCommand) {
    case USB_CMD_GET_DEVICE_STATUS:
        if (USB_CMD_GET_DEVICE_STATUS_DATA_PHASE == ulData) {
            return ucDeviceStatus;
        }
        break;
    default:
        if ((ulCommand & ~0x00ff0000) == USB_CMD_SELECT_EP) {
            unsigned char ucEndpoint = (unsigned char)((ulCommand & 0x00ff0000) >> 16);
            if ((ulData & ~0x00ff0000) == USB_CMD_SELECT_EP_DATA_PHASE) {
                if ((unsigned char)((ulData & 0x00ff0000) >> 16) == ucEndpoint) {
                    return ucEndpointStatus[ucEndpoint];
                }
            }
        }
        break;
    }
    return 0;
}


extern unsigned long fnUSB_command_command(unsigned long ulCommand, unsigned long ulData)
{
    static unsigned char ucEndpoint = 0;
    switch (ulCommand) {
    case USB_CMD_SET_DEVICE_STATUS:
        break;
    case USB_CMD_SET_ADDRESS:
        if (USB_CMD_SET_ADDRESS_DATA_PHASE == (ulData & ~0x00ff0000)) {
            ucUSB_address = (unsigned char)(ulData >> 16);
        }
        break;
    case USB_CMD_VALID_EP_BUFFER:                                        // validataes that the IN buffer corresponding to the presently selected endpoint is reads to be dispatched
        ulEndpoint_in_valid[ucEndpoint/2] = (USBTxPLen | 0x80000000);    // mark that data is ready and set its length
        if (ucEndpoint <= 1) {
            ucEndpointStatus[ucEndpoint] |= (USB_EP_BUFFER_1_FULL | USB_EP_FULL_EMPTY); // mark that buffer 1 is full and also set the full flag
        }
        else {
            ucEndpointStatus[ucEndpoint] |= (USB_EP_BUFFER_1_FULL | USB_EP_FULL_EMPTY); // mark that buffer 1 is full and also set the full flag
        }
        break;
    default:
        if ((ulCommand & ~0x00ff0000) == USB_CMD_SELECT_EP) {
            ucEndpoint = (unsigned char)((ulCommand & 0x00ff0000) >> 16);// select physical enpoint index
        }
        break;
    }
    return 0;
}





static volatile unsigned char *fnGetUSB_FIFOAdd(volatile unsigned char *ptrAdd, int iDirection)
{
    /*
    if (iDirection != 0) {                                               // transmission buffer
        if (UDP_FDR0_ADD == ptrAdd) {
            return ucEndpointFifoOut0;
        }
        if (UDP_FDR1_ADD == ptrAdd) {
            return ucEndpointFifoOut1[0];
        }
        if (UDP_FDR2_ADD == ptrAdd) {
            return ucEndpointFifoOut2[0];
        }
        if (UDP_FDR3_ADD == ptrAdd) {
            return ucEndpointFifoOut3;
        }
        if (UDP_FDR4_ADD == ptrAdd) {
            return ucEndpointFifoOut4[0];
        }
        if (UDP_FDR5_ADD == ptrAdd) {
            return ucEndpointFifoOut5[0];
        }
    }
    else {
        if (UDP_FDR0_ADD == ptrAdd) {
            return ucEndpointFifoIn0;
        }
        if (UDP_FDR1_ADD == ptrAdd) {
            return ucEndpointFifoIn1[0];
        }
        if (UDP_FDR2_ADD == ptrAdd) {
            return ucEndpointFifoIn2[0];
        }
        if (UDP_FDR3_ADD == ptrAdd) {
            return ucEndpointFifoIn3;
        }
        if (UDP_FDR4_ADD == ptrAdd) {
            return ucEndpointFifoIn4[0];
        }
        if (UDP_FDR5_ADD == ptrAdd) {
            return ucEndpointFifoIn5[0];
        }
    }
    return ptrAdd;
    */
    return 0;
}

extern unsigned long fnGetUSB_from_FIFO(void)
{
    if (USBCtrl & CTRL_RD_EN) {
        unsigned char ucEndpoint = (unsigned char)((USBCtrl >> 2) & 0x0f);// logical endpoint
        unsigned long ulData;
        switch (ucEndpoint) {
        case 0:
            ulData  =  ucEndpointFifoIn0[usEndpoint_in_index[0]++];
            ulData |= (ucEndpointFifoIn0[usEndpoint_in_index[0]++] << 8);
            ulData |= (ucEndpointFifoIn0[usEndpoint_in_index[0]++] << 16);
            ulData |= (ucEndpointFifoIn0[usEndpoint_in_index[0]++] << 24);
            break;

        case 2:
            ulData  =  ucEndpointFifoIn2[ucEndpoint_in_buffer_get[2]][usEndpoint_in_index[2]++];
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[2]][usEndpoint_in_index[2]++] << 8);
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[2]][usEndpoint_in_index[2]++] << 16);
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[2]][usEndpoint_in_index[2]++] << 24);
            if (usEndpoint_in_index[2] >= fnGetEndpointInfo(2)) {
                usEndpoint_in_index[2] = 0;
                ucEndpoint_in_buffer_get[2] ^= 0x01;
            }
            break;

        case 5:
            ulData  =  ucEndpointFifoIn2[ucEndpoint_in_buffer_get[5]][usEndpoint_in_index[5]++];
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[5]][usEndpoint_in_index[5]++] << 8);
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[5]][usEndpoint_in_index[5]++] << 16);
            ulData |= (ucEndpointFifoIn2[ucEndpoint_in_buffer_get[5]][usEndpoint_in_index[5]++] << 24);
            if (usEndpoint_in_index[5] >= fnGetEndpointInfo(5)) {
                usEndpoint_in_index[5] = 0;
                ucEndpoint_in_buffer_get[5] ^= 0x01;
            }
            break;
        }
        return ulData;
    }
    return 0;
}

// Inject USB transactions for test purposes
//
extern int fnSimulateUSB(int iDevice, int iEndPoint, unsigned char ucPID, unsigned char *ptrDebugIn, unsigned short usLenEvent)
{
    if (ptrDebugIn == 0) {
        switch (usLenEvent) {
        case USB_RESET_CMD:
//          memset(iTxDataToggle, 0, sizeof(iTxDataToggle));
//          memset(iRxDataBank, 0, sizeof(iRxDataBank));
//          memset(ulTxDataBank, 0, sizeof(ulTxDataBank));
            USBDevIntSt |= DEVICE_STATUS_DEV_STAT;                       // flag that a USB bus reset has been detected
            ucDeviceStatus = USB_STATUS_RST;
            break;
        case USB_SLEEP_CMD:
            USBDevIntSt |= DEVICE_STATUS_DEV_STAT;                       // flag that a suspend reset has been detected
            ucDeviceStatus = (0x00000004 | 0x00000008);
            break;
        case USB_RESUME_CMD:
//          UDP_ISR = RXRSM;
            break;
        case USB_STALL_SUCCESS:
        case USB_IN_SUCCESS:
            break;
        default:
            return 0;
        }
    }
    else {                                                               // data being injected
        if (((ucUSB_address & DEV_EN) == 0) || ((ucUSB_address & ~DEV_EN) != iDevice)) { // if the USB address is either not enabled or is different to the addressed one
            if (iDevice != 0xff) {                                       // special broadcast for simulator use so that it doesn't have to know the USB address
                return 1;
            }
        }
        switch (iEndPoint) {
        case 0:                                                          // endpoint 0 - control/bulk/interrupt - max endpoint size 8 bytes - not dual-banked
            USBEpIntSt |= 0x00000001;                                    // mark that this endpoint has a pending interrupt
            if (USBEpIntSt & 0x00000001) {                               // if the interrupt is enabled set the device interrupt state
                if (USBEpIntPri & 0x00000001) {
                    USBDevIntSt |= DEVICE_STATUS_EP_FAST;
                }
                else {
                    USBDevIntSt |= DEVICE_STATUS_EP_SLOW;
                }
            }
            if (SETUP_PID == ucPID) {                                    // if a setup packet set the RXSETUP bit
                ucEndpointStatus[0] = USB_EP_SETUP_PACKET_RECEIVE;
            }
            else if (OUT_PID == ucPID) {
                ucEndpointStatus[0] = 0;
            }
            USBRxPLen = (usLenEvent | PKT_DV | PKT_RDY);                 // the reception length
            if (usLenEvent > sizeof(ucEndpointFifoIn0)) {                // limit data length to the FIFO length
                usLenEvent = sizeof(ucEndpointFifoIn0);
            }
            memcpy(ucEndpointFifoIn0, ptrDebugIn, usLenEvent); 
            usEndpoint_in_index[0] = 0;
            break;
        case 2:                                                          // double-buffered bulk
            USBEpIntSt |= 0x00000010;                                    // mark that this endpoint has a pending interrupt
            if (USBEpIntSt & 0x00000010) {                               // if the interrupt is enabled set the device interrupt state
                if (USBEpIntPri & 0x00000010) {
                    USBDevIntSt |= DEVICE_STATUS_EP_FAST;
                }
                else {
                    USBDevIntSt |= DEVICE_STATUS_EP_SLOW;
                }
            }
            ucEndpointStatus[2] = 0;
            USBRxPLen = (usLenEvent | PKT_DV | PKT_RDY);                 // the reception length
            if (usLenEvent > sizeof(ucEndpointFifoIn2)) {                // limit data length to the FIFO length
                usLenEvent = sizeof(ucEndpointFifoIn2);
            }
            memcpy(ucEndpointFifoIn2[ucEndpoint_in_buffer_put[2]], ptrDebugIn, usLenEvent); 
            usEndpoint_in_index[2] = 0;
            ucEndpoint_in_buffer_put[2] ^= 0x01;
            break;
        case 5:                                                          // double-buffered bulk
            USBEpIntSt |= 0x00100000;                                    // mark that this endpoint has a pending interrupt
            if (USBEpIntSt & 0x00100000) {                               // if the interrupt is enabled set the device interrupt state
                if (USBEpIntPri & 0x00100000) {
                    USBDevIntSt |= DEVICE_STATUS_EP_FAST;
                }
                else {
                    USBDevIntSt |= DEVICE_STATUS_EP_SLOW;
                }
            }
            ucEndpointStatus[5] = 0;
            USBRxPLen = (usLenEvent | PKT_DV | PKT_RDY);                 // the reception length
            if (usLenEvent > sizeof(ucEndpointFifoIn5)) {                // limit data length to the FIFO length
                usLenEvent = sizeof(ucEndpointFifoIn5);
            }
            memcpy(ucEndpointFifoIn5[ucEndpoint_in_buffer_put[5]], ptrDebugIn, usLenEvent); 
            usEndpoint_in_index[5] = 0;
            ucEndpoint_in_buffer_put[5] ^= 0x01;
            break;
        }
    }
    if (USBDevIntSt & USBDevIntEn) {                    

        IRQ0_31_SPR |= (1 << irq_USB_ID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if (IRQ0_31_SER & (1 << irq_USB_ID)) {                           // if USB interrupt is not disabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)((unsigned char *)((unsigned char *)&vector_ram));
            ptrVect->processor_interrupts.irq_USB();                     // call the interrupt handler
        } 
    }
    return 0;
}
#endif

#if defined SUPPORT_TOUCH_SCREEN                                         // {1}
static int iPenLocationX = 0;                                            // last sample ADC input value
static int iPenLocationY = 0;
static int iPenDown = 0;

#define MIN_X_TOUCH          0x2000                                      // reference values for 4-wire touch on LPC2478-STK
#define MAX_X_TOUCH          0xe800
#define MIN_Y_TOUCH          0x2900
#define MAX_Y_TOUCH          0xe000

extern void fnPenPressed(int iX, int iY)
{
    iPenLocationX = (iX - 6);
    iPenLocationY = (iY - 3);
    iPenDown = 1;
}

extern void fnPenMoved(int iX, int iY)
{
    iPenLocationX = (iX - 6);
    iPenLocationY = (iY - 3);
}

extern void fnPenLifted(void)
{
    iPenLocationX = 0;
    iPenLocationY = 0;
    iPenDown = 0;
}

extern void fnGetPenSamples(unsigned short *ptrX, unsigned short *ptrY)
{
    if (iPenLocationX != 0) {
        iPenLocationX = iPenLocationX;
    }
    if (iPenDown == 0) {
        *ptrX = 0;
        *ptrY = 0;
    }
    else {
        *ptrX = (MIN_X_TOUCH + ((iPenLocationX * ((MAX_X_TOUCH - MIN_X_TOUCH)))/GLCD_X));
        *ptrY = (MIN_Y_TOUCH + ((iPenLocationY * ((MAX_Y_TOUCH - MIN_Y_TOUCH)))/GLCD_Y));
    }
}
#endif

#ifdef BATTERY_BACKED_RAM
// Return all RTC content which is battery backed up
//
extern int fnGetBatteryRAMContent(unsigned char *ucData, unsigned long ulReference)
{
    return 0;                                                            // all data saved
}
extern int fnPutBatteryRAMContent(unsigned char ucData, unsigned long ulReference)
{
    return 0;                                                            // no more data accepted
}
#endif

#endif
