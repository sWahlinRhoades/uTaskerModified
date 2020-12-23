/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_SDHC.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    27.07.2013 Ensure that SDHC read is not performed using a command that can't save to misaligned memory {47}
    18.12.2013 Add monitoring of SDHC transmission buffer and break on stall {62}
    18.12.2013 Reset SDHC flags at re-initialisation to ensure no old error flags remain {63}
    02.03.2018 Add block read support and optimised sector read making use of FIFO and optional DMA {64}
    09.03.2020 Share Ethernet driver with iMX project

*/


extern void fnSetSD_clock(unsigned long ulSpeed)
{
#if defined _iMX
    SDHC_SYSCTL = (SDHC_SYSCTL_RESERVED | SDHC_SYSCTL_IPP_RST_N | SDHC_SYSCTL_DTOCV_SDCLK_29 | ulSpeed); // set new speed
#else
    SDHC_SYSCTL &= ~SDHC_SYSCTL_SDCLKEN;                                 // stop clock to set frequency
    #if defined CLOCK_SDHC_FROM_OSCERCLK
    OSC0_CR |= (OSC_CR_ERCLKEN);                                         // ensure that the OSCERCLK is enabled (it is assumed that there is a external clock applied or the oscillator is operating)
    SIM_SOPT2 &= ~(SIM_SOPT2_SDHCSRC_EXT_BYPASS);
    SIM_SOPT2 |= (SIM_SOPT2_SDHCSRC_OSCERCLK);                           // select the OSCERCLK input for the SDHC controller
    #elif defined CLOCK_SDHC_FROM_IRC48M
    POWER_UP_ATOMIC(4, USBOTG);                                          // power up the USB controller module
    USB_CLK_RECOVER_IRC_EN = (USB_CLK_RECOVER_IRC_EN_REG_EN | USB_CLK_RECOVER_IRC_EN_IRC_EN); // the IRC48M is only usable when enabled via the USB module
    SIM_SOPT2 &= ~(SIM_SOPT2_SDHCSRC_EXT_BYPASS);
    SIM_SOPT2 |= (SIM_SOPT2_SDHCSRC_MCG_IRC48M | SIM_SOPT2_PLLFLLSEL_IRC48M); // select the IRC48M input for the SDHC controller
    #endif
    SDHC_SYSCTL = (SDHC_SYSCTL_DTOCV_227 | ulSpeed);                     // set new speed
#endif
    _WAIT_REGISTER_FALSE(SDHC_PRSSTAT, SDHC_PRSSTAT_SDSTB);              // wait for new speed to stabilise
#if !defined _iMX
    fnDelayLoop(50);                                                     // the SDHC_PRSSTAT_SDSTB doesn't seem to behave correctly so ensure with a delay
    SDHC_SYSCTL = (SDHC_SYSCTL_SDCLKEN | SDHC_SYSCTL_DTOCV_227 | ulSpeed); // enable clock
#endif
}


// Initialise the SD controllers interface with low speed clock during initialiation sequence
//
#if defined KINETIS_K65 || defined KINETIS_K66
    #define READ_WATERMARK  128
    #define WRITE_WATERMARK 128
#else
    #define READ_WATERMARK  1
    #define WRITE_WATERMARK 1
#endif

#if defined SDCARD_DETECT_INPUT_INTERRUPT && defined _iMX
static void (*sdcard_handler[NUMBER_OF_SDHC])(void) = {0};
// Interrupt call-back on change in SD-card presence status
//
static void sdcard_state_change(void)
{
    WRITE_ONE_TO_CLEAR(SDHC_IRQSTAT, (SDHC_IRQSTAT_CINS | SDHC_IRQSTAT_CRM)); // clear interrupt flag(s)
    if (sdcard_handler[0] != 0) {
        uDisable_Interrupt();
            sdcard_handler[0]();
        uEnable_Interrupt();
    }
}
#endif

extern void fnInitSDCardInterface(void (*int_handler)(void))
{
#if defined _iMX
    #if defined USE_uSDHC2
    POWER_UP_ATOMIC(6, USDHC2_CLOCKS);                                   // power up the SDHC2 controller
    #else
    POWER_UP_ATOMIC(6, USDHC1_CLOCKS);                                   // power up the SDHC1 controller
    #endif

    POWER_TO_SD_CARD();                                                  // apply power to the SD card itself

    // Configure the pins for the hardware
    //
    #if defined iMX_RT105X || defined iMX_RT106X
        #if defined USE_uSDHC2
            #if defined uSDHC2_ON_SD                                     // uSDHC2 connected on GPIO_SD_B1_XX connection set
    IOMUXC_USDHC2_CMD_SELECT_INPUT = IOMUXC_USDHC2_CMD_SELECT_INPUT_GPIO_SD_B1_05_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_05, USDHC2_CMD, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_CMD on GPIO3-15 - alt function 0
    IOMUXC_USDHC2_CLK_SELECT_INPUT = IOMUXC_USDHC2_CLK_SELECT_INPUT_GPIO_SD_B1_04_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_04, USDHC2_CLK, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_CLK on GPIO3-04 - alt function 0
    IOMUXC_USDHC2_DATA0_SELECT_INPUT = IOMUXC_USDHC2_DATA0_SELECT_INPUT_GPIO_SD_B1_03_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_03, USDHC2_DATA0, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA0 on GPIO3-03 - alt function 0
    IOMUXC_USDHC2_DATA1_SELECT_INPUT = IOMUXC_USDHC2_DATA1_SELECT_INPUT_GPIO_SD_B1_02_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_02, USDHC2_DATA1, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA1 on GPIO3-02 - alt function 0
    IOMUXC_USDHC2_DATA2_SELECT_INPUT = IOMUXC_USDHC2_DATA2_SELECT_INPUT_GPIO_SD_B1_01_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_01, USDHC2_DATA2, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA2 on GPIO3-01 - alt function 0
    IOMUXC_USDHC2_DATA3_SELECT_INPUT = IOMUXC_USDHC2_DATA3_SELECT_INPUT_GPIO_SD_B1_00_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_00, USDHC2_DATA3, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA3 on GPIO3-00 - alt function 0
                #if defined SDCARD_DETECT_INPUT_INTERRUPT || defined SDCARD_DETECT_INPUT_POLL
    IOMUXC_USDHC2_CD_B_SELECT_INPUT = IOMUXC_USDHC2_CD_B_SELECT_INPUT_GPIO_EMC_39_ALT6;
    _CONFIG_PERIPHERAL(GPIO_EMC_39, USDHC2_CD_B, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_LOW)); // select USDHC2_CD_B on GPIO3-25 - alt function 6
                #endif
                #if defined uSDHC2_8_BIT_BUS
    IOMUXC_USDHC2_DATA4_SELECT_INPUT = IOMUXC_USDHC2_DATA4_SELECT_INPUT_GPIO_SD_B1_08_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_08, USDHC2_DATA4, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA0 on GPIO3-08 - alt function 0
    IOMUXC_USDHC2_DATA5_SELECT_INPUT = IOMUXC_USDHC2_DATA5_SELECT_INPUT_GPIO_SD_B1_09_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_09, USDHC2_DATA5, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA1 on GPIO3-09 - alt function 0
    IOMUXC_USDHC2_DATA6_SELECT_INPUT = IOMUXC_USDHC2_DATA6_SELECT_INPUT_GPIO_SD_B1_10_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_10, USDHC2_DATA6, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA2 on GPIO3-10 - alt function 0
    IOMUXC_USDHC2_DATA7_SELECT_INPUT = IOMUXC_USDHC2_DATA7_SELECT_INPUT_GPIO_SD_B1_11_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B1_11, USDHC2_DATA7, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA3 on GPIO3-11 - alt function 0
                #endif
            #else                                                        // uSDHC2 connected on GPIO_AD_B1_XX connection set
    IOMUXC_USDHC2_CMD_SELECT_INPUT = IOMUXC_USDHC2_CMD_SELECT_INPUT_GPIO_AD_B1_08_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_08, USDHC2_CMD, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_CMD on GPIO1-24 - alt function 6
    IOMUXC_USDHC2_CLK_SELECT_INPUT = IOMUXC_USDHC2_CLK_SELECT_INPUT_GPIO_AD_B1_09_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_09, USDHC2_CLK, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_CLK on GPIO1-25 - alt function 6
    IOMUXC_USDHC2_DATA0_SELECT_INPUT = IOMUXC_USDHC2_DATA0_SELECT_INPUT_GPIO_AD_B1_04_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_04, USDHC2_DATA0, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA0 on GPIO1-20 - alt function 6
    IOMUXC_USDHC2_DATA1_SELECT_INPUT = IOMUXC_USDHC2_DATA1_SELECT_INPUT_GPIO_AD_B1_05_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_05, USDHC2_DATA1, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA1 on GPIO1-21 - alt function 6
    IOMUXC_USDHC2_DATA2_SELECT_INPUT = IOMUXC_USDHC2_DATA2_SELECT_INPUT_GPIO_AD_B1_06_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_06, USDHC2_DATA2, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA2 on GPIO1-22 - alt function 6
    IOMUXC_USDHC2_DATA3_SELECT_INPUT = IOMUXC_USDHC2_DATA3_SELECT_INPUT_GPIO_AD_B1_07_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_07, USDHC2_DATA3, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA3 on GPIO1-23 - alt function 6
                #if defined SDCARD_DETECT_INPUT_INTERRUPT || defined SDCARD_DETECT_INPUT_POLL
    IOMUXC_USDHC2_CD_B_SELECT_INPUT = IOMUXC_USDHC2_CD_B_SELECT_INPUT_GPIO_AD_B1_03_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_03, USDHC2_CD_B, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_LOW)); // select USDHC2_CD_B on GPIO1-19 - alt function 6
                #endif
                #if defined uSDHC2_8_BIT_BUS
    IOMUXC_USDHC2_DATA4_SELECT_INPUT = IOMUXC_USDHC2_DATA4_SELECT_INPUT_GPIO_AD_B1_12_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_12, USDHC2_DATA4, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA4 on GPIO1-28 - alt function 6
    IOMUXC_USDHC2_DATA5_SELECT_INPUT = IOMUXC_USDHC2_DATA5_SELECT_INPUT_GPIO_AD_B1_13_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_13, USDHC2_DATA5, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA5 on GPIO1-29 - alt function 6
    IOMUXC_USDHC2_DATA6_SELECT_INPUT = IOMUXC_USDHC2_DATA6_SELECT_INPUT_GPIO_AD_B1_14_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_14, USDHC2_DATA6, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA6 on GPIO1-30 - alt function 6
    IOMUXC_USDHC2_DATA7_SELECT_INPUT = IOMUXC_USDHC2_DATA7_SELECT_INPUT_GPIO_AD_B1_15_ALT6;
    _CONFIG_PERIPHERAL(GPIO_AD_B1_15, USDHC2_DATA7, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC2_DATA7 on GPIO1-31 - alt function 6

                #endif
            #endif
        #else
    _CONFIG_PERIPHERAL(GPIO_SD_B0_00, USDHC1_CMD, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_CMD on GPIO3-12 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_01, USDHC1_CLK, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_CLK on GPIO3-13 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_02, USDHC1_DATA0, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA0 on GPIO3-14 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_03, USDHC1_DATA1, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA1 on GPIO3-15 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_04, USDHC1_DATA2, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA2 on GPIO3-16 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_05, USDHC1_DATA3, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA3 on GPIO3-17 - alt function 0
            #if defined SDCARD_DETECT_INPUT_INTERRUPT || defined SDCARD_DETECT_INPUT_POLL
    IOMUXC_USDHC1_CD_B_SELECT_INPUT = IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_B1_12_ALT6;
    _CONFIG_PERIPHERAL(GPIO_B1_12, USDHC1_CD_B, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_LOW)); // select USDHC1_CD_B on GPIO2-28 - alt function 6
            #endif
        #endif
    #else
    _CONFIG_PERIPHERAL(GPIO_SD_B0_02, USDHC1_CMD, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_CMD on GPIO3-15 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_03, USDHC1_CLK, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_CLK on GPIO3-16 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_04, USDHC1_DATA0, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA0 on GPIO3-17 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_05, USDHC1_DATA1, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA1 on GPIO3-18 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_00, USDHC1_DATA2, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA2 on GPIO3-13 - alt function 0
    _CONFIG_PERIPHERAL(GPIO_SD_B0_01, USDHC1_DATA3, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_47k_UP | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM | IOMUXC_SW_PAD_CTL_PAD_DSE_1 | IOMUXC_SW_PAD_CTL_PAD_SRE)); // select USDHC1_DATA3 on GPIO3-14 - alt function 0
        #if defined SDCARD_DETECT_INPUT_INTERRUPT || defined SDCARD_DETECT_INPUT_POLL
    IOMUXC_USDHC1_CD_B_SELECT_INPUT = IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_SD_B0_06_ALT0;
    _CONFIG_PERIPHERAL(GPIO_SD_B0_06, USDHC1_CD_B, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_SPEED_LOW)); // select USDHC1_CD_B on GPIO3-19 - alt function 0
        #endif
    #endif
    #if (defined SDCARD_DETECT_INPUT_INTERRUPT || defined SDCARD_DETECT_INPUT_POLL) && defined _WINDOWS
    fnCheckSD_Detect();                                                  // synchronise the initial SD card detecion state
    #endif
    SDHC_SYSCTL |= SDHC_SYSCTL_RSTA;                                     // reset the SDHC controller
    _WAIT_REGISTER_TRUE(SDHC_SYSCTL, SDHC_SYSCTL_RSTA);                  // wait for the reset to complete
    SDHC_PROCTL = (SDHC_PROCTL_BURST_LEN_EN_INCR | SDHC_PROCTL_EMODE_LITTLE); // set little endian mode
    #if defined SDCARD_DETECT_INPUT_INTERRUPT
    sdcard_handler[0] = int_handler;
    #if defined USE_uSDHC2
    fnEnterInterrupt(irq_USDHC2_ID, SDCARD1_STATUS_INTERRUPT_PRIORITY, sdcard_state_change);
    #else
    fnEnterInterrupt(irq_USDHC1_ID, SDCARD1_STATUS_INTERRUPT_PRIORITY, sdcard_state_change);
    #endif
    SDHC_IRQSIGEN = (SDHC_IRQSIGEN_CRMIEN | SDHC_IRQSIGEN_CINSIEN);      // enable interrupts on card insertion and removal
    SDHC_IRQSTATEN = 0xffffffff;                                         // allow all enabled interrupts
    #endif
#else
    POWER_UP_ATOMIC(3, SDHC);                                            // power up the SDHC controller

    // Configure the pins for the hardware
    //
    #if defined SDHC_ON_PORTA
    _CONFIG_PERIPHERAL(A, 27, (PA_27_SDHC0_CMD | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_CMD on PA.27 (alt. function 4)
    _CONFIG_PERIPHERAL(A, 25, (PA_25_SDHC0_D0 | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D0 on PA.25  (alt. function 4)
    _CONFIG_PERIPHERAL(A, 24, (PA_24_SDHC0_D1 | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D1 on PA.24  (alt. function 4)
    _CONFIG_PERIPHERAL(A, 29, (PA_29_SDHC0_D2 | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D2 on PA.29  (alt. function 4)
    _CONFIG_PERIPHERAL(A, 28, (PA_28_SDHC0_D3 | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D3 on PA.28  (alt. function 4)
    _CONFIG_PERIPHERAL(A, 26, (PA_26_SDHC0_DCLK | PORT_DSE_HIGH));       // SDHC_DCLK on PA.26 (alt. function 4)
    #else
    _CONFIG_PERIPHERAL(E, 3, (PE_3_SDHC0_CMD  | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_CMD on PE.3 (alt. function 4)
    _CONFIG_PERIPHERAL(E, 1, (PE_1_SDHC0_D0   | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D0 on PE.1  (alt. function 4)
    _CONFIG_PERIPHERAL(E, 0, (PE_0_SDHC0_D1   | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D1 on PE.0  (alt. function 4)
    _CONFIG_PERIPHERAL(E, 5, (PE_5_SDHC0_D2   | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D2 on PE.5  (alt. function 4)
    _CONFIG_PERIPHERAL(E, 4, (PE_4_SDHC0_D3   | PORT_DSE_HIGH | PORT_PS_UP_ENABLE)); // SDHC_D3 on PE.4  (alt. function 4)
    _CONFIG_PERIPHERAL(E, 2, (PE_2_SDHC0_DCLK | PORT_DSE_HIGH));         // SDHC_DCLK on PE.2 (alt. function 4)
    #endif

    SDHC_VENDOR = 0;                                                     // disable external DMA request
    SDHC_WML = ((WRITE_WATERMARK << 16) | READ_WATERMARK);               // FIFO watermark setting for read and write
    SDHC_PROCTL = (SDHC_PROCTL_EMODE_LITTLE);                            // set little endian mode
#endif
    fnSetSD_clock(SDHC_SYSCTL_SPEED_SLOW);                               // set initial speed to about 390kHz (between 100k and 400k)

    SDHC_IRQSTAT = 0xffffffff;                                           // {63} clear all (error) flags
}


// Send a command to the SD-card and return a result, plus optional returned arguments
//
extern int fnSendSD_command(const unsigned char ucCommand[6], unsigned char *ucResult, unsigned char *ptrReturnData)
{
    #define MAX_POLL_COUNT   10000
    static int iCommandYieldCount = 0;
    static int iCommandState = 0;
    int iRtn = 0;

    if (iCommandYieldCount > MAX_POLL_COUNT) {                           // the present access is taking too long - quit with SD card error
    #if !defined UTFAT_DISABLE_DEBUG_OUT
        fnDebugMsg("TIMEOUT!!!\r\n");
    #endif
        iRtn = ERROR_CARD_TIMEOUT;
    }
    else {
        switch (iCommandState) {
        case 0:
            while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB) != 0) {            // wait for idle command line
    #if defined _WINDOWS
                SDHC_PRSSTAT &= ~SDHC_PRSSTAT_CIHB;
    #endif
            }
            SDHC_CMDARG = ((ucCommand[1] << 24) | (ucCommand[2] << 16) | (ucCommand[3] << 8) | ucCommand[4]); // set the argument of the command argument register
            switch (ucCommand[0]) {                                      // case on command type
            case GO_IDLE_STATE_CMD0:
    #if defined _iMX
                SDHC_MIXCTRL = (SDHC_MIXCTRL_RESERVED | SDHC_MIXCTRL_DTDSEL_WRITE);
                SDHC_XFERTYP = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_CMDTYP_NORM); // launch the command (no response)
    #else
                SDHC_XFERTYP  = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_DTDSEL_WRITE | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_RSPTYP_48 | SDHC_XFERTYP_CMDTYP_NORM); // launch the command (no response)
    #endif
    #if defined _WINDOWS
                SDHC_IRQSTAT = SDHC_IRQSTAT_CC;                          // since there is no response, just set the sent flag
    #endif
                break;
            case SEND_CSD_CMD9:
            case SEND_CID_CMD2:
    #if defined _iMX
                SDHC_MIXCTRL = (SDHC_MIXCTRL_RESERVED | SDHC_MIXCTRL_DTDSEL_WRITE);
                SDHC_XFERTYP = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_RSPTYP_126 | SDHC_XFERTYP_CMDTYP_NORM); // launch the command with long response
    #else
                SDHC_XFERTYP  = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_DTDSEL_WRITE | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_RSPTYP_126 | SDHC_XFERTYP_CMDTYP_NORM); // launch the command with long response
    #endif
    #if defined _WINDOWS
                SDHC_IRQSTAT = SDHC_IRQSTAT_CC;                          // since there is no response, just set the sent flag
    #endif
                break;
    #if defined UTFAT_MULTIPLE_BLOCK_WRITE
            case WRITE_MULTIPLE_BLOCK_CMD25:                             // multiple block write
                SDHC_DSADDR = 0;
                SDHC_BLKATTR = ((*(unsigned long *)ptrReturnData << SDHC_BLKATTR_BLKCNT_SHIFT) | 512); // defined block count with 512 byte size
                SDHC_XFERTYP  = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_AC12EN | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_CMDTYP_NORM | SDHC_XFERTYP_BCEN | SDHC_XFERTYP_DTDSEL_WRITE | SDHC_XFERTYP_MSBSEL | SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_RSPTYP_48); // launch the command with a write
        #if defined _WINDOWS
                SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);
        #endif
                break;
    #endif
            case WRITE_BLOCK_CMD24:                                      // single sector write
    #if READ_WATERMARK != 1 && WRITE_WATERMARK == 1
                SDHC_WML = ((WRITE_WATERMARK << 16) | 1);                // set read FIFO watermark setting to 1 during write otherwise it stops the buffer write enable flag from being set
    #endif
                SDHC_DSADDR = 0;
                SDHC_BLKATTR = ((1 << SDHC_BLKATTR_BLKCNT_SHIFT) | 512); // 1 block with 512 byte size
    #if defined _iMX
                SDHC_MIXCTRL = (SDHC_MIXCTRL_RESERVED | SDHC_MIXCTRL_DTDSEL_WRITE/* | SDHC_MIXCTRL_AC12EN | SDHC_MIXCTRL_BCEN | SDHC_MIXCTRL_MSBSEL*/);
                SDHC_XFERTYP = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CMDTYP_NORM | SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_RSPTYP_48); // launch the command with a write
    #else
                SDHC_XFERTYP  = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_AC12EN | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_CMDTYP_NORM | SDHC_XFERTYP_BCEN | SDHC_XFERTYP_DTDSEL_WRITE | SDHC_XFERTYP_MSBSEL | SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_RSPTYP_48); // launch the command with a write
    #endif
    #if defined _WINDOWS
                SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);
    #endif
                break;
            case READ_SINGLE_BLOCK_CMD17:                                // single sector read
    #if defined _iMX
                SDHC_MIXCTRL = (SDHC_MIXCTRL_RESERVED | SDHC_MIXCTRL_DTDSEL_READ | SDHC_MIXCTRL_AC12EN);
    #endif
                SDHC_DSADDR = 0;                                         // reset DMA system address register 
                SDHC_BLKATTR = ((1 << SDHC_BLKATTR_BLKCNT_SHIFT) | 512); // 1 block with 512 byte size
                SDHC_XFERTYP  = ((READ_SINGLE_BLOCK_CMD17 << SDHC_XFERTYP_CMDINX_SHIFT) | // launch the command
    #if !defined _iMX
                    SDHC_XFERTYP_AC12EN |
    #endif
                    SDHC_XFERTYP_CCCEN |                                 // enable CRC
                    SDHC_XFERTYP_CICEN |                                 // enable command index check - check the CRC in the response
                    SDHC_XFERTYP_CMDTYP_NORM |                           // not a special command (suspend, resume or abort)
    #if !defined _iMX
                    SDHC_XFERTYP_DTDSEL_READ |                           // read card to host
    #endif
                    SDHC_XFERTYP_DPSEL |                                 // data line is to be used
                    SDHC_XFERTYP_RSPTYP_48);                             // response length is 48 (don't check busy)
    #if defined _WINDOWS
                SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);
    #endif
                break;
    #if defined UTFAT_MULTIPLE_BLOCK_READ                                // {64}
            case READ_MULTIPLE_BLOCK_CMD18:                              // multiple block read
                SDHC_DSADDR = 0;                                         // reset DMA system address register 
                SDHC_BLKATTR = ((*(unsigned long *)ptrReturnData << SDHC_BLKATTR_BLKCNT_SHIFT) | 512); // defined block count with 512 byte size
                SDHC_XFERTYP  = ((READ_MULTIPLE_BLOCK_CMD18 << SDHC_XFERTYP_CMDINX_SHIFT) | // launch the command
                    SDHC_XFERTYP_AC12EN |                                // automatic termination after the defined block count is read
                    SDHC_XFERTYP_CCCEN |                                 // enable CRC
                    SDHC_XFERTYP_CICEN |                                 // enable command index check - check the CRC in the response
                    SDHC_XFERTYP_CMDTYP_NORM |                           // not a special command (suspend, resume or abort)
                    SDHC_XFERTYP_BCEN |                                  // enable block count (not an infinite transfer)
                    SDHC_XFERTYP_DTDSEL_READ |                           // read card to host
                    SDHC_XFERTYP_MSBSEL |                                // multiple blocks
                    SDHC_XFERTYP_DPSEL |                                 // data line is to be used
                    SDHC_XFERTYP_RSPTYP_48);                             // response length is 48 bytes (don't check busy)
        #if defined _WINDOWS
                SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);
        #endif
                break;
    #endif
    #if defined UTFAT_MULTIPLE_BLOCK_WRITE || defined UTFAT_MULTIPLE_BLOCK_READ // {64}
            case STOP_TRANSMISSION_CMD12:                                // terminate multiple block mode
              //SDHC_PROCTL |= (SDHC_PROCTL_SABGREQ);                    // request stopping block transmission (when not all previously specified blocks have been sent)
              //while ((SDHC_IRQSTAT & SDHC_IRQSTAT_TC) == 0) {          // wait for any active transfer to complete
        #if defined _WINDOWS
              //    SDHC_IRQSTAT |= SDHC_IRQSTAT_TC;
        #endif
              //}
              //SDHC_PROCTL &= ~(SDHC_PROCTL_SABGREQ);
                SDHC_IRQSTAT = (SDHC_IRQSTAT_TC | SDHC_IRQSTAT_BWR | SDHC_IRQSTAT_AC12E); // reset flags
                // Fall through intentionally
                //
    #endif
          //case PRE_ERASE_BLOCKS_CMD23:                                 // prepare multiple block write
            default:
    #if defined _iMX
                SDHC_MIXCTRL = (SDHC_MIXCTRL_RESERVED | SDHC_MIXCTRL_DTDSEL_WRITE);
                SDHC_XFERTYP = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_RSPTYP_48 | SDHC_XFERTYP_CMDTYP_NORM); // launch the command (short response)
    #else
                SDHC_XFERTYP  = ((ucCommand[0] << SDHC_XFERTYP_CMDINX_SHIFT) | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_CICEN | SDHC_XFERTYP_DTDSEL_WRITE | SDHC_XFERTYP_RSPTYP_NONE | SDHC_XFERTYP_RSPTYP_48 | SDHC_XFERTYP_CMDTYP_NORM); // launch the command (short response)
    #endif
    #if defined _WINDOWS
                if (SEND_OP_COND_ACMD_CMD41 == ucCommand[0]) {
                    SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_CCE); // this response has no valid CRC
                }
                else {
                    SDHC_IRQSTAT = SDHC_IRQSTAT_CC;                      // set the successful response flag
                }
    #endif
                break;
            }
    #if defined _WINDOWS
            _fnSimSD_write((unsigned char)(ucCommand[0] | 0x40));_fnSimSD_write(ucCommand[1]);_fnSimSD_write(ucCommand[2]);_fnSimSD_write(ucCommand[3]);_fnSimSD_write(ucCommand[4]);_fnSimSD_write(0);_fnSimSD_write(0xff);
    #endif
        case 6:
            do {                                                         // loop until the command has been transferred
                if ((SDHC_IRQSTAT & SDHC_IRQSTAT_CC) != 0) {             // command has completed
                    if (GO_IDLE_STATE_CMD0 == ucCommand[0]) {            // this command receives no response
                        *ucResult = R1_IN_IDLE_STATE;                    // dummy response for compatibility
                    }
                    else if (/*(READ_SINGLE_BLOCK_CMD17 == ucCommand[0]) || */(WRITE_BLOCK_CMD24 == ucCommand[0])) {
                        *ucResult = 0;                                   // for compatibility
                    }
                    else {
                        if ((SDHC_IRQSTAT & SDHC_IRQSTAT_CTOE) != 0) {   // timeout occurred while waiting for command to complete
                            SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CTOE); // clear flags
                            iCommandYieldCount++;                        // monitor the maximum number of timeouts
                            iCommandState = 0;                           // allow resend of command
                            uTaskerStateChange(TASK_MASS_STORAGE, UTASKER_GO); // switch to polling mode of operation
                            return CARD_BUSY_WAIT;
                        }
                        else if ((SDHC_IRQSTAT & SDHC_IRQSTAT_CCE) != 0) { // CRC error detected
                            if (ucCommand[0] == SEND_OP_COND_ACMD_CMD41) { // this responds with command value 0x3f and CRC-7 0xff
                                *ucResult = 0;                           // for compatibility
                            }
                            else {                                       // other CRC errors cause retry
                                *ucResult = R1_COMMAND_CRC_ERROR;
                            }
                            SDHC_IRQSTAT = (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CCE); // clear flags
                            break;
                        }

                        if ((SDHC_IRQSTAT & (SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE)) == 0) { // if matching response and no error
                            switch (ucCommand[0]) {
                            case SEND_IF_COND_CMD8:
                                *ucResult = SDC_CARD_VERSION_2PLUS;      // for compatibility
                                break;
                            case APP_CMD_CMD55:
                                *ucResult = R1_IN_IDLE_STATE;            // for compatibility
                                break;
                            default:
                                *ucResult = 0;                           // for compatibility
                                break;
                            }
                        }
                    }
                    WRITE_ONE_TO_CLEAR(SDHC_IRQSTAT, (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CCE)); // clear flags
                    break;
                }
                else {                                                   // the command/response is still being executed
                    iCommandState = 6;
                    if (iCommandYieldCount == 0) {                       // just count once as timeout
                        uTaskerStateChange(TASK_MASS_STORAGE, UTASKER_GO); // switch to polling mode of operation                        
                    }
                    iCommandYieldCount++;;
                    return CARD_BUSY_WAIT;                               // poll up to 20 times before yielding
                }
            } while ((*ucResult & SD_CARD_BUSY) != 0);                   // poll the card until it is no longer indicating busy and returns the value
            if (ptrReturnData != 0) {                                    // if the caller requests data, read it here
                if ((ucCommand[0] == SEND_CSD_CMD9) || (ucCommand[0] == SEND_CID_CMD2)) { // exception requiring 16 bytes
    #if defined _WINDOWS
                    _fnSimSD_write(0xff);_fnSimSD_write(0xff);           // for compatibility
                    SDHC_CMDRSP3  = (_fnSimSD_write(0xff) << 16);
                    SDHC_CMDRSP3 |= (_fnSimSD_write(0xff) << 8);
                    SDHC_CMDRSP3 |= _fnSimSD_write(0xff);
                    SDHC_CMDRSP2  = (_fnSimSD_write(0xff) << 24);
                    SDHC_CMDRSP2 |= (_fnSimSD_write(0xff) << 16);
                    SDHC_CMDRSP2 |= (_fnSimSD_write(0xff) << 8);
                    SDHC_CMDRSP2 |=  _fnSimSD_write(0xff);
                    SDHC_CMDRSP1  = (_fnSimSD_write(0xff) << 24);
                    SDHC_CMDRSP1 |= (_fnSimSD_write(0xff) << 16);
                    SDHC_CMDRSP1 |= (_fnSimSD_write(0xff) << 8);
                    SDHC_CMDRSP1 |=  _fnSimSD_write(0xff);
                    SDHC_CMDRSP0  = (_fnSimSD_write(0xff) << 24);
                    SDHC_CMDRSP0 |= (_fnSimSD_write(0xff) << 16);
                    SDHC_CMDRSP0 |= (_fnSimSD_write(0xff) << 8);
                    SDHC_CMDRSP0 |=  _fnSimSD_write(0xff);
                    _fnSimSD_write(0xff);                                // dummy for checksum
    #endif
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP3 >> 16);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP3 >> 8);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP3);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP2 >> 24);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP2 >> 16);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP2 >> 8);
                    *ptrReturnData++ = (unsigned char)SDHC_CMDRSP2;
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP1 >> 24);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP1 >> 16);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP1 >> 8);
                    *ptrReturnData++ = (unsigned char)SDHC_CMDRSP1;
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 24);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 16);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 8);
                    *ptrReturnData++ = (unsigned char)SDHC_CMDRSP0;
                    *ptrReturnData   = 0;                                // checksum not received from controller
                }
                else {
    #if defined _WINDOWS
                    if (ucCommand[0] != SEND_OP_COND_ACMD_CMD41) {
                        _fnSimSD_write(0xff);_fnSimSD_write(0xff);       // for compatibility
                    }
                    SDHC_CMDRSP0  = (_fnSimSD_write(0xff) << 24);        // get the response
                    SDHC_CMDRSP0 |= (_fnSimSD_write(0xff) << 16);
                    SDHC_CMDRSP0 |= (_fnSimSD_write(0xff) << 8);
                    SDHC_CMDRSP0 |=  _fnSimSD_write(0xff);
    #endif
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 24);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 16);
                    *ptrReturnData++ = (unsigned char)(SDHC_CMDRSP0 >> 8);
                    *ptrReturnData   = (unsigned char)SDHC_CMDRSP0;
                }
            }
            break;
        }
    }
    if (iCommandYieldCount != 0) {
        uTaskerStateChange(TASK_MASS_STORAGE, UTASKER_STOP);             // switch to event mode of operation since write has completed
        iCommandYieldCount = 0;
    }
    iCommandState = 0;
    return iRtn;
}

// Temporary debug counters (can be removed)
//
static unsigned long ulSectorReadCount = 0;
static unsigned long ulPartialSectorReadCount = 0;
static unsigned long ulSectorWriteCount = 0;

// Read a sector from SD card into the specified data buffer (the buffer should be long word aligned for maximum efficiency)
//
extern int fnGetSector(unsigned char *ptrBuf)
{
    unsigned long *ptrData = (unsigned long *)ptrBuf;                    // the Kinetis/iMX RT driver ensures that the buffer is long word aligned
    #if READ_WATERMARK != 128
    unsigned long *ptrTerminate = (ptrData + (512 / sizeof(unsigned long))); // end of sector buffer
    #endif
    int iFifoFill = 0;
    int iMonitorRead;                                                    // {62}
    #if defined _COMPILE_KEIL                                            // {47}
    unsigned long ulTempBuffer[512/sizeof(unsigned long)];               // long word aligned intermediate buffer
    int iIntermediateBuffer;
    if ((unsigned long)ptrData & 0x03) {                                 // check that the caller's buffer is long word aligned
        ptrData = ulTempBuffer;                                          // read to intermediate buffer to avoid access problems
        iIntermediateBuffer = 1;
    }
    else {
        iIntermediateBuffer = 0;
    }
    #endif
    #if defined _WINDOWS
    _fnSimSD_write(0xff);                                                // dummy for simulator compatibility
    #endif
    while ((SDHC_PRSSTAT & (SDHC_PRSSTAT_DLA | SDHC_PRSSTAT_SDOFF)) == SDHC_PRSSTAT_DLA) { // ensure that the previous command has completed (in multiple block reads the clock may be gated off to pause and so this is accepted)
    #if defined _WINDOWS
        SDHC_PRSSTAT &= ~SDHC_PRSSTAT_DLA;                               // data lines on the SD bus no longer active
    #endif
    };
    ulSectorReadCount++;                                                 // counter to indicate how many times a sector read has been executed
    #if READ_WATERMARK != 128
    FOREVER_LOOP() {
    #endif
        iMonitorRead = 0;                                                // {62}
        while ((SDHC_PRSSTAT & SDHC_PRSSTAT_BREN) == 0) {                // wait for the buffer read enable flag to become set (fifo ready to be read)
    #if defined _WINDOWS
            SDHC_PRSSTAT |= SDHC_PRSSTAT_BREN;
    #endif
            if ((SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE)) != 0) { // check for read errors
                SDHC_IRQSTAT = (SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE | SDHC_IRQSTAT_BRR); // reset error flags
                return UTFAT_DISK_READ_ERROR;                            // return error
            }
            if (++iMonitorRead >= 1000) {                                // {62} if the buffer stalls (can happen when SD card removed during operation)
                return UTFAT_DISK_READ_ERROR;                            // return with error
            }
        }
    #if (READ_WATERMARK == 128) && defined SDCARD_RX_DMA_CHANNEL && !defined DEVICE_WITHOUT_DMA && !defined _WINDOWS // {64}
        if (((CAST_POINTER_ARITHMETIC)ptrData & 0x3) == 0) {             // if long word aligned we can use DMA transfer
            // Note that the DMA copy is not necessarily faster than the loop copy but it allows interrupts to be handled without slowing the read
            //
            fnConfigDMA_buffer(SDCARD_RX_DMA_CHANNEL, 0, 512, SDHC_DATPORT_ADDR, ptrData, (DMA_DIRECTION_INPUT | DMA_LONG_WORDS | DMA_SINGLE_CYCLE | DMA_SW_TRIGGER_WAIT_TERMINATION), 0, 0); // configure the transfer, start and wait for termination
        }
        else {
    #endif
            while (++iFifoFill <= READ_WATERMARK) {                      // {64} tight loop for read of available fifo content
    #if defined _WINDOWS                                                 // read data from SD card simulator
                SDHC_DATPORT = _fnSimSD_write(0xff);
                SDHC_DATPORT |= (_fnSimSD_write(0xff) << 8);
                SDHC_DATPORT |= (_fnSimSD_write(0xff) << 16);
                SDHC_DATPORT |= (_fnSimSD_write(0xff) << 24);
    #endif
                *ptrData++ = SDHC_DATPORT;                               // this access should be long word aligned for maximum efficiency            
            }
    #if READ_WATERMARK != 128
            if (ptrData >= ptrTerminate) {                               // complete data read
                break;
            }
            iFifoFill = 0;
    }
    #endif
    #if (READ_WATERMARK == 128) && defined SDCARD_RX_DMA_CHANNEL && !defined DEVICE_WITHOUT_DMA && !defined _WINDOWS
        }
    #endif
    while ((SDHC_IRQSTAT & SDHC_IRQSTAT_TC) == 0) {                      // wait for transfer to complete
        if ((SDHC_PRSSTAT & SDHC_PRSSTAT_CCIHB) != 0) {                  // during block reads the data inhibit remains set and we use it to continue
            break;
        }
    #if defined _WINDOWS
        SDHC_IRQSTAT |= SDHC_IRQSTAT_TC;
    #endif
    }
    #if defined _WINDOWS
    _fnSimSD_write(0xff);_fnSimSD_write(0xff);                           // dummy for simulator compatibility
    #endif
    SDHC_IRQSTAT = (SDHC_IRQSTAT_TC | SDHC_IRQSTAT_BRR | SDHC_IRQSTAT_AC12E); // reset flags
    #if defined _COMPILE_KEIL                                            // {47}
    if (iIntermediateBuffer != 0) {                                      // read data is in intermediate buffer
        uMemcpy(ptrBuf, ((unsigned char *)ulTempBuffer), 512);           // copy from intermediate buffer to user's buffer which isn't aligned
    }
    #endif
    return UTFAT_SUCCESS;                                                // read successfully terminated
}

// Read a specified amount of data from present SD card sector into the specified data buffer (usStart and usStop are offset from start of sector and avoid other data outside of this range being overwritten)
//
extern int fnReadPartialSector(unsigned char *ptrBuf, unsigned short usStart, unsigned short usStop)
{
    int iReadState;
    unsigned long ulTempBuffer[512/sizeof(unsigned long)];               // since the SD controller will always read a complete sector we use an intermediate buffer (it is long word aligned)
    ulPartialSectorReadCount++;
    if ((iReadState = fnGetSector((unsigned char *)ulTempBuffer)) == UTFAT_SUCCESS) { // fill the temporary buffer with the sector data
        uMemcpy(ptrBuf, (((unsigned char *)ulTempBuffer) + usStart), (usStop - usStart)); // just copy the part required
    }
    return iReadState;
}

// Write present sector with buffer data
//
extern int fnPutSector(unsigned char *ptrBuf, int iMultiBlock)
{
    #if WRITE_WATERMARK == 128
    int iMonitorWrite = 0;
    #endif
    unsigned long *ptrData = (unsigned long *)ptrBuf;
    int i = (512/sizeof(unsigned long));                                 // the number of long words
    ulSectorWriteCount++;
    #if defined _WINDOWS
    _fnSimSD_write(0xfe);                                                // dummy for simulator compatibility
    #endif
    #if WRITE_WATERMARK == 128
    while ((SDHC_PRSSTAT & SDHC_PRSSTAT_BWEN) == 0) {                    // wait for the buffer write enable flag to become set (fifo ready to be written to)
        #if defined _WINDOWS
        SDHC_PRSSTAT |= SDHC_PRSSTAT_BWEN;
        #endif
        if ((SDHC_IRQSTAT & (SDHC_IRQSTAT_DMAE | SDHC_IRQSTAT_AC12E | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE)) != 0) { // check for write errors
            SDHC_IRQSTAT = (SDHC_IRQSTAT_DMAE | SDHC_IRQSTAT_AC12E | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE); // reset error flags
            return UTFAT_DISK_WRITE_ERROR;                               // return error
        }
        if (++iMonitorWrite >= 10000) {                                  // if the buffer stalls (can happen when SD card removed during operation)
             return UTFAT_DISK_WRITE_ERROR;                              // return with error
        }
    }
    #if (WRITE_WATERMARK == 128) && defined SDCARD_TX_DMA_CHANNEL && !defined DEVICE_WITHOUT_DMA && !defined _WINDOWS
    if (((CAST_POINTER_ARITHMETIC)ptrBuf & 0x3) == 0) {                  // if long word aligned we can use DMA transfer
        // Note that the DMA copy is not necessarily faster than the loop copy but it allows interrupts to be handled without slowing the write
        //
        fnConfigDMA_buffer(SDCARD_TX_DMA_CHANNEL, 0, 512, ptrBuf, SDHC_DATPORT_ADDR, (DMA_DIRECTION_OUTPUT | DMA_LONG_WORDS | DMA_SINGLE_CYCLE | DMA_SW_TRIGGER_WAIT_TERMINATION), 0, 0); // configure the transfer, start and wait for termination
    }
    else {
    #endif
        while (i-- != 0) {                                               // for each long word of the sector buffer to be written
            SDHC_DATPORT = *ptrData++;                                   // copy the data to be sent to the SDHC buffer (it will be transferred by the controller as fast as possible to the card)
            #if defined _WINDOWS                                         // simulate the write of 4 bytes from the long word buffer entry
            _fnSimSD_write((unsigned char)SDHC_DATPORT);
            _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 8));
            _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 16));
            _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 24));
            #endif
        }
    #if (WRITE_WATERMARK == 128) && defined SDCARD_TX_DMA_CHANNEL && !defined DEVICE_WITHOUT_DMA && !defined _WINDOWS
    }
    #endif
    #else
    while (i-- != 0) {                                                   // for each long word of the sector buffer to be written
        _WAIT_REGISTER_FALSE(SDHC_PRSSTAT, SDHC_PRSSTAT_BWEN);           // wait until there is buffer space when the transmission buffer fills
        SDHC_DATPORT = *ptrData++;                                       // copy the data to be sent to the SDHC buffer (it will be transferred by the controller as fast as possible to the card)
        #if defined _WINDOWS                                             // simulate the write of 4 bytes from the long word buffer entry
        _fnSimSD_write((unsigned char)SDHC_DATPORT);
        _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 8));
        _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 16));
        _fnSimSD_write((unsigned char)(SDHC_DATPORT >> 24));
        #endif
    }
    #endif
    #if defined _WINDOWS
    _fnSimSD_write(0xff);_fnSimSD_write(0xff);_fnSimSD_write(0xff);      // dummy for simulator compatibility after sector data transfer
    #endif
    if (iMultiBlock == 0) {                                              // when performing multiple block transfers the transfer complete is not waited for
        while ((SDHC_IRQSTAT & SDHC_IRQSTAT_TC) == 0) {                  // wait for transfer to complete
        #if defined _WINDOWS
            SDHC_IRQSTAT |= SDHC_IRQSTAT_TC;
        #endif
        }
        SDHC_IRQSTAT = (SDHC_IRQSTAT_TC | SDHC_IRQSTAT_BWR | SDHC_IRQSTAT_AC12E); // reset flags
    }
    #if READ_WATERMARK != 1 && WRITE_WATERMARK == 1
    SDHC_WML = ((WRITE_WATERMARK << 16) | READ_WATERMARK);               // return read watermark setting after write
    #endif
    return UTFAT_SUCCESS;
}
