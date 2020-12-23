/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      stm32_DMA.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2019
    *********************************************************************
    21.06.2019 Correct DMA interrupt flag position calculation           {1}
    08.07.2019 Clear possible pending interrupts when a DMA interrupt callback is entered {2}

*/

#if defined _DMA_SHARED_CODE

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static __interrupt void _DMA_Interrupt_0(void);
static __interrupt void _DMA_Interrupt_1(void);
static __interrupt void _DMA_Interrupt_2(void);
static __interrupt void _DMA_Interrupt_3(void);
#if DMA_CHANNEL_COUNT > 4
    static __interrupt void _DMA_Interrupt_4(void);
#endif
#if DMA_CHANNEL_COUNT > 5
    static __interrupt void _DMA_Interrupt_5(void);
#endif
#if DMA_CHANNEL_COUNT > 6
    static __interrupt void _DMA_Interrupt_6(void);
#endif
#if DMA_CHANNEL_COUNT > 7
    static __interrupt void _DMA_Interrupt_7(void);
#endif
#if DMA_CHANNEL_COUNT > 8
    static __interrupt void _DMA_Interrupt_8(void);
#endif
#if DMA_CHANNEL_COUNT > 9
    static __interrupt void _DMA_Interrupt_9(void);
#endif
#if DMA_CHANNEL_COUNT > 10
    static __interrupt void _DMA_Interrupt_10(void);
#endif
#if DMA_CHANNEL_COUNT > 11
    static __interrupt void _DMA_Interrupt_11(void);
#endif
#if DMA_CHANNEL_COUNT > 12
    static __interrupt void _DMA_Interrupt_12(void);
#endif
#if DMA_CHANNEL_COUNT > 13
    static __interrupt void _DMA_Interrupt_13(void);
#endif
#if DMA_CHANNEL_COUNT > 14
    static __interrupt void _DMA_Interrupt_14(void);
#endif
#if DMA_CHANNEL_COUNT > 15
    static __interrupt void _DMA_Interrupt_15(void);
#endif


/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

static const unsigned char *_DMA_Interrupt[DMA_CHANNEL_COUNT] = {
    (unsigned char *)_DMA_Interrupt_0,
    (unsigned char *)_DMA_Interrupt_1,
    (unsigned char *)_DMA_Interrupt_2,
    (unsigned char *)_DMA_Interrupt_3,
#if DMA_CHANNEL_COUNT > 4
    (unsigned char *)_DMA_Interrupt_4,
#endif
#if DMA_CHANNEL_COUNT > 5
    (unsigned char *)_DMA_Interrupt_5,
#endif
#if DMA_CHANNEL_COUNT > 6
    (unsigned char *)_DMA_Interrupt_6,
#endif
#if DMA_CHANNEL_COUNT > 7
    (unsigned char *)_DMA_Interrupt_7,
#endif
#if DMA_CHANNEL_COUNT > 8
    (unsigned char *)_DMA_Interrupt_8,
#endif
#if DMA_CHANNEL_COUNT > 9
    (unsigned char *)_DMA_Interrupt_9,
#endif
#if DMA_CHANNEL_COUNT > 10
    (unsigned char *)_DMA_Interrupt_10,
#endif
#if DMA_CHANNEL_COUNT > 11
    (unsigned char *)_DMA_Interrupt_11,
#endif
#if DMA_CHANNEL_COUNT > 12
    (unsigned char *)_DMA_Interrupt_12,
#endif
#if DMA_CHANNEL_COUNT > 13
    (unsigned char *)_DMA_Interrupt_13,
#endif
#if DMA_CHANNEL_COUNT > 14
    (unsigned char *)_DMA_Interrupt_14,
#endif
#if DMA_CHANNEL_COUNT > 15
    (unsigned char *)_DMA_Interrupt_15,
#endif
};

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

// Interrupt handlers for DMA operation
// - either a complete or a half buffer has been completed
//
static void (*_DMA_handler[DMA_CHANNEL_COUNT])(void) = {0};              // user DMA interrupt handlers
static unsigned short usDMA_Length[DMA_CHANNEL_COUNT] = {0};

/* =================================================================== */
/*                       DMA Interrupt Handlers                        */
/* =================================================================== */

static void _DMA_Handler(int iChannel)
{
    if (_DMA_handler[iChannel] != 0) {                                   // if there is a user handler
        uDisable_Interrupt();
            _DMA_handler[iChannel]();                                    // call user handling function
        uEnable_Interrupt();
    }
}


static __interrupt void _DMA_Interrupt_0(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_LIFCR, DMA_LIFCR_TCIF0);                     // clear the DMA interrupt (DMA1 - stream 0)
        #if defined _WINDOWS
    DMA1_LISR &= ~(DMA_LISR_TCIF0);
        #endif
    #endif
    _DMA_Handler(0);
}

static __interrupt void _DMA_Interrupt_1(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_LIFCR, DMA_LIFCR_TCIF1);                     // clear the DMA interrupt (DMA1 - stream 1)
        #if defined _WINDOWS
    DMA1_LISR &= ~(DMA_LISR_TCIF1);
        #endif
    #endif
    _DMA_Handler(1);
}

static __interrupt void _DMA_Interrupt_2(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_LIFCR, DMA_LIFCR_TCIF2);                     // clear the DMA interrupt (DMA1 - stream 2)
        #if defined _WINDOWS
    DMA1_LISR &= ~(DMA_LISR_TCIF2);
        #endif
    #endif
    _DMA_Handler(2);
}

static __interrupt void _DMA_Interrupt_3(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_LIFCR, DMA_LIFCR_TCIF3);                     // clear the DMA interrupt (DMA1 - stream 3)
        #if defined _WINDOWS
    DMA1_LISR &= ~(DMA_LISR_TCIF3);
        #endif
    #endif
    _DMA_Handler(3);
}
#if DMA_CHANNEL_COUNT > 4
static __interrupt void _DMA_Interrupt_4(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_HIFCR, DMA_HIFCR_TCIF4);                     // clear the DMA interrupt (DMA1 - stream 4)
        #if defined _WINDOWS
    DMA1_HISR &= ~(DMA_HISR_TCIF4);
        #endif
    #endif
    _DMA_Handler(4);
}
#endif
#if DMA_CHANNEL_COUNT > 5
static __interrupt void _DMA_Interrupt_5(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_HIFCR, DMA_HIFCR_TCIF5);                     // clear the DMA interrupt (DMA1 - stream 5)
        #if defined _WINDOWS
    DMA1_HISR &= ~(DMA_HISR_TCIF5);
        #endif
    #endif
    _DMA_Handler(5);
}
#endif
#if DMA_CHANNEL_COUNT > 6
static __interrupt void _DMA_Interrupt_6(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_HIFCR, DMA_HIFCR_TCIF6);                     // clear the DMA interrupt (DMA1 - stream 6)
        #if defined _WINDOWS
    DMA1_HISR &= ~(DMA_HISR_TCIF6);
        #endif
    #endif
    _DMA_Handler(6);
}
#endif
#if DMA_CHANNEL_COUNT > 7
static __interrupt void _DMA_Interrupt_7(void)
{
    #if defined DMA1_LIFCR
    WRITE_ONE_TO_CLEAR(DMA1_HIFCR, DMA_HIFCR_TCIF7);                     // clear the DMA interrupt (DMA1 - stream 7)
        #if defined _WINDOWS
    DMA1_HISR &= ~(DMA_HISR_TCIF7);
        #endif
    #endif
    _DMA_Handler(7);
}
#endif
#if DMA_CHANNEL_COUNT > 8
static __interrupt void _DMA_Interrupt_8(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_LIFCR, DMA_LIFCR_TCIF0);                     // clear the DMA interrupt (DMA2 - stream 0)
        #if defined _WINDOWS
    DMA2_LISR &= ~(DMA_LISR_TCIF0);
        #endif
    #endif
    _DMA_Handler(8);
}
#endif
#if DMA_CHANNEL_COUNT > 9
static __interrupt void _DMA_Interrupt_9(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_LIFCR, DMA_LIFCR_TCIF1);                     // clear the DMA interrupt (DMA2 - stream 1)
        #if defined _WINDOWS
    DMA2_LISR &= ~(DMA_LISR_TCIF1);
        #endif
    #endif
    _DMA_Handler(9);
}
#endif
#if DMA_CHANNEL_COUNT > 10
static __interrupt void _DMA_Interrupt_10(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_LIFCR, DMA_LIFCR_TCIF2);                     // clear the DMA interrupt (DMA2 - stream 2)
        #if defined _WINDOWS
    DMA2_LISR &= ~(DMA_LISR_TCIF2);
        #endif
    #endif
    _DMA_Handler(10);
}
#endif
#if DMA_CHANNEL_COUNT > 11
static __interrupt void _DMA_Interrupt_11(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_LIFCR, DMA_LIFCR_TCIF3);                     // clear the DMA interrupt (DMA2 - stream 3)
        #if defined _WINDOWS
    DMA2_LISR &= ~(DMA_LISR_TCIF3);
        #endif
    #endif
    _DMA_Handler(11);
}
#endif
#if DMA_CHANNEL_COUNT > 12
static __interrupt void _DMA_Interrupt_12(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_HIFCR, DMA_HIFCR_TCIF4);                     // clear the DMA interrupt (DMA2 - stream 4)
        #if defined _WINDOWS
    DMA2_HISR &= ~(DMA_HISR_TCIF4);
        #endif
    #endif
    _DMA_Handler(12);
}
#endif
#if DMA_CHANNEL_COUNT > 13
static __interrupt void _DMA_Interrupt_13(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_HIFCR, DMA_HIFCR_TCIF5);                     // clear the DMA interrupt (DMA2 - stream 5)
        #if defined _WINDOWS
    DMA2_HISR &= ~(DMA_HISR_TCIF5);
        #endif
    #endif
    _DMA_Handler(13);
}
#endif
#if DMA_CHANNEL_COUNT > 14
static __interrupt void _DMA_Interrupt_14(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_HIFCR, DMA_HIFCR_TCIF6);                     // clear the DMA interrupt (DMA2 - stream 6)
        #if defined _WINDOWS
    DMA2_HISR &= ~(DMA_HISR_TCIF6);
        #endif
    #endif    
    _DMA_Handler(14);
}
#endif
#if DMA_CHANNEL_COUNT > 15
static __interrupt void _DMA_Interrupt_15(void)
{
    #if defined DMA2_LIFCR
    WRITE_ONE_TO_CLEAR(DMA2_HIFCR, DMA_HIFCR_TCIF7);                     // clear the DMA interrupt (DMA2 - stream 7)
        #if defined _WINDOWS
    DMA2_HISR &= ~(DMA_HISR_TCIF7);
        #endif
    #endif
    _DMA_Handler(15);
}
#endif

#if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
static void fnClearChannelInterruptFlags(unsigned long ulDmaTriggerSource, unsigned char ucFlags)
{
    static const unsigned char ucClearFlagPosition[4] = {0, 6, 16, 22};
    STM32_DMA *ptrDMA_controller;
    unsigned long _ulDmaTriggerSource = (ulDmaTriggerSource & 0x7);
    volatile unsigned long *ptrDLA_flag;
    unsigned long ulFlags;
    if ((DMA_CONTROLLER_REF_2 & ulDmaTriggerSource) != 0) {
        ptrDMA_controller = (STM32_DMA *)DMA2_BLOCK;
    }
    else {
        ptrDMA_controller = (STM32_DMA *)DMA1_BLOCK;
    }
    if ((ulDmaTriggerSource & 0x7) >= 4) {
        ptrDLA_flag = &ptrDMA_controller->DMA_HIFCR;                     // high interrupt flag clear register
        _ulDmaTriggerSource -= 4;
    }
    else {
        ptrDLA_flag = &ptrDMA_controller->DMA_LIFCR;                     // low interrupt flag clear register
    }
    ulFlags = (ucFlags << ucClearFlagPosition[_ulDmaTriggerSource]);     // {1}
    *ptrDLA_flag = ulFlags;                                              // clear the flag
    #if defined _WINDOWS
    *ptrDLA_flag = 0;
    *(ptrDLA_flag - 2) &= ~(ulFlags);
    #endif
}
#endif

// Buffer source to fixed destination address or fixed source address to buffer (beware that only DMA controller 2 can perform memory to memory transfers)
//
extern int fnConfigDMA_buffer(unsigned long ulDmaTriggerSource, unsigned long ulBufLength, void *ptrBufSource, void *ptrBufDest, unsigned long ulRules, void (*int_handler)(void), int int_priority)
{
    unsigned char ucSize = (unsigned char)(ulRules & 0x07);              // transfer size 1, 2 or 4 bytes
#if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    unsigned long ulTransferType;
    STM32_DMA_STREAM *ptrDMAstream;
#endif
    STM32_DMA *ptrDMA_controller;
    int iStream = (ulDmaTriggerSource & 0x7);
    int iIntChannel = iStream;
    int iInterruptID;
    if ((ulDmaTriggerSource & DMA_CONTROLLER_REF_2) != 0) {              // set a pointer to the DMA controller to be used
        POWER_UP(AHB1, RCC_AHB1ENR_DMA2EN);                              // ensure DMA controller is enabled
        ptrDMA_controller = (STM32_DMA *)DMA2_BLOCK;
        if (iStream >= 5) {
            iInterruptID = (irq_DMA2_Stream5_ID + (iStream - 5));
        }
        else {
            iInterruptID = (irq_DMA2_Stream0_ID + iStream);
        }
        iIntChannel += 8;
    }
    else {
        POWER_UP(AHB1, RCC_AHB1ENR_DMA1EN);                              // ensure DMA controller is enabled
        ptrDMA_controller = (STM32_DMA *)DMA1_BLOCK;
        if (iStream == 7) {
            iInterruptID = irq_DMA1_Stream7_ID;
        }
        else {
            iInterruptID = (irq_DMA1_Stream0_ID + iStream);
        }
    }
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    ptrDMAstream = &ptrDMA_controller->DMA_stream[iStream];              // select the stream registers to be used
    ptrDMAstream->DMA_SxCR = 0;                                          // disable stream
    ulTransferType = (DMA_SxCR_CHSEL_7 & ulDmaTriggerSource);
    if ((ulRules & DMA_DIRECTION_OUTPUT) != 0) {                         // peripheral destination
        ptrDMAstream->DMA_SxM0AR = (unsigned long)ptrBufSource;          // address of memory source
        ptrDMAstream->DMA_SxPAR = (unsigned long)ptrBufDest;             // address of peripheral destination
        ulTransferType |= (DMA_SxCR_MINC | DMA_SxCR_DIR_M2P);
    }
    else {
        ptrDMAstream->DMA_SxPAR = (unsigned long)ptrBufSource;           // address of peripheral source
        ptrDMAstream->DMA_SxM0AR = (unsigned long)ptrBufDest;            // address of memory destination
        ulTransferType |= (DMA_SxCR_MINC | DMA_SxCR_DIR_P2M);
    }
    ptrDMAstream->DMA_SxNDTR = usDMA_Length[iIntChannel] = (unsigned short)(ulBufLength / ucSize); // the number of transfers to be performed
    switch (ucSize) {
    case 1:
        ulTransferType |= (DMA_SxCR_PSIZE_8 | DMA_SxCR_MSIZE_8 | DMA_SxCR_PL_HIGH); // set up DMA operation for byte transfer
        break;
    case 2:
        ulTransferType |= (DMA_SxCR_PSIZE_16 | DMA_SxCR_MSIZE_16 | DMA_SxCR_PL_HIGH); // set up DMA operation for short word transfer
        break;
    case 4:
        ulTransferType |= (DMA_SxCR_PSIZE_32 | DMA_SxCR_MSIZE_32 | DMA_SxCR_PL_HIGH); // set up DMA operation for long word transfer
        break;
    }
    #else
    DMA_CNDTR_MEMCPY = ((unsigned long)ulBufLength);                     // the number of byte transfers to be made (max 0xffff)
    DMA_CMAR_MEMCPY  = (unsigned long)ptrBufSource;                      // address of first byte to be transferred
    DMA_CPAR_MEMCPY  = (unsigned long)ptrBufDest;                        // address of first destination byte
    DMA_CCR_MEMCPY   = (DMA1_CCR1_EN | DMA1_CCR1_PINC | DMA1_CCR1_MINC | DMA1_CCR1_PSIZE_8 | DMA1_CCR1_MSIZE_8 | DMA1_CCR1_PL_MEDIUM | DMA1_CCR1_MEM2MEM | DMA1_CCR1_DIR); // set up DMA operation and start DMA transfer
    #endif
    if (int_handler != 0) {
        _DMA_handler[iIntChannel] = int_handler;                         // enter the user interrupt callback
        fnEnterInterrupt(iInterruptID, int_priority, (void(*)(void))_DMA_Interrupt[iIntChannel]); // enter interrupt handler for the DMA controller stream
        // Ensure pending interrupt is cleared
        //
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
        fnClearChannelInterruptFlags(ulDmaTriggerSource, (DMA_LIFCR_TCIF0 | DMA_LIFCR_HTIF0 | DMA_LIFCR_TEIF0 | DMA_LIFCR_FEIFO0 | DMA_LIFCR_DMEIF0)); // {2}
    #endif
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
        if ((ulRules & DMA_HALF_BUFFER_INTERRUPT) != 0) {
            ulTransferType |= DMA_SxCR_HTIE;                             // enable interrupt on half-transfer completion
        }
        else {
            ulTransferType |= DMA_SxCR_TCIE;                             // enable interrupt on completion
        }
    #endif
    }
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    ptrDMAstream->DMA_SxCR = ulTransferType;
    #endif

    // Note that the DMA channel has not been activated yet - to do this fnDMA_BufferReset(channel_number, DMA_BUFFER_START); is performed
    //
    return 0;
}

extern STM32_DMA_STREAM *fnGetDMA_stream(unsigned long ulDmaTriggerSource)
{
    STM32_DMA *ptrDMA_controller;
    int iStream = (ulDmaTriggerSource & 0x7);
    if ((ulDmaTriggerSource & DMA_CONTROLLER_REF_2) != 0) {              // set a pointer to the DMA controller to be used
        ptrDMA_controller = (STM32_DMA *)DMA2_BLOCK;
    }
    else {
        ptrDMA_controller = (STM32_DMA *)DMA1_BLOCK;
    }
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    return (&ptrDMA_controller->DMA_stream[iStream]);                    // return the stream register pointer
    #else
    return 0;
    #endif
}


extern void fnDMA_BufferReset(unsigned long ulDmaTriggerSource, int iAction)
{
    STM32_DMA_STREAM *ptrDMAstream = fnGetDMA_stream(ulDmaTriggerSource);// select the stream registers to be used
    switch (iAction) {
    case DMA_BUFFER_START_FINISH:                                        // start a prepared transfer with software trigger and return only after the transfer has compeleted
        break;
    case DMA_BUFFER_START:
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
        fnClearChannelInterruptFlags(ulDmaTriggerSource, (DMA_LIFCR_TCIF0 | DMA_LIFCR_HTIF0 | DMA_LIFCR_TEIF0 | DMA_LIFCR_FEIFO0 | DMA_LIFCR_DMEIF0)); // ensure all flags are clear (the DMA may not start if old flags are not cleared)
        ptrDMAstream->DMA_SxCR |= DMA_SxCR_EN;                           // enable the channel's operation
        #if defined _WINDOWS
        if ((DMA_CONTROLLER_REF_2 & ulDmaTriggerSource) != 0) {
            iDMA |= (DMA_CONTROLLER_8 << (ulDmaTriggerSource & 0x7));
        }
        else {
            iDMA |= (DMA_CONTROLLER_0 << (ulDmaTriggerSource & 0x7));
        }
        #endif
    #endif
        break;
    case DMA_BUFFER_RESET:                                               // reset the DMA back to the start of the present buffer
    case DMA_BUFFER_RESTART:                                             // reset and start again
        {
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
        #if defined _WINDOWS
            int iSize = 1;                                               // default is single byte size
            register unsigned short usTransferRemaining;
        #endif
            int iChannelIndex = (ulDmaTriggerSource & 0x7);
            if ((ulDmaTriggerSource & DMA_CONTROLLER_REF_2) != 0) {
                iChannelIndex += 8;
            }
            ptrDMAstream->DMA_SxCR &= ~DMA_SxCR_EN;                      // disable DMA operation on the channel
        #if defined _WINDOWS                                             // temporary - the pointer in the DMA controller doesn't change in hardware but the simulator presently does...
            usTransferRemaining = (unsigned short)ptrDMAstream->DMA_SxNDTR; // the number of transfers that haven't been performed before stopping
            if ((ptrDMAstream->DMA_SxCR & DMA_SxCR_MSIZE_32) == DMA_SxCR_MSIZE_16) {
                iSize = 2;                                               // transfer size is half-words
            }
            else if ((ptrDMAstream->DMA_SxCR & DMA_SxCR_MSIZE_32) == DMA_SxCR_MSIZE_32) {
                iSize = 4;                                               // transfer size is long word
            }
            ptrDMAstream->DMA_SxM0AR += (usTransferRemaining * iSize);   // project to the end of the transfer that is remaining
            ptrDMAstream->DMA_SxM0AR -= (usDMA_Length[iChannelIndex] * iSize); // set back to start of the input buffer
        #endif
            ptrDMAstream->DMA_SxNDTR = usDMA_Length[iChannelIndex];      // set the cycle length
            if (iAction != DMA_BUFFER_RESET) {                           // if not a buffer reset without continued operation
                fnClearChannelInterruptFlags(ulDmaTriggerSource, (DMA_LIFCR_TCIF0 | DMA_LIFCR_HTIF0 | DMA_LIFCR_TEIF0 | DMA_LIFCR_FEIFO0 | DMA_LIFCR_DMEIF0)); // ensure all flags are clear (the DMA may not start if old flags are not cleared)
                ptrDMAstream->DMA_SxCR |= DMA_SxCR_EN;                   // restart DMA from the start of the buffer
        #if defined _WINDOWS
                if ((DMA_CONTROLLER_REF_2 & ulDmaTriggerSource) != 0) {
                    iDMA |= (DMA_CONTROLLER_8 << (ulDmaTriggerSource & 0x7));
                }
                else {
                    iDMA |= (DMA_CONTROLLER_0 << (ulDmaTriggerSource & 0x7));
                }
        #endif
            }
    #endif
        }
        break;
    }
}
#endif

/* =================================================================== */
/*            DMA based memcpy(), memset() and reverse memcpy()        */
/* =================================================================== */

#if defined _DMA_MEM_TO_MEM
#if defined DMA_MEMCPY_SET && !defined DEVICE_WITHOUT_DMA
#define SMALLEST_DMA_COPY 20                                             // smaller copies have no DMA advantage

// memset implementation
//
extern void *uMemset(void *ptrTo, int iValue, size_t Size)               // {37}
{
    void *buffer = ptrTo;
    unsigned char ucValue = (unsigned char)iValue;                       // {37}
    unsigned char *ptr = (unsigned char *)ptrTo;
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    if ((Size >= SMALLEST_DMA_COPY) && (Size <= (0xffff * sizeof(unsigned long)))) { // {27} if large enough to be worth while
        #if MEMCPY_CHANNEL > 7
        STM32_DMA *ptrDMA_controller = (STM32_DMA *)DMA2_BLOCK;
        #define MEMCPY_CHANNEL_OFFSET (MEMCPY_CHANNEL - 8)
        #else
        STM32_DMA *ptrDMA_controller = (STM32_DMA *)DMA1_BLOCK;
        #define MEMCPY_CHANNEL_OFFSET (MEMCPY_CHANNEL)
        #endif
        STM32_DMA_STREAM *ptrDMAstream = &ptrDMA_controller->DMA_stream[MEMCPY_CHANNEL_OFFSET]; // select the stream registers to be used
        if (ptrDMAstream->DMA_SxCR == 0) {                               // if not already in use
            volatile unsigned long ulToCopy = (ucValue | (ucValue << 8) | (ucValue << 16) | (ucValue << 24));
            unsigned short usTransferSize;
        #if MEMCPY_CHANNEL_OFFSET < 4
            volatile unsigned long *ptrDLA_flag = &ptrDMA_controller->DMA_LISR;
            #if MEMCPY_CHANNEL_OFFSET == 0
                #define DMA_COMPLETE_FLAG    DMA_LISR_TCIF0
                #define DMA_ALL_FLAGS        (DMA_LIFCR_TCIF0 | DMA_LIFCR_HTIF0 | DMA_LIFCR_DMEIF0 | DMA_LIFCR_FEIFO0 | DMA_LIFCR_DMEIF0)
            #elif MEMCPY_CHANNEL_OFFSET == 1
                #define DMA_COMPLETE_FLAG    DMA_LISR_TCIF1
                #define DMA_ALL_FLAGS        (DMA_LIFCR_TCIF1 | DMA_LIFCR_HTIF1 | DMA_LIFCR_DMEIF1 | DMA_LIFCR_FEIFO1 | DMA_LIFCR_DMEIF1)
            #elif MEMCPY_CHANNEL_OFFSET == 2
                #define DMA_COMPLETE_FLAG    DMA_LISR_TCIF2
                #define DMA_ALL_FLAGS        (DMA_LIFCR_TCIF2 | DMA_LIFCR_HTIF2 | DMA_LIFCR_DMEIF2 | DMA_LIFCR_FEIFO2 | DMA_LIFCR_DMEIF2)
            #elif MEMCPY_CHANNEL_OFFSET == 3
                #define DMA_COMPLETE_FLAG    DMA_LISR_TCIF3
                #define DMA_ALL_FLAGS        (DMA_LIFCR_TCIF3 | DMA_LIFCR_HTIF3 | DMA_LIFCR_DMEIF3 | DMA_LIFCR_FEIFO3 | DMA_LIFCR_DMEIF3)
            #endif
        #else
            volatile unsigned long *ptrDLA_flag = &ptrDMA_controller->DMA_HISR;
            #if MEMCPY_CHANNEL_OFFSET == 4
                #define DMA_COMPLETE_FLAG    DMA_HISR_TCIF4
                #define DMA_ALL_FLAGS        (DMA_HISR_TCIF4 | DMA_HIFCR_HTIF4 | DMA_HIFCR_DMEIF4 | DMA_HIFCR_FEIFO4 | DMA_HIFCR_DMEIF4)
            #elif MEMCPY_CHANNEL_OFFSET == 5
                #define DMA_COMPLETE_FLAG    DMA_HISR_TCIF5
                #define DMA_ALL_FLAGS        (DMA_HIFCR_TCIF5 | DMA_HIFCR_HTIF5 | DMA_HIFCR_DMEIF5 | DMA_HIFCR_FEIFO5 | DMA_HIFCR_DMEIF5)
            #elif MEMCPY_CHANNEL_OFFSET == 6
                #define DMA_COMPLETE_FLAG    DMA_HISR_TCIF6
                #define DMA_ALL_FLAGS        (DMA_HIFCR_TCIF6 | DMA_HIFCR_HTIF6 | DMA_HIFCR_DMEIF6 | DMA_HIFCR_FEIFO6 | DMA_HIFCR_DMEIF6)
            #elif MEMCPY_CHANNEL_OFFSET == 7
                #define DMA_COMPLETE_FLAG    DMA_HISR_TCIF7
                #define DMA_ALL_FLAGS        (DMA_HIFCR_TCIF7 | DMA_HIFCR_HTIF7 | DMA_HIFCR_DMEIF7 | DMA_HIFCR_FEIFO7 | DMA_HIFCR_DMEIF7)
            #endif
        #endif
            while ((((unsigned long)ptr) & 0x3) != 0) {                  // move to a long word bounday
                *ptr++ = ucValue;
                Size--;
            }
            usTransferSize = ((unsigned short)(Size/sizeof(unsigned long))); // the number of long words to transfer by DMA
            ptrDMAstream->DMA_SxNDTR = usTransferSize;                   // the number of long word transfers to be made (max. 0xffff)
            ptrDMAstream->DMA_SxPAR = (unsigned long)&ulToCopy;          // address of long word to be transfered
            ptrDMAstream->DMA_SxM0AR = (unsigned long)ptr;               // address of first destination long word
            ptrDMAstream->DMA_SxCR = (DMA_SxCR_MINC | DMA_SxCR_PSIZE_32 | DMA_SxCR_MSIZE_32 | DMA_SxCR_PL_MEDIUM | DMA_SxCR_DIR_M2M); // set up DMA operation
            ptrDMAstream->DMA_SxCR |= DMA_SxCR_EN;                       // start operation
            usTransferSize *= sizeof(unsigned long);                     // the number of bytes being transferred by the DMA process
            ptr += usTransferSize;                                       // move the destination pointer to beyond the transfer
            Size -= usTransferSize;                                      // bytes remaining
            while (Size-- != 0) {
                *ptr++ = ucValue;
            }
            while ((*ptrDLA_flag & DMA_COMPLETE_FLAG) == 0) { SIM_DMA(0) }; // wait until the DMA transfer has terminated
            *(ptrDLA_flag + 2) = (DMA_ALL_FLAGS);                        // clear flags
        #if defined _WINDOWS
            *(ptrDLA_flag) &= ~(*(ptrDLA_flag + 2));
            *(ptrDLA_flag + 2) = 0;
        #endif
            ptrDMAstream->DMA_SxCR = 0;                                  // mark that the DMA stream is free for use again
            return buffer;
        }
    }
    #else
    if ((Size >= SMALLEST_DMA_COPY) && (Size <= 0xffff) && (DMA_CNDTR_MEMCPY == 0)) { // if large enought to be worth while and if not already in use
        volatile unsigned char ucToCopy = ucValue;
        DMA_CNDTR_MEMCPY = ((unsigned long)(Size));                      // the number of byte transfers to be made (max. 0xffff)
        DMA_CMAR_MEMCPY  = (unsigned long)&ucToCopy;                     // address of byte to be transfered
        DMA_CPAR_MEMCPY  = (unsigned long)ptr;                           // address of first destination byte
        DMA_CCR_MEMCPY   = (DMA1_CCR1_EN | DMA1_CCR1_PINC | DMA1_CCR1_PSIZE_8 | DMA1_CCR1_MSIZE_8 | DMA1_CCR1_PL_MEDIUM | DMA1_CCR1_MEM2MEM | DMA1_CCR1_DIR); // set up DMA operation and start DMA transfer       
        while (DMA_CNDTR_MEMCPY != 0) { SIM_DMA(0) };                    // wait until the transfer has terminated
        DMA_CCR_MEMCPY = 0;
        return buffer;
    }
    #endif
    // Normal memset method
    //
    while (Size-- != 0) {
        *ptr++ = ucValue;
    }
    return buffer;
}

extern void *uMemcpy(void *ptrTo, const void *ptrFrom, size_t Size)
{
    void *buffer = ptrTo;
    unsigned char *ptr1 = (unsigned char *)ptrTo;
    unsigned char *ptr2 = (unsigned char *)ptrFrom;
    #if (defined _STM32F2XX || defined _STM32F4XX || defined _STM32F7XX || defined _STM32H7XX)
    if ((Size >= SMALLEST_DMA_COPY) && (Size <= 0xffff)) {               // {27} if large enough to be worth while and if not already in use
        #if MEMCPY_CHANNEL > 7
        STM32_DMA *ptrDMA_controller = (STM32_DMA *)DMA2_BLOCK;
        #else
        STM32_DMA *ptrDMA_controller = (STM32_DMA *)DMA1_BLOCK;
        #endif
        STM32_DMA_STREAM *ptrDMAstream = &ptrDMA_controller->DMA_stream[MEMCPY_CHANNEL_OFFSET]; // select the stream registers to be used
        if (ptrDMAstream->DMA_SxCR == 0) {                               // if not already in use
            unsigned short usTransferSize;
        #if MEMCPY_CHANNEL_OFFSET < 4
            volatile unsigned long *ptrDLA_flag = &ptrDMA_controller->DMA_LISR;
        #else
            volatile unsigned long *ptrDLA_flag = &ptrDMA_controller->DMA_HISR;
        #endif
            while ((((unsigned long)ptr1) & 0x3) != 0) {                 // move to a long word boundary (the source is not guaranteed to be on a boundary, which can make the lomng word copy less efficient)
                *ptr1++ = *ptr2++;
                Size--;
            }
            ptrDMAstream->DMA_SxPAR = (unsigned long)ptr2;               // address of source
            ptrDMAstream->DMA_SxM0AR = (unsigned long)ptr1;              // address of destination
            if (((unsigned long)ptr2 & 0x3) == 0) {                      // if both source and destination are long word aligned
                usTransferSize = ((unsigned short)(Size/sizeof(unsigned long))); // the number of long words to transfer by DMA
                ptrDMAstream->DMA_SxNDTR = usTransferSize;               // the number of byte transfers to be made (max. 0xffff)
                ptrDMAstream->DMA_SxCR = (DMA_SxCR_PINC | DMA_SxCR_MINC | DMA_SxCR_PSIZE_32 | DMA_SxCR_MSIZE_32 | DMA_SxCR_PL_MEDIUM | DMA_SxCR_DIR_M2M); // set up DMA operation
                usTransferSize *= sizeof(unsigned long);                 // the number of bytes being transferred by the DMA process
            }
            else if (((unsigned long)ptr2 & 0x1) == 0) {                 // if both source and destination are short word aligned
                usTransferSize = ((unsigned short)(Size/sizeof(unsigned short))); // the number of short words to transfer by DMA
                ptrDMAstream->DMA_SxNDTR = usTransferSize;               // the number of byte transfers to be made (max. 0xffff)
                ptrDMAstream->DMA_SxCR = (DMA_SxCR_PINC | DMA_SxCR_MINC | DMA_SxCR_PSIZE_16 | DMA_SxCR_MSIZE_16 | DMA_SxCR_PL_MEDIUM | DMA_SxCR_DIR_M2M); // set up DMA operation
                usTransferSize *= sizeof(unsigned short);                // the number of bytes being transferred by the DMA process
            }
            else {
            #if defined _WINDOWS
                if (Size > 0xffff) {
                    _EXCEPTION("DMA transfer doesn't support more than 64k!!");
                }
            #endif
                usTransferSize = (unsigned short)Size;
                ptrDMAstream->DMA_SxNDTR = usTransferSize;               // the number of byte transfers to be made (max. 0xffff)
                ptrDMAstream->DMA_SxCR = (DMA_SxCR_PINC | DMA_SxCR_MINC | DMA_SxCR_PSIZE_8 | DMA_SxCR_MSIZE_8 | DMA_SxCR_PL_MEDIUM | DMA_SxCR_DIR_M2M); // set up DMA operation
            }
            ptrDMAstream->DMA_SxCR |= DMA_SxCR_EN;                       // start operation
            ptr1 += usTransferSize;                                      // move the destination pointer to beyond the transfer
            ptr2 += usTransferSize;                                      // move the source pointer to beyond the transfer
            Size -= usTransferSize;                                      // bytes remaining
            while ((*ptrDLA_flag & DMA_COMPLETE_FLAG) == 0) { SIM_DMA(0) }; // wait until the transfer has terminated
            *(ptrDLA_flag + 2) = (DMA_ALL_FLAGS);                        // clear flags
        #if defined _WINDOWS
            *(ptrDLA_flag) &= ~(*(ptrDLA_flag + 2));
            *(ptrDLA_flag + 2) = 0;
        #endif
            while (Size-- != 0) {                                        // {29}
                *ptr1++ = *ptr2++;
            }
            ptrDMAstream->DMA_SxCR = 0;                                  // mark that the DMA stream is free for use again
            return buffer;
        }
    }
    #else
    if ((Size >= SMALLEST_DMA_COPY) && (Size <= 0xffff) && (DMA_CNDTR_MEMCPY == 0)) { // if large enough to be worthwhile and if not already in use
        DMA_CNDTR_MEMCPY = ((unsigned long)(Size));                      // the number of byte transfers to be made (max 0xffff)
        DMA_CMAR_MEMCPY  = (unsigned long)ptrFrom;                       // address of first byte to be transferred
        DMA_CPAR_MEMCPY  = (unsigned long)ptrTo;                         // address of first destination byte
        DMA_CCR_MEMCPY   = (DMA1_CCR1_EN | DMA1_CCR1_PINC | DMA1_CCR1_MINC | DMA1_CCR1_PSIZE_8 | DMA1_CCR1_MSIZE_8 | DMA1_CCR1_PL_MEDIUM | DMA1_CCR1_MEM2MEM | DMA1_CCR1_DIR); // set up DMA operation and start DMA transfer
        while (DMA_CNDTR_MEMCPY != 0) { SIM_DMA(0) };                    // wait until the transfer has terminated
        DMA_CCR_MEMCPY = 0;
        return buffer;
    }
    #endif
    // Normal memcpy method
    //
    while (Size-- != 0) {
        *ptr1++ = *ptr2++;
    }
    return buffer; 
}
#endif
#endif
