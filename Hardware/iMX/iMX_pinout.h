/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      iMX_pinout.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************/

#if defined iMX_RT1011
#if PIN_COUNT == PIN_COUNT_80_PIN                                        // 80 pin LQFP
    #define _PIN_COUNT              0
#else
    #error "Package not clearly defined!!"
#endif

#if defined _PIN_DEFAULTS
static const unsigned char cPinDefaults[PORTS_AVAILABLE][PORT_WIDTH] = {
    {                                                                    // GPIO1
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        GPIO_AD_08_JTAG_TRSTB,
        GPIO_AD_09_JTAG_TDO,
        GPIO_AD_10_JTAG_TDI,
        GPIO_AD_11_JTAG_MOD,
        GPIO_AD_13_JTAG_TMS,
        GPIO_AD_12_JTAG_TCK,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        0,
        0,
        0
    },
    {                                                                    // GPIO2
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        GPIO_SD_03_SRC_BOOT_MODE01,
        GPIO_SD_04_SRC_BOOT_MODE00,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    },
    {                                                                    // GPIO3
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO4
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO5
        PAD_PMIC_ON_REQ_SNVS_LP_PMIC_ON_REQ,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    },
};
#endif

#define ALTERNATIVE_FUNCTIONS   7                                        // GPIO plus 7 possible peripheral/extra/system functions


static const char *cPinNumber[PORTS_AVAILABLE + 1][PORT_WIDTH][3] = {
    {
        // 80LQFP                                                        [GPIO1]
        {  "13",        },                                               // GPIO_00 [GPIO1.IO[0]]
        {  "12",        },                                               // GPIO_01 [GPIO1.IO[1]]
        {  "11",        },                                               // GPIO_02 [GPIO1.IO[2]]
        {  "10",        },                                               // GPIO_03 [GPIO1.IO[3]]
        {  "9",         },                                               // GPIO_04 [GPIO1.IO[4]]
        {  "8",         },                                               // GPIO_05 [GPIO1.IO[5]]
        {  "6",         },                                               // GPIO_06 [GPIO1.IO[6]]
        {  "5",         },                                               // GPIO_07 [GPIO1.IO[7]]
        {  "4",         },                                               // GPIO_08 [GPIO1.IO[8]]
        {  "3",         },                                               // GPIO_09 [GPIO1.IO[9]]
        {  "2",         },                                               // GPIO_10 [GPIO1.IO[10]]
        {  "1",         },                                               // GPIO_11 [GPIO1.IO[11]]
        {  "80",        },                                               // GPIO_12 [GPIO1.IO[12]]
        {  "79",        },                                               // GPIO_13 [GPIO1.IO[13]]
        {  "60",        },                                               // GPIO_AD_00 [GPIO1.IO[14]]
        {  "59",        },                                               // GPIO_AD_01 [GPIO1.IO[15]]
        {  "58",        },                                               // GPIO_AD_02 [GPIO1.IO[16]]
        {  "57",        },                                               // GPIO_AD_03 [GPIO1.IO[17]]
        {  "56",        },                                               // GPIO_AD_04 [GPIO1.IO[18]]
        {  "55",        },                                               // GPIO_AD_05 [GPIO1.IO[19]]
        {  "52",        },                                               // GPIO_AD_06 [GPIO1.IO[20]]
        {  "51",        },                                               // GPIO_AD_07 [GPIO1.IO[21]]
        {  "49",        },                                               // GPIO_AD_08 [GPIO1.IO[22]] jtag_trstb
        {  "48",        },                                               // GPIO_AD_09 [GPIO1.IO[23]] jtag_tdo
        {  "47",        },                                               // GPIO_AD_10 [GPIO1.IO[24]] jtag_tdi
        {  "46",        },                                               // GPIO_AD_11 [GPIO1.IO[25]] jtag_mod
        {  "45",        },                                               // GPIO_AD_12 [GPIO1.IO[26]] jtag_tck
        {  "44",        },                                               // GPIO_AD_13 [GPIO1.IO[27]] jtag_tms
        {  "43",        },                                               // GPIO_AD_14 [GPIO1.IO[28]]
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
    },
    {
        // 80LQFP                                                        GPIO_EMC [GPIO2]
        {  "76",        },                                               // GPIO_SD_00 [GPIO2.IO[0]]
        {  "75",        },                                               // GPIO_SD_00 [GPIO2.IO[1]]
        {  "74",        },                                               // GPIO_SD_00 [GPIO2.IO[2]]
        {  "73",        },                                               // GPIO_SD_00 [GPIO2.IO[3]] src.BOOT_MODE[1] GPIO_B1_01
        {  "72",        },                                               // GPIO_SD_00 [GPIO2.IO[4]] src.BOOT_MODE[0] GPIO_B1_00
        {  "70",        },                                               // GPIO_SD_00 [GPIO2.IO[5]]
        {  "69",        },                                               // GPIO_SD_00 [GPIO2.IO[6]]
        {  "68",        },                                               // GPIO_SD_00 [GPIO2.IO[7]]
        {  "67",        },                                               // GPIO_SD_00 [GPIO2.IO[8]]
        {  "66",        },                                               // GPIO_SD_00 [GPIO2.IO[9]]
        {  "65",        },                                               // GPIO_SD_00 [GPIO2.IO[10]]
        {  "64",        },                                               // GPIO_SD_00 [GPIO2.IO[11]]
        {  "62",        },                                               // GPIO_SD_00 [GPIO2.IO[12]]
        {  "61",        },                                               // GPIO_SD_00 [GPIO2.IO[13]]
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
    },
    {
        // 80LQFP                                                        [GPIO3]
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
    },
    {
        // 80LQFP
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
    },
    {
        // 80LQFP                                                        [GPIO5]
        {  "24",        },                                               // PMIC_ON_REQ [GPIO5.IO[00]]
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
        {  "-",         },                                               // 
    },
    { 
        // 80LQFP                                                        dedicated ADC pins
        {  "-",         },                                               // ADC0_DP0
        {  "-",         },                                               // ADC0_DM0
        {  "-",         },                                               // ADC0_DP3
        {  "-",         },                                               // ADC0_DM3
        {  "-",         },                                               // VREFH
        {  "-",         },                                               // VREFL
        {  "-",         },                                               // ADC0_DP1
        {  "-",         },                                               // ADC0_DM1
        {  "-",         },                                               // VREF_OUT/CMP1_IN5/CMP0_IN5/ADC0_SE22
        {  "-",         },                                               // DAC0_OUT/CMP1_IN3/ADC0_SE23
        {  "-",         },                                               // RTC_WAKEUP_B
        {  "-",         },                                               // XTAL32
        {  "-",         },                                               // EXTAL32
        {  "-",         },                                               // VBAT
        {  "-",         },                                               // RESET_b
        {  "-",         },                                               // ADC0_SE16
        {  "-",         },                                               // VDDIO_E
        {  "-",         },                                               // VDDIO_E
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
        {  "-",         },                                               //
    }
};

static int ADC_DEDICATED_CHANNEL[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_DEDICATED_MODULE[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_MUX_CHANNEL[PORTS_AVAILABLE][PORT_WIDTH] = {
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port A
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port B
#if PORTS_AVAILABLE > 2
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port C
#endif
#if PORTS_AVAILABLE > 3
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port D
#endif
#if PORTS_AVAILABLE > 4
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port E
#endif
#if PORTS_AVAILABLE > 5
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port F
#endif
};


static const char *cPer[PORTS_AVAILABLE][PORT_WIDTH][8] = {
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7   GPIO1
        {  "FLEXSPI_B_DQS","SAI3_MCLK","LPSPI2_PCS3","LPSPI1_PCS3","PIT_TRIGGER00","GPIO1_IO00", "-",  "-", }, // pad GPIO_00 [GPIO1-00]
        {  "SAI1_RX_BCLK", "WDOG1_ANY","FLEXPWM1_PWM0_B","LPI2C1_SDA","KPP_ROW03","GPIO1_IO01","-",    "-", }, // pad GPIO_01 [GPIO1-01]
        {  "SAI1_RX_SYNC", "WDOG2_B","FLEXPWM1_PWM0_A","LPI2C1_SCL","KPP_COL03","GPIO1_IO02","-",      "-", }, // pad GPIO_02 [GPIO1-02]
        {  "SAI1_RX_DATA00","GPT1_COMPARE3","FLEXPWM1_PWM1_B","-","SPDIF_SR_CLK","GPIO1_IO03","-",     "-", }, // pad GPIO_03 [GPIO1-03]
        {  "SAI1_TX_DATA00","GPT1_CAPTURE2","FLEXPWM1_PWM1_A","-","SPDIF_IN", "GPIO1_IO04","-",        "-", }, // pad GPIO_04 [GPIO1-04]
        {  "SAI1_TX_DATA01","GPT1_COMPARE2","FLEXPWM1_PWM2_B","LPUART4_RXD","SPDIF_OUT","GPIO1_IO05","-","-", }, // pad GPIO_05 [GPIO1-05]
        {  "SAI1_TX_BCLK","GPT1_CAPTURE1","FLEXPWM1_PWM2_A","LPUART4_TXD","SPDIF_EXT_CLK","GPIO1_IO06","-","-", }, // pad GPIO_06 [GPIO1-06]
        {  "SAI1_TX_SYNC","GPT1_COMPARE1","FLEXPWM1_PWM3_B","LPUART3_RXD","SPDIF_LOCK","GPIO1_IO07","LPUART1_RTS_B","-", }, // pad GPIO_07 [GPIO1-07]
        {  "SAI1_MCLK","GPT1_CLK","FLEXPWM1_PWM3_A","LPUART3_TXD","FLEXIO1_IO00","GPIO1_IO08","LPUART1_CTS_B","-", }, // pad GPIO_08 [GPIO1-08]
        {  "LPUART1_RXD","WDOG1_B","FLEXSPI_A_SS1_B","LPI2C2_SDA","FLEXIO1_IO01","GPIO1_IO09 ","SPDIF_SR_CLK","-", }, // pad GPIO_09 [GPIO1-09]
        {  "LPUART1_TXD","LPI2C1_HREQ","EWM_OUT_B","LPI2C2_SCL","FLEXIO1_IO02","GPIO1_IO10","SPDIF_IN","-", }, // pad GPIO_10 [GPIO1-10]
        {  "LPUART3_RXD","LPI2C1_SDA","KPP_ROW00","FLEXSPI_B_SS1_B","FLEXIO1_IO03","GPIO1_IO11","SPDIF_OUT","ARM_CM7_TRACE03", }, // pad GPIO_11 [GPIO1-11]
        {  "LPUART3_TXD","LPI2C1_SCL","KPP_COL00","USB_OTG1_OC","FLEXIO1_IO04","GPIO1_IO12","SPDIF_EXT_CLK","ARM_CM7_TRACE02", }, // pad GPIO_12 [GPIO1-12]
        {  "LPUART2_RXD","LPSPI2_PCS2","KPP_ROW03","OTG1_ID","FLEXIO1_IO05","GPIO1_IO13","SPDIF_LOCK","ARM_CM7_TRACE01", }, // pad GPIO_13 [GPIO1-13]
        {  "LPUART2_TXD","LPSPI1_PCS2","KPP_COL03","USB_OTG1_PWR","FLEXIO1_IO20","GPIO1_IO14","NMI_GLUE_NMI","ARM_CM7_TRACE00", }, // pad GPIO_AD_00 [GPIO1-14]
        {  "LPUART4_RXD","LPSPI2_PCS1","WDOG1_ANY","LPI2C2_SDA","MQS_LEFT","GPIO1_IO15","USB_OTG1_OC","ARM_CM7_TRACE_SWO", }, // pad GPIO_AD_01 [GPIO1-15]
        {  "LPUART4_TXD","LPSPI1_PCS1","WDOG2_B","LPI2C2_SCL","MQS_RIGHT","GPIO1_IO16","-","ARM_CM7_TRACE_CLK", }, // pad GPIO_AD_02 [GPIO1-16]
        {  "LPSPI1_SDI","PIT_TRIGGER03","FLEXPWM1_PWM2_B","KPP_ROW02","GPT2_CLK","GPIO1_IO17","SNVS_HP_VIO_5_B","JTAG_DE_B", }, // pad GPIO_AD_03 [GPIO1-17]
        {  "LPSPI1_SDO","PIT_TRIGGER02","FLEXPWM1_PWM2_A","KPP_COL02","GPT2_COMPARE1","GPIO1_IO18","SNVS_HP_VIO_5_CTL","-", }, // pad GPIO_AD_04 [GPIO1-18]
        {  "LPSPI1_PCS0","PIT_TRIGGER01","FLEXPWM1_PWM3_B","KPP_ROW01","GPT2_CAPTURE1","GPIO1_IO19","-","-", }, // pad GPIO_AD_05 [GPIO1-19]
        {  "LPSPI1_SCK","PIT_TRIGGER00","FLEXPWM1_PWM3_A","KPP_COL01","GPT2_COMPARE2","GPIO1_IO20","LPI2C1_HREQ","-", }, // pad GPIO_AD_06 [GPIO1-20]
        {  "LPI2C2_SDA","LPUART3_RXD","ARM_CM7_RXEV","LPUART2_RTS_B","GPT2_CAPTURE2","GPIO1_IO21","OCOTP_FUSE_LATCHED","XBAR1_INOUT03", }, // pad GPIO_AD_07 [GPIO1-21]
        {  "LPI2C2_SCL","LPUART3_TXD","ARM_CM7_TXEV","LPUART2_CTS_B","GPT2_COMPARE3","GPIO1_IO22","EWM_OUT_B","JTAG_TRSTB", }, // pad GPIO_AD_08 [GPIO1-22]
        {  "LPSPI2_SDI","FLEXPWM1_PWM3_X","KPP_ROW02","ARM_TRACE_SWO","FLEXIO1_IO21","GPIO1_IO23","REF_32K_OUT","JTAG_TDO", }, // pad GPIO_AD_09 [GPIO1-23]
        {  "LPSPI2_SDO","FLEXPWM1_PWM2_X","KPP_COL02","PIT_TRIGGER03","FLEXIO1_IO22","GPIO1_IO24","OTG1_ID","JTAG_TDI", }, // pad GPIO_AD_10 [GPIO1-24]
        {  "LPSPI2_PCS0","FLEXPWM1_PWM1_X","KPP_ROW01","PIT_TRIGGER02","FLEXIO1_IO23","GPIO1_IO25","WDOG1_B","JTAG_MOD", }, // pad GPIO_AD_11 [GPIO1-25]
        {  "LPSPI2_SCK","FLEXPWM1_PWM0_X","KPP_COL01","PIT_TRIGGER01","FLEXIO1_IO24","GPIO1_IO26","USB_OTG1_PWR","JTAG_TCK", }, // pad GPIO_AD_12 [GPIO1-26]
        {  "LPI2C1_SDA","LPUART3_RTS_B","KPP_ROW00","LPUART4_RTS_B","FLEXIO1_IO25","GPIO1_IO27","NMI_GLUE_NMI","JTAG_TMS", }, // pad GPIO_AD_13 [GPIO1-27]
        {  "LPI2C1_SCL","LPUART3_CTS_B","KPP_COL00","LPUART4_CTS_B","FLEXIO1_IO26","GPIO1_IO28","REF_CLK_24M","XBAR1_INOUT02", }, // pad GPIO_AD_14 [GPIO1-28]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7                GPIO2
        {  "FLEXSPI_B_SS0_B","SAI3_TX_SYNC","ARM_CM7_RXEV","CCM_STOP","FLEXIO1_IO06","GPIO2_IO00","SRC_BT_CFG02","-", }, // pad GPIO_SD_00 [GPIO2-00]
        {  "FLEXSPI_B_DATA01","SAI3_TX_BCLK","FLEXPWM1_PWM0_B","CCM_CLKO2","FLEXIO1_IO07","GPIO2_IO01","SRC_BT_CFG01","-", }, // pad GPIO_SD_01 [GPIO2-01]
        {  "FLEXSPI_B_DATA02","SAI3_TX_DATA","FLEXPWM1_PWM0_A","CCM_CLKO1","FLEXIO1_IO08","GPIO2_IO02","SRC_BT_CFG00","-", }, // pad GPIO_SD_02 [GPIO2-02]
        {  "FLEXSPI_B_DATA00","SAI3_RX_DATA","FLEXPWM1_PWM1_B","CCM_REF_EN_B","FLEXIO1_IO09","GPIO2_IO03","SRC_BOOT_MODE01","-", }, // pad GPIO_SD_03 [GPIO2-03]
        {  "FLEXSPI_B_DATA03","SAI3_RX_SYNC","FLEXPWM1_PWM1_A","CCM_WAIT","FLEXIO1_IO10","GPIO2_IO04","SRC_BOOT_MODE00","-", }, // pad GPIO_SD_04 [GPIO2-04]
        {  "FLEXSPI_A_SS1_B","LPI2C1_SDA","LPSPI1_SDI","-","FLEXIO1_IO11","GPIO2_IO05","-","-", }, // pad GPIO_SD_05 [GPIO2-05]
        {  "FLEXSPI_A_SS0_B","LPI2C1_SCL","LPSPI1_SDO","-","FLEXIO1_IO12","GPIO2_IO06","-","-", }, // pad GPIO_SD_06 [GPIO2-06]
        {  "FLEXSPI_A_DATA1","LPI2C2_SDA","LPSPI1_PCS0","-","FLEXIO1_IO13","GPIO2_IO07","-","-", }, // pad GPIO_SD_07 [GPIO2-07]
        {  "FLEXSPI_A_DATA2","LPI2C2_SCL","LPSPI1_SCK","-","FLEXIO1_IO14","GPIO2_IO08","-","-", }, // pad GPIO_SD_08 [GPIO2-08]
        {  "FLEXSPI_A_DATA0","LPSPI2_SDI","LPUART2_RXD","-","FLEXIO1_IO15","GPIO2_IO09","-","-", }, // pad GPIO_SD_09 [GPIO2-09]
        {  "FLEXSPI_A_SCLK","LPSPI2_SDO","LPUART2_TXD","-","FLEXIO1_IO16","GPIO2_IO10","-","-", }, // pad GPIO_SD_10 [GPIO2-10]
        {  "FLEXSPI_A_DATA3","LPSPI2_SCK","LPUART1_RXD","-","FLEXIO1_IO17","GPIO2_IO11","WDOG1_RST_B_DEB","-", }, // pad GPIO_SD_11 [GPIO2-11]
        { "FLEXSPI_A_DQS","LPSPI2_PCS0","LPUART1_TXD","-","FLEXIO1_IO18","GPIO2_IO12","WDOG2_RST_B_DEB","-", }, // pad GPIO_SD_12 [GPIO2-12]
        { "FLEXSPI_B_SCLK","SAI3_RX_BCLK","ARM_CM7_TXEV","CCM_PMIC_RDY","FLEXIO1_IO19","GPIO2_IO13","SRC_BT_CFG03","-", }, // pad GPIO_SD_13 [GPIO2-13]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7         GPIO3
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7          GPIO4
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", }
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7       GPIO5
        {  "SNVS_LP_PMIC_ON_REQ","-","-",        "-",          "-",           "GPIO5_IO00","-",        "-", }, // pad PMIC_ON_REQ [GPIO5-00]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-", }
    },
};
#elif defined iMX_RT1015
#if PIN_COUNT == PIN_COUNT_100_PIN                                       // 100 pin LQFP
    #define _PIN_COUNT              0
#else
    #error "Package not clearly defined!!"
#endif

#if defined _PIN_DEFAULTS
static const unsigned char cPinDefaults[PORTS_AVAILABLE][PORT_WIDTH] = {
    {                                                                    // GPIO1
        GPIO_AD_B0_00_JTAG_TMS,
        GPIO_AD_B0_01_JTAG_TCK,
        GPIO_AD_B0_02_JTAG_MOD,
        GPIO_AD_B0_03_JTAG_TDI,
        GPIO_AD_B0_04_JTAG_TDO,
        GPIO_AD_B0_05_JTAG_TRSTB,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO2
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        GPIO_EMC_16_SRC_BOOT_MODE00,
        GPIO_EMC_17_SRC_BOOT_MODE01,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO3
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO4
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO5
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
};
#endif

#define ALTERNATIVE_FUNCTIONS   7                                        // GPIO plus 7 possible peripheral/extra/system functions

static const char *cPinNumber[PORTS_AVAILABLE + 1][PORT_WIDTH][3] = {
    {
        // 100LQFP                                                       GPIO_AD_B0/1 [GPIO1]
        {  "78",     },                                                  // GPIO_AD_B0_00 [GPIO1.IO[0]] jtag_mux.TMS
        {  "77",     },                                                  // GPIO_AD_B0_01 [GPIO1.IO[1]] jtag_mux.TCK
        {  "76",     },                                                  // GPIO_AD_B0_02 [GPIO1.IO[2]] jtag_mux.MOD
        {  "75",     },                                                  // GPIO_AD_B0_03 [GPIO1.IO[3]] jtag_mux.TDI
        {  "74",     },                                                  // GPIO_AD_B0_04 [GPIO1.IO[4]] jtag_mux.TDO
        {  "74",     },                                                  // GPIO_AD_B0_05 [GPIO1.IO[5]] jtag_mux.TRSTB
        {  "72",     },                                                  // GPIO_AD_B0_06 [GPIO1.IO[6]]
        {  "68",     },                                                  // GPIO_AD_B0_07 [GPIO1.IO[7]]
        {  "67",     },                                                  // GPIO_AD_B0_08 [GPIO1.IO[8]]
        {  "66",     },                                                  // GPIO_AD_B0_09 [GPIO1.IO[9]]
        {  "65",     },                                                  // GPIO_AD_B0_10 [GPIO1.IO[10]]
        {  "64",     },                                                  // GPIO_AD_B0_11 [GPIO1.IO[11]]
        {  "63",     },                                                  // GPIO_AD_B0_12 [GPIO1.IO[12]]
        {  "62",     },                                                  // GPIO_AD_B0_13 [GPIO1.IO[13]]
        {  "61",     },                                                  // GPIO_AD_B0_14 [GPIO1.IO[14]]
        {  "60",     },                                                  // GPIO_AD_B0_15 [GPIO1.IO[15]]
        {  "-",      },                                                  // GPIO_AD_B1_00 [GPIO1.IO[16]]
        {  "-",      },                                                  // GPIO_AD_B1_01 [GPIO1.IO[17]]
        {  "-",      },                                                  // GPIO_AD_B1_02 [GPIO1.IO[18]]
        {  "-",      },                                                  // GPIO_AD_B1_03 [GPIO1.IO[19]]
        {  "-",      },                                                  // GPIO_AD_B1_04 [GPIO1.IO[20]]
        {  "-",      },                                                  // GPIO_AD_B1_05 [GPIO1.IO[21]]
        {  "-",      },                                                  // GPIO_AD_B1_06 [GPIO1.IO[22]]
        {  "-",      },                                                  // GPIO_AD_B1_07 [GPIO1.IO[23]]
        {  "-",      },                                                  // GPIO_AD_B1_08 [GPIO1.IO[24]]
        {  "-",      },                                                  // GPIO_AD_B1_09 [GPIO1.IO[25]]
        {  "57",     },                                                  // GPIO_AD_B1_10 [GPIO1.IO[26]]
        {  "56",     },                                                  // GPIO_AD_B1_11 [GPIO1.IO[27]]
        {  "55",     },                                                  // GPIO_AD_B1_12 [GPIO1.IO[28]]
        {  "54",     },                                                  // GPIO_AD_B1_13 [GPIO1.IO[29]]
        {  "53",     },                                                  // GPIO_AD_B1_14 [GPIO1.IO[30]]
        {  "52",     },                                                  // GPIO_AD_B1_15 [GPIO1.IO[31]]
    },                                                                   
    {
        // 100LQFP                                                       GPIO_EMC [GPIO2]
        {  "-",      },                                                  // GPIO_EMC_00 [GPIO2.IO[0]] GPIO_B0_00
        {  "-",      },                                                  // GPIO_EMC_01 [GPIO2.IO[1]] GPIO_B0_01
        {  "-",      },                                                  // GPIO_EMC_02 [GPIO2.IO[2]] GPIO_B0_02
        {  "-",      },                                                  // GPIO_EMC_03 [GPIO2.IO[3]] GPIO_B0_03
        {  "7",      },                                                  // GPIO_EMC_04 [GPIO2.IO[4]] GPIO_B0_04
        {  "6",      },                                                  // GPIO_EMC_05 [GPIO2.IO[5]] GPIO_B0_05
        {  "4",      },                                                  // GPIO_EMC_06 [GPIO2.IO[6]] GPIO_B0_06
        {  "3",      },                                                  // GPIO_EMC_07 [GPIO2.IO[7]] GPIO_B0_07
        {  "2",      },                                                  // GPIO_EMC_08 [GPIO2.IO[8]] GPIO_B0_08
        {  "1",      },                                                  // GPIO_EMC_09 [GPIO2.IO[9]] GPIO_B0_09
        {  "-",      },                                                  // GPIO_EMC_10 [GPIO2.IO[10]] GPIO_B0_10
        {  "-",      },                                                  // GPIO_EMC_11 [GPIO2.IO[11]] GPIO_B0_11
        {  "-",      },                                                  // GPIO_EMC_12 [GPIO2.IO[12]] GPIO_B0_12
        {  "-",      },                                                  // GPIO_EMC_13 [GPIO2.IO[13]] GPIO_B0_13
        {  "-",      },                                                  // GPIO_EMC_14 [GPIO2.IO[14]] GPIO_B0_14
        {  "-",      },                                                  // GPIO_EMC_15 [GPIO2.IO[15]] GPIO_B0_15
        {  "100",    },                                                  // GPIO_EMC_16 [GPIO2.IO[16]] src.BOOT_MODE[0] GPIO_B1_00
        {  "99",     },                                                  // GPIO_EMC_17 [GPIO2.IO[17]] src.BOOT_MODE[1] GPIO_B1_01
        {  "98",     },                                                  // GPIO_EMC_18 [GPIO2.IO[18]] GPIO_B1_02
        {  "97",     },                                                  // GPIO_EMC_19 [GPIO2.IO[19]] GPIO_B1_03
        {  "96",     },                                                  // GPIO_EMC_20 [GPIO2.IO[20]] GPIO_B1_04
        {  "94",     },                                                  // GPIO_EMC_21 [GPIO2.IO[21]] GPIO_B1_05
        {  "93",     },                                                  // GPIO_EMC_22 [GPIO2.IO[22]] GPIO_B1_06
        {  "90",     },                                                  // GPIO_EMC_23 [GPIO2.IO[23]] GPIO_B1_07
        {  "89",     },                                                  // GPIO_EMC_24 [GPIO2.IO[24]] GPIO_B1_08
        {  "88",     },                                                  // GPIO_EMC_25 [GPIO2.IO[25]] GPIO_B1_09
        {  "87",     },                                                  // GPIO_EMC_26 [GPIO2.IO[26]] GPIO_B1_10
        {  "86",     },                                                  // GPIO_EMC_27 [GPIO2.IO[27]] GPIO_B1_11
        {  "-",      },                                                  // GPIO_EMC_28 [GPIO2.IO[28]] GPIO_B1_12
        {  "-",      },                                                  // GPIO_EMC_29 [GPIO2.IO[29]] GPIO_B1_13
        {  "-",      },                                                  // GPIO_EMC_30 [GPIO2.IO[30]] GPIO_B1_14
        {  "-",      },                                                  // GPIO_EMC_31 [GPIO2.IO[31]] GPIO_B1_15
    },
    {
        // 100LQFP                                                       GPIO_EMC/GPIO_SD_B [GPIO3]
        {  "85",     },                                                  // GPIO_EMC_32   [GPIO3.IO[0]]  GPIO_SD_B1_00
        {  "84",     },                                                  // GPIO_EMC_33   [GPIO3.IO[1]]  GPIO_SD_B1_01
        {  "83",     },                                                  // GPIO_EMC_34   [GPIO3.IO[2]]  GPIO_SD_B1_02
        {  "82",     },                                                  // GPIO_EMC_35   [GPIO3.IO[3]]  GPIO_SD_B1_03
        {  "-",      },                                                  // GPIO_EMC_36   [GPIO3.IO[4]]  GPIO_SD_B1_04
        {  "-",      },                                                  // GPIO_EMC_37   [GPIO3.IO[5]]  GPIO_SD_B1_05
        {  "-",      },                                                  // GPIO_EMC_38   [GPIO3.IO[6]]  GPIO_SD_B1_06
        {  "-",      },                                                  // GPIO_EMC_39   [GPIO3.IO[7]]  GPIO_SD_B1_07
        {  "-",      },                                                  // GPIO_EMC_40   [GPIO3.IO[8]]  GPIO_SD_B1_08
        {  "-",      },                                                  // GPIO_EMC_41   [GPIO3.IO[9]]  GPIO_SD_B1_09
        {  "-",      },                                                  //               [GPIO3.IO[10]] GPIO_SD_B1_10
        {  "-",      },                                                  //               [GPIO3.IO[11]] GPIO_SD_B1_11
        {  "-",      },                                                  //               [GPIO3.IO[12]] GPIO_SD_B0_00
        {  "-",      },                                                  // GPIO_SD_B0_00 [GPIO3.IO[13]] GPIO_SD_B0_01
        {  "-",      },                                                  // GPIO_SD_B0_01 [GPIO3.IO[14]] GPIO_SD_B0_02
        {  "-",      },                                                  // GPIO_SD_B0_02 [GPIO3.IO[15]] GPIO_SD_B0_03
        {  "-",      },                                                  // GPIO_SD_B0_03 [GPIO3.IO[16]] GPIO_SD_B0_04
        {  "-",      },                                                  // GPIO_SD_B0_04 [GPIO3.IO[17]] GPIO_SD_B0_05
        {  "-",      },                                                  // GPIO_SD_B0_05 [GPIO3.IO[18]] GPIO_EMC_32
        {  "-",      },                                                  // GPIO_SD_B0_06 [GPIO3.IO[19]] GPIO_EMC_33
        {  "22",     },                                                  // GPIO_SD_B1_00 [GPIO3.IO[20]] GPIO_EMC_34
        {  "21",     },                                                  // GPIO_SD_B1_01 [GPIO3.IO[21]] GPIO_EMC_35
        {  "17",     },                                                  // GPIO_SD_B1_02 [GPIO3.IO[22]] GPIO_EMC_36
        {  "16",     },                                                  // GPIO_SD_B1_03 [GPIO3.IO[23]] GPIO_EMC_37
        {  "15",     },                                                  // GPIO_SD_B1_04 [GPIO3.IO[24]] GPIO_EMC_38
        {  "14",     },                                                  // GPIO_SD_B1_05 [GPIO3.IO[25]] GPIO_EMC_39
        {  "13",     },                                                  // GPIO_SD_B1_06 [GPIO3.IO[26]] GPIO_EMC_40
        {  "12",     },                                                  // GPIO_SD_B1_07 [GPIO3.IO[27]] GPIO_EMC_41
        {  "11",     },                                                  // GPIO_SD_B1_08 [GPIO3.IO[28]]
        {  "10",     },                                                  // GPIO_SD_B1_09 [GPIO3.IO[29]]
        {  "9",      },                                                  // GPIO_SD_B1_10 [GPIO3.IO[30]]
        {  "8",      },                                                  // GPIO_SD_B1_11 [GPIO3.IO[31]]
    },
    {
        // 100LQFP
        {  "-",      },                                                  // GPIO_EMC_00  [GPIO4.IO[0]] 
        {  "-",      },                                                  // GPIO_EMC_01  [GPIO4.IO[1]] 
        {  "-",      },                                                  // GPIO_EMC_02  [GPIO4.IO[2]] 
        {  "-",      },                                                  // GPIO_EMC_03  [GPIO4.IO[3]] 
        {  "-",      },                                                  // GPIO_EMC_04  [GPIO4.IO[4]] 
        {  "-",      },                                                  // GPIO_EMC_05  [GPIO4.IO[5]] 
        {  "-",      },                                                  // GPIO_EMC_06  [GPIO4.IO[6]] 
        {  "-",      },                                                  // GPIO_EMC_07  [GPIO4.IO[7]] 
        {  "-",      },                                                  // GPIO_EMC_08  [GPIO4.IO[8]] 
        {  "-",      },                                                  // GPIO_EMC_09  [GPIO4.IO[9]] 
        {  "-",      },                                                  // GPIO_EMC_10  [GPIO4.IO[10]]
        {  "-",      },                                                  // GPIO_EMC_11  [GPIO4.IO[11]]
        {  "-",      },                                                  // GPIO_EMC_12  [GPIO4.IO[12]]
        {  "-",      },                                                  // GPIO_EMC_13  [GPIO4.IO[13]]
        {  "-",      },                                                  // GPIO_EMC_14  [GPIO4.IO[14]]
        {  "-",      },                                                  // GPIO_EMC_15  [GPIO4.IO[15]]
        {  "-",      },                                                  // GPIO_EMC_16  [GPIO4.IO[16]]
        {  "-",      },                                                  // GPIO_EMC_17  [GPIO4.IO[17]]
        {  "-",      },                                                  // GPIO_EMC_18  [GPIO4.IO[18]]
        {  "-",      },                                                  // GPIO_EMC_19  [GPIO4.IO[19]]
        {  "-",      },                                                  // GPIO_EMC_20  [GPIO4.IO[20]]
        {  "-",      },                                                  // GPIO_EMC_21  [GPIO4.IO[21]]
        {  "-",      },                                                  // GPIO_EMC_22  [GPIO4.IO[22]]
        {  "-",      },                                                  // GPIO_EMC_23  [GPIO4.IO[23]]
        {  "-",      },                                                  // GPIO_EMC_24  [GPIO4.IO[24]]
        {  "-",      },                                                  // GPIO_EMC_25  [GPIO4.IO[25]]
        {  "-",      },                                                  // GPIO_EMC_26  [GPIO4.IO[26]]
        {  "-",      },                                                  // GPIO_EMC_27  [GPIO4.IO[27]]
        {  "-",      },                                                  // GPIO_EMC_28  [GPIO4.IO[28]]
        {  "-",      },                                                  // GPIO_EMC_29  [GPIO4.IO[29]]
        {  "-",      },                                                  // GPIO_EMC_30  [GPIO4.IO[30]]
        {  "-",      },                                                  // GPIO_EMC_31  [GPIO4.IO[31]]
    },
    {
        // 100LQFP                                                       [GPIO5]
        {  "-",      },                                                  // WAKEUP [GPIO5.IO[00]]
        {  "-",      },                                                  // PMIC_ON_REQ [GPIO5.IO[01]]
        {  "-",      },                                                  // PMIC_STBY_REQ [GPIO5.IO[02]]
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
        {  "-",      },                                                  // 
    },
    { 
        // 100LQFP                                                       dedicated ADC pins
        {  "36",     },                                                  // RTC_XTALI
        {  "37",     },                                                  // RTC_XTALO
        {  "45",     },                                                  // USB_OTG1_CHD_B
        {  "41",     },                                                  // USB_OTG1_DN
        {  "42",     },                                                  // USB_OTG1_DP
        {  "38",     },                                                  // USB_OTG1_VBUS
        {  "46",     },                                                  // XTALI
        {  "47",     },                                                  // XTALO
        {  "40",     },                                                  // VDD_USB_CAP
        {  "23",     },                                                  // DCDC_IN
        {  "27",     },                                                  // DCDC_IN_Q
        {  "25",     },                                                  // DCDC_LP
        {  "26",     },                                                  // DCDC_PSWITCH
        {  "34",     },                                                  // VDD_SNVS_IN
        {  "35",     },                                                  // VDD_SNVS_CAP
        {  "51",     },                                                  // VDDA_ADC_3P3
        {  "50",     },                                                  // NVCC_PLL
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
        {  "-",      },                                                  //
    }
};

#define _DEDICATED_PINS
static CHAR *cDedicated[PORT_WIDTH] = {                                  // dedicated pin functions
    "RTC_XTALI",
    "RTC_XTALO",
    "USB_OTG1_CHD_B",
    "USB_OTG1_DN",
    "USB_OTG1_DP",
    "USB_OTG1_VBUS",
    "XTALI",
    "XTALO",
    "VDD_USB_CAP",
    "DCDC_IN",
    "DCDC_IN_Q",
    "DCDC_LP",
    "DCDC_PSWITCH",
    "VDD_SNVS_IN",
    "VDD_SNVS_CAP",
    "VDDA_ADC_3P3",
    "NVCC_PLL",
};

static int ADC_DEDICATED_CHANNEL[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_DEDICATED_MODULE[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_MUX_CHANNEL[PORTS_AVAILABLE][PORT_WIDTH] = {
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port A
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port B
#if PORTS_AVAILABLE > 2
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port C
#endif
#if PORTS_AVAILABLE > 3
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port D
#endif
#if PORTS_AVAILABLE > 4
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port E
#endif
#if PORTS_AVAILABLE > 5
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port F
#endif
};


static const char *cPer[PORTS_AVAILABLE][PORT_WIDTH][10] = {
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7           ALT 8       ALT 9  GPIO1
        {  "JTAG_TMS",     "-",     "-",         "-",          "-",           "GPIO1_IO00","-",        "GPT1_COMPARE1","-","-" }, // pad GPIO_AD_B0_00 [GPIO1-0]
        {  "JTAG_TCK",     "-",     "-",         "-",          "-",           "GPIO1_IO01","-",        "GPT1_CAPTURE2","-","-" }, // pad GPIO_AD_B0_01 [GPIO1-1]
        {  "JTAG_MOD",     "-",     "-",         "-",          "-",           "GPIO1_IO02","-",        "GPT1_CAPTURE1","-","-" }, // pad GPIO_AD_B0_02 [GPIO1-2]
        {  "JTAG_TDI",     "USDHC2_CD_B","WDOG1_B","SAI1_MCLK","USDHC1_WP",   "GPIO1_IO03","USB_OTG1_OC","CCM_PMIC_RDY","-","-" }, // pad GPIO_AD_B0_03 [GPIO1-3]
        {  "JTAG_TDO",     "FLEXCAN1_TX","USDHC1_WP","QTIMER2_TIMER0","ENET_MDIO","GPIO1_IO04","USB_OTG1_PWR","EWM_OUT_B","-","-" }, // pad GPIO_AD_B0_04 [GPIO1-4]
        {  "JTAG_TRSTB",   "FLEXCAN1_RX","USDHC1_CD_B","QTIMER2_TIMER1","ENET_MDC","GPIO1_IO05","USB_OTG1_ID","NMI_GLUE_NMI","-","-" },// pad GPIO_AD_B0_05 [GPIO1-5]
        {  "PIT_TRIGGER00","MQS_RIGHT","LPUART1_TX","QTIMER2_TIMER2","FLEXPWM2_PWMA03","GPIO1_IO06","REF_32K_OUT","-","-","-" }, // pad GPIO_AD_B0_06 [GPIO1-6]
        {  "PIT_TRIGGER01","MQS_LEFT","LPUART1_RX","QTIMER2_TIMER3","FLEXPWM2_PWMB03","GPIO1_IO07","REF_24M_OUT","-","-","-" }, // pad GPIO_AD_B0_07 [GPIO1-7]
        {  "ENET_TX_CLK",  "LPI2C3_SCL","LPUART1_CTS_B","KPP_COL00","ENET_REF_CLK1","GPIO1_IO08","ARM_CM7_TXEV","-","-","-" }, // pad GPIO_AD_B0_08 [GPIO1-8]
        {  "ENET_RDATA01", "LPI2C3_SDA","LPUART1_RTS_B","KPP_ROW00","-",      "GPIO1_IO09","ARM_CM7_RXEV","-","-","-" }, // pad GPIO_AD_B0_09 [GPIO1-9]
        {  "ENET_RDATA00", "LPSPI1_SCK","LPUART5_TX","KPP_COL01","FLEXPWM2_PWMA02","GPIO1_IO10","ARM_CM7_TRACE_CLK","-","-","-" }, // pad GPIO_AD_B0_10 [GPIO1-10]
        {  "ENET_RX_EN",   "LPSPI1_PCS0","LPUART5_RX","KPP_ROW01","FLEXPWM2_PWMB02","GPIO1_IO11","ARM_CM7_TRACE_SWO","-","-","-" }, // pad GPIO_AD_B0_11 [GPIO1-11]
        {  "ENET_RX_ER",   "LPSPI1_SDO","LPUART3_CTS_B","KPP_COL02","FLEXPWM2_PWMA01","GPIO1_IO12","ARM_CM7_TRACE00","SNVS_HP_VIO_5_CTL","-","-" }, // pad GPIO_AD_B0_12 [GPIO1-12]
        {  "ENET_TX_EN",   "LPSPI1_SDI","LPUART3_RTS_B","KPP_ROW02","FLEXPWM2_PWMB01","GPIO1_IO13","ARM_CM7_TRACE01","SNVS_HP_VIO_5_B","-","-" }, // pad GPIO_AD_B0_13 [GPIO1-13]
        {  "ENET_TDATA00", "FLEXCAN2_TX","LPUART3_TX","KPP_COL03","FLEXPWM2_PWMA00","GPIO1_IO14","ARM_CM7_TRACE02","WDOG1_ANY","-","-" }, // pad GPIO_AD_B0_14 [GPIO1-14]
        {  "ENET_TDATA01", "FLEXCAN2_RX","LPUART3_RX","KPP_ROW03","FLEXPWM2_PWMB00","GPIO1_IO15","ARM_CM7_TRACE03","-","-","-" }, // pad GPIO_AD_B0_15 [GPIO1-15]
        {  "SEMC_READY",   "FLEXSPI_A_DATA03","FLEXCAN2_TX","SAI1_MCLK","FLEXIO1_FLEXIO15","GPIO1_IO16","ENET_1588_EVENT2_OUT","KPP_COL04","-","-" }, // pad GPIO_AD_B1_00 [GPIO1-16]
        {  "SEMC_CSX00",   "FLEXSPI_A_SCLK","FLEXCAN2_RX","SAI1_TX_BCLK","FLEXIO1_FLEXIO14","GPIO1_IO17","ENET_1588_EVENT2_IN","KPP_ROW04","-","-" }, // pad GPIO_AD_B1_01 [GPIO1-17]
        {  "SEMC_CSX01",   "FLEXSPI_A_DATA00","LPSPI4_SCK","SAI1_TX_SYNC","FLEXIO1_FLEXIO13","GPIO1_IO18","ENET_1588_EVENT3_OUT","KPP_COL05","-","-" }, // pad GPIO_AD_B1_02 [GPIO1-18]
        {  "SEMC_CSX02",   "FLEXSPI_A_DATA02","LPSPI4_PCS0","SAI1_TX_DATA00","FLEXIO1_FLEXIO12","GPIO1_IO19","ENET_1588_EVENT3_IN","KPP_ROW05","-" }, // pad GPIO_AD_B1_03 [GPIO1-19]
        {  "SEMC_CSX03",   "FLEXSPI_A_DATA01","LPSPI4_SDO","SAI1_RX_SYNC","FLEXIO1_FLEXIO11","GPIO1_IO20","LPSPI1_PCS1","KPP_COL06","-","-" }, // pad GPIO_AD_B1_04 [GPIO1-20]
        {  "USDHC1_WP",    "FLEXSPI_A_SS0_B","LPSPI4_SDI","SAI1_RX_DATA00","FLEXIO1_FLEXIO10","GPIO1_IO21","LPSPI1_PCS2","KPP_ROW06","-","-" }, // pad GPIO_AD_B1_05 [GPIO1-21]
        {  "USDHC1_RESET_B","FLEXPWM1_PWMA00","LPUART2_CTS_B","SAI1_RX_BCLK","FLEXIO1_FLEXIO09","GPIO1_IO22","LPSPI1_PCS3","KPP_COL07","-","-" }, // pad GPIO_AD_B1_06 [GPIO1-22]
        {  "USDHC1_VSELECT","FLEXPWM1_PWMB00","LPUART2_RTS_B","SAI1_TX_DATA01","FLEXIO1_FLEXIO08","GPIO1_IO23","LPSPI3_PCS3","KPP_ROW07","-","-" }, // pad GPIO_AD_B1_07 [GPIO1-23]
        {  "LPI2C2_SCL",   "FLEXPWM1_PWMA01","LPUART2_TX","SAI1_TX_DATA02","FLEXIO1_FLEXIO07","GPIO1_IO24","LPSPI3_PCS2","XBAR1_INOUT12","-","-" }, // pad GPIO_AD_B1_08 [GPIO1-24]
        {  "LPI2C2_SDA",   "FLEXPWM1_PWMB01","LPUART2_RX","SAI1_TX_DATA03","FLEXIO1_FLEXIO06","GPIO1_IO25","LPSPI3_PCS1","XBAR1_INOUT13","-","-" }, // pad GPIO_AD_B1_09 [GPIO1-25]
        {  "USB_OTG1_PWR", "FLEXPWM1_PWMA02","LPUART4_TX","USDHC1_CD_B","FLEXIO1_FLEXIO05","GPIO1_IO26","GPT2_CAPTURE1","-","-","-" }, // pad GPIO_AD_B1_10 [GPIO1-26]
        {  "USB_OTG1_ID",  "FLEXPWM1_PWMB02","LPUART4_RX","USDHC1_WP","FLEXIO1_FLEXIO04","GPIO1_IO27","GPT2_COMPARE1","-","-","-" }, // pad GPIO_AD_B1_11 [GPIO1-27]
        {  "USB_OTG1_OC",  "ACMP1_OUT","LPSPI3_SCK","USDHC2_CD_B","FLEXIO1_FLEXIO03","GPIO1_IO28","FLEXPWM1_PWMA03","-","-","-" }, // pad GPIO_AD_B1_12 [GPIO1-28]
        {  "LPI2C1_HREQ",  "ACMP2_OUT","LPSPI3_PCS0","USDHC2_WP","FLEXIO1_FLEXIO02","GPIO1_IO29","FLEXPWM1_PWMB03","-","-","-" }, // pad GPIO_AD_B1_13 [GPIO1-29]
        {  "LPI2C1_SCL",   "ACMP3_OUT","LPSPI3_SDO","ENET_1588_EVENT0_OUT","FLEXIO1_FLEXIO01","GPIO1_IO30","-","-","-","-" }, // pad GPIO_AD_B1_14 [GPIO1-30]
        {  "LPI2C1_SDA",   "ACMP4_OUT","LPSPI3_SDI","ENET_1588_EVENT0_IN","FLEXIO1_FLEXIO00","GPIO1_IO31","-","-","-","-" }, // pad GPIO_AD_B1_14 [GPIO1-31]
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7     ALT8                GPIO2
        {  "SEMC_DATA00",  "QTIMER2_TIMER0","LPUART4_CTS_B","SPDIF_SR_CLK","LPSPI2_SCK","GPIO2_IO00","FLEXCAN1_TX","PIT_TRIGGER02","-","-" }, // pad GPIO_EMC_00 [GPIO2-0]
        {  "SEMC_DATA01",  "QTIMER2_TIMER1","LPUART4_RTS_B","SPDIF_OUT","LPSPI2_PCS0","GPIO2_IO01","FLEXCAN1_RX","PIT_TRIGGER03","-","-" }, // pad GPIO_EMC_01 [GPIO2-1]
        {  "SEMC_DATA02",  "QTIMER2_TIMER2","LPUART4_TX","SPDIF_LOCK","LPSPI2_SDO","GPIO2_IO02","LPI2C1_SCL","-","-","-" }, // pad GPIO_EMC_02 [GPIO2-2]
        {  "SEMC_DATA03",  "QTIMER2_TIMER3","LPUART4_RX","SPDIF_EXT_CLK","LPSPI2_SDI","GPIO2_IO03","LPI2C1_SDA","-","-","-" }, // pad GPIO_EMC_03 [GPIO2-3]
        {  "SEMC_DATA04",  "XBAR1_INOUT04","SPDIF_OUT","SAI2_TX_BCLK","FLEXIO1_FLEXIO16","GPIO2_IO04","-","-","-","-" }, // pad GPIO_EMC_04 [GPIO2-4]
        {  "SEMC_DATA05",  "XBAR1_INOUT05","SPDIF_IN","SAI2_TX_SYNC","FLEXIO1_FLEXIO17","GPIO2_IO05","-","-","-","-" }, // pad GPIO_EMC_05 [GPIO2-5]
        {  "SEMC_DATA06",  "XBAR1_INOUT06","LPUART3_TX","SAI2_TX_DATA","FLEXIO1_FLEXIO18","GPIO2_IO06","-","-","-","-" }, // pad GPIO_EMC_06 [GPIO2-6]
        {  "SEMC_DATA07",  "XBAR1_INOUT07","LPUART3_RX","SAI2_RX_SYNC","FLEXIO1_FLEXIO19","GPIO2_IO07","-","-","-","-" }, // pad GPIO_EMC_07 [GPIO2-7]
        {  "SEMC_DM00",    "XBAR1_INOUT08","FLEXCAN2_TX","SAI2_RX_DATA","FLEXIO1_FLEXIO20","GPIO2_IO08","-","-","-","-" }, // pad GPIO_EMC_08 [GPIO2-8]
        {  "SEMC_WE",      "XBAR1_INOUT09","FLEXCAN2_RX","SAI2_RX_BCLK","FLEXIO1_FLEXIO21","GPIO2_IO09","-","-","-","-" }, // pad GPIO_EMC_09 [GPIO2-9]
        {  "SEMC_CAS",     "XBAR1_INOUT10","LPI2C4_SDA","SAI1_TX_SYNC","LPSPI2_SCK","GPIO2_IO10","FLEXPWM2_PWMX00","-","-","-" }, // pad GPIO_EMC_10 [GPIO2-10]
        {  "SEMC_RAS",     "XBAR1_INOUT11","LPI2C4_SCL","SAI1_TX_BCLK","LPSPI2_PCS0","GPIO2_IO11","FLEXPWM2_PWMX01","-","-","-" }, // pad GPIO_EMC_11 [GPIO2-11]
        {  "SEMC_CS0",     "XBAR1_INOUT12","LPUART6_TX","SAI1_TX_DATA00","LPSPI2_SDO","GPIO2_IO12","FLEXPWM2_PWMX02","-","-","-" }, // pad GPIO_EMC_12 [GPIO2-12]
        {  "SEMC_BA0",     "XBAR1_INOUT13","LPUART6_RX","SAI1_RX_DATA00","LPSPI2_SDI","GPIO2_IO13","FLEXPWM2_PWMX03","CCM_PMIC_RDY","-","-" }, // pad GPIO_EMC_13 [GPIO2-13]
        {  "SEMC_BA1",     "XBAR1_INOUT14","LPUART6_CTS_B","SAI1_RX_BCLK","LPSPI2_PCS1","GPIO2_IO14","FLEXCAN1_TX","-","-","-" }, // pad GPIO_EMC_14 [GPIO2-14]
        {  "SEMC_ADDR10",  "XBAR1_INOUT15","LPUART6_RTS_B","SAI1_RX_SYNC","WDOG1_B","GPIO2_IO15","FLEXCAN1_RX","-","-","-" }, // pad GPIO_EMC_15 [GPIO2-15]
        {  "SEMC_ADDR00",  "-",     "MQS_RIGHT", "SAI2_MCLK",  "-",           "GPIO2_IO16","SRC_BOOT_MODE00","-","-","-" }, // pad GPIO_EMC_16 [GPIO2-16]
        {  "SEMC_ADDR01",  "-",     "MQS_LEFT",  "SAI3_MCLK",  "-",           "GPIO2_IO17","SRC_BOOT_MODE01","-","-","-" }, // pad GPIO_EMC_17 [GPIO2-17]
        {  "SEMC_ADDR02",  "XBAR1_INOUT16","LPI2C2_SDA","SAI1_RX_SYNC","FLEXIO1_FLEXIO22","GPIO2_IO18","SRC_BT_CFG00","-","-","-" }, // pad GPIO_EMC_18 [GPIO2-18]
        {  "SEMC_ADDR03",  "XBAR1_INOUT17","LPI2C2_SCL","SAI1_RX_BCLK","FLEXIO1_FLEXIO23","GPIO2_IO19","SRC_BT_CFG01","-","-","-" }, // pad GPIO_EMC_19 [GPIO2-19]
        {  "SEMC_ADDR04",  "FLEXPWM1_PWMA03","LPUART2_CTS_B","SAI1_MCLK","FLEXIO1_FLEXIO24","GPIO2_IO20","SRC_BT_CFG02","-","-","-" }, // pad GPIO_EMC_20 [GPIO2-20]
        {  "SEMC_ADDR05",  "FLEXPWM1_PWMB03","LPUART2_RTS_B","SAI1_RX_DATA00","FLEXIO1_FLEXIO25","GPIO2_IO21","SRC_BT_CFG03","-","-","-" }, // pad GPIO_EMC_21 [GPIO2-21]
        {  "SEMC_ADDR06",  "FLEXPWM1_PWMA02","LPUART2_TX","SAI1_TX_DATA03","FLEXIO1_FLEXIO26","GPIO2_IO22","SRC_BT_CFG04","-","-","-" }, // pad GPIO_EMC_22 [GPIO2-22]
        {  "SEMC_ADDR07",  "FLEXPWM1_PWMB02","LPUART2_RX","SAI1_TX_DATA02","FLEXIO1_FLEXIO27","GPIO2_IO23","SRC_BT_CFG05","-","-","-" }, // pad GPIO_EMC_23 [GPIO2-23]
        {  "SEMC_ADDR08",  "FLEXPWM1_PWMA01","LPUART8_CTS_B","SAI1_TX_DATA01","FLEXIO1_FLEXIO28","GPIO2_IO24","SRC_BT_CFG06","-","-","-" }, // pad GPIO_EMC_24 [GPIO2-24]
        {  "SEMC_ADDR09",  "FLEXPWM1_PWMB01","LPUART8_RTS_B","SAI1_TX_DATA00","FLEXIO1_FLEXIO29","GPIO2_IO25","SRC_BT_CFG07","-","-","-" }, // pad GPIO_EMC_25 [GPIO2-25]
        {  "SEMC_ADDR11",  "FLEXPWM1_PWMA00","LPUART8_TX","SAI1_TX_BCLK","FLEXIO1_FLEXIO30","GPIO2_IO26","SRC_BT_CFG08","-","-","-" }, // pad GPIO_EMC_26 [GPIO2-26]
        {  "SEMC_ADDR12",  "FLEXPWM1_PWMB00","LPUART8_RX","SAI1_TX_SYNC","FLEXIO1_FLEXIO31","GPIO2_IO27","SRC_BT_CFG09","-","-","-" }, // pad GPIO_EMC_27 [GPIO2-27]
        {  "SEMC_DQS",     "FLEXPWM2_PWMA03","XBAR1_INOUT18","SAI3_MCLK","EWM_OUT_B","GPIO2_IO28","GPT2_CAPTURE2","FLEXPWM1_PWMX00","-","-" }, // pad GPIO_EMC_28 [GPIO2-28]
        {  "SEMC_CKE",     "FLEXPWM2_PWMB03","XBAR1_INOUT19","SAI3_RX_BCLK","WDOG2_RST_B_DEB","GPIO2_IO29","GPT2_COMPARE2","FLEXPWM1_PWMX01","-","-" }, // pad GPIO_EMC_29 [GPIO2-29]
        {  "SEMC_CLK",     "FLEXPWM2_PWMA02","LPUART4_CTS_B","SAI3_RX_SYNC","WDOG1_RST_B_DEB","GPIO2_IO30","GPT2_COMPARE3","FLEXPWM1_PWMX02","-","-" }, // pad GPIO_EMC_30 [GPIO2-30]
        {  "SEMC_DM01",    "FLEXPWM2_PWMB02","LPUART4_RTS_B","SAI3_RX_DATA","WDOG2_B","GPIO2_IO31","GPT2_CLK","FLEXPWM1_PWMX03","-","-" }  // pad GPIO_EMC_31 [GPIO2-31]
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7     ALT 8         GPIO3
        {  "-",            "QTIMER1_TIMER0","LPUART4_TX","SAI3_TX_DATA","-",  "GPIO3_IO00","-",        "REF_24M_OUT","-","-" }, // pad GPIO_EMC_32 [GPIO3-0]
        {  "-",            "QTIMER1_TIMER1","LPUART4_RX","SAI3_TX_BCLK","-",  "GPIO3_IO01","-",        "-","-","-" }, // pad GPIO_EMC_33 [GPIO3-1]
        {  "-",            "QTIMER1_TIMER2","-", "SAI3_TX_SYNC","-",          "GPIO3_IO02","-",        "-","-","-" }, // pad GPIO_EMC_34 [GPIO3-2]
        {  "-",            "QTIMER1_TIMER3","-", "-",          "-",           "GPIO3_IO03","-",        "-",      "-","-" }, // pad GPIO_EMC_35 [GPIO3-3]
        {  "SEMC_DATA12",  "FLEXPWM2_PWMA01","LPUART5_CTS_B","CCM_PMIC_RDY","LPSPI4_PCS1","GPIO3_IO04","ENET_RX_CLK","USDHC1_WP","-","-" }, // pad GPIO_EMC_36 [GPIO3-4]
        {  "SEMC_DATA13",  "FLEXPWM2_PWMB01","LPUART5_RTS_B","MQS_RIGHT","LPSPI4_PCS2","GPIO3_IO05","ENET_RX_CLK","USDHC1_VSELECT","-","-" }, // pad GPIO_EMC_37 [GPIO3-5]
        {  "SEMC_DATA14",  "FLEXPWM2_PWMA00","LPUART5_TX","MQS_LEFT","LPSPI4_PCS3","GPIO3_IO06","ENET_RDATA02","USDHC1_CD_B","-","-" }, // pad GPIO_EMC_38 [GPIO3-6]
        {  "SEMC_DATA15",  "FLEXPWM2_PWMB00","LPUART5_RX","USB_OTG1_OC","WDOG1_B","GPIO3_IO07","ENET_TX_ER","GPT1_CLK","-","-" }, // pad GPIO_EMC_39 [GPIO3-7]
        {  "SEMC_CSX00",   "XBAR1_INOUT18","SPDIF_OUT","USB_OTG1_ID","ENET_MDIO","GPIO3_IO08","ENET_TDATA03","GPT1_COMPARE3","-","-" }, // pad GPIO_EMC_40 [GPIO3-8]
        {  "SEMC_READY",   "XBAR1_INOUT19","SPDIF_IN","USB_OTG1_PWR","ENET_MDC","GPIO3_IO09","ENET_TDATA02","GPT1_COMPARE2","-","-" }, // pad GPIO_EMC_41 [GPIO3-9]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "USDHC1_DATA2", "QTIMER1_TIMER0","SAI1_MCLK","SAI2_MCLK","LPI2C3_SCL","GPIO3_IO13","FLEXSPI_A_SS1_B","XBAR1_INOUT14","-","-" }, // pad GPIO_SD_B0_00 [GPIO3-13]
        {  "USDHC1_DATA3", "QTIMER1_TIMER1","REF_24M_OUT","SAI2_RX_SYNC","LPI2C3_SDA","GPIO3_IO14","FLEXSPI_B_SS1_B","XBAR1_INOUT15","-","-" }, // pad GPIO_SD_B0_01 [GPIO3-14]
        {  "USDHC1_CMD",   "QTIMER1_TIMER2","LPUART7_CTS_B","SAI2_RX_BCLK","LPSPI1_SCK","GPIO3_IO15","ENET_MDIO","XBAR1_INOUT16","-","-" }, // pad GPIO_SD_B0_02 [GPIO3-15]
        {  "USDHC1_CLK",   "QTIMER1_TIMER3","LPUART7_RTS_B","SAI2_RX_DATA","LPSPI1_PCS0","GPIO3_IO16","ENET_MDC","-","-","-" }, // pad GPIO_SD_B0_03 [GPIO3-16]
        {  "USDHC1_DATA0", "FLEXCAN2_TX","LPUART7_TX","SAI2_TX_DATA","LPSPI1_SDO","GPIO3_IO17","FLEXSPI_B_SS0_B","-","-","-" }, // pad GPIO_SD_B0_04 [GPIO3-17]
        {  "USDHC1_DATA1", "FLEXCAN2_RX","LPUART7_RX","SAI2_TX_BCLK","LPSPI1_SDI","GPIO3_IO18","FLEXSPI_B_DQS","-","-","-" }, // pad GPIO_SD_B0_05 [GPIO3-18]
        {  "USDHC1_CD_B",  "USDHC1_RESET_B","REF_32K_OUT","SAI2_TX_SYNC","WDOG1_B","GPIO3_IO19","XBAR1_INOUT17","-","-","-" }, // pad GPIO_SD_B0_06 [GPIO3-19]
        {  "USDHC2_DATA2", "FLEXSPI_B_DATA03","LPUART6_TX","XBAR1_INOUT10","FLEXCAN1_TX","GPIO3_IO20","-","-","-","-" }, // pad GPIO_SD_B1_00 [GPIO3-20]
        {  "USDHC2_DATA3", "FLEXSPI_B_SCLK","LPUART6_RX","FLEXSPI_A_SS1_B","FLEXCAN1_RX","GPIO3_IO21","-","-","-","-" }, // pad GPIO_SD_B1_01 [GPIO3-21]
        {  "USDHC2_CMD",   "FLEXSPI_B_DATA00","LPUART8_TX","LPI2C4_SCL","ENET_1588_EVENT1_OUT","GPIO3_IO22","CCM_CLKO1","-","-","-" }, // pad GPIO_SD_B1_02 [GPIO3-22]
        {  "USDHC2_CLK",   "FLEXSPI_B_DATA02","LPUART8_RX","LPI2C4_SDA","ENET_1588_EVENT1_IN","GPIO3_IO23","CCM_CLKO2","-","-","-" }, // pad GPIO_SD_B1_03 [GPIO3-23]
        {  "USDHC2_DATA0", "FLEXSPI_B_DATA01","ENET_TX_CLK","ENET_REF_CLK1","EWM_OUT_B","GPIO3_IO24","CCM_WAIT","-","-","-" }, // pad GPIO_SD_B1_04 [GPIO3-24]
        {  "USDHC2_DATA1", "FLEXSPI_A_DQS","ENET_RDATA01","SAI3_MCLK","FLEXSPI_B_SS0_B","GPIO3_IO25","CCM_PMIC_RDY","-","-","-" }, // pad GPIO_SD_B1_05 [GPIO3-25]
        {  "USDHC2_CD_B",  "FLEXSPI_A_DATA03","ENET_RDATA00","SAI3_TX_BCLK","LPSPI2_PCS0","GPIO3_IO26","CCM_STOP","-","-","-" }, // pad GPIO_SD_B1_06 [GPIO3-26]
        {  "USDHC2_RESET_B","FLEXSPI_A_SCLK","ENET_RX_EN","SAI3_TX_SYNC","LPSPI2_SCK","GPIO3_IO27","-","-","-","-" }, // pad GPIO_SD_B1_07 [GPIO3-27]
        {  "USDHC2_DATA4", "FLEXSPI_A_DATA00","ENET_RX_ER","SAI3_TX_DATA","LPSPI2_SDO","GPIO3_IO28","-","-","-","-" }, // pad GPIO_SD_B1_08 [GPIO3-28]
        {  "USDHC2_DATA5", "FLEXSPI_A_DATA02","ENET_TX_EN","SAI3_RX_BCLK","LPSPI2_SDI","GPIO3_IO29","-","-","-","-" }, // pad GPIO_SD_B1_09 [GPIO3-29]
        {  "USDHC2_DATA6", "FLEXSPI_A_DATA01","ENET_TDATA00","SAI3_RX_SYNC","LPSPI2_PCS2","GPIO3_IO30","-","-","-","-" }, // pad GPIO_SD_B1_10 [GPIO3-30]
        {  "USDHC2_DATA7", "FLEXSPI_A_SS0_B","ENET_TDATA01","SAI3_RX_DATA","LPSPI2_PCS3","GPIO3_IO31","-","-","-","-" }, // pad GPIO_SD_B1_11 [GPIO3-31]
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7    ALT 8          GPIO4
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" }

    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7       ALT8       GPIO5
        {  "-",            "-",     "-",         "-",          "-",           "GPIO5_IO00","-",        "NMI_GLUE_NMI","-","-" }, // pad WAKEUP [GPIO5]
        {  "SNVS_LP_PMIC_ON_REQ","-","-",        "-",          "-",           "GPIO5_IO01","-",        "-","-","-" }, // pad PMIC_ON_REQ
        {  "CCM_PMIC_VSTBY_REQ","-","-",         "-",          "-",           "GPIO5_IO02","-",        "-","-","-" }, // pad PMIC_STBY_REQ
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" }
    },
};
#else                                                                    // defined iMX_RT102X / iMX_RT105X / iMX_RT106X
#if PIN_COUNT == PIN_COUNT_100_PIN                                       // 100 pin LQFP
    #define _PIN_COUNT              0
#elif PIN_COUNT == PIN_COUNT_144_PIN
    #define _PIN_COUNT              1                                    // 144 pin LQFP
#elif PIN_COUNT == PIN_COUNT_196_PIN
    #define _PIN_COUNT              2                                    // 196 pin MAPBGA
#else
    #error "Package not clearly defined!!"
#endif

#if defined _PIN_DEFAULTS
static const unsigned char cPinDefaults[PORTS_AVAILABLE][PORT_WIDTH] = {
    {                                                                    // GPIO1
    #if defined iMX_RT105X || defined iMX_RT106X
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        GPIO_AD_B0_04_SRC_BOOT_MODE00,
        GPIO_AD_B0_05_SRC_BOOT_MODE01,
        GPIO_AD_B0_06_JTAG_TMS,
        GPIO_AD_B0_07_JTAG_TCK,
        GPIO_AD_B0_08_JTAG_MOD,
        GPIO_AD_B0_09_JTAG_TDI,
        GPIO_AD_B0_10_JTAG_TDO,
        GPIO_AD_B0_11_JTAG_TRSTB,
    #else
        GPIO_AD_B0_00_JTAG_TMS,
        GPIO_AD_B0_01_JTAG_TCK,
        GPIO_AD_B0_02_JTAG_MOD,
        GPIO_AD_B0_03_JTAG_TDI,
        GPIO_AD_B0_04_JTAG_TDO,
        GPIO_AD_B0_05_JTAG_TRSTB,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
    #endif
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO2
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
  #if defined iMX_RT105X || defined iMX_RT106X
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
  #else
        GPIO_EMC_16_SRC_BOOT_MODE00,
        GPIO_EMC_17_SRC_BOOT_MODE01,
  #endif
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO3
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO4
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
    {                                                                    // GPIO5
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO,
        IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO
    },
};
#endif

#define ALTERNATIVE_FUNCTIONS   8                                        // GPIO plus 8 possible peripheral/extra/system functions

static const char *cPinNumber[PORTS_AVAILABLE + 1][PORT_WIDTH][3] = {
    {
        // 100LQFP     144LQFPP  196MAPBGA                               GPIO_AD_B0/1 [GPIO1]
        {  "78",       "111",    "M14",    },                            // GPIO_AD_B0_00 [GPIO1.IO[0]] jtag_mux.TMS
        {  "77",       "110",    "H10",    },                            // GPIO_AD_B0_01 [GPIO1.IO[1]] jtag_mux.TCK
        {  "76",       "109",    "M11",    },                            // GPIO_AD_B0_02 [GPIO1.IO[2]] jtag_mux.MOD
        {  "75",       "108",    "G11",    },                            // GPIO_AD_B0_03 [GPIO1.IO[3]] jtag_mux.TDI
        {  "74",       "107",    "F11",    },                            // GPIO_AD_B0_04 [GPIO1.IO[4]] jtag_mux.TDO
        {  "74",       "106",    "G14",    },                            // GPIO_AD_B0_05 [GPIO1.IO[5]] jtag_mux.TRSTB
        {  "72",       "105",    "E14",    },                            // GPIO_AD_B0_06 [GPIO1.IO[6]]
        {  "68",       "101",    "F12",    },                            // GPIO_AD_B0_07 [GPIO1.IO[7]]
        {  "67",       "100",    "F13",    },                            // GPIO_AD_B0_08 [GPIO1.IO[8]]
        {  "66",       "99",     "F14",    },                            // GPIO_AD_B0_09 [GPIO1.IO[9]]
        {  "65",       "98",     "G13",    },                            // GPIO_AD_B0_10 [GPIO1.IO[10]]
        {  "64",       "97",     "G10",    },                            // GPIO_AD_B0_11 [GPIO1.IO[11]]
        {  "63",       "96",     "K14",    },                            // GPIO_AD_B0_12 [GPIO1.IO[12]]
        {  "62",       "95",     "L14",    },                            // GPIO_AD_B0_13 [GPIO1.IO[13]]
        {  "61",       "94",     "H14",    },                            // GPIO_AD_B0_14 [GPIO1.IO[14]]
        {  "60",       "93",     "L10",    },                            // GPIO_AD_B0_15 [GPIO1.IO[15]]
        {  "-",        "92",     "J11",    },                            // GPIO_AD_B1_00 [GPIO1.IO[16]]
        {  "-",        "91",     "K11"     },                            // GPIO_AD_B1_01 [GPIO1.IO[17]]
        {  "-",        "90",     "L11",    },                            // GPIO_AD_B1_02 [GPIO1.IO[18]]
        {  "-",        "89",     "M12",    },                            // GPIO_AD_B1_03 [GPIO1.IO[19]]
        {  "-",        "88",     "L12",    },                            // GPIO_AD_B1_04 [GPIO1.IO[20]]
        {  "-",        "87",     "K12",    },                            // GPIO_AD_B1_05 [GPIO1.IO[21]]
        {  "-",        "84",     "J12",    },                            // GPIO_AD_B1_06 [GPIO1.IO[22]]
        {  "-",        "83",     "K10",    },                            // GPIO_AD_B1_07 [GPIO1.IO[23]]
        {  "-",        "82",     "H13",    },                            // GPIO_AD_B1_08 [GPIO1.IO[24]]
        {  "-",        "81",     "M13",    },                            // GPIO_AD_B1_09 [GPIO1.IO[25]]
        {  "57",       "80",     "L13",    },                            // GPIO_AD_B1_10 [GPIO1.IO[26]]
        {  "56",       "79",     "J13",    },                            // GPIO_AD_B1_11 [GPIO1.IO[27]]
        {  "55",       "78",     "H12",    },                            // GPIO_AD_B1_12 [GPIO1.IO[28]]
        {  "54",       "76",     "H11",    },                            // GPIO_AD_B1_13 [GPIO1.IO[29]]
        {  "53",       "75",     "G12",    },                            // GPIO_AD_B1_14 [GPIO1.IO[30]]
        {  "52",       "74",     "J14",    },                            // GPIO_AD_B1_15 [GPIO1.IO[31]]
    },
    {
        // 100LQFP     144LQFP   196MAPBGA                               GPIO_EMC [GPIO2]             iMX_RT106X
        {  "-",        "18",     "D7",     },                            // GPIO_EMC_00 [GPIO2.IO[0]] GPIO_B0_00
        {  "-",        "17",     "E7",     },                            // GPIO_EMC_01 [GPIO2.IO[1]] GPIO_B0_01
        {  "-",        "16",     "E8",     },                            // GPIO_EMC_02 [GPIO2.IO[2]] GPIO_B0_02
        {  "-",        "15",     "D8",     },                            // GPIO_EMC_03 [GPIO2.IO[3]] GPIO_B0_03
        {  "7",        "14",     "C8",     },                            // GPIO_EMC_04 [GPIO2.IO[4]] GPIO_B0_04
        {  "6",        "13",     "B8",     },                            // GPIO_EMC_05 [GPIO2.IO[5]] GPIO_B0_05
        {  "4",        "12",     "A8",     },                            // GPIO_EMC_06 [GPIO2.IO[6]] GPIO_B0_06
        {  "3",        "10",     "A9",     },                            // GPIO_EMC_07 [GPIO2.IO[7]] GPIO_B0_07
        {  "2",        "9",      "B9",     },                            // GPIO_EMC_08 [GPIO2.IO[8]] GPIO_B0_08
        {  "1",        "8",      "C9",     },                            // GPIO_EMC_09 [GPIO2.IO[9]] GPIO_B0_09
        {  "-",        "7",      "D9",     },                            // GPIO_EMC_10 [GPIO2.IO[10]] GPIO_B0_10
        {  "-",        "4",      "A10",    },                            // GPIO_EMC_11 [GPIO2.IO[11]] GPIO_B0_11
        {  "-",        "3",      "C10",    },                            // GPIO_EMC_12 [GPIO2.IO[12]] GPIO_B0_12
        {  "-",        "2",      "D10",    },                            // GPIO_EMC_13 [GPIO2.IO[13]] GPIO_B0_13
        {  "-",        "1",      "E10",    },                            // GPIO_EMC_14 [GPIO2.IO[14]] GPIO_B0_14
        {  "-",        "143",    "E11",    },                            // GPIO_EMC_15 [GPIO2.IO[15]] GPIO_B0_15
        {  "100",      "142",    "A11",    },                            // GPIO_EMC_16 [GPIO2.IO[16]] src.BOOT_MODE[0] GPIO_B1_00
        {  "99",       "141",    "B11",    },                            // GPIO_EMC_17 [GPIO2.IO[17]] src.BOOT_MODE[1] GPIO_B1_01
        {  "98",       "140",    "C11",    },                            // GPIO_EMC_18 [GPIO2.IO[18]] GPIO_B1_02
        {  "97",       "139",    "D11",    },                            // GPIO_EMC_19 [GPIO2.IO[19]] GPIO_B1_03
        {  "96",       "138",    "E12",    },                            // GPIO_EMC_20 [GPIO2.IO[20]] GPIO_B1_04
        {  "94",       "137",    "D12",    },                            // GPIO_EMC_21 [GPIO2.IO[21]] GPIO_B1_05
        {  "93",       "136",    "C12",    },                            // GPIO_EMC_22 [GPIO2.IO[22]] GPIO_B1_06
        {  "90",       "133",    "B12",    },                            // GPIO_EMC_23 [GPIO2.IO[23]] GPIO_B1_07
        {  "89",       "132",    "A12",    },                            // GPIO_EMC_24 [GPIO2.IO[24]] GPIO_B1_08
        {  "88",       "131",    "A13",    },                            // GPIO_EMC_25 [GPIO2.IO[25]] GPIO_B1_09
        {  "87",       "130",    "B13",    },                            // GPIO_EMC_26 [GPIO2.IO[26]] GPIO_B1_10
        {  "86",       "129",    "C13",    },                            // GPIO_EMC_27 [GPIO2.IO[27]] GPIO_B1_11
        {  "-",        "128",    "D13",    },                            // GPIO_EMC_28 [GPIO2.IO[28]] GPIO_B1_12
        {  "-",        "127",    "D14",    },                            // GPIO_EMC_29 [GPIO2.IO[29]] GPIO_B1_13
        {  "-",        "126",    "C14",    },                            // GPIO_EMC_30 [GPIO2.IO[30]] GPIO_B1_14
        {  "-",        "125",    "B14",    },                            // GPIO_EMC_31 [GPIO2.IO[31]] GPIO_B1_15
    },
    {
        // 100LQFP     144LQFP   196MAPBGA                               GPIO_EMC/GPIO_SD_B [GPIO3]      iMX_RT106X
        {  "85",       "124",    "L5",     },                            // GPIO_EMC_32   [GPIO3.IO[0]]  GPIO_SD_B1_00
        {  "84",       "123",    "M5",     },                            // GPIO_EMC_33   [GPIO3.IO[1]]  GPIO_SD_B1_01
        {  "83",       "122",    "M3",     },                            // GPIO_EMC_34   [GPIO3.IO[2]]  GPIO_SD_B1_02
        {  "82",       "121",    "M4",     },                            // GPIO_EMC_35   [GPIO3.IO[3]]  GPIO_SD_B1_03
        {  "-",        "120",    "P2",     },                            // GPIO_EMC_36   [GPIO3.IO[4]]  GPIO_SD_B1_04
        {  "-",        "119",    "N3",     },                            // GPIO_EMC_37   [GPIO3.IO[5]]  GPIO_SD_B1_05
        {  "-",        "118",    "L3",     },                            // GPIO_EMC_38   [GPIO3.IO[6]]  GPIO_SD_B1_06
        {  "-",        "117",    "L4",     },                            // GPIO_EMC_39   [GPIO3.IO[7]]  GPIO_SD_B1_07
        {  "-",        "116",    "P3",     },                            // GPIO_EMC_40   [GPIO3.IO[8]]  GPIO_SD_B1_08
        {  "-",        "115",    "N4",     },                            // GPIO_EMC_41   [GPIO3.IO[9]]  GPIO_SD_B1_09
        {  "-",        "-",      "P4",     },                            //               [GPIO3.IO[10]] GPIO_SD_B1_10
        {  "-",        "-",      "P5",     },                            //               [GPIO3.IO[11]] GPIO_SD_B1_11
        {  "-",        "-",      "J4",     },                            //               [GPIO3.IO[12]] GPIO_SD_B0_00
        {  "-",        "48",     "J3",     },                            // GPIO_SD_B0_00 [GPIO3.IO[13]] GPIO_SD_B0_01
        {  "-",        "47",     "J1",     },                            // GPIO_SD_B0_01 [GPIO3.IO[14]] GPIO_SD_B0_02
        {  "-",        "46",     "K1",     },                            // GPIO_SD_B0_02 [GPIO3.IO[15]] GPIO_SD_B0_03
        {  "-",        "45",     "H2",     },                            // GPIO_SD_B0_03 [GPIO3.IO[16]] GPIO_SD_B0_04
        {  "-",        "43",     "J2",     },                            // GPIO_SD_B0_04 [GPIO3.IO[17]] GPIO_SD_B0_05
        {  "-",        "42",     "D5",     },                            // GPIO_SD_B0_05 [GPIO3.IO[18]] GPIO_EMC_32
        {  "-",        "41",     "C4",     },                            // GPIO_SD_B0_06 [GPIO3.IO[19]] GPIO_EMC_33
        {  "22",       "33",     "D4",     },                            // GPIO_SD_B1_00 [GPIO3.IO[20]] GPIO_EMC_34
        {  "21",       "32",     "E5",     },                            // GPIO_SD_B1_01 [GPIO3.IO[21]] GPIO_EMC_35
        {  "17",       "30",     "C3",     },                            // GPIO_SD_B1_02 [GPIO3.IO[22]] GPIO_EMC_36
        {  "16",       "28",     "E4",     },                            // GPIO_SD_B1_03 [GPIO3.IO[23]] GPIO_EMC_37
        {  "15",       "27",     "D6",     },                            // GPIO_SD_B1_04 [GPIO3.IO[24]] GPIO_EMC_38
        {  "14",       "26",     "B7",     },                            // GPIO_SD_B1_05 [GPIO3.IO[25]] GPIO_EMC_39
        {  "13",       "25",     "A7",     },                            // GPIO_SD_B1_06 [GPIO3.IO[26]] GPIO_EMC_40
        {  "12",       "24",     "C7",     },                            // GPIO_SD_B1_07 [GPIO3.IO[27]] GPIO_EMC_41
        {  "11",       "23",     "23",     },                            // GPIO_SD_B1_08 [GPIO3.IO[28]]
        {  "10",       "22",     "22",     },                            // GPIO_SD_B1_09 [GPIO3.IO[29]]
        {  "9",        "21",     "21",     },                            // GPIO_SD_B1_10 [GPIO3.IO[30]]
        {  "8",        "19",     "19",     },                            // GPIO_SD_B1_11 [GPIO3.IO[31]]
    },
    {
        // 100LQFP     144LQFP   196MAPBGA                                  iMX_RT106X
        {  "-",        "-",      "E3",     },                            // GPIO_EMC_00  [GPIO4.IO[0]] 
        {  "-",        "-",      "F3",     },                            // GPIO_EMC_01  [GPIO4.IO[1]] 
        {  "-",        "-",      "F4",     },                            // GPIO_EMC_02  [GPIO4.IO[2]] 
        {  "-",        "-",      "G4",     },                            // GPIO_EMC_03  [GPIO4.IO[3]] 
        {  "-",        "-",      "F2",     },                            // GPIO_EMC_04  [GPIO4.IO[4]] 
        {  "-",        "-",      "G5",     },                            // GPIO_EMC_05  [GPIO4.IO[5]] 
        {  "-",        "-",      "H5",     },                            // GPIO_EMC_06  [GPIO4.IO[6]] 
        {  "-",        "-",      "H4",     },                            // GPIO_EMC_07  [GPIO4.IO[7]] 
        {  "-",        "-",      "H3",     },                            // GPIO_EMC_08  [GPIO4.IO[8]] 
        {  "-",        "-",      "C2",     },                            // GPIO_EMC_09  [GPIO4.IO[9]] 
        {  "-",        "-",      "G1",     },                            // GPIO_EMC_10  [GPIO4.IO[10]]
        {  "-",        "-",      "G3",     },                            // GPIO_EMC_11  [GPIO4.IO[11]]
        {  "-",        "-",      "H1",     },                            // GPIO_EMC_12  [GPIO4.IO[12]]
        {  "-",        "-",      "A6",     },                            // GPIO_EMC_13  [GPIO4.IO[13]]
        {  "-",        "-",      "B6",     },                            // GPIO_EMC_14  [GPIO4.IO[14]]
        {  "-",        "-",      "B1",     },                            // GPIO_EMC_15  [GPIO4.IO[15]]
        {  "-",        "-",      "A5",     },                            // GPIO_EMC_16  [GPIO4.IO[16]]
        {  "-",        "-",      "A4",     },                            // GPIO_EMC_17  [GPIO4.IO[17]]
        {  "-",        "-",      "B2",     },                            // GPIO_EMC_18  [GPIO4.IO[18]]
        {  "-",        "-",      "B4",     },                            // GPIO_EMC_19  [GPIO4.IO[19]]
        {  "-",        "-",      "A3",     },                            // GPIO_EMC_20  [GPIO4.IO[20]]
        {  "-",        "-",      "C1",     },                            // GPIO_EMC_21  [GPIO4.IO[21]]
        {  "-",        "-",      "F1",     },                            // GPIO_EMC_22  [GPIO4.IO[22]]
        {  "-",        "-",      "G2",     },                            // GPIO_EMC_23  [GPIO4.IO[23]]
        {  "-",        "-",      "D3",     },                            // GPIO_EMC_24  [GPIO4.IO[24]]
        {  "-",        "-",      "D2",     },                            // GPIO_EMC_25  [GPIO4.IO[25]]
        {  "-",        "-",      "B3",     },                            // GPIO_EMC_26  [GPIO4.IO[26]]
        {  "-",        "-",      "A2",     },                            // GPIO_EMC_27  [GPIO4.IO[27]]
        {  "-",        "-",      "D1",     },                            // GPIO_EMC_28  [GPIO4.IO[28]]
        {  "-",        "-",      "E1",     },                            // GPIO_EMC_29  [GPIO4.IO[29]]
        {  "-",        "-",      "C6",     },                            // GPIO_EMC_30  [GPIO4.IO[30]]
        {  "-",        "-",      "C5",     },                            // GPIO_EMC_31  [GPIO4.IO[31]]
    },
    {
        // 100LQFP     144LQFP   196MAPBGA                               [GPIO5]
        {  "-",        "52",     "L6",     },                            // WAKEUP [GPIO5.IO[00]]
        {  "-",        "53",     "K7",     },                            // PMIC_ON_REQ [GPIO5.IO[01]]
        {  "-",        "54",     "L7",     },                            // PMIC_STBY_REQ [GPIO5.IO[02]]
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
    },
#if defined PORT6
    {
        // 100LQFP     144LQFP   196MAPBGA                               [GPIO6]
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
    },
#endif
#if defined PORT7
    {
        // 100LQFP     144LQFP   196MAPBGA                               [GPIO7]
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
    },
#endif
#if defined PORT8
    {
        // 100LQFP     144LQFP   196MAPBGA                               [GPIO8]
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
    },
#endif
#if defined PORT9
    {
        // 100LQFP     144LQFP   196MAPBGA                               [GPIO9]
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
        {  "-",        "-",      "-",      },                            // 
    },
#endif
    { 
        // 100LQFP     144LQFP   196MAPBGA                               dedicated ADC pins
        {  "-",        "-",      "P11",    },                            // XTALI
        {  "-",        "-",      "N11",    },                            // XTALO
        {  "-",        "-",      "N9",     },                            // RTC_XTALI
        {  "-",        "-",      "P9",     },                            // RTC_XTALO
        {  "-",        "-",      "N7",     },                            // POR
        {  "-",        "-",      "M6",     },                            // ONOFF
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
        {  "-",        "-",      "-",      },                            //
    }
};

static int ADC_DEDICATED_CHANNEL[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_DEDICATED_MODULE[PORT_WIDTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int ADC_MUX_CHANNEL[PORTS_AVAILABLE][PORT_WIDTH] = {
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port A
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port B
#if PORTS_AVAILABLE > 2
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port C
#endif
#if PORTS_AVAILABLE > 3
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port D
#endif
#if PORTS_AVAILABLE > 4
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port E
#endif
#if PORTS_AVAILABLE > 5
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }, // port F
#endif
};


static const char *cPer[PORTS_AVAILABLE][PORT_WIDTH][10] = {
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7           ALT 8       ALT 9  GPIO1
#if defined iMX_RT105X || defined iMX_RT106X
        {  "FLEXPWM2_PWMA03","XBAR1_INOUT14","REF_CLK_32K","USB_OTG2_ID","LPI2C1_SCLS","GPIO1_IO00","USDHC1_RESET_B","LPSPI3_SCK","-" }, // pad GPIO_AD_B0_00 [GPIO1-0]
        {  "FLEXPWM2_PWMB03","XBAR1_INOUT15","REF_CLK_24M","USB_OTG1_ID","LPI2C1_SDAS","GPIO1_IO01","EWM_OUT_B","LPSPI3_SDO","-","-" }, // pad GPIO_AD_B0_01 [GPIO1-1]
        {  "FLEXCAN2_TX",  "XBAR1_INOUT16","LPUART6_TX","USB_OTG1_PWR","FLEXPWM1_PWMX00","GPIO1_IO02","LPI2C1_HREQ","LPSPI3_SDI","-","-" }, // pad GPIO_AD_B0_02 [GPIO1-2]
        {  "FLEXCAN2_RX",  "XBAR1_INOUT17","LPUART6_RX","USB_OTG1_OC","FLEXPWM1_PWMX01","GPIO1_IO03","REF_CLK_24M","LPSPI3_PCS0","-","-" }, // pad GPIO_AD_B0_03 [GPIO1-3]
        {  "SRC_BOOT_MODE00","MQS_RIGHT","ENET_TX_DATA03","SAI2_TX_SYNC","CSI_DATA09","GPIO1_IO04","PIT_TRIGGER00","LPSPI3_PCS1","-","-" }, // pad GPIO_AD_B0_04 [GPIO1-4]
        {  "SRC_BOOT_MODE01","MQS_LEFT","ENET_TX_DATA02","SAI2_TX_BCLK","CSI_DATA08","GPIO1_IO05","XBAR1_INOUT17","LPSPI3_PCS2","-","-" },// pad GPIO_AD_B0_05 [GPIO1-5]
        {  "JTAG_TMS",     "GPT2_COMPARE1","ENET_RX_CLK","SAI2_RX_BCLK","CSI_DATA07","GPIO1_IO06","XBAR1_INOUT18","LPSPI3_PCS3","-","-" }, // pad GPIO_AD_B0_06 [GPIO1-6]
        {  "JTAG_TCK",     "GPT2_COMPARE2","ENET_TX_ER","SAI2_RX_SYNC","CSI_DATA06","GPIO1_IO07","XBAR1_INOUT19","ENET_1588_EVENT3_OUT","-","-" }, // pad GPIO_AD_B0_07 [GPIO1-7]
        {  "JTAG_MOD",     "GPT2_COMPARE3","ENET_RX_DATA03","SAI2_RX_DATA","CSI_DATA05","GPIO1_IO08","XBAR1_IN20","ENET_1588_EVENT3_IN","-","-" }, // pad GPIO_AD_B0_08 [GPIO1-8]
        {  "JTAG_TDI",     "FLEXPWM2_PWMA03","ENET_RX_DATA02","SAI2_TX_DATA","CSI_DATA04","GPIO1_IO09","XBAR1_IN21","GPT2_CLK","-","SEMC_DQS4" }, // pad GPIO_AD_B0_09 [GPIO1-9]
        {  "JTAG_TDO",     "FLEXPWM1_PWMA03","ENET_CRS","SAI2_MCLK","CSI_DATA03","GPIO1_IO10","XBAR1_IN22","ENET_1588_EVENT0_OUT","FLEXCAN3_TX","ARM_TRACE_SWO" }, // pad GPIO_AD_B0_10 [GPIO1-10]
        {  "JTAG_TRSTB",   "FLEXPWM1_PWMB03","ENET_COL","WDOG1_WDOG_B","CSI_DATA02","GPIO1_IO11","XBAR1_IN23","ENET_1588_EVENT0_IN","FLEXCAN3_RX","SEMC_CLK6" }, // pad GPIO_AD_B0_11 [GPIO1-11]
        {  "LPI2C4_SCL",   "CCM_PMIC_READY","LPUART1_TX","WDOG2_WDOG_B","FLEXPWM1_PWMX02","GPIO1_IO12","ENET_1588_EVENT1_OUT","NMI_GLUE_NMI","-","-" }, // pad GPIO_AD_B0_12 [GPIO1-12]
        {  "LPI2C4_SDA",   "GPT1_CLK","LPUART1_RX","EWM_OUT_B","FLEXPWM1_PWMX03","GPIO1_IO13","ENET_1588_EVENT1_IN","REF_CLK_24M","-","-" }, // pad GPIO_AD_B0_13 [GPIO1-13]
        {  "USB_OTG2_OC",  "XBAR1_IN24","LPUART1_CTS_B","ENET_1588_EVENT0_OUT","CSI_VSYNC","GPIO1_IO14","FLEXCAN2_TX","-","FLEXCAN3_TX","-" }, // pad GPIO_AD_B0_14 [GPIO1-14]
        {  "USB_OTG2_PWR", "XBAR1_IN25","LPUART1_RTS_B","ENET_1588_EVENT0_IN","CSI_HSYNC","GPIO1_IO15","FLEXCAN2_RX","WDOG1_WDOG_RST_B_DEB","FLEXCAN3_RX","-" }, // pad GPIO_AD_B0_15 [GPIO1-15]
        {  "USB_OTG2_ID",  "QTIMER3_TIMER0","LPUART2_CTS_B","LPI2C1_SCL","WDOG1_B","GPIO1_IO16","USDHC1_WP","KPP_ROW07","ENET2_1588_EVENT0_OUT","FLEXIO3_FLEXIO00" }, // pad GPIO_AD_B1_00 [GPIO1-16]
        {  "USB_OTG1_PWR", "QTIMER3_TIMER1","LPUART2_RTS_B","LPI2C1_SDA","CCM_PMIC_READY","GPIO1_IO17","USDHC1_VSELECT","KPP_COL07","ENET2_1588_EVENT0_IN","FLEXIO3_FLEXIO01" }, // pad GPIO_AD_B1_01 [GPIO1-17]
        {  "USB_OTG1_ID",  "QTIMER3_TIMER2","LPUART2_TX","SPDIF_OUT","ENET_1588_EVENT2_OUT","GPIO1_IO18","USDHC1_CD_B","KPP_ROW06","GPT2_CLK","FLEXIO3_FLEXIO02" }, // pad GPIO_AD_B1_02 [GPIO1-18]
        {  "USB_OTG1_OC",  "QTIMER3_TIMER3","LPUART2_RX","SPDIF_IN","ENET_1588_EVENT2_IN","GPIO1_IO19","USDHC2_CD_B","KPP_COL06","GPT2_CAPTURE1","FLEXIO3_FLEXIO03" }, // pad GPIO_AD_B1_03 [GPIO1-19]
        {  "FLEXSPIB_DATA03","ENET_MDC","LPUART3_CTS_B","SPDIF_SR_CLK","CSI_PIXCLK","GPIO1_IO20","USDHC2_DATA0","KPP_ROW05","GPT2_CAPTURE2","FLEXIO3_FLEXIO04" }, // pad GPIO_AD_B1_04 [GPIO1-20]
        {  "FLEXSPIB_DATA02","ENET_MDIO","LPUART3_RTS_B","SPDIF_OUT","CSI_MCLK","GPIO1_IO21","USDHC2_DATA1","KPP_COL05","GPT2_COMPARE1","FLEXIO3_FLEXIO05" }, // pad GPIO_AD_B1_05 [GPIO1-21]
        {  "FLEXSPIB_DATA01","LPI2C3_SDA","LPUART3_TX","SPDIF_LOCK","CSI_VSYNC","GPIO1_IO22","USDHC2_DATA2","KPP_ROW04","-","-" }, // pad GPIO_AD_B1_06 [GPIO1-22]
        {  "FLEXSPIB_DATA00","LPI2C3_SCL","LPUART3_RX","SPDIF_EXT_CLK","CSI_HSYNC","GPIO1_IO23","USDHC2_DATA3","KPP_COL04","GPT2_COMPARE3","FLEXIO3_FLEXIO07" }, // pad GPIO_AD_B1_07 [GPIO1-23]
        {  "FLEXSPIA_SS1_B","FLEXPWM4_PWMA00","FLEXCAN1_TX","CCM_PMIC_READY","CSI_DATA09","GPIO1_IO24","USDHC2_CMD","KPP_ROW03","-","FLEXIO3_FLEXIO08" }, // pad GPIO_AD_B1_08 [GPIO1-24]
        {  "FLEXSPIA_DQS", "FLEXPWM4_PWMA01","FLEXCAN1_RX","SAI1_MCLK","CSI_DATA08","GPIO1_IO25","USDHC2_CLK","KPP_COL03","-","FLEXIO3_FLEXIO09" }, // pad GPIO_AD_B1_09 [GPIO1-25]
        {  "FLEXSPIA_DATA03","WDOG1_B","LPUART8_TX","SAI1_RX_SYNC","CSI_DATA07","GPIO1_IO26","USDHC2_WP","KPP_ROW02","ENET2_1588_EVENT1_OUT","FLEXIO3_FLEXIO10" }, // pad GPIO_AD_B1_10 [GPIO1-26]
        {  "FLEXSPIA_DATA02","EWM_OUT_B","LPUART8_RX","SAI1_RX_BCLK","CSI_DATA06","GPIO1_IO27","USDHC2_RESET_B","KPP_COL02","ENET2_1588_EVENT1_IN","FLEXIO3_FLEXIO11" }, // pad GPIO_AD_B1_11 [GPIO1-27]
        {  "FLEXSPIA_DATA01","ACMP_OUT00","LPSPI3_PCS0","SAI1_RX_DATA00","CSI_DATA05","GPIO1_IO28","USDHC2_DATA4","KPP_ROW01","ENET2_1588_EVENT2_OUT","FLEXIO3_FLEXIO12" }, // pad GPIO_AD_B1_12 [GPIO1-28]
        {  "FLEXSPIA_DATA00","ACMP_OUT01","LPSPI3_SDI","SAI1_TX_DATA00","CSI_DATA04","GPIO1_IO29","USDHC2_DATA5","KPP_COL01","ENET2_1588_EVENT2_IN","FLEXIO3_FLEXIO13" }, // pad GPIO_AD_B1_13 [GPIO1-29]
        {  "FLEXSPIA_SCLK","ACMP_OUT02","LPSPI3_SDO","SAI1_TX_BCLK","CSI_DATA03","GPIO1_IO30","USDHC2_DATA6","KPP_ROW00","ENET2_1588_EVENT3_OUT","FLEXIO3_FLEXIO14" }, // pad GPIO_AD_B1_14 [GPIO1-30]
        {  "FLEXSPIA_SS0_B","ACMP_OUT03","LPSPI3_SCK","SAI1_TX_SYNC","CSI_DATA02","GPIO1_IO31","USDHC2_DATA7","KPP_COL00","ENET2_1588_EVENT3_IN","FLEXIO3_FLEXIO15" }, // pad GPIO_AD_B1_14 [GPIO1-31]
#else
        {  "JTAG_TMS",     "-",     "-",         "-",          "-",           "GPIO1_IO00","-",        "GPT1_COMPARE1","-","-" }, // pad GPIO_AD_B0_00 [GPIO1-0]
        {  "JTAG_TCK",     "-",     "-",         "-",          "-",           "GPIO1_IO01","-",        "GPT1_CAPTURE2","-","-" }, // pad GPIO_AD_B0_01 [GPIO1-1]
        {  "JTAG_MOD",     "-",     "-",         "-",          "-",           "GPIO1_IO02","-",        "GPT1_CAPTURE1","-","-" }, // pad GPIO_AD_B0_02 [GPIO1-2]
        {  "JTAG_TDI",     "USDHC2_CD_B","WDOG1_B","SAI1_MCLK","USDHC1_WP",   "GPIO1_IO03","USB_OTG1_OC","CCM_PMIC_RDY","-","-" }, // pad GPIO_AD_B0_03 [GPIO1-3]
        {  "JTAG_TDO",     "FLEXCAN1_TX","USDHC1_WP","QTIMER2_TIMER0","ENET_MDIO","GPIO1_IO04","USB_OTG1_PWR","EWM_OUT_B","-","-" }, // pad GPIO_AD_B0_04 [GPIO1-4]
        {  "JTAG_TRSTB",   "FLEXCAN1_RX","USDHC1_CD_B","QTIMER2_TIMER1","ENET_MDC","GPIO1_IO05","USB_OTG1_ID","NMI_GLUE_NMI","-","-" },// pad GPIO_AD_B0_05 [GPIO1-5]
        {  "PIT_TRIGGER00","MQS_RIGHT","LPUART1_TX","QTIMER2_TIMER2","FLEXPWM2_PWMA03","GPIO1_IO06","REF_32K_OUT","-","-","-" }, // pad GPIO_AD_B0_06 [GPIO1-6]
        {  "PIT_TRIGGER01","MQS_LEFT","LPUART1_RX","QTIMER2_TIMER3","FLEXPWM2_PWMB03","GPIO1_IO07","REF_24M_OUT","-","-","-" }, // pad GPIO_AD_B0_07 [GPIO1-7]
        {  "ENET_TX_CLK",  "LPI2C3_SCL","LPUART1_CTS_B","KPP_COL00","ENET_REF_CLK1","GPIO1_IO08","ARM_CM7_TXEV","-","-","-" }, // pad GPIO_AD_B0_08 [GPIO1-8]
        {  "ENET_RDATA01", "LPI2C3_SDA","LPUART1_RTS_B","KPP_ROW00","-",      "GPIO1_IO09","ARM_CM7_RXEV","-","-","-" }, // pad GPIO_AD_B0_09 [GPIO1-9]
        {  "ENET_RDATA00", "LPSPI1_SCK","LPUART5_TX","KPP_COL01","FLEXPWM2_PWMA02","GPIO1_IO10","ARM_CM7_TRACE_CLK","-","-","-" }, // pad GPIO_AD_B0_10 [GPIO1-10]
        {  "ENET_RX_EN",   "LPSPI1_PCS0","LPUART5_RX","KPP_ROW01","FLEXPWM2_PWMB02","GPIO1_IO11","ARM_CM7_TRACE_SWO","-","-","-" }, // pad GPIO_AD_B0_11 [GPIO1-11]
        {  "ENET_RX_ER",   "LPSPI1_SDO","LPUART3_CTS_B","KPP_COL02","FLEXPWM2_PWMA01","GPIO1_IO12","ARM_CM7_TRACE00","SNVS_HP_VIO_5_CTL","-","-" }, // pad GPIO_AD_B0_12 [GPIO1-12]
        {  "ENET_TX_EN",   "LPSPI1_SDI","LPUART3_RTS_B","KPP_ROW02","FLEXPWM2_PWMB01","GPIO1_IO13","ARM_CM7_TRACE01","SNVS_HP_VIO_5_B","-","-" }, // pad GPIO_AD_B0_13 [GPIO1-13]
        {  "ENET_TDATA00", "FLEXCAN2_TX","LPUART3_TX","KPP_COL03","FLEXPWM2_PWMA00","GPIO1_IO14","ARM_CM7_TRACE02","WDOG1_ANY","-","-" }, // pad GPIO_AD_B0_14 [GPIO1-14]
        {  "ENET_TDATA01", "FLEXCAN2_RX","LPUART3_RX","KPP_ROW03","FLEXPWM2_PWMB00","GPIO1_IO15","ARM_CM7_TRACE03","-","-","-" }, // pad GPIO_AD_B0_15 [GPIO1-15]
        {  "SEMC_READY",   "FLEXSPI_A_DATA03","FLEXCAN2_TX","SAI1_MCLK","FLEXIO1_FLEXIO15","GPIO1_IO16","ENET_1588_EVENT2_OUT","KPP_COL04","-","-" }, // pad GPIO_AD_B1_00 [GPIO1-16]
        {  "SEMC_CSX00",   "FLEXSPI_A_SCLK","FLEXCAN2_RX","SAI1_TX_BCLK","FLEXIO1_FLEXIO14","GPIO1_IO17","ENET_1588_EVENT2_IN","KPP_ROW04","-","-" }, // pad GPIO_AD_B1_01 [GPIO1-17]
        {  "SEMC_CSX01",   "FLEXSPI_A_DATA00","LPSPI4_SCK","SAI1_TX_SYNC","FLEXIO1_FLEXIO13","GPIO1_IO18","ENET_1588_EVENT3_OUT","KPP_COL05","-","-" }, // pad GPIO_AD_B1_02 [GPIO1-18]
        {  "SEMC_CSX02",   "FLEXSPI_A_DATA02","LPSPI4_PCS0","SAI1_TX_DATA00","FLEXIO1_FLEXIO12","GPIO1_IO19","ENET_1588_EVENT3_IN","KPP_ROW05","-" }, // pad GPIO_AD_B1_03 [GPIO1-19]
        {  "SEMC_CSX03",   "FLEXSPI_A_DATA01","LPSPI4_SDO","SAI1_RX_SYNC","FLEXIO1_FLEXIO11","GPIO1_IO20","LPSPI1_PCS1","KPP_COL06","-","-" }, // pad GPIO_AD_B1_04 [GPIO1-20]
        {  "USDHC1_WP",    "FLEXSPI_A_SS0_B","LPSPI4_SDI","SAI1_RX_DATA00","FLEXIO1_FLEXIO10","GPIO1_IO21","LPSPI1_PCS2","KPP_ROW06","-","-" }, // pad GPIO_AD_B1_05 [GPIO1-21]
        {  "USDHC1_RESET_B","FLEXPWM1_PWMA00","LPUART2_CTS_B","SAI1_RX_BCLK","FLEXIO1_FLEXIO09","GPIO1_IO22","LPSPI1_PCS3","KPP_COL07","-","-" }, // pad GPIO_AD_B1_06 [GPIO1-22]
        {  "USDHC1_VSELECT","FLEXPWM1_PWMB00","LPUART2_RTS_B","SAI1_TX_DATA01","FLEXIO1_FLEXIO08","GPIO1_IO23","LPSPI3_PCS3","KPP_ROW07","-","-" }, // pad GPIO_AD_B1_07 [GPIO1-23]
        {  "LPI2C2_SCL",   "FLEXPWM1_PWMA01","LPUART2_TX","SAI1_TX_DATA02","FLEXIO1_FLEXIO07","GPIO1_IO24","LPSPI3_PCS2","XBAR1_INOUT12","-","-" }, // pad GPIO_AD_B1_08 [GPIO1-24]
        {  "LPI2C2_SDA",   "FLEXPWM1_PWMB01","LPUART2_RX","SAI1_TX_DATA03","FLEXIO1_FLEXIO06","GPIO1_IO25","LPSPI3_PCS1","XBAR1_INOUT13","-","-" }, // pad GPIO_AD_B1_09 [GPIO1-25]
        {  "USB_OTG1_PWR", "FLEXPWM1_PWMA02","LPUART4_TX","USDHC1_CD_B","FLEXIO1_FLEXIO05","GPIO1_IO26","GPT2_CAPTURE1","-","-","-" }, // pad GPIO_AD_B1_10 [GPIO1-26]
        {  "USB_OTG1_ID",  "FLEXPWM1_PWMB02","LPUART4_RX","USDHC1_WP","FLEXIO1_FLEXIO04","GPIO1_IO27","GPT2_COMPARE1","-","-","-" }, // pad GPIO_AD_B1_11 [GPIO1-27]
        {  "USB_OTG1_OC",  "ACMP1_OUT","LPSPI3_SCK","USDHC2_CD_B","FLEXIO1_FLEXIO03","GPIO1_IO28","FLEXPWM1_PWMA03","-","-","-" }, // pad GPIO_AD_B1_12 [GPIO1-28]
        {  "LPI2C1_HREQ",  "ACMP2_OUT","LPSPI3_PCS0","USDHC2_WP","FLEXIO1_FLEXIO02","GPIO1_IO29","FLEXPWM1_PWMB03","-","-","-" }, // pad GPIO_AD_B1_13 [GPIO1-29]
        {  "LPI2C1_SCL",   "ACMP3_OUT","LPSPI3_SDO","ENET_1588_EVENT0_OUT","FLEXIO1_FLEXIO01","GPIO1_IO30","-","-","-","-" }, // pad GPIO_AD_B1_14 [GPIO1-30]
        {  "LPI2C1_SDA",   "ACMP4_OUT","LPSPI3_SDI","ENET_1588_EVENT0_IN","FLEXIO1_FLEXIO00","GPIO1_IO31","-","-","-","-" }, // pad GPIO_AD_B1_14 [GPIO1-31]
#endif
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7     ALT8                GPIO2
#if defined iMX_RT105X || defined iMX_RT106X
        {  "LCD_CLK",      "QTIMER2_TIMER0","MQS_RIGHT","LPSPI4_PCS0","FLEXIO2_FLEXIO00","GPIO2_IO00","SEMC_CSX01","-","ENET2_MDC","-" }, // pad GPIO_B0_00 [GPIO2-0]
    #if defined iMX_RT106X
        {  "LCD_ENABLE",   "QTIMER2_TIMER1","LPUART4_RTS_B","SPDIF_OUT","LPSPI2_PCS0","GPIO2_IO01","FLEXCAN1_RX","PIT_TRIGGER03","ENET2_MDIO","-" }, // pad GPIO_B0_01 [GPIO2-1]
    #else
        {  "LCD_ENABLE",   "QTIMER2_TIMER1","LPUART4_RTS_B","SPDIF_OUT","LPSPI2_PCS0","GPIO2_IO01","FLEXCAN1_RX","PIT_TRIGGER03","-","-" }, // pad GPIO_B0_01 [GPIO2-1]
    #endif
        // NEXT BLOCK NOT DONE
        {  "LCD_HSYNC",    "QTIMER2_TIMER2","LPUART4_TX","SPDIF_LOCK","LPSPI2_SDO","GPIO2_IO02","LPI2C1_SCL","-","-","-" }, // pad GPIO_B0_02 [GPIO2-2]
        {  "LCD_VSYNC",    "QTIMER2_TIMER3","LPUART4_RX","SPDIF_EXT_CLK","LPSPI2_SDI","GPIO2_IO03","LPI2C1_SDA","-","-","-" }, // pad GPIO_B0_03 [GPIO2-3]
        {  "LCD_DATA00",   "XBAR1_INOUT04","SPDIF_OUT","SAI2_TX_BCLK","FLEXIO1_FLEXIO16","GPIO2_IO04","-","-","-","-" }, // pad GPIO_B0_04 [GPIO2-4]
        {  "LCD_DATA01",   "XBAR1_INOUT05","SPDIF_IN","SAI2_TX_SYNC","FLEXIO1_FLEXIO17","GPIO2_IO05","-","-","-","-" }, // pad GPIO_B0_05 [GPIO2-5]
        {  "LCD_DATA02",   "XBAR1_INOUT06","LPUART3_TX","SAI2_TX_DATA","FLEXIO1_FLEXIO18","GPIO2_IO06","-","-","-","-" }, // pad GPIO_B0_06 [GPIO2-6]
        {  "LCD_DATA03",   "XBAR1_INOUT07","LPUART3_RX","SAI2_RX_SYNC","FLEXIO1_FLEXIO19","GPIO2_IO07","-","-","-","-" }, // pad GPIO_B0_07 [GPIO2-7]
        {  "LCD_DATA04",   "XBAR1_INOUT08","FLEXCAN2_TX","SAI2_RX_DATA","FLEXIO1_FLEXIO20","GPIO2_IO08","-","-","-","-" }, // pad GPIO_B0_08 [GPIO2-8]
    #if defined iMX_RT106X
        {  "LCD_DATA05",   "XBAR1_INOUT09","FLEXCAN2_RX","SAI2_RX_BCLK","FLEXIO1_FLEXIO21","GPIO2_IO09","-","ENET2_CRS","-","-" }, // pad GPIO_B0_09 [GPIO2-9]
    #else
        {  "LCD_DATA05",   "XBAR1_INOUT09","FLEXCAN2_RX","SAI2_RX_BCLK","FLEXIO1_FLEXIO21","GPIO2_IO09","-","-","-","-" }, // pad GPIO_B0_09 [GPIO2-9]
    #endif
        {  "LCD_DATA06",   "XBAR1_INOUT10","LPI2C4_SDA","SAI1_TX_SYNC","LPSPI2_SCK","GPIO2_IO10","FLEXPWM2_PWMX00","-","-","-" }, // pad GPIO_B0_10 [GPIO2-10]
        {  "LCD_DATA07",   "XBAR1_INOUT11","LPI2C4_SCL","SAI1_TX_BCLK","LPSPI2_PCS0","GPIO2_IO11","FLEXPWM2_PWMX01","-","-","-" }, // pad GPIO_B0_11 [GPIO2-11]
    #if defined iMX_RT106X
        {  "LCD_DATA08",   "XBAR1_INOUT12","LPUART6_TX","SAI1_TX_DATA00","LPSPI2_SDO","GPIO2_IO12","FLEXPWM2_PWMX02","-","ENET2_TDATA0","-" }, // pad GPIO_B0_12 [GPIO2-12]
        {  "LCD_DATA09",   "XBAR1_INOUT13","LPUART6_RX","SAI1_RX_DATA00","LPSPI2_SDI","GPIO2_IO13","FLEXPWM2_PWMX03","CCM_PMIC_RDY","ENET2_TDATA1","-" }, // pad GPIO_B0_13 [GPIO2-13]
        {  "LCD_DATA10",   "XBAR1_INOUT14","LPUART6_CTS_B","SAI1_RX_BCLK","LPSPI2_PCS1","GPIO2_IO14","FLEXCAN1_TX","-","ENET2_TX_EN","-" }, // pad GPIO_B0_14 [GPIO2-14]
        {  "LCD_DATA11",   "XBAR1_INOUT15","LPUART6_RTS_B","SAI1_RX_SYNC","WDOG1_B","GPIO2_IO15","FLEXCAN1_RX","-","ENET2_TX_CLK","-" }, // pad GPIO_B0_15 [GPIO2-15]
    #else
        {  "LCD_DATA08",   "XBAR1_INOUT12","LPUART6_TX","SAI1_TX_DATA00","LPSPI2_SDO","GPIO2_IO12","FLEXPWM2_PWMX02","-","-","-" }, // pad GPIO_B0_12 [GPIO2-12]
        {  "LCD_DATA09",   "XBAR1_INOUT13","LPUART6_RX","SAI1_RX_DATA00","LPSPI2_SDI","GPIO2_IO13","FLEXPWM2_PWMX03","CCM_PMIC_RDY","-","-" }, // pad GPIO_B0_13 [GPIO2-13]
        {  "LCD_DATA10",   "XBAR1_INOUT14","LPUART6_CTS_B","SAI1_RX_BCLK","LPSPI2_PCS1","GPIO2_IO14","FLEXCAN1_TX","-","-","-" }, // pad GPIO_B0_14 [GPIO2-14]
        {  "LCD_DATA11",   "XBAR1_INOUT15","LPUART6_RTS_B","SAI1_RX_SYNC","WDOG1_B","GPIO2_IO15","FLEXCAN1_RX","-","-","-" }, // pad GPIO_B0_15 [GPIO2-15]
    #endif
        {  "LCD_DATA12",   "XBAR1_INOUT14","LPUART4_TXD","SAI1_RX_DATA00","FLEXIO2_D16","GPIO2_IO16","FLEXPWM1_PWM3_A","-","ENET2_RX_ER","FLEXIO3_FLEXIO16" }, // pad GPIO_B1_00 [GPIO2-16]
        {  "LCD_DATA13",   "XBAR1_INOUT15","LPUART4_RXD","SAI1_TX_DATA00","FLEXIO2_D17","GPIO2_IO17","FLEXPWM1_PWM3_B","-","ENET2_RDATA00","FLEXIO3_FLEXIO17" }, // pad GPIO_B1_01 [GPIO2-17]
        {  "LCD_DATA14",   "XBAR1_INOUT16","LPSPI4_PCS2","SAI1_TX_BCLK","FLEXIO2_D18","GPIO2_IO18","FLEXPWM2_PWM3_A","-","ENET2_RDATA01","FLEXIO3_FLEXIO18" }, // pad GPIO_B1_02 [GPIO2-18]
        {  "LCD_DATA15",   "XBAR1_INOUT17","LPSPI4_PCS1","SAI1_TX_SYNC","FLEXIO2_D19","GPIO2_IO19","FLEXPWM2_PWM3_B","-","ENET2_RX_EN","FLEXIO3_FLEXIO19" }, // pad GPIO_B1_03 [GPIO2-19]
        {  "LCD_DATA16",   "LPSPI4_PCS0","CSI_DATA15","ENET_RX_DATA00","FLEXIO2_D20","GPIO2_IO20","-","-","GPT1_CLK","FLEXIO3_FLEXIO20" }, // pad GPIO_B1_04 [GPIO2-20]
        {  "LCD_DATA17",   "LPSPI4_SDI","CSI_DATA14","ENET_RX_DATA01","FLEXIO2_D21","GPIO2_IO21","-","-","GPT1_CAPTURE1","FLEXIO3_FLEXIO21" }, // pad GPIO_B1_05 [GPIO2-21]
        {  "LCD_DATA18",   "LPSPI4_SDO","CSI_DATA13","ENET_RX_EN","FLEXIO2_D22","GPIO2_IO22","-","-","GPT1_CAPTURE2","FLEXIO3_FLEXIO22" }, // pad GPIO_B1_06 [GPIO2-22]
        {  "LCD_DATA19",   "LPSPI4_SCK","CSI_DATA12","ENET_TX_DATA00","FLEXIO2_D23","GPIO2_IO23","-","-","GPT1_COMPARE1","FLEXIO3_FLEXIO23" }, // pad GPIO_B1_07 [GPIO2-23]
        {  "LCD_DATA20",   "TMR1_TIMER3","CSI_DATA11","ENET_TX_DATA01","FLEXIO2_D24","GPIO2_IO24","FLEXCAN2_TX","-","GPT1_COMPARE2","FLEXIO3_FLEXIO24" }, // pad GPIO_B1_08 [GPIO2-24]
        {  "LCD_DATA21",   "TMR2_TIMER3","CSI_DATA10","ENET0_TX_EN","FLEXIO2_D25","GPIO2_IO25","FLEXCAN2_RX","-","GPT1_COMPARE3","FLEXIO3_FLEXIO25" }, // pad GPIO_B1_09 [GPIO2-25]
        {  "LCD_DATA22",   "TMR3_TIMER3","CSI_DATA00","ENET_TX_CLK","FLEXIO2_D26","GPIO2_IO26","ENET_REF_CLK","-","-","FLEXIO3_FLEXIO26" }, // pad GPIO_B1_10 [GPIO2-26]
        {  "LCD_DATA23",   "TMR4_TIMER3","CSI_DATA01","ENET_RX_ER","FLEXIO2_D27","GPIO2_IO27","LPSPI4_PCS3","-","-","FLEXIO3_FLEXIO27" }, // pad GPIO_B1_11 [GPIO2-27]
        {  "-",            "LPUART5_TXD","CSI_PIXCLK","ENET_1588_EVENT0_IN","FLEXIO2_D28","GPIO2_IO28","USDHC1_CD_B","-","-","FLEXIO3_FLEXIO28" }, // pad GPIO_B1_12 [GPIO2-28]
        {  "WDOG1_B",      "LPUART5_RXD","CSI_VSYNC","ENET_1588_EVENT0_OUT","FLEXIO2_D29","GPIO2_IO29","USDHC1_WP","-","SEMC_DQS4","FLEXIO3_FLEXIO29" }, // pad GPIO_B1_13 [GPIO2-29]
        {  "ENET_MDC",     "FLEXPWM4_PWM2_A","CSI_HSYNC","XBAR_INOUT02","FLEXIO2_D30","GPIO2_IO30","USDHC1_VSELECT","-","ENET2_TDATA00","FLEXIO3_FLEXIO30" }, // pad GPIO_B1_14 [GPIO2-30]
        {  "ENET_MDIO",    "FLEXPWM4_PWM3_A","CSI_MCLK","XBAR_INOUT03","FLEXIO2_D31","GPIO2_IO31","USDHC1_RESET_B","-","ENET2_TDATA01S","FLEXIO3_FLEXIO31" }  // pad GPIO_B1_15 [GPIO2-31]
#else
        {  "SEMC_DATA00",  "QTIMER2_TIMER0","LPUART4_CTS_B","SPDIF_SR_CLK","LPSPI2_SCK","GPIO2_IO00","FLEXCAN1_TX","PIT_TRIGGER02","-","-" }, // pad GPIO_EMC_00 [GPIO2-0]
        {  "SEMC_DATA01",  "QTIMER2_TIMER1","LPUART4_RTS_B","SPDIF_OUT","LPSPI2_PCS0","GPIO2_IO01","FLEXCAN1_RX","PIT_TRIGGER03","-","-" }, // pad GPIO_EMC_01 [GPIO2-1]
        {  "SEMC_DATA02",  "QTIMER2_TIMER2","LPUART4_TX","SPDIF_LOCK","LPSPI2_SDO","GPIO2_IO02","LPI2C1_SCL","-","-","-" }, // pad GPIO_EMC_02 [GPIO2-2]
        {  "SEMC_DATA03",  "QTIMER2_TIMER3","LPUART4_RX","SPDIF_EXT_CLK","LPSPI2_SDI","GPIO2_IO03","LPI2C1_SDA","-","-","-" }, // pad GPIO_EMC_03 [GPIO2-3]
        {  "SEMC_DATA04",  "XBAR1_INOUT04","SPDIF_OUT","SAI2_TX_BCLK","FLEXIO1_FLEXIO16","GPIO2_IO04","-","-","-","-" }, // pad GPIO_EMC_04 [GPIO2-4]
        {  "SEMC_DATA05",  "XBAR1_INOUT05","SPDIF_IN","SAI2_TX_SYNC","FLEXIO1_FLEXIO17","GPIO2_IO05","-","-","-","-" }, // pad GPIO_EMC_05 [GPIO2-5]
        {  "SEMC_DATA06",  "XBAR1_INOUT06","LPUART3_TX","SAI2_TX_DATA","FLEXIO1_FLEXIO18","GPIO2_IO06","-","-","-","-" }, // pad GPIO_EMC_06 [GPIO2-6]
        {  "SEMC_DATA07",  "XBAR1_INOUT07","LPUART3_RX","SAI2_RX_SYNC","FLEXIO1_FLEXIO19","GPIO2_IO07","-","-","-","-" }, // pad GPIO_EMC_07 [GPIO2-7]
        {  "SEMC_DM00",    "XBAR1_INOUT08","FLEXCAN2_TX","SAI2_RX_DATA","FLEXIO1_FLEXIO20","GPIO2_IO08","-","-","-","-" }, // pad GPIO_EMC_08 [GPIO2-8]
        {  "SEMC_WE",      "XBAR1_INOUT09","FLEXCAN2_RX","SAI2_RX_BCLK","FLEXIO1_FLEXIO21","GPIO2_IO09","-","-","-","-" }, // pad GPIO_EMC_09 [GPIO2-9]
        {  "SEMC_CAS",     "XBAR1_INOUT10","LPI2C4_SDA","SAI1_TX_SYNC","LPSPI2_SCK","GPIO2_IO10","FLEXPWM2_PWMX00","-","-","-" }, // pad GPIO_EMC_10 [GPIO2-10]
        {  "SEMC_RAS",     "XBAR1_INOUT11","LPI2C4_SCL","SAI1_TX_BCLK","LPSPI2_PCS0","GPIO2_IO11","FLEXPWM2_PWMX01","-","-","-" }, // pad GPIO_EMC_11 [GPIO2-11]
        {  "SEMC_CS0",     "XBAR1_INOUT12","LPUART6_TX","SAI1_TX_DATA00","LPSPI2_SDO","GPIO2_IO12","FLEXPWM2_PWMX02","-","-","-" }, // pad GPIO_EMC_12 [GPIO2-12]
        {  "SEMC_BA0",     "XBAR1_INOUT13","LPUART6_RX","SAI1_RX_DATA00","LPSPI2_SDI","GPIO2_IO13","FLEXPWM2_PWMX03","CCM_PMIC_RDY","-","-" }, // pad GPIO_EMC_13 [GPIO2-13]
        {  "SEMC_BA1",     "XBAR1_INOUT14","LPUART6_CTS_B","SAI1_RX_BCLK","LPSPI2_PCS1","GPIO2_IO14","FLEXCAN1_TX","-","-","-" }, // pad GPIO_EMC_14 [GPIO2-14]
        {  "SEMC_ADDR10",  "XBAR1_INOUT15","LPUART6_RTS_B","SAI1_RX_SYNC","WDOG1_B","GPIO2_IO15","FLEXCAN1_RX","-","-","-" }, // pad GPIO_EMC_15 [GPIO2-15]
        {  "SEMC_ADDR00",  "-",     "MQS_RIGHT", "SAI2_MCLK",  "-",           "GPIO2_IO16","SRC_BOOT_MODE00","-","-","-" }, // pad GPIO_EMC_16 [GPIO2-16]
        {  "SEMC_ADDR01",  "-",     "MQS_LEFT",  "SAI3_MCLK",  "-",           "GPIO2_IO17","SRC_BOOT_MODE01","-","-","-" }, // pad GPIO_EMC_17 [GPIO2-17]
        {  "SEMC_ADDR02",  "XBAR1_INOUT16","LPI2C2_SDA","SAI1_RX_SYNC","FLEXIO1_FLEXIO22","GPIO2_IO18","SRC_BT_CFG00","-","-","-" }, // pad GPIO_EMC_18 [GPIO2-18]
        {  "SEMC_ADDR03",  "XBAR1_INOUT17","LPI2C2_SCL","SAI1_RX_BCLK","FLEXIO1_FLEXIO23","GPIO2_IO19","SRC_BT_CFG01","-","-","-" }, // pad GPIO_EMC_19 [GPIO2-19]
        {  "SEMC_ADDR04",  "FLEXPWM1_PWMA03","LPUART2_CTS_B","SAI1_MCLK","FLEXIO1_FLEXIO24","GPIO2_IO20","SRC_BT_CFG02","-","-","-" }, // pad GPIO_EMC_20 [GPIO2-20]
        {  "SEMC_ADDR05",  "FLEXPWM1_PWMB03","LPUART2_RTS_B","SAI1_RX_DATA00","FLEXIO1_FLEXIO25","GPIO2_IO21","SRC_BT_CFG03","-","-","-" }, // pad GPIO_EMC_21 [GPIO2-21]
        {  "SEMC_ADDR06",  "FLEXPWM1_PWMA02","LPUART2_TX","SAI1_TX_DATA03","FLEXIO1_FLEXIO26","GPIO2_IO22","SRC_BT_CFG04","-","-","-" }, // pad GPIO_EMC_22 [GPIO2-22]
        {  "SEMC_ADDR07",  "FLEXPWM1_PWMB02","LPUART2_RX","SAI1_TX_DATA02","FLEXIO1_FLEXIO27","GPIO2_IO23","SRC_BT_CFG05","-","-","-" }, // pad GPIO_EMC_23 [GPIO2-23]
        {  "SEMC_ADDR08",  "FLEXPWM1_PWMA01","LPUART8_CTS_B","SAI1_TX_DATA01","FLEXIO1_FLEXIO28","GPIO2_IO24","SRC_BT_CFG06","-","-","-" }, // pad GPIO_EMC_24 [GPIO2-24]
        {  "SEMC_ADDR09",  "FLEXPWM1_PWMB01","LPUART8_RTS_B","SAI1_TX_DATA00","FLEXIO1_FLEXIO29","GPIO2_IO25","SRC_BT_CFG07","-","-","-" }, // pad GPIO_EMC_25 [GPIO2-25]
        {  "SEMC_ADDR11",  "FLEXPWM1_PWMA00","LPUART8_TX","SAI1_TX_BCLK","FLEXIO1_FLEXIO30","GPIO2_IO26","SRC_BT_CFG08","-","-","-" }, // pad GPIO_EMC_26 [GPIO2-26]
        {  "SEMC_ADDR12",  "FLEXPWM1_PWMB00","LPUART8_RX","SAI1_TX_SYNC","FLEXIO1_FLEXIO31","GPIO2_IO27","SRC_BT_CFG09","-","-","-" }, // pad GPIO_EMC_27 [GPIO2-27]
        {  "SEMC_DQS",     "FLEXPWM2_PWMA03","XBAR1_INOUT18","SAI3_MCLK","EWM_OUT_B","GPIO2_IO28","GPT2_CAPTURE2","FLEXPWM1_PWMX00","-","-" }, // pad GPIO_EMC_28 [GPIO2-28]
        {  "SEMC_CKE",     "FLEXPWM2_PWMB03","XBAR1_INOUT19","SAI3_RX_BCLK","WDOG2_RST_B_DEB","GPIO2_IO29","GPT2_COMPARE2","FLEXPWM1_PWMX01","-","-" }, // pad GPIO_EMC_29 [GPIO2-29]
        {  "SEMC_CLK",     "FLEXPWM2_PWMA02","LPUART4_CTS_B","SAI3_RX_SYNC","WDOG1_RST_B_DEB","GPIO2_IO30","GPT2_COMPARE3","FLEXPWM1_PWMX02","-","-" }, // pad GPIO_EMC_30 [GPIO2-30]
        {  "SEMC_DM01",    "FLEXPWM2_PWMB02","LPUART4_RTS_B","SAI3_RX_DATA","WDOG2_B","GPIO2_IO31","GPT2_CLK","FLEXPWM1_PWMX03","-","-" }  // pad GPIO_EMC_31 [GPIO2-31]
#endif
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7     ALT 8         GPIO3
#if defined iMX_RT105X || defined iMX_RT106X
        {  "USDHC2_DATA3", "FLEXSPIB_DATA03","FLEXPWM1_PWMA03","SAI1_TX_DATA03","LPUART4_TX","GPIO3_IO00","-", "-","SAI3_RX_DATA","-" }, // pad GPIO_SD_B1_00 [GPIO3-0]

        {  "USDHC2_DATA2", "FLEXSPIB_DATA02","FLEXPWM1_PWMB03","SAI1_TX_DATA02","LPUART4_RX","GPIO3_IO01","-","-","SAI3_TX_DATA","-" }, // pad GPIO_SD_B1_01 [GPIO3-1]
        {  "USDHC2_DATA1", "FLEXSPIB_DATA01","FLEXPWM2_PWMA03","SAI1_TX_DATA01","FLEXCAN1_TX","GPIO3_IO02","CCM_WAIT","-","SAI3_TX_SYNC","-" }, // pad GPIO_SD_B1_02 [GPIO3-2]
        {  "USDHC2_DATA0", "FLEXSPIB_DATA00","FLEXPWM2_PWMB03","SAI1_MCLK",     "FLEXCAN1_RX","GPIO3_IO03","CCM_PMIC_READY","-","SAI3_TX_BCLK","-" }, // pad GPIO_SD_B1_03 [GPIO3-3]
        {  "USDHC2_CLK",   "FLEXSPIB_SCLK",  "LPI2C1_SCL",     "SAI1_RX_SYNC",  "FLEXSPIA_SS1_B","GPIO3_IO04","CCM_STOP","-","SAI3_MCLK","-" }, // pad GPIO_SD_B1_04 [GPIO3-4]
        {  "USDHC2_CMD",   "FLEXSPIA_DQS","LPI2C1_SDA","SAI1_RX_BCLK","FLEXSPIB_SS0_B","GPIO3_IO05","-","-","SAI3_RX_SYNC","-" }, // pad GPIO_SD_B1_05 [GPIO3-5]
        {  "USDHC2_RESET_B","FLEXSPIA_SS0_B","LPUART7_CTS_B","SAI1_RX_DATA00","LPSPI2_PCS0","GPIO3_IO06","-","-","SAI3_RX_BCLK","-" }, // pad GPIO_SD_B1_06 [GPIO3-6]
        {  "SEMC_CSX01",   "FLEXSPIA_SCLK","LPUART7_RTS_B","SAI1_TX_DATA00","LPSPI2_SCK","GPIO3_IO07","-","-","-","-" }, // pad GPIO_SD_B1_07 [GPIO3-7]
        {  "USDHC2_DATA4", "FLEXSPIA_DATA00","LPUART7_TX","SAI1_TX_BCLK","LPSPI2_SD0","GPIO3_IO08","SEMC_CSX02","-","-","-" }, // pad GPIO_SD_B1_08 [GPIO3-8]
        {  "USDHC2_DATA5", "FLEXSPIA_DATA01","LPUART7_RX","SAI1_TX_SYNC","LPSPI2_SDI","GPIO3_IO09","-","-","-","-" }, // pad GPIO_SD_B1_09 [GPIO3-9]
        {  "USDHC2_DATA6", "FLEXSPIA_DATA02","LPUART2_RX","LPI2C2_SDA","LPSPI2_PCS2","GPIO3_IO10","-","-","-","-" }, // pad GPIO_SD_B1_10 [GPIO3-10]
        {  "USDHC2_DATA7", "FLEXSPIA_DATA03","LPUART2_TX","LPI2C2_SCL","LPSPI2_PCS3","GPIO3_IO11","-","-","-","-" }, // pad GPIO_SD_B1_11 [GPIO3-11]
        {  "USDHC1_CMD",   "FLEXPWM1_PWMA00","LPI2C3_SCL","XBAR1_INOUT04","LPSPI1_SCK","GPIO3_IO12","FLEXSPIA_SS1_B","-","ENET2_TX_EN","SEMC_DQS4" }, // pad GPIO_SD_B0_00 [GPIO3-12]
        {  "USDHC1_CLK",   "FLEXPWM1_PWMB00","LPI2C3_SDA","XBAR1_INOUT05","LPSPI1_PCS0","GPIO3_IO13","FLEXSPIB_SS1_B","-","ENET2_TX_CLK","ENET2_REF_CLK2" }, // pad GPIO_SD_B0_01 [GPIO3-13]
        {  "USDHC1_DATA0", "FLEXPWM1_PWMA01","LPUART8_CTS_B","XBAR1_INOUT06","LPSPI1_SDO","GPIO3_IO14","-","-","ENET2_RX_ER","SEMC_CLK5" }, // pad GPIO_SD_B0_02 [GPIO3-14]
        {  "USDHC1_DATA1", "FLEXPWM1_PWMB01","LPUART8_RTS_B","XBAR1_INOUT07","LPSPI1_SDI","GPIO3_IO15","-","-","ENET2_RDATA00","SEMC_CLK6" }, // pad GPIO_SD_B0_03 [GPIO3-15]
        {  "USDHC1_DATA2", "FLEXPWM1_PWMA02","LPUART8_TX","XBAR1_INOUT08","FLEXSPIB_SS0_B","GPIO3_IO16","CCM_CLKO1","-","ENET2_RDATA01","-" }, // pad GPIO_SD_B0_04 [GPIO3-16]
        {  "USDHC1_DATA3", "FLEXPWM1_PWMB02","LPUART8_RX","XBAR1_INOUT09","FLEXSPIB_DQS","GPIO3_IO17","CCM_CLKO2","-","ENET2_RX_EN","-" }, // pad GPIO_SD_B0_05 [GPIO3-17]
        {  "SEMC_DATA10",  "FLEXPWM3_PWMB01","LPUART7_RX","CCM_PMIC_RDY","CSI_DATA21","GPIO3_IO18","-","-","-","-" }, // pad GPIO_EMC_32 [GPIO3-18]
        {  "SEMC_DATA11",  "FLEXPWM3_PWMA02","USDHC1_RESET_B","SAI3_RX_DATA","CSI_DATA20","GPIO3_IO19","-","-","-","-" }, // pad GPIO_EMC_33 [GPIO3-19]
        {  "SEMC_DATA12",  "FLEXPWM3_PWMB02","USDHC1_VSELECT","SAI3_RX_SYNC","CSI_DATA19","GPIO3_IO20","-","-","-","-" }, // pad GPIO_EMC_34 [GPIO3-20]
        {  "SEMC_DATA13",  "XBAR1_INOUT18","GPT1_COMPARE1","SAI3_RX_BCLK","CSI_DATA18","GPIO3_IO21","USDHC1_CD_B","-","-","-" }, // pad GPIO_EMC_35 [GPIO3-21]
        {  "SEMC_DATA14",  "XBAR1_IN22","GPT1_COMPARE2","SAI3_TX_DATA","CSI_DATA17","GPIO3_IO22","USDHC1_WP","-","-","-" }, // pad GPIO_EMC_36 [GPIO3-22]
        {  "SEMC_DATA15",  "XBAR1_IN23","GPT1_COMPARE3","SAI3_MCLK","CSI_DATA16","GPIO3_IO23","USDHC2_WP","-","-","-" }, // pad GPIO_EMC_37 [GPIO3-23]
        {  "SEMC_DM01",    "FLEXPWM1_PWMA03","LPUART8_TX","SAI3_TX_BCLK","CSI_FIELD","GPIO3_IO24","USDHC2_VSELECT","-","-","-" }, // pad GPIO_EMC_38 [GPIO3-24]
        {  "SEMC_DQS",     "FLEXPWM1_PWMB03","LPUART8_RX","SAI3_TX_SYNC","WDOG1_WDOG_B","GPIO3_IO25","USDHC2_CD_B","-","-","-" }, // pad GPIO_EMC_39 [GPIO3-25]
        {  "SEMC_RDY",     "GPT2_CAPTURE2","LPSPI1_PCS2","USB_OTG2_OC","ENET_MDC","GPIO3_IO26","USDHC2_RESET_B","-","-","-" }, // pad GPIO_EMC_40 [GPIO3-26]
        {  "SEMC_CSX00",   "GPT2_CAPTURE1","LPSPI1_PCS3","USB_OTG2_PWR","ENET_MDIO","GPIO3_IO27","USDHC1_VSELECT","-","-","-" }, // pad GPIO_EMC_41 [GPIO3-27]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
#else
        {  "SEMC_DATA12",  "QTIMER1_TIMER0","LPUART4_TX","SAI3_TX_DATA","LPSPI4_SCK","GPIO3_IO00","-", "REF_24M_OUT","-","-" }, // pad GPIO_EMC_32 [GPIO3-0]
        {  "SEMC_DATA09",  "QTIMER1_TIMER1","LPUART4_RX","SAI3_TX_BCLK","LPSPI4_PCS0","GPIO3_IO01","-","-","-","-" }, // pad GPIO_EMC_33 [GPIO3-1]
        {  "SEMC_DATA10",  "QTIMER1_TIMER2","LPUART7_TX","SAI3_TX_SYNC","LPSPI4_SDO","GPIO3_IO02","ENET_CRS","-","-","-" }, // pad GPIO_EMC_34 [GPIO3-2]
        {  "SEMC_DATA11",  "QTIMER1_TIMER3","LPUART7_RX","USDHC2_WP","LPSPI4_SDI","GPIO3_IO03","ENET_COL","-","-","-" }, // pad GPIO_EMC_35 [GPIO3-3]
        {  "SEMC_DATA12",  "FLEXPWM2_PWMA01","LPUART5_CTS_B","CCM_PMIC_RDY","LPSPI4_PCS1","GPIO3_IO04","ENET_RX_CLK","USDHC1_WP","-","-" }, // pad GPIO_EMC_36 [GPIO3-4]
        {  "SEMC_DATA13",  "FLEXPWM2_PWMB01","LPUART5_RTS_B","MQS_RIGHT","LPSPI4_PCS2","GPIO3_IO05","ENET_RX_CLK","USDHC1_VSELECT","-","-" }, // pad GPIO_EMC_37 [GPIO3-5]
        {  "SEMC_DATA14",  "FLEXPWM2_PWMA00","LPUART5_TX","MQS_LEFT","LPSPI4_PCS3","GPIO3_IO06","ENET_RDATA02","USDHC1_CD_B","-","-" }, // pad GPIO_EMC_38 [GPIO3-6]
        {  "SEMC_DATA15",  "FLEXPWM2_PWMB00","LPUART5_RX","USB_OTG1_OC","WDOG1_B","GPIO3_IO07","ENET_TX_ER","GPT1_CLK","-","-" }, // pad GPIO_EMC_39 [GPIO3-7]
        {  "SEMC_CSX00",   "XBAR1_INOUT18","SPDIF_OUT","USB_OTG1_ID","ENET_MDIO","GPIO3_IO08","ENET_TDATA03","GPT1_COMPARE3","-","-" }, // pad GPIO_EMC_40 [GPIO3-8]
        {  "SEMC_READY",   "XBAR1_INOUT19","SPDIF_IN","USB_OTG1_PWR","ENET_MDC","GPIO3_IO09","ENET_TDATA02","GPT1_COMPARE2","-","-" }, // pad GPIO_EMC_41 [GPIO3-9]
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "USDHC1_DATA2", "QTIMER1_TIMER0","SAI1_MCLK","SAI2_MCLK","LPI2C3_SCL","GPIO3_IO13","FLEXSPI_A_SS1_B","XBAR1_INOUT14","-","-" }, // pad GPIO_SD_B0_00 [GPIO3-13]
        {  "USDHC1_DATA3", "QTIMER1_TIMER1","REF_24M_OUT","SAI2_RX_SYNC","LPI2C3_SDA","GPIO3_IO14","FLEXSPI_B_SS1_B","XBAR1_INOUT15","-","-" }, // pad GPIO_SD_B0_01 [GPIO3-14]
        {  "USDHC1_CMD",   "QTIMER1_TIMER2","LPUART7_CTS_B","SAI2_RX_BCLK","LPSPI1_SCK","GPIO3_IO15","ENET_MDIO","XBAR1_INOUT16","-","-" }, // pad GPIO_SD_B0_02 [GPIO3-15]
        {  "USDHC1_CLK",   "QTIMER1_TIMER3","LPUART7_RTS_B","SAI2_RX_DATA","LPSPI1_PCS0","GPIO3_IO16","ENET_MDC","-","-","-" }, // pad GPIO_SD_B0_03 [GPIO3-16]
        {  "USDHC1_DATA0", "FLEXCAN2_TX","LPUART7_TX","SAI2_TX_DATA","LPSPI1_SDO","GPIO3_IO17","FLEXSPI_B_SS0_B","-","-","-" }, // pad GPIO_SD_B0_04 [GPIO3-17]
        {  "USDHC1_DATA1", "FLEXCAN2_RX","LPUART7_RX","SAI2_TX_BCLK","LPSPI1_SDI","GPIO3_IO18","FLEXSPI_B_DQS","-","-","-" }, // pad GPIO_SD_B0_05 [GPIO3-18]
        {  "USDHC1_CD_B",  "USDHC1_RESET_B","REF_32K_OUT","SAI2_TX_SYNC","WDOG1_B","GPIO3_IO19","XBAR1_INOUT17","-","-","-" }, // pad GPIO_SD_B0_06 [GPIO3-19]
        {  "USDHC2_DATA2", "FLEXSPI_B_DATA03","LPUART6_TX","XBAR1_INOUT10","FLEXCAN1_TX","GPIO3_IO20","-","-","-","-" }, // pad GPIO_SD_B1_00 [GPIO3-20]
        {  "USDHC2_DATA3", "FLEXSPI_B_SCLK","LPUART6_RX","FLEXSPI_A_SS1_B","FLEXCAN1_RX","GPIO3_IO21","-","-","-","-" }, // pad GPIO_SD_B1_01 [GPIO3-21]
        {  "USDHC2_CMD",   "FLEXSPI_B_DATA00","LPUART8_TX","LPI2C4_SCL","ENET_1588_EVENT1_OUT","GPIO3_IO22","CCM_CLKO1","-","-","-" }, // pad GPIO_SD_B1_02 [GPIO3-22]
        {  "USDHC2_CLK",   "FLEXSPI_B_DATA02","LPUART8_RX","LPI2C4_SDA","ENET_1588_EVENT1_IN","GPIO3_IO23","CCM_CLKO2","-","-","-" }, // pad GPIO_SD_B1_03 [GPIO3-23]
        {  "USDHC2_DATA0", "FLEXSPI_B_DATA01","ENET_TX_CLK","ENET_REF_CLK1","EWM_OUT_B","GPIO3_IO24","CCM_WAIT","-","-","-" }, // pad GPIO_SD_B1_04 [GPIO3-24]
        {  "USDHC2_DATA1", "FLEXSPI_A_DQS","ENET_RDATA01","SAI3_MCLK","FLEXSPI_B_SS0_B","GPIO3_IO25","CCM_PMIC_RDY","-","-","-" }, // pad GPIO_SD_B1_05 [GPIO3-25]
        {  "USDHC2_CD_B",  "FLEXSPI_A_DATA03","ENET_RDATA00","SAI3_TX_BCLK","LPSPI2_PCS0","GPIO3_IO26","CCM_STOP","-","-","-" }, // pad GPIO_SD_B1_06 [GPIO3-26]
        {  "USDHC2_RESET_B","FLEXSPI_A_SCLK","ENET_RX_EN","SAI3_TX_SYNC","LPSPI2_SCK","GPIO3_IO27","-","-","-","-" }, // pad GPIO_SD_B1_07 [GPIO3-27]
        {  "USDHC2_DATA4", "FLEXSPI_A_DATA00","ENET_RX_ER","SAI3_TX_DATA","LPSPI2_SDO","GPIO3_IO28","-","-","-","-" }, // pad GPIO_SD_B1_08 [GPIO3-28]
        {  "USDHC2_DATA5", "FLEXSPI_A_DATA02","ENET_TX_EN","SAI3_RX_BCLK","LPSPI2_SDI","GPIO3_IO29","-","-","-","-" }, // pad GPIO_SD_B1_09 [GPIO3-29]
        {  "USDHC2_DATA6", "FLEXSPI_A_DATA01","ENET_TDATA00","SAI3_RX_SYNC","LPSPI2_PCS2","GPIO3_IO30","-","-","-","-" }, // pad GPIO_SD_B1_10 [GPIO3-30]
        {  "USDHC2_DATA7", "FLEXSPI_A_SS0_B","ENET_TDATA01","SAI3_RX_DATA","LPSPI2_PCS3","GPIO3_IO31","-","-","-","-" }, // pad GPIO_SD_B1_11 [GPIO3-31]
#endif
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7    ALT 8          GPIO4
        #if defined iMX_RT105X || defined iMX_RT106X
        {  "SEMC_DATA00",  "FLEXPWM4_PWMA00","LPSPI2_SCK","XBAR1_XBAR_IN02","FLEXIO1_FLEXIO00","GPIO4_IO00","-","-","-","-" }, // pad GPIO_EMC_00 [GPIO4-0]
        {  "SEMC_DATA01",  "FLEXPWM4_PWMB00","LPSPI2_PCS0","XBAR1_IN03","FLEXIO1_FLEXIO01","GPIO4_IO01","-","-","-","-" }, // pad GPIO_EMC_01 [GPIO4-1]
        {  "SEMC_DATA02",  "FLEXPWM4_PWMA01","LPSPI2_SDO","XBAR1_INOUT04","FLEXIO1_FLEXIO02","GPIO4_IO02","-","-","-","-" }, // pad GPIO_EMC_02 [GPIO4-2]
        {  "SEMC_DATA03",  "FLEXPWM4_PWMB01","LPSPI2_SDI","XBAR1_INOUT05","FLEXIO1_FLEXIO03","GPIO4_IO03","-","-","-","-" }, // pad GPIO_EMC_03 [GPIO4-3]
        {  "SEMC_DATA04",  "FLEXPWM4_PWMA02","SAI2_TX_DATA","XBAR1_INOUT06","FLEXIO1_FLEXIO04","GPIO4_IO04","-","-","-","-" }, // pad GPIO_EMC_04 [GPIO4-4]
        {  "SEMC_DATA05",  "FLEXPWM4_PWMB02","SAI2_TX_SYNC","XBAR1_INOUT07","FLEXIO1_FLEXIO05","GPIO4_IO05","-","-","-","-" }, // pad GPIO_EMC_05 [GPIO4-5]
        {  "SEMC_DATA06",  "FLEXPWM2_PWMA00","SAI2_TX_BCLK","XBAR1_INOUT08","FLEXIO1_FLEXIO06","GPIO4_IO06","-","-","-","-" }, // pad GPIO_EMC_06 [GPIO4-6]
        {  "SEMC_DATA07",  "FLEXPWM2_PWMB00","SAI2_MCLK","XBAR1_INOUT09","FLEXIO1_FLEXIO07","GPIO4_IO07","-","-","-","-" }, // pad GPIO_EMC_07 [GPIO4-7]
        {  "SEMC_DM00",    "FLEXPWM2_PWMA01","SAI2_RX_DATA","XBAR1_INOUT17","FLEXIO1_FLEXIO08","GPIO4_IO08","-","-","-","-" }, // pad GPIO_EMC_08 [GPIO4-8]
        {  "SEMC_ADDR00",  "FLEXPWM2_PWMB01","SAI2_RX_SYNC","FLEXCAN2_TX","FLEXIO1_FLEXIO09","GPIO4_IO09","-","-","-","-" }, // pad GPIO_EMC_09 [GPIO4-9]
        {  "SEMC_ADDR01",  "FLEXPWM2_PWMA02","SAI2_RX_BCLK","FLEXCAN2_RX","FLEXIO1_FLEXIO10","GPIO4_IO10","-","-","-","-" }, // pad GPIO_EMC_10 [GPIO4-10]
        {  "SEMC_ADDR02",  "FLEXPWM2_PWMB02","LPI2C4_SDA","USDHC2_RESET_B","FLEXIO1_FLEXIO11","GPIO4_IO11","-","-","-","-" }, // pad GPIO_EMC_11 [GPIO4-11]
        {  "SEMC_ADDR03",  "XBAR1_IN24","LPI2C4_SCL","USDHC1_WP","FLEXPWM1_PWMA03","GPIO4_IO12","-","-","-","-" }, // pad GPIO_EMC_12 [GPIO4-12]
        {  "SEMC_ADDR04",  "XBAR1_IN25","LPUART3_TX","MQS_RIGHT","FLEXPWM1_PWMB03","GPIO4_IO13","-","-","-","-" }, // pad GPIO_EMC_13 [GPIO4-13]
        {  "SEMC_ADDR05",  "XBAR1_INOUT19","LPUART3_RX","MQS_LEFT","LPSPI2_PCS1","GPIO4_IO14","-","-","-","-" }, // pad GPIO_EMC_14 [GPIO4-14]
        {  "SEMC_ADDR06",  "XBAR1_IN20","LPUART3_CTS_B","SPDIF_OUT","QTIMER3_TIMER0","GPIO4_IO15","-","-","-","-" }, // pad GPIO_EMC_15 [GPIO4-15]
        {  "SEMC_ADDR07",  "XBAR1_IN21","LPUART3_RTS_B","SPDIF_IN","QTIMER3_TIMER1","GPIO4_IO16","-","-","-","-" }, // pad GPIO_EMC_16 [GPIO4-16]
        {  "SEMC_ADDR08",  "FLEXPWM4_PWMA03","LPUART4_CTS_B","FLEXCAN1_TX","QTIMER3_TIMER2","GPIO4_IO17","-","-","-","-" }, // pad GPIO_EMC_17 [GPIO4-17]
        {  "SEMC_ADDR09",  "FLEXPWM4_PWMB03","LPUART4_RTS_B","FLEXCAN1_RX","QTIMER3_TIMER3","GPIO4_IO18","SNVS_VIO_5_CTL","-","-","-" }, // pad GPIO_EMC_18 [GPIO4-18]
        {  "SEMC_ADDR11",  "FLEXPWM2_PWMA03","LPUART4_TX","ENET_RDATA01","QTIMER2_TIMER0","GPIO4_IO19","SNVS_VIO_5","-","-","-" }, // pad GPIO_EMC_19 [GPIO4-19]
        {  "SEMC_ADDR12",  "FLEXPWM2_PWMB03","LPUART4_RX","ENET_RDATA00","QTIMER2_TIMER1","GPIO4_IO20","-","-","-","-" }, // pad GPIO_EMC_20 [GPIO4-20]
        {  "SEMC_BA0",     "FLEXPWM3_PWMA03","LPI2C3_SDA","ENET_TDATA01","QTIMER2_TIMER2","GPIO4_IO21","-","-","-","-" }, // pad GPIO_EMC_21 [GPIO4-21]
        {  "SEMC_BA1",     "FLEXPWM3_PWMB03","LPI2C3_SCL","ENET_TDATA00","QTIMER2_TIMER3","GPIO4_IO22","-","-","-","-" }, // pad GPIO_EMC_22 [GPIO4-22]
        {  "SEMC_ADDR10",  "FLEXPWM1_PWMA00","LPUART5_TX","ENET_RX_EN","GPT1_CAPTURE2","GPIO4_IO23","-","-","-","-" }, // pad GPIO_EMC_23 [GPIO4-23]
        {  "SEMC_CAS",     "FLEXPWM1_PWMB00","LPUART5_RX","ENET_TX_EN","GPT1_CAPTURE1","GPIO4_IO24","-","-","-","-" }, // pad GPIO_EMC_24 [GPIO4-24]
        {  "SEMC_RAS",     "FLEXPWM1_PWMA01","LPUART6_TX","ENET_TX_CLK","ENET_REF_CLK","GPIO4_IO25","-","-","-","-" }, // pad GPIO_EMC_25 [GPIO4-25]
        {  "SEMC_CLK",     "FLEXPWM1_PWMB01","LPUART6_RX","ENET_RX_ER","FLEXIO1_FLEXIO12","GPIO4_IO26","-","-","-","-" }, // pad GPIO_EMC_26 [GPIO4-26]
        {  "SEMC_CKE",     "FLEXPWM1_PWMA02","LPUART5_RTS_B","LPSPI1_SCK","FLEXIO1_FLEXIO13","GPIO4_IO27","-","-","-","-" }, // pad GPIO_EMC_27 [GPIO4-27]
        {  "SEMC_WE",      "FLEXPWM1_PWMB02","LPUART5_CTS_B","LPSPI1_SDO","FLEXIO1_FLEXIO14","GPIO4_IO28","-","-","-","-" }, // pad GPIO_EMC_28 [GPIO4-28]
        {  "SEMC_CS0",     "FLEXPWM3_PWMA00","LPUART6_RTS_B","LPSPI1_SDI","FLEXIO1_FLEXIO15","GPIO4_IO29","-","-","-","-" }, // pad GPIO_EMC_29 [GPIO4-29]
        {  "SEMC_DATA08",  "FLEXPWM3_PWMB00","LPUART6_CTS_B","LPSPI1_PCS0","CSI_DATA23","GPIO4_IO30","-","-","ENET2_TDATA00" }, // pad GPIO_EMC_30 [GPIO4-30]
        {  "SEMC_DATA09",  "FLEXPWM3_PWMA01","LPUART7_TX","LPSPI1_PCS1","CSI_DATA22","GPIO4_IO31","-","-","ENET2_TDATA01" }, // pad GPIO_EMC_31 [GPIO4-31]
    #else
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" }
    #endif
    },
    {
        // ALT 0           ALT 1    ALT2         ALT 3         ALT 4          ALT 5        ALT 6       ALT 7       ALT8       GPIO5
        {  "-",            "-",     "-",         "-",          "-",           "GPIO5_IO00","-",        "NMI_GLUE_NMI","-","-" }, // pad WAKEUP [GPIO5]
        {  "SNVS_LP_PMIC_ON_REQ","-","-",        "-",          "-",           "GPIO5_IO01","-",        "-","-","-" }, // pad PMIC_ON_REQ
        {  "CCM_PMIC_VSTBY_REQ","-","-",         "-",          "-",           "GPIO5_IO02","-",        "-","-","-" }, // pad PMIC_STBY_REQ
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" },
        {  "-",            "-",     "-",         "-",          "-",           "-",         "-",        "-","-","-" }
    },
};
#endif