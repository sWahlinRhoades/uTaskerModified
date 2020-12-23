/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:   	    STR91XF.h
   Project: 	Single Chip Embedded Internet
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2019
   *********************************************************************/

#if defined _WINDOWS
    #include "simSTR91XF.h"
    #define _SIM_PORT_CHANGE   fnSimPorts(-1);
#else
    #ifdef COMPILE_IAR
        #ifdef COMPILE_IAR_4_4
            #include <intrinsics.h>                                      // include IAR intrinsic (disable/enable interrupt) V4.4
        #else
            #include <intrinsic.h>                                       // include IAR intrinsic (disable/enable interrupt) V4.2
        #endif
    #endif
    #define _SIM_PORT_CHANGE
#endif
#ifndef __CONFIG__
    #include "config.h"
#endif

#define _MALLOC_ALIGN                                                    // support malloc with align option since LAN memory should be on specific boundary
#define _ALIGN_HEAP_4                                                    // all heap blocks are aligned
#define SDCARD_MALLOC(x) uMalloc(x)                                      // use standard uMalloc()

#define PLL_OUTPUT_FREQ  (((2 * CRYSTAL_FREQ)/(OSC_DIVIDE*(0x01 << POST_DIVIDE_EXP))) * PLL_MUL)
#define MASTER_CLOCK   (PLL_OUTPUT_FREQ)                                 // take PLL frequency from project specific "app_hw_str91xf.h"
#define RCLK            MASTER_CLOCK
#define HCLK            MASTER_CLOCK
#define BRCLK           MASTER_CLOCK/2
#define FMICLK          RCLK
#define BCLK            HCLK
#define USBCLK          MASTER_CLOCK
#define MII_PHYCLK      CRYSTAL_FREQ

#define CAST_POINTER_ARITHMETIC unsigned long                            // STR91XF uses 32 bit pointers
#define _LITTLE_ENDIAN                                                   // STR91XF always works in little endian mode
#define start_application(call_address)   ((void (*)(void))call_address)(); // call specified address

/*****************************************************************************************************/


#ifdef _WINDOWS
  #define FMI_BLOCK                     ((unsigned char *)(&ucSTR91XF.ucFMI))      // Flash Memory Interface

  #define WIU_BLOCK                     ((unsigned char *)(&ucSTR91XF.ucWIU))      // Wake-up / Interrupt Unit
  #define TIM0_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucTimer0))   // Timer 0
  #define TIM1_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucTimer1))   // Timer 1
  #define TIM2_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucTimer2))   // Timer 2
  #define TIM3_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucTimer3))   // Timer 3

  #define GPIO_PORT0_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[0])) // GPIO 0 PORT
  #define GPIO_PORT1_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[1])) // GPIO 1 PORT
  #define GPIO_PORT2_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[2])) // GPIO 2 PORT
  #define GPIO_PORT3_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[3])) // GPIO 3 PORT
  #define GPIO_PORT4_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[4])) // GPIO 4 PORT
  #define GPIO_PORT5_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[5])) // GPIO 5 PORT
  #define GPIO_PORT6_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[6])) // GPIO 6 PORT
  #define GPIO_PORT7_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[7])) // GPIO 7 PORT
  #define GPIO_PORT8_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[8])) // GPIO 8 PORT
  #define GPIO_PORT9_BLOCK              ((unsigned char *)(&ucSTR91XF.ucPorts[9])) // GPIO 9 PORT

  #define SCU_BLOCK                     ((unsigned char *)(&ucSTR91XF.ucSCU))      // System Control Unit

  #define UART0_BLOCK                   ((unsigned char *)(&ucSTR91XF.ucUart[0]))  // UART0
  #define UART1_BLOCK                   ((unsigned char *)(&ucSTR91XF.ucUart[1]))  // UART1
  #define UART2_BLOCK                   ((unsigned char *)(&ucSTR91XF.ucUart[2]))  // UART2
  #define SSP0_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucSSP[0]))   // Synchronous Serial Peripheral 0
  #define SSP1_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucSSP[1]))   // Synchronous Serial Peripheral 1

  #define ADC_BLOCK                     ((unsigned char *)(&ucSTR91XF.ucADC))      // Analog/Digital Converter
  #define WDT_BLOCK                     ((unsigned char *)(&ucSTR91XF.ucWDT))      // Watchdog Timer
  #define I2C0_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucIIC[0]))   // IIC0
  #define I2C1_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucIIC[1]))   // IIC1

  #define ENET_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucENET))     // Ethernet Controller
  #define VIC1_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucVIC1))     // Vectored Interrupt Controller 1
  #define VIC0_BLOCK                    ((unsigned char *)(&ucSTR91XF.ucVIC0))     // Vectored Interrupt Controller 0
#else
  #ifdef BUFFERED_PERIPHERALS
      #define BUFFERED_OFFSET           0
  #else
      #define BUFFERED_OFFSET           0x10000000                       // NON-buffered accesses
  #endif
  #define FMI_BLOCK                     (0x44000000 + BUFFERED_OFFSET)   // Flash Memory Interface
  #define APB0_BLOCK                    (0x48000000 + BUFFERED_OFFSET)   // AHB/APB0 Bridge
  #define WIU_BLOCK                     (0x48001000 + BUFFERED_OFFSET)   // Wake-up / Interrupt Unit
  #define TIM0_BLOCK                    (0x48002000 + BUFFERED_OFFSET)   // Timer 0
  #define TIM1_BLOCK                    (0x48003000 + BUFFERED_OFFSET)   // Timer 1
  #define TIM2_BLOCK                    (0x48004000 + BUFFERED_OFFSET)   // Timer 2
  #define TIM3_BLOCK                    (0x48005000 + BUFFERED_OFFSET)   // Timer 3
  #define GPIO_PORT0_BLOCK              (0x48006000 + BUFFERED_OFFSET)   // GPIO 0 PORT
  #define GPIO_PORT1_BLOCK              (0x48007000 + BUFFERED_OFFSET)   // GPIO 1 PORT
  #define GPIO_PORT2_BLOCK              (0x48008000 + BUFFERED_OFFSET)   // GPIO 2 PORT
  #define GPIO_PORT3_BLOCK              (0x48009000 + BUFFERED_OFFSET)   // GPIO 3 PORT
  #define GPIO_PORT4_BLOCK              (0x4800a000 + BUFFERED_OFFSET)   // GPIO 4 PORT
  #define GPIO_PORT5_BLOCK              (0x4800b000 + BUFFERED_OFFSET)   // GPIO 5 PORT
  #define GPIO_PORT6_BLOCK              (0x4800c000 + BUFFERED_OFFSET)   // GPIO 6 PORT
  #define GPIO_PORT7_BLOCK              (0x4800d000 + BUFFERED_OFFSET)   // GPIO 7 PORT
  #define GPIO_PORT8_BLOCK              (0x4800e000 + BUFFERED_OFFSET)   // GPIO 8 PORT
  #define GPIO_PORT9_BLOCK              (0x4800f000 + BUFFERED_OFFSET)   // GPIO 9 PORT
  #define APB1_BLOCK                    (0x4c000000 + BUFFERED_OFFSET)   // AHB/APB1 Bridge
  #define RTC_BLOCK                     (0x4c001000 + BUFFERED_OFFSET)   // Real Time Clock
  #define SCU_BLOCK                     (0x4c002000 + BUFFERED_OFFSET)   // System Control Unit
  #define MC_BLOCK                      (0x4c003000 + BUFFERED_OFFSET)   // Induction Motor Control
  #define UART0_BLOCK                   (0x4c004000 + BUFFERED_OFFSET)   // UART0
  #define UART1_BLOCK                   (0x4c005000 + BUFFERED_OFFSET)   // UART1
  #define UART2_BLOCK                   (0x4c006000 + BUFFERED_OFFSET)   // UART2
  #define SSP0_BLOCK                    (0x4c007000 + BUFFERED_OFFSET)   // Synchronous Serial Peripheral 0
  #define SSP1_BLOCK                    (0x4c008000 + BUFFERED_OFFSET)   // Synchronous Serial Peripheral 1
  #define CAN_BLOCK                     (0x4c009000 + BUFFERED_OFFSET)   // Controller Area Network
  #define ADC_BLOCK                     (0x4c00a000 + BUFFERED_OFFSET)   // Analog/Digital Converter
  #define WDT_BLOCK                     (0x4c00b000 + BUFFERED_OFFSET)   // Watchdog Timer
  #define I2C0_BLOCK                    (0x4c00c000 + BUFFERED_OFFSET)   // IIC 0
  #define I2C1_BLOCK                    (0x4c00d000 + BUFFERED_OFFSET)   // IIC 1
  #define USB_BLOCK                     (0x60000000 + BUFFERED_OFFSET)   // Universal Serial Bus
  #define EMI_BLOCK                     (0x64000000 + BUFFERED_OFFSET)   // External Memory Interface
  #define DMAC_BLOCK                    (0x68000000 + BUFFERED_OFFSET)   // 8 channel DMA Controller
  #define ENET_BLOCK                    (0x6c000000 + BUFFERED_OFFSET)   // Ethernet Controller
  #define VIC1_BLOCK                    (0xfc000000)                     // Vectored Interrupt Controller 1
  #define VIC0_BLOCK                    (0xfffff000)                     // Vectored Interrupt Controller 0
#endif

// Flash Memory Interface
//
#define FMI_BBSR                        *(unsigned long *)(FMI_BLOCK + 0x00)  // Boot Bank Size Register
#define FMI_NBBSR                       *(unsigned long *)(FMI_BLOCK + 0x04)  // Non-Boot Bank Size Register

#define FMI_BBADR                       *(unsigned long *)(FMI_BLOCK + 0x0c)  // Boot Bank Base Address Register
#define FMI_NBBADR                      *(unsigned long *)(FMI_BLOCK + 0x10)  // Non-Boot Bank Base Address Register

#define FMI_CR                          *(unsigned long *)(FMI_BLOCK + 0x18)  // FMI Control Register
  #define FMI_B0EN                      0x00000008                            // Enable FLASH Bank 0
  #define FMI_B1EN                      0x00000010                            // Enable FLASH Bank 1
  #define FMI_OMIE                      0x00000080                            // Out of Memory Interrupt Enable
#define FMI_SR                          *(unsigned long *)(FMI_BLOCK + 0x1c)  // FMI Status Register
#define FMI_BCE5ADDR                    *(unsigned long *)(FMI_BLOCK + 0x20)  // Branch Carry 5th Entry Target Address Register


// FLASH memory address defines
//
#define STR91XF_BANK0_SECTOR_0          OFFSET_BANK0
#define STR91XF_BANK0_SECTOR_1          (OFFSET_BANK0 + (1*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_2          (OFFSET_BANK0 + (2*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_3          (OFFSET_BANK0 + (3*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_4          (OFFSET_BANK0 + (4*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_5          (OFFSET_BANK0 + (5*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_6          (OFFSET_BANK0 + (6*FLASH_GRANULARITY_BANK0))
#define STR91XF_BANK0_SECTOR_7          (OFFSET_BANK0 + (7*FLASH_GRANULARITY_BANK0))
#define BANK0_SECTORS                   8

#define STR91XF_BANK1_SECTOR_0          OFFSET_BANK1
#define STR91XF_BANK1_SECTOR_1          (OFFSET_BANK1 + (1*FLASH_GRANULARITY_BANK1))
#define STR91XF_BANK1_SECTOR_2          (OFFSET_BANK1 + (2*FLASH_GRANULARITY_BANK1))
#define STR91XF_BANK1_SECTOR_3          (OFFSET_BANK1 + (3*FLASH_GRANULARITY_BANK1))
#define BANK1_SECTORS                   4

#define PROTECTED_FILE                                                   // Flash is protected out of reset


#define FLASH_GRANULARITY_BANK0          (64*1024)                                        // smallest sector which can be erased independently
#define FLASH_GRANULARITY_BANK1          (8*1024)                                         // smallest sector which can be erased independently
#define MAX_SECTOR_PARS_BANK0            (FLASH_GRANULARITY_BANK0 - 2)                    // The number of user bytes fitting into first parameter block
#define MAX_SECTOR_PARS_BANK1            (FLASH_GRANULARITY_BANK1 - 2)                    // The number of user bytes fitting into first parameter block

#define FLASH_GRANULARITY                 FLASH_GRANULARITY_BANK0                         // default value from largest bank (only for compatibility)


// Command User Interface (CUI) used for FLASH programming (Bank and/or sector)
//
  #define CUI_INVALID_RESET             0x00
  #define CUI_PROTECT_LEVEL1_CONFIRM    0x01
  #define CUI_WRITE_FLASH_CONF_CONFIRM  0x03
  #define CUI_ALT_PROG_SETUP            0x10
  #define CUI_SECTOR_ERASE_SETUP        0x20
  #define CUI_PROGRAM_SETUP             0x40
  #define CUI_CLEAR_STATUS_REGISTER     0x50
  #define CUI_PROT_LEVEL1_CONFIG_SETUP  0x60
  #define CUI_READ_STATUS_REGISTER      0x70
  #define CUI_BANK_ERASE_SETUP          0x80
  #define CUI_READ_ELECTRONIC_SIGNATURE 0x90
  #define CUI_READ_OTP_SECTOR           0x98
  #define CUI_PROG_ERASE_SUSPEND        0xb0
  #define CUI_WRITE_OTP                 0xc0
  #define CUI_PROG_ERASE_RESUME         0xd0
  #define CUI_ERASE_CONFIRM             0xd0
  #define CUI_UNPROTECT_LEVEL1_CONFIRM  0xd0
  #define CUI_READ_ARRAY                0xff

  #define RSIG_MANUFACTURER_CODE        0x00
  #define RSIG_DEVICE_CODE              0x02
  #define RSIG_DIE_REVISION_CODE        0x04
  #define RSIG_PROTECTION_LEVEL2_REG    0x08
  #define RSIG_PROTECTION_LEVEL1_REG    0x0a
  #define RSIG_FLASH_CONFIGURATION_REG  0x0c

  #define OTP_WORD_0                    0x00
  #define OTP_WORD_1                    0x01
  #define OTP_WORD_2                    0x02
  #define OTP_WORD_3                    0x03
  #define OTP_WORD_4                    0x04
  #define OTP_WORD_5                    0x05
  #define OTP_WORD_6                    0x06
  #define OTP_WORD_7                    0x07

// Bank Status Register
//
  #define BSR_PECS_READY                0x80                                  // Programm / Erase controller status
  #define BSR_ESS_SUSPENDED             0x40                                  // Erase Suspend Status
  #define BSR_ES_FAILURE                0x20                                  // Erase Status failure
  #define BSR_PS_FAILURE                0x10                                  // Program Status failure
  #define BSR_PSS_SUSPENDED             0x04                                  // Program Suspend Status
  #define BSR_SP_PROTECTION_ERROR       0x02                                  // Sector Protection status

  #define BSR_ILLEGAL_WRITE             0x01                                  // This bit doesn't exist but this error can be returned by the checking routine

// Protection Level 2 Register
//
  #define PL2_OTP_LOCK                  0x2000                                // OTP Sector locked
  #define PL2_JTAG_SECURITY             0x1000                                // JTAG access not allowed
  #define PL2_BANK1_3_WP                0x0800                                // Bank 1 sector 3 level 2 write protected
  #define PL2_BANK1_2_WP                0x0400                                // Bank 1 sector 2 level 2 write protected
  #define PL2_BANK1_1_WP                0x0200                                // Bank 1 sector 1 level 2 write protected
  #define PL2_BANK1_0_WP                0x0100                                // Bank 1 sector 0 level 2 write protected
  #define PL2_BANK0_7_WP                0x0080                                // Bank 0 sector 7 level 2 write protected
  #define PL2_BANK0_6_WP                0x0040                                // Bank 0 sector 6 level 2 write protected
  #define PL2_BANK0_5_WP                0x0020                                // Bank 0 sector 5 level 2 write protected
  #define PL2_BANK0_4_WP                0x0010                                // Bank 0 sector 4 level 2 write protected
  #define PL2_BANK0_3_WP                0x0008                                // Bank 0 sector 3 level 2 write protected
  #define PL2_BANK0_2_WP                0x0004                                // Bank 0 sector 2 level 2 write protected
  #define PL2_BANK0_1_WP                0x0002                                // Bank 0 sector 1 level 2 write protected
  #define PL2_BANK0_0_WP                0x0001                                // Bank 0 sector 0 level 2 write protected

// Protection Level 1 Register
//
  #define PL1_BANK1_3_WP                0x0800                                // Bank 1 sector 3 level 1 write protected
  #define PL1_BANK1_2_WP                0x0400                                // Bank 1 sector 2 level 1 write protected
  #define PL1_BANK1_1_WP                0x0200                                // Bank 1 sector 1 level 1 write protected
  #define PL1_BANK1_0_WP                0x0100                                // Bank 1 sector 0 level 1 write protected
  #define PL1_BANK0_7_WP                0x0080                                // Bank 0 sector 7 level 1 write protected
  #define PL1_BANK0_6_WP                0x0040                                // Bank 0 sector 6 level 1 write protected
  #define PL1_BANK0_5_WP                0x0020                                // Bank 0 sector 5 level 1 write protected
  #define PL1_BANK0_4_WP                0x0010                                // Bank 0 sector 4 level 1 write protected
  #define PL1_BANK0_3_WP                0x0008                                // Bank 0 sector 3 level 1 write protected
  #define PL1_BANK0_2_WP                0x0004                                // Bank 0 sector 2 level 1 write protected
  #define PL1_BANK0_1_WP                0x0002                                // Bank 0 sector 1 level 1 write protected
  #define PL1_BANK0_0_WP                0x0001                                // Bank 0 sector 0 level 1 write protected

// Flash Configuration Register
//
  #define FCR_WAIT_STATES_1             0x0000                                // 1 wait state for frequencies below 66MHz
  #define FCR_WAIT_STATES_2             0x0800                                // 2 wait states require for from 75MHz
  #define FCR_WAIT_STATES_3             0x1000
  #define FCR_PWD                       0x0400                                // Power down enabled
  #define FCR_LVDEN                     0x0200                                // Low Voltage Detector disabled
  #define FCR_BUSCFG                    0x0010                                // FLASH bus clock > 66 MHz


// Wake-up / Interrupt Unit
//
#define WIU_CTRL                        *(unsigned long *)(WIU_BLOCK + 0x00)  // WUI Control Register
  #define WIUINT_EN                     0x00000002                            // Global WIU Interrupt Enable
  #define WKUP_INT                      0x00000001                            // WIU Event Enable
#define WIU_MR                          *(unsigned long *)(WIU_BLOCK + 0x04)  // WUI Control Register
  #define WUM_EXTINT2_MASK              0x00000004
  #define WUM_EXTINT3_MASK              0x00000008
  #define WUM_EXTINT4_MASK              0x00000010
  #define WUM_EXTINT5_MASK              0x00000020
  #define WUM_EXTINT6_MASK              0x00000040
  #define WUM_EXTINT7_MASK              0x00000080
  #define WUM_EXTINT8_MASK              0x00000100
  #define WUM_EXTINT9_MASK              0x00000200
  #define WUM_EXTINT10_MASK             0x00000400
  #define WUM_EXTINT11_MASK             0x00000800
  #define WUM_EXTINT12_MASK             0x00001000
  #define WUM_EXTINT13_MASK             0x00002000
  #define WUM_EXTINT14_MASK             0x00004000
  #define WUM_EXTINT15_MASK             0x00008000
  #define WUM_EXTINT16_MASK             0x00010000
  #define WUM_EXTINT17_MASK             0x00020000
  #define WUM_EXTINT18_MASK             0x00040000
  #define WUM_EXTINT19_MASK             0x00080000
  #define WUM_EXTINT20_MASK             0x00100000
  #define WUM_EXTINT21_MASK             0x00200000
  #define WUM_EXTINT22_MASK             0x00400000
  #define WUM_EXTINT23_MASK             0x00800000
  #define WUM_EXTINT24_MASK             0x01000000
  #define WUM_EXTINT25_MASK             0x02000000
  #define WUM_EXTINT26_MASK             0x04000000
  #define WUM_EXTINT27_MASK             0x08000000
  #define WUM_EXTINT28_MASK             0x10000000
  #define WUM_EXTINT29_MASK             0x20000000
  #define WUM_EXTINT30_MASK             0x40000000
  #define WUM_EXTINT31_MASK             0x80000000
#define WIU_TR                          *(unsigned long *)(WIU_BLOCK + 0x08)  // WUI Trigger Register
  #define WUM_EXTINT2_RISING_EDGE       0x00000004
  #define WUM_EXTINT3_RISING_EDGE       0x00000008
  #define WUM_EXTINT4_RISING_EDGE       0x00000010
  #define WUM_EXTINT5_RISING_EDGE       0x00000020
  #define WUM_EXTINT6_RISING_EDGE       0x00000040
  #define WUM_EXTINT7_RISING_EDGE       0x00000080
  #define WUM_EXTINT8_RISING_EDGE       0x00000100
  #define WUM_EXTINT9_RISING_EDGE       0x00000200
  #define WUM_EXTINT10_RISING_EDGE      0x00000400
  #define WUM_EXTINT11_RISING_EDGE      0x00000800
  #define WUM_EXTINT12_RISING_EDGE      0x00001000
  #define WUM_EXTINT13_RISING_EDGE      0x00002000
  #define WUM_EXTINT14_RISING_EDGE      0x00004000
  #define WUM_EXTINT15_RISING_EDGE      0x00008000
  #define WUM_EXTINT16_RISING_EDGE      0x00010000
  #define WUM_EXTINT17_RISING_EDGE      0x00020000
  #define WUM_EXTINT18_RISING_EDGE      0x00040000
  #define WUM_EXTINT19_RISING_EDGE      0x00080000
  #define WUM_EXTINT20_RISING_EDGE      0x00100000
  #define WUM_EXTINT21_RISING_EDGE      0x00200000
  #define WUM_EXTINT22_RISING_EDGE      0x00400000
  #define WUM_EXTINT23_RISING_EDGE      0x00800000
  #define WUM_EXTINT24_RISING_EDGE      0x01000000
  #define WUM_EXTINT25_RISING_EDGE      0x02000000
  #define WUM_EXTINT26_RISING_EDGE      0x04000000
  #define WUM_EXTINT27_RISING_EDGE      0x08000000
  #define WUM_EXTINT28_RISING_EDGE      0x10000000
  #define WUM_EXTINT29_RISING_EDGE      0x20000000
  #define WUM_EXTINT30_RISING_EDGE      0x40000000
  #define WUM_EXTINT31_RISING_EDGE      0x80000000
#define WIU_PR                          *(unsigned long *)(WIU_BLOCK + 0x0c)  // WUI Pending Register
#define WIU_INTR                        *(unsigned long *)(WIU_BLOCK + 0x10)  // WUI Software Interrupt Register


// Timer (0 and 1 support DMA)
//
#define TIM_IC1R_0                      *(unsigned long *)(TIM0_BLOCK + 0x00) // Input Capture Register 1 - Timer 0
#define TIM_IC2R_0                      *(unsigned long *)(TIM0_BLOCK + 0x04) // Input Capture Register 2 - Timer 0
#define TIM_OC1R_0                      *(unsigned long *)(TIM0_BLOCK + 0x08) // Output Compare Register 1 - Timer 0
#define TIM_OC2R_0                      *(unsigned long *)(TIM0_BLOCK + 0x0c) // Output Compare Register 2 - Timer 0
#define TIM_CNTR_0                      *(volatile unsigned long *)(TIM0_BLOCK + 0x10) // Counter - Timer 0 (read only)
#define TIM_CR1_0                       *(unsigned long *)(TIM0_BLOCK + 0x14) // Control Register 1 - Timer 0
  #define COUNTER_ENABLE                0x8000
  #define PWMI                          0x4000
  #define OLVL2_HIGH                    0x0200
  #define OLVL2_LOW                     0x0000
  #define OLVL1_HIGH                    0x0100
  #define OLVL1_LOW                     0x0000
  #define OC2E                          0x0080
  #define OC1E                          0x0040
  #define PWM                           0x0010
  #define ECKEN                         0x0001

#define TIM_CR2_0                       *(unsigned long *)(TIM0_BLOCK + 0x18) // Control Register 2 - Timer 0
  #define CLOCK_CONTROL_MASK            0x00ff                                // PCLK divide
  #define CLOCK_DIV_1                   0x0000 
  #define CLOCK_DIV_8                   0x0007 
  #define CLOCK_DIV_32                  0x001f                                 
  #define CLOCK_DIV_64                  0x003f
  #define CLOCK_DIV_256                 0x00ff 
  #define DMAE                          0x0400
  #define OC2IE                         0x0800
  #define IC2IE                         0x1000
  #define TOIE                          0x2000
  #define OC1IE                         0x4000
  #define IC1IE                         0x8000
#define TIM_SR_0                        *(volatile unsigned long *)(TIM0_BLOCK + 0x1c) // Status Register - Timer 0
  #define TIM_SR_OCF2                   0x0800
  #define TIM_SR_ICF2                   0x1000
  #define TIM_SR_TOF                    0x2000
  #define TIM_SR_OCF1                   0x4000
  #define TIM_SR_ICF1                   0x8000

#define TIM_IC1R_1                      *(unsigned long *)(TIM1_BLOCK + 0x00) // Input Capture Register 1 - Timer 1
#define TIM_IC2R_1                      *(unsigned long *)(TIM1_BLOCK + 0x04) // Input Capture Register 2 - Timer 1
#define TIM_OC1R_1                      *(unsigned long *)(TIM1_BLOCK + 0x08) // Output Compare Register 1 - Timer 1
#define TIM_OC2R_1                      *(unsigned long *)(TIM1_BLOCK + 0x0c) // Output Compare Register 2 - Timer 1
#define TIM_CNTR_1                      *(volatile unsigned long *)(TIM1_BLOCK + 0x10) // Counter - Timer 1 (read only)
#define TIM_CR1_1                       *(volatile unsigned long *)(TIM1_BLOCK + 0x14) // Control Register 1 - Timer 1
#define TIM_CR2_1                       *(unsigned long *)(TIM1_BLOCK + 0x18) // Control Register 2 - Timer 1
#define TIM_SR_1                        *(unsigned long *)(TIM1_BLOCK + 0x1c) // Status Register - Timer 1

#define TIM_IC1R_2                      *(unsigned long *)(TIM2_BLOCK + 0x00) // Input Capture Register 1 - Timer 2
#define TIM_IC2R_2                      *(unsigned long *)(TIM2_BLOCK + 0x04) // Input Capture Register 2 - Timer 2
#define TIM_OC1R_2                      *(unsigned long *)(TIM2_BLOCK + 0x08) // Output Compare Register 1 - Timer 2
#define TIM_OC2R_2                      *(unsigned long *)(TIM2_BLOCK + 0x0c) // Output Compare Register 2 - Timer 2
#define TIM_CNTR_2                      *(volatile unsigned long *)(TIM2_BLOCK + 0x10) // Counter - Timer 2 (read only)
#define TIM_CR1_2                       *(unsigned long *)(TIM2_BLOCK + 0x14) // Control Register 1 - Timer 2
#define TIM_CR2_2                       *(unsigned long *)(TIM2_BLOCK + 0x18) // Control Register 2 - Timer 2
  #define LOW_RES_MS                     (PCLK / 256 / 1000)                  // slowest clock rate to timer, giving longest period but minimum resolution
  #define MED_RES_MS                     (PCLK / 64 / 1000)                   // medium clock rate to timer, giving medium period and medium resolution
  #define HIGH_RES_MS                    (PCLK / 32 / 1000)                   // high clock rate to timer, giving minimum period and maximum resolution

#define TIM_SR_2                        *(unsigned long *)(TIM2_BLOCK + 0x1c) // Status Register - Timer 2

#define TIM_IC1R_3                      *(unsigned long *)(TIM3_BLOCK + 0x00) // Input Capture Register 1 - Timer 3
#define TIM_IC2R_3                      *(unsigned long *)(TIM3_BLOCK + 0x04) // Input Capture Register 2 - Timer 3
#define TIM_OC1R_3                      *(unsigned long *)(TIM3_BLOCK + 0x08) // Output Compare Register 1 - Timer 3
#define TIM_OC2R_3                      *(unsigned long *)(TIM3_BLOCK + 0x0c) // Output Compare Register 2 - Timer 3
#define TIM_CNTR_3                      *(volatile unsigned long *)(TIM3_BLOCK + 0x10) // Counter - Timer 3 (read only)
#define TIM_CR1_3                       *(volatile unsigned long *)(TIM3_BLOCK + 0x14) // Control Register 1 - Timer 3
#define TIM_CR2_3                       *(unsigned long *)(TIM3_BLOCK + 0x18) // Control Register 2 - Timer 3
#define TIM_SR_3                        *(unsigned long *)(TIM3_BLOCK + 0x1c) // Status Register - Timer 3


// GPIO
//
#define GPIO_DATA_0                     *(volatile unsigned long *)(GPIO_PORT0_BLOCK + 0x3fc) // GPIO Data Register - GPIO 0
                                        // This register occupies 0..0x3fc where the changed/read bit is addresses by A9:A2
#define GPIO_DIR_0                      *(unsigned long *)(GPIO_PORT0_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 0
#define GPIO_SEL_0                      *(unsigned long *)(GPIO_PORT0_BLOCK + 0x420) // GPIO Select Register - GPIO 0

#define GPIO_DATA_1                     *(volatile unsigned long *)(GPIO_PORT1_BLOCK + 0x3fc) // GPIO Data Register - GPIO 1
#define GPIO_DIR_1                      *(unsigned long *)(GPIO_PORT1_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 1
#define GPIO_SEL_1                      *(unsigned long *)(GPIO_PORT1_BLOCK + 0x420) // GPIO Select Register - GPIO 1

#define GPIO_DATA_2                     *(volatile unsigned long *)(GPIO_PORT2_BLOCK + 0x3fc) // GPIO Data Register - GPIO 2
#define GPIO_DIR_2                      *(unsigned long *)(GPIO_PORT2_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 2
#define GPIO_SEL_2                      *(unsigned long *)(GPIO_PORT2_BLOCK + 0x420) // GPIO Select Register - GPIO 2

#define GPIO_DATA_3                     *(volatile unsigned long *)(GPIO_PORT3_BLOCK + 0x3fc) // GPIO Data Register - GPIO 3
#define GPIO_DIR_3                      *(unsigned long *)(GPIO_PORT3_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 3
#define GPIO_SEL_3                      *(unsigned long *)(GPIO_PORT3_BLOCK + 0x420) // GPIO Select Register - GPIO 3

#define GPIO_DATA_4                     *(volatile unsigned long *)(GPIO_PORT4_BLOCK + 0x3fc) // GPIO Data Register - GPIO 4
#define GPIO_DIR_4                      *(unsigned long *)(GPIO_PORT4_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 4
#define GPIO_SEL_4                      *(unsigned long *)(GPIO_PORT4_BLOCK + 0x420) // GPIO Select Register - GPIO 4

#define GPIO_DATA_5                     *(volatile unsigned long *)(GPIO_PORT5_BLOCK + 0x3fc) // GPIO Data Register - GPIO 5
#define GPIO_DIR_5                      *(unsigned long *)(GPIO_PORT5_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 5
#define GPIO_SEL_5                      *(unsigned long *)(GPIO_PORT5_BLOCK + 0x420) // GPIO Select Register - GPIO 5

#define GPIO_DATA_6                     *(volatile unsigned long *)(GPIO_PORT6_BLOCK + 0x3fc) // GPIO Data Register - GPIO 6
#define GPIO_DIR_6                      *(unsigned long *)(GPIO_PORT6_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 6
#define GPIO_SEL_6                      *(unsigned long *)(GPIO_PORT6_BLOCK + 0x420) // GPIO Select Register - GPIO 6

#define GPIO_DATA_7                     *(volatile unsigned long *)(GPIO_PORT7_BLOCK + 0x3fc) // GPIO Data Register - GPIO 7
#define GPIO_DIR_7                      *(unsigned long *)(GPIO_PORT7_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 7
#define GPIO_SEL_7                      *(unsigned long *)(GPIO_PORT7_BLOCK + 0x420) // GPIO Select Register - GPIO 7

#define GPIO_DATA_8                     *(volatile unsigned long *)(GPIO_PORT8_BLOCK + 0x3fc) // GPIO Data Register - GPIO 8
#define GPIO_DIR_8                      *(unsigned long *)(GPIO_PORT8_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 8
#define GPIO_SEL_8                      *(unsigned long *)(GPIO_PORT8_BLOCK + 0x420) // GPIO Select Register - GPIO 8

#define GPIO_DATA_9                     *(volatile unsigned long *)(GPIO_PORT9_BLOCK + 0x3fc) // GPIO Data Register - GPIO 9
#define GPIO_DIR_9                      *(unsigned long *)(GPIO_PORT9_BLOCK + 0x400) // GPIO Data Direction Register - GPIO 9
#define GPIO_SEL_9                      *(unsigned long *)(GPIO_PORT9_BLOCK + 0x420) // GPIO Select Register - GPIO 9

#define CHANGE_BIT_0                    0x004
#define CHANGE_BIT_1                    0x008
#define CHANGE_BIT_2                    0x010
#define CHANGE_BIT_3                    0x020
#define CHANGE_BIT_4                    0x040
#define CHANGE_BIT_5                    0x080
#define CHANGE_BIT_6                    0x100
#define CHANGE_BIT_7                    0x200

#define SINGLE_BIT_SHIFT                2

#define PORT0_BIT0                      0x01
#define PORT0_BIT1                      0x02
#define PORT0_BIT2                      0x04
#define PORT0_BIT3                      0x08
#define PORT0_BIT4                      0x10
#define PORT0_BIT5                      0x20
#define PORT0_BIT6                      0x40
#define PORT0_BIT7                      0x80

#define PORT1_BIT0                      0x01
#define PORT1_BIT1                      0x02
#define PORT1_BIT2                      0x04
#define PORT1_BIT3                      0x08
#define PORT1_BIT4                      0x10
#define PORT1_BIT5                      0x20
#define PORT1_BIT6                      0x40
#define PORT1_BIT7                      0x80

#define PORT2_BIT0                      0x01
#define PORT2_BIT1                      0x02
#define PORT2_BIT2                      0x04
#define PORT2_BIT3                      0x08
#define PORT2_BIT4                      0x10
#define PORT2_BIT5                      0x20
#define PORT2_BIT6                      0x40
#define PORT2_BIT7                      0x80

#define PORT3_BIT0                      0x01
#define PORT3_BIT1                      0x02
#define PORT3_BIT2                      0x04
#define PORT3_BIT3                      0x08
#define PORT3_BIT4                      0x10
#define PORT3_BIT5                      0x20
#define PORT3_BIT6                      0x40
#define PORT3_BIT7                      0x80

#define PORT4_BIT0                      0x01
#define PORT4_BIT1                      0x02
#define PORT4_BIT2                      0x04
#define PORT4_BIT3                      0x08
#define PORT4_BIT4                      0x10
#define PORT4_BIT5                      0x20
#define PORT4_BIT6                      0x40
#define PORT4_BIT7                      0x80

#define PORT5_BIT0                      0x01
#define PORT5_BIT1                      0x02
#define PORT5_BIT2                      0x04
#define PORT5_BIT3                      0x08
#define PORT5_BIT4                      0x10
#define PORT5_BIT5                      0x20
#define PORT5_BIT6                      0x40
#define PORT5_BIT7                      0x80

#define PORT6_BIT0                      0x01
#define PORT6_BIT1                      0x02
#define PORT6_BIT2                      0x04
#define PORT6_BIT3                      0x08
#define PORT6_BIT4                      0x10
#define PORT6_BIT5                      0x20
#define PORT6_BIT6                      0x40
#define PORT6_BIT7                      0x80

#define PORT7_BIT0                      0x01
#define PORT7_BIT1                      0x02
#define PORT7_BIT2                      0x04
#define PORT7_BIT3                      0x08
#define PORT7_BIT4                      0x10
#define PORT7_BIT5                      0x20
#define PORT7_BIT6                      0x40
#define PORT7_BIT7                      0x80

#define PORT8_BIT0                      0x01
#define PORT8_BIT1                      0x02
#define PORT8_BIT2                      0x04
#define PORT8_BIT3                      0x08
#define PORT8_BIT4                      0x10
#define PORT8_BIT5                      0x20
#define PORT8_BIT6                      0x40
#define PORT8_BIT7                      0x80

#define PORT9_BIT0                      0x01
#define PORT9_BIT1                      0x02
#define PORT9_BIT2                      0x04
#define PORT9_BIT3                      0x08
#define PORT9_BIT4                      0x10
#define PORT9_BIT5                      0x20
#define PORT9_BIT6                      0x40
#define PORT9_BIT7                      0x80



// Port macros (WARNING: ports 8 and 9, if available, have neither SCU_GPIOOUT nor SCU_GPIOIN register. These macros are therefore not necessarily suitable for operation on these ports!!)
//
// Configure pins as output, including enabling power and digital use. eg. _CONFIG_PORT_OUTPUT(1, (PORT1_BIT6 | PORT1_BIT2));
//
// First the port is powered and enabled, then the peripheral function selection is masked, port out function set, inputs disabled and the output is finally driven (it drives the original port line state)
#define _CONFIG_PORT_OUTPUT(ref, pins) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref\
                                       SCU_GPIOOUT##ref &= ~((((pins) & 0x01) | (((pins) & 0x01) << 1)) | ((((pins) & 0x02) << 1) | (((pins) & 0x02) << 2))  | ((((pins) & 0x04) << 2) | (((pins) & 0x04) << 3))   | ((((pins) & 0x08) << 3) | (((pins) & 0x08) << 4))   | ((((pins) & 0x10) << 4) | (((pins) & 0x10) << 5))   | ((((pins) & 0x20) << 5) | (((pins) & 0x20) << 6))   | ((((pins) & 0x40) << 6) | (((pins) & 0x40) << 7)) | ((((pins) & 0x80) << 7) | (((pins) & 0x80) << 8))); \
                                       SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       SCU_GPIOIN##ref  &= ~(pins); \
                                       GPIO_DIR_##ref   |=  (pins); _SIM_PORT_CHANGE

#define _CONFIG_PORT_OUTPUT_8_9(ref, pins) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref\
                                       GPIO_DIR_##ref   |=  (pins); _SIM_PORT_CHANGE

// Configure pins as input, including enabling power and digital use. eg. _CONFIG_PORT_INPUT(2, PORT2_BIT4);
//
#define _CONFIG_PORT_INPUT(ref, pins)  SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       SCU_GPIOOUT##ref &= ~((((pins) & 0x01) | (((pins) & 0x01) << 1)) | ((((pins) & 0x02) << 1) | (((pins) & 0x02) << 2))  | ((((pins) & 0x04) << 2) | (((pins) & 0x04) << 3))   | ((((pins) & 0x08) << 3) | (((pins) & 0x08) << 4))   | ((((pins) & 0x10) << 4) | (((pins) & 0x10) << 5))   | ((((pins) & 0x20) << 5) | (((pins) & 0x20) << 6))   | ((((pins) & 0x40) << 6) | (((pins) & 0x40) << 7))   | ((((pins) & 0x80) << 7) | (((pins) & 0x80) << 8))); \
                                       SCU_GPIOIN##ref  &= ~(pins); \
                                       GPIO_DIR_##ref   &= ~(pins); _SIM_PORT_CHANGE

#define _CONFIG_PORT_INPUT_8_9(ref, pins)  SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       GPIO_DIR_##ref   &= ~(pins); _SIM_PORT_CHANGE

// Configure a peripheral function
//
#define _CONFIG_PERIPHERAL(power_block, port_ref, periph, function_2, function_3, pins, inputs) \
        SCU_PCGR##power_block |= (periph); SCU_PRR##power_block |= (periph); \
        SCU_GPIOOUT##port_ref &= ~((((pins) & 0x01) | (((pins) & 0x01) << 1)) | ((((pins) & 0x02) << 1) | (((pins) & 0x02) << 2))  | ((((pins) & 0x04) << 2) | (((pins) & 0x04) << 3)) | ((((pins) & 0x08) << 3) | (((pins) & 0x08) << 4)) | ((((pins) & 0x10) << 4) | (((pins) & 0x10) << 5)) | ((((pins) & 0x20) << 5) | (((pins) & 0x20) << 6)) | ((((pins) & 0x40) << 6) | (((pins) & 0x40) << 7)) | ((((pins) & 0x80) << 7) | (((pins) & 0x80) << 8))); \
        SCU_GPIOOUT##port_ref |= ((((function_2) & 0x01) << 1) | (((function_2) & 0x02) << 2) | (((function_2) & 0x04) << 3) | (((function_2) & 0x08) << 4) | (((function_2) & 0x10) << 5) | (((function_2) & 0x20) << 6) | (((function_2) & 0x40) << 7) | (((function_2) & 0x80) << 8)); \
        SCU_GPIOOUT##port_ref |= ((((function_3) & 0x01) | (((function_3) & 0x01) << 1)) | ((((function_3) & 0x02) << 1) | (((function_3) & 0x02) << 2)) | ((((function_3) & 0x04) << 2) | (((function_3) & 0x04) << 3)) | ((((function_3) & 0x08) << 3) | (((function_3) & 0x08) << 4)) | ((((function_3) & 0x10) << 4) | (((function_3) & 0x10) << 5)) | ((((function_3) & 0x20) << 5) | (((function_3) & 0x20) << 6)) | ((((function_3) & 0x40) << 6) | (((function_3) & 0x40) << 7))   | ((((function_3) & 0x80) << 7) | (((function_3) & 0x80) << 8))); \
        SCU_GPIOIN##port_ref  |= inputs; _SIM_PORT_CHANGE


// Write full port width
//
#define _WRITE_PORT(ref, value)        GPIO_DATA_##ref = (value); _SIM_PORT_CHANGE


// Write to a port with a mask
//
#ifdef _WINDOWS
    #define _WRITE_PORT_MASK(ref, value, mask)    GPIO_DATA_##ref = ((GPIO_DATA_##ref & ~mask) | (value & mask)); _SIM_PORT_CHANGE
#else
    #define _WRITE_PORT_MASK(ref, value, mask)    *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*mask) = (value);
#endif

// Toggle a port with a mask
//
#define _TOGGLE_PORT(ref, mask)        GPIO_DATA_##ref ^= (mask); _SIM_PORT_CHANGE

// Read full port width
//
#define _READ_PORT(ref)                (GPIO_DATA_##ref); _SIM_PORT_CHANGE

// Read from a port with a mask
//
#ifdef _WINDOWS
    #define _READ_PORT_MASK(ref, mask) (GPIO_DATA_##ref & mask)
#else
    #define _READ_PORT_MASK(ref, mask) (*(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*mask))
#endif

// Configure outputs and then set a value to them
//
#ifdef _WINDOWS
    #define _CONFIG_DRIVE_PORT_OUTPUT_VALUE(ref, pins, value) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       SCU_GPIOOUT##ref &= ~(((pins & 0x01) | ((pins & 0x01) << 1)) | (((pins & 0x02) << 1) | ((pins & 0x02) << 2))  | (((pins & 0x04) << 2) | ((pins & 0x04) << 3))   | (((pins & 0x08) << 3) | ((pins & 0x08) << 4))   | (((pins & 0x10) << 4) | ((pins & 0x10) << 5))   | (((pins & 0x20) << 5) | ((pins & 0x20) << 6))   | (((pins & 0x40) << 6) | ((pins & 0x40) << 7))   | (((pins & 0x80) << 7) | ((pins & 0x80) << 8))); \
                                       SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       SCU_GPIOIN##ref  &= ~(pins); \
                                       GPIO_DIR_##ref   |=  (pins); \
                                       GPIO_DATA_##ref = ((GPIO_DATA_##ref & ~pins) | (value & pins)); _SIM_PORT_CHANGE

    #define _CONFIG_DRIVE_PORT_OUTPUT_VALUE_8_9(ref, pins, value) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       GPIO_DIR_##ref   |=  (pins); \
                                       GPIO_DATA_##ref = ((GPIO_DATA_##ref & ~pins) | (value & pins)); _SIM_PORT_CHANGE
#else
    #define _CONFIG_DRIVE_PORT_OUTPUT_VALUE(ref, pins, value) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       SCU_GPIOOUT##ref &= ~(((pins & 0x01) | ((pins & 0x01) << 1)) | (((pins & 0x02) << 1) | ((pins & 0x02) << 2))  | (((pins & 0x04) << 2) | ((pins & 0x04) << 3))   | (((pins & 0x08) << 3) | ((pins & 0x08) << 4))   | (((pins & 0x10) << 4) | ((pins & 0x10) << 5))   | (((pins & 0x20) << 5) | ((pins & 0x20) << 6))   | (((pins & 0x40) << 6) | ((pins & 0x40) << 7))   | (((pins & 0x80) << 7) | ((pins & 0x80) << 8))); \
                                       SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       SCU_GPIOIN##ref  &= ~(pins); \
                                       GPIO_DIR_##ref   |=  (pins); \
                                       *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*pins) = (value)

    #define _CONFIG_DRIVE_PORT_OUTPUT_VALUE_8_9(ref, pins, value) SCU_PCGR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; SCU_PRR1 |= GPIO##ref; \
                                       GPIO_DIR_##ref   |=  (pins); \
                                       *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*pins) = (value)
#endif

// Set from outputs to inputs
//
#define _FLOAT_PORT(ref, pins)         SCU_GPIOOUT##ref &= ~(((pins & 0x01) | ((pins & 0x01) << 1)) | (((pins & 0x02) << 1) | ((pins & 0x02) << 2))  | (((pins & 0x04) << 2) | ((pins & 0x04) << 3))   | (((pins & 0x08) << 3) | ((pins & 0x08) << 4))   | (((pins & 0x10) << 4) | ((pins & 0x10) << 5))   | (((pins & 0x20) << 5) | ((pins & 0x20) << 6))   | (((pins & 0x40) << 6) | ((pins & 0x40) << 7))   | (((pins & 0x80) << 7) | ((pins & 0x80) << 8))); \
                                       GPIO_DIR_##ref &= ~(pins); _SIM_PORT_CHANGE

// Set from inputs to outputs 
//
#define _DRIVE_PORT_OUTPUT(ref, pins)  SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       GPIO_DIR_##ref |= (pins); _SIM_PORT_CHANGE

// Set from inputs to outputs and set a value to them
//
#ifdef _WINDOWS
    #define _DRIVE_PORT_OUTPUT_VALUE(ref, pins, value) SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       GPIO_DIR_##ref |= (pins); \
                                       GPIO_DATA_##ref = ((GPIO_DATA_##ref & ~pins) | (value & pins)); _SIM_PORT_CHANGE
#else
    #define _DRIVE_PORT_OUTPUT_VALUE(ref, pins, value) SCU_GPIOOUT##ref |= (((pins) & 0x01) | (((pins) & 0x02) << 1) | (((pins) & 0x04) << 2) | (((pins) & 0x08) << 3) | (((pins) & 0x10) << 4) | (((pins) & 0x20) << 5) | (((pins) & 0x40) << 6) | (((pins) & 0x80) << 7)); \
                                       GPIO_DIR_##ref |= (pins); \
                                       *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*pins) = (value)
#endif

// Set and clear individual bits of a port
//
#ifdef _WINDOWS
    #define _SETBITS(ref, mask)        GPIO_DATA_##ref |= mask; _SIM_PORT_CHANGE
    #define _CLEARBITS(ref, mask)      GPIO_DATA_##ref &= ~mask; _SIM_PORT_CHANGE
#else
    #define _SETBITS(ref, mask)        *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*mask) = 0xff
    #define _CLEARBITS(ref, mask)      *(unsigned long *)(GPIO_PORT##ref##_BLOCK + 4*mask) = 0x00
#endif





// UARTs
//
#define UART_DR_0                       *(volatile unsigned long *)(UART0_BLOCK + 0x00) // Data Register UART 0
#define UART_RSECR_0                    *(volatile unsigned long *)(UART0_BLOCK + 0x04) // Receive Status Register / Error Clear register UART 0
#define UART_FR_0                       *(volatile unsigned long *)(UART0_BLOCK + 0x18) // Flag Register UART 0
  #define TXFE                          0x0080
  #define RXFF                          0x0040
#define UART_ILPR_0                     *(unsigned long *)(UART0_BLOCK + 0x20) // IrDA Low Power Counter Divisor Register UART 0
#define IBRD_OFFSET                     0x24
#define UART_IBRD_0                     *(unsigned long *)(UART0_BLOCK + 0x24) // Integer Baud Rate Register UART 0
#define UART_FBRD_0                     *(unsigned long *)(UART0_BLOCK + 0x28) // Fractional Baud Rate Register UART 0
#define UART_LCR_0                      *(unsigned long *)(UART0_BLOCK + 0x2c) // Line Control Register UART 0
  #define STR_PARITY_ENABLE             0x02
  #define STR_ODD_PARITY                (STR_PARITY_ENABLE)
  #define STR_EVEN_PARITY               (STR_PARITY_ENABLE | 0x04)
  #define STR_NO_PARITY                 0x00
  #define STR_2_STOPS                   0x08
  #define STR_CHAR_7                    0x40
  #define STR_CHAR_8                    0x60

#define CR_OFFSET                       0x30
#define UART_CR_0                       *(unsigned long *)(UART0_BLOCK + 0x30) // Control Register UART 0
  #define UARTEN                        0x0001
  #define LBE                           0x0080
  #define TXE                           0x0100
  #define RXE                           0x0200
#define UART_IFLS_0                     *(unsigned long *)(UART0_BLOCK + 0x34) // Interrupt FIFO Level Select Register UART 0
#define IMSC_OFFSET                     0x38
#define UART_IMSC_0                     *(unsigned long *)(UART0_BLOCK + 0x38) // Interrupt Mask Set / Clear Register UART 0
  #define TXIM                          0x0020
  #define RXIM                          0x0010
#define UART_RIS_0                      *(volatile unsigned long *)(UART0_BLOCK + 0x3c) // RAW Interrupt Status Register UART 0
  #define TXRIS                         0x0020
  #define RXRIS                         0x0010
#define UART_MIS_0                      *(volatile unsigned long *)(UART0_BLOCK + 0x40) // Masked Interrupt Status Register UART 0
  #define ICR_OFFSET                    0x44
#define UART_ICR_0                      *(volatile unsigned long *)(UART0_BLOCK + 0x44) // Interrupt Clear Register UART 0
  #define OEIC                          0x0400
  #define BEIC                          0x0200
  #define PEIC                          0x0100
  #define FEIC                          0x0080
  #define RTIC                          0x0040
  #define TXIC                          0x0020
  #define RXIC                          0x0010
#define UART_DMACR_0                    *(unsigned long *)(UART0_BLOCK + 0x48) // DMA Control Register UART 0

#define UART_DR_1                       *(volatile unsigned long *)(UART1_BLOCK + 0x00) // Data Register UART 1
#define UART_RSECR_1                    *(volatile unsigned long *)(UART1_BLOCK + 0x04) // Receive Status Register / Error Clear register UART 1
#define UART_FR_1                       *(volatile unsigned long *)(UART1_BLOCK + 0x18) // Flag Register UART 1
#define UART_ILPR_1                     *(unsigned long *)(UART1_BLOCK + 0x20) // IrDA Low Power Counter Divisor Register UART 1
#define UART_IBRD_1                     *(unsigned long *)(UART1_BLOCK + 0x24) // Integer Baud Rate Register UART 1
#define UART_FBRD_1                     *(unsigned long *)(UART1_BLOCK + 0x28) // Fractional Baud Rate Register UART 1
#define UART_LCR_1                      *(unsigned long *)(UART1_BLOCK + 0x2c) // Line Control Register UART 1
#define UART_CR_1                       *(unsigned long *)(UART1_BLOCK + 0x30) // Control Register UART 1
#define UART_IFLS_1                     *(unsigned long *)(UART1_BLOCK + 0x34) // Interrupt FIFO Level Select Register UART 1
#define UART_IMSC_1                     *(unsigned long *)(UART1_BLOCK + 0x38) // Interrupt Mask Set / Clear Register UART 1
#define UART_RIS_1                      *(volatile unsigned long *)(UART1_BLOCK + 0x3c) // RAW Interrupt Status Register UART 1
#define UART_MIS_1                      *(volatile unsigned long *)(UART1_BLOCK + 0x40) // Masked Interrupt Status Register UART 1
#define UART_ICR_1                      *(volatile unsigned long *)(UART1_BLOCK + 0x44) // Interrupt Clear Register UART 1
#define UART_DMACR_1                    *(unsigned long *)(UART1_BLOCK + 0x48) // DMA Control Register UART 1

#define UART_DR_2                       *(volatile unsigned long *)(UART2_BLOCK + 0x00) // Data Register UART 2
#define UART_RSECR_2                    *(volatile unsigned long *)(UART2_BLOCK + 0x04) // Receive Status Register / Error Clear register UART 2
#define UART_FR_2                       *(volatile unsigned long *)(UART2_BLOCK + 0x18) // Flag Register UART 2
#define UART_ILPR_2                     *(unsigned long *)(UART2_BLOCK + 0x20) // IrDA Low Power Counter Divisor Register UART 2
#define UART_IBRD_2                     *(unsigned long *)(UART2_BLOCK + 0x24) // Integer Baud Rate Register UART 2
#define UART_FBRD_2                     *(unsigned long *)(UART2_BLOCK + 0x28) // Fractional Baud Rate Register UART 2
#define UART_LCR_2                      *(unsigned long *)(UART2_BLOCK + 0x2c) // Line Control Register UART 2
#define UART_CR_2                       *(unsigned long *)(UART2_BLOCK + 0x30) // Control Register UART 2
#define UART_IFLS_2                     *(unsigned long *)(UART2_BLOCK + 0x34) // Interrupt FIFO Level Select Register UART 2
#define UART_IMSC_2                     *(unsigned long *)(UART2_BLOCK + 0x38) // Interrupt Mask Set / Clear Register UART 2
#define UART_RIS_2                      *(volatile unsigned long *)(UART2_BLOCK + 0x3c) // RAW Interrupt Status Register UART 2
#define UART_MIS_2                      *(volatile unsigned long *)(UART2_BLOCK + 0x40) // Masked Interrupt Status Register UART 2
#define UART_ICR_2                      *(volatile unsigned long *)(UART2_BLOCK + 0x44) // Interrupt Clear Register UART 2
#define UART_DMACR_2                    *(unsigned long *)(UART2_BLOCK + 0x48) // DMA Control Register UART 2


// SSP
//
#define SSP0_CR_0                       *(unsigned long *)(SSP0_BLOCK + 0x00)  // Control Register 0 SSP 0
  #define SCR_MASK                      0xff00                                 // Serial Clock Rate mask
  #define CPHA                          0x0080                                 // Serial Clock Phase
  #define CPOL                          0x0040                                 // Serial Clock Polarity
  #define FRAME_FORMAT_MOT              0x0000
  #define FRAME_FORMAT_TI               0x0010
  #define FRAME_FORMAT_NAT              0x0020
  #define SSP_DATA_SIZE_MASK            0x000f
#define SSP0_CR_1                       *(unsigned long *)(SSP0_BLOCK + 0x04)  // Control Register 1 SSP 0
  #define SSP_SOD                       0x0008                                 // Slave Mode Output Disable
  #define SSP_MS                        0x0004                                 // nMaster/Slave Mode
  #define SSP_SSE                       0x0002                                 // SSP enable
  #define SSP_LBM                       0x0001                                 // Loop back mode
#define SSP0_DR                         *(volatile unsigned long *)(SSP0_BLOCK + 0x08)  // Data Register SSP 0
#define SSP0_SR                         *(volatile unsigned long *)(SSP0_BLOCK + 0x0c)  // Status Register SSP 0
  #define SSP_BUSY                      0x0010                                 // SSP Busy
  #define SSP_RFF                       0x0008                                 // Receive FIFO Full
  #define SSP_RNE                       0x0004                                 // Receive FIFO not empty
  #define SSP_TNF                       0x0002                                 // Transmit FIFO not full
  #define SSP_TFE                       0x0001                                 // Transmit FIFO empty
#define SSP0_PR                         *(unsigned long *)(SSP0_BLOCK + 0x10)  // Clock Prescaler Register SSP 0
  #define SSP_MAX_CLOCK                 2
#define SSP0_IMSCR                      *(unsigned long *)(SSP0_BLOCK + 0x14)  // Interrupt Mask Set and Clear Register SSP 0
  #define SSP_TXIM                      0x0008                                 // Transmit FIFO Interrupt Mask
  #define SSP_RXIM                      0x0004                                 // Receive FIFO Interrupt Mask
  #define SSP_RTIM                      0x0002                                 // Receive Timeout Interrupt Mask
  #define SSP_RORIM                     0x0001                                 // Receive Overrun Interrupt Mask
#define SSP0_RISR                       *(volatile unsigned long *)(SSP0_BLOCK + 0x18) // RAW Interrupt Status Register SSP 0
  #define SSP_TXRIS                     0x0008                                 // Transmit FIFO RAW Status Flag
  #define SSP_RXRIS                     0x0004                                 // Receive FIFO RAW Status Flag
  #define SSP_RTRIS                     0x0002                                 // Receive Timeout RAW Status Flag
  #define SSP_RORIS                     0x0001                                 // Receive Overrun RAW Status Flag
#define SSP0_MISR                       *(volatile unsigned long *)(SSP0_BLOCK + 0x1c) // Masked Interrupt Status Register SSP 0
  #define SSP_TXMIS                     0x0008                                 // Transmit FIFO Masked Status Flag
  #define SSP_RXMIS                     0x0004                                 // Receive FIFO Masked Status Flag
  #define SSP_RTMIS                     0x0002                                 // Receive Timeout Masked Status Flag
  #define SSP_ROMIS                     0x0001                                 // Receive Overrun Masked Status Flag
#define SSP0_ICR                        *(volatile unsigned long *)(SSP0_BLOCK + 0x20 ) // Interrupt Clear Register SSP 0
  #define SSP_RTIC                      0x0002                                 // Clear RX Timeout Interrupt
  #define SSP_RORIC                     0x0001                                 // Clear Rx Overrun Interrupt
#define SSP0_DMACR                      *(unsigned long *)(SSP0_BLOCK + 0x20 ) // Interrupt Clear Register SSP 0
  #define SSP_TXDMAE                    0x0002                                 // Transmit DMA Enabled
  #define SSP_RXDMAE                    0x0001                                 // Receive DMA Enabled

#define SSP1_CR_0                       *(unsigned long *)(SSP1_BLOCK + 0x00)  // Control Register 0 SSP 1
#define SSP1_CR_1                       *(unsigned long *)(SSP1_BLOCK + 0x04)  // Control Register 1 SSP 1
#define SSP1_DR                         *(volatile unsigned long *)(SSP1_BLOCK + 0x08)  // Data Register SSP 1
#define SSP1_SR                         *(volatile unsigned long *)(SSP1_BLOCK + 0x0c)  // Status Register SSP 1
#define SSP1_PR                         *(unsigned long *)(SSP1_BLOCK + 0x10)  // Clock Prescaler Register SSP 1
#define SSP1_IMSCR                      *(unsigned long *)(SSP1_BLOCK + 0x14)  // Interrupt Mask Set and Clear Register SSP 1
#define SSP1_RISR                       *(volatile unsigned long *)(SSP1_BLOCK + 0x18) // RAW Interrupt Status Register SSP 1
#define SSP1_MISR                       *(volatile unsigned long *)(SSP1_BLOCK + 0x1c) // Masked Interrupt Status Register SSP 1
#define SSP1_ICR                        *(volatile unsigned long *)(SSP1_BLOCK + 0x20 ) // Interrupt Clear Register SSP 1
#define SSP1_DMACR                      *(unsigned long *)(SSP1_BLOCK + 0x20 ) // Interrupt Clear Register SSP 1

// ADC
//
#define ADC_CR                          *(volatile unsigned long *)(ADC_BLOCK + 0x00)  // ADC Control Register
  #define STR                           0x00000001
  #define POR                           0x00000002                                     // power on / reset mode
  #define STB                           0x00000008
  #define ADC_ECV                       0x00008000
  #define ACD_CHANNEL_SHIFT             6
#define ADC_CCR                         *(unsigned long *)(ADC_BLOCK + 0x04)           // ADC Channel Control Register
  #define NO_ACD_CONVERSION             0x00
  #define LOW_THRESHOLD_WATCHDOG        0x01
  #define HIGH_THRESHOLD_WATCHDOG       0x02
  #define PURE_ADC                      0x03
#define ADC_HTR                         *(unsigned long *)(ADC_BLOCK + 0x08)           // ADC High Threshold Register
#define ADC_LTR                         *(unsigned long *)(ADC_BLOCK + 0x0c)           // ADC Low Threshold Register
#define ADC_CRR                         *(unsigned long *)(ADC_BLOCK + 0x10)           // ADC Channel Compare Result Register
#define ADC_DR0                         *(volatile unsigned long *)(ADC_BLOCK + 0x14)  // ADC Channel 0 Converted Register
#define ADC_DR1                         *(volatile unsigned long *)(ADC_BLOCK + 0x18)  // ADC Channel 1 Converted Register
#define ADC_DR2                         *(volatile unsigned long *)(ADC_BLOCK + 0x1c)  // ADC Channel 2 Converted Register
#define ADC_DR3                         *(volatile unsigned long *)(ADC_BLOCK + 0x20)  // ADC Channel 3 Converted Register
#define ADC_DR4                         *(volatile unsigned long *)(ADC_BLOCK + 0x24)  // ADC Channel 4 Converted Register
#define ADC_DR5                         *(volatile unsigned long *)(ADC_BLOCK + 0x28)  // ADC Channel 5 Converted Register
#define ADC_DR6                         *(volatile unsigned long *)(ADC_BLOCK + 0x2c)  // ADC Channel 6 Converted Register
#define ADC_DR7                         *(volatile unsigned long *)(ADC_BLOCK + 0x30)  // ADC Channel 7 Converted Register
#define ADC_PRS                         *(volatile unsigned long *)(ADC_BLOCK + 0x34)  // ADC Clock Prescaler

// Watchdog Timer
//
#define WDG_CR                          *(unsigned long *)(WDT_BLOCK + 0x00)           // Watchdog Timer Control Register
  #define WATCHDOG_EE                   0x0004                                         // Exteral clock enable
  #define WATCHDOG_SC                   0x0002                                         // Start counting
  #define WATCHDOG_WE                   0x0001                                         // Watchdog enable - can be set but not cleared
#define WDG_PR                          *(unsigned long *)(WDT_BLOCK + 0x04)           // Watchdog Timer Prescale Register
#define WDG_VR                          *(unsigned long *)(WDT_BLOCK + 0x08)           // Watchdog Timer Preload Value
#define WDG_CNT                         *(volatile unsigned long *)(WDT_BLOCK + 0x0c)  // Watchdog Timer Count register Register - read only
#define WDG_SR                          *(unsigned long *)(WDT_BLOCK + 0x10)           // Watchdog Timer Status Register
  #define WATCHDOG_EC                   0x0001                                         // End of Count reached
#define WDG_MR                          *(unsigned long *)(WDT_BLOCK + 0x14)           // Watchdog Timer Mask Register
  #define WATCHDOG_ECM                  0x0001                                         // End of count interrupt mask
#define WDG_KR                          *(volatile unsigned long *)(WDT_BLOCK + 0x18)  // Watchdog Timer Key Register
  #define WDT_FEED1                     0xa55a
  #define WDT_FEED2                     0x5aa5


// IIC
//
#define I2C0_CR                         *(volatile unsigned long *)(I2C0_BLOCK + 0x00) // IIC Control Register IIC 0
  #define IIC_ITE                       0x01                                           // IIC Interrupt enable
  #define STOP_COND                     0x02                                           // IIC Start condition generation
  #define IIC_ACK                       0x04                                           // IIC ACK generation
  #define START_COND                    0x08                                           // IIC Start condition generation
  #define IIC_PE                        0x20                                           // IIC peripheral enable

#define I2C_SR1_OFFSET                  0x04
#define I2C0_SR1                        *(volatile unsigned long *)(I2C0_BLOCK + 0x04) // IIC Status Register 1 IIC 0
  #define EVF                           0x80                                           // Event flag
  #define ADD10                         0x40                                           // Master has sent header byte
  #define TRA                           0x20                                           // Data byte transmitted
  #define IIC_BUSY                      0x10                                           // Bus Busy
  #define BTF                           0x08                                           // Byte Transfer Finished
  #define ADSL                          0x04                                           // Address Matched, slave mode
  #define IIC_M_SL                      0x02                                           // Master Mode
  #define IIC_START_BIT                 0x01                                           // Start condition generated
#define I2C0_SR2                        *(volatile unsigned long *)(I2C0_BLOCK + 0x08) // IIC Status Register 2 IIC 0
  #define ENDAD                         0x20                                           // End if address transmission
  #define IIC_AF                        0x10                                           // Acknowledge failure
  #define STOPF                         0x08                                           // Stop detection, slave mode
  #define ARLO                          0x04                                           // Arbitration lost
  #define IIC_BERR                      0x02                                           // Bus Error
  #define GCAL                          0x01                                           // General Call, slave mode
#define I2C0_CCR                        *(volatile unsigned long *)(I2C0_BLOCK + 0x0c) // IIC Clock Control Register IIC 0
  #define FM_FS                         0x80                                           // Fast I2C Mode
#define I2C0_OAR1                       *(volatile unsigned long *)(I2C0_BLOCK + 0x10) // IIC Own Address Register 1 IIC 0
#define I2C0_OAR2                       *(volatile unsigned long *)(I2C0_BLOCK + 0x14) // IIC Own Address Register 2 IIC 0
  #define FR0                           0x20
  #define FR1                           0x40
  #define FR2                           0x80

#define I2C_DR_OFFSET                   0x18
#define I2C0_DR                         *(volatile unsigned long *)(I2C0_BLOCK + 0x18) // IIC Data Register IIC 0
#define I2C0_ECCR                       *(volatile unsigned long *)(I2C0_BLOCK + 0x1c) // IIC Extended Clock Control Register IIC 0

#define I2C1_CR                         *(volatile unsigned long *)(I2C1_BLOCK + 0x00) // IIC Control Register IIC 1
#define I2C1_SR1                        *(volatile unsigned long *)(I2C1_BLOCK + 0x04) // IIC Status Register 1 IIC 1
#define I2C1_SR2                        *(volatile unsigned long *)(I2C1_BLOCK + 0x08) // IIC Status Register 2 IIC 1
#define I2C1_CCR                        *(volatile unsigned long *)(I2C1_BLOCK + 0x0c) // IIC Clock Control Register IIC 1
#define I2C1_OAR1                       *(volatile unsigned long *)(I2C1_BLOCK + 0x10) // IIC Own Address Register 1 IIC 1
#define I2C1_OAR2                       *(volatile unsigned long *)(I2C1_BLOCK + 0x14) // IIC Own Address Register 2 IIC 1
#define I2C1_DR                         *(volatile unsigned long *)(I2C1_BLOCK + 0x18) // IIC Data Register IIC 1
#define I2C1_ECCR                       *(volatile unsigned long *)(I2C1_BLOCK + 0x1c) // IIC Extended Clock Control Register IIC 1


// System Control Unit
//
#define SCU_CLKCNTR                     *(unsigned long *)(SCU_BLOCK + 0x00)   // Clock control register
  #define MCLKSEL_PLL                   0x00000000                             // Master clock from PLL
  #define MCLKSEL_RTC                   0x00000001                             // Master clock from RTC
  #define MCLKSEL_OSC                   0x00000002                             // Master clock from oscillator (default after power on)
  #define BR_SEL                        0x00000200                             // Connect BCLK to master clock
  #define USB_SEL_DIV_1                 0x00000000                             // USB clock is Master clock
  #define USB_SEL_DIV_2                 0x00000400                             // USB clock is Master clock/2
  #define USB_SEL_EXT                   0x00000800                             // USB clock from external pin
  #define PHY_SEL                       0x00001000                             // Enable 25MHz PHY clock output
  #define EMIRATION_1                   0x00000000                             // BCLK = HCLK
  #define EMIRATION_2                   0x00020000                             // BCLK = HCLK/2 (default after power on)

  #define APBDIV_1                      0x00000000                             // PCLK = RCLK
  #define APBDIV_2                      0x00000080                             // PCLK = RCLK/2
  #define APBDIV_4                      0x00000100                             // PCLK = RCLK/4
  #define APBDIV_8                      0x00000180                             // PCLK = RCLK/8

  #define SCU_BRSEL                     0x00000200                             // BRCLK / Master clock / 2
#define SCU_PLLCONF                     *(unsigned long *)(SCU_BLOCK + 0x04)   // PLL Configuration register
  #define PLL_MDIV_MASK                 0x000000ff
  #define PLL_NDIV_MASK                 0x0000ff00
  #define PLL_PDIV_MASK                 0x00070000
  #define PLL_ENABLE                    0x00080000
  #define PLL_SETTING                   ((POST_DIVIDE_EXP << 16) | (PLL_MUL << 8) | (OSC_DIVIDE))
#define SCU_SYSSTATUS                   *(volatile unsigned long *)(SCU_BLOCK + 0x08) // System Status register
  #define PLL_LOCK                      0x00000001                             // PLL locked flag
  #define LVD_RST                       0x00000008
#define SCU_PWRMNG                      *(volatile unsigned long *)(SCU_BLOCK + 0x0c) // System Status register
  #define PWR_MODE_RUN                  0x00000000                             // normal operating mode
  #define PWR_MODE_IDLE                 0x00000001                             // IDLE operating mode
  #define PWR_MODE_SLEEP                0x00000002                             // SLEEP mode
#define SCU_ITCSK                       *(volatile unsigned long *)(SCU_BLOCK + 0x10) // Interrupt Mask register
#define SCU_PCGR0                       *(volatile unsigned long *)(SCU_BLOCK + 0x14) // Peripheral Clock Gating register 0
  #define FMI                           0x00000001
  #define PQFBC                         0x00000002
  #define SRAM                          0x00000008
  #define SRAM_ARBITER                  0x00000010
  #define VIC                           0x00000020
  #define EMI                           0x00000040
  #define EXT_MEM_CLK                   0x00000080
  #define DMA                           0x00000100
  #define USB                           0x00000200
  #define USB_48M                       0x00000400
  #define MAC                           0x00000800
#define SCU_PCGR1                       *(volatile unsigned long *)(SCU_BLOCK + 0x18) // Peripheral Clock Gating register 1
  #define TIM01                         0x00000001
  #define TIM23                         0x00000002
  #define MOT_CONTROL                   0x00000004
  #define UART0                         0x00000008
  #define UART1                         0x00000010
  #define UART2                         0x00000020
  #define IIC0                          0x00000040
  #define IIC1                          0x00000080
  #define SSP0                          0x00000100
  #define SSP1                          0x00000200
  #define CAN                           0x00000400
  #define ADC                           0x00000800
  #define WATCHDOG                      0x00001000
  #define WIU                           0x00002000
  #define GPIO0                         0x00004000
  #define GPIO1                         0x00008000
  #define GPIO2                         0x00010000
  #define GPIO3                         0x00020000
  #define GPIO4                         0x00040000
  #define GPIO5                         0x00080000
  #define GPIO6                         0x00100000
  #define GPIO7                         0x00200000
  #define GPIO8                         0x00400000
  #define GPIO9                         0x00800000
  #define RTC                           0x01000000
#define SCU_PRR0                        *(volatile unsigned long *)(SCU_BLOCK + 0x1c) // Peripheral Reset Register 0
#define SCU_PRR1                        *(volatile unsigned long *)(SCU_BLOCK + 0x20) // Peripheral Reset Register 1
#define SCU_MGR0                        *(volatile unsigned long *)(SCU_BLOCK + 0x24) // Idle mode Mask Gating Register 0
#define SCU_MGR1                        *(volatile unsigned long *)(SCU_BLOCK + 0x28) // Idel mode Mask Gating Register 1
#define SCU_PECGR0                      *(volatile unsigned long *)(SCU_BLOCK + 0x2c) // Peripheral Emulation Clock Gating Register 0
#define SCU_PECGR1                      *(volatile unsigned long *)(SCU_BLOCK + 0x30) // Peripheral Emulation Clock Gating Register 2
#define SCU_SCR0                        *(volatile unsigned long *)(SCU_BLOCK + 0x34) // System Configuration Register 0
#define WSR_AHB                         0x04
#define WSR_DTCM                        0x02
#define EN_PFQBC                        0x01
#define SCU_SCR1                        *(volatile unsigned long *)(SCU_BLOCK + 0x38) // System Configuration Register 1
#define SCU_SCR2                        *(volatile unsigned long *)(SCU_BLOCK + 0x3c) // System Configuration Register 2

#define SCU_GPIOOUT0                    *(volatile unsigned long *)(SCU_BLOCK + 0x44) // General Purpose Output Register 0
#define SCU_GPIOOUT1                    *(volatile unsigned long *)(SCU_BLOCK + 0x48) // General Purpose Output Register 1
#define SCU_GPIOOUT2                    *(volatile unsigned long *)(SCU_BLOCK + 0x4c) // General Purpose Output Register 2
#define SCU_GPIOOUT3                    *(volatile unsigned long *)(SCU_BLOCK + 0x50) // General Purpose Output Register 3
#define SCU_GPIOOUT4                    *(volatile unsigned long *)(SCU_BLOCK + 0x54) // General Purpose Output Register 4
#define SCU_GPIOOUT5                    *(volatile unsigned long *)(SCU_BLOCK + 0x58) // General Purpose Output Register 5
#define SCU_GPIOOUT6                    *(volatile unsigned long *)(SCU_BLOCK + 0x5c) // General Purpose Output Register 6
#define SCU_GPIOOUT7                    *(volatile unsigned long *)(SCU_BLOCK + 0x60) // General Purpose Output Register 7
#define SCU_GPIOIN0                     *(volatile unsigned long *)(SCU_BLOCK + 0x64) // General Purpose Input Register 0
#define SCU_GPIOIN1                     *(volatile unsigned long *)(SCU_BLOCK + 0x68) // General Purpose Input Register 1
#define SCU_GPIOIN2                     *(volatile unsigned long *)(SCU_BLOCK + 0x6c) // General Purpose Input Register 2
#define SCU_GPIOIN3                     *(volatile unsigned long *)(SCU_BLOCK + 0x70) // General Purpose Input Register 3
#define SCU_GPIOIN4                     *(volatile unsigned long *)(SCU_BLOCK + 0x74) // General Purpose Input Register 4
#define SCU_GPIOIN5                     *(volatile unsigned long *)(SCU_BLOCK + 0x78) // General Purpose Input Register 5
#define SCU_GPIOIN6                     *(volatile unsigned long *)(SCU_BLOCK + 0x7c) // General Purpose Input Register 6
#define SCU_GPIOIN7                     *(volatile unsigned long *)(SCU_BLOCK + 0x80) // General Purpose Input Register 7
#define SCU_GPIOTYPE0                   *(volatile unsigned long *)(SCU_BLOCK + 0x84) // General Purpose Type Register 0
#define SCU_GPIOTYPE1                   *(volatile unsigned long *)(SCU_BLOCK + 0x88) // General Purpose Type Register 1
#define SCU_GPIOTYPE2                   *(volatile unsigned long *)(SCU_BLOCK + 0x8c) // General Purpose Type Register 2
#define SCU_GPIOTYPE3                   *(volatile unsigned long *)(SCU_BLOCK + 0x90) // General Purpose Type Register 3
#define SCU_GPIOTYPE4                   *(volatile unsigned long *)(SCU_BLOCK + 0x94) // General Purpose Type Register 4
#define SCU_GPIOTYPE5                   *(volatile unsigned long *)(SCU_BLOCK + 0x98) // General Purpose Type Register 5
#define SCU_GPIOTYPE6                   *(volatile unsigned long *)(SCU_BLOCK + 0x9c) // General Purpose Type Register 6
#define SCU_GPIOTYPE7                   *(volatile unsigned long *)(SCU_BLOCK + 0xa0) // General Purpose Type Register 7
#define SCU_GPIOTYPE8                   *(volatile unsigned long *)(SCU_BLOCK + 0xa4) // General Purpose Type Register 8
#define SCU_GPIOTYPE9                   *(volatile unsigned long *)(SCU_BLOCK + 0xa8) // General Purpose Type Register 9
#define SCU_GPIOEMI                     *(volatile unsigned long *)(SCU_BLOCK + 0xac) // 
#define SCU_WKUPSEL                     *(volatile unsigned long *)(SCU_BLOCK + 0xb0) // Wakeup selection register

#define SCU_GPIOANA                     *(volatile unsigned long *)(SCU_BLOCK + 0xbc) // GPIO Analog Mode




#ifdef _WINDOWS                                                          // {9}
    #define _POWER_UP(module)            PMC_PCSR |= module; PMC_PCER = PMC_PCDR = 0      // synchronise the status register
    #define _POWER_DOWN(module)          PMC_PCSR &= ~module; PMC_PCER = PMC_PCDR = 0
#else
    #define _POWER_UP(module)
    #define _POWER_DOWN(module)
#endif
#define POWER_UP(reg, module)            SCU_PCGR##reg |= (module); SCU_PRR##reg |= (module); _POWER_UP(module)  //  power up a module (apply clock to it)
#define POWER_DOWN(reg, module)          SCU_PCGR##reg &= ~(module); _POWER_DOWN(module) // power down a module (disable clock to it)





#define MASK_FUNCTION                   0x03
#define PORT_OUT_FUNCTION               0x01
#define ALT_OUTPUT_2_FUNCTION           0x02
#define ALT_OUTPUT_3_FUNCTION           0x03


// Vectored Interrupt Controller
//
#define VIC0_ISR                        *(unsigned long *)(VIC0_BLOCK + 0x00)  // Interrupt Status Register VIC 0
#define VIC0_FSR                        *(unsigned long *)(VIC0_BLOCK + 0x04)  // Fast Interrupt Status Register VIC 0
#define VIC0_RINTSR                     *(unsigned long *)(VIC0_BLOCK + 0x08)  // RAW Interrupt Status Register VIC 0
#define VIC0_INTSR                      *(unsigned long *)(VIC0_BLOCK + 0x0c)  // Interrupt Select Register VIC 0
#define VIC0_INTER_ADDRESS               (unsigned long *)(VIC0_BLOCK + 0x10)
#define VIC0_INTER                      *(unsigned long *)(VIC0_BLOCK + 0x10)  // Interrupt Enable Register VIC 0
#define VIC0_INTECR                     *(unsigned long *)(VIC0_BLOCK + 0x14)  // Interrupt Enable Clear Register VIC 0
#define VIC0_SWINTR                     *(unsigned long *)(VIC0_BLOCK + 0x18)  // Software Interrupt Register VIC 0
#define VIC0_SWINTCR                    *(unsigned long *)(VIC0_BLOCK + 0x1c)  // Software Interrupt Clear Register VIC 0
#define VIC0_PER                        *(unsigned long *)(VIC0_BLOCK + 0x20)  // Protection VIC 0

#define VIC0_VAR                        *(unsigned long *)(VIC0_BLOCK + 0x30)  // Current Vector Address VIC 0
#define VIC0_DVAR                       *(volatile unsigned long *)(VIC0_BLOCK + 0x34)  // Default Vector Address VIC 0

#define VIC0_VA0R                       *(unsigned long *)(VIC0_BLOCK + 0x100) // Vector Address 0 VIC 0
#define VIC0_VA1R                       *(unsigned long *)(VIC0_BLOCK + 0x104) // Vector Address 1 VIC 0
#define VIC0_VA2R                       *(unsigned long *)(VIC0_BLOCK + 0x108) // Vector Address 2 VIC 0
#define VIC0_VA3R                       *(unsigned long *)(VIC0_BLOCK + 0x10c) // Vector Address 3 VIC 0
#define VIC0_VA4R                       *(unsigned long *)(VIC0_BLOCK + 0x110) // Vector Address 4 VIC 0
#define VIC0_VA5R                       *(unsigned long *)(VIC0_BLOCK + 0x114) // Vector Address 5 VIC 0
#define VIC0_VA6R                       *(unsigned long *)(VIC0_BLOCK + 0x118) // Vector Address 6 VIC 0
#define VIC0_VA7R                       *(unsigned long *)(VIC0_BLOCK + 0x11c) // Vector Address 7 VIC 0
#define VIC0_VA8R                       *(unsigned long *)(VIC0_BLOCK + 0x120) // Vector Address 8 VIC 0
#define VIC0_VA9R                       *(unsigned long *)(VIC0_BLOCK + 0x124) // Vector Address 9 VIC 0
#define VIC0_VA10R                      *(unsigned long *)(VIC0_BLOCK + 0x128) // Vector Address 10 VIC 0
#define VIC0_VA11R                      *(unsigned long *)(VIC0_BLOCK + 0x12c) // Vector Address 11 VIC 0
#define VIC0_VA12R                      *(unsigned long *)(VIC0_BLOCK + 0x130) // Vector Address 12 VIC 0
#define VIC0_VA13R                      *(unsigned long *)(VIC0_BLOCK + 0x134) // Vector Address 13 VIC 0
#define VIC0_VA14R                      *(unsigned long *)(VIC0_BLOCK + 0x138) // Vector Address 14 VIC 0
#define VIC0_VA15R                      *(unsigned long *)(VIC0_BLOCK + 0x13c) // Vector Address 15 VIC 0

#define VIC0_VC0R_ADDRESS                (unsigned long *)(VIC0_BLOCK + 0x200)
#define VIC0_VC0R                       *(unsigned long *)(VIC0_BLOCK + 0x200) // Vector Control 0 VIC 0
#define VIC0_VC1R                       *(unsigned long *)(VIC0_BLOCK + 0x204) // Vector Control 1 VIC 0
#define VIC0_VC2R                       *(unsigned long *)(VIC0_BLOCK + 0x208) // Vector Control 2 VIC 0
#define VIC0_VC3R                       *(unsigned long *)(VIC0_BLOCK + 0x20c) // Vector Control 3 VIC 0
#define VIC0_VC4R                       *(unsigned long *)(VIC0_BLOCK + 0x210) // Vector Control 4 VIC 0
#define VIC0_VC5R                       *(unsigned long *)(VIC0_BLOCK + 0x214) // Vector Control 5 VIC 0
#define VIC0_VC6R                       *(unsigned long *)(VIC0_BLOCK + 0x218) // Vector Control 6 VIC 0
#define VIC0_VC7R                       *(unsigned long *)(VIC0_BLOCK + 0x21c) // Vector Control 7 VIC 0
#define VIC0_VC8R                       *(unsigned long *)(VIC0_BLOCK + 0x220) // Vector Control 8 VIC 0
#define VIC0_VC9R                       *(unsigned long *)(VIC0_BLOCK + 0x224) // Vector Control 9 VIC 0
#define VIC0_VC10R                      *(unsigned long *)(VIC0_BLOCK + 0x228) // Vector Control 10 VIC 0
#define VIC0_VC11R                      *(unsigned long *)(VIC0_BLOCK + 0x22c) // Vector Control 11 VIC 0
#define VIC0_VC12R                      *(unsigned long *)(VIC0_BLOCK + 0x230) // Vector Control 12 VIC 0
#define VIC0_VC13R                      *(unsigned long *)(VIC0_BLOCK + 0x234) // Vector Control 13 VIC 0
#define VIC0_VC14R                      *(unsigned long *)(VIC0_BLOCK + 0x238) // Vector Control 14 VIC 0
#define VIC0_VC15R                      *(unsigned long *)(VIC0_BLOCK + 0x23c) // Vector Control 15 VIC 0

  #define ENABLE_INTERRUPT              0x0020

  #define WATCHDOG_INT_0                0x0000                           // VIC 0.0
  #define SOFTWARE_INT_0                0x0001                           // VIC 0.1
  #define DEBUG_RX_INT_0                0x0002                           // VIC 0.2
  #define DEBUG_TX_INT_0                0x0003                           // VIC 0.3
  #define TIMER0_INT_0                  0x0004                           // VIC 0.4
  #define TIMER1_INT_0                  0x0005                           // VIC 0.5
  #define TIMER2_INT_0                  0x0006                           // VIC 0.6
  #define TIMER3_INT_0                  0x0007                           // VIC 0.7
  #define USBH_INT_0                    0x0008                           // VIC 0.8
  #define USBL_INT_0                    0x0009                           // VIC 0.9
  #define SCU_INT_0                     0x000a                           // VIC 0.10
  #define ETH_INT_0                     0x000b                           // VIC 0.11
  #define DMA_INT_0                     0x000c                           // VIC 0.12
  #define CAN_INT_0                     0x000d                           // VIC 0.13
  #define MC_INT_0                      0x000e                           // VIC 0.14
  #define ADC_INT_0                     0x000f                           // VIC 0.15


#define VIC1_ISR                        *(unsigned long *)(VIC1_BLOCK + 0x00)  // Interrupt Status Register VIC 1
#define VIC1_FSR                        *(unsigned long *)(VIC1_BLOCK + 0x04)  // Fast Interrupt Status Register VIC 1
#define VIC1_RINTSR                     *(unsigned long *)(VIC1_BLOCK + 0x08)  // RAW Interrupt Status Register VIC 1
#define VIC1_INTSR                      *(unsigned long *)(VIC1_BLOCK + 0x0c)  // Interrupt Select Register VIC 1
#define VIC1_INTER                      *(unsigned long *)(VIC1_BLOCK + 0x10)  // Interrupt Enable Register VIC 1
#define VIC1_INTER_ADDRESS               (unsigned long *)(VIC1_BLOCK + 0x10)
#define VIC1_INTECR                     *(unsigned long *)(VIC1_BLOCK + 0x14)  // Interrupt Enable Clear Register VIC 1
#define VIC1_SWINTR                     *(unsigned long *)(VIC1_BLOCK + 0x18)  // Software Interrupt Register VIC 1
#define VIC1_SWINTCR                    *(unsigned long *)(VIC1_BLOCK + 0x1c)  // Software Interrupt Clear Register VIC 1
#define VIC1_PER                        *(unsigned long *)(VIC1_BLOCK + 0x20)  // Protection VIC 1

#define VIC1_VAR                        *(unsigned long *)(VIC1_BLOCK + 0x30)  // Current Vector Address VIC 1
#define VIC1_DVAR                       *(unsigned long *)(VIC1_BLOCK + 0x34)  // Default Vector Address VIC 1

#define VIC1_VA0R_ADDRESS                (unsigned long *)(VIC1_BLOCK + 0x100)
#define VIC1_VA0R                       *(unsigned long *)(VIC1_BLOCK + 0x100) // Vector Address 0 VIC 1
#define VIC1_VA1R                       *(unsigned long *)(VIC1_BLOCK + 0x104) // Vector Address 1 VIC 1
#define VIC1_VA2R                       *(unsigned long *)(VIC1_BLOCK + 0x108) // Vector Address 2 VIC 1
#define VIC1_VA3R                       *(unsigned long *)(VIC1_BLOCK + 0x10c) // Vector Address 3 VIC 1
#define VIC1_VA4R                       *(unsigned long *)(VIC1_BLOCK + 0x110) // Vector Address 4 VIC 1
#define VIC1_VA5R                       *(unsigned long *)(VIC1_BLOCK + 0x114) // Vector Address 5 VIC 1
#define VIC1_VA6R                       *(unsigned long *)(VIC1_BLOCK + 0x118) // Vector Address 6 VIC 1
#define VIC1_VA7R                       *(unsigned long *)(VIC1_BLOCK + 0x11c) // Vector Address 7 VIC 1
#define VIC1_VA8R                       *(unsigned long *)(VIC1_BLOCK + 0x120) // Vector Address 8 VIC 1
#define VIC1_VA9R                       *(unsigned long *)(VIC1_BLOCK + 0x124) // Vector Address 9 VIC 1
#define VIC1_VA10R                      *(unsigned long *)(VIC1_BLOCK + 0x128) // Vector Address 10 VIC 1
#define VIC1_VA11R                      *(unsigned long *)(VIC1_BLOCK + 0x12c) // Vector Address 11 VIC 1
#define VIC1_VA12R                      *(unsigned long *)(VIC1_BLOCK + 0x130) // Vector Address 12 VIC 1
#define VIC1_VA13R                      *(unsigned long *)(VIC1_BLOCK + 0x134) // Vector Address 13 VIC 1
#define VIC1_VA14R                      *(unsigned long *)(VIC1_BLOCK + 0x138) // Vector Address 14 VIC 1
#define VIC1_VA15R                      *(unsigned long *)(VIC1_BLOCK + 0x13c) // Vector Address 15 VIC 1

#define VIC1_VC0R_ADDRESS                (unsigned long *)(VIC1_BLOCK + 0x200)
#define VIC1_VC0R                       *(unsigned long *)(VIC1_BLOCK + 0x200) // Vector Control 0 VIC 1
#define VIC1_VC1R                       *(unsigned long *)(VIC1_BLOCK + 0x204) // Vector Control 1 VIC 1
#define VIC1_VC2R                       *(unsigned long *)(VIC1_BLOCK + 0x208) // Vector Control 2 VIC 1
#define VIC1_VC3R                       *(unsigned long *)(VIC1_BLOCK + 0x20c) // Vector Control 3 VIC 1
#define VIC1_VC4R                       *(unsigned long *)(VIC1_BLOCK + 0x210) // Vector Control 4 VIC 1
#define VIC1_VC5R                       *(unsigned long *)(VIC1_BLOCK + 0x214) // Vector Control 5 VIC 1
#define VIC1_VC6R                       *(unsigned long *)(VIC1_BLOCK + 0x218) // Vector Control 6 VIC 1
#define VIC1_VC7R                       *(unsigned long *)(VIC1_BLOCK + 0x21c) // Vector Control 7 VIC 1
#define VIC1_VC8R                       *(unsigned long *)(VIC1_BLOCK + 0x220) // Vector Control 8 VIC 1
#define VIC1_VC9R                       *(unsigned long *)(VIC1_BLOCK + 0x224) // Vector Control 9 VIC 1
#define VIC1_VC10R                      *(unsigned long *)(VIC1_BLOCK + 0x228) // Vector Control 10 VIC 1
#define VIC1_VC11R                      *(unsigned long *)(VIC1_BLOCK + 0x22c) // Vector Control 11 VIC 1
#define VIC1_VC12R                      *(unsigned long *)(VIC1_BLOCK + 0x230) // Vector Control 12 VIC 1
#define VIC1_VC13R                      *(unsigned long *)(VIC1_BLOCK + 0x234) // Vector Control 13 VIC 1
#define VIC1_VC14R                      *(unsigned long *)(VIC1_BLOCK + 0x238) // Vector Control 14 VIC 1
#define VIC1_VC15R                      *(unsigned long *)(VIC1_BLOCK + 0x23c) // Vector Control 15 VIC 1

  #define UART0_INT_1                   0x0000                           // VIC 1.0
  #define UART1_INT_1                   0x0001                           // VIC 1.1
  #define UART2_INT_1                   0x0002                           // VIC 1.2
  #define IIC0_INT_1                    0x0003                           // VIC 1.3
  #define IIC1_INT_1                    0x0004                           // VIC 1.4
  #define SSP0_INT_1                    0x0005                           // VIC 1.5
  #define SSP1_INT_1                    0x0006                           // VIC 1.6
  #define SCU_INT_1                     0x0007                           // VIC 1.7
  #define RTC_INT_1                     0x0008                           // VIC 1.8
  #define WIU_ALL_INT_1                 0x0009                           // VIC 1.9
  #define WIU_GROUP0_INT_1              0x000a                           // VIC 1.10
  #define WIU_GROUP1_INT_1              0x000b                           // VIC 1.11
  #define WIU_GROUP2_INT_1              0x000c                           // VIC 1.12
  #define WIU_GROUP3_INT_1              0x000d                           // VIC 1.13
  #define USB_INT_1                     0x000e                           // VIC 1.14
  #define PFQ_BC_INT_1                  0x000f                           // VIC 1.15

  #define VECTOR_0                      0x0001                           // WATCHDOG/UART0
  #define VECTOR_1                      0x0002                           // SOFTWARE/UART1
  #define VECTOR_2                      0x0004                           // DEBUGRX/UART2
  #define VECTOR_3                      0x0008                           // DEBUGTX/IIC0
  #define VECTOR_4                      0x0010                           // TIMER0/IIC1
  #define VECTOR_5                      0x0020                           // TIMER1/SSP0
  #define VECTOR_6                      0x0040                           // TIMER2/SSP1
  #define VECTOR_7                      0x0080                           // TIMER3/SCU
  #define VECTOR_8                      0x0100                           // USBH/RTC
  #define VECTOR_9                      0x0200                           // USBL/WIU ALL
  #define VECTOR_10                     0x0400                           // SCU/WIU GROUP 0
  #define VECTOR_11                     0x0800                           // ETH/WIU GROUP 1
  #define VECTOR_12                     0x1000                           // DMA/WIU GROUP 2
  #define VECTOR_13                     0x2000                           // CAN/WIU GROUP 3
  #define VECTOR_14                     0x4000                           // MC/USB
  #define VECTOR_15                     0x8000                           // ADC/PFQ_BC



// Ethernet controller - MAC802.3
//
typedef struct stSTR91XF_BD
{
volatile unsigned long    bd_dma_control;
volatile unsigned char   *bd_dma_start_address;
volatile unsigned char   *bd_dma_next;
volatile unsigned long    bd_packet_status;
} STR91XF_BD;

#define TX_FRAME_ABORTED                0x00000001
#define TX_JABBER_TIMEOUT               0x00000002
#define TX_NO_CARRIER                   0x00000004
#define TX_LOSS_OF_CARRIER              0x00000008
#define TX_EXCESSIVE_DEFERRAL           0x00000010
#define TX_LATE_COLLISION               0x00000020
#define TX_EXCESSIVE_COLLISIONS         0x00000040
#define TX_UNDERRUN                     0x00000080
#define TX_DEFERRED                     0x00000100
#define TX_LATE_COLLISION_OBSERVED      0x00000200
#define TX_COLLISION_COUNT_MASK         0x00003c00
#define TX_HEARTBEAT_FAIL               0x00004000
#define TX_BD_VALID                     0x00010000
#define TX_BYTE_COUNTER_MASK            0x7ffc0000
#define TX_PACKET_RETRY                 0x80000000

#define RX_FRAME_LENGTH_MASK            0x000007ff
#define RX_FRAME_TOO_LONG               0x00002000
#define RX_WATCHDOG_TIMEOUT             0x00004000
#define RX_RUNT_FRAME                   0x00008000
#define RX_VALID_MASK                   0x00010000
#define RX_LATE_COLLISION               0x00020000
#define RX_FRAME_TYPE                   0x00040000
#define RX_MII_ERROR                    0x00080000
#define RX_DRIBBLING_BITS               0x00100000
#define RX_CRC_ERROR                    0x00200000
#define RX_ONE_LEVEL_VLAN               0x00400000
#define RX_TWO_LEVEL_VLAN               0x00800000
#define RX_LENGTH_ERROR                 0x01000000
#define RX_CONTROL_FRAME                0x02000000
#define RX_UNSUPPORTED_CONTROL_FRAME    0x04000000
#define RX_MULTICAST_FRAME              0x08000000
#define RX_BROADCAST_FRAME              0x10000000
#define RX_FILTERING_FAIL               0x20000000
#define RX_PACKET_FILTER                0x40000000
#define RX_MISSING_FRAME                0x80000000

#define ENET_SCR                        *(unsigned long *)(ENET_BLOCK + 0x000) // DMA Status Control Register
  #define SRESET                        0x00000001                             // Software reset
  #define LOOPB                         0x00000002                             // loopback mode
  #define RX_MAX_BURST_SIZE_SINGLE      0x00000030                             // single  transfers only
  #define TX_MAX_BURST_SIZE_SINGLE      0x000000c0                             // single  transfers only
#define ENET_IER                        *(unsigned long *)(ENET_BLOCK + 0x004) // DMA Interrupt Enable Register
#define ENET_ISR                        *(unsigned long *)(ENET_BLOCK + 0x008) // DMA Interrupt Status Register
  #define RX_EMPTY_EN                   0x00000001
  #define RX_FULL_EN                    0x00000002
  #define RX_ENTRY_EN                   0x00000004
  #define RX_TO_EN                      0x00000008
  #define PACKET_LOSS_EN                0x00000020
  #define RX_NEXT_EN                    0x00000040
  #define RX_DONE_EN                    0x00000080
  #define RX_MERR_INT_EN                0x00000200
  #define RX_CURR_DONE_EN               0x00008000
  #define TX_EMPTY_EN                   0x00010000
  #define TX_FULL_EN                    0x00020000
  #define TX_ENTRY_EN                   0x00040000
  #define TX_TO_EN                      0x00080000
  #define TX_NEXT_EN                    0x00400000
  #define TX_DONE_EN                    0x00800000
  #define TX_MERR_INT_EN                0x02000000
  #define MAC802_3_INT_EN               0x1000000
  #define TX_CURR_DONE_EN               0x80000000
#define ENET_CCR                        *(unsigned long *)(ENET_BLOCK + 0x00c) // DMA Clock Configuration Register
  #define HCLK_EQUALS_PCLK              0x00000000
  #define HCLK_DOUBLE_PCLK              0x00000004
#define ENET_RXSTR                      *(unsigned long *)(ENET_BLOCK + 0x010) // RX Start Register
  #define RX_DMA_ENABLE                 0x00000001                             // write '1' to cause abort
  #define DFETCH_DELAY                  0x00010000
#define ENET_RXCR                       *(unsigned long *)(ENET_BLOCK + 0x014) // RX Control Register
  #define DMA_XFERCOUNT                 0x00000fff
  #define NXT_EN                        0x00004000                             // Next descriptor Fetch Mode Enable
#define ENET_RXSAR                      *(unsigned long *)(ENET_BLOCK + 0x018) // RX Start Address Register
#define ENET_RXNDAR                     *(unsigned long *)(ENET_BLOCK + 0x01c) // RX Next Descriptor Address
#define ENET_RXCAR                      *(unsigned long *)(ENET_BLOCK + 0x020) // RX Current Address Register
#define ENET_RXCTCR                     *(unsigned long *)(ENET_BLOCK + 0x024) // RX Current Transfer Count Register
#define ENET_RXTOR                      *(unsigned long *)(ENET_BLOCK + 0x028) // RX Time-Out Register
#define ENET_RXSR                       *(unsigned long *)(ENET_BLOCK + 0x02c) // RX Status Register
#define ENET_TXSTR                      *(unsigned long *)(ENET_BLOCK + 0x030) // TX Start Register
  #define TX_DMA_ENABLE                 0x00000001                             // write '1' to cause abort
  #define ENET_TX_UNDERRUN              0x00000020                             // Underrun enable
  #define START_FETCH                   0x00000004                             // Start DMA BD fetching
#define ENET_TXCR                       *(unsigned long *)(ENET_BLOCK + 0x034) // TX Control Register
#define ENET_TXSAR                      *(unsigned long *)(ENET_BLOCK + 0x038) // TX Start Address Register
#define ENET_TXNDAR                     *(unsigned long *)(ENET_BLOCK + 0x03c) // TX Next Descriptor Address
  #define NPOL_EN                       0x00000001                             // Next descriptor polling enable
#define ENET_TXCAR                      *(unsigned long *)(ENET_BLOCK + 0x040) // TX Current Address Register
#define ENET_TXCTCR                     *(unsigned long *)(ENET_BLOCK + 0x044) // TX Current Transfer Count Register
#define ENET_TXTOR                      *(unsigned long *)(ENET_BLOCK + 0x048) // TX Time-Out Register
#define ENET_TXSR                       *(unsigned long *)(ENET_BLOCK + 0x04c) // TX Status Register
#define ENET_RX_FIFO_START               (unsigned long *)(ENET_BLOCK + 0x100)
#define ENET_TX_FIFO_START               (unsigned long *)(ENET_BLOCK + 0x200)
#define ENET_MCR                        *(unsigned long *)(ENET_BLOCK + 0x400) // MAC Control Register
  #define MCR_RCFA                      0x00000001                             // reverse control frame address
  #define MCR_RE                        0x00000004                             // reception enable
  #define MCR_TE                        0x00000008                             // transmission enable
  #define MCR_RVBE                      0x00000010                             // enable VCI Rx interface to use burst writes
  #define MCR_DCE                       0x00000020                             // Deferral check enable
  #define BL_10                         0x00000000                             // Backoff limit counter time slot counter initialised with 10
  #define BL_8                          0x00000040                             // Backoff limit counter time slot counter initialised with 8
  #define BL_6                          0x00000080                             // Backoff limit counter time slot counter initialised with 6
  #define BL_3                          0x000000c0                             // Backoff limit counter time slot counter initialised with 3
  #define MCR_APR                       0x00000100                             // Automatic Pad Removal
  #define MCR_RVFF                      0x00000200                             // VCI Rx Frame filtering
  #define MCR_DPR                       0x00000400                             // Disable Packet Retry
  #define MCR_DBF                       0x00000800                             // Disable Broadcast Frame
  #define MCR_ELC                       0x00001000                             // Enable Late Collision
  #define MCR_VFM                       0x00008000                             // VLAN Filtering Mode
  #define MCR_PWF                       0x00010000                             // Pass Wrong Frame
  #define MCR_AFM                       0x000e0000                             // Disable Broadcast Frame
  #define MCR_FDM                       0x00100000                             // Full Duplex Mode
  #define MCR_LM                        0x00200000                             // Loopback Mode
  #define MCR_DRO                       0x00800000                             // Disable Receive Own
  #define MCR_PRESCALE                  0x01000000                             // prescale required for master clocks of greater than 50MHz
  #define ENDIANITY_BIG                 0x40000000                             
  #define RECEIVE_ALL                   0x80000000                             // Receive all bit
#define ENET_MAH                        *(unsigned long *)(ENET_BLOCK + 0x404) // MAC Address High Register
#define ENET_MAL                        *(unsigned long *)(ENET_BLOCK + 0x408) // MAC Address Low Register
#define ENET_MCHA                       *(unsigned long *)(ENET_BLOCK + 0x40c) // Multicast Address High Register
#define ENET_MCLA                       *(unsigned long *)(ENET_BLOCK + 0x410) // Multicast Address Low Register
#define ENET_MIIA                       *(volatile unsigned long *)(ENET_BLOCK + 0x414) // MII Address Register
  #define PHY_BUSY                       0x00000001
  #define PHY_READ                       0x00000000
  #define PHY_WRITE                      0x00000002
  #define READ_FROM_PHY (PHY_READ  | (PHY_ADDRESS_ << 11) | PHY_BUSY)
  #define WRITE_TO_PHY  (PHY_WRITE | (PHY_ADDRESS_ << 11) | PHY_BUSY)
#define ENET_MIID                       *(volatile unsigned long *)(ENET_BLOCK + 0x418) // MII Data Register
#define ENET_MCF                        *(unsigned long *)(ENET_BLOCK + 0x41c) // MII Control Frame Register
#define ENET_VL1                        *(unsigned long *)(ENET_BLOCK + 0x420) // VLAN1 Register
#define ENET_VL2                        *(unsigned long *)(ENET_BLOCK + 0x424) // VLAN2 Register
#define ENET_MTS                        *(unsigned long *)(ENET_BLOCK + 0x428) // MAC Transmission Status Register
#define ENET_MRS                        *(unsigned long *)(ENET_BLOCK + 0x42c) // MAC Reception Status Register


// STE100P PHYSICAL
//
#define PHY_BASIC_CONTROL_REGISTER                 (0x0 << 10)
  #define PHY_DISABLE_TRANSMITTER                     0x0001
  #define PHY_ENABLE_COLLISION_TEST                   0x0080
  #define PHY_FULL_DUPLEX_MODE                        0x0100
  #define PHY_RESTART_AUTO_NEGOTIATION                0x0200
  #define PHY_ELECTRICALLY_ISOLATE                    0x0400
  #define PHY_POWER_DOWN_MODE                         0x0800
  #define PHY_ENABLE_AUTO_NEGOTIATION                 0x1000
  #define PHY_SELECT_100M_SPEED                       0x2000
  #define PHY_LOOP_BACK                               0x4000
  #define PHY_SOFTWARE_RESET                          0x8000
#define PHY_BASIC_STATUS_REGISTER                  (0x1 << 10)
  #define PHY_SUPPORT_EXTENDED_CAPABILITIES           0x0001
  #define PHY_JABBER_DETECTED                         0x0002
  #define PHY_LINK_IS_UP                              0x0004
  #define PHY_AUTO_NEGOTIATION_CAPABLE                0x0008
  #define PHY_REMOTE_FAULT_DETECTED                   0x0010
  #define PHY_AUTO_NEGOTIATION_COMPLETE               0x0020
  #define PHY_NO_PREAMBLE                             0x0040
  #define PHY_10MEG_HALF_DUPLEX                       0x0800
  #define PHY_10MEG_FULL_DUPLEX                       0x1000
  #define PHY_100MEG_HALF_DUPLEX                      0x2000
  #define PHY_100MEG_FULL_DUPLEX                      0x4000
  #define PHY_100MEG_T4_CAPABLE                       0x8000
#define PHY_IDENTIFIER_1                           (0x2  << 6)
#define PHY_IDENTIFIER_2                           (0x3  << 6)
#define PHY_AUTO_NEG_ADVERT_REG                    (0x4  << 6)
#define PHY_AUTO_NEG_LINK_PARTNER_ABIL_REG         (0x5  << 6)
#define PHY_AUTO_NEG_EXPANSION_REG                 (0x6  << 6)
#define PHY_INTERRUPT_CONTROL_STATUS_REGISTER      (17   << 6)
  #define PHY_SPEED_IS_100                            0x0200
  #define PHY_MODE_IS_FULL_DUPLEX                     0x0100
  #define PHY_PAUSE_FUNCTION_IS_ENABLED               0x0080
  #define PHY_ANC_HAS_COMPLETED                       0x0040
  #define PHY_RFD_IS_DETECTED                         0x0020
  #define PHY_LINK_IS_DOWN                            0x0010
  #define PHY_LINK_CODE_WORD_IS_RECEIVED              0x0008
  #define PHY_PARALLEL_DETECTION_IS_FAULT             0x0004
  #define PHY_AUT_NEG_PAGE_IS_RECEIVED                0x0002
  #define PHY_64_ERROR_PACKETS_ARE_RECEIVED           0x0001
#define PHY_INTERRUPT_ENABLE_REGISTER              (18   << 6)
  #define PHY_AN_COMPLETE_INTERRUPT_ENABLE            0x0040
  #define PHY_REMOTE_FAULT_INTERRUPT_ENABLE           0x0020
  #define PHY_LINK_DOWN_INTERRUPT_ENABLE              0x0010
  #define PHY_AN_ACK_INTERRUPT_ENABLE                 0x0008
  #define PHY_FAULT_PARALLEL_INTERRUPT_ENABLE         0x0004
  #define PHY_AN_PAGE_INTERRUPT_ENABLE                0x0002
  #define PHY_RX_ERR_64_INTERRUPT_ENABLE              0x0001
#define PHY_100BASETX_CONTROL_STATUS_REGISTER      (19   << 6)
#define PHY_MODE_CONTROL_REGISTER                  (20   << 6)
//#define PHY_AUTO_NEG_NEXT_PAGE_REG               (0x7  << 6) // STE100P doesn't support this
//#define PHY_LINK_PARTNER_NEXT_PAGE_ABIL          (0x8  << 6) // STE100P doesn't support this
//#define PHY_RXER_COUNTER_REGISTER                (0x15 << 6) // STE100P doesn't support this
//#define PHY_INTERRUPT_CONTROL_STATUS_REGISTER    (0x1b << 6) // STE100P doesn't support this
//#define PHY_LINK_UP_INT_ENABLE                      0x0100
//#define PHY_REMOTE_FAULT_INT_ENABLE                 0x0200
//#define PHY_LINK_DOWN_INT_ENABLE                    0x0400
//#define PHY_LINK_PARTNER_ACK_INT_ENABLE             0x0800
//#define PHY_PARALLEL_DETECT_FAULT_INT_ENABLE        0x1000
//#define PHY_PAGE_RECEIVED_INT_ENABLE                0x2000
//#define PHY_RECEIVE_ERROR_INT_ENABLE                0x4000
//#define PHY_JABBER_INT_ENABLE                       0x8000
//#define PHY_LINK_UP_INT                             0x0001
//#define PHY_REMOTE_FAULT_INT                        0x0002
//#define PHY_LINK_DOWN_INT                           0x0004
//#define PHY_LINK_PARTNER_ACK_INT                    0x0008
//#define PHY_PARALLEL_DETECT_FAULT_INT               0x0010
//#define PHY_PAGE_RECEIVED_INT                       0x0020
//#define PHY_RECEIVE_ERROR_INT                       0x0040
//#define PHY_JABBER_INT                              0x0080
//#define PHY_100BASETX_PHY_CONTROL_REGISTER         (0x1f << 10) // STE100P doesn't support this



// Software references used when setting up interrupts
//
#define PORT_INTERRUPT            1
#define PORT_CHANNEL_INTERRUPT    2
#define TIMER_INTERRUPT           3
#define TIMER_FREQUENCY           4

#define WUI_FALLING_EDGE          0
#define WUI_RISING_EDGE           1

#define IRQ_RISING_EDGE           WUI_RISING_EDGE                        // for general project compatibility
#define IRQ_FALLING_EDGE          WUI_FALLING_EDGE

#define EXINT_2                   2
#define EXINT_3                   3
#define EXINT_4                   4
#define EXINT_5                   5
#define EXINT_6                   6
#define EXINT_7                   7
#define EXINT_8                   8
#define EXINT_9                   9
#define EXINT_10                 10
#define EXINT_11                 11
#define EXINT_12                 12
#define EXINT_13                 13
#define EXINT_14                 14
#define EXINT_15                 15
#define EXINT_16                 16
#define EXINT_17                 17
#define EXINT_18                 18
#define EXINT_19                 19
#define EXINT_20                 20
#define EXINT_21                 21
#define EXINT_22                 22
#define EXINT_23                 23
#define EXINT_24                 24
#define EXINT_25                 25
#define EXINT_26                 26
#define EXINT_27                 27
#define EXINT_28                 28
#define EXINT_29                 29
#define EXINT_30                 30
#define EXINT_31                 31


// Define interrupt setup structures to suit this processor
//
typedef struct stINTERRUPT_SETUP
{
    void ( *int_handler )( void );                                       // interrupt handler to be configured
    unsigned char    int_type;                                           // identifier for when configuring
    unsigned char    int_priority;                                       // priority the user wants to set

    unsigned char    int_port_bit;                                       // the input number (0..30)
    unsigned char    int_port_sense;                                     // level sensitive, falling, rising, both
} INTERRUPT_SETUP;

typedef struct stTIMER_INTERRUPT_SETUP
{
    void ( *int_handler )( void );                                       // interrupt handler to be configured
    unsigned char    int_type;                                           // identifier for when configuring
    unsigned char    int_priority;                                       // priority the user wants to set

    unsigned char    timer_reference;                                    // hardware timer to use
    unsigned short   timer_value;                                        // the ms delay value
    unsigned short   timer_us_value;                                     // us value when delay is under 1ms
} TIMER_INTERRUPT_SETUP;

#ifdef CHIP_80_PIN
    #define PORTS_AVAILABLE 7
#else
    #define PORTS_AVAILABLE 10
#endif

#define PORT_WIDTH          8
