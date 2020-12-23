/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      sim_iMX.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/  

#if NUMBER_OF_SDHC > 0
    extern void fnCheckSD_Detect(void);
#endif

typedef struct st_iMX_DCDC
{
    unsigned long REG0;
    unsigned long REG1;
    unsigned long REG2;
    unsigned long REG3;
} iMX_DCDC;


#if defined iMX_RT1011
    typedef struct st_iMX_OTFAD
    {
        unsigned long OTFAD_CR;
        unsigned long OTFAD_SR;
        unsigned long ulRes0[62];
        unsigned long OTFAD_CTX0_KEY0;
        unsigned long OTFAD_CTX0_KEY1;
        unsigned long OTFAD_CTX0_KEY2;
        unsigned long OTFAD_CTX0_KEY3;
        unsigned long OTFAD_CTX0_CTR0;
        unsigned long OTFAD_CTX0_CTR1;
        unsigned long OTFAD_CTX0_RGD_W0;
        unsigned long OTFAD_CTX0_RGD_W1;
        unsigned long ulRes1[8];
        unsigned long OTFAD_CTX1_KEY0;
        unsigned long OTFAD_CTX1_KEY1;
        unsigned long OTFAD_CTX1_KEY2;
        unsigned long OTFAD_CTX1_KEY3;
        unsigned long OTFAD_CTX1_CTR0;
        unsigned long OTFAD_CTX1_CTR1;
        unsigned long OTFAD_CTX1_RGD_W0;
        unsigned long OTFAD_CTX1_RGD_W1;
        unsigned long ulRes2[8];
        unsigned long OTFAD_CTX2_KEY0;
        unsigned long OTFAD_CTX2_KEY1;
        unsigned long OTFAD_CTX2_KEY2;
        unsigned long OTFAD_CTX2_KEY3;
        unsigned long OTFAD_CTX2_CTR0;
        unsigned long OTFAD_CTX2_CTR1;
        unsigned long OTFAD_CTX2_RGD_W0;
        unsigned long OTFAD_CTX2_RGD_W1;
        unsigned long ulRes3[8];
        unsigned long OTFAD_CTX3_KEY0;
        unsigned long OTFAD_CTX3_KEY1;
        unsigned long OTFAD_CTX3_KEY2;
        unsigned long OTFAD_CTX3_KEY3;
        unsigned long OTFAD_CTX3_CTR0;
        unsigned long OTFAD_CTX3_CTR1;
        unsigned long OTFAD_CTX3_RGD_W0;
        unsigned long OTFAD_CTX3_RGD_W1;
    } iMX_OTFAD;
#else
    typedef struct st_iMX_BEE
    {
        unsigned long BEE_CTRL;
        unsigned long BEE_ADDR_OFFSET0;
        unsigned long BEE_ADDR_OFFSET1;
        unsigned long BEE_AES_KEY0_W0;
        unsigned long BEE_AES_KEY0_W1;
        unsigned long BEE_AES_KEY0_W2;
        unsigned long BEE_AES_KEY0_W3;
        unsigned long BEE_STATUS;
        unsigned long BEE_CTR_NONCE0_W0;
        unsigned long BEE_CTR_NONCE0_W1;
        unsigned long BEE_CTR_NONCE0_W2;
        unsigned long BEE_CTR_NONCE0_W3;
        unsigned long BEE_CTR_NONCE1_W0;
        unsigned long BEE_CTR_NONCE1_W1;
        unsigned long BEE_CTR_NONCE1_W2;
        unsigned long BEE_CTR_NONCE1_W3;
        unsigned long BEE_REGION1_TOP;
        unsigned long BEE_REGION1_BOT;
    } iMX_BEE;
#endif

typedef struct st_iMX_XBARA
{
    unsigned short XBARA_SEL0;
    unsigned short XBARA_SEL1;
    unsigned short XBARA_SEL2;
    unsigned short XBARA_SEL3;
    unsigned short XBARA_SEL4;
    unsigned short XBARA_SEL5;
    unsigned short XBARA_SEL6;
    unsigned short XBARA_SEL7;
    unsigned short XBARA_SEL8;
    unsigned short XBARA_SEL9;
    unsigned short XBARA_SEL10;
    unsigned short XBARA_SEL11;
    unsigned short XBARA_SEL12;
    unsigned short XBARA_SEL13;
    unsigned short XBARA_SEL14;
    unsigned short XBARA_SEL15;
    unsigned short XBARA_SEL16;
    unsigned short XBARA_SEL17;
    unsigned short XBARA_SEL18;
    unsigned short XBARA_SEL19;
    unsigned short XBARA_SEL20;
    unsigned short XBARA_SEL21;
    unsigned short XBARA_SEL22;
    unsigned short XBARA_SEL23;
    unsigned short XBARA_SEL24;
    unsigned short XBARA_SEL25;
    unsigned short XBARA_SEL26;
    unsigned short XBARA_SEL27;
    unsigned short XBARA_SEL28;
    unsigned short XBARA_SEL29;
    unsigned short XBARA_SEL30;
    unsigned short XBARA_SEL31;
    unsigned short XBARA_SEL32;
    unsigned short XBARA_SEL33;
    unsigned short XBARA_SEL34;
    unsigned short XBARA_SEL35;
    unsigned short XBARA_SEL36;
    unsigned short XBARA_SEL37;
    unsigned short XBARA_SEL38;
    unsigned short XBARA_SEL39;
    unsigned short XBARA_SEL40;
    unsigned short XBARA_SEL41;
    unsigned short XBARA_SEL42;
    unsigned short XBARA_SEL43;
    unsigned short XBARA_SEL44;
    unsigned short XBARA_SEL45;
    unsigned short XBARA_SEL46;
    unsigned short XBARA_SEL47;
    unsigned short XBARA_SEL48;
    unsigned short XBARA_SEL49;
    unsigned short XBARA_SEL50;
    unsigned short XBARA_SEL51;
    unsigned short XBARA_SEL52;
    unsigned short XBARA_SEL53;
    unsigned short XBARA_SEL54;
    unsigned short XBARA_SEL55;
    unsigned short XBARA_SEL56;
    unsigned short XBARA_SEL57;
    unsigned short XBARA_SEL58;
    unsigned short XBARA_SEL59;
    unsigned short XBARA_SEL60;
    unsigned short XBARA_SEL61;
    unsigned short XBARA_SEL62;
    unsigned short XBARA_SEL63;
    unsigned short XBARA_SEL64;
    unsigned short XBARA_SEL65;
    unsigned short XBARA_CTRL0;
    unsigned short XBARA_CTRL1;
} iMX_XBARA;

#if XBARA_AVAILABLE > 1
typedef struct st_iMX_XBARB
{
    unsigned short XBARB_SEL0;
    unsigned short XBARB_SEL1;
    unsigned short XBARB_SEL2;
    unsigned short XBARB_SEL3;
    unsigned short XBARB_SEL4;
    unsigned short XBARB_SEL5;
    unsigned short XBARB_SEL6;
    unsigned short XBARB_SEL7;
} iMX_XBARB;
#endif

typedef struct st_iMX_AOI
{
    unsigned short AOI_BFCRT010;
    unsigned short AOI_BFCRT230;
    unsigned short AOI_BFCRT011;
    unsigned short AOI_BFCRT231;
    unsigned short AOI_BFCRT012;
    unsigned short AOI_BFCRT232;
    unsigned short AOI_BFCRT013;
    unsigned short AOI_BFCRT233;
} iMX_AOI;

typedef struct st_iMX_IOMUXC_SNVS_GPR
{
    unsigned long IOMUXC_SNVS_GPR_GPR0;
    unsigned long IOMUXC_SNVS_GPR_GPR1;
    unsigned long IOMUXC_SNVS_GPR_GPR2;
    unsigned long IOMUXC_SNVS_GPR_GPR3;
} iMX_IOMUXC_SNVS_GPR;

typedef struct st_iMX_IOMUXC_SNVS
{
    unsigned long IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP;
    unsigned long IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ;
    unsigned long IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_STBY_REQ;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_TEST_MODE;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_POR_B;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_ONOFF;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_WAKEUP;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_PMIC_ON_REQ;
    unsigned long IOMUXC_SNVS_SW_PAD_CTL_PAD_PMIC_STBY_REQ;
} iMX_IOMUXC_SNVS;

typedef struct st_iMX_IOMUXC
{
    unsigned long IOMUXC_GPR_GPR0;
    unsigned long IOMUXC_GPR_GPR1;
    unsigned long IOMUXC_GPR_GPR2;
    unsigned long IOMUXC_GPR_GPR3;
    unsigned long IOMUXC_GPR_GPR4;
    unsigned long IOMUXC_GPR_GPR5;
    unsigned long IOMUXC_GPR_GPR6;
    unsigned long IOMUXC_GPR_GPR7;
    unsigned long IOMUXC_GPR_GPR8;
    unsigned long IOMUXC_GPR_GPR9;
    unsigned long IOMUXC_GPR_GPR10;
    unsigned long IOMUXC_GPR_GPR11;
    unsigned long IOMUXC_GPR_GPR12;
    unsigned long IOMUXC_GPR_GPR13;
    unsigned long IOMUXC_GPR_GPR14;
    unsigned long IOMUXC_GPR_GPR15;
    unsigned long IOMUXC_GPR_GPR16;
    unsigned long IOMUXC_GPR_GPR17;
    unsigned long IOMUXC_GPR_GPR18;
    unsigned long IOMUXC_GPR_GPR19;
    unsigned long IOMUXC_GPR_GPR20;
    unsigned long IOMUXC_GPR_GPR21;
    unsigned long IOMUXC_GPR_GPR22;
    unsigned long IOMUXC_GPR_GPR23;
    unsigned long IOMUXC_GPR_GPR24;
    unsigned long IOMUXC_GPR_GPR25;
#if defined iMX_RT106X
    unsigned long IOMUXC_GPR_GPR26;
    unsigned long IOMUXC_GPR_GPR27;
    unsigned long IOMUXC_GPR_GPR28;
    unsigned long IOMUXC_GPR_GPR29;
    unsigned long IOMUXC_GPR_GPR30;
    unsigned long IOMUXC_GPR_GPR31;
    unsigned long IOMUXC_GPR_GPR32;
    unsigned long IOMUXC_GPR_GPR33;
    unsigned long IOMUXC_GPR_GPR34;
#endif
} iMX_IOMUXC;

typedef struct st_iMX_IOMUXC_SW
{
#if defined iMX_RT1011
    unsigned long ulRes[4];
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_14;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_00;
    unsigned long IOMUXC_USB_OTG_ID_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA_SELECT_INPUT_0;
    unsigned long IOMUXC_FLEXPWM1_PWMA_SELECT_INPUT_1;
    unsigned long IOMUXC_FLEXPWM1_PWMA_SELECT_INPUT_2;
    unsigned long IOMUXC_FLEXPWM1_PWMA_SELECT_INPUT_3;
    unsigned long IOMUXC_FLEXPWM1_PWMB_SELECT_INPUT_0;
    unsigned long IOMUXC_FLEXPWM1_PWMB_SELECT_INPUT_1;
    unsigned long IOMUXC_FLEXPWM1_PWMB_SELECT_INPUT_2;
    unsigned long IOMUXC_FLEXPWM1_PWMB_SELECT_INPUT_3;
    unsigned long IOMUXC_FLEXSPI_DQS_FA_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_DQS_FB_SELECT_INPUT;
    unsigned long IOMUXC_KPP_COL_SELECT_INPUT_0;
    unsigned long IOMUXC_KPP_COL_SELECT_INPUT_1;
    unsigned long IOMUXC_KPP_COL_SELECT_INPUT_2;
    unsigned long IOMUXC_KPP_COL_SELECT_INPUT_3;
    unsigned long IOMUXC_KPP_ROW_SELECT_INPUT_0;
    unsigned long IOMUXC_KPP_ROW_SELECT_INPUT_1;
    unsigned long IOMUXC_KPP_ROW_SELECT_INPUT_2;
    unsigned long IOMUXC_KPP_ROW_SELECT_INPUT_3;
    unsigned long IOMUXC_LPI2C1_HREQ_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_PCS_SELECT_INPUT_0;
    unsigned long IOMUXC_LPSPI1_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_PCS_SELECT_INPUT_0;
    unsigned long IOMUXC_LPSPI2_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPUART1_RXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART1_TXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_RXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_TXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_RXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_TXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_RXD_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_TXD_SELECT_INPUT;
    unsigned long IOMUXC_NMI_GLUE_NMI_SELECT_INPUT;
    unsigned long IOMUXC_SPDIF_IN1_SELECT_INPUT;
    unsigned long IOMUXC_SPDIF_TX_CLK2_SELECT_INPUT;
    unsigned long IOMUXC_USB_OTG_OC_SELECT_INPUT;
    unsigned long IOMUXC_XEV_GLUE_RXEV_SELECT_INPUT;
#else
    unsigned long ulRes[5];
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_14;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_15;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_11;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_12;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_13;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_14;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_15;
    #if defined iMX_RT105X || defined iMX_RT106X
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_04;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_05;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_06;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_07;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_08;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_09;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_10;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_13;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_01;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_03;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14;
        unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15;
    #endif
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05;
    #if !defined iMX_RT105X && !defined iMX_RT106X
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_06;
    #endif
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_01;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_02;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_03;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_04;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_05;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_06;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_07;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_08;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_09;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_10;
    unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_15;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_16;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_17;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_18;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_19;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_20;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_21;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_22;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_23;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_25;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_26;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_27;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_28;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_29;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_30;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_31;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_33;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_34;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_35;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_36;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_37;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_38;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_39;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_41;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_15;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_15;
    #if defined iMX_RT105X || defined iMX_RT106X
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_15;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_12;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_13;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_14;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_15;
    #endif
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_05;
    #if !defined iMX_RT105X && !defined iMX_RT106X
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_06;
    #endif
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_00;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_01;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_02;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_03;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_04;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_05;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_06;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_07;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_08;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_09;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_10;
    unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B1_11;
    #if defined iMX_RT105X || defined iMX_RT106X
    unsigned long IOMUXC_ANATOP_USB_OTG1_ID_SELECT_INPUT;
    unsigned long IOMUXC_ANATOP_USB_OTG2_ID_SELECT_INPUT;
    unsigned long IOMUXC_CCM_PMIC_READY_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA02_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA03_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA04_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA05_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA06_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA07_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA08_SELECT_INPUT;
    unsigned long IOMUXC_CSI_DATA09_SELECT_INPUT;
    unsigned long IOMUXC_CSI_HSYNC_SELECT_INPUT;
    unsigned long IOMUXC_CSI_PIXCLK_SELECT_INPUT;
    unsigned long IOMUXC_CSI_VSYNC_SELECT_INPUT;
    unsigned long IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT;
    unsigned long IOMUXC_ENET_MDIO_SELECT_INPUT;
    unsigned long IOMUXC_ENET0_RXDATA_SELECT_INPUT;
    unsigned long IOMUXC_ENET1_RXDATA_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RXEN_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RXERR_SELECT_INPUT;
    unsigned long IOMUXC_ENET0_TIMER_SELECT_INPUT;
    unsigned long IOMUXC_ENET_TXCLK_SELECT_INPUT;
    unsigned long IOMUXC_FLEXCAN1_RX_SELECT_INPUT;
    unsigned long IOMUXC_FLEXCAN2_RX_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM4_PWMA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM4_PWMA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM4_PWMA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM4_PWMA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DQS_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_B_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_B_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_B_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_B_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C3_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C3_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C4_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C4_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI3_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI3_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI3_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI3_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_CTS_B_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART5_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART5_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART6_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART6_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART7_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART7_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART8_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART8_TX_SELECT_INPUT;
    unsigned long IOMUXC_NMI_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER0_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER1_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER2_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER3_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER3_TIMER0_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER3_TIMER1_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER3_TIMER2_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER3_TIMER3_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_MCLK2_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_TX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_TX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_MCLK2_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_TX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_TX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SPDIF_IN_SELECT_INPUT;
    unsigned long IOMUXC_USB_OTG2_OC_SELECT_INPUT;
    unsigned long IOMUXC_USB_OTG1_OC_SELECT_INPUT;
    unsigned long IOMUXC_USDHC1_CD_B_SELECT_INPUT;
    unsigned long IOMUXC_USDHC1_WP_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_CLK_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_CD_B_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_CMD_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA4_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA5_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA6_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_DATA7_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_WP_SELECT_INPUT;
        #if defined iMX_RT105X || defined iMX_RT106X
        unsigned long IOMUXC_XBAR1_IN02_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN03_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN04_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN05_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN06_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN07_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN08_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN09_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN17_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN18_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN20_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN22_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN23_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN24_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN14_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN15_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN16_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN25_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN19_SELECT_INPUT;
        unsigned long IOMUXC_XBAR1_IN21_SELECT_INPUT;
            #if defined iMX_RT1064
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_00;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_01;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_02;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_03;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_04;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_05;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_06;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_07;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_08;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_09;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_10;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_11;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_12;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_13;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_00;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_01;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_02;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_03;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_04;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_05;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_06;
            unsigned long IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_07;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_00;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_01;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_02;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_03;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_04;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_05;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_06;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_07;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_08;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_09;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_10;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_11;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_12;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_13;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_00;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_01;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_02;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_03;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_04;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_05;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_06;
            unsigned long IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_07;
            #endif
            #if !defined iMX_RT105X
            unsigned long IOMUXC_ENET2_IPG_CLK_RMII_SELECT_INPUT;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_MDIO_SELECT_INPUT;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_0;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_RXDATA_SELECT_INPUT_1;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_RXEN_SELECT_INPUT;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_RXERR_SELECT_INPUT;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_TIMER_SELECT_INPUT_0;
            unsigned long IOMUXC_ENET2_IPP_IND_MAC0_TXCLK_SELECT_INPUT;
            unsigned long ulRes1[11];
            unsigned long IOMUXC_GPT1_IPP_IND_CAPIN1_SELECT_INPUT;
            unsigned long IOMUXC_GPT1_IPP_IND_CAPIN2_SELECT_INPUT;
            unsigned long IOMUXC_GPT1_IPP_IND_CLKIN_SELECT_INPUT;
            unsigned long IOMUXC_GPT2_IPP_IND_CAPIN1_SELECT_INPUT;
            unsigned long IOMUXC_GPT2_IPP_IND_CAPIN2_SELECT_INPUT;
            unsigned long IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT;
            unsigned long IOMUXC_SAI3_IPG_CLK_SAI_MCLK_SELECT_INPUT_2;
            unsigned long IOMUXC_SAI3_IPP_IND_SAI_RXBCLK_SELECT_INPUT;
            unsigned long IOMUXC_SAI3_IPP_IND_SAI_RXDATA_SELECT_INPUT_0;
            unsigned long IOMUXC_SAI3_IPP_IND_SAI_RXSYNC_SELECT_INPUT;
            unsigned long IOMUXC_SAI3_IPP_IND_SAI_TXBCLK_SELECT_INPUT;
            unsigned long IOMUXC_SAI3_IPP_IND_SAI_TXSYNC_SELECT_INPUT;
            unsigned long IOMUXC_SEMC_I_IPP_IND_DQS4_SELECT_INPUT;
            unsigned long IOMUXC_CANFD_IPP_IND_CANRX_SELECT_INPUT;
            #endif
        #else
        unsigned long IOMUXC_XBAR_INOUT02_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT03_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT04_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT05_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT06_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT07_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT08_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT09_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT17_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT18_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT20_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT22_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT23_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT24_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT14_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT15_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT16_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT25_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT19_SELECT_INPUT;
        unsigned long IOMUXC_XBAR_INOUT21_SELECT_INPUT;
        #endif
    #else
    unsigned long IOMUXC_ANATOP_USB_OTG_ID_SELECT_INPUT;
    unsigned long IOMUXC_CCM_PMIC_READY_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RMII_SELECT_INPUT;
    unsigned long IOMUXC_ENET_MDIO_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RX_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RX_EN_SELECT_INPUT;
    unsigned long IOMUXC_ENET_RX_ERR_SELECT_INPUT;
    unsigned long IOMUXC_ENET_TX_CLK_SELECT_INPUT;
    unsigned long IOMUXC_FLEXCAN1_RX_SELECT_INPUT;
    unsigned long IOMUXC_FLEXCAN2_RX_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM1_PWMB3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXPWM2_PWMB3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_FLEXSPI_A_SCLK_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C1_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C2_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C3_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C3_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C4_SCL_SELECT_INPUT;
    unsigned long IOMUXC_LPI2C4_SDA_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI1_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI2_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_PCS0_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SCK_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SDI_SELECT_INPUT;
    unsigned long IOMUXC_LPSPI4_SDO_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_CTS_B_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART2_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART3_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_CTS_B_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART4_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART5_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART5_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART6_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART6_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART7_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART7_TX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART8_RX_SELECT_INPUT;
    unsigned long IOMUXC_LPUART8_TX_SELECT_INPUT;
    unsigned long IOMUXC_NMI_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER1_TIMER0_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER1_TIMER1_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER1_TIMER2_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER1_TIMER3_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER0_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER1_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER2_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_QTIMER2_TIMER3_INPUT_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_MCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA1_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA2_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_DATA3_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_RX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_TX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI1_TX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_MCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_RX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_TX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI2_TX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_MCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_RX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_RX_DATA0_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_RX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_TX_BCLK_SELECT_INPUT;
    unsigned long IOMUXC_SAI3_TX_SYNC_SELECT_INPUT;
    unsigned long IOMUXC_SEMC_READY_SELECT_INPUT;
    unsigned long IOMUXC_SPDIF_IN_SELECT_INPUT;
    unsigned long IOMUXC_USB_OTG_OC_SELECT_INPUT;
    unsigned long IOMUXC_USDHC1_CD_B_SELECT_INPUT;
    unsigned long IOMUXC_USDHC1_WP_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_CD_B_SELECT_INPUT;
    unsigned long IOMUXC_USDHC2_WP_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN14_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN15_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN16_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN17_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN10_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN12_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN13_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN18_SELECT_INPUT;
    unsigned long IOMUXC_XBAR1_IN19_SELECT_INPUT;
    #endif
#endif
} iMX_IOMUXC_SW;

#if defined SEMC_AVAILABLE
typedef struct st_iMX_SEMC
{
    unsigned long SEMC_MCR;
    unsigned long SEMC_IOCR;
    unsigned long SEMC_BMCR0;
    unsigned long SEMC_BMCR1;
    unsigned long SEMC_BR0;
    unsigned long SEMC_BR1;
    unsigned long SEMC_BR2;
    unsigned long SEMC_BR3;
    unsigned long SEMC_BR4;
    unsigned long SEMC_BR5;
    unsigned long SEMC_BR6;
    unsigned long SEMC_BR7;
    unsigned long SEMC_BR8;
    unsigned long ulRes0[1];
    unsigned long SEMC_INTEN;
    unsigned long SEMC_INTR;
    unsigned long SEMC_SDRAMCR0;
    unsigned long SEMC_SDRAMCR1;
    unsigned long SEMC_SDRAMCR2;
    unsigned long SEMC_SDRAMCR3;
    unsigned long SEMC_NANDCR0;
    unsigned long SEMC_NANDCR1;
    unsigned long SEMC_NANDCR2;
    unsigned long SEMC_NANDCR3;
    unsigned long SEMC_NORCR0;
    unsigned long SEMC_NORCR1;
    unsigned long SEMC_NORCR2;
    unsigned long SEMC_NORCR3;
    unsigned long SEMC_SRAMCR0;
    unsigned long SEMC_SRAMCR1;
    unsigned long SEMC_SRAMCR2;
    unsigned long SEMC_SRAMCR3;
    unsigned long SEMC_DBICR0;
    unsigned long SEMC_DBICR1;
    unsigned long ulRes1[2];
    unsigned long SEMC_IPCR0;
    unsigned long SEMC_IPCR1;
    unsigned long SEMC_IPCR2;
    unsigned long SEMC_IPCMD;
    unsigned long SEMC_IPTXDAT;
    unsigned long ulRes2[3];
    unsigned long SEMC_IPRXDAT;
    unsigned long ulRes3[3];
    unsigned long SEMC_STS0;
    unsigned long SEMC_STS1;
    unsigned long SEMC_STS2;
    unsigned long SEMC_STS3;
    unsigned long SEMC_STS4;
    unsigned long SEMC_STS5;
    unsigned long SEMC_STS6;
    unsigned long SEMC_STS7;
    unsigned long SEMC_STS8;
    unsigned long SEMC_STS9;
    unsigned long SEMC_STS10;
    unsigned long SEMC_STS11;
    unsigned long SEMC_STS12;
    unsigned long SEMC_STS13;
    unsigned long SEMC_STS14;
    unsigned long SEMC_STS15;
} iMX_SEMC;
#endif

typedef struct _st_iMX_FLEX_SPI
{
    unsigned long FLEX_SPI_MCR0;
    unsigned long FLEX_SPI_MCR1;
    unsigned long FLEX_SPI_MCR2;
    unsigned long FLEX_SPI_AHBCR;
    unsigned long FLEX_SPI_INTEN;
    unsigned long FLEX_SPI_INTR;
    unsigned long FLEX_SPI_LUTKEY;
    unsigned long FLEX_SPI_LUTCR;
    unsigned long FLEX_SPI_AHBRXBUF0CR0;
    unsigned long FLEX_SPI_AHBRXBUF1CR0;
    unsigned long FLEX_SPI_AHBRXBUF2CR0;
    unsigned long FLEX_SPI_AHBRXBUF3CR0;
    unsigned long ulRes0[12];
    unsigned long FLEX_SPI_FLSHA1CR0;
    unsigned long FLEX_SPI_FLSHA2CR0;
    unsigned long FLEX_SPI_FLSHB1CR0;
    unsigned long FLEX_SPI_FLSHB2CR0;
    unsigned long FLEX_SPI_FLSHA1CR1;
    unsigned long FLEX_SPI_FLSHA2CR1;
    unsigned long FLEX_SPI_FLSHB1CR1;
    unsigned long FLEX_SPI_FLSHB2CR1;
    unsigned long FLEX_SPI_FLSHA1CR2;
    unsigned long FLEX_SPI_FLSHA2CR2;
    unsigned long FLEX_SPI_FLSHB1CR2;
    unsigned long FLEX_SPI_FLSHB2CR2;
    unsigned long ulRes1;
    unsigned long FLEX_SPI_FLSHCR4;
    unsigned long ulRes2[2];
    unsigned long FLEX_SPI_IPCR0;
    unsigned long FLEX_SPI_IPCR1;
    unsigned long ulRes3[2];
    unsigned long FLEX_SPI_IPCMD;
    unsigned long ulRes4;
    unsigned long FLEX_SPI_IPRXFCR;
    unsigned long FLEX_SPI_IPTXFCR;
    unsigned long FLEX_SPI_DLLACR;
    unsigned long FLEX_SPI_DLLBCR;
    unsigned long ulRes5[6];
    unsigned long FLEX_SPI_STS0;
    unsigned long FLEX_SPI_STS1;
    unsigned long FLEX_SPI_STS2;
    unsigned long FLEX_SPI_AHBSPNDSTS;
    unsigned long FLEX_SPI_IPRXFSTS;
    unsigned long FLEX_SPI_IPTXFSTS;
    unsigned long ulRes6[2];
    unsigned long FLEX_SPI_RFDR0;
    unsigned long FLEX_SPI_RFDR1;
    unsigned long FLEX_SPI_RFDR2;
    unsigned long FLEX_SPI_RFDR3;
    unsigned long FLEX_SPI_RFDR4;
    unsigned long FLEX_SPI_RFDR5;
    unsigned long FLEX_SPI_RFDR6;
    unsigned long FLEX_SPI_RFDR7;
    unsigned long FLEX_SPI_RFDR8;
    unsigned long FLEX_SPI_RFDR9;
    unsigned long FLEX_SPI_RFDR10;
    unsigned long FLEX_SPI_RFDR11;
    unsigned long FLEX_SPI_RFDR12;
    unsigned long FLEX_SPI_RFDR13;
    unsigned long FLEX_SPI_RFDR14;
    unsigned long FLEX_SPI_RFDR15;
    unsigned long FLEX_SPI_RFDR16;
    unsigned long FLEX_SPI_RFDR17;
    unsigned long FLEX_SPI_RFDR18;
    unsigned long FLEX_SPI_RFDR19;
    unsigned long FLEX_SPI_RFDR20;
    unsigned long FLEX_SPI_RFDR21;
    unsigned long FLEX_SPI_RFDR22;
    unsigned long FLEX_SPI_RFDR23;
    unsigned long FLEX_SPI_RFDR24;
    unsigned long FLEX_SPI_RFDR25;
    unsigned long FLEX_SPI_RFDR26;
    unsigned long FLEX_SPI_RFDR27;
    unsigned long FLEX_SPI_RFDR28;
    unsigned long FLEX_SPI_RFDR29;
    unsigned long FLEX_SPI_RFDR30;
    unsigned long FLEX_SPI_RFDR31;
    unsigned long FLEX_SPI_TFDR0;
    unsigned long FLEX_SPI_TFDR1;
    unsigned long FLEX_SPI_TFDR2;
    unsigned long FLEX_SPI_TFDR3;
    unsigned long FLEX_SPI_TFDR4;
    unsigned long FLEX_SPI_TFDR5;
    unsigned long FLEX_SPI_TFDR6;
    unsigned long FLEX_SPI_TFDR7;
    unsigned long FLEX_SPI_TFDR8;
    unsigned long FLEX_SPI_TFDR9;
    unsigned long FLEX_SPI_TFDR10;
    unsigned long FLEX_SPI_TFDR11;
    unsigned long FLEX_SPI_TFDR12;
    unsigned long FLEX_SPI_TFDR13;
    unsigned long FLEX_SPI_TFDR14;
    unsigned long FLEX_SPI_TFDR15;
    unsigned long FLEX_SPI_TFDR16;
    unsigned long FLEX_SPI_TFDR17;
    unsigned long FLEX_SPI_TFDR18;
    unsigned long FLEX_SPI_TFDR19;
    unsigned long FLEX_SPI_TFDR20;
    unsigned long FLEX_SPI_TFDR21;
    unsigned long FLEX_SPI_TFDR22;
    unsigned long FLEX_SPI_TFDR23;
    unsigned long FLEX_SPI_TFDR24;
    unsigned long FLEX_SPI_TFDR25;
    unsigned long FLEX_SPI_TFDR26;
    unsigned long FLEX_SPI_TFDR27;
    unsigned long FLEX_SPI_TFDR28;
    unsigned long FLEX_SPI_TFDR29;
    unsigned long FLEX_SPI_TFDR30;
    unsigned long FLEX_SPI_TFDR31;
    unsigned long FLEX_SPI_LUT0;
    unsigned long FLEX_SPI_LUT1;
    unsigned long FLEX_SPI_LUT2;
    unsigned long FLEX_SPI_LUT3;
    unsigned long FLEX_SPI_LUT4;
    unsigned long FLEX_SPI_LUT5;
    unsigned long FLEX_SPI_LUT6;
    unsigned long FLEX_SPI_LUT7;
    unsigned long FLEX_SPI_LUT8;
    unsigned long FLEX_SPI_LUT9;
    unsigned long FLEX_SPI_LUT10;
    unsigned long FLEX_SPI_LUT11;
    unsigned long FLEX_SPI_LUT12;
    unsigned long FLEX_SPI_LUT13;
    unsigned long FLEX_SPI_LUT14;
    unsigned long FLEX_SPI_LUT15;
    unsigned long FLEX_SPI_LUT16;
    unsigned long FLEX_SPI_LUT17;
    unsigned long FLEX_SPI_LUT18;
    unsigned long FLEX_SPI_LUT19;
    unsigned long FLEX_SPI_LUT20;
    unsigned long FLEX_SPI_LUT21;
    unsigned long FLEX_SPI_LUT22;
    unsigned long FLEX_SPI_LUT23;
    unsigned long FLEX_SPI_LUT24;
    unsigned long FLEX_SPI_LUT25;
    unsigned long FLEX_SPI_LUT26;
    unsigned long FLEX_SPI_LUT27;
    unsigned long FLEX_SPI_LUT28;
    unsigned long FLEX_SPI_LUT29;
    unsigned long FLEX_SPI_LUT30;
    unsigned long FLEX_SPI_LUT31;
    unsigned long FLEX_SPI_LUT32;
    unsigned long FLEX_SPI_LUT33;
    unsigned long FLEX_SPI_LUT34;
    unsigned long FLEX_SPI_LUT35;
    unsigned long FLEX_SPI_LUT36;
    unsigned long FLEX_SPI_LUT37;
    unsigned long FLEX_SPI_LUT38;
    unsigned long FLEX_SPI_LUT39;
    unsigned long FLEX_SPI_LUT40;
    unsigned long FLEX_SPI_LUT41;
    unsigned long FLEX_SPI_LUT42;
    unsigned long FLEX_SPI_LUT43;
    unsigned long FLEX_SPI_LUT44;
    unsigned long FLEX_SPI_LUT45;
    unsigned long FLEX_SPI_LUT46;
    unsigned long FLEX_SPI_LUT47;
    unsigned long FLEX_SPI_LUT48;
    unsigned long FLEX_SPI_LUT49;
    unsigned long FLEX_SPI_LUT50;
    unsigned long FLEX_SPI_LUT51;
    unsigned long FLEX_SPI_LUT52;
    unsigned long FLEX_SPI_LUT53;
    unsigned long FLEX_SPI_LUT54;
    unsigned long FLEX_SPI_LUT55;
    unsigned long FLEX_SPI_LUT56;
    unsigned long FLEX_SPI_LUT57;
    unsigned long FLEX_SPI_LUT58;
    unsigned long FLEX_SPI_LUT59;
    unsigned long FLEX_SPI_LUT60;
    unsigned long FLEX_SPI_LUT61;
    unsigned long FLEX_SPI_LUT62;
    unsigned long FLEX_SPI_LUT63;
} _iMX_FLEX_SPI;

typedef struct st_iMX_SRC
{
    unsigned long SRC_SCR;
    unsigned long SRC_SBMR1;
    unsigned long SRC_SRSR;
    unsigned long ulRes0[3];
    unsigned long SRC_SBMR2;
    unsigned long SRC_GPR1;
    unsigned long SRC_GPR2;
    unsigned long SRC_GPR3;
    unsigned long SRC_GPR4;
    unsigned long SRC_GPR5;
    unsigned long SRC_GPR6;
    unsigned long SRC_GPR7;
    unsigned long SRC_GPR8;
    unsigned long SRC_GPR9;
    unsigned long SRC_GPR10;
} iMX_SRC;

typedef struct st_iMX_GPT
{
    unsigned long GPT_CR;
    unsigned long GPT_PR;
    unsigned long GPT_SR;
    unsigned long GPT_IR;
    unsigned long GPT_OCR1;
    unsigned long GPT_OCR2;
    unsigned long GPT_OCR3;
    unsigned long GPT_ICR1;
    unsigned long GPT_ICR2;
    unsigned long GPT_CNT;
} iMX_GPT;

typedef struct st_iMX_FLEXPWM
{
    unsigned short FLEXPWM_SM0CNT;
    unsigned short FLEXPWM_SM0INIT;
    unsigned short FLEXPWM_SM0CTRL2;
    unsigned short FLEXPWM_SM0CTRL;
    unsigned short usRev0;
    unsigned short FLEXPWM_SM0VAL0;
    unsigned short FLEXPWM_SM0FRACVAL1;
    unsigned short FLEXPWM_SM0VAL1;
    unsigned short FLEXPWM_SM0FRACVAL2;
    unsigned short FLEXPWM_SM0VAL2;
    unsigned short FLEXPWM_SM0FRACVAL3;
    unsigned short FLEXPWM_SM0VAL3;
    unsigned short FLEXPWM_SM0FRACVAL4;
    unsigned short FLEXPWM_SM0VAL4;
    unsigned short FLEXPWM_SM0FRACVAL5;
    unsigned short FLEXPWM_SM0VAL5;
    unsigned short FLEXPWM_SM0FRCTRL;
    unsigned short FLEXPWM_SM0OCTRL;
    unsigned short FLEXPWM_SM0STS;
    unsigned short FLEXPWM_SM0INTEN;
    unsigned short FLEXPWM_SM0DMAEN;
    unsigned short FLEXPWM_SM0TCTRL;
    unsigned short FLEXPWM_SM0DISPMAP0;
    unsigned short FLEXPWM_SM0DISPMAP1;
    unsigned short FLEXPWM_SM0DTCNT0;
    unsigned short FLEXPWM_SM0DTCNT1;
    unsigned short FLEXPWM_SM0CAPTCTRLA;
    unsigned short FLEXPWM_SM0CAPTCOMPA;
    unsigned short FLEXPWM_SM0CAPTCTRLB;
    unsigned short FLEXPWM_SM0CAPTCOMPB;
    unsigned short FLEXPWM_SM0CAPTCTRLX;
    unsigned short FLEXPWM_SM0CAPTCOMPX;
    unsigned short FLEXPWM_SM0CVAL0;
    unsigned short FLEXPWM_SM0CVAL0CYC;
    unsigned short FLEXPWM_SM0CVAL1;
    unsigned short FLEXPWM_SM0CVAL1CYC;
    unsigned short FLEXPWM_SM0CVAL2;
    unsigned short FLEXPWM_SM0CVAL2CYC;
    unsigned short FLEXPWM_SM0CVAL3;
    unsigned short FLEXPWM_SM0CVAL3CYC;
    unsigned short FLEXPWM_SM0CVAL4;
    unsigned short FLEXPWM_SM0CVAL4CYC;
    unsigned short FLEXPWM_SM0CVAL5;
    unsigned short FLEXPWM_SM0CVAL5CYC;
    unsigned short usRev1[4];
    unsigned short FLEXPWM_SM1CNT;
    unsigned short FLEXPWM_SM1INIT;
    unsigned short FLEXPWM_SM1CTRL2;
    unsigned short FLEXPWM_SM1CTRL;
    unsigned short usRev2;
    unsigned short FLEXPWM_SM1VAL0;
    unsigned short FLEXPWM_SM1FRACVAL1;
    unsigned short FLEXPWM_SM1VAL1;
    unsigned short FLEXPWM_SM1FRACVAL2;
    unsigned short FLEXPWM_SM1VAL2;
    unsigned short FLEXPWM_SM1FRACVAL3;
    unsigned short FLEXPWM_SM1VAL3;
    unsigned short FLEXPWM_SM1FRACVAL4;
    unsigned short FLEXPWM_SM1VAL4;
    unsigned short FLEXPWM_SM1FRACVAL5;
    unsigned short FLEXPWM_SM1VAL5;
    unsigned short FLEXPWM_SM1FRCTRL;
    unsigned short FLEXPWM_SM1OCTRL;
    unsigned short FLEXPWM_SM1STS;
    unsigned short FLEXPWM_SM1INTEN;
    unsigned short FLEXPWM_SM1DMAEN;
    unsigned short FLEXPWM_SM1TCTRL;
    unsigned short FLEXPWM_SM1DISPMAP0;
    unsigned short FLEXPWM_SM1DISPMAP1;
    unsigned short FLEXPWM_SM1DTCNT0;
    unsigned short FLEXPWM_SM1DTCNT1;
    unsigned short FLEXPWM_SM1CAPTCTRLA;
    unsigned short FLEXPWM_SM1CAPTCOMPA;
    unsigned short FLEXPWM_SM1CAPTCTRLB;
    unsigned short FLEXPWM_SM1CAPTCOMPB;
    unsigned short FLEXPWM_SM1CAPTCTRLX;
    unsigned short FLEXPWM_SM1CAPTCOMP;
    unsigned short FLEXPWM_SM1CVAL0;
    unsigned short FLEXPWM_SM1CVAL0CYC;
    unsigned short FLEXPWM_SM1CVAL1;
    unsigned short FLEXPWM_SM1CVAL1CYC;
    unsigned short FLEXPWM_SM1CVAL2;
    unsigned short FLEXPWM_SM1CVAL2CYC;
    unsigned short FLEXPWM_SM1CVAL3;
    unsigned short FLEXPWM_SM1CVAL3CYC;
    unsigned short FLEXPWM_SM1CVAL4;
    unsigned short FLEXPWM_SM1CVAL4CYC;
    unsigned short FLEXPWM_SM1CVAL5;
    unsigned short FLEXPWM_SM1CVAL5CYC;
    unsigned short usRev3[4];
    unsigned short FLEXPWM_SM2CNT;
    unsigned short FLEXPWM_SM2INIT;
    unsigned short FLEXPWM_SM2CTRL2;
    unsigned short FLEXPWM_SM2CTRL;
    unsigned short usRev4;
    unsigned short FLEXPWM_SM2VAL0;
    unsigned short FLEXPWM_SM2FRACVAL1;
    unsigned short FLEXPWM_SM2VAL1;
    unsigned short FLEXPWM_SM2FRACVAL2;
    unsigned short FLEXPWM_SM2VAL2;
    unsigned short FLEXPWM_SM2FRACVAL3;
    unsigned short FLEXPWM_SM2VAL3;
    unsigned short FLEXPWM_SM2FRACVAL4;
    unsigned short FLEXPWM_SM2VAL4;
    unsigned short FLEXPWM_SM2FRACVAL5;
    unsigned short FLEXPWM_SM2VAL5;
    unsigned short FLEXPWM_SM2FRCTRL;
    unsigned short FLEXPWM_SM2OCTRL;
    unsigned short FLEXPWM_SM2STS;
    unsigned short FLEXPWM_SM2INTEN;
    unsigned short FLEXPWM_SM2DMAEN;
    unsigned short FLEXPWM_SM2TCTRL;
    unsigned short FLEXPWM_SM2DISPMAP0;
    unsigned short FLEXPWM_SM2DISPMAP1;
    unsigned short FLEXPWM_SM2DTCNT0;
    unsigned short FLEXPWM_SM2DTCNT1;
    unsigned short FLEXPWM_SM2CAPTCTRLA;
    unsigned short FLEXPWM_SM2CAPTCOMPA;
    unsigned short FLEXPWM_SM2CAPTCTRLB;
    unsigned short FLEXPWM_SM2CAPTCOMPB;
    unsigned short FLEXPWM_SM2CAPTCTRLX;
    unsigned short FLEXPWM_SM2CAPTCOMPX;
    unsigned short FLEXPWM_SM2CVAL0;
    unsigned short FLEXPWM_SM2CVAL0CYC;
    unsigned short FLEXPWM_SM2CVAL1;
    unsigned short FLEXPWM_SM2CVAL1CYC;
    unsigned short FLEXPWM_SM2CVAL2;
    unsigned short FLEXPWM_SM2CVAL2CYC;
    unsigned short FLEXPWM_SM2CVAL3;
    unsigned short FLEXPWM_SM2CVAL3CYC;
    unsigned short FLEXPWM_SM2CVAL4;
    unsigned short FLEXPWM_SM2CVAL4CYC;
    unsigned short FLEXPWM_SM2CVAL5;
    unsigned short FLEXPWM_SM2CVAL5CYC;
    unsigned short usRev5[4];
    unsigned short FLEXPWM_SM3CNT;
    unsigned short FLEXPWM_SM3INIT;
    unsigned short FLEXPWM_SM3CTRL2;
    unsigned short FLEXPWM_SM3CTRL;
    unsigned short usRev6;
    unsigned short FLEXPWM_SM3VAL0;
    unsigned short FLEXPWM_SM3FRACVAL1;
    unsigned short FLEXPWM_SM3VAL1;
    unsigned short FLEXPWM_SM3FRACVAL2;
    unsigned short FLEXPWM_SM3VAL2;
    unsigned short FLEXPWM_SM3FRACVAL3;
    unsigned short FLEXPWM_SM3VAL3;
    unsigned short FLEXPWM_SM3FRACVAL4;
    unsigned short FLEXPWM_SM3VAL4;
    unsigned short FLEXPWM_SM3FRACVAL5;
    unsigned short FLEXPWM_SM3VAL5;
    unsigned short FLEXPWM_SM3FRCTRL;
    unsigned short FLEXPWM_SM3OCTRL;
    unsigned short FLEXPWM_SM3STS;
    unsigned short FLEXPWM_SM3INTEN;
    unsigned short FLEXPWM_SM3DMAEN;
    unsigned short FLEXPWM_SM3TCTRL;
    unsigned short FLEXPWM_SM3DISPMAP0;
    unsigned short FLEXPWM_SM3DISPMAP1;
    unsigned short FLEXPWM_SM3DTCNT0;
    unsigned short FLEXPWM_SM3DTCNT1;
    unsigned short FLEXPWM_SM3CAPTCTRLA;
    unsigned short FLEXPWM_SM3CAPTCOMPA;
    unsigned short FLEXPWM_SM3CAPTCTRLB;
    unsigned short FLEXPWM_SM3CAPTCOMPB;
    unsigned short FLEXPWM_SM3CAPTCTRLX;
    unsigned short FLEXPWM_SM3CAPTCOMPX;
    unsigned short FLEXPWM_SM3CVAL0;
    unsigned short FLEXPWM_SM3CVAL0CYC;
    unsigned short FLEXPWM_SM3CVAL1;
    unsigned short FLEXPWM_SM3CVAL1CYC;
    unsigned short FLEXPWM_SM3CVAL2;
    unsigned short FLEXPWM_SM3CVAL2CYC;
    unsigned short FLEXPWM_SM3CVAL3;
    unsigned short FLEXPWM_SM3CVAL3CYC;
    unsigned short FLEXPWM_SM3CVAL4;
    unsigned short FLEXPWM_SM3CVAL4CYC;
    unsigned short FLEXPWM_SM3CVAL5;
    unsigned short FLEXPWM_SM3CVAL5CYC;
    unsigned short usRev7[4];
    unsigned short FLEXPWM_OUTEN;
    unsigned short FLEXPWM_MASK;
    unsigned short FLEXPWM_SWCOUT;
    unsigned short FLEXPWM_DTSRCSEL;
    unsigned short FLEXPWM_MCTRL;
    unsigned short FLEXPWM_MCTRL2;
    unsigned short FLEXPWM_FCTRL0;
    unsigned short FLEXPWM_FSTS0;
    unsigned short FLEXPWM_FFILT0;
    unsigned short FLEXPWM_FTST0;
    unsigned short FLEXPWM_FCTRL20;
} iMX_FLEXPWM;

typedef struct st_iMX_CCM
{
    unsigned long CCM_CCR;
    unsigned long ulRes0;
    unsigned long CCM_CSR;
    unsigned long CCM_CCSR;
#if !defined iMX_RT1011
    unsigned long CCM_CACRR;
#else
    unsigned long ulRes1;
#endif
    unsigned long CCM_CBCDR;
    unsigned long CCM_CBCMR;
    unsigned long CCM_CSCMR1;
    unsigned long CCM_CSCMR2;
    unsigned long CCM_CSCDR1;
    unsigned long CCM_CS1CDR;
    unsigned long CCM_CS2CDR;
    unsigned long CCM_CDCDR;
    unsigned long ulRes2;
    unsigned long CCM_CSCDR2;
    unsigned long CCM_CSCDR3;
    unsigned long ulRes3[2];
    unsigned long CCM_CDHIPR;
    unsigned long ulRes4[2];
    unsigned long CCM_CLPCR;
    unsigned long CCM_CISR;
    unsigned long CCM_CIMR;
    unsigned long CCM_CCOSR;
    unsigned long CCM_CGPR;
    unsigned long CCM_CCGR0;
    unsigned long CCM_CCGR1;
    unsigned long CCM_CCGR2;
    unsigned long CCM_CCGR3;
    unsigned long CCM_CCGR4;
    unsigned long CCM_CCGR5;
    unsigned long CCM_CCGR6;
    unsigned long CCM_CMEOR;
} iMX_CCM;

typedef struct st_iMX_SNVS
{
    unsigned long SNVS_HPLR;
    unsigned long SNVS_HPCOMR;
    unsigned long SNVS_HPCR;
    unsigned long ulRes0;
    unsigned long SNVS_HPSR;
    unsigned long ulRes1[3];
    unsigned long SNVS_HPRTCMR;
    unsigned long SNVS_HPRTCLR;
    unsigned long SNVS_HPTAMR;
    unsigned long SNVS_HPTALR;
    unsigned long SNVS_LPLR;
    unsigned long SNVS_LPCR;
    unsigned long ulRes2[4];
    unsigned long SNVS_LPSR;
    unsigned long ulRes3[3];
    unsigned long SNVS_LPSMCMR;
    unsigned long SNVS_LPSMCLR;
    unsigned long ulRes4;
    unsigned long SNVS_LPGPR0_legacy_alias;
    unsigned long ulRes5[9];
    unsigned long SNVS_LPGPR0_alias;
    unsigned long SNVS_LPGPR1_alias;
    unsigned long SNVS_LPGPR2_alias;
    unsigned long SNVS_LPGPR3_alias;
    unsigned long ulRes6[24];
    unsigned long SNVS_LPGPR0;
    unsigned long SNVS_LPGPR1;
    unsigned long SNVS_LPGPR2;
    unsigned long SNVS_LPGPR3;
    unsigned long ulRes7[698];
    unsigned long SNVS_HPVIDR1;
    unsigned long SNVS_HPVIDR2;
} iMX_SNVS;

typedef struct st_iMX_CCM_ANALOG
{
#if defined ARM_PLL_AVAILABLE
    unsigned long CCM_ANALOG_PLL_ARM;
    unsigned long CCM_ANALOG_PLL_ARM_SET;
    unsigned long CCM_ANALOG_PLL_ARM_CLR;
    unsigned long CCM_ANALOG_PLL_ARM_TOG;
#else
    unsigned long ulRes0[4];
#endif
    unsigned long CCM_ANALOG_PLL_USB1;
    unsigned long CCM_ANALOG_PLL_USB1_SET;
    unsigned long CCM_ANALOG_PLL_USB1_CLR;
    unsigned long CCM_ANALOG_PLL_USB1_TOG;
#if HSUSB_CONTROLLERS > 1
    unsigned long CCM_ANALOG_PLL_USB2;
    unsigned long CCM_ANALOG_PLL_USB2_SET;
    unsigned long CCM_ANALOG_PLL_USB2_CLR;
    unsigned long CCM_ANALOG_PLL_USB2_TOG;
#else
    unsigned long ulRes1[4];
#endif
    unsigned long CCM_ANALOG_PLL_SYS;
    unsigned long CCM_ANALOG_PLL_SYS_SET;
    unsigned long CCM_ANALOG_PLL_SYS_CLR;
    unsigned long CCM_ANALOG_PLL_SYS_TOG;
    unsigned long CCM_ANALOG_PLL_SYS_SS;
    unsigned long ulRes2[3];
    unsigned long CCM_ANALOG_PLL_SYS_NUM;
    unsigned long ulRes3[3];
    unsigned long CCM_ANALOG_PLL_SYS_DENOM;
    unsigned long ulRes4[3];
    unsigned long CCM_ANALOG_PLL_AUDIO;
    unsigned long CCM_ANALOG_PLL_AUDIO_SET;
    unsigned long CCM_ANALOG_PLL_AUDIO_CLR;
    unsigned long CCM_ANALOG_PLL_AUDIO_TOG;
    unsigned long CCM_ANALOG_PLL_AUDIO_NUM;
    unsigned long ulRes5[3];
    unsigned long CCM_ANALOG_PLL_AUDIO_DENOM;
    #if defined ENHANCED_LCD_INTERFACE
       unsigned long ulRes6[3];
        unsigned long  CCM_ANALOG_PLL_VIDEO;
        unsigned long  CCM_ANALOG_PLL_VIDEO_SET;
        unsigned long  CCM_ANALOG_PLL_VIDEO_CLR;
        unsigned long  CCM_ANALOG_PLL_VIDEO_TOG;
        unsigned long  CCM_ANALOG_PLL_VIDEO_NUM;
        unsigned long ulRes6a[3];
        unsigned long  CCM_ANALOG_PLL_VIDEO_DENOM;
        unsigned long ulRes6b[7];
    #else
        unsigned long ulRes6[19];
    #endif
    unsigned long CCM_ANALOG_PLL_ENET;
    unsigned long CCM_ANALOG_PLL_ENET_SET;
    unsigned long CCM_ANALOG_PLL_ENET_CLR;
    unsigned long CCM_ANALOG_PLL_ENET_TOG;
    unsigned long CCM_ANALOG_PFD_480;
    unsigned long CCM_ANALOG_PFD_480_SET;
    unsigned long CCM_ANALOG_PFD_480_CLR;
    unsigned long CCM_ANALOG_PFD_480_TOG;
    unsigned long CCM_ANALOG_PFD_528;
    unsigned long CCM_ANALOG_PFD_528_SET;
    unsigned long CCM_ANALOG_PFD_528_CLR;
    unsigned long CCM_ANALOG_PFD_528_TOG;
    // PMU within CCM_ANALOG_BLOCK
    //
    unsigned long PMU_REG_1P1;
    unsigned long PMU_REG_1P1_SET;
    unsigned long PMU_REG_1P1_CLR;
    unsigned long PMU_REG_1P1_TOG;
    unsigned long PMU_REG_3P0;
    unsigned long PMU_REG_3P0_SET;
    unsigned long PMU_REG_3P0_CLR;
    unsigned long PMU_REG_3P0_TOG;
    unsigned long PMU_REG_2P5;
    unsigned long PMU_REG_2P5_SET;
    unsigned long PMU_REG_2P5_CLR;
    unsigned long PMU_REG_2P5_TOG;
    unsigned long PMU_REG_CORE;
    unsigned long PMU_REG_CORE_SET;
    unsigned long PMU_REG_CORE_CLR;
    unsigned long PMU_REG_CORE_TOG;
    //
    unsigned long CCM_ANALOG_MISC0;
    unsigned long CCM_ANALOG_MISC0_SET;
    unsigned long CCM_ANALOG_MISC0_CLR;
    unsigned long CCM_ANALOG_MISC0_TOG;
    unsigned long CCM_ANALOG_MISC1;
    unsigned long CCM_ANALOG_MISC1_SET;
    unsigned long CCM_ANALOG_MISC1_CLR;
    unsigned long CCM_ANALOG_MISC1_TOG;
    unsigned long CCM_ANALOG_MISC2;
    unsigned long CCM_ANALOG_MISC2_SET;
    unsigned long CCM_ANALOG_MISC2_CLR;
    unsigned long CCM_ANALOG_MISC2_TOG;
    unsigned long ulRes7[8];
    // USB analog
    //
    unsigned long USB_ANALOG_USB1_VBUS_DETECT;
    unsigned long USB_ANALOG_USB1_VBUS_DETECT_SET;
    unsigned long USB_ANALOG_USB1_VBUS_DETECT_CLR;
    unsigned long USB_ANALOG_USB1_VBUS_DETECT_TOG;
    unsigned long USB_ANALOG_USB1_CHRG_DETECT;
    unsigned long USB_ANALOG_USB1_CHRG_DETECT_SET;
    unsigned long USB_ANALOG_USB1_CHRG_DETECT_CLR;
    unsigned long USB_ANALOG_USB1_CHRG_DETECT_TOG;
    unsigned long USB_ANALOG_USB1_VBUS_DETECT_STAT;
    unsigned long ulRes8[3];
    unsigned long USB_ANALOG_USB1_CHRG_DETECT_STAT;
    unsigned long ulRes9[3];
    unsigned long USB_ANALOG_USB1_LOOPBACK;
    unsigned long USB_ANALOG_USB1_LOOPBACK_SET;
    unsigned long USB_ANALOG_USB1_LOOPBACK_CLR;
    unsigned long USB_ANALOG_USB1_LOOPBACK_TOG;
    unsigned long USB_ANALOG_USB1_MISC;
    unsigned long USB_ANALOG_USB1_MISC_SET;
    unsigned long USB_ANALOG_USB1_MISC_CLR;
    unsigned long USB_ANALOG_USB1_MISC_TOG;
#if HSUSB_CONTROLLERS > 1
    unsigned long USB_ANALOG_USB2_VBUS_DETECT;
    unsigned long USB_ANALOG_USB2_VBUS_DETECT_SET;
    unsigned long USB_ANALOG_USB2_VBUS_DETECT_CLR;
    unsigned long USB_ANALOG_USB2_VBUS_DETECT_TOG;
    unsigned long USB_ANALOG_USB2_CHRG_DETECT;
    unsigned long USB_ANALOG_USB2_CHRG_DETECT_SET;
    unsigned long USB_ANALOG_USB2_CHRG_DETECT_CLR;
    unsigned long USB_ANALOG_USB2_CHRG_DETECT_TOG;
    unsigned long USB_ANALOG_USB2_VBUS_DETECT_STAT;
    unsigned long ulRes10[3];
    unsigned long USB_ANALOG_USB2_CHRG_DETECT_STAT;
    unsigned long ulRes11[3];
    unsigned long USB_ANALOG_USB2_LOOPBACK;
    unsigned long USB_ANALOG_USB2_LOOPBACK_SET;
    unsigned long USB_ANALOG_USB2_LOOPBACK_CLR;
    unsigned long USB_ANALOG_USB2_LOOPBACK_TOG;
    unsigned long USB_ANALOG_USB2_MISC;
    unsigned long USB_ANALOG_USB2_MISC_SET;
    unsigned long USB_ANALOG_USB2_MISC_CLR;
    unsigned long USB_ANALOG_USB2_MISC_TOG;
    unsigned long USB_ANALOG_DIGPROG;
#endif
} iMX_CCM_ANALOG;

typedef struct _st_iMX_ADC
{
    unsigned long ADC_HC0;
    unsigned long ADC_HC1;
    unsigned long ADC_HC2;
    unsigned long ADC_HC3;
    unsigned long ADC_HC4;
    unsigned long ADC_HC5;
    unsigned long ADC_HC6;
    unsigned long ADC_HC7;
    volatile unsigned long ADC_HS;
    volatile unsigned long ADC_R0;
    volatile unsigned long ADC_R1;
    volatile unsigned long ADC_R2;
    volatile unsigned long ADC_R3;
    volatile unsigned long ADC_R4;
    volatile unsigned long ADC_R5;
    volatile unsigned long ADC_R6;
    volatile unsigned long ADC_R7;
    unsigned long ADC_CFG;
    unsigned long ADC_GC;
    volatile unsigned long ADC_GS;
    unsigned long ADC_CV;
    unsigned long ADC_OFS;
    volatile unsigned long ADC_CAL;
} _iMX_ADC;

typedef struct st_iMX_USBHS
{
    unsigned long USBHS_ID;
    unsigned long USBHS_HWGENERAL;
    unsigned long USBHS_HWHOST;
    unsigned long USBHS_HWDEVICE;
    unsigned long USBHS_HWTXBUF;
    unsigned long USBHS_HWRXBUF;
    unsigned long ulRes1[0x1a];
    unsigned long USBHS_GPTIMER0LD;
    unsigned long USBHS_GPTIMER0CTL;
    unsigned long USBHS_GPTIMER1LD;
    unsigned long USBHS_GPTIMER1CTL;
    unsigned long USBHS_USB_SBUSCFG;
    unsigned long ulRes2[0x1b];
    unsigned long USBHS_HCIVERSION;
    unsigned long USBHS_HCSPARAMS;
    unsigned long USBHS_HCCPARAMS;
    unsigned long ulRes3[0x5];
    unsigned short usRes4;
    unsigned short USBHS_DCIVERSION;
    unsigned long USBHS_DCCPARAMS;
    unsigned long ulRes5[0x6];
    unsigned long USBHS_USBCMD;
    unsigned long USBHS_USBSTS;
    unsigned long USBHS_USBINTR;
    unsigned long USBHS_FRINDEX;
    unsigned long ulRes6;
    unsigned long USBHS_PERIODICLISTBASE_DEVICEADDR;
    unsigned long USBHS_ASYNCLISTADDR_EPLISTADDR;
    unsigned long USBHS_TTCTRL;
    unsigned long USBHS_BURSTSIZE;
    unsigned long USBHS_TXFILLTUNING;
    unsigned long ulRes7[0x2];
    unsigned long USBHS_ULPI_VIEWPORT;
    unsigned long ulRes8;
    unsigned long USBHS_ENDPTNAK;
    unsigned long USBHS_ENDPTNAKEN;
    unsigned long USBHS_CONFIGFLAG;
    unsigned long USBHS_PORTSC1;
    unsigned long ulRes9[0x7];
    unsigned long USBHS_OTGSC;
    unsigned long USBHS_USBMODE;
    unsigned long USBHS_EPSETUPSR;
    unsigned long USBHS_EPPRIME;
    unsigned long USBHS_EPFLUSH;
    unsigned long USBHS_EPSR;
    unsigned long USBHS_EPCOMPLETE;
    unsigned long USBHS_EPCR0;
    unsigned long USBHS_EPCR1;
    unsigned long USBHS_EPCR2;
    unsigned long USBHS_EPCR3;
    unsigned long USBHS_EPCR4;
    unsigned long USBHS_EPCR5;
    unsigned long USBHS_EPCR6;
    unsigned long USBHS_EPCR7;
    unsigned long USBHS_EPCR8;
    unsigned long USBHS_EPCR9;
    unsigned long USBHS_EPCR10;
    unsigned long USBHS_EPCR11;
    unsigned long USBHS_EPCR12;
    unsigned long USBHS_EPCR13;
    unsigned long USBHS_EPCR14;
    unsigned long USBHS_EPCR15;
    unsigned long USBHS_USBGENCTRL;
} iMX_USBHS;

typedef struct st_iMX_USBPHY
{
    unsigned long USBPHY_PWD;
    unsigned long USBPHY_PWD_SET;
    unsigned long USBPHY_PWD_CLR;
    unsigned long USBPHY_PWD_TOG;
    unsigned long USBPHY_TX;
    unsigned long USBPHY_TX_SET;
    unsigned long USBPHY_TX_CLR;
    unsigned long USBPHY_TX_TOG;
    unsigned long USBPHY_RX;
    unsigned long USBPHY_RX_SET;
    unsigned long USBPHY_RX_CLR;
    unsigned long USBPHY_RX_TOG;
    unsigned long USBPHY_CTRL;
    unsigned long USBPHY_CTRL_SET;
    unsigned long USBPHY_CTRL_CLR;
    unsigned long USBPHY_CTRL_TOG;
    unsigned long USBPHY_STATUS;
    unsigned long ulRes0[3];
    unsigned long USBPHY_DEBUG;
    unsigned long USBPHY_DEBUG_SET;
    unsigned long USBPHY_DEBUG_CLR;
    unsigned long USBPHY_DEBUG_TOG;
    unsigned long USBPHY_DEBUG0_STATUS;
    unsigned long ulRes1[3];
    unsigned long USBPHY_DEBUG1;
    unsigned long USBPHY_DEBUG1_SET;
    unsigned long USBPHY_DEBUG1_CLR;
    unsigned long USBPHY_DEBUG1_TOG;
    unsigned long USBPHY_VERSION;
} iMX_USBPHY;

typedef struct st_iMX_USBNC
{
    unsigned long USBNC_USB_OTG1_CTRL;
#if HSUSB_CONTROLLERS > 1
    unsigned long USBNC_USB_OTG2_CTRL;
    unsigned long ulRes0[4];
#else
    unsigned long ulRes0[5];
#endif
    unsigned long USBNC_USB_OTG1_PHY_CTRL_0;
#if HSUSB_CONTROLLERS > 1
    unsigned long USBNC_USB_OTG2_PHY_CTRL_0;
#endif
} iMX_USBNC;

#if defined TRUE_RANDOM_NUMBER_GENERATOR
typedef struct stiMX_TRNG
{
    unsigned long TRNG0_MCTL;
    unsigned long TRNG0_SCMISC;
    unsigned long TRNG0_PKRRNG;
    unsigned long TRNG0_PKRMAX_PKRSQ;
    unsigned long TRNG0_SDCTL;
    unsigned long TRNG0_SBLIM_TOTSAM;
    unsigned long TRNG0_FRQMIN;
    unsigned long TRNG0_FRQMAX_FRQCNT;
    unsigned long TRNG0_SCML_SCMC;
    unsigned long TRNG0_SCR1L_SCR1C;
    unsigned long TRNG0_SCR2L;
    unsigned long TRNG0_SCR3L;
    unsigned long TRNG0_SCR4L;
    unsigned long TRNG0_SCR5L;
    unsigned long TRNG0_SCR6L;
    unsigned long TRNG0_STATUS;
    unsigned long TRNG0_ENT0;
    unsigned long TRNG0_ENT1;
    unsigned long TRNG0_ENT2;
    unsigned long TRNG0_ENT3;
    unsigned long TRNG0_ENT4;
    unsigned long TRNG0_ENT5;
    unsigned long TRNG0_ENT6;
    unsigned long TRNG0_ENT7;
    unsigned long TRNG0_ENT8;
    unsigned long TRNG0_ENT9;
    unsigned long TRNG0_ENT10;
    unsigned long TRNG0_ENT11;
    unsigned long TRNG0_ENT12;
    unsigned long TRNG0_ENT13;
    unsigned long TRNG0_ENT14;
    unsigned long TRNG0_ENT15;
    unsigned long TRNG0_PKRCNT10;
    unsigned long TRNG0_PKRCNT32;
    unsigned long TRNG0_PKRCNT54;
    unsigned long TRNG0_PKRCNT76;
    unsigned long TRNG0_PKRCNT98;
    unsigned long TRNG0_PKRCNTBA;
    unsigned long TRNG0_PKRCNTDC;
    unsigned long TRNG0_PKRCNTFE;
    unsigned long TRNG0_SEC_CFG;
    unsigned long TRNG0_INT_CTRL;
    unsigned long TRNG0_INT_MASK;
    unsigned long TRNG0_INT_STATUS;
    unsigned long ulReso[0x10];
    unsigned long TRNG0_VID1;
    unsigned long TRNG0_VID2;
} iMX_TRNG;
#endif

typedef struct stiMX_eDMA
    {
    unsigned long DMA_CR;
    unsigned long DMA_ES;
    unsigned long ulRes0;
    unsigned long DMA_ERQ;
    unsigned long ulRes1;
    unsigned long DMA_EEI;
    unsigned char DMA_CEEI;
    unsigned char DMA_SEEI;
    unsigned char DMA_CERQ;
    unsigned char DMA_SERQ;
    unsigned char DMA_CDNE;
    unsigned char DMA_SSRT;
    unsigned char DMA_CERR;
    unsigned char DMA_CINT;
    unsigned long ulRes2;
    unsigned long DMA_INT;
    unsigned long ulRes3;
    unsigned long DMA_ERR;
    unsigned long ulRes4;
    unsigned long DMA_HRS;
    unsigned long ulRes5[3];
    unsigned long DMA_EARS;
    unsigned long ulRes6[46];
    unsigned char DMA_DCHPRI3;
    unsigned char DMA_DCHPRI2;
    unsigned char DMA_DCHPRI1;
    unsigned char DMA_DCHPRI0;
    unsigned char DMA_DCHPRI7;
    unsigned char DMA_DCHPRI6;
    unsigned char DMA_DCHPRI5;
    unsigned char DMA_DCHPRI4;
    unsigned char DMA_DCHPRI11;
    unsigned char DMA_DCHPRI10;
    unsigned char DMA_DCHPRI9;
    unsigned char DMA_DCHPRI8;
    unsigned char DMA_DCHPRI15;
    unsigned char DMA_DCHPRI14;
    unsigned char DMA_DCHPRI13;
    unsigned char DMA_DCHPRI12;
} iMX_eDMA;


#define _PORT1                  0
#define _PORT2                  1
#define _PORT3                  2
#define _PORT4                  3
#define _PORT5                  4


typedef struct stiMX_CORTEX_M7
{
    unsigned long ulRes1;
    const unsigned long INT_CONT_TYPE;
    unsigned long CORTEX_ACTLR;
    unsigned long ulRes2;
    unsigned long SYSTICK_CSR;
    unsigned long SYSTICK_RELOAD;
    volatile unsigned long SYSTICK_CURRENT;
    const unsigned long SYSTICK_CALIB;
    unsigned long ulRes3[56];
    unsigned long IRQ0_31_SER;   
    unsigned long IRQ32_63_SER;  
    unsigned long IRQ64_95_SER;  
    unsigned long IRQ96_127_SER; 
    unsigned long IRQ128_159_SER;
    unsigned long IRQ160_191_SER;
    unsigned long IRQ192_223_SER;
    unsigned long IRQ224_239_SER;
    unsigned long ulRes4[24];
    unsigned long IRQ0_31_CER;
    unsigned long IRQ32_63_CER;  
    unsigned long IRQ64_95_CER;
    unsigned long IRQ96_127_CER;
    unsigned long IRQ128_159_CER;
    unsigned long IRQ160_191_CER;
    unsigned long IRQ192_223_CER;
    unsigned long IRQ224_239_CER;
    unsigned long ulRes5[24];
    unsigned long IRQ0_31_SPR;
    unsigned long IRQ32_63_SPR;
    unsigned long IRQ64_95_SPR;
    unsigned long IRQ96_127_SPR;
    unsigned long IRQ128_159_SPR;
    unsigned long IRQ160_191_SPR;
    unsigned long IRQ192_223_SPR;
    unsigned long IRQ224_239_SPR;
    unsigned long ulRes6[24];
    unsigned long IRQ0_31_CPR;
    unsigned long IRQ32_63_CPR;
    unsigned long IRQ64_95_CPR;
    unsigned long IRQ96_127_CPR;
    unsigned long IRQ128_159_CPR;
    unsigned long IRQ160_191_CPR;
    unsigned long IRQ192_223_CPR;
    unsigned long IRQ224_239_CPR;
    unsigned long ulRes7[24];
    unsigned long IRQ0_31_ABR;
    unsigned long IRQ32_63_ABR;
    unsigned long IRQ64_95_ABR;
    unsigned long IRQ96_127_ABR;
    unsigned long IRQ128_159_ABR;
    unsigned long IRQ160_191_ABR;
    unsigned long IRQ192_223_ABR;
    unsigned long IRQ224_239_ABR;
    unsigned long ulRes8[56];
    unsigned long IRQ0_3_PRIORITY_REGISTER;
    unsigned long IRQ4_7_PRIORITY_REGISTER;
    unsigned long IRQ8_11_PRIORITY_REGISTER;
    unsigned long IRQ12_15_PRIORITY_REGISTER;
    unsigned long IRQ16_19_PRIORITY_REGISTER;
    unsigned long IRQ20_23_PRIORITY_REGISTER;
    unsigned long IRQ24_27_PRIORITY_REGISTER;
    unsigned long IRQ28_31_PRIORITY_REGISTER;
    unsigned long IRQ32_35_PRIORITY_REGISTER;
    unsigned long IRQ36_39_PRIORITY_REGISTER;
    unsigned long IRQ40_43_PRIORITY_REGISTER;
    unsigned long IRQ44_47_PRIORITY_REGISTER;
    unsigned long IRQ48_51_PRIORITY_REGISTER;
    unsigned long IRQ52_55_PRIORITY_REGISTER;
    unsigned long IRQ56_59_PRIORITY_REGISTER;
    unsigned long IRQ60_63_PRIORITY_REGISTER;
    unsigned long IRQ64_67_PRIORITY_REGISTER;
    unsigned long IRQ68_71_PRIORITY_REGISTER;
    unsigned long IRQ72_75_PRIORITY_REGISTER;
    unsigned long IRQ76_79_PRIORITY_REGISTER;
    unsigned long IRQ80_83_PRIORITY_REGISTER;
    unsigned long IRQ84_87_PRIORITY_REGISTER;
    unsigned long IRQ88_91_PRIORITY_REGISTER;
    unsigned long IRQ92_95_PRIORITY_REGISTER;
    unsigned long IRQ96_99_PRIORITY_REGISTER;
    unsigned long IRQ100_103_PRIORITY_REGISTER;
    unsigned long IRQ104_107_PRIORITY_REGISTER;
    unsigned long IRQ108_111_PRIORITY_REGISTER;
    unsigned long IRQ112_115_PRIORITY_REGISTER;
    unsigned long IRQ116_119_PRIORITY_REGISTER;
    unsigned long IRQ120_123_PRIORITY_REGISTER;
    unsigned long IRQ124_127_PRIORITY_REGISTER;
    unsigned long IRQ128_131_PRIORITY_REGISTER;
    unsigned long IRQ132_135_PRIORITY_REGISTER;
    unsigned long IRQ136_139_PRIORITY_REGISTER;
    unsigned long IRQ140_143_PRIORITY_REGISTER;
    unsigned long IRQ144_147_PRIORITY_REGISTER;
    unsigned long IRQ148_151_PRIORITY_REGISTER;
    unsigned long IRQ152_155_PRIORITY_REGISTER;
    unsigned long IRQ156_159_PRIORITY_REGISTER;
    unsigned long IRQ160_163_PRIORITY_REGISTER;
    unsigned long IRQ164_167_PRIORITY_REGISTER;
    unsigned long IRQ168_171_PRIORITY_REGISTER;
    unsigned long IRQ172_175_PRIORITY_REGISTER;
    unsigned long IRQ176_179_PRIORITY_REGISTER;
    unsigned long IRQ180_183_PRIORITY_REGISTER;
    unsigned long IRQ184_187_PRIORITY_REGISTER;
    unsigned long IRQ188_191_PRIORITY_REGISTER;
    unsigned long IRQ192_195_PRIORITY_REGISTER;
    unsigned long IRQ196_199_PRIORITY_REGISTER;
    unsigned long IRQ200_203_PRIORITY_REGISTER;
    unsigned long IRQ204_207_PRIORITY_REGISTER;
    unsigned long IRQ208_211_PRIORITY_REGISTER;
    unsigned long IRQ212_215_PRIORITY_REGISTER;
    unsigned long IRQ216_219_PRIORITY_REGISTER;
    unsigned long IRQ220_223_PRIORITY_REGISTER;
    unsigned long IRQ224_227_PRIORITY_REGISTER;
    unsigned long IRQ228_231_PRIORITY_REGISTER;
    unsigned long IRQ232_235_PRIORITY_REGISTER;
    unsigned long IRQ236_239_PRIORITY_REGISTER;
    unsigned long ulRes9[516];
    unsigned long CPUID_BASE_REGISTER;
    unsigned long INT_CONT_STATE_REG;
    unsigned long VECTOR_TABLE_OFFSET_REG;
    unsigned long APPLICATION_INT_RESET_CTR_REG;
    unsigned long SYSTEM_CONTROL_REGISTER;
    unsigned long CONFIGURATION_CONTROL_REGISTER;
    unsigned long SYSTEM_HANDLER_4_7_PRIORITY_REGISTER;
    unsigned long SYSTEM_HANDLER_8_11_PRIORITY_REGISTER;
    unsigned long SYSTEM_HANDLER_12_15_PRIORITY_REGISTER;
    unsigned long SHCSR;
    unsigned long ulRes10[22];
    unsigned long CSSIDR;
    unsigned long CSSELR;
    unsigned long CPACR;
    unsigned long ulRes10a;
    unsigned long MPU_TYPE;
    unsigned long MPU_CTRL;
    unsigned long MPU_RNR;
    unsigned long MPU_RBAR;
    unsigned long MPU_RASR;
    unsigned long MPU_RBAR_A1;
    unsigned long MPU_RASR_A1;
    unsigned long MPU_RBAR_A2;
    unsigned long MPU_RASR_A2;
    unsigned long MPU_RBAR_A3;
    unsigned long MPU_RASR_A3;
    unsigned long ulRes11[94];
    unsigned long FPCCR;
    unsigned long ulRes12[6];
    unsigned long ICIALLU;
    unsigned long ulRes13[3];
    unsigned long DCISW;
} iMX_CORTEX_M7;

#if defined ARM_MATH_CM4 || defined ARM_MATH_CM7
typedef struct stiMX_CORTEX_M7_DEBUG
{
    unsigned long DHCSR;
    unsigned long DCRSR;
    unsigned long DCRDR;
    unsigned long DEMCR;
} iMX_CORTEX_M7_DEBUG;

typedef struct stiMX_CORTEX_M7_TRACE
{
    unsigned long DWT_CTRL;
    unsigned long DWT_CYCCNT;
    unsigned long DWT_CPICNT;
    unsigned long DWT_EXCCNT;
    unsigned long DWT_SLEEPVNT;
    unsigned long DWT_LSUCNT;
    unsigned long DWT_FOLDCNT;
    unsigned long DWT_PCSR;
    unsigned long DWT_COMP0;
    unsigned long DWT_MASK0;
    unsigned long DWT_FUNCTION0;
    unsigned long ulRes0;
    unsigned long DWT_COMP1;
    unsigned long DWT_MASK1;
    unsigned long DWT_FUNCTION1;
    unsigned long ulRes1;
    unsigned long DWT_COMP2;
    unsigned long DWT_MASK2;
    unsigned long DWT_FUNCTION2;
    unsigned long ulRes2;
    unsigned long DWT_COMP3;
    unsigned long DWT_MASK3;
    unsigned long DWT_FUNCTION3;
    #if defined ARM_MATH_CM7
    unsigned long ulRes3[981];
    unsigned long DWT_LAR;
    unsigned long DWT_LSR;
    unsigned long ulRes4[6];
    #else
    unsigned long ulRes3[989];
    #endif
    unsigned long DWT_PID4;
    unsigned long DWT_PID5;
    unsigned long DWT_PID6;
    unsigned long DWT_PID7;
    unsigned long DWT_PID0;
    unsigned long DWT_PID1;
    unsigned long DWT_PID2;
    unsigned long DWT_PID3;
    unsigned long DWT_CID0;
    unsigned long DWT_CID1;
    unsigned long DWT_CID2;
    unsigned long DWT_CID3;
} iMX_CORTEX_M7_TRACE;
#endif

typedef struct stiMX_CORTEX_M7_REGS
{
    unsigned long ulR0;                                                  // low registers
    unsigned long ulR1;
    unsigned long ulR2;
    unsigned long ulR3;
    unsigned long ulR4;
    unsigned long ulR5;
    unsigned long ulR6;
    unsigned long ulR7;
    unsigned long ulR8;                                                  // high registers
    unsigned long ulR9;
    unsigned long ulR10;
    unsigned long ulR11;
    unsigned long ulR12;
    unsigned long ulR13_SP;                                              // stack pointer
    unsigned long ulPSP;                                                 // banked versions of stack pointer
    unsigned long ulMSP;
    unsigned long ulR14_LR;                                              // link register
    unsigned long ulR15_PC;                                              // program counter
    // Special registers
    //
    unsigned long ulPSR;                                                 // program status register
    unsigned long ulPRIMASK;
    unsigned long ulFAULTMASK;
    unsigned long ulBASEPRI;
    unsigned long ulCONTROL;                                             // control
} iMX_CORTEX_M7_REGS;

#define INTERRUPT_MASKED 0x00000001



typedef struct stiMX_eDMADES
{
unsigned long  DMA_TCD0_SADDR;
  signed short DMA_TCD0_SOFF;
unsigned short DMA_TCD0_ATTR;
unsigned long  DMA_TCD0_NBYTES_MLNO;
//unsigned long  DMA_TCD0_NBYTES_MLOFFNO;
//unsigned long  DMA_TCD0_NBYTES_MLOFFYES;
unsigned long  DMA_TCD0_SLAST;
unsigned long  DMA_TCD0_DADDR;
  signed short DMA_TCD0_DOFF;
unsigned short DMA_TCD0_CITER_ELINKYES;
//unsigned short DMA_TCD0_CITER_ELINKNO;
unsigned long  DMA_TCD0_DLASTSGA;
unsigned short DMA_TCD0_CSR;
unsigned short DMA_TCD0_BITER_ELINKYES;
//unsigned short DMA_TCD0_BITER_ELINKNO;
unsigned long  DMA_TCD1_SADDR;
  signed short DMA_TCD1_SOFF;
unsigned short DMA_TCD1_ATTR;
unsigned long  DMA_TCD1_NBYTES_MLNO;
//unsigned long  DMA_TCD1_NBYTES_MLOFFNO;
//unsigned long  DMA_TCD1_NBYTES_MLOFFYES;
unsigned long  DMA_TCD1_SLAST;
unsigned long  DMA_TCD1_DADDR;
  signed short DMA_TCD1_DOFF;
unsigned short DMA_TCD1_CITER_ELINKYES;
//unsigned short DMA_TCD1_CITER_ELINKNO;
unsigned long  DMA_TCD1_DLASTSGA;
unsigned short DMA_TCD1_CSR;
unsigned short DMA_TCD1_BITER_ELINKYES;
//unsigned short DMA_TCD1_BITER_ELINKNO;
unsigned long  DMA_TCD2_SADDR;
  signed short DMA_TCD2_SOFF;
unsigned short DMA_TCD2_ATTR;
unsigned long  DMA_TCD2_NBYTES_MLNO;
//unsigned long  DMA_TCD2_NBYTES_MLOFFNO;
//unsigned long  DMA_TCD2_NBYTES_MLOFFYES;
unsigned long  DMA_TCD2_SLAST;
unsigned long  DMA_TCD2_DADDR;
  signed short DMA_TCD2_DOFF;
unsigned short DMA_TCD2_CITER_ELINKYES;
//unsigned short DMA_TCD2_CITER_ELINKNO;
unsigned long  DMA_TCD2_DLASTSGA;
unsigned short DMA_TCD2_CSR;
unsigned short DMA_TCD2_BITER_ELINKYES;
//unsigned short DMA_TCD2_BITER_ELINKNO;
unsigned long  DMA_TCD3_SADDR;
  signed short DMA_TCD3_SOFF;
unsigned short DMA_TCD3_ATTR;
unsigned long  DMA_TCD3_NBYTES_MLNO;
//unsigned long  DMA_TCD3_NBYTES_MLOFFNO;
//unsigned long  DMA_TCD3_NBYTES_MLOFFYES;
unsigned long  DMA_TCD3_SLAST;
unsigned long  DMA_TCD3_DADDR;
  signed short DMA_TCD3_DOFF;
unsigned short DMA_TCD3_CITER_ELINKYES;
//unsigned short DMA_TCD3_CITER_ELINKNO;
unsigned long  DMA_TCD3_DLASTSGA;
unsigned short DMA_TCD3_CSR;
unsigned short DMA_TCD3_BITER_ELINKYES;
//unsigned short DMA_TCD3_BITER_ELINKNO;
#if defined irq_DMA4_ID
    unsigned long  DMA_TCD4_SADDR;
      signed short DMA_TCD4_SOFF;
    unsigned short DMA_TCD4_ATTR;
    unsigned long  DMA_TCD4_NBYTES_MLNO;
    //unsigned long  DMA_TCD4_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD4_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD4_SLAST;
    unsigned long  DMA_TCD4_DADDR;
      signed short DMA_TCD4_DOFF;
    unsigned short DMA_TCD4_CITER_ELINKYES;
    //unsigned short DMA_TCD4_CITER_ELINKNO;
    unsigned long  DMA_TCD4_DLASTSGA;
    unsigned short DMA_TCD4_CSR;
    unsigned short DMA_TCD4_BITER_ELINKYES;
    //unsigned short DMA_TCD4_BITER_ELINKNO;
    unsigned long  DMA_TCD5_SADDR;
      signed short DMA_TCD5_SOFF;
    unsigned short DMA_TCD5_ATTR;
    unsigned long  DMA_TCD5_NBYTES_MLNO;
    //unsigned long  DMA_TCD5_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD5_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD5_SLAST;
    unsigned long  DMA_TCD5_DADDR;
      signed short DMA_TCD5_DOFF;
    unsigned short DMA_TCD5_CITER_ELINKYES;
    //unsigned short DMA_TCD5_CITER_ELINKNO;
    unsigned long  DMA_TCD5_DLASTSGA;
    unsigned short DMA_TCD5_CSR;
    unsigned short DMA_TCD5_BITER_ELINKYES;
    //unsigned short DMA_TCD5_BITER_ELINKNO;
    unsigned long  DMA_TCD6_SADDR;
      signed short DMA_TCD6_SOFF;
    unsigned short DMA_TCD6_ATTR;
    unsigned long  DMA_TCD6_NBYTES_MLNO;
    //unsigned long  DMA_TCD6_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD6_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD6_SLAST;
    unsigned long  DMA_TCD6_DADDR;
      signed short DMA_TCD6_DOFF;
    unsigned short DMA_TCD6_CITER_ELINKYES;
    //unsigned short DMA_TCD6_CITER_ELINKNO;
    unsigned long  DMA_TCD6_DLASTSGA;
    unsigned short DMA_TCD6_CSR;
    unsigned short DMA_TCD6_BITER_ELINKYES;
    //unsigned short DMA_TCD6_BITER_ELINKNO;
    unsigned long  DMA_TCD7_SADDR;
      signed short DMA_TCD7_SOFF;
    unsigned short DMA_TCD7_ATTR;
    unsigned long  DMA_TCD7_NBYTES_MLNO;
    //unsigned long  DMA_TCD7_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD7_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD7_SLAST;
    unsigned long  DMA_TCD7_DADDR;
      signed short DMA_TCD7_DOFF;
    unsigned short DMA_TCD7_CITER_ELINKYES;
    //unsigned short DMA_TCD7_CITER_ELINKNO;
    unsigned long  DMA_TCD7_DLASTSGA;
    unsigned short DMA_TCD7_CSR;
    unsigned short DMA_TCD7_BITER_ELINKYES;
    //unsigned short DMA_TCD7_BITER_ELINKNO;
    unsigned long  DMA_TCD8_SADDR;
      signed short DMA_TCD8_SOFF;
    unsigned short DMA_TCD8_ATTR;
    unsigned long  DMA_TCD8_NBYTES_MLNO;
    //unsigned long  DMA_TCD8_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD8_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD8_SLAST;
    unsigned long  DMA_TCD8_DADDR;
      signed short DMA_TCD8_DOFF;
    unsigned short DMA_TCD8_CITER_ELINKYES;
    //unsigned short DMA_TCD8_CITER_ELINKNO;
    unsigned long  DMA_TCD8_DLASTSGA;
    unsigned short DMA_TCD8_CSR;
    unsigned short DMA_TCD8_BITER_ELINKYES;
    //unsigned short DMA_TCD8_BITER_ELINKNO;
    unsigned long  DMA_TCD9_SADDR;
      signed short DMA_TCD9_SOFF;
    unsigned short DMA_TCD9_ATTR;
    unsigned long  DMA_TCD9_NBYTES_MLNO;
    //unsigned long  DMA_TCD9_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD9_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD9_SLAST;
    unsigned long  DMA_TCD9_DADDR;
      signed short DMA_TCD9_DOFF;
    unsigned short DMA_TCD9_CITER_ELINKYES;
    //unsigned short DMA_TCD9_CITER_ELINKNO;
    unsigned long  DMA_TCD9_DLASTSGA;
    unsigned short DMA_TCD9_CSR;
    unsigned short DMA_TCD9_BITER_ELINKYES;
    //unsigned short DMA_TCD9_BITER_ELINKNO;
    unsigned long  DMA_TCD10_SADDR;
      signed short DMA_TCD10_SOFF;
    unsigned short DMA_TCD10_ATTR;
    unsigned long  DMA_TCD10_NBYTES_MLNO;
    //unsigned long  DMA_TCD10_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD10_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD10_SLAST;
    unsigned long  DMA_TCD10_DADDR;
      signed short DMA_TCD10_DOFF;
    unsigned short DMA_TCD10_CITER_ELINKYES;
    //unsigned short DMA_TCD10_CITER_ELINKNO;
    unsigned long  DMA_TCD10_DLASTSGA;
    unsigned short DMA_TCD10_CSR;
    unsigned short DMA_TCD10_BITER_ELINKYES;
    //unsigned short DMA_TCD10_BITER_ELINKNO;
    unsigned long  DMA_TCD11_SADDR;
      signed short DMA_TCD11_SOFF;
    unsigned short DMA_TCD11_ATTR;
    unsigned long  DMA_TCD11_NBYTES_MLNO;
    //unsigned long  DMA_TCD11_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD11_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD11_SLAST;
    unsigned long  DMA_TCD11_DADDR;
      signed short DMA_TCD11_DOFF;
    unsigned short DMA_TCD11_CITER_ELINKYES;
    //unsigned short DMA_TCD11_CITER_ELINKNO;
    unsigned long  DMA_TCD11_DLASTSGA;
    unsigned short DMA_TCD11_CSR;
    unsigned short DMA_TCD11_BITER_ELINKYES;
    //unsigned short DMA_TCD11_BITER_ELINKNO;
    unsigned long  DMA_TCD12_SADDR;
      signed short DMA_TCD12_SOFF;
    unsigned short DMA_TCD12_ATTR;
    unsigned long  DMA_TCD12_NBYTES_MLNO;
    //unsigned long  DMA_TCD12_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD12_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD12_SLAST;
    unsigned long  DMA_TCD12_DADDR;
      signed short DMA_TCD12_DOFF;
    unsigned short DMA_TCD12_CITER_ELINKYES;
    //unsigned short DMA_TCD12_CITER_ELINKNO;
    unsigned long  DMA_TCD12_DLASTSGA;
    unsigned short DMA_TCD12_CSR;
    unsigned short DMA_TCD12_BITER_ELINKYES;
    //unsigned short DMA_TCD12_BITER_ELINKNO;
    unsigned long  DMA_TCD13_SADDR;
      signed short DMA_TCD13_SOFF;
    unsigned short DMA_TCD13_ATTR;
    unsigned long  DMA_TCD13_NBYTES_MLNO;
    //unsigned long  DMA_TCD13_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD13_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD13_SLAST;
    unsigned long  DMA_TCD13_DADDR;
      signed short DMA_TCD13_DOFF;
    unsigned short DMA_TCD13_CITER_ELINKYES;
    //unsigned short DMA_TCD13_CITER_ELINKNO;
    unsigned long  DMA_TCD13_DLASTSGA;
    unsigned short DMA_TCD13_CSR;
    unsigned short DMA_TCD13_BITER_ELINKYES;
    //unsigned short DMA_TCD13_BITER_ELINKNO;
    unsigned long  DMA_TCD14_SADDR;
      signed short DMA_TCD14_SOFF;
    unsigned short DMA_TCD14_ATTR;
    unsigned long  DMA_TCD14_NBYTES_MLNO;
    //unsigned long  DMA_TCD14_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD14_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD14_SLAST;
    unsigned long  DMA_TCD14_DADDR;
      signed short DMA_TCD14_DOFF;
    unsigned short DMA_TCD14_CITER_ELINKYES;
    //unsigned short DMA_TCD14_CITER_ELINKNO;
    unsigned long  DMA_TCD14_DLASTSGA;
    unsigned short DMA_TCD14_CSR;
    unsigned short DMA_TCD14_BITER_ELINKYES;
    //unsigned short DMA_TCD14_BITER_ELINKNO;
    unsigned long  DMA_TCD15_SADDR;
      signed short DMA_TCD15_SOFF;
    unsigned short DMA_TCD15_ATTR;
    unsigned long  DMA_TCD15_NBYTES_MLNO;
    //unsigned long  DMA_TCD15_NBYTES_MLOFFNO;
    //unsigned long  DMA_TCD15_NBYTES_MLOFFYES;
    unsigned long  DMA_TCD15_SLAST;
    unsigned long  DMA_TCD15_DADDR;
      signed short DMA_TCD15_DOFF;
    unsigned short DMA_TCD15_CITER_ELINKYES;
    //unsigned short DMA_TCD15_CITER_ELINKNO;
    unsigned long  DMA_TCD15_DLASTSGA;
    unsigned short DMA_TCD15_CSR;
    unsigned short DMA_TCD15_BITER_ELINKYES;
    //unsigned short DMA_TCD15_BITER_ELINKNO;
    #if defined KINETIS_K_FPU && !defined KINETIS_K21 && !defined KINETIS_K22 && !defined KINETIS_K64
        unsigned long  DMA_TCD16_SADDR;
          signed short DMA_TCD16_SOFF;
        unsigned short DMA_TCD16_ATTR;
        unsigned long  DMA_TCD16_NBYTES_MLNO;
        //unsigned long  DMA_TCD16_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD16_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD16_SLAST;
        unsigned long  DMA_TCD16_DADDR;
          signed short DMA_TCD16_DOFF;
        unsigned short DMA_TCD16_CITER_ELINKYES;
        //unsigned short DMA_TCD0_CITER_ELINKNO;
        unsigned long  DMA_TCD16_DLASTSGA;
        unsigned short DMA_TCD16_CSR;
        unsigned short DMA_TCD16_BITER_ELINKYES;
        //unsigned short DMA_TCD16_BITER_ELINKNO;
        unsigned long  DMA_TCD17_SADDR;
          signed short DMA_TCD17_SOFF;
        unsigned short DMA_TCD17_ATTR;
        unsigned long  DMA_TCD17_NBYTES_MLNO;
        //unsigned long  DMA_TCD17_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD17_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD17_SLAST;
        unsigned long  DMA_TCD17_DADDR;
          signed short DMA_TCD17_DOFF;
        unsigned short DMA_TCD17_CITER_ELINKYES;
        //unsigned short DMA_TCD17_CITER_ELINKNO;
        unsigned long  DMA_TCD17_DLASTSGA;
        unsigned short DMA_TCD17_CSR;
        unsigned short DMA_TCD17_BITER_ELINKYES;
        //unsigned short DMA_TCD17_BITER_ELINKNO;
        unsigned long  DMA_TCD18_SADDR;
          signed short DMA_TCD18_SOFF;
        unsigned short DMA_TCD18_ATTR;
        unsigned long  DMA_TCD18_NBYTES_MLNO;
        //unsigned long  DMA_TCD18_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD18_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD18_SLAST;
        unsigned long  DMA_TCD18_DADDR;
          signed short DMA_TCD18_DOFF;
        unsigned short DMA_TCD18_CITER_ELINKYES;
        //unsigned short DMA_TCD18_CITER_ELINKNO;
        unsigned long  DMA_TCD18_DLASTSGA;
        unsigned short DMA_TCD18_CSR;
        unsigned short DMA_TCD18_BITER_ELINKYES;
        //unsigned short DMA_TCD18_BITER_ELINKNO;
        unsigned long  DMA_TCD19_SADDR;
          signed short DMA_TCD19_SOFF;
        unsigned short DMA_TCD19_ATTR;
        unsigned long  DMA_TCD19_NBYTES_MLNO;
        //unsigned long  DMA_TCD19_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD19_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD19_SLAST;
        unsigned long  DMA_TCD19_DADDR;
          signed short DMA_TCD19_DOFF;
        unsigned short DMA_TCD19_CITER_ELINKYES;
        //unsigned short DMA_TCD19_CITER_ELINKNO;
        unsigned long  DMA_TCD19_DLASTSGA;
        unsigned short DMA_TCD19_CSR;
        unsigned short DMA_TCD19_BITER_ELINKYES;
        //unsigned short DMA_TCD19_BITER_ELINKNO;
        unsigned long  DMA_TCD20_SADDR;
          signed short DMA_TCD20_SOFF;
        unsigned short DMA_TCD20_ATTR;
        unsigned long  DMA_TCD20_NBYTES_MLNO;
        //unsigned long  DMA_TCD20_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD20_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD20_SLAST;
        unsigned long  DMA_TCD20_DADDR;
          signed short DMA_TCD20_DOFF;
        unsigned short DMA_TCD20_CITER_ELINKYES;
        //unsigned short DMA_TCD20_CITER_ELINKNO;
        unsigned long  DMA_TCD20_DLASTSGA;
        unsigned short DMA_TCD20_CSR;
        unsigned short DMA_TCD20_BITER_ELINKYES;
        //unsigned short DMA_TCD20_BITER_ELINKNO;
        unsigned long  DMA_TCD21_SADDR;
          signed short DMA_TCD21_SOFF;
        unsigned short DMA_TCD21_ATTR;
        unsigned long  DMA_TCD21_NBYTES_MLNO;
        //unsigned long  DMA_TCD21_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD21_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD21_SLAST;
        unsigned long  DMA_TCD21_DADDR;
          signed short DMA_TCD21_DOFF;
        unsigned short DMA_TCD21_CITER_ELINKYES;
        //unsigned short DMA_TCD21_CITER_ELINKNO;
        unsigned long  DMA_TCD21_DLASTSGA;
        unsigned short DMA_TCD21_CSR;
        unsigned short DMA_TCD21_BITER_ELINKYES;
        //unsigned short DMA_TCD21_BITER_ELINKNO;
        unsigned long  DMA_TCD22_SADDR;
          signed short DMA_TCD22_SOFF;
        unsigned short DMA_TCD22_ATTR;
        unsigned long  DMA_TCD22_NBYTES_MLNO;
        //unsigned long  DMA_TCD22_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD22_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD22_SLAST;
        unsigned long  DMA_TCD22_DADDR;
          signed short DMA_TCD22_DOFF;
        unsigned short DMA_TCD22_CITER_ELINKYES;
        //unsigned short DMA_TCD22_CITER_ELINKNO;
        unsigned long  DMA_TCD22_DLASTSGA;
        unsigned short DMA_TCD22_CSR;
        unsigned short DMA_TCD22_BITER_ELINKYES;
        //unsigned short DMA_TCD22_BITER_ELINKNO;
        unsigned long  DMA_TCD23_SADDR;
          signed short DMA_TCD23_SOFF;
        unsigned short DMA_TCD23_ATTR;
        unsigned long  DMA_TCD23_NBYTES_MLNO;
        //unsigned long  DMA_TCD23_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD23_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD23_SLAST;
        unsigned long  DMA_TCD23_DADDR;
          signed short DMA_TCD23_DOFF;
        unsigned short DMA_TCD23_CITER_ELINKYES;
        //unsigned short DMA_TCD23_CITER_ELINKNO;
        unsigned long  DMA_TCD23_DLASTSGA;
        unsigned short DMA_TCD23_CSR;
        unsigned short DMA_TCD23_BITER_ELINKYES;
        //unsigned short DMA_TCD23_BITER_ELINKNO;
        unsigned long  DMA_TCD24_SADDR;
          signed short DMA_TCD24_SOFF;
        unsigned short DMA_TCD24_ATTR;
        unsigned long  DMA_TCD24_NBYTES_MLNO;
        //unsigned long  DMA_TCD24_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD24_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD24_SLAST;
        unsigned long  DMA_TCD24_DADDR;
          signed short DMA_TCD24_DOFF;
        unsigned short DMA_TCD24_CITER_ELINKYES;
        //unsigned short DMA_TCD24_CITER_ELINKNO;
        unsigned long  DMA_TCD24_DLASTSGA;
        unsigned short DMA_TCD24_CSR;
        unsigned short DMA_TCD24_BITER_ELINKYES;
        //unsigned short DMA_TCD24_BITER_ELINKNO;
        unsigned long  DMA_TCD25_SADDR;
          signed short DMA_TCD25_SOFF;
        unsigned short DMA_TCD25_ATTR;
        unsigned long  DMA_TCD25_NBYTES_MLNO;
        //unsigned long  DMA_TCD25_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD25_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD25_SLAST;
        unsigned long  DMA_TCD25_DADDR;
          signed short DMA_TCD25_DOFF;
        unsigned short DMA_TCD25_CITER_ELINKYES;
        //unsigned short DMA_TCD25_CITER_ELINKNO;
        unsigned long  DMA_TCD25_DLASTSGA;
        unsigned short DMA_TCD25_CSR;
        unsigned short DMA_TCD25_BITER_ELINKYES;
        //unsigned short DMA_TCD25_BITER_ELINKNO;
        unsigned long  DMA_TCD26_SADDR;
          signed short DMA_TCD26_SOFF;
        unsigned short DMA_TCD26_ATTR;
        unsigned long  DMA_TCD26_NBYTES_MLNO;
        //unsigned long  DMA_TCD26_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD26_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD26_SLAST;
        unsigned long  DMA_TCD26_DADDR;
          signed short DMA_TCD26_DOFF;
        unsigned short DMA_TCD26_CITER_ELINKYES;
        //unsigned short DMA_TCD26_CITER_ELINKNO;
        unsigned long  DMA_TCD26_DLASTSGA;
        unsigned short DMA_TCD26_CSR;
        unsigned short DMA_TCD26_BITER_ELINKYES;
        //unsigned short DMA_TCD26_BITER_ELINKNO;
        unsigned long  DMA_TCD27_SADDR;
          signed short DMA_TCD27_SOFF;
        unsigned short DMA_TCD27_ATTR;
        unsigned long  DMA_TCD27_NBYTES_MLNO;
        //unsigned long  DMA_TCD27_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD27_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD27_SLAST;
        unsigned long  DMA_TCD27_DADDR;
          signed short DMA_TCD27_DOFF;
        unsigned short DMA_TCD27_CITER_ELINKYES;
        //unsigned short DMA_TCD27_CITER_ELINKNO;
        unsigned long  DMA_TCD27_DLASTSGA;
        unsigned short DMA_TCD27_CSR;
        unsigned short DMA_TCD27_BITER_ELINKYES;
        //unsigned short DMA_TCD27_BITER_ELINKNO;
        unsigned long  DMA_TCD28_SADDR;
          signed short DMA_TCD28_SOFF;
        unsigned short DMA_TCD28_ATTR;
        unsigned long  DMA_TCD28_NBYTES_MLNO;
        //unsigned long  DMA_TCD28_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD28_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD28_SLAST;
        unsigned long  DMA_TCD28_DADDR;
          signed short DMA_TCD28_DOFF;
        unsigned short DMA_TCD28_CITER_ELINKYES;
        //unsigned short DMA_TCD28_CITER_ELINKNO;
        unsigned long  DMA_TCD28_DLASTSGA;
        unsigned short DMA_TCD28_CSR;
        unsigned short DMA_TCD28_BITER_ELINKYES;
        //unsigned short DMA_TCD28_BITER_ELINKNO;
        unsigned long  DMA_TCD29_SADDR;
          signed short DMA_TCD29_SOFF;
        unsigned short DMA_TCD29_ATTR;
        unsigned long  DMA_TCD29_NBYTES_MLNO;
        //unsigned long  DMA_TCD29_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD29_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD29_SLAST;
        unsigned long  DMA_TCD29_DADDR;
          signed short DMA_TCD29_DOFF;
        unsigned short DMA_TCD29_CITER_ELINKYES;
        //unsigned short DMA_TCD29_CITER_ELINKNO;
        unsigned long  DMA_TCD29_DLASTSGA;
        unsigned short DMA_TCD29_CSR;
        unsigned short DMA_TCD29_BITER_ELINKYES;
        //unsigned short DMA_TCD29_BITER_ELINKNO;
        unsigned long  DMA_TCD30_SADDR;
          signed short DMA_TCD30_SOFF;
        unsigned short DMA_TCD30_ATTR;
        unsigned long  DMA_TCD30_NBYTES_MLNO;
        //unsigned long  DMA_TCD30_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD30_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD30_SLAST;
        unsigned long  DMA_TCD30_DADDR;
          signed short DMA_TCD30_DOFF;
        unsigned short DMA_TCD30_CITER_ELINKYES;
        //unsigned short DMA_TCD30_CITER_ELINKNO;
        unsigned long  DMA_TCD30_DLASTSGA;
        unsigned short DMA_TCD30_CSR;
        unsigned short DMA_TCD30_BITER_ELINKYES;
        //unsigned short DMA_TCD30_BITER_ELINKNO;
        unsigned long  DMA_TCD31_SADDR;
          signed short DMA_TCD31_SOFF;
        unsigned short DMA_TCD31_ATTR;
        unsigned long  DMA_TCD31_NBYTES_MLNO;
        //unsigned long  DMA_TCD31_NBYTES_MLOFFNO;
        //unsigned long  DMA_TCD31_NBYTES_MLOFFYES;
        unsigned long  DMA_TCD31_SLAST;
        unsigned long  DMA_TCD31_DADDR;
          signed short DMA_TCD31_DOFF;
        unsigned short DMA_TCD31_CITER_ELINKYES;
        //unsigned short DMA_TCD31_CITER_ELINKNO;
        unsigned long  DMA_TCD31_DLASTSGA;
        unsigned short DMA_TCD31_CSR;
        unsigned short DMA_TCD31_BITER_ELINKYES;
        //unsigned short DMA_TCD31_BITER_ELINKNO;
    #endif
#endif
} iMX_eDMADES;

typedef struct stKINETIS_FB
{
    unsigned long CSAR0;
    unsigned long CSMR0;
    unsigned long CSCR0;
    unsigned long CSAR1;
    unsigned long CSMR1;
    unsigned long CSCR1;
    unsigned long CSAR2;
    unsigned long CSMR2;
    unsigned long CSCR2;
    unsigned long CSAR3;
    unsigned long CSMR3;
    unsigned long CSCR3;
    unsigned long CSAR4;
    unsigned long CSMR4;
    unsigned long CSCR4;
    unsigned long CSAR5;
    unsigned long CSMR5;
    unsigned long CSCR5;
    unsigned long ulRes0[6];
    unsigned long CSPMCR;
} KINETIS_FB;



typedef struct stiMX_LPSPI
{
    volatile unsigned long LPSPI_VERID;
    volatile unsigned long LPSPI_PARAM;
    unsigned long ulRes0[2];
    volatile unsigned long LPSPI_CR;
    volatile unsigned long LPSPI_SR;
    unsigned long LPSPI_IER;
    unsigned long LPSPI_DER;
    unsigned long LPSPI_CFGR0;
    unsigned long LPSPI_CFGR1;
    unsigned long ulRes1[2];
    unsigned long LPSPI_DMR0;
    unsigned long LPSPI_DMR1;
    unsigned long ulRes2[2];
    unsigned long LPSPI_CCR;
    unsigned long ulRes3[5];
    unsigned long LPSPI_FCR;
    volatile unsigned long LPSPI_FSR;
    unsigned long LPSPI_TCR;
    unsigned long LPSPI_TDR;
    unsigned long ulRes4[2];
    unsigned long LPSPI_RSR;
    unsigned long LPSPI_RDR;
} iMX_LPSPI;



typedef struct stKINETIC_CRC
{
unsigned long CRC_CRC;
unsigned long CRC_GPOLY;
unsigned long CRC_CTRL;
} KINETIC_CRC;


#if !defined DEVICE_WITHOUT_DMA
typedef struct stKINETIS_DMAMUX
{
unsigned char DMAMUX_CHCFG0;
    #if !defined KINETIS_KM
    unsigned long DMAMUX_CHCFG1;
    unsigned longDMAMUX_CHCFG2;
    unsigned longDMAMUX_CHCFG3;
        #if DMA_CHANNEL_COUNT > 4
        unsigned longDMAMUX_CHCFG4;
        unsigned longDMAMUX_CHCFG5;
        unsigned longDMAMUX_CHCFG6;
        unsigned longDMAMUX_CHCFG7;
        #endif
        #if DMA_CHANNEL_COUNT > 8
        unsigned longDMAMUX_CHCFG8;
        unsigned longDMAMUX_CHCFG9;
        unsigned longDMAMUX_CHCFG10;
        unsigned longDMAMUX_CHCFG11;
        unsigned longDMAMUX_CHCFG12;
        unsigned longDMAMUX_CHCFG13;
        unsigned longDMAMUX_CHCFG14;
        unsigned longDMAMUX_CHCFG15;
        #endif
        #if DMA_CHANNEL_COUNT > 16
        unsigned longDMAMUX_CHCFG16;
        unsigned longDMAMUX_CHCFG17;
        unsigned longDMAMUX_CHCFG18;
        unsigned longDMAMUX_CHCFG19;
        unsigned longDMAMUX_CHCFG20;
        unsigned longDMAMUX_CHCFG21;
        unsigned longDMAMUX_CHCFG22;
        unsigned longDMAMUX_CHCFG23;
        unsigned longDMAMUX_CHCFG24;
        unsigned longDMAMUX_CHCFG25;
        unsigned longDMAMUX_CHCFG26;
        unsigned longDMAMUX_CHCFG27;
        unsigned longDMAMUX_CHCFG28;
        unsigned longDMAMUX_CHCFG29;
        unsigned longDMAMUX_CHCFG30;
        unsigned longDMAMUX_CHCFG31;
        #endif
    #endif
} KINETIS_DMAMUX;
#endif

#if I2S_AVAILABLE > 0
typedef struct st_KINETIS_I2S
{
unsigned long I2S_TCSR;
unsigned long ulRes0;
unsigned long I2S_TCR2;
unsigned long I2S_TCR3;
unsigned long I2S_TCR4;
unsigned long I2S_TCR5;
unsigned long ulRes1[2];
volatile unsigned long I2S_TDR0;
unsigned long ulRes2[15];
unsigned long I2S_TMR;
unsigned long ulRes3[7];
unsigned long I2S_RCSR;
unsigned long ulRes4;
unsigned long I2S_RCR2;
unsigned long I2S_RCR3;
unsigned long I2S_RCR4;
unsigned long I2S_RCR5;
unsigned long ulRes5[2];
volatile unsigned long I2S_RDR0;
unsigned long ulRes6[15];
unsigned long I2S_RMR;
unsigned long ulRes7[7];
unsigned long I2S_MCR;
} KINETIS_I2S;
#endif


typedef struct st_iMX_WDOG
{
    unsigned short WDOG_WCR;
    unsigned short WDOG_WSR;
    unsigned short WDOG_WRSR;
    unsigned short WDOG_WICR;
    unsigned short WDOG_WMCR;
} iMX_WDOG;

typedef struct st_iMX_WDOG3
{
    unsigned long WDOG3_CS;
    unsigned long WDOG3_CNT;
    unsigned long WDOG3_TOVAL;
    unsigned long WDOG3_WIN;
} iMX_WDOG3;

#if QUAD_TIMER_AVAILABLE > 0
typedef struct st_iMX_QUAD_TIMER
{
    unsigned short TMR_COMP10;
    unsigned short TMR_COMP20;
    unsigned short TMR_CAPT0;
    unsigned short TMR_LOAD0;
    unsigned short TMR_HOLD0;
    unsigned short TMR_CNTR0;
    unsigned short TMR_CTRL0;
    unsigned short TMR_SCTRL0;
    unsigned short TMR_CMPLD10;
    unsigned short TMR_CMPLD20;
    unsigned short TMR_CSCTRL0;
    unsigned short TMR_FILT0;
    unsigned short TMR_DMA0;
    unsigned short usRes0[2];
    unsigned short TMR_ENBL;
    unsigned short TMR_COMP11;
    unsigned short TMR_COMP21;
    unsigned short TMR_CAPT1;
    unsigned short TMR_LOAD1;
    unsigned short TMR_HOLD1;
    unsigned short TMR_CNTR1;
    unsigned short TMR_CTRL1;
    unsigned short TMR_SCTRL1;
    unsigned short TMR_CMPLD11;
    unsigned short TMR_CMPLD21;
    unsigned short TMR_CSCTRL1;
    unsigned short TMR_FILT1;
    unsigned short TMR_DMA1;
    unsigned short usRes1[3];
    unsigned short TMR_COMP12;
    unsigned short TMR_COMP22;
    unsigned short TMR_CAPT2;
    unsigned short TMR_LOAD2;
    unsigned short TMR_HOLD2;
    unsigned short TMR_CNTR2;
    unsigned short TMR_CTRL2;
    unsigned short TMR_SCTRL2;
    unsigned short TMR_CMPLD12;
    unsigned short TMR_CMPLD22;
    unsigned short TMR_CSCTRL2;
    unsigned short TMR_FILT2;
    unsigned short TMR_DMA2;
    unsigned short usRes2[3];
    unsigned short TMR_COMP13;
    unsigned short TMR_COMP23;
    unsigned short TMR_CAPT3;
    unsigned short TMR_LOAD3;
    unsigned short TMR_HOLD3;
    unsigned short TMR_CNTR3;
    unsigned short TMR_CTRL3;
    unsigned short TMR_SCTRL3;
    unsigned short TMR_CMPLD13;
    unsigned short TMR_CMPLD23;
    unsigned short TMR_CSCTRL3;
    unsigned short TMR_FILT3;
    unsigned short TMR_DMA3;
} iMX_QUAD_TIMER;
#endif

typedef struct st_iMX_FLEXRAM
{
    unsigned long FLEXRAM_TCM_CTRL;
    unsigned long ulRes[3];
    unsigned long FLEXRAM_INT_STATUS;
    unsigned long FLEXRAM_INT_STAT_EN;
    unsigned long FLEXRAM_INT_SIG_EN;
} iMX_FLEXRAM;


#if defined ENHANCED_LCD_INTERFACE
typedef struct st_iMX_LCD
{
    unsigned long LCDIF_CTRL;
    unsigned long LCDIF_CTRL_SET;
    unsigned long LCDIF_CTRL_CLR;
    unsigned long LCDIF_CTRL_TOG;
    unsigned long LCDIF_CTRL1;
    unsigned long LCDIF_CTRL1_SET;
    unsigned long LCDIF_CTRL1_CLR;
    unsigned long LCDIF_CTRL1_TOG;
    unsigned long LCDIF_CTRL2;
    unsigned long LCDIF_CTRL2_SET;
    unsigned long LCDIF_CTRL2_CLR;
    unsigned long LCDIF_CTRL2_TOG;
    unsigned long LCDIF_TRANSFER_COUNT;
    unsigned long ulRes0[3];
    unsigned long LCDIF_CUR_BUF;
    unsigned long ulRes1[3];
    unsigned long LCDIF_NEXT_BUF;
    unsigned long ulRes2[7];
    unsigned long LCDIF_VDCTRL0;
    unsigned long LCDIF_VDCTRL0_SET;
    unsigned long LCDIF_VDCTRL0_CLR;
    unsigned long LCDIF_VDCTRL0_TOG;
    unsigned long LCDIF_VDCTRL1;
    unsigned long ulRes3[3];
    unsigned long LCDIF_VDCTRL2;
    unsigned long ulRes4[3];
    unsigned long LCDIF_VDCTRL3;
    unsigned long ulRes5[3];
    unsigned long LCDIF_VDCTRL4;
    unsigned long ulRes6[55];
    unsigned long LCDIF_BM_ERROR_STAT;
    unsigned long ulRes7[3];
    unsigned long LCDIF_CRC_STAT;
    unsigned long ulRes8[3];
    unsigned long LCDIF_STAT;
    unsigned long ulRes9[59];
    unsigned long LCDIF_RGB_ADJUST;
    unsigned long LCDIF_RGB_ADJUST_SET;
    unsigned long LCDIF_RGB_ADJUST_CLR;
    unsigned long LCDIF_RGB_ADJUST_TOG;
    unsigned long ulRes10[34];
    unsigned long LCDIF_PIGEONCTRL0;
    unsigned long LCDIF_PIGEONCTRL0_SET;
    unsigned long LCDIF_PIGEONCTRL0_CLR;
    unsigned long LCDIF_PIGEONCTRL0_TOG;
    unsigned long LCDIF_PIGEONCTRL1;
    unsigned long LCDIF_PIGEONCTRL1_SET;
    unsigned long LCDIF_PIGEONCTRL1_CLR;
    unsigned long LCDIF_PIGEONCTRL1_TOG;
    unsigned long LCDIF_PIGEONCTRL2;
    unsigned long LCDIF_PIGEONCTRL2_SET;
    unsigned long LCDIF_PIGEONCTRL2_CLR;
    unsigned long LCDIF_PIGEONCTRL2_TOG;
    unsigned long ulRes11[276];
    unsigned long LCDIF_PIGEON_0_0;
    unsigned long ulRes12[3];
    unsigned long LCDIF_PIGEON_0_1;
    unsigned long ulRes13[3];
    unsigned long LCDIF_PIGEON_0_2;
    unsigned long ulRes14[7];
    unsigned long LCDIF_PIGEON_1_0;
    unsigned long ulRes15[3];
    unsigned long LCDIF_PIGEON_1_1;
    unsigned long ulRes16[3];
    unsigned long LCDIF_PIGEON_1_2;
    unsigned long ulRes17[7];
    unsigned long LCDIF_PIGEON_2_0;
    unsigned long ulRes18[3];
    unsigned long LCDIF_PIGEON_2_1;
    unsigned long ulRes19[3];
    unsigned long LCDIF_PIGEON_2_2;
    unsigned long ulRes20[7];
    unsigned long LCDIF_PIGEON_3_0;
    unsigned long ulRes21[3];
    unsigned long LCDIF_PIGEON_3_1;
    unsigned long ulRes22[3];
    unsigned long LCDIF_PIGEON_3_2;
    unsigned long ulRes23[7];
    unsigned long LCDIF_PIGEON_4_0;
    unsigned long ulRes24[3];
    unsigned long LCDIF_PIGEON_4_1;
    unsigned long ulRes25[3];
    unsigned long LCDIF_PIGEON_4_2;
    unsigned long ulRes26[7];
    unsigned long LCDIF_PIGEON_5_0;
    unsigned long ulRes27[3];
    unsigned long LCDIF_PIGEON_5_1;
    unsigned long ulRes28[3];
    unsigned long LCDIF_PIGEON_5_2;
    unsigned long ulRes29[7];
    unsigned long LCDIF_PIGEON_6_0;
    unsigned long ulRes30[3];
    unsigned long LCDIF_PIGEON_6_1;
    unsigned long ulRes31[3];
    unsigned long LCDIF_PIGEON_6_2;
    unsigned long ulRes32[7];
    unsigned long LCDIF_PIGEON_7_0;
    unsigned long ulRes33[3];
    unsigned long LCDIF_PIGEON_7_1;
    unsigned long ulRes34[3];
    unsigned long LCDIF_PIGEON_7_2;
    unsigned long ulRes35[7];
    unsigned long LCDIF_PIGEON_8_0;
    unsigned long ulRes36[3];
    unsigned long LCDIF_PIGEON_8_1;
    unsigned long ulRes37[3];
    unsigned long LCDIF_PIGEON_8_2;
    unsigned long ulRes38[7];
    unsigned long LCDIF_PIGEON_9_0;
    unsigned long ulRes39[3];
    unsigned long LCDIF_PIGEON_9_1;
    unsigned long ulRes40[3];
    unsigned long LCDIF_PIGEON_9_2;
    unsigned long ulRes41[7];
    unsigned long LCDIF_PIGEON_10_0;
    unsigned long ulRes42[3];
    unsigned long LCDIF_PIGEON_10_1;
    unsigned long ulRes43[3];
    unsigned long LCDIF_PIGEON_10_2;
    unsigned long ulRes44[7];
    unsigned long LCDIF_PIGEON_11_0;
    unsigned long ulRes45[3];
    unsigned long LCDIF_PIGEON_11_1;
    unsigned long ulRes46[3];
    unsigned long LCDIF_PIGEON_11_2;
    unsigned long ulRes47[7];
    unsigned long LCDIF_LUT_CTRL;
    unsigned long ulRes48[3];
    unsigned long LCDIF_LUT0_ADDR;
    unsigned long ulRes49[3];
    unsigned long LCDIF_LUT0_DATA;
    unsigned long ulRes50[3];
    unsigned long LCDIF_LUT1_ADDR;
    unsigned long ulRes51[3];
    unsigned long LCDIF_LUT1_DATA;
} iMX_LCD;
#endif

typedef struct stiMX_PIT
{
unsigned long PIT_MCR;
unsigned long ulRes0[55];
unsigned long PIT_LTMR64H;
unsigned long PIT_LTMR64L;
unsigned long ulRes1[6];
unsigned long PIT_LDVAL0;
unsigned long PIT_CVAL0;
unsigned long PIT_TCTRL0;
unsigned long PIT_TFLG0;
unsigned long PIT_LDVAL1;
unsigned long PIT_CVAL1;
unsigned long PIT_TCTRL1;
unsigned long PIT_TFLG1;
    #if PITS_AVAILABLE > 2
    unsigned long PIT_LDVAL2;
    unsigned long PIT_CVAL2;
    unsigned long PIT_TCTRL2;
    unsigned long PIT_TFLG2;
    unsigned long PIT_LDVAL3;
    unsigned long PIT_CVAL3;
    unsigned long PIT_TCTRL3;
    unsigned long PIT_TFLG3;
    #endif
} iMX_PIT;

#if FLEX_TIMERS_AVAILABLE > 0
typedef struct stKINETIS_FTM
{
#if defined KINETIS_KL28
    unsigned long FTM_VERID;
    unsigned long FTM_PARAM;
    unsigned long FTM_GLOBAL;
    unsigned long ulRes_0;
#endif
unsigned long FTM_SC;
unsigned long FTM_CNT;
unsigned long FTM_MOD;
#if defined KINETIS_KL28
    unsigned long FTM_STATUS;
#endif
unsigned long FTM_C0SC;
unsigned long FTM_C0V;
unsigned long FTM_C1SC;
unsigned long FTM_C1V;
unsigned long FTM_C2SC;
unsigned long FTM_C2V;
unsigned long FTM_C3SC;
unsigned long FTM_C3V;
unsigned long FTM_C4SC;
unsigned long FTM_C4V;
unsigned long FTM_C5SC;
unsigned long FTM_C5V;
#if defined KINETIS_KL || (defined KINETIS_KE && !defined KINETIS_KE06)
    #if defined KINETIS_KL28
        unsigned long ulRes0[5];
        unsigned long FTM_COMBINE;
        unsigned long ulRes1;
        unsigned long FTM_TRIG;
        unsigned long FTM_POL;
        unsigned long ulRes2;
        unsigned long FTM_FILTER;
        unsigned long ulRes3;
        unsigned long FTM_QDCTRL;
    #else
        unsigned long ulRes0[6];
        unsigned long FTM_STATUS;
        unsigned long ulRes1[12];
    #endif
    unsigned long FTM_CONF;
#else
    unsigned long FTM_C6SC;
    unsigned long FTM_C6V;
    unsigned long FTM_C7SC;
    unsigned long FTM_C7V;
    unsigned long FTM_CNTIN;
    unsigned long FTM_STATUS;
    unsigned long FTM_MODE;
    unsigned long FTM_SYNC;
    unsigned long FTM_OUTINIT;
    unsigned long FTM_OUTMASK;
    unsigned long FTM_COMBINE;
    unsigned long FTM_DEADTIME;
    unsigned long FTM_EXTTRIG;
    unsigned long FTM_POL;
    unsigned long FTM_FMS;
    unsigned long FTM_FILTER;
    unsigned long FTM_FLTCTRL;
    #if defined KINETIS_KE06
    unsigned long ulRes10;
    #else
    unsigned long FTM_QDCTRL;
    #endif
    unsigned long FTM_CONF;
    unsigned long FTM_FLTPOL;
    unsigned long FTM_SYNCONF;
    unsigned long FTM_INVCTRL;
    unsigned long FTM_SWOCTRL;
    unsigned long FTM_PWMLOAD;
#endif
} KINETIS_FTM;
#endif

typedef struct stKINETIS_ADC
{
#if defined KINETIS_KE15
    volatile unsigned long ADC_SC1A;
    unsigned long ADC_SC1B;
    unsigned long ulRes0[14];
    unsigned long ADC_CFG1;
    unsigned long ADC_CFG2;
    volatile unsigned long ADC_RA;
    volatile unsigned long ADC_RB;
    unsigned long ulRes1[14];
    unsigned long ADC_CV1;
    unsigned long ADC_CV2;
    volatile unsigned long ADC_SC2;
    volatile unsigned long ADC_SC3;
    unsigned long ADC_BASE_OFS;
    unsigned long ADC_OFS;
    unsigned long ADC_USR_OFS;
    unsigned long ADC_XOFS;
    unsigned long ADC_YOFS;
    unsigned long ADC_G;
    unsigned long ADC_UG;
    unsigned long ADC_CLPS;
    unsigned long ADC_CLP3;
    unsigned long ADC_CLP2;
    unsigned long ADC_CLP1;
    unsigned long ADC_CLP0;
    unsigned long ADC_CLPX;
    unsigned long ADC_CLP9;
    unsigned long ADC_CLPS_OFS;
    unsigned long ADC_CLP3_OFS;
    unsigned long ADC_CLP2_OFS;
    unsigned long ADC_CLP1_OFS;
    unsigned long ADC_CLP0_OFS;
    unsigned long ADC_CLPX_OFS;
    unsigned long ADC_CLP9_OFS;
#elif defined KINETIS_KE
    unsigned long ADC_SC1;
    unsigned long ADC_SC2;
    unsigned long ADC_SC3;
    unsigned long ADC_SC4;
    unsigned long ADC_R;
    unsigned long ADC_CV;
    unsigned long ADC_APCTL1;
#else
    unsigned long ADC_SC1A;
    unsigned long ADC_SC1B;
    unsigned long ADC_CFG1;
    unsigned long ADC_CFG2;
    unsigned long ADC_RA;
    unsigned long ADC_RB;
    unsigned long ADC_CV1;
    unsigned long ADC_CV2;
    unsigned long ADC_SC2;
    unsigned long ADC_SC3;
    unsigned long ADC_OFS;
    unsigned long ADC_PG;
    unsigned long ADC_MG;
    unsigned long ADC_CLPD;
    unsigned long ADC_CLPS;
    unsigned long ADC_CLP4;
    unsigned long ADC_CLP3;
    unsigned long ADC_CLP2;
    unsigned long ADC_CLP1;
    unsigned long ADC_CLP0;
    unsigned long ADC_PGA;
    unsigned long ADC_CLMD;
    unsigned long ADC_CLMS;
    unsigned long ADC_CLM4;
    unsigned long ADC_CLM3;
    unsigned long ADC_CLM2;
    unsigned long ADC_CLM1;
    unsigned long ADC_CLM0;
#endif
} KINETIS_ADC;


typedef struct st_iMX_SDHC
{
unsigned long SDHC_DSADDR;
unsigned long SDHC_BLKATTR;
unsigned long SDHC_CMDARG;
unsigned long SDHC_XFERTYP;
unsigned long SDHC_CMDRSP0;
unsigned long SDHC_CMDRSP1;
unsigned long SDHC_CMDRSP2;
unsigned long SDHC_CMDRSP3;
unsigned long SDHC_DATPORT;
unsigned long SDHC_PRSSTAT;
unsigned long SDHC_PROCTL;
unsigned long SDHC_SYSCTL;
unsigned long SDHC_IRQSTAT;
unsigned long SDHC_IRQSTATEN;
unsigned long SDHC_IRQSIGEN;
unsigned long SDHC_AC12ERR;
unsigned long SDHC_HTCAPBLT;
unsigned long SDHC_WML;
unsigned long ulRes0[2];
unsigned long SDHC_FEVT;
unsigned long SDHC_ADMAES;
unsigned long SDHC_ADSADDR;
unsigned long ulRes1[25];
unsigned long SDHC_VENDOR;
unsigned long SDHC_MMCBOOT;
unsigned long ulRes2[13];
unsigned long SDHC_HOSTVER;
} iMX_SDHC;

typedef struct stKINETIS_OSC
{
unsigned char OSC_CR;
} KINETIS_OSC;

#if I2C_AVAILABLE > 0
typedef struct stKINETIS_I2C
{
unsigned char I2C_A1;
unsigned char I2C_F;
unsigned char I2C_C1;
unsigned char I2C_S;
unsigned char I2C_D;
unsigned char I2C_C2;
unsigned char I2C_FLT;
unsigned char I2C_RA;
unsigned char I2C_SMB;
unsigned char I2C_A2;
unsigned char I2C_SLTH;
unsigned char I2C_SLTL;
} KINETIS_I2C;
#endif

typedef struct stiMX_LPI2C
{
    unsigned long  LPI2C_VERID;
    unsigned long  LPI2C_PARAM;
    unsigned long  ulRes0[2];
    unsigned long  LPI2C_MCR;
    unsigned long  LPI2C_MSR;
    unsigned long  LPI2C_MIER;
    unsigned long  LPI2C_MDER;
    unsigned long  LPI2C_MCFGR0;
    unsigned long  LPI2C_MCFGR1;
    unsigned long  LPI2C_MCFGR2;
    unsigned long  LPI2C_MCFGR3;
    unsigned long  ulRes1[4];
    unsigned long  LPI2C_MDMR;
    unsigned long  ulRes2;
    unsigned long  LPI2C_MCCR0;
    unsigned long  ulRes3;
    unsigned long  LPI2C_MCCR1;
    unsigned long  ulRes4;
    unsigned long  LPI2C_MFCR;
    unsigned long  LPI2C_MFSR;
    unsigned long  LPI2C_MTDR;
    unsigned long  ulRes5[3];
    unsigned long  LPI2C_MRDR;
    unsigned long  ulRes6[27];
    unsigned long  LPI2C_SCR;
    unsigned long  LPI2C_SSR;
    unsigned long  LPI2C_SIER;
    unsigned long  LPI2C_SDER;
    unsigned long  ulRes7;
    unsigned long  LPI2C_SCFGR1;
    unsigned long  LPI2C_SCFGR2;
    unsigned long  ulRes8[4];
    unsigned long  LPI2C_SAMR;
    unsigned long  ulRes9[3];
    unsigned long  LPI2C_SASR;
    unsigned long  LPI2C_STAR;
    unsigned long  ulRes10[2];
    unsigned long  LPI2C_STDR;
    unsigned long  ulRes11[3];
    unsigned long  LPI2C_SRDR;
} iMX_LPI2C;

typedef struct stiMX_OCOTP
{
    unsigned long HW_OCOTP_CTRL;
    unsigned long HW_OCOTP_CTRL_SET;
    unsigned long HW_OCOTP_CTRL_CLR;
    unsigned long HW_OCOTP_CTRL_TOG;
    unsigned long HW_OCOTP_TIMING;
    unsigned long ulRes0[3];
    unsigned long HW_OCOTP_DATA;
    unsigned long ulRes1[3];
    unsigned long HW_OCOTP_READ_CTRL;
    unsigned long ulRes2[3];
    unsigned long HW_OCOTP_READ_FUSE_DATA;
    unsigned long ulRes3[3];
    unsigned long HW_OCOTP_SW_STICKY;
    unsigned long ulRes4[3];
    unsigned long HW_OCOTP_SCS;
    unsigned long HW_OCOTP_SCS_SET;
    unsigned long HW_OCOTP_SCS_CLR;
    unsigned long HW_OCOTP_SCS_TOG;
    unsigned long ulRes5[8];
    unsigned long HW_OCOTP_VERSION;
    unsigned long ulRes6[27];
    unsigned long HW_OCOTP_TIMING2;
    unsigned long ulRes7[191];
    unsigned long HW_OCOTP_LOCK;
    unsigned long ulRes8[3];
    unsigned long HW_OCOTP_CFG0;
    unsigned long ulRes9[3];
    unsigned long HW_OCOTP_CFG1;
    unsigned long ulRes10[3];
    unsigned long HW_OCOTP_CFG2;
    unsigned long ulRes11[3];
    unsigned long HW_OCOTP_CFG3;
    unsigned long ulRes12[3];
    unsigned long HW_OCOTP_CFG4;
    unsigned long ulRes13[3];
    unsigned long HW_OCOTP_CFG5;
    unsigned long ulRes14[3];
    unsigned long HW_OCOTP_CFG6;
    unsigned long ulRes15[3];
    unsigned long HW_OCOTP_MEM0;
    unsigned long ulRes16[3];
    unsigned long HW_OCOTP_MEM1;
    unsigned long ulRes17[3];
    unsigned long HW_OCOTP_MEM2;
    unsigned long ulRes18[3];
    unsigned long HW_OCOTP_MEM3;
    unsigned long ulRes19[3];
    unsigned long HW_OCOTP_MEM4;
    unsigned long ulRes20[3];
    unsigned long HW_OCOTP_ANA0;
    unsigned long ulRes21[3];
    unsigned long HW_OCOTP_ANA1;
    unsigned long ulRes22[3];
    unsigned long HW_OCOTP_ANA2;
    unsigned long ulRes23[35];
    unsigned long HW_OCOTP_SRK0;
    unsigned long ulRes24[3];
    unsigned long HW_OCOTP_SRK1;
    unsigned long ulRes25[3];
    unsigned long HW_OCOTP_SRK2;
    unsigned long ulRes26[3];
    unsigned long HW_OCOTP_SRK3;
    unsigned long ulRes27[3];
    unsigned long HW_OCOTP_SRK4;
    unsigned long ulRes28[3];
    unsigned long HW_OCOTP_SRK5;
    unsigned long ulRes29[3];
    unsigned long HW_OCOTP_SRK6;
    unsigned long ulRes30[3];
    unsigned long HW_OCOTP_SRK7;
    unsigned long ulRes31[3];
    unsigned long HW_OCOTP_SJC_RESP0;
    unsigned long ulRes32[3];
    unsigned long HW_OCOTP_SJC_RESP1;
    unsigned long ulRes33[3];
    unsigned long HW_OCOTP_MAC0;
    unsigned long ulRes34[3];
    unsigned long HW_OCOTP_MAC1;
    unsigned long ulRes35[3];
#if ETHERNET_COUNT > 1
    unsigned long HW_OCOTP_MAC2;
    unsigned long ulRes36[3];
    unsigned long HW_OCOTPOTPK_CRC32;
    unsigned long ulRes36a[3];
#else
    unsigned long HW_OCOTP_GP3;
    unsigned long ulRes36[7];
#endif
    unsigned long HW_OCOTP_GP1;
    unsigned long ulRes37[3];
    unsigned long HW_OCOTP_GP2;
    unsigned long ulRes38[3];
    unsigned long HW_OCOTP_SW_GP1;
    unsigned long ulRes39[3];
    unsigned long HW_OCOTP_SW_GP20;
    unsigned long ulRes40[3];
    unsigned long HW_OCOTP_SW_GP21;
    unsigned long ulRes41[3];
    unsigned long HW_OCOTP_SW_GP22;
    unsigned long ulRes42[3];
    unsigned long HW_OCOTP_SW_GP23;
    unsigned long ulRes43[3];
    unsigned long HW_OCOTP_MISC_CONF0;
    unsigned long ulRes44[3];
    unsigned long HW_OCOTP_MISC_CONF1;
    unsigned long ulRes45[3];
    unsigned long HW_OCOTP_SRK_REVOKE;
    unsigned long ulRes46[3];
#if defined iMX_RT106X
    unsigned long ulRes47[43];
    unsigned long HW_OCOTP_ROM_PATCH0;
    unsigned long ulRes48[3];
    unsigned long HW_OCOTP_ROM_PATCH1;
    unsigned long ulRes49[3];
    unsigned long HW_OCOTP_ROM_PATCH2;
    unsigned long ulRes50[3];
    unsigned long HW_OCOTP_ROM_PATCH3;
    unsigned long ulRes51[3];
    unsigned long HW_OCOTP_ROM_PATCH4;
    unsigned long ulRes52[3];
    unsigned long HW_OCOTP_ROM_PATCH5;
    unsigned long ulRes53[3];
    unsigned long HW_OCOTP_ROM_PATCH6;
    unsigned long ulRes54[3];
    unsigned long HW_OCOTP_ROM_PATCH7;
    unsigned long ulRes55[3];
    unsigned long HW_OCOTP_GP30;
    unsigned long ulRes56[3];
    unsigned long HW_OCOTP_GP31;
    unsigned long ulRes57[3];
    unsigned long HW_OCOTP_GP32;
    unsigned long ulRes58[3];
    unsigned long HW_OCOTP_GP33;
    unsigned long ulRes59[3];
    unsigned long HW_OCOTP_GP40;
    unsigned long ulRes60[3];
    unsigned long HW_OCOTP_GP41;
    unsigned long ulRes61[3];
    unsigned long HW_OCOTP_GP42;
    unsigned long ulRes62[3];
    unsigned long HW_OCOTP_GP43;
    unsigned long ulRes63[3];
#endif
} iMX_OCOTP;

typedef struct stiMX_eFUSE
{
    unsigned long OCOTP_LOCK[4];
    unsigned long OCOTP_CFG0[4];
    unsigned long OCOTP_CFG1[4];
    unsigned long OCOTP_CFG2[4];
    unsigned long OCOTP_CFG3[4];
    unsigned long OCOTP_CFG4[4];
    unsigned long OCOTP_CFG5[4];
    unsigned long OCOTP_CFG6[4];
    unsigned long OCOTP_MEM0[4];
    unsigned long OCOTP_MEM1[4];
    unsigned long OCOTP_MEM2[4];
    unsigned long OCOTP_MEM3[4];
    unsigned long OCOTP_MEM4[4];
    unsigned long OCOTP_ANA0[4];
    unsigned long OCOTP_ANA1[4];
    unsigned long OCOTP_ANA2[4];
    unsigned long OCOTP_SRK[32];
    unsigned long OCOTP_SJC_RESP[8];
    unsigned long OCOTP_MAC0[4];
    unsigned long OCOTP_MAC1[4];
#if ETHERNET_COUNT > 1
        unsigned long OCOTP_MAC2[4];
        unsigned long OCOTP_OTPK_CRC32[4];
#else
        unsigned long OCOTP_GP3[4];
        unsigned long ulRes0[4];
#endif
    unsigned long OCOTP_GP1[4];
    unsigned long OCOTP_GP2[4];
    unsigned long OCOTP_SW_GP1[4];
    unsigned long OCOTP_SW_GP20[4];
    unsigned long OCOTP_SW_GP21[4];
    unsigned long OCOTP_SW_GP22[4];
    unsigned long OCOTP_SW_GP23[4];
    unsigned long OCOTP_MISC_CONF0[4];
    unsigned long OCOTP_MISC_CONF1[4];
    unsigned long OCOTP_SRK_REVOKE[4];
#if defined iMX_RT106X
        unsigned long OCOTP_ROM_PATCH0[4];
        unsigned long OCOTP_ROM_PATCH1[4];
        unsigned long OCOTP_ROM_PATCH2[4];
        unsigned long OCOTP_ROM_PATCH3[4];
        unsigned long OCOTP_ROM_PATCH4[4];
        unsigned long OCOTP_ROM_PATCH5[4];
        unsigned long OCOTP_ROM_PATCH6[4];
        unsigned long OCOTP_ROM_PATCH7[4];
        unsigned long OCOTP_GP30[4];
        unsigned long OCOTP_GP31[4];
        unsigned long OCOTP_GP32[4];
        unsigned long OCOTP_GP33[4];
        unsigned long OCOTP_GP40[4];
        unsigned long OCOTP_GP41[4];
        unsigned long OCOTP_GP42[4];
        unsigned long OCOTP_GP43[4];
#endif
} iMX_eFUSE;

typedef struct stiMX_LPUART
{
    volatile unsigned long  LPUART_VERID;
    volatile unsigned long  LPUART_PARAM;
    unsigned long  LPUART_GLOBAL;
    unsigned long  LPUART_PINCFG;
    unsigned long LPUART_BAUD;
    volatile unsigned long LPUART_STAT;
    volatile unsigned long LPUART_CTRL;
    volatile unsigned long LPUART_DATA;
    unsigned long LPUART_MATCH;
    unsigned long LPUART_MODIR;
    unsigned long LPUART_FIFO;
    unsigned long LPUART_WATER;
} iMX_LPUART;

typedef struct stKINETIS_USB
{
unsigned char PER_ID;
unsigned char ucRes1[3];
unsigned char ID_COMP;
unsigned char ucRes2[3];
unsigned char REV;
unsigned char ucRes3[3];
unsigned char ADD_INFO;
unsigned char ucRes4[3];
unsigned char OTG_INT_STAT;
unsigned char ucRes5[3];
unsigned char OTG_INT_EN;
unsigned char ucRes6[3];
unsigned char OTG_STATUS;
unsigned char ucRes7[3];
unsigned char OTG_CTRL;
unsigned char ucRes8[0x63];
unsigned char INT_STAT;
unsigned char ucRes9[3];
unsigned char INT_ENB;
unsigned char ucRes10[3];
unsigned char ERR_STAT;
unsigned char ucRes11[3];
unsigned char ERR_ENG;
unsigned char ucRes12[3];
unsigned char STAT;
unsigned char ucRes13[3];
unsigned char CTL;
unsigned char ucRes14[3];
unsigned char ADDR;
unsigned char ucRes15[3];
unsigned char BDT_PAGE_01;
unsigned char ucRes16[3];
unsigned char FRM_NUML;
unsigned char ucRes17[3];
unsigned char FRM_NUMH;
unsigned char ucRes18[3];
unsigned char TOKEN;
unsigned char ucRes19[3];
unsigned char SOF_THLD;
unsigned char ucRes20[3];
unsigned char BDT_PAGE_02;
unsigned char ucRes21[3];
unsigned char BDT_PAGE_03;
unsigned char ucRes22[11];
unsigned char ENDPT0;
unsigned char ucRes23[3];
unsigned char ENDPT1;
unsigned char ucRes24[3];
unsigned char ENDPT2;
unsigned char ucRes25[3];
unsigned char ENDPT3;
unsigned char ucRes26[3];
unsigned char ENDPT4;
unsigned char ucRes27[3];
unsigned char ENDPT5;
unsigned char ucRes28[3];
unsigned char ENDPT6;
unsigned char ucRes29[3];
unsigned char ENDPT7;
unsigned char ucRes30[3];
unsigned char ENDPT8;
unsigned char ucRes31[3];
unsigned char ENDPT9;
unsigned char ucRes32[3];
unsigned char ENDPT10;
unsigned char ucRes33[3];
unsigned char ENDPT11;
unsigned char ucRes34[3];
unsigned char ENDPT12;
unsigned char ucRes35[3];
unsigned char ENDPT13;
unsigned char ucRes36[3];
unsigned char ENDPT14;
unsigned char ucRes37[3];
unsigned char ENDPT15;
unsigned char ucRes38[3];
unsigned char USB_CTRL;
unsigned char ucRes39[3];
unsigned char USB_OTG_OBSERVE;
unsigned char ucRes40[3];
unsigned char USB_OTG_CONTROL;
unsigned char ucRes41[3];
unsigned char USB_USBTRC0;
#if defined KINETIS_HAS_IRC48M
    unsigned char ucRes42[7];
    unsigned char USB_USBFRMADJUST;
    unsigned char ucRes43[43];
    unsigned char USB_CLK_RECOVER_CTRL;
    unsigned char ucRes44[3];
    unsigned char USB_CLK_RECOVER_IRC_EN;
    #if defined KINETIS_KL28
    unsigned char ucRes45[15];
    unsigned char USB_CLK_RECOVER_INT_EN;
    unsigned char ucRes46[7];
    #else
    unsigned char ucRes45[23];
    #endif
    unsigned char USB_CLK_RECOVER_INT_STATUS;
#endif
} KINETIS_USB;


#if defined KINETIS_KE
    typedef struct stKINETIS_ACMP                                        // analogue comparator
    {
    unsigned char ACMP_CS;
    unsigned char ACMP_C0;
    unsigned char ACMP_C1;
    unsigned char ACMP_C2;
    } KINETIS_ACMP;
#else
    typedef struct stKINETIS_CMP                                         // comparator
    {
    unsigned char CMP_CR0;
    unsigned char CMP_CR1;
    unsigned char CMP_FPR;
    unsigned char CMP_SCR;
    unsigned char CMP_DACCR;
    unsigned char CMP_MUXCR;
    unsigned char ucRes[2];
    } KINETIS_CMP;
#endif

#if !defined KINETIS_KL
    typedef struct stKINETIS_VREF                                        // VREF
    {
    unsigned char VREF_TRM;
    unsigned char VREF_SC;
    } KINETIS_VREF;
#endif


#if defined DEVICE_WITH_SLCD
typedef struct stKINETIS_SLCD
{
unsigned long LCD_GCR;
unsigned long LCD_AR;
unsigned long LCD_FDCR;
unsigned long LCD_FDSR;
unsigned long LCD_PENL;
unsigned long LCD_PENH;
unsigned long LCD_BPENL;
unsigned long LCD_BPENH;
unsigned long LCD_WF3TO0;
unsigned long LCD_WF7TO4;
unsigned long LCD_WF11TO8;
unsigned long LCD_WF15TO12;
unsigned long LCD_WF19TO16;
unsigned long LCD_WF23TO20;
unsigned long LCD_WF27TO24;
unsigned long LCD_WF31TO28;
unsigned long LCD_WF35TO32;
unsigned long LCD_WF39TO36;
unsigned long LCD_WF43TO40;
unsigned long LCD_WF47TO44;
unsigned long LCD_WF51TO48;
unsigned long LCD_WF55TO52;
unsigned long LCD_WF59TO56;
unsigned long LCD_WF63TO60;
} KINETIS_SLCD;
#endif

#if (NUMBER_OF_CAN_INTERFACES > 0)
typedef struct stiMX_CAN
{
    unsigned long  CAN_MCR;
    unsigned long  CAN_CTRL;
    unsigned long  CAN_TIMER;
    unsigned long  ulRes1;
    unsigned long  RXGMASK;
    unsigned long  RX14MASK;
    unsigned long  RX15MASK;
    unsigned long  CAN_ECR;
    unsigned long  CAN_ESR1;
    unsigned long  CAN_IMASK2;
    unsigned long  CAN_IMASK1;
    unsigned long  CAN_IFLAG2;
    unsigned long  CAN_IFLAG1;
    unsigned long  CAN_CTRL2;
    unsigned long  CAN_ESR2;
    unsigned long  ulRes2[2];
    unsigned long  CAN_CRCR;
    unsigned long  CAN_RXFGMASK;
    unsigned long  CAN_RXFIR;
    unsigned long  ulRes3[2];
    unsigned long  FLEXCAN_DBG1;
    unsigned long  FLEXCAN0_DBG2;
    unsigned long  ulRes4[8];
    KINETIS_CAN_BUF  MBUFF0;
    KINETIS_CAN_BUF  MBUFF1;
    KINETIS_CAN_BUF  MBUFF2;
    KINETIS_CAN_BUF  MBUFF3;
    KINETIS_CAN_BUF  MBUFF4;
    KINETIS_CAN_BUF  MBUFF5;
    KINETIS_CAN_BUF  MBUFF6;
    KINETIS_CAN_BUF  MBUFF7;
    KINETIS_CAN_BUF  MBUFF8;
    KINETIS_CAN_BUF  MBUFF9;
    KINETIS_CAN_BUF  MBUFF10;
    KINETIS_CAN_BUF  MBUFF11;
    KINETIS_CAN_BUF  MBUFF12;
    KINETIS_CAN_BUF  MBUFF13;
    KINETIS_CAN_BUF  MBUFF14;
    KINETIS_CAN_BUF  MBUFF15;
    KINETIS_CAN_BUF  MBUFF16;
    KINETIS_CAN_BUF  MBUFF17;
    KINETIS_CAN_BUF  MBUFF18;
    KINETIS_CAN_BUF  MBUFF19;
    KINETIS_CAN_BUF  MBUFF20;
    KINETIS_CAN_BUF  MBUFF21;
    KINETIS_CAN_BUF  MBUFF22;
    KINETIS_CAN_BUF  MBUFF23;
    KINETIS_CAN_BUF  MBUFF24;
    KINETIS_CAN_BUF  MBUFF25;
    KINETIS_CAN_BUF  MBUFF26;
    KINETIS_CAN_BUF  MBUFF27;
    KINETIS_CAN_BUF  MBUFF28;
    KINETIS_CAN_BUF  MBUFF29;
    KINETIS_CAN_BUF  MBUFF30;
    KINETIS_CAN_BUF  MBUFF31;
    KINETIS_CAN_BUF  MBUFF32;
    KINETIS_CAN_BUF  MBUFF33;
    KINETIS_CAN_BUF  MBUFF34;
    KINETIS_CAN_BUF  MBUFF35;
    KINETIS_CAN_BUF  MBUFF36;
    KINETIS_CAN_BUF  MBUFF37;
    KINETIS_CAN_BUF  MBUFF38;
    KINETIS_CAN_BUF  MBUFF39;
    KINETIS_CAN_BUF  MBUFF40;
    KINETIS_CAN_BUF  MBUFF41;
    KINETIS_CAN_BUF  MBUFF42;
    KINETIS_CAN_BUF  MBUFF43;
    KINETIS_CAN_BUF  MBUFF44;
    KINETIS_CAN_BUF  MBUFF45;
    KINETIS_CAN_BUF  MBUFF46;
    KINETIS_CAN_BUF  MBUFF47;
    KINETIS_CAN_BUF  MBUFF48;
    KINETIS_CAN_BUF  MBUFF49;
    KINETIS_CAN_BUF  MBUFF50;
    KINETIS_CAN_BUF  MBUFF51;
    KINETIS_CAN_BUF  MBUFF52;
    KINETIS_CAN_BUF  MBUFF53;
    KINETIS_CAN_BUF  MBUFF54;
    KINETIS_CAN_BUF  MBUFF55;
    KINETIS_CAN_BUF  MBUFF56;
    KINETIS_CAN_BUF  MBUFF57;
    KINETIS_CAN_BUF  MBUFF58;
    KINETIS_CAN_BUF  MBUFF59;
    KINETIS_CAN_BUF  MBUFF60;
    KINETIS_CAN_BUF  MBUFF61;
    KINETIS_CAN_BUF  MBUFF62;
    KINETIS_CAN_BUF  MBUFF63;
    unsigned long  ulRes5[256];
    unsigned long  CAN_RXIMR0;
    unsigned long  CAN_RXIMR1;
    unsigned long  CAN_RXIMR2;
    unsigned long  CAN_RXIMR3;
    unsigned long  CAN_RXIMR4;
    unsigned long  CAN_RXIMR5;
    unsigned long  CAN_RXIMR6;
    unsigned long  CAN_RXIMR7;
    unsigned long  CAN_RXIMR8;
    unsigned long  CAN_RXIMR9;
    unsigned long  CAN_RXIMR10;
    unsigned long  CAN_RXIMR11;
    unsigned long  CAN_RXIMR12;
    unsigned long  CAN_RXIMR13;
    unsigned long  CAN_RXIMR14;
    unsigned long  CAN_RXIMR15;
    unsigned long  CAN_RXIMR16;
    unsigned long  CAN_RXIMR17;
    unsigned long  CAN_RXIMR18;
    unsigned long  CAN_RXIMR19;
    unsigned long  CAN_RXIMR20;
    unsigned long  CAN_RXIMR21;
    unsigned long  CAN_RXIMR22;
    unsigned long  CAN_RXIMR23;
    unsigned long  CAN_RXIMR24;
    unsigned long  CAN_RXIMR25;
    unsigned long  CAN_RXIMR26;
    unsigned long  CAN_RXIMR27;
    unsigned long  CAN_RXIMR28;
    unsigned long  CAN_RXIMR29;
    unsigned long  CAN_RXIMR30;
    unsigned long  CAN_RXIMR31;
    unsigned long  CAN_RXIMR32;
    unsigned long  CAN_RXIMR33;
    unsigned long  CAN_RXIMR34;
    unsigned long  CAN_RXIMR35;
    unsigned long  CAN_RXIMR36;
    unsigned long  CAN_RXIMR37;
    unsigned long  CAN_RXIMR38;
    unsigned long  CAN_RXIMR39;
    unsigned long  CAN_RXIMR40;
    unsigned long  CAN_RXIMR41;
    unsigned long  CAN_RXIMR42;
    unsigned long  CAN_RXIMR43;
    unsigned long  CAN_RXIMR44;
    unsigned long  CAN_RXIMR45;
    unsigned long  CAN_RXIMR46;
    unsigned long  CAN_RXIMR47;
    unsigned long  CAN_RXIMR48;
    unsigned long  CAN_RXIMR49;
    unsigned long  CAN_RXIMR50;
    unsigned long  CAN_RXIMR51;
    unsigned long  CAN_RXIMR52;
    unsigned long  CAN_RXIMR53;
    unsigned long  CAN_RXIMR54;
    unsigned long  CAN_RXIMR55;
    unsigned long  CAN_RXIMR56;
    unsigned long  CAN_RXIMR57;
    unsigned long  CAN_RXIMR58;
    unsigned long  CAN_RXIMR59;
    unsigned long  CAN_RXIMR60;
    unsigned long  CAN_RXIMR61;
    unsigned long  CAN_RXIMR62;
    unsigned long  CAN_RXIMR63;
    unsigned long  ulRes6[18];
    unsigned long  CAN_GFWR;
} iMX_CAN;
#endif

#if defined ETHERNET_AVAILABLE
typedef struct stiMX_EMAC
{
unsigned long ulRes;
unsigned long EIR;
unsigned long EIMR;
unsigned long ulRes0;
unsigned long RDAR;
unsigned long TDAR;
unsigned long ulRes1[3];
unsigned long ECR;
unsigned long ulRes2[6];
unsigned long MMFR;
unsigned long MSCR;
unsigned long ulRes3[7];
unsigned long MIBC;
unsigned long ulRes4[7];
unsigned long RCR;
unsigned long ulRes5[15];
unsigned long TCR;
unsigned long ulRes6[7];
unsigned long PALR;
unsigned long PAUR;
unsigned long OPD;
unsigned long ulRes7[10];
unsigned long IAUR;
unsigned long IALR;
unsigned long GAUR;
unsigned long GALR;
unsigned long ulRes8[7];
unsigned long ENET_TFWR;
unsigned long ulRes9[14];
unsigned long ERDSR;
unsigned long ETDSR;
unsigned long EMRBR;
unsigned long ulRes10;
unsigned long ENET_RSFL;
unsigned long ENET_RSEM;
unsigned long ENET_RAEM;
unsigned long ENET_RAFL;
unsigned long ENET_TSEM;
unsigned long ENET_TAEM;
unsigned long ENET_TAFL;
unsigned long ENET_TIPG;
unsigned long ENET_FTRL;
unsigned long ulRes11[3];
unsigned long ENET_TACC;
unsigned long ENET_RACC;
unsigned long ulRes12[14];
unsigned long ulMIB[512/sizeof (unsigned long)];                         // counters
unsigned long ENET_ATCR;
unsigned long ENET_ATVR;
unsigned long ENET_ATOFF;
unsigned long ENET_ATPER;
unsigned long ENET_ATCOR;
unsigned long ENET_ATINC;
unsigned long ENET_ATSTMP;
unsigned long ulRes13[122];
unsigned long ENET_TGSR;
unsigned long ENET_TCSR0;
unsigned long ENET_TCCR0;
unsigned long ENET_TCSR1;
unsigned long ENET_TCCR1;
unsigned long ENET_TCSR2;
unsigned long ENET_TCCR2;
unsigned long ENET_TCSR3;
unsigned long ENET_TCCR3;
} iMX_EMAC;
#endif

typedef struct stKINETIS_DAC
{
#if defined KINETIS_KE18
    unsigned long DAC0_DAT0;
    unsigned long DAC0_DAT1;
    unsigned long DAC0_DAT2;
    unsigned long DAC0_DAT3;
    unsigned long DAC0_DAT4;
    unsigned long DAC0_DAT5;
    unsigned long DAC0_DAT6;
    unsigned long DAC0_DAT7;
    unsigned long DAC0_STATCTRL;
#else
    unsigned char DAC_DAT0L;
    unsigned char DAC_DAT0H;
    unsigned char DAC_DAT1L;
    unsigned char DAC_DAT1H;
    #if defined KINETIS_KL && !defined KINETIS_KL28
        unsigned char ucRes0[28];
    #else
        unsigned char DAC_DAT2L;
        unsigned char DAC_DAT2H;
        unsigned char DAC_DAT3L;
        unsigned char DAC_DAT3H;
        unsigned char DAC_DAT4L;
        unsigned char DAC_DAT4H;
        unsigned char DAC_DAT5L;
        unsigned char DAC_DAT5H;
        unsigned char DAC_DAT6L;
        unsigned char DAC_DAT6H;
        unsigned char DAC_DAT7L;
        unsigned char DAC_DAT7H;
        unsigned char DAC_DAT8L;
        unsigned char DAC_DAT8H;
        unsigned char DAC_DAT9L;
        unsigned char DAC_DAT9H;
        unsigned char DAC_DAT10L;
        unsigned char DAC_DAT10H;
        unsigned char DAC_DAT11L;
        unsigned char DAC_DAT11H;
        unsigned char DAC_DAT12L;
        unsigned char DAC_DAT12H;
        unsigned char DAC_DAT13L;
        unsigned char DAC_DAT13H;
        unsigned char DAC_DAT14L;
        unsigned char DAC_DAT14H;
        unsigned char DAC_DAT15L;
        unsigned char DAC_DAT15H;
    #endif
    unsigned char DAC_SR;
    unsigned char DAC_C0;
    unsigned char DAC_C1;
    unsigned char DAC_C2;
#endif
} KINETIS_DAC;


#if defined CHIP_HAS_FLEXIO
typedef struct stKINETIS_FLEXIO
{
unsigned long FLEXIO_VERID;
unsigned long FLEXIO_PARAM;
unsigned long FLEXIO_CTRL;
    #if defined KINETIS_KL28 || defined KINETIS_K80
unsigned long FLEXIO_PIN;
    #else
unsigned long ulRes0;
    #endif
unsigned long FLEXIO_SHIFTSTAT;
unsigned long FLEXIO_SHIFTERR;
unsigned long FLEXIO_TIMSTAT;
unsigned long ulRes1;
unsigned long FLEXIO_SHIFTSIEN;
unsigned long FLEXIO_SHIFTEIEN;
unsigned long FLEXIO_TIMIEN;
unsigned long ulRes2;
unsigned long FLEXIO_SHIFTSDEN;
unsigned long ulRes3[19];
unsigned long FLEXIO_SHIFTCTL0;
unsigned long FLEXIO_SHIFTCTL1;
unsigned long FLEXIO_SHIFTCTL2;
unsigned long FLEXIO_SHIFTCTL3;
unsigned long ulRes4[28];
unsigned long FLEXIO_SHIFTCFG0;
unsigned long FLEXIO_SHIFTCFG1;
unsigned long FLEXIO_SHIFTCFG2;
unsigned long FLEXIO_SHIFTCFG3;
unsigned long ulRes5[60];
unsigned long FLEXIO_SHIFTBUF0;
unsigned long FLEXIO_SHIFTBUF1;
unsigned long FLEXIO_SHIFTBUF2;
unsigned long FLEXIO_SHIFTBUF3;
unsigned long ulRes6[28];
unsigned long FLEXIO_SHIFTBUFBIS0;
unsigned long FLEXIO_SHIFTBUFBIS1;
unsigned long FLEXIO_SHIFTBUFBIS2;
unsigned long FLEXIO_SHIFTBUFBIS3;
unsigned long ulRes7[28];
unsigned long FLEXIO_SHIFTBUFBYS0;
unsigned long FLEXIO_SHIFTBUFBYS1;
unsigned long FLEXIO_SHIFTBUFBYS2;
unsigned long FLEXIO_SHIFTBUFBYS3;
unsigned long ulRes8[28];
unsigned long FLEXIO_SHIFTBUFBBS0;
unsigned long FLEXIO_SHIFTBUFBBS1;
unsigned long FLEXIO_SHIFTBUFBBS2;
unsigned long FLEXIO_SHIFTBUFBBS3;
unsigned long ulRes9[28];
unsigned long FLEXIO_TIMCTL0;
unsigned long FLEXIO_TIMCTL1;
unsigned long FLEXIO_TIMCTL2;
unsigned long FLEXIO_TIMCTL3;
unsigned long ulRes10[28];
unsigned long FLEXIO_TIMCFG0;
unsigned long FLEXIO_TIMCFG1;
unsigned long FLEXIO_TIMCFG2;
unsigned long FLEXIO_TIMCFG3;
unsigned long ulRes11[28];
unsigned long FLEXIO_TIMCMP0;
unsigned long FLEXIO_TIMCMP1;
unsigned long FLEXIO_TIMCMP2;
unsigned long FLEXIO_TIMCMP3;
} KINETIS_FLEXIO;
#endif

#if defined LTC_AVAILABLE
typedef struct stKINETIS_LTC
{
unsigned long LTC0_MD_MDPK;
unsigned long ulRes0;
unsigned long LTC0_KS;
unsigned long LTC0_DS;
unsigned long ulRes1;
unsigned long LTC0_ICVS;
unsigned long ulRes2[5];
unsigned long LTC0_COM;
unsigned long LTC0_CTL;
unsigned long ulRes3[2];
unsigned long LTC0_CW;
unsigned long ulRes4;
unsigned long LTC0_STA;
unsigned long LTC0_ESTA;
unsigned long ulRes5[2];
unsigned long LTC0_AADSZ;
unsigned long ulRes6;
unsigned long LTC0_IVS;
unsigned long ulRes7;
unsigned long LTC0_DPAMS;
unsigned long ulRes8[5];
unsigned long LTC0_PKASZ;
unsigned long ulRes9;
unsigned long LTC0_PKBSZ;
unsigned long ulRes10;
unsigned long LTC0_PKNSZ;
unsigned long ulRes11;
unsigned long LTC0_PKESZ;
unsigned long ulRes12[26];
unsigned long LTC0_CTX_0;
unsigned long LTC0_CTX_1;
unsigned long LTC0_CTX_2;
unsigned long LTC0_CTX_3;
unsigned long LTC0_CTX_4;
unsigned long LTC0_CTX_5;
unsigned long LTC0_CTX_6;
unsigned long LTC0_CTX_7;
unsigned long LTC0_CTX_8;
unsigned long LTC0_CTX_9;
unsigned long LTC0_CTX_10;
unsigned long LTC0_CTX_11;
unsigned long LTC0_CTX_12;
unsigned long LTC0_CTX_13;
unsigned long LTC0_CTX_14;
unsigned long LTC0_CTX_15;
unsigned long ulRes13[49];
unsigned long LTC0_KEY_0;
unsigned long LTC0_KEY_1;
unsigned long LTC0_KEY_2;
unsigned long LTC0_KEY_3;
unsigned long LTC0_KEY_4;
unsigned long LTC0_KEY_5;
unsigned long LTC0_KEY_6;
unsigned long LTC0_KEY_7;
unsigned long ulRes14[361];
unsigned long LTC0_FIFOSTA;
unsigned long ulRes15[8];
unsigned long LTC0_IFIFO;
unsigned long ulRes16[4];
unsigned long LTC0_OFIFO;
} KINETIS_LTC;
#endif

typedef struct st_iMX_PERIPH
{
    iMX_DCDC              DCDC;                                          // DCDC block
#if defined iMX_RT1011
    iMX_OTFAD             OTFAD;                                         // OTFAD block
#else
    iMX_BEE               BEE;                                           // bus encryption engine
#endif
    iMX_XBARA             XBARA;                                         // XBARA block
#if XBARA_AVAILABLE > 1
    iMX_XBARB             XBARB[XBARA_AVAILABLE - 1];                    // XBARB block
#endif
    iMX_AOI               AOI;                                           // And-Or-Inverter block
    iMX_IOMUXC_SNVS_GPR   IOMUXC_SNVS_GPR;                               // IOMUX controller SNVS GPR
    iMX_IOMUXC_SNVS       IOMUXC_SNVS;                                   // IOMUX controller SNVS
    iMX_IOMUXC            IOMUXC;                                        // IOMUX controller
    iMX_IOMUXC_SW         IOMUXC_SW;                                     // IOMUXC SW control
#if defined SEMC_AVAILABLE
    iMX_SEMC              SEMC;                                          // SEMC
#endif
    iMX_GPIO              GPIO[5];                                       // GPIO (warning - these are not contiguous in the physically memory map)
    iMX_OCOTP             OCOTP;                                         // on - chip OTP controller
    iMX_LPI2C             LPI2C[LPI2C_AVAILABLE];                        // LPI2C
    iMX_LPUART            LPUART[LPUARTS_AVAILABLE];                     // LPUART
    iMX_LPSPI             LPSPI[LPSPI_AVAILABLE];                        // LPSPI
    _iMX_FLEX_SPI         FLEX_SPI[FLEX_SPI_INTERFACES];                 // FLEX SPI
    iMX_SRC               SRC;                                           // system reset controller
    iMX_GPT               GPT[GPT_AVAILABLE];                            // general purpose timers
    iMX_FLEXPWM           FLEXPWM[FLEXPWM_AVAILABLE];                    // enhanced FlexPWM
    iMX_CCM               CCM;                                           // clock control module
    iMX_SNVS              SNVS;                                          // secure non-volatile storage module
    iMX_CCM_ANALOG        CCM_ANALOG;                                    // clock control module analog
    _iMX_ADC              ADC[ADC_CONTROLLERS];                          // ADC
    iMX_USBHS             USBHS[HSUSB_CONTROLLERS];                      // HSUSB
    iMX_USBPHY            USBHSPHY[HSUSB_CONTROLLERS];                   // USB PHY
    iMX_USBNC             USBHSNON_CORE;                                 // USB OTG non-core registers
#if defined TRUE_RANDOM_NUMBER_GENERATOR                                 // true random number generator
    iMX_TRNG              TRNG;
#endif
#if (NUMBER_OF_CAN_INTERFACES > 0)
    iMX_CAN               CAN[NUMBER_OF_CAN_INTERFACES];
#endif
#if NUMBER_OF_SDHC > 0
    iMX_SDHC              SDHC[NUMBER_OF_SDHC];                          // SDHC
#endif
#if defined ETHERNET_AVAILABLE
    iMX_EMAC              EMAC[ETHERNET_COUNT];                           // ethernet controller
#endif
    iMX_eDMA              eDMA;
    iMX_eDMADES           eDMADES;
#if !defined IMX_WITHOUT_PIT
    iMX_PIT               PIT;                                           // PITs
#endif
    iMX_WDOG              WDOG[2];
    iMX_WDOG3             WDOG3;
#if QUAD_TIMER_AVAILABLE > 0
    iMX_QUAD_TIMER        TMR[QUAD_TIMER_AVAILABLE];                     // Quad timers
#endif
#if defined ENHANCED_LCD_INTERFACE
    iMX_LCD               LCD;                                           // enhanced LCD controller (eLCDIF)
#endif
    iMX_CORTEX_M7         CORTEX_M7;
#if defined ARM_MATH_CM4 || defined ARM_MATH_CM7
    iMX_CORTEX_M7_DEBUG   CORTEX_M7_DEBUG;
    iMX_CORTEX_M7_TRACE   CORTEX_M7_TRACE;
#endif
    iMX_CORTEX_M7_REGS    CORTEX_M7_REGS;                                // only for simulation use
} iMX_PERIPH;


typedef struct stKINETIS_PERIPH
{
#if !defined KINETIS_KL && !defined KINETIS_KE
    KINETIS_FB         FB;
#endif
#if defined KINETIS_KL || defined KINETIS_KE
    #if defined KINETIS_KE
    KINETIS_KBI        KBI[2];
    #endif
#else
    KINETIC_CRC        CRC;
#endif
#if defined KINETIS_KM
    KINETIS_DMAMUX     DMAMUX[4];
#elif defined DMAMUX1_AVAILABLE
    KINETIS_DMAMUX     DMAMUX[2];
#elif !defined DEVICE_WITHOUT_DMA
    KINETIS_DMAMUX     DMAMUX[1];
#endif
#if I2S_AVAILABLE > 0
    KINETIS_I2S        I2S_SAI[I2S_AVAILABLE];
#endif
#if FLEX_TIMERS_AVAILABLE > 0
    KINETIS_FTM        FTM[FLEX_TIMERS_AVAILABLE];
#endif
    KINETIS_ADC        ADC0;
#if ADC_CONTROLLERS > 2
    KINETIS_ADC        ADC2;
#endif
#if !defined KINETIS_WITHOUT_RTC
    #if defined KINETIS_K80
    KINETIS_RTC_REGISTER_FILE     RTC_REGISTER_FILE;                     // RTC register file
    KINETIS_SYSTEM_REGISTER_FILE  SYSTEM_REGISTER_FILE;                  // system register file
    #endif
#endif


#if defined CHIP_HAS_FLEXIO
    KINETIS_FLEXIO     FLEXIO;
#endif
#if defined LTC_AVAILABLE
    KINETIS_LTC        LTC;
#endif
    KINETIS_OSC        OSC[2];
#if I2C_AVAILABLE > 0
    KINETIS_I2C        I2C[I2C_AVAILABLE];
#endif
    KINETIS_USB        USB;
#if defined KINETIS_KE
    KINETIS_ACMP       ACMP[2];
#elif NUMBER_OF_COMPARATORS != 0
    KINETIS_CMP        CMP[NUMBER_OF_COMPARATORS];
#endif
#if !defined KINETIS_KL
    KINETIS_VREF       VREF;
#endif
#if !defined KINETIS_KL
    KINETIS_ADC        ADC1;
#endif
#if ADC_CONTROLLERS > 3
    KINETIS_ADC        ADC3;
#endif
#if DAC_CONTROLLERS > 0
    KINETIS_DAC        DAC[DAC_CONTROLLERS];
#endif
} KINETIS_PERIPH;


extern iMX_PERIPH  iMX;
extern KINETIS_PERIPH kinetis;

#define _PORTS_AVAILABLE_   PORTS_AVAILABLE

#define _PORT1                  0
#define _PORT2                  1
#define _PORT3                  2
#define _PORT4                  3
#define _PORT5                  4
#define _PORT6                  5
#define _PORT7                  6
#define _PORT8                  7
#define _PORT9                  8

#define _GPIO_ADC               (_PORTS_AVAILABLE_)                      // dedicated ADC pins

#define _PORT_EXT_0             (_PORTS_AVAILABLE_ + 1)
#define _PORT_EXT_1             (_PORTS_AVAILABLE_ + 2)                  // these port numbers should be in order following the internal ports and a fictive ADC port
#define _PORT_EXT_2             (_PORTS_AVAILABLE_ + 3)
#define _PORT_EXT_3             (_PORTS_AVAILABLE_ + 4)
#define _PORT_EXT_4             (_PORTS_AVAILABLE_ + 5)
#define _PORT_EXT_5             (_PORTS_AVAILABLE_ + 6)
#define _PORT_EXT_6             (_PORTS_AVAILABLE_ + 7)
#define _PORT_EXT_7             (_PORTS_AVAILABLE_ + 8)


// Simuation references for specific peripherals
//
#define SIMULATION_SOURCE_GPT         0x10000000
#define SIMULATION_SOURCE_FLEX_PWM    0x20000000

#define SIMULATION_SOURCE_GPT1        (SIMULATION_SOURCE_GPT | 0x01000000)
#define SIMULATION_SOURCE_GPT2        (SIMULATION_SOURCE_GPT | 0x02000000)
#define SIMULATION_SOURCE_FLEX_PWM_1  (SIMULATION_SOURCE_FLEX_PWM | 0x03000000)
#define SIMULATION_SOURCE_FLEX_PWM_2  (SIMULATION_SOURCE_FLEX_PWM | 0x04000000)
#define SIMULATION_SOURCE_FLEX_PWM_3  (SIMULATION_SOURCE_FLEX_PWM | 0x05000000)
#define SIMULATION_SOURCE_FLEX_PWM_4  (SIMULATION_SOURCE_FLEX_PWM | 0x06000000)

#define SIMULATION_SOURCE_CAPTURE     0x00100000
#define SIMULATION_SOURCE_COMPARE     0x00200000
#define SIMULATION_SOURCE_CLOCK       0x00300000

#define SIMULATION_CHANNEL_MASK       0x0000000f
#define SIMULATION_EVENT_MASK         0x00f00000
#define SIMULATION_SOURCE_MASK        0x0f000000
#define SIMULATION_SOURCE_SHIFT       24
#define SIMULATION_TIMER_MASK         0xf0000000

#define SIMULATE_PWM_A                0x00010000
#define SIMULATE_PWM_B                0x00020000
#define SIMULATE_PWM_X                0x00030000

#define iMX_GPT_1_CAPTURE_1           (SIMULATION_SOURCE_GPT1 | SIMULATION_SOURCE_CAPTURE | 0)
#define iMX_GPT_1_CAPTURE_2           (SIMULATION_SOURCE_GPT1 | SIMULATION_SOURCE_CAPTURE | 1)
#define iMX_GPT_2_CAPTURE_1           (SIMULATION_SOURCE_GPT2 | SIMULATION_SOURCE_CAPTURE | 0)
#define iMX_GPT_2_CAPTURE_2           (SIMULATION_SOURCE_GPT2 | SIMULATION_SOURCE_CAPTURE | 1)

#define iMX_GPT_1_CLOCK               (SIMULATION_SOURCE_GPT1 | SIMULATION_SOURCE_CLOCK)
#define iMX_GPT_2_CLOCK               (SIMULATION_SOURCE_GPT2 | SIMULATION_SOURCE_CLOCK)

#define iMX_FLEXPWM_1_0_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 0)
#define iMX_FLEXPWM_1_1_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 1)
#define iMX_FLEXPWM_1_2_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 2)
#define iMX_FLEXPWM_1_3_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 3)

#define iMX_FLEXPWM_1_0_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 0)
#define iMX_FLEXPWM_1_1_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 1)
#define iMX_FLEXPWM_1_2_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 2)
#define iMX_FLEXPWM_1_3_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 3)

#define iMX_FLEXPWM_1_0_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 0)
#define iMX_FLEXPWM_1_1_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 1)
#define iMX_FLEXPWM_1_2_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 2)
#define iMX_FLEXPWM_1_3_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_1 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 3)

#define iMX_FLEXPWM_2_0_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 0)
#define iMX_FLEXPWM_2_1_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 1)
#define iMX_FLEXPWM_2_2_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 2)
#define iMX_FLEXPWM_2_3_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 3)

#define iMX_FLEXPWM_2_0_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 0)
#define iMX_FLEXPWM_2_1_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 1)
#define iMX_FLEXPWM_2_2_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 2)
#define iMX_FLEXPWM_2_3_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 3)

#define iMX_FLEXPWM_2_0_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 0)
#define iMX_FLEXPWM_2_1_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 1)
#define iMX_FLEXPWM_2_2_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 2)
#define iMX_FLEXPWM_2_3_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_2 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 3)

#define iMX_FLEXPWM_3_0_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 0)
#define iMX_FLEXPWM_3_1_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 1)
#define iMX_FLEXPWM_3_2_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 2)
#define iMX_FLEXPWM_3_3_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 3)

#define iMX_FLEXPWM_3_0_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 0)
#define iMX_FLEXPWM_3_1_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 1)
#define iMX_FLEXPWM_3_2_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 2)
#define iMX_FLEXPWM_3_3_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 3)

#define iMX_FLEXPWM_3_0_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 0)
#define iMX_FLEXPWM_3_1_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 1)
#define iMX_FLEXPWM_3_2_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 2)
#define iMX_FLEXPWM_3_3_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_3 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 3)

#define iMX_FLEXPWM_4_0_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 0)
#define iMX_FLEXPWM_4_1_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 1)
#define iMX_FLEXPWM_4_2_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 2)
#define iMX_FLEXPWM_4_3_A_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_A | 3)

#define iMX_FLEXPWM_4_0_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 0)
#define iMX_FLEXPWM_4_1_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 1)
#define iMX_FLEXPWM_4_2_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 2)
#define iMX_FLEXPWM_4_3_B_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_B | 3)

#define iMX_FLEXPWM_4_0_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 0)
#define iMX_FLEXPWM_4_1_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 1)
#define iMX_FLEXPWM_4_2_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 2)
#define iMX_FLEXPWM_4_3_X_OUTPUT      (SIMULATION_SOURCE_FLEX_PWM_4 | SIMULATION_SOURCE_COMPARE | SIMULATE_PWM_X | 3)

/*
#define KINETIS_PERIPHERAL_ADC0 0
#define KINETIS_PERIPHERAL_ADC1 1
#define KINETIS_PERIPHERAL_ADC2 2
#define KINETIS_PERIPHERAL_ADC3 3
#define KINETIS_PERIPHERAL_FTM0 4
#define KINETIS_PERIPHERAL_FTM1 5
#define KINETIS_PERIPHERAL_FTM2 6
#define KINETIS_PERIPHERAL_FTM3 7*/
