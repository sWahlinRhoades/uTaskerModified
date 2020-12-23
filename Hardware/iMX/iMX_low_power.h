/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_low_power.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
*/

#if defined LOW_POWER_CYCLING_MODE
    int iLowPowerLoopMode = 0;
#endif

// This routine switches to low power mode. It is always called with disabled interrupts
// - when the routine is called there were no system events pending so the system can be set to a low power mode until a masked interrupt becomes pending
//
extern void fnDoLowPower(void)
{
    register unsigned long ulDeepSleepMode = SYSTEM_CONTROL_REGISTER;    // present stop mode type
    if ((ulDeepSleepMode & SLEEPDEEP) != 0) {                            // the present mode is a stop based mode
        if (ulPeripheralNeedsClock != 0) {                               // don't enter stop mode if there is a peripheral that hasn't completed use of the clocks that will be stopped
            SYSTEM_CONTROL_REGISTER &= ~(SLEEPDEEP);                     // use wait mode until the transmission has completed
        }
        else {                                                           // there is no active transmission so we check that there is no reception starting and enable reception wakeup interrupts
    #if defined SERIAL_INTERFACE_
            if (IS_POWERED_UP(5, LPUART1_CLOCK)) {                       // if LPUART1 is enabled
                LPUART0_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART0_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART0_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #if LPUARTS_AVAILABLE > 1
            if (IS_POWERED_UP(0, LPUART2_CLOCK)) {                       // if LPUART2 is enabled
                LPUART1_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART1_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART1_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 2
            if (IS_POWERED_UP(0, LPUART3_CLOCK)) {                       // if LPUART3 is enabled
                LPUART2_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART2_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART2_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 3
            if (IS_POWERED_UP(1, LPUART4_CLOCK)) {                       // if LPUART4 is enabled
                LPUART3_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART3_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART3_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 4
            if (IS_POWERED_UP(3, LPUART5_CLOCK)) {                       // if LPUART5 is enabled
                LPUART4_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART4_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART4_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 5
            if (IS_POWERED_UP(3, LPUART6_CLOCK)) {                       // if LPUART6 is enabled
                LPUART5_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART5_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART5_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 6
            if (IS_POWERED_UP(5, LPUART7_CLOCK)) {                       // if LPUART7 is enabled
                LPUART6_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART6_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART6_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
        #if LPUARTS_AVAILABLE > 7
            if (IS_POWERED_UP(6, LPUART8_CLOCK)) {                       // if LPUART8 is enabled
                LPUART7_STAT |= LPUART_STAT_RXEDGIF;                     // clear edge flag
                LPUART7_BAUD |= LPUART_BAUD_RXEDGIE;                     // enable wakeup on RxD falling edge
                if ((LPUART7_STAT & LPUART_STAT_RAF) != 0) {             // if the receiver active flag is set it means that reception has already started so we don't enter stop mode
                    SYSTEM_CONTROL_REGISTER &= ~SLEEPDEEP;               // use wait mode until the reception has completed
                }
            }
        #endif
    #endif
        }
    }
    #if defined LOW_POWER_CYCLING_MODE
    if (iLowPowerLoopMode == LOW_POWER_CYCLING_PAUSED) {                 // if the low power cycling had been paused we enable it before sleeping
        iLowPowerLoopMode = LOW_POWER_CYCLING_ENABLED;
    }
    do {
    #endif
    #if !defined _WINDOWS
  //TOGGLE_TEST_OUTPUT();
    __sleep_mode();                                                      // enter low power mode using wait for interrupt processor state
  //TOGGLE_TEST_OUTPUT();
    #endif
    // The processor will continue after being woken by any pending interrupt (also when the global interrupt mask is still set)
    // - this mean that the processor has woken again when the code execution reaches this location
    //
    #if defined LOW_POWER_CYCLING_MODE
    } while ((iLowPowerLoopMode >= LOW_POWER_CYCLING_ENABLED) && (fnVirtualWakeupInterruptHandler((ulDeepSleepMode & SLEEPDEEP) != 0))); // allow the user to optionally quickly handle wakeup event without quitting the sleep loop
    #endif
    SYSTEM_CONTROL_REGISTER = ulDeepSleepMode;                           // ensure present stop mode has been returned (we may have used wait mode instead due to present peripheral activity)
    #if defined SERIAL_INTERFACE_
    if (IS_POWERED_UP(5, LPUART1_CLOCK) != 0) {                          // if LPUART1 is enabled
        LPUART0_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #if LPUARTS_AVAILABLE > 1
    if (IS_POWERED_UP(0, LPUART2_CLOCK) != 0) {                          // if LPUART2 is enabled
        LPUART1_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 2
    if (IS_POWERED_UP(0, LPUART3_CLOCK) != 0) {                          // if LPUART3 is enabled
        LPUART2_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 3
    if (IS_POWERED_UP(1, LPUART4_CLOCK) != 0) {                          // if LPUART4 is enabled
        LPUART3_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 4
    if (IS_POWERED_UP(3, LPUART5_CLOCK) != 0) {                          // if LPUART5 is enabled
        LPUART4_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 5
    if (IS_POWERED_UP(3, LPUART6_CLOCK) != 0) {                          // if LPUART6 is enabled
        LPUART5_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 6
    if (IS_POWERED_UP(5, LPUART7_CLOCK) != 0) {                          // if LPUART7 is enabled
        LPUART6_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
        #if LPUARTS_AVAILABLE > 7
    if (IS_POWERED_UP(6, LPUART8_CLOCK) != 0) {                          // if LPUART8 is enabled
        LPUART7_BAUD &= ~LPUART_BAUD_RXEDGIE;                            // disable edge interrupt on RxD since we never want to handle the actual interrupt (used just for waking)
    }
        #endif
    #endif
  //TOGGLE_TEST_OUTPUT();
    uEnable_Interrupt();                                                 // enable interrupts so that the masked interrupt that became pending can be taken
  //TOGGLE_TEST_OUTPUT();
}

static int fnPresentLP_mode(void)
{
    if ((SYSTEM_CONTROL_REGISTER & SLEEPDEEP) != 0) {                    // if the sleep deep flag is set the stop mode is active
        return STOP_MODE;
    }
    return WAIT_MODE;
}

// Determine the present low power mode that is presently configured
//
extern int fnGetLowPowerMode(void)
{
    // Determine the present low power mode of operation
    //
    if ((uGetTaskState(TASK_LOW_POWER) & (UTASKER_GO | UTASKER_POLLING)) == 0) {
        return RUN_MODE;                                                 // low power task not operating
    }
    return (fnPresentLP_mode());
}


// Set a new low power mode
// - note that some modes will not be able to fully operate or changes between certain modes may not be possible
//
extern void fnSetLowPowerMode(int new_lp_mode)
{
    int next_lp_mode = (new_lp_mode & ~(LOW_POWER_OPTIONS));
    switch (next_lp_mode) {                                              // the new mode to be used
    case RUN_MODE:                                                       // no low power operation
        uTaskerStateChange(TASK_LOW_POWER, UTASKER_STOP);                // stop low power task from operating
        // Fall through intentionally
        //
    case WAIT_MODE:                                                      // wait mode
        SYSTEM_CONTROL_REGISTER &= ~(SLEEPDEEP);
        if (RUN_MODE == next_lp_mode) {
            return;
        }
        break;
    case STOP_MODE:                                                      // stop mode
        SYSTEM_CONTROL_REGISTER |= SLEEPDEEP;
        break;
    }
    uTaskerStateChange(TASK_LOW_POWER, UTASKER_GO);                      // ensure low power task is operating
}
