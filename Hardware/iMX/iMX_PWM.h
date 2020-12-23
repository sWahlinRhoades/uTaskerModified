/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_PWM.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/


#if defined _PWM_CODE
/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static __interrupt void _PWM_1_Channel_Interrupt_0(void);
static __interrupt void _PWM_1_Channel_Interrupt_1(void);
static __interrupt void _PWM_1_Channel_Interrupt_2(void);
static __interrupt void _PWM_1_Channel_Interrupt_3(void);
    #if FLEXPWM_AVAILABLE > 1
static __interrupt void _PWM_2_Channel_Interrupt_0(void);
static __interrupt void _PWM_2_Channel_Interrupt_1(void);
static __interrupt void _PWM_2_Channel_Interrupt_2(void);
static __interrupt void _PWM_2_Channel_Interrupt_3(void);
    #endif
    #if FLEXPWM_AVAILABLE > 2
static __interrupt void _PWM_3_Channel_Interrupt_0(void);
static __interrupt void _PWM_3_Channel_Interrupt_1(void);
static __interrupt void _PWM_3_Channel_Interrupt_2(void);
static __interrupt void _PWM_3_Channel_Interrupt_3(void);
    #endif
    #if FLEXPWM_AVAILABLE > 3
static __interrupt void _PWM_4_Channel_Interrupt_0(void);
static __interrupt void _PWM_4_Channel_Interrupt_1(void);
static __interrupt void _PWM_4_Channel_Interrupt_2(void);
static __interrupt void _PWM_4_Channel_Interrupt_3(void);
    #endif

/* =================================================================== */
/*                              local consts                           */
/* =================================================================== */

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static void(*_PWM_ChannelHandler[FLEXPWM_AVAILABLE][FLEXPWM_CHANNELS])(void) = {{0}}; // user channel periodic interrupt handlers
static unsigned char ucEnabledChannelInterrupts[FLEXPWM_AVAILABLE] = {0};// enabled channel flags for each timer

static void (*_PWM_ChannelInterrupt[FLEXPWM_AVAILABLE][FLEXPWM_CHANNELS])(void) = {
    {
        _PWM_1_Channel_Interrupt_0,
        _PWM_1_Channel_Interrupt_1,
        _PWM_1_Channel_Interrupt_2,
        _PWM_1_Channel_Interrupt_3
    },
    #if FLEXPWM_AVAILABLE > 1
    {
        _PWM_2_Channel_Interrupt_0,
        _PWM_2_Channel_Interrupt_1,
        _PWM_2_Channel_Interrupt_2,
        _PWM_2_Channel_Interrupt_3
    },
    #endif
    #if FLEXPWM_AVAILABLE > 2
    {
        _PWM_3_Channel_Interrupt_0,
        _PWM_3_Channel_Interrupt_1,
        _PWM_3_Channel_Interrupt_2,
        _PWM_3_Channel_Interrupt_3
    },
    #endif
    #if FLEXPWM_AVAILABLE > 3
    {
        _PWM_4_Channel_Interrupt_0,
        _PWM_4_Channel_Interrupt_1,
        _PWM_4_Channel_Interrupt_2,
        _PWM_4_Channel_Interrupt_3
    }
    #endif
};

/* =================================================================== */
/*                   PWM cycle Interrupt Handlers                      */
/* =================================================================== */

#if defined _WINDOWS_                                                     // reset channel event flags when simulating
static void fnResetChannelEventFlags(int iTimerRef, FLEX_TIMER_MODULE *ptrTimer)
{
    int iChannel = 0;
    while (iChannel < (ucSumChannels[iTimerRef + 1] - ucSumChannels[iTimerRef])) { // for each channel
        ptrTimer->FTM_channel[iChannel++].FTM_CSC &= ~FTM_CSC_CHF;       // reset the channel event flag
    }
}
#endif

// General PWM channel periodic interrupt handler
//
static void fnHandlePWMChannels(int iFlexPWM, FLEXPWM_CHANNEL *ptrFlexPWMchannel, int iChannel)
{
    if ((ptrFlexPWMchannel->FLEXPWM_SMSTS & FLEXPWM_SMSTS_RF) != 0) {    // if the reload flag is set
        WRITE_ONE_TO_CLEAR(ptrFlexPWMchannel->FLEXPWM_SMSTS, FLEXPWM_SMSTS_RF); // clear the interrupt flag
        if (_PWM_ChannelHandler[iFlexPWM][iChannel] != 0) {              // if there is a user handler installed
            uDisable_Interrupt();
                _PWM_ChannelHandler[iFlexPWM][iChannel]();               // call user interrupt handler
            uEnable_Interrupt();
        }
    }
}

static __interrupt void _PWM_1_Channel_Interrupt_0(void)
{
    fnHandlePWMChannels(0, &((FLEXPWM_MODULE *)FLEXPWM1_BLOCK)->flexPWM_ch[0], 0);
}

static __interrupt void _PWM_1_Channel_Interrupt_1(void)
{
    fnHandlePWMChannels(0, &((FLEXPWM_MODULE *)FLEXPWM1_BLOCK)->flexPWM_ch[1], 1);
}

static __interrupt void _PWM_1_Channel_Interrupt_2(void)
{
    fnHandlePWMChannels(0, &((FLEXPWM_MODULE *)FLEXPWM1_BLOCK)->flexPWM_ch[2], 2);
}

static __interrupt void _PWM_1_Channel_Interrupt_3(void)
{
    fnHandlePWMChannels(0, &((FLEXPWM_MODULE *)FLEXPWM1_BLOCK)->flexPWM_ch[3], 3);
}

    #if FLEXPWM_AVAILABLE > 1
static __interrupt void _PWM_2_Channel_Interrupt_0(void)
{
    fnHandlePWMChannels(1, &((FLEXPWM_MODULE *)FLEXPWM2_BLOCK)->flexPWM_ch[0], 0);
}

static __interrupt void _PWM_2_Channel_Interrupt_1(void)
{
    fnHandlePWMChannels(1, &((FLEXPWM_MODULE *)FLEXPWM2_BLOCK)->flexPWM_ch[1], 1);
}

static __interrupt void _PWM_2_Channel_Interrupt_2(void)
{
    fnHandlePWMChannels(1, &((FLEXPWM_MODULE *)FLEXPWM2_BLOCK)->flexPWM_ch[2], 2);
}

static __interrupt void _PWM_2_Channel_Interrupt_3(void)
{
    fnHandlePWMChannels(1, &((FLEXPWM_MODULE *)FLEXPWM2_BLOCK)->flexPWM_ch[3], 3);
}
    #endif

    #if FLEXPWM_AVAILABLE > 2
static __interrupt void _PWM_3_Channel_Interrupt_0(void)
{
    fnHandlePWMChannels(2, &((FLEXPWM_MODULE *)FLEXPWM3_BLOCK)->flexPWM_ch[0], 0);
}

static __interrupt void _PWM_3_Channel_Interrupt_1(void)
{
    fnHandlePWMChannels(2, &((FLEXPWM_MODULE *)FLEXPWM3_BLOCK)->flexPWM_ch[1], 1);
}

static __interrupt void _PWM_3_Channel_Interrupt_2(void)
{
    fnHandlePWMChannels(2, &((FLEXPWM_MODULE *)FLEXPWM3_BLOCK)->flexPWM_ch[2], 2);
}

static __interrupt void _PWM_3_Channel_Interrupt_3(void)
{
    fnHandlePWMChannels(2, &((FLEXPWM_MODULE *)FLEXPWM3_BLOCK)->flexPWM_ch[3], 3);
}
    #endif

    #if FLEXPWM_AVAILABLE > 3
static __interrupt void _PWM_4_Channel_Interrupt_0(void)
{
    fnHandlePWMChannels(3, &((FLEXPWM_MODULE *)FLEXPWM4_BLOCK)->flexPWM_ch[0], 0);
}

static __interrupt void _PWM_4_Channel_Interrupt_1(void)
{
    fnHandlePWMChannels(3, &((FLEXPWM_MODULE *)FLEXPWM4_BLOCK)->flexPWM_ch[1], 1);
}

static __interrupt void _PWM_4_Channel_Interrupt_2(void)
{
    fnHandlePWMChannels(3, &((FLEXPWM_MODULE *)FLEXPWM4_BLOCK)->flexPWM_ch[2], 2);
}

static __interrupt void _PWM_4_Channel_Interrupt_3(void)
{
    fnHandlePWMChannels(3, &((FLEXPWM_MODULE *)FLEXPWM4_BLOCK)->flexPWM_ch[3], 3);
}
    #endif
#endif


#if defined _PWM_CONFIG_CODE
        {
            PWM_INTERRUPT_SETUP *ptrPWM_settings = (PWM_INTERRUPT_SETUP *)ptrSettings;
            int iInterruptID;
            unsigned long ulMode = ptrPWM_settings->pwm_mode;
            unsigned char ucChannel = (ptrPWM_settings->pwm_reference & 0x03);
            unsigned char ucFlexPWM_Ref = ((ptrPWM_settings->pwm_reference & _TIMER_MODULE_MASK) >> _TIMER_MODULE_SHIFT);
            FLEXPWM_MODULE *ptrFlexPWM;
            POWER_UP_ATOMIC(2, XBAR1_CLOCK);                             // ensure that XBAR1 is powered (which connects the FlexPWM fault inputs)
            switch (ucFlexPWM_Ref) {
            case 0:
                POWER_UP_ATOMIC(4, PWM1_CLOCKS);                         // ensure that the FlexPWM module is powered up
                ptrFlexPWM = (FLEXPWM_MODULE *)FLEXPWM1_BLOCK;
                iInterruptID = irq_FLEXPWM1_0_ID;
                XBAR1_OUT_FLEXPWM1_FAULT0(XBAR1_LOGIC_HIGH);             // drive fault inputs with logic '1' to disable them
                XBAR1_OUT_FLEXPWM1_FAULT1(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM1_FAULT2(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM1_FAULT3(XBAR1_LOGIC_HIGH);
                break;
    #if FLEXPWM_AVAILABLE > 1
            case 1:
                POWER_UP_ATOMIC(4, PWM2_CLOCKS);                         // ensure that the FlexPWM module is powered up
                ptrFlexPWM = (FLEXPWM_MODULE *)FLEXPWM2_BLOCK;
                iInterruptID = irq_FLEXPWM2_0_ID;
                XBAR1_OUT_FLEXPWM2_FAULT0(XBAR1_LOGIC_HIGH);             // drive fault inputs with logic '1' to disable them
                XBAR1_OUT_FLEXPWM2_FAULT1(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM2_FAULT2(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM2_FAULT3(XBAR1_LOGIC_HIGH);
                break;
    #endif
    #if FLEXPWM_AVAILABLE > 2
            case 2:
                POWER_UP_ATOMIC(4, PWM3_CLOCKS);                         // ensure that the FlexPWM module is powered up
                ptrFlexPWM = (FLEXPWM_MODULE *)FLEXPWM3_BLOCK;
                iInterruptID = irq_FLEXPWM3_0_ID;
                XBAR1_OUT_FLEXPWM3_FAULT0(XBAR1_LOGIC_HIGH);             // drive fault inputs with logic '1' to disable them
                XBAR1_OUT_FLEXPWM3_FAULT1(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM3_FAULT2(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM3_FAULT3(XBAR1_LOGIC_HIGH);
                break;
    #endif
    #if FLEXPWM_AVAILABLE > 3
            case 3:
                POWER_UP_ATOMIC(4, PWM4_CLOCKS);                         // ensure that the FlexPWM module is powered up
                ptrFlexPWM = (FLEXPWM_MODULE *)FLEXPWM4_BLOCK;
                iInterruptID = irq_FLEXPWM4_0_ID;
                XBAR1_OUT_FLEXPWM4_FAULT0(XBAR1_LOGIC_HIGH);             // drive fault inputs with logic '1' to disable them
                XBAR1_OUT_FLEXPWM4_FAULT1(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM4_FAULT2(XBAR1_LOGIC_HIGH);
                XBAR1_OUT_FLEXPWM4_FAULT3(XBAR1_LOGIC_HIGH);
                break;
    #endif
            default:
    #if defined _WINDOWS
                _EXCEPTION("Invalid FlexPWM !!");
    #endif
                return;                                                  // invalid FlexPWM
            }
            WRITE_ONE_TO_CLEAR(ptrFlexPWM->FLEXPWM_FSTS0, (FLEXPWM_FSTS0_FFLAG_0 | FLEXPWM_FSTS0_FFLAG_1 | FLEXPWM_FSTS0_FFLAG_2 | FLEXPWM_FSTS0_FFLAG_3)); // ensure fault flags are cleared
            ptrFlexPWM->FLEXPWM_MCTRL |= (FLEXPWM_MCTRL_CLDOK_0 << ucChannel); // clear load OK flag
    #if defined _WINDOWS
            ptrFlexPWM->FLEXPWM_MCTRL &= ~((FLEXPWM_MCTRL_CLDOK_0 | FLEXPWM_MCTRL_LDOK_0) << ucChannel);
    #endif
            ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMCTRL = ((unsigned short)(ulMode & FLEXPWM_SMCTRL_PRSC_128) | FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_LDFQ_1); // set prescaler and reload new register values at the end of a cycle
            ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMCTRL2 = (FLEXPWM_SMCTRL2_CLK_SEL_IPBus | FLEXPWM_SMCTRL2_INDEP); // set clock source
            if ((ulMode & PWM_NO_OUTPUT) == 0) {                         // if the output is not disabled
                fnConfigPWM_Pin(ucFlexPWM_Ref, ucChannel, ptrPWM_settings->pwm_reference, (PORT_SRE_FAST | PORT_DSE_HIGH)); // configure the corresponding PWM output pin(s)
            }
            if ((ulMode & PWM_POLARITY) != 0) {                          // if inverse polarity
                ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMOCTRL = (FLEXPWM_SMOCTRL_POLA | FLEXPWM_SMOCTRL_POLB | FLEXPWM_SMOCTRL_POLX);
            }
            else {
                ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMOCTRL = 0;   // normal polarity
            }
            ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMCTRL2 |= (FLEXPWM_SMCTRL2_FORCE | FLEXPWM_SMCTRL2_DBGEN | FLEXPWM_SMCTRL2_WAITEN); // force values to be loaded and continue running in wait and debug modes
    #if 0 //!defined DEVICE_WITHOUT_DMA_
            if ((ulMode & PWM_DMA_CHANNEL_ENABLE) != 0) {                // DMA trigger to be used
              //ptrFlexTimer->FTM_channel[ucChannel].FTM_CSC |= (FTM_CSC_DMA | FTM_CSC_CHIE); // enable DMA trigger from this channel (also the interrupt needs to be enabled for the DMA to operate - interrupt is however not generated in this configuration)
            }
    #endif
            ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMINIT = 0;
            ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMDTCNT0 = ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMDTCNT1 = 0;
            ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[0].FLEXPWM_SMVAL = (ptrPWM_settings->pwm_frequency/2); // center value
            ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[1].FLEXPWM_SMVAL = ptrPWM_settings->pwm_frequency; // period
            if ((ulMode & PWM_EDGE_ALIGNED) != 0) {                      // if center-aligned
                ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[2].FLEXPWM_SMVAL = ((ptrPWM_settings->pwm_frequency - ptrPWM_settings->pwm_value)/2); // set the duty cycle for the particular channel on A
                ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[3].FLEXPWM_SMVAL = ((ptrPWM_settings->pwm_frequency + ptrPWM_settings->pwm_value)/2);
                ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[4].FLEXPWM_SMVAL = ((ptrPWM_settings->pwm_frequency - ptrPWM_settings->pwm_value)/2); // set the duty cycle for the particular channel on B
                ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[5].FLEXPWM_SMVAL = ((ptrPWM_settings->pwm_frequency + ptrPWM_settings->pwm_value)/2);
            }
            else {                                                       // edge aligned
                if ((ptrPWM_settings->pwm_reference & _FLEX_PWM_B_OUTPUT) != 0) { // B output
                    ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[4].FLEXPWM_SMVAL = 0;
                    ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[5].FLEXPWM_SMVAL = ptrPWM_settings->pwm_value; // set the duty cycle for the particular channel on B
                    ptrFlexPWM->FLEXPWM_OUTEN |= (FLEXPWM_OUTEN_PWMB_EN_0 << ucChannel);
                }
                else {                                                   // A output
                    ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[2].FLEXPWM_SMVAL = 0;
                    ptrFlexPWM->flexPWM_ch[ucChannel].flexPWM_val[3].FLEXPWM_SMVAL = ptrPWM_settings->pwm_value; // set the duty cycle for the particular channel on A
                    ptrFlexPWM->FLEXPWM_OUTEN |= (FLEXPWM_OUTEN_PWMA_EN_0 << ucChannel);
                }
            }
    #if  0//!defined DEVICE_WITHOUT_DMA
            if ((ulMode & (PWM_FULL_BUFFER_DMA | PWM_HALF_BUFFER_DMA)) != 0) { // if DMA is being specified
                unsigned long ulDMA_rules = (DMA_DIRECTION_OUTPUT | DMA_HALF_WORDS); // default DMA rules
                void *ptrRegister;
                if ((ulMode & PWM_FULL_BUFFER_DMA_AUTO_REPEAT) != 0) {
                    ulDMA_rules |= DMA_AUTOREPEAT;
                }
                if ((ulMode & PWM_HALF_BUFFER_DMA) != 0) {
                    ulDMA_rules |= DMA_HALF_BUFFER_INTERRUPT;
                }
                if ((ulMode & PWM_DMA_SPECIFY_DESTINATION) == 0) {
                    if ((ulMode & PWM_DMA_CONTROL_FREQUENCY) != 0) {
                      //ptrRegister = (void *)&ptrFlexTimer->FTM_MOD;    // each DMA trigger causes a new frequency to be set
                    }
                    else {
                      //ptrRegister = (void *)&ptrFlexTimer->FTM_channel[ucChannel].FTM_CV; // each DMA trigger causes a new PWM value to be set
                    }
                }
                else {
                    ptrRegister = ptrPWM_settings->ptrRegister;          // else use the destination register as specified by the user
                    if (((PWM_DMA_SPECIFY_LONG_WORD | PWM_DMA_SPECIFY_BYTE) & ulMode) != 0) { // and use the specified destination width
                        ulDMA_rules &= ~(DMA_HALF_WORDS);
                        if ((PWM_DMA_SPECIFY_LONG_WORD & ulMode) != 0) {
                            ulDMA_rules |= (DMA_LONG_WORDS);
                        }
                        else {
                            ulDMA_rules |= (DMA_BYTES);
                        }
                    }
                }
                fnConfigDMA_buffer(ptrPWM_settings->ucDmaChannel, ptrPWM_settings->usDmaTriggerSource, ptrPWM_settings->ulPWM_buffer_length, ptrPWM_settings->ptrPWM_Buffer, ptrRegister, ulDMA_rules, ptrPWM_settings->dma_int_handler, ptrPWM_settings->dma_int_priority); // source is the PWM buffer and destination is the PWM mark-space ratio register
                fnDMA_BufferReset(ptrPWM_settings->ucDmaChannel, DMA_BUFFER_START);
            }
    #endif
            if (ptrPWM_settings->int_handler != 0) {                     // if an interrupt handler is specified it is called at each period
                _PWM_ChannelHandler[ucFlexPWM_Ref][ucChannel] = ptrPWM_settings->int_handler; // user's period interrupt handler
                fnEnterInterrupt(iInterruptID, ptrPWM_settings->int_priority, _PWM_ChannelInterrupt[ucFlexPWM_Ref][ucChannel]); // enter the interrupt handler
                ptrFlexPWM->flexPWM_ch[ucChannel].FLEXPWM_SMINTEN = (FLEXPWM_SMINTEN_REI); // enable reload interrupt for this FlexPWM channel
            }
            else {
                ulMode = 0;
            }
            /*
            if (((ptrPWM_settings->pwm_mode & PWM_CHANNEL_INTERRUPT) != 0) && (ptrPWM_settings->channel_int_handler != 0)) { // if a channel match interrupt is to be generated
                _PWM_ChannelHandler[ucChannel] = ptrPWM_settings->channel_int_handler; // channel interrupt handler
                fnEnterInterrupt((iInterruptID + ucChannel), ptrPWM_settings->int_priority, _PWM_TimerInterrupt[ucFlexPWM_Ref]);
                ucEnabledChannelInterrupts[ucFlexPWM_Ref] |= (1 << ucChannel); //  mark that this flextimer channel is enabled for interrupts
                ulMode = (FLEXPWM_SMINTEN_CMPIE_0 << ucChannel);         // enable channel interrupt
            }
            else {
                ucEnabledChannelInterrupts[ucFlexPWM_Ref] &= ~(1 << ucChannel); // channel match interrupt is not enabled
            }
            */
            ptrFlexPWM->FLEXPWM_MCTRL |= (FLEXPWM_MCTRL_LDOK_0 << ucChannel); // set the load Ok flag
            ptrFlexPWM->FLEXPWM_MCTRL |= (FLEXPWM_MCTRL_RUN_0 << ucChannel); // set the run flag
        }
#endif
