/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland
    
    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#define _DCD_TABLE                                                       // this module owns the DCD table (in application's app_hw_iMX.h or bare-minimum loader's config.h)
#if defined _WINDOWS
    #include "config.h"
    extern void fnUpdateOperatingDetails(void);
    #define INITHW  extern
    extern void fec_txf_isr(void);
    extern void fnSimulateDMA(int channel, unsigned char ucTriggerSource);
    static void fnCheckDCD(void);
    #define __disable_interrupt()
    #define __enable_interrupt()
    #define START_CODE 0
    #if defined MMDVSQ_AVAILABLE
        #include <math.h>
    #endif
#else
    #define OPSYS_CONFIG                                                 // this module owns the operating system configuration (application's TaskConfig.h)
    #define INITHW  static
    #include "config.h"
    #define  fnSimulateDMA(x, y)
    #if defined _COMPILE_KEIL
        extern void __main(void);
        #define START_CODE keil_entry
        #define uMask_Interrupt(ucMaskLevel)  __set_BASEPRI(ucMaskLevel)
    #elif defined _COMPILE_IAR
        extern void __iar_program_start(void);                           // IAR library initialisation routine
        #define START_CODE iar_ghs_entry
    #elif defined _COMPILE_GHS
        extern void _start_T(void);                                      // GHS library initialisation routine
        #define START_CODE iar_ghs_entry
    #else
        #define START_CODE main
    #endif
#endif

#if defined _iMX

#if defined _GNU
    #define NEVER_INLINE_SUBROUTINE          __attribute__(( noinline ))
    #define SUBROUTINE_OPTIMISATION_LEVEL1   __attribute__(( optimize(1)))
#else
    #define NEVER_INLINE_SUBROUTINE
    #define SUBROUTINE_OPTIMISATION_LEVEL1
#endif


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#define __interrupt

#if defined _APPLICATION_VALIDATION
    #define _RESET_VECTOR  RESET_VECTOR_VALIDATION
#elif defined INTERRUPT_VECTORS_IN_FLASH
    #define _RESET_VECTOR   VECTOR_TABLE
    const _RESET_VECTOR __vector_table;
#else
    #define _RESET_VECTOR  RESET_VECTOR
#endif

#define UART0_TX_CLK_REQUIRED 0x00000001
#define UART1_TX_CLK_REQUIRED 0x00000002
#define UART2_TX_CLK_REQUIRED 0x00000004
#define UART3_TX_CLK_REQUIRED 0x00000008
#define UART4_TX_CLK_REQUIRED 0x00000010
#define UART5_TX_CLK_REQUIRED 0x00000020
#define UART6_TX_CLK_REQUIRED 0x00000040
#define UART7_TX_CLK_REQUIRED 0x00000080
#define UART0_RX_CLK_REQUIRED 0x00000100
#define UART1_RX_CLK_REQUIRED 0x00000200
#define UART2_RX_CLK_REQUIRED 0x00000400
#define UART3_RX_CLK_REQUIRED 0x00000800
#define UART4_RX_CLK_REQUIRED 0x00001000
#define UART5_RX_CLK_REQUIRED 0x00002000
#define UART6_RX_CLK_REQUIRED 0x00004000
#define UART7_RX_CLK_REQUIRED 0x00008000


/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

/* =================================================================== */
/*                global function prototype declarations               */
/* =================================================================== */

#if defined RUN_IN_FREE_RTOS
    extern void xPortPendSVHandler(void);                                // PendSV interrupt handler
    extern void vPortSVCHandler(void);                                   // SCV interrupt handler
    extern void xPortSysTickHandler(void);                               // FreeRTOS tick handler callback
#endif

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static void _LowLevelInit(void);

#if defined SUPPORT_LOW_POWER
    static int fnPresentLP_mode(void);
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if !defined ONLY_INTERNAL_FLASH_STORAGE
    static const STORAGE_AREA_ENTRY default_flash = {                    // an area encompasing both ITC and DTC ram is default
        0,                                                               // end of list
    #if defined iMX_BOOTLOADER
        #if defined MIMXRT1024 || defined MIMXRT1064
        (unsigned char *)(FLEXSPI2_FLASH_BASE),                          // start address of internal QSPI flash
        (unsigned char *)(FLEXSPI2_FLASH_BASE + (FLEXSPI2_FLASH_SIZE - 1)), // end of internal QSPI flash
        _STORAGE_INTERNAL_FLASH,                                         // type
        #else
        (unsigned char *)(FLEXSPI_FLASH_BASE),                           // start address of QSPI flash
        (unsigned char *)(FLEXSPI_FLASH_BASE + (FLEXSPI_FLASH_SIZE - 1)),// end of QSPI flash
        _STORAGE_SPI_FLASH,                                              // type
        #endif
    #else
        (unsigned char *)(RAM_START_ADDRESS_ITC),                        // start address of ram area
        (unsigned char *)(RAM_START_ADDRESS_DTC + (SIZE_OF_FLEX_RAM - 1)), // end of ram area
        _STORAGE_PROGRAM,                                                // type
    #endif
        0                                                                // not multiple devices
    };

    STORAGE_AREA_ENTRY *UserStorageListPtr = (STORAGE_AREA_ENTRY *)&default_flash; // default entry
#endif

#if !defined SPI_FLASH_ST && !defined SPI_FLASH_SST25 && !defined SPI_FLASH_W25Q && !defined SPI_FLASH_S25FL1_K && !defined SPI_FLASH_MX25L && !defined SPI_FLASH_IS25 && !defined SPI_FLASH_S26KL && !defined SPI_FLASH_ATXP && !defined SPI_FLASH_AT25SF
    #define SPI_FLASH_ATMEL                                              // default if not otherwise defined
#endif
#define _SPI_DEFINES
    #include "../SPI_Memory/spi_flash_AT25SF.h"
    #include "../SPI_Memory/spi_flash_IS25LP.h"
    #include "../SPI_Memory/spi_flash_S26KL.h"
    #include "../SPI_Memory/spi_flash_ATXP.h"
    #include "../SPI_Memory/spi_flash_w25q.h"
#undef _SPI_DEFINES


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static int iInterruptLevel = 0;                                          // present level of disable nesting

#if defined SUPPORT_LOW_POWER
    static unsigned long ulPeripheralNeedsClock = 0;                     // stop mode is blocked if a peripheral is in use that needs a clock that will be stopped
#endif
#if defined RANDOM_NUMBER_GENERATOR && !defined RND_HW_SUPPORT
    unsigned short *ptrSeed;
#endif
#if !defined NO_FLASH_SUPPORT && (defined SPI_SW_UPLOAD || defined SPI_FLASH_FAT || ((defined SPI_FILE_SYSTEM && defined FLASH_FILE_SYSTEM) && !defined iMX_BOOTLOADER))
    #if !defined SPI_FLASH_DEVICE_COUNT
        #define SPI_FLASH_DEVICE_COUNT 1
    #endif
    static unsigned long SPI_FLASH_Danger[SPI_FLASH_DEVICE_COUNT] = {0}; // signal that the FLASH status should be checked before using since there is a danger that it is still busy
    static unsigned char ucSPI_FLASH_Type[SPI_FLASH_DEVICE_COUNT] = { NO_SPI_FLASH_AVAILABLE }; // list of attached FLASH devices

    #if defined SPI_FLASH_MULTIPLE_CHIPS
        unsigned long ulChipSelect[SPI_FLASH_DEVICE_COUNT] = {
            CS0_LINE,
            CS1_LINE                                                     // at least 2 expected when multiple devices are defined
        #if defined CS2_LINE
            ,CS2_LINE
            #if defined CS3_LINE
            ,CS3_LINE
            #endif
        #endif
        };
        #define EXTENDED_CS , &iChipSelect
        #define _EXTENDED_CS  ptrAccessDetails->ucDeviceNumber,
    #else
        #define EXTENDED_CS
        #define _EXTENDED_CS
    #endif
#endif
#if (defined SPI_EEPROM_FILE_SYSTEM && defined FLASH_FILE_SYSTEM)
    #if !defined SPI_EEPROM_DEVICE_COUNT
        #define SPI_EEPROM_DEVICE_COUNT 1
    #endif
    static unsigned long SPI_EEPROM_Danger[SPI_EEPROM_DEVICE_COUNT] = {0}; // signal that the FLASH status should be checked before using since there is a danger that it is still busy
#endif


/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

#if defined _WINDOWS                              
    extern unsigned char vector_ram[sizeof(VECTOR_TABLE)];               // vector table in simulated RAM (long word aligned)
    #if defined SERIAL_INTERFACE
        extern unsigned char ucTxLast[NUMBER_SERIAL] = {0};
    #endif
    unsigned char FlexSPI_CS = 0;
#endif


/* =================================================================== */
/*                      local function definitions                     */
/* =================================================================== */

#if !defined _WINDOWS
    static void fnInitHW(void);
#endif


/* =================================================================== */
/*                      global function definitions                    */
/* =================================================================== */

#if !defined _WINDOWS
    extern __interrupt void _start(void);                                // reset vector location
#endif

#if (defined iMX_BOOT || defined iMX_BOOTLOADER) && (defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK)
/* =================================================================== */
/*                        FlexSPI FLASH driver                         */
/* =================================================================== */
    #if !defined iMX_BOOTLOADER
        #include "iMX_FLASH.h"                                           // include FLASH driver code
    #endif

    #define _SPI_FLASH_INTERFACE                                         // insert manufacturer dependent SPI Flash driver code
        #include "../SPI_Memory/spi_flash_AT25SF.h"
        #include "../SPI_Memory/spi_flash_IS25LP.h"
        #include "../SPI_Memory/spi_flash_S26KL.h"
        #include "../SPI_Memory/spi_flash_ATXP.h"
        #include "../SPI_Memory/spi_flash_w25q.h"
    #undef _SPI_FLASH_INTERFACE
#endif


/* =================================================================== */
/*                            STARTUP CODE                             */
/* =================================================================== */

#if defined _WINDOWS
    #define _DATA_MEMORY_BARRIER()
    #define _SYNCHRONISATION_BARRIER()
    #define _SYNCHRONISATION_DATA_BARRIER()
#elif defined _COMPILE_IAR
    #define _DATA_MEMORY_BARRIER()           __DMB()
    #define _SYNCHRONISATION_BARRIER()       __DSB(); \
                                             __ISB()
    #define _SYNCHRONISATION_DATA_BARRIER()  __DSB()
#elif defined _COMPILE_KEIL
    #define _DATA_MEMORY_BARRIER()           __asm("dmb")
    #define _SYNCHRONISATION_BARRIER()       __asm("dsb"); \
                                             __asm("isb")
    #define _SYNCHRONISATION_DATA_BARRIER()  __asm("dsb")
#else
    #define _DATA_MEMORY_BARRIER()           asm volatile ("dmb 0xf":::"memory")
    #define _SYNCHRONISATION_BARRIER()       asm volatile ("dsb 0xf":::"memory"); \
                                             asm volatile ("isb 0xf":::"memory")
    #define _SYNCHRONISATION_DATA_BARRIER()  asm volatile ("dsb 0xf":::"memory")
#endif

/* =================================================================== */
/*                        DCD table checking code                      */
/* =================================================================== */

#if defined _WINDOWS
const IMAGE_VECTOR_TABLE __image_vector_table;

// This routine does a check of the DCD content to warn of any fields that may have not been correctly set
//
static void fnCheckDCD(void)
{
    const unsigned char *ptrDCD = (const unsigned char *)__image_vector_table.dcd; // pointer to the DCD entry
    unsigned short usDCD_length;
    unsigned short ucCommandLength;
    unsigned long ulCommand;
    unsigned long ulAddress;
    unsigned long ulValue;
    if (ptrDCD == 0) {
        return;                                                          // no DCD specified
    }
    if (*ptrDCD == 0) {                                                  // no entry
        return;
    }
    if (*ptrDCD++ != DCD_TAG) {
        _EXCEPTION("DCD tag is not valid!!");
    }
    usDCD_length = (*ptrDCD++ << 8);                                     // extract the complete table length
    usDCD_length |= (*ptrDCD++);
    if (*ptrDCD++ != DCD_VERSION) {
        _EXCEPTION("DCD version is not valid!!");
    }
    while (usDCD_length >= 4) {                                          // scan the content to check commands and length entries
        ulCommand = (*ptrDCD++ << 24);
        ucCommandLength = (*ptrDCD++ << 8);
        ucCommandLength |= (*ptrDCD++);
        ulCommand |= (*ptrDCD++);
        usDCD_length -= 4;
        ucCommandLength -= 4;
        switch (ulCommand) {
        case DCD_WAIT_SET_LONG_WORD:                                     // a loop to wait for a flag is expected
            if (ucCommandLength != 8) {
                _EXCEPTION("DCD command length invalid!!!");
            }
            ulAddress = (*ptrDCD++ << 24);
            ulAddress |= (*ptrDCD++ << 16);
            ulAddress |= (*ptrDCD++ << 8);
            ulAddress |= (*ptrDCD++);
            if (ulAddress != ADDRESS_MARKER) {
                _EXCEPTION("DCD command found!!!");
            }
            ulValue = (*ptrDCD++ << 24);
            ulValue |= (*ptrDCD++ << 16);
            ulValue |= (*ptrDCD++ << 8);
            ulValue |= (*ptrDCD++);
            usDCD_length -= 8;
            break;
        case DCD_LONG_WORD_WRITE_GROUP:
            while (ucCommandLength != 0) {
                if (ucCommandLength < 8) {
                    _EXCEPTION("DCD command length invalid!!!");
                }
                ucCommandLength -= 8;
                ulAddress = (*ptrDCD++ << 24);
                ulAddress |= (*ptrDCD++ << 16);
                ulAddress |= (*ptrDCD++ << 8);
                ulAddress |= (*ptrDCD++);
                if (ulAddress != ADDRESS_MARKER) {
                    _EXCEPTION("DCD command found!!!");
                }
                ulValue = (*ptrDCD++ << 24);
                ulValue |= (*ptrDCD++ << 16);
                ulValue |= (*ptrDCD++ << 8);
                ulValue |= (*ptrDCD++);
                usDCD_length -= 8;
            }
            break;
        default:
            _EXCEPTION("Unknown DCD command found!!!");
            break;
        }
        usDCD_length -= 4;
    }
    if (usDCD_length != 0) {
        _EXCEPTION("DCD table length in question!!!");
    }
}
#endif


static void fnConfigWdogs(void)
{
    // The watchdog power down counter is enabled out of reset and fires after 16s if not stopped here
    //
    WDOG1_WMCR = 0;                                                      // stop and power down watchdog
    WDOG2_WMCR = 0;

    // We presently use WDOG3 only
    //
    WDOG1_WCR = (WDOG_WCR_WDA | WDOG_WCR_SRS);                           // disable watchdog timer 1
    WDOG2_WCR = (WDOG_WCR_WDA | WDOG_WCR_SRS);                           // disable watchdog timer 2

    if (WATCHDOG_DISABLE() == 0) {                                       // if the watchdog disable input is not active
        ACTIVATE_WATCHDOG();                                             // allow user configuration of internal watchdog timer
    }
    else {                                                               // disable the watchdog
        UNLOCK_WDOG3();                                                  // open a window to write to watchdog 3
        WDOG3_CS = WDOG_CS_UPDATE;                                       // disable watchdog but allow updates
        WDOG3_TOVAL = 0xffff;
        WDOG3_WIN = 0;
    }
}


extern unsigned char *fnGetPersistentMemory(void)
{
    unsigned char ucStack;                                               // put a variable on to stack (it is assumed that the present stack usage is not greater than 32k)
    unsigned char *ptrStack = &ucStack;
    ptrStack = (unsigned char *)((unsigned long)(ptrStack + FLEX_RAM_GRANULARITY) & ~(FLEX_RAM_GRANULARITY - 1)); // round up to the top of the RAM bank the stack point is in
#if defined _WINDOWS
    return &uninitialisedRAM[2];
#else
    return (ptrStack - 2);                                               // final unsigned short in SRAM
#endif
}


extern void fnGetFlexRAM_banks(unsigned char *ptr_ucInstructionBanks, unsigned char *ptr_ucDataBanks)
{
    unsigned char *ptrTopOfMemory = fnGetPersistentMemory();
    int iDataRamBanks = 1;
#if defined _WINDOWS
    ptrTopOfMemory = (unsigned char *)(SIZE_OF_FLEX_RAM / 2);
#else
    ptrTopOfMemory -= RAM_START_ADDRESS_DTC;                             // the size of data memory
#endif
    while ((unsigned long)ptrTopOfMemory > FLEX_RAM_GRANULARITY) {
        ptrTopOfMemory -= FLEX_RAM_GRANULARITY;
        iDataRamBanks++;
    }
    *ptr_ucDataBanks = iDataRamBanks;
    *ptr_ucInstructionBanks = (FLEX_RAM_BANKS - iDataRamBanks);
}

#if !defined _WINDOWS
    extern void __segment_init(void);
    #if defined  _COMPILE_IAR
        #pragma segment=".data"
        #pragma segment=".bss"
        #if !defined iMX_BOOTLOADER
        static unsigned char *ptrTopOfUsedMemory = 0;
        #define HEAP_START_ADDRESS ptrTopOfUsedMemory                    // IAR compiler - last location of static variables
        #endif
        #define  __sleep_mode() __WFI()                                  // IAR intrinsic
    #elif defined _COMPILE_KEIL
        #define HEAP_START_ADDRESS    _keil_ram_size(0)
        #define __disable_interrupt() __disable_irq()                    // KEIL intrinsics
        #define __enable_interrupt()  __enable_irq()
        #define __sleep_mode()        __wfi()
    #else                                                                // disable interrupt in assembler code
        extern unsigned char __heap_end__;
        #define HEAP_START_ADDRESS &__heap_end__                         // GNU last location of static variables
        #if defined ROWLEY || defined _KDS
            #define asm(x) __asm__(x)
        #elif defined _COMPILE_COSMIC
            #define asm(x) _asm(x)
        #endif
        #if defined _COMPILE_GHS
            #define __disable_interrupt() asm("cpsid i")                 // __DI() intrinsics are not used because they are asm("cpsid if") which doesn't allow correct low power mode operation
            #define __enable_interrupt()  asm("cpsie i")                 // __EI()
            #define __sleep_mode()        asm("wfi")
        #else
            #define __disable_interrupt() asm("cpsid   i")
            #define __enable_interrupt()  asm("cpsie   i")
            #define __sleep_mode()        asm("wfi")
        #endif
    #endif

    #if defined _COMPILE_KEIL && !defined APPLICATION_WITHOUT_OS
typedef struct stREGION_TABLE
{
    unsigned char *ptrConst;                                             // const variables belonging to block
    unsigned char *ptrDestination;                                       // destination in RAM
    unsigned long  ulLength;                                             // length of the block in SRAM
    unsigned long  ulUnknown;                                            // entry unknown
} REGION_TABLE;

// Calculate the end of used SRAM from the Keil linker information and optionally initialise variables
//
static unsigned char *_keil_ram_size(int iInit)
{
    extern REGION_TABLE Region$$Table$$Base;                            // table location supplied by linker
    extern REGION_TABLE Region$$Table$$Limit;                           // end of table list
    REGION_TABLE *ptrEntries = &Region$$Table$$Base;                    // first block
    unsigned char *ptrRam = ptrEntries->ptrDestination;                 // RAM address
    do {
        if (iInit != 0) {
            if (ptrEntries->ulUnknown == 0x60) {                        // not valid for uVision4
                uMemset(ptrRam, 0, ptrEntries->ulLength);               // zero data
            }
            else {
                uMemcpy(ptrRam, ptrEntries->ptrConst, ptrEntries->ulLength); // intialise data (uVision4 uses compressed data in Flash and so can't be used like this
            }
        }
        ptrRam += ptrEntries->ulLength;                                 // add length
        ptrEntries++;                                                   // move to next block
    } while (ptrEntries != &Region$$Table$$Limit);
    return ptrRam;
}
    #endif

/* =================================================================== */
/*                                main()                               */
/* =================================================================== */

#if !defined APPLICATION_WITHOUT_OS
    #if !defined RUN_IN_FREE_RTOS && !defined _WINDOWS
        static void fn_uTasker_main(void);
    #elif defined RUN_IN_FREE_RTOS && !defined _WINDOWS
        extern void fnFreeRTOS_main(void);
    #endif
#endif

// Main entry for the target code
//
extern int main(void)
{
#if defined _COMPILE_IAR && !defined iMX_BOOTLOADER
    if (__sfe(".bss") > __sfe(".data")) {                                // set last used SRAM address
        ptrTopOfUsedMemory = __sfe(".bss");
    }
    else {
        ptrTopOfUsedMemory = __sfe(".data");
    }
#elif !defined _COMPILE_IAR && defined iMX_BOOTLOADER
    do {
        IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // immediately after a reset ensure that the FlexRAM layout is taken from the eFuse setting (default configuration)
    } while (IOMUXC_GPR_GPR16 != (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN));
#endif
    fnInitHW();                                                          // perform hardware initialisation (note that we do not have heap yet)
#if defined RANDOM_NUMBER_GENERATOR && !defined RND_HW_SUPPORT
    ptrSeed = (unsigned short *)RANDOM_SEED_LOCATION;
#endif
#if !defined APPLICATION_WITHOUT_OS
    fnInitialiseHeap(ctOurHeap, HEAP_START_ADDRESS);                     // initialise heap
    #if defined RUN_IN_FREE_RTOS
    fnFreeRTOS_main();                                                   // never return in normal situations
    FOREVER_LOOP() {
        // This only happens when there was a failure to initialise and start FreeRTOS (usually not enough heap)
        //
        _EXCEPTION("FreeRTOS failed to initialise");
    }
    #else
    fn_uTasker_main();                                                   // never return
    #endif
#endif
    return 0;                                                            // we never return....
}
#endif                                                                   // end if not _WINDOWS

#if !defined APPLICATION_WITHOUT_OS
    #if defined RUN_IN_FREE_RTOS
    extern void fn_uTasker_main(void *pars)
    #else
    static void fn_uTasker_main(void)
    #endif
{
    #if !defined _WINDOWS && defined MULTISTART
    MULTISTART_TABLE *prtInfo;
    unsigned char *pucHeapStart;
    prtInfo = ptMultiStartTable;                                         // if the user has already set to alternative start configuration
    if (prtInfo == 0) {                                                  // no special start required
    _abort_multi:
        fnInitialiseHeap(ctOurHeap, HEAP_START_ADDRESS);                 // initialise heap
        uTaskerStart((UTASKTABLEINIT *)ctTaskTable, ctNodes, PHYSICAL_QUEUES); // start the operating system (and TICK interrupt)
        while ((prtInfo = (MULTISTART_TABLE *)uTaskerSchedule()) == 0) {}// schedule uTasker
    }

    do {
        pucHeapStart = HEAP_START_ADDRESS;
        if (prtInfo->new_hw_init) {                                      // info to next task configuration available
            pucHeapStart = prtInfo->new_hw_init(JumpTable);              // get heap details from next configuration
            if (0 == pucHeapStart) {
                goto _abort_multi;                                       // this can happen if the jump table version doesn't match - prefer to stay in boot mode than start an application which will crash
            }
        }

        fnInitialiseHeap(prtInfo->ptHeapNeed, pucHeapStart);             // initialise the new heap
                                                                         // start the operating system with next configuration
        uTaskerStart((UTASKTABLEINIT *)prtInfo->ptTaskTable, prtInfo->ptNodesTable, PHYSICAL_QUEUES);
        while ((prtInfo = (MULTISTART_TABLE *)uTaskerSchedule()) == 0) {}// schedule uTasker

    } FOREVER_LOOP();
    #elif !defined _WINDOWS || defined RUN_IN_FREE_RTOS
    uTaskerStart((UTASKTABLEINIT *)ctTaskTable, ctNodes, PHYSICAL_QUEUES); // start the operating system (and TICK interrupt)
    FOREVER_LOOP() {
        uTaskerSchedule();                                               // schedule uTasker
    }
    #endif
}
#endif


#if (defined iMX_SERIAL_LOADER || defined iMX_BOOTLOADER)
#if defined iMX_RT1011
    #if defined _COMPILE_IAR
#pragma inline=never
    #elif defined _COMPILE_KEIL
#pragma no_inline
    #elif defined _GNU
static void fnRamCode(volatile unsigned long *ulOTFAD_CR, unsigned long *ptrScrambler) NEVER_INLINE_SUBROUTINE SUBROUTINE_OPTIMISATION_LEVEL1;
    #endif
static void fnRamCode(volatile unsigned long *ulOTFAD_CR, unsigned long *ptrScrambler)
{
    unsigned long *ptrIn = (unsigned long *)FLEXSPI_FLASH_BASE;
    unsigned long ulLength = ((0x2000 - iMX_LOADER_KEY_STORAGE_AREA_SIZE)/sizeof(unsigned long)); // the number of bytes from the start of the QSPI flash and the original AES256 key area
    int i;
    *ulOTFAD_CR = OTFAD_CR_GE;                                           // enable decryption (all reads from the defined area in QSPI flash are on-the-fly decrypted)
    #if !defined iMX_BOOTLOADER
    _SYNCHRONISATION_BARRIER();                                          // ensure that decryption is operating before continuing
    #endif
    while (ulLength != 0) {
        for (i = 0; i < (iMX_LOADER_KEY_STORAGE_AREA_SIZE / sizeof(unsigned long)); ) {
            ptrScrambler[i++] ^= *(unsigned long *)fnGetFlashAdd((unsigned char *)ptrIn++); // calculate the scrambler/de-scrambler array from the code stored in QSPI flash so that it will change if the code were ever modified
            if (--ulLength == 0) {
                break;
            }
        }
    }
    ptrIn += (iMX_LOADER_KEY_STORAGE_AREA_SIZE / sizeof(unsigned long));   // jump over original key area, which is not used in the calculation
    ulLength = ((FALL_BACK_SERIAL_LOADER_OFFSET - 0x2000 - iMX_LOADER_KEY_STORAGE_AREA_SIZE) / sizeof(unsigned long)); // the number of bytes up to the secured AES256 key area
    while (ulLength != 0) {
        for (; i < (iMX_LOADER_KEY_STORAGE_AREA_SIZE / sizeof(unsigned long)); ) { // note that i initially retains the value after the first area calculation
            ptrScrambler[i++] ^= *(unsigned long *)fnGetFlashAdd((unsigned char *)ptrIn++); // calculate the scrambler/descrambler array from the code stored in QSPI flash so that it will change if the code were ever modified
            if (--ulLength == 0) {
                break;
            }
        }
        i = 0;
    }
    *ulOTFAD_CR = 0;                                                     // disable further OTFAD decryption from QSPI flash
    _SYNCHRONISATION_BARRIER();                                          // ensure that decryption is disabled before continuing
}
#else
    #if defined _COMPILE_IAR
#pragma inline=never
    #elif defined _GNU
static void fnRamCode(volatile unsigned long *ulBEE_CTRL, volatile unsigned long *ulBEE_STATUS, volatile unsigned long *ulGPR18, volatile unsigned long *ulGPR11, unsigned long *ptrScrambler) NEVER_INLINE_SUBROUTINE SUBROUTINE_OPTIMISATION_LEVEL1;
    #endif
static void fnRamCode(volatile unsigned long *ulBEE_CTRL, volatile unsigned long *ulBEE_STATUS, volatile unsigned long *ulGPR18, volatile unsigned long *ulGPR11, unsigned long *ptrScrambler)
{
    unsigned long *ptrIn = (unsigned long *)FLEXSPI_FLASH_BASE;
    unsigned long ulLength = ((0x2000 - iMX_LOADER_KEY_STORAGE_AREA_SIZE)/sizeof(unsigned long)); // the number of bytes from the start of the QSPI flash and the original AES256 key area
    int i;
    *ulGPR18 = (FLEXSPI_FLASH_BASE);                                     // region 0 starting at the beginning of SPI flash
    *ulBEE_CTRL |= BEE_CTRL_BEE_ENABLE;                                  // enable decryption (all AHB reads from the defined area in QSPI flash are now on-the-fly decrypted)
    _WAIT_REGISTER_FALSE(*ulBEE_STATUS, BEE_STATUS_IDLE);                // wait until BEE is idle
    while (ulLength != 0) {                                              // for the first block length
        for (i = 0; i < (iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long)); ) {
            ptrScrambler[i++] ^= *(unsigned long *)fnGetFlashAdd((unsigned char *)ptrIn++); // calculate the scrambler/descrambler array from the code stored in QSPI flash so that it will change if the code were ever modified
            if (--ulLength == 0) {
                break;
            }
        }
    }
    ptrIn += (iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long));   // jump over original key area, which is not used in the calculation
    ulLength = ((FALL_BACK_SERIAL_LOADER_OFFSET - 0x2000 - iMX_LOADER_KEY_STORAGE_AREA_SIZE)/sizeof(unsigned long)); // the number of bytes up to the secured AES256 key area
    while (ulLength != 0) {
        for ( ; i < (iMX_LOADER_KEY_STORAGE_AREA_SIZE / sizeof(unsigned long)); ) { // note that i initially retains the value after the first area calculation
            ptrScrambler[i++] ^= *(unsigned long *)fnGetFlashAdd((unsigned char *)ptrIn++); // calculate the scrambler/descrambler array from the code stored in QSPI flash so that it will change if the code were ever modified
            if (--ulLength == 0) {
                break;
            }
        }
        i = 0;
    }
    *ulGPR11 = (0);                                                      // disable further BEE decryption from QSPI flash
    _SYNCHRONISATION_BARRIER();                                          // ensure that the descrption has been disabled before continuing
}
#endif

extern void fnGetDecryptionKey(unsigned long *ptrScrambler)
{
#if !defined ENABLE_PRODUCT_SECURITY
    static const unsigned long aesNonce[16/sizeof(unsigned long)] = {0,          0x8899aabb, 0x44556677, 0x00112233}; // only lower two long word are used by i.MX RT 1011's OTFAD and BEE should have first long word set to 0
    static const unsigned long aesKey[16/sizeof(unsigned long)]   = {0x0d0e0f10, 0x090a0b0c, 0x05060708, 0x01020304}; // this is used only to taken from secret eFUSE value when secured
#endif

#if !defined iMX_RT1011 && !defined ENABLE_PRODUCT_SECURITY
    unsigned long *ptrReg;
    unsigned long *ptrIn;
#endif
#if defined iMX_BOOTLOADER || (!defined iMX_RT1011 && !defined ENABLE_PRODUCT_SECURITY)
    int i = 0;
#endif
#if defined iMX_BOOTLOADER                                               // the boot loader runs from QSPI flash and so needs to use a routine relocated in RAM
    #define PROG_WORD_SIZE 512                                           // adequate space for the small program that runs in RAM
    #if defined iMX_RT1011
    void (*fnRAM_code)(volatile unsigned long *ulOTFAD_CR, unsigned long *ptrScrambler) = 0;
    #else
    void (*fnRAM_code)(volatile unsigned long *ulBEE_CTRL, volatile unsigned long *ulBEE_STATUS, volatile unsigned long *ulGPR18, volatile unsigned long *ulGPR11, unsigned long *ptrScrambler) = 0;
    #endif
    unsigned char *ptrThumb2 = (unsigned char *)fnRamCode;
    static unsigned short usProgSpace[PROG_WORD_SIZE] = {0};             // make space for the routine in static memory (this will have an even boundary)

    ptrThumb2 =  (unsigned char *)(((CAST_POINTER_ARITHMETIC)ptrThumb2) & ~0x1); // thumb 2 address
    while (i < PROG_WORD_SIZE) {                                         // copy program to SRAM
        usProgSpace[i++] = *(unsigned short *)ptrThumb2;
        ptrThumb2 += sizeof (unsigned short);
    }
    ptrThumb2 = (unsigned char *)usProgSpace;
    ptrThumb2++;                                                         // create a thumb 2 call
    #if defined iMX_RT1011
    fnRAM_code = (void(*)(volatile unsigned long *ulOTFAD_CR, unsigned long *ptrScrambler))(ptrThumb2);
    #else
    fnRAM_code = (void(*)(volatile unsigned long *ulBEE_CTRL, volatile unsigned long *ulBEE_STATUS, volatile unsigned long *ulGPR18, volatile unsigned long *ulGPR11, unsigned long *ptrScrambler))(ptrThumb2);
    #endif
#endif
#if defined iMX_RT1011
    OTFAD_CR = 0;                                                        // decryption initially disabled
    OTFAD_CTX0_RGD_W0 = (FLEXSPI_FLASH_BASE & OTFAD_RGD_W0_SRTADDR_MASK);// set start address of region (1k aligned)
    #if defined ENABLE_PRODUCT_SECURITY
    POWER_UP_ATOMIC(2, OCOTP_CTRL_CLOCK);
    HW_OCOTP_CFG5 |= uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2;                // take the AES key selection for BEE_KEY0 from SW-GP2 and not from the registers
    OTFAD_CTX0_CTR0 = HW_OCOTP_CFG0;                                     // use this devices's unique ID as nonce
    OTFAD_CTX0_CTR1 = HW_OCOTP_CFG1;
    #else
    HW_OCOTP_CFG5 &= ~(uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2);             // take the AES key selection for BEE_KEY0 from registers and not from SW-GP2
    OTFAD_CTX0_KEY0 = aesKey[0];                                         // set encryption key
    OTFAD_CTX0_KEY1 = aesKey[1];
    OTFAD_CTX0_KEY2 = aesKey[2];
    OTFAD_CTX0_KEY3 = aesKey[3];
    OTFAD_CTX0_CTR0 = aesNonce[0];                                       // set encryption counters
    OTFAD_CTX0_CTR1 = aesNonce[1];
    #endif
    OTFAD_CTX0_RGD_W1 = (((FLEXSPI_FLASH_BASE + FALL_BACK_SERIAL_LOADER_OFFSET) & OTFAD_RGD_W1_ENDADDR_MASK) | OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD);  // set end address of region and validate its operation
    #if !defined iMX_BOOTLOADER || defined _WINDOWS
    fnRamCode(OTFAD_CR_ADD, ptrScrambler);                               // get the decoding key
    #else
    fnRAM_code(OTFAD_CR_ADD, ptrScrambler);                              // get the decoding key - this runs in RAM so that it can be used by the "Bare-Minimum" loader operating in QSPI flash
    #endif
    OTFAD_CTX0_RGD_W1 = 0;                                               // ensure that the decyption range is invalidated so that it doesn't decrypt if on-the-fly XiP decryption is later enabled
#else
    POWER_UP_ATOMIC(4, BEE_CLOCK);                                       // power up the bus encryption engine
    IOMUXC_GPR_GPR18 = (FLEXSPI_FLASH_BASE + FALL_BACK_SERIAL_LOADER_OFFSET); // region 0 starting after loader
    IOMUXC_GPR_GPR19 = (FLEXSPI_FLASH_BASE + FALL_BACK_SERIAL_LOADER_OFFSET); // region 0 ending after loader (not yet a valid range so that the "BM loader can still operate)
    IOMUXC_GPR_GPR11 = (IOMUXC_GPR_GPR11_BEE_DE_RX_EN_0);                // enable decryption (don't lock changes)
    BEE_CTRL = (BEE_CTRL_CTRL_CLK_EN | BEE_CTRL_CTRL_SFTRST_N);
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    BEE_CTRL = (BEE_CTRL_SECURITY_LEVEL_R0 | BEE_CTRL_CTRL_AES_MODE_R0 | BEE_CTRL_SECURITY_LEVEL_R1 | BEE_CTRL_CTRL_AES_MODE_R1 | BEE_CTRL_CTRL_CLK_EN | BEE_CTRL_CTRL_SFTRST_N);
    BEE_ADDR_OFFSET0 = 0;
    BEE_ADDR_OFFSET1 = 0;
    BEE_REGION1_TOP = 0;
    BEE_REGION1_BOT = 0;
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    #if defined ENABLE_PRODUCT_SECURITY
    HW_OCOTP_CFG5 |= uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2;                // take the AES key selection for BEE_KEY0 from SW-GP2 and not from the registers
    #else
    HW_OCOTP_CFG5 &= ~(uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2);             // take the AES key selection for BEE_KEY0 from registers and not from SW-GP2
    ptrReg = BEE_AES_KEY0_W0_ADD;
    ptrIn = (unsigned long *)aesKey;
    for (i = 0; i < 4; i++) {
        *ptrReg++ = *ptrIn++;                                            // load the AES128 key to the bus encryption engine (4 aligned long words)
    }
    #endif
    BEE_CTRL |= BEE_CTRL_BEE_ENABLE;
    BEE_CTRL |= BEE_CTRL_KEY_VALID;
    _WAIT_REGISTER_FALSE(BEE_CTRL, BEE_CTRL_KEY_VALID);
    BEE_CTRL &= ~(unsigned long)BEE_CTRL_BEE_ENABLE;
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    #if defined ENABLE_PRODUCT_SECURITY
    POWER_UP_ATOMIC(2, OCOTP_CTRL_CLOCK);
    BEE_CTR_NONCE0_W0 = 0;
    BEE_CTR_NONCE0_W1 = HW_OCOTP_CFG0;                                   // use this devices's unique ID and silicon revision as nonce
    BEE_CTR_NONCE0_W2 = HW_OCOTP_CFG1;
    BEE_CTR_NONCE0_W3 = HW_OCOTP_CFG3;
    #else
    ptrReg = BEE_CTR_NONCE0_ADD;
    ptrIn = (unsigned long *)aesNonce;
    for (i = 0; i < 4; i++) {
        *ptrReg++ = *ptrIn++;                                            // load the the initial vector to the bus encryption engine (4 aligned long words)
    }
    #endif
    #if !defined iMX_BOOTLOADER || defined _WINDOWS
    fnRamCode(BEE_CTRL_ADD, BEE_STATUS_ADD, IOMUXC_GPR_GPR18_ADD, IOMUXC_GPR_GPR11_ADD, ptrScrambler); // get the decoding key
    #else
    fnRAM_code(BEE_CTRL_ADD, BEE_STATUS_ADD, IOMUXC_GPR_GPR18_ADD, IOMUXC_GPR_GPR11_ADD, ptrScrambler); // get the decoding key - this runs in RAM so that it can be used by the "Bare-Minimum" loader operating in QSPI flash
    #endif

    BEE_CTRL &= ~(unsigned long)(BEE_CTRL_BEE_ENABLE | BEE_CTRL_CTRL_SFTRST_N | BEE_CTRL_CTRL_CLK_EN | BEE_CTRL_KEY_VALID); // de-initialise the bus encryption engine after use
    POWER_DOWN_ATOMIC(4, BEE_CLOCK);
#endif
}

extern void fnDecryptKey(unsigned long *ptrKeys, unsigned long *ptrScrambler)
{
    unsigned long ulLength = (iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long));
    while (ulLength-- != 0) {
        *ptrKeys++ ^= *ptrScrambler++;
    }
}
#endif

#if defined iMX_BOOTLOADER
    #include "iMX_Loader.h"
#endif

#include "iMX_Caches_MPU.h"

#if defined RANDOM_NUMBER_GENERATOR
    #if defined RND_HW_SUPPORT
extern void fnInitialiseRND(unsigned short *usSeedValue)
{
    #if defined TRUE_RANDOM_NUMBER_GENERATOR
    POWER_UP_ATOMIC(6, TRNG_CLOCK);                                      // power up TRNG
    TRNG0_MCTL = (TRNG_MCTL_PRGM | TRNG_MCTL_RST_DEF);                   // ensure we are in programming mode with defaults set
    TRNG0_FRQMIN = 0x640;
    TRNG0_FRQMAX = 0x6400;
    TRNG0_SDCTL = ((3200 << 16) | (2500));                               // entropy delay and sample size reduced to half of default
    TRNG0_MCTL = (TRNG_MCTL_PRGM_RUN | TRNG_MCTL_SAMP_MODE_VON_NEUMANN | TRNG_MCTL_TRNG_ACC | TRNG_MCTL_OSC_DIV_1); // set to run mode with TRNG access
    #endif
}

// Get a random number from the RNG
//
extern unsigned short fnGetRndHW(void)
{
    #if defined TRUE_RANDOM_NUMBER_GENERATOR
    static int iEntropyIndex = 24;                                       // the first 12 long words (24 half words) in the TRNG have been found to be identical after each reset so these are ignored
    volatile unsigned long *ptrEntropy = TRNG0_ENT0_ADD;                 // the first entropy register
    volatile unsigned long ulRandomValue;
        #if defined _WINDOWS
    if (IS_POWERED_UP(6, TRNG_CLOCK) == 0) {
        _EXCEPTION("Warning: TRNG0 being used before initialised!!!");
    }
        #endif
    while ((TRNG0_MCTL & TRNG_MCTL_ENT_VAL) == 0) {                      // wait until random value is ready
        #if defined _WINDOWS
        TRNG0_ENT0 = rand();
        TRNG0_ENT0 |= (rand() << 16);
        TRNG0_ENT1 = rand();
        TRNG0_ENT1 |= (rand() << 16);
        TRNG0_ENT2 = rand();
        TRNG0_ENT2 |= (rand() << 16);
        TRNG0_ENT3 = rand();
        TRNG0_ENT3 |= (rand() << 16);
        TRNG0_ENT4 = rand();
        TRNG0_ENT4 |= (rand() << 16);
        TRNG0_ENT5 = rand();
        TRNG0_ENT5 |= (rand() << 16);
        TRNG0_ENT6 = rand();
        TRNG0_ENT6 |= (rand() << 16);
        TRNG0_ENT7 = rand();
        TRNG0_ENT7 |= (rand() << 16);
        TRNG0_ENT8 = rand();
        TRNG0_ENT8 |= (rand() << 16);
        TRNG0_ENT9 = rand();
        TRNG0_ENT9 |= (rand() << 16);
        TRNG0_ENT10 = rand();
        TRNG0_ENT10 |= (rand() << 16);
        TRNG0_ENT11 = rand();
        TRNG0_ENT11 |= (rand() << 16);
        TRNG0_ENT12 = rand();
        TRNG0_ENT12 |= (rand() << 16);
        TRNG0_ENT13 = rand();
        TRNG0_ENT13 |= (rand() << 16);
        TRNG0_ENT14 = rand();
        TRNG0_ENT14 |= (rand() << 16);
        TRNG0_ENT15 = rand();
        TRNG0_ENT15 |= (rand() << 16);
        TRNG0_MCTL |= TRNG_MCTL_ENT_VAL;                                 // flag that the entropy values are valid
        #endif
        if ((TRNG0_MCTL & TRNG_MCTL_ERR) != 0) {                         // if an error is signalled
            break;
        }
        fnRetriggerWatchdog();
    }
    ptrEntropy += (iEntropyIndex/2);                                     // read entropy registers 0..15 (to give 32 half words)
    ulRandomValue = *ptrEntropy;
    if ((iEntropyIndex & 1) != 0) {
        ulRandomValue >>= 16;
    }
    if (++iEntropyIndex >= 32) {
        iEntropyIndex = 0;                                               // loop back to the start of the list
        #if defined _WINDOWS
        TRNG0_MCTL &= ~(TRNG_MCTL_ENT_VAL);                              // reading the last entropy register causes the next conversion to be started
        #endif
    }
    return (unsigned short)ulRandomValue;
    #else
    return 0;
    #endif
}
    #else
// How the random number seed is set depends on the hardware possibilities available.
//
extern void fnInitialiseRND(unsigned short *usSeedValue)
{
    if ((*usSeedValue = *ptrSeed) == 0) {                                // we hope that the content of the random seed is random after a power up
        *usSeedValue = 0x127b;                                           // we must not set a zero - so we set a fixed value
    }                                                                    // after a reset it should be well random since it will contain the value at the reset time
    *ptrSeed = fnRandom();                                               // ensure that the seed value is modified between resets
}
    #endif
#endif

#if defined iMX_RT1011
// It has been found that copying bytes from QSPI flash is unreliable (triggers a fault when copying from certain alignments)
// if a synchronisation barrier is not added after the byte read (only experienced with i.MX RT 1011)
//
unsigned char fnSynchronisedByteRead(const unsigned char *ptrByte)
{
    unsigned char Value = *ptrByte;
    _SYNCHRONISATION_BARRIER();
    return Value;
}
#endif



extern void fnDelayLoop(unsigned long ulDelay_us)
{
#if !defined TICK_USES_LPTMR && !defined TICK_USES_RTC                   // if the SYSTICK is operating we use it as a us timer for best independence of code execution speed and compiler (KL typically +15% longer then requested value between 100us and 10ms)
    #define CORE_US (SYSTICK_CLOCK/1000000)                              // the number of core clocks in a us
    register unsigned long ulMatch;
    register unsigned long _ulDelay_us = ulDelay_us;                     // ensure that the compiler puts the variable in a register rather than work with it on the stack
    (void)SYSTICK_CSR;                                                   // clear the SysTick reload flag
    ulMatch = SYSTICK_CURRENT;                                           // counter reference on entry
    if (_ulDelay_us == 0) {                                              // minimum delay is 1us
        _ulDelay_us = 1;
    }
    do {
    #if !defined _WINDOWS
        if (ulMatch >= CORE_US) {                                        // don't allow the match value to underflow
            ulMatch -= CORE_US;                                          // set the next 1us match
        }
        else {
            ulMatch = 0;                                                 // set to zero instead of under-flowing
        }
        while (SYSTICK_CURRENT > ulMatch) {                              // wait until the us period has expired
            if ((SYSTICK_CSR & SYSTICK_COUNTFLAG) != 0) {                // if we missed a reload (that is, the SysTick was reloaded with its reload value after reaching zero)
                ulMatch = SYSTICK_RELOAD;                                // prepare the next match value based on the fact that the SYSTICK has reloaded
                (void)SYSTICK_CSR;                                       // clear the SysTick reload flag
                break;                                                   // assume a single us period expired
            }
        }
    #endif
    } while (--_ulDelay_us != 0);                                        // one us period has expired so count down the requested periods until zero
#else
    volatile register unsigned long _ulDelay_us = ulDelay_us;            // ensure that the compiler puts the variable in a register rather than work with it on the stack
    volatile register unsigned long ul_us;
    while (_ulDelay_us-- != 0) {                                         // for each us required        
        ul_us = (CORE_CLOCK/8000000);                                    // tuned but may be slightly compiler dependent - interrupt processing may increase delay
        while (ul_us-- != 0) {}                                          // simple loop tuned to perform us timing
    }
#endif
}


// Basic hardware initialisation of specific hardware
//
INITHW void fnInitHW(void)                                               // perform hardware initialisation
{
#if defined _WINDOWS
    unsigned long ulPortPullups[] = {
        PORT1_DEFAULT_INPUT,                                             // set the port states out of reset in the project file app_hw_iMX.h
    #if PORTS_AVAILABLE > 1
        PORT2_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 2
        PORT3_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 3
        PORT4_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 4
        PORT5_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 5
        PORT6_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 6
        PORT7_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 7
        PORT8_DEFAULT_INPUT,
    #endif
    #if PORTS_AVAILABLE > 8
        PORT9_DEFAULT_INPUT,
    #endif
    #if defined SUPPORT_ADC
        ((ADC0_0_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_1_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_2_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_3_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_4_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_5_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_6_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_7_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_8_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_9_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_10_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_11_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_12_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_13_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_14_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_15_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_16_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_17_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_18_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_19_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_20_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_21_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_22_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC0_23_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        #if ADC_CONTROLLERS > 1
        ((ADC1_0_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_1_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_2_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_3_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_4_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_5_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_6_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_7_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_8_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_9_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_10_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_11_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_12_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_13_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_14_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_15_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_16_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_17_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_18_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_19_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_20_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_21_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_22_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC1_23_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        #endif
        #if ADC_CONTROLLERS > 2
        ((ADC2_0_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_1_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_2_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_3_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_4_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_5_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_6_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_7_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_8_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_9_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_10_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_11_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_12_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_13_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_14_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_15_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_16_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_17_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_18_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_19_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_20_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_21_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_22_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC2_23_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        #endif
        #if ADC_CONTROLLERS > 3
        ((ADC3_0_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_1_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_2_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_3_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_4_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_5_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_6_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_7_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_8_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_9_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_10_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_11_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_12_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_13_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_14_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_15_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_16_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_17_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_18_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_19_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_20_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_21_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_22_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        ((ADC3_23_START_VOLTAGE * 0xffff) / ADC_REFERENCE_VOLTAGE),
        #endif
    #endif
    };
    #if defined RANDOM_NUMBER_GENERATOR && !defined RND_HW_SUPPORT
    static unsigned short usRandomSeed = 0;
    ptrSeed = &usRandomSeed;
    #endif
    fnInitialiseDevice(ulPortPullups);
#endif
    _LowLevelInit();                                                     // configure watchdog and set the CPU to operating speed (system variables will be initialised at latest here)
#if !defined iMX_BOOTLOADER
    #if !defined DEVICE_WITHOUT_DMA
    POWER_UP_ATOMIC(5, DMA_CLOCK);                                       // power up the DMA module
        #if defined DEVICE_WITH_TWO_DMA_GROUPS
    DMA_CR = (DMA_CR_GRP0PRI_0 | DMA_CR_GRP1PRI_1);                      // set the two DMA groups to non-conflicting priorities
        #else
    DMA_CR = 0;
        #endif
    #endif
    #if defined DMA_MEMCPY_SET && !defined DEVICE_WITHOUT_DMA            // set the eDMA registers to a known zero state
    {
        unsigned long *ptr_eDMAdes = (unsigned long *)eDMA_DESCRIPTORS;
        KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
        ptrDMA_TCD += DMA_MEMCPY_CHANNEL;                                // the DMA channel used for memory copy DMA
        while (ptr_eDMAdes < eDMA_DESCRIPTORS_END) {
            *ptr_eDMAdes++ = 0;                                          // clear out DMA descriptors after reset
        }
        ptrDMA_TCD->DMA_TCD_SOFF = 4;                                    // source increment one long word for uMemcpy()
        ptrDMA_TCD->DMA_TCD_DOFF = 4;                                    // destination increment one long word
        ptrDMA_TCD->DMA_TCD_BITER_ELINK = 1;
        ptrDMA_TCD->DMA_TCD_ATTR = (DMA_TCD_ATTR_DSIZE_32 | DMA_TCD_ATTR_SSIZE_32); // default transfer sizes long words
        #if defined DMA_MEMCPY_CHANNEL_ALT
        ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
        ptrDMA_TCD += DMA_MEMCPY_CHANNEL_ALT;                            // move to the alternate channel (may be used by interrupts)
        ptrDMA_TCD->DMA_TCD_SOFF = 4;                                    // source increment one long word for uMemcpy()
        ptrDMA_TCD->DMA_TCD_DOFF = 4;                                    // destination increment one long word
        ptrDMA_TCD->DMA_TCD_BITER_ELINK = 1;
        ptrDMA_TCD->DMA_TCD_ATTR = (DMA_TCD_ATTR_DSIZE_32 | DMA_TCD_ATTR_SSIZE_32); // default transfer sizes long words
        #endif
        #if defined DMA_CHANNEL_0_PRIORITY                               // user defined channel priorities
        _SET_DMA_CHANNEL_PRIORITY(0, DMA_CHANNEL_0_PRIORITY);            // DMA priority, whereby channel can suspend a lower priority channel
        _SET_DMA_CHANNEL_PRIORITY(1, DMA_CHANNEL_1_PRIORITY);            // all channel priorities are set before use since it can be dangerous to change them later when DMA operations could take place during the process
        _SET_DMA_CHANNEL_PRIORITY(2, DMA_CHANNEL_2_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(3, DMA_CHANNEL_3_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(4, DMA_CHANNEL_4_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(5, DMA_CHANNEL_5_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(6, DMA_CHANNEL_6_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(7, DMA_CHANNEL_7_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(8, DMA_CHANNEL_8_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(9, DMA_CHANNEL_9_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(10, DMA_CHANNEL_10_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(11, DMA_CHANNEL_11_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(12, DMA_CHANNEL_12_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(13, DMA_CHANNEL_13_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(14, DMA_CHANNEL_14_PRIORITY);
        _SET_DMA_CHANNEL_PRIORITY(15, DMA_CHANNEL_15_PRIORITY);
        _SET_DMA_CHANNEL_CHARACTERISTIC(DMA_MEMCPY_CHANNEL, (DMA_DCHPRI_ECP | DMA_DCHPRI_DPA)); // can be pre-empted by higher priority channel - it is expected that this channel will normally have priority 0
            #if defined DMA_MEMCPY_CHANNEL_ALT
        _SET_DMA_CHANNEL_CHARACTERISTIC(DMA_MEMCPY_CHANNEL_ALT, (DMA_DCHPRI_ECP | DMA_DCHPRI_DPA)); // can be pre-empted by higher priority channel - it is expected that this channel will normally have priority 1
            #endif
        #else                                                            // leave default channel priority (equal to the corresponding channel number)
        _SET_DMA_CHANNEL_PRIORITY(DMA_MEMCPY_CHANNEL, (DMA_DCHPRI_DPA | DMA_DCHPRI_ECP | 0)); // {137} lowest DMA priority and can be pre-empted by higher priority channel
            #if DMA_MEMCPY_CHANNEL != 0
        _SET_DMA_CHANNEL_PRIORITY(0, (DMA_MEMCPY_CHANNEL));              // no two priorities may ever be the same when the controller is used - switch priorities to avoid
            #endif
            #if defined DMA_MEMCPY_CHANNEL_ALT
        _SET_DMA_CHANNEL_PRIORITY(DMA_MEMCPY_CHANNEL_ALT, (DMA_DCHPRI_DPA | DMA_DCHPRI_ECP | 1)); // {137} second lowest DMA priority and can be pre-empted by higher priority channel
                #if DMA_MEMCPY_CHANNEL_ALT != 1
        _SET_DMA_CHANNEL_PRIORITY(1, (DMA_MEMCPY_CHANNEL_ALT));          // no two priorities may ever be the same when the controller is used - switch priorities to avoid
                #endif
            #endif
        #endif
    }
    #endif
    fnUserHWInit();                                                      // allow the user to initialise hardware specific things - note that heap cannot be used by this routine
    #if defined _WINDOWS
    fnSimPorts(-1);                                                      // ensure port states are recognised
    #endif
#endif
}


/* =================================================================== */
/*                    General Interrupt Control                        */
/* =================================================================== */


// Routine to disable interrupts during critical region
//
extern void uDisable_Interrupt(void)
{
#if defined _WINDOWS
    iMX.CORTEX_M7_REGS.ulPRIMASK = INTERRUPT_MASKED;                     // mark that interrupts are masked
#else
    #if defined SYSTEM_NO_DISABLE_LEVEL
    uMask_Interrupt(SYSTEM_NO_DISABLE_LEVEL << __NVIC_PRIORITY_SHIFT);   // interrupts with higher priorities are not disabled
    #else
    __disable_interrupt();                                               // disable interrupts to core
    #endif
#endif
    iInterruptLevel++;                                                   // monitor the level of disable nesting
}

// Routine to re-enable interrupts on leaving a critical region (IAR uses intrinsic function)
//
extern void uEnable_Interrupt(void)
{
#if defined _WINDOWS
    if (iInterruptLevel == 0) {                                          // it is expected that this routine is only called when interrupts are presently disabled
        // A routine is enabling interrupt although they are presently off. This may not be a serious error but it is unexpected so best check why...
        //
        _EXCEPTION("Unsymmetrical use of interrupt disable/enable detected!!");
    }
#endif
    if ((--iInterruptLevel) == 0) {                                      // only when no more interrupt nesting,
#if defined _WINDOWS
        extern void fnExecutePendingInterrupts(int iRecursive);
        iMX.CORTEX_M7_REGS.ulPRIMASK = 0;                                // unmask global interrupts
    #if defined RUN_IN_FREE_RTOS
        fnExecutePendingInterrupts(0);                                   // pending interrupts that were blocked by the main task can be executed now
    #endif
#else
    #if defined SYSTEM_NO_DISABLE_LEVEL
        uMask_Interrupt(LOWEST_PRIORITY_PREEMPT_LEVEL << __NVIC_PRIORITY_SHIFT); // allow all interrupts again
    #else
        __enable_interrupt();                                            // enable processor interrupts
    #endif
#endif
    }
}

// Routine to change interrupt level mask
//
#if !defined _COMPILE_KEIL
    #if defined _GNU
        #define DONT_INLINE __attribute__((noinline))
    #else
        #define DONT_INLINE
    #endif
extern void DONT_INLINE uMask_Interrupt(unsigned char ucMaskLevel)
{
    #if !defined ARM_MATH_CM0PLUS                                         // mask not supported by Cortex-m0+
        #if defined _WINDOWS
    iMX.CORTEX_M7_REGS.ulBASEPRI = ucMaskLevel;                           // value 0 has no  - non-zero defines the base priority for exception processing (the processor does not process any exception with a priority value greater than or equal to BASEPRI))
        #else
    asm("msr basepri, r0");                                               // modify the base priority to block interrupts with a lower priority than this level
    asm("bx lr");                                                         // return
        #endif
    #endif
}
#endif



// Function used to enter processor interrupts
//
extern void fnEnterInterrupt(int iInterruptID, unsigned char ucPriority, void (*InterruptFunc)(void))
{
    volatile unsigned long *ptrIntSet = IRQ0_31_SER_ADD;
#if defined ARM_MATH_CM0PLUS                                             // only long word accesses are possible to the priority registers
    volatile unsigned long *ptrPriority = (unsigned long *)IRQ0_3_PRIORITY_REGISTER_ADD;
    int iShift;
#else
    volatile unsigned char *ptrPriority = IRQ0_3_PRIORITY_REGISTER_ADD;
#endif
#if !defined INTERRUPT_VECTORS_IN_FLASH
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    void (**processor_ints)(void);
#endif
#if defined _WINDOWS                                                     // back up the present enabled interrupt registers
    unsigned long ulState0 = IRQ0_31_SER;
    unsigned long ulState1 = IRQ32_63_SER;
    unsigned long ulState2 = IRQ64_95_SER;
    unsigned long ulState3 = IRQ96_127_SER;
    unsigned long ulState4 = IRQ128_159_SER;
    IRQ0_31_SER = IRQ32_63_SER = IRQ64_95_SER = IRQ96_127_SER = IRQ128_159_SER  = IRQ0_31_CER = IRQ32_63_CER = IRQ64_95_CER = IRQ96_127_CER = IRQ128_159_CER = 0; // reset registers
#endif
#if defined _WINDOWS                                                     // check for valid interrupt priority range
    if (ucPriority >= 16) {
        _EXCEPTION("Invalid Cortex-M7 priority being used!!");
    }
#endif
#if !defined INTERRUPT_VECTORS_IN_FLASH
    processor_ints = (void (**)(void))&ptrVect->processor_interrupts;    // first processor interrupt location in the vector table
    processor_ints += iInterruptID;                                      // move the pointer to the location used by this interrupt number
    *processor_ints = InterruptFunc;                                     // enter the interrupt handler into the vector table
#endif
    ptrPriority += iInterruptID;                                         // move to the priority location used by this interrupt
    *ptrPriority = (ucPriority << __NVIC_PRIORITY_SHIFT);                // define the interrupt's priority (16 levels for Cortex-m4 and 4 levels for Cortex-m0+)
    ptrIntSet += (iInterruptID/32);                                      // move to the interrupt enable register in which this interrupt is controlled
    *ptrIntSet = (0x01 << (iInterruptID % 32));                          // enable the interrupt
#if defined _WINDOWS
    IRQ0_31_SER  |= ulState0;                                            // synchronise the interrupt masks
    IRQ32_63_SER |= ulState1;
    IRQ64_95_SER |= ulState2;
    IRQ96_127_SER |= ulState3;
    IRQ128_159_SER |= ulState4;
    IRQ0_31_CER   = IRQ0_31_SER;
    IRQ32_63_CER  = IRQ32_63_SER;
    IRQ64_95_CER  = IRQ64_95_SER;
    IRQ96_127_CER = IRQ96_127_SER;
    IRQ128_159_CER = IRQ128_159_SER;
#endif
}

extern void fnMaskInterrupt(int iInterruptID)
{
#if defined _WINDOWS                                                     // back up the present enabled interrupt registers
    IRQ0_31_CER = IRQ32_63_CER = IRQ64_95_CER = IRQ96_127_CER = IRQ128_159_CER = 0; // reset registers
#endif
    volatile unsigned long *ptrIntMask = IRQ0_31_CER_ADD;
    ptrIntMask += (iInterruptID/32);                                     // move to the clear enable register in which this interrupt is controlled
    *ptrIntMask = (0x01 << (iInterruptID % 32));                         // disable the interrupt
#if defined _WINDOWS
    IRQ0_31_SER  &= ~IRQ0_31_CER;                                        // synchronise the interrupt masks
    IRQ32_63_SER &= ~IRQ32_63_CER;
    IRQ64_95_SER &= ~IRQ64_95_CER;
    IRQ96_127_SER &= ~IRQ96_127_CER;
    IRQ128_159_SER &= ~IRQ128_159_CER;
    IRQ0_31_CER   = IRQ0_31_SER;
    IRQ32_63_CER  = IRQ32_63_SER;
    IRQ96_127_CER = IRQ96_127_SER;
    IRQ128_159_CER = IRQ128_159_SER;
#endif
}

extern void fnUnmaskInterrupt(int iInterruptID)
{
#if defined _WINDOWS                                                     // back up the present enabled interrupt registers
    unsigned long ulState0 = IRQ0_31_SER;
    unsigned long ulState1 = IRQ32_63_SER;
    unsigned long ulState2 = IRQ64_95_SER;
    unsigned long ulState3 = IRQ96_127_SER;
    unsigned long ulState4 = IRQ128_159_SER;
    IRQ0_31_SER = IRQ32_63_SER = IRQ64_95_SER = IRQ96_127_SER = IRQ128_159_SER = IRQ0_31_CER = IRQ32_63_CER = IRQ64_95_CER = IRQ96_127_CER = IRQ128_159_CER = 0; // reset registers
#endif
    volatile unsigned long *ptrIntSet = IRQ0_31_SER_ADD;
    ptrIntSet += (iInterruptID / 32);                                    // move to the clear enable register in which this interrupt is controlled
    *ptrIntSet = (0x01 << (iInterruptID % 32));                          // enable the interrupt
#if defined _WINDOWS
    IRQ0_31_SER |= ulState0;                                            // synchronise the interrupt masks
    IRQ32_63_SER |= ulState1;
    IRQ64_95_SER |= ulState2;
    IRQ96_127_SER |= ulState3;
    IRQ128_159_SER |= ulState4;
    IRQ0_31_CER = IRQ0_31_SER;
    IRQ32_63_CER = IRQ32_63_SER;
    IRQ64_95_CER = IRQ64_95_SER;
    IRQ96_127_CER = IRQ96_127_SER;
    IRQ128_159_CER = IRQ128_159_SER;
#endif
}

extern int fnIsPending(int iInterruptID)
{
    volatile unsigned long *ptrIntActive = IRQ0_31_CPR_ADD;              // the first clear pending register, which also shows pending interrupts
    ptrIntActive += (iInterruptID/32);                                   // move to the clear pending interrupt enable register in which this interrupt is controlled
    return ((*ptrIntActive & (0x01 << (iInterruptID % 32))) != 0);       // return the pending state of this interrupt
}


/* =================================================================== */
/*                                 TICK                                */
/* =================================================================== */

#if !defined APPLICATION_WITHOUT_TICK
// Tick interrupt
//
static __interrupt void _RealTimeInterrupt(void)
{
#if defined TICK_USES_LPTMR                                              // tick interrupt from low power timer
    LPTMR0_CSR = LPTMR0_CSR;                                             // clear pending interrupt
#elif defined TICK_USES_RTC                                              // tick interrupt from RTC
    RTC_SC |= RTC_SC_RTIF;                                               // clear pending interrupt
#else                                                                    // tick interrupt from systick
    INT_CONT_STATE_REG = PENDSTCLR;                                      // reset interrupt
    #if defined _WINDOWS
    INT_CONT_STATE_REG &= ~(PENDSTSET | PENDSTCLR);
    #endif
#endif
#if defined RUN_IN_FREE_RTOS && !defined _WINDOWS
    xPortSysTickHandler();
#endif
    uDisable_Interrupt();                                                // ensure tick handler cannot be interrupted
        fnRtmkSystemTick();                                              // operating system tick
    uEnable_Interrupt();
}
#endif

#if defined SUPPORT_LOW_VOLTAGE_DETECTION                                // enable low voltage detection interrupt warning
// Interrupt to warn that the voltage is close to the reset threshold
// - the interrupt is disabled since in the case of power loss in progress it will not be possible to clear the interrupt flag
// - the user callback can request the interrupt to be re-enabled and a clear be attempted by returing a non-zero value if it is prepared to handle multiple interrupts
//
static __interrupt void _low_voltage_irq(void)
{
    PMC_LVDSC2 &= ~(PMC_LVDSC2_LVWIE);                                   // disable further interrupts so that the processor can continue operation (it can decide to re-enable interrupts if desired)
    if (fnPowerFailureWarning() != 0) {                                  // user supplied routine to handle power faiure (interrupt call back)
        OR_ONE_TO_CLEAR(PMC_LVDSC2, (PMC_LVDSC2_LVWACK));                // acknowledge the interrupt and attempt to clear the flag (in the case of power loss in progress this will never be able to clear the flag)
        PMC_LVDSC2 |= (PMC_LVDSC2);                                      // re-enable the interrupt
    }
}
#endif

#if !defined APPLICATION_WITHOUT_TICK
// Routine to initialise the tick interrupt (uses Cortex M7 SysTick timer, RTC or low power timer)
//
extern void fnStartTick(void)
{
#if defined TICK_USES_LPTMR                                              // use the low power timer to derive the tick interrupt from
    POWER_UP_ATOMIC(5, LPTMR0);                                          // ensure that the timer can be accessed
    LPTMR0_CSR = LPTMR_CSR_TCF;                                          // reset the timer and ensure no pending interrupts
    #if defined LPTMR_CLOCK_LPO                                          // define the low power clock speed for calculations
    LPTMR0_PSR = (LPTMR_PSR_PCS_LPO | LPTMR_PSR_PBYP);
    #elif defined LPTMR_CLOCK_INTERNAL_30_40kHz
    MCG_C2 &= ~MCG_C2_IRCS;                                              // select slow internal reference clock
    LPTMR0_PSR = (LPTMR_PSR_PCS_MCGIRCLK; | LPTMR_PSR_PBYP);
    #elif defined LPTMR_CLOCK_INTERNAL_4MHz
    MCG_C2 |= MCG_C2_IRCS;                                               // select fast internal reference clock
    LPTMR0_PSR = (LPTMR_PSR_PCS_MCGIRCLK | LPTMR_PSR_PBYP);
    #elif defined LPTMR_CLOCK_RTC_32kHz
    POWER_UP_ATOMIC(6, RTC);                                             // enable access and interrupts to the RTC
    if ((RTC_SR & RTC_SR_TIF) != 0) {                                    // if timer invalid
        RTC_SR = 0;                                                      // ensure stopped
        RTC_TSR = 0;                                                     // write to clear RTC_SR_TIF in status register when not yet enabled
        #if !defined KINETIS_KL
        RTC_CR = (RTC_CR_OSCE);                                          // enable oscillator and supply it to other peripherals
        #endif
    }
    SIM_SOPT1 = ((SIM_SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) | SIM_SOPT1_OSC32KSEL_32k); // select ERCLK32K from the RTC 32k clock
        #if defined KINETIS_K22
  //SIM_SOPT1 = ((SIM_SOPT1 & ~SIM_SOPT1_OSC32KOUT_MASK) | SIM_SOPT1_OSC32KOUT_PTE0); // 32kHz output to CLKOUT32k pin
        #endif
        #if defined LPTMR_PRESCALE
    LPTMR0_PSR = (LPTMR_PSR_PCS_OSC0ERCLK | ((LPTMR_PRESCALE_VALUE) << LPTMR_PSR_PRESCALE_SHIFT)); // program prescaler
        #else
    LPTMR0_PSR = (LPTMR_PSR_PCS_ERCLK32K | LPTMR_PSR_PBYP);
        #endif
    #else                                                                // LPTMR_CLOCK_EXTERNAL_32kHz or LPTMR_CLOCK_OSCERCLK
    OSC0_CR |= (OSC_CR_ERCLKEN | OSC_CR_EREFSTEN);                       // enable the external reference clock and keep it enabled in stop mode
        #if defined LPTMR_CLOCK_EXTERNAL_32kHz                           // 32kHz crystal
            #if defined LPTMR_PRESCALE
    LPTMR0_PSR = (LPTMR_PSR_PCS_OSC0ERCLK | ((LPTMR_PRESCALE_VALUE) << LPTMR_PSR_PRESCALE_SHIFT)); // program prescaler
            #else
    LPTMR0_PSR = (LPTMR_PSR_PCS_ERCLK32K | LPTMR_PSR_PBYP);
            #endif
        #else
            #if defined LPTMR_PRESCALE
    LPTMR0_PSR = (LPTMR_PSR_PCS_OSC0ERCLK | ((LPTMR_PRESCALE_VALUE) << LPTMR_PSR_PRESCALE_SHIFT)); // program prescaler
            #else
    LPTMR0_PSR = (LPTMR_PSR_PCS_OSC0ERCLK | LPTMR_PSR_PBYP);
            #endif
        #endif
    #endif
    fnEnterInterrupt(irq_LPTMR0_ID, LPTMR0_INTERRUPT_PRIORITY, (void (*)(void))_RealTimeInterrupt); // enter interrupt handler
    LPTMR0_CSR |= LPTMR_CSR_TIE;                                         // enable timer interrupt
    LPTMR0_CMR = LPTMR_US_DELAY((TICK_RESOLUTION));                      // TICK period
    #if defined _WINDOWS
    if (LPTMR0_CMR > 0xffff) {
        _EXCEPTION("LPTMR0_CMR value too large (16 bits)");
    }
    #endif
    LPTMR0_CSR |= LPTMR_CSR_TEN;                                         // enable the low power timer
#elif defined TICK_USES_RTC                                              // use RTC to derive the tick interrupt from
    POWER_UP_ATOMIC(6, RTC);                                             // ensure the RTC is powered
    fnEnterInterrupt(irq_RTC_OVERFLOW_ID, PRIORITY_RTC, (void (*)(void))_RealTimeInterrupt); // enter interrupt handler
    #if defined RTC_USES_EXT_CLK
    RTC_MOD = (unsigned long)((((unsigned long long)((unsigned long long)TICK_RESOLUTION * (unsigned long long)_EXTERNAL_CLOCK)/RTC_CLOCK_PRESCALER_1)/1000000) - 1); // set the match value
    #elif defined RTC_USES_INT_REF
    RTC_MOD = ((((TICK_RESOLUTION * ICSIRCLK)/RTC_CLOCK_PRESCALER_1)/1000000) - 1); // set the match value
    #else
    RTC_MOD = ((((TICK_RESOLUTION * ICSIRCLK) / RTC_CLOCK_PRESCALER_2) / 1000000) - 1); // set the match value
    #endif
    RTC_SC = (RTC_SC_RTIE | RTC_SC_RTIF | _RTC_CLOCK_SOURCE | _RTC_PRESCALER); // clock the RTC from the defined clock source/pre-scaler and enable interrupt
    #if defined _WINDOWS
    if (RTC_MOD > 0xffff) {
        _EXCEPTION("MOD value too large (16 bits)");
    }
    #endif
#else                                                                    // use systick to derive the tick interrupt from
    #define REQUIRED_US (1000000/(TICK_RESOLUTION))                      // the TICK frequency we require in MHz
    #define TICK_DIVIDE (((SYSTICK_CLOCK + REQUIRED_US/2)/REQUIRED_US) - 1) // the divide ratio required (for systick)

    #if TICK_DIVIDE > 0x00ffffff
        #error "TICK value cannot be achieved with SYSTICK at this core frequency!!"
    #endif
    #if !defined INTERRUPT_VECTORS_IN_FLASH
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    ptrVect->ptrSysTick = _RealTimeInterrupt;                            // enter interrupt handler
    #endif
    SYSTICK_RELOAD = TICK_DIVIDE;                                        // set reload value to determine the period
    SYSTICK_CURRENT = TICK_DIVIDE;
    SYSTEM_HANDLER_12_15_PRIORITY_REGISTER |= (unsigned long)((unsigned long long)SYSTICK_PRIORITY << (24 + __NVIC_PRIORITY_SHIFT)); // enter the SYSTICK priority
    #if defined SYSTICK_EXTERNAL_CLOCK_SOURCE                            // SYSTICK clocked from OSC_CLK divied down to 100kHz
    SYSTICK_CSR = (SYSTICK_ENABLE | SYSTICK_TICKINT);                    // enable timer and its interrupt
    #else
    SYSTICK_CSR = (SYSTICK_CORE_CLOCK | SYSTICK_ENABLE | SYSTICK_TICKINT); // enable timer and its interrupt
    #endif
    #if defined _WINDOWS
    SYSTICK_RELOAD &= SYSTICK_COUNT_MASK;                                // mask any values which are out of range
    SYSTICK_CURRENT = SYSTICK_RELOAD;                                    // prime the reload count
    #endif
#endif
#if defined MONITOR_PERFORMANCE                                          // configure a timer that will be used to measure the duration of task operation
    INITIALISE_MONITOR_TIMER();
#endif
#if defined SUPPORT_LOW_VOLTAGE_DETECTION                                // enable low voltage detection interrupt warning
    #if !defined LOW_VOLTAGE_DETECTION_VOLTAGE_mV                        // if no value is defined we delault to 2.10V warning threshold and 1.6V reset threshold
        #define LOW_VOLTAGE_DETECTION_VOLTAGE_mV   2100
    #endif
    fnEnterInterrupt(irq_LOW_VOLTAGE_ID, 0, _low_voltage_irq);           // enter highest priority interrupt to warn of failing voltage
    #if LOW_VOLTAGE_DETECTION_VOLTAGE_mV > 2400                          // sensitive detection level
    // K64 reference: high reset threshold is typically 2.56V
    //
    PMC_LVDSC1 = (PMC_LVDSC1_LVDV | PMC_LVDSC1_LVDRE | PMC_LVDSC1_LVDACK); // high voltage level trip with reset enabled (clear flag)
        #if LOW_VOLTAGE_DETECTION_VOLTAGE_mV >= 3000
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_HIGH | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 3.00V typical
        #elif LOW_VOLTAGE_DETECTION_VOLTAGE_mV >= 2950
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_MID2 | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 2.90V typical
        #elif LOW_VOLTAGE_DETECTION_VOLTAGE_mV >= 2850
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_MID1 | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 2.80V typical
        #else
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_LOW | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 2.70V typical
        #endif
    #else
    // K64 reference: low reset threshold is typically 1.6V
    //
    PMC_LVDSC1 = (PMC_LVDSC1_LVDRE | PMC_LVDSC1_LVDACK);                 // low voltage level trip with reset enabled (clear flag)
        #if LOW_VOLTAGE_DETECTION_VOLTAGE_mV > 2050
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_HIGH | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 2.10V typical
        #elif LOW_VOLTAGE_DETECTION_VOLTAGE_mV >= 1950
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_MID2 | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 2.00V typical
        #elif LOW_VOLTAGE_DETECTION_VOLTAGE_mV >= 1850
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_MID1 | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 1.90V typical
        #else
    PMC_LVDSC2 = (PMC_LVDSC2_LVWV_LOW | PMC_LVDSC2_LVWIE | PMC_LVDSC2_LVWACK); // enable low voltage warning interrupt (clear flag) 1.80V typical
        #endif
    #endif
#endif
}
#endif

/* =================================================================== */
/*                             Watchdog                                */
/* =================================================================== */

// Support watchdog re-triggering of specific hardware
//
extern void fnRetriggerWatchdog(void)
{
    if ((WDOG1_WCR & WDOG_WCR_WDE) != 0) {                               // if the WDOG1 is enabled retrigger it
        KICK_WATCHDOG_1();
    }
    if ((WDOG2_WCR & WDOG_WCR_WDE) != 0) {                               // if the WDOG2 is enabled retrigger it
        KICK_WATCHDOG_2();
    }
    if ((WDOG3_CS & WDOG_CS_EN) != 0) {                                  // if the WDOG3 is enabled retrigger it
        KICK_WATCHDOG_3();
    }
    TOGGLE_WATCHDOG_LED();                                               // optionally flash a watchdog (heart-beat) LED
}

/* =================================================================== */
/*                       Clock configuration                           */
/* =================================================================== */

#include "iMX_Clock.h"

#if !defined DEVICE_WITHOUT_DMA
/* =================================================================== */
/*                                 DMA                                 */
/* =================================================================== */
    #define _DMA_SHARED_CODE
        #include "../Kinetis/kinetis_DMA.h"                              // include driver code for peripheral/buffer DMA
    #undef _DMA_SHARED_CODE
    #define _DMA_MEM_TO_MEM
        #include "../Kinetis/kinetis_DMA.h"                              // include memory-memory transfer code 
    #undef _DMA_MEM_TO_MEM
#endif

#if (defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET)
/* =================================================================== */
/*                          Ethernet Controller                        */
/* =================================================================== */
    #include "../Kinetis/kinetis_ENET.h"                                 // include Ethernet controller hardware driver code
#endif

#if defined USB_INTERFACE
/* =================================================================== */
/*                                USB                                  */
/* =================================================================== */
    #include "../Kinetis/kinetis_USB_HS_OTG.h"                           // include USB controller hardware HS device driver code
#endif

#if defined SERIAL_INTERFACE
/* =================================================================== */
/*                    Serial Interface - UART                          */
/* =================================================================== */
    #include "../Kinetis/kinetis_UART.h"                                 // include LPUART hardware driver code (driver shared with kinetis)
#endif

#if defined SPI_INTERFACE
/* =================================================================== */
/*                            SPI Interface                            */
/* =================================================================== */
    #include "iMX_SPI.h"                                                 // include SPI hardware driver code
#endif

#if defined CAN_INTERFACE && (NUMBER_OF_CAN_INTERFACES > 0)
/* =================================================================== */
/*                            FlexCAN/MSCAN                            */
/* =================================================================== */
    #include "../Kinetis/kinetis_CAN.h"                                  // include FlexCAN hardware driver code
#endif

#if defined I2C_INTERFACE
/* =================================================================== */
/*                                  I2C                                */
/* =================================================================== */
    #include "../Kinetis/kinetis_LPI2C.h"                                // include LPI2C hardware driver code (driver shared with kinetis)
#endif

#if defined SUPPORT_RTC || defined SUPPORT_SW_RTC
/* =================================================================== */
/*                           Real Time Clock                           */
/* =================================================================== */
    #include "iMX_RTC.h"                                                 // include RTC driver code
#endif

#if defined SDCARD_SUPPORT && defined SD_CONTROLLER_AVAILABLE
/* =================================================================== */
/*                                SDHC                                 */
/* =================================================================== */
    #include "../Kinetis/kinetis_SDHC.h"                                 // include SDHC driver code
#endif

#if (defined SUPPORT_PITS || defined USB_HOST_SUPPORT) && !defined KINETIS_WITHOUT_PIT
/* =================================================================== */
/*                     Periodic Interrupt Timer (PIT)                  */
/* =================================================================== */
#define _PIT_CODE
    #include "../Kinetis/kinetis_PIT.h"                                  // include PIT driver code
#undef _PIT_CODE
#endif

#if defined SUPPORT_LPTMR && !defined TICK_USES_LPTMR && !defined KINETIS_KE
/* =================================================================== */
/*                       Low Power Timer (LPTMR)                       */
/* =================================================================== */
#define _LPTMR_CODE
    #include "iMX_LPTMR.h"                                               // include LPTMR driver code
#undef _LPTMR_CODE
#endif

#if defined SUPPORT_TIMER && (defined SUPPORT_PWM_MODULE || defined SUPPORT_CAPTURE)
/* =================================================================== */
/*                            GPT pin connections                      */
/* =================================================================== */
    #include "iMX_timer_pins.h"
#endif

#if defined SUPPORT_TIMER                                                // basic timer support based on GPT
/* =================================================================== */
/*                                 GPT                                 */
/* =================================================================== */
#define _GPT_CODE
    #include "iMX_GPT.h"                                                  // include GPT driver code
#undef _GPT_CODE
#endif

#if defined SUPPORT_TIMER && defined SUPPORT_PWM_MODULE
/* =================================================================== */
/*                                 PWM                                 */
/* =================================================================== */
#define _PWM_CODE
    #include "iMX_PWM.h"                                                 // include PWM code
#undef _PWM_CODE
#endif


#if defined SUPPORT_KEYBOARD_INTERRUPTS && (KBIS_AVAILABLE > 0)
/* =================================================================== */
/*                        Key Board Interrupt                          */
/* =================================================================== */
#define _KBI_INTERRUPT_CODE
    #include "iMX_KBI.h"                                                 // include KBI interrupt driver code
#undef _KBI_INTERRUPT_CODE
#endif

#if defined SUPPORT_I2S_SAI && (I2S_AVAILABLE > 0)
/* =================================================================== */
/*                                I2S / SAI                            */
/* =================================================================== */
#define _I2S_CODE
    #include "iMX_I2S.h"                                                 // include I2S/SAI driver code
#undef _I2S_CODE
#endif

#if defined SUPPORT_PORT_INTERRUPTS
/* =================================================================== */
/*                           Port Interrupts                           */
/* =================================================================== */
#define _PORT_INTERRUPT_CODE
    #include "iMX_PORTS.h"                                               // include port interrupt driver code
#undef _PORT_INTERRUPT_CODE
#endif

#define _PORT_MUX_CODE
    #include "iMX_PORTS.h"                                               // include port pin multiplexing code
#undef _PORT_MUX_CODE


#if defined SUPPORT_ADC
/* =================================================================== */
/*                                 ADC                                 */
/* =================================================================== */
#define _ADC_INTERRUPT_CODE
    #include "iMX_ADC.h"                                                 // include driver code for ADC
#undef _ADC_INTERRUPT_CODE
#endif

#if defined SUPPORT_COMPARATOR
/* =================================================================== */
/*                            Comparator                               */
/* =================================================================== */
#define _CMP_INTERRUPT_CODE
    #include "iMX_CMP.h"                                                 // include driver code for comparator
#undef _CMP_INTERRUPT_CODE
#endif


/* =================================================================== */
/*                General Peripheral/Interrupt Interface               */
/* =================================================================== */

// Configure a specific interrupt, including processor specific settings and enter a handler routine.
// Some specific peripheral control may be performed here.
//
extern void fnConfigureInterrupt(void *ptrSettings)
{
    switch (((INTERRUPT_SETUP *)ptrSettings)->int_type) {
#if defined SUPPORT_KEYBOARD_INTERRUPTS && (KBIS_AVAILABLE > 0)
    case KEYBOARD_INTERRUPT:
    #define _KBI_CONFIG_CODE
        #include "iMX_KBI.h"                                             // include KBI configuration code
    #undef _KBI_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_PORT_INTERRUPTS
    case PORT_INTERRUPT:
    #define _PORT_INT_CONFIG_CODE
        #include "iMX_PORTS.h"                                           // include port interrupt configuration code
    #undef _PORT_INT_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_I2S_SAI && (I2S_AVAILABLE > 0)
    case I2S_SAI_INTERRUPT:
    #define _I2S_SAI_CONFIG_CODE
        #include "iMX_I2S.h"                                             // include I2S/SAI configuration code
    #undef _I2S_SAI_CONFIG_CODE
        break;
#endif
#if (defined SUPPORT_PITS || defined USB_HOST_SUPPORT) && !defined IMX_WITHOUT_PIT // up to 4 x 32bit PITs
    case PIT_INTERRUPT:
    #define _PIT_CONFIG_CODE
        #include "../Kinetis/kinetis_PIT.h"                              // include PIT configuration code
    #undef _PIT_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_TIMER && defined SUPPORT_PWM_MODULE
    case PWM_INTERRUPT:
    #define _PWM_CONFIG_CODE
        #include "iMX_PWM.h"                                             // include PWM configuration code
    #undef _PWM_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_TIMER
    case TIMER_INTERRUPT:                                                // GPT used in periodic or monostable interrupt mode
    #define _GPT_CONFIG_CODE
        #include "iMX_GPT.h"                                             // include GPT configuration code
    #undef _GPT_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_LPTMR && !defined TICK_USES_LPTMR && !defined KINETIS_KE // [prescale bypassed]
    case LPTMR_INTERRUPT:
    #define _LPTMR_CONFIG_CODE
        #include "iMX_LPTMR.h"                                           // include LPTMR configuration code
    #undef _LPTMR_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_DAC && (DAC_CONTROLLERS > 0)
    case DAC_INTERRUPT:
    #define _DAC_CONFIG_CODE
        #include "iMX_DAC.h"                                             // include DAC configuration code
    #undef _DAC_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_ADC
    case ADC_INTERRUPT:
    #define _ADC_CONFIG_CODE
        #include "iMX_ADC.h"                                             // include ADC configuration code
    #undef _ADC_CONFIG_CODE
        break;
#endif
#if defined SUPPORT_COMPARATOR
    case COMPARATOR_INTERRUPT:
    #define _CMP_CONFIG_CODE
        #include "iMX_CMP.h"                                             // include comparator configuration code
    #undef _CMP_CONFIG_CODE
        break;
#endif
    default: 
        _EXCEPTION("Attempting configuration of interrupt interface without appropriate support enabled!!");
        break;
    }
}


/* =================================================================== */
/*                     Dynamic Low Power Interface                     */
/* =================================================================== */

#if defined SUPPORT_LOW_POWER
    #include "iMX_low_power.h"                                           // include driver code for low power mode
#endif


/* =================================================================== */
/*                                 Reset                               */
/* =================================================================== */

// This routine is called to reset the processor
//
extern void fnResetBoard(void)
{
    uDisable_Interrupt();
    APPLICATION_INT_RESET_CTR_REG = (VECTKEY | SYSRESETREQ);             // request cortex core reset, which will cause the software reset bit to be set in the mode controller for recognition after restart
    IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // set the FlexRAM layout back to that taken from the eFuse setting (default configuration) - although the reset is commanded before executing this line it still operates and avoids the RAM layout being changed when the code is still running
#if !defined _WINDOWS
    FOREVER_LOOP() {}
#endif
}


#if !defined NO_FLASH_SUPPORT && (defined SPI_SW_UPLOAD || defined SPI_FLASH_FAT || ((defined SPI_FILE_SYSTEM && defined FLASH_FILE_SYSTEM) && !defined iMX_BOOTLOADER))
// Routine to request local SPI FLASH type
//
extern unsigned char fnSPI_Flash_available(void)
{
    return ucSPI_FLASH_Type[0];
}

    #if defined SPI_FLASH_MULTIPLE_CHIPS
extern unsigned char fnSPI_FlashExt_available(int iExtension)
{
    if (iExtension >= SPI_FLASH_DEVICE_COUNT) {
        return 0;
    }
    return ucSPI_FLASH_Type[iExtension];
}
    #endif
#endif


#if !defined _MINIMUM_IRQ_INITIALISATION && !defined NMI_IN_FLASH && !defined iMX_BOOTLOADER
// Serious error interrupts
//
static void irq_hard_fault(void)
{
}

static void irq_memory_man(void)
{
}

static void irq_bus_fault(void)
{
}

static void irq_usage_fault(void)
{
}

static void irq_debug_monitor(void)
{
}

    #if !defined RUN_IN_FREE_RTOS
static void irq_SVCall(void)
{
}

static void irq_pend_sv(void)
{
}
    #endif
#endif

#if (!defined _MINIMUM_IRQ_INITIALISATION || defined NMI_IN_FLASH) && !defined iMX_BOOTLOADER
static void irq_NMI(void)
{
}
#endif

#if !defined iMX_BOOTLOADER
// Default handler to indicate processor input from unconfigured source
//
static void irq_default(void)
{
    _EXCEPTION("undefined interrupt!!! - read the exception number in the Cortex CPU IPSR register to see which vector is pending");
}
#endif


#if defined (_GNU)
extern unsigned char __data_start__, __data_end__;
extern const unsigned char __data_load_start__;
extern const unsigned char __text_load_start__;
extern unsigned char __bss_start__, __bss_end__;
    #if defined iMX_BOOT_RUN_RAM
extern const unsigned long __text_start__, __rodata_start__;
extern const unsigned long __text_end__, __rodata_run_end__;
extern unsigned long __prog_run_start__, __rodata_run__;
    #endif

// Variable initialisation
//
static void __init_gnu_data(void)
{
    #if defined iMX_BOOT_RUN_RAM
    const unsigned long *ptrCodeSource = &__text_start__;                // source in QSPI
    unsigned long *ptrCode = &__prog_run_start__;                        // destination in ITC, after interrupt vectors
    unsigned long *ptrConst = &__rodata_run__;                           // destination in DTC
    #endif
    unsigned char *ptrData;
    unsigned long ulInitDataLength;
    const unsigned char *ptrFlash = &__data_load_start__;
    ulInitDataLength = (&__data_end__ - &__data_start__);
    ptrData = &__data_start__;
    while (ulInitDataLength-- != 0) {                                    // initialise data
        *ptrData++ = *ptrFlash++;
    }
    ptrData = &__bss_start__;
    ulInitDataLength = (&__bss_end__ - &__bss_start__);
    while (ulInitDataLength-- != 0) {                                    // initialise bss
        *ptrData++ = 0;
    }
    #if defined iMX_BOOT_RUN_RAM
    // Copy the code from QSPI flash to ITC memory
    //
    ulInitDataLength = (((&__text_end__ - &__text_start__) + (sizeof(unsigned long) - 1)) / sizeof(unsigned long));
    while (ulInitDataLength-- != 0) {
        *ptrCode++ = *ptrCodeSource++;
    }
    ptrCodeSource = &__rodata_start__;                                   // source in QSPI
    ulInitDataLength = (((&__rodata_run_end__ - &__rodata_run__) + (sizeof(unsigned long) - 1)) / sizeof(unsigned long));
    while (ulInitDataLength-- != 0) {
        *ptrConst++ = *ptrCodeSource++;
    }
    #endif
}
#endif


#if defined iMX_BOOTLOADER
static void fnWaitDebugger(void)
{
    if (*BOOT_MAIL_BOX == RESET_TO_DEBUGGER) {                           // the application has commanded that the start be delayed so that the debugger could be connected at the start of teh program
        volatile unsigned long ulStartDelay = (10000000/4);
        while (ulStartDelay-- != 0) {                                    // wait in a startup delay loop to allow debugger connection
            IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // immediately after a reset ensure that the FlexRAM layout is taken from the eFuse setting (default configuration)
            fnRetriggerWatchdog();
        }
    }
    FOREVER_LOOP() {
        if (WAIT_DEBUGGER_NOT_ACTIVE()) {                                // if button is pressed wait in loop until it is releaed
            break;
        }
        else {
            IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // immediately after a reset ensure that the FlexRAM layout is taken from the eFuse setting (default configuration)
            fnRetriggerWatchdog();
        }
    }
}
#else
// If the watchdog fires this interrupt is called 255 clock cycles before the watchdog reset takes place
//
static __interrupt void wdog3_irq(void)
{
    IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // set the FlexRAM layout back to that taken from the eFuse setting (default configuration)
    FOREVER_LOOP() {}
}
#endif


// Beware that these routines work with stack memory only - there are no initialised variables yet
//
static void fnStartupCode(void)
{
#if defined iMX_BOOTLOADER
    fnSaveResetCause();                                                  // save the cause of the reset to persistent RAM
#endif
    fnDisableClocks();                                                   // disable all clocks by default - they will be enabled only when needed
    INIT_WATCHDOG_DISABLE();                                             // optionally configure an input used to control watchdog operation
#if defined iMX_BOOTLOADER
    CONFIG_WAIT_INPUT();                                                 // optionally prepare an input that can delay startup in order to help debugger connection
#endif
    fnConfigWdogs();                                                     // configure the watchdogs
#if defined iMX_BOOTLOADER
    fnWaitDebugger();
#endif
#if defined USER_STARTUP_CODE
    USER_STARTUP_CODE;                                                   // allow user defined start-up code immediately after the watchdog configuration and before clock configuration to be defined
#endif
    fnConfigure_iMX_RT_Clocks();                                         // configure clocks
    fnInitialise_iMX_caches_mpu();                                       // configure cache and MPU
    INIT_WATCHDOG_LED();                                                 // allow user configuration of a blink LED
}

#if defined _COMPILE_IAR || defined _COMPILE_GHS
// This is the first function called so that it can immediately disable the watchdog so that it doesn't fire during subsequent variable initialisation
// - called by start-up code by jump
//
static void iar_ghs_entry(void)
{
    #if defined iMX_BOOTLOADER
    do {
        IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // immediately after a reset ensure that the FlexRAM layout is taken from the eFuse setting (default configuration)
    } while (IOMUXC_GPR_GPR16 != (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN));
    #endif
    fnStartupCode();                                                     // execute initialisation code (watchdog, clocks, cache)
    #if defined _COMPILE_GHS
    _start_T();                                                          // now call the GHS initialisation code which initialises variables and then calls main() 
    #else
    __iar_program_start();                                               // now call the IAR initialisation code which initialises variables and then calls main() 
    #endif
}
#endif

#if defined _COMPILE_KEIL
// Keil demands the use of a __main() call to correctly initialise variables - it then calls main()
//
extern void keil_entry(void)
{
    #if defined iMX_BOOTLOADER
    IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_RESERVED | IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN); // immediately after a reset ensure that the FlexRAM layout is taken from the eFuse setting (default configuration)
    #endif
    fnStartupCode();                                                     // execute initialisation code (watchdog, clocks, cache)
    __main();                                                            // Keil initialises variables and then calls main()
}
#endif

// Perform very low level initialisation
//
static void _LowLevelInit(void)
{
#if !defined INTERRUPT_VECTORS_IN_FLASH && !defined iMX_BOOTLOADER
    VECTOR_TABLE *ptrVect;
        #if !defined _MINIMUM_IRQ_INITIALISATION
    void (**processor_ints)(void);
        #endif
#endif
#if defined _WINDOWS                                                     // check the DCD when simulating
    fnCheckDCD();                                                        // check the validity of the DCD table that may be interpreted by the boot ROM
#endif
#if !defined _COMPILE_IAR && !defined _COMPILE_KEIL
    fnStartupCode();                                                     // execute initialisation code (watchdog, clocks, cache)
#endif
#if defined _GNU
    __init_gnu_data();                                                   // initialise variables
#endif
#if defined iMX_BOOT && (defined SPI_SW_UPLOAD || defined SPI_FLASH_FAT || ((defined SPI_FILE_SYSTEM || defined SPI_EEPROM_FILE_SYSTEM) && defined FLASH_FILE_SYSTEM))
    #define _CHECK_SPI_CHIPS                                             // insert manufacturer dependent code to configure and check the SPI flash chip(s)
    #if defined SPI_FILE_SYSTEM
        #include "../SPI_Memory/spi_flash_AT25SF.h"
        #include "../SPI_Memory/spi_flash_IS25LP.h"
        #include "../SPI_Memory/spi_flash_S26KL.h"
        #include "../SPI_Memory/spi_flash_ATXP.h"
        #include "../SPI_Memory/spi_flash_w25q.h"
    #endif
    #if defined SPI_EEPROM_FILE_SYSTEM
        #include "spi_eeprom_iMX_25AA160.h"
    #endif
    #undef _CHECK_SPI_CHIPS
#endif
#if defined iMX_BOOTLOADER
    if (*BOOT_MAIL_BOX == RESET_TO_APPLICATION) {                        // if the serial loader or application have commanded the application to be started
        start_application(iMX_APPLICATION_LOCATION + iMX_FILE_OBJECT_SIZE); // jump to the application
        // If this call returns it means that there is no application in place
        //
    }
    if (*BOOT_MAIL_BOX != RESET_TO_FALLBACK_LOADER) {                    // as long as the serial loader or application have not commanded the fall-back loader to be started
        start_application(iMX_SERIAL_LOADER_LOCATION + iMX_FILE_OBJECT_SIZE); // jump to the serial loader
        // If this call returns it means that there is no serial loader in place se we fallback to the fall-back loader
        //
    }
    start_application(iMX_FALLBACK_LOADER_LOCATION);                     // jump to the fall-back loader - this is expected to always be loaded together with the bare-minimum loader and will never return
    FOREVER_LOOP() {                                                     // if this were to return it would mean that the fallback serial loader is either missing or corrupt so we wait here for someone to debug the cause
        fnRetriggerWatchdog();                                           // don't allow the watchdog to fire - a debugger can connect to verify that we are here - this will normally never happen since the boot loader is combined and loaded with the fall-back serial loader
        #if defined _WINDOWS
        break;
        #endif
    }
#else
    #if defined INTERRUPT_VECTORS_IN_FLASH
    VECTOR_TABLE_OFFSET_REG = ((unsigned long)&__vector_table);
    #else
        #if defined _WINDOWS
    VECTOR_TABLE_OFFSET_REG = (unsigned long)&vector_ram;                // position the vector table at the bottom of RAM
        #else
    VECTOR_TABLE_OFFSET_REG = (unsigned long)RAM_START_ADDRESS_ITC;      // position the vector table at the bottom of instruction RAM
        #endif
    ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;                   // note that if the vector table is in i.MX RT ITCM it is at address 0x00000000, which is a NULL pointer
                                                                         // by reading back this value from the register (declared as volatile) the compiler is not allowed to assume that a NULL pointer is being used
                                                                         // which ensures that it doesn't trigger the compiler undefined behaviour when it knows that a NULL pointer is being used
                                                                         // this technique thus ensures that the following code is not removed/optimised away by GCC and remains portable for any vector table location
        #if !defined _MINIMUM_IRQ_INITIALISATION
    ptrVect->ptrNMI           = irq_NMI;
    ptrVect->ptrHardFault     = irq_hard_fault;
    ptrVect->ptrMemManagement = irq_memory_man;
    ptrVect->ptrBusFault      = irq_bus_fault;
    ptrVect->ptrUsageFault    = irq_usage_fault;
    ptrVect->ptrDebugMonitor  = irq_debug_monitor;
    ptrVect->ptrSysTick       = irq_default;
            #if defined RUN_IN_FREE_RTOS
    ptrVect->ptrPendSV        = xPortPendSVHandler;                      // FreeRTOS's PendSV handler
    ptrVect->ptrSVCall        = vPortSVCHandler;                         // FreeRTOS's SCV handler
    ptrVect->reset_vect.ptrResetSP = (void *)(RAM_START_ADDRESS + (SIZE_OF_RAM - NON_INITIALISED_RAM_SIZE)); // the stack pointer value will be taken from the vector base area so enter it in SRAM
            #else
    ptrVect->ptrPendSV        = irq_pend_sv;
    ptrVect->ptrSVCall        = irq_SVCall;
            #endif
    processor_ints = (void (**)(void))&ptrVect->processor_interrupts;    // fill all processor specific interrupts with a default handler
    do {
        *processor_ints = irq_default;
        if (processor_ints == (void (**)(void))&ptrVect->processor_interrupts.LAST_PROCESSOR_IRQ) {
            break;
        }
        processor_ints++;
    } FOREVER_LOOP();
        #else
            #if defined NMI_IN_FLASH
    ptrVect->ptrNMI           = irq_NMI;
            #else
    ptrVect->ptrNMI           = irq_default;
            #endif
    ptrVect->ptrHardFault     = irq_default;
    ptrVect->ptrMemManagement = irq_default;
    ptrVect->ptrBusFault      = irq_default;
    ptrVect->ptrUsageFault    = irq_default;
    ptrVect->ptrDebugMonitor  = irq_default;
    ptrVect->ptrPendSV        = irq_default;
    ptrVect->ptrSVCall        = irq_default;
        #endif
    #endif
    #if defined USE_SDRAM_
    fnConfigureSDRAM();                                                  // configure SDRAM
    #endif
    #if defined _WINDOWS && !defined INTERRUPT_VECTORS_IN_FLASH          // check that the size of the interrupt vectors has not grown beyond that what is expected (increase its space in the script file if necessary!!)
    if (VECTOR_SIZE > CHECK_VECTOR_SIZE) {
        _EXCEPTION("Check the vector table size setting!!");
    }
        #if defined USE_SECTION_PROGRAMMING
    memset(ucFlexRam, 0xff, sizeof(ucFlexRam));                          // when used as data flash the flex ram is initialised to all 0xff
        #endif
    #endif
    CPACR = (CPACR_CP10_FULL_ACCESS | CPACR_CP11_FULL_ACCESS);           // enable access to FPU
    #if defined _WINDOWS
    fnUpdateOperatingDetails();                                          // update operating details to be displayed in the simulator
    #endif
    #if defined SET_POWER_MODE
    SET_POWER_MODE();
    #endif
    #if !defined iMX_BOOTLOADER                                          // WDOG_CS_INT must be set in WDOG3_CS
    fnEnterInterrupt(irq_RTWDOG_ID, 0, wdog3_irq);                       // enter WDOG3 interrupt (highest priority)
    #endif
    #if defined DWT_CYCCNT && defined USE_CORTEX_CYCLE_COUNTER
    DEMCR |= DHCSR_TRCENA;                                               // enable trace for DWT features
        #if defined ARM_MATH_CM7
    DWT_LAR = DWT_LAR_UNLOCK;                                            // unlock access to DWT registers
        #endif
    DWT_CYCCNT = 0;                                                      // reset the cycle count value
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;                                      // enable the cycle counter
    #endif
#endif
#if defined RUN_LOOPS_IN_RAM
    fnInitDriver();                                                      // initialise driver code in SRAM (must be first)
    #if defined USE_IP && ((!defined IP_RX_CHECKSUM_OFFLOAD && !defined IP_TX_CHECKSUM_OFFLOAD && !defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD) || defined _WINDOWS)
    fnInitIP();                                                          // initialise IP routines to run from SRAM
    #endif
#endif
}

#if defined iMX_BOOTLOADER || defined iMX_SERIAL_LOADER
    #if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)
extern void fniMXHeaderToLittleEndian(iMX_BOOT_HEADER *file_header)
{
    unsigned short usShort;
    unsigned long  ulLong;

    usShort = (file_header->usCRC >> 8);
    file_header->usCRC <<= 8;
    file_header->usCRC |= usShort;

    usShort = (file_header->usMagicNumber >> 8);
    file_header->usMagicNumber <<= 8;
    file_header->usMagicNumber |= usShort;

    ulLong = (file_header->ulCodeLength >> 24);
    ulLong |= (((file_header->ulCodeLength >> 16) & 0xff) << 8);
    ulLong |= (((file_header->ulCodeLength >> 8) & 0xff) << 16);
    ulLong |= (((file_header->ulCodeLength) & 0xff) << 24);
    file_header->ulCodeLength = ulLong;

        #if defined _ENCRYPTED
    usShort = (file_header->usRAWCRC >> 8);
    file_header->usRAWCRC <<= 8;
    file_header->usRAWCRC |= usShort;
        #endif
}
    #endif

extern unsigned long fnCheckValidApplication(iMX_BOOT_HEADER *ptrApplicationHeader, const unsigned char *ptrCode)
{
    if ((ptrApplicationHeader->usMagicNumber & ~(BOOT_LOADER_TYPE_MASK)) == PROJECT_APPLICATION_MAGIC_NUMBER) { // check the correct magic number of the application (whichever format it is in)
        unsigned long ulMaximumCodeSize;
        switch (ptrApplicationHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK) {
    #if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
        case BOOT_LOADER_TYPE_PLAIN_RAM_EXECUTION:                       // code that executes from ITC
    #endif
        case BOOT_LOADER_TYPE_AES256_RAM_EXECUTION:                      // code that executes from ITC, after being decrypted
            ulMaximumCodeSize = MAXIMUM_ITC_CODE_SIZE;                   // maximum code size that fits into instruction memory
            break;
        default:                                                         // XiP code (executes directly from spi flash) or SDRAM (it is assumed that the SDRAM can handle the maximum code size that fits into QSPI flash)
            ulMaximumCodeSize = (FLEXSPI_FLASH_SIZE - (BM_FLASH_AREA_SIZE + SERIAL_LOADER_FLASH_AREA_SIZE + iMX_FILE_OBJECT_SIZE)); // maximum code size that fits into flash
            break;
        }
        if ((ptrApplicationHeader->ulCodeLength != 0) && (ptrApplicationHeader->ulCodeLength < ulMaximumCodeSize)) { // if the program size is within the acceptable range
    #if !defined iMX_BOOTLOADER
            #define CRC_BLOCK_LENGTH    (4 * 1024)
            unsigned char ucBuffer[CRC_BLOCK_LENGTH];                    // temporary buffer
    #else
            #define CRC_BLOCK_LENGTH    0xffff
    #endif
            static const unsigned char ucKey[] = APPLICATION_AUTHENTICATION_KEY; // the project's secret authentication key
            unsigned long ulLength = ptrApplicationHeader->ulCodeLength;
            unsigned short usThisBlock;
            unsigned short usCRC = 0;
            while (ulLength > 0) {                                       // perform a check sum over the raw content
                if (ulLength > CRC_BLOCK_LENGTH) {
                    usThisBlock = CRC_BLOCK_LENGTH;                      // maximum CRC block size
                }
                else {
                    usThisBlock = (unsigned short)ulLength;              // final block size
                }
                fnRetriggerWatchdog();
    #if !defined iMX_BOOTLOADER
                fnGetParsFile((unsigned char *)ptrCode, ucBuffer, usThisBlock); // copy to the buffer
                usCRC = fnCRC16(usCRC, (unsigned char *)ucBuffer, usThisBlock); // perform CRC of buffer content
    #else
                usCRC = fnCRC16(usCRC, (unsigned char *)ptrCode, usThisBlock); // perform CRC of buffer content
    #endif
                ulLength -= usThisBlock;
                ptrCode += usThisBlock;
            }
            usCRC = fnCRC16(usCRC, (unsigned char *)ucKey, sizeof(ucKey)); // add the secret key
            if (ptrApplicationHeader->usCRC == usCRC) {                  // verify that the application image is valid
                return (ptrApplicationHeader->ulCodeLength);             // application image is valid so return its length
            }
        }
    }
    return 0;                                                            // no valid application present
}
#endif


#if !defined FREERTOS_NOT_COMPILED && !defined iMX_BOOTLOADER /*defined RUN_IN_FREE_RTOS || defined _WINDOWS*/ // to satisfy FreeRTOS callbacks - even when FreeRTOS not linked
extern void *pvPortMalloc(int iSize)
{
    return uMalloc((MAX_MALLOC)iSize);                                   // use uMalloc() which assumes that memory is never freed
}
extern void vPortFree(void *free)
{
    _EXCEPTION("Unexpected free call!!");
}
extern void vApplicationStackOverflowHook(void)
{
}
extern void vApplicationTickHook(void)                                   // FreeRTOS tick interrupt
{
}
extern void vMainConfigureTimerForRunTimeStats(void)
{
}
extern unsigned long ulMainGetRunTimeCounterValue(void)
{
    return uTaskerSystemTick;
}
extern void vApplicationIdleHook(void)
{
}
extern void prvSetupHardware(void)
{
}
#endif


// The initial stack pointer and PC value - this is usually linked at address 0x60002000 in QSPI flash (or to application start location when working with a boot loader)
//
#if defined _COMPILE_IAR
__root const _RESET_VECTOR __vector_table @ ".intvec"                    // __root forces the function to be linked in IAR project
#elif defined _GNU
const _RESET_VECTOR __attribute__((section(".vectors"))) __vector_table
#elif defined _COMPILE_KEIL
__attribute__((section("VECT"))) const _RESET_VECTOR __vector_table
#else
const _RESET_VECTOR __vector_table
#endif
= {
    (void *)(RAM_START_ADDRESS + (SIZE_OF_RAM - _NON_INITIALISED_RAM_SIZE)), // stack pointer to top of data RAM - leaving space for persistent variables
    (void (*)(void))START_CODE,                                          // start address that is immediately junped to
#if defined _APPLICATION_VALIDATION
    {0x87654321, 0xffffffff},                                            // signal that this application supports validation
    {0xffffffff, 0xffffffff},                                            // overwrite first location with 0x55aa33cc to validate the application
    #endif
#endif
};

#if !defined _BM_BUILD && !(defined _APPLICATION_VALIDATION && defined _GNU)
// FlexSPI NOR configuration block (located at start of the FlexSPI Flash and read as first action by the ROM bootloader)
//
#if defined _COMPILE_IAR
__root const FLEXSPI_NOR_BOOT_CONFIGURATION __boot_config @ ".boot_hdr.conf" // __root forces the function to be linked in IAR project
#elif defined _GNU
const FLEXSPI_NOR_BOOT_CONFIGURATION __attribute__((section(".boot_hdr.conf"))) __boot_config
#elif defined _COMPILE_KEIL
__attribute__((section("_CONFIG"))) const FLEXSPI_NOR_BOOT_CONFIGURATION __boot_config
#else
const FLEXSPI_NOR_BOOT_CONFIGURATION __boot_config
#endif
= {
#if defined iMX_RT1011
    {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 256 bytes reserved for OTFAD keyblob
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    },
    {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, // 768 bytes
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    },
#endif
    {
        FLEXSPI_CFG_BLK_TAG,                                             // [0x000-0x003] tag, fixed value 0x42464346UL
        FLEXSPI_CFG_BLK_VERSION,                                         // [0x004-0x007] version,[31:24] -'V', [23:16] - major, [15:8] - minor, [7:0] - bugfix
        0,                                                               // [0x008-0x00b] reserved for future use
#if defined BOOT_HYPER_FLASH
        kFlexSPIReadSampleClk_ExternalInputFromDqsPad,                   // [0x00c-0x00c] read sample clock source, valid value: 0/1/3
        3,                                                               // [0x00d-0x00d] CS hold time, default value: 3
        3,                                                               // [0x00e-0x00e] CS setup time, default value: 3
        3,                                                               // [0x00f-0x00f] column address width, for HyperBus protocol, it is fixed to 3, for serial NAND refer to datasheet
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        kFlexSPIReadSampleClk_ExternalInputFromDqsPad,                   // [0x00c-0x00c] read sample clock source, valid value: 0/1/3
        3,                                                               // [0x00d-0x00d] CS hold time, default value: 3
        3,                                                               // [0x00e-0x00e] CS setup time, default value: 3
        0,                                                               // [0x00f-0x00f] column address width, for HyperBus protocol, it is fixed to 3, for serial NAND refer to datasheet
#elif defined TEENSY_4_0 || defined TEENSY_4_1
    #if defined APPLICATION_REQUIRES_GPIO_SD_B1_05
        kFlexSPIReadSampleClk_LoopbackInternally,                        // [0x00c-0x00c] read sample clock source, valid value: 0/1/3
    #else
        kFlexSPIReadSampleClk_LoopbackFromDqsPad
    #endif
        1,                                                               // [0x00d-0x00d] CS hold time, default value: 3
        2,                                                               // [0x00e-0x00e] CS setup time, default value: 3
        0,                                                               // [0x00f-0x00f] column address width, for HyperBus protocol, it is fixed to 3, for serial NAND refer to datasheet
#else
    #if defined APPLICATION_REQUIRES_GPIO_SD_B1_05 || defined APPLICATION_REQUIRES_GPIO_SD_12
        kFlexSPIReadSampleClk_LoopbackInternally,                        // [0x00c-0x00c] read sample clock source, valid value: 0/1/3
    #else
        kFlexSPIReadSampleClk_LoopbackFromDqsPad,
    #endif
        3,                                                               // [0x00d-0x00d] CS hold time, default value: 3
        3,                                                               // [0x00e-0x00e] CS setup time, default value: 3
        0,                                                               // [0x00f-0x00f] column address width, for HyperBus protocol, it is fixed to 3, for serial NAND refer to datasheet
#endif
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        1,                                                               // [0x010-0x010] device mode configure enable flag, 1 - Enable, 0 - Disable
#else
        0,                                                               // [0x010-0x010] device mode configure enable flag, 1 - Enable, 0 - Disable
#endif
#if defined iMX_RT101X
        0,                                                               // [0x011-0x011] reserved
#else
        0,                                                               // [0x011-0x011] specify the configuration command type: Quad Enable, DPI/QPI/OPI switch, generic configuration, etc.
#endif
        0,                                                               // [0x012-0x013] wait time for all configuration commands, unit: 100us, Used for DPI/QPI/OPI switch or reset command
        {                                                                // [0x014-0x017] device mode sequence info, [7:0] - LUT sequence id, [15:8] - LUT sequence number, [31:16] Reserved
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
            // Sequence for changing device mode. In this sequence we write to status/control regs 2-3.
            // This will switch EcoXiP to Octal-DDR mode and modify the number of dummy cycles used by it.
            //
            1,                                                           // sequence number, valid number: 1-16
            14,                                                          // sequence index, valid number: 0-15
#else
            0,                                                           // sequence number, valid number: 1-16
            0,                                                           // sequence index, valid number: 0-15
#endif
            {0,0},                                                       // reserved
        },
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        (0x88 | (CTRL_REG_BYTE3_VAL << 8)),                              // [0x018-0x01b] argument/parameter for device configuration (values to be written to status/control regs 2-3)
#else
        0,                                                               // [0x018-0x01b] argument/parameter for device configuration
#endif
        0,                                                               // [0x01c-0x01c] configure command enable flag, 1 - Enable, 0 - Disable
#if defined iMX_RT101X
        {0,0,0},                                                         // [0x01d-0x01f] reserved
#else
        {0,0,0},                                                         // [0x01d-0x01f] configure mode type, similar to deviceModeTpe
#endif
        {                                                                // [0x020-0x02b] sequence info for device configuration command, similar to deviceModeSeq
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
        },
        0,                                                               // [0x02c-0x02f] reserved for future use
        {0,0,0},                                                         // [0x030-0x03b] arguments/parameters for device configuration commands
        0,                                                               // [0x03c-0x03f] reserved for future use
#if defined BOOT_HYPER_FLASH                                             // for S26KS512SDPBHI02 on MIMXRT1050
        (1 << kFlexSpiMiscOffset_DdrModeEnable) | (1 << kFlexSpiMiscOffset_WordAddressableEnable) | (1 << kFlexSpiMiscOffset_SafeConfigFreqEnable) | (1 << kFlexSpiMiscOffset_DiffClkEnable), // [0x040-0x043] controller misc options, see misc feature bit definitions for more details
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART           // adesto EcoXiP ATXP032
        (1 << kFlexSpiMiscOffset_DdrModeEnable) | (1 << kFlexSpiMiscOffset_SafeConfigFreqEnable), // [0x040-0x043] controller misc options, see misc feature bit definitions for more details
#else
        0,                                                               // [0x040-0x043] controller misc options, see misc feature bit definitions for more details
#endif
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART             // adesto EcoXiP ATXP032
        kSerialNOR,                                                      // [0x044-0x044] device Type: see flash type definition for more details
#else
        0,                                                               // [0x044-0x044] device Type: see flash type definition for more details
#endif
#if defined BOOT_HYPER_FLASH || defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        kSerialFlash_8Pads,                                              // [0x045-0x045] serial flash pad type: 1 - single, 2 - dual, 4 - quad, 8 - octal
        kFlexSpiSerialClk_133MHz,                                        // [0x046-0x046] serial flash frequency - device specific definitions; see system boot chapter for more details
#elif defined TEENSY_4_0 || defined TEENSY_4_1
        kSerialFlash_4Pads,                                              // [0x045-0x045] serial flash pad type: 1 - single, 2 - dual, 4 - quad, 8 - octal
    #if defined APPLICATION_REQUIRES_GPIO_SD_B1_05
        kFlexSpiSerialClk_60MHz,                                         // [0x046-0x046] serial flash frequency - device specific definitions; see system boot chapter for more details
    #else
        kFlexSpiSerialClk_133MHz,                                        // [0x046-0x046] serial flash frequency - device specific definitions; see system boot chapter for more details
    #endif
#else
        kSerialFlash_4Pads,                                              // [0x045-0x045] serial flash pad type: 1 - single, 2 - dual, 4 - quad, 8 - octal
    #if defined APPLICATION_REQUIRES_GPIO_SD_B1_05 || defined APPLICATION_REQUIRES_GPIO_SD_12
        kFlexSpiSerialClk_60MHz,                                         // [0x046-0x046] serial flash frequency - device specific definitions; see system boot chapter for more details
    #else
        kFlexSpiSerialClk_133MHz,                                        // [0x046-0x046] serial flash frequency - device specific definitions; see system boot chapter for more details
    #endif
#endif
        0,                                                               // [0x047-0x047] LUT customization enable, it is required if the program/erase cannot be performed using 1 LUT sequence, currently only applicable to HyperFLASH
        {0,0},                                                           // [0x048-0x04f] reserved for future use
#if defined iMX_RT1024 || defined iMX_RT1064
        (4 * 1024 * 1024),                                               // [0x050-0x053] size of flash connected to A1 -internal QSPI flash
#else
        FLEXSPI_FLASH_SIZE,                                              // [0x050-0x053] size of flash connected to A1
#endif
        0,                                                               // [0x054-0x057] size of flash connected to A2
        0,                                                               // [0x058-0x05b] size of flash connected to B1
        0,                                                               // [0x05c-0x05f] size of flash connected to B2
        0,                                                               // [0x060-0x063] CS pad setting override value
        0,                                                               // [0x064-0x067] SCK pad setting override value
        0,                                                               // [0x068-0x06b] data pad setting override value
        0,                                                               // [0x06c-0x06f] DQS pad setting override value
        0,                                                               // [0x070-0x073] timeout threshold for read status command
        0,                                                               // [0x074-0x077] CS deselect interval between two commands
#if defined BOOT_HYPER_FLASH
        { 16, 16 },                                                      // [0x078-0x07b] CLK edge to data valid time for PORT A and PORT B, in terms of 0.1ns
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        { 20, 0},                                                        // [0x078-0x07b] CLK edge to data valid time for PORT A and PORT B, in terms of 0.1ns
#else
        { 0, 0 },                                                        // [0x078-0x07b] CLK edge to data valid time for PORT A and PORT B, in terms of 0.1ns
#endif
        0,                                                               // [0x07c-0x07d] busy offset, valid value: 0-31
        0,                                                               // [0x07e-0x07f] busy flag polarity, 0 - busy flag is 1 when flash device is busy, 1 - busy flag is 0 when flash device is busy
        {                                                                // [0x080-0x17f] lookup table holds flash command sequences [64]
#if defined BOOT_HYPER_FLASH
            // Read LUT
            //
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_DDR, FLEXSPI_8PAD, (HYPTER_PROTOCOL_CA0_READ | HYPTER_PROTOCOL_CA0_LINEAR_BURST), FLEXSPI_Command_RADDR_DDR, FLEXSPI_8PAD, ADDRESS_LENGTH_24BITS),
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_CADDR_DDR, FLEXSPI_8PAD, ADDRESS_LENGTH_16BITS, FLEXSPI_Command_DUMMY_DDR, FLEXSPI_8PAD, 0x06),
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_READ_DDR, FLEXSPI_8PAD, 0x04, FLEXSPI_Command_STOP, FLEXSPI_1PAD, 0x0),
            0,
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
            // Read
            //
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_SDR, FLEXSPI_8PAD, FAST_READ, FLEXSPI_Command_RADDR_DDR, FLEXSPI_8PAD, ADDRESS_LENGTH_32BITS),
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_DUMMY_DDR, FLEXSPI_8PAD,(ECOXIP_READ_NON_SPI_DUMMY_CYCLES * 2 + 1), FLEXSPI_Command_READ_DDR, FLEXSPI_8PAD, 0x80),
            0,0,
            // Read Status
            //
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_SDR, FLEXSPI_8PAD, READ_STATUS_REGISTER_1, FLEXSPI_Command_DUMMY_DDR, FLEXSPI_8PAD, 0x08),
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_READ_DDR, FLEXSPI_8PAD, 0x01, FLEXSPI_Command_STOP, FLEXSPI_1PAD, 0x0),
            0,0,
            0,0,0,0,
            // Write Enable
            //
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_SDR, FLEXSPI_1PAD, WRITE_ENABLE, FLEXSPI_Command_STOP, FLEXSPI_1PAD, 0x0),
            0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            // Write Status/Control Registers (this specific sequence will write 2 bytes to status/control regs 2-3)
            //
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_SDR, FLEXSPI_1PAD, WRITE_STATUS_CONTROL_REGISTERS, FLEXSPI_Command_SDR, FLEXSPI_1PAD, 0x02),
            FLEXSPI_LUT_SEQ(FLEXSPI_Command_WRITE_SDR, FLEXSPI_1PAD, 0x02, FLEXSPI_Command_STOP, FLEXSPI_1PAD, 0x0),
            0,0,
            0,0,0,0,
#elif defined TEENSY_4_0 || defined TEENSY_4_1
            // Read LUT
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ_QUAD_I_O) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
            FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 0x06) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
            0,0,
            // Read status register
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, READ_STATUS_REGISTER_1) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
            0,0,0,

            0,0,0,0,
            
            // Write enable
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, WRITE_ENABLE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
            0,0,0,

            0,0,0,0,

            // Sub-sector erase
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, SUB_SECTOR_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
            0,0,0,

            0,0,0,0,

            0,0,0,0,

            // Block erase
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, BLOCK_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
            0,0,0,

            // Page program
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, PAGE_PROG) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_WRITE_SDR, 0x04) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
            0,0,

            0,0,0,0,

            // Chip erase
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, CHIP_ERASE) | FLEXSPI_LUT_1PAD_SEQ_2(FLEXSPI_Command_STOP, 0x00),
            0,0,0,

            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
            0,0,0,0,
#else
            // Read LUT
            //
            FLEXSPI_LUT_1PAD_SEQ_1(FLEXSPI_Command_SDR, FAST_READ_QUAD_I_O) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_RADDR_SDR, ADDRESS_LENGTH_24BITS),
            FLEXSPI_LUT_4PAD_SEQ_1(FLEXSPI_Command_DUMMY_SDR, 0x06) | FLEXSPI_LUT_4PAD_SEQ_2(FLEXSPI_Command_READ_SDR, 0x04),
            0,0,
            0,
#endif
        },
        {                                                                // [0x180-0x1af] customisable LUT sequences [12]
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
            {
                0,                                                       // sequence number, valid number: 1-16
                0,                                                       // sequence index, valid number: 0-15
                {0,0},                                                   // reserved
            },
        },
        {0,0,0,0},                                                       // [0x1b0-0x1bf] reserved for future use
    },
    SPI_FLASH_PAGE_LENGTH,                                               // [0x1c0] page size of serial NOR
#if defined iMX_RT1024 || defined iMX_RT1064
    (4 * 1024),                                                          // [0x1c4] sector size of Serial NOR (internal QSPI flash)
#elif defined BOOT_HYPER_FLASH
    (256 * 1024),                                                        // [0x1c4] sector size of Serial NOR
#else
    SPI_FLASH_SUB_SECTOR_LENGTH,                                         // [0x1c4] sector size of Serial NOR
#endif
#if defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART || defined TEENSY_4_0 || defined TEENSY_4_1
    1,                                                                   // [0x1c8] clock frequency for IP command
#else
    0,                                                                   // [0x1c8] clock frequency for IP command
#endif
#if defined iMX_RT101X
    {0},                                                                 // reserved for future use
#else
    #if defined BOOT_HYPER_FLASH || defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
    1,                                                                   // [0x1c9] sector/block size is the same (uniform block size)
    #else
    0,                                                                   // [0x1c9] sector/block size is the same (non-uniform block size)
    #endif
    {0,0},                                                               // [0x1ca-0x1cb] reserved for future use
    0,                                                                   // [0x1cc] serial NOR flash type: 0/1/2/3
    0,                                                                   // [0x1cd] need to exit NoCmd mode before other IP command
    0,                                                                   // [0x1ce] half the serial clock for non-read command: true/false
    0,                                                                   // [0x1cf] need to restore NoCmd mode after IP commmand execution
#if defined BOOT_HYPER_FLASH
    (256 * 1024),                                                        // [0x1d0-0x1d3] block size
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
    SPI_FLASH_SUB_SECTOR_LENGTH,                                         // [0x1d0-0x1d3] block size
#elif defined TEENSY_4_0 || defined TEENSY_4_1
    SPI_FLASH_SECTOR_LENGTH,
#elif defined iMX_RT1024 || defined iMX_RT1064
    (256 * 1024),                                                        // [0x1d0-0x1d3] block size
#else
    SPI_FLASH_BLOCK_LENGTH,                                              // [0x1d0-0x1d3] block size
#endif
    {0},                                                                 // [0x1d4-0x1ff] reserved for future use
#endif
};


#if defined _COMPILE_IAR
__root const BOOT_DATA __boot_data @ ".boot_hdr.boot_data"               // __root forces the function to be linked in IAR project
#elif defined _GNU
const BOOT_DATA __attribute__((section(".boot_hdr.boot_data"))) __boot_data
#elif defined _COMPILE_KEIL
__attribute__((section("_BOOT_DATA"))) const BOOT_DATA __boot_data
#else
const BOOT_DATA __boot_data
#endif
= {
#if defined iMX_RT1024 || defined iMX_RT1064
    FLEXSPI2_FLASH_BASE,                                                 // boot start location in internal flash
    FLEXSPI2_FLASH_SIZE,                                                 // size of internal flash
#else
    FLEXSPI_FLASH_BASE,                                                  // boot start location
    FLEXSPI_FLASH_SIZE,                                                  // size
#endif
    PLUGIN_FLAG,                                                         // plugin flag
    0xffffffff                                                           // empty - extra data word
};


// Image Vector Table (located at 0x1000 offset in the FlexSPI Flash)
//
#if defined _COMPILE_IAR
__root const IMAGE_VECTOR_TABLE __image_vector_table @ ".boot_hdr.ivt" // __root forces the function to be linked in IAR project
#elif defined _GNU
const IMAGE_VECTOR_TABLE __attribute__((section(".boot_hdr.ivt"))) __image_vector_table
#elif defined _COMPILE_KEIL
__attribute__((section("_IVT"))) const IMAGE_VECTOR_TABLE __image_vector_table
#else
const IMAGE_VECTOR_TABLE __image_vector_table
#endif
= {
    IVT_HEADER,                                                          // IVT Header
    (unsigned long)&__vector_table,                                      // image entry function
    0,                                                                   // reserved = 0
    #if defined GROUP1_ENTRIES                                           // if a DCD table is defined in app_hw_iMX.h
    (unsigned long)&__dcd_data,                                          // address where DCD information is stored
    #else
    0,                                                                   // don't use DCD
    #endif
    (unsigned long)&__boot_data,                                         // address where BOOT data structure is stored
    (unsigned long)&__image_vector_table,                                // pointer to IVT self (absolute address - used internally by the boot ROM)
    (unsigned long)0,                                                    // address where CSF file is stored (0 when not performing secure boot)
    0                                                                    // reserved = 0
};

#endif
