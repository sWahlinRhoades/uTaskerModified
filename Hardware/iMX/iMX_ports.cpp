/************************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      iMX_ports.cpp
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#include "config.h"

#if defined _iMX

#define BUF1SIZE 200

#if _VC80_UPGRADE >= 0x0600
    #define STRCPY(a, b) strcpy_s (a, BUF1SIZE, b)
    #define SPRINTF(a, b, c) sprintf_s(a, BUF1SIZE, b, c)
    #define STRCAT(a, b) strcat_s(a, BUF1SIZE, b)
#else
    #define STRCPY strcpy
    #define SPRINTF sprintf
    #define STRCAT strcat
#endif

#include "iMX_pinout.h"

#if defined FAST_GPIO_AVAILABLE
    extern "C" int fnGetFastport(int iPortNumber, unsigned long ulPortBit);
#endif

#if defined SUPPORT_ADC 
    static void fnAddVoltage(int iPort, char *cPortDetails, int iBit);
#endif
#if defined SUPPORT_PWM_MODULE 
    static void fnAddPWM(int iPort, char *cPortDetails, int iBit);
#endif

static unsigned char *_ptrPerFunctions;
#if defined SUPPORT_ADC                                                  // {2}
    typedef struct stPorts
    {
        unsigned char  *ports;
        unsigned short *adcs;
    } PORTS;

    static unsigned short *_ptrADC;
#endif

static const char *pads[] = {
#if defined iMX_RT1011
    "GPIO_00",                                                     // GPIO1
    "GPIO_01",
    "GPIO_02",
    "GPIO_03",
    "GPIO_04",
    "GPIO_05",
    "GPIO_06",
    "GPIO_07",
    "GPIO_08",
    "GPIO_09",
    "GPIO_10",
    "GPIO_11",
    "GPIO_12",
    "GPIO_13",
    "GPIO_AD_00",
    "GPIO_AD_01",
    "GPIO_AD_02",
    "GPIO_AD_03",
    "GPIO_AD_04",
    "GPIO_AD_05",
    "GPIO_AD_06",
    "GPIO_AD_07",
    "GPIO_AD_08",
    "GPIO_AD_09",
    "GPIO_AD_10",
    "GPIO_AD_11",
    "GPIO_AD_12",
    "GPIO_AD_13",
    "GPIO_AD_14",
    0,0,0,
#else
    "GPIO_AD_B0_00",                                                     // GPIO1
    "GPIO_AD_B0_01",
    "GPIO_AD_B0_02",
    "GPIO_AD_B0_03",
    "GPIO_AD_B0_04",
    "GPIO_AD_B0_05",
    "GPIO_AD_B0_06",
    "GPIO_AD_B0_07",
    "GPIO_AD_B0_08",
    "GPIO_AD_B0_09",
    "GPIO_AD_B0_10",
    "GPIO_AD_B0_11",
    "GPIO_AD_B0_12",
    "GPIO_AD_B0_13",
    "GPIO_AD_B0_14",
    "GPIO_AD_B0_15",
    "GPIO_AD_B1_00",
    "GPIO_AD_B1_01",
    "GPIO_AD_B1_02",
    "GPIO_AD_B1_03",
    "GPIO_AD_B1_04",
    "GPIO_AD_B1_05",
    "GPIO_AD_B1_06",
    "GPIO_AD_B1_07",
    "GPIO_AD_B1_08",
    "GPIO_AD_B1_09",
    "GPIO_AD_B1_10",
    "GPIO_AD_B1_11",
    "GPIO_AD_B1_12",
    "GPIO_AD_B1_13",
    "GPIO_AD_B1_14",
    "GPIO_AD_B1_15",
#endif
#if defined iMX_RT1011
    "GPIO_SD_00",                                                     // GPIO2
    "GPIO_SD_01",
    "GPIO_SD_02",
    "GPIO_SD_03",
    "GPIO_SD_04",
    "GPIO_SD_05",
    "GPIO_SD_06",
    "GPIO_SD_07",
    "GPIO_SD_08",
    "GPIO_SD_09",
    "GPIO_SD_10",
    "GPIO_SD_11",
    "GPIO_SD_12",
    "GPIO_SD_13",
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#elif defined iMX_RT105X || defined iMX_RT106X
    "GPIO_B0_00",                                                     // GPIO2
    "GPIO_B0_01",
    "GPIO_B0_02",
    "GPIO_B0_03",
    "GPIO_B0_04",
    "GPIO_B0_05",
    "GPIO_B0_06",
    "GPIO_B0_07",
    "GPIO_B0_08",
    "GPIO_B0_09",
    "GPIO_B0_10",
    "GPIO_B0_11",
    "GPIO_B0_12",
    "GPIO_B0_13",
    "GPIO_B0_14",
    "GPIO_B0_15",
    "GPIO_B1_00",
    "GPIO_B1_01",
    "GPIO_B1_02",
    "GPIO_B1_03",
    "GPIO_B1_04",
    "GPIO_B1_05",
    "GPIO_B1_06",
    "GPIO_B1_07",
    "GPIO_B1_08",
    "GPIO_B1_09",
    "GPIO_B1_10",
    "GPIO_B1_11",
    "GPIO_B1_12",
    "GPIO_B1_13",
    "GPIO_B1_14",
    "GPIO_B1_15",
#else
    "GPIO_EMC_00",                                                       // GPIO2
    "GPIO_EMC_01",
    "GPIO_EMC_02",
    "GPIO_EMC_03",
    "GPIO_EMC_04",
    "GPIO_EMC_05",
    "GPIO_EMC_06",
    "GPIO_EMC_07",
    "GPIO_EMC_08",
    "GPIO_EMC_09",
    "GPIO_EMC_10",
    "GPIO_EMC_11",
    "GPIO_EMC_12",
    "GPIO_EMC_13",
    "GPIO_EMC_14",
    "GPIO_EMC_15",
    "GPIO_EMC_16",
    "GPIO_EMC_17",
    "GPIO_EMC_18",
    "GPIO_EMC_19",
    "GPIO_EMC_20",
    "GPIO_EMC_21",
    "GPIO_EMC_22",
    "GPIO_EMC_23",
    "GPIO_EMC_24",
    "GPIO_EMC_25",
    "GPIO_EMC_26",
    "GPIO_EMC_27",
    "GPIO_EMC_28",
    "GPIO_EMC_29",
    "GPIO_EMC_30",
    "GPIO_EMC_31",
#endif
#if defined iMX_RT1011
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     // GPIO3
#elif defined iMX_RT105X || defined iMX_RT106X
        "GPIO_SD_B1_00",                                                 // GPIO3
        "GPIO_SD_B1_01",
        "GPIO_SD_B1_02",
        "GPIO_SD_B1_03",
        "GPIO_SD_B1_04",
        "GPIO_SD_B1_05",
        "GPIO_SD_B1_06",
        "GPIO_SD_B1_07",
        "GPIO_SD_B1_08",
        "GPIO_SD_B1_09",
        "GPIO_SD_B1_10",
        "GPIO_SD_B1_11",
        "GPIO_SD_B0_00",
        "GPIO_SD_B0_01",
        "GPIO_SD_B0_02",
        "GPIO_SD_B0_03",
        "GPIO_SD_B0_04",
        "GPIO_SD_B0_05",
        "GPIO_EMC_32",
        "GPIO_EMC_33",
        "GPIO_EMC_34",
        "GPIO_EMC_35",
        "GPIO_EMC_36",
        "GPIO_EMC_37",
        "GPIO_EMC_38",
        "GPIO_EMC_39",
        "GPIO_EMC_40",
        "GPIO_EMC_41",
        "NONE",
        "NONE",
        "NONE",
        "NONE",
#else
    "GPIO_EMC_32",                                                       // GPIO3
    "GPIO_EMC_33",
    "GPIO_EMC_34",
    "GPIO_EMC_35",
    "GPIO_EMC_36",
    "GPIO_EMC_37",
    "GPIO_EMC_38",
    "GPIO_EMC_39",
    "GPIO_EMC_40",
    "GPIO_EMC_41",
    "GPIO_EMC_42",
    "GPIO_EMC_43",
    "GPIO_EMC_44",
    "GPIO_SD_B0_00",
    "GPIO_SD_B0_01",
    "GPIO_SD_B0_02",
    "GPIO_SD_B0_03",
    "GPIO_SD_B0_04",
    "GPIO_SD_B0_05",
    "GPIO_SD_B0_06",
    "GPIO_SD_B1_00",
    "GPIO_SD_B1_01",
    "GPIO_SD_B1_02",
    "GPIO_SD_B1_03",
    "GPIO_SD_B1_04",
    "GPIO_SD_B1_05",
    "GPIO_SD_B1_06",
    "GPIO_SD_B1_07",
    "GPIO_SD_B1_08",
    "GPIO_SD_B1_09",
    "GPIO_SD_B1_10",
    "GPIO_SD_B1_11",
#endif
#if defined iMX_RT105X || defined iMX_RT106X
    "GPIO_EMC_00",                                                       // GPIO4
    "GPIO_EMC_01",
    "GPIO_EMC_02",
    "GPIO_EMC_03",
    "GPIO_EMC_04",
    "GPIO_EMC_05",
    "GPIO_EMC_06",
    "GPIO_EMC_07",
    "GPIO_EMC_08",
    "GPIO_EMC_09",
    "GPIO_EMC_10",
    "GPIO_EMC_11",
    "GPIO_EMC_12",
    "GPIO_EMC_13",
    "GPIO_EMC_14",
    "GPIO_EMC_15",
    "GPIO_EMC_16",
    "GPIO_EMC_17",
    "GPIO_EMC_18",
    "GPIO_EMC_19",
    "GPIO_EMC_20",
    "GPIO_EMC_21",
    "GPIO_EMC_22",
    "GPIO_EMC_23",
    "GPIO_EMC_24",
    "GPIO_EMC_25",
    "GPIO_EMC_26",
    "GPIO_EMC_27",
    "GPIO_EMC_28",
    "GPIO_EMC_29",
    "GPIO_EMC_30",
    "GPIO_EMC_31",
#else
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     // GPIO4
#endif
#if defined iMX_RT1011
    "SNVS_LP_PMIC_ON_REQ",
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#else
    "WAKEUP",                                                            // GPIO5
    "PMIC_ON_REQ",
    "PMIC_STBY_REQ",
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#endif
};

extern void fnSetPortDetails(char *cPortDetails, int iPort, int iBit, unsigned long *ulPortStates, unsigned long *ulPortFunction, unsigned long *ulPortPeripheral, int iMaxLength)
{
    int i;
    char *ptrBuf = cPortDetails;
    int iLength = 0;
    int iPortLength = PORT_WIDTH;
    char cBuf[BUF1SIZE + 1];
    int iPinPad = 0;
    #if defined FAST_GPIO_AVAILABLE
    int iFastPort;
    #endif
    iBit = (iPortLength - iBit - 1);                                     // bit position
    #if defined FAST_GPIO_AVAILABLE
    iFastPort = fnGetFastport(iPort, (unsigned long)(1 << iBit));
    #endif
    if (iBit < 0) {
        return;
    }
    
    switch (iPort) {
    case _PORT1:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort != iPort) {
            STRCPY(cPortDetails, "Muxed to Fast GPIO6");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO1");
        break;
    case _PORT2:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort != iPort) {
            STRCPY(cPortDetails, "Muxed to Fast GPIO7");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO2");
        iPinPad = 32;
        break;
#if (PORTS_AVAILABLE > 2)
    case _PORT3:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort != iPort) {
            STRCPY(cPortDetails, "Muxed to Fast GPIO8");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO3");
        iPinPad = 64;
        break;
    case _PORT4:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort != iPort) {
            STRCPY(cPortDetails, "Muxed to Fast GPIO9");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO4");
        iPinPad = 96;
        break;
    case _PORT5:
        STRCPY(cPortDetails, "GPIO5");
        iPinPad = 128;
        break;
#endif
#if defined PORT6
    case _PORT6:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort == iPort) {
            STRCPY(cPortDetails, "Not muxed");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO6");
        iPinPad = 0;
        break;
#endif
#if defined PORT7
    case _PORT7:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort == iPort) {
            STRCPY(cPortDetails, "Not muxed");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO7");
        iPinPad = 32;
        break;
#endif
#if defined PORT8
    case _PORT8:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort == iPort) {
            STRCPY(cPortDetails, "Not muxed");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO8");
        iPinPad = 64;
        break;
#endif
#if defined PORT9
    case _PORT9:
    #if defined FAST_GPIO_AVAILABLE
        if (iFastPort == iPort) {
            STRCPY(cPortDetails, "Not muxed");
            return;
        }
    #endif
        STRCPY(cPortDetails, "GPIO9");
        iPinPad = 96;
        break;
#endif
    case (PORTS_AVAILABLE):
        STRCPY(cPortDetails, "Analog: ");
    #if defined _PIN_COUNT
        if (*cPinNumber[iPort][iBit][_PIN_COUNT] == '-') {
            return;
        }
        STRCAT(cPortDetails, "Pin: ");
        STRCAT(cPortDetails, cPinNumber[iPort][iBit][_PIN_COUNT]);
        STRCAT(cPortDetails, " - ");
    #endif
    #if defined _DEDICATED_PINS
        if (cDedicated[iBit] != 0) {
            STRCAT(cPortDetails, cDedicated[iBit]);
        }
    #endif
    #if defined SUPPORT_ADC 
        fnAddVoltage(_GPIO_ADC, cPortDetails, iBit);                     // display the voltage applied to ADC inputs
    #endif
        return;
    default:
        return;
    }
    #if defined FAST_GPIO_AVAILABLE
    iPort = iFastPort;
    #endif
    SPRINTF(cBuf, " Bit %i ", iBit);
    STRCAT(cPortDetails, cBuf);
    iPinPad += iBit;
    if (pads[iPinPad] != 0) {
        SPRINTF(cBuf, "[%s]", pads[iPinPad]);
        STRCAT(cPortDetails, cBuf);
    }
    STRCAT(cPortDetails, " Pin: ");
    if (*cPinNumber[iPort][iBit][_PIN_COUNT] == '-') {
        return;
    }
    STRCPY(cBuf, cPinNumber[iPort][iBit][_PIN_COUNT]);
    STRCAT(cPortDetails, cBuf);
    STRCAT(cPortDetails, " {");
    STRCAT(cPortDetails, cPer[iPort][iBit][0]);

    for (i = 1; i <= ALTERNATIVE_FUNCTIONS; i++) {
        if (*cPer[iPort][iBit][i] != '-') {
            STRCAT(cPortDetails, "/");
            STRCAT(cPortDetails, cPer[iPort][iBit][i]);
        }
    }
    STRCAT(cPortDetails, "} ");

    if ((ulPortPeripheral[iPort] & (0x01 << iBit)) != 0) {               // if programmed as peripheral function
        unsigned char *ptrList = _ptrPerFunctions;
        int _iPort = iPort;
        int _iBit = iBit;
        while (_iPort-- != 0) {
            ptrList += (PORT_WIDTH);
        }
        while (_iBit-- != 0) {
            ptrList++;
        }
        if (*ptrList > ALTERNATIVE_FUNCTIONS) {
            STRCPY(&cBuf[0], "??");
        }
        else {
            if (*cPer[iPort][iBit][*ptrList] == '-') {
                STRCPY(&cBuf[0], "INVALID");
            }
            else {
                STRCPY(&cBuf[0], cPer[iPort][iBit][*ptrList]);
            }
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
#if defined SUPPORT_ADC 
    fnAddVoltage(iPort, cPortDetails, iBit);                             // display the voltage applied to ADC inputs
#endif
#if defined SUPPORT_PWM_MODULE 
    fnAddPWM(iPort, cPortDetails, iBit);                                 // display PWM output details
#endif
}

extern unsigned long fnGetPortMask(int iPortNumber)
{
    unsigned long ulPortMask = 0x00000000;
    unsigned long ulMaskBit = 0x00000001;
    int i;
    #if defined FAST_GPIO_AVAILABLE
    int iMappedPort;
    #endif
    if (iPortNumber >= (PORTS_AVAILABLE + 1)) {                          // handle external port mask
    #if defined _EXT_PORT_32_BIT
        return 0;
    #elif defined _EXT_PORT_28_BIT
        return 0xf0000000;
    #elif defined _EXT_PORT_16_BIT
        return 0xffff0000;
    #else
        return 0xffffff00;                                               // 8 bit external ports
    #endif
    }
    for (i = 0; i < PORT_WIDTH; i++) {
    #if defined FAST_GPIO_AVAILABLE
        iMappedPort = fnGetFastport(iPortNumber, (unsigned long)(1 << i));
        if (*cPinNumber[iMappedPort][i][_PIN_COUNT] == '-') {
            ulPortMask |= ulMaskBit;
        }
    #else
        if (*cPinNumber[iPortNumber][i][_PIN_COUNT] == '-') {
            ulPortMask |= ulMaskBit;
        }
    #endif
        ulMaskBit <<= 1;
    }
    #if defined iMX_RT105X || defined iMX_RT106X
    if (iPortNumber == PORT3) {
        ulPortMask |= 0xf0000000;
    }
        #if defined PORT8
    else if (iPortNumber == PORT8) {
        ulPortMask |= 0xf0000000;
    }
        #endif
    #endif
    return ulPortMask;
} 

extern "C" int fnGetADC_sim_channel(int iPort, int iBit)
{
#if defined _PIN_COUNT
    if (iPort == _GPIO_ADC) {                                            // dedicated analog
        if (ADC_DEDICATED_MODULE[iBit] == 0) {                           // not assigned
            return -1;                                                   // not ADC function
        }
        return (((ADC_DEDICATED_MODULE[iBit] - 1) * 32) + ADC_DEDICATED_CHANNEL[iBit]);
    }
    else if (iPort > _GPIO_ADC) {                                        // extended port - not expected to have ADC
        return -1;
    }
    else {                                                               // multiplexed port
    #if defined KINETIS_KE && !defined KINETIS_KE15
        if (ADC_MUX_CHANNEL[iPort][7 - iBit] == -1) {
            return -1;                                                   // not ADC function
        }
    #else
        return ADC_MUX_CHANNEL[iPort][31 - iBit];                        // return -1 if not ADC, otherwise the ADC channel
    #endif
    }
#endif
    return 0;
}

#if defined SUPPORT_ADC 
static void fnAddVoltage(int iPort, char *cPortDetails, int iBit)
{
    #if defined _PIN_COUNT
    char cBuf[BUF1SIZE];
        #if defined KINETIS_KE && !defined KINETIS_KE15
    signed int iAdc = fnGetADC_sim_channel(iPort, (7 - iBit));
        #else
    signed int iAdc = fnGetADC_sim_channel(iPort, (31 - iBit));
        #endif
    if (iAdc < 0) {
        return;
    }
    if (iAdc < (32 * ADC_CONTROLLERS)) {                                 // {6}
        SPRINTF(cBuf, " [%fV]", (((float)_ptrADC[iAdc]*((float)ADC_REFERENCE_VOLTAGE/(float)1000))/(float)0xffff));
        STRCAT(cPortDetails, cBuf);
    }
    #endif
}
#endif
#if defined SUPPORT_PWM_MODULE
extern "C" int fnGetPWM_sim_channel(int iPort, int iPin, unsigned long *ptr_ulFrequency, unsigned char *ptr_ucMSR);
static void fnAddPWM(int iPort, char *cPortDetails, int iBit)
{
    #if defined _PIN_COUNT
    char cBuf[BUF1SIZE];
    unsigned long ulFrequency;
    unsigned char ucMSR;
    signed int iPWM = fnGetPWM_sim_channel(iPort, iBit, &ulFrequency, &ucMSR);
    if (iPWM < 0) {
        return;
    }
    SPRINTF(cBuf, " [F=%d Hz", ulFrequency);
    STRCAT(cPortDetails, cBuf);
    SPRINTF(cBuf, " MSR=%d %%]", ucMSR);
    STRCAT(cPortDetails, cBuf);
    #endif
}
#endif


// Allow the hardware simulator to pass a pointer to hardware details which are useful for simulating this type
//
extern "C" void fnEnterHW_table(void *hw_table)
{
    #if defined SUPPORT_ADC                                              // {2}
    PORTS *_ports = (PORTS *)hw_table;
    _ptrPerFunctions = _ports->ports;
    _ptrADC          = _ports->adcs;
    #else
    _ptrPerFunctions = (unsigned char *)hw_table;
    #endif
}
#endif
