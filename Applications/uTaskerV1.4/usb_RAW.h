/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_RAW.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#if defined HID_RAW_DEFINES
    #define HID_RAW_TX_SIZE                     32
    #define HID_RAW_TX_RATE                     2                        // ms

    #define HID_RAW_RX_SIZE                     64
    #define HID_RAW_RX_RATE                     8                        // ms
#endif


#if defined HID_RAW_LOCAL_VARIABLES
    #if HID_RAW_TX_SIZE > HID_RAW_RX_SIZE
    static unsigned char ucRawData[HID_RAW_TX_SIZE];                     // static reception buffer
    #else
    static unsigned char ucRawData[HID_RAW_RX_SIZE];                     // static reception buffer so that the data remains stable when being sent (no data queue used)
    #endif
#endif

#if defined HID_RAW_FUNCTIONS
// Handle potential RAW HID reception data
//
static void fnHandleHID_RAW_reception(QUEUE_HANDLE USBPortID_HID_raw)
{
    if (fnRead(USBPortID_HID_raw, ucRawData, HID_RAW_RX_SIZE) == HID_RAW_RX_SIZE) {
        fnWrite(USBPortID_HID_raw, ucRawData, HID_RAW_RX_SIZE);          // send back
    }
}
#endif

