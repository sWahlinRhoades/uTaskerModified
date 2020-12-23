/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_HUB.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************


*/

#if defined HUB_DEFINES
    // States
    //
    #define HUB_STATE_NOT_CONNECTED                 0
    #define HUB_STATE_REQUESTING_DESCRIPTOR         1
    #define HUB_STATE_REQUESTING_DEVICE_STATUS      2
    #define HUB_STATE_CONFIGURING                   3
    #define HUB_STATE_ENABLING_CONFIGURATION        4
    #define HUB_STATE_CONFIGURED                    5
    #define HUB_STATE_REQUESTING_HUB_STATUS         6
    #define HUB_STATE_SETTING_REMOTE_WAKEUP         7
    #define HUB_STATE_OPERATING                     8

    // Events
    //
    #define HUB_DEVICE_STATUS_STAGE_RECEPTION       0
    #define HUB_DEVICE_DATA_RECEPTION               1
    #define HUB_DEVICE_CONNECTED                    2
    #define HUB_DEVICE_ACTIVATED                    3
    #define HUB_DEVICE_DISCONNECTED                 4
    #define HUB_DEVICE_SETUP_ACKED                  5
    #define HUB_DEVICE_SHOW_CONFIG                  6
#endif

#if defined HUB_HOST_STRUCT
    typedef struct stHUB_HOST_INSTANCE
    {
        int iHubState;
        unsigned char ucConfiguration ;
        unsigned char ucHubPorts;
        unsigned char ucCharacteristics;
        unsigned char ucPortDelay_ms;
        unsigned char ucHubCurrent;
        unsigned char ucPortNonRemovable[32];                            // supports up to 255 ports
        unsigned char ucPresentHubPort;
    } HUB_HOST_INSTANCE;
#endif

#if defined HUB_HOST_CONST
    static const USB_HOST_DESCRIPTOR getHubDescriptor = {
        (STANDARD_DEVICE_TO_HOST | REQUEST_DEVICE_CLASS),                // 0xa0
        USB_REQUEST_GET_DESCRIPTOR,                                      // request type (0x06)
        {LITTLE_SHORT_WORD_BYTES(0x2900)},                               // wValue
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wIndex
        {LITTLE_SHORT_WORD_BYTES(0x0047)},                               // length that can be accepted
    };

    static const USB_HOST_DESCRIPTOR getDeviceStatus = {
        (STANDARD_DEVICE_TO_HOST),                                       // 0x80
        USB_REQUEST_GET_STATUS,                                          // request type (0x00)
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wValue
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wIndex
        {LITTLE_SHORT_WORD_BYTES(0x0002)},                               // length that can be accepted
    };

    static const USB_HOST_DESCRIPTOR getHubStatus = {
        (STANDARD_DEVICE_TO_HOST | REQUEST_DEVICE_CLASS),                // 0xa0
        USB_REQUEST_GET_STATUS,                                          // request type (0x00)
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wValue
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wIndex
        {LITTLE_SHORT_WORD_BYTES(0x0004)},                               // length that can be accepted
    };

    static const USB_HOST_DESCRIPTOR setRemoteWakeup = {
        (STANDARD_HOST_TO_DEVICE | REQUEST_DEVICE_STANDARD),             // 0x00
        USB_REQUEST_SET_FEATURE,                                         // request type (0x03)
        {LITTLE_SHORT_WORD_BYTES(0x0001)},                               // wValue
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // wIndex
        {LITTLE_SHORT_WORD_BYTES(0x0000)},                               // length that can be accepted
    };
#endif

#if defined HUB_FUNCTIONS
// Send a set port feature to enable power to the specified hub port
//
static void fnSetPortPower(unsigned char ucHubPort)
{
    static USB_HOST_DESCRIPTOR hub_set_port_feature;
    hub_set_port_feature.bmRecipient_device_direction = (STANDARD_HOST_TO_DEVICE | REQUEST_OTHER_CLASS); // 0x23
    hub_set_port_feature.bRequest = USB_REQUEST_SET_FEATURE;             // 0x03
    hub_set_port_feature.wValue[0] = 0x08;
    hub_set_port_feature.wValue[1] = 0x00;
    hub_set_port_feature.wIndex[0] = ucHubPort;                          // port reference
    hub_set_port_feature.wIndex[1] = 0x00;
    hub_set_port_feature.wLength[0] = 0x00;
    hub_set_port_feature.wLength[1] = 0x00;
    // send Set Port Feature for ports 1..4
    // setup (OUT) 0x23 0x03 0x08 0x00 (port reference) 0x00 0x00 0x00
    // expect zero data response
    fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&hub_set_port_feature, sizeof(hub_set_port_feature));
}

static void fnGetPortStatus(unsigned char ucHubPort)
{
    static USB_HOST_DESCRIPTOR hub_get_status;
    hub_get_status.bmRecipient_device_direction = (STANDARD_DEVICE_TO_HOST | REQUEST_OTHER_CLASS); // 0xa3
    hub_get_status.bRequest = USB_REQUEST_GET_STATUS;                    // 0x00
    hub_get_status.wValue[0] = 0x00;
    hub_get_status.wValue[1] = 0x00;
    hub_get_status.wIndex[0] = ucHubPort;                                // port reference
    hub_get_status.wIndex[1] = 0x00;
    hub_get_status.wLength[0] = 0x04;
    hub_get_status.wLength[1] = 0x00;
    // send Get Port Feature for ports 1..4
    // setup (IN) 0xa3 0x00 0x00 0x00 (iEvent - 7) 0x00 0x04 0x00
    // expect data response with 4 bytes
    // and terminate with zero data frame
    fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&hub_get_status, sizeof(hub_get_status));
}

// Host hub class state event machine
//
static int fnHubStateMachine(int iEvent, unsigned char *ptr_ucData, unsigned short usLength)
{
    static HUB_HOST_INSTANCE hub_instance = {0};

    if (HUB_DEVICE_DISCONNECTED == iEvent) {
        uMemset(&hub_instance, 0, sizeof(hub_instance));
        if (USBPortID_hub_host != NO_ID_ALLOCATED) {
            fnClose(&USBPortID_hub_host);                                // free up device specific queue memory and invalidate their handles
        }
        return 0;
    }
    if (HUB_DEVICE_SETUP_ACKED == iEvent) {
        if ((hub_instance.iHubState > HUB_STATE_NOT_CONNECTED) && (hub_instance.iHubState < HUB_STATE_OPERATING)) { // during hub enumeration
            return -1;                                                   // during hub enumeration don't handle standard enumeration handing
        }
        else {
            return 0;                                                    // no hub or hub enumerated so allow standard enumberation handling
        }
    }

    switch (hub_instance.iHubState) {
    case HUB_STATE_NOT_CONNECTED:
        switch (iEvent) {
        case HUB_DEVICE_CONNECTED:
            hub_instance.ucConfiguration = *ptr_ucData;                  // save the configuration reference
            hub_instance.iHubState = HUB_STATE_REQUESTING_DESCRIPTOR;
            fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&getHubDescriptor, sizeof(getHubDescriptor));
            // Send control transfer
            // setup (IN) 0xa0 0x06 0x00 0x29 0x00 0x00 0x47 0x00
            // expected read 0x09 0x29 0x04 0e9 0x00 0x32 0x64 0x00 0xff
            // and terminate with zero data frame
            break;
        }
        break;

    case HUB_STATE_REQUESTING_DESCRIPTOR:
        switch (iEvent) {
        case HUB_DEVICE_DATA_RECEPTION:
            {
                USB_HUB_DESCRIPTOR *ptrHub = (USB_HUB_DESCRIPTOR *)ptr_ucData;
                if (ptrHub->bDescriptorType == USB_HUB_DESCRIPTOR_TYPE) {// hub descriptor (0x29)
                    unsigned char ucVariableLength;
                    hub_instance.ucHubPorts = ptrHub->bNbrPorts;
                    ucVariableLength = ((ptrHub->bNbrPorts + 7) / 8);
                    uMemcpy(hub_instance.ucPortNonRemovable, &ptrHub->DeviceRemovable_PortPwrCtrlMask[0], ucVariableLength);
                    hub_instance.ucCharacteristics = ptrHub->wHubCharacteristics[0];
                    hub_instance.ucPortDelay_ms = (ptrHub->bPwrOn2PwrGood * 2);
                    hub_instance.ucHubCurrent = ptrHub->bHubContrCurrent;
                }
            }
            return -1;                                                   // handled, so don't allow enumeration processing to be used
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:
            hub_instance.iHubState = HUB_STATE_REQUESTING_DEVICE_STATUS;
            fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&getDeviceStatus, sizeof(getDeviceStatus));
            // The get device status
            // setup (IN) 0x80 0x00 0x00 0x00 0x00 0x00 0x02 0x00
            // expected 0x01 0x00
            // and terminate with zero data frame
            return -1;                                                   // handled, so don't allow enumeration processing to be used
        }
        break;

    case HUB_STATE_REQUESTING_DEVICE_STATUS:
        switch (iEvent) {
        case HUB_DEVICE_DATA_RECEPTION:
            // Device status
            // - 0x01 0x00
            return -1;                                                   // handled, so don't allow enumeration processing to be used
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:
            hub_instance.iHubState = HUB_STATE_ENABLING_CONFIGURATION;   // change the state before calling the enumeration state event machine so that its callback will be ignored
            fnDriver(USB_control[USB_HOST_REF], (TX_ON), hub_instance.ucConfiguration); // enable configuration - this causes the generic USB layer to send SetConfiguration (acknowledged with the event E_USB_ACTIVATE_CONFIGURATION))
            hub_instance.iHubState = HUB_STATE_CONFIGURING;
            return -1;                                                   // handled, so don't allow enumeration processing to be used
        }
        break;

    case HUB_STATE_ENABLING_CONFIGURATION:
        return 0;

    case HUB_STATE_CONFIGURING:
        switch (iEvent) {
        case HUB_DEVICE_ACTIVATED:
            hub_instance.ucPresentHubPort = 1;                           // start with first port
            fnSetPortPower(1);                                           // enable the hub port's power
            break;
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:                          // the previous port setup was acknowledged
            ++hub_instance.ucPresentHubPort;
            if ((hub_instance.ucPresentHubPort <= hub_instance.ucHubPorts) && ((hub_instance.ucCharacteristics & USB_HUB_LOGICAL_POWER_SWITCHING_MODE_MASK) == USB_HUB_LOGICAL_POWER_SWITCHING_MODE_INDIVIDUAL)) {
                fnSetPortPower(hub_instance.ucPresentHubPort);           // enable the next hub port's power
            }
            else {                                                       // all hub port power outputs have been enabled
                hub_instance.iHubState = HUB_STATE_CONFIGURED;
                hub_instance.ucPresentHubPort = 1;                       // start with first port
                fnGetPortStatus(1);                                      // get the first port's status
            }
            return -1;                                                   // handled, so don't allow enumeration processing to be used
        }
        break;

    case HUB_STATE_CONFIGURED:                                           // from here we cotinue to block standard enumeration handling with the exception of reception zero data termination control
        switch (iEvent) {
        case HUB_DEVICE_DATA_RECEPTION:
            // Response to first control transfer
            // - 0x00 0x01 0x00 0x00
            return -2;                                                   // handled, so don't allow enumeration processing to be used (allow reception zero data termination control)
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:                          // the previous port setup was acknowledged
            ++hub_instance.ucPresentHubPort;
if (hub_instance.ucPresentHubPort <= hub_instance.ucHubPorts) {
    fnGetPortStatus(hub_instance.ucPresentHubPort);          // get the next port's status
}
else {                                                       // all hub port power outputs have been enabled
    hub_instance.iHubState = HUB_STATE_REQUESTING_HUB_STATUS;
    fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&getHubStatus, sizeof(getHubStatus));
}
return -2;                                                   // handled, so don't allow enumeration processing to be used (allow reception zero data termination control)
        }
        break;

    case HUB_STATE_REQUESTING_HUB_STATUS:
        switch (iEvent) {
        case HUB_DEVICE_DATA_RECEPTION:
            // Response to hub status
            // - 0x00 0x00 0x00 0x00
            return -2;                                                   // handled, so don't allow enumeration processing to be used (allow reception zero data termination control)
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:                          // the previous port setup was acknowledged
            hub_instance.iHubState = HUB_STATE_SETTING_REMOTE_WAKEUP;
            fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&setRemoteWakeup, sizeof(setRemoteWakeup));
            return -2;                                                   // handled, so don't allow enumeration processing to be used (allow reception zero data termination control)
        }
        break;

    case HUB_STATE_SETTING_REMOTE_WAKEUP:
        switch (iEvent) {
        case HUB_DEVICE_STATUS_STAGE_RECEPTION:                          // the hub status request completed
            hub_instance.iHubState = HUB_STATE_OPERATING;
            //fnDriver(USBPortID_hub_host, (RX_ON), 0);                    // enable IN polling on hub interrupt endpoint
            fnInterruptMessage(OWN_TASK, EVENT_HUB_CONFIGURED);
            return -2;                                                   // handled, so don't allow enumeration processing to be used (allow reception zero data termination control)
        }
        break;

    case HUB_STATE_OPERATING:
        switch (iEvent) {
        case HUB_DEVICE_SHOW_CONFIG:                                     // show hub details
            fnDebugMsg("HUB configured:\r\n");
            fnDebugDec(hub_instance.ucHubPorts, 0);
            fnDebugMsg(" ports\r\n");
            fnDebugMsg("Consumption = ");
            fnDebugDec(hub_instance.ucHubCurrent, 0);
            fnDebugMsg("mA\r\n");
            if ((hub_instance.ucCharacteristics & USB_HUB_LOGICAL_POWER_SWITCHING_MODE_MASK) == USB_HUB_LOGICAL_POWER_SWITCHING_MODE_INDIVIDUAL) {
                fnDebugMsg("Individual");
            }
            else {
                fnDebugMsg("Ganged");
            }
            fnDebugMsg(" port power with");
            fnDebugDec(hub_instance.ucPortDelay_ms, WITH_SPACE);
            fnDebugMsg("ms switch time\r\n");
            if ((hub_instance.ucCharacteristics & USB_HUB_COMPOUND_DEVICE) != 0) {
                fnDebugMsg("Compound device\r\n");
            }
            switch (hub_instance.ucCharacteristics & USB_HUB_OVER_CURRENT_PROTECTION_MODE_MASK) {
            case USB_HUB_OVER_CURRENT_PROTECTION_MODE_GLOBAL:
                fnDebugMsg("Global");
                break;
            case USB_HUB_OVER_CURRENT_PROTECTION_MODE_INDIVIDUAL:
                fnDebugMsg("Individual");
                break;
            default:
                fnDebugMsg("No");
                break;
            }
            fnDebugMsg(" current protection\r\n");
            switch (hub_instance.ucCharacteristics & USB_HUB_THINK_TIME_MASK) {
            case USB_HUB_THINK_TIME_8_FS:
                fnDebugMsg("8");
                break;
            case USB_HUB_THINK_TIME_16_FS:
                fnDebugMsg("16");
                break;
            case USB_HUB_THINK_TIME_24_FS:
                fnDebugMsg("24");
                break;
            case USB_HUB_THINK_TIME_32_FS:
                fnDebugMsg("32");
                break;
            }
            fnDebugMsg(" FS think time\r\n");
            if ((hub_instance.ucCharacteristics & USB_HUB_DOWN_STREAM_PORT_INDICATOR) != 0) {
                fnDebugMsg("With down-stream port indicator\r\n");
            }
            break;
        }
        break;
    }
    return 0;
}

// hub interrupt endpoint reception interrupt callback
//
static int hub_interrupt_callback(unsigned char *ptrData, unsigned short length, int iType, int iChannel)
{
    if (OUT_DATA_RECEPTION == iType) {
        int iEvent = 0;
        unsigned char ucPortChanges = *ptrData;
        while (ucPortChanges != 0) {
            if (((1 << iEvent) & ucPortChanges) != 0) {                  // first bit indicated hub change (EVENT_HUB_CHANGED) and subsequent bits indicate port changes (HUB_PORTx_CHANGED)
                fnInterruptMessage(OWN_TASK, (unsigned char)(EVENT_HUB_CHANGED + iEvent));
                ucPortChanges &= ~(unsigned char)(1 << iEvent);
            }
            iEvent++;
        }
        return BUFFER_CONSUMED;
    }
    return 0;
}
#endif

/********************************** HUB timer event handling **********************************/

#if defined HUB_INTERRUPT_EVENTS
            case EVENT_HUB_CONFIGURED:                                    // hub has been configured
                fnHubStateMachine(HUB_DEVICE_SHOW_CONFIG, 0, 0);
                break;
            case EVENT_HUB_CHANGED:
                fnDebugMsg("Hub change\n");
                break;
            case HUB_PORT1_CHANGED:
                fnDebugMsg("Port 1 change\n");
                // request port status 0xa3 0x00 0x00 0x00 (port ref) 0x00 0x04 0x00 - expect 0x03 0x03 0x10 0x00 (plus send zero terminator)
                // command clear port feature 0x23 0x01 0x14 0x00 0x01 0x00 0x00 0x00 (C_PORT_RESET) - receive zero terminator
                // then do enumeration of newly connected device (address 0) including setting ist address (hub + 1)
            case HUB_PORT2_CHANGED:
                fnDebugMsg("Port 2 change\n");
                break;
            case HUB_PORT3_CHANGED:
                fnDebugMsg("Port 3 change\n");
                break;
            case HUB_PORT4_CHANGED:
                fnDebugMsg("Port 4 change\n");
                break;
            case HUB_PORT5_CHANGED:
                fnDebugMsg("Port 5 change\n");
                break;
            case HUB_PORT6_CHANGED:
                fnDebugMsg("Port 6 change\n");
                break;
            case HUB_PORT7_CHANGED:
                fnDebugMsg("Port 7 change\n");
                break;
#endif
