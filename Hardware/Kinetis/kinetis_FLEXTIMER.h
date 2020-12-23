/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_FLEXTIMER.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    21.06.2014 Adjust FlexTimer/TPM use of FTM_SC_TOF to clear interrupt correctly {1}
    22.07.2014 Add clock source selection to TPM                         {2}
    04.01.2017 Don't adjust the RC clock setting when the processor is running from it {3}
    26.01.2017 Add external clock selection for KL parts                 {4}
    26.04.2017 Add KL82 TPM clock input selection                        {5}
    20.05.2017 Add capture mode to Kinetis                               {6}
    07.02.2018 Ensure that the count value is reset to zero when starting timer {7}
    09.08.2018 Add control to freeze and release an operating timer      {8}
    09.08.2018 Add TPM1 and TPM2 extension (for K65/K66/K80)             {9}
    21.08.2018 Add flexible clocking (fixed clock source on individual modules) {10}
    21.08.2018 Correct FTM2 clock gating for K64, K65 and K66            {11}
    27.08.2018 Correct input capture mode pre-scaler setting             {12}
    01.11.2018 Correct clearing of capture flag for FlexTimer and TPM types {13}
    05.02.2020 Add optional flex timer capture callback for each channel {14}
    15.09.2020 Add option to generate a square wave output when configuring for periodic mode {15}

*/

#if defined _FLEXTIMER_CODE

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static __interrupt void _flexTimerInterrupt_0(void);
    #if FLEX_TIMERS_AVAILABLE > 1 && !defined NO_FLEX_TIMER_2
static __interrupt void _flexTimerInterrupt_1(void);
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
static __interrupt void _flexTimerInterrupt_2(void);
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3
static __interrupt void _flexTimerInterrupt_3(void);
    #endif
    #if FLEX_TIMERS_AVAILABLE > 4
static __interrupt void _flexTimerInterrupt_4(void);
    #endif
    #if FLEX_TIMERS_AVAILABLE > 5
static __interrupt void _flexTimerInterrupt_5(void);
    #endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined SUPPORT_CAPTURE
static const unsigned char ucChannelsAvailable[FLEX_TIMERS_AVAILABLE] = {// {13} the number of channels of each FlexTimer / TPM
    FLEX_TIMERS_0_CHANNELS,
    #if FLEX_TIMERS_AVAILABLE > 1
    FLEX_TIMERS_1_CHANNELS,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
    FLEX_TIMERS_2_CHANNELS,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3
    FLEX_TIMERS_3_CHANNELS,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 4
    FLEX_TIMERS_4_CHANNELS,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 5
    FLEX_TIMERS_5_CHANNELS,
    #endif
};
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static unsigned short usFlexTimerMode[FLEX_TIMERS_AVAILABLE] = {0};      // operating mode details of each FlexTimer
#if defined SUPPORT_CAPTURE && defined CAPTURE_CHANNEL_USER_DISPATCHER && !defined CAPTURE_CHANNEL_CALLBACK
static void (*_flexTimerHandler[FLEX_TIMERS_AVAILABLE])(unsigned char ucCaptureChannels) = {0}; // user interrupt handlers
#else
static void (*_flexTimerHandler[FLEX_TIMERS_AVAILABLE])(void) = {0};     // user interrupt handlers
#endif
#if defined CAPTURE_CHANNEL_CALLBACK
    static void(*_CaptureHandler[FLEX_TIMERS_CHANNEL_COUNT])(void) = {0};// user interrupt handlers
#endif
static unsigned char ucClockSource[FLEX_TIMERS_AVAILABLE] = {0};         // {8} clock source backup for each FlexTimer

static void (*_flexTimerInterrupt[FLEX_TIMERS_AVAILABLE])(void) = {      // user interrupt callbacks
    _flexTimerInterrupt_0,
    #if FLEX_TIMERS_AVAILABLE > 1
        #if defined NO_FLEX_TIMER_2
    0,                                                                   // dummy when the timer is not available
        #else
    _flexTimerInterrupt_1,
        #endif
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
    _flexTimerInterrupt_2,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3
    _flexTimerInterrupt_3,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 4
    _flexTimerInterrupt_4,
    #endif
    #if FLEX_TIMERS_AVAILABLE > 5
    _flexTimerInterrupt_5,
    #endif
};



/* =================================================================== */
/*                  FlexTimer Interrupt Handlers                       */
/* =================================================================== */

#if defined CAPTURE_CHANNEL_CALLBACK
static int fnGetChannelRef(int iTimerReference)
{
    int iChannelRef = 0;
    int iFlexTimer = 0;
    while (iFlexTimer < iTimerReference) {                               // calculate the reference for the first channel of this flex timer
        iChannelRef += ucChannelsAvailable[iFlexTimer++];
    }
    return iChannelRef;
}
#endif

// Generic interrupt handling
//
static void fnHandleFlexTimer(FLEX_TIMER_MODULE *ptrFlexTimer, int iFlexTimerReference)
{
#if defined SUPPORT_CAPTURE
    #if defined CAPTURE_CHANNEL_USER_DISPATCHER && !defined CAPTURE_CHANNEL_CALLBACK
    unsigned char ucCaptureChannels = 0;
    #endif
    if ((usFlexTimerMode[iFlexTimerReference] & FTM_SC_TOIE) == 0) {     // capture mode rather than timer overflow mode
        unsigned char ucChannel;
    #if defined CAPTURE_CHANNEL_CALLBACK
        int iChannelHandlerRef = fnGetChannelRef(iFlexTimerReference);
    #endif
        for (ucChannel = 0; ucChannel < ucChannelsAvailable[iFlexTimerReference]; ucChannel++) { // {13}
            if ((ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC & (FTM_CSC_CHIE | FTM_CSC_CHF)) == (FTM_CSC_CHIE | FTM_CSC_CHF)) { // if the channel interrupt is enabled and its event flag set
    #if defined TPMS_AVAILABLE_TOO                                       // device with both flex timers and TPM
                if (iFlexTimerReference < (FLEX_TIMERS_AVAILABLE - TPMS_AVAILABLE_TOO)) { // if flex timer type
                    ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC = (ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC & ~(FTM_CSC_CHF)); // clear the interrupt (by writing FTM_CSC_CHF to '0') - this allows the next capture event to be released
                }
                else {                                                   // else TPM type
                    ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC = ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC; // clear the interrupt (by writing FTM_CSC_CHF to '1') - this allows the next capture event to be released
                }
    #elif !defined TPMS_AVAILABLE                                        // flex timer type
                ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC = (ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC & ~(FTM_CSC_CHF)); // clear the interrupt (by writing FTM_CSC_CHF to '0') - this allows the next capture event to be released
    #else                                                                // TPM type
                ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC = ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC; // clear the interrupt (by writing FTM_CSC_CHF to '1') - this allows the next capture event to be released
    #endif
    #if defined _WINDOWS
                ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC &= ~(FTM_CSC_CHF);
    #endif
    #if defined CAPTURE_CHANNEL_CALLBACK
                if (_CaptureHandler[iChannelHandlerRef + ucChannel] != 0) {
                    uDisable_Interrupt();
                        _CaptureHandler[iChannelHandlerRef + ucChannel](); // call user handler for this capture channel
                    uEnable_Interrupt();
                }
    #elif defined CAPTURE_CHANNEL_USER_DISPATCHER
                ucCaptureChannels |= (1 << ucChannel);
    #endif
            }
        }
    #if defined CAPTURE_CHANNEL_CALLBACK
        return;
    #endif
    }
    else
#endif
    if ((ptrFlexTimer->FTM_SC & FTM_SC_TOF) != 0) {                      // flag will always be set but it has to be read at '1' before it can be reset
        if ((usFlexTimerMode[iFlexTimerReference] & FLEX_TIMER_PERIODIC) != 0) { // if the timer is being used in periodic mode
            ptrFlexTimer->FTM_SC = (usFlexTimerMode[iFlexTimerReference] & FTM_SC_USED_MASK); // reset interrupt and allow the FlexTimer to continue running for periodic interrupts
        }
        else {                                                           // single shot mode
            ptrFlexTimer->FTM_SC = FTM_SC_TOF;                           // stop further activity (single-shot mode)
            switch (iFlexTimerReference) {                               // power down the FlexTimer after single-shot use
            case 0:
                POWER_DOWN_ATOMIC(6, FTM0);
                break;
    #if FLEX_TIMERS_AVAILABLE > 1 && !defined NO_FLEX_TIMER_2
            case 1:
                POWER_DOWN_ATOMIC(6, FTM1);
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
            case 2:
        #if defined KINETIS_KL ||defined KINETIS_K22_SF7 || defined KINETIS_K64 || defined KINETIS_K65 || defined KINETIS_K66 || defined KINETIS_KV50
                POWER_DOWN_ATOMIC(6, FTM2);
        #else
                POWER_DOWN_ATOMIC(3, FTM2);
        #endif
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3
            case 3:
        #if defined KINETIS_K22_SF7
                POWER_DOWN_ATOMIC(6, FTM3);
        #else
                POWER_DOWN_ATOMIC(3, FTM3);
        #endif
                break;
    #endif
            }
        }
    }
    #if defined _WINDOWS
    ptrFlexTimer->FTM_SC &= ~FTM_SC_TOF;
    #endif
    if (_flexTimerHandler[iFlexTimerReference] != 0) {                   // if there is a user handler
        uDisable_Interrupt();
    #if defined SUPPORT_CAPTURE && defined CAPTURE_CHANNEL_USER_DISPATCHER && !defined CAPTURE_CHANNEL_CALLBACK
        _flexTimerHandler[iFlexTimerReference](ucCaptureChannels);       // call user handler
    #else
            _flexTimerHandler[iFlexTimerReference]();                    // call user handler
    #endif
        uEnable_Interrupt();
    }
}

static __interrupt void _flexTimerInterrupt_0(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_0, 0);
}

    #if FLEX_TIMERS_AVAILABLE > 1 && !defined NO_FLEX_TIMER_2
static __interrupt void _flexTimerInterrupt_1(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_1, 1);
}
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
static __interrupt void _flexTimerInterrupt_2(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_2, 2);
}
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3
static __interrupt void _flexTimerInterrupt_3(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_3, 3);
}
    #endif
    #if FLEX_TIMERS_AVAILABLE > 4
static __interrupt void _flexTimerInterrupt_4(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_4, 4);
}
    #endif
    #if FLEX_TIMERS_AVAILABLE > 5
static __interrupt void _flexTimerInterrupt_5(void)
{
    fnHandleFlexTimer((FLEX_TIMER_MODULE *)FTM_BLOCK_5, 5);
}
    #endif
#endif


/* =================================================================== */
/*                     FlexTimer Configuration                         */
/* =================================================================== */


#if defined _FLEXTIMER_CONFIG_CODE
        {
            TIMER_INTERRUPT_SETUP *ptrTimerSetup = (TIMER_INTERRUPT_SETUP *)ptrSettings;
            int iInterruptID;
            register int iTimerReference = ptrTimerSetup->timer_reference;
            register unsigned long ulDelay = ptrTimerSetup->timer_value;
            register int iPrescaler = 0;
            FLEX_TIMER_MODULE *ptrFlexTimer;
            int iReducedFunctionality = 0;
    #if defined KINETIS_KL
        #if !defined KINETIS_WITH_PCC
            unsigned long ulExtSelect;
            #if defined TPM_CLOCKED_FROM_MCGIRCLK                        // {2}
                #if !defined RUN_FROM_LIRC                               // {3} if the processor is running from the the internal clock we don't adjust settings here
            MCG_C1 |= (MCG_C1_IRCLKEN | MCG_C1_IREFSTEN);                // enable internal reference clock and allow it to continue running in stop modes
                    #if defined USE_FAST_INTERNAL_CLOCK
            MCG_SC = 0;                                                  // remove fast IRC divider
            MCG_C2 |= MCG_C2_IRCS;                                       // select fast internal reference clock (4MHz/8MHz)
                    #else
            MCG_C2 &= ~MCG_C2_IRCS;                                      // select slow internal reference clock (32kHz/2MHz)
                    #endif
                #endif
            SIM_SOPT2 |= SIM_SOPT2_TPMSRC_MCGIRCLK;                      // use MCGIRCLK as timer clock source
            #elif defined TPM_CLOCKED_FROM_IRC48M && defined SIM_SOPT2_PLLFLLSEL_IRC48M && (SIM_SOPT2_PLLFLLSEL_IRC48M != 0)
            SIM_SOPT2 |= (SIM_SOPT2_PLLFLLSEL_IRC48M | SIM_SOPT2_TPMSRC_MCG); // use IRC48M
            #elif defined TPM_CLOCKED_FROM_OSCERCLK
            OSC0_CR |= (OSC_CR_ERCLKEN | OSC_CR_EREFSTEN);               // enable the external reference clock and keep it enabled in stop mode
            SIM_SOPT2 |= (SIM_SOPT2_TPMSRC_OSCERCLK);                    // use OSCERCLK as timer clock source
            #else
            SIM_SOPT2 |= (SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TPMSRC_MCG);   // use MCGPLLCLK/2 (or MCGFLL if FLL is used) - devices with KINETIS_WITH_MCG_LITE and HIRC48M will use 48MHz
            #endif
        #endif
    #endif
            switch (iTimerReference) {                                   // FlexTimer/TPM to be used
            case 0:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
    #if defined KINETIS_WITH_PCC
                    PCC_FTM0 = 0;                                        // disable clocks to module
    #else
                    POWER_DOWN_ATOMIC(6, FTM0);
    #endif
                    return;
                }
    #if defined KINETIS_WITH_PCC && !defined KINETIS_KE15
                SELECT_PCC_PERIPHERAL_SOURCE(FTM0, FTM0_PCC_SOURCE);     // select the PCC clock used by FlexTimer/TPM 0
    #endif
                POWER_UP_ATOMIC(6, FTM0);                                // ensure that the FlexTimer module is powered up
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;         // KL and KE parts actually use the TPM which is however very similar to the FlexTimer
    #if defined KINETIS_KL
                iInterruptID = irq_TPM0_ID;
        #if !defined KINETIS_WITH_PCC
            #if defined KINETIS_KL82                                     // {5}
                ulExtSelect = SIM_SOPT9_TPM0CLKSEL;
            #else
                ulExtSelect = SIM_SOPT4_FTM0CLKSEL;
            #endif
        #endif
    #else
                iInterruptID = irq_FTM0_ID;
    #endif
    #if defined FLEX_TIMER_0_REDUCED
                iReducedFunctionality = 1;
    #endif
                break;
    #if FLEX_TIMERS_AVAILABLE > 1 && !defined NO_FLEX_TIMER_2
            case 1:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
        #if defined KINETIS_WITH_PCC
                    PCC_FTM1 = 0;                                        // disable clocks to module
        #else
                    POWER_DOWN_ATOMIC(6, FTM1);
        #endif
                    return;
                }
        #if defined KINETIS_WITH_PCC && !defined KINETIS_KE15
                SELECT_PCC_PERIPHERAL_SOURCE(FTM1, FTM1_PCC_SOURCE);     // select the PCC clock used by FlexTimer/TPM 1
        #endif
                POWER_UP_ATOMIC(6, FTM1);                                // ensure that the FlexTimer module is powered up
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_1;         // KL and KE parts actually use the TPM which is however very similar to the FlexTimer
        #if defined KINETIS_KL
                iInterruptID = irq_TPM1_ID;
            #if !defined KINETIS_WITH_PCC
                #if defined KINETIS_KL82                                 // {5}
                ulExtSelect = SIM_SOPT9_TPM1CLKSEL;
                #else
                ulExtSelect = SIM_SOPT4_FTM1CLKSEL;
                #endif
            #endif
        #else
                iInterruptID = irq_FTM1_ID;
        #endif
        #if defined FLEX_TIMER_1_REDUCED
                iReducedFunctionality = 1;
        #endif
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 2
            case 2:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
        #if defined KINETIS_WITH_PCC
                    PCC_FTM2 = 0;                                        // disable clocks to module
        #else
            #if defined KINETIS_KL || defined KINETIS_K22_SF7 || defined KINETIS_K64 || defined KINETIS_K65 || defined KINETIS_K66 || defined KINETIS_KV50
                    POWER_DOWN_ATOMIC(6, FTM2);
            #else
                    POWER_DOWN_ATOMIC(3, FTM2);
            #endif
        #endif
                    return;
                }
        #if defined KINETIS_WITH_PCC && !defined KINETIS_KE15
                SELECT_PCC_PERIPHERAL_SOURCE(FTM2, FTM2_PCC_SOURCE);     // select the PCC clock used by FlexTimer/TPM 2
        #endif
        #if defined KINETIS_KL || defined KINETIS_K22_SF7 || defined KINETIS_K64 || defined KINETIS_K65 || defined KINETIS_K66 || defined KINETIS_KV50 // {11}
                POWER_UP_ATOMIC(6, FTM2);                                // ensure that the FlexTimer module is powered up
        #else
                POWER_UP_ATOMIC(3, FTM2);                                // ensure that the FlexTimer module is powered up
        #endif
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_2;         // KL and KE parts actually use the TPM which is however very similar to the FlexTimer
        #if defined KINETIS_KL
                iInterruptID = irq_TPM2_ID;
            #if !defined KINETIS_WITH_PCC
                #if defined KINETIS_KL82                                 // {5}
                ulExtSelect = SIM_SOPT9_TPM2CLKSEL;
                #else
                ulExtSelect = SIM_SOPT4_FTM2CLKSEL;
                #endif
            #endif
        #else
                iInterruptID = irq_FTM2_ID;
        #endif
        #if defined FLEX_TIMER_2_REDUCED
                iReducedFunctionality = 1;
        #endif
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 3 && !defined KINETIS_KE18
            case 3:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
        #if defined KINETIS_WITH_PCC
                    PCC_FTM3 = 0;                                        // disable clocks to module
        #elif defined KINETIS_K22_SF7
                    POWER_DOWN_ATOMIC(6, FTM3);
        #else
                    POWER_DOWN_ATOMIC(3, FTM3);
        #endif
                    return;
                }
        #if defined KINETIS_WITH_PCC && !defined KINETIS_KE15
                SELECT_PCC_PERIPHERAL_SOURCE(FTM3, FTM3_PCC_SOURCE);     // select the PCC clock used by FlexTimer/TPM 3
        #endif
        #if defined KINETIS_K22_SF7
                POWER_UP_ATOMIC(6, FTM3);                                // ensure that the FlexTimer module is powered up
        #else
                POWER_UP_ATOMIC(3, FTM3);                                // ensure that the FlexTimer module is powered up
        #endif
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_3;
                iInterruptID = irq_FTM3_ID;
        #if defined FLEX_TIMER_3_REDUCED
                iReducedFunctionality = 1;
        #endif
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 4                                        // {9} K65/K66/K80 - TPM1
            case 4:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
                    POWER_DOWN_ATOMIC(2, TPM1);
                    return;
                }
                POWER_UP_ATOMIC(2, TPM1);                                // ensure that the FlexTimer module is powered up
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_4;
                iInterruptID = irq_TPM1_ID;
        #if defined FLEX_TIMER_4_REDUCED
                iReducedFunctionality = 1;
        #endif
                break;
    #endif
    #if FLEX_TIMERS_AVAILABLE > 5                                        // {9} K65/K66/K80 - TPM2
            case 5:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
                    POWER_DOWN_ATOMIC(2, TPM2);
                    return;
                }
                POWER_UP_ATOMIC(2, TPM2);                                // ensure that the FlexTimer module is powered up
                ptrFlexTimer = (FLEX_TIMER_MODULE *)FTM_BLOCK_5;
                iInterruptID = irq_TPM2_ID;
        #if defined FLEX_TIMER_4_REDUCED
                iReducedFunctionality = 1;
        #endif
                break;
    #endif
            default:
                _EXCEPTION("FlexTimer is unavailable!!");
                return;
            }
            if ((ptrTimerSetup->timer_mode & (TIMER_FREEZE | TIMER_CONTINUE)) != 0) { // {8}
                if ((ptrTimerSetup->timer_mode & TIMER_FREEZE) != 0) {
                    ucClockSource[iTimerReference] = (unsigned char)(ptrFlexTimer->FTM_SC & FTM_SC_CLKS_MASK); // back-up the original clock source
                    ptrFlexTimer->FTM_SC &= ~(FTM_SC_CLKS_MASK);         // disable the clock so that the timer freezes
                }
                else {
                    ptrFlexTimer->FTM_SC |= ucClockSource[iTimerReference]; // restore the clock so that the timer continues to run again
                }
                return;                                                  // work complete
            }
            ptrFlexTimer->FTM_SC = 0;                                    // ensure not operating
            if ((ptrFlexTimer->FTM_SC & FTM_SC_TOF) != 0) {              // ensure no pending interrupt
                ptrFlexTimer->FTM_SC = 0;                                // clear pending interrupt (requires a read of the interrupt bit at '1' beforehand)
            }
            if (iReducedFunctionality == 0) {                            // don't configure debugger behaviour on timers with reduced functionality
                ptrFlexTimer->FTM_CONF = FTM_DEBUG_BEHAVIOUR;            // set the debugging behaviour (whether the counter runs in debug mode and how the outputs react)
            }
    #if (!defined KINETIS_KL && !defined KINETIS_KE) || (defined KINETIS_KE06 || defined KINETIS_KE14 || defined KINETIS_KE15 || defined KINETIS_KE16 || defined KINETIS_KE18)
        #if defined KINETIS_KE06
            if (iReducedFunctionality == 0) {
        #endif
                ptrFlexTimer->FTM_CNTIN = 0;                             // counter start value
        #if defined KINETIS_KE06
            }
        #endif
            ptrFlexTimer->FTM_CNT = 0;                                   // {7} cause the counter to be set to zero
    #endif
    #if defined SUPPORT_CAPTURE
            if ((ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) != 0) { // {6} if capture mode is required
                unsigned long ulCharacteristics = PORT_PS_UP_ENABLE;
                unsigned long ulEdge;
        #if defined _WINDOWS                                             // {12} check valid pre-scaler values
                switch (ptrTimerSetup->capture_prescaler) {
                case 1:
                case 2:
                case 4:
                case 8:
                case 16:
                case 32:
                case 64:
                case 128:
                    break;
                default:
                    _EXCEPTION("Invalid prescaler in input capture mode!! (1,2,4,8,16,32,64 or 128 allowed)");
                    break;
                }
        #endif
                ulDelay = (0xffff * ptrTimerSetup->capture_prescaler);   // {12} set maximum count value so that the timer free-runs
                switch (ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) {
                case TIMER_CAPTURE_RISING_FALLING:
                    ulEdge = (FTM_CSC_ELSA | FTM_CSC_ELSB);              // capture on rising and falling edges
                    break;
                case TIMER_CAPTURE_RISING:
        #if defined PORT_PS_DOWN_ENABLE
                    ulCharacteristics = PORT_PS_DOWN_ENABLE;
        #endif
                    ulEdge = FTM_CSC_ELSA;                               // capture on rising edge
                    break;
              //case TIMER_CAPTURE_FALLING:
                default:
                    ulEdge = FTM_CSC_ELSB;                               // capture on falling edge
                    break;
                }
        #if defined KINETIS_KL_ && defined _WINDOWS
                if (ptrTimerSetup->capture_channel > 3) {
                    _EXCEPTION("Invalid capture channel");
                }
        #endif
                fnConfigTimerPin(iTimerReference, ptrTimerSetup->capture_channel, ulCharacteristics); // configure the channel's input capture pin
                ptrFlexTimer->FTM_channel[ptrTimerSetup->capture_channel].FTM_CSC = ulEdge; // program the edge sensitivity of the capture input
        #if defined KINETIS_KL_                                          // flextimers default to their external channel inputs
                ptrFlexTimer->FTM_CONF |= (FTM_CONF_TRGSEL0 << ptrTimerSetup->capture_channel); // enable trigger source
        #endif
            }
    #endif
            while (ulDelay > 0xffff) {                                   // calculate the optimal prescaler setting
                if (iPrescaler >= 7) {
                    ulDelay = 0xffff;                                    // set maximum delay
                    break;
                }
                iPrescaler++;
                ulDelay /= 2;
            }
          //iPrescaler &= 0x7f;
            usFlexTimerMode[iTimerReference] = (unsigned short)iPrescaler;
            if ((ptrTimerSetup->timer_mode & TIMER_PERIODIC) != 0) {     // if periodic operation required
                usFlexTimerMode[iTimerReference] |= FLEX_TIMER_PERIODIC; // mark that periodic mode is being used
                if ((ptrTimerSetup->timer_mode & TIMER_SQUARE_WAVE) != 0) { // if  square wave output is to be enabled
    #if defined SUPPORT_TIMER_SQUARE_WAVE                                // {15}
                    ptrFlexTimer->FTM_channel[0].FTM_CV = ulDelay;       // set upper count value on first channel
                    ptrFlexTimer->FTM_channel[0].FTM_CSC = (FTM_CSC_MSA | FTM_CSC_ELSA); // toggle channel output on each match
                    fnConfigTimerPin(iTimerReference, 0, (PORT_SRE_FAST | PORT_DSE_HIGH)); // configure the compare output pin
    #endif
                }
            }
            ptrFlexTimer->FTM_MOD = ulDelay;                             // set upper count value
    #if defined KINETIS_KL
            if ((ptrTimerSetup->timer_mode & (TIMER_EXT_CLK_0 | TIMER_EXT_CLK_1)) != 0) { // {4} the external clock source is to be used
                usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_EXT | FTM_SC_TOIE | FTM_SC_TOF); // select external clock (which should be half the speed of the module's clock due to synchronisation requirements)
        #if defined KINETIS_KL28                                         // select extenal clock input which increments the counter on each rising edge (synchronous to module clock)
                switch (iTimerReference) {
                case 0:
            #if defined TMP0_CLKIN_ON_A
                    _CONFIG_PERIPHERAL(A, 18, (PA_18_TPM0_CLKIN | PORT_PS_UP_ENABLE)); // TPM0_CLKIN on PA.18 (alt. function 4)
            #elif defined TMP0_CLKIN_ON_B
                    _CONFIG_PERIPHERAL(B, 16, (PB_16_TPM0_CLKIN | PORT_PS_UP_ENABLE)); // TPM0_CLKIN on PB.16 (alt. function 4)
            #elif defined TMP0_CLKIN_ON_C
                    _CONFIG_PERIPHERAL(C, 12, (PC_12_TPM0_CLKIN | PORT_PS_UP_ENABLE)); // TPM0_CLKIN on PC.12 (alt. function 4)
            #elif defined TMP0_CLKIN_ON_E_LOW
                    _CONFIG_PERIPHERAL(E, 16, (PE_16_TPM0_CLKIN | PORT_PS_UP_ENABLE)); // TPM0_CLKIN on PE.16 (alt. function 4)
            #else
                    _CONFIG_PERIPHERAL(E, 29, (PE_29_TPM0_CLKIN | PORT_PS_UP_ENABLE)); // TPM0_CLKIN on PE.29 (alt. function 4)
            #endif
                    break;
                case 1:
            #if defined TMP1_CLKIN_ON_A
                    _CONFIG_PERIPHERAL(A, 19, (PA_19_TPM1_CLKIN | PORT_PS_UP_ENABLE)); // TPM1_CLKIN on PA.19 (alt. function 4)
            #elif defined TMP1_CLKIN_ON_B
                    _CONFIG_PERIPHERAL(B, 17, (PB_17_TPM1_CLKIN | PORT_PS_UP_ENABLE)); // TPM1_CLKIN on PB.17 (alt. function 4)
            #elif defined TMP1_CLKIN_ON_C
                    _CONFIG_PERIPHERAL(C, 13, (PC_13_TPM1_CLKIN | PORT_PS_UP_ENABLE)); // TPM1_CLKIN on PC.13 (alt. function 4)
            #elif defined TMP1_CLKIN_ON_E_LOW
                    _CONFIG_PERIPHERAL(E, 17, (PE_17_TPM1_CLKIN | PORT_PS_UP_ENABLE)); // TPM1_CLKIN on PE.17 (alt. function 4)
            #else
                    _CONFIG_PERIPHERAL(E, 30, (PE_30_TPM1_CLKIN | PORT_PS_UP_ENABLE)); // TPM1_CLKIN on PE.30 (alt. function 4)
            #endif
                    break;
                case 2:
            #if defined TMP2_CLKIN_ON_A
                    _CONFIG_PERIPHERAL(A, 20, (PA_20_TPM2_CLKIN | PORT_PS_UP_ENABLE)); // TPM2_CLKIN on PA.20 (alt. function 4)
            #elif defined TMP2_CLKIN_ON_B
                    _CONFIG_PERIPHERAL(B, 11, (PB_11_TPM2_CLKIN | PORT_PS_UP_ENABLE)); // TPM2_CLKIN on PB.11 (alt. function 4)
            #else
                    _CONFIG_PERIPHERAL(E, 31, (PE_31_TPM2_CLKIN | PORT_PS_UP_ENABLE)); // TPM2_CLKIN on PE.31 (alt. function 4)
            #endif
                    break;
                }
        #else
                if ((ptrTimerSetup->timer_mode & (TIMER_EXT_CLK_1)) != 0) {
            #if !defined KINETIS_WITH_PCC
                #if defined KINETIS_KL82                                     // {5}
                    SIM_SOPT9 |= ulExtSelect;                            // select CLKIN1 source to this timer
                #else
                    SIM_SOPT4 |= ulExtSelect;                            // select CLKIN1 source to this timer
                #endif
            #endif
            #if defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL05
                    _CONFIG_PERIPHERAL(B, 6, (PB_6_TPM_CLKIN1 | PORT_PS_UP_ENABLE)); // TPM_CLKIN1 on PB.6 (alt. function 3)
            #else
                    _CONFIG_PERIPHERAL(E, 30, (PE_30_TPM_CLKIN1 | PORT_PS_UP_ENABLE)); // TPM_CLKIN1 on PE.30 (alt. function 4)
            #endif
                }
                else {
            #if !defined KINETIS_WITH_PCC
                #if defined KINETIS_KL82                                  // {5}
                    SIM_SOPT9 &= ~(ulExtSelect);                         // select CLKIN0 source to this timer
               #else
                    SIM_SOPT4 &= ~(ulExtSelect);                         // select CLKIN0 source to this timer
                #endif
            #endif
            #if defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL05
                    _CONFIG_PERIPHERAL(A, 12, (PA_12_TPM_CLKIN0 | PORT_PS_UP_ENABLE)); // TPM_CLKIN0 on PA.12 (alt. function 3)
                #if defined KINETIS_KL05
                  //_CONFIG_PERIPHERAL(A, 14, (PA_14_TPM_CLKIN0 | PORT_PS_UP_ENABLE)); // TPM_CLKIN0 on PA.14 (alt. function 3)
                #endif
                  //_CONFIG_PERIPHERAL(A, 1, (PA_1_TPM_CLKIN0 | PORT_PS_UP_ENABLE)); // TPM_CLKIN0 on PA.1 (alt. function 2)
            #else
                    _CONFIG_PERIPHERAL(E, 29, (PE_29_TPM_CLKIN0 | PORT_PS_UP_ENABLE)); // TPM_CLKIN0 on PE.29 (alt. function 4)
            #endif
                }
        #endif
            }
            else {
                usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_SYS | FTM_SC_TOF); // set mode to start (shared by all channels) - system clock with overflow interrupt enabled [FTM_SC_TOF must be written with 1 to clear]
            }
    #elif defined FTM_FLEXIBLE_CLOCKING                                  // {11} support mixed individual clock sources (MCGFFCLK was set up during global clock initialisation)
            usFlexTimerMode[iTimerReference] &= ~(FTM_SC_CLKS_MASK);
            if ((ptrTimerSetup->timer_mode & TIMER_FIXED_CLK) != 0) {
                usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_FIX);   // set fixed clock for this channel
            }
            else {
                usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_SYS);   // set mode to start (for this channel) - system clock with overflow interrupt enabled [FTM_SC_TOF must be written with 0 to clear]
            }
    #elif defined FTM_CLOCKED_FROM_MCGFFLCLK
            // Ensure that slow internal clock is enabled and selected for MCGFFCLK
            //
            MCG_C1 |= (MCG_C1_IRCLKEN | MCG_C1_IREFSTEN | MCG_C1_IREFS); // enable internal reference clock and allow it to continue running in stop modes, plus select it as FLL reference (MCGFFLCLK)
            usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_FIX);       // set fixed clock
    #else
            usFlexTimerMode[iTimerReference] |= (FTM_SC_CLKS_SYS);       // {1} set mode to start (shared by all channels) - system clock with overflow interrupt enabled [FTM_SC_TOF must be written with 0 to clear]
    #endif
            if ((_flexTimerHandler[iTimerReference] = ptrTimerSetup->int_handler) != 0) { // enter flex timer user interrupt handler (not used in capture mode
                fnEnterInterrupt(iInterruptID, ptrTimerSetup->int_priority, _flexTimerInterrupt[iTimerReference]); // enter flex timer interrupt handler
    #if defined SUPPORT_CAPTURE
                if ((ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) != 0) {
        #if defined CAPTURE_CHANNEL_CALLBACK
                    _CaptureHandler[fnGetChannelRef(iTimerReference) + ptrTimerSetup->capture_channel] = ptrTimerSetup->int_handler;  // enter the capture channel's user callback
        #endif
                    ptrFlexTimer->FTM_channel[ptrTimerSetup->capture_channel].FTM_CSC |= FTM_CSC_CHIE; // enable channel interrupt
                }
                else {
    #endif
                    usFlexTimerMode[iTimerReference] |= FTM_SC_TOIE;     // enable timer overflow interrupt
    #if defined SUPPORT_CAPTURE
                }
    #endif
            }
    #if !defined DEVICE_WITHOUT_DMA
            if ((ptrTimerSetup->timer_mode & TIMER_DMA_TRIGGER) != 0) {  // when DMA required
                ptrFlexTimer->FTM_channel[0].FTM_CV = 0;                 // set the match value for channel 0
                ptrFlexTimer->FTM_channel[0].FTM_CSC = FTM_CSC_DMA;      // enable DMA trigger on match
            }
    #endif
            ptrFlexTimer->FTM_SC = (usFlexTimerMode[iTimerReference] & FTM_SC_USED_MASK); // start timer
        }
#endif

