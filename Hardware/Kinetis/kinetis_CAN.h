/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_CAN.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    11.01.2013 Set CAN controller clock source before moving to freeze mode {65}
    04.03.2014 Correct CAN time stamp request option                     {70}
    02.03.2017 Move controller clock source configuration to after setting freeze mode (due to problems with at least some parts) {3}
    14.03.2017 Set CAN clock before entering freeze mode but write further setting after moving to freeze mode {4}
    03.09.2018 Extend to 3 CAN controllers
    15.02.2019 Wait until freeze mode has been confirmed when setting filters {5}
    20.02.2019 Verify filter setting writes before continuing and optimise code to save space {6}
    20.03.2019 Add SGC/PCC based clock configuration                     {7}
    17.12.2019 Share CAN driver with iMX project
    06.02.2020 Add tx buffer flush support                               {8}

*/

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

// CAN interface queues
//
typedef struct stCANQue
{
    unsigned long  ulPartnerID;                                          // default transmission ID (msb signifies extended)
    unsigned long  ulOwnID;                                              // our own ID (msb signifies extended)
    QUEUE_HANDLE   DriverID;                                             // handle this buffer belongs to
    unsigned char  ucMode;
    unsigned char  ucErrors;                                             // tx error counter per buffer
    UTASK_TASK     TaskToWake;                                           // task responsible for processing rx frames, errors or tx frame complete
} CANQUE;

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static CANQUE can_queue[NUMBER_OF_CAN_INTERFACES][NUMBER_CAN_MESSAGE_BUFFERS] = {{{0}}};


static void fnCAN_Message(int iChannel)
{
#if defined MSCAN_CAN_INTERFACE
    // To do...
    //
#else
    static unsigned char can_int_message[HEADER_LENGTH] = {INTERNAL_ROUTE, INTERNAL_ROUTE, 0, INTERRUPT_EVENT, 0};
    KINETIS_CAN_CONTROL *ptrCAN_control;
    unsigned long ulBufferFlag = 0x00000001;
    CANQUE *ptrCanQue;
    int iNoWake = 0;
    unsigned int iBufferID = 0;
    switch (iChannel) {
    case 0:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        break;
    #endif
    }
    while (ptrCAN_control->CAN_IFLAG1 != 0) {                            // while message buffers flagged
        if ((ptrCAN_control->CAN_IFLAG1 & ulBufferFlag) != 0) {          // this buffer is signalling a successful transmission or reception
            ptrCanQue = &can_queue[iChannel][iBufferID];
            can_int_message[MSG_DESTINATION_TASK] = ptrCanQue->TaskToWake; // the task that should be woken when this buffer is ready
            if ((ptrCanQue->ucMode & CAN_TX_BUF) != 0) {                 // determine whether it is an rx or tx buffer
                if ((CAN_RX_REMOTE_RX & ptrCanQue->ucMode) != 0) {       // this buffer has just transmitted a remote frame request
                    if ((CAN_TX_BUF_ACTIVE & ptrCanQue->ucMode) != 0) {  // interrupt due to transmission
                        ptrCanQue->ucMode &= ~CAN_TX_BUF_ACTIVE;
                        iNoWake = 1;
                    }
                    else {                                               // a response has been detected
                        can_int_message[MSG_INTERRUPT_EVENT] = CAN_RX_REMOTE_MSG; // rx message received
                        ptrCanQue->ucMode = (CAN_TX_BUF | CAN_RX_REMOTE_RX | CAN_RX_BUF_FULL); // indicate message waiting
                    }
                }
                else if ((CAN_TX_BUF_REMOTE & ptrCanQue->ucMode) != 0) {
                    if ((ptrCanQue->ucMode & CAN_TX_ACK_ON) != 0) {
                        can_int_message[MSG_INTERRUPT_EVENT] = CAN_TX_REMOTE_ACK;// remote frame transmission message successfully acked
                    }
                    else {
                        iNoWake = 1;
                    }
                    ptrCanQue->ucMode &= ~CAN_TX_ACK_ON;                 // we only ack once
                }
                else {
                    if ((CAN_TX_ACK_ON & ptrCanQue->ucMode) != 0) {
                        can_int_message[MSG_INTERRUPT_EVENT] = CAN_TX_ACK; // tx message successfully acked
                    }
                    else {
                        iNoWake = 1;                                     // no acknowledgement required
                    }
                    ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE);  // indicate successful transmission, buffer free for further use
                }
            }
            else {                                                       // reception
                KINETIS_CAN_BUF *ptrMessageBuffer;
                switch (iChannel) {
                case 0:
                    ptrMessageBuffer = MBUFF0_ADD_0;
                    break;
                #if NUMBER_OF_CAN_INTERFACES > 1
                case 1:
                    ptrMessageBuffer = MBUFF0_ADD_1;
                    break;
                #endif
                #if NUMBER_OF_CAN_INTERFACES > 2
                case 2:
                    ptrMessageBuffer = MBUFF0_ADD_2;
                    break;
                #endif
                }
                ptrMessageBuffer += iBufferID;
                ptrMessageBuffer->ulCode_Len_TimeStamp &= ~CAN_RX_CODE_FIELD; // deactivate buffer so that it doesn't get overwritten

                can_int_message[MSG_INTERRUPT_EVENT] = CAN_RX_MSG;       // rx message received
                ptrCanQue->ucMode |= CAN_RX_BUF_FULL;                    // indicate message waiting
            }
    #if defined _WINDOWS
            ptrCAN_control->CAN_IFLAG1 &= ~(ulBufferFlag);
    #else
            ptrCAN_control->CAN_IFLAG1 = ulBufferFlag;                   // clear interrupt flag
    #endif
            if (iNoWake == 0) {
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when putting message to queue
                    fnWrite(INTERNAL_ROUTE, (unsigned char *)can_int_message, HEADER_LENGTH); // inform the handling task of event
                uEnable_Interrupt();
            }
        }
        ulBufferFlag <<= 1;
        if (++iBufferID >= NUMBER_CAN_MESSAGE_BUFFERS) {                 // last possible buffer processed so start at beginning again
            ulBufferFlag = 0x00000001;
            iBufferID = 0;
        }
    }
#endif
}

#if defined irq_CAN0_ERROR_ID || defined irq_CAN1_ERROR_ID || defined irq_CAN2_ERROR_ID
static void fnCAN_error(int iChannel)
{
#if defined MSCAN_CAN_INTERFACE
    // To do...
    //
#else
    unsigned char can_error_int_message[HEADER_LENGTH];
    KINETIS_CAN_CONTROL *ptrCAN_control;
    KINETIS_CAN_BUF *ptrMessageBuffer;
    volatile unsigned long ulError;                                      // read the error status register, clearing error bits

    can_error_int_message[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;
    can_error_int_message[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;
    can_error_int_message[MSG_DESTINATION_TASK] = CAN_ERROR_TASK;
    can_error_int_message[MSG_SOURCE_TASK]      = INTERRUPT_EVENT;

    switch (iChannel) {
    case 0:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_0;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_1;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_2;
        break;
    #endif
    }

    while (((ulError = ptrCAN_control->CAN_ESR1) & (BIT1ERROR | BIT0ERROR | CAN_CRC_ERR | CAN_ACK_ERR | CAN_FRM_ERR | CAN_STF_ERR)) != 0) { // read the error status register, clearing error bits
    #if defined _WINDOWS
        ptrCAN_control->CAN_ESR1 &= ~(BIT1ERROR | BIT0ERROR | CAN_CRC_ERR | CAN_ACK_ERR | CAN_FRM_ERR | CAN_STF_ERR);
    #endif
        if ((ulError & CAN_ACK_ERR) != 0) {                              // a transmission received no ack
            CANQUE *ptrCanQue = can_queue[iChannel];                     // we need to search for buffers which are transmitting
            int i = NUMBER_CAN_MESSAGE_BUFFERS;
            can_error_int_message[MSG_INTERRUPT_EVENT] = 0;
            while (i-- != 0) {
                if ((ptrCanQue->ucMode & CAN_TX_BUF_ACTIVE) != 0) {      // this buffer is presently transmitting a message
                    if (++(ptrCanQue->ucErrors) >= MAX_TX_CAN_TRIES) {   // we allow a few attempts before quitting (it also filters counting normal active buffers)
                        ptrMessageBuffer->ulCode_Len_TimeStamp = ((ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD) | MB_TX_INACTIVE); // stop transmission
                        can_error_int_message[MSG_DESTINATION_TASK] = ptrCanQue->TaskToWake;
                        if ((ptrCanQue->ucMode & CAN_TX_BUF_REMOTE) != 0) {
                            can_error_int_message[MSG_INTERRUPT_EVENT] = CAN_TX_REMOTE_ERROR;
                            ptrCanQue->ucMode = (CAN_TX_BUF | CAN_RX_BUF_FULL | CAN_TX_BUF_REMOTE); // mark that it is an errored transmission buffer
                        }
                        else {
                            can_error_int_message[MSG_INTERRUPT_EVENT] = CAN_TX_ERROR;
                            ptrCanQue->ucMode = (CAN_TX_BUF | CAN_RX_BUF_FULL); // mark that it is an errored transmission buffer
                        }
                        break;
                    }
                }
                ptrCanQue++;
                ptrMessageBuffer++;
            }
        }
        else {
            ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_TX_INACTIVE | (ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_KEEP_CONTENTS)); // free the buffer
            can_error_int_message[MSG_INTERRUPT_EVENT] = CAN_OTHER_ERROR;
        }
        ptrCAN_control->CAN_ESR1 = ERRINT;                               // clear the error interrupt
    #if defined _WINDOWS
        ptrCAN_control->CAN_ESR1 &= ~(ERRINT);
    #endif
        if (can_error_int_message[MSG_INTERRUPT_EVENT] != 0) {
            uDisable_Interrupt();                                        // ensure interrupts remain blocked when putting message to queue
                fnWrite(INTERNAL_ROUTE, can_error_int_message, HEADER_LENGTH); // inform the handling task of event
            uEnable_Interrupt();
        }
    }
#endif
}
#endif

#if defined MSCAN_CAN_INTERFACE
// To do...
//
#else
static __interrupt void _CAN0_Message_Interrupt(void)
{
    fnCAN_Message(0);
}
    #if defined irq_CAN0_BUS_OFF_ID
static __interrupt void _CAN0_BusOff_Interrupt(void)
{
    CAN0_ESR1 = BOFFINT;                                                 // clear the interrupt
}
    #endif
    #if defined irq_CAN0_ERROR_ID
static __interrupt void _CAN0_Error_Interrupt(void)
{
    fnCAN_error(0);
}
    #endif
    #if defined irq_CAN0_RX_ID
static __interrupt void _CAN0_Tx_Interrupt(void)
{
}
static __interrupt void _CAN0_Rx_Interrupt(void)
{
}
    #endif
    #if defined irq_CAN0_WAKE_UP_ID
static __interrupt void _CAN0_Wakeup_Interrupt(void)
{
}
    #endif
    #if defined irq_CAN0_IMEU_ID
static __interrupt void _CAN0_IMEU_Interrupt(void)
{
}
    #endif


    #if NUMBER_OF_CAN_INTERFACES > 1
// Interrupt when a message has been successfully transmitted or received from/into any buffer
//
static __interrupt void _CAN1_Message_Interrupt(void)
{
    fnCAN_Message(1);
}
        #if defined irq_CAN1_BUS_OFF_ID
static __interrupt void _CAN1_BusOff_Interrupt(void)
{
    CAN1_ESR1 = BOFFINT;                                                 // clear the interrupt
}
        #endif
        #if defined irq_CAN1_ERROR_ID
static __interrupt void _CAN1_Error_Interrupt(void)
{
    fnCAN_error(1);
}
        #endif
        #if defined irq_CAN1_RX_ID
static __interrupt void _CAN1_Tx_Interrupt(void)
{
}
static __interrupt void _CAN1_Rx_Interrupt(void)
{
}
        #endif
        #if defined irq_CAN1_WAKE_UP_ID
static __interrupt void _CAN1_Wakeup_Interrupt(void)
{
}
        #endif
        #if defined irq_CAN1_IMEU_ID
static __interrupt void _CAN1_IMEU_Interrupt(void)
{
}
        #endif
    #endif

    #if NUMBER_OF_CAN_INTERFACES > 2
// Interrupt when a message has been successfully transmitted or received from/into any buffer
//
static __interrupt void _CAN2_Message_Interrupt(void)
{
    fnCAN_Message(2);
}
static __interrupt void _CAN2_BusOff_Interrupt(void)
{
    CAN2_ESR1 = BOFFINT;                                                 // clear the interrupt
}
#if defined irq_CAN2_ERROR_ID
static __interrupt void _CAN2_Error_Interrupt(void)
{
    fnCAN_error(2);
}
#endif
static __interrupt void _CAN2_Tx_Interrupt(void)
{
}
static __interrupt void _CAN2_Rx_Interrupt(void)
{
}
static __interrupt void _CAN2_Wakeup_Interrupt(void)
{
}
        #if defined irq_CAN2_IMEU_ID
static __interrupt void _CAN2_IMEU_Interrupt(void)
{
}
        #endif
    #endif
#endif

// The best choice of clock input is from the external crystal (lowest jitter), however this may not always enable the best settings to achieve the required speed.
// The choice of clock source is user-defined but this routine tries to achieve best settings using highest time quanta resolution. 
//
// There are up to 25 time quanta in a CAN bit time and the bit frequency is equal to the clock frequency divided by the quanta number (8..25 time quanta range)
// There is always a single time quanta at the start of a bit period called the SYNC_SEG which cannot be changed (transitions are expected to occur on the bus during this period)
// The sampling occurs after time segment 1, which is made up of a propagation segment (1..8 time quanta) plus a phase buffer segment 1 (1..8 time quanta),
// followed by time segment 2, made up of phase buffer segment 2 (2..8 time quanta)
//
// CAN standard compliant bit segment settings give the following ranges (re-synchronisation jump width of 2 is used since it is compliant with all)
// Time segment 1 should be 5..10 when time segment 2 is 2 (min/max time quanta per bit is 8/13)
// Time segment 1 should be 4..11 when time segment 2 is 3 (min/max time quanta per bit is 8/15)
// Time segment 1 should be 5..12 when time segment 2 is 4 (min/max time quanta per bit is 10/17)
// Time segment 1 should be 6..13 when time segment 2 is 5 (min/max time quanta per bit is 12/19)
// Time segment 1 should be 7..14 when time segment 2 is 6 (min/max time quanta per bit is 14/21)
// Time segment 1 should be 8..15 when time segment 2 is 7 (min/max time quanta per bit is 16/23)
// Time segment 1 should be 9..16 when time segment 2 is 8 (min/max time quanta per bit is 18/25)
//
static unsigned long fnOptimalCAN_clock(unsigned short usMode, unsigned long ulSpeed)
{
    unsigned long ulClockSourceFlag = EXTAL_CLK_SOURCE;
    unsigned long ulClockSpeed;
    unsigned long ulLowestError = 0xffffffff;
    unsigned long ulCanSpeed;
    unsigned long ulError;
    unsigned long ulPrescaler;
    int iTimeQuanta = 25;                                                // highest value for highest control resolution
    int iBestTimeQuanta = 25;
    unsigned long ulBestPrescaler = 0;
    #if defined _iMX && defined ERR050235 && defined CAN_CLK_ROOT_FROM_PLL3_SW_CLK_6
    // When selecting the CCM CAN clock source with CAN_CLK_SEL set to 2, the UART clock gate
    // will not open and CAN_CLK_ROOT will be off. Therefore we workaround this by opening any LPUART clock gate
    //
    POWER_UP_ATOMIC(5, LPUART1_CLOCK);
    #endif
    if ((CAN_USER_SETTINGS & usMode) != 0) {
        return ulSpeed;                                                  // the user is passing optimal configuration settings directly
    }
    #if defined _iMX
    ulClockSpeed = CAN_CLK_ROOT_FREQUENCY;
    #else
    if ((CAN_PLL_CLOCK & usMode) != 0) {                                 // use the bus clock rather than crystal input directly
        ulClockSpeed = (BUS_CLOCK);                                      // the clock speed for calculation use is the bus speed
        ulClockSourceFlag = CLK_SRC_PERIPH_CLK;                          // peripheral clock will be used as clock source
    }
    else {
        #if defined KINETIS_WITH_SCG                                     // {7}
            #if defined _WINDOWS
        if (SOSCDIV2_FREQUENCY == 0) {
            _EXCEPTION("SOSCDIV2 frequency needs to be defined by using SOSCDIV2_DIVIDE!");
        }
            #endif
        ulClockSpeed = SOSCDIV2_FREQUENCY;                               // the clock speed for calculation use is the SOSCDIV2 clock speed
        #else
        ulClockSpeed = _EXTERNAL_CLOCK;                                  // the clock speed for calculation use is the external/crystal clock speed
        #endif
        // If the oscillator clock is used its frequency must be lower than the bus clock
        //
        #if defined _WINDOWS
        if (ulClockSpeed > BUS_CLOCK) {
            _EXCEPTION("Oscillator clock must be lower than the bus clock!!");
        }
        #endif
    }
    #endif
    while (iTimeQuanta >= 8) {                                           // test for best time quanta
        ulCanSpeed = (ulClockSpeed/iTimeQuanta);                         // speed without prescaler
        ulPrescaler = ((ulCanSpeed + (ulSpeed/2))/ulSpeed);              // best prescale value
        if (ulPrescaler > 256) {
            ulPrescaler = 256;                                           // maximum possible prescale divider
        }
        else if (ulPrescaler == 0) {
            ulPrescaler = 1;
        }
        ulCanSpeed /= ulPrescaler;
        if (ulCanSpeed >= ulSpeed) {                                     // determine the absolute error value with this quanta setting
            ulError = (ulCanSpeed - ulSpeed);
        }
        else {
            ulError = (ulSpeed - ulCanSpeed);
        }
        if (ulError < ulLowestError) {                                   // if this is an improvement
            ulLowestError = ulError;
            iBestTimeQuanta = iTimeQuanta;                               // best time quanta value
            ulBestPrescaler = ulPrescaler;
        }
        iTimeQuanta--;
    }
    ulBestPrescaler--;                                                   // convert to register setting value
    ulBestPrescaler <<= 24;                                              // move the prescale value into position
    if (iBestTimeQuanta >= 18) {                                         // determine the phase buffer length value
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN8;
        iBestTimeQuanta -= (8 + 1);                                      // remaining time quanta (time segment 1) after removal of the time segment 2 and the SYN_SEG
    }
    else if (iBestTimeQuanta >= 16) {
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN7;
        iBestTimeQuanta -= (7 + 1);
    }
    else if (iBestTimeQuanta >= 14) {
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN6;
        iBestTimeQuanta -= (6 + 1);
    }
    else if (iBestTimeQuanta >= 12) {
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN5;
        iBestTimeQuanta -= (5 + 1);
    }
    else if (iBestTimeQuanta >= 10) {
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN4;
        iBestTimeQuanta -= (4 + 1);
    }
    else {
        ulBestPrescaler |= PHASE_BUF_SEG2_LEN3;
        iBestTimeQuanta -= (3 + 1);
    }
    if ((iBestTimeQuanta & 0x1) != 0) {                                  // odd
        iBestTimeQuanta /= 2;                                            // PROP_SEG and PSEG1 to achieve time segment 1
        ulBestPrescaler |= iBestTimeQuanta;                              // set propogation bit time (1 more than phase buffer segment 1)
        iBestTimeQuanta--;
        ulBestPrescaler |= (iBestTimeQuanta << 19);                      // set phase buffer segment 1
    }
    else {                                                               // even
        iBestTimeQuanta /= 2;                                            // PROP_SEG and PSEG1 to achieve time segment 1 and phase buffer segment 1
        iBestTimeQuanta--;
        ulBestPrescaler |= ((iBestTimeQuanta << 19) | (iBestTimeQuanta));// set equal propogation bit times
    }
    return (RJW_2 | ulClockSourceFlag | ulBestPrescaler);                // initialise the CAN controller with the required speed and parameters
}

// Hardware initialisation of CAN controller
//
extern void fnInitCAN(CANTABLE *pars)
{
#if defined MSCAN_CAN_INTERFACE
    // To do...
    //
#else
    unsigned long ulCanCtrValue;
    KINETIS_CAN_BUF *ptrMessageBuffer;
    KINETIS_CAN_CONTROL *ptrCAN_control;

    #if defined KINETIS_WITH_SCG                                         // {7}
    if ((CAN_PLL_CLOCK & pars->usMode) == 0) {                           // if the external clock is to be used enable it
        if ((SCG_SOSCCSR & SCG_SOSCCSR_SOSCVLD) == 0) {                  // if the clock has not been enabled (or is not yet valid)
            SCG_SOSCCSR |= SCG_SOSCCSR_SOSCEN;                           // enable the external reference clock source for CAN use (SOSCDIV2_CLK)
        }
    }
    #elif defined ERRATA_ID_2583                                         // in early silicon the CAN controllers only work when the OSC is enabled
    if ((OSC0_CR & OSC_CR_ERCLKEN) == 0) {
        OSC0_CR = OSC_CR_ERCLKEN;                _SIM_PER_CHANGE;        // enable the external reference clock source whether the bus clock or external clock is to be used
    }
    #elif !defined _iMX
    if ((CAN_PLL_CLOCK & pars->usMode) == 0) {                           // if the external clock is to be used enable it
        if ((OSC0_CR & OSC_CR_ERCLKEN) == 0) {                           // if the clock has not been enabled
            OSC0_CR = OSC_CR_ERCLKEN;            _SIM_PER_CHANGE;        // enable the external reference clock source for CAN use
        }
    }
    #endif
    switch (pars->Channel) {
    case 0:
    #if defined KINETIS_WITH_PCC                                         // {7}
        SELECT_PCC_PERIPHERAL_SOURCE(FLEXCAN0, FLEXCAN0_PCC_SOURCE);     // select the PCC clock used by FlexCAN0
    #endif
    #if defined _iMX
        POWER_UP_ATOMIC(0, CAN1_CLOCK);                                  // supply the clock to the flexcan controller
    #else
        POWER_UP_ATOMIC(6, FLEXCAN0);
    #endif
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_0;                                 // the first of 16 (64 iMX) message buffers in the FlexCan module
    #if defined _iMX
        #if defined iMX_RT105X || defined iMX_RT106X
        IOMUXC_FLEXCAN1_RX_SELECT_INPUT = IOMUXC_FLEXCAN1_RX_SELECT_INPUT_GPIO_AD_B1_09_ALT2; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B1_08, FLEXCAN1_TX, (PORT_DSE_MID));  // FLEXCAN1_TX on GPIO1-24 - alt function 2 [iMX FLEXCAN count 1..2]
        _CONFIG_PERIPHERAL(GPIO_AD_B1_09, FLEXCAN1_RX, (PORT_DSE_MID));  // FLEXCAN1_RX on GPIO1-25 - alt function 2 [iMX FLEXCAN count 1..2]
        #else
        IOMUXC_FLEXCAN1_RX_SELECT_INPUT = IOMUXC_FLEXCAN1_RX_SELECT_INPUT_GPIO_SD_B1_01_ALT4; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_SD_B1_00, FLEXCAN1_TX, (PORT_DSE_MID));  // FLEXCAN1_TX on GPIO3-20 - alt function 4 [iMX FLEXCAN count 1..2]
        _CONFIG_PERIPHERAL(GPIO_SD_B1_01, FLEXCAN1_RX, (PORT_DSE_MID));  // FLEXCAN1_RX on GPIO3-21 - alt function 4 [iMX FLEXCAN count 1..2]
        #endif
    #elif defined KINETIS_KE18
        #if defined CAN0_ON_PE
        _CONFIG_PERIPHERAL(E, 5, PE_5_CAN0_TX);                          // CAN0_TX on PE5 (alt. function 5)
        _CONFIG_PERIPHERAL(E, 6, PE_6_CAN0_RX);                          // CAN0_RX on PE6 (alt. function 5)
        #else
        _CONFIG_PERIPHERAL(C, 3, PC_3_CAN0_TX);                          // CAN0_TX on PC3 (alt. function 3)
        _CONFIG_PERIPHERAL(C, 2, PC_2_CAN0_RX);                          // CAN0_RX on PC2 (alt. function 3)
        #endif
    #elif defined CAN0_ON_PB_LOW
        _CONFIG_PERIPHERAL(B, 16, PB_16_CAN0_TX);                        // CAN0_TX on PB18 (alt. function 5)
        _CONFIG_PERIPHERAL(B, 17, PB_17_CAN0_RX);                        // CAN0_RX on PB19 (alt. function 5)
    #elif defined CAN0_ON_PB
        _CONFIG_PERIPHERAL(B, 18, PB_18_CAN0_TX);                        // CAN0_TX on PB18 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 19, PB_19_CAN0_RX);                        // CAN0_RX on PB19 (alt. function 2)
    #else
        _CONFIG_PERIPHERAL(A, 12, PA_12_CAN0_TX);                        // CAN0_TX on PA12 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 13, PA_13_CAN0_RX);                        // CAN0_RX on PA13 (alt. function 2)
    #endif
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        #if defined KINETIS_WITH_PCC                                     // {7}
        SELECT_PCC_PERIPHERAL_SOURCE(FLEXCAN1, FLEXCAN1_PCC_SOURCE);     // select the PCC clock used by FlexCAN1
        #endif
        #if defined _iMX
        POWER_UP_ATOMIC(0, CAN2_CLOCK);                                  // supply the clock to the flexcan controller
        #elif defined KINETIS_KV50
        POWER_UP_ATOMIC(6, FLEXCAN1);
        #else
        POWER_UP_ATOMIC(3, FLEXCAN1);
        #endif
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_1;                                 // the first of 16 (64 iMX) message buffers in the FlexCan module
        #if defined _iMX
            #if defined iMX_RT105X || defined iMX_RT106X
        IOMUXC_FLEXCAN2_RX_SELECT_INPUT = IOMUXC_FLEXCAN2_RX_SELECT_INPUT_GPIO_AD_B0_15_ALT6; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_14, FLEXCAN2_TX, (PORT_DSE_MID));  // FLEXCAN2_TX on GPIO1-14 - alt function 6 [iMX FLEXCAN count 1..2]
        _CONFIG_PERIPHERAL(GPIO_AD_B0_15, FLEXCAN2_RX, (PORT_DSE_MID));  // FLEXCAN2_RX on GPIO1-15 - alt function 6 [iMX FLEXCAN count 1..2]
            #else
        IOMUXC_FLEXCAN2_RX_SELECT_INPUT = IOMUXC_FLEXCAN2_RX_SELECT_INPUT_GPIO_AD_B0_15_ALT1; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_14, FLEXCAN2_TX, (PORT_DSE_MID));  // FLEXCAN2_TX on GPIO1-14 - alt function 1 [iMX FLEXCAN count 1..2]
        _CONFIG_PERIPHERAL(GPIO_AD_B0_15, FLEXCAN2_RX, (PORT_DSE_MID));  // FLEXCAN2_RX on GPIO1-15 - alt function 1 [iMX FLEXCAN count 1..2]
            #endif
        #elif defined KINETIS_KE18
            #if defined CAN0_ON_PA
        _CONFIG_PERIPHERAL(A, 13, PA_13_CAN1_TX);                        // CAN1_TX on PA13 (alt. function 3)
        _CONFIG_PERIPHERAL(A, 12, PA_12_CAN1_RX);                        // CAN1_RX on PA12 (alt. function 3)
            #else
        _CONFIG_PERIPHERAL(C, 7, PC_7_CAN1_TX);                          // CAN1_TX on PC7 (alt. function 3)
        _CONFIG_PERIPHERAL(C, 6, PC_6_CAN1_RX);                          // CAN1_RX on PC6 (alt. function 3)
            #endif
        #elif defined CAN1_ON_PE
        _CONFIG_PERIPHERAL(E, 24, PE_24_CAN1_TX);                        // CAN1_TX on PE24 (alt. function 2)
        _CONFIG_PERIPHERAL(E, 25, PE_25_CAN1_RX);                        // CAN1_RX on PE25 (alt. function 2)
        #else
        _CONFIG_PERIPHERAL(C, 17, PC_17_CAN1_TX);                        // CAN1_TX on PC17 (alt. function 2)
        _CONFIG_PERIPHERAL(C ,16, PC_16_CAN1_RX);                        // CAN1_RX on PC16 (alt. function 2)
        #endif
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        #if defined _iMX
        POWER_UP_ATOMIC(0, CAN2_CLOCK);                                  // supply the clock to the flexcan controller
        #else
        POWER_UP_ATOMIC(3, FLEXCAN2);
        #endif
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_2;                                 // the first of 16 message buffers in the FlexCan module
        #if defined _iMX
        _CONFIG_PERIPHERAL(GPIO_AD_B0_10, FLEXCAN3_TX, (PORT_DSE_MID));  // FLEXCAN3_TX on GPIO1-10 - alt function 8 [iMX FLEXCAN count 1..2]
        _CONFIG_PERIPHERAL(GPIO_AD_B0_11, FLEXCAN3_RX, (PORT_DSE_MID));  // FLEXCAN2_RX on GPIO1-11 - alt function 8 [iMX FLEXCAN count 1..2]
        #elif defined CAN2_ON_PC
        _CONFIG_PERIPHERAL(C, 12, PC_12_CAN2_TX);                        // CAN2_TX on PC12 (alt. function 2)
        _CONFIG_PERIPHERAL(C, 13, PC_13_CAN2_RX);                        // CAN2_RX on PC13 (alt. function 2)
        #elif defined CAN2_ON_PB
        _CONFIG_PERIPHERAL(B, 6, PB_6_CAN2_TX);                          // CAN2_TX on PB6 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 7, PB_7_CAN2_RX);                          // CAN2_RX on PB7 (alt. function 2)
        #else
        _CONFIG_PERIPHERAL(A, 14, PA_14_CAN2_TX);                        // CAN2_TX on PA14 (alt. function 4)
        _CONFIG_PERIPHERAL(A, 15, PA_15_CAN2_RX);                        // CAN2_RX on PA15 (alt. function 4)
        #endif
        break;
    #endif
    default:
        return;
    }

    uMemset(ptrMessageBuffer, 0x00, (sizeof(KINETIS_CAN_BUF) * NUMBER_CAN_MESSAGE_BUFFERS)); // the buffers are not reset so clear here
                                                                         // calculate the settings for the required speed
    ulCanCtrValue = fnOptimalCAN_clock(pars->usMode, pars->ulSpeed);
    ptrCAN_control->CAN_CTRL1 = (CLK_SRC_PERIPH_CLK & ulCanCtrValue);    // {4}{3}{65} select the clock source while the module is in disable mode
                                                                         // note that the crystal can be used and has better jitter performance than the PLL
    ptrCAN_control->CAN_MCR &= ~CAN_MDIS;                                // move from disabled to freeze mode (un-synchronised to the CAN bus)
    _WAIT_REGISTER_TRUE(ptrCAN_control->CAN_MCR, CAN_LPMACK);             // wait for CAN controller to leave disabled mode
    ptrCAN_control->CAN_CTRL1 = ulCanCtrValue;                           // {4} write the clock setting

    if ((pars->usMode & CAN_LOOPBACK) != 0) {
        ptrCAN_control->CAN_CTRL1 |= LPB;                                // set loopback mode
    }

    switch (pars->Channel) {
    case 0:
        fnEnterInterrupt(irq_CAN0_MESSAGE_ID, PRIORITY_CAN0_MESSAGE, _CAN0_Message_Interrupt); // enter all CAN interrupts
    #if defined irq_CAN0_BUS_OFF_ID
        fnEnterInterrupt(irq_CAN0_BUS_OFF_ID, PRIORITY_CAN0_BUS_OFF, _CAN0_BusOff_Interrupt);
    #endif
    #if defined irq_CAN0_ERROR_ID
        fnEnterInterrupt(irq_CAN0_ERROR_ID, PRIORITY_CAN0_ERROR, _CAN0_Error_Interrupt);
    #endif
    #if defined irq_CAN0_RX_ID
        fnEnterInterrupt(irq_CAN0_TX_ID, PRIORITY_CAN0_TX, _CAN0_Tx_Interrupt);
        fnEnterInterrupt(irq_CAN0_RX_ID, PRIORITY_CAN0_RX, _CAN0_Rx_Interrupt);
    #endif
    #if defined irq_CAN0_WAKE_UP_ID
        fnEnterInterrupt(irq_CAN0_WAKE_UP_ID, PRIORITY_CAN0_WAKEUP, _CAN0_Wakeup_Interrupt);
    #endif
    #if defined irq_CAN0_IMEU_ID
        fnEnterInterrupt(irq_CAN0_IMEU_ID, PRIORITY_CAN0_IMEU, _CAN0_IMEU_Interrupt);
    #endif
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        fnEnterInterrupt(irq_CAN1_MESSAGE_ID, PRIORITY_CAN1_MESSAGE, _CAN1_Message_Interrupt); // enter all CAN interrupts
        #if defined irq_CAN1_BUS_OFF_ID
        fnEnterInterrupt(irq_CAN1_BUS_OFF_ID, PRIORITY_CAN1_BUS_OFF, _CAN1_BusOff_Interrupt);
        #endif
        #if defined irq_CAN1_ERROR_ID
        fnEnterInterrupt(irq_CAN1_ERROR_ID, PRIORITY_CAN1_ERROR, _CAN1_Error_Interrupt);
        #endif
        #if defined irq_CAN1_RX_ID
        fnEnterInterrupt(irq_CAN1_TX_ID, PRIORITY_CAN1_TX, _CAN1_Tx_Interrupt);
        fnEnterInterrupt(irq_CAN1_RX_ID, PRIORITY_CAN1_RX, _CAN1_Rx_Interrupt);
        #endif
        #if defined irq_CAN1_WAKE_UP_ID
        fnEnterInterrupt(irq_CAN1_WAKE_UP_ID, PRIORITY_CAN1_WAKEUP, _CAN1_Wakeup_Interrupt);
        #endif
        #if defined irq_CAN1_IMEU_ID
        fnEnterInterrupt(irq_CAN1_IMEU_ID, PRIORITY_CAN1_IMEU, _CAN1_IMEU_Interrupt);
        #endif
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        fnEnterInterrupt(irq_CAN2_MESSAGE_ID, PRIORITY_CAN2_MESSAGE, _CAN2_Message_Interrupt); // enter all CAN interrupts
        #if defined irq_CAN1_BUS_OFF_ID
        fnEnterInterrupt(irq_CAN2_BUS_OFF_ID, PRIORITY_CAN2_BUS_OFF, _CAN2_BusOff_Interrupt);
        #endif
        #if defined irq_CAN2_ERROR_ID
        fnEnterInterrupt(irq_CAN2_ERROR_ID, PRIORITY_CAN2_ERROR, _CAN2_Error_Interrupt);
        #endif
        #if defined irq_CAN2_RX_ID
        fnEnterInterrupt(irq_CAN2_TX_ID, PRIORITY_CAN2_TX, _CAN2_Tx_Interrupt);
        fnEnterInterrupt(irq_CAN2_RX_ID, PRIORITY_CAN2_RX, _CAN2_Rx_Interrupt);
        #endif
        #if defined irq_CAN2_WAKE_UP_ID
        fnEnterInterrupt(irq_CAN2_WAKE_UP_ID, PRIORITY_CAN2_WAKEUP, _CAN1_Wakeup_Interrupt);
        #endif
        #if defined irq_CAN2_IMEU_ID
        fnEnterInterrupt(irq_CAN2_IMEU_ID, PRIORITY_CAN2_IMEU, _CAN2_IMEU_Interrupt);
        #endif
        break;
    #endif
    }

    ptrCAN_control->CAN_IFLAG1 = (CAN_ALL_BUFFERS_INT);                  // ensure no pending interrupts
    ptrCAN_control->CAN_IMASK1 = (CAN_ALL_BUFFERS_INT);                  // enable buffer interrupts on all message boxes

    ptrCAN_control->CAN_ESR1 = (ERRINT | BOFFINT);                       // ensure no pending error interrupts 
    ptrCAN_control->CAN_CTRL1 |= (ERRMSK | BOFFMSK);
   
    ptrCAN_control->CAN_MCR &= ~(CAN_FRZ | CAN_HALT);                    // leave freeze mode and start synchronisation

    #if defined _WINDOWS
    ptrCAN_control->CAN_IFLAG1 = 0;
    ptrCAN_control->CAN_ESR1 = 0;
    fnSimCAN(pars->Channel, 0, CAN_SIM_INITIALISE);
    #endif
#endif
}

static void fnSetFilterMask(KINETIS_CAN_CONTROL *ptrCAN_control, KINETIS_CAN_BUF *ptrMessageBuffer, unsigned long ulRxIDMask)
{
    volatile unsigned long *ptrMaskRegister;
    if (&ptrCAN_control->CAN_MBUFF15 == ptrMessageBuffer) {
        ptrMaskRegister = &ptrCAN_control->CAN_RX15MASK;                 // first allocated receiver buffer has unique filter mask
    }
    else if (&ptrCAN_control->CAN_MBUFF14 == ptrMessageBuffer) {
        ptrMaskRegister = &ptrCAN_control->CAN_RX14MASK;                 // second allocated receiver buffer has unique filter mask
    }
    else {
        ptrMaskRegister = &ptrCAN_control->CAN_RXGMASK;                  // subsequent receiver buffers have general filter mask
    }
    do {
        *ptrMaskRegister = ulRxIDMask;                                   // write the mask value
    } while (*ptrMaskRegister != ulRxIDMask);                            // {6} even after entering in to freeze mode it is sometimes found that this register can't be set immediately so we write until it sticks
}

// Hardware configuration of CAN controller
//
extern void fnConfigCAN(QUEUE_HANDLE DriverID, CANTABLE *pars)
{
#if defined MSCAN_CAN_INTERFACE
    #if defined MSCAN_ON_PORT_C
    SIM_PINSEL1 &= ~(SIM_PINSEL1_MSCANPS);
    _CONFIG_PERIPHERAL(C, 7, PC_7_CAN0_TX);                              // MSCAN TX on PC7 (alt. function 5)
    _CONFIG_PERIPHERAL(C, 6, PC_6_CAN0_RX);                              // MSCAN RX on PC6 (alt. function 5)
    #else
    SIM_PINSEL1 |= SIM_PINSEL1_MSCANPS;                                  // select alternate pins
    _CONFIG_PERIPHERAL(E, 7, PE_7_CAN0_TX);                              // MSCAN TX on PE7 (alt. function 5)
    _CONFIG_PERIPHERAL(H, 2, PH_2_CAN0_RX);                              // MSCAN RX on PH2 (alt. function 5)
    #endif
    MSCAN_CANCTL1 = MSCAN_CANCTL1_CANE;                                  // enable operation
                                                                         _SIM_PER_CHANGE;
#else
    int i;
    unsigned char ucTxCnt = pars->ucTxBuffers;
    unsigned char ucRxCnt = pars->ucRxBuffers;
    KINETIS_CAN_BUF *ptrMessageBuffer;
    KINETIS_CAN_CONTROL *ptrCAN_control;
    CANQUE *ptrCanQue;

    switch (pars->Channel) {                                             // the CAN controller to use
    case 0:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        break;
    #endif
    default:
        return;
    }
    ptrCanQue = can_queue[pars->Channel];
    ptrCAN_control->CAN_MCR |= (CAN_FRZ | CAN_HALT);                     // move to freeze mode
    ptrMessageBuffer = &ptrCAN_control->CAN_MBUFF0;                      // the first of 16 (64 iMX) message buffers in the FlexCan module
    _WAIT_REGISTER_FALSE(ptrCAN_control->CAN_MCR, CAN_FRZACK);           // {5} wait until freeze mode has been entered

    for (i = 0; i < NUMBER_CAN_MESSAGE_BUFFERS; i++) {                   // initialise the requested number of transmit buffers
        if (ucTxCnt == 0) {
            break;
        }
        if (ptrCanQue->DriverID == 0) {                                  // not yet allocated
            ptrCanQue->DriverID = DriverID;
            ptrCanQue->TaskToWake = pars->Task_to_wake;
            ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE);
            if ((pars->ulTxID & CAN_EXTENDED_ID) != 0) {
                ptrCanQue->ulPartnerID = pars->ulTxID;                   // enter default destination ID
                ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_TX_INACTIVE | IDE); // enable extended ID transmission
            }
            else {
                ptrCanQue->ulPartnerID = ((pars->ulTxID << CAN_STANDARD_SHIFT) & CAN_STANDARD_BITMASK); // enter reception ID for the buffer
                ptrMessageBuffer->ulCode_Len_TimeStamp = MB_TX_INACTIVE;
            }
            if ((pars->ulRxID & CAN_EXTENDED_ID) != 0) {
                ptrCanQue->ulOwnID = pars->ulRxID;
            }
            else {
                ptrCanQue->ulOwnID = ((pars->ulRxID << CAN_STANDARD_SHIFT) & CAN_STANDARD_BITMASK);
            }
            ucTxCnt--;
        }
        ptrMessageBuffer++;
        ptrCanQue++;
    }
    #if defined _iMX
    ptrMessageBuffer = &ptrCAN_control->CAN_MBUFF63;                     // the last of 64 message buffers in the FlexCan module
    #else
    ptrMessageBuffer = &ptrCAN_control->CAN_MBUFF15;                     // the last of 16 message buffers in the FlexCan module
    #endif
    ptrCanQue = &can_queue[pars->Channel][NUMBER_CAN_MESSAGE_BUFFERS - 1]; // start at last receive message buffer
    for (i = 0; i < NUMBER_CAN_MESSAGE_BUFFERS; i++) {                   // initialise the requested number of receive buffers
        if (ucRxCnt == 0) {
            break;
        }
        if (0 == ptrCanQue->DriverID) {                                  // not yet allocated
            unsigned long ulFilterMask;
            unsigned long ulReceptionID;
            unsigned long ulExtendedReception;
            ucRxCnt--;
            ptrCanQue->DriverID = DriverID;
            ptrCanQue->TaskToWake = pars->Task_to_wake;
            if ((pars->ulRxID & CAN_EXTENDED_ID) != 0) {
                ulFilterMask = pars->ulRxIDMask;
                ulReceptionID = (pars->ulRxID & CAN_EXTENDED_MASK);
                ulExtendedReception = (MB_RX_EMPTY | IDE);               // extended reception
            }
            else {
                ulFilterMask = ((pars->ulRxIDMask << CAN_STANDARD_SHIFT) & CAN_STANDARD_BITMASK);
                ulReceptionID = ((pars->ulRxID << CAN_STANDARD_SHIFT) & CAN_STANDARD_BITMASK); // enter reception ID for the buffer
                ulExtendedReception = MB_RX_EMPTY;                       // standard reception
            }
            fnSetFilterMask(ptrCAN_control, ptrMessageBuffer, ulFilterMask); // {6} set filter mask
            ptrMessageBuffer->ulID = ulReceptionID;                      // enter reception ID for the buffer
            ptrMessageBuffer->ulCode_Len_TimeStamp = ulExtendedReception;// enable reception at the buffer
        }
        ptrMessageBuffer--;
        ptrCanQue--;
    }
    ptrCAN_control->CAN_MCR &= ~(CAN_FRZ | CAN_HALT);                    // leave freeze mode and start synchronisation
    #if defined _WINDOWS
    ptrCAN_control->CAN_MCR &= ~CAN_FRZACK;
    #endif
#endif
}

// The CAN driver has called this to send a message
//
extern unsigned char fnCAN_tx(QUEUE_HANDLE Channel, QUEUE_HANDLE DriverID, unsigned char *ptBuffer, QUEUE_TRANSFER Counter)
{
#if defined MSCAN_CAN_INTERFACE
    // To do...
    //
#else
    int i = 0;
    KINETIS_CAN_BUF *ptrMessageBuffer;
    unsigned char ucTxMode = (unsigned char)(Counter & CAN_TX_MSG_MASK);
    CANQUE *ptrCanQue;
    unsigned char ucRtnCnt; 
    unsigned long ulExtendedID = 0;
    unsigned char ucFreedTxBuffers = 0;                                  // {8}

    switch (Channel) {                                                   // the CAN controller to use
    case 0:
        ptrMessageBuffer = MBUFF0_ADD_0;                                 // the first of 16 (64 in iMX) message buffers in the FlexCan module
        ptrCanQue = can_queue[0];
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrMessageBuffer = MBUFF0_ADD_1;                                 // the first of 16 (64 in iMX) message buffers in the FlexCan module
        ptrCanQue = can_queue[1];
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrMessageBuffer = MBUFF0_ADD_2;                                 // the first of 16 (64 in iMX) message buffers in the FlexCan module
        ptrCanQue = can_queue[2];
        break;
    #endif
    default:
        return 0;
    }

    Counter &= ~CAN_TX_MSG_MASK;
    ucRtnCnt = (unsigned char)Counter;

    if ((ucTxMode & (TX_REMOTE_FRAME | TX_REMOTE_STOP)) != 0) {          // only one remote transmit buffer allowed
        for (; i < NUMBER_CAN_MESSAGE_BUFFERS; i++) { 
            if (ptrCanQue->DriverID == DriverID) {                       // find a buffer belonging to us
                if ((ptrCanQue->ucMode & CAN_TX_BUF_REMOTE) != 0) {      // active remote buffer found
                    if ((ucTxMode & TX_REMOTE_STOP) != 0) {
                        ptrMessageBuffer->ulCode_Len_TimeStamp = MB_TX_INACTIVE; // disable and free buffer
                        ptrMessageBuffer->ulID = (ptrCanQue->ulPartnerID & ~CAN_EXTENDED_ID); // set tx message buffer to default id
                        ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE); // buffer remain inactive tx buffer
    #if defined _WINDOWS
                        fnSimCAN(Channel, i, CAN_SIM_FREE_BUFFER);
    #endif
                        return 1;                                        // remote frame transmission disabled
                    }
                    else {
                        ptrMessageBuffer->ulCode_Len_TimeStamp = MB_TX_INACTIVE; // disable so that changes can be made
                        ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE); // buffer remain inactive tx buffer
                    }
                    break;                                               // the remote buffer has been found
                }
            }
            ptrMessageBuffer++;
            ptrCanQue++;
        }
        if (i >= NUMBER_CAN_MESSAGE_BUFFERS) {                           // no remote buffer present
            i = 0;                                                       // reset ready for restart
            switch (Channel) {                                           // the CAN controller to use
            case 0:
                ptrMessageBuffer = MBUFF0_ADD_0;                         // the first of 16 (64 in iMX) message buffers in the FlexCan module
                ptrCanQue = can_queue[0];
                break;
            #if NUMBER_OF_CAN_INTERFACES > 1
            case 1:
                ptrMessageBuffer = MBUFF0_ADD_1;                         // the first of 16 (64 in iMX) message buffers in the FlexCan module
                ptrCanQue = can_queue[1];
                break;
            #endif
            #if NUMBER_OF_CAN_INTERFACES > 2
            case 2:
                ptrMessageBuffer = MBUFF0_ADD_2;                         // the first of 16 (64 in iMX) message buffers in the FlexCan module
                ptrCanQue = can_queue[2];
                break;
            #endif
            }
        }
    }

    for (; i < NUMBER_CAN_MESSAGE_BUFFERS; i++) {
        if (ptrCanQue->DriverID == DriverID) {                           // find a buffer belonging to us
            if (((ptrCanQue->ucMode & CAN_TX_BUF_FREE) != 0) && ((TX_REMOTE_STOP & ucTxMode) == 0)) { // if the transmit buffer is free and not stopping transmission
                if ((TX_REMOTE_FRAME & ucTxMode) != 0) {                 // depositing a message to be sent on a remote request
                    if ((ptrCanQue->ulPartnerID & CAN_EXTENDED_ID) != 0) {
                        ulExtendedID = (IDE | MB_TX_SEND_ON_REQ | ((unsigned long)Counter << 16)); // use extended destination ID                           
                    }
                    else {
                        ulExtendedID = (MB_TX_SEND_ON_REQ | ((unsigned long)Counter << 16));
                    }
                    ptrMessageBuffer->ulID = (ptrCanQue->ulPartnerID & ~CAN_EXTENDED_ID); // set tx message buffer to default ID address
                    ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_ACTIVE | CAN_TX_BUF_REMOTE | (CAN_TX_ACK_ON & ucTxMode)); // mark that the buffer is in use
                }
                else {
                    if ((ucTxMode & SPECIFIED_ID) != 0) {                // is the user specifying a destination ID or can default be used?
                        unsigned long ulPartnerID = 0;
                        if (ptBuffer == 0) {                             // {8}
                            continue;
                        }
                        ulPartnerID = *ptBuffer++;
                        ulPartnerID <<= 8;
                        ulPartnerID |= *ptBuffer++;
                        ulPartnerID <<= 8;
                        ulPartnerID |= *ptBuffer++;
                        ulPartnerID <<= 8;
                        ulPartnerID |= *ptBuffer++;
                        if ((ulPartnerID & CAN_EXTENDED_ID) != 0) {
                            ulExtendedID = IDE;
                            ptrMessageBuffer->ulID = (ulPartnerID & ~CAN_EXTENDED_ID); // send to specified extended ID address
                        }
                        else {
                            ptrMessageBuffer->ulID = ((ulPartnerID << CAN_STANDARD_SHIFT) & CAN_STANDARD_BITMASK); // send to specified standard ID address
                        }
                        Counter -= 4;
                    }
                    else {                                               // transmission to default ID
                        if ((ptBuffer == 0) && (Counter == 0)) {         // remote frame request -  after transmission the buffer will become a receiver until read or freed
                            if ((ptrCanQue->ulOwnID & CAN_EXTENDED_ID) != 0) {
                                ulExtendedID = (MB_TX_SEND_ONCE | RTR | IDE); // use extended destination ID
                            }
                            else {
                                ulExtendedID = (MB_TX_SEND_ONCE | RTR);
                            }
                            ptrMessageBuffer->ulID = (ptrCanQue->ulOwnID & ~CAN_EXTENDED_ID); // request from our default ID address
                            ptrCanQue->ucMode = (CAN_TX_BUF | CAN_RX_REMOTE_RX | CAN_TX_BUF_ACTIVE | CAN_TX_BUF_REMOTE);
                        }
                        else {
                            if ((ptrCanQue->ulPartnerID & CAN_EXTENDED_ID) != 0) {
                                ulExtendedID = IDE;                          // use extended destination ID
                            }
                            ptrMessageBuffer->ulID = (ptrCanQue->ulPartnerID & ~CAN_EXTENDED_ID); // send to default ID address
                        }
                    }
                    if (Counter > 8) {                                   // limit the count to maximum
                        Counter = 8;
                    }
                    if (Counter != 0) {                                  // data so it is not a remote frame request   
                        ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_ACTIVE | (CAN_TX_ACK_ON & ucTxMode)); // mark that the buffer is in use                    
                        ulExtendedID |= (MB_TX_SEND_ONCE | ((unsigned long)Counter << 16));
                    }
                }
                // The data needs to be stored as long words in big-endian format
                //
                if (ptBuffer != 0) {
                    ptrMessageBuffer->ulData[0] = ((*(ptBuffer) << 24) | (*(ptBuffer + 1) << 16) | (*(ptBuffer + 2) << 8) | (*(ptBuffer + 3)));
                    ptrMessageBuffer->ulData[1] = ((*(ptBuffer + 4) << 24) | (*(ptBuffer + 5) << 16) | (*(ptBuffer + 6) << 8) | (*(ptBuffer + 7)));
                }
                ptrCanQue->ucErrors = 0;
                ptrMessageBuffer->ulCode_Len_TimeStamp = ulExtendedID;   // start the process
    #if defined _WINDOWS
                fnSimCAN(Channel, i, 0);                                 // simulate the buffer operation
    #endif
                return (unsigned char)ucRtnCnt;
            }
            else if ((ptBuffer == 0) && ((ptrCanQue->ucMode & CAN_TX_BUF) != 0)) { // {8} blocked transmit buffers should be freed
                ptrMessageBuffer->ulCode_Len_TimeStamp = ((ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD) | MB_TX_INACTIVE); // stop transmission
                ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE);      // buffer free to be used again
                ucFreedTxBuffers++;                                      // count the number of blocked buffers that were freed
            }
        }
        ptrMessageBuffer++;
        ptrCanQue++;
    }
#endif
    return ucFreedTxBuffers;                                             // {8} no free buffer was found...(or the number of flushed buffers)
}


// The CAN driver calls this to collect a received message
//
extern unsigned char fnCAN_get_rx(QUEUE_HANDLE Channel, QUEUE_HANDLE DriverID, unsigned char *ptBuffer, QUEUE_TRANSFER Counter)
{
#if defined MSCAN_CAN_INTERFACE
    // To do...
    //
#else
    int i;
    unsigned char ucLength;
    KINETIS_CAN_BUF *ptrMessageBuffer;
    CANQUE *ptrCanQue;
    KINETIS_CAN_CONTROL *ptrCAN_control;
    unsigned char ucCommand = (unsigned char)Counter;
    switch (Channel) {                                                   // the CAN controller to use
    case 0:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
    #if defined _iMX
        ptrMessageBuffer = MBUFF63_ADD_0;                                // the last of 64 message buffers in the FlexCan module
    #else
        ptrMessageBuffer = MBUFF15_ADD_0;                                // the last of 16 message buffers in the FlexCan module
    #endif
        ptrCanQue = &can_queue[0][NUMBER_CAN_MESSAGE_BUFFERS - 1];
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        #if defined _iMX
        ptrMessageBuffer = MBUFF63_ADD_1;                                // the last of 64 message buffers in the FlexCan module
        #else
        ptrMessageBuffer = MBUFF15_ADD_1;                                // the last of 16 message buffers in the FlexCan module
        #endif
        ptrCanQue = &can_queue[1][NUMBER_CAN_MESSAGE_BUFFERS - 1];
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        #if defined _iMX
        ptrMessageBuffer = MBUFF63_ADD_2;                                // the last of 64 message buffers in the FlexCan module
        #else
        ptrMessageBuffer = MBUFF15_ADD_2;                                // the last of 16 message buffers in the FlexCan module
        #endif
        ptrCanQue = &can_queue[2][NUMBER_CAN_MESSAGE_BUFFERS - 1];
        break;
    #endif
    default:
        return 0;
    }

    for (i = 0; i < NUMBER_CAN_MESSAGE_BUFFERS; i++) {                   // search through receive buffers
        if (ptrCanQue->DriverID == DriverID) {                           // find a buffer belonging to us
            if (((ucCommand & FREE_CAN_RX_REMOTE) != 0) && (ptrCanQue->ucMode & CAN_RX_REMOTE_RX)) { // release a waiting remote frame receiver buffer
                ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_TX_INACTIVE | (ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_KEEP_CONTENTS));
                ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE);      // buffer is now a normal tx buffer again
    #if defined _WINDOWS
                fnSimCAN(Channel, ((NUMBER_CAN_MESSAGE_BUFFERS - 1) - i), CAN_SIM_FREE_BUFFER);
    #endif
                return 1;
            }

            if ((ptrCanQue->ucMode & CAN_RX_BUF_FULL) != 0) {            // if the buffer contains data
                if (((ucCommand & (GET_CAN_TX_ERROR | GET_CAN_TX_REMOTE_ERROR | GET_CAN_RX_REMOTE | FREE_CAN_RX_REMOTE)) == 0) // normal rx buffer read
                     || (((ucCommand & GET_CAN_TX_REMOTE_ERROR) != 0) && ((ptrCanQue->ucMode & CAN_TX_BUF_REMOTE) != 0)) // requesting remote error details
                     || (((ucCommand & GET_CAN_RX_REMOTE) != 0) && ((ptrCanQue->ucMode & CAN_RX_REMOTE_RX) != 0)) // specifically requesting a remote reception buffer
                     || (((ucCommand & GET_CAN_TX_ERROR) != 0) && ((ptrCanQue->ucMode & CAN_TX_BUF) != 0))) { // requesting errored tx buffer                    

                    if ((ucCommand & (GET_CAN_TX_ERROR | GET_CAN_TX_REMOTE_ERROR)) == 0) { // reception
                        while ((ptrMessageBuffer->ulCode_Len_TimeStamp & MB_RX_BUSY_BIT) != 0) {} // wait until buffer update has terminated

                        if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_RX_CODE_FIELD) == MB_RX_OVERRUN) { // the buffer is now locked
                            *ptBuffer = CAN_RX_OVERRUN;                  // receiver overrun - we have lost an intermediate message
                        }
                        else {
                            *ptBuffer = 0;
                        }

                        if ((ptrCanQue->ucMode & CAN_RX_REMOTE_RX) != 0) { // temporary rx buffer - so set back to tx
                            *ptBuffer++ |= CAN_REMOTE_MSG_RX; 
                            ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_TX_INACTIVE | (ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_LENGTH_AND_TIME));
                            ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE); // buffer is now a normal tx buffer again
                        }
                        else {                                                 
                            *ptBuffer++ |= CAN_MSG_RX;
                        }
                        if ((GET_CAN_RX_TIME_STAMP & ucCommand) != 0) {  // {70}
                            *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulCode_Len_TimeStamp >> 8); // write time stamp to input buffer
                            *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulCode_Len_TimeStamp);
                        }
                    }

                    if ((ucCommand & (GET_CAN_TX_ERROR | GET_CAN_TX_REMOTE_ERROR | GET_CAN_RX_ID)) != 0) {
                        unsigned long ulID = ptrMessageBuffer->ulID;
                        if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                            ulID |= CAN_EXTENDED_ID;                     // mark that it is an extended address
                        }
                        else {
                            ulID >>= CAN_STANDARD_SHIFT;                 // the address if a standard address
                        }
                        *ptBuffer++ = (unsigned char)(ulID >> 24);       // write failed ID to buffer or rx ID
                        *ptBuffer++ = (unsigned char)(ulID >> 16); 
                        *ptBuffer++ = (unsigned char)(ulID >> 8);
                        *ptBuffer++ = (unsigned char)(ulID);
                    }

                    ucLength = (unsigned char)((ptrMessageBuffer->ulCode_Len_TimeStamp >> 16) & 0x0f);
                    // The data needs is stored as long words in big-endian format
                    //
                    if (ucLength != 0) {
                        *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[0] >> 24);
                        if (ucLength > 1) {
                            *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[0] >> 16);
                            if (ucLength > 2) {
                                *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[0] >> 8);
                                if (ucLength > 3) {
                                    *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[0]);
                                    if (ucLength > 4) {
                                        *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[1] >> 24);
                                        if (ucLength > 5) {
                                            *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[1] >> 16);
                                            if (ucLength > 6) {
                                                *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[1] >> 8);
                                                if (ucLength > 7) {
                                                    *ptBuffer++ = (unsigned char)(ptrMessageBuffer->ulData[1]);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if ((ucCommand & (GET_CAN_TX_ERROR | GET_CAN_TX_REMOTE_ERROR)) != 0) {
                        ptrCanQue->ucMode = (CAN_TX_BUF | CAN_TX_BUF_FREE); // the buffer may be used again since the lost data has been rescued
                    }
                    else {
                        if ((GET_CAN_RX_TIME_STAMP & ucCommand) != 0) {  // {70}
                            ucLength += 3;                               // status and time stamp
                        }
                        else {
                            ucLength++;                                  // status only
                        }
                        if ((ptrCanQue->ucMode & CAN_TX_BUF_FREE) == 0) {// if not a remote reception read
                            ptrCanQue->ucMode &= (~CAN_RX_BUF_FULL);
                            ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_RX_EMPTY | (ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD));// free up buffer for further use
                        }
                    }
                    if ((ucCommand & (GET_CAN_TX_ERROR | GET_CAN_TX_REMOTE_ERROR | GET_CAN_RX_ID)) != 0) {
                        ucLength += 4;                                   // id length
                    }
                    ptrCAN_control->CAN_TIMER;                           // read the free running timer to release internal lock
                    return ucLength;                                     // the number of bytes copied, including OK byte and timestamp
                }
            }
        }
        ptrMessageBuffer--;
        ptrCanQue--;
    }
#endif
    return 0;                                                            // no data was found...
}

    #if defined _WINDOWS
extern UTASK_TASK fnGetCANOwner(int iChannel, int i)
{
    return (can_queue[iChannel][i].TaskToWake);
}
    #endif

