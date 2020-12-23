/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:        STR91XF.c
   Project:     Single Chip Embedded Internet
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2019
   *********************************************************************/

#if defined _STR91XF

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#ifdef _WINDOWS
    #include "config.h"
    #define INITHW  extern
    extern void fnOpenDefaultHostAdapter(void);
    #define _SIM_START_BIT     *ulReg |= IIC_START_BIT;
    #define CLEAR_ALL_INTS     0x00000000
#else
    #define OPSYS_CONFIG                                                 // this module owns the operating system configuration
    #define INITHW  static
    #define _SIM_START_BIT
    #include "config.h"
    #define CLEAR_ALL_INTS     0xffffffff
    #if defined (_GNU)
        extern void __str_disable_interrupt(void);
        extern void __str_enable_interrupt(void);
    #endif
#endif



/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#ifdef COMPILE_IAR                                                       // interrupt vector configuration as required by IAR compiler
    #pragma language=extended
    #define __interrupt
#else
    #define __interrupt
#endif

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */


/* =================================================================== */
/*                global function prototype declarations               */
/* =================================================================== */


/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

#if !defined SPI_INTERFACE && defined SPI_FILE_SYSTEM
    static void fnConfigSPIFileSystem(unsigned char ucSpeed);
#endif

#ifdef GLOBAL_HARDWARE_TIMER
    static __interrupt void hwtimer_interrupt(void);
#endif
static void fnProtect(volatile unsigned short *usSectorPointer, unsigned short usCommand);

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#ifdef RANDOM_NUMBER_GENERATOR
    unsigned short *ptrSeed;
#endif

static int iInterruptLevel = 0;

#ifdef ETH_INTERFACE
    static unsigned long ulRxEthernetBufferSize;
#endif

#ifdef _WINDOWS
    static unsigned short usPhyMode = 0;
#endif

#ifdef SPI_FILE_SYSTEM
    static unsigned char ucEEPROMDanger = 0;
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */
#ifdef ETH_INTERFACE
    extern ETHERNETQue *eth_tx_control;                                  // Ethernet structure pointers for Ethernet Interrupt handling
    extern ETHERNETQue *eth_rx_control;
#endif
#ifdef IIC_INTERFACE
    extern IICQue *IIC_rx_control[NUMBER_IIC];
    extern IICQue *IIC_tx_control[NUMBER_IIC];
#endif

/* =================================================================== */
/*                      local function definitions                     */
/* =================================================================== */

#ifndef _WINDOWS
    static void fnInitHW(void);
#endif


/* =================================================================== */
/*                      global function definitions                    */
/* =================================================================== */


/* =================================================================== */
/*                      STARTUP CODE                                   */
/* =================================================================== */



#ifndef _WINDOWS
extern void __segment_init(void);
    #if defined (COMPILE_IAR)
        #pragma segment="DATA16_Z"
        #define HEAP_START_ADDRESS __segment_end("DATA16_Z")             // IAR compiler - last location of static variables
    #elif defined _COMPILE_KEIL
        #define HEAP_START_ADDRESS    _keil_ram_size()
        #define __disable_interrupt() __disable_irq()                    // KEIL intrinsics
        #define __enable_interrupt()  __enable_irq()
    #else
  //extern unsigned char _end;
  //#define HEAP_START_ADDRESS &_end                                     // GNU last location of static variables
    extern unsigned char __heap_end__;
    #define HEAP_START_ADDRESS &__heap_end__                             // GNU last location of static variables (with library malloc)
    #endif

#ifdef _COMPILE_KEIL
typedef struct stREGION_TABLE
{
    unsigned char *ptrConst;                                             // const variables belonging to block
    unsigned char *ptrDestination;                                       // destination in RAM
    unsigned long  ulLength;                                             // length of the block in SRAM
    unsigned long  ulUnknown;                                            // entry unknown
} REGION_TABLE;

// Calculate the end of used SRAM from the Keil linker information
//
static unsigned char *_keil_ram_size(void)
{
    extern REGION_TABLE Region$$Table$$Base;                            // table location supplied by linker
    extern REGION_TABLE Region$$Table$$Limit;                           // end of table list
    REGION_TABLE *ptrEntries = &Region$$Table$$Base;                    // first block
    unsigned char *ptrRam = ptrEntries->ptrDestination;                 // RAM address
    do {
        ptrRam += ptrEntries->ulLength;                                 // add length
        ptrEntries++;                                                   // move to next block
    } while (ptrEntries != &Region$$Table$$Limit);
    return ptrRam;
}
#endif

// main entry for the target code
//
extern int main(void)
{
#ifdef MULTISTART
    MULTISTART_TABLE *ptrInfo;
    unsigned char *pucHeapStart;
#endif
#ifdef RANDOM_NUMBER_GENERATOR
    unsigned short usRandomSeed;                                         // we put an uninitialised variable on to the stack for use as a random seed
    ptrSeed = &usRandomSeed;
#endif

#ifdef _BOOT_PROGRAM
    __segment_init();
#endif				
    fnInitHW();                                                          // perform hardware initialisation (note that we do not have heap yet)   
#ifdef MULTISTART
    ptrInfo = ptMultiStartTable;                                         // if the user has already set to alternative start configuration
    if (ptrInfo == 0) {                                                  // no special start required
_abort_multi:
        fnInitialiseHeap(ctOurHeap, HEAP_START_ADDRESS);                 // initialise heap     
        uTaskerStart((UTASKTABLEINIT *)ctTaskTable, ctNodes, PHYSICAL_QUEUES); // start the operating system
        while ((ptrInfo = (MULTISTART_TABLE*)uTaskerSchedule()) == 0) {} // schedule uTasker
    }

    do {
        pucHeapStart = HEAP_START_ADDRESS;
        if (ptrInfo->new_hw_init) {                                      // info to next task configuration available
    #ifdef DYNAMIC_MULTISTART
            pucHeapStart = ptrInfo->new_hw_init(JumpTable, (void **)&ptrInfo, OurConfigNr); // get heap details from next configuration
    #else
            pucHeapStart = ptrInfo->new_hw_init(JumpTable);              // get heap details from next configuration
    #endif
            if (!pucHeapStart) {
                goto _abort_multi;                                       // this can happen if the jump table version doesn't match - prefer to stay in boot mode than start an application which will crash
            }
        }

        fnInitialiseHeap(ptrInfo->ptHeapNeed, pucHeapStart);             // initialise the new heap
                                                                         // start the operating system with next configuration
        uTaskerStart((UTASKTABLEINIT *)ptrInfo->ptTaskTable, ptrInfo->ptNodesTable, PHYSICAL_QUEUES);

        while ((ptrInfo = (MULTISTART_TABLE*)uTaskerSchedule()) == 0) {} // schedule uTasker

    } while (1);
#else
    fnInitialiseHeap(ctOurHeap, HEAP_START_ADDRESS);                     // initialise heap
    uTaskerStart((UTASKTABLEINIT *)ctTaskTable, ctNodes, PHYSICAL_QUEUES); // start the operating system

    while (1) {
        uTaskerSchedule();                                               // schedule uTasker
    }
#endif

  return 0;                                                              // we never return....
}
#endif


#ifdef RANDOM_NUMBER_GENERATOR
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

#if OFFSET_BANK1 == 0
static void fnConfigFlash(unsigned short usConfig)
{
    *(unsigned short *)0 = CUI_PROT_LEVEL1_CONFIG_SETUP;
    *(unsigned short *)((usConfig << 2)) = CUI_WRITE_FLASH_CONF_CONFIRM;
}
#endif

// Configure FLASH by writing into bank 1 space. This is only called during initialisation and so isn't protected against interrupts
//
static void fnSetFMI(unsigned short usConfig)
{
#ifdef _WINDOWS
    #define PROG_WORD_SIZE_3 300
#else
    #define PROG_WORD_SIZE_3 30                                          // adequate space for the small program
#endif
#if OFFSET_BANK1 == 0
    unsigned short usProgSpace[PROG_WORD_SIZE_3];                        // make temporary space for the routine on stack (this will have a word boundary)
    void (*fnRAM_code)(unsigned short);
    fnRAM_code = (void(*)(unsigned short))(fnCopyProgram((unsigned char *)fnConfigFlash, PROG_WORD_SIZE_3, usProgSpace));
    #ifndef _WINDOWS
    fnRAM_code(usConfig);
    #endif
#else
    #ifndef _WINDOWS
    *(unsigned short *)OFFSET_BANK1 = CUI_PROT_LEVEL1_CONFIG_SETUP;
    *(unsigned short *)((usConfig << 2) + OFFSET_BANK1) = CUI_WRITE_FLASH_CONF_CONFIRM;
    #endif
#endif
}


// Support watchdog re-triggering of specific hardware
//
extern void fnRetriggerWatchdog(void)
{
    RETRIGGER_WATCHDOG();                                                // retrigger the watchdog
                                                                         // flash an LED for visualisation
    TOGGLE_WATCHDOG_LED();                                               // define in project defines whether this is performed and where
}


// Routine to disable interrupts during critical region (IAR uses intrinsic function)
//
extern void uDisable_Interrupt()
{
#ifndef _WINDOWS
    #if defined COMPILE_IAR || defined _COMPILE_KEIL
    __disable_interrupt();                                               // IAR intrinsic routine
    #elif defined (_GNU)
    __str_disable_interrupt();    
    #endif
#endif
    iInterruptLevel++;                                                   // monitor the level of interrupt nesting
}
extern void uEnable_Interrupt()
{
#ifdef _WINDOWS
    if (!iInterruptLevel) {
        *(int *)0 = 0;                                                   // basic error - cause exception
    }
#endif
    if (!(--iInterruptLevel)) {                                          // only when no more interrupt nesting,
#ifndef _WINDOWS
    #if defined COMPILE_IAR || defined _COMPILE_KEIL
    __enable_interrupt();                                                // IAR intrinsic routine
    #elif defined (_GNU)
    __str_enable_interrupt();  
    #endif
#endif
    }
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




/**************************** Real Time Clock interrupt ******************************************/
#ifdef COMPILE_IAR                                                       // disable optimising away the reading of the timer (to reset interrupt)
    #pragma optimize=none
#endif
__interrupt static void RealTimeInterrupt(void)
{
    fnRtmkSystemTick();                                                  // operating system tick
    TIM_SR_TICK_TIMER = 0;                                               // reset interrupt flag
}

// Dummy routine for default Interrupt Service Routine tests
//
__interrupt static void fnISR(void)
{
    int iTest = 10;
    while (iTest--) {}
}


// Routine to initialise the Real Time Tick interrupt
// We use a user definable 16 bit timer - to get accurate value from 1ms to 1s setting
#define REQUIRED_MS ((1000/TICK_RESOLUTION))                             // the TICK period in ms
#define TICK_VALUE ((((PCLK/256 + REQUIRED_MS/2)/REQUIRED_MS)) - 4)      // the divide ratio required

extern void fnStartTick(void)
{

//  SCU_SCR2   = (256-1);                                                // timer clocked by master clock / 256 (don't use external clock and divider due to errata in initial chips)

    TIM_CR1_TICK_TIMER  = PWM;                                           // ensure stopped

    TIM_CR2_TICK_TIMER  = (CLOCK_DIV_256 | OC2IE);                       // interrupt on compare - take clock from PCLK / 256
    TIM_OC2R_TICK_TIMER = TICK_VALUE;                                    // match value
    TIM_OC1R_TICK_TIMER = (TIM_OC2R_3 / 2);
    TIM_SR_TICK_TIMER   = 0;                                             // reset any waiting interrupt (useful after new start without a real reset)

    VIC0_DVAR   = (unsigned long)fnISR;                                  // set default ISR

    INSTALL_TICK_INT(RealTimeInterrupt);                                 // configure interrupt and enter the handling interrupt routine in the vector table

    TIM_CR1_TICK_TIMER  = (COUNTER_ENABLE | PWM | PWMI);                 // set periodic timer, enable timer and its interrupt

#ifdef GLOBAL_HARDWARE_TIMER                                             // initialise the timer for use with global hardware timers
    TIM_CR1_HW_TICK_TIMER  = 0;                                          // disable timer
    TIM_SR_HW_TICK_TIMER   = 0;                                          // Reset interrupt flag

    INSTALL_HW_TICK_INT(hwtimer_interrupt);                              // configure and enter the handling interrupt routine in the vector table
#endif
}



#ifdef ETH_INTERFACE
/* =================================================================== */
/*                          Ethernet Interface                         */
/* =================================================================== */

static const unsigned char EMAC_RX_int_message[ HEADER_LENGTH ] = { 0, 0 , TASK_ETHERNET, INTERRUPT_EVENT, EMAC_RX_INTERRUPT };   // define fixed interrupt event

static STR91XF_BD *ptrRxBd, *ptrTxBd;


// This is called by the Ethernet task to handle an Ethernet event, which is usually a reception frame
//
extern signed char fnEthernetEvent(unsigned char *ucEvent, ETHERNET_FRAME *rx_frame)
{
#ifdef _LAN_REPORT_ACTIVITY
    static const unsigned char EMAC_rx_int_message[ HEADER_LENGTH ] = { 0, 0, INTERRUPT_TASK_LAN_EXCEPTIONS, INTERRUPT_EVENT, EMAC_RX_INTERRUPT };
#endif
    if (EMAC_RX_INTERRUPT == *ucEvent) {
        if (ptrRxBd->bd_packet_status & RX_VALID_MASK) {                 // check the ownership
            return -1;                                                   // belongs to DMA so not ready
        }
        rx_frame->frame_size = (unsigned short)((ptrRxBd->bd_dma_control & RX_FRAME_LENGTH_MASK) - 4); // frame length
        rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)ptrRxBd->bd_dma_start_address; // pointer to frame in memory
#ifdef _LAN_REPORT_ACTIVITY
        fnWrite(INTERNAL_ROUTE, (unsigned char*)EMAC_rx_int_message, HEADER_LENGTH); // this is not sent here since it distrurbs (reason unknown) - it is instead sent in the RX interrupt routine
#endif
        return 0;                                                        // channel received from 0 (fixed value for compatibility with other devices)
    }
#ifdef USE_IP_STATS
    else if (ETHERNET_RX_OVERRUN == *ucEvent) {
        fnIncrementEthernetStats(TOTAL_LOST_RX_FRAMES);                  // we lost a frame due to RX overrun so count the event
    }
    else {
        fnIncrementEthernetStats(TOTAL_OTHER_EVENTS);                    // count other unexpected events
    }
#endif
    return -1;                                                           // invalid channel
}

// PHY Read
//
static unsigned short fnReadMII(unsigned long ulRegAdr)
{
    ENET_MIIA = (READ_FROM_PHY | ulRegAdr);                              // command read from given address

#ifdef _WINDOWS
    if (PHY_IDENTIFIER_1 == ulRegAdr) {                                  // simulate reads from identifier register
        ENET_MIID = (PHY_IDENTIFIER>>16);
    }
    else if (PHY_IDENTIFIER_2 == ulRegAdr) {
        ENET_MIID = (PHY_IDENTIFIER & 0x0000ffff);
    }
    else if (PHY_INTERRUPT_CONTROL_STATUS_REGISTER == ulRegAdr) {
        ENET_MIID = (PHY_LINK_CODE_WORD_IS_RECEIVED | PHY_SPEED_IS_100);
    }
    ENET_MIIA &= ~PHY_BUSY;
#endif

    while (ENET_MIIA & PHY_BUSY) { }                                      // wait until the command has terminated

    return (unsigned short)ENET_MIID;                                     // return the value read
}


// PHY write
    //
static void fnWriteMII(unsigned long ulRegAdr, unsigned short usData)
{
    ENET_MIID = usData;                                                  // prepare the data to be sent
    ENET_MIIA = (WRITE_TO_PHY | ulRegAdr);                               // command write to given address

#ifdef _WINDOWS
    if (PHY_BASIC_CONTROL_REGISTER == ulRegAdr) {
        usPhyMode = usData;
    }
    ENET_MIIA &= ~PHY_BUSY;
#endif

    while (ENET_MIIA & PHY_BUSY) { }                                      // wait until the command has terminated
}


 /**************************** PHY Ethernet interrupt ****************************************************/
// PHY interrupt
//
#ifdef COMPILE_IAR                                                       // disable optimising away reading (to reset interrupt)
 #pragma optimize=2
#endif
__interrupt static void PHY_Interrupt(void)
{
#ifdef INTERRUPT_TASK_PHY
    unsigned char int_phy_message[ HEADER_LENGTH ] = { 0, 0 , INTERRUPT_TASK_PHY, INTERRUPT_EVENT, UNKNOWN_INTERRUPT };    // define standard interrupt event
#endif
    volatile unsigned short usInterrupt;

    RESET_PHY_INT();                                                     // clear interrupt

    usInterrupt = fnReadMII(PHY_INTERRUPT_CONTROL_STATUS_REGISTER);      // read the cause(s) of the interrupt, which resets the bits
#ifdef INTERRUPT_TASK_PHY
    if (PHY_LINK_IS_DOWN & usInterrupt) {
        int_phy_message[MSG_INTERRUPT_EVENT] = LAN_LINK_DOWN;
    }
    else if (PHY_LINK_CODE_WORD_IS_RECEIVED & usInterrupt) {
        if (usInterrupt & PHY_SPEED_IS_100) {
            int_phy_message[MSG_INTERRUPT_EVENT] = LAN_LINK_UP_100;
        }
        else {
            int_phy_message[MSG_INTERRUPT_EVENT] = LAN_LINK_UP_10;
        }
    }

    iInterruptLevel = 1;                                                 // ensure interrupts remain blocked when putting message to queue
    fnWrite(INTERNAL_ROUTE, (unsigned char*)int_phy_message, HEADER_LENGTH); // inform the corresponding task
    iInterruptLevel = 0;                                                 // release
#endif
}


// EMAC interrupt displatcher
//
__interrupt void EMAC_Interrupt(void)
{
    while (ENET_ISR & ENET_IER) {                                        // read the interrupt status register
#ifdef LAN_REPORT_ACTIVITY
        unsigned char EMAC_int_message[ HEADER_LENGTH ] = { 0, 0, INTERRUPT_TASK_LAN_EXCEPTIONS, INTERRUPT_EVENT, EMAC_TX_INTERRUPT };
#endif
        if (ENET_ISR & TX_CURR_DONE_EN) {                                // current frame tramsmitted
            ENET_ISR = TX_CURR_DONE_EN;                                  // reset flag
#ifdef _WINDOWS
            ENET_ISR &= ~TX_CURR_DONE_EN;
#endif
#ifdef LAN_REPORT_ACTIVITY
            if (ptrTxBd->bd_packet_status & (TX_LATE_COLLISION | TX_EXCESSIVE_COLLISIONS)) {  // collision detection
                EMAC_int_message[MSG_INTERRUPT_EVENT] = ETHERNET_COLLISION;
            }
            iInterruptLevel = 1;                                         // ensure interrupts remain blocked when putting message to queue
            fnWrite(INTERNAL_ROUTE, (unsigned char*)EMAC_int_message, HEADER_LENGTH); // inform the task of event
            iInterruptLevel = 0;                                         // release
#endif
            ptrTxBd = (STR91XF_BD *)ptrTxBd->bd_dma_next;                // move to next buffer descriptor
        }

        if (ENET_ISR & RX_ENTRY_EN) {                                    // complete reception frame available
            ENET_ISR = RX_ENTRY_EN;                                      // reset flag
#ifdef _WINDOWS
            ENET_ISR &= ~RX_ENTRY_EN;
#endif
            iInterruptLevel = 1;                                         // ensure interrupts remain blocked when putting message to queue
            fnWrite(INTERNAL_ROUTE, (unsigned char*)EMAC_RX_int_message, HEADER_LENGTH); // inform the Ethernet task
#ifdef LAN_REPORT_ACTIVITY
            EMAC_int_message[MSG_INTERRUPT_EVENT] = EMAC_RX_INTERRUPT;
            fnWrite(INTERNAL_ROUTE, (unsigned char*)EMAC_int_message, HEADER_LENGTH); // inform the task of event
#endif
            iInterruptLevel = 0;                                         // release
        }
    }
}


// Ethernet configuration routine
//
extern void fnConfigEthernet(ETHTABLE *pars)
{
    unsigned long ulPhyIdentifier, ulRegister;
    unsigned short usMIIData;
    volatile int i;
    STR91XF_BD *ptrBd, *ptrFirstTxBd;
    unsigned char *ptrBuffer;

    SCU_PCGR0 |= MAC;                                                    // enable clocks and take MAC out of reset
    SCU_PRR0  |= MAC;
    SCU_CLKCNTR |= PHY_SEL;                                              // generate 25MHz PHY clock

    // set up the processor ports for the Ethernet interface - since it is not variable, it is set here in the code
    // MDIO - dedicated pin
    // MII_RXDV    P0.7  default input function
    // MII_RX_CLK  P0.6  default input function
    // MII_RXD3    P0.5  default input function
    // MII_RXD2    P0.4  default input function
    // MII_RXD1    P0.3  default input function
    // MII_RXD0    P0.2  default input function
    // MII_TX_CLK  P0.0  default input function
    SCU_GPIOOUT0 &= ~((MASK_FUNCTION << (2*7)) | (MASK_FUNCTION << (2*6)) | (MASK_FUNCTION << (2*5)) | (MASK_FUNCTION << (2*4)) | (MASK_FUNCTION << (2*3)) | (MASK_FUNCTION << (2*2)) | (MASK_FUNCTION << (2*0)));
    SCU_GPIOIN0 &= ~(PORT0_BIT7 | PORT0_BIT6 | PORT0_BIT5 | PORT0_BIT4 | PORT0_BIT3 | PORT0_BIT2 | PORT0_BIT0);

    // MDC         P1.7  first output function
    // MII_CRS     P1.6  default input function
    // MII_COL     P1.5  default input function
    // MII_TXD3    P1.4  first output function
    // MII_TXD2    P1.3  first output function
    // MII_TXD1    P1.2  first output function
    // MII_TXD0    P1.1  first output function
    // MII_RX_ER   P1.0  default input function
    SCU_GPIOOUT1 &= ~((MASK_FUNCTION << (2*7)) | (MASK_FUNCTION << (2*6)) | (MASK_FUNCTION << (2*5)) | (MASK_FUNCTION << (2*4)) | (MASK_FUNCTION << (2*3)) | (MASK_FUNCTION << (2*2)) | (MASK_FUNCTION << (2*1)) | (MASK_FUNCTION << (2*0)));
    SCU_GPIOOUT1 |= ((ALT_OUTPUT_2_FUNCTION << (2*7)) | (ALT_OUTPUT_2_FUNCTION << (2*4)) | (ALT_OUTPUT_2_FUNCTION << (2*3)) | (ALT_OUTPUT_2_FUNCTION << (2*2)) | (ALT_OUTPUT_2_FUNCTION << (2*1)));
    SCU_GPIOIN1 &= ~(PORT1_BIT6 | PORT1_BIT5 | PORT1_BIT0);

    // MII_TX_EN   P5.3  first output function
    // MII_PHY_CLK P5.2  first output function
    SCU_GPIOOUT5 &= ~((MASK_FUNCTION << (2*3)) | (MASK_FUNCTION << (2*2)));
    SCU_GPIOOUT5 |= ((ALT_OUTPUT_2_FUNCTION << (2*3)) | (ALT_OUTPUT_2_FUNCTION << (2*2)));



    /********************************* Check and configure PHY **********************************************/

    ENET_SCR &= ~SRESET;                                                 // de-assert reset to allow module to operate
#if PLL_OUTPUT_FREQ > 50000000
    ENET_MCR = (MCR_DCE | MCR_APR | MCR_RVFF | MCR_PRESCALE);
#else
    ENET_MCR = (MCR_DCE | MCR_APR | MCR_RVFF);
#endif

    if (pars->usMode & PROMISCUOUS) {
        ENET_MCR |= RECEIVE_ALL;                                         // no destination address checking
    }
    if (pars->usMode & LOOPBACK) {
        ENET_MCR |= MCR_LM;                                              // enable loopback
    }
    if (pars->usMode & NO_BROADCAST) {
        ENET_MCR |= MCR_DBF;                                             // disable broadcast reception
    }

    ulRegister = ENET_SCR;                                               // get present status register value
    ulRegister |= (RX_MAX_BURST_SIZE_SINGLE | TX_MAX_BURST_SIZE_SINGLE);
    ENET_SCR = ulRegister;                                               // set burst sizes
#if PCLK == HCLK
    ENET_CCR = HCLK_EQUALS_PCLK;
#else
    ENET_CCR = HCLK_DOUBLE_PCLK;
#endif

    fnWriteMII(PHY_BASIC_CONTROL_REGISTER, PHY_SOFTWARE_RESET);          // reset the PHY
    i = 0xfffff;
    while (i--) {}                                                       // delay to ensure PHY reset

    ulPhyIdentifier = fnReadMII(PHY_IDENTIFIER_1);                       // check that the PHY is working correctly by reading its identifier
    ulPhyIdentifier <<= 16;
    ulPhyIdentifier |= fnReadMII(PHY_IDENTIFIER_2);                      // check that the PHY is working correctly by reading its identifier

    if ((ulPhyIdentifier & PHY_MASK) != PHY_IDENTIFIER) {
        return;                                                          // if the identifier is incorrect the phy isn't resonding correctly
    }

    usMIIData = 0;                                                       // configure the mode which we could like to use
    if (pars->usMode & (LAN_10M | LAN_100M)) {
        if (pars->usMode & LAN_100M)  {
            usMIIData |= PHY_SELECT_100M_SPEED;
        }
    }
    else {
        usMIIData |= (PHY_ENABLE_AUTO_NEGOTIATION | PHY_RESTART_AUTO_NEGOTIATION);
    }

    if (pars->usMode & FULL_DUPLEX) {
        ENET_MCR |= MCR_FDM;                                             // set full duplex operation
        usMIIData |= PHY_FULL_DUPLEX_MODE;
    }
    else {
        ENET_MCR |= MCR_DRO;                                             // disable frame reception during transmission
    }

    fnWriteMII(PHY_BASIC_CONTROL_REGISTER, usMIIData);                   // write the PHY operating mode

    /************************************************************ Get MAC working ****************************************/

    ENET_MAH = ((pars->ucMAC[5] << 8)  | (pars->ucMAC[4]));              // set the MAC address
    ENET_MAL = ((pars->ucMAC[3] << 24) | (pars->ucMAC[2] << 16) | (pars->ucMAC[1] << 8) | pars->ucMAC[0]);

    ulRegister = (pars->usSizeRx &= DMA_XFERCOUNT);                      // limit the size
    while (ulRegister & 0x3) {
        ulRegister++;                                                    // ensure the size of the buffer space is 32 bit aligned
    }
    ulRxEthernetBufferSize = (NXT_EN | ulRegister);                      // save rx buffer size and setting

    i = NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE;
    ptrBuffer = uMallocAlign((MAX_MALLOC)(ulRegister * NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE), 4);// create receiver buffer memory pool
    ptrBd = uMallocAlign((MAX_MALLOC)(sizeof(STR91XF_BD)*NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE), 4); // create a smaller pool of buffer descriptors
    ENET_RXNDAR = (unsigned long)ptrBd;                                  // inform the MAC where the first receive buffer is located
    ptrRxBd = ptrBd;                                                     // remember where it starts
    while (i--) {                                                        // initialise the buffer descriptors
        ptrBd->bd_dma_start_address = ptrBuffer;                         // buffer location
        ptrBd->bd_dma_control = ulRxEthernetBufferSize;                  // size of frame which can be accepted in the buffer
        ptrBd->bd_packet_status = RX_VALID_MASK;
        if (i != 0) {
            ptrBd->bd_dma_next = (unsigned char *)(ptrBd+1);
            ptrBuffer += ulRegister;
            ptrBd++;
        }
    }
    ptrBd->bd_dma_next = (unsigned char *)ptrRxBd;                       // set wrap in last buffer descriptior

    ulRegister = pars->usSizeTx;
    while (ulRegister & 0x3) {
        ulRegister++;                                                    // ensure the size of the buffer space is 32 bit aligned
    }

    i = NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE;

    ptrBuffer = uMallocAlign((MAX_MALLOC)(ulRegister * NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE), 4);// create transmitter buffer memory pool
    ptrBd = uMallocAlign((MAX_MALLOC)(sizeof(STR91XF_BD)*NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE), 4); // create a smaller pool of buffer descriptors

    ENET_TXNDAR = (unsigned long)ptrBd;                                  // inform the MAC where the first receive buffer is located
    ptrFirstTxBd = ptrTxBd = ptrBd;                                      // remember where it starts
    while (i--) {                                                        // initialise the buffer descriptors
        ptrBd->bd_dma_start_address = ptrBuffer;                         // set buffer address
      //ptrBd->bd_dma_control = 0;
      //ptrBd->bd_packet_status = 0;
        if (i != 0) {
            ptrBd->bd_dma_next = (unsigned char *)(ptrBd+1);
            ptrBuffer += ulRegister;
            ptrBd++;
        }
    }
    ptrBd->bd_dma_next = (unsigned char *)ptrFirstTxBd;                  // set wrap in last buffer
    ENET_TXNDAR |= NPOL_EN;

    ENET_ISR = CLEAR_ALL_INTS;                                           // reset all interrupts

    ulRegister = ENET_RXSTR;                                             // get present status register value
    ulRegister |= (DFETCH_DELAY);                                        // set rx start DFETCH delay for Rx block
    ENET_RXSTR = ulRegister;                                             // set burst sizes

    ulRegister = ENET_TXSTR;                                             // get present status register value
    ulRegister |= (DFETCH_DELAY | ENET_TX_UNDERRUN);                     // set tx start DFETCH delay for Tx block and enable underrun auto repeat
    ENET_TXSTR = ulRegister;                                             // set burst sizes

    VIC0_VA11R  = (unsigned long)EMAC_Interrupt;                         // enter the handling interrupt routine in the vector table
    VIC0_INTER |= VECTOR_11;
    VIC0_INTSR &= ~VECTOR_11;
    VIC0_VC11R  = (ENABLE_INTERRUPT | ETH_INT_0);

    ENET_IER = (                                                         // enable only necessary interrupts
                  RX_ENTRY_EN | TX_CURR_DONE_EN
               // | RX_EMPTY_EN | RX_FULL_EN
               // | RX_TO_EN | PACKET_LOSS_EN | RX_NEXT_EN | RX_DONE_EN | RX_MERR_INT_EN | RX_CURR_DONE_EN
               // | TX_EMPTY_EN | TX_FULL_EN | TX_ENTRY_EN | TX_TO_EN | TX_NEXT_EN | TX_DONE_EN | TX_MERR_INT_EN | MAC802_3_INT_EN
               );

    ENET_MCR |= (MCR_TE | MCR_RE);                                       // enable transmission and reception
    ENET_RXSTR |= START_FETCH;                                           // start RX DMA fetching BDs

    CONFIGURE_PHY_INT((unsigned long)PHY_Interrupt);                     // configure and enter the handling interrupt routine in the vector table

    fnWriteMII(PHY_INTERRUPT_ENABLE_REGISTER,                            // enable various PHY interrupts
              (PHY_AN_COMPLETE_INTERRUPT_ENABLE | PHY_REMOTE_FAULT_INTERRUPT_ENABLE |
               PHY_LINK_DOWN_INTERRUPT_ENABLE | PHY_AN_ACK_INTERRUPT_ENABLE | 
               PHY_FAULT_PARALLEL_INTERRUPT_ENABLE | PHY_AN_PAGE_INTERRUPT_ENABLE |
               PHY_RX_ERR_64_INTERRUPT_ENABLE)
               );

    #ifdef _WINDOWS
    fnOpenDefaultHostAdapter();                                          // configure PC NIC for Ethernet operation
    #endif
}


#ifdef _WINDOWS
    // This routine is supplied to aid the simulator. It checks whether Ethereal frames should be accepted based
    // on the programmed MAC address and the Ethernet operating mode
    //
    extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen)
    {
        unsigned char ucProgrammedMAC[MAC_LENGTH];

        // This function checks to see whether we can receive the simulated frame and can block it if wanted
        if (usLen < (2*MAC_LENGTH + 2)) {
            return 0;                                                    // too short to be a valid Ethernet frame
        }

        if ((!(ENET_MCR & MCR_DBF)) && (!(uMemcmp(ucData, cucBroadcast, MAC_LENGTH)))) {
            return 1;                                                    // broadcast
        }

        ucProgrammedMAC[5] = (unsigned char)(ENET_MAH>>8);
        ucProgrammedMAC[4] = (unsigned char)(ENET_MAH);
        ucProgrammedMAC[3] = (unsigned char)(ENET_MAL>>24);
        ucProgrammedMAC[2] = (unsigned char)(ENET_MAL>>16);
        ucProgrammedMAC[1] = (unsigned char)(ENET_MAL>>8);
        ucProgrammedMAC[0] = (unsigned char)(ENET_MAL);

        if (!(uMemcmp(ucData, ucProgrammedMAC, MAC_LENGTH))) {
            return 2;                                                    // our MAC
        }

        if (!uMemcmp(&ucData[MAC_LENGTH], ucProgrammedMAC, MAC_LENGTH)) {
            return 0;                                                    // we never accept echos of our own transmissions
        }

        if (ENET_MCR & RECEIVE_ALL) return 3;                            // in promiscuous mode we accept everything

        return 0;                                                        // no match so reject
    }
#endif

// This can be called by the Ethernet driver to wait a short time for the transmit buffer to become free
//
extern int fnWaitTxFree(void)
{
    return 0;                                                            // not needed with STR91XF
}

// This can be called by the ethernet driver to find out how many receive buffers the Ethernet Controller has
//
extern int fnGetQuantityRxBuf(void)
{
    return NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE;
}

// This can be called by the ethernet driver to get the address of the corresponding Ethernet receive buffer
//
extern unsigned char *fnGetRxBufferAdd(int iBufNr)
{
    return 0;                                                            // for compatibility - the STR91XF uses buffer descriptors for tracking buffers
}

// This can be called by the ethernet driver to get the address of the corresponding Ethernet transmit buffer
//
extern unsigned char *fnGetTxBufferAdd(int iBufNr)
{
    return (unsigned char *)ptrTxBd->bd_dma_start_address;
}

// This can be called by the ethernet driver to free an Ethernet receive buffer after it has been processed
//
extern void fnFreeEthernetBuffer(int iBufNr)
{
    ptrRxBd->bd_dma_control = ulRxEthernetBufferSize;
    ptrRxBd->bd_packet_status = RX_VALID_MASK;                           // allow the buffer to be used again
    ptrRxBd = (STR91XF_BD *)ptrRxBd->bd_dma_next;                        // go to next buffer descriptor
    ENET_RXSTR |= START_FETCH;                                           // start RX DMA fetching BDs
}

// Data has been prepared, now send it over the ETHERNET interface
//
extern QUEUE_TRANSFER fnStartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    while (DataLen < 60) {
        *ptr_put++ = 0x00;                                               // pad with zeros if smaller than 60 [chip must send at least 60]
        DataLen++;
    }

    ptrTxBd->bd_dma_control = DataLen;                                   // set new frame length
    ptrTxBd->bd_packet_status = TX_BD_VALID;                             // frame ready
    ENET_TXSTR |= START_FETCH;                                           // start TX DMA fetching BDs

#ifdef _WINDOWS
    fnSimulateEthTx((QUEUE_TRANSFER)(ptrTxBd->bd_dma_control));

    if ((usPhyMode & PHY_LOOP_BACK) || (ENET_MCR & MCR_LM)) {            // if the PHY or EMAC is in loop back mode, simulate reception of sent frame
        fnSimulateEthernetIn((ptr_put - DataLen), DataLen, 1);
    }
    ENET_ISR |= TX_CURR_DONE_EN;
    ptrTxBd->bd_packet_status = 0;                                       // simulate the ownership changing
    EMAC_Interrupt();                                                    // simulate end of transmission - tx interrupt
#endif
    return (DataLen);
}

#endif



#ifdef SERIAL_INTERFACE
/* =================================================================== */
/*                    Serial Interface - UART                          */
/* =================================================================== */

// Get a pointer to the start of the corresponding UART control block - control register
//
unsigned long *fnSelectChannel(QUEUE_HANDLE Channel)
{
    switch (Channel) {
    case 0:
        return (unsigned long *)(UART0_BLOCK + CR_OFFSET);
    case 1:
        return (unsigned long *)(UART1_BLOCK + CR_OFFSET);
    case 2:
        return (unsigned long *)(UART2_BLOCK + CR_OFFSET);
    default:
        return 0;
    }
}

/**************************** SCI0 interrupt ******************************************/

__interrupt void SCI0_Interrupt(void)
{
    while (UART_MIS_0 & (RXIM | TXIM)) {                                 // while interrupts present from UART0
        if (UART_MIS_0 & RXIM) {                                         // rx interrupt
            UART_ICR_0 = RXIC;                                           // clear receive interrupt
            fnSciRxByte((unsigned char)UART_DR_0, 0);                    // receive data interrupt
#ifdef _WINDOWS
            UART_RIS_0 &= ~(RXIM);                                       // simulate reset of interrupt flag
            UART_MIS_0 = (UART_RIS_0 & UART_IMSC_0);
#endif
        }

        if (UART_MIS_0 & TXIM) {
            UART_ICR_0 = TXIC;                                           // clear transmit interrupt
            fnSciTxByte(0);                                              // transmit data empty interrupt
#ifdef _WINDOWS
            UART_RIS_0 &= ~(TXIM);                                       // simulate reset of interrupt flag
            UART_MIS_0 = (UART_RIS_0 & UART_IMSC_0);
#endif
        }
    }
}


/**************************** SCI1 interrupt ******************************************/

__interrupt void SCI1_Interrupt(void)
{
    while (UART_MIS_1 & (RXIM | TXIM)) {                                 // while interrupts present from UART0
        if (UART_MIS_1 & RXIM) {                                         // Rx interrupt
            UART_ICR_1 = RXIC;                                           // clear receive interrupt
            fnSciRxByte((unsigned char)UART_DR_1, 1);                    // receive data interrupt
#ifdef _WINDOWS
            UART_RIS_1 &= ~(RXIM);                                       // simulate reset of interrupt flag
            UART_MIS_1 = (UART_RIS_1 & UART_IMSC_1);
#endif
        }

        if (UART_MIS_1 & TXIM) {
            UART_ICR_1 = TXIC;                                           // clear transmit interrupt
            fnSciTxByte(1);                                              // transmit data empty interrupt
#ifdef _WINDOWS
            UART_RIS_1 &= ~(TXIM);                                       // simulate reset of interrupt flag
            UART_MIS_1 = (UART_RIS_1 & UART_IMSC_1);
#endif
        }
    }
}

/**************************** SCI2 interrupt ******************************************/

__interrupt void SCI2_Interrupt(void)
{
    while (UART_MIS_2 & (RXIM | TXIM)) {                                 // while interrupts present from UART0
        if (UART_MIS_2 & RXIM) {                                         // Rx interrupt
            UART_ICR_2 = RXIC;                                           // clear receive interrupt
            fnSciRxByte((unsigned char)UART_DR_2, 2);                    // receive data interrupt
#ifdef _WINDOWS
            UART_RIS_2 &= ~(RXIM);                                       // simulate reset of interrupt flag
            UART_MIS_2 = (UART_RIS_2 & UART_IMSC_2);
#endif
        }

        if (UART_MIS_2 & TXIM) {                                         // Tx interrupt
            UART_ICR_2 = TXIC;                                           // clear transmit interrupt
            fnSciTxByte(2);                                              // transmit data empty interrupt
#ifdef _WINDOWS
            UART_RIS_2 &= ~(TXIM);                                       // simulate reset of interrupt flag
            UART_MIS_2 = (UART_RIS_2 & UART_IMSC_2);
#endif
        }
    }
}

#ifdef UART_EXTENDED_MODE
extern unsigned char fnGetMultiDropByte(QUEUE_HANDLE channel)            // dummy
{
    return 0;
}
#endif

// Configure the SCI hardware
//
extern void fnConfigSCI(QUEUE_HANDLE Channel, TTYTABLE *pars)
{
    unsigned long *ulReg = fnSelectChannel(Channel);
    unsigned char ucFraction;

    switch (Channel) {
    case 0:
      //SCU_PCGR1 |= UART0;                                              // enable clocks to UART0
      //SCU_PRR1  |= UART0;                                              // take UART0 out of reset
        SET_UART0_PERIPHERAL_PINS();                                     // connect UART0 to the chosen pins

        VIC1_VA0R   = (unsigned long)SCI0_Interrupt;                     // enter the handling interrupt routine in the vector table
        VIC1_INTER |= VECTOR_0;
        VIC1_INTSR &= ~VECTOR_0;
        VIC1_VC0R   = (ENABLE_INTERRUPT | UART0_INT_1);
        break;

    case 1:
      //SCU_PCGR1 |= UART1;                                              // enable clocks to UART1
      //SCU_PRR1  |= UART1;                                              // take UART1 out of reset
        SET_UART1_PERIPHERAL_PINS();                                     // connect UART1 to the chosen pins

        VIC1_VA1R   = (unsigned long)SCI1_Interrupt;                     // enter the handling interrupt routine in the vector table
        VIC1_INTER |= VECTOR_1;
        VIC1_INTSR &= ~VECTOR_1;
        VIC1_VC1R   = (ENABLE_INTERRUPT | UART1_INT_1);
        break;

    case 2:
      //SCU_PCGR1 |= UART1;                                              // enable clocks to UART2
      //SCU_PRR1  |= UART1;                                              // take UART2 out of reset
        SET_UART2_PERIPHERAL_PINS();                                     // connect UART2 to the chosen pins

        VIC1_VA2R   = (unsigned long)SCI2_Interrupt;                     // enter the handling interrupt routine in the vector table
        VIC1_INTER |= VECTOR_2;
        VIC1_INTSR &= ~VECTOR_2;
        VIC1_VC2R   = (ENABLE_INTERRUPT | UART2_INT_1);
        break;
    default:
        return;
    }

    *ulReg = 0;                                                          // start with uart disabled
    ulReg -= ((CR_OFFSET - IBRD_OFFSET)/sizeof(unsigned long));          // set to baud rate generator register

    switch (pars->ucSpeed) {
    case SERIAL_BAUD_300:
        *ulReg = (BRCLK/16/300);                                         // set 300
        ucFraction = (unsigned char)((float)((float)BRCLK/16/300 - (int)(BRCLK/16/300)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_600:
        *ulReg = (BRCLK/16/600);                                         // set 600
        ucFraction = (unsigned char)((float)((float)BRCLK/16/600 - (int)(BRCLK/16/600)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_1200:
        *ulReg = (BRCLK/16/1200);                                        // set 1200
        ucFraction = (unsigned char)((float)((float)BRCLK/16/1200 - (int)(BRCLK/16/1200)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_2400:
        *ulReg = (BRCLK/16/2400);                                        // set 2400
        ucFraction = (unsigned char)((float)((float)BRCLK/16/2400 - (int)(BRCLK/16/2400)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_4800:
        *ulReg = (BRCLK/16/4800);                                        // set 4800
        ucFraction = (unsigned char)((float)((float)BRCLK/16/4800 - (int)(BRCLK/16/4800)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_9600:
        *ulReg = (BRCLK/16/9600);                                        // set 9600
        ucFraction = (unsigned char)((float)((float)BRCLK/16/9600 - (int)(BRCLK/16/9600)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_14400:
        *ulReg = (BRCLK/16/14400);                                       // set 14400
        ucFraction = (unsigned char)((float)((float)BRCLK/16/14400 - (int)(BRCLK/16/14400)) * 64 + 0.5);
        break;
    default:                                                             // if not valid value set this one
    case SERIAL_BAUD_19200:
        *ulReg = (BRCLK/16/19200);                                       // set 19200
        ucFraction = (unsigned char)((float)((float)BRCLK/16/19200 - (int)(BRCLK/16/19200)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_38400:
        *ulReg = (BRCLK/16/38400);                                       // set 38400
        ucFraction = (unsigned char)((float)((float)BRCLK/16/38400 - (int)(BRCLK/16/38400)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_57600:
        *ulReg = (BRCLK/16/57600);                                       // set 57600
        ucFraction = (unsigned char)((float)((float)BRCLK/16/57600 - (int)(BRCLK/16/57600)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_115200:
        *ulReg = (BRCLK/16/115200);                                      // set 115200
        ucFraction = (unsigned char)((float)((float)BRCLK/16/115200 - (int)(BRCLK/16/115200)) * 64 + 0.5);
        break;
    case SERIAL_BAUD_230400:
        {
        *ulReg = (BRCLK/16/230400);                                      // set 230400
        ucFraction = (unsigned char)((float)((float)BRCLK/16/230400 - (int)(BRCLK/16/230400)) * 64 + 0.5);
        }
        break;
    }
    ulReg++;                                                             // move to fractional register
    *ulReg++ = ucFraction;                                               // set baud rate fraction and move to line control register

    if (pars->Config & (RS232_ODD_PARITY | RS232_EVEN_PARITY)) {
        if (pars->Config & RS232_ODD_PARITY) {
            *ulReg = STR_ODD_PARITY;
        }
        else {
            *ulReg = STR_EVEN_PARITY;
        }
    }
    else {
        *ulReg = STR_NO_PARITY;
    }

    if (pars->Config & (TWO_STOPS | ONE_HALF_STOPS)) {                   // chip doesn't support 1.5 parity bits so always set 2
#ifdef _WINDOWS
        pars->Config &= ~ONE_HALF_STOPS;                                 // ensure the simulator uses the correct mode
        pars->Config |= TWO_STOPS;
#endif
        *ulReg |= STR_2_STOPS;
    }

    if (pars->Config & CHAR_7) {
        *ulReg |= STR_CHAR_7;
    }
    else {
        *ulReg |= STR_CHAR_8;
    }

#ifdef _WINDOWS
    ulReg -= 2;                                                          // set back to speed setting
    fnConfigSimSCI(Channel, BRCLK/(*ulReg*16), pars);                    // open a serial port on PC if desired
#endif
}


// Enable reception on the defined channel
//
extern void fnRxOn(QUEUE_HANDLE Channel)
{
    unsigned long *ulReg = fnSelectChannel(Channel);

    *ulReg |= (UARTEN | RXE);                                            // enable the receiver
    ulReg += ((ICR_OFFSET - CR_OFFSET)/sizeof(unsigned long));           // move to interrupt clear register
    *ulReg = (OEIC | BEIC | PEIC | FEIC | RTIC | RXIC);                  // clear Rx interrupts
    ulReg -= ((ICR_OFFSET - IMSC_OFFSET)/sizeof(unsigned long));         // move to interrupt mask register
    *ulReg |= (RXIM);                                                    // enable interrupts when character received
}

// Disable reception on the defined channel
//
extern void fnRxOff(QUEUE_HANDLE Channel)
{
    unsigned long *ulReg = fnSelectChannel(Channel);
    *ulReg &= ~(RXE);                                                    // disable the receiver
}

// Enable transmission on the defined channel
//
extern void fnTxOn(QUEUE_HANDLE Channel)
{
    unsigned long *ulReg = fnSelectChannel(Channel);

    *ulReg |= (UARTEN | TXE);                                            // enable the transmitter
    ulReg += ((ICR_OFFSET - CR_OFFSET)/sizeof(unsigned long));           // move to interrupt clear register
    *ulReg = (TXIC);                                                     // clear Tx interrupts
    ulReg -= ((ICR_OFFSET - IMSC_OFFSET)/sizeof(unsigned long));         // move to interrupt mask register
    *ulReg |= (TXIM);                                                    // enable interrupts when character transmitted
}

// Disable transmission on the defined channel
//
extern void fnTxOff(QUEUE_HANDLE Channel)
{
    unsigned long *ulReg = fnSelectChannel(Channel);
    *ulReg &= ~(TXE);                                                    // disable the transmitter
}

// The TTY driver uses this call to disable the transmit interrupt of the serial port
//
extern void fnClearTxInt(QUEUE_HANDLE channel)
{
    switch (channel) {
    case 0:
        UART_IMSC_0 &= ~TXIM;
        break;
    case 1:
        UART_IMSC_1 &= ~TXIM;
        break;
    case 2:
        UART_IMSC_2 &= ~TXIM;
        break;
    }
}

// The TTY driver uses this call to send a byte of data over the serial port
//
extern int fnTxByte(QUEUE_HANDLE channel, unsigned char ucTxByte)
{
    switch (channel) {
    case 0:
        if (!(UART_FR_0 & TXFE)) {
            return 1;                                                    // busy, wait
        }
        UART_IMSC_0 |= TXIM;                                             // insure Tx interrupt is enabled
        UART_DR_0 = ucTxByte;                                            // send the byte
  #ifdef _WINDOWS
        UART_FR_0 &= ~TXFE;
        iInts |= CHANNEL_0_SERIAL_INT;                                   // simulate interrupt
  #endif
        break;

    case 1:
        if (!(UART_FR_1 & TXFE)) {
            return 1;                                                    // busy, wait
        }
        UART_IMSC_1 |= TXIM;                                             // insure Tx interrupt is enabled
        UART_DR_1 = ucTxByte;                                            // send the byte
  #ifdef _WINDOWS
        UART_FR_1 &= ~TXFE;
        iInts |= CHANNEL_1_SERIAL_INT;                                   // simulate interrupt
  #endif
        break;

    case 2:
        if (!(UART_FR_2 & TXFE)) {
            return 1;                                                    // busy, wait
        }
        UART_IMSC_2 |= TXIM;                                             // insure Tx interrupt is enabled
        UART_DR_2 = ucTxByte;                                            // send the byte
  #ifdef _WINDOWS
        UART_FR_2 &= ~TXFE;
        iInts |= CHANNEL_2_SERIAL_INT;                                   // simulate interrupt
  #endif
        break;
    default:
        return 1;
    }
    return 0;                                                            // OK. Byte sent
}

#ifdef SUPPORT_HW_FLOW                                                   // not yet supported
extern void fnControlLine(QUEUE_HANDLE channel, unsigned short usModifications, UART_MODE_CONFIG OperatingMode)
{
}

extern QUEUE_TRANSFER fnControlLineInterrupt(QUEUE_HANDLE channel, unsigned short usModifications, UART_MODE_CONFIG OperatingMode)
{
    return 0;
}
#endif

#endif




/* =================================================================== */
/*                           FLASH driver                              */
/* =================================================================== */


#ifdef SPI_FILE_SYSTEM
extern QUEUE_HANDLE SPI_handle;                                          // EEPROM commands
#define M95XXX_WRITE_ENABLE   0x06                                       // send this prior to write or write status register command
#define M95XXX_WRITE_DISABLE  0x04
#define M95XXX_READ_STAT_REG  0x05
#define M95XXX_WRITE_STAT_REG 0x01
#define M95XXX_READ_MEMORY    0x03
#define M95XXX_WRITE_MEMORY   0x02


int fnWriteBytesEEPROM(unsigned char *ucDestination, unsigned char *ucData, unsigned short usLength)
{
    unsigned char usByteWrite[4];                                        // our file sytem is in external EEPROM via SPI bus.
    usByteWrite[0] = M95XXX_WRITE_MEMORY;
    usByteWrite[1] = (unsigned char)((unsigned short)(ucDestination)>> 8);
    usByteWrite[2] = (unsigned char)((unsigned short)ucDestination);
    usByteWrite[3] = PREPARE_PAGE_WRITE;
    #ifdef SPI_INTERFACE
    fnWrite(SPI_handle, usByteWrite, sizeof(usByteWrite));               // set the byte write address
    fnWrite(SPI_handle, ucData, usLength);                               // write the data
    #else
    fnSendSPIMessage(usByteWrite, sizeof(usByteWrite));                  // send immediately
    fnSendSPIMessage(ucData, usLength);                                  // write the data immediately
    #endif
    ucEEPROMDanger = 1;                                                  // we should avoid reading until we know that the write has terminated
    return 0;
}


// SPI reserved exclusively for a EEPROM file system
//
void fnSendSPIMessage(unsigned char *ptrData, QUEUE_TRANSFER Length)     // master transmission
{
    volatile unsigned long ulDummy;                                      // dummy variable for clearing flags
    unsigned long ulCS_state = (SPI_CS_PORT & SPI_EEPROM_CS);            // check the state of the CS line on entry
    int iDelete = (ptrData == 0);                                        // a page delete has been commanded

    if (ulCS_state) {                                                    // if this is the first entry it has a control byte at the end
        Length--;                                                        // reduce the count so we don't send the control byte
        if (Length) {
            SPI_CS_PORT &= ~(SPI_EEPROM_CS);                             // assert CS
        }
    }

    ulDummy = SSP_DR;                                                    // reset receive data flag with dummy read - the rx data is not interesting here

    while (Length--) {                                                   // for each byte we want to send
        if (iDelete) {
            SSP_DR = 0xff;
        }
        else {
            SSP_DR = (*ptrData++);                                       // start byte transmission
        }
#ifdef _WINDOWS
        fnSimM95xxx(M95XXX_WRITE, (unsigned char)SSP_DR, (unsigned char)((SPI_CS_PORT & SPI_EEPROM_CS) != 0)); // simulate the SPI EEPROM device
        SSP_SR |= (SSP_RNE | SSP_TFE);                                   // simulate tx and rx interrupt flags being set
#endif
        while (SSP_SR & SSP_BUSY) {};                                    // wait until tx byte has been sent and the received byte is complete
        ulDummy = SSP_DR;                                                // reset receive data flag with dummy read - the rx data is not interesting here
    }

    if ((!ulCS_state || (PREPARE_READ != *(ptrData)))) {
        SPI_CS_PORT |= SPI_EEPROM_CS;                                    // deassert SS on exit
    }
#ifdef _WINDOWS
    fnSimM95xxx(M95XXX_CHECK_SS, 0, (unsigned char)((SPI_CS_PORT & SPI_EEPROM_CS) != 0)); // simulate the SPI EEPROM device
#endif
}


// We use this in master mode to get the next SPI byte as fast as possible
//
void fnGetSPI_bytes(unsigned char *ptrBuffer, MAX_FILE_LENGTH Length)
{                                                                        // master reception - we assume SSEL asserted before entry (commanded previously)
    while (SSP_SR & SSP_BUSY) {};                                        // wait until free - should be very short wait

    while (Length--) {
        SSP_DR = (0xff);                                                 // send a dummy byte to cause reception transfer
#ifdef _WINDOWS
        SSP_DR = fnSimM95xxx(M95XXX_READ, (unsigned char)SSP_DR, (unsigned char)((SPI_CS_PORT & SPI_EEPROM_CS) != 0));
        SSP_SR |= (SSP_RNE | SSP_TFE);                                   // simulate tx and rx interrupt flags being set
#endif
        while (SSP_SR & SSP_BUSY) {};                                    // wait for the byte to be transmitted / received (check rx)
        *ptrBuffer++ = (unsigned char)SSP_DR;                            // copy the received byte to the input buffer
#ifdef _WINDOWS
        SSP_SR &= ~(SSP_RNE | SSP_TFE);
#endif
    }

    SPI_CS_PORT |= SPI_EEPROM_CS;                                        // always deassert CS on exit

#ifdef _WINDOWS
    fnSimM95xxx(M95XXX_CHECK_SS, 0, (unsigned char)((SPI_CS_PORT & SPI_EEPROM_CS) != 0));
#endif
}

static void fnWaitWriteComplete(void)
{
    #define WIP 0x01
    static const unsigned char ucRead[] = {M95XXX_READ_STAT_REG, PREPARE_READ}; // read from status register
    unsigned char ucStatus;

    if (!ucEEPROMDanger) return;                                         // no point in checking since we are sure no write is in progress

    do {
 #ifdef SPI_INTERFACE
        fnWrite(SPI_handle, (unsigned char *)ucRead, sizeof(ucRead));
        fnRead(SPI_handle, &ucStatus, 1);                                // read the present status
 #else
        fnSendSPIMessage((unsigned char *)ucRead, sizeof(ucRead));
        fnGetSPI_bytes(&ucStatus, 1);
 #endif
    } while (ucStatus & WIP);                                            // wait until an in progress write is complete
    ucEEPROMDanger = 0;                                                  // mark eeprom not writimg so no danger reading data from it
 #ifdef _WINDOWS
    fnEEPROM_safe();
 #endif
}

#endif


#ifdef ACTIVE_FILE_SYSTEM

// This routine to delete FLASH sectors - we put it in SRAM
// Warning: this is designed for little endian Thumb mode ONLY
//
static int fnDelFlashSect(volatile unsigned short *ptrSector)
{
    int iErrorStatus;
#ifdef _WINDOWS
    unsigned long ulSectorLength = fnGetSectorSize((unsigned char *)ptrSector);
    unsigned char *ptrSectData;

    if (fnIsProtected((unsigned char *)ptrSector)) {
        return BSR_SP_PROTECTION_ERROR;
    }

    ptrSector = (volatile unsigned short *)fnGetFlashAdd((unsigned char *)ptrSector);
    ptrSectData = (unsigned char *)ptrSector;
#endif

    *ptrSector = CUI_SECTOR_ERASE_SETUP;
    *ptrSector = CUI_ERASE_CONFIRM;
#ifndef _WINDOWS
    while (!(*ptrSector & BSR_PECS_READY)) {}                            // wait until ready
    iErrorStatus = (*ptrSector & (BSR_ES_FAILURE | BSR_PS_FAILURE | BSR_SP_PROTECTION_ERROR));
#else
    iErrorStatus = 0;
#endif

    *ptrSector = CUI_CLEAR_STATUS_REGISTER;                              // clear status register
    *ptrSector = CUI_READ_ARRAY;                                         // completed

#ifdef _WINDOWS
    uMemset(ptrSectData, 0xff, ulSectorLength);                          // delete the sector contents
#endif
    return iErrorStatus;                                                 // OK
}


// Erase the sectore in which the address is in
//
#if defined (COMPILE_IAR)                                                // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
 #pragma optimize=2
#endif
extern int fnEraseFlashSector(MEMORY_RANGE_POINTER usSectorPointer, MAX_FILE_LENGTH Length)
{
    do {
#ifdef SPI_FILE_SYSTEM
        static const unsigned char ucWriteEnable[] = {M95XXX_WRITE_ENABLE, TERMINATE_WRITE}; // enable write
        unsigned short ucDeleted = 0;
        unsigned char *ptrEEPROM = (unsigned char *)ptrSector;
        unsigned short usTemp = (unsigned short)ptrEEPROM;

        usTemp &= ~(EEPROM_PAGE_SIZE-1);                                 // we want to erase a block (program them to 0xff)
        Length += ((unsigned short)((CAST_POINTER_ARITHMETIC)ptrSector) - usTemp);
        ptrEEPROM = (unsigned char *)usTemp;
        while (ucDeleted < FILE_GRANULARITY) {
            fnWaitWriteComplete();                                       // wait until free to write
        #ifdef SPI_INTERFACE
            fnWrite(SPI_handle, (unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
            fnWriteBytesEEPROM(ptrEEPROM, ucDel, EEPROM_PAGE_SIZE);      // delete a page
        #else
            fnSendSPIMessage((unsigned char *)ucWriteEnable, sizeof(ucWriteEnable)); // prepare write
            fnWriteBytesEEPROM(ptrEEPROM, 0, EEPROM_PAGE_SIZE);          // delete an eeprom page
        #endif
            ucDeleted += EEPROM_PAGE_SIZE;
            ptrEEPROM += EEPROM_PAGE_SIZE;
        }
        if (Length <= FLASH_GRANULARITY) {
            break;
        }
        Length -= FLASH_GRANULARITY;
        return 0;
#else
    #ifdef _WINDOWS
    #define PROG_WORD_SIZE_2 500
    #else
    #define PROG_WORD_SIZE_2 100                                         // adequate space for the small program
    #endif
    static unsigned short usProgSpace[PROG_WORD_SIZE_2];                 // make space for the routine on stack (this will have a word boundary)
    static int (*fnRAM_code)(volatile unsigned short *) = 0;
    volatile unsigned short *ptrSector = (volatile unsigned short *)usSectorPointer;
    volatile unsigned short *usSectorPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrSector & ~0x03);
    int iErrorStatus;
    Length += (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(FLASH_GRANULARITY-1)));
    ptrSector = (volatile unsigned short *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(FLASH_GRANULARITY-1)); // set to sector boundary


    if (!fnRAM_code) {                                                   // real address of code in thumb mode
        fnRAM_code = (int(*)(volatile unsigned short *))(fnCopyProgram((unsigned char *)fnDelFlashSect, PROG_WORD_SIZE_2, usProgSpace));
    }

    fnProtect(usSectorPointer, CUI_UNPROTECT_LEVEL1_CONFIRM);            // unprotect sector ready for delete

    uDisable_Interrupt();                                                // protect this region from interrupts
    #ifdef _WINDOWS
    iErrorStatus = fnDelFlashSect(usSectorPointer);
    #else
    iErrorStatus = fnRAM_code(usSectorPointer);
    #endif
    uEnable_Interrupt();                                                 // safe to accept interrupts again

    fnProtect(usSectorPointer, CUI_PROTECT_LEVEL1_CONFIRM);              // protect sector after use

    if (iErrorStatus != 0) {
        return iErrorStatus;
    }
    ptrSector += FLASH_GRANULARITY;
    if (Length <= FLASH_GRANULARITY) {
        break;
    }
    Length -= FLASH_GRANULARITY;
    } while (Length);
    return 0;                                                            // OK
#endif
}


#ifdef FLASH_FILE_SYSTEM

// This routine runs from SRAM and is designed for little endian Thumb
//
static void fnProtect(volatile unsigned short *usSectorPointer, unsigned short usCommand)
{
#ifdef _WINDOWS
    unsigned short usOriginalFlash;
    if (CUI_UNPROTECT_LEVEL1_CONFIRM == usCommand) {
        fnUnprotectSector(usSectorPointer);
    }
    else if (CUI_PROTECT_LEVEL1_CONFIRM == usCommand) {
        fnProtectSector(usSectorPointer);
    }
    usSectorPointer = (volatile unsigned short *)fnGetFlashAdd((unsigned char *)usSectorPointer);
    usOriginalFlash = *usSectorPointer;
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
#if defined (COMPILE_IAR)                                                // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
 #pragma optimize=2
#endif
extern void fnProtectFlash(unsigned char *ptrSector, unsigned char ucProtection)
{
#ifdef _WINDOWS
    #define PROG_WORD_SIZE_1 300
#else
    #define PROG_WORD_SIZE_1 30                                          // adequate space for the small program
#endif
    static unsigned short usProgSpace[PROG_WORD_SIZE_1];                 // make space for the routine on stack (this will have a word boundary)
    static void (*fnRAM_code)(volatile unsigned short *, unsigned short) = 0;
    volatile unsigned short *usSectorPointer = (unsigned short *)((CAST_POINTER_ARITHMETIC)ptrSector & ~0x03);
    unsigned short usCommand;

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
#ifdef _WINDOWS
    fnProtect(usSectorPointer, usCommand);
#else
    fnRAM_code(usSectorPointer, usCommand);
#endif
    uEnable_Interrupt();                                                 // safe to accept interrupts again
}

// This routine to command FLASH - we put it in SRAM
// Warning: this is designed for little endian Thumb mode ONLY
//
static int fnFlashFlash(volatile unsigned short *usAddressPointer, volatile unsigned short *usWritePointer, unsigned short usValue)
{
    int iErrorStatus;
#ifdef _WINDOWS
    unsigned short usCommandValue = *usAddressPointer;                   // backup for simulation
    if (fnIsProtected(fnPutFlashAdd((unsigned char *)usAddressPointer))) { // check whether sector is protected
        return BSR_SP_PROTECTION_ERROR;
    }
    iErrorStatus = BSR_PECS_READY;
#endif
    *usAddressPointer = CUI_PROGRAM_SETUP;                               // prepare sector to program
    *usWritePointer = usValue;                                           // word to program
#ifndef _WINDOWS
    while (!(*usAddressPointer & BSR_PECS_READY)) {                      // wait until ready
    }
    iErrorStatus = *usAddressPointer;                                    // read the status after the write
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
#if defined (COMPILE_IAR)                                                // disable optimising because it moves the SRAM call outside of the enable/disable irq !!
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
#endif

    if (*usWordPointer == usValue) {                                     // if value is already programmed return with success
        return 0;
    }
    if ((*usWordPointer & usValue) != usValue) {
        return (BSR_ILLEGAL_WRITE);                                      // if the write is illegal (tries to change bits from 0 to 1) signal this with pseudo bit
    }

    uDisable_Interrupt();                                                // protect this region from interrupts
#ifdef _WINDOWS
    iErrorStatus = fnFlashFlash(usAddressPointer, usWordPointer, usValue);
#else
    iErrorStatus = fnRAM_code(usAddressPointer, usWordPointer, usValue); // commit the word to FLASH and return the state
#endif
    uEnable_Interrupt();                                                 // safe to accept interrupts again

    return (iErrorStatus);                                               // return the state
}
#else
  static void fnProtect(volatile unsigned short *usSectorPointer, unsigned short usCommand) {} // dummy
  extern void fnProtectFlash(unsigned char *ptrSector, unsigned char ucProtection) {}          // dummy
#endif

// RAW write of data to non-volatile memory
//
extern int fnWriteBytesFlash(unsigned char *ucDestination, unsigned char *ucData, MAX_FILE_LENGTH Length)
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
  #if defined _LITTLE_ENDIAN || defined (_WINDOWS)                       // little-endian operation
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
  #else                                                                  // big-endian operation
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
        if (fnWriteWordFlash((unsigned short *)ucDestination, usValue)) {
            return 1;
        }
        ucDestination += 2;
    }
    return 0;
#endif
}

#ifdef _OLD_VERSION
// Erase a FLASH sector. The pointer can be anywhere in the sector to be erased
//
extern int fnEraseFlashSector(MEMORY_RANGE_POINTER ptrFile, MAX_FILE_LENGTH Length)
{
    int iSectors = FILE_GRANULARITY/FLASH_GRANULARITY;                   // the number of flash sectors in a file block

    ptrFile = (unsigned char *)((unsigned long)ptrFile & ~(FILE_GRANULARITY-1));

    while (iSectors--) {
        if (fnEraseFlash((volatile unsigned short *)ptrFile)) return 1;
        ptrFile += FLASH_GRANULARITY;
    }
    return 0;
}
#endif

#endif




/************************************************************************************************************/


#ifdef IIC_INTERFACE

static void _IIC_Interrupt(QUEUE_HANDLE Channel)
{
    IICQue *tx_control = IIC_tx_control[Channel];
    IICQue *rx_control = IIC_rx_control[Channel];
    volatile unsigned long *ulReg;
    unsigned long ulStatus1, ulStatus2;

    if (Channel == 0) {
        ulReg = (unsigned long *)I2C0_BLOCK;
        ulStatus1 = I2C0_SR1;
        ulStatus2 = I2C0_SR2;
    }
    else {
        ulReg = (unsigned long *)I2C1_BLOCK;
        ulStatus1 = I2C1_SR1;
        ulStatus2 = I2C1_SR2;
    }

    if (tx_control->ucState & RX_ACTIVE) {
        unsigned char ucFirstRead = tx_control->ucState & RX_ACTIVE_FIRST_READ;
        tx_control->ucState &= ~RX_ACTIVE_FIRST_READ;
        if (tx_control->ucPresentLen == 1) {
            *ulReg = (IIC_PE | IIC_ITE);                                 // we don't acknowledge last byte
        }
        else if (!tx_control->ucPresentLen) {                            // we have completed the read
            *ulReg = (IIC_PE | STOP_COND);                               // send end condition and disable interrupts                                          //
            tx_control->ucState &= ~(TX_WAIT | TX_ACTIVE | RX_ACTIVE);
            rx_control->msgs++;
            if ( rx_control->wake_task ) {                               // wake up the receiver task if desired
               uTaskerStateChange( rx_control->wake_task, UTASKER_ACTIVATE); // wake up owner task
            }
        }
        else {
            *ulReg = (IIC_PE | IIC_ACK | IIC_ITE);                       // ensure we acknowledge multibyte reads
        }

        ulReg += (I2C_DR_OFFSET/sizeof(unsigned long));                  // move to DR register

        if (ucFirstRead) {                                               // have we just sent the slave address?
            volatile unsigned long ulRx;
            ulRx = *ulReg;                                               // dummy read
        }
        else {
            *rx_control->IIC_queue.put++ = (unsigned char)*ulReg;        // read the byte
            rx_control->IIC_queue.chars++;                               // and put it into the rx buffer
            if (rx_control->IIC_queue.put >= rx_control->IIC_queue.buffer_end) {
                rx_control->IIC_queue.put = rx_control->IIC_queue.QUEbuffer;
            }
        }

        if (tx_control->ucPresentLen) {
            tx_control->ucPresentLen--;
 #ifdef _WINDOWS
            ulReg -= ((I2C_DR_OFFSET - I2C_SR1_OFFSET)/ sizeof(unsigned long));
            *ulReg |= (TRA);
            ulReg += ((I2C_DR_OFFSET - I2C_SR1_OFFSET)/ sizeof(unsigned long));
            *ulReg = fnSimIIC_devices(IIC_RX_DATA, (unsigned char)*ulReg); // simulate the interrupt directly
            if (Channel == 0) {
                iInts |= IIC_INT0;
            }
            else {
                iInts |= IIC_INT1;
            }
 #endif
        }
        else {                                                           // read sequence complete. Continue with next write if something is waiting.
            if (tx_control->IIC_queue.chars != 0) {
                fnTxIIC(tx_control, Channel);                            // we have another message to send so we can send a repeated start condition
            }
        }
        return;
    }

    if (tx_control->ucPresentLen--) {                                    // send next byte to send if available
        if (ulStatus2 & ENDAD) {
            *ulReg = (IIC_PE | IIC_ITE);                                 // clear the ENDAD interrupt
#ifdef _WINDOWS
            *(ulReg + 2) = 0;
#endif
        }
        ulReg += (I2C_DR_OFFSET/sizeof(unsigned long));                  // move to DR register
        *ulReg = *tx_control->IIC_queue.get++;
        if (tx_control->IIC_queue.get >= tx_control->IIC_queue.buffer_end) {
            tx_control->IIC_queue.get = tx_control->IIC_queue.QUEbuffer; // handle the ring buffer
        }
 #ifdef _WINDOWS
        ulReg -= ((I2C_DR_OFFSET - I2C_SR1_OFFSET)/ sizeof(unsigned long));
        *ulReg |= (TRA | BTF);                                           // simulate the interrupt directly
        ulReg += ((I2C_DR_OFFSET - I2C_SR1_OFFSET)/ sizeof(unsigned long));
        fnSimIIC_devices(IIC_TX_DATA, (unsigned char)*ulReg);
        if (Channel == 0) {
            iInts |= IIC_INT0;
        }
        else {
            iInts |= IIC_INT1;
        }
 #endif
    }
    else {
        *ulReg = (IIC_PE | STOP_COND);
        if (!(tx_control->IIC_queue.chars)) {                            // transmission complete
          //*ulReg = (IIC_PE | STOP_COND);                               // send stop condition and disable interrupts

            tx_control->ucState &= ~(TX_WAIT | TX_ACTIVE | RX_ACTIVE);

            if ( tx_control->wake_task ) {
               uTaskerStateChange( tx_control->wake_task, UTASKER_ACTIVATE);// wake up owner task since the transmission has terminated
            }
        }
        else {
            fnTxIIC(tx_control, Channel);                                // we have another message to send so we can send a repeated start condition
        }
    }
}

static __interrupt void _IIC_Interrupt_0(void)
{
    volatile unsigned long ulEvent = I2C0_SR1;                           // read status register (followed by read or write to data register to clear interrupt)
    _IIC_Interrupt(0);
}

static __interrupt void _IIC_Interrupt_1(void)
{
    volatile unsigned long ulEvent = I2C1_SR1;                           // read status register (followed by read or write to data register to clear interrupt)
    _IIC_Interrupt(1);
}

// Configure the IIC hardware
//
void fnConfigIIC(IICTABLE *pars)
{
    unsigned long ulSpeedReg, ulECCR;

    #if PCLK <= 10000000
        #define FREQUENCY_BITS 0
    #elif PCLK <= 16670000
        #define FREQUENCY_BITS FR0
    #elif PCLK <= 26670000
        #define FREQUENCY_BITS FR1
    #elif PCLK <= 40000000
        #define FREQUENCY_BITS (FR1 | FR0)
    #else
        #define FREQUENCY_BITS FR2
    #endif
    #define IIC_DIVIDER_100K ((PCLK/(2*100000)) - 7)                     // ratio required for 100kHz
    #define IIC_DIVIDER_400K ((PCLK/(3*400000)) - 9)                     // ratio required for 400kHz

    if (pars->usSpeed == 400) {                                          // high speed 400k
        ulSpeedReg = ((IIC_DIVIDER_400K & 0x7f) | FM_FS);                // Fast I2C mode above 100kHz
        ulECCR = (IIC_DIVIDER_400K >> 7);
    }
    else {                                                               // 100k
        ulSpeedReg = (IIC_DIVIDER_100K & 0x7f);
        ulECCR = (IIC_DIVIDER_100K >> 7);
    }

    if (pars->Channel != 0) {
        SCU_PCGR1 |= IIC1;                                               // enable clocks
        SCU_PRR1  |= IIC1;                                               // take out of reset

        SET_IIC1_PERIPHERAL_PINS();         _SIM_PORT_CHANGE             // connect the IIC controller to the desired pins

        do {
            I2C1_CCR = ulSpeedReg;                                       // set desired speed
        } while (I2C1_CCR != ulSpeedReg);                                // ensure first command accepted after enabling peripheral
        I2C1_ECCR = ulECCR;
        I2C1_OAR2 = FREQUENCY_BITS;

        VIC1_VA4R   = (unsigned long)_IIC_Interrupt_1;                   // enter the handling interrupt routine in the vector table
        VIC1_INTER |= VECTOR_4;
        VIC1_INTSR &= ~VECTOR_4;
        VIC1_VC4R   = (ENABLE_INTERRUPT | IIC1_INT_1);

        I2C1_CR = IIC_PE;                                                // activate IIC interface
    }
    else {
        SCU_PCGR1 |= IIC0;                                               // enable clocks
        SCU_PRR1  |= IIC0;                                               // take out of reset

        SET_IIC0_PERIPHERAL_PINS();         _SIM_PORT_CHANGE             // connect the IIc controller to the desired pins

        do {
            I2C0_CCR = ulSpeedReg;                                       // set desired speed
        } while (I2C0_CCR != ulSpeedReg);                                // ensure first command accepted after enabling peripheral
        I2C0_ECCR = ulECCR;
        I2C0_OAR2 = FREQUENCY_BITS;

        VIC1_VA3R   = (unsigned long)_IIC_Interrupt_0;                   // enter the handling interrupt routine in the vector table
        VIC1_INTER |= VECTOR_3;
        VIC1_INTSR &= ~VECTOR_3;
        VIC1_VC3R   = (ENABLE_INTERRUPT | IIC0_INT_1);

        I2C0_CR = IIC_PE;                                                // activate IIC interface
    }
#ifdef _WINDOWS
    fnConfigSimIIC(pars->Channel, (pars->usSpeed * 1000));
#endif
}


// Send a first byte to IIC bus
//
extern void fnTxIIC(IICQue *ptIICQue, QUEUE_HANDLE Channel)
{
    unsigned char ucAddress;
    IICQue *tx_control = IIC_tx_control[Channel];
    IICQue *rx_control = IIC_rx_control[Channel];
    volatile unsigned long *ulReg;
    if (Channel == 0) {
        ulReg = (unsigned long *)I2C0_BLOCK;
    }
    else {
        ulReg = (unsigned long *)I2C1_BLOCK;
    }

    ptIICQue->ucPresentLen = *ptIICQue->IIC_queue.get++;                 // get present length
    if (ptIICQue->IIC_queue.get >= ptIICQue->IIC_queue.buffer_end) {
        ptIICQue->IIC_queue.get = ptIICQue->IIC_queue.QUEbuffer;         // handle circular buffer
    }

    *ulReg = (IIC_PE | START_COND);                                      // set transmit mode without interrupt enabled and cause start condition to be sent
    ucAddress = *ptIICQue->IIC_queue.get++;                              // get the address to send
    ulReg++;                                                             // move to status register
    while (!(*ulReg & IIC_START_BIT)) { _SIM_START_BIT }                 // wait until start has been sent

    ulReg += ((I2C_DR_OFFSET - 1)/sizeof(unsigned long));                // move to DR register

    *ulReg = ucAddress;                                                  // send the slave address (this includes the rd/wr bit)
    if (ptIICQue->IIC_queue.get >= ptIICQue->IIC_queue.buffer_end) {
        ptIICQue->IIC_queue.get = ptIICQue->IIC_queue.QUEbuffer;         // handle circular buffer
    }

    if (ucAddress & 0x01) {                                              // reading
        tx_control->ucState |= (RX_ACTIVE | RX_ACTIVE_FIRST_READ | TX_ACTIVE);
        ptIICQue->IIC_queue.chars -= 3;
        rx_control->wake_task = *ptIICQue->IIC_queue.get++;              // enter task to be woken when reception has completed
        if (ptIICQue->IIC_queue.get >= ptIICQue->IIC_queue.buffer_end) {
            ptIICQue->IIC_queue.get = ptIICQue->IIC_queue.QUEbuffer;     // handle circular buffer
        }
    }
    else {
        tx_control->ucState |= (TX_ACTIVE);                              // writing
        ptIICQue->IIC_queue.chars -= (ptIICQue->ucPresentLen+1);
    }
    ulReg -= ((I2C_DR_OFFSET)/sizeof(unsigned long));                    // move to control register
    *ulReg = (IIC_PE | IIC_ITE);                                         // enable interrupt

    #ifdef _WINDOWS
    ulReg += (I2C_SR1_OFFSET/ sizeof(unsigned long));
    *ulReg |= (EVF | TRA | BTF);                                         // simulate the interrupt directly
    *(ulReg+1) = ENDAD;
    ulReg += ((I2C_DR_OFFSET - I2C_SR1_OFFSET)/ sizeof(unsigned long));
    fnSimIIC_devices(IIC_ADDRESS, (unsigned char)*ulReg);
    if (Channel == 0) {
        iInts |= IIC_INT0;
    }
    else {
        iInts |= IIC_INT1;
    }
    #endif
}

#endif


/************************************************************************************************************/

#ifdef SPI_INTERFACE
// configure the hardware
//
void fnConfigSPI(SPITABLE *pars)
{
}

// we send a byte over the SPI as quickly as possible - without using interrupts since it should be very fast
//
int fnSendSPIByte(unsigned char ucByte)
{
    static unsigned char ucLastRx = 0x55;
    int iRpt = 0;
    #define MAX_SPI_REP  20                                              // limit retries to this amount

//    while (iRpt++ < MAX_SPI_REP) {
        while (!(SPISR & SPTEF)) {};                                     // wait until buffer free (should be very short wait)

        SPIDR = ucByte;                                                  // this sends the byte

#ifdef _WINDOWS
        SPISR |= SPIF;                                                   // simulate rx received
        SPIDR = ucLastRx;                                                // simulate a slave sending inverted last byte as confirmation back
#endif
        while (!(SPISR & SPIF)) {};                                      // wait until receive byte available

        for (iRpt = 0; iRpt < 1000; iRpt++) {};
        return 0;

        if (ucLastRx == SPIDR) {
          ucLastRx = ~ucByte;
#ifdef _WINDOWS
          fnSimSPITx(SPIDR);
#endif
          return 0;                                                      // transmission was acknowledged
        }
    //}

    return 1;                                                            // failed since no acknowledge from slave
}
#else
    #ifdef SPI_FILE_SYSTEM                                               // only a file system on SPI
static void fnConfigSPIFileSystem(unsigned char ucSpeed)
{
    POWERUP_SPI();                                                       // apply clocks to SSP and take it out of reset
    CONFIG_SPI_PINS();                                  _SIM_PORT_CHANGE // configure the I/O pins for SPI
    CONFIG_CS_LINE();                                                    // configure the CS control output
    SSP_CR_0 = (CPOL | FRAME_FORMAT_NAT | (8 - 1));                      // set format and data size

    switch (ucSpeed) {
    case MAX_SPI:                                                        // set maximum possible SPI speed
	    SSP_PR = (SSP_MAX_CLOCK);
	    break;

    case SPI_4MEG:
	    SSP_PR = ((PCLK + 4000000/2)/4000000);
	    break;

    case SPI_2MEG:
	    SSP_PR = ((PCLK + 2000000/2)/2000000);
	    break;

    case SPI_1MEG:
	    SSP_PR = ((PCLK + 1000000/2)/1000000);
	    break;

    case SPI_SLAVE:
	    return;
    }
    SSP_CR_1 = SSP_SSE;                                                  // enable SPI
}
    #endif
#endif

#if (defined ACTIVE_FILE_SYSTEM  || defined USE_PARAMETER_BLOCK || defined INTERNAL_USER_FILES) && defined _WINDOWS
    extern int iFetchingInternalMemory = 0;
#endif

extern void fnGetParsFile(unsigned char *ParLocation, unsigned char *ptrValue, MAX_FILE_LENGTH Size)
{
#ifdef ACTIVE_FILE_SYSTEM
    #ifdef _WINDOWS
    if (iFetchingInternalMemory != 0) {
        uMemcpy(ptrValue, ParLocation, Size);                            
        iFetchingInternalMemory = 0;
        return;
    }
    #endif
    #ifdef SPI_FILE_SYSTEM                                               // our file sytem is in external EEPROM via SPI bus.
    unsigned char ucRead[4];                                             // our file sytem is in external EEPROM via SPI bus.
    ucRead[0] = M95XXX_READ_MEMORY;
    ucRead[1] = (unsigned char)((unsigned short)(ParLocation)>> 8);
    ucRead[2] = (unsigned char)((unsigned short)ParLocation);
    ucRead[3] = PREPARE_READ;
    fnWaitWriteComplete();                                               // ensure the chip is ready to be read
        #ifdef SPI_INTERFACE
    fnWrite(SPI_handle, ucRead, sizeof(ucRead));                         // set the read address
    fnRead(SPI_handle, ptrValue, usSize);                                // read from the device to the return buffer
        #else
    fnSendSPIMessage(ucRead, sizeof(ucRead));                            // send immediately
    fnGetSPI_bytes(ptrValue, Size);                                      // fetch immediately
        #endif
    #else
    uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), Size);                 // the STR91XF uses a file system in FLASH with no access restrictions so we can simply copy the data
    #endif
#endif
}

#ifdef USE_PARAMETER_BLOCK

#define PARAMETER_BLOCK_1     PARAMETER_FLASH_START
#ifdef USE_PAR_SWAP_BLOCK
    #define PARAMETER_BLOCK_2 (PARAMETER_BLOCK_1 + PARAMETER_BLOCK_SIZE)
#endif
#define FLASH_SECTORS_IN_PAR_BLOCK (PARAMETER_BLOCK_SIZE/PAR_FLASH_GRANULARITY)

// The STR91XF has 8k or 64k byte blocks which can be individually modified so we use one of these, and a backup if desired
unsigned char fnGetValidPars(unsigned char ucValid)
{
    unsigned short usValidUse[2];
    unsigned short usCompare;

#ifdef SPI_FILE_SYSTEM
    fnWaitWriteComplete();                                               // if a write is in progress to the first sector we must wait to ensure we read the correct value
#endif

    // The first 2 word location defines the validity of the block. 0x5555 0x5555 means it is fully valid. 0xffff 0x5555 means it contains temporary unvalidated data
    fnGetParsFile(PARAMETER_BLOCK_1, (unsigned char*)usValidUse, sizeof(usValidUse));

    if (PRESENT_TEMP_BLOCK != ucValid) {
        usCompare = 0x5555;                                              // we are looking for validated pattern
    }
    else {
        usCompare = 0xffff;                                              // we are looking for temporary pattern
    }

    if ((usValidUse[0] == 0x5555) && (usValidUse[1] == usCompare)) {
        return 1;                                                        // first block matched request
    }
 #ifdef USE_PAR_SWAP_BLOCK
    else {
        fnGetParsFile(PARAMETER_BLOCK_2, (unsigned char*)usValidUse, sizeof(usValidUse));
        if ((usValidUse[0] == 0x5555) && (usValidUse[1] == usCompare)) {
            return 2;                                                    // second block matched request
        }
    }
 #endif
    return 0;                                                            // all other cases indicate the searched block doesn't exist
}


static int fnDeleteParBlock(unsigned char *ptrAddInPar)
{
    return (fnEraseFlashSector(ptrAddInPar, PARAMETER_BLOCK_SIZE));
/*
    int iSectors = FLASH_SECTORS_IN_PAR_BLOCK;

    ptrAddInPar = (unsigned char *)((unsigned long)ptrAddInPar & ~(PARAMETER_BLOCK_SIZE-1));

    while (iSectors--) {
        fnProtect((volatile unsigned short *)ptrAddInPar, CUI_UNPROTECT_LEVEL1_CONFIRM); // unprotect sector ready for delete
        if (fnEraseFlash((volatile unsigned short *)ptrAddInPar)) return 1;
        fnProtect((volatile unsigned short *)ptrAddInPar, CUI_PROTECT_LEVEL1_CONFIRM); // protect sector after use
        ptrAddInPar += PAR_FLASH_GRANULARITY;
    }
    return 0;*/
}

int fnGetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned char *ptrPar = PARAMETER_BLOCK_1;

#ifdef USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar += (PARAMETER_BLOCK_2 - PARAMETER_BLOCK_1);
    }
#endif

    ptrPar += (usParameterReference);
    ptrPar += 4;                                                         // first parameter starts after validation information

    while (usLength--) {
        fnGetParsFile(ptrPar, ucValue++, 1);
        ptrPar++;
    }
    return 0;
}

int fnSetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned char *ptrPar = PARAMETER_BLOCK_1, *ptrStart;
    unsigned short usValid = 0x5555;
    unsigned char  ucCheck;
    int iBlockUse = 0;

    if (TEMP_PARS & ucValidBlock) {
        ucValidBlock &= ~TEMP_PARS;
        iBlockUse = 1;
    }

    if (ucValidBlock & BLOCK_INVALID) {
        // No valid parameter blocks have been found so we can use the first for saving the data.
        // We delete it to be sure it is fresh
        fnDeleteParBlock(PARAMETER_BLOCK_1);
#ifdef USE_PAR_SWAP_BLOCK
        fnDeleteParBlock(PARAMETER_BLOCK_2);
#endif
        ucValidBlock = 1;
    }

#ifdef USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar += (PARAMETER_BLOCK_2 - PARAMETER_BLOCK_1);
    }
#endif
    ptrStart = ptrPar;
#ifndef _WINDOWS
    // It has been noted that often the first write to a new block causes the first word to be set to 0x00000000, which is unwanted and causes the file system and parameter blocks to fail.
    // The reason was not clear and the only explaination is that a write is being performed to the address before the first write is performed - however this could not be found.
    // It was then found that a write of 0xffffffff to the affected address cures the problem (would reverse the write of 0x00000000).
    // It is possible that this is a speciality of the chip, but this solution is alse hoped to be a painless cure...
    *(unsigned long *)ptrPar = 0xffffffff;
#endif
    ptrPar += (usParameterReference);
    ptrPar += 4;                                                         // first parameter starts after validation information

    if (!usLength) {
        if (iBlockUse) {                                                 // if we have a new set waiting, set it to use state
            iBlockUse = 0;
#ifdef USE_PAR_SWAP_BLOCK
            if (ptrStart == PARAMETER_BLOCK_1) {
                ptrPar = PARAMETER_BLOCK_2;
            }
            else {
                ptrPar = PARAMETER_BLOCK_1;
            }
            fnDeleteParBlock(ptrPar);
#endif
        }
        else {                                                           // we must delete the block
            fnDeleteParBlock(ptrStart);
            if (usParameterReference) return 0;                          // we don't validate it again
        }
    }
    else {
        fnProtect((volatile unsigned short *)ptrStart, CUI_UNPROTECT_LEVEL1_CONFIRM); // unprotect sector ready for writing
        while (usLength--) {
            fnGetParsFile(ptrPar, &ucCheck, 1);                          // get the present value to see whether it is already set or if it can not be modified
            if (ucCheck != *ucValue) {                                   // The value is different
                if ((ucCheck ^ *ucValue) & *ucValue)  {                  // we can not change a bit from 0 to 1
                    // we presently don't do anything in this case - but it is not possible set bits from 0 to 1.
                }
                else {
                    fnWriteBytesFlash(ptrPar, ucValue, 1);               // write each parameter to FLASH
                }
            }
            ucValue++;
            ptrPar++;
        }
        fnProtect((volatile unsigned short *)ptrStart, CUI_PROTECT_LEVEL1_CONFIRM); // protect sector after use
    }
    fnProtect((volatile unsigned short *)ptrStart, CUI_UNPROTECT_LEVEL1_CONFIRM);   // unprotect sector ready for writing

    // Now we must validate the parameter block which we have just written to
    fnWriteBytesFlash(ptrStart, (unsigned char *)&usValid, sizeof(usValid)); // validate data
    if (!iBlockUse) {
        fnWriteBytesFlash(ptrStart+sizeof(usValid), (unsigned char *)&usValid, sizeof(usValid)); // use data
    }
    fnProtect((volatile unsigned short *)ptrStart, CUI_PROTECT_LEVEL1_CONFIRM); // protect sector after use
    return 0;
}


#endif



#ifdef GLOBAL_HARDWARE_TIMER

static __interrupt void hwtimer_interrupt(void)
{
    static const unsigned char hw_timer_int_message[HEADER_LENGTH] = { INTERNAL_ROUTE, INTERNAL_ROUTE, TASK_TIMER, INTERRUPT_EVENT, 0 };
    TIM_CR2_2  = 0;                                                      // disable timer interrupts
    TIM_CR1_2  = 0;                                                      // disable further operation
    TIM_CNTR_2 = 0;
    TIM_SR_2   = 0;                                                      // reset interrupt flag

    iInterruptLevel = 1;                                                 // ensure interrupts remain blocked when putting message to queue
    fnWrite(INTERNAL_ROUTE, (unsigned char*)hw_timer_int_message, HEADER_LENGTH); // inform the global interrupt task
    iInterruptLevel = 0;                                                 // release
}


// In order to support a global hardware timer a milli-second timer is required. We use the 16 bit Timer 2 (which doesn't need to be power up since it is powered with the TICK timer 3)
//
extern CLOCK_LIMIT fnSetHardwareTimer(CLOCK_LIMIT *hw_delay)
{                                                                        // timer set in ms
    CLOCK_LIMIT Adjusted = 0;
    CLOCK_LIMIT NewDelay = (*hw_delay - 3);                              // clock count desired

    uDisable_Interrupt();                                                // protect region against interrupts

    if (TIM_CR1_2 & COUNTER_ENABLE) {                                    // timer already running
        CLOCK_LIMIT Remaining = (CLOCK_LIMIT)(TIM_OC2R_2 - TIM_CNTR_2);  // time remaining before next timeout (in clock pulses)
        if (NewDelay >= Remaining) {
            uEnable_Interrupt();                                         // enable interrupt and quit
            return 0;
        }
        TIM_CR1_2 = 0;                                                   // freeze the timer and disable interrupt
        Adjusted = (CLOCK_LIMIT)TIM_CNTR_2;                              // note the frozen value
    }
    TIM_OC2R_2 = NewDelay;                                               // set reference
    #if MILLISEC == LOW_RES_MS
    TIM_CR2_2  = (CLOCK_DIV_256 | OC2IE);                                // PCLK /256, enable interrupt on match
    #elif MILLISEC == MED_RES_MS
    TIM_CR2_2  = (CLOCK_DIV_64 | OC2IE);                                 // PCLK / 64, enable interrupt on match
    #else
    TIM_CR2_2  = (CLOCK_DIV_32 | OC2IE);                                 // PCLK / 32, enable interrupt on match
    #endif
    TIM_CR1_2 = (COUNTER_ENABLE);                                        // enable timer

    uEnable_Interrupt();                                                 // enable interrupts again

    return Adjusted;
}
#endif

static void ( *wui_handler[30] )( void ) = {0};                          // global handlers
static void ( *wui_channel_handler[4] )( void ) = {0};                   // channel specific handlers

// Wake-up port global interrupt handler
//
__interrupt static void WUIInterrupt(void)
{
    unsigned long ulInterruptSource = (WIU_PR & WIU_MR);                 // pending interrupts
    do {
        int iSource = 0;
        unsigned long ulSingleSource = 0x00000004;                       // first possible interrupt source
        if (!(ulInterruptSource & 0x0000fffc)) {                         // check to see whether interrupts are waiting in lower half of register
            ulSingleSource = 0x00010000;
            iSource = 14;                                                // just some to improve efficiency
        }
        while (ulInterruptSource != 0) {
            while ((ulSingleSource & ulInterruptSource) == 0) {          // search for next interrupt source
                ulSingleSource <<= 1;
                iSource++;
            }
            WIU_PR = ulSingleSource;                                     // clear interrupt for this source
#ifdef _WINDOWS
            WIU_PR &= ~ulSingleSource;
#endif
            if (wui_handler[iSource] != 0) {                             // if there is a handler entered
                iInterruptLevel = 1;
                wui_handler[iSource]();                                  // call handler
                iInterruptLevel = 0;
            }
            ulInterruptSource &= ~ulSingleSource;
        }
    } while ((ulInterruptSource = (WIU_PR & WIU_MR)) != 0);
}

// Channel specific handlers
__interrupt static void WUIInterrupt_channel_0(void)
{
    WIU_PR = 0x000000ff;                                                 // clear interrupt for this channel source
    if (wui_channel_handler[0] != 0) {                                   // if there is a handler entered
        iInterruptLevel = 1;
        wui_channel_handler[0]();                                        // call handler
        iInterruptLevel = 0;
    }
}

__interrupt static void WUIInterrupt_channel_1(void)
{
    WIU_PR = 0x0000ff00;                                                 // clear interrupt for this channel source
    if (wui_channel_handler[1] != 0) {                                   // if there is a handler entered
        iInterruptLevel = 1;
        wui_channel_handler[1]();                                        // call handler
        iInterruptLevel = 0;
    }
}

__interrupt static void WUIInterrupt_channel_2(void)
{
    WIU_PR = 0x00ff0000;                                                 // clear interrupt for this channel source
    if (wui_channel_handler[2] != 0) {                                   // if there is a handler entered
        iInterruptLevel = 1;
        wui_channel_handler[2]();                                        // call handler
        iInterruptLevel = 0;
    }
}

__interrupt static void WUIInterrupt_channel_3(void)
{
    WIU_PR = 0xff000000;                                                 // clear interrupt for this channel source
    if (wui_channel_handler[3] != 0) {                                   // if there is a handler entered
        iInterruptLevel = 1;
        wui_channel_handler[3]();                                        // call handler
        iInterruptLevel = 0;
    }
}

// Configure a specific interrupt, including processor specific settings and enter a handler routine.
// Some specific peripheral control may be performed here.
//
extern void fnConfigureInterrupt(void *ptrSettings)
{
    switch (((INTERRUPT_SETUP *)ptrSettings)->int_type) {
    case PORT_CHANNEL_INTERRUPT:                                         // assign interrpt to one particular channel (only one per channel should be defined). Generally not mixed with global interrupst.
    case PORT_INTERRUPT:                                                 // define global port interrupt
        {
            int iChannel = 0;
            unsigned long ulWakeupMask = (0x00000001 << (((INTERRUPT_SETUP *)ptrSettings)->int_port_bit));
            unsigned char ucInterruptNumber = ((((INTERRUPT_SETUP *)ptrSettings)->int_port_bit) - EXINT_2);            
            unsigned char ucChannelNumber = ucInterruptNumber;
            if (ulWakeupMask & 0xff000000) {
                iChannel = 3;
                ucChannelNumber -= 24;
            }
            else if (ulWakeupMask & 0x00ff0000) {
                iChannel = 2;
                ucChannelNumber -= 16;
            }
            else if (ulWakeupMask & 0x0000ff00) {
                iChannel = 1;
                ucChannelNumber -= 8;
            }
            if ((((INTERRUPT_SETUP *)ptrSettings)->int_handler) != 0) {
                SCU_PCGR1 |= WIU;                                        // ensure wake-up unit powered up
                SCU_PRR1 |= WIU;                                         // and not in reset
                if ((((INTERRUPT_SETUP *)ptrSettings)->int_type) == PORT_CHANNEL_INTERRUPT) {
                    wui_channel_handler[iChannel] = (((INTERRUPT_SETUP *)ptrSettings)->int_handler);
                    SCU_WKUPSEL &= ~(0x07 << (iChannel * 3));            // mask out channel in wakeup select register
                    SCU_WKUPSEL |= (ucChannelNumber << (iChannel * 3));
                    switch (iChannel) {
                    case 0:
                        VIC1_VA10R  = (unsigned long)WUIInterrupt_channel_0; // enter channel specific interrupt handling routine
                        VIC1_INTER |= VECTOR_10;
                        VIC1_INTSR &= ~VECTOR_10;
                        VIC1_VC10R  = (ENABLE_INTERRUPT | WIU_GROUP0_INT_1);
                        break;
                    case 1:
                        VIC1_VA11R  = (unsigned long)WUIInterrupt_channel_1; // enter channel specific interrupt handling routine
                        VIC1_INTER |= VECTOR_11;
                        VIC1_INTSR &= ~VECTOR_11;
                        VIC1_VC11R  = (ENABLE_INTERRUPT | WIU_GROUP1_INT_1);
                        break;
                    case 2:
                        VIC1_VA12R  = (unsigned long)WUIInterrupt_channel_2; // enter channel specific interrupt handling routine
                        VIC1_INTER |= VECTOR_12;
                        VIC1_INTSR &= ~VECTOR_12;
                        VIC1_VC12R  = (ENABLE_INTERRUPT | WIU_GROUP2_INT_1);
                        break;
                    case 3:
                        VIC1_VA13R  = (unsigned long)WUIInterrupt_channel_3; // enter channel specific interrupt handling routine
                        VIC1_INTER |= VECTOR_13;
                        VIC1_INTSR &= ~VECTOR_13;
                        VIC1_VC13R  = (ENABLE_INTERRUPT | WIU_GROUP3_INT_1);
                        break;
                    }
                }
                else {
                    wui_handler[ucInterruptNumber] = (((INTERRUPT_SETUP *)ptrSettings)->int_handler);
                    VIC1_VA9R   = (unsigned long)WUIInterrupt;           // enter global interrupt handling routine
                    VIC1_INTER |= VECTOR_9;
                    VIC1_INTSR &= ~VECTOR_9;
                    VIC1_VC9R   = (ENABLE_INTERRUPT | WIU_ALL_INT_1);
                }
                if (((INTERRUPT_SETUP *)ptrSettings)->int_port_sense == WUI_RISING_EDGE) {
                   WIU_TR |= ulWakeupMask;                               // set interrupt on rising edge 
                }
                else {
                   WIU_TR &= ~ulWakeupMask;                              // set interrupt on falling edge 
                }
                WIU_CTRL  |= WIUINT_EN;                                  // enable global wake up interrupt
                WIU_MR    |= ulWakeupMask;                               // unmask the specified input
            }
            else {
                WIU_MR &= ~ulWakeupMask;                                 // mask the specified input
                if ((((INTERRUPT_SETUP *)ptrSettings)->int_type) == PORT_CHANNEL_INTERRUPT) {
                    wui_channel_handler[iChannel] = 0;
                    VIC1_INTECR = (VECTOR_10 << iChannel);               // disable interrupt
                }
                else {
                    wui_handler[ucInterruptNumber] = 0;
                }
            }
            if (WIU_MR == 0) {                                           // no more wake up interrupts programmed
                VIC1_INTECR = VECTOR_9;
                WIU_CTRL &= ~WIUINT_EN;
                SCU_PRR1 &= ~WIU;                                        // reset
                SCU_PCGR1 &= ~WIU;                                       // power down
            }
        }
        break;

    case TIMER_FREQUENCY:
        if ((((TIMER_INTERRUPT_SETUP *)ptrSettings)->timer_reference) < 2) {
            SCU_PCGR1 |= TIM01;                                          // enable clocks to timer module
            SCU_PRR1  |= TIM01;                                          // take timer module out of reset
        }
        else {
            SCU_PCGR1 |= TIM23;                                          // enable clocks to timer module
            SCU_PRR1  |= TIM23;                                          // take timer module out of reset
        }

        TIM_CR2_1  = CLOCK_DIV_1;
        TIM_OC2R_1 = ((((TIMER_INTERRUPT_SETUP *)ptrSettings)->timer_value) - 5);// set the frequency
        TIM_OC1R_1 = (TIM_OC2R_0/2);                                     // 50/50 mark/space
        TIM_CR1_1  = (OLVL1_HIGH | OLVL2_LOW | OC2E | PWM | COUNTER_ENABLE);
        SCU_GPIOOUT6 |= (ALT_OUTPUT_2_FUNCTION << (2*3));                // P6.3 is output
        break;

    case TIMER_INTERRUPT:
        break;
    }
}


// This routine switches to low power mode. It is always called with disabled interrupts
// Warning - do not use with chip revisions less that FA!!
// Due to chip bugs this mode is not recommended unless the possible interrupt sources are synchronised to this call.
// An interrupt between the idle command and the idle state being entered will cause the device to hang!!
//
extern void fnDoLowPower(void)
{
#ifndef _WINDOWS
    volatile int iLoop = ((MASTER_CLOCK / CRYSTAL_FREQ)*12);             // overkill to be safe
    SCU_MGR0 = SCU_PCGR0;                                                // set idle mode mask to same setting as main clock mask
    SCU_MGR1 = SCU_PCGR1;
    SCU_PWRMNG = PWR_MODE_IDLE;                                          // disable the processor clock to enter idle mode
                                                                         // the processor runs for about 12 oscillator cycles before entering the low power mode
                                                                         // the number of instructions that can be executed is the (master clock / oscillator)
                                                                         // for 96MHz operation and 12MHz crystal this would be about 96 instructions
    uEnable_Interrupt();                                                 // allow wakeup on interrupts
    while (--iLoop != 0) {
    }

#else
    uEnable_Interrupt();                                                 // allow wakeup on interrupts
#endif
}

// This routine is called to reset the card - it uses the Watchdog
//
extern void fnResetBoard(void)
{
    WDG_PR = 0;                                                          // set shortest watchdog time and enable watchdog in case not alread enabled
    WDG_VR = 0;
    WDG_CR = WATCHDOG_WE;
    WDG_KR = WDT_FEED1;
    WDG_KR = WDT_FEED2;
}


#if defined (_GNU)
extern unsigned char __data_start__, __data_end__;
extern const unsigned char __data_load_start__;
extern unsigned char __text_start__, __text_end__;
extern const unsigned char __text_load_start__;
extern unsigned char __bss_start__, __bss_end__;

extern void __init_gnu_data(void)
{
    unsigned char *ptrData = &__data_start__;
    const unsigned char *ptrFlash = &__data_load_start__;
    unsigned long ulInitDataLength = (&__data_end__ - &__data_start__);
    while (ulInitDataLength--) {                                         // initialise data
        *ptrData++ = *ptrFlash++;
    }

    ptrData = &__text_start__;
    ptrFlash = &__text_load_start__;
    if (ptrData != ptrFlash) {                                           // only when copy is needed
        ulInitDataLength = (&__text_end__ - &__text_start__);
        while (ulInitDataLength--) {                                     // initialise text
            *ptrData++ = *ptrFlash++;
        }
    }

    ptrData = &__bss_start__;
    ulInitDataLength = (&__bss_end__ - &__bss_start__);
    while (ulInitDataLength--) {                                         // initialise bss
        *ptrData++ = 0;
    }
}
#endif


// Basic hardware initialisation of specific hardware
//
INITHW void fnInitHW(void)                                               // perform hardware initialisation
{
#ifdef _WINDOWS
    unsigned char ucPortPullups[] = {
        GPIO_DEFAULT_INPUT_0,                                            // set the port states out of reset in the project file app_hw_str91xf.h
        GPIO_DEFAULT_INPUT_1,
        GPIO_DEFAULT_INPUT_2,
        GPIO_DEFAULT_INPUT_3,
        GPIO_DEFAULT_INPUT_4,
        GPIO_DEFAULT_INPUT_5,
        GPIO_DEFAULT_INPUT_6,
        GPIO_DEFAULT_INPUT_7,
        GPIO_DEFAULT_INPUT_8,
        GPIO_DEFAULT_INPUT_9
    };
  #ifdef RANDOM_NUMBER_GENERATOR
    static unsigned short usRandomSeed = 0;
    ptrSeed = &usRandomSeed;
  #endif
    fnInitialiseDevice((void *)ucPortPullups);
#endif

#if MASTER_CLOCK >= 75000000
    fnSetFMI(FCR_WAIT_STATES_2 | FCR_PWD | FCR_BUSCFG);                  // configure FLASH for speeds of 75MHz and above
#elif MASTER_CLOCK > 66000000
    fnSetFMI(FCR_WAIT_STATES_1 | FCR_PWD | FCR_BUSCFG);                  // configure FLASH for speeds above 66MHz
#else
    fnSetFMI(FCR_WAIT_STATES_1 | FCR_PWD);                               // configure FLASH for speeds of 66MHz and below
#endif

    SCU_PLLCONF = (PLL_SETTING | PLL_ENABLE);                            // enable the PLL with required setting
    while (!(SCU_SYSSTATUS & PLL_LOCK)) {}                               // wait for the PLL to lock before continuing
#if RCLK > 48000000
    SCU_CLKCNTR |= APBDIV_2;                                             // PCLK is RCLK/2 when RCLK is > 48MHz
#endif
    SCU_CLKCNTR &= ~(MCLKSEL_OSC | MCLKSEL_RTC);                         // switch to PLL clock and BRCLK = MASTER/2

    SCU_SCR0 &= ~(WSR_AHB | WSR_DTCM);                                   // remove wait states from SRAM
    SCU_SCR0 |= EN_PFQBC;                                                // ensure FLASH bypass enabled

#if defined (_GNU)
    __init_gnu_data();                                                   // initialise variables
#endif

    SCU_PCGR1 |= TICK_TIMER;                                             // enable clocks to timer module
    SCU_PRR1  |= TICK_TIMER;                                             // take timer module out of reset
    SCU_PCGR0 |= VIC;                                                    // enable clocks to vector interrupt controller - do this early to ensure that it is ready when configured in timer set up
    SCU_PRR0  |= VIC;                                                    // take vector interrupt controller out of reset

    INIT_WATCHDOG_LED();                                                 // allow user configuration of a blink LED
    INIT_WATCHDOG_DISABLE();                                             // initialise ready for checking of watchdog diabled
    if (!WATCHDOG_DISABLE()) {
        SCU_PCGR1 |= (WATCHDOG);                                         // enable clocks to watchdog
        SCU_PRR1  |= (WATCHDOG);                                         // take watchdog module out of reset
        CONFIGURE_WATCHDOG();
    }
    fnUserHWInit();                _SIM_PORT_CHANGE                      // allow the user to initialise hardware specific things

#ifndef SPI_INTERFACE
    #ifdef SPI_FILE_SYSTEM
    fnConfigSPIFileSystem(SPI_2MEG);                                     // configure SPI interface for SPI file system and 2M speed
    #endif
#endif
}



#endif
