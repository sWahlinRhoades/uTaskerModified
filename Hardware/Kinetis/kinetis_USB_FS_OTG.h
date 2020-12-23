/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_USB_FS_OTG.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    28.03.2013 Add USB HS support                                        {33}
    05.03.2014 Add USB error counters and clear errors in the USB interrupt routine {71}
    28.05.2014 Move re-enable of USB SIE token processing to after input handling to avoid sporadic SETUP frame loss {83}
    30.10.2014 Add optional crystal-less USB clock for K64, K63, K22 and K24 {104}
    01.12.2015 Allow control endpoint to operate with reversed data toggle (required to receive fast multi-frame OUTs belonging to a SETUP block) {1}
    23.12.2015 Add zero copy OUT endpoint buffer option                  {2}
    12.02.2017 Add KL82 support and allow USB host operation with shared endpoints {3}
    24.03.2017 Reset ulNextRxData0 in host mode when non-0 endpoint are re-used {4}
    12.04.2017 Add optional USB event logging for operation analysis     {5}
    12.04.2017 Add setup frame response timeout and repetition in host mode {6}
    12.04.2017 Temporarily disable USB operation on host mode reset to ensure that state change flags can be reliably reset {7}
    02.01.2018 Allow new KL devices with SIM_CLKDIV2 to control the USB clock divider {8}
    06.09.2019 Add fnDisableIN() to automatically disable IN polling when any other token is prepared (host mode) {9}
    06.09.2019 Mask interrupt endpoints with CONTROL_HOST_INTERRUPT_ENDPOINT and perform single-shot INs on such endpoints {10}
    03.10.2019 Improve detection/debouncing of USB device connection     {11}
    28.12.2019 Add fnUnplugUSBDevice() and fnPlugUSBDevice()             {12}
    06.06.2020 Allow use together with HS driver (kinetis parst with FS and HS)
    14.06.2020 Add bus turnaround error handling when connecting to devices that don't initially respond so that repeated attempts ensure the operate when they are ready {13}


*/

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined USB_FS_MALLOC
    #define USB_FS_MALLOC(x) uMalloc((MAX_MALLOC)(x)) 
#endif
#if !defined USB_FS_MALLOC_ALIGN
    #define USB_FS_MALLOC_ALIGN(x, y) uMallocAlign((MAX_MALLOC)(x), (y))
#endif

#if defined USB_HOST_SUPPORT
    #define USE_BUS_TIMEOUT
#endif

#if defined ERRATA_ID_7166 && defined SUPPORT_LOW_POWER
    #error "errate e7166 - SOC: SDHC, NFC, USBOTG, and cache modules are not clocked correctly in low-power modes - do not enable low power operation when USB is used!"
#endif


#if defined USB_HOST_SUPPORT
    #define __USB_HOST_MODE()         ((usb_hardware.ucModeType & USB_HOST_MODE) != 0)
    #define __USB_DEVICE_MODE()       ((usb_hardware.ucModeType & USB_HOST_MODE) == 0)
    #define SETUP_ACK_DELAY_MS        10                                 // an ACK to a setup is expected within 10ms
#else
    #define __USB_HOST_MODE()         (0)
    #define __USB_DEVICE_MODE()       (1)
    #define USB_DEVICE_SUPPORT                                           // enable device only support if host support hasn't specifically been defined
#endif

#if defined USB_HS_INTERFACE
    extern void fnConfigHSUSB(QUEUE_HANDLE Channel, USBTABLE *pars);
    extern int  fnGetHSUSB_HW(int iEndpoint, USB_HW **ptr_hs_usb_hardware);
    extern void fnActivateHS_HWEndpoint(unsigned char ucEndpointType, unsigned char ucEndpointRef, unsigned short usEndpointLength, unsigned short usMaxEndpointLength, USB_ENDPOINT *ptrEndpoint);
#endif

#define fnGetEndPointCtr(iEndPoint) (unsigned char *)(ENDPT0_ADD + ((iEndPoint) * sizeof(unsigned long)))

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

typedef struct stUSB_ERRORS                                              // {71}
{
    unsigned long ulUSB_errors_CRC5;
    unsigned long ulUSB_errors_CRC16;
#if defined USE_BUS_TIMEOUT
    unsigned long ulUSB_timeouts;
#endif
} USB_ERRORS;

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static USB_HW usb_hardware = {0};                                        // {33}
static KINETIS_USB_ENDPOINT_BD *ptrBDT = 0;
static USB_END_POINT *usb_endpoints = 0;
static USB_ERRORS USB_errors = {0};                                      // {71}

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static void fnUSB_reset(int iAttached);
#if defined USB_HOST_SUPPORT
    static void fnUSB_HostDelay(void (*timeout_handler)(void), unsigned long ulDelay);
#endif

/* =================================================================== */
/*                                USB                                  */
/* =================================================================== */

//#define USB_LOG_SIZE    100                                            // {5} enable USB activity log when this exists and is set to a non-zero value

#if defined USB_LOG_SIZE && (USB_LOG_SIZE > 0)
    volatile CHAR usb_log[USB_LOG_SIZE * 2] = { 0 };
    volatile unsigned long ulLogLength = 0;

    static void usb_monitor(const CHAR *reference, unsigned char ucData)
    {
        if (ulLogLength >= (2 * USB_LOG_SIZE)) {
            return;
        }
        usb_log[ulLogLength++] = *reference;
        usb_log[ulLogLength++] = (CHAR)ucData;
    }
#else
    #define usb_monitor(x, y)                                            // remove activity log
#endif

#if defined USB_HOST_SUPPORT
static void fnDisableIN(void)                                            // {9}
{
    while (((TOKEN >> 4) == IN_PID) && ((CTL & TXSUSPEND_TOKENBUSY) != 0)) { // if an IN token is in progress (otherwise there is no need to stop the IN polling)
        if ((INT_STAT & USB_RST) != 0) {                                 // reset state detected during wait for token to complete
            CTL &= ~TXSUSPEND_TOKENBUSY;                                 // abort present token transaction
            break;                                                       // quit
        }
      //if ((ENDPT0 & RETRY_DIS) == 0) {                                 // if continuous IN
            ENDPT0 |= RETRY_DIS;                                         // disable retries so that the continuous IN polling stops
      //}
    #if defined _WINDOWS
        CTL &= ~(TXSUSPEND_TOKENBUSY);
    #endif
    }
}

static int fnSendToken(unsigned char ucPID)
{
    #define MAX_TOKEN_WAIT 1000
    volatile int iTimeout = 0;
    while ((CTL & TXSUSPEND_TOKENBUSY) != 0) {                           // wait if there is already a token in progress
        if ((INT_STAT & USB_RST) != 0) {                                 // reset state detected during wait for token to complete
            CTL &= ~TXSUSPEND_TOKENBUSY;                                 // abort present token transaction
            return -1;                                                   // quit
        }
        if (ucPID == TOKEN) {                                            // if an IN token on the same endpoint is being set while already polling we can ignore the command
            if ((ucPID >> 4) == IN_PID) {
                return 0;
            }
        }
        if (++iTimeout >= MAX_TOKEN_WAIT) {                              // if a token has not completed after a certain time we abort it so that we can continue (this may happen when a device fails but a reset is not detected)
            CTL &= ~TXSUSPEND_TOKENBUSY;                                 // abort present token transaction
        }
    #if defined _WINDOWS
        if ((TOKEN >> 4) == OUT_PID) {                                   // OUT in operation
            fnSimInts(0);                                                // allow simulator to send it
        }
    #endif
        fnDisableIN();                                                   // {9} automatically stop any continuous IN polling if necessary
    }
    if ((ucPID >> 4) == IN_PID) {                                        // if an IN PID is to be sent
        unsigned char ucEndpoint = (ucPID & 0x0f);                       // the endpoint
        if ((ucEndpoint != usb_hardware.ucHostEndpointActive)            // change of endpoint detected
  //#if defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS                       // {3} always synchronise the reception buffer characteristics on application endpoints since they may be shared and this don't change the value betweeen tx and rx usage
        || (ucEndpoint != 0))
  //#endif
        {
            usb_hardware.ptrEndpoint = &usb_endpoints[ucEndpoint];       // the endpoint's characteristics
            usb_hardware.ucHostEndpointActive = ucEndpoint;              // the newly active endpoint
            if ((usb_endpoints[ucEndpoint].ulNextRxData0 & DATA_1) != 0) { // ensure that the receive buffer has the endpoint's data flags set correctly
                *(usb_hardware.ptr_ulUSB_Rx_BDControl) |= DATA_1;
                *(usb_hardware.ptr_ulUSB_Alt_Rx_BDControl) &= ~(DATA_1);
            }
            else {
                *(usb_hardware.ptr_ulUSB_Rx_BDControl) &= ~(DATA_1);
                *(usb_hardware.ptr_ulUSB_Alt_Rx_BDControl) |= DATA_1;
            }
        }
        if ((usb_hardware.ptrEndpoint->ulEndpointSize & CONTROL_HOST_INTERRUPT_ENDPOINT) == 0) { // {10}
            ENDPT0 &= ~(RETRY_DIS);                                      // enable retries on non-interrupt endpoint (the IN token will be repeated while the device responds with NAK_PID)
        }
        else {
            ENDPT0 |= RETRY_DIS;                                         // disable retries on interrupt endpoint (a single IN token will be sent)
        }
    }
    else {
        ENDPT0 &= ~(RETRY_DIS);                                          // {10} ensure retries are enabled for SETUP and DATA transmissions
    }
    TOKEN = ucPID;                                                       // start transmission of token to endpoint
    #if defined _WINDOWS
    CTL |= TXSUSPEND_TOKENBUSY;                                          // reflect that a token is in progress
    fnAddToken(TOKEN);                                                   // inform simulator which token is being sent
    #endif
    INT_ENB = (TOK_DNE_EN | STALL_EN | USB_ERROR_EN | USB_RST);          // interrupt when complete
    return 0;                                                            // success
}

static void usb_start(void)
{
    fnUSB_handle_frame(USB_DEVICE_DETECTED, 0, 0, &usb_hardware);        // generic handler routine which will always request a setup frame to be sent
}

// USB host reset pulse complete (after >= 10ms of driving reset state)
//
static void usb_reset_timeout(void)
{
    KINETIS_USB_ENDPOINT_BD *ptEndpointBD = ptrBDT;                      // start of BDT
#if defined USE_BUS_TIMEOUT
    ERR_ENB = (CRC5_EOF_EN | CRC16_EN | BTO_ERR_EN);                     // enable some error sources
#else
    ERR_ENB = (CRC5_EOF_EN | CRC16_EN);                                  // enable some error sources
#endif
    CTL &= ~USB_RESET;                                                   // remove USB bus reset
    CTL |= USB_EN_SOF_EN;                                                // start sending SOF packets
    usb_hardware.ptrEndpoint = &usb_endpoints[0];
    usb_hardware.ucHostEndpointActive = 0;
    usb_hardware.ucBufferUsage = 0;                                      // reset to even reception and even transmit buffers
    usb_hardware.ptr_ulUSB_Rx_BDControl = &ptEndpointBD->usb_bd_rx_even.ulUSB_BDControl; // first reception after a bus reset is to the even buffer
    usb_hardware.ptr_ulUSB_Alt_Rx_BDControl = &ptEndpointBD->usb_bd_rx_odd.ulUSB_BDControl; // the alternate buffer is the odd buffer
    usb_hardware.ptr_ulUSB_BDControl = &ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl; // first transmission is from even buffer (DATA0)
    usb_hardware.ptrTxDatBuffer = usb_hardware.ptrUSB_BD_EvenData = &ptEndpointBD->usb_bd_tx_even.ptrUSB_BD_Data;
    if ((ADDR & LS_EN) != 0) {
        usb_hardware.ucDeviceSpeed = USB_DEVICE_SPEED_LOW;               // attached device is a low speed device
    }
    else {
        usb_hardware.ucDeviceSpeed = USB_DEVICE_SPEED_FULL;              // attached device is a full-speed device
    }
    fnUSB_HostDelay(usb_start, PIT_MS_DELAY(40));                        // short wait to allow slow devices to become ready
  //fnUSB_handle_frame(USB_DEVICE_DETECTED, 0, 0, &usb_hardware);        // generic handler routine which will always request a setup frame to be sent (this is instead called after a delay to avoid problems with some devices that need some time to become ready)
}

#if 0
// SETUP was sent but not responded to within timeout period
//
static void usb_timeout(void)                                            // {6}
{
    uEnable_Interrupt();                                                 // don't block interrupts when stopping IN polling
        fnHostEndpoint(0, IN_POLLING, 0);                                // stop active IN polling (this will usually cause a NAK result to be flagged on the present rx buffer, which is freed by the USB interrupt being called in the process (hence the need to allow nested interrupt and the USB interrupt having higher priority than the timer interrupt)
    uDisable_Interrupt();
    fnUSB_handle_frame(USB_DEVICE_TIMEOUT, 0, 0, &usb_hardware);         // report the error so that the generic enumeration can repeat
}
#endif

extern int fnHostEndpoint(unsigned char ucEndpoint, int iCommand, int iEvent)
{
    switch (iCommand) {
    case IN_POLLING:
        if (iEvent != 0) {                                               // enable IN polling
            fnSendToken((unsigned char)((IN_PID << 4) | ucEndpoint));    // start IN polling on this endpoint (the new endpoint is set as active)
        }
        else {                                                           // disable IN polling
            fnDisableIN();                                               // {9}
        }
    }
    return 0;
}

extern void fnResetDataToggle(int iChannel, int iEndpoint, USB_HW *ptrUSB_HW)
{
    usb_endpoints[(iEndpoint & ~(IN_ENDPOINT))].ulNextRxData0 &= ~(DATA_1); // next IN reception from the endpoint will be changed to DATA 0
}

static void fnFreeHostRx(int iEndpoint_ref, KINETIS_USB_BD *ptUSB_BD_rx)
{
    ptUSB_BD_rx->ulUSB_BDControl &= DATA_1;                              // retain the DATA1 state of the receive buffer
    ptUSB_BD_rx->ulUSB_BDControl |= (OWN | ((64 << USB_CNT_SHIFT)/*usb_endpoints[iEndpoint_ref].ulEndpointSize*/ & (USB_BYTE_CNT_MASK | DTS))); // re-enable reception on this endpoint
    usb_endpoints[iEndpoint_ref].ulNextRxData0 = ptUSB_BD_rx->ulUSB_BDControl; // last buffer processed
}
#endif

// This routine handles all SETUP and OUT frames. It can send an empty data frame if required by control endpoints or stall on errors.
// It usually clears the handled input buffer when complete, unless the buffer is specified to remain owned by the processor.
//
static int fnProcessInput(int iEndpoint_ref, USB_HW *ptrUSB_HW, unsigned char ucFrameType, KINETIS_USB_BD *ptUSB_BD_rx, KINETIS_USB_ENDPOINT_BD *ptEndpointBD)
{
    int iEvenTxBuffer = 0;
    ptrUSB_HW->ptrEndpoint = &usb_endpoints[iEndpoint_ref];              // mark the present transmit endpoint information for possible subroutine or response use
    if (_USB_HOST_MODE()) {
    #if defined USB_HOST_SUPPORT
        if ((ptrUSB_HW->ucBufferUsage & TX_TRANSACTION) == 0) {          // host uses only endpoint 0 for transmission
            iEvenTxBuffer = 1;
        }
    #endif
    }
    else {
    #if defined USB_DEVICE_SUPPORT
        if ((usb_endpoints[iEndpoint_ref].ulEndpointSize & ALTERNATE_TX_BUFFER) == 0) {
            iEvenTxBuffer = 1;
        }
    #endif
    }
    if (iEvenTxBuffer != 0) {                                            // set the next transmit pointer details
        ptrUSB_HW->ptr_ulUSB_BDControl = &ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl; // prepare hardware relevant data for the generic handler's use
        ptrUSB_HW->ptrTxDatBuffer = &ptEndpointBD->usb_bd_tx_even.ptrUSB_BD_Data;
    }
    else {
        ptrUSB_HW->ptr_ulUSB_BDControl = &ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl; // prepare hardware relevant data for the generic handler's use
        ptrUSB_HW->ptrTxDatBuffer = &ptEndpointBD->usb_bd_tx_odd.ptrUSB_BD_Data;
    }
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when putting messages to queue
    switch (fnUSB_handle_frame(ucFrameType, (unsigned char *)ptUSB_BD_rx->ptrUSB_BD_Data, iEndpoint_ref, ptrUSB_HW)) { // generic handler routine
    case TERMINATE_ZERO_DATA:                                            // send zero data packet to complete status stage of control transfer
        *ptrUSB_HW->ptr_ulUSB_BDControl = (OWN | ptrUSB_HW->ptrEndpoint->ulNextTxData0); // transmit a zero data packet on control endpoint
        _SIM_USB(0, USB_SIM_TX, iEndpoint_ref, ptrUSB_HW);
        ptrUSB_HW->ptrEndpoint->ulNextTxData0 ^= DATA_1;                 // toggle the data packet
    #if defined USB_HOST_SUPPORT
        if ((ptrUSB_HW->ucModeType & USB_HOST_MODE) != 0) {
            ptrUSB_HW->ucBufferUsage ^= TX_TRANSACTION;                  // toggle the next transmission buffer to use
            fnSendToken((unsigned char)((OUT_PID << 4) | iEndpoint_ref));// start zero data transfer
            fnFreeHostRx(iEndpoint_ref, ptUSB_BD_rx);                    // free the used buffer
            uEnable_Interrupt();
            return 0;
        }
    #endif
    #if defined USB_DEVICE_SUPPORT
        ptrUSB_HW->ptrEndpoint->ulEndpointSize ^= ALTERNATE_TX_BUFFER;
        ptUSB_BD_rx->ulUSB_BDControl |= (OWN);                           // re-enable reception for this endpoint buffer since it has been freed
    #endif
        break;
    #if defined USB_DEVICE_SUPPORT
    case MAINTAIN_OWNERSHIP:                                             // don't free the buffer - the application will do this later
        uEnable_Interrupt();
        return MAINTAIN_OWNERSHIP;
    #endif
    #if defined USB_HOST_SUPPORT
    case INITIATE_IN_TOKEN:
        fnSendToken((unsigned char)(IN_PID << 4) | iEndpoint_ref);       // start transmission of next IN token
        fnFreeHostRx(iEndpoint_ref, ptUSB_BD_rx);                        // free the used buffer
        uEnable_Interrupt();
        return 0;
    #endif
    #if defined USB_DEVICE_SUPPORT
    case STALL_ENDPOINT:                                                 // send stall
        if (iEndpoint_ref == 0) {                                        // check whether control 0 endpoint
            ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl = (OWN | BDT_STALL); // force stall handshake on both control 0 buffers
            ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl  = (OWN | BDT_STALL);
            fnSetUSBEndpointState(iEndpoint_ref, USB_ENDPOINT_STALLED, 0);       
            _SIM_USB(0, USB_SIM_STALL, iEndpoint_ref, ptrUSB_HW);
        }
        else {
            int iChannel = 0;
            KINETIS_USB_ENDPOINT_BD *ptOUT_BD = ptEndpointBD;
            int iIN_ref = fnGetPairedIN(iEndpoint_ref, iChannel);        // get the paired IN endpoint reference
            ptEndpointBD = ptrBDT;
            ptEndpointBD += iIN_ref;
            ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl = (OWN | BDT_STALL); // stall/halt the corresponding IN endpoint
            ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl  = (OWN | BDT_STALL);
            ptEndpointBD = ptOUT_BD;
            fnSetUSBEndpointState(iIN_ref, USB_ENDPOINT_STALLED, 0);     // mark the stall at the IN endpoint
            _SIM_USB(0, USB_SIM_STALL, iIN_ref, ptrUSB_HW);
        }
    #endif
        // Fall through to free buffer
        //
    default:
    #if defined USB_HOST_SUPPORT
        if ((CTL & HOST_MODE_EN) != 0) {
            fnFreeHostRx(iEndpoint_ref, ptUSB_BD_rx);                    // free the used buffer
            uEnable_Interrupt();
            return 0;
        }
    #endif
    #if defined USB_DEVICE_SUPPORT                                       // enable further reception on the endpoint
        ptrUSB_HW->ulRxControl |= (usb_endpoints[iEndpoint_ref].ulEndpointSize & (USB_BYTE_CNT_MASK | DTS)); // reception buffer size
        if ((iEndpoint_ref == 0) && ((usb_hardware.ulRxControl & CONTROL_DATA_TOGGLE_REVERSED) != 0)) { // {1} if the data toggle on the control endpoint is swapped
            ptUSB_BD_rx->ulUSB_BDControl = (OWN | (ptrUSB_HW->ulRxControl ^ DATA_1)); // re-enable reception on this endpoint
        }
        else {
            ptUSB_BD_rx->ulUSB_BDControl = (OWN | ptrUSB_HW->ulRxControl); // re-enable reception on this endpoint
        }
        usb_endpoints[iEndpoint_ref].ulNextRxData0 = ptrUSB_HW->ulRxControl; // last buffer processed
    #endif
        break;
    }
    uEnable_Interrupt();
    return 0;
}

    #if defined USB_DEVICE_SUPPORT
// When the clear feature is received for a stalled endpoint it is cleared in the hardware by calling this routine
//
extern void fnUnhaltEndpoint(unsigned char ucEndpoint)
{
    KINETIS_USB_ENDPOINT_BD *ptEndpointBD = ptrBDT;
    ptEndpointBD += (ucEndpoint & ~(IN_ENDPOINT));
    if ((ucEndpoint & IN_ENDPOINT) != 0) {
        unsigned char *ptrEndPointCtr;
        ucEndpoint &= ~(IN_ENDPOINT);
        usb_endpoints[ucEndpoint].ulNextTxData0 &= ~DATA_1;
        ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl = 0;
        ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl  = 0;
        ptrEndPointCtr = fnGetEndPointCtr(ucEndpoint);
        *ptrEndPointCtr &= ~(EP_STALL);
    }
    else {
        ptEndpointBD->usb_bd_rx_even.ulUSB_BDControl &= ~(BDT_STALL);
        ptEndpointBD->usb_bd_rx_odd.ulUSB_BDControl  &= ~(BDT_STALL);
    }
}
    #endif

static void fnUSB_reset(int iAttached)
{
#if defined USB_HOST_SUPPORT                                             // {7}
    if ((CTL & HOST_MODE_EN) != 0) {
        CTL &= ~(USB_EN_SOF_EN);                                         // it is required to temporarily disable the USB operation otherwise the reset and attach flags cannot be reset
    }
#endif
    INT_STAT = (USB_RST | SLEEP | RESUME_EN | USB_ERROR | ATTACH);       // reset flags
    CTL |= ODD_RST;                                                      // reset all odd BDTs which specifies the even bank
    INT_ENB &= ~(RESUME_EN | SLEEP_EN);                                  // disable resume and suspend interrupts
    usb_endpoints[0].ulEndpointSize &= ~ALTERNATE_TX_BUFFER;
    usb_endpoints[0].ulNextRxData0 = RX_DATA_TOGGLE;                     // DATA 1 is always first reception (as response to SETUP)
    CTL &= ~ODD_RST;
    ADDR = 0;                                                            // reset device address
    ENDPT0 = EP_HSHK;                                                    // disable reception and transmission, with enabled handshake on transaction on control endpoint 0
    ENDPT1 = 0;                                                          // disable all other endpoints
    ENDPT2 = 0;
    ENDPT3 = 0;
    ENDPT4 = 0;
    ENDPT5 = 0;
    ENDPT6 = 0;
    ENDPT7 = 0;
    ENDPT8 = 0;
    ENDPT9 = 0;
    ENDPT10 = 0;
    ENDPT11 = 0;
    ENDPT12 = 0;
    ENDPT13 = 0;
    ENDPT14 = 0;
    ENDPT15 = 0;
    _WAIT_REGISTER_TRUE(ENDPT0, EP_TX_ENABLE);
                                                                         // reconfigure all packet buffers of the control endpoint
    ptrBDT->usb_bd_rx_odd.ulUSB_BDControl  = (SET_FRAME_LENGTH(ENDPOINT_0_SIZE) | DTS | OWN); // set control endpoint rx size and configure Data Toggle Synchronisation (receive DATA 0 frames)
    ptrBDT->usb_bd_rx_even.ulUSB_BDControl = (ptrBDT->usb_bd_rx_odd.ulUSB_BDControl | DATA_1); // set control endpoint rx size and configure Data Toggle Synchronisation (Receive DATA 1 frames)
                
#if defined USB_HOST_SUPPORT
    if ((CTL & HOST_MODE_EN) != 0) {
        if (iAttached != 0) {                                            // reset during device attachment
            return;
        }
        fnUSB_handle_frame(USB_DEVICE_REMOVED, 0, 0, &usb_hardware);     // generic handler routine for device removal
        INT_ENB = ATTACH_EN;                                             // enable an interrupt on the attach of a device
        return;
    }
#endif
#if defined USB_DEVICE_SUPPORT
    fnUSB_handle_frame(USB_RESET_DETECTED, 0, 0, &usb_hardware);         // generic handler routine
    usb_hardware.ucUSBAddress = 0;                                       // reset the address to revert back to the default state
    ENDPT0 = (EP_HSHK | EP_RX_ENABLE | EP_TX_ENABLE);                    // enable control endpoint reception and transmission
#endif
}

    #if defined USB_HOST_SUPPORT
        #if !defined USB_HOST_PIT_CHANNEL
            #define USB_HOST_PIT_CHANNEL    0                            // user PIT0 if no other is defined
        #endif
// A single-shot HW timer reserved for USB host timing use. It uses a user definable PIT channel
//
static void fnUSB_HostDelay(void (*timeout_handler)(void), unsigned long ulDelay)
{
    PIT_SETUP pit_setup;                                                 // interrupt configuration parameters
    pit_setup.int_type = PIT_INTERRUPT;
    pit_setup.ucPIT = USB_HOST_PIT_CHANNEL;
    if (ulDelay == 0) {
        pit_setup.mode = PIT_STOP;                                       // stop the timer
    }
    else {
        pit_setup.mode = PIT_SINGLE_SHOT;                                // one-shot interrupt
        pit_setup.int_handler = timeout_handler;
        pit_setup.int_priority = USB_PIT_INTERRUPT_PRIORITY;
        pit_setup.count_delay = ulDelay;                                 // delay
    }
    fnConfigureInterrupt((void *)&pit_setup);                            // enter interrupt
}

extern void fnUSB_DeviceResetCycle(int iChannel)
{
    WRITE_ONE_TO_CLEAR(INT_STAT, ATTACH);                                // {11} clear the attach flag
    fnDelayLoop(5);                                                      // {11} 5us delay to see whether the attach flag is set again (takes 2.5us to be detected)
    #if defined _WINDOWS
    INT_STAT |= ATTACH;
    #endif
    if ((INT_STAT & ATTACH) != 0) {                                      // a device has really been attached
        fnUSB_reset(1);

        if ((CTL & JSTATE) == 0) {                                       // low speed device connected
            usb_hardware.ucUSBAddress = LS_EN;
            ADDR = LS_EN;                                                // select low speed mode
        }
        ENDPT0 = (HOST_WO_HUB | EP_HSHK | EP_RX_ENABLE | EP_TX_ENABLE);  // enable control 0 endpoint transmission and reception for host usage
        WRITE_ONE_TO_CLEAR(INT_STAT, (USB_RST | SLEEP | RESUME_EN | USB_ERROR)); // reset flags that may be present after some signal bouncing at connection
        CTL |= USB_RESET;                                                // send reset to USB bus for a minimum of 10ms
        fnUSB_HostDelay(usb_reset_timeout, PIT_MS_DELAY(50));            // remove the reset after this delay (50ms is used instead of 10ms since it avoids problems with some devices)
    }
    else {                                                               // spurious attach - activate the attach interrupt again
        INT_ENB = ATTACH_EN;                                             // enable an interrupt on the attach of a device
    }
}

// After a short delay after a possible attach, check the state and decide whether we need to continue with the attached device
//
static void usb_possible_attach(void)
{
    fnUSB_DeviceResetCycle(0);
}

extern void fnHostReleaseBuffer(int iEndpoint, unsigned char ucTransferType, USB_HW *ptrUSB_HW)
{
    if (ucTransferType == SETUP_PID) {                                   // initiate a SETUP       
        fnSendToken((unsigned char)((SETUP_PID << 4) | iEndpoint));      // release the setup frame on the corresponding endpoint
      //fnUSB_HostDelay(usb_timeout, PIT_MS_DELAY(SETUP_ACK_DELAY_MS));  // {6} monitor the successful setup transaction by using a hardware timer to detect a timeout when the device doesn't respond to subsequent IN token polling (there is no way to detect when the device doesn't ack the SETUP itself)
    }
    else if (ucTransferType == OUT_PID) {                                // OUT
        fnSendToken((unsigned char)((OUT_PID << 4) | iEndpoint));        // release the data frame on the corresponding endpoint
    }
    else {
        _EXCEPTION("Token to be added!!");
    }
    ptrUSB_HW->ucHostEndpointActive = (unsigned char)iEndpoint;          // remember the endpoint that the host is presently transmitting data on
}
    #endif


extern unsigned long fnUSB_error_counters(int iChannel, int iValue)      // {71}
{
    switch (iValue) {
    case USB_CRC_5_ERROR_COUNTER:
        return USB_errors.ulUSB_errors_CRC5;
    case USB_CRC_16_ERROR_COUNTER:
        return USB_errors.ulUSB_errors_CRC16;
    case USB_ERRORS_RESET:
        uMemset(&USB_errors, 0, sizeof(USB_errors));
        break;
    }
    return 0;
}

// USB OTG Interrupt handler
//
static __interrupt void _usb_otg_isr(void)
{
    #if defined USB_HOST_SUPPORT
    int iBusTurnaroundTimeout;
    #endif
    unsigned char ucUSB_Int_status;

    usb_monitor("I", 0);                                                 // record activity for optional analysis purposes
    FOREVER_LOOP() {                                                     // allow multiple interrupt cycles to be possible
        ucUSB_Int_status = (unsigned char)INT_ENB;                       // the interrupts that we are concerned with
        ucUSB_Int_status &= (unsigned char)INT_STAT;                     // mask with interrupt(s) that are pending at this moment in time
        if (ucUSB_Int_status == 0) {                                     // if no (more) interrupt(s) pending we quit the service routine
            usb_monitor("E", 0);                                         // record activity for optional analysis purposes
            break;
        }
    #if defined USB_HOST_SUPPORT
        iBusTurnaroundTimeout = 0;                                       // initially no bus turnaround time error detected
    #endif
        usb_monitor("S", ucUSB_Int_status);                              // record activity for optional analysis purposes
        if ((ucUSB_Int_status & ~TOK_DNE) != 0) {                        // check first for bus state changes
    #if defined USB_HOST_SUPPORT
            if ((ATTACH & ucUSB_Int_status) != 0) {                      // device has been attached
                INT_ENB  = 0;                                            // disable further interrupts
                fnUSB_HostDelay(usb_possible_attach, PIT_MS_DELAY(50));  // wait a short time before checking whether this was a real attach
            }
    #endif
            uDisable_Interrupt();                                        // ensure interrupts remain blocked when putting messages to queue
            if ((ucUSB_Int_status & USB_RST) != 0) {                     // reset detected - D+ and D- in SE0 (single ended logic 0) state for > 2.5us
                fnUSB_reset(0);
            }
            if ((ucUSB_Int_status & USB_ERROR) != 0) {                   // error detected
    #if defined USE_BUS_TIMEOUT
                if ((ERR_STAT & BTO_ERR) != 0) {                         // bus turnaround timeout error
        #if defined USB_HOST_SUPPORT
                    iBusTurnaroundTimeout = 1;                           // {13} mark that a bus turnaround timeout has been detected (this means that the host transaction terminated with an error)
        #endif
                    USB_errors.ulUSB_timeouts++;                         // count the bus turnaround timeouts for statistic purposes
                }
    #endif
                if ((ERR_STAT & CRC16) != 0) {                           // CRC-16 error
                    USB_errors.ulUSB_errors_CRC16++;                     // count errors
            	}
                if ((ERR_STAT & CRC5_EOF) != 0) {                        // CRC-5 error
                    USB_errors.ulUSB_errors_CRC5++;                      // count errors
            	}
            	ERR_STAT = (CRC16 | CRC5_EOF | BTO_ERR);                 // clear error sources
                INT_STAT = USB_ERROR;                                    // reset flag
            }
    #if defined USB_DEVICE_SUPPORT
            if ((ucUSB_Int_status & SLEEP) != 0) {                       // suspend state detected - 3ms of idle USB bus detected (low speed D+ = 0, D- = 1 / high speed D+ = 1, D- = 0)
                INT_STAT = (SLEEP | RESUME);                             // reset flag
                INT_ENB |= RESUME_EN;                                    // enable resume interrupt
                fnUSB_handle_frame(USB_SUSPEND_DETECTED, 0, 0, &usb_hardware); // generic handler routine
            }
            if ((ucUSB_Int_status & RESUME) != 0) {                      // resume detected - 10ms inverted idle USB bus state (low speed D+ = 1, D- = 0 / high speed D+ = 0, D- = 1)
                INT_STAT = (SLEEP | RESUME);                             // reset flag
                INT_ENB &= ~RESUME_EN;                                   // disable resume interrupt
                fnUSB_handle_frame(USB_RESUME_DETECTED, 0, 0, &usb_hardware); // generic handler routine
            }
    #endif
            uEnable_Interrupt();
            INT_STAT = (unsigned char)(ucUSB_Int_status & ~TOK_DNE);     // reset all other flags
    #if defined _WINDOWS
            INT_STAT &= (~(ucUSB_Int_status & ~TOK_DNE) | ATTACH);       // leave ATTACH status in the register
    #endif
        }
        if ((ucUSB_Int_status & TOK_DNE) != 0) {                         // current processed token complete
            KINETIS_USB_ENDPOINT_BD *ptEndpointBD = ptrBDT;              // start of BDT
            KINETIS_USB_BD *ptUSB_BD;                                    // specific BD
            int iEndpoint_ref = (STAT >> END_POINT_SHIFT);               // the endpoint belonging to this event (this is always 0 in host mode)
            ptEndpointBD += iEndpoint_ref;                               // set to endpoint buffer descriptor block as indicated by the status register
            if ((STAT & TX_TRANSACTION) != 0) {                          // determine the specific BD for the transaction
    #if defined USB_HOST_SUPPORT
                int iHostDo;
    #endif
                int iLastTxEven = 0;
                if ((CTL & HOST_MODE_EN) != 0) {                         // if in host mode
    #if defined USB_HOST_SUPPORT
                    usb_monitor("D", STAT);                              // record activity for optional analysis purposes
                    iEndpoint_ref = usb_hardware.ucHostEndpointActive;   // the presently active host endpoint
                    usb_hardware.ptrEndpoint = &usb_endpoints[iEndpoint_ref];
                    if ((usb_hardware.ucBufferUsage & TX_TRANSACTION) == 0) { // if the last transmission on the fixed endpoint 0 was in the even buffer
                        iLastTxEven = 1;
                    }
    #endif
                }
                else {
    #if defined USB_DEVICE_SUPPORT
                    usb_hardware.ptrEndpoint = &usb_endpoints[iEndpoint_ref];
                    if ((usb_hardware.ptrEndpoint->ulEndpointSize & ALTERNATE_TX_BUFFER) == 0) { // if the last transmission on this endpoint was in the even buffer
                        iLastTxEven = 1;
                    }
    #endif
                }  
                if (iLastTxEven != 0) {                                  // set a pointer to the buffer descriptor of the next transmission buffer to use
                    usb_hardware.ptr_ulUSB_BDControl = &ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl; // use even buffer for next transmission
                    usb_hardware.ptrTxDatBuffer = &ptEndpointBD->usb_bd_tx_even.ptrUSB_BD_Data; // the buffer space associated with the even bufer
                }
                else {
                    usb_hardware.ptr_ulUSB_BDControl = &ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl; // use odd buffer for next transmission
                    usb_hardware.ptrTxDatBuffer = &ptEndpointBD->usb_bd_tx_odd.ptrUSB_BD_Data; // the buffer space associated with the odd bufer
                }
                uDisable_Interrupt();                                    // ensure interrupts remain blocked when handling the next possible transmission
    #if defined USB_HOST_SUPPORT
                    iHostDo = fnUSB_handle_frame(USB_TX_ACKED, 0, iEndpoint_ref, &usb_hardware); // handle tx ack event
    #else
                    fnUSB_handle_frame(USB_TX_ACKED, 0, iEndpoint_ref, &usb_hardware); // handle tx ack event
    #endif
                uEnable_Interrupt();                                     // allow higher priority interrupts again
    #if defined USB_HOST_SUPPORT
                if ((CTL & HOST_MODE_EN) == 0) {                         // if in device mode
    #endif
                    ADDR = usb_hardware.ucUSBAddress;                    // program the address to be used by device
    #if defined USB_HOST_SUPPORT
                }
                else {
                    if (INITIATE_IN_TOKEN == iHostDo) {                  // if IN token polling is to be initiated
                        if (iBusTurnaroundTimeout != 0) {                // {13} although the previous operation was handled as if it was successful (so that the generic host layer could be satisfied) the error must be corrected by retrying/reconnecting again
                            fnUSB_HostDelay(usb_possible_attach, PIT_MS_DELAY(100)); // wait a short time before checking that the device is still attached and repeating a reset sequence
                        }
                        else {
                            fnSendToken((unsigned char)((IN_PID << 4) | usb_hardware.ucHostEndpointActive)); // start transmission of IN token on the appropriate endpoint
                        }
                    }
                }
    #endif
            }
            else {                                                       // receive packet
    #if defined USB_DEVICE_SUPPORT
                usb_hardware.ulRxControl &= (CONTROL_DATA_TOGGLE_REVERSED); // {1} reset all but the reversed data toggle flag
    #endif
                usb_monitor("R", STAT);                                  // record activity for optional analysis purposes
                if ((STAT & ODD_BANK) != 0) {                            // check whether odd or even bank and update local flag
                    ptUSB_BD = &ptEndpointBD->usb_bd_rx_odd;             // received data in odd buffer
                    if ((CTL & HOST_MODE_EN) != 0) {                     // host always receives on endpoint 0
    #if defined USB_HOST_SUPPORT
                        fnUSB_HostDelay(0, 0);                           // stop SETUP transmission monitoring timer since the device has responded
                        usb_hardware.ptr_ulUSB_Rx_BDControl = &ptEndpointBD->usb_bd_rx_even.ulUSB_BDControl; // next reception will be in the even buffer
                        usb_hardware.ptr_ulUSB_Alt_Rx_BDControl = &ptEndpointBD->usb_bd_rx_odd.ulUSB_BDControl;
                        usb_hardware.ucBufferUsage &= ~(ODD_BANK);       // next reception will be in the even buffer
    #endif
                    }
                    else {
    #if defined USB_DEVICE_SUPPORT
                        usb_hardware.ulRxControl |= DATA_1;              // mark that the reception is in the odd buffer
    #endif
                    }
                }
                else {                                                   // reception is in the even buffer
                    ptUSB_BD = &ptEndpointBD->usb_bd_rx_even;            // data in even buffer
                    if ((CTL & HOST_MODE_EN) != 0) {                     // host always receives on endpoint 0
    #if defined USB_HOST_SUPPORT
                        fnUSB_HostDelay(0, 0);                           // stop SETUP transmission monitoring timer since the device has responded
                        usb_hardware.ptr_ulUSB_Rx_BDControl = &ptEndpointBD->usb_bd_rx_odd.ulUSB_BDControl; // next reception will be in the odd buffer
                        usb_hardware.ptr_ulUSB_Alt_Rx_BDControl = &ptEndpointBD->usb_bd_rx_even.ulUSB_BDControl;
                        usb_hardware.ucBufferUsage |= ODD_BANK;          // next reception will be in the odd buffer
    #endif
                    }
                  //else {                                               // {1}
    #if defined USB_DEVICE_SUPPORT
                  //    usb_hardware.ulRxControl &= ~(CONTROL_DATA_TOGGLE_REVERSED);
    #endif
                  //}
                }
    #if defined USB_DEVICE_SUPPORT
                usb_hardware.ptrRxDatBuffer = &(ptUSB_BD->ptrUSB_BD_Data);
    #endif
                usb_hardware.usLength = GET_FRAME_LENGTH();              // the length of the received frame
                switch (ptUSB_BD->ulUSB_BDControl & RX_PID_MASK) {
    #if defined USB_HOST_SUPPORT
                case (DATA1_PID << RX_PID_SHIFT):                        // host IN reception (DATA1) [ptUSB_BD->ulUSB_BDControl has DATA_1 set]
                case (DATA0_PID << RX_PID_SHIFT):                        // host IN reception (DATA0) [ptUSB_BD->ulUSB_BDControl has DATA_1 clear]
                    iEndpoint_ref = usb_hardware.ucHostEndpointActive;
    #endif
                case (OUT_PID << RX_PID_SHIFT):                          // OUT frame - for any endpoint
                    {
                        unsigned long ulDataToggle;
                        if ((usb_endpoints[iEndpoint_ref].ulEndpointSize & DTS) != 0) { // data toggling active on this endpoint (not isochronous)
                            ulDataToggle = (usb_endpoints[iEndpoint_ref].ulNextRxData0 & RX_DATA_TOGGLE); // the expected DATA
                            if ((ulDataToggle != 0) != ((ptUSB_BD->ulUSB_BDControl & DATA_1) != 0)) { // if the receiver is not synchronous to the data toggling it represents repeated data since an ACK from us has been lost
    #if defined _SUPRESS_REPEAT_DATA
                                unsigned long ulNextRxData0_required = usb_endpoints[iEndpoint_ref].ulNextRxData0;
                                fnProcessInput(iEndpoint_ref, &usb_hardware, USB_DATA_REPEAT, ptUSB_BD, ptEndpointBD);
                                usb_endpoints[iEndpoint_ref].ulNextRxData0 = ulNextRxData0_required; // ensure we remain synchron with read stream
                                break;
    #endif
                            }
                        }
                        else {
                            ulDataToggle = RX_DATA_TOGGLE;
                        }
                        if ((*fnGetEndPointCtr(iEndpoint_ref) & EP_CTL_DIS) != 0) { // check whether this endpoint is a non-control type
                            fnProcessInput(iEndpoint_ref, &usb_hardware, USB_OUT_FRAME, ptUSB_BD, ptEndpointBD); // non-control endpoint
                        }
                        else {                                           // control endpoint
                            fnProcessInput(iEndpoint_ref, &usb_hardware, USB_CONTROL_OUT_FRAME, ptUSB_BD, ptEndpointBD);
                        }
    #if defined USB_HOST_SUPPORT
                        if ((CTL & HOST_MODE_EN) != 0) {
                            usb_endpoints[iEndpoint_ref].ulNextRxData0 ^= (DATA_1); // invert the data token for next frame
                            break;
                        }
    #endif
    #if defined USB_DEVICE_SUPPORT
                        if (ulDataToggle != 0) {                         // synchronise the data toggle to detect repeated data
                            usb_endpoints[iEndpoint_ref].ulNextRxData0 &= ~(RX_DATA_TOGGLE);
                        }
                        else {
                            usb_endpoints[iEndpoint_ref].ulNextRxData0 |= RX_DATA_TOGGLE; 
                        }
    #endif
                    }
                    break;
    #if defined USB_DEVICE_SUPPORT
                case (SETUP_PID << RX_PID_SHIFT):                        // setup PID - on control endpoint (only received at device)
                    ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl = 0;    // disable all packet buffers
                    ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl  = 0;
                    // Synchronise the reception data buffers on each setup reception
                    //
                    if ((usb_hardware.ulRxControl & DATA_1) == 0) {      // setup received in even buffer
                        ptrBDT->usb_bd_rx_even.ulUSB_BDControl = ((usb_endpoints[iEndpoint_ref].ulEndpointSize & (USB_BYTE_CNT_MASK | DTS))); // prepare buffer for next reception but don't free until consumed (it will receive DATA0 OUT frames)
                        ptrBDT->usb_bd_rx_odd.ulUSB_BDControl = (ptrBDT->usb_bd_rx_even.ulUSB_BDControl | DATA_1 | OWN); // free alternate buffer ready for next reception (it will receive DATA1 OUT frames)
                        usb_hardware.ulRxControl = 0;                    // {1}
                    }
                    else {                                               // setup received in odd buffer
                        ptrBDT->usb_bd_rx_odd.ulUSB_BDControl = ((usb_endpoints[iEndpoint_ref].ulEndpointSize & (USB_BYTE_CNT_MASK | DTS))); // prepare buffer for next reception but don't free until consumed (it will receive DATA0 OUT frames)
                        ptrBDT->usb_bd_rx_even.ulUSB_BDControl = (ptrBDT->usb_bd_rx_odd.ulUSB_BDControl | DATA_1 | OWN); // free alternate buffer ready for next reception (it will receive DATA1 OUT frames)
                        usb_hardware.ulRxControl = (DATA_1 | CONTROL_DATA_TOGGLE_REVERSED); // {1} flag that data toggling is presently reversed
                    }
                    usb_endpoints[iEndpoint_ref].ulNextTxData0 = (DATA_1 | DTS); // always a data 1 packet following a setup packet
                    
                  //CTL &= ~TXSUSPEND_TOKENBUSY;                         // allow SIE to continue token processing

                    if (fnProcessInput(iEndpoint_ref, &usb_hardware, USB_SETUP_FRAME, ptUSB_BD, ptEndpointBD) != MAINTAIN_OWNERSHIP) {
                        usb_endpoints[iEndpoint_ref].ulNextRxData0 |= (RX_DATA_TOGGLE);
                    }
                    CTL &= ~TXSUSPEND_TOKENBUSY;                         // {83} allow SIE to continue token processing
                    break;
    #endif
    #if defined USB_HOST_SUPPORT
                case (STALL_PID << RX_PID_SHIFT):                        // the host has returned a stall
                    // Free the stalled reception buffer
                    //
                    if (*(usb_hardware.ptr_ulUSB_Rx_BDControl) & DATA_1) { // restore the buffer with correct DATA1 flag
                        ptUSB_BD->ulUSB_BDControl = 0;
                    }
                    else {
                        ptUSB_BD->ulUSB_BDControl = DATA_1;
                    }
                    ptUSB_BD->ulUSB_BDControl |= (OWN | (usb_endpoints[usb_hardware.ucHostEndpointActive].ulEndpointSize & (USB_BYTE_CNT_MASK | DTS))); // re-enable reception on this endpoint
                    usb_endpoints[usb_hardware.ucHostEndpointActive].ulNextRxData0 = ptUSB_BD->ulUSB_BDControl; // last buffer processed
                    fnUSB_handle_frame(USB_HOST_STALL_DETECTED, 0, (usb_hardware.ucHostEndpointActive | IN_ENDPOINT), &usb_hardware); // inform about the stall on this endpoint
                    CTL &= ~TXSUSPEND_TOKENBUSY;                         // allow SIE to continue token processing
                    break;

                case (ACK_PID << RX_PID_SHIFT):
                    fnUSB_handle_frame(USB_HOST_ACK_PID_DETECTED, 0, usb_hardware.ucHostEndpointActive, &usb_hardware); // error code 0
                    break;
                case (NAK_PID << RX_PID_SHIFT):                          // an IN token (or terminated IN polling sequence) received a NAK from the device
                  //if ((usb_endpoints[iEndpoint_ref].ulEndpointSize & CONTROL_HOST_INTERRUPT_ENDPOINT) != 0) {
                  //    fnUSB_handle_frame(USB_HOST_NACK_PID_DETECTED, 0, usb_hardware.ucHostEndpointActive, &usb_hardware); // error code 1 (we only report this on interrupt endpoints)
                  //}
                    fnFreeHostRx(iEndpoint_ref, ptUSB_BD);               // free the buffer that reported the error
                    break;
                case (0x00 << RX_PID_SHIFT):                             // bus timeout
                    fnUSB_handle_frame(USB_HOST_BUS_TIMEOUT_DETECTED, 0, usb_hardware.ucHostEndpointActive, &usb_hardware); // error code 2
                    break;
                case (0x0f << RX_PID_SHIFT):                             // data error (data token received in the present buffer doesn't match that expected)
                    fnUSB_handle_frame(USB_HOST_DATA_ERROR_DETECTED, 0, usb_hardware.ucHostEndpointActive, &usb_hardware); // error code 3
                    break;
    #endif
    #if defined USB_DEVICE_SUPPORT
                default:                                                 // if anything else is received it indicates an error so stall
                    ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl = (OWN | BDT_STALL); // stall the endpoint
                    ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl  = (OWN | BDT_STALL);
                    fnSetUSBEndpointState(iEndpoint_ref, USB_ENDPOINT_STALLED, 0);
                    break;
    #endif
                }
            }
            WRITE_ONE_TO_CLEAR_INTERRUPT(INT_STAT, TOK_DNE, irq_USB_OTG_ID); // clear the flag (don't clear until processing has completed to ensure that the STAT register stays valid)     
        }
    }
}

// Set the transmit control and buffer to next active one and check whether it is free
//
extern int fnGetUSB_HW(int iEndpoint, USB_HW **ptr_usb_hardware)
{
    KINETIS_USB_ENDPOINT_BD *ptEndpointBD;
#if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    if (UBS_HANDLE_TO_CHANNEL(iEndpoint) != 0) {
        return fnGetHSUSB_HW(iEndpoint, ptr_usb_hardware);
    }
    (*ptr_usb_hardware)->ucController = 0;
#endif
    if ((*fnGetEndPointCtr(iEndpoint) & EP_TX_ENABLE) == 0) {            // if the endpoint is not enabled for transmission
        return ENDPOINT_NOT_ACTIVE;
    }
    ptEndpointBD = ptrBDT;                                               // start of BDT
    if (__USB_HOST_MODE()) {                                             // in host mode
    #if defined USB_HOST_SUPPORT
        if ((*(usb_hardware.ptr_ulUSB_BDControl) & OWN) != 0) {          // check whether the USB controller owns the transmit buffer descriptor
            return ENDPOINT_NOT_FREE;                                    // endpoint is presently busy so no data may be copied
        }
        *ptr_usb_hardware = &usb_hardware;                               // set the caller's pointer to the usb hardware structure
        if (iEndpoint != usb_hardware.ucHostEndpointActive) {            // if change of endpoint
            ptEndpointBD += iEndpoint;
            usb_hardware.ptrEndpoint = &usb_endpoints[iEndpoint];        // the endpoint's characteristics
            usb_hardware.ucHostEndpointActive = iEndpoint;               // the newly active endpoint
            if ((usb_endpoints[iEndpoint].ulNextRxData0 & DATA_1) != 0) {
                *(usb_hardware.ptr_ulUSB_Rx_BDControl) |= DATA_1;
                *(usb_hardware.ptr_ulUSB_Alt_Rx_BDControl) &= ~(DATA_1);
            }
            else {
                *(usb_hardware.ptr_ulUSB_Rx_BDControl) &= ~(DATA_1);
                *(usb_hardware.ptr_ulUSB_Alt_Rx_BDControl) |= DATA_1;
            }
        }
    #endif
    }
    else {
    #if defined USB_DEVICE_SUPPORT
        USB_END_POINT *ptrThisEndpoint;
        unsigned long *ptrThisControl;
        unsigned char **ptrThisBuffer;
        ptEndpointBD += iEndpoint;                                       // move to the buffer descriptor for this endpoint
        ptrThisEndpoint = &usb_endpoints[iEndpoint];
        (*ptr_usb_hardware)->ptrEndpoint = ptrThisEndpoint;              // this is modified even if the endpoint is not free
        uDisable_Interrupt();                                            // protect from interrupts
            if ((ptrThisEndpoint->ulEndpointSize & ALTERNATE_TX_BUFFER) == 0) { // the transmit buffer to be used
                ptrThisControl = &ptEndpointBD->usb_bd_tx_even.ulUSB_BDControl;
                ptrThisBuffer = &ptEndpointBD->usb_bd_tx_even.ptrUSB_BD_Data;
            }
            else {
                ptrThisControl = &ptEndpointBD->usb_bd_tx_odd.ulUSB_BDControl;
                ptrThisBuffer = &ptEndpointBD->usb_bd_tx_odd.ptrUSB_BD_Data;
            }
            if ((*ptrThisControl & OWN) != 0) {                          // check whether the USB controller owns the transmit buffer descriptor
                uEnable_Interrupt();
                return ENDPOINT_NOT_FREE;                                // endpoint is presently busy so no data may be copied
            }    
            (*ptr_usb_hardware)->ptr_ulUSB_BDControl = ptrThisControl;   // update the hardware pointers (only when the endpoint is free)
            (*ptr_usb_hardware)->ptrTxDatBuffer = ptrThisBuffer;
            #if defined USB_HOST_SUPPORT
            (*ptr_usb_hardware)->ucModeType = usb_hardware.ucModeType;
            #endif
        uEnable_Interrupt();
    #endif
    }
    return ENDPOINT_FREE;                                                // endpoint transmit buffer is free
}

// The hardware interface used to activate USB endpoints
//
extern void fnActivateHWEndpoint(unsigned char ucEndpointType, unsigned char ucEndpointRef, unsigned short usEndpointLength, unsigned short usMaxEndpointLength, USB_ENDPOINT *ptrEndpoint)
{
    KINETIS_USB_ENDPOINT_BD *ptrEndpointBDT = ptrBDT;
    unsigned long  ulControlContent = (DTS | DATA_1);                    // prepare Data Toggle Synchronisation as default
    unsigned char *ptrEndPointCtr = fnGetEndPointCtr(ucEndpointRef);     // get a pointer to the endpoint control register
    unsigned char  endpoint_config;
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    if (UBS_HANDLE_TO_CHANNEL(ucEndpointRef) != 0) {                     // if HS USB is to be configured
        fnActivateHS_HWEndpoint(ucEndpointType, ucEndpointRef, usEndpointLength, usMaxEndpointLength, ptrEndpoint); // use HS USB interface
        return;                                                          // return since the HS USB action has been complete
    }
    #endif
    ptrEndpointBDT += ucEndpointRef;                                     // set pointer to the endpoint's buffer descriptor
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
    if (ENDPOINT_DISABLE == ucEndpointType) {                            // if this endpoint is to be disabled
        *ptrEndPointCtr = 0;                                             // disable endpoint transmitter and receiver
        while (*ptrEndPointCtr & EP_TX_ENABLE) {}                        // wait for tx disable to complete
        ptrEndpointBDT->usb_bd_tx_odd.ulUSB_BDControl = 0;               // reset endpoint buffer descriptor control entries
        ptrEndpointBDT->usb_bd_tx_even.ulUSB_BDControl = 0;
        ptrEndpointBDT->usb_bd_rx_odd.ulUSB_BDControl = 0;
        ptrEndpointBDT->usb_bd_rx_even.ulUSB_BDControl = 0;
        usb_endpoints[ucEndpointRef].ulEndpointSize = 0;
        usb_endpoints[ucEndpointRef].ulNextRxData0 = 0;
        INT_ENB &= ~(SLEEP_EN | RESUME_EN);                              // disable suspend and resume interrupts when not configured
        *ptrEndPointCtr = 0;
        return;                                                          // disabled so return
    }
    INT_ENB |= SLEEP_EN;                                                 // since we have just been configured, enable suspend interrupt
    endpoint_config = *ptrEndPointCtr;                                   // get the original configuration setting

    if ((ucEndpointType & ~IN_ENDPOINT) == ENDPOINT_ISOCHRONOUS) {       // is this an isochronous endpoint?
        endpoint_config = (RETRY_DIS | EP_TX_ENABLE);                    // no retries on isochronous endpoint
        ulControlContent = 0;                                            // disable data toggle synchronisation on receiver
        usb_endpoints[ucEndpointRef].ulNextTxData0 = 0;                  // disable data toggle synchronisation on transmitter
    }
    else {
        usb_endpoints[ucEndpointRef].ulNextTxData0 = DTS;                // set data toggle synchronisation on transmission
        endpoint_config |= (EP_HSHK);                                    // bulk/control/interrupt endpoint - handshake enabled
    }
        #if defined USB_HOST_SUPPORT
    if ((usb_hardware.ucModeType & USB_HOST_MODE) != 0) {
        ucEndpointType ^= IN_ENDPOINT;                                   // in host mode reverse the direction of the trasnmission/reception
    }
        #endif
    if ((ucEndpointType & IN_ENDPOINT) == 0) {                           // OUT type endpoint
        if (ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data == 0) {        // if the endpoint is being used for the first time
            if (usMaxEndpointLength > usEndpointLength) {                // if no larger specified take the specified value
                usEndpointLength = usMaxEndpointLength;
            }
        #if defined USB_DEVICE_SUPPORT
            if (__USB_DEVICE_MODE()) {
                ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data = (unsigned char *)USB_FS_MALLOC(usEndpointLength); // alloc buffer memory for control endpoint
                ptrEndpointBDT->usb_bd_rx_odd.ptrUSB_BD_Data  = (unsigned char *)USB_FS_MALLOC(usEndpointLength);
            }
        #endif
        }
        endpoint_config |= EP_RX_ENABLE;                                 // enable reception on this endpoint
        ulControlContent |= SET_FRAME_LENGTH(usEndpointLength);          // set endpoint rx size                                     
        ptrEndpointBDT->usb_bd_rx_odd.ulUSB_BDControl = (ulControlContent | OWN); // odd buffer for DATA1 frames
        ulControlContent &= ~DATA_1;                                     // even buffer for DATA0 frames
        ptrEndpointBDT->usb_bd_rx_even.ulUSB_BDControl = (ulControlContent | OWN); // allow endpoints to receive
        #if defined USB_HOST_SUPPORT
        if ((usb_hardware.ucModeType & USB_HOST_MODE) != 0) {            // {4}
            usb_endpoints[ucEndpointRef].ulNextRxData0 = 0;              // all endpoints initially receive DATA0
            if ((ucEndpointType & ~IN_ENDPOINT) == ENDPOINT_INTERRUPT) {
                ulControlContent |= CONTROL_HOST_INTERRUPT_ENDPOINT;     // {10} flag that this endpoint is an interrupt endpoint and so the host doesn't send IN tokens to it that will repeat on NAK
            }
        }
        #endif
        usb_endpoints[ucEndpointRef].ulEndpointSize = ulControlContent;
    }
    else {
        endpoint_config |= EP_TX_ENABLE;                                 // enable transmission on IN endpoint
    }
    if ((ucEndpointType & ~(IN_ENDPOINT)) != ENDPOINT_CONTROL) {         // non-control endpoint
        endpoint_config |= EP_CTL_DIS;                                   // not control endpoint
    }
    #else
    *ptrEndPointCtr &= ~(EP_TX_ENABLE | EP_RX_ENABLE);                   // disable endpoint transmitter and receiver
    _WAIT_REGISTER_TRUE(*ptrEndPointCtr, EP_TX_ENABLE);                  // wait for tx disable to complete
    ptrEndpointBDT->usb_bd_tx_odd.ulUSB_BDControl = 0;                   // reset endpoint buffer descriptor control entries
    ptrEndpointBDT->usb_bd_tx_even.ulUSB_BDControl = 0;
    ptrEndpointBDT->usb_bd_rx_odd.ulUSB_BDControl = 0;
    ptrEndpointBDT->usb_bd_rx_even.ulUSB_BDControl = 0;
    usb_endpoints[ucEndpointRef].ulEndpointSize = 0;
    usb_endpoints[ucEndpointRef].ulNextRxData0 = 0;

    if (ENDPOINT_DISABLE == ucEndpointType) {                            // if this endpoint is to be disabled
        INT_ENB &= ~(SLEEP_EN | RESUME_EN);                              // disable suspend and resume interrupts when not configured
        *ptrEndPointCtr = 0;
        return;                                                          // disabled so return
    }
    INT_ENB |= SLEEP_EN;                                                 // since we have just been configured, enable suspend interrupt

    if ((ucEndpointType & ~IN_ENDPOINT) == ENDPOINT_ISOCHRONOUS) {       // is this an isochronous endpoint?
        endpoint_config = (RETRY_DIS | EP_TX_ENABLE);                    // no retries on isochronous endpoint
        ulControlContent = 0;                                            // disable data toggle synchronisation on receiver
        usb_endpoints[ucEndpointRef].ulNextTxData0 = 0;                  // disable data toggle synchronisation on transmitter
    }
    else {
        usb_endpoints[ucEndpointRef].ulNextTxData0 = DTS;                // set data toggle synchronisation on transmission
        endpoint_config = (EP_HSHK | EP_TX_ENABLE);                      // bulk/control/interrupt endpoint - handshake enabled
    }
        #if defined USB_HOST_SUPPORT
    if ((usb_hardware.ucModeType & USB_HOST_MODE) != 0) {
        ucEndpointType ^= IN_ENDPOINT;                                   // in host mode reverse the direction of the transmission/reception
    }
        #endif
    if ((ucEndpointType & IN_ENDPOINT) == 0) {                           // OUT type endpoint
        if (ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data == 0) {        // if the endpoint is being used for the first time
            if (usMaxEndpointLength > usEndpointLength) {                // if no larger specified, take the value
                usEndpointLength = usMaxEndpointLength;
            }
        #if defined USB_DEVICE_SUPPORT
            if (__USB_DEVICE_MODE()) {
                if ((ptrEndpoint->usParameters & USB_OUT_ZERO_COPY) != 0) { // {2} use zero copy type of endpoint buffer
                    QUEQUE *ptrQueue = (QUEQUE *)(ptrEndpoint->ptrEndpointOutCtr);
                    ptrQueue->new_chars = 0;
                    ptrQueue->put = ptrQueue->QUEbuffer;
                    ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data = ptrQueue->QUEbuffer; // set descriptor data pointer to start of linear reception buffer
                    ptrEndpointBDT->usb_bd_rx_odd.ptrUSB_BD_Data = (ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data + usEndpointLength); // next descriptor's data pointer is set to the next block in the linear reception buffer
                }
                else {
                    ptrEndpointBDT->usb_bd_rx_even.ptrUSB_BD_Data = (unsigned char *)USB_FS_MALLOC(usEndpointLength); // allocate buffer memory for control endpoint
                    ptrEndpointBDT->usb_bd_rx_odd.ptrUSB_BD_Data  = (unsigned char *)USB_FS_MALLOC(usEndpointLength);
                }
            }
        #endif
        }
        endpoint_config |= EP_RX_ENABLE;                                 // enable reception on this endpoint
        ulControlContent |= SET_FRAME_LENGTH(usEndpointLength);          // set endpoint rx size                                     
        ptrEndpointBDT->usb_bd_rx_odd.ulUSB_BDControl = (ulControlContent | OWN); // odd buffer for DATA1 frames
        ulControlContent &= ~DATA_1;                                     // even buffer for DATA 0 frames
        ptrEndpointBDT->usb_bd_rx_even.ulUSB_BDControl = (ulControlContent | OWN); // allow endpoints to receive
        usb_endpoints[ucEndpointRef].ulEndpointSize = ulControlContent;
    }
    if ((ucEndpointType & ~IN_ENDPOINT) != ENDPOINT_CONTROL) {           // non-control endpoint
        endpoint_config |= EP_CTL_DIS;                                   // not control endpoint
        if ((ucEndpointType & IN_ENDPOINT) == 0) {                       // out type
            endpoint_config &= ~EP_TX_ENABLE;                            // disable transmission
        }
    }
    #endif
    *ptrEndPointCtr = endpoint_config;                                   // set the endpoint mode
    _SIM_USB(0, USB_SIM_ENUMERATED, 0, 0);                               // inform the simulator that USB enumeration has completed
}

// This routine is called with interrupts blocked for deferred read of a reception buffer
//
extern int fnConsumeUSB_out(unsigned char ucEndpointRef)
{
    unsigned short usLength;
    unsigned long ulBuffer;
    KINETIS_USB_BD *ptUSB_BD;                                             // specific BD
    KINETIS_USB_ENDPOINT_BD *ptEndpointBD = ptrBDT;                       // start of BDT
    ptEndpointBD += ucEndpointRef;
    if ((usb_endpoints[ucEndpointRef].ulNextRxData0 & DATA_1) != 0) {
        ptUSB_BD = &ptEndpointBD->usb_bd_rx_even;
        ulBuffer = 0;
    }
    else {
        ptUSB_BD = &ptEndpointBD->usb_bd_rx_odd;
        ulBuffer = DATA_1;
    }
    if ((ptUSB_BD->ulUSB_BDControl & OWN) != 0) {                        // no data available
        return USB_BUFFER_NO_DATA;
    }
    usLength = GET_FRAME_LENGTH();                                       // the number of bytes waiting in the buffer
    if (fnEndpointData(ucEndpointRef, ptUSB_BD->ptrUSB_BD_Data, usLength, OUT_DATA_RECEPTION, &usb_hardware) != MAINTAIN_OWNERSHIP) {
        ulBuffer |= (usb_endpoints[ucEndpointRef].ulEndpointSize & (USB_BYTE_CNT_MASK | DTS));// reception buffer size
        ptUSB_BD->ulUSB_BDControl = (ulBuffer | OWN);                    // re-enable reception on this endpoint
        usb_endpoints[ucEndpointRef].ulNextRxData0 &= (RX_DATA_TOGGLE);  // don't disturb this flag
        usb_endpoints[ucEndpointRef].ulNextRxData0 |= ulBuffer;          // last buffer processed
        return USB_BUFFER_FREED;                                         // buffer consumed and freed
    }
    return USB_BUFFER_BLOCKED;
}

// USB Configuration
//
extern void fnConfigUSB(QUEUE_HANDLE Channel, USBTABLE *pars)
{
    #if defined USB_BDT_FIXED
        #if defined _WINDOWS
    static KINETIS_USB_ENDPOINT_BD USB_BDT_RAM[NUMBER_OF_USB_ENDPOINTS * 2];
    static KINETIS_USB_ENDPOINT_BD *__USB_BDT_RAM = USB_BDT_RAM;
        #else
    extern KINETIS_USB_ENDPOINT_BD __USB_BDT_RAM[];
        #endif
    #endif
    #if (defined KINETIS_K64 || (defined KINETIS_K24 && (SIZE_OF_FLASH == (1024 * 1024)))) && (defined RUN_FROM_HIRC_PLL || defined RUN_FROM_HIRC)
    int iIRC48M_workaround = 0;                                          // mark that we need to temporarily switch system clock source during the USB reset command
    unsigned char original_MCG_C1 = MCG_C1;
    unsigned char original_MCG_C2 = MCG_C2;
    unsigned char original_MCG_SC = MCG_SC;
    #endif
    unsigned char ucEndpoints;
    unsigned char ucEP0_size;
    #if defined USE_USB_OTG_CHARGE_PUMP
    QUEUE_HANDLE i2c_handle;
    #endif
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    if (Channel != 0) {
        fnConfigHSUSB(0, pars);                                          // pass on to high speed driver
        return;                                                          // return since HS USB configuration has been completed
    }
    #endif
    ucEndpoints = (pars->ucEndPoints + 1);                               // endpoint count, including endpoint 0
    #if !defined KINETIS_KL
    FMC_PFAPR |= FMC_FPAPR_USB_FS;                                       // allow USB controller to read from flash
    #endif
    #if !defined KINETIS_KL82 && !(defined KINETIS_K22 && (SIZE_OF_RAM == (24 * 1024))) // {3} KL82 doesn't have regulator control
    SIM_SOPT1_SET(SIM_SOPT1_USBREGEN, SIM_SOPT1CFG_URWE);                // ensure USB regulator is enabled
    SIM_SOPT1_CLR(SIM_SOPT1_USBSTBY, SIM_SOPT1CFG_UVSWE);                // and not in standby
    #endif
    switch (pars->ucClockSource) {                                       // set USB clock source
    case INTERNAL_USB_CLOCK:
    #if (defined KINETIS_K_FPU || (KINETIS_MAX_SPEED > 100000000)) && !defined KINETIS_K21 && !defined KINETIS_K22 && !defined KINETIS_K24 && !defined KINETIS_K26 && !defined KINETIS_K64 && !defined KINETIS_K65 && !defined KINETIS_K66 && !defined KINETIS_K80
        #if defined USB_CLOCK_SOURCE_MCGPLL0CLK
            #define USB_CLOCK_SOURCE   MCGPLLCLK
        SIM_SOPT2 |= (SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_USBFSRC_MCGPLL0); // set the source to MCGPLL0CLK
        #elif defined USB_CLOCK_SOURCE_MCGPLL1CLK                        // {90} use PLL1, dedicated for peripheral use
            #define USB_CLOCK_SOURCE   MCGPLL1CLK                        // use PLL1
        SIM_SOPT2 |= (SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_USBFSRC_MCGPLL1); // set the source to MCGPLL1CLK
        #else
            #define USB_CLOCK_SOURCE   MCGPLLCLK
        SIM_SOPT2 |= (SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_USBFSRC_MCGPLLCLK); // set the source to MCGPLLCLK
        #endif
    #elif defined KINETIS_HAS_IRC48M && defined USB_CRYSTAL_LESS         // {104}
        #if defined KINETIS_WITH_PCC
            #define USB_CLOCK_SOURCE  (FIRC_CLK)
            #if USB_CLOCK_SOURCE != 48000000
                #error "USB clock must be 48MHz in order to use crystal-less mode"
            #endif
        SELECT_PCC_PERIPHERAL_SOURCE(USB0FS, PCC_PCS_SCGFIRCLK);         // set the source to fast IRC
        #else
            #define USB_CLOCK_SOURCE   48000000
        SIM_SOPT2 |= (SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL_IRC48M);    // set the source to IRC48M
        #endif
    #else
        #define USB_CLOCK_SOURCE   MCGPLLCLK                             // fixed clock source for USB
        SIM_SOPT2 |= (SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL);           // set the source to MCGPLLCLK
        #if defined _WINDOWS && defined KINETIS_HAS_IRC48M               // check that the clock rate programmed is accurate for USB
        if ((SIM_SOPT2 & SIM_SOPT2_PLLFLLSEL_IRC48M) != SIM_SOPT2_PLLFLLSEL_PLL) {
            _EXCEPTION("IRC48M has been selected by another peripheral - all peripherals must be set to use MCGPLLCLK for USB compatibility!!");
        }
        #endif
    #endif
    #if !defined SIM_CLKDIV2                                             // {8}
        #if defined KINETIS_HAS_IRC48M && defined USB_CRYSTAL_LESS       // {104}
        #elif defined _WINDOWS
            #if (MCGPLLCLK/2) != 48000000                                // check that the clock has the correct frequency for USB operation
                #error "USB clock not 48MHz!!"
            #endif
        #endif
    #else
        #if defined KINETIS_HAS_IRC48M && defined USB_CRYSTAL_LESS       // {104}
        SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV_1;                              // no divide when IRC48M is used
        #elif (USB_CLOCK_SOURCE/48000000) > 4
        SIM_CLKDIV2 |= (((USB_CLOCK_SOURCE/48000000) - 1) << 1);         // the value of the clock source must be chosen to allow 48MHz to be achieved - possible without fraction
        #else
        SIM_CLKDIV2 |= (((((USB_CLOCK_SOURCE * 2)/48000000) - 1) << 1) | SIM_CLKDIV2_USBFRAC); // the value of the clock source must be chosen to allow 48MHz to be achieved!
        #endif
        #if defined _WINDOWS                                             // check that the clock rate programmed is accurate for USB
        if ((SIM_CLKDIV2 & SIM_CLKDIV2_USBFRAC) != 0) {
            if (((USB_CLOCK_SOURCE/(((SIM_CLKDIV2 >> 1) & 0x07) + 1)) * 2) != 48000000) {
                unsigned long _USB_CLOCK_SOURCE = USB_CLOCK_SOURCE;
                unsigned long _SIM_CLKDIV2 = SIM_CLKDIV2;
                _EXCEPTION("USB Clock not 48MHz!!!!!!");
            }
        }
        else {
            if ((USB_CLOCK_SOURCE/(((SIM_CLKDIV2 >> 1) & 0x07) + 1)) != 48000000) {
                unsigned long _USB_CLOCK_SOURCE = USB_CLOCK_SOURCE;
                unsigned long _SIM_CLKDIV2 = SIM_CLKDIV2;
                _EXCEPTION("USB Clock not 48MHz!!!!!!");
            }
        }
        #endif
    #endif
        break;
  //case EXTERNAL_USB_CLOCK:
  //case SPECIAL_USB_CLOCK:                                              // use alternative clock input pin
    default:
    #if defined USB_CLK_ON_PORT_A
        _CONFIG_PERIPHERAL(A, 5, PA_5_USB_CLKIN);                        // USB_CLKIN on PTA5 (alt. function 2)
    #else
        _CONFIG_PERIPHERAL(E, 26, PE_26_USB_CLKIN);                      // USB_CLKIN on PTE26 (alt. function 7)
    #endif
    #if defined KINETIS_WITH_PCC
        SELECT_PCC_PERIPHERAL_SOURCE(USB0FS, PCC_PCS_CLOCK_OFF);         // disable interal sources so that the external one is used
    #else
        SIM_SOPT2 &= ~(SIM_SOPT2_USBSRC);                                // select external bypass clock (USB_CLKIN)
    #endif
        break;
    }
    #if defined MPU_AVAILABLE                                            // devices with memory protection unit
    MPU_CESR = 0;                                                        // allow concurrent access to MPU controller
    #endif
    #if (defined KINETIS_K64 || (defined KINETIS_K24 && (SIZE_OF_FLASH == (1024 * 1024)))) && (defined RUN_FROM_HIRC_PLL || defined RUN_FROM_HIRC)
    if (IS_POWERED_UP(4, USBOTG)) {                                      // if the USB module is already powered up it means that it has to be enabled to turn on the IRC48M
        iIRC48M_workaround = 1;                                          // mark that we need to temporarily switch system clock source during the USB reset command
    }
    #endif
    POWER_UP_ATOMIC(4, USBOTG);                                          // power up the USB controller module

    if (ucEndpoints > NUMBER_OF_USB_ENDPOINTS) {                         // limit endpoint count
        ucEndpoints = NUMBER_OF_USB_ENDPOINTS;                           // limit to maximum available in device
    }

    usb_endpoints = uMalloc((MAX_MALLOC)(sizeof(USB_END_POINT) * ucEndpoints)); // get endpoint control structures

    #if (defined KINETIS_K64 || (defined KINETIS_K24 && (SIZE_OF_FLASH == (1024 * 1024)))) && (defined RUN_FROM_HIRC_PLL || defined RUN_FROM_HIRC)
    // Early devices may disable the IRC48M when the reset command is executed so we need to switch to a temporary clock source and switch back again once completed
    //
    if (iIRC48M_workaround != 0) {                                       // oder K64 device which disables the IRC48M when the USB reset command is executed
        // Temporarily move to an alternative clock source
        //
        #if defined RUN_FROM_HIRC_PLL                                    // operating from PLL (PEE state)
        MCG_C1 = (MCG_C1_CLKS_EXTERN_CLK | MCG_C1_FRDIV_1280);           // move from PEE to PBE
        while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_EXTERN_CLK) {   // loop until the external clock is selected
            #if defined _WINDOWS
            MCG_S &= ~MCG_S_CLKST_MASK;
            MCG_S |= MCG_S_CLKST_EXTERN_CLK;
            #endif
        }
        MCG_C6 = (CLOCK_MUL - MCG_C6_VDIV0_LOWEST);                      // move to FBE
        while ((MCG_S & MCG_S_PLLST) != 0) {                             // loop until the PLLS clock is no longer valid
                #if defined _WINDOWS
            MCG_S &= ~MCG_S_PLLST;
                #endif
        }
        #endif
        MCG_SC &= ~(MCG_SC_FCRDIV_128);                                  // remove any divide values to achieve 4MHz
        MCG_C2 |= MCG_C2_IRCS;                                           // ensure fast internal clock is selected for MCGIRCLK
        MCG_C1 = ((MCG_C1 & ~MCG_C1_CLKS_EXTERN_CLK) | MCG_C1_CLKS_INTERN_CLK | MCG_C1_IRCLKEN); // switch MCGOUTCLK to the fast internal clock
        while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_INTERN_CLK) {   // wait for the output to be set
            #if defined _WINDOWS
            MCG_S &= ~MCG_S_CLKST_MASK;
            MCG_S |= MCG_S_CLKST_INTERN_CLK;
            #endif
        }
        // We are now in FBI state with the sytem clock driven by the 4MHz IRC
        //
    }
    #endif
    USB_USBTRC0 |= USB_USBTRC0_USBRESET;                                 // reset USB controller
    while ((USB_USBTRC0 & USB_USBTRC0_USBRESET) != 0) {                  // wait for the reset to complete
    #if defined _WINDOWS
        USB_USBTRC0 = 0;
    #endif
    }
    if ((pars->usConfig & USB_HOST_MODE) == 0) {                         // only USB device can use crystal-less mode
    #if defined KINETIS_HAS_IRC48M && defined USB_CRYSTAL_LESS           // {104}
        #if defined USB_CLK_RECOVER_INT_EN
        USB_CLK_RECOVER_INT_EN = 0;                                      // disable the clock synchronisation overflow error interrupt which is enabled by default
        #endif
        USB_CLK_RECOVER_IRC_EN = USB_CLK_RECOVER_IRC_EN_IRC_EN;          // enable 48MHz IRC clock and clock recovery (this may have been disabled by the USB reset command)
        USB_CLK_RECOVER_CTRL = USB_CLK_RECOVER_CTRL_CLOCK_RECOVER_EN;
    #elif (defined KINETIS_K64 || (defined KINETIS_K24 && (SIZE_OF_FLASH == (1024 * 1024)))) && (defined RUN_FROM_HIRC_PLL || defined RUN_FROM_HIRC)
        if (iIRC48M_workaround != 0) {
            USB_CLK_RECOVER_IRC_EN = (USB_CLK_RECOVER_IRC_EN_REG_EN | USB_CLK_RECOVER_IRC_EN_IRC_EN); // the IRC48M is only usable when enabled via the USB module so re-enable since it was disabled by the USB reset command
        }
    #endif
    #if (defined KINETIS_K64 || (defined KINETIS_K24 && (SIZE_OF_FLASH == (1024 * 1024)))) && (defined RUN_FROM_HIRC_PLL || defined RUN_FROM_HIRC)
        if (iIRC48M_workaround != 0) {
            // Move back to original clock source
            //
        #if defined RUN_FROM_HIRC_PLL                                    // we are in FBI state and must move to FBE
            MCG_C1 = (MCG_C1_CLKS_EXTERN_CLK | MCG_C1_FRDIV_1280);       // switch the external clock source also to the FLL to satisfy the PBE state requirement
            MCG_C5 = ((CLOCK_DIV - 1) | MCG_C5_PLLSTEN0);                // PLL remains enabled in normal stop modes
            MCG_C6 = ((CLOCK_MUL - MCG_C6_VDIV0_LOWEST) | MCG_C6_PLLS);  // complete PLL configuration and move to PBE
            while ((MCG_S & MCG_S_PLLST) == 0) {                         // loop until the PLLS clock source becomes valid
            #if defined _WINDOWS
                MCG_S |= MCG_S_PLLST;
            #endif
            }
            while ((MCG_S & MCG_S_LOCK) == 0) {                          // loop until PLL locks
            #if defined _WINDOWS
                MCG_S |= MCG_S_LOCK;
            #endif
            }
            MCG_C1 = (MCG_C1_CLKS_PLL_FLL | MCG_C1_FRDIV_1024);          // finally move from PBE to PEE mode - switch to PLL clock
            while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST_PLL) {      // loop until the PLL clock is selected
            #if defined _WINDOWS
                MCG_S &= ~MCG_S_CLKST_MASK;
                MCG_S |= MCG_S_CLKST_PLL;
            #endif
            }
        #else
            MCG_C1 = ((MCG_C1 & ~MCG_C1_CLKS_INTERN_CLK) | (original_MCG_C1 & MCG_S_CLKST_EXTERN_CLK)); // switch MCGOUTCLK back to the original clock source
            while ((MCG_S & MCG_S_CLKST_MASK) != (original_MCG_C1 & MCG_S_CLKST_EXTERN_CLK)) { // wait for the output to be set
            #if defined _WINDOWS
                MCG_S &= ~MCG_S_CLKST_MASK;
                MCG_S |= (original_MCG_C1 & MCG_S_CLKST_EXTERN_CLK);
            #endif
            }
        #endif
            MCG_C2 = original_MCG_C2;                                    // return original clock settings
            MCG_SC = original_MCG_SC;
            MCG_C1 = original_MCG_C1;
        }
    #endif
    }
    #if defined _WINDOWS && defined KINETIS_HAS_IRC48M && defined USB_CRYSTAL_LESS
    else {
        _EXCEPTION("USB FS Host cannot operate correctly in crystal-less mode!!");
    }
    #endif

    CTL = ODD_RST;                                                       // reset all odd BDTs which specifies the even bank
  //USB_CTRL = PDE;                                                      // take transceiver out of suspend mode (monitor 2.2.2014)

    #if defined USB_BDT_FIXED
        #if defined _WINDOWS                                             // set buffer to 512 byte aligned boundary
    __USB_BDT_RAM = (KINETIS_USB_ENDPOINT_BD *)(((CAST_POINTER_ARITHMETIC)__USB_BDT_RAM) & 0xffffff00);
    if (__USB_BDT_RAM < USB_BDT_RAM)  {
        __USB_BDT_RAM = (KINETIS_USB_ENDPOINT_BD *)(((CAST_POINTER_ARITHMETIC)(__USB_BDT_RAM + NUMBER_OF_USB_ENDPOINTS)) & 0xffffff00);
    }
        #endif
    ptrBDT = __USB_BDT_RAM;
    uMemset(ptrBDT, 0x00, (ucEndpoints * ENDPOINT_BD_SIZE));
    #else
    ptrBDT = uMallocAlign((MAX_MALLOC)(ucEndpoints * ENDPOINT_BD_SIZE), 512); // get BDT - buffer descriptor table memory (zeroed) - aligned to 512 byte boundars
    #endif

    BDT_PAGE_01 = (unsigned char)((CAST_POINTER_ARITHMETIC)ptrBDT >> 8); // enter the BDT location
    BDT_PAGE_02 = (unsigned char)((CAST_POINTER_ARITHMETIC)ptrBDT >> 16);
    BDT_PAGE_03 = (unsigned char)((CAST_POINTER_ARITHMETIC)ptrBDT >> 24);

    ADDR = 0;                                                            // set default address
    CTL = 0;                                                             // reset complete
                                                                         
    ENDPT0 = 0;                                                          // configure endpoint 0 - the only one used during enumeration: USB then in default state
    while ((ENDPT0 & EP_TX_ENABLE) != 0) {}                              // wait for tx disable to complete
                                                                         // configure rx side of endpoint
    if ((pars->usConfig & USB_HOST_MODE) != 0) {                         // host mode
        ucEP0_size = 64;                                                 // full size endpoint 0 since it is used by the host for all transmission/reception (note that high speed would need 1024 bytes to be able to opeate with max. HS isochronous length)
    }
    else {
        ucEP0_size = ENDPOINT_0_SIZE;                                    // device's endpoint 0 size
    }
    ptrBDT->usb_bd_rx_even.ptrUSB_BD_Data  = (unsigned char *)uMalloc(ucEP0_size); // alloc buffer memory for control endpoint (the size must be full-sized since this is used for all endpoints in host mode)
    ptrBDT->usb_bd_rx_odd.ptrUSB_BD_Data   = (unsigned char *)uMalloc(ucEP0_size);
    ptrBDT->usb_bd_rx_odd.ulUSB_BDControl  = (SET_FRAME_LENGTH(ucEP0_size) | DTS | OWN); // set endpoint rx size and configure Data Toggle Synchronisation (receive DATA 0 frames)
    ptrBDT->usb_bd_rx_even.ulUSB_BDControl = (ptrBDT->usb_bd_rx_odd.ulUSB_BDControl | DATA_1); // set endpoint rx size and configure Data Toggle Synchronisation (receive DATA 1 frames)
    usb_endpoints[0].ulEndpointSize = (SET_FRAME_LENGTH(ucEP0_size) | DTS);
    usb_endpoints[0].ulNextRxData0 = RX_DATA_TOGGLE;                     // DATA 1 is always first reception (as response to SETUP)
    ENDPT0 = (EP_HSHK | EP_RX_ENABLE | EP_TX_ENABLE);                    // enable control endpoint transmission and reception

    OTG_INT_STAT = (A_VBUS_CHG | BSESS_CHG | SESS_VLD_CHG | LINE_STAT_CHG | MSEC_1 | ID_CHG); // reset any pending OTG interrupts
    INT_STAT = (STALL | ATTACH | RESUME | SLEEP | TOK_DNE | SOF_TOK | USB_ERROR | USB_RST); // reset any pending USB interrupts
    ERR_STAT = (PID_ERR | CRC5_EOF | CRC16 | DFN8 | BTO_ERR | USB_DMA_ERR | BTS_ERR); // reset any pending USB error interrupts
    #if defined _WINDOWS
    INT_STAT = ERR_STAT = 0;
    #endif

    fnEnterInterrupt(irq_USB_OTG_ID, PRIORITY_USB_OTG, _usb_otg_isr);    // configure and enter the USB handling interrupt routine in the vector table

    if ((pars->usConfig & USB_HOST_MODE) != 0) {                         // host mode
    #if defined USB_HOST_SUPPORT
        CTL = HOST_MODE_EN;                                              // enable host mode operation (pull-down resistors enabled) but don't generate SOF (SOF counter loaded with 12000)
        USB_CTRL = PDE;                                                  // take transceiver out of suspend mode with pull-down resistors enabled
        OTG_CTRL = (DP_LOW_DM_LOW | OTG_EN);                             // pull down D+ and D- with weak pulldowns
        SOF_THLD = 74;                                                   // worst case threshold value (in byte times) for 64 bytes (32 byte packets could use 42, 16 byte packets 26 and 8 byte packets 18)
        INT_ENB = ATTACH_EN;                                             // enable an interrupt on the attach of a device
        USB_HOST_POWER_ON();                                             // enable host power (5V)
        usb_hardware.ucModeType = USB_HOST_MODE;                         // mark that we are in host mode
    #endif
    }
    else {                                                               // device mode
    #if defined USB_DEVICE_SUPPORT
        USB_CTRL = 0;                                                    // take transceiver out of suspend mode (with no pull-downs enabled)
        INT_ENB = (STALL_EN | USB_ERROR_EN | TOK_DNE_EN | USB_RST_EN);   // enable USB interrupts
        ERR_ENB = (CRC5_EOF_EN | CRC16_EN);                              // enable some error sources, for statistic use (CRC errors)
        CTL = USB_EN_SOF_EN;                                             // enable USB module device operation
        if ((pars->usConfig & USB_FULL_SPEED) != 0) {
            USB_USBTRC0 = USB_USBTRC0_RES;                               // this bit is not defined but should be set for correct operation
            USB_OTG_CONTROL = USB_OTG_CONTROL_DPPULLUPNONOTG;            // enable pull up on D+ for full speed operation           
        }
        else {
            //OTG_CTRL = (DM_HIGH | OTG_EN);                             // enable pull up on D- for low speed operation (this doesn't seem possible)
        }

        #if defined USE_USB_OTG_CHARGE_PUMP
        if (pars->OTG_I2C_Channel == 0) {                                // if no I2C interface opened, open it now
            I2CTABLE tI2CParameters;

            tI2CParameters.Channel = CHARGE_PUMP_I2C_CHANNEL;
            tI2CParameters.usSpeed = 100;                                // 100k
            tI2CParameters.Rx_tx_sizes.TxQueueSize = 10;
            tI2CParameters.Rx_tx_sizes.RxQueueSize = 0;
            tI2CParameters.Task_to_wake = 0;                             // no wake on transmission

            i2c_handle = fnOpen(TYPE_I2C, FOR_I_O, &tI2CParameters);     // open the channel with defined configurations
        }
        else {
            i2c_handle = pars->OTG_I2C_Channel;                          // share already available I2C interface
        }
        if (i2c_handle != 0) {
            #define CONTROL_REGISTER_1  0x10
            #define CONTROL_REGISTER_2  0x11
            #define  BDISC_ACONN        0x04
            #define  DP_PULLUP          0x10
            #define  DM_PULLUP          0x20
            const static unsigned char ucEnableDevice[] = {OTG_CHARGE_PUMP_I2C_ADD, CONTROL_REGISTER_2, 0x00}; // this assumes the MAX3353 
            unsigned char ucFS_Device[]    = {OTG_CHARGE_PUMP_I2C_ADD, CONTROL_REGISTER_1, (BDISC_ACONN | DP_PULLUP)};
            fnWrite(i2c_handle, (unsigned char *)ucEnableDevice, sizeof(ucEnableDevice)); // turn on device
            if ((pars->usConfig & USB_FULL_SPEED) == 0) {
                ucFS_Device[2] = (BDISC_ACONN | DM_PULLUP);              // low speed mode
            }
            fnWrite(i2c_handle, ucFS_Device, sizeof(ucFS_Device));       // configure for device mode
        }
        #endif
    #endif
    }
}

// Disconnect USB device (cause reset)
//
extern int fnUnplugUSBDevice(int iDevice)                                // {12}
{
    USB_OTG_CONTROL = 0;                                                 // disable pull up on D+ for full speed operation           
    return 0;
}

// Re-connect USB device (cause re-enumeration to start)
//
extern int fnPlugUSBDevice(int iDevice)                                  // {12}
{
    USB_OTG_CONTROL = USB_OTG_CONTROL_DPPULLUPNONOTG;                    // enable pull up on D+ for full speed operation           
    return 0;
}
