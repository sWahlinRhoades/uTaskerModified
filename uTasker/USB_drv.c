/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      USB_drv.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    06.10.2008 Adjust USB read/writes to support FIFO operation          {1}
    17.10.2008 Add control endpoint direction control and setup clear    {2}
    25.12.2008 Additional tx buffer depth check (no longer optional)     {3}
    04.01.2009 Additional setup clears added                             {4}
    10.01.2009 Correct tx buffer depth check                             {5}
    10.01.2009 Break from loop rather than individual return values      {6}
    29.01.2009 Clear endpoint FIFO depth on disconnect                   {7}
    07.03.2009 Remove unnecessary usEndPoints                            {8}
    12.05.2009 Optionally don't sent zero-data frame on non-control endpoints {9}
    02.02.2010 Sum outstanding data when multiple buffers                {10}
    21.03.2010 Allow control of whether an endpoint sends zero-date frame{11}
    21.03.2010 Allow receptions to be handled by both a callback and a buffer {12}
    21.03.2010 Add USB_REQUEST_CLEAR_FEATURE support                     {13}
    21.03.2010 Modify the routine fnSetUSBEndpointState() to only set state bits {14}
    22.03.2010 Extract indexed data from FIFO, when FIFO used            {15}
    02.04.2010 Add VALIDATE_NEW_CONFIGURATION() and fnGetPairedIN()      {16}
    18.04.2010 Add USB DMA support                                       {17}
    20.06.2010 Add CRITICAL_OUT for FIFO endpoints which have to consume data {18}
    13.07.2010 Add intermediate FIFO support                             {19}
    14.02.2011 Configure usb_endpoint_control before configuring hardware to avoid possibility of reset interrupt trying to use it too soon {20}
    15.02.2012 Correct queuing for devices with more than double-buffer depth {21} (this correction isn't important for devices previously supported up to its introduction)
    15.02.2012 Add MULTIPLE_TX option for devices that can independently handle multiple IN packets {22}
    04.06.2013 Added USB_DRV_MALLOC() default                            {23}
    06.06.2013 Remove device qualifier descriptor when USB1.1 is used    {24}
    14.08.2013 Add option for high speed devices than control multiple IN packets {25}
    14.01.2015 Add capability to share IN/OUT endpoints (USB_SIMPLEX_ENDPOINTS)
    03.03.2015 Only send zero frame to acknowledgement of block sizes equal to the endpoint size when it is a control or terminating endpoint {26}
    08.06.2015 Add optional IN_COMPLETE_CALLBACK to callback on successful IN frame (useful for interrupt endpoints to known when the last host poll has completed) {27}
    04.10.2015 Change fnGetUSB_HW() parameter from pointer to pointer to pointer {28}
    21.10.2015 Supports device or host or both host and device           {29}
    02.12.2015 Add prtUSBcontrolSetup to allow automating zero termination on data sent by application handling class setup IN frames {30}
    23.12.2015 Add zero copy operation                                   {31}
    27.01.2016 Add USB tx message mode operation                         {32}
    13.04.2016 Change parameter of fnGetUSB_string_entry() to unsigned char {33}
    12.04.2017 In host mode reset previous string reception length counter when reception completes with a zero length frame {34}
    12.04.2017 Use USB_DEVICE_TIMEOUT in host mode to repeat a get descriptor request {35}
    24.07.2019 Host string request descriptor made static so that it remains stable when used directly by USB buffer descriptors {36}
    13.08.2019 Add USB_HOST_CALLBACK_SETUP_ACK option (used by RNDIS host mode) {37}
    31.03.2020 Add multiple-USB controller support
    06.06.2020 Add fnDisplayEndpoint()
    12.06.2020 Add host option to filter device interfaces to be supported {38}
    30.06.2020 Add fnShowEndpoint
    31.07.2020 Add option to send custom TX_FREE interrupt event         {39}

*/

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"

#if defined USB_INTERFACE

//#define _USB_CATCH_ERROR                                               // debug define to catch FIFO errors 
//#define SET_INTERFACE

#if defined _USB_CATCH_ERROR
static void fnError(int iErrorNumber)
{
    FOREVER_LOOP() {}
}
#endif

#if !defined USBS_AVAILABLE
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE)           // kinetis parts with FS and HS, and both in use
        #define USBS_AVAILABLE     2
    #else
        #define USBS_AVAILABLE     1
    #endif
#endif

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined USB_DRV_MALLOC                                              // {23}
    #define USB_DRV_MALLOC(x)      uMalloc((MAX_MALLOC)(x))
#endif


#define DIRECTION_OUT() ((ucFirstByte & STANDARD_DEVICE_TO_HOST) == 0)
#define DIRECTION_IN()  ((ucFirstByte & STANDARD_DEVICE_TO_HOST) != 0)

#if defined USB_FIFO && !defined _WINDOWS                                // {1}
    #define GET_USB_DATA()         (unsigned char)(*(volatile unsigned char *)ptrData) // get from a fifo
    #define GET_USB_DATA_NO_INC()  (unsigned char)(*(volatile unsigned char *)ptrData)
#else
    #define GET_USB_DATA()         *ptrData++                            // get from a linear buffer
    #define GET_USB_DATA_NO_INC()  *ptrData
#endif

#if defined USB_HOST_SUPPORT                                             // {29}
    #define USB_DEVICE_ADDRESS       1                                   // single USB device with fixed address (allocated when host)

    #define _DEVICE_DATA             0
    #define _HOST_DATA               1
    #define _DEVICE_HOST_DATA        2                                   // data (not setup) in device or host mode
    #define _DEVICE_DATA_HOST_SETUP  3                                   // either device/host data or host setup
    #define _HOST_SETUP              4                                   // definitely a setup in host mode

    #define DEVICE_DATA             _DEVICE_DATA,
    #define HOST_DATA               _HOST_DATA,
    #define DEVICE_HOST_DATA        _DEVICE_HOST_DATA,
    #define DEVICE_DATA_HOST_SETUP  _DEVICE_DATA_HOST_SETUP,
    #define HOST_SETUP              _HOST_SETUP,


    #define HOST_ENUMERATION_IDLE                            0           // host state-event-machine states
    #define HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR      1
    #define HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR_ACK  2
    #define HOST_ENUMERATION_SET_ADDRESS                     3
    #define HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR        4
    #define HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR_ACK    5
    #define HOST_ENUMERATION_REQUEST_STRING                  6
    #define HOST_ENUMERATION_REQUEST_STRING_ACK              7
    #define HOST_ENUMERATION_SET_CONFIGURATION               8
    #define HOST_ENUMERATION_SET_CONFIGURATION_ACK           9
    #define HOST_ENUMERATION_SET_INTERFACE_ACK               10
    #define HOST_ENUMERATION_CONFIGURED                      11
    #define HOST_ENUMERATION_CONFIGURED_ACK                  12
    #define HOST_ENUMERATION_CONFIGURED_IDLE                 13

    #if !defined MAX_CONFIGURATION_DESCRIPTOR_LENGTH
        #define MAX_CONFIGURATION_DESCRIPTOR_LENGTH     255
    #endif


    #define SUCCESSFUL_TERMINATION   0                                   // host state-event-machine events
    #define ZERO_DATA_TERMINATOR     1
    #define DEVICE_DETECTED          2
    #define DEVICE_TIMEOUT           3
    #define DATA_RECEPTION_COMPLETE  4
    #define HOST_APPLICATION_ON      5
    #define HOST_APPLICATION_OFF     6
#else                                                                    // not used in device only mode
    #define DEVICE_DATA
    #define HOST_DATA
    #define DEVICE_HOST_DATA
    #define DEVICE_DATA_HOST_SETUP
    #define DATA_HOST_SETUP

    #define USB_DEVICE_SUPPORT                                           // enable device only support if host support hasn't specifically defined
#endif

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */


/* =================================================================== */
/*                             constants                               */
/* =================================================================== */
#if defined USB_DEVICE_SUPPORT
    static const unsigned short usInterfaceStatus = 0x0000;              // the interface status is presently reserved in the USB specification so a fixed value of zeros can be used
    static const unsigned short usEndpointStalled = LITTLE_SHORT_WORD(ENDPOINT_STATUS_STALLED);
    #define usEndpointOK usInterfaceStatus                               // use the same zero status
#endif


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static USB_ENDPOINT  *usb_endpoint_control[USBS_AVAILABLE] = {0};        // pointer to a list of endpoint control structures
static unsigned char  ucActiveConfiguration[USBS_AVAILABLE] = {0};       // present active configuration
//static unsigned char  usEndPoints = 0;                                 // {8} the number of endpoints configured
#if defined USB_DEVICE_SUPPORT
    static unsigned short usDeviceStatus[USBS_AVAILABLE] = {0};          // state of device (SELF_POWERED_BIT, REMOTE_WAKEUP_BIT)
    static USB_SETUP_HEADER *prtUSBcontrolSetup[USBS_AVAILABLE] = {0};   // {30}
#endif
#if defined USB_HOST_SUPPORT                                             // {29}
    static unsigned short usRequestingReceived[USBS_AVAILABLE] = {0};
    static unsigned short usRequestLengthRemaining[USBS_AVAILABLE] = {0};
    static unsigned short usConfigDescriptorLength[USBS_AVAILABLE] = {0};
    static USB_DEVICE_DESCRIPTOR device_descriptor[USBS_AVAILABLE] = {{0}}; // device descriptor of connected device
    static unsigned char ucRequestType[USBS_AVAILABLE] = {HOST_ENUMERATION_IDLE};
    static unsigned char ucStalledEndpoint[USBS_AVAILABLE] = {0};
    static unsigned char config_descriptor[USBS_AVAILABLE][MAX_CONFIGURATION_DESCRIPTOR_LENGTH] = {0};
    #if defined USB_STRING_OPTION
        static unsigned char ucStringList[USB_MAX_STRINGS + 1][(USB_MAX_STRING_LENGTH + 1) * 2] = {{0}}; // uni-code string space (first is language ID)
        static int iStringReference[USBS_AVAILABLE] = {0};
    #endif
#endif

/* =================================================================== */
/*                      local function definitions                     */
/* =================================================================== */

#if defined USB_HOST_SUPPORT                                             // {29}
    static QUEUE_TRANSFER fnPrepareOutData(int iMode, unsigned char *ptrData, unsigned short usLength, unsigned short ucMaxLength, int iEndpoint, USB_HW *ptrUSB_HW);
#else
    static QUEUE_TRANSFER fnPrepareOutData(unsigned char *ptrData, unsigned short usLength, unsigned short ucMaxLength, int iEndpoint, USB_HW *ptrUSB_HW);
#endif
#if defined USB_DEVICE_SUPPORT                                           // {29}
    static void *fnSetUSBInterfaceState(int iCommand, unsigned char ucInterface, unsigned char ucAlternativeInterface, int iChannel);
          #define USB_INTERFACE_DEACTIVATE 0
          #define USB_INTERFACE_ACTIVATE   1
          #define USB_INTERFACE_ALTERNATE  2
#endif

#if defined USB_HOST_SUPPORT                                             // {29}
    static int fnHostEnumeration(int iEndpoint, int iEvent, USB_HW *ptrUSB_HW);
#endif

/* =================================================================== */
/*                      global function definitions                    */
/* =================================================================== */


// Standard entry call to driver - dispatches required sub-routine
//
extern QUEUE_TRANSFER entry_usb(QUEUE_HANDLE channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID)
{
    QUEUE_TRANSFER rtn_val = 0;
    USBQUE *ptrUsbQueue;

    uDisable_Interrupt();                                                // disable all interrupts

    switch (ucCallType) {
    case CALL_DRIVER:                                                    // request changes and return status
        if ((TX_ON & Counter) != 0) {                                    // enable a configuration
#if defined USB_HOST_SUPPORT
            USB_HW temp_usb_hardware;
            USB_HW *ptr_usb_hardware = &temp_usb_hardware;
            fnGetUSB_HW(channel, &ptr_usb_hardware);                     // get a pointer to the hardware details
    #if defined USB_DEVICE_SUPPORT
            if ((ptr_usb_hardware->ucModeType & USB_HOST_MODE) != 0) {
    #endif
                ucActiveConfiguration[UBS_HANDLE_TO_CHANNEL(channel)] = (unsigned char)(CAST_POINTER_ARITHMETIC)ptBuffer; // set the configuration number that will be activated
                if (fnHostEnumeration(0, HOST_APPLICATION_ON, ptr_usb_hardware) == USB_HOST_ERROR) {
                    ucActiveConfiguration[UBS_HANDLE_TO_CHANNEL(channel)] = 0;
                }
    #if defined USB_DEVICE_SUPPORT
                break;
            }
    #endif
#endif
#if defined USB_DEVICE_SUPPORT
            ptrUsbQueue = (USBQUE *)que_ids[DriverID].output_buffer_control; // set to output control block
            // Enable continuous isochronous transmission
            //
            if ((ptrUsbQueue->endpoint_control->ucState & TX_ACTIVE) == 0) { // if transmission is already in progress don't initiate any more activity
                ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.QUEbuffer;
                ptrUsbQueue->USB_queue.chars = ptrUsbQueue->USB_queue.buf_length;
                rtn_val = fnStartUSB_send(channel, ptrUsbQueue, ptrUsbQueue->USB_queue.chars);
            }
#endif
            break;
        }
        else if ((TX_OFF & Counter) != 0) {                              // disable a configuration
#if defined USB_HOST_SUPPORT
            USB_HW temp_usb_hardware;
            USB_HW *ptr_usb_hardware = &temp_usb_hardware;
            fnGetUSB_HW(channel, &ptr_usb_hardware);                     // get a pointer to the hardware details
    #if defined USB_DEVICE_SUPPORT
            if ((ptr_usb_hardware->ucModeType & USB_HOST_MODE) != 0) {
    #endif
                ucActiveConfiguration[UBS_HANDLE_TO_CHANNEL(channel)] = 0;
                fnHostEnumeration(0, HOST_APPLICATION_OFF, 0);
    #if defined USB_DEVICE_SUPPORT
                break;
            }
    #endif
#endif
#if defined USB_DEVICE_SUPPORT
            // Disable continuous isochronous transmission
            //
            ptrUsbQueue = (USBQUE *)que_ids[DriverID].output_buffer_control; // set to output control block
            ptrUsbQueue->endpoint_control->ucState &= ~(TX_ACTIVE);
#endif
            break;
        }
#if defined USB_HOST_SUPPORT                                             // {29}
        if ((RX_ON & Counter) != 0) {                                    // enable host IN polling on this endpoint
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control); // set to input control block
            uEnable_Interrupt();                                         // don't block interrupts when starting IN poll
                fnHostEndpoint(ptrUsbQueue->endpoint_control->ucEndpointNumber, IN_POLLING, 1);
            uDisable_Interrupt();
        }
        else if ((RX_OFF & Counter) != 0) {                              // disable host IN polling on this endpoint
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control); // set to input control block
            uEnable_Interrupt();                                         // don't block interrupts when stopping IN poll
                fnHostEndpoint(ptrUsbQueue->endpoint_control->ucEndpointNumber, IN_POLLING, 0);
            uDisable_Interrupt();
        }
#endif
        if (((CAST_POINTER_ARITHMETIC)ptBuffer & MODIFY_TX) != 0) {
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].output_buffer_control); // set to output control block
        }
        else {
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control); // set to input control block
        }
        if ((Counter & MODIFY_WAKEUP) != 0) {
            if ((ptrUsbQueue->endpoint_control->ucState & USB_ENDPOINT_ACTIVE) != 0) { // don't allow changes when endpoint is not active
#if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
                ptrUsbQueue->endpoint_control->event_task_in = (UTASK_TASK)((CAST_POINTER_ARITHMETIC)ptBuffer & 0x7f);
#else
                ptrUsbQueue->endpoint_control->event_task = (UTASK_TASK)((CAST_POINTER_ARITHMETIC)ptBuffer & 0x7f);
#endif
#if defined CUSTOM_USB_TX_FREE
                ptrUsbQueue->endpoint_control->ucCustomTxFree_event = (unsigned char)(((CAST_POINTER_ARITHMETIC)ptBuffer >> 8) & 0xf);
#endif
            }
        }
        rtn_val = ptrUsbQueue->endpoint_control->ucState;                // return the present state
        break;

    case CALL_INPUT:                                                     // request the number or input characters waiting
        ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control);// set to input control block
        rtn_val = ptrUsbQueue->USB_queue.chars;
        break;

    case CALL_WRITE:                                                     // write data into the output queue
                                                                         // copy the data to the output buffer and start transmission if not already done
        ptrUsbQueue = (USBQUE *)que_ids[DriverID].output_buffer_control; // set to output control block
        if (ptrUsbQueue == 0) {                                          // no output buffer so use direct method
            USB_HW temp_usb_hardware;                                    // temporary copy of the hardware information
            USB_HW *ptr_usb_hardware = &temp_usb_hardware;               // {28}
            if (fnGetUSB_HW(channel, &ptr_usb_hardware) == ENDPOINT_FREE) { // if hardware is ready and buffers not full (the routine fills out needed hardware information)
                unsigned short usMaxLength = Counter;
#if defined USB_DEVICE_SUPPORT
    #if defined USB_HOST_SUPPORT
                if ((ptr_usb_hardware->ucModeType & USB_HOST_MODE) == 0) {
    #endif
                    if ((UBS_HANDLE_TO_ENDPOINT(channel) == 0) && (prtUSBcontrolSetup[UBS_HANDLE_TO_CHANNEL(channel)] != 0)) { // {30} if device control endpoint 0 is sending setup IN data
                        usMaxLength = (prtUSBcontrolSetup[UBS_HANDLE_TO_CHANNEL(channel)]->wLength[0] | (prtUSBcontrolSetup[UBS_HANDLE_TO_CHANNEL(channel)]->wLength[1] << 8)); // pass the limit so that zero termination can be handled appropriately
                    }
    #if defined USB_HOST_SUPPORT
                }
    #endif
#endif
                rtn_val = fnPrepareOutData(DEVICE_DATA_HOST_SETUP ptBuffer, Counter, usMaxLength, UBS_HANDLE_TO_ENDPOINT(channel), ptr_usb_hardware);
            }
            else {
                fnDebugMsg("Not Free!\r\n");
            }
            break;
        }
        if ((ptrUsbQueue->endpoint_control->ucState & USB_ENDPOINT_ACTIVE) == 0) { // only allow the copy if the endpoint is active
            break;
        }
        if (ptBuffer == 0) {                                             // the caller wants to see whether the data will fit and not copy data so inform
#if defined USB_TX_MESSAGE_MODE
            if (ptrUsbQueue->endpoint_control->messageQueue != 0) {      // {32}
                Counter += (ptrUsbQueue->endpoint_control->usMax_frame_length - 1); // assume worst case where a single byte will be located in the final frame buffer
                if ((ptrUsbQueue->USB_queue.buf_length - ptrUsbQueue->USB_queue.chars) >= Counter) { // if there is space for the message
                    if ((ptrUsbQueue->USB_queue.chars != 0) && (ptrUsbQueue->endpoint_control->messageQueue->ucInIndex == ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex)) { // if the fifo is full
                        break;
                    }
                    else {
                        rtn_val = (ptrUsbQueue->USB_queue.buf_length - ptrUsbQueue->USB_queue.chars - (ptrUsbQueue->endpoint_control->usMax_frame_length - 1)); // the remaining space
                    }
                }
            }
            else {
#endif
                if ((ptrUsbQueue->USB_queue.buf_length - ptrUsbQueue->USB_queue.chars) >= Counter) {
                    rtn_val = (ptrUsbQueue->USB_queue.buf_length - ptrUsbQueue->USB_queue.chars); // the remaining space
#if defined HS_USB_AVAILABLE
                    if ((ptrUsbQueue->endpoint_control->usParameters & USB_ENDPOINT_ZERO_COPY_ISO_IN) != 0) {
                        fnStartUSB_send(channel, ptrUsbQueue, Counter);  // start sending alternating half-buffer message content
                    }
#endif
                }
#if defined USB_TX_MESSAGE_MODE
            }
#endif
        }
        else {                                                           // buffered mode
#if defined USB_TX_MESSAGE_MODE
            if (ptrUsbQueue->endpoint_control->messageQueue != 0) {      // {32}
                unsigned char *ptrTo = ptrUsbQueue->USB_queue.put;
                QUEUE_TRANSFER remainingSpace = (ptrUsbQueue->USB_queue.buffer_end - ptrTo); // remaining space to the end of the buffer
                if (Counter <= remainingSpace) {                         // if the message fits into the remaining buffer space
                    remainingSpace -= Counter;                           // remaining space after the copy
                    if ((remainingSpace != 0) && (remainingSpace < ptrUsbQueue->endpoint_control->usMax_frame_length)) { // if the remaining length will be less that the endpont length
                        ptrUsbQueue->USB_queue.put = ptrUsbQueue->USB_queue.QUEbuffer; // next message will start at the beginning of the buffer (some space at the end fo the buffer will not be used)
                    }
                    else {
                        ptrUsbQueue->USB_queue.put += Counter;           // following message will be located here
                    }
                    ptrUsbQueue->USB_queue.chars += Counter;             // new total bytes waiting to be sent
                    ptrUsbQueue->endpoint_control->messageQueue->usLength[ptrUsbQueue->endpoint_control->messageQueue->ucInIndex++] = Counter; // set the length to the length fifo
                    if (ptrUsbQueue->endpoint_control->messageQueue->ucInIndex >= ptrUsbQueue->endpoint_control->messageQueue->ucQuantity) { // handle length fifo overflow
                        ptrUsbQueue->endpoint_control->messageQueue->ucInIndex = 0;
                    }
                    uEnable_Interrupt();                                 // allow interrupts during copy to the buffer since the critical counters and pointers have been prepared for the state after the copy can completed
                    uMemcpy(ptrTo, ptBuffer, Counter);                   // copy data to reserved area without blocked interrupts
                    uDisable_Interrupt();                                // disable interrupts again for compatibility
                }
                else {                                                   // wrap results
                    int iSpace = (remainingSpace % ptrUsbQueue->endpoint_control->usMax_frame_length); // the space at the end of the buffer that must be left unused
                    int iRemainingCopy;
                    remainingSpace -= iSpace;                            // the length of the first block (divisible by the endpoint length)
                    iRemainingCopy = (Counter - remainingSpace);
                    ptrUsbQueue->USB_queue.put = (ptrUsbQueue->USB_queue.QUEbuffer + iRemainingCopy);
                    ptrUsbQueue->endpoint_control->messageQueue->usLength[ptrUsbQueue->endpoint_control->messageQueue->ucInIndex++] = Counter; // set the length to the length fifo
                    if (ptrUsbQueue->endpoint_control->messageQueue->ucInIndex >= ptrUsbQueue->endpoint_control->messageQueue->ucQuantity) { // handle length fifo overflow
                        ptrUsbQueue->endpoint_control->messageQueue->ucInIndex = 0;
                    }
                    ptrUsbQueue->USB_queue.chars += Counter;             // new total bytes waiting to be sent
                    uEnable_Interrupt();                                 // allow interrupts
                    uMemcpy(ptrTo, ptBuffer, remainingSpace);            // copy data to reserved area without blocked interrupts
                    ptBuffer += remainingSpace;
                    uMemcpy(ptrUsbQueue->USB_queue.QUEbuffer, ptBuffer, iRemainingCopy); // complete copy of second part of the block
                    uDisable_Interrupt();
                }
                rtn_val = Counter;                                       // message size to be sent
            }
            else {
#endif
                uEnable_Interrupt();                                     // fnFillBuffer disables and then re-enables interrupts - be sure we are compatible
                    rtn_val = fnFillBuf(&ptrUsbQueue->USB_queue, ptBuffer, Counter); // copy the input data to the output circular buffer
                uDisable_Interrupt();
#if defined USB_TX_MESSAGE_MODE
            }
#endif
            if ((ptrUsbQueue->endpoint_control->ucState & TX_ACTIVE) == 0) { // if transmission is already in progress don't initiate any more activity
                rtn_val = fnStartUSB_send(channel, ptrUsbQueue, rtn_val);// start sending message content
            }
        }
        break;

    case CALL_READ:                                                      // read data from the queue
        ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control);// set to input control block
        rtn_val = fnGetBuf(&ptrUsbQueue->USB_queue, ptBuffer, Counter);  // interrupts are re-enabled as soon as no longer critical
        if ((ptrUsbQueue->endpoint_control->ucState & USB_ENDPOINT_BLOCKED) != 0) { // the buffer has been previously blocked due to lack of space
            uDisable_Interrupt();
                ptrUsbQueue->endpoint_control->ucState &= ~USB_ENDPOINT_BLOCKED; // remove the blocked state - it will be set again if the data in the input USB buffer still can't be put to the buffer
                while (fnConsumeUSB_out(ptrUsbQueue->endpoint_control->ucEndpointNumber) == USB_BUFFER_FREED) {} // copy data from waiting buffer and free buffer for further use
            uEnable_Interrupt();
        }
        return rtn_val;                                                  // the amount of data returned

#if defined SUPPORT_FLUSH
    case CALL_FLUSH:                                                     // flush input or output queue completely
        if (Counter != FLUSH_RX) {                                       // tx
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].output_buffer_control); // set to output control block
        }
        else {                                                           // rx
            ptrUsbQueue = (USBQUE *)(que_ids[DriverID].input_buffer_control); // set to input control block
        }
        ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.put = ptrUsbQueue->USB_queue.QUEbuffer;
        ptrUsbQueue->USB_queue.chars = 0;
        break;
#endif

    default:
       break;
    }
    uEnable_Interrupt();                                                 // enable interrupts
    return (rtn_val);
}

extern QUEUE_TRANSFER fnStartUSB_send(QUEUE_HANDLE channel, USBQUE *ptrUsbQueue, QUEUE_TRANSFER txLength)
{
#if defined USB_DMA_TX && defined USB_RAM_START
    ptrUsbQueue->endpoint_control->ucState |= TX_ACTIVE;                 // mark that the transmitter is active
    (usb_endpoint_control[0] + channel)->usCompleteMessage = (usb_endpoint_control[0] + channel)->usSent = txLength = fnPrepareUSBOutData(ptrUsbQueue, txLength, channel, ptr_usb_hardware);
    return txLength;
#else
    USB_HW temp_usb_hardware;                                            // temporary copy of the present hardware information
    USB_HW *ptr_usb_hardware = &temp_usb_hardware;                       // {28}
    if (fnGetUSB_HW(channel, &ptr_usb_hardware) == ENDPOINT_FREE) {      // check that there is a free buffer to send with
        unsigned short usLength;
        ptrUsbQueue->endpoint_control->ucState |= TX_ACTIVE;             // mark that the transmitter is active
    #if defined USB_TX_MESSAGE_MODE
        if (ptrUsbQueue->endpoint_control->messageQueue != 0) {          // {32}
            usLength = txLength;                                         // in message mode we always handle each new transmission as a single message, even if it involves circular buffer operation
        }
        else {
    #endif
            usLength = (ptrUsbQueue->USB_queue.buffer_end - ptrUsbQueue->USB_queue.get); // maximum linear part of buffer
            if (txLength < usLength) {                                   // if the requested transmission length is less that the maximum possible linear length
                usLength = txLength;                                     // set requested length
            }
    #if defined USB_TX_MESSAGE_MODE
        }
    #endif
        return (fnPrepareOutData(DEVICE_HOST_DATA ptrUsbQueue->USB_queue.get, usLength, usLength, UBS_HANDLE_TO_ENDPOINT(channel), ptr_usb_hardware));
    }
    else {                                                               // the transmitter is not busy but it was not possible to send the data
        return (0);                                                      // signal that there was a problem that may need to be handled
    }
#endif
}

// fnOpen() calls this for the USB interface
//
extern QUEUE_HANDLE fnOpenUSB(USBTABLE *pars, unsigned char driver_mode)
{
    register int iController = 0;
    QUEUE_HANDLE DriverID;
    QUEUE_TRANSFER (*entry_add)(QUEUE_HANDLE channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID) = entry_usb;
    USB_ENDPOINT *usb_endpoint_queue;
    #if defined USB_HOST_SUPPORT && defined USB_DEVICE_SUPPORT
        #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    unsigned char ucControllerEndpoint;
    iController = pars->ucUSB_controller;
    ucControllerEndpoint = USB_CONTROLLER_ENDPOINT(pars->Endpoint, iController);
    usb_endpoint_queue = usb_endpoint_control[iController];
        #else
    unsigned char ucControllerEndpoint = pars->Endpoint;
    usb_endpoint_queue = usb_endpoint_control[iController];
        #endif
    if ((DriverID = fnSearchID(entry_add, ucControllerEndpoint)) == NO_ID_ALLOCATED) {
        DriverID = fnAllocateQueue(&pars->queue_sizes, ucControllerEndpoint, entry_add, sizeof(USBQUE)); // allocate queue and buffers
    }
    #else                                                                // either only device or only host mode used
        #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    iController = pars->ucUSB_controller;                                // the controller interface to be used
        #endif
    usb_endpoint_queue = usb_endpoint_control[iController];
    DriverID = fnAllocateQueue(&pars->queue_sizes, USB_CONTROLLER_ENDPOINT(pars->Endpoint, iController), entry_add, sizeof(USBQUE)); // allocate queue and buffers
    #endif
    if (pars->Endpoint != 0) {                                           // endpoint 0, used for general configuration
        usb_endpoint_queue += pars->Endpoint;
        usb_endpoint_queue->usMax_frame_length = pars->usEndpointSize;   // enter end point limit
        if ((pars->usConfig & USB_TERMINATING_ENDPOINT) != 0) {          // {11}
            usb_endpoint_queue->usParameters = USB_ENDPOINT_TERMINATES;  // set endpoint's mode parameter
        }
        if ((pars->usConfig & USB_OUT_ZERO_COPY) != 0) {                 // {31}
            usb_endpoint_queue->usParameters |= USB_ENDPOINT_ZERO_COPY_OUT; // set endpoint's mode parameter
        }
        if ((pars->usConfig & USB_IN_ZERO_COPY) != 0) {                  // {31}
            usb_endpoint_queue->usParameters |= USB_ENDPOINT_ZERO_COPY_IN; // set endpoint's mode parameter
        }
    #if defined USB_HOST_SUPPORT
        if ((pars->usConfig & USB_IN_AUTO_POLL) != 0) {                  // {40}
            usb_endpoint_queue->usParameters |= USB_ENDPOINT_AUTO_IN;    // set endpoint's mode parameter
        }
    #endif
    #if defined HS_USB_AVAILABLE
        if ((pars->usConfig & USB_IN_ZERO_COPY_ISO) != 0) {
            usb_endpoint_queue->usParameters |= USB_ENDPOINT_ZERO_COPY_ISO_IN;
        }
    #endif
    #if defined USB_TX_MESSAGE_MODE
        if ((pars->usConfig & USB_IN_MESSAGE_MODE) != 0) {               // {32}
            unsigned char ucQuantity = (unsigned char)((pars->usConfig & USB_IN_FIFO_MASK) >> USB_IN_FIFO_SHIFT);
            usb_endpoint_queue->usParameters |= USB_ENDPOINT_IN_MESSAGE_MODE; // set endpoint's mode parameter
            usb_endpoint_queue->messageQueue = (USB_MESSAGE_QUEUE *)USB_DRV_MALLOC(sizeof(USB_MESSAGE_QUEUE) + ((ucQuantity - 1) * sizeof(unsigned short)));
            usb_endpoint_queue->messageQueue->ucQuantity = ucQuantity;   // message fifo size
        }
    #endif
    #if defined IN_COMPLETE_CALLBACK                                     // {27}
        usb_endpoint_queue->fnINcomplete = pars->INcallback;             // call back to be used when an IN frame has completed
    #endif
    }
    else {                                                               // default control endpoint
        usb_endpoint_control[iController] = usb_endpoint_queue = USB_DRV_MALLOC(((pars->ucEndPoints + 1) * sizeof(USB_ENDPOINT))); // {8} create the control queue for all end points
        usb_endpoint_queue->usMax_frame_length = ENDPOINT_0_SIZE;        // enter end point 0 limit
        usb_endpoint_queue->event_task = pars->owner_task;               // the task notified of USB state changes
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
        fnConfigUSB(iController, pars);                                  // configure hardware for control end point
    #else
        fnConfigUSB(0, pars);                                            // configure hardware for control end point {20}
    #endif
    }
    if ((usb_endpoint_queue->ptrEndpointInCtr = (que_ids[DriverID - 1].output_buffer_control)) != 0) { // set to output control block
        ((USBQUE *)(usb_endpoint_queue->ptrEndpointInCtr))->endpoint_control = usb_endpoint_queue;
    #if defined WAKE_BLOCKED_USB_TX
        usb_endpoint_queue->low_water_level = pars->low_water_level;
      //usb_endpoint_queue->event_task = 0;                              // uMalloc returns this as zero
    #endif
    }
    usb_endpoint_queue->ucEndpointNumber = USB_CONTROLLER_ENDPOINT(pars->Endpoint, iController); // enter the endpoint number
    if ((pars->Endpoint != 0) && (pars->Paired_RxEndpoint != 0)) {       // this is a paired endpoint, where this endpoint is the receiver (OUT) in the paired channel
        unsigned short usParameters = usb_endpoint_queue->usParameters;
        usb_endpoint_queue = (usb_endpoint_control[iController] + pars->Paired_RxEndpoint);
        usb_endpoint_queue->ucEndpointNumber = USB_CONTROLLER_ENDPOINT(pars->Paired_RxEndpoint, iController); // enter the endpoint number
        usb_endpoint_queue->ucPaired_IN = pars->Endpoint;                // {16}
        usb_endpoint_queue->usMax_frame_length = pars->usEndpointSize;   // enter endpoint size limit
        usb_endpoint_queue->usParameters = usParameters;                 // copy the endpoint parameters to the paired endpoint
    }
    if ((usb_endpoint_queue->ptrEndpointOutCtr = (que_ids[DriverID - 1].input_buffer_control)) != 0) { // set to input control block
        ((USBQUE *)(usb_endpoint_queue->ptrEndpointOutCtr))->endpoint_control = usb_endpoint_queue;
        usb_endpoint_queue->event_task = pars->owner_task;
    }
    usb_endpoint_queue->usb_callback = pars->usb_callback;               // enter optional callback routine for endpoint (used by reception - OUT)
    #if (defined USB_DMA_TX || defined USB_DMA_RX) && defined USB_RAM_START // {17}
    if (NO_MEMORY == fnAllocateUSBBuffer(DriverID, usb_endpoint_queue, &pars->queue_sizes)) {
        return NO_ID_ALLOCATED;
    }
    #endif
    return (DriverID);                                                   // return the allocated ID (handle for the end point)
}

// Send a message to the endpoint owner task
//
static void fnUSB_message(unsigned char ucEvent, unsigned char *ptrData, unsigned char ucDataLength, UTASK_TASK destination)
{
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    unsigned char ucMessage[HEADER_LENGTH + 1 + 4];                      // interface and alternative configuration as payload
#else
    unsigned char ucMessage[HEADER_LENGTH + 1 + 1];                      // maximum 1 byte configuration as payload
#endif
    if (destination == 0) {                                              // ignore if there is no destination task
        return;
    }
#if defined _WINDOWS
    if (ucDataLength > (sizeof(ucMessage) - (HEADER_LENGTH + 1))) {
        _EXCEPTION("Message queue length needs to be increased!!");      // we need to increase the message queue length above if this happens
    }
#endif

    uMemcpy(&ucMessage[HEADER_LENGTH + 1], ptrData, ucDataLength);       // add the payload

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = destination;                       // destination task
    ucMessage[MSG_SOURCE_TASK]      = TASK_USB;                          // own task (fictional)
    ucMessage[MSG_CONTENT_LENGTH]   = ++ucDataLength;                    // message length
    ucMessage[MSG_CONTENT_COMMAND]  = ucEvent;                           // event message type

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(ucDataLength + HEADER_LENGTH)); // send message to USB task
}

// Handle the lengths of individual FIFO buffers - the depth is hardware dependent
//
static void fnPushLastLength(USBQUE *endpoint_queue, unsigned short usLength)
{
    if (endpoint_queue != 0) {
#if defined _USB_CATCH_ERROR
        if (endpoint_queue->endpoint_control->ucFIFO_depth > 1) {        // catch writing beyond end of FIFO length buffer
            fnError(1);
        }
#endif
        endpoint_queue->endpoint_control->usLength[endpoint_queue->endpoint_control->ucFIFO_depth++] = usLength; // enter next
    }
}

// Get the length of a previously transmitted buffer
// 
static unsigned short fnPullLastLength(USB_ENDPOINT *endpoint_control)
{
    unsigned short usLength = endpoint_control->usLength[0];             // length of last data token
#if USB_FIFO_BUFFER_DEPTH > 1
    int iEntry = 1;
    while (iEntry < endpoint_control->ucFIFO_depth) {
        endpoint_control->usLength[iEntry - 1] = endpoint_control->usLength[iEntry]; // shift the lengths in the queue
        iEntry++;
    }
#endif
#if defined _USB_CATCH_ERROR
    if (endpoint_control->ucFIFO_depth == 0) {                           // catch writing before start of FIFO length buffer
        fnError(2);
    }
#endif
    endpoint_control->ucFIFO_depth--;                                    // FIFO one less in depth
    return (usLength);
}

// Get the total amount of data passed to USB buffers but not yet acknowledged
//
static unsigned short fnOutstandingData(USB_ENDPOINT *endpoint_control)
{
    int iEntries = endpoint_control->ucFIFO_depth;
    unsigned short usOutstanding = 0;
#if defined _USB_CATCH_ERROR
    if (iEntries > 1) {                                                  // catch writing beyond end of FIFO length buffer
        fnError(3);
    }
#endif
    while (iEntries-- != 0) {
        usOutstanding += endpoint_control->usLength[iEntries];           // {10}
    }
    return usOutstanding;
}

// Request whether an endpoint is a control endpoint
//
static int fnControlEndpoint(int iEndpoint, int iChannel, unsigned char ucCheck) // {11}
{
    /* {26}
    if (iEndpoint == 0) {
        return 1;                                                        // endpoint 0 is always a control endpoint
    }
    else {*/
        USB_ENDPOINT *usb_endpoint_queue = (usb_endpoint_control[iChannel] + iEndpoint);
        if ((usb_endpoint_queue->ucState & ucCheck) != 0) {              // this endpoint is configured as the check state
            return 1;
        }
    //}
    return 0;
}

// Enter the data to be transmitted into the management queue of its specific end point and start first packet transfer
//
#if defined USB_HOST_SUPPORT                                             // {29}
static QUEUE_TRANSFER fnPrepareOutData(int iMode, unsigned char *ptrData, unsigned short usLength, unsigned short ucMaxLength, int iEndpoint, USB_HW *ptrUSB_HW)
#else
static QUEUE_TRANSFER fnPrepareOutData(unsigned char *ptrData, unsigned short usLength, unsigned short ucMaxLength, int iEndpoint, USB_HW *ptrUSB_HW)
#endif
{
#if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    register int iChannel = ptrUSB_HW->ucController;
#else
    register int iChannel = 0;
#endif
#if defined USB_HOST_SUPPORT
    unsigned char ucTransferType;
#endif
    USB_ENDPOINT *tx_queue = (usb_endpoint_control[iChannel] + iEndpoint);
    unsigned short usAdditionalLength;                                   // {21}

    if (tx_queue->usCompleteMessage != 0) {                              // transmission already in progress
        return 0;
    }
#if defined USB_HOST_SUPPORT                                             // {29}
    if (_USB_HOST_MODE()) {
        if ((_HOST_SETUP == iMode) || ((iEndpoint == 0) && (_DEVICE_DATA_HOST_SETUP == iMode) && (8 == usLength))) { // is host setup data is to be prepared
            fnPrepareSetup(ptrUSB_HW);                                   // prepare for setup frame transmission (this often requires the data token to be resysnchronised because the next transmitted data token is DATA0 and the next received data token is DATA1)
            ucTransferType = SETUP_PID;                                  // a SETUP token is to be sent
        }
        else {
            ucTransferType = OUT_PID;                                    // an OUT frame is to be sent
        }
    }
    else {
        iMode = _DEVICE_DATA;                                            // not host mode
    }
#endif
    tx_queue->usLimitLength = ucMaxLength;
    if (usLength > ucMaxLength) {                                        // if host cannot accept full length, cut it shorter
        usLength = ucMaxLength;
    }

    tx_queue->usCompleteMessage = usLength;                              // total queued length
#if defined USB_AUTO_TX                                                  // {25}
    if (ptrUSB_HW->ucDeviceType == USB_DEVICE_HS) {
        tx_queue->usSent = usLength;                                     // always pass a single buffer which the USB HS controller sends in multiple frames
    }
    else  {
#endif
#if defined MULTIPLE_TX                                                  // {22}
        if (iEndpoint == 0) {
            if (usLength > (MULTIPLE_TX*tx_queue->usMax_frame_length)) {
                tx_queue->usSent = (MULTIPLE_TX*tx_queue->usMax_frame_length); // maximum endpoint length
            }
            else {
                tx_queue->usSent = usLength;                             // complete message in one buffer
            }
        }
        else {
            if (usLength > tx_queue->usMax_frame_length) {
                tx_queue->usSent = tx_queue->usMax_frame_length;         // maximum endpoint length
            }
            else {
                tx_queue->usSent = usLength;                             // complete message in one buffer
            }
        }
#else
        if (usLength > tx_queue->usMax_frame_length) {                   // if the data length is greater than the endpoint size
            tx_queue->usSent = tx_queue->usMax_frame_length;             // set maximum endpoint length for first packet
        }
        else {
            tx_queue->usSent = usLength;                                 // complete message in one buffer
        }
#endif
#if defined USB_AUTO_TX                                                  // {23}
    }
#endif
    usAdditionalLength = tx_queue->usSent;
    FNSEND_USB_DATA(ptrData, usAdditionalLength, iEndpoint, ptrUSB_HW);  // prepare packet in hardware buffer
    fnPushLastLength((USBQUE *)(tx_queue->ptrEndpointInCtr), usAdditionalLength); // save last length for use later
    tx_queue->ptrStart = ptrData;                                        // set the start pointer to the start of the data packet
    while (tx_queue->usCompleteMessage > tx_queue->usSent) {             // controllers usually have multiple output buffers so fill up as many as possible
      //unsigned short usAdditionalLength = tx_queue->usSent;            // {21} set before entering the loop
        USBQUE *endpoint_queue = (USBQUE *)(tx_queue->ptrEndpointInCtr); // {3} additional check of tx buffer depth
        if (endpoint_queue != 0) {                                       // if the endpoint is using a circular output buffer
            if (endpoint_queue->endpoint_control->ucFIFO_depth >= USB_FIFO_BUFFER_DEPTH) { // {5} if all endpoint buffers are in use
              //return tx_queue->usCompleteMessage;
                break;                                                   // {6} all available buffers have been prepared
            }
        }
        if (fnGetUSB_HW(USB_CONTROLLER_ENDPOINT(iEndpoint, iChannel), &ptrUSB_HW) != ENDPOINT_FREE) { // {28} get the next free buffer if possible
            //return tx_queue->usCompleteMessage;
            break;                                                       // {6} no further buffer ready to accept data
        }
        usLength -= usAdditionalLength;                                  // remaining length to be queued
        ptrData += usAdditionalLength;
        if (usLength > tx_queue->usMax_frame_length) {
            usAdditionalLength = tx_queue->usMax_frame_length;           // maximum endpoint length
        }
        else {
            usAdditionalLength = usLength;                               // complete message prepared
        }
#if defined USB_TX_MESSAGE_MODE
        if (tx_queue->messageQueue != 0) {                               // {32}
            QUEQUE *ptTxQueue = &((USBQUE *)(tx_queue->ptrEndpointInCtr))->USB_queue;
            if ((ptrData + usAdditionalLength) >= ptTxQueue->buffer_end) { // if the buffer doesn't fit in the circular buffer
                ptrData = ptTxQueue->QUEbuffer;                          // move to start of circular buffer
                tx_queue->ptrStart = (ptrData - tx_queue->usSent);       // set virtual buffer start to compensate for the wrap-around
            }
        }
#endif
        FNSEND_USB_DATA(ptrData, usAdditionalLength, iEndpoint, ptrUSB_HW); // prepare hardware buffer
        fnPushLastLength((USBQUE *)(tx_queue->ptrEndpointInCtr), usAdditionalLength); // save last length for later use
        tx_queue->usSent += usAdditionalLength;                          // size of present frame in progress
    }
#if defined USB_HOST_SUPPORT                                             // {29}
    if (iMode > _DEVICE_DATA) {                                          // if host data has been prepared
        fnHostReleaseBuffer(iEndpoint, ucTransferType, ptrUSB_HW);       // allow host to release prepared data
    }
#endif
    return tx_queue->usCompleteMessage;                                  // message length that was accepted
}

#if defined USB_DEVICE_SUPPORT
// Extract (certain values) from standard descriptor requests in little-endian format
//
static void fnExtract(unsigned char *ptrData, unsigned char ucFlags, unsigned short *usValues)
{
    #define VALUE_INDEX   0x01
    #define INDEX_INDEX   0x02
    #define LENGTH_INDEX  0x04

    if ((ucFlags & VALUE_INDEX) != 0) {
        *usValues =  GET_USB_DATA();
        *usValues++ |= (GET_USB_DATA() << 8);
    }
    else {
#if defined USB_FIFO
        GET_USB_DATA();GET_USB_DATA();                                   // {15}
#else
        ptrData += sizeof(unsigned short);
#endif
    }
    if ((ucFlags & INDEX_INDEX) != 0) {
        *usValues =  GET_USB_DATA();
        *usValues++ |= (GET_USB_DATA() << 8);
    }
    else {
#if defined USB_FIFO
        GET_USB_DATA();GET_USB_DATA();                                   // {15}
#else
        ptrData += sizeof(unsigned short);
#endif
    }
    if ((ucFlags & LENGTH_INDEX) != 0) {
        *usValues =  GET_USB_DATA();
        *usValues |= (GET_USB_DATA() << 8);
    }
}
#endif

#if defined WAKE_BLOCKED_USB_TX
// Wake a blocked USB IN endpoint queue by informing its owner task that there is space available to put new data to
//
static void fnWakeBlockedTxUSB(USBQUE *ptrUsbQueue)
{
    QUEUE_TRANSFER low_water = ptrUsbQueue->endpoint_control->low_water_level;
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
    if ((ptrUsbQueue->endpoint_control->event_task_in != 0) && (ptrUsbQueue->USB_queue.chars <= low_water))
    #else
    if ((ptrUsbQueue->endpoint_control->event_task != 0) && (ptrUsbQueue->USB_queue.chars <= low_water))
    #endif
    {                                                                    // we have just adequately emptied buffer content so inform waiting transmitter task
        unsigned char tx_continue_message[HEADER_LENGTH]; // = { INTERNAL_ROUTE, INTERNAL_ROUTE , ptrUsbQueue->tx_queue->event_task, INTERRUPT_EVENT, TX_FREE };  // define standard interrupt event
        tx_continue_message[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;
        tx_continue_message[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
        tx_continue_message[MSG_DESTINATION_TASK] = ptrUsbQueue->endpoint_control->event_task_in;
    #else
        tx_continue_message[MSG_DESTINATION_TASK] = ptrUsbQueue->endpoint_control->event_task;
    #endif
        tx_continue_message[MSG_SOURCE_TASK]      = INTERRUPT_EVENT;
    #if defined CUSTOM_USB_TX_FREE
        tx_continue_message[MSG_INTERRUPT_EVENT] = (unsigned char)(TX_FREE - ptrUsbQueue->endpoint_control->ucCustomTxFree_event); // {39}
    #else
        tx_continue_message[MSG_INTERRUPT_EVENT] = (unsigned char)TX_FREE;
    #endif
        fnWrite(INTERNAL_ROUTE, (unsigned char*)tx_continue_message, HEADER_LENGTH); // inform the blocked task
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
        ptrUsbQueue->endpoint_control->event_task_in = 0;                // remove task since this is only performed once
    #else
        ptrUsbQueue->endpoint_control->event_task = 0;                   // remove task since this is only performed once
    #endif
    }
}
#endif

#if defined USB_HOST_SUPPORT                                             // {29}
    #if defined USB_STRING_OPTION                                        // requests strings if required
// Search for next string in the device descriptor (it is assumed that they count from 1..max. but the order is not important)
//
static int fnNeedString(int iStringReference, int iChannel)
{
    unsigned char *ptrStringRef = &device_descriptor[iChannel].iManufacturer;
    if (iStringReference == 0) {                                         // language id is always valid
        return 0;
    }
    while (ptrStringRef <= &device_descriptor[iChannel].iSerialNumber) { // check manufacturer, product and serial number references
        if (*ptrStringRef++ == (unsigned char)(iStringReference)) {
            return 0;
        }
    }
    return -1;
}
    #endif

// Clears a stalled device endpoint by sending the ClearFeature
//
extern int iDebug;
extern QUEUE_TRANSFER fnClearFeature(QUEUE_LIMIT control_handle, unsigned char ucEndpoint, int iChannel)
{
    static unsigned char ucClearFeature[USBS_AVAILABLE][8];              // use static memory so that it remains stable when sending on non-buffered endpoint
    ucStalledEndpoint[iChannel] = ucEndpoint;                            // note the endpoint that has stalled and will be cleared
    ucClearFeature[iChannel][0] = REQUEST_ENDPOINT_STANDARD;             // 0x02 request type standard, recipient is endpoint, host to device
    ucClearFeature[iChannel][1] = USB_REQUEST_CLEAR_FEATURE;             // 0x01 clear feature
    ucClearFeature[iChannel][2] = 0x00;                                  // endpoint halt
    ucClearFeature[iChannel][3] = 0x00;
    ucClearFeature[iChannel][4] = ucStalledEndpoint[iChannel];           // endpoint reference
    ucClearFeature[iChannel][5] = 0x00;
    ucClearFeature[iChannel][6] = 0x00;                                  // length 0
    ucClearFeature[iChannel][7] = 0x00;
  //iDebug = 2;
    return (fnWrite(control_handle, (unsigned char *)&ucClearFeature[iChannel], 8)); // send clear feature on endpoint 0
}

// Zero data reception on control endpoint received after sending a request endpoint feature
//
static int fnUnstallAck(USB_HW *ptrUSB_HW)
{
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    register int iChannel = ptrUSB_HW->ucController;                     // the interface the acknowlege was received on
    #else
    register int iChannel = 0;
    #endif
    if (ucStalledEndpoint[iChannel] != 0) {                              // acknowledge from unstalling endpoint is expected
      //ucStalledEndpoint[iChannel] &= ~(IN_ENDPOINT);
        // After clearing an endpoint the data toggle needs to be resynchronised (DATA 1 is next transmission and DATA 0 is next reception)
        //
        fnResetDataToggle(iChannel, ucStalledEndpoint[iChannel], ptrUSB_HW);
        fnUSB_message((E_USB_DEVICE_CLEARED + iChannel), &ucStalledEndpoint[iChannel], 1, usb_endpoint_control[iChannel]->event_task); // inform the owner task so that it can decide what to do
        ucStalledEndpoint[iChannel] = 0;                                 // the endpoint has been cleared
        return 1;
    }
    return 0;
}

int iDebug = 0;


// USB host enumeration controller
//
static int fnHostEnumeration(int iEndpoint, int iEvent, USB_HW *ptrUSB_HW)
{
    static const USB_HOST_DESCRIPTOR get_device_descriptor = {
        (STANDARD_DEVICE_TO_HOST),                                       // 0x80 - recipient host, type standard, device-to-host
        USB_REQUEST_GET_DESCRIPTOR,                                      // 0x06 - get descriptor from device
        {LITTLE_SHORT_WORD_BYTES(STANDARD_DEVICE_DESCRIPTOR)},           // standard device descriptor
        {LITTLE_SHORT_WORD_BYTES(0)},                                    // no index
        {LITTLE_SHORT_WORD_BYTES(sizeof(USB_DEVICE_DESCRIPTOR))}         // size of response that is expected
    };

    static const USB_HOST_DESCRIPTOR set_address = {
        (STANDARD_HOST_TO_DEVICE),                                       // 0x00 - recipient device, type standard, host-to-device
        USB_REQUEST_SET_ADDRESS,                                         // 0x05 - set device's address
        {LITTLE_SHORT_WORD_BYTES(USB_DEVICE_ADDRESS)},                   // address
        {LITTLE_SHORT_WORD_BYTES(0)},                                    // no index
        {LITTLE_SHORT_WORD_BYTES(0)}                                     // no response expected
    };

    static const USB_HOST_DESCRIPTOR get_configuration_descriptor = {
        (STANDARD_DEVICE_TO_HOST),                                       // 0x80 - recipient host, type standard, device-to-host
        USB_REQUEST_GET_DESCRIPTOR,                                      // 0x06 - get descriptor from device
        {LITTLE_SHORT_WORD_BYTES(STANDARD_CONFIG_DESCRIPTOR)},           // standard device descriptor
        {LITTLE_SHORT_WORD_BYTES(0)},                                    // no index
        {LITTLE_SHORT_WORD_BYTES(MAX_CONFIGURATION_DESCRIPTOR_LENGTH)}   // size of response that is possible
    };

    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    register int iChannel = ptrUSB_HW->ucController;
    #else
    register int iChannel = 0;
    #endif
    USB_ENDPOINT *usb_endpoint_queue = usb_endpoint_control[iChannel];
    usb_endpoint_queue += iEndpoint;

    if ((iEvent == HOST_APPLICATION_ON) && (ucRequestType[iChannel] != HOST_ENUMERATION_SET_CONFIGURATION)) {
        return USB_HOST_ERROR;                                           // only allow the application to set the configuration when in the required state
    }
  //if (DEVICE_TIMEOUT == iEvent) {
  //    ucRequestType[iChannel]--;                                       // repeat last request due to no response
  //}
    switch (ucRequestType[iChannel]) {
    case HOST_ENUMERATION_IDLE:                                          // starting from device connection
    #if defined USB_STRING_OPTION
        iStringReference[iChannel] = 0;
    #endif
        usRequestLengthRemaining[iChannel] = sizeof(USB_DEVICE_DESCRIPTOR);
      //iDebug = 1;
        fnPrepareOutData(HOST_SETUP (unsigned char *)&get_device_descriptor, sizeof(get_device_descriptor), 8, 0, ptrUSB_HW); // prepare setup stage of get device descriptor
        fnInterruptMessage(usb_endpoint_control[iChannel]->event_task, (unsigned char)(EVENT_USB_DETECT_LS + iChannel + (ptrUSB_HW->ucDeviceSpeed * 2))); // inform of the attachment and bus speed (+2 is EVENT_USB_DETECT_FS, +4 is EVENT_USB_DETECT_HS)
        ucRequestType[iChannel] = HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR; // note the request type that is in operation
        break;

    case HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR:                    // standard device descriptor has been received        
        ucRequestType[iChannel] = HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR_ACK; // set next state since we expect an ack
        return TERMINATE_ZERO_DATA;                                      // reception buffer has been consumed and we need to send a zero termination

    case HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR_ACK:
        fnPrepareOutData(HOST_SETUP  (unsigned char *)&set_address, sizeof(set_address), 8, 0, ptrUSB_HW);
        ucRequestType[iChannel] = HOST_ENUMERATION_SET_ADDRESS;
        break;

    case HOST_ENUMERATION_SET_ADDRESS:                                   // address has been acknowledged by the device
        fnSetUSB_device_address(USB_DEVICE_ADDRESS);                     // set the device address since it has been acknowledged
        usRequestLengthRemaining[iChannel] = MAX_CONFIGURATION_DESCRIPTOR_LENGTH; // maximum response length
        fnPrepareOutData(HOST_SETUP  (unsigned char *)&get_configuration_descriptor, sizeof(get_configuration_descriptor), 8, 0, ptrUSB_HW); // send setup stage of get configuration descriptor (this is sent to the new address)
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR; // set next state
        break;

    case HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR:
        usConfigDescriptorLength[iChannel] = usRequestingReceived[iChannel]; // the length of the received configuration descriptor
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR_ACK; // set next state
        return TERMINATE_ZERO_DATA;                                      // reception buffer has been consumed and we need to send a zero termination
    #if defined USB_STRING_OPTION
    case HOST_ENUMERATION_REQUEST_STRING_ACK:
        iStringReference[iChannel]++;
        // Fall through intentional
        //
    #endif
    case HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR_ACK:                  // when we arrive here we have received the standard device descriptor and configuration descriptor and so have adequate information concerning the device attached to decide whether to work with it
    #if defined USB_STRING_OPTION                                        // requests strings if required
        if ((iStringReference[iChannel] <= USB_MAX_STRINGS) && (fnNeedString(iStringReference[iChannel], iChannel) == 0)) { // get next string
            static USB_HOST_DESCRIPTOR get_string = {0};                 // {36} static descriptor space so that the memory remains stable when used directly by the buffer descriptor
            get_string.bmRecipient_device_direction = STANDARD_DEVICE_TO_HOST; // 0x80 - recipient host, type standard, device-to-host
            get_string.bRequest = USB_REQUEST_GET_DESCRIPTOR;            // 0x06 - get descriptor from device
            get_string.wValue[0] = (unsigned char)iStringReference[iChannel];
            get_string.wValue[1] = DESCRIPTOR_TYPE_STRING;               // 0x03
            if (iStringReference[iChannel] == 0) {                       // if requesting string language
                get_string.wIndex[0] = get_string.wIndex[1] = 0;
            }
            else {
                get_string.wIndex[0] = ucStringList[0][2];
                get_string.wIndex[1] = ucStringList[0][3];
            }
            get_string.wLength[0] = (unsigned char)((USB_MAX_STRING_LENGTH * 2) + 1); // length and uni-code string
            get_string.wLength[1] = 0;

            usRequestLengthRemaining[iChannel] = (unsigned char)(USB_MAX_STRING_LENGTH * 2); // up to maximum string length
            fnPrepareOutData(HOST_SETUP  (unsigned char *)&get_string, sizeof(get_string), 8, 0, ptrUSB_HW); // send setup stage of get device descriptor
            ucRequestType[iChannel] = HOST_ENUMERATION_REQUEST_STRING;
            break;
        }
    #endif
        fnUSB_message((E_USB_DEVICE_INFO + iChannel), 0, 0, usb_endpoint_queue->event_task); // inform the USB application that this information is ready
        ucRequestType[iChannel] = HOST_ENUMERATION_SET_CONFIGURATION;    // enumeration information has been collected - the application will decide whether a configuration is set
        break;
    #if defined USB_STRING_OPTION
    case HOST_ENUMERATION_REQUEST_STRING:
        usRequestingReceived[iChannel] = 0;                              // {34} reset previous string reception length counter since this transaction has completed
        ucRequestType[iChannel] = HOST_ENUMERATION_REQUEST_STRING_ACK;   // set next state since we expect an ack
        return TERMINATE_ZERO_DATA;                                      // reception buffer has been consumed and we need to send a zero termination
    #endif
    case HOST_ENUMERATION_SET_CONFIGURATION:                             // the configuration is to be set
        {
            static USB_HOST_DESCRIPTOR set_configuration;
    #if defined USB_HOST_HUB
            if (usb_endpoint_queue->usb_callback != 0) {                 // during hub enumeration the application may perform transactions in this state and so an event is generate
                if (usb_endpoint_queue->usb_callback(0, 0, STATUS_STAGE_RECEPTION_ENUM, iChannel) == USB_HOST_ERROR) { // inform the user of the fact that the transaction was completed successfully (the user can also prepare new control data)
                    break;                                               // the event has been consumed by the application so we do not handle it in the enumeration state-event machine
                }
            }
    #endif
            uMemset(&set_configuration, 0, sizeof(set_configuration));
            set_configuration.bmRecipient_device_direction = STANDARD_HOST_TO_DEVICE; // 0x00 - recipient device, type standard, host-to-device
            set_configuration.bRequest = USB_REQUEST_SET_CONFIGURATION;  // 0x09 - set configuration
            set_configuration.wValue[0] = ucActiveConfiguration[iChannel];
            fnPrepareOutData(HOST_SETUP  (unsigned char *)&set_configuration, sizeof(set_configuration), 8, 0, ptrUSB_HW); // send setup stage of set configuration
            ucRequestType[iChannel] = HOST_ENUMERATION_SET_CONFIGURATION_ACK;
            break;
        }
        break;
    case HOST_ENUMERATION_SET_CONFIGURATION_ACK:
    #if defined SET_INTERFACE
        {
    static const USB_HOST_DESCRIPTOR set_interface = {
        (STANDARD_HOST_TO_DEVICE | REQUEST_INTERFACE_STANDARD),          // 0x01 request class specific interface
        USB_REQUEST_SET_INTERFACE,                                       // 0x0b
        {LITTLE_SHORT_WORD_BYTES(0)},
        {LITTLE_SHORT_WORD_BYTES(0)},
        {LITTLE_SHORT_WORD_BYTES(0)},
    };

            fnPrepareOutData(HOST_SETUP  (unsigned char *)&set_interface, sizeof(set_interface), 8, 0, ptrUSB_HW); // send setup stage of set configuration
            ucRequestType[iChannel] = HOST_ENUMERATION_SET_INTERFACE_ACK;
        }
        break;
    case HOST_ENUMERATION_SET_INTERFACE_ACK:
    #endif
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURED;           // the device has been configured and the host is fully operating
    #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
        fnUSB_message((E_USB_ACTIVATE_CONFIGURATION + iChannel), &ucActiveConfiguration[iChannel], sizeof(ucActiveConfiguration[iChannel]), usb_endpoint_control[iChannel]->event_task); // enumeration has completed
    #else
        fnUSB_message(E_USB_ACTIVATE_CONFIGURATION, &ucActiveConfiguration[0], sizeof(ucActiveConfiguration[0]), usb_endpoint_control[0]->event_task); // enumeration has completed
    #endif
        break;

    case HOST_ENUMERATION_CONFIGURED:
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURED_ACK;       // we will send a zero data frame once the device has responded
        return TERMINATE_ZERO_DATA;                                      // reception buffer has been consumed and we need to send a zero termination

    case HOST_ENUMERATION_CONFIGURED_ACK:
    #if defined USB_RNDIS_HOST
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURED;           // return to configured state
    #else
        ucRequestType[iChannel] = HOST_ENUMERATION_CONFIGURED_IDLE;
    #endif
      //fnDebugMsg("IDLE\r\n");
        if (usb_endpoint_queue->usb_callback != 0) {
            return (usb_endpoint_queue->usb_callback(0, 0, STATUS_STAGE_RECEPTION, iChannel)); // inform the user of the fact that the transaction was completed successfully (the user can also prepare new control data)
        }
        break;
    case HOST_ENUMERATION_CONFIGURED_IDLE:
        fnUnstallAck(ptrUSB_HW);                                         // only a confirmation of an uninstalled endpoint is expected in this state
        break;
    default:
        break;
    }
    return BUFFER_CONSUMED;                                              // nothing to do
}

// Interface for application to access device information
//
extern void *fnGetDeviceInfo(int iInfoRef, int iChannel)
{
    if (iInfoRef == REQUEST_USB_DEVICE_DESCRIPTOR) {                     // return pointer to device descriptor
        return &device_descriptor[iChannel];
    }
    else if (iInfoRef == REQUEST_USB_CONFIG_DESCRIPTOR) {                // return pointer to configuration descriptor
        return &config_descriptor[iChannel];
    }
    #if defined USB_STRING_OPTION
    else {                                                               // strings
        int iStringRef = (iInfoRef - REQUEST_USB_STRING_DESCRIPTOR);
        if (iStringRef > USB_MAX_STRINGS) {
            return 0;
        }
        return (ucStringList[iStringRef]);
    }
    #endif
    return 0;
}
#endif


// Generic USB data handler - called from interrupt routine
//
extern int fnUSB_handle_frame(unsigned char ucType, unsigned char *ptrData, int iEndpoint, USB_HW *ptrUSB_HW)
{
#if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    register int iChannel = ptrUSB_HW->ucController;
#else
    register int iChannel = 0;
#endif
    switch (ucType) {                                                    // depending on the event
    case USB_TX_ACKED:                                                   // a previous transmission has been successfully acked
        {
            USB_ENDPOINT *tx_queue = (usb_endpoint_control[iChannel] + iEndpoint);
            USBQUE *ptrUsbQueue = (USBQUE *)(tx_queue->ptrEndpointInCtr);
            if (tx_queue->usCompleteMessage == 0) {                      // no message in queue
#if defined USB_HOST_SUPPORT                                             // {29}
                if ((iEndpoint == 0) && _USB_HOST_MODE()) {              // acknowledgement to a zero data termination
                    return (fnHostEnumeration(iEndpoint, SUCCESSFUL_TERMINATION, ptrUSB_HW));
                }
#endif
                if (ptrUsbQueue == 0) {
                    break;
                }
                if (ptrUsbQueue->USB_queue.chars != 0) {                 // more data added to queue in the mean time
#if defined USB_DMA_TX && defined USB_RAM_START
                    tx_queue->usCompleteMessage = tx_queue->usSent = fnPrepareUSBOutData(ptrUsbQueue, ptrUsbQueue->USB_queue.chars, iEndpoint, ptrUSB_HW);
#else
                    unsigned short usLength;
                    if (ptrUsbQueue->USB_queue.get >= ptrUsbQueue->USB_queue.buffer_end) { // handle circular buffer
                        ptrUsbQueue->USB_queue.get -= ptrUsbQueue->USB_queue.buf_length;
                    }
                    usLength = (ptrUsbQueue->USB_queue.buffer_end - ptrUsbQueue->USB_queue.get); // linear buffer remaining (maximum linear block size possible)
                    if (ptrUsbQueue->USB_queue.chars < usLength) {       // if there are less additional characters waiting reduce
                        usLength = ptrUsbQueue->USB_queue.chars;         // next block length required
                    }
                    fnPrepareOutData(DEVICE_HOST_DATA  ptrUsbQueue->USB_queue.get, usLength, usLength, iEndpoint, ptrUSB_HW); // prepare next buffer(s)
#endif
#if defined WAKE_BLOCKED_USB_TX
                    fnWakeBlockedTxUSB(ptrUsbQueue);                     // we have just emptied buffer content so inform waiting transmitter task
#endif
                }
                else {
                    ptrUsbQueue->endpoint_control->ucState &= ~TX_ACTIVE;// mark that the transmitter is idle and should be started when the next data is added to the buffer
#if defined IN_COMPLETE_CALLBACK
                    if (tx_queue->fnINcomplete != 0) {                   // if there is an IN complete callback execute it now
                        tx_queue->usSent = 0;
                        tx_queue->usCompleteMessage = 0;
                        tx_queue->ucFIFO_depth = 0; // temp during development!!
                        tx_queue->fnINcomplete(tx_queue->ucEndpointNumber);
                    }
#endif
                }
                break;
            }
            if (tx_queue->usSent >= tx_queue->usCompleteMessage) {       // message transmission content is completely in transmission so we can prepare next block if available
                if ((ptrUsbQueue != 0) && (ptrUsbQueue->USB_queue.chars != 0)) { // if there is transmit buffer data waiting add it now
#if defined USB_DMA_TX && defined USB_RAM_START
                    ptrUsbQueue->USB_queue.chars -= tx_queue->usCompleteMessage;
    #if defined WAKE_BLOCKED_USB_TX
                    fnWakeBlockedTxUSB(ptrUsbQueue);                     // we have just reduced buffer content so inform waiting transmitter task
    #endif
                    ptrUsbQueue->USB_queue.get += tx_queue->usCompleteMessage; // set get pointer to end of outstanding block
                    tx_queue->usCompleteMessage = 0;
                    tx_queue->usSent = 0;
                    if (ptrUsbQueue->USB_queue.get >= ptrUsbQueue->USB_queue.buffer_end) { // handle circular buffer
                        ptrUsbQueue->USB_queue.get -= ptrUsbQueue->USB_queue.buf_length;
                    }
                    if (ptrUsbQueue->USB_queue.chars == 0) {             // no more data queued
                        ptrUsbQueue->endpoint_control->ucState &= ~TX_ACTIVE;// mark that the transmitter is idle
                    }
                    else {
                        fnPrepareUSBOutData(ptrUsbQueue, ptrUsbQueue->USB_queue.chars, iEndpoint, ptrUSB_HW);
                    }
#else
                    unsigned short usNonAcknowledged;
                    ptrUsbQueue->USB_queue.chars -= fnPullLastLength(ptrUsbQueue->endpoint_control); // the acknowledged data (there may be still open buffers)
                    usNonAcknowledged = fnOutstandingData(ptrUsbQueue->endpoint_control);
                    if (ptrUsbQueue->USB_queue.chars == usNonAcknowledged) { // no further data waiting in the output buffer which has not already been placed in the output FIFO buffers
                        if (ptrUsbQueue->USB_queue.chars == 0) {         // no more data queued
                            if (fnControlEndpoint(iEndpoint, iChannel, (USB_CONTROL_ENDPOINT | USB_ENDPOINT_TERMINATES)) != 0) { // {9}{11} if the endpoint is a control end point send zero data on termination if last frame was the same length as the endpoint
                                if ((tx_queue->usCompleteMessage % tx_queue->usMax_frame_length) == 0) {
                                    FNSEND_ZERO_DATA(ptrUSB_HW, iEndpoint); // send a zero frame to terminate status stage
                                    usNonAcknowledged = 1;               // ensure that the transmitter state is not deactivated yet
                                }
                            }
    #if defined USB_TX_MESSAGE_MODE
                            if (ptrUsbQueue->endpoint_control->messageQueue != 0) { // {32} if the endpoint is working in message mode
                                ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex = ptrUsbQueue->endpoint_control->messageQueue->ucInIndex;
                                ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.put;
                            }
                            else {
    #endif
                                ptrUsbQueue->USB_queue.get += tx_queue->usCompleteMessage; // set get pointer to end of outstanding block
                                if (ptrUsbQueue->USB_queue.get >= ptrUsbQueue->USB_queue.buffer_end) { // handle circular buffer
                                    ptrUsbQueue->USB_queue.get -= ptrUsbQueue->USB_queue.buf_length;
                                }
    #if defined USB_TX_MESSAGE_MODE
                            }
    #endif
                            tx_queue->usCompleteMessage = 0;             // no more processing required
                            if (usNonAcknowledged == 0) {
                                ptrUsbQueue->endpoint_control->ucState &= ~TX_ACTIVE; // mark that the transmitter is idle
                            }
                        }
                        else {                                           // wait for final buffer acknowledgement(s) - nothing to do otherwise
                            if ((ptrUsbQueue->endpoint_control->usParameters & USB_ENDPOINT_ZERO_COPY_IN) != 0) {
                                tx_queue->usCompleteMessage = 0;         // reset old block length so that a new one will be accepted
                                ptrUsbQueue->USB_queue.chars += ptrUsbQueue->USB_queue.buf_length;
                                fnPrepareOutData(DEVICE_HOST_DATA  ptrUsbQueue->USB_queue.get, ptrUsbQueue->USB_queue.buf_length, ptrUsbQueue->USB_queue.buf_length, iEndpoint, ptrUSB_HW); // prepare next buffer(s)
                            }
                        }
                    }
                    else {                                               // there is follow-on data waiting in the buffer so start this now
                        unsigned short usLength;
                        ptrUsbQueue->USB_queue.get += tx_queue->usCompleteMessage; // set get pointer to end of outstanding block
    #if defined USB_TX_MESSAGE_MODE
                        if (ptrUsbQueue->endpoint_control->messageQueue != 0) { // {32} if in message mode we must handle next individual message and not necessarily the total queued data
                            usLength = ptrUsbQueue->endpoint_control->messageQueue->usLength[ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex];
                            ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex++;
                            if (ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex >= ptrUsbQueue->endpoint_control->messageQueue->ucQuantity) {
                                ptrUsbQueue->endpoint_control->messageQueue->ucOutIndex = 0;
                            }
                            if (tx_queue->ptrStart < ptrUsbQueue->USB_queue.QUEbuffer) { //  message wrap resulted in unused space at the end of the circular buffer
                                ptrUsbQueue->USB_queue.get = (ptrUsbQueue->USB_queue.QUEbuffer + 1);
                                ptrUsbQueue->USB_queue.get += (ptrUsbQueue->USB_queue.QUEbuffer - tx_queue->ptrStart);
                            }
                            else if ((ptrUsbQueue->USB_queue.buffer_end - ptrUsbQueue->USB_queue.get) < ptrUsbQueue->endpoint_control->usMax_frame_length) {
                                ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.QUEbuffer; // the next message starts at the beginning of the buffer
                            }
                        }
                        else {
    #endif
                            if (ptrUsbQueue->USB_queue.get >= ptrUsbQueue->USB_queue.buffer_end) { // handle circular buffer
                                ptrUsbQueue->USB_queue.get -= ptrUsbQueue->USB_queue.buf_length;
                            }
                            usLength = (ptrUsbQueue->USB_queue.buffer_end - ptrUsbQueue->USB_queue.get); // linear buffer remaining (maximum linear block size possible)
                            if ((ptrUsbQueue->USB_queue.chars - usNonAcknowledged) < usLength) { // if there are less additional characters waiting reduce
                                usLength = (ptrUsbQueue->USB_queue.chars - usNonAcknowledged); // next block length required
                            }
    #if defined USB_TX_MESSAGE_MODE
                        }
    #endif
                        tx_queue->usCompleteMessage = 0;                 // reset old block length so that the new one will be accepted
                        fnPrepareOutData(DEVICE_HOST_DATA  ptrUsbQueue->USB_queue.get, usLength, usLength, iEndpoint, ptrUSB_HW); // prepare next buffer(s)
                    }
    #if defined WAKE_BLOCKED_USB_TX
                    fnWakeBlockedTxUSB(ptrUsbQueue);                     // we have just emptied buffer content so inform waiting transmitter task
    #endif
#endif
                }
                else {                                                   // output buffer is empty, so all data has been sent
#if defined USB_HOST_SUPPORT                                             // {29}
                    if (_USB_HOST_MODE()) {                              // if host transmission has been acked
                        tx_queue->usCompleteMessage = 0;                 // message completely sent
                        if (ucRequestType[iChannel] != HOST_ENUMERATION_IDLE) { // if we are requesting setup information
    #if defined USB_HOST_CALLBACK_SETUP_ACK                              // {37}
                            if (ucRequestType[iChannel] >= HOST_ENUMERATION_CONFIGURED) {
                                if (usb_endpoint_control[iChannel]->usb_callback != 0) {
                                    return (usb_endpoint_control[iChannel]->usb_callback(0, 0, HOST_SETUP_ACKED, iChannel));
                                }
                            }
    #endif
                            return INITIATE_IN_TOKEN;                    // initiate IN token stage since we expect a reply from the device
                        }
                        return BUFFER_CONSUMED;
                    }
#endif
#if defined USB_DEVICE_SUPPORT                                           // {29}
                    if ((tx_queue->usCompleteMessage % tx_queue->usMax_frame_length) == 0) { // if the transmitted data length was divisible by the frame length
                        if ((tx_queue->usLimitLength != tx_queue->usCompleteMessage) || (fnControlEndpoint(iEndpoint, iChannel, (USB_ENDPOINT_TERMINATES)) != 0)) { // {26}
                            FNSEND_ZERO_DATA(ptrUSB_HW, iEndpoint);      // send a zero frame to terminate status stage
                        }
                    }
                    tx_queue->usCompleteMessage = 0;                     // message completely sent
    #if defined IN_COMPLETE_CALLBACK                                     // {27}
                    if (tx_queue->fnINcomplete != 0) {                   // if there is an IN complete callback execute it now
                        tx_queue->fnINcomplete(tx_queue->ucEndpointNumber);
                    }
    #endif
#endif
                }
            }
            else {                                                       // part of a linear block
                unsigned short usDataLength;
                unsigned short usRemaining = (tx_queue->usCompleteMessage - tx_queue->usSent);
                if (usRemaining < tx_queue->usMax_frame_length) {
                    usDataLength = usRemaining;                          // limit packet length
                }
                else {
                    usDataLength = tx_queue->usMax_frame_length;
                }
                if (ptrUsbQueue != 0) {                                  // endpoint has a circular buffer
                    unsigned short usLastLength = fnPullLastLength(ptrUsbQueue->endpoint_control); // the amount of data being acknowledged
                    ptrUsbQueue->USB_queue.chars -= usLastLength;        // make space for fresh data to be queued
    #if defined WAKE_BLOCKED_USB_TX
                    fnWakeBlockedTxUSB(ptrUsbQueue);                     // we have just emptied buffer content so inform waiting transmitter task
    #endif
    #if defined USB_TX_MESSAGE_MODE
                    if (ptrUsbQueue->endpoint_control->messageQueue != 0) { // {32} if in message mode
                        if ((tx_queue->ptrStart + tx_queue->usSent) > (ptrUsbQueue->USB_queue.buffer_end - tx_queue->usMax_frame_length)) { // if there is less that a complete endpoint size in the circular buffer
                            if (usRemaining > (ptrUsbQueue->USB_queue.buffer_end - (tx_queue->ptrStart + tx_queue->usSent))) { // if the remaining message required a message wrap-around
                                tx_queue->ptrStart = (ptrUsbQueue->USB_queue.QUEbuffer - tx_queue->usSent); // set the start pointer to compensate for the message wrap-around
                            }
                        }
                    }
    #endif
                }
                FNSEND_USB_DATA((tx_queue->ptrStart + tx_queue->usSent), usDataLength, iEndpoint, ptrUSB_HW); // transmit next buffer
                fnPushLastLength(ptrUsbQueue, usDataLength);             // save last length for use later
                tx_queue->usSent += usDataLength;                        // total frame length in progress
    #if defined USB_HOST_SUPPORT                                         // {29}
                if (_USB_HOST_MODE()) {                                  // if host data has been prepared
                    fnHostReleaseBuffer(iEndpoint, OUT_PID, ptrUSB_HW); // allow host to release prepared data
                }
    #endif
            }
        }
        break;
#if defined USB_DEVICE_SUPPORT                                           // {29} setup only received by device
    case USB_SETUP_FRAME:                                                // a setup token has been received (usually during enumeration - always end point 0)
        {
            int iReturn;
            unsigned char ucFirstByte = GET_USB_DATA();
            unsigned char ucRequest;                                     // {1}
            usb_endpoint_control[iChannel]->usCompleteMessage = 0;       // a setup frame will cancel any existing transmissions so ensure the count is reset
            SET_CONTROL_DIRECTION(ptrUSB_HW, ucFirstByte);               // {2} prepare the direction of endpoint response
            switch ((ucFirstByte & ~STANDARD_DEVICE_TO_HOST)) {          // set up packet recipient and type, masking the direction
            case REQUEST_DEVICE_STANDARD:                                // 0x00 - bmRequestType.Recipient == Device, bmRequestType.Type == Standard
                ucRequest = GET_USB_DATA();                              // {1}
                switch (ucRequest) {                                     // bRequest
                case USB_REQUEST_GET_DESCRIPTOR:                         // 0x06
                    {
                        unsigned short wLength;
                        unsigned short wIndex;
                        unsigned short wValue = GET_USB_DATA();          // get descriptor type in little-endian format
                        wValue |= (GET_USB_DATA() << 8);
                        wIndex  = GET_USB_DATA();                        // language-ID
                        wIndex |= (GET_USB_DATA() << 8);
                        wLength = GET_USB_DATA();
                        wLength |= (GET_USB_DATA_NO_INC() << 8);         // the length that the host can accept
                        CLEAR_SETUP(ptrUSB_HW);                          // {2}
                        switch (wValue & 0xff00) {
                        case STANDARD_DEVICE_DESCRIPTOR:                 // 0x01xx
                            {
                                unsigned char *ptrDecr;
                                unsigned short usLength;
                                ptrDecr = fnGetUSB_device_descriptor(&usLength, iChannel); // the application must return the device descriptor
                                fnPrepareOutData(DEVICE_DATA  ptrDecr, usLength, wLength, 0, ptrUSB_HW); // send the device descriptor back to the host on the control endpoint
                            }
                            break;

                        case STANDARD_CONFIG_DESCRIPTOR:                 // 0x02xx
                            {
                                unsigned char *ptrDecr;
                                unsigned short usLength;
                                ptrDecr = fnGetUSB_config_descriptor(&usLength, iChannel); // the application must return the configuration descriptor
                                fnPrepareOutData(DEVICE_DATA  ptrDecr, usLength, wLength, 0, ptrUSB_HW); // send the configuration descriptor back to the host on the control endpoint
                            }
                            break;

                        case STANDARD_STRING_DESCRIPTOR:                 // 0x03xx optional
                            {
            #if defined USB_STRING_OPTION
                                unsigned short usLength;
                                unsigned char *ptrStart;
                #if defined MICROSOFT_OS_STRING_DESCRIPTOR
                                if (((unsigned char)wValue == (unsigned char)0xee) && (wLength == 18)) { // the OS string descriptor with the expected length
                                    #define MOS_VENDOR_CODE 1            // never set to 0
                                    static const unsigned char OSDescriptor[]  = {18,  DESCRIPTOR_TYPE_STRING, 'M',0, 'S',0, 'F',0, 'T',0, '1',0, '0',0, '0',0, MOS_VENDOR_CODE,0}; // the MOS V1.00 response with vendor code
                                    usLength = OSDescriptor[0];
                                    ptrStart =  (unsigned char *)OSDescriptor;
                                }
                                else {
                #endif
                                    ptrStart = fnGetUSB_string_entry((unsigned char)wValue, &usLength, iChannel); // {33} get the corresponding string (passing its index [0..255])
                                    if (ptrStart == 0) {                 // check for invalid string index receptions
                                        return STALL_ENDPOINT;           // stall endpoint - this happens, for example, when the String Microsoft OS string is requested
                                    }
                #if defined MICROSOFT_OS_STRING_DESCRIPTOR
                                }
                #endif
                                fnPrepareOutData(DEVICE_DATA  ptrStart, usLength, wLength, 0, ptrUSB_HW);
            #else
                                return STALL_ENDPOINT;                   // no string support, stall endpoint
            #endif
                            }
                            break;
            #if USB_SPEC_VERSION == USB_SPEC_VERSION_2_0                 // {24} if USB1.1 is reported the host doesn't request this descriptor
                        case DEVICE_QUALIFIER_DESCRIPTOR:                // 0x06xx we presently support USB2.0 devices which can only operate at full speed so the answer is fixed
                            {
                                static const USB_DEVICE_QUALIFIER_DESCRIPTOR qualifier_descriptor = {
                                    DEVICE_QUALIFIER_DESCRIPTOR_LENGTH,  // device qualifier descriptor length (0x0a)
                                    DESCRIPTOR_DEVICE_QUALIFIER,
                                    {LITTLE_SHORT_WORD_BYTES(USB_SPEC_VERSION_2_0)}, // always USB2.0
                                    DEVICE_CLASS_AT_INTERFACE,           // device class, sub-class and protocol
                                    64,                                  // size of endpoint reception buffer - must be 64
                                    NUMBER_OF_POSSIBLE_CONFIGURATIONS,   // number of configurations possible
                                    0
                                };
                                fnPrepareOutData(DEVICE_DATA  (unsigned char *)&qualifier_descriptor, sizeof(qualifier_descriptor), wLength, 0, ptrUSB_HW);
                            }
                            break;
            #endif
                        default:
                            if (usb_endpoint_control[iChannel]->usb_callback == 0) {
                                return STALL_ENDPOINT;                   // stall on any unknown requests if no application support
                            }
            #if defined USB_FIFO                                         // {1}
                            ptrData = fnReadUSB_FIFO(ptrData, 2, 6);     // extract remaining data from FIFO input
                            *ptrData++ = ucFirstByte;                    // add the read bytes to the intermediate buffer
                            *ptrData++ = (unsigned char)wValue;
                            *ptrData++ = (unsigned char)(wValue >> 8);
                            *ptrData++ = (unsigned char)wIndex;
                            *ptrData++ = (unsigned char)(wIndex >> 8);
                            *ptrData++ = (unsigned char)wLength;
                            *ptrData++ = (unsigned char)(wLength >> 8);
            #endif
                            CLEAR_SETUP(ptrUSB_HW);                      // {2}
                            return (usb_endpoint_control[iChannel]->usb_callback((ptrData - 6), ptrUSB_HW->usLength, SETUP_DATA_RECEPTION, iChannel)); // allow the USB application to handle any non-standard (or non-supported) requests
                        }
                    }
                    break;

                case USB_REQUEST_SET_ADDRESS:                            // 0x05 - we are receiving an address to use
                    ptrUSB_HW->ucUSBAddress = GET_USB_DATA_NO_INC();     // this is the address which we will commit once the status stage has successfully terminated
                    CLEAR_SETUP(ptrUSB_HW);                              // {2}
                    return TERMINATE_ZERO_DATA;

                case USB_REQUEST_SET_CONFIGURATION:                      // 0x09 - activate a configuration
                    ucActiveConfiguration[iChannel] = GET_USB_DATA_NO_INC(); // save the active configuration
                    if (fnSetUSBConfigState(USB_CONFIG_ACTIVATE, ucActiveConfiguration[iChannel], iChannel) != 0) {
                        ucActiveConfiguration[iChannel] = 0;
                        return STALL_ENDPOINT;                           // invalid configuration so stall
                    }
                    VALIDATE_NEW_CONFIGURATION();                        // {16}
            #if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                    fnUSB_message((E_USB_ACTIVATE_CONFIGURATION + iChannel), ptrData, 1, usb_endpoint_control[iChannel]->event_task); // inform the task of successful activation of this configuration
            #else
                    fnUSB_message(E_USB_ACTIVATE_CONFIGURATION, ptrData, 1, usb_endpoint_control[0]->event_task); // inform the task of successful activation of this configuration
            #endif
                    CLEAR_SETUP(ptrUSB_HW);                              // {2}
                    return TERMINATE_ZERO_DATA;

                case USB_REQUEST_GET_CONFIGURATION:                      // 0x08 - answer with presently active configuration
                    CLEAR_SETUP(ptrUSB_HW);                              // {4}
                    fnPrepareOutData(DEVICE_DATA  &ucActiveConfiguration[iChannel], sizeof(ucActiveConfiguration[iChannel]), sizeof(ucActiveConfiguration[iChannel]), 0, ptrUSB_HW);
                    break;

                case USB_REQUEST_GET_STATUS:                             // 0x00
                    CLEAR_SETUP(ptrUSB_HW);                              // {4}
                    fnPrepareOutData(DEVICE_DATA  (unsigned char *)&usDeviceStatus[iChannel], sizeof(usDeviceStatus[iChannel]), sizeof(usDeviceStatus[iChannel]), 0, ptrUSB_HW);
                    break;

              //case USB_REQUEST_CLEAR_FEATURE:                          // 0x01 - not supported
              //case USB_REQUEST_SET_FEATURE:                            // 0x03
              //case USB_REQUEST_SET_DESCRIPTOR:                         // 0x07
              //case USB_REQUEST_SYNCH_FRAME:                            // 0x0c
                default:
                    if ((usb_endpoint_control[iChannel]->usb_callback) == 0) {
                        return STALL_ENDPOINT;                           // stall on any unknown requests if no application support
                    }
            #if defined USB_FIFO                                         // {1}
                    ptrData = fnReadUSB_FIFO(ptrData, 6, 2);             // extract remaining data from FIFO input
                    *ptrData++ = ucFirstByte;                            // add the read bytes to the intermediate buffer
                    *ptrData++ = ucRequest;
            #endif
                    CLEAR_SETUP(ptrUSB_HW);                              // {2}
                    return (usb_endpoint_control[iChannel]->usb_callback((ptrData - 2), ptrUSB_HW->usLength, SETUP_DATA_RECEPTION, iChannel)); // allow the USB application to handle any non-standard (or non-supported) requests
                }
                break;

            case REQUEST_INTERFACE_STANDARD:                             // 0x01 - bmRequestType.Recipient == Interface, bmRequestType.Type == Standard
                ucRequest = GET_USB_DATA();                              // {1}
                switch (ucRequest) {                                     // bRequest
                case USB_REQUEST_SET_INTERFACE:                          // 0x0b - set an alternative interface
                    if (ucActiveConfiguration[iChannel] == 0) {
                        return STALL_ENDPOINT;                           // stall if not configured
                    }
                    else {
                        unsigned short usReferences[2];
                        fnExtract(ptrData, (INDEX_INDEX | VALUE_INDEX), usReferences); // extract the interface and alternative interface from the request
                        fnSetUSBInterfaceState(USB_INTERFACE_DEACTIVATE, (unsigned char)usReferences[1], 0xff, iChannel); // deactivate all endpoints on this interface
                        if (fnSetUSBInterfaceState(USB_INTERFACE_ACTIVATE, (unsigned char)usReferences[1], (unsigned char)usReferences[0], iChannel) != 0) { // activate this alternative interface, adjusting endpoint sizes where needed
                            return STALL_ENDPOINT;                       // stall if alternative interface not valid
                        }
            #if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
                        else {
                            fnUSB_message((E_USB_SET_INTERFACE + iChannel), (unsigned char *)&usReferences, sizeof(usReferences), usb_endpoint_control[iChannel]->event_task);
                        }
            #endif
                    }
                    CLEAR_SETUP(ptrUSB_HW);                              // {4}
                    return TERMINATE_ZERO_DATA;

                case USB_REQUEST_GET_INTERFACE:                          // 0x0a - request alternative interface
                    if (ucActiveConfiguration[iChannel] == 0) {
                        return STALL_ENDPOINT;                           // stall if not configured
                    }
                    else {
                        unsigned short alternative_interface;
                        const USB_INTERFACE_DESCRIPTOR *interface_descriptor;
                        fnExtract(ptrData, (INDEX_INDEX), &alternative_interface); // extract the interface and alternative interface from the request
                        interface_descriptor = (const USB_INTERFACE_DESCRIPTOR *)fnSetUSBInterfaceState(USB_INTERFACE_ALTERNATE, (unsigned char)alternative_interface, 0, iChannel); // get alternative interface
                        if (interface_descriptor == 0) {
                            return STALL_ENDPOINT;                       // stall if interface is not valid
                        }
                        CLEAR_SETUP(ptrUSB_HW);                          // {4}
                        fnPrepareOutData(DEVICE_DATA  (unsigned char *)&interface_descriptor->bAlternateSetting, 1, 1, 0, ptrUSB_HW);
                    }
                    break;

                case USB_REQUEST_GET_STATUS:                             // 0x00 - get status of interface
                    if (ucActiveConfiguration[iChannel] == 0) {          // only allowed when configured
                        return STALL_ENDPOINT;
                    }
                    CLEAR_SETUP(ptrUSB_HW);                              // {4}
                    fnPrepareOutData(DEVICE_DATA  (unsigned char *)&usInterfaceStatus, sizeof(usInterfaceStatus), sizeof(usInterfaceStatus), 0, ptrUSB_HW);
                    break;

                default:
                    if (usb_endpoint_control[iChannel]->usb_callback == 0) {
                        return STALL_ENDPOINT;                           // stall on any unknown requests if no applicatin support
                    }
            #if defined USB_FIFO                                         // {1}
                    ptrData = fnReadUSB_FIFO(ptrData, 6, 2);             // extract remaining data from FIFO input
                    *ptrData++ = ucFirstByte;                            // add the read bytes to the intermediate buffer
                    *ptrData++ = ucRequest;
            #endif
                    CLEAR_SETUP(ptrUSB_HW);                              // {2}
                    return (usb_endpoint_control[iChannel]->usb_callback((ptrData - 2), ptrUSB_HW->usLength, SETUP_DATA_RECEPTION, iChannel)); // allow the USB application to handle any non-standard (or non-supported) requests
                }
                break;

            case REQUEST_ENDPOINT_STANDARD:                              // 0x02
                ucRequest = GET_USB_DATA();                              // {1}
                switch (ucRequest) {                                     // bRequest
                case USB_REQUEST_CLEAR_FEATURE:                          // {13}
                case USB_REQUEST_GET_STATUS:                             // 0x00 - get status of endpoint
                    {
                        USB_ENDPOINT *tx_queue = usb_endpoint_control[iChannel];
                        unsigned short usEndpoint;
                        fnExtract(ptrData, (INDEX_INDEX), &usEndpoint);  // extract the endpoint from the request details
                        if ((ucActiveConfiguration[iChannel] == 0) && (usEndpoint != 0)) { // only endpoint 0 allowed when not configured
                            return STALL_ENDPOINT;
                        }
                        tx_queue += (usEndpoint & 0x1f);                 // {13}
                        CLEAR_SETUP(ptrUSB_HW);                          // {4}
                        if (ucRequest == USB_REQUEST_CLEAR_FEATURE) {    // {13}
                            if ((tx_queue->ucState & USB_ENDPOINT_STALLED) != 0) {
                                tx_queue->ucState &= ~USB_ENDPOINT_STALLED;
                                if ((usb_endpoint_control[iChannel]->usb_callback) != 0) {
                                    unsigned char ucEndpoint = (unsigned char)usEndpoint;
                                    fnUnhaltEndpoint(ucEndpoint);        // clear the stall state in the hardware
                                    return (usb_endpoint_control[iChannel]->usb_callback(&ucEndpoint, 0, ENDPOINT_CLEARED, iChannel)); // inform that the endpoint has been freed
                                }                                
                            }
                            break;
                        }
                        if ((tx_queue->ucState & USB_ENDPOINT_STALLED) != 0) {
                            fnPrepareOutData(DEVICE_DATA  (unsigned char *)&usEndpointStalled, sizeof(usEndpointStalled), sizeof(usEndpointStalled), 0, ptrUSB_HW);
                        }
                        else {
                            fnPrepareOutData(DEVICE_DATA  (unsigned char *)&usEndpointOK, sizeof(usEndpointOK), sizeof(usEndpointOK), 0, ptrUSB_HW);
                        }
                    }
                    break;              
                default:
                    if (usb_endpoint_control[iChannel]->usb_callback == 0) {
                        return STALL_ENDPOINT;                           // stall on any unknown requests if no application support
                    }
            #if defined USB_FIFO                                         // {1}
                    ptrData = fnReadUSB_FIFO(ptrData, 6, 2);             // extract remaining data from FIFO input
                    *ptrData++ = ucFirstByte;                            // add the read bytes to the intermediate buffer
                    *ptrData++ = ucRequest;
            #endif
                    CLEAR_SETUP(ptrUSB_HW);                              // {2}
                    return (usb_endpoint_control[iChannel]->usb_callback((ptrData - 2), 8, ENDPOINT_REQUEST_TYPE, iChannel)); // allow the USB application to handle any non-standard (or non-supported) requests
                }
                break;
            #if defined MICROSOFT_OS_STRING_DESCRIPTOR
            case REQUEST_DEVICE_VENDOR:
                #if defined USB_FIFO
                ptrData = fnReadUSB_FIFO(ptrData, 7, 1);                 // extract remaining data from FIFO input
                *ptrData++ = ucFirstByte;                                // add the first byte to the intermediate buffer
                #endif
                if (*ptrData == MOS_VENDOR_CODE) {                       // Microsoft OS feature descriptor
                  //int iInterface = *(ptrData + 1);
                    unsigned char ucIndex = *(ptrData + 3);
                    unsigned short wLength = (*(ptrData + 5) | (*(ptrData + 6) << 8));
                    switch (ucIndex) {
                    case 0x04:                                           // extended compat ID
                #if defined USB_CDC_RNDIS
                        {
                            static const unsigned char RNDIS_compat[]  = {LITTLE_LONG_WORD_BYTES(0x28),  LITTLE_SHORT_WORD_BYTES(0x0100), LITTLE_SHORT_WORD_BYTES(0x4), 1, 0,0,0,0,0,0,0, 0, 1, 'R','N','D','I','S',0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0};
                            fnPrepareOutData(DEVICE_DATA  (unsigned char *)RNDIS_compat, 0x28, wLength, 0, ptrUSB_HW);
                        }
                        break;
                #endif
                    case 0x01:                                           // genre
                    case 0x05:                                           // extended properties
                        return STALL_ENDPOINT;                           // stall on any unknown requests if no application support
                    }
                    break;
                }
            #endif
            default:                                                     // let the application call back handle any class specific requests
                if (usb_endpoint_control[iChannel]->usb_callback == 0) {
                    return STALL_ENDPOINT;                               // stall on any unknown requests if no application support
                }
            #if defined USB_FIFO && !defined MICROSOFT_OS_STRING_DESCRIPTOR // {1}
                ptrData = fnReadUSB_FIFO(ptrData, 7, 1);                 // extract remaining data from FIFO input
                *ptrData++ = ucFirstByte;                                // add the first byte to the intermediate buffer
            #endif
                CLEAR_SETUP(ptrUSB_HW);                                  // {2}
                if (DIRECTION_IN()) {                                    // {30} if we are handling a class IN request
                    prtUSBcontrolSetup[iChannel] = (USB_SETUP_HEADER *)(ptrData - 1); // set a pointer to the setup packet that will be valid during the application's handling
                }
                iReturn = usb_endpoint_control[iChannel]->usb_callback((ptrData - 1), ptrUSB_HW->usLength, SETUP_DATA_RECEPTION, iChannel); // allow the USB application to handle any non-standard (or non-supported) requests
                prtUSBcontrolSetup[iChannel] = 0;                        // {30} invalidate the setup pointer after application handling has completed
                return iReturn;
            }
        }
        break;
#endif
    case USB_CONTROL_OUT_FRAME:
      //fnDebugMsg("O");
        if (ptrUSB_HW->usLength == 0) {                                  // a zero data packet status
          //fnDebugMsg("0");
#if defined USB_HOST_SUPPORT                                             // {29}
            if ((iEndpoint == 0) && (ucRequestType[iChannel] != HOST_ENUMERATION_IDLE) && (ucRequestType[iChannel] < HOST_ENUMERATION_CONFIGURED)) {
              //fnDebugMsg("?");
                return fnHostEnumeration(iEndpoint, ZERO_DATA_TERMINATOR, ptrUSB_HW); // handle the enumeration process
            }
#endif
          //fnDebugMsg("*");
            return (fnEndpointData(iEndpoint, ptrData, 0, STATUS_STAGE_RECEPTION, ptrUSB_HW));
        }
      //fnDebugMsg("1");
        // Fall through intentionally
        //
    case USB_OUT_FRAME:
        return (fnEndpointData(iEndpoint, ptrData, ptrUSB_HW->usLength, OUT_DATA_RECEPTION, ptrUSB_HW));

    case USB_RESET_DETECTED:                                             // USB bus reset detected
        if (ptrUSB_HW->ucUSBAddress != 0) {                              // if we are past the address definition phase
            fnInterruptMessage(usb_endpoint_control[iChannel]->event_task, (EVENT_USB_RESET + iChannel)); // inform of the occurrence
        }
        fnSetUSBConfigState(USB_CONFIG_ACTIVATE, 0, iChannel);           // set zero configuration to deactivate all endpoint control structures
        _SIM_USB(iChannel, USB_SIM_RESET, 0, 0);                         // simulate reset
        break;

    case USB_SUSPEND_DETECTED:
        fnInterruptMessage(usb_endpoint_control[iChannel]->event_task, (EVENT_USB_SUSPEND + iChannel)); // inform of the occurrence - it is up to the application to reduce power to conform to the specification if required
        _SIM_USB(iChannel, USB_SIM_SUSPEND, 0, 0);                       // simulate reset
        break;
    case USB_RESUME_DETECTED:
        fnInterruptMessage(usb_endpoint_control[iChannel]->event_task, (EVENT_USB_RESUME + iChannel)); // inform of the occurrence - the application can adjust power consumption accordingly
        break;
    case USB_DATA_REPEAT:                                                // a repeated data reception occurs only when our ACK has been lost. This should be extremely rare.
        break;                                                           // we do nothing, except free the buffer - it could be counted if required
#if defined USB_HOST_SUPPORT                                             // {29}
    case USB_DEVICE_DETECTED:                                            // device has been connected to the USB bus, reset has been completed and SOFs are being sent
        ucRequestType[iChannel] = HOST_ENUMERATION_IDLE;                 // initialise host state on detection of device (bus reset has been executed)
        // Fall through intentionally
        //
    case USB_HOST_SOF:                                                   // start of frame event
        return (fnHostEnumeration(iEndpoint, DEVICE_DETECTED, ptrUSB_HW)); // initiate host state-event-machine
    case USB_DEVICE_REMOVED:                                             // device has been removed from the bus
        {
            USB_ENDPOINT *tx_queue = usb_endpoint_control[iChannel];
            tx_queue->usCompleteMessage = 0;                             // ensure that the transmitter queue is free
            tx_queue->ucFIFO_depth = 0;
            fnInterruptMessage(usb_endpoint_control[iChannel]->event_task, (EVENT_USB_REMOVAL + iChannel));
            ucRequestType[iChannel] = HOST_ENUMERATION_IDLE;
            _SIM_USB(iChannel, USB_SIM_RESET, 0, 0);
        }
        break;
    case USB_HOST_STALL_DETECTED:                                        // the host has detected a stall, which means that intervention is required to clear the stalled endpoint
        {
            unsigned char ucEndpoint = (unsigned char)iEndpoint;
            fnUSB_message((E_USB_DEVICE_STALLED + iChannel), &ucEndpoint, 1, usb_endpoint_control[iChannel]->event_task); // inform the owner task so that it can decide what to do
        }
        break;
    case USB_DEVICE_TIMEOUT:                                             // {35}
        ucRequestType[iChannel]--;                                       // set previous state
        return (fnHostEnumeration(iEndpoint, 0, ptrUSB_HW));             // repeat previous setup request since there was no answer
    case USB_HOST_ACK_PID_DETECTED:                                      // error code 0
    case USB_HOST_NACK_PID_DETECTED:                                     // error code 1
    case USB_HOST_BUS_TIMEOUT_DETECTED:                                  // error code 2
    case USB_HOST_DATA_ERROR_DETECTED:                                   // error code 3
        {
            unsigned char ucErrorNumber = (unsigned char)((ucType - USB_HOST_ACK_PID_DETECTED) | (iEndpoint << 4));
            fnUSB_message(E_USB_HOST_ERROR_REPORT, &ucErrorNumber, 1, usb_endpoint_control[iChannel]->event_task); // inform the owner task so that it can decide what to do
        }
        break;
#endif
    }
    return BUFFER_CONSUMED;
}


// This is the generic OUT data handler
//
extern int fnEndpointData(int iEndpoint, unsigned char *ptrData, unsigned short usLength, int iControl, USB_HW *ptrUSB_HW)
{
#if (defined USB_HS_INTERFACE && defined USB_FS_INTERFACE) || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    register int iChannel = ptrUSB_HW->ucController;
#else
    register int iChannel = 0;
#endif
    USB_ENDPOINT *usb_endpoint_queue = usb_endpoint_control[iChannel];
    USBQUE *ptrQueue;
    #if defined USB_HOST_SUPPORT                                         // {29}
    int iHostUserData = 0;
    if ((iEndpoint == 0) && (ucRequestType[iChannel] != HOST_ENUMERATION_IDLE)) { // data on control endpoint 0 and the generic USB host is expecting a response
        unsigned char *ptrResponseDestination;
        switch (ucRequestType[iChannel]) {
        case HOST_ENUMERATION_STANDARD_DEVICE_DESCRIPTOR:                // receiving data for standard device descriptor
            ptrResponseDestination = (unsigned char *)&device_descriptor[iChannel];// the storage location for this descriptor
            break;
        case HOST_ENUMERATION_CONFIGURATION_DESCRIPTOR:                  // receiving data for configuration descriptor
            ptrResponseDestination = (unsigned char *)&config_descriptor[iChannel]; // the storage location for this descriptor
            break;
        #if defined USB_HOST_HUB
        case HOST_ENUMERATION_SET_CONFIGURATION:                         // control endpoint data can be received in this state when enumerating a hub
        #endif
        case HOST_ENUMERATION_CONFIGURED:                                // user requested data being received
            if ((usLength == 0) && (fnUnstallAck(ptrUSB_HW) != 0)) {
                return BUFFER_CONSUMED;                                  // zero data terminator for an unstall endpoint setup received
            }
            iHostUserData = 1;
            goto _user_data_reception;                                   // the data is for the user
        #if defined USB_STRING_OPTION
        case HOST_ENUMERATION_REQUEST_STRING:                            // receiving strings
            ptrResponseDestination = (unsigned char *)&ucStringList[iStringReference[iChannel]]; // the storage location for this string data
            break;
        #endif
        case HOST_ENUMERATION_CONFIGURED_IDLE:
            fnUnstallAck(ptrUSB_HW);                                     // only a confirmation of an uninstalled endpoint is expected in this state
            return BUFFER_CONSUMED;
        default:
        #if defined _WINDOWS        	
            _EXCEPTION("Unexpected data reception on endpoint 0!!");
        #endif            
            return BUFFER_CONSUMED;                                      // this will not happen normally
        }
        if (usLength > usRequestLengthRemaining[iChannel]) {             // cut down over-sized responses
            usLength = usRequestLengthRemaining[iChannel];
        }
        uMemcpy((ptrResponseDestination + usRequestingReceived[iChannel]), ptrData, usLength); // copy received device descriptor content
        usRequestLengthRemaining[iChannel] -= usLength;
        if ((usRequestLengthRemaining[iChannel] == 0) || (usLength != device_descriptor[iChannel].bMaxPacketSize0)) { // if information is complete
            int iReturn;
            usRequestingReceived[iChannel] += usLength;
            iReturn = fnHostEnumeration(iEndpoint, DATA_RECEPTION_COMPLETE, ptrUSB_HW); // handle the enumeration process
            usRequestingReceived[iChannel] = 0;
            return iReturn;
        }
        else {
            usRequestingReceived[iChannel] += usLength;
        }
        return INITIATE_IN_TOKEN;                                        // reception buffer has been consumed but we expect more
    }
_user_data_reception:
    #endif
    usb_endpoint_queue += iEndpoint;
    if ((usb_endpoint_queue->usParameters & USB_ENDPOINT_ZERO_COPY_OUT) != 0) { // {31} if this OUT endpoint is using a zero copy buffer
        QUEQUE *ptrQueue = ((QUEQUE *)(usb_endpoint_queue->ptrEndpointOutCtr));
        unsigned char *ptrNextPut;
        ptrQueue->put += usb_endpoint_queue->usMax_frame_length;
        iControl = ptrQueue->new_chars;
        if (ptrQueue->put >= ptrQueue->buffer_end) {
            ptrQueue->put = ptrQueue->QUEbuffer;
            ptrQueue->new_chars = 0;
        }
        else {
            ptrQueue->new_chars++;
        }
        if (ptrUSB_HW->ptrRxDatBuffer != 0) {
            ptrNextPut = ptrQueue->put;
            ptrNextPut += usb_endpoint_queue->usMax_frame_length;
            if (ptrNextPut >= ptrQueue->buffer_end) {
                ptrNextPut = ptrQueue->QUEbuffer;
            }
            *ptrUSB_HW->ptrRxDatBuffer = ptrNextPut;                     // alter the driver's next buffer pointer
        }
    }
    if (usb_endpoint_queue->usb_callback != 0) {                         // if there is a callback defined to handle this endpoint
        int iReturn;                                                     // {12} 
    #if defined USB_FIFO                                                 // {1}
        ptrData = fnReadUSB_FIFO(ptrData, usLength, 0);                  // extract data from FIFO input to an intermediate working buffer
    #endif       
        iReturn = usb_endpoint_queue->usb_callback(ptrData, usLength, iControl, iChannel); // call the callback
        if (iReturn != TRANSPARENT_CALLBACK) {                           // {12} allow use of buffered info if the callback doesn't want to handle the processing
    #if defined USB_HOST_SUPPORT                                         // {29}
            if (iHostUserData != 0) {                                    // host user data has been received
                if (TERMINATE_ZERO_DATA == iReturn) {                    // all data has been received
                    return fnHostEnumeration(iEndpoint, ZERO_DATA_TERMINATOR, ptrUSB_HW); // handle the host state-event-machine
                }
            }
    #endif
            return iReturn;
        }
    }
    if (usb_endpoint_queue->ptrEndpointOutCtr == 0) {
        return BUFFER_CONSUMED;                                          // neither a call back nor a buffer available - data ignored
    }
    if (usb_endpoint_queue->event_task != 0) {
        uTaskerStateChange(usb_endpoint_queue->event_task, UTASKER_ACTIVATE); // wake task monitoring this endpoint
    }
    ptrQueue = (USBQUE *)usb_endpoint_queue->ptrEndpointOutCtr;
    #if defined USB_FIFO                                                 // {18}
    if (usb_endpoint_queue->usb_callback != 0) {                         // {12} the FIFO has already been extracted
        fnFillBuf(&ptrQueue->USB_queue, ptrData, (QUEUE_TRANSFER)usLength); // unconditionally copy to input buffer since there will be space to accept it
        if ((ptrQueue->USB_queue.buf_length - ptrQueue->USB_queue.chars) < usb_endpoint_queue->usMax_frame_length) { // {18} check whether there is space to receive maximum following data
            usb_endpoint_queue->ucState |= USB_ENDPOINT_BLOCKED;         // mark that this FIFO endpoint is critically full and needs to be stopped
            return CRITICAL_OUT;                                         // inform that the input queue content is critical and no further data should be accepted at the moment
        }
    }
    else {
        if ((usb_endpoint_queue->ucState & USB_ENDPOINT_BLOCKED) != 0) {
            return MAINTAIN_OWNERSHIP;                                   // the input buffer is presently blocked so maintain the USB buffer until it is freed by the application
        }
        if ((ptrQueue->USB_queue.buf_length - ptrQueue->USB_queue.chars) >= usLength) { // is there enough space to put this data packet in to the input buffer?
            fnFillBuf_FIFO(&ptrQueue->USB_queue, ptrData, (QUEUE_TRANSFER)usLength); // copy to input buffer, bypassing the intermediate memory
        }
        else {
            usb_endpoint_queue->ucState |= USB_ENDPOINT_BLOCKED;         // mark that this endpoint is presently blocked
            return MAINTAIN_OWNERSHIP;                                   // this buffer could not be freed so keep it until it is cleared
        }
    }
    #elif defined USB_FIFO_INTERMEDIATE_BUFFER                           // {19}
    if (usb_endpoint_queue->usb_callback != 0) {                         // the FIFO has already been extracted
        fnFillBuf(&ptrQueue->USB_queue, ptrData, (QUEUE_TRANSFER)usLength); // unconditionally copy to input buffer since there will be space to accept it
        if ((ptrQueue->USB_queue.buf_length - ptrQueue->USB_queue.chars) < usb_endpoint_queue->usMax_frame_length) { // check whether there is space to receive maximum following data
            usb_endpoint_queue->ucState |= USB_ENDPOINT_BLOCKED;         // mark that this FIFO endpoint is critically full and needs to be stopped
            return CRITICAL_OUT;                                         // inform that the input queue content is critical and no further data should be accepted at the moment
        }
    }
    else {
        if ((ptrQueue->USB_queue.buf_length - ptrQueue->USB_queue.chars) >= usLength) { // is there enough space to put this data packet in to the input buffer?
            fnFillBuf(&ptrQueue->USB_queue, ptrData, (QUEUE_TRANSFER)usLength); // copy to input buffer
        }
        else {
            usb_endpoint_queue->ucState |= USB_ENDPOINT_BLOCKED;         // mark that this endpoint is presently blocked
            return MAINTAIN_OWNERSHIP;                                   // this buffer could not be freed so keep it until it is cleared
        }
    }
    #else
    if ((usb_endpoint_queue->usParameters & USB_ENDPOINT_ZERO_COPY_OUT) != 0) { // {31}
        return BUFFER_CONSUMED;
    }
    if ((usb_endpoint_queue->ucState & USB_ENDPOINT_BLOCKED) != 0) {
        return MAINTAIN_OWNERSHIP;                                       // the input buffer is presently blocked so maintain the USB buffer until it is freed by the application
    }
    if ((ptrQueue->USB_queue.buf_length - ptrQueue->USB_queue.chars) >= usLength) { // is there enough space to put this data packet in to the input buffer?
        fnFillBuf(&ptrQueue->USB_queue, ptrData, (QUEUE_TRANSFER)usLength); // copy to input buffer
        #if defined USB_HOST_SUPPORT
        if ((usb_endpoint_queue->usParameters & USB_ENDPOINT_AUTO_IN) != 0) { // {40} if the IN polling should be restarted automatically
            return INITIATE_IN_TOKEN;
        }
        #endif
    }
    else {
        usb_endpoint_queue->ucState |= USB_ENDPOINT_BLOCKED;             // mark that this endpoint is presently blocked
        return MAINTAIN_OWNERSHIP;                                       // this buffer could not be freed so keep it until it is cleared
    }
    #endif
    return BUFFER_CONSUMED;                                              // OK - data consumed so free buffer for more
}

// Called to activated an endpoint, calling the hardware activation after setting endpoint variables
//
static void fnActivateEndpoint(const USB_ENDPOINT_DESCRIPTOR *ptrEndpointDesc, unsigned short usMaxLength, int iChannel)
{
    unsigned short usEndpointLength = ptrEndpointDesc->wMaxPacketSize[0];
    unsigned char ucEndpointRef = (ptrEndpointDesc->bEndpointAddress & 0x7f);
    unsigned char ucEndpointType = (ptrEndpointDesc->bmAttributes | (ptrEndpointDesc->bEndpointAddress & IN_ENDPOINT));
    USB_ENDPOINT *usb_endpoint_queue = (usb_endpoint_control[iChannel] + ucEndpointRef);

    usEndpointLength |= (ptrEndpointDesc->wMaxPacketSize[1] << 8);
    fnActivateHWEndpoint(ucEndpointType, USB_CONTROLLER_ENDPOINT(ucEndpointRef, iChannel), usEndpointLength, usMaxLength, usb_endpoint_queue); // activate the hardware based on the details
    if (ptrEndpointDesc->bmAttributes == ENDPOINT_CONTROL) {
        usb_endpoint_queue->ucState = (USB_ENDPOINT_ACTIVE | USB_CONTROL_ENDPOINT);
    }
    else {
        usb_endpoint_queue->ucState = (unsigned char)(USB_ENDPOINT_ACTIVE | usb_endpoint_queue->usParameters); // {11} set active state plus any extra parameters
    }
}

// Called to deactivate an endpoint, calling hardware deactivation as well as resetting variables and flushing queues
//
static void fnDeactivateEndpoint(const USB_ENDPOINT_DESCRIPTOR *ptrEndpointDesc, int iChannel)
{
    unsigned char ucEndpointRef = (ptrEndpointDesc->bEndpointAddress & 0x7f);
    USB_ENDPOINT *usb_endpoint_queue = (usb_endpoint_control[iChannel] + ucEndpointRef);
    USBQUE *ptrUsbQueue = (USBQUE *)(usb_endpoint_queue->ptrEndpointInCtr);
    fnActivateHWEndpoint(ENDPOINT_DISABLE, USB_CONTROLLER_ENDPOINT(ucEndpointRef, iChannel), 0, 0, 0); // deactivate the hardware
    usb_endpoint_queue->ucState = 0;                                     // return to addressed state
    usb_endpoint_queue->usCompleteMessage = 0;                           // stop any transmission
    usb_endpoint_queue->ucFIFO_depth = 0;                                // {7}
    if (ptrUsbQueue != 0) {                                              // flush any input queue
        ptrUsbQueue->USB_queue.chars = 0;
        ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.put = ptrUsbQueue->USB_queue.QUEbuffer;
    }
    ptrUsbQueue = (USBQUE *)(usb_endpoint_queue->ptrEndpointOutCtr);
    if (ptrUsbQueue != 0) {                                              // flush any output queue
        ptrUsbQueue->USB_queue.chars = 0;
        ptrUsbQueue->USB_queue.get = ptrUsbQueue->USB_queue.put = ptrUsbQueue->USB_queue.QUEbuffer;
    }
}

// Routine for building a temporary queue of endpoints and their required buffer characteristics
//
static int fnAdd_to_EndpointList(USB_ENDPOINT_DESCRIPTOR_ENTRIES *ptrEndpoints, const USB_ENDPOINT_DESCRIPTOR *ptrEndpointdesc, int iAdd)
{
    while (ptrEndpoints->ptrActiveEndpoint != 0) {                       // search through present list
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
        if (iAdd == 0) {
            if ((ptrEndpointdesc->bEndpointAddress & 0x7f) == (ptrEndpoints->ptrActiveEndpoint->bEndpointAddress & 0x7f)) { // entry already present
                unsigned short usMaxLength = ptrEndpoints->ptrActiveEndpoint->wMaxPacketSize[0];
                usMaxLength |= (ptrEndpoints->ptrActiveEndpoint->wMaxPacketSize[1] << 8);
                if (usMaxLength > ptrEndpoints->usMaxEndpointLength) {
                     ptrEndpoints->usMaxEndpointLength = usMaxLength;    // enter maximum length on this endpoint
                }
                return 0;
            }
        }
    #else
        if ((ptrEndpointdesc->bEndpointAddress & 0x7f) == (ptrEndpoints->ptrActiveEndpoint->bEndpointAddress & 0x7f)) { // entry already present
            if (iAdd == 0) {
                unsigned short usMaxLength = ptrEndpoints->ptrActiveEndpoint->wMaxPacketSize[0];
                usMaxLength |= (ptrEndpoints->ptrActiveEndpoint->wMaxPacketSize[1] << 8);
                if (usMaxLength > ptrEndpoints->usMaxEndpointLength) {
                     ptrEndpoints->usMaxEndpointLength = usMaxLength;    // enter maximum length on this endpoint
                }
            }
            return 0;                                                    // entry already exists so complete
        }
    #endif
        ptrEndpoints++;
    }
    if (iAdd != 0) {                                                     // new entry to be added
        ptrEndpoints->ptrActiveEndpoint = ptrEndpointdesc;
        return 1;
    }
    return 0;
}

// Collect endpoint information based on the configuration descriptor, configuration, interface and alternative interface
//
static void *fnConfigInformation(unsigned char ucConfig, unsigned char ucInterface, unsigned char ucAlternativeInterface, USB_ENDPOINT_DESCRIPTOR_ENTRIES *ptrEndpoints, int iChannel)
{
    int iConfigNotFound = 1;
    int iConfigValid = 0;
    int iSummedEndpoints = 0;
    unsigned short usLength;
    unsigned char *ptrDesc = (unsigned char *)fnGetUSB_config_descriptor(&usLength, iChannel); // request the configuration descriptor and activate all endpoints belonging to the configuration
    unsigned char ucThisLength;                                          // the individual descriptor length
    unsigned long ulIgnoreInterfaces = 0;                                // accept all interfaces by default
    unsigned char ucPresentInterface = 0;
#if defined USB_HOST_SUPPORT                                             // {29}
    int iHost = 0;
    if (ptrDesc == 0) {                                                  // if there is no config descriptor we are operating as a host
        ptrDesc = config_descriptor[iChannel];                           // set the pointer to the config descriptor that has been received from the device
        usLength = usConfigDescriptorLength[iChannel];
    #if defined USB_HOST_INTERFACE_FILTER                                // {38}
        ulIgnoreInterfaces = fnGetUSB_InterfaceFilter(iChannel);
    #endif
    }
#endif

    do {
        ucThisLength = *ptrDesc;                                         // length of this descriptor
        usLength -= ucThisLength;                                        // remove from total length
        switch (*(ptrDesc + 1)) {                                        // the descriptor type
        case DESCRIPTOR_TYPE_CONFIGURATION:
            {
                USB_CONFIGURATION_DESCRIPTOR *config_desc = (USB_CONFIGURATION_DESCRIPTOR *)ptrDesc;
                if ((ucConfig == 0) || (config_desc->bConfigurationValue == ucConfig)) { // is this the configuration to be activated?
                    iConfigValid = 1;                                    // this configuration should be counted
                    iConfigNotFound = 0;                                 // the searched configuration has been found
                }
                else {
                    iConfigValid = 0;                                    // this configuration should not be counted
                }
            }
            break;
        case DESCRIPTOR_TYPE_INTERFACE:                                  // interface descriptor
            {
                if (iConfigValid != 0) {
                    USB_INTERFACE_DESCRIPTOR *interface_descriptor = (USB_INTERFACE_DESCRIPTOR *)ptrDesc;
                    if ((ucInterface == 0xff) || (interface_descriptor->bInterfaceNumber == ucInterface)) { // interface match
                        ucPresentInterface = interface_descriptor->bInterfaceNumber;
                        if ((ucAlternativeInterface == 0xff) || (interface_descriptor->bAlternateSetting == ucAlternativeInterface)) {
                            iConfigValid = 2;                            // endpoints belonging to this interface should be counted
                            if (ptrEndpoints == 0) {                     // request for alternative interface
                                return (interface_descriptor);           // return a pointer to the interface descriptor
                            }
                        }
                        else {
                            iConfigValid = 3;                            // endpoints belonging to this interface should not be counted but their size should be respected (for memory initialisation purposes)
                        }
                    }
                }
            }
            break;
        case DESCRIPTOR_TYPE_ENDPOINT:                                   // endpoint type
            if ((iConfigValid != 0) && (ptrEndpoints != 0)) {            // add endpoint
                if (((1 << ucPresentInterface) & ulIgnoreInterfaces) != 0) { // check interface filter
                    break;                                               // ignore endpoints belonging to this interface
                }
#if defined USB_HOST_SUPPORT
                if (iHost != 0) {
                    if (iSummedEndpoints >= (NUMBER_OF_USB_ENDPOINTS - 1)) { // if our USB controller can't handle the number of enpoints we stop and make this visible so that endpoint filtering can be enabled
                        fnDebugMsg("Physical endpoint count reached!\r\n");
                        return (void *)ptrEndpoints;                         // maximum possible endpoints belonging to this configuration have been collected
                    }
                }
#endif
                iSummedEndpoints += fnAdd_to_EndpointList(ptrEndpoints, (USB_ENDPOINT_DESCRIPTOR *)ptrDesc, (iConfigValid == 2));
            }
            break;
        default:                                                         // ignore the content of other types
            break;
        }
        ptrDesc += ucThisLength;                                         // progress to next
    } while (usLength != 0);
    if (iConfigNotFound != 0) {
        return 0;
    }
    return (void *)ptrEndpoints;                                        // no more endpoints belonging to this configuration
}


// Change state of a specified configuration
//
extern int fnSetUSBConfigState(int iCommand, unsigned char ucConfig, int iChannel)
{
    int iEndpoints = 0;
    unsigned char ucAlternativeInterface = 0;
    #if defined USB_SIMPLEX_ENDPOINTS || defined SUPPORT_USB_SIMPLEX_HOST_ENDPOINTS
    USB_ENDPOINT_DESCRIPTOR_ENTRIES EndpointList[NUMBER_OF_USB_ENDPOINTS * 2]; // temporary list of endpoints belonging to this particular configuration
    #else
    USB_ENDPOINT_DESCRIPTOR_ENTRIES EndpointList[NUMBER_OF_USB_ENDPOINTS]; // temporary list of endpoints belonging to this particular configuration
    #endif

    while (ucAlternativeInterface < 2) {
        uMemset(EndpointList, 0x00, sizeof(EndpointList));
        iEndpoints = 0;

#if defined USE_USB_AUDIO_ && defined USE_USB_CDC                        // test (count all endpoints - also when they don't match the interface)
        if (fnConfigInformation(ucConfig, 0xff, 0xff, EndpointList, iChannel) == 0) { // collect a list of endpoints belonging to all default interfaces of this configuration
            return 1;                                                    // not a valid configuration
        }
#else
        if (fnConfigInformation(ucConfig, 0xff, ucAlternativeInterface, EndpointList, iChannel) == 0) { // collect a list of endpoints belonging to all default interfaces of this configuration
            return 1;                                                    // not a valid configuration
        }
#endif
#if defined USE_USB_AUDIO_ && !defined USE_USB_CDC
        // Test
        //
        if (EndpointList[iEndpoints].ptrActiveEndpoint == 0) {           // no endpoints found so try alternative interface
            if (fnConfigInformation(ucConfig, 0xff, 1, EndpointList, iChannel) == 0) { // collect a list of endpoints belonging to all default interfaces of this configuration
                return 1;                                                // not a valid configuration
            }
        }
#endif
        while (EndpointList[iEndpoints].ptrActiveEndpoint != 0) {        // for each endpoint to be activated
            switch (iCommand) {
            case USB_CONFIG_ACTIVATE:
                if (ucConfig == 0) {
                    fnDeactivateEndpoint(EndpointList[iEndpoints].ptrActiveEndpoint, iChannel); // deactivate the endpoint
                }
                else {
                    fnActivateEndpoint(EndpointList[iEndpoints].ptrActiveEndpoint, EndpointList[iEndpoints].usMaxEndpointLength, iChannel); // configure and activate the endpoint for operation
                }
                break;

            default:                                                     // USB_DEVICE_SUSPEND / USB_DEVICE_RESUME
                {
                    USB_ENDPOINT *usb_endpoint_queue = usb_endpoint_control[iChannel];
                    usb_endpoint_queue += (EndpointList[iEndpoints].ptrActiveEndpoint->bEndpointAddress & 0x7f);
                    if (USB_DEVICE_SUSPEND == iCommand) {
                        usb_endpoint_queue->ucState |= USB_ENDPOINT_SUSPENDED;
                        usb_endpoint_queue->ucState &= ~USB_ENDPOINT_ACTIVE;
                    }
                    else {                                               // USB_DEVICE_RESUME
                        usb_endpoint_queue->ucState &= ~USB_ENDPOINT_SUSPENDED;
                        usb_endpoint_queue->ucState |= USB_ENDPOINT_ACTIVE;
                    }
                }
                break;
            }
            iEndpoints++;
        }
        ucAlternativeInterface++;
    }
    return 0;
}

#if defined USB_DEVICE_SUPPORT
// Change state of specified interface
//
static void *fnSetUSBInterfaceState(int iCommand, unsigned char ucInterface, unsigned char ucAlternativeInterface, int iChannel)
{
    if (USB_INTERFACE_ALTERNATE == iCommand) {
        return (fnConfigInformation(ucActiveConfiguration[iChannel], ucInterface, 1, 0, iChannel));
    }
    else {
        int iEndpoint = 0;
    #if defined USB_SIMPLEX_ENDPOINTS
        USB_ENDPOINT_DESCRIPTOR_ENTRIES EndpointList[NUMBER_OF_USB_ENDPOINTS * 2]; // temporary list of endpoints belonging to this particular configuration
    #else
        USB_ENDPOINT_DESCRIPTOR_ENTRIES EndpointList[NUMBER_OF_USB_ENDPOINTS]; // temporary list of endpoints belonging to this particular configuration
    #endif
        uMemset(EndpointList, 0x00, sizeof(EndpointList));

        switch (iCommand) {
        case USB_INTERFACE_DEACTIVATE:                                   // deactivate the specified interface
            fnConfigInformation(ucActiveConfiguration[iChannel], ucInterface, 0xff, EndpointList, iChannel); // get a list of endpoints belonging to this main interface and deactivate them all
            while (EndpointList[iEndpoint].ptrActiveEndpoint != 0) {
                fnDeactivateEndpoint(EndpointList[iEndpoint].ptrActiveEndpoint, iChannel); // deactivate the endpoint
                iEndpoint++;
            }
            break;

        case USB_INTERFACE_ACTIVATE:                                     // activate the alternative interface
            if (fnConfigInformation(ucActiveConfiguration[iChannel], ucInterface, ucAlternativeInterface, EndpointList, iChannel) == 0) { // get a list of endpoints belonging to this alternative interface and activate them all
                return (void *)1;                                        // the alternative interface does not exist
            }
            while (EndpointList[iEndpoint].ptrActiveEndpoint != 0) {
                fnActivateEndpoint(EndpointList[iEndpoint].ptrActiveEndpoint, 0, iChannel); // activate the endpoint
                iEndpoint++;
            }
            break;
        }
    }
    return 0;
}
#endif

// Called to set a new endpoint state (for example when the hardware driver needs to flag stalled state)
//
extern void fnSetUSBEndpointState(int iEndpoint, unsigned char ucStateSet, int iChannel)
{
    USB_ENDPOINT *usb_endpoint_queue = (usb_endpoint_control[iChannel] + iEndpoint);
    usb_endpoint_queue->ucState |= ucStateSet;                           // {14}
}

// Called to get the reference to the IN endpoint belonging to an IN/OUT pair
//
extern int fnGetPairedIN(int iEndpoint_OUT, int iChannel)                // {16}
{
    USB_ENDPOINT *usb_endpoint_queue = usb_endpoint_control[iChannel];
    usb_endpoint_queue += iEndpoint_OUT;
    return usb_endpoint_queue->ucPaired_IN;
}

#if defined USB_HOST_SUPPORT
extern void fnShowEndpoint(unsigned char ucEndpointReference)
{
    if ((ucEndpointReference & IN_ENDPOINT) != 0) {
        fnDebugMsg("IN");
    }
    else {
        fnDebugMsg("OUT");
    }
    fnDebugMsg(" EP-");
    fnDebugDec((ucEndpointReference & ~(IN_ENDPOINT)), WITH_CR_LF);      // display the endpoint number
}

// Extract and optionally display endpint details
//
extern void fnDisplayEndpoint(USB_ENDPOINT_DESCRIPTOR *ptr_endpoint_desc, USB_ENDPOINT_INFO *ptrEndpointInfo, unsigned char ucHostSpeed, int iDebugOut)
{
    CHAR cEP_details[64];
    CHAR *ptrBuf = cEP_details;
    unsigned short usEndpointLength = (ptr_endpoint_desc->wMaxPacketSize[0] | (ptr_endpoint_desc->wMaxPacketSize[1] << 8));
    unsigned char ucEndpointNumber = (ptr_endpoint_desc->bEndpointAddress & ~(IN_ENDPOINT));
    ptrEndpointInfo->ucEndpointNumber = ucEndpointNumber;
    ptrEndpointInfo->usEndpointSize = (usEndpointLength & 0x07ff);
    ptrEndpointInfo->ucEndpointType = ptr_endpoint_desc->bmAttributes;
    ptrBuf = fnBufferDec(ucEndpointNumber, 0, ptrBuf);
    ptrBuf = uStrcpy(ptrBuf, " = ");
    switch (ptr_endpoint_desc->bmAttributes) {
    case ENDPOINT_BULK:
        ptrBuf = uStrcpy(ptrBuf, "BULK");
        if ((ptr_endpoint_desc->bEndpointAddress & IN_ENDPOINT) != 0) {
            ptrBuf = uStrcpy(ptrBuf, " IN");
            ptrEndpointInfo->ucEndpointType = ENDPOINT_BULK_IN;
        }
        else {
            ptrBuf = uStrcpy(ptrBuf, " OUT");
            ptrEndpointInfo->ucEndpointType = ENDPOINT_BULK_OUT;
        }
        break;
    case ENDPOINT_INTERRUPT:
        ptrBuf = uStrcpy(ptrBuf, "INTERRUPT");
        break;
    case ENDPOINT_ISOCHRONOUS:
        ptrBuf = uStrcpy(ptrBuf, "ISOCHRONOUS");
        break;
    case ENDPOINT_CONTROL:
        ptrBuf = uStrcpy(ptrBuf, "CONTROL");
        break;
    default:
        *ptrBuf = '?';
        break;
    }
    if (*ptrBuf != '?') {                                                // if a valid endpoint type
        ptrBuf = uStrcpy(ptrBuf, " with size");
        ptrBuf = fnBufferDec(ptrEndpointInfo->usEndpointSize, WITH_SPACE, ptrBuf);
        if ((ENDPOINT_INTERRUPT == ptr_endpoint_desc->bmAttributes) || (ENDPOINT_ISOCHRONOUS == ptr_endpoint_desc->bmAttributes)) {
    #if defined USB_HS_INTERFACE
            switch (usEndpointLength & 0x1800) {
            case 0x0800:                                                 // two transactions per microframe
                ptrBuf = uStrcpy(ptrBuf, " [2 transactions]");
                ptrEndpointInfo->ucTransactions = 2;
                break;
            case 0x1000:                                                 // three transactions per microframe
                ptrBuf = uStrcpy(ptrBuf, " [3 transactions]");
                ptrEndpointInfo->ucTransactions = 3;
                break;
            default:
                ptrEndpointInfo->ucTransactions = 1;
                break;
            }
    #endif
            ptrBuf = uStrcpy(ptrBuf, " (interval =");
    #if defined USB_HS_INTERFACE
            if ((ucHostSpeed & USB_EVENT_INTERFACE_MASK) == EVENT_USB_DETECT_HS) {
                unsigned long ulRate = 125;                              // fastest polling rate (unit 125us)
                unsigned char ucValue = ptr_endpoint_desc->bInterval;
                if (ucValue > 16) {
                    ucValue = 16;
                }
                while (ucValue-- > 1) {
                    ulRate *= 2;
                }
                ptrEndpointInfo->pollingRate_us = ulRate;
                ptrEndpointInfo->pollingRate = (DELAY_LIMIT)((ulRate * SEC) / 8000000);
                if (ulRate >= 1000) {                                    // if not less that ms
                    ulRate /= 1000;                                      // convert from us to ms
                    ptrBuf = fnBufferDec(ulRate, WITH_SPACE, ptrBuf);
                    ptrBuf = uStrcpy(ptrBuf, "ms)");
                }
                else {
                    ptrBuf = fnBufferDec(ulRate, WITH_SPACE, ptrBuf);
                    ptrBuf = uStrcpy(ptrBuf, "us)");
                }
            }
            else {
    #endif
                ptrBuf = fnBufferDec(ptr_endpoint_desc->bInterval, WITH_SPACE, ptrBuf);
                ptrBuf = uStrcpy(ptrBuf, "ms)");
                ptrEndpointInfo->pollingRate = (DELAY_LIMIT)((ptr_endpoint_desc->bInterval * SEC) / 1000);
                ptrEndpointInfo->pollingRate_us = (ptr_endpoint_desc->bInterval * 1000);
    #if defined USB_HS_INTERFACE
            }
    #endif
        }
    }
    else {
        ptrEndpointInfo->ucEndpointType = ENDPOINT_INVALID;
        ++ptrBuf;
    }
    ptrBuf = uStrcpy(ptrBuf, "\r\n");
    if (iDebugOut != 0) {
        fnDebugMsg(cEP_details);
    }
}
#endif
#endif
