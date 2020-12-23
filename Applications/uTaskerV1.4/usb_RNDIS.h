/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_RNDIS.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    28.06.2020 Extracted RNDIS device and host code from usb_application.c to this file
    02.09.2020 Defer handling of RNDIS device notifications until control transfers have completed {1}

*/


#if defined RNDIS_DEFINES
    #define DEBUG_RNDIS_ON                                               // warning can't be used with telnet as debug output!!
  //#define RNDIS_HOST_DEBUG

    #define KEEPALIVE_TIME                      (DELAY_LIMIT)(10 * SEC)  // host keep-alive timer period according to the RNDIS specification recommendation 5s
    #define RNDIS_CONTROL_TIMEOUT               (DELAY_LIMIT)(12 * SEC)  // maximum time the host waits for any request sent on the control channel

    #define REFRESH_KEEPALIVE_ON_DATA_RECEPTION                          // restart the keep alive timer each time valid RNDIS traffic data is received

    #define RNDIS_DEBUG_LEVEL_OFF               0x00000000
    #define RNDIS_DEBUG_LEVEL_LOW               0x00000001
    #define RNDIS_DEBUG_LEVEL_HIGH              0x00000002

    #define RNDIS_STATE_UNINITIALISED           0
    #define RNDIS_STATE_BUS_INITIALISED         1
    #define RNDIS_INITIALISED                   2
    #define RNDIS_DATA_INITIALISED              3

    #define RNDIS_HOST_OPERATION_REMOTE_NDIS_INITIALIZE_MSG       0

    #define REMOTE_NDIS_GET_RESPONSE            0x00000055               // fictive messages which control the next RNDIS host control sequence
    #define REMOTE_NDIS_QUERY_LIST              0x00000056

    #define RNDIS_MEDIA_DISCONNECTED            0
    #define RNDIS_MEDIA_CONNECTED               1
    #define RNDIS_INIT_COMPLETE                 2

    #define RNDIS_HOST_STARTING_DATA            1
    #define RNDIS_HOST_TERMINATED_DATA          2
    #define RNDIS_HOST_POLL_INTERRUPT           3
    #define RNDIS_HOST_DATA_PACKET_TERMINATED   4
    #define RNDIS_HOST_POLL_DATA                5
    #define RNDIS_HOST_CTRL_TRANSACTION_START   6
    #define RNDIS_HOST_CTRL_TERMINATING         7
    #define RNDIS_HOST_DATA_TRANSFER            8
    #define RNDIS_HOST_CTRL_TERMINATED          9

    #define RNDIS_HOST_MANAGEMENT_DATA_ACTIVE         0x00000001
    #define RNDIS_HOST_MANAGEMENT_DELAY_INT_POLL      0x00000002
    #define RNDIS_HOST_MANAGEMENT_CTL_ACTIVE          0x00000004
    #define RNDIS_HOST_MANAGEMENT_CTL_TERMINATING     0x00000008
    #define RNDIS_HOST_MANAGEMENT_DELAY_CTL_INT_POLL  0x00000010

    #define RNDIS_MANAGEMENT_NO_BLOCK           0
    #define RNDIS_MANAGEMENT_BLOCK_POLL         1
    #define RNDIS_MANAGEMENT_BLOCK_DATA         1

    #define RDNIS_VENDOR_OUI                    NON_REGISTERED_VENDOR    // no IEEE registered OUI exists
    #define RNDIS_NIC_IDENTIFIER                0x01

    #define RNDIS_CONTROL_DATA_BUFFER_SIZE      256
    #define MAX_DEVICE_OIDS                     64
    #define RNDIS_STATE_INITIALISED             0x01
    #define RNDIS_STATE_CONNECTED               0x02
    #define RNDIS_STATE_INITIALISED_AND_CONNECTED (RNDIS_STATE_INITIALISED | RNDIS_STATE_CONNECTED)

    #define RNDIS_CONTROL_QUEUE_SIZE            5
#endif

#if defined RNDIS_HOST_STRUCT
    typedef struct stOID_INFO
    {
        unsigned long ulOID;
        unsigned long ulOID_data_length;
        const unsigned char *ptrData;
    } OID_INFO;

    typedef struct stRNDIS_DEVICE_DETAILS
    {
        unsigned long ulDeviceOIDs[MAX_DEVICE_OIDS];
        unsigned long ulRNDIS_MaxFrameSize;
        unsigned long ulRNDIS_MaxTotalSize;
        unsigned long ulRNDIS_ConnectStatus;
        unsigned char ucDeviceOIDs;
        unsigned char ucRNDIS_PermanentMAC[MAC_LENGTH];
        unsigned char ucRNDIS_CurrentMAC[MAC_LENGTH];
    } RNDIS_DEVICE_DETAILS;

    typedef struct stRNDIS_HOST_INSTANCE
    {
        unsigned long ulDeviceRequestID;
        unsigned long ulHostRequestID;
        RNDIS_DEVICE_DETAILS sRNDIS_deviceDatails;
        const OID_INFO *ptrOID;
        unsigned char ucRNDIS_HostState;
        unsigned char ucControlDataReception[RNDIS_CONTROL_DATA_BUFFER_SIZE];
        int iControlDataLength;
        int iWaitingControlDataLength;

        volatile unsigned long ulHostManagement;

        unsigned long ulRNDIS_ControlMessageQueue[RNDIS_CONTROL_QUEUE_SIZE];
        unsigned char ucRNDIS_queue_in;
        unsigned char ucRNDIS_queue_out;

    } RNDIS_HOST_INSTANCE;
#endif

#if defined RNDIS_HOST_CONST
    static const unsigned char packetFilter1[] = { LITTLE_LONG_WORD_BYTES(0x03) };
    static const unsigned char multiCastList[] = { LITTLE_LONG_WORD_BYTES(0x00003333), LITTLE_SHORT_WORD_BYTES(0x0100) };
    static const unsigned char packetFilter2[] = { LITTLE_LONG_WORD_BYTES(0x07) };

    static const OID_INFO ulDeviceOID_config_list[] = {
        // GET
        //
        { OID_GEN_SUPPORTED_LIST, 0, 0},                                 // 0x00010101 [must be first in the list]

        { 0x00010202, 4, 0 },                                            // unknown
        { OID_GEN_MAXIMUM_FRAME_SIZE, 4, 0 },                            // 0x00010106
        { OID_GEN_LINK_SPEED, 4, 0 },                                    // 0x00010107
        { OID_GEN_MEDIA_CONNECT_STATUS, 4, 0 },                          // 0x00010114
        { OID_802_3_CURRENT_ADDRESS, 6, 0 },                             // 0x01010102
        { OID_802_3_PERMANENT_ADDRESS, 6, 0 },                           // 0x01010101
        // SET
        //
        { OID_GEN_CURRENT_PACKET_FILTER, sizeof(packetFilter1), packetFilter1 }, // 0x0001010e
        { OID_802_3_MULTICAST_LIST, sizeof(multiCastList), multiCastList }, // 0x01010103
        { OID_GEN_CURRENT_PACKET_FILTER, sizeof(packetFilter2), packetFilter2 }, // 0x0001010e
        // GET
        //
        { OID_GEN_MAXIMUM_TOTAL_SIZE, 4, 0 },                            // 0x00010111
        {0}                                                              // end
    };

    #define OID_INFO_ENTRIES   ((sizeof(ulDeviceOID_config_list) / sizeof(ulDeviceOID_config_list[0])) - 1)

    static const REMOTE_NDIS_INITIALIZE_MESSAGE init_msg = {
        { LITTLE_LONG_WORD_BYTES(REMOTE_NDIS_INITIALIZE_MSG) },          // type of message being sent (0x00000002)
        { LITTLE_LONG_WORD_BYTES(sizeof(REMOTE_NDIS_INITIALIZE_MESSAGE)) }, // total length of this message (0x0018)
        { LITTLE_LONG_WORD_BYTES(0) },                                   // RNDIS message ID value (changes on each transmission)
        { LITTLE_LONG_WORD_BYTES(RNDIS_MAJOR_VERSION) },                 // RNDIS protocol version implemented by the host - major
        { LITTLE_LONG_WORD_BYTES(RNDIS_MINOR_VERSION) },                 // RNDIS protocol version implemented by the host - minor
        { LITTLE_LONG_WORD_BYTES(RNDIS_CONTROL_DATA_BUFFER_SIZE) },      // maximum size that can be received                            
    };
    static const REMOTE_NDIS_QUERY_MESSAGE query_msg = {
        { LITTLE_LONG_WORD_BYTES(REMOTE_NDIS_QUERY_MSG) },               // type of message being sent (0x00000004)
        { LITTLE_LONG_WORD_BYTES(sizeof(REMOTE_NDIS_QUERY_MESSAGE)) },   // total length of this message (0x001c)
        { LITTLE_LONG_WORD_BYTES(0) },                                   // RNDIS message ID value (changes on each transmission)
        { LITTLE_LONG_WORD_BYTES(OID_GEN_SUPPORTED_LIST) },              // supported OID list request
        { LITTLE_LONG_WORD_BYTES(0) },                                   // information buffer length
        { LITTLE_LONG_WORD_BYTES(0x14) },                                // information buffer offset
        { LITTLE_LONG_WORD_BYTES(0) },                                   // reserved for connection-oriented devices (aways 0)
    };
    static const REMOTE_NDIS_KEEPALIVE_MESSAGE keepalive_msg = {
        { LITTLE_LONG_WORD_BYTES(REMOTE_NDIS_KEEPALIVE_MSG) },           // type of message being sent (0x00000008)
        { LITTLE_LONG_WORD_BYTES(sizeof(REMOTE_NDIS_KEEPALIVE_MESSAGE)) }, // total length of this message (0x000c)
        { LITTLE_LONG_WORD_BYTES(0) },                                   // RNDIS message ID value (changes on each transmission)
    };
    static const REMOTE_NDIS_SET_KEEPALIVE_CMPLT keepalive_response = {
        { LITTLE_LONG_WORD_BYTES(REMOTE_NDIS_KEEPALIVE_COMPLETE) },      // type of message being sent (0x80000008)
        { LITTLE_LONG_WORD_BYTES(sizeof(REMOTE_NDIS_SET_KEEPALIVE_CMPLT)) }, // total length of this message (0x0010)
        { LITTLE_LONG_WORD_BYTES(0) },                                   // RNDIS message ID value (matching the requester's)
        { LITTLE_LONG_WORD_BYTES(0) },                                   // status OK
    };
#endif

#if defined RNDIS_DEVICE_CONSTS

    #define SUPPORTED_OID_CNT                       26                   // the number of object identifiers supported

    // List of supported OIDs
    //
    static const unsigned char supportedOIDs[SUPPORTED_OID_CNT][4] = {   // list or required OIDs that are supported
        {LITTLE_LONG_WORD_BYTES(OID_GEN_SUPPORTED_LIST)},                // start of general OIDs for RNDIS
        {LITTLE_LONG_WORD_BYTES(OID_GEN_HARDWARE_STATUS)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_MEDIA_SUPPORTED)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_MEDIA_IN_USE)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_MAXIMUM_FRAME_SIZE)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_LINK_SPEED)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_TRANSMIT_BLOCK_SIZE)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_RECEIVE_BLOCK_SIZE)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_VENDOR_ID)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_VENDOR_DESCRIPTION)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_VENDOR_DRIVER_VERSION)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_CURRENT_PACKET_FILTER)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_MAXIMUM_TOTAL_SIZE)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_MEDIA_CONNECT_STATUS)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_XMIT_OK)},                       // start of general statistics OIDs for RNDIS
        {LITTLE_LONG_WORD_BYTES(OID_GEN_RCV_OK)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_XMIT_ERROR)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_RCV_ERROR)},
        {LITTLE_LONG_WORD_BYTES(OID_GEN_RCV_NO_BUFFER)},
        {LITTLE_LONG_WORD_BYTES(OID_802_3_PERMANENT_ADDRESS)},           // start of 802.3 OIDs for RNDIS
        {LITTLE_LONG_WORD_BYTES(OID_802_3_CURRENT_ADDRESS)},
        {LITTLE_LONG_WORD_BYTES(OID_802_3_MULTICAST_LIST)},
        {LITTLE_LONG_WORD_BYTES(OID_802_3_MAXIMUM_LIST_SIZE)},
        {LITTLE_LONG_WORD_BYTES(OID_802_3_RCV_ERROR_ALIGNMENT)},         // start of 802.3 statistic OIDs for RNDIS
        {LITTLE_LONG_WORD_BYTES(OID_802_3_XMIT_ONE_COLLISION)},
        {LITTLE_LONG_WORD_BYTES(OID_802_3_XMIT_MORE_COLLISIONS)},
    };

    static const CHAR cVendorDescription[] = "uTasker RNDIS adapter";                                

    static const REMOTE_NDIS_INIT_COMPLETE_MESSAGE cRemoteNDISinitComplete = {
        {LITTLE_LONG_WORD_BYTES(REMOTE_NDIS_INITIALIZE_COMPLETE)},
        {0, 0, 0, 0},                                                    // place for length
        {0, 0, 0, 0},                                                    // place for request ID
        {0, 0, 0, 0},                                                    // success status
        {LITTLE_LONG_WORD_BYTES(RNDIS_MAJOR_VERSION)},
        {LITTLE_LONG_WORD_BYTES(RNDIS_MINOR_VERSION)},
        {LITTLE_LONG_WORD_BYTES(RNDIS_CONNECTION_ORIENTED_FLAGS)},
        {LITTLE_LONG_WORD_BYTES(RNDIS_MEDIUM_802_3)},                    // Ethernet
        {LITTLE_LONG_WORD_BYTES(1)},                                     // packets per transfer
        {LITTLE_LONG_WORD_BYTES(USB_ETH_MTU)},
        {LITTLE_LONG_WORD_BYTES(3)},                                     // packet alignment factor
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    };

    static unsigned char remoteNDISreception[USB_CDC_RNDIS_COUNT][sizeof(REMOTE_NDIS_QUERY_MESSAGE) + 64]; // buffer for collecting remote NDIS control messages in (44 extra bytes is largest known)
#endif

#if defined RNDIS_LOCAL_VARIABLES
    #if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    unsigned long ulRNDIS_debug_level = RNDIS_DEBUG_LEVEL_LOW;
    #endif
    #if defined USB_HOST_SUPPORT && defined USB_HOST_CALLBACK_SETUP_ACK && defined USB_RNDIS_HOST
    static unsigned char ucLocalRNDISEvent[USB_CONTROLLERS] = {0};
    #endif
    #if (defined USB_RNDIS_HOST || defined USB_CDC_RNDIS) && defined USB_TO_TCP_IP
    static int iHandlingRNDIS = 0;                                       // variable indicating whether an RNDIS data packet is presently being procssed by the local TCP/IP stack
    static REMOTE_NDIS_ETHERNET_PACKET_MESSAGE *ptrRNDIS_eth_packet;
    #endif
    #if (defined USB_DEVICE_SUPPORT && defined USE_USB_CDC && defined USB_CDC_RNDIS) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
    static unsigned short usRNDISpacketLength = 0;                       // length of data in present frame
    static unsigned short usExpectedRNDISlength = 0;                     // the advertised length of the present frame being collected
    #endif
    #if defined USE_USB_CDC && defined USB_CDC_RNDIS
    static REMOTE_NDIS_CONTROL remoteNDIScontrol[USB_CDC_RNDIS_COUNT] = {{0}};
    #endif
#endif

#if defined RNDIS_FUNCTIONS
#if (defined USB_DEVICE_SUPPORT && defined USE_USB_CDC && defined USB_CDC_RNDIS) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
extern void fnSetRNDIS_debugLevel(unsigned long ulNewLevel)
{
    switch (ulNewLevel) {
    case RNDIS_DEBUG_LEVEL_OFF:
    default:
        ulRNDIS_debug_level = RNDIS_DEBUG_LEVEL_OFF;
        fnDebugMsg("Off");
        return;
    case RNDIS_DEBUG_LEVEL_LOW:
        fnDebugMsg("Low");
        break;
    case RNDIS_DEBUG_LEVEL_HIGH:
        fnDebugMsg("High");
        break;
    }
    ulRNDIS_debug_level = ulNewLevel;
}

// Handle potential data arrived on the RNDIS data endpoint
//
static void fnRNDIS_USB_DataRx(QUEUE_HANDLE RNDIS_data_handle)
{
    static REMOTE_NDIS_ETHERNET_PACKET_MESSAGE remoteNDISpacket = {{{0}}}; // static buffer for collecting a data frame from the RNDIS host/device in
    QUEUE_TRANSFER Length;
    unsigned short usMaxDataLength;
    if (usExpectedRNDISlength == 0) {                                    // if the size of the present frame is not yet known
        usMaxDataLength = ((sizeof(remoteNDISpacket.rndis_message.ucMessageType) + sizeof(remoteNDISpacket.rndis_message.ucMessageLength)) - usRNDISpacketLength); // read initially the message type and length
    }
    else {
        usMaxDataLength = (usExpectedRNDISlength - usRNDISpacketLength); // remaining message length of present packet being received
    }
    while ((Length = fnRead(RNDIS_data_handle, (unsigned char *)&((unsigned char *)&remoteNDISpacket)[usRNDISpacketLength], usMaxDataLength)) != 0) { // while USB data from the endpoint
    #if defined DEBUG_RNDIS_ON
        if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
            fnDebugMsg("Chunk: ");
            fnDebugDec(Length, 0);
            fnDebugMsg(":");
            fnDebugDec(usMaxDataLength, WITH_CR_LF);
        }
    #endif
        usRNDISpacketLength += Length;                                   // the amount of content that has been received for this frame
        if (usExpectedRNDISlength == 0) {                                // if we do not yet know the size of the message presently being received
            if (usRNDISpacketLength == (sizeof(remoteNDISpacket.rndis_message.ucMessageType) + sizeof(remoteNDISpacket.rndis_message.ucMessageLength))) { // if message type and length have been collected
                if (remoteNDISpacket.rndis_message.ucMessageType[0] == (unsigned char)REMOTE_NDIS_PACKET_MSG) { // we only expect this message type
                    usExpectedRNDISlength = (remoteNDISpacket.rndis_message.ucMessageLength[0] | (remoteNDISpacket.rndis_message.ucMessageLength[1] << 8)); // the length that we expect
    #if defined DEBUG_RNDIS_ON
                    if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
                        fnDebugMsg("Expect - ");
                    }
    #endif
                    if (usExpectedRNDISlength%RNDIS_DATA_ENDPOINT_SIZE == 0) { // if the packet size is divisible by the endpoint size the host will send an additional zero as padding, which we need to later remove
                        usExpectedRNDISlength++;                         // add padding byte to overall length (this is later discarded)
    #if defined DEBUG_RNDIS_ON
                        if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
                            fnDebugMsg("(p) ");
                        }
    #endif
                    }
    #if defined DEBUG_RNDIS_ON
                    if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
                        fnDebugDec(usExpectedRNDISlength, 0);
                    }
    #endif
                }
                else {                                                   // invalid message type so continue searching
    #if defined DEBUG_RNDIS_ON
                    if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
                        int iDump = 0;
                        unsigned long ulRNDIS_type = (remoteNDISpacket.rndis_message.ucMessageType[0] | (remoteNDISpacket.rndis_message.ucMessageType[1] << 8) | (remoteNDISpacket.rndis_message.ucMessageType[2] << 16) | (remoteNDISpacket.rndis_message.ucMessageType[3] << 24));
                        fnDebugMsg("\r\nUSB_RNDIS type");
                        fnDebugHex(ulRNDIS_type, (sizeof(ulRNDIS_type) | WITH_SPACE | WITH_LEADIN));
                        fnDebugMsg(" rejected\r\n");
                        fnDebugMsg("Error: Len = ");
                        fnDebugDec(usRNDISpacketLength, 0);
                        fnDebugMsg("\r\nDump = ");
                        while (iDump < usRNDISpacketLength) {
                            fnDebugHex((*(unsigned char *)&((unsigned char *)&remoteNDISpacket)[iDump++]), (sizeof(unsigned char) | WITH_LEADIN | WITH_SPACE));
                        }
                        fnDebugMsg("\r\n");
                    }
    #endif
                    usExpectedRNDISlength = 0;                           // reset for next message
                    usRNDISpacketLength = 0;
                    usMaxDataLength = (sizeof(remoteNDISpacket.rndis_message.ucMessageType) + sizeof(remoteNDISpacket.rndis_message.ucMessageLength)); // maximum initial read size
    #if defined USB_RNDIS_HOST
                    fnPollRNDIS_data();                                  // re-enable IN polling
    #endif
                    continue;
                }
            }
            else {
    #if defined USB_RNDIS_HOST
                fnPollRNDIS_data();                                      // re-enable IN polling
    #endif
                continue;
            }
        }
    #if defined USB_RNDIS_HOST
        fnPollRNDIS_data();                                              // re-enable IN polling
    #endif
        if ((usExpectedRNDISlength != 0) && (usRNDISpacketLength >= usExpectedRNDISlength)) { // if the complete data packet has been received
            int iPayloadOffset = (remoteNDISpacket.rndis_message.ucDataOffset[0] + 8);
            int iDataLength = (remoteNDISpacket.rndis_message.ucDataLength[0] | (remoteNDISpacket.rndis_message.ucDataLength[1] << 8));
    #if defined USB_TO_TCP_IP || (defined USB_TO_ETHERNET && !defined NO_USB_ETHERNET_BRIDGING)
            unsigned char *ptrPayload = (unsigned char *)&((unsigned char *)&remoteNDISpacket)[iPayloadOffset];
    #endif
            Length = usExpectedRNDISlength;
            usExpectedRNDISlength = 0;                                   // reset for next message
            usRNDISpacketLength = 0;
            usMaxDataLength = (sizeof(remoteNDISpacket.rndis_message.ucMessageType) + sizeof(remoteNDISpacket.rndis_message.ucMessageLength)); // maximum initial read size
    #if defined REFRESH_KEEPALIVE_ON_DATA_RECEPTION
            uTaskerGlobalMonoTimer(OWN_TASK, KEEPALIVE_TIME, T_RNDIS_KEEPALIVE); // retrigger the keep-alive timer on received data
    #endif
            if (iDataLength != 0) {                                      // if the packet includes Ethernet payload
    #if defined USB_TO_TCP_IP || (defined USB_TO_ETHERNET && !defined NO_USB_ETHERNET_BRIDGING)
                int iConsumed = 0;
    #endif
    #if defined USB_TO_TCP_IP
                ETHERNET_FRAME rx_frame;
        #if defined DEBUG_RNDIS_ON
                if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
                    fnDebugMsg(" Out = ");
                    fnDebugDec(iDataLength, WITH_CR_LF);
                }
        #endif
                rx_frame.frame_size = (unsigned short)iDataLength;
                rx_frame.ptEth = (ETHERNET_FRAME_CONTENT *)ptrPayload;
                rx_frame.usDataLength = 0;
                rx_frame.usIPLength = 0;
        #if defined USB_TO_TCP_IP && (IP_INTERFACE_COUNT > 1)
                rx_frame.ucInterface = (RNDIS_INTERFACE >> INTERFACE_SHIFT); // reception is on the RNDIS interface
                rx_frame.ucInterfaceHandling = (DEFAULT_INTERFACE_CHARACTERISTICS | INTERFACE_NO_MAC_FILTERING); // default interface handling
        #endif
        #if defined IPV4_SUPPORT_RX_DEFRAGMENTATION && defined IP_RX_CHECKSUM_OFFLOAD
                rx_frame.ucSpecialHandling = (INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING | INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_MAC_FILTERING);
        #endif
                iHandlingRNDIS = 1;                                      // mark that the RNDIS payload is being processed by the local stack and it should not be bridged to the same interface
                    if ((iConsumed = fnHandleEthernetFrame(&rx_frame, RNDIS_Handle)) == 0) { // handle the reception
                        iHandlingRNDIS = 0;
        #if defined DEBUG_RNDIS_ON
                        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
                            fnDebugMsg("RNDIS->Stack:");                 // frame was consumed by the local stack and not bridged
                            fnDebugDec((unsigned long)iDataLength, 0);
                            fnDebugMsg("-");
                            fnDebugDec(Length, WITH_CR_LF);
                        }
        #endif
                        continue;                                        // if consumed by the local TCP/IP stack we don't bridge on to the Ethernet interface
                    }
                iHandlingRNDIS = 0;                                      // not consumed by the local TCP/IP stack
    #else
        #if defined DEBUG_RNDIS_ON
                if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
                    fnDebugMsg(" Out = ");
                    fnDebugDec(iDataLength, WITH_CR_LF);
                }
        #endif
    #endif
    #if defined USB_TO_ETHERNET && !defined NO_USB_ETHERNET_BRIDGING
                if (Ethernet_handle[DEFAULT_IP_INTERFACE] != NO_ID_ALLOCATED) { // if the Ethernet interface is available
                    if (fnWrite(Ethernet_handle[DEFAULT_IP_INTERFACE], ptrPayload, (QUEUE_TRANSFER)iDataLength) != 0) { // prepare the content for transmission on the Ethernet interface
                        fnWrite(Ethernet_handle[DEFAULT_IP_INTERFACE], 0, 0); // transmit the ETHERNET frame
        #if defined DEBUG_RNDIS_ON
                        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
                            if (iConsumed == 2) {                        // if bridging after also being handled by the local stack
                                fnDebugMsg("RNDIS->Stack->Eth:");
                            }
                            else {
                                fnDebugMsg("RNDIS->Eth:");
                            }
                            fnDebugDec((unsigned long)iDataLength, 0);
                            fnDebugMsg("-");
                            fnDebugDec(Length, WITH_CR_LF);
                        }
        #endif
                    }
                }
    #endif
            }
        }
        else {
            usMaxDataLength = (usExpectedRNDISlength - usRNDISpacketLength); // the remaining length of data belonging to the present packet
        }
    }
}
#endif

#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
// Reset the RNDIS device instance and stop its software timers
//
static void fnRNDIS_device_remove(int iController)
{
    ucLocalRNDISEvent[iController] = 0;
    uTaskerGlobalStopTimer(OWN_TASK, T_RNDIS_INTERRUPT_INTERVAL);        // stop the interrupt endpoint timer
    uTaskerGlobalStopTimer(OWN_TASK, T_RNDIS_KEEPALIVE);                 // stop the keep-alive timer
    uTaskerGlobalStopTimer(OWN_TASK, T_RNDIS_COMMAND_TIMEOUT);           // stop the command timer
    uMemset(&sRNDIS_Host, 0, sizeof(sRNDIS_Host));
    #if defined SUPPORT_QUEUE_DEALLOCATION
    fnClose(&USBPortID_interrupt[FIRST_CDC_INTERFACE_HANDLE]);           // free up device specific queue memory and invalidate their handles
    fnClose(&USBPortID_comms[FIRST_CDC_RNDIS_HANDLE]);
        #if (USB_CDC_COUNT > 1) && defined TELIT_LE910C1
    fnClose(&USBPortID_interrupt[VCOM_CDC_INTERFACE_HANDLE]);
    fnClose(&USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE]);
        #endif
    #endif
}

// A previous data transmission has terminated
//
static void fnRNDIS_Data_Tx_Complete(void)
{
    if (fnRNDIS_HostManagement(RNDIS_HOST_DATA_PACKET_TERMINATED) == RNDIS_MANAGEMENT_NO_BLOCK) { // if an interrupt endpoint poll was not performed due to active data transmission
        fnInterrupEndpointPoll();                                        // perform the interrupt endpoint poll now
    }
    fnPollRNDIS_data();                                                  // restart data IN polling after transmission has completed
}

// Handle potential RNDIS interrupt endpoint reception
//
static void fnRNDIS_USB_InterruptRx(QUEUE_HANDLE RNDIS_interrupt_handle, unsigned char *ptrBuffer)
{
    while (fnMsgs(RNDIS_interrupt_handle) != 0) {                        // while reception from interrupt endpoint
        // The only defined device notification is the response available notification so we don't check the content here but assume
        // that the device is signalling that is has a status change/response to return
        //
    #if defined RNDIS_HOST_DEBUG
        int i;
        QUEUE_TRANSFER Length
    #endif
        if (fnRNDIS_HostManagement(RNDIS_HOST_DATA_TRANSFER) == RNDIS_MANAGEMENT_BLOCK_DATA) { // {1}
            // Although we have a control transfer that is still in progress the device has responded with a notification
            // - we leave this in the queue and wîll handle it once the control transfer has completed
            //
            return;
        }
    #if defined RNDIS_HOST_DEBUG
        Length = fnRead(RNDIS_interrupt_handle, ptrBuffer, LARGE_MESSAGE); // read the content to the input buffer
        fnDebugMsg("Interrupt message =");
        for (i = 0; i < Length; i++) {
            fnDebugHex(ptrBuffer[i], (1 | WITH_LEADIN | WITH_SPACE));
        }
        fnDebugMsg("\r\n");
    #else
        fnRead(RNDIS_interrupt_handle, ptrBuffer, LARGE_MESSAGE);        // read the content (flush) to the input buffer
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("Device notification: ");                         // we assume that RNDIS interrupt receptions are notifying that a response can now be read
        }
    #endif
        fnRNDIS_Control(REMOTE_NDIS_GET_RESPONSE);                       // request the response
    }
}

    #if (USB_CDC_COUNT > 1) && defined TELIT_LE910C1
// Handle potential modem interrupt endpoint reception
//
static void fnRNDIS_USB_InterruptModem(QUEUE_HANDLE Modem_interrupt_handle, unsigned char *ptrBuffer)
{
    while (fnMsgs(Modem_interrupt_handle) != 0) {                        // while reception from modem interrupt endpoint
        int i = 0;
        QUEUE_TRANSFER Length = fnRead(Modem_interrupt_handle, ptrBuffer, LARGE_MESSAGE); // read the content
        fnDebugMsg("Modem-interrupt = ");
        if (CDC_REQUEST_TYPE_NOTIFICATION == *ptrBuffer) {               // the modem is reporting its UART status
            CDC_NOTIFICATION_HEADER *ptrNotification = (CDC_NOTIFICATION_HEADER *)ptrBuffer;
            switch (ptrNotification->bNotificationCode) {                // handle recognised messages
            case CDC_NETWORK_CONNECTION:
                fnDebugMsg("Network connection");
                break;
            case CDC_RESPONSE_AVAILABLE:
                fnDebugMsg("Response available");
                break;
            case CDC_AUX_JACK_HOOK_STATE:
                fnDebugMsg("Jack hook state");
                break;
            case CDC_RING_DETECT:
                fnDebugMsg("Ring detect");
                break;
            case CDC_SERIAL_STATE:
                fnDebugMsg("Serial state");
                break;
            case CDC_CALL_STATE_CHANGE:
                fnDebugMsg("Call state");
                break;
            case CDC_LINE_STATE_CHANGE:
                fnDebugMsg("Line state");
                break;
            case CDC_CONNECTION_SPEED_CHANGE:
                fnDebugMsg("Connection speed");
                break;
            default:
                fnDebugMsg("Unknown");
                break;
            }
        }
        while (i++ < Length) {
            fnDebugHex(ptrBuffer[i++], (WITH_SPACE | WITH_LEADIN | sizeof(unsigned char)));
        }
        fnDebugMsg("\r\n");
    }
}
    #endif
#endif

#if (defined USE_USB_CDC && defined USB_CDC_RNDIS) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_CDC_HOST)
    #if (defined USB_TO_ETHERNET || defined USB_TO_TCP_IP) && (defined ETHERNET_BRIDGING || defined ETHERNET_FILTERED_BRIDGING)
// Bridging Ethernet reception content using an RNDIS data packet transmission
//
extern void fnBridgeEthernetFrame(ETHERNET_FRAME *ptr_rx_frame)
{
        #if defined USB_TO_TCP_IP && (IP_INTERFACE_COUNT > 1)
    if (ptr_rx_frame->ucInterface != (ETHERNET_INTERFACE >> INTERFACE_SHIFT)) {
        return;                                                          // source is not Ethernet interface (we bridge to the Ethernet interface after returning)
    }
        #endif
        #if (defined USE_USB_CDC && defined USB_CDC_RNDIS)
    if (RNDIS_DATA_INITIALISED != remoteNDIScontrol[DEFAULT_IP_INTERFACE].ucRNDIS_state) { // block bridging to the RNDIS host/device until ready
        return;
    }
    if (fnGetLinkState(DEFAULT_IP_INTERFACE) <= LAN_LINK_DOWN) {         // block bridging to the RNDIS if the Ethernet link is presently down
        return;
    }
        #endif
        #if defined USB_CDC_RNDIS && defined USB_TO_TCP_IP
    if (iHandlingRNDIS != 0) {
        return;                                                          // don't brige content from the RNDIS interface to itself
    }
        #endif
    if (USBPortID_comms[FIRST_CDC_RNDIS_HANDLE] != 0) {                  // serial interface handle is valid
        #if defined DEBUG_RNDIS_ON
        if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
            fnDebugMsg("In = ");
        }
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("Eth->RNDIS:");
        }
        if (fnSendRNDIS_Data(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], ptr_rx_frame->ptEth->ethernet_destination_MAC, 0, ptr_rx_frame->frame_size) == 0) {
            if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
                fnDebugMsg("FAILED\r\n");
            }
        }
        #else
        fnSendRNDIS_Data(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], ptr_rx_frame->ptEth->ethernet_destination_MAC, 0, ptr_rx_frame->frame_size);
        #endif
    }
}
    #endif
#endif

#if (defined USE_USB_CDC && defined USB_CDC_RNDIS && (defined USB_TO_ETHERNET || defined USB_TO_TCP_IP)) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
static QUEUE_TRANSFER fnSubmitRNDIS(QUEUE_HANDLE rndis_handle, unsigned char *ptrRNDIS_message, unsigned short usLength)
{
    QUEUE_TRANSFER iSent;
    #if defined _iMX
        #define MAX_RNDIS_WAIT_COUNT  2000000                            // maximum wait count
    #else
        #define MAX_RNDIS_WAIT_COUNT  2000                               // maximum wait count
    #endif
    volatile int iTimeout = 0;
    do {
    #if defined USB_TX_MESSAGE_MODE
        if (fnWrite(rndis_handle, 0, usLength) != 0) {                   // check that there is adequate space in the USB queue
            break;                                                       // space confirmed so we can write to the queue
        }
    #else
        if (fnWrite(rndis_handle, 0, sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)) != 0) { // check that the tx buffer is completely free
            fnFlush(rndis_handle, FLUSH_TX);                             // flush to avoid circular buffer operation (only the final frame transmission can be less that a full endpoint buffer size)
            break;                                                       // free buffer confirmed so we can start transmission
        }
    #endif
        if (++iTimeout >= MAX_RNDIS_WAIT_COUNT) {                        // if there was no queue space we repeat
            if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
                fnDebugMsg("RNDIS Tx Abort 0\r\n");
            }
            return 0;                                                    // timeout
        }
    } FOREVER_LOOP();
    #if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    iTimeout = 0;
    while (fnRNDIS_HostManagement(RNDIS_HOST_DATA_TRANSFER) != RNDIS_MANAGEMENT_NO_BLOCK) { // wait until control transfers complete before continuing (max. wait of around 100us expected)
        if (++iTimeout >= MAX_RNDIS_WAIT_COUNT) {                        // if the USB interface doesn't become free
            if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("RNDIS Tx Abort 1\r\n");
            fnDebugHex(sRNDIS_Host.ulHostManagement, (WITH_LEADIN | WITH_CR_LF | sizeof(sRNDIS_Host.ulHostManagement)));
        }
            return 0;                                                    // timeout
        }
    }
    fnRNDIS_HostManagement(RNDIS_HOST_STARTING_DATA);
    #endif
    fnDriver(rndis_handle, MODIFY_WAKEUP, (MODIFY_TX | OWN_TASK | (USB_RNDIS_HOST_TX_FREE_REF << 8))); // we want to be woken when the queue is free again (that is, when the complete data has been sent)
    iSent = fnWrite(rndis_handle, ptrRNDIS_message, usLength);           // write to the queue
    #if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    if (iSent == 0) {
        fnRNDIS_HostManagement(RNDIS_HOST_TERMINATED_DATA);              // since transmission was not possible we are already terminated
    }
    #endif
    return iSent;
}

// Send RNDIS payload
//
static QUEUE_TRANSFER fnSendRNDIS_Data(QUEUE_HANDLE rndis_handle, unsigned char *ptrData, REMOTE_NDIS_ETHERNET_PACKET_MESSAGE *ptrRNDIS_message, unsigned short usEthernetLength)
{
        #if defined DEBUG_RNDIS_ON
    if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
        fnDebugDec(usEthernetLength, 0);                                 // ethernet content length
    }
        #endif
    if (usEthernetLength > (sRNDIS_Host.sRNDIS_deviceDatails.ulRNDIS_MaxFrameSize + ETH_HEADER_LEN)) {
        #if defined DEBUG_RNDIS_ON
      //if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg(" Overlength\r\n");
      //}
        #endif
        return -1;                                                       // ignore overlength frames
    }
        #if (defined USE_USB_CDC && defined USB_CDC_RNDIS && (defined ETHERNET_BRIDGING || defined ETHERNET_FILTERED_BRIDGING)) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
    if (ptrData != 0) {                                                  // raw Ethernet buffer transmission
        REMOTE_NDIS_ETHERNET_PACKET_MESSAGE rndis_packet;                // temporary packet buffer
        unsigned short usPacketLength;
        uMemset(&rndis_packet.rndis_message, 0, sizeof(rndis_packet.rndis_message)); // start with zeroed header content
        rndis_packet.rndis_message.ucMessageType[0] = (unsigned char)REMOTE_NDIS_PACKET_MSG; // the message type
        rndis_packet.rndis_message.ucDataLength[0] = (unsigned char)usEthernetLength;
        rndis_packet.rndis_message.ucDataLength[1] = (unsigned char)(usEthernetLength >> 8);
        usPacketLength = (usEthernetLength + sizeof(REMOTE_NDIS_PACKET_MESSAGE));
        rndis_packet.rndis_message.ucMessageLength[0] = (unsigned char)usPacketLength; // valid paket length
        rndis_packet.rndis_message.ucMessageLength[1] = (unsigned char)(usPacketLength >> 8);
        rndis_packet.rndis_message.ucDataOffset[0] = (sizeof(REMOTE_NDIS_PACKET_MESSAGE) - 8);
        uMemcpy(&rndis_packet.ucRNDISPayload, ptrData, usEthernetLength);// copy the raw Ethernet payload
        if ((usPacketLength%RNDIS_DATA_ENDPOINT_SIZE) == 0) {            // in case the complete length is divisible by the endpoint size
            rndis_packet.ucRNDISPayload[usEthernetLength++] = 0;         // add a padding zero
            usPacketLength++;
        }
        #if defined DEBUG_RNDIS_ON
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg(":");
            fnDebugDec(usPacketLength, WITH_CR_LF);                      // usb content length (including possible padding)
        }
        #endif
        return (fnSubmitRNDIS(rndis_handle, (unsigned char *)&rndis_packet, usPacketLength));
    }
    else {                                                               // we know that the USB buffer can accept the length so we queue it to be send as soon as possible as a single USB message
        #endif
        uMemset(&ptrRNDIS_message->rndis_message, 0, sizeof(ptrRNDIS_message->rndis_message)); // start with zeroed header content
        ptrRNDIS_message->rndis_message.ucMessageType[0] = (unsigned char)REMOTE_NDIS_PACKET_MSG; // the message type
        ptrRNDIS_message->rndis_message.ucDataLength[0] = (unsigned char)usEthernetLength;
        ptrRNDIS_message->rndis_message.ucDataLength[1] = (unsigned char)(usEthernetLength >> 8);
        usEthernetLength += sizeof(REMOTE_NDIS_PACKET_MESSAGE);
        ptrRNDIS_message->rndis_message.ucMessageLength[0] = (unsigned char)usEthernetLength;
        ptrRNDIS_message->rndis_message.ucMessageLength[1] = (unsigned char)(usEthernetLength >> 8);
        ptrRNDIS_message->rndis_message.ucDataOffset[0] = (sizeof(REMOTE_NDIS_PACKET_MESSAGE) - 8);
        if ((usEthernetLength%RNDIS_DATA_ENDPOINT_SIZE) == 0) {          // if the length is a multiple of the endpoint size
            ((unsigned char *)ptrRNDIS_message)[usEthernetLength++] = 0; // add a padding zero
        }
        #if defined DEBUG_RNDIS_ON
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("-");
            fnDebugDec(usEthernetLength, WITH_CR_LF);                    // usb content length (including possible padding)
        }
        #endif
        return (fnSubmitRNDIS(rndis_handle, (unsigned char *)ptrRNDIS_message, usEthernetLength));
        #if (defined USE_USB_CDC && defined USB_CDC_RNDIS && (defined ETHERNET_BRIDGING || defined ETHERNET_FILTERED_BRIDGING)) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
    }
        #endif
}
#endif


#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
static unsigned long fnGetWord(unsigned char **ptr_ptr_data)
{
    unsigned long ulValue;
    unsigned char *ptrData = *ptr_ptr_data;
    ulValue = *ptrData++;
    ulValue |= (*ptrData++ << 8);
    ulValue |= (*ptrData++ << 16);
    ulValue |= (*ptrData++ << 24);
    *ptr_ptr_data = ptrData;
    return ulValue;
}
static void fnInsertWord(unsigned char *ptrBuffer, unsigned long ulWord)
{
    *ptrBuffer++ = (unsigned char)ulWord;
    *ptrBuffer++ = (unsigned char)(ulWord >> 8);
    *ptrBuffer++ = (unsigned char)(ulWord >> 16);
    *ptrBuffer = (unsigned char)(ulWord >> 24);
}

static int fnDeviceSupportsOID(unsigned long ulDeviceOID_value)
{
    unsigned char iIndex = 0;
    if (ulDeviceOID_value == OID_GEN_SUPPORTED_LIST) {
        return 0;                                                        // always accept the supported list OID
    }
    while (iIndex < (int)(sRNDIS_Host.sRNDIS_deviceDatails.ucDeviceOIDs)) {
        if (sRNDIS_Host.sRNDIS_deviceDatails.ulDeviceOIDs[iIndex++] == ulDeviceOID_value) {
            return 0;
        }
    }
    return -1;
}

static int fnRNDIS_HostManagement(int iEvent)
{
    int iRtn = 0;
    uDisable_Interrupt();                                                // protect against control endpoint interrupt callback
    switch (iEvent) {
    case RNDIS_HOST_STARTING_DATA:
        sRNDIS_Host.ulHostManagement |= RNDIS_HOST_MANAGEMENT_DATA_ACTIVE; // mark that we are actively transmitting a data packet
        break;
    case RNDIS_HOST_TERMINATED_DATA:
        sRNDIS_Host.ulHostManagement &= ~(RNDIS_HOST_MANAGEMENT_DATA_ACTIVE); // mark that we are no longer transmitting a data packet
        break;
    case RNDIS_HOST_POLL_INTERRUPT:                                      // the interrupt endpont should be polled
        if ((sRNDIS_Host.ulHostManagement & RNDIS_HOST_MANAGEMENT_DATA_ACTIVE) != 0) { // if we are presently transmitting a data packet the interrupt polling will be delayed
            sRNDIS_Host.ulHostManagement |= RNDIS_HOST_MANAGEMENT_DELAY_INT_POLL; // mark that the interrupt endpoint can be polled after the data packet has completed
            iRtn = RNDIS_MANAGEMENT_BLOCK_POLL;                          // poll should not be performed
        }
        else if ((sRNDIS_Host.ulHostManagement & (RNDIS_HOST_MANAGEMENT_CTL_ACTIVE | RNDIS_HOST_MANAGEMENT_CTL_TERMINATING)) != 0) {
            sRNDIS_Host.ulHostManagement |= RNDIS_HOST_MANAGEMENT_DELAY_CTL_INT_POLL; // mark that the interrupt endpoint can be polled after the control transaction has terminated
            iRtn = RNDIS_MANAGEMENT_BLOCK_POLL;                          // poll should not be performed
        }
        break;
    case RNDIS_HOST_DATA_PACKET_TERMINATED:
        if ((sRNDIS_Host.ulHostManagement & RNDIS_HOST_MANAGEMENT_DELAY_INT_POLL) == 0) {
            iRtn = RNDIS_MANAGEMENT_BLOCK_POLL;                          // poll should not be performed
        }
        sRNDIS_Host.ulHostManagement &= ~(RNDIS_HOST_MANAGEMENT_DATA_ACTIVE | RNDIS_HOST_MANAGEMENT_DELAY_INT_POLL);
        break;
    case RNDIS_HOST_POLL_DATA:
        if ((sRNDIS_Host.ulHostManagement & (RNDIS_HOST_MANAGEMENT_DATA_ACTIVE | RNDIS_HOST_MANAGEMENT_DELAY_INT_POLL | RNDIS_HOST_MANAGEMENT_CTL_ACTIVE | RNDIS_HOST_MANAGEMENT_CTL_TERMINATING)) != 0) {
            iRtn = RNDIS_MANAGEMENT_BLOCK_POLL;                          // poll should not be performed
        }
        break;
    case RNDIS_HOST_CTRL_TRANSACTION_START:                              // a control transaction is being started and we need to block interrupt endpoint polling and data packet transmission until it has completed
        sRNDIS_Host.ulHostManagement |= RNDIS_HOST_MANAGEMENT_CTL_ACTIVE;// mark that we are in a control transaction
        break;
    case RNDIS_HOST_CTRL_TERMINATING:                                    // the status stage of the control transfer has been successful and so other operations can take place from this point - the USB task will be informed (event from USB interrupt callback)
        sRNDIS_Host.ulHostManagement |= RNDIS_HOST_MANAGEMENT_CTL_TERMINATING; // mark that the control transaction has completed and can subsequently be terminated by the task, or a waiting data packet can be allowed to start
        break;
    case RNDIS_HOST_DATA_TRANSFER:
        if ((sRNDIS_Host.ulHostManagement & (RNDIS_HOST_MANAGEMENT_CTL_ACTIVE | RNDIS_HOST_MANAGEMENT_CTL_TERMINATING)) == RNDIS_HOST_MANAGEMENT_CTL_ACTIVE) {
#if defined _WINDOWS
            fnSimInts(0);                                                // allow pending USB interrupt to be handled if needed
#endif
            iRtn = RNDIS_MANAGEMENT_BLOCK_DATA;
        }
        break;
    case RNDIS_HOST_CTRL_TERMINATED:                                     // the USB task has learned that a control transfer has completed
        if ((sRNDIS_Host.ulHostManagement & RNDIS_HOST_MANAGEMENT_DELAY_CTL_INT_POLL) == 0) { // if there was no blocked interrupt endpoint poll we don't need to send one now
            iRtn = RNDIS_MANAGEMENT_BLOCK_POLL;                          // poll should not be performed
        }
        sRNDIS_Host.ulHostManagement &= ~(RNDIS_HOST_MANAGEMENT_DELAY_CTL_INT_POLL | RNDIS_HOST_MANAGEMENT_CTL_ACTIVE | RNDIS_HOST_MANAGEMENT_CTL_TERMINATING);
        break;
    }
    uEnable_Interrupt();
    return iRtn;
}

// Perform an interrupt endpoint poll if possible, or queue it to be performed when other transactions have completed
//
static void fnInterrupEndpointPoll(void)
{
    if (USBPortID_interrupt[FIRST_CDC_RNDIS_HANDLE] == NO_ID_ALLOCATED) {// no valid endpoint exists
        return;
    }
    if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_ACTIVE) == 0) {           // not RNDIS class
        return;
    }
    if (fnRNDIS_HostManagement(RNDIS_HOST_POLL_INTERRUPT) == RNDIS_MANAGEMENT_NO_BLOCK) { // if interrupt endpoint polling is not blocked
    #if !defined USB_HS_INTERFACE
        fnDriver(USBPortID_interrupt[FIRST_CDC_RNDIS_HANDLE], (RX_ON), 0); // perform IN interrupt poll (single IN will be sent)
        uTaskerGlobalMonoTimer(OWN_TASK, CDC_InterruptPollingRate[FIRST_CDC_RNDIS_INTERFACE], T_RNDIS_INTERRUPT_INTERVAL); // start the next interrupt endpoint interval
    #endif
    }
}

// Enable or re-enable RNDIS IN data endpoint polling
//
static void fnPollRNDIS_data(void)
{
#if !defined TELIT_LE910C1
    if (sRNDIS_Host.ucRNDIS_HostState != RNDIS_STATE_INITIALISED_AND_CONNECTED) { // if we are not initialised and connected we don't poll the RNDIS IN data endpoint
        return;
    }
#endif
    if (fnRNDIS_HostManagement(RNDIS_HOST_POLL_DATA) != RNDIS_MANAGEMENT_NO_BLOCK) {
        return;
    }
#if !defined USB_HS_INTERFACE
    fnDriver(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], (RX_ON), 0);       // enable IN polling on RNDIS data endpoint
#endif
}

static const OID_INFO *fnGetNextOID(const OID_INFO *ptrLastOID)
{
    if (ptrLastOID == 0) {                                               // on first use
        return (ulDeviceOID_config_list);                                // return first entry
    }
    ++ptrLastOID;                                                        // increment the entry on each call
    if (ptrLastOID->ulOID == 0) {                                        // check for end of list
        return 0;                                                        // end of list reached
    }
    else {
        return ptrLastOID;                                               // return the next entry
    }
}

static int fnCheckControlSetup(unsigned long ulMsg)
{
    if (REMOTE_NDIS_QUERY_LIST == ulMsg) {
        do {                                                             // work through the list of queries to be sent (if the device has stated its support for them)
            sRNDIS_Host.ptrOID = fnGetNextOID(sRNDIS_Host.ptrOID);
        } while ((sRNDIS_Host.ptrOID != 0) && (fnDeviceSupportsOID(sRNDIS_Host.ptrOID->ulOID) < 0));
        if (sRNDIS_Host.ptrOID == 0) {                                   // end of list reached
            return 1;                                                    // list is complete
        }
    }
    return 0;
}

static void fnSafeReleaseSetup(int iController)
{
#if defined PARALLEL_USD_DEVICE_INTERFACE                                // both high speed and full speed USB controllers used
    if (iController == USB_HS_HOST_REF) {                                // high speed controller doesn't need to data IN polling when sendingg cpntrol sequences
        return;
    }
#endif
#if !defined HSUSB_CONTROLLERS || (HSUSB_CONTROLLERS == 0)
    if (USBPortID_comms[FIRST_CDC_RNDIS_HANDLE] != NO_ID_ALLOCATED) {    // if the data endpoint is in operation
        fnDriver(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], (RX_OFF), 0);  // stop IN polling on the data interface (this will also wait for interrupt endpoint polling to complete if presently active)
        // If there was a pending IN reception it will have been handled as interrupt before returning so that we can be sure that nothing is pending when starting the control transaction
        //
    }
#endif
}

static void fnSend_RNDIS_hostControlSetup(unsigned long ulMsg)
{
    volatile int iRetries = 0;
    static USB_HOST_DESCRIPTOR rndis_control_setup = {0};
    unsigned short usLength = RNDIS_CONTROL_DATA_BUFFER_SIZE;
    unsigned char ucDirection = (STANDARD_HOST_TO_DEVICE | REQUEST_INTERFACE_CLASS);
    unsigned char ucRequest = 0;
    switch (ulMsg) {
    case REMOTE_NDIS_INITIALIZE_MSG:
        usLength = sizeof(REMOTE_NDIS_INITIALIZE_MESSAGE);               // 0x18 (24)
        break;
    case REMOTE_NDIS_GET_RESPONSE:                                       // request a response
        usLength = RNDIS_CONTROL_DATA_BUFFER_SIZE;
        ucDirection = (STANDARD_DEVICE_TO_HOST | REQUEST_INTERFACE_CLASS);
        ucRequest = 1;
        break;
    case REMOTE_NDIS_QUERY_LIST:
        usLength = (unsigned short)(sizeof(REMOTE_NDIS_QUERY_MESSAGE) + sRNDIS_Host.ptrOID->ulOID_data_length);
        break;
    case REMOTE_NDIS_KEEPALIVE_MSG:
        usLength = sizeof(REMOTE_NDIS_KEEPALIVE_MESSAGE);                // 0x0c (12)
        break;
    case REMOTE_NDIS_KEEPALIVE_COMPLETE:
        usLength = sizeof(REMOTE_NDIS_SET_KEEPALIVE_CMPLT);              // 0x10 (16)
        break;
    }
    rndis_control_setup.bmRecipient_device_direction = ucDirection;
    rndis_control_setup.bRequest = ucRequest;
    rndis_control_setup.wLength[0] = (unsigned char)usLength;
    rndis_control_setup.wLength[1] = (unsigned char)(usLength >> 8);
    uTaskerGlobalMonoTimer(OWN_TASK, RNDIS_CONTROL_TIMEOUT, T_RNDIS_COMMAND_TIMEOUT); // start the control timeout to monitor device failure
    fnRNDIS_HostManagement(RNDIS_HOST_CTRL_TRANSACTION_START);           // block any data transmission and interrupt endpoint polling until the control sequence has terminated
    fnSafeReleaseSetup(USB_HOST_REF);                                    // we stop any active IN polling (although the driver may do this automatically) to allow any pending In interrupts to be handled before we start the control transaction
    while (fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&rndis_control_setup, sizeof(rndis_control_setup)) == 0) { // return directly (non-buffered) - the data must remain stable [SETUP]
        if (++iRetries > 5) {
            fnDebugMsg("Failed!!!\r\n");
            return;                                                      // abort if the bus doesn't become free (probably due to USB reset)
        }
        fnDebugMsg("SETUP not sent!!!\r\n");                             // data transmission is probably in progress
        fnDelayLoop(100);                                                // delay 100us before retrying
    }
                                                                         // successful transmission of this control setup is handled in the control endpoint callback (event HOST_SETUP_ACKED)
}

// The class request (setup data) has been successfully sent and now we send further data belonging to it the request
//
static void fnSend_RNDIS_hostControlData(unsigned long ulMsg)
{
    #define MAX_RNDIS_CONTROL_DATA 64
    static unsigned char ucControlData[MAX_RNDIS_CONTROL_DATA] = {0};    // static memory used so that the content remains stable through USB transmission
    QUEUE_TRANSFER msg_len = 0;
    int iEchoID = 0;
    switch (ulMsg) {
    case REMOTE_NDIS_INITIALIZE_MSG:
        sRNDIS_Host.ulHostRequestID = 2;                                 // initial request ID (changes subsequently for each message)
        uMemcpy(ucControlData, &init_msg, sizeof(init_msg));             // send REMOTE_NDIS_INITIALIZE_MSG
        msg_len = sizeof(init_msg);
        fnDebugMsg("RNDIS Init\r\n");
        break;
    case REMOTE_NDIS_QUERY_LIST:
        uMemcpy(ucControlData, &query_msg, sizeof(query_msg));           // send REMOTE_NDIS_QUERY_MSG with OID_GEN_SUPPORTED_LIST
        msg_len = sizeof(query_msg);
        fnDebugMsg("Query: ");
        if (sRNDIS_Host.ptrOID->ulOID_data_length != 0) {
            unsigned long ulMessageLength;
            msg_len += (QUEUE_TRANSFER)(sRNDIS_Host.ptrOID->ulOID_data_length);
            if (sRNDIS_Host.ptrOID->ptrData != 0) {
                ucControlData[0] = (unsigned char)REMOTE_NDIS_SET_MSG;   // modify to REMOTE_NDIS_SET_MSG
                uMemcpy(&ucControlData[sizeof(query_msg)], sRNDIS_Host.ptrOID->ptrData, sRNDIS_Host.ptrOID->ulOID_data_length); // add data
            }
            else {
                uMemset(&ucControlData[sizeof(query_msg)], 0, sRNDIS_Host.ptrOID->ulOID_data_length); // fill with zeros
            }
            ulMessageLength = msg_len;
            fnInsertWord(&ucControlData[4], ulMessageLength);            // message length
            fnInsertWord(&ucControlData[12], sRNDIS_Host.ptrOID->ulOID);
            fnInsertWord(&ucControlData[16], sRNDIS_Host.ptrOID->ulOID_data_length);// information buffer length
        }
        break;
    case REMOTE_NDIS_KEEPALIVE_MSG:
        uMemcpy(ucControlData, &keepalive_msg, sizeof(keepalive_msg)); // send REMOTE_NDIS_KEEPALIVE_MSG
        msg_len = sizeof(keepalive_msg);
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("KEEP-ALIVE: ");
        }
        break;
    case REMOTE_NDIS_KEEPALIVE_COMPLETE:
        uMemcpy(ucControlData, &keepalive_response, sizeof(keepalive_response)); // send REMOTE_NDIS_KEEPALIVE_COMPLETE
        msg_len = sizeof(keepalive_response);
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("Keepalive response\r\n");
        }
        iEchoID = 1;
        break;
    }
    if (iEchoID != 0) {
        fnInsertWord(&ucControlData[8], sRNDIS_Host.ulDeviceRequestID);
    }
    else {
        fnInsertWord(&ucControlData[8], sRNDIS_Host.ulHostRequestID);
    }
    fnWrite(USB_control[USB_HOST_REF], ucControlData, msg_len);          // transmit the control transaction data
    sRNDIS_Host.ulHostRequestID++;                                       // increment the request ID ready for the next message
                                                                         // successful transmission of the control data is handled in the control endpoint callback (event HOST_SETUP_ACKED)
}

// Queue the control message for transmission
//
static int fnRNDIS_Control(unsigned long ulMessage)
{
    unsigned long ulThisMessage = 0;
    if (sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_in] != 0) {
        _EXCEPTION("Too many RNDIS control messages queued!!");
        return -1;
    }
    sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_in] = ulMessage; // put the new message into the queue
    if (sRNDIS_Host.ucRNDIS_queue_in == sRNDIS_Host.ucRNDIS_queue_out) { // if no control transactions in progress
        if (fnCheckControlSetup(ulMessage) != 0) {
            sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_in] = 0; // nothing to be sent so delete from the list
            return 1;
        }
        if (REMOTE_NDIS_GET_RESPONSE == ulMessage) {
            sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_in] = 0;
        }
        ulThisMessage = ulMessage;
    }
    if (++(sRNDIS_Host.ucRNDIS_queue_in) >= RNDIS_CONTROL_QUEUE_SIZE) {
        sRNDIS_Host.ucRNDIS_queue_in = 0;
    }
    if (ulMessage != 0) {                                                // if a message is to be sent
        fnSend_RNDIS_hostControlSetup(ulThisMessage);                    // start the transaction after the queue has been updated since the callback could occur so quickly that the queue is corrupted by it
    }
    return 0;
}

static int fnRNDIS_ControlData(void)
{
    if (sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_out] == 0) { // if nothing in control message queue
        if (++(sRNDIS_Host.ucRNDIS_queue_out) >= RNDIS_CONTROL_QUEUE_SIZE) {
            sRNDIS_Host.ucRNDIS_queue_out = 0;
        }
        return -1;
    }
    fnSend_RNDIS_hostControlData(sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_out]);
    sRNDIS_Host.ulRNDIS_ControlMessageQueue[sRNDIS_Host.ucRNDIS_queue_out] = 0;
    return 0;
}


static void fnDisplayOID(unsigned long ulOID, unsigned char *ptrData)
{
    fnDebugHex(ulOID, (sizeof(ulOID)));
    fnDebugMsg(" = ");
    switch (ulOID) {
        // General
        //
    case OID_GEN_SUPPORTED_LIST:
        fnDebugMsg("OID_GEN_SUPPORTED_LIST");
        break;
    case OID_GEN_HARDWARE_STATUS:
        fnDebugMsg("OID_GEN_HARDWARE_STATUS");
        break;
    case OID_GEN_MEDIA_SUPPORTED:
        fnDebugMsg("OID_GEN_MEDIA_SUPPORTED");
        break;
    case OID_GEN_MEDIA_IN_USE:
        fnDebugMsg("OID_GEN_MEDIA_IN_USE");
        break;
    case OID_GEN_MAXIMUM_LOOKAHEAD:
        fnDebugMsg("OID_GEN_MAXIMUM_LOOKAHEAD");
        break;
    case OID_GEN_MAXIMUM_FRAME_SIZE:
        fnDebugMsg("OID_GEN_MAXIMUM_FRAME_SIZE");
        if (ptrData != 0) {
            sRNDIS_Host.sRNDIS_deviceDatails.ulRNDIS_MaxFrameSize = fnGetWord(&ptrData);
        }
        break;
    case OID_GEN_LINK_SPEED:
        fnDebugMsg("OID_GEN_LINK_SPEED");
        break;
    case OID_GEN_TRANSMIT_BUFFER_SPACE:
        fnDebugMsg("OID_GEN_TRANSMIT_BUFFER_SPACE");
        break;
    case OID_GEN_RECEIVE_BUFFER_SPACE:
        fnDebugMsg("OID_GEN_RECEIVE_BUFFER_SPACE");
        break;
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
        fnDebugMsg("OID_GEN_TRANSMIT_BLOCK_SIZE");
        break;
    case OID_GEN_RECEIVE_BLOCK_SIZE:
        fnDebugMsg("OID_GEN_RECEIVE_BLOCK_SIZE");
        break;
    case OID_GEN_VENDOR_ID:
        fnDebugMsg("OID_GEN_VENDOR_ID");
        break;
    case OID_GEN_VENDOR_DESCRIPTION:
        fnDebugMsg("OID_GEN_VENDOR_DESCRIPTION");
        break;
    case OID_GEN_CURRENT_PACKET_FILTER:
        fnDebugMsg("OID_GEN_CURRENT_PACKET_FILTER");
        break;
    case OID_GEN_CURRENT_LOOKAHEAD:
        fnDebugMsg("OID_GEN_CURRENT_LOOKAHEAD");
        break;
    case OID_GEN_DRIVER_VERSION:
        fnDebugMsg("OID_GEN_DRIVER_VERSION");
        break;
    case OID_GEN_MAXIMUM_TOTAL_SIZE:
        fnDebugMsg("OID_GEN_MAXIMUM_TOTAL_SIZE");
        if (ptrData != 0) {
            sRNDIS_Host.sRNDIS_deviceDatails.ulRNDIS_MaxTotalSize = fnGetWord(&ptrData);
        }
        break;
    case OID_GEN_PROTOCOL_OPTIONS:
        fnDebugMsg("OID_GEN_PROTOCOL_OPTIONS");
        break;
    case OID_GEN_MAC_OPTIONS:
        fnDebugMsg("OID_GEN_MAC_OPTIONS");
        break;
    case OID_GEN_MEDIA_CONNECT_STATUS:
        fnDebugMsg("OID_GEN_MEDIA_CONNECT_STATUS");
        if (ptrData != 0) {
            sRNDIS_Host.sRNDIS_deviceDatails.ulRNDIS_ConnectStatus = fnGetWord(&ptrData);
        }
        break;
    case OID_GEN_MAXIMUM_SEND_PACKETS:
        fnDebugMsg("OID_GEN_MAXIMUM_SEND_PACKETS");
        break;
    case OID_GEN_VENDOR_DRIVER_VERSION:
        fnDebugMsg("OID_GEN_VENDOR_DRIVER_VERSION");
        break;
    case OID_GEN_XMIT_OK:
        fnDebugMsg("OID_GEN_XMIT_OK");
        break;
    case OID_GEN_RCV_OK:
        fnDebugMsg("OID_GEN_RCV_OK");
        break;
    case OID_GEN_XMIT_ERROR:
        fnDebugMsg("OID_GEN_XMIT_ERROR");
        break;
    case OID_GEN_RCV_ERROR:
        fnDebugMsg("OID_GEN_RCV_ERROR");
        break;
    case OID_GEN_RCV_NO_BUFFER:
        fnDebugMsg("OID_GEN_RCV_NO_BUFFER");
        break;
    case OID_GEN_DIRECTED_BYTES_XMIT:
        fnDebugMsg("OID_GEN_DIRECTED_BYTES_XMIT");
        break;
    case OID_GEN_DIRECTED_FRAMES_XMIT:
        fnDebugMsg("OID_GEN_DIRECTED_FRAMES_XMIT");
        break;
    case OID_GEN_MULTICAST_BYTES_XMIT:
        fnDebugMsg("OID_GEN_MULTICAST_BYTES_XMIT");
        break;
    case OID_GEN_MULTICAST_FRAMES_XMIT:
        fnDebugMsg("OID_GEN_MULTICAST_FRAMES_XMIT");
        break;
    case OID_GEN_BROADCAST_BYTES_XMIT:
        fnDebugMsg("OID_GEN_BROADCAST_BYTES_XMIT");
        break;
    case OID_GEN_BROADCAST_FRAMES_XMIT:
        fnDebugMsg("OID_GEN_BROADCAST_FRAMES_XMIT");
        break;
    case OID_GEN_DIRECTED_BYTES_RCV:
        fnDebugMsg("OID_GEN_DIRECTED_BYTES_RCV");
        break;
    case OID_GEN_DIRECTED_FRAMES_RCV:
        fnDebugMsg("OID_GEN_DIRECTED_FRAMES_RCV");
        break;
    case OID_GEN_MULTICAST_BYTES_RCV:
        fnDebugMsg("OID_GEN_MULTICAST_BYTES_RCV");
        break;
    case OID_GEN_MULTICAST_FRAMES_RCV:
        fnDebugMsg("OID_GEN_MULTICAST_FRAMES_RCV");
        break;
    case OID_GEN_BROADCAST_BYTES_RCV:
        fnDebugMsg("OID_GEN_BROADCAST_BYTES_RCV");
        break;
    case OID_GEN_BROADCAST_FRAMES_RCV:
        fnDebugMsg("OID_GEN_BROADCAST_FRAMES_RCV");
        break;
    case OID_GEN_RCV_CRC_ERROR:
        fnDebugMsg("OID_GEN_RCV_CRC_ERROR");
        break;
    case OID_GEN_TRANSMIT_QUEUE_LENGTH:
        fnDebugMsg("OID_GEN_TRANSMIT_QUEUE_LENGTH");
        break;
    case OID_GEN_GET_TIME_CAPS:
        fnDebugMsg("OID_GEN_GET_TIME_CAPS");
        break;
    case OID_GEN_GET_NETCARD_TIME:
        fnDebugMsg("OID_GEN_GET_NETCARD_TIME");
        break;


        // 802.3 Ethernet operational characteristics
        //
    case OID_802_3_PERMANENT_ADDRESS:
        fnDebugMsg("OID_802_3_PERMANENT_ADDRESS");
        if (ptrData != 0) {
            uMemcpy(sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_PermanentMAC, ptrData, sizeof(sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_PermanentMAC));
    #if defined USB_TO_TCP_IP
            uMemcpy(&network[DEFAULT_NETWORK].ucOurMAC[0], sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_PermanentMAC, MAC_LENGTH);
            uMemcpy(&temp_pars->temp_network[DEFAULT_NETWORK].ucOurMAC[0], sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_PermanentMAC, MAC_LENGTH);
    #endif
        }
        break;
    case OID_802_3_CURRENT_ADDRESS:
        fnDebugMsg("OID_802_3_CURRENT_ADDRESS");
        if (ptrData != 0) {
            uMemcpy(sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_CurrentMAC, ptrData, sizeof(sRNDIS_Host.sRNDIS_deviceDatails.ucRNDIS_CurrentMAC));
        }
        break;
    case OID_802_3_MULTICAST_LIST:
        fnDebugMsg("OID_802_3_MULTICAST_LIST");
        break;
    case OID_802_3_MAXIMUM_LIST_SIZE:
        fnDebugMsg("OID_802_3_MAXIMUM_LIST_SIZE");
        break;
    case OID_802_3_MAC_OPTIONS:
        fnDebugMsg("OID_802_3_MAC_OPTIONS");
        break;


        // 802.3 Ethernet statistics
        //
    case OID_802_3_RCV_ERROR_ALIGNMENT:
        fnDebugMsg("OID_802_3_RCV_ERROR_ALIGNMENT");
        break;
    case OID_802_3_XMIT_ONE_COLLISION:
        fnDebugMsg("OID_802_3_XMIT_ONE_COLLISION");
        break;
    case OID_802_3_XMIT_MORE_COLLISIONS:
        fnDebugMsg("OID_802_3_XMIT_MORE_COLLISIONS");
        break;
    case OID_802_3_XMIT_DEFERRED:
        fnDebugMsg("OID_802_3_XMIT_DEFERRED");
        break;
    case OID_802_3_XMIT_MAX_COLLISIONS:
        fnDebugMsg("OID_802_3_XMIT_MAX_COLLISIONS");
        break;
    case OID_802_3_RCV_OVERRUN:
        fnDebugMsg("OID_802_3_RCV_OVERRUN");
        break;
    case OID_802_3_XMIT_UNDERRUN:
        fnDebugMsg("OID_802_3_XMIT_UNDERRUN");
        break;
    case OID_802_3_XMIT_HEARTBEAT_FAILURE:
        fnDebugMsg("OID_802_3_XMIT_HEARTBEAT_FAILURE");
        break;
    case OID_802_3_XMIT_TIMES_CRS_LOST:
        fnDebugMsg("OID_802_3_XMIT_TIMES_CRS_LOST");
        break;
    case OID_802_3_XMIT_LATE_COLLISIONS:
        fnDebugMsg("OID_802_3_XMIT_LATE_COLLISIONS");
        break;
    default:
        fnDebugMsg("Unknown");
        break;
    }
}

static void fnRNDIS_connection(int iEvent)
{
    #if !defined RNDIS_INTERFACE
        #define RNDIS_INTERFACE       DEFAULT_IP_INTERFACE
    #endif
    #define RNDIS_MEDIA_DISCONNECTED 0
    #define RNDIS_MEDIA_CONNECTED    1
    #define RNDIS_INIT_COMPLETE      2
    unsigned char ucOriginalState = sRNDIS_Host.ucRNDIS_HostState;
    switch (iEvent) {
    case RNDIS_INIT_COMPLETE:                                            // initialisation sequence completed
        sRNDIS_Host.ucRNDIS_HostState |= RNDIS_STATE_INITIALISED;
        break;
    case RNDIS_MEDIA_DISCONNECTED:                                       // media has disconnected
        sRNDIS_Host.ucRNDIS_HostState &= ~(RNDIS_STATE_CONNECTED);
        break;
    case RNDIS_MEDIA_CONNECTED:                                          // media has connected
        sRNDIS_Host.ucRNDIS_HostState |= RNDIS_STATE_CONNECTED;
        break;
    }
    if (((ucOriginalState != sRNDIS_Host.ucRNDIS_HostState) && (sRNDIS_Host.ucRNDIS_HostState == RNDIS_STATE_INITIALISED_AND_CONNECTED)) // if newly initialised and connected
    #if defined TELIT_LE910C1
        || ((ucOriginalState == RNDIS_MEDIA_DISCONNECTED) && (sRNDIS_Host.ucRNDIS_HostState == RNDIS_STATE_INITIALISED)) // if newly initialised
    #endif
    ) {
    #if defined USE_DHCP_CLIENT
        if ((temp_pars->temp_parameters.usServers[DEFAULT_NETWORK] & ACTIVE_DHCP) != 0) {
            fnStartDHCP((UTASK_TASK)(FORCE_INIT | TASK_APPLICATION), (DHCP_CLIENT_OPERATION | defineNetwork(DEFAULT_NETWORK) | RNDIS_INTERFACE)); // activate DHCP client on this network
            fnDebugMsg("Starting DHCP\r\n");
        }
        fnDebugMsg("RNDIS operating\r\n");
    #endif
    }
}

static int fnInterpretRNDIS_deviceControlData(unsigned char *ptrControlData, int iLength)
{
    int i;
    unsigned char *ptrData = ptrControlData;
    unsigned long ulMessageType;
    unsigned long ulMessageLength;
    unsigned long ulStatus;
    unsigned long ulResponseID;
    unsigned long ulVersionMajor;
    unsigned long ulVersionMinor;
    unsigned long ulDeviceFlags;
    unsigned long ulEthernet;
    unsigned long ulMaxConcatonated;
    unsigned long ulMaxTransferSize;
    unsigned long ulAlignmentFactor;
    unsigned long ulInfoLength;
    unsigned long ulInfoOffset;
    unsigned long ulOID;
    #if defined RNDIS_HOST_DEBUG
    fnDebugMsg("Control message = ");
    for (i = 0; i < iLength; i++) {
        fnDebugHex(ptrControlData[i], (1 | WITH_LEADIN | WITH_SPACE));
        fnDebugMsg(",");
    }
    fnDebugMsg("\r\n");
    #endif
    ulMessageType = fnGetWord(&ptrData);
    ulMessageLength = fnGetWord(&ptrData);
    ulResponseID = fnGetWord(&ptrData);
    ulStatus = fnGetWord(&ptrData);
    switch (ulMessageType) {
    case REMOTE_NDIS_INITIALIZE_COMPLETE:
        fnDebugMsg("REMOTE_NDIS_INITIALIZE_COMPLETE");
        fnDebugMsg("\r\nVersion = ");
        ulVersionMajor = fnGetWord(&ptrData);
        fnDebugDec(ulVersionMajor, 0);
        fnDebugMsg(".");
        ulVersionMinor = fnGetWord(&ptrData);
        fnDebugDec(ulVersionMajor, WITH_CR_LF);
        ulDeviceFlags = fnGetWord(&ptrData);
        ulEthernet = fnGetWord(&ptrData);
        if (ulEthernet == 0) {
            fnDebugMsg("ETHERNET type\r\n");
        }
        else {
            fnDebugMsg("UNKNOWN type ");
            fnDebugDec(ulEthernet, WITH_CR_LF);
        }
        ulMaxConcatonated = fnGetWord(&ptrData);
        ulMaxTransferSize = fnGetWord(&ptrData);
        ulAlignmentFactor = fnGetWord(&ptrData);
        uTaskerGlobalMonoTimer(OWN_TASK, KEEPALIVE_TIME, T_RNDIS_KEEPALIVE); // start the keep-alive timer after the initialisation has succeeded
        break;
    case REMOTE_NDIS_QUERY_COMPLETE:
        fnDebugMsg("REMOTE_NDIS_QUERY_COMPLETE");
        ulInfoLength = fnGetWord(&ptrData);
        ulInfoOffset = fnGetWord(&ptrData);
        if (sRNDIS_Host.ptrOID->ulOID == OID_GEN_SUPPORTED_LIST) {       // requesting OID list
            fnDebugMsg("\r\nOIDs\r\n");
            sRNDIS_Host.sRNDIS_deviceDatails.ucDeviceOIDs = 0;
            while (ulInfoLength != 0) {
                ulOID = fnGetWord(&ptrData);
                fnDisplayOID(ulOID, 0);
                fnDebugMsg("\r\n");
                ulInfoLength -= sizeof(unsigned long);
                sRNDIS_Host.sRNDIS_deviceDatails.ulDeviceOIDs[sRNDIS_Host.sRNDIS_deviceDatails.ucDeviceOIDs++] = ulOID;
                if (sRNDIS_Host.sRNDIS_deviceDatails.ucDeviceOIDs >= MAX_DEVICE_OIDS) {
                    fnDebugMsg("List aborted!!\r\n");
                    break;
                }
            }
        }
        else {
            fnDebugMsg("\r\nOID ");
            fnDisplayOID(sRNDIS_Host.ptrOID->ulOID, ptrData);
            fnDebugMsg(" received [");
            if (ulInfoLength == 4) {
                unsigned long ulLongWord = fnGetWord(&ptrData);
                fnDebugHex(ulLongWord, (sizeof(ulLongWord) | WITH_LEADIN));
            }
            else {
                for (i = 0; i < (int)ulInfoLength; i++) {
                    fnDebugHex(ptrData[i], (1 | WITH_LEADIN | WITH_SPACE));
                }
            }
            fnDebugMsg("]\r\n");
        }
        break;
    case REMOTE_NDIS_SET_COMPLETE:                                       // the device is acknowledging that it has received and accepted our SET content
        fnDebugMsg("Set complete\r\n");
        break;
    case REMOTE_NDIS_HALT_MSG:
        fnDebugMsg("RNDIS Halt!\r\n");
        uTaskerGlobalStopTimer(OWN_TASK, T_RNDIS_KEEPALIVE);             // stop the keep-alive timer (T_RNDIS_KEEPALIVE)
        break;
    case REMOTE_NDIS_KEEPALIVE_COMPLETE:
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("OK\r\n");
        }
        uTaskerGlobalMonoTimer(OWN_TASK, KEEPALIVE_TIME, T_RNDIS_KEEPALIVE); // start the next keep-alive timer period
        break;
    case REMOTE_NDIS_KEEPALIVE_MSG:
        // A device can optionally send a keep alive message when the RNDIS interface has been idle for a certain period of time
        // - we need to respond with a REMOTE_NDIS_KEEPALIVE_COMPLETE message
        //
        sRNDIS_Host.ulDeviceRequestID = ulResponseID;                    // save the request ID so that we can return it in our response
        if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
            fnDebugMsg("Device Keep-alive!!\r\n");
        }
        fnRNDIS_Control(REMOTE_NDIS_KEEPALIVE_COMPLETE);                 // respond with a keep-alive complete
        return 0;
    case REMOTE_NDIS_INDICATE_STATUS_MSG:                                // indicating a change of state at the device or an error when an unrecognised message is received
        fnDebugMsg("RNDIS Status: ");
        switch (ulResponseID) {                                          // note that the status in this message is at the place of the response ID
        case RNDIS_STATUS_MEDIA_CONNECT:
            fnRNDIS_connection(RNDIS_MEDIA_CONNECTED);
            fnDebugMsg("Connected");
            break;
        case RNDIS_STATUS_MEDIA_DISCONNECT:
            fnRNDIS_connection(RNDIS_MEDIA_DISCONNECTED);
            fnDriver(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], (RX_OFF), 0); // stop IN polling on the data interface
            fnDebugMsg("Disconnected");
            break;
        case RNDIS_STATUS_SUCCESS:
            fnDebugMsg("OK");
            break;
        default:
            fnDebugHex(ulResponseID, (sizeof(ulResponseID) | WITH_LEADIN));
            break;
        }
        fnDebugMsg("\r\n");
        return 1;                                                        // the status information may not have been requested directly
    }
    uTaskerGlobalStopTimer(OWN_TASK, T_RNDIS_COMMAND_TIMEOUT);           // stop the command timer since we have received a response to a request
    return 0;                                                            // response to a query
}
#endif


#if defined TELIT_LE910C1
#define TELIT_MODEM_STATE_IDLE                             0
#define TELIT_MODEM_STATE_ENUMERATED                       1
#define TELIT_MODEM_STATE_COMMANDING_CONTROL_LINE_STATE    2
#define TELIT_MODEM_STATE_COMMANDING_SETUP_LINE_CODING     3
#define TELIT_MODEM_STATE_SENDING_LINE_CODING              4
#define TELIT_MODEM_STATE_INITIALISED                      5

static int fnTelitModemStateEvent(int iEvent, unsigned char ucEndpoint)
{
    static int iTelitModemState = TELIT_MODEM_STATE_IDLE;
    switch (iEvent) {
    case TELIT_ENUMERATED:                                               // enumeration of the modem has just completed
        iTelitModemState = TELIT_MODEM_STATE_ENUMERATED;
        // In order for the modem communication to operate we must:
        // - clear device feature (remote wakeup) - optional and not done
        // - clear endpoint feature (9 IN and 6 OUT)
        fnClearFeature(USB_control[USB_HOST_REF], (ucCDCBulkInEndpoint[SECOND_CDC_HOST_INTERFACE] | IN_ENDPOINT), USB_HOST_REF); // start by clearing/unstalling the IN endpoint
        // Send control transfer (setup 21 22 03 00 06 00 00 00)
        // Send control transfer (setup 21 20 00 00 06 00 07 00, data 00 c2 01 00 07 f8 08)
        // start polling ine IN bulk endpoint
        break;
    case TELIT_ENDPOINT_CLEARED:
        if (ucEndpoint == (ucCDCBulkInEndpoint[SECOND_CDC_HOST_INTERFACE] | IN_ENDPOINT)) {
            fnClearFeature(USB_control[USB_HOST_REF], (ucCDCBulkOutEndpoint[SECOND_CDC_HOST_INTERFACE]), USB_HOST_REF); // clear/unstall the OUT endpoint
        }
        else {
            static unsigned char ucControlLineState[8];
            ucControlLineState[0] = REQUEST_INTERFACE_CLASS;             // 0x21 
            ucControlLineState[1] = SET_CONTROL_LINE_STATE;              // 0x22
            ucControlLineState[2] = 0x03;
            ucControlLineState[3] = 0x00;
            ucControlLineState[4] = SECOND_CDC_HOST_INTERFACE;           // modem interface
            ucControlLineState[5] = 0x00;
            ucControlLineState[6] = 0x00;
            ucControlLineState[7] = 0x00;
            iTelitModemState = TELIT_MODEM_STATE_COMMANDING_CONTROL_LINE_STATE;
            fnWrite(USB_control[USB_HOST_REF], (unsigned char *)ucControlLineState, 8); // send set control line state
        }
        break;
    case MODEM_SET_LINE_CODING:
        {
            static unsigned char ucSetLineCoding[8];
            ucSetLineCoding[0] = REQUEST_INTERFACE_CLASS;                // 0x21
            ucSetLineCoding[1] = SET_LINE_CODING;                        // 0x20
            ucSetLineCoding[2] = 0x00;
            ucSetLineCoding[3] = 0x00;
            ucSetLineCoding[4] = SECOND_CDC_HOST_INTERFACE;              // modem interface
            ucSetLineCoding[5] = 0x00;
            ucSetLineCoding[6] = 0x07;
            ucSetLineCoding[7] = 0x00;
            iTelitModemState = TELIT_MODEM_STATE_COMMANDING_SETUP_LINE_CODING;
            fnWrite(USB_control[USB_HOST_REF], (unsigned char *)ucSetLineCoding, 8); // send set line coding setup
        }
        break;
    case MODEM_SET_LINE_CODING_DATA:
        {
            static unsigned char ucLineCoding[7];
            ucLineCoding[0] = 0x21;
            ucLineCoding[1] = 0x20;
            ucLineCoding[2] = 0x00;
            ucLineCoding[3] = 0x00;
            ucLineCoding[4] = SECOND_CDC_HOST_INTERFACE;                 // modem interface
            ucLineCoding[5] = 0x00;
            ucLineCoding[6] = 0x07;
            iTelitModemState = TELIT_MODEM_STATE_SENDING_LINE_CODING;
            fnWrite(USB_control[USB_HOST_REF], ucLineCoding, 7);         // send the line coding data
        }
        break;
    case MODEM_INITIALISATION_COMPLETE:
        fnDebugMsg("Start Modem IN polling\r\n");
        iTelitModemState = TELIT_MODEM_STATE_INITIALISED;
        fnDriver(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], (RX_ON), 0);// enable IN polling on the modem's CDC interface
        fnRNDIS_Control(REMOTE_NDIS_INITIALIZE_MSG);                     // after enumeration in RNDIS host mode the host starts the RNDIS initialisation with a REMOTE_NDIS_INITIALIZE_MSG
        uTaskerGlobalMonoTimer(OWN_TASK, CDC_InterruptPollingRate[FIRST_CDC_RNDIS_INTERFACE], T_RNDIS_INTERRUPT_INTERVAL); // start the interrupt endpoint interval (used by FS USB but not by HS USB)
        break;
    case TELIT_STATUS_STAGE_OK:                                          // this is an endpoint 0 callback in the USB interrupt
        switch (iTelitModemState) {
        case TELIT_MODEM_STATE_COMMANDING_CONTROL_LINE_STATE:
            fnInterruptMessage(OWN_TASK, MODEM_SET_LINE_CODING);         // send the defined event to the USB task
            return BUFFER_CONSUMED;                                      // this should not be handled by RNDIS
        case TELIT_MODEM_STATE_SENDING_LINE_CODING:                      // line coding data has been successfully sent
            fnInterruptMessage(OWN_TASK, MODEM_INITIALISATION_COMPLETE); // move to RNDIS initialisation
            return BUFFER_CONSUMED;                                      // this should not be handled by RNDIS
        }
        break;
    case TELIT_SETUP_ACKED:                                              // this is an endpoint 0 callback in the USB interrupt
        if (iTelitModemState < 5) {
            switch (iTelitModemState) {
            case TELIT_MODEM_STATE_COMMANDING_SETUP_LINE_CODING:         // set line coding setup has been successfully sent
                fnInterruptMessage(OWN_TASK, MODEM_SET_LINE_CODING_DATA);// send the defined event to the USB task
                return BUFFER_CONSUMED;
            }
            return INITIATE_IN_TOKEN;                                    // don't allow the RNDIS handler to work yet but cause an IN to be sent to either retrive the response or zero data
        }
        break;
    }
    return USB_HOST_ERROR;                                               // use this return value to indicate that the modem state-event machine should not stop the RNDIS handling
}
#endif

#if (defined USB_CDC_RNDIS || defined USB_RNDIS_HOST) && defined USB_TO_TCP_IP
static int fnUSBConfigEthernet(ETHTABLE *pars)
{
    if (ptrRNDIS_eth_packet == 0) {                                      // on first use
        ptrRNDIS_eth_packet = uMalloc(sizeof(REMOTE_NDIS_PACKET_MESSAGE) + pars->usSizeTx); // buffer for constructing data message in
    }
    return 0;
}

static int fnUSBGetQuantityRxBuf(void)
{
    return 0;                                                            // no reception buffer
}

static unsigned char *fnUSBGetTxBufferAdd(int iBufNr)
{
    return ptrRNDIS_eth_packet->ucRNDISPayload;                          // return a pointer to the payload area of the message
}

static int fnUSBWaitTxFree(void)
{
    return 0;
}

static void fnUSBPutInBuffer(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes)
{
    uMemcpy(ptrOut, ptrIn, nr_of_bytes);                                 // copy the data to the payload area
}

// Initiate transmission of prepared Ethernet content to USB-RNDIS interface
//
static QUEUE_TRANSFER fnUSBStartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    #if defined DEBUG_RNDIS_ON
    if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
        fnDebugMsg("RIn = ");
    }
    #endif
    #if defined USB_HOST_SUPPORT
        #if defined TELIT_LE910C1
    if ((sRNDIS_Host.ucRNDIS_HostState & RNDIS_STATE_INITIALISED) == 0) // ignore all transmission attempts when the interface is not initialised
        #else
    if (sRNDIS_Host.ucRNDIS_HostState != RNDIS_STATE_INITIALISED_AND_CONNECTED) // ignore all transmission attempts when the interface is not ready
        #endif
    {
        return 0;
    }
        #if defined DEBUG_RNDIS_ON
    if (ulRNDIS_debug_level != RNDIS_DEBUG_LEVEL_OFF) {
        fnDebugMsg("Stack->RNDIS:");
    }
        #endif
    #endif
    return (fnSendRNDIS_Data(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], 0, ptrRNDIS_eth_packet, DataLen)); // send as data packet
}

static void fnUSBFreeEthernetBuffer(int iBufNr)                          // dummy since not used
{
}

static void fnConfigRNDIS_interface(void)
{
    ETHTABLE ethernet;                                                   // configuration structure to be passed to the Ethernet configuration
    static const ETHERNET_FUNCTIONS USBEthernetFunctions = {
    fnUSBConfigEthernet,                                                 // configuration function for this interface
    fnUSBGetQuantityRxBuf,
    fnUSBGetTxBufferAdd,
    fnUSBWaitTxFree,
    fnUSBPutInBuffer,
    fnUSBStartEthTx,
    fnUSBFreeEthernetBuffer,
    #if defined USE_IGMP
    fnUSBModifyMulticastFilter,
    #endif
    };
    ethernet.ptrEthernetFunctions = (void *)&USBEthernetFunctions;       // enter the Ethernet function list for the default internal controller
    ethernet.Task_to_wake  = 0;
    #if defined ETH_INTERFACE
    ethernet.Channel       = 1;                                          // the Ethernet controller has channel 0 so RNDIS used Ethernet channel 1
    #else
    ethernet.Channel       = 0;                                          // default channel number
    #endif
    ethernet.usMode        = network[RNDIS_NETWORK].usNetworkOptions;    // options to be used by the interface
    #if defined USE_IPV6                                                 // {5}{11} generate an IPv6 link-local address from the MAC address
    ucLinkLocalIPv6Address[RNDIS_NETWORK][0]  = 0xfe;                    // link-local unicast
    ucLinkLocalIPv6Address[RNDIS_NETWORK][1]  = 0x80;                    // link-local unicast
                                                                         // intermediate values left at 0x00
    ucLinkLocalIPv6Address[RNDIS_NETWORK][8]  = (network[RNDIS_NETWORK].ucOurMAC[0] | 0x2); // invert the universal/local bit (since it is always '0' it means setting to '1')
    ucLinkLocalIPv6Address[RNDIS_NETWORK][9]  = network[RNDIS_NETWORK].ucOurMAC[1];
    ucLinkLocalIPv6Address[RNDIS_NETWORK][10] = network[RNDIS_NETWORK].ucOurMAC[2];
    ucLinkLocalIPv6Address[RNDIS_NETWORK][11] = 0xff;                    // insert standard 16 bit value to extend MAC-48 to EUI-64
    ucLinkLocalIPv6Address[RNDIS_NETWORK][12] = 0xfe;
    ucLinkLocalIPv6Address[RNDIS_NETWORK][13] = network[RNDIS_NETWORK].ucOurMAC[3];
    ucLinkLocalIPv6Address[RNDIS_NETWORK][14] = network[RNDIS_NETWORK].ucOurMAC[4];
    ucLinkLocalIPv6Address[RNDIS_NETWORK][15] = network[RNDIS_NETWORK].ucOurMAC[5];
    ethernet.usMode |= CON_MULTICAST;                                    // enable multicast when using IPV6
    #else
    uMemcpy(ethernet.ucMAC, &network[RNDIS_NETWORK].ucOurMAC[0], MAC_LENGTH); // the MAC address to be used by the interface
    #endif
    #if defined ETHERNET_BRIDGING
    ethernet.usMode        |= (PROMISCUOUS);                         // Ethernet bridging requires promiscuous operation
    #endif
    ethernet.usSizeTx      = (sizeof(ETHERNET_FRAME_CONTENT));       // transmit buffer size requested by user (Ethernet MTU)
    ethernet.ucEthTypes    = (ARP | IPV4);                           // enable reception of these protocols (used only by NE64 controller)
    ethernet.usExtEthTypes = 0;                                      // specify extended frame types (only used by NE64 controller)
    RNDIS_Handle = fnOpen(TYPE_ETHERNET, FOR_I_O, &ethernet);
    #if IP_INTERFACE_COUNT > 1
    fnEnterInterfaceHandle(RNDIS_IP_INTERFACE, RNDIS_Handle, ((INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING))); // enter the queue as RNDIS interface handler
    #else
    Ethernet_handle[0] = RNDIS_Handle;
    #endif
}
#endif



#if defined USE_USB_CDC && defined USB_CDC_RNDIS                         // RNDIS device
static void fnResetRNDIS(void)
{
    uMemset(remoteNDIScontrol, 0, sizeof(remoteNDIScontrol));
    remoteNDIScontrol[0].ucRNDIS_state = RNDIS_STATE_BUS_INITIALISED;
}

static void fnHandleRNDIS_EncapsulatedCommand(int iInterface)
{
    REMOTE_NDIS_CONTROL *ptrRNDIScontrol = &remoteNDIScontrol[iInterface];
    switch (remoteNDISreception[iInterface][0]) {                        // use just one byte of the message type used, so only one checked
    case (unsigned char)REMOTE_NDIS_INITIALIZE_MSG:                      // this is the first command that will be received after enumeration
        {
            REMOTE_NDIS_INITIALIZE_MESSAGE *ptrMsg = (REMOTE_NDIS_INITIALIZE_MESSAGE *)remoteNDISreception[iInterface];
            REMOTE_NDIS_INIT_COMPLETE_MESSAGE *ptrResponse = (REMOTE_NDIS_INIT_COMPLETE_MESSAGE *)ptrRNDIScontrol->remoteNDISresponse;
            ptrRNDIScontrol->ulMaxRNDISmessageLength = (ptrMsg->ucMaxTransferSize[0] | (ptrMsg->ucMaxTransferSize[1] << 8) | (ptrMsg->ucMaxTransferSize[2] << 16) | (ptrMsg->ucMaxTransferSize[3] << 24)); // the maximum message length that the host can receive
            uMemcpy(ptrResponse, &cRemoteNDISinitComplete, sizeof(REMOTE_NDIS_INIT_COMPLETE_MESSAGE)); // prepare standard response
            uMemcpy(ptrResponse->ucRequestID, ptrMsg->ucRequestID, sizeof(ptrMsg->ucRequestID)); // modify the ID to match the request
            ptrResponse->ucMessageLength[0] = sizeof(REMOTE_NDIS_INIT_COMPLETE_MESSAGE);
            if (ptrRNDIScontrol->ulMaxRNDISmessageLength >= sizeof(REMOTE_NDIS_INIT_COMPLETE_MESSAGE)) {
                ptrRNDIScontrol->encapsulatedResponseLength = sizeof(REMOTE_NDIS_INIT_COMPLETE_MESSAGE);
            }
            else {
                ptrRNDIScontrol->encapsulatedResponseLength = (QUEUE_TRANSFER)ptrRNDIScontrol->ulMaxRNDISmessageLength;
            }
            ptrRNDIScontrol->ucRNDIS_state = RNDIS_INITIALISED;
            fnDebugMsg("RNDIS Init.\r\n");
        }
        break;
    case (unsigned char)REMOTE_NDIS_HALT_MSG:
        fnDebugMsg("RNDIS halted!!\r\n");                                // a halt may be due to an initialisation error - in any case we should stop network operations (no response to this command)
        return;
    case (unsigned char)REMOTE_NDIS_SET_MSG:
    case (unsigned char)REMOTE_NDIS_KEEPALIVE_MSG:
    case (unsigned char)REMOTE_NDIS_QUERY_MSG:
        {
            #define DATA_TYPE_INVALID   -1
            #define DATA_TYPE_BUFFER    0
            #define DATA_TYPE_LONG_WORD 1
            REMOTE_NDIS_QUERY_MESSAGE *ptrMsg = (REMOTE_NDIS_QUERY_MESSAGE *)remoteNDISreception[iInterface];
            REMOTE_NDIS_QUERY_COMPLETE_MESSAGE *ptrResponse = (REMOTE_NDIS_QUERY_COMPLETE_MESSAGE *)ptrRNDIScontrol->remoteNDISresponse;
            unsigned char *ptrObjectData = ptrResponse->ucRequestID;
            int iDataType = DATA_TYPE_INVALID;
            unsigned long ulValue = 0;
            unsigned long ulLength = 0;
            unsigned char *ptrBuffer = 0;
            uMemset(ptrResponse, 0, sizeof(REMOTE_NDIS_QUERY_COMPLETE_MESSAGE)); // clear message content
            if (remoteNDISreception[iInterface][0] == (unsigned char)REMOTE_NDIS_KEEPALIVE_MSG) {
                ptrResponse->ucMessageType[0] = (unsigned char)REMOTE_NDIS_KEEPALIVE_COMPLETE;
                ptrResponse->ucMessageType[3] = (unsigned char)(REMOTE_NDIS_KEEPALIVE_COMPLETE >> 24);
                ptrResponse->ucMessageLength[0] = sizeof(REMOTE_NDIS_SET_COMPLETE_MESSAGE); // the keep alive response is the same as a set complete response
                ptrRNDIScontrol->encapsulatedResponseLength = ptrResponse->ucMessageLength[0];
                uMemcpy(ptrResponse->ucRequestID, ptrMsg->ucRequestID, sizeof(ptrMsg->ucRequestID)); // set the ID to match the request
                fnWrite(USBPortID_interrupt[iInterface], (unsigned char *)&ResponseAvailable, sizeof(ResponseAvailable)); // prepare response available response on interrupt endpoint
                fnDebugMsg("RNDIS keepalive\r\n");
                return;
            }
            switch (ptrMsg->ucOid[3]) {
            case OID_GENERAL_OBJECT:
                switch (ptrMsg->ucOid[2]) {
                case OID_GENERAL_OPERATIONAL_CHAR:
                    switch (ptrMsg->ucOid[0]) {
                    case (unsigned char)OID_GEN_SUPPORTED_LIST:          // mandatory - return a list of supported OIDs (this will be the first query received)
                        iDataType = DATA_TYPE_BUFFER;
                        ulLength = sizeof(supportedOIDs);                // information buffer length
                        ptrBuffer = (unsigned char *)supportedOIDs;      // list of supported OIDs
                        break;
                    case (unsigned char)OID_GEN_HARDWARE_STATUS:
                        break;
                    case (unsigned char)OID_GEN_MEDIA_SUPPORTED:
                        break;
                    case (unsigned char)OID_GEN_MEDIA_IN_USE:
                        break;
                    case (unsigned char)OID_GEN_MAXIMUM_LOOKAHEAD:
                        break;
                    case (unsigned char)OID_GEN_MAXIMUM_TOTAL_SIZE:      // mandatory
                    case (unsigned char)OID_GEN_MAXIMUM_FRAME_SIZE:      // mandatory
                        iDataType = DATA_TYPE_LONG_WORD;
                        ulValue = (ETH_MTU + ETH_HEADER_LEN);
                        break;
                    case (unsigned char)OID_GEN_LINK_SPEED:              // mandatory
                        iDataType = DATA_TYPE_LONG_WORD;
                        ulValue = (100000000/100);
                        break;
                    case (unsigned char)OID_GEN_TRANSMIT_BUFFER_SPACE:
                        break;
                    case (unsigned char)OID_GEN_RECEIVE_BUFFER_SPACE:
                        break;
                    case (unsigned char)OID_GEN_TRANSMIT_BLOCK_SIZE:
                        break;
                    case (unsigned char)OID_GEN_RECEIVE_BLOCK_SIZE:
                        break;
                    case (unsigned char)OID_GEN_VENDOR_ID:               // mandatory
                        iDataType = DATA_TYPE_LONG_WORD;
                        ulValue = (RDNIS_VENDOR_OUI | (unsigned char)iInterface);
                        break;
                    case (unsigned char)OID_GEN_VENDOR_DESCRIPTION:      // mandatory
                        iDataType = DATA_TYPE_BUFFER;
                        ulLength = (sizeof(cVendorDescription) - 1);     // information buffer length
                        ptrBuffer = (unsigned char *)cVendorDescription; // description of the RNDIS network card
                        break;
                    case (unsigned char)OID_GEN_CURRENT_PACKET_FILTER:
                        break;
                    case (unsigned char)OID_GEN_CURRENT_LOOKAHEAD:
                        break;
                    case (unsigned char)OID_GEN_DRIVER_VERSION:
                        break;
                    case (unsigned char)OID_GEN_PROTOCOL_OPTIONS:
                        break;
                    case (unsigned char)OID_GEN_MAC_OPTIONS:
                        break;
                    case (unsigned char)OID_GEN_MEDIA_CONNECT_STATUS:
                        {
    #if defined ETH_INTERFACE && !defined NO_USB_ETHERNET_BRIDGING
                            unsigned char ucLinkState = fnGetLinkState(DEFAULT_IP_INTERFACE);
                            iDataType = DATA_TYPE_LONG_WORD;
                            if (ucLinkState > LAN_LINK_DOWN) {
                                ulValue = NDIS_MEDIA_STATE_CONNECTED;
                            }
                            else if (ucLinkState == LAN_LINK_DOWN) {
                                ulValue = NDIS_MEDIA_STATE_DISCONNECTED;
                            }
                            else {
                                ulValue = NDIS_MEDIA_STATE_UNKNOWN;
                            }
    #else
                            iDataType = DATA_TYPE_LONG_WORD;
                            ulValue = NDIS_MEDIA_STATE_CONNECTED;
    #endif
                        }
                        break;
                    case (unsigned char)OID_GEN_MAXIMUM_SEND_PACKETS:
                        break;
                    case (unsigned char)OID_GEN_VENDOR_DRIVER_VERSION:
                        break;
                    default:
                        break;
                    }
                    break;
                case OID_GENERAL_STATISTICS:
                    switch (ptrMsg->ucOid[1]) {
                    case OID_GENERAL_STATISTICS_1:
                        switch (ptrMsg->ucOid[0]) {
                        case (unsigned char)OID_GEN_XMIT_OK:
                            //break;
                        case (unsigned char)OID_GEN_RCV_OK:
                            //break;
                        case (unsigned char)OID_GEN_XMIT_ERROR:
                            //break;
                        case (unsigned char)OID_GEN_RCV_ERROR:
                            //break;
                        case (unsigned char)OID_GEN_RCV_NO_BUFFER:
                            iDataType = DATA_TYPE_LONG_WORD;
                            ulValue = (0);
                            break;
                        default:
                            break;
                        }
                        break;
                    case OID_GENERAL_STATISTICS_2:
                        switch (ptrMsg->ucOid[0]) {
                        case (unsigned char)OID_GEN_DIRECTED_BYTES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_DIRECTED_FRAMES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_MULTICAST_BYTES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_MULTICAST_FRAMES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_BROADCAST_BYTES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_BROADCAST_FRAMES_XMIT:
                            //break;
                        case (unsigned char)OID_GEN_DIRECTED_BYTES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_DIRECTED_FRAMES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_MULTICAST_BYTES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_MULTICAST_FRAMES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_BROADCAST_BYTES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_BROADCAST_FRAMES_RCV:
                            //break;
                        case (unsigned char)OID_GEN_RCV_CRC_ERROR:
                            //break;
                        case (unsigned char)OID_GEN_TRANSMIT_QUEUE_LENGTH:
                            //break;
                        case (unsigned char)OID_GEN_GET_TIME_CAPS:
                            //break;
                        case (unsigned char)OID_GEN_GET_NETCARD_TIME:
                            iDataType = DATA_TYPE_LONG_WORD;
                            ulValue = (0);
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;
            case OID_ETHERNET_OBJECT:
                switch (ptrMsg->ucOid[2]) {
                case OID_ETHERNET_OPERATIONAL_CHAR:
                    switch (ptrMsg->ucOid[0]) {
                    case (unsigned char)OID_802_3_PERMANENT_ADDRESS:
                    case (unsigned char)OID_802_3_CURRENT_ADDRESS:
                        iDataType = DATA_TYPE_BUFFER;
                        ulLength = MAC_LENGTH;                           // information buffer length
                        ptrBuffer = network[RNDIS_NETWORK].ucOurMAC;     // mac address
                        break;
                    case (unsigned char)OID_802_3_MULTICAST_LIST:
                        break;
                    case (unsigned char)OID_802_3_MAXIMUM_LIST_SIZE:     // mandatory
                        #define RNDIS_MULTICAST_LIST_SIZE    0
                        iDataType = DATA_TYPE_LONG_WORD;
                        ulValue = (RNDIS_MULTICAST_LIST_SIZE);
                        break;
                    case (unsigned char)OID_802_3_MAC_OPTIONS:
                        break;
                    default:
                        break;
                    }
                    break;
                case OID_ETHERNET_STATISTICS:
                    switch (ptrMsg->ucOid[1]) {
                    case OID_ETHERNET_STATISTICS_1:
                        switch (ptrMsg->ucOid[0]) {
                        case (unsigned char)OID_802_3_RCV_ERROR_ALIGNMENT:
                            break;
                        case (unsigned char)OID_802_3_XMIT_ONE_COLLISION:
                            break;
                        case (unsigned char)OID_802_3_XMIT_MORE_COLLISIONS:
                            break;
                        default:
                            break;
                        }
                        break;
                    case OID_ETHERNET_STATISTICS_2:
                        switch (ptrMsg->ucOid[0]) {
                        case (unsigned char)OID_802_3_XMIT_DEFERRED:
                            break;
                        case (unsigned char)OID_802_3_XMIT_MAX_COLLISIONS:
                            break;
                        case (unsigned char)OID_802_3_RCV_OVERRUN:
                            break;
                        case (unsigned char)OID_802_3_XMIT_UNDERRUN:
                            break;
                        case (unsigned char)OID_802_3_XMIT_HEARTBEAT_FAILURE:
                            break;
                        case (unsigned char)OID_802_3_XMIT_TIMES_CRS_LOST:
                            break;
                        case (unsigned char)OID_802_3_XMIT_LATE_COLLISIONS:
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                }
                break;
            default:
                break;
            }
            if (remoteNDISreception[iInterface][0] == REMOTE_NDIS_SET_MSG) { // temp - act as if the set was accepted
                ptrResponse->ucMessageType[0] = (unsigned char)REMOTE_NDIS_SET_COMPLETE;
                ptrResponse->ucMessageType[3] = (unsigned char)(REMOTE_NDIS_SET_COMPLETE >> 24);
                ptrResponse->ucMessageLength[0] = sizeof(REMOTE_NDIS_SET_COMPLETE_MESSAGE);
                ptrRNDIScontrol->encapsulatedResponseLength = ptrResponse->ucMessageLength[0];
                uMemcpy(ptrResponse->ucRequestID, ptrMsg->ucRequestID, sizeof(ptrMsg->ucRequestID)); // set the ID to match the request
                fnWrite(USBPortID_interrupt[iInterface], (unsigned char *)&ResponseAvailable, sizeof(ResponseAvailable)); // prepare response available response on interrupt endpoint
                ptrRNDIScontrol->ucRNDIS_state = RNDIS_DATA_INITIALISED; // the device can now exchange any message allowed on the control or data channels
                fnDebugMsg("RNDIS Data\r\n");
    #if !defined ETH_INTERFACE || defined NO_USB_ETHERNET_BRIDGING
                fnInterruptMessage(TASK_USB, LAN_LINK_UP_100_FD);        // report link up so that the virtual adapter is operational without requiring the Ethernet connection
    #endif
                return;
            }
            ptrObjectData += 0x10;                                       // the object identifier's data is located at this offset from the request ID in the response
            switch (iDataType) {
            case DATA_TYPE_BUFFER:                                       // buffer
                uMemcpy(ptrObjectData, ptrBuffer, ulLength); // add the object identifier information
                break;
            case DATA_TYPE_LONG_WORD:
                *ptrObjectData++ = (unsigned char)ulValue;
                *ptrObjectData++ = (unsigned char)(ulValue >> 8);
                *ptrObjectData++ = (unsigned char)(ulValue >> 16);
                *ptrObjectData++ = (unsigned char)(ulValue >> 24);
                ulLength = 4;
                break;
            default:
                fnDebugMsg("Unsupported OID ");
                fnDebugHex(ptrMsg->ucOid[3], (sizeof(ptrMsg->ucOid[3]) | WITH_LEADIN));
                fnDebugHex(ptrMsg->ucOid[2], (sizeof(ptrMsg->ucOid[2])));
                fnDebugHex(ptrMsg->ucOid[1], (sizeof(ptrMsg->ucOid[1])));
                fnDebugHex(ptrMsg->ucOid[0], (sizeof(ptrMsg->ucOid[0]) | WITH_CR_LF));
                return;
            }
            ptrResponse->ucInformationBufferLength[0] = (unsigned char)ulLength;
            ptrResponse->ucMessageLength[0] = (sizeof(REMOTE_NDIS_QUERY_COMPLETE_MESSAGE) + ptrResponse->ucInformationBufferLength[0]);
            if (ptrRNDIScontrol->ulMaxRNDISmessageLength >= ptrResponse->ucMessageLength[0]) { // if the host can accept the length
                ptrRNDIScontrol->encapsulatedResponseLength = ptrResponse->ucMessageLength[0];
            }
            else {                                                       // else limit to host's input buffer size
                ptrRNDIScontrol->encapsulatedResponseLength = (QUEUE_TRANSFER)ptrRNDIScontrol->ulMaxRNDISmessageLength;
            }
            ptrResponse->ucMessageType[0] = (unsigned char)REMOTE_NDIS_QUERY_COMPLETE;
            ptrResponse->ucMessageType[3] = (unsigned char)(REMOTE_NDIS_QUERY_COMPLETE >> 24);
            uMemcpy(ptrResponse->ucRequestID, ptrMsg->ucRequestID, sizeof(ptrMsg->ucRequestID)); // set the ID to match the request
            ptrResponse->ucInformationBufferOffset[0] = 0x10;            // offset from the RequestId field to start of information buffer content
        }
        break;
    case (unsigned char)REMOTE_NDIS_RESET_MSG:
        fnResetRNDIS();
        fnDebugMsg("RNDIS reset!!\r\n");
        return;                                                          // REMOTE_NDIS_RESET_COMPLETE to be returned
    default:
    #if defined DEBUG_RNDIS_ON
        fnDebugMsg("RNDIS ??");
        if (ulRNDIS_debug_level == RNDIS_DEBUG_LEVEL_HIGH) {
            int i;
            for (i = 0; i < sizeof(remoteNDISreception); i++) {
                fnDebugHex(remoteNDISreception[iInterface][i], (sizeof(unsigned char) | WITH_LEADIN | WITH_SPACE));
            }
        }
        fnDebugMsg("\r\n");
    #else
        fnDebugMsg("RNDIS ?!!\r\n");
    #endif
        return;
    }
    // The response has been prepared so send a response available message on the interrupt endpoint which will cause the host to collect the response
    //
    fnWrite(USBPortID_interrupt[iInterface], (unsigned char *)&ResponseAvailable, sizeof(ResponseAvailable));
}

// This routine handles data belonging to an encapsulated command (class SETUP reception on control endpoint 0)
// - since there can be multiple data frames making up the message it is connected here and saved to the interface's buffer
// before scheduling the task to handle the complete command
//
static void fnRNDIS_control(unsigned char *ptrData, unsigned short usLength, unsigned short usExpected, int iInterface)
{
    unsigned char *ptrSet = remoteNDISreception[iInterface];             // the interface's reception buffer
    REMOTE_NDIS_CONTROL *ptrRNDIScontrol = &remoteNDIScontrol[iInterface];

    if (usExpected > sizeof(remoteNDISreception[iInterface])) {          // some protection against corrupted reception
        usExpected = sizeof(remoteNDISreception[iInterface]);
    }
    if (ptrRNDIScontrol->usTotalExpectControlData == 0) {
        ptrRNDIScontrol->usTotalExpectControlData = usExpected;          // the new total length that we are expecting
    }
    if (usLength > usExpected) {
        usLength = usExpected;
    }
    ptrSet += (ptrRNDIScontrol->usTotalExpectControlData - usExpected);
    uMemcpy(ptrSet, ptrData, usLength);

    if (usLength >= usExpected) {                                        // all data has been copied
        ptrRNDIScontrol->usTotalExpectControlData = 0;                   // reset
        fnInterruptMessage(OWN_TASK, (unsigned char)(EVENT_HANDLE_ENCAPSULATED_COMMAND + iInterface)); // schedule the task to handle the encapsulated command
    }
}

static void fnSendEncapsulatedResponse(int iInterface, unsigned short usMaxDataLength)
{
    if (remoteNDIScontrol[iInterface].encapsulatedResponseLength != 0) { // if an encapsulated response has been prepared
        fnWrite(USB_control[USB_DEVICE_REF], remoteNDIScontrol[iInterface].remoteNDISresponse, remoteNDIScontrol[iInterface].encapsulatedResponseLength);
        remoteNDIScontrol[iInterface].encapsulatedResponseLength = 0;
        if (remoteNDIScontrol[iInterface].statusResponseLength != 0) {   // if there is a status response also waiting
            fnWrite(USBPortID_interrupt[iInterface], (unsigned char *)&ResponseAvailable, sizeof(ResponseAvailable)); // report that the status response should be requested
            fnDebugMsg("Queue\r\n");
        }
        return;                                                          // return and leave and status change reports to be sent only when there are no encapsulated responses waiting
    }
    if (remoteNDIScontrol[iInterface].statusResponseLength != 0) {       // if a status change is to be reported
        if (remoteNDIScontrol[iInterface].statusResponseLength < usMaxDataLength) {
            usMaxDataLength = remoteNDIScontrol[iInterface].statusResponseLength;
        }
        fnWrite(USB_control[USB_DEVICE_REF], remoteNDIScontrol[iInterface].remoteNDISstatus, usMaxDataLength);
        remoteNDIScontrol[iInterface].statusResponseLength = 0;
        fnDebugMsg("Report\r\n");
        return;
    }
    fnDebugMsg("N\r\n");
}

static void fnRNDIS_report_link_state(void)
{
    if (remoteNDIScontrol[DEFAULT_IP_INTERFACE].ucRNDIS_state == RNDIS_DATA_INITIALISED) {
        REMOTE_NDIS_INDICATE_STATUS_MESSAGE *ptrStatus = (REMOTE_NDIS_INDICATE_STATUS_MESSAGE *)remoteNDIScontrol[DEFAULT_IP_INTERFACE].remoteNDISstatus;
        unsigned long ulMediaConnectState;
        uMemset(ptrStatus, 0, sizeof(remoteNDIScontrol[DEFAULT_IP_INTERFACE].remoteNDISstatus));
        ptrStatus->ucMessageType[0] = (unsigned char)REMOTE_NDIS_INDICATE_STATUS_MSG;
        ptrStatus->ucMessageLength[0] = sizeof(REMOTE_NDIS_INDICATE_STATUS_MESSAGE);
        ptrStatus->ucStatusBufferOffset[0] = sizeof(REMOTE_NDIS_INDICATE_STATUS_MESSAGE);
        remoteNDIScontrol[DEFAULT_IP_INTERFACE].statusResponseLength = ptrStatus->ucMessageLength[0];
    #if defined ETH_INTERFACE && !defined NO_USB_ETHERNET_BRIDGING
        if (fnGetLinkState(DEFAULT_IP_INTERFACE) > LAN_LINK_DOWN) {      // if the link is up
            ulMediaConnectState = RNDIS_STATUS_MEDIA_CONNECT;
        }
        else {                                                           // else the link is down
            ulMediaConnectState = RNDIS_STATUS_MEDIA_DISCONNECT;
        }
    #else
        fnDebugMsg("Link up\r\n");
        ulMediaConnectState = RNDIS_STATUS_MEDIA_CONNECT;
    #endif
        ptrStatus->ucStatus[0] = (unsigned char)ulMediaConnectState; 
        ptrStatus->ucStatus[1] = (unsigned char)(ulMediaConnectState >> 8);
        ptrStatus->ucStatus[2] = (unsigned char)(ulMediaConnectState >> 16);
        ptrStatus->ucStatus[3] = (unsigned char)(ulMediaConnectState >> 24);
        fnWrite(USBPortID_interrupt[0], (unsigned char *)&ResponseAvailable, sizeof(ResponseAvailable)); // prepare response available response on interrupt endpoint
    }
}
#endif

#endif

/********************************** RNDIS timer event handling **********************************/
#if defined RNDIS_HOST_TIMER_EVENTS
            case T_RNDIS_INTERRUPT_INTERVAL:                             // interrupt IN endpoint poll period expired
                fnInterrupEndpointPoll();                                // try to poll (if not possible it will be queued for repetition)
                fnPollRNDIS_data();                                      // restart data IN polling if connected
                break;
            case T_RNDIS_KEEPALIVE:                                      // keep alive perido expired
                fnRNDIS_Control(REMOTE_NDIS_KEEPALIVE_MSG);
                break;
            case T_RNDIS_COMMAND_TIMEOUT:
                fnDebugMsg("RNDIS command timeout!!\r\n");
                break;
#endif

/********************************** RNDIS interrupt event handling **********************************/
#if defined RNDIS_HOST_INTERRUPT_EVENTS
            case EVENT_RNDIS_DEVICE_CONTROL_OK:                          // RNDIS control has been successfully sent and we now wait for the device to respond on its interrupt endpoint
                if (fnRNDIS_HostManagement(RNDIS_HOST_CTRL_TERMINATED) == RNDIS_MANAGEMENT_NO_BLOCK) { // if an interrupt endpoint poll has been with-held due to the control transaction
                    fnInterrupEndpointPoll();                            // poll the interrupt endpoint how
                }
                fnPollRNDIS_data();                                      // re-start RNDIS data IN polling if connected
                break;
            case EVENT_RNDIS_CONTROL_DATA_READY:
                if (fnInterpretRNDIS_deviceControlData(sRNDIS_Host.ucControlDataReception, sRNDIS_Host.iWaitingControlDataLength) == 0) {
                    // The received message was expected so we continue the initialisation sequence
                    //
                    if (((sRNDIS_Host.ucRNDIS_HostState & RNDIS_STATE_INITIALISED) == 0) && (fnRNDIS_Control(REMOTE_NDIS_QUERY_LIST) > 0)) {   // queue the next query untill all queries have been sent
                        fnDebugMsg("RNDIS initialisation complete\r\n");
                        fnRNDIS_connection(RNDIS_INIT_COMPLETE);         // if connected we can start
                    }
                }
                sRNDIS_Host.iWaitingControlDataLength = 0;
                fnPollRNDIS_data();                                      // start/re-start IN polling on the data interface
    #if defined USB_HS_INTERFACE
                fnDriver(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE], (RX_ON), 0); // enable IN polling on RNDIS data endpoint
    #endif
                break;
    #if defined TELIT_LE910C1
            case MODEM_SET_LINE_CODING:
            case MODEM_SET_LINE_CODING_DATA:
            case MODEM_INITIALISATION_COMPLETE:
                fnTelitModemStateEvent(ucInputMessage[MSG_INTERRUPT_EVENT], 0);
                break;
    #endif
    #if defined USE_USB_CDC && defined USB_CDC_RNDIS
            case LAN_LINK_UP_100_FD:                                     // link state change
            case LAN_LINK_DOWN:
                fnRNDIS_report_link_state();                             // report the new link state if connected
                break;
    #endif
#endif
