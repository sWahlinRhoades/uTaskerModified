/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland
    
    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_Clock.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if defined ARM_PLL_AVAILABLE && !defined PLL1_CLOCK_OSC_CLK
    #define _ENABLE_PLL1
#endif

#if defined PERIPH_CLK_SOURCE_PLL2_528M
    #define _ENABLE_PLL2
#endif
#if defined PERIPH_CLK_SOURCE_PLL2_PFD3
    #define _ENABLE_PLL2
    #define PLL2_PFD3_USED
#endif
#if defined SEMC_CLK_FROM_PLL2_PFD2 || defined FLEXSPI_CLK_FROM_PLL2_PFD2
    #define _ENABLE_PLL2
    #define PLL2_PFD2_USED
#endif
#if !defined iMX_BOOTLOADER 
    #if defined FLEXSPI2_CLK_FROM_PLL2
        #define _ENABLE_PLL2
    #endif
    #if defined LPSPI_CLK_FROM_PLL2
        #define _ENABLE_PLL2
    #endif
    #if defined LPSPI_CLK_FROM_PLL2_PDF2
        #define _ENABLE_PLL2
        #define PLL2_PFD2_USED
    #endif
    #if defined SDCARD_SUPPORT
        #define _ENABLE_PLL2
    #endif
#endif

#if defined PERIPH_CLK_SOURCE_PLL3_SW_CLK || defined PERIPH_CLK_SOURCE_PLL3_PFD3 || (defined USB_INTERFACE && ((HSUSB_CONTROLLERS == 1) || (defined USE_BOTH_USB || !defined USE_USB2_ONLY)))
    #define _ENABLE_PLL3
#endif
#if defined ETH_INTERFACE || defined PERIPH_CLK_SOURCE_PLL6_500M
    #define _ENABLE_PLL6
#endif
#if (HSUSB_CONTROLLERS > 1) && defined USB_INTERFACE && (defined USE_USB2_ONLY || defined USE_BOTH_USB)
    #define _ENABLE_PLL7
#endif


// Generate a clock output on CCM_CLKO1 (with optional 1..8 pre-scaler)
//
extern void _fnSetClock1Output(unsigned long ulClockSource, unsigned char ucDivide)
{
    CCM_CCOSR = ((CCM_CCOSR & ~(CCM_CCOSR_CLKO1_SEL_PLL4_MAIN_CLK | CCM_CCOSR_CLKO1_DIV8 | CCM_CCOSR_CLKOUT_SEL_CLKO2)) | CCM_CCOSR_CLKO1_EN | ulClockSource | (((ucDivide - 1) & 0x7) << 4)); // select the CCM_CLKO1 source
#if defined iMX_RT1011
    _CONFIG_PERIPHERAL(GPIO_SD_02, CCM_CLKO1, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO1 on GPIO2-02 - alt 3
#elif defined iMX_RT105X || defined iMX_RT106X
    _CONFIG_PERIPHERAL(GPIO_SD_B0_04, CCM_CLKO1, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO1 on GPIO3-16 - alt 6
#else
    _CONFIG_PERIPHERAL(GPIO_SD_B1_02, CCM_CLKO1, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO1 on GPIO3-22 - alt 6
#endif
}


// Generate a clock output on CCM_CLKO2 (with optional 1..8 pre-scaler)
//
extern void _fnSetClock2Output(unsigned long ulClockSource, unsigned char ucDivide)
{
    CCM_CCOSR = ((CCM_CCOSR & ~(CCM_CCOSR_CLKO2_SEL_MASK | CCM_CCOSR_CLKO2_DIV8)) | CCM_CCOSR_CLKO2_EN | ulClockSource | (((ucDivide - 1) & 0x7) << 21)); // select the CCM_CLKO2 source
#if defined iMX_RT1011
    _CONFIG_PERIPHERAL(GPIO_SD_01, CCM_CLKO2, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO2 on GPIO2-01 - alt 3
#elif defined iMX_RT105X || defined iMX_RT106X
    _CONFIG_PERIPHERAL(GPIO_SD_B0_05, CCM_CLKO2, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO2 on GPIO3-17 - alt 6
#else
    _CONFIG_PERIPHERAL(GPIO_SD_B1_03, CCM_CLKO2, (PORT_SRE_FAST | PORT_SPEED_MAX | PORT_DSE_MID)); // CCM_CLKO2 on GPIO3-23 - alt 6
#endif
}


// Beware that this routine MAY NOT access global variables since they haven't been initialised at this point
//
static void fnConfigure_iMX_RT_Clocks(void)
{
#if !defined RUN_FROM_DEFAULT_CLOCK
    // Initially the source clock for the core is derived from OSC_CLK (24MHz) from bypassed system PLL2
    // - we switch this initially to OSC_CLK input from the PERIPH CLK2 path so that any subsequent changes to the system PLL configuration can't cause potentially out of specification frequencies to transfer to the core or clocks derived from it
    //
    _CLEAR_REGISTER(XTALOSC24M_MISC0, CCM_ANALOG_MISC0_XTAL_24M_PWD);    // make sure that the 24MHz oscillator is powered up
    _WAIT_REGISTER_FALSE(XTALOSC24M_LOWPWR_CTRL, XTALOSC24M_LOWPWR_CTRL_XTALOSC_PWRUP_STAT); // wait until the oscillator is stable and ready to use
    _SET_REGISTER(XTALOSC24M_MISC0, CCM_ANALOG_MISC0_OSC_XTALOK_EN);     // enable detector that signals when the 24MHz crystal oscillator is stable
    _WAIT_REGISTER_FALSE(CCM_ANALOG_MISC0, CCM_ANALOG_MISC0_OSC_XTALOK); // wait until the frequency is stable
    _CLEAR_REGISTER(XTALOSC24M_MISC0, CCM_ANALOG_MISC0_OSC_XTALOK_EN);   // disable detector after use
    _CLEAR_REGISTER(XTALOSC24M_LOWPWR_CTRL, (XTALOSC24M_LOWPWR_CTRL_OSC_SEL_RC_OSC | XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN)); // select the 24MHz crystal as source and disable the RC oscillator
    #if defined PERIPH_CLK_SOURCE_OSC                                    // PERIPH_CLK sourced from OSC_CLK
        #if defined CCM_CBCDR_PERIPH_CLK2_PODF_MASK                      // optional OSC_CLK prescaler available
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_PERIPH_CLK2_PODF_MASK)) | ((PERIPH_CLK_SOURCE_OSC_DIVIDE - 1) << 27) | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #else
    CCM_CBCDR = (CCM_CBCDR | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #endif
    #else
        #if defined CCM_CBCDR_PERIPH_CLK2_PODF_MASK                      // optional OSC_CLK prescaler available
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_PERIPH_CLK2_PODF_MASK)) | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #else
    CCM_CBCDR = (CCM_CBCDR | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #endif
    #endif
    _WAIT_REGISTER_TRUE(CCM_CDHIPR, CCM_CDHIPR_PERIPH2_CLK_SEL_BUSY);    // wait until the mux handshake process has completed
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CBCMR_PERIPH_CLK2_SEL_MASK)) | CCM_CBCMR_PERIPH_CLK2_SEL_OSC_CLK);
#endif
#if !defined iMX_BOOTLOADER && !defined iMX_RT1011                       // access to CCM_CCOSR fails on RT 1011 here, although the calls work later from application.c
    // Test internal clock frequencies
    //
    #if defined iMX_RT105X || defined iMX_RT106X
    //fnSetClock1Output(USB1_PLL_DIV2, 8);
    //fnSetClock1Output(SYS_PLL_DIV2, 8);
    //fnSetClock1Output(VIDEO_PLL_DIV2, 8);
    //fnSetClock1Output(SEMC_CLK_ROOT, 8);
    //fnSetClock1Output(AHB_CLK_ROOT, 8);
    //fnSetClock1Output(IPG_CLK_ROOT, 8);
    //fnSetClock1Output(PERCLK_ROOT, 8);
    //fnSetClock1Output(CKIL_SYNC_CLK_ROOT, 8);
    //fnSetClock1Output(PLL4_MAIN_CLK, 8);

    //fnSetClock2Output(USDHC1_CLK_ROOT, 8);
    //fnSetClock2Output(LPI2C_CLK_ROOT, 8);
    //fnSetClock2Output(OSC_CLK_ROOT, 8);
    //fnSetClock2Output(LPSPI_CLK_ROOT, 8);
    //fnSetClock2Output(USDHC2_CLK_ROOT, 8);
    //fnSetClock2Output(SAI1_CLK_ROOT, 8);
    //fnSetClock2Output(SAI2_CLK_ROOT, 8);
    //fnSetClock2Output(SAI3_CLK_ROOT, 8);
    //fnSetClock2Output(TRACE_CLK_ROOT, 8);
    //fnSetClock2Output(CAN_CLK_ROOT, 8);
    //fnSetClock2Output(FLEXSPI_CLK_ROOT, 8);
    //fnSetClock2Output(UART_CLK_ROOT, 8);
    //fnSetClock2Output(SPDIF0_CLK_ROOT, 8);
    #else
    //fnSetClock1Output(ENET_PLL_DIV2, 8);
    //fnSetClock2Output(USDHC1_CLK_ROOT, 8);
    #endif
#endif
#if !defined iMX_BOOTLOADER                                              // since the boot loader boots from QSPI flash we don't touch its configuration
    #if defined SEMC_CLK_AVAILABLE
    // Prepare SEMC_CLK_ROOT
    //
        #if defined SEMC_CLK_FROM_PLL2_PFD2
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_SEMC_ALT_CLK_SEL_PLL3_PFD1 | CCM_CBCDR_SEMC_PODF_DIV8)) | CCM_CBCDR_SEMC_CLK_SEL_SEMC | ((SEMC_CLK_ROOT_DIVIDE - 1) << 16));
        #elif defined SEMC_CLK_FROM_PLL3_PFD1
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_SEMC_PODF_DIV8)) | CCM_CBCDR_SEMC_ALT_CLK_SEL_PLL3_PFD1 | CCM_CBCDR_SEMC_CLK_SEL_SEMC | ((SEMC_CLK_ROOT_DIVIDE - 1) << 16));
        #else                                                            // SEMC_CLK_FROM_PERIPH_CLK
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_SEMC_CLK_SEL_SEMC | CCM_CBCDR_SEMC_ALT_CLK_SEL_PLL3_PFD1 | CCM_CBCDR_SEMC_PODF_DIV8)) | ((SEMC_CLK_ROOT_DIVIDE - 1) << 16));
        #endif
    #endif

    // Prepare FLEXSPI_CLK_ROOT
    //
    #if defined FLEXSPI_CLK_FROM_PLL3_PFD0
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_FLEXSPI_PODF_MASK)) | CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | ((FLEXSPI_CLK_ROOT_DIVIDE - 1) << 23));
    #elif defined FLEXSPI_CLK_FROM_PLL2_PFD2
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CSCMR1_FLEXSPI_PODF_MASK)) | CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL2_PFD2 | ((FLEXSPI_CLK_ROOT_DIVIDE - 1) << 23));
    #elif defined FLEXSPI_CLK_FROM_PLL3_SW_CLK
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CSCMR1_FLEXSPI_PODF_MASK)) | CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_SW_CLK | ((FLEXSPI_CLK_ROOT_DIVIDE - 1) << 23));
    #elif defined iMX_RT1011                                             // FLEXSPI_CLK_FROM_PLL2
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CSCMR1_FLEXSPI_PODF_MASK)) | ((FLEXSPI_CLK_ROOT_DIVIDE - 1) << 23));
    #else                                                                // FLEXSPI_CLK_FROM_SEMC_CLK
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CSCMR1_FLEXSPI_PODF_MASK)) | ((FLEXSPI_CLK_ROOT_DIVIDE - 1) << 23));
    #endif

    // Prepare FLEXSPI2_CLK_ROOT
    //
    #if (defined iMX_RT1064 || defined iMX_RT1064) && defined FLEXSPI2_CLK_FROM_PLL2
    CCM_CBCMR = (((CCM_CBCMR & ~(CCM_CBCMR_FLEXSPI2_CLK_SEL_MASK | CCM_CBCMR_FLEXSPI2_PODF_DIV8))) | (CCM_CBCMR_FLEXSPI2_CLK_SEL_PLL2 | ((FLEXSPI2_CLK_ROOT_DIVIDE - 1) << 29)));
    #endif

    // Prepare LPSPI_CLK_ROOT
    //
    #if defined LPSPI_CLK_FROM_PLL3_PDF0
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CBCMR_LPSPI_PODF_MASK)) | CCM_CBCMR_LPSPI_CLK_SEL_PLL3_PFD0 | ((LPSPI_CLK_ROOT_DIVIDE - 1) << 26));
    #elif defined LPSPI_CLK_FROM_PLL3_PDF1
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CBCMR_LPSPI_PODF_MASK)) | ((LPSPI_CLK_ROOT_DIVIDE - 1) << 26));
    #elif defined LPSPI_CLK_FROM_PLL2_PDF2
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0 | CCM_CBCMR_LPSPI_PODF_MASK)) | CCM_CBCMR_LPSPI_CLK_SEL_PLL2_PFD2 | ((LPSPI_CLK_ROOT_DIVIDE - 1) << 26));
    #else                                                                // LPSPI_CLK_FROM_PLL2
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CBCMR_LPSPI_CLK_SEL_MASK | CCM_CBCMR_LPSPI_PODF_MASK)) | CCM_CBCMR_LPSPI_CLK_SEL_PLL2 | ((LPSPI_CLK_ROOT_DIVIDE - 1) << 26));
    #endif

    // Prepare PERCLK_CLK_ROOT
    //
    #if defined PERCLK_CLK_ROOT_SOURCE_IPG_CLK                           // PERCLK (feeds PIT, GPT) derived from IPG clock with optional pre-scaler
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_PERCLK_PODF_MASK | CCM_CSCMR1_PERCLK_SEL_OSC_CLK)) | (PERCLK_CLK_ROOT_DIVIDE - 1));
    #else                                                                // else PERCLK is derived from OSC_CLK with optional pre-scaler
    CCM_CSCMR1 = (((CCM_CSCMR1 & ~(CCM_CSCMR1_PERCLK_PODF_MASK)) | (CCM_CSCMR1_PERCLK_SEL_OSC_CLK | (PERCLK_CLK_ROOT_DIVIDE - 1)));
    #endif
#endif

#if defined _ENABLE_PLL2                                                 // enable PLL2 to generate fixed 528MHz
    {
        unsigned long ulDisableOutputs = (0
    #if !defined PLL2_PFD3_USED
            | CCM_ANALOG_PFD_528_PFD3_CLKGATE
    #endif
    #if !defined PLL2_PFD2_USED
            | CCM_ANALOG_PFD_528_PFD2_CLKGATE
    #endif
    #if !defined PLL2_PFD1_USED
            | CCM_ANALOG_PFD_528_PFD1_CLKGATE
    #endif
    #if !defined PLL2_PFD0_USED
            | CCM_ANALOG_PFD_528_PFD0_CLKGATE
    #endif
            );
        _SET_REGISTER(CCM_ANALOG_PFD_528, ulDisableOutputs);             // disable PFD outputs that are not used
    }
    CCM_ANALOG_PLL_SYS = (CCM_ANALOG_PLL_SYS_POWERUP | CCM_ANALOG_PLL_SYS_ENABLE | CCM_ANALOG_PLL_SYS_BYPASS | CCM_ANALOG_PLL_SYS_DIV_SELECT); // power up the PLL, enable its output but don't yet remove its bypass
    #if defined PLL2_FREQUENCY && (PLL2_FREQUENCY != 528000000)          // optionally set PLL2's frequency (range 528MNz..552MHz)
    CCM_ANALOG_PLL_SYS_NUM = (PLL2_FREQUENCY - 528000000);
    CCM_ANALOG_PLL_SYS_DENOM = _EXTERNAL_CLOCK;
    #else
        #if defined PLL2_SPREAD_SPECTRUM
            #if PLL2_SPREAD_SPECTRUM_RANGE > 0xffff
                #error "Spread spectrum range is limited to 65534Hz"
            #endif
            #if PLL2_SPREAD_SPECTRUM_MOD_FREQ > 0x7fff
                #error "Spread spectrum modulation is limited to 32767Hz"
            #endif
    CCM_ANALOG_PLL_SYS_NUM = _EXTERNAL_CLOCK;                            // ensure the start frequency (528MHz) is used
    CCM_ANALOG_PLL_SYS_DENOM = _EXTERNAL_CLOCK;                          // ensure the start frequency (528MHz) is used
    CCM_ANALOG_PLL_SYS_SS = ((PLL2_SPREAD_SPECTRUM_RANGE << 16) | PLL2_SPREAD_SPECTRUM_MOD_FREQ | CCM_ANALOG_PLL_SYS_SS_ENABLE); // configure and enable spread spectrum modulation and frequency
        #else
    CCM_ANALOG_PLL_SYS_NUM = 0;                                          // ensure the fixed frequency (528MHz) is used
        #endif
    #endif
#endif

#if defined _ENABLE_PLL3                                                 // enable USB1 PLL (PLL3) to generate fixed 480MHz
    _SET_REGISTER(CCM_ANALOG_PFD_480, (CCM_ANALOG_PFD_480_PFD3_CLKGATE | CCM_ANALOG_PFD_480_PFD2_CLKGATE | CCM_ANALOG_PFD_480_PFD1_CLKGATE | CCM_ANALOG_PFD_480_PFD0_CLKGATE)); // disable PFD outputs
    CCM_ANALOG_PLL_USB1 = (CCM_ANALOG_PLL_USB1_POWER | CCM_ANALOG_PLL_USB1_ENABLE | CCM_ANALOG_PLL_USB1_BYPASS); // power up the PLL, enable its output but don't yet remove its bypass
#endif
#if defined _ENABLE_PLL7                                                 // enable USB2 PLL (PLL7) to generate fixed 480MHz for PHY2
    CCM_ANALOG_PLL_USB2 = (CCM_ANALOG_PLL_USB2_POWER | CCM_ANALOG_PLL_USB2_ENABLE | CCM_ANALOG_PLL_USB2_BYPASS); // power up the PLL, enable its output but don't yet remove its bypass
#endif

#if !defined iMX_BOOTLOADER
    // Prepare LPUART clock
    //
    #if defined UART_CLK_ROOT_FROM_PLL3_SW_CLK_6 && defined SERIAL_INTERFACE
    CCM_CSCDR1 = ((CCM_CSCDR1 & ~(CCM_CSCDR1_UART_CLK_PODF | CCM_CSCDR1_UART_CLK_SEL_OSC_CLK)) | (UART_CLK_ROOT_DIVIDER - 1)); // select the LPUART source clock divider from pll3_sw_clk/6
    #else
    CCM_CSCDR1 = ((CCM_CSCDR1 & ~(CCM_CSCDR1_UART_CLK_PODF)) | (UART_CLK_ROOT_DIVIDER - 1) | CCM_CSCDR1_UART_CLK_SEL_OSC_CLK); // select the LPUART source clock divider from OSC_CLK
    #endif

    // Prepare LPI2C clock
    //
    #if defined LPI2C_CLK_ROOT_FROM_PLL3_SW_CLK_8 && defined I2C_INTERFACE
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LPI2C_CLK_PODF | CCM_CSCDR2_LPI2C_CLK_SEL_OSC_CLK)) | ((LPI2C_CLK_ROOT_DIVIDER - 1) << 19)); // select the LPI2C source clock divider from pll3_sw_clk/8
    #else
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LPI2C_CLK_PODF)) | ((LPI2C_CLK_ROOT_DIVIDER - 1) << 19) | CCM_CSCDR2_LPI2C_CLK_SEL_OSC_CLK); // select the LPUART source clock divider from OSC_CLK
    #endif

    #if NUMBER_OF_CAN_INTERFACES > 0
    // Prepare FLEXCAN clock
    //
        #if defined CAN_CLK_ROOT_FROM_PLL3_SW_CLK_6 && defined CAN_INTERFACE
    CCM_CSCMR2 = ((CCM_CSCMR2 & ~(CCM_CSCMR2_CAN_CLK_PODF | CCM_CSCMR2_CAN_CLK_SEL_MASK)) | ((CAN_CLK_ROOT_DIVIDER - 1) << 2) | CCM_CSCMR2_CAN_CLK_SEL_PLL3_SW_CLK_6); // select the FLEXCAN source clock divider from pll3_sw_clk/6
        #elif defined CAN_CLK_ROOT_FROM_PLL3_SW_CLK_8 && defined CAN_INTERFACE
    CCM_CSCMR2 = ((CCM_CSCMR2 & ~(CCM_CSCMR2_CAN_CLK_PODF | CCM_CSCMR2_CAN_CLK_SEL_MASK)) | ((CAN_CLK_ROOT_DIVIDER - 1) << 2)); // select the FLEXCAN source clock divider from pll3_sw_clk/8
        #else
    CCM_CSCMR2 = ((CCM_CSCMR2 & ~(CCM_CSCMR2_CAN_CLK_PODF | CCM_CSCMR2_CAN_CLK_SEL_MASK)) | ((CAN_CLK_ROOT_DIVIDER - 1) << 2) | CCM_CSCMR2_CAN_CLK_SEL_OSC_CLK); // select the FLEXCAN source clock divider from OSC_CLK
        #endif
    #endif
    #if defined ENHANCED_LCD_INTERFACE && defined SUPPORT_GLCD
    // Prepare LCD clock
    //
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CBCMR_LCDIF_PODF_MASK)) | ((LCDIF_CLK_SOURCE_DIVIDE - 1) << 23)); // set post divider
        #if defined LCDIF_CLK_SOURCE_PLL3_PFD3
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_PLL3_PFD3 | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #elif defined LCDIF_CLK_SOURCE_PLL5                              // video PLL
    _CLEAR_REGISTER(CCM_ANALOG_PLL_VIDEO, (CCM_ANALOG_PLL_VIDEO_ENABLE | CCM_ANALOG_PLL_VIDEO_BYPASS_CLKSRC)); // disable video PLL output
    _SET_REGISTER(CCM_ANALOG_PLL_VIDEO, (CCM_ANALOG_PLL_VIDEO_BYPASS_CLKSRC_REF_CLK_24M | CCM_ANALOG_PLL_VIDEO_BYPASS)); // bypass the PLL during configuration
    // Set PLL5 loop divider and post divider
    //
    CCM_ANALOG_PLL_VIDEO_NUM = VIDEO_PLL_LOOP_NUMERATOR;                 // set the numerator
    CCM_ANALOG_PLL_VIDEO_DENOM = VIDEO_PLL_LOOP_DENOMINATOR;             // set the denominator
    _CLEAR_REGISTER(CCM_ANALOG_MISC2, CCM_ANALOG_MISC2_VIDEO_DIV_MASK);
    _SET_REGISTER(CCM_ANALOG_MISC2, CCM_ANALOG_MISC2_VIDEO_DIV_VALUE);   // set pll5 post divider 2
    CCM_ANALOG_PLL_VIDEO = ((CCM_ANALOG_PLL_VIDEO & ~(CCM_ANALOG_PLL_VIDEO_POWERDOWN | CCM_ANALOG_PLL_VIDEO_DIV_SELECT_MASK)) | (CCM_ANALOG_PLL_VIDEO_ENABLE | CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT_2_VALUE | VIDEO_PLL_LOOP_DIVIDER)); // set pll5 loop divider and post divider 1
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_VIDEO, CCM_ANALOG_PLL_VIDEO_LOCK); // wait for the video PLL to lock
    _CLEAR_REGISTER(CCM_ANALOG_PLL_VIDEO, CCM_ANALOG_PLL_VIDEO_BYPASS); // disable the bypass so that pll5 signal is output

    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_PLL5 | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #elif defined LCDIF_CLK_SOURCE_PLL2_PFD0
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_PLL2_PFD0 | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #elif defined LCDIF_CLK_SOURCE_PLL2_PFD1
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_PLL2_PFD2 | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #elif defined LCDIF_CLK_SOURCE_PLL3_PFD1
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_PLL3_PFD1 | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #else                                                            // LCDIF_CLK_SOURCE_PLL2
    CCM_CSCDR2 = ((CCM_CSCDR2 & ~(CCM_CSCDR2_LCDIF_PRED_MASK | CCM_CSCDR2_LCDIF_PRE_CLK_SEL_MASK)) | ((LDCIF_CLK_PRESCALER - 1) << 12)); // set pre-scaler and source
        #endif
    #endif
#endif
#if defined _ENABLE_PLL6                                                 // enable PLL6 to generate fixed 500MHz since it will be required for main clock and/or ethernet use
    CCM_ANALOG_PLL_ENET = (CCM_ANALOG_PLL_ENET_NOT_POWEREDDOWN | CCM_ANALOG_PLL_ENET_ENABLE | CCM_ANALOG_PLL_ENET_BYPASS | CCM_ANALOG_PLL_ENET_500M_REF_EN | CCM_ANALOG_PLL_ENET_DIV_SELECT_25M); // power up the PLL, enable its output but don't yet remove its bypass
#endif
#if defined _ENABLE_PLL1
    CCM_ANALOG_PLL_ARM = (CCM_ANALOG_PLL_ARM_NOT_POWERDOWN | CCM_ANALOG_PLL_ARM_ENABLE | CCM_ANALOG_PLL_ARM_BYPASS | ARM_PLL_MULTIPLIER); // power up the PLL, enable its output but don't yet remove its bypass
#endif

#if defined RUN_FROM_DEFAULT_CLOCK
    CCM_ANALOG_PLL_SYS = (CCM_ANALOG_PLL_SYS_BYPASS | CCM_ANALOG_PLL_SYS_ENABLE | CCM_ANALOG_PLL_SYS_POWERDOWN | CCM_ANALOG_PLL_SYS_DIV_SELECT | CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_REF_CLK_24M); // bypass the PLL so that the system is clocked from the 24MHz oscillator

    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_IPG_PODF_MASK | CCM_CBCDR_AHB_PODF_MASK)) | ((IPG_CLK_ROOT_DIVIDE - 1) << 8) | ((AHB_CLK_ROOT_DIVIDE - 1) << 10)); // AHB (core clock) and IPG divider (ADC - XBAR from ARM core clock)
    _WAIT_REGISTER_TRUE(CCM_CDHIPR, CCM_CDHIPR_AHB_PODF_BUSY);           // wait until the divider handshake process has completed
#else
    #if CORE_CLOCK > 500000000
    DCDC_REG3 = ((DCDC_REG3 & ~DCDC_REG3_TRG_MASK) | DCDC_REG3_TRG_1_275V); // increase the DCDC voltage from default 1.15V to 1.275V
    #else
    DCDC_REG3 = ((DCDC_REG3 & ~DCDC_REG3_TRG_MASK) | DCDC_REG3_TRG_1_250V); // increase the DCDC voltage from default 1.15V to 1.25V
    #endif
    _WAIT_REGISTER_FALSE(DCDC_REG0, DCDC_REG0_STS_DC_OK);                // wait until the DCDC output stabilises
    // Prepare the AHB and IPG pre-scalers to ensure the frequencies are in range when the source is switched to - beware that this cannot be performed when using on-the-fly decryption!!
    //
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_IPG_PODF_MASK | CCM_CBCDR_AHB_PODF_MASK)) | ((IPG_CLK_ROOT_DIVIDE - 1) << 8) | ((AHB_CLK_ROOT_DIVIDE - 1) << 10)); // AHB (core clock) and IPG divider (ADC - XBAR from ARM core clock)
    _WAIT_REGISTER_TRUE(CCM_CDHIPR, CCM_CDHIPR_AHB_PODF_BUSY);           // wait until the divider handshake process has completed

    #if defined SDCARD_SUPPORT
    // Select USDHC1/2_CLK_ROOT source and speeds
    //
    CCM_CSCDR1 = ((CCM_CSCDR1 & ~(CCM_CSCDR1_USDHC1_PODF_8 | CCM_CSCDR1_USDHC2_PODF_8)) | (((USDHC1_CLK_ROOT_DIVIDE - 1) << 11) | ((USDHC2_CLK_ROOT_DIVIDE - 1) << 16)));  // configure USDHC1 and USDHC2 pre-scalers
        #if defined USDHC1_CLK_SOURCE_PLL2_PFD0
    CCM_CSCMR1 |= (CCM_CSCMR1_USDHC1_CLK_SEL_PLL2_PFD0);                 // USDHC1 selected from PLL2-PFD0
    CCM_ANALOG_PFD_528 = ((CCM_ANALOG_PFD_528 & ~(CCM_ANALOG_PFD_528_PFD0_CLKGATE | CCM_ANALOG_PFD_528_PFD0_FRACT_MASK)) | (PLL2_PFD0_FRACTION)); // enable PLL2 PFD0 output and ensure the frequency is as required
        #else
    CCM_CSCMR1 &= ~(CCM_CSCMR1_USDHC1_CLK_SEL_PLL2_PFD0);                // USDHC1 selected from PLL2-PFD2
    CCM_ANALOG_PFD_528 = ((CCM_ANALOG_PFD_528 & ~(CCM_ANALOG_PFD_528_PFD2_CLKGATE | CCM_ANALOG_PFD_528_PFD2_FRACT_MASK)) | (PLL2_PFD2_FRACTION << 16)); // enable PLL2 PFD2 output and ensure the frequency is as required
        #endif
        #if defined USDHC2_CLK_SOURCE_PLL2_PFD0
    CCM_CSCMR1 |= (CCM_CSCMR1_USDHC2_CLK_SEL_PLL2_PFD0);                 // USDHC2 selected from PLL2-PFD0
    CCM_ANALOG_PFD_528 = ((CCM_ANALOG_PFD_528 & ~(CCM_ANALOG_PFD_528_PFD0_CLKGATE | CCM_ANALOG_PFD_528_PFD0_FRACT_MASK)) | (PLL2_PFD0_FRACTION)); // enable PLL2 PFD0 output and ensure the frequency is as required
        #else
    CCM_CSCMR1 &= ~(CCM_CSCMR1_USDHC2_CLK_SEL_PLL2_PFD0);                // USDHC1 selected from PLL2-PFD2
    CCM_ANALOG_PFD_528 = ((CCM_ANALOG_PFD_528 & ~(CCM_ANALOG_PFD_528_PFD2_CLKGATE | CCM_ANALOG_PFD_528_PFD2_FRACT_MASK)) | (PLL2_PFD2_FRACTION << 16)); // enable PLL2 PFD2 output and ensure the frequency is as required
        #endif
    #endif

    // Select PERIPH_CLK source and speed
    // - PERIPH_CLK supplies the ARM core clock [AHB_CLK_ROOT] and IP_CLK_ROOT, with optional dividers
    //
    #if defined PERIPH_CLK_SOURCE_PLL2_PFD3
        #if !defined PLL2_CLOCK_OSC_CLK
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_LOCK);   // wait for PLL2 to lock at 528MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_BYPASS);      // clear the bypass
        #endif
    CCM_ANALOG_PFD_528 = ((CCM_ANALOG_PFD_528 & ~(CCM_ANALOG_PFD_528_PFD3_FRACT_MASK | CCM_ANALOG_PFD_528_PFD3_CLKGATE)) | (PLL2_PFD3_FRACTION << 24)); // configure PLL-PFD3 frequency
    CCM_CBCMR = ((CCM_CBCMR &~CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL6) | CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL2_PFD3); // select PLL2-PFD3 as source
    CCM_CBCDR &= ~(CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
    #define RELEASE_PLL3
    #define RELEASE_PLL7
    #elif defined PERIPH_CLK_SOURCE_PLL3_PFD3
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_LOCK); // wait for PLL3 to lock at 480MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_BYPASS);    // clear the bypass
    CCM_ANALOG_PFD_480 = ((CCM_ANALOG_PFD_480 & ~(CCM_ANALOG_PFD_480_PFD3_FRACT_MASK | CCM_ANALOG_PFD_480_PFD3_CLKGATE)) | (PLL3_PFD3_FRACTION << 24)); // configure PLL-PFD3 frequency
    CCM_CBCMR = ((CCM_CBCMR &~CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL6) | CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL3_PFD3); // select PLL2-PFD3 as source
    CCM_CBCDR &= ~(CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
    #define RELEASE_PLL2
    #define RELEASE_PLL7
    #elif defined PERIPH_CLK_SOURCE_PLL2_528M                            // PERIPH_CLK sourced from system PLL (528MHz)
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_LOCK);   // wait for PLL2 to lock at 528MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_BYPASS);      // clear the bypass
    CCM_CBCMR &= ~CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;                     // select PLL2 as source
    CCM_CBCDR &= ~(CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
    #define RELEASE_PLL3
    #define RELEASE_PLL7
    #elif defined PERIPH_CLK_SOURCE_ARM_PLL                              // PERIPH_CLK sourced from ARM PLL (PLL1)
    CCM_CACRR = (PERIPH_CLK_SOURCE_ARM_PLL_DIVIDE - 1);                  // ARM clock root divider
        #if !defined PLL1_CLOCK_OSC_CLK
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_ARM, CCM_ANALOG_PLL_ARM_LOCK);   // wait until the PLL has locked
    _CLEAR_REGISTER(CCM_ANALOG_PLL_ARM, CCM_ANALOG_PLL_ARM_BYPASS);      // disable bypass
        #endif
    CCM_CBCMR |= CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL1;                      // select divided PLL1 as source
    CCM_CBCDR &= ~(CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
    #define RELEASE_PLL2
    #define RELEASE_PLL3
    #define RELEASE_PLL7
    #elif defined PERIPH_CLK_SOURCE_PLL6_500M                            // PERIPH_CLK sourced from 500MHz PLL6
        #if defined CCM_CACRR
    CCM_CACRR = (PERIPH_CLK_SOURCE_PLL6_DIVIDE - 1);                     // ARM clock root divider
    _WAIT_REGISTER_TRUE(CCM_CDHIPR, CCM_CDHIPR_ARM_PODF_BUSY);           // wait until the divider handshake process has completed
        #endif
        #if !defined PLL6_CLOCK_OSC_CLK
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_ENET, CCM_ANALOG_PLL_ENET_LOCK); // wait for PLL6 to lock at 500MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_ENET, CCM_ANALOG_PLL_ENET_BYPASS);    // clear the bypass
        #endif
    CCM_CBCMR |= CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL6;                      // select divided PLL6 as source
    CCM_CBCDR &= ~(CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
    #define RELEASE_PLL2
    #define RELEASE_PLL3
    #define RELEASE_PLL7
    #elif defined PERIPH_CLK_SOURCE_PLL3_SW_CLK                          // PERIPH_CLK sourced from PLL3 - either OSC_CLK when bypassed or 480MHz fixed frequency
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_LOCK); // wait for PLL3 to lock at 480MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_BYPASS);    // clear the bypass so that the output frequency is supplied to following consumers
        #if defined CCM_CBCDR_PERIPH_CLK2_PODF_MASK
    CCM_CBCDR = ((CCM_CBCDR & ~(CCM_CBCDR_PERIPH_CLK2_PODF_MASK)) | ((PERIPH_CLK_SOURCE_PLL_SW_DIVIDE - 1) << 27) | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #else
    CCM_CBCDR = (CCM_CBCDR | CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK);
        #endif
    _WAIT_REGISTER_TRUE(CCM_CDHIPR, CCM_CDHIPR_PERIPH2_CLK_SEL_BUSY);    // wait until the mux handshake process has completed
    CCM_CBCMR = ((CCM_CBCMR & ~(CCM_CBCMR_PERIPH_CLK2_SEL_MASK)) | CCM_CBCMR_PERIPH_CLK2_SEL_PLL3_SW_CLK);
    #define RELEASE_PLL2
    #define RELEASE_PLL7
    #endif

    #if defined _ENABLE_PLL2 && defined RELEASE_PLL2                     // if PLL2 required
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_LOCK);   // wait for PLL2 to lock at 528MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_SYS, CCM_ANALOG_PLL_SYS_BYPASS);      // clear the bypass so that the output frequency is supplied to following consumers
    #endif
    #if defined _ENABLE_PLL3 && defined RELEASE_PLL3
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_LOCK); // wait for PLL3 to lock at 480MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_BYPASS);    // clear the bypass
    #endif
    #if defined _ENABLE_PLL7 && defined RELEASE_PLL7
    _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_USB2, CCM_ANALOG_PLL_USB2_LOCK); // wait for PLL7 to lock at 480MHz
    _CLEAR_REGISTER(CCM_ANALOG_PLL_USB2, CCM_ANALOG_PLL_USB2_BYPASS);    // clear the bypass
    #endif

    // Select PERCLK source and speed
    // - PERCLK_CLK_ROOT supplies the PIT and GPT
    //
    #if defined PERCLK_CLK_ROOT_SOURCE_IPG_CLK                           // PERCLK sourced from IPG clock root
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_PERCLK_PODF_MASK)) | (PERCLK_CLK_ROOT_DIVIDE - 1));
    #else                                                                // PERCLK sourced from OSC
    CCM_CSCMR1 = ((CCM_CSCMR1 & ~(CCM_CSCMR1_PERCLK_PODF_MASK)) | CCM_CSCMR1_PERCLK_SEL_OSC_CLK | (PERCLK_CLK_ROOT_DIVIDE - 1));
    #endif
#endif
#if !defined TICK_USES_LPTMR && !defined TICK_USES_RTC
    SYSTICK_RELOAD = SYSTICK_COUNT_MASK;                                 // temporarily set maximum reload value
    SYSTICK_CURRENT = SYSTICK_COUNT_MASK;                                // write to the current value to cause the counter value to be reset to 0 and the reload value be set
    (void)SYSTICK_CSR;                                                   // ensure that the SYSTICK_COUNTFLAG flag is cleared
    SYSTICK_CSR = (SYSTICK_CORE_CLOCK | SYSTICK_ENABLE);                 // allow SYSTICK to run so that loop delays can already use it
#endif
}


static void fnDisableClocks(void)
{
    SYSTICK_CSR = 0;                                                     // disable SYSTICK which may have been used by the boot loader
    return;
    // As many clocks as possible are gated off when starting - they will later be re-enabled if and when used
    //
#if defined iMX_RT105X || defined iMX_RT106X
    //CCM_CCGR0 = (CCM_CCGR0_GPIO2_CLOCK_OFF | CCM_CCGR0_LPUART2_CLOCK_OFF | CCM_CCGR0_LPUART3_CLOCK_OFF | CCM_CCGR0_GPT2_SERIAL_CLOCKS_OFF | CCM_CCGR0_GPT2_BUS_CLOCKS_OFF | CCM_CCGR0_TRACE_CLOCK_OFF | CCM_CCGR0_CAN2_SERIAL_CLOCK_OFF | CCM_CCGR0_CAN2_CLOCK_OFF | CCM_CCGR0_CAN1_SERIAL_CLOCK_OFF | CCM_CCGR0_CAN1_CLOCK_OFF | CCM_CCGR0_DCP_CLOCK_OFF | CCM_CCGR0_SIM_M_CLK_R_CLK_OFF | CCM_CCGR0_MQS_CLOCK_OFF | CCM_CCGR0_AIPS_TZ2_CLOCK2_OFF | CCM_CCGR0_AIPS_TZ1_CLOCK2_OFF);
#else
    //CCM_CCGR0 = (CCM_CCGR0_GPIO2_CLOCK_OFF | CCM_CCGR0_LPUART2_CLOCK_OFF | CCM_CCGR0_LPUART3_CLOCK_OFF | CCM_CCGR0_FLEXSPI_EXSC_CLOCK_OFF | CCM_CCGR0_GPT2_SERIAL_CLOCKS_OFF | CCM_CCGR0_GPT2_BUS_CLOCKS_OFF | CCM_CCGR0_TRACE_CLOCK_OFF | CCM_CCGR0_CAN2_SERIAL_CLOCK_OFF | CCM_CCGR0_CAN2_CLOCK_OFF | CCM_CCGR0_CAN1_SERIAL_CLOCK_OFF | CCM_CCGR0_CAN1_CLOCK_OFF | CCM_CCGR0_DCP_CLOCK_OFF | CCM_CCGR0_SIM_M_CLK_R_CLK_OFF | CCM_CCGR0_MQS_CLOCK_OFF | CCM_CCGR0_AIPS_TZ2_CLOCK2_OFF | CCM_CCGR0_AIPS_TZ1_CLOCK2_OFF);
    CCM_CCGR0 &= ~(CCM_CCGR0_GPIO2_CLOCK_STOP | CCM_CCGR0_LPUART2_CLOCK_STOP | CCM_CCGR0_LPUART3_CLOCK_STOP | CCM_CCGR0_CAN1_CLOCK_OFF | CCM_CCGR0_CAN2_CLOCK_OFF);
#endif
#if defined iMX_RT105X || defined iMX_RT106X
    //CCM_CCGR1 = (CCM_CCGR1_AOI2_CLOCKS_OFF | CCM_CCGR1_CSU_CLOCK_OFF | CCM_CCGR1_GPIO1_CLOCK_OFF | CCM_CCGR1_LPUART4_CLOCK_OFF | CCM_CCGR1_GPT1_SERIAL_CLOCK_OFF | CCM_CCGR1_GPT1_BUS_CLOCK_OFF | CCM_CCGR1_SEMC_EXSC_CLOCK_OFF | CCM_CCGR1_ADC1_CLOCK_OFF | CCM_CCGR1_PIT_CLOCKS_OFF | CCM_CCGR1_ENET_CLOCK_OFF | CCM_CCGR1_ADC2_CLOCK_OFF | CCM_CCGR1_LPSPI4_CLOCKS_OFF | CCM_CCGR1_LPSPI3_CLOCKS_OFF | CCM_CCGR1_LPSPI2_CLOCKS_OFF | CCM_CCGR1_LPSPI1_CLOCKS_OFF);
#else
    #if defined iMX_RT1011
    POWER_UP_ATOMIC(1, GPIO1_CLOCK);                                 // power up the port (workaround - if this is not done here QSPI based code will not run)
    #endif
    //CCM_CCGR1 = (CCM_CCGR1_CSU_CLOCK_OFF | CCM_CCGR1_GPIO1_CLOCK_OFF | CCM_CCGR1_LPUART4_CLOCK_OFF | CCM_CCGR1_GPT1_SERIAL_CLOCK_OFF | CCM_CCGR1_GPT1_BUS_CLOCK_OFF | CCM_CCGR1_SEMC_EXSC_CLOCK_OFF | CCM_CCGR1_ADC1_CLOCK_OFF | CCM_CCGR1_PIT_CLOCKS_OFF | CCM_CCGR1_ENET_CLOCK_OFF | CCM_CCGR1_ADC2_CLOCK_OFF | CCM_CCGR1_LPSPI4_CLOCKS_OFF | CCM_CCGR1_LPSPI3_CLOCKS_OFF | CCM_CCGR1_LPSPI2_CLOCKS_OFF | CCM_CCGR1_LPSPI1_CLOCKS_OFF);
    CCM_CCGR1 &= ~(/*CCM_CCGR1_GPIO1_CLOCK_STOP | */CCM_CCGR1_LPUART4_CLOCK_STOP | CCM_CCGR1_PIT_CLOCKS_STOP/* | CCM_CCGR1_ENET_CLOCK_STOP*/);
#endif
    //CCM_CCGR2 = (CCM_CCGR2_GPIO3_CLOCK_OFF | CCM_CCGR2_XBAR2_CLOCK_OFF | CCM_CCGR2_XBAR1_CLOCK_OFF | CCM_CCGR2_OCOTP_CTRL_CLOCK_OFF | CCM_CCGR2_LPI2C3_CLOCK_OFF | CCM_CCGR2_LPI2C2_CLOCK_OFF | CCM_CCGR2_LPI2C1_CLOCK_OFF | CCM_CCGR2_IMUX_SNVS_CLOCK_OFF | CCM_CCGR2_OCRAM_EXSC_CLOCK_OFF);
    //CCM_CCGR2 &= ~(CCM_CCGR2_GPIO3_CLOCK_STOP);
    CCM_CCGR2 &= ~(CCM_CCGR2_LPI2C3_CLOCK_OFF | CCM_CCGR2_LPI2C2_CLOCK_OFF | CCM_CCGR2_LPI2C1_CLOCK_OFF);
#if defined iMX_RT105X || defined iMX_RT106X
    //CCM_CCGR3 = (CCM_CCGR3_GPIO4_CLOCK_OFF | CCM_CCGR3_IMUX_SNVS_GRP_CLOCK_OFF | CCM_CCGR3_OCRAM_CLOCK_OFF | CCM_CCGR3_ACMP4_CLOCKS_OFF | CCM_CCGR3_ACMP3_CLOCKS_OFF | CCM_CCGR3_ACMP2_CLOCKS_OFF | CCM_CCGR3_ACMP2_CLOCKS_STOP | CCM_CCGR3_ACMP1_CLOCKS_OFF | CCM_CCGR3_FLEXRAM1_CLOCK_OFF | CCM_CCGR3_WDOG1_CLOCK_OFF | CCM_CCGR3_EWM_CLOCK_OFF | CCM_CCGR3_LPUART6_CLOCK_OFF | CCM_CCGR3_SEMC_CLOCKS_OFF | CCM_CCGR3_LPUART5_CLOCK_OFF);
#else
    //CCM_CCGR3 = (CCM_CCGR3_IMUX_SNVS_GRP_CLOCK_OFF | CCM_CCGR3_OCRAM_CLOCK_OFF | CCM_CCGR3_ACMP4_CLOCKS_OFF | CCM_CCGR3_ACMP3_CLOCKS_OFF | CCM_CCGR3_ACMP2_CLOCKS_OFF | CCM_CCGR3_ACMP2_CLOCKS_STOP | CCM_CCGR3_ACMP1_CLOCKS_OFF | CCM_CCGR3_FLEXRAM1_CLOCK_OFF | CCM_CCGR3_WDOG1_CLOCK_OFF | CCM_CCGR3_EWM_CLOCK_OFF | CCM_CCGR3_AOI1_CLOCK_OFF | CCM_CCGR3_LPUART6_CLOCK_OFF | CCM_CCGR3_SEMC_CLOCKS_OFF | CCM_CCGR3_LPUART5_CLOCK_OFF);
    CCM_CCGR3 &= ~(CCM_CCGR3_LPUART6_CLOCK_STOP | CCM_CCGR3_LPUART5_CLOCK_STOP/* | CCM_CCGR3_IMUX_SNVS_GRP_CLOCK_STOP*/);
#endif
    //CCM_CCGR4 = (CCM_CCGR4_ENC2_CLOCKS_OFF | CCM_CCGR4_ENC1_CLOCKS_OFF | CCM_CCGR4_PWM2_CLOCKS_OFF | CCM_CCGR4_PWM1_CLOCKS_OFF | CCM_CCGR4_SIM_EMS_CLOCKS_OFF | CCM_CCGR4_SIM_M_CLOCKS_OFF | CCM_CCGR4_SIM_M7_CLOCK_OFF | CCM_CCGR4_BEE_CLOCK_OFF | CCM_CCGR4_IOMUX_GRP_CLOCK_OFF | CCM_CCGR4_IOMUX_CLOCK_OFF | CCM_CCGR4_SIM_M7_CLK_R_OFF);
    //CCM_CCGR4 &= ~(CCM_CCGR4_IOMUX_GRP_CLOCK_STOP | CCM_CCGR4_IOMUX_CLOCK_STOP);
    //CCM_CCGR5 = (CCM_CCGR5_SNVS_LP_CLOCK_OFF | CCM_CCGR5_SNVS_HP_CLOCK_OFF | CCM_CCGR5_LPUART7_CLOCK_OFF | CCM_CCGR5_LPUART1_CLOCK_OFF | CCM_CCGR5_SAI3_CLOCK_OFF | CCM_CCGR5_SAI2_CLOCK_OFF | CCM_CCGR5_SAI1_CLOCK_OFF | CCM_CCGR5_SPDIF_CLOCK_OFF | CCM_CCGR5_AIPSTZ4_CLOCKS_OFF | CCM_CCGR5_WDOG2_CLOCK_OFF | CCM_CCGR5_KPP_CLOCK_OFF | CCM_CCGR5_DMA_CLOCK_OFF/* | CCM_CCGR5_WDOG3_CLOCK_OFF*/ | CCM_CCGR5_FLEXIO1_CLOCK_OFF | CCM_CCGR5_ROM_CLOCK_OFF);
    CCM_CCGR5 &= ~(CCM_CCGR5_LPUART7_CLOCK_STOP | CCM_CCGR5_LPUART1_CLOCK_STOP);
    //CCM_CCGR6 = (CCM_CCGR6_TIMER2_CLOCKS_OFF | CCM_CCGR6_TIMER1_CLOCKS_OFF | CCM_CCGR6_LPI2C4_SERIAL_CLOCK_OFF | CCM_CCGR6_ANADIG_CLOCKS_OFF | CCM_CCGR6_SIM_PER_CLOCK_OFF | CCM_CCGR6_AIPS_TZ3_CLOCK_OFF | CCM_CCGR6_LPUART8_CLOCK_OFF | CCM_CCGR6_TRNG_CLOCK_OFF);
    CCM_CCGR6 &= ~(CCM_CCGR6_LPUART8_CLOCK_STOP | CCM_CCGR6_LPI2C4_SERIAL_CLOCK_STOP | CCM_CCGR6_USDHC1_CLOCKS_STOP | CCM_CCGR6_USDHC2_CLOCKS_STOP);
}
