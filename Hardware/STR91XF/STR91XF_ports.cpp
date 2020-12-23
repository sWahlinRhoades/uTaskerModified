/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:        STR91XF_ports.c
   Project:     Single Chip Embedded Internet
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2011
   *********************************************************************/

#include "config.h"

#ifdef _STR91XF

#define BUF1SIZE 100

#if _VC80_UPGRADE>=0x0600
  #define STRCPY(a, b) strcpy_s (a, BUF1SIZE, b)
  #define SPRINTF(a, b, c) sprintf_s(a, BUF1SIZE, b, c)
  #define STRCAT(a, b) strcat_s(a, BUF1SIZE, b)
#else
  #define STRCPY strcpy
  #define SPRINTF sprintf
  #define STRCAT strcat
#endif

#ifdef CHIP_80_PIN
static const char *cPinNumber[PORTS_AVAILABLE][PORT_WIDTH] = {
    // GPIO 0 (0..7)
    { "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA"},
    // GPIO 1 (0..7)
    { "NA", "NA", "NA", "NA", "NA", "NA", "NA", "NA"},
    // GPIO 2 (0..7)
    { "7 {P2.0/UART0_CTS/I2C0_CLK/ETM_PCK0}", "8 {P2.1/UART0_DSR/I2C0_DATA/ETM_PCK1}", "21 {P2.2/UART0_DCD/I2C1_CLK/ETM_PCK2}", "22 {P2.3/UART0_RI/I2C1_DATA/ETM_PCK3}", "23 {P2.4/EXTCLK_T0T1/SSP0_SCLK/ETM_PSTAT0}", "29 {P2.5/EXTCLK_T2T3/SSP0_MOSI/ETM_PSTAT1}", "32 {P2.6/SSP0_MISO/ETM_PSTAT2}", "33 {P2.7/USB_CLK48M/SSP0_NSS/ETM_TRSYNC}"},
    // GPIO 3 (0..7)
    { "34 {P3.0/DMA_RQST0/UART0_RX/UART2_TX/TIM0_OCMP1}", "37 {P3.1/DMA_RQST1/UART2_RX/UART0_TX/TIM1_OCMP1}", "38 {P3.2/EXINT2/UART1_RX/CAN_TX/UART0_DTR}", "39 {P3.3/EXINT3/CAN_RX/UART1_TX/UART0_RTS}", "40 {P3.4/EXINT4/SSP1_SCLK/UART0_TX}", "41 {P3.5/EXINT5/SSP1_MOSI/UART2_TX}", "42 {P3.6/EXINT6/SSP1_MISO/CAN_TX}", "43 {P3.7/EXINT7/SSP1_NSS/TIM1_OCMP1}"},
    // GPIO 4 (0..7)
    { "4 {P4.0/ADC0/TIM0_ICAP1/TIM0_OCMP1/ETM_PCK0}", "3 {P4.1/ADC1/TIM0_ICAP2/TIM0_OCMP2/ETM_PCK1}", "2 {P4.2/ADC2/TIM1_ICAP1/TIM1_OCMP1/ETM_PCK2}", "1 {P4.3/ADC3/TIM1_ICAP2/TIM1_OCMP2/ETM_PCK3}",  "80 {P4.4/ADC4/TIM2_ICAP1/TIM2_OCMP1/ETM_PSTAT0}", "79 {P4.5/ADC5/TIM2_ICAP2/TIM2_OCMP2/ETM_PSTAT1}", "78 {P4.6/ADC6/TIM3_ICAP1/TIM3_OCMP1/ETM_PSTAT2}",  "77 {P4.7/ADC7/TIM3_ICAP2/TIM3_OCMP2/ETM_TRSYNC}"},
    // GPIO 5 (0..7)
    { "9 {P5.0/EXINT8/CAN_RX/ETM_TRCLK/UART0_TX}", "12 {P5.1/EXINT9/UART0_RX/CAN_TX/UART2_TX}", "17 {P5.2/EXINT10/UART2_RX/MII_PHYCLK/TIM3_OCMP1}", "18 {P5.3/EXINT11/ETM_EXTRIG/MII_TX_EN/TIM2_OCMP1}", "44 {P5.4/EXINT12/SSP0_SCLK/EMI_CS0}", "47 {P5.5/EXINT13/SSP0_MOSI/EMI_CS1}", "48 {P5.6/EXINT14/SSP0_MISO/EMI_CS2}", "49 {P5.7/EXINT15/SSP0_NSS/EMI_CS3}"},
    // GPIO 6 (0..7)
    { "19 {P6.0/EXINT16/TIM0_ICAP1/TIM0_OCMP1/MC_UH}", "20 {P6.1/EXINT17/TIM0_ICAP2/TIM0_OCMP2/MC_UL}", "13 {P6.2/EXINT18/TIM1_ICAP1/TIM1_OCMP1/MC_VH}", "14 {P6.3/EXINT19/TIM1_ICAP2/TIM1_OCMP2/MC_VL}", "52 {P6.4/EXINT20/TIM2_ICAP1/TIM2_OCMP1/MC_WH}", "53 {P6.5/EXINT21/TIM2_ICAP2/TIM2_OCMP2/MC_WL}", "57 {P6.6/EXINT22_TRIG/UART0_RX/TIM3_OCMP1/ETM_TRCLK}", "58 {P6.7/EXINT23_STOP/ETM_EXTRIG/TIM3_OCMP2/UART0_TX}"}
};
#else
static const char *cPinNumber[PORTS_AVAILABLE][PORT_WIDTH] = {
    // GPIO 0 (0..7)
    { "67-L11 {P0.0/MII_TX_CLK/I2C0_CLK/ETM_PCK0}", "69-K10 {P0.1/I2C0_DATA/ETM_PCK1}", "71-J11 {P0.2/MII_RXD0/I2C1_CLK/ETM_PCK2}", "76-H12 {P0.3/MII_RXD1/I2C1_DATA/ETM_PCK3}", "78-H10 {P0.4/MII_RXD2/TIM0_ICAP1/EMI_CS0/ETM_PSTAT0}", "85-F11 {P0.5/MII_RXD3/TIM0_ICAP2/EMI_CS1/ETM_PSTAT1}", "88-E11 {P0.6/MII_RX_CLK/TIM2_ICAP1/EMI_CS2/ETM_PSTAT2}", "90-B12 {P0.7/MII_RX_DV/TIM2_ICAP2/EMI_CS3/ETM_TRSYNC}"},    
    // GPIO 1 (0..7)
    { "98-B10 {P1.0/MII_RX_ER/ETM_EXTRIG/UART1_TX/SSP1_SCLK}", "99-C10 {P1.1/UART1_RX/MII_TXD0/SSP1_MOSI}", "101-B9 {P1.2/SSP1_MISO/MII_TXD1/UART0_TX}", "106-C8 {P1.3/UART2_RX/MII_TXD2/SSP1_NSS}", "109-B7 {P1.4/I2C0_CLK/MII_TXD3}", "110-A7 {P1.5/MII_COL/CAN_RX/UART2_TX/ETM_TRCLK}", "114-F7 {P1.6/MII_CRS/I2C0_DATA/CAN_TX}", "116-D6 {P1.7/ETM_EXTRIG/MII_MDC/ETM_TRCLK"},
    // GPIO 2 (0..7)
    { "10-E2 {P2.0/UART0_CTS/I2C0_CLK/ETM_PCK0}", "11-E3 {P2.1/UART0_DSR/I2C0_DATA/ETM_PCK1}", "33-M1 {P2.2/UART0_DCD/I2C1_CLK/ETM_PCK2}", "35-K3 {P2.3/UART0_RI/I2C1_DATA/ETM_PCK3}", "37-L4 {P2.4/EXTCLK_T0T1/SSP0_SCLK/ETM_PSTAT0}", "45-J5 {P2.5/EXTCLK_T2T3/SSP0_MOSI/ETM_PSTAT1}", "53-G6 {P2.6/SSP0_MISO/ETM_PSTAT2}", "54-L7 {P2.7/USB_CLK48M/SSP0_NSS/ETM_TRSYNC}"},
    // GPIO 3 (0..7)
    { "55-K7 {P3.0/DMA_RQST0/UART0_RX/UART2_TX/TIM0_OCMP1}", "59-M10 {P3.1/DMA_RQST1/UART2_RX/UART0_TX/TIM1_OCMP1}", "60-M11 {P3.2/EXINT2/UART1_RX/CAN_TX/UART0_DTR}", "61-J8 {P3.3/EXINT3/CAN_RX/UART1_TX/UART0_RTS}", "63-L9 {P3.4/EXINT4/SSP1_SCLK/UART0_TX}", "65-L10 {P3.5/EXINT5/SSP1_MOSI/UART2_TX}", "66-M12 {P3.6/EXINT6/SSP1_MISO/CAN_TX}", "68-K11 {P3.7/EXINT7/SSP1_NSS/TIM1_OCMP1}"},
    // GPIO 4 (0..7)
    { "3-C2 {P4.0/ADC0/TIM0_ICAP1/TIM0_OCMP1/ETM_PCK0}", "2-B2 {P4.1/ADC1/TIM0_ICAP2/TIM0_OCMP2/ETM_PCK1}", "1-A1 {P4.2/ADC2/TIM1_ICAP1/TIM1_OCMP1/ETM_PCK2}", "128-B3 {P4.3/ADC3/TIM1_ICAP2/TIM1_OCMP2/ETM_PCK3}",  "127-C4 {P4.4/ADC4/TIM2_ICAP1/TIM2_OCMP1/ETM_PSTAT0}", "126-B4 {P4.5/ADC5/TIM2_ICAP2/TIM2_OCMP2/ETM_PSTAT1}", "125-A4 {P4.6/ADC6/TIM3_ICAP1/TIM3_OCMP1/ETM_PSTAT2}",  "124-D5 {P4.7/ADC7/TIM3_ICAP2/TIM3_OCMP2/ETM_TRSYNC}"},
    // GPIO 5 (0..7)
    { "12-E4 {P5.0/EXINT8/CAN_RX/ETM_TRCLK/UART0_TX}", "18-F6 {P5.1/EXINT9/UART0_RX/CAN_TX/UART2_TX}", "25-K1 {P5.2/EXINT10/UART2_RX/MII_PHYCLK/TIM3_OCMP1}", "27-H2 {P5.3/EXINT11/ETM_EXTRIG/MII_TX_EN/TIM2_OCMP1}", "70-J12 {P5.4/EXINT12/SSP0_SCLK/EMI_CS0}", "77-H11 {P5.5/EXINT13/SSP0_MOSI/EMI_CS1}", "79-H9 {P5.6/EXINT14/SSP0_MISO/EMI_CS2}", "80-G12 {P5.7/EXINT15/SSP0_NSS/EMI_CS3}"},
    // GPIO 6 (0..7)
    { "29-H4 {P6.0/EXINT16/TIM0_ICAP1/TIM0_OCMP1/MC_UH}", "31-J3 {P6.1/EXINT17/TIM0_ICAP2/TIM0_OCMP2/MC_UL}", "19-G2 {P6.2/EXINT18/TIM1_ICAP1/TIM1_OCMP1/MC_VH}", "20-G3 {P6.3/EXINT19/TIM1_ICAP2/TIM1_OCMP2/MC_VL}", "83-G8 {P6.4/EXINT20/TIM2_ICAP1/TIM2_OCMP1/MC_WH}", "84-G7 {P6.5/EXINT21/TIM2_ICAP2/TIM2_OCMP2/MC_WL}", "92-E9 {P6.6/EXINT22_TRIG/UART0_RX/TIM3_OCMP1/ETM_TRCLK}", "93-D12 {P6.7/EXINT23_STOP/ETM_EXTRIG/TIM3_OCMP2/UART0_TX}"},
    // GPIO 7 (0..7)
    { "5-D1 {P7.0/EXINT24/TIM0_ICAP1/EMI_A0_16/ETM_PCK0}", "6-D2 {P7.1/EXINT25/TIM0_ICAP2/EMI_A1_17/ETM_PCK1}", "7-B1 {P7.2/EXINT26/TIM2_ICAP1/EMI_A2_18/ETM_PCK2}", "13-F1 {P7.3/EXINT27/TIM2_ICAP2/EMI_A3_19/ETM_PCK3}", "14-G1 {P7.4/EXINT28/UART0_RX/EMI_A4_20/EMI_CS3}", "15-E5 {P7.5/EXINT29/ETM_EXTRIG/EMI_A5_21/EMI_CS2}", "118-E6 {P7.6/EXINT30/TIM3_ICAP1/EMI_A6_22/EMI_CS1}", "119-A5 {P7.7/EXINT31/TIM3_ICAP2/EMI_CS0/EMI_A7_23}"},


    // GPIO 8 (0..7)
    { "26-L1 {P8.0/EMI_D0_AD0}", "28-H3 {P8.1/EMI_D1_AD1}", "30-J2 {P8.2/EMI_D2_AD2}", "32-K2 {P8.3/EMI_D3_AD3}", "34-L3 {P8.4/EMI_D4_AD4}", "36-M4 {P8.5/EMI_D5_AD5}", "38-M2 {P8.6/EMI_D6_AD6}", "44-K5 {P8.7/EMI_D7_AD7}"},


    // GPIO 9 (0..7)
    { "46-M6 {P9.0/EMI_A8_AD8}", 
    "47-M7 {P9.1/EMI_A9_AD9}", 
    "50-K6 {P9.2/EMI_A10_AD10}", 
    "51-J6 {P9.3/EMI_A11_AD11}", 
    "52-H6 {P9.4/EMI_A12_AD12}", 
    "58-L8 {P9.5/EMI_A13_AD13}", 
    "62-M9 {P9.6/EMI_A14_AD14}", 
    "64-K9 {P9.7/EMI_A15_AD15}"},
};
#endif


static const char *cPer[PORTS_AVAILABLE][PORT_WIDTH][5] = {
    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "MII_TX_CLK","I2C0_CLKIN",  "I2C0_CLKOUT","ETM_PCK0"    },     // GPIO 0
        {  "Q", "INVALID",   "I2C0_DIN",    "I2C0_DOUT",  "ETM_PCK1"    },
        {  "Q", "MII_RXD0",  "I2C1_CLKIN",  "I2C1_CLKOUT","ETM_PCK2"    },
        {  "Q", "MII_RXD1",  "I2C1_DIN",    "I2C1_DOUT",  "ETM_PCK3"    },
        {  "Q", "MII_RXD2",  "TIM0_ICAP1",  "EMI_CS0",    "ETM_PSTAT0"  },
        {  "Q", "MII_RXD3",  "TIM0_ICAP2",  "EMI_CS1",    "ETM_PSTAT1"  },
        {  "Q", "MII_RX_CLK","TIM2_ICAP1",  "EMI_CS2",    "ETM_PSTAT2"  },
        {  "Q", "MII_RX_DV", "TIM2_ICAP2",  "EMI_CS3",    "ETM_TRSYNC"  }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "MII_RX_ER", "ETM_EXTRIG",  "UART1_TX",   "SSP1_SCLK"   },     // GPIO 1
        {  "Q", "INVALID",   "UART1_RX",    "MII_TXD0",   "SSP1_MOSI"   },
        {  "Q", "INVALID",   "SSP1_MISO",   "MII_TXD1",   "UART0_TX"    },
        {  "Q", "INVALID",   "UART2_RX",    "MII_TXD2",   "SSP1_NSS"    },
        {  "Q", "INVALID",   "I2C0_CLKIN",  "MII_TXD3",   "I2C0_CLKOUT" },
        {  "Q", "MII_COL",   "CAN_RX",      "UART2_TX",   "ETM_TRCLK"   },
        {  "Q", "MII_CRS",   "I2C0_DIN",    "CAN_TX",     "I2C0_DOUT"   },
        {  "Q", "INVALID",   "ETM_EXTRIG",  "MII_MDC",    "ETM_TRCLK"   }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "UART0_CTS", "I2C0_CLKIN",  "I2C0_CLKOUT","ETM_PCK0"    },     // GPIO 2
        {  "Q", "UART0_DSR", "I2C0_DIN",    "I2C0_DOUT",  "ETM_PCK1"    },
        {  "Q", "UART0_DCD", "I2C1_CLKIN",  "I2C1_CLKOUT","ETM_PCK2"    },
        {  "Q", "UART0_RI",  "I2C1_DIN",    "I2C1_DOUT",  "ETM_PCK3"    },
        {  "Q", "EXTCLK_T0T1","SSP0_SCLK",  "SSP0_SCLK",  "ETM_PSTAT0"  },
        {  "Q", "EXTCLK_T2T3","SSP0_MOSI",  "SSP0_MOSI",  "ETM_PSTAT1"  },
        {  "Q", "INVALID",   "SSP0_MISO",   "SSP0_MISO",  "ETM_PSTAT2"  },
        {  "Q", "USB_CLK48M","SSP0_NSS",    "SSP0_NSS",   "ETM_TRSYNC"  }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "DMA_RQST0", "UART0_RX",    "UART2_TX",   "TIM0_OCMP1"  },     // GPIO 3
        {  "Q", "DMA_RQST1", "UART2_RX",    "UART0_TX",   "TIM1_OCMP1"  },
        {  "EXINT2", "EXINT2",    "UART1_RX",    "CAN_TX",     "UART0_DTR"   },
        {  "EXINT3", "EXINT3",    "CAN_RX",      "UART1_TX",   "UART0_RTS"   },
        {  "EXINT4", "EXINT4",    "SSP1_SCLK",   "SSP1_SCLK",  "UART0_TX"    },
        {  "EXINT5", "EXINT5",    "SSP1_MOSI",   "SSP1_MOSI",  "UART2_TX"    },
        {  "EXINT6", "EXINT6",    "SSP1_MISO",   "SSP1_MISO",  "CAN_TX"      },
        {  "EXINT7", "EXINT7",    "SSP1_NSS",    "SSP1_NSS",   "TIM1_OCMP1"  }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "ADC0",      "TIM0_ICAP1",  "TIM0_OCMP1", "ETM_PCK0"    },     // GPIO 4
        {  "Q", "ADC1",      "TIM0_ICAP2",  "TIM0_OCMP2", "ETM_PCK1"    },
        {  "Q", "ADC2",      "TIM1_ICAP1",  "TIM1_OCMP1", "ETM_PCK2"    },
        {  "Q", "ADC3",      "TIM1_ICAP2",  "TIM1_OCMP2", "ETM_PCK3"    },
        {  "Q", "ADC4",      "TIM2_ICAP1",  "TIM2_OCMP1", "ETM_PSTAT0"  },
        {  "Q", "ADC5",      "TIM2_ICAP2",  "TIM2_OCMP2", "ETM_PSTAT1"  },
        {  "Q", "ADC6",      "TIM3_ICAP1",  "TIM3_OCMP1", "ETM_PSTAT2"  },
        {  "Q", "ADC7",      "TIM3_ICAP2",  "TIM3_OCMP2", "ETM_TRSYNC"  }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "EXINT8", "EXINT8",    "CAN_RX",      "ETM_TRCLK",  "UART0_TX"    },     // GPIO 5
        {  "EXINT9", "EXINT9",    "UART0_RX",    "CAN_TX",     "UART2_TX"    },
        {  "EXINT10", "EXINT10",   "UART2_RX",    "MII_PHYCLK", "TIM3_OCMP1"  },
        {  "EXINT11", "EXINT11",   "ETM_EXTRIG",  "MII_TX_EN",  "TIM2_OCMP1"  },
        {  "EXINT12", "EXINT12",   "SSP0_SCLK",   "SSP0_SCLK",  "EMI_CS0"     },
        {  "EXINT13", "EXINT13",   "SSP0_MOSI",   "SSP0_MOSI",  "EMI_CS1"     },
        {  "EXINT14", "EXINT14",   "SSP0_MISO",   "SSP0_MISO",  "EMI_CS2"     },
        {  "EXINT15", "EXINT15",   "SSP0_NSS",    "SSP0_NSS",   "EMI_CS3"     }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "EXINT16", "EXINT16",   "TIM0_ICAP1",  "TIM0_OCMP1", "MC_UH"       },     // GPIO 6
        {  "EXINT17", "EXINT17",   "TIM0_ICAP2",  "TIM0_OCMP2", "MC_UL"       },
        {  "EXINT18", "EXINT18",   "TIM1_ICAP1",  "TIM1_OCMP1", "MC_VH"       },
        {  "EXINT19", "EXINT19",   "TIM1_ICAP2",  "TIM1_OCMP2", "MC_VL"       },
        {  "EXINT20", "EXINT20",   "TIM2_ICAP1",  "TIM2_OCMP1", "MC_WH"       },
        {  "EXINT21", "EXINT21",   "TIM2_ICAP2",  "TIM2_OCMP2", "MC_WL"       },
        {  "EXINT22_TRIG", "EXINT22_TRIG","UART0_RX",  "TIM3_OCMP1", "ETM_TRCLK"   },
        {  "EXINT23_STOP", "EXINT23_STOP","ETM_EXTRIG","TIM3_OCMP2", "UART0_TX"    }
    },
#ifndef CHIP_80_PIN
    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "EXINT24", "EXINT24",   "TIM0_ICAP1",  "EMI_A0_16", "ETM_PCK0"     },     // GPIO 7
        {  "EXINT25", "EXINT25",   "TIM0_ICAP2",  "EMI_A1_17", "ETM_PCK1"     },
        {  "EXINT26", "EXINT26",   "TIM2_ICAP1",  "EMI_A2_18", "ETM_PCK2"     },
        {  "EXINT27", "EXINT27",   "TIM2_ICAP2",  "EMI_A3_19", "ETM_PCK3"     },
        {  "EXINT28", "EXINT28",   "UART0_RX",    "EMI_A4_20", "EMI_CS3"      },
        {  "EXINT29", "EXINT29",   "ETM_EXTRIG",  "EMI_A5_21", "EMI_CS2"      },
        {  "EXINT30", "EXINT30",   "TIM3_ICAP1",  "EMI_A6_22", "EMI_CS1"      },
        {  "EXINT31", "EXINT31",   "TIM3_ICAP2",  "EMI_CS0",   "EMI_A7_23"    }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "INVALID",   "INVALID",    "EMI_D0_AD0", "INVALID"      },     // GPIO 8
        {  "Q", "INVALID",   "INVALID",    "EMI_D1_AD1", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D2_AD2", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D3_AD3", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D4_AD4", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D5_AD5", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D6_AD6", "INVALID"      },
        {  "Q", "INVALID",   "INVALID",    "EMI_D7_AD7", "INVALID"      }
    },

    {
        //  Q   Input,       Alt.Input,     Alt.Out 2,    Alt.Out 3
        {  "Q", "INVALID",   "INVALID",    "EMI_A8_AD8",  "INVALID"     },     // GPIO 9
        {  "Q", "INVALID",   "INVALID",    "EMI_A9_AD9",  "INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A10_AD10","INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A11_AD11","INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A12_AD12","INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A13_AD13","INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A14_AD14","INVALID"     },
        {  "Q", "INVALID",   "INVALID",    "EMI_A15_AD15","INVALID"     }
    },
#endif
};


static unsigned char *_ptrPerFunctions;

extern void fnSetPortDetails(char *cPortDetails, int iPort, int iBit, unsigned long *ulPortStates, unsigned long *ulPortFunction, unsigned long *ulPortPeripheral, int iMaxLength)
{
    char *ptrBuf = cPortDetails;
    int iLength = 0;
    int iPortLength = PORT_WIDTH;
    char cBuf[BUF1SIZE+1];

    switch (iPort) {
    case _GPIO_0:
#ifdef CHIP_80_PIN
        STRCPY(cPortDetails, "GPIO 0 - not available on 80 pin device");
        return;
#else
        STRCPY(cPortDetails, "GPIO 0");
        break;
#endif
    case _GPIO_1:
#ifdef CHIP_80_PIN
        STRCPY(cPortDetails, "GPIO 1 - not available on 80 pin device");
        return;
#else
        STRCPY(cPortDetails, "GPIO 1");
        break;
#endif
    case _GPIO_2:
        STRCPY(cPortDetails, "GPIO 2");
        break;
    case _GPIO_3:
        STRCPY(cPortDetails, "GPIO 3");
        break;
    case _GPIO_4:
        STRCPY(cPortDetails, "GPIO 4");
        break;
    case _GPIO_5:
        STRCPY(cPortDetails, "GPIO 5");
        break;
    case _GPIO_6:
        STRCPY(cPortDetails, "GPIO 6");
        break;
    case _GPIO_7:
#ifdef CHIP_80_PIN
        STRCPY(cPortDetails, "GPIO 7 - not available on 80 pin device");
        return;
#else
        STRCPY(cPortDetails, "GPIO 7");
        break;
#endif
    case _GPIO_8:
#ifdef CHIP_80_PIN
        STRCPY(cPortDetails, "GPIO 8 - not available on 80 pin device");
        return;
#else
        STRCPY(cPortDetails, "GPIO 8");
        break;
#endif
    case _GPIO_9:
#ifdef CHIP_80_PIN
        STRCPY(cPortDetails, "GPIO 9 - not available on 80 pin device");
        return;
#else
        STRCPY(cPortDetails, "GPIO 9");
        break;
#endif

    default:
        STRCPY(cPortDetails, "?????");
        return;
    }

    // Bit position
    iBit = iPortLength - iBit - 1;
    if (iBit < 0) return;
    SPRINTF(cBuf, " Bit %i Pin: ", iBit);
    STRCAT(cPortDetails, cBuf);
    STRCPY(cBuf, cPinNumber[iPort][iBit]);
    STRCAT(cPortDetails, cBuf);
    if (!strcmp(cBuf, "NA")) return;

    if (ulPortPeripheral[iPort] & (0x01 << iBit)) {
        unsigned char *ptrList = _ptrPerFunctions;
        int _iPort = iPort;
        int _iBit = iBit;
        while (_iPort--) {
            ptrList += (PORT_WIDTH);
        }
        while (_iBit--) {
            ptrList++;
        }
        cBuf[0] = ' ';
        if (*ptrList > 4) {
            STRCPY(&cBuf[1], "??");
        }
        else {
            STRCPY(&cBuf[1], cPer[iPort][iBit][*ptrList]);
        }
        STRCAT(cPortDetails, cBuf);
    }
    else {
        if (ulPortFunction[iPort] & (0x01 << iBit)) {
            STRCAT(cPortDetails, " Output");
        }
        else {
            STRCAT(cPortDetails, " Input");
        }
    }
}

// Allow the hardware simulator to pass a pointer to hardware details which are useful for simulating this type
//
extern "C" void fnEnterHW_table(void *hw_table)
{
    _ptrPerFunctions = (unsigned char *)hw_table;
}
#endif