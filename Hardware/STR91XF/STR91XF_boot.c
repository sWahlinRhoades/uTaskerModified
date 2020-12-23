/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:        STR91XF_boot.c [ST ARM9]
   Project:     Single Chip Embedded Internet - boot loader hardware support
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2011
   *********************************************************************/

#ifdef _STR91XF

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */


#ifdef _WINDOWS
  #include "config.h"

  #include "../../WinSim\WinSim.h"
  #define INITHW  extern
  extern void fnOpenDefaultHostAdapter(void);
  #define __interrupt__
  #define SIM_LOCK MCF_CLOCK_SYNSR |= PLLOCKED;
  #define _LITTLE_ENDIAN
  #define SIM_DMA_3 fnSimDMA(0);
  #define BACKDOOR_FLASH 0

  NETWORK_PARAMETERS network;
#else
  #define OPSYS_CONFIG                                                   // this module owns the operating system configuration
  #define INITHW  static
  #define SIM_LOCK
  #define SIM_DMA_3
  #define BACKDOOR_FLASH (CFM_FLASH)
  #ifdef _GNU
    #define __interrupt__
  #endif
  #include "config.h"
#endif


// This routine is called to reset the card
//
extern void fnResetBoard(void)
{
    WDG_PR = 0;                                                          // set shortest watchdog time and enable watchdog in case not alread enabled
    WDG_VR = 0;
    WDG_CR = WATCHDOG_WE;
    WDG_KR = WDT_FEED1;
    WDG_KR = WDT_FEED2;
}


// CRC-16 routine
//
extern unsigned short fnCRC16(unsigned short usCRC, unsigned char *ptrInput, unsigned short usBlockSize)
{
    while (usBlockSize--) {
        usCRC = (unsigned char)(usCRC >> 8) | (usCRC << 8);
        usCRC ^= *ptrInput++;
        usCRC ^= (unsigned char)(usCRC & 0xff) >> 4;
        usCRC ^= (usCRC << 8) << 4;
        usCRC ^= ((usCRC & 0xff) << 4) << 1;
    }
    return usCRC;
}

// Boot initialisation doesn't program the PLL. It copies the data required by the boot program and fills the exception vector with default interrupt hanlder
//
extern void str91xf_boot_init(void)
{
    SCU_PLLCONF = (PLL_SETTING | PLL_ENABLE);                            // Enable the PLL with required setting
    while (!(SCU_SYSSTATUS & PLL_LOCK)) {}                               // wait for the PLL to lock before continuing
    SCU_CLKCNTR &= ~(MCLKSEL_OSC | MCLKSEL_RTC);                         // switch to PLL clock and BRCLK = MASTER/2

    SCU_SCR0 &= ~(WSR_AHB | WSR_DTCM);                                   // remove wait states from SRAM
    SCU_SCR0 |= EN_PFQBC;                                                // Ensure FLASH bypass enabled

    SCU_PCGR1 |= (WATCHDOG);                                             // enable clocks to watchdog
    SCU_PRR1  |= (WATCHDOG);                                             // take watchdog module out of reset
}

#ifndef COMPILE_IAR                                                      // IAR uses intrinsic function
static int iInterruptLevel = 0;

// Routine to disable interrupts during critical regions
//
void uDisable_Interrupt()
{
#ifndef _WINDOWS
    asm_int_off();
#endif
    iInterruptLevel++;                                                   // Monitor the level of interrupt nesting
}

// Routine to re-enable interrupts on leaving a critical region
//
void __enable_interrupt()                                                // interrupts are ALWAYS off when entering
{
#ifdef _WINDOWS
    if (!iInterruptLevel) {
        *(int *)0 = 0;                                                   // basic error - casue exception
    }
#endif
    if (!(--iInterruptLevel)) {                                          // only when no more interrupt nesting,
#ifndef _WINDOWS
        asm_int_on();
#endif
    }
}
#endif

// memcpy implementation
//
extern void *uMemcpy(void *ptrTo, const void *ptrFrom, size_t Size)
{
    void *buffer = ptrTo;
    unsigned char *ptr1 = (unsigned char *)ptrTo;
    unsigned char *ptr2 = (unsigned char *)ptrFrom;

    while (Size--) {
        *ptr1++ = *ptr2++;
    }

    return buffer;
}

#ifdef _WINDOWS
  #define PROG_SIZE  100
#else
  #define PROG_SIZE  80                                                  // optimised from experience
#endif


static void *fnCopyProgram(unsigned char *ptrFrom, int iSize, unsigned short usProgSpace[])
{
    unsigned char *ptrThumb = ptrFrom;
    int i = 0;

    ptrThumb--;                                                          // real address of thumb routine

    while (i < iSize) {                                                  // copy program to SRAM
        usProgSpace[i++] = *(unsigned short*)ptrThumb;
        ptrThumb += 2;
    }
    ptrThumb = (unsigned char *)usProgSpace;
    ptrThumb++;                                                          // create a thumb call
    return (ptrThumb);
}

// This routine to command FLASH - we put it in SRAM
// Warning: this is designed for little endian Thumb mode ONLY
//
static int fnFlashFlash(volatile unsigned short *usAddressPointer, volatile unsigned short *usWritePointer, unsigned short usValue)
{
#ifdef _WINDOWS
    unsigned short usCommandValue = *usAddressPointer;
#endif
    int iErrorStatus;

    *usAddressPointer = CUI_PROGRAM_SETUP;                               // prepare sector to program
    *usWritePointer = usValue;                                           // word to program
#ifndef _WINDOWS
    while (!(*usAddressPointer & BSR_PECS_READY)) {                      // wait until ready
    }
    iErrorStatus = *usAddressPointer;                                    // read the status after the write
#else
    iErrorStatus = BSR_PECS_READY;
#endif

    *usAddressPointer = CUI_CLEAR_STATUS_REGISTER;                       // clear status register
    *usAddressPointer = CUI_READ_ARRAY;                                  // completed so switch back to read mode
#ifdef _WINDOWS
    *usAddressPointer = usCommandValue;
    *usWritePointer = usValue;
#endif
    return (iErrorStatus & (BSR_PS_FAILURE | BSR_SP_PROTECTION_ERROR));
}


// The address should be even or it may fail (it is assumed that any protection has already been removed)
//
#if defined (COMPILE_IAR) && !defined (COMPILE_IAR_4_4)                  // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
 #pragma optimize=2
#endif
static int fnWriteWordFlash(unsigned short *ptrAdd, unsigned short usValue)
{
#ifdef _WINDOWS
    #define PROG_WORD_SIZE 200                                           // adequate space for the small program
#else
    #define PROG_WORD_SIZE 150                                           // adequate space for the small program
#endif
    static unsigned short usProgSpace[PROG_WORD_SIZE];                   // make space for the routine on stack (this will have a word boundary)
    static int (*fnRAM_code)(volatile unsigned short *, volatile unsigned short *, unsigned short) = 0;
    volatile unsigned short *usAddressPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrAdd & ~0x03);
    volatile unsigned short *usWordPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrAdd & ~0x01);
    int iErrorStatus;

    if (!fnRAM_code) {                                                   // real address of code in thumb mode
        fnRAM_code = (int(*)(volatile unsigned short *, volatile unsigned short *, unsigned short))(fnCopyProgram((unsigned char *)fnFlashFlash, PROG_WORD_SIZE, usProgSpace));
    }

 #ifdef _WINDOWS
    usWordPointer = (volatile unsigned short *)fnGetFlashAdd((unsigned char *)usWordPointer);
    usAddressPointer = (volatile unsigned short *)fnGetFlashAdd((unsigned char *)usAddressPointer);

    if ((*usWordPointer != 0xffff) && (usValue != *usWordPointer)) {
        ptrAdd = 0;                                                      // this will fail so signal it when debugging
        *ptrAdd = 0;
    }
 #endif

    uDisable_Interrupt();                                                // protect this region from interrupts
    iErrorStatus = fnRAM_code(usAddressPointer, usWordPointer, usValue); // commit the word to FLASH and return the state
    uEnable_Interrupt();                                                 // safe to accept interrupts again

    return (iErrorStatus);                                               // return the state
}

// This routine to delete FLASH sectors - we put it in SRAM
// Warning: this is designed for little endian Thumb mode ONLY
//
static int fnDelFlashSect(volatile unsigned short *ptrSector)
{
#ifdef _WINDOWS
    unsigned long ulSectorLength = FLASH_GRANULARITY;
    unsigned char *ptrSectData = (unsigned char *)ptrSector;
#endif
    int iErrorStatus;

    *ptrSector = CUI_SECTOR_ERASE_SETUP;
    *ptrSector = CUI_ERASE_CONFIRM;
#ifndef _WINDOWS
    while (!(*ptrSector & BSR_PECS_READY)) {                             // wait until ready
    }
    iErrorStatus = (*ptrSector & (BSR_ES_FAILURE | BSR_PS_FAILURE | BSR_SP_PROTECTION_ERROR));
#else
    iErrorStatus = 0;
#endif

    *ptrSector = CUI_CLEAR_STATUS_REGISTER;                              // clear status register
    *ptrSector = CUI_READ_ARRAY;                                         // completed

#ifdef _WINDOWS
    while (ulSectorLength--) {
        *ptrSectData++ = 0xff;                                           // delete the sector contents
    }
#endif
    return iErrorStatus;                                                 // OK
}

// This routine runs from SRAM and is designed for little endian Thumb
//
static void fnProtect(volatile unsigned short *usSectorPointer, unsigned short usCommand)
{
#ifdef _WINDOWS
    unsigned short usOriginalFlash = *usSectorPointer;
#endif
    *usSectorPointer = CUI_PROT_LEVEL1_CONFIG_SETUP;
    *usSectorPointer = usCommand;
    *usSectorPointer = CUI_READ_ARRAY;                                   // completed
#ifdef _WINDOWS
    *usSectorPointer = usOriginalFlash;
#endif
}

// Protect or unprotect sectors in FLASH
//
#if defined (COMPILE_IAR) && !defined (COMPILE_IAR_4_4)                  // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
 #pragma optimize=2
#endif
extern void fnProtectFlash(unsigned char *ptrSector, unsigned char ucProtection)
{
#ifdef _WINDOWS
    #define PROG_WORD_SIZE_1 200
#else
    #define PROG_WORD_SIZE_1 30                                          // adequate space for the small program
#endif
    static unsigned short usProgSpace[PROG_WORD_SIZE_1];                 // make space for the routine on stack (this will have a word boundary)
    static void (*fnRAM_code)(volatile unsigned short *, unsigned short) = 0;
    volatile unsigned short *usSectorPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrSector & ~0x03);
    unsigned short usCommand;

    #ifdef _WINDOWS
    usSectorPointer = (volatile unsigned short *)((unsigned long)ucFLASH + (unsigned long)usSectorPointer - (unsigned long)OFFSET_BANK0);
    #endif

    if (!fnRAM_code) {                                                   // real address of code in thumb mode
        fnRAM_code = (void(*)(volatile unsigned short *, unsigned short))(fnCopyProgram((unsigned char *)fnProtect, PROG_WORD_SIZE_1, usProgSpace));
    }

    if (UNPROTECT_SECTOR == ucProtection) {
        usCommand = CUI_UNPROTECT_LEVEL1_CONFIRM;
    }
    else {
        usCommand = CUI_PROTECT_LEVEL1_CONFIRM;
    }

    uDisable_Interrupt();                                                // protect this region from interrupts
    fnRAM_code(usSectorPointer, usCommand);
    uEnable_Interrupt();                                                 // safe to accept interrupts again
}


// Erase the sectore in which the address is in
//
#if defined (COMPILE_IAR) && !defined (COMPILE_IAR_4_4)                  // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
 #pragma optimize=2
#endif
extern unsigned long fnEraseFlash(unsigned char *ptrSector_comp)
{
    volatile unsigned short *ptrSector = (volatile unsigned short *)ptrSector_comp;
#ifdef SPI_FILE_SYSTEM
    static const unsigned char ucWriteEnable[] = {M95XXX_WRITE_ENABLE, TERMINATE_WRITE}; // enable write
    unsigned short ucDeleted = 0;
    unsigned char *ptrEEPROM = (unsigned char *)ptrSector;
    unsigned short usTemp = (unsigned short)ptrEEPROM;

    usTemp &= ~(EEPROM_PAGE_SIZE-1);                                     // We want to erase a block (program them to 0xff)
    ptrEEPROM = (unsigned char *)usTemp;
    while (ucDeleted < FILE_GRANULARITY) {
        fnWaitWriteComplete();                                           // wait until free to write
 #ifdef SPI_INTERFACE
        fnWrite(SPI_handle, (unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
        fnWriteBytesEEPROM(ptrEEPROM, ucDel, EEPROM_PAGE_SIZE);          // delete a page
 #else
        fnSendSPIMessage((unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
        fnWriteBytesEEPROM(ptrEEPROM, 0, EEPROM_PAGE_SIZE);              // delete an eeprom page
 #endif
        ucDeleted += EEPROM_PAGE_SIZE;
        ptrEEPROM += EEPROM_PAGE_SIZE;
    }
    return 0;
#else
    #ifdef _WINDOWS
    #define PROG_WORD_SIZE_2 300
    #else
    #define PROG_WORD_SIZE_2 100                                         // adequate space for the small program
    #endif
    static unsigned short usProgSpace[PROG_WORD_SIZE_2];                 // make space for the routine on stack (this will have a word boundary)
    static int (*fnRAM_code)(volatile unsigned short *) = 0;
    volatile unsigned short *usSectorPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrSector & ~0x03);
    int iErrorStatus;

    fnProtectFlash(ptrSector_comp, UNPROTECT_SECTOR); // Automatically remove protection

    #ifdef _WINDOWS
    usSectorPointer = (volatile unsigned short *)fnGetFlashAdd((unsigned char *)usSectorPointer);
    #endif

    if (!fnRAM_code) {                                                   // real address of code in thumb mode
        fnRAM_code = (int(*)(volatile unsigned short *))(fnCopyProgram((unsigned char *)fnDelFlashSect, PROG_WORD_SIZE_2, usProgSpace));
    }

    uDisable_Interrupt();                                                // protect this region from interrupts
    iErrorStatus = fnRAM_code(usSectorPointer);
    uEnable_Interrupt();                                                 // safe to accept interrupts again

    return iErrorStatus;                                                 // OK
#endif
}


// RAW write of data to non-volatile memory
//
int fnWriteBytesFlash(unsigned char *ucDestination, unsigned char *ucData, MAX_FILE_LENGTH Length)
{
#ifdef SPI_FILE_SYSTEM                                                   // simulate the same using SPI EEPROM
    static const unsigned char ucWriteEnable[] = {M95XXX_WRITE_ENABLE, TERMINATE_WRITE}; // enable write
    unsigned short usPageLength;
    while (Length) {
        usPageLength = ((unsigned short)ucDestination) & (EEPROM_PAGE_SIZE-1); // calculate the page length based on EEPROM page boundaries
        usPageLength = EEPROM_PAGE_SIZE - usPageLength;
        if (Length < usPageLength) {
            usPageLength = (unsigned short)Length;
        }
        fnWaitWriteComplete();
  #ifdef SPI_INTERFACE
        fnWrite(SPI_handle, (unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
        fnWriteBytesEEPROM(ucDestination, ucData, usPageLength);           // write a page
  #else
        fnSendSPIMessage((unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
        fnWriteBytesEEPROM(ucDestination, ucData, usPageLength);           // write a page
  #endif
        Length -= usPageLength;
        ucData += usPageLength;
        ucDestination += usPageLength;
    }

    return 0;
#else
    unsigned short usValue;

    // The STR91XF writes always in words so we must ensure that the data is correctly aligned and filled
    while (Length--) {
  #if defined _LITTLE_ENDIAN || defined (_WINDOWS)                       // Little-endian operation
        if ((CAST_POINTER_ARITHMETIC)ucDestination & 0x01) {
            ucDestination--;
            usValue = *(fnGetFlashAdd(ucDestination));
            usValue |= ((*ucData++) << 8);
        }
        else {
            usValue = *ucData++;
            if (Length) {
              Length--;
              usValue |= ((*ucData++) << 8);
            }
            else {
              usValue |= (*(fnGetFlashAdd(ucDestination+1)))<<8;
            }
        }
  #else                                                                 // Big-endian operation
        if ((CAST_POINTER_ARITHMETIC)ucDestination & 0x01) {
            ucDestination--;
            Length--;
            usValue = *ucData++;
            usValue |= (*(fnGetFlashAdd(ucDestination)) << 8);
        }
        else {
            usValue = *ucData++;
            usValue <<= 8;
            if (Length) {
              Length--;
              usValue |= *ucData++;
            }
            else {
              usValue |= *(fnGetFlashAdd(ucDestination+1));
            }
        }
  #endif

      if (fnWriteWordFlash((unsigned short *)ucDestination, usValue)) return 1;

      ucDestination += 2;
    }

    return 0;
#endif
}



extern void fnGetPars(unsigned char *ParLocation, unsigned char *ptrValue, MAX_FILE_LENGTH Size)
{
#ifdef SPI_FILE_SYSTEM
    unsigned char usRead[4];                                             // our file sytem is in external EEPROM via SPI bus.
    usRead[0] = M95XXX_READ_MEMORY;
    usRead[1] = (unsigned char)((unsigned short)(ParLocation)>> 8);
    usRead[2] = (unsigned char)((unsigned short)ParLocation);
    usRead[3] = PREPARE_READ;
    fnWaitWriteComplete();                                               // ensure the chip is ready to be read
  #ifdef SPI_INTERFACE
    fnWrite(SPI_handle, usRead, sizeof(usRead));                         // set the read address
    fnRead(SPI_handle, ptrValue, usSize);                                // read from the device to the return buffer
  #else
    fnSendSPIMessage(usRead, sizeof(usRead));                            // send immediately
    fnGetSPI_bytes(ptrValue, Size);                                      // fetch immediately
  #endif
#else
    uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), Size);                 // The STR91X uses a file system in FLASH with no access restrictions so we can simply copy the data
#endif
}

extern void Undefined_Handler(void)
{
}

extern void SWI_Handler(void)
{
}

extern void Prefetch_Handler(void)
{
}

extern void Abort_Handler(void)
{
}

extern void FIQ_Handler(void)
{
}






#ifdef _WINDOWS
// The following routines are only for simulator compatibility

extern void *fnGetHeapStart(void) { return 0; }

// memset implementation
//
extern void *uMemset(void *ptrTo, unsigned char ucValue, size_t Size)
{
    void *buffer = ptrTo;
    unsigned char *ptr = (unsigned char *)ptrTo;

    while (Size--) {
        *ptr++ = ucValue;
    }

    return buffer;
}

// Convert a MAC address to a string
//
signed char *fnMACStr(unsigned char *ptrMAC, signed char *cStr)
{
    signed char cDummyMac[] = "--:--:--:--:--:--";
    strcpy(cStr, cDummyMac);
    return (cStr + strlen(cDummyMac));
}

// Convert an IP address to a string
//
signed char *fnIPStr(unsigned char *ptrIP, signed char *cStr)
{
    signed char cDummyIP[] = "---.---.---.---";
    strcpy(cStr, cDummyIP);
    return (cStr + strlen(cDummyIP));
}

extern unsigned char *fnGetTxBufferAdd(int iBufNr) { return 0;}
__interrupt__ void sw_wdog_timeout(void){}
__interrupt__ void ephy_isr(void){}
__interrupt__ void fec_rx_frame_isr(void){}
__interrupt__ void QSPI_Interrupt(void){}
__interrupt__ void irq1_handler(void){}
__interrupt__ void irq2_handler(void){}
__interrupt__ void irq3_handler(void){}
__interrupt__ void irq4_handler(void){}
extern void DMA0_handler(void) {}
extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen) {return 0;}
TASK_LIMIT uTaskerStart(const UTASKTABLEINIT *ptATaskTable, const signed char *a_node_descriptions, const PHYSICAL_Q_LIMIT nr_physicalQueues) {return 0;}
void fnInitialiseHeap(const HEAP_NEEDS *ctOurHeap, void *start_heap ){}


// Hardware specific port reset states - if the hardware has pullups set '1' for each bit, else '0'
// (assumed all inputs have pullups)
#define QS_DEFAULT_INPUT           0xff
#define AN_DEFAULT_INPUT           0xff
#define LD_DEFAULT_INPUT           0xff
#define NQ_DEFAULT_INPUT           0xff
#define TA_DEFAULT_INPUT           0xff
#define TC_DEFAULT_INPUT           0xff
#define TD_DEFAULT_INPUT           0xff
#define UA_DEFAULT_INPUT           0xff
#define UB_DEFAULT_INPUT           0xff
#define UC_DEFAULT_INPUT           0xff
#define AS_DEFAULT_INPUT           0xff
#define GP_DEFAULT_INPUT           0xff
#define DD_DEFAULT_INPUT           0xff

INITHW void fnInitHW(void)
{
	extern unsigned long __VECTOR_RAM[];
    unsigned char ucPortPullups[] = {
        QS_DEFAULT_INPUT,                                                // set the port states out of reset in the project file app_hw_str91xf.h
        AN_DEFAULT_INPUT,
        LD_DEFAULT_INPUT,
        NQ_DEFAULT_INPUT,
        TA_DEFAULT_INPUT,
        TC_DEFAULT_INPUT,
        TD_DEFAULT_INPUT,
        UA_DEFAULT_INPUT,
        UB_DEFAULT_INPUT,
        UC_DEFAULT_INPUT,
        AS_DEFAULT_INPUT,
        GP_DEFAULT_INPUT,
        DD_DEFAULT_INPUT
    };

//    __VECTOR_RAM[PIT0_VECTOR] = (unsigned long)fnDummyTick;

    fnInitialiseDevice((void *)ucPortPullups);
    str91xf_boot_init();                                                 // Call initialisation routine for code checking only
}


extern void uTaskerBoot(void);
extern void uTaskerSchedule( void )
{
    static int iDone = 0;

    if (!iDone) {
        iDone = 1;
        uTaskerBoot();
    }
}
 #endif
#endif
