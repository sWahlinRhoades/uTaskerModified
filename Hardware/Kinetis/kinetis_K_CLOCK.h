/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_K_CLOCK.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    24.04.2018 Allow clocking from RTC crystal with automatic detection of revision 1 and revision 2 parts {1}
    21.08.2018 Add MCGFFLCLK configuration when used for FlexTimer fixed clock {2}
    20.01.2020 Disable FLL/PLL in bypass mode when running from external clock {3}
    20.01.2020 Add wait for HSRUN mode switch                                  {4}

*/


// Initially the processor is in FEI (FLL engaged internal) - running from 20..25MHz internal clock (32.768kHz IRC x 640 FLL factor; 20.97MHz)
//
#if defined RUN_FROM_DEFAULT_CLOCK                                       // no configuration performed - remain in default clocked mode (FLL is used from 32kHz IRC)
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides
    #if defined FLL_FACTOR                                               // if a different FLL multiplication factor is defined
    MCG_C4 = ((MCG_C4 & ~(MCG_C4_DMX32 | MCG_C4_HIGH_RANGE)) | (_FLL_VALUE)); // adjust FLL factor to obtain the required operating frequency
    #endif
#elif defined KINETIS_K22 && defined RUN_FROM_LIRC
    MCG_SC = MCG_SC_FCRDIV_1;                                            // no divide after fast clock (4MHz)
    MCG_C2 |= MCG_C2_IRCS;                                               // select fast internal reference clock (rather than slow one) for MCGIRCLK
    MCG_C1 = (MCG_C1_IREFSTEN | MCG_C1_IRCLKEN | MCG_C1_CLKS_INTERN_CLK);// enable and select 4MHz IRC clock source and allow it to continue operating in STOP mode
    while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_INTERN_CLK) {       // wait until the 4MHz IRC source is selected
    #if defined _WINDOWS
        MCG_S &= ~MCG_S_CLKST_MASK;
        MCG_S |= MCG_S_CLKST_INTERN_CLK;
    #endif
    }
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides
    MCG_C2 |= MCG_C2_LP;                                                 // disable FLL in bypass mode
#else
    #if defined EXTERNAL_CLOCK || defined CLOCK_FROM_RTC_OSCILLATOR      // external oscillator source - first move from state FEI to state FBE
        #if defined RUN_FROM_HIRC_PLL
    MCG_C7 = MCG_C7_OSCSEL_IRC48MCLK;                                    // route the IRC48M clock to the external reference clock input (this enables IRC48M)
        #endif
        #if defined CLOCK_FROM_RTC_OSCILLATOR || defined RUN_FROM_RTC_FLL// the input clock is derived from the RTC clock so we need to ensure that the RTC is enabled
    POWER_UP_ATOMIC(6, RTC);                                             // enable access to the RTC
            #if defined KINETIS_REVISION_1 && defined KINETIS_REVISION_2 // {1} revision 1 and revision 2 part compatibility is required
    SIM_SOPT2 |= SIM_SOPT2_MCGCLKSEL;                                    // attempt to select external source (revision 1 parts)
    if ((SIM_SOPT2 & SIM_SOPT2_MCGCLKSEL) == 0) {                        // rev 2 parts won't set this bit so we can tell that the setting must be performed in the MCG module instead
        MCG_C7 = MCG_C7_OSCSEL_32K;                                      // select the RTC clock as external clock input to the FLL or MCGOUTCLK
    }
            #elif defined KINETIS_REVISION_2
    MCG_C7 = MCG_C7_OSCSEL_32K;                                          // select the RTC clock as external clock input to the FLL or MCGOUTCLK
            #else
    SIM_SOPT2 |= SIM_SOPT2_MCGCLKSEL;                                    // select the RTC clock as external clock input to the FLL or MCGOUTCLK (revison 1 parts did this in the SIM module rather than in the MCG module)
            #endif
    RTC_CR = (RTC_CR_OSCE);                                              // enable RTC oscillator and output the 32.768kHz output clock so that it can be used by the MCG (the first time that it starts it can have a startup/stabilisation time but this is not critical for the FLL usage)
            #if defined FLL_FACTOR
    MCG_C1 = ((MCG_C1_CLKS_PLL_FLL | MCG_C1_FRDIV_RANGE0_1) & ~MCG_C1_IREFS); // switch the FLL input to the undivided external clock source (RTC)
            #endif
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides ready for the final operating frequency
    _WAIT_REGISTER_TRUE(MCG_S, MCG_S_IREFST);                             // wait until the switch to the external RTC source has completed
            #if defined FLL_FACTOR
    MCG_C4 = ((MCG_C4 & ~(MCG_C4_DMX32 | MCG_C4_HIGH_RANGE)) | (_FLL_VALUE)); // adjust FLL factor to obtain the required operating frequency
            #endif
        #elif defined FLL_FACTOR
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides ready for the final operating frequency
    MCG_C4 = ((MCG_C4 & ~(MCG_C4_DMX32 | MCG_C4_HIGH_RANGE)) | (_FLL_VALUE)); // adjust FLL factor to obtain the required operating frequency
        #else                                                            // external oscillator
            #if EXTERNAL_CLOCK >= 8000000
    MCG_C2 = (MCG_C2_RANGE_8M_32M | MCG_C2_LOCRE0);                      // select external clock source (with reset on clock loss)
    MCG_C1 = (MCG_C1_CLKS_EXTERN_CLK | MCG_C1_FRDIV_1024);               // switch to external input clock (the FLL input clock is set to as close to its input range as possible, although this is not absolutely necessary since the FLL will not be used)
            #elif !defined CLOCK_FROM_RTC_OSCILLATOR
    MCG_C2 = (MCG_C2_RANGE_1M_8M | MCG_C2_LOCRE0);                       // select external clock source (with reset on clock loss)
    MCG_C1 = (MCG_C1_CLKS_EXTERN_CLK | MCG_C1_FRDIV_128);                // switch to external input clock (the FLL input clock is set to as close to its input range as possible, although this is not absolutely necessary since the FLL will not be used)
            #endif
        #endif
    #else                                                                // crystal source
        #if defined FLL_FACTOR || defined RUN_FROM_EXTERNAL_CLOCK
    OSC0_CR = (OSC_CR_ERCLKEN | OSC_CR_EREFSTEN);                        // enable the oscillator and allow it to continue oscillating in stop mode
        #endif
    MCG_C2 = (MCG_C2_FREQ_RANGE | MCG_C2_GAIN_MODE | MCG_C2_EREFS | MCG_C2_LOCRE0); // select crystal oscillator and select a suitable range
    MCG_C1 = (MCG_C1_CLKS_EXTERN_CLK | MCG_C1_FRDIV_VALUE);              // switch to external source (the FLL input clock is set to as close to its input range as possible, although this is not absolutely necessary if the FLL will not be used)
    _WAIT_REGISTER_FALSE(MCG_S, MCG_S_OSCINIT);                          // loop until the crystal source has been selected
        #if defined FLL_FACTOR
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides
    MCG_C4 = ((MCG_C4 & ~(MCG_C4_DMX32 | MCG_C4_HIGH_RANGE)) | (_FLL_VALUE)); // adjust FLL factor to obtain the required operating frequency
    MCG_C1 &= ~(MCG_C1_CLKS_INTERN_CLK | MCG_C1_CLKS_EXTERN_CLK | MCG_C1_IREFS); // move to FEE (MCGOUTCLK is derived from the FLL clock), selecting the external clock as source (rather than slow internal clock)
    while ((MCG_S & (MCG_S_CLKST_EXTERN_CLK | MCG_S_CLKST_INTERN_CLK)) != MCG_S_CLKST_FLL) { // loop until the FLL clock source becomes valid
            #if defined _WINDOWS
        MCG_S &= ~(MCG_S_CLKST_EXTERN_CLK | MCG_S_CLKST_INTERN_CLK);
            #endif
    }
        #endif
    #endif
        #if !defined CLOCK_FROM_RTC_OSCILLATOR
    _WAIT_REGISTER_TRUE(MCG_S, MCG_S_IREFST);                            // loop until the FLL source is no longer the internal reference clock
        #if !defined FLL_FACTOR
    while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_EXTERN_CLK) {       // loop until the external reference clock source is valid
            #if defined _WINDOWS
        MCG_S &= ~MCG_S_CLKST_MASK;
        MCG_S |= MCG_S_CLKST_EXTERN_CLK;
            #endif
    }
            #if defined RUN_FROM_EXTERNAL_CLOCK                          // {101}
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides
    MCG_C2 |= MCG_C2_LP;                                                 // {3} disable FLL in bypass mode
            #endif
        #endif
        #if defined CLOCK_DIV_1                                          // {90} PLL1 is to be configured and enabled 
    MCG_C11 = (MCG_C11_PLLREFSEL1_OSC0 | MCG_C11_PLLCLKEN1 | (CLOCK_DIV_1 - 1)); // use OSCO clock source for PLL1 with input divide set
    MCG_C12 = (CLOCK_MUL_1 - MCG_C12_VDIV1_LOWEST);                      // set multiplier - we don't wait for PLL1 to lock until after PLL0 setup has completed (allow them to lock in parallel)
        #endif
        #if !defined FLL_FACTOR && !defined RUN_FROM_EXTERNAL_CLOCK      // {95}
    MCG_C5 = ((CLOCK_DIV - 1) | MCG_C5_PLLSTEN0);                        // now move from state FEE to state PBE (or FBE) PLL remains enabled in normal stop modes
    MCG_C6 = ((CLOCK_MUL - MCG_C6_VDIV0_LOWEST) | MCG_C6_PLLS);          // set the PLL multiplication factor
    while ((MCG_S & MCG_S_PLLST) == 0) {                                 // loop until the PLLS clock source becomes valid
            #if defined _WINDOWS
        MCG_S |= MCG_S_PLLST;
            #endif
    }
    while ((MCG_S & MCG_S_LOCK) == 0) {                                  // loop until PLL locks
            #if defined _WINDOWS
        MCG_S |= MCG_S_LOCK;
            #endif
    }
    SIM_CLKDIV1 = (((SYSTEM_CLOCK_DIVIDE - 1) << 28) | ((BUS_CLOCK_DIVIDE - 1) << 24) | ((FLEX_CLOCK_DIVIDE - 1) << 20) | ((FLASH_CLOCK_DIVIDE - 1) << 16)); // prepare bus clock divides
            #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined HIGH_SPEED_RUN_MODE_REQUIRED
    SMC_PMPROT = SMC_PMPROT_LOW_POWER_LEVEL;                             // ensure protection allows HSRUN mode (if allowed)
    SMC_PMCTRL = SMC_PMCTRL_RUNM_HSRUN;                                  // {118} set high speed run mode (restrictions apply) so that the clock speeds can be obtained
    while (SMC_PMSTAT != SMC_PMSTAT_HSRUN) {                             // {4} ensure that HSRUN mode has been entered before increasing the system frequency
                #if defined _WINDOWS
        SMC_PMSTAT = SMC_PMSTAT_HSRUN;
                    #endif
    }
            #endif
    MCG_C1 = (MCG_C1_CLKS_PLL_FLL | MCG_C1_FRDIV_1024);                  // finally move from PBE to PEE mode - switch to PLL clock
    while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_PLL) {              // loop until the PLL clock is selected
            #if defined _WINDOWS
        MCG_S &= ~MCG_S_CLKST_MASK;
        MCG_S |= MCG_S_CLKST_PLL;
            #endif
    }
        #endif
        #if defined CLOCK_DIV_1                                          // {90} PLL1 used by FS USB or SDRAM
    while ((MCG_S2 & MCG_S2_LOCK1) == 0) {                               // loop until PLL1 locks
                #if defined _WINDOWS
        MCG_S2 |= MCG_S2_LOCK1;
                #endif
    }
        #endif
        #if defined PERIPHERAL_CLOCK_DIVIDE_VALUE                        // configure the optional peripheral clock divide early since it shouldn't be changed once LPUARTs or TPU have started using it
    SIM_CLKDIV3 = PERIPHERAL_CLOCK_DIVIDE_VALUE;
        #endif
    #endif
#endif

#if defined FTM_FLEXIBLE_CLOCKING                                        // {2}
    // When FTM flexible clocking is set together with an MCGFFCLK setting the MCGFFCLK is configured immediately and it is assumed that this will not interfere with FLL operation if used!
    //
    #if defined MCGFFLCLK_32kHz_IRC                                      // if the 32kHz IRC is used as MCGFFCLK
    // Ensure that slow internal clock is enabled and selected for MCGFFCLK
    //
    MCG_C1 |= (MCG_C1_IRCLKEN | MCG_C1_IREFSTEN | MCG_C1_IREFS);         // enable internal reference clock and allow it to continue running in stop modes, plus select it as FLL reference (MCGFFLCLK)
    #elif defined MCGFFLCLK_EXTERNAL
    // Select external oscillator/crystal as source for MCGFFCK and the desired FRDIV divider
    //
    MCG_C1 &= ~(MCG_C1_IREFS | MCG_C1_FRDIV_RANGE0_128);                 // select external path as FLL reference (MCGFFLCLK)
    MCG_C1 |= (MCG_C1_FRDIV_VALUE);                                      // set FRDIV value
    MCG_C7 = MCG_C7_OSCSEL_OSCCLK;                                       // select the external oscillator path
    #elif defined MCGFFLCLK_IRC48M
    // Select 48MHz IRC48M as source for MCGFFCK and the desired FRDIV divider
    //
    MCG_C1 &= ~(MCG_C1_IREFS | MCG_C1_FRDIV_RANGE0_128);                 // select external path as FLL reference (MCGFFLCLK)
    MCG_C1 |= (MCG_C1_FRDIV_VALUE);                                      // set FRDIV value
    MCG_C7 = MCG_C7_OSCSEL_IRC48MCLK;                                    // set the IRC48M path
    #endif
#endif
