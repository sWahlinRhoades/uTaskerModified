/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_CDC.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    10.07.2020 Extracted CDC device and host code from usb_application.c to this file

*/


#if defined CDC_DEFINES
#endif


#if defined CDC_LOCAL_VARIABLES
    #if USB_CDC_VCOM_COUNT > 0
        static CDC_PSTN_LINE_CODING uart_setting[USB_CDC_VCOM_COUNT];    // use a static struct to ensure that non-buffered transmission remains stable. Used also to receive new settings to
        #if USB_CDC_VCOM_COUNT > 1
            #if defined SERIAL_INTERFACE
            static QUEUE_HANDLE CDCSerialPortID[USB_CDC_VCOM_COUNT] = {NO_ID_ALLOCATED};
            #endif
            static UART_MODE_CONFIG CDC_UART_SerialMode[USB_CDC_VCOM_COUNT - 1];
            static unsigned char ucCDC_UART_SerialSpeed[USB_CDC_VCOM_COUNT - 1];
        #endif
    #endif
#endif

#if defined CDC_FUNCTIONS
#if defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0) && defined USE_MAINTENANCE
static void fnResetCDC_commandLine(int iInterface)
{
    if (usUSB_state != ES_NO_CONNECTION) {                               // if the USB connection was being used for debug menu, restore previous interface
    #if defined SERIAL_INTERFACE && defined DEMO_UART                    // {8}
        DebugHandle = SerialPortID;
    #else
        DebugHandle = NETWORK_HANDLE;
    #endif
        fnGotoNextState(ES_NO_CONNECTION);
        usUSB_state = ES_NO_CONNECTION;
    }
}
#endif

#if defined USB_DEVICE_SUPPORT && defined FREEMASTER_CDC && (USB_CDC_VCOM_COUNT == 1)
#elif defined USB_DEVICE_SUPPORT && ((defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0)) && defined USE_MAINTENANCE) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && (!defined USB_RNDIS_HOST || defined TELIT_LE910C1)) // USB-CDC with command line interface
    #if !defined FREE_RTOS_USB || (USB_CDC_COUNT > 1)
// Handle potential CDC data on the VCOM interface
//
static void fnHandleCDC_reception(QUEUE_HANDLE CDC_data_handle, unsigned char ucInputMessage[LARGE_MESSAGE])
{
    QUEUE_TRANSFER Length;
    while (fnMsgs(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE]) != 0) {    // while reception from OUT endpoint on first CDC interface
        #if defined USB_CDC_HOST
            #if defined USB_DEVICE_SUPPORT                               // {48} when both modes are possible
        if (USB_HOST_MODE_OF_OPERATION == iUSB_mode[USB_DEVICE_REF]) {   // only enable IN polling when in host mode
            #endif
            fnDriver(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], (RX_ON), 0); // enable IN polling again so that further data can be received
            #if defined USB_DEVICE_SUPPORT
        }
            #endif
        #endif
        #if defined USB_CDC_INTERFACE_PASSTHROUGH
        Length = fnRead(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, LARGE_MESSAGE); // read the content
        fnWrite(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE + 1], ucInputMessage, Length); // send input to next USB-CDC interface
        #elif defined USE_MAINTENANCE
        if ((usUSB_state & (ES_USB_DOWNLOAD_MODE | ES_USB_RS232_MODE)) != 0) {
            if ((usUSB_state & ES_USB_RS232_MODE) != 0) {
            #if defined SERIAL_INTERFACE && defined DEMO_UART            // {8}
                if (fnWrite(SerialPortID, 0, MEDIUM_MESSAGE) != 0) {     // check that there is space for a block of data
                    Length = fnRead(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], ucInputMessage, MEDIUM_MESSAGE); // read the content
                    fnWrite(SerialPortID, ucInputMessage, Length);       // send input to serial port
                }
                else {
                    fnDriver(SerialPortID, MODIFY_WAKEUP, (MODIFY_TX | OWN_TASK)); // we want to be woken when the queue is free again
                    break;                                               // leave present USB data in the input buffer until we have enough serial output buffer space
                                                                         // the TX_FREE event is not explicitly handled since it is used to wake a next check of the buffer progress
                }
            #else
                fnRead(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, LARGE_MESSAGE); // read the content to empty the queue
            #endif
            }
            else {                                                       // loading mode
                Length = fnRead(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], ucInputMessage, LARGE_MESSAGE); // read the content
            #if defined ACTIVE_FILE_SYSTEM && !defined USB_CDC_HOST
                fnDownload(ucInputMessage, Length);                      // pass the input to the downloader
            #endif
            }
            continue;
        }
        else {
            Length = fnRead(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], ucInputMessage, LARGE_MESSAGE); // read the content
        }
        #else                                                            // no maintenance interface
        if (fnWrite(SerialPortID, 0, MEDIUM_MESSAGE) != 0) {             // check that there is space for a block of data
            Length = fnRead(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, MEDIUM_MESSAGE); // read the content
            fnWrite(SerialPortID, ucInputMessage, Length);               // send input to serial port
        }
        else {
            fnDriver(SerialPortID, MODIFY_WAKEUP, (MODIFY_TX | OWN_TASK)); // we want to be woken when the queue is free again
            break;                                                       // leave present USB data in the input buffer until we have enough serial output buffer space
                                                                         // the TX_FREE event is not explicitly handled since it is used to wake a next check of the buffer progress
        }
        #endif
        #if defined USB_DEVICE_SUPPORT && defined USE_USB_CDC
            #if defined USB_HOST_SUPPORT                                 // {48} when host or device modes can operate
        if (USB_DEVICE_MODE_OF_OPERATION == iUSB_mode[USB_DEVICE_REF]) { // echo input only in device mode
            #endif
            #if !defined USB_CDC_INTERFACE_PASSTHROUGH
            fnWrite(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, Length); // echo input
                #if !defined USB_CDC_ECHO
            fnHandleUSB_command_input(ucInputMessage, Length);
                #endif
            #endif
            #if defined USB_HOST_SUPPORT
        }
            #endif
        #endif
    }
}
    #endif
#endif

#if defined USE_USB_CDC || defined USB_CDC_HOST
// Called to transmit data from defined USB endpoint (serial input uses this)
//
extern QUEUE_TRANSFER fnSendToUSB(unsigned char *ptrData, QUEUE_TRANSFER Length)
{
    #if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST && !defined TELIT_LE910C1
    return 0;
    #else
        #if defined USB_CDC_HOST
    int iResult;
    if (USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE] == NO_ID_ALLOCATED) {
            #if defined USE_MAINTENANCE
        usUSB_state &= ~(ES_USB_RS232_MODE);
            #endif
        fnDebugMsg("No USB-CDC device connection!\r\n");
        return 0;
    }
    iResult = fnWrite(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], ptrData, Length); // pass the buffer on to USB-CDC interface
    fnDriver(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], (RX_ON), 0);    // enable IN polling again
    return iResult;
        #else
            #if USB_CDC_VCOM_COUNT > 0
    return (fnWrite(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ptrData, Length)); // send to USB-CDC interface
            #else
    return 0;
            #endif
        #endif
    #endif
}
#endif

#if defined USE_USB_CDC
// Set the global debug interface handle to the USB handle
//
extern void fnSetUSB_debug(void)
{
    #if USB_CDC_VCOM_COUNT > 0
    DebugHandle = USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE];           // first usb-cdc interface is used as debug interface
    #endif
}

    #if defined USB_DEVICE_SUPPORT && !defined USB_CDC_INTERFACE_PASSTHROUGH && !defined USB_CDC_ECHO
static void fnHandleUSB_command_input(unsigned char ucInputMessage[LARGE_MESSAGE], QUEUE_TRANSFER Length)
{
    if (usUSB_state == ES_NO_CONNECTION) {
        if (fnCommandInput(ucInputMessage, Length, SOURCE_USB) != 0) {
            if (fnInitiateLogin(ES_USB_LOGIN) == TELNET_ON_LINE) {
                static const CHAR ucCOMMAND_MODE_BLOCKED[] = "Command line blocked\r\n";
                fnWrite(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], (unsigned char *)ucCOMMAND_MODE_BLOCKED, (sizeof(ucCOMMAND_MODE_BLOCKED) - 1));
            }
        }
    }
    else {
        fnCommandInput(ucInputMessage, Length, SOURCE_USB);
    }
}
    #endif

    #if defined RUN_IN_FREE_RTOS && defined FREE_RTOS_USB
// Called by FreeRTOS task to get the USB interface handled
//
QUEUE_HANDLE fnGetUSB_Handle(void)
{
    #if USB_CDC_VCOM_COUNT > 1
    return USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE + 1];
    #elif USB_CDC_VCOM_COUNT > 0
    return USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE];
    #else
    return (NO_ID_ALLOCATED);
    #endif
}
    #endif

    #if defined USE_USB_CDC && defined SERIAL_INTERFACE && defined USB_SERIAL_CONNECTIONS && (USB_CDC_VCOM_COUNT > 1)
static QUEUE_HANDLE fnOpenUART(TTYTABLE *PtrInterfaceParameters, unsigned char ucDriverMode)
{
    QUEUE_HANDLE serialHandle;
        #if defined FRDM_KL27Z || defined CAPUCCINO_KL27                 // DMA transmission doesn't work on UART2 of KL27 - to be investigated
    if (PtrInterfaceParameters->Channel == 2) {
        PtrInterfaceParameters->ucDMAConfig = 0;
    }
        #endif
    if ((serialHandle = fnOpen(TYPE_TTY, ucDriverMode, PtrInterfaceParameters)) != NO_ID_ALLOCATED) { // open the channel with defined configurations (initially inactive)
        fnDriver(serialHandle, (TX_ON | RX_ON), 0);                      // enable rx and tx
        if (PtrInterfaceParameters->Config & RTS_CTS) {
            fnDriver(serialHandle, (MODIFY_INTERRUPT | ENABLE_CTS_CHANGE), 0); // activate CTS interrupt when working with HW flow control (this returns also the present control line states)
            fnDriver(serialHandle, (MODIFY_CONTROL | SET_RTS), 0 );      // activate RTS line when working with HW flow control
        }
    }
    return serialHandle;
}

static void fnOpenUSB_UARTs(int iInterface)
{
    static const QUEUE_HANDLE UART_channels[USB_CDC_VCOM_COUNT - 1] = {  // the UARTs mapped to each USB-CDC interface
        (CDC_UART_1 + 1),
        #if USB_CDC_VCOM_COUNT > 2
        (CDC_UART_2 + 1),
        #endif
        #if USB_CDC_VCOM_COUNT > 3
        (CDC_UART_3 + 1),
        #endif
        #if USB_CDC_VCOM_COUNT > 4
        (CDC_UART_4 + 1),
        #endif
        #if USB_CDC_VCOM_COUNT > 5
        (CDC_UART_5 + 1),
        #endif
        #if defined CDC_UART_6 && (USB_CDC_VCOM_COUNT > 6)
        (CDC_UART_6 + 1),
        #endif
        #if defined CDC_UART_7 && (USB_CDC_VCOM_COUNT > 7)
        (CDC_UART_7 + 1),
        #endif
        #if defined CDC_UART_8 && (USB_CDC_VCOM_COUNT > 8)
        (CDC_UART_8 + 1),
        #endif
    };
    TTYTABLE tInterfaceParameters;                                       // table for passing information to driver
    int iUART;
    tInterfaceParameters.ucSpeed = SERIAL_BAUD_115200;                   // baud rate
    tInterfaceParameters.Rx_tx_sizes.RxQueueSize = RX_BUFFER_SIZE;       // input buffer size
    tInterfaceParameters.Rx_tx_sizes.TxQueueSize = TX_BUFFER_SIZE;       // output buffer size
    tInterfaceParameters.Task_to_wake = OWN_TASK;                        // wake self when messages have been received
        #if defined SUPPORT_FLOW_HIGH_LOW
    tInterfaceParameters.ucFlowHighWater = 80;                           // set the flow control high and low water levels in %
    tInterfaceParameters.ucFlowLowWater = 20;
        #endif
    tInterfaceParameters.Config = (CHAR_8 | NO_PARITY | ONE_STOP | CHAR_MODE);
        #if defined SERIAL_SUPPORT_DMA
    tInterfaceParameters.ucDMAConfig = 0;                                // disable DMA
    tInterfaceParameters.ucDMAConfig = UART_TX_DMA;                      // activate DMA on transmission
        #endif
        #if defined SUPPORT_HW_FLOW
  //tInterfaceParameters.Config |= RTS_CTS;                              // enable RTS/CTS operation
        #endif
    #if defined USER_DEFINED_UART_RX_HANDLER                             // {50}
    tInterfaceParameters.receptionHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_RX_BREAK_DETECTION
    tInterfaceParameters.receiveBreakHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_TX_FRAME_COMPLETE
    tInterfaceParameters.txFrameCompleteHandler = 0;
    #endif
    if (iInterface == 0) {
        for (iUART = 0; iUART < (USB_CDC_VCOM_COUNT - 1); iUART++) {     // configure and open each UART attached to a USB-CDC connection
            tInterfaceParameters.Channel = (UART_channels[iUART] - 1);   // set UART channel for serial use
            if ((UART_channels[iUART] != 0) && (tInterfaceParameters.Channel < NUMBER_SERIAL)) { // only open available and mapped UART interfaces
                CDCSerialPortID[iUART + 1] = fnOpenUART(&tInterfaceParameters, FOR_I_O);
            }
            ucCDC_UART_SerialSpeed[iUART] = tInterfaceParameters.ucSpeed; // set additional interface parameters to the same as the main one
            CDC_UART_SerialMode[iUART] = tInterfaceParameters.Config;
        }
    }
    else {                                                               // modify single interface
        iInterface--;
        tInterfaceParameters.ucSpeed = ucCDC_UART_SerialSpeed[iInterface];
        tInterfaceParameters.Config = CDC_UART_SerialMode[iInterface];
        tInterfaceParameters.Channel = (UART_channels[iInterface] - 1);  // set UART channel for serial use
        CDCSerialPortID[iInterface + 1] = fnOpenUART(&tInterfaceParameters, MODIFY_CONFIG);
    }
}
    #endif
#endif

#if defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0)
// Convert UART interface settings format to CDC setting format and report to CDC host
//
static void fnReturnUART_settings(int iInterface)
{
    unsigned long ulSpeed;
    UART_MODE_CONFIG SerialMode = 0;
    unsigned char ucSerialSpeed = SERIAL_BAUD_115200;

    if (iInterface == 0) {
        ucSerialSpeed = temp_pars->temp_parameters.ucSerialSpeed;
        SerialMode = temp_pars->temp_parameters.SerialMode;
    }
    #if USB_CDC_VCOM_COUNT > 1
    else {                                                               // supplementary CDC interfaces
        ucSerialSpeed = ucCDC_UART_SerialSpeed[iInterface - 1];
        SerialMode = CDC_UART_SerialMode[iInterface - 1];
    }
    #endif

    switch (ucSerialSpeed) {                                             // the present baud rate
    case SERIAL_BAUD_300:
        ulSpeed = 300;
        break;
    case SERIAL_BAUD_600:
        ulSpeed = 600;
        break;
    case SERIAL_BAUD_1200:
        ulSpeed = 1200;
        break;
    case SERIAL_BAUD_2400:
        ulSpeed = 2400;
        break;
    case SERIAL_BAUD_4800:
        ulSpeed = 4800;
        break;
    case SERIAL_BAUD_9600:
        ulSpeed = 9600;
        break;
    case SERIAL_BAUD_14400:
        ulSpeed = 14400;
        break;
    case SERIAL_BAUD_19200:
    default:
        ulSpeed = 19200;
        break;
    case SERIAL_BAUD_38400:
        ulSpeed = 38400;
        break;
    case SERIAL_BAUD_57600:
        ulSpeed = 57600;
        break;
    case SERIAL_BAUD_115200:
        ulSpeed = 115200;
        break;
    case SERIAL_BAUD_230400:
        ulSpeed = 230400;
        break;
    case SERIAL_BAUD_250K:
        ulSpeed = 250000;
        break;
    }
    uart_setting[iInterface].dwDTERate[0] = (unsigned char)ulSpeed;
    ulSpeed >>= 8;
    uart_setting[iInterface].dwDTERate[1] = (unsigned char)ulSpeed;
    ulSpeed >>= 8;
    uart_setting[iInterface].dwDTERate[2] = (unsigned char)ulSpeed;
    ulSpeed >>= 8;
    uart_setting[iInterface].dwDTERate[3] = (unsigned char)ulSpeed;

    if ((SerialMode & CHAR_7) != 0) {                                    // set present character length
        uart_setting[iInterface].bDataBits = CDC_PSTN_7_DATA_BITS;
    }
    else {
        uart_setting[iInterface].bDataBits = CDC_PSTN_8_DATA_BITS;
    }

    if ((SerialMode & RS232_EVEN_PARITY) != 0) {                         // set present parity
        uart_setting[iInterface].bParityType = CDC_PSTN_EVEN_PARITY;
    }
    else if ((SerialMode & RS232_ODD_PARITY) != 0) {
        uart_setting[iInterface].bParityType = CDC_PSTN_ODD_PARITY;
    }
    else {
        uart_setting[iInterface].bParityType = CDC_PSTN_NO_PARITY;
    }

    if ((SerialMode & ONE_HALF_STOPS) != 0) {                            // set present format
        uart_setting[iInterface].bCharFormat = CDC_PSTN_1_5_STOP_BIT;
    }
    else if ((SerialMode & TWO_STOPS) != 0) {
        uart_setting[iInterface].bCharFormat = CDC_PSTN_2_STOP_BITS;
    }
    else {
        uart_setting[iInterface].bCharFormat = CDC_PSTN_1_STOP_BIT;
    }
    fnWrite(USB_control[USB_DEVICE_REF], (unsigned char *)&uart_setting[iInterface], sizeof(uart_setting[iInterface])); // return directly (non-buffered)
}

// Convert new UART settings that have been received from the CDC host
//
static void fnNewUART_settings(unsigned char *ptrData, unsigned short usLength, unsigned short usExpected, int iInterface)
{
    unsigned char *ptrSet = (unsigned char *)&uart_setting[iInterface];

    if (usExpected > sizeof(uart_setting[iInterface])) {                 // some protection against corrupted reception
        usExpected = sizeof(uart_setting[iInterface]);
    }
    if (usLength > usExpected) {
        usLength = usExpected;
    }

    ptrSet += (sizeof(uart_setting[iInterface]) - usExpected);
    uMemcpy(ptrSet, ptrData, usLength);

    if (usLength >= usExpected) {                                        // all data has been copied
        fnInterruptMessage(OWN_TASK, (unsigned char)(EVENT_COMMIT_NEW_UART_SETTING + iInterface)); // perform the UART modification in the task
    }
}

// Convert the CDC setting format to the UART interface settings format and modify the UART setting
//
static void fnCommit_UART(int iInterface)
{
    unsigned long ulSpeed = uart_setting[iInterface].dwDTERate[0];
    unsigned short usNewSerialMode = 0;
    unsigned char ucNewSerialSpeed;
    ulSpeed |= (uart_setting[iInterface].dwDTERate[1] << 8);
    ulSpeed |= (uart_setting[iInterface].dwDTERate[2] << 16);
    ulSpeed |= (uart_setting[iInterface].dwDTERate[3] << 24);

    if (ulSpeed > 230400) {
        ucNewSerialSpeed = SERIAL_BAUD_250K;
    }
    else if (ulSpeed > 115200) {
        ucNewSerialSpeed = SERIAL_BAUD_230400;
    }
    else if (ulSpeed > 57600) {
        ucNewSerialSpeed = SERIAL_BAUD_115200;
    }
    else if (ulSpeed > 38400) {
        ucNewSerialSpeed = SERIAL_BAUD_57600;
    }
    else if (ulSpeed > 19200) {
        ucNewSerialSpeed = SERIAL_BAUD_38400;
    }
    else if (ulSpeed > 14400) {
        ucNewSerialSpeed = SERIAL_BAUD_19200;
    }
    else if (ulSpeed > 9600) {
        ucNewSerialSpeed = SERIAL_BAUD_14400;
    }
    else if (ulSpeed > 4800) {
        ucNewSerialSpeed = SERIAL_BAUD_9600;
    }
    else if (ulSpeed > 2400) {
        ucNewSerialSpeed = SERIAL_BAUD_4800;
    }
    else if (ulSpeed > 1200) {
        ucNewSerialSpeed = SERIAL_BAUD_2400;
    }
    else if (ulSpeed > 600) {
        ucNewSerialSpeed = SERIAL_BAUD_1200;
    }
    else if (ulSpeed > 300) {
        ucNewSerialSpeed = SERIAL_BAUD_600;
    }
    else {
        ucNewSerialSpeed = SERIAL_BAUD_300;
    }

    if (uart_setting[iInterface].bDataBits == CDC_PSTN_7_DATA_BITS) {
        usNewSerialMode |= CHAR_7;
    }

    switch (uart_setting[iInterface].bParityType) {
    case CDC_PSTN_EVEN_PARITY:
        usNewSerialMode |= RS232_EVEN_PARITY;
        break;
    case CDC_PSTN_ODD_PARITY:
        usNewSerialMode |= RS232_ODD_PARITY;
        break;
    }

    switch (uart_setting[iInterface].bCharFormat) {
    case CDC_PSTN_1_5_STOP_BIT:
        usNewSerialMode |= ONE_HALF_STOPS;
        break;
    case CDC_PSTN_2_STOP_BITS:
        usNewSerialMode |= TWO_STOPS;
        break;
    }

    if (iInterface == 0) {                                               // first interface controls the debug UART interface
        temp_pars->temp_parameters.SerialMode &= ~(CHAR_7 | RS232_EVEN_PARITY | RS232_ODD_PARITY | ONE_HALF_STOPS | TWO_STOPS);
        temp_pars->temp_parameters.SerialMode |= usNewSerialMode;
        temp_pars->temp_parameters.ucSerialSpeed = ucNewSerialSpeed;
    #if defined SERIAL_INTERFACE && defined DEMO_UART                    // {8}
        fnSetNewSerialMode(0, MODIFY_CONFIG);                            // {49} modify the debug interface setting accordingly
        return;
    #endif
    }
    #if USB_CDC_VCOM_COUNT > 1
    else {                                                               // supplimentary CDC interfaces
        iInterface--;
        CDC_UART_SerialMode[iInterface] &= ~(CHAR_7 | RS232_EVEN_PARITY | RS232_ODD_PARITY | ONE_HALF_STOPS | TWO_STOPS);
        CDC_UART_SerialMode[iInterface] |= usNewSerialMode;
        ucCDC_UART_SerialSpeed[iInterface] = ucNewSerialSpeed;
        #if defined USE_USB_CDC && defined SERIAL_INTERFACE && defined USB_SERIAL_CONNECTIONS && (USB_CDC_VCOM_COUNT > 1)
        fnOpenUSB_UARTs(iInterface + 1);                                 // modify the single interface
        #endif
    }
    #endif
}
#endif

#if defined USE_MAINTENANCE && defined USB_INTERFACE && defined USE_USB_CDC && defined USB_CDC_VCOM_COUNT && (USB_CDC_VCOM_COUNT > 0)
extern int fnUSB_CDC_TX(int iStart)
{
    static unsigned long ulTxCount[USB_CDC_VCOM_COUNT] = {0};
    static unsigned char ucTxComplete[USB_CDC_VCOM_COUNT] = {0};
    #if SIZE_OF_RAM < (16 * 1024)
    static unsigned char ucTestData[128];                                // the transmit block size is best a length of half the output buffer size so that it can keep the output buffer filled
    #else
    static unsigned char ucTestData[512];
    #endif
    int i;
    int iNotComplete = 0;
    if (iStart != 0) {
        static int iNotTested = 0;
        iNotTested = 0;
        uMemset(ucTxComplete, 0, sizeof(ucTxComplete));
        for (i = 0; i < USB_CDC_VCOM_COUNT; i++) {
            if (fnWrite(USBPortID_comms[i], 0, 1) == 0) {                // if it is not possible to queue a single byte of data the interface is not active
                ucTxComplete[i] = 1;                                     // mark that this interface will not be tested
                iNotTested++;                                            // count the number of interfaces that will not be tested
            }
            else {
                fnFlush(USBPortID_comms[i], FLUSH_TX);                   // flush the transmitter before starting in order to have an aligned circular buffer for optimal efficiency
            }
        }
        if (iNotTested >= USB_CDC_VCOM_COUNT) {                          // if there are no active interfaces
            fnDebugMsg("No USB-CDC interface active!\r\n");              // abort the test
            return 0;
        }
        uMemset(ulTxCount, 0, sizeof(ulTxCount));
        uMemset(ucTestData, 'X', sizeof(ucTestData));                    // set the transmission pattern
    }
    for (i = 0; i < USB_CDC_VCOM_COUNT; i++) {                           // for each possible interface
        if (ucTxComplete[i] != 0) {                                      // ignore interfaces that have completed or aren't to be tested
            continue;
        }
        if (fnWrite(USBPortID_comms[i], 0, sizeof(ucTestData)) != 0) {   // check whether the data can be copied to the output buffer
            fnWrite(USBPortID_comms[i], ucTestData, sizeof(ucTestData)); // copy data for transmission
            ulTxCount[i] += sizeof(ucTestData);                          // the amount of data queued for transmission
            if (ulTxCount[i] >= (10 * 1024 * 1024)) {                    // has all the test data been sent over this interface?
                ucTxComplete[i] = 1;                                     // mark that this interface has completed
                continue;                                                // avoid setting the "not complete" flag due to the final queued data block
            }
        }
        iNotComplete = 1;
    }
    if (iNotComplete != 0) {                                             // if not complete
        fnInterruptMessage(TASK_DEBUG, E_USB_TX_CONTINUE);               // schedule the next transmission
        return 1;
    }
    // All USB CDC interfaces have terminated
    //
    fnDebugMsg("\r\nComplete!\r\n");
    return 0;
}
#endif
#endif
