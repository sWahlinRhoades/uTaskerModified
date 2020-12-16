/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      kinetis_port_kw.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2015
    *********************************************************************

*/


#if defined KINETIS_KW2X

#define _PIN_COUNT      0                                                // 63 LGA


#define ALTERNATIVE_FUNCTIONS   7                                        // GPIO plus 7 possible peripheral/extra/system functions

static const char *cPinNumber[PORTS_AVAILABLE + 1][PORT_WIDTH][1] = {
    {
        // 63 LGA                                                        GPIO A
        {  "33",   },                                                    // PA0
        {  "34",   },                                                    // PA1
        {  "35",   },                                                    // PA2
        {  "36",   },                                                    // PA3
        {  "37",   },                                                    // PA4
        {  "-",    },                                                    // PA5
        {  "-",    },                                                    // PA6
        {  "-",    },                                                    // PA7
        {  "-",    },                                                    // PA8
        {  "-",    },                                                    // PA9
        {  "-",    },                                                    // PA10
        {  "-",    },                                                    // PA11
        {  "-",    },                                                    // PA12
        {  "-",    },                                                    // PA13
        {  "-",    },                                                    // PA14
        {  "-",    },                                                    // PA15
        {  "-",    },                                                    // PA16
        {  "-",    },                                                    // PA17
        {  "39",   },                                                    // PA18
        {  "40",   },                                                    // PA19
        {  "-",    },                                                    // PA20
        {  "-",    },                                                    // PA21
        {  "-",    },                                                    // PA22
        {  "-",    },                                                    // PA23
        {  "-",    },                                                    // PA24
        {  "-",    },                                                    // PA25
        {  "-",    },                                                    // PA26
        {  "-",    },                                                    // PA27
        {  "-",    },                                                    // PA28
        {  "-",    },                                                    // PA29
        {  "-",    },                                                    // PA30
        {  "-",    },                                                    // PA31
    },
    {
        // 63 LGA                                                        GPIO B
        {  "-",    },                                                    // PB0
        {  "-",    },                                                    // PB1
        {  "-",    },                                                    // PB2
        {  "M",    },                                                    // PB3
        {  "-",    },                                                    // PB4
        {  "-",    },                                                    // PB5
        {  "-",    },                                                    // PB6
        {  "-",    },                                                    // PB7
        {  "-",    },                                                    // PB8
        {  "-",    },                                                    // PB9
        {  "M",    },                                                    // PB10
        {  "M",    },                                                    // PB11
        {  "-",    },                                                    // PB12
        {  "-",    },                                                    // PB13
        {  "-",    },                                                    // PB14
        {  "-",    },                                                    // PB15
        {  "M",    },                                                    // PB16
        {  "M",    },                                                    // PB17
        {  "-",    },                                                    // PB18
        {  "M",    },                                                    // PB19
        {  "-",    },                                                    // PB20
        {  "-",    },                                                    // PB21
        {  "-",    },                                                    // PB22
        {  "-",    },                                                    // PB23
        {  "-",    },                                                    // PB24
        {  "-",    },                                                    // PB25
        {  "-",    },                                                    // PB26
        {  "-",    },                                                    // PB27
        {  "-",    },                                                    // PB28
        {  "-",    },                                                    // PB29
        {  "-",    },                                                    // PB30
        {  "-",    },                                                    // PB31
    },
    {
        // 63 LGA                                                        GPIO C
        {  "M",    },                                                    // PC0
        {  "M",    },                                                    // PC1
        {  "-",    },                                                    // PC2
        {  "M",    },                                                    // PC3
        {  "4",    },                                                    // PC4
        {  "5",    },                                                    // PC5
        {  "6",    },                                                    // PC6
        {  "7",    },                                                    // PC7
        {  "-",    },                                                    // PC8
        {  "-",    },                                                    // PC9
        {  "-",    },                                                    // PC10
        {  "-",    },                                                    // PC11
        {  "-",    },                                                    // PC12
        {  "-",    },                                                    // PC13
        {  "-",    },                                                    // PC14
        {  "-",    },                                                    // PC15
        {  "-",    },                                                    // PC16
        {  "-",    },                                                    // PC17
        {  "-",    },                                                    // PC18
        {  "-",    },                                                    // PC19
        {  "-",    },                                                    // PC20
        {  "-",    },                                                    // PC21
        {  "-",    },                                                    // PC22
        {  "-",    },                                                    // PC23
        {  "-",    },                                                    // PC24
        {  "-",    },                                                    // PC25
        {  "-",    },                                                    // PC26
        {  "-",    },                                                    // PC27
        {  "-",    },                                                    // PC28
        {  "-",    },                                                    // PC29
        {  "-",    },                                                    // PC30
        {  "-",    },                                                    // PC31
    },
    {
        // 63 LGA                                                        GPIO D
        {  "-",    },                                                    // PD0
        {  "8",    },                                                    // PD1
        {  "9",    },                                                    // PD2
        {  "10",   },                                                    // PD3
        {  "11",   },                                                    // PD4
        {  "12",   },                                                    // PD5
        {  "13",   },                                                    // PD6
        {  "14",   },                                                    // PD7
        {  "-",    },                                                    // PD8
        {  "-",    },                                                    // PD9
        {  "-",    },                                                    // PD10
        {  "-",    },                                                    // PD11
        {  "-",    },                                                    // PD12
        {  "-",    },                                                    // PD13
        {  "-",    },                                                    // PD14
        {  "-",    },                                                    // PD15
        {  "-",    },                                                    // PD16
        {  "-",    },                                                    // PD17
        {  "-",    },                                                    // PD18
        {  "-",    },                                                    // PD19
        {  "-",    },                                                    // PD20
        {  "-",    },                                                    // PD21
        {  "-",    },                                                    // PD22
        {  "-",    },                                                    // PD23
        {  "-",    },                                                    // PD24
        {  "-",    },                                                    // PD25
        {  "-",    },                                                    // PD26
        {  "-",    },                                                    // PD27
        {  "-",    },                                                    // PD28
        {  "-",    },                                                    // PD29
        {  "-",    },                                                    // PD30
        {  "-",    },                                                    // PD31
    },
    {
        // 63 LGA                                                        GPIO E
        {  "15",   },                                                    // PE0
        {  "16",   },                                                    // PE1
        {  "17",   },                                                    // PE2
        {  "18",   },                                                    // PE3
        {  "19",   },                                                    // PE4
        {  "-",    },                                                    // PE5
        {  "-",    },                                                    // PE6
        {  "-",    },                                                    // PE7
        {  "-",    },                                                    // PE8
        {  "-",    },                                                    // PE9
        {  "-",    },                                                    // PE10
        {  "-",    },                                                    // PE11
        {  "-",    },                                                    // PE12
        {  "-",    },                                                    // PE13
        {  "-",    },                                                    // PE14
        {  "-",    },                                                    // PE15
#if defined KINETIS_KW21
        {  "21",   },                                                    // PE16
        {  "22",   },                                                    // PE17
        {  "23",   },                                                    // PE18
        {  "24",   },                                                    // PE19
#else
        {  "-",    },                                                    // PE16
        {  "-",    },                                                    // PE17
        {  "-",    },                                                    // PE18
        {  "-",    },                                                    // PE19
#endif
        {  "-",    },                                                    // PE20
        {  "-",    },                                                    // PE21
        {  "-",    },                                                    // PE22
        {  "-",    },                                                    // PE23
        {  "-",    },                                                    // PE24
        {  "-",    },                                                    // PE25
        {  "-",    },                                                    // PE26
        {  "-",    },                                                    // PE27
        {  "-",    },                                                    // PE28
        {  "-",    },                                                    // PE29
        {  "-",    },                                                    // PE30
        {  "-",    },                                                    // PE31
    },
    {
        // 63 LGA                                                        dedicated pins
        {  "1",    },                                                    // EXTAL_32M
        {  "2",    },                                                    // GPIO1
        {  "3",    },                                                    // GPIO2
    #if defined KINETIS_KW22 || defined KINETIS_KW24
        {  "21",   },                                                    // USB0_DP
        {  "22",   },                                                    // USB0_DM
        {  "23",   },                                                    // VOUT33
        {  "24",   },                                                    // VREGIN
    #else
        {  "-",    },
        {  "-",    },
        {  "-",    },
        {  "-",    },
    #endif
        {  "25",   },                                                    // VDDA
        {  "26",   },                                                    // VREFH
        {  "27",   },                                                    // VREFL
        {  "28",   },                                                    // VSSA
        {  "29",   },                                                    // TAMPER0/RTC_WAKEUP_B
        {  "30",   },                                                    // XTAL32
        {  "31",   },                                                    // EXTAL32
        {  "32",   },                                                    // VBAT_MCU
        {  "38",   },                                                    // VDD_MCU
        {  "41",   },                                                    // RESET_b
        {  "42",   },                                                    // VBAT2_RF
        {  "43",   },                                                    // VDD_REGD
        {  "44",   },                                                    // ANT_A
        {  "45",   },                                                    // ANT_B
        {  "46",   },                                                    // RX_SWITCH
        {  "47",   },                                                    // TX_SWITCH
        {  "48",   },                                                    // GND_PA
        {  "49",   },                                                    // RF_OUTP
        {  "50",   },                                                    // RF_OUTN
        {  "51",   },                                                    // GND_PA
        {  "52",   },                                                    // VDD_PA
        {  "53",   },                                                    // VDD_IF
        {  "54",   },                                                    // VDD_RF
        {  "55",   },                                                    // VBAT_RF
        {  "56",   },                                                    // XTAL_32M
    }
};

static int ADC_DEDICATED_CHANNEL[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ADC_DM1_SINGLE, 0, 0};
static int ADC_DEDICATED_MODULE[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
static int ADC_MUX_CHANNEL[PORTS_AVAILABLE][PORT_WIDTH] = {0};


static const char *cPer[PORTS_AVAILABLE][PORT_WIDTH][8] = {
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7
        {  "-",            "PTA0",  "UART0_CTS_b(UART0_COL_b", "FTM0_CH5","-", "-",        "-",        "JTAG_TCLK/SWD_CLK" }, // PORT A
        {  "-",            "PTA1",  "UART0_RX",  "FTM0_CH6",   "-",           "-",         "-",        "JTAG_TDI"          },
        {  "-",            "PTA2",  "UART0_TX",  "FTM0_CH7",   "-",           "-",         "-",        "JTAG_TDO/TRACE_SWO"},
        {  "-",            "PTA3",  "UART0_RTS_b","FTM0_CH0",  "-",           "-",         "-",        "FJTAG_TMS/SWD_DIO" },
        {  "-",            "PTA4/LLWU_P3", "-",  "FTM0_CH1",   "-",           "-",         "-",        "NMI_b"             },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "EXTAL0",       "PTA18", "-",         "FTM0_FLT2",  "FTM_CLKIN0",  "-",         "-",        "-"                 },
        {  "XTAL0",        "PTA19", "-",         "FTM1_FLT0",  "FTM_CLKIN1",  "-",         "LPTMR0_ALT1","-"               },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },                
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }, // PORT B
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "IRQ_B", "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "R_SSEL_B","-",       "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "R_SCLK","-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "R_MOSI","-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "R_MISO","-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "RST_B", "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7
        {  "-",            "GPIO5", "-",         "-",          "-",           "-",         "-",        "-"                 }, // PORT C
        {  "-",            "GPIO4", "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "GPIO3", "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "PTC4/LLWU_P8","SPI0_PCS0","UART1_TX","FTM0_CH3",  "-",         "CMP1_OUT", "-"                 },
        {  "-",            "PTC5/LLWU_P9","SPI0_SCK","LPTMR0_ALT2","I2S0_RXD0","-",        "CMP0_OUT", "-"                 },
        {  "CMP0_IN0",     "PTC6/LLWU_P10","SPI0_SOUT","PDB0_EXTRG","I2S0_RX_BCLK","-",    "I2S0_MCLK","-"                 },
        {  "CMP0_IN1",     "PTC7",  "SPI0_SIN", "USB_SOF_OUT", "I2S0_RX_FS",  "",          "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }, // PORT D
        {  "ADC0_SE5b",    "PTD1",  "SPI0_SCK",  "UART2_CTS_b","-",           "-",         "-",        "-"                 },
        {  "-",            "PTD2/LLWU_P13","SPI0_SOUT","UART2_RX","I2C0_SCL", "-",         "-",        "-"                 },
        {  "-",            "PTD3",  "SPI0_SIN",  "UART2_TX",  "I2C0_SDA",     "-",         "-",        "-"                 },
        {  "mADC0_SE21",   "PTD4/LLWU_P14","SPI0_PCS1","UART0_RTS_b","FTM0_CH4","-",       "EWM_IN",   "-"                 },
        {  "ADC0_SE6b",    "PTD5",  "SPI0_PCS2", "UART0_CTS_b/UART0_COL_b","FTM0_CH5","-", "EWM_OUT_b","-"                 },
        {  "ADC0_SE7b",    "PTD6/LLWU_P15","SPI0_PCS3","UART0_RX","FTM0_CH0", "-",         "FTM0_FLT0","-"                 },
        {  "mADC0_SE22",   "PTD7",  "CMT_IRO",   "UART0_TX",   "FTM0_CH7",   "-",          "FTM0_FLT1","-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7
        {  "mADC0_SE10",   "PTE0",  "SPI1_PCS1", "UART1_TX",   "-",           "mTRACE_CLKOUT","I2C1_SDA","RTC_CLKOUT"      }, // PORT E
        {  "mADC0_SE11",   "PTE1/LLWU_P0","SPI1_SOUT","UART1_RX","-",         "mTRACE_D3", "I2C1_SCL", "SPI1_SIN"          },
        {  "mADC0_DP1",    "PTE2/LLWU_P1","SPI1_SCK","UART1_CTS_b","-",       "mTRACE_D2", "-",        "-"                 },
        {  "mADC0_DM1",    "PTE3",  "SPI1_SIN",  "UART1_RTS_b","-",           "mTRACE_D1", "-",        "SPI1_SOUT"         },
        {  "-",            "PTE4/LLWU_P2","SPI1_PCS0","-",     "-",           "mTRACE_D0", "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
    #if defined KINETIS_KW21
        {  "ADC0_SE4a",    "PTE16", "SPI0_PCS0", "UART2_TX",   "FTM_CLKIN0",  "-",         "FTM0_FLT3","-"                 },
        {  "ADC0_SE5a",    "PTE17", "SPI0_SCK",  "UART2_RX",   "FTM_CLKIN1",  "-",         "LPTMR0_ALT3","-"               },
        {  "ADC0_SE6a",    "PTE18", "SPI0_SOUT", "UART2_CTS_b","I2C0_SDA",    "-",         "-",        "-"                 },
        {  "ADC0_SE7a",    "PTE19", "SPI0_SIN",  "UART2_RTS_b","I2C0_SCL",    "-",         "-",        "-"                 },
    #else
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
    #endif
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-"                 }
    }
};
#endif