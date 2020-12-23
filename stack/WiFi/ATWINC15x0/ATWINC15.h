/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      ATWINC15.h
    Project:   Single Chip Embedded Internet - interface between uTasker and ATWINC15 wifi module
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#if defined WIFI_INTERFACE && defined WIFI_ATWINC15

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "driver/include/m2m_wifi.h"
#include "driver/source/m2m_hif.h"
#include "driver/source/nmasic.h"
#include "driver/include/m2m_periph.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static tstrM2mRev strtmp = { 0 };
static unsigned char MACaddr[6];
static int uTaskerEnableDebug = 0;
static unsigned char winc_rx_buf[(ETH_MTU + ETH_HEADER_LEN + 2)];        // space includes CRC];
static unsigned char winc_tx_buf[(ETH_MTU + ETH_HEADER_LEN + 2)];


static unsigned char *fnGetWIFI_TxBufferAdd(int iBufNr)
{
    return winc_tx_buf;
}

static void fnPutInBuffer_WIFI(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes)
{

}

static int fnWaitWIFI_TxFree(void)
{
    return 0;
}

// Transmit a frame over WIFI interface
//
static QUEUE_TRANSFER fnStartEthTxWIFI(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO3, 1); // turn LED on
uTaskerEnableDebug = 1;
    if (M2M_SUCCESS == m2m_wifi_send_ethernet_pkt(winc_tx_buf, DataLen)) {
uTaskerEnableDebug = 0;
        m2m_periph_gpio_set_val(M2M_PERIPH_GPIO3, 0); // turn LED off
        return DataLen;
    }
uTaskerEnableDebug = 0;
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1); // turn LED on to indicate failure
    return 0;
}

static int fnGetQuantityWIFIRxBuf(void)
{
    return 0;
}

static void fnFreeEthernetBufferWIFI(int iBufNr)
{
}

    #if defined USE_IGMP
static void fnModifyMulticastFilterWIFI(QUEUE_TRANSFER action, unsigned char *ptrIP)
{
}
    #endif


#if defined _WINDOWS && !defined REMOTE_SIMULATION_INTERFACE             // simulation of the wifi device                                 
static int iWIFI_assert = 0;
static int iWIFI_Tx_cnt = 0;
static int iWIFI_Rx_cnt = 0;
static int iWIFI_Rx_Response = 0;
static unsigned char ucPayload = 0;
static unsigned long ulAddress = 0;
static unsigned long ulData = 0;
static unsigned long ulCount = 0;
static unsigned char ucWIFI_Tx[100];
static unsigned char ucWIFI_Rx[100] = { 0 };
static int iF3_command = 0;

static unsigned long ulE824 = 0x0000002e;
static unsigned long ul1000 = 0x001003a0;                                // chip ID
static unsigned long ul1014 = 0x807c082d;                                // 0x80000000 flag means ROM has loaded
static unsigned long ul207bc = 0x00003f00;                               // wait for host reg
static unsigned long ulc0008 = 0x000044dc;
static unsigned long ulc000c = 0x10add09e;                               // BOOTROM_REG
static unsigned long ul106c = 0x00000000;                                // WIFI_HOST_RCV_CTRL_3
static unsigned long ul1070 = 0x00000031;                                // WIFI_HOST_RCV_CTRL_1
static unsigned long ul1078 = 0x00000000;                                // WIFI_HOST_RCV_CTRL_2
static unsigned long ul1084 = 0x0003d6e0;
static unsigned long ul108c = 0x00000000;                                // NMI_STATE_REG
static unsigned long ul13f4 = 0x00000001;
static unsigned long ul1408 = 0x30000044;
static unsigned long ul14a0 = 0x00000000;                                // rNMI_GP_REG_1
static unsigned long ul1a00 = 0x00000000;
static unsigned long ul150400 = 0x0003d6e0;                              // WIFI_HOST_RCV_CTRL_4
static unsigned long ul20100 = 0x00000000;                               // GPIO data register
static unsigned long ul20108 = 0x00000000;                               // GPIO DDR register
static unsigned long ul344dc[8 / 4] = { 0xeeeb95e4, 0x0000be5c };
static unsigned long ul395e4[2] = { 0xad05f0f8,0x000068a4 };               // MAC address
static unsigned long ul3be5c[0x28 / 4] = {
    0x001503a0,
    0x13010613,
    0x614d0003,
    0x33322079,
    0x31303220,
    0x34310038,
    0x3a39333a,
    0x00003631,
    0x00004179,
    0x00000000
};
static unsigned long ul3d6e0[2] = {
    0, 0
};
static unsigned long ul3d6e8[100] = {0};
static unsigned long ul3d702[1600] = {0};                                // tx buffer

static void fnWiFiRdWr(unsigned long ulAddress, int iWrite, unsigned char *ptrBuffer, unsigned long ulData, unsigned long ulCount)
{
    unsigned long *ptrReg = 0;
    switch (ulAddress) {
    case 0xe824:
        ptrReg = &ulE824;
        break;
    case 0x1000:
        ptrReg = &ul1000;
        break;
    case 0x1014:
        ptrReg = &ul1014;
        break;
    case 0x106c:
        ptrReg = &ul106c;
        break;
    case 0x1070:
        ptrReg = &ul1070;
        break;
    case 0x1078:
        ptrReg = &ul1078;
        if (iWrite == 0) {
            static int iDelay = 0;
            if (++iDelay >= 5) {
                ul1078 = 0;                                              // NBIT1 goes low after a short delay
                iDelay = 0;
            }
        }
        break;
    case 0x1084:
        ptrReg = &ul1084;
        break;
    case 0x108c:
        ptrReg = &ul108c;
        if (iWrite == 0) {
            static int iDelay = 0;
            if (++iDelay >= 5) {
                ul108c = 0x02532636;
                iDelay = 0;
            }
        }
        break;
    case 0x13f4:
        ptrReg = &ul13f4;
        break;
    case 0x1408:
        ptrReg = &ul1408;
        break;
    case 0x14a0:
        ptrReg = &ul14a0;
        break;
    case 0x1a00:
        ptrReg = &ul1a00;
        break;
    case 0x150400:
        ptrReg = &ul150400;
        break;
    case 0x207bc:
        ptrReg = &ul207bc;
        break;
    case 0x20100:
        ptrReg = &ul20100;
        break;
    case 0x20108:
        ptrReg = &ul20108;
        break;
    case 0x0c0008:
        ptrReg = &ulc0008;
        break;
    case 0x0c000c:
        ptrReg = &ulc000c;
        break;
    case 0x344dc:
        ptrReg = &ul344dc[0];
        break;
    case 0x395e4:
        ptrReg = &ul395e4[0];
        break;
    case 0x3be5c:
        ptrReg = &ul3be5c[0];
        break;
    case 0x3d6e0:
        ptrReg = &ul3d6e0[0];
        break;
    case 0x3d6e8:
        ptrReg = &ul3d6e8[0];
        break;
    case 0x3d702:
        ptrReg = &ul3d702[0];
        break;
    default:
        _EXCEPTION("To do!!");
        return;
    }
    if (iWrite != 0) {
        *ptrReg = ulData;
        while (ulCount > 4) {
            ptrReg++;
            ulData = (unsigned char)*ptrBuffer++;
            ulData <<= 8;
            ulData |= (unsigned char)*ptrBuffer++;
            ulData <<= 8;
            ulData |= (unsigned char)*ptrBuffer++;
            ulData <<= 8;
            ulData |= (unsigned char)*ptrBuffer++;
            ulCount -= 4;
            *ptrReg = ulData;
        }
    }
    else {
        *ptrBuffer++ = (unsigned char)*ptrReg;
        *ptrBuffer++ = (unsigned char)(*ptrReg >> 8);
        *ptrBuffer++ = (unsigned char)(*ptrReg >> 16);
        *ptrBuffer++ = (unsigned char)(*ptrReg >> 24);
        while (ulCount > 4) {
            ptrReg++;
            *ptrBuffer++ = (unsigned char)*ptrReg;
            *ptrBuffer++ = (unsigned char)(*ptrReg >> 8);
            *ptrBuffer++ = (unsigned char)(*ptrReg >> 16);
            *ptrBuffer++ = (unsigned char)(*ptrReg >> 24);
            ulCount -= 4;
        }
    }
}

static void fnWiFiTxCommand(int iCRC, unsigned char *ptr_ucWIFI_Tx, int iWIFI_Tx_cnt)
{
    if (iCRC != 0) {
        iWIFI_Tx_cnt--;
    }
    switch (iWIFI_Tx_cnt) {
    case (3 + 1):
        ucWIFI_Rx[0] = *ptr_ucWIFI_Tx++;
        ucWIFI_Rx[1] = 0x00;
        ucWIFI_Rx[2] = 0xf3;                                             // WINC acknowledges the command by sending three bytes
        iWIFI_Rx_cnt = 0;
        iWIFI_Rx_Response = (3 + 4);
        ulAddress = *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx;

        switch (ucWIFI_Rx[0]) {
        case 0xca:                                                       // DMA single word read
            fnWiFiRdWr(ulAddress, 0, &ucWIFI_Rx[3], 0, 4);
            if (ulE824 == 0x0000002e) {                                  // if CRC enabled
                ucWIFI_Rx[7] = 0x11;                                     // add CRC16
                ucWIFI_Rx[8] = 0xd4;
                iWIFI_Rx_Response += 2;
            }
            break;
        }
        break;
    case (5 + 1):
        break;
    case (6 + 1):
        ucWIFI_Rx[0] = *ptr_ucWIFI_Tx++;
        ucWIFI_Rx[1] = 0x00;
        ucWIFI_Rx[2] = 0xf3;                                             // WINC acknowledges the command by sending three bytes
        iWIFI_Rx_cnt = 0;
        iWIFI_Rx_Response = (3 + 4);
        ulAddress = *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx++;
        ulCount = *ptr_ucWIFI_Tx++;
        ulCount <<= 8;
        ulCount |= *ptr_ucWIFI_Tx++;
        ulCount <<= 8;
        ulCount |= *ptr_ucWIFI_Tx;

        switch (ucWIFI_Rx[0]) {
        case 0xc7:                                                       // DMA extended write
            iF3_command = 1;
            break;
        case 0xc8:                                                       // DMA extended read
            fnWiFiRdWr(ulAddress, 0, &ucWIFI_Rx[3], 0, ulCount);
            break;
        }
        break;
    case (7 + 1):
        ucWIFI_Rx[0] = *ptr_ucWIFI_Tx++;
        ucWIFI_Rx[1] = 0x00;                                             // WINC acknowledges the command by sending two bytes
        iWIFI_Rx_cnt = 0;
        iWIFI_Rx_Response = 2;
        ulAddress = *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx++;
        ulAddress <<= 8;
        ulAddress |= *ptr_ucWIFI_Tx++;
        ulData = *ptr_ucWIFI_Tx++;
        ulData <<= 8;
        ulData |= *ptr_ucWIFI_Tx++;
        ulData <<= 8;
        ulData |= *ptr_ucWIFI_Tx++;
        ulData <<= 8;
        ulData |= *ptr_ucWIFI_Tx;
        switch (ucWIFI_Rx[0]) {
        case 0xc9:                                                       // DMA single word write
            fnWiFiRdWr(ulAddress, 1, &ucWIFI_Rx[3], ulData, 4);
            break;
        }
        break;
    default:
        if (*ptr_ucWIFI_Tx++ == 0xf3) {
            ulData = *ptr_ucWIFI_Tx++;
            ulData <<= 8;
            ulData |= *ptr_ucWIFI_Tx++;
            ulData <<= 8;
            ulData |= *ptr_ucWIFI_Tx++;
            ulData <<= 8;
            ulData |= *ptr_ucWIFI_Tx++;
            fnWiFiRdWr(ulAddress, 1, ptr_ucWIFI_Tx, ulData, ulCount);    // DMA extended write
            ucWIFI_Rx[0] = 0;
            ucWIFI_Rx[1] = 0xc3;
            ucWIFI_Rx[2] = 0x0;                                          // WINC acknowledges the command by sending three bytes
            iWIFI_Rx_cnt = 0;
            iWIFI_Rx_Response = (3);
        }
        break;
    }
}

static void fnSimulteWIFI(unsigned char data)
{
    if (iWIFI_assert != 0) {
        if ((iWIFI_Tx_cnt == 0) && (iF3_command != 0) && (ucWIFI_Tx[0] == 0xf3)) {
            iWIFI_Tx_cnt = 1;
            iF3_command = 0;
        }
        switch (iWIFI_Tx_cnt) {
        case 0:
            ucWIFI_Tx[0] = data;
            switch (data) {
            case 0x00:                                                   // used to recognise a read
                LPSPI1_RDR = ucWIFI_Rx[iWIFI_Rx_cnt++];
                return;
            case 0xc1:                                                   // DMA write transaction
                ucPayload = 5;                                           // address [23:0] followed by count [15:0]
                break;
            case 0xc2:                                                   // DMA read transaction
                ucPayload = 5;                                           // address [23:0] followed by count [15:0]
                break;
            case 0xc3:                                                   // internal register write
                ucPayload = 6;                                           // offset [15:0] followed by data [31:0]
                break;
            case 0xc4:                                                   // internal register read
                ucPayload = 3;                                           // address [15:0] follows + 0
                break;
            case 0xc5:                                                   // transaction termination
                ucPayload = 3;                                           // 3 x 0
                break;
            case 0xc6:                                                   // repeat data packet command
                ucPayload = 3;                                           // 3 x 0
                break;
            case 0xc7:                                                   // DMA extended write transaction
                ucPayload = 6;                                           // address [23:0] followed by count [23:0]
                break;
            case 0xc8:                                                   // DMA extended read transaction
                ucPayload = 6;                                           // address [23:0] followed by count [23:0]
                break;
            case 0xc9:                                                   // DMA single word write
                ucPayload = 7;                                           // address [23:0] followed by data [31:0]
                break;
            case 0xca:                                                   // DMA single word read
                ucPayload = 3;                                           // address [23:0]
                break;
            case 0xcf:                                                   // soft reset command
                ucPayload = 3;                                           // 3 x 0xff
                break;
            case 0xf3:                                                   // unknown!
                ucPayload = (unsigned char)ulCount;
                if (iF3_command == 0) {
                    _EXCEPTION("Unexpected!");
                }
                break;
            default:
                _EXCEPTION("To do");
                break;
            }
            iWIFI_Tx_cnt++;
            break;
        default:
            ucWIFI_Tx[iWIFI_Tx_cnt++] = data;
            if (ucPayload == 0) {
                fnWiFiTxCommand(1, ucWIFI_Tx, iWIFI_Tx_cnt);
            }
            if (--ucPayload == 0) {
                // Payload received
                //
                if (ulE824 != 0x0000002e) {                              // if CRC has been disabled
                    fnWiFiTxCommand(0, ucWIFI_Tx, iWIFI_Tx_cnt);
                }
            }
            break;
        }
        LPSPI1_RDR = 0;
    }
}

#define SIMULATE_WIFI(data)    fnSimulteWIFI(data)
#define SIMULATE_WIFI_ASSERT() iWIFI_assert = 1
#define SIMULATE_WIFI_NEGATE() iWIFI_assert = 0; iWIFI_Tx_cnt = 0
#else
    #define SIMULATE_WIFI(data)
    #define SIMULATE_WIFI_ASSERT()
    #define SIMULATE_WIFI_NEGATE()
#endif

void nm_bsp_cs_low(void)
{
    SIMULATE_WIFI_ASSERT();
    ASSERT_CS_WIFI();
}

void nm_bsp_cs_high(void)
{
    NEGATE_CS_WIFI();
    SIMULATE_WIFI_NEGATE();
}

// If there is a wifi reception frame waiting return a frame pointer
//
static CHAR fnEthernetMuxEvent(ETHERNET_FRAME *ptrFrame, int *ptr_iEthernetInterface)
{
    return -1;                                                           // no frame waiting
}


void winc_netif_rx_callback(uint8 msg_type, void * msg, void *ctrl_buf)
{
    fnDebugMsg("winc_netif_rx_callback");
}

//-----------------------------------------------------------------------------
// Callback to get the Wi-Fi status update
//
// \param[in] u8MsgType type of Wi-Fi notification. Possible types are:
//  - [M2M_WIFI_RESP_CURRENT_RSSI](@ref M2M_WIFI_RESP_CURRENT_RSSI)
//  - [M2M_WIFI_RESP_CON_STATE_CHANGED](@ref M2M_WIFI_RESP_CON_STATE_CHANGED)
//  - [M2M_WIFI_RESP_CONNTION_STATE](@ref M2M_WIFI_RESP_CONNTION_STATE)
//  - [M2M_WIFI_RESP_SCAN_DONE](@ref M2M_WIFI_RESP_SCAN_DONE)
//  - [M2M_WIFI_RESP_SCAN_RESULT](@ref M2M_WIFI_RESP_SCAN_RESULT)
//  - [M2M_WIFI_REQ_WPS](@ref M2M_WIFI_REQ_WPS)
//  - [M2M_WIFI_RESP_IP_CONFIGURED](@ref M2M_WIFI_RESP_IP_CONFIGURED)
//  - [M2M_WIFI_RESP_IP_CONFLICT](@ref M2M_WIFI_RESP_IP_CONFLICT)
//  - [M2M_WIFI_RESP_P2P](@ref M2M_WIFI_RESP_P2P)
//  - [M2M_WIFI_RESP_AP](@ref M2M_WIFI_RESP_AP)
//  - [M2M_WIFI_RESP_CLIENT_INFO](@ref M2M_WIFI_RESP_CLIENT_INFO)
// 		\param[in] pvMsg A pointer to a buffer containing the notification parameters
// 		(if any). It should be casted to the correct data type corresponding to the
// 		notification type. Existing types are:
//  - tstrM2mWifiStateChanged
//  - tstrM2MWPSInfo
//  - tstrM2MP2pResp
//  - tstrM2MAPResp
//  - tstrM2mScanDone
//  - tstrM2mWifiscanResult
//-----------------------------------------------------------------------------
void winc_command_cb(uint8_t u8MsgType, void *pvMsg)
{
    static int scan_request_index = 0;
    static int found = 0;
    switch (u8MsgType) {
    case M2M_WIFI_RESP_SCAN_DONE:                                        // the ATWINC15 has completed a scan for available devices - it may inform that there was nothing found or that there are a certain number available
    {
        tstrM2mScanDone *pstrInfo = (tstrM2mScanDone *)pvMsg;
        fnDebugMsg("SCAN DONE\r\n");
        fnDebugMsg("State = ");
        fnDebugDec(pstrInfo->s8ScanState, WITH_CR_LF);
        if (pstrInfo->u8NumofCh >= 1) {
            fnDebugMsg("Found = ");
            found = pstrInfo->u8NumofCh;
            fnDebugDec(pstrInfo->u8NumofCh, WITH_CR_LF);
            m2m_wifi_req_scan_result(scan_request_index);                // use the device index as reference to request its details
            scan_request_index++;
        }
        else {
            m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
            fnDebugMsg("Next scan...");
        }
    }
    break;

    case M2M_WIFI_RESP_SCAN_RESULT:                                      // the results of a specific found device are being received
    {
        tstrM2mWifiscanResult *pstrScanResult = (tstrM2mWifiscanResult *)pvMsg;

        fnDebugMsg("Scan result: Index =");
        fnDebugDec(scan_request_index, WITH_SPACE);
        fnDebugMsg(" SSID ");
        fnDebugMsg((CHAR *)pstrScanResult->au8SSID);
        fnDebugMsg(" RSSI ");
        fnDebugDec((signed long)(pstrScanResult->s8rssi), DISPLAY_NEGATIVE);
        fnDebugMsg("dBm\r\n");

        if (scan_request_index < found) {
            m2m_wifi_req_scan_result(scan_request_index);
            scan_request_index++;
        }
        else {
            fnDebugMsg("Scan finished !\r\n");
        }
    }
    break;

    case M2M_WIFI_RESP_CON_STATE_CHANGED:
    {
        tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
        fnDebugMsg("State changed:\r\n");
        fnDebugMsg("Code = ");
        fnDebugHex(pstrWifiState->u8ErrCode, (sizeof(pstrWifiState->u8ErrCode) | WITH_CR_LF | WITH_LEADIN));
        if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
            fnDebugMsg("CONNECTED\r\n");
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);                // turn LED on
            m2m_wifi_req_curr_rssi();                                    // get RSSI
        }
        else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
            fnDebugMsg("DISCONNECTED\r\n");
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 0);                // turn LED off
            if (pstrWifiState->u8ErrCode == M2M_ERR_CONN_INPROGRESS) {
                m2m_wifi_disconnect();                                   // abort a connection in progress
            }
        }
    }
    break;


    case M2M_WIFI_RESP_CURRENT_RSSI:
    {
        fnDebugMsg("RSSI ");
        fnDebugDec((signed long)(*(sint8 *)pvMsg), DISPLAY_NEGATIVE);
        fnDebugMsg("dBm\r\n");
    }
    break;

    default:
        fnDebugMsg("winc_command_cb ");
        fnDebugDec(u8MsgType, WITH_CR_LF);
        break;

    }
}



extern void fnPrintATWINC15_Info(void)
{
    fnDebugMsg("ATWIN15Cx0: ");
    if (strtmp._PAD8_ != 0) {
        CHAR cBuf[18];
        fnDebugMsg("Chip ID ");
        fnDebugHex(strtmp.u32Chipid, (sizeof(strtmp.u32Chipid) | WITH_LEADIN | WITH_CR_LF));
        fnDebugMsg("Firmware ");
        fnDebugDec(strtmp.u8FirmwareMajor, 0);
        fnDebugMsg(".");
        fnDebugDec(strtmp.u8FirmwareMinor, 0);
        fnDebugMsg(".");
        fnDebugDec(strtmp.u8FirmwarePatch, WITH_CR_LF);
        fnDebugMsg("Driver ");
        fnDebugDec(strtmp.u8DriverMajor, 0);
        fnDebugMsg(".");
        fnDebugDec(strtmp.u8DriverMinor, 0);
        fnDebugMsg(".");
        fnDebugDec(strtmp.u8DriverPatch, WITH_CR_LF);
        fnDebugMsg((CHAR *)strtmp.BuildDate);
        fnDebugMsg(" ");
        fnDebugMsg((CHAR *)strtmp.BuildTime);
        fnDebugMsg("\r\nWIFI MAC address = ");
        fnMACStr(MACaddr, cBuf);
        fnDebugMsg(cBuf);
    }
    else {
        fnDebugMsg("Not Initialised");
    }
    fnDebugMsg("\r\n");
}



// Power-up sequence involves not appling any voltages to the pins until the supply is on
// chip enable can be applied immediately after power is available
// reset should remain low for at least 5ms after the chip enable has been set high in order to ensure that the clock oscillator has stabilised
//
extern void fnReleaseWifi(void)
{
    ASSERT_RESET_WIFI();                                                 // assert the reset (if not already asserted)
    fnDelayLoop(10);                                                     // 1us is enough to reset during operation (apply for 10us)
    NEGATE_RESET_WIFI();                                                 // remove the reset so that the wifi module can start (the module has been held in reset for 50ms)
    fnDelayLoop(2000);                                                   // wait 2ms before continuing
}

static void fnInitialiseWIFI(void)
{
    tstrWifiInitParam param;
    tstrEthInitParam  ethParam;
    uMemset((unsigned char *)&param, 0, sizeof(tstrWifiInitParam));
    uMemset((unsigned char *)&ethParam, 0, sizeof(tstrEthInitParam));

    ethParam.pfAppEthCb = winc_netif_rx_callback;
    ethParam.au8ethRcvBuf = winc_rx_buf;
    ethParam.u16ethRcvBufSize = (ETH_MTU + ETH_HEADER_LEN + 2);          // space includes CRC
    ethParam.u8EthernetEnable = 1;                                       // bypass the TCPIP stack of WINC

    param.strEthInitParam = ethParam;

    // Initialise ATWINC15x0 Wi-Fi driver with data and command callback
    //
    param.pfAppWifiCb = winc_command_cb;
    m2m_wifi_init(&param);

    // Setup ATWINC15x0 GPIO for LED control
    //
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO3, 1);	                     // set as output - intially all are off
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO4, 1);	                     // set as output
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO5, 1);	                     // set as output
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO6, 1);	                     // set as output

    // Get firmware info
    //
    nm_get_firmware_full_info(&strtmp);
    strtmp._PAD8_ = 1;                                                   // mark that the module has been configured

    m2m_wifi_get_mac_address(MACaddr);                                   // get the WIFI's MAC address
    fnPrintATWINC15_Info();                                              // show module details
    m2m_wifi_request_scan(M2M_WIFI_CH_ALL);                              // start a scan of all channels
    fnDebugMsg("Scanning...\r\n");
}

static int fnConfigWiFi(ETHTABLE *pars)
{
    fnReleaseWifi();                                                     // this is called afer 50ms (at least) which means that the module has stabilised - its enable line is already asserted
    fnInitialiseWIFI();
    return 0;
}



//-----------------------------------------------------------------------------
//	@fn		nm_bsp_sleep
//	@brief	Sleep in units of msec
//	@param[IN]	u32TimeMsec
//				Time in milliseconds
//-----------------------------------------------------------------------------
extern void nm_bsp_sleep(uint32 u32TimeMsec)
{
    fnDelayLoop(u32TimeMsec * 1000);
}

extern void nm_bsp_sleep_us(uint32 u32TimeUsec)
{
    fnDelayLoop(u32TimeUsec);
}

//-----------------------------------------------------------------------------
//	@fn		nm_bsp_register_isr
//	@brief	Register interrupt service routine
//	@param[IN]	pfIsr
//				Pointer to ISR handler
//-----------------------------------------------------------------------------
extern void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier to configure port interrupt
    interrupt_setup.int_handler = (void(*)(void))pfIsr;                  // handling function
    interrupt_setup.int_priority = WIFI_IRQ_PRIORITY;                    // interrupt priority level
    interrupt_setup.int_port = WIFI_IRQ_INPUT_PORT;                      // the port that the interrupt input is on
    interrupt_setup.int_port_bits = (WIFI_IRQ_INPUT);
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON);
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
}

//-----------------------------------------------------------------------------
//	@fn		nm_bsp_interrupt_ctrl
//	@brief	Enable/Disable interrupts
//	@param[IN]	u8Enable
//				'0' disable interrupts. '1' enable interrupts
//-----------------------------------------------------------------------------
extern void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
    if (u8Enable != 0) {
        GPIO3_IMR |= WIFI_IRQ_INPUT;                                     // enable the port pin interrupt
    }
    else {
        GPIO3_IMR &= ~WIFI_IRQ_INPUT;                                    // disable the port pin interrupt
    }
}

//-----------------------------------------------------------------------------
//	@fn		nm_bus_init
//	@brief	Initialize the bus wrapper
//	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
//-----------------------------------------------------------------------------
extern sint8 nm_bus_init(void *pvinit)
{
    fnReleaseWifi();
    return M2M_SUCCESS;
}

//-----------------------------------------------------------------------------
//	@fn		nm_bus_deinit
//	@brief	De-initialize the bus wrapper
//-----------------------------------------------------------------------------
extern sint8 nm_bus_deinit(void)
{
    return M2M_SUCCESS;
}

#define NM_BUS_MAX_TRX_SZ	256

tstrNmBusCapabilities egstrNmBusCapabilities =
{
    NM_BUS_MAX_TRX_SZ
};

static unsigned char sendCharSPI1(unsigned char data)
{
    SIMULATE_WIFI(data);
    WRITE_WIFI_DATA(data);                                               // send byte of data
    WAIT_WIFI_TX_END();                                                  // wait for the byte to be completely sent (and received)
    return READ_WIFI_DATA();                                             // return the read value
}

//-----------------------------------------------------------------------------
// called by nm_bus_ioctl()
//-----------------------------------------------------------------------------

static sint8 spi_rw(uint8 *pu8Mosi, uint8 *pu8Miso, uint16 u16Sz)
{
    unsigned char ucRxData;
    if (uTaskerEnableDebug != 0) {
        if (pu8Mosi != 0) {
            fnDebugMsg("TX");
            if (pu8Miso != 0) {
                fnDebugMsg("/RX");
            }
            fnDebugMsg(":");
        }
        else {
            fnDebugMsg("RX:");
        }
    }
    nm_bsp_cs_low();                                                     // assert the wifi chip select line
    while (u16Sz-- != 0) {
        if (pu8Mosi != 0) {                                              // if transmission is defined
            if (uTaskerEnableDebug != 0) {
                fnDebugHex(*pu8Mosi, (sizeof(*pu8Mosi) | WITH_SPACE | WITH_LEADIN));
            }
            ucRxData = sendCharSPI1(*pu8Mosi++);
            if (pu8Miso != 0) {                                          // if also reception is defined
                if (uTaskerEnableDebug != 0) {
                    fnDebugHex(ucRxData, (sizeof(ucRxData) | WITH_SPACE | WITH_LEADIN));
                }
                *pu8Miso++ = ucRxData;
            }
        }
        else if (pu8Miso != 0) {                                         // if reception alone is defined
            ucRxData = sendCharSPI1(0x00);                               // read data by sending 0x00
            if (uTaskerEnableDebug != 0) {
                fnDebugHex(ucRxData, (sizeof(ucRxData) | WITH_SPACE | WITH_LEADIN));
            }
            *pu8Miso++ = ucRxData;
        }
    }
    nm_bsp_cs_high();                                                     // negate the wifi chip select line
    if (uTaskerEnableDebug != 0) {
        fnDebugMsg("\r\n");
    }
    return M2M_SUCCESS;
}

//-----------------------------------------------------------------------------
// THIS IS CALLED BY BOTH NMI_SPI_READ() AND NMI_SPI_WRITE()
//	@fn		nm_bus_ioctl
//	@brief	send/receive from the bus
//	@param[IN]	u8Cmd
//				IOCTL command for the operation
//	@param[IN]	pvParameter
//					Arbitrary parameter depending on IOCTL
//	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
//	@note	For SPI only, it's important to be able to send/receive at the same time
//-----------------------------------------------------------------------------
sint8 nm_bus_ioctl(uint8 u8Cmd, void* pvParameter)
{
    sint8 s8Ret = 0;
    switch (u8Cmd) {
    case NM_BUS_IOCTL_RW: {
        tstrNmSpiRw *pstrParam = (tstrNmSpiRw *)pvParameter;
        s8Ret = spi_rw(pstrParam->pu8InBuf, pstrParam->pu8OutBuf, pstrParam->u16Sz);
    }
                          break;
    default:
        s8Ret = -1;
        fnDebugMsg("invalid ioctl cmd\n\r");
        break;
    }
    return s8Ret;
}

extern int fnConnectWifi(void)
{
    uTaskerEnableDebug = 1;
    //m2m_wifi_connect((char *)"wifipass_Pierre_Vacances", 24, M2M_WIFI_SEC_WPA_PSK, (void *)"123456", M2M_WIFI_CH_ALL);
    m2m_wifi_connect((char *)"HUAWEI P smart 2019", 19, M2M_WIFI_SEC_WPA_PSK, (void *)"4a09f08c588d", M2M_WIFI_CH_ALL);

    uTaskerEnableDebug = 0;
    return 0;
}


extern void os_hook_isr(void)
{
    uTaskerStateChange(TASK_ETHERNET, UTASKER_ACTIVATE);                 // schedule the Ethernet task
}


extern void fnHandleWifi_Event(void)
{
  //fnDebugMsg("WIFI Event\r\n");
    //uTaskerEnableDebug = 1;
    m2m_wifi_handle_events(0);
    //uTaskerEnableDebug = 0;
}

extern void fnATWINC15_wdog(void)
{
    if (strtmp._PAD8_ != 0) {                                            // if the wifi module has been configured
        strtmp._PAD8_ ^= 0x2;
        if ((strtmp._PAD8_ & 0x2) != 0) {                                // toggle an LED to show that the module is alive
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO6, 1);                // set high
        }
        else {
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO6, 0);                // set low
        }
    }
}
#endif

