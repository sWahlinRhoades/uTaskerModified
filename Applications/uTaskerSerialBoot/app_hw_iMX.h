/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      app_hw_iMX.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

    Application specific hardware configuration
*/

#if defined _iMX && !defined __APP_HW_iMX__
#define __APP_HW_iMX__

#define ENABLE_DATA_CACHE
#define ENABLE_INSTRUCTION_CACHE

#define _DELETE_BOOT_MAILBOX()     *(BOOT_MAIL_BOX) = 0

#if defined SPI_FLASH_ATXP                                               // this flash device has long erase times and so we ensure the watchdog timeout is more lenient
    #define ACTIVATE_WATCHDOG()        UNLOCK_WDOG3(); WDOG3_TOVAL = (4 * 32000); WDOG3_WIN = 0; WDOG3_CS = (WDOG_CS_INT | WDOG_CS_CLK_LPO | WDOG_CS_FLG | WDOG_CS_CMD32EN | WDOG_CS_EN | WDOG_CS_UPDATE); // enable watchdog with 4s timeout (32kHz reference)
#else
    #define ACTIVATE_WATCHDOG()        UNLOCK_WDOG3(); WDOG3_TOVAL = (2 * 32000); WDOG3_WIN = 0; WDOG3_CS = (WDOG_CS_INT | WDOG_CS_CLK_LPO | WDOG_CS_FLG | WDOG_CS_CMD32EN | WDOG_CS_EN | WDOG_CS_UPDATE); // enable watchdog with 2s timeout (32kHz reference)
#endif


// Define clock settings
//
#if defined MIMXRT1010
    #define CRYSTAL_FREQUENCY    24000000                                // 24 MHz crystal (only this crystal frequency should be used)
    #define _EXTERNAL_CLOCK      CRYSTAL_FREQUENCY                       // the clock source if the crystal oscillator

  //#define SYSTICK_EXTERNAL_CLOCK_SOURCE                                // clock SYSTICK from the external clock/240 [100kHz] source rather than the core frequency

    // Select the speed of PLLs (use only to force PLLs to remain in bypassed mode)
    //
  //#define PLL2_CLOCK_OSC_CLK                                           // keep PLL2 disabled in bypass mode and use OSC_CLK instead (else run at fixed 528MHz when required)
  //#define PLL3_CLOCK_OSC_CLK                                           // keep PLL3 disabled in bypass mode and use OSC_CLK instead (else run at fixed 480MHz when required)
  //#define PLL6_CLOCK_OSC_CLK                                           // keep PLL6 disabled in bypass mode and use OSC_CLK instead (else run at fixed 500MHz when required)

    // Select the source of the PERIPH CLK (which feeds AHB_CLK_ROOT - the core clock frequency)
    // - choose just one of the options
    //
  //#define RUN_FROM_DEFAULT_CLOCK                                       // run at default 24.0Mz frequency (available as reference but generally not used due to PERIPH_CLK_SOURCE_OSC being equivalent but with additional pre-scaler possibility)
  //#define PERIPH_CLK_SOURCE_OSC                                        // PERIPHCLK sourced from OSC_CLK (24MHz) - this option is equivalent to RUN_FROM_DEFAULT_CLOCK since i.MX RT 101X has no optional input divider
    #define PERIPH_CLK_SOURCE_PLL6_500M                                  // PERIPHCLK sourced from 500MHz (ENET PLL)
  //#define PERIPH_CLK_SOURCE_PLL3_SW_CLK                                // PERIPHCLK sourced from PLL3_SW_CLK (480MHz)
  //#define PERIPH_CLK_SOURCE_PLL2_528M                                  // PERIPHCLK sourced from PLL2 (528MHz) - beware that AHB_CLK_ROOT_DIVIDE should be >1
  //#define PERIPH_CLK_SOURCE_PLL2_PFD3                                  // PERIPHCLK sourced from PLL2-PFD3
        #define PLL2_PFD3_FRACTION              16                       // default PLL3-PFD3 = 528 x 18 / 16 = 594MHz (range 12..35)
  //#define PERIPH_CLK_SOURCE_PLL3_PFD3                                  // PERIPHCLK sourced from PLL3-PFD3
        #define PLL3_PFD3_FRACTION              19                       // default PLL3-PFD3 = 480 x 18 / 19 = 454.7368421MHz (range 12..35)

    // Now define the pre-scaler used to feed PERIPH CLK to AHB_CLK_ROOT (core clock - maximum 500MHz)
    //
    #define AHB_CLK_ROOT_DIVIDE                 1                        // 1..8 source is always PERIPH_CLK
    // and the pre-scaler used to feed AHB_CLK_ROOT to IPG_CLK_ROOT (supplying ADC and XBAR, when used - maximum 150MHz)
    //
    #define IPG_CLK_ROOT_DIVIDE                 4                        // 1..4 source is always ARM core clock (AHB_CLK_ROOT)


    // Choose FLEXSPI_CLK_ROOT source
    //
  //#define FLEXSPI_CLK_FROM_PLL3_PFD0
  //#define FLEXSPI_CLK_FROM_PLL2_PFD2
  //#define FLEXSPI_CLK_FROM_PLL3_SW_CLK
    #define FLEXSPI_CLK_FROM_PLL2
        #define FLEXSPI_CLK_ROOT_DIVIDE         8                        // 1..8 with optional divider (valid for all sources)

    // Choose LPSPI_CLK_ROOT source
    //
    #define LPSPI_CLK_FROM_PLL2
  //#define LPSPI_CLK_FROM_PLL3_PDF0
  //#define LPSPI_CLK_FROM_PLL3_PDF1
  //#define LPSPI_CLK_FROM_PLL2_PDF2
        #define LPSPI_CLK_ROOT_DIVIDE           4                        // 1..8 with optional divider (valid for all sources)


    // Choose PERCLK_CLK_ROOT source (supplying PIT and GPT - max. 75MHz)
    //
    #define PERCLK_CLK_ROOT_SOURCE_IPG_CLK                               // PERCLK sourced from IPG_CLK_ROOT (as used by ADC and XBAR)
  //#define PERCLK_CLK_ROOT_SOURCE_OSC_CLK                               // PERCLK sourced from 24.0MHz OSC_CLK (used by ADC and XBAR)
        #define PERCLK_CLK_ROOT_DIVIDE          7                        // 1..64 with optional divider (valid for either IPG or OSC source)

    // Choose UART_CLK_ROOT source (supplying all LPUARTs)
    //
    #define UART_CLK_ROOT_FROM_PLL3_SW_CLK_6                             // UART_CLK_ROOT sourced from pll3_sw_clk/6 (80MHz)
  //#define UART_CLK_ROOT_FROM_OSC_CLK                                   // UART_CLK_ROOT sourced from OSC source (24MHz)
        #define UART_CLK_ROOT_DIVIDER           5                        // UART clock root source divided by 1..64 (valid for either pll3_sw_clk/6 or OSC source)


    // Choose LPI2C_CLK_ROOT source (supplying all LPI2C controllers)
    //
    #define LPI2C_CLK_ROOT_FROM_PLL3_SW_CLK_8                            // LPI2C_CLK_ROOT sourced from pll3_sw_clk/8 (60MHz)
  //#define LPI2C_CLK_ROOT_FROM_OSC_CLK                                  // LPI2C_CLK_ROOT sourced from OSC source (24MHz)
        #define LPI2C_CLK_ROOT_DIVIDER          8                        // LPI2C clock root source divided by 1..64 (valid for either pll3_sw_clk/8 or OSC source)

    #define USB_CLOCK_GENERATED_INTERNALLY                               // use USB clock from internal source rather than external pin
#elif defined MIMXRT1015 || defined MIMXRT1020 || defined MIMXRT1024
    #define CRYSTAL_FREQUENCY    24000000                                // 24 MHz crystal (only this crystal frequency should be used)
    #define _EXTERNAL_CLOCK      CRYSTAL_FREQUENCY                       // the clock source if the crystal oscillator

  //#define SYSTICK_EXTERNAL_CLOCK_SOURCE                                // clock SYSTICK from the external clock/240 [100kHz] source rather than the core frequency

    // Select the speed of PLLs (use only to force PLLs to remain in bypassed mode)
    //
  //#define PLL2_CLOCK_OSC_CLK                                           // keep PLL2 disabled in bypass mode and use OSC_CLK instead (else run at fixed 528MHz when required)
  //#define PLL3_CLOCK_OSC_CLK                                           // keep PLL3 disabled in bypass mode and use OSC_CLK instead (else run at fixed 480MHz when required)
  //#define PLL6_CLOCK_OSC_CLK                                           // keep PLL6 disabled in bypass mode and use OSC_CLK instead (else run at fixed 500MHz when required)

    // Select the source of the PERIPH CLK (which feeds AHB_CLK_ROOT - the core clock frequency)
    // - choose just one of the options
    //
  //#define RUN_FROM_DEFAULT_CLOCK                                       // run at default 24.0Mz frequency (available as reference but generally not used due to PERIPH_CLK_SOURCE_OSC being equivalent but with additional pre-scaler possibility)

  //#define PERIPH_CLK_SOURCE_OSC                                        // PERIPHCLK sourced from OSC_CLK (24MHz) - this option is equivalent to RUN_FROM_DEFAULT_CLOCK but with additional pre-scaler option
        #define PERIPH_CLK_SOURCE_OSC_DIVIDE    1                        // 1..8 with optional divider

    #define PERIPH_CLK_SOURCE_PLL6_500M                                  // PERIPHCLK sourced from 500MHz (ENET PLL)
        #define PERIPH_CLK_SOURCE_PLL6_DIVIDE   1                        // 1..8 with optional divider

  //#define PERIPH_CLK_SOURCE_PLL3_SW_CLK                                // PERIPHCLK sourced from PLL3_SW_CLK (480MHz)
        #define PERIPH_CLK_SOURCE_PLL_SW_DIVIDE 1                        // 1..8 with optional divider

  //#define PERIPH_CLK_SOURCE_PLL2_528M                                  // PERIPHCLK sourced from PLL2 (528MHz) - beware that AHB_CLK_ROOT_DIVIDE should be >1

  //#define PERIPH_CLK_SOURCE_PLL2_PFD3                                  // PERIPHCLK sourced from PLL2-PFD3
        #define PLL2_PFD3_FRACTION              16                       // default PLL3-PFD3 = 528 x 18 / 16 = 594MHz (range 12..35)

  //#define PERIPH_CLK_SOURCE_PLL3_PFD3                                  // PERIPHCLK sourced from PLL3-PFD3
        #define PLL3_PFD3_FRACTION              19                       // default PLL3-PFD3 = 480 x 18 / 19 = 454.7368421MHz (range 12..35)


    // Now define the pre-scaler used to feed PERIPH CLK to AHB_CLK_ROOT (core clock - maximum 500MHz)
    //
    #define AHB_CLK_ROOT_DIVIDE                 1                        // 1..8 source is always PERIPH_CLK
    // and the pre-scaler used to feed AHB_CLK_ROOT to IPG_CLK_ROOT (supplying ADC and XBAR, when used - maximum 150MHz)
    //
    #define IPG_CLK_ROOT_DIVIDE                 4                        // 1..4 source is always ARM core clock (AHB_CLK_ROOT)


    // Choose SEMC_CLK_ROOT source
    //
  //#define SEMC_CLK_FROM_PLL2_PFD2
  //#define SEMC_CLK_FROM_PLL3_PFD1
    #define SEMC_CLK_FROM_PERIPH_CLK
        #define SEMC_CLK_ROOT_DIVIDE            4                        // 1..8 with optional divider (valid for all sources)

    // Choose FLEXSPI_CLK_ROOT source
    //
  //#define FLEXSPI_CLK_FROM_PLL3_PFD0
  //#define FLEXSPI_CLK_FROM_PLL2_PFD2
  //#define FLEXSPI_CLK_FROM_PLL3_SW_CLK
    #define FLEXSPI_CLK_FROM_SEMC_CLK
        #define FLEXSPI_CLK_ROOT_DIVIDE         8//1                        // 1..8 with optional divider (valid for all sources)

    // Choose LPSPI_CLK_ROOT source
    //
    #define LPSPI_CLK_FROM_PLL2
  //#define LPSPI_CLK_FROM_PLL3_PDF0
  //#define LPSPI_CLK_FROM_PLL3_PDF1
  //#define LPSPI_CLK_FROM_PLL2_PDF2
        #define LPSPI_CLK_ROOT_DIVIDE           4                        // 1..8 with optional divider (valid for all sources)


    // Choose PERCLK_CLK_ROOT source (supplying PIT and GPT - max. 75MHz)
    //
    #define PERCLK_CLK_ROOT_SOURCE_IPG_CLK                               // PERCLK sourced from IPG_CLK_ROOT (as used by ADC and XBAR)
  //#define PERCLK_CLK_ROOT_SOURCE_OSC_CLK                               // PERCLK sourced from 24.0MHz OSC_CLK (used by ADC and XBAR)
        #define PERCLK_CLK_ROOT_DIVIDE          7                        // 1..64 with optional divider (valid for either IPG or OSC source)

    // Choose UART_CLK_ROOT source (supplying all LPUARTs)
    //
    #define UART_CLK_ROOT_FROM_PLL3_SW_CLK_6                             // UART_CLK_ROOT sourced from pll3_sw_clk/6 (80MHz)
  //#define UART_CLK_ROOT_FROM_OSC_CLK                                   // UART_CLK_ROOT sourced from OSC source (24MHz)
        #define UART_CLK_ROOT_DIVIDER           5                        // UART clock root source divided by 1..64 (valid for either pll3_sw_clk/6 or OSC source)


    // Choose LPI2C_CLK_ROOT source (supplying all LPI2C controllers)
    //
    #define LPI2C_CLK_ROOT_FROM_PLL3_SW_CLK_8                            // LPI2C_CLK_ROOT sourced from pll3_sw_clk/8 (60MHz)
  //#define LPI2C_CLK_ROOT_FROM_OSC_CLK                                  // LPI2C_CLK_ROOT sourced from OSC source (24MHz)
        #define LPI2C_CLK_ROOT_DIVIDER          8                        // LPI2C clock root source divided by 1..64 (valid for either pll3_sw_clk/8 or OSC source)

    // Choose CAN_CLK_ROOT source (supplying all FLEXCAN controllers)
    //
    #define CAN_CLK_ROOT_FROM_PLL3_SW_CLK_6                              // CAN_CLK_ROOT sourced from pll3_sw_clk/6 (80MHz)
  //#define CAN_CLK_ROOT_FROM_PLL3_SW_CLK_8                              // CAN_CLK_ROOT sourced from pll3_sw_clk/8 (60MHz)
  //#define CAN_CLK_ROOT_FROM_OSC_CLK                                    // CAN_CLK_ROOT sourced from OSC source (24MHz)
        #define CAN_CLK_ROOT_DIVIDER            2                        // CAN clock root source divided by 1..64 (valid for either pll3_sw_clk/6, pll3_sw_clk/8 or OSC source)

    // Choose USDHC1_CLK_ROOT source
    //
    #define USDHC1_CLK_SOURCE_PLL2_PFD2                                  // USDHC1_CLK_ROOT sourced from pll2.pfd2
  //#define USDHC1_CLK_SOURCE_PLL2_PFD0                                  // USDHC1_CLK_ROOT sourced from pll2.pfd0
        #define USDHC1_CLK_ROOT_DIVIDE          2                        // USDHC1 clock root source divided by 1..8 (valid for either pll2.pfd2 or pll2.pfd0 source)

    // Choose USDHC2_CLK_ROOT source
    //
    #define USDHC2_CLK_SOURCE_PLL2_PFD2                                  // USDHC2_CLK_ROOT sourced from pll2.pfd2
  //#define USDHC2_CLK_SOURCE_PLL2_PFD0                                  // USDHC2_CLK_ROOT sourced from pll2.pfd0
        #define USDHC2_CLK_ROOT_DIVIDE          2                        // USDHC2 clock root source divided by 1..8 (valid for either pll2.pfd2 or pll2.pfd0 source)

    #define USB_CLOCK_GENERATED_INTERNALLY                               // use USB clock from internal source rather than external pin
#elif defined MIMXRT1050 || defined MIMXRT1060 || defined MIMXRT1064 || defined TEENSY_4_0 || defined TEENSY_4_1 || defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART || defined ARCH_MIX
    #define CRYSTAL_FREQUENCY    24000000                                // 24 MHz crystal (only this crystal frequency should be used)
    #define _EXTERNAL_CLOCK      CRYSTAL_FREQUENCY                       // the clock source if the crystal oscillator

  //#define SYSTICK_EXTERNAL_CLOCK_SOURCE                                // clock SYSTICK from the external clock/240 [100kHz] source rather than the core frequency

    // Select the speed of PLLs (use only to force PLLs to remain in bypassed mode)
    //
  //#define PLL1_CLOCK_OSC_CLK                                           // keep PLL1 disabled in bypass mode and use OSC_CLK instead (else run at fixed 528MHz when required)
  //#define PLL2_CLOCK_OSC_CLK                                           // keep PLL2 disabled in bypass mode and use OSC_CLK instead (else run at fixed 528MHz when required)
  //#define PLL3_CLOCK_OSC_CLK                                           // keep PLL3 disabled in bypass mode and use OSC_CLK instead (else run at fixed 480MHz when required)
  //#define PLL6_CLOCK_OSC_CLK                                           // keep PLL6 disabled in bypass mode and use OSC_CLK instead (else run at fixed 500MHz when required)

    // Select the source of the PERIPH CLK (which feeds AHB_CLK_ROOT - the core clock frequency)
    // - choose just one of the options
    //
  //#define RUN_FROM_DEFAULT_CLOCK                                       // run at default 24.0Mz frequency (available as reference but generally not used due to PERIPH_CLK_SOURCE_OSC being equivalent but with additional pre-scaler possibility)

  //#define PERIPH_CLK_SOURCE_OSC                                        // PERIPHCLK sourced from OSC_CLK (24MHz) - this option is equivalent to RUN_FROM_DEFAULT_CLOCK but with additional pre-scaler option
        #define PERIPH_CLK_SOURCE_OSC_DIVIDE    1                        // 1..8 with optional divider

    #define PERIPH_CLK_SOURCE_ARM_PLL                                    // PERIPHCLK sourced from ARM PLL (650MHz .. 1.3GHz)
        #if (defined MIMXRT1060 && defined DEV8) || (defined iMX_RT1062_EMB_ART) // industrial grade parts limited to 528MHz
            #define ARM_PLL_MULTIPLIER          88                       // 54..108 ((24MHz * 100 / 2) for 1.056GHz to give 528MHz periph clk
        #else
            #define ARM_PLL_MULTIPLIER          100                      // 54..108 ((24MHz * 100 / 2) for 1.2GHz
        #endif
        #define PERIPH_CLK_SOURCE_ARM_PLL_DIVIDE  2                      // 1..8 with optional divider (2 for 600MHz)

  //#define PERIPH_CLK_SOURCE_PLL3_SW_CLK                                // PERIPHCLK sourced from PLL3_SW_CLK (480MHz)
        #define PERIPH_CLK_SOURCE_PLL_SW_DIVIDE 1                        // 1..8 with optional divider

  //#define PERIPH_CLK_SOURCE_PLL2_528M                                  // PERIPHCLK sourced from PLL2 (528MHz)

  //#define PERIPH_CLK_SOURCE_PLL2_PFD3                                  // PERIPHCLK sourced from PLL2-PFD3
        #define PLL2_PFD3_FRACTION              16                       // default PLL3-PFD3 = 528 x 18 / 16 = 594MHz (range 12..35)

  //#define PERIPH_CLK_SOURCE_PLL3_PFD3                                  // PERIPHCLK sourced from PLL3-PFD3
        #define PLL3_PFD3_FRACTION              19                       // default PLL3-PFD3 = 480 x 18 / 19 = 454.7368421MHz (range 12..35)


    // Now define the pre-scaler used to feed PERIPH CLK to AHB_CLK_ROOT (core clock - maximum 500MHz)
    //
    #define AHB_CLK_ROOT_DIVIDE                 1                        // 1..8 source is always PERIPH_CLK
    // and the pre-scaler used to feed AHB_CLK_ROOT to IPG_CLK_ROOT (supplying ADC and XBAR, when used - maximum 150MHz)
    //
    #define IPG_CLK_ROOT_DIVIDE                 4                        // 1..4 source is always ARM core clock (AHB_CLK_ROOT)


    // Choose SEMC_CLK_ROOT source
    //
  //#define SEMC_CLK_FROM_PLL2_PFD2
  //#define SEMC_CLK_FROM_PLL3_PFD1
    #define SEMC_CLK_FROM_PERIPH_CLK
        #define SEMC_CLK_ROOT_DIVIDE            4                        // 1..8 with optional divider (valid for all sources)

    // Choose FLEXSPI_CLK_ROOT source
    //
  //#define FLEXSPI_CLK_FROM_PLL3_PFD0
  //#define FLEXSPI_CLK_FROM_PLL2_PFD2
  //#define FLEXSPI_CLK_FROM_PLL3_SW_CLK
    #define FLEXSPI_CLK_FROM_SEMC_CLK
        #define FLEXSPI_CLK_ROOT_DIVIDE         4                        // 1..8 with optional divider (valid for all sources)

    #if defined MIMXRT1064
    // Choose FLEXSPI2_CLK_ROOT source
    //
  //#define FLEXSPI2_CLK_FROM_PLL3_PFD1
    #define FLEXSPI2_CLK_FROM_PLL2
  //#define FLEXSPI2_CLK_FROM_PLL3_SW_CLK
  //#define FLEXSPI2_CLK_FROM_SEMC_CLK
        #define FLEXSPI2_CLK_ROOT_DIVIDE        5                        // 1..8 with optional divider (valid for all sources)
    #endif

    // Choose LPSPI_CLK_ROOT source
    //
    #define LPSPI_CLK_FROM_PLL2
  //#define LPSPI_CLK_FROM_PLL3_PDF0
  //#define LPSPI_CLK_FROM_PLL3_PDF1
  //#define LPSPI_CLK_FROM_PLL2_PDF2
        #define LPSPI_CLK_ROOT_DIVIDE           4                        // 1..8 with optional divider (valid for all sources)


    // Choose PERCLK_CLK_ROOT source (supplying PIT and GPT - max. 75MHz)
    //
    #define PERCLK_CLK_ROOT_SOURCE_IPG_CLK                               // PERCLK sourced from IPG_CLK_ROOT (as used by ADC and XBAR)
  //#define PERCLK_CLK_ROOT_SOURCE_OSC_CLK                               // PERCLK sourced from 24.0MHz OSC_CLK (used by ADC and XBAR)
        #define PERCLK_CLK_ROOT_DIVIDE          7                        // 1..64 with optional divider (valid for either IPG or OSC source)

    // Choose UART_CLK_ROOT source (supplying all LPUARTs)
    //
    #define UART_CLK_ROOT_FROM_PLL3_SW_CLK_6                             // UART_CLK_ROOT sourced from pll3_sw_clk/6 (80MHz)
  //#define UART_CLK_ROOT_FROM_OSC_CLK                                   // UART_CLK_ROOT sourced from OSC source (24MHz)
        #define UART_CLK_ROOT_DIVIDER           5                        // UART clock root source divided by 1..64 (valid for either pll3_sw_clk/6 or OSC source)


    // Choose LPI2C_CLK_ROOT source (supplying all LPI2C controllers)
    //
    #define LPI2C_CLK_ROOT_FROM_PLL3_SW_CLK_8                            // LPI2C_CLK_ROOT sourced from pll3_sw_clk/8 (60MHz)
  //#define LPI2C_CLK_ROOT_FROM_OSC_CLK                                  // LPI2C_CLK_ROOT sourced from OSC source (24MHz)
        #define LPI2C_CLK_ROOT_DIVIDER          8                        // LPI2C clock root source divided by 1..64 (valid for either pll3_sw_clk/8 or OSC source)

    // Choose CAN_CLK_ROOT source (supplying all FLEXCAN controllers)
    //
    #define CAN_CLK_ROOT_FROM_PLL3_SW_CLK_6                              // CAN_CLK_ROOT sourced from pll3_sw_clk/6 (80MHz)
  //#define CAN_CLK_ROOT_FROM_PLL3_SW_CLK_8                              // CAN_CLK_ROOT sourced from pll3_sw_clk/8 (60MHz)
  //#define CAN_CLK_ROOT_FROM_OSC_CLK                                    // CAN_CLK_ROOT sourced from OSC source (24MHz)
        #define CAN_CLK_ROOT_DIVIDER            2                        // CAN clock root source divided by 1..64 (valid for either pll3_sw_clk/6, pll3_sw_clk/8 or OSC source)

    // Choose USDHC1_CLK_ROOT source
    //
    #define USDHC1_CLK_SOURCE_PLL2_PFD2                                  // USDHC1_CLK_ROOT sourced from pll2.pfd2
  //#define USDHC1_CLK_SOURCE_PLL2_PFD0                                  // USDHC1_CLK_ROOT sourced from pll2.pfd0
        #define USDHC1_CLK_ROOT_DIVIDE          2                        // USDHC1 clock root source divided by 1..8 (valid for either pll2.pfd2 or pll2.pfd0 source)

    // Choose USDHC2_CLK_ROOT source
    //
    #define USDHC2_CLK_SOURCE_PLL2_PFD2                                  // USDHC2_CLK_ROOT sourced from pll2.pfd2
  //#define USDHC2_CLK_SOURCE_PLL2_PFD0                                  // USDHC2_CLK_ROOT sourced from pll2.pfd0
        #define USDHC2_CLK_ROOT_DIVIDE          2                        // USDHC2 clock root source divided by 1..8 (valid for either pll2.pfd2 or pll2.pfd0 source)

    #define USB_CLOCK_GENERATED_INTERNALLY                               // use USB clock from internal source rather than external pin
#endif


/*
    #if !defined RUN_FROM_DEFAULT_CLOCK                                  // use 528MHz PLL2 (SYS PLL)
        #define CLOCK_DIV            512                                 // the PLL divide factor
        #define CLOCK_MUL            500                                 // the PLL multiplication factor to achieve operating frequency of 500MHz
    #endif
*/

// Default settings can be over-ridden here by adding board-specific settings at the start of the list
//
#if defined MIMXRT1010
    #define PIN_COUNT           PIN_COUNT_80_PIN                         // 80 pin LQFP package
    #define PACKAGE_TYPE        PACKAGE_LQFP                             // LQFP
    #define QSPI_FILE_SYSTEM                                             // user QSPI interface
    #define FLEXSPI_FLASH_SIZE  (16 * 1024 * 1024)                       // 16 Mbytes AT25SF128A-SHB-T on board
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined MIMXRT1015
    #define PIN_COUNT           PIN_COUNT_100_PIN                        // 100 pin LQFP package
    #define PACKAGE_TYPE        PACKAGE_LQFP                             // LQFP
    #define QSPI_FILE_SYSTEM                                             // user QSPI interface
    #define FLEXSPI_FLASH_SIZE  (16 * 1024 * 1024)                       // 16 Mbytes AT25SF128A-SHB-T on board
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined MIMXRT1020
  //#define PIN_COUNT           PIN_COUNT_100_PIN                        // 100 pin LQFP package
    #define PIN_COUNT           PIN_COUNT_144_PIN                        // 141 pin LQFP package
    #define PACKAGE_TYPE        PACKAGE_LQFP                             // LQFP
    #define QSPI_FILE_SYSTEM                                             // user QSPI interface
    #define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                        // 8 Mbytes IS25LP064A-JBLE on board
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined MIMXRT1024
  //#define PIN_COUNT           PIN_COUNT_100_PIN                        // 100 pin LQFP package
    #define PIN_COUNT           PIN_COUNT_144_PIN                        // 141 pin LQFP package
    #define PACKAGE_TYPE        PACKAGE_LQFP                             // LQFP
    #define QSPI_FILE_SYSTEM                                             // user QSPI interface
    #if defined USE_INTERNAL_QSPI_FLASH
        #define FLEXSPI_FLASH_SIZE  (FLEXSPI2_FLASH_SIZE)                // 4 Mbytes internal flash (on-board Winbond W25Q32JV)
    #else
        #define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                    // 8 Mbytes IS25LP064A-JBLE on board
    #endif
    #define FLASH_START_ADDRESS    FLEXSPI_FLASH_BASE
#elif defined MIMXRT1050 || defined ARCH_MIX
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // user QSPI interface
    #define BOOT_HYPER_FLASH                                             // boot from hyper flash rather than QSPI flash
  //#define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                        // 8 Mbytes IS25LP064A-JBLE on board
    #define FLEXSPI_FLASH_SIZE  (64 * 1024 * 1024)                       // 64 Mbytes S26KS512SDPBHI02 on board
    #define SPI_FLASH_S26KL
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // boot from QSPI interface
    #define FLEXSPI_FLASH_SIZE  (4 * 1024 * 1024)                        // 4 Mbytes ATXP032 on board
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
    #if defined DEV9
        #define BOOT_LOADER_SUPPORTS_SDRAM
        #define SDRAM_START_ADDRESS     0x80000000
        #define MAX_SDRAM_CODE_LENGTH   (2 * 1024 * 1024)
    #endif
#elif defined MIMXRT1060
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // boot from QSPI interface
  //#define BOOT_HYPER_FLASH                                             // boot from hyper flash rather than QSPI flash
    #define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                        // 8 Mbytes IS25LP064A-JBLE on board
  //#define FLEXSPI_FLASH_SIZE  (64 * 1024 * 1024)                       // 64 Mbytes S26KS512SDPBHI02 on board
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
    #if defined DEV8
        #define BOOT_LOADER_SUPPORTS_SDRAM
        #define SDRAM_START_ADDRESS     0x80000000
        #define MAX_SDRAM_CODE_LENGTH   (2 * 1024 * 1024)
    #endif
#elif defined TEENSY_4_0
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // boot from QSPI interface
    #define FLEXSPI_FLASH_SIZE  (2 * 1024 * 1024)                        // 2 Mbytes W25Q16
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined TEENSY_4_1
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // boot from QSPI interface
    #define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                        // 8 Mbytes W25Q64
    #define FLASH_START_ADDRESS FLEXSPI_FLASH_BASE
#elif defined MIMXRT1064
    #define PIN_COUNT           PIN_COUNT_196_PIN                        // 196 pin MAPBGA package
    #define PACKAGE_TYPE        PACKAGE_MAPBGA                           // MAPBGA
    #define QSPI_FILE_SYSTEM                                             // boot from QSPI interface
  //#define BOOT_HYPER_FLASH                                             // use hyper flash rather than QSPI flash
    #if defined USE_INTERNAL_QSPI_FLASH
        #define FLEXSPI_FLASH_SIZE  (FLEXSPI2_FLASH_SIZE)                // 4 Mbytes internal flash (on-board Winbond W25Q32JV)
    #else
        #if defined BOOT_HYPER_FLASH
            #define FLEXSPI_FLASH_SIZE  (64 * 1024 * 1024)               // 64 Mbytes S26KS512SDPBHI02 on board
        #else
            #define FLEXSPI_FLASH_SIZE  (8 * 1024 * 1024)                // 8 Mbytes IS25LP064A-JBLE on board
        #endif
    #endif
    #define FLASH_START_ADDRESS    FLEXSPI_FLASH_BASE
#endif


#define SUPPORT_OCOTP                                                    // support reading from and writing to the on-chip OTP controller

// Peripheral clocking strategy
//
//#define CLOCKS_DISABLED_IN_WAIT_AND_STOP                               // clock is on in RUN mode but off in WAIT and STOP
#define CLOCKS_DISABLED_ONLY_IN_STOP                                     // clock is on during all modes except STOP mode


#if defined TWR_K64F120M && !defined TWR_SER2                            // this combination doesn't report link state changes when the MII clock is stopped
    #define STOP_MII_CLOCK                                               // only apply clock when needed (don't use when DEVELOP_PHY_CONTROL is set in debug.c)
#endif
#if !defined DEVICE_WITHOUT_ETHERNET
    #define EMAC_ENHANCED                                                // use enhanced mode for Ethernet controller operation
        #define EMAC_IEEE1588                                            // enable IEEE 1588 time stamping (needs EMAC_ENHANCED)
#endif

#if defined MIMXRT1020 || defined MIMXRT1024 || defined MIMXRT1050 || defined MIMXRT1060 || defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART || defined MIMXRT1064
    #define MII_MANAGEMENT_CLOCK_SPEED    2500000                        // typ. 2.5MHz Speed
  //#define MSCR_HOLDTIME          7
    #define _KSZ8081RNB                                                  // the PHY used
    #define ETHERNET_RMII                                                // RMII mode of operation instead of MII
    #define RESET_PHY              PORT1_BIT4                            // control the PHY's reset so that its configuration can be controlled
    #define PHY_ISOLATE            PORT1_BIT12
    #define PHY_INTERRUPT_OUTPUT   PORT1_BIT22
    #define ASSERT_PHY_RST()       _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (RESET_PHY), (0), (PORT_SRE_SLOW | PORT_DSE_MID)) // start with PHY reset asserted and INTPR/NAND_TREE forced high
    #define CONFIG_PHY_STRAPS()    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (PHY_INTERRUPT_OUTPUT | PHY_ISOLATE), (PHY_INTERRUPT_OUTPUT), (PORT_SRE_SLOW | PORT_DSE_MID))
    #define FORCE_PHY_CONFIG
    #define FNFORCE_PHY_CONFIG()   _SETBITS(1, RESET_PHY); fnDelayLoop(10000)
  //#define SCAN_PHY_ADD                                                 // enable to scan for the PHY and use the address that it is found on
    #define PHY_ADDRESS            0x02                                  // address of external PHY on board (fixed address used when scan not enabled)
    #define PHY_IDENTIFIER         0x00221560                            // MICREL KSZ8081RNB identifier
    #if defined FreeLON
        #define INTERRUPT_TASK_PHY     TASK_NETWORK_INDICATOR            // link status reported to this task (do not use together with LAN_REPORT_ACTIVITY)
        #define PHY_INTERRUPT_PORT     PORTA
        #define PHY_INTERRUPT          PORTA_BIT4                        // IRQ1 is used as PHY interrupt input (set J6 to position 7-8 on TWR-SER board) - this is connected to PTD15
    #else
        #define PHY_POLL_LINK                                            // no interrupt line connected so poll the link state
        #define INTERRUPT_TASK_PHY     TASK_NETWORK_INDICATOR            // enable link state output messages
    #endif
#endif

// If a design doesn't have the phy interrupt line connected, the following can be used to cause the network indicator task to poll the state of the link and synchronise the Ethernet controller accordingly
//
//#define PHY_POLL_LINK
//#define INTERRUPT_TASK_PHY     TASK_NETWORK_INDICATOR                  // link status reported to this task (do not use together with LAN_REPORT_ACTIVITY)

#if defined _PHY_KSZ8863
    #define PHY_ADDRESS            2                                     // use 0x02 since 0x01 may not correctly return the PHYID
    #define PHY_ADDRESS_2          1
    #define PHY_IDENTIFIER         0x00221430                            // phy address 2
    #if !defined MII_MANAGEMENT_CLOCK_SPEED
        #define MII_MANAGEMENT_CLOCK_SPEED   2500000                     // 2.5MHz
    #endif
#endif

#define PHY_MASK                   0xfffffff0                            // don't check the revision number
#if defined PHY_INTERRUPT && !defined SUPPORT_PORT_INTERRUPTS            // port interrupt support must be enabled to use the PHY interrupt
    #define SUPPORT_PORT_INTERRUPTS
#endif

#if !defined ETH_INTERFACE
    #undef INTERRUPT_TASK_PHY
    #undef PHY_POLL_LINK
    #undef LAN_REPORT_ACTIVITY
#endif


//#define SUPPORT_TIMER                                                  // support hardware timer interrupt configuration (FlexTimer or TPM)
    #define SUPPORT_CAPTURE                                              // support capture mode of operation



//#define SUPPORT_PORT_INTERRUPTS                                        // support code for port interrupts - see the following video showing port interrupt operation in a KL27: https://youtu.be/CubinvMuTwU
  //#define PORT_INTERRUPT_USER_DISPATCHER                               // use a single port interrupt callback since the user dispatches according to interrupted input (valid also for low-leakage wakeup interrupts)

  //#define NO_PORT_INTERRUPTS_PORT1                                     // remove port interrupt support from port 1
  //#define NO_PORT_INTERRUPTS_PORT2                                     // remove port interrupt support from port 2
  //#define NO_PORT_INTERRUPTS_PORT3                                     // remove port interrupt support from port 3
  //#define NO_PORT_INTERRUPTS_PORT4                                     // remove port interrupt support from port 4
  //#define NO_PORT_INTERRUPTS_PORT5                                     // remove port interrupt support from port 5
  //#define NO_PORT_INTERRUPTS_PORT6                                     // remove port interrupt support from port 6
  //#define NO_PORT_INTERRUPTS_PORT7                                     // remove port interrupt support from port 7
  //#define NO_PORT_INTERRUPTS_PORT8                                     // remove port interrupt support from port 8

// Include the iMX hardware header here
// - beware that the header delivers rules for subsequent parts of this header file but also accepts some rules from previous parts,
// therefore its position should only be moved after careful consideration of its consequences
//
#include "../../Hardware/iMX/iMX.h"                                      // include the iMX processor header at this location

#if defined RNG_AVAILABLE                                                // hardware RNG support available
    #if defined TRUE_RANDOM_NUMBER_GENERATOR                             // TRNG module is very slow and not yet suitable as general purpose random number generator (it generates 64 bytes at a time and required tyically 620ms to do this at 60MHz bus clock)
        #define RND_HW_SUPPORT                                           // enable the use of the hardware resources in this chip
    #else
        #define RND_HW_SUPPORT                                           // enable the use of the hardware resources in this chip
    #endif
#endif

#if defined SDCARD_SUPPORT
    #define SDCARD_SIM_SIZE   SDCARD_SIZE_4G                             // the size of SD card when simulating
  //#define _NO_SD_CARD_INSERTED                                         // simulate no SD card inserted initially

    #if defined TEENSY_4_0_ || defined TEENSY_4_1_                       // presently the interface doesn't operate if this pin is not used (even when not connected)
        #define SDCARD_DETECTION()      0                                // card detection input not present
    #else
        #define SDCARD_DETECT_INPUT_INTERRUPT                            // use card detect interrupt rather than polling the card
        #define SDCARD_DETECTION()      ((SDHC_PRSSTAT & SDHC_PRSSTAT_CINST) != 0) // card detection input
    #endif
    #if !defined SDCARD_DETECT_INPUT_INTERRUPT
      //#define SDCARD_DETECT_INPUT_POLL                                 // use card detect switch for detection polling (use together with T_CHECK_CARD_REMOVAL)
        #define T_CHECK_CARD_REMOVAL    ((DELAY_LIMIT)(SEC * 10))        // if the card has no detection switch it can be polled to detect removal
    #endif

    #if NUMBER_OF_SDHC > 0                                               // use SDHC controller
        #define SD_CONTROLLER_AVAILABLE                                  // SDHC driver used
        #define SET_SD_CS_HIGH()                                         // dummy since not required by SD controller
        #define SET_SD_CS_LOW()                                          // dummy since not required by SD controller
        #if defined MIMXRT1020 || defined MIMXRT1024
            #define POWER_TO_SD_CARD()     _CONFIG_DRIVE_PORT_OUTPUT_VALUE(3, PORT3_BIT24, PORT3_BIT24, (PORT_SRE_SLOW | PORT_DSE_LOW)) // GPIO_SD_B1_04
            #define POWER_DOWN_SD_CARD()   _CLEARBITS(3, PORT3_BIT24)    // remove power from SD card interface
        #elif defined MIMXRT1060 || defined MIMXRT1064
            #define POWER_TO_SD_CARD()     _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, PORT1_BIT19, PORT1_BIT19, (PORT_SRE_SLOW | PORT_DSE_LOW)) // GPIO_AD_B1_03
            #define POWER_DOWN_SD_CARD()   _CLEARBITS(1, PORT1_BIT19)    // remove power from SD card interface
        #elif defined TEENSY_4_0 || defined TEENSY_4_1
            #define POWER_TO_SD_CARD()                                   // now power control available
            #define POWER_DOWN_SD_CARD()
        #elif defined iMX_RT1062_EMB_ART
            #define POWER_TO_SD_CARD()     _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, PORT1_BIT5, PORT1_BIT5, (PORT_SRE_SLOW | PORT_DSE_LOW)) // GPIO_AD_B0_05
            #define POWER_DOWN_SD_CARD()   _CLEARBITS(1, PORT1_BIT5)     // remove power from SD card interface
        #endif
        #define POWER_UP_SD_CARD()         SDHC_SYSCTL |= SDHC_SYSCTL_INITA; _WAIT_REGISTER_TRUE(SDHC_SYSCTL, SDHC_SYSCTL_INITA) // apply power to the SD card if appropriate (we use this to send 80 clocks - self-clearing bit)
        #define SDHC_SYSCTL_SPEED_SLOW     (SDHC_SYSCTL_SDCLKFS_128 | SDHC_SYSCTL_DVS_4)  // 390kHz when 200MHz clock
        #define SDHC_SYSCTL_SPEED_FAST     (SDHC_SYSCTL_SDCLKFS_2 | SDHC_SYSCTL_DVS_4)    // 25MHz when 200MHz clock
        #define SET_SPI_SD_INTERFACE_FULL_SPEED()  fnSetSD_clock(SDHC_SYSCTL_SPEED_FAST); SDHC_PROCTL |= SDHC_PROCTL_DTW_4BIT

        #define GET_SDCARD_WP_STATE()      (0)                           // no write protect input
    #else                                                                // use SPI mode
    #endif
#endif
#if defined SDCARD_SUPPORT || defined USB_MSD_HOST_LOADER
    #if !defined OWN_SD_CARD_DISPLAY
        #if defined SERIAL_INTERFACE || (defined USB_INTERFACE && defined USE_USB_CDC) // when UART or USB-CDC interface is available
            #if defined USB_MSD_HOST_LOADER
                #define _DISPLAY_SD_CARD_NOT_PRESENT() fnDebugMsg("Mem-Stick not present\r\n")
                #define _DISPLAY_SD_CARD_NOT_FORMATTED() fnDebugMsg("Mem-Stick not formatted\r\n")
                #define _DISPLAY_SD_CARD_PRESENT()    fnDebugMsg("Mem-Stick present\r\n")
                #define _DISPLAY_NO_FILE()            fnDebugMsg("No file on Mem-Stick\r\n")
            #else
                #define _DISPLAY_SD_CARD_NOT_PRESENT() fnDebugMsg("SD-Card not present\r\n")
                #define _DISPLAY_SD_CARD_NOT_FORMATTED() fnDebugMsg("SD-Card not formatted\r\n")
                #define _DISPLAY_SD_CARD_PRESENT()    fnDebugMsg("SD-Card present\r\n")
                #define _DISPLAY_NO_FILE()            fnDebugMsg("No file on card\r\n")
            #endif
            #define _DISPLAY_VALID_CONTENT()          fnDebugMsg("File valid\r\n")
            #define _DISPLAY_INVALID_CONTENT()        fnDebugMsg("File invalid\r\n")
            #define _DISPLAY_SW_OK()                  fnDebugMsg("Software OK\r\n")
            #define _DISPLAY_SW_UPDATED()             fnDebugMsg("Software Updated\r\n")
            #define _DISPLAY_ERROR()                  fnDebugMsg("Update failed\r\n")
            #define _DISPLAY_OVERSIZE_CONTENT()       fnDebugMsg("File oversized!\r\n")
        #else
            #define _DISPLAY_SD_CARD_NOT_PRESENT()   
            #define _DISPLAY_SD_CARD_NOT_FORMATTED() 
            #define _DISPLAY_NO_FILE()               
            #define _DISPLAY_SD_CARD_PRESENT()       
            #define _DISPLAY_VALID_CONTENT()         
            #define _DISPLAY_INVALID_CONTENT()       
            #define _DISPLAY_SW_OK()
            #define _DISPLAY_SW_UPDATED()
            #define _DISPLAY_ERROR()
            #define _DISPLAY_OVERSIZE_CONTENT()
        #endif
    #endif
#endif


#define TICK_INTERRUPT()                                                 // user callback from system TICK (dummy if left empty)
#if defined MONITOR_PERFORMANCE
    #if PITS_AVAILABLE > 2                                               // PIT3 is used to monitor task durations
        #define INITIALISE_MONITOR_TIMER()        POWER_UP_ATOMIC(6, PIT0); PIT_MCR = 0; LOAD_PIT(3, 0xffffffff); PIT_TCTRL3 = PIT_TCTRL_TEN
        #define EXECUTION_DURATION()              (0xffffffff - PIT_CVAL3); LOAD_PIT(3, 0xffffffff); PIT_TCTRL3 = PIT_TCTRL_TEN // read the elapsed count value and reset the counter back to 0xffffffff
    #elif defined LPITS_AVAILABLE && (LPIT_CHANNELS > 2)
        #define INITIALISE_MONITOR_TIMER()        _EXCEPTION("To do")
        #define EXECUTION_DURATION()              _EXCEPTION("To do")
    #else                                                                // PIT0 is used to monitor task durations
        #define INITIALISE_MONITOR_TIMER()        POWER_UP_ATOMIC(6, PIT0); PIT_MCR = 0; LOAD_PIT(0, 0xffffffff); PIT_TCTRL0 = PIT_TCTRL_TEN
        #define EXECUTION_DURATION()              (0xffffffff - PIT_CVAL0); LOAD_PIT(0, 0xffffffff); PIT_TCTRL0 = PIT_TCTRL_TEN // read the elapsed count value and reset the counter back to 0xffffffff
    #endif
    #define PIT_TIMER_USED_BY_PERFORMANCE_MONITOR                        // since a PIT timer is used for the monitoring function don't allow PITS to be powered down
#endif


#if defined SPI_FLASH_IS25
    #if defined MIMXRT1010
        #define SPI_FLASH_IS25LP128A                                     // 16MBytes
    #else
        #define SPI_FLASH_IS25LP064A                                     // 8MBytes
    #endif
  //#define SPI_FLASH_IS25LP256D                                         // 32MBytes 2.3V..3.6V
  //#define SPI_FLASH_IS25WP256D                                         // 32MBytes 1.65V..1.95V
    #if defined SPI_FLASH_IS25LP256D || defined SPI_FLASH_IS25WP256D
        #define SPI_FLASH_PAGES          (128 * 1024)
    #elif defined SPI_FLASH_IS25LP128A
        #define SPI_FLASH_PAGES          (64 * 1024)
    #elif defined SPI_FLASH_IS25LP064A
        #define SPI_FLASH_PAGES          (32 * 1024)
    #endif
    #define SPI_FLASH_PAGE_LENGTH        (256)
    #define SPI_FLASH_SUB_SECTOR_LENGTH  (4 * 1024)                      // sub-sector size of SPI FLASH
    #define SPI_FLASH_HALF_SECTOR_LENGTH (32 * 1024)                     // half-sector size of SPI FLASH
    #define SPI_FLASH_SECTOR_LENGTH      (64 * 1024)                     // block size of SPI FLASH
    #define SPI_FLASH_BLOCK_LENGTH       SPI_FLASH_HALF_SECTOR_LENGTH    // for compatibility - file system granularity
    #define SPI_FLASH_SIZE               (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH)
    #define FLASH_GRANULARITY            SPI_FLASH_SUB_SECTOR_LENGTH
#elif defined SPI_FLASH_AT25SF
    #define SPI_FLASH_AT25FSF128A                                        // 16MBytes
    #define SPI_FLASH_PAGES              (64 * 1024)
    #define SPI_FLASH_PAGE_LENGTH        (256)
    #define SPI_FLASH_SUB_SECTOR_LENGTH  (4 * 1024)                      // sub-sector size of SPI FLASH
    #define SPI_FLASH_HALF_SECTOR_LENGTH (32 * 1024)                     // half-sector size of SPI FLASH
    #define SPI_FLASH_SECTOR_LENGTH      (64 * 1024)                     // block size of SPI FLASH
    #define SPI_FLASH_BLOCK_LENGTH       SPI_FLASH_HALF_SECTOR_LENGTH    // for compatibility - file system granularity
    #define SPI_FLASH_SIZE               (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH)
    #define FLASH_GRANULARITY            SPI_FLASH_SUB_SECTOR_LENGTH
#elif defined SPI_FLASH_S26KL
    #define SPI_FLASH_PAGES              (128 * 1024)
    #define SPI_FLASH_PAGE_LENGTH        (512)
    #define SPI_FLASH_SIZE               (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH)
    #define SPI_FLASH_SECTOR_LENGTH      (256 * 1024)
    #define SPI_FLASH_BLOCK_LENGTH       SPI_FLASH_SECTOR_LENGTH
    #define FLASH_GRANULARITY            SPI_FLASH_BLOCK_LENGTH
#elif defined SPI_FLASH_ATXP
    #define SPI_FLASH_ATXP032                                            // 4MBytes
  //#define SPI_FLASH_ATXP064                                            // 8MBytes
  //#define SPI_FLASH_ATXP128                                            // 16MBytes
    #define SPI_FLASH_PAGES              (16 * 1024)
    #define SPI_FLASH_PAGE_LENGTH        (256)
    #define SPI_FLASH_SUB_SECTOR_LENGTH  (4 * 1024)                      // sub-sector size of SPI FLASH
    #define SPI_FLASH_HALF_SECTOR_LENGTH (32 * 1024)                     // half-sector size of SPI FLASH
    #define SPI_FLASH_SECTOR_LENGTH      (64 * 1024)                     // block size of SPI FLASH
    #define SPI_FLASH_BLOCK_LENGTH       SPI_FLASH_HALF_SECTOR_LENGTH    // for compatibility - file system granularity
    #define SPI_FLASH_SIZE               (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH)
    #define SPI_FLASH_SECTOR_PROTECTION_SIZE (SPI_FLASH_SIZE/16)
    #define FLASH_GRANULARITY            SPI_FLASH_SUB_SECTOR_LENGTH
#elif defined SPI_FLASH_ST                                               // ST SPI FLASH used
#elif defined SPI_FLASH_MX25L
#elif defined SPI_FLASH_S25FL1_K
#elif defined SPI_FLASH_W25Q
  //#define SPI_FLASH_W25Q128
  //#define SPI_FLASH_W25Q64
  //#define SPI_FLASH_W25Q32
  //#define SPI_FLASH_W25Q16
    #if defined SPI_FLASH_W25Q128
        #define SPI_FLASH_PAGES          (64 * 1024)
    #elif defined SPI_FLASH_W25Q64
        #define SPI_FLASH_PAGES          (32 * 1024)
    #elif defined SPI_FLASH_W25Q32
        #define SPI_FLASH_PAGES          (16 * 1024)
    #else
        #define SPI_FLASH_PAGES          (8 * 1024)
    #endif
    #define SPI_FLASH_PAGE_LENGTH        (256)
    #define SPI_FLASH_SUB_SECTOR_LENGTH  (4 * 1024)                      // sub-sector size of SPI FLASH
    #define SPI_FLASH_HALF_SECTOR_LENGTH (32 * 1024)                     // half-sector size of SPI FLASH
    #define SPI_FLASH_SECTOR_LENGTH      (64 * 1024)                     // sector size of SPI FLASH (not available on A-versions)
    #define SPI_FLASH_BLOCK_LENGTH       SPI_FLASH_HALF_SECTOR_LENGTH    // for compatibility - file system granularity
    #define FLASH_GRANULARITY            SPI_FLASH_SUB_SECTOR_LENGTH
    #define SPI_FLASH_SIZE               (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH)
#elif defined SPI_FLASH_SST25
#else                                                                    // AT45DBxxx 
#endif

#define SPI_DATA_FLASH_0_SIZE   (SPI_FLASH_PAGES * SPI_FLASH_PAGE_LENGTH) 
#define SPI_DATA_FLASH_1_SIZE   SPI_DATA_FLASH_0_SIZE 
#define SPI_DATA_FLASH_2_SIZE   SPI_DATA_FLASH_0_SIZE
#define SPI_DATA_FLASH_3_SIZE   SPI_DATA_FLASH_0_SIZE
#if defined SPI_FLASH_MULTIPLE_CHIPS
    #define SPI_FLASH_DEVICE_COUNT      2
    #if SPI_FLASH_DEVICE_COUNT >= 4
        #define SPI_DATA_FLASH_SIZE     (SPI_DATA_FLASH_0_SIZE + SPI_DATA_FLASH_1_SIZE + SPI_DATA_FLASH_2_SIZE + SPI_DATA_FLASH_3_SIZE)
    #elif SPI_FLASH_DEVICE_COUNT >= 3
        #define SPI_DATA_FLASH_SIZE     (SPI_DATA_FLASH_0_SIZE + SPI_DATA_FLASH_1_SIZE + SPI_DATA_FLASH_2_SIZE)
    #else
        #define SPI_DATA_FLASH_SIZE     (SPI_DATA_FLASH_0_SIZE + SPI_DATA_FLASH_1_SIZE)
    #endif
#else
    #define SPI_DATA_FLASH_SIZE         SPI_DATA_FLASH_0_SIZE
#endif
#if defined SPI_EEPROM_FILE_SYSTEM
    #define SPI_DATA_EEPROM_SIZE        (2 * 1024)                       // 25AA160
    #define SPI_EEPROM_PAGE_LENGTH      16
#endif


#if defined USE_PARAMETER_BLOCK
    #define PARAMETER_BLOCK_GRANULARITY     (32 * 1024)
    #define PARAMETER_BLOCK_SIZE            PARAMETER_BLOCK_GRANULARITY  // use the smallest size for the device
    #if defined USE_PAR_SWAP_BLOCK
        #define PAR_BLOCK_SIZE              (2 * PARAMETER_BLOCK_SIZE)
    #else
        #define PAR_BLOCK_SIZE              (1 * PARAMETER_BLOCK_SIZE)
    #endif
    #if defined ACTIVE_FILE_SYSTEM
        #define PARAMETER_BLOCK_START       (0x607f0000)                 // 64k before the end of QSPI flash
    #else
        #define PARAMETER_BLOCK_START       (SIZE_OF_FLASH - PAR_BLOCK_SIZE) // parameters just before the end of flash
    #endif
#elif defined USE_PARAMETER_AREA
    #define PAR_BLOCK_SIZE                  FLASH_GRANULARITY
    #define PARAMETER_BLOCK_START           (SIZE_OF_FLASH - PAR_BLOCK_SIZE) // parameters just before the end of flash
#else
    #define PAR_BLOCK_SIZE  (0)
#endif

// FLASH configuration settings
//
#define BACKDOOR_KEY_0     0x00                                          // note that values with all 0x00 or all 0xff are not valid
#define BACKDOOR_KEY_1     0x00
#define BACKDOOR_KEY_2     0x00
#define BACKDOOR_KEY_3     0x00
#define BACKDOOR_KEY_4     0x00
#define BACKDOOR_KEY_5     0x00
#define BACKDOOR_KEY_6     0x00
#define BACKDOOR_KEY_7     0x01


// Serial interfaces
//
#if defined SERIAL_INTERFACE
    #define NUMBER_EXTERNAL_SERIAL 0                                     // when set to 2 or 4 then one or two SC16IS7XX can be connected for further UARTs via SPI
    #if NUMBER_EXTERNAL_SERIAL > 0
        #define EXT_UART_SC16IS7XX
    #endif
    #define NUMBER_SERIAL   (UARTS_AVAILABLE + LPUARTS_AVAILABLE)        // the number of physical queues needed for serial interface(s)
    #define SIM_COM_EXTENDED                                             // COM ports defined from 1..255
    #define SERIAL_PORT_0    8                                           // if we open UART channel 0 we simulate using comx on the PC
    #define SERIAL_PORT_1    4                                           // if we open UART channel 1 we simulate using comx on the PC
    #define SERIAL_PORT_2    10                                          // if we open UART channel 2 we simulate using comx on the PC
    #define SERIAL_PORT_3    4                                           // if we open UART channel 3 we simulate using comx on the PC
    #define SERIAL_PORT_4    4                                           // if we open UART channel 4 we simulate using comx on the PC
    #define SERIAL_PORT_5    4                                           // if we open UART channel 5 we simulate using comx on the PC
    #define SERIAL_PORT_6    4                                           // if we open UART channel 6 we simulate using comx on the PC
    #define SERIAL_PORT_7    8                                           // if we open UART channel 7 we simulate using comx on the PC

    #if defined TEENSY_4_0 || defined TEENSY_4_1
        #define LOADER_UART      iMX_LPUART_3                            // LPUART3
        #define LPUART3_ON_EMC
    #elif defined iMX_RT1062_EMB_ART && defined DEV9
        #define LOADER_UART      iMX_LPUART_2                            // LPUART2
    #elif defined MIMXRT1060 && defined DEV8
        #define LOADER_UART      iMX_LPUART_8                            // LPUART8
        #define LPUART8_ON_AD
    #else
        #define LOADER_UART      iMX_LPUART_1                            // LPUART1
    #endif

    #define MODBUS_UART_0        iMX_LPUART_2                            // note - to use via USB use NUMBER_SERIAL instead of UART number
    #define MODBUS_UART_1        iMX_LPUART_3
    #define MODBUS_UART_2        iMX_LPUART_4
    #define PPP_UART             iMX_LPUART_5
    #define FREEMASTER_UART_CH   iMX_LPUART_6

    #define SERIAL_SUPPORT_DMA                                           // enable UART DMA support (TX)
        #define SERIAL_SUPPORT_DMA_RX                                    // enable also DMA on receiver (used less that transmit DMA)
            #define SERIAL_SUPPORT_DMA_RX_FREERUN                        // support free-running reception mode

    #define TX_BUFFER_SIZE   (QUEUE_TRANSFER)(2 * 1024)                  // the size of RS232 input and output buffers
    #if defined _WINDOWS
        #define RX_BUFFER_SIZE   (32000)                                 // used for simulation to ensure that the rx buffer doesn't overflow
    #else
        #define RX_BUFFER_SIZE   (512)
    #endif
  //#define TRUE_UART_TX_2_STOPS                                         // allow true 2 stop bit transmission timing on devices without this UART controller support

    #if defined SUPPORT_LOW_POWER
        #define LPUART_CHARACTERISTICS     (0 /* | LPUART_CTRL_DOZEEN*/)
    #else
        #define LPUART_CHARACTERISTICS     (0)
    #endif

    #if !defined REMOVE_SREC_LOADING && !defined USE_MODBUS && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER && !defined DEVICE_WITHOUT_DMA
      //#define FREE_RUNNING_RX_DMA_RECEPTION                            // user free-running uart rx DMA for SREC/iHEX loader
    #endif
#else
    #define TX_BUFFER_SIZE   (256)
    #define RX_BUFFER_SIZE   (256)
#endif


// ADC
//
//#define SUPPORT_ADC
#define ADC_REFERENCE_VOLTAGE                      3300                  // ADC uses 3.3V reference
#define ADC_SIM_STEP_SIZE                          200                   // 200mV steps when simulating

#if defined SUPPORT_ADC
    #define ADC0_0_START_VOLTAGE                   33                    // initial voltages when simulating (mV)
    #define ADC0_1_START_VOLTAGE                   500
    #define ADC0_2_START_VOLTAGE                   1000
    #define ADC0_3_START_VOLTAGE                   1500
    #define ADC0_4_START_VOLTAGE                   2000
    #define ADC0_5_START_VOLTAGE                   2500
    #define ADC0_6_START_VOLTAGE                   3000
    #define ADC0_7_START_VOLTAGE                   3300
    #define ADC0_8_START_VOLTAGE                   600
    #define ADC0_9_START_VOLTAGE                   1000
    #define ADC0_10_START_VOLTAGE                  1500
    #define ADC0_11_START_VOLTAGE                  2000
    #define ADC0_12_START_VOLTAGE                  2500
    #define ADC0_13_START_VOLTAGE                  3000
    #define ADC0_14_START_VOLTAGE                  3300
    #define ADC0_15_START_VOLTAGE                  1500
    #define ADC0_16_START_VOLTAGE                  2000
    #define ADC0_17_START_VOLTAGE                  2500
    #define ADC0_18_START_VOLTAGE                  3000
    #define ADC0_19_START_VOLTAGE                  3300
    #define ADC0_20_START_VOLTAGE                  1500
    #define ADC0_21_START_VOLTAGE                  2000
    #define ADC0_22_START_VOLTAGE                  2500
    #define ADC0_23_START_VOLTAGE                  1650
    #if ADC_CONTROLLERS > 1
        #define ADC1_0_START_VOLTAGE               33
        #define ADC1_1_START_VOLTAGE               500
        #define ADC1_2_START_VOLTAGE               1000
        #define ADC1_3_START_VOLTAGE               1500
        #define ADC1_4_START_VOLTAGE               2000
        #define ADC1_5_START_VOLTAGE               2500
        #define ADC1_6_START_VOLTAGE               3000
        #define ADC1_7_START_VOLTAGE               3300
        #define ADC1_8_START_VOLTAGE               500
        #define ADC1_9_START_VOLTAGE               1000
        #define ADC1_10_START_VOLTAGE              1500
        #define ADC1_11_START_VOLTAGE              2000
        #define ADC1_12_START_VOLTAGE              2500
        #define ADC1_13_START_VOLTAGE              3000
        #define ADC1_14_START_VOLTAGE              3300
        #define ADC1_15_START_VOLTAGE              1500
        #define ADC1_16_START_VOLTAGE              2000
        #define ADC1_17_START_VOLTAGE              2500
        #define ADC1_18_START_VOLTAGE              3000
        #define ADC1_19_START_VOLTAGE              3300
        #define ADC1_20_START_VOLTAGE              1500
        #define ADC1_21_START_VOLTAGE              2000
        #define ADC1_22_START_VOLTAGE              2500
        #define ADC1_23_START_VOLTAGE              3000
    #endif
    #if ADC_CONTROLLERS > 2
        #define ADC2_0_START_VOLTAGE               33
        #define ADC2_1_START_VOLTAGE               500
        #define ADC2_2_START_VOLTAGE               1000
        #define ADC2_3_START_VOLTAGE               1500
        #define ADC2_4_START_VOLTAGE               2000
        #define ADC2_5_START_VOLTAGE               2500
        #define ADC2_6_START_VOLTAGE               3000
        #define ADC2_7_START_VOLTAGE               3300
        #define ADC2_8_START_VOLTAGE               500
        #define ADC2_9_START_VOLTAGE               1000
        #define ADC2_10_START_VOLTAGE              1500
        #define ADC2_11_START_VOLTAGE              2000
        #define ADC2_12_START_VOLTAGE              2500
        #define ADC2_13_START_VOLTAGE              3000
        #define ADC2_14_START_VOLTAGE              3300
        #define ADC2_15_START_VOLTAGE              1500
        #define ADC2_16_START_VOLTAGE              2000
        #define ADC2_17_START_VOLTAGE              2500
        #define ADC2_18_START_VOLTAGE              3000
        #define ADC2_19_START_VOLTAGE              3300
        #define ADC2_20_START_VOLTAGE              1500
        #define ADC2_21_START_VOLTAGE              2000
        #define ADC2_22_START_VOLTAGE              2500
        #define ADC2_23_START_VOLTAGE              3000
    #endif
    #if ADC_CONTROLLERS > 3
        #define ADC3_0_START_VOLTAGE               33
        #define ADC3_1_START_VOLTAGE               500
        #define ADC3_2_START_VOLTAGE               1000
        #define ADC3_3_START_VOLTAGE               1500
        #define ADC3_4_START_VOLTAGE               2000
        #define ADC3_5_START_VOLTAGE               2500
        #define ADC3_6_START_VOLTAGE               3000
        #define ADC3_7_START_VOLTAGE               3300
        #define ADC3_8_START_VOLTAGE               500
        #define ADC3_9_START_VOLTAGE               1000
        #define ADC3_10_START_VOLTAGE              1500
        #define ADC3_11_START_VOLTAGE              2000
        #define ADC3_12_START_VOLTAGE              2500
        #define ADC3_13_START_VOLTAGE              3000
        #define ADC3_14_START_VOLTAGE              3300
        #define ADC3_15_START_VOLTAGE              1500
        #define ADC3_16_START_VOLTAGE              2000
        #define ADC3_17_START_VOLTAGE              2500
        #define ADC3_18_START_VOLTAGE              3000
        #define ADC3_19_START_VOLTAGE              3300
        #define ADC3_20_START_VOLTAGE              1500
        #define ADC3_21_START_VOLTAGE              2000
        #define ADC3_22_START_VOLTAGE              2500
        #define ADC3_23_START_VOLTAGE              3000
    #endif
#endif

#define SUPPORT_COMPARATOR

//#define SUPPORT_LOW_VOLTAGE_DETECTION                                  // enable a low voltage detection interrupt handler
    #define LOW_VOLTAGE_DETECTION_VOLTAGE_mV       2900                  // 2.9V warning threshold


//#define SUPPORT_I2S_SAI                                                // support I2S/SAI

#if !defined KINETIS_WITHOUT_PIT
    #define SUPPORT_PITS                                                 // support PITs
      //#define SUPPORT_PIT_DMA_PORT_TOGGLE                              // PIT driver supports triggering port toggles
    // Define behavior of low power PIT (when available) in debug and doze mode
    #if defined SUPPORT_LOW_POWER
        #define LPIT_CHARACTERISTICS   (LPIT_MCR_DOZE_EN | LPIT_MCR_DBG_EN) // allow the LPIT to continue running in doze modes since it will otherwise freeze whenever the processor uses WAIT 
    #else
        #define LPIT_CHARACTERISTICS   (LPIT_MCR_DBG_EN)                 // allow the LPIT to continue running when the debug pauses the processor
    #endif
#endif
#if defined MODBUS_RTU && !defined SUPPORT_PITS
    #define SUPPORT_PITS                                                 // support PITs
#endif


#if defined SUPPORT_TIMER
    #define SUPPORT_PWM_MODULE                                           // enable PWM support from FlexTimers
    #if defined KINETIS_KE
      //#define FTM0_0_ON_B
        #if defined FRDM_KE04Z
            #define FTM0_1_ON_B                                          // blue LED on FRDM_KE04Z
        #endif
        #define FTM1_0_ON_H
      //#define FTM1_1_ON_E
      //#define FTM2_0_ON_H
        #if defined KINETIS_KE06
          //#define FTM2_0_ON_F
          //#define FTM2_1_ON_F
          //#define FTM2_2_ON_G
          //#define FTM2_3_ON_G
          //#define FTM2_4_ON_G
          //#define FTM2_5_ON_G
        #endif
      //#define FTM2_1_ON_H
      //#define FTM2_2_ON_D
    #else
        #if defined FRDM_K64F || defined FRDM_K82F
            #define FTM0_0_ON_C                                          // FTM0 channel 0 on port C rather than port A
            #define FTM0_1_ON_C                                          // FTM0 channel 1 on port C rather than port A
            #define FTM0_4_ON_D                                          // FTM0 channel 4 on port D rather than port A
            #define FTM0_5_ON_D                                          // FTM0 channel 5 on port D rather than port A
        #endif
        #if !defined TWR_KL46Z48M && !defined TWR_K64F120M && !defined BLAZE_K22
            #define FTM0_2_ON_C                                          // FTM0 channel 2 on port C rather than port A
        #endif
        #if defined KINETIS_K64
            #define FTM0_3_ON_C                                          // FTM0 channel 3 on port C rather than port A
        #endif
      //#define FTM0_6_ON_D                                              // FTM0 channel 6 on port D rather than port A
        #define FTM0_7_ON_D                                              // FTM0 channel 7 on port D rather than port A

      //#define FTM1_0_ALT_A                                             // FTM1 channel 0 on second port A location
      //#define FTM1_0_ON_B                                              // FTM1 channel 0 on port B rather than port A
      //#define FTM1_1_ALT_C                                             // FTM1 channel 1 on second port A location
      //#define FTM1_1_ON_B                                              // FTM1 channel 1 on port B rather than port A
        #if defined FRDM_KL26Z
            #define TPM1_0_ON_E
            #define TPM1_1_ON_E
        #endif

      //#define FTM2_0_ON_B                                              // FTM2 channel 0 on port B rather than port A
        #if defined FRDM_K66F
            #define FTM2_1_ON_B                                          // FTM2 channel 1 on port B rather than port A
        #else
          //#define FTM2_1_ON_B                                          // FTM2 channel 1 on port B rather than port A
        #endif
      //#define FTM2_0_ON_B_LOW
        #define FTM2_0_ON_B
        #define FTM2_1_ON_B_LOW

        #define FTM3_0_ON_D                                              // FTM3 channel 0 on port D rather than port E
        #define FTM3_1_ON_D                                              // FTM3 channel 1 on port D rather than port E
        #define FTM3_2_ON_D                                              // FTM3 channel 2 on port D rather than port E
        #define FTM3_3_ON_D                                              // FTM3 channel 3 on port D rather than port E
        #define FTM3_4_ON_C                                              // FTM3 channel 4 on port C rather than port E
        #define FTM3_5_ON_C                                              // FTM3 channel 5 on port C rather than port E
        #define FTM3_6_ON_C                                              // FTM3 channel 6 on port C rather than port E
        #define FTM3_7_ON_C                                              // FTM3 channel 7 on port C rather than port E

      //#define FTM_CLKIN_1                                              // use CLKIN1 rather than CLKIN0 as FlexTimer external clock
    #endif
#endif

#define FTM_DEBUG_BEHAVIOUR        FTM_CONF_BDMMODE_3                    // allow timer to continue operating when debugging
                                                                         // FTM_CONF_BDMMODE_2 - FTM counter stops and outputs are frozen in bdm mode
                                                                         // FTM_CONF_BDMMODE_1 - FTM counter stops and outputs set to safe state value in bdm mode
                                                                         // FTM_CONF_BDMMODE_0 - FTM counter stops in bdm mode

#define _TIMER_INTERRUPT_SETUP     PIT_SETUP

#define MODBUS0_PIT_TIMER_CHANNEL  0
#define MODBUS1_PIT_TIMER_CHANNEL  1
#define MODBUS2_PIT_TIMER_CHANNEL  2
#define MODBUS3_PIT_TIMER_CHANNEL  3

#define MODBUS0_PIT_INTERRUPT_PRIORITY  PIT0_INTERRUPT_PRIORITY
#define MODBUS1_PIT_INTERRUPT_PRIORITY  PIT1_INTERRUPT_PRIORITY
#define MODBUS2_PIT_INTERRUPT_PRIORITY  PIT2_INTERRUPT_PRIORITY
#define MODBUS3_PIT_INTERRUPT_PRIORITY  PIT3_INTERRUPT_PRIORITY

#if defined USE_MODBUS
    #if defined KINETIS_KL || defined KINETIS_KE
        #define UART_FRAME_END_COMPLETE                                  // make use of the end of character interrupt to inform of real end of frame
    #else
        #define AUTO_RS485_RTS_SUPPORT                                   // the Kinetis allows automatic RS485 RTS control on all UARTs
    #endif
#endif
#if !defined USE_DMX_RDM_MASTER
    #undef UART_FRAME_COMPLETE                                           // this can be disabled if not specifically needed for other purposes to MODBUS RS485 mode or DMX512 master
#endif

#if defined SUPPORT_LOW_POWER
    #define UART_PULL_UPS  (PORT_PS_UP_ENABLE)                           // activate pull-ups on UART lines to avoid them floating in low power modes (which causes leakage currents to flow)
#endif

//#define SUPPORT_KEYBOARD_INTERRUPTS                                    // support code for keyboard interrupts (KE/KEA devices)

// I2C Interface
//
#if defined I2C_INTERFACE
    #if defined MIMXRT1050 || defined MIMXRT1060 || defined MIMXRT1064
        #define OUR_I2C_CHANNEL        iMX_LPI2C_1                       // use LPI2C1 for reference
    #else
        #define OUR_I2C_CHANNEL        iMX_LPI2C_4                       // use LPI2C4 for reference
    #endif
    #if defined SUPPORT_LOW_POWER
        #define LPI2C_CHARACTERISTICS  (LPI2C_MCR_DOZEN | LPI2C_MCR_DBGEN) // allow the LPI2C to continue running in doze modes since it will otherwise freeze whenever the processor uses STOP (debug mode should always be enabled otherwise strange effects may be encountered! - reference https://community.nxp.com/thread/465202) 
    #else
        #define LPI2C_CHARACTERISTICS  (0 | LPI2C_MCR_DBGEN)             // define whether the LPI2C controller operation is enabled in debug mode (debug mode should always be enabled otherwise strange effects may be encountered! - reference https://community.nxp.com/thread/465202)
    #endif
#endif

// LAN interface
//
#if defined ETH_INTERFACE
    #define NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE         2            // allocate 2 buffers for transmission
    #define NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE         6            // allocate 6 (full) buffers for reception
    //#define USE_MULTIPLE_BUFFERS                                       // construct the receive buffer space using multiple 256 byte buffers (improved overrun performance for total memory requirement but reduced efficiency due to extra copy)
#endif
#if defined ETH_INTERFACE || defined USB_CDC_RNDIS || defined USE_PPP
    #if !defined USB_TO_ETHERNET && !defined USE_PPP
        #define IP_RX_CHECKSUM_OFFLOAD                                   // allow the HW to perform IPv4/v6 UDP, TCP and ICMP checksum verification so that no such calculation is required in code
        #define IP_TX_CHECKSUM_OFFLOAD                                   // allow the HW to insert IPv4/v6 header checksum and so the software doesn't need to calculate and insert this
            #define IP_TX_PAYLOAD_CHECKSUM_OFFLOAD                       // allow the HW to insert IPv4/v6 payload checksum and so the software doesn't need to calculate and insert this
    #endif

    #if defined USE_IPV6
        #define FORCE_PAYLOAD_ICMPV6_TX                                  // calculate value since the automatic off-loading doesn't do it
        #define FORCE_PAYLOAD_ICMPV6_RX                                  // perform checksum in software since the automatic offloading doesn't do it
    #endif

    // Set the interface handling characteristics based on the checksum offload settings used
    //
    #if (!defined IP_TX_CHECKSUM_OFFLOAD && !defined IP_RX_CHECKSUM_OFFLOAD) || defined _WINDOWS
        #define DEFAULT_INTERFACE_CHARACTERISTICS (INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING) // all checksum calculations are performed in software
    #elif defined IP_TX_CHECKSUM_OFFLOAD && defined IP_RX_CHECKSUM_OFFLOAD
        #if defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
            #define DEFAULT_INTERFACE_CHARACTERISTICS (0)                // all checksum calculations are offloaded
        #else
            #define DEFAULT_INTERFACE_CHARACTERISTICS (INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING) // only calculate checksums on transmitted payload
        #endif
    #elif defined IP_RX_CHECKSUM_OFFLOAD
        #define DEFAULT_INTERFACE_CHARACTERISTICS (INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING) // only calculate checksums on transmitted frames
    #elif defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
        #define DEFAULT_INTERFACE_CHARACTERISTICS     (0)                // all checksum calculations are offloaded
    #else
        #define DEFAULT_INTERFACE_CHARACTERISTICS (INTERFACE_NO_RX_CS_OFFLOADING) // only calculate checksums on received frames
    #endif

    #if defined USE_BUFFERED_TCP                                         // if using a buffer for TCP to allow interractive data applications (like TELNET)
        #define TCP_BUFFER            2800                               // size of TCP buffer (with USE_BUFFERED_TCP) - generous with Kinetis
        #define TCP_BUFFER_FRAME      1400                               // allow this max. TCP frame size
    #endif

    #if defined USE_HTTP
        #define HTTP_BUFFER_LENGTH    1400                               // we send frames with this maximum amount of payload data - generous with Kinetis
    #endif

    #if defined USE_FTP                                                  // specify FTP support details
        #define FTP_WELCOME_RESPONSE  "220 Welcome KINETIS FTP.\r\n"
        //                                 ^^^^^^^^^^^^^^^^^^^           customise the FTP welcome here
    #endif

    #define _MAGIC_RESET_FRAME        0x1234                             // allow a magic Ethernet reception frame to reset the board to boot loader mode (Ethernet protocol used) followed by three repetitions of the same value (total frame length 61 bytes)

  //#define LAN_REPORT_ACTIVITY                                          // transmissions/receptions and errors are reported to chosen task (for link LED control, etc)
    #if defined LAN_REPORT_ACTIVITY
        #define ACTIVITY_LED_OFF()     
        #define ACTIVITY_LED_ON()      
        #define TURN_LINK_LED_ON()     
        #define SPEED_LED_ON()         
        #define SPEED_LED_OFF()        
        #define LINK_DOWN_LEDS()       

        #define CONFIGURE_LAN_LEDS()
    #endif
#endif

#if defined USB_INTERFACE                                                // default is use of HSUSB1 only
    #define USE_BOTH_USB                                                 // select if both HSUSB1 and HSUSB2 will be used
  //#define USE_USB2_ONLY                                                // select if only HSUSB2 will be used (not HSUSB1)
#endif

// Special support for this processor type
//
// Presently this is left disabled becasue it hasn't shown any speed advantages: https://community.nxp.com/thread/518925
//
//#define DMA_MEMCPY_SET                                                 // memcpy and memset functions performed by DMA (if supported by processor - uses DMA channel DMA_MEMCPY_CHANNEL)
//#define CONFIGURE_CROSSBAR_SWITCH()  
//#define RUN_LOOPS_IN_RAM                                               // allow certain routines with tight loops to run from SRAM where it is generally faster than from FLASH - uses slightly more SRAM


// Define interrupt priorities in the system (M7 supports 0..15 - 0 is highest priority and 15 is lowest priority)
//
#define SYSTICK_PRIORITY           15                                    // lowest priority
#define USB_PIT_INTERRUPT_PRIORITY 14                                    // the PIT used by USB host must have a lower interrupt priority than the USB host itself!
#define PRIORITY_USB_HS_OTG0       13
#define PRIORITY_USB_HS_OTG1       13
#define PRIORITY_LPUART0           12
#define PRIORITY_LPUART1           11
#define PRIORITY_LPUART2           10
#define PRIORITY_LPUART3           9
#define PRIORITY_LPUART4           8
#define PRIORITY_LPUART5           7
#define PRIORITY_LPUART6           7
#define PRIORITY_LPUART7           7
#define PRIORITY_LPUART8           7
#define PRIORITY_DMA31             7
#define PRIORITY_DMA30             7
#define PRIORITY_DMA29             7
#define PRIORITY_DMA28             7
#define PRIORITY_DMA27             7
#define PRIORITY_DMA26             7
#define PRIORITY_DMA25             7
#define PRIORITY_DMA24             7
#define PRIORITY_DMA23             7
#define PRIORITY_DMA22             7
#define PRIORITY_DMA21             7
#define PRIORITY_DMA20             7
#define PRIORITY_DMA19             7
#define PRIORITY_DMA18             7
#define PRIORITY_DMA17             7
#define PRIORITY_DMA16             7
#define PRIORITY_DMA15             7
#define PRIORITY_DMA14             7
#define PRIORITY_DMA13             7
#define PRIORITY_DMA12             7
#define PRIORITY_DMA11             7
#define PRIORITY_DMA10             7
#define PRIORITY_DMA9              7
#define PRIORITY_DMA8              7
#define PRIORITY_DMA7              7
#define PRIORITY_DMA6              7
#define PRIORITY_DMA5              7
#define PRIORITY_DMA4              7
#define PRIORITY_DMA3              7
#define PRIORITY_DMA2              7
#define PRIORITY_DMA1              7
#define PRIORITY_DMA0              7
#define SDCARD1_STATUS_INTERRUPT_PRIORITY  6
#define SDCARD2_STATUS_INTERRUPT_PRIORITY  6
#define LPTMR0_INTERRUPT_PRIORITY  6
#define PRIORITY_PORT_1_0          6                                     // port 1 bits 0..7 individual interrupt priorities
#define PRIORITY_PORT_1_1          6
#define PRIORITY_PORT_1_2          6
#define PRIORITY_PORT_1_3          6
#define PRIORITY_PORT_1_4          6
#define PRIORITY_PORT_1_5          6
#define PRIORITY_PORT_1_6          6
#define PRIORITY_PORT_1_7          6
#define PRIORITY_PORT_1_LOW        6                                     // port 1 bits 0..15 interrupt priority
#define PRIORITY_PORT_1_HIGH       6                                     // port 1 bits 31..16 interrupt priority
#define PRIORITY_PORT_2_LOW        6                                     // port 2 bits 0..15 interrupt priority
#define PRIORITY_PORT_2_HIGH       6                                     // port 2 bits 31..16 interrupt priority
#define PRIORITY_PORT_3_LOW        6                                     // port 3 bits 0..15 interrupt priority
#define PRIORITY_PORT_3_HIGH       6                                     // port 3 bits 31..16 interrupt priority
#define PRIORITY_PORT_5_LOW        6                                     // port 5 bits 0..15 interrupt priority
#define PRIORITY_PORT_5_HIGH       6                                     // port 5 bits 31..16 interrupt priority
#define PRIORITY_PHY_PORT_INT      6
#define PRIORITY_USB_OTG           6
#define PRIORITY_PDB               6
#define PRIORITY_CAN0_MESSAGE      5
#define PRIORITY_CAN0_BUS_OFF      5
#define PRIORITY_CAN0_ERROR        5
#define PRIORITY_CAN0_TX           5
#define PRIORITY_CAN0_RX           5
#define PRIORITY_CAN0_WAKEUP       5
#define PRIORITY_CAN0_IMEU         5
#define PRIORITY_CAN1_MESSAGE      5
#define PRIORITY_CAN1_BUS_OFF      5
#define PRIORITY_CAN1_ERROR        5
#define PRIORITY_CAN1_TX           5
#define PRIORITY_CAN1_RX           5
#define PRIORITY_CAN1_WAKEUP       5
#define PRIORITY_CAN1_IMEU         5
#define PRIORITY_CAN2_MESSAGE      5
#define PRIORITY_CAN2_BUS_OFF      5
#define PRIORITY_CAN2_ERROR        5
#define PRIORITY_CAN2_TX           5
#define PRIORITY_CAN2_RX           5
#define PRIORITY_CAN2_WAKEUP       5
#define PRIORITY_CAN2_IMEU         5
#define PRIORITY_HW_TIMER          5
#define PRIORITY_TIMERS            5
#define PRIORITY_I2C0              4
#define PRIORITY_I2C1              4
#define PRIORITY_I2C2              4
#define PRIORITY_I2C3              4
#define PIT0_INTERRUPT_PRIORITY    4
#define PIT1_INTERRUPT_PRIORITY    4
#define PIT2_INTERRUPT_PRIORITY    4
#define PIT3_INTERRUPT_PRIORITY    4
#define PRIORITY_LCD               3
#define PRIORITY_TICK_TIMER        3
#define PRIORITY_SPI0              3
#define PRIORITY_SPI1              3
#define PRIORITY_SPI2              3
#define PRIORITY_COMPARATOR        3
#define PRIORITY_ADC               2
#define USB_PRIORITY               2
#define PRIORITY_RTC               2
#define PRIORITY_EMAC              1

//#define EMAC_PREEMPT_LEVEL            PIT0_INTERRUPT_PRIORITY          // don't allow interrupts of this priority and below to pre-empt Ethernet transmissions (such interrupts can then safely send Ethernet frames)
//#define SYSTEM_NO_DISABLE_LEVEL       1                                // allow interrupts of higher priority than this to not be blocked in critical regions
#define LOWEST_PRIORITY_PREEMPT_LEVEL   0                                // normal level is for all interrupts to be able to operate

#define ADC_ERR_PRIORITY           PRIORITY_ADC

// Define DMA channel use (channels and priorities must be unique for used peripherals)
//
#define DMA_UART0_TX_CHANNEL   2                                         // use this DMA channel when using UART 0 for transmission driven by DMA (must be 0..15)
#define DMA_UART1_TX_CHANNEL   3                                         // use this DMA channel when using UART 1 for transmission driven by DMA (must be 0..15)
#define DMA_UART2_TX_CHANNEL   20                                        // use this DMA channel when using UART 2 for transmission driven by DMA (must be 16..31)
#define DMA_UART3_TX_CHANNEL   21                                        // use this DMA channel when using UART 3 for transmission driven by DMA (must be 16..31)
#define DMA_UART4_TX_CHANNEL   22                                        // use this DMA channel when using UART 4 for transmission driven by DMA (must be 16..31)
#define DMA_UART5_TX_CHANNEL   23                                        // use this DMA channel when using UART 5 for transmission driven by DMA (must be 16..31)
#define DMA_UART6_TX_CHANNEL   24                                        // use this DMA channel when using UART 6 for transmission driven by DMA (must be 16..31)
#define DMA_UART7_TX_CHANNEL   25                                        // use this DMA channel when using UART 7 for transmission driven by DMA (must be 16..31)

#define DMA_UART0_RX_CHANNEL   8                                         // use this DMA channel when using UART 0 for reception driven by DMA (must be 0..15)
#define DMA_UART1_RX_CHANNEL   9                                         // use this DMA channel when using UART 1 for reception driven by DMA (must be 0..15)
#define DMA_UART2_RX_CHANNEL   14                                        // use this DMA channel when using UART 2 for reception driven by DMA (must be 16..31)
#define DMA_UART3_RX_CHANNEL   15                                        // use this DMA channel when using UART 3 for reception driven by DMA (must be 16..31)
#define DMA_UART4_RX_CHANNEL   16                                        // use this DMA channel when using UART 4 for reception driven by DMA (must be 16..31)
#define DMA_UART5_RX_CHANNEL   17                                        // use this DMA channel when using UART 5 for reception driven by DMA (must be 16..31)
#define DMA_UART6_RX_CHANNEL   18                                        // use this DMA channel when using UART 6 for reception driven by DMA (must be 16..31)
#define DMA_UART7_RX_CHANNEL   19                                        // use this DMA channel when using UART 7 for reception driven by DMA (must be 16..31)

                                                                         
#define DMA_UART0_TX_INT_PRIORITY  (PRIORITY_DMA2)                       // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART1_TX_INT_PRIORITY  (PRIORITY_DMA3)                       // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART2_TX_INT_PRIORITY  (PRIORITY_DMA20)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART3_TX_INT_PRIORITY  (PRIORITY_DMA21)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART4_TX_INT_PRIORITY  (PRIORITY_DMA22)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART5_TX_INT_PRIORITY  (PRIORITY_DMA23)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART6_TX_INT_PRIORITY  (PRIORITY_DMA24)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART7_TX_INT_PRIORITY  (PRIORITY_DMA25)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used

#define DMA_UART0_RX_INT_PRIORITY  (PRIORITY_DMA8)                       // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART1_RX_INT_PRIORITY  (PRIORITY_DMA9)                       // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART2_RX_INT_PRIORITY  (PRIORITY_DMA14)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART3_RX_INT_PRIORITY  (PRIORITY_DMA15)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART4_RX_INT_PRIORITY  (PRIORITY_DMA16)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART5_RX_INT_PRIORITY  (PRIORITY_DMA17)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART6_RX_INT_PRIORITY  (PRIORITY_DMA18)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used
#define DMA_UART7_RX_INT_PRIORITY  (PRIORITY_DMA19)                      // the interrupts used by the DMA transfer completion need to match with the DMA channel used

                                                                         
#define DMA_MEMCPY_CHANNEL_ALT 1//30                                     // alternative DMA channel to use if DMA based memory to memory operations are already in progress
#define DMA_MEMCPY_CHANNEL     0//31                                     // use this DMA channel when memory to memory operations are performed (this should have lowest priority and can be stalled by higher priority channels)


// Ports
//
#if defined MIMXRT1010
    #define BLINK_LED              (PORT1_BIT11)                         // USER_LED ('1' drives LED on)

    #define USER_BUTTON            PORT2_BIT5
    #define USER_BUTTON_PORT       _PORT2

    #define INIT_WATCHDOG_LED()   _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))

    #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(2, USER_BUTTON, PORT_PS_UP_ENABLE) // configure input
    #define WATCHDOG_DISABLE()      (_READ_PORT_MASK(2, USER_BUTTON) == 0) // pull this input down to disable watchdog (hold USER_BUTTON at reset)

    #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(1, BLINK_LED)

    #if defined iMX_FALLBACK_SERIAL_LOADER
        #define FORCE_BOOT()            (1)                              // always start in fall-back serial loader
        #define RETAIN_LOADER_MODE()    (1)                              // always retain the fall-back serial loader
    #else
        #define FORCE_BOOT()            ((_READ_PORT_MASK(2, USER_BUTTON) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // hold user button at reset to force loader mode
        #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(2, USER_BUTTON) == 0) // hold user button at reset to force loader mode
    #endif

    #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1010-EVK.bmp"

    #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {349, 230,  369, 252}},

        // '0'          '1'           input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
    #define KEYPAD_LED_DEFINITIONS  \
        {RGB(0,0,0), RGB(0, 255, 0),  0,    {235, 227, 243, 236}, _PORT1, BLINK_LED}

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#elif defined MIMXRT1015
    #define USER_LED               PIN_GPIO_SD_B1_00_GPIO3_IO20          // USER_LED ('0' drives LED on)

    #define DEMO_LED_1             (USER_LED)                            // if the port is changed (eg. A to B) the port macros will require appropriate adjustment too
    #define BLINK_LED              DEMO_LED_1

    #define USER_BUTTON            PIN_GPIO_EMC_09_GPIO2_IO09
    #define USER_BUTTON_PORT       _PORT2

    #define INIT_WATCHDOG_LED() _CONFIG_DRIVE_PORT_OUTPUT_VALUE(3, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))

    #if defined iMX_FALLBACK_SERIAL_LOADER
        #define FORCE_BOOT()            (1)                              // always start in fall-back serial loader
        #define RETAIN_LOADER_MODE()    (1)                              // always retain the fall-back serial loader
    #else
        #define FORCE_BOOT()            ((_READ_PORT_MASK(2, USER_BUTTON) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // hold user button at reset to force loader mode
        #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(2, USER_BUTTON) == 0) // hold user button at reset to force loader mode
    #endif

    #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(2, USER_BUTTON, PORT_PS_UP_ENABLE) // configure input
    #define WATCHDOG_DISABLE()      (_READ_PORT_MASK(2, USER_BUTTON) == 0)  // pull this input down to disable watchdog (hold USER_BUTTON at reset)

    #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(3, BLINK_LED)

    #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1015-EVK.bmp"

    #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {13, 102,  34, 124}},

        // '0'          '1'           input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
    #define KEYPAD_LED_DEFINITIONS  \
        {RGB(0, 255, 0),  RGB(0,0,0), 1,    {181, 198, 188, 209}, _PORT3, USER_LED},

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#elif defined MIMXRT1020 || defined MIMXRT1024
    #define USER_LED               (PORT1_BIT5)                          // USER_LED ('0' drives LED on)

    #define BLINK_LED              USER_LED

    #define USER_BUTTON            PORT5_BIT0                            // user button SW4
    #define USER_BUTTON_PORT       _PORT5

    #define INIT_WATCHDOG_LED()    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))

    #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(5, USER_BUTTON, PORT_PS_UP_ENABLE)
    #define WATCHDOG_DISABLE()      (0)                                  // pull this input down to disable watchdog (hold USER_BUTTON at reset)

    #if defined iMX_FALLBACK_SERIAL_LOADER
        #define FORCE_BOOT()            (1)                              // always start in fall-back serial loader
        #define RETAIN_LOADER_MODE()    (1)                              // always retain the fall-back serial loader
    #else
        #define FORCE_BOOT()            ((_READ_PORT_MASK(5, USER_BUTTON) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // hold user button SW4 at reset to force loader mode
        #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(5, USER_BUTTON) == 0) // keep user button SW4 held down to retain loader mode after SD card or memory stick has been checked
    #endif

    #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(1, BLINK_LED)

    #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1020-EVK.bmp"

    #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {17, 103, 35, 120 }},

        // '0'          '1'           input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
    #define KEYPAD_LED_DEFINITIONS  \
        {RGB(0, 255,  0  ), RGB(0,0,0),  1, {257, 186,  264, 195 }, _PORT1, USER_LED}

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#elif defined MIMXRT1050 || defined MIMXRT1060 || defined MIMXRT1064 || defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
    #if defined MIMXRT1060 && defined DEV8
        #define USER_LED           PIN_WAKEUP_GPIO5_IO00                 // USER_LED ('0' drives LED on)
        #define USER_LED_PORT     _PORT5
        #define USER_BUTTON        PIN_GPIO_AD_B0_14_GPIO1_IO14
        #define USER_BUTTON_PORT   _PORT1
    #else
        #define USER_LED           PIN_GPIO_AD_B0_09_GPIO1_IO09          // USER_LED ('0' drives LED on)
        #define USER_LED_PORT     _PORT1
        #define USER_BUTTON        PIN_WAKEUP_GPIO5_IO00
        #define USER_BUTTON_PORT   _PORT5
    #endif

    #define BLINK_LED              USER_LED

    #if defined MIMXRT1060 && defined DEV8
        #define INIT_WATCHDOG_LED()    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(5, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))
    #else
        #define INIT_WATCHDOG_LED()    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))
    #endif

    #if defined iMX_FALLBACK_SERIAL_LOADER
        #define FORCE_BOOT()            (1)                              // always start in fall-back serial loader
        #define RETAIN_LOADER_MODE()    (1)                              // always retain the fall-back serial loader
    #else
        #if defined MIMXRT1060 && defined DEV8
            #define FORCE_BOOT()            ((_READ_PORT_MASK(1, USER_BUTTON) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // hold user button SW4 at reset to force loader mode
            #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(1, USER_BUTTON) == 0) // keep input pulled down to retain loader mode after SD card or memory stick has been checked
        #else
            #define FORCE_BOOT()            ((_READ_PORT_MASK(5, USER_BUTTON) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // hold user button SW4 at reset to force loader mode
            #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(5, USER_BUTTON) == 0) // keep user button SW4 held down to retain loader mode after SD card or memory stick has been checked
        #endif
    #endif

    #if defined MIMXRT1060 && defined DEV8
        #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(1, USER_BUTTON, PORT_PS_UP_ENABLE) // configure input
        #define WATCHDOG_DISABLE()      (_READ_PORT_MASK(1, USER_BUTTON) == 0)  // pull this input down to disable watchdog (hold USER_BUTTON at reset)
    #else
        #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(5, USER_BUTTON, PORT_PS_UP_ENABLE) // configure input
        #define WATCHDOG_DISABLE()      (_READ_PORT_MASK(5, USER_BUTTON) == 0)  // pull this input down to disable watchdog (hold USER_BUTTON at reset)
    #endif

    #if defined MIMXRT1060 && defined DEV8
        #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(5, BLINK_LED)
    #else
        #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(1, BLINK_LED)
    #endif

    #if defined MIMXRT1060
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1060-EVK.bmp"
    #elif defined MIMXRT1064
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1064-EVK.bmp"
    #elif defined iMX_RT1052_EMB_ART || defined iMX_RT1062_EMB_ART
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/EmbeddedArt_OEM.bmp"
    #else
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/MIMXRT1050-EVK.bmp"
    #endif

    #if defined MIMXRT1064
        #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {155, 98, 169, 113 }},
        //                             '0'           '1'     input state  center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS {RGB(0,255,0), RGB(0,0,0),  1,     {344, 200,  350, 209 },               USER_LED_PORT,    USER_LED}
    #else
        #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {167, 97, 185, 113 }},
        //                             '0'           '1'     input state  center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS {RGB(0,255,0), RGB(0,0,0),  1,     {382, 212,  389, 223 },               USER_LED_PORT,    USER_LED}
    #endif

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#elif defined ARCH_MIX
    #define DEMO_LED_1             (PORT1_BIT10)                         // if the port is changed (eg. A to B) the port macros will require appropriate adjustment too
    #define DEMO_LED_2             (PORT1_BIT9)                          // red led - if the port is changed (eg. A to B) the port macros will require appropriate adjustment too
    #define DEMO_LED_3             (PORT1_BIT11)                         // blue led - if the port is changed (eg. A to B) the port macros will require appropriate adjustment too
    #define DEMO_LED_4             (0)
    #define BLINK_LED              DEMO_LED_1

    #define USER_BUTTON            PORT5_BIT0
    #define USER_BUTTON_PORT       _PORT5

    #if defined USE_MAINTENANCE && !defined REMOVE_PORT_INITIALISATIONS
        #define INIT_WATCHDOG_LED()                                      // let the port set up do this (the user can disable blinking)
    #else
        #define INIT_WATCHDOG_LED() _CONFIG_DRIVE_PORT_OUTPUT_VALUE(1, (BLINK_LED | DEMO_LED_2), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))
    #endif

    #define SHIFT_DEMO_LED_1        10                                   // since the port bits are spread out shift each to the lowest 4 bits
    #define SHIFT_DEMO_LED_2        8
    #define SHIFT_DEMO_LED_3        9
    #define SHIFT_DEMO_LED_4        0

    #define MAPPED_DEMO_LED_1       (DEMO_LED_1 >> SHIFT_DEMO_LED_1)
    #define MAPPED_DEMO_LED_2       (DEMO_LED_2 >> SHIFT_DEMO_LED_2)
    #define MAPPED_DEMO_LED_3       (DEMO_LED_3 >> SHIFT_DEMO_LED_3)
    #define MAPPED_DEMO_LED_4       (DEMO_LED_4 >> SHIFT_DEMO_LED_4)

    #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(5, USER_BUTTON, PORT_PS_UP_ENABLE) // configure input
    #define WATCHDOG_DISABLE()      (_READ_PORT_MASK(5, USER_BUTTON) == 0)  // pull this input down to disable watchdog (hold USER_BUTTON at reset)

    #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(1, BLINK_LED)

    #define CONFIG_TEST_OUTPUT()                                         // we use DEMO_LED_2 which is configured by the user code (and can be disabled in parameters if required)
    #define TOGGLE_TEST_OUTPUT()    _TOGGLE_PORT(1, DEMO_LED_2)
    #define SET_TEST_OUTPUT()       _SETBITS(1, DEMO_LED_2)
    #define CLEAR_TEST_OUTPUT()     _CLEARBITS(1, DEMO_LED_2)

    #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/ArchMix.bmp"

    #define BUTTON_KEY_DEFINITIONS  {USER_BUTTON_PORT, USER_BUTTON,   {410, 161, 434, 178 }},

    #define MULTICOLOUR_LEDS        {0, 2}                               // single LED made up of entries 0, 1 and 2

    // '0'          '1'             input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
    #define KEYPAD_LED_DEFINITIONS  \
            {RGB(255,0,0),  RGB(40,40,40),  1, {421, 137, 0, 8}, _PORT1, DEMO_LED_2}, \
            {RGB(255,255,0),RGB(40,40,40),  1, {421, 137, 0, 8}, _PORT1, DEMO_LED_3},\
            {RGB(0,255,0),  RGB(40,40,40),  1, {421, 137, 0, 8}, _PORT1, DEMO_LED_1},

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#elif defined TEENSY_4_0 || defined TEENSY_4_1
    #define BLINK_LED              PORT2_BIT3                            // USER_LED ('1' drives LED on)
    #define FORCE_LOADER_INPUT     PORT1_BIT3

    #define INIT_WATCHDOG_LED() _CONFIG_DRIVE_PORT_OUTPUT_VALUE(2, (BLINK_LED), (BLINK_LED), (PORT_SRE_SLOW | PORT_DSE_HIGH))

    #if defined iMX_FALLBACK_SERIAL_LOADER
        #define FORCE_BOOT()            (1)                              // always start in fall-back serial loader
        #define RETAIN_LOADER_MODE()    (1)                              // always retain the fall-back serial loader
    #else
        #define FORCE_BOOT()            ((_READ_PORT_MASK(1, FORCE_LOADER_INPUT) == 0) || (*BOOT_MAIL_BOX == RESET_TO_SERIAL_LOADER)) // connect pin 0 to 0V at reset to force loader mode
        #define RETAIN_LOADER_MODE()    (_READ_PORT_MASK(1, FORCE_LOADER_INPUT) == 0) // keep pin 0 connected to 0V to retain loader mode after SD card or memory stick has been checked
    #endif

    #define INIT_WATCHDOG_DISABLE() _CONFIG_PORT_INPUT(1, PORT1_BIT3, PORT_PS_UP_ENABLE)
    #define WATCHDOG_DISABLE()      0                                    // watchdog always enabled

    #define TOGGLE_WATCHDOG_LED()   _TOGGLE_PORT(2, BLINK_LED)

    #if defined TEENSY_4_1
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/Teensy_4-1.bmp"

            // '0'          '1'           input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS  \
            {RGB(0, 0,  0  ), RGB(255,128,0),  0, {468, 97,  478, 119 }, _PORT2, BLINK_LED},
    #else
        #define KEYPAD "../../uTaskerV1.4/Simulator/KeyPads/Teensy_4-0.bmp"

        #define BUTTON_KEY_DEFINITIONS  {_PORT1,           PORT1_BIT3,    { 58, 192,  72, 209 }}, \
                                        {_PORT1,           PORT1_BIT2,    { 89, 192, 106, 209 }}, \
                                        {_PORT4,           PORT4_BIT4,    {121, 192, 140, 209 }}, \
                                        {_PORT4,           PORT4_BIT5,    {152, 192, 171, 209 }}, \
                                        {_PORT4,           PORT4_BIT6,    {184, 192, 202, 209 }}, \
                                        {_PORT4,           PORT4_BIT8,    {214, 192, 233, 209 }}, \
                                        {_PORT2,           PORT2_BIT10,   {248, 192, 267, 209 }}, \
                                        {_PORT2,           PORT2_BIT17,   {277, 192, 297, 209 }}, \
                                        {_PORT2,           PORT2_BIT16,   {309, 192, 329, 209 }}, \
                                        {_PORT2,           PORT2_BIT11,   {342, 192, 360, 209 }}, \
                                        {_PORT2,           PORT2_BIT0,    {373, 192, 393, 209 }}, \
                                        {_PORT2,           PORT2_BIT2,    {404, 192, 422, 209 }}, \
                                        {_PORT2,           PORT2_BIT1,    {435, 192, 455, 209 }}, \
                                        {_PORT2,           PORT2_BIT3,    {435,   9, 455,  24 }}, \
                                        {_PORT1,           PORT1_BIT18,   {404,   9, 422,  24 }}, \
                                        {_PORT1,           PORT1_BIT19,   {373,   9, 393,  24 }}, \
                                        {_PORT1,           PORT1_BIT23,   {342,   9, 360,  24 }}, \
                                        {_PORT1,           PORT1_BIT22,   {309,   9, 329,  24 }}, \
                                        {_PORT1,           PORT1_BIT17,   {277,   9, 297,  24 }}, \
                                        {_PORT1,           PORT1_BIT16,   {248,   9, 267,  24 }}, \
                                        {_PORT1,           PORT1_BIT26,   {214,   9, 233,  24 }}, \
                                        {_PORT1,           PORT1_BIT27,   {184,   9, 202,  24 }}, \
                                        {_PORT1,           PORT1_BIT24,   {152,   9, 171,  24 }}, \
                                        {_PORT1,           PORT1_BIT25,   {121,   9, 140,  24 }},

            // '0'          '1'           input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS  \
            {RGB(0, 0,  0  ), RGB(255,128,0),  0, {413, 51,  426, 71 }, _PORT2, BLINK_LED}, \
            {RGB(0, 0,  0  ), RGB(255,0,  0),  0, {443, 17,  0,    3 }, _PORT2, BLINK_LED},
    #endif

    extern unsigned char FlexSPI_CS;
    #define SPI_CS0_PORT FlexSPI_CS
    #define CS0_LINE     1
#endif

#if !defined USB_HOST_POWER_CONFIG                                       // dummy USB host controls when nothing specific is defined
    #define USB_HOST_POWER_CONFIG()
    #define USB_HOST_POWER_ON()
    #define USB_HOST_POWER_OFF()
#endif


#define CONFIG_TIMER_TEST_LEDS()   
#define TIMER_TEST_LED_ON()        
#define TIMER_TEST_LED2_ON()       
#define TIMER_TEST_LED_OFF()       
#define TIMER_TEST_LED2_OFF()  


#define PORT1_DEFAULT_INPUT        0xffffffff                            // initial input states for port simulator (port 1)
#define PORT2_DEFAULT_INPUT        0xffffffff                            // port 2
#define PORT3_DEFAULT_INPUT        0xffffffff                            // port 3
#define PORT4_DEFAULT_INPUT        0xffffffff                            // port 4
#define PORT5_DEFAULT_INPUT        0xffffffff                            // port 5
#define PORT6_DEFAULT_INPUT        0xffffffff                            // port 6
#define PORT7_DEFAULT_INPUT        0xffffffff                            // port 7
#define PORT8_DEFAULT_INPUT        0xffffffff                            // port 8
#define PORT9_DEFAULT_INPUT        0xffffffff                            // port 9

// User port mapping
//
#define USER_PORT_1_BIT            PORT2_BIT0                             // use free pins on Eval board
#define USER_PORT_2_BIT            PORT2_BIT1
#define USER_PORT_3_BIT            PORT2_BIT2
#define USER_PORT_4_BIT            PORT2_BIT3
#define USER_PORT_5_BIT            PORT2_BIT4
#define USER_PORT_6_BIT            PORT2_BIT5
#define USER_PORT_7_BIT            PORT2_BIT6
#define USER_PORT_8_BIT            PORT2_BIT7
#define USER_PORT_9_BIT            PORT2_BIT8
#define USER_PORT_10_BIT           PORT2_BIT9
#define USER_PORT_11_BIT           PORT2_BIT10
#define USER_PORT_12_BIT           PORT2_BIT11
#define USER_PORT_13_BIT           PORT2_BIT12
#define USER_PORT_14_BIT           PORT2_BIT13
#define USER_PORT_15_BIT           PORT2_BIT14
#define USER_PORT_16_BIT           PORT2_BIT15

#define _CONFIG_OUTPUT_PORT_1()    _CONFIG_PORT_OUTPUT(2, USER_PORT_1_BIT,  (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_2()    _CONFIG_PORT_OUTPUT(2, USER_PORT_2_BIT,  (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_3()    _CONFIG_PORT_OUTPUT(2, USER_PORT_3_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_4()    _CONFIG_PORT_OUTPUT(2, USER_PORT_4_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_5()    _CONFIG_PORT_OUTPUT(2, USER_PORT_5_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_6()    _CONFIG_PORT_OUTPUT(2, USER_PORT_6_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_7()    _CONFIG_PORT_OUTPUT(2, USER_PORT_7_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_8()    _CONFIG_PORT_OUTPUT(2, USER_PORT_8_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_9()    _CONFIG_PORT_OUTPUT(2, USER_PORT_9_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_10()   _CONFIG_PORT_OUTPUT(2, USER_PORT_10_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_11()   _CONFIG_PORT_OUTPUT(2, USER_PORT_11_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_12()   _CONFIG_PORT_OUTPUT(2, USER_PORT_12_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_13()   _CONFIG_PORT_OUTPUT(2, USER_PORT_13_BIT,  (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_14()   _CONFIG_PORT_OUTPUT(2, USER_PORT_14_BIT,  (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_15()   _CONFIG_PORT_OUTPUT(2, USER_PORT_15_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))
#define _CONFIG_OUTPUT_PORT_16()   _CONFIG_PORT_OUTPUT(2, USER_PORT_16_BIT, (PORT_SRE_SLOW | PORT_DSE_MID))

// Port use definitions
//
#define USER_SET_PORT_1        GPIO1_DR_SET
#define USER_CLEAR_PORT_1      GPIO1_DR_CLEAR
#define USER_PORT_1            GPIO1_DR
#define USER_TOGGLE_PORT_1     GPIO1_DR_TOGGLE
#define USER_DDR_1             GPIOD_PDDR

#define USER_SET_PORT_2        GPIO1_DR_SET
#define USER_CLEAR_PORT_2      GPIO1_DR_CLEAR
#define USER_PORT_2            GPIO1_DR
#define USER_TOGGLE_PORT_2     GPIO1_DR_TOGGLE
#define USER_DDR_2             GPIOD_PDDR

#define USER_SET_PORT_3        GPIO1_DR_SET
#define USER_CLEAR_PORT_3      GPIO1_DR_CLEAR
#define USER_PORT_3            GPIO1_DR
#define USER_TOGGLE_PORT_3     GPIO1_DR_TOGGLE
#define USER_DDR_3             GPIOD_PDDR

#define USER_SET_PORT_4        GPIO1_DR_SET
#define USER_TOGGLE_PORT_4     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_4      GPIO1_DR
#define USER_PORT_4            GPIO1_DR_TOGGLE
#define USER_DDR_4             GPIOD_PDDR

#define USER_SET_PORT_5        GPIO1_DR_SET
#define USER_TOGGLE_PORT_5     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_5      GPIO1_DR
#define USER_PORT_5            GPIO1_DR_TOGGLE
#define USER_DDR_5             GPIOD_PDDR

#define USER_SET_PORT_6        GPIO1_DR_SET
#define USER_TOGGLE_PORT_6     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_6      GPIO1_DR
#define USER_PORT_6            GPIO1_DR_TOGGLE
#define USER_DDR_6             GPIOD_PDDR

#define USER_SET_PORT_7        GPIO1_DR_SET
#define USER_TOGGLE_PORT_7     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_7      GPIO1_DR
#define USER_PORT_7            GPIO1_DR_TOGGLE
#define USER_DDR_7             GPIOD_PDDR

#define USER_SET_PORT_8        GPIO1_DR_SET
#define USER_TOGGLE_PORT_8     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_8      GPIO1_DR
#define USER_PORT_8            GPIO1_DR_TOGGLE
#define USER_DDR_8             GPIOD_PDDR

#define USER_SET_PORT_9        GPIO1_DR_SET
#define USER_TOGGLE_PORT_9     GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_9      GPIO1_DR
#define USER_PORT_9            GPIO1_DR_TOGGLE
#define USER_DDR_9             GPIOD_PDDR

#define USER_SET_PORT_10       GPIO1_DR_SET
#define USER_TOGGLE_PORT_10    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_10     GPIO1_DR
#define USER_PORT_10           GPIO1_DR_TOGGLE
#define USER_DDR_10            GPIOD_PDDR

#define USER_SET_PORT_11       GPIO1_DR_SET
#define USER_TOGGLE_PORT_11    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_11     GPIO1_DR
#define USER_PORT_11           GPIO1_DR_TOGGLE
#define USER_DDR_11            GPIOD_PDDR

#define USER_SET_PORT_12       GPIO1_DR_SET
#define USER_TOGGLE_PORT_12    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_12     GPIO1_DR
#define USER_PORT_12           GPIO1_DR_TOGGLE
#define USER_DDR_12            GPIOD_PDDR

#define USER_SET_PORT_13       GPIO1_DR_SET
#define USER_TOGGLE_PORT_13    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_13     GPIO1_DR
#define USER_PORT_13           GPIO1_DR_TOGGLE
#define USER_DDR_13            GPIOD_PDDR

#define USER_SET_PORT_14       GPIO1_DR_SET
#define USER_TOGGLE_PORT_14    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_14     GPIO1_DR
#define USER_PORT_14           GPIO1_DR_TOGGLE
#define USER_DDR_14            GPIOD_PDDR

#define USER_SET_PORT_15       GPIO1_DR_SET
#define USER_TOGGLE_PORT_15    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_15     GPIO1_DR
#define USER_PORT_15           GPIO1_DR_TOGGLE
#define USER_DDR_15            GPIOD_PDDR

#define USER_SET_PORT_16       GPIO1_DR_SET
#define USER_TOGGLE_PORT_16    GPIO1_DR_CLEAR
#define USER_CLEAR_PORT_16     GPIO1_DR
#define USER_PORT_16           GPIO1_DR_TOGGLE
#define USER_DDR_16            GPIOD_PDDR

#define USB_DOWNLOAD_FILE      "z"                                       // software upload via USB-CDC is saved to this internal file location

#if defined NAND_FLASH_FAT
    #define MT29F1G08ABADAH4                                             // 8 bit
  //#define MT29F2G16ABAEAWP                                             // 16-bit default
    #define GET_SDCARD_WP_STATE() 0                                      // write protection disabled (change to read switch is required)
    #define SET_SD_CS_HIGH()
    #define POWER_DOWN_SD_CARD()
#endif

#if defined SDRAM_CONTROLLER_AVAILABLE_
    #if defined SUPPORT_TFT || defined TFT_GLCD_MODE                     // TWR-LCD-RGB
        #define USE_SDRAM                                                // enable SDRAM since it is required by the LCD controller
        #define SDRAM_SIZE   (64 * 1024 * 1024)                          // 64 Meg SDRAM
        #define SDRAM_ADDR   0x70000000
    #else
        #define USE_SDRAM                                                // enable SDRAM
    #endif
    #if defined TWR_K70F120M || defined K70F150M_12M
        #define MT47H64M16HR                                             // used on K70 tower kit card
        #define DDRPORT_SYNC                                             // synchronous DDR
    #else
        #define MT46H32M16LFBF                                           // used on EMCRAFT_K70F120M/EMCRAFT_K61F150M
    #endif

    #if defined USE_SDRAM
        #if !defined USB_CLOCK_SOURCE_MCGPLL1CLK                         // if FS USB defines the clock speed we don't determine it here
            #define CLOCK_DIV_1     5                                    // PLL1 uses clock input at 50MHz so divide by 5 to 10MHz
            #if KINETIS_MAX_DDR_SPEED >= 150000000
                #define CLOCK_MUL_1 30                                   // to give 150MHz output (300MHz PLL output is divided by 2)
            #else
                #define CLOCK_MUL_1 25                                   // to give 125MHz output (according to JEDEC DDR2 specification the speed should by >= 125MHz for DDR2)
            #endif
        #endif
      //#define SECONDARY_UMALLOC                                        // secondary uMalloc() area in SDRAM
      //#define HEAP2_ADDRESS     0x70000000                             // DRAM start address in memory map
      //#define HEAP2_SIZE        (64 * 1024 * 1024)                     // 64 Meg SDRAM

        #if defined SECONDARY_UMALLOC
            // Configure SRAM heap utilisation [undefine usage type to put heap to SDRAM rather than internal SRAM]
            // - application code can directly specify uMalloc() or uMalloc2() as required
            //
            #define OS_MALLOC(x)                  uMalloc2((unsigned long)(x))
          //#define USB_FS_MALLOC(x)              uMalloc2((unsigned long)(x))
          //#define USB_FS_MALLOC_ALIGN(x, y)     uMallocAlign2((MAX_MALLOC)(x), (y))
          //#define USB_HS_MALLOC(x)              uMalloc2((unsigned long)(x))
          //#define USB_HS_MALLOC_ALIGN(x, y)     uMallocAlign2((MAX_MALLOC)(x), (y))
            #define ETH_BD_MALLOC_ALIGN(x, y)     uMallocAlign2((MAX_MALLOC)(x), (y))
            #define ETH_RX_BUF_MALLOC_ALIGN(x, y) uMallocAlign2((MAX_MALLOC)(x), (y))
            #define ETH_TX_BUF_MALLOC_ALIGN(x, y) uMallocAlign2((MAX_MALLOC)(x), (y))
          //#define SSC_DRV_MALLOC(x)             uMalloc2((unsigned long)(x))
          //#define SCC_DRV_MALLOC_ALIGN(x, y)    uMallocAlign2((MAX_MALLOC)(x), (y))
            #define ARP_NN_MALLOC(x)              uMalloc2((unsigned long)(x))
          //#define HTTP_MALLOC(x)                uMalloc2((unsigned long)(x))
          //#define TCP_SOCKET_MALLOC(x)          uMalloc2((unsigned long)(x))
          //#define TCP_BUFFER_MALLOC(x)          uMalloc2((unsigned long)(x))
          //#define TELNET_MALLOC(x)              uMalloc2((unsigned long)(x))
          //#define UDP_SOCKET_MALLOC(x)          uMalloc2((unsigned long)(x))
          //#define QUEUE_MALLOC(x)               uMalloc2((unsigned long)(x))
          //#define TTY_DRV_MALLOC(x)             uMalloc2((unsigned long)(x))
          //#define USB_DRV_MALLOC(x)             uMalloc2((unsigned long)(x))
            #define ETH_DRV_MALLOC(x)             uMalloc2((unsigned long)(x))
          //#define UFILE_MALLOC(x)               uMalloc2((unsigned long)(x))
          //#define UNETWORK_MALLOC(x)            uMalloc2((unsigned long)(x))
          //#define NAND_MALLOC(x)                uMalloc2((unsigned long)(x))
          //#define SDCARD_MALLOC(x)              uMalloc2((unsigned long)(x))
        #endif
    #endif
#endif

// Character LCD interface
//
typedef unsigned long LCD_BUS_PORT_SIZE;                                 // we use 32 bit ports
typedef unsigned long LCD_CONTROL_PORT_SIZE;
//#define LCD_BUS_8BIT                                                   // data bus in 8 bit mode
#define LCD_BUS_4BIT                                                     // data bus in 4 bit mode


#if defined LCD_BUS_8BIT
    #define LCD_BUS_MASK        0x7f800000
    #define DATA_SHIFT_RIGHT    0    
    #define DATA_SHIFT_LEFT     23                                      // byte shift down required to bring data into position
#else
    #define LCD_BUS_MASK        0x0f000000
    #define DATA_SHIFT_RIGHT    0                                   
    #define DATA_SHIFT_LEFT     20                                      // nibble shift down required to bring data into position
#endif

#define O_CONTROL_RS            PORTC_BIT16
#define O_WRITE_READ            PORTC_BIT17
#define O_CONTROL_EN            PORTC_BIT18
#define O_LCD_BACKLIGHT         PORTC_BIT19

#define O_CONTROL_LINES         (O_CONTROL_RS | O_WRITE_READ | O_CONTROL_EN)
#define IO_BUS_PORT_DAT_IN      GPIOC_PDIR

#define SET_DATA_LINES_INPUT()  _CONFIG_PORT_INPUT(C, LCD_BUS_MASK, PORT_PS_UP_ENABLE)
#define SET_DATA_LINES_OUTPUT() _DRIVE_PORT_OUTPUT(C, LCD_BUS_MASK)
#define SET_CONTROL_LINES_OUTPUT(x) _CONFIG_PORT_OUTPUT(C, x, PORT_SRE_SLOW)
#define SET_BUS_DATA(x)         _WRITE_PORT_MASK(C, x,  LCD_BUS_MASK)
#define SET_CONTROL_LINES(x)    _WRITE_PORT_MASK(C, x, O_CONTROL_LINES)
#define O_SET_CONTROL_LOW(x)    _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _CLEARBITS(C, x); _SIM_PORTS;
#define O_SET_CONTROL_HIGH(x)   _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SETBITS(C, x); _SIM_PORTS;


// Drive the control lines R/W + LCD Backlight '1', RS + E '0' and the data lines with all high impedance at start up
// enable clocks to port first
//
#define INITIALISE_LCD_CONTROL_LINES()       SET_DATA_LINES_INPUT(); \
                                             O_SET_CONTROL_LOW(O_CONTROL_LINES); O_SET_CONTROL_HIGH(O_LCD_BACKLIGHT); \
                                             _SIM_PORTS; SET_CONTROL_LINES_OUTPUT(O_CONTROL_LINES | O_LCD_BACKLIGHT);

#define LCD_DRIVE_DATA()       SET_DATA_LINES_OUTPUT();  SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT(); SET_DATA_LINES_OUTPUT();
                               // ensure data bus outputs (delay) by repetitions according to processor speed

#define CLOCK_EN_HIGH()        O_SET_CONTROL_HIGH(O_CONTROL_EN);
                               // clock EN to high state - repreat to slow down (delay)

#define DELAY_ENABLE_CLOCK_HIGH() O_SET_CONTROL_LOW(O_CONTROL_EN);



// Graphic LCD
//
#if defined _WINDOWS
    #define MAX_GLCD_WRITE_BURST   10000                                 // the maximum number of writes to the GLCD before the task yields
#else
    #define MAX_GLCD_WRITE_BURST   1024                                  // the maximum number of writes to the GLCD before the task yields
#endif
#define DEL_USB_SYMBOL()                                                 // control display of USB enumeration - clear
#define SET_USB_SYMBOL()                                                 // control display of USB enumeration - set

#if defined FT800_GLCD_MODE
    #define ENABLE_BACKLIGHT()                                           // dummy

    // On SPI0
    //
  #if 1
    #define FLUSH_LCD_SPI_FIFO_AND_FLAGS()      SPI0_MCR |= (SPI_MCR_CLR_RXF); SPI0_SR = (SPI_SR_EOQF | SPI_SR_TFUF | SPI_SR_TFFF | SPI_SR_RFOF | SPI_SR_RFDF)

    #define CONFIGURE_GLCD()          POWER_UP_ATOMIC(6, SPI0);\
                                      _CONFIG_PERIPHERAL(D, 0, (PD_0_SPI0_PCS0 | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(D, 1, (PD_1_SPI0_SCK | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(D, 2, (PD_2_SPI0_SOUT | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(D, 3, PD_3_SPI0_SIN);\
                                      SPI0_MCR = (SPI_MCR_MSTR | SPI_MCR_DCONF_SPI | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF | SPI_MCR_PCSIS_CS0 | SPI_MCR_PCSIS_CS1 | SPI_MCR_PCSIS_CS2 | SPI_MCR_PCSIS_CS3 | SPI_MCR_PCSIS_CS4 | SPI_MCR_PCSIS_CS5);\
                                      SPI0_CTAR0 = (SPI_CTAR_DBR | SPI_CTAR_FMSZ_8 | SPI_CTAR_PDT_7 | SPI_CTAR_BR_4 | SPI_CTAR_ASC_0); // for 50MHz bus, 25MHz speed and 140ns min de-select time      
    #if !defined _WINDOWS
        #define _FT8XXEMU_cs(x)                                          // dummy
        #define _FT8XXEMU_transfer(x)                                    // dummy
    #endif
    #define WRITE_LCD_SPI_CMD0(byte)            SPI0_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); _FT8XXEMU_transfer(byte);  // write a single byte to the output FIFO - assert CS line
    #define WRITE_LCD_SPI_CMD0_FIRST(byte)      FLUSH_LCD_SPI_FIFO_AND_FLAGS(); _FT8XXEMU_cs(1); WRITE_LCD_SPI_CMD0(byte) // assert the chip select line and send a byte
    #define WRITE_LCD_SPI_CMD0_LAST(byte)       SPI0_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); _FT8XXEMU_transfer(byte); _FT8XXEMU_cs(1) // write final byte to output FIFO - this will negate the CS line when complete
    #define WAIT_LCD_SPI_RECEPTION_END()        while ((SPI0_SR & SPI_SR_RFDF) == 0) {}
    #define READ_LCD_SPI_FLUSH_DATA()           (unsigned char)SPI0_POPR
    #define CLEAR_LCD_SPI_RECEPTION_FLAG()      SPI0_SR |= SPI_SR_RFDF
    #if defined _WINDOWS
        #define READ_LCD_SPI_CMD0(byte, value)  SPI0_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); value = _FT8XXEMU_transfer(byte)
        #define READ_LCD_SPI_CMD0_LAST(byte, value) SPI0_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); value = _FT8XXEMU_transfer(byte); _FT8XXEMU_cs(1)
    #else
        #define READ_LCD_SPI_CMD0(byte, value)  SPI0_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); WAIT_LCD_SPI_RECEPTION_END(); value = READ_LCD_SPI_FLUSH_DATA(); CLEAR_LCD_SPI_RECEPTION_FLAG()
        #define READ_LCD_SPI_CMD0_LAST(byte, value) SPI0_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); WAIT_LCD_SPI_RECEPTION_END(); value = READ_LCD_SPI_FLUSH_DATA(); CLEAR_LCD_SPI_RECEPTION_FLAG()
    #endif
    #define FLUSH_LCD_SPI_RX(count)             {int i = count; while (i != 0) {WAIT_LCD_SPI_RECEPTION_END(); (void)SPI0_POPR; CLEAR_LCD_SPI_RECEPTION_FLAG(); i--;}}
    #if defined LCD_PWRDN_N
        #define GLCD_RST_H()                    _SETBITS(C, LCD_PWRDN_N);
    #endif
  #else                                                                  // on SPI1 - reference for KBED64
	#define LCD_SPI_MCR		SPI1_MCR
	#define LCD_SPI_SR		SPI1_SR
	#define LCD_SPI_POPR	SPI1_POPR

	#define FLUSH_LCD_SPI_FIFO() 	LCD_SPI_MCR |= (SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF);		//clear fifo and fifo-counter of rx/tx fifo
	#define FLUSH_LCD_SPI_FLAGS()	LCD_SPI_SR = (SPI_SR_TCF | SPI_SR_EOQF | SPI_SR_TFUF | SPI_SR_TFFF | SPI_SR_RFOF | SPI_SR_RFDF);
	#define FLUSH_LCD_SPI_FIFO_AND_FLAGS()	FLUSH_LCD_SPI_FIFO();FLUSH_LCD_SPI_FLAGS();

	#define CLEAR_LCD_SPI_RECEPTION_FLAG()      LCD_SPI_SR |= SPI_SR_RFDF

	#define WAIT_LCD_SPI_RECEPTION_END()  while (!(SPI1_SR & SPI_SR_RFDF)) {};
	#define READ_LCD_SPI_FLUSH_DATA()   (unsigned char)LCD_SPI_POPR
	#define FLUSH_LCD_SPI_DATA()        (void) LCD_SPI_POPR

	#define FT800_SPI_CTAR_CSSCK		0x000002000			//time cs enable ->sck
	#define FT800_SPI_CTAR_ASC			0x000000200			//time sck ->cs disable (time after sck)
	#define FT800_SPI_CTAR_DT			0x000000020			//time cs min. disabled (required 0)

    #define CONFIGURE_GLCD()          POWER_UP_ATOMIC(6, SPI1);\
                                      _CONFIG_PORT_OUTPUT(A, PORTA_BIT2,PORT_SRE_FAST);_CLEARBITS(A, PORTA_BIT2);\
                                      _CONFIG_PERIPHERAL(E, 4, (PE_4_SPI1_PCS0 | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(E, 2, (PE_2_SPI1_SCK | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(E, 1, (PE_1_SPI1_SOUT | PORT_SRE_FAST | PORT_DSE_HIGH));\
                                      _CONFIG_PERIPHERAL(E, 3, PE_3_SPI1_SIN);\
                                      SPI1_MCR = (SPI_MCR_MSTR | SPI_MCR_DCONF_SPI | SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF | SPI_MCR_PCSIS_CS0 | SPI_MCR_PCSIS_CS1 | SPI_MCR_PCSIS_CS2 | SPI_MCR_PCSIS_CS3 | SPI_MCR_PCSIS_CS4 | SPI_MCR_PCSIS_CS5);\
                                      SPI1_CTAR0 = (SPI_CTAR_FMSZ_8 | SPI_CTAR_PDT_7 | SPI_CTAR_BR_4 | FT800_SPI_CTAR_CSSCK | FT800_SPI_CTAR_ASC | FT800_SPI_CTAR_DT);
    #if !defined _WINDOWS
        #define _FT8XXEMU_cs(x)
        #define _FT8XXEMU_transfer(x)
    #endif
	#define WRITE_LCD_SPI_CMD0_FIRST(byte)      FLUSH_LCD_SPI_FIFO_AND_FLAGS(); _FT8XXEMU_cs(1); WRITE_LCD_SPI_CMD0(byte); // assert the chip select line and send a byte
    #define WRITE_LCD_SPI_CMD0(byte)            SPI1_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); _FT8XXEMU_transfer(byte); WAIT_LCD_SPI_RECEPTION_END(); FLUSH_LCD_SPI_DATA(); CLEAR_LCD_SPI_RECEPTION_FLAG();// write a single byte to the output FIFO - assert CS line
    #define WRITE_LCD_SPI_CMD0_LAST(byte)       SPI1_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); WAIT_LCD_SPI_RECEPTION_END(); FLUSH_LCD_SPI_DATA(); _FT8XXEMU_transfer(byte); _FT8XXEMU_cs(1); // write final byte to output FIFO - this will negate the CS line when complete

    #if defined _WINDOWS
        #define READ_LCD_SPI_CMD0(byte, value) SPI1_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); value = _FT8XXEMU_transfer(byte)
        #define READ_LCD_SPI_CMD0_LAST(byte, value) SPI1_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); value = _FT8XXEMU_transfer(byte); _FT8XXEMU_cs(1)
    #else
        #define READ_LCD_SPI_CMD0(byte, value)  SPI1_PUSHR = (byte | SPI_PUSHR_CONT | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); WAIT_LCD_SPI_RECEPTION_END(); value = READ_LCD_SPI_FLUSH_DATA(); CLEAR_LCD_SPI_RECEPTION_FLAG()
        #define READ_LCD_SPI_CMD0_LAST(byte, value) SPI1_PUSHR = (byte | SPI_PUSHR_EOQ | SPI_PUSHR_PCS0 | SPI_PUSHR_CTAS_CTAR0); WAIT_LCD_SPI_RECEPTION_END(); value = READ_LCD_SPI_FLUSH_DATA(); CLEAR_LCD_SPI_RECEPTION_FLAG()
    #endif
    #define FLUSH_LCD_SPI_RX(count)    //  (void)SPI1_POPR; CLEAR_LCD_SPI_RECEPTION_FLAG();     //{int i = count; while (i != 0) {WAIT_LCD_SPI_RECEPTION_END(); (void)SPI1_POPR; CLEAR_LCD_SPI_RECEPTION_FLAG(); i--;}}
    #define GLCD_RST_L()              _CLEARBITS(A, PORTA_BIT2);
    #define GLCD_RST_H()              _SETBITS(A, PORTA_BIT2);
  #endif
    #if defined _WINDOWS
        #define MAX_GLCD_WRITE_BURST   10000                             // the maximum number of writes to the GLCD before the task yields
    #else
        #define MAX_GLCD_WRITE_BURST   1000                              // the maximum number of writes to the GLCD before the task yields
    #endif

    #define _GLCD_BACKLIGHT_TIMER         6                              // PWM channel 6 (TC)
    #define _GLCD_TIMER_MODE_OF_OPERATION (PWM_PRESCALER_128 | PWM_SCALED_CLOCK_INPUT | PWM_POLARITY)
    #define _GLCD_BACKLIGHT_PWM_FREQUENCY (unsigned char)(PWM_US_DELAY(PWM_FREQUENCY_VALUE(2000 * 128)))
#endif
#if !defined GLCD_RST_H
    #define GLCD_RST_H()
#endif
#if !defined GLCD_RST_L
    #define GLCD_RST_L()
#endif

// Keypad
//
#if defined KEY_COLUMNS && (KEY_COLUMNS > 0)                             // matrix keypad
    #if defined NET_K60
        #define KEY_ROW_IN_1           PORTD_BIT7
        #define KEY_ROW_IN_PORT_1      GPIOD_PDIR
        #define KEY_ROW_IN_PORT_1_REF  _PORTD

        #define KEY_ROW_IN_2           PORTD_BIT8
        #define KEY_ROW_IN_PORT_2      GPIOD_PDIR
        #define KEY_ROW_IN_PORT_2_REF  _PORTD

        #define KEY_ROW_IN_3           PORTD_BIT9
        #define KEY_ROW_IN_PORT_3      GPIOD_PDIR
        #define KEY_ROW_IN_PORT_3_REF  _PORTD

        #define KEY_ROW_IN_4           PORTD_BIT10
        #define KEY_ROW_IN_PORT_4      GPIOD_PDIR
        #define KEY_ROW_IN_PORT_4_REF  _PORTD

        #define KEY_COL_OUT_1          PORTC_BIT18
        #define KEY_COL_OUT_PORT_1     (~GPIOC_PDOR)                     // invert since there is a hardware inversion (only used by simulator)
        #define KEY_COL_OUT_DDR_1      GPIOC_PDDR

        #define KEY_COL_OUT_2          PORTC_BIT19
        #define KEY_COL_OUT_PORT_2     (~GPIOC_PDOR)                     // invert since there is a hardware inversion (only used by simulator)
        #define KEY_COL_OUT_DDR_2      GPIOC_PDDR
 
        // Drive each column low (inverted in hardware)
        //
        #define DRIVE_COLUMN_1()       _SETBITS(C, KEY_COL_OUT_1)        // drive output CO1 high (column 1) - this will drive a '0' COL1
	    #define DRIVE_COLUMN_2()       _SETBITS(C, KEY_COL_OUT_2)        // drive output CO2 high (column 2) - this will drive a '0' COL2

        // Drive COL high (inverted in hardware)
	    //
        #define RELEASE_COLUMN_1()     _CLEARBITS(C, KEY_COL_OUT_1); fnDelayLoop(5)
        #define RELEASE_COLUMN_2()     _CLEARBITS(C, KEY_COL_OUT_2)
 
        // Key scan initialisation
        //
        #define INIT_KEY_SCAN()        _CONFIG_PORT_OUTPUT(C, (KEY_COL_OUT_1 | KEY_COL_OUT_2), (PORT_SRE_SLOW)); \
                                       _CONFIG_PORT_INPUT(D, (KEY_ROW_IN_1 | KEY_ROW_IN_2 | KEY_ROW_IN_3 | KEY_ROW_IN_4), (PORT_SRE_SLOW)); \
                                       RESET_SCAN()

        // Reset any changes ready for next scan sequence
        //
        #define RESET_SCAN()           _CLEARBITS(C, (KEY_COL_OUT_1 | KEY_COL_OUT_2)); // prepare outputs low for next time
    #else
        #define KEY_ROW_IN_1           PORTB_BIT20
        #define KEY_ROW_IN_PORT_1      GPIOB_PDIR
        #define KEY_ROW_IN_PORT_1_REF  _PORTB

        #define KEY_ROW_IN_2           PORTB_BIT21
        #define KEY_ROW_IN_PORT_2      GPIOB_PDIR
        #define KEY_ROW_IN_PORT_2_REF  _PORTB

        #define KEY_ROW_IN_3           PORTB_BIT22
        #define KEY_ROW_IN_PORT_3      GPIOB_PDIR
        #define KEY_ROW_IN_PORT_3_REF  _PORTB

        #define KEY_ROW_IN_4           PORTB_BIT23
        #define KEY_ROW_IN_PORT_4      GPIOB_PDIR
        #define KEY_ROW_IN_PORT_4_REF  _PORTB

        #define KEY_COL_OUT_1          PORTA_BIT24
        #define KEY_COL_OUT_PORT_1     GPIOA_PDOR
        #define KEY_COL_OUT_DDR_1      GPIOA_PDDR

        #define KEY_COL_OUT_2          PORTA_BIT25
        #define KEY_COL_OUT_PORT_2     GPIOA_PDOR
        #define KEY_COL_OUT_DDR_2      GPIOA_PDDR

        #define KEY_COL_OUT_3          PORTA_BIT26
        #define KEY_COL_OUT_PORT_3     GPIOA_PDOR
        #define KEY_COL_OUT_DDR_3      GPIOA_PDDR

        #define KEY_COL_OUT_4          PORTA_BIT27
        #define KEY_COL_OUT_PORT_4     GPIOA_PDOR
        #define KEY_COL_OUT_DDR_4      GPIOA_PDDR

        // Drive each column low
        //
        #define DRIVE_COLUMN_1()       _DRIVE_PORT_OUTPUT(A, KEY_COL_OUT_1) // drive output low (column 1) - this will drive a '0' since the output has been prepared
        #define DRIVE_COLUMN_2()       _DRIVE_PORT_OUTPUT(A, KEY_COL_OUT_2) // drive output low (column 2) - this will drive a '0' since the output has been prepared
        #define DRIVE_COLUMN_3()       _DRIVE_PORT_OUTPUT(A, KEY_COL_OUT_3) // drive output low (column 3) - this will drive a '0' since the output has been prepared
        #define DRIVE_COLUMN_4()       _DRIVE_PORT_OUTPUT(A, KEY_COL_OUT_4) // drive output low (column 4) - this will drive a '0' since the output has been prepared

        // Drive high (to avoid slow rise time) then set back as input
        //
        #define RELEASE_COLUMN_1()     _SETBITS(A, KEY_COL_OUT_1); _FLOAT_PORT(A, KEY_COL_OUT_1)
        #define RELEASE_COLUMN_2()     _SETBITS(A, KEY_COL_OUT_2); _FLOAT_PORT(A, KEY_COL_OUT_2)
        #define RELEASE_COLUMN_3()     _SETBITS(A, KEY_COL_OUT_3); _FLOAT_PORT(A, KEY_COL_OUT_3)
        #define RELEASE_COLUMN_4()     _SETBITS(A, KEY_COL_OUT_4); _FLOAT_PORT(A, KEY_COL_OUT_4)

        // Reset any changes ready for next scan sequence
        //
        #define RESET_SCAN()           _CLEARBITS(A, (KEY_COL_OUT_1 | KEY_COL_OUT_2 | KEY_COL_OUT_3 | KEY_COL_OUT_4)); // prepare outputs low for next time

        // Key scan initialisation
        //
        #define INIT_KEY_SCAN()        _CONFIG_PORT_INPUT(A, (KEY_COL_OUT_1 | KEY_COL_OUT_2 | KEY_COL_OUT_3 | KEY_COL_OUT_4), (PORT_PS_UP_ENABLE)); \
                                       _CONFIG_PORT_INPUT(B, (KEY_ROW_IN_1 | KEY_ROW_IN_2 | KEY_ROW_IN_3 | KEY_ROW_IN_4), (PORT_SRE_SLOW)); \
                                       RESET_SCAN()

        // LEDs
        //
                                        // '0'            '1'     input state  center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS {RGB(215,215,215), RGB(255,0,0), 0, {265, 20,  0,   7 },   _PORTA, DEMO_LED_1}, \
                                       {RGB(215,215,215), RGB(0,255,0), 0, {265, 88,  0,   7 },   _PORTD, USER_PORT_1_BIT}, \
                                       {RGB(215,215,215), RGB(0,0,255), 0, {265, 156, 0,   7 },   _PORTD, USER_PORT_2_BIT}, \
                                       {RGB(215,215,215), RGB(255,0,0), 0, {260, 216, 270, 226 }, _PORTD, USER_PORT_3_BIT}
    #endif

#elif defined KEY_COLUMNS && KEY_COLUMNS == 0                            // linear keyboard (key connected directly to inputs)
    #if !defined FRDM_KL82Z && (TSI_AVAILABLE > 0)
        #define TOUCH_SENSOR_INPUTS                                      // use touch sensor inputs
    #endif
    #if defined TOUCH_SENSOR_INPUTS
        #if defined TWR_K20D50M || defined TWR_K20D72M
            #define KEY_ROW_IN_1        PORTB_BIT0
            #define KEY_ROW_IN_2        PORTB_BIT1
            #define KEY_ROW_IN_3        0
            #define KEY_ROW_IN_4        0

            #define KEY_1_PORT_REF      _TOUCH_PORTB
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _TOUCH_PORTB
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _TOUCH_PORTB
            #define KEY_3_PORT_PIN      0

            #define KEY_4_PORT_REF      _TOUCH_PORTB
            #define KEY_4_PORT_PIN      0

            #define INIT_KEY_STATE      0x00000000

            #define READ_KEY_INPUTS()   fnReadTouchSensorInputs()

                                                                         // power up the touch sense input module; select TSI channels on the ports
                                                                         // enable TSI channels
            #define INIT_KEY_SCAN()     POWER_UP_ATOMIC(5, TSI); _CONFIG_PERIPHERAL(B, 0,  PB_0_TSI_CH0); _CONFIG_PERIPHERAL(B, 1,  PB_1_TSI_CH6); \
                                        TSI0_PEN = (TSI_PEN_PEN0 | TSI_PEN_PEN6); \
                                        TSI0_SCANC = (TSI_SCANC_SMOD_CONTINUOUS | TSI_SCANC_AMCLKS_BUS_CLK | TSI_SCANC_AMPSC_128); \
                                        TSI0_GENCS = (TSI_GENCS_STM_SW_TRIG | TSI_GENCS_SWTS | TSI_GENCS_TSIEN | TSI_GENCS_PS_32);
        #elif defined FRDM_KL26Z || defined FRDM_KL25Z || defined FRDM_KL03Z
            #define KEY_ROW_IN_1        PORTB_BIT16
            #define KEY_ROW_IN_2        PORTB_BIT17

            #define KEY_1_PORT_REF      _TOUCH_PORTB
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _TOUCH_PORTB
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _TOUCH_PORTB
            #define KEY_3_PORT_PIN      0

            #define INIT_KEY_STATE      0x00000000

            #define READ_KEY_INPUTS()   fnReadTouchSensorInputs()

                                                                         // power up the touch sense input module; select TSI channels on the ports and prepare operating mode; start first measurement
            #define INIT_KEY_SCAN()     POWER_UP_ATOMIC(5, TSI); _CONFIG_PERIPHERAL(B, 16,  PB_16_TSI_CH9); _CONFIG_PERIPHERAL(B, 17,  PB_17_TSI_CH10); \
                                        TSI0_GENCS = (TSI_GENCS_EOSF | TSI_GENCS_STM_SW | TSI_GENCS_NSCN_32 | TSI_GENCS_TSIEN | TSI_GENCS_PS_DIV_1 | TSI_GENCS_EXTCHRG_32uA | TSI_GENCS_DVOLT_1_03 | TSI_GENCS_REFCHRG_32uA | TSI_GENCS_MODE_CAPACITIVE); \
                                        TSI0_DATA = (TSI0_DATA_SWTS | TSI0_DATA_TSICH_9)
            #define FIRST_TSI_INPUT     TSI0_DATA_TSICH_9                // alternate between channels 9 and 10
            #define SECOND_TSI_INPUT    TSI0_DATA_TSICH_10
        #elif defined rcARM_KL26
            #define KEY_ROW_IN_1        PORTB_BIT16
            #define KEY_ROW_IN_2        PORTB_BIT17
            #define KEY_ROW_IN_3        PORTB_BIT18

            #define KEY_1_PORT_REF      _TOUCH_PORTB
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _TOUCH_PORTB
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _TOUCH_PORTB
            #define KEY_3_PORT_PIN      KEY_ROW_IN_3

            #define INIT_KEY_STATE      0x00000000

            #define READ_KEY_INPUTS()   fnReadTouchSensorInputs()

                                                                         // power up the touch sense input module; select TSI channels on the ports and prepare operating mode; start first measurement
            #define INIT_KEY_SCAN()     POWER_UP_ATOMIC(5, TSI); _CONFIG_PERIPHERAL(B, 16,  PB_16_TSI_CH9); _CONFIG_PERIPHERAL(B, 17,  PB_17_TSI_CH10);  _CONFIG_PERIPHERAL(B, 18,  PB_18_TSI_CH11); \
                                        TSI0_GENCS = (TSI_GENCS_EOSF | TSI_GENCS_STM_SW | TSI_GENCS_NSCN_32 | TSI_GENCS_TSIEN | TSI_GENCS_PS_DIV_1 | TSI_GENCS_EXTCHRG_32uA | TSI_GENCS_DVOLT_1_03 | TSI_GENCS_REFCHRG_32uA | TSI_GENCS_MODE_CAPACITIVE); \
                                        TSI0_DATA = (TSI0_DATA_SWTS | TSI0_DATA_TSICH_9)
            #define FIRST_TSI_INPUT     TSI0_DATA_TSICH_9                // alternate between channels 9, 10 and 11
            #define SECOND_TSI_INPUT    TSI0_DATA_TSICH_10
            #define THIRD_TSI_INPUT     TSI0_DATA_TSICH_11
        #else
            #define KEY_ROW_IN_1        PORTA_BIT4
            #define KEY_ROW_IN_2        PORTB_BIT3
            #define KEY_ROW_IN_3        PORTB_BIT2
            #define KEY_ROW_IN_4        PORTB_BIT16

            #define KEY_1_PORT_REF      _TOUCH_PORTA
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _TOUCH_PORTB
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _TOUCH_PORTB
            #define KEY_3_PORT_PIN      KEY_ROW_IN_3

            #define KEY_4_PORT_REF      _TOUCH_PORTB
            #define KEY_4_PORT_PIN      KEY_ROW_IN_4

            #define INIT_KEY_STATE      0x00000000

            #define READ_KEY_INPUTS()   fnReadTouchSensorInputs()

                                                                         // power up the touch sense input module; select TSI channels on the ports
                                                                         // enable TSI channels
            #define INIT_KEY_SCAN()     POWER_UP_ATOMIC(5, TSI); _CONFIG_PERIPHERAL(B, 3,  PB_3_TSI_CH8); _CONFIG_PERIPHERAL(A, 4,  PA_4_TSI_CH5); _CONFIG_PERIPHERAL(B, 2,  PB_2_TSI_CH7); _CONFIG_PERIPHERAL(B, 16,  PB_16_TSI_CH9); \
                                        TSI0_PEN = (TSI_PEN_PEN5 | TSI_PEN_PEN7 | TSI_PEN_PEN8 | TSI_PEN_PEN9); \
                                        TSI0_SCANC = (TSI_SCANC_SMOD_CONTINUOUS | TSI_SCANC_AMCLKS_BUS_CLK | TSI_SCANC_AMPSC_128); \
                                        TSI0_GENCS = (TSI_GENCS_STM_SW_TRIG | TSI_GENCS_SWTS | TSI_GENCS_TSIEN | TSI_GENCS_PS_32);
        #endif
    #else
        #if defined FRDM_KL82Z
            #define KEY_ROW_IN_1        PORTC_BIT0
            #define KEY_ROW_IN_2        PORTC_BIT3
            #define KEY_ROW_IN_3        PORTA_BIT14
            #define KEY_ROW_IN_4        PORTE_BIT7
            #define KEY_ROW_IN_5        PORTC_BIT5
            #define KEY_ROW_IN_6        PORTA_BIT4
            #define KEY_ROW_IN_7        PORTE_BIT11
            #define KEY_ROW_IN_8        PORTE_BIT8
            #define KEY_ROW_IN_9        PORTC_BIT10
            #define KEY_ROW_IN_10       PORTA_BIT2
            #define KEY_ROW_IN_11       PORTA_BIT15
            #define KEY_ROW_IN_12       PORTA_BIT1

            #define KEY_1_PORT_REF      _PORTC
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _PORTC
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _PORTA
            #define KEY_3_PORT_PIN      KEY_ROW_IN_3

            #define KEY_4_PORT_REF      _PORTE
            #define KEY_4_PORT_PIN      KEY_ROW_IN_4

            #define KEY_5_PORT_REF      _PORTC
            #define KEY_5_PORT_PIN      KEY_ROW_IN_5

            #define KEY_6_PORT_REF      _PORTA
            #define KEY_6_PORT_PIN      KEY_ROW_IN_6

            #define KEY_7_PORT_REF      _PORTE
            #define KEY_7_PORT_PIN      KEY_ROW_IN_7

            #define KEY_8_PORT_REF      _PORTE
            #define KEY_8_PORT_PIN      KEY_ROW_IN_8

            #define KEY_9_PORT_REF      _PORTC
            #define KEY_9_PORT_PIN      KEY_ROW_IN_9

            #define KEY_10_PORT_REF     _PORTA
            #define KEY_10_PORT_PIN     KEY_ROW_IN_10

            #define KEY_11_PORT_REF     _PORTA
            #define KEY_11_PORT_PIN     KEY_ROW_IN_11

            #define KEY_12_PORT_REF     _PORTA
            #define KEY_12_PORT_PIN     KEY_ROW_IN_12

            #define INIT_KEY_STATE      0xfffff000

            #define INIT_KEY_SCAN()     _CONFIG_PORT_INPUT(C, (KEY_ROW_IN_1 | KEY_ROW_IN_2 | KEY_ROW_IN_5 | KEY_ROW_IN_9), (PORT_PS_UP_ENABLE)); \
                                        _CONFIG_PORT_INPUT(A, (KEY_ROW_IN_3 | KEY_ROW_IN_6 | KEY_ROW_IN_10 | KEY_ROW_IN_11 | KEY_ROW_IN_12), (PORT_PS_UP_ENABLE)); \
                                        _CONFIG_PORT_INPUT(E, (KEY_ROW_IN_4 | KEY_ROW_IN_7 | KEY_ROW_IN_8), (PORT_PS_UP_ENABLE))

            #define READ_KEY_INPUTS()   ~((_READ_PORT_MASK(C, (KEY_ROW_IN_1))) | \
                                          (_READ_PORT_MASK(C, (KEY_ROW_IN_2))  >> (3  - 1))  | \
                                          (_READ_PORT_MASK(A, (KEY_ROW_IN_3))  >> (14 - 2))  | \
                                          (_READ_PORT_MASK(E, (KEY_ROW_IN_4))  >> (7  - 3))  | \
                                          (_READ_PORT_MASK(C, (KEY_ROW_IN_5))  >> (5  - 4))  | \
                                          (_READ_PORT_MASK(A, (KEY_ROW_IN_6))  << 1)  | \
                                          (_READ_PORT_MASK(E, (KEY_ROW_IN_7))  >> (11 - 6))  | \
                                          (_READ_PORT_MASK(E, (KEY_ROW_IN_8))  >> (8  - 7))  | \
                                          (_READ_PORT_MASK(C, (KEY_ROW_IN_9))  >> (10 - 8))  | \
                                          (_READ_PORT_MASK(A, (KEY_ROW_IN_10)) << 7)  | \
                                          (_READ_PORT_MASK(A, (KEY_ROW_IN_11)) >> (15 - 10))  | \
                                          (_READ_PORT_MASK(A, (KEY_ROW_IN_12)) << 10))
        #else
            #define KEY_ROW_IN_1        PORTB_BIT20
            #define KEY_ROW_IN_2        PORTB_BIT21
            #define KEY_ROW_IN_3        PORTB_BIT22
            #define KEY_ROW_IN_4        PORTB_BIT23

            #define KEY_1_PORT_REF      _PORTB
            #define KEY_1_PORT_PIN      KEY_ROW_IN_1

            #define KEY_2_PORT_REF      _PORTB
            #define KEY_2_PORT_PIN      KEY_ROW_IN_2

            #define KEY_3_PORT_REF      _PORTB
            #define KEY_3_PORT_PIN      KEY_ROW_IN_3

            #define KEY_4_PORT_REF      _PORTB
            #define KEY_4_PORT_PIN      KEY_ROW_IN_4

            #define INIT_KEY_STATE      0x0000000f

            #define READ_KEY_INPUTS()   (_READ_PORT_MASK(B, (KEY_ROW_IN_1 | KEY_ROW_IN_2 | KEY_ROW_IN_3 | KEY_ROW_IN_4)) >> 20)

            #define INIT_KEY_SCAN()     _CONFIG_PORT_INPUT(B, (KEY_ROW_IN_1 | KEY_ROW_IN_2 | KEY_ROW_IN_3 | KEY_ROW_IN_4), (PORT_PS_UP_ENABLE));
        #endif
    #endif

    // LEDs
    //
    #if !defined TWR_K20D50M && !defined TWR_K20D72M && !defined FRDM_K64F && !defined FRDM_KL27Z && !defined FRDM_KL28Z && !defined FRDM_KL26Z && !defined FRDM_KL25Z && !defined FRDM_KL03Z && !defined FreeLON && !defined FRDM_KL82Z
                                        // '0'            '1'     input state   center (x,   y)   0 = circle, radius, controlling port, controlling pin 
        #define KEYPAD_LED_DEFINITIONS  {RGB(200,200,200),RGB(255,75,0),  0, {340, 123, 348, 131 }, _PORTA, DEMO_LED_1}, \
                                        {RGB(200,200,200),RGB(255,255,0), 0, {340, 160, 348, 168 }, _PORTA, DEMO_LED_2}, \
                                        {RGB(200,200,200),RGB(0,255,0),   0, {340, 195, 348, 203 }, _PORTA, DEMO_LED_3}, \
                                        {RGB(200,200,200),RGB(255,0,0),   0, {340, 231, 348, 239 }, _PORTA, DEMO_LED_4}


                                        // column, row, key press rectangle
        #define KEYPAD_KEY_DEFINITIONS  {0, 0, {317, 114, 372, 140}}, \
                                        {0, 1, {317, 152, 372, 175}}, \
                                        {0, 2, {317, 186, 372, 212}}, \
                                        {0, 3, {317, 222, 372, 249}} 
    #endif
#endif

//#define BUZZER_SOUND                                                   // enable buzzer sound generation in the simulator (needs Winmm.lib added to the simulator linker setting)

#define SENDERS_EMAIL_ADDRESS       "iMX_RT@uTasker.com"                 // fictional Email address of the board being used
#define EMAIL_SUBJECT               "iMX_RT Test"                        // email subject
#define EMAIL_CONTENT               "Hello!!\r\nThis is an email message from the iMX_RT demo.\r\nI hope that you have received this test and have fun using the uTasker operating system with integrated TCP/IP stack.\r\r\nRegards your iMX_RT demo!!";

// SMTP account details if not in parameters
//
#define SMTP_ACCOUNT_NAME           "User Name";
#define SMTP_PASSWORD               "Password";

// Internal HTML message pages
//
#define SUPPORT_INTERNAL_HTML_FILES                                      // enable the use of the following files
#define UPLOAD_FAILED         "<html><head><title>SW Upload failed</title></head><body bgcolor=#ff9000 text=#000000 topmargin=3 marginheight=3><center><td valign=top class=h><font color=#ff0000 style=font-size:30px><b style='mso-bidi-font-weight:normal'>&micro;Tasker</font></b></td><br></td><td align=left><br><br>Sorry but upload has failed.</font><br><br><a href=""javascript:history.back()"">Return</a></body></html>"
#define SW_UPLOAD_COMPLETED   "<html><head><meta http-equiv=""refresh"" content=""5;URL=0Menu.htm""><title>KINETIS SW Uploaded</title></head><body bgcolor=#ffffff text=#000000 topmargin=3 marginheight=3><center><td valign=top class=h><font color=#ff0000 style=font-size:30px><b style='mso-bidi-font-weight:normal'>&micro;Tasker</font> - SW Update</b></td><br></td><td align=left><br><br>SW Upload successful. The KINETIS target will now reset and start the new program. Please wait 5s...</body></html>"


#if defined USE_TFTP
    #define TFTP_ERROR_MESSAGE     "uTasker TFTP Error"                  // fixed TFTP error test
#endif

#if defined USE_IP || defined USB_TO_ETHERNET
    #define LAN_BUFFER_SIZE        1514                                  // i.MX has ample space for full IP payload (if VLAN is enabled the size is automatically adjusted to allow tags)
#else
    #define LAN_BUFFER_SIZE        256                                   // if using Ethernet without IP the buffer size can be set here
#endif

#if defined KBED_FPGA
    #define FPGA_ADDR              0x61000000

    #define CONFIGURE_FPGA()      _CONFIG_PERIPHERAL(C, 8,  (PC_8_FB_AD7   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(C, 9,  (PC_9_FB_AD6   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(C, 10, (PC_10_FB_AD5  | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 2,  (PD_2_FB_AD4   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 3,  (PD_3_FB_AD3   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 4,  (PD_4_FB_AD2   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 5,  (PD_5_FB_AD1   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 6,  (PD_6_FB_AD0   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D, 1,  (PD_1_FB_CS0   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(C, 11, (PC_11_FB_RW   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(D,  0, (PD_0_FB_ALE   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(B, 19, (PB_19_FB_OE   | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        _CONFIG_PERIPHERAL(C,  3, (PC_3_FB_CLKO  | PORT_DSE_HIGH | PORT_PS_DOWN_ENABLE)); \
        CSAR0  = FPGA_ADDR; \
        CSMR0  = (0x00010000 | FB_CS_VALID); \
        CSCR0  = (FB_BLS | FB_AA | PORT_SIZE_8); 

    #define fnFPGA_IO_0_7(data)            *((volatile unsigned char*)FPGA_ADDR + 0x00) = (data)
    #define fnFPGA_IO_8_15(data)           *((volatile unsigned char*)FPGA_ADDR + 0x01) = (data)
    #define fnFPGA_IO_16_23(data)          *((volatile unsigned char*)FPGA_ADDR + 0x02) = (data)
    #define fnFPGA_IO_24_31(data)          *((volatile unsigned char*)FPGA_ADDR + 0x03) = (data)

    #define fnFPGA_IODC_0_7(data)          *((volatile unsigned char*)FPGA_ADDR + 0x80) = (data)
    #define fnFPGA_IODC_8_15(data)         *((volatile unsigned char*)FPGA_ADDR + 0x81) = (data)
    #define fnFPGA_IODC_16_23(data)        *((volatile unsigned char*)FPGA_ADDR + 0x82) = (data)
    #define fnFPGA_IODC_24_31(data)        *((volatile unsigned char*)FPGA_ADDR + 0x83) = (data)

    #define fnFPGA_IOFC_0_7(data)          *((volatile unsigned char*)FPGA_ADDR + 0x84) = (data)
    #define fnFPGA_IOFC_8_15(data)         *((volatile unsigned char*)FPGA_ADDR + 0x85) = (data)
    #define fnFPGA_IOFC_16_23(data)        *((volatile unsigned char*)FPGA_ADDR + 0x86) = (data)
    #define fnFPGA_IOFC_24_31(data)        *((volatile unsigned char*)FPGA_ADDR + 0x87) = (data)

    #define fnFPGA_PWM0_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x10) = (data)
    #define fnFPGA_PWM1_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x12) = (data)
    #define fnFPGA_PWM2_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x14) = (data)
    #define fnFPGA_PWM3_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x16) = (data)
    #define fnFPGA_PWM4_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x18) = (data)
    #define fnFPGA_PWM5_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x1a) = (data)
    #define fnFPGA_PWM6_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x1c) = (data)
    #define fnFPGA_PWM7_L(data)            *((volatile unsigned char*)FPGA_ADDR + 0x1e) = (data)

    #define fnFPGA_SETBITS_16_23(data)     *((volatile unsigned char*)FPGA_ADDR + 0x06) = (data)
    #define fnFPGA_CLEARBITS_16_23(data)   *((volatile unsigned char*)FPGA_ADDR + 0x0a) = (data)
    #define fnFPGA_TOGGLEBITS_16_23(data)  *((volatile unsigned char*)FPGA_ADDR + 0x0e) = (data)

    #define fnFPGA_ID_INDEX(data)          *((volatile unsigned char*)FPGA_ADDR + 0xff) = (data)
    #define fnFPGA_RD_ID()        	       *((volatile unsigned char*)FPGA_ADDR + 0xff)

    #define fnFPGA_RD(addr)                *((volatile unsigned char*)FPGA_ADDR + addr)
#endif

#if defined BLINKY || defined HELLO_WORLD                                // remove peripheral support when BLINKY/HELLO_WORLD is used
    #undef SUPPORT_PITS
    #undef SUPPORT_KEYBOARD_INTERRUPTS
    #undef SUPPORT_RTC
    #undef SUPPORT_PORT_INTERRUPTS
    #undef SUPPORT_ADC
    #undef SUPPORT_LLWU
#endif


#if defined _DCD_TABLE
    #if defined MIMXRT1050 && defined SUPPORT_GLCD

#define GROUP1_ENTRIES       117
#define GROUP2_ENTRIES       2
#define GROUP3_ENTRIES       2
#define GROUP4_ENTRIES       3
#define GROUP5_ENTRIES       1

typedef struct _PACK stDCD_TABLE {
    DCD_HEADER    dcd_header;
    unsigned char dcd_command1[GROUP_SIZE(GROUP1_ENTRIES)];
    unsigned char dcd_wait1[DCD_WAIT_SIZE];
    unsigned char dcd_command2[GROUP_SIZE(GROUP2_ENTRIES)];
    unsigned char dcd_wait2[DCD_WAIT_SIZE];
    unsigned char dcd_command3[GROUP_SIZE(GROUP3_ENTRIES)];
    unsigned char dcd_wait3[DCD_WAIT_SIZE];
    unsigned char dcd_command4[GROUP_SIZE(GROUP4_ENTRIES)];
    unsigned char dcd_wait4[DCD_WAIT_SIZE];
    unsigned char dcd_command5[GROUP_SIZE(GROUP5_ENTRIES)];
} DCD_TABLE;


#if defined _COMPILE_IAR
__root const DCD_TABLE __dcd_data @ ".boot_hdr.dcd_data" // __root forces the function to be linked in IAR project
#elif defined _GNU
static const DCD_TABLE __attribute__((section(".boot_hdr.dcd_data"))) __dcd_data
#elif defined _COMPILE_KEIL
__attribute__((section(".boot_hdr.dcd_data"))) const DCD_TABLE __dcd_data
#else
static const DCD_TABLE __dcd_data                                        // IS42S16160J-6BLI SDRAM configuration
#endif
= {
    { DCD_TAG, {BIG_SHORT_WORD_BYTES(sizeof(DCD_TABLE))}, DCD_VERSION }, // header
    {   // Command group 1
        //
        _DCD_WRITE_LONG_WORD_GROUP(GROUP1_ENTRIES),
        _DCD_WRITE_LONG_WORD(CCM_CCGR0, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR1, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR2, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR3, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR4, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR5, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_CCGR6, (0xffffffff)),
        _DCD_WRITE_LONG_WORD(CCM_ANALOG_PLL_SYS, (CCM_ANALOG_PLL_SYS_ENABLE | CCM_ANALOG_PLL_SYS_DIV_SELECT)), // enable the system PLL (PLL2)
        _DCD_WRITE_LONG_WORD(CCM_ANALOG_PFD_528, (29 << 16)), // system PLL PFD2 fractional mask for 327.7241379MHz
      //_DCD_WRITE_LONG_WORD(CCM_ANALOG_PFD_528, (35 << 16)), // system PLL PFD2 fractional mask for 271.5428571MHz
        _DCD_WRITE_LONG_WORD(CCM_CBCDR, (CCM_CBCDR_SEMC_PODF_DIV2 | CCM_CBCDR_AHB_PODF_DIV4 | CCM_CBCDR_IPG_PODF_DIV2 | CCM_CBCDR_SEMC_CLK_SEL_SEMC | CCM_CBCDR_SEMC_ALT_CLK_SEL_PLL2_PFD2)), // select SEMC_CLK_ROOT from PLL2-PFD2 divided by 2 to give 163.86MHz (max. 166MHz)
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00, GPIO_EMC_00_SEMC_DATA00),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01, GPIO_EMC_01_SEMC_DATA01),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02, GPIO_EMC_02_SEMC_DATA02),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03, GPIO_EMC_03_SEMC_DATA03),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04, GPIO_EMC_04_SEMC_DATA04),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05, GPIO_EMC_05_SEMC_DATA05),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06, GPIO_EMC_06_SEMC_DATA06),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07, GPIO_EMC_07_SEMC_DATA07),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08, GPIO_EMC_08_SEMC_DM00),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09, GPIO_EMC_09_SEMC_ADDR00),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10, GPIO_EMC_10_SEMC_ADDR01),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11, GPIO_EMC_11_SEMC_ADDR02),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12, GPIO_EMC_12_SEMC_ADDR03),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13, GPIO_EMC_13_SEMC_ADDR04),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14, GPIO_EMC_14_SEMC_ADDR05),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15, GPIO_EMC_15_SEMC_ADDR06),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16, GPIO_EMC_16_SEMC_ADDR07),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17, GPIO_EMC_17_SEMC_ADDR08),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18, GPIO_EMC_18_SEMC_ADDR09),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19, GPIO_EMC_19_SEMC_ADDR11),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20, GPIO_EMC_20_SEMC_ADDR12),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21, GPIO_EMC_21_SEMC_BA0),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22, GPIO_EMC_22_SEMC_BA1),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23, GPIO_EMC_23_SEMC_ADDR10),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24, GPIO_EMC_24_SEMC_CAS),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25, GPIO_EMC_25_SEMC_RAS),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26, GPIO_EMC_26_SEMC_CLK),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27, GPIO_EMC_27_SEMC_CKE),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28, GPIO_EMC_28_SEMC_WE),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29, GPIO_EMC_29_SEMC_CS0),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30, GPIO_EMC_30_SEMC_DATA08),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31, GPIO_EMC_31_SEMC_DATA09),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32, GPIO_EMC_32_SEMC_DATA10),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33, GPIO_EMC_33_SEMC_DATA11),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34, GPIO_EMC_34_SEMC_DATA12),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35, GPIO_EMC_35_SEMC_DATA13),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36, GPIO_EMC_36_SEMC_DATA14),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37, GPIO_EMC_37_SEMC_DATA15),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38, GPIO_EMC_38_SEMC_DM01),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39, IOMUXC_SW_MUX_CTL_PAD_SION),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40, GPIO_EMC_40_SEMC_RDY),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41, GPIO_EMC_41_SEMC_CSX0),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_01, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_02, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_03, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_04, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_05, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_06, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_08, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_09, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_10, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_11, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_12, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_13, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_14, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_15, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_16, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_17, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_18, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_19, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_20, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_21, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_22, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_23, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_25, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_26, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_27, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_28, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_29, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_30, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_31, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_33, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_34, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_35, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_36, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_37, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_38, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_39, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_41, (IOMUXC_SW_PAD_CTL_PAD_HYS | IOMUXC_SW_PAD_CTL_PAD_PKE | IOMUXC_SW_PAD_CTL_PAD_SPEED_MAX | IOMUXC_SW_PAD_CTL_PAD_DSE_7 | IOMUXC_SW_PAD_CTL_PAD_SRE)),
        _DCD_WRITE_LONG_WORD(SEMC_MCR, SEMC_MCR_DQSMD),
        _DCD_WRITE_LONG_WORD(SEMC_BMCR0, 0x00030524),
        _DCD_WRITE_LONG_WORD(SEMC_BMCR1, 0x06030524),
        _DCD_WRITE_LONG_WORD(SEMC_BR0, 0x8000001b),
        _DCD_WRITE_LONG_WORD(SEMC_BR1, 0x8200001b),
        _DCD_WRITE_LONG_WORD(SEMC_BR2, 0x8400001b),
        _DCD_WRITE_LONG_WORD(SEMC_BR3, 0x8600001b),
        _DCD_WRITE_LONG_WORD(SEMC_BR4, 0x90000021),
        _DCD_WRITE_LONG_WORD(SEMC_BR5, 0xa0000019),
        _DCD_WRITE_LONG_WORD(SEMC_BR6, 0xa8000017),
        _DCD_WRITE_LONG_WORD(SEMC_BR7, 0xa900001b),
        _DCD_WRITE_LONG_WORD(SEMC_BR8, 0x00000021),
        _DCD_WRITE_LONG_WORD(SEMC_IOCR, 0x000079a8),
        _DCD_WRITE_LONG_WORD(SEMC_SDRAMCR0, 0x00000f31),
        _DCD_WRITE_LONG_WORD(SEMC_SDRAMCR1, 0x00652922),
        _DCD_WRITE_LONG_WORD(SEMC_SDRAMCR2, 0x00010920),
        _DCD_WRITE_LONG_WORD(SEMC_SDRAMCR3, 0x50210a08),
        _DCD_WRITE_LONG_WORD(SEMC_DBICR0, 0x00000021),
        _DCD_WRITE_LONG_WORD(SEMC_DBICR1, 0x00888888),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR1, 0x00000002),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR2, 0x00000000),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR0, 0x80000000),
        _DCD_WRITE_LONG_WORD(SEMC_IPCMD, (SEMC_IPCMD_KEY | SEMC_IPCMD_SDRAM_PRECHARGE_ALL)),
    },
    {   // Wait 1
        //
        _BCD_WAIT_SET(SEMC_INTR, SEMC_INTR_IPCMDDONE),
    },
    {   // Command group 2
        //
        _DCD_WRITE_LONG_WORD_GROUP(GROUP2_ENTRIES),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR0, (0x80000000)),
        _DCD_WRITE_LONG_WORD(SEMC_IPCMD, (SEMC_IPCMD_KEY | SEMC_IPCMD_SDRAM_AUTO_REFRESH)),
    },
    {   // Wait 2
        //
        _BCD_WAIT_SET(SEMC_INTR, SEMC_INTR_IPCMDDONE),
    },
    {   // Command group 3
        //
        _DCD_WRITE_LONG_WORD_GROUP(GROUP3_ENTRIES),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR0, (0x80000000)),
        _DCD_WRITE_LONG_WORD(SEMC_IPCMD, (SEMC_IPCMD_KEY | SEMC_IPCMD_SDRAM_AUTO_REFRESH)),
    },
    {   // Wait 3
        //
        _BCD_WAIT_SET(SEMC_INTR, SEMC_INTR_IPCMDDONE),
    },
    {   // Command group 4
        //
        _DCD_WRITE_LONG_WORD_GROUP(GROUP4_ENTRIES),
        _DCD_WRITE_LONG_WORD(SEMC_IPTXDAT, (0x00000033)),
        _DCD_WRITE_LONG_WORD(SEMC_IPCR0, (0x80000000)),
        _DCD_WRITE_LONG_WORD(SEMC_IPCMD, (SEMC_IPCMD_KEY | SEMC_IPCMD_SDRAM_MODESET)),
    },
    {   // Wait 4
        //
        _BCD_WAIT_SET(SEMC_INTR, SEMC_INTR_IPCMDDONE),
    },
    {   // Command group 5
        //
        _DCD_WRITE_LONG_WORD_GROUP(GROUP5_ENTRIES),
        _DCD_WRITE_LONG_WORD(SEMC_SDRAMCR3, (0x50210a08 | SEMC_SDRAMCR3_REN)),
    },
};
    #endif
#endif
#endif                                                                   // end of file
