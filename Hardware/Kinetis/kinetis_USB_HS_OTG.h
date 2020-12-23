/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_USB_HS_OTG.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    28.03.2013 Add USB HS support                                        {33}
    04.06.2013 Added USB_FS_MALLOC(), USB_FS_MALLOC_ALIGN(), USB_HS_MALLOC(), USB_HS_MALLOC_ALIGN() defaults {42}
    05.03.2014 Add USB error counters and clear errors in the USB interrupt routine {71}
    28.05.2014 Move re-enable of USB SIE token processing to after input handling to avoid sporadic SETUP frame loss {83}
    18.07.2015 Add USB_SIMPLEX_ENDPOINTS suport to HS device             {1}
    05.10.2015 fnGetUSB_HW() modification for compatibility with host mode
    23.12.2015 Add zero copy OUT endpoint buffer option                  {2}
    07.02.2016 Set length to 8 when receiving setup frames (to avoid old lengths from OUTs on the endpoint from being kept) {3}
    08.05.2017 Correct internal HS USB 12MHz oscillator setting          {4}
    06.01.2020 Shared with iMX project and extended to support multiple controllers
    06.06.2020 Add capability to use FS and HS drivers at the same time (kinetis) and add HS isochronous support for high speed (up to 192MBit/s) operation
    27.06.2020 Added periodic queue operation of interrupt endpoints and allow multiple endpoints to operate in parallel

*/
#if defined USB_HS_INTERFACE
/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if defined NUMBER_OF_HOST_ENDPOINTS && (NUMBER_OF_HOST_ENDPOINTS > NUMBER_OF_USBHS_ENDPOINTS)
    #define ASYNCHRONOUS_SCHEDULE_LENGTH          (NUMBER_OF_HOST_ENDPOINTS * 2) // one asynchronous schedule for each required endpoint
#else
    #define ASYNCHRONOUS_SCHEDULE_LENGTH          (NUMBER_OF_USBHS_ENDPOINTS * 2) // one asynchronous schedule for each possible endpoint
#endif
#define AYNCHRONOUS_SCHEDULE_TRANSFER_OVERLAYS    2                      // at least 1 (presently only one is ever used)
#define HSUSB_INPUT_BUFFER_SIZE                   512

#if !defined USB_HS_MALLOC                                               // {42}
    #define USB_HS_MALLOC(x) uMalloc((MAX_MALLOC)(x))
#endif
#if !defined USB_HS_MALLOC_ALIGN                                         // {42}
    #define USB_HS_MALLOC_ALIGN(x, y) uMallocAlign((MAX_MALLOC)(x), (y))
#endif

#if defined USB_HOST_SUPPORT
    #if (HSUSB_CONTROLLERS > 1)
        #define __USB_HOST_MODE()         ((hs_usb_hardware[Channel].ucModeType & USB_HOST_MODE) != 0)
        #define __USB_DEVICE_MODE()       ((hs_usb_hardware[Channel].ucModeType & USB_HOST_MODE) == 0)
    #else
        #define __USB_HOST_MODE()         ((hs_usb_hardware->ucModeType & USB_HOST_MODE) != 0)
        #define __USB_DEVICE_MODE()       ((hs_usb_hardware->ucModeType & USB_HOST_MODE) == 0)
    #endif
#else
    #define __USB_HOST_MODE()         (0)
    #define __USB_DEVICE_MODE()       (1)
    #define USB_DEVICE_SUPPORT                                           // enable device only support if host support hasn't specifically been defined
#endif

#if !defined HSUSB_CONTROLLERS
    #define HSUSB_CONTROLLERS         1
#endif

#if !defined MAX_HSUSB_ENDPOINT_PERIOD
    #define MAX_HSUSB_ENDPOINT_PERIOD     32                             // default if no other maximum period is specified
#endif
#if MAX_HSUSB_ENDPOINT_PERIOD == 8
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_8
#elif MAX_HSUSB_ENDPOINT_PERIOD == 16
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_16
#elif MAX_HSUSB_ENDPOINT_PERIOD == 32
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_32
#elif MAX_HSUSB_ENDPOINT_PERIOD == 64
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_64
#elif MAX_HSUSB_ENDPOINT_PERIOD == 128
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_128
#elif MAX_HSUSB_ENDPOINT_PERIOD == 256
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_256
#elif MAX_HSUSB_ENDPOINT_PERIOD == 512
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_512
#elif MAX_HSUSB_ENDPOINT_PERIOD == 1024
    #define _FRAME_LIST_SIZE_VALUE        USBHS_USBCMD_FS_1024
#else
    #error "MAX_HSUSB_ENDPOINT_PERIOD must be 8,8,16,32,64,128,256,512 or 1024ms"
#endif
#define PERIODIC_FRAME_LIST_ELEMENTS (MAX_HSUSB_ENDPOINT_PERIOD)

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

#if !defined USB_FS_INTERFACE
typedef struct stUSB_ERRORS
{
    unsigned long ulUSB_errors_CRC5;
    unsigned long ulUSB_errors_CRC16;
} USB_ERRORS;
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static USB_HW hs_usb_hardware[HSUSB_CONTROLLERS] = {{0}};
static USB_END_POINT *hs_usb_endpoints[HSUSB_CONTROLLERS] = {0};
static USB_ERRORS hs_USB_errors[HSUSB_CONTROLLERS] = {{0}};
static KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrHS_EndpointQueueHeader[HSUSB_CONTROLLERS] = {0};
static unsigned char ucHS_EndpointCount[HSUSB_CONTROLLERS] = {0};
#if defined USB_HOST_SUPPORT
    static KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrPeriodicHeader[HSUSB_CONTROLLERS][NUMBER_OF_HOST_ENDPOINTS] = {0};
    static void (*timeout_handler0)(KINETIS_USBHS *ptrHSUSB, int iChannel) = 0;
    #if 0
    static void (*timeout_handler1)(KINETIS_USBHS *ptrHSUSB, int iChannel) = 0;
    #endif
#endif


/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static void fnUSBHS_init(QUEUE_HANDLE Channel, unsigned char ucEndpoints);
#if (HSUSB_CONTROLLERS > 1) && defined USE_BOTH_USB
    static __interrupt void _usb_hs_otg_isr1(void);
    static __interrupt void _usb_hs_otg_isr2(void);
    static void _usb_hs_otg_isr_common(KINETIS_USBHS *ptrHSUSB, int iChannel);
#else
    static __interrupt void _usb_hs_otg_isr(void);
#endif
#if defined USB_HOST_SUPPORT
    static void fnInitialiseScheduleList(int Channel);
#endif


/* =================================================================== */
/*                                USB                                  */
/* =================================================================== */


#if defined USB_DEVICE_SUPPORT
// When the clear feature is received for a stalled endpoint it is cleared in the hardware by calling this routine
//
    #if defined USB_FS_INTERFACE
extern void fnUnhaltHS_Endpoint(unsigned char ucEndpoint)
    #else
extern void fnUnhaltEndpoint(unsigned char ucEndpoint)
    #endif
{
    volatile unsigned long *ptrControl = USBHS0_EPCR0_ADDR;
    ptrControl += (ucEndpoint & ~IN_ENDPOINT);
    if ((ucEndpoint & IN_ENDPOINT) != 0) {                               // IN endpoint
        *ptrControl |= USBHS_EPCR_TXR;                                   // resynchronise the data toggle
        *ptrControl &= ~(USBHS_EPCR_TXS);                                // remove stall on IN
    }
    else {                                                               // not IN endpoint
        *ptrControl |= USBHS_EPCR_RXR;                                   // resynchronise the data toggle        
        *ptrControl &= ~(USBHS_EPCR_RXS);                                // remove stall on OUT
    }
}

    #if defined USB_HS_ISO_SUPPORT
static unsigned long fnOptimiseMultiISO(unsigned long ulCapabilities, USB_HS_TRANSFER_OVERLAY *ptr_dTD, unsigned short usLen, unsigned long ulMaximumPacketLength)
{
    unsigned long ulRemainingSize = usLen;
    if (ulRemainingSize <= ulMaximumPacketLength) {
        ptr_dTD->ul_dtToken |= ENDPOINT_QUEUE_HEADER_TOKEN_MULTO_1;      // return single data frame in the isochronous micro-frame
        ptr_dTD->ulNextdTD_pointer |= ENDPOINT_QUEUE_HEADER_NEXT_INVALID;// terminate the end of the queue
        return 0;
    }
    ulRemainingSize -= ulMaximumPacketLength;
    if (ulCapabilities == ENDPOINT_QUEUE_HEADER_MULT_1) {
        ptr_dTD->ul_dtToken |= ENDPOINT_QUEUE_HEADER_TOKEN_MULTO_1;      // return single data frame in the isochronous micro-frame
    }
    else {
        if (ulRemainingSize <= ulMaximumPacketLength) {
            ptr_dTD->ul_dtToken |= ENDPOINT_QUEUE_HEADER_TOKEN_MULTO_2;  // return two data frames in the isochronous micro-frame
            ptr_dTD->ulNextdTD_pointer |= ENDPOINT_QUEUE_HEADER_NEXT_INVALID; // terminate the end of the queue
            return 0;
        }
        ulRemainingSize -= ulMaximumPacketLength;
        if (ulCapabilities == ENDPOINT_QUEUE_HEADER_MULT_2) {
            ptr_dTD->ul_dtToken |= ENDPOINT_QUEUE_HEADER_TOKEN_MULTO_2;  // return two data frames in the isochronous micro-frame
        }
        else {
            ptr_dTD->ul_dtToken |= ENDPOINT_QUEUE_HEADER_TOKEN_MULTO_3;  // return three data frames in the isochronous micro-frame
            if (ulRemainingSize <= ulMaximumPacketLength) {
                ptr_dTD->ulNextdTD_pointer |= ENDPOINT_QUEUE_HEADER_NEXT_INVALID; // terminate the end of the queue
                return 0;
            }
            ulRemainingSize -= ulMaximumPacketLength;
        }
    }
    return ulRemainingSize;
}

static USB_HS_TRANSFER_OVERLAY *fnExtendeQueue(USB_HS_TRANSFER_OVERLAY *ptr_dTD, unsigned long ulCapabilities, unsigned char *pData, unsigned short usLength, unsigned long *ptr_ulRemainingLength, unsigned long ulMaximumPacketLength)
{
    unsigned long ulOriginalRemainingLength = *ptr_ulRemainingLength;
    unsigned short usNewLength = (unsigned short)(usLength - ulOriginalRemainingLength);
    USB_HS_TRANSFER_OVERLAY *ptrNext;
    unsigned short usThisLength;
    if ((ptr_dTD->ulNextdTD_pointer & ~(ENDPOINT_QUEUE_HEADER_NEXT_INVALID)) == 0) { // if a new descriptor is required
        fnDebugMsg("M = ");
        fnDebugHex(ptr_dTD->ulNextdTD_pointer, (WITH_LEADIN | sizeof(ptr_dTD->ulNextdTD_pointer) | WITH_CR_LF));
        ptr_dTD->ulNextdTD_pointer = (unsigned long)uMallocAlign(sizeof(USB_HS_TRANSFER_OVERLAY), 32);
        ptrNext = (USB_HS_TRANSFER_OVERLAY *)ptr_dTD->ulNextdTD_pointer;
        ptrNext->ulNextdTD_pointer |= ENDPOINT_QUEUE_HEADER_NEXT_INVALID;// end of the list
    }
    else {
        ptr_dTD->ulNextdTD_pointer &= ~(ENDPOINT_QUEUE_HEADER_NEXT_INVALID);
        ptrNext = (USB_HS_TRANSFER_OVERLAY *)ptr_dTD->ulNextdTD_pointer;
    }
    pData += usNewLength;
    ptrNext->ulBufferPointerPage[0] = (unsigned long)pData;              // enter the buffer pointers on incremented 4k page boundaries
    ptrNext->ulBufferPointerPage[1] = ((unsigned long)(pData + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
    //ptrNext->ulBufferPointerPage[2] = (ptrNext->ulBufferPointerPage[1] + (4 * 1024)); // further page buffers could be used for handling large transmissions of more than 4k
    //ptrNext->ulBufferPointerPage[3] = (ptrNext->ulBufferPointerPage[2] + (4 * 1024));
    //ptrNext->ulBufferPointerPage[4] = (ptrNext->ulBufferPointerPage[3] + (4 * 1024));

    ptrNext->ul_dtToken = (ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE);
    *ptr_ulRemainingLength = fnOptimiseMultiISO(ulCapabilities, ptrNext, (unsigned short)ulOriginalRemainingLength, ulMaximumPacketLength);
    usThisLength = (unsigned short)(ulOriginalRemainingLength - *ptr_ulRemainingLength);
    ptrNext->ul_dtToken |= (usThisLength << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT);
    return ptrNext;
}

        #if 0                                                            // debugging routine to display how a queue has been configured
static void fnQueueShowSetup(KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHead)
{
    USB_HS_TRANSFER_OVERLAY *ptr_pTD;
    USB_HS_TRANSFER_OVERLAY *ptr_start_pTD;
    int i = 0;
    unsigned long *ptrEntry = (unsigned long *)ptrQueueHead;
    fnDebugMsg("Queue header:\r\n");
    while (i < (sizeof(KINETIS_USBHS_ENDPOINT_QUEUE_HEADER) / 4)) {
        fnDebugHex(*ptrEntry++, (sizeof(unsigned long) | WITH_LEADIN | WITH_SPACE));
        if ((++i % 4) == 0) {
            fnDebugMsg("\r\n");
        }
    }
    ptr_pTD = ptr_start_pTD = ptrQueueHead->my_dTD;
    ptrEntry = (unsigned long *)ptr_pTD;
    i = 0;
    fnDebugMsg("dTD:\r\n");
    while (i < (sizeof(USB_HS_TRANSFER_OVERLAY) / 4)) {
        fnDebugHex(*ptrEntry++, (sizeof(unsigned long) | WITH_LEADIN | WITH_SPACE));
        if ((++i % 4) == 0) {
            fnDebugMsg("\r\n");
        }
    }
    FOREVER_LOOP() {
        ptr_pTD = (USB_HS_TRANSFER_OVERLAY *)ptr_pTD->ulNextdTD_pointer;
        ptrEntry = (unsigned long *)ptr_pTD;
        if (((*ptrEntry & 1) != 0) || (ptr_start_pTD == ptr_pTD)) {
            fnDebugMsg("End\r\n");
            return;
        }
        i = 0;
        fnDebugMsg("dTD:\r\n");
        while (i < (sizeof(USB_HS_TRANSFER_OVERLAY) / 4)) {
            fnDebugHex(*ptrEntry++, (sizeof(unsigned long) | WITH_LEADIN | WITH_SPACE));
            if ((++i % 4) == 0) {
                fnDebugMsg("\r\n");
            }
        }
    }
}
        #endif
    #endif
#endif

// This routine checks through all endpoints and stops any that are presently active - all endpoints are disabled
// - it also needs to clean up queues
//
static void fnDeInitEndpoints(KINETIS_USBHS *ptrHSUSB, int iChannel)
{
    KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR;
    int i;
    volatile unsigned long *ptrEP_Ctrl = &ptrHSUSB->EPCR0;
    unsigned long ulRxEp = USBHS_EPFLUSH_FERB0;
    unsigned long ulTxEp = USBHS_EPFLUSH_FETB0;
    unsigned long ulFlushEndpoints = 0;
    if (ptrQueueHeader == 0) {
        return;                                                          // queue header not yet allocated so ignore
    }
    _WAIT_REGISTER_TRUE(ptrHSUSB->EPPRIME, 0xffffffff);                  // cancel all primed status by waiting until all bits in the EPPRIME are 0 and then writing to EPFLUSH
    for (i = 0; i < ucHS_EndpointCount[iChannel]; i++) {
        *ptrEP_Ctrl &= ~(USBHS_EPCR_RXE | USBHS_EPCR_RXT_INT | USBHS_EPCR_TXE | USBHS_EPCR_TXT_INT); // disable endpoint rx and tx operation and reset endpoints types
        if ((ptrQueueHeader->dTD.ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE) != 0) { // if the reception endpoint is presently active (as reset ocurred)
            ulFlushEndpoints |= ulRxEp;                                  // collect the enpoints to be flushed
        }
        ptrQueueHeader++;                                                // move to transmit queue header
        if ((ptrQueueHeader->dTD.ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE) != 0) { // if the transmission endpoint is presently active (as reset ocurred)
            ulFlushEndpoints |= ulTxEp;                                  // collect the enpoints to be flushed
        }
        ptrQueueHeader++;
        ulRxEp <<= 1;
        ulTxEp <<= 1;
        ptrEP_Ctrl++;
    }

    do {
        ptrHSUSB->EPFLUSH = ulFlushEndpoints;                            // command a flush of the active endpoints
        _WAIT_REGISTER_TRUE(ptrHSUSB->EPFLUSH, 0xffffffff);              // this operation may take a large amount of time depending on the USB bus activity and it is not desirable to have it in an interrupt routine
    } while ((ulFlushEndpoints = ptrHSUSB->EPSR) != 0);                  // check whether the flush failed - this can happen when a packet to a particular endpoint was in progress
}


// It has been found that the USBHS controller can send data from SRAM or FLASH source addresses therefore we don't copy the data into an intermediate buffer
// but instead send it from where it is (assuming that it remains stable)
// - two buffer pointers are prepared, whereby the second is set to a 4k page boundary so that at least 4k of linear input data can be sent even when its buffer crosses a 4k boundary
//
extern void fnTxUSBHS(unsigned char *pData, unsigned short usLen, int iEndpoint, USB_HW *ptrUSB_HW)
{
    KINETIS_USBHS *ptrHSUSB = ptrUSB_HW->ptrHSUSB;
#if defined USB_HOST_SUPPORT || defined _WINDOWS || defined USB_HS_ISO_SUPPORT
    #if (HSUSB_CONTROLLERS > 1)
    int iChannel = ptrUSB_HW->ucController;
    #else
    int iChannel = 0;
    #endif
#endif
#if defined USB_HOST_SUPPORT
    if ((ptrUSB_HW->ucModeType & USB_HOST_MODE) != 0) {
        KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrTxQueueHeader;
        ptrUSB_HW = &hs_usb_hardware[iChannel];                          // ensure that we use the global hardware details
        int _iEndpoint;
        if (iEndpoint & 0x80000000) {
            iEndpoint &= ~0x80000000;
            _iEndpoint = ((iEndpoint * 2) + 1);
        }
        else {
            _iEndpoint = (iEndpoint * 2);
        }
        ptrTxQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrUSB_HW->ptrQueueHeader + _iEndpoint); // set to the endpoint queue
        if ((ptrTxQueueHeader->ulFirmwareFlags & ASYNC_ENDPOINT_IGNORE) == 0) { // endpoint is in operation already
            if ((ptrTxQueueHeader->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) == HOST_OVERLAY_CONTROL_PID_CODE_IN) { // IN active
                // We must stop the present operation and ensure that any IN reception is first handled before continuing - the IN operation will be restarted by the application after transmission has completed
                //
                ptrHSUSB->USBCMD &= ~(USBHS_USBCMD_ASE);                 // temporarily disable the asynchronous scheduling
                if (iInterruptLevel != 0) {
                    int iOriginalInterruptLevel = iInterruptLevel;
//TOGGLE_TEST_OUTPUT();
                    do {
                        uEnable_Interrupt();
                    } while (iInterruptLevel != 0);
                    _WAIT_REGISTER_TRUE(ptrHSUSB->USBSTS, USBHS_USBSTS_AS);  // wait for it to terminate - interrupts can be handled during the wait
                    ptrTxQueueHeader->my_HostdTD->ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE; // stop the entry activity
                    ptrTxQueueHeader->host_transfer_overlay.ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE;
                    do {
                        uDisable_Interrupt();
                    } while (iInterruptLevel != iOriginalInterruptLevel);
//TOGGLE_TEST_OUTPUT();
                }
                else {
                    _WAIT_REGISTER_TRUE(ptrHSUSB->USBSTS, USBHS_USBSTS_AS);  // wait for it to terminate - interrupts can be handled during the wait
                    ptrTxQueueHeader->my_HostdTD->ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE; // stop the entry activity
                    ptrTxQueueHeader->host_transfer_overlay.ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE;
                }
                ptrTxQueueHeader->ulFirmwareFlags = ASYNC_ENDPOINT_IGNORE; // this endpoint can be ignored now
                ptrHSUSB->USBCMD |= (USBHS_USBCMD_ASE);                  // restart the scheduling (without IN endpoint activity)
            }
        }

        while (hs_usb_hardware[iChannel].ucUSB_HostEvent[_iEndpoint] != USB_NO_FRAME) { // if we are waiting for an event on the endpoint we don't continue until it has completed (USB interrupt will handle it)
            if (iInterruptLevel != 0) {
                _EXCEPTION("Interrupts must be enabled !!!!");
            }
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
            _SIM_USB(hs_usb_hardware[iChannel].ucController, USB_SIM_WAIT, iEndpoint, ptrUSB_HW); // allow USB interrupt simulation
    #else
            _SIM_USB(iChannel, USB_SIM_WAIT, iEndpoint, ptrUSB_HW);      // allow USB interrupt simulation
    #endif
        }

        ptrTxQueueHeader->my_HostdTD->ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
        ptrTxQueueHeader->my_HostdTD->ptrNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;

        ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[0] = (unsigned long)pData; // enter the buffer pointers on incremented 4k page boundaries
        ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[1] = ((unsigned long)(pData + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
        //ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[2] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[1] + (4 * 1024)); // further page buffers could be used for handling large transmissions of more than 4k
        //ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[3] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[2] + (4 * 1024));
        //ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[4] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[3] + (4 * 1024));

        ptrTxQueueHeader->my_HostdTD->ulControl = (usLen << HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT); // prepare the length of the data to be sent
        return;
    }
    else {
#endif
#if defined USB_DEVICE_SUPPORT
        KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrTxQueueHeader = (ptrUSB_HW->ptrQueueHeader + 1); // transmission queue head
    #if defined USB_HS_ISO_SUPPORT
        if ((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3) != 0) { // if high speed isochronous endpoint
            ptrUSB_HW = &hs_usb_hardware[iChannel];                          // ensure that we use the global hardware details
            if (usLen != 0) {
                ptrUSB_HW->iso_state = 0;
                ptrUSB_HW->iso_length = usLen;
                usLen = (ptrUSB_HW->iso_length/2);
            }
            else if (ptrUSB_HW->iso_state == 0) {
                unsigned long ulMaximumPacketLength = ((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_MASK) >> ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT);
                unsigned long ulRemainingLength;
                USB_HS_TRANSFER_OVERLAY *ptr_dTD = ptrTxQueueHeader->my_dTD;
                usLen = (ptrUSB_HW->iso_length / 2);
                pData += usLen;
                ptr_dTD->ulBufferPointerPage[0] = (unsigned long)pData; // enter the buffer pointers on incremented 4k page boundaries
                ptr_dTD->ulBufferPointerPage[1] = ((unsigned long)(pData + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
                ptr_dTD->ul_dtToken = 0;
                ulRemainingLength = fnOptimiseMultiISO((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), ptr_dTD, usLen, ulMaximumPacketLength); // set the multi packet override for optimal transmission
                ptr_dTD->ul_dtToken |= (((usLen - ulRemainingLength) << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT) | ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE);
                while (ulRemainingLength != 0) {
                    ptr_dTD = fnExtendeQueue(ptr_dTD, (ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), pData, usLen, &ulRemainingLength, ulMaximumPacketLength);
                }
                ptr_dTD->ulNextdTD_pointer &= ~(ENDPOINT_QUEUE_HEADER_NEXT_INVALID);
                ptrUSB_HW->iso_mid_buffer = (USB_HS_TRANSFER_OVERLAY *)ptr_dTD->ulNextdTD_pointer;
                ptr_dTD->ul_dtToken |= (ENDPOINT_QUEUE_HEADER_TOKEN_IOC);// enable interrupt on last item in the queue
                ptrUSB_HW->iso_state = 1;

                ptrHSUSB->EPPRIME |= ((USBHS_EPPRIME_PETB0) << iEndpoint); // prime the transmission
        #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
                _SIM_USB(ptrUSB_HW->ucController, USB_SIM_TX, iEndpoint, ptrUSB_HW);
        #else
                _SIM_USB(iChannel, USB_SIM_TX, iEndpoint, ptrUSB_HW);
        #endif
                return;
            }
            else {
                unsigned long ulMaximumPacketLength = ((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_MASK) >> ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT);
                unsigned long ulRemainingLength;
                USB_HS_TRANSFER_OVERLAY *ptr_dTD = ptrUSB_HW->iso_mid_buffer;
                ptr_dTD->ulBufferPointerPage[0] = (unsigned long)pData; // enter the buffer pointers on incremented 4k page boundaries
                ptr_dTD->ulBufferPointerPage[1] = ((unsigned long)(pData + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
                usLen = (ptrUSB_HW->iso_length / 2);
                ptr_dTD->ul_dtToken = 0;
                ulRemainingLength = fnOptimiseMultiISO((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), ptr_dTD, usLen, ulMaximumPacketLength); // set the multi packet override for optimal transmission
                ptr_dTD->ul_dtToken |= (((usLen - ulRemainingLength) << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT) | ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE);
                while (ulRemainingLength != 0) {
                    ptr_dTD = fnExtendeQueue(ptr_dTD, (ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), pData, usLen, &ulRemainingLength, ulMaximumPacketLength);
                }
                ptr_dTD->ulNextdTD_pointer &= ~(ENDPOINT_QUEUE_HEADER_NEXT_INVALID);
                ptr_dTD->ul_dtToken |= (ENDPOINT_QUEUE_HEADER_TOKEN_IOC);// enable interrupt on last item in the queue
                ptrUSB_HW->iso_state = 0;

                ptrHSUSB->EPPRIME |= ((USBHS_EPPRIME_PETB0) << iEndpoint); // prime the transmission
        #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
                _SIM_USB(ptrUSB_HW->ucController, USB_SIM_TX, iEndpoint, ptrUSB_HW);
        #else
                _SIM_USB(iChannel, USB_SIM_TX, iEndpoint, ptrUSB_HW);
        #endif
                return;
            }
        }
    #endif

        ptrTxQueueHeader->dTD.ulBufferPointerPage[0] = (unsigned long)pData; // enter the buffer pointers on incremented 4k page boundaries
        ptrTxQueueHeader->dTD.ulBufferPointerPage[1] = ((unsigned long)(pData + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
        //ptrTxQueueHeader->dTD.ulBufferPointerPage[2] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[1] + (4 * 1024)); // further page buffers could be used for handling large transmissions of more than 4k
        //ptrTxQueueHeader->dTD.ulBufferPointerPage[3] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[2] + (4 * 1024));
        //ptrTxQueueHeader->dTD.ulBufferPointerPage[4] = (ptrTxQueueHeader->dTD.ulBufferPointerPage[3] + (4 * 1024));

        // We don't use a linked list so the first entry is always set
        //
        ptrTxQueueHeader->dTD.ul_dtToken = ((usLen << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT) | ENDPOINT_QUEUE_HEADER_TOKEN_IOC | ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE); // the length of data to be sent with interrupt on completion
    #if defined USB_HS_ISO_SUPPORT
        if ((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3) != 0) { // if high speed isochronous endpoint
            unsigned long ulMaximumPacketLength = ((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_MASK) >> ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT);
            unsigned long ulRemainingLength = fnOptimiseMultiISO((ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), &ptrTxQueueHeader->dTD, usLen, ulMaximumPacketLength); // set the multi packet override for optimal transmission
            if (ulRemainingLength != 0) {                                // if more data is to be sent than can be transmitted in a single isochronous transaction a queue is created with an interrupt on the final termination
                USB_HS_TRANSFER_OVERLAY *ptr_dTD = ptrTxQueueHeader->my_dTD;
                ptrTxQueueHeader->dTD.ul_dtToken &= ~(ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_MASK | ENDPOINT_QUEUE_HEADER_TOKEN_IOC); // remove the interrupt on the first transaction
                ptrTxQueueHeader->dTD.ul_dtToken |= ((usLen - ulRemainingLength) << ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT); // adapt the first transaction's length
                while (ulRemainingLength != 0) {
                    ptr_dTD = fnExtendeQueue(ptr_dTD, (ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), pData, usLen, &ulRemainingLength, ulMaximumPacketLength);
                }
                ptr_dTD->ul_dtToken |= (ENDPOINT_QUEUE_HEADER_TOKEN_IOC);// enable interrupt on last item in the queue
                ulRemainingLength = usLen;                               // create second half of linked list with interrupt on final buffer descriptor
                while (ulRemainingLength != 0) {
                    ptr_dTD = fnExtendeQueue(ptr_dTD, (ptrTxQueueHeader->ulCapabilities & ENDPOINT_QUEUE_HEADER_MULT_3), (pData + usLen), usLen, &ulRemainingLength, ulMaximumPacketLength);
                }
                ptr_dTD->ul_dtToken |= (ENDPOINT_QUEUE_HEADER_TOKEN_IOC);// enable interrupt on last item in the queue
                ptr_dTD->ulNextdTD_pointer = (unsigned long)ptrTxQueueHeader->my_dTD; // finally make into circular buffer of descriptors
            }
        }
    #endif
        ptrTxQueueHeader->CurrentdTD_pointer = ptrTxQueueHeader->my_dTD;
        /*
        if (iEndpoint == 2) {
            static int iCnt = 0;
            if (iCnt++ < 2) {
                fnDebugMsg("Debug\r\n");
                fnDebugHex((unsigned long)ptrTxQueueHeader->ulCapabilities, (WITH_LEADIN | 4 | WITH_CR_LF));
                fnDebugHex((unsigned long)ptrTxQueueHeader->my_dTD->ul_dtToken, (WITH_LEADIN | 4 | WITH_CR_LF));
            }
        }*/
        ptrHSUSB->EPPRIME |= ((USBHS_EPPRIME_PETB0) << iEndpoint);       // prime the transmission
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
        _SIM_USB(ptrUSB_HW->ucController, USB_SIM_TX, iEndpoint, ptrUSB_HW);
    #else
        _SIM_USB(iChannel, USB_SIM_TX, iEndpoint, ptrUSB_HW);
    #endif
#endif
#if defined USB_HOST_SUPPORT
    }
#endif
}

#if defined USB_DEVICE_SUPPORT
// This routine prepares an endpoint's reception buffer to receive data to
//
static void fnPrimeReception(int iChannel, unsigned short usLastLength, KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader, int iEndpointNumber)
{
    KINETIS_USBHS *ptrHSUSB;
    #if (HSUSB_CONTROLLERS > 1)
    if (iChannel != 0) {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE2_ADD;
    }
    else {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE1_ADD;
    }
    #else
    ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE_ADD;
    #endif
    if (usLastLength != 0) {                                             // if we haven't just received zero data
        ptrQueueHeader->dTD.ulBufferPointerPage[0] = (unsigned long)ptrQueueHeader->my_dTD->ptrBufferStart; // reset the first page buffer pointer
        ptrQueueHeader->dTD.ulBufferPointerPage[1] = ((unsigned long)(ptrQueueHeader->my_dTD->ptrBufferStart + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
    }
    ptrQueueHeader->dTD.ul_dtToken = (ptrQueueHeader->ulBufferLength | ENDPOINT_QUEUE_HEADER_TOKEN_IOC | ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE);
    ptrHSUSB->EPPRIME |= ((USBHS_EPPRIME_PERB0) << iEndpointNumber);     // prime the reception again on the endpoint's reception
    #if defined _WINDOWS
    uMemcpy((void *)ptrQueueHeader->CurrentdTD_pointer, &ptrQueueHeader->dTD, (sizeof(ptrQueueHeader->dTD) - sizeof(unsigned long))); // the USBHS controller automatically copied the dTD content to the transfer buffer
    #endif
}
#endif

// This routine handles all SETUP and OUT frames. It can send an empty data frame if required by control endpoints or stall on errors.
// It usually clears the handled input buffer when complete, unless the buffer is specified to remain owned by the processor.
//
static int fnProcessHSInput(int iEndpoint_ref, USB_HW *hs_usb_hardware, unsigned char ucFrameType, unsigned char *ptrData)
{
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when putting messages to queue
    switch (fnUSB_handle_frame(ucFrameType, ptrData, iEndpoint_ref, hs_usb_hardware)) { // generic handler routine
#if defined USB_HOST_SUPPORT
    case TERMINATE_ZERO_DATA_NON_E:
#endif
    case TERMINATE_ZERO_DATA:                                            // send zero data packet to complete status stage of control transfer
        fnTxUSBHS(0, 0, iEndpoint_ref, hs_usb_hardware);
#if defined USB_HOST_SUPPORT
        if ((hs_usb_hardware->ucModeType & USB_HOST_MODE) != 0) {        // if in host mode we need to release the prepared zero frame
            fnHostReleaseBuffer(iEndpoint_ref, OUT_PID, hs_usb_hardware);// allow host to release prepared zero data
            uEnable_Interrupt();
            return 0;
        }
#endif
        break;
    case MAINTAIN_OWNERSHIP:                                             // don't free the buffer - the application will do this later
        uEnable_Interrupt();
        return MAINTAIN_OWNERSHIP;
#if defined USB_DEVICE_SUPPORT
    case STALL_ENDPOINT:                                                 // send stall
        {
    #if (HSUSB_CONTROLLERS > 1)
            register int iChannel = hs_usb_hardware->ucController;
    #else
            register int iChannel = 0;
    #endif
            if (iEndpoint_ref == 0) {                                    // check whether control 0 endpoint
                hs_usb_hardware->ptrHSUSB->EPCR0 |= (USBHS_EPCR_TXS);    // the stall is cleared when a following SETUP packet is received
                fnSetUSBEndpointState(0, USB_ENDPOINT_STALLED, iChannel);
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
                _SIM_USB(hs_usb_hardware[iChannel].ucController, USB_SIM_STALL, 0, hs_usb_hardware); // allow USB interrupt simulation
    #else
                _SIM_USB(iChannel, USB_SIM_STALL, 0, hs_usb_hardware);
    #endif
            }
            else {                                                       // assume IN endpoint is always one higher than OUT endpoint
                int iIN_ref = fnGetPairedIN(iEndpoint_ref, iChannel);    // get the paired IN endpoint reference
                *(&hs_usb_hardware->ptrHSUSB->EPCR0 + iIN_ref) |= (USBHS_EPCR_TXS); // stall
                fnSetUSBEndpointState(iIN_ref, USB_ENDPOINT_STALLED, iChannel); // mark the stall at the IN endpoint
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
                _SIM_USB(hs_usb_hardware[iChannel].ucController, USB_SIM_STALL, iIN_ref, hs_usb_hardware); // allow USB interrupt simulation
    #else
                _SIM_USB(iChannel, USB_SIM_STALL, iIN_ref, hs_usb_hardware);
    #endif
            }
            break;
        }
#endif
#if defined USB_HOST_SUPPORT
    case INITIATE_IN_TOKEN:
        fnHostReleaseBuffer(iEndpoint_ref, IN_PID, hs_usb_hardware);     // start transmission of IN token on the appropriate endpoint
        uEnable_Interrupt();
        return 0;
#endif
    }
#if defined USB_DEVICE_SUPPORT
    #if defined USB_HOST_SUPPORT
    if ((hs_usb_hardware->ucModeType & USB_HOST_MODE) != 0) {            // if in host mode we need to release the prepared zero frame
        uEnable_Interrupt();
        return 0;
    }
    #endif
    if (ucFrameType != USB_SETUP_FRAME) {                                // setup frames don't require reception to be re-enabled
    #if (HSUSB_CONTROLLERS > 1)
        register int iChannel = hs_usb_hardware->ucController;
    #else
        register int iChannel = 0;
    #endif
        fnPrimeReception(iChannel, hs_usb_hardware->usLength, hs_usb_hardware->ptrQueueHeader, iEndpoint_ref); // prepare for next reception
    }
#endif
    uEnable_Interrupt();
    return 0;
}

#if defined USB_HOST_SUPPORT

#if defined _iMX || defined KINETIS_WITH_USBPHY
static void fnPHY_detach(int iEnable, int iChannel)
{
    USBPHY *phy;
    #if HSUSB_CONTROLLERS > 1
    if (iChannel != 0) {
        phy = (USBPHY *)USBHS_PHY2_ADD;
    }
    else {
        phy = (USBPHY *)USBHS_PHY1_ADD;
    }
    #else
    phy = (USBPHY *)USBHS_PHY_ADD;
    #endif
    if (iEnable != 0) {
        _SET_REGISTER(phy->USBPHY_CTRL, USBPHY_CTRL_ENHOSTDISCONDETECT);
    }
    else {
        _CLEAR_REGISTER(phy->USBPHY_CTRL, USBPHY_CTRL_ENHOSTDISCONDETECT);
    }
}
#endif

// A single-shot HW timer reserved for USB host timing use
//
static void fnUSB_HS_HostDelaySingle(KINETIS_USBHS *ptrHSUSB, void (*timeout_handler)(KINETIS_USBHS *ptrHSUSB, int iChannel), unsigned long ulDelay)
{
    timeout_handler0 = timeout_handler;
    if (ulDelay == 0) {
        ptrHSUSB->GPTIMER0CTL = 0;                                       // stop operation
        return;
    }
    ptrHSUSB->GPTIMER0LD = (ulDelay - 1);                                // set timer delay
    ptrHSUSB->GPTIMER0CTL |= (USBHS_GPTIMERCTL_MODE_SINGLE | USBHS_GPTIMERCTL_RST | USBHS_GPTIMERCTL_RUN); // start operation
    #if defined _WINDOWS
    ptrHSUSB->GPTIMER0CTL &= ~(USBHS_GPTIMERCTL_RST | USBHS_GPTIMERCTL_GPTCNT_MASK);
    ptrHSUSB->GPTIMER0CTL |= (ptrHSUSB->GPTIMER0LD & USBHS_GPTIMERCTL_GPTCNT_MASK); // initial count value (in us) loaded and counts down until zero
    #endif
}

// Delay after the host commands a reset to the device
//
static void device_reset(KINETIS_USBHS *ptrHSUSB, int iChannel)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = ((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHS_EndpointQueueHeader[iChannel]));
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostHead->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
    _WAIT_REGISTER_TRUE(ptrHSUSB->PORTSC1, USBHS_PORTSC1_PR);            // this will generally not result in a delay because the reset pulse will have completed but it has been found that a read is required before the flag self-resets
    ptrHostHead->ulHostCharacteristics &= ~(HOST_CHARACTERISTICS_EP_SPEED_MASK | HOST_CHARACTERISTICS_C); // prepare the speed field
    switch (ptrHSUSB->PORTSC1 & USBHS_PORTSC1_PSPD_MASK) {               // check the speed of the attached device
    case USBHS_PORTSC1_PSPD_HS:                                          // high speed device attached
    default:
    #if defined _iMX || defined KINETIS_WITH_USBPHY
        fnPHY_detach(1, iChannel);                                       // enable HS host disconnection detection
    #endif
        hs_usb_hardware[iChannel].ucDeviceSpeed = USB_DEVICE_SPEED_HIGH; // attached device is a high-speed device
        ptrHostHead->ulHostCharacteristics |= (HOST_CHARACTERISTICS_EP_SPEED_HS); // insert the device speed
        break;
    case USBHS_PORTSC1_PSPD_FS:
        hs_usb_hardware[iChannel].ucDeviceSpeed = USB_DEVICE_SPEED_FULL; // attached device is a full-speed device
        ptrHostHead->ulHostCharacteristics |= (HOST_CHARACTERISTICS_C | HOST_CHARACTERISTICS_EP_SPEED_FS); // insert the device speed
        break;
    case USBHS_PORTSC1_PSPD_LS:
        hs_usb_hardware[iChannel].ucDeviceSpeed = USB_DEVICE_SPEED_LOW;  // attached device is a low speed device
        ptrHostHead->ulHostCharacteristics |= (HOST_CHARACTERISTICS_C | HOST_CHARACTERISTICS_EP_SPEED_LS); // insert the device speed
        break;
    }
    ptrHostQueueHeader->ulHostCharacteristics = ((ptrHostQueueHeader->ulHostCharacteristics & ~(HOST_CHARACTERISTICS_EP_SPEED_MASK | HOST_CHARACTERISTICS_C)) | (ptrHostHead->ulHostCharacteristics & (HOST_CHARACTERISTICS_EP_SPEED_MASK | HOST_CHARACTERISTICS_C)));
    ptrHSUSB->USBCMD |= USBHS_USBCMD_ASE;                                // enable asynchronous scheduling so that the host sends control and bulk data
    fnUSB_handle_frame(USB_DEVICE_DETECTED, 0, 0, &hs_usb_hardware[iChannel]); // generic handler routine which will always request a setup frame to be sent
}

static void fnUSB_HS_ResetCycle(KINETIS_USBHS *ptrHSUSB, int iChannel)
{
    if ((ptrHSUSB->PORTSC1 & USBHS_PORTSC1_CCS) != 0) {                  // if still indicating device attached
        ptrHSUSB->PORTSC1 |= (USBHS_PORTSC1_PR);                         // command a reset (this is self-timing and clears the reset after 10ms)
        fnUSB_HS_HostDelaySingle(ptrHSUSB, device_reset, (20000));       // delay slightly longer than the reset output
    }
}

extern void fnUSB_DeviceResetCycle(int iChannel)
{
    KINETIS_USBHS *ptrHSUSB = hs_usb_hardware[iChannel].ptrHSUSB;
    fnUSB_HS_ResetCycle(ptrHSUSB, iChannel);
}


// After a short delay after a possible attach, check the state and decide whether we need to continue with the attached device
//
static void usb_possible_attach(KINETIS_USBHS *ptrHSUSB, int iChannel)
{
    fnUSB_HS_ResetCycle(ptrHSUSB, iChannel);
}

    #if 0
// A "repetitive" HW timer reserved for USB host timing use
//
static void fnUSB_HS_HostDelayRepeat(void(*timeout_handler)(KINETIS_USBHS *ptrHSUSB, int iChannel), unsigned long ulDelay)
{
    timeout_handler1 = timeout_handler;
    ptrHSUSB->GPTIMER1LD = (ulDelay - 1);                                // set timer 1 delay
    ptrHSUSB->GPTIMER1CTL |= (USBHS_GPTIMERCTL_MODE_SINGLE | USBHS_GPTIMERCTL_RST | USBHS_GPTIMERCTL_RUN); // start operation
    #if defined _WINDOWS
    ptrHSUSB->GPTIMER1CTL &= ~(USBHS_GPTIMERCTL_RST | USBHS_GPTIMERCTL_GPTCNT_MASK);
    ptrHSUSB->GPTIMER1CTL |= (ptrHSUSB->GPTIMER0LD & USBHS_GPTIMERCTL_GPTCNT_MASK); // initial count value (in us) loaded and counts down until zero
    #endif
}
    #endif

// Set the device address and operating speed to all asynchronous schedules
//
extern void fnSetHSUSB_device_add(unsigned char ucDeviceAddress, USB_HW *ptrUSB_HW)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead;
    unsigned long ulAddressSpeedFields;
    int iList = 0;
    ptrUSB_HW->ucUSBAddress = ucDeviceAddress;
    ptrHostHead = ((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrUSB_HW->ptrQueueHeader));
    ulAddressSpeedFields = (ptrHostHead->ulHostCharacteristics & HOST_CHARACTERISTICS_EP_SPEED_MASK); // extract the speed field which is maintained at endpoint 0 and transfer it to all other endpoints
    ulAddressSpeedFields |= (ucDeviceAddress & HOST_CHARACTERISTICS_DEVICE_ADDRESS_MASK); // add the address
    while (iList++ < ASYNCHRONOUS_SCHEDULE_LENGTH) {
        ptrHostHead->ulHostCharacteristics = ((ptrHostHead->ulHostCharacteristics & ~(HOST_CHARACTERISTICS_DEVICE_ADDRESS_MASK | HOST_CHARACTERISTICS_EP_SPEED_MASK)) | ulAddressSpeedFields); // set the device address and speed to all endpoints
        ptrHostHead++;
    }
}
#endif

#if defined USB_HOST_SUPPORT
// Handle all pending periodic IN transactions
//
static void fnHandlePeriodic(int Channel)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrThisPeriodicHeader;
    int iPeriodicEndpoint;
    for (iPeriodicEndpoint = 0; iPeriodicEndpoint < NUMBER_OF_HOST_ENDPOINTS; iPeriodicEndpoint++) { // check each possible period endpoints
        ptrThisPeriodicHeader = ptrPeriodicHeader[Channel][iPeriodicEndpoint];
        if ((ptrThisPeriodicHeader != 0) && ((ptrThisPeriodicHeader->ulFirmwareFlags & PERIODIC_ENDPOINT_IGNORE) == 0)) { // there is an entry for this endpoint that should not presently be ignored
            if ((ptrThisPeriodicHeader->my_HostdTD->ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) == 0) { // terminated
                unsigned short usRxLength;
                unsigned short usMaxLength;
                usMaxLength = ((ptrThisPeriodicHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) >> HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT); // maximum length that could have been received
                usRxLength = (usMaxLength - ((ptrThisPeriodicHeader->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_TOTAL_BYTES_MASK) >> HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT)); // subtract the remaining length that could be received to get the actual amount of data
                hs_usb_hardware[Channel].usLength = usRxLength;
                hs_usb_endpoints[Channel][iPeriodicEndpoint].ulNextRxData0 ^= HOST_OVERLAY_CONTROL_DT;

                fnProcessHSInput(iPeriodicEndpoint, &hs_usb_hardware[Channel], USB_OUT_FRAME, (unsigned char *)ptrThisPeriodicHeader->ulBuffer0); // handle the received data

                if ((ptrThisPeriodicHeader->ulFirmwareFlags & PERIODIC_ENDPOINT_IGNORE) == 0) { // if not expressly disabled by USB_OUT_FRAME processing
                    // Allow further reception on the interrupt IN endpoint
                    //
                    ptrThisPeriodicHeader->my_HostdTD->ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
                    ptrThisPeriodicHeader->my_HostdTD->ptrNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
                    ptrThisPeriodicHeader->my_HostdTD->ulBufferPointerPage[0] = ptrThisPeriodicHeader->ulBuffer0;
                    ptrThisPeriodicHeader->my_HostdTD->ulBufferPointerPage[1] = ((ptrThisPeriodicHeader->ulBuffer0 + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
                    ptrThisPeriodicHeader->my_HostdTD->ulControl = (hs_usb_endpoints[Channel][iPeriodicEndpoint].ulNextRxData0 | HOST_OVERLAY_CONTROL_PID_CODE_IN | HOST_OVERLAY_CONTROL_IOC | HOST_OVERLAY_CONTROL_STATUS_ACTIVE | (usMaxLength << HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT));
                    ptrThisPeriodicHeader->host_transfer_overlay.ptrNext_qTD = (unsigned char *)ptrThisPeriodicHeader->my_HostdTD; // release the transmission
    #if defined _WINDOWS
                    ptrThisPeriodicHeader->host_transfer_overlay.ulControl = ptrThisPeriodicHeader->my_HostdTD->ulControl;
                    memcpy((unsigned char *)ptrThisPeriodicHeader->host_transfer_overlay.ulBufferPointerPage, (unsigned char *)ptrThisPeriodicHeader->my_HostdTD->ulBufferPointerPage, sizeof(ptrThisPeriodicHeader->host_transfer_overlay.ulBufferPointerPage));
    #endif
                }
            }
        }
    }
}

static unsigned short fnGetRxLength(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader)
{
    unsigned short usRxLength = ((ptrHostQueueHeader->ulHostCharacteristics & HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) >> HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT); // maximum length that could have been received
    usRxLength -= ((ptrHostQueueHeader->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_TOTAL_BYTES_MASK) >> HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT); // subtract the remaining length that could be received to get the actual amount of data                        
    return usRxLength;
}

// Handle all pending asynchronous IN/OUT transactions
//
static void fnHandleAsync(int Channel)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_EndpointQueueHeader[Channel];
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrThisHostQueueHeader;
    int _iEndpoint;
    for (_iEndpoint = 0; _iEndpoint < ASYNCHRONOUS_SCHEDULE_LENGTH; _iEndpoint++) { // check each possible asynchronous list entry
        int iEndpoint = (_iEndpoint/2);
        if (hs_usb_endpoints[Channel][iEndpoint].ulNextTxData0 != ENDPOINT_DISABLE) { // treat only endpoints that are active
            ptrThisHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
            if ((((ptrThisHostQueueHeader->ulFirmwareFlags & ASYNC_ENDPOINT_IGNORE) == 0)) && ((ptrThisHostQueueHeader->my_HostdTD->ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) == 0)) { // terminated on endpoint that should presently not be ignored
                unsigned char ucEvent = hs_usb_hardware[Channel].ucUSB_HostEvent[_iEndpoint]; // the event that this transfer triggers
                ptrThisHostQueueHeader->ulFirmwareFlags = ASYNC_ENDPOINT_IGNORE; // ignore this endpoint until it is used again
                hs_usb_hardware[Channel].ucUSB_HostEvent[_iEndpoint] = USB_NO_FRAME; // one-shot event no longer queued
                ptrThisHostQueueHeader->host_transfer_overlay.ptrNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
                switch (ucEvent) {
                case USB_TX_ACKED:                                       // our last transmission was acknowledged
#define TOGGLE_TEST_OUTPUT1() _TOGGLE_PORT(1, DEMO_LED_3)
#define TOGGLE_TEST_OUTPUT2() _TOGGLE_PORT(1, DEMO_LED_4)
//TOGGLE_TEST_OUTPUT1();
                    if (INITIATE_IN_TOKEN == fnUSB_handle_frame(ucEvent, 0, iEndpoint, &hs_usb_hardware[Channel])) { // handle ack event
                        fnHostReleaseBuffer(iEndpoint, IN_PID, &hs_usb_hardware[Channel]); // start transmission of IN token on the appropriate endpoint
                    }
                    break;
                case 0:                                                  // no specific event configured so it was a standard OUT reception
                    ucEvent = USB_OUT_FRAME;
                    // Fall through intentionally
                    //
                case USB_CONTROL_OUT_FRAME:                              // data reception from a setup
                    if ((ptrThisHostQueueHeader->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_STATUS_HALTED) != 0) { // the transmission was not successful
                        if (ptrThisHostQueueHeader->host_transfer_overlay.ulControl & (HOST_OVERLAY_CONTROL_STATUS_DATA_BUFFER_ERROR | HOST_OVERLAY_CONTROL_STATUS_BABBLE | HOST_OVERLAY_CONTROL_STATUS_TRANSACTION_ERROR | HOST_OVERLAY_CONTROL_STATUS_MISSED_uFRAME | HOST_OVERLAY_CONTROL_STATUS_SPLITXSTATE | HOST_OVERLAY_CONTROL_STATUSPING)) {
                        }
                        else {
                        }
                        fnDebugMsg("S:");
                        fnDebugDec(iEndpoint, 0);
                        fnDebugHex(ptrThisHostQueueHeader->host_transfer_overlay.ulControl, (sizeof(ptrThisHostQueueHeader->host_transfer_overlay.ulControl) | WITH_SPACE | WITH_LEADIN));
                        fnDebugHex(((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)hs_usb_hardware[Channel].ptrQueueHeader)->ulHostCharacteristics, (sizeof(((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)hs_usb_hardware[Channel].ptrQueueHeader)->ulHostCharacteristics) | WITH_SPACE | WITH_LEADIN | WITH_CR_LF));
                        if (iEndpoint == 0) {                            // the endpoint size is cleared by the controller in the case of a stall on the control endpoint
                            ((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)hs_usb_hardware[Channel].ptrQueueHeader)->ulHostCharacteristics |= (64 << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT); // configure the length again so that the endpoint can be used to clear the stall
                        }
                        ucEvent = USB_HOST_STALL_DETECTED;               // change the event to be handled into a stall event
                        ptrThisHostQueueHeader->host_transfer_overlay.ulControl &= ~(HOST_OVERLAY_CONTROL_STATUS_HALTED | HOST_OVERLAY_CONTROL_STATUS_DATA_BUFFER_ERROR | HOST_OVERLAY_CONTROL_STATUS_BABBLE | HOST_OVERLAY_CONTROL_STATUS_TRANSACTION_ERROR | HOST_OVERLAY_CONTROL_STATUS_MISSED_uFRAME | HOST_OVERLAY_CONTROL_STATUS_SPLITXSTATE | HOST_OVERLAY_CONTROL_STATUSPING); // clear the halt flags so that the queue can be used again
                        if ((ptrThisHostQueueHeader->host_transfer_overlay.ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) == HOST_OVERLAY_CONTROL_PID_CODE_IN) {
                            iEndpoint |= IN_ENDPOINT;                    // the stall was on the IN endpoint
                        }
                        fnProcessHSInput(iEndpoint, &hs_usb_hardware[Channel], ucEvent, hs_usb_hardware[Channel].ptrRx[iEndpoint]); // handle the stall
                    }
                    else {
//                        TOGGLE_TEST_OUTPUT2();
                        hs_usb_hardware[Channel].usLength = fnGetRxLength(ptrThisHostQueueHeader);
                        hs_usb_endpoints[Channel][iEndpoint].ulNextRxData0 ^= HOST_OVERLAY_CONTROL_DT; // toggle the data reception on each frame
                        if (fnProcessHSInput(iEndpoint, &hs_usb_hardware[Channel], ucEvent, hs_usb_hardware[Channel].ptrRx[iEndpoint]) != MAINTAIN_OWNERSHIP) { // handle the received data
                            ptrThisHostQueueHeader->ulHostCharacteristics = ((ptrThisHostQueueHeader->ulHostCharacteristics & ~HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) | (hs_usb_endpoints[Channel][iEndpoint].ulEndpointSize << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT));
                        }
                    }
                    break;
                }

            }
        }
        ptrHostQueueHeader++;
    }
}

static void fnDeinitHost(USB_HW *ptrUSB_HW)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrUSB_HW->ptrQueueHeader;
    unsigned long *ptrList = (unsigned long *)ptrUSB_HW->ptrHSUSB->PERIODICLISTBASE_DEVICEADDR;
    #if (HSUSB_CONTROLLERS > 1)
    int Channel = ptrUSB_HW->ucController;
    #else
    int Channel = 0;
    #endif
    unsigned char ucEndpointRef = 0;
    if (ptrList != 0) {                                                  // if the periodic list has exists
        int iList = 0;
        while (iList++ < PERIODIC_FRAME_LIST_ELEMENTS) {
            *ptrList++ = HOST_QUEUE_HEAD_WORD0_T;                        // invalid so that it is ignored
        }
    #if defined SUPPORT_QUEUE_DEALLOCATION
        while (ucEndpointRef < NUMBER_OF_HOST_ENDPOINTS) {
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = ptrPeriodicHeader[Channel][ucEndpointRef];
            if (ptrHostHead != 0) {
                if (ptrHostHead->my_HostdTD != 0) {
                    if (ptrHostHead->my_HostdTD->ulBufferPointerPage[0] != 0) {                    
                        uCFree((void *)ptrHostHead->my_HostdTD->ulBufferPointerPage[0]);
                    }
                    uCFree(ptrHostHead->my_HostdTD);
                }
                uCFree(--ptrHostHead);
                ptrPeriodicHeader[Channel][ucEndpointRef] = 0;
            }
            ucEndpointRef++;
        }
    #endif
    }
    if (ptrHostQueueHeader != 0) {
        ucEndpointRef = 0;
        while (++ucEndpointRef < NUMBER_OF_HOST_ENDPOINTS) {             // ensure all non endpoint 0 endpoints are ignored before configured
            ptrHostQueueHeader++;
            ptrHostQueueHeader->ulFirmwareFlags = ASYNC_ENDPOINT_IGNORE;
            ptrHostQueueHeader->host_transfer_overlay.ulControl = 0;
            if (ptrHostQueueHeader->my_HostdTD != 0) {
                ptrHostQueueHeader->my_HostdTD->ulControl = 0;
            }
        }
    }
    uMemset((void *)hs_usb_hardware[Channel].ucUSB_HostEvent, 0, sizeof(hs_usb_hardware[Channel].ucUSB_HostEvent)); // clear all pending host events
    fnInitialiseScheduleList(Channel);
}
#endif

// High-speed USB interrupt handler
//
#if (HSUSB_CONTROLLERS > 1) && defined USE_BOTH_USB
static void _usb_hs_otg_isr_common(KINETIS_USBHS *ptrHSUSB, int Channel)
#else
static __interrupt void _usb_hs_otg_isr(void)
#endif
{
    #define USBHS_STATE_RESETTING 0x01
    #define USBHS_STATE_SUSPENDED 0x02
    static unsigned char ucUSBHS_state[HSUSB_CONTROLLERS] = {0};
#if defined USB_DEVICE_SUPPORT
    unsigned char ucSetupBuffer[8];
#endif
    unsigned long ulInterrupts;
    register unsigned long ulUSB_StatusRegister;
    register unsigned long ulUSB_InterruptEnableRegister;
#if (HSUSB_CONTROLLERS > 1) && defined USE_BOTH_USB
#elif (HSUSB_CONTROLLERS > 1) && !defined USE_BOTH_USB && defined USE_USB2_ONLY
    KINETIS_USBHS *ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE2_ADD;
    int Channel = 1;
#else
    KINETIS_USBHS *ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE_ADD;
    int Channel = 0;
#endif

    FOREVER_LOOP() {                                                     // while enabled interrupt pending
        ulUSB_InterruptEnableRegister = ptrHSUSB->USBINTR;               // present state of interrupt enable register - interrupts that we want to handle
        ulUSB_StatusRegister = ptrHSUSB->USBSTS;                         // present state of interrupt status register - interrupts that may be pending
        if ((ulInterrupts = (ulUSB_InterruptEnableRegister & ulUSB_StatusRegister)) == 0) { // if there are no pending interrupt that we are interested in
            break;                                                       // leave the interrupt handeler
        }
        WRITE_ONE_TO_CLEAR(ptrHSUSB->USBSTS, ulInterrupts);              // clear all interrupts that will be handled
        if ((ulInterrupts & USBHS_USBINTR_URE) != 0) {                   // handle USB reset interrupt
            fnDeInitEndpoints(ptrHSUSB, Channel);                        // abort any active endpoints and free transfer buffers
            ptrHSUSB->EPCR0 = (USBHS_EPCR_RXE | USBHS_EPCR_RXR | USBHS_EPCR_TXE | USBHS_EPCR_TXR); // reset data toggle (synchronise) on endpoint 0
            ptrHSUSB->EPSETUPSR = ptrHSUSB->EPSETUPSR;                   // clear all setup token semaphores by reading the EPSETUSR register and writing the same value back
            ptrHSUSB->EPCOMPLETE = ptrHSUSB->EPCOMPLETE;                 // clear all the endpoint complete status bits by reading the EPCOMPLETE register and writing the same value back
#if defined _WINDOWS
            ptrHSUSB->EPSETUPSR = 0;
            ptrHSUSB->EPCOMPLETE = 0;
            ptrHSUSB->USBSTS = USBHS_USBSTS_PCI;                         // usually a port change interrupt follows a reset
#endif
            if ((ptrHSUSB->PORTSC1 & USBHS_PORTSC1_PR) == 0) {           // if we are too slow responding the port reset will have completed
                ptrHSUSB->USBCMD &= ~(USBHS_USBCMD_RS);                  // ensure not in run mode
                ptrHSUSB->USBCMD = USBHS_USBCMD_RST;                     // command a hardware reset
                fnUSBHS_init((QUEUE_LIMIT)Channel, ucHS_EndpointCount[Channel]); // re-initialise the controller
                ucUSBHS_state[Channel] = 0;
            }
            else {
                ucUSBHS_state[Channel] = USBHS_STATE_RESETTING;          // the ulpi is still detecting the reset state
            }

            ptrHSUSB->USBINTR &= ~(USBHS_USBINTR_SLE);                   // disable the suspend interrupt
            ptrHSUSB->PERIODICLISTBASE_DEVICEADDR = 0;                   // reset device address
            uDisable_Interrupt();                                        // ensure interrupts remain blocked when putting messages to queue
                fnUSB_handle_frame(USB_RESET_DETECTED, 0, 0, &hs_usb_hardware[Channel]); // generic handler routine
                hs_usb_hardware[Channel].ucUSBAddress = 0;               // reset the address to revert back to the default state
            uEnable_Interrupt();                                         // re-enable interrupts
        }
        if ((ulInterrupts & USBHS_USBSTS_UI) != 0) {                     // handle transfer complete interrupt(s)
#if defined USB_DEVICE_SUPPORT
            unsigned long ulBit = USBHS_EPSETUPSR_SETUP0;
            unsigned long ulEndpointBitReference;
            int iEndpointNumber = 0;
#endif
            if (__USB_HOST_MODE()) {                                     // if operating in host mode
#if defined USB_HOST_SUPPORT
                fnHandlePeriodic(Channel);
                fnHandleAsync(Channel);
#endif
            }
#if defined USB_DEVICE_SUPPORT
            else {
                ulEndpointBitReference = ptrHSUSB->EPSETUPSR;            // the transfer endpoint (status is not cleared here since it is cleared when the setup data is read from the buffer)
                while (ulEndpointBitReference != 0) {                    // first handle all waiting setup tokens
                    if ((ulBit & ulEndpointBitReference) != 0) {
                        hs_usb_hardware[Channel].ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR + (2 * iEndpointNumber);
                        ptrHSUSB->EPSETUPSR = ulBit;                     // clear the status bit
                        // Setup reception data is special in that it is stored directly in the 8 byte space in the queue header
                        // - we extract it here, with help from the tripwire semaphore
                        //
                        do {
                            ptrHSUSB->USBCMD |= (USBHS_USBCMD_SUTW);     // setup tripwire semaphore
                            uMemcpy(ucSetupBuffer, hs_usb_hardware[Channel].ptrQueueHeader->ucSetupBuffer, 8); // copy the fixed length setup content locally
                        } while ((ptrHSUSB->USBCMD & USBHS_USBCMD_SUTW) == 0); // if the hardware has cleared the semaphone we need to repeat to be sure of correct data
                        ptrHSUSB->USBCMD &= ~USBHS_USBCMD_SUTW;          // remove tripwire semaphore
                        hs_usb_hardware[Channel].usLength = 8;           // {3}
                        _WAIT_REGISTER_TRUE(ptrHSUSB->EPSETUPSR, ulEndpointBitReference);
                        ptrHSUSB->EPPRIME |= (USBHS_EPPRIME_PERB0 << iEndpointNumber); // prime endpoint 0 reception for OUT data that will follow
                        fnProcessHSInput(iEndpointNumber, &hs_usb_hardware[Channel], USB_SETUP_FRAME, ucSetupBuffer); // process the setup frame that has been received
                        ulEndpointBitReference &= ~(ulBit);              // handled
                    }
                    iEndpointNumber++;
                    ulBit <<= 1;
                }
            }
            ulEndpointBitReference = ptrHSUSB->EPCOMPLETE;
            WRITE_ONE_TO_CLEAR(ptrHSUSB->EPCOMPLETE, ulEndpointBitReference); // clear the endpoints since we are going to handle them here
            ulBit = USBHS_EPCOMPLETE_ERCE0;
            iEndpointNumber = 0;
            while ((ulEndpointBitReference & 0x0000ffff) != 0) {     // handle all waiting receptions
                if ((ulBit & ulEndpointBitReference) != 0) {
                    hs_usb_hardware[Channel].ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR + (2 * iEndpointNumber);
                    hs_usb_hardware[Channel].usLength = (unsigned short)((hs_usb_hardware[Channel].ptrQueueHeader->ulBufferLength >> ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT) - (hs_usb_hardware[Channel].ptrQueueHeader->my_dTD->ul_dtToken >> ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT)); // the length of the present frame
                    if ((*(&ptrHSUSB->EPCR0 + iEndpointNumber) & USBHS_EPCR_RXT_INT) != 0) { // check whether non-control endpoint
                        fnProcessHSInput(iEndpointNumber, &hs_usb_hardware[Channel], USB_OUT_FRAME, hs_usb_hardware[Channel].ptrQueueHeader->my_dTD->ptrBufferStart); // non-control endpoint
                    }
                    else {
                        fnProcessHSInput(iEndpointNumber, &hs_usb_hardware[Channel], USB_CONTROL_OUT_FRAME, hs_usb_hardware[Channel].ptrQueueHeader->my_dTD->ptrBufferStart); // control endpoint
                    }
                    ulEndpointBitReference &= ~(ulBit);
                }
                iEndpointNumber++;
                ulBit <<= 1;
            }
            ulBit = USBHS_EPCOMPLETE_ETCE0;
            iEndpointNumber = 0;
            while ((ulEndpointBitReference & 0xffff0000) != 0) {     // then handle all transmission complete interrupts
                if ((ulBit & ulEndpointBitReference) != 0) {
                    hs_usb_hardware[Channel].ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR + (2 * iEndpointNumber);
                    fnUSB_handle_frame(USB_TX_ACKED, 0, iEndpointNumber, &hs_usb_hardware[Channel]); // handle ack event
                    ptrHSUSB->PERIODICLISTBASE_DEVICEADDR = (hs_usb_hardware[Channel].ucUSBAddress << USBHS_DEVICEADDR_USBADR_SHIFT); // program the address to be used
                    ulEndpointBitReference &= ~(ulBit);
                }
                iEndpointNumber++;
                ulBit <<= 1;
            }
#endif
        }
        if ((ulInterrupts & USBHS_USBSTS_PCI) != 0) {                    // handle port change interrupt
            if (__USB_HOST_MODE()) {                                     // if operating in host mode
#if defined USB_HOST_SUPPORT
                if ((ptrHSUSB->PORTSC1 & USBHS_PORTSC1_CSC) != 0) {      // connection state change
    #if (HSUSB_CONTROLLERS > 1) && defined USE_BOTH_USB
                    if (Channel != 0) {
                        CLEAR_PORTSC1_FLAGS(1, USBHS_PORTSC1_CSC);       // clear connection state change flag
                    }
                    else {
                        CLEAR_PORTSC1_FLAGS(0, USBHS_PORTSC1_CSC);       // clear connection state change flag
                    }
    #else
                    CLEAR_PORTSC1_FLAGS(0, USBHS_PORTSC1_CSC);           // clear connection state change flag
    #endif
                    // NXP uses a loop with 1ms delay to clear the flag
                    // - to monitor
                    //
                    if ((ptrHSUSB->PORTSC1 & USBHS_PORTSC1_CCS) != 0) {  // attach detected
                        fnUSB_HS_HostDelaySingle(ptrHSUSB, usb_possible_attach, 50000); // wait a short time before checking whether this was a real attach (50000us)
                    }
                    else {                                               // detach detected
                        ptrHSUSB->USBCMD &= ~(USBHS_USBCMD_FS_128 | USBHS_USBCMD_FS2 | USBHS_USBCMD_PSE | USBHS_USBCMD_ASE); // stop asynchronous and periodic polling operation
    #if defined _iMX || defined KINETIS_WITH_USBPHY
                        fnPHY_detach(0, Channel);                        // disable HS host disconnection detection
    #endif
                        fnUSB_HS_HostDelaySingle(ptrHSUSB, 0, 0);        // kill the attach timer
                        fnDeinitHost(&hs_usb_hardware[Channel]);
                        fnUSB_handle_frame(USB_DEVICE_REMOVED, 0, 0, &hs_usb_hardware[Channel]); // generic handler routine
                    }
                }
#endif
            }
            else {
#if defined USB_DEVICE_SUPPORT
                ucUSBHS_state[Channel] &= ~USBHS_STATE_RESETTING;        // a reset must be complete when a port change interrupt is received
                if ((ptrHSUSB->PORTSC1 & USBHS_PORTSC1_SUSP) == 0) {     // not in the suspended state
                    if ((ucUSBHS_state[Channel] & USBHS_STATE_SUSPENDED) != 0) { // resume detected - 10ms inverted idle USB bus state (low speed D+ = 1, D- = 0 / high speed D+ = 0, D- = 1)
                        ucUSBHS_state[Channel] &= ~USBHS_STATE_SUSPENDED; // no longer in the suspend state
                        uDisable_Interrupt();                            // ensure interrupts remain blocked when putting messages to queue
                        fnUSB_handle_frame(USB_RESUME_DETECTED, 0, 0, 0); // generic handler routine
                        uEnable_Interrupt();                             // re-enable interrupts
                    }
                }
#endif
            }
        }
#if defined USB_HOST_SUPPORT                                             // handle USB timers
        if ((ulInterrupts & USBHS_USBINTR_TIE0) != 0) {                  // timer 0 timeout (single shot timer)
            if (timeout_handler0 != 0) {
                timeout_handler0(ptrHSUSB, Channel);
            }
        }
    #if 0
        if ((ulInterrupts & USBHS_USBINTR_TIE1) != 0) {                  // timer 1 timeout (repetitive timer)
            if (timeout_handler1 != 0) {
                timeout_handler1(ptrHSUSB, Channel);
            }
        }
    #endif
#endif
        if ((ulInterrupts & USBHS_USBINTR_UEE) != 0) {                   // handle USB error interrupt (these are not expected but such events could be debugged here)
            fnDebugMsg("HS USB error - ");
            fnDebugDec(Channel, WITH_CR_LF);
        }
        if ((ulInterrupts & USBHS_USBINTR_SLE) != 0) {                   // suspend state detected - 3ms of idle USB bus detected (low speed D+ = 0, D- = 1 / high speed D+ = 1, D- = 0)
            ucUSBHS_state[Channel] |= USBHS_STATE_SUSPENDED;
            uDisable_Interrupt();                                        // ensure interrupts remain blocked when putting messages to queue
                fnUSB_handle_frame(USB_SUSPEND_DETECTED, 0, 0, &hs_usb_hardware[Channel]); // generic handler routine
            uEnable_Interrupt();                                         // re-enable interrupts
        }
    }
}

#if (HSUSB_CONTROLLERS > 1) && defined USE_BOTH_USB
static __interrupt void _usb_hs_otg_isr1(void)
{
    _usb_hs_otg_isr_common((KINETIS_USBHS *)USBHS_BASE1_ADD, 0);
}

static __interrupt void _usb_hs_otg_isr2(void)
{
    _usb_hs_otg_isr_common((KINETIS_USBHS *)USBHS_BASE2_ADD, 1);
}
#endif

#if defined USB_FS_INTERFACE
    extern unsigned long fnUSB_HS_error_counters(int iChannel, int iValue)
#else
    extern unsigned long fnUSB_error_counters(int iChannel, int iValue)
#endif
{
    switch (iValue) {
    case USB_CRC_5_ERROR_COUNTER:
        return hs_USB_errors[iChannel].ulUSB_errors_CRC5;
    case USB_CRC_16_ERROR_COUNTER:
        return hs_USB_errors[iChannel].ulUSB_errors_CRC16;
    case USB_ERRORS_RESET:
        uMemset(&hs_USB_errors[iChannel], 0, sizeof(hs_USB_errors[iChannel]));
        break;
    }
    return 0;
}

// Set the transmit control and buffer to next active one and check whether it is free
//
#if defined USB_FS_INTERFACE
    extern int fnGetHSUSB_HW(int iEndpoint, USB_HW **ptr_hs_usb_hardware)
#else
    extern int fnGetUSB_HW(int iEndpoint, USB_HW **ptr_hs_usb_hardware)
#endif
{
    KINETIS_USBHS *ptrHSUSB;
#if (HSUSB_CONTROLLERS > 1)
    register int iChannel = UBS_HANDLE_TO_CHANNEL(iEndpoint);
#else
    register int iChannel = 0;
#endif
#if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    iEndpoint = UBS_HANDLE_TO_ENDPOINT(iEndpoint);
#endif
    ptrHSUSB = hs_usb_hardware[iChannel].ptrHSUSB;
    (*ptr_hs_usb_hardware)->ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR;
#if defined USB_HOST_SUPPORT
    if ((hs_usb_hardware[iChannel].ucModeType & USB_HOST_MODE) != 0) {
        if (hs_usb_endpoints[iChannel][iEndpoint].ulNextTxData0 == ENDPOINT_DISABLE) { // if the particular endpoint is not enabled
            return ENDPOINT_NOT_ACTIVE;
        }
        (*ptr_hs_usb_hardware)->ptrQueueHeader += 1;                     // move over the head element to first asynchronous list entry
    }
    else {
        if ((*(&ptrHSUSB->EPCR0 + iEndpoint) & USBHS_EPCR_TXE) == 0) {   // if the particular endpoint is not enabled for transmission 
            return ENDPOINT_NOT_ACTIVE;
        }
        (*ptr_hs_usb_hardware)->ptrQueueHeader += (2 * iEndpoint);
    }
#else
    if ((*(&ptrHSUSB->EPCR0 + iEndpoint) & USBHS_EPCR_TXE) == 0) {       // if the particular endpoint is not enabled for transmission 
        return ENDPOINT_NOT_ACTIVE;
    }
    (*ptr_hs_usb_hardware)->ptrQueueHeader += (2 * iEndpoint);
#endif
    #if defined USB_DEVICE_SUPPORT
    (*ptr_hs_usb_hardware)->ucDeviceType = USB_DEVICE_HS;
    #endif
    (*ptr_hs_usb_hardware)->ptrEndpoint = &hs_usb_endpoints[iChannel][iEndpoint];
#if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    (*ptr_hs_usb_hardware)->ucController = hs_usb_hardware[iChannel].ucController;
#endif
    (*ptr_hs_usb_hardware)->ptrHSUSB = ptrHSUSB;
#if defined USB_HOST_SUPPORT
    (*ptr_hs_usb_hardware)->ucModeType = hs_usb_hardware[iChannel].ucModeType;
#endif
    (*ptr_hs_usb_hardware)->ucDeviceType = USB_DEVICE_HS;
    return ENDPOINT_FREE;
}

// The hardware interface used to activate USB endpoints
//
#if defined USB_FS_INTERFACE
    extern void fnActivateHS_HWEndpoint(unsigned char ucEndpointType, unsigned char ucEndpointRef, unsigned short usEndpointLength, unsigned short usMaxEndpointLength, USB_ENDPOINT *ptrEndpoint)
#else
    extern void fnActivateHWEndpoint(unsigned char ucEndpointType, unsigned char ucEndpointRef, unsigned short usEndpointLength, unsigned short usMaxEndpointLength, USB_ENDPOINT *ptrEndpoint)
#endif
{
#if defined USB_DEVICE_SUPPORT
    KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader;
#endif
#if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)               // part with FS nd HS interface
    int Channel = 0;
#else
    int Channel = UBS_HANDLE_TO_CHANNEL(ucEndpointRef);
#endif
#if defined USB_DEVICE_SUPPORT
    unsigned long ulMult = ENDPOINT_QUEUE_HEADER_MULT_1;
    KINETIS_USBHS *ptrHSUSB = hs_usb_hardware[Channel].ptrHSUSB;
    volatile unsigned long *ptrEndPointCtr;
    unsigned long  endpoint_config;
    #if defined USB_SIMPLEX_ENDPOINTS                                    // {1}
    unsigned long original_endpoint_config;
    #endif
    if (usEndpointLength > 1024) {
        switch (usEndpointLength & (USB_HS_3_ISO_TRANSACTION | USB_HS_2_ISO_TRANSACTION)) {
        case USB_HS_2_ISO_TRANSACTION:
            ulMult = ENDPOINT_QUEUE_HEADER_MULT_2;
            break;
        case USB_HS_3_ISO_TRANSACTION:
            ulMult = ENDPOINT_QUEUE_HEADER_MULT_3;
            break;
        }
        usEndpointLength &= (USB_HS_2_ISO_TRANSACTION - 1);              // remove the mult field
    }
#endif
    ucEndpointRef = UBS_HANDLE_TO_ENDPOINT(ucEndpointRef);
#if defined USB_DEVICE_SUPPORT
    ptrEndPointCtr = (&ptrHSUSB->EPCR0 + ucEndpointRef);                 // endpoint control register (used by device)
    #if !defined USB_SIMPLEX_ENDPOINTS
    *ptrEndPointCtr &= ~(USBHS_EPCR_RXE | USBHS_EPCR_TXE);               // disable endpoint transmitter and receiver
    #endif
#endif
    hs_usb_endpoints[Channel][ucEndpointRef].ulNextRxData0 = 0;
    hs_usb_endpoints[Channel][ucEndpointRef].ulNextTxData0 = 0;
#if defined USB_HOST_SUPPORT
    if (__USB_HOST_MODE()) {
        hs_usb_endpoints[Channel][ucEndpointRef].ulEndpointSize = usEndpointLength;
        hs_usb_endpoints[Channel][ucEndpointRef].ulNextTxData0 = ucEndpointType; // use this entry as endpoint operation reference
        if ((ucEndpointType & (ENDPOINT_INTERRUPT | IN_ENDPOINT)) == (ENDPOINT_INTERRUPT | IN_ENDPOINT)) { // if the endpoint is an interrupt IN
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead;
            KINETIS_USBHS *ptrHSUSB = hs_usb_hardware[Channel].ptrHSUSB;
            unsigned long *ptrList = (unsigned long *)ptrHSUSB->PERIODICLISTBASE_DEVICEADDR;
            ptrHSUSB->USBCMD = ((ptrHSUSB->USBCMD & ~(USBHS_USBCMD_FS_128 | USBHS_USBCMD_FS2)) | USBHS_USBCMD_PSE | _FRAME_LIST_SIZE_VALUE); // enable periodic scheduling of 32 elements
            // Interrupt IN endpoint is added to the periodic frame list
            //
    #if defined SUPPORT_QUEUE_DEALLOCATION
            ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)uCallocAlign((MAX_MALLOC)(2 * sizeof(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER)), 32); // two 32 byte aligned queue head structure
    #else
            ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)uMallocAlign((MAX_MALLOC)(2 * sizeof(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER)), 32); // two 32 byte aligned queue head structure
    #endif
            ptrHostHead->ulHostCharacteristics = HOST_CHARACTERISTICS_H; // head of reclamation list (only the first entry is the head)
            ptrHostHead->ulQueueheadWord0 = ((unsigned long)(ptrHostHead + 1) | HOST_QUEUE_HEAD_WORD0_TYPE_QH); // point to next entry in the list
            ptrHostHead++;
            ptrHostHead->ulHostCharacteristics = (HOST_CHARACTERISTICS_RL_MASK | (usEndpointLength << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT) | HOST_CHARACTERISTICS_DTC | HOST_CHARACTERISTICS_EP_SPEED_HS | (ucEndpointRef << HOST_CHARACTERISTICS_ENDPOINT_SHIFT) | 1); // assume device address is 1
            ptrHostHead->ulQueueheadWord0 = ((unsigned long)(ptrHostHead - 1) | HOST_QUEUE_HEAD_WORD0_TYPE_QH | HOST_QUEUE_HEAD_WORD0_T); // // set wrap at the end (terminate periodic activity)
            ptrHostHead->ulHostCapabilities = (HOST_CAPABILITIES_MULT_1_TRANSACTION | 1);
    #if defined SUPPORT_QUEUE_DEALLOCATION
            ptrHostHead->my_HostdTD = (USB_HS_HOST_TRANSFER_OVERLAY *)uCallocAlign((MAX_MALLOC)(sizeof(USB_HS_HOST_TRANSFER_OVERLAY)), (32)); // assign a single 32 byte aligned transfer overlay for an empty entry
    #else
            ptrHostHead->my_HostdTD = (USB_HS_HOST_TRANSFER_OVERLAY *)uMallocAlign((MAX_MALLOC)(sizeof(USB_HS_HOST_TRANSFER_OVERLAY)), (32)); // assign a single 32 byte aligned transfer overlay for an empty entry
    #endif
            ptrHostHead->my_HostdTD->ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
            ptrHostHead->my_HostdTD->ptrNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
            ptrHostHead->my_HostdTD->ulControl = (HOST_OVERLAY_CONTROL_PID_CODE_IN | HOST_OVERLAY_CONTROL_IOC | HOST_OVERLAY_CONTROL_STATUS_ACTIVE | (usEndpointLength << HOST_OVERLAY_CONTROL_TOTAL_BYTES__SHIFT));
    #if defined SUPPORT_QUEUE_DEALLOCATION
            ptrHostHead->my_HostdTD->ulBufferPointerPage[0] = (unsigned long)uCalloc(1, usEndpointLength); // allocate reception buffer
    #else
            ptrHostHead->my_HostdTD->ulBufferPointerPage[0] = (unsigned long)uMalloc(usEndpointLength); // allocate reception buffer
    #endif
            ptrHostHead->ulBuffer0 = ptrHostHead->my_HostdTD->ulBufferPointerPage[0];
            ptrHostHead->my_HostdTD->ulBufferPointerPage[1] = (unsigned long)((ptrHostHead->my_HostdTD->ulBufferPointerPage[0] + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
            ptrHostHead->host_transfer_overlay.ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
            ptrHostHead->host_transfer_overlay.ptrNext_qTD = (unsigned char *)ptrHostHead->my_HostdTD; // release the transfer
    #if defined _WINDOWS
            ptrHostHead->host_transfer_overlay.ulControl = ptrHostHead->my_HostdTD->ulControl;
            memcpy((unsigned char *)ptrHostHead->host_transfer_overlay.ulBufferPointerPage, (unsigned char *)ptrHostHead->my_HostdTD->ulBufferPointerPage, sizeof(ptrHostHead->host_transfer_overlay.ulBufferPointerPage));
    #endif
            while ((*ptrList & HOST_QUEUE_HEAD_WORD0_T) == 0) {          // find next empty entry in the frame list
                ptrList++;
            }
            ptrPeriodicHeader[Channel][ucEndpointRef] = ptrHostHead;
            *ptrList = ((unsigned long)(ptrHostHead - 1) | HOST_QUEUE_HEAD_WORD0_TYPE_QH); // entry is active
        }
        else {                                                           // IN (not interrupt) or OUT
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)hs_usb_hardware[Channel].ptrQueueHeader;
            ptrHostHead += (ucEndpointRef * 2);
            if ((ucEndpointType & IN_ENDPOINT) != 0) {                  // if an IN endpoint
                hs_usb_hardware[Channel].ptrRx[ucEndpointRef] = (unsigned char *)uMalloc(usEndpointLength); // allocate linear reception memory
                ptrHostHead++;
            }
            ptrHostHead->ulHostCharacteristics |= (usEndpointLength << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT);
        }
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
        _SIM_USB(hs_usb_hardware[Channel].ucController, USB_SIM_ENUMERATED, 0, 0); // allow USB interrupt simulation
    #else
        _SIM_USB(Channel, USB_SIM_ENUMERATED, 0, 0);                     // inform the simulator that USB enumeration has completed
    #endif
        return;
    }
#endif
#if defined USB_DEVICE_SUPPORT
    hs_usb_endpoints[Channel][ucEndpointRef].ulEndpointSize = 0;
    ptrEndPointCtr = (&ptrHSUSB->EPCR0 + ucEndpointRef);                 // endpoint control register (used by device)
    if (ENDPOINT_DISABLE == ucEndpointType) {
    #if defined USB_SIMPLEX_ENDPOINTS
        *ptrEndPointCtr = 0;                                             // disable endpoint transmitter and receiver
    #endif
        ptrHSUSB->USBINTR &= ~(USBHS_USBINTR_SLE);                       // disable suspend interrupt when not configured
        return;
    }

    #if defined USB_SIMPLEX_ENDPOINTS
    original_endpoint_config = *ptrEndPointCtr;                          // {1} backup original endpoint configuration
    #endif
    switch (ucEndpointType & ~IN_ENDPOINT) {
    case ENDPOINT_ISOCHRONOUS:
        endpoint_config = (USBHS_EPCR_TXE | USBHS_EPCR_TXT_ISO | USBHS_EPCR_RXE | USBHS_EPCR_RXT_ISO); 
        break;
    case ENDPOINT_BULK:
        endpoint_config = (USBHS_EPCR_TXE | USBHS_EPCR_TXT_BULK | USBHS_EPCR_RXE | USBHS_EPCR_RXT_BULK);
        break;
    case ENDPOINT_INTERRUPT:
        endpoint_config = (USBHS_EPCR_TXE | USBHS_EPCR_TXT_INT | USBHS_EPCR_RXE | USBHS_EPCR_RXT_INT);
        break;
    case ENDPOINT_CONTROL:
    default:
        endpoint_config = (USBHS_EPCR_TXE | USBHS_EPCR_TXT_CON | USBHS_EPCR_RXE | USBHS_EPCR_RXT_CON);
        break;
    }
    WRITE_ONE_TO_CLEAR(ptrHSUSB->USBSTS, USBHS_USBINTR_SLE);             // clear possible pending suspend interrupt
    ptrHSUSB->USBINTR |= (USBHS_USBINTR_SLE);                            // since we have just been configured, enable suspend interrupt
    ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR + (2 * ucEndpointRef); // reception queue header
    if ((ucEndpointType & IN_ENDPOINT) == 0) {                           // output type endpoint (or control)
        if ((ucEndpointType & ~IN_ENDPOINT) != ENDPOINT_CONTROL) {
            endpoint_config &= ~(USBHS_EPCR_TXE | USBHS_EPCR_TXT_INT);   // disable transmission
        }
        if (ptrQueueHeader->my_dTD->ptrBufferStart == 0) {               // if the endpoint is being used for the first time
            if (usMaxEndpointLength > usEndpointLength) {                // if no larger specified take the value
                usEndpointLength = usMaxEndpointLength;
            }
            ptrQueueHeader->ulBufferLength = usEndpointLength;
            if ((ptrEndpoint->usParameters & USB_OUT_ZERO_COPY) != 0) {  // {2} use zero copy type of endpoint buffer
                QUEQUE *ptrQueue = (QUEQUE *)(ptrEndpoint->ptrEndpointOutCtr);
                ptrQueue->new_chars = 0;
                ptrQueue->put = ptrQueue->QUEbuffer;
                ptrQueueHeader->my_dTD->ptrBufferStart = ptrQueue->QUEbuffer; // set descriptor data pointer to start of linear reception buffer
            }
            else {
                ptrQueueHeader->my_dTD->ptrBufferStart = (unsigned char *)uMalloc((MAX_MALLOC)(ptrQueueHeader->ulBufferLength));
            }
            ptrQueueHeader->ulBufferLength <<= ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT; // move the length into position for later register use
        }
        ptrQueueHeader->ulCapabilities = ptrQueueHeader->ulEndpointLength = (usEndpointLength << ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT);
        if ((ucEndpointType & ~IN_ENDPOINT) != ENDPOINT_CONTROL) {
            ptrQueueHeader->ulCapabilities |= ENDPOINT_QUEUE_HEADER_ZLT; // disable zero data frame on non-control endpoints
        }

        // Prepare for reception on this endpoint
        //
        ptrQueueHeader->my_dTD->ulNextdTD_pointer = ENDPOINT_QUEUE_HEADER_NEXT_INVALID; // single transfer block
        ptrQueueHeader->CurrentdTD_pointer = ptrQueueHeader->my_dTD;
        ptrQueueHeader->dTD.ulBufferPointerPage[0] = (unsigned long)(ptrQueueHeader->my_dTD->ptrBufferStart);
        ptrQueueHeader->dTD.ulBufferPointerPage[1] = ((unsigned long)(ptrQueueHeader->dTD.ulBufferPointerPage[0] + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK);
        fnPrimeReception(Channel, 0, ptrQueueHeader, ucEndpointRef);
    }
    if (((ucEndpointType & IN_ENDPOINT) != 0) || ((ucEndpointType & ~IN_ENDPOINT) == ENDPOINT_CONTROL)) { // input type endpoint (or control)
        ptrQueueHeader++;                                                // move to transmission queue header
        ptrQueueHeader->ulCapabilities = ptrQueueHeader->ulEndpointLength = (usEndpointLength << ENDPOINT_QUEUE_HEADER_MAX_PACKET_LEN_SHIFT);
        if ((ucEndpointType & ~IN_ENDPOINT) != ENDPOINT_CONTROL) {
            if ((ucEndpointType & ~IN_ENDPOINT) == ENDPOINT_ISOCHRONOUS) {
                ptrQueueHeader->ulCapabilities |= ulMult;                // set ISO transactions per frame
            }
            else {
                ptrQueueHeader->ulCapabilities |= ENDPOINT_QUEUE_HEADER_ZLT; // disable zero data frame on non-control endpoints
            }
            endpoint_config &= ~(USBHS_EPCR_RXE | USBHS_EPCR_RXT_INT);   // disable reception
        }
    }
    #if defined USB_SIMPLEX_ENDPOINTS
    endpoint_config |= original_endpoint_config;                         // {1} combine original with new endpoint configuration to allow shared IN/OUT operation
    #endif
    // When only one direction of an endpoint is used the unused endpoint pair should be configured as a bulk endpoint
    //
    if ((endpoint_config & USBHS_EPCR_RXE) == 0) {                       // receiver in pair is not being used
        endpoint_config |= USBHS_EPCR_RXT_BULK;                          // set to bulk
    }
    if ((endpoint_config & USBHS_EPCR_TXE) == 0) {                       // transmitter in pair is not being usd
        endpoint_config |= USBHS_EPCR_TXT_BULK;                          // set to bulk
    }
    *ptrEndPointCtr = endpoint_config;                                   // set the endpoint mode
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    _SIM_USB(hs_usb_hardware[Channel].ucController, USB_SIM_ENUMERATED, 0, 0); // inform the simulator that USB enumeration has completed
    #else
    _SIM_USB(Channel, USB_SIM_ENUMERATED, 0, 0);                         // inform the simulator that USB enumeration has completed
    #endif
#endif
}

// This routine is called with interrupts blocked for deferred read of a reception buffer
//
#if defined USB_FS_INTERFACE
    extern int fnConsumeHSUSB_out(unsigned char ucEndpointRef)
#else
    extern int fnConsumeUSB_out(unsigned char ucEndpointRef)
#endif
{
#if defined USB_DEVICE_SUPPORT
    KINETIS_USBHS *ptrHSUSB;
    KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *ptrQueueHeader;
#endif
    unsigned short usLength;
#if (HSUSB_CONTROLLERS > 1)
    int Channel = UBS_HANDLE_TO_CHANNEL(ucEndpointRef);
    ucEndpointRef = UBS_HANDLE_TO_ENDPOINT(ucEndpointRef);
#else
    int Channel = 0;
#endif
#if defined USB_HOST_SUPPORT
    if (__USB_HOST_MODE()) {
        KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_EndpointQueueHeader[Channel];
        ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrHostQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_MASK);
        ptrHostQueueHeader += ((2 * ucEndpointRef) + 1);
        if ((ptrHostQueueHeader->my_HostdTD->ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) == 0) { // the buffer is expected to not be active
            int iAction;
            usLength = fnGetRxLength(ptrHostQueueHeader);
            if (usLength == 0) {
                return USB_BUFFER_BLOCKED;
            }
            if ((iAction = fnEndpointData(ucEndpointRef, hs_usb_hardware[Channel].ptrRx[ucEndpointRef], usLength, OUT_DATA_RECEPTION, &hs_usb_hardware[Channel])) != MAINTAIN_OWNERSHIP) {
                ptrHostQueueHeader->ulHostCharacteristics = ((ptrHostQueueHeader->ulHostCharacteristics & ~HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK) | (hs_usb_endpoints[Channel][ucEndpointRef].ulEndpointSize << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT));
                if (INITIATE_IN_TOKEN == iAction) {
                    fnHostReleaseBuffer(ucEndpointRef, IN_PID, &hs_usb_hardware[Channel]); // start transmission of IN token on the endpoint
                }            
                return USB_BUFFER_FREED;                                         // buffer consumed and freed
            }
        }
        return USB_BUFFER_BLOCKED;
    }
#endif
#if defined USB_DEVICE_SUPPORT
    ptrHSUSB = hs_usb_hardware[Channel].ptrHSUSB;
    ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)ptrHSUSB->ASYNCLISTADDR_EPLISTADDR + (2 * ucEndpointRef);
    if ((ptrQueueHeader->dTD.ul_dtToken & ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE) != 0) { // no data available
        return USB_BUFFER_NO_DATA;
    }
    usLength = (unsigned short)((ptrQueueHeader->ulBufferLength >> ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT) - (ptrQueueHeader->my_dTD->ul_dtToken >> ENDPOINT_QUEUE_HEADER_TOKEN_TOTAL_BYTES_SHIFT)); // the length of the present frame
    if (fnEndpointData(ucEndpointRef, ptrQueueHeader->my_dTD->ptrBufferStart, usLength, OUT_DATA_RECEPTION, &hs_usb_hardware[Channel]) != MAINTAIN_OWNERSHIP) {
        fnPrimeReception(Channel, usLength, ptrQueueHeader, ucEndpointRef); // re-enable reception on this endpoint
        return USB_BUFFER_FREED;                                         // buffer consumed and freed
    }
#endif
    return USB_BUFFER_BLOCKED;
}


#if defined USB_HOST_SUPPORT
extern int iDebug;
static void fnDisplay(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrTxQueueHeader)
{
    int i;
    unsigned long *ptrContent = (unsigned long *)ptrTxQueueHeader;
    unsigned char *ptrDataContent;
    for (i = 0; i < (sizeof(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER) / 4); i++) {
        fnDebugHex(*ptrContent++, (sizeof(unsigned long) | WITH_SPACE | WITH_LEADIN));
        if (i%4 == 3) {
            fnDebugMsg("\r\n");
        }
    }
    fnDebugMsg("\r\n");
    ptrContent = (unsigned long *)ptrTxQueueHeader->my_HostdTD;
    for (i = 0; i < (sizeof(USB_HS_HOST_TRANSFER_OVERLAY) / 4); i++) {
        fnDebugHex(*ptrContent++, (sizeof(unsigned long) | WITH_SPACE | WITH_LEADIN));
        if (i % 4 == 3) {
            fnDebugMsg("\r\n");
        }
    }
    fnDebugMsg("\r\n");
    ptrDataContent = (unsigned char *)ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[0];
    for (i = 0; i < 8; i++) {
        fnDebugHex(*ptrDataContent++, (sizeof(unsigned char) | WITH_SPACE | WITH_LEADIN));
    }
    fnDebugMsg("\r\n");
}

extern void fnHostReleaseBuffer(int iEndpoint, unsigned char ucTransferType, USB_HW *ptrUSB_HW)
{
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrTxQueueHeader;
    #if HSUSB_CONTROLLERS > 1
    int iChannel = ptrUSB_HW->ucController;
    iEndpoint = UBS_HANDLE_TO_ENDPOINT(iEndpoint);
    #else
    int iChannel = 0;
    #endif
    #if defined _WINDOWS
    int iSimEndpoint = (iEndpoint * 2);
    #endif
    ptrUSB_HW = &hs_usb_hardware[iChannel];                              // ensure that we are using the real hardware object and not a temporary one (passed by generic driver in some circumstances)
    ptrTxQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)(ptrUSB_HW->ptrQueueHeader + (iEndpoint * 2)); // the endpoint queue
    switch (ucTransferType) {
    case OUT_PID:                                                        // sending an OUT
        ptrTxQueueHeader->my_HostdTD->ulControl |= ((hs_usb_endpoints[iChannel][iEndpoint].ulNextTxData0 & HOST_OVERLAY_CONTROL_DT) | HOST_OVERLAY_CONTROL_IOC | HOST_OVERLAY_CONTROL_STATUS_ACTIVE | HOST_OVERLAY_CONTROL_PID_CODE_OUT); // insert the PID type
        hs_usb_endpoints[iChannel][iEndpoint].ulNextTxData0 ^= HOST_OVERLAY_CONTROL_DT; // toggle the data
        if ((ptrTxQueueHeader->ulQueueheadWord0 & HOST_QUEUE_HEAD_WORD0_TYPE_FSTN) != HOST_QUEUE_HEAD_WORD0_TYPE_iTD) { // not ISO endpoints
            ptrUSB_HW->ucUSB_HostEvent[iEndpoint * 2] = USB_TX_ACKED;    // event to be handled when the OUT has been acknowledged
        }
        break;
    case SETUP_PID:                                                      // sending a SETUP
        {
            unsigned short usRequestedLength = *((unsigned char *)ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[0] + 7); // extract the requested length from the setup content
            usRequestedLength <<= 8;
            usRequestedLength |= *((unsigned char *)ptrTxQueueHeader->my_HostdTD->ulBufferPointerPage[0] + 6);
            ptrUSB_HW->usEndpointLength[iEndpoint] = usRequestedLength;  // remember the length that is being requested
            ptrTxQueueHeader->my_HostdTD->ulControl |= (HOST_OVERLAY_CONTROL_IOC | HOST_OVERLAY_CONTROL_STATUS_ACTIVE | HOST_OVERLAY_CONTROL_PID_CODE_SETUP); // insert the PID type
            ptrUSB_HW->ucUSB_HostEvent[iEndpoint * 2] = USB_TX_ACKED;    // event to be handled when the SETUP has been acknowledged
            hs_usb_endpoints[iChannel][iEndpoint].ulNextTxData0 |= HOST_OVERLAY_CONTROL_DT; // reset the data toggle at each setup
            hs_usb_endpoints[iChannel][iEndpoint].ulNextRxData0 |= HOST_OVERLAY_CONTROL_DT;
        }
        break;
    case IN_PID:                                                         // start IN polling on the endpoint
        {
            unsigned short usLength = ptrUSB_HW->usEndpointLength[iEndpoint];
            if (iEndpoint == 0) {                                        // control endpoint will have sent a setup prior to setting the IN tokens
                if (hs_usb_endpoints[iChannel][iEndpoint].ulEndpointSize < usLength) { // if the remaining length of expected data is less that the endpoint's ength
                    usLength = (unsigned short)hs_usb_endpoints[iChannel][iEndpoint].ulEndpointSize; // limit the single token to the maximum endpoint length
                }
                ptrUSB_HW->usEndpointLength[iEndpoint] -= usLength;      // decrement the remaining expected maximum data
            }
            fnTxUSBHS(ptrUSB_HW->ptrRx[iEndpoint], usLength, (iEndpoint | 0x80000000), ptrUSB_HW);
            ptrTxQueueHeader++;
            ptrTxQueueHeader->ulHostCharacteristics = ((ptrTxQueueHeader->ulHostCharacteristics & ~(HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_MASK)) | (usLength << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT));
            ptrTxQueueHeader->my_HostdTD->ulControl |= (hs_usb_endpoints[iChannel][iEndpoint].ulNextRxData0 | HOST_OVERLAY_CONTROL_IOC | HOST_OVERLAY_CONTROL_STATUS_ACTIVE | HOST_OVERLAY_CONTROL_PID_CODE_IN);
            if ((hs_usb_endpoints[iChannel][iEndpoint].ulNextTxData0 & ENDPOINT_INVALID) == ENDPOINT_CONTROL) { // if a control endpoint
                ptrUSB_HW->ucUSB_HostEvent[(iEndpoint * 2) + 1] = USB_CONTROL_OUT_FRAME; // event to be handled when the IN has terminated
            }
    #if defined _WINDOWS
            iSimEndpoint++;
    #endif
        }
        break;
    }
    ptrTxQueueHeader->host_transfer_overlay.ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
    ptrTxQueueHeader->host_transfer_overlay.ptrNext_qTD = (unsigned char *)ptrTxQueueHeader->my_HostdTD; // release the transmission
    ptrTxQueueHeader->ulFirmwareFlags = 0;                               // the endpoint should no longer be ignored when handling async interrupts
    if (iDebug != 0) {
        fnDisplay(ptrTxQueueHeader);
        iDebug = 0;
    }
    #if (HSUSB_CONTROLLERS > 1) || (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    _SIM_USB(hs_usb_hardware[iChannel].ucController, USB_SIM_TX, iSimEndpoint, ptrUSB_HW); // allow USB interrupt simulation
    #else
    _SIM_USB(iChannel, USB_SIM_TX, iSimEndpoint, ptrUSB_HW);
    #endif
}

extern void fnResetDataToggle(int iChannel, int iEndpoint, USB_HW *ptrUSB_HW)
{    
    hs_usb_endpoints[iChannel][(iEndpoint & ~(IN_ENDPOINT))].ulNextRxData0 &= ~(HOST_OVERLAY_CONTROL_DT); // next IN reception from the endpoint will be changed to DATA 0
}

extern int fnHostEndpoint(unsigned char ucEndpoint, int iCommand, int iEvent)
{
    switch (iCommand) {
    case IN_POLLING:
        {
            int iChannel = UBS_HANDLE_TO_CHANNEL(ucEndpoint);
            KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostQueueHeader;
            USB_HW *ptrUSB_HW = &hs_usb_hardware[iChannel];
            ucEndpoint = UBS_HANDLE_TO_ENDPOINT(ucEndpoint);
            ptrHostQueueHeader = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrUSB_HW->ptrQueueHeader;
            ptrHostQueueHeader += ucEndpoint;
            if (iEvent != 0) {                                           // enable continuous IN polling
                volatile unsigned long ulMaxWait = 50000;
                if ((ptrHostQueueHeader->my_HostdTD->ulControl & HOST_OVERLAY_CONTROL_PID_CODE_MASK) == HOST_OVERLAY_CONTROL_PID_CODE_IN) { // if the endpoint was previously used for an IN token
                    if ((ptrHostQueueHeader->ulFirmwareFlags & ASYNC_ENDPOINT_IGNORE) == 0) { // if a previous IN transaction on this endpoint hasn't been handled yet ignore
                        return 0;                                            
                    }
                }
                while ((ptrHostQueueHeader->my_HostdTD->ulControl & HOST_OVERLAY_CONTROL_STATUS_ACTIVE) != 0) { // wait until any other present token has completed
    #if defined _WINDOWS
                    fnSimInts(0);                                        // allow the USB interrupts to be simulated
    #endif
                    if (--ulMaxWait == 0) {
                        fnDebugMsg("POLL-fail ");
                        fnDebugDec(ucEndpoint, 0);
                        fnDebugHex(ptrHostQueueHeader->my_HostdTD->ulControl, (WITH_SPACE | WITH_LEADIN | sizeof(ptrHostQueueHeader->my_HostdTD->ulControl) | WITH_CR_LF));
                        return -1;
                    }
                }
                ptrUSB_HW->usEndpointLength[ucEndpoint] = HSUSB_INPUT_BUFFER_SIZE; // allow maximum reception size
                fnHostReleaseBuffer(ucEndpoint, IN_PID, ptrUSB_HW);      // start IN polling on this endpoint (the new endpoint is set as active)
            }
            else {                                                       // disable IN polling
                ptrHostQueueHeader->my_HostdTD->ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE;
                ptrHostQueueHeader->host_transfer_overlay.ulControl &= ~HOST_OVERLAY_CONTROL_STATUS_ACTIVE;
            }
        }
        break;
    }
    return 0;
}

// Create or reset an asynchronous schedule list
//
static void fnInitialiseScheduleList(int Channel)
{
    int iList = 0;
    KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *ptrHostHead = (KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)ptrHS_EndpointQueueHeader[Channel]; // the head of reclamation list
    hs_usb_hardware[Channel].ptrQueueHeader = (KINETIS_USBHS_ENDPOINT_QUEUE_HEADER *)(ptrHostHead + 1); // enter the first endpoint entry
    while (iList < (ASYNCHRONOUS_SCHEDULE_LENGTH + 1)) {                 // for each entry
        if (iList == 0) {                                                // head
            ptrHostHead->ulHostCharacteristics = HOST_CHARACTERISTICS_H; // head of reclamation list (only the first entry is the head)
        }
        else {                                                           // endpoint entries
            ptrHostHead->ulHostCharacteristics = (HOST_CHARACTERISTICS_RL_MASK | HOST_CHARACTERISTICS_DTC | HOST_CHARACTERISTICS_EP_SPEED_HS | (((iList - 1)/2) << HOST_CHARACTERISTICS_ENDPOINT_SHIFT)); // set default characteristics with incrementing endpoint numbers
            ptrHostHead->ulHostCapabilities = HOST_CAPABILITIES_MULT_1_TRANSACTION;
            if (ptrHostHead->my_HostdTD == 0) {                          // initialisation and not reset
                ptrHostHead->my_HostdTD = (USB_HS_HOST_TRANSFER_OVERLAY *)uMallocAlign((MAX_MALLOC)(AYNCHRONOUS_SCHEDULE_TRANSFER_OVERLAYS * 32), (32)); // assign a list of 32 byte aligned transfer overlays for each entry
            }
        }
        ptrHostHead->host_transfer_overlay.ptrAlternateNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE; // initially not active
        ptrHostHead->host_transfer_overlay.ptrNext_qTD = (unsigned char *)HOST_OVERLAY_TERMINATE;
        ptrHostHead->ulFirmwareFlags = ASYNC_ENDPOINT_IGNORE;
        if (iList >= ASYNCHRONOUS_SCHEDULE_LENGTH) {
            ptrHostHead->ulQueueheadWord0 = ((unsigned long)ptrHS_EndpointQueueHeader[Channel] | HOST_QUEUE_HEAD_WORD0_TYPE_QH); // set wrap at the end
        }
        else {
            ptrHostHead->ulQueueheadWord0 = ((unsigned long)(ptrHostHead + 1) | HOST_QUEUE_HEAD_WORD0_TYPE_QH); // point to next entry in the list
        }
        ptrHostHead++;
        iList++;
    }
    if (hs_usb_hardware[Channel].ptrRx[0] == 0) {                        // on creation
        hs_usb_hardware[Channel].ptrRx[0] = uMalloc(64);                 // allocate memory for linear HS USB reception buffer for the control endpoint
        hs_usb_endpoints[Channel][0].ulEndpointSize = 64;
    }
    ((KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER *)hs_usb_hardware[Channel].ptrQueueHeader)->ulHostCharacteristics |= (64 << HOST_CHARACTERISTICS_MAX_PACKET_LENGTH_SHIFT);
}
#endif

// USBHS controller initialisation
//
static void fnUSBHS_init(QUEUE_HANDLE Channel, unsigned char ucEndpoints)
{
    KINETIS_USBHS *ptrHSUSB;
    #if (HSUSB_CONTROLLERS > 1)
    if (Channel != 0) {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE2_ADD;
    }
    else {
        ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE1_ADD;
    }
    #else
    ptrHSUSB = (KINETIS_USBHS *)USBHS_BASE_ADD;                          // HSUSB base registers address
    #endif

    if (__USB_HOST_MODE()) {                                             // host mode of operation
    #if defined USB_HOST_SUPPORT
        ptrHSUSB->USBCMD = USBHS_USBCMD_RST;                             // reset the controller
        _WAIT_REGISTER_TRUE(ptrHSUSB->USBCMD, USBHS_USBCMD_RST);         // wait for the reset to complete
        ptrHSUSB->USBMODE = (USBHS_USBMODE_RESERVED_BITS | USBHS_USBMODE_CM_HOST | USBHS_USBMODE_ES_LITTLE/* | USBHS_USBMODE_SLOM | USBHS_USBMODE_SDIS*/); // set little-endian host mode of operation
        ptrHSUSB->PORTSC1 = (USBHS_PORTSC1_PTW_16 | USBHS_PORTSC1_PP);   // turn on power port in 16-bit parallel mode
        ptrHSUSB->USBCMD = (USBHS_USBCMD_ITC_8_MF | USBHS_USBCMD_ASPE | USBHS_USBCMD_ASP3 | USBHS_USBCMD_RS); // set run mode in order to start the controller with maximum interrupt rate set to 8 microframes and using the asynchronous scheduler
        ptrHSUSB->PERIODICLISTBASE_DEVICEADDR = 0;                       // ensure the period list pointer is 0 to mark that it is not yet in use
    #endif
    }
    else {                                                               // device mode
    #if defined USB_DEVICE_SUPPORT
        #if defined _iMX
        do {
            ptrHSUSB->USBMODE = (USBHS_USBMODE_RESERVED_BITS | USBHS_USBMODE_CM_DEVICE | USBHS_USBMODE_ES_LITTLE | USBHS_USBMODE_SLOM/* | USBHS_USBMODE_SDIS*/); // note that stream is disabled so that single rx buffering is possible (this may reduce OUT speed but is simpler, especially when using for software uploading where the speed is limited by flash programming anyway)
        } while ((ptrHSUSB->USBMODE & USBHS_USBMODE_CM_DEVICE) == 0);    // since the USB controller software reset may not have completed retry until the mode has been successfully set
        #else
        ptrHSUSB->USBMODE = (USBHS_USBMODE_CM_DEVICE | USBHS_USBMODE_ES_LITTLE | USBHS_USBMODE_SLOM/* | USBHS_USBMODE_SDIS*/); // note that stream is disabled so that single rx buffering is possible (this may reduce OUT speed but is simpler, especially when using for software uploading where the speed is limited by flash programming anyway)
        #endif
    #endif
    }
    WRITE_ONE_TO_CLEAR(ptrHSUSB->EPSETUPSR, 0xffffffff);                 // clear endpoint setup register (write '1' to clear each flag)
    if (ptrHS_EndpointQueueHeader[Channel] == 0) {                       // allocate endpoint header queue on first call
        if (__USB_HOST_MODE()) {
    #if defined USB_HOST_SUPPORT
            // Create an endpoint queue - each header is 48 bytes in size but has to be aligned on 64 byte boundary
            //
            int iList = 0;
            unsigned long *ptrList = (unsigned long *)uMallocAlign((MAX_MALLOC)((PERIODIC_FRAME_LIST_ELEMENTS * sizeof(unsigned long))), (4 * 1024)); // 4k aligned periodic frame list
            ptrHSUSB->PERIODICLISTBASE_DEVICEADDR = (unsigned long)ptrList;
            while (iList++ < PERIODIC_FRAME_LIST_ELEMENTS) {             // PERIODIC_FRAME_LIST_ELEMENTS should be set to enable the maximum polling period to be used
                *ptrList++ = HOST_QUEUE_HEAD_WORD0_T;                    // invalid so that it is ignored
            }
            iList = 0;
            while (++iList < ucEndpoints) {
                hs_usb_endpoints[Channel][iList].ulNextTxData0 = ENDPOINT_DISABLE; // mark all but endpoint 0 as disabled
            }
            ptrHS_EndpointQueueHeader[Channel] = uMallocAlign((MAX_MALLOC)(sizeof(KINETIS_USBHS_HOST_ENDPOINT_QUEUE_HEADER) * (ASYNCHRONOUS_SCHEDULE_LENGTH + 1)), 64); // get endpoint queue headers starting at a 64 byte boundary - one head and one for each endpoint
            fnInitialiseScheduleList(Channel);
    #endif
        }
        else {
    #if defined USB_DEVICE_SUPPORT
            // Create an endpoint queue - each header is 48 bytes in size but has to be aligned on 64 byte boundary - the queue itself must be 2k aligned
            //
            int i;
            unsigned char *ptrEP0_rx;
            ptrHS_EndpointQueueHeader[Channel] = uMallocAlign((MAX_MALLOC)(sizeof(KINETIS_USBHS_ENDPOINT_QUEUE_HEADER) * ucEndpoints * 2), (2 * 1024)); // get endpoint queue headers starting at a 2k boundary
            for (i = 0; i < (ucEndpoints * 2); i++) {                    // create single transfer block for each receive and transmit pipe and endpoint
                (ptrHS_EndpointQueueHeader[Channel] + i)->my_dTD = (USB_HS_TRANSFER_OVERLAY *)uMallocAlign((MAX_MALLOC)(32), (32)); // allocate and enter the location of the dedicated transfer block (32 byte size dTD aligned on 32 byte boundary)
                (ptrHS_EndpointQueueHeader[Channel] + i)->my_dTD->ulNextdTD_pointer = ENDPOINT_QUEUE_HEADER_NEXT_INVALID; // single transfer block
            }
            // Endpoint 0 requires a small buffer in case it needs to receive OUT data (it often receives zero data too so must be primed at least for this reception)
            //
            ptrEP0_rx = uMalloc(64);                                     // allocate 64 bytes for endpoint 0 reception
            ptrHS_EndpointQueueHeader[Channel]->dTD.ulBufferPointerPage[0] = (unsigned long)ptrEP0_rx;
            ptrHS_EndpointQueueHeader[Channel]->dTD.ulBufferPointerPage[1] = ((unsigned long)(ptrEP0_rx + (4 * 1024)) & ENDPOINT_QUEUE_HEADER_BUFFER_POINTER_MASK); // two pages defined to allow any page boundary for the first 4k of data
            ptrHS_EndpointQueueHeader[Channel]->ulBufferLength = ptrHS_EndpointQueueHeader[Channel]->ulEndpointLength = (64 << 16);
            //ptrHS_EndpointQueueHeader[Channel]->dTD.ulBufferPointerPage[2] = (ptrUSB_HW->ptrQueueHeader->dTD.ulBufferPointerPage[1] + (4 * 1024)); // further page buffers could be used in case of large reception content
            //ptrHS_EndpointQueueHeader[Channel]->dTD.ulBufferPointerPage[3] = (ptrUSB_HW->ptrQueueHeader->dTD.ulBufferPointerPage[2] + (4 * 1024));
            //ptrHS_EndpointQueueHeader[Channel]->dTD.ulBufferPointerPage[4] = (ptrUSB_HW->ptrQueueHeader->dTD.ulBufferPointerPage[3] + (4 * 1024));

            ptrHS_EndpointQueueHeader[Channel]->CurrentdTD_pointer = ptrHS_EndpointQueueHeader[Channel]->my_dTD;
            ptrHS_EndpointQueueHeader[Channel]->dTD.ul_dtToken = (ptrHS_EndpointQueueHeader[Channel]->ulBufferLength | ENDPOINT_QUEUE_HEADER_TOKEN_IOC | ENDPOINT_QUEUE_HEADER_TOKEN_STATUS_ACTIVE);
            ptrHS_EndpointQueueHeader[Channel]->my_dTD->ptrBufferStart = ptrEP0_rx;  // insert software pointer reference to the start of the buffer (this is due to the fact that the first page buffer's offset increments with data reception)

            for (i = 0; i < (ucEndpoints * 2); i++) {                            // initialise the device queue headers
                (ptrHS_EndpointQueueHeader[Channel] + i)->ulCapabilities = ((64 << 16) | ENDPOINT_QUEUE_HEADER_CTL_IOS); // default is maximum 64 byte endpoint size (maximum is 1024 for high-speed USB) - setup tokens generate an interrupt
                (ptrHS_EndpointQueueHeader[Channel] + i)->dTD.ulNextdTD_pointer = ENDPOINT_QUEUE_HEADER_NEXT_INVALID; // pointer is not yet valid
            }
            ptrHSUSB->EPCR0 = (USBHS_EPCR_RXE | USBHS_EPCR_RXR | USBHS_EPCR_TXE | USBHS_EPCR_TXR); // reset data toggle (synchronise) on endpoint 0
    #endif
        }
        ucHS_EndpointCount[Channel] = ucEndpoints;
        ptrHSUSB->ASYNCLISTADDR_EPLISTADDR = (unsigned long)ptrHS_EndpointQueueHeader[Channel]; // set to the asynchronous endpoint list
    }
    hs_usb_hardware[Channel].ptrHSUSB = ptrHSUSB;                        // enter the USB controller's HW base address
    #if defined _iMX
        #if (HSUSB_CONTROLLERS > 1)
    hs_usb_hardware[Channel].ucController = (unsigned char)Channel;      // enter the USB controller channel number
    if (Channel == 0) {
            #if defined USE_BOTH_USB
        fnEnterInterrupt(irq_USB_OTG0_ID, PRIORITY_USB_HS_OTG0, _usb_hs_otg_isr1); //configure and enter the USB handling interrupt routine in the vector table
            #elif !defined USE_USB2_ONLY
        fnEnterInterrupt(irq_USB_OTG0_ID, PRIORITY_USB_HS_OTG0, _usb_hs_otg_isr); //configure and enter the USB handling interrupt routine in the vector table
            #endif
    }
    else {
            #if defined USE_BOTH_USB
        fnEnterInterrupt(irq_USB_OTG1_ID, PRIORITY_USB_HS_OTG1, _usb_hs_otg_isr2); //configure and enter the USB handling interrupt routine in the vector table
            #elif defined USE_USB2_ONLY
        fnEnterInterrupt(irq_USB_OTG1_ID, PRIORITY_USB_HS_OTG1, _usb_hs_otg_isr); //configure and enter the USB handling interrupt routine in the vector table
            #endif
    }
        #else
    fnEnterInterrupt(irq_USB_HS_ID, PRIORITY_USB_HS_OTG0, _usb_hs_otg_isr); //configure and enter the USB handling interrupt routine in the vector table
        #endif
    #else
        #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)
    hs_usb_hardware[Channel].ucController = 1;                           // HS USB is always controller 1 on Kinetis parts when both FS and HS interfaces are in operation together (FS USB is referenced as controller 0)
        #endif
    fnEnterInterrupt(irq_USB_HS_ID, PRIORITY_USB_HS_OTG, _usb_hs_otg_isr); //configure and enter the USB handling interrupt routine in the vector table
    #endif
    if (__USB_HOST_MODE()) {
    #if defined USB_HOST_SUPPORT
        ptrHSUSB->USBINTR = (USBHS_USBINTR_UE | USBHS_USBINTR_UEE | USBHS_USBINTR_PCE | USBHS_USBINTR_AAE | USBHS_USBINTR_SEE | USBHS_USBINTR_TIE0 | USBHS_USBINTR_TIE1); // enable interrupt sources used in host mode
    #endif
    }
    else {
    #if defined USB_DEVICE_SUPPORT
        ptrHSUSB->USBINTR = (USBHS_USBINTR_UE | USBHS_USBINTR_UEE | USBHS_USBINTR_PCE | USBHS_USBINTR_URE); // enable interrupt sources used in device mode
        ptrHSUSB->USBCMD = (/*USBHS_USBCMD_FS_1024 | */USBHS_USBCMD_RS); // set to run mode (in device mode this causes the controller to enable a pull-up on D+ and initiate an attach event)
        ptrHSUSB->EPPRIME |= (USBHS_EPPRIME_PERB0);                      // prime the reception

        #if defined _WINDOWS
        uMemcpy((void *)ptrHS_EndpointQueueHeader[Channel]->CurrentdTD_pointer, &ptrHS_EndpointQueueHeader[Channel]->dTD, (sizeof(ptrHS_EndpointQueueHeader[Channel]->dTD) - sizeof(unsigned long))); // the USBHS controller automatically copied the dTD content to the transfer buffer
        #endif
    #endif
    }
}

// USB Configuration
//
#if defined USB_FS_INTERFACE
    extern void fnConfigHSUSB(QUEUE_HANDLE Channel, USBTABLE *pars)
#else
    extern void fnConfigUSB(QUEUE_HANDLE Channel, USBTABLE *pars)
#endif
{
    #if defined USB_BDT_FIXED
        #if defined _WINDOWS
    static KINETIS_USB_ENDPOINT_BD USB_BDT_RAM[NUMBER_OF_HSUSB_ENDPOINTS * 2];
    static KINETIS_USB_ENDPOINT_BD *__USB_BDT_RAM = USB_BDT_RAM;
        #else
    extern KINETIS_USB_ENDPOINT_BD __USB_BDT_RAM[];
        #endif
    #endif
    #if defined _iMX || defined KINETIS_WITH_USBPHY
    USBPHY *phy;
    #endif
    unsigned char ucEndpoints = (pars->ucEndPoints + 1);                 // endpoint count, including endpoint 0
    hs_usb_hardware[Channel].ucDeviceType = USB_DEVICE_HS;               // mark that the HS controller is being used (rather than FS)
    #if defined USB_HOST_SUPPORT && defined USB_DEVICE_SUPPORT           // both host and device used
    if ((pars->usConfig & USB_HOST_MODE) != 0) {                         // host mode of operation
        hs_usb_hardware[Channel].ucModeType = USB_HOST_MODE;             // mark that we are in host mode
    }
    #elif defined USB_HOST_SUPPORT
    hs_usb_hardware[Channel].ucModeType = USB_HOST_MODE;                 // mark that we are in host mode
    #endif

    #if defined ENABLE_HSUSB_TRANSCEIVER
    ENABLE_HSUSB_TRANSCEIVER();                                          // enable the external HS USB transceiver
    #endif
    #if !defined _iMX
        #if defined MPU_AVAILABLE
    MPU_CESR = 0;                                                        // allow concurrent access to MPU controller
        #endif
    FMC_PFAPR |= FMC_FPAPR_USB_HS;                                       // allow USBHS controller to read from flash
    #endif
    #if defined _iMX || defined KINETIS_WITH_USBPHY                      // device with integrated HS PHY
        #if defined _iMX                                                 // PLL3 (USB1_PLL) has been configured in the clock initialisation and is ready at this point
    POWER_UP_ATOMIC(6, USBOH3_CLOCK);                                    // power up the USB controller module
            #if HSUSB_CONTROLLERS > 1
    if (Channel == 0) {
                #if defined USE_USB2_ONLY
        _EXCEPTION("Disable USE_USB2_ONLY to use HSUSB1");
                #else
        phy = (USBPHY *)USBHS_PHY1_ADD;
        _SET_REGISTER(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_EN_USB_CLKS); // supply 9-phase PLL3 outputs to PHY1
                #endif
    }
    else {
                #if !defined USE_BOTH_USB && !defined USE_USB2_ONLY
        _EXCEPTION("Enable HSUSB support with USE_USB2_ONLY or USE_BOTH_USB");
                #else
        phy = (USBPHY *)USBHS_PHY2_ADD;
        _SET_REGISTER(CCM_ANALOG_PLL_USB2, CCM_ANALOG_PLL_USB2_EN_USB_CLKS); // supply 9-phase PLL7 outputs to PHY2
                #endif
    }
            #else
    phy = (USBPHY *)USBHS_PHY1_ADD;
    _SET_REGISTER(CCM_ANALOG_PLL_USB1, CCM_ANALOG_PLL_USB1_EN_USB_CLKS); // supply 9-phase PLL3 outputs to PHY1
            #endif
    phy->USBPHY_CTRL = (USBPHY_CTRL_CLKGATE | USBPHY_CTRL_ENDPDMCHG_WKUP); // release PHY from reset (clock still gated)
    phy->USBPHY_CTRL = (USBPHY_CTRL_ENDPDMCHG_WKUP);                     // apply phase clocks by ungating the USB clock
    phy->USBPHY_PWD = 0;
    phy->USBPHY_CTRL = (USBPHY_CTRL_ENDPDMCHG_WKUP | USBPHY_CTRL_ENAUTOCLR_PHY_PWD | USBPHY_CTRL_ENAUTOCLR_CLKGATE | USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3); // support low speed device and external FS hub with low speed device connected
            #if HSUSB_CONTROLLERS > 1
    if (Channel == 0) {
                #if !defined USE_USB2_ONLY
        PMU_REG_3P0 = (PMU_REG_3P0_OUTPUT_TRG_3V200 | PMU_REG_3P0_BO_OFFSET_175mV | PMU_REG_3P0_ENABLE_ILIMIT | PMU_REG_3P0_ENABLE_LINREG | PMU_REG_3P0_VBUS_SEL_1); // enable current limited LDO output at 3.2V from VBUS1 input
                #endif
    }
    else {
                #if defined USE_BOTH_USB || defined USE_USB2_ONLY
                #endif
                #if !defined USE_BOTH_USB && defined USE_USB2_ONLY
        PMU_REG_3P0 = (PMU_REG_3P0_OUTPUT_TRG_3V200 | PMU_REG_3P0_BO_OFFSET_175mV | PMU_REG_3P0_ENABLE_ILIMIT | PMU_REG_3P0_ENABLE_LINREG | PMU_REG_3P0_VBUS_SEL_2); // enable current limited LDO output at 3.2V from VBUS2 input
                #endif
    }
            #else
    USBHS0_USBCMD = (USBHS_USBCMD_RST);                                  // reset the HS USB controller
    fnDelayLoop(50);                                                     // delay to ensure that there is a D+ pull-up sequence
    PMU_REG_3P0 = (PMU_REG_3P0_OUTPUT_TRG_3V200 | PMU_REG_3P0_BO_OFFSET_175mV | PMU_REG_3P0_ENABLE_ILIMIT | PMU_REG_3P0_ENABLE_LINREG | PMU_REG_3P0_VBUS_SEL_1); // enable current limited LDO output at 3.2V from VBUS1 input
            #endif
    USB_ANALOG_USB1_CHRG_DETECT = (USB_ANALOG_USB1_CHRG_DETECT_EN_B | USB_ANALOG_USB1_CHRG_DETECT_CHK_CHRG_B); // disable charger detection
        #else
    phy = (USBPHY *)USBHS_PHY_ADD;
    POWER_UP_ATOMIC(3, USBHS);                                           // power up the USB HS controller module
    // Requirements for operation are:
    // - VREGIN0 or VREGIN1 connected to 5V so that 3.3V USB is valid
    // - 32kHz slow clock is enabled
    // - external reference clock is enabled and is 12MHz, 16MHz or 24MHz
    // 
    MCG_C1 |= MCG_C1_IRCLKEN;                                            // 32kHz IRC enable
    OSC0_CR |= OSC_CR_ERCLKEN;                                           // external reference clock enable
    SIM_SOPT2 |= SIM_SOPT2_USBREGEN;                                     // enable USB PHY PLL regulator
    POWER_UP_ATOMIC(3, USBHSPHY);                                        // enable clocks to PHY
    // Note: NXP adds a delay here (in host mode)
    // - to monitor
    SIM_USBPHYCTL = (SIM_USBPHYCTL_USBVOUTTRG_3_310V | SIM_USBPHYCTL_USBVREGSEL); // 3.310V source VREG_IN1 (in case both are powered)
    phy->USBPHY_TRIM_OVERRIDE_EN = 0x0000001f;                           // override IFR values
    phy->USBPHY_CTRL = (USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3); // release PHY from reset and enable its clock
            #if _EXTERNAL_CLOCK == 24000000
    phy->USBPHY_PLL_SIC = (USBPHY_PLL_SIC_PLL_POWER | USBPHY_PLL_SIC_PLL_ENABLE | USBPHY_PLL_SIC_PLL_BYPASS | USBPHY_PLL_SIC_PLL_DIV_SEL_24MHz); // power up PLL to run at 480MHz from 24MHz clock input
            #elif _EXTERNAL_CLOCK == 16000000
    phy->USBPHY_PLL_SIC = (USBPHY_PLL_SIC_PLL_POWER | USBPHY_PLL_SIC_PLL_ENABLE | USBPHY_PLL_SIC_PLL_BYPASS | USBPHY_PLL_SIC_PLL_DIV_SEL_16MHz); // power up PLL to run at 480MHz from 16MHz clock input
            #elif _EXTERNAL_CLOCK == 12000000
    phy->USBPHY_PLL_SIC = (USBPHY_PLL_SIC_PLL_POWER | USBPHY_PLL_SIC_PLL_ENABLE | USBPHY_PLL_SIC_PLL_BYPASS | USBPHY_PLL_SIC_PLL_DIV_SEL_12MHz); // {4} power up PLL to run at 480MHz from 12MHz clock input
            #else
            #error "USB PLL requires an external reference of 12MHz, 16MHz or 24MHz!"
            #endif
            #if defined ERRATA_ID_9712 && defined EXTERNAL_CLOCK
    // Error 9712 workaround is being enabled - advise use of crystal rather than external clock!
    //
    MCG_C2 |= MCG_C2_EREFS;                                              // pretend that crystal is being used so that the PLL will lock
            #endif
    phy->USBPHY_PLL_SIC &= ~(USBPHY_PLL_SIC_PLL_BYPASS);                 // clear the bypass
    phy->USBPHY_PLL_SIC |= (USBPHY_PLL_SIC_PLL_EN_USB_CLKS);             // enable USB clock output from PHY PLL
    _WAIT_REGISTER_FALSE(phy->USBPHY_PLL_SIC, USBPHY_PLL_SIC_PLL_LOCK);  // wait for the PLL to lock
            #if defined ERRATA_ID_9712 && defined EXTERNAL_CLOCK
    // Error 9712 workaround is being enabled - advise use of crystal rather than external clock!
    //
    //MCG_C2 &= ~MCG_C2_EREFS;                                           // remove the external reference from oscillator requested flag
            #endif
    phy->USBPHY_PWD = 0;                                                 // for normal operation
        #endif
    if (__USB_HOST_MODE()) {
        #if defined USB_HOST_SUPPORT
        phy->USBPHY_TX = ((phy->USBPHY_TX & ~USBPHY_TX_DCAL_VALUE) | 0x0c); // trim the nominal 17.78mA current source for the high speed drivers (USB_DP and USB_DM) (taken from NXP reference)
        #endif
    }
    else {
        #if defined _KINETIS
        phy->USBPHY_ANACTRL = ((24 << 4) | 4);                           // frac = 24 and  Clk /4
        _WAIT_REGISTER_FALSE(phy->USBPHY_ANACTRL, USBPHY_ANACTRL_PFD_STABLE);
        phy->USBPHY_TX |= (1 << 24);                                     // reserved (taken from NXP reference)
        #endif
    }
    #else                                                                // kinetis HSUSB requiring external PHY
    POWER_UP_ATOMIC(6, USBHS);                                           // power up the USB HS controller module
    _CONFIG_PERIPHERAL(A, 7,  PA_7_ULPI_DIR);                            // ULPI_DIR on PA.7    (alt. function 2)
    _CONFIG_PERIPHERAL(A, 8,  PA_8_ULPI_NXT);                            // ULPI_NXT on PA.8    (alt. function 2)
    _CONFIG_PERIPHERAL(A, 10, PA_10_ULPI_DATA0);                         // ULPI_DATA0 on PA.10 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 11, PA_11_ULPI_DATA1);                         // ULPI_DATA1 on PA.11 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 24, PA_24_ULPI_DATA2);                         // ULPI_DATA2 on PA.24 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 25, PA_25_ULPI_DATA3);                         // ULPI_DATA3 on PA.25 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 26, PA_26_ULPI_DATA4);                         // ULPI_DATA4 on PA.26 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 27, PA_27_ULPI_DATA5);                         // ULPI_DATA5 on PA.27 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 28, PA_28_ULPI_DATA6);                         // ULPI_DATA6 on PA.28 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 29, PA_29_ULPI_DATA7);                         // ULPI_DATA7 on PA.29 (alt. function 2)
    _CONFIG_PERIPHERAL(A, 6,  PA_6_ULPI_CLK);                            // ULPI_CLK on PA.6    (alt. function 2)
    _CONFIG_PERIPHERAL(A, 9,  PA_9_ULPI_STP);                            // ULPI_STP on PA.9    (alt. function 2)
    #endif

    #if defined USB_DEVICE_SUPPORT
        #if defined USB_HOST_SUPPORT                                     // both host and device used
    if ((pars->usConfig & USB_HOST_MODE) == 0) {                         // device mode of operation
        #endif
        if (ucEndpoints > NUMBER_OF_USBHS_ENDPOINTS) {                   // limit endpoint count
            ucEndpoints = NUMBER_OF_USBHS_ENDPOINTS;                     // limit to maximum available in device
        }
        #if defined USB_HOST_SUPPORT                                     // both host and device used
    }
        #endif
    #endif

    hs_usb_endpoints[Channel] = uMalloc((MAX_MALLOC)(sizeof(USB_END_POINT) * ucEndpoints)); // get endpoint control structures
    fnUSBHS_init(Channel, ucEndpoints);
}
#endif
