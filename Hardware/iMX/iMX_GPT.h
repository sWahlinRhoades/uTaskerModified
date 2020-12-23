/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_GPT.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#if defined _GPT_CODE

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static __interrupt void _GPT_Interrupt_1(void);
static __interrupt void _GPT_Interrupt_2(void);


/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined SUPPORT_CAPTURE_
static const unsigned char ucChannelsAvailable[GPT_AVAILABLE] = {// the number of channels of each GPT
    2, 2
};
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static unsigned long ulGPTimerMode[GPT_AVAILABLE] = {0};                 // operating mode details of each GPT
static void (*_GP_TimerHandler[GPT_AVAILABLE])(void) = {0};              // user interrupt handlers
static unsigned short usClockSource[GPT_AVAILABLE] = {0};                // clock source backup for each GPT

static void (*_GPTimerInterrupt[GPT_AVAILABLE])(void) = {
    _GPT_Interrupt_1,
    _GPT_Interrupt_2
};


/* =================================================================== */
/*                      GPT Interrupt Handlers                         */
/* =================================================================== */

// Generic interrupt handling
//
static void fnHandleGPT(GPTIMER_MODULE *ptrGPT_Timer, int iGPTReference)
{
#if defined SUPPORT_CAPTURE_
    if ((usFlexTimerMode[iFlexTimerReference] & FTM_SC_TOIE) == 0) {     // capture mode rather than timer overflow mode
        unsigned char ucChannel;
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
            }
        }
    }
    else
#endif
    if ((ulGPTimerMode[iGPTReference] & TIMER_PERIODIC) != 0) {          // if the timer is being used in periodic mode
        WRITE_ONE_TO_CLEAR(ptrGPT_Timer->GPT_SR, (GPT_SR_OF1));          // clear the status flag
    }
    else {
        ptrGPT_Timer->GPT_CR = 0;                                        // stop further activity (single-shot mode)
        switch (iGPTReference) {                                         // power down the GPT after single-shot use
        case 0:
            POWER_DOWN_ATOMIC(1, GPT1_SERIAL_CLOCK);
            POWER_DOWN_ATOMIC(1, GPT1_BUS_CLOCK);                        // disable clocks to module
            break;
        case 1:
            POWER_UP_ATOMIC(0, GPT2_SERIAL_CLOCK);                       // ensure that the GPT module is powered up
            POWER_UP_ATOMIC(0, GPT2_BUS_CLOCK);
            break;
        }
    }
#if 0
    if ((ptrFlexTimer->FTM_SC & FTM_SC_TOF) != 0) {                      // flag will always be set but it has to be read at '1' before it can be reset
        if ((usFlexTimerMode[iFlexTimerReference] & FLEX_TIMER_PERIODIC) != 0) { // if the timer is being used in periodic mode
            ptrFlexTimer->FTM_SC = (usFlexTimerMode[iFlexTimerReference] & FTM_SC_USED_MASK); // reset interrupt and allow the FlexTimer to continue running for periodic interrupts
        }
        else {                                                           // single shot mode
            ptrFlexTimer->FTM_SC = FTM_SC_TOF;                           // stop further activity (single-shot mode)
            switch (iGPTReference) {                                     // power down the GPT after single-shot use
            case 0:
                POWER_DOWN_ATOMIC(1, GPT1_SERIAL_CLOCK);
                POWER_DOWN_ATOMIC(1, GPT1_BUS_CLOCK);                    // disable clocks to module
                break;
            case 1:
                POWER_UP_ATOMIC(0, GPT2_SERIAL_CLOCK);                   // ensure that the GPT module is powered up
                POWER_UP_ATOMIC(0, GPT2_BUS_CLOCK);
                break;
            }
        }
    }
    #if defined _WINDOWS
    ptrFlexTimer->FTM_SC &= ~FTM_SC_TOF;
    #endif
#endif
    if (_GP_TimerHandler[iGPTReference] != 0) {                         // if there is a user handler
        uDisable_Interrupt();
            _GP_TimerHandler[iGPTReference]();                          // call user handler
        uEnable_Interrupt();
    }
}

static __interrupt void _GPT_Interrupt_1(void)
{
    fnHandleGPT((GPTIMER_MODULE *)GPT1_BLOCK, 0);
}

static __interrupt void _GPT_Interrupt_2(void)
{
    fnHandleGPT((GPTIMER_MODULE *)GPT2_BLOCK, 1);
}
#endif


/* =================================================================== */
/*                     FlexTimer Configuration                         */
/* =================================================================== */


#if defined _GPT_CONFIG_CODE
        {
            TIMER_INTERRUPT_SETUP *ptrTimerSetup = (TIMER_INTERRUPT_SETUP *)ptrSettings;
            int iInterruptID;
            register int ucGPTimerRef = ptrTimerSetup->timer_reference;
            GPTIMER_MODULE *ptrGPT_Timer;
            switch (ucGPTimerRef) {                                      // GPT to be used
            case iMX_GPT_1:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
                    POWER_DOWN_ATOMIC(1, GPT1_SERIAL_CLOCK);
                    POWER_DOWN_ATOMIC(1, GPT1_BUS_CLOCK);                // disable clocks to module
                    return;
                }
                POWER_UP_ATOMIC(1, GPT1_SERIAL_CLOCK);                   // ensure that the GPT module is powered up
                POWER_UP_ATOMIC(1, GPT1_BUS_CLOCK);
                ptrGPT_Timer = (GPTIMER_MODULE *)GPT1_BLOCK;
                iInterruptID = irq_GPT1_ID;
                break;
            case iMX_GPT_2:
                if ((ptrTimerSetup->timer_mode & TIMER_STOP) != 0) {
                    POWER_DOWN_ATOMIC(0, GPT2_SERIAL_CLOCK);
                    POWER_DOWN_ATOMIC(0, GPT2_BUS_CLOCK);                // disable clocks to module
                    return;
                }
                POWER_UP_ATOMIC(0, GPT2_SERIAL_CLOCK);                   // ensure that the GPT module is powered up
                POWER_UP_ATOMIC(0, GPT2_BUS_CLOCK);
                ptrGPT_Timer = (GPTIMER_MODULE *)GPT2_BLOCK;
                iInterruptID = irq_GPT2_ID;
                break;
            default:
                _EXCEPTION("GPT is unavailable!!");
                return;
            }
            if ((ptrTimerSetup->timer_mode & (TIMER_FREEZE | TIMER_CONTINUE)) != 0) { // if the timer is being frozen or unfrozen
                if ((ptrTimerSetup->timer_mode & TIMER_FREEZE) != 0) {
                    usClockSource[ucGPTimerRef] = (unsigned short)(ptrGPT_Timer->GPT_CR & GPT_CLKSRC_MASK); // back-up the original clock source
                    ptrGPT_Timer->GPT_CR &= ~(GPT_CLKSRC_MASK);          // disable the clock so that the timer freezes
                }
                else {
                    ptrGPT_Timer->GPT_CR |= usClockSource[ucGPTimerRef]; // restore the clock so that the timer continues to run again
                }
                return;                                                  // work complete
            }
            ulGPTimerMode[ucGPTimerRef] = ptrTimerSetup->timer_mode;     // save the mode for later interrupt use
            ptrGPT_Timer->GPT_CR = 0;                                    // initially disable GPT
            ptrGPT_Timer->GPT_IR = 0;                                    // no interrupt enabled
            if ((ptrTimerSetup->timer_mode & TIMER_USES_PRESCALER) != 0) {
                if ((ptrTimerSetup->timer_mode & TIMER_24M_REF_CLK) != 0) {
            #if defined _WINDOWS
                    if ((ptrTimerSetup->prescaler < 1) || (ptrTimerSetup->prescaler > 16)) {
                        _EXCEPTION("Invalid prescaler ! (1 to 16 allowed)");
                    }
            #endif
                    ptrGPT_Timer->GPT_PR = ((ptrTimerSetup->prescaler - 1) << 12); // set the 24MHz prescaler
                }
                else {
            #if defined _WINDOWS
                    if ((ptrTimerSetup->prescaler < 1) || (ptrTimerSetup->prescaler > 4096)) {
                        _EXCEPTION("Invalid prescaler ! (1 to 4096 allowed)");
                    }
            #endif
                    ptrGPT_Timer->GPT_PR = (ptrTimerSetup->prescaler - 1); // set the main prescaler
                }
            }
            else {
                ptrGPT_Timer->GPT_PR = 0;                                // no prescaler
            }
            // Program the required clock source
            //
            if ((ptrTimerSetup->timer_mode & TIMER_HS_REF_CLK) != 0) {
                ptrGPT_Timer->GPT_CR = (GPT_CLKSRC_HF_REF);              // clock source is the high speed reference clock (ipg_clk_highfreq)
            }
            else if ((ptrTimerSetup->timer_mode & TIMER_LS_REF_CLK) != 0) {
                ptrGPT_Timer->GPT_CR = (GPT_CLKSRC_LF_REF);              // clock source is the low speed reference clock (ipg_clk_32k)
            }
            else if ((ptrTimerSetup->timer_mode & TIMER_24M_REF_CLK) != 0) {
                ptrGPT_Timer->GPT_CR = (GPT_CLKSRC_24M_REF | GPT_EN_24M);// clock source is the 24MHz oscillator/crystal source (ipg_clk_24M)
            }
            else if ((ptrTimerSetup->timer_mode & TIMER_EXT_CLK_0) != 0) {
                fnConfigGPT_Clk(ucGPTimerRef, (PORT_PS_UP_ENABLE | PORT_WITH_HYSTERESIS));
                ptrGPT_Timer->GPT_CR = (GPT_CLKSRC_EXT_CLK);             // clock source is the GPT external clock input pin
            }
            else {
                ptrGPT_Timer->GPT_CR = (GPT_CLKSRC_PERCLK);              // clock source is peripheral clock (ipg_clk)
            }
            _WAIT_REGISTER_FALSE(ptrGPT_Timer->GPT_CR, GPT_CLKSRC_MASK); // wait until the clock has been enabled
            WRITE_ONE_TO_CLEAR(ptrGPT_Timer->GPT_SR, (GPT_SR_OF1 | GPT_SR_OF2 | GPT_SR_OF3 | GPT_SR_IF1 | GPT_SR_IF2 | GPT_SR_ROV)); // clear all status bits
            ptrGPT_Timer->GPT_CR |= (GPT_CR_ENMOD | (GPT_PROJECT_MODE)); // counter is reset to zero when disabled
            #if defined SUPPORT_CAPTURE
            if ((ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) != 0) { // if capture mode is required
                unsigned long ulCharacteristics = (PORT_PS_UP_ENABLE | PORT_WITH_HYSTERESIS);
                unsigned long ulEdge;
                if ((ptrTimerSetup->capture_channel < 1) || (ptrTimerSetup->capture_channel > 2)) {
                    _EXCEPTION("Invalid capture channel!! (1 or 2 allowed)");
                    return;
                }
        #if defined _WINDOWS_                                            // check valid pre-scaler values
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
              //ulDelay = (0xffff * ptrTimerSetup->capture_prescaler);   // set maximum count value so that the timer free-runs
                switch (ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) {
                case TIMER_CAPTURE_RISING_FALLING:
                    ulEdge = GPT_IM1_BOTH;                               // capture on rising and falling edges
                    break;
                case TIMER_CAPTURE_RISING:
                    ulCharacteristics = (PORT_PS_DOWN_ENABLE | PORT_WITH_HYSTERESIS);
                    ulEdge = GPT_IM1_RISING;                             // capture on rising edge
                    break;
              //case TIMER_CAPTURE_FALLING:
                default:
                    ulEdge = GPT_IM1_FALLING;                            // capture on falling edge
                    break;
                }
                fnConfigGPT_Pin(ucGPTimerRef, (ptrTimerSetup->capture_channel - 1), ulCharacteristics); // configure the channel's input capture pin
                ptrGPT_Timer->GPT_CR |= (ulEdge << ((ptrTimerSetup->capture_channel - 1) * 2)); // program the edge sensitivity of the capture input
            }
            else {
    #endif
                ptrGPT_Timer->GPT_OCR[ucGPTimerRef] = ptrTimerSetup->timer_value; // main output channel match value
                if ((ptrTimerSetup->timer_mode & GPT_OM1_MASK) != 0) {
                    switch ((ptrTimerSetup->timer_mode & GPT_OM1_MASK)) {
                    case GPT_OM1_TOGGLE:
                    case GPT_OM1_SET:
                        ptrGPT_Timer->GPT_CR |= GPT_OM1_CLEAR;           // set initial state to '0'
                        break;
                    case GPT_OM1_CLEAR:
                    case GPT_OM1_PULSE:
                        ptrGPT_Timer->GPT_CR |= GPT_OM1_SET;             // set initial state to '1'
                        break;
                    }
                    ptrGPT_Timer->GPT_CR |= GPT_FO1;                     // force the compare output initial state
    #if defined _WINDOWS
                    ptrGPT_Timer->GPT_CR &= ~GPT_FO1;                    // thsi bit reads back as '0'
    #endif
                    ptrGPT_Timer->GPT_CR &= ~(GPT_OM1_MASK);
                    fnConfigGPT_Pin(ucGPTimerRef, 0, (PORT_SRE_FAST | PORT_DSE_MID)); // configure the GPTx_COMPARE1 output pin
                }
    #if defined SUPPORT_CAPTURE
            }
    #endif
            if ((_GP_TimerHandler[ucGPTimerRef] = ptrTimerSetup->int_handler) != 0) { // enter the user interrupt handler
                fnEnterInterrupt(iInterruptID, ptrTimerSetup->int_priority, _GPTimerInterrupt[ucGPTimerRef]); // enter GPT interrupt handler
    #if defined SUPPORT_CAPTURE
                if ((ptrTimerSetup->timer_mode & TIMER_CAPTURE_RISING_FALLING) != 0) {
                    ptrGPT_Timer->GPT_IR = (GPT_IR_IF1IE << (ptrTimerSetup->capture_channel - 1)); // enable channel interrupt on input capture
                }
                else {
    #endif
                    ptrGPT_Timer->GPT_IR = GPT_IR_OF1IE;                 // enable channel 1 event interrupt
    #if defined SUPPORT_CAPTURE
                }
    #endif
            }
#if 0
            if ((ptrTimerSetup->timer_mode & TIMER_DMA_TRIGGER) != 0) {  // when DMA required
                ptrFlexTimer->FTM_channel[0].FTM_CV = 0;                 // set the match value for channel 0
                ptrFlexTimer->FTM_channel[0].FTM_CSC = FTM_CSC_DMA;      // enable DMA trigger on match
            }
#endif
            ptrGPT_Timer->GPT_CR |= (GPT_CR_EN | (ptrTimerSetup->timer_mode & GPT_OM1_MASK)); // enable GPT operation with optional GPTx_COMPARE1 output mode
        }
#endif

