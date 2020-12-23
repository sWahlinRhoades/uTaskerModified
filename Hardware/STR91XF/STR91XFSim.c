/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:   	    STR91XFSim.c
   Project: 	Single Chip Embedded Internet
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2011
   *********************************************************************/  


/* =================================================================== */
/*                           include files                             */
/* =================================================================== */
#ifdef _STR91XF

#include "config.h"



/**************************************************************************************************/
/********************************* ST STR91XF *****************************************************/
/**************************************************************************************************/


#define GET_PERIPHERALS 0
#define GET_OUTPUTS     1
#define GET_PER_TYPE    2
#define GET_OUTPUT_MASK 3
static unsigned char ucGetPortType(int portNr, int iRequest, int iStart);

static int iFlagRefresh = 0;


static unsigned char ucPort_in_0;
static unsigned char ucPort_in_1;
static unsigned char ucPort_in_2;
static unsigned char ucPort_in_3;
static unsigned char ucPort_in_4;
static unsigned char ucPort_in_5;
static unsigned char ucPort_in_6;
static unsigned char ucPort_in_7;
static unsigned char ucPort_in_8;
static unsigned char ucPort_in_9;

static unsigned char ucPortFunctions[PORTS_AVAILABLE][PORT_WIDTH] = { 0 };

// Initialisation for all non-zero registers in the device
//
static void fnSetDevice(unsigned char *port_inits)
{
    extern void fnEnterHW_table(void *hw_table);

    FMI_CR = FMI_B0EN;
    FMI_BCE5ADDR = 0x00000006;

    SCU_CLKCNTR = (EMIRATION_2 | MCLKSEL_OSC);
    SCU_PLLCONF = 0x0003c019;
    SCU_SYSSTATUS = (LVD_RST | PLL_LOCK);                                // set also the locked bit for simulator
    SCU_ITCSK = 0x1f;
    SCU_PCGR0 = 0xdb;
    SCU_PRR0  = 0x1053;
                                                                         // Set up the FMI registers to reflect project FLASH settings (as performed in assembler init)
#ifdef _BOOT_BANK1
    FMI_BBSR   = 0;                                                      // Boot bank size is 32k
    FMI_NBBSR  = 6;                                                      // Non-Boot Bank Size is 512k
    FMI_BBADR  = (OFFSET_BANK1/4);                                       // Boot bank starts at 0
    FMI_NBBADR = (OFFSET_BANK0/4);                                       // Non-Boot bank starts at 
#else
    FMI_BBSR   = 4;                                                      // Boot bank size is 512k
    FMI_NBBSR  = 3;                                                      // Non-Boot Bank Size is 32k
    FMI_BBADR  = (OFFSET_BANK1/4);                                       // Boot bank starts at 0
    FMI_NBBADR = (OFFSET_BANK0/4);                                       // Non-Boot bank starts at
#endif
    FMI_CR     = (FMI_B0EN | FMI_B1EN);

    TIM_OC1R_0 = 0x8000;                                                 // Timers
    TIM_OC2R_0 = 0x8000;
    TIM_CNTR_0 = 0xfffc;
    TIM_CR2_0  = 0x0001;
    TIM_OC1R_1 = 0x8000;
    TIM_OC2R_1 = 0x8000;
    TIM_CNTR_1 = 0xfffc;
    TIM_CR2_1  = 0x0001;
    TIM_OC1R_2 = 0x8000;
    TIM_OC2R_2 = 0x8000;
    TIM_CNTR_2 = 0xfffc;
    TIM_CR2_2  = 0x0001;
    TIM_OC1R_3 = 0x8000;
    TIM_OC2R_3 = 0x8000;
    TIM_CNTR_3 = 0xfffc;
    TIM_CR2_3  = 0x0001;

    WDG_PR = 0x00ff;                                                     // Watchdog timer
    WDG_VR = 0xffff; 
    WDG_CNT = 0xffff;

    UART_FR_0  = 0x0090;                                                 // UARTs
    UART_FR_1  = 0x0090;
    UART_FR_2  = 0x0090;

    UART_CR_0  = (TXE | RXE);
    UART_CR_1  = (TXE | RXE);
    UART_CR_2  = (TXE | RXE);

    GPIO_DATA_0 = ucPort_in_0 = *port_inits++;                           // Set port inputs to default states
    GPIO_DATA_1 = ucPort_in_1 = *port_inits++;
    GPIO_DATA_2 = ucPort_in_2 = *port_inits++;
    GPIO_DATA_3 = ucPort_in_3 = *port_inits++;
    GPIO_DATA_4 = ucPort_in_4 = *port_inits++;
    GPIO_DATA_5 = ucPort_in_5 = *port_inits++;
    GPIO_DATA_6 = ucPort_in_6 = *port_inits++;
    GPIO_DATA_7 = ucPort_in_7 = *port_inits++;
    GPIO_DATA_8 = ucPort_in_8 = *port_inits++;
    GPIO_DATA_9 = ucPort_in_9 = *port_inits;

    ENET_SCR  = 0x5a5a0101;                                              // Ethernet
    ENET_ISR  = 0x00050001;
    ENET_RXSR = 0x00000001;
    ENET_MAH  = 0x0000ffff;
    ENET_MAL  = 0x0000ffff;
    ENET_VL1  = 0x00008100;
    ENET_VL2  = 0x00008100;

    fnEnterHW_table(ucPortFunctions);
}



static unsigned char ucFLASH[MAIN_FLASH_SIZE + SECONDARY_FLASH_SIZE];    // copy of FLASH memory


extern void fnInitialiseDevice(void * port_inits)
{
    uMemset(ucFLASH, 0xff, sizeof(ucFLASH));                             // we start with deleted FLASH memory contents
    fnPrimeFileSystem();                                                 // the project can then set parameters or files as required
    fnSetDevice((unsigned char *)port_inits);                            // set device registers to startup condition (if not zerod)
}

extern void fnDeleteFlashSector(unsigned char *ptrSector)
{
	int i = FLASH_GRANULARITY;

	while (i--) {
		*ptrSector++ = 0xff;
	}
}

static int iUnprotected[BANK0_SECTORS + BANK1_SECTORS] = {0};            // primary and secondard blocks (protected at reset)
static const unsigned long ulSectorLengths[] = {
#if OFFSET_BANK1 == 0
    0x2000, 0x2000, 0x2000, 0x2000,                                      // secondary FLASH sector lengths
    0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000  // primary FLASH sector lengths
#else
    0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, // primary FLASH sector lengths
    0x2000, 0x2000, 0x2000, 0x2000                                       // secondary FLASH sector lengths
#endif
};


// Determine the sector from the address
//
static int fnGetSector(unsigned char *ptrSector)
{
    int iSector = 0;

#ifdef _BOOT_BANK1
    if (ptrSector < (unsigned char *)OFFSET_BANK0) {
        unsigned char *ptrSectorEnd = (unsigned char *)FLASH_GRANULARITY_BANK1;
        while (iSector < BANK1_SECTORS) {
            if (ptrSector < ptrSectorEnd) {
                return iSector;
            }
            iSector++;
            ptrSectorEnd += FLASH_GRANULARITY_BANK1;
        }
    }
    else {
        unsigned char *ptrSectorEnd = (unsigned char *)(FLASH_GRANULARITY_BANK0 + OFFSET_BANK0);
        while (iSector < BANK0_SECTORS) {
            if (ptrSector < ptrSectorEnd) {
                return (iSector + BANK1_SECTORS);
            }
            iSector++;
            ptrSectorEnd += FLASH_GRANULARITY_BANK0;
        }
    }
#else
    if (ptrSector < (unsigned char *)OFFSET_BANK1) {
        unsigned char *ptrSectorEnd = (unsigned char *)FLASH_GRANULARITY_BANK0;
        while (iSector < BANK0_SECTORS) {
            if (ptrSector < ptrSectorEnd) {
                return iSector;
            }
            iSector++;
            ptrSectorEnd += FLASH_GRANULARITY_BANK0;
        }
    }
    else {
        unsigned char *ptrSectorEnd = (unsigned char *)(FLASH_GRANULARITY_BANK1 + OFFSET_BANK1);
        while (iSector < BANK1_SECTORS) {
            if (ptrSector < ptrSectorEnd) {
                return (iSector + BANK0_SECTORS);
            }
            iSector++;
            ptrSectorEnd += FLASH_GRANULARITY_BANK1;
        }
    }
#endif
    return 0;                                                            // this should never happen
}

extern const unsigned long fnGetSectorSize(unsigned char *ptrSector)
{
    return (ulSectorLengths[fnGetSector(ptrSector)]);
}

extern int fnIsProtected(unsigned char *ptrSectorPointer)
{
    return (iUnprotected[fnGetSector(ptrSectorPointer)] == 0);
}

extern void fnUnprotectSector(volatile unsigned short *usSectorPointer)
{
    iUnprotected[fnGetSector((unsigned char *)usSectorPointer)] = 1;     // mark unprotected
}

extern void fnProtectSector(volatile unsigned short *usSectorPointer)
{
    iUnprotected[fnGetSector((unsigned char *)usSectorPointer)] = 0;     // mark protected
}


// Transform physical access address to address in simulated FLASH
//
extern unsigned char *fnGetFlashAdd(unsigned char *ucAdd)               
{
    if (ucAdd > ((unsigned char *)(MAIN_FLASH_SIZE*2))) {                // if the address seems to be in PC memory - don't concert (this may not be fool-proof!!)
        return ucAdd;
    }
#ifdef _BOOT_BANK1
    if (ucAdd < (unsigned char *)OFFSET_BANK0) {                         // access in BANK 1
        unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)FLASH_START_ADDRESS;
        return ucAdd + sim_add;
    }
    else {                                                               // access in BANK 0
        unsigned long sim_add = (unsigned long)&ucFLASH[SECONDARY_FLASH_SIZE] - (unsigned long)OFFSET_BANK0;
        return ucAdd + sim_add;
    }
#else
    if (ucAdd > (unsigned char *)OFFSET_BANK0) {                         // access in BANK 1
        unsigned long sim_add = (unsigned long)&ucFLASH[MAIN_FLASH_SIZE] - (unsigned long)OFFSET_BANK1;
        return ucAdd + sim_add;
    }
    else {                                                               // access in BANK 0
        unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)FLASH_START_ADDRESS;
        return ucAdd + sim_add;
    }
#endif
}

// Transform simulated address in simulated FLASH back to physical address in real FLASH
//
extern unsigned char *fnPutFlashAdd(unsigned char *ucAdd) 
{
#ifdef _BOOT_BANK1
    if (ucAdd >= (ucFLASH + SECONDARY_FLASH_SIZE)) {
        unsigned long sim_add = (unsigned long)ucFLASH + (unsigned long)SECONDARY_FLASH_SIZE;
        return (OFFSET_BANK0 + (ucAdd - sim_add));                       // address in primary flash bank
    }
    else {                                                               // address in secondary flash bank
        unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)START_OF_FLASH;
        return (ucAdd - sim_add);
    }
#else 
    unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)START_OF_FLASH;
    return (ucAdd - sim_add);
#endif
}



extern unsigned char *fnGetFileSystemStart(int iMemory)
{
#if defined (SPI_FILE_SYSTEM)
    return (fnGetEEPROM(uFILE_START));
#elif defined (SAVE_COMPLETE_FLASH)
    return (&ucFLASH[0]);
#else
    return (&ucFLASH[uFILE_START-FLASH_START_ADDRESS]);
#endif
}

extern unsigned long fnGetFlashSize(int iMemory)
{
#if defined (SPI_FILE_SYSTEM)
    return (fnGetEEPROMSize());
#elif defined (SAVE_COMPLETE_FLASH)
    return (sizeof(ucFLASH));
#else
    return (FILE_SYSTEM_SIZE);
#endif
}


/******************************************************************************************************/



// A copy of the STR91XF peripheral registers for simulation purposes
extern STR91XF_PERIPH ucSTR91XF = {0};







static void fnCallInt(int iVicNumber, unsigned char ucInterruptSource)
{
    int iIntCnt = 0;
    unsigned long *ptrVect;
    unsigned long *ptrVectEnabled;
    unsigned long ulBit = 0x00000001;
    void (*interrupt_routine)(void) = 0;

    if (iVicNumber == 1) {
        ptrVect = VIC1_VC0R_ADDRESS;
        ptrVectEnabled = VIC1_INTER_ADDRESS;
    }
    else {
        ptrVect = VIC0_VC0R_ADDRESS;
        ptrVectEnabled = VIC0_INTER_ADDRESS;
    }

    while (iIntCnt < 16) {
        if (*ptrVectEnabled & ulBit) {
            if (*ptrVect & ENABLE_INTERRUPT) {
                if ((unsigned char)(*ptrVect & ~ENABLE_INTERRUPT) == ucInterruptSource) {
                    ptrVect -= (VIC1_VC0R_ADDRESS - VIC1_VA0R_ADDRESS);
                    interrupt_routine = (void (*)(void))(*ptrVect);
                    interrupt_routine();                                 // call the corresponding interrupt routine
                    return;
                }
            }
        }
        ptrVect++;
        ulBit <<= 1;
        iIntCnt++;
    }
}




// Process simulated interrupts
//
extern unsigned long fnSimInts(char *argv[])
{    
    unsigned long ulNewActions = 0;
    int *ptrCnt;

	if ((iInts & CHANNEL_0_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[0];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_0_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_0_SERIAL_INT;                          // interrupt has been handled
#ifdef SERIAL_INTERFACE
	            fnLogTx0((unsigned char)UART_DR_0);
                ulNewActions |= SEND_COM_0;
		        UART_FR_0 |= TXFE;   
                UART_RIS_0 |= TXRIS;                                     // simulate Tx int 
                if (UART_CR_0 & LBE) {                                   // check whether we are in tx loop back mode
			        UART_RIS_0 |= RXRIS;;                                // and Rx interrupt due to loop
		        }
                UART_MIS_0 = (UART_RIS_0 & UART_IMSC_0);
                if (UART_MIS_0 & (RXIM | TXIM)) {                        // If interrupts enabled
                    void (*interrupt_call)(void);
                    interrupt_call = (void (*)(void))VIC1_VA0R;
                    interrupt_call();
                }
#endif
            }
        }
	}

	if ((iInts & CHANNEL_1_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[1];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_1_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
#ifdef SERIAL_INTERFACE
		        iInts &= ~CHANNEL_1_SERIAL_INT;                          // interrupt has been handled
	            fnLogTx1((unsigned char)UART_DR_1);
                ulNewActions |= SEND_COM_1;
		        UART_FR_1 |= TXFE;   
                UART_RIS_1 |= TXRIS;                                     // simulate Tx int 
                if (UART_CR_1 & LBE) {                                   // check whether we are in tx loop back mode
			        UART_RIS_1 |= RXRIS;;                                // and Rx interrupt due to loop
		        }
                UART_MIS_1 = (UART_RIS_1 & UART_IMSC_1);
                if (UART_MIS_1 & (RXIM | TXIM)) {                        // If interrupts enabled
                    void (*interrupt_call)(void);
                    interrupt_call = (void (*)(void))VIC1_VA1R;
                    interrupt_call();
                }
#endif
            }
        }
	}


	if ((iInts & CHANNEL_2_SERIAL_INT) && (argv)) {
        ptrCnt = (int *)argv[2];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_2_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
#ifdef SERIAL_INTERFACE
		        iInts &= ~CHANNEL_2_SERIAL_INT;                          // interrupt has been handled
	            fnLogTx2((unsigned char)UART_DR_2);
                ulNewActions |= SEND_COM_2;
		        UART_FR_2 |= TXFE;   
                UART_RIS_2 |= TXRIS;                                     // simulate Tx int 
                if (UART_CR_2 & LBE) {                                   // check whether we are in tx loop back mode
			        UART_RIS_2 |= RXRIS;;                                // and Rx interrupt due to loop
		        }
                UART_MIS_2 = (UART_RIS_2 & UART_IMSC_2);
                if (UART_MIS_2 & (RXIM | TXIM)) {                        // If interrupts enabled
                    void (*interrupt_call)(void);
                    interrupt_call = (void (*)(void))VIC1_VA2R;
                    interrupt_call();
                }
#endif
            }
        }
	}

    if (iInts & IIC_INT0) {
        ptrCnt = (int *)argv[3];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= IIC_INT0;                                      // enough IIC interupts handled in this tick period
            }
            else {
#ifdef IIC_INTERFACE
		        iInts &= ~IIC_INT0;
                fnCallInt(1, IIC0_INT_1);                                // call interrupt handler if enabled
#endif
            }
        }
    }

    if (iInts & IIC_INT1) {
        ptrCnt = (int *)argv[4];
        if (*ptrCnt) {
            if (--(*ptrCnt) == 0) {
                iMasks |= IIC_INT1;                                      // enough IIC interupts handled in this tick period
            }
            else {
#ifdef IIC_INTERFACE
		        iInts &= ~IIC_INT1;
                fnCallInt(1, IIC1_INT_1);                                // call interrupt handler if enabled
#endif
            }
        }
    }

    return ulNewActions;
}

// Process simulated DMA
extern unsigned long fnSimDMA(char *argv[])
{
    return 0;
}

// Periodic tick (on timer 3) // dummy since the timer is handled by timer simulator
//
extern void RealTimeInterrupt(void)
{
}

// Simulate the reception of serial data by inserting bytes into the input buffer and calling interrupts
//
extern void fnSimulateSerialIn(int iPort, unsigned char *ptrDebugIn, unsigned short usLen)
{
 #ifdef SERIAL_INTERFACE
    void (*interrupt_call)(void);

    switch (iPort) {
    case 0:                                                              // UART 0
	    while (usLen--) {
            UART_RIS_0 |= RXRIS;                                         // set RAW interrupt bit
            UART_MIS_0 = (UART_RIS_0 & UART_IMSC_0);                     // mask interrupts
		    UART_DR_0 = *ptrDebugIn++;                                   // put received byte to input buffer
            if (UART_MIS_0  & RXIM) {                                    // if interrupt enabled, call ISR
                interrupt_call = (void (*)(void))VIC1_VA0R;
                interrupt_call();
            }
	    }
        break;

    case 1:                                                              // UART 1
	    while (usLen--) {
            UART_RIS_1 |= RXRIS;                                         // set RAW interrupt bit
            UART_MIS_1 = (UART_RIS_1 & UART_IMSC_1);                     // mask interrupts
		    UART_DR_1 = *ptrDebugIn++;                                   // put received byte to input buffer
            if (UART_MIS_1  & RXIM) {                                    // if interrupt enabled, call ISR
                interrupt_call = (void (*)(void))VIC1_VA1R;
                interrupt_call();
            }
	    }
        break;

    case 2:                                                              // UART 2
	    while (usLen--) {
            UART_RIS_2 |= RXRIS;                                         // set RAW interrupt bit
            UART_MIS_2 = (UART_RIS_2 & UART_IMSC_2);                     // mask interrupts
		    UART_DR_2 = *ptrDebugIn++;                                   // put received byte to input buffer
            if (UART_MIS_2  & RXIM) {                                    // if interrupt enabled, call ISR
                interrupt_call = (void (*)(void))VIC1_VA2R;
                interrupt_call();
            }
	    }
        break;

    default:
        break;
    }
 #endif
}

extern void fnSimulateModemChange(int iPort, unsigned long ulNewState, unsigned long ulOldState)
{
#ifdef SUPPORT_HW_FLOW
    switch (iPort) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    }
#endif
}

// UART Break detection simulation
//
extern void fnSimulateBreak(int iPort)
{
}

#ifdef SUPPORT_KEY_SCAN
static unsigned char fnMapPortBit(unsigned char ucRealBit)
{
    unsigned char ucBit = 0x80;
    unsigned char ucRef = 0;

    while (ucBit) {
        if (ucRealBit & ucBit) {
            break;
        }
        ucBit >>= 1;
        ucRef++;
    }
    return ucRef;
}


int iKeyPadInputs[KEY_COLUMNS][KEY_ROWS] = {0};

extern void fnSimulateKeyChange(int *intTable)
{
    memcpy(iKeyPadInputs, intTable, sizeof(iKeyPadInputs));              // copy key pad state to local set
}

static int fnColumnLow(int iColumnOutput)
{
#ifdef KEY_COLUMNS
    switch (iColumnOutput) {
    case 0:
        return (KEY_COL_OUT_1 & ~KEY_COL_OUT_PORT_1 & KEY_COL_OUT_DDR_1 && KEY_COL_OUTFUNC_PORT_1());
    #if KEY_COLUMNS > 1
    case 1:
        return (KEY_COL_OUT_2 & ~KEY_COL_OUT_PORT_2 & KEY_COL_OUT_DDR_2 && KEY_COL_OUTFUNC_PORT_2());
    #endif
    #if KEY_COLUMNS > 2
    case 2:
        return (KEY_COL_OUT_3 & ~KEY_COL_OUT_PORT_3 & KEY_COL_OUT_DDR_3 && KEY_COL_OUTFUNC_PORT_3());
    #endif
    #if KEY_COLUMNS > 3
    case 3:
        return (KEY_COL_OUT_4 & ~KEY_COL_OUT_PORT_4 & KEY_COL_OUT_DDR_4 && KEY_COL_OUTFUNC_PORT_4());
    #endif
    #if KEY_COLUMNS > 4
    case 4:
        return (KEY_COL_OUT_5 & ~KEY_COL_OUT_PORT_5 & KEY_COL_OUT_DDR_5 && KEY_COL_OUTFUNC_PORT_5());
    #endif
    #if KEY_COLUMNS > 5
    case 5:
        return (KEY_COL_OUT_6 & ~KEY_COL_OUT_PORT_6 & KEY_COL_OUT_DDR_6 && KEY_COL_OUTFUNC_PORT_6());
    #endif
    #if KEY_COLUMNS > 6
    case 6:
        return (KEY_COL_OUT_7 & ~KEY_COL_OUT_PORT_7 & KEY_COL_OUT_DDR_7 && KEY_COL_OUTFUNC_PORT_7());
    #endif
    #if KEY_COLUMNS > 7
    case 7:
        return (KEY_COL_OUT_8 & ~KEY_COL_OUT_PORT_8 & KEY_COL_OUT_DDR_8 && KEY_COL_OUTFUNC_PORT_8());
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
}

// This routine updates the ports to respect the present setting of a connected matrix key pad
//
extern void fnSimMatrixKB(void)
{
    int i, j;

    iFlagRefresh = fnPortChanges(1);                                     // synchronise with present settings
    // Check whether a column control is being driven low. If this is the case, any pressed ones in the column are seen at the row input
    for (i = 0; i < KEY_COLUMNS; i++) {
        if (fnColumnLow(i)) {
            for (j = 0; j < KEY_ROWS; j++) {
                fnSetRowInput(j, iKeyPadInputs[i][j]);
            }
        }
    }
}
#endif

extern void fnSimulateInputChange(unsigned char ucPort, unsigned char ucPortBit, int iChange)
{
    unsigned char ucBit;
    ucPortBit = (7 - ucPortBit);
    ucBit = (0x01 << ucPortBit);

    switch (ucPort) {
    case _GPIO_0:                                                        
        if (((SCU_GPIOOUT0 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN0 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO0)) || (!(SCU_PCGR1 & GPIO0))) {
            return;                                                      // reset or not clocked
        }
        if (ucBit & GPIO_DIR_0) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_0 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_0 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_0 |= ucBit;                                        // set the input high
            GPIO_DATA_0 |= ucBit;
        }
        else {
            ucPort_in_0 &= ~ucBit;                                       // set the input low
            GPIO_DATA_0 &= ~ucBit;
        }
        break;

    case _GPIO_1:                                                        
        if (((SCU_GPIOOUT1 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN1 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO1)) || (!(SCU_PCGR1 & GPIO1))) {
            return;                                                      // reset or not clocked
        }
        if (ucBit & GPIO_DIR_1) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_1 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_1 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_1 |= ucBit;                                        // set the input high
            GPIO_DATA_1 |= ucBit;
        }
        else {
            ucPort_in_1 &= ~ucBit;                                       // set the input low
            GPIO_DATA_1 &= ~ucBit;
        }
        break;

    case _GPIO_2:
        if (((SCU_GPIOOUT2 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN2 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO2)) || (!(SCU_PCGR1 & GPIO2))) {
            return;                                                      // reset or not clocked
        }
        if (ucBit & GPIO_DIR_2) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_2 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_2 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_2 |= ucBit;                                        // set the input high
            GPIO_DATA_2 |= ucBit;
        }
        else {
            ucPort_in_2 &= ~ucBit;                                       // set the input low
            GPIO_DATA_2 &= ~ucBit;
        }
        break;

    case _GPIO_3:
        if (((SCU_GPIOOUT3 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN3 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO3)) || (!(SCU_PCGR1 & GPIO3))) {
            if (((!(SCU_PRR1 & WIU)) || (!(SCU_PCGR1 & WIU))) || (!(ucBit & WIU_MR))) { // accept enabled wake up inputs
                return;                                                  // reset or not clocked
            }
        }
        if (ucBit & GPIO_DIR_3) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_3 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_3 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_3 |= ucBit;                                        // set the input high
            GPIO_DATA_3 |= ucBit;
        }
        else {
            ucPort_in_3 &= ~ucBit;                                       // set the input low
            GPIO_DATA_3 &= ~ucBit;
        }
        break;


    case _GPIO_4:
        if (((SCU_GPIOOUT4 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN5 & ucBit) {
            return;                                                      // input connected to peripheral
        }

        if (SCU_GPIOANA & ucBit) {                                       // analogue input
            return;
        }

        if ((!(SCU_PRR1 & GPIO4)) || (!(SCU_PCGR1 & GPIO4))) {
            return;                                                      // reset or not clocked
        }
        if (ucBit & GPIO_DIR_4) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_4 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_4 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_4 |= ucBit;                                        // set the input high
            GPIO_DATA_4 |= ucBit;
        }
        else {
            ucPort_in_4 &= ~ucBit;                                       // set the input low
            GPIO_DATA_4 &= ~ucBit;
        }
        break;

    case _GPIO_5:
        if (((SCU_GPIOOUT5 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN5 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO5)) || (!(SCU_PCGR1 & GPIO5))) {
            if (((!(SCU_PRR1 & WIU)) || (!(SCU_PCGR1 & WIU))) || (!(((unsigned long)ucBit << 8) & WIU_MR))) { // accept enabled wake up inputs
                return;                                                  // reset or not clocked
            }
        }
        if (ucBit & GPIO_DIR_5) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_5 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_5 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_5 |= ucBit;                                        // set the input high
            GPIO_DATA_5 |= ucBit;
        }
        else {
            ucPort_in_5 &= ~ucBit;                                       // set the input low
            GPIO_DATA_5 &= ~ucBit;
        }
        break;

    case _GPIO_6:
        if (((SCU_GPIOOUT6 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN6 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO6)) || (!(SCU_PCGR1 & GPIO6))) {
            if (((!(SCU_PRR1 & WIU)) || (!(SCU_PCGR1 & WIU))) || (!(((unsigned long)ucBit << 16) & WIU_MR))) { // accept enabled wake up inputs
                return;                                                  // reset or not clocked
            }
        }
        if (ucBit & GPIO_DIR_6) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_6 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_6 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_6 |= ucBit;                                        // set the input high
            GPIO_DATA_6 |= ucBit;
        }
        else {
            ucPort_in_6 &= ~ucBit;                                       // set the input low
            GPIO_DATA_6 &= ~ucBit;
        }
        break;

    case _GPIO_7:
        if (((SCU_GPIOOUT7 >> (2*ucPortBit)) & 0x3) >= 2) {
            return;                                                      // pin not programmed as input
        }
        if (SCU_GPIOIN7 & ucBit) {
            return;                                                      // input connected to peripheral
        }
        if ((!(SCU_PRR1 & GPIO7)) || (!(SCU_PCGR1 & GPIO7))) {
            if (((!(SCU_PRR1 & WIU)) || (!(SCU_PCGR1 & WIU))) || (!(((unsigned long)ucBit << 24) & WIU_MR))) { // accept enabled wake up inputs
                return;                                                  // reset or not clocked
            }
        }
        if (ucBit & GPIO_DIR_7) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_7 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_7 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_7 |= ucBit;                                        // set the input high
            GPIO_DATA_7 |= ucBit;
        }
        else {
            ucPort_in_7 &= ~ucBit;                                       // set the input low
            GPIO_DATA_7 &= ~ucBit;
        }
        break;

    case _GPIO_8:
        if (ucBit & GPIO_DIR_8) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_8 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_8 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_8 |= ucBit;                                        // set the input high
            GPIO_DATA_8 |= ucBit;
        }
        else {
            ucPort_in_8 &= ~ucBit;                                       // set the input low
            GPIO_DATA_8 &= ~ucBit;
        }
        break;

    case _GPIO_9:
        if (ucBit & GPIO_DIR_9) {
            return;                                                      // output
        }
        if (iChange == TOGGLE_INPUT) {
            ucPort_in_9 ^= ucBit;                                        // toggle the input state
            GPIO_DATA_9 ^= ucBit;
        }
        else if (iChange == SET_INPUT) {
            ucPort_in_9 |= ucBit;                                        // set the input high
            GPIO_DATA_9 |= ucBit;
        }
        else {
            ucPort_in_9 &= ~ucBit;                                       // set the input low
            GPIO_DATA_9 &= ~ucBit;
        }
        break;
    }
}


// Simulate buffers for Ethernet use
//
unsigned char ucSimRxABuf[1536];
unsigned char ucSimRxBBuf[1536];

unsigned char *fnGetSimTxBufferAdd(void)
{
#ifdef ETH_INTERFACE
    return (fnGetTxBufferAdd(0));
#else
    return 0;
#endif
}




#ifdef ETH_INTERFACE
    extern void EMAC_Interrupt(void);
    extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen);
#endif

extern int fnSimulateEthernetIn(unsigned char *ucData, unsigned short usLen, int iForce)
{
 #ifdef ETH_INTERFACE                                                    // we feed frames in promiscuous mode and filter them according to their details
    int iType;
    static int iFirstRec = 0;
    static STR91XF_BD *ptrStartRxBd;
    STR91XF_BD *ptrRxBd;
    unsigned short usFrameLength = usLen;
    int iFirstBuffer = 1;

    if ((!iForce) && (!(iType = fnCheckEthernetMode(ucData, usLen)))) {  // if we are not in promiscuous mode and the MAC address is not defined to be received we ignore the frame
        return 0;
    }

    ptrRxBd = (STR91XF_BD *)ENET_RXNDAR;                                 // present receive buffer description
    if ((ptrRxBd->bd_dma_control & DMA_XFERCOUNT) < usLen) {             // check that the buffer has enough space
        usLen = (unsigned short)(ptrRxBd->bd_dma_control & DMA_XFERCOUNT);
    }
    ptrRxBd->bd_dma_control &= ~DMA_XFERCOUNT;
    ptrRxBd->bd_dma_control |= (usLen + 4);                              // size plus crc etc.
    ptrRxBd->bd_packet_status &= ~RX_VALID_MASK;                         // change ownership
    uMemcpy((unsigned char *)ptrRxBd->bd_dma_start_address, ucData, usLen); // copy frame to input buffer                       
    ENET_ISR |= RX_ENTRY_EN;                                              // set interrupt flag
    EMAC_Interrupt(); 	                                                 // simulate frame received
    if (ptrRxBd->bd_dma_control & NXT_EN) {                              // go to next buffer descriptor
        ENET_RXNDAR = (unsigned long)(ptrRxBd->bd_dma_next);
    }
    return 1;
 #else
    return 0;
 #endif
}


// We can update port state displays if we want
//
extern void fnSimPorts(void)
{
    static unsigned long ulGPIOPER[PORTS_AVAILABLE] = {0};
    unsigned long ulNewPeriph;
    int iPort = 0;

    while (iPort < PORTS_AVAILABLE) {
        ulNewPeriph = fnGetPresentPortPeriph(iPort+1);                  
        if (ulGPIOPER[iPort] != ulNewPeriph) {
            ulGPIOPER[iPort] = ulNewPeriph;
            iFlagRefresh = PORT_CHANGE;                                  // ensure that ports are updated when a peripheral function changes
        }
        iPort++;
    }
}


static unsigned char fnGetPorts(int port_ref, unsigned char *ptr)
{
    unsigned char ucPeripherals = ucGetPortType(port_ref, GET_OUTPUT_MASK, 0);
    if (ptr != 0) {
        unsigned char ucBit = 0x01;
        int iBit = 0;

        while (ucBit) {
            *ptr++ = ucGetPortType(port_ref, GET_PER_TYPE, iBit++);
            ucBit <<= 1;
        }
    }
    return ucPeripherals;
}

static void fnWakeupInts(unsigned long ulWakeup, unsigned long ulNewWakeups)
{
    unsigned long ulChanges;

    if (!(WIU_CTRL & WIUINT_EN)) return;                                 // no interrupt enabled

    ulChanges = ulWakeup ^ ulNewWakeups;

    ulChanges &= WIU_MR;                                                 // mask change interrupts
    if (ulChanges) {
        unsigned long ulBit = 0x0000002;
        while (ulChanges) {
            if (ulBit & ulChanges & ulNewWakeups & WIU_TR) {             // rising edge interrupt
                WIU_PR |= ulBit;
                fnCallInt(1, WIU_ALL_INT_1);                             // call interrupt handler if enabled
            }
            else if (ulBit & ulChanges & ~ulNewWakeups & ~WIU_TR) {      // falling edge interrupt
                WIU_PR |= ulBit;
                fnCallInt(1, WIU_ALL_INT_1);                             // call interrupt handler if enabled
            }
            ulChanges &= ~ulBit;
            ulBit <<= 1;
        }
    }
}

#define UPDATE(port, ddr, pullup, backup_port, backup_ddr, backup_per, port_ref)  \
    ucMask = fnGetPorts(port_ref, 0);                       \
    port &= (ddr & ~ucMask);                                \
    port |= (~ddr & pullup);                                \
    if (ucMask != backup_per) {                             \
        backup_per = ucMask;                                \
        fnGetPorts(port_ref, ucPortFunctions[port_ref]);    \
        iRtn |= PORT_CHANGE;                                \
    }                                                       \
    if ((port != backup_port) || (ddr != backup_ddr)) {     \
        backup_port = port;                                 \
        backup_ddr = ddr;                                   \
        iRtn |= PORT_CHANGE;                                \
    } 

extern int fnPortChanges(int iForce)
{
    static unsigned long ucGPIO0 = 0, ucGPIO1 = 0, ucGPIO2 = 0, ucGPIO3 = 0, ucGPIO4 = 0, ucGPIO5 = 0, ucGPIO6 = 0, ucGPIO7 = 0, ucGPIO8 = 0, ucGPIO9 = 0;
    static unsigned long ucPort_out_0 = 0, ucPort_out_1 = 0, ucPort_out_2 = 0, ucPort_out_3 = 0, ucPort_out_4 = 0, ucPort_out_5 = 0, ucPort_out_6 = 0, ucPort_out_7 = 0, ucPort_out_8 = 0, ucPort_out_9 = 0;
    static unsigned long ucGPIODIR0 = 0, ucGPIODIR1 = 0, ucGPIODIR2 = 0, ucGPIODIR3 = 0, ucGPIODIR4 = 0, ucGPIODIR5 = 0, ucGPIODIR6 = 0, ucGPIODIR7 = 0, ucGPIODIR8 = 0, ucGPIODIR9 = 0;
    static unsigned long ucGPIO_per_0 = 0, ucGPIO_per_1 = 0, ucGPIO_per_2 = 0, ucGPIO_per_3 = 0, ucGPIO_per_4 = 0, ucGPIO_per_5 = 0, ucGPIO_per_6 = 0, ucGPIO_per_7 = 0, ucGPIO_per_8 = 0, ucGPIO_per_9 = 0;
    static unsigned long ulWakeup = 0;
    unsigned char ucMask;
    unsigned long ulNewWakeups;

    int iRtn = iFlagRefresh;
    iFlagRefresh = 0;

    UPDATE(GPIO_DATA_0, GPIO_DIR_0, ucPort_in_0, ucGPIO0, ucGPIODIR0, ucGPIO_per_0, _GPIO_0);
    UPDATE(GPIO_DATA_1, GPIO_DIR_1, ucPort_in_1, ucGPIO1, ucGPIODIR1, ucGPIO_per_1, _GPIO_1);    
    UPDATE(GPIO_DATA_2, GPIO_DIR_2, ucPort_in_2, ucGPIO2, ucGPIODIR2, ucGPIO_per_2, _GPIO_2);
    UPDATE(GPIO_DATA_3, GPIO_DIR_3, ucPort_in_3, ucGPIO3, ucGPIODIR3, ucGPIO_per_3, _GPIO_3);
    UPDATE(GPIO_DATA_4, GPIO_DIR_4, ucPort_in_4, ucGPIO4, ucGPIODIR4, ucGPIO_per_4, _GPIO_4);
    UPDATE(GPIO_DATA_5, GPIO_DIR_5, ucPort_in_5, ucGPIO5, ucGPIODIR5, ucGPIO_per_5, _GPIO_5); 
    UPDATE(GPIO_DATA_6, GPIO_DIR_6, ucPort_in_6, ucGPIO6, ucGPIODIR6, ucGPIO_per_6, _GPIO_6);
    UPDATE(GPIO_DATA_7, GPIO_DIR_7, ucPort_in_7, ucGPIO7, ucGPIODIR7, ucGPIO_per_7, _GPIO_7);
    UPDATE(GPIO_DATA_8, GPIO_DIR_8, ucPort_in_8, ucGPIO8, ucGPIODIR8, ucGPIO_per_8, _GPIO_8);
    UPDATE(GPIO_DATA_9, GPIO_DIR_9, ucPort_in_9, ucGPIO9, ucGPIODIR9, ucGPIO_per_9, _GPIO_9);    

    // handle wake-up interrupts
    ulNewWakeups = ((ucGPIO3 & 0xfc) | (ucGPIO5 << 8) | (ucGPIO6 << 16) | (ucGPIO7 << 24));
    if (!(SCU_PRR1 & WIU)) {
        WIU_CTRL = WIU_MR = WIU_TR = 0;                                  // If WIU in reset, reset registers
    }
    if (ulWakeup != ulNewWakeups) {                                      // if a change has been detected
        fnWakeupInts(ulWakeup, ulNewWakeups);
        ulWakeup = ulNewWakeups;
    }

    return iRtn;
}

extern unsigned long fnGetPresentPortState(int portNr)
{
    switch (portNr-1) {
    case _GPIO_0:
    return (GPIO_DATA_0);                               
    case _GPIO_1:
    return (GPIO_DATA_1);                                 
    case _GPIO_2:
    return (GPIO_DATA_2);                                
    case _GPIO_3:
    return (GPIO_DATA_3);                                  
    case _GPIO_4:
    return (GPIO_DATA_4);                                 
    case _GPIO_5:
    return (GPIO_DATA_5);                                  
    case _GPIO_6:
    return (GPIO_DATA_6);                                  
    case _GPIO_7:
    return (GPIO_DATA_7);                                  
    case _GPIO_8:
    return (GPIO_DATA_8);  
    case _GPIO_9:
    return (GPIO_DATA_9);  
    default:
        return 0;
    }
}


static unsigned char ucGetPortType(int portNr, int iRequest, int i)
{
    unsigned char ucPeripherals = 0;
    unsigned char ucBit = 0x01;
    unsigned char ucType;

    switch (portNr) {
    case _GPIO_0:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT0 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN0 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_1:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT1 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN1 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_2:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT2 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN2 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_3:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT3 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN3 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
                if ((GET_OUTPUT_MASK != iRequest) && (i >= 2) && (WIU_MR & (WUM_EXTINT2_MASK << (i-2)))) {// check whether programmed as wake up input
                    if (SCU_PCGR1 & SCU_PRR1 & WIU) {                    // check wake up module enabled
                        ucPeripherals |= ucBit;                          // signal peripheral rather than input in this case
                    }
                }
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_4:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT4 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN4 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else if (SCU_GPIOANA & (0x01 << i)) {                        // if set for analogue function
                ucPeripherals |= ucBit;
            }
            else {
                ucType = 0;
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_5:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT5 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN5 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;                                              // input type
                if ((GET_OUTPUT_MASK != iRequest) &&  (WIU_MR & (WUM_EXTINT8_MASK << i))) { // check whether programmed as wake up input
                    if (SCU_PCGR1 & SCU_PRR1 & WIU) {                    // check wake up module enabled
                        ucPeripherals |= ucBit;                          // signal peripheral rather than input in this case
                    }
                }
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_6:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT6 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN6 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
                if ((GET_OUTPUT_MASK != iRequest) && (WIU_MR & (WUM_EXTINT16_MASK << i))) { // check whether programmed as wake up input
                    if (SCU_PCGR1 & SCU_PRR1 & WIU) {                    // check wake up module enabled
                        ucPeripherals |= ucBit;                          // signal peripheral rather than input in this case
                    }
                }
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_7:
        while (i < 8) {
            ucType = (unsigned char)((SCU_GPIOOUT7 >> (2*i)) & 0x03);
            if ((GET_OUTPUTS == iRequest) && (ucType == 1)) {
                ucPeripherals |= ucBit;                                  // pin used as output
            }
            else if ((ucType > 1) || ((ucType == 0) && (SCU_GPIOIN7 & (0x01 << i)))) {
                ucPeripherals |= ucBit;                                  // pin used for peripheral function
                if (ucType == 0) {
                    ucType++;
                }
            }
            else {
                ucType = 0;
                if ((GET_OUTPUT_MASK != iRequest) && (WIU_MR & (WUM_EXTINT24_MASK << i))) { // check whether programmed as wake up input
                    if (SCU_PCGR1 & SCU_PRR1 & WIU) {                    // check wake up module enabled
                        ucPeripherals |= ucBit;                          // signal peripheral rather than input in this case
                        ucType = 1;
                    }
                }
            }
            if (GET_PER_TYPE == iRequest) {
                return (ucType + 1);
            }
            ucBit <<= 1;
            i++;
        }
        break;

    case _GPIO_8:
        if (GET_PER_TYPE == iRequest) {
            return 0;
        }
        if (GET_OUTPUTS == iRequest) {
            ucPeripherals = (unsigned char)GPIO_DIR_8;                   // pins used as output depends on DIR
        }
        break;

    case _GPIO_9:
        if (GET_PER_TYPE == iRequest) {
            return 0;
        }
        if (GET_OUTPUTS == iRequest) {
            ucPeripherals = (unsigned char)GPIO_DIR_9;                   // pins used as output depends on DIR
        }
        break;
    }
    return ucPeripherals;
}


extern unsigned long fnGetPresentPortPeriph(int portNr)
{
    return (ucGetPortType(portNr-1, GET_PERIPHERALS, 0));
}

extern unsigned long fnGetPresentPortDir(int portNr)
{
    switch (--portNr) {
    case _GPIO_0:
    return (GPIO_DIR_0 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_1:
    return (GPIO_DIR_1 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_2:
    return (GPIO_DIR_2 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_3:
    return (GPIO_DIR_3 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_4:
    return (GPIO_DIR_4 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_5:
    return (GPIO_DIR_5 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_6:
    return (GPIO_DIR_6 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_7:
    return (GPIO_DIR_7 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_8:
    return (GPIO_DIR_8 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    case _GPIO_9:
    return (GPIO_DIR_9 & ucGetPortType(portNr, GET_OUTPUTS, 0));         // Non outputs returned as zeros
    default:
        return 0;
    }
}


extern void fnSimulateLinkUp(void)
{
#ifdef ETH_INTERFACE
    ASSERT_PHY_INT();
    fnPortChanges(1);
    RELEASE_PHY_INT();
    fnPortChanges(1);
    fnUpdateIPConfig();                                                  // Update display in simulator
#endif
}

// We can simulate timers during a tick here if required
extern int fnSimTimers(void)
{
    if (WDG_CR & WATCHDOG_WE) {                                          // watchdog timer enabled
        unsigned long ulTickCountdown = (((PCLK / (((WDG_PR & 0xff)+1))) * TICK_RESOLUTION)/1000);
        if (ulTickCountdown >= WDG_CNT) {
            WDG_CNT = 0;
            WDG_SR |= WATCHDOG_EC;
            return RESET_CARD_WATCHDOG;                                  // signal that the watchdog timer has fired and the board should be reset
        }
        else {
            WDG_CNT -= ulTickCountdown;
        }
    }

    if (TIM_CR1_0 & COUNTER_ENABLE) {
        unsigned long ulCountValue;
        unsigned long ulNewCount;
        
        if ((TIM_CR2_0 & CLOCK_CONTROL_MASK) == 0) {                     // check for high frequency (proabably used as clock generator
            ulCountValue = 0xffff;
        }
        else {
            ulCountValue = PCLK / (TIM_CR2_0 & CLOCK_CONTROL_MASK) / (1000/TICK_RESOLUTION); // count increment in tick period
        }
        ulNewCount = TIM_CNTR_0 + ulCountValue;

        if ((TIM_CNTR_0 < TIM_OC2R_0) && (ulNewCount >= TIM_OC2R_0)) {
            TIM_CNTR_0 = 0xfffc;                                         // timer reset value
            if (TIM_CR2_0 & OC2IE) {                                     // if interrupt is enabled
                fnCallInt(0, TIMER0_INT_0);                              // call interrupt handler if enabled
            }
        }
        else {
            if ((ulNewCount > 0xffff) || (ulNewCount >= TIM_OC2R_0)) {
                TIM_CNTR_0 = 0xfffc;                                     // timer reset value
                if (TIM_CR2_0 & OC2IE) {                                 // if interrupt is enabled
                    fnCallInt(0, TIMER0_INT_0);                          // call interrupt handler if enabled
                }
            }
            else {
                TIM_CNTR_0 = (unsigned short)ulNewCount;
            }
        }
    }

    if (TIM_CR1_1 & COUNTER_ENABLE) {
        unsigned long ulCountValue;
        unsigned long ulNewCount;
        
        if ((TIM_CR2_1 & CLOCK_CONTROL_MASK) == 0) {                     // check for high frequency (proabably used as clock generator
            ulCountValue = 0xffff;
        }
        else {
            ulCountValue = PCLK / (TIM_CR2_1 & CLOCK_CONTROL_MASK) / (1000/TICK_RESOLUTION); // count increment in tick period
        }
        ulNewCount = TIM_CNTR_1 + ulCountValue;

        if ((TIM_CNTR_1 < TIM_OC2R_1) && (ulNewCount >= TIM_OC2R_1)) {
            TIM_CNTR_1 = 0xfffc;                                         // timer reset value
            if (TIM_CR2_1 & OC2IE) {                                     // if interrupt is enabled
                fnCallInt(0, TIMER1_INT_0);                              // call interrupt handler if enabled
            }
        }
        else {
            if ((ulNewCount > 0xffff) || (ulNewCount >= TIM_OC2R_1)) {
                TIM_CNTR_1 = 0xfffc;                                     // timer reset value
                if (TIM_CR2_1 & OC2IE) {                                 // if interrupt is enabled
                    fnCallInt(0, TIMER1_INT_0);                          // call interrupt handler if enabled
                }
            }
            else {
                TIM_CNTR_1 = (unsigned short)ulNewCount;
            }
        }
    }

    if (TIM_CR1_2 & COUNTER_ENABLE) {
        unsigned long ulCountValue = PCLK / (TIM_CR2_2 & CLOCK_CONTROL_MASK) / (1000/TICK_RESOLUTION); // count increment in tick period
        unsigned long ulNewCount = TIM_CNTR_2 + ulCountValue;

        if ((TIM_CNTR_2 < TIM_OC2R_2) && (ulNewCount >= TIM_OC2R_2)) {
            TIM_CNTR_2 = 0xfffc;                                         // timer reset value
            if (TIM_CR2_2 & OC2IE) {                                     // if interrupt is enabled
                fnCallInt(0, TIMER2_INT_0);                              // call interrupt handler if enabled
            }
        }
        else {
            if ((ulNewCount > 0xffff) || (ulNewCount >= TIM_OC2R_2)) {
                TIM_CNTR_2 = 0xfffc;                                     // timer reset value
                if (TIM_CR2_2 & OC2IE) {                                 // if interrupt is enabled
                    fnCallInt(0, TIMER2_INT_0);                          // call interrupt handler if enabled
                }
            }
            else {
                TIM_CNTR_2 = (unsigned short)ulNewCount;
            }
        }
    }

    if (TIM_CR1_3 & COUNTER_ENABLE) {
        unsigned long ulCountValue = PCLK / (TIM_CR2_3 & CLOCK_CONTROL_MASK) / (1000/TICK_RESOLUTION); // count increment in tick period
        unsigned long ulNewCount = TIM_CNTR_3 + ulCountValue;

        if ((TIM_CNTR_3 < TIM_OC2R_3) && (ulNewCount >= TIM_OC2R_3)) {
            TIM_CNTR_3 = 0xfffc;                                         // timer reset value
            if (TIM_CR2_3 & OC2IE) {                                     // if interrupt is enabled
                fnCallInt(0, TIMER3_INT_0);                              // call interrupt handler if enabled
            }
        }
        else {
            if ((ulNewCount > 0xffff) || (ulNewCount >= TIM_OC2R_3)) {
                TIM_CNTR_3 = 0xfffc;                                     // timer reset value
                if (TIM_CR2_3 & OC2IE) {                                 // if interrupt is enabled
                    fnCallInt(0, TIMER3_INT_0);                          // call interrupt handler if enabled
                }
            }
            else {
                TIM_CNTR_3 = (unsigned short)ulNewCount;
            }
        }
    }
    return 0;
}

#endif




