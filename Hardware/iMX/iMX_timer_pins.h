/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_timer_pins.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#if defined _WINDOWS
    extern void fnEnterTimer(int iPortRef, int iPinRef, unsigned long ulReference, int iOnOff);
#else
    #define fnEnterTimer(A, B, C, D)
#endif

// GPT pin configuration that can be used by square wave outputs or capture inputs
//
static void fnConfigGPT_Pin(int iTimer, int iChannel, unsigned long ulCharacteristics)
{
    switch (iTimer) {
    case 0:                                                              // GPT 1
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // GPT 1, channel 1
            if ((ulCharacteristics & IOMUXC_SW_PAD_CTL_PAD_DSE_7) != 0) {// output type
#if defined iMX_RT105X || defined iMX_RT106X
                _CONFIG_PERIPHERAL(GPIO_EMC_35, GPT1_COMPARE1, (ulCharacteristics)); // select GPT1_COMPARE1 on GPIO3-21 - alt function 2
    #if defined iMX_RT106X
                _CONFIG_PERIPHERAL(GPIO_B1_07, GPT1_COMPARE1, (ulCharacteristics)); // select GPT1_COMPARE1 on GPIO2-23 - alt function 8
    #endif
#elif defined iMX_RT1011
                _CONFIG_PERIPHERAL(GPIO_07, GPT1_COMPARE1, (ulCharacteristics)); // select GPT1_COMPARE1 on GPIO1-07 - alt function 1
#else
                _CONFIG_PERIPHERAL(GPIO_AD_B0_00, GPT1_COMPARE1, (ulCharacteristics)); // select GPT1_COMPARE1 on GPIO1-0 - alt function 7
#endif
                return;
            }
            // Input
            //
#if defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_EMC_24, GPT1_CAPTURE1, (ulCharacteristics)); // select GPT1_CAPTURE1 on GPIO4-24 - alt function 4
            fnEnterTimer(_PORT4, 24, iMX_GPT_1_CAPTURE_1, 1);
    #if defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_B1_05, GPT1_CAPTURE1, (ulCharacteristics)); // select GPT1_CAPTURE1 on GPIO2-21 - alt function 8
            fnEnterTimer(_PORT2, 21, iMX_GPT_1_CAPTURE_1, 1);
    #endif
#elif defined iMX_RT1011
            _CONFIG_PERIPHERAL(GPIO_06, GPT1_CAPTURE1, (ulCharacteristics)); // select GPT1_CAPTURE1 on GPIOMUX_IO06 - alt function 1
            fnEnterTimer(_PORT1, 6, iMX_GPT_1_CAPTURE_1, 1);
#else
            _CONFIG_PERIPHERAL(GPIO_AD_B0_02, GPT1_CAPTURE1, (ulCharacteristics)); // select GPT1_CAPTURE1 on GPIO1-02 - alt function 7
            fnEnterTimer(_PORT1, 2, iMX_GPT_1_CAPTURE_1, 1);
#endif
            break;
        case 1:                                                          // GPT 1, channel 2
            if ((ulCharacteristics & IOMUXC_SW_PAD_CTL_PAD_DSE_7) != 0) {// output type
                return;
            }
            // Input
            //
#if defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_EMC_23, GPT1_CAPTURE2, (ulCharacteristics)); // select GPT1_CAPTURE2 on GPIO4-23 - alt function 4
            fnEnterTimer(_PORT4, 23, iMX_GPT_1_CAPTURE_2, 1);
    #if defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_B1_06, GPT1_CAPTURE2, (ulCharacteristics)); // select GPT1_CAPTURE2 on GPIO2-22 - alt function 8
            fnEnterTimer(_PORT2, 22, iMX_GPT_1_CAPTURE_2, 1);
    #endif
#elif !defined iMX_RT1011
            _CONFIG_PERIPHERAL(GPIO_AD_B0_01, GPT1_CAPTURE2, (ulCharacteristics)); // select GPT1_CAPTURE2 on GPIO1-01 - alt function 7
            fnEnterTimer(_PORT1, 1, iMX_GPT_1_CAPTURE_2, 1);
#endif
            break;
        default:
    #if defined _WINDOWS
            _EXCEPTION("Invalid GPT 1 channel!!");
    #endif
            return;                                                      // invalid channel
        }
        break;

    case 1:                                                              // GPT 2
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // GPT 2, channel 1
            if ((ulCharacteristics & IOMUXC_SW_PAD_CTL_PAD_DSE_7) != 0) {// output type
                return;
            }
            // Input
            //
#if defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_EMC_41, GPT2_CAPTURE1, (ulCharacteristics)); // select GPT2_CAPTURE1 on GPIO3-27 - alt function 1
            fnEnterTimer(_PORT3, 27, iMX_GPT_2_CAPTURE_1, 1);
    #if defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B1_03, GPT2_CAPTURE1, (ulCharacteristics)); // select GPT2_CAPTURE1 on GPIO1-19 - alt function 8
            fnEnterTimer(_PORT1, 19, iMX_GPT_2_CAPTURE_1, 1);
    #endif
#elif defined iMX_RT1011
            _CONFIG_PERIPHERAL(GPIO_AD_05, GPT2_CAPTURE1, (ulCharacteristics)); // select GPT2_CAPTURE1 on GPIOMUX_IO09 - alt function 1
            fnEnterTimer(_PORT1, 9, iMX_GPT_2_CAPTURE_1, 1);
#else
            _CONFIG_PERIPHERAL(GPIO_AD_B1_10, GPT2_CAPTURE1, (ulCharacteristics)); // select GPT2_CAPTURE1 on GPIO1-26 - alt function 6
            fnEnterTimer(_PORT1, 26, iMX_GPT_2_CAPTURE_1, 1);
#endif
            break;
        case 1:                                                          // GPT 2, channel 2
            if ((ulCharacteristics & IOMUXC_SW_PAD_CTL_PAD_DSE_7) != 0) {// output type
                return;
            }
            // Input
            //
#if defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_EMC_40, GPT2_CAPTURE2, (ulCharacteristics)); // select GPT2_CAPTURE2 on GPIO3-26 - alt function 1
            fnEnterTimer(_PORT3, 26, iMX_GPT_2_CAPTURE_2, 1);
    #if defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B1_04, GPT2_CAPTURE2, (ulCharacteristics)); // select GPT2_CAPTURE2 on GPIO1-20- alt function 8
            fnEnterTimer(_PORT1, 20, iMX_GPT_2_CAPTURE_2, 1);
    #endif
#elif defined iMX_RT1011
            _CONFIG_PERIPHERAL(GPIO_AD_07, GPT2_CAPTURE2, (ulCharacteristics)); // select GPT2_CAPTURE2 on GPIOMUX_IO21 - alt function 4
            fnEnterTimer(_PORT1, 21, iMX_GPT_2_CAPTURE_2, 1);
#else
            _CONFIG_PERIPHERAL(GPIO_EMC_28, GPT2_CAPTURE2, (ulCharacteristics)); // select GPT2_CAPTURE2 on GPIO2-28 - alt function 6
            fnEnterTimer(_PORT2, 28, iMX_GPT_2_CAPTURE_2, 1);
#endif
            break;

        default:
            _EXCEPTION("Invalid GPT 2 channel!!");
            return;                                                      // invalid channel
        }
        break;
    }
}

static void fnConfigGPT_Clk(int iTimer, unsigned long ulCharacteristics)
{
    switch (iTimer) {
    case iMX_GPT_1:                                                      // GPT 1
#if defined iMX_RT105X || defined iMX_RT106X
    #if defined GTP1_CLK_ON_AD_B0 || defined iMX_RT105X
        #if defined iMX_RT106X
        IOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT = IOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT_GPIO_AD_B0_13_ALT1;
        #endif
        _CONFIG_PERIPHERAL(GPIO_AD_B0_13, GPT1_CLK, (ulCharacteristics));// select GPT1_CLK on GPIO1-13 - alt function 1
        fnEnterTimer(_PORT1, 13, iMX_GPT_1_CLOCK, 1);
    #else
        IOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT = IOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT_GPIO_B1_04_ALT8;
        _CONFIG_PERIPHERAL(GPIO_B1_04, GPT1_CLK, (ulCharacteristics));   // select GPT1_CLK on GPIO2-20 - alt function 8
        fnEnterTimer(_PORT2, 20, iMX_GPT_1_CLOCK, 1);
    #endif
#elif defined iMX_RT1011
        _CONFIG_PERIPHERAL(GPIO_08, GPT1_CLK, (ulCharacteristics));      // select GPT1_CLK on GPIO1-08 - alt function 1
        fnEnterTimer(_PORT1, 8, iMX_GPT_1_CLOCK, 1);
#else
        _CONFIG_PERIPHERAL(GPIO_EMC_39, GPT1_CLK, (ulCharacteristics));  // select GPT1_CLK on GPIO3-07 - alt function 7
        fnEnterTimer(_PORT3, 7, iMX_GPT_1_CLOCK, 1);
#endif
        break;
    case iMX_GPT_2:                                                      // GPT 2
#if defined iMX_RT105X || defined iMX_RT106X
    #if defined GTP1_CLK_ON_AD_B0 || defined iMX_RT105X
        #if defined iMX_RT106X
        IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT = IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT_GPIO_AD_B0_09_ALT7;
        #endif
        _CONFIG_PERIPHERAL(GPIO_AD_B0_09, GPT2_CLK, (ulCharacteristics)); // select GPT2_CLK on GPIO1-09 - alt function 7
        fnEnterTimer(_PORT1, 9, iMX_GPT_2_CLOCK, 1);
    #else
        IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT = IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT_GPIO_AD_B1_02_ALT8;
        _CONFIG_PERIPHERAL(GPIO_AD_B1_02, GPT2_CLK, (ulCharacteristics));// select GPT2_CLK on GPIO1-18 - alt function 8
        fnEnterTimer(_PORT1, 18, iMX_GPT_2_CLOCK, 1);
    #endif
#elif defined iMX_RT1011
        _CONFIG_PERIPHERAL(GPIO_AD_03, GPT2_CLK, (ulCharacteristics));   // select GPT1_CLK on GPIO1-17 - alt function 4
        fnEnterTimer(_PORT1, 17, iMX_GPT_2_CLOCK, 1);
#else
        _CONFIG_PERIPHERAL(GPIO_EMC_31, GPT2_CLK, (ulCharacteristics));  // select GPT2_CLK on GPIO2-31 - alt function 6
        fnEnterTimer(_PORT2, 31, iMX_GPT_2_CLOCK, 1);
#endif
        break;
    }
}

// FlexPWM pin configuration that can be used by PWM outputs or capture inputs
//
static void fnConfigPWM_Pin(int iFlexPWM, int iChannel, unsigned char pwm_reference, unsigned long ulCharacteristics)
{
    switch (iFlexPWM) {
    case 0:                                                              // FlexPWM 1
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // FlexPWM 1, channel 0
            if ((pwm_reference & _FLEX_PWM_A_OUTPUT) != 0) {
#if defined iMX_RT105X || defined iMX_RT106X
    #if defined FLEXPWM1_PWM3_A_ON_EMC
                _CONFIG_PERIPHERAL(GPIO_EMC_23, FLEXPWM1_PWMA00, (ulCharacteristics)); // select FLEXPWM1_PWMA00 on GPIO4-23 - alt function 1
                IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT_GPIO_EMC_23_ALT1;
                fnEnterTimer(_PORT4, 23, iMX_FLEXPWM_1_0_A_OUTPUT, 1);
    #else
                _CONFIG_PERIPHERAL(GPIO_SD_B0_00, FLEXPWM1_PWMA00, (ulCharacteristics)); // select FLEXPWM1_PWMA00 on GPIO3-12 - alt function 1
                IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT_GPIO_SD_B0_00_ALT1;
                fnEnterTimer(_PORT3, 12, iMX_FLEXPWM_1_0_A_OUTPUT, 1);
    #endif
#endif
            }
            break;
        case 1:                                                          // FlexPWM 1, channel 1
            break;
        case 2:                                                          // FlexPWM 1, channel 2
            break;
        case 3:                                                          // FlexPWM 1, channel 3
            if ((pwm_reference & _FLEX_PWM_A_OUTPUT) != 0) {
#if defined iMX_RT105X || defined iMX_RT106X
    #if defined FLEXPWM1_PWM3_A_ON_EMC_38
                _CONFIG_PERIPHERAL(GPIO_EMC_38, FLEXPWM1_PWM3_A, (ulCharacteristics)); // select FLEXPWM1_PWM3_A on GPIO3-24 - alt function 1
                IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT_PIO_EMC_38_ALT1;
                fnEnterTimer(_PORT3, 24, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
    #elif defined FLEXPWM1_PWM3_A_ON_EMC_12
                _CONFIG_PERIPHERAL(GPIO_EMC_12, FLEXPWM1_PWMA03, (ulCharacteristics)); // select FLEXPWM1_PWM3_A on GPIO4-12 - alt function 4
                IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT_GPIO_EMC_12_ALT4;
                fnEnterTimer(_PORT4, 12, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
    #elif defined FLEXPWM1_PWM3_A_ON_SD
                _CONFIG_PERIPHERAL(GPIO_SD_B1_00, FLEXPWM1_PWMA03, (ulCharacteristics)); // select FLEXPWM1_PWM3_A on GPIO3-00 - alt function 2
                IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT_GPIO_SD_B1_00_ALT2;
                fnEnterTimer(_PORT3, 0, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
    #elif defined FLEXPWM1_PWM3_A_ON_AD
                _CONFIG_PERIPHERAL(GPIO_AD_B0_10, FLEXPWM1_PWMA03, (ulCharacteristics)); // select FLEXPWM1_PWM3_A on GPIO3-00 - alt function 1
                IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT_GPIO_AD_B0_10_ALT1;
                fnEnterTimer(_PORT1, 10, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
    #else
                _CONFIG_PERIPHERAL(GPIO_B1_00, FLEXPWM1_PWM3_A, (ulCharacteristics)); // select FLEXPWM1_PWM3_A on GPIO3-00 - alt function 6
                IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT_GPIO_B1_00_ALT6;
                fnEnterTimer(_PORT2, 16, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
    #endif
#elif !defined iMX_RT1011
                _CONFIG_PERIPHERAL(GPIO_EMC_20, FLEXPWM1_PWMA03, (ulCharacteristics)); // select FLEXPWM1_PWMA3 on GPIO2-20 - alt function 1
                fnEnterTimer(_PORT2, 20, iMX_FLEXPWM_1_3_A_OUTPUT, 1);
#endif
            }
            break;
        default:
            _EXCEPTION("Invalid FlexPWM1 channel!!");
            break;
        }
        break;
#if FLEXPWM_AVAILABLE > 1
    case 1:                                                              // FlexPWM 2
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // FlexPWM 2, channel 0
            break;
        case 1:                                                          // FlexPWM 2, channel 1
            break;
        case 2:                                                          // FlexPWM 2, channel 2
            break;
        case 3:                                                          // FlexPWM 2, channel 3
            break;
        default:
            _EXCEPTION("Invalid FlexPWM2 channel!!");
            break;
        }
        break;
#endif
#if FLEXPWM_AVAILABLE > 2
    case 2:                                                              // FlexPWM 3
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // FlexPWM 3, channel 0
            break;
        case 1:                                                          // FlexPWM 3, channel 1
            break;
        case 2:                                                          // FlexPWM 3, channel 2
            if ((pwm_reference & _FLEX_PWM_A_OUTPUT) != 0) {
                _CONFIG_PERIPHERAL(GPIO_EMC_33, FLEXPWM3_PWMA02, (ulCharacteristics)); // select FLEXPWM3_PWM2_A on GPIO3-19 - alt function 1
                fnEnterTimer(_PORT3, 19, iMX_FLEXPWM_3_2_A_OUTPUT, 1);
            }
            break;
        case 3:                                                          // FlexPWM 3, channel 3
            break;
        default:
            _EXCEPTION("Invalid FlexPWM3 channel!!");
            break;
        }
        break;
#endif
#if FLEXPWM_AVAILABLE > 3
    case 3:                                                              // FlexPWM 4
        switch (iChannel) {                                              // configure appropriate pin for the timer signal
        case 0:                                                          // FlexPWM 4, channel 0
            break;
        case 1:                                                          // FlexPWM 4, channel 1
            break;
        case 2:                                                          // FlexPWM 4, channel 2
            if ((pwm_reference & _FLEX_PWM_A_OUTPUT) != 0) {
 #if defined iMX_RT105X || defined iMX_RT106X
    #if defined FLEXPWM4_PWM2_A_ON_EMC
                _CONFIG_PERIPHERAL(GPIO_EMC_04, FLEXPWM4_PWMA02, (ulCharacteristics)); // select FLEXPWM4_PWM2_A on GPIO4-04 - alt function 1
                IOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT = IOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT_GPIO_EMC_04_ALT1;
                fnEnterTimer(_PORT4, 4, iMX_FLEXPWM_4_2_A_OUTPUT, 1);
    #else
                _CONFIG_PERIPHERAL(GPIO_B1_14, FLEXPWM4_PWM2_A, (ulCharacteristics)); // select FLEXPWM4_PWM2_A on GPIO2-30 - alt function 1
                IOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT = IOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT_GPIO_B1_14_ALT1;
                fnEnterTimer(_PORT2, 30, iMX_FLEXPWM_4_2_A_OUTPUT, 1);
    #endif
#endif         
            }
            break;
        case 3:                                                          // FlexPWM 4, channel 3
            if ((pwm_reference & _FLEX_PWM_A_OUTPUT) != 0) {
#if defined iMX_RT105X || defined iMX_RT106X
    #if defined FLEXPWM4_PWM2_A_ON_EMC
                _CONFIG_PERIPHERAL(GPIO_EMC_17, FLEXPWM4_PWMA03, (ulCharacteristics)); // select FLEXPWM4_PWM3_A on GPIO4-17 - alt function 1
                IOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT_GPIO_EMC_17_ALT1;
                fnEnterTimer(_PORT4, 17, iMX_FLEXPWM_4_3_A_OUTPUT, 1);
    #else
                _CONFIG_PERIPHERAL(GPIO_B1_15, FLEXPWM4_PWM3_A, (ulCharacteristics)); // select FLEXPWM4_PWM3_A on GPIO2-31 - alt function 1
                IOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT = IOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT_GPIO_B1_15_ALT1;
                fnEnterTimer(_PORT2, 31, iMX_FLEXPWM_4_3_A_OUTPUT, 1);
    #endif
#endif
            }
            break;
        default:                                                         // invalid channel
            _EXCEPTION("Invalid FlexPWM4 channel!!");
            break;
        }
        break;
#endif
    default:
        _EXCEPTION("Invalid FlexPWM module!!");
        break;
    }
}
