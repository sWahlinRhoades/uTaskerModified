/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      iMXSim.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
 
*/  
                          

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"
#define _PIN_DEFAULTS
#include "iMX_pinout.h"

#if defined _iMX                                                         // only on iMX parts

#if defined CAN_INTERFACE && defined SIM_KOMODO
    #include "..\..\WinSim\Komodo\komodo.h" 
#endif

#define PROGRAM_ONCE_AREA_SIZE   64                                      // 64 bytes in 8 blocks of 8 bytes

iMX_PERIPH  iMX = {0};

KINETIS_PERIPH  kinetis = {0};

iMX_eFUSE iMX_OPT = {{0}};

unsigned char uninitialisedRAM[NON_INITIALISED_RAM_SIZE];

unsigned long vector_ram[(sizeof(VECTOR_TABLE))/sizeof(unsigned long)];  // long word aligned

#if defined _EXTERNAL_PORT_COUNT && _EXTERNAL_PORT_COUNT > 0
    extern unsigned long fnGetExtPortFunction(int iExtPortReference);
    extern unsigned long fnGetExtPortState(int iExtPortReference);
#endif

static unsigned long ulPort_in_1, ulPort_in_2, ulPort_in_3, ulPort_in_4, ulPort_in_5, ulPort_in_6, ulPort_in_7, ulPort_in_8, ulPort_in_9;
#if defined SERIAL_INTERFACE
    static int iUART_rx_Active[LPUARTS_AVAILABLE + UARTS_AVAILABLE] = {0};
#endif
static unsigned char ucPortFunctions[PORTS_AVAILABLE][PORT_WIDTH] = {0};
static unsigned long ulPeripherals[PORTS_AVAILABLE] = {0};
#if defined SUPPORT_ADC
    static unsigned short usADC_values[ADC_CONTROLLERS][32] = {{0}};
    static void *ptrPorts[2] = {(void *)ucPortFunctions, (void *)usADC_values};
#endif
#if defined RUN_IN_FREE_RTOS
    extern void fnExecutePendingInterrupts(int iRecursive);
#endif


static void fnPortInterrupt(int iPort, unsigned long ulNewState, unsigned long ulChangedBit);
extern int fnSimulateDMA(int channel, unsigned char ucTriggerSource);
#if defined CRYPTOGRAPHY
    static void fnDoOnTheFlyDecryption(unsigned char *ptrStart, unsigned char *ptrDestination, unsigned long ulLength, int iChannel);
#endif

static int iFlagRefresh = 0;

#if defined USB_INTERFACE
    #define _fnLE_add(x) (void *)x

    static unsigned char ucHSTxBuffer[HSUSB_CONTROLLERS][NUMBER_OF_USBHS_ENDPOINTS] = {{0}}; // monitor the high speed controller's transmission buffer use
    static unsigned long ulHSEndpointInt[HSUSB_CONTROLLERS] = {0};

    static unsigned char ucTxBuffer[HSUSB_CONTROLLERS][NUMBER_OF_USB_ENDPOINTS] = {{0}}; // monitor the controller's transmission buffer use
    static unsigned char ucRxBank[HSUSB_CONTROLLERS][NUMBER_OF_USB_ENDPOINTS] = {{0}}; // monitor the buffer to inject to
    #if defined USB_HOST_SUPPORT
    static int iData1Frame[HSUSB_CONTROLLERS][NUMBER_OF_USB_ENDPOINTS] = {{0}};
    #endif
#endif
#if defined KINETIS_KL
    static unsigned long ulCOPcounter = 0;
#endif
    #if defined KINETIS_KM
    #define PORT_CAST unsigned char
#else
    #define PORT_CAST unsigned long
#endif

extern void fnGetSW_GP2(unsigned char ucKey[16])
{
    ucKey[15] = (unsigned char)iMX_OPT.OCOTP_SW_GP20[0];
    ucKey[14] = (unsigned char)(iMX_OPT.OCOTP_SW_GP20[0] >> 8);
    ucKey[13] = (unsigned char)(iMX_OPT.OCOTP_SW_GP20[0] >> 16);
    ucKey[12] = (unsigned char)(iMX_OPT.OCOTP_SW_GP20[0] >> 24);
    ucKey[11] = (unsigned char)iMX_OPT.OCOTP_SW_GP21[0];
    ucKey[10] = (unsigned char)(iMX_OPT.OCOTP_SW_GP21[0] >> 8);
    ucKey[9] = (unsigned char)(iMX_OPT.OCOTP_SW_GP21[0] >> 16);
    ucKey[8] = (unsigned char)(iMX_OPT.OCOTP_SW_GP21[0] >> 24);
    ucKey[7] = (unsigned char)iMX_OPT.OCOTP_SW_GP22[0];
    ucKey[6] = (unsigned char)(iMX_OPT.OCOTP_SW_GP22[0] >> 8);
    ucKey[5] = (unsigned char)(iMX_OPT.OCOTP_SW_GP22[0] >> 16);
    ucKey[4] = (unsigned char)(iMX_OPT.OCOTP_SW_GP22[0] >> 24);
    ucKey[3] = (unsigned char)iMX_OPT.OCOTP_SW_GP23[0];
    ucKey[2] = (unsigned char)(iMX_OPT.OCOTP_SW_GP23[0] >> 8);
    ucKey[1] = (unsigned char)(iMX_OPT.OCOTP_SW_GP23[0] >> 16);
    ucKey[0] = (unsigned char)(iMX_OPT.OCOTP_SW_GP23[0] >> 24);
}

extern void fnProgramOTP(void)
{
    if ((HW_OCOTP_CTRL & HW_OCOTP_CTRL_WR_UNLOCK_MASK) == HW_OCOTP_CTRL_WR_UNLOCK) {
        switch (HW_OCOTP_CTRL & HW_OCOTP_CTRL_ADDR_MASK) {
        case SHADOW_REG_INDEX_LOCK:
            iMX_OPT.OCOTP_LOCK[3] |= HW_OCOTP_DATA;
            iMX_OPT.OCOTP_LOCK[0] = iMX_OPT.OCOTP_LOCK[1] = iMX_OPT.OCOTP_LOCK[2] = iMX_OPT.OCOTP_LOCK[3];
            break;
        case SHADOW_REG_INDEX_TEST_0:
        case SHADOW_REG_INDEX_TEST_1:
        case SHADOW_REG_INDEX_TEST_2:
        case SHADOW_REG_INDEX_TEST_3:
        case SHADOW_REG_INDEX_BOOT_CFG_0:
        case SHADOW_REG_INDEX_BOOT_CFG_1:
        case SHADOW_REG_INDEX_BOOT_CFG_2:
        case SHADOW_REG_INDEX_MEM_0:
        case SHADOW_REG_INDEX_MEM_1:
        case SHADOW_REG_INDEX_MEM_2:
        case SHADOW_REG_INDEX_MEM_3:
        case SHADOW_REG_INDEX_MEM_4:
        case SHADOW_REG_INDEX_ANALOG_0:
        case SHADOW_REG_INDEX_ANALOG_1:
        case SHADOW_REG_INDEX_ANALOG_2:
        case SHADOW_REG_INDEX_SRK_0:
        case SHADOW_REG_INDEX_SRK_1:
        case SHADOW_REG_INDEX_SRK_2:
        case SHADOW_REG_INDEX_SRK_3:
        case SHADOW_REG_INDEX_SRK_4:
        case SHADOW_REG_INDEX_SRK_5:
        case SHADOW_REG_INDEX_SRK_6:
        case SHADOW_REG_INDEX_SRK_7:
        case SHADOW_REG_INDEX_SJC_0:
        case SHADOW_REG_INDEX_SJC_1:
        case SHADOW_REG_INDEX_MAC_0:
        case SHADOW_REG_INDEX_MAC_1:
#if ETHERNET_COUNT > 1
        case SHADOW_REG_INDEX_MAC_2:
#else
        case SHADOW_REG_INDEX_GP3:
#endif
            break;
        case SHADOW_REG_INDEX_GP1:
        case SHADOW_REG_INDEX_GP2:
        case SHADOW_REG_INDEX_SW_GP_1:
            break;
        case SHADOW_REG_INDEX_SW_GP_20:
            if ((HW_OCOTP_LOCK & eFUSE_LOCK_SW_GP2_LOCK) == 0) {
                iMX_OPT.OCOTP_SW_GP20[3] |= HW_OCOTP_DATA;
                iMX_OPT.OCOTP_SW_GP20[0] = iMX_OPT.OCOTP_SW_GP20[1] = iMX_OPT.OCOTP_SW_GP20[2] = iMX_OPT.OCOTP_SW_GP20[3];
            }
            break;
        case SHADOW_REG_INDEX_SW_GP_21:
            if ((HW_OCOTP_LOCK & eFUSE_LOCK_SW_GP2_LOCK) == 0) {
                iMX_OPT.OCOTP_SW_GP21[3] |= HW_OCOTP_DATA;
                iMX_OPT.OCOTP_SW_GP21[0] = iMX_OPT.OCOTP_SW_GP21[1] = iMX_OPT.OCOTP_SW_GP21[2] = iMX_OPT.OCOTP_SW_GP21[3];
            }
            break;
        case SHADOW_REG_INDEX_SW_GP_22:
            if ((HW_OCOTP_LOCK & eFUSE_LOCK_SW_GP2_LOCK) == 0) {
                iMX_OPT.OCOTP_SW_GP22[3] |= HW_OCOTP_DATA;
                iMX_OPT.OCOTP_SW_GP22[0] = iMX_OPT.OCOTP_SW_GP22[1] = iMX_OPT.OCOTP_SW_GP22[2] = iMX_OPT.OCOTP_SW_GP22[3];
            }
            break;
        case SHADOW_REG_INDEX_SW_GP_23:
            if ((HW_OCOTP_LOCK & eFUSE_LOCK_SW_GP2_LOCK) == 0) {
                iMX_OPT.OCOTP_SW_GP23[3] |= HW_OCOTP_DATA;
                iMX_OPT.OCOTP_SW_GP23[0] = iMX_OPT.OCOTP_SW_GP23[1] = iMX_OPT.OCOTP_SW_GP23[2] = iMX_OPT.OCOTP_SW_GP23[3];
            }
            break;
        case SHADOW_REG_INDEX_MISC_CONF_0:
        case SHADOW_REG_INDEX_MISC_CONF_1:
        case SHADOW_REG_INDEX_SRK_REVOKE:
#if defined iMX_RT106X
        case SHADOW_REG_INDEX_ROM_PATCH_0:
        case SHADOW_REG_INDEX_ROM_PATCH_1:
        case SHADOW_REG_INDEX_ROM_PATCH_2:
        case SHADOW_REG_INDEX_ROM_PATCH_3:
        case SHADOW_REG_INDEX_ROM_PATCH_4:
        case SHADOW_REG_INDEX_ROM_PATCH_5:
        case SHADOW_REG_INDEX_ROM_PATCH_6:
        case SHADOW_REG_INDEX_ROM_PATCH_7:
        case SHADOW_REG_INDEX_GP3_0:
        case SHADOW_REG_INDEX_GP3_1:
        case SHADOW_REG_INDEX_GP3_2:
        case SHADOW_REG_INDEX_GP3_3:
        case SHADOW_REG_INDEX_GP4_0:
        case SHADOW_REG_INDEX_GP4_1:
        case SHADOW_REG_INDEX_GP4_2:
        case SHADOW_REG_INDEX_GP4_3:
#endif
            break;
        }
    }
}

extern void fnUpdateShadowRegisters(void)
{
    HW_OCOTP_LOCK = iMX_OPT.OCOTP_LOCK[0];
    HW_OCOTP_CFG0 = iMX_OPT.OCOTP_CFG0[0];
    HW_OCOTP_CFG1 = iMX_OPT.OCOTP_CFG1[0];
    HW_OCOTP_CFG2 = iMX_OPT.OCOTP_CFG2[0];
    HW_OCOTP_CFG3 = iMX_OPT.OCOTP_CFG3[0];
    HW_OCOTP_CFG4 = iMX_OPT.OCOTP_CFG4[0];
    HW_OCOTP_CFG5 = iMX_OPT.OCOTP_CFG5[0];
    HW_OCOTP_CFG6 = iMX_OPT.OCOTP_CFG6[0];
    HW_OCOTP_MEM0 = iMX_OPT.OCOTP_MEM0[0];
    HW_OCOTP_MEM1 = iMX_OPT.OCOTP_MEM1[0];
    HW_OCOTP_MEM2 = iMX_OPT.OCOTP_MEM2[0];
    HW_OCOTP_MEM3 = iMX_OPT.OCOTP_MEM3[0];
    HW_OCOTP_MEM4 = iMX_OPT.OCOTP_MEM4[0];
    HW_OCOTP_ANA0 = iMX_OPT.OCOTP_ANA0[0];
    HW_OCOTP_ANA1 = iMX_OPT.OCOTP_ANA1[0];
    HW_OCOTP_ANA2 = iMX_OPT.OCOTP_ANA2[0];
    HW_OCOTP_SRK0 = iMX_OPT.OCOTP_SRK[0];
    HW_OCOTP_SRK1 = iMX_OPT.OCOTP_SRK[4];
    HW_OCOTP_SRK2 = iMX_OPT.OCOTP_SRK[8];
    HW_OCOTP_SRK3 = iMX_OPT.OCOTP_SRK[12];
    HW_OCOTP_SRK4 = iMX_OPT.OCOTP_SRK[16];
    HW_OCOTP_SRK5 = iMX_OPT.OCOTP_SRK[20];
    HW_OCOTP_SRK6 = iMX_OPT.OCOTP_SRK[24];
    HW_OCOTP_SRK7 = iMX_OPT.OCOTP_SRK[28];
    HW_OCOTP_SJC_RESP0 = iMX_OPT.OCOTP_SJC_RESP[0];
    HW_OCOTP_SJC_RESP1 = iMX_OPT.OCOTP_SJC_RESP[4];
    HW_OCOTP_MAC0 = iMX_OPT.OCOTP_MAC0[0];
    HW_OCOTP_MAC1 = iMX_OPT.OCOTP_MAC1[0];
#if ETHERNET_COUNT > 1
    HW_OCOTP_MAC2 = iMX_OPT.OCOTP_MAC2[0];
    HW_OCOTP_OTPK_CRC32 = iMX_OPT.OCOTP_OTPK_CRC32[0];
#else
    HW_OCOTP_GP3 = iMX_OPT.OCOTP_GP3[0];
#endif
    HW_OCOTP_GP1 = iMX_OPT.OCOTP_GP1[0];
    HW_OCOTP_GP2 = iMX_OPT.OCOTP_GP2[0];
    HW_OCOTP_SW_GP1 = iMX_OPT.OCOTP_SW_GP1[0];
    if ((HW_OCOTP_LOCK & eFUSE_LOCK_SW_GP2_RLOCK) != 0) {
        HW_OCOTP_SW_GP20 = 0xbadabada;                                   // set pattern showin that the true content can't be read
        HW_OCOTP_SW_GP21 = 0xbadabada;
        HW_OCOTP_SW_GP22 = 0xbadabada;
        HW_OCOTP_SW_GP23 = 0xbadabada;
    }
    else {
        HW_OCOTP_SW_GP20 = iMX_OPT.OCOTP_SW_GP20[0];
        HW_OCOTP_SW_GP21 = iMX_OPT.OCOTP_SW_GP21[0];
        HW_OCOTP_SW_GP22 = iMX_OPT.OCOTP_SW_GP22[0];
        HW_OCOTP_SW_GP23 = iMX_OPT.OCOTP_SW_GP23[0];
    }
    HW_OCOTP_MISC_CONF0 = iMX_OPT.OCOTP_MISC_CONF0[0];
    HW_OCOTP_MISC_CONF1 = iMX_OPT.OCOTP_MISC_CONF1[0];
    HW_OCOTP_SRK_REVOKE = iMX_OPT.OCOTP_SRK_REVOKE[0];
#if defined iMX_RT106X
    HW_OCOTP_ROM_PATCH0 = iMX_OPT.OCOTP_ROM_PATCH0[0];
    HW_OCOTP_ROM_PATCH1 = iMX_OPT.OCOTP_ROM_PATCH1[0];
    HW_OCOTP_ROM_PATCH2 = iMX_OPT.OCOTP_ROM_PATCH2[0];
    HW_OCOTP_ROM_PATCH3 = iMX_OPT.OCOTP_ROM_PATCH3[0];
    HW_OCOTP_ROM_PATCH4 = iMX_OPT.OCOTP_ROM_PATCH4[0];
    HW_OCOTP_ROM_PATCH5 = iMX_OPT.OCOTP_ROM_PATCH5[0];
    HW_OCOTP_ROM_PATCH6 = iMX_OPT.OCOTP_ROM_PATCH6[0];
    HW_OCOTP_ROM_PATCH7 = iMX_OPT.OCOTP_ROM_PATCH7[0];
    HW_OCOTP_GP30 = iMX_OPT.OCOTP_GP30[0];
    HW_OCOTP_GP31 = iMX_OPT.OCOTP_GP31[0];
    HW_OCOTP_GP32 = iMX_OPT.OCOTP_GP32[0];
    HW_OCOTP_GP33 = iMX_OPT.OCOTP_GP33[0];
    HW_OCOTP_GP40 = iMX_OPT.OCOTP_GP40[0];
    HW_OCOTP_GP41 = iMX_OPT.OCOTP_GP41[0];
    HW_OCOTP_GP42 = iMX_OPT.OCOTP_GP42[0];
    HW_OCOTP_GP43 = iMX_OPT.OCOTP_GP43[0];
#endif
}

// Initialisation of all non-zero registers in the device
//
static void fnSetDevice(unsigned long *port_inits)
{
    extern void fnEnterHW_table(void *hw_table);

#if defined SUPPORT_ADC
    int i;
    int j;
#endif
    unsigned long initial_input_state;

    iMX.CORTEX_M7_REGS.ulPRIMASK = INTERRUPT_MASKED;                     // interrupts are masked out of reset

    if (iMX_OPT.OCOTP_LOCK[0] == 0) {                                    // if the content has not been set by a file we set defaults here
        iMX_OPT.OCOTP_LOCK[0] = iMX_OPT.OCOTP_LOCK[1] = iMX_OPT.OCOTP_LOCK[2] = iMX_OPT.OCOTP_LOCK[3] = (0x40128000 | eFUSE_LOCK_TEST_LOCK_MASK); // 0x400 = 0x40128003 in fresh device
#if defined MIMXRT1050                                                   // unique ID and silicon revision of reference iMXRT1050-EVB
        iMX_OPT.OCOTP_CFG0[0] = iMX_OPT.OCOTP_CFG0[1] = iMX_OPT.OCOTP_CFG0[2] = iMX_OPT.OCOTP_CFG0[3] = 0x65f82484; // 0x410 - eFUSE_UNIQUE_ID_0
        iMX_OPT.OCOTP_CFG1[0] = iMX_OPT.OCOTP_CFG1[1] = iMX_OPT.OCOTP_CFG1[2] = iMX_OPT.OCOTP_CFG1[3] = 0x190b51d2; // 0x420 - eFUSE_UNIQUE_ID_1
        iMX_OPT.OCOTP_CFG2[0] = iMX_OPT.OCOTP_CFG2[1] = iMX_OPT.OCOTP_CFG2[2] = iMX_OPT.OCOTP_CFG2[3] = 0x500100d0; // 0x430 - eFUSE_SILICON_REV
        iMX_OPT.OCOTP_CFG3[0] = iMX_OPT.OCOTP_CFG3[1] = iMX_OPT.OCOTP_CFG3[2] = iMX_OPT.OCOTP_CFG3[3] = (0x00400002 | eFUSE_SPEED_GRADING_600MHz); // 0x440 = 0x00420002 - eFUSE_SPEED_GRADING
        iMX_OPT.OCOTP_MEM1[0] = iMX_OPT.OCOTP_MEM1[1] = iMX_OPT.OCOTP_MEM1[2] = iMX_OPT.OCOTP_MEM1[3] = 0x00009400; // 0x490
        iMX_OPT.OCOTP_MEM2[0] = iMX_OPT.OCOTP_MEM2[1] = iMX_OPT.OCOTP_MEM2[2] = iMX_OPT.OCOTP_MEM2[3] = 0x00000097; // 0x4a0
        iMX_OPT.OCOTP_ANA1[0] = iMX_OPT.OCOTP_ANA1[1] = iMX_OPT.OCOTP_ANA1[2] = iMX_OPT.OCOTP_ANA1[3] = 0x4ec4875f; // 0x4e0
#elif defined MIMXRT1064                                                   // unique ID and silicon revision of reference iMXRT1064-EVB
        iMX_OPT.OCOTP_CFG0[0] = iMX_OPT.OCOTP_CFG0[1] = iMX_OPT.OCOTP_CFG0[2] = iMX_OPT.OCOTP_CFG0[3] = 0xb96fe9ba; // 0x410 - eFUSE_UNIQUE_ID_0
        iMX_OPT.OCOTP_CFG1[0] = iMX_OPT.OCOTP_CFG1[1] = iMX_OPT.OCOTP_CFG1[2] = iMX_OPT.OCOTP_CFG1[3] = 0x916bfa5a; // 0x420 - eFUSE_UNIQUE_ID_1
        iMX_OPT.OCOTP_CFG2[0] = iMX_OPT.OCOTP_CFG2[1] = iMX_OPT.OCOTP_CFG2[2] = iMX_OPT.OCOTP_CFG2[3] = 0x500100d0; // 0x430 - eFUSE_SILICON_REV
        iMX_OPT.OCOTP_CFG3[0] = iMX_OPT.OCOTP_CFG3[1] = iMX_OPT.OCOTP_CFG3[2] = iMX_OPT.OCOTP_CFG3[3] = (0x00100002 | eFUSE_SPEED_GRADING_600MHz); // 0x440 = 0x00120002 - eFUSE_SPEED_GRADING
        iMX_OPT.OCOTP_CFG5[0] = iMX_OPT.OCOTP_CFG5[1] = iMX_OPT.OCOTP_CFG5[2] = iMX_OPT.OCOTP_CFG5[3] = uFUSE_BOOT_CFG2_DIR_BT_DIS; // 0x460 = 0x00000008 - eFUSE_CFG1
        iMX_OPT.OCOTP_MEM1[0] = iMX_OPT.OCOTP_MEM1[1] = iMX_OPT.OCOTP_MEM1[2] = iMX_OPT.OCOTP_MEM1[3] = 0x00009c00; // 0x490
        iMX_OPT.OCOTP_MEM2[0] = iMX_OPT.OCOTP_MEM2[1] = iMX_OPT.OCOTP_MEM2[2] = iMX_OPT.OCOTP_MEM2[3] = 0x000000b2; // 0x4a0
        iMX_OPT.OCOTP_ANA1[0] = iMX_OPT.OCOTP_ANA1[1] = iMX_OPT.OCOTP_ANA1[2] = iMX_OPT.OCOTP_ANA1[3] = 0x5294c169; // 0x4e0
        iMX_OPT.OCOTP_MISC_CONF1[0] = iMX_OPT.OCOTP_MISC_CONF1[1] = iMX_OPT.OCOTP_MISC_CONF1[2] = iMX_OPT.OCOTP_MISC_CONF1[3] = 0xb2000000; // 0x6e0 - eFUSE_CONFIG1
#elif defined iMX_RT1062_EMB_ART
    #if 0                                                                // from reference "1"
        iMX_OPT.OCOTP_CFG0[0] = iMX_OPT.OCOTP_CFG0[1] = iMX_OPT.OCOTP_CFG0[2] = iMX_OPT.OCOTP_CFG0[3] = 0x65f8294a; // 0x410 - eFUSE_UNIQUE_ID_0
        iMX_OPT.OCOTP_CFG1[0] = iMX_OPT.OCOTP_CFG1[1] = iMX_OPT.OCOTP_CFG1[2] = iMX_OPT.OCOTP_CFG1[3] = 0x151a09d2; // 0x420 - eFUSE_UNIQUE_ID_1
        iMX_OPT.OCOTP_CFG2[0] = iMX_OPT.OCOTP_CFG2[1] = iMX_OPT.OCOTP_CFG2[2] = iMX_OPT.OCOTP_CFG2[3] = 0x50000097; // 0x430 - eFUSE_SILICON_REV
        iMX_OPT.OCOTP_CFG3[0] = iMX_OPT.OCOTP_CFG3[1] = iMX_OPT.OCOTP_CFG3[2] = iMX_OPT.OCOTP_CFG3[3] = (0x00400002 | eFUSE_SPEED_GRADING_500MHz); // 0x440 = 0x00410002 - eFUSE_SPEED_GRADING
        iMX_OPT.OCOTP_MEM1[0] = iMX_OPT.OCOTP_MEM1[1] = iMX_OPT.OCOTP_MEM1[2] = iMX_OPT.OCOTP_MEM1[3] = 0x0000a400; // 0x490
        iMX_OPT.OCOTP_MEM2[0] = iMX_OPT.OCOTP_MEM2[1] = iMX_OPT.OCOTP_MEM2[2] = iMX_OPT.OCOTP_MEM2[3] = 0x00000056; // 0x4a0
        iMX_OPT.OCOTP_ANA1[0] = iMX_OPT.OCOTP_ANA1[1] = iMX_OPT.OCOTP_ANA1[2] = iMX_OPT.OCOTP_ANA1[3] = 0x51b49e69; // 0x4e0
    #endif
    #if 1                                                                // from reference "2"
        iMX_OPT.OCOTP_CFG0[0] = iMX_OPT.OCOTP_CFG0[1] = iMX_OPT.OCOTP_CFG0[2] = iMX_OPT.OCOTP_CFG0[3] = 0x65f8294a; // 0x410 - eFUSE_UNIQUE_ID_0
        iMX_OPT.OCOTP_CFG1[0] = iMX_OPT.OCOTP_CFG1[1] = iMX_OPT.OCOTP_CFG1[2] = iMX_OPT.OCOTP_CFG1[3] = 0x1e1e09d2; // 0x420 - eFUSE_UNIQUE_ID_1
        iMX_OPT.OCOTP_CFG2[0] = iMX_OPT.OCOTP_CFG2[1] = iMX_OPT.OCOTP_CFG2[2] = iMX_OPT.OCOTP_CFG2[3] = 0x50000041; // 0x430 - eFUSE_SILICON_REV
        iMX_OPT.OCOTP_CFG3[0] = iMX_OPT.OCOTP_CFG3[1] = iMX_OPT.OCOTP_CFG3[2] = iMX_OPT.OCOTP_CFG3[3] = (0x00400002 | eFUSE_SPEED_GRADING_500MHz); // 0x440 = 0x00410002 - eFUSE_SPEED_GRADING
        iMX_OPT.OCOTP_MEM1[0] = iMX_OPT.OCOTP_MEM1[1] = iMX_OPT.OCOTP_MEM1[2] = iMX_OPT.OCOTP_MEM1[3] = 0x00009400; // 0x490
        iMX_OPT.OCOTP_MEM2[0] = iMX_OPT.OCOTP_MEM2[1] = iMX_OPT.OCOTP_MEM2[2] = iMX_OPT.OCOTP_MEM2[3] = 0x0000005d; // 0x4a0
        iMX_OPT.OCOTP_ANA1[0] = iMX_OPT.OCOTP_ANA1[1] = iMX_OPT.OCOTP_ANA1[2] = iMX_OPT.OCOTP_ANA1[3] = 0x54e4ca69; // 0x4e0
    #endif
#else                                                                    // fictional board
        iMX_OPT.OCOTP_CFG0[0] = iMX_OPT.OCOTP_CFG0[1] = iMX_OPT.OCOTP_CFG0[2] = iMX_OPT.OCOTP_CFG0[3] = 0x12345678; // 0x410 - eFUSE_UNIQUE_ID_0
        iMX_OPT.OCOTP_CFG1[0] = iMX_OPT.OCOTP_CFG1[1] = iMX_OPT.OCOTP_CFG1[2] = iMX_OPT.OCOTP_CFG1[3] = 0xabcdef01; // 0x420 - eFUSE_UNIQUE_ID_1
        iMX_OPT.OCOTP_CFG2[0] = iMX_OPT.OCOTP_CFG2[1] = iMX_OPT.OCOTP_CFG2[2] = iMX_OPT.OCOTP_CFG2[3] = 0x500100d0; // 0x430 - eFUSE_SILICON_REV
        iMX_OPT.OCOTP_CFG3[0] = iMX_OPT.OCOTP_CFG3[1] = iMX_OPT.OCOTP_CFG3[2] = iMX_OPT.OCOTP_CFG3[3] = (0x00400002 | eFUSE_SPEED_GRADING_500MHz); // 0x440 - eFUSE_SPEED_GRADING
        iMX_OPT.OCOTP_MEM1[0] = iMX_OPT.OCOTP_MEM1[1] = iMX_OPT.OCOTP_MEM1[2] = iMX_OPT.OCOTP_MEM1[3] = 0x00009400; // 0x490
        iMX_OPT.OCOTP_MEM2[0] = iMX_OPT.OCOTP_MEM2[1] = iMX_OPT.OCOTP_MEM2[2] = iMX_OPT.OCOTP_MEM2[3] = 0x00000097; // 0x4a0
        iMX_OPT.OCOTP_ANA1[0] = iMX_OPT.OCOTP_ANA1[1] = iMX_OPT.OCOTP_ANA1[2] = iMX_OPT.OCOTP_ANA1[3] = 0x4ec4875f; // 0x4e0
#endif
        iMX_OPT.OCOTP_MISC_CONF0[0] = iMX_OPT.OCOTP_MISC_CONF0[1] = iMX_OPT.OCOTP_MISC_CONF0[2] = iMX_OPT.OCOTP_MISC_CONF0[3] = eFUSE_CONFIG0_USB_VBUS; // 0x6d0 = 0x00000040 - eFUSE_CONFIG0
    }

    // Copy OPT content to visible shadow registers
    //
    fnUpdateShadowRegisters();

    // IOMUXC default values
    //
    IOMUXC_SNVS_SW_PAD_CTL_PAD_TEST_MODE = 0x000030a0;
    IOMUXC_SNVS_SW_PAD_CTL_PAD_POR_B = 0x0001b0a0;
    IOMUXC_SNVS_SW_PAD_CTL_PAD_ONOFF = 0x0001b0a0;
    IOMUXC_SNVS_SW_PAD_CTL_PAD_PMIC_ON_REQ = 0x0000b8a0;
#if !defined iMX_RT1011
    IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP = 0x00000005;
    IOMUXC_SNVS_SW_PAD_CTL_PAD_WAKEUP = 0x0001b0a0;
    IOMUXC_SNVS_SW_PAD_CTL_PAD_PMIC_STBY_REQ = 0x0000a0a0;
#endif
    IOMUXC_GPR_GPR3 = (0x0000ffe0 | IOMUXC_GPR_GPR3_DCP_KEY_SEL_HI);
    IOMUXC_GPR_GPR10 = (IOMUXC_GPR_GPR10_NIDEN | IOMUXC_GPR_GPR10_DBG_EN | IOMUXC_GPR_GPR10_SEC_ERR_RESP);
    IOMUXC_GPR_GPR15 = 0xffffffff;
#if defined iMX_RT1011
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14 = cPinDefaults[_PORT1][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_13 = cPinDefaults[_PORT1][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_12 = cPinDefaults[_PORT1][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_11 = cPinDefaults[_PORT1][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_10 = cPinDefaults[_PORT1][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_09 = cPinDefaults[_PORT1][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_08 = cPinDefaults[_PORT1][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_07 = cPinDefaults[_PORT1][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_06 = cPinDefaults[_PORT1][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_05 = cPinDefaults[_PORT1][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_04 = cPinDefaults[_PORT1][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_03 = cPinDefaults[_PORT1][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_02 = cPinDefaults[_PORT1][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_01 = cPinDefaults[_PORT1][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_00 = cPinDefaults[_PORT1][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_13 = cPinDefaults[_PORT2][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_12 = cPinDefaults[_PORT2][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_11 = cPinDefaults[_PORT2][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_10 = cPinDefaults[_PORT2][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_09 = cPinDefaults[_PORT2][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_08 = cPinDefaults[_PORT2][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_07 = cPinDefaults[_PORT2][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_06 = cPinDefaults[_PORT2][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_05 = cPinDefaults[_PORT2][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_04 = cPinDefaults[_PORT2][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_03 = cPinDefaults[_PORT2][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_02 = cPinDefaults[_PORT2][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_01 = cPinDefaults[_PORT2][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_00 = cPinDefaults[_PORT2][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_13 = cPinDefaults[_PORT1][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_12 = cPinDefaults[_PORT1][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_11 = cPinDefaults[_PORT1][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_10 = cPinDefaults[_PORT1][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_09 = cPinDefaults[_PORT1][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_08 = cPinDefaults[_PORT1][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_07 = cPinDefaults[_PORT1][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_06 = cPinDefaults[_PORT1][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_05 = cPinDefaults[_PORT1][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_04 = cPinDefaults[_PORT1][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_03 = cPinDefaults[_PORT1][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_02 = cPinDefaults[_PORT1][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_01 = cPinDefaults[_PORT1][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_00 = cPinDefaults[_PORT1][0];

    IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ = cPinDefaults[_PORT5][0];
#else
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00 = cPinDefaults[_PORT1][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01 = cPinDefaults[_PORT1][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 = cPinDefaults[_PORT1][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 = cPinDefaults[_PORT1][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_04 = cPinDefaults[_PORT1][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05 = cPinDefaults[_PORT1][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_06 = cPinDefaults[_PORT1][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_07 = cPinDefaults[_PORT1][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_08 = cPinDefaults[_PORT1][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_09 = cPinDefaults[_PORT1][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_10 = cPinDefaults[_PORT1][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_11 = cPinDefaults[_PORT1][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12 = cPinDefaults[_PORT1][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_13 = cPinDefaults[_PORT1][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_14 = cPinDefaults[_PORT1][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_15 = cPinDefaults[_PORT1][15];
    #if !defined iMX_RT1015
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00 = cPinDefaults[_PORT1][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_01 = cPinDefaults[_PORT1][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 = cPinDefaults[_PORT1][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03 = cPinDefaults[_PORT1][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_04 = cPinDefaults[_PORT1][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_05 = cPinDefaults[_PORT1][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06 = cPinDefaults[_PORT1][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_07 = cPinDefaults[_PORT1][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08 = cPinDefaults[_PORT1][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09 = cPinDefaults[_PORT1][25];
    #endif
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_10 = cPinDefaults[_PORT1][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_11 = cPinDefaults[_PORT1][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_12 = cPinDefaults[_PORT1][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_13 = cPinDefaults[_PORT1][29];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_14 = cPinDefaults[_PORT1][30];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_15 = cPinDefaults[_PORT1][31];

    #if defined iMX_RT105X || defined iMX_RT106X
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 = cPinDefaults[_PORT2][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01 = cPinDefaults[_PORT2][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02 = cPinDefaults[_PORT2][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = cPinDefaults[_PORT2][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_04 = cPinDefaults[_PORT2][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_05 = cPinDefaults[_PORT2][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_06 = cPinDefaults[_PORT2][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_07 = cPinDefaults[_PORT2][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_08 = cPinDefaults[_PORT2][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_09 = cPinDefaults[_PORT2][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_10 = cPinDefaults[_PORT2][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = cPinDefaults[_PORT2][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12 = cPinDefaults[_PORT2][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_13 = cPinDefaults[_PORT2][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 = cPinDefaults[_PORT2][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 = cPinDefaults[_PORT2][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = cPinDefaults[_PORT2][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_01 = cPinDefaults[_PORT2][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02 = cPinDefaults[_PORT2][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_03 = cPinDefaults[_PORT2][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 = cPinDefaults[_PORT2][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 = cPinDefaults[_PORT2][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 = cPinDefaults[_PORT2][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 = cPinDefaults[_PORT2][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 = cPinDefaults[_PORT2][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 = cPinDefaults[_PORT2][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = cPinDefaults[_PORT2][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = cPinDefaults[_PORT2][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 = cPinDefaults[_PORT2][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13 = cPinDefaults[_PORT2][29];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = cPinDefaults[_PORT2][30];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = cPinDefaults[_PORT2][31];
    #else
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00 = cPinDefaults[_PORT2][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01 = cPinDefaults[_PORT2][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 = cPinDefaults[_PORT2][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 = cPinDefaults[_PORT2][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 = cPinDefaults[_PORT2][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 = cPinDefaults[_PORT2][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 = cPinDefaults[_PORT2][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = cPinDefaults[_PORT2][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 = cPinDefaults[_PORT2][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 = cPinDefaults[_PORT2][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 = cPinDefaults[_PORT2][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 = cPinDefaults[_PORT2][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 = cPinDefaults[_PORT2][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 = cPinDefaults[_PORT2][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 = cPinDefaults[_PORT2][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 = cPinDefaults[_PORT2][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16 = cPinDefaults[_PORT2][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17 = cPinDefaults[_PORT2][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 = cPinDefaults[_PORT2][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 = cPinDefaults[_PORT2][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20 = cPinDefaults[_PORT2][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21 = cPinDefaults[_PORT2][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = cPinDefaults[_PORT2][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = cPinDefaults[_PORT2][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = cPinDefaults[_PORT2][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = cPinDefaults[_PORT2][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = cPinDefaults[_PORT2][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = cPinDefaults[_PORT2][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = cPinDefaults[_PORT2][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = cPinDefaults[_PORT2][29];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30 = cPinDefaults[_PORT2][30];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31 = cPinDefaults[_PORT2][31];
    #endif
    #if defined iMX_RT105X || defined iMX_RT106X
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00 = cPinDefaults[_PORT3][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_01 = cPinDefaults[_PORT3][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_02 = cPinDefaults[_PORT3][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_03 = cPinDefaults[_PORT3][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_04 = cPinDefaults[_PORT3][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_05 = cPinDefaults[_PORT3][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_06 = cPinDefaults[_PORT3][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_07 = cPinDefaults[_PORT3][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_08 = cPinDefaults[_PORT3][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_09 = cPinDefaults[_PORT3][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_10 = cPinDefaults[_PORT3][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_11 = cPinDefaults[_PORT3][11];

    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 = cPinDefaults[_PORT3][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = cPinDefaults[_PORT3][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 = cPinDefaults[_PORT3][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = cPinDefaults[_PORT3][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = cPinDefaults[_PORT3][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = cPinDefaults[_PORT3][17];

    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00 = cPinDefaults[_PORT4][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01 = cPinDefaults[_PORT4][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 = cPinDefaults[_PORT4][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 = cPinDefaults[_PORT4][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 = cPinDefaults[_PORT4][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 = cPinDefaults[_PORT4][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 = cPinDefaults[_PORT4][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = cPinDefaults[_PORT4][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 = cPinDefaults[_PORT4][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 = cPinDefaults[_PORT4][9];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 = cPinDefaults[_PORT4][10];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 = cPinDefaults[_PORT4][11];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 = cPinDefaults[_PORT4][12];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 = cPinDefaults[_PORT4][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 = cPinDefaults[_PORT4][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 = cPinDefaults[_PORT4][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16 = cPinDefaults[_PORT4][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17 = cPinDefaults[_PORT4][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 = cPinDefaults[_PORT4][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 = cPinDefaults[_PORT4][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20 = cPinDefaults[_PORT4][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21 = cPinDefaults[_PORT4][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = cPinDefaults[_PORT4][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = cPinDefaults[_PORT4][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = cPinDefaults[_PORT4][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = cPinDefaults[_PORT4][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = cPinDefaults[_PORT4][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = cPinDefaults[_PORT4][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = cPinDefaults[_PORT4][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = cPinDefaults[_PORT4][29];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30 = cPinDefaults[_PORT4][30];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31 = cPinDefaults[_PORT4][31];

    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32 = cPinDefaults[_PORT3][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33 = cPinDefaults[_PORT3][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34 = cPinDefaults[_PORT3][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 = cPinDefaults[_PORT3][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36 = cPinDefaults[_PORT3][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37 = cPinDefaults[_PORT3][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 = cPinDefaults[_PORT3][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39 = cPinDefaults[_PORT3][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = cPinDefaults[_PORT3][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 = cPinDefaults[_PORT3][27];
    #else
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32 = cPinDefaults[_PORT3][0];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33 = cPinDefaults[_PORT3][1];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34 = cPinDefaults[_PORT3][2];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 = cPinDefaults[_PORT3][3];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36 = cPinDefaults[_PORT3][4];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37 = cPinDefaults[_PORT3][5];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 = cPinDefaults[_PORT3][6];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39 = cPinDefaults[_PORT3][7];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = cPinDefaults[_PORT3][8];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 = cPinDefaults[_PORT3][9];

    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 = cPinDefaults[_PORT3][13];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = cPinDefaults[_PORT3][14];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 = cPinDefaults[_PORT3][15];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = cPinDefaults[_PORT3][16];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = cPinDefaults[_PORT3][17];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = cPinDefaults[_PORT3][18];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_06 = cPinDefaults[_PORT3][19];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00 = cPinDefaults[_PORT3][20];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_01 = cPinDefaults[_PORT3][21];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_02 = cPinDefaults[_PORT3][22];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_03 = cPinDefaults[_PORT3][23];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_04 = cPinDefaults[_PORT3][24];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_05 = cPinDefaults[_PORT3][25];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_06 = cPinDefaults[_PORT3][26];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_07 = cPinDefaults[_PORT3][27];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_08 = cPinDefaults[_PORT3][28];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_09 = cPinDefaults[_PORT3][29];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_10 = cPinDefaults[_PORT3][30];
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_11 = cPinDefaults[_PORT3][31];
    #endif

    IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP = cPinDefaults[_PORT5][0];
    IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ = cPinDefaults[_PORT5][1];
    IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_STBY_REQ = cPinDefaults[_PORT5][2];
#endif
#if defined iMX_RT1064
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_01 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_02 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_03 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_04 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_05 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_06 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_07 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_08 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_09 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_10 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_11 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B0_12 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_00 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_01 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_02 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_03 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_04 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_05 = 0x00000005;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SPI_B1_06 = 0x00000005;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_00 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_01 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_02 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_03 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_04 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_05 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_06 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_07 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_08 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_09 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_10 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_11 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_12 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B0_13 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_00 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_01 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_02 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_03 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_04 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_05 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_06 = 0x000010b0;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_SPI_B1_07 = 0x000010b0;
#endif
    DCDC_REG0 = 0x14030111;
    DCDC_REG1 = 0x111ba29c;
    DCDC_REG2 = 0x00000009;
    DCDC_REG3 = 0x0000010e;

    PMU_REG_1P1 = 0x00001073;
    PMU_REG_3P0 = (PMU_REG_3P0_OUTPUT_TRG_3V000 | PMU_REG_3P0_BO_OFFSET_175mV | PMU_REG_3P0_ENABLE_ILIMIT); // USB LDO default
    PMU_REG_2P5 = 0x00001073;
    PMU_REG_CORE = 0x00482012;
    PMU_MISC0 = 0x04000000;
    PMU_MISC2 = 0x00272727;

    SNVS_HPSR = 0x80000000;
    SNVS_LPCR = SNVS_LPCR_DP_EN;
    SNVS_LPSR = 0x00000008;
    SNVS_HPVIDR1 = 0x003e0104;
    SNVS_HPVIDR2 = 0x06000000;

    HW_OCOTP_TIMING = 0x060d9755;
    HW_OCOTP_VERSION = 0x06000000;
    HW_OCOTP_TIMING2 = 0x01c30092;

    // Clock configuration module (CCM)
    //
    CCM_CCR = (CCM_CCR_COSC_EN | 0x00010000 | (32 << 21) | (127));
    CCM_CSR = (CCM_CSR_COSC_READY | 0x00000010);
    CCM_CCSR = 0x00000100;
#if defined iMX_RT1011
    CCM_CBCDR = 0x00028000;
    CCM_CBCMR = 0x0c088020;
    CCM_CSCMR1 = 0x0080fc00;
    CCM_CSCMR2 = 0x00180000;
    CCM_CSCDR1 = 0x06000000;
#elif defined iMX_RT105X
    CCM_CBCDR = 0x000a8300;
    CCM_CBCMR = 0x2dae8324;
    CCM_CSCMR1 = 0x04900000;
    CCM_CSCMR2 = 0x13192f06;
    CCM_CSCDR1 = 0x06490b00;
#else
    CCM_CBCDR = 0x000a8000;
    CCM_CBCMR = 0x2daa8324;
    CCM_CSCMR1 = 0x04900000;
    CCM_CSCMR2 = 0x13192f06;
    CCM_CSCDR1 = 0x06490b00;
#endif
    CCM_CS1CDR = 0x0ec102c1;
    CCM_CS2CDR = 0x000736c1;
    CCM_CDCDR = 0x33f71f92;
    CCM_CSCDR2 = 0x00029150;
    CCM_CSCDR3 = 0x00030841;
    CCM_CLPCR = 0x00000079;
    CCM_CIMR = 0xffffffff;
#if defined iMX_RT105X || defined iMX_RT106X
    CCM_CCOSR = (0x000a0000 | CCM_CCOSR_CLKO1_SEL_SYS_PLL_DIV2);
#else
    CCM_CCOSR = (0x000a0000 | CCM_CCOSR_CLKO1_SEL_PLL2_DIV2);
#endif
    CCM_CGPR = (CCM_CGPR_SYS_MEM_DS_CTRL_ENABLE_IN_STOP | CCM_CGPR_FIXED_1);
    CCM_CCGR0 = 0xffffffff;                                              // clocks are on during all modes except STOP mode
    CCM_CCGR1 = 0xffffffff;                                              // clocks are on during all modes except STOP mode
    CCM_CCGR2 = 0xfc3fffff;                                              // xbar1 and xbar2 clocks stopped
    CCM_CCGR3 = 0xffffffcf;                                              // semc clocks stopped
    CCM_CCGR4 = 0xffffffff;                                              // clocks are on during all modes except STOP mode
    CCM_CCGR5 = 0xffffffff;                                              // clocks are on during all modes except STOP mode
    CCM_CCGR6 = 0xffffffff;                                              // clocks are on during all modes except STOP mode
    CCM_CMEOR = 0xffffffff;

    CCM_ANALOG_MISC0 = 0x04000000;

    CCM_ANALOG_PFD_528 = 0x1018101b;
    CCM_ANALOG_PLL_SYS = 0x00013001;
    CCM_ANALOG_PLL_SYS_DENOM = 0x00000012;

    WDOG1_WCR = (WDOG_WCR_WDA | WDOG_WCR_SRS);
    WDOG1_WRSR = WDOG_WRSR_POR;
    WDOG1_WICR = 4;
    WDOG1_WMCR = WDOG_WMCR_PDE;

    WDOG2_WCR = (WDOG_WCR_WDA | WDOG_WCR_SRS);
    WDOG2_WRSR = WDOG_WRSR_POR;
    WDOG2_WICR = 4;
    WDOG2_WMCR = WDOG_WMCR_PDE;

    WDOG3_CS = (WDOG_CS_CMD32EN | WDOG_CS_CLK_LPO | WDOG_CS_EN);
    WDOG3_TOVAL = 0x00007d00;

    SRC_SCR = 0xa0480520;
    SRC_SRSR = SRC_SRSR_IPP_RESET_B;

    IOMUXC_GPR_GPR16 = (IOMUXC_GPR_GPR16_INIT_ITCM_EN | IOMUXC_GPR_GPR16_INIT_DTCM_EN | IOMUXC_GPR_GPR16_RESERVED);

    FLEX_SPI_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_ARDFEN | FLEX_SPI_MCR0_ATDFEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_MDIS);
    FLEX_SPI_MCR1 = (FLEX_SPI_MCR1_AHBBUSWAIT_MASK | FLEX_SPI_MCR1_SEQWAIT_MASK);
    FLEX_SPI_MCR2 = (FLEX_SPI_MCR2_SAMEDEVICEEN | FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    FLEX_SPI_AHBCR = (FLEX_SPI_AHBCR_CACHABLEEN | FLEX_SPI_AHBCR_BUFFERABLEEN);
    FLEX_SPI_INTR = FLEX_SPI_INTR_IPTXWE;
    FLEX_SPI_LUTKEY = FLEX_SPI_LUTKEY_KEY;
    FLEX_SPI_LUTCR = FLEX_SPI_LUTCR_UNLOCK;
    FLEX_SPI_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_1);
    FLEX_SPI_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_2);
    FLEX_SPI_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_3);
    FLEX_SPI_FLSHA1CR0 = 0x00010000;
    FLEX_SPI_FLSHA2CR0 = 0x00010000;
    FLEX_SPI_FLSHB1CR0 = 0x00010000;
    FLEX_SPI_FLSHB2CR0 = 0x00010000;
    FLEX_SPI_FLSHA1CR1 = 0x00000063;
    FLEX_SPI_FLSHA2CR1 = 0x00000063;
    FLEX_SPI_FLSHB1CR1 = 0x00000063;
    FLEX_SPI_FLSHB2CR1 = 0x00000063;
    FLEX_SPI_DLLACR = 0x00000100;
    FLEX_SPI_DLLBCR = 0x00000100;
    FLEX_SPI_STS0 = (FLEX_SPI_STS0_SEQIDLE | FLEX_SPI_STS0_ARBIDLE);
    FLEX_SPI_STS2 = 0x01000100;
#if FLEX_SPI_INTERFACES > 1
    FLEX_SPI2_MCR0 = (FLEX_SPI_MCR0_IPGRANTWAIT_MASK | FLEX_SPI_MCR0_AHBGRANTWAIT_MASK | FLEX_SPI_MCR0_RESERVED_ONE | FLEX_SPI_MCR0_ARDFEN | FLEX_SPI_MCR0_ATDFEN | FLEX_SPI_MCR0_RXCLKSRC_INTERNAL_LOOPBACK | FLEX_SPI_MCR0_MDIS);
    FLEX_SPI2_MCR1 = (FLEX_SPI_MCR1_AHBBUSWAIT_MASK | FLEX_SPI_MCR1_SEQWAIT_MASK);
    FLEX_SPI2_MCR2 = (FLEX_SPI_MCR2_SAMEDEVICEEN | FLEX_SPI_MCR2_RESUMEWAIT_DEFAULT | FLEX_SPI_MCR2_RESERVED);
    FLEX_SPI2_AHBCR = (FLEX_SPI_AHBCR_CACHABLEEN | FLEX_SPI_AHBCR_BUFFERABLEEN);
    FLEX_SPI2_INTR = FLEX_SPI_INTR_IPTXWE;
    FLEX_SPI2_LUTKEY = FLEX_SPI_LUTKEY_KEY;
    FLEX_SPI2_LUTCR = FLEX_SPI_LUTCR_UNLOCK;
    FLEX_SPI2_AHBRXBUF0CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_0);
    FLEX_SPI2_AHBRXBUF1CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_1);
    FLEX_SPI2_AHBRXBUF2CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_2);
    FLEX_SPI2_AHBRXBUF3CR0 = (FLEX_SPI_AHBRXBUFnCR0_BUFSZ_DEFAULT | FLEX_SPI_AHBRXBUFnCR0_PREFETCHEN | FLEX_SPI_AHBRXBUFnCR0_MSTRID_3);
    FLEX_SPI2_FLSHA1CR0 = 0x00010000;
    FLEX_SPI2_FLSHA2CR0 = 0x00010000;
    FLEX_SPI2_FLSHB1CR0 = 0x00010000;
    FLEX_SPI2_FLSHB2CR0 = 0x00010000;
    FLEX_SPI2_FLSHA1CR1 = 0x00000063;
    FLEX_SPI2_FLSHA2CR1 = 0x00000063;
    FLEX_SPI2_FLSHB1CR1 = 0x00000063;
    FLEX_SPI2_FLSHB2CR1 = 0x00000063;
    FLEX_SPI2_DLLACR = 0x00000100;
    FLEX_SPI2_DLLBCR = 0x00000100;
    FLEX_SPI2_STS0 = (FLEX_SPI_STS0_SEQIDLE | FLEX_SPI_STS0_ARBIDLE);
    FLEX_SPI2_STS2 = 0x01000100;
#endif
#if defined MPU_AVAILABLE
    MPU_TYPE = 0x00000800;
#endif
    GPT1_OCR1 = 0xffffffff;
    GPT1_OCR2 = 0xffffffff;
    GPT1_OCR3 = 0xffffffff;
    GPT2_OCR1 = 0xffffffff;
    GPT2_OCR2 = 0xffffffff;
    GPT2_OCR3 = 0xffffffff;

    PIT_MCR = PIT_MCR_MDIS;                                              // PITs disabled
#if QUAD_TIMER_AVAILABLE > 0
    TMR1_ENBL = (TMR_ENBL_CH_0 | TMR_ENBL_CH_1 | TMR_ENBL_CH_2 | TMR_ENBL_CH_3);
#endif
#if QUAD_TIMER_AVAILABLE > 1
    TMR2_ENBL = (TMR_ENBL_CH_0 | TMR_ENBL_CH_1 | TMR_ENBL_CH_2 | TMR_ENBL_CH_3);
#endif
#if QUAD_TIMER_AVAILABLE > 2
    TMR3_ENBL = (TMR_ENBL_CH_0 | TMR_ENBL_CH_1 | TMR_ENBL_CH_2 | TMR_ENBL_CH_3);
#endif
#if QUAD_TIMER_AVAILABLE > 3
    TMR4_ENBL = (TMR_ENBL_CH_0 | TMR_ENBL_CH_1 | TMR_ENBL_CH_2 | TMR_ENBL_CH_3);
#endif
#if (LPUARTS_AVAILABLE > 0)
    LPUART0_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART0_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART0_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 1
    LPUART1_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART1_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART1_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 2
    LPUART2_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART2_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART2_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 3
    LPUART3_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART3_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART3_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 4
    LPUART4_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART4_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART4_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 5
    LPUART5_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART5_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART5_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 6
    LPUART6_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART6_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART6_DATA = (LPUART_DATA_RXEMPT);
#endif
#if LPUARTS_AVAILABLE > 7
    LPUART7_BAUD = (LPUART_BAUD_OSR_16 | 0x00000004);
    LPUART7_STAT = (LPUART_STAT_TDRE | LPUART_STAT_TC);
    LPUART7_DATA = (LPUART_DATA_RXEMPT);
#endif
    LPSPI1_VERID = 0x01010004;
    LPSPI1_PARAM = 0x00000404;
    LPSPI1_SR = LPSPI_SR_TDF;
    LPSPI1_TCR = LPSPI_TCR_FRAMESZ_32;
    LPSPI1_RSR = LPSPI_RSR_RXEMPTY;
#if LPSPI_AVAILABLE > 1
    LPSPI2_VERID = 0x01010004;
    LPSPI2_PARAM = 0x00000404;
    LPSPI2_SR = LPSPI_SR_TDF;
    LPSPI2_TCR = LPSPI_TCR_FRAMESZ_32;
    LPSPI2_RSR = LPSPI_RSR_RXEMPTY;
#endif
#if LPSPI_AVAILABLE > 2
    LPSPI3_VERID = 0x01010004;
    LPSPI3_PARAM = 0x00000404;
    LPSPI3_SR = LPSPI_SR_TDF;
    LPSPI3_TCR = LPSPI_TCR_FRAMESZ_32;
    LPSPI3_RSR = LPSPI_RSR_RXEMPTY;
#endif
#if LPSPI_AVAILABLE > 3
    LPSPI4_VERID = 0x01010004;
    LPSPI4_PARAM = 0x00000404;
    LPSPI4_SR = LPSPI_SR_TDF;
    LPSPI4_TCR = LPSPI_TCR_FRAMESZ_32;
    LPSPI4_RSR = LPSPI_RSR_RXEMPTY;
#endif
#if defined ENHANCED_LCD_INTERFACE
    CCM_ANALOG_PLL_VIDEO = 0x0001100c;
    CCM_ANALOG_PLL_VIDEO_NUM = 0x05f5e100;
    CCM_ANALOG_PLL_VIDEO_DENOM = 0x10a24447;
    LCDIF_CTRL = 0xc0000000;
    LCDIF_CTRL1 = 0x000f0000;
    LCDIF_CTRL2 = LCDIF_CTRL2_OUTSTANDING_REQS_2;
    LCDIF_TRANSFER_COUNT = 0x00010000;
    LCDIF_STAT = 0x95000000;
    LCDIF_LUT_CTRL = 0x00000001;
#endif
#if NUMBER_OF_SDHC > 0
    SDHC_PROCTL    = (SDHC_PROCTL_BURST_LEN_EN_INCR | SDHC_PROCTL_EMODE_LITTLE | 0x00800000); // SDHC1
    SDHC_SYSCTL    = (SDHC_SYSCTL_IPP_RST_N | SDHC_SYSCTL_SDCLKFS_256 | 0x0000000f);
    SDHC_HTCAPBLT  = 0x07f3b407;
    SDHC_WML       = 0x08100810;
    SDHC_MIXCTRL   = 0x80000000;
    SDHC_DLL_STATUS = 0x00000200;
    SDHC_VENDOR    = (0x20007801 | SDHC_VENDOR_AC12_WR_CHKBUSY_EN);
    SDHC_VEND_SPEC2 = 0x00001006;
    SDHC_TUNING_CTRL = 0x00212800;
#endif
#if NUMBER_OF_SDHC > 1
    SDHC2_PROCTL   = (SDHC_PROCTL_BURST_LEN_EN_INCR | SDHC_PROCTL_EMODE_LITTLE | 0x00800000); // SDHC2
    SDHC2_SYSCTL   = (SDHC_SYSCTL_IPP_RST_N | SDHC_SYSCTL_SDCLKFS_256 | 0x0000000f);
    SDHC2_HTCAPBLT = 0x07f3b407;
    SDHC2_WML      = 0x08100810;
    SDHC2_MIXCTRL  = 0x80000000;
    SDHC2_DLL_STATUS = 0x00000200;
    SDHC2_VENDOR   = (0x20007801 | SDHC_VENDOR_AC12_WR_CHKBUSY_EN);
    SDHC2_VEND_SPEC2 = 0x00001006;
    SDHC2_TUNING_CTRL = 0x00212800;
#endif
 #if NUMBER_OF_CAN_INTERFACES > 0
    CAN0_MCR       = 0x5980000f;                                         // FlexCAN
    CAN0_RXGMASK   = 0xffffffff;
    CAN0_RX14MASK  = 0xffffffff;
    CAN0_RX15MASK  = 0xffffffff;
    CAN0_RXFGMASK  = 0xffffffff;
    CAN0_GFWR      = 0x0000007f;
#endif
#if NUMBER_OF_CAN_INTERFACES > 1
    CAN1_MCR       = 0x5980000f;
    CAN1_RXGMASK   = 0xffffffff;
    CAN1_RX14MASK  = 0xffffffff;
    CAN1_RX15MASK  = 0xffffffff;
    CAN1_RXFGMASK  = 0xffffffff;
    CAN1_GFWR      = 0x0000007f;
#endif
#if !defined KINETIS_KL && !defined KINETIS_KM && !defined KINETIS_KE && !defined CROSSBAR_SWITCH_LITE
    DMA_DCHPRI0    = 0;                                                  // default DMA channel priorities
    DMA_DCHPRI1    = 1;
    DMA_DCHPRI2    = 2;
    DMA_DCHPRI3    = 3;
    DMA_DCHPRI4    = 4;
    DMA_DCHPRI5    = 5;
    DMA_DCHPRI6    = 6;
    DMA_DCHPRI7    = 7;
    DMA_DCHPRI8    = 8;
    DMA_DCHPRI9    = 9;
    DMA_DCHPRI10   = 10;
    DMA_DCHPRI11   = 11;
    DMA_DCHPRI12   = 12;
    DMA_DCHPRI13   = 13;
    DMA_DCHPRI14   = 14;
    DMA_DCHPRI15   = 15;
#endif

#if ADC_CONTROLLERS > 0                                                  // ADC1
    ADC1_HC0 = ADC_HC_ADCH_OFF;
    ADC1_HC1 = ADC_HC_ADCH_OFF;
    ADC1_HC2 = ADC_HC_ADCH_OFF;
    ADC1_HC3 = ADC_HC_ADCH_OFF;
    ADC1_HC4 = ADC_HC_ADCH_OFF;
    ADC1_HC5 = ADC_HC_ADCH_OFF;
    ADC1_HC6 = ADC_HC_ADCH_OFF;
    ADC1_HC7 = ADC_HC_ADCH_OFF;
    ADC1_CFG = ADC_CFG_ADSTS_6;
#endif
#if ADC_CONTROLLERS > 1                                                  // ADC2
    ADC2_HC0 = ADC_HC_ADCH_OFF;
    ADC2_HC1 = ADC_HC_ADCH_OFF;
    ADC2_HC2 = ADC_HC_ADCH_OFF;
    ADC2_HC3 = ADC_HC_ADCH_OFF;
    ADC2_HC4 = ADC_HC_ADCH_OFF;
    ADC2_HC5 = ADC_HC_ADCH_OFF;
    ADC2_HC6 = ADC_HC_ADCH_OFF;
    ADC2_HC7 = ADC_HC_ADCH_OFF;
    ADC2_CFG = ADC_CFG_ADSTS_6;
#endif
#if DAC_CONTROLLERS > 0
    DAC0_SR     = DAC_SR_DACBFRPTF;                                      // DAC
    DAC0_C2     = 0x0f;
    #if DAC_CONTROLLERS > 1
    DAC1_SR     = DAC_SR_DACBFRPTF;
    DAC1_C2     = 0x0f;
    #endif
#endif
#if defined RNG_AVAILABLE                                                // random number generator
    #if defined TRUE_RANDOM_NUMBER_GENERATOR
    TRNG0_MCTL = (TRNG_MCTL_PRGM | TRNG_MCTL_TSTOP_OK | TRNG_MCTL_SAMP_MODE_RAW);
    TRNG0_SCMISC = 0x00010022;
    TRNG0_PKRRNG = 0x000009a3;
    TRNG0_PKRMAX = 0x00006920;
    TRNG0_SDCTL = 0x0c8009c4;
    TRNG0_SBLIM = 0x0000003f;
    TRNG0_FRQMIN = 0x00000640;
    TRNG0_FRQMAX = 0x00006400;
    TRNG0_SCML = 0x010c0568;
    TRNG0_SCR1L = 0x00b20195;
    TRNG0_SCR2L = 0x007a00dc;
    TRNG0_SCR3L = 0x0058007d;
    TRNG0_SCR4L = 0x0040004b;
    TRNG0_SCR5L = 0x002e002f;
    TRNG0_SCR6L = 0x002e002f;
    TRNG0_INT_CTRL = 0xffffffff;
    TRNG0_VID1 = 0x00300100;
    #endif
#endif
#if defined ETHERNET_AVAILABLE
    ECR        = 0xf0000000;                                             // ENET
    MIBC       = 0xc0000000;
    RCR        = 0x05ee0001;
    PAUR       = 0x00008808;
    OPD        = 0x00010000;
    ENET_RAEM  = 0x00000004;
    ENET_RAFL  = 0x00000004;
    ENET_TAEM  = 0x00000004;
    ENET_TAFL  = 0x00000008;
    ENET_TIPG  = 0x0000000c;
    ENET_FTRL  = 0x000007ff;
    ENET_ATPER = 0x3b9aca00;
#endif
#if LPI2C_AVAILABLE > 0
    LPI2C0_VERID = 0x01010003;
    LPI2C0_PARAM = 0x00000202;
    LPI2C0_MSR   = 0x00000001;
    LPI2C0_MRDR  = 0x00004000;
    LPI2C0_SASR  = 0x00004000;
    LPI2C0_SRDR  = 0x00004000;
#endif
#if LPI2C_AVAILABLE > 1
    LPI2C1_VERID = 0x01010003;
    LPI2C1_PARAM = 0x00000202;
    LPI2C1_MSR = 0x00000001;
    LPI2C1_MRDR = 0x00004000;
    LPI2C1_SASR = 0x00004000;
    LPI2C1_SRDR = 0x00004000;
#endif
#if LPI2C_AVAILABLE > 2
    LPI2C2_VERID = 0x01010003;
    LPI2C2_PARAM = 0x00000202;
    LPI2C2_MSR = 0x00000001;
    LPI2C2_MRDR = 0x00004000;
    LPI2C2_SASR = 0x00004000;
    LPI2C2_SRDR = 0x00004000;
#endif
#if LPI2C_AVAILABLE > 3
    LPI2C3_VERID = 0x01010003;
    LPI2C3_PARAM = 0x00000202;
    LPI2C3_MSR = 0x00000001;
    LPI2C3_MRDR = 0x00004000;
    LPI2C3_SASR = 0x00004000;
    LPI2C3_SRDR = 0x00004000;
#endif
    initial_input_state = fnKeyPadState(*port_inits++, _PORT1);          // allow key input defaults override port input defaults
    GPIO1_PSR  = (PORT_CAST)(ulPort_in_1 = initial_input_state);         // set port inputs to default states
#if PORTS_AVAILABLE > 1
    initial_input_state = fnKeyPadState(*port_inits++, _PORT2);
    GPIO2_PSR  = (PORT_CAST)(ulPort_in_2 = initial_input_state);
#endif
#if defined PORT3 && (PORTS_AVAILABLE > 2)
    initial_input_state = fnKeyPadState(*port_inits++, _PORT3);
    GPIO3_PSR  = (PORT_CAST)(ulPort_in_3 = initial_input_state);
#endif
#if (defined iMX_RT105X || defined iMX_RT106X) && (PORTS_AVAILABLE > 3)
    initial_input_state = fnKeyPadState(*port_inits++, _PORT4);
    GPIO4_PSR = (PORT_CAST)(ulPort_in_4 = initial_input_state);
#endif
#if PORTS_AVAILABLE > 4
    initial_input_state = fnKeyPadState(*port_inits++, _PORT5);
    GPIO5_PSR  = (PORT_CAST)(ulPort_in_5 = initial_input_state);
#endif
#if PORTS_AVAILABLE > 5
    initial_input_state = fnKeyPadState(*port_inits++, _PORT6);
    GPIO6_PSR  = (PORT_CAST)(ulPort_in_6 = initial_input_state);
#endif
#if PORTS_AVAILABLE > 6
    initial_input_state = fnKeyPadState(*port_inits++, _PORT7);
    GPIO7_PSR = (PORT_CAST)(ulPort_in_7 = initial_input_state);
#endif
#if PORTS_AVAILABLE > 7
    initial_input_state = fnKeyPadState(*port_inits++, _PORT8);
    GPIO8_PSR = (PORT_CAST)(ulPort_in_8 = initial_input_state);
#endif
#if PORTS_AVAILABLE > 8
    initial_input_state = fnKeyPadState(*port_inits++, _PORT9);
    GPIO9_PSR = (PORT_CAST)(ulPort_in_9 = initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 0) && defined PORT_EXT0_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT0_DEFAULT_INPUT, _PORT_EXT_0);
    fnSetI2CPort(_PORT_EXT_0, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 1) && defined PORT_EXT1_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT1_DEFAULT_INPUT, _PORT_EXT_1);
    fnSetI2CPort(_PORT_EXT_1, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 2) && defined PORT_EXT2_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT2_DEFAULT_INPUT, _PORT_EXT_2);
    fnSetI2CPort(_PORT_EXT_2, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 3) && defined PORT_EXT3_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT3_DEFAULT_INPUT, _PORT_EXT_3);
    fnSetI2CPort(_PORT_EXT_3, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 4) && defined PORT_EXT0_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT4_DEFAULT_INPUT, _PORT_EXT_4);
    fnSetI2CPort(_PORT_EXT_4, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 5) && defined PORT_EXT1_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT5_DEFAULT_INPUT, _PORT_EXT_5);
    fnSetI2CPort(_PORT_EXT_5, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 6) && defined PORT_EXT2_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT6_DEFAULT_INPUT, _PORT_EXT_6);
    fnSetI2CPort(_PORT_EXT_6, DEFINE_INPUT, initial_input_state);
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 7) && defined PORT_EXT3_DEFAULT_INPUT
    initial_input_state = fnKeyPadState(PORT_EXT7_DEFAULT_INPUT, _PORT_EXT_7);
    fnSetI2CPort(_PORT_EXT_7, DEFINE_INPUT, initial_input_state);
#endif
#if !defined KINETIS_KL && !defined KINETIS_KE && (FLEX_TIMERS_AVAILABLE > 0)
    FTM0_MODE = FTM_MODE_WPDIS;                                          // FlexTimer
    FTM1_MODE = FTM_MODE_WPDIS;
    FTM2_MODE = FTM_MODE_WPDIS;
#endif
#if defined SEMC_AVAILABLE
    SEMC_MCR = (SEMC_MCR_BTO_DEFAULT | SEMC_MCR_MDIS); 
    SEMC_BR0 = 0x8000001d;
    SEMC_BR1 = 0x8400001c;
    SEMC_BR2 = 0x8800001c;
    SEMC_BR3 = 0x8c00001c;
    SEMC_BR4 = 0x9e00001a;
    SEMC_BR5 = 0x90000018;
    SEMC_BR6 = 0x98000018;
    SEMC_BR7 = 0x9c00001a;
    SEMC_BR8 = 0x00000026;
    SEMC_SDRAMCR0 = 0x00000c26;
    SEMC_SDRAMCR1 = 0x00994934;
    SEMC_SDRAMCR2 = 0x80000eee;
    SEMC_SDRAMCR3 = 0x40808000;
    SEMC_NANDCR2 = 0x00010410;
    SEMC_STS0 = 0x00000001;
#endif
    USBHS0_ID = 0xe4a1fa05;
    USBHS0_HWGENERAL = 0x00000035;
    USBHS0_HWHOST = 0x10020001;
    USBHS0_HWDEVICE = 0x00000011;
    USBHS0_HWTXBUF = 0x80080b08;
    USBHS0_HWRXBUF = 0x00000808;
    USBHS0_USB_SBUSCFG = 0x00000002;
    USBHS0_HCIVERSION = 0x01000040;
    USBHS0_HCSPARAMS = 0x00010011;
    USBHS0_HCCPARAMS = 0x00000006;
    USBHS0_DCIVERSION = 0x0001;
    USBHS0_DCCPARAMS = 0x00000188;
    USBHS0_USBCMD = 0x00080000;
    USBHS0_BURSTSIZE = 0x00000808;
    USBHS0_CONFIGFLAG = 0x00000001;
    USBHS0_PORTSC1 = 0x10000000;
    USBHS0_OTGSC = 0x00001120;
    USBHS0_USBMODE = 0x00005000;
    USBHS0_EPCR0 = 0x00800080;
    USBPHY1_PWD = (USBPHY_PWD_RXPWDRX | USBPHY_PWD_RXPWDDIFF | USBPHY_PWD_RXPWD1PT1 | USBPHY_PWD_RXPWDENV | USBPHY_PWD_TXPWDV2I | USBPHY_PWD_TXPWDIBIAS | USBPHY_PWD_TXPWDFS);
    USBPHY1_TX = 0x10060607;
    USBPHY1_CTRL = (USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE | USBPHY_CTRL_ENDPDMCHG_WKUP);
    USBPHY1_DEBUG = 0x7f180000;
    USBPHY1_DEBUG1 = 0x00001000;
    USBPHY1_VERSION = 0x04030000;
    USBNC_USB_OTG1_CTRL = 0x30001000;
    USBNC_USB_OTG1_PHY_CTRL_0 = 0;
#if HSUSB_CONTROLLERS > 1
    USBHS1_ID = 0xe4a1fa05;
    USBHS1_HWGENERAL = 0x00000035;
    USBHS1_HWHOST = 0x10020001;
    USBHS1_HWDEVICE = 0x00000011;
    USBHS1_HWTXBUF = 0x80080b08;
    USBHS1_HWRXBUF = 0x00000808;
    USBHS1_USB_SBUSCFG = 0x00000002;
    USBHS1_HCIVERSION = 0x01000040;
    USBHS1_HCSPARAMS = 0x00010011;
    USBHS1_HCCPARAMS = 0x00000006;
    USBHS1_DCIVERSION = 0x0001;
    USBHS1_DCCPARAMS = 0x00000188;
    USBHS1_USBCMD = 0x00080000;
    USBHS1_BURSTSIZE = 0x00000808;
    USBHS1_CONFIGFLAG = 0x00000001;
    USBHS1_PORTSC1 = 0x10000000;
    USBHS1_OTGSC = 0x00001120;
    USBHS1_USBMODE = 0x00005000;
    USBHS1_EPCR0 = 0x00800080;
    USBPHY2_PWD = (USBPHY_PWD_RXPWDRX | USBPHY_PWD_RXPWDDIFF | USBPHY_PWD_RXPWD1PT1 | USBPHY_PWD_RXPWDENV | USBPHY_PWD_TXPWDV2I | USBPHY_PWD_TXPWDIBIAS | USBPHY_PWD_TXPWDFS);
    USBPHY2_TX = 0x10060607;
    USBPHY2_CTRL = (USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE | USBPHY_CTRL_ENDPDMCHG_WKUP);
    USBPHY2_DEBUG = 0x7f180000;
    USBPHY2_DEBUG1 = 0x00001000;
    USBPHY2_VERSION = 0x04030000;
    USBNC_USB_OTG2_CTRL = 0x30001000;
    USBNC_USB_OTG2_PHY_CTRL_0 = 0x00000098;
#endif
#if defined CHIP_HAS_FLEXIO
    FLEXIO0_VERID = 0x01010001;
    FLEXIO0_PARAM = 0x04200808;
#endif
    FLEXPWM1_SM0CTRL = 0x0400;
    FLEXPWM1_SM0DISPMAP0 = 0xffff;
    FLEXPWM1_SM0DISPMAP1 = 0xffff;
    FLEXPWM1_SM0DTCNT0 = 0x07ff;
    FLEXPWM1_SM0DTCNT1 = 0x07ff;
#if defined FLEXPWM_AVAILABLE > 1
    FLEXPWM2_SM0CTRL = 0x0400;
    FLEXPWM2_SM0DISPMAP0 = 0xffff;
    FLEXPWM2_SM0DISPMAP1 = 0xffff;
    FLEXPWM2_SM0DTCNT0 = 0x07ff;
    FLEXPWM2_SM0DTCNT1 = 0x07ff;
#endif
#if defined FLEXPWM_AVAILABLE > 2
    FLEXPWM3_SM0CTRL = 0x0400;
    FLEXPWM3_SM0DISPMAP0 = 0xffff;
    FLEXPWM3_SM0DISPMAP1 = 0xffff;
    FLEXPWM3_SM0DTCNT0 = 0x07ff;
    FLEXPWM3_SM0DTCNT1 = 0x07ff;
#endif
#if defined FLEXPWM_AVAILABLE > 3
    FLEXPWM4_SM0CTRL = 0x0400;
    FLEXPWM4_SM0DISPMAP0 = 0xffff;
    FLEXPWM4_SM0DISPMAP1 = 0xffff;
    FLEXPWM4_SM0DTCNT0 = 0x07ff;
    FLEXPWM4_SM0DTCNT1 = 0x07ff;
#endif
#if !defined KINETIS_KL && !defined KINETIS_KE
    CRC_CRC   = 0xffffffff;                                              // CRC
    CRC_GPOLY = 0x00001021;                                              // default CRC-16 polynomial
#endif
#if defined LLWU_AVAILABLE && !defined KINETIS_KL03 && !defined KINETIS_KL05
    LLWU_RST = LLWU_RST_LLRSTE;
#endif
    fnSimPers();                                                         // synchronise peripheral states with default settings
#if defined SUPPORT_ADC
    for (j = 0; j < ADC_CONTROLLERS; j++) {
        for (i = 0; i < ADC_CHANNELS; i++) {
            usADC_values[j][i] = (unsigned short)*port_inits++;          // prime initial ADC values
        }
      //usADC_values[j][ADC_TEMP_SENSOR] = (unsigned short)((VTEMP_25_MV * 0xffff) / ADC_REFERENCE_VOLTAGE); // 25°C
      //usADC_values[j][ADC_BANDGAP] = (unsigned short)((1800 / ADC_REFERENCE_VOLTAGE) * 0xffff); // 1.8V
        usADC_values[j][ADC_VREFH] = 0xffff;
    }
    fnEnterHW_table(ptrPorts);                                           
#else
    fnEnterHW_table(ucPortFunctions);
#endif
}

#define SIZE_OF_SIM_FLASH (FLEXSPI_FLASH_SIZE)

unsigned char ucFLASH[SIZE_OF_SIM_FLASH];

extern void fnInitialiseDevice(void *port_inits)
{
    memset(ucFLASH, 0xff, sizeof(ucFLASH));                              // we start with deleted FLASH memory contents (use memset() rather than uMemset() since the DMA controller may not be initialised yet)
    fnPrimeFileSystem();                                                 // the project can then set parameters or files as required
    fnSetDevice((unsigned long *)port_inits);                            // set device registers to startup condition (if not zerod)
}


extern unsigned char *fnGetEEPROM(unsigned short usOffset);

extern unsigned char *fnGetFileSystemStart(int iMemory)
{
#if defined SPI_FILE_SYSTEM && !defined FLASH_FILE_SYSTEM
    return (fnGetEEPROM(uFILE_START));
#elif defined (SAVE_COMPLETE_FLASH)
    return (&ucFLASH[0]);
#else
    return (&ucFLASH[uFILE_START - FLASH_START_ADDRESS]);
#endif
}


extern unsigned long fnGetFlashSize(int iMemory)
{
#if defined SPI_FILE_SYSTEM && !defined FLASH_FILE_SYSTEM
    return (fnGetEEPROMSize());
#elif defined (SAVE_COMPLETE_FLASH)
    return (sizeof(ucFLASH));
#else
    return (FILE_SYSTEM_SIZE);
#endif
}


// Transform physical access address to address in simulated FLASH
//
extern unsigned char *fnGetFlashAdd(unsigned char *ucAdd)               
{
    unsigned char *ucSimulatedFlash = fnGetDataFlashStart();
    ucAdd -= (unsigned long)FLEXSPI_FLASH_BASE;
    if ((unsigned long)ucAdd >= fnGetDataFlashSize()) { // check flash bounds
        _EXCEPTION("Attempted access outside of QSPI Flash bounds!!!");
    }
#if defined CRYPTOGRAPHY
    #if defined iMX_RT1011
    if ((OTFAD_CR & OTFAD_CR_GE) != 0) {                                 // if on-the-fly decryption is active
        static unsigned char ucDecryptedFlash[2][0x4000];
        static unsigned long ulDecryptedRangeStart[2] = { 0 };
        static unsigned long ulDecryptedRangeEnd[2] = { 0 };
        static int           iDecryptedContentValid[2] = { 0 };
        static unsigned long ulKeyRef[2] = { 0 };
        unsigned long ulPresentKey = (HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2);
        unsigned long ulStart = OTFAD_CTX0_RGD_W0;
        unsigned long ulEnd = OTFAD_CTX0_RGD_W1;
        unsigned long ulThisAddress = (unsigned long)((unsigned long)ucAdd + (unsigned long)FLEXSPI_FLASH_BASE);
        if ((ulThisAddress >= ulStart) && (ulThisAddress < ulEnd)) {     // if the read is from the decrypted area
            if ((iDecryptedContentValid[0] == 0) || (ulKeyRef[0] != ulPresentKey) || (ulThisAddress < ulDecryptedRangeStart[0]) || (ulThisAddress > ulDecryptedRangeEnd[0])) {
                ulKeyRef[0] = ulPresentKey;
                ulDecryptedRangeStart[0] = ulThisAddress;
                ulDecryptedRangeEnd[0] = (ulThisAddress + sizeof(ucDecryptedFlash[0]));
                iDecryptedContentValid[0] = 1;
                fnDoOnTheFlyDecryption((unsigned char *)ulThisAddress, ucDecryptedFlash[0], sizeof(ucDecryptedFlash[0]), 0);
            }
            ucSimulatedFlash = ucDecryptedFlash[0];
            ucSimulatedFlash += (ulThisAddress - ulDecryptedRangeStart[0]);
            return (ucSimulatedFlash);                                   // location in simulated decrypted FLASH memory
        }
        ulPresentKey = (HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_BEE_KEY1_SEL_SW_GP2);
        ulStart = OTFAD_CTX1_RGD_W0;
        ulEnd = OTFAD_CTX1_RGD_W1;
        ulThisAddress = (unsigned long)((unsigned long)ucAdd + (unsigned long)FLEXSPI_FLASH_BASE);
        if ((ulThisAddress >= ulStart) && (ulThisAddress < ulEnd)) {     // if the read is from the decrypted area
            if ((iDecryptedContentValid[1] == 0) || (ulKeyRef[1] != ulPresentKey) || (ulThisAddress < ulDecryptedRangeStart[1]) || (ulThisAddress > ulDecryptedRangeEnd[1])) {
                ulKeyRef[1] = ulPresentKey;
                ulDecryptedRangeStart[1] = ulThisAddress;
                ulDecryptedRangeEnd[1] = (ulThisAddress + sizeof(ucDecryptedFlash[1]));
                iDecryptedContentValid[1] = 1;
                fnDoOnTheFlyDecryption((unsigned char *)ulThisAddress, ucDecryptedFlash[1], sizeof(ucDecryptedFlash[1]), 1);
            }
            ucSimulatedFlash = ucDecryptedFlash[1];
            ucSimulatedFlash += (ulThisAddress - ulDecryptedRangeStart[1]);
            return (ucSimulatedFlash);                                   // location in simulated decrypted FLASH memory
        }
    }
    #else
    if ((BEE_CTRL & BEE_CTRL_BEE_ENABLE) != 0) {                         // if BEE is enabled
        if ((IOMUXC_GPR_GPR11 & IOMUXC_GPR_GPR11_BEE_DE_RX_EN_0) != 0) { // if on-the-fly decryption is active
            static unsigned char ucDecryptedFlash[2][0x4000];
            static unsigned long ulDecryptedRangeStart[2] = { 0 };
            static unsigned long ulDecryptedRangeEnd[2] = { 0 };
            static int           iDecryptedContentValid[2] = { 0 };
            static unsigned long ulKeyRef[2] = { 0 };
            unsigned long ulPresentKey = (HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2);
            unsigned long ulStart = IOMUXC_GPR_GPR18;
            unsigned long ulEnd = IOMUXC_GPR_GPR19;
            unsigned long ulThisAddress = (unsigned long)((unsigned long)ucAdd + (unsigned long)FLEXSPI_FLASH_BASE);
            if ((ulThisAddress >= ulStart) && (ulThisAddress < ulEnd)) { // if the read is from the decrypted area
                if ((iDecryptedContentValid[0] == 0) || (ulKeyRef[0] != ulPresentKey) || (ulThisAddress < ulDecryptedRangeStart[0]) || (ulThisAddress > ulDecryptedRangeEnd[0])) {
                    ulKeyRef[0] = ulPresentKey;
                    ulDecryptedRangeStart[0] = ulThisAddress;
                    ulDecryptedRangeEnd[0] = (ulThisAddress + sizeof(ucDecryptedFlash[0]));
                    iDecryptedContentValid[0] = 1;
                    fnDoOnTheFlyDecryption((unsigned char *)ulThisAddress, ucDecryptedFlash[0], sizeof(ucDecryptedFlash[0]), 0);
                }
                ucSimulatedFlash = ucDecryptedFlash[0];
                ucSimulatedFlash += (ulThisAddress - ulDecryptedRangeStart[0]);
                return (ucSimulatedFlash);                               // location in simulated decrypted FLASH memory
            }
            ulPresentKey = (HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_BEE_KEY1_SEL_SW_GP2);
            ulStart = IOMUXC_GPR_GPR20;
            ulEnd = IOMUXC_GPR_GPR21;
            ulThisAddress = (unsigned long)((unsigned long)ucAdd + (unsigned long)FLEXSPI_FLASH_BASE);
            if ((ulThisAddress >= ulStart) && (ulThisAddress < ulEnd)) { // if the read is from the decrypted area
                if ((iDecryptedContentValid[1] == 0) || (ulKeyRef[1] != ulPresentKey) || (ulThisAddress < ulDecryptedRangeStart[1]) || (ulThisAddress > ulDecryptedRangeEnd[1])) {
                    ulKeyRef[1] = ulPresentKey;
                    ulDecryptedRangeStart[1] = ulThisAddress;
                    ulDecryptedRangeEnd[1] = (ulThisAddress + sizeof(ucDecryptedFlash[1]));
                    iDecryptedContentValid[1] = 1;
                    fnDoOnTheFlyDecryption((unsigned char *)ulThisAddress, ucDecryptedFlash[1], sizeof(ucDecryptedFlash[1]), 1);
                }
                ucSimulatedFlash = ucDecryptedFlash[1];
                ucSimulatedFlash += (ulThisAddress - ulDecryptedRangeStart[1]);
                return (ucSimulatedFlash);                               // location in simulated decrypted FLASH memory
            }
        }
    }
    #endif
#endif
    ucSimulatedFlash += (unsigned long)ucAdd;
    return (ucSimulatedFlash);                                           // location in simulated internal FLASH memory
}

// Transform physical access address to address in simulated FLASH (ignoring decryption mode)
//
extern unsigned char *fnGetFlashRealAdd(unsigned char *ucAdd)
{
    unsigned char *ucSimulatedFlash = fnGetDataFlashStart();
    ucAdd -= (unsigned long)FLEXSPI_FLASH_BASE;
    if ((unsigned long)ucAdd >= fnGetDataFlashSize()) { // check flash bounds
        _EXCEPTION("Attempted access outside of QSPI Flash bounds!!!");
    }
    ucSimulatedFlash += (unsigned long)ucAdd;
    return (ucSimulatedFlash);                                           // location in simulated internal FLASH memory
}

// Transform simulated address in simulated FLASH back to physical address in real FLASH
//
extern unsigned char *fnPutFlashAdd(unsigned char *ucAdd)
{
    unsigned long sim_add = (unsigned long)ucFLASH - (unsigned long)FLEXSPI_FLASH_BASE;
    return (ucAdd - sim_add);
}

extern unsigned char fnMapPortBit(unsigned long ulRealBit)
{
    unsigned long ulBit = 0x80000000;
    unsigned char ucRef = 0;

    while (ulBit) {
        if (ulRealBit & ulBit) {
            break;
        }
        ulBit >>= 1;
        ucRef++;
    }
    return ucRef;
}


extern unsigned char *fnGetIMX_OPTstart(void)
{
    return ((unsigned char *)&iMX_OPT);
}

extern unsigned long fnGetIMX_OPTsize(void)
{
    return (sizeof(iMX_OPT));
}

// Periodic tick (on timer 1) - dummy since timer simulation used now
//
extern void RealTimeInterrupt(void)
{
}


// Check whether a particular interrupt is enabled in the NVIC
//
static int fnGenInt(int iIrqID)
{
    if (iIrqID < 32) {
        IRQ0_31_SPR |= (1 << iIrqID);
        IRQ0_31_CPR = IRQ0_31_SPR;
        if ((IRQ0_31_SER & (1 << iIrqID)) != 0) {                        // if interrupt is not disabled
            return 1;
        }
    }
    else if (iIrqID < 64) {
        IRQ32_63_SPR |= (1 << (iIrqID - 32));
        IRQ32_63_CPR = IRQ32_63_SPR;
        if ((IRQ32_63_SER & (1 << (iIrqID - 32))) != 0) {                // if interrupt is not disabled
            return 1;
        }
    }
    else if (iIrqID < 96) {
        IRQ64_95_SPR |= (1 << (iIrqID - 64));
        IRQ64_95_CPR = IRQ64_95_SPR;
        if ((IRQ64_95_SER & (1 << (iIrqID - 64))) != 0) {                // if interrupt is not disabled
            return 1;
        }
    }
    else if (iIrqID < 128) {
        IRQ96_127_SPR |= (1 << (iIrqID - 96));
        IRQ96_127_CPR = IRQ96_127_SPR;
        if ((IRQ96_127_SER & (1 << (iIrqID - 96))) != 0) {               // if interrupt is not disabled
            return 1;
        }
    }
    else if (iIrqID < 160) {
        IRQ128_159_SPR |= (1 << (iIrqID - 128));
        IRQ128_159_CPR = IRQ128_159_SPR;
        if ((IRQ128_159_SER & (1 << (iIrqID - 128))) != 0) {             // if interrupt is not disabled
            return 1;
        }
    }
    else if (iIrqID < 192) {
        IRQ160_191_SPR |= (1 << (iIrqID - 164));
        IRQ160_191_CPR = IRQ160_191_SPR;
        if ((IRQ160_191_SER & (1 << (iIrqID - 164))) != 0) {             // if interrupt is not disabled
            return 1;
        }
    }
    return 0;
}

// Reset a particular interrupt in the NVIC
//
extern int fnResetNVICInt(int iIrqID)
{
    if (iIrqID < 32) {
        IRQ0_31_SPR &= ~(1 << iIrqID);
    }
    else if (iIrqID < 64) {
        IRQ32_63_SPR &= ~(1 << (iIrqID - 32));
    }
    else if (iIrqID < 96) {
        IRQ64_95_SPR &= ~(1 << (iIrqID - 64));
    }
    else if (iIrqID < 128) {
        IRQ96_127_SPR &= ~(1 << (iIrqID - 96));
    }
    else if (iIrqID < 160) {
        IRQ128_159_SPR |= (1 << (iIrqID - 128));
    }
    else if (iIrqID < 192) {
        IRQ160_191_SPR &= ~(1 << (iIrqID - 164));
    }
    return 0;
}

#if defined FAST_GPIO_AVAILABLE
extern int fnGetFastport(int iPortNumber, unsigned long ulPortBit)
{
    switch (iPortNumber) {
    case _PORT1:
        if ((IOMUXC_GPR_GPR26 & ulPortBit) != 0) {
            return (iPortNumber + 5);
        }
        break;
    case _PORT2:
        if ((IOMUXC_GPR_GPR27 & ulPortBit) != 0) {
            return (iPortNumber + 5);
        }
        break;
    case _PORT3:
        if ((IOMUXC_GPR_GPR28 & ulPortBit) != 0) {
            return (iPortNumber + 5);
        }
        break;
    case _PORT4:
        if ((IOMUXC_GPR_GPR29 & ulPortBit) != 0) {
            return (iPortNumber + 5);
        }
        break;
    case _PORT6:
        if ((IOMUXC_GPR_GPR26 & ulPortBit) != 0) {
            return (iPortNumber - 5);
        }
        break;
    case _PORT7:
        if ((IOMUXC_GPR_GPR27 & ulPortBit) != 0) {
            return (iPortNumber - 5);
        }
        break;
    case _PORT8:
        if ((IOMUXC_GPR_GPR28 & ulPortBit) != 0) {
            return (iPortNumber - 5);
        }
        break;
    case _PORT9:
        if ((IOMUXC_GPR_GPR29 & ulPortBit) != 0) {
            return (iPortNumber - 5);
        }
        break;
    }
    return iPortNumber;                                                  // no mapping
}
#endif

// Get the present state of a port
//
extern unsigned long fnGetPresentPortState(int portNr)
{
    portNr--;
    switch (portNr) {
    case _PORT1:
        return ((GPIO1_GDIR & GPIO1_PSR) | (~GPIO1_GDIR & ulPort_in_1));
#if PORTS_AVAILABLE > 1
    case _PORT2:
        return ((GPIO2_GDIR & GPIO2_PSR) | (~GPIO2_GDIR & ulPort_in_2));
#endif
#if PORTS_AVAILABLE > 2
    case _PORT3:
    #if defined PORT3
        return ((GPIO3_GDIR & GPIO3_PSR) | (~GPIO3_GDIR & ulPort_in_3));
    #else
        return 0;
    #endif
#endif
#if (defined iMX_RT105X || defined iMX_RT106X) && (PORTS_AVAILABLE > 3)
    case _PORT4:
        return ((GPIO4_GDIR & GPIO4_PSR) | (~GPIO4_GDIR & ulPort_in_4));
#endif
#if PORTS_AVAILABLE > 4
    case _PORT5:
        return ((GPIO5_GDIR & GPIO5_PSR) | (~GPIO5_GDIR & ulPort_in_5));
#endif
#if defined PORT6
    case _PORT6:
        return ((GPIO6_GDIR & GPIO6_PSR) | (~GPIO6_GDIR & ulPort_in_6));
#endif
#if defined PORT7
    case _PORT7:
        return ((GPIO7_GDIR & GPIO7_PSR) | (~GPIO7_GDIR & ulPort_in_7));
#endif
#if defined PORT8
    case _PORT8:
        return ((GPIO8_GDIR & GPIO8_PSR) | (~GPIO8_GDIR & ulPort_in_8));
#endif
#if defined PORT9
    case _PORT9:
        return ((GPIO9_GDIR & GPIO9_PSR) | (~GPIO9_GDIR & ulPort_in_9));
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 0)
    case _PORT_EXT_0:                                                    // external ports extensions
    case _PORT_EXT_1:
    case _PORT_EXT_2:
    case _PORT_EXT_3:
    case _PORT_EXT_4:
    case _PORT_EXT_5:
    case _PORT_EXT_6:
    case _PORT_EXT_7:
        return (fnGetExtPortState(portNr));                              // pin states of external port
#endif
    default:
        return 0;
    }
}

// Get the present state of a port direction
//
extern unsigned long fnGetPresentPortDir(int portNr)
{
    unsigned long ulConnectedGPIO;
    unsigned long ulCheckedPin;
    unsigned long ulBit = 0x00000001;
    portNr--;
    switch (portNr) {
    case _PORT1:
        ulCheckedPin = ulConnectedGPIO = (GPIO1_GDIR);
        break;
#if PORTS_AVAILABLE > 1
    case _PORT2:
        ulCheckedPin = ulConnectedGPIO = (GPIO2_GDIR);
        break;
#endif
#if PORTS_AVAILABLE > 2
    case _PORT3:
    #if defined PORT3
        ulCheckedPin = ulConnectedGPIO = (GPIO3_GDIR);
    #else
        ulCheckedPin = ulConnectedGPIO = 0;
    #endif
        break;
#endif
#if PORTS_AVAILABLE > 3 && defined PORT4
    case _PORT4:
        ulCheckedPin = ulConnectedGPIO = (GPIO4_GDIR);
        break;
#endif
#if PORTS_AVAILABLE > 4
    case _PORT5:
        ulCheckedPin = ulConnectedGPIO = (GPIO5_GDIR);
        break;
#endif
#if defined PORT6
    case _PORT6:
        ulCheckedPin = ulConnectedGPIO = (GPIO6_GDIR);
        break;
#endif
#if defined PORT7
    case _PORT7:
        ulCheckedPin = ulConnectedGPIO = (GPIO7_GDIR);
        break;
#endif
#if defined PORT8
    case _PORT8:
        ulCheckedPin = ulConnectedGPIO = (GPIO8_GDIR);
        break;
#endif
#if defined PORT9
    case _PORT9:
        ulCheckedPin = ulConnectedGPIO = (GPIO9_GDIR);
        break;
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 0)
    case _PORT_EXT_0:                                                    // external ports extensions
    case _PORT_EXT_1:
    case _PORT_EXT_2:
    case _PORT_EXT_3:
    case _PORT_EXT_4:
    case _PORT_EXT_5:
    case _PORT_EXT_6:
    case _PORT_EXT_7:
        return (fnGetExtPortDirection(portNr));
#endif
    default:
        return 0;
    }
    return (ulConnectedGPIO);                                            // connected ports configured as GPIO outputs
}


extern unsigned long fnGetPresentPortPeriph(int portNr)
{
    portNr--;
    switch (portNr) {
    case _PORT1:
        return (ulPeripherals[PORT1]);
    case _PORT2:
        return (ulPeripherals[PORT2]);
#if defined PORT3
    case _PORT3:
        return (ulPeripherals[PORT3]);
#endif
#if defined PORT4
    case _PORT4:
        return (ulPeripherals[PORT4]);
#endif
    case _PORT5:
        return (ulPeripherals[PORT5]);
#if defined PORT6
    case _PORT6:
        return (ulPeripherals[PORT6]);
#endif
#if defined PORT7
    case _PORT7:
        return (ulPeripherals[PORT7]);
#endif
#if defined PORT8
    case _PORT8:
        return (ulPeripherals[PORT8]);
#endif
#if defined PORT9
    case _PORT9:
        return (ulPeripherals[PORT9]);
#endif
    default:
        return 0;
    }
}


// See whether there has been a port change which should be displayed
//
extern int fnPortChanges(int iForce)
{
    int iRtn = iFlagRefresh;
    static unsigned long ulPortVal0 = 0, ulPortVal1 = 0, ulPortVal2 = 0, ulPortVal3 = 0, ulPortVal4 = 0;
    static unsigned long ulPortFunction0 = 0, ulPortFunction1 = 0, ulPortFunction2 = 0, ulPortFunction3 = 0, ulPortFunction4 = 0;
#if PORTS_AVAILABLE > 5
    static unsigned long ulPortFunction5 = 0; 
    static unsigned long ulPortVal5 = 0;
#endif
#if PORTS_AVAILABLE > 6
    static unsigned long ulPortFunction6 = 0;
    static unsigned long ulPortVal6 = 0;
#endif
#if PORTS_AVAILABLE > 7
    static unsigned long ulPortFunction7 = 0;
    static unsigned long ulPortVal7 = 0;
#endif
#if PORTS_AVAILABLE > 8
    static unsigned long ulPortFunction8 = 0;
    static unsigned long ulPortVal8 = 0;
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 0)
    static unsigned long ulPortExtValue[_EXTERNAL_PORT_COUNT] = {0};
    int iExPort = 0;
#endif
    unsigned long ulNewValue;
    unsigned long ulNewPortPer;
    iFlagRefresh = 0;

    ulNewValue = fnGetPresentPortState(_PORT1 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT1 + 1);
    if ((ulNewValue != ulPortVal0) || (ulNewPortPer != ulPortFunction0)) {
        ulPortVal0 = ulNewValue;
        ulPortFunction0 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
    ulNewValue = fnGetPresentPortState(_PORT2 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT2 + 1);
    if ((ulNewValue != ulPortVal1) || (ulNewPortPer != ulPortFunction1)) {
        ulPortVal1 = ulNewValue;
        ulPortFunction1 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT3 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT3 + 1);
    if ((ulNewValue != ulPortVal2) || (ulNewPortPer != ulPortFunction2)) {
        ulPortVal2 = ulNewValue;
        ulPortFunction2 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT4 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT4 + 1);
    if ((ulNewValue != ulPortVal3) || (ulNewPortPer != ulPortFunction3)) {
        ulPortVal3 = ulNewValue;
        ulPortFunction3 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }   
    ulNewValue = fnGetPresentPortState(_PORT5 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT5 + 1);
    if ((ulNewValue != ulPortVal4) || (ulNewPortPer != ulPortFunction4)) {
        ulPortVal4 = ulNewValue;
        ulPortFunction4 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
#if PORTS_AVAILABLE > 5
    ulNewValue = fnGetPresentPortState(_PORT6 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT6 + 1);
    if ((ulNewValue != ulPortVal5) || (ulNewPortPer != ulPortFunction5)) {
        ulPortVal5 = ulNewValue;
        ulPortFunction5 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
#endif
#if PORTS_AVAILABLE > 6
    ulNewValue = fnGetPresentPortState(_PORT7 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT7 + 1);
    if ((ulNewValue != ulPortVal6) || (ulNewPortPer != ulPortFunction6)) {
        ulPortVal6 = ulNewValue;
        ulPortFunction6 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
#endif
#if PORTS_AVAILABLE > 7
    ulNewValue = fnGetPresentPortState(_PORT8 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT8 + 1);
    if ((ulNewValue != ulPortVal7) || (ulNewPortPer != ulPortFunction7)) {
        ulPortVal7 = ulNewValue;
        ulPortFunction7 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
#endif
#if PORTS_AVAILABLE > 8
    ulNewValue = fnGetPresentPortState(_PORT9 + 1);
    ulNewPortPer = fnGetPresentPortPeriph(_PORT9 + 1);
    if ((ulNewValue != ulPortVal8) || (ulNewPortPer != ulPortFunction8)) {
        ulPortVal8 = ulNewValue;
        ulPortFunction8 = ulNewPortPer;
        iRtn |= PORT_CHANGE;
    }
#endif
#if defined _EXTERNAL_PORT_COUNT && (_EXTERNAL_PORT_COUNT > 0)
    while (iExPort < _EXTERNAL_PORT_COUNT) {                             // external ports extensions
        ulNewValue = fnGetPresentPortState(iExPort + (_PORT_EXT_0 + 1));
        if (ulNewValue != ulPortExtValue[iExPort]) {
            ulPortExtValue[iExPort] = ulNewValue;
            iRtn |= PORT_CHANGE;
        }
        iExPort++;
    }
#endif
    return iRtn;
}


#if defined SUPPORT_ADC_
static unsigned short fnConvertSimADCvalue(iMX_ADC_REGS *ptrADC, unsigned short usStandardValue)
{
    switch (ptrADC->ADC_CFG & ADC_WORD_LENGTH_MASK) {
    default:
        _EXCEPTION("ADC doesn't support 16 bit mode");
        break;
    case ADC_CFG_MODE_12:                                                // conversion mode - single-ended 12 bit or differential 13 bit
        usStandardValue >>= 4;                                           // right aligned
        break;
    case ADC_CFG_MODE_10:                                                // conversion mode - single-ended 10 bit or differential 11 bit
        usStandardValue >>= 6;                                           // right aligned
        break;
    case ADC_CFG_MODE_8:                                                 // conversion mode - single-ended 8 bit or differential 9 bit
        usStandardValue >>= 8;                                           // right aligned
        break;
    }
    return usStandardValue;
}

static void fnSimADC(int iChannel)
{
    iMX_ADC_REGS *ptrADC;
    int iValue = 0;
    unsigned short usADCvalue;
    if (iChannel == 0) {
        ptrADC = (iMX_ADC_REGS *)ADC1_BLOCK;
    }
    #if ADC_CONTROLLERS > 1
    else if (iChannel == 1) {
        ptrADC = (iMX_ADC_REGS *)ADC2_BLOCK;
    }
    #endif
    else {
        return;
    }
#if 0
    iValue += (ptrADC->ADC_SC1A & ADC_SC1A_ADCH_OFF);                    // the input being converted
    usADCvalue = fnConvertSimADCvalue(ptrADC, usADC_values[iChannel][iValue]); // convert the standard value to the format used by the present mode
    if ((ptrADC->ADC_SC2 & ADC_SC2_ACFE) != 0) {                         // if the compare function is enabled
    #if !defined KINETIS_KE
        if ((ptrADC->ADC_SC2 & ADC_SC2_ACREN) != 0) {                    // range enabled (uses CV1 and CV2)
            if ((ptrADC->ADC_SC2 & ADC_SC2_ACFGT) != 0) {                // greater or equal
                if (ptrADC->ADC_CV1 <= ptrADC->ADC_CV2) {                // inside range inclusive
                    if ((usADCvalue < ptrADC->ADC_CV1) || (usADCvalue > ptrADC->ADC_CV2)) {
                        return;
                    }
                }
                else {                                                   // outside range inclusive
                    if ((usADCvalue < ptrADC->ADC_CV1) && (usADCvalue > ptrADC->ADC_CV2)) {
                        return;
                    }
                }
            }
            else {
                if (ptrADC->ADC_CV1 <= ptrADC->ADC_CV2) {                // outside range not inclusive
                    if ((usADCvalue >= ptrADC->ADC_CV1) && (usADCvalue <= ptrADC->ADC_CV2)) {
                        return;
                    }
                }
                else {                                                   // inside range not inclusive
                    if ((usADCvalue >= ptrADC->ADC_CV1) || (usADCvalue <= ptrADC->ADC_CV2)) {
                        return;
                    }
                }
            }
        }
        else {                                                           // uses only CV1
    #endif
            if (ptrADC->ADC_SC2 & ADC_SC2_ACFGT) {                       // greater or equal
                if (usADCvalue < ptrADC->ADC_CV1) {                      // if the range is not true the result is not copied to the data register and the conversion is not complete
                    return;
                }
            }
            else {                                                       // less
                if (usADCvalue >= ptrADC->ADC_CV1) {                     // if the range is not true the result is not copied to the data register and the conversion is not complete
                    return;
                }
            }
        }
    #if !defined KINETIS_KE
    }
    #endif
    ptrADC->ADC_RA = usADCvalue;                                         // set the latest voltage to the result register
    ptrADC->ADC_SC1A |= ADC_SC1A_COCO;                                   // set end of conversion flag
#endif
}

extern int fnGetADC_sim_channel(int iPort, int iBit);
static int fnHandleADCchange(int iChange, int iPort, unsigned char ucPortBit)
{
    if ((iChange & (TOGGLE_INPUT | TOGGLE_INPUT_NEG | TOGGLE_INPUT_POS | SET_INPUT)) != 0) {
        int iADC = 0;
        unsigned short usStepSize;
        signed int iAdcChannel = fnGetADC_sim_channel(iPort, (/*31 - */ucPortBit));
        if (iAdcChannel < 0) {                                           // ignore if not valid ADC port
            return -1;                                                   // not analoge input so ignore
        }
        while (iAdcChannel >= 32) {
            iADC++;
            iAdcChannel -= 32;
        }
        if ((TOGGLE_INPUT_ANALOG & iChange) != 0) {
            usStepSize = (0xffff/3);
        }
        else {
            usStepSize = ((ADC_SIM_STEP_SIZE * 0xffff) / ADC_REFERENCE_VOLTAGE);
        }
        if ((TOGGLE_INPUT_NEG & iChange) != 0) {                         // force a smaller voltage
            if (usADC_values[iADC][iAdcChannel] >= usStepSize) {
                usADC_values[iADC][iAdcChannel] -= usStepSize;           // decrease the voltage on the pin
            }
        }
        else {
            if ((usADC_values[iADC][iAdcChannel] + usStepSize) <= 0xffff) {
                usADC_values[iADC][iAdcChannel] += usStepSize;           // increase the voltage on the pin
            }
        }
    }
    return 0;
}
#endif

#if defined SUPPORT_TIMER                                                // general purpose timers (GPT)
static int fnGPTimerPowered(int iTimer)
{
    switch (iTimer) {
    case 0:
        return (IS_POWERED_UP(1, GPT1_BUS_CLOCK));
    case 1:
        return (IS_POWERED_UP(0, GPT2_BUS_CLOCK));
    default:
        return 0;
    }
}

static GPTIMER_MODULE *fnGetGPTimerReg(int iChannel)
{
    switch (iChannel) {
    case 0:
        return (GPTIMER_MODULE *)GPT1_BLOCK;
    case 1:
        return (GPTIMER_MODULE *)GPT2_BLOCK;
    default:
        return 0;
    }
}

unsigned long fnGetGPTimer_clock(int iChannel)
{
    unsigned long ulClockSpeed = 0;
    GPTIMER_MODULE *ptrTimer = fnGetGPTimerReg(iChannel);
    switch (ptrTimer->GPT_CR & GPT_CLKSRC_MASK) {                        // switch on the clock source selected for the GPT
    case GPT_CLKSRC_PERCLK:
    case GPT_CLKSRC_HF_REF:
        ulClockSpeed = PERCLK_CLK_ROOT_FREQUENCY;
        break;
    case GPT_CLKSRC_EXT_CLK:                                             // clocked by GPTx_CLK pin
        return 0;
    case GPT_CLKSRC_LF_REF:
        break;
    case GPT_CLKSRC_24M_REF:
        ulClockSpeed = 24000000;
        ulClockSpeed /= ((ptrTimer->GPT_PR >> 12) + 1);                  // apply 24MHz pre-scaler
        return ulClockSpeed;
    }
    ulClockSpeed /= ((ptrTimer->GPT_PR & 0xfff) + 1);                    // apply main pre-scaler
    return ulClockSpeed;
}

static void fnExecuteTimerInterrupt(int iTimerRef)
{
    switch (iTimerRef) {
    case 0:
        if (fnGenInt(irq_GPT1_ID) != 0) {                                // if GPT1 interrupt is not disabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
            ptrVect->processor_interrupts.irq_GPT1();                    // call the interrupt handler
        }
        break;
    case 1:
        if (fnGenInt(irq_GPT2_ID) != 0) {                                // if GPT2 interrupt is not disabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
            ptrVect->processor_interrupts.irq_GPT2();                    // call the interrupt handler
        }
        break;
    }
}

static void fnHandleGPTimer(int iTimerRef, GPTIMER_MODULE *ptrTimer)
{
    static int iChannelFired[GPT_AVAILABLE][3] = {{0}};
    unsigned long ulCountIncrease = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)fnGetGPTimer_clock(iTimerRef))/1000000);
    int iTimerInterrupt = 0;
    int iChannel = 0;
    ulCountIncrease += ptrTimer->GPT_CNT;                                // new counter value
    if (ulCountIncrease >= ptrTimer->GPT_OCR[iTimerRef]) {               // period match
        memset(iChannelFired[iTimerRef], 0, sizeof(iChannelFired[iTimerRef])); // reset individual channel fired flags
        ptrTimer->GPT_SR |= GPT_SR_OF1;                                  // set event flag
        if ((ptrTimer->GPT_CR & GPT_FRR) == 0) {                         // if not in free-running mode
            ulCountIncrease = (ulCountIncrease - ptrTimer->GPT_OCR[iTimerRef]);
        }
    }
    ptrTimer->GPT_CNT = ulCountIncrease;                                 // new counter value
    if (((ptrTimer->GPT_SR & GPT_SR_OF1) != 0) && ((ptrTimer->GPT_IR & (GPT_IR_OF1IE | GPT_IR_OF2IE | GPT_IR_OF3IE)) != 0)) { // if overflow occurred and interrupt enabled
        iTimerInterrupt = 1;
    }
    if (iTimerInterrupt != 0) {
        fnExecuteTimerInterrupt(iTimerRef);
    }
}


static unsigned long ulTimerPinEnabled[PORTS_AVAILABLE] = {0};
static unsigned long ulPinTimerDetails[PORTS_AVAILABLE][PORT_WIDTH] = {0};

extern void fnEnterTimer(int iPortRef, int iPinRef, unsigned long ulReference, int iOnOff)
{
    if (iOnOff != 0) {
        ulTimerPinEnabled[iPortRef] |= (1 << iPinRef);                   // mark that this pin has been programmed for timer operation
        ulPinTimerDetails[iPortRef][iPinRef] = ulReference;
    }
    else {
        ulTimerPinEnabled[iPortRef] &= ~(1 << iPinRef);                  // mark that this pin has not been programmed for timer operation
    }
}

static int fnFlexPWMPowered(int iFlexPWM)
{
    switch (iFlexPWM) {
    case 0:
        return (IS_POWERED_UP(4, PWM1_CLOCKS));
#if FLEXPWM_AVAILABLE > 1
    case 1:
        return (IS_POWERED_UP(4, PWM2_CLOCKS));
#endif
#if FLEXPWM_AVAILABLE > 2
    case 2:
        return (IS_POWERED_UP(4, PWM3_CLOCKS));
#endif
#if FLEXPWM_AVAILABLE > 3
    case 3:
        return (IS_POWERED_UP(4, PWM4_CLOCKS));
#endif
    }
    return 0;
}

FLEXPWM_MODULE *fnGetFlexPWM_Reg(int iFlexPWM)
{
    switch (iFlexPWM) {
    case 0:
        return (FLEXPWM_MODULE *)FLEXPWM1_BLOCK;
#if FLEXPWM_AVAILABLE > 1
    case 1:
        return (FLEXPWM_MODULE *)FLEXPWM2_BLOCK;
#endif
#if FLEXPWM_AVAILABLE > 2
    case 2:
        return (FLEXPWM_MODULE *)FLEXPWM3_BLOCK;
#endif
#if FLEXPWM_AVAILABLE > 3
    case 3:
        return (FLEXPWM_MODULE *)FLEXPWM4_BLOCK;
#endif
    }
    return 0;
}

static unsigned long fnGetFlexPWM_clock(int iPrecaler)
{
    iPrecaler = (1 << iPrecaler);
    return (IPG_CLK_ROOT_FREQUENCY/iPrecaler);
}

extern int fnGetPWM_sim_channel(int iPort, int iPin, unsigned long *ptr_ulFrequency, unsigned char *ptr_ucMSR)
{
    if (((ulTimerPinEnabled[iPort] & (1 << iPin)) != 0) && ((ulPinTimerDetails[iPort][iPin] & SIMULATION_TIMER_MASK) == SIMULATION_SOURCE_FLEX_PWM)) { // if this pin has been programmed as a FlexPWM function
        int iFlexPWM = (((ulPinTimerDetails[iPort][iPin] - SIMULATION_SOURCE_FLEX_PWM_1) & SIMULATION_SOURCE_MASK) >> SIMULATION_SOURCE_SHIFT);
        int iChannel = (ulPinTimerDetails[iPort][iPin] & SIMULATION_CHANNEL_MASK);
        if (fnFlexPWMPowered(iFlexPWM) != 0) {                           // the FlexPWM must be powered
            FLEXPWM_MODULE *ptrFlexPWM = fnGetFlexPWM_Reg(iFlexPWM);
            unsigned long ulMOD_value;
            unsigned long ulMatchValue;
            if (ptrFlexPWM->flexPWM_ch[iChannel].FLEXPWM_SMINIT != 0) {  // if center-aligned
              //ulMOD_value = (ptrTimer->FTM_MOD * 2);
              //ulMatchValue = (ptrTimer->FTM_channel[iChannel].FTM_CV * 2);
            }
            else {
                ulMOD_value = ptrFlexPWM->flexPWM_ch[iChannel].flexPWM_val[1].FLEXPWM_SMVAL;
                ulMatchValue = ptrFlexPWM->flexPWM_ch[iChannel].flexPWM_val[3].FLEXPWM_SMVAL; // valid for PWM A
            }
            if (ulMOD_value == 0) {
                ulMOD_value = 1;
            }
            *ptr_ucMSR = (unsigned char)((float)((float)(ulMatchValue * 100) / (float)ulMOD_value) + (float)0.5); // MSR in percent
            *ptr_ulFrequency = (fnGetFlexPWM_clock((ptrFlexPWM->flexPWM_ch[iChannel].FLEXPWM_SMCTRL & FLEXPWM_SMCTRL_PRSC_128) >> 4)/ulMOD_value);
            if ((ptrFlexPWM->flexPWM_ch[iChannel].FLEXPWM_SMOCTRL & FLEXPWM_SMOCTRL_POLA) != 0) { // polarity inverted
                ulMatchValue = (*ptr_ulFrequency - *ptr_ucMSR);
            }
            return 0;
        }
    }
    return -1;
}


// Handle timer clocks or capture inputs
//
static void fnHandleTimerInput(int iPort, unsigned long ulNewState, unsigned long ulChangedBit, unsigned long *ptrPortConfig)
{
    if ((ulTimerPinEnabled[iPort] & ulChangedBit) != 0) {                // ignore all pins not configured as timers
        int iPinBit = 0;
        while ((ulChangedBit & 1) == 0) {
            ulChangedBit >>= 1;
            iPinBit++;
        }
        if ((ulPinTimerDetails[iPort][iPinBit] & SIMULATION_TIMER_MASK) == SIMULATION_SOURCE_GPT) { // handle input changes only when the pin is configured as a GPT input
            int iGPT = (((ulPinTimerDetails[iPort][iPinBit] - SIMULATION_SOURCE_GPT1) & SIMULATION_SOURCE_MASK) >> SIMULATION_SOURCE_SHIFT);
            int iChannel = (ulPinTimerDetails[iPort][iPinBit] & SIMULATION_CHANNEL_MASK);
            ulChangedBit <<= iPinBit;
            // If the GPT is powered up
            //
            if (fnGPTimerPowered(iGPT) != 0) {
                GPTIMER_MODULE *ptrGPT_Timer = fnGetGPTimerReg(iGPT);
                switch (ulPinTimerDetails[iPort][iPinBit] & SIMULATION_EVENT_MASK) {
                case SIMULATION_SOURCE_CAPTURE:
                    // The input is attached to a timer so we capture its present input value if the sense matches
                    //
                    switch ((ptrGPT_Timer->GPT_CR >> (iGPT * 2)) & (GPT_IM1_BOTH)) {
                    case 0:                                              // not in input capture mode
                        return;
                    case TIMER_CAPTURE_RISING:                           // rising edge capture
                        if ((ulNewState & ulChangedBit) == 0) {
                            return;                                      // ignore a falling edge
                        }
                        break;
                    case GPT_IM1_FALLING:                                // falling edge capture
                        if ((ulNewState & ulChangedBit) != 0) {
                            return;                                      // ignore a rising edge
                        }
                        break;
                    case GPT_IM1_BOTH:
                        break;                                           // accept any edge
                    }
                    // Capture event has take place
                    //
                    ptrGPT_Timer->GPT_ICR[iGPT] = ptrGPT_Timer->GPT_CNT; // capture the present timer count vlue
                    ptrGPT_Timer->GPT_SR |= (GPT_SR_IF1 << iGPT);        // set the channel input capture event flag
                    if ((ptrGPT_Timer->GPT_IR & (GPT_IR_IF1IE << iGPT)) != 0) { // if the channel's interrupt is enabled
                        fnExecuteTimerInterrupt(iGPT);
                    }
                    break;
                case SIMULATION_SOURCE_CLOCK:
                    if ((ulNewState & ulChangedBit) != 0) {              // count on clock rising edges
                        if ((ptrGPT_Timer->GPT_CR & GPT_CLKSRC_MASK) == GPT_CLKSRC_EXT_CLK) {
                            ptrGPT_Timer->GPT_CNT++;
                            fnHandleGPTimer(iGPT, ptrGPT_Timer);
                        }
                    }
                    break;
                }
            }
        }
    }
}
#endif


static int fnIsGPIO_clocked(int iPortRef)
{
    switch (iPortRef) {
    case PORT1:
        if ((CCM_CCGR1 & CCM_CCGR1_GPIO1_CLOCK_MASK) == CCM_CCGR1_GPIO1_CLOCK_OFF){
            return 0;
        }
        break;
    case PORT2:
        if ((CCM_CCGR0 & CCM_CCGR0_GPIO2_CLOCK_MASK) == CCM_CCGR0_GPIO2_CLOCK_OFF) {
            return 0;
        }
        break;
    #if defined PORT3
    case PORT3:
        if ((CCM_CCGR2 & CCM_CCGR2_GPIO3_CLOCK_MASK) == CCM_CCGR2_GPIO3_CLOCK_OFF) {
            return 0;
        }
        break;
    #endif
    #if defined PORT4
    case PORT4:
        if ((CCM_CCGR3 & CCM_CCGR3_GPIO4_CLOCK_MASK) == CCM_CCGR3_GPIO4_CLOCK_OFF) {
            return 0;
        }
        break;
    #endif
    case PORT5:
        break;
    default:
        _EXCEPTION("Bad port reference!");
        break;
    }
    return 1;                                                            // GPIO is clocked
}

#if !defined DEVICE_WITHOUT_DMA
// Handle peripheral DMA triggers
//
static void fnHandleDMA_triggers(int iTriggerSource, int iDMAmux)
{
    #if defined KINETIS_KM
    int iMuxChannels = 1;                                                // KM DMA has one DMAMUX per channel
    #else
    int iMuxChannels = DMA_CHANNEL_COUNT;                                // DMAMUX has the same channel count as there are DMA channels
    #endif
    unsigned long *ptrMux;
    int iChannel = 0;
    switch (iDMAmux) {
    case 0:
        ptrMux = DMAMUX0_CHCFG_ADD;
        break;
    #if defined DMAMUX1_CHCFG_ADD
    case 1:
        ptrMux = DMAMUX1_CHCFG_ADD;
        break;
    #endif
    #if defined DMAMUX2_CHCFG_ADD
    case 2:
        ptrMux = DMAMUX2_CHCFG_ADD;
        break;
    #endif
    #if defined DMAMUX3_CHCFG_ADD
    case 3:
        ptrMux = DMAMUX3_CHCFG_ADD;
        break;
    #endif
    default:
        return;
    }

    #if defined TRGMUX_AVAILABLE
    if ((iTriggerSource & DMAMUX_CHCFG_TRIG) != 0) {                     // periodic trigger (LPIT source)
        iTriggerSource >>= 8;                                            // the LPIT that is being checked for
        if ((TRGMUX_DMAMUX0 & TRGMUX_SEL0) == (TRGMUX_SEL_LPIT0_CHANNEL_0 + iTriggerSource)) { // check that the LPIT trigger is connected to the DMAMUX
            iTriggerSource = DMAMUX0_CHCFG_SOURCE_DMAMUX0;
            iMuxChannels = 4;                                            // period triggers are limited to the first 4 channels
        }
        else {
            return;                                                      // trigger is not connected so it can't trigger a DMA request
        }
    }
    #endif
    iTriggerSource &= ~(DMAMUX_CHCFG_TRIG);
    while (iChannel < iMuxChannels) {
        if ((*ptrMux++ & ~(DMAMUX_CHCFG_TRIG)) == (DMAMUX_CHCFG_ENBL | (unsigned char)iTriggerSource)) { // matching enabled trigger
    #if (!defined KINETIS_KL && !defined KINETIS_KM) || defined DEVICE_WITH_eDMA
            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << iChannel)) != 0) {           // if the DMA channel is enabled
                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                ptrDMA_TCD += iChannel;
                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE);         // trigger
                fnSimulateDMA(iChannel, (unsigned char)iTriggerSource);
            }
    #else
            fnSimulateDMA(iChannel, (unsigned char)iTriggerSource);
    #endif
        }
        iChannel++;
    }
}
#endif

#if defined SUPPORT_XBAR
static void fnXbarTrigger(int iXbarOut, int iState)
{
    unsigned short usControl;
    volatile unsigned short *ptrControl;
    int iDMA_channel;
    if (iXbarOut < 2) {
        ptrControl = XBARA1_CTRL0_ADD;
        iDMA_channel = DMAMUX0_CHCFG_SOURCE_XBAR1_0;
    }
    else {
        ptrControl = XBARA1_CTRL1_ADD;
        iDMA_channel = DMAMUX0_CHCFG_SOURCE_XBAR1_2;
    }
    usControl = *ptrControl;
    if ((iXbarOut & 1) != 0) {
        usControl >>= 8;
        iDMA_channel++;
    }
    switch (usControl & XBARA_CTRL0_EDGE0_MASK) {
    case XBARA_CTRL0_EDGE0_RISE:
        if (iState == 0) {
            return;                                                      // only triggered on a rising edge
        }
        break;
    case XBARA_CTRL0_EDGE0_FALL:
        if (iState != 0) {
            return;                                                      // only triggered on a falling edge
        }
        break;
    case XBARA_CTRL0_EDGE0_BOTH:
        break;                                                           // triggered on both edges
    default:                                                             // no trigger enabled
        return;
    }
    // Trigger detected
    //
    switch (usControl & (XBARA_CTRL0_DEN0 | XBARA_CTRL0_IEN0)) {         // which event is programmed?
    case (XBARA_CTRL0_DEN0 | XBARA_CTRL0_IEN0):
        _EXCEPTION("It is illegal to set both interrupt and DMA mode on the same XBAR trigger!");
        break;
    case (XBARA_CTRL0_IEN0):                                             // generate an interrupt
        *ptrControl |= (XBARA_CTRL0_STS0 << ((iXbarOut & 1) * 8));
#if defined iMX_RT1011
        if (fnGenInt(irq_XBAR1_0_1_2_3_ID) != 0) {                       // if the interrupt is enabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
            ptrVect->processor_interrupts.irq_XBAR1_0_1_2_3();           // call the interrupt handler
        }
#else
        if (iXbarOut < 2) {
            if (fnGenInt(irq_XBAR1_0_1_ID) != 0) {                       // if the interrupt is enabled
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                ptrVect->processor_interrupts.irq_XBAR1_0_1();           // call the interrupt handler
            }
        }
        else {
            if (fnGenInt(irq_XBAR1_2_3_ID) != 0) {                       // if the interrupt is enabled
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                ptrVect->processor_interrupts.irq_XBAR1_2_3();           // call the interrupt handler
            }
        }
#endif
        break;
    case (XBARA_CTRL0_DEN0):                                             // generate a DMA trigger
        *ptrControl |= (XBARA_CTRL0_STS0 << ((iXbarOut & 1) * 8));
        fnHandleDMA_triggers(iDMA_channel, 0);                           // trigger DMA transfer
        break;
    case 0:
        break;
    }
}


static void fnXbarConnected(int iXbarOut, int iState)
{
#if defined iMX_RT1011
    if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6 & (IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_2 << (iXbarOut - 2))) == 0) { // if pin has input function (not output function)
        return;
    }
#else
    if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6 & (IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4 << (iXbarOut - 4))) == 0) { // if pin has input function (not output function)
        return;
    }
#endif
    switch (iXbarOut) {
#if defined iMX_RT1011
    case 2:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_14_XBAR1_INOUT02) { // port is programmed as XBAR02function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT28;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT28;
            }
        }
        break;
    case 3:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_07_XBAR1_INOUT03) { // port is programmed as XBAR03function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT21;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT21;
            }
        }
        break;
#elif defined iMX_RT105X || defined iMX_RT106X
    case 4:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_02_XBAR1_INOUT04) { // port is programmed as XBAR04 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT2;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT2;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_00_XBAR1_INOUT04) { // port is programmed as XBAR04 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT12;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT12;
            }
        }
        break;
    case 5:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_03_XBAR1_INOUT05) { // port is programmed as XBAR05 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT3;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT3;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_01_XBAR1_INOUT05) { // port is programmed as XBAR05 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT13;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT13;
            }
        }
        break;
    case 6:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_04_XBAR1_INOUT06) { // port is programmed as XBAR06 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT4;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT4;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_02_XBAR1_INOUT06) { // port is programmed as XBAR06 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT14;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT14;
            }
        }
        break;
    case 7:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_05_XBAR1_INOUT07) { // port is programmed as XBAR07 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT5;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT5;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_03_XBAR1_INOUT07) { // port is programmed as XBAR06 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT15;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT15;
            }
        }
        break;
    case 8:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_06_XBAR1_INOUT08) { // port is programmed as XBAR08 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT6;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT6;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_04_XBAR1_INOUT08) { // port is programmed as XBAR08 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT16;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT16;
            }
        }
        break;
    case 9:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_07_XBAR1_INOUT09) { // port is programmed as XBAR09 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT7;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT7;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_05_XBAR1_INOUT09) { // port is programmed as XBAR09 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT17;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT17;
            }
        }
        break;
    case 10:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_12_XBAR1_INOUT10) { // port is programmed as XBAR10 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT12;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT12;
            }
        }
        break;
    case 11:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_13 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_13_XBAR1_INOUT11) { // port is programmed as XBAR11 function
            if (iState != 0) {
                ulPort_in_2 |= PORT3_BIT13;
            }
            else {
                ulPort_in_2 &= ~PORT3_BIT13;
            }
        }
        break;
    case 12:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_14_XBAR1_INOUT12) { // port is programmed as XBAR12 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT14;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT14;
            }
        }
        break;
    case 13:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_15_XBAR1_INOUT13) { // port is programmed as XBAR13 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT15;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT15;
            }
        }
        break;
    case 14:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT0;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT0;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT16;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT16;
            }
        }
        break;
    case 15:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT1;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT1;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT17;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT17;
            }
        }
        break;
    case 16:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT2;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT2;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT18;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT18;
            }
        }
        break;
    case 17:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_03_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT3;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT3;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_05_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT5;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT5;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_03_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT19;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT19;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_08_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT8;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT8;
            }
        }
        break;
    case 18:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_35_XBAR1_INOUT18) { // port is programmed as XBAR18 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT21;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT21;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_06_XBAR1_INOUT18) { // port is programmed as XBAR18 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT6;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT6;
            }
        }
        break;
    case 19:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_14_XBAR1_INOUT19) { // port is programmed as XBAR19 function
            if (iState != 0) {
                ulPort_in_4 |= PORT4_BIT14;
            }
            else {
                ulPort_in_4 &= ~PORT4_BIT14;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_07_XBAR1_INOUT19) { // port is programmed as XBAR19 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT7;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT7;
            }
        }
        break;
#else
    case 4:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_04_XBAR1_INOUT04) { // port is programmed as XBAR04 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT4;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT4;
            }
        }
        break;
    case 5:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_05_XBAR1_INOUT05) { // port is programmed as XBAR05 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT5;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT5;
            }
        }
        break;
    case 6:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_06_XBAR1_INOUT06) { // port is programmed as XBAR06 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT6;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT6;
            }
        }
        break;
    case 7:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_07_XBAR1_INOUT07) { // port is programmed as XBAR07 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT7;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT7;
            }
        }
        break;
    case 8:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_08_XBAR1_INOUT08) { // port is programmed as XBAR08 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT8;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT8;
            }
        }
        break;
    case 9:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_09_XBAR1_INOUT09) { // port is programmed as XBAR09 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT9;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT9;
            }
        }
        break;
    case 10:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_10_XBAR1_INOUT10) { // port is programmed as XBAR10 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT10;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT10;
            }
        }
        break;
    #if !defined iMX_RT1015
    case 11:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_11_XBAR1_INOUT11) { // port is programmed as XBAR11 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT11;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT11;
            }
        }
        break;
    case 12:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_08_XBAR1_INOUT12) { // port is programmed as XBAR12 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT24;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT24;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_12_XBAR1_INOUT12) { // port is programmed as XBAR12 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT12;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT12;
            }
        }
        break;
    case 13:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_09_XBAR1_INOUT13) { // port is programmed as XBAR13 function
            if (iState != 0) {
                ulPort_in_1 |= PORT1_BIT25;
            }
            else {
                ulPort_in_1 &= ~PORT1_BIT25;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_13_XBAR1_INOUT13) { // port is programmed as XBAR13 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT13;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT13;
            }
        }
        break;
    case 14:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT13;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT13;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_14_XBAR1_INOUT14) { // port is programmed as XBAR14 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT14;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT14;
            }
        }
        break;
    case 15:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT14;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT14;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_15_XBAR1_INOUT15) { // port is programmed as XBAR15 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT15;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT15;
            }
        }
        break;
    #endif
    case 16:
    #if !defined iMX_RT1015
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT15;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT15;
            }
        }
    #endif
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_18_XBAR1_INOUT16) { // port is programmed as XBAR16 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT18;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT18;
            }
        }
        break;
    case 17:
    #if !defined iMX_RT1015
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_06_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT19;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT19;
            }
        }
    #endif
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_19_XBAR1_INOUT17) { // port is programmed as XBAR17 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT19;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT19;
            }
        }
        break;
    #if !defined iMX_RT1015
    case 18:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_40_XBAR1_INOUT18) { // port is programmed as XBAR18 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT8;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT8;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_28_XBAR1_INOUT18) { // port is programmed as XBAR18 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT28;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT28;
            }
        }
        break;
    case 19:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_41_XBAR1_INOUT19) { // port is programmed as XBAR19 function
            if (iState != 0) {
                ulPort_in_3 |= PORT3_BIT9;
            }
            else {
                ulPort_in_3 &= ~PORT3_BIT9;
            }
        }
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_29_XBAR1_INOUT19) { // port is programmed as XBAR19 function
            if (iState != 0) {
                ulPort_in_2 |= PORT2_BIT29;
            }
            else {
                ulPort_in_2 &= ~PORT2_BIT29;
            }
        }
        break;
    #endif
#endif
    }
}

static void fnCheckXbarTrigger(unsigned char ucInputRef, int iState)
{
    unsigned short *ptrXBAR_Sel = XBARA1_SEL0_ADD;
    int i = 0;
    while (i < 4) {                                                      // search for an XBAR output connected to this input that can generate an interrupt or DMA (first 4 channels)
        if ((*ptrXBAR_Sel & 0x007f) == ucInputRef) {
            fnXbarTrigger(i, iState);
            fnXbarConnected(i, iState);
        }
        i++;
        if ((*ptrXBAR_Sel & 0x7f00) == (ucInputRef << 8)) {
            fnXbarTrigger(i, iState);
            fnXbarConnected(i, iState);
        }
        i++;
        ptrXBAR_Sel++;
    }
    // Check also whether an input is connected directly to an XBAR output
    //
    while (i < (65 * 2)) {                                               // for each possible output
        if ((*ptrXBAR_Sel & 0x007f) == ucInputRef) {
            fnXbarConnected(i, iState);
        }
        i++;
        if ((*ptrXBAR_Sel & 0x7f00) == (ucInputRef << 8)) {
            fnXbarConnected(i, iState);
        }
        i++;
        ptrXBAR_Sel++;
    }
}


static void fnHandleXBAR(unsigned char ucPort, unsigned long ulPortState, unsigned long ulBit, unsigned char ucBitRef)
{
    int iState = ((ulPortState & ulBit) != 0);
    #if defined iMX_RT1011
    if (ucPort == _PORT1) {
        switch (ulBit) {
        case PORT1_BIT21:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_07_XBAR1_INOUT03) { // port is programmed as XBAR03 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_3) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(3, iState);
                }
            }
            break;
        case PORT1_BIT28:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_14_XBAR1_INOUT02) { // port is programmed as XBAR02 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_2) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(2, iState);
                }
            }
            break;
        }
    }
    #elif defined iMX_RT105X || defined iMX_RT106X
    if (ucPort == _PORT1) {
        switch (ulBit) {
        case PORT1_BIT0:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(14, iState);
                }
            }
            break;
        case PORT1_BIT1:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(15, iState);
                }
            }
            break;
        case PORT1_BIT2:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(16, iState);
                }
            }
            break;
        case PORT1_BIT3:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_03_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        case PORT1_BIT5:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_05_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        case PORT1_BIT6:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_06_XBAR1_INOUT18) { // port is programmed as XBAR18 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(18, iState);
                }
            }
            break;
        case PORT1_BIT7:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_07_XBAR1_INOUT19) { // port is programmed as XBAR19 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(19, iState);
                }
            }
            break;
        }
    }
    else if (ucPort == _PORT2) {
        switch (ulBit) {
        case PORT2_BIT12:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_12 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_12_XBAR1_INOUT10) { // port is programmed as XBAR10 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_10) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(10, iState);
                }
            }
            break;
        case PORT2_BIT13:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_13 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_13_XBAR1_INOUT11) { // port is programmed as XBAR11 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_11) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(11, iState);
                }
            }
            break;
        case PORT2_BIT14:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_14_XBAR1_INOUT12) { // port is programmed as XBAR12 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(12, iState);
                }
            }
            break;
        case PORT2_BIT15:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B0_15_XBAR1_INOUT13) { // port is programmed as XBAR13 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(13, iState);
                }
            }
            break;
        case PORT2_BIT16:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(14, iState);
                }
            }
            break;
        case PORT2_BIT17:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(15, iState);
                }
            }
            break;
        case PORT2_BIT18:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(16, iState);
                }
            }
            break;
        case PORT2_BIT19:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_03_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        }
    }
    else if (ucPort == _PORT3) {
        switch (ulBit) {
        case PORT3_BIT12:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_00_XBAR1_INOUT04) { // port is programmed as XBAR04 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(4, iState);
                }
            }
            break;
        case PORT3_BIT13:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_01_XBAR1_INOUT05) { // port is programmed as XBAR05 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(5, iState);
                }
            }
            break;
        case PORT3_BIT14:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_02_XBAR1_INOUT06) { // port is programmed as XBAR06 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(6, iState);
                }
            }
            break;
        case PORT3_BIT15:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_03_XBAR1_INOUT07) { // port is programmed as XBAR07 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(7, iState);
                }
            }
            break;
        case PORT3_BIT16:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_04_XBAR1_INOUT08) { // port is programmed as XBAR08 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(8, iState);
                }
            }
            break;
        case PORT3_BIT17:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_05_XBAR1_INOUT09) { // port is programmed as XBAR09 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(9, iState);
                }
            }
            break;
        case PORT3_BIT21:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_35_XBAR1_INOUT18) { // port is programmed as XBAR18 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(18, iState);
                }
            }
            break;
        }
    }
    else if (ucPort == _PORT4) {
        switch (ulBit) {
        case PORT4_BIT2:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_02_XBAR1_INOUT04) { // port is programmed as XBAR04 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(4, iState);
                }
            }
            break;
        case PORT4_BIT3:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_03_XBAR1_INOUT05) { // port is programmed as XBAR05 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(5, iState);
                }
            }
            break;
        case PORT4_BIT4:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_04_XBAR1_INOUT06) { // port is programmed as XBAR06 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(6, iState);
                }
            }
            break;
        case PORT4_BIT5:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_05_XBAR1_INOUT07) { // port is programmed as XBAR07 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(7, iState);
                }
            }
            break;
        case PORT4_BIT6:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_06_XBAR1_INOUT08) { // port is programmed as XBAR08 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(8, iState);
                }
            }
            break;
        case PORT4_BIT7:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_07_XBAR1_INOUT09) { // port is programmed as XBAR09 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(9, iState);
                }
            }
            break;
        case PORT4_BIT8:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_08_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        case PORT4_BIT14:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_14_XBAR1_INOUT19) { // port is programmed as XBAR19 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(19, iState);
                }
            }
            break;
        }
    }
    #else
    if (ucPort == _PORT2) {
        switch (ulBit) {
        case PORT2_BIT4:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_04_XBAR1_INOUT04) { // port is programmed as XBAR04 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_4) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(4, iState);
                }
            }
            break;
        case PORT2_BIT5:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_05_XBAR1_INOUT05) { // port is programmed as XBAR05 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_5) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(5, iState);
                }
            }
            break;
        case PORT2_BIT6:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_06_XBAR1_INOUT06) { // port is programmed as XBAR06 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(6, iState);
                }
            }
            break;
        case PORT2_BIT7:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_07_XBAR1_INOUT07) { // port is programmed as XBAR07 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_7) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(7, iState);
                }
            }
            break;
        case PORT2_BIT8:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_08_XBAR1_INOUT08) { // port is programmed as XBAR08 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_8) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(8, iState);
                }
            }
            break;
        case PORT2_BIT9:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_09_XBAR1_INOUT09) { // port is programmed as XBAR09 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_9) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(9, iState);
                }
            }
            break;
        case PORT2_BIT10:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_10_XBAR1_INOUT10) { // port is programmed as XBAR10 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_10) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(10, iState);
                }
            }
            break;
        #if !defined iMX_RT1015
        case PORT2_BIT11:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_11_XBAR1_INOUT11) { // port is programmed as XBAR11 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_11) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(11, iState);
                }
            }
            break;
        case PORT2_BIT12:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_12_XBAR1_INOUT12) { // port is programmed as XBAR12 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(12, iState);
                }
            }
            break;
        case PORT2_BIT13:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_13_XBAR1_INOUT13) { // port is programmed as XBAR13 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(13, iState);
                }
            }
            break;
        case PORT2_BIT14:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_14_XBAR1_INOUT14) { // port is programmed as XBAR14 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(14, iState);
                }
            }
            break;
        case PORT2_BIT15:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_15_XBAR1_INOUT15) { // port is programmed as XBAR15 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(15, iState);
                }
            }
            break;
        #endif
        case PORT2_BIT18:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_18_XBAR1_INOUT16) { // port is programmed as XBAR16 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(16, iState);
                }
            }
            break;
        case PORT2_BIT19:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_19_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        #if !defined iMX_RT1015
        case PORT2_BIT28:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_28_XBAR1_INOUT18) { // port is programmed as XBAR18 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(18, iState);
                }
            }
            break;
        case PORT2_BIT29:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_29_XBAR1_INOUT19) { // port is programmed as XBAR19 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(19, iState);
                }
            }
            break;
        #endif
        }
    }
        #if !defined iMX_RT1015
    else if (ucPort == _PORT1) {
        switch (ulBit) {
        case PORT1_BIT24:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_08_XBAR1_INOUT12) { // port is programmed as XBAR12 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_12) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(12, iState);
                }
            }
            break;
        case PORT1_BIT25:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_09_XBAR1_INOUT13) { // port is programmed as XBAR13 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_13) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(13, iState);
                }
            }
            break;
        }
    }
    else if (ucPort == _PORT3) {
        switch (ulBit) {
        case PORT3_BIT13:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_00_XBAR1_INOUT14) { // port is programmed as XBAR14 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(14, iState);
                }
            }
            break;
        case PORT3_BIT14:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_01_XBAR1_INOUT15) { // port is programmed as XBAR15 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_15) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(15, iState);
                }
            }
            break;
        case PORT3_BIT15:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_02_XBAR1_INOUT16) { // port is programmed as XBAR16 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_16) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(16, iState);
                }
            }
            break;
        case PORT3_BIT19:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_06_XBAR1_INOUT17) { // port is programmed as XBAR17 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_17) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(17, iState);
                }
            }
            break;
        case PORT3_BIT8:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_40_XBAR1_INOUT18) { // port is programmed as XBAR18 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_18) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(18, iState);
                }
            }
            break;
        case PORT3_BIT9:
            if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_41_XBAR1_INOUT19) { // port is programmed as XBAR19 function
                if ((IOMUXC_GPR_GPR6 & IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_19) == 0) { // if pin has input function (not output function)
                    fnCheckXbarTrigger(19, iState);
                }
            }
            break;
        }
    }
        #endif
    #endif
}
#endif

// Simulate setting, clearing or toggling of an input pin
//
extern void fnSimulateInputChange(unsigned char ucPort, unsigned char ucPortBit, int iChange)
{
    unsigned long ulBit = (0x80000000 >> ucPortBit);
    iFlagRefresh = PORT_CHANGE;
#if defined SUPPORT_ADC_
    if (fnHandleADCchange(iChange, ucPort, ucPortBit) == 0) {            // handle possible ADC function on pin
        return;                                                          // if ADC we do not handle digital functions
    }
#endif
    switch (ucPort) {
    case _PORT1:
        if ((~GPIO1_GDIR & ulBit) != 0) {                                // if configured as input
            unsigned long ulOriginal_port_state = ulPort_in_1;
            if (fnIsGPIO_clocked(PORT1) == 0) {                          // ignore if port is not clocked
                return;
            }
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_1 ^= ulBit;                                    // set new pin state
                GPIO1_PSR &= ~ulBit;
                GPIO1_PSR |= (ulPort_in_1 & ulBit);                      // set new input register state
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_1 |= ulBit;
                GPIO1_PSR |= ulBit;                                      // set new input register state
            }
            else {
                ulPort_in_1 &= ~ulBit;
                GPIO1_PSR &= ~ulBit;                                     // set new input register state
            }
            if (ulPort_in_1 != ulOriginal_port_state) {                  // if a change took place
                fnPortInterrupt(_PORT1, ulPort_in_1, ulBit);             // handle interrupts and DMA on the pin
 #if defined SUPPORT_XBAR
                fnHandleXBAR(_PORT1, ulPort_in_1, ulBit, (31 - ucPortBit));
#endif
#if defined SUPPORT_TIMER
                fnHandleTimerInput(_PORT1, ulPort_in_1, ulBit, 0);
#endif
            }
        }
        break;
#if PORTS_AVAILABLE > 1
    case _PORT2:
        if ((~GPIO2_GDIR & ulBit) != 0) {                                // if configured as input
            unsigned long ulOriginal_port_state = ulPort_in_2;
            if (fnIsGPIO_clocked(PORT2) == 0) {                          // ignore if port is not clocked
                return;
            }
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_2 ^= ulBit;                                    // set new pin state
                GPIO2_PSR &= ~ulBit;
                GPIO2_PSR |= (ulPort_in_2 & ulBit);                      // set new input register state
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_2 |= ulBit;
                GPIO2_PSR |= ulBit;                                      // set new input register state
            }
            else {
                ulPort_in_2 &= ~ulBit;
                GPIO2_PSR &= ~ulBit;                                     // set new input register state
            }
            if (ulPort_in_2 != ulOriginal_port_state) {                  // if a change took place
                fnPortInterrupt(_PORT2, ulPort_in_2, ulBit);             // handle interrupts and DMA on the pin
    #if defined SUPPORT_XBAR
                fnHandleXBAR(_PORT2, ulPort_in_2, ulBit, (31 - ucPortBit));
    #endif
    #if defined SUPPORT_TIMER
                fnHandleTimerInput(_PORT2, ulPort_in_2, ulBit, 0);
    #endif
            }
        }
        break;
#endif
#if defined PORT3 && (PORTS_AVAILABLE > 2)
    case _PORT3:
        if (~GPIO3_GDIR & ulBit) {                                       // if configured as input
            unsigned long ulOriginal_port_state = ulPort_in_3;
            if (fnIsGPIO_clocked(PORT3) == 0) {                          // ignore if port is not clocked
                return;
            }
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_3 ^= ulBit;                                    // set new pin state
                GPIO3_PSR &= ~ulBit;
                GPIO3_PSR |= (ulPort_in_3 & ulBit);                      // set new input register state
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_3 |= ulBit;
                GPIO3_PSR |= ulBit;                                      // set new input register state
            }
            else {
                ulPort_in_3 &= ~ulBit;
                GPIO3_PSR &= ~ulBit;                                     // set new input register state
            }
            if (ulPort_in_3 != ulOriginal_port_state) {                  // if a change took place
                fnPortInterrupt(_PORT3, ulPort_in_3, ulBit);             // handle interrupts and DMA on the pin
    #if defined SUPPORT_XBAR
                fnHandleXBAR(_PORT3, ulPort_in_3, ulBit, (31 - ucPortBit));
    #endif
    #if defined SUPPORT_TIMER
                fnHandleTimerInput(_PORT3, ulPort_in_3, ulBit, 0);
    #endif
            }
        }
        break;
#endif
#if (defined iMX_RT105X || defined iMX_RT106X) && (PORTS_AVAILABLE > 3)
    case _PORT4:
        if (~GPIO4_GDIR & ulBit) {                                       // if configured as input
            unsigned long ulOriginal_port_state = ulPort_in_4;
            if (fnIsGPIO_clocked(PORT4) == 0) {                          // ignore if port is not clocked
                return;
            }
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_4 ^= ulBit;                                    // set new pin state
                GPIO4_PSR &= ~ulBit;
                GPIO4_PSR |= (ulPort_in_4 & ulBit);                      // set new input register state
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_4 |= ulBit;
                GPIO4_PSR |= ulBit;                                      // set new input register state
            }
            else {
                ulPort_in_4 &= ~ulBit;
                GPIO4_PSR &= ~ulBit;                                     // set new input register state
            }
            if (ulPort_in_4 != ulOriginal_port_state) {                  // if a change took place
                fnPortInterrupt(_PORT4, ulPort_in_4, ulBit);             // handle interrupts and DMA on the pin
    #if defined SUPPORT_XBAR
                fnHandleXBAR(_PORT4, ulPort_in_4, ulBit, (31 - ucPortBit));
    #endif
    #if defined SUPPORT_TIMER
                fnHandleTimerInput(_PORT4, ulPort_in_4, ulBit, 0);
    #endif
            }
        }
        break;
#endif
#if PORTS_AVAILABLE > 4
    case _PORT5:
        if ((~GPIO5_GDIR & ulBit) != 0) {                                // if configured as input
            unsigned long ulOriginal_port_state = ulPort_in_5;
            if (iChange == TOGGLE_INPUT) {
                ulPort_in_5 ^= ulBit;                                    // set new pin state
                GPIO5_PSR &= ~ulBit;
                GPIO5_PSR |= (ulPort_in_5 & ulBit);                      // set new input register state
            }
            else if (iChange == SET_INPUT) {
                ulPort_in_5 |= ulBit;
                GPIO5_PSR |= ulBit;                                      // set new input register state
            }
            else {
                ulPort_in_5 &= ~ulBit;
                GPIO5_PSR &= ~ulBit;                                     // set new input register state
            }
            if (ulPort_in_5 != ulOriginal_port_state) {                  // if a change took place
                fnPortInterrupt(_PORT5, ulPort_in_5, ulBit);             // handle interrupts and DMA on the pin
    #if defined SUPPORT_TIMER
                fnHandleTimerInput(_PORT5, ulPort_in_5, ulBit, 0);
    #endif
            }
        }
        break;
#endif
    case _PORT_EXT_0:                                                    // external ports extensions
    case _PORT_EXT_1:
    case _PORT_EXT_2:
    case _PORT_EXT_3:
    case _PORT_EXT_4:
    case _PORT_EXT_5:
    case _PORT_EXT_6:
    case _PORT_EXT_7:
#if defined HANDLE_EXT_PORT
        HANDLE_EXT_PORT(ucPort, iChange, ulBit);
#endif
        break;
    }
}



// Update ports based on present register settings
//
extern void fnSimPorts(int iThisPort)
{
    unsigned long ulNewState;
    if ((CCM_CCGR1 & CCM_CCGR1_GPIO1_CLOCK_MASK) != CCM_CCGR1_GPIO1_CLOCK_OFF) { // if this port is clocked
        ulNewState = ((GPIO1_DR & GPIO1_GDIR) | GPIO1_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO1_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO1_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO1_DR = ulNewState;
        GPIO1_PSR = ((ulPort_in_1 & ~GPIO1_GDIR) | (GPIO1_DR & GPIO1_GDIR)); // input state
        GPIO1_PSR &= ~(ulPeripherals[PORT1]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO1_DR_SET = GPIO1_DR_CLEAR = GPIO1_DR_TOGGLE = 0;                 // registers always read 0
    if ((CCM_CCGR0 & CCM_CCGR0_GPIO2_CLOCK_MASK) != CCM_CCGR0_GPIO2_CLOCK_OFF) { // if this port is clocked
        ulNewState = ((GPIO2_DR & GPIO2_GDIR) | GPIO2_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO2_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO2_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO2_DR = ulNewState;
        GPIO2_PSR = ((ulPort_in_2 & ~GPIO2_GDIR) | (GPIO2_DR & GPIO2_GDIR)); // input state
        GPIO2_PSR &= ~(ulPeripherals[PORT2]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO2_DR_SET = GPIO2_DR_CLEAR = GPIO2_DR_TOGGLE = 0;                 // registers always read 0
#if defined PORT3
    if ((CCM_CCGR2 & CCM_CCGR2_GPIO3_CLOCK_MASK) != CCM_CCGR2_GPIO3_CLOCK_OFF) { // if this port is clocked
        ulNewState = ((GPIO3_DR & GPIO3_GDIR) | GPIO3_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO3_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO3_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO3_DR = ulNewState;
        GPIO3_PSR = ((ulPort_in_3 & ~GPIO3_GDIR) | (GPIO3_DR & GPIO3_GDIR)); // input state
        GPIO3_PSR &= ~(ulPeripherals[PORT3]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO3_DR_SET = GPIO3_DR_CLEAR = GPIO3_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT4
    if ((CCM_CCGR2 & CCM_CCGR3_GPIO4_CLOCK_MASK) != CCM_CCGR3_GPIO4_CLOCK_OFF) { // if this port is clocked
        ulNewState = ((GPIO4_DR & GPIO4_GDIR) | GPIO4_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO4_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO4_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO4_DR = ulNewState;
        GPIO4_PSR = ((ulPort_in_4 & ~GPIO4_GDIR) | (GPIO4_DR & GPIO4_GDIR)); // input state
        GPIO4_PSR &= ~(ulPeripherals[PORT4]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO4_DR_SET = GPIO4_DR_CLEAR = GPIO4_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT5
    {                                                                    // port 5 doesn't have a clock gate
        ulNewState = ((GPIO5_DR & GPIO5_GDIR) | GPIO5_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO5_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO5_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO5_DR = ulNewState;
        GPIO5_PSR = ((ulPort_in_5 & ~GPIO5_GDIR) | (GPIO5_DR & GPIO5_GDIR)); // input state
        GPIO5_PSR &= ~(ulPeripherals[PORT5]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO5_DR_SET = GPIO5_DR_CLEAR = GPIO5_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT6
    {                                                                    // port 6 doesn't have a clock gate
        ulNewState = ((GPIO6_DR & GPIO6_GDIR) | GPIO6_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO6_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO6_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO6_DR = ulNewState;
        GPIO6_PSR = ((ulPort_in_6 & ~GPIO6_GDIR) | (GPIO6_DR & GPIO6_GDIR)); // input state
        GPIO6_PSR &= ~(ulPeripherals[PORT6]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO6_DR_SET = GPIO6_DR_CLEAR = GPIO6_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT7
    {                                                                    // port 7 doesn't have a clock gate
        ulNewState = ((GPIO7_DR & GPIO7_GDIR) | GPIO7_DR_SET);          // set bits from set register
        ulNewState &= ~(GPIO7_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO7_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO7_DR = ulNewState;
        GPIO7_PSR = ((ulPort_in_7 & ~GPIO7_GDIR) | (GPIO7_DR & GPIO7_GDIR)); // input state
        GPIO7_PSR &= ~(ulPeripherals[PORT7]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO7_DR_SET = GPIO7_DR_CLEAR = GPIO7_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT8
    {                                                                    // port 8 doesn't have a clock gate
        ulNewState = ((GPIO8_DR & GPIO8_GDIR) | GPIO8_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO8_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO8_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO8_DR = ulNewState;
        GPIO8_PSR = ((ulPort_in_8 & ~GPIO8_GDIR) | (GPIO8_DR & GPIO8_GDIR)); // input state
        GPIO8_PSR &= ~(ulPeripherals[PORT8]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO8_DR_SET = GPIO8_DR_CLEAR = GPIO8_DR_TOGGLE = 0;                 // registers always read 0
#endif
#if defined PORT9
    {                                                                    // port 9 doesn't have a clock gate
        ulNewState = ((GPIO9_DR & GPIO9_GDIR) | GPIO9_DR_SET);           // set bits from set register
        ulNewState &= ~(GPIO9_DR_CLEAR);                                 // clear bits from clear register
        ulNewState ^= GPIO9_DR_TOGGLE;                                   // toggle bits from toggle register
        GPIO9_DR = ulNewState;
        GPIO9_PSR = ((ulPort_in_9 & ~GPIO9_GDIR) | (GPIO9_DR & GPIO9_GDIR)); // input state
        GPIO9_PSR &= ~(ulPeripherals[PORT9]);                            // bits read 0 when the pin is not connected to GPIO
    }
    GPIO9_DR_SET = GPIO9_DR_CLEAR = GPIO9_DR_TOGGLE = 0;                 // registers always read 0
#endif
}


// Update peripherals based on present port register settings - this is only called when a peripheral setting has been changed
//
extern void fnSimPers(void)
{
    int iPort = 0;
    int iPin = 0;
    unsigned long ulBit;
    for (iPort = 0; iPort < PORTS_AVAILABLE; iPort++) {
        ulPeripherals[iPort] = 0;
        ulBit = 0x00000001;
        for (iPin = 0; iPin < 32; iPin++) {                              // for each bit in a port
            switch (iPort) {
            case _PORT1:
#if defined PORT6
            case _PORT6:
#endif
                {
#if defined iMX_RT1011
                    if (iPin <= 28) {
                        unsigned long *ptrGPIO_Pad;
                        if (iPin <= 13) {
                            ptrGPIO_Pad = IOMUXC_SW_MUX_CTL_PAD_GPIO_00_ADD;
                            ptrGPIO_Pad -= iPin;
                        }
                        else {
                            ptrGPIO_Pad = IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_00_ADD;
                            ptrGPIO_Pad -= (iPin - 14);
                        }
                        ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_Pad & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                        if ((unsigned char)(*ptrGPIO_Pad & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) { // if a peripheral function is selected
                            ulPeripherals[iPort] |= ulBit;
                        }
                    }
#else
                    unsigned long *ptrGPIO_AD_B0 = IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00_ADD;
                    ptrGPIO_AD_B0 += iPin;
                    ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_AD_B0 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                    if ((unsigned char)(*ptrGPIO_AD_B0 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) { // if a peripheral function is selected
                        ulPeripherals[iPort] |= ulBit;
                    }
#endif
                }
                break;
            case _PORT2:
#if defined PORT7
            case _PORT7:
#endif
                {
#if defined iMX_RT1011
                    if (iPin <= 13) {
                        unsigned long *ptrGPIO_Pad = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_00_ADD;
                        ptrGPIO_Pad -= iPin;
                        ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_Pad & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                        if ((unsigned char)(*ptrGPIO_Pad & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) { // if a peripheral function is selected
                            ulPeripherals[iPort] |= ulBit;
                        }
                    }
#else
    #if defined iMX_RT105X || defined iMX_RT106X
                    unsigned long *ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00_ADD;
    #else
                    unsigned long *ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00_ADD;
    #endif
                    ptrGPIO_EMC += iPin;
                    ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                    if ((unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) {
                        ulPeripherals[iPort] |= ulBit;
                    }
#endif
                }
                break;
#if defined PORT3
            case _PORT3:
    #if defined PORT8
            case _PORT8:
    #endif
                {
    #if defined iMX_RT105X || defined iMX_RT106X
                    unsigned long *ptrGPIO_EMC;
                    if (iPin >= 18) {
                        ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32_ADD;
                        ptrGPIO_EMC += (iPin - 18);
                    }
                    else if (iPin >= 12) {
                        ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00_ADD;
                        ptrGPIO_EMC += (iPin - 12);
                    }
                    else {
                        ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B1_00_ADD;
                        ptrGPIO_EMC += iPin;
                    }
    #else
                    unsigned long *ptrGPIO_EMC;
                    if (iPin >= 13) {
                        ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00_ADD;
                        ptrGPIO_EMC += (iPin - 13);
                    }
                    else {
                        ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32_ADD;
                        ptrGPIO_EMC += iPin;
                    }
    #endif
                    ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                    if ((unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) {
                        ulPeripherals[iPort] |= ulBit;
                    }
                }
                break;
#endif
#if defined PORT4
            case _PORT4:
    #if defined PORT9
            case _PORT9:
    #endif
                {
                    unsigned long *ptrGPIO_EMC;
                    ptrGPIO_EMC = IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00_ADD;
                    ptrGPIO_EMC += iPin;
                    ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                    if ((unsigned char)(*ptrGPIO_EMC & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) {
                        ulPeripherals[iPort] |= ulBit;
                    }
                }
                break;
#endif
            case _PORT5:
                {
#if defined iMX_RT1011
                    if (iPort == 0) {
                        ucPortFunctions[iPort][iPin] = (unsigned char)(IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                        if ((unsigned char)(IOMUXC_SNVS_SW_MUX_CTL_PAD_PMIC_ON_REQ & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) {
                            ulPeripherals[iPort] |= ulBit;
                        }
                    }
#else
                    unsigned long *ptrGPIO_Special = IOMUXC_SNVS_SW_MUX_CTL_PAD_WAKEUP_ADD;
                    ptrGPIO_Special += iPin;
                    ucPortFunctions[iPort][iPin] = (unsigned char)(*ptrGPIO_Special & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK);
                    if ((unsigned char)(*ptrGPIO_Special & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) != IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_GPIO) {
                        ulPeripherals[iPort] |= ulBit;
                    }
#endif
                }
                break;
            }
            ulBit <<= 1;
        }
    }
    iFlagRefresh = PORT_CHANGE;
}

extern int fnSimulateDMA(int channel, unsigned char ucTriggerSource)
{
#if !defined DEVICE_WITHOUT_DMA
    KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
    ptrDMA_TCD += channel;

    if (channel >= DMA_CHANNEL_COUNT) {
        _EXCEPTION("Warning - invalid DMA channel being used!!");
    }
    if (ucTriggerSource != 0) {
        ptrDMA_TCD->DMA_TCD_CSR |= DMA_TCD_CSR_ACTIVE;
    }
    if ((ptrDMA_TCD->DMA_TCD_CSR & DMA_TCD_CSR_ACTIVE) != 0) {           // peripheral trigger
        if (ucTriggerSource != 0) {
            unsigned long *ptrDMUX = (DMAMUX0_CHCFG_ADD + channel);
            // Check that the trigger source is correctly connected to the DMA channel
            //
            if ((*ptrDMUX & ~(DMAMUX_CHCFG_TRIG | DMAMUX_CHCFG_ENBL)) != ucTriggerSource) {
                _EXCEPTION("DMUX source is not connected!!!");
            }
        }
    }
    if ((ptrDMA_TCD->DMA_TCD_CSR & (DMA_TCD_CSR_START | DMA_TCD_CSR_ACTIVE)) != 0) { // sw commanded start or active
        int interrupt = 0;
        ptrDMA_TCD->DMA_TCD_CSR |= DMA_TCD_CSR_ACTIVE;                   // mark active 
        ptrDMA_TCD->DMA_TCD_CSR &= ~DMA_TCD_CSR_START;                   // clear the start bit
        if (ptrDMA_TCD->DMA_TCD_CITER_ELINK != 0) {                      // main loop iterations
            unsigned long ulMinorLoop = ptrDMA_TCD->DMA_TCD_NBYTES_ML;   // the number of bytes to transfer
            if ((ptrDMA_TCD->DMA_TCD_ATTR & DMA_TCD_ATTR_DSIZE_32) != 0) { // handle long word transfers
                if ((ptrDMA_TCD->DMA_TCD_DOFF & 0x3) != 0) {
                    _EXCEPTION("DMA destination offset error!!");
                }
                if ((ptrDMA_TCD->DMA_TCD_SOFF & 0x3) != 0) {
                    _EXCEPTION("DMA source offset error!!");
                }
                if ((ulMinorLoop & 0x3) != 0) {
                    _EXCEPTION("DMA copy size error!!");
                }
                while (ulMinorLoop != 0) {
                    *(unsigned long *)ptrDMA_TCD->DMA_TCD_DADDR = *(unsigned long *)ptrDMA_TCD->DMA_TCD_SADDR; // long word transfer
                    ptrDMA_TCD->DMA_TCD_DADDR = ptrDMA_TCD->DMA_TCD_DADDR + ptrDMA_TCD->DMA_TCD_DOFF;
                    ptrDMA_TCD->DMA_TCD_SADDR = ptrDMA_TCD->DMA_TCD_SADDR + ptrDMA_TCD->DMA_TCD_SOFF;
                    ulMinorLoop -= sizeof(unsigned long);
                }
            }
            else if ((ptrDMA_TCD->DMA_TCD_ATTR & DMA_TCD_ATTR_DSIZE_16) != 0) { // handle short word transfers
                if ((ptrDMA_TCD->DMA_TCD_DOFF & 0x1) != 0) {
                    _EXCEPTION("DMA destination offset error!!");
                }
                if ((ptrDMA_TCD->DMA_TCD_SOFF & 0x1) != 0) {
                    _EXCEPTION("DMA source offset error!!");
                }
                if ((ulMinorLoop & 0x1) != 0) {
                    _EXCEPTION("DMA copy size error!!");
                }
                while (ulMinorLoop != 0) {
                    *(unsigned short *)ptrDMA_TCD->DMA_TCD_DADDR = *(unsigned short *)ptrDMA_TCD->DMA_TCD_SADDR; // short word transfer
                    ptrDMA_TCD->DMA_TCD_DADDR = (ptrDMA_TCD->DMA_TCD_DADDR + ptrDMA_TCD->DMA_TCD_DOFF);
                    ptrDMA_TCD->DMA_TCD_SADDR = (ptrDMA_TCD->DMA_TCD_SADDR + ptrDMA_TCD->DMA_TCD_SOFF);
                    if (ulMinorLoop <= sizeof(unsigned short)) {
                        ulMinorLoop = 0;
                    }
                    else {
                        ulMinorLoop -= sizeof(unsigned short);
                    }
                }
            }
            else {
                while (ulMinorLoop-- != 0) {                             // minor loop count
                    *(unsigned char *)ptrDMA_TCD->DMA_TCD_DADDR = *(unsigned char *)ptrDMA_TCD->DMA_TCD_SADDR; // byte transfer
                    ptrDMA_TCD->DMA_TCD_DADDR = ptrDMA_TCD->DMA_TCD_DADDR + ptrDMA_TCD->DMA_TCD_DOFF;
                    ptrDMA_TCD->DMA_TCD_SADDR = ptrDMA_TCD->DMA_TCD_SADDR + ptrDMA_TCD->DMA_TCD_SOFF;
                }
            }
            (ptrDMA_TCD->DMA_TCD_CITER_ELINK)--;
            if (ptrDMA_TCD->DMA_TCD_CITER_ELINK == 0) {                  // major loop completed
                if ((ptrDMA_TCD->DMA_TCD_CSR & DMA_TCD_CSR_INTMAJOR) != 0) { // {18}
                    interrupt = 1;                                       // possible interrupt
                }
                if ((ptrDMA_TCD->DMA_TCD_CSR & DMA_TCD_CSR_DREQ) != 0) { // disable on completion of major loop
                    DMA_ERQ &= ~(DMA_ERQ_ERQ0 << channel);
                    ptrDMA_TCD->DMA_TCD_CSR &= ~DMA_TCD_CSR_ACTIVE;      // completed
                }
                else {                                                   // continuous operation
                    ptrDMA_TCD->DMA_TCD_CITER_ELINK = ptrDMA_TCD->DMA_TCD_BITER_ELINK; // set back the main loop count value
                }
                ptrDMA_TCD->DMA_TCD_CSR |= DMA_TCD_CSR_DONE;
                ptrDMA_TCD->DMA_TCD_DADDR += ptrDMA_TCD->DMA_TCD_DLASTSGA;
                ptrDMA_TCD->DMA_TCD_SADDR += ptrDMA_TCD->DMA_TCD_SLAST;
            }
            else if (ptrDMA_TCD->DMA_TCD_CITER_ELINK == (ptrDMA_TCD->DMA_TCD_BITER_ELINK/2)) { // half complete
                if ((ptrDMA_TCD->DMA_TCD_CSR & DMA_TCD_CSR_INTHALF) != 0) { // check whether half-buffer interrupt has been configured
                    interrupt = 1;
                }
            }

            if (interrupt != 0) {                                        // if possible interrupt to generate
                DMA_INT |= (DMA_INT_INT0 << channel);
    #if defined eDMA_SHARES_INTERRUPTS
                if (fnGenInt(irq_DMA0_ID + (channel%(DMA_CHANNEL_COUNT/2))) != 0)
    #else
                if (fnGenInt(irq_DMA0_ID + channel) != 0)
    #endif
                {                                                        // if DMA channel interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    switch (channel) {
                    case 0:
    #if defined eDMA_SHARES_INTERRUPTS
                    case (0 + (DMA_CHANNEL_COUNT / 2)):
    #endif
                        ptrVect->processor_interrupts.irq_DMA0();        // call the interrupt handler for DMA channel 0 (and possibly shared channel)
                        break;
                    case 1:
    #if defined eDMA_SHARES_INTERRUPTS
                    case (1 + (DMA_CHANNEL_COUNT / 2)):
    #endif
                        ptrVect->processor_interrupts.irq_DMA1();        // call the interrupt handler for DMA channel 1 (and possibly shared channel)
                        break;
                    case 2:
    #if defined eDMA_SHARES_INTERRUPTS
                    case (2 + (DMA_CHANNEL_COUNT / 2)):
    #endif
                        ptrVect->processor_interrupts.irq_DMA2();        // call the interrupt handler for DMA channel 2 (and possibly shared channel)
                        break;
                    case 3:
    #if defined eDMA_SHARES_INTERRUPTS
                    case (3 + (DMA_CHANNEL_COUNT / 2)):
    #endif
                        ptrVect->processor_interrupts.irq_DMA3();        // call the interrupt handler for DMA channel 3 (and possibly shared channel)
                        break;
    #if DMA_CHANNEL_COUNT > 4
                    case 4:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (4 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA4();        // call the interrupt handler for DMA channel 4 (and possibly shared channel)
                        break;
                    case 5:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (5 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA5();        // call the interrupt handler for DMA channel 5 (and possibly shared channel)
                        break;
                    case 6:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (6 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA6();        // call the interrupt handler for DMA channel 6 (and possibly shared channel)
                        break;
                    case 7:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (7 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA7();        // call the interrupt handler for DMA channel 7 (and possibly shared channel)
                        break;
    #endif
    #if DMA_CHANNEL_COUNT > 8
                    case 8:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (8 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA8();        // call the interrupt handler for DMA channel 8 (and possibly shared channel)
                        break;
                    case 9:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (9 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA9();        // call the interrupt handler for DMA channel 9 (and possibly shared channel)
                        break;
                    case 10:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (10 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA10();       // call the interrupt handler for DMA channel 10 (and possibly shared channel)
                        break;
                    case 11:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (11 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA11();       // call the interrupt handler for DMA channel 11 (and possibly shared channel)
                        break;
                    case 12:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (12 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA12();       // call the interrupt handler for DMA channel 12 (and possibly shared channel)
                        break;
                    case 13:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (13 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA13();       // call the interrupt handler for DMA channel 13 (and possibly shared channel)
                        break;
                    case 14:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (14 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA14();       // call the interrupt handler for DMA channel 14 (and possibly shared channel)
                        break;
                    case 15:
        #if defined eDMA_SHARES_INTERRUPTS
                    case (15 + (DMA_CHANNEL_COUNT / 2)):
        #endif
                        ptrVect->processor_interrupts.irq_DMA15();       // call the interrupt handler for DMA channel 15 (and possibly shared channel)
                        break;
    #endif
                    }
                }
            }
            if (ptrDMA_TCD->DMA_TCD_CITER_ELINK == 0) {
                return 0;                                                // completed
            }
        }
        return 1;                                                        // not completed
    }
#endif
    return -1;                                                           // no operation
}


static iMX_GPIO *fnGetGPIO(int iPort)
{
    switch (iPort) {
    case PORT1:
        return (iMX_GPIO *)GPIO1_BLOCK;
    case PORT2:
        return (iMX_GPIO *)GPIO2_BLOCK;
#if defined PORT3
    case PORT3:
        return (iMX_GPIO *)GPIO3_BLOCK;
#endif
#if defined PORT4
    case PORT4:
        return (iMX_GPIO *)GPIO4_BLOCK;
#endif
#if defined PORT5
    case PORT5:
        return (iMX_GPIO *)GPIO5_BLOCK;
#endif
    default:
        _EXCEPTION("Bad port reference!");
        return 0;
    }
}

// Handle port interrupts on input changes
//
static void fnPortInterrupt(int iPort, unsigned long ulNewState, unsigned long ulChangedBit)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    iMX_GPIO *ptrGPIO = fnGetGPIO(iPort);
    unsigned long ulReg;
#if NUMBER_OF_SDHC > 0
    fnCheckSD_Detect();                                                  // synchronise the SD card detecion state
#endif

    if ((ptrGPIO->GPIO_IMR & ulChangedBit) == 0) {                       // if no interrupt on this pin
        return;
    }
    if ((ptrGPIO->GPIO_EDGE_SEL & ulChangedBit) == 0) {                  // interrupt not generated on rising and falling edges
        if ((ulChangedBit & 0xffff0000) != 0) {
            unsigned long _ulBit = ulChangedBit;
            ulReg = ptrGPIO->GPIO_ICR2;
            while ((_ulBit & 0x00010000) == 0) {
                ulReg >>= 2;
                _ulBit >>= 1;
            }
        }
        else {
            unsigned long _ulBit = ulChangedBit;
            ulReg = ptrGPIO->GPIO_ICR1;
            while ((_ulBit & 0x00000001) == 0) {
                ulReg >>= 2;
                _ulBit >>= 1;
            }
        }
        switch (ulReg & PORT_ICR1_0_INT_MASK) {
        case PORT_ICR1_0_LOW_LEVEL:
        case PORT_ICR1_0_FALLING:
            if ((ulNewState & ulChangedBit) != 0) {
                return;                                                  // not correct level
            }
            break;
        case PORT_ICR1_0_RISING:
        case PORT_ICR1_0_HIGH_LEVEL:
            if ((ulNewState & ulChangedBit) == 0) {
                return;                                                  // not correct level
            }
            break;
        }
    }
    ptrGPIO->GPIO_ISR |= ulChangedBit;                                   // GPIO interrupt triggered
    switch (iPort) {
    case PORT1:
        switch (ulChangedBit) {
#if defined irq_GPIO1_Hi_0_ID
        case PORT1_BIT0:
            if (fnGenInt(irq_GPIO1_Hi_0_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_0();          // call port interrupt handler
            }
            break;
        case PORT1_BIT1:
            if (fnGenInt(irq_GPIO1_Hi_1_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_1();          // call port interrupt handler
            }
            break;
        case PORT1_BIT2:
            if (fnGenInt(irq_GPIO1_Hi_2_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_2();          // call port interrupt handler
            }
            break;
        case PORT1_BIT3:
            if (fnGenInt(irq_GPIO1_Hi_3_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_3();          // call port interrupt handler
            }
            break;
        case PORT1_BIT4:
            if (fnGenInt(irq_GPIO1_Hi_4_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_4();          // call port interrupt handler
            }
            break;
        case PORT1_BIT5:
            if (fnGenInt(irq_GPIO1_Hi_5_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_5();          // call port interrupt handler
            }
            break;
        case PORT1_BIT6:
            if (fnGenInt(irq_GPIO1_Hi_6_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_6();          // call port interrupt handler
            }
            break;
        case PORT1_BIT7:
            if (fnGenInt(irq_GPIO1_Hi_7_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO1_Hi_7();          // call port interrupt handler
            }
            break;
#endif
        default:
            if (ulChangedBit >= 0x00010000) {
                if (fnGenInt(irq_GPIO1_16_31_ID) != 0) {                 // if port interrupt is not disabled
                    ptrVect->processor_interrupts.irq_GPIO1_16_31();     // call port interrupt handler
                }
            }
            else {
                if (fnGenInt(irq_GPIO1_0_15_ID) != 0) {                  // if port interrupt is not disabled
                    ptrVect->processor_interrupts.irq_GPIO1_0_15();      // call port interrupt handler
                }
            }
            break;
        }
        break;
    case PORT2:
        if (ulChangedBit >= 0x00010000) {
#if defined irq_GPIO2_16_31_ID
            if (fnGenInt(irq_GPIO2_16_31_ID) != 0) {                     // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO2_16_31();         // call port interrupt handler
            }
#endif
        }
        else {
            if (fnGenInt(irq_GPIO2_0_15_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO2_0_15();          // call port interrupt handler
            }
        }
        break;
#if defined PORT3
    case PORT3:
        if (ulChangedBit >= 0x00010000) {
            if (fnGenInt(irq_GPIO3_16_31_ID) != 0) {                     // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO3_16_31();         // call port interrupt handler
            }
        }
        else {
            if (fnGenInt(irq_GPIO3_0_15_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO3_0_15();          // call port interrupt handler
            }
        }
        break;
#endif
#if defined PORT4
    case PORT4:
        if (ulChangedBit >= 0x00010000) {
            if (fnGenInt(irq_GPIO4_16_31_ID) != 0) {                     // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO4_16_31();         // call port interrupt handler
            }
        }
        else {
            if (fnGenInt(irq_GPIO4_0_15_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO4_0_15();          // call port interrupt handler
            }
        }
        break;
#endif
#if defined PORT5
    case PORT5:
        if (ulChangedBit >= 0x00010000) {
    #if defined irq_GPIO5_16_31_ID
            if (fnGenInt(irq_GPIO5_16_31_ID) != 0) {                     // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO5_16_31();         // call port interrupt handler
            }
    #endif
        }
        else {
            if (fnGenInt(irq_GPIO5_0_15_ID) != 0) {                      // if port interrupt is not disabled
                ptrVect->processor_interrupts.irq_GPIO5_0_15();          // call port interrupt handler
            }
        }
        break;
#endif
    }
}

#if defined I2C_INTERFACE

static void fnInterruptI2C(int irq_I2C_ID)
{
    if (fnGenInt(irq_I2C_ID) != 0){                                      // if I2C interrupt is not disabled
        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
        switch (irq_I2C_ID) {
    #if defined irq_LPI2C0_ID                                            // low power I2C
        case irq_LPI2C0_ID:
            ptrVect->processor_interrupts.irq_LPI2C0();                  // call the interrupt handler
            break;
    #else
        case irq_I2C0_ID:
            ptrVect->processor_interrupts.irq_I2C0();                    // call the interrupt handler
            break;
    #endif
    #if I2C_AVAILABLE > 1
        #if defined irq_LPI2C0_ID                                        // low power I2C
        case irq_LPI2C1_ID:
            ptrVect->processor_interrupts.irq_LPI2C1();                  // call the interrupt handler
            break;
        #elif defined irq_I2C1_EXTENDED_ID
        case irq_I2C1_EXTENDED_ID:
            ptrVect->processor_interrupts.irq_I2C1();                    // call the interrupt handler
            break;
        #else
        case irq_I2C1_ID:
            ptrVect->processor_interrupts.irq_I2C1();                    // call the interrupt handler
            break;
        #endif
    #endif
    #if I2C_AVAILABLE > 2
        #if !defined irq_I2C2_ID
        case INTMUX0_PERIPHERAL_I2C2:
            fnCallINTMUX(INTMUX_I2C2, INTMUX0_PERIPHERAL_I2C2, (unsigned char *)&ptrVect->processor_interrupts.irq_LPI2C2);
        #else
        case irq_I2C2_ID:
            ptrVect->processor_interrupts.irq_I2C2();                    // call the interrupt handler
            break;
        #endif
    #endif
    #if I2C_AVAILABLE > 3
        case irq_I2C3_ID:
            ptrVect->processor_interrupts.irq_I2C3();                    // call the interrupt handler
            break;
    #endif
        }
    }
}

extern void fnSimulateI2C(int iPort, unsigned char *ptrDebugIn, unsigned short usLen, int iRepeatedStart)
{
#if I2C_AVAILABLE > 0
    KINETIS_I2C_CONTROL *ptrI2C = 0;
    int iI2C_irq = 0;
    switch (iPort) {
    case 0:
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C0_BLOCK;
    #if defined irq_LPI2C0_ID
        iI2C_irq = irq_LPI2C0_ID;
    #else
        iI2C_irq = irq_I2C0_ID;
    #endif
        break;
    #if I2C_AVAILABLE > 1
    case 1:
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C1_BLOCK;
        #if defined irq_LPI2C1_ID
        iI2C_irq = irq_LPI2C1_ID;
        #elif !defined irq_I2C1_ID
        iI2C_irq = irq_I2C1_EXTENDED_ID;
        #else
        iI2C_irq = irq_I2C1_ID;
        #endif
        break;
    #endif
    #if I2C_AVAILABLE > 2
    case 2:
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C2_BLOCK;
        #if defined irq_LPI2C2_EXTENDED_ID
        iI2C_irq = irq_LPI2C2_EXTENDED_ID;
        #else
        iI2C_irq = irq_I2C2_ID;
        #endif
        break;
    #endif
    #if I2C_AVAILABLE > 3
    case 3:
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C3_BLOCK;
        iI2C_irq = irq_I2C3_ID;
        break;
    #endif
    default:
        _EXCEPTION("Injecting to invalid I2C channel!");
        break;
    }
    ptrI2C->I2C_S = (I2C_IBB);                                           // bus is busy
    #if defined I2C_START_CONDITION_INTERRUPT || defined DOUBLE_BUFFERED_I2C
    ptrI2C->I2C_FLT |= (I2C_FLT_FLT_STARTF);                             // start condition detect flag
    if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_SSIE) != 0) {                     // if the start/stop condition interrupt is enabled
        ptrI2C->I2C_S |= I2C_IIF;                                        // interrupt flag set
        fnInterruptI2C(iI2C_irq);                                        // generate the I2C interrupt
    }
    #endif
    if ((*ptrDebugIn & 0xfe) == (ptrI2C->I2C_A1 & 0xfe)) {               // if the address matches the slave address
        ptrI2C->I2C_S = (I2C_IBB | I2C_IAAS | I2C_TCF);                  // bus is busy, addressed as slave and transfer is complete
        ptrI2C->I2C_D = *ptrDebugIn++;                                   // the address is put to the data register
        if ((ptrI2C->I2C_D & 0x01) != 0) {                               // addressed for read
            ptrI2C->I2C_S |= I2C_SRW;
            fnInterruptI2C(iI2C_irq);                                    // generate the I2C interrupt
            usLen = *ptrDebugIn;                                         // the number of bytes to be read
            while (usLen-- > 0) {
                if (usLen == 0) {
                    ptrI2C->I2C_S |= I2C_RXACK;                          // the master doesn't acknowledge the final byte
                }
                else {
                    ptrI2C->I2C_S &= ~I2C_RXACK;
                }
                fnInterruptI2C(iI2C_irq);                                // generate the I2C interrupt
            }
            ptrI2C->I2C_S = I2C_SRW;                                     // slave transmit
        }
        else {
            fnInterruptI2C(iI2C_irq);                                    // generate the I2C interrupt
            while (usLen-- > 1) {
                ptrI2C->I2C_D = *ptrDebugIn++;                           // next byte is put to the data register
                fnInterruptI2C(iI2C_irq);                                // generate the I2C interrupt
            }
        }
    }
    else {
        ptrI2C->I2C_S = (I2C_IBB | I2C_TCF);                             // bus is busy and transfer is complete
    }
    ptrI2C->I2C_S = 0;
    #if defined DOUBLE_BUFFERED_I2C
    ptrI2C->I2C_S = (I2C_TCF);                                           // transfer completed flag is set automatically in double-buffered mode
    #endif
    if (iRepeatedStart == 0) {
        ptrI2C->I2C_FLT |= (I2C_FLT_FLT_STOPF);                          // stop condition detect flag
        if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_INT) != 0) {                  // if the start/stop condition interrupt is enabled
            ptrI2C->I2C_S |= I2C_IIF;                                    // interrupt flag set
            fnInterruptI2C(iI2C_irq);                                    // generate the I2C interrupt
        }
    }
#endif
}
#endif


#if LPUARTS_AVAILABLE > 0
    #define LPUART0_CH_NUMBER     0
    #define LPUART1_CH_NUMBER     1
    #define LPUART2_CH_NUMBER     2
    #define LPUART3_CH_NUMBER     3
    #define LPUART4_CH_NUMBER     4
    #define LPUART5_CH_NUMBER     5
    #define LPUART6_CH_NUMBER     6
    #define LPUART7_CH_NUMBER     7

    #if defined DMAMUX0_CHCFG_SOURCE_LPUART0_RX
        #define DMUX_UART0_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART0_RX)
    #else
        #define DMUX_UART0_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART1_RX)
    #endif
    #if defined DMAMUX0_CHCFG_SOURCE_LPUART1_RX
        #define DMUX_UART1_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART1_RX)
    #else
        #define DMUX_UART1_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_UART1_RX)
    #endif
    #if defined DMAMUX0_CHCFG_SOURCE_LPUART2_RX
        #define DMUX_UART2_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART2_RX)
    #else
        #define DMUX_UART2_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_UART2_RX)
    #endif
    #if defined DMAMUX0_CHCFG_SOURCE_LPUART3_RX
        #define DMUX_UART3_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART3_RX)
    #else
        #define DMUX_UART3_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_UART3_RX)
    #endif
    #if defined DMAMUX0_CHCFG_SOURCE_LPUART4_RX
        #define DMUX_UART4_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART4_RX)
    #else
        #define DMUX_UART4_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_UART4_RX)
    #endif
    #define DMUX_UART5_RX_CHANNEL (DMAMUX0_CHCFG_SOURCE_LPUART5_RX)

    #if defined SERIAL_INTERFACE && !defined DEVICE_WITHOUT_DMA
static const unsigned char ucUART_channel[] = {
    DMA_UART0_RX_CHANNEL,
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 1
    DMA_UART1_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 2
    DMA_UART2_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 3
    DMA_UART3_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 4
    DMA_UART4_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 5
    DMA_UART5_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 6
    DMA_UART6_RX_CHANNEL,
        #endif
        #if (LPUARTS_AVAILABLE + UARTS_AVAILABLE) > 7
    DMA_UART7_RX_CHANNEL
        #endif
};

static const unsigned char ucUART_DMUX_channel[] = {
    DMAMUX0_CHCFG_SOURCE_LPUART1_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART2_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART3_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART4_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART5_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART6_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART7_RX,
    DMAMUX0_CHCFG_SOURCE_LPUART8_RX,
};
    #endif
#endif

extern void fnSimulateSPIIn(int iPort, unsigned char *ptrDebugIn, unsigned short usLen)
{
#if defined SPI_INTERFACE
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    switch (iPort) {
    case 0:
        while (usLen-- != 0) {                                           // for each byte
        #if defined DSPI_SPI
            SPI0_POPR = *ptrDebugIn++;
            SPI0_SR |= SPI_SR_RFDF;
            if ((SPI0_RSER & SPI_SRER_RFDF_RE) != 0) {
                ptrVect->processor_interrupts.irq_SPI0();                // call the interrupt handler
            }
        #else
            _EXCEPTION("To do...");
        #endif
        }
        break;
    #if SPI_AVAILABLE > 1
    case 1:
        while (usLen-- != 0) {                                           // for each byte
        #if defined DSPI_SPI
            SPI1_POPR = *ptrDebugIn++;
            SPI1_SR |= SPI_SR_RFDF;
            if ((SPI1_RSER & SPI_SRER_RFDF_RE) != 0) {
                ptrVect->processor_interrupts.irq_SPI1();                // call the interrupt handler
            }
        #else
            _EXCEPTION("To do...");
        #endif
        }
        break;
    #endif
    #if SPI_AVAILABLE > 2
    case 2:
        while (usLen-- != 0) {                                           // for each byte
        #if defined DSPI_SPI
            SPI2_POPR = *ptrDebugIn++;
            SPI2_SR |= SPI_SR_RFDF;
            if ((SPI2_RSER & SPI_SRER_RFDF_RE) != 0) {
                ptrVect->processor_interrupts.irq_SPI2();                // call the interrupt handler
            }
        #else
            _EXCEPTION("To do...");
        #endif
        }
        break;
    #endif
    }
#endif
}

// Simulate the reception of serial data by inserting bytes into the input buffer and calling interrupts
//
extern void fnSimulateSerialIn(int iPort, unsigned char *ptrDebugIn, unsigned short usLen)
{
#if defined SERIAL_INTERFACE
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    #if defined LOG_UART_RX
    if (ptrDebugIn != 0) {
        fnLogRx(iPort, ptrDebugIn, usLen);
    }
    #endif
    #if NUMBER_EXTERNAL_SERIAL > 0
    if (iPort >= NUMBER_SERIAL) {
        extern int fnRxExtSCI(int iChannel, unsigned char *ptrData, unsigned short usLength);
        int iHandled;
        while ((iHandled = fnRxExtSCI((iPort - NUMBER_SERIAL), ptrDebugIn, usLen)) > 0) { // handle reception of each byte
            switch (iPort - NUMBER_SERIAL) {                             // generate an interrupt on the corresponding port line (assumed negative)
            case 0:
            case 1:
                fnSimulateInputChange(EXT_UART_0_1_INT_PORT, fnMapPortBit(EXT_UART_0_1_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters
                break;
        #if NUMBER_EXTERNAL_SERIAL > 2
            case 2:
            case 3:
                fnSimulateInputChange(EXT_UART_2_3_INT_PORT, fnMapPortBit(EXT_UART_2_3_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters              
                break;
        #endif
            }
            ptrDebugIn += iHandled;
            usLen -= iHandled;
        }
        return;
    }
    #endif
    #if defined SERIAL_INTERFACE
    if (usLen != 0) {
        iUART_rx_Active[iPort] = 1;                                      // mark that the reception line is not idle
    }
    #endif
    #if LPUARTS_AVAILABLE > 0                                            // parts with LPUART
        switch (iPort) {
        case 0:                                                          // LPUART0
            if ((LPUART0_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                if (ptrDebugIn == 0) {                                   // idle line detection
                    LPUART0_STAT |= LPUART_STAT_IDLE;                    // mark idle line status
                    if ((LPUART0_CTRL & LPUART_CTRL_ILIE) != 0) {        // if the idle line interrupt is enabled
                        if (fnGenInt(irq_LPUART0_ID) != 0) {             // if LPUART0 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_LPUART0(); // call the interrupt handler
                        }
                    }
                    return;
                }
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART0_DATA = *ptrDebugIn++;
                    LPUART0_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART0_CTRL & LPUART_CTRL_RIE) != 0) {
                                                                         // if reception interrupt is enabled
        #if !defined DEVICE_WITHOUT_DMA                                  // if the device supports DMA
                        if ((LPUART0_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the LPUART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART0_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART0_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART0_CH_NUMBER], ucUART_DMUX_channel[LPUART0_CH_NUMBER]); // trigger DMA transfer on the UART's channel
                                LPUART0_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
        #endif
                            if (fnGenInt(irq_LPUART0_ID) != 0) {         // if LPUART0 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART0(); // call the interrupt handler
                            }
        #if !defined DEVICE_WITHOUT_DMA                                  // if the device supports DMA
                        }
        #endif
                    }
                }
            }
            break;
        #if LPUARTS_AVAILABLE > 1
        case 1:                                                          // LPUART1
            if ((LPUART1_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                if (ptrDebugIn == 0) {                                   // idle line detection
                    LPUART1_STAT |= LPUART_STAT_IDLE;                    // mark idle line status
                    if ((LPUART1_CTRL & LPUART_CTRL_ILIE) != 0) {        // if the idle line interrupt is enabled
                        if (fnGenInt(irq_LPUART1_ID) != 0) {             // if LPUART1 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_LPUART1(); // call the interrupt handler
                        }
                    }
                    return;
                }
                while ((usLen--) != 0) {                                 // for each reception character
                    LPUART1_DATA = *ptrDebugIn++;
                    LPUART1_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART1_CTRL & LPUART_CTRL_RIE) != 0) {
                                                                         // if reception interrupt is enabled
            #if !defined DEVICE_WITHOUT_DMA                              // if the device supports DMA
                        if ((LPUART1_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the UART is operating in DMA reception mode
                #if defined SERIAL_SUPPORT_DMA
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART1_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART1_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART1_CH_NUMBER], ucUART_DMUX_channel[LPUART1_CH_NUMBER]); // trigger DMA transfer on the UART's channel
                                LPUART1_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
                #endif
                        }
                        else {
            #endif
                            if (fnGenInt(irq_LPUART1_ID) != 0) {         // if LPUART1 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART1(); // call the interrupt handler
                            }
            #if !defined DEVICE_WITHOUT_DMA                              // if the device supports DMA
                        }
            #endif
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 2
        case 2:                                                          // LPUART2
            if ((LPUART2_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                if (ptrDebugIn == 0) {                                   // idle line detection
                    LPUART2_STAT |= LPUART_STAT_IDLE;                    // mark idle line status
                    if ((LPUART2_CTRL & LPUART_CTRL_ILIE) != 0) {        // if the idle line interrupt is enabled
                        if (fnGenInt(irq_LPUART2_ID) != 0)  {            // if LPUART2 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_LPUART2(); // call the interrupt handler
                        }
                    }
                    return;
                }
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART2_DATA = *ptrDebugIn++;
                    LPUART2_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART2_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART2_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the LPUART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART2_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART2_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART2_CH_NUMBER], ucUART_DMUX_channel[LPUART2_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART2_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART2_ID) != 0) {         // if LPUART2 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART2(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 3
        case 3:
            if ((LPUART3_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART3_DATA = *ptrDebugIn++;
                    LPUART3_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART3_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART3_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the LPUART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA && defined DMA_UART3_RX_CHANNEL
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART3_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART3_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART3_CH_NUMBER], ucUART_DMUX_channel[LPUART3_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART3_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART3_ID) != 0) {         // if LPUART3 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART3(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 4
        case 4:
            if ((LPUART4_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART4_DATA = *ptrDebugIn++;                        // save the received byte into the reception data register
                    LPUART4_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART4_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART4_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the UART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA && defined DMA_UART4_RX_CHANNEL
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART4_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART4_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART4_CH_NUMBER], ucUART_DMUX_channel[LPUART4_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART4_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART4_ID) != 0) {         // if LPUART4 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART4(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 5
        case 5:
            if ((LPUART5_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART5_DATA = *ptrDebugIn++;                        // save the received byte into the reception data register
                    LPUART5_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART5_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART5_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the UART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA && defined DMA_UART5_RX_CHANNEL
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART5_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART5_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART5_CH_NUMBER], ucUART_DMUX_channel[LPUART5_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART5_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART5_ID) != 0) {         // if LPUART5 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART5(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 6
        case 6:
            if ((LPUART6_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART6_DATA = *ptrDebugIn++;                        // save the received byte into the reception data register
                    LPUART6_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART6_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART6_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the UART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA && defined DMA_UART6_RX_CHANNEL
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART6_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART6_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART6_CH_NUMBER], ucUART_DMUX_channel[LPUART6_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART6_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART6_ID) != 0) {         // if LPUART6 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART6(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 7
        case 7:
            if ((LPUART7_CTRL & LPUART_CTRL_RE) != 0) {                  // if receiver enabled
                while (usLen-- != 0) {                                   // for each reception character
                    LPUART7_DATA = *ptrDebugIn++;                        // save the received byte into the reception data register
                    LPUART7_STAT |= LPUART_STAT_RDRF;                    // set interrupt cause
                    if ((LPUART7_CTRL & LPUART_CTRL_RIE) != 0) {         // if reception interrupt is enabled
                        if ((LPUART7_BAUD & LPUART_BAUD_RDMAE) != 0) {   // if the UART is operating in DMA reception mode
            #if defined SERIAL_SUPPORT_DMA && defined DMA_UART7_RX_CHANNEL
                            if ((DMA_ERQ & (DMA_ERQ_ERQ0 << ucUART_channel[LPUART7_CH_NUMBER])) != 0) { // if source enabled
                                KINETIS_DMA_TDC *ptrDMA_TCD = (KINETIS_DMA_TDC *)eDMA_DESCRIPTORS;
                                ptrDMA_TCD += ucUART_channel[LPUART7_CH_NUMBER];
                                ptrDMA_TCD->DMA_TCD_CSR |= (DMA_TCD_CSR_ACTIVE); // trigger
                                fnSimulateDMA(ucUART_channel[LPUART7_CH_NUMBER], ucUART_DMUX_channel[LPUART7_CH_NUMBER]); // trigger DMA transfer on the LPUART's channel
                                LPUART7_STAT &= ~LPUART_STAT_RDRF;       // remove interrupt cause
                            }
            #endif
                        }
                        else {
                            if (fnGenInt(irq_LPUART7_ID) != 0) {         // if LPUART6 interrupt is not disabled
                                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                                ptrVect->processor_interrupts.irq_LPUART7(); // call the interrupt handler
                            }
                        }
                    }
                }
            }
            break;
        #endif
        }
        return;
        #if UARTS_AVAILABLE > 0
    }
        #endif
    #endif
#endif
}


// Handle UART transmission interrupts
//
static void fnUART_Tx_int(int iChannel)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    #if LPUARTS_AVAILABLE > 0
        #if UARTS_AVAILABLE > 0
    if (uart_type[iChannel] == UART_TYPE_LPUART) {
        #endif
        switch (iChannel) {
        case 0:                                                          // LPUART0
            if ((LPUART0_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART0_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART0_CTRL & LPUART0_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART0_ID) != 0) {                 // if LPUART0 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART0();     // call the interrupt handler
                    }
                }
            }
            break;
        #if LPUARTS_AVAILABLE > 1
        case 1:
            if ((LPUART1_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART1_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART1_CTRL & LPUART1_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART1_ID) != 0) {                 // if LPUART1 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART1();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 2
        case 2:
            if ((LPUART2_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART2_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART2_CTRL & LPUART2_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART2_ID) != 0) {                 // if LPUART2 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART2();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 3
        case 3:
            if ((LPUART3_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART3_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART3_CTRL & LPUART3_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART3_ID) != 0) {                 // if LPUART3 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART3();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 4
        case 4:
            if ((LPUART4_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART4_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART4_CTRL & LPUART4_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART4_ID) != 0) {                 // if LPUART4 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART4();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 5
        case 5:
            if ((LPUART5_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART5_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART5_CTRL & LPUART5_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART5_ID) != 0) {                 // if LPUART5 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART5();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 6
        case 6:
            if ((LPUART6_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART6_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART6_CTRL & LPUART6_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART6_ID) != 0) {                 // if LPUART6 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART6();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        #if LPUARTS_AVAILABLE > 7
        case 7:
            if ((LPUART7_CTRL & LPUART_CTRL_TE) != 0) {                  // if transmitter enabled
                LPUART7_STAT |= (LPUART_STAT_TDRE | LPUART_STAT_TC);     // set interrupt cause
                if ((LPUART7_CTRL & LPUART7_STAT) != 0) {                // if transmit interrupt type enabled
                    if (fnGenInt(irq_LPUART7_ID) != 0) {                 // if LPUART7 interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPUART7();     // call the interrupt handler
                    }
                }
            }
            break;
        #endif
        }
        return;
        #if UARTS_AVAILABLE > 0
    }
        #endif
    #endif
}

#if defined USB_INTERFACE
static void fnHandleUSB_Interrupts(int iController)
{
    if (iController != 0) {
    #if (USBS_AVAILABLE > 1)
        if ((iInts & USBHS1_INT) != 0) {
            int iEndPoint = 0;
            iInts &= ~USBHS1_INT;
            while ((iEndPoint < (NUMBER_OF_USBHS_ENDPOINTS * 2)) && (ulHSEndpointInt[1] != 0)) {
                if (ulHSEndpointInt[1] & (1 << iEndPoint)) {
                    ulHSEndpointInt[1] &= ~(1 << iEndPoint);
                    fnCheckUSBOut(1, 0, iEndPoint);
                }
                iEndPoint++;
            }
        }
    #endif
        return;
    }
    if ((iInts & USBHS_INT) != 0) {
        int iEndPoint = 0;
        iInts &= ~USBHS_INT;
        while ((iEndPoint < (NUMBER_OF_USBHS_ENDPOINTS * 2)) && (ulHSEndpointInt[0] != 0)) {
            if (ulHSEndpointInt[0] & (1 << iEndPoint)) {
                ulHSEndpointInt[0] &= ~(1 << iEndPoint);
                fnCheckUSBOut(0, 0, iEndPoint);
            }
            iEndPoint++;
        }
    }
}
#endif

// Process simulated interrupts
//
extern unsigned long fnSimInts(char *argv[])
{
#if defined SERIAL_INTERFACE
    extern unsigned char ucTxLast[NUMBER_SERIAL];
#endif
    unsigned long ulNewActions = 0;
    int *ptrCnt;

    if (((iInts & CHANNEL_0_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART0];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_0_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_0_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx0(ucTxLast[0]);
                ulNewActions |= SEND_COM_0;
                fnUART_Tx_int(0);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_1_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART1];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_1_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_1_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx1(ucTxLast[1]);
                ulNewActions |= SEND_COM_1;
                fnUART_Tx_int(1);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_2_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART2];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_2_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_2_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx2(ucTxLast[2]);
                ulNewActions |= SEND_COM_2;
                fnUART_Tx_int(2);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_3_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART3];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_3_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_3_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx3(ucTxLast[3]);
                ulNewActions |= SEND_COM_3;
                fnUART_Tx_int(3);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_4_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART4];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_4_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_4_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx4(ucTxLast[4]);
                ulNewActions |= SEND_COM_4;
                fnUART_Tx_int(4);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_5_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART5];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_5_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_5_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx5(ucTxLast[5]);
                ulNewActions |= SEND_COM_5;
                fnUART_Tx_int(5);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_6_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART6];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_6_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_6_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx6(ucTxLast[6]);
                ulNewActions |= SEND_COM_6;
                fnUART_Tx_int(6);
#endif
            }
        }
	}

    if (((iInts & CHANNEL_7_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_UART7];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_7_SERIAL_INT;                          // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_7_SERIAL_INT;                          // interrupt has been handled
#if defined SERIAL_INTERFACE
	            fnLogTx7(ucTxLast[7]);
                ulNewActions |= SEND_COM_7;
                fnUART_Tx_int(7);
#endif
            }
        }
	}

#if NUMBER_EXTERNAL_SERIAL > 0
	if (((iInts & CHANNEL_0_EXT_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_EXT_UART0];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_0_EXT_SERIAL_INT;                      // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_0_EXT_SERIAL_INT;                      // interrupt has been handled
                if (fnLogExtTx0() != 0) {
                    fnSimulateInputChange(EXT_UART_0_1_INT_PORT, fnMapPortBit(EXT_UART_0_1_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters
                }
                ulNewActions |= SEND_EXT_COM_0;
            }
        }
	}
	if (((iInts & CHANNEL_1_EXT_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_EXT_UART1];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_1_EXT_SERIAL_INT;                      // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_1_EXT_SERIAL_INT;                      // interrupt has been handled
                if (fnLogExtTx1() != 0) {
                    fnSimulateInputChange(EXT_UART_0_1_INT_PORT, fnMapPortBit(EXT_UART_0_1_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters
                }
                ulNewActions |= SEND_EXT_COM_1;
            }
        }
	}
    #if NUMBER_EXTERNAL_SERIAL > 2
	if (((iInts & CHANNEL_2_EXT_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_EXT_UART2];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_2_EXT_SERIAL_INT;                      // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_2_EXT_SERIAL_INT;                      // interrupt has been handled
                if (fnLogExtTx2() != 0) {
                    fnSimulateInputChange(EXT_UART_2_3_INT_PORT, fnMapPortBit(EXT_UART_2_3_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters
                }
                ulNewActions |= SEND_EXT_COM_2;
            }
        }
	}
	if (((iInts & CHANNEL_3_EXT_SERIAL_INT) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_EXT_UART3];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= CHANNEL_3_EXT_SERIAL_INT;                      // enough serial interupts handled in this tick period
            }
            else {
		        iInts &= ~CHANNEL_3_EXT_SERIAL_INT;                      // interrupt has been handled
                if (fnLogExtTx3() != 0) {
                    fnSimulateInputChange(EXT_UART_2_3_INT_PORT, fnMapPortBit(EXT_UART_2_3_INT_BIT), TOGGLE_INPUT_NEG); // generate interrupts for each character or a block of characters
                }
                ulNewActions |= SEND_EXT_COM_3;
            }
        }
	}
    #endif
#endif

#if defined I2C_INTERFACE
    if (((iInts & I2C_INT0) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C0];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= I2C_INT0;                                      // enough I2C interupts handled in this tick period
            }
            else {
		        iInts &= ~I2C_INT0;                                      // interrupt has been handled
		        iInts &= ~I2C_INT0;
                if (((LPI2C0_MIER & LPI2C0_MSR) & (LPI2C_MIER_TDIE | LPI2C_MIER_RDIE | LPI2C_MIER_EPIE | LPI2C_MIER_SDIE | LPI2C_MIER_NDIE | LPI2C_MIER_ALIE | LPI2C_MIER_FEIE | LPI2C_MIER_PLTIE | LPI2C_MIER_DMIE)) != 0) { // if an enabled interrupt source is active
                    if (fnGenInt(irq_LPI2C0_ID) != 0) {
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPI2C0();      // call the interrupt handler
                    }
                }
            }
        }
	}
    #if LPI2C_AVAILABLE > 1
    if (((iInts & I2C_INT1) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C1];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= I2C_INT1;                                      // enough I2C interupts handled in this tick period
            }
            else {
		        iInts &= ~I2C_INT1;                                      // interrupt has been handled
                if (((LPI2C1_MIER & LPI2C1_MSR) & (LPI2C_MIER_TDIE | LPI2C_MIER_RDIE | LPI2C_MIER_EPIE | LPI2C_MIER_SDIE | LPI2C_MIER_NDIE | LPI2C_MIER_ALIE | LPI2C_MIER_FEIE | LPI2C_MIER_PLTIE | LPI2C_MIER_DMIE)) != 0) { // if an enabled interrupt source is active
                    if (fnGenInt(irq_LPI2C1_ID) != 0) {
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPI2C1();      // call the interrupt handler
                    }
                }
            }
        }
	}
    #endif
    #if LPI2C_AVAILABLE > 2
    if (((iInts & I2C_INT2) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C2];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= I2C_INT2;                                      // enough I2C interupts handled in this tick period
            }
            else {
		        iInts &= ~I2C_INT2;                                      // interrupt has been handled
                if (((LPI2C2_MIER & LPI2C2_MSR) & (LPI2C_MIER_TDIE | LPI2C_MIER_RDIE | LPI2C_MIER_EPIE | LPI2C_MIER_SDIE | LPI2C_MIER_NDIE | LPI2C_MIER_ALIE | LPI2C_MIER_FEIE | LPI2C_MIER_PLTIE | LPI2C_MIER_DMIE)) != 0) { // if an enabled interrupt source is active
                    if (fnGenInt(irq_LPI2C2_ID) != 0) {
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPI2C2();      // call the interrupt handler
                    }
                }
            }
        }
	}
    #endif
    #if LPI2C_AVAILABLE > 3
    if (((iInts & I2C_INT3) != 0) && (argv != 0)) {
        ptrCnt = (int *)argv[THROUGHPUT_I2C3];
        if (*ptrCnt != 0) {
            if (--(*ptrCnt) == 0) {
                iMasks |= I2C_INT3;                                      // enough I2C interupts handled in this tick period
            }
            else {
		        iInts &= ~I2C_INT3;                                      // interrupt has been handled
                if (((LPI2C3_MIER & LPI2C3_MSR) & (LPI2C_MIER_TDIE | LPI2C_MIER_RDIE | LPI2C_MIER_EPIE | LPI2C_MIER_SDIE | LPI2C_MIER_NDIE | LPI2C_MIER_ALIE | LPI2C_MIER_FEIE | LPI2C_MIER_PLTIE | LPI2C_MIER_DMIE)) != 0) { // if an enabled interrupt source is active
                    if (fnGenInt(irq_LPI2C3_ID) != 0) {
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPI2C3();      // call the interrupt handler
                    }
                }
            }
        }
	}
    #endif
#endif
#if defined SPI_INTERFACE
    if ((iInts & CHANNEL_0_SPI_INT) != 0) {
        iInts &= ~CHANNEL_0_SPI_INT;                                     // interrupt has been handled
    #if defined DSPI_SPI
        SPI0_SR |= SPI_SR_TFFF;
        if ((SPI0_RSER & SPI_SRER_TFFF_RE) != 0) {                       // if transmitter fifo not full interrupt enabled
            if (fnGenInt(irq_SPI0_ID) != 0) {                            // if SPI0 interrupt is not disabled
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                ptrVect->processor_interrupts.irq_SPI0();                // call the interrupt handler
            }
        }
    #else
        _EXCEPTION("To do..");
    #endif
    }
    #if SPI_AVAILABLE > 1
    if ((iInts & CHANNEL_1_SPI_INT) != 0) {
        iInts &= ~CHANNEL_1_SPI_INT;                                     // interrupt has been handled
        #if defined DSPI_SPI
        SPI1_SR |= SPI_SR_TFFF;
        if ((SPI1_RSER & SPI_SRER_TFFF_RE) != 0)
        #elif defined LPSPI_SPI
        if (0)
        #else
        SPI1_S |= (SPI_S_SPTEF | SPI_S_SPRF);                            // set tranmitter empty and receiver not empty flags
        if ((SPI1_C1 & (SPI_C1_SPTIE | SPI_C1_SPIE)) != 0)               // if either of the interrupt sources are enabled
        #endif
        {                                                                // if transmitter fifo not full interrupt enabled
            #if !defined irq_SPI1_ID && defined INTMUX0_AVAILABLE
            if (fnGenInt(irq_INTMUX0_0_ID + INTMUX_SPI1) != 0)
            #else
            if (fnGenInt(irq_SPI1_ID) != 0)                              // if SPI1 interrupt is not disabled
            #endif
            {
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
            #if !defined irq_SPI1_ID && defined INTMUX0_AVAILABLE
                fnCallINTMUX(INTMUX_SPI1, INTMUX0_PERIPHERAL_SPI1, (unsigned char *)&ptrVect->processor_interrupts.irq_SPI1);
            #else
                ptrVect->processor_interrupts.irq_SPI1();                // call the interrupt handler
            #endif
            }
        }
    }
    #endif
    #if SPI_AVAILABLE > 2
    if ((iInts & CHANNEL_2_SPI_INT) != 0) {
        iInts &= ~CHANNEL_2_SPI_INT;                                     // interrupt has been handled
        #if defined DSPI_SPI
        SPI2_SR |= SPI_SR_TFFF;
        if ((SPI2_RSER & SPI_SRER_TFFF_RE) != 0) {                       // if transmitter fifo not full interrupt enabled
            if (fnGenInt(irq_SPI2_ID) != 0) {                            // if SPI2 interrupt is not disabled
                VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                ptrVect->processor_interrupts.irq_SPI2();                // call the interrupt handler
            }
        }
        #else
        _EXCEPTION("To do..");
        #endif
    }
    #endif
#endif
#if defined USB_INTERFACE
    fnHandleUSB_Interrupts(0);
    #if (USBS_AVAILABLE > 1)
    fnHandleUSB_Interrupts(1);
    #endif
#endif
    return ulNewActions;
}

#if defined USB_INTERFACE

#if defined USB_HOST_SUPPORT
static int fnGetPeriodicListLength(unsigned long ulUSBCMC)
{
    int iPeriodListLength = 0;
    switch (ulUSBCMC & (USBHS_USBCMD_FS_128 | USBHS_USBCMD_FS2)) {
    case USBHS_USBCMD_FS_8:
        iPeriodListLength = 8;
        break;
    case USBHS_USBCMD_FS_16:
        iPeriodListLength = 16;
        break;
    case USBHS_USBCMD_FS_32:
        iPeriodListLength = 32;
        break;
    case USBHS_USBCMD_FS_64:
        iPeriodListLength = 64;
        break;
    case USBHS_USBCMD_FS_128:
        iPeriodListLength = 128;
        break;
    case USBHS_USBCMD_FS_256:
        iPeriodListLength = 256;
        break;
    case USBHS_USBCMD_FS_512:
        iPeriodListLength = 512;
        break;
    case USBHS_USBCMD_FS_1024:
        iPeriodListLength = 1024;
        break;
    }
    return iPeriodListLength;
}

static KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *fnGetPeriodicEntry(unsigned long ulFRINDEX, unsigned long ulPERIODICLISTBASE, int iPeriodListLength)// {53}
{
    unsigned long *ptrList = (unsigned long *)(ulPERIODICLISTBASE + (((ulFRINDEX & 0x00003ff8) >> 1) & ((iPeriodListLength << 2) - 1))); // pointer into the periodic list
    if ((*ptrList & HOST_QUEUE_HEAD_WORD0_T) == 0) {                     // if the list entry is valid
        KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(*ptrList & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
        if ((*ptrList & HOST_QUEUE_HEAD_WORD0_TYPE_FSTN) != HOST_QUEUE_HEAD_WORD0_TYPE_QH) {
            _EXCEPTION("Queue head expected!");
        }
        ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostHead->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
        return ptrHostHead;
    }
    else {
        return 0;
    }
}
#endif

#define _fnLE_add(x) (void *)x

// Inject USB transactions for test purposes
//
extern int fnSimulateUSB(int iChannel, int iDevice, int iEndPoint, unsigned char ucPID, unsigned char *ptrDebugIn, unsigned short usLenEvent)
{
    #define ODD_BANK           0x04
    static unsigned char ucHSRxBank[HSUSB_CONTROLLERS][NUMBER_OF_USB_ENDPOINTS] = {{0}}; // monitor the buffer to inject to
    int iUSB_IRQ = irq_USB_HS_ID;
    KINETIS_USBHS *ptrHS_USB = (KINETIS_USBHS *)USBHS_BASE1_ADD;         // HS USB controller 1
    #if defined USB_HOST_SUPPORT
    int iPeriodic = 0;
    if (iEndPoint < 0) {
        iEndPoint = -iEndPoint;
        iPeriodic = 1;
    }
    #endif
    #if (USBS_AVAILABLE > 1)
    if (iChannel > 0) {
        ptrHS_USB = (KINETIS_USBHS *)USBHS_BASE2_ADD;                    // HS USB controller 2
        iUSB_IRQ = irq_USB_OTG1_ID;
    }
    #endif
    if ((ptrHS_USB->USBCMD & USBHS_USBCMD_RS) != 0) {                    // if the high speed USB controller is in the running state
        if (ptrDebugIn == 0) {                                           // bus state changes
            ptrHS_USB->USBSTS = 0;
            if ((usLenEvent & USB_RESET_CMD) != 0) {
                ptrHS_USB->USBSTS |= (USBHS_USBINTR_URE);                // set USB reset interrupt flag
                ptrHS_USB->PORTSC1 = (USBHS_PORTSC1_PR | USBHS_PORTSC1_CSC);
                ptrHS_USB->USBSTS |= USBHS_USBINTR_PCE;
                memset(ucHSRxBank,   0, sizeof(ucHSRxBank));             // default is even bank
                memset(ucHSTxBuffer, 0, sizeof(ucHSTxBuffer));           // default is even buffer
            }
            if ((usLenEvent & USB_SLEEP_CMD) != 0) {
                ptrHS_USB->PORTSC1 |= USBHS_PORTSC1_SUSP;
                ptrHS_USB->USBSTS |= USBHS_USBINTR_SLE;
            }
            if ((usLenEvent & USB_RESUME_CMD) != 0) {
                ptrHS_USB->PORTSC1 &= ~USBHS_PORTSC1_SUSP;
                ptrHS_USB->USBSTS |= USBHS_USBSTS_PCI;
            }
            if ((usLenEvent & USB_IN_SUCCESS) != 0) {
    #if defined USB_HOST_SUPPORT
                if ((ptrHS_USB->USBMODE & USBHS_USBMODE_CM_HOST_FLAG) != 0) {
                    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_USB->ASYNCLISTADDR_EPLISTADDR;
                    ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                    if (((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_OVERLAY_TERMINATE) == 0) { // if valid
                        USB_HS_HOST_TRANSFER_OVERLAY *ptr_qTD = (USB_HS_HOST_TRANSFER_OVERLAY *)((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                        unsigned char ucType = (ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_WORD0_TYPE_FSTN);
                        switch (ucType) {
                        case HOST_QUEUE_HEAD_WORD0_TYPE_iTD:             // item type - isochronous transfer descriptor
                        case HOST_QUEUE_HEAD_WORD0_TYPE_QsiTD:           // item type - split transaction isochronous transfer descriptor
                        case HOST_QUEUE_HEAD_WORD0_TYPE_FSTN:            // item type - frame span traversal node
                            _EXCEPTION("TO DO");
                            break;
                        case HOST_QUEUE_HEAD_WORD0_TYPE_QH:              // item type - queue head
                            if ((ptr_qTD->ulControl & HOST_OVERLAY_CONTROL_IOC) == 0) { // if interrupt not enabled
                                return 0;
                            }
                            ptrHS_USB->USBSTS |= USBHS_USBSTS_UI;        // transfer complete status
                            break;
                        }
                    }
                }
                else {
    #endif
                    ptrHS_USB->EPCOMPLETE |= (USBHS_EPCOMPLETE_ETCE0 << iEndPoint); // transmission complete
                    ptrHS_USB->USBSTS |= USBHS_USBSTS_UI;                // transfer complete status
    #if defined USB_HOST_SUPPORT
                }
    #endif
            }
    #if defined USB_HOST_SUPPORT
            if ((USB_HIGHSPEED_ATTACH_CMD & usLenEvent) != 0) {
                ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_PSPD_HS);
                if ((ptrHS_USB->PORTSC1 & USBHS_PORTSC1_CSC) == 0) {     // if not connected
                    ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_CCS | USBHS_PORTSC1_CSC); // change connection status to connectd
                }
                ptrHS_USB->USBSTS |= USBHS_USBSTS_PCI;
            }
            else if ((USB_FULLSPEED_ATTACH_CMD & usLenEvent) != 0) {     // full speed device attaches
                ptrHS_USB->PORTSC1 &= ~(USBHS_PORTSC1_PSPD_HS);
                ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_PSPD_FS);
                if ((ptrHS_USB->PORTSC1 & USBHS_PORTSC1_CSC) == 0) {     // if not connected
                    ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_CCS | USBHS_PORTSC1_CSC); // change connection status to connectd
                }
                ptrHS_USB->USBSTS |= USBHS_USBSTS_PCI;
            }
            else if ((USB_LOWSPEED_ATTACH_CMD & usLenEvent) != 0) {
                ptrHS_USB->PORTSC1 &= ~(USBHS_PORTSC1_PSPD_HS);
                ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_PSPD_LS);
                if ((ptrHS_USB->PORTSC1 & USBHS_PORTSC1_CSC) == 0) {     // if not connected
                    ptrHS_USB->PORTSC1 |= (USBHS_PORTSC1_CCS | USBHS_PORTSC1_CSC); // change connection status to connectd
                }
                ptrHS_USB->USBSTS |= USBHS_USBSTS_PCI;
            }
    #endif
        }
        else {                                                           // data
    #if defined USB_HOST_SUPPORT
            if ((ptrHS_USB->USBMODE & USBHS_USBMODE_CM_HOST_FLAG) != 0) {// if in host mode of operation
                KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader;
                if (iPeriodic != 0) {                                    // use periodic schedule
                    ptrHostQueueHeader = fnGetPeriodicEntry(ptrHS_USB->FRINDEX, ptrHS_USB->PERIODICLISTBASE_DEVICEADDR, fnGetPeriodicListLength(ptrHS_USB->USBCMD));
                    ptrHostQueueHeader->ptrCurrent_qTD = ptrHostQueueHeader->host_transfer_overlay.ptrNext_qTD;
                }
                else {                                                   // use asynchronous schedule
                    ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_USB->ASYNCLISTADDR_EPLISTADDR;
                    ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                    ptrHostQueueHeader += ((iEndPoint * 2) + 1);
                }
                if (((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_OVERLAY_TERMINATE) == 0) { // if valid
                    USB_HS_HOST_TRANSFER_OVERLAY *ptr_qTD = (USB_HS_HOST_TRANSFER_OVERLAY *)((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                    unsigned char ucType = (ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_WORD0_TYPE_FSTN);
                    switch (ucType) {
                    case HOST_QUEUE_HEAD_WORD0_TYPE_iTD:             // item type - isochronous transfer descriptor
                    case HOST_QUEUE_HEAD_WORD0_TYPE_QsiTD:           // item type - split transaction isochronous transfer descriptor
                    case HOST_QUEUE_HEAD_WORD0_TYPE_FSTN:            // item type - frame span traversal node
                        _EXCEPTION("TO DO");
                        break;
                    case HOST_QUEUE_HEAD_WORD0_TYPE_QH:              // item type - queue head
                        if ((ptr_qTD->ulControl & HOST_OVERLAY_CONTROL_IOC) == 0) { // if interrupt not enabled
                            return 0;
                        }
                        memcpy((unsigned char *)ptr_qTD->ulBufferPointerPage[0], ptrDebugIn, usLenEvent);
                        ptr_qTD->ulControl &= ~(HOST_OVERLAY_CONTROL_TOTAL_BYTES_MASK | HOST_OVERLAY_CONTROL_STATUS_ACTIVE);
                        ptr_qTD->ulControl |= ((((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) >> HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT) - usLenEvent) << HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT);
                        ptr_qTD->ulControl &= ~(HOST_OVERLAY_CONTROL_DT);
                        ptr_qTD->ulBufferPointerPage[0] += usLenEvent;
                        ptrHostQueueHeader->host_transfer_overlay.ulControl = ptr_qTD->ulControl;
                        ptrHostQueueHeader->host_transfer_overlay.ulBufferPointerPage[0] += usLenEvent;
                        ptrHostQueueHeader->host_transfer_overlay.ulBufferPointerPage[1] += 0x10;
                        ptrHS_USB->USBSTS |= USBHS_USBSTS_UI;
                        break;
                    }
                }

            }
            else {
    #endif
                KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHS_USB->ASYNCLISTADDR_EPLISTADDR + (2 * iEndPoint); // endpoint's reception queue header
                if ((unsigned char)(ptrHS_USB->PERIODICLISTBASE_DEVICEADDR >> USBHS_DEVICEADDR_USBADR_SHIFT) != (unsigned char)iDevice) { // not our device address so ignore
                    if (iDevice != 0xff) {                               // special broadcast for simulator use so that it doesn't have to know the USB address
                        return 1;
                    }
                }
                ptrHS_USB->USBSTS |= USBHS_USBSTS_UI;                    // transfer complete status

                switch (ucPID) {
                case SETUP_PID:
                    memcpy(ptrQueueHeader->ucSetupBuffer, ptrDebugIn, usLenEvent); // setup data content (always 8 bytes in length) is stored directly in the queue header
                    ptrHS_USB->EPSETUPSR |= (USBHS_EPSETUPSR_SETUP0 << iEndPoint); // setup complete
                    break;
                case OUT_PID:                                            // this presently generates an end of reception on each packet received - the USBHS controller does it only on complete data reception
                    if (ptrQueueHeader->CurrentdTD_pointer != 0) {
                        if (ptrQueueHeader->CurrentdTD_pointer->ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE) { // transfer buffer enabled for reception
                            if (ptrQueueHeader->CurrentdTD_pointer->ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_IOC) { // if an interrupt is to be generated on completion
                                ptrHS_USB->EPCOMPLETE |= (USBHS_EPCOMPLETE_ERCE0 << iEndPoint); // reception complete
                            }
                            memcpy((void *)ptrQueueHeader->CurrentdTD_pointer->ulBufferPointerPage[0], ptrDebugIn, usLenEvent); // copy data
                            ptrQueueHeader->CurrentdTD_pointer->ul_dtToken -= (usLenEvent << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT);
                            ptrQueueHeader->CurrentdTD_pointer->ul_dtToken &= ~(ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_MASK);
                        }
                    }
                    break;
                }
    #if defined USB_HOST_SUPPORT
            }
    #endif
            ucRxBank[iChannel][iEndPoint] ^= ODD_BANK;                   // the next one to be used - toggle mechanism
        }
        if (fnGenInt(iUSB_IRQ) != 0) {                                   // if USB HS interrupt is not disabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    #if (USBS_AVAILABLE > 1)
            if (iChannel > 0) {
                ptrVect->processor_interrupts.irq_USB_OTG1();            // call the controller 1interrupt handler
                return 0;
            }
    #endif
            ptrVect->processor_interrupts.irq_USB_OTG0();                // call the controller 0 interrupt handler
        } 
    }
    return 0;
}

// Handle data sent by USB
//
extern void fnSimUSB(int iChannel, int iType, int iEndPoint, USB_HW *ptrUSB_HW)
{
    extern void fnChangeUSBState(int iNewState, int iController);
    switch (iType) {
    case USB_SIM_RESET:
        {
            int x;
            fnChangeUSBState(0, iChannel);
            for (x = 0; x < NUMBER_OF_USB_ENDPOINTS; x++) {
                fnLogUSB(x, 0, 0, (unsigned char *)0xffffffff, 0);       // log reset on each endpoint
            }
        }
        break;
    case USB_SIM_SOF:
        _EXCEPTION("To do");
      //if ((CTL & HOST_MODE_EN) && (INT_ENB & SOF_TOK_EN)) {            // if host mode and SOF enabled
      //    iInts |= USB_INT;;
      //    ulEndpointInt = 1;                                           // flag endpoint - always endpoint 0 used
      //}
        break;
    case USB_SIM_WAIT:
        fnHandleUSB_Interrupts(iChannel);
        break;
    case USB_SIM_TX:                                                     // a frame transmission has just been started
        if (ptrUSB_HW->ucDeviceType == USB_DEVICE_HS) {
            int iUSB_IRQ = USBHS_INT;
            KINETIS_USBHS *ptrHS_USB = (KINETIS_USBHS *)USBHS_BASE1_ADD; // HS USB controller 1
            KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader;
    #if  (USBS_AVAILABLE > 1)
            if (iChannel != 0) {
                ptrHS_USB = (KINETIS_USBHS *)USBHS_BASE2_ADD;            // HS USB controller 2
                iUSB_IRQ = USBHS1_INT;
            }
    #endif
    #if defined USB_HOST_SUPPORT
            if ((ptrHS_USB->USBMODE & USBHS_USBMODE_CM_HOST_FLAG) != 0) { // if the HS USB controller is operating in host mode
                KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_USB->ASYNCLISTADDR_EPLISTADDR;
                ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                ptrHostQueueHeader += iEndPoint;
                if (((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_OVERLAY_TERMINATE) == 0) { // if valid
                    USB_HS_HOST_TRANSFER_OVERLAY *ptr_qTD;
                    unsigned char ucType;
                    ptrHostQueueHeader->ptrCurrent_qTD = ptrHostQueueHeader->host_transfer_overlay.ptrNext_qTD;
                    memcpy(&ptrHostQueueHeader->host_transfer_overlay, (unsigned char *)ptrHostQueueHeader->ptrCurrent_qTD, sizeof(ptrHostQueueHeader->host_transfer_overlay));
                    ptr_qTD = (USB_HS_HOST_TRANSFER_OVERLAY *)((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                    ucType = (ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_WORD0_TYPE_FSTN);
                    switch (ucType) {
                    case HOST_QUEUE_HEAD_WORD0_TYPE_iTD:             // item type - isochronous transfer descriptor
                    case HOST_QUEUE_HEAD_WORD0_TYPE_QsiTD:           // item type - split transaction isochronous transfer descriptor
                    case HOST_QUEUE_HEAD_WORD0_TYPE_FSTN:            // item type - frame span traversal node
                        _EXCEPTION("TO DO");
                        break;
                    case HOST_QUEUE_HEAD_WORD0_TYPE_QH:              // item type - queue head
                        {
                            if ((ptr_qTD->ulControl & HOST_OVERLAY_CONTROL_IOC) != 0) { // if interrupt on completion flag is set
                                iInts |= iUSB_IRQ;                   // flag that the interrupt should be handled
                                ulHSEndpointInt[iChannel] |= (1 << iEndPoint); // flag endpoint
                            }
                        }
                        break;
                    }
                }
                break;
            }
    #endif
            ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHS_USB->ASYNCLISTADDR_EPLISTADDR;
            ptrQueueHeader += ((2 * iEndPoint) + 1);                     // move to the transmitter queue header
            if (ptrQueueHeader->CurrentdTD_pointer != 0) {
                memcpy((void *)ptrQueueHeader->CurrentdTD_pointer, &ptrQueueHeader->dTD, sizeof(USB_HS_TRANSFER_OVERLAY)); // transfer the transfer block content to the overlay block
                if ((ptrQueueHeader->CurrentdTD_pointer->ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_IOC) != 0) { // if transfer termination should generate an interrupt
                    iInts |= iUSB_IRQ;                                   // flag that the interrupt should be handled
                    ulHSEndpointInt[iChannel] |= (1 << iEndPoint);       // flag endpoint
                }
            }
            break;
        }
        if ((OWN & *ptrUSB_HW->ptr_ulUSB_BDControl) == 0) {              // if the ownership has not been passed to the USB controller ignore it
            _EXCEPTION("Ignored frame due to incorrect buffer ownership");
            return;
        }
        iInts |= USBHS_INT;                                              // flag that the interrupt should be handled
        ulHSEndpointInt[iChannel] |= (1 << iEndPoint);                   // flag endpoint
        break;
    case USB_SIM_ENUMERATED:                                             // flag that we have completed enumeration
        fnChangeUSBState(1, iChannel);
        break;
    case USB_SIM_STALL:
        fnLogUSB(iEndPoint, 0, 1, (unsigned char *)0xffffffff, 0);       // log stall
        break;
    case USB_SIM_SUSPEND:
        fnChangeUSBState(0, iChannel);
        break;
    }
}


#if defined USB_HOST_SUPPORT

#define MAX_TOKENS 10
static unsigned char ucTokenQueue[MAX_TOKENS] = {0};
static int iTokenIn = 0;
static int iTokenOut = 0;

extern void fnAddToken(unsigned char ucToken)
{
    ucTokenQueue[iTokenIn++] = ucToken;
    if (iTokenIn >= MAX_TOKENS) {
        iTokenIn = 0;
    }
    iInts |= USBHS_INT;                                                  // there will generally be an interrupt after this so set the interrupt flag
    ulHSEndpointInt[0] |= (0x1 << (ucToken & 0x0f));
}

static unsigned char ucGetToken(int iNoIN)
{
    unsigned char ucToken;
    do {
        ucToken = ucTokenQueue[iTokenOut];
        if (ucToken != 0) {
            ucTokenQueue[iTokenOut] = 0;
            ++iTokenOut;
            if (iTokenOut >= MAX_TOKENS) {
                iTokenOut = 0;
            }
        }
    } while ((iNoIN != 0) && ((ucToken >> 4) == IN_PID));                // skip IN tokens if so requested
    return ucToken;
}

#define MOBILE_PHONE                                                     // simulate mobile RNDIS phone case
//#define SIMULATE_HUB_CASE


#if defined USB_MSD_HOST && !defined USB_RNDIS_HOST
#define MEMORY_STICK_USES_SHARED_ENDPOINTS
#define DEVICE_ENDPOINT0_LENGTH   64
static const unsigned char ucDeviceDescriptor[] = {                      // constant device descriptor (example of memory stick)
    0x12,
    0x01,
    0x00, 0x02,
    0x00, 0x00, 0x00,
    DEVICE_ENDPOINT0_LENGTH,
    #if defined MOBILE_PHONE
    0xd1, 0x12,
    0x7e, 0x10,
    0x99, 0x02,
    #else
    0x0d, 0x1d,
    0x13, 0x02,
    0x10, 0x01,
    #endif
    0x01, 0x02, 0x03,
    0x01
};

static const unsigned char ucConfigDescriptor[] = {                      // constant configuration descriptor (example of memory stick)
    #if defined MOBILE_PHONE
    0x09, 0x02, 0x3e, 0x00, 0x02, 0x01, 0x04, 0xc0, 0xfa,                // configuration descriptor (2 interfaces)
    0x09, 0x04, 0x00, 0x00, 0x03, 0xff, 0xff, 0x00, 0x05,                // interface descriptor (vendor specific 3 endpoints)
    0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00,                            // bulk IN endpoint - 64 bytes on endpoint 1
    0x07, 0x05, 0x01, 0x02, 0x00, 0x02, 0x00,                            // bulk OUT endpoint - 64 bytes on endpoint 1
    0x07, 0x05, 0x82, 0x03, 0x1c, 0x00, 0x06,                            // interrupt IN endpoint - 2 bytes - 1ms polling rate
    0x09, 0x04, 0x01, 0x00, 0x02, 0x08, 0x06, 0x50, 0x06,                // interface descriptor (mass storage 2 endpoints
    0x07, 0x05, 0x83, 0x02, 0x00, 0x02, 0x00,                            // bulk IN endpoint - 64 bytes on endpoint 3
    0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x01,                            // bulk OUT endpoint - 64 bytes on endpoint 2
    #else
    0x09, 0x02, 0x27, 0x00, 0x01, 0x01, 0x00, 0x80, 0x64,                // configuration descriptor
    0x09, 0x04, 0x00, 0x00, 0x03, 0x08, 0x06, 0x50, 0x00,                // interface descriptor
        #if defined MEMORY_STICK_USES_SHARED_ENDPOINTS
    0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,                            // bulk IN endpoint - 64 bytes on endpoint 2
        #else
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,                            // bulk IN endpoint - 64 bytes on endpoint 1
        #endif
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,                            // bulk OUT endpoint - 64 bytes on endpoint 2
    0x07, 0x05, 0x83, 0x03, 0x02, 0x00, 0x01                             // interrupt IN endpoint - 2 bytes - 1ms polling rate
    #endif
};

static unsigned char ucDataTransport[512] = {0};
static unsigned short usDataTransportLength = 0;
static int iStall = 0;
static unsigned long ulFrameCount = 0;
static unsigned char ucLastOpCode = 0;
static unsigned char ucPresentLUN = 0;
static unsigned long ulLBA = 0;
static unsigned char ucMemoryStickSector[512] = {0};
static int iSectorOffset = 0;

static unsigned char ucStatusTransport[] = {
    0x55, 0x53, 0x42, 0x53,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00
};

static const unsigned char ucMaxLum[1] = {0};

static int fnMsdModel(unsigned char *ptrUSBData, unsigned short usUSBLength)
{
    USB_MASS_STORAGE_CBW_LW *ptrBlockWrapper = (USB_MASS_STORAGE_CBW_LW *)ptrUSBData;
    if (ucLastOpCode == UFI_WRITE_10) {                                  // OUT data that belongs to a write is not a block wrapper
        if (usUSBLength > 512) {
            _EXCEPTION("Max. Endpoint size of 512 expected!!");
        }
        memcpy(&ucMemoryStickSector[iSectorOffset], ptrUSBData, usUSBLength); // collect the data into the temporary buffer
        iSectorOffset += usUSBLength;                                    // the new length of data collected
        if (--ulFrameCount == 0) {
            ucLastOpCode = 0;                                            // complete write has been received so reset the op code
        }
        if (iSectorOffset >= 512) {                                      // if a complete sector is available
            fnPutSimDiskData(ucMemoryStickSector, ucPresentLUN, ulLBA++);// save it to the memory stick
            iSectorOffset = 0;
        }
        return 1;                                                        // OUT belonging to a write has been handled
    }
    if (memcmp(ptrUSBData, "USBC", 4) != 0) {
        _EXCEPTION("USBC expected!");
    }
    ptrUSBData += 4;
    memcpy(&ucStatusTransport[4], ptrUSBData, 4);                        // tag ID for status stage
    ulFrameCount = 1;                                                    // default is that the content fits in a single frame
    ucLastOpCode = ptrBlockWrapper->CBWCB[0];
    switch (ucLastOpCode) {                                              // op code
    case UFI_INQUIRY:
        {
            static const unsigned char ucUFI_INQUIRY[] = {               // reference response to UFI_INQUIRY
                0x00, 0x80, 0x02, 0x02,                                  // removable SCSI-2
                0x1f, 0x00, 0x00, 0x00,                                  // length 31
                0x53, 0x57, 0x49, 0x53,                                  // SWISSBIT
                0x53, 0x42, 0x49, 0x54,
                0x56, 0x69, 0x63, 0x74,                                  // Victorinox 2.0
                0x6f, 0x72, 0x69, 0x6e,
                0x6f, 0x78, 0x20, 0x32,
                0x2e, 0x30, 0x20, 0x20,
                0x32, 0x2e, 0x30, 0x30
            };
            memcpy(ucDataTransport, ucUFI_INQUIRY, sizeof(ucUFI_INQUIRY));
            usDataTransportLength = sizeof(ucUFI_INQUIRY);
          //iStall = 1;                                                  // test stalling
        }
        break;
    case UFI_REQUEST_SENSE:
        {
            static const unsigned char ucUFI_REQUEST_SENSE[] = {         // reference response to UFI_REQUEST_SENSE
                0x70, 0x00, 0x06, 0x00,                                  // unit attention
                0x00, 0x00, 0x00, 0x0a,
                0x00, 0x00, 0x00, 0x00,
                0x28, 0x00, 0x00, 0x00,
                0x00, 0x00
            };
            memcpy(ucDataTransport, ucUFI_REQUEST_SENSE, sizeof(ucUFI_REQUEST_SENSE));
            usDataTransportLength = sizeof(ucUFI_REQUEST_SENSE);
        }
        break;
    case UFI_READ_FORMAT_CAPACITY:
        {
            static const unsigned char ucUFI_READ_FORMAT_CAPACITY[] = {  // reference response to UFI_READ_FORMAT_CAPACITY
                0x00, 0x00, 0x00, 0x08,                                  // capacity list length 8
                0x01, 0x00, 0x00, 0x00,                                  // number of blocks 16777216
                0x03, 0x00, 0x02, 0x00                                   // block size 512
            };
            memcpy(ucDataTransport, ucUFI_READ_FORMAT_CAPACITY, sizeof(ucUFI_READ_FORMAT_CAPACITY));
            usDataTransportLength = sizeof(ucUFI_READ_FORMAT_CAPACITY);
        }
        break;
    case UFI_READ_CAPACITY:
        {
            static const unsigned char ucUFI_READ_CAPACITY[] = {         // reference response to UFI_READ_CAPACITY
                0x00, 0x0f, 0x82, 0xff,                                  // logical bock address 1016575
                0x01, 0x00, 0x20, 0x00                                   // logical block length 512 (496.392 MB capacity)
            };
            memcpy(ucDataTransport, ucUFI_READ_CAPACITY, sizeof(ucUFI_READ_CAPACITY));
            usDataTransportLength = sizeof(ucUFI_READ_CAPACITY);
        }
        break;
    case UFI_WRITE_10:
    case UFI_READ_10:
        {
            if (ptrBlockWrapper->dCBWCBLength != 10) {
                _EXCEPTION("Incorrect read length!!");
            }
            ulLBA = ((ptrBlockWrapper->CBWCB[2] << 24) | (ptrBlockWrapper->CBWCB[3] << 16) | (ptrBlockWrapper->CBWCB[4] << 8) | (ptrBlockWrapper->CBWCB[5])); // the block number where the read starts
            ulFrameCount = ((ptrBlockWrapper->CBWCB[6] << 16) | (ptrBlockWrapper->CBWCB[7] << 8) | (ptrBlockWrapper->CBWCB[8])); // the number of bocks to be read
            ulFrameCount *= (512/64);                                    // the total amount of USB frames that will be returned
            ucPresentLUN = ptrBlockWrapper->dCBWLUN;
            if (UFI_READ_10 == ucLastOpCode) {
                fnGetSimDiskData(ucMemoryStickSector, ucPresentLUN, ulLBA); // read the sector's content from the simulated memory stick
                usDataTransportLength = 64;                              // endpoint assumed to be 64 bytes in length
                memcpy(ucDataTransport, ucMemoryStickSector, usDataTransportLength); // prepare the frame to be returned
                iSectorOffset = 64;
            }
            else {
                usDataTransportLength = 0;
                iSectorOffset = 0;
                return 1;                                                // write
            }
        }
        break;
    default:
        _EXCEPTION("Unexpected Op Code!");
        break;
    }
    return 0;
}
#elif defined USB_HOST_HUB && defined SIMULATE_HUB_CASE
    static int iHubResponse = 0;
    #define DEVICE_ENDPOINT0_LENGTH   64
    static const unsigned char ucDeviceDescriptor[] = {                      // constant device descriptor (example of hub)
        0x12, 0x01, 0x00, 0x02, 0x09, 0x00, 0x01, 0x40, 0xE3, 0x05, 0x05, 0x06, 0x0B, 0x06,
        0x00, 0x01, 0x00, 0x01
    };

    static const unsigned char ucConfigDescriptor[] = {                      // constant configuration descriptor (example of hub)
        0x09, 0x02, 0x19, 0x00, 0x01, 0x01, 0x00, 0xe0, 0x32,                // config descriptor
        0x09, 0x04, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00,                // interface descriptor
        0x07, 0x05, 0x81, 0x03, 0x01, 0x00, 0x0C                             // endpoint descriptor
    };
#elif defined USB_CDC_HOST
    #if defined MOBILE_PHONE
    #define DEVICE_ENDPOINT0_LENGTH   64
    static const unsigned char ucDeviceDescriptor[] = {                      // constant device descriptor (example of CDC)
        0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xd1, 0x12, 0x8a, 0x10, 0x99, 0x02,
        0x01, 0x02, 0x03, 0x01
    };
    #elif defined TELIT_LE910C1
    #define DEVICE_ENDPOINT0_LENGTH   64
    static const unsigned char ucDeviceDescriptor[] = {                      // constant device descriptor (example of CDC)
        #if 1                                                                // configuration 1203
        0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xC7, 0x1B, 0x03, 0x12, 0x18, 0x03,
        0x01, 0x02, 0x03, 0x01
        #else                                                                // configuration 1201
        0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, DEVICE_ENDPOINT0_LENGTH, 0xc7, 0x1b, 0x01, 0x12, 0x18, 0x03,
        0x01, 0x02, 0x03, 0x01 
        #endif
    };
    #else
    #define DEVICE_ENDPOINT0_LENGTH   8
    static const unsigned char ucDeviceDescriptor[] = {                      // constant device descriptor (example of CDC)
        0x12, 0x01, 0x00, 0x02, 0xef, 0x02, 0x01, DEVICE_ENDPOINT0_LENGTH, 0xA2, 0x15, 0x44, 0x00, 0x00, 0x01,
        0x01, 0x02, 0x03, 0x01 
    };
    #endif

static const unsigned char ucConfigDescriptor[] = {                      // constant configuration descriptor (example of CDC)
#if defined MOBILE_PHONE
    0x09, 0x02, 0x4B, 0x00, 0x02, 0x01, 0x04, 0xC0,
    0xFA, 0x08, 0x0B, 0x00, 0x02, 0xE0, 0x01, 0x03,
    0x07, 0x09, 0x04, 0x00, 0x00, 0x01, 0xE0, 0x01,
    0x03, 0x05, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05,
    0x24, 0x01, 0x00, 0x01, 0x04, 0x24, 0x02, 0x00,
    0x05, 0x24, 0x06, 0x00, 0x01, 0x07, 0x05, 0x82,
    0x03, 0x08, 0x00, 0x09, 0x09, 0x04, 0x01, 0x00,
    0x02, 0x0A, 0x00, 0x00, 0x06, 0x07, 0x05, 0x81,
    0x02, 0x00, 0x02, 0x00, 0x07, 0x05, 0x01, 0x02,
    0x00, 0x02, 0x00
#elif defined TELIT_LE910C1
    #if 1
    0x09, 0x02, 0x3D, 0x01, 0x08, 0x01, 0x00, 0xA0, 0xFA, 0x08, 0x0B, 0x00, 0x02, 0x02,
    0x06, 0x00, 0x08, 0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0xFF, 0x06, 0x05, 0x24,
    0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x01, 0x04, 0x24, 0x02, 0x00, 0x05, 0x24,
    0x06, 0x00, 0x01, 0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0x09, 0x09, 0x04, 0x01, 0x00,
    0x02, 0x0A, 0x00, 0x00, 0x07, 0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00, 0x07, 0x05,
    0x01, 0x02, 0x00, 0x02, 0x00, 0x09, 0x04, 0x02, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x00,
    0x07, 0x05, 0x83, 0x02, 0x00, 0x02, 0x00, 0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00,
    0x09, 0x04, 0x03, 0x00, 0x02, 0xFF, 0x42, 0x01, 0x0A, 0x07, 0x05, 0x03, 0x02, 0x00,
    0x02, 0x00, 0x07, 0x05, 0x84, 0x02, 0x00, 0x02, 0x00, 0x09, 0x04, 0x04, 0x00, 0x03,
    0xFF, 0x00, 0x00, 0x00, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x00,
    0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06, 0x00, 0x00, 0x07, 0x05, 0x86, 0x03, 0x0A,
    0x00, 0x09, 0x07, 0x05, 0x85, 0x02, 0x00, 0x02, 0x00, 0x07, 0x05, 0x04, 0x02, 0x00,
    0x02, 0x00, 0x09, 0x04, 0x05, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x05, 0x24, 0x00,
    0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06,
    0x00, 0x00, 0x07, 0x05, 0x88, 0x03, 0x0A, 0x00, 0x09, 0x07, 0x05, 0x87, 0x02, 0x00,
    0x02, 0x00, 0x07, 0x05, 0x05, 0x02, 0x00, 0x02, 0x00, 0x09, 0x04, 0x06, 0x00, 0x03,
    0xFF, 0x00, 0x00, 0x00, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x00,
    0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06, 0x00, 0x00, 0x07, 0x05, 0x8A, 0x03, 0x0A,
    0x00, 0x09, 0x07, 0x05, 0x89, 0x02, 0x00, 0x02, 0x00, 0x07, 0x05, 0x06, 0x02, 0x00,
    0x02, 0x00, 0x09, 0x04, 0x07, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x05, 0x24, 0x00,
    0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06,
    0x00, 0x00, 0x07, 0x05, 0x8C, 0x03, 0x0A, 0x00, 0x09, 0x07, 0x05, 0x8B, 0x02, 0x00,
    0x02, 0x00, 0x07, 0x05, 0x07, 0x02, 0x00, 0x02, 0x00
    #else
    0x09, 0x02, 0x19, 0x01, 0x07, 0x01, 0x00, 0xA0,
    0xFA, 0x09, 0x04, 0x00, 0x00, 0x02, 0xFF, 0xFF,
    0xFF, 0x00, 0x07, 0x05, 0x81, 0x02, 0x00, 0x02,
    0x00, 0x07, 0x05, 0x01, 0x02, 0x00, 0x02, 0x00,
    0x09, 0x04, 0x01, 0x00, 0x02, 0xFF, 0x42, 0x01,
    0x06, 0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00,
    0x07, 0x05, 0x82, 0x02, 0x00, 0x02, 0x00, 0x09,
    0x04, 0x02, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0x00,
    0x07, 0x05, 0x84, 0x03, 0x08, 0x00, 0x09, 0x07,
    0x05, 0x83, 0x02, 0x00, 0x02, 0x00, 0x07, 0x05,
    0x03, 0x02, 0x00, 0x02, 0x00, 0x09, 0x04, 0x03,
    0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x05, 0x24,
    0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x00, 0x00,
    0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06, 0x00,
    0x00, 0x07, 0x05, 0x86, 0x03, 0x0A, 0x00, 0x09,
    0x07, 0x05, 0x85, 0x02, 0x00, 0x02, 0x00, 0x07,
    0x05, 0x04, 0x02, 0x00, 0x02, 0x00, 0x09, 0x04,
    0x04, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x05,
    0x24, 0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x00,
    0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06,
    0x00, 0x00, 0x07, 0x05, 0x88, 0x03, 0x0A, 0x00,
    0x09, 0x07, 0x05, 0x87, 0x02, 0x00, 0x02, 0x00,
    0x07, 0x05, 0x05, 0x02, 0x00, 0x02, 0x00, 0x09,
    0x04, 0x05, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x00,
    0x05, 0x24, 0x00, 0x10, 0x01, 0x05, 0x24, 0x01,
    0x00, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24,
    0x06, 0x00, 0x00, 0x07, 0x05, 0x8A, 0x03, 0x0A,
    0x00, 0x09, 0x07, 0x05, 0x89, 0x02, 0x00, 0x02,
    0x00, 0x07, 0x05, 0x06, 0x02, 0x00, 0x02, 0x00,
    0x09, 0x04, 0x06, 0x00, 0x03, 0xFF, 0x00, 0x00,
    0x00, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05, 0x24,
    0x01, 0x00, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05,
    0x24, 0x06, 0x00, 0x00, 0x07, 0x05, 0x8C, 0x03,
    0x0A, 0x00, 0x09, 0x07, 0x05, 0x8B, 0x02, 0x00,
    0x02, 0x00, 0x07, 0x05, 0x07, 0x02, 0x00, 0x02,
    0x00
    #endif
#else
    #if 0                                                                // CDC with two interfaces
    0x09, 0x02, 0x8D, 0x00, 0x04, 0x01, 0x04, 0xC0, 0x00, 0x08, 0x0B, 0x00, 0x02, 0x02,
    0x02, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x05, 0x05, 0x24,
    0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x01, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24,
    0x06, 0x00, 0x01, 0x07, 0x05, 0x82, 0x03, 0x40, 0x00, 0x0A, 0x09, 0x04, 0x01, 0x00,
    0x02, 0x0A, 0x00, 0x00, 0x05, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05,
    0x81, 0x02, 0x40, 0x00, 0x00, 0x08, 0x0B, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x09,
    0x04, 0x02, 0x00, 0x01, 0x02, 0x02, 0x00, 0x05, 0x05, 0x24, 0x00, 0x10, 0x01, 0x05,
    0x24, 0x01, 0x01, 0x00, 0x04, 0x24, 0x02, 0x02, 0x05, 0x24, 0x06, 0x00, 0x01, 0x07,
    0x05, 0x84, 0x03, 0x40, 0x00, 0x0A, 0x09, 0x04, 0x03, 0x00, 0x02, 0x0A, 0x00, 0x00,
    0x05, 0x07, 0x05, 0x03, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x83, 0x02, 0x40, 0x00,
    0x00
    #elif 1                                                              // CDC + MSD composite
    0x09, 0x02, 0x79, 0x00, 0x04, 0x01, 0x00, 0xC0, 0xE8, 0x08, 0x0B, 0x00, 0x02, 0x02,
    0x03, 0x00, 0x02, 0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x05, 0x24,
    0x00, 0x10, 0x01, 0x05, 0x24, 0x01, 0x01, 0x01, 0x04, 0x24, 0x02, 0x06, 0x05, 0x24,
    0x06, 0x00, 0x01, 0x07, 0x05, 0x85, 0x03, 0x10, 0x00, 0x0A, 0x09, 0x04, 0x01, 0x00,
    0x02, 0x0A, 0x00, 0x00, 0x00, 0x07, 0x05, 0x83, 0x02, 0x10, 0x00, 0x00, 0x07, 0x05,
    0x04, 0x02, 0x10, 0x00, 0x00, 0x09, 0x04, 0x02, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x04,
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
    0x09, 0x04, 0x03, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00, 0x07, 0x05, 0x86, 0x02, 0x40,
    0x00, 0x00, 0x07, 0x05, 0x07, 0x02, 0x40, 0x00, 0x00
    #else
    0x09, 0x02, 0x4b, 0x00, 0x02, 0x01, 0x04, 0xc0, 0x00,                // configuration descriptor
    0x08, 0x0b, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00,                      // interface association descriptor
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x05,                // interface descriptor
    0x05, 0x24, 0x00, 0x00, 0x02,                             §           // class descriptor
    0x05, 0x24, 0x01, 0x01, 0x00,                                        // class descriptor
    0x04, 0x24, 0x02, 0x02,                                              // class descriptor
    0x05, 0x24, 0x06, 0x00, 0x01,                                        // class descriptor
    0x07, 0x05, 0x81, 0x03, 0x20, 0x00, 0x0a,                            // interrupt IN endpoint - 32 bytes on endpoint 1
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x05,                // interface descriptor
    0x07, 0x05, 0x02, 0x02, 0x20, 0x00, 0x00,                            // bulk OUT endpoint - 32 bytes on endpoint 2
    0x07, 0x05, 0x82, 0x02, 0x20, 0x00, 0x00,                            // bulk OUT endpoint - 32 bytes on endpoint 2
    #endif
#endif
};
#endif


static unsigned char ucStringDescriptor[] = {                            // string descriptor
    0x00,                                                                // length
    0x03,                                                                // type
    0x00, 0x00,                                                          // variable length content
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
};

#define QUEUE_DEVICE_DESCRIPTOR 1
#define QUEUE_ZERO_DATA         2
#define QUEUE_CONFIG_DESCRIPTOR 3
#define QUEUE_STRING_DESCRIPTOR 4
#define QUEUE_MAX_LUN           5

static void fnSimulateUSB_ep(int iChannel, int iAdd, unsigned char *ptrData, unsigned short usSize, int iEndPoint, int iPeriodic)
{
    unsigned short usSent = 0;
    unsigned short usRemaining = usSize;
    if (iPeriodic != 0) {
        iEndPoint = -iEndPoint;
    }
    while (usRemaining != 0) {
        if (usRemaining > DEVICE_ENDPOINT0_LENGTH) {
            fnSimulateUSB(iChannel, iAdd, iEndPoint, 0, ptrData, DEVICE_ENDPOINT0_LENGTH);
            usRemaining -= DEVICE_ENDPOINT0_LENGTH;
            ptrData += DEVICE_ENDPOINT0_LENGTH;
        }
        else {
            fnSimulateUSB(iChannel, iAdd, iEndPoint, 0, ptrData, usRemaining);
            if (usRemaining == DEVICE_ENDPOINT0_LENGTH) {
                fnSimulateUSB(iChannel, iAdd, iEndPoint, 0, ptrData, 0); // terminate with zero length frame
            }
            break;
        }
    }
}

#if defined USB_RNDIS_HOST
static unsigned char ucResponseMsg[128] = { 0 };
static unsigned char ucResponseMsgLength = 0;
static unsigned long ulLastMessageID = 0;
static int fnRNDIS_hostModel(unsigned char *ptrData, unsigned short usLength)
{
    unsigned long ulMessageID;
    unsigned long ulMessageLength;
    unsigned long ulOID;
    unsigned long ulMessageType = *ptrData++;
    ulMessageType |= (*ptrData++ << 8);
    ulMessageType |= (*ptrData++ << 16);
    ulMessageType |= (*ptrData++ << 24);
    ulMessageLength = *ptrData++;
    ulMessageLength |= (*ptrData++ << 8);
    ulMessageLength |= (*ptrData++ << 16);
    ulMessageLength |= (*ptrData++ << 24);
    ulMessageID = *ptrData++;
    ulMessageID |= (*ptrData++ << 8);
    ulMessageID |= (*ptrData++ << 16);
    ulMessageID |= (*ptrData++ << 24);
    ulLastMessageID = ulMessageID;
    switch (ulMessageType) {
    case REMOTE_NDIS_INITIALIZE_MSG:
        return 2;
    case REMOTE_NDIS_QUERY_MSG:
        ulOID = *ptrData++;
        ulOID |= (*ptrData++ << 8);
        ulOID |= (*ptrData++ << 16);
        ulOID |= (*ptrData++ << 24);
        switch (ulOID) {
        case OID_GEN_SUPPORTED_LIST:                                     // 0x00010101
            return 3;
        case 0x00010202:                                                 // unknown
            return 4;
        case OID_GEN_MAXIMUM_FRAME_SIZE:                                 // 0x00010106:
            return 5;
        case OID_GEN_LINK_SPEED:                                         // 0x00010107
            return 6;
        case OID_GEN_MEDIA_CONNECT_STATUS:                               // 0x00010114
            return 7;
        case OID_802_3_CURRENT_ADDRESS:                                  // 0x01010102
            return 8;
        case OID_802_3_PERMANENT_ADDRESS:                                // 0x01010101
            return 9;
        case OID_GEN_MAXIMUM_TOTAL_SIZE:                                 // 0x00010111
            return 11;
        }
        break;
    case REMOTE_NDIS_SET_MSG:
        return 10;
    case REMOTE_NDIS_KEEPALIVE_MSG:
        return 12;
    }
    return 0;
}

    #if defined USB_HOST_HUB && defined SIMULATE_HUB_CASE
static const unsigned char ucHubResponse1_ref[] = {
    0x09, 0x29, 0x04, 0xe9, 0x00, 0x32, 0x64, 0x00, 0xff
};

static const unsigned char ucHubResponse2_ref[] = {
    0x01, 0x00
};

static const unsigned char ucHubResponse3_ref[] = {
    0x00, 0x01, 0x00, 0x00
};

static const unsigned char ucHubResponse4_ref[] = {
    0x00, 0x00, 0x00, 0x00
};
    #endif

static const unsigned char ucResponse1_ref[] = {
    0x02, 0x00, 0x00, 0x80, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x2C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

static const unsigned char ucResponse2_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x70, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x01, 0x01, 0x00,
    0x03, 0x01, 0x01, 0x00, 0x04, 0x01, 0x01, 0x00, 0x06, 0x01, 0x01, 0x00, 0x07, 0x01, 0x01, 0x00,
    0x0a, 0x01, 0x01, 0x00, 0x0b, 0x01, 0x01, 0x00, 0x0c, 0x01, 0x01, 0x00, 0x0d, 0x01, 0x01, 0x00,
    0x16, 0x01, 0x01, 0x00, 0x0e, 0x01, 0x01, 0x00, 0x11, 0x01, 0x01, 0x00, 0x14, 0x01, 0x01, 0x00,
    0x02, 0x02, 0x01, 0x00, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x03, 0x01, 0x02, 0x00,
    0x04, 0x01, 0x02, 0x00, 0x05, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x03, 0x01, 0x01, 0x01, 0x05, 0x01, 0x01, 0x01, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01,
    0x02, 0x01, 0x02, 0x01, 0x03, 0x01, 0x02, 0x01
};

static const unsigned char ucResponse3_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1C, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char ucResponse4_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xdc, 0x05, 0x00, 0x00
};

static const unsigned char ucResponse5_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char ucResponse6_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};

static const unsigned char ucResponse7_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x6a, 0xda, 0x12, 0x2d, 0xe5, 0x6d
};

static const unsigned char ucResponse8_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1e, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x6a, 0xda, 0x12, 0x2d, 0xe5, 0x6d
};

static const unsigned char ucResponse9_ref[] = {
    0x05, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char ucResponse10_ref[] = {
    0x04, 0x00, 0x00, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x16, 0x06, 0x00, 0x00
};

static const unsigned char ucResponseLinkup_ref[] = {
    0x07, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00
};

static const unsigned char ucResponseKeepalive_ref[] = {
    0x08, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};


static void fnInjectResponse(int iChannel, int iAddress, int iEndPoint, const unsigned char *ptrData, int iSize)
{
    unsigned char ucResponse[256];
    memcpy(ucResponse, ptrData, iSize);
    if (ucResponse[0] != 0x07) {                                         // status change has no ID
        ucResponse[8] = (unsigned char)ulLastMessageID;
        ucResponse[9] = (unsigned char)(ulLastMessageID >> 8);
        ucResponse[10] = (unsigned char)(ulLastMessageID >> 16);
        ucResponse[11] = (unsigned char)(ulLastMessageID >> 24);
    }
    fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucResponse, (unsigned short)iSize, iEndPoint, 0);
}
#endif

// This routine reacts to embedded host mode transfers as a device would do
//
static int fnUSBHostModel(int iChannel, int iEndPoint, unsigned char ucPID, unsigned short usUSBLength, unsigned char *ptrUSBData, KINETIS_USBHS *ptrHSUSB, int iPeriodic)
{
#if defined USB_RNDIS_HOST
    static unsigned short usRNDIS_hostSetupDataLengthExpected[NUMBER_OF_USB_ENDPOINTS] = {0};
    static unsigned short usRNDIS_hostSetupDataReceived[NUMBER_OF_USB_ENDPOINTS] = {0};
    static unsigned char  ucRNDIS_hostSetupData[NUMBER_OF_USB_ENDPOINTS][64] = {{0}};
    static int iRNDIS_ready[NUMBER_OF_USB_ENDPOINTS] = {0};
    static int iZeroData = 0;
#endif
    static unsigned char ucStringRef = 0;
    static int iHostQueue = 0;

    switch (ucPID) {
    case 0:                                                              // reset
        iHostQueue = 0;
        break;
    case SETUP_PID:                                                      // the host is sending setup data
        {
            USB_HOST_DESCRIPTOR *ptrDescriptor = (USB_HOST_DESCRIPTOR *)ptrUSBData;
            if (usUSBLength != 8) {
                _EXCEPTION("Bad descriptor length detected!");
            }
    #if defined USB_HOST_HUB && defined SIMULATE_HUB_CASE
            if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_DEVICE_TO_HOST | REQUEST_DEVICE_CLASS)) { // class specific device request
                if (USB_REQUEST_GET_DESCRIPTOR == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iHubResponse = 1;                                    // respond on next IN
                    break;
                }
                else if (USB_REQUEST_GET_STATUS == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iHubResponse = 4;                                    // respond on next IN
                    break;
                }
            }
            else if (ptrDescriptor->bmRecipient_device_direction == STANDARD_DEVICE_TO_HOST) {
                if (USB_REQUEST_GET_STATUS == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iHubResponse = 2;                                    // respond on next IN
                    break;
                }
            }
            else if (ptrDescriptor->bmRecipient_device_direction == (REQUEST_OTHER_CLASS | STANDARD_DEVICE_TO_HOST)) {
                if (USB_REQUEST_GET_STATUS == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iHubResponse = 3;                                    // respond on next IN
                    break;
                }
            }
            else if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_HOST_TO_DEVICE | REQUEST_OTHER_CLASS)) {
                if (USB_REQUEST_SET_FEATURE == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iZeroData = 1;                                       // the control endpoint should terminate with a zero data status 
                    break;
                }
            }
            else if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_HOST_TO_DEVICE | REQUEST_DEVICE_STANDARD)) {
                if (USB_REQUEST_SET_FEATURE == ptrDescriptor->bRequest) {
                    unsigned short usMaxData;
                    usMaxData = ptrDescriptor->wLength[0];               // max. length of data content
                    usMaxData |= (ptrDescriptor->wLength[1] << 8);
                    iZeroData = 1;                                       // the control endpoint should terminate with a zero data status 
                    break;
                }
            }
    #endif
    #if defined USB_RNDIS_HOST
            if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_HOST_TO_DEVICE | REQUEST_INTERFACE_CLASS)) { // class specific interface request
                if ((USB_REQUEST_GET_STATUS == ptrDescriptor->bRequest) || (SET_CONTROL_LINE_STATE == ptrDescriptor->bRequest) || (SET_LINE_CODING == ptrDescriptor->bRequest)) {
                    USB_HOST_DESCRIPTOR *ptrControlSetup = (USB_HOST_DESCRIPTOR *)ptrUSBData;
                    usRNDIS_hostSetupDataLengthExpected[iEndPoint] = ptrControlSetup->wLength[0]; // length of data content expected
                    usRNDIS_hostSetupDataLengthExpected[iEndPoint] |= (ptrControlSetup->wLength[1] << 8);
                    usRNDIS_hostSetupDataReceived[iEndPoint] = 0;        // no data received yet
                    if (usRNDIS_hostSetupDataLengthExpected[iEndPoint] == 0) {
                        iZeroData = 1;                                   // the control endpoint should terminate with a zero data status 
                    }
                }
                break;
            }
            else if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_DEVICE_TO_HOST | REQUEST_INTERFACE_CLASS)) {
                if (USB_REQUEST_CLEAR_FEATURE == ptrDescriptor->bRequest) {
                    USB_HOST_DESCRIPTOR *ptrControlSetup = (USB_HOST_DESCRIPTOR *)ptrUSBData;
                    unsigned short usMaxData;
                    usMaxData = ptrControlSetup->wLength[0]; // max. length of data content
                    usMaxData |= (ptrControlSetup->wLength[1] << 8);
                }
            }
    #endif
            if (ptrDescriptor->bmRecipient_device_direction == (STANDARD_DEVICE_TO_HOST | REQUEST_INTERFACE_CLASS)) { // class specific interface request
    #if defined USB_RNDIS_HOST
                if (ucResponseMsgLength != 0) {
                    iRNDIS_ready[iEndPoint] = 11;
                }
    #endif
    #if defined USB_MSD_HOST && !defined USB_RNDIS_HOST
                if (GET_MAX_LUN == ptrDescriptor->bRequest) {            // requesting maximum LUN
                    iHostQueue = QUEUE_MAX_LUN;                          // send the data
                }
    #endif
                break;
            }
            if (USB_REQUEST_GET_DESCRIPTOR == ptrDescriptor->bRequest) { // host is requesting a descriptor 
                if (ptrDescriptor->wValue[1] == (STANDARD_DEVICE_DESCRIPTOR >> 8)) { // requesting the standard device descriptor
                    iHostQueue = QUEUE_DEVICE_DESCRIPTOR;                // send the device descriptor on next IN
                }
                else if (ptrDescriptor->wValue[1] == (STANDARD_CONFIG_DESCRIPTOR >> 8)) { // requesting the standard configuration descriptor
                    iHostQueue = QUEUE_CONFIG_DESCRIPTOR;                // send the configuration descriptor on next IN
                }
                else if (ptrDescriptor->wValue[1] == (STANDARD_STRING_DESCRIPTOR >> 8)) { // host requesting a string
                    iHostQueue = QUEUE_STRING_DESCRIPTOR;                // send the configuration descriptor on next IN
                    switch (ptrDescriptor->wValue[0]) {                  // the string referenced
                    case 0:                                              // string language ID
                        ucStringDescriptor[0] = 4;                       // length
                        ucStringDescriptor[2] = 0x09;                    // English (US)
                        ucStringDescriptor[3] = 0x04;
                        break;
                    case 1:                                              // manufacturer
                        ucStringDescriptor[0] = 0x12;                    // length
                        ucStringDescriptor[2] = 0x54;
                        ucStringDescriptor[3] = 0x00;
                        ucStringDescriptor[4] = 0x44;
                        ucStringDescriptor[5] = 0x00;
                        ucStringDescriptor[6] = 0x4b;
                        ucStringDescriptor[7] = 0x00;
                        ucStringDescriptor[8] = 0x4d;
                        ucStringDescriptor[9] = 0x00;
                        ucStringDescriptor[10] = 0x65;
                        ucStringDescriptor[11] = 0x00;
                        ucStringDescriptor[12] = 0x64;
                        ucStringDescriptor[13] = 0x00;
                        ucStringDescriptor[14] = 0x69;
                        ucStringDescriptor[15] = 0x00;
                        ucStringDescriptor[16] = 0x61;
                        ucStringDescriptor[17] = 0x00;
                        break;
                    case 2:                                              // product
                        ucStringDescriptor[0] = 0x10;                    // length
                        ucStringDescriptor[2] = 'C';
                        ucStringDescriptor[3] = 0x00;
                        ucStringDescriptor[4] = 'D';
                        ucStringDescriptor[5] = 0x00;
                        ucStringDescriptor[6] = 'C';
                        ucStringDescriptor[7] = 0x00;
                        ucStringDescriptor[8] = ' ';
                        ucStringDescriptor[9] = 0x00;
                        ucStringDescriptor[10] = ' ';
                        ucStringDescriptor[11] = 0x00;
                        ucStringDescriptor[12] = ' ';
                        ucStringDescriptor[13] = 0x00;
                        ucStringDescriptor[14] = ' ';
                        ucStringDescriptor[15] = 0x00;
                        break;
                    case 3:                                              // serial number
                        ucStringDescriptor[0] = 0x0c;                    // length
                        ucStringDescriptor[2] = 0x31;
                        ucStringDescriptor[3] = 0x00;
                        ucStringDescriptor[4] = 0x32;
                        ucStringDescriptor[5] = 0x00;
                        ucStringDescriptor[6] = 0x33;
                        ucStringDescriptor[7] = 0x00;
                        ucStringDescriptor[8] = 0x34;
                        ucStringDescriptor[9] = 0x00;
                        ucStringDescriptor[10] = 0x35;
                        ucStringDescriptor[11] = 0x00;
                        break;
                    }
                }
            }
            else if (USB_REQUEST_SET_ADDRESS == ptrDescriptor->bRequest) { // address being set
                static unsigned char ucDeviceAddress;
                ucDeviceAddress = ptrDescriptor->wValue[0];
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else if (USB_REQUEST_SET_CONFIGURATION == ptrDescriptor->bRequest) { // configuration being set
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else if (USB_REQUEST_CLEAR_FEATURE == ptrDescriptor->bRequest) { // clear feature
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else if (USB_REQUEST_SET_INTERFACE == ptrDescriptor->bRequest) { // set interface
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else if (SET_CONTROL_LINE_STATE == ptrDescriptor->bRequest) {
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else if (SET_LINE_CODING == ptrDescriptor->bRequest) {
                iHostQueue = QUEUE_ZERO_DATA;                            // device responds with a zero data IN
            }
            else {
                _EXCEPTION("Unknown request");
            }
        }
        break;
    case IN_PID:                                                         // the device can insert data since the host is sending IN PID
        {
            int iAddress;
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR;
            ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
            iAddress = (ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_DEVICE_ADDRESS_MASK);
            switch (iHostQueue) {
            case QUEUE_DEVICE_DESCRIPTOR:                                // inject the device descriptor
                fnSimulateUSB_ep(iChannel, iAddress,(unsigned char *)&ucDeviceDescriptor, sizeof(ucDeviceDescriptor), 0, 0);
                break;
            case QUEUE_CONFIG_DESCRIPTOR:
                fnSimulateUSB_ep(iChannel, iAddress,(unsigned char *)&ucConfigDescriptor, sizeof(ucConfigDescriptor), 0, 0);
                break;
            case QUEUE_STRING_DESCRIPTOR:
                fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)&ucStringDescriptor, ucStringDescriptor[0], 0, 0);
                break;
    #if defined USB_MSD_HOST && !defined USB_RNDIS_HOST
            case QUEUE_MAX_LUN:
                fnSimulateUSB(iChannel, iAddress, 0, 0, (unsigned char *)&ucMaxLum, sizeof(ucMaxLum));
                break;
    #endif
    #if defined USB_RNDIS_HOST
        case 0:
            if ((iZeroData != 0) && (iEndPoint == 0)) {
                iZeroData = 0;
                fnSimulateUSB(iChannel, iAddress, 0, 0, (unsigned char *)1, 0); // inject a zero data frame
            }
            else {
                int iThisEvent = iRNDIS_ready[iEndPoint];
                iRNDIS_ready[iEndPoint] = 0;
                switch (iThisEvent) {
                case 1:
                    {
                        static const unsigned char ucReady[] = { 1,0,0,0,0,0,0 };
                        fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucReady, sizeof(ucReady), iEndPoint, iPeriodic);
                    }
                    break;
                case 2:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse1_ref, sizeof(ucResponse1_ref));
                    break;
                case 3:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse2_ref, sizeof(ucResponse2_ref));
                    break;
                case 4:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse3_ref, sizeof(ucResponse3_ref));
                    break;
                case 5:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse4_ref, sizeof(ucResponse4_ref));
                    break;
                case 6:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse5_ref, sizeof(ucResponse5_ref));
                    break;
                case 7:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse6_ref, sizeof(ucResponse6_ref));
                    break;
                case 8:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse7_ref, sizeof(ucResponse7_ref));
                    break;
                case 9:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse8_ref, sizeof(ucResponse8_ref));
                    break;
                case 10:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse9_ref, sizeof(ucResponse9_ref));
                    break;
                case 11:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponse10_ref, sizeof(ucResponse10_ref));
                    iRNDIS_ready[2] = 1;                                 // when interrupt endpoint 2 is next polled it should return a response available message
                    iRNDIS_ready[iEndPoint] = 13;
                    break;
                case 13:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponseLinkup_ref, sizeof(ucResponseLinkup_ref));
                    break;
                case 12:
                    fnInjectResponse(iChannel, iAddress, iEndPoint, ucResponseKeepalive_ref, sizeof(ucResponseKeepalive_ref));
                    break;
                default:
         #if defined USB_HOST_HUB && defined SIMULATE_HUB_CASE
                    switch (iHubResponse) {
                    case 1:
                        fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucHubResponse1_ref, (unsigned short)sizeof(ucHubResponse1_ref), iEndPoint, 0);
                        iHubResponse = 0;
                        break;
                    case 2:
                        fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucHubResponse2_ref, (unsigned short)sizeof(ucHubResponse2_ref), iEndPoint, 0);
                        iHubResponse = 0;
                        break;
                    case 3:
                        fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucHubResponse3_ref, (unsigned short)sizeof(ucHubResponse3_ref), iEndPoint, 0);
                        iHubResponse = 0;
                        break;
                    case 4:
                        fnSimulateUSB_ep(iChannel, iAddress, (unsigned char *)ucHubResponse4_ref, (unsigned short)sizeof(ucHubResponse4_ref), iEndPoint, 0);
                        iHubResponse = 0;
                        break;
                    default:
                        break;
                    }
         #endif
                    return -1;
                }
            }
            break;
    #endif
            case QUEUE_ZERO_DATA:
                fnSimulateUSB(iChannel, iAddress, 0, 0, (unsigned char *)1, 0); // inject a zero data frame
                break;
            default:
    #if defined USB_MSD_HOST && !defined USB_RNDIS_HOST
        #if defined MEMORY_STICK_USES_SHARED_ENDPOINTS
                if (iEndPoint == 2)                                      // IN is on endpoint 2
        #else
                if (iEndPoint == 1)                                      // IN is on endpoint 1
        #endif
                {
                    if (iStall != 0) {
                        // Stall the endoint
                        //
                        iStall = 0;
                        fnSimulateUSB(iChannel, iAddress, iEndPoint, STALL_PID, (unsigned char *)1, 0); // inject a stall token on this IN endpoint
                        break;
                    }
                    if (ulFrameCount != 0) {
                        fnSimulateUSB(iChannel, iAddress, iEndPoint, 0, (unsigned char *)&ucDataTransport, usDataTransportLength);
                        ulFrameCount--;
                        if (ulFrameCount != 0) {                         // if the data occupies multiple frames
                            if (ucLastOpCode == UFI_READ_10) {           // we are presently reading sector content
                                if (iSectorOffset >= 512) {              // if a complete sector has been read
                                    iSectorOffset = 0;
                                    ulLBA++;                             // move to the next sector
                                    fnGetSimDiskData(ucMemoryStickSector, ucPresentLUN, ulLBA); // read the sector's content from the simulated memory stick
                                }
                                usDataTransportLength = 64;              // endpoint assumed to be 64 bytes in length
                                memcpy(ucDataTransport, (ucMemoryStickSector + iSectorOffset), usDataTransportLength); // prepare the frame to be returned
                                iSectorOffset += 64;
                            }
                            else {
                                _EXCEPTION("Add further op codes!!");
                            }

                        }
                    }
                    else {
                        fnSimulateUSB(iChannel, iAddress, iEndPoint, 0, (unsigned char *)&ucStatusTransport, sizeof(ucStatusTransport));
                    }
                }
    #endif
                break;
            }
        }
        iHostQueue = 0;
        break;
    case OUT_PID:
    #if defined USB_RNDIS_HOST
        if (usRNDIS_hostSetupDataLengthExpected[iEndPoint] != 0) {
            if ((usRNDIS_hostSetupDataReceived[iEndPoint] + usUSBLength) > usRNDIS_hostSetupDataLengthExpected[iEndPoint]) {
                _EXCEPTION("Too much data sent after setup report");
            }
            else {
                uMemcpy(&ucRNDIS_hostSetupData[iEndPoint][usRNDIS_hostSetupDataReceived[iEndPoint]], ptrUSBData, usUSBLength); // collect the data
                usRNDIS_hostSetupDataReceived[iEndPoint] += usUSBLength;
                if (usRNDIS_hostSetupDataReceived[iEndPoint] == usRNDIS_hostSetupDataLengthExpected[iEndPoint]) {
                    iRNDIS_ready[iEndPoint] = fnRNDIS_hostModel(&ucRNDIS_hostSetupData[iEndPoint][0], usRNDIS_hostSetupDataReceived[iEndPoint]);
                    iZeroData = 1;                                       // the control endpoint should terminate with a zero data status
                    iRNDIS_ready[2] = 1;                                 // when interrupt endpoint 2 is polled it should return a response available message
                    usRNDIS_hostSetupDataLengthExpected[iEndPoint] = usRNDIS_hostSetupDataReceived[iEndPoint] = 0;
                }
            }
        }
    #endif
    #if defined USB_MSD_HOST && !defined USB_RNDIS_HOST
        if (2 == iEndPoint) {                                            // we expect bulk out on this endpoint only
            fnMsdModel(ptrUSBData, usUSBLength);                         // allow MSD model to handle the data
        }
        else if ((0 == iEndPoint) && (usUSBLength == 0)) {               // expect only zero terminations on endpoint 0
        }
        else {
            _EXCEPTION("Unexpected");
        }
    #endif
        break;
    }
    return 0;
}

// Synchronise to correct data frame for all endpoints
//
extern void fnResetUSB_buffers(int iChannel)
{
    int i = 0;
    iData1Frame[iChannel][i++] = 1;                                      // reset endpoint 0 to DATA1
    while (i < NUMBER_OF_USB_ENDPOINTS) {                                // {42}
        iData1Frame[iChannel][i++] = 0;                                  // reset additional endpoint data frames to DATA0
    }
}
#endif

// Check whether data has been prepared for transmission
//
extern void fnCheckUSBOut(int iChannel, int iDevice, int iEndPoint)
{
    KINETIS_USBHS *ptrHSUSB;
    #if TICK_RESOLUTION >= 1000
    int iMaxUSB_ints = (TICK_RESOLUTION/1000);
    #else
    int iMaxUSB_ints = 1;
    #endif
    #if (USBS_AVAILABLE > 1)
    if (iChannel != 0) {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE2_ADD;
    }
    else {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE_ADD;
    }
    #else
    ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE_ADD;
    #endif
    if ((ptrHSUSB->USBCMD & USBHS_USBCMD_RS) != 0) {                     // if USB HS controller operating
    #if defined USB_HOST_SUPPORT
        if ((ptrHSUSB->USBMODE & USBHS_USBMODE_CM_HOST_FLAG) != 0) {     // if operating in host mode
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR;
            ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
            ptrHostQueueHeader += iEndPoint;
            if (((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_OVERLAY_TERMINATE) == 0) { // if valid
                USB_HS_HOST_TRANSFER_OVERLAY *ptr_qTD = (USB_HS_HOST_TRANSFER_OVERLAY *)((unsigned long)(ptrHostQueueHeader->ptrCurrent_qTD) & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
                unsigned short usDataLength = (unsigned short)((ptr_qTD->ulControl & HOST_OVERLAY_CONTROL_TOTAL_BYTES_MASK) >> HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT);
                unsigned short usMaxPacket = (unsigned short)((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) >> HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT);
                unsigned char *ptrUSBData = (unsigned char *)ptr_qTD->ulBufferPointerPage[0];
                do {                                                     // the USB HS controller automatically sends complete buffers as single frames and there is only a single interrupt on completion
                    if (usDataLength < usMaxPacket) {
                        usMaxPacket = usDataLength;
                    }
                    switch (ptr_qTD->ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) {
                    case HOST_OVERLAY_CONTROL_PID_CODE_SETUP:
                        fnLogUSB(iEndPoint, SETUP_PID, usDataLength, ptrUSBData, 0); // log the transmitted data
                        fnUSBHostModel(iChannel, ((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_ENDPOINT_MASK) >> 8), SETUP_PID, usDataLength, ptrUSBData, ptrHSUSB, 0); // let the host model handle the setup
                        ptr_qTD->ulControl |= HOST_OVERLAY_CONTROL_DT;   // set data toggle
                        ptr_qTD->ulBufferPointerPage[0] += usDataLength;
                        ptrHostQueueHeader->host_transfer_overlay.ptrNext_qTD = 0;
                        ptrHostQueueHeader->host_transfer_overlay.ptrAlternateNext_qTD = (unsigned char *)0x1e; // don't know what this signifies at the moment
                        ptrHostQueueHeader->host_transfer_overlay.ulControl = (ptr_qTD->ulControl & ~(HOST_OVERLAY_CONTROL_STATUS_ACTIVE));
                        ptrHostQueueHeader->host_transfer_overlay.ulBufferPointerPage[0] = ptr_qTD->ulBufferPointerPage[0];
                        ptrHostQueueHeader->host_transfer_overlay.ulBufferPointerPage[1] += 2; // don't know what this signifies at the moment
                        ptrHostQueueHeader->host_transfer_overlay.ulBufferPointerPage[2] += 0x17; // don't know what this signifies at the moment
                        break;
                    case HOST_OVERLAY_CONTROL_PID_CODE_IN:
                        if (fnUSBHostModel(iChannel, ((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_ENDPOINT_MASK) >> 8), IN_PID, 0, 0, ptrHSUSB, 0) < 0) { // let the host model prepare the data
                            return;                                      // ignore
                        }
                        ucHSTxBuffer[iChannel][iEndPoint] ^= ODD_BANK;   // toggle buffer
                        ptr_qTD->ulControl &= ~(HOST_OVERLAY_CONTROL_STATUS_ACTIVE);
                        return;
                    case HOST_OVERLAY_CONTROL_PID_CODE_OUT:
                        ptr_qTD->ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE;
                        ptrHostQueueHeader->host_transfer_overlay.ulControl = ptr_qTD->ulControl;
                        if (usDataLength != 0) {                         // if not zero data
                            fnUSBHostModel(iChannel, ((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_ENDPOINT_MASK) >> 8), OUT_PID, usDataLength, ptrUSBData, ptrHSUSB, 0); // let the host model handle the data
                        }
                        break;
                    }
                    usDataLength -= usMaxPacket;
                    ptrUSBData += usMaxPacket;
                } while (usDataLength != 0);
                ptr_qTD->ulControl &= ~(HOST_OVERLAY_CONTROL_STATUS_ACTIVE);
                fnSimulateUSB(iChannel, iDevice, iEndPoint, 0, 0, USB_IN_SUCCESS); // generate tx interrupt
            }
            return;
        }
    #endif
        KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR;
        ptrQueueHeader += ((2 * iEndPoint) + 1);                         // move to the transmitter queue header
        if (ptrQueueHeader->CurrentdTD_pointer != 0) {
            unsigned short usDataLength = (unsigned short)(ptrQueueHeader->CurrentdTD_pointer->ul_dtToken >> ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT);
            unsigned short usMaxPacket = (unsigned short)(ptrQueueHeader->ulCapabilities >> 16);
            unsigned char *ptrUSBData = (unsigned char *)ptrQueueHeader->CurrentdTD_pointer->ulBufferPointerPage[0];
            do {                                                         // the USB HS controller automatically sends complete buffers as single frames and there is only a single interrupt on completion
                if (usDataLength < usMaxPacket) {
                    usMaxPacket = usDataLength;
                }
                fnLogUSB(iEndPoint, 0, usMaxPacket, ptrUSBData, ((ucHSTxBuffer[iChannel][iEndPoint] & ODD_BANK) != 0));
                ucHSTxBuffer[iChannel][iEndPoint] ^= ODD_BANK;           // toggle buffer
                usDataLength -= usMaxPacket;
                ptrUSBData += usMaxPacket;
            } while (usDataLength != 0);
            fnSimulateUSB(iChannel, iDevice, iEndPoint, 0, 0, USB_IN_SUCCESS);  // generate tx interrupt
            ptrQueueHeader->CurrentdTD_pointer->ulBufferPointerPage[0] = (unsigned long)ptrUSBData;
        }
    }
}

// Request an endpoint buffer size
//
extern unsigned short fnGetEndpointInfo(int iEndPoint)
{
    KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)USBHS0_EPLISTADDR;
    #if HSUSB_CONTROLLERS > 1
    if ((USB1_FLAG & iEndPoint) != 0) {
        ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)USBHS1_EPLISTADDR;
    }
    #endif
    iEndPoint &= ~(USB1_FLAG);
    ptrQueueHeader += ((2 * iEndPoint) + 1);                             // move to the receiver queue header
    return (unsigned short)((ptrQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_MASK) >> ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT); // the endpoint reception buffer size
}
#endif

extern void fnSimulateModemChange(int iPort, unsigned long ulNewState, unsigned long ulOldState)
{
// Note that the modem status bits are according to the MS specifications as passed by GetCommModemStatus().
// To avoid including MS headers, the bits are defined here - it is not expected that they will ever change...
//
#define MS_CTS_ON  0x0010
#define MS_DSR_ON  0x0020
#define MS_RING_ON 0x0040
#define MS_RLSD_ON 0x0080                                                // carrier detect
#if defined SUPPORT_HW_FLOW
    unsigned long ulChange = (ulNewState ^ ulOldState);
#endif
}

#if LPUARTS_AVAILABLE > 0
static void fnGenericLPUARTBreakHandling(KINETIS_LPUART_CONTROL *ptrLPUART, int iPort, int LPUART_irq_ID, void (*irq_LPUART)(void), int iUseIntMux, int iMuxChannel, int iInterruptAssignment)
{
    ptrLPUART->LPUART_STAT |= (LPUART_STAT_LBKDIF | LPUART_STAT_RDRF);   // set the status flags
    if ((ptrLPUART->LPUART_BAUD & LPUART_BAUD_LBKDIE) != 0) {            // if the break interrupt is enabled
        ptrLPUART->LPUART_DATA = 0;                                      // a break character is seen as a data reception of 0
        if (iUseIntMux != 0) {                                           // if the interrupt is assigned via the interrupt multiplexer
    #if defined INTMUX0_AVAILABLE
            fnCallINTMUX(iMuxChannel, iInterruptAssignment, (unsigned char *)&irq_LPUART);
    #else
            _EXCEPTION("Interrupt multiplexer not available!");
    #endif
        }
        else if (fnGenInt(LPUART_irq_ID) != 0) {                         // if LPUART interrupt is not disabled
            irq_LPUART();                                                // call the interrupt handler
        }
    }
    else {
        unsigned char ucBreakChar = 0;
        fnSimulateSerialIn(iPort, (unsigned char *)&ucBreakChar, 1);     // a break chactater is seen as a data reception of 0
    }
}
#endif
#if UARTS_AVAILABLE > 0
static void fnGenericUARTBreakHandling(KINETIS_UART_CONTROL *ptrUART, int iPort, int UART_irq_ID, void (*irq_UART)(void), int iUseIntMux, int iMuxChannel, int iInterruptAssignment)
{
    if ((ptrUART->UART_BDH & UART_BDH_LBKDIE) != 0) {                    // if break detection is enabled
        ptrUART->UART_S1 |= (UART_S1_FE | UART_S1_RDRF);                 // set framing error flag
    }
    if (((ptrUART->UART_S1 & UART_S1_FE) != 0) && ((ptrUART->UART_BDH & UART_BDH_LBKDIE) != 0)) { // if break detection interrupt is enabled
        ptrUART->UART_D = 0;                                             // a break character is seen as a data reception of 0
        if (fnGenInt(UART_irq_ID) != 0) {                                // if UART interrupt is not disabled
            irq_UART();                                                  // call the interrupt handler
        }
    }
    else {
        unsigned char ucBreakChar = 0;
        fnSimulateSerialIn(iPort, (unsigned char *)&ucBreakChar, 1);     // a break chactater is seen as a data reception of 0
    }
}
#endif

// UART Break detection simulation
//
extern void fnSimulateBreak(int iPort)
{
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    switch (iPort) {
#if LPUARTS_AVAILABLE > 0
    #if defined LPUARTS_PARALLEL
        #define LPUART0_CH_NUMBER     UARTS_AVAILABLE
    #else
        #define LPUART0_CH_NUMBER     0
    #endif
    case LPUART0_CH_NUMBER:                                              // LPUART 0
    #if defined irq_LPUART0_RX_ID
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART0_BLOCK, iPort, irq_LPUART0_RX_ID, ptrVect->processor_interrupts.irq_LPUART0_RX, 0, 0, 0);
    #else
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART0_BLOCK, iPort, irq_LPUART0_ID, ptrVect->processor_interrupts.irq_LPUART0, 0, 0, 0);
    #endif
        break;
#endif
#if LPUARTS_AVAILABLE > 1
    #if defined LPUARTS_PARALLEL
        #define LPUART1_CH_NUMBER     (UARTS_AVAILABLE + 1)
    #else
        #define LPUART1_CH_NUMBER     1
    #endif
    case LPUART1_CH_NUMBER:                                              // LPUART 1
    #if defined irq_LPUART1_RX_ID
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART1_BLOCK, iPort, irq_LPUART1_RX_ID, ptrVect->processor_interrupts.irq_LPUART1_RX, 0, 0, 0);
    #else
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART1_BLOCK, iPort, irq_LPUART1_ID, ptrVect->processor_interrupts.irq_LPUART1, 0, 0, 0);
    #endif
        break;
#endif
#if LPUARTS_AVAILABLE > 2
    #if defined LPUARTS_PARALLEL
        #define LPUART2_CH_NUMBER     (UARTS_AVAILABLE + 2)
    #else
        #define LPUART2_CH_NUMBER     2
    #endif
    case LPUART2_CH_NUMBER:                                              // LPUART 2
    #if !defined irq_LPUART2_ID && defined INTMUX0_AVAILABLE
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART2_BLOCK, iPort, (irq_INTMUX0_0_ID + INTMUX_LPUART2), ptrVect->processor_interrupts.irq_LPUART2, 1, INTMUX_LPUART2, INTMUX0_PERIPHERAL_LPUART2);
    #elif defined irq_LPUART2_RX_ID
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART2_BLOCK, iPort, irq_LPUART2_RX_ID, ptrVect->processor_interrupts.irq_LPUART2_RX, 0, 0, 0);
    #else
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART2_BLOCK, iPort, irq_LPUART2_ID, ptrVect->processor_interrupts.irq_LPUART2, 0, 0, 0);
    #endif
        break;
#endif
#if LPUARTS_AVAILABLE > 3
    #if defined LPUARTS_PARALLEL
        #define LPUART3_CH_NUMBER     (UARTS_AVAILABLE + 3)
    #else
        #define LPUART3_CH_NUMBER     3
    #endif
    case LPUART3_CH_NUMBER:                                              // LPUART 3
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART3_BLOCK, iPort, irq_LPUART3_ID, ptrVect->processor_interrupts.irq_LPUART3, 0, 0, 0);
        break;
#endif
#if LPUARTS_AVAILABLE > 4
    #if defined LPUARTS_PARALLEL
        #define LPUART4_CH_NUMBER     (UARTS_AVAILABLE + 4)
    #else
        #define LPUART4_CH_NUMBER     4
    #endif
    case LPUART4_CH_NUMBER:                                              // LPUART 4
        fnGenericLPUARTBreakHandling((KINETIS_LPUART_CONTROL *)LPUART4_BLOCK, iPort, irq_LPUART4_ID, ptrVect->processor_interrupts.irq_LPUART4, 0, 0, 0);
        break;
#endif
#if UARTS_AVAILABLE > 0
    #if defined UART0_BLOCK
    case 0:
        #if defined irq_UART0_1_ID                                       // when UARTs 0 and 1 share an interrupt
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART0_BLOCK, iPort, irq_UART0_1_ID, ptrVect->processor_interrupts.irq_UART0_1, 0, 0, 0);
        #else
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART0_BLOCK, iPort, irq_UART0_ID, ptrVect->processor_interrupts.irq_UART0, 0, 0, 0);
        #endif
        break;
    #endif
    #if UARTS_AVAILABLE > 1
    case 1:
        #if defined irq_UART0_1_ID                                       // when UARTs 0 and 1 share an interrupt
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART1_BLOCK, iPort, irq_UART0_1_ID, ptrVect->processor_interrupts.irq_UART0_1, 0, 0, 0);
        #else
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART1_BLOCK, iPort, irq_UART1_ID, ptrVect->processor_interrupts.irq_UART1, 0, 0, 0);
        #endif
        break;
    #endif
    #if UARTS_AVAILABLE > 2
    case 2:
        #if defined irq_UART2_3_ID                                       // when UARTs 2 and 3 share an interrupt
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART2_BLOCK, iPort, irq_UART2_3_ID, ptrVect->processor_interrupts.irq_UART2_3, 0, 0, 0);
        #else
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART2_BLOCK, iPort, irq_UART2_ID, ptrVect->processor_interrupts.irq_UART2, 0, 0, 0);
        #endif
        break;
    #endif
    #if UARTS_AVAILABLE > 3
    case 3:
        #if defined irq_UART2_3_ID                                       // when UARTs 2 and 3 share an interrupt
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART3_BLOCK, iPort, irq_UART2_3_ID, ptrVect->processor_interrupts.irq_UART2_3, 0, 0, 0);
        #else
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART3_BLOCK, iPort, irq_UART3_ID, ptrVect->processor_interrupts.irq_UART3, 0, 0, 0);
        #endif
        break;
    #endif
    #if UARTS_AVAILABLE > 4
    case 4:
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART4_BLOCK, iPort, irq_UART4_ID, ptrVect->processor_interrupts.irq_UART4, 0, 0, 0);
        break;
    #endif
    #if UARTS_AVAILABLE > 5
    case 5:
        fnGenericUARTBreakHandling((KINETIS_UART_CONTROL *)UART5_BLOCK, iPort, irq_UART5_ID, ptrVect->processor_interrupts.irq_UART5, 0, 0, 0);
        break;
    #endif
#endif
    }
}

#if defined SERIAL_INTERFACE && defined SERIAL_SUPPORT_DMA
// Collect a pointer to referenced LPUART control block
//
static void *fnSelectLPUARTChannel(QUEUE_HANDLE Channel)
{
    static const void *uart_control_block[UARTS_AVAILABLE + LPUARTS_AVAILABLE] = {
    #if LPUARTS_AVAILABLE > 0
                                                              (void *)LPUART0_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 1
                                                              (void *)LPUART1_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 2
                                                              (void *)LPUART2_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 3
                                                              (void *)LPUART3_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 4
                                                              (void *)LPUART4_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 5
                                                              (void *)LPUART5_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 6
                                                              (void *)LPUART6_BLOCK,
    #endif
    #if LPUARTS_AVAILABLE > 7
                                                              (void *)LPUART7_BLOCK,
    #endif
    };
    return (void *)uart_control_block[Channel];
}

static void fnLPUART_TX_DMA(int iLPUART_ref, int iDMAChannel, int *iThroughput, unsigned long *ptr_ulNewActions, unsigned long ulChannel)
{
    KINETIS_UART_CONTROL *uart_reg = fnSelectLPUARTChannel(iLPUART_ref);
    if ((uart_reg->LPUART_BAUD & LPUART_BAUD_TDMAE) != 0) {              // if DMA operation is enabled
        if (*iThroughput != 0) {                                         // the number of characters in each tick period on this channel
            if (--(*iThroughput) == 0) {
                iMasks |= ulChannel;                                     // enough serial DMA transfers handled in this tick period
            }
            else {
                unsigned char ucTrigger;
                iDMA &= ~ulChannel;
                switch (iLPUART_ref) {
                case 0:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART1_TX;
                    break;
                case 1:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART2_TX;
                    break;
                case 2:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART3_TX;
                    break;
                case 3:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART4_TX;
                    break;
    #if LPUARTS_AVAILABLE > 4
                case 4:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART5_TX;
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 5
                case 5:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART6_TX;
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 6
                case 6:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART7_TX;
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 7
                case 7:
                    ucTrigger = DMAMUX0_CHCFG_SOURCE_LPUART8_TX;
                    break;
    #endif
                }
                if (fnSimulateDMA(iDMAChannel, ucTrigger) > 0) { // process the trigger
                    iDMA |= ulChannel;                       // further DMA triggers
                }
                else {
                    fnUART_Tx_int(iLPUART_ref);              // handle possible pending interrupt after DMA completion
                }
                switch (iLPUART_ref) {
                case 0:
                    fnLogTx0((unsigned char)uart_reg->LPUART_DATA);
                    break;
                case 1:
                    fnLogTx1((unsigned char)uart_reg->LPUART_DATA);
                    break;
                case 2:
                    fnLogTx2((unsigned char)uart_reg->LPUART_DATA);
                    break;
                case 3:
                    fnLogTx3((unsigned char)uart_reg->LPUART_DATA);
                    break;
    #if LPUARTS_AVAILABLE > 4
                case 4:
                    fnLogTx4((unsigned char)uart_reg->LPUART_DATA);
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 5
                case 5:
                    fnLogTx5((unsigned char)uart_reg->LPUART_DATA);
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 6
                case 6:
                    fnLogTx6((unsigned char)uart_reg->LPUART_DATA);
                    break;
    #endif
    #if LPUARTS_AVAILABLE > 7
                case 7:
                    fnLogTx7((unsigned char)uart_reg->LPUART_DATA);
                    break;
    #endif
                }
                *ptr_ulNewActions |= (SEND_COM_0 << iLPUART_ref);
            }
        }
    }
}
#endif


// Process simulated DMA
//
extern unsigned long fnSimDMA(char *argv[])
{
    unsigned long ulNewActions = 0;
    unsigned long ulChannel = 0x00000001;
    unsigned long iChannel = 0;
    int _iDMA = iDMA;
    while (_iDMA != 0) {                                                 // while DMA operations to be performed
        if ((_iDMA & ulChannel) != 0) {                                  // DMA request on this channel
            _iDMA &= ~ulChannel;
            switch (iChannel) {
    #if defined SERIAL_INTERFACE && defined SERIAL_SUPPORT_DMA
            case DMA_UART0_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART1
                fnLPUART_TX_DMA(0, iChannel, (int *)argv[THROUGHPUT_UART0], &ulNewActions, ulChannel);
                break;
        #if (LPUARTS_AVAILABLE > 1)
            case DMA_UART1_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 2
                fnLPUART_TX_DMA(1, iChannel, (int *)argv[THROUGHPUT_UART1], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 2)
            case DMA_UART2_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 3
                fnLPUART_TX_DMA(2, iChannel, (int *)argv[THROUGHPUT_UART2], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 3)
            case DMA_UART3_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 4
                fnLPUART_TX_DMA(3, iChannel, (int *)argv[THROUGHPUT_UART3], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 4)
            case DMA_UART4_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 5
                fnLPUART_TX_DMA(4, iChannel, (int *)argv[THROUGHPUT_UART4], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 5)
            case DMA_UART5_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 6
                fnLPUART_TX_DMA(5, iChannel, (int *)argv[THROUGHPUT_UART5], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 6)
            case DMA_UART6_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 7
                fnLPUART_TX_DMA(6, iChannel, (int *)argv[THROUGHPUT_UART6], &ulNewActions, ulChannel);
                break;
        #endif
        #if (LPUARTS_AVAILABLE > 7)
            case DMA_UART7_TX_CHANNEL:                                   // handle UART DMA transmission on LPUART 8
                fnLPUART_TX_DMA(7, iChannel, (int *)argv[THROUGHPUT_UART7], &ulNewActions, ulChannel);
                break;
        #endif
    #endif
            default:                                                     // not a channel used by LPUART/UART
                iDMA &= ~ulChannel;
                if (fnSimulateDMA(iChannel, 0) > 1) {                   // process the trigger
                    iDMA |= ulChannel;                                  // further DMA triggers
                }
                break;
            }
        }
        ulChannel <<= 1;
        iChannel++;
    }
    return ulNewActions;
}


extern void fnSimulateLinkUp(void)
{
#if defined ETH_INTERFACE
    #if defined PHY_INTERRUPT
    unsigned long ulBit = PHY_INTERRUPT;
    unsigned char ucPortBit = 0;
    while ((ulBit & 0x80000000) == 0) {
        ucPortBit++;
        ulBit <<= 1;
    }
    MMFR = PHY_LINK_UP_INT;
    fnSimulateInputChange(PHY_INTERRUPT_PORT, ucPortBit, CLEAR_INPUT);   // clear level sensitive interrupt input
    #endif
    fnUpdateIPConfig();                                                  // update display in simulator
#elif defined USB_CDC_RNDIS || defined USE_PPP
    fnUpdateIPConfig();                                                  // update display in simulator
#endif
}

#if (defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET)
extern void fec_txf_isr(void)
{
    EIR |= (TXF | TXB);                                                  // set frame and buffer interrupt events
    if (EIMR & TXF) {                                                    // if interrupt is enabled
        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
      //ptrVect->processor_interrupts.irq_ETH_TX();                      // call the interrupt handler
        ptrVect->processor_interrupts.irq_ETH();                         // call the interrupt handler
	}
}
#endif

#if (defined SUPPORT_RTC && !defined KINETIS_WITHOUT_RTC) || defined SUPPORT_SW_RTC
#define NTP_TO_1970_TIME 2208988800u
#define LEAP_YEAR(year) ((year%4)==0)                                    // valid from 1970 to 2038
static const unsigned char monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
// Synchronise the internal RTC to the PC time when simulating
//
extern void fnInitInternalRTC(char *argv[])
{
    unsigned short *ptrShort = (unsigned short *)*argv;
    unsigned short RTC_YEAR, RTC_MONTH, RTC_DOW, RTC_DOM, RTC_HOUR, RTC_MIN, RTC_SEC;
    unsigned long ulComputerTime;
    unsigned long ulLeapYears = 1970;
    RTC_YEAR = *ptrShort++;
    RTC_MONTH = *ptrShort++;
    RTC_DOW = *ptrShort++;
    RTC_DOM = *ptrShort++;
    RTC_HOUR = *ptrShort++;
    RTC_MIN = *ptrShort++;
    RTC_SEC = *ptrShort++;

    ulComputerTime = ((RTC_YEAR - 1970) * 365);                          // years since reference time, represented in days without respecting leap years
    while (ulLeapYears <= RTC_YEAR) {                                    //
        if (LEAP_YEAR(ulLeapYears) != 0) {                               // count leap years
            if (ulLeapYears == RTC_YEAR) {                               // presently in a leap year
                if ((RTC_MONTH > 2) && (RTC_DOM > 28)) {                 // past February 28 so count extra leap day in this year
                    ulComputerTime++;
                }
            }
            else {
                ulComputerTime++;                                        // count extra days in passed leap years
            }
        }
        ulLeapYears++;
    }
    while (--RTC_MONTH != 0) {
        ulComputerTime += monthDays[RTC_MONTH - 1];                      // add past days of previous months of this year
    }
    ulComputerTime += (RTC_DOM - 1);                                     // add past number of days in present month
    ulComputerTime *= 24;                                                // convert days to hours
    ulComputerTime += RTC_HOUR;                                          // add hours passed in present day
    ulComputerTime *= 60;                                                // convert hours to minutes
    ulComputerTime += RTC_MIN;                                           // add minutes in present hour
    ulComputerTime *= 60;                                                // convert minutes to seconds
    ulComputerTime += RTC_SEC;                                           // add seconds in present minute
    #if !defined SUPPORT_SW_RTC
    SNVS_HPRTCMR = (ulComputerTime >> 17);                               // set the initial seconds count value (since 1970)
    SNVS_HPRTCLR = (ulComputerTime << 15);
    #else
    *RTC_SECONDS_LOCATION = ulKinetisTime;                               // set time information to non-initialised ram
    *RTC_PRESCALER_LOCATION = 0;
    *RTC_ALARM_LOCATION = 0;
    *RTC_VALID_LOCATION = RTC_VALID_PATTERN;
    #endif
}
#endif

#if defined SUPPORT_ADC_
static void fnTriggerADC(int iADC, int iHW_trigger)
{
    switch (iADC) {
    case 0:                                                              // ADC0
        if (IS_POWERED_UP(6, ADC0) && ((ADC0_SC1A & ADC_SC1A_ADCH_OFF) != ADC_SC1A_ADCH_OFF)) { // ADC0 powered up and operating
            if ((iHW_trigger != 0) || ((ADC0_SC2 & ADC_SC2_ADTRG_HW) == 0)) { // hardware or software trigger
                fnSimADC(0);                                             // perform ADC conversion
                if ((ADC0_SC1A & ADC_SC1A_COCO) != 0) {                  // if conversion has completed
    #if !defined DEVICE_WITHOUT_DMA
                    fnHandleDMA_triggers(DMAMUX0_CHCFG_SOURCE_ADC0, 0);  // handle DMA triggered on ADC0 conversion
    #endif
                    if ((ADC0_SC1A & ADC_SC1A_AIEN) != 0) {              // end of conversion interrupt enabled
    #if defined irq_ADCA_ID
                        if (fnGenInt(irq_ADCA_ID) != 0) {                // if ADCA interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADCA();    // call the interrupt handler
                        }
    #else
                        if (fnGenInt(irq_ADC0_ID) != 0) {                // if ADC0 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADC0();    // call the interrupt handler
                        }
    #endif
                    }
                }
            }
        }
        break;
    #if ADC_CONTROLLERS > 1
    case 1:                                                              // ADC1
        #if defined KINETIS_K22_SF7
        if (IS_POWERED_UP(6, ADC1) && ((ADC1_SC1A & ADC_SC1A_ADCH_OFF) != ADC_SC1A_ADCH_OFF))
        #else
        if (IS_POWERED_UP(3, ADC1) && ((ADC1_SC1A & ADC_SC1A_ADCH_OFF) != ADC_SC1A_ADCH_OFF))
        #endif
        {                                                                // ADC1 powered up and operating
            if ((iHW_trigger != 0) || ((ADC1_SC2 & ADC_SC2_ADTRG_HW) == 0)) { // hardware or software trigger mode
                fnSimADC(1);                                             // perform ADC conversion
                if ((ADC1_SC1A & ADC_SC1A_COCO) != 0) {                  // if conversion has completed
                    fnHandleDMA_triggers(DMAMUX0_CHCFG_SOURCE_ADC1, 0);  // handle DMA triggered on ADC1 conversion
                    if ((ADC1_SC1A & ADC_SC1A_AIEN) != 0) {              // end of conversion interrupt enabled
        #if defined irq_ADCB_ID
                        if (fnGenInt(irq_ADCB_ID) != 0) {                // if ADCB interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADCB();    // call the interrupt handler
                        }
        #else
                        if (fnGenInt(irq_ADC1_ID) != 0) {                // if ADC1 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADC1();    // call the interrupt handler
                        }
        #endif
                    }
                }
            }
        }
        break;
    #endif
    #if ADC_CONTROLLERS > 2
    case 2:                                                              // ADC2
        if ((IS_POWERED_UP(6, ADC2)) && ((ADC2_SC1A & ADC_SC1A_ADCH_OFF) != ADC_SC1A_ADCH_OFF)) { // ADC2 powered up and operating
            if ((iHW_trigger != 0) || ((ADC2_SC2 & ADC_SC2_ADTRG_HW) == 0)) { // hardware or software trigger mode
                fnSimADC(2);                                             // perform ADC conversion
                if ((ADC2_SC1A & ADC_SC1A_COCO) != 0) {                  // {40} if conversion has completed
        #if !defined KINETIS_KE18                                        // to be done
                    fnHandleDMA_triggers(DMAMUX1_CHCFG_SOURCE_ADC2, 1);  // handle DMA triggered on ADC2 conversion
                    if ((ADC2_SC1A & ADC_SC1A_AIEN) != 0) {              // end of conversion interrupt enabled
                        if (fnGenInt(irq_ADC2_ID) != 0) {                // if ADC2 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADC2();    // call the interrupt handler
                        }
                    }
        #endif
                }
            }
        }
        break;
    #endif
    #if ADC_CONTROLLERS > 3
    case 3:                                                              // ADC3
        if ((IS_POWERED_UP(3, ADC3)) && ((ADC3_SC1A & ADC_SC1A_ADCH_OFF) != ADC_SC1A_ADCH_OFF)) { // ADC3 powered up and operating
            if ((iHW_trigger != 0) || ((ADC3_SC2 & ADC_SC2_ADTRG_HW) == 0)) { // hardware or software trigger mode
                fnSimADC(3);                                             // perform ADC conversion
                if ((ADC3_SC1A & ADC_SC1A_COCO) != 0) {                  // if conversion has completed
                    fnHandleDMA_triggers(DMAMUX1_CHCFG_SOURCE_ADC3, 1);  // handle DMA triggered on ADC3 conversion
                    if ((ADC3_SC1A & ADC_SC1A_AIEN) != 0) {              // end of conversion interrupt enabled
                        if (fnGenInt(irq_ADC3_ID) != 0) {                // if ADC3 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_ADC3();    // call the interrupt handler
                        }
                    }
                }
            }
        }
        break;
    #endif
    }
}
#endif


// We can simulate timers during a tick here if required
//
extern int fnSimTimers(void)
{
#if defined SERIAL_INTERFACE
    int iUART = 0;
#endif
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;

#if defined CAN_INTERFACE && defined SIM_KOMODO
    fnSimCAN(0, 0, CAN_SIM_CHECK_RX);                                    // poll the CAN interface at the tick rate
#endif

    if ((APPLICATION_INT_RESET_CTR_REG & SYSRESETREQ) != 0) {
        return RESET_SIM_CARD;                                           // commanded reset
    }
    // Watchdog
    //
    if ((WDOG3_CS & WDOG_CS_EN) != 0) {                                  // if the watchdog is enabled
    #if TICK_RESOLUTION >= 1000
        unsigned long ulCounter = (TICK_RESOLUTION / 32768);             // assume 32kHz LPO clock
    #else
        unsigned long ulCounter = 1;                                     // assume 32kHz LPO clock
    #endif
        unsigned long ulWdogCnt = WDOG3_CNT;                             // present watchdog count value
        unsigned long ulWdogTimeout = WDOG3_TOVAL;                       // timeout value
        if ((WDOG3_CS & WDOG_CS_PRES_256) != 0) {                        // if the fixed 256 prescaler is enabled
            ulCounter /= 256;
        }
        ulWdogCnt += ulCounter;                                          // next value
        if (ulWdogCnt >= ulWdogTimeout) {
            if ((WDOG3_CS & WDOG_CS_INT) != 0) {                         // if pre-reset interrupt is enabled
                if (fnGenInt(irq_RTWDOG_ID) != 0) {                      // if WDOG3 interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_RTWDOG();          // call the interrupt handler
                }
            }
            return RESET_CARD_WATCHDOG;                                  // watchdog reset
        }
        WDOG3_CNT = (unsigned short)ulWdogCnt;                           // new watchdog count value
    }
    // SysTick
    //
    if ((SYSTICK_CSR & SYSTICK_ENABLE) != 0) {                           // SysTick is enabled
        unsigned long ulTickCount = 0;
        if ((SYSTICK_CSR & SYSTICK_CORE_CLOCK) != 0) {
            ulTickCount = (unsigned long)((unsigned long long)((unsigned long long)TICK_RESOLUTION * (unsigned long long)SYSTEM_CLOCK)/1000000); // count per tick period from internal clock
        }
        if (ulTickCount < SYSTICK_CURRENT) {
            SYSTICK_CURRENT -= ulTickCount;
        }
        else {
            SYSTICK_CURRENT = SYSTICK_RELOAD;
            if ((SYSTICK_CSR & SYSTICK_TICKINT) != 0) {                  // if interrupt enabled
                INT_CONT_STATE_REG |= PENDSTSET;                         // set the systick as pending
                if ((iMX.CORTEX_M7_REGS.ulPRIMASK & INTERRUPT_MASKED) == 0) { // if global interrupts have been enabled, call interrupt handler
#if defined RUN_IN_FREE_RTOS
                    fnExecutePendingInterrupts(0);
#else
                    ptrVect->ptrSysTick();                               // call the systick interrupt service routine
#endif
                }
            }
        }
    }
    // PIT
    //
#if !defined KINETIS_WITHOUT_PIT
    #if defined LPITS_AVAILABLE
    if (((PCC_LPIT0 & PCC_CGC) != 0) && ((LPIT0_MCR & (LPIT_MCR_M_CEN | LPIT_MCR_SW_RST)) == LPIT_MCR_M_CEN)) { // if the LPIT is enabled and not in reset state
        unsigned long ulCount;
        switch (PCC_LPIT0 & PCC_PCS_MASK) {
        case PCC_PCS_SCGFIRCLK:
            ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)FIRC_CLK) / 1000000); // count in a tick period
            break;
        case PCC_PCS_OSCCLK:
        case PCC_PCS_SCGIRCLK:
        case PCC_PCS_SCGPCLK:
            _EXCEPTION("To do!");
            break;
        case PCC_PCS_CLOCK_OFF:
        default:
            _EXCEPTION("No LPIT clock!");
            break;

        }
        if ((LPIT0_TCTRL0 & LPIT_TCTRL_T_EN) != 0) {                     // if channel 0 is enabled
            if (LPIT0_CVAL0 <= ulCount) {
                ulCount -= LPIT0_CVAL0;
                LPIT0_CVAL0 = LPIT0_TVAL0;                               // reload
                if (ulCount < LPIT0_TVAL0) {
                    LPIT0_CVAL0 -= ulCount;
                }
                LPIT0_MSR |= LPIT_MSR_TIF0;                              // flag that a reload occurred
                fnHandleDMA_triggers(DMAMUX0_DMA0_CHCFG_SOURCE_PIT0, 0); // handle DMA triggered on LPIT0
                if ((LPIT0_MIER & LPIT_MIER_TIE0) != 0) {                // if PIT interrupt is enabled
        #if defined irq_LPIT0_CH0_ID
                    if (fnGenInt(irq_LPIT0_CH0_ID) != 0) {               // if PIT channel interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0_CH0();   // call the interrupt handler
                    }
        #else
                    if (fnGenInt(irq_LPIT0_ID) != 0) {                   // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0();       // call the shared interrupt handler
                    }
        #endif
                }
            }
            else {
                LPIT0_CVAL0 -= ulCount;
            }
        }
        if ((LPIT0_TCTRL1 & LPIT_TCTRL_T_EN) != 0) {                     // if channel 1 is enabled
            if (LPIT0_CVAL1 <= ulCount) {
                ulCount -= LPIT0_CVAL1;
                LPIT0_CVAL1 = LPIT0_TVAL1;                               // reload
                if (ulCount < LPIT0_TVAL1) {
                    LPIT0_CVAL1 -= ulCount;
                }
                LPIT0_MSR |= LPIT_MSR_TIF1;                              // flag that a reload occurred
        #if !defined KINETIS_KL82                                        // not yet supported
                fnHandleDMA_triggers(DMAMUX0_DMA0_CHCFG_SOURCE_PIT1, 0); // handle DMA triggered on LPIT1
        #endif
                if ((LPIT0_MIER & LPIT_MIER_TIE1) != 0) {                // if PIT interrupt is enabled
        #if defined irq_LPIT0_CH1_ID
                    if (fnGenInt(irq_LPIT0_CH1_ID) != 0) {               // if PIT channel interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0_CH1();   // call the interrupt handler
                    }
        #else
                    if (fnGenInt(irq_LPIT0_ID) != 0) {                   // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0();       // call the shared interrupt handler
                    }
        #endif
                }
            }
            else {
                LPIT0_CVAL1 -= ulCount;
            }
        }
        if ((LPIT0_TCTRL2 & LPIT_TCTRL_T_EN) != 0) {                     // if channel 2 is enabled
            if (LPIT0_CVAL2 <= ulCount) {
                ulCount -= LPIT0_CVAL2;
                LPIT0_CVAL2 = LPIT0_TVAL2;                               // reload
                if (ulCount < LPIT0_TVAL2) {
                    LPIT0_CVAL2 -= ulCount;
                }
                LPIT0_MSR |= LPIT_MSR_TIF2;                              // flag that a reload occurred
        #if !defined KINETIS_KL82                                        // not yet supported
                fnHandleDMA_triggers(DMAMUX0_DMA0_CHCFG_SOURCE_PIT2, 0); // handle DMA triggered on LPIT2
        #endif
                if ((LPIT0_MIER & LPIT_MIER_TIE2) != 0) {                // if PIT interrupt is enabled
        #if defined irq_LPIT0_CH2_ID
                    if (fnGenInt(irq_LPIT0_CH2_ID) != 0) {               // if PIT channel interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0_CH2();   // call the interrupt handler
                    }
        #else
                    if (fnGenInt(irq_LPIT0_ID) != 0) {                   // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0();       // call the shared interrupt handler
                    }
        #endif
                }
            }
            else {
                LPIT0_CVAL2 -= ulCount;
            }
        }
        if ((LPIT0_TCTRL3 & LPIT_TCTRL_T_EN) != 0) {                     // if channel 3 is enabled
            if (LPIT0_CVAL3 <= ulCount) {
                ulCount -= LPIT0_CVAL3;
                LPIT0_CVAL3 = LPIT0_TVAL3;                               // reload
                if (ulCount < LPIT0_TVAL3) {
                    LPIT0_CVAL3 -= ulCount;
                }
                LPIT0_MSR |= LPIT_MSR_TIF3;                              // flag that a reload occurred
        #if !defined KINETIS_KL82                                        // not yet supported
                fnHandleDMA_triggers(DMAMUX0_DMA0_CHCFG_SOURCE_PIT3, 0); // handle DMA triggered on LPIT3
        #endif
                if ((LPIT0_MIER & LPIT_MIER_TIE3) != 0) {                // if PIT interrupt is enabled
        #if defined irq_LPIT0_CH3_ID
                    if (fnGenInt(irq_LPIT0_CH3_ID) != 0) {               // if PIT channel interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0_CH3();   // call the interrupt handler
                    }
        #else
                    if (fnGenInt(irq_LPIT0_ID) != 0) {                   // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPIT0();       // call the shared interrupt handler
                    }
        #endif
                }
            }
            else {
                LPIT0_CVAL3 -= ulCount;
            }
        }
    }
    #else
    if ((PIT_MCR & PIT_MCR_MDIS) == 0) {                                 // if PIT module is not disabled
        if ((PIT_TCTRL0 & PIT_TCTRL_TEN) != 0) {                         // if PIT0 is enabled
            unsigned long ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)PIT_CLOCK)/1000000); // count in a tick period
            if (PIT_CVAL0 <= ulCount) {
                ulCount -= PIT_CVAL0;
                if (ulCount >= PIT_LDVAL0) {
                    ulCount = 0;
                }
                PIT_CVAL0 = PIT_LDVAL0;                                  // reload
                PIT_CVAL0 -= ulCount;
                PIT_TFLG0 = PIT_TFLG_TIF;                                // flag that a reload occurred
                if ((PIT_TCTRL0 & PIT_TCTRL_TIE) != 0) {                 // if PIT interrupt is enabled
                    if (fnGenInt(irq_PIT_ID) != 0) {                     // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_PIT();         // call the shared interrupt handler
                    }
                }
            }
            else {
                PIT_CVAL0 -= ulCount;
            }
        }
        if ((PIT_TCTRL1 & PIT_TCTRL_TEN) != 0) {                         // if PIT1 is enabled
            unsigned long ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)PIT_CLOCK)/1000000); // count in a tick period
            if (PIT_CVAL1 <= ulCount) {
                ulCount -= PIT_CVAL1;
                if (ulCount >= PIT_LDVAL1) {
                    ulCount = 0;
                }
                PIT_CVAL1 = PIT_LDVAL1;                                  // reload
                PIT_CVAL1 -= ulCount;
                PIT_TFLG1 = PIT_TFLG_TIF;                                // flag that a reload occurred
                if ((PIT_TCTRL1 & PIT_TCTRL_TIE) != 0) {                 // if PIT interrupt is enabled
                    if (fnGenInt(irq_PIT_ID) != 0) {                     // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_PIT();         // call the shared interrupt handler
                    }
                }
            }
            else {
                PIT_CVAL1 -= ulCount;
            }
        }
        if ((PIT_TCTRL2 & PIT_TCTRL_TEN) != 0) {                         // if PIT2 is enabled
            unsigned long ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)PIT_CLOCK) / 1000000); // count in a tick period
            if (PIT_CVAL2 <= ulCount) {
                ulCount -= PIT_CVAL2;
                if (ulCount >= PIT_LDVAL2) {
                    ulCount = 0;
                }
                PIT_CVAL2 = PIT_LDVAL2;                                  // reload
                PIT_CVAL2 -= ulCount;
                PIT_TFLG2 = PIT_TFLG_TIF;                                // flag that a reload occurred
                if ((PIT_TCTRL2 & PIT_TCTRL_TIE) != 0) {                 // if PIT interrupt is enabled
                    if (fnGenInt(irq_PIT_ID) != 0) {                     // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_PIT();         // call the shared interrupt handler
                    }
                }
            }
            else {
                PIT_CVAL2 -= ulCount;
            }
        }
        if ((PIT_TCTRL3 & PIT_TCTRL_TEN) != 0) {                         // if PIT3 is enabled
            unsigned long ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)PIT_CLOCK) / 1000000); // count in a tick period
            if (PIT_CVAL3 <= ulCount) {
                ulCount -= PIT_CVAL3;
                if (ulCount >= PIT_LDVAL3) {
                    ulCount = 0;
                }
                PIT_CVAL3 = PIT_LDVAL3;                                  // reload
                PIT_CVAL3 -= ulCount;
                PIT_TFLG3 = PIT_TFLG_TIF;                                // flag that a reload occurred
                if ((PIT_TCTRL3 & PIT_TCTRL_TIE) != 0) {                 // if PIT interrupt is enabled
                    if (fnGenInt(irq_PIT_ID) != 0) {                     // if general PIT interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_PIT();         // call the shared interrupt handler
                    }
                }
            }
            else {
                PIT_CVAL3 -= ulCount;
            }
        }
    }
    #endif
#endif
#if defined USB_HOST_SUPPORT
    // HS USB general purpose timers
    //
    if (IS_POWERED_UP(6, USBOH3_CLOCK)) {
        if ((USBHS0_GPTIMER0CTL & USBHS_GPTIMERCTL_RUN) != 0) {          // if USB0 timer 0 is running
            unsigned long ulPresentCount = (USBHS0_GPTIMER0CTL & USBHS_GPTIMERCTL_GPTCNT_MASK);
            if (ulPresentCount != 0) {                                   // if it hasn't yet counted down to zero
                unsigned long ulCounter = TICK_RESOLUTION;               // us in a tick period
                if (ulCounter >= ulPresentCount) {
                    if ((USBHS0_GPTIMER0CTL & USBHS_GPTIMERCTL_MODE_REPEAT) != 0) { // if in repeat mode
                        ulPresentCount = USBHS0_GPTIMER0LD;              // reload the counter value again
                    }
                    else {
                        ulPresentCount = 0;
                    }
                    USBHS0_GPTIMER0CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS0_GPTIMER0CTL |= (ulPresentCount);
                    USBHS0_USBSTS |= USBHS_USBINTR_TIE0;                 // set timeout flag
                    if ((USBHS0_USBINTR & USBHS_USBINTR_TIE0) != 0) {    // if interrupt is enabled on counter timeout
                        if (fnGenInt(irq_USB_HS_ID) != 0) {              // if HS USB interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_USB_OTG0();// call the interrupt handler
                        }
                    }
                }
                else {
                    ulPresentCount -= ulCounter;
                    USBHS0_GPTIMER0CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS0_GPTIMER0CTL |= (ulPresentCount);
                }
            }
        }
        if ((USBHS0_GPTIMER1CTL & USBHS_GPTIMERCTL_RUN) != 0) {          // if USB0 timer 1 is running
            unsigned long ulPresentCount = (USBHS0_GPTIMER1CTL & USBHS_GPTIMERCTL_GPTCNT_MASK);
            if (ulPresentCount != 0) {                                   // if it hasn't yet counted down to zero
                unsigned long ulCounter = TICK_RESOLUTION;               // us in a tick period
                if (ulCounter >= ulPresentCount) {
                    if ((USBHS0_GPTIMER1CTL & USBHS_GPTIMERCTL_MODE_REPEAT) != 0) { // if in repeat mode
                        ulPresentCount = USBHS0_GPTIMER0LD;               // reload the counter value again
                    }
                    else {
                        ulPresentCount = 0;
                    }
                    USBHS0_GPTIMER1CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS0_GPTIMER1CTL |= (ulPresentCount);
                    USBHS0_USBSTS |= USBHS_USBINTR_TIE1;                 // set timeout flag
                    if ((USBHS0_USBINTR & USBHS_USBINTR_TIE1) != 0) {    // if interrupt is enabled on counter timeout
                        if (fnGenInt(irq_USB_HS_ID) != 0) {              // if HS USB interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_USB_OTG0();// call the interrupt handler
                        }
                    }
                }
                else {
                    ulPresentCount -= ulCounter;
                    USBHS0_GPTIMER1CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS0_GPTIMER1CTL |= (ulPresentCount);
                }
            }
        }
        // HS USB 0 periodic scheduler
        //
        if ((USBHS0_USBCMD & USBHS_USBCMD_PSE) != 0) {                   // if the periodic schedule is enabled
            int iPeriodListLength = 0;
            int iThisPeriodListLength;
            unsigned long ulMicroFrames = (TICK_RESOLUTION / 125);       // microframes in a tick period
            if (USBHS0_PERIODICLISTBASE != 0) {
                iPeriodListLength = fnGetPeriodicListLength(USBHS0_USBCMD);
                iThisPeriodListLength = iPeriodListLength;
                while (ulMicroFrames-- != 0) {                           // for each microframe in the tick period
                    if ((USBHS0_FRINDEX & 0x7) == 0) {                   // if a ms increment has taken place
                        KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = fnGetPeriodicEntry(USBHS0_FRINDEX, USBHS0_PERIODICLISTBASE, iPeriodListLength);
                        if (ptrHostHead != 0) {
                            if ((ptrHostHead->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) != 0) {
                                if ((ptrHostHead->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) == HOST_OVERLAY_CONTROL_PID_CODE_IN) {
                                    unsigned char ucEndpoint = ((ptrHostHead->ulHostCharacteristics & HOST_CHARACTERISTICS_ENDPOINT_MASK) >> HOST_CHARACTERISTICS_ENDPOINT_SHIFT);
                                    fnUSBHostModel(0, ucEndpoint, IN_PID, 0, 0, (KINETIS_USBHS *)USBHS_BASE_ADD, 1); // allow the host model to inject frames
                                }
                            }
                        }
                        if (--iThisPeriodListLength == 0) {
                            USBHS0_FRINDEX = ((USBHS0_FRINDEX + 1) & 0x00003fff);
                            break;
                        }
                    }
                    USBHS0_FRINDEX = ((USBHS0_FRINDEX + 1) & 0x00003fff);
                }
            }
        }
    #if HSUSB_CONTROLLERS > 1
        if ((USBHS1_GPTIMER0CTL & USBHS_GPTIMERCTL_RUN) != 0) {          // if USB1 timer 0 is running
            unsigned long ulPresentCount = (USBHS1_GPTIMER0CTL & USBHS_GPTIMERCTL_GPTCNT_MASK);
            if (ulPresentCount != 0) {                                   // if it hasn't yet counted down to zero
                unsigned long ulCounter = TICK_RESOLUTION;               // us in a tick period
                if (ulCounter >= ulPresentCount) {
                    if ((USBHS1_GPTIMER0CTL & USBHS_GPTIMERCTL_MODE_REPEAT) != 0) { // if in repeat mode
                        ulPresentCount = USBHS1_GPTIMER0LD;              // reload the counter value again
                    }
                    else {
                        ulPresentCount = 0;
                    }
                    USBHS1_GPTIMER0CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS1_GPTIMER0CTL |= (ulPresentCount);
                    USBHS1_USBSTS |= USBHS_USBINTR_TIE0;                 // set timeout flag
                    if ((USBHS1_USBINTR & USBHS_USBINTR_TIE0) != 0) {    // if interrupt is enabled on counter timeout
                        if (fnGenInt(irq_USB_OTG1_ID) != 0) {            // if HS USB interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_USB_OTG1();// call the interrupt handler
                        }
                    }
                }
                else {
                    ulPresentCount -= ulCounter;
                    USBHS1_GPTIMER0CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS1_GPTIMER0CTL |= (ulPresentCount);
                }
            }
        }
        if ((USBHS1_GPTIMER1CTL & USBHS_GPTIMERCTL_RUN) != 0) {          // if USB1 timer 1 is running
            unsigned long ulPresentCount = (USBHS1_GPTIMER1CTL & USBHS_GPTIMERCTL_GPTCNT_MASK);
            if (ulPresentCount != 0) {                                   // if it hasn't yet counted down to zero
                unsigned long ulCounter = TICK_RESOLUTION;               // us in a tick period
                if (ulCounter >= ulPresentCount) {
                    if ((USBHS1_GPTIMER1CTL & USBHS_GPTIMERCTL_MODE_REPEAT) != 0) { // if in repeat mode
                        ulPresentCount = USBHS1_GPTIMER0LD;               // reload the counter value again
                    }
                    else {
                        ulPresentCount = 0;
                    }
                    USBHS1_GPTIMER1CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS1_GPTIMER1CTL |= (ulPresentCount);
                    USBHS1_USBSTS |= USBHS_USBINTR_TIE1;                 // set timeout flag
                    if ((USBHS1_USBINTR & USBHS_USBINTR_TIE1) != 0) {    // if interrupt is enabled on counter timeout
                        if (fnGenInt(irq_USB_OTG1_ID) != 0) {            // if HS USB interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_USB_OTG1();// call the interrupt handler
                        }
                    }
                }
                else {
                    ulPresentCount -= ulCounter;
                    USBHS1_GPTIMER1CTL &= ~(USBHS_GPTIMERCTL_GPTCNT_MASK);
                    USBHS1_GPTIMER1CTL |= (ulPresentCount);
                }
            }
        }
        // HS USB 1 periodic scheduler
        //
        if ((USBHS1_USBCMD & USBHS_USBCMD_PSE) != 0) {                   // if the periodic schedule is enabled
            int iPeriodListLength = 0;
            int iThisPeriodListLength;
            unsigned long ulMicroFrames = (TICK_RESOLUTION / 125);       // microframes in a tick period
            if (USBHS1_PERIODICLISTBASE != 0) {
                iPeriodListLength = fnGetPeriodicListLength(USBHS1_USBCMD);
                iThisPeriodListLength = iPeriodListLength;
                while (ulMicroFrames-- != 0) {                           // for each microframe in the tick period
                    if ((USBHS1_FRINDEX & 0x7) == 0) {                   // if a ms increment has taken place
                        KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = fnGetPeriodicEntry(USBHS1_FRINDEX, USBHS1_PERIODICLISTBASE, iPeriodListLength);
                        if (ptrHostHead != 0) {
                            if ((ptrHostHead->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) != 0) {
                                if ((ptrHostHead->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) == HOST_OVERLAY_CONTROL_PID_CODE_IN) {
                                    unsigned char ucEndpoint = ((ptrHostHead->ulHostCharacteristics & HOST_CHARACTERISTICS_ENDPOINT_MASK) >> HOST_CHARACTERISTICS_ENDPOINT_SHIFT);
                                    fnUSBHostModel(1, ucEndpoint, IN_PID, 0, 0, (KINETIS_USBHS *)USBHS_BASE2_ADD, 1); // allow the host model to inject frames
                                }
                            }
                        }
                        if (--iThisPeriodListLength == 0) {
                            USBHS1_FRINDEX = ((USBHS1_FRINDEX + 1) & 0x00003fff);
                            break;
                        }
                    }
                    USBHS1_FRINDEX = ((USBHS1_FRINDEX + 1) & 0x00003fff);
                }
            }
        }
    #endif
    }
#endif
    // RTC
    //
#if defined SUPPORT_RTC
    if ((SNVS_HPCR & SNVS_HPCR_RTC_EN) != 0) {                           // RTC is enabled
        if ((SNVS_LPSR & SNVS_LPSR_SPO) == 0) {                          // if invalid flag not set
            int iInterrupt = 0;
            unsigned long ulCounter;
            unsigned long ulOriginalValue = SNVS_HPRTCLR;
          //if ((RTC_CR & RTC_CR_OSCE) != 0) {                           // if oscillator is enabled
                ulCounter = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)32768) / 1000000); // 32kHz clock pulses in a TICK period
          //}
            if ((SNVS_HPRTCLR + ulCounter) < SNVS_HPRTCLR) {
                SNVS_HPRTCMR = (SNVS_HPRTCMR + 1);
            }
            SNVS_HPRTCMR &= 0x0000ffff;
            SNVS_HPRTCLR += ulCounter;
            if (((SNVS_HPCR & SNVS_HPCR_PI_EN) != 0) && ((SNVS_HPCR & SNVS_HPCR_PI_FREQ) != SNVS_HPCR_PI_FREQ)) {
                _EXCEPTION("Unexpected RTC interrupt frequency!");
            }
            if ((ulOriginalValue ^ SNVS_HPRTCLR) & 0x8000) {             // only interrupt on bit 15 count change is expected/supported (1s)
                SNVS_HPSR |= SNVS_HPSR_PI;
                if ((SNVS_HPCR & SNVS_HPCR_PI_EN) != 0) {
                    iInterrupt = 1;
                }
            }
            if (SNVS_HPRTCMR == (unsigned short)SNVS_HPTAMR) {
                if ((SNVS_HPRTCLR >> 15) == (SNVS_HPTALR >> 15)) {       // alarm match
                    SNVS_HPSR |= SNVS_HPSR_HPTA;
                    if ((SNVS_HPCR & SNVS_HPCR_HPTA_EN) != 0) {
                        iInterrupt = 1;
                    }
                }
            }
            if (iInterrupt != 0) {
                if (fnGenInt(irq_SNVS_Func_ID) != 0) {                   // if RTC interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_SNVS_Func();       // call the interrupt handler
                }
            }
        }
    }
#endif
#if defined KINETIS_KE && !defined KINETIS_WITH_SRTC
    if ((SIM_SCGC & SIM_SCGC_RTC) != 0) {
        unsigned long ulCount = 0;
        switch (RTC_SC & RTC_SC_RTCLKS_BUS) {                            // RTC clock source
        case RTC_SC_RTCLKS_EXT:                                          // external clock
    #if !defined _EXTERNAL_CLOCK                                         // if no external clock is available the OSCERCLK is not valid
        #define _EXTERNAL_CLOCK 0
    #endif
            ulCount = (unsigned long)((((unsigned long long)TICK_RESOLUTION) * (unsigned long long)_EXTERNAL_CLOCK)/1000000); // prescaler count in tick interval
            switch (RTC_SC & RTC_SC_RTCPS_1000) {
            case RTC_SC_RTCPS_1:
                break;
            case RTC_SC_RTCPS_2:
                ulCount /= 2;
                break;
            case RTC_SC_RTCPS_4:
                ulCount /= 4;
                break;
            case RTC_SC_RTCPS_8:
                ulCount /= 8;
                break;
            case RTC_SC_RTCPS_16:
                ulCount /= 16;
                break;
            case RTC_SC_RTCPS_32:
                ulCount /= 32;
                break;
            case RTC_SC_RTCPS_64:
                ulCount /= 64;
                break;
            }
            break;
        case RTC_SC_RTCLKS_INT:                                          // internal 32kHz clock
            ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)ICSIRCLK)/1000000); // count value in a tick period
            switch (RTC_SC & RTC_SC_RTCPS_1000) {                        // apply prescaler
            case RTC_SC_RTCPS_1:
                break;
            case RTC_SC_RTCPS_2:
                ulCount /= 2;
                break;
            case RTC_SC_RTCPS_4:
                ulCount /= 4;
                break;
            case RTC_SC_RTCPS_8:
                ulCount /= 8;
                break;
            case RTC_SC_RTCPS_16:
                ulCount /= 16;
                break;
            case RTC_SC_RTCPS_32:
                ulCount /= 32;
                break;
            case RTC_SC_RTCPS_64:
                ulCount /= 64;
                break;
            }
            break;
        case RTC_SC_RTCLKS_1K:                                           // internal 1kHz clock
    #if TICK_RESOLUTION >= 1000
            ulCount = (TICK_RESOLUTION/1000);
    #else
            ulCount = 1;
    #endif
            switch (RTC_SC & RTC_SC_RTCPS_1000) {
            case RTC_SC_RTCPS_128:
                ulCount /= 128;
                break;
            case RTC_SC_RTCPS_256:
                ulCount /= 256;
                break;
            case RTC_SC_RTCPS_512:
                ulCount /= 512;
                break;
            case RTC_SC_RTCPS_1024:
                ulCount /= 1024;
                break;
            case RTC_SC_RTCPS_2048:
                ulCount /= 2048;
                break;
            case RTC_SC_RTCPS_100:
                ulCount /= 100;
                break;
            case RTC_SC_RTCPS_1000:
                ulCount /= 1000;
                break;
            }
            break;
        case RTC_SC_RTCLKS_BUS:                                          // bus clock
            ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)BUS_CLOCK)/1000000);
            switch (RTC_SC & RTC_SC_RTCPS_1000) {
            case RTC_SC_RTCPS_128:
                ulCount /= 128;
                break;
            case RTC_SC_RTCPS_256:
                ulCount /= 256;
                break;
            case RTC_SC_RTCPS_512:
                ulCount /= 512;
                break;
            case RTC_SC_RTCPS_1024:
                ulCount /= 1024;
                break;
            case RTC_SC_RTCPS_2048:
                ulCount /= 2048;
                break;
            case RTC_SC_RTCPS_100:
                ulCount /= 100;
                break;
            case RTC_SC_RTCPS_1000:
                ulCount /= 1000;
                break;
            }
            break;
        }
        ulCount += RTC_CNT;
        if (ulCount > RTC_MOD) {
            ulCount -= RTC_MOD;
            RTC_CNT = ulCount;
            if ((RTC_SC & RTC_SC_RTIE) != 0) {                           // if interrupt enabled
                if (fnGenInt(irq_RTC_OVERFLOW_ID) != 0) {                // if core interrupt interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_RTC_OVERFLOW();    // call the interrupt handler
                }
            }
        }
        else {
            RTC_CNT = ulCount;
        }
    }
#endif
#if defined SUPPORT_LPTMR && (LPTMR_AVAILABLE > 0)
    #if defined KINETIS_KM
    if (((IS_POWERED_UP(6, LPTMR0)) != 0) && ((LPTMR0_CSR & LPTMR_CSR_TEN) != 0))
    #else
    if (((IS_POWERED_UP(5, LPTMR0)) != 0) && ((LPTMR0_CSR & LPTMR_CSR_TEN) != 0))
    #endif
    {                                                                    // if the low power timer is enabled and running
        unsigned long ulCount = 0;                                       // count in a tick period
        switch (LPTMR0_PSR & LPTMR_PSR_PCS_OSC0ERCLK) {
        case LPTMR_PSR_PCS_LPO:                                          // LPO source
    #if TICK_RESOLUTION >= 1000
            ulCount = (TICK_RESOLUTION/1000);                            // counts in a tick interval
    #else
            ulCount = 1;
    #endif
            break;
    #if defined KINETIS_WITH_SCG
        case LPTMR_PSR_PCS_SIRCCLK:
            _EXCEPTION("To add");
            break;
    #else
        case LPTMR_PSR_PCS_MCGIRCLK:
            if ((MCG_C2 & MCG_C2_IRCS) != 0) {                           // using 4MHz RC
                ulCount = (TICK_RESOLUTION * (4000000/1000000));
            }
            else {
                ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)35000)/1000000); // using 35kHz RC
            }
            break;
    #endif
        case LPTMR_PSR_PCS_ERCLK32K:
            ulCount = ((TICK_RESOLUTION * 32768)/1000000);               // counts in a tick interval
            break;
        case LPTMR_PSR_PCS_OSC0ERCLK:
    #if defined _EXTERNAL_CLOCK
            ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)_EXTERNAL_CLOCK)/1000000); // external clocks in a tick period (assuming no pre-scaler)
    #else
            _EXCEPTION("no external clock defined so this selection should not be used");
    #endif
            break;
        }
        if ((LPTMR0_PSR & LPTMR_PSR_PBYP) == 0) {                        // if the prescaler bypass hasn't been disabled
            ulCount >>= (((LPTMR0_PSR & LPTMR_PSR_PRESCALE_MASK) >> LPTMR_PSR_PRESCALE_SHIFT) + 1);
        }
        if (LPTMR0_CNR <= LPTMR0_CMR) {                                  // timer count has not yet reached the match value
            ulCount = (LPTMR0_CNR + ulCount);                            // the next count value
            if (ulCount > LPTMR0_CMR) {
                if ((LPTMR0_CSR & LPTMR_CSR_TFC_FREERUN) == 0) {
                    ulCount = (ulCount - LPTMR0_CMR);
                    if (ulCount > LPTMR0_CMR) {
                        ulCount = LPTMR0_CMR;
                    }
                }
                if ((LPTMR0_CSR & LPTMR_CSR_TIE) != 0) {                 // if LPTMR interrupt is enabled
    #if defined irq_LPTMR_PWT_ID
                    if (fnGenInt(irq_LPTMR_PWT_ID) != 0) {               // if LPTMR interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPTMR_PWT();   // call the interrupt handler
                    }
    #else
                    if (fnGenInt(irq_LPTMR0_ID) != 0) {                  // if LPTMR interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                        ptrVect->processor_interrupts.irq_LPTMR0();      // call the interrupt handler
                    }
    #endif
                }
            }
        }
        else {
            ulCount = (LPTMR0_CNR + ulCount);
        }
        if (ulCount > 0xffff) {
            ulCount = (ulCount - 0xffff);
        }
        LPTMR0_CNR = ulCount;
    }
    #if LPTMR_AVAILABLE > 1
        if (((IS_POWERED_UP(5, LPTMR1)) != 0) && ((LPTMR1_CSR & LPTMR_CSR_TEN) != 0)) { // if the low power timer is enabled and running
        unsigned long ulCount = 0;                                       // count in a tick period
        switch (LPTMR1_PSR & LPTMR_PSR_PCS_OSC0ERCLK) {
        case LPTMR_PSR_PCS_LPO:
    #if TICK_RESOLUTION >= 1000
            ulCount = (TICK_RESOLUTION/1000);                            // counts in a tick interval
    #else
            ulCount = 1;
    #endif
            break;
    #if defined KINETIS_WITH_SCG
        case LPTMR_PSR_PCS_SIRCCLK:
            _EXCEPTION("To add");
            break;
    #else
        case LPTMR_PSR_PCS_MCGIRCLK:
            if ((MCG_C2 & MCG_C2_IRCS) != 0) {
                ulCount = (TICK_RESOLUTION * (4000000/1000000));
            }
            else {
                ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)35000)/1000000);
            }
            break;
    #endif
        case LPTMR_PSR_PCS_ERCLK32K:
            ulCount = ((TICK_RESOLUTION * 32768)/1000000);               // counts in a tick interval
            break;
        case LPTMR_PSR_PCS_OSC0ERCLK:
    #if defined _EXTERNAL_CLOCK
            ulCount = (unsigned long)(((unsigned long long)TICK_RESOLUTION * (unsigned long long)_EXTERNAL_CLOCK)/1000000); // external clocks in a tick period (assuming no pre-scaler)
    #else
            _EXCEPTION("no external clock defined so this selection should not be used");
    #endif
            break;
        }
        if ((LPTMR1_PSR & LPTMR_PSR_PBYP) == 0) {                        // if the prescaler bypass hasn't been disabled
            ulCount >>= (((LPTMR1_PSR & LPTMR_PSR_PRESCALE_MASK) >> LPTMR_PSR_PRESCALE_SHIFT) + 1);
        }
        if (LPTMR1_CNR <= LPTMR1_CMR) {                                  // timer count has not yet reached the match value
            ulCount = (LPTMR1_CNR + ulCount);                            // the next count value
            if (ulCount > LPTMR1_CMR) {
                if ((LPTMR1_CSR & LPTMR_CSR_TFC_FREERUN) == 0) {
                    ulCount = (ulCount - LPTMR1_CMR);
                    if (ulCount > LPTMR1_CMR) {
                        ulCount = LPTMR1_CMR;
                    }
                }
                if ((LPTMR1_CSR & LPTMR_CSR_TIE) != 0) {                 // if LPTMR interrupt is enabled
        #if !defined irq_LPTMR1_ID && defined INTMUX0_AVAILABLE
                    if (fnGenInt(irq_INTMUX0_0_ID + INTMUX_LPTMR1) != 0)
        #else
                    if (fnGenInt(irq_LPTMR1_ID) != 0)
        #endif
                    {                                                    // if LPTMR interrupt is not disabled
                        VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
        #if !defined irq_LPTMR1_ID
                        fnCallINTMUX(INTMUX_LPTMR1, INTMUX0_PERIPHERAL_LPTMR1, (unsigned char *)&ptrVect->processor_interrupts.irq_LPTMR1);
        #else
                        ptrVect->processor_interrupts.irq_LPTMR1();      // call the interrupt handler
        #endif
                    }
                }
            }
        }
        else {
            ulCount = (LPTMR1_CNR + ulCount);
        }
        if (ulCount > 0xffff) {
            ulCount = (ulCount - 0xffff);
        }
        LPTMR1_CNR = ulCount;
    }
    #endif
#endif
#if defined SUPPORT_ADC_
    fnTriggerADC(0, 0);                                                  // handle software triggered ADC0
    #if ADC_CONTROLLERS > 1
    fnTriggerADC(1, 0);                                                  // handle software triggered ADC1
    #endif
    #if ADC_CONTROLLERS > 2
    fnTriggerADC(2, 0);                                                  // handle software triggered ADC2
    #endif
    #if ADC_CONTROLLERS > 3
    fnTriggerADC(3, 0);                                                  // handle software triggered ADC3
    #endif
#endif
#if defined SUPPORT_TIMER
    if ((fnGPTimerPowered(0) != 0) && ((GPT1_CR & GPT_CLKSRC_MASK) != 0)) { // if the GPT1 is powered and clocked
        fnHandleGPTimer(0, (GPTIMER_MODULE *)GPT1_BLOCK);
    }
    if ((fnGPTimerPowered(1) != 0) && ((GPT2_CR & GPT_CLKSRC_MASK) != 0)) { // if the GPT2 is powered and clocked
        fnHandleGPTimer(1, (GPTIMER_MODULE *)GPT2_BLOCK);
    }
#endif
#if defined SERIAL_INTERFACE
    // Idle line detection on UARTs
    //
    while (iUART < (LPUARTS_AVAILABLE + UARTS_AVAILABLE)) {
        if (iUART_rx_Active[iUART] != 0) {                               // if the UART's input has been active
            if (iUART_rx_Active[iUART] == 1) {                           // detect idle line
                fnSimulateSerialIn(iUART, 0, 0);                         // simulate idle line event
            }
            iUART_rx_Active[iUART]--;
        }
        iUART++;
    }
#endif
    return 0;
}

#if defined RUN_IN_FREE_RTOS
extern void fnExecutePendingInterrupts(int iRecursive)
{
    static int iExecuting = 0;
    static unsigned char ucPresentPriority = 255;                        // lowest priority that doesn't block anything
    unsigned char ucPreviousPriority = ucPresentPriority;
    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
    if (iExecuting != 0) {
        if (iRecursive == 0) {
            return;                                                      // ignore when called non-recursively when already executing
        }
    }
    iExecuting = 1;
    if ((iMX.CORTEX_M7_REGS.ulPRIMASK & INTERRUPT_MASKED) != 0) {
        iExecuting = 0;
        return;                                                          // if the global interrupt is masked we quit
    }
    if ((INT_CONT_STATE_REG & PENDSTSET) != 0) {                         // systick is pending
        unsigned char ucPriority = (SYSTEM_HANDLER_12_15_PRIORITY_REGISTER >> (24 + __NVIC_PRIORITY_SHIFT)); // systick interrupt priority
        if (ucPriority < ucPresentPriority) {                            // check that the interrupt has adequate priority to be called
            ucPresentPriority = ucPriority;                              // set the new priority level
            ptrVect->ptrSysTick();                                       // call the systick interrupt service routine
            ucPresentPriority = ucPreviousPriority;
            fnExecutePendingInterrupts(1);                               // allow further pending interrupt to be executed if needed
        }
    }
    iExecuting = 0;
}
#endif

extern unsigned char *fnGetSimTxBufferAdd(void)
{
#if defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET
    return (fnGetTxBufferAdd(0));
#else
    return 0;
#endif
}


#if defined ETH_INTERFACE
    extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen);
#endif

#if defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET // we feed frames in promiscuous mode and filter them according to their details
static KINETIS_FEC_BD *ptrSimRxBD = 0;
static int iFirstRec = 0;

extern void fnResetENET(void)
{
    iFirstRec = 0;
}

static int fnCheckEthernet(unsigned char *ucData, unsigned short usLen, int iForce)
{
	unsigned char *ptrInput;
    unsigned short usCopyLen;
    unsigned short usFullLength;
    #if defined USE_MULTIPLE_BUFFERS
    unsigned short ucMaxRxBufLen = 256;
    #else
    unsigned short ucMaxRxBufLen = 0x600;
    #endif

    if ((iForce == 0) && (fnCheckEthernetMode(ucData, usLen) == 0)) {
        return 0;                                                        // if we are not in promiscuous mode and the MAC address is not defined to be received we ignore the frame
    }

    if (iFirstRec == 0) {
        if ((ECR & ETHER_EN) == 0) {
            return 0;                                                    // ignore if the FEC has not yet been programmed
        }
        iFirstRec = 1;                                                   // we do this only once
        ptrSimRxBD = (KINETIS_FEC_BD *)ERDSR;                            // set to first BD
    }

    if ((ptrSimRxBD->usBDControl & EMPTY_BUFFER) == 0) {                 // drop reception frame if there are no free buffer descriptors
        return 0;
    }

    usFullLength = usLen;

    while (usLen != 0) {
        ptrInput = ptrSimRxBD->ptrBD_Data;
        if (usLen > ucMaxRxBufLen) {
            ptrSimRxBD->usBDLength = ucMaxRxBufLen;
            usCopyLen = ucMaxRxBufLen;
        }
        else {
            usCopyLen = ptrSimRxBD->usBDLength = usLen;
            ptrSimRxBD->usBDLength = usFullLength;
            ptrSimRxBD->usBDControl |= LAST_IN_FRAME_RX;
        }
        usLen -= usCopyLen;
	    while (usCopyLen-- != 0) {
	        *ptrInput++ = *ucData++;                                     // put bytes in input buffer
	    }
        if (usLen == 0) {                                                // last buffer
            if ((RCR & CRCFWD) == 0) {                                   // if CRC stripping is not disabled
                ptrSimRxBD->usBDLength += 4;                             // add dummy CRC32 - this corrects the receive count and also tests that there is adequate buffer space
            }
            *ptrInput++ = 0x33;
            *ptrInput++ = 0x66;
            *ptrInput++ = 0xaa;
            *ptrInput   = 0x55;
        }
        ptrSimRxBD->usBDControl &= ~EMPTY_BUFFER;                        // mark that the buffer contains
        EIR |= RXF;                                                      // set receive frame interrupt event
        if (((EIMR & RXF) != 0) && (ptrSimRxBD->usBDControl & LAST_IN_FRAME_RX)) { // if interrupts are enabled
            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
          //ptrVect->processor_interrupts.irq_ETH_RX();                  // call the interrupt handler
            ptrVect->processor_interrupts.irq_ETH();                     // call the interrupt handler
	    }
        if ((ptrSimRxBD->usBDControl & WRAP_BIT_RX) != 0) {
            ptrSimRxBD = (KINETIS_FEC_BD *)ERDSR;                        // set to first BD
        }
        else {
            ptrSimRxBD++;                                                // set to next RX BD
        }
    }
    return 1;
}
#endif




extern int fnSimulateEthernetIn(unsigned char *ucData, unsigned short usLen, int iForce)
{
    int iReturn = 1;
#if defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET // we feed frames in promiscuous mode and filter them according to their details
    iReturn = fnCheckEthernet(ucData, usLen, iForce);
#endif
#if defined ETH_INTERFACE && defined ENC424J600_INTERFACE
    iReturn &= ~(fnCheckENC424J600(ucData, usLen, iForce));
#endif
    return iReturn;
}


#if defined SUPPORT_KEY_SCAN

#if KEY_COLUMNS == 0
    #define _NON_MATRIX
    #undef KEY_COLUMNS
    #define KEY_COLUMNS  VIRTUAL_KEY_COLUMNS
    #undef KEY_ROWS
    #define KEY_ROWS VIRTUAL_KEY_ROWS
#endif


int iKeyPadInputs[KEY_COLUMNS][KEY_ROWS] = {0};

extern void fnSimulateKeyChange(int *intTable)
{
#if defined _NON_MATRIX
    int iCol, iRow;
    int iChange;
    for (iCol = 0; iCol < KEY_COLUMNS; iCol++) {
        for (iRow = 0; iRow < KEY_ROWS; iRow++) {
            iChange = iKeyPadInputs[iCol][iRow];                         // original value
            iKeyPadInputs[iCol][iRow] = *intTable++;                     // new value
            if (iChange != iKeyPadInputs[iCol][iRow]) {
    #if defined KEY_POLARITY_POSITIVE
                if (iChange)
    #else
                if (iChange != 0)                                        // generally a key press is a '0' 
    #endif
                {
                    iChange = SET_INPUT;
                }
                else {
                    iChange = CLEAR_INPUT;
                }
                switch ((iCol * KEY_ROWS) + iRow) {                      // the specific input
    #if defined KEY_1_PORT_REF
                case 0:                                                  // first key
                    fnSimulateInputChange(KEY_1_PORT_REF, fnMapPortBit(KEY_1_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_2_PORT_REF
                case 1:                                                  // second key
                    fnSimulateInputChange(KEY_2_PORT_REF, fnMapPortBit(KEY_2_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_3_PORT_REF
                case 2:                                                  // third key
                    fnSimulateInputChange(KEY_3_PORT_REF, fnMapPortBit(KEY_3_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_4_PORT_REF
                case 3:                                                  // fourth key
                    fnSimulateInputChange(KEY_4_PORT_REF, fnMapPortBit(KEY_4_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_5_PORT_REF
                case 4:                                                  // fifth key
                    fnSimulateInputChange(KEY_5_PORT_REF, fnMapPortBit(KEY_5_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_6_PORT_REF
                case 5:                                                  // sixth key
                    fnSimulateInputChange(KEY_6_PORT_REF, fnMapPortBit(KEY_6_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_7_PORT_REF
                case 6:                                                  // seventh key
                    fnSimulateInputChange(KEY_7_PORT_REF, fnMapPortBit(KEY_7_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_8_PORT_REF
                case 7:                                                  // eighth key
                    fnSimulateInputChange(KEY_8_PORT_REF, fnMapPortBit(KEY_8_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_9_PORT_REF
                case 8:                                                  // ninth key
                    fnSimulateInputChange(KEY_9_PORT_REF, fnMapPortBit(KEY_9_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_10_PORT_REF
                case 9:                                                  // tenth key
                    fnSimulateInputChange(KEY_10_PORT_REF, fnMapPortBit(KEY_10_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_11_PORT_REF
                case 10:                                                 // eleventh key
                    fnSimulateInputChange(KEY_11_PORT_REF, fnMapPortBit(KEY_11_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_12_PORT_REF
                case 11:                                                 // twelf key
                    fnSimulateInputChange(KEY_12_PORT_REF, fnMapPortBit(KEY_12_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_13_PORT_REF
                case 12:                                                 // thirteenth key
                    fnSimulateInputChange(KEY_13_PORT_REF, fnMapPortBit(KEY_13_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_14_PORT_REF
                case 13:                                                 // fourteenth key
                    fnSimulateInputChange(KEY_14_PORT_REF, fnMapPortBit(KEY_14_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_15_PORT_REF
                case 14:                                                 // fifteenth key
                    fnSimulateInputChange(KEY_15_PORT_REF, fnMapPortBit(KEY_15_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_16_PORT_REF
                case 15:                                                 // sixteenth key
                    fnSimulateInputChange(KEY_16_PORT_REF, fnMapPortBit(KEY_16_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_17_PORT_REF
                case 16:                                                 // seventeenth key
                    fnSimulateInputChange(KEY_17_PORT_REF, fnMapPortBit(KEY_17_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_18_PORT_REF
                case 17:                                                 // eighteenth key
                    fnSimulateInputChange(KEY_18_PORT_REF, fnMapPortBit(KEY_18_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_19_PORT_REF
                case 18:                                                 // nineteenth key
                    fnSimulateInputChange(KEY_19_PORT_REF, fnMapPortBit(KEY_19_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_20_PORT_REF
                case 19:                                                 // twentieth key
                    fnSimulateInputChange(KEY_20_PORT_REF, fnMapPortBit(KEY_20_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_21_PORT_REF
                case 20:                                                 // twenty first key
                    fnSimulateInputChange(KEY_21_PORT_REF, fnMapPortBit(KEY_21_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_22_PORT_REF
                case 21:                                                 // twenty second key
                    fnSimulateInputChange(KEY_22_PORT_REF, fnMapPortBit(KEY_22_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_23_PORT_REF
                case 22:                                                 // twenty third key
                    fnSimulateInputChange(KEY_23_PORT_REF, fnMapPortBit(KEY_23_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_24_PORT_REF
                case 23:                                                 // twenty fourth key
                    fnSimulateInputChange(KEY_24_PORT_REF, fnMapPortBit(KEY_24_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_25_PORT_REF
                case 24:                                                 // twenty fifth key
                    fnSimulateInputChange(KEY_25_PORT_REF, fnMapPortBit(KEY_25_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_26_PORT_REF
                case 25:                                                 // twenty sixth key
                    fnSimulateInputChange(KEY_26_PORT_REF, fnMapPortBit(KEY_26_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_27_PORT_REF
                case 26:                                                  // twenty seventh key
                    fnSimulateInputChange(KEY_27_PORT_REF, fnMapPortBit(KEY_27_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_28_PORT_REF
                case 27:                                                  // twenty eighth key
                    fnSimulateInputChange(KEY_28_PORT_REF, fnMapPortBit(KEY_28_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_29_PORT_REF
                case 28:                                                  // twenty ninth key
                    fnSimulateInputChange(KEY_29_PORT_REF, fnMapPortBit(KEY_29_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_30_PORT_REF
                case 29:                                                  // thirtieth key
                    fnSimulateInputChange(KEY_30_PORT_REF, fnMapPortBit(KEY_30_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_31_PORT_REF
                case 30:                                                  // thirty first key
                    fnSimulateInputChange(KEY_31_PORT_REF, fnMapPortBit(KEY_31_PORT_PIN), iChange);
                    break;
    #endif
    #if defined KEY_32_PORT_REF
                case 31:                                                  // thirty second key
                    fnSimulateInputChange(KEY_32_PORT_REF, fnMapPortBit(KEY_32_PORT_PIN), iChange);
                    break;
    #endif
                }
            }
        }
    }
#else
    memcpy(iKeyPadInputs, intTable, sizeof(iKeyPadInputs));              // copy key pad state to local set
#endif
}

static int fnColumnLow(int iColumnOutput)
{
#if defined KEY_COLUMNS && !defined _NON_MATRIX
    switch (iColumnOutput) {
    case 0:
        return (KEY_COL_OUT_1 & ~KEY_COL_OUT_PORT_1 & KEY_COL_OUT_DDR_1);// if column 1 is being driven low
    #if KEY_COLUMNS > 1
    case 1:
        return (KEY_COL_OUT_2 & ~KEY_COL_OUT_PORT_2 & KEY_COL_OUT_DDR_2);// if column 2 is being driven low
    #endif
    #if KEY_COLUMNS > 2
    case 2:
        return (KEY_COL_OUT_3 & ~KEY_COL_OUT_PORT_3 & KEY_COL_OUT_DDR_3);// if column 3 is being driven low
    #endif
    #if KEY_COLUMNS > 3
    case 3:
        return (KEY_COL_OUT_4 & ~KEY_COL_OUT_PORT_4 & KEY_COL_OUT_DDR_4);// if column 4 is being driven low
    #endif
    #if KEY_COLUMNS > 4
    case 4:
        return (KEY_COL_OUT_5 & ~KEY_COL_OUT_PORT_5 & KEY_COL_OUT_DDR_5);// if column 5 is being driven low
    #endif
    #if KEY_COLUMNS > 5
    case 5:
        return (KEY_COL_OUT_6 & ~KEY_COL_OUT_PORT_6 & KEY_COL_OUT_DDR_6);// if column 6 is being driven low
    #endif
    #if KEY_COLUMNS > 6
    case 6:
        return (KEY_COL_OUT_7 & ~KEY_COL_OUT_PORT_7 & KEY_COL_OUT_DDR_7);// if column 7 is being driven low
    #endif
    #if KEY_COLUMNS > 7
    case 7:
        return (KEY_COL_OUT_8 & ~KEY_COL_OUT_PORT_8 & KEY_COL_OUT_DDR_8);// if column 8 is being driven low
    #endif
    }
#endif
    return 0;
}

static void fnSetRowInput(int iRowInput, int iState)
{
    int iChange;

    if (iState) {
        iChange = CLEAR_INPUT;
    }
    else {
        iChange = SET_INPUT;
    }

#if !defined _NON_MATRIX
    switch (iRowInput) {
    case 0:
    #if defined KEY_ROWS
        fnSimulateInputChange(KEY_ROW_IN_PORT_1_REF, fnMapPortBit(KEY_ROW_IN_1), iChange);
    #endif
        break;
    case 1:
    #if KEY_ROWS > 1
        fnSimulateInputChange(KEY_ROW_IN_PORT_2_REF, fnMapPortBit(KEY_ROW_IN_2), iChange);
    #endif
        break;
    case 2:
    #if KEY_ROWS > 2
        fnSimulateInputChange(KEY_ROW_IN_PORT_3_REF, fnMapPortBit(KEY_ROW_IN_3), iChange);
    #endif
        break;
    case 3:
    #if KEY_ROWS > 3
        fnSimulateInputChange(KEY_ROW_IN_PORT_4_REF, fnMapPortBit(KEY_ROW_IN_4), iChange);
    #endif
        break;
    case 4:
    #if KEY_ROWS > 4
        fnSimulateInputChange(KEY_ROW_IN_PORT_5_REF, fnMapPortBit(KEY_ROW_IN_5), iChange);
    #endif
        break;
    case 5:
    #if KEY_ROWS > 5
        fnSimulateInputChange(KEY_ROW_IN_PORT_6_REF, fnMapPortBit(KEY_ROW_IN_6), iChange);
    #endif
        break;
    case 6:
    #if KEY_ROWS > 6
        fnSimulateInputChange(KEY_ROW_IN_PORT_7_REF, fnMapPortBit(KEY_ROW_IN_7), iChange);
    #endif
        break;
    case 7:
    #if KEY_ROWS > 7
        fnSimulateInputChange(KEY_ROW_IN_PORT_8_REF, fnMapPortBit(KEY_ROW_IN_8), iChange);
    #endif
        break;
    }
#endif
}

// This routine updates the ports to respect the present setting of a connected matrix key pad
//
extern void fnSimMatrixKB(void)
{
    int i, j;
    iFlagRefresh = fnPortChanges(1);                                     // synchronise with present settings                                                                         
    for (i = 0; i < KEY_COLUMNS; i++) {                                  // check whether a column control is being driven low. If this is the case, any pressed ones in the column are seen at the row input
        if (fnColumnLow(i)) {
            for (j = 0; j < KEY_ROWS; j++) {
                fnSetRowInput(j, iKeyPadInputs[i][j]);
            }
        }
    }
}
#endif

#if defined DEVICE_WITH_SLCD
static unsigned long lcd_wf3to0 = 0;
static unsigned long lcd_wf7to4 = 0;
static unsigned long lcd_wf11to8 = 0;
static unsigned long lcd_wf15to12 = 0;
static unsigned long lcd_wf19to16 = 0;
static unsigned long lcd_wf23to20 = 0;
static unsigned long lcd_wf27to24 = 0;
static unsigned long lcd_wf31to28 = 0;
static unsigned long lcd_wf35to32 = 0;
static unsigned long lcd_wf39to36 = 0;
static unsigned long lcd_wf43to40 = 0;
static unsigned long lcd_wf47to44 = 0;
static unsigned long lcd_wf51to48 = 0;
static unsigned long lcd_wf55to52 = 0;
static unsigned long lcd_wf59to56 = 0;
static unsigned long lcd_wf63to60 = 0;

// check for changes in SLCD segment registers and call display updates when necessary
//
extern void fnSimulateSLCD(void)
{
    #if defined SLCD_FILE
        #if defined KINETIS_KL || defined KINETIS_KL33 || defined KINETIS_KL43
    if (((SIM_SCGC5 & SIM_SCGC5_SLCD) == 0) || ((LCD_GCR & LCD_GCR_LCDEN) == 0)) { // if SLCD controller not enabled
        return;
    }
        #else
    if (((SIM_SCGC3 & SIM_SCGC3_SLCD) == 0) || ((LCD_GCR & LCD_GCR_LCDEN) != 0)) { // if SLCD controller not enabled
        return;
    }
        #endif
    if (LCD_WF3TO0 != lcd_wf3to0) {
        CollectCommand(0, LCD_WF3TO0);                                   // inform of the new value (register 0)
        lcd_wf3to0 = LCD_WF3TO0;
    }
    if (LCD_WF7TO4 != lcd_wf7to4) {
        CollectCommand(1, LCD_WF7TO4);                                   // inform of the new value (register 1)
        lcd_wf7to4 = LCD_WF7TO4;
    }
    if (LCD_WF11TO8 != lcd_wf11to8) {
        CollectCommand(2, LCD_WF11TO8);                                  // inform of the new value (register 2)
        lcd_wf11to8 = LCD_WF11TO8;
    }
    if (LCD_WF15TO12 != lcd_wf15to12) {
        CollectCommand(3, LCD_WF15TO12);                                 // inform of the new value (register 3)
        lcd_wf15to12 = LCD_WF15TO12;
    }
    if (LCD_WF19TO16 != lcd_wf19to16) {
        CollectCommand(4, LCD_WF19TO16);                                 // inform of the new value (register 4)
        lcd_wf19to16 = LCD_WF19TO16;
    }
    if (LCD_WF23TO20 != lcd_wf23to20) {
        CollectCommand(5, LCD_WF23TO20);                                 // inform of the new value (register 5)
        lcd_wf23to20 = LCD_WF23TO20;
    }
    if (LCD_WF27TO24 != lcd_wf27to24) {
        CollectCommand(6, LCD_WF27TO24);                                 // inform of the new value (register 6)
        lcd_wf27to24 = LCD_WF27TO24;
    }
    if (LCD_WF31TO28 != lcd_wf31to28) {
        CollectCommand(7, LCD_WF31TO28);                                 // inform of the new value (register 7)
        lcd_wf31to28 = LCD_WF31TO28;
    }
    if (LCD_WF35TO32 != lcd_wf35to32) {
        CollectCommand(8, LCD_WF35TO32);                                 // inform of the new value (register 8)
        lcd_wf35to32 = LCD_WF35TO32;
    }
    if (LCD_WF39TO36 != lcd_wf39to36) {
        CollectCommand(9, LCD_WF39TO36);                                 // inform of the new value (register 9)
        lcd_wf39to36 = LCD_WF39TO36;
    }
    if (LCD_WF43TO40 != lcd_wf43to40) {
        CollectCommand(10, LCD_WF43TO40);                                // inform of the new value (register 10)
        lcd_wf43to40 = LCD_WF43TO40;
    }
    if (LCD_WF47TO44 != lcd_wf47to44) {
        CollectCommand(11, LCD_WF47TO44);                                // inform of the new value (register 11)
        lcd_wf47to44 = LCD_WF47TO44;
    }
    if (LCD_WF51TO48 != lcd_wf51to48) {
        CollectCommand(12, LCD_WF51TO48);                                // inform of the new value (register 12)
        lcd_wf51to48 = LCD_WF51TO48;
    }
    if (LCD_WF55TO52 != lcd_wf55to52) {
        CollectCommand(13, LCD_WF55TO52);                                // inform of the new value (register 13)
        lcd_wf55to52 = LCD_WF55TO52;
    }
    if (LCD_WF59TO56 != lcd_wf59to56) {
        CollectCommand(14, LCD_WF59TO56);                                 // inform of the new value (register 14)
        lcd_wf59to56 = LCD_WF59TO56;
    }
    if (LCD_WF63TO60 != lcd_wf63to60) {
        CollectCommand(15, LCD_WF63TO60);                                 // inform of the new value (register 15)
        lcd_wf63to60 = LCD_WF63TO60;
    }
    #endif
}
#endif


#if defined BATTERY_BACKED_RAM
// Return all RTC content which is battery backed up
//
extern int fnGetBatteryRAMContent(unsigned char *ucData, unsigned long ulReference)
{
    return 0;                                                            // all data saved
}
extern int fnPutBatteryRAMContent(unsigned char ucData, unsigned long ulReference)
{
    return 0;                                                            // no more data accepted
}
#endif

#if NUMBER_OF_SDHC > 0
extern void fnCheckSD_Detect(void)
{
    unsigned long ulInitialState = SDHC_PRSSTAT;
    #if defined iMX_RT105X || defined iMX_RT106X
    switch (IOMUXC_USDHC1_CD_B_SELECT_INPUT) {
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_EMC_35_ALT6:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_35_USDHC1_CD_B) {
            if ((ulPort_in_3 & PORT3_BIT21) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_AD_B1_02_ALT6:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_02_USDHC1_CD_B) {
            if ((ulPort_in_1 & PORT1_BIT18) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_B1_12_ALT6:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_B1_12_USDHC1_CD_B) {
            if ((ulPort_in_2 & PORT2_BIT28) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    }
    #else
    switch (IOMUXC_USDHC1_CD_B_SELECT_INPUT) {
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_SD_B0_06_ALT0:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_06 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_SD_B0_06_USDHC1_CD_B) {
            if ((ulPort_in_3 & PORT3_BIT19) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_AD_B0_05_ALT2:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_05 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B0_05_USDHC1_CD_B) {
            if ((ulPort_in_1 & PORT1_BIT5) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_AD_B1_10_ALT3:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_10 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_AD_B1_10_USDHC1_CD_B) {
            if ((ulPort_in_1 & PORT1_BIT26) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    case IOMUXC_USDHC1_CD_B_SELECT_INPUT_GPIO_EMC_38_ALT7:
        if ((IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 & IOMUXC_SW_MUX_CTL_PAD_MUX_MODE_MASK) == GPIO_EMC_38_USDHC1_CD_B) {
            if ((ulPort_in_1 & PORT1_BIT26) != 0) {
                SDHC_PRSSTAT &= ~(SDHC_PRSSTAT_CINST);
            }
            else {
                SDHC_PRSSTAT |= SDHC_PRSSTAT_CINST;
            }
        }
        break;
    }
    #endif
    if ((ulInitialState ^ SDHC_PRSSTAT) != 0) {                          // a change in card detection state occurred
        if ((SDHC_PRSSTAT & SDHC_PRSSTAT_CINST) != 0) {                  // inserted
            SDHC_IRQSTAT |= SDHC_IRQSTAT_CINS;
            if ((SDHC_IRQSIGEN & SDHC_IRQSIGEN_CINSIEN) != 0) {          // interrupt on insertion
                if (fnGenInt(irq_USDHC1_ID) != 0) {                      // if SD card interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_USDHC1();          // call the interrupt handler
                }
            }
        }
        else {                                                           // removed
            SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM;
            if ((SDHC_IRQSIGEN & SDHC_IRQSIGEN_CRMIEN) != 0) {           // interrupt on removal
                if (fnGenInt(irq_USDHC1_ID) != 0) {                      // if SD card interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_USDHC1();          // call the interrupt handler
                }
            }
        }
    }
}
#endif

#if defined CRYPTOGRAPHY
    #if defined OTFAD_BLOCK
static void fnDoOnTheFlyDecryption(unsigned char *ptrStart, unsigned char *ptrDestination, unsigned long ulLength, int iChannel)
{
    unsigned char *ptrStartOfDecryp;
    unsigned char *ptrEndOfDecryp;
    unsigned long *ptrKey;
    unsigned long *ptrNonce;
    unsigned long ulKeySelect;
    switch (iChannel) {
    case 0:
        ptrStartOfDecryp = (unsigned char *)(OTFAD_CTX0_RGD_W0 & OTFAD_RGD_W0_SRTADDR_MASK); // 1k aligned start address
        ptrKey = OTFAD_CTX0_KEY0_ADD;
        ptrNonce = OTFAD_CTX0_CTR0_ADD;
        ptrEndOfDecryp = (unsigned char *)(OTFAD_CTX0_RGD_W1 & OTFAD_RGD_W1_ENDADDR_MASK); // 1k aligned end address
        if ((OTFAD_CTX0_RGD_W1 & (OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD)) != (OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD)) {
            _EXCEPTION("Channel not enabled!");
        }
        ulKeySelect = uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2;
        break;
    case 1:
        ptrStartOfDecryp = (unsigned char *)(OTFAD_CTX1_RGD_W0 & OTFAD_RGD_W0_SRTADDR_MASK); // 1k aligned start address
        ptrKey = OTFAD_CTX1_KEY0_ADD;
        ptrNonce = OTFAD_CTX1_CTR0_ADD;
        ptrEndOfDecryp = (unsigned char *)(OTFAD_CTX1_RGD_W1 & OTFAD_RGD_W1_ENDADDR_MASK); // 1k aligned end address
        if ((OTFAD_CTX1_RGD_W1 & (OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD)) != (OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD)) {
            _EXCEPTION("Channel not enabled!");
        }
        ulKeySelect = uFUSE_BOOT_CFG2_BEE_KEY1_SEL_SW_GP2;
        break;
    default:
        _EXCEPTION("OTFAD channel no supported!");
        break;
    }
    if ((ptrStart < ptrStartOfDecryp) || ((ptrStart + ulLength) > ptrEndOfDecryp)) {
        _EXCEPTION("Area not in decryption mode!!!");
        return ;
    }
    else {
        unsigned long *ptrIn = (unsigned long *)fnGetFlashRealAdd(ptrStart);
        unsigned char *ptrDecrypted = ptrDestination;
        unsigned char ucNonceCtr[16];
        unsigned char ucKey[16];
        unsigned long ulTemp;
        int iLength = 0;
        if ((HW_OCOTP_CFG5 & ulKeySelect) != 0) {                        // SW_GP2 is being used as key
            fnGetSW_GP2(ucKey);
        }
        else {
            ucKey[0] = (unsigned char)*ptrKey;
            ucKey[1] = (unsigned char)(*ptrKey >> 8);
            ucKey[2] = (unsigned char)(*ptrKey >> 16);
            ucKey[3] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[4] = (unsigned char)*ptrKey;
            ucKey[5] = (unsigned char)(*ptrKey >> 8);
            ucKey[6] = (unsigned char)(*ptrKey >> 16);
            ucKey[7] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[8] = (unsigned char)*ptrKey;
            ucKey[9] = (unsigned char)(*ptrKey >> 8);
            ucKey[10] = (unsigned char)(*ptrKey >> 16);
            ucKey[11] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[12] = (unsigned char)*ptrKey;
            ucKey[13] = (unsigned char)(*ptrKey >> 8);
            ucKey[14] = (unsigned char)(*ptrKey >> 16);
            ucKey[15] = (unsigned char)(*ptrKey >> 24);
        }

        fnAES_Init((AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_RESET_IV), (const unsigned char *)ucKey, 128); // register the key

        ulTemp = (*ptrNonce ^ *(ptrNonce + 1));                          // concatonated with XOR or two counter register values
        ucNonceCtr[8] = (unsigned char)ulTemp;
        ucNonceCtr[9] = (unsigned char)(ulTemp >> 8);
        ucNonceCtr[10] = (unsigned char)(ulTemp >> 16);
        ucNonceCtr[11] = (unsigned char)(ulTemp >> 24);

        ucNonceCtr[0] = (unsigned char)*ptrNonce;                        // concatonated with the two counter registers
        ucNonceCtr[1] = (unsigned char)(*ptrNonce >> 8);
        ucNonceCtr[2] = (unsigned char)(*ptrNonce >> 16);
        ucNonceCtr[3] = (unsigned char)(*ptrNonce >> 24);
        ptrNonce++;
        ucNonceCtr[4] = (unsigned char)*ptrNonce;
        ucNonceCtr[5] = (unsigned char)(*ptrNonce >> 8);
        ucNonceCtr[6] = (unsigned char)(*ptrNonce >> 16);
        ucNonceCtr[7] = (unsigned char)(*ptrNonce >> 24);

        // Decrypt the area using the defined keys
        //
        while (ulLength >= 16) {                                         // overall length is expected to be divisible by 16
            ulTemp = (((unsigned long)ptrStart) & ~0x3);                 // address
            ucNonceCtr[15] = (unsigned char)ulTemp;
            ucNonceCtr[14] = (unsigned char)(ulTemp >> 8);
            ucNonceCtr[13] = (unsigned char)(ulTemp >> 16);
            ucNonceCtr[12] = (unsigned char)(ulTemp >> 24);
            fnAES_Init(AES_COMMAND_AES_PRIME_IV, (const unsigned char *)ucNonceCtr, 0); // prime the initial vector
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT_128, (const unsigned char *)ptrIn, (ptrDecrypted + iLength), 16); // decrypt the flash content
            iLength += 16;
            ulLength -= 16;
            ptrStart += 16;
            ptrIn += (16 / sizeof(unsigned long));
        }
    }
}
    #else
static void fnDoOnTheFlyDecryption(unsigned char *ptrStart, unsigned char *ptrDestination, unsigned long ulLength, int iChannel)
{
    unsigned char *ptrStartOfDecryp;
    unsigned char *ptrEndOfDecryp;
    unsigned long *ptrKey;
    unsigned long *ptrNonce;
    unsigned long ulKeySelect;
    if (IS_POWERED_UP(4, BEE_CLOCK) == 0) {
        _EXCEPTION("BEE powered down!!");
    }
    switch (iChannel) {
    case 0:
        ptrStartOfDecryp = (unsigned char *)(IOMUXC_GPR_GPR18);          // start address
        ptrKey = BEE_AES_KEY0_W0_ADD;
        ptrNonce = BEE_CTR_NONCE0_ADD;
        ptrEndOfDecryp = (unsigned char *)(IOMUXC_GPR_GPR19);            // end address
        if ((IOMUXC_GPR_GPR11 & IOMUXC_GPR_GPR11_BEE_DE_RX_EN_0) == 0) {
            _EXCEPTION("Channel not enabled!");
        }
        ulKeySelect = uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2;
        break;
    case 1:
        ptrStartOfDecryp = (unsigned char *)(IOMUXC_GPR_GPR20);          // start address
        ptrKey = BEE_AES_KEY0_W0_ADD;
        ptrNonce = BEE_CTR_NONCE1_ADD;
        ptrEndOfDecryp = (unsigned char *)(IOMUXC_GPR_GPR21);            // end address
        if ((IOMUXC_GPR_GPR11 & IOMUXC_GPR_GPR11_BEE_DE_RX_EN_1) == 0) {
            _EXCEPTION("Channel not enabled!");
        }
        ulKeySelect = uFUSE_BOOT_CFG2_BEE_KEY1_SEL_SW_GP2;
        break;
    default:
        _EXCEPTION("OTFAD channel no supported!");
        break;
    }
    if ((BEE_CTRL & (BEE_CTRL_BEE_ENABLE | BEE_CTRL_KEY_VALID)) != ((BEE_CTRL_BEE_ENABLE | BEE_CTRL_KEY_VALID))) {
        _EXCEPTION("BEE not enabled!");
    }
    if ((BEE_CTRL & BEE_CTRL_CTRL_AES_MODE_R0) == 0) {
        _EXCEPTION("AES128 CTR mode expected!");
    }
    if ((ptrStart < ptrStartOfDecryp) || ((ptrStart + ulLength) > ptrEndOfDecryp)) {
        _EXCEPTION("Area not in decryption mode!!!");
        return ;
    }
    else {
        unsigned long *ptrIn = (unsigned long *)fnGetFlashRealAdd(ptrStart); // get a pointer to the simulated unencrypted flash
        unsigned char *ptrDecrypted = ptrDestination;
        unsigned char ucNonceCtr[16];
        unsigned char ucKey[16];
        unsigned long ulTemp;
        int iLength = 0;
        if ((HW_OCOTP_CFG5 & ulKeySelect) != 0) {                        // SW_GP2 is being used as key
            fnGetSW_GP2(ucKey);
        }
        else {
            ucKey[15] = (unsigned char)*ptrKey;
            ucKey[14] = (unsigned char)(*ptrKey >> 8);
            ucKey[13] = (unsigned char)(*ptrKey >> 16);
            ucKey[12] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[11] = (unsigned char)*ptrKey;
            ucKey[10] = (unsigned char)(*ptrKey >> 8);
            ucKey[9] = (unsigned char)(*ptrKey >> 16);
            ucKey[8] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[7] = (unsigned char)*ptrKey;
            ucKey[6] = (unsigned char)(*ptrKey >> 8);
            ucKey[5] = (unsigned char)(*ptrKey >> 16);
            ucKey[4] = (unsigned char)(*ptrKey >> 24);
            ptrKey++;
            ucKey[3] = (unsigned char)*ptrKey;
            ucKey[2] = (unsigned char)(*ptrKey >> 8);
            ucKey[1] = (unsigned char)(*ptrKey >> 16);
            ucKey[0] = (unsigned char)(*ptrKey >> 24);
        }
        fnAES_Init((AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_RESET_IV), (const unsigned char *)ucKey, 128); // register the key   

        if (*ptrNonce != 0x00000000) {
            _EXCEPTION("First nonce long word should be 0 for correct operation");
        }
        ptrNonce++;                                                      // first register is not used
        ucNonceCtr[11] = (unsigned char)*ptrNonce;
        ucNonceCtr[10] = (unsigned char)(*ptrNonce >> 8);
        ucNonceCtr[9] = (unsigned char)(*ptrNonce >> 16);
        ucNonceCtr[8] = (unsigned char)(*ptrNonce >> 24);
        ptrNonce++;
        ucNonceCtr[7] = (unsigned char)*ptrNonce;
        ucNonceCtr[6] = (unsigned char)(*ptrNonce >> 8);
        ucNonceCtr[5] = (unsigned char)(*ptrNonce >> 16);
        ucNonceCtr[4] = (unsigned char)(*ptrNonce >> 24);
        ptrNonce++;
        ucNonceCtr[3] = (unsigned char)*ptrNonce;
        ucNonceCtr[2] = (unsigned char)(*ptrNonce >> 8);
        ucNonceCtr[1] = (unsigned char)(*ptrNonce >> 16);
        ucNonceCtr[0] = (unsigned char)(*ptrNonce >> 24);

        // Decrypt the area using the defined keys
        //
        while (ulLength >= 16) {                                         // overall length is expected to be divisible by 16
            ulTemp = (((unsigned long)ptrStart) & ~0x3);                 // address
            ulTemp >>= 4;
            ucNonceCtr[15] = (unsigned char)ulTemp;
            ucNonceCtr[14] = (unsigned char)(ulTemp >> 8);
            ucNonceCtr[13] = (unsigned char)(ulTemp >> 16);
            ucNonceCtr[12] = (unsigned char)(ulTemp >> 24);
            fnAES_Init(AES_COMMAND_AES_PRIME_IV, (const unsigned char *)ucNonceCtr, 0); // prime the initial vector
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT_128, (const unsigned char *)ptrIn, (ptrDecrypted + iLength), 16); // decrypt the flash content
            iLength += 16;
            ulLength -= 16;
            ptrStart += 16;
            ptrIn += (16 / sizeof(unsigned long));
        }
    }
}
    #endif
#endif

#if defined CAN_INTERFACE
static int iLastTxBuffer;

typedef struct stTCP_MESSAGE                                             // definition of a data frame structure
{
  //TCP_HEADER     tTCP_Header;                                          // reserve header space
    unsigned char  ucTCP_Message[50];                                    // space for content
} TCP_MESSAGE;

#define SIM_CAN_TX_OK           0
#define SIM_CAN_TX_REMOTE_OK    1
#define SIM_CAN_TX_FAIL         2
#define SIM_CAN_TX_REMOTE_FAIL  3
#define SIM_CAN_RX_READY        4


static void fnBufferSent(int iBuffer, int iRemote)
{
#if defined MSCAN_CAN_INTERFACE
#else
    int iChannel = (iBuffer >> 24);                                      // extract the channel number
    KINETIS_CAN_BUF *ptrMessageBuffer;
    KINETIS_CAN_CONTROL *ptrCAN_control;

    iBuffer &= 0x00ffffff;                                               // remove channel number
    switch (iChannel) {
    case 0:
        ptrMessageBuffer = MBUFF0_ADD_0;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrMessageBuffer = MBUFF0_ADD_1;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrMessageBuffer = MBUFF0_ADD_2;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        break;
    #endif
    }
    ptrMessageBuffer += iBuffer;

    switch (iRemote) {        
    case SIM_CAN_TX_OK:
        if (((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) == MB_TX_SEND_ONCE) && (ptrMessageBuffer->ulCode_Len_TimeStamp & RTR)) { // sending remote frame
            ptrMessageBuffer->ulCode_Len_TimeStamp = (((ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD) | MB_RX_EMPTY) | (ptrCAN_control->CAN_TIMER & 0x0000ffff)); // convert to temporary rx message buffer
        }
        else {
            ptrMessageBuffer->ulCode_Len_TimeStamp = (((ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD) | MB_TX_INACTIVE) | (ptrCAN_control->CAN_TIMER & 0x0000ffff));
        }
        break;
    case SIM_CAN_TX_FAIL:
        {
            int x;
            for (x = 0; x < MAX_TX_CAN_TRIES; x++) {                     // simulate frame transmission failing
                ptrCAN_control->CAN_ESR1 |= (CAN_ACK_ERR | TXWRN | CAN_BUS_IDLE | CAN_ERROR_PASSIVE | ERRINT);
                if ((ptrCAN_control->CAN_CTRL1 & ERRMSK) != 0) {         // only generate interrupt when interrupt is enabled
                    switch (iChannel) {
                    case 0:
                        if (fnGenInt(irq_CAN0_MESSAGE_ID) != 0) {        // if FLEXCAN1 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_CAN0();    // call the interrupt handler
                        }
                        break;
                    #if NUMBER_OF_CAN_INTERFACES > 1
                    case 1:
                        if (fnGenInt(irq_CAN1_MESSAGE_ID) != 0) {        // if FLEXCAN2 interrupt is not disabled
                            VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                            ptrVect->processor_interrupts.irq_CAN1();    // call the interrupt handler
                        }
                        break;
                    #endif
                    }
                }
            }
        }
        break;
    case SIM_CAN_TX_REMOTE_OK:                                           // don't clear the message buffer on remote transmissions
    case SIM_CAN_RX_READY:
        break;
    }
    switch (iBuffer) {                                                   // call transmission complete interrupt
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        ptrCAN_control->CAN_IFLAG1 |= (1 << iBuffer);                    // set interrupt flag for the buffer
        if (ptrCAN_control->CAN_IMASK1 & (1 << iBuffer)) {               // check whether the interrupt is enabled on this buffer
            switch (iChannel) {
            case 0:
                if (fnGenInt(irq_CAN0_MESSAGE_ID) != 0) {                // if FLEXCAN1 interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_CAN0();            // call the interrupt handler
                }
                break;
    #if NUMBER_OF_CAN_INTERFACES > 1
            case 1:
                if (fnGenInt(irq_CAN1_MESSAGE_ID) != 0) {                // if FLEXCAN2 interrupt is not disabled
                    VECTOR_TABLE *ptrVect = (VECTOR_TABLE *)VECTOR_TABLE_OFFSET_REG;
                    ptrVect->processor_interrupts.irq_CAN1();            // call the interrupt handler
                }
                break;
    #endif
            }
        }
        break;
    default:                                                             // this happens when something is received
        break;
    }
#endif
}

    #if !defined _LOCAL_SIMULATION && defined SIM_KOMODO
// Komodo has received something on the CAN bus
//
static void fnCAN_reception(int iChannel, unsigned char ucDataLength, unsigned char *ptrData, unsigned long ulId, int iExtendedAddress, int iRemodeRequest, unsigned short usTimeStamp, Komodo km)
{
#if defined MSCAN_CAN_INTERFACE
#else
    KINETIS_CAN_BUF *ptrMessageBuffer;
    KINETIS_CAN_CONTROL *ptrCAN_control;
    int i = 0;
    int iRxAvailable = 0;
    int iOverrun = 0;
    unsigned char ucBuffer;
    if ((ucDataLength == 0) && (iRemodeRequest == 0)) {
        return;                                                          // ignore when reception without data
    }
    switch (iChannel) {
    case 0:
        ptrMessageBuffer = MBUFF0_ADD_0;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrMessageBuffer = MBUFF0_ADD_1;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrMessageBuffer = MBUFF0_ADD_2;
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        break;
    #endif
    }

    if (iExtendedAddress == 0) {
        ulId <<= CAN_STANDARD_SHIFT;
        ulId &= CAN_STANDARD_BITMASK;
    }

    while (i < NUMBER_CAN_MESSAGE_BUFFERS) {
        if (ptrMessageBuffer->ulID == ulId) {
            iRxAvailable++;
            if (iRemodeRequest != 0) {                                   // remote request being received
                if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) == MB_TX_SEND_ON_REQ) { // remote message waiting to be sent
                    int iResult;
                    km_can_packet_t pkt;
                    unsigned long arb_count = 0;
                    unsigned char ucTxDataLength;
                    unsigned char ucData[8];
                    pkt.id = ptrMessageBuffer->ulID;
                    // Send the CAN frame via remote simulator
                    // 
                    pkt.remote_req = 0;          
                    if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                        pkt.id |= CAN_EXTENDED_ID;                       // the address is to be handled as extended
                        pkt.extend_addr = 1;
                    }   
                    else {
                        pkt.id >>= CAN_STANDARD_SHIFT;                   // the address if a standard address
                        pkt.extend_addr = 0;
                    }
                    ucTxDataLength = (unsigned char)((ptrMessageBuffer->ulCode_Len_TimeStamp >> 16) & 0x0f);
                    pkt.dlc = ucTxDataLength;
                    // Convert from long word, big-endian format
                    //
                    if (ucTxDataLength != 0) {
                        ucData[0] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 24);
                        if (ucTxDataLength > 1) {
                            ucData[1] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 16);
                            if (ucTxDataLength > 2) {
                                ucData[2] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 8);
                                if (ucTxDataLength > 3) {
                                    ucData[3] = (unsigned char)(ptrMessageBuffer->ulData[0]);
                                    if (ucTxDataLength > 4) {
                                        ucData[4] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 24);
                                        if (ucTxDataLength > 5) {
                                            ucData[5] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 16);
                                            if (ucTxDataLength > 6) {
                                                ucData[6] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 8);
                                                if (ucTxDataLength > 7) {
                                                    ucData[7] = (unsigned char)(ptrMessageBuffer->ulData[1]);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    km_can_async_submit(km, iChannel, KM_CAN_ONE_SHOT, &pkt, ucTxDataLength, (const unsigned char *)ucData); // send
                    iResult = km_can_async_collect(km, 10, &arb_count);  // collect the result of the last transmission
                    switch (iResult) {
                    case KM_OK:
                        fnBufferSent(((iChannel << 24) | i), SIM_CAN_TX_REMOTE_OK);
                        break;
                    default:
                        fnBufferSent(((iChannel << 24) | i), SIM_CAN_TX_FAIL);
                        break;
                    }
                    return;
                }
            }
            else if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) == MB_RX_EMPTY) {
                ucBuffer = (unsigned char)i;                             // we use this buffer for reception since it suits
                break;
            }
        }
        ptrMessageBuffer++;
        i++;
    }

    switch (iChannel) {
    case 0:
        ptrMessageBuffer = MBUFF0_ADD_0;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrMessageBuffer = MBUFF0_ADD_1;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrMessageBuffer = MBUFF0_ADD_2;
        break;
    #endif
    }

    if (iRxAvailable != 0) {
        i = 0;
        while (i < NUMBER_CAN_MESSAGE_BUFFERS) {
            if (ptrMessageBuffer->ulID == ulId) {
                if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) == (MB_RX_FULL)) {
                    ucBuffer = (unsigned char)i;                         // we use this buffer for reception - it will set overrun...
                    iOverrun = 1;
                    break;
                }
            }
            ptrMessageBuffer++;
            i++;
        }
    }
    else {
        return;                                                          // no reception buffer found so ignore
    }

    switch (iChannel) {
    case 0:
        ptrMessageBuffer = MBUFF0_ADD_0;
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrMessageBuffer = MBUFF0_ADD_1;
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrMessageBuffer = MBUFF0_ADD_2;
        break;
    #endif
    }
    ptrMessageBuffer += ucBuffer;                                        // set the local simulate buffer correspondingly

    if (iOverrun != 0) {
        ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_RX_OVERRUN | (ptrMessageBuffer->ulCode_Len_TimeStamp & ~(CAN_CODE_FIELD | CAN_LENGTH_AND_TIME)));
    }
    else {
        ptrMessageBuffer->ulCode_Len_TimeStamp = ((MB_RX_FULL) | (ptrMessageBuffer->ulCode_Len_TimeStamp & ~(CAN_CODE_FIELD | CAN_LENGTH_AND_TIME))); // clear out code, length and time stamp fields
    }

    ptrMessageBuffer->ulCode_Len_TimeStamp |= usTimeStamp;
    ptrMessageBuffer->ulCode_Len_TimeStamp |= (ucDataLength << 16);
    ptrMessageBuffer->ulID = ulId;

    i = 0;
    // Save in long word, big-endian format
    //
    ptrMessageBuffer->ulData[0] = ((*(ptrData) << 24) | (*(ptrData + 1) << 16) | (*(ptrData + 2) << 8) | (*(ptrData + 3)));
    ptrMessageBuffer->ulData[1] = ((*(ptrData + 4) << 24) | (*(ptrData + 5) << 16) | (*(ptrData + 6) << 8) | (*(ptrData + 7)));
    fnBufferSent((ucBuffer | (iChannel << 24)), SIM_CAN_RX_READY);       // generate interrupt
#endif
}
    #endif

#if defined CAN_INTERFACE
extern void fnSimulateCanIn(int iChannel, unsigned long ilID, int iRTR, unsigned char *ptrData, unsigned char ucDLC)
{
    static unsigned short usTimeStamp = 0;
    unsigned long ilID_extended;
    if ((ilID & 0x80000000) != 0) {
        ilID_extended = ilID & ~0x80000000;
        ilID = 0;
    }
    else {
        ilID_extended = 0;
    }
    fnCAN_reception(iChannel, ucDLC, ptrData, ilID, ilID_extended, iRTR, usTimeStamp++, 0);
}
#endif

extern void fnSimCAN(int iChannel, int iBufferNumber, int iSpecial)
{
#if defined MSCAN_CAN_INTERFACE
#else
    KINETIS_CAN_BUF *ptrMessageBuffer;
    KINETIS_CAN_CONTROL *ptrCAN_control;
    #if !defined _LOCAL_SIMULATION
        #if defined USE_TELNET && defined UTASKER_SIM
    unsigned short usTxDataLength;
    TCP_MESSAGE tcp_message;
        #elif defined SIM_KOMODO
    unsigned char ucTxDataLength;
    static Komodo km = -1;
        #endif
    #endif

    #if defined SIM_KOMODO
    if (CAN_SIM_CHECK_RX == iSpecial) {
        u08 data_in[8];
        km_can_packet_t pkt;
        km_can_info_t   info;
        pkt.remote_req  = 0;
        pkt.extend_addr = 0;
        pkt.dlc         = 8;
        pkt.id          = 0xff;                                          // promiscuous

        if (km < 0) {
            return;
        }
        (CAN0_TIMER)++;                                                  // normally these only count when CAN controller is enabled and they would count at the CAN bit rate
        #if NUMBER_OF_CAN_INTERFACES > 1
        (CAN1_TIMER)++;
        #endif
        while ((km_can_read(km, &info, &pkt, 8, data_in)) >= 0) {        // has something been received?
            fnCAN_reception(info.channel, pkt.dlc, data_in, pkt.id, pkt.extend_addr, pkt.remote_req, (unsigned short)(info.timestamp), km);
        }
        return;
    }
    else if (CAN_SIM_TERMINATE == iSpecial) {
        if (km < 0) {
            return;
        }
        km_disable(km);
        km_close(KOMODO_USB_PORT);
        return;
    }
    #endif
    switch (iChannel) {
    case 0:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN0_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_0;                                 // the first of 16 message buffers in the FlexCan module
        break;
    #if NUMBER_OF_CAN_INTERFACES > 1
    case 1:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN1_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_1;                                 // the first of 16 message buffers in the FlexCan module
        break;
    #endif
    #if NUMBER_OF_CAN_INTERFACES > 2
    case 2:
        ptrCAN_control = (KINETIS_CAN_CONTROL *)CAN2_BASE_ADD;
        ptrMessageBuffer = MBUFF0_ADD_2;                                 // the first of 16 message buffers in the FlexCan module
        break;
    #endif
    }

    // Configuration changes
    //
    if ((ptrCAN_control->CAN_MCR & (CAN_FRZ | CAN_HALT)) != 0x00) {
        return;                                                          // off so don't do anything
    }

    #if defined USE_TELNET && defined UTASKER_SIM
    if (SimSocket == -1) {
        #if defined IMMEDIATE_MEMORY_ALLOCATION                          // extra parameter for tx buffer size to immediately allocate (uses default size)
        SimSocket = fnStartTelnet(usSimPort, 0xffff, 0, 0, 0, fnCANListener);
        #else
        SimSocket = fnStartTelnet(usSimPort, 0xffff, 0, 0, fnCANListener);
        #endif
        fnTelnet(SimSocket, TELNET_RAW_MODE);                            // set to RAW operating mode
        fnTCP_Connect(SimSocket, ucSimIP, usSimPort, 0, 0);              // attempt to establish a connection to remote  server
        return;
    }
    #elif defined SIM_KOMODO
    if (CAN_SIM_INITIALISE == iSpecial) {
        if (km < 0) {
            km = km_open(KOMODO_USB_PORT);                               // open a link to the Komodo via USB (channel 0 uses CAN A, channel 1 uses CAN B)
        }
        if (km >= 0) {
            int iCAN_speed = 0;
            if ((CLK_SRC_PERIPH_CLK & ptrCAN_control->CAN_CTRL1) != 0) { // CAN clock derived from external clock/crystal (lowest jitter)
                iCAN_speed = ((_EXTERNAL_CLOCK/25)/(ptrCAN_control->CAN_CTRL1 >> 24)); // assume that time quanta is 25
            }
            if (iChannel != 0) {                                         // CAN B
                km_disable(km);
                km_acquire(km, (KM_FEATURE_CAN_B_CONFIG | KM_FEATURE_CAN_B_CONTROL | KM_FEATURE_CAN_B_LISTEN)); // acquire features
                km_timeout(km, KM_TIMEOUT_IMMEDIATE);
                km_can_bitrate(km, KM_CAN_CH_B, iCAN_speed);             // set bitrate
              //km_can_target_power(km, KM_CAN_CH_B, 1);                 // set target power
                km_enable(km);
              //km_can_target_power(km, KM_CAN_CH_B, 0);                 // disable target power
            }
            else {                                                       // CAN A
                km_disable(km);
                km_acquire(km, (KM_FEATURE_CAN_A_CONFIG | KM_FEATURE_CAN_A_CONTROL | KM_FEATURE_CAN_A_LISTEN)); // acquire features
                km_timeout(km, KM_TIMEOUT_IMMEDIATE);
                km_can_bitrate(km, KM_CAN_CH_A, iCAN_speed);             // set bitrate
              //km_can_target_power(km, KM_CAN_CH_A, 1);                 // set target power
                km_enable(km);
              //km_can_target_power(km, KM_CAN_CH_A, 0);                 // disable target power
            }
        }
    }
    #endif

    ptrMessageBuffer += iBufferNumber;                                   // move to the buffer

    // Transmission
    //
    switch (ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) {
    case MB_TX_INACTIVE:
        // Assume that we were previously active - for example sending remote frames - disable the HW simulator
        //
    #if !defined _LOCAL_SIMULATION
        #if defined USE_TELNET && defined UTASKER_SIM
        if (fnGetCANOwner(iChannel, iBufferNumber) != TASK_CAN_SIM) {
            if (iSpecial == CAN_SIM_FREE_BUFFER) {
                tcp_message.ucTCP_Message[0] = 'd';
            }
            else  {
                tcp_message.ucTCP_Message[0] = 'D';
            }
            tcp_message.ucTCP_Message[1] = (unsigned char)iBufferNumber; // the buffer number
            fnSendBufTCP(SimSocket, (unsigned char *)&tcp_message.ucTCP_Message, 2, TCP_BUF_SEND); // send data to HW simulator
        }
        #elif defined SIM_KOMODO
        if (fnGetCANOwner(iChannel, iBufferNumber) != TASK_CAN_SIM) {
            if (iSpecial == CAN_SIM_FREE_BUFFER) {
                // Nothing to do in this case
                //
            }
            else  {
                unsigned char data[3] = {0x01, 0x02, 0x03};
                km_can_packet_t pkt;
                pkt.remote_req   = 0;
                pkt .extend_addr = 1;
                pkt.dlc          = 3;
                pkt.id           = 0x105;
                km_can_async_submit(km, iChannel, 0, &pkt, pkt.dlc, data);
            }
        }
        #endif
    #endif
        break;

    case MB_TX_SEND_ON_REQ:                                              // this buffer containes a queued message to be sent on a Remote Frame
    #if defined _LOCAL_SIMULATION
        // fall through
    #endif
    case MB_TX_SEND_ONCE:                                           
        if ((ptrMessageBuffer->ulCode_Len_TimeStamp & RTR) != 0) {       // remote frame is to be transmitted
    #if defined _LOCAL_SIMULATION
        #if defined _TX_OK
            // The buffer converts automatically to a receive buffer
            //
            ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_RX_FULL | RTR | (0x03 << 16));
            ptrMessageBuffer->ucData[0] = 0x01;                          // receive some dummy data
            ptrMessageBuffer->ucData[1] = 0x02;
            ptrMessageBuffer->ucData[2] = 0x03;
            goto _rx_int;
        #else
            // The remote transmission failed
            //
            int x;
            for (x = 0; x < MAX_TX_CAN_TRIES; x++) {                     // simulate frame transmission failing
                ptrCAN_control->CAN_ESR1 |= (CAN_ACK_ERR | TXWRN | CAN_BUS_IDLE | CAN_ERROR_PASSIVE | ERRINT);
                if (ptrCAN_control->CAN_CTRL1 & ERRMSK) {                // only generate interrupt when interrupt is enabled
                    CAN_error_Interrupt();                               // simulate a number of errors
                }
            }
        #endif
    #else
            if (fnGetCANOwner(iChannel, iBufferNumber) != TASK_CAN_SIM) {// send a remote frame
        #if defined USE_TELNET && defined UTASKER_SIM
                unsigned long ulID;
                tcp_message.ucTCP_Message[0] = 'r';
                tcp_message.ucTCP_Message[1] = (unsigned char)iBufferNumber; // the buffer number

                ulID = ptrMessageBuffer->ulID;
                if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                    ulID |= CAN_EXTENDED_ID;                             // the address is to be handled as extended
                }   
                else {
                    ulID >>= CAN_STANDARD_SHIFT;                         // the address if a standard address
                }
                tcp_message.ucTCP_Message[2] = (unsigned char)(ulID >> 24);
                tcp_message.ucTCP_Message[3] = (unsigned char)(ulID >> 16);
                tcp_message.ucTCP_Message[4] = (unsigned char)(ulID >> 8);
                tcp_message.ucTCP_Message[5] = (unsigned char)(ulID);

                fnSendBufTCP(SimSocket, (unsigned char *)&tcp_message.ucTCP_Message, 6, TCP_BUF_SEND); // send data to HW simulator
        #elif defined SIM_KOMODO
                int iResult;
                km_can_packet_t pkt;
                unsigned long arb_count = 0;
                pkt.id = ptrMessageBuffer->ulID;
                // Send the CAN frame via remote simulator
                // 
                pkt.remote_req = 1;          
                if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                    pkt.id |= CAN_EXTENDED_ID;                           // the address is to be handled as extended
                    pkt.extend_addr = 1;
                }   
                else {
                    pkt.id >>= CAN_STANDARD_SHIFT;                       // the address if a standard address
                    pkt.extend_addr = 0;
                }
                iLastTxBuffer = iBufferNumber;
                iBufferNumber = 0;
                ucTxDataLength = (unsigned char)((ptrMessageBuffer->ulCode_Len_TimeStamp >> 16) & 0x0f);
                pkt.dlc = ucTxDataLength;
                km_can_async_submit(km, iChannel, KM_CAN_ONE_SHOT, &pkt, 0, 0); // send
                iResult = km_can_async_collect(km, 10, &arb_count);      // collect the result of the last transmission
                switch (iResult) {
                case KM_OK:
                    fnBufferSent(((iChannel << 24) | iLastTxBuffer), SIM_CAN_TX_OK);
                    break;
                default:
                    fnBufferSent(((iChannel << 24) | iLastTxBuffer), SIM_CAN_TX_FAIL);
                    break;
                }
        #endif
            }
            else {
                ptrMessageBuffer->ulCode_Len_TimeStamp = (MB_RX_FULL | RTR | (0x03 << 16)); // the buffer converts automatically to a receive buffer
                ptrMessageBuffer->ulData[0] = 0x01020300;                // receive some dummy data

                fnBufferSent(iBufferNumber, SIM_CAN_TX_REMOTE_OK);
            }
    #endif
        }
        else {                                                           // this buffer contains a message to be transmitted once
    #if defined _LOCAL_SIMULATION                                        // simple simulation for simple testing of driver
        #if defined _TX_OK
            ptrMessageBuffer->ulCode_Len_TimeStamp = ((ptrMessageBuffer->ulCode_Len_TimeStamp & ~CAN_CODE_FIELD) | MB_TX_INACTIVE);
_rx_int:
            CAN_IFLAG1 |= (0x00000001 << iBufferNumber);                 // set interrupt flag
            if (CAN_IMASK1 & (0x00000001 << iBufferNumber)) {
                CAN_buf_Interrupt(iBufferNumber);                        // call interrupt routine
            }
        #else
            int x;
            for (x = 0; x < MAX_TX_CAN_TRIES; x++) {                     // simulate frame transmission failing
                ptrCAN_control->CAN_ESR1 |= (CAN_ACK_ERR | TXWRN | CAN_BUS_IDLE | CAN_ERROR_PASSIVE | ERRINT);
                if (ptrCAN_control->CAN_CTRL1 & ERRMSK) {                // only generate interrupt when interrupt is enabled
                    CAN_error_Interrupt();                               // simulate a number of errors
                }
            }
        #endif
    #else
            if (fnGetCANOwner(iChannel, iBufferNumber) != TASK_CAN_SIM) {
        #if defined USE_TELNET && defined UTASKER_SIM
                unsigned long ulID;
                // Send the CAN frame to remote simulator
                // [['T'] - BUFFER NUMBER - LENGTH - ID - DATA[...]]
                if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) ==  MB_TX_SEND_ON_REQ) {
                    tcp_message.ucTCP_Message[0] = 'R';                  // set remote transmission
                }
                else {
                    tcp_message.ucTCP_Message[0] = 'T';
                }
                tcp_message.ucTCP_Message[1] = (unsigned char)iBufferNumber; // the buffer number
                tcp_message.ucTCP_Message[2] = (unsigned char)((ptrMessageBuffer->ulCode_Len_TimeStamp >> 16) & 0x0f);
                ulID = ptrMessageBuffer->ulID;
                if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                    ulID |= CAN_EXTENDED_ID;                             // the address is to be handled as extended
                }   
                else {
                    ulID >>= CAN_STANDARD_SHIFT;                         // the address if a standard address
                }
                tcp_message.ucTCP_Message[3] = (unsigned char)(ulID >> 24);
                tcp_message.ucTCP_Message[4] = (unsigned char)(ulID >> 16);
                tcp_message.ucTCP_Message[5] = (unsigned char)(ulID >> 8);
                tcp_message.ucTCP_Message[6] = (unsigned char)(ulID);

                iLastTxBuffer = iBufferNumber;

                iBufferNumber = 0;
                usTxDataLength = tcp_message.ucTCP_Message[2] + 7;
                while (iBufferNumber++ < tcp_message.ucTCP_Message[2]) {
                    tcp_message.ucTCP_Message[iBufferNumber+6] = ptrMessageBuffer->ucData[iBufferNumber-1];
                }
                fnSendBufTCP(SimSocket, (unsigned char *)&tcp_message.ucTCP_Message, usTxDataLength, TCP_BUF_SEND); // send data to HW simulator
        #elif defined SIM_KOMODO
                int iResult;
                km_can_packet_t pkt;
                unsigned long arb_count = 0;
                unsigned char ucData[8];
                pkt.id = ptrMessageBuffer->ulID;
                // Send the CAN frame via remote simulator
                // 
                if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) == MB_TX_SEND_ON_REQ) {
                    return;                                              // the message is not sent in this case but will be sent on any remote frame request receptions
                }
                else {
                    pkt.remote_req = 0;
                }             
                if (ptrMessageBuffer->ulCode_Len_TimeStamp & IDE) {
                    pkt.id |= CAN_EXTENDED_ID;                           // the address is to be handled as extended
                    pkt.extend_addr = 1;
                }   
                else {
                    pkt.id >>= CAN_STANDARD_SHIFT;                       // the address if a standard address
                    pkt.extend_addr = 0;
                }

                iLastTxBuffer = iBufferNumber;
                iBufferNumber = 0;
                ucTxDataLength = (unsigned char)((ptrMessageBuffer->ulCode_Len_TimeStamp >> 16) & 0x0f);
                pkt.dlc = ucTxDataLength;
                // Convert from long word, big-endian format
                //
                if (ucTxDataLength != 0) {
                    ucData[0] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 24);
                    if (ucTxDataLength > 1) {
                        ucData[1] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 16);
                        if (ucTxDataLength > 2) {
                            ucData[2] = (unsigned char)(ptrMessageBuffer->ulData[0] >> 8);
                            if (ucTxDataLength > 3) {
                                ucData[3] = (unsigned char)(ptrMessageBuffer->ulData[0]);
                                if (ucTxDataLength > 4) {
                                    ucData[4] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 24);
                                    if (ucTxDataLength > 5) {
                                        ucData[5] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 16);
                                        if (ucTxDataLength > 6) {
                                            ucData[6] = (unsigned char)(ptrMessageBuffer->ulData[1] >> 8);
                                            if (ucTxDataLength > 7) {
                                                ucData[7] = (unsigned char)(ptrMessageBuffer->ulData[1]);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                km_can_async_submit(km, iChannel, KM_CAN_ONE_SHOT, &pkt, ucTxDataLength, (const unsigned char *)ucData); // send
                iResult = km_can_async_collect(km, 10, &arb_count);      // collect the result of the last transmission
iResult = KM_OK;
                switch (iResult) {
                case KM_OK:
                    fnBufferSent(((iChannel << 24) | iLastTxBuffer), SIM_CAN_TX_OK);
                    break;
                default:
                    fnBufferSent(((iChannel << 24) | iLastTxBuffer), SIM_CAN_TX_FAIL);
                    break;
                }
        #endif
            }
            else {
                if ((ptrMessageBuffer->ulCode_Len_TimeStamp & CAN_CODE_FIELD) ==  MB_TX_SEND_ON_REQ) {
                    fnBufferSent(((iChannel << 24) | iBufferNumber), SIM_CAN_TX_REMOTE_OK);
                }
                else {
                    fnBufferSent(((iChannel << 24) | iBufferNumber), SIM_CAN_TX_OK);
                }
                return;
            }
    #endif
        }
        break;
    default:
        break;
    }
#endif
}
#endif

#if defined SUPPORT_TOUCH_SCREEN
    #if defined TOUCH_FT5406
        #define TOUCH_SCREEN_PEN_COUNT    5
    #else
        #define TOUCH_SCREEN_PEN_COUNT    1
    #endif
static int iPenLocationX[TOUCH_SCREEN_PEN_COUNT] = {0};                  // last sample ADC input value
static int iPenLocationY[TOUCH_SCREEN_PEN_COUNT] = {0};
static int iPenDown[TOUCH_SCREEN_PEN_COUNT] = {0};

    #if defined K70F150M_12M
        #define MIN_X_TOUCH          0x0230                              // tuned values - for calibration these should be taken from parameters
        #define MAX_X_TOUCH          0x0e90
        #define MIN_Y_TOUCH          0x03c5
        #define MAX_Y_TOUCH          0x0d30
    #elif defined TOUCH_FT6206
        #define MIN_X_TOUCH          0x0230                              // tuned values - for calibration these should be taken from parameters
        #define MAX_X_TOUCH          0x0e90
        #define MIN_Y_TOUCH          0x03c5
        #define MAX_Y_TOUCH          0x0d30
    #endif

extern void fnSimulateFT5406(int iPen, int iPenEvent, int iX, int iY);

extern void fnPenPressed(int iX, int iY, int iPenRef)
{
    iPenLocationX[iPenRef] = (iX);
    iPenLocationY[iPenRef] = (iY);
    iPenDown[iPenRef] = 1;
    #if defined TOUCH_FT5406
    fnSimulateFT5406(iPenRef, 1, iPenLocationX[iPenRef], iPenLocationY[iPenRef]);
    #endif
}

extern void fnPenMoved(int iX, int iY, int iPenRef)
{
    iPenLocationX[iPenRef] = (iX);
    iPenLocationY[iPenRef] = (iY);
    #if defined TOUCH_FT5406
    fnSimulateFT5406(iPenRef, 0, iPenLocationX[iPenRef], iPenLocationY[iPenRef]);
    #endif
}

extern void fnPenLifted(int iPenRef)
{
    iPenLocationX[iPenRef] = 0;
    iPenLocationY[iPenRef] = 0;
    iPenDown[iPenRef] = 0;
    #if defined TOUCH_FT5406
    fnSimulateFT5406(iPenRef, -1, 0, 0);
    #endif
}

    #if ! defined TOUCH_FT5406
extern void fnGetPenSamples(unsigned short *ptrX, unsigned short *ptrY)
{
    if (iPenLocationX[0] != 0) {
        iPenLocationX[0] = iPenLocationX[0];
    }
    if (iPenDown[0] == 0) {                                              // if the pen is not applied a high voltage is expected - higher than the normal maximum pressed value
        *ptrX = (MAX_X_TOUCH + 1);
        *ptrY = (MAX_Y_TOUCH + 1);
    }
    else {
        *ptrX = (MIN_X_TOUCH + ((iPenLocationX[0] * ((MAX_X_TOUCH - MIN_X_TOUCH)))/GLCD_X));
        *ptrY = (MIN_Y_TOUCH + ((iPenLocationY[0] * ((MAX_Y_TOUCH - MIN_Y_TOUCH)))/GLCD_Y));
    }
}
    #endif
#endif

// Calculate a bit-banded peripheral address back to its original register and bit - then set the bit in the register
//
extern void fnSetBitBandPeripheralValue(unsigned long *bit_band_address)
{
    unsigned long ulRegAddress;
    unsigned long ulBit;
    ulRegAddress = ((unsigned long)bit_band_address - 0x42000000);
    ulBit = ((ulRegAddress / 4) % 32);
    ulBit = (1 << ulBit);
    ulRegAddress /= 32;
    ulRegAddress &= ~0x3;
    *(unsigned long *)ulRegAddress |= ulBit;
}

// Calculate a bit-banded peripheral address back to its original register and bit - then clear the bit in the register
//
extern void fnClearBitBandPeripheralValue(unsigned long *bit_band_address)
{
    unsigned long ulRegAddress;
    unsigned long ulBit;
    ulRegAddress = ((unsigned long)bit_band_address - 0x42000000);
    ulBit = ((ulRegAddress / 4) % 32);
    ulBit = (1 << ulBit);
    ulRegAddress /= 32;
    ulRegAddress &= ~0x3;
    *(unsigned long *)ulRegAddress &= ~ulBit;
}

// Calculate a bit-banded peripheral address back to its original register and bit - then check the bit in the register
//
extern int fnCheckBitBandPeripheralValue(unsigned long *bit_band_address)
{
    unsigned long ulRegAddress;
    unsigned long ulBit;
    ulRegAddress = ((unsigned long)bit_band_address - 0x42000000);
    ulBit = ((ulRegAddress / 4) % 32);
    ulBit = (1 << ulBit);
    ulRegAddress /= 32;
    ulRegAddress &= ~0x3;
    return ((*(unsigned long *)ulRegAddress & ulBit) != 0);
}


extern unsigned long *fnGetRegisterAddress(unsigned long ulAddress)
{
    ulAddress -= 0xe000e000;
    ulAddress += (unsigned long)CORTEX_M7_BLOCK;
    return (unsigned long *)ulAddress;
}

extern void fnSetReg(int iRef, unsigned long ulValue)
{
    switch (iRef) {
    case 0:
        iMX.CORTEX_M7_REGS.ulR0 = ulValue;
        break;
    case 14:
        iMX.CORTEX_M7_REGS.ulPSP = ulValue;
        break;
    case 15:
        iMX.CORTEX_M7_REGS.ulMSP = ulValue;
        break;
    case 19:
        iMX.CORTEX_M7_REGS.ulPRIMASK = ulValue;
        break;
    case 20:
        iMX.CORTEX_M7_REGS.ulFAULTMASK = ulValue;
        break;
    case 22:
        iMX.CORTEX_M7_REGS.ulCONTROL = ulValue;
        break;
    }
}

// Prepare a string to be displayed in the simulator status bar
//
extern void fnUpdateOperatingDetails(void)
{
#if !defined NO_STATUS_BAR
    extern void fnPostOperatingDetails(char *ptrDetails);
    unsigned long ulDivide;
    unsigned long ulSourceSpeed;
    CHAR cOperatingDetails[256];
    CHAR *ptrBuffer = cOperatingDetails;
    ptrBuffer = uStrcpy(ptrBuffer, "SRAM = ");
    ptrBuffer = fnBufferDec((SIZE_OF_RAM/1024), 0, ptrBuffer);
    if ((CCM_CBCDR & CCM_CBCDR_PERIPH_CLK2_SEL_PERIPH_CLK2_CLK) != 0) {  // PERIPH_CLK selected from PERIPH_CLK2
    #if defined CCM_CBCDR_PERIPH_CLK2_PODF_MASK
        ulDivide = (((CCM_CBCDR & CCM_CBCDR_PERIPH_CLK2_PODF_MASK) >> 27) + 1); // source divide
    #else
        ulDivide = 1;
    #endif
        switch (CCM_CBCMR & CCM_CBCMR_PERIPH_CLK2_SEL_MASK) {
        case CCM_CBCMR_PERIPH_CLK2_SEL_PLL3_SW_CLK:
            ulSourceSpeed = PLL3_SW_CLK;
            break;
        case CCM_CBCMR_PERIPH_CLK2_SEL_OSC_CLK:
            ulSourceSpeed = _EXTERNAL_CLOCK;
            break;
        case CCM_CBCMR_PERIPH_CLK2_SEL_PLL2_BYPASS_CLK:
            _EXCEPTION("To do");
            break;
        default:
            _EXCEPTION("Bad PERIPH_CLK2 source");
            break;
        }
        ulSourceSpeed /= ulDivide;                                       // PERIPH_CLK speed
    }
    else {                                                               // PERIPH_CLK selected from PRE_PERIPH_CLK
        switch (CCM_CBCMR & CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK) {
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL2:                          // PRE_PERIPH clock derived from PLL2
            if ((CCM_ANALOG_PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS) != 0) { // if bypassed
                ulSourceSpeed = _EXTERNAL_CLOCK;
            }
            else {
                ulSourceSpeed = iMX_PLL2;
            }
            break;
    #if defined ARM_PLL_AVAILABLE
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL1:
            ulSourceSpeed = iMX_PLL1;
            ulSourceSpeed /= (((CCM_CACRR & CCM_CACRR_ARM_PODF_DIV8) + 1));
            break;
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL2_PFD0:                     // PRE_PERIPH clock derived from PLL3 PFD0
            if ((CCM_ANALOG_PLL_USB1 & CCM_ANALOG_PLL_SYS_BYPASS) != 0) {// if bypassed
                ulSourceSpeed = _EXTERNAL_CLOCK;
            }
            else {
                ulSourceSpeed = (unsigned long)((unsigned long long)((unsigned long long)iMX_PLL2 * (unsigned long long)18) / (CCM_ANALOG_PFD_480 & CCM_ANALOG_PFD_480_PFD0_FRACT_MASK));
            }
            break;
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL2_PFD2:                     // PRE_PERIPH clock derived from PLL2 PFD2
            if ((CCM_ANALOG_PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS) != 0) { // if bypassed
                ulSourceSpeed = _EXTERNAL_CLOCK;
            }
            else {
                ulSourceSpeed = (unsigned long)((unsigned long long)((unsigned long long)iMX_PLL2 * (unsigned long long)18)/((CCM_ANALOG_PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRACT_MASK) >> 16));
            }
            break;
    #else
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL3_PFD3:                     // PRE_PERIPH clock derived from PLL3 PFD3
            if ((CCM_ANALOG_PLL_USB1 & CCM_ANALOG_PLL_USB1_BYPASS) != 0) { // if bypassed
                ulSourceSpeed = _EXTERNAL_CLOCK;
            }
            else {
                ulSourceSpeed = (unsigned long)((unsigned long long)((unsigned long long)iMX_PLL3 * (unsigned long long)18) / ((CCM_ANALOG_PFD_480 & CCM_ANALOG_PFD_480_PFD3_FRACT_MASK) >> 24));
            }
            break;
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL2_PFD3:                     // PRE_PERIPH clock derived from PLL2 PFD3
            if ((CCM_ANALOG_PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS) != 0) { // if bypassed
                ulSourceSpeed = _EXTERNAL_CLOCK;
            }
            else {
                ulSourceSpeed = (unsigned long)((unsigned long long)((unsigned long long)iMX_PLL2 * (unsigned long long)18)/((CCM_ANALOG_PFD_528 & CCM_ANALOG_PFD_528_PFD3_FRACT_MASK) >> 24));
            }
            break;
        case CCM_CBCMR_PRE_PERIPH_CLK_SEL_PLL6:                          // PRE_PERIPH clock derived from divided PLL6
            ulSourceSpeed = iMX_PLL6;
        #if defined CCM_CACRR
            ulSourceSpeed /= ((CCM_CACRR & CCM_CACRR_ARM_PODF_DIV8) + 1);
        #endif
            break;
    #endif
        }
    }
    // AHB divide
    //
    ulDivide = (((CCM_CBCDR & CCM_CBCDR_AHB_PODF_MASK) >> 10) + 1);      // AHB_CLK_ROOT divide
    ulSourceSpeed /= ulDivide;                                           // core clock
    ptrBuffer = uStrcpy(ptrBuffer, " Core clock = ");
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);
    ptrBuffer = uStrcpy(ptrBuffer, "MHz IPG clock = ");
    ulSourceSpeed /= (((CCM_CBCDR & CCM_CBCDR_IPG_PODF_MASK) >> 8) + 1);
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);
    ptrBuffer = uStrcpy(ptrBuffer, "MHz PER clock = ");
    if ((CCM_CSCMR1 & CCM_CSCMR1_PERCLK_SEL_OSC_CLK) != 0) {
        ulSourceSpeed = _EXTERNAL_CLOCK;
    }
    ulSourceSpeed /= ((CCM_CSCMR1 & CCM_CSCMR1_PERCLK_PODF_MASK) + 1);
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);
    ptrBuffer = uStrcpy(ptrBuffer, " SEMC clock = ");
    if ((CCM_CBCDR & CCM_CBCDR_SEMC_CLK_SEL_SEMC) != 0) {                // if SEMC alternative clock is used as SEMC clock root
        if ((CCM_CBCDR & CCM_CBCDR_SEMC_ALT_CLK_SEL_PLL3_PFD1) != 0) {   // if SEMC alternative clock selected as PLL3 PFD1
            ulSourceSpeed = ((((iMX_PLL3 / 10) * 18) / PLL3_PFD1_FRACTION) * 10);
        }
        else {                                                           // SEMC alternative clock selected as PLL2 PFD2
            ulSourceSpeed = ((((iMX_PLL2 / 10) * 18) / PLL2_PFD2_FRACTION) * 10);
        }
    }
    else {
        ulSourceSpeed = PERIPH_CLK_FREQUENCY;
    }
    #if defined CCM_CBCDR_SEMC_PODF_DIV8
    ulSourceSpeed /= (((CCM_CBCDR & CCM_CBCDR_SEMC_PODF_DIV8) >> 16) + 1);
    #endif
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);

    ptrBuffer = uStrcpy(ptrBuffer, " FLEXSPI clock = ");
    switch (CCM_CSCMR1 & CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0) {
    #if defined iMX_RT1011
    case CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL2:
        ulSourceSpeed = iMX_PLL2;
        break;
    #else
    case CCM_CSCMR1_FLEXSPI_CLK_SEL_SEMC:                                // FLEXSPI clock route is sourced from SEMC_CLK_ROOT
        break;
    #endif
    case CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_SW_CLK:                         // FLEXSPI clock route is sourced from pll3_sw_clk
        ulSourceSpeed = PLL3_SW_CLK;
        break;
    case CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL2_PFD2:                           // FLEXSPI clock route is sourced from PLL2-PDF2
        ulSourceSpeed = ((((iMX_PLL2 / 10) * 18) / PLL2_PFD2_FRACTION) * 10);
        break;
    case CCM_CSCMR1_FLEXSPI_CLK_SEL_PLL3_PFD0:                           // FLEXSPI clock route is sourced from PLL3-PDF0
        ulSourceSpeed = ((((iMX_PLL3 / 10) * 18) / PLL3_PFD0_FRACTION) * 10);
        break;
    }
    ulSourceSpeed /= (((CCM_CSCMR1 & CCM_CSCMR1_FLEXSPI_PODF_MASK) >> 23) + 1);
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);

    ptrBuffer = uStrcpy(ptrBuffer, " LPSPI clock = ");
    switch (CCM_CBCMR & CCM_CBCMR_LPSPI_CLK_SEL_MASK) {
    case CCM_CBCMR_LPSPI_CLK_SEL_PLL2:                                   // LPSPI clock route is sourced from PLL2
        ulSourceSpeed = iMX_PLL2;
        break;
    case CCM_CBCMR_LPSPI_CLK_SEL_PLL3_PFD0:                              // LPSPI clock route is sourced from pll3_pfd0
        ulSourceSpeed = ((((iMX_PLL3 / 10) * 18) / PLL3_PFD0_FRACTION) * 10);
        break;
    case CCM_CBCMR_LPSPI_CLK_SEL_PLL3_PFD1:                              // LPSPI clock route is sourced from pll3_pfd1
        ulSourceSpeed = ((((iMX_PLL3 / 10) * 18) / PLL3_PFD1_FRACTION) * 10);
        break;
    case CCM_CBCMR_LPSPI_CLK_SEL_PLL2_PFD2:                              // LPSPI clock route is sourced from pll2_pdf2
        ulSourceSpeed = ((((iMX_PLL2 / 10) * 18) / PLL2_PFD2_FRACTION) * 10);
        break;
    }
    ulSourceSpeed /= (((CCM_CBCMR & CCM_CBCMR_LPSPI_PODF_MASK) >> 26) + 1);
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);

    #if defined SDCARD_SUPPORT
    ptrBuffer = uStrcpy(ptrBuffer, " USDHC1 clock = ");
    if ((CCM_CSCMR1 & CCM_CSCMR1_USDHC1_CLK_SEL_PLL2_PFD0) != 0) {
        ulSourceSpeed = ((((iMX_PLL2 / 10) * 18) / PLL2_PFD0_FRACTION) * 10);
    }
    else {
        ulSourceSpeed = ((((iMX_PLL2 / 10) * 18) / PLL2_PFD2_FRACTION) * 10);
    }
    ulSourceSpeed /= (((CCM_CSCDR1 & CCM_CSCDR1_USDHC1_PODF_8) >> 11) + 1);
    ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);
    #endif
    #if defined ENHANCED_LCD_INTERFACE && defined SUPPORT_GLCD
        #if defined LCDIF_CLK_SOURCE_PLL5
    {
        double nominator = (double)CCM_ANALOG_PLL_VIDEO_NUM;
        double denominator = (double)CCM_ANALOG_PLL_VIDEO_DENOM;
        if ((CCM_ANALOG_PLL_VIDEO_NUM == 0) || (CCM_ANALOG_PLL_VIDEO_DENOM == 0)) {
            nominator = 0;
        }
        else {
            nominator /= denominator;
        }
        nominator += (CCM_ANALOG_PLL_VIDEO & CCM_ANALOG_PLL_VIDEO_DIV_SELECT_MASK);
        nominator *= OSC_CLK;                                            // pll5 output
        switch (CCM_ANALOG_MISC2 & CCM_ANALOG_MISC2_VIDEO_DIV_MASK) {
        case CCM_ANALOG_MISC2_VIDEO_DIV_1:
            break;
        case CCM_ANALOG_MISC2_VIDEO_DIV_2:
            nominator /= 2;
            break;
        case CCM_ANALOG_MISC2_VIDEO_DIV_4:
            nominator /= 4;
            break;
        default:
            _EXCEPTION("Invalid CCM_ANALOG_MISC2_VIDEO_DIV_MASK value!");
            break;
        }
        switch (CCM_ANALOG_PLL_VIDEO & CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT) {
        case CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT_1:
            break;
        case CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT_2:
            nominator /= 2;
            break;
        case CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT_4:
            nominator /= 4;
            break;
        default:
            _EXCEPTION("Invalid CCM_ANALOG_PLL_VIDEO_DIV_SELECT_MASK value!");
            break;
        }
        ulSourceSpeed = (unsigned long)nominator;
        ulSourceSpeed /= (((CCM_CSCDR2 & CCM_CSCDR2_LCDIF_PRED_MASK) >> 12) + 1);
        ulSourceSpeed /= (((CCM_CBCMR & CCM_CBCMR_LCDIF_PODF_MASK) >> 23) + 1);
        ptrBuffer = uStrcpy(ptrBuffer, " LCDIF clock = ");
        ptrBuffer = fnBufferDec(ulSourceSpeed, 0, ptrBuffer);
    }
        #endif
    #endif
    fnPostOperatingDetails(cOperatingDetails);
#endif
}
#endif
