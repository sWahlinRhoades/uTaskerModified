/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      Port_Interrupts.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    28.10.2009 Modify SAM7X initialisation to include glitch filer configuration {1}
    30.12.2009 Modify M522xx use of IRQ7 to NMI technique                 {2}
    10.02.2010 NMI port check only with _M5223X                           {3}
    21.04.2011 Update calls from sendCAN() to fnSendCAN()                 {4}
    04.12.2011 Add Kinetis test support                                   {5}
    11.01.2015 Add wake-up test support                                   {6}
    11.12.2015 Add DMA port mirroring reference                           {7}
    19.10.2017 Add DMA_SPI_BURST reference (DMA port trigger of SPI Tx/Rx sequence controlled by DMA) {8}
    20.01.2019 Add iMX
    19.02.2019 fnSendCAN() removed
    Note that the external interrupt tests are not suitable for LPC210x as in this file

    The file is otherwise not specifically linked in to the project since it
    is included by application.c when needed.

    See the following video showing port interrupt operation on a KL27: https://youtu.be/CubinvMuTwU

*/

#if !defined _PORT_INTS_CONFIG
    #define _PORT_INTS_CONFIG

    #if !defined K70F150M_12M && !defined TWR_K53N512 && !defined TWR_K40X256 && !defined TWR_K40D100M && !defined KWIKSTIK
      //#define IRQ_TEST                                                 // test IRQ port interrupts
          //#define DMA_PORT_MIRRORING                                   // demonstrate using DMA to control one or more output ports to follow an input port (only available on Kinetis)
      //#define DMA_SPI_BURST                                            // {8} demonstrate input port triggering of an SPI burst using DMA
        #if defined SUPPORT_LOW_POWER && defined IRQ_TEST
          //#define WAKEUP_TEST                                          // test wake-up port interrupts (wake-up from kinetis low leakage mode)
        #endif
    #endif
    #if !defined HYSTERESIS_ON
        #define HYSTERESIS_ON        0                                   // devices without input hysteresis control ignore this setting
    #endif

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */
    #if defined IRQ_TEST || defined WAKEUP_TEST
        static void fnInitIRQ(void);

        #if defined _M5223X                                              // {2}
            static unsigned long ulNMI_event_count = 0;
            static unsigned long ulNMI_processed_count = 0;
        #endif
    #endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

    #if defined PORT_INTERRUPT_USER_DISPATCHER && defined IRQ_TEST
        static int iLastPin = 0;
    #endif
#endif

#if defined _M5223X && defined _PORT_NMI_CHECK && defined IRQ_TEST       // {2}{3} check for NMI interrupt each time the application is scheduled
        while (ulNMI_event_count != ulNMI_processed_count) {             // if there are open events
            ulNMI_processed_count++;                                     // this one processed - note that ulNMI_event_count is not written since it may be modified by the NMI during such an access
            fnDebugMsg("NMI_int\r\n");
        }
#endif

#if defined _PORT_INTS_EVENTS && defined IRQ_TEST                        // monostable timer event handling (included in input queue handler of application task)
                if ((IRQ1_EVENT <= ucInputMessage[MSG_INTERRUPT_EVENT]) && (IRQ11_EVENT >= ucInputMessage[MSG_INTERRUPT_EVENT])) { // interrupt events of interest
    #if defined WAKEUP_TEST
                    fnDebugMsg("WOKEN - ");
                    switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {
                    case IRQ4_EVENT:
                        fnDebugMsg("going to LSS again\r\n");
                        break;
                    case IRQ5_EVENT:
                        fnSetLowPowerMode(WAIT_MODE);
                        fnDebugMsg("restoring WAIT mode\r\n");
                        break;
                    }
    #else 
        #if !(defined TRK_KEA8 && defined SUPPORT_LOW_POWER)
                    fnDebugMsg("IRQ_");
        #endif
                    switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {
                    case IRQ1_EVENT:
                        fnDebugMsg("1");
                        break;
                    case IRQ4_EVENT:
        #if defined TRK_KEA8 && defined SUPPORT_LOW_POWER
                        switch (fnGetLowPowerMode()) {
                        case STOP_MODE:
                            fnSetLowPowerMode(RUN_MODE);
                            fnDebugMsg("RUN");
                            break;
                        case RUN_MODE:
                            fnSetLowPowerMode(WAIT_MODE);
                            fnDebugMsg("WAIT");
                            break;
                        default:
                            fnSetLowPowerMode(STOP_MODE);
                            fnDebugMsg("STOP");
                            break;
                        }
                        fnDebugMsg(" mode\r\n");
        #else
            #if defined PORT_INTERRUPT_USER_DISPATCHER
                        fnDebugDec(iLastPin, 0);
            #else
                        fnDebugMsg("4");
            #endif
        #endif
                        break;
                    case IRQ5_EVENT:
                        fnDebugMsg("5");
                        break;
                    case IRQ7_EVENT:
                        fnDebugMsg("7");
                        break;
                    case IRQ11_EVENT:
                        fnDebugMsg("11");
                        break;
                    default:
                        break;
                    }
                    fnDebugMsg("\r\n");
                    break;
    #endif
                }
#endif




#if defined _PORT_INT_CODE && defined IRQ_TEST
// Test routines to handle the IRQ test inputs
//
    #if !defined TEST_DS1307 && !(defined _M5225X && !defined INTERRUPT_TASK_PHY) && !defined _KINETIS && !defined _STM32 && !defined _iMX // use this input for RTC
static void test_irq_1(void)
{
    fnInterruptMessage(OWN_TASK, IRQ1_EVENT);                            // send an interrupt event to the task
}
    #endif

#if defined PORT_INTERRUPT_USER_DISPATCHER
static void __callback_interrupt test_irq_4(int iPinRef)
#else
static void __callback_interrupt test_irq_4(void)
#endif
{
#if defined PORT_INTERRUPT_USER_DISPATCHER
    iLastPin = iPinRef;
#endif
    fnInterruptMessage(OWN_TASK, IRQ4_EVENT);
    #if defined _KINETIS && !defined KINETIS_KE
  //_DIS_ARM_PORT_INTERRUPT(A, 19);                                      // example of disabling the interrupt
  //_RE_ARM_PORT_INTERRUPT(A, 19, PORT_IRQC_RISING);                     // example of re-enabling or changing its sensitivity
    #endif
}
    #if !defined M52259DEMO && !defined _LPC23XX && !defined _LPC17XX && !defined _STM32
#if defined PORT_INTERRUPT_USER_DISPATCHER
static void __callback_interrupt test_irq_5(int iPinRef)
#else
static void __callback_interrupt test_irq_5(void)
#endif
{
    fnInterruptMessage(OWN_TASK, IRQ5_EVENT);                            // send an interrupt event to the task
}
    #endif
    #if defined _M5223X                                                  // {2}
static void test_nmi_7(void)
{
    // The M522XX irq7 has NMI characteristics and so should avoid operating system calls involving queues
    // - this technique shows a safe method of achieving the same effect
    //
    ulNMI_event_count++;                                                 // mark that new event has occurred
    uTaskerStateChange(OWN_TASK, UTASKER_ACTIVATE);                      // safely schedule the task to handle the event
}
    #elif !defined _LPC23XX && !defined _LPC17XX && !defined _KINETIS && !defined _STM32 && !defined _iMX
static void test_irq_7(void)
{
    fnInterruptMessage(OWN_TASK, IRQ7_EVENT);                            // send an interrupt event to the task
}
    #endif
    #if !defined _M521X && !defined _KINETIS && !defined _STM32 && !defined _iMX
static void test_irq_11(void)
{
    fnInterruptMessage(OWN_TASK, IRQ11_EVENT);                            // send an interrupt event to the task
}
    #endif



    #if defined DMA_SPI_BURST && (defined FRDM_K64F || defined TWR_K80F150M) // {8}
static void spi_half_buffer(void)
{
    // This is called each time the SPI Rx buffer is half-full or full so that the previous half buffer content can be retrieved while the next half is still being filled
    //
}
    #endif

// Configure several IRQ inputs to demonstrate port change/wakeup interrupts
//
static void fnInitIRQ(void)
{
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    #if defined _KINETIS || defined _iMX                                 // {5}
    interrupt_setup.int_type       = PORT_INTERRUPT;                     // identifier to configure port interrupt
    interrupt_setup.int_handler    = test_irq_4;                         // handling function
        #if defined _iMX
    interrupt_setup.int_priority   = PRIORITY_PORT_1_HIGH;               // interrupt priority level
    interrupt_setup.int_port       = PORT1;                              // the port that the interrupt input is on
            #if defined MIMXRT1064
    interrupt_setup.int_port_bits  = (PIN_GPIO_AD_B1_00_GPIO1_IO16 |     // the IRQ inputs connected J24-10
                                    PIN_GPIO_AD_B1_01_GPIO1_IO17 |       // J24-9
                                    PIN_GPIO_AD_B1_03_GPIO1_IO19);       // J22-8
            #elif defined MIMXRT1015
    interrupt_setup.int_port_bits = (PIN_GPIO_AD_B1_14_GPIO1_IO30 |      // the IRQ inputs connected J19-10
                                    PIN_GPIO_AD_B1_15_GPIO1_IO31 |       // J19-9
                                    PIN_GPIO_AD_B0_10_GPIO1_IO10);       // J19-6
            #else
    interrupt_setup.int_port_bits = (PORT1_BIT26);                       // the IRQ input connected
            #endif
        #elif defined K02F100M
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
            #endif
    interrupt_setup.int_port_bits  = PORTC_BIT6;                         // the IRQ input connected (SWITCH_3 on FRDM-KL43Z) - LLWU_P7
        #elif defined FRDM_KL46Z || defined FRDM_KL43Z || defined TWR_KL43Z48M
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
            #if defined FRDM_KL43Z || defined TWR_KL43Z48M
                #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
                #endif
    interrupt_setup.int_port_bits  = PORTC_BIT3;                         // the IRQ input connected (SWITCH_3 on FRDM-KL43Z) - LLWU_P7
            #else
    interrupt_setup.int_port_bits  = PORTC_BIT12;                        // the IRQ input connected (SWITCH_3 on FRDM-KL46Z)
            #endif
        #elif defined TRK_KEA8
    // Keyboard
    //
    interrupt_setup.int_type       = KEYBOARD_INTERRUPT;                 // define keyboard interrupt rather than IRQ
    interrupt_setup.int_priority   = PRIORITY_KEYBOARD_INT;              // interrupt priority level
    interrupt_setup.int_port       = KE_PORTC;                           // the port that the interrupt input is on (KE_PORTA, KE_PORTB, KE_PORTC and KE_PORTD are the same)
    interrupt_setup.int_port_bits  = (KE_PORTC_BIT4);                    // the IRQ input connected (switch 1)
        #elif defined FRDM_KE02Z || defined FRDM_KE02Z40M || defined TRK_KEA64 || defined TRK_KEA128 || defined FRDM_KEAZN32Q64 || defined FRDM_KEAZ64Q64 || defined FRDM_KEAZ128Q80
    // Keyboard
    //
  //interrupt_setup.int_type       = KEYBOARD_INTERRUPT;                 // define keyboard interrupt rather than IRQ
  //interrupt_setup.int_priority   = PRIORITY_KEYBOARD_INT;              // interrupt priority level
  //interrupt_setup.int_port       = KE_PORTD;                           // the port that the interrupt input is on (KE_PORTA, KE_PORTB, KE_PORTC and KE_PORTD are the same)
  //interrupt_setup.int_port_bits  = (KE_PORTD_BIT5 | KE_PORTB_BIT3);    // the IRQs input connected
    // IRQ
    //
    interrupt_setup.int_priority   = PRIORITY_PORT_IRQ_INT;              // interrupt priority level
    interrupt_setup.int_port       = KE_PORTA;                           // the port that the interrupt input is on (when using PTA5 as IRQ SIM_SOPT_KE_DEFAULT must be configured to disable the reset function on the pin)
    interrupt_setup.int_port_bits  = KE_PORTA_BIT5;                      // the IRQ input connected
        #elif defined FRDM_KE04Z
    // Keyboard
    //
    interrupt_setup.int_type       = KEYBOARD_INTERRUPT;                 // define keyboard interrupt rather than IRQ
    interrupt_setup.int_priority   = PRIORITY_KEYBOARD_INT;              // interrupt priority level
    interrupt_setup.int_port       = KE_PORTA;                           // the port that the interrupt input is on (KE_PORTA, KE_PORTB, KE_PORTC and KE_PORTD are the same)
    interrupt_setup.int_port_bits  = (KE_PORTA_BIT1 | KE_PORTA_BIT2);    // the IRQs input connected
    // IRQ
    //
  //interrupt_setup.int_priority   = PRIORITY_PORT_IRQ_INT;              // interrupt priority level
  //interrupt_setup.int_port       = KE_PORTA;                           // the port that the interrupt input is on (when using PTA5 as IRQ SIM_SOPT_KE_DEFAULT must be configured to disable the reset function on the pin)
  //interrupt_setup.int_port_bits  = KE_PORTA_BIT5;                      // the IRQ input connected
    //
  //interrupt_setup.int_port       = KE_PORTI;                           // the port that the interrupt input is on
  //interrupt_setup.int_port_bits  = KE_PORTI_BIT6;                      // the IRQ input connected
        #elif defined FRDM_KE06Z
    // Keyboard
    //
    interrupt_setup.int_type       = KEYBOARD_INTERRUPT;                 // define keyboard interrupt rather than IRQ
    interrupt_setup.int_priority   = PRIORITY_KEYBOARD_INT;              // interrupt priority level
  //interrupt_setup.int_port       = KE_PORTD;                           // the port that the interrupt input is on (KE_PORTA, KE_PORTB, KE_PORTC and KE_PORTD are the same)
    interrupt_setup.int_port       = KE_PORTH;                           // the port that the interrupt input is on (KE_PORTE, KE_PORTF, KE_PORTG and KE_PORTH are the same)
    interrupt_setup.int_port_bits  = (KE_PORTH_BIT3 | KE_PORTH_BIT4 | KE_PORTH_BIT5 | KE_PORTH_BIT6 | KE_PORTH_BIT7); // the IRQs input connected
        #elif defined TWR_K24F120M || defined TWR_K64F120M || defined FRDM_K64F || defined TWR_K21F120M || defined TWR_K22F120M || defined TEENSY_3_1 || defined TWR_K21D50M
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
            #endif
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTC_BIT6;                         // the IRQ input connected (SWITCH_2 on TWR_K24F120M and FRDM-K64F/SWITCH_1 on TWR_K24F120M/SW1 on TWR_K22F120M/Pin 11 on TEENSY3.1/SW3 on TWR_K21D50M) LLWU_P10
            #if defined TWR_K21F120M || defined TWR_K22F120M || defined TWR_K21D50M
    interrupt_setup.int_handler    = test_irq_5;
            #endif
        #elif defined FRDM_K20D50M
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
    interrupt_setup.int_priority   = PRIORITY_PORT_D_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTD;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTD_BIT0;                         // the IRQ input connected - LLWU_P12
            #endif
        #elif defined TWR_K20D50M
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTC_BIT2;                         // the IRQ input connected (SWITCH_3 on TWR-K20D50M)
        #elif defined TWR_K20D72M || defined TWR_KL46Z48M
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
            #endif
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
            #if defined TWR_KL46Z48M
    interrupt_setup.int_port_bits  = PORTC_BIT3;                         // (SW2 on TWR-KL46Z48M) LLWU_P7
            #else
    interrupt_setup.int_port_bits  = PORTC_BIT1;                         // (SW1 on TWR-K20D72M) LLWU_P6
            #endif
        #elif defined FRDM_KL26Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27 || defined FRDM_K22F || defined TWR_K53N512 || defined TWR_K40D100M
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
  //fnEnterNMI(test_irq_5);
            #endif
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
            #if defined FRDM_K22F || defined FRDM_KL27Z
    interrupt_setup.int_port_bits  = PORTC_BIT1;                         // the IRQ input connected (SW2 on FRDM-K22F) LLWU_P6 (SW3 on FRDM-KL27Z)
                #if defined FRDM_KL27Z
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON);     // interrupt is to be falling edge sensitive
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
    interrupt_setup.int_port_bits = PORTC_BIT4;
    interrupt_setup.int_handler = test_irq_4;
                #endif
            #elif defined TWR_K53N512 || defined TWR_K40D100M
    interrupt_setup.int_port_bits  = PORTC_BIT5;                         // the IRQ input connected (SW1 on TWR-K53N512 and TWR-K40D100M) LLWU_P9
            #else
    interrupt_setup.int_port_bits  = PORTC_BIT3;                         // the IRQ input connected (SW1 on FRDM-KL26Z) LLWU_P7
            #endif
        #elif defined FRDM_KL25Z
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
    interrupt_setup.int_priority   = PRIORITY_PORT_C_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTC_BIT3;                         // J1-5 on FRDM-KL25Z
            #else
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTA_BIT16;                        // J2-9 on FRDM-KL25Z
    interrupt_setup.int_priority   = PRIORITY_PORT_A_INT;                // interrupt priority level
            #endif
        #elif defined FRDM_KL02Z || defined FRDM_KL03Z || defined FRDM_KE15Z
            #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_port_bits  = SWITCH_2;                           // PTB0
            #else
                #if defined FRDM_KE15Z
    interrupt_setup.int_port_bits  = SWITCH_2;                           // SW2 (PTB11) on FRDM-KE15Z
                #elif defined FRDM_KL02Z
    interrupt_setup.int_port_bits  = PORTB_BIT5;
                #else
    interrupt_setup.int_port_bits  = PORTB_BIT7;                         // J1-6 on FRDM-KL03Z
                #endif
            #endif
    interrupt_setup.int_priority   = PRIORITY_PORT_B_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTB;                              // the port that the interrupt input is on
        #else
    interrupt_setup.int_priority   = PRIORITY_PORT_A_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
            #if defined FRDM_KL05Z || defined TEENSY_LC
                #if defined WAKEUP_TEST
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
        #if defined TEENSY_LC
    interrupt_setup.int_port_bits  = PORTB_BIT0;
        #else
    interrupt_setup.int_handler    = test_irq_5;
    interrupt_setup.int_port_bits  = PORTA_BIT7;                         // PTA7 (LLWU_P3) on J7-4 of FRDM-KL05Z
        #endif
                #else
    interrupt_setup.int_port_bits  = PORTA_BIT5;                         // the IRQ input connected (LLWU_P1)
                #endif
            #elif defined TWR_KV31F120M || defined TWR_KM34Z50M
    interrupt_setup.int_port_bits  = PORTA_BIT4;                         // the IRQ input connected (SWITCH_3 on TWR_KV31F120M)
            #elif defined FRDM_K66F
    interrupt_setup.int_port_bits = PORTA_BIT10;                         // the IRQ input connected (SW3 on FRDM-K66F)
            #elif defined TWR_KV58F220M
    interrupt_setup.int_port_bits = PORTA_BIT4;                          // the IRQ input connected (SWITCH_2 on TWR_KV58F220M)
            #else
    interrupt_setup.int_port_bits  = PORTA_BIT19;                        // the IRQ input connected (SWITCH_1 on TWR_K60N512)
            #endif
        #endif
        #if defined _iMX && defined DMA_PORT_MIRRORING && defined SUPPORT_XBAR
    interrupt_setup.int_port_sense = (IRQ_BOTH_EDGES | PULLUP_ON | PORT_DMA_MODE); // DMA on both edges
    interrupt_setup.int_handler = 0;                                     // no interrupt handler when using DMA
    interrupt_setup.xbar_output = 2;                                     // the XBAR1 output used to perform the DMA trigger (0..3 possible)
            #if defined MIMXRT1010
    interrupt_setup.int_port = PORT1;
    interrupt_setup.int_port_bits = PORT1_BIT28;
            #elif defined iMX_RT105X || defined iMX_RT106X
    interrupt_setup.int_port = PORT4;
    interrupt_setup.int_port_bits = PORT4_BIT3;
            #else
    interrupt_setup.int_port = PORT2;
    interrupt_setup.int_port_bits = PORT2_BIT4;
                #if defined MIMXRT1020_                                  // experimental XBAR code
    POWER_UP_ATOMIC(2, XBAR1_CLOCK);                                     // ensure that XBAR1 is powered
    IOMUXC_GPR_GPR6 &= ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4);         // ensure XBAR_INOUT04 is an input
    _CONFIG_PERIPHERAL(GPIO_EMC_04, XBAR1_INOUT04, PORT_DSE_MID);        // select XBAR_INOUT04 on GPIO2_IO04 (GPIO_EMC_04) alt. function 1
    IOMUXC_GPR_GPR6 |= (IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5);          // set XBAR_INOUT05 as an output
    _CONFIG_PERIPHERAL(GPIO_EMC_05, XBAR1_INOUT05, PORT_DSE_MID);        // select XBAR_INOUT05 on GPIO2_IO05 (GPIO_EMC_05) alt. function 1
  //XBAR1_IOMUX_XBAR_OUT05(XBAR1_IOMUX_XBAR_INOUT04_IN);                 // connect the XBAR_IN04 input to XBAR_OUT05
  //XBAR1_IOMUX_XBAR_OUT05(XBAR1_PIT_TRIGGER0);                          // connect PIT0 trigger to XBAR_OUT5
                    #if 1                                                // experimental AOI code
    POWER_UP_ATOMIC(2, XBAR1_CLOCK);                                     // ensure that XBAR1 is powered
    POWER_UP_ATOMIC(2, XBAR2_CLOCK);                                     // ensure that XBAR2 is powered
    XBAR2_OUT_AOI1_IN00(XBAR2_PIT_TRIGGER0);                             // connect the PIT trigger 0 to AOI1 A output
    XBAR2_OUT_AOI1_IN01(XBAR2_PIT_TRIGGER1);                             // connect the PIT trigger 1 to AOI1 B output
    XBAR2_OUT_AOI1_IN02(XBAR2_DMA_DONE5);                                // connect the DMA DONE 5 to AOI1 C output
  //XBAR2_OUT_AOI1_IN03(XBAR2_ACMP1_OUT);                                // connect the ACMP1 output to AOI1 D output
    CONFIGURE_LOGIC_AOI1_EVENT(0,
        (AOI_A_CONNECTED + AOI_B_HIGH + AOI_C_HIGH + AOI_D_HIGH),        // (a AND b AND c AND d) OR
        (AOI_A_HIGH + AOI_B_CONNECTED + AOI_C_HIGH + AOI_D_HIGH),        // (a AND b AND c AND d) OR
        (AOI_A_HIGH + AOI_B_HIGH + AOI_C_CONNECTED + AOI_D_HIGH),        // (a AND b AND c AND d) OR
        (AOI_A_LOW + AOI_B_LOW + AOI_C_LOW + AOI_D_LOW));                // (a AND b AND c AND d)
    XBAR1_IOMUX_XBAR_OUT05(XBAR1_AOI1_OUT0);                             // connect the AOI1 channel 0 output to XBAR_OUT05
                    #endif
                #endif
            #endif
    {
        // Configure the DMA trigger from an input pin change to toggle an alternative port so that the input signal is mirrored to that output without CPU intervention
        //
        static const unsigned long ulOutput = PORT1_BIT15;               // the output to be mirrored to
        _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (PORT1_BIT15), (0), (PORT_SRE_SLOW | PORT_DSE_MID)); // ensure that the output is enabled, initially driving '0'
        fnConfigDMA_buffer(5, DMAMUX0_CHCFG_SOURCE_XBAR1_2, sizeof(ulOutput), (void *)&ulOutput, (void *)&(((iMX_GPIO *)GPIO1_BLOCK)->GPIO_DR_TOGGLE), (DMA_FIXED_ADDRESSES | DMA_LONG_WORDS), 0, 0); // use DMA channel 5 without any interrupts (free-runnning) - triggered from XBAR output 2
        fnDMA_BufferReset(5, DMA_BUFFER_START);                          // enable the DMA operation
    }
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt (DMA trigger)
        #elif defined DMA_PORT_MIRRORING && (defined FRDM_K64F || defined FRDM_KL82Z) // {7}
    interrupt_setup.int_port       = PORTB;                              // the port that the interrupt input is on
            #if defined FRDM_KL82Z
    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(C, (PORTC_BIT16), (0), (PORT_SRE_FAST | PORT_DSE_HIGH));
    interrupt_setup.int_port_bits = PORTB_BIT0;
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLDOWN_ON | PORT_DMA_MODE); // DMA on rising edge
            #else
    interrupt_setup.int_port_bits  = PORTB_BIT16;                        // UART input pin on FRDM-K64F
    interrupt_setup.int_port_sense = (IRQ_BOTH_EDGES | PULLUP_ON | PORT_DMA_MODE | PORT_KEEP_PERIPHERAL); // DMA on both edges and keep peripheral function
            #endif
    interrupt_setup.int_handler = 0;                                     // no interrupt handler when using DMA
    {
        // Configure the DMA trigger from the UART input pin change to toggle an alternative port so that the input signal is mirrored to that output without CPU intervention
        //
        static const unsigned long ulOutput = PORTC_BIT16;               // the output to be mirrored to
        fnConfigDMA_buffer(5, DMAMUX0_CHCFG_SOURCE_PORTB, sizeof(ulOutput), (void *)&ulOutput, (void *)&(((GPIO_REGS *)GPIOC_ADD)->PTOR), (DMA_FIXED_ADDRESSES | DMA_LONG_WORDS), 0, 0); // use DMA channel 5 without any interrupts (free-runnning)
        fnDMA_BufferReset(5, DMA_BUFFER_START);                          // enable the DMA operation
    }
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
        #elif defined DMA_SPI_BURST && (defined FRDM_K64F || defined TWR_K80F150M) // {8}
    // Initialise SPI interface
    //
    POWER_UP_ATOMIC(6, SPI0);
    _CONFIG_PERIPHERAL(D, 0, (PD_0_SPI0_PCS0 | PORT_SRE_FAST | PORT_DSE_HIGH));
    _CONFIG_PERIPHERAL(D, 1, (PD_1_SPI0_SCK | PORT_SRE_FAST | PORT_DSE_HIGH));
    _CONFIG_PERIPHERAL(D, 2, (PD_2_SPI0_SOUT | PORT_SRE_FAST | PORT_DSE_HIGH));
    _CONFIG_PERIPHERAL(D, 3, (PD_3_SPI0_SIN));
    SPI0_MCR = (SPI_MCR_MSTR | SPI_MCR_DCONF_SPI | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF | SPI_MCR_PCSIS_CS0 | SPI_MCR_PCSIS_CS1 | SPI_MCR_PCSIS_CS2 | SPI_MCR_PCSIS_CS3 | SPI_MCR_PCSIS_CS4 | SPI_MCR_PCSIS_CS5);
    SPI0_RSER = (SPI_SRER_TFFF_DIRS | SPI_SRER_TFFF_RE | SPI_SRER_RFDF_DIRS | SPI_SRER_RFDF_RE); // enable rx and tx DMA requests
    SPI0_CTAR0 = (SPI_CTAR_DBR | SPI_CTAR_FMSZ_8 | SPI_CTAR_PDT_7 | SPI_CTAR_BR_4 | SPI_CTAR_CPHA | SPI_CTAR_CPOL); // for 50MHz bus, 25MHz speed and 140ns min de-select time

    interrupt_setup.int_port       = PORTE;                              // the port that the interrupt/dma input is on
    #if defined TWR_K80F150M
    interrupt_setup.int_port_bits = PORTE_BIT4;
    #else
    interrupt_setup.int_port_bits  = PORTE_BIT24;
    #endif
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON | PORT_DMA_MODE); // DMA on falling edge
    interrupt_setup.int_handler = 0;                                     // no interrupt handler when using DMA
    {
        // Configure the DMA trigger from an input pin edge to start an SPI transfer
        //
        #define DMA_CHANNEL_FOR_CS_END    6                              // use DMA channel 6 for CS end trigger
        #define DMA_CHANNEL_FOR_PORT_EDGE 7                              // use DMA channel 7 for SPI sequence start trigger
        #define DMA_CHANNEL_FOR_SPI_TX    8                              // use DMA channel 8 for SPI Tx
        #define DMA_CHANNEL_FOR_SPI_RX    9                              // use DMA channel 9 for SPI Rx
        static const unsigned long ulSPI_TX[8] = {                       // fixed SPI transmission (0x01, 0x02,.. 0x08) with CS asserted throughout the frame
                (0x01 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x02 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x03 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x04 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x05 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x06 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x07 | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0),
                (0x08 | SPI_PUSHR_EOQ  | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0), // final byte negates CS after transmission
        };
        static volatile unsigned char ucRxData[128] = {0};               // circular reception buffer
        static const unsigned char ucDMA_start = ((DMA_ERQ_ERQ0 << (DMA_CHANNEL_FOR_SPI_TX - 8)) | (DMA_ERQ_ERQ0 << (DMA_CHANNEL_FOR_SPI_RX - 8))); // the value to be written to start the SPI DMA transfer
        static const unsigned long ulSPI_clear = (SPI_SR_RFDF | SPI_SR_RFOF | SPI_SR_TFUF | SPI_SR_EOQF | SPI_SR_TCF); // this is written to the SPI status register after the CS negates in order to clear flags and allow subsequent transfers

        fnConfigDMA_buffer(DMA_CHANNEL_FOR_SPI_TX,    DMAMUX0_CHCFG_SOURCE_SPI0_TX, sizeof(ulSPI_TX),    (void *)ulSPI_TX,       (void *)SPI0_PUSHR_ADDR,                      (DMA_DIRECTION_OUTPUT | DMA_LONG_WORDS | DMA_SINGLE_CYCLE), 0, 0); // source is the tx buffer and destination is the SPI transmit register without interrupts (free-running)
        fnConfigDMA_buffer(DMA_CHANNEL_FOR_SPI_RX,    DMAMUX0_CHCFG_SOURCE_SPI0_RX, sizeof(ucRxData),    (void *)SPI0_POPR_ADDR, (void *)ucRxData,                             (DMA_DIRECTION_INPUT | DMA_BYTES | DMA_HALF_BUFFER_INTERRUPT), spi_half_buffer, PRIORITY_DMA9); // source is the SPI reception register and destination is the input buffer with interrupt at half- and full buffer
        fnConfigDMA_buffer(DMA_CHANNEL_FOR_PORT_EDGE, DMAMUX0_CHCFG_SOURCE_PORTE,  sizeof(ucDMA_start), (void *)&ucDMA_start,   (void *)(((unsigned char *)DMA_ERQ_ADDR) + 1), (DMA_FIXED_ADDRESSES | DMA_BYTES), 0, 0); // use DMA channel without any interrupts (free-runnning)
        fnDMA_BufferReset(DMA_CHANNEL_FOR_PORT_EDGE,  DMA_BUFFER_START); // enable the DMA operation - a falling edge on the port will now trigger SPI Tx and Rx DMA operation
        fnConfigureInterrupt((void *)&interrupt_setup);                  // configure cs port DMA (falling edge)
        interrupt_setup.int_port = PORTD;                                // the port that the interrupt input is on
        interrupt_setup.int_port_bits = PORTD_BIT0;
        interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLUP_ON | PORT_DMA_MODE | PORT_KEEP_PERIPHERAL); // DMA on rising edge (keep CS peripheral to trigger on the end of a transfer
        fnConfigDMA_buffer(DMA_CHANNEL_FOR_CS_END, DMAMUX0_CHCFG_SOURCE_PORTD, sizeof(ulSPI_clear), (void *)&ulSPI_clear, (void *)SPI0_SR_ADDR, (DMA_FIXED_ADDRESSES | DMA_LONG_WORDS), 0, 0); // use DMA channel without any interrupts (free-runnning)
        fnDMA_BufferReset(DMA_CHANNEL_FOR_CS_END, DMA_BUFFER_START);     // enable the DMA operation - a rising edge on the port will now clear the the SPI status register
        fnConfigureInterrupt((void *)&interrupt_setup);                  // configure cs port DMA (rising edge)
    }
        #else
            #if defined FRDM_KL25Z || defined FRDM_KL05Z || defined FRDM_KL27Z
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON | ENABLE_PORT_MODE); // set the pin to port mode - this is needed if the pin is disabled by default otherwise the pull-up/LLWU functions won't work
            #elif defined TRK_KEA8
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLUP_DOWN_OFF);// interrupt is to be rising edge sensitive
            #else
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON);     // interrupt is to be falling edge sensitive
            #endif
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
            #if defined KINETIS_KE06
    // IRQ
    //
    interrupt_setup.int_type = PORT_INTERRUPT;
    interrupt_setup.int_priority   = PRIORITY_PORT_IRQ_INT;              // interrupt priority level
  //interrupt_setup.int_port       = KE_PORTA;                           // the port that the interrupt input is on (when using PTA5 as IRQ SIM_SOPT_KE_DEFAULT must be configured to disable the reset function on the pin)
  //interrupt_setup.int_port_bits  = KE_PORTA_BIT5;                      // the IRQ input connected
    //
    interrupt_setup.int_port       = KE_PORTI;                           // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = KE_PORTI_BIT6;                      // the IRQ input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
            #endif
        #endif
        #if (defined DMA_PORT_MIRRORING || defined DMA_SPI_BURST) && (defined FRDM_K64F || defined TWR_K80F150M) // {7} put back the standard port sense
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON);     // interrupt is to be falling edge sensitive
        #endif
        #if (PORTS_AVAILABLE > 4) && (!defined KINETIS_KL || defined TEENSY_LC) && !defined TWR_K22F120M && !defined TWR_K20D50M && !defined TWR_K20D72M && !defined TWR_K53N512 && !defined TWR_K40D100M && !defined TWR_K21D50M && !defined TWR_K21F120M  && !defined FRDM_KE15Z
    interrupt_setup.int_handler    = test_irq_5;                         // handling function
            #if defined _iMX
                #if defined MIMXRT1064
    interrupt_setup.int_port = PORT1;                                    // the port that the interrupt input is on
    interrupt_setup.int_port_bits = PIN_GPIO_AD_B0_03_GPIO1_IO03;        // the input to generate an interrupt (J24-1)
    interrupt_setup.int_priority = PRIORITY_PORT_1_3;                    // interrupt priority level
                #elif defined MIMXRT1050
    interrupt_setup.int_port = PORT2;                                    // the port that the interrupt input is on
    interrupt_setup.int_port_bits = PIN_GPIO_B1_06_GPIO2_IO22;           // the input to generate an interrupt
    interrupt_setup.int_priority = PRIORITY_PORT_2_HIGH;                 // interrupt priority level
                #elif defined MIMXRT1010
    interrupt_setup.int_port = PORT1;                                    // the port that the interrupt input is on
    interrupt_setup.int_port_bits = PIN_GPIO_01_GPIO1_IO01;              // the input to generate an interrupt
    interrupt_setup.int_priority = PRIORITY_PORT_1_LOW;                  // interrupt priority level
                #else
    interrupt_setup.int_port = PORT3;                                    // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORT3_BIT0;                         // the input to generate an interrupt
    interrupt_setup.int_priority = PRIORITY_PORT_3_LOW;                  // interrupt priority level
                #endif
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLDOWN_ON | HYSTERESIS_ON); // interrupt is to be rising edge sensitive
            #elif defined TWR_K24F120M
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTA_BIT13;                        // LLWU_P4
            #elif defined TWR_K64F120M || defined FRDM_K64F
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTA_BIT4;                         // (SWITCH_3 on TWR_K24F120M and FRDM-K64F) LLWU_P3
            #elif defined FRDM_K66F
    interrupt_setup.int_port       = PORTD;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTD_BIT11;                        // SW2 on FRDM-K66F
            #elif defined TEENSY_3_1
    interrupt_setup.int_port       = PORTD;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTD_BIT4;                         // (pin 6) LLWU_P14
            #elif defined TEENSY_LC
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTC_BIT6;                         // (pin 16) LLWU_P5
            #else
    interrupt_setup.int_type       = PORT_INTERRUPT;
    interrupt_setup.int_priority   = PRIORITY_PORT_E_INT;                // interrupt priority level
    interrupt_setup.int_port       = PORTE;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTE_BIT26;                        // the IRQ input connected (SWITCH_2 on TWR_K60N512)
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLUP_ON);      // interrupt is to be rising edge sensitive
            #endif
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
        #endif
    #elif defined _STM32                                                 // connects only one input at a time
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_port = PORTB;                                    // the port used
    interrupt_setup.int_priority = 15;                                   // port interrupt priority (0..15:highest..lowest)
    interrupt_setup.int_port_bit = PORTB_BIT12;                          // the input connected
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE);                 // interrupt on this edge
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    #elif defined _HW_SAM7X
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_priority = PRIORITY_PIOA;                        // port interrupt priority
    interrupt_setup.int_port = PORT_A;                                   // the port used
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | IRQ_GLITCH_ENABLE); // {1} interrupt on this edge
    interrupt_setup.int_port_bits = PA29;                                // the input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_port = PORTA_IRQ0;                               // the port used (fixed interrupt)
    interrupt_setup.int_handler = test_irq_5;                            // handling function
    interrupt_setup.int_port_sense = IRQ_FALLING_EDGE;                   // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_7;                            // handling function
    interrupt_setup.int_priority = PRIORITY_PIOB;                        // port interrupt priority
    interrupt_setup.int_port = PORTB;                                    // the port used
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | IRQ_GLITCH_ENABLE); // {1} interrupt on rising edges
    interrupt_setup.int_port_bits = (PB25);                              // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_11;                           // handling function
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | IRQ_FALLING_EDGE | IRQ_GLITCH_ENABLE); // {1} interrupt on both edges
    interrupt_setup.int_port_bits = (PB27 | PB24);                       // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
    #elif defined _HW_AVR32
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_priority = PRIORITY_GPIO;                        // port interrupt priority
    interrupt_setup.int_port = PORT_0;                                   // the port used
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | IRQ_ENABLE_GLITCH_FILER); // interrupt on this edge with active glitch filter
    interrupt_setup.int_port_bits = PA22;                                // the input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_port = PORT_1;                                   // the port used
    interrupt_setup.int_handler = test_irq_5;                            // handling function
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | IRQ_ENABLE_GLITCH_FILER); // interrupt on this edge with active glitch filter
    interrupt_setup.int_port_bits = (PB22 | PB23);                       // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_7;                            // handling function
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE);                  // interrupt on rising edges
    interrupt_setup.int_port_bits = (PB24);                              // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_1;                            // handling function
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE);                  // interrupt on rising edges
    interrupt_setup.int_port_bits = (PB31);                              // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_port = EXT_INT_CONTROLLER;                       // external interrupt controller
    interrupt_setup.int_handler = test_irq_11;                           // handling function
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | IRQ_ENABLE_GLITCH_FILER); // interrupt on rising edge with active filter
    interrupt_setup.int_port_bits = (EXT_INT_0 | EXT_INT_3);             // the inputs connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
    #elif defined _STR91XF
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_priority = (0);                                  // port interrupt priority
    interrupt_setup.int_port_bit = EXINT_2;                              // the input connected
    interrupt_setup.int_port_sense = WUI_RISING_EDGE;                    // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_5;                            // handling function
    interrupt_setup.int_port_bit = EXINT_15;                             // the input connected
    interrupt_setup.int_port_sense = WUI_FALLING_EDGE;                   // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_7;                            // handling function
    interrupt_setup.int_port_bit = EXINT_21;                             // the input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_handler = test_irq_1;                            // handling function
    interrupt_setup.int_port_bit = EXINT_22;                             // the input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt

    interrupt_setup.int_type = PORT_CHANNEL_INTERRUPT;
    interrupt_setup.int_handler = test_irq_11;                           // handling function
    interrupt_setup.int_port_bit = EXINT_31;                             // the input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
    #elif defined _LM3SXXXX
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_port = PORTC;                                    // the port used
    interrupt_setup.int_priority = 3;                                    // port interrupt priority
    interrupt_setup.int_port_bit = PORTC_BIT7;                           // the input connected
    interrupt_setup.int_port_characteristic = PULLUP_ON;                 // enable pull-up resistor at input
    interrupt_setup.int_port_sense = IRQ_RISING_EDGE;                    // interrupt on this edge
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_port = PORTA;                                    // the port used
    interrupt_setup.int_port_bit = PORTA_BIT7;                           // the input connected
    interrupt_setup.int_handler = test_irq_1;                            // handling function
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_port_bit = PORTA_BIT6;                           // the input connected
    interrupt_setup.int_handler = test_irq_5;                            // handling function
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_port_bit = PORTA_BIT5;                           // the input connected
    interrupt_setup.int_port_sense = IRQ_FALLING_EDGE;                   // interrupt on this edge
    interrupt_setup.int_handler = test_irq_7;                            // handling function
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_port_bit = PORTA_BIT4;                           // the input connected
    interrupt_setup.int_handler = test_irq_11;                           // handling function
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    #elif defined _LPC23XX || defined _LPC17XX
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = test_irq_4;                            // handling function
    interrupt_setup.int_port = PORT_0;                                   // the port used
    interrupt_setup.int_priority = 3;                                    // port interrupt priority
    interrupt_setup.int_port_bits = PORT0_BIT0;                          // the input connected
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_DOWN_OFF); // interrupt on this edge
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_handler = test_irq_1;                            // handling function
    interrupt_setup.int_port_bits = PORT0_BIT4;                          // the input connected
    interrupt_setup.int_port_sense = (IRQ_RISING_EDGE | PULLUP_DOWN_OFF); // interrupt on this edge
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    interrupt_setup.int_handler = test_irq_11;                           // handling function
    interrupt_setup.int_port = EXTERNALINT;                              // an external interrupt rather than a port interrupt
    interrupt_setup.int_port_bits = EINT3;                               // the input connected is EINT1 (0 to 3 is possible whereby EINT3 is shared with GPIO interrupts)
    interrupt_setup.int_priority = 12;                                   // external interrupt priority
    fnConfigureInterrupt(&interrupt_setup);                              // configure test interrupt
    #else                                                                // M5223X
    interrupt_setup.int_type     = PORT_INTERRUPT;                       // identifier when configuring
        #if !defined TEST_DS1307 && !(defined _M5225X && !defined INTERRUPT_TASK_PHY) // uses this input for RTC or PHY
    interrupt_setup.int_handler  = test_irq_1;                           // handling function
    interrupt_setup.int_priority = (INTERRUPT_LEVEL_1);                  // interrupt priority level (this cannot be modified for IRQ1..IRQ7 so the value is not really relevant)
    interrupt_setup.int_port_bit = 1;                                    // the IRQ input connected
    interrupt_setup.int_port_sense = IRQ_BOTH_EDGES;                     // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
        #endif
    interrupt_setup.int_priority = (INTERRUPT_LEVEL_4);                  // interrupt priority level (this cannot be modified for IRQ1..IRQ7 so the value is not really relevant)
    interrupt_setup.int_handler  = test_irq_4;                           // handling function
    interrupt_setup.int_port_bit = 4;                                    // the IRQ input connected
    interrupt_setup.int_port_sense = IRQ_RISING_EDGE;                    // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
        #if !defined M52259DEMO                                          // this board uses the pin for PHY communication
    interrupt_setup.int_priority = (INTERRUPT_LEVEL_5);                  // interrupt priority level (this cannot be modified for IRQ1..IRQ7 so the value is not really relevant)
    interrupt_setup.int_handler  = test_irq_5;                           // handling function
    interrupt_setup.int_port_bit = 5;                                    // the IRQ input connected
    interrupt_setup.int_port_sense = IRQ_RISING_EDGE;                    // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
        #endif
    interrupt_setup.int_priority = (INTERRUPT_LEVEL_7);                  // interrupt priority level (this cannot be modified for IRQ1..IRQ7 so the value is not really relevant)
    interrupt_setup.int_handler  = test_nmi_7;                           // {2} handling function
    interrupt_setup.int_port_bit = 7;                                    // the NMI input connected
    interrupt_setup.int_port_sense = IRQ_FALLING_EDGE;                   // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt  (test not available on 80 pin devices)
        #if !defined _M521X
    interrupt_setup.int_priority = (IRQ11_INTERRUPT_PRIORITY);           // set level and priority
    interrupt_setup.int_handler  = test_irq_11;                          // handling function
    interrupt_setup.int_port_bit = 11;                                   // the IRQ input connected (on all devices)
    interrupt_setup.int_port_sense = IRQ_RISING_EDGE;                    // interrupt on this edge
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure test interrupt
        #endif
    #endif
    #if defined FRDM_K64F_ && defined WAKEUP_TEST                        // configure all K64 LLWU pins
    interrupt_setup.int_type       = PORT_INTERRUPT;                     // identifier to configure port interrupt
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON/* | ENABLE_PORT_MODE*/);
    interrupt_setup.int_type       = WAKEUP_INTERRUPT;                   // configure as wake-up interrupt
    interrupt_setup.int_handler    = test_irq_5;
    interrupt_setup.int_port       = PORTA;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = (PORTA_BIT4 | PORTA_BIT13);
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure wakeup pins on this port
    interrupt_setup.int_port       = PORTB;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PORTB_BIT0;
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure wakeup pins on this port
    interrupt_setup.int_port       = PORTC;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = (PORTC_BIT1 | PORTC_BIT3 | PORTC_BIT4 | PORTC_BIT5 | PORTC_BIT6 | PORTC_BIT11);
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure wakeup pins on this port
    interrupt_setup.int_port       = PORTD;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = (PORTD_BIT0 | PORTD_BIT2 | PORTD_BIT4 | PORTD_BIT6);
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure wakeup pins on this port
    interrupt_setup.int_port       = PORTE;                              // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = (PORTE_BIT1 | PORTE_BIT2 | PORTE_BIT4);
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure wakeup pins on this port
    #endif
    #if defined WAKEUP_TEST && defined SUPPORT_LPTMR && defined FRDM_K22F
    interrupt_setup.int_type = WAKEUP_INTERRUPT;
    interrupt_setup.int_port = PORT_MODULE;                              // define a wakeup interrupt on a module
    interrupt_setup.int_port_bits = (MODULE_LPTMR0);                     // wakeup on low power timer match
        #if defined TICK_USES_LPTMR
    interrupt_setup.int_handler = 0;                                     // no handler since it will be serviced by the tick interrupt
        #else
    interrupt_setup.int_handler = test_irq_5;
        #endif
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
    #elif (defined FRDM_K64F || defined FRDM_K22F || defined FRDM_KL25Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27 || defined FRDM_KL26Z || defined FRDM_KL43Z || defined TWR_KL43Z48M || defined FRDM_KL05Z) && defined WAKEUP_TEST && defined SUPPORT_RTC
    interrupt_setup.int_type = WAKEUP_INTERRUPT;
    interrupt_setup.int_port = PORT_MODULE;                              // define a wakeup interrupt on a module
    interrupt_setup.int_port_bits = (MODULE_RTC_ALARM);                  // wakeup on RTC alarm interrupts
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
    #endif
}
#endif



