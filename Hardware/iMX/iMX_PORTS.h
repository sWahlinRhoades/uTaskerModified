/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_PORTS.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    03.09.2020 Added input DMA triggering support based on XBAR

    See the following video showing port interrupt operation on a KL27: https://youtu.be/CubinvMuTwU
    See XBAR video explaining the method used for input DMA triggering: https://www.youtube.com/watch?v=zNWIG-O7ZW0&feature=youtu.be

*/

#if defined _PORT_MUX_CODE
// This routine is used to connect one or more pins to their GPIO function with defined characteristics (it also powers the port in question)
//
extern void fnConnectGPIO(int iPortRef, unsigned long ulPortBits, unsigned long ulCharacteristics)
{
    unsigned long ulBit = 0x00000001;
    unsigned long *ptrGPIO;
    unsigned long *ptrChars;
    int iPortChange = 0;
    #if defined iMX_RT1011
    unsigned long ulMaxPortBit;
    #endif

    switch (iPortRef) {
    case PORT1:
        POWER_UP_ATOMIC(1, GPIO1_CLOCK);                                 // power up the port
    #if defined iMX_RT1011
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_00_ADD;                     // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_00_ADD;                    // pad characteristics register block
        ulMaxPortBit = PORT_BIT28;
        iPortChange = 1;
    #else
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00_ADD;               // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_00_ADD;              // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR26 &= ~(ulPortBits);                               // ensure fast ports are not multiplexed to
        #endif
    #endif
        break;
    case PORT2:
        POWER_UP_ATOMIC(0, GPIO2_CLOCK);                                 // power up the port
    #if defined iMX_RT1011
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_00_ADD;                  // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_00_ADD;                 // pad characteristics register block
        ulMaxPortBit = PORT_BIT13;
    #elif defined iMX_RT105X || defined iMX_RT106X
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00_ADD;                  // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_00_ADD;                 // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR27 &= ~(ulPortBits);                               // ensure fast ports are not multiplexed to
        #endif
    #else
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00_ADD;                 // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00_ADD;                // pad characteristics register block
    #endif
        break;
    #if defined PORT3
    case PORT3:
        POWER_UP_ATOMIC(2, GPIO3_CLOCK);                                 // power up the port
        #if defined iMX_RT105X || defined iMX_RT106X
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00_ADD;               // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_00_ADD;              // pad characteristics register block
            #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR28 &= ~(ulPortBits);                               // ensure fast ports are not multiplexed to
            #endif
        #else
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32_ADD;                 // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32_ADD;                // pad characteristics register block
        #endif
        iPortChange = 1;
        break;
    #endif
    #if defined PORT4
    case PORT4:
        POWER_UP_ATOMIC(3, GPIO4_CLOCK);                                 // power up the port
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00_ADD;                 // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00_ADD;                // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR29 &= ~(ulPortBits);                               // ensure fast ports are not multiplexed to
        #endif
        break;
    #endif
    case PORT5:
    #if defined iMX_RT1011
        ptrGPIO = IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ_ADD;            // pad multiplexer register block
        ptrChars = (unsigned long *)IOMUXC_SNVS_SW_PAD_CTL_PAD_PMIC_ON_REQ_ADD; // pad characteristics register block
        ulMaxPortBit = PORT_BIT0;
    #else
        ptrGPIO = IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP_ADD;                 // pad multiplexer register block
        ptrChars = IOMUXC_SNVS_SW_PAD_CTL_PAD_WAKEUP_ADD;                // pad characteristics register block
    #endif
        break;
    #if defined PORT6
    case PORT6:                                                          // fast port muxed to GPIO1 pins
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00_ADD;               // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_00_ADD;              // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR26 |= (ulPortBits);                                // ensure fast ports are multiplexed to
        #endif
        break;
    #endif
    #if defined PORT7
    case PORT7:                                                          // fast port muxed to GPIO2 pins
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00_ADD;                  // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_00_ADD;                 // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR27 |= (ulPortBits);                                // ensure fast ports are multiplexed to
        #endif
        break;
    #endif
    #if defined PORT8
    case PORT8:                                                          // fast port muxed to GPIO3 pins
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00_ADD;               // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_00_ADD;              // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR28 |= (ulPortBits);                                // ensure fast ports are multiplexed to
        #endif
        break;
    #endif
    #if defined PORT9
    case PORT9:                                                          // fast port muxed to GPIO4 pins
        ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00_ADD;                 // pad multiplexer register block
        ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00_ADD;                // pad characteristics register block
        #if defined FAST_GPIO_AVAILABLE
        IOMUXC_GPR_GPR29 |= (ulPortBits);                                // ensure fast ports are multiplexed to
        #endif
        break;
    #endif
    default:
        _EXCEPTION("The port that is being accessed is not available on this processor!!");
        return;
    }

    // Complete set of GPIO pins as reference (i.MX RT 1020)
    //
    /*
    _CONFIG_PERIPHERAL(GPIO_AD_B0_00, GPIO1_IO00, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_01, GPIO1_IO01, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_02, GPIO1_IO02, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_03, GPIO1_IO03, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_04, GPIO1_IO04, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_05, GPIO1_IO05, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_06, GPIO1_IO06, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_07, GPIO1_IO07, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_08, GPIO1_IO08, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_09, GPIO1_IO09, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_10, GPIO1_IO10, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_11, GPIO1_IO11, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_12, GPIO1_IO12, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_13, GPIO1_IO13, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_14, GPIO1_IO14, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B0_15, GPIO1_IO15, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_00, GPIO1_IO16, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_01, GPIO1_IO17, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_02, GPIO1_IO18, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_03, GPIO1_IO19, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_04, GPIO1_IO20, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_05, GPIO1_IO21, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_06, GPIO1_IO22, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_07, GPIO1_IO23, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_08, GPIO1_IO24, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_09, GPIO1_IO25, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_10, GPIO1_IO26, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_11, GPIO1_IO27, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_12, GPIO1_IO28, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_13, GPIO1_IO29, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_14, GPIO1_IO30, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_AD_B1_15, GPIO1_IO31, UART_PULL_UPS);

    _CONFIG_PERIPHERAL(GPIO_EMC_00, GPIO2_IO00, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_01, GPIO2_IO01, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_02, GPIO2_IO02, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_03, GPIO2_IO03, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_04, GPIO2_IO04, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_05, GPIO2_IO05, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_06, GPIO2_IO06, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_07, GPIO2_IO07, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_08, GPIO2_IO08, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_09, GPIO2_IO09, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_10, GPIO2_IO10, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_11, GPIO2_IO11, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_12, GPIO2_IO12, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_13, GPIO2_IO13, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_14, GPIO2_IO14, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_15, GPIO2_IO15, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_16, GPIO2_IO16, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_17, GPIO2_IO17, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_18, GPIO2_IO18, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_19, GPIO2_IO19, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_20, GPIO2_IO20, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_21, GPIO2_IO21, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_22, GPIO2_IO22, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_23, GPIO2_IO23, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_24, GPIO2_IO24, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_25, GPIO2_IO25, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_26, GPIO2_IO26, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_27, GPIO2_IO27, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_28, GPIO2_IO28, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_29, GPIO2_IO29, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_30, GPIO2_IO30, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_31, GPIO2_IO31, UART_PULL_UPS);

    _CONFIG_PERIPHERAL(GPIO_EMC_32, GPIO3_IO00, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_33, GPIO3_IO01, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_34, GPIO3_IO02, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_35, GPIO3_IO03, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_36, GPIO3_IO04, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_37, GPIO3_IO05, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_38, GPIO3_IO06, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_39, GPIO3_IO07, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_40, GPIO3_IO08, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_EMC_41, GPIO3_IO09, UART_PULL_UPS);

    _CONFIG_PERIPHERAL(GPIO_SD_B0_00, GPIO3_IO13, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_01, GPIO3_IO14, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_02, GPIO3_IO15, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_03, GPIO3_IO16, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_04, GPIO3_IO17, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_05, GPIO3_IO18, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B0_06, GPIO3_IO19, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_00, GPIO3_IO20, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_01, GPIO3_IO21, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_02, GPIO3_IO22, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_03, GPIO3_IO23, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_04, GPIO3_IO24, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_05, GPIO3_IO25, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_06, GPIO3_IO26, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_07, GPIO3_IO27, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_08, GPIO3_IO28, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_09, GPIO3_IO29, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_10, GPIO3_IO30, UART_PULL_UPS);
    _CONFIG_PERIPHERAL(GPIO_SD_B1_11, GPIO3_IO31, UART_PULL_UPS);

    // Exceptions
    //
    IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP = PAD_WAKEUP_GPIO5_IO00;
    IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ = PAD_PMIC_ON_REQ_GPIO5_IO01:
    IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_STBY_REQ = PAD_PMIC_STBY_REQ_GPIO5_IO02
    */

    while (ulPortBits != 0) {                                            // for each specified pin
        if ((ulPortBits & ulBit) != 0) {                                 // if this port bit is to be set as GPIO
            if ((PORT_PSEUDO_FLAG_SET_ONLY_PULLS & ulCharacteristics) == 0) { // don't allow the multiplexer setting to be changed so that peripheral function setting is not modified
                *ptrGPIO = PAD_MUX_MODE_GPIO;                            // set the GPIO mode for this pin
    #if defined _WINDOWS
        #if defined PIN_GPIO_SD_B1_05_GPIO3_IO25 && !defined APPLICATION_REQUIRES_GPIO_SD_B1_05
                if (iPortRef == PORT3) {
                    if (ulBit == PIN_GPIO_SD_B1_05_GPIO3_IO25) {
                        if ((FLEX_SPI_MCR0 & FLEX_SPI_MCR0_RXCLKSRC_STROBE) == FLEX_SPI_MCR0_RXCLKSRC_DQS_LOOPBACK) {
                            _EXCEPTION("PIN_GPIO_SD_B1_05_GPIO3_IO25 is being used by FlexSPI mode for its sample clock loopback!!! Use APPLICATION_REQUIRES_GPIO_SD_B1_05 which reduces the FlexSPI clock from max. 133MHz to 60MHz");
                        }
                    }
                }
        #endif
        #if defined PIN_GPIO_SD_12_GPIO2_IO12 && !defined APPLICATION_REQUIRES_GPIO_SD_12
                if (iPortRef == PORT2) {
                    if (ulBit == PIN_GPIO_SD_12_GPIO2_IO12) {
                        if ((FLEX_SPI_MCR0 & FLEX_SPI_MCR0_RXCLKSRC_STROBE) == FLEX_SPI_MCR0_RXCLKSRC_DQS_LOOPBACK) {
                            _EXCEPTION("PIN_GPIO_SD_12_GPIO2_IO12 is being used by FlexSPI mode for its sample clock loopback!!! Use APPLICATION_REQUIRES_GPIO_SD_12 which reduces the FlexSPI clock from max. 133MHz to 60MHz");
                        }
                    }
                }
        #endif
        #if defined PIN_GPIO_EMC_39_GPIO3_IO25 && !defined APPLICATION_REQUIRES_GPIO_EMC_39
                if (iPortRef == PORT3) {
                    if (ulBit == PIN_GPIO_EMC_39_GPIO3_IO25) {
                        if ((SEMC_MCR & SEMC_MCR_DQSMD) != 0) {
                        _EXCEPTION("PIN_GPIO_EMC_39_GPIO3_IO25 is being used by SEMC for its dummy read strobe loopback!!! Use APPLICATION_REQUIRES_GPIO_EMC_39 which reduces the SEMC clock from max. 133MHz to 60MHz");
                        }
                    }
                }
        #endif
    #endif
            }
            *ptrChars = ulCharacteristics;                               // set the GPIO characteristics for each port pin
            ulPortBits &= ~(ulBit);                                      // pin has been set
        }
    #if defined iMX_RT1011                                               // mux and pad registers are in reversed order
        ptrGPIO--;
        ptrChars--;
    #else
        ptrGPIO++;
        ptrChars++;
    #endif
        ulBit <<= 1;
    #if defined iMX_RT1011
        if (ulBit > ulMaxPortBit) {                                      // if there are no further bits available on this port
            _SIM_PER_CHANGE;
            return;
        }
        if ((iPortChange != 0) && (ulBit == PORT1_BIT14)) {              // exception for port 1, which moves from GPIO pads to GPIO_AD pads
            iPortChange = 0;
            ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_00_ADD;
            ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_00_ADD;
        }
    #elif defined iMX_RT105X || defined iMX_RT106X
        if (iPortChange != 0) {
            if (ulBit == PORT3_BIT12) {                                  // exception for port 3, which moves from SD_B1 pads to SD B0 pads and then EMC pads
                ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00_ADD;
                ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_00_ADD;
            }
            else if (ulBit == PORT3_BIT18) {
                ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32_ADD;
                ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32_ADD;
            }
            else if (ulBit == PORT3_BIT28) {                             // GPIO3-28..GPIO3-31 don't exist
                break;
            }
        }
    #else
        if ((iPortChange != 0) && (ulBit == PORT3_BIT13)) {              // exception for port 3, which moves from EMC pads to SD pads
            iPortChange = 0;
            ptrGPIO = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00_ADD;
            ptrChars = IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_00_ADD;
        }
    #endif
    }
    _SIM_PER_CHANGE;
}
#endif

#if defined _PORT_INTERRUPT_CODE
    #if !defined NO_PORT_INTERRUPTS_PORT1                                // if port 1 support has not been removed
        #if defined PORT_INTERRUPT_USER_DISPATCHER
static void (*gpio_handlers_1)(int) = 0;                                 // a single handler for port 1
        #else
static void (*gpio_handlers_1[PORT_WIDTH])(void) = {0};                  // a handler for each possible port 1 pin
        #endif
    #endif
    #if !defined NO_PORT_INTERRUPTS_PORT2                                // if port 2 support has not been removed
        #if defined PORT_INTERRUPT_USER_DISPATCHER
static void (*gpio_handlers_2)(int) = 0;                                 // a single handler for port 2
        #else
static void (*gpio_handlers_2[PORT_WIDTH])(void) = {0};                  // a handler for each possible port 2 pin
        #endif
    #endif
    #if defined PORT3 && !defined NO_PORT_INTERRUPTS_PORT3               // if port 3 support has not been removed
        #if defined PORT_INTERRUPT_USER_DISPATCHER
static void (*gpio_handlers_3)(int) = 0;                                 // a single handler for port 3
        #else
static void (*gpio_handlers_3[PORT_WIDTH])(void) = {0};                  // a handler for each possible port 3 pin
        #endif
    #endif
    #if defined PORT4 && !defined NO_PORT_INTERRUPTS_PORT4               // if port 4 support has not been removed
        #if defined PORT_INTERRUPT_USER_DISPATCHER
static void (*gpio_handlers_4)(int) = 0;                                 // a single handler for port 4
        #else
static void (*gpio_handlers_4[PORT_WIDTH])(void) = {0};                  // a handler for each possible port 4 pin
        #endif
    #endif
    #if defined PORT5 && !defined NO_PORT_INTERRUPTS_PORT5               // if port 5 support has not been removed
        #if defined PORT_INTERRUPT_USER_DISPATCHER
static void (*gpio_handlers_5)(int) = 0;                                 // a single handler for port 5
        #else
static void (*gpio_handlers_5[PORT_WIDTH])(void) = {0};                  // a handler for each possible port 5 pin
        #endif
    #endif

    #if !defined NO_PORT_INTERRUPTS_PORT1                                // if port 1 support has not been removed
        #if defined irq_GPIO1_Hi_0_ID
// Interrupt routine called to handle port 1 input interrupt on bit 0
//
static __interrupt void _port_1_0_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_0);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(0);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[0]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_0);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 1
//
static __interrupt void _port_1_1_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_1);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(1);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[1]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_1_INT_MASK) < PORT_ICR1_1_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_1);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 2
//
static __interrupt void _port_1_2_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_2);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(2);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[2]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_2_INT_MASK) < PORT_ICR1_2_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_2);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 3
//
static __interrupt void _port_1_3_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_3);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(3);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[3]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_3_INT_MASK) < PORT_ICR1_3_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_3);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 4
//
static __interrupt void _port_1_4_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_4);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(4);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[4]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_4_INT_MASK) < PORT_ICR1_4_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_4);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 5
//
static __interrupt void _port_1_5_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_5);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(5);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[5]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_5_INT_MASK) < PORT_ICR1_5_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_5);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 6
//
static __interrupt void _port_1_6_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_6);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
#if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(6);                                              // call the application handler (this is expected to clear level sensitive input sources)
#else
        gpio_handlers_1[6]();                                            // call the application handler (this is expected to clear level sensitive input sources)
#endif
        if ((GPIO1_ICR1 & PORT_ICR1_6_INT_MASK) < PORT_ICR1_6_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_6);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}

// Interrupt routine called to handle port 1 input interrupt on bit 7
//
static __interrupt void _port_1_7_isr(void)
{
    WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_7);                       // reset the edge sensitive interrupt that will be handled
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when user callback operates
            #if defined PORT_INTERRUPT_USER_DISPATCHER
        gpio_handlers_1(7);                                              // call the application handler (this is expected to clear level sensitive input sources)
            #else
        gpio_handlers_1[7]();                                            // call the application handler (this is expected to clear level sensitive input sources)
            #endif
        if ((GPIO1_ICR1 & PORT_ICR1_7_INT_MASK) < PORT_ICR1_7_RISING) {  // if level sensitive type
            WRITE_ONE_TO_CLEAR(GPIO1_ISR, GPIO_ISR_ISR_7);               // attempt to clear the level sensitive interrupt "after" the user has serviced it
        }
    uEnable_Interrupt();
}
        #endif
// Interrupt routine called to handle port 1 input interrupts (dedicated to port 1 bits 8..15 or 0..15 if irq_GPIO1_0_15_ID is valid)
//
static __interrupt void _port_1_low_isr(void)
{
        #if defined irq_GPIO1_0_15_ID                                    // vector for 0..15
            #define FIRST_PORT_1_INTERRUPT       0
            #define FIRST_PORT_1_INTERRUPT_MASK  0x0000ffff
            #define FIRST_PORT_1_INTERRUPT_BIT   0x00000001
        #else                                                            // vector for 8..15
            #define FIRST_PORT_1_INTERRUPT       8
            #define FIRST_PORT_1_INTERRUPT_MASK  0x0000ff00
            #define FIRST_PORT_1_INTERRUPT_BIT   0x00000100
        #endif
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO1_ISR & FIRST_PORT_1_INTERRUPT_MASK) & GPIO1_IMR)) != 0) { // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO1_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = FIRST_PORT_1_INTERRUPT_BIT;
        iInterrupt = FIRST_PORT_1_INTERRUPT;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_1(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_1[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO1_ICR1 >> (iInterrupt * 2)) & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO1_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
        #if defined _WINDOWS
                    GPIO1_ISR &= ~ulPortBit;
        #endif
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}

// Interrupt routine called to handle port 1 input interrupts (dedicated to port 1 bits 16..31)
//
static __interrupt void _port_1_high_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO1_ISR & 0xffff0000) & GPIO1_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO1_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00010000;
        iInterrupt = 16;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_1(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_1[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO1_ICR2 >> (iInterrupt * 2)) & PORT_ICR2_16_INT_MASK) < PORT_ICR2_16_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO1_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}
    #endif
    #if defined PORT2 &&  !defined NO_PORT_INTERRUPTS_PORT2              // if port 2 support has not been removed
// Interrupt routine called to handle port 2 input interrupts (dedicated to port 2 bits 0..15)
//
static __interrupt void _port_2_low_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO2_ISR & 0x0000ffff) & GPIO2_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO2_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00000001;
        iInterrupt = 0;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_2(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_2[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO2_ICR1 >> (iInterrupt * 2)) & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO2_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}

        #if defined irq_GPIO2_16_31_ID
// Interrupt routine called to handle port 2 input interrupts (dedicated to port 2 bits 16..31)
//
static __interrupt void _port_2_high_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO2_ISR & 0xffff0000) & GPIO2_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO2_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00010000;
        iInterrupt = 16;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_2(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_2[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO2_ICR2 >> (iInterrupt * 2)) & PORT_ICR2_16_INT_MASK) < PORT_ICR2_16_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO2_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}
        #endif
    #endif
    #if defined PORT3 && !defined NO_PORT_INTERRUPTS_PORT3               // if port 3 support has not been removed
// Interrupt routine called to handle port 3 input interrupts (dedicated to port 3 bits 0..15)
//
static __interrupt void _port_3_low_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO3_ISR & 0x0000ffff) & GPIO3_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO3_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00000001;
        iInterrupt = 0;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_3(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_3[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO3_ICR1 >> (iInterrupt * 2)) & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO3_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}

// Interrupt routine called to handle port 3 input interrupts (dedicated to port 3 bits 16..31)
//
static __interrupt void _port_3_high_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO3_ISR & 0xffff0000) & GPIO3_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO3_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00010000;
        iInterrupt = 16;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                         // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_3(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_3[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO3_ICR2 >> (iInterrupt * 2)) & PORT_ICR2_16_INT_MASK) < PORT_ICR2_16_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO3_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}
    #endif
    #if defined PORT4 && !defined NO_PORT_INTERRUPTS_PORT4               // if port 4 support has not been removed
// Interrupt routine called to handle port 4 input interrupts (dedicated to port 4 bits 0..15)
//
static __interrupt void _port_4_low_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO4_ISR & 0x0000ffff) & GPIO4_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO4_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00000001;
        iInterrupt = 0;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_4(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_4[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO4_ICR1 >> (iInterrupt * 2)) & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO4_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}

// Interrupt routine called to handle port 4 input interrupts (dedicated to port 4 bits 16..31)
//
static __interrupt void _port_4_high_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO4_ISR & 0xffff0000) & GPIO4_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO4_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00010000;
        iInterrupt = 16;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                         // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_4(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_4[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO4_ICR2 >> (iInterrupt * 2)) & PORT_ICR2_16_INT_MASK) < PORT_ICR2_16_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO4_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}
    #endif
    #if defined PORT5 && !defined NO_PORT_INTERRUPTS_PORT5               // if port 5 support has not been removed
// Interrupt routine called to handle port 5 input interrupts (dedicated to port 5 bits 0..15)
//
static __interrupt void _port_5_low_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO5_ISR & 0x0000ffff) & GPIO5_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO5_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00000001;
        iInterrupt = 0;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                          // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_5(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_5[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO5_ICR1 >> (iInterrupt * 2)) & PORT_ICR1_0_INT_MASK) < PORT_ICR1_0_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO5_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}

        #if defined irq_GPIO5_16_31_ID
// Interrupt routine called to handle port 5 input interrupts (dedicated to port 5 bits 16..31)
//
static __interrupt void _port_5_high_isr(void)
{
    unsigned long ulSources;
    unsigned long ulPortBit;
    int iInterrupt;
    while ((ulSources = ((GPIO5_ISR & 0xffff0000) & GPIO5_IMR)) != 0) {  // read which enabled pins are generating interrupts
        WRITE_ONE_TO_CLEAR(GPIO5_ISR, ulSources);                        // reset the edge sensitive ones that will be handled
        ulPortBit = 0x00010000;
        iInterrupt = 16;
        while (ulSources != 0) {
            if ((ulSources & ulPortBit) != 0) {                         // pending interrupt detected on this input
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when user callback operates
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                    gpio_handlers_5(iInterrupt);                         // call the application handler (this is expected to clear level sensitive input sources)
        #else
                    gpio_handlers_5[iInterrupt]();                       // call the application handler (this is expected to clear level sensitive input sources)
        #endif
                    if (((GPIO5_ICR2 >> (iInterrupt * 2)) & PORT_ICR2_16_INT_MASK) < PORT_ICR2_16_RISING) { // if level sensitive type
                        WRITE_ONE_TO_CLEAR(GPIO5_ISR, ulPortBit);        // attempt to clear the level sensitive interrupt "after" the user has serviced it
                    }
                    ulSources &= ~ulPortBit;                             // clear the processed port pin interrupt
                uEnable_Interrupt();
            }
            ulPortBit <<= 1;
            iInterrupt++;
        }
    }
}
        #endif
    #endif

static void fnProgramInterruptSensitivity(iMX_GPIO *ptrGPIO, unsigned long ulChars, unsigned long ulBit, int port_bit)
{
    ptrGPIO->GPIO_IMR &= ~(ulBit);                                       // disable the pin interrupt (to avoid generating an interrupt if sensitivity is changed while still enabled)
    if ((ulChars & PORT_ICR_DISABLE) != 0) {
        return;
    }
    else {
        // Program the interrupt sensitivity
        //
        if ((PORT_ICR_BOTH & ulChars) != 0) {
            ptrGPIO->GPIO_EDGE_SEL |= (GPIO_EDGE_SEL_0 << port_bit);     // both edges
        }
        else {
            ptrGPIO->GPIO_EDGE_SEL &= ~(GPIO_EDGE_SEL_0 << port_bit);
            if (port_bit >= 16) {
                ptrGPIO->GPIO_ICR2 &= ~(PORT_ICR2_16_INT_MASK << ((port_bit - 16) * 2));
                ptrGPIO->GPIO_ICR2 |= ((ulChars & PORT_ICR2_16_INT_MASK) << ((port_bit - 16) * 2));
            }
            else {
                ptrGPIO->GPIO_ICR1 &= ~(PORT_ICR1_0_INT_MASK << (port_bit * 2));
                ptrGPIO->GPIO_ICR1 |= ((ulChars & PORT_ICR1_0_INT_MASK) << (port_bit * 2));
            }
        }
        WRITE_ONE_TO_CLEAR(ptrGPIO->GPIO_ISR, ulBit);                    // clear possible pending interrupt
        ptrGPIO->GPIO_IMR |= (ulBit);                                    // enable the pin interrupt
    }
}

// This routine enters the user handler for a port interrupt. The handler can be assigned to multiple inputs
//
static void fnEnterPortInterruptHandler(INTERRUPT_SETUP *port_interrupt, unsigned long ulChars)
{
    register unsigned long ulPortBits = port_interrupt->int_port_bits;
    register int ucPortRef = (int)(port_interrupt->int_port);
    unsigned long ulBit = 0x00000001;
    int port_bit = 0;
    while (ulPortBits != 0) {                                            // for each enabled port bit
        if ((ulPortBits & ulBit) != 0) {                                 // if this port bit is enabled
            switch (ucPortRef) {                                         // switch on the port
    #if !defined NO_PORT_INTERRUPTS_PORT1                                // if port 1 support has not been removed
            case PORT1:                                                  // port 1
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                if ((gpio_handlers_1 = port_interrupt->int_handler) != 0)// enter the application handler
        #else
                if ((gpio_handlers_1[port_bit] = port_interrupt->int_handler) != 0) // enter the application handler
        #endif
                {
                    switch (ulBit) {
        #if defined irq_GPIO1_Hi_0_ID
                    case PORT1_BIT0:
                        fnEnterInterrupt(irq_GPIO1_Hi_0_ID, port_interrupt->int_priority, _port_1_0_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT1:
                        fnEnterInterrupt(irq_GPIO1_Hi_1_ID, port_interrupt->int_priority, _port_1_1_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT2:
                        fnEnterInterrupt(irq_GPIO1_Hi_2_ID, port_interrupt->int_priority, _port_1_2_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT3:
                        fnEnterInterrupt(irq_GPIO1_Hi_3_ID, port_interrupt->int_priority, _port_1_3_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT4:
                        fnEnterInterrupt(irq_GPIO1_Hi_4_ID, port_interrupt->int_priority, _port_1_4_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT5:
                        fnEnterInterrupt(irq_GPIO1_Hi_5_ID, port_interrupt->int_priority, _port_1_5_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT6:
                        fnEnterInterrupt(irq_GPIO1_Hi_6_ID, port_interrupt->int_priority, _port_1_6_isr); // ensure that the handler for this pin is entered
                        break;
                    case PORT1_BIT7:
                        fnEnterInterrupt(irq_GPIO1_Hi_7_ID, port_interrupt->int_priority, _port_1_7_isr); // ensure that the handler for this pin is entered
                        break;
        #endif
                    default:
                        if (ulBit >= 0x00010000) {
                            fnEnterInterrupt(irq_GPIO1_16_31_ID, port_interrupt->int_priority, _port_1_high_isr); // ensure that the handler for this port is entered
                        }
                        else {
                            fnEnterInterrupt(irq_GPIO1_0_15_ID, port_interrupt->int_priority, _port_1_low_isr); // ensure that the handler for this port is entered
                        }
                    }
                }
                else {
                    ulChars |= PORT_ICR_DISABLE;
                }
                fnProgramInterruptSensitivity((iMX_GPIO *)GPIO1_BLOCK, ulChars, ulBit, port_bit);
                if ((port_interrupt->int_port_sense & PORT_KEEP_PERIPHERAL) == 0) { // if the interrupt is being added to a peripheral function
                    _CONFIG_PORT_INPUT(1, ulBit, ulChars);               // configure the port bit as input with the required interrupt sensitivity and characteristics
                }
                break;
    #endif
    #if defined PORT2 && !defined NO_PORT_INTERRUPTS_PORT2               // if port 2 support has not been removed
            case PORT2:                                                  // port 2
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                if ((gpio_handlers_2 = port_interrupt->int_handler) != 0)// enter the application handler
        #else
                if ((gpio_handlers_2[port_bit] = port_interrupt->int_handler) != 0) // {enter the application handler
        #endif
                {
                    if (ulBit >= 0x00010000) {                           // GPIO2-IO16..31
        #if defined irq_GPIO2_16_31_ID
                        fnEnterInterrupt(irq_GPIO2_16_31_ID, port_interrupt->int_priority, _port_2_high_isr); // ensure that the handler for this port is entered
        #endif
                    }
                    else {                                               // GPIO2-IO00..15
                        fnEnterInterrupt(irq_GPIO2_0_15_ID, port_interrupt->int_priority, _port_2_low_isr); // ensure that the handler for this port is entered
                    }
                }
                else {
                    ulChars |= PORT_ICR_DISABLE;
                }
                fnProgramInterruptSensitivity((iMX_GPIO *)GPIO2_BLOCK, ulChars, ulBit, port_bit); // configure the required interrupt sensitivity
                if ((port_interrupt->int_port_sense & PORT_KEEP_PERIPHERAL) == 0) { // if the interrupt is being added to a peripheral function
                    _CONFIG_PORT_INPUT(2, ulBit, (ulChars & (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PUS_MASK | IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_PKE)));               // configure the port bit as input with the required characteristics (pull-up/down, hysterisis)
                }
                break;
    #endif
    #if defined PORT3 && !defined NO_PORT_INTERRUPTS_PORT3               // if port 3 support has not been removed
            case PORT3:                                                  // port 3
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                if ((gpio_handlers_3 = port_interrupt->int_handler) != 0)// enter the application handler
        #else
                if ((gpio_handlers_3[port_bit] = port_interrupt->int_handler) != 0) // {enter the application handler
        #endif
                {
                    if (ulBit >= 0x00010000) {
                        fnEnterInterrupt(irq_GPIO3_16_31_ID, port_interrupt->int_priority, _port_3_high_isr); // ensure that the handler for this port is entered
                    }
                    else {
                        fnEnterInterrupt(irq_GPIO3_0_15_ID, port_interrupt->int_priority, _port_3_low_isr); // ensure that the handler for this port is entered
                    }
                }
                else {
                    ulChars |= PORT_ICR_DISABLE;
                }
                fnProgramInterruptSensitivity((iMX_GPIO *)GPIO3_BLOCK, ulChars, ulBit, port_bit);
                if ((port_interrupt->int_port_sense & PORT_KEEP_PERIPHERAL) == 0) { // if the interrupt is being added to a peripheral function
                    _CONFIG_PORT_INPUT(3, ulBit, ulChars);               // configure the port bit as input with the required interrupt sensitivity and characteristics
                }
                break;
    #endif
    #if defined PORT4 && !defined NO_PORT_INTERRUPTS_PORT4               // if port 4 support has not been removed
            case PORT4:                                                  // port 4
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                if ((gpio_handlers_4 = port_interrupt->int_handler) != 0)// enter the application handler
        #else
                if ((gpio_handlers_4[port_bit] = port_interrupt->int_handler) != 0) // {enter the application handler
        #endif
                {
                    if (ulBit >= 0x00010000) {
                        fnEnterInterrupt(irq_GPIO4_16_31_ID, port_interrupt->int_priority, _port_4_high_isr); // ensure that the handler for this port is entered
                    }
                    else {
                        fnEnterInterrupt(irq_GPIO4_0_15_ID, port_interrupt->int_priority, _port_4_low_isr); // ensure that the handler for this port is entered
                    }
                }
                else {
                    ulChars |= PORT_ICR_DISABLE;
                }
                fnProgramInterruptSensitivity((iMX_GPIO *)GPIO4_BLOCK, ulChars, ulBit, port_bit);
                if ((port_interrupt->int_port_sense & PORT_KEEP_PERIPHERAL) == 0) { // if the interrupt is being added to a peripheral function
                    _CONFIG_PORT_INPUT(4, ulBit, ulChars);               // configure the port bit as input with the required interrupt sensitivity and characteristics
                }
                break;
    #endif
    #if defined PORT5 && !defined NO_PORT_INTERRUPTS_PORT5               // if port 5 support has not been removed
            case PORT5:                                                  // port 5
        #if defined PORT_INTERRUPT_USER_DISPATCHER
                if ((gpio_handlers_5 = port_interrupt->int_handler) != 0)// enter the application handler
        #else
                if ((gpio_handlers_5[port_bit] = port_interrupt->int_handler) != 0) // {enter the application handler
        #endif
                {
                    if (ulBit >= 0x00010000) {
        #if defined irq_GPIO5_16_31_ID
                        fnEnterInterrupt(irq_GPIO5_16_31_ID, port_interrupt->int_priority, _port_5_high_isr); // ensure that the handler for this port is entered
        #endif
                    }
                    else {
                        fnEnterInterrupt(irq_GPIO5_0_15_ID, port_interrupt->int_priority, _port_5_low_isr); // ensure that the handler for this port is entered
                    }
                }
                else {
                    ulChars |= PORT_ICR_DISABLE;
                }
                fnProgramInterruptSensitivity((iMX_GPIO *)GPIO5_BLOCK, ulChars, ulBit, port_bit);
                if ((port_interrupt->int_port_sense & PORT_KEEP_PERIPHERAL) == 0) { // if the interrupt is being added to a peripheral function
                    _CONFIG_PORT_INPUT(5, ulBit, ulChars);               // configure the port bit as input with the required interrupt sensitivity and characteristics
                }
                break;
    #endif
            default:
                _EXCEPTION("Warning - port for interrupt not enabled (check also that this port interrupt is possible on the particular device being used)!!");
                return;
            }
            ulPortBits &= ~ulBit;
        }
        ulBit <<= 1;
        port_bit++;
    }
    _SIM_PORT_CHANGE(-1);
}
#endif

#if defined _PORT_INT_CONFIG_CODE
        {
            INTERRUPT_SETUP *port_interrupt = (INTERRUPT_SETUP *)ptrSettings;
            unsigned long ulCharacteristics = PORT_ICR_DISABLE;          // default is to disable interrupts
    #if defined SUPPORT_XBAR
            unsigned short usDMA_config = 0;
    #endif
            if ((port_interrupt->int_port_sense & IRQ_DISABLE_INT) == 0) { // if the interrupt is not being disabled
                if ((port_interrupt->int_port_sense & IRQ_LOW_LEVEL) != 0) {
                    ulCharacteristics = PORT_ICR_LOW_LEVEL;
                }
                else if ((port_interrupt->int_port_sense & IRQ_HIGH_LEVEL) != 0) {
                    ulCharacteristics = PORT_ICR_HIGH_LEVEL;
                }
                else if ((port_interrupt->int_port_sense & IRQ_RISING_EDGE) != 0) {
                    if ((port_interrupt->int_port_sense & IRQ_FALLING_EDGE) != 0) {
                        ulCharacteristics = PORT_ICR_BOTH;
    #if defined SUPPORT_XBAR
                        usDMA_config = (XBARA_CTRL1_DEN3 | XBARA_CTRL1_EDGE3_BOTH | XBARA_CTRL1_STS3);
    #endif
                    }
                    else {
                        ulCharacteristics = PORT_ICR_RISING;
    #if defined SUPPORT_XBAR
                        usDMA_config = (XBARA_CTRL1_DEN3 | XBARA_CTRL1_EDGE3_RISE | XBARA_CTRL1_STS3);
    #endif
                    }
                }
                else if ((port_interrupt->int_port_sense & IRQ_FALLING_EDGE) != 0) {
                    ulCharacteristics = PORT_ICR_FALLING;
    #if defined SUPPORT_XBAR
                    usDMA_config = (XBARA_CTRL1_DEN3 | XBARA_CTRL1_EDGE3_FALL | XBARA_CTRL1_STS3);
    #endif
                }
            }
            if ((port_interrupt->int_port_sense & PULLUP_ON) != 0) {
                ulCharacteristics |= (PORT_PS_UP_ENABLE);                // enable pull-up resistor on the input
            }
            else if ((port_interrupt->int_port_sense & PULLDOWN_ON) != 0) {
                ulCharacteristics |= (PORT_PS_DOWN_ENABLE);              // enable pull-down resistor on the input
            }
            if ((port_interrupt->int_port_sense & HYSTERESIS_ON) != 0) {
                ulCharacteristics |= (PORT_WITH_HYSTERESIS);             // enable hysteresis on the input
            }
            if ((port_interrupt->int_port_sense & PORT_DMA_MODE) != 0) { // DMA trigger rather than interrupt
    #if defined SUPPORT_XBAR
                unsigned char ucXbarInput = 0;
                unsigned long ulInputConfig = (ulCharacteristics & (PORT_WITH_HYSTERESIS | PORT_PS_DOWN_ENABLE | PORT_PS_UP_ENABLE));
                POWER_UP_ATOMIC(2, XBAR1_CLOCK);                         // ensure that XBAR1 is powered
        #if defined MIMXRT1010
                if (port_interrupt->int_port != PORT1) {
                    _EXCEPTION("DMA trigger only possible on port 1!");
                    return;
                }
                switch (port_interrupt->int_port_bits) {
                case PORT1_BIT28:
                    IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_2); // ensure XBAR_INOUT02 is an input
                    _CONFIG_PERIPHERAL(GPIO_AD_14, XBAR1_INOUT02, ulInputConfig); // select XBAR_INOUT02 on GPIOMUX-28 (GPIO_AD_14) alt. function 7
                    ucXbarInput = XBAR1_IOMUX_XBAR_INOUT02_IN;
                    break;
                case PORT1_BIT21:
                    IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_3);     // ensure XBAR_INOUT03 is an input
                    _CONFIG_PERIPHERAL(GPIO_AD_07, XBAR1_INOUT03, ulInputConfig); // select XBAR_INOUT03 on GPIOMUX-21 (GPIO_AD_07) alt. function 7
                    ucXbarInput = XBAR1_IOMUX_XBAR_INOUT03_IN;
                    break;
                default:
                    _EXCEPTION("DMA trigger not supported on defined pin!");
                    return;
                }
        #else
                if (port_interrupt->int_port == PORT2) {
                    switch (port_interrupt->int_port_bits) {
            #if defined iMX_RT105X || defined iMX_RT106X
                    case PORT2_BIT12:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_10); // ensure XBAR_INOUT10 is an input
                        _CONFIG_PERIPHERAL(GPIO_B0_12, XBAR1_INOUT10, ulInputConfig); // select XBAR_INOUT10 on GPIO2_IO12 (GPIO_B0_12) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT10_IN;
                        break;
                    case PORT2_BIT13:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_11); // ensure XBAR_INOUT11 is an input
                        _CONFIG_PERIPHERAL(GPIO_B0_13, XBAR1_INOUT11, ulInputConfig); // select XBAR_INOUT11 on GPIO2_IO13 (GPIO_B0_13) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT11_IN;
                        break;
                    case PORT2_BIT14:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12); // ensure XBAR_INOUT12 is an input
                        _CONFIG_PERIPHERAL(GPIO_B0_14, XBAR1_INOUT12, ulInputConfig); // select XBAR_INOUT12 on GPIO2_IO14 (GPIO_B0_14) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT12_IN;
                        break;
                    case PORT2_BIT15:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13); // ensure XBAR_INOUT13 is an input
                        _CONFIG_PERIPHERAL(GPIO_B0_15, XBAR1_INOUT13, ulInputConfig); // select XBAR_INOUT13 on GPIO2_IO15 (GPIO_B0_15) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT13_IN;
                        break;
                    case PORT2_BIT16:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14); // ensure XBAR_INOUT14 is an input
                        IOMUXC_XBAR1_IN14_SELECT_INPUT = IOMUXC_XBAR1_IN14_SELECT_INPUT_GPIO_B1_00_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_B1_00, XBAR1_INOUT14, ulInputConfig); // select XBAR_INOUT14 on GPIO2_IO16 (GPIO_B1_00) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT14_IN;
                        break;
                    case PORT2_BIT17:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15); // ensure XBAR_INOUT15 is an input
                        IOMUXC_XBAR1_IN15_SELECT_INPUT = IOMUXC_XBAR1_IN15_SELECT_INPUT_GPIO_B1_01_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_B1_01, XBAR1_INOUT15, ulInputConfig); // select XBAR_INOUT15 on GPIO2_IO17 (GPIO_B1_01) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT15_IN;
                        break;
                    case PORT2_BIT18:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16); // ensure XBAR_INOUT16 is an input
                        IOMUXC_XBAR1_IN16_SELECT_INPUT = IOMUXC_XBAR1_IN16_SELECT_INPUT_GPIO_B1_02_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_B1_02, XBAR1_INOUT16, ulInputConfig); // select XBAR_INOUT16 on GPIO2_IO18 (GPIO_B1_02) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT16_IN;
                        break;
                    case PORT2_BIT19:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT17 is an input
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_B1_03_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_B1_03, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO2_IO19 (GPIO_B1_03) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                        break;
            #else
                    case PORT2_BIT4:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4); // ensure XBAR_INOUT04 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_04, XBAR1_INOUT04, ulInputConfig); // select XBAR_INOUT04 on GPIO2_IO04 (GPIO_EMC_04) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT04_IN;
                        break;
                    case PORT2_BIT5:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5); // ensure XBAR_INOUT05 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_05, XBAR1_INOUT05, ulInputConfig); // select XBAR_INOUT05 on GPIO2_IO05 (GPIO_EMC_05) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT05_IN;
                        break;
                    case PORT2_BIT6:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6); // ensure XBAR_INOUT06 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_06, XBAR1_INOUT06, ulInputConfig); // select XBAR_INOUT06 on GPIO2_IO06 (GPIO_EMC_06) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT06_IN;
                        break;
                    case PORT2_BIT7:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7); // ensure XBAR_INOUT07 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_07, XBAR1_INOUT07, ulInputConfig); // select XBAR_INOUT07 on GPIO2_IO07 (GPIO_EMC_07) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT07_IN;
                        break;
                    case PORT2_BIT8:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8); // ensure XBAR_INOUT08 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_08, XBAR1_INOUT08, ulInputConfig); // select XBAR_INOUT08 on GPIO2_IO08 (GPIO_EMC_08) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT08_IN;
                        break;
                    case PORT2_BIT9:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9); // ensure XBAR_INOUT09 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_09, XBAR1_INOUT09, ulInputConfig); // select XBAR_INOUT09 on GPIO2_IO09 (GPIO_EMC_09) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT09_IN;
                        break;
                    case PORT2_BIT10:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_10); // ensure XBAR_INOUT10 is an input
            #if !defined iMX_RT1015
                        IOMUXC_XBAR1_IN10_SELECT_INPUT = IOMUXC_XBAR1_IN10_SELECT_INPUT_GPIO_EMC_10_ALT1;
            #endif
                        _CONFIG_PERIPHERAL(GPIO_EMC_10, XBAR1_INOUT10, ulInputConfig); // select XBAR_INOUT10 on GPIO2_IO10 (GPIO_EMC_10) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT10_IN;
                        break;
            #if !defined iMX_RT1015
                    case PORT2_BIT11:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_11); // ensure XBAR_INOUT11 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_11, XBAR1_INOUT11, ulInputConfig); // select XBAR_INOUT11 on GPIO2_IO11 (GPIO_EMC_11) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT11_IN;
                        break;
                    case PORT2_BIT12:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12); // ensure XBAR_INOUT12 is an input
                        IOMUXC_XBAR1_IN12_SELECT_INPUT = IOMUXC_XBAR1_IN12_SELECT_INPUT_GPIO_EMC_12_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_12, XBAR1_INOUT12, ulInputConfig); // select XBAR_INOUT12 on GPIO2_IO12 (GPIO_EMC_12) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT12_IN;
                        break;
                    case PORT2_BIT13:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13); // ensure XBAR_INOUT13 is an input
                        IOMUXC_XBAR1_IN13_SELECT_INPUT = IOMUXC_XBAR1_IN13_SELECT_INPUT_GPIO_EMC_13_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_13, XBAR1_INOUT13, ulInputConfig); // select XBAR_INOUT13 on GPIO2_IO13 (GPIO_EMC_13) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT13_IN;
                        break;
                    case PORT2_BIT14:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14); // ensure XBAR_INOUT14 is an input
                        IOMUXC_XBAR1_IN14_SELECT_INPUT = IOMUXC_XBAR1_IN14_SELECT_INPUT_GPIO_EMC_14_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_14, XBAR1_INOUT14, ulInputConfig); // select XBAR_INOUT14 on GPIO2_IO14 (GPIO_EMC_14) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT14_IN;
                        break;
                    case PORT2_BIT15:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15); // ensure XBAR_INOUT15 is an input
                        IOMUXC_XBAR1_IN15_SELECT_INPUT = IOMUXC_XBAR1_IN15_SELECT_INPUT_GPIO_EMC_15_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_15, XBAR1_INOUT15, ulInputConfig); // select XBAR_INOUT15 on GPIO2_IO15 (GPIO_EMC_15) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT15_IN;
                        break;
            #endif
                    case PORT2_BIT18:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16); // ensure XBAR_INOUT16 is an input
            #if !defined iMX_RT1015
                        IOMUXC_XBAR1_IN16_SELECT_INPUT = IOMUXC_XBAR1_IN16_SELECT_INPUT_GPIO_EMC_18_ALT1;
            #endif
                        _CONFIG_PERIPHERAL(GPIO_EMC_18, XBAR1_INOUT16, ulInputConfig); // select XBAR_INOUT16 on GPIO2_IO18 (GPIO_EMC_18) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT16_IN;
                        break;
                    case PORT2_BIT19:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT17 is an input
            #if !defined iMX_RT1015
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_EMC_19_ALT1;
            #endif
                        _CONFIG_PERIPHERAL(GPIO_EMC_19, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO2_IO19 (GPIO_EMC_19) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                        break;
            #if !defined iMX_RT1015
                    case PORT2_BIT28:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18); // ensure XBAR_INOUT18 is an input
                        IOMUXC_XBAR1_IN18_SELECT_INPUT = IOMUXC_XBAR1_IN18_SELECT_INPUT_GPIO_EMC_28_ALT2;
                        _CONFIG_PERIPHERAL(GPIO_EMC_28, XBAR1_INOUT18, ulInputConfig); // select XBAR_INOUT18 on GPIO2_IO28 (GPIO_EMC_28) alt. function 2
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT18_IN;
                        break;
                    case PORT2_BIT29:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19); // ensure XBAR_INOUT19 is an input
                        IOMUXC_XBAR1_IN19_SELECT_INPUT = IOMUXC_XBAR1_IN19_SELECT_INPUT_GPIO_EMC_29_ALT2;
                        _CONFIG_PERIPHERAL(GPIO_EMC_29, XBAR1_INOUT19, ulInputConfig); // select XBAR_INOUT19 on GPIO2_IO29 (GPIO_EMC_29) alt. function 2
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT19_IN;
                        break;
            #endif
            #endif
                    default:
                        _EXCEPTION("DMA trigger not supported on defined pin!");
                        return;
                    }
                }
            #if !defined iMX_RT1015
                else if (port_interrupt->int_port == PORT1) {
                    switch (port_interrupt->int_port_bits) {
                #if defined iMX_RT105X || defined iMX_RT106X
                    case PORT1_BIT0:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14); // ensure XBAR_INOUT14 is an input
                        IOMUXC_XBAR1_IN14_SELECT_INPUT = IOMUXC_XBAR1_IN14_SELECT_INPUT_GPIO_AD_B0_00_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_00, XBAR1_INOUT14, ulInputConfig); // select XBAR_INOUT14 on GPIO1_IO00 (GPIO_AD_B0_00) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT14_IN;
                        break;
                    case PORT1_BIT1:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15); // ensure XBAR_INOUT15 is an input
                        IOMUXC_XBAR1_IN15_SELECT_INPUT = IOMUXC_XBAR1_IN15_SELECT_INPUT_GPIO_AD_B0_01_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_01, XBAR1_INOUT15, ulInputConfig); // select XBAR_INOUT15 on GPIO1_IO01 (GPIO_AD_B0_01) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT15_IN;
                        break;
                    case PORT1_BIT2:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16); // ensure XBAR_INOUT16 is an input
                        IOMUXC_XBAR1_IN16_SELECT_INPUT = IOMUXC_XBAR1_IN16_SELECT_INPUT_GPIO_AD_B0_02_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_02, XBAR1_INOUT16, ulInputConfig); // select XBAR_INOUT16 on GPIO1_IO02 (GPIO_AD_B0_02) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT16_IN;
                        break;
                    case PORT1_BIT3:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT17 is an input
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_AD_B0_03_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_03, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO1_IO03 (GPIO_AD_B0_03) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                        break;
                    case PORT1_BIT5:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT17 is an input
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_AD_B0_05_ALT6;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_05, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO1_IO05 (GPIO_AD_B0_05) alt. function 6
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                        break;
                    case PORT1_BIT6:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18); // ensure XBAR_INOUT18 is an input
                        IOMUXC_XBAR1_IN18_SELECT_INPUT = IOMUXC_XBAR1_IN18_SELECT_INPUT_GPIO_AD_B0_06_ALT6;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_06, XBAR1_INOUT18, ulInputConfig); // select XBAR_INOUT18 on GPIO1_IO06 (GPIO_AD_B0_06) alt. function 6
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT18_IN;
                        break;
                    case PORT1_BIT7:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19); // ensure XBAR_INOUT19 is an input
                        IOMUXC_XBAR1_IN19_SELECT_INPUT = IOMUXC_XBAR1_IN19_SELECT_INPUT_GPIO_AD_B0_07_ALT6;
                        _CONFIG_PERIPHERAL(GPIO_AD_B0_07, XBAR1_INOUT19, ulInputConfig); // select XBAR_INOUT19 on GPIO1_IO07 (GPIO_AD_B0_07) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT19_IN;
                        break;
                #else
                    case PORT1_BIT24:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12); // ensure XBAR_INOUT12 is an input
                        IOMUXC_XBAR1_IN12_SELECT_INPUT = IOMUXC_XBAR1_IN12_SELECT_INPUT_GPIO_SD_B1_08_ALT7;
                        _CONFIG_PERIPHERAL(GPIO_AD_B1_08, XBAR1_INOUT12, ulInputConfig); // select XBAR_INOUT12 on GPIO1_IO24 (GPIO_AD_B1_08) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT12_IN;
                        break;
                    case PORT1_BIT25:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13); // ensure XBAR_INOUT13 is an input
                        IOMUXC_XBAR1_IN13_SELECT_INPUT = IOMUXC_XBAR1_IN13_SELECT_INPUT_GPIO_SD_B1_09_ALT7;
                        _CONFIG_PERIPHERAL(GPIO_AD_B1_09, XBAR1_INOUT13, ulInputConfig); // select XBAR_INOUT13 on GPIO1_IO25 (GPIO_AD_B1_09) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT13_IN;
                        break;
                 #endif
                    default:
                        _EXCEPTION("DMA trigger not supported on defined pin!");
                        return;
                    }
                }
                else if (port_interrupt->int_port == PORT3) {
                    switch (port_interrupt->int_port_bits) {
                #if defined iMX_RT105X || defined iMX_RT106X
                    case PORT3_BIT12:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4); // ensure XBAR_INOUT04 is an input
                        IOMUXC_XBAR1_IN04_SELECT_INPUT = IOMUXC_XBAR1_IN04_SELECT_INPUT_GPIO_SD_B0_00_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_00, XBAR1_INOUT04, ulInputConfig); // select XBAR_INOUT04 on GPIO3_IO12 (GPIO_SD_B0_00) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT04_IN;
                        break;
                    case PORT3_BIT13:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5); // ensure XBAR_INOUT05 is an input
                        IOMUXC_XBAR1_IN05_SELECT_INPUT = IOMUXC_XBAR1_IN05_SELECT_INPUT_GPIO_SD_B0_01_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_01, XBAR1_INOUT05, ulInputConfig); // select XBAR_INOUT05 on GPIO3_IO13 (GPIO_SD_B0_01) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT05_IN;
                        break;
                    case PORT3_BIT14:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6); // ensure XBAR_INOUT06 is an input
                        IOMUXC_XBAR1_IN06_SELECT_INPUT = IOMUXC_XBAR1_IN06_SELECT_INPUT_GPIO_SD_B0_02_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_02, XBAR1_INOUT06, ulInputConfig); // select XBAR_INOUT06 on GPIO3_IO14 (GPIO_SD_B0_02) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT06_IN;
                        break;
                    case PORT3_BIT15:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7); // ensure XBAR_INOUT07 is an input
                        IOMUXC_XBAR1_IN07_SELECT_INPUT = IOMUXC_XBAR1_IN07_SELECT_INPUT_GPIO_SD_B0_03_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_03, XBAR1_INOUT07, ulInputConfig); // select XBAR_INOUT07 on GPIO3_IO15 (GPIO_SD_B0_03) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT07_IN;
                        break;
                    case PORT3_BIT16:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8); // ensure XBAR_INOUT08 is an input
                        IOMUXC_XBAR1_IN08_SELECT_INPUT = IOMUXC_XBAR1_IN08_SELECT_INPUT_GPIO_SD_B0_04_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_04, XBAR1_INOUT08, ulInputConfig); // select XBAR_INOUT08 on GPIO3_IO16 (GPIO_SD_B0_04) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT08_IN;
                        break;
                    case PORT3_BIT17:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9); // ensure XBAR_INOUT09 is an input
                        IOMUXC_XBAR1_IN09_SELECT_INPUT = IOMUXC_XBAR1_IN09_SELECT_INPUT_GPIO_SD_B0_05_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_05, XBAR1_INOUT09, ulInputConfig); // select XBAR_INOUT09 on GPIO3_IO17 (GPIO_SD_B0_05) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT09_IN;
                        break;
                    case PORT3_BIT21:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18); // ensure XBAR_INOUT18 is an input
                        _CONFIG_PERIPHERAL(GPIO_EMC_35, XBAR1_INOUT18, ulInputConfig); // select XBAR_INOUT18 on GPIO3_IO21 (GPIO_EMC_35) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT10_IN;
                        break;
                #else
                    case PORT3_BIT20:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_10); // ensure XBAR_INOUT10 is an input
                        IOMUXC_XBAR1_IN10_SELECT_INPUT = IOMUXC_XBAR1_IN10_SELECT_INPUT_GPIO_SD_B1_00_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_SD_B1_00, XBAR1_INOUT10, ulInputConfig); // select XBAR_INOUT10 on GPIO3_IO20 (GPIO_SD_B1_00) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT10_IN;
                        break;
                    case PORT3_BIT13:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14); // ensure XBAR_INOUT14 is an input
                        IOMUXC_XBAR1_IN14_SELECT_INPUT = IOMUXC_XBAR1_IN14_SELECT_INPUT_GPIO_SD_B0_00_ALT7;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_00, XBAR1_INOUT14, ulInputConfig); // select XBAR_INOUT14 on GPIO3_IO13 (GPIO_SD_B0_00) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT14_IN;
                        break;
                    case PORT3_BIT14:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15); // ensure XBAR_INOUT15 is an input
                        IOMUXC_XBAR1_IN15_SELECT_INPUT = IOMUXC_XBAR1_IN15_SELECT_INPUT_GPIO_SD_B0_01_ALT7;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_01, XBAR1_INOUT15, ulInputConfig); // select XBAR_INOUT15 on GPIO3_IO14 (GPIO_SD_B0_01) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT15_IN;
                        break;
                    case PORT3_BIT15:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16); // ensure XBAR_INOUT16 is an input
                        IOMUXC_XBAR1_IN16_SELECT_INPUT = IOMUXC_XBAR1_IN16_SELECT_INPUT_GPIO_SD_B0_02_ALT7;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_02, XBAR1_INOUT16, ulInputConfig); // select XBAR_INOUT16 on GPIO3_IO15 (GPIO_SD_B0_02) alt. function 7
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT16_IN;
                        break;
                    case PORT3_BIT19:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT17 is an input
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_SD_B0_06_ALT6;
                        _CONFIG_PERIPHERAL(GPIO_SD_B0_06, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO3_IO19 (GPIO_SD_B0_06) alt. function 6
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                    case PORT3_BIT8:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18); // ensure XBAR_INOUT18 is an input
                        IOMUXC_XBAR1_IN18_SELECT_INPUT = IOMUXC_XBAR1_IN18_SELECT_INPUT_GPIO_EMC_40_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_40, XBAR1_INOUT18, ulInputConfig); // select XBAR_INOUT18 on GPIO3_IO08 (GPIO_EMC_40) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT18_IN;
                        break;
                    case PORT3_BIT9:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19); // ensure XBAR_INOUT19 is an input
                        IOMUXC_XBAR1_IN19_SELECT_INPUT = IOMUXC_XBAR1_IN19_SELECT_INPUT_GPIO_EMC_41_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_41, XBAR1_INOUT19, ulInputConfig); // select XBAR_INOUT19 on GPIO3_IO09 (GPIO_EMC_41) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT19_IN;
                        break;
                #endif
                    default:
                        _EXCEPTION("DMA trigger not supported on defined pin!");
                        return;
                    }
                }
            #endif
            #if defined iMX_RT105X || defined iMX_RT106X
                else if (port_interrupt->int_port == PORT4) {
                    switch (port_interrupt->int_port_bits) {
                    case PORT4_BIT2:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4); // ensure XBAR_INOUT04 is an input
                        IOMUXC_XBAR1_IN04_SELECT_INPUT = IOMUXC_XBAR1_IN04_SELECT_INPUT_GPIO_EMC_02_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_02, XBAR1_INOUT04, ulInputConfig); // select XBAR_INOUT04 on GPIO4_IO02 (GPIO_EMC_02) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT04_IN;
                        break;
                    case PORT4_BIT3:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5); // ensure XBAR_INOUT05 is an input
                        IOMUXC_XBAR1_IN05_SELECT_INPUT = IOMUXC_XBAR1_IN05_SELECT_INPUT_GPIO_EMC_03_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_03, XBAR1_INOUT05, ulInputConfig); // select XBAR_INOUT05 on GPIO4_IO03 (GPIO_EMC_03) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT05_IN;
                        break;
                    case PORT4_BIT4:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6); // ensure XBAR_INOUT06 is an input
                        IOMUXC_XBAR1_IN06_SELECT_INPUT = IOMUXC_XBAR1_IN06_SELECT_INPUT_GPIO_EMC_04_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_04, XBAR1_INOUT06, ulInputConfig); // select XBAR_INOUT06 on GPIO4_IO04 (GPIO_EMC_04) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT06_IN;
                        break;
                    case PORT4_BIT5:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7); // ensure XBAR_INOUT07 is an input
                        IOMUXC_XBAR1_IN07_SELECT_INPUT = IOMUXC_XBAR1_IN07_SELECT_INPUT_GPIO_EMC_05_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_05, XBAR1_INOUT07, ulInputConfig); // select XBAR_INOUT07 on GPIO4_IO05 (GPIO_EMC_05) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT07_IN;
                        break;
                    case PORT4_BIT6:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8); // ensure XBAR_INOUT08 is an input
                        IOMUXC_XBAR1_IN08_SELECT_INPUT = IOMUXC_XBAR1_IN08_SELECT_INPUT_GPIO_EMC_06_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_06, XBAR1_INOUT08, ulInputConfig); // select XBAR_INOUT08 on GPIO4_IO06 (GPIO_EMC_06) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT08_IN;
                        break;
                    case PORT4_BIT7:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9); // ensure XBAR_INOUT09 is an input
                        IOMUXC_XBAR1_IN09_SELECT_INPUT = IOMUXC_XBAR1_IN09_SELECT_INPUT_GPIO_EMC_07_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_07, XBAR1_INOUT09, ulInputConfig); // select XBAR_INOUT09 on GPIO4_IO07 (GPIO_EMC_07) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT09_IN;
                        break;
                    case PORT4_BIT8:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17); // ensure XBAR_INOUT017 is an input
                        IOMUXC_XBAR1_IN17_SELECT_INPUT = IOMUXC_XBAR1_IN17_SELECT_INPUT_GPIO_EMC_08_ALT3;
                        _CONFIG_PERIPHERAL(GPIO_EMC_08, XBAR1_INOUT17, ulInputConfig); // select XBAR_INOUT17 on GPIO4_IO08 (GPIO_EMC_08) alt. function 3
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT17_IN;
                        break;
                    case PORT4_BIT14:
                        IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19); // ensure XBAR_INOUT019 is an input
                        IOMUXC_XBAR1_IN19_SELECT_INPUT = IOMUXC_XBAR1_IN19_SELECT_INPUT_GPIO_EMC_14_ALT1;
                        _CONFIG_PERIPHERAL(GPIO_EMC_14, XBAR1_INOUT19, ulInputConfig); // select XBAR_INOUT19 on GPIO4_IO14 (GPIO_EMC_14) alt. function 1
                        ucXbarInput = XBAR1_IOMUX_XBAR_INOUT19_IN;
                        break;
                    }
                }
            #endif
                else {
                    _EXCEPTION("DMA trigger not possible on this port!");
                    return;
                }
        #endif
                switch (port_interrupt->xbar_output) {
                case 0:
                    XBAR1_OUT00(ucXbarInput);                            // connect the XBAR input to XBAR_OUT02
                    XBARA1_CTRL0 = ((XBARA1_CTRL0 & 0xff00) | (usDMA_config >> 8)); // DMA generated on specified edges
        #if defined _WINDOWS
                    XBARA1_CTRL0 &= ~(XBARA_CTRL0_STS0);
        #endif
                    break;
                case 1:
                    XBAR1_OUT01(ucXbarInput);                            // connect the XBAR input to XBAR_OUT03
                    XBARA1_CTRL0 = ((XBARA1_CTRL0 & 0x00ff) | (usDMA_config)); // DMA generated on specified edges
        #if defined _WINDOWS
                    XBARA1_CTRL0 &= ~(XBARA_CTRL0_STS1);
        #endif
                    break;
                case 2:
                    XBAR1_OUT02(ucXbarInput);                            // connect the XBAR input to XBAR_OUT02
                    XBARA1_CTRL1 = ((XBARA1_CTRL1 & 0xff00) | (usDMA_config >> 8)); // DMA generated on specified edges
        #if defined _WINDOWS
                    XBARA1_CTRL1 &= ~(XBARA_CTRL1_STS2);
        #endif
                    break;
                case 3:
                    XBAR1_OUT03(ucXbarInput);                            // connect the XBAR input to XBAR_OUT03
                    XBARA1_CTRL1 = ((XBARA1_CTRL1 & 0x00ff) | (usDMA_config)); // DMA generated on specified edges
        #if defined _WINDOWS
                    XBARA1_CTRL1 &= ~(XBARA_CTRL1_STS3);
        #endif
                    break;
                default:
                    _EXCEPTION("Invalid XBAR output specified - usually 0..3 are possible");
                    return;
                }
    #else
                _EXCEPTION("SUPPORT_XBAR must be enabled for port DMA triggering");
    #endif
            }
            else {
                fnEnterPortInterruptHandler(port_interrupt, ulCharacteristics); // configure the interrupt and port bits according to the interrupt requirements
            }
        }
#endif

