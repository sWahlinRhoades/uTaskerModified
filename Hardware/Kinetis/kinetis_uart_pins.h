/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_uart_pins.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    07.01.2019 Add return value to fnConfigureUARTpin() and handle UART_RTS_RS485_MANUAL_MODE case when channel operated in manual RS485 mode
    09.01.2019 Shared with iMX project
    09.03.2019 Add error handling to devices with independent error interrupt {1}
    22.11.2019 Add complete KE15 LPUART RX/TX mux options
    02.01.2020 Add complete K02 UART1 RX/TX mux options
    30.10.2020 UART/LPUART in loop-back mode doesn't configure the UARTx Rx/LPUARTx Rx pin


*/

#if !defined UART_PULL_UPS                                               // if the user wishes pull-up/downs on the UART Rx pins this can be set to PORT_PS_UP_ENABLE or PORT_PS_DOWN_ENABLE in app_hw_kinetis.h
    #define UART_PULL_UPS    (PORT_NO_PULL)                              // if not overridden by the user there are no pull-up/downs on the UART Rx pins
#endif


static int fnConfigureUARTpin(QUEUE_HANDLE Channel, int iPinReference)
{
    int iInterruptID = 0;
    unsigned char ucPriority = 0;
    void (*InterruptFunc)(void) = 0;
    #if defined UART_ERROR_INTERRUPT_VECTOR                              // {1}
    void (*InterruptErrorFunc)(void) = 0;
    unsigned char ucErrorPriority = 0;
    #endif
    switch (Channel) {
    #if defined FIRST_LPUART_CHANNEL
    case FIRST_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART0 tx pin configuration
        #if defined _iMX
            #if defined iMX_RT1011
            IOMUXC_LPUART1_TXD_SELECT_INPUT = IOMUXC_LPUART1_TXD_SELECT_INPUT_GPIO_10_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_10, LPUART1_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 Tx on GPIO1-10  - alt function 0 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B0_12, LPUART1_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 Tx on GPIO1-12 - alt function 2 [iMX LPUARTs count 1..8]
            #else
            _CONFIG_PERIPHERAL(GPIO_AD_B0_06, LPUART1_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 Tx on GPIO1-06 - alt function 2 [iMX LPUARTs count 1..8]
            #endif
        #elif defined KINETIS_KE15
            #if defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 1, (PB_1_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB1 (alt. function 2)
            #elif defined LPUART0_ON_A_HIGH
            _CONFIG_PERIPHERAL(A, 10, (PA_10_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA10 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 3, (PA_3_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA3 (alt. function 6)
            #endif
        #elif defined KINETIS_K27 || defined KINETIS_K28
            _CONFIG_PERIPHERAL(C, 24, (PC_24_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PC24 (alt. function 3)
        #elif defined KINETIS_KL03
            #if defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 3, (PA_3_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA3 (alt. function 4)
            #elif defined LPUART0_ON_B_HIGH
            _CONFIG_PERIPHERAL(B, 3, (PB_3_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB2 (alt. function 3)
            #elif defined LPUART0_ON_B_ALT
            _CONFIG_PERIPHERAL(B, 2, (PB_2_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB2 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 1, (PB_1_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB1 (alt. function 2)
            #endif
        #elif defined KINETIS_K65 || defined KINETIS_K66
            #if defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 9, (PD_9_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PD9 (alt. function 5)
            #elif defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 2, (PA_2_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA2 (alt. function 5)
            #else
            _CONFIG_PERIPHERAL(E, 8, (PE_8_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PE8 (alt. function 5)
            #endif
        #elif defined KINETIS_KL43 || defined KINETIS_KL33 || defined KINETIS_KL17 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL82 || defined KINETIS_K80 || defined KINETIS_K32L2A || defined KINETIS_K32L2B
            #if !defined KINETIS_K80 && defined LPUART0_ON_E
            _CONFIG_PERIPHERAL(E, 20, (PE_20_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PE20 (alt. function 4)
            #elif (defined KINETIS_KL43 || defined KINETIS_KL33) && defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 7, (PD_7_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PD7 (alt. function 3)
            #elif defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 17, (PB_17_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB17 (alt. function 3)
            #elif defined LPUART0_ON_A_HIGH && (defined KINETIS_K80 || defined KINETIS_KL28 || defined KINETIS_K32L2A)
            _CONFIG_PERIPHERAL(A, 14, (PA_14_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA14 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 2, (PA_2_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PA2 (alt. function 2)
            #endif
        #else                                                            // K22, KV31
            #if defined LPUART0_ON_E
            _CONFIG_PERIPHERAL(E, 4, (PE_4_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PE4 (alt. function 3)
            #elif defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 11, (PB_11_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PB11 (alt. function 3)
            #elif defined LPUART0_ON_D_HIGH && defined KINETIS_K22
            _CONFIG_PERIPHERAL(D, 9, (PD_9_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PD9 (alt. function 5)
            #elif defined LPUART0_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 3, (PD_3_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PD3 (alt. function 6)
            #elif defined LPUART0_ON_C_HIGH
            _CONFIG_PERIPHERAL(C, 17, (PC_17_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PC17 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(C, 4, (PC_4_LPUART0_TX | UART_PULL_UPS)); // LPUART0_TX on PC4 (alt. function 7)
            #endif
        #endif
        #if defined irq_LPUART0_TX_ID
            iInterruptID = irq_LPUART0_TX_ID;                            // LPUART0 transmitter has unique interrupt vector
        #else
            iInterruptID = irq_LPUART0_ID;                               // LPUART0 transmitter shares interrupt vector with receiver
        #endif
            ucPriority = PRIORITY_LPUART0;
            InterruptFunc = _LPSCI0_Interrupt;
            break;

        case LPUART_RX_PIN:                                              // LPUART0 rx pin configuration
        #if defined irq_LPUART0_RX_ID
            iInterruptID = irq_LPUART0_RX_ID;                            // LPUART0 receiver has unique interrupt vector
        #else
            iInterruptID = irq_LPUART0_ID;                               // LPUART0 receiver shares interrupt vector with transmitter
        #endif
            ucPriority = PRIORITY_LPUART0;
            InterruptFunc = _LPSCI0_Interrupt;
            if ((LPUART0_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
        #if defined _iMX
            #if defined iMX_RT1011
            IOMUXC_LPUART1_RXD_SELECT_INPUT = IOMUXC_LPUART1_RXD_SELECT_INPUT_GPIO_09_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_09, LPUART1_RXD, UART_PULL_UPS);     // select LPUART1 Rx on GPIO1-09    alt. function 0 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B0_13, LPUART1_RX, UART_PULL_UPS);// select LPUART1 Rx on GPIO1-13 alt. function 2 [iMX LPUARTs count 1..8]
            #else
            _CONFIG_PERIPHERAL(GPIO_AD_B0_07, LPUART1_RX, UART_PULL_UPS);// select LPUART1 Rx on GPIO1-07 alt. function 2 [iMX LPUARTs count 1..8]
            #endif
        #elif defined KINETIS_KE15
            #if defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 0, (PB_0_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB0 (alt. function 2)
            #elif defined LPUART0_ON_A_HIGH
            _CONFIG_PERIPHERAL(A, 11, (PA_11_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA11 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 2, (PA_2_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA2 (alt. function 6)
            #endif
        #elif defined KINETIS_K27 || defined KINETIS_K28
            _CONFIG_PERIPHERAL(C, 25, (PC_25_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PC25 (alt. function 3)
        #elif defined KINETIS_KL03
            #if defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 4, (PA_4_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA4 (alt. function 4)
            #elif defined LPUART0_ON_B_HIGH && !defined LPUART_OPTION_1
            _CONFIG_PERIPHERAL(B, 4, (PB_4_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB4 (alt. function 3)
            #elif defined LPUART0_ON_B_ALT
            _CONFIG_PERIPHERAL(B, 1, (PB_1_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB1 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 2, (PB_2_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB2 (alt. function 2)
            #endif
        #elif defined KINETIS_K65 || defined KINETIS_K66
            #if defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 8, (PD_8_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PD8 (alt. function 5)
            #elif defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 1, (PA_1_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA1 (alt. function 5)
            #else
            _CONFIG_PERIPHERAL(E, 9, (PE_9_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PE9 (alt. function 5)
            #endif
        #elif defined KINETIS_KL17 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL33 || defined KINETIS_KL43 || defined KINETIS_KL82 || defined KINETIS_K80 || defined KINETIS_K32L2A || defined KINETIS_K32L2B
            #if !defined KINETIS_K80 && defined LPUART0_ON_E
            _CONFIG_PERIPHERAL(E, 21, (PE_21_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PE21 (alt. function 4)
            #elif (defined KINETIS_KL43 || defined KINETIS_KL33 || defined KINETIS_KL28 || defined KINETIS_K80 || defined KINETIS_K32L2A) && defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 6, (PD_6_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PD6 (alt. function 3)
            #elif defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 16, (PB_16_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB16 (alt. function 3)
            #elif defined LPUART0_ON_A_HIGH && (defined KINETIS_K80 || defined KINETIS_KL28 || defined KINETIS_K32L2A)
            _CONFIG_PERIPHERAL(A, 15, (PA_15_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA15 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 1, (PA_1_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PA1 (alt. function 2)
            #endif
        #else                                                            // K22, KV31
            #if defined LPUART0_ON_E
            _CONFIG_PERIPHERAL(E, 5, (PE_5_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PE5 (alt. function 3)
            #elif defined LPUART0_ON_B
            _CONFIG_PERIPHERAL(B, 10, (PB_10_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PB10 (alt. function 3)
            #elif defined LPUART0_ON_D_HIGH && defined KINETIS_K22
            _CONFIG_PERIPHERAL(D, 8, (PD_8_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PD8 (alt. function 5)
            #elif defined LPUART0_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 2, (PD_2_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PD2 (alt. function 6)
            #elif defined LPUART0_ON_C_HIGH
            _CONFIG_PERIPHERAL(C, 16, (PC_16_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PC16 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(C, 3, (PC_3_LPUART0_RX | UART_PULL_UPS)); // LPUART0_RX on PC3 (alt. function 7)
            #endif
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
                #if defined _iMX && defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B0_14, LPUART1_CTS_B, UART_PULL_UPS); // select LPUART1 CTS on GPIO1-14 - alt function 2 [iMX LPUARTs count 1..8]
                #else
            _EXCEPTION("To do");
                #endif
            #endif
            break;
            #if defined LPUART0_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_RTS_CONTROL
        case LPUART_RTS_PIN_ASSERT:
            if (ucRTS_neg[FIRST_LPUART_CHANNEL] == 0) {
                _SET_LPRTS_0_HIGH();
            }
            else {
                _SET_LPRTS_0_LOW();
            }
            break;
        case LPUART_RTS_PIN_NEGATE:
            if (ucRTS_neg[FIRST_LPUART_CHANNEL] == 0) {
                _SET_LPRTS_0_LOW();
            }
            else {
                _SET_LPRTS_0_HIGH();
            }
            break;
            #endif

        case LPUART_RTS_PIN_INVERTED:
        case LPUART_RTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_RTS_CONTROL
            if (iPinReference != LPUART_RTS_PIN_INVERTED) {
                _CONFIGURE_LPRTS_0_LOW();                                // configure RTS output and set to '0'
                ucRTS_neg[FIRST_LPUART_CHANNEL] = 0;                     // not inverted RTS mode
            }
            else {
                _CONFIGURE_LPRTS_0_HIGH();                               // configure RTS output and set to '1'
                ucRTS_neg[FIRST_LPUART_CHANNEL] = 1;                     // inverted RTS mode
            }
            #elif defined _iMX
                #if defined iMX_RT1011
            _CONFIG_PERIPHERAL(GPIO_07, LPUART1_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 RTS on GPIO1-07 [iMX LPUARTs count 1..8]
                #elif defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B0_15, LPUART1_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 RTS on GPIO1-15 [iMX LPUARTs count 1..8]
                #else
            _CONFIG_PERIPHERAL(GPIO_AD_B0_09, LPUART1_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART1 RTS on GPIO1-09 [iMX LPUARTs count 1..8]
                #endif
            #elif defined KINETIS_K65 || defined KINETIS_K66
                #if defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 10, (PD_10_LPUART0_RTS));              // LPUART0_RTS on PD10 (alt. function 5)
                #elif defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 3, (PA_3_LPUART0_RTS));                // LPUART0_RTS on PA3 (alt. function 5)
                #else
            _CONFIG_PERIPHERAL(E, 11, (PE_11_LPUART0_RTS));              // LPUART0_RTS on PE11 (alt. function 5)
                #endif
            #elif defined KINETIS_KE15
            _CONFIG_PERIPHERAL(C, 9, (PC_9_LPUART0_RTS));                // LPUART0_RTS on PC9 (alt. function 6)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined SECOND_LPUART_CHANNEL
    case SECOND_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART1 tx pin configuration
        #if defined _iMX
            #if defined iMX_RT1011
            IOMUXC_LPUART2_TXD_SELECT_INPUT = IOMUXC_LPUART2_TXD_SELECT_INPUT_GPIO_AD_00_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_00, LPUART2_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Tx on GPIO1-14  - alt function 0 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT1015
            IOMUXC_LPUART2_TX_SELECT_INPUT = IOMUXC_LPUART2_TX_SELECT_INPUT_GPIO_EMC_22_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_22, LPUART2_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Tx on GPIO2-22 - alt function 2 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART2_TX_SELECT_INPUT = IOMUXC_LPUART2_TX_SELECT_INPUT_GPIO_AD_B1_02_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_02, LPUART2_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Tx on GPIO1-18 - alt function 2 [iMX LPUARTs count 1..8]
            #else
                #if defined LPUART2_ON_EMC
            IOMUXC_LPUART2_TX_SELECT_INPUT = IOMUXC_LPUART2_TX_SELECT_INPUT_GPIO_EMC_22_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_22, LPUART2_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Tx on pad GPIO_EMC_22 on GPIO2-22 - alt function 2 [iMX LPUARTs count 1..8]
                #else
            IOMUXC_LPUART2_TX_SELECT_INPUT = IOMUXC_LPUART2_TX_SELECT_INPUT_GPIO_AD_B1_08_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_08, LPUART2_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Tx on pad GPIO_AD_B1_08 on GPIO1-24 - alt function 2 [iMX LPUARTs count 1..8]
                #endif
            #endif
        #elif defined KINETIS_KL17 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL33 || defined KINETIS_KL43 || defined KINETIS_K80 || defined KINETIS_K32L2A || defined KINETIS_K32L2B
            #if defined LPUART1_ON_E
            _CONFIG_PERIPHERAL(E, 0, (PE_0_LPUART1_TX | UART_PULL_UPS)); // LPUART1_TX on PE0 (alt. function 3)
            #elif defined LPUART1_ON_C
            _CONFIG_PERIPHERAL(C, 4, (PC_4_LPUART1_TX | UART_PULL_UPS)); // LPUART1_TX on PC4 (alt. function 3)
            #elif !defined KINETIS_K80
            _CONFIG_PERIPHERAL(A, 19, (PA_19_LPUART1_TX | UART_PULL_UPS)); // LPUART1_TX on PA19 (alt. function 3)
            #endif
        #elif defined KINETIS_KE15
            #if defined LPUART1_ON_C_HIGH
            _CONFIG_PERIPHERAL(C, 9, (PC_9_LPUART1_TX | UART_PULL_UPS)); // LPUART1_TX on PC9 (alt. function 2)
            #else
            _CONFIG_PERIPHERAL(C, 7, (PC_7_LPUART1_TX | UART_PULL_UPS)); // LPUART1_TX on PC7 (alt. function 2)
            #endif
        #endif
        #if defined irq_LPUART1_TX_ID
            iInterruptID = irq_LPUART1_TX_ID;                            // LPUART1 transmitter has a unique vector
        #elif defined irq_LPUART1_ID
            iInterruptID = irq_LPUART1_ID;                               // LPUART1 transmitter shares interrupt vector with receiver
        #else
            iInterruptID = irq_LPUART0_ID;                               // LPUART1 shares interrupt vector with LPUART0
        #endif
            ucPriority = PRIORITY_LPUART1;
            InterruptFunc = _LPSCI1_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART1 rx pin configuration
        #if defined irq_LPUART1_RX_ID
            iInterruptID = irq_LPUART1_RX_ID;                            // LPUART1 receiver has unique interrupt vector
        #elif defined irq_LPUART1_ID
            iInterruptID = irq_LPUART1_ID;                               // LPUART1 receiver shares interrupt vector with transmitter
        #else
            iInterruptID = irq_LPUART0_ID;                               // LPUART1 shares interrupt vector with LPUART0
        #endif
            ucPriority = PRIORITY_LPUART1;
            InterruptFunc = _LPSCI1_Interrupt;
            if ((LPUART1_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
        #if defined _iMX
            #if defined iMX_RT1011
            IOMUXC_LPUART2_RXD_SELECT_INPUT = IOMUXC_LPUART2_RXD_SELECT_INPUT_GPIO_13_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_13, LPUART2_RXD, UART_PULL_UPS);     // select LPUART2 Rx on GPIO1-13    alt. function 0 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT1015
            IOMUXC_LPUART2_RX_SELECT_INPUT = IOMUXC_LPUART2_RX_SELECT_INPUT_GPIO_EMC_23_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_23, LPUART2_RX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 Rx on GPIO2-23 - alt function 2 [iMX LPUARTs count 1..8]
            #elif defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART2_RX_SELECT_INPUT = IOMUXC_LPUART2_RX_SELECT_INPUT_GPIO_AD_B1_03_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_03, LPUART2_RX, UART_PULL_UPS); // select LPUART2 Rx on GPIO1-19 - alt function 2 [iMX LPUARTs count 1..8]
            #else
                #if defined LPUART2_ON_EMC
            IOMUXC_LPUART2_RX_SELECT_INPUT = IOMUXC_LPUART2_RX_SELECT_INPUT_GPIO_EMC_23_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_23, LPUART2_RX, UART_PULL_UPS);  // select LPUART2 Rx on GPIO2-23 - alt function 2 [iMX LPUARTs count 1..8]
                #else
            IOMUXC_LPUART2_RX_SELECT_INPUT = IOMUXC_LPUART2_RX_SELECT_INPUT_GPIO_AD_B1_09_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_09, LPUART2_RX, UART_PULL_UPS);// select LPUART2 Rx on GPIO1-25 - alt function 2 [iMX LPUARTs count 1..8]
                #endif
            #endif
        #elif defined KINETIS_KL17 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL33 || defined KINETIS_KL43 || defined KINETIS_K80 || defined KINETIS_K32L2A || defined KINETIS_K32L2B
            #if defined LPUART1_ON_E
            _CONFIG_PERIPHERAL(E, 1, (PE_1_LPUART1_RX | UART_PULL_UPS)); // LPUART1_RX on PE1 (alt. function 3)
            #elif defined LPUART1_ON_C
            _CONFIG_PERIPHERAL(C, 3, (PC_3_LPUART1_RX | UART_PULL_UPS)); // LPUART1_RX on PC3 (alt. function 3)
            #elif !defined KINETIS_K80
            _CONFIG_PERIPHERAL(A, 18, (PA_18_LPUART1_RX | UART_PULL_UPS)); // LPUART1_RX on PA18 (alt. function 3)
            #endif
        #elif defined KINETIS_KE15
            #if defined LPUART1_ON_C_HIGH
            _CONFIG_PERIPHERAL(C, 8, (PC_8_LPUART1_RX | UART_PULL_UPS)); // LPUART1_RX on PC8 (alt. function 2)
            #else
            _CONFIG_PERIPHERAL(C, 6, (PC_6_LPUART1_RX | UART_PULL_UPS)); // LPUART1_RX on PC6 (alt. function 2)
            #endif
        #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            #if defined _iMX
                #if defined iMX_RT1011 || defined iMX_RT1015
            _EXCEPTION("To do");
                #elif defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B1_01, LPUART2_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 RTS on GPIO1-17 [iMX LPUARTs count 1..8]
                #else
            _CONFIG_PERIPHERAL(GPIO_AD_B1_07, LPUART2_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART2 RTS on GPIO1-25 [iMX LPUARTs count 1..8]
                #endif
            #elif defined KINETIS_KE15
            _CONFIG_PERIPHERAL(E, 6, (PE_6_LPUART1_RTS));                // LPUART1_RTS on PE6 (alt. function 6)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined THIRD_LPUART_CHANNEL
    case THIRD_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART2 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT1011
            IOMUXC_LPUART3_TXD_SELECT_INPUT = IOMUXC_LPUART3_TXD_SELECT_INPUT_GPIO_12_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_12, LPUART3_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART3 Tx on GPIO1-12 alt. function 0 [iMX LPUARTs count 1..8]
                #elif defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART3_TX_SELECT_INPUT = IOMUXC_LPUART3_TX_SELECT_INPUT_GPIO_AD_B1_06_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_06, LPUART3_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART3 Tx on GPIO1-22 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART3_ON_EMC
            IOMUXC_LPUART3_TX_SELECT_INPUT = IOMUXC_LPUART3_TX_SELECT_INPUT_GPIO_EMC_06_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_06, LPUART3_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART3 Tx on GPIO2-06 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART3_TX_SELECT_INPUT = IOMUXC_LPUART3_TX_SELECT_INPUT_GPIO_AD_B0_14_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B0_14, LPUART3_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART3 Tx on GPIO1-14 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_KE15
                #if defined LPUART2_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 7, (PD_7_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PD7 (alt. function 2)
                #else
            _CONFIG_PERIPHERAL(E, 12, (PE_12_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PE12 (alt. function 2)
                #endif
            #elif defined KINETIS_KL28 || defined KINETIS_K32L2A
                #if defined LPUART2_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 16, (PE_16_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PE16 (alt. function 3)
                #elif defined LPUART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 22, (PE_22_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PE22 (alt. function 4)
                #elif defined LPUART2_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 3, (PD_3_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PD3 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(D, 5, (PD_5_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PD5 (alt. function 3)
                #endif
            #elif defined KINETIS_K80
                #if defined LPUART2_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 12, (PE_12_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PE12 (alt. function 3)
                #elif defined LPUART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 16, (PE_16_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PE16 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(D, 3, (PD_3_LPUART2_TX | UART_PULL_UPS)); // LPUART2_TX on PD3 (alt. function 3)
                #endif
            #endif
            #if !defined irq_LPUART2_ID && defined INTMUX0_AVAILABLE
            iInterruptID = (irq_INTMUX0_0_ID + INTMUX_LPUART2);          // LPUART2 interrupt vector based on INTMUX
            ucPriority = INTMUX0_PERIPHERAL_LPUART2;
            #elif defined irq_LPUART2_TX_ID
            iInterruptID = irq_LPUART2_TX_ID;                            // LPUART2 transmitter has a unique vector
            ucPriority = PRIORITY_LPUART2;
            #else
            iInterruptID = irq_LPUART2_ID;                               // LPUART2 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART2;
            #endif
            InterruptFunc = _LPSCI2_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART2 rx pin configuration
            #if !defined irq_LPUART2_ID && defined INTMUX0_AVAILABLE
            iInterruptID = (irq_INTMUX0_0_ID + INTMUX_LPUART2);          // LPUART2 interrupt vector based on INTMUX
            ucPriority = INTMUX0_PERIPHERAL_LPUART2;
            #elif defined irq_LPUART2_RX_ID
            iInterruptID = irq_LPUART2_RX_ID;                            // LPUART2 receiver has a unique vector
            ucPriority = PRIORITY_LPUART2;
            #else
            iInterruptID = irq_LPUART2_ID;                               // LPUART2 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART2;
            #endif
            InterruptFunc = _LPSCI2_Interrupt;
            if ((LPUART2_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT1011
            IOMUXC_LPUART3_RXD_SELECT_INPUT = IOMUXC_LPUART3_RXD_SELECT_INPUT_GPIO_11_ALT0; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_11, LPUART3_RXD, UART_PULL_UPS);     // select LPUART3 Rx on GPIO1-11 - alt. function 0 [iMX LPUARTs count 1..8]
                #elif defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART3_RX_SELECT_INPUT = IOMUXC_LPUART3_RX_SELECT_INPUT_GPIO_AD_B1_07_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_07, LPUART3_RX, UART_PULL_UPS); // select LPUART3 Rx on GPIO1-23 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART3_ON_EMC
            IOMUXC_LPUART3_RX_SELECT_INPUT = IOMUXC_LPUART3_RX_SELECT_INPUT_GPIO_EMC_07_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_07, LPUART3_RX, UART_PULL_UPS); // select LPUART3 Rx on GPIO2-07 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART3_RX_SELECT_INPUT = IOMUXC_LPUART3_RX_SELECT_INPUT_GPIO_AD_B0_15_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B0_15, LPUART3_RX, UART_PULL_UPS); // select LPUART3 Rx on GPIO1-15 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_KE15
                #if defined LPUART2_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 6, (PD_6_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PD6 (alt. function 2)
                #else
            _CONFIG_PERIPHERAL(D, 17, (PD_17_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PD17 (alt. function 2)
                #endif
            #elif defined KINETIS_KL28 || defined KINETIS_K32L2A
                #if defined LPUART2_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 17, (PE_17_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PE17 (alt. function 3)
                #elif defined LPUART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 23, (PE_23_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PE23 (alt. function 4)
                #elif defined LPUART2_ON_D_LOW
            _CONFIG_PERIPHERAL(D, 2, (PD_2_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PD2 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(D, 4, (PD_4_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PD4 (alt. function 3)
                #endif
            #elif defined KINETIS_K80
               #if defined LPUART2_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 13, (PE_13_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PE13 (alt. function 3)
                #elif defined LPUART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 17, (PE_17_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PE17 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(D, 2, (PD_2_LPUART2_RX | UART_PULL_UPS)); // LPUART2_RX on PD2 (alt. function 3)
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            #if defined _iMX
                #if defined iMX_RT1011 || defined iMX_RT1015
            _EXCEPTION("To do");
                #elif defined iMX_RT105X || defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_AD_B1_05, LPUART3_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART3 RTS on GPIO1-21 [iMX LPUARTs count 1..8]
                #endif
            #elif defined KINETIS_KE15
            _CONFIG_PERIPHERAL(D, 12, (PD_12_LPUART2_RTS));              // LPUART2_RTS on PD12 (alt. function 6)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined FOURTH_LPUART_CHANNEL
    case FOURTH_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART3 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT1011
            IOMUXC_LPUART4_TXD_SELECT_INPUT = IOMUXC_LPUART4_TXD_SELECT_INPUT_GPIO_06_ALT3; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_06, LPUART4_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO1-06 - alt. function 3 [iMX LPUARTs count 1..8]
                #elif defined iMX_RT105X || defined iMX_RT106X
                    #if defined UART4_ON_4
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_EMC_19_ALT2; // special case to select the pad involved in daisy chain
            CONFIG_PERIPHERAL(GPIO_EMC_19, LPUART4_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO4-19 - alt function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_B1_00_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_B1_00, LPUART4_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO2-16 - alt function 2 [iMX LPUARTs count 1..8]
                    #endif
                #elif defined iMX_RT1015
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_EMC_32_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_32, LPUART4_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO3-00 - alt. function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART3_ON_EMC_LOW
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_EMC_02_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_02, LPUART4_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO2-02 - alt. function 2 [iMX LPUARTs count 1..8]
                    #elif defined LPUART3_ON_EMC_HIGH
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_EMC_32_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_32, LPUART4_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO3-00 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART4_TX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_AD_B1_10_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_10, LPUART4_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Tx on GPIO1-26 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_K80
               #if defined LPUART2_ON_E
            _CONFIG_PERIPHERAL(E, 4, (PE_4_LPUART3_TX | UART_PULL_UPS)); // LPUART3_TX on PE4 (alt. function 3)
                #elif defined LPUART3_ON_B
            _CONFIG_PERIPHERAL(B, 11, (PB_11_LPUART3_TX | UART_PULL_UPS)); // LPUART3_TX on PB11 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(C, 17, (PC_17_LPUART3_TX | UART_PULL_UPS)); // LPUART3_TX on PC17 (alt. function 3)
                #endif
            #endif
            iInterruptID = irq_LPUART3_ID;                               // LPUART3 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART3;
            InterruptFunc = _LPSCI3_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART3 rx pin configuration
            iInterruptID = irq_LPUART3_ID;                               // LPUART3 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART3;
            InterruptFunc = _LPSCI3_Interrupt;
            if ((LPUART3_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT1011
            IOMUXC_LPUART4_RXD_SELECT_INPUT = IOMUXC_LPUART4_RXD_SELECT_INPUT_GPIO_05_ALT3; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_05, LPUART4_RXD, UART_PULL_UPS);     // select LPUART4 Rx on GPIO1-05    alt. function 3 [iMX LPUARTs count 1..8]
                #elif defined iMX_RT105X || defined iMX_RT106X
                    #if defined UART4_ON_4
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_RX_SELECT_INPUT_GPIO_EMC_20_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_20, LPUART4_RX, UART_PULL_UPS); // select LPUART4 Rx on GPIO4-20 - alt function 2 [iMX LPUARTs count 1..8]
                   #else
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_RX_SELECT_INPUT_GPIO_B1_01_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_B1_01, LPUART4_RXD, UART_PULL_UPS); // select LPUART4 Rx on GPIO2-17 - alt function 2 [iMX LPUARTs count 1..8]
                   #endif
                #elif defined iMX_RT1015
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_TX_SELECT_INPUT_GPIO_EMC_32_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_33, LPUART4_RX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Rx on GPIO3-01 - alt. function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART3_ON_EMC_LOW
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_RX_SELECT_INPUT_GPIO_EMC_03_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_03, LPUART4_RX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Rx on GPIO2-03 - alt. function 2 [iMX LPUARTs count 1..8]
                    #elif defined LPUART3_ON_EMC_HIGH
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_RX_SELECT_INPUT_GPIO_EMC_33_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_33, LPUART4_RX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Rx on GPIO3-01 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART4_RX_SELECT_INPUT = IOMUXC_LPUART4_RX_SELECT_INPUT_GPIO_AD_B1_11_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_11, LPUART4_RX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 Rx on GPIO1-27 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_K80
               #if defined LPUART2_ON_E
            _CONFIG_PERIPHERAL(E, 5, (PE_3_LPUART3_RX | UART_PULL_UPS)); // LPUART3_RX on PE3 (alt. function 3)
                #elif defined LPUART3_ON_B
            _CONFIG_PERIPHERAL(B, 10, (PB_10_LPUART3_RX | UART_PULL_UPS)); // LPUART3_RX on PB10 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(C, 16, (PC_16_LPUART3_RX | UART_PULL_UPS)); // LPUART3_RX on PC16 (alt. function 3)
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
                #if defined _iMX && defined iMX_RT106X
            _CONFIG_PERIPHERAL(GPIO_EMC_17, LPUART4_CTS_B, UART_PULL_UPS); // select LPUART4 CTS on GPIO4-17 - alt function 2 [iMX LPUARTs count 1..8]
                #else
            _EXCEPTION("To do");
                #endif
            #endif
            break;
        case LPUART_RTS_PIN:
            #if defined _iMX
                #if (defined iMX_RT105X || defined iMX_RT106X) && defined UART4_ON_4
            _CONFIG_PERIPHERAL(GPIO_EMC_18, LPUART4_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART4 RTS on GPIO4-18 [iMX LPUARTs count 1..8]
                #else
            _EXCEPTION("To do...");
                #endif
            #else
            _EXCEPTION("To do...");
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined FIFTH_LPUART_CHANNEL
    case FIFTH_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART4 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART5_TX_SELECT_INPUT = IOMUXC_LPUART5_TX_SELECT_INPUT_GPIO_B1_12_ALT1; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_B1_12, LPUART5_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART5 Tx on GPIO2-28 - alt function 1 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART5_ON_EMC
            IOMUXC_LPUART5_TX_SELECT_INPUT = IOMUXC_LPUART5_TX_SELECT_INPUT_GPIO_EMC_38_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_38, LPUART5_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART5 Tx on GPIO3-06 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART5_TX_SELECT_INPUT = IOMUXC_LPUART5_TX_SELECT_INPUT_GPIO_AD_B0_10_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B0_10, LPUART5_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART5 Tx on GPIO1-10 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_K80
               #if defined LPUART4_ON_A
            _CONFIG_PERIPHERAL(A, 20, (PA_20_LPUART4_TX | UART_PULL_UPS)); // LPUART4_TX on PA20 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(C, 15, (PC_15_LPUART4_TX | UART_PULL_UPS)); // LPUART4_TX on PC15 (alt. function 3)
                #endif
            #endif
            iInterruptID = irq_LPUART4_ID;                               // LPUART4 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART4;
            InterruptFunc = _LPSCI4_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART4 rx pin configuration
            iInterruptID = irq_LPUART4_ID;                               // LPUART4 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART4;
            InterruptFunc = _LPSCI4_Interrupt;
            if ((LPUART4_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART5_RX_SELECT_INPUT = IOMUXC_LPUART5_RX_SELECT_INPUT_GPIO_B1_13_ALT1; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_B1_13, LPUART5_RXD, UART_PULL_UPS); // select LPUART5 Rx on GPIO2-29 - alt function 1 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART5_ON_EMC
            IOMUXC_LPUART5_RX_SELECT_INPUT = IOMUXC_LPUART5_RX_SELECT_INPUT_GPIO_EMC_39_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_39, LPUART5_RX, (UART_PULL_UPS));// select LPUART5 Rx on GPIO3-07 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART5_RX_SELECT_INPUT = IOMUXC_LPUART5_RX_SELECT_INPUT_GPIO_AD_B0_11_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B0_11, LPUART5_RX, (UART_PULL_UPS)); // select LPUART5 Rx on GPIO1-11 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #elif defined KINETIS_K80
               #if defined LPUART4_ON_A
            _CONFIG_PERIPHERAL(A, 21, (PA_21_LPUART4_RX | UART_PULL_UPS)); // LPUART4_RX on PA21 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(C, 14, (PC_14_LPUART4_RX | UART_PULL_UPS)); // LPUART4_RX on PC14 (alt. function 3)
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            _EXCEPTION("To do...");
            break;
        #endif
        }
        break;
    #endif

    #if defined SIXTH_LPUART_CHANNEL
    case SIXTH_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART5 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART6_TX_SELECT_INPUT = IOMUXC_LPUART6_TX_SELECT_INPUT_GPIO_EMC_25_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_25, LPUART6_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART6 Tx on GPIO4-25 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART6_ON_EMC
            IOMUXC_LPUART6_TX_SELECT_INPUT = IOMUXC_LPUART6_TX_SELECT_INPUT_GPIO_EMC_12_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_12, LPUART6_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART6 Tx on GPIO2-12 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART6_TX_SELECT_INPUT = IOMUXC_LPUART6_TX_SELECT_INPUT_GPIO_SD_B1_00_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B1_00, LPUART6_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART6 Tx on GPIO3-20 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            iInterruptID = irq_LPUART5_ID;                               // LPUART5 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART5;
            InterruptFunc = _LPSCI5_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART5 rx pin configuration
            iInterruptID = irq_LPUART5_ID;                               // LPUART5 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART5;
            InterruptFunc = _LPSCI5_Interrupt;
            if ((LPUART5_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART6_RX_SELECT_INPUT = IOMUXC_LPUART6_RX_SELECT_INPUT_GPIO_EMC_26_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_26, LPUART6_RXD, UART_PULL_UPS); // select LPUART6 Rx on GPIO4-26 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART6_ON_EMC
            IOMUXC_LPUART6_RX_SELECT_INPUT = IOMUXC_LPUART6_RX_SELECT_INPUT_GPIO_EMC_13_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_13, LPUART6_RX, (UART_PULL_UPS)); // select LPUART6 Rx on GPIO2-13 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART6_RX_SELECT_INPUT = IOMUXC_LPUART6_RX_SELECT_INPUT_GPIO_SD_B1_01_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B1_01, LPUART6_RX, (UART_PULL_UPS)); // select LPUART6 Rx on GPIO3-21 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            #if defined _iMX
                #if (defined iMX_RT105X || defined iMX_RT106X)
            _CONFIG_PERIPHERAL(GPIO_EMC_29, LPUART6_RTS_B, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART6 RTS on GPIO4-29 [iMX LPUARTs count 1..8]
                #else
            _EXCEPTION("To do...");
                #endif
            #else
            _EXCEPTION("To do...");
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined SEVENTH_LPUART_CHANNEL
    case SEVENTH_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART6 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART7_TX_SELECT_INPUT = IOMUXC_LPUART7_TX_SELECT_INPUT_GPIO_EMC_31_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_31, LPUART7_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART7 Tx on GPIO4-31 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART7_ON_EMC
            IOMUXC_LPUART7_TX_SELECT_INPUT = IOMUXC_LPUART7_TX_SELECT_INPUT_GPIO_EMC_34_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_34, LPUART7_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART7 Tx on GPIO3-02 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART7_TX_SELECT_INPUT = IOMUXC_LPUART7_TX_SELECT_INPUT_GPIO_SD_B0_04_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B0_04, LPUART7_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART7 Tx on GPIO3-17 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            iInterruptID = irq_LPUART6_ID;                               // LPUART6 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART6;
            InterruptFunc = _LPSCI6_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART6 rx pin configuration
            iInterruptID = irq_LPUART6_ID;                               // LPUART6 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART6;
            InterruptFunc = _LPSCI6_Interrupt;
            if ((LPUART6_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
            IOMUXC_LPUART7_RX_SELECT_INPUT = IOMUXC_LPUART7_RX_SELECT_INPUT_GPIO_EMC_32_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_32, LPUART7_RXD, UART_PULL_UPS); // select LPUART7 Rx on GPIO3-18 - alt function 2 [iMX LPUARTs count 1..8]
                #else
                    #if defined LPUART7_ON_EMC
            IOMUXC_LPUART7_RX_SELECT_INPUT = IOMUXC_LPUART7_RX_SELECT_INPUT_GPIO_EMC_35_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_35, LPUART7_RX, (UART_PULL_UPS)); // select LPUART7 Rx on GPIO3-03 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART7_RX_SELECT_INPUT = IOMUXC_LPUART7_RX_SELECT_INPUT_GPIO_SD_B0_05_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B0_05, LPUART7_RX, (UART_PULL_UPS)); // select LPUART7 Rx on GPIO3-18 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            _EXCEPTION("To do...");
            break;
        #endif
        }
        break;
    #endif

    #if defined EIGHTH_LPUART_CHANNEL
    case EIGHTH_LPUART_CHANNEL:
        switch (iPinReference) {
        case LPUART_TX_PIN:                                              // LPUART7 tx pin configuration
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
                    #if defined LPUART8_ON_AD
            IOMUXC_LPUART8_TX_SELECT_INPUT = IOMUXC_LPUART8_TX_SELECT_INPUT_GPIO_AD_B1_10_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_10, LPUART8_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART8 Tx on GPIO1-26 - alt function 2 [iMX LPUARTs count 1..8]
                    #elif defined LPUART8_ON_SD
            IOMUXC_LPUART8_TX_SELECT_INPUT = IOMUXC_LPUART8_TX_SELECT_INPUT_GPIO_SD_B0_04_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B0_04, LPUART8_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART8 Tx on GPIO3-16 - alt function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART8_TX_SELECT_INPUT = IOMUXC_LPUART8_TX_SELECT_INPUT_GPIO_EMC_38_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_38, LPUART8_TXD, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART8 Tx on GPIO3-24 - alt function 2 [iMX LPUARTs count 1..8]
                    #endif
                #else
                    #if defined LPUART8_ON_EMC
            IOMUXC_LPUART8_TX_SELECT_INPUT = IOMUXC_LPUART8_TX_SELECT_INPUT_GPIO_EMC_26_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_26, LPUART8_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART8 Tx on GPIO2-26 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART8_TX_SELECT_INPUT = IOMUXC_LPUART8_TX_SELECT_INPUT_GPIO_SD_B1_02_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B1_02, LPUART8_TX, (UART_PULL_UPS | PORT_DSE_MID)); // select LPUART8 Tx on GPIO3-22 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            iInterruptID = irq_LPUART7_ID;                               // LPUART7 transmitter shares interrupt vector with receiver
            ucPriority = PRIORITY_LPUART7;
            InterruptFunc = _LPSCI7_Interrupt;
            break;
        case LPUART_RX_PIN:                                              // LPUART7 rx pin configuration
            iInterruptID = irq_LPUART7_ID;                               // LPUART7 receiver shares interrupt vector with transmitter
            ucPriority = PRIORITY_LPUART7;
            InterruptFunc = _LPSCI7_Interrupt;
            if ((LPUART7_CTRL & LPUART_CTRL_LOOPS) != 0) {               // loop-back/single-wire mode doesn't configure/use the LPUARTx_Rx pin
                break;
            }
            #if defined _iMX
                #if defined iMX_RT105X || defined iMX_RT106X
                    #if defined LPUART8_ON_AD
            IOMUXC_LPUART8_RX_SELECT_INPUT = IOMUXC_LPUART8_RX_SELECT_INPUT_GPIO_AD_B1_11_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_AD_B1_11, LPUART8_RXD, UART_PULL_UPS); // select LPUART8 Rx on GPIO1-27 - alt function 2 [iMX LPUARTs count 1..8]
                    #elif defined LPUART8_ON_SD
            IOMUXC_LPUART8_RX_SELECT_INPUT = IOMUXC_LPUART8_RX_SELECT_INPUT_GPIO_SD_B0_05_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B0_05, LPUART8_RXD, UART_PULL_UPS); // select LPUART8 Rx on GPIO3-17 - alt function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART8_RX_SELECT_INPUT = IOMUXC_LPUART8_RX_SELECT_INPUT_GPIO_EMC_39_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_39, LPUART8_RXD, UART_PULL_UPS); // select LPUART8 Rx on GPIO3-24 - alt function 2 [iMX LPUARTs count 1..8]
                    #endif
                #else
                    #if defined LPUART8_ON_EMC
            IOMUXC_LPUART8_RX_SELECT_INPUT = IOMUXC_LPUART8_RX_SELECT_INPUT_GPIO_EMC_27_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_EMC_27, LPUART8_RX, (UART_PULL_UPS));// select LPUART8 Rx on GPIO2-27 - alt. function 2 [iMX LPUARTs count 1..8]
                    #else
            IOMUXC_LPUART8_RX_SELECT_INPUT = IOMUXC_LPUART8_RX_SELECT_INPUT_GPIO_SD_B1_03_ALT2; // special case to select the pad involved in daisy chain
            _CONFIG_PERIPHERAL(GPIO_SD_B1_03, LPUART8_RX, (UART_PULL_UPS)); // select LPUART8 Rx on GPIO3-23 - alt. function 2 [iMX LPUARTs count 1..8]
                    #endif
                #endif
            #endif
            break;
        #if defined SUPPORT_HW_FLOW
        case LPUART_CTS_PIN:
            #if defined LPUART_WITHOUT_MODEM_CONTROL || defined LPUART0_MANUAL_CTS_CONTROL // use GPIO interrupt
            _EXCEPTION("To do");
            #else                                                        // use dedicated CTS line
            _EXCEPTION("To do");
            #endif
            break;
        case LPUART_RTS_PIN:
            _EXCEPTION("To do...");
            break;
        #endif
        }
        break;
    #endif

    #if defined FIRST_UART_CHANNEL
    case FIRST_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART0 tx pin configuration
        #if defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05
            #if defined KINETIS_KL03 && defined UART0_OPTION_3
            _CONFIG_PERIPHERAL(A, 3, (PA_3_UART0_TX | UART_PULL_UPS));   // UART0_TX on PA3 (alt. function 4)
            #elif defined UART0_OPTION_2
            _CONFIG_PERIPHERAL(B, 3, (PB_3_UART0_TX | UART_PULL_UPS));   // UART0_TX on PB3 (alt. function 3)
            #elif defined UART0_OPTION_1
            _CONFIG_PERIPHERAL(B, 2, (PB_2_UART0_TX | UART_PULL_UPS));   // UART0_TX on PB2 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 1, (PB_1_UART0_TX | UART_PULL_UPS));   // UART0_TX on PB1 (alt. function 2)
            #endif
        #elif defined KINETIS_K02
            #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 2, (PA_2_UART0_TX | UART_PULL_UPS));   // UART0_TX on PA2 (alt. function 2)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 7, (PD_7_UART0_TX | UART_PULL_UPS));   // UART0_TX on PD7 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 17, (PB_17_UART0_TX | UART_PULL_UPS)); // UART0_TX on PB17 (alt. function 3)
            #endif
        #elif defined KINETIS_KE
            #if defined UART0_ON_A
            SIM_PINSEL0 |= (SIM_PINSEL_UART0PS);
            _CONFIG_PERIPHERAL(A, 3, (PA_3_UART0_TX | UART_PULL_UPS));   // UART0_TX on PA3 (alt. function 2)
            #else
            _CONFIG_PERIPHERAL(B, 1, (PB_1_UART0_TX | UART_PULL_UPS));   // UART0_TX on PB1 (alt. function 2)
            #endif
        #else
            #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 2, (PA_2_UART0_TX | UART_PULL_UPS));   // UART0_TX on PA2 (alt. function 2)
            #elif defined UART0_ON_B_LOW && (defined KINETIS_KV31 || defined KINETIS_KV50)
            _CONFIG_PERIPHERAL(B, 1, (PB_1_UART0_TX | UART_PULL_UPS));   // UART0_TX on PB1 (alt. function 7)
            #elif defined UART0_ON_C && defined KINETIS_KV50
            _CONFIG_PERIPHERAL(C, 7, (PC_7_UART0_TX | UART_PULL_UPS));   // UART0_TX on PC7 (alt. function 5)
            #elif defined UART0_ON_B
            _CONFIG_PERIPHERAL(B, 17, (PB_17_UART0_TX | UART_PULL_UPS)); // UART0_TX on PB17 (alt. function 3)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 7, (PD_7_UART0_TX | UART_PULL_UPS));   // UART0_TX on PD7 (alt. function 3)
            #elif defined KINETIS_KL && defined UART0_ON_E
            _CONFIG_PERIPHERAL(E, 20, (PE_20_UART0_TX | UART_PULL_UPS)); // UART0_TX on PE20 (alt. function 4)
            #else
            _CONFIG_PERIPHERAL(A, 14, (PA_14_UART0_TX | UART_PULL_UPS)); // UART0_TX on PA14 (alt. function 3)
            #endif
        #endif
        #if defined irq_UART0_1_ID
            iInterruptID = irq_LPUART0_TX_ID;                            // UART0/1 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART0_ID;                                 // UART0 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART0;
            InterruptFunc = _SCI0_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART0 rx pin configuration
        #if defined irq_UART0_1_ID
            iInterruptID = irq_UART0_1_ID;                               // UART0 / 1 transmitter / receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART0_ID;                                 // UART0 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART0;
            InterruptFunc = _SCI0_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI0_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART0;
        #endif
            if ((UART0_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05
            #if defined KINETIS_KL03 && defined UART0_OPTION_3
            _CONFIG_PERIPHERAL(A, 4, (PA_4_UART0_RX | UART_PULL_UPS));   // UART0_RX on PA4 (alt. function 4)
            #elif defined UART0_OPTION_2
            _CONFIG_PERIPHERAL(B, 4, (PB_4_UART0_RX | UART_PULL_UPS));   // UART0_RX on PB4 (alt. function 3)
            #elif defined UART0_OPTION_1
            _CONFIG_PERIPHERAL(B, 1, (PB_1_UART0_RX | UART_PULL_UPS));   // UART0_RX on PB1 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 2, (PB_2_UART0_RX | UART_PULL_UPS));   // UART0_RX on PB2 (alt. function 2)
            #endif
        #elif defined KINETIS_K02
            #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 1, (PA_1_UART0_RX | UART_PULL_UPS));   // UART0_RX on PA1 (alt. function 2)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 6, (PD_6_UART0_RX | UART_PULL_UPS));   // UART0_RX on PD6 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(B, 16, (PB_16_UART0_RX | UART_PULL_UPS)); // UART0_RX on PB16 (alt. function 3)
            #endif
        #elif defined KINETIS_KE
            #if defined UART0_ON_A
            SIM_PINSEL0 |= (SIM_PINSEL_UART0PS);
            _CONFIG_PERIPHERAL(A, 2, (PA_2_UART0_RX | UART_PULL_UPS));   // UART0_RX on PA2 (alt. function 2)
            #else
            _CONFIG_PERIPHERAL(B, 0, (PB_0_UART0_RX | UART_PULL_UPS));   // UART0_RX on PB0 (alt. function 2)
            #endif
        #else
            #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 1, (PA_1_UART0_RX | UART_PULL_UPS));   // UART0_RX on PA1 (alt. function 2)
            #elif defined UART0_ON_B_LOW && (defined KINETIS_KV31 || defined KINETIS_KV50)
            _CONFIG_PERIPHERAL(B, 0, (PB_0_UART0_RX | UART_PULL_UPS));   // UART0_RX on PB0 (alt. function 7)
            #elif defined UART0_ON_C && defined KINETIS_KV50
            _CONFIG_PERIPHERAL(C, 6, (PC_6_UART0_RX | UART_PULL_UPS));   // UART0_RX on PC6 (alt. function 5)
            #elif defined UART0_ON_B
            _CONFIG_PERIPHERAL(B, 16, (PB_16_UART0_RX | UART_PULL_UPS)); // UART0_RX on PB16 (alt. function 3)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 6, (PD_6_UART0_RX | UART_PULL_UPS));   // UART0_RX on PD6 (alt. function 3)
            #elif defined KINETIS_KL && defined UART0_ON_E
            _CONFIG_PERIPHERAL(E, 21, (PE_21_UART0_RX | UART_PULL_UPS)); // UART0_RX on PE21 (alt. function 4)
            #else
            _CONFIG_PERIPHERAL(A, 15, (PA_15_UART0_RX | UART_PULL_UPS)); // UART0_RX on PA15 (alt. function 3)
            #endif
        #endif
            break;
        #if defined SUPPORT_HW_FLOW
            #if defined UART0_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART0_MANUAL_RTS_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[0] == 0) {
                _SET_RTS_0_HIGH();
            }
            else {
                _SET_RTS_0_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[0] == 0) {
                _SET_RTS_0_LOW();
            }
            else {
                _SET_RTS_0_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART0_MANUAL_RTS_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_0_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[0] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_0_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[0] = 1;                                        // inverted RTS mode
            }
            #elif defined KINETIS_K02
                #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 3, (PA_3_UART0_RTS | UART_PULL_UPS));  // UART0_RX on PA3 (alt. function 2)
                #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 4, (PD_4_UART0_RTS | UART_PULL_UPS));  // UART0_RX on PD4 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(B, 2, (PB_2_UART0_RTS | UART_PULL_UPS));  // UART0_RX on PB2 (alt. function 3)
                #endif
            #elif defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 3, (PA_3_UART0_RTS | UART_PULL_UPS));  // UART0_RTS on PA3 (alt. function 2)
            #elif defined UART0_ON_B
            _CONFIG_PERIPHERAL(B, 2, (PB_2_UART0_RTS | UART_PULL_UPS));  // UART0_RTS on PB2 (alt. function 3)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 4, (PD_4_UART0_RTS | UART_PULL_UPS));  // UART0_RTS on PD4 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 17, (PA_17_UART0_RTS | UART_PULL_UPS));// UART0_RTS on A17 (alt. function 3)
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            #elif defined KINETIS_K02
                #if defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 0, (PA_0_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PA0 (alt. function 2)
                #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 5, (PD_5_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PD5 (alt. function 3)
                #else
            _CONFIG_PERIPHERAL(A, 0, (PA_0_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PA0 (alt. function 2)
                #endif
            #elif defined UART0_A_LOW
            _CONFIG_PERIPHERAL(A, 0, (PA_0_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PA0 (alt. function 2)
            #elif defined UART0_ON_B
            _CONFIG_PERIPHERAL(B, 3, (PB_3_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PB3 (alt. function 3)
            #elif defined UART0_ON_D
            _CONFIG_PERIPHERAL(D, 5, (PD_5_UART0_CTS | UART_PULL_UPS));  // UART0_CTS on PD5 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(A, 16, (PA_16_UART0_CTS | UART_PULL_UPS));// UART0_CTS on PA16 (alt. function 3)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined SECOND_UART_CHANNEL
    case SECOND_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART1 tx pin configuration
        #if defined KINETIS_KM
            _CONFIG_PERIPHERAL(I, 1, (PI_1_SCI1_TX | UART_PULL_UPS));    // UART1_TX on PI1 (alt. function 2)
        #elif defined KINETIS_KE
            _CONFIG_PERIPHERAL(C, 7, (PC_7_UART1_TX | UART_PULL_UPS));   // UART1_TX on PC7 (alt. function 2)
        #elif defined KINETIS_KV10 || defined KINETIS_KV11
            _CONFIG_PERIPHERAL(D, 1, (PD_1_UART1_TX | UART_PULL_UPS));   // UART1_TX on PD1 (alt. function 5)
        #elif defined KINETIS_K02
            #if defined UART1_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 16, (PE_16_UART1_TX | UART_PULL_UPS)); // UART1_TX on PE16 (alt. function 3)
            #elif defined UART1_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 0, (PE_0_UART1_TX | UART_PULL_UPS));   // UART1_TX on PE0 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(C, 4, (PC_4_UART1_TX | UART_PULL_UPS));   // UART1_TX on PC4 (alt. function 3)
            #endif
        #else
            #if defined UART1_ON_C && !defined UART1_ON_A_TX
            _CONFIG_PERIPHERAL(C, 4, (PC_4_UART1_TX | UART_PULL_UPS));   // UART1_TX on PC4 (alt. function 3)
            #elif defined KINETIS_KL && (defined UART1_ON_A || defined UART1_ON_A_TX)
            _CONFIG_PERIPHERAL(A, 19, (PA_19_UART1_TX | UART_PULL_UPS)); // UART1_TX on PA19 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(E, 0, (PE_0_UART1_TX | UART_PULL_UPS));   // UART1_TX on PE0 (alt. function 3)
            #endif
        #endif
        #if defined irq_UART0_1_ID
            iInterruptID = irq_UART0_1_ID;                               // UART0/1 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART1_ID;                                 // UART1 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART1;
            InterruptFunc = _SCI1_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART1 rx pin configuration
        #if defined irq_UART0_1_ID
            iInterruptID = irq_UART0_1_ID;                               // UART0/1 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART1_ID;                                 // UART1 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART1;
            InterruptFunc = _SCI1_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI1_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART1;
        #endif
            if ((UART1_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined KINETIS_KM
            _CONFIG_PERIPHERAL(I, 0, (PI_0_SCI1_TX | UART_PULL_UPS));    // UART1_RX on PI0 (alt. function 2)
        #elif defined KINETIS_KE
            _CONFIG_PERIPHERAL(C, 6, (PC_6_UART1_RX | UART_PULL_UPS));   // UART1_RX on PC6 (alt. function 2)
        #elif defined KINETIS_KV10 || defined KINETIS_KV11
            _CONFIG_PERIPHERAL(D, 0, (PD_0_UART1_RX | UART_PULL_UPS));   // UART1_RX on PD0 (alt. function 5)
        #elif defined KINETIS_K02
            #if defined UART1_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 17, (PE_17_UART1_RX | UART_PULL_UPS)); // UART1_RX on PE17 (alt. function 3)
            #elif defined UART1_ON_E_LOW
            _CONFIG_PERIPHERAL(E, 1, (PE_1_UART1_RX | UART_PULL_UPS));   // UART1_RX on PE1 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(C, 3, (PC_3_UART1_RX | UART_PULL_UPS));   // UART1_RX on PC3 (alt. function 3)
            #endif
        #else
            #if defined UART1_ON_C
            _CONFIG_PERIPHERAL(C, 3, (PC_3_UART1_RX | UART_PULL_UPS));   // UART1_RX on PC3 (alt. function 3)
            #elif defined KINETIS_KL && defined UART1_ON_A
            _CONFIG_PERIPHERAL(A, 18, (PA_18_UART1_RX | UART_PULL_UPS)); // UART1_RX on PA18 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(E, 1, (PE_1_UART1_RX | UART_PULL_UPS));   // UART1_RX on PE1 (alt. function 3)
            #endif
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
            #if defined UART1_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART1_MANUAL_RTS_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[1] == 0) {
                _SET_RTS_1_HIGH();
            }
            else {
                _SET_RTS_1_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[1] == 0) {
                _SET_RTS_1_LOW();
            }
            else {
                _SET_RTS_1_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:                                               // configure RTS1 pin
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART1_MANUAL_RTS_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_1_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[1] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_1_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[1] = 1;                                        // inverted RTS mode
            }
            #elif defined UART1_ON_C
            _CONFIG_PERIPHERAL(C, 1, (PC_1_UART1_RTS | UART_PULL_UPS));  // UART1_RTS on PC1 (alt. function 3) {16}
            #else
            _CONFIG_PERIPHERAL(E, 3, (PE_3_UART1_RTS | UART_PULL_UPS));  // UART1_RTS on PE3 (alt. function 3)
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            #elif defined UART1_ON_C
            _CONFIG_PERIPHERAL(C, 2, (PC_2_UART1_CTS | UART_PULL_UPS));  // UART1_CTS on PC2 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(E, 2, (PE_2_UART1_CTS | UART_PULL_UPS));  // UART1_CTS on PE2 (alt. function 3)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined THIRD_UART_CHANNEL
    case THIRD_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART2 tx pin configuration
        #if defined KINETIS_KE
            #if defined UART2_ON_I
            SIM_PINSEL1 |= (SIM_PINSEL1_UART2PS);                        // UART2_TX on PI1
            _CONFIG_PERIPHERAL(I, 1, (PI_1_UART2_TX | UART_PULL_UPS));
            #else
            _CONFIG_PERIPHERAL(D, 7, (PD_7_UART2_TX | UART_PULL_UPS));   // UART2_TX on PD7 (alt. function 2)
            #endif
        #else
            #if (defined KINETIS_K61 || defined KINETIS_K70 || defined KINETIS_K21 || defined KINETIS_KL || defined KINETIS_KV31 || defined KINETIS_KW2X || defined KINETIS_K26 || defined KINETIS_K65) && defined UART2_ON_E // {25}
            _CONFIG_PERIPHERAL(E, 16, (PE_16_UART2_TX | UART_PULL_UPS)); // UART2_TX on PE16 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART2_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 14, (PF_14_UART2_TX | UART_PULL_UPS)); // UART2_TX on PF14 (alt. function 4)
            #elif defined KINETIS_KL && defined UART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 22, (PE_22_UART2_TX | UART_PULL_UPS)); // UART2_TX on PE22 (alt. function 4)
            #elif defined KINETIS_KL && defined UART2_ON_D_HIGH
            _CONFIG_PERIPHERAL(D, 5, (PE_5_UART2_TX | UART_PULL_UPS));   // UART2_TX on PD5 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(D, 3, (PD_3_UART2_TX | UART_PULL_UPS));   // UART2_TX on PD3 (alt. function 3)
            #endif
        #endif
        #if defined irq_UART2_3_ID
            iInterruptID = irq_UART2_3_ID;                               // UART2/3 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART2_ID;                                 // UART2 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART2;
            InterruptFunc = _SCI2_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART2 rx pin configuration
        #if defined irq_UART2_3_ID
            iInterruptID = irq_UART2_3_ID;                               // UART2/3 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART2_ID;                                 // UART2 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART2;
            InterruptFunc = _SCI2_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI2_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART2;
        #endif
            if ((UART2_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined KINETIS_KE
            #if defined UART2_ON_I
            SIM_PINSEL1 |= (SIM_PINSEL1_UART2PS);                        // UART2_RX on PI0
            _CONFIG_PERIPHERAL(I, 0, (PI_0_UART2_RX | UART_PULL_UPS));
            #else
            _CONFIG_PERIPHERAL(D, 6, (PD_6_UART2_RX | UART_PULL_UPS));   // UART2_RX on PD6
            #endif
        #else
            #if (defined KINETIS_K61 || defined KINETIS_K70 || defined KINETIS_K21 || defined KINETIS_KL || defined KINETIS_KV31 || defined KINETIS_KW2X || defined KINETIS_K26 || defined KINETIS_K65) && defined UART2_ON_E // {25}
            _CONFIG_PERIPHERAL(E, 17, (PE_17_UART2_RX | UART_PULL_UPS)); // UART2_RX on PE17 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART2_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 13, (PF_13_UART2_RX | UART_PULL_UPS)); // UART2_RX on PF13 (alt. function 4)
            #elif defined KINETIS_KL && defined UART2_ON_E_HIGH
            _CONFIG_PERIPHERAL(E, 23, (PE_23_UART2_RX | UART_PULL_UPS)); // UART2_RX on PE23 (alt. function 4)
            #elif defined KINETIS_KL && defined UART2_ON_D_HIGH
            _CONFIG_PERIPHERAL(D, 4, (PE_4_UART2_RX | UART_PULL_UPS));   // UART2_RX on PD4 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(D, 2, (PD_2_UART2_RX | UART_PULL_UPS));   // UART2_RX on PD2 (alt. function 3)
            #endif
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
            #if defined UART2_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART2_MANUAL_RTS_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[2] == 0) {
                _SET_RTS_2_HIGH();
            }
            else {
                _SET_RTS_2_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[2] == 0) {
                _SET_RTS_2_LOW();
            }
            else {
                _SET_RTS_2_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL || defined UART2_MANUAL_RTS_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_2_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[2] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_2_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[2] = 1;                                        // inverted RTS mode
            }
            #elif (defined KINETIS_K61 || defined KINETIS_K70 || defined KINETIS_K21 || defined KINETIS_KW2X) && defined UART2_ON_E // {25}
            _CONFIG_PERIPHERAL(E, 19, (PE_19_UART2_RTS | UART_PULL_UPS));// UART2_RTS on PE19 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART2_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 11, (PF_11_UART2_RTS | UART_PULL_UPS));// UART2_RTS on PF11 (alt. function 4)
            #else
            _CONFIG_PERIPHERAL(D, 0, (PD_0_UART2_RTS | UART_PULL_UPS));  // UART2_RTS on PD0 (alt. function 3)
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            #elif (defined KINETIS_K61 || defined KINETIS_K70 || defined KINETIS_K21 || defined KINETIS_KW2X) && defined UART2_ON_E // {25}
            _CONFIG_PERIPHERAL(E, 18, (PE_18_UART2_CTS | UART_PULL_UPS));// UART2_CTS on PE18 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART2_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 12, (PF_12_UART2_CTS | UART_PULL_UPS));// UART2_CTS on PF12 (alt. function 4)
            #else
            _CONFIG_PERIPHERAL(D, 1, (PD_1_UART2_CTS | UART_PULL_UPS));  // UART2_CTS on PD1 (alt. function 3)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined FOURTH_UART_CHANNEL
    case FOURTH_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART3 tx pin configuration
        #if defined UART3_ON_B
            _CONFIG_PERIPHERAL(B, 11, (PB_11_UART3_TX | UART_PULL_UPS)); // UART3_TX on PB11 (alt. function 3)
        #elif defined UART3_ON_C
            _CONFIG_PERIPHERAL(C, 17, (PC_17_UART3_TX | UART_PULL_UPS)); // UART3_TX on PC17 (alt. function 3)
        #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART3_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 8, (PF_8_UART3_TX | UART_PULL_UPS));   // UART3_TX on PF8 (alt. function 4) {12}
        #else
            _CONFIG_PERIPHERAL(E, 4, (PE_4_UART3_TX | UART_PULL_UPS));   // UART3_TX on PE4 (alt. function 3)
        #endif
        #if defined irq_UART2_3_ID
            iInterruptID = irq_UART2_3_ID;                               // UART2/3 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART3_ID;                                 // UART3 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART3;
            InterruptFunc = _SCI3_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART3 rx pin configuration
        #if defined irq_UART2_3_ID
            iInterruptID = irq_UART2_3_ID;                               // UART2/3 transmitter/receiver shared interrupt vector ID
        #else
            iInterruptID = irq_UART3_ID;                                 // UART3 transmitter/receiver interrupt vector ID
        #endif
            ucPriority = PRIORITY_UART3;
            InterruptFunc = _SCI3_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI3_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART3;
        #endif
            if ((UART3_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined UART3_ON_B
            _CONFIG_PERIPHERAL(B, 10, (PB_10_UART3_RX | UART_PULL_UPS)); // UART3_RX on PB10 (alt. function 3)
        #elif defined UART3_ON_C
            _CONFIG_PERIPHERAL(C, 16, (PC_16_UART3_RX | UART_PULL_UPS)); // UART3_RX on PC16 (alt. function 3)
        #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART3_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 7, (PF_7_UART3_RX | UART_PULL_UPS));   // UART3_RX on PF7 (alt. function 4) {12}
        #else
            _CONFIG_PERIPHERAL(E, 5, (PE_5_UART3_RX | UART_PULL_UPS));   // UART3_RX on PE5 (alt. function 3)
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
            #if defined UART3_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[3] == 0) {
                _SET_RTS_3_HIGH();
            }
            else {
                _SET_RTS_3_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[3] == 0) {
                _SET_RTS_3_LOW();
            }
            else {
                _SET_RTS_3_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_3_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[3] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_3_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[3] = 1;                                        // inverted RTS mode
            }
            #elif defined UART3_ON_B
            _CONFIG_PERIPHERAL(B, 8, (PB_8_UART3_RTS | UART_PULL_UPS));  // UART3_RTS on PB8 (alt. function 3)
            #elif defined UART3_ON_C
            _CONFIG_PERIPHERAL(C, 18, (PC_18_UART3_RTS | UART_PULL_UPS));// UART3_RTS on PC18 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART3_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 9, (PF_9_UART3_RTS | UART_PULL_UPS));  // UART3_RTS on PF9 (alt. function 4) {12}
            #else
            _CONFIG_PERIPHERAL(E, 7, (PE_7_UART3_RTS | UART_PULL_UPS));  // UART3_RTS on PE7 (alt. function 3)
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            #elif defined UART3_ON_B
            _CONFIG_PERIPHERAL(B, 9, (PB_9_UART3_CTS | UART_PULL_UPS)); // UART3_CTS on PB9 (alt. function 3)
            #elif defined UART3_ON_C
            _CONFIG_PERIPHERAL(C, 19, (PC_19_UART3_CTS | UART_PULL_UPS));// UART3_CTS on PC19 (alt. function 3)
            #elif (defined KINETIS_K61 || defined KINETIS_K70) && defined UART3_ON_F // {25}
            _CONFIG_PERIPHERAL(F, 10, (PF_10_UART3_CTS | UART_PULL_UPS));// UART3_CTS on PF10 (alt. function 4) {12}
            #else
            _CONFIG_PERIPHERAL(E, 6, (PE_6_UART3_CTS | UART_PULL_UPS));  // UART3_CTS on PE6 (alt. function 3)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined FIFTH_UART_CHANNEL
    case FIFTH_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART4 tx pin configuration
        #if defined UART4_ON_C
            _CONFIG_PERIPHERAL(C, 15, (PC_15_UART4_TX | UART_PULL_UPS)); // UART4_TX on PC15 (alt. function 3)
        #else
            _CONFIG_PERIPHERAL(E, 24, (PE_24_UART4_TX | UART_PULL_UPS)); // UART4_TX on PE24 (alt. function 3)
        #endif
            iInterruptID = irq_UART4_ID;                                 // UART4 transmitter/receiver interrupt vector ID
            ucPriority = PRIORITY_UART4;
            InterruptFunc = _SCI4_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART4 rx pin configuration
            iInterruptID = irq_UART4_ID;                                 // UART4 transmitter/receiver interrupt vector ID
            ucPriority = PRIORITY_UART4;
            InterruptFunc = _SCI4_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI4_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART4;
        #endif
            if ((UART4_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined UART4_ON_C
            _CONFIG_PERIPHERAL(C, 14, (PC_14_UART4_RX | UART_PULL_UPS)); // UART4_RX on PC14 (alt. function 3)
        #else
            _CONFIG_PERIPHERAL(E, 25, (PE_25_UART4_RX | UART_PULL_UPS)); // UART4_RX on PE25 (alt. function 3)
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
            #if defined UART4_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[4] == 0) {
                _SET_RTS_4_HIGH();
            }
            else {
                _SET_RTS_4_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[4] == 0) {
                _SET_RTS_4_LOW();
            }
            else {
                _SET_RTS_4_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_4_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[4] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_4_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[4] = 1;                                        // inverted RTS mode
            }
            #elif defined UART4_ON_C
            _CONFIG_PERIPHERAL(C, 12, (PC_12_UART4_RTS | UART_PULL_UPS));// UART4_RTS on PC12 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(E, 27, (PE_27_UART4_RTS | UART_PULL_UPS));// UART4_RTS on PE27 (alt. function 3)
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            #elif defined UART4_ON_C
            _CONFIG_PERIPHERAL(C, 13, (PC_13_UART4_CTS | UART_PULL_UPS));// UART4_CTS on PC13 (alt. function 3)
            #else
            _CONFIG_PERIPHERAL(E, 26, (PE_26_UART4_CTS | UART_PULL_UPS));// UART4_CTS on PE26 (alt. function 3)
            #endif
            break;
        #endif
        }
        break;
    #endif

    #if defined SIXTH_UART_CHANNEL
    case SIXTH_UART_CHANNEL:
        switch (iPinReference) {
        case UART_TX_PIN:                                                // UART5 tx pin configuration
        #if defined UART5_ON_D
            _CONFIG_PERIPHERAL(D, 9, (PD_9_UART5_TX | UART_PULL_UPS));   // UART5_TX on PD9 (alt. function 3)
        #else
            _CONFIG_PERIPHERAL(E, 8, (PE_8_UART5_TX | UART_PULL_UPS));   // UART5_TX on PE8 (alt. function 3)
        #endif
            iInterruptID = irq_UART5_ID;                                 // UART5 transmitter/receiver interrupt vector ID
            ucPriority = PRIORITY_UART5;
            InterruptFunc = _SCI5_Interrupt;
            break;

        case UART_RX_PIN:                                                // UART5 rx pin configuration
            iInterruptID = irq_UART5_ID;                                 // UART5 transmitter/receiver interrupt vector ID
            ucPriority = PRIORITY_UART5;
            InterruptFunc = _SCI5_Interrupt;
        #if defined UART_ERROR_INTERRUPT_VECTOR                          // {1}
            InterruptErrorFunc = _SCI5_Error_Interrupt;
            ucErrorPriority = PRIORITY_ERROR_UART5;
        #endif
            if ((UART5_C1 & UART_C1_LOOPS) != 0) {                       // loop-back/single-wire mode doesn't configure/use the UARTx_Rx pin
                break;
            }
        #if defined UART5_ON_D
            _CONFIG_PERIPHERAL(D, 8, (PD_8_UART5_RX | UART_PULL_UPS));   // UART5_RX on PD8 (alt. function 3)
        #else
            _CONFIG_PERIPHERAL(E, 9, (PE_9_UART5_RX | UART_PULL_UPS));   // UART5_RX on PE9 (alt. function 3)
        #endif
            break;

        #if defined SUPPORT_HW_FLOW
            #if defined UART5_MANUAL_RTS_CONTROL
        case UART_RTS_RS485_MANUAL_MODE:
            return 1;                                                    // this UART's RTS line is manually controlled in RS485 mode
            #endif
            #if defined UART_WITHOUT_MODEM_CONTROL
        case UART_RTS_PIN_ASSERT:
            if (ucRTS_neg[5] == 0) {
                _SET_RTS_5_HIGH();
            }
            else {
                _SET_RTS_5_LOW();
            }
            break;
        case UART_RTS_PIN_NEGATE:
            if (ucRTS_neg[5] == 0) {
                _SET_RTS_5_LOW();
            }
            else {
                _SET_RTS_5_HIGH();
            }
            break;
            #endif

        case UART_RTS_PIN_INVERTED:
        case UART_RTS_PIN:
            #if defined UART_WITHOUT_MODEM_CONTROL
            if (iPinReference != UART_RTS_PIN_INVERTED) {
                _CONFIGURE_RTS_5_LOW();                                  // configure RTS output and set to '0'
                ucRTS_neg[5] = 0;                                        // not inverted RTS mode
            }
            else {
                _CONFIGURE_RTS_5_HIGH();                                 // configure RTS output and set to '1'
                ucRTS_neg[5] = 1;                                        // inverted RTS mode
            }
            #elif defined UART5_ON_D
            _CONFIG_PERIPHERAL(D, 10, (PD_10_UART5_RTS | UART_PULL_UPS));// UART5_RTS on PD10 (alt. function 3)
            #else
                #if UARTS_AVAILABLE == 5
                    #if defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 3, (PA_3_LPUART0_RTS | UART_PULL_UPS));// LPUART0_RTS on PA3 (alt. function 5)
                    #elif defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 10, (PD_10_LPUART0_RTS | UART_PULL_UPS)); // LPUART0_RTS on PD10 (alt. function 5)
                    #else
            _CONFIG_PERIPHERAL(E, 11, (PE_11_LPUART0_RTS | UART_PULL_UPS)); // LPUART0_RTS on PE11 (alt. function 5)
                    #endif
                #else
            _CONFIG_PERIPHERAL(E, 11, (PE_11_UART5_RTS | UART_PULL_UPS));// UART5_RTS on PE11 (alt. function 3)
                #endif
            #endif
            break;

        case UART_CTS_PIN:
            #if defined UART5_ON_D
            _CONFIG_PERIPHERAL(D, 11, (PD_11_UART5_CTS | UART_PULL_UPS)); // UART5_CTS on PD11 (alt. function 3)
            #else
                #if UARTS_AVAILABLE == 5
                    #if defined LPUART0_ON_A
            _CONFIG_PERIPHERAL(A, 0, (PA_0_LPUART0_CTS | UART_PULL_UPS)); // LPUART0_CTS on PA0 (alt. function 5)
                    #elif defined LPUART0_ON_D
            _CONFIG_PERIPHERAL(D, 11, (PD_11_LPUART0_CTS | UART_PULL_UPS)); // LPUART0_CTS on PD11 (alt. function 5)
                    #else
            _CONFIG_PERIPHERAL(E, 10, (PE_10_LPUART0_CTS | UART_PULL_UPS)); // LPUART0_CTS on PE10 (alt. function 5)
                    #endif
                #else
            _CONFIG_PERIPHERAL(E, 10, (PE_10_UART5_CTS | UART_PULL_UPS)); // UART5_CTS on PE10 (alt. function 3)
                #endif
            #endif
            break;
        #endif
        }
        break;
    #endif

    default:
        _EXCEPTION("UART doesn't exist!!");
        return 0;
    }

    if (InterruptFunc != 0) {                                            // if the interrupt handler is to be installed
        fnEnterInterrupt(iInterruptID, ucPriority, InterruptFunc);       // enter UART/LPUART interrupt handler
    #if defined UART_ERROR_INTERRUPT_VECTOR                              // {1}
        if (InterruptErrorFunc != 0) {
            fnEnterInterrupt((iInterruptID + 1), ucErrorPriority, InterruptErrorFunc); // enter UART error interrupt handler
        }
    #endif
    }
    return 0;
}
