/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_RTC.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    05.08.2013 Add UTC option when setting and retrieving time/date      {51}
    17.04.2014 Use RTC seconds interrupt rather than alarm interrupt, when available {75}
    24.04.2014 Add KL RTC support based on LPO clock or external 32kHz oscillator {77}
    08.02.2015 Time keeping routines moved to uTasker\time_keeper.c      {114}
    24.06.2015 Always re-synchronise the variable counters to the RTC counter when fnRestoreRTC() is used {1}
    24.06.2015 Synchronise the variable counters to the RTC counter when KL devices are used without LPO {2}
    02.07.2015 Allow setting alarm directly from local UTC value         {3}
    24.02.2016 Correct stop watch interrupt by removing redundant flag   {4}
    13.06.2017 Synchronise prescaler when setting new time               {5}
    18.09.2017 Use new dependency KINETIS_WITH_RTC_CRYSTAL               {6}
    05.11.2017 Clear pending RTC alarm interrupt on initialisation (this tends to be pending after each power on reset or after a wake-up) {7}
    13.03.2018 Enable RTC oscillator with internal clock disabled when not already configured {8}
    01.10.2020 Don't access RTC HW of KE part when SW RTC is used        {9}
    15.10.2020 Use fnClearPending() to clear pending interrupt           {10}

*/



static int rtc_interrupts = 0;
static unsigned long rtc_alarm = 0;
static unsigned long ulRTC_stopwatch = 0;

static void (*rtc_interrupt_handler[6])(void) = {0};                     // RTC interrupt handlers

// This interrupt arrives once a second as the RTC's seconds counter increments
//
static __interrupt void _rtc_handler(void)
{
    #if !defined irq_RTC_SECONDS_ID && !(defined KINETIS_KE && !defined KINETIS_WITH_SRTC) && !defined SUPPORT_SW_RTC // {75}
    register unsigned long ulNewSeconds = RTC_TSR;                       // the present seconds count value
    RTC_TAR = ulNewSeconds;                                              // set timer alarm interrupt to next second which also resets the alarm source
    #endif
    #if defined KINETIS_KL && defined RTC_USES_LPO_1kHz                  // {77}
    RTC_SR = 0;                                                          // temporarily disable RTC to avoid potential interrupt
    RTC_TPR = (32768 - 1000);                                            // use 1kHz internal clock to generate 1s time base by adjusting the prescaler value
    RTC_SR = RTC_SR_TCE;                                                 // enable counter again
    *RTC_SECONDS_LOCATION = RTC_TSR;                                     // update the count and pre-scaler value in non-initialised ram
    *RTC_PRESCALER_LOCATION = (unsigned short)RTC_TPR;
    #endif
    #if defined SUPPORT_SW_RTC || (defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
    unsigned long ulNewSeconds = *RTC_SECONDS_LOCATION;
    ulNewSeconds++;
    *RTC_SECONDS_LOCATION = ulNewSeconds;                                // increment the seconds count in non-initialised ram
        #if !defined SUPPORT_SW_RTC                                      // {9}
    RTC_SC |= RTC_SC_RTIF;                                               // clear pending interrupt
        #endif
    #endif
    #if !defined irq_RTC_SECONDS_ID || defined SUPPORT_SW_RTC
    if ((rtc_interrupts & RTC_ALARM_INT) != 0) {
        if (ulNewSeconds == rtc_alarm) {                                 // alarm date / time match
            rtc_interrupts |= RTC_ALARM_INT_MATCH;
        }
    }
    #endif
    if ((ulRTC_stopwatch != 0) && (--ulRTC_stopwatch == 0)) {            // if a stop watch countdown is active and it reaches zero
      //if ((rtc_interrupts & RTC_STOPWATCH) != 0) {                     // {4} stopwatch interrupt enabled
            rtc_interrupts |= RTC_STOPWATCH_MATCH;                       // mark that the stop watch handler should be executed
      //}
    }
    fnSecondsTick(rtc_interrupt_handler, rtc_interrupts);                // {114} generic handling
    rtc_interrupts &= ~(RTC_ALARM_INT_MATCH | RTC_STOPWATCH_MATCH);      // ensure that alarm and stopwatch interrupts are single-shot interrupts
}

    #if defined irq_RTC_SECONDS_ID && !defined SUPPORT_SW_RTC
static __interrupt void _rtc_alarm_handler(void)
{
    RTC_TAR = 0;                                                         // clear the alarm source
    if (rtc_interrupt_handler[2] != 0) {
        uDisable_Interrupt();
            rtc_interrupt_handler[2]();                                  // call handling function (alarm match interrupt)
        uEnable_Interrupt();
    }
}
    #endif

    #if defined KINETIS_KL && defined RTC_USES_LPO_1kHz
// This function is called with iResync set when the processor has woken from a low leakage power mode in which the RTC seconds interrupt was not serviced
// Since the RTC was being clocked with 1kHz rather than 32.768kHz it has lost time and this is compensated for here
// If there is an alarm that hasn't fired its remainig time is also compensated accordingly, remembering that its value was adjusted when moving to the low leakage power mode
// When called with iResync set to 0 it was due to a a reset (or wakeup via reset) which requires the same adjustment but is being called during RTC initialisation
//
static void fnRestoreRTC(int iResync)
{
    unsigned long ulLostSeconds = 0;
    unsigned long ulMissedCounts;
    if (iResync != 0) {
        if ((IS_POWERED_UP(6, RTC) == 0) || ((RTC_SR & RTC_SR_TCE) == 0)) { // if the RTC is not enabled or not presently running we ignore any further checks
            return;
        }
        RTC_SR = 0;                                                      // suspend RTC counting during synchronisation
    }
    // The RTC is suspended at this point
    //
    ulLostSeconds = (RTC_TSR - *RTC_SECONDS_LOCATION);
    if (ulLostSeconds != 0) {
        ulLostSeconds--;
        ulLostSeconds *= 3277;                                           // 32.77 is compensated to allow 40 days of correction without 32 bit overflows
        ulLostSeconds /= 100;
    }
    if (*RTC_PRESCALER_LOCATION > RTC_TPR) {
        ulMissedCounts = (0x00008000 - (*RTC_PRESCALER_LOCATION - RTC_TPR));
    }
    else {
        ulMissedCounts = (RTC_TPR - *RTC_PRESCALER_LOCATION);
    }
    ulMissedCounts /= 1000;
    ulLostSeconds += ulMissedCounts;
    RTC_TSR = (*RTC_SECONDS_LOCATION + ulLostSeconds);                   // add lost seconds
    RTC_TPR = (32768 - 1000);                                            // next interrupt in 1s
    *RTC_SECONDS_LOCATION = RTC_TSR;
    *RTC_PRESCALER_LOCATION = (unsigned short)RTC_TPR;
    if (RTC_TAR != 0) {                                                  // alarm was active
        RTC_TAR = *RTC_ALARM_LOCATION;                                   // set the original alarm match time
        if (RTC_TAR < RTC_TSR) {                                         // if the alarm has been missed
            RTC_TAR = RTC_TSR;                                           // alarm will fire at next increment
        }
    }
    fnConvertSecondsTime(0, RTC_TSR);                                    // {1} take the present RTC seconds count value, convert and set to time and date
    if (iResync != 0) {
      //fnConvertSecondsTime(0, RTC_TSR);                                // {1} take the present seconds count value, convert and set to time and date
        RTC_SR = RTC_SR_TCE;                                             // allow the RTC operation to continue
    }
}


        #if defined SUPPORT_LOW_POWER
// The processor is to be set to a low leakage mode with no interrupt support, in which the RTC will be clocked with 1kHz rather than 32.768kHz
// If there is an alarm programmed to wake the processor its alarm time must be compensated accordingly (called from kinetis_low_power.h)
//
static void fnSlowRTC(void)
{
    if ((IS_POWERED_UP(6, RTC) != 0) && ((RTC_SR & RTC_SR_TCE) != 0) && (RTC_TAR != 0)) { // if the RTC is not enabled or not presently running we ignore any further checks
        unsigned long ulAlarmDelay;
        RTC_SR = 0;                                                      // suspend RTC counting during adjustment
        if (RTC_TAR != 0) {                                              // if alarm still hasn't fired before disabing RTC
            ulAlarmDelay = (RTC_TAR - RTC_TSR);                          // seconds remaining until a programmed alarm
            if (ulAlarmDelay > 1) {                                      // if the alarm will fire within one second period there is no adjustment required (because the prescaler will not overflow without an interrupt handling it)
                unsigned long ulFullSeconds;
                signed long slRemainer;
                unsigned long ulSecondCounts = ulAlarmDelay;             // full second delay remaining
                ulSecondCounts *= 1000;
                ulSecondCounts /= 32768;                                 // the complete second counter value for LPO clocking without prescaler adjustment
                ulFullSeconds = (ulSecondCounts * 32768);                // the total number of pulses accounted for by full second increments
                slRemainer = ((ulAlarmDelay * 1000) - ulFullSeconds);    // not accounted for pulses
                slRemainer = (32768 - slRemainer);                       // pre-scaler value to give the remainder
                slRemainer -= (RTC_TPR - (32768 - 1000));                // compensate for elapsed prescaler time since setting the alarm
                if (slRemainer < 0) {
                    slRemainer += 32768;
                    ulFullSeconds++;
                }
                RTC_TPR = (unsigned short)slRemainer;                    // adjust the present prescaler value to match the remainder
                *RTC_PRESCALER_LOCATION = (unsigned short)RTC_TPR;       // save the prescaler reference value so that the compensation can be reversed when it wakes
                RTC_TAR = (RTC_TSR + ulSecondCounts);                    // alarm match to be used by the RTC
            }
        }
        RTC_SR = RTC_SR_TCE;                                             // allow the RTC operation to continue
    }
}
        #endif
    #endif



// This routine attempts a high level of compatibility with the Coldfire V2 RTC
//
extern int fnConfigureRTC(void *ptrSettings)
{
    int iIRQ = 0;
    RTC_SETUP *ptr_rtc_setup = (RTC_SETUP *)ptrSettings;
    switch (ptr_rtc_setup->command & ~(RTC_DISABLE | RTC_INITIALISATION | RTC_SET_UTC | RTC_INCREMENT)) { // {51}
    case RTC_TIME_SETTING:                                               // set time to RTC
    #if !defined SUPPORT_SW_RTC && !(defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
        #if defined KINETIS_KM
        POWER_UP_ATOMIC(5, IRTC);                                        // ensure the RTC is powered
        #else
        POWER_UP_ATOMIC(6, RTC);                                         // ensure the RTC is powered
        #endif
        RTC_SR = 0;                                                      // temporarily disable RTC to avoid potential interrupt
    #endif
        if ((ptr_rtc_setup->command & RTC_SET_UTC) != 0) {               // {51} allow setting from UTC seconds value
            fnConvertSecondsTime(0, ptr_rtc_setup->ulLocalUTC);          // convert to time for internal use
    #if defined SUPPORT_SW_RTC || (defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
            *RTC_SECONDS_LOCATION = ptr_rtc_setup->ulLocalUTC;           // directly set the seconds value
    #else
            RTC_TSR = ptr_rtc_setup->ulLocalUTC;                         // directly set the seconds value
    #endif
        }
        else {
    #if defined SUPPORT_SW_RTC || (defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
            *RTC_SECONDS_LOCATION = fnConvertTimeSeconds(ptr_rtc_setup, 1); // set the present time and date as seconds since 1970 (Unix epoch)
    #else
            RTC_TPR = 0;                                                 // reset the prescaler 
            RTC_TSR = fnConvertTimeSeconds(ptr_rtc_setup, 1);            // set the present time and date as seconds since 1970 (Unix epoch)
    #endif
        }
    #if !defined SUPPORT_SW_RTC && !(defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
        #if !defined irq_RTC_SECONDS_ID                                  // {75}
        RTC_TAR = RTC_TSR;                                               // set next second interrupt alarm match
        #endif
        #if defined KINETIS_KL                                           // {5}
            #if defined RTC_USES_LPO_1kHz
        RTC_TPR = (32768 - 1000);                                        // synchronise the prescaler to trigger in 1s time
            #else
        RTC_TPR = 0;                                                     // reset the prescaler
            #endif
        #endif
        RTC_SR = RTC_SR_TCE;                                             // enable counter again
    #endif
        break;
    #if defined irq_RTC_SECONDS_ID && !defined SUPPORT_SW_RTC
    case RTC_GET_ALARM:
        fnConvertSecondsTime(ptr_rtc_setup, RTC_TAR);
        break;
    #else
    case RTC_GET_ALARM:
        fnConvertSecondsTime(ptr_rtc_setup, rtc_alarm);
        break;
    #endif
    case RTC_GET_TIME:                                                   // get the present time
        fnGetRTC(ptr_rtc_setup);
        break;
    case RTC_CONVERT_TO_UTC:                                             // {51}
        ptr_rtc_setup->ulLocalUTC = fnConvertTimeSeconds(ptr_rtc_setup, 0); // take time/date as input and return UTC value
        break;
    case RTC_CONVERT_FROM_UTC:                                           // {51}
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
            if ((RTC_SET_UTC & ptr_rtc_setup->command) != 0) {           // {3}
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
    #if defined KINETIS_KE && !defined KINETIS_WITH_SRTC && defined SUPPORT_RTC
        POWER_UP_ATOMIC(6, RTC);                                         // ensure the KE's RTC is powered
        rtc_interrupt_handler[iIRQ] = ((RTC_SETUP *)ptrSettings)->int_handler; // enter the handling interrupt
        fnEnterInterrupt(irq_RTC_OVERFLOW_ID, PRIORITY_RTC, (void (*)(void))_rtc_handler); // enter interrupt handler
        #if defined RTC_USES_EXT_CLK
        RTC_MOD = (((_EXTERNAL_CLOCK)/RTC_CLOCK_PRESCALER_1) - 1);       // set the match value for 1s
        #elif defined RTC_USES_INT_REF
        ICS_C1 |= (ICS_C1_IRCLKEN | ICS_C1_IREFSTEN);                    // enable the internal reference clock and allow it to continue running in stop mode
        RTC_MOD = (((ICSIRCLK)/RTC_CLOCK_PRESCALER_1) - 1);              // set the match value for 1s
        #elif defined RTC_USES_LPO_1kHz
        RTC_MOD = (((1000)/ RTC_CLOCK_PRESCALER_2) - 1);                 // set the match value for 1s
        #else
        RTC_MOD = (((_EXTERNAL_CLOCK)/RTC_CLOCK_PRESCALER_2) - 1);       // set the match value for 1s
        #endif
        RTC_SC = (RTC_SC_RTIE | RTC_SC_RTIF | _RTC_CLOCK_SOURCE | _RTC_PRESCALER); // clock the RTC from the defined clock source/pre-scaler and enable interrupt
        #if defined _WINDOWS
        if (RTC_MOD > 0xffff) {
            _EXCEPTION("MOD value too large (16 bits)");
        }
        #endif
    #elif defined SUPPORT_SW_RTC
        rtc_interrupt_handler[iIRQ] = ((RTC_SETUP *)ptrSettings)->int_handler; // enter the handling interrupt
    #else
        #if defined KINETIS_KM
        POWER_UP_ATOMIC(5, IRTC);                                        // enable access and interrupts to the RTC
        #else
        POWER_UP_ATOMIC(6, RTC);                                         // enable access and interrupts to the RTC
        #endif
        if ((RTC_SR & RTC_SR_TIF) != 0) {                                // if time invalid
        #if defined _WINDOWS
            unsigned long ulTSR = RTC_TSR;
        #endif
            RTC_SR = 0;                                                  // ensure stopped
            RTC_TSR = 0;                                                 // write to clear RTC_SR_TIF in status register when not yet enabled
        #if defined _WINDOWS
            RTC_TSR = ulTSR;                                             // return the present time (since we always exercise the power up case but do have the local time)
        #endif
        #if defined KINETIS_WITH_RTC_CRYSTAL                             // {6} devices with RTC crystal oscillator circuity
            #if defined RTC_USES_RTC_CLKIN                               // RTC counter clock supplied from RTC_CLKIN pin
                #if defined SIM_CHIPCTL_RTC32CLKSEL_RTC_CLKIN
            _CONFIG_PERIPHERAL(A, 7, (PA_7_RTC_CLKIN | PORT_NO_PULL));   // configure the RTC_CLKIN pin function
            SIM_CHIPCTL |= SIM_CHIPCTL_RTC32CLKSEL_RTC_CLKIN;            // RTC32CLK input selected from RTC_CLKIN pin
                #else
                #endif
            #elif defined RTC_USES_LPO_1kHz && defined RTC_CR_LPOS_LPO
            RTC_CR |= (RTC_CR_LPOS_LPO);                                 // clock the RTC counter from 1kHz LPO source (increments by 32 on each edge and so approximately 32kHz)
            #else
            if ((RTC_CR & RTC_CR_OSCE) == 0) {                           // {8} the RTC oscillator may have been enabled in the clock initialisation where the clock is being supplied to other peripherals
                RTC_CR = (RTC_CR_OSCE | RTC_CR_CLKO);                    // enable oscillator but don't supply clock to other peripherals (since not yet enabled)
            }
            return WAIT_STABILISING_DELAY;                               // the oscillator requires some time to stabilise so the user should call again after this time has expired
            #endif
        #endif
        }
        rtc_interrupt_handler[iIRQ] = ((RTC_SETUP *)ptrSettings)->int_handler; // enter the handling interrupt
        RTC_SR = 0;                                                      // temporarily disable RTC to avoid potentially missed seconds count
    #endif
        rtc_interrupts |= (0x01 << iIRQ);                                // flag interrupt(s) enabled
    #if defined irq_RTC_SECONDS_ID && !defined SUPPORT_SW_RTC            // {75} use the seconds interrupt rather than the alarm interrupt when available
        if (2 == iIRQ) {                                                 // alarm being set
            if (RTC_TSR > rtc_alarm) {                                   // avoid setting match value in the past since it won't (ever) fire
                rtc_alarm = RTC_TSR;                                     // set to next second value
            }
            RTC_TAR = rtc_alarm;                                         // set timer alarm interrupt match (write to RTC_TAR resets a pending alarm flag)
        #if defined KINETIS_KL && defined RTC_USES_LPO_1kHz
            *RTC_ALARM_LOCATION = rtc_alarm;
        #endif
        #if !defined irq_RTC_ALARM_ID && defined INTMUX0_AVAILABLE
            fnEnterInterrupt((irq_INTMUX0_0_ID + INTMUX_RTC_ALARM), INTMUX0_PERIPHERAL_RTC_ALARM, _rtc_alarm_handler); // enter RTC alarm interrupt handler based on INTMUX
        #else
            fnEnterInterrupt(irq_RTC_ALARM_ID, PRIORITY_RTC, _rtc_alarm_handler); // enter the alarm interrupt handler
        #endif
            RTC_IER |= RTC_IER_TAIE;                                     // enable alarm interrupt
        }
        else {
            fnEnterInterrupt(irq_RTC_SECONDS_ID, PRIORITY_RTC, _rtc_handler);
            RTC_IER = RTC_IER_TSIE;                                      // enable seconds interrupt
        }
    #elif !defined SUPPORT_SW_RTC && !(defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
        RTC_TAR = (RTC_TSR);                                             // set timer alarm interrupt at next second
        fnEnterInterrupt(irq_RTC_ALARM_ID, PRIORITY_RTC, _rtc_handler);
        RTC_IER = RTC_IER_TAIE;                                          // enable alarm interrupt
    #endif
        if ((ptr_rtc_setup->command & RTC_INITIALISATION) != 0) {
    #if defined irq_RTC_ALARM_ID
            fnClearPending(irq_RTC_ALARM_ID);                            // {10} clear pending alarm interrupt at initialisation
      //#if irq_RTC_ALARM_ID < 32                                        // {7} clear pending alarm interrupt at initialisation
      //    IRQ0_31_CPR = (1 << irq_RTC_ALARM_ID);
      //#elif irq_RTC_ALARM_ID < 64
      //    IRQ32_63_CPR = (1 << (irq_RTC_ALARM_ID - 32));
      //#else
      //    IRQ64_95_CPR = (1 << (irq_RTC_ALARM_ID - 64));
      //#endif
    #endif
    #if defined SUPPORT_SW_RTC || (defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
            if (*RTC_VALID_LOCATION != RTC_VALID_PATTERN) {              // power on reset
                *RTC_SECONDS_LOCATION = 0;                               // update the count and pre-scaler value in non-initialised ram
                *RTC_PRESCALER_LOCATION = 0;
                *RTC_ALARM_LOCATION = 0;
                *RTC_VALID_LOCATION = RTC_VALID_PATTERN;
            }
            else {
                fnConvertSecondsTime(0, *RTC_SECONDS_LOCATION);          // take the present seconds count value, convert and set to time and date
            }
    #elif defined KINETIS_KL                                             // {77}
        #if defined RTC_USES_RTC_CLKIN && !defined KINETIS_WITH_RTC_CRYSTAL // {6} external clock oscillator input on dedicated pin
            #if defined KINETIS_KL05
            _CONFIG_PERIPHERAL(A, 5,  (PA_5_RTC_CLKIN));                 // ensure the port is clocked and select pin function
            #else
            _CONFIG_PERIPHERAL(C, 1,  (PC_1_RTC_CLKIN));                 // ensure the port is clocked and select pin function
            #endif
            SIM_SOPT1 = ((SIM_SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) | SIM_SOPT1_OSC32KSEL_RTC_CLKIN); // select external clock source
            fnConvertSecondsTime(0, RTC_TSR);                            // {2} take the present seconds count value, convert and set to time and date
        #elif defined RTC_USES_LPO_1kHz                                  // clock the RTC from the 1kHz LPI clock source
            SIM_SOPT1 = ((SIM_SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) | SIM_SOPT1_OSC32KSEL_LPO_1kHz); // select 1kHz clock as source
            if (((RCM_SRS0 & (RCM_SRS0_POR | RCM_SRS0_LVD)) != 0) || (*RTC_VALID_LOCATION != RTC_VALID_PATTERN)) { // power on reset
            #if defined _WINDOWS
                RTC_TSR = 0;
            #endif
                *RTC_SECONDS_LOCATION = 0;                               // update the count and pre-scaler value in non-initialised ram
                *RTC_PRESCALER_LOCATION = 0;
                *RTC_ALARM_LOCATION = 0;
                *RTC_VALID_LOCATION = RTC_VALID_PATTERN;                 // mark that the content is now valid
            }
            else {                                                       // not a power fail reset
                fnRestoreRTC(0);                                         // since the values in RAM are valid we can set a start value to the RTC
            }
            if (RTC_TPR < (32768 - 1000)) {                              // check whether the prescaler is indicating that the RTC is free-running without interrupt synchronisation support (eg. when re-started after debugging or after a reset)
                RTC_TPR = (32768 - 1000);                                // use 1kHz internal clock to generate 1s time base
            }
        #else                                                            // use 32kHz oscillator (OSC32KCLK) - this requires a 32kHz crystal to be used
            #if CRYSTAL_FREQUENCY != 32768
                #error "RTC operation from OSC32KCLK requires a 32768Hz crystal to be used by the oscillator!!"
            #endif
            SIM_SOPT1 = (SIM_SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK);         // select the clock source
            #if defined KINETIS_WITH_RTC_CRYSTAL                         // {6}
            SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL_32k;                        // select the 32kHz RTC oscillator clock as input
            #endif
            fnConvertSecondsTime(0, RTC_TSR);                            // {2} take the present seconds count value, convert and set to time and date
        #endif
    #else
            fnConvertSecondsTime(0, RTC_TSR);                            // take the present seconds count value, convert and set to time and date
    #endif
        }
    #if !defined SUPPORT_SW_RTC && !(defined KINETIS_KE && !defined KINETIS_WITH_SRTC)
        #if defined RTC_CLOCKOUT_ENABLE_32kHz && defined RTC_CR_CPE
            #if defined RTC_CLOCKOUT_ENABLE_32kHz_PIN
                #if defined RTC_CLKOUT_ON_D
        _CONFIG_PERIPHERAL(D, 13, (PD_13_RTC_CLKOUT | PORT_SRE_SLOW | PORT_DSE_LOW)); // configure the RTC_CLKOUT pin function
                #elif defined RTC_CLKOUT_ON_C_5
        _CONFIG_PERIPHERAL(C, 5,  (PC_5_RTC_CLKOUT | PORT_SRE_SLOW | PORT_DSE_LOW)); // configure the RTC_CLKOUT pin function
                #else       
        _CONFIG_PERIPHERAL(C, 4,  (PC_4_RTC_CLKOUT | PORT_SRE_SLOW | PORT_DSE_LOW)); // configure the RTC_CLKOUT pin function
                #endif
            #endif
        RTC_CR |= (RTC_CR_CPE | RTC_CR_CPS_32k);                         // select 32kHz for output on RTC_CLKOUT
        RTC_CR &= ~(RTC_CR_CLKO);                                        // allow the 32kHz signal to be passed to other peripherals
        #endif
        RTC_SR = RTC_SR_TCE;                                             // enable counter
    #endif
        break;
    }
    return 0;
}

