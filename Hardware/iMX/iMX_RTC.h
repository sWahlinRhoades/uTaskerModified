/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_RTC.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/


static int rtc_interrupts = 0;
static unsigned long rtc_alarm = 0;
static unsigned long ulRTC_stopwatch = 0;

static void (*rtc_interrupt_handler[6])(void) = {0};                     // RTC interrupt handlers

// This interrupt arrives once a second as the RTC's seconds counter increments
//
static __interrupt void _rtc_handler(void)
{
    #if defined SUPPORT_SW_RTC
    unsigned long ulNewSeconds = *RTC_SECONDS_LOCATION;
    ulNewSeconds++;
    *RTC_SECONDS_LOCATION = ulNewSeconds;                                // increment the seconds count in non-initialised ram
        #if defined KINETIS_KE
    RTC_SC |= RTC_SC_RTIF;                                               // clear pending interrupt
        #endif
    if ((rtc_interrupts & RTC_ALARM_INT) != 0) {
        if (ulNewSeconds == rtc_alarm) {                                 // alarm date / time match
            rtc_interrupts |= RTC_ALARM_INT_MATCH;
        }
    }
    #else
    if (((SNVS_HPCR & SNVS_HPCR_HPTA_EN)) && ((SNVS_HPSR & SNVS_HPSR_HPTA) != 0)) { // alarm interrupt enabled and fired
        WRITE_ONE_TO_CLEAR(SNVS_HPSR, SNVS_HPSR_HPTA);                   // clear the alarm interrupt flag
        if (rtc_interrupt_handler[2] != 0) {
            uDisable_Interrupt();
                rtc_interrupt_handler[2]();                              // call handling function (alarm match interrupt)
            uEnable_Interrupt();
        }
        if ((SNVS_HPSR & SNVS_HPSR_PI) == 0) {                           // if not seconds interrupt
            return;                                                      // return since complete
        }
    }
    WRITE_ONE_TO_CLEAR(SNVS_HPSR, SNVS_HPSR_PI);                         // clear the second interrupt flag
    #endif
    if ((ulRTC_stopwatch != 0) && (--ulRTC_stopwatch == 0)) {            // if a stop watch countdown is active and it reaches zero
        rtc_interrupts |= RTC_STOPWATCH_MATCH;                           // mark that the stop watch handler should be executed
    }
    fnSecondsTick(rtc_interrupt_handler, rtc_interrupts);                // generic handling
    rtc_interrupts &= ~(RTC_ALARM_INT_MATCH | RTC_STOPWATCH_MATCH);      // ensure that alarm and stopwatch interrupts are single-shot interrupts
}


// This routine attempts a high level of compatibility with the Coldfire V2 RTC
//
extern int fnConfigureRTC(void *ptrSettings)
{
    int iIRQ = 0;
    RTC_SETUP *ptr_rtc_setup = (RTC_SETUP *)ptrSettings;
    switch (ptr_rtc_setup->command & ~(RTC_DISABLE | RTC_INITIALISATION | RTC_SET_UTC | RTC_INCREMENT)) {
    case RTC_TIME_SETTING:                                               // set time to RTC
    {
        unsigned long ulSeconds = ptr_rtc_setup->ulLocalUTC;
    #if !defined SUPPORT_SW_RTC
        POWER_UP_ATOMIC(5, SNVS_HP_CLOCK);                               // ensure the high power secure non-volatile storage module is powered
        SNVS_HPCOMR |= SNVS_HPCOMR_NPSWA_EN;                             // allow any software to access privileged registers
        SNVS_HPCR &= ~SNVS_HPCR_RTC_EN;                                  // temporarily disable RTC to avoid potential interrupt (this is synchronised to the 32kHz clock)
    #endif
        if ((ptr_rtc_setup->command & RTC_SET_UTC) != 0) {               // allow setting from UTC seconds value
            fnConvertSecondsTime(0, ulSeconds);                          // convert to time for internal use
        }
        else {
            ulSeconds = fnConvertTimeSeconds(ptr_rtc_setup, 1);          // set the present time and date as seconds since 1970 (Unix epoch)
        }
    #if defined SUPPORT_SW_RTC
        *RTC_SECONDS_LOCATION = ulSeconds;                               // directly set the seconds value
    #else
        while ((SNVS_HPCR & SNVS_HPCR_RTC_EN) != 0) {}                   // wait until disabled before setting new time
        SNVS_HPRTCMR = (ulSeconds >> 17);                                // directly set the seconds value
        SNVS_HPRTCLR = (ulSeconds << 15);
    #endif
    #if !defined SUPPORT_SW_RTC
        SNVS_HPCR |= SNVS_HPCR_RTC_EN;                                   // enable counter again
    #endif
        break;
    }
    #if !defined SUPPORT_SW_RTC
    case RTC_GET_ALARM:
    {
        unsigned long ulAlarmSeconds = (SNVS_HPTALR >> 16);
        ulAlarmSeconds |= (SNVS_HPTAMR << 16);
        fnConvertSecondsTime(ptr_rtc_setup, ulAlarmSeconds);
        break;
    }
    #else
    case RTC_GET_ALARM:
        fnConvertSecondsTime(ptr_rtc_setup, rtc_alarm);
        break;
    #endif
    case RTC_GET_TIME:                                                   // get the present time
        fnGetRTC(ptr_rtc_setup);
        break;
    case RTC_CONVERT_TO_UTC:
        ptr_rtc_setup->ulLocalUTC = fnConvertTimeSeconds(ptr_rtc_setup, 0); // take time/date as input and return UTC value
        break;
    case RTC_CONVERT_FROM_UTC:
        fnConvertSecondsTime(ptr_rtc_setup, ptr_rtc_setup->ulLocalUTC);  // take UTC value and convert it to time/date
        break;
    case RTC_TICK_HOUR:                                                  // interrupt on each hour
        iIRQ++;
    case RTC_TICK_SEC:                                                   // interrupt on each second
    #if defined SUPPORT_SW_RTC
        if ((RTC_INCREMENT & ptr_rtc_setup->command) != 0) {             // second increment is to be performed as if it were a RTC interrupt
            _rtc_handler();
            break;
        }
    #endif
        iIRQ++;
    case RTC_TICK_DAY:                                                   // interrupt on each day
        iIRQ++;
    case RTC_ALARM_TIME:                                                 // interrupt at specific date and time
        if (iIRQ == 0) {                                                 // RTC_ALARM_TIME
            rtc_interrupts &= ~RTC_ALARM_INT;                            // disable ALARM interrupt
            if ((RTC_SET_UTC & ptr_rtc_setup->command) != 0) {
                rtc_alarm = ptr_rtc_setup->ulLocalUTC;                   // set directly the alarm match from UTC value
            }
            else {
                rtc_alarm = fnConvertTimeSeconds(ptr_rtc_setup, 0);      // set alarm match in seconds
            }
        }
        iIRQ++;
    case RTC_TICK_MIN:                                                   // interrupt each minute
        iIRQ++;
    case RTC_STOPWATCH_GO:                                               // interrupt after a certain number of minutes
        if (iIRQ == 0) {                                                 // RTC_STOPWATCH_GO            
            ulRTC_stopwatch = ((ptr_rtc_setup->ucHours * 60 * 60) + (ptr_rtc_setup->ucMinutes * 60) + ptr_rtc_setup->ucSeconds); // set a stop watch interrupt after this many additional minutes (1 to 64 minutes for Coldfire compatibility but extended to hours and seconds too)
        }
        if ((RTC_DISABLE & ptr_rtc_setup->command) != 0) {
            rtc_interrupts &= ~(0x01 << iIRQ);                           // disable interrupt
            return 0;                                                    // disable function rather than enable
        }
    #if defined SUPPORT_SW_RTC
        rtc_interrupt_handler[iIRQ] = ((RTC_SETUP *)ptrSettings)->int_handler; // enter the handling interrupt
    #else
        POWER_UP_ATOMIC(5, SNVS_HP_CLOCK);                               // ensure the high power secure non-volatile storage module is powered
        if ((SNVS_LPSR & SNVS_LPSR_SPO) != 0) {                          // if time invalid
            WRITE_ONE_TO_CLEAR(SNVS_LPSR, (SNVS_LPSR_SPO | SNVS_LPSR_EO | SNVS_LPSR_MCR)); // write to clear RTC_SR_TIF in status register when not yet enabled
            _SET_REGISTER(XTALOSC24M_MISC0, CCM_ANALOG_MISC0_RTC_XTAL_SOURCE_RTC_XTAL); // use crystal oscillator source (rather than internal RC 32k surce)
            return WAIT_STABILISING_DELAY;                               // the oscillator requires some time to stabilise so the user should call again after this time has expired
        }
        SNVS_HPCR &= ~SNVS_HPCR_RTC_EN;                                  // temporarily disable RTC to avoid potentially missed seconds count (this is synchronised to the 32kHz clock)
        rtc_interrupt_handler[iIRQ] = ((RTC_SETUP *)ptrSettings)->int_handler; // enter the handling interrupt
    #endif
        rtc_interrupts |= (0x01 << iIRQ);                                // flag interrupt(s) enabled
    #if !defined SUPPORT_SW_RTC
        while ((SNVS_HPCR & SNVS_HPCR_RTC_EN) != 0) {}                   // wait until disabled before continuing
        fnEnterInterrupt(irq_SNVS_Func_ID, PRIORITY_RTC, _rtc_handler);  // enter the alarm interrupt handler
        if (2 == iIRQ) {                                                 // alarm being set
            unsigned long ulPresentAlarm = (SNVS_HPRTCMR << 17);
            ulPresentAlarm |= (SNVS_HPRTCLR >> 15);
            SNVS_HPCR &= ~(SNVS_HPCR_HPTA_EN);                           // ensure alarm disabled so that a new value will be accepted
            WRITE_ONE_TO_CLEAR(SNVS_HPSR, SNVS_HPSR_HPTA);               // clear potential pending alarm interrupt flag
            if (ulPresentAlarm > rtc_alarm) {                            // avoid setting match value in the past since it won't (ever) fire
                rtc_alarm = ulPresentAlarm;                              // set to next second value
            }
            SNVS_HPTAMR = (rtc_alarm >> 17);
            SNVS_HPTALR = (rtc_alarm << 15);                             // set timer alarm interrupt match
            SNVS_HPCR |= SNVS_HPCR_HPTA_EN;                              // enable alarm interrupt
        }
        else {                                                           // second interrupt
            SNVS_HPCR |= (SNVS_HPCR_PI_EN | SNVS_HPCR_PI_FREQ);          // enable seconds interrupt
        }
    #endif
        if ((ptr_rtc_setup->command & RTC_INITIALISATION) != 0) {
    #if !defined SUPPORT_SW_RTC					
            unsigned long ulPresentSecondsLow;
            unsigned long ulPresentSecondsHigh;
    #endif
    #if defined irq_SNVS_Func_ID
        #if irq_SNVS_Func_ID < 32                                        // clear pending alarm interrupt at initialisation
            IRQ0_31_CPR = (1 << irq_SNVS_Func_ID);
        #elif irq_SNVS_Func_ID < 64
            IRQ32_63_CPR = (1 << (irq_SNVS_Func_ID - 32));
        #else
            IRQ64_95_CPR = (1 << (irq_SNVS_Func_ID - 64));
        #endif
    #endif
    #if defined SUPPORT_SW_RTC
            if (*RTC_VALID_LOCATION != RTC_VALID_PATTERN) {              // power on reset
                *RTC_SECONDS_LOCATION = 0;                               // update the count and pre-scaler value in non-initialised ram
                *RTC_ALARM_LOCATION = 0;
                *RTC_VALID_LOCATION = RTC_VALID_PATTERN;
            }
            else {
                fnConvertSecondsTime(0, *RTC_SECONDS_LOCATION);          // take the present seconds count value, convert and set to time and date
            }
    #else
            do {
                ulPresentSecondsLow = (SNVS_HPRTCLR >> 15);              // read low order part of the counter
                ulPresentSecondsHigh = (SNVS_HPRTCMR << 17);             // read high order part of counter
            } while (ulPresentSecondsLow != (SNVS_HPRTCLR >> 15));       // if the low order part changed between initial read and check repeat to ensure that both remain synchronised

            fnConvertSecondsTime(0, (ulPresentSecondsLow | ulPresentSecondsHigh)); // take the present seconds count value, convert and set to time and date
    #endif
        }
    #if !defined SUPPORT_SW_RTC
        SNVS_HPCR |= SNVS_HPCR_RTC_EN;                                   // enable counter
    #endif
        break;
    }
    return 0;
}

