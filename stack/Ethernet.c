/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      Ethernet.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    01.03.2007 EthernetStats array made conditional on USE_IP_STATS      {1}
    14.03.2007 Added RARP reception support                              {2}
    15.03.2007 Added VLAN support                                        {3}
    09.06.2008 Allow variable uNetwork protocol                          {4}
    10.01.2010 Add IPV6                                                  {5}
    27.08.2010 Limit to optionally handling only one reception at a time {6}
    06.09.2010 Add rx offload support                                    {7}
    25.08.2011 Modify optional handling of one reception frame at a time to operate without queued events and control
               maximum number of frames handled each pass                {8}
    10.11.2011 Quit initialisation if the Ethernet configuration fails   {9}
    11.03.2012 Modified to handle protocols carried by IPv6              {10}
    25.03.2012 Generate IPv6 link-local address from MAC-48              {11}
    16.04.2012 Add optional bridging call                                {12}
    07.10.2012 Return 0 from fnGetEthernetStats() if the parameter is invalid {13}
    16.12.2012 Add fnRemoveVLAN_tag() and fnAddVLAN_tag()                {14}
    16.12.2012 Add dynamic VLAN support                                  {15}
    17.12.2012 Changed fnHandleUDP() interface to allow IPv6 and multi-homed operation {16}
    06.08.2013 Add SUPPORT_MULTICAST_RX support (in promiscuous bridging mode) {17}
    16.04.2014 Extend IGMP counters and IP statistics to multiple networks {18}
    05.12.2015 Add ETHERNET_FILTERED_BRIDGING support                    {19}
    15.03.2016 Allow IPv4 broadcast/multicast reception to be handled on multiple networks {20}
    12.05.2017 Add optional Ethernet error flags support                 {21}

*/


/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"

#if defined ETH_INTERFACE || defined WIFI_INTERFACE|| defined USB_CDC_RNDIS || defined USB_RNDIS_HOST || defined USE_PPP

#if defined _WINDOWS
    extern void fnOpenDefaultHostAdapter(void);
#endif
#if defined WIFI_INTERFACE
    extern void fnHandleWifi_Event(void);
#endif

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#define OWN_TASK               TASK_ETHERNET

#if IP_NETWORK_COUNT > 1
    #define _NETWORK_ID        ucNetworkID
#else
    #define _NETWORK_ID        DEFAULT_NETWORK 
#endif
#if !defined ETHERNET_IP_INTERFACE
    #define ETHERNET_IP_INTERFACE (DEFAULT_IP_INTERFACE)
#endif


/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

#if defined USE_IP
    static void fnHandleIP_protocol(unsigned char ucNextProtocol, ETHERNET_FRAME *ptrRx_frame);
#endif
#if defined USE_IPV6
    static void fnHandleIPv6_protocol(ETHERNET_FRAME *ptrFrame);
#endif
#if defined WIFI_INTERFACE
    static CHAR fnEthernetMuxEvent(ETHERNET_FRAME *ptrFrame, int *ptr_iEthernetInterface);
    #if !defined REMOTE_SIMULATION_INTERFACE
    static int fnConfigWiFi(ETHTABLE *pars);
    static int fnGetQuantityWIFIRxBuf(void);
    static unsigned char *fnGetWIFI_TxBufferAdd(int iBufNr);
    static int fnWaitWIFI_TxFree(void);
    static void fnPutInBuffer_WIFI(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes);
    static QUEUE_TRANSFER fnStartEthTxWIFI(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
    static void fnFreeEthernetBufferWIFI(int iBufNr);
        #if defined USE_IGMP
    static void fnModifyMulticastFilterWIFI(QUEUE_TRANSFER action, unsigned char *ptrIP);
        #endif
    #endif
#elif defined ENC424J600_INTERFACE && (!defined ETHERNET_AVAILABLE || defined NO_INTERNAL_ETHERNET || ETHERNET_INTERFACES > 1) && !defined REMOTE_SIMULATION_INTERFACE
    static CHAR fnEthernetMuxEvent(ETHERNET_FRAME *ptrFrame, int *ptr_iEthernetInterface);
    static int fnConfigENC424J600(ETHTABLE *pars);
    static int fnGetQuantityENC424J600RxBuf(void);
    static unsigned char *fnGetENC424J600TxBufferAdd(int iBufNr);
    static int fnWaitENC424J600TxFree(void);
    static void fnPutInBuffer_ENC424J600(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes);
    static QUEUE_TRANSFER fnStartEthTxENC424J600(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
    static void fnFreeEthernetBufferENC424J600(int iBufNr);
        #if defined USE_IGMP
    static void fnModifyMulticastFilterENC424J600(QUEUE_TRANSFER action, unsigned char *ptrIP);
        #endif
#endif
#if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING
    static QUEUE_TRANSFER fnStartEthTx0(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
    static QUEUE_TRANSFER fnStartEthTx1(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
    static QUEUE_TRANSFER fnStartEthTx2(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
    static QUEUE_TRANSFER fnStartEthTx12(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
#endif


/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined ETH_INTERFACE && defined ETHERNET_RELEASE_AFTER_EVERY_FRAME  // {8}
    static const unsigned char ucEMAC_RX_INTERRUPT = EMAC_RX_INTERRUPT;
#endif
#if !defined ETHERNET_AVAILABLE || defined NO_INTERNAL_ETHERNET || (IP_INTERFACE_COUNT > 1) || defined USB_CDC_RNDIS || defined USB_RNDIS_HOST|| defined WIFI_INTERFACE
    #if defined ETH_INTERFACE && defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET
    static const ETHERNET_FUNCTIONS InternalEthernetFunctions = {
        fnConfigEthernet,                                                // configuration function for this interface
        fnGetQuantityRxBuf,
        fnGetTxBufferAdd,
        fnWaitTxFree,
        fnPutInBuffer,
        #if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING
        fnStartEthTx0,
        #else
        fnStartEthTx,
        #endif
        fnFreeEthernetBuffer,
        #if defined USE_IGMP
        fnModifyMulticastFilter,
        #endif
    };
    #endif
    #if defined ENC424J600_INTERFACE && !defined REMOTE_SIMULATION_INTERFACE
    static const ETHERNET_FUNCTIONS ENC424J600EthernetFunctions = {
        fnConfigENC424J600,                                              // configuration function for this interface
        fnGetQuantityENC424J600RxBuf,
        fnGetENC424J600TxBufferAdd,
        fnWaitENC424J600TxFree,
        fnPutInBuffer_ENC424J600,
        fnStartEthTxENC424J600,
        fnFreeEthernetBufferENC424J600,
        #if defined USE_IGMP
        fnModifyMulticastFilterENC424J600,
        #endif
    };
    #endif
    #if defined WIFI_INTERFACE && !defined REMOTE_SIMULATION_INTERFACE
    static const ETHERNET_FUNCTIONS WiFi_EthernetFunctions = {
        fnConfigWiFi,                                              // configuration function for this interface
        fnGetQuantityWIFIRxBuf,
        fnGetWIFI_TxBufferAdd,
        fnWaitWIFI_TxFree,
        fnPutInBuffer_WIFI,
        fnStartEthTxWIFI,
        fnFreeEthernetBufferWIFI,
        #if defined USE_IGMP
        fnModifyMulticastFilterWIFI,
        #endif
    };
    #endif
    #if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING
    static QUEUE_TRANSFER(*tail_tag_tx[3])(QUEUE_TRANSFER, unsigned char *) = { // call-back used to release a prepared transmit buffer
        fnStartEthTx1, fnStartEthTx2, fnStartEthTx12
    };
    #endif
#endif
/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

QUEUE_HANDLE Ethernet_handle[ETHERNET_INTERFACES] = {NO_ID_ALLOCATED};

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if defined USE_IP_STATS                                                 // {1}
    static unsigned long EthernetStats[IP_NETWORK_COUNT][sizeof(ETHERNET_STATS)/sizeof(unsigned long)] = {{0}}; // {18}
#endif
#if defined ENC424J600_INTERFACE
    static int iENC424J600_frames = 0;
#endif

#if defined ETH_INTERFACE || defined WIFI_INTERFACE
// Ethernet task
//
extern void fnTaskEthernet(TTASKTABLE *ptrTaskTable)
{
#if !defined ETHERNET_RELEASE_AFTER_EVERY_FRAME                          // {8} no queue used
    QUEUE_HANDLE PortIDInternal = ptrTaskTable->TaskID;                  // queue ID for task input
    unsigned char ucInputMessage[MEDIUM_MESSAGE];                        // reserve space for receiving messages
#elif defined ETHERNET_RELEASE_LIMIT
    int iMaxRxFrames = ETHERNET_RELEASE_LIMIT;                           // allow the task to handle up to this many reception frames before yielding
#endif
    int iEthernetInterface = 0;
    ETHERNET_FRAME rx_frame;
    signed char cEthernetBuffer;

    if (Ethernet_handle[ETHERNET_IP_INTERFACE] == NO_ID_ALLOCATED) {     // if not yet configured
        ETHTABLE ethernet;                                               // configuration structure to be passed to the Ethernet configuration
#if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1)
        ETHERNET_FUNCTIONS *EthernetPhyFunctions;
#endif
#if (defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET && (ETHERNET_INTERFACES > 1)) || defined USB_CDC_RNDIS || defined USB_RNDIS_HOST || defined USE_PPP
        ethernet.ptrEthernetFunctions = (void *)&InternalEthernetFunctions; // enter the Ethernet function list for the default internal controller
#endif
#if defined REMOTE_SIMULATION_INTERFACE                                  // when being uses as simulation extension node
        if (RemoteSimUDPSocket < 0) {                                    // if the USD socket has not yet been configured
            RemoteSimUDPSocket = fnGetUDP_socket(TOS_MINIMISE_DELAY, fnRemoteSimUDP_Listner, (UDP_OPT_SEND_CS | UDP_OPT_CHECK_CS));
            fnBindSocket(RemoteSimUDPSocket, REMOTE_SIM_UDP_PORT);       // bind socket and listen
            CONFIGURE_ENC424J600_SPI_MODE();                             // prepare the SPI for use
            fnConfigureENC424J600Interrupt();                            // enable interrupts
        }
#endif
        fnGetEthernetPars();                                             // we get the network parameters for the storage system before configuring
        ethernet.Task_to_wake  = OWN_TASK;                               // when there are Ethernet receptions this task will handle them
        ethernet.Channel       = 0;                                      // default channel number
        ethernet.usMode        = network[DEFAULT_NETWORK].usNetworkOptions; // options to be used by the interface
#if defined USE_IPV6                                                     // {5}{11} generate an IPv6 link-local address from the MAC address
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][0]  = 0xfe;              // link-local unicast
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][1]  = 0x80;              // link-local unicast
                                                                         // intermediate values left at 0x00
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][8]  = (network[DEFAULT_NETWORK].ucOurMAC[0] | 0x2); // invert the universal/local bit (since it is always '0' it means setting to '1')
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][9]  = network[DEFAULT_NETWORK].ucOurMAC[1];
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][10] = network[DEFAULT_NETWORK].ucOurMAC[2];
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][11] = 0xff;              // insert standard 16 bit value to extend MAC-48 to EUI-64
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][12] = 0xfe;
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][13] = network[DEFAULT_NETWORK].ucOurMAC[3];
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][14] = network[DEFAULT_NETWORK].ucOurMAC[4];
        ucLinkLocalIPv6Address[DEFAULT_NETWORK][15] = network[DEFAULT_NETWORK].ucOurMAC[5];
        ethernet.usMode |= CON_MULTICAST;                                // enable multicast when using IPV6
#else
        uMemcpy(ethernet.ucMAC, &network[DEFAULT_NETWORK].ucOurMAC[0], MAC_LENGTH); // the MAC address to be used by the interface (if not overridden to use a factory programmed one)
#endif
#if defined ETHERNET_BRIDGING                                            // {12}
        ethernet.usMode        |= (PROMISCUOUS);                         // Ethernet bridging requires promiscuous operation
#endif
        ethernet.usSizeRx      = LAN_BUFFER_SIZE;                        // receive buffer size requested by user
        ethernet.usSizeTx      = LAN_BUFFER_SIZE;                        // transmit buffer size requested by user
        ethernet.ucEthTypes    = (ARP | IPV4);                           // enable reception of these protocols (used only by NE64 controller)
        ethernet.usExtEthTypes = 0;                                      // specify extended frame types (only used by NE64 controller)
#if defined ETHERNET_AVAILABLE && defined ETH_INTERFACE && !defined NO_INTERNAL_ETHERNET // default internal controller
        Ethernet_handle[ETHERNET_IP_INTERFACE] = fnOpen(TYPE_ETHERNET, FOR_I_O, &ethernet); // open the channel with defined configuration
        if (Ethernet_handle[ETHERNET_IP_INTERFACE] == NO_ID_ALLOCATED) { // {9} if the hardware configuration failed
            return;                                                      // stop any further activity - the developer can catch this here and identify the cause in the hardware-specific initialisation (usually due to PHY not being detected)
        }
#endif
#if defined WIFI_INTERFACE
        ethernet.ptrEthernetFunctions = (void *)&WiFi_EthernetFunctions;
        Ethernet_handle[ETHERNET_IP_INTERFACE] = fnOpen(TYPE_ETHERNET, FOR_I_O, &ethernet); // open the channel with defined configuration
#endif
#if !defined ETHERNET_AVAILABLE || defined NO_INTERNAL_ETHERNET || (IP_INTERFACE_COUNT > 1)
    #if defined ETHERNET_AVAILABLE && !defined NO_INTERNAL_ETHERNET
        #define ENC424J600_INTERFACE_REFERENCE 1
        #if IP_INTERFACE_COUNT > 1
        fnEnterInterfaceHandle(ETHERNET_IP_INTERFACE, Ethernet_handle[ETHERNET_IP_INTERFACE], DEFAULT_INTERFACE_CHARACTERISTICS); // enter the local controller as interface handler
        #endif
    #else
        #define ENC424J600_INTERFACE_REFERENCE 0                         // external Ethernet controller is the default one
    #endif
    #if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING              
        do {
            EthernetPhyFunctions = uMalloc(sizeof(ETHERNET_FUNCTIONS));
            uMemcpy(EthernetPhyFunctions, &InternalEthernetFunctions, sizeof(InternalEthernetFunctions)); // share the ethernet driver
            EthernetPhyFunctions->fnConfigEthernet = 0;                  // but without re-initialisation of the hardware
            EthernetPhyFunctions->fnStartEthTx = (QUEUE_TRANSFER(*)(QUEUE_TRANSFER, unsigned char *))tail_tag_tx[ethernet.Channel]; // enter the appropriate tail tagging transmission routine
            ethernet.ptrEthernetFunctions = (void *)EthernetPhyFunctions;
            (ethernet.Channel)++;
            Ethernet_handle[ethernet.Channel] = fnOpen(TYPE_ETHERNET, FOR_I_O, &ethernet); // allocate further ethernet handles
        #if IP_INTERFACE_COUNT > 1
            fnEnterInterfaceHandle(ethernet.Channel, Ethernet_handle[ethernet.Channel], (DEFAULT_INTERFACE_CHARACTERISTICS));
        #endif
        } while (ethernet.Channel < (PHY_MULTI_PORT - 1));
        fnSetTailTagMode(1);                                             // enable tail-tagging mode -> to incorporate in driver
    #endif
    #if defined ENC424J600_INTERFACE && (!defined ETHERNET_AVAILABLE || defined NO_INTERNAL_ETHERNET || (IP_INTERFACE_COUNT > 1)) && !defined REMOTE_SIMULATION_INTERFACE
        ethernet.ptrEthernetFunctions = (void *)&ENC424J600EthernetFunctions;
        ethernet.Channel = ENC424J600_INTERFACE_REFERENCE;
        #if defined NO_INTERNAL_ETHERNET || defined DEVICE_WITHOUT_ETHERNET
        ethernet.usMode |= USE_FACTORY_MAC;                              // use the factory programmed MAC address from the Ethernet controller
        #endif
        Ethernet_handle[ENC424J600_INTERFACE_REFERENCE] = fnOpen(TYPE_ETHERNET, FOR_I_O, &ethernet);
        #if IP_INTERFACE_COUNT > 1
        fnEnterInterfaceHandle(ENC424J00_IP_INTERFACE, Ethernet_handle[ENC424J600_INTERFACE_REFERENCE], (INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING));
        #else
            #if !defined USE_IPV6
        uMemcpy(&network[DEFAULT_NETWORK].ucOurMAC[0], ethernet.ucMAC, MAC_LENGTH);
            #endif
        network[DEFAULT_NETWORK].usNetworkOptions |= NETWORK_VALUES_FIXED; // fix the values so that the MAC address doesn't get overwritten by reloading parameters
        #endif
    #endif
#endif
#if defined _WINDOWS && (defined ETH_INTERFACE && !defined NO_INTERNAL_ETHERNET)
        fnOpenDefaultHostAdapter();                                      // open the interface for simulation use
#endif
    }
#if defined WIFI_INTERFACE
    fnHandleWifi_Event();                                                // handle potential WIFI event
#endif
#if !defined ETHERNET_RELEASE_AFTER_EVERY_FRAME                          // {8}
    while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH)) {      // check task input queue
        switch (ucInputMessage[MSG_SOURCE_TASK]) {
        case INTERRUPT_EVENT:
            while ((cEthernetBuffer = fnEthernetEvent(&ucInputMessage[MSG_INTERRUPT_EVENT], &rx_frame)) >= 0)
#elif (defined ENC424J600_INTERFACE || defined WIFI_INTERFACE) && (!defined ETHERNET_AVAILABLE || !defined ETH_INTERFACE || defined NO_INTERNAL_ETHERNET || ETHERNET_INTERFACES > 1) && !defined REMOTE_SIMULATION_INTERFACE
            while ((cEthernetBuffer = fnEthernetMuxEvent(&rx_frame, &iEthernetInterface)) >= 0) // check for reception on multiple interface types
#else
            while ((cEthernetBuffer = fnEthernetEvent((unsigned char *)&ucEMAC_RX_INTERRUPT, &rx_frame)) >= 0) // check the ethernet interface for frame reception
#endif
            {
#if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING
                iEthernetInterface = rx_frame.ucRxPort;                  // the interface handling the reception
#endif
                if (rx_frame.frame_size != 0) {                          // don't handle zero length frames (errored)
                    fnHandleEthernetFrame(&rx_frame, Ethernet_handle[iEthernetInterface]);
                }
                fnFreeBuffer(Ethernet_handle[iEthernetInterface], cEthernetBuffer); // free up the reception buffer since we have completed working with it
#if defined ETHERNET_RELEASE_AFTER_EVERY_FRAME                           // {6}
    #if defined ETHERNET_RELEASE_LIMIT                                   // {8}
                if (--iMaxRxFrames >= 0) {                               // if the task has not yet handled the maximum number of frames allow it to continue running
                    continue;                                            // processes next possibly waiting reception
                }
    #endif
                uTaskerStateChange(OWN_TASK, UTASKER_ACTIVATE);          // schedule the task to check the input once more after allowing other tasks to run
                return;                                                  // quit for the moment
#endif
            }                                                            // end while frames waiting
#if !defined ETHERNET_RELEASE_AFTER_EVERY_FRAME                          // {8}
            break;

        default:
            break;
        }                                                                // end switch
    }                                                                    // end while input messages queued
#endif
}
#endif

extern int fnHandleEthernetFrame(ETHERNET_FRAME *ptr_rx_frame, QUEUE_HANDLE interface_handle)
{
#if defined ETHERNET_BRIDGING                                            // {12}
    int iBridgeFrame;                                                    // flag for bridging option
#endif
#if defined ETH_ERROR_FLAGS                                              // {21}
    ptr_rx_frame->ucErrorFlags = 0;                                      // initially no errors to report for this receptionETH_ERROR_FLAGS
#endif
#if defined IP_RX_CHECKSUM_OFFLOAD                                       // {7} the reception has flagged a corrupted frame
    if (ptr_rx_frame->frame_size == 0) {                                 // reception is invalid IPv4/v6 frame so discard
    #if defined ETHERNET_BRIDGING
        iBridgeFrame = 1;                                                // allow bridging corrupted frames in case they are a special protocol
    #endif
        goto _bridge_ethernet_buffer;
    }
#endif
#if IP_INTERFACE_COUNT > 1
    ptr_rx_frame->Tx_handle = interface_handle;                          // mark the interface handle to be used when responding directly
#endif
#if defined USE_IP || defined USE_IPV6
    #if defined SUPPORT_VLAN
        #if defined SUPPORT_DYNAMIC_VLAN                                 // {15} dynamic VLAN on reception
    if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] == (unsigned char)(TAG_PROTOCOL_IDENTIFIER >> 8)) && // check for VLAN tag
        (ptr_rx_frame->ptEth->ethernet_frame_type[1] == (unsigned char)(TAG_PROTOCOL_IDENTIFIER))) {
        ptr_rx_frame->ucVLAN_content = (VLAN_TAGGED_FRAME | VLAN_CONTENT_PRESENT); // flag that the frame is associated to a VLAN but has had the content removed
        fnRemoveVLAN_tag(ptr_rx_frame);                                  // remove the tag from the frame content
    }
    else {                                                               // no VLAN tag
        ptr_rx_frame->ucVLAN_content = 0;                                // frame is not VLAN tagged
        ptr_rx_frame->usVLAN_ID = 0xffff;                                // mark no VLAN
    }
        #else                                                            // standard VLAN operation on a single fixed VLAN
    if (vlan_active != 0) {                                              // if VLAN mode active, accept only frames to our identifier [vlan_active and vlan_vid are supplied by the user]
        if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] != (unsigned char)(TAG_PROTOCOL_IDENTIFIER  >> 8)) 
            || (ptr_rx_frame->ptEth->ethernet_frame_type[1] != (unsigned char)(TAG_PROTOCOL_IDENTIFIER))
            || ((ptr_rx_frame->ptEth->ucData[0] & 0x0f) != (unsigned char)((vlan_vid >> 8) & 0x0f)) 
            || (ptr_rx_frame->ptEth->ucData[1] != (unsigned char)vlan_vid)) {
            #if defined ETHERNET_BRIDGING
            iBridgeFrame = 1;                                            // allow bridging of VLAN tagged frames that don't match our vlan
            #endif
            goto _bridge_ethernet_buffer;
        }
        else {
            unsigned char *ptrTo = (unsigned char *)ptr_rx_frame->ptEth + (2 * MAC_LENGTH + (VLAN_TAG_LENGTH - 1));
            unsigned char *ptrFrom = (unsigned char *)ptr_rx_frame->ptEth + (2 * MAC_LENGTH - 1);
            while (ptrFrom >= (unsigned char *)ptr_rx_frame->ptEth) {
                *ptrTo-- = *ptrFrom--;                                   // shift the header to overwrite the VLAN tag, resulting in filtered frame
            }
            ptr_rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)((unsigned char *)ptr_rx_frame->ptEth + VLAN_TAG_LENGTH);
            ptr_rx_frame->frame_size -= VLAN_TAG_LENGTH;                 // {15}
        }
    }
        #endif
    #endif
#endif
#if defined ETHERNET_BRIDGING                                            // {12}
    if (uMemcmp(ptr_rx_frame->ptEth->ethernet_destination_MAC, cucBroadcast, MAC_LENGTH) == 0) { // if the frame is a broadcast
        iBridgeFrame = 2;                                               // broadcast frames must be passed on after processing by us
    }
    #if defined SUPPORT_MULTICAST_RX                                     // {17}
    else if ((ptr_rx_frame->ptEth->ethernet_destination_MAC[0] & 0x01) && (fnMulticastMemberMAC(ptr_rx_frame->ptEth->ethernet_destination_MAC) != 0)) {
        iBridgeFrame = 2;                                                // multicast frames must be passed on after processing by us
    }
    #endif
    else if (uMemcmp(ptr_rx_frame->ptEth->ethernet_destination_MAC, network[DEFAULT_NETWORK].ucOurMAC, MAC_LENGTH) != 0) {
        iBridgeFrame = 1;                                                // other frames not addressed to us are passed on
        goto _bridge_ethernet_buffer;                                    // don't handle locally but allow bridging
    }
    else {
        iBridgeFrame = 0;                                                // this is a frame to us so don't pass it on
    }
#endif
#if defined USE_IP
    #if defined SUPPORT_VLAN && defined SUPPORT_DYNAMIC_VLAN             // {15}
    if (fnVLAN_Accept(ptr_rx_frame) == 0) {                              // check whether the received frame may be handled locally [this routine must be supplied by the user]
        goto _bridge_ethernet_buffer;                                    // don't handle locally but allow bridging
    }
    #endif
    #if defined USE_IPV6                                                 // {5}
    if ((ptr_rx_frame->ptEth->ethernet_frame_type[1] == (unsigned char)(PROTOCOL_IPv6)) && (ptr_rx_frame->ptEth->ethernet_frame_type[0] == (unsigned char)(PROTOCOL_IPv6 >> 8))) { // check for IPV6
        fnHandleIPv6_protocol(ptr_rx_frame);                             // handle pure IPv6 frame content
        goto _bridge_ethernet_buffer;                                    // don't handle locally but allow bridging
    }
    #endif
    // Note that we check whether it is probably an ARP message by looking only at the second byte in the protocol field
    // This is for efficiency - only call ARP routine when it is probable that it is an ARP (the ARP routine then only checks first byte in type field
    //
    if (ptr_rx_frame->ptEth->ethernet_frame_type[0] == (unsigned char)(PROTOCOL_IPv4 >> 8)) { // if IPv4 type
        if ((ptr_rx_frame->ptEth->ethernet_frame_type[1] != (unsigned char)PROTOCOL_ARP)
    #if defined USE_RARP                                                 // {2}
            && (ptr_rx_frame->ptEth->ethernet_frame_type[1] != (unsigned char)PROTOCOL_RARP) 
    #endif
            || (fnProcessARP(ptr_rx_frame) == 0)) {                      // if (R)ARP, handle it
            if (fnHandleIPv4(ptr_rx_frame) != 0) {                       // if IPv4 (with matching address for the network) handle it
    #if defined USE_IPV6
                ptr_rx_frame->ucIPV6_Protocol = 0;                       // {10} not IPv6
                ptr_rx_frame->ucIPV4_Protocol = ptr_rx_frame->ptEth->ucData[IPV4_PROTOCOL_OFFSET]; // IPv4 protocol
    #endif
    #if (IP_NETWORK_COUNT > 1) && (IP_INTERFACE_COUNT < 2)               // {20}
                FOREVER_LOOP() {
    #endif
                    fnHandleIP_protocol(ptr_rx_frame->ptEth->ucData[IPV4_PROTOCOL_OFFSET], ptr_rx_frame); // {10} handle the next protocol layer (above IPv4)
    #if (IP_NETWORK_COUNT > 1) && (IP_INTERFACE_COUNT < 2)               // {20} when the reception is to be handled on multiple networks (broadcasts/matching multicasts)
                    ptr_rx_frame->ucBroadcastResponse &= ~(1 << ptr_rx_frame->ucNetworkID); // reset the network that the reception has been handled on
                    if (ptr_rx_frame->ucBroadcastResponse == 0) {        // if no additional networks are to handle the reception
                        break;                                           // complete
                    }
                    else {                                               // if additional networks are still valid
                        do {
                            ++(ptr_rx_frame->ucNetworkID);               // increment to the next possible network
                        } while ((ptr_rx_frame->ucBroadcastResponse & (1 << ptr_rx_frame->ucNetworkID)) == 0); // until the next valid one has been found
                    }
                }
    #endif
            }
        }                                                                // end if not ARP
    }                                                                    // end if PROTOCOL IP V4
    #if defined SUPPORT_DISTRIBUTED_NODES
        #if defined VARIABLE_PROTOCOL_UNETWORK                           // {4}
    else if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] == uc_uNetworkProtocol[0]) && (ptr_rx_frame->ptEth->ethernet_frame_type[1] == uc_uNetworkProtocol[1])) {
        fnHandle_unet(ptr_rx_frame->ptEth->ucData);
            #if defined ETHERNET_BRIDGING
        return 0;                                                        // never bridge uNetwork frames
            #endif
    }
        #else
    else if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] == (unsigned char)(PROTOCOL_UNETWORK>>8)) && (ptr_rx_frame->ptEth->ethernet_frame_type[1] == (unsigned char)(PROTOCOL_UNETWORK))) {
        fnHandle_unet(ptr_rx_frame->ptEth->ucData);
            #if defined ETHERNET_BRIDGING
        return 0;                                                        // never bridge uNetwork frames
            #endif
    }
        #endif
    #endif
#elif defined SUPPORT_DISTRIBUTED_NODES
    #if defined VARIABLE_PROTOCOL_UNETWORK                               // {4}
    if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] == uc_uNetworkProtocol[0]) && (ptr_rx_frame->ptEth->ethernet_frame_type[1] == uc_uNetworkProtocol[1])) {
        fnHandle_unet(ptr_rx_frame->ptEth->ucData);
            #if defined ETHERNET_BRIDGING
        return 0;                                                        // never bridge uNetwork frames
            #endif
    }
    #else
    if ((ptr_rx_frame->ptEth->ethernet_frame_type[0] == (unsigned char)(PROTOCOL_UNETWORK>>8)) && (ptr_rx_frame->ptEth->ethernet_frame_type[1] == (unsigned char)(PROTOCOL_UNETWORK))) {
         fnHandle_unet(ptr_rx_frame->ptEth->ucData);
            #if defined ETHERNET_BRIDGING
        return 0;                                                        // never bridge uNetwork frames
            #endif
    }
    #endif
#endif
#if defined ETHERNET_BRIDGING || defined IP_RX_CHECKSUM_OFFLOAD || defined SUPPORT_VLAN || defined USE_IPV6
_bridge_ethernet_buffer:
#endif
#if defined ETHERNET_BRIDGING                                            // {12}
    if (iBridgeFrame != 0) {                                             // if Ethernet frame is to be bridged
        fnBridgeEthernetFrame(ptr_rx_frame);                             // the user layer must supply this and can copy the data content to other interfaces
        return iBridgeFrame;                                             // the frame has been bridged
    }
    return 0;                                                            // frame was not bridged
#elif defined ETHERNET_FILTERED_BRIDGING                                 // {19}
    fnBridgeEthernetFrame(ptr_rx_frame);                                 // the user layer must supply this and can copy the data content to other interfaces
    return 1;                                                            // frame was bridged
#else
    return 0;                                                            // frame was not bridged
#endif
}

#if defined USE_IP
static void fnHandleIP_protocol(unsigned char ucNextProtocol, ETHERNET_FRAME *ptrRx_frame)
{
    switch (ucNextProtocol) {                                            // switch depending on the IP frame's protocol
    #if defined USE_ICMP
    case IP_ICMP:
        fnHandleICMP(ptrRx_frame);                                       // handle ICMP reception
        break;
    #endif
    #if defined USE_IPV6                                                 // {5}
    case IP_ICMPV6:
        fnHandleICMPV6(ptrRx_frame);
        break;
        #if defined USE_IPV6INV4
    case IP_6IN4:                                                        // IPv4 is being used to tunnel IPv6 content
        if (fnHandleIPV6in4(ptrRx_frame, ptrRx_frame->usIPLength) != 0) {
            fnHandleIPv6_protocol(ptrRx_frame);                          // tunneled IPv6 content is for the global IPv6 address
        }
        break;
    #    endif
    #endif
    #if defined USE_UDP
    case IP_UDP:
        fnHandleUDP(ptrRx_frame);                                        // {16}
        break;
    #endif
    #if defined USE_TCP
    case IP_TCP:
        fnHandleTCP(ptrRx_frame);                                        // {10}
        break;
    #endif
    #if defined USE_IGMP
    case IP_IGMPV2:
        fnHandleIGMP(ptrRx_frame);
        break;
    #endif
    default:
        break;                                                           // unsupported protocol is silently ignored
    }
}
#endif

#if defined USE_IPV6
static void fnHandleIPv6_protocol(ETHERNET_FRAME *ptrFrame)
{
    #if IP_NETWORK_COUNT > 1
    register unsigned char ucNetworkID;
    #endif
    IP_PACKET_V6 *ipv6 = (IP_PACKET_V6 *)ptrFrame->ptEth->ucData;
    ptrFrame->ucIPV4_Protocol = 0;                                       // not IPv4
    ptrFrame->ucIPV6_Protocol = ipv6->next_header;
    #if IP_NETWORK_COUNT > 1
    ptrFrame->ucBroadcastResponse = 0;                                   // default is no response to broadcasts - the user must decide the behaviour
    ucNetworkID = fnAssignNetwork(ptrFrame, ipv6->destination_IP_address);
    #endif
    #if defined USE_IP_STATS
    fnRxStats((unsigned char)(ptrFrame->ucIPV6_Protocol | IPV6_FRAME), _NETWORK_ID); // count IPv6 receptions
    #endif
    ptrFrame->usDataLength = ((ipv6->payload_length[0] << 8) | ipv6->payload_length[1]);
    ptrFrame->usIPLength = 40;                                           // payload located at this offset
    fnHandleIP_protocol(ptrFrame->ucIPV6_Protocol, ptrFrame);            // {10} handle the next protocol layer above IPv6
}
#endif

static unsigned char ucEthernetState[IP_NETWORK_COUNT] = {0};
extern void fnEthernetStateChange(int iInterface, unsigned char ucEvent)
{
    fnDebugMsg("Eth");
    fnDebugDec(iInterface, 0);
    fnDebugMsg(" link-");
    ucEthernetState[iInterface] = ucEvent;
    if (ucEvent == LAN_LINK_DOWN) {
        fnDebugMsg("down\r\n");
        return;
    }
    fnDebugMsg("up 10");
    if ((ucEvent == LAN_LINK_UP_100_FD) || (ucEvent == LAN_LINK_UP_100)) {
        fnDebugMsg("0");
    }
    if ((ucEvent == LAN_LINK_UP_100_FD) || (ucEvent == LAN_LINK_UP_10_FD)) {
        fnDebugMsg(" full");
    }
    else {
        fnDebugMsg(" half");
    }
    fnDebugMsg("-duplex\r\n");
}

extern unsigned char fnGetLinkState(int iInterface)
{
    return ucEthernetState[iInterface];
}

#if defined USE_IP_STATS
extern void fnIncrementEthernetStats(unsigned char ucStat, int iNetworkReference) // {18}
{
    EthernetStats[iNetworkReference][ucStat]++;
    if (ucStat <= LAST_RX_COUNTER) {
        EthernetStats[iNetworkReference][TOTAL_RX_FRAMES]++;
    }
    else if (ucStat <= LAST_TX_COUNTER) {
        EthernetStats[iNetworkReference][TOTAL_TX_FRAMES]++;
    }
}

extern unsigned long fnGetEthernetStats(unsigned char ucStat, int iNetworkReference) // {18}
{
    if (ucStat >= sizeof(ETHERNET_STATS)/sizeof(unsigned long)) {        // {13}
        return 0;
    }
    return EthernetStats[iNetworkReference][ucStat];                     // return the specified value
}

extern void fnTxStats(unsigned char ucProtType, int iNetworkReference)   // {18}
{
    unsigned char ucCounter;
    switch (ucProtType) {                                                // set counter according to protocol being sent
    case IP_UDP:
        ucCounter = SENT_UDP_FRAMES;
        break;

    case IP_TCP:
        ucCounter = SENT_TCP_FRAMES;
        break;
    #if defined USE_IGMP                                                 // {18}
    case IP_IGMPV2:
        ucCounter = SENT_IGMP_FRAMES;
        break;
    #endif
    case IP_ICMP:
        ucCounter = SENT_ICMP_FRAMES;
        break;
    #if defined USE_IPV6
    case IP_ICMPV6:
        ucCounter = TRANSMITTED_ICMPV6_FRAMES;
        break;
    case TRANSMITTED_TCPV6_FRAMES:
        ucCounter = ucProtType;
        break;
    #endif
    default:
        EthernetStats[iNetworkReference][TOTAL_TX_FRAMES]++;             // we have sent a frame with a protocol which is not specifically counted
        return;
    }
    fnIncrementEthernetStats(ucCounter, iNetworkReference);
}

extern void fnRxStats(unsigned char ucProtType, int iNetworkReference)   // {18}
{
    unsigned char ucCounter;
    switch (ucProtType & ~(FOREIGN_FRAME | IPV6_FRAME)) {                // set counter according to protocol being received
    case IP_UDP:
    #if defined USE_IPV6
        if (ucProtType & IPV6_FRAME) {
            ucCounter = RECEIVED_UDPV6_FRAMES;
        }
        else {
            ucCounter = RECEIVED_UDP_FRAMES;
        }
    #else
        ucCounter = RECEIVED_UDP_FRAMES;
    #endif
        break;
    case IP_TCP:
    #if defined USE_IPV6
        if (ucProtType & IPV6_FRAME) {
            ucCounter = RECEIVED_TCPV6_FRAMES;
        }
        else {
            ucCounter = RECEIVED_TCP_FRAMES;
        }
    #else
        ucCounter = RECEIVED_TCP_FRAMES;
    #endif
        break;
    #if defined USE_IGMP                                                 // {18}
    case IP_IGMPV2:
        ucCounter = RECEIVED_IGMP_FRAMES;
        break;
    #endif
    case IP_ICMP:
        ucCounter = RECEIVED_ICMP_FRAMES;
        break;
    #if defined USE_IPV6
    case IP_ICMPV6:
        ucCounter = RECEIVED_ICMPV6_FRAMES;
        break;
    #endif
    default:
        EthernetStats[iNetworkReference][UNSUPPORTED_PROTOCOL_FRAMES]++; // we have received a frame with a protocol which is not specifically supported
        EthernetStats[iNetworkReference][TOTAL_RX_FRAMES]++;
        return;
    }
    if (ucProtType & FOREIGN_FRAME) {
        ucCounter += (SEEN_FOREIGN_ICMP_FRAMES - RECEIVED_ICMP_FRAMES);  // set index to foreign counters
    }
    fnIncrementEthernetStats(ucCounter, iNetworkReference);
}

// Reset all IP counters
//
extern void fnDeleteEthernetStats(int iNetworkReference)
{
    uMemset(&EthernetStats[iNetworkReference], 0, sizeof(EthernetStats[iNetworkReference]));
}
#endif


#if defined PHY_MULTI_PORT && (PHY_MULTI_PORT > 1) && defined PHY_TAIL_TAGGING
// Release prepared ethernet transmission on switch
//
static QUEUE_TRANSFER fnStartEthTx0(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    while (DataLen < 60) {                                               // when tail tagging we need to fill out short frames
        *ptr_put++ = 0x00;                                               // pad with zeros if smaller than 60 [chip must send at least 60]
        DataLen++;
    }
    *ptr_put = 0;                                                        // specify port for the frame to be output to  
    return (fnStartEthTx(DataLen, ptr_put));
}

// Release prepared ethernet transmission on port 1
//
static QUEUE_TRANSFER fnStartEthTx1(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    while (DataLen < 60) {                                               // when tail tagging we need to fill out short frames
        *ptr_put++ = 0x00;                                               // pad with zeros if smaller than 60 [chip must send at least 60]
        DataLen++;
    }
    *ptr_put = 1;                                                        // specify port for the frame to be output to  
    return (fnStartEthTx(DataLen, ptr_put));
}

// Release prepared ethernet transmission on port 2
//
static QUEUE_TRANSFER fnStartEthTx2(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    while (DataLen < 60) {                                               // when tail tagging we need to fill out short frames
        *ptr_put++ = 0x00;                                               // pad with zeros if smaller than 60 [chip must send at least 60]
        DataLen++;
    }
    *ptr_put = 2;                                                        // specify port for the frame to be output to  
    return (fnStartEthTx(DataLen, ptr_put));
}

// Release prepared ethernet transmission on both ports 1 and 2
//
static QUEUE_TRANSFER fnStartEthTx12(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    while (DataLen < 60) {                                               // when tail tagging we need to fill out short frames
        *ptr_put++ = 0x00;                                               // pad with zeros if smaller than 60 [chip must send at least 60]
        DataLen++;
    }
    *ptr_put = 3;                                                        // specify port for the frame to be output to  
    return (fnStartEthTx(DataLen, ptr_put));
}
#endif


#if defined SUPPORT_VLAN && defined SUPPORT_DYNAMIC_VLAN                 // {14}
// Remove the VLAN content from a VLAN frame
//
extern void fnRemoveVLAN_tag(ETHERNET_FRAME *rx_frame)
{
    if ((rx_frame->ucVLAN_content & (VLAN_TAGGED_FRAME | VLAN_CONTENT_PRESENT)) == (VLAN_TAGGED_FRAME | VLAN_CONTENT_PRESENT)) { // only treat VLAN frames with content
        unsigned char *ptrTo = (unsigned char *)rx_frame->ptEth + (2 * MAC_LENGTH + (VLAN_TAG_LENGTH - 1));
        unsigned char *ptrFrom = (unsigned char *)rx_frame->ptEth + (2 * MAC_LENGTH - 1);
        rx_frame->usVLAN_ID = ((rx_frame->ptEth->ucData[0] & 0x0f) << 8);
        rx_frame->usVLAN_ID |= (rx_frame->ptEth->ucData[1]);             // note the VLAN ID of the reception frame
        while (ptrFrom >= (unsigned char *)rx_frame->ptEth) {
            *ptrTo-- = *ptrFrom--;                                       // shift the header to overwrite the VLAN tag, resulting in filtered frame
        }
        rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)((unsigned char *)rx_frame->ptEth + VLAN_TAG_LENGTH);
        rx_frame->frame_size -= VLAN_TAG_LENGTH;
        rx_frame->ucVLAN_content &= ~(VLAN_CONTENT_PRESENT);             // flag that the VLAN content has been removed
    }
}

// Insert the VLAN content in to a VLAN frame with removed content (beware that the buffer used by rx_frame->ptEth must allow data to be inserted VLAN_TAG_LENGTH before its initial value!!)
//
extern void fnAddVLAN_tag(ETHERNET_FRAME *rx_frame)                      // {14}
{
    if ((rx_frame->ucVLAN_content & (VLAN_TAGGED_FRAME | VLAN_CONTENT_PRESENT)) == (VLAN_TAGGED_FRAME)) { // only treat VLAN frames without content
        rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)((unsigned char *)rx_frame->ptEth - VLAN_TAG_LENGTH); // set the frame pointer back to the start of the original buffer
        uMemcpy(rx_frame->ptEth, &(rx_frame->ptEth->ethernet_destination_MAC[VLAN_TAG_LENGTH]), (2 * MAC_LENGTH)); // shift the MAC addresses back into their original position 
        rx_frame->ptEth->ethernet_frame_type[0] = (unsigned char)(TAG_PROTOCOL_IDENTIFIER >> 8);
        rx_frame->ptEth->ethernet_frame_type[1] = (unsigned char)(TAG_PROTOCOL_IDENTIFIER); // insert the VLAN type
        rx_frame->ptEth->ucData[0] = (unsigned char)(rx_frame->usVLAN_ID >> 8);
        rx_frame->ptEth->ucData[1] = (unsigned char)(rx_frame->usVLAN_ID); // insert the original VLAN ID
        rx_frame->frame_size += VLAN_TAG_LENGTH;
        rx_frame->ucVLAN_content |= (VLAN_CONTENT_PRESENT);              // flag that the VLAN content is contained in the frame
    }
}
#endif

#if defined ENC424J600_INTERFACE
    #include "EthernetController/ENC424J600.h"                           // include ENC424J600 driver
#endif
#if defined WIFI_INTERFACE && defined WIFI_ATWINC15
    #include "WiFi/ATWINC15x0/ATWINC15.h"                                // include ATWINC15 driver
#endif


#if defined REMOTE_SIMULATION_INTERFACE

// UDP data server - reception call back function
//
static int fnRemoteSimUDP_Listner(USOCKET SocketNr, unsigned char ucEvent, unsigned char *ucIP, unsigned short usPortNr, unsigned char *data, unsigned short usLength)
{
    unsigned short usThisLength;
    unsigned char  ucThisCommand = 0;
    switch (ucEvent) {
    case UDP_EVENT_RXDATA:
        while (usLength != 0) {
            usThisLength = usLength;
            switch (*data) {
    #if defined nRF24L01_INTERFACE
            case REMOTE_RF_DISABLE_RX_TX:
                DISABLE_RX_TX();                                         // set the enable line to '0'
                usThisLength = 2;
                break;
            case REMOTE_RF_ENABLE_RX_TX:
                ENABLE_RX_TX();                                         // set the enable line to '1'
                usThisLength = 2;
                break;
            case REMOTE_RF_W_COMMAND:
                ucThisCommand = *(data + 2);
                fnCommand_nRF24L01_0(ucThisCommand);                     // command the single byte command
                usThisLength = 3;
                break;
            case REMOTE_RF_W_REGISTER:
                ucThisCommand = *(data + 2);
                usThisLength = 3;
                break;
            case REMOTE_RF_W_REGISTER_VALUE:
                usThisLength = *(data + 1);
                fnWrite_nRF24L01(ucThisCommand, (data + 2), (unsigned char)usThisLength);
                usThisLength += 2;
                break;
    #endif
    #if defined ENC424J600_INTERFACE
            case REMOTE_ETH_CMD_WITHOUT_DATA:
                ucThisCommand = *(data + 3);
                usThisLength = 4;
                enc424j600ExecuteOp0(ucThisCommand);
                break;
            case REMOTE_ETH_CMD:
                ucThisCommand = *(data + 3);
                usThisLength = 4;
                break;
            case REMOTE_ETH_DATA:
                usThisLength = ((*(data + 1) << 8) + *(data + 2));
                if (usThisLength == 2) {
                    unsigned short usThisData = ((*(data + 3) << 8) + *(data + 4));
                    unsigned short usResult = enc424j600ExecuteOp16(ucThisCommand, usThisData);
                    if ((ucThisCommand & 0xc0) == 0) {                   // read command
                        RemoteSimUDP_Frame.ucUDP_Message[0] = REMOTE_ETH_INTERFACE;
                        RemoteSimUDP_Frame.ucUDP_Message[1] = REMOTE_ETH_RX_DATA;
                        RemoteSimUDP_Frame.ucUDP_Message[2] = 0;
                        RemoteSimUDP_Frame.ucUDP_Message[3] = 2;         // length
                        RemoteSimUDP_Frame.ucUDP_Message[4] = (unsigned char)(usResult >> 8);
                        RemoteSimUDP_Frame.ucUDP_Message[5] = (unsigned char)(usResult);
                        fnSendUDP(RemoteSimUDPSocket, ucRemoteSimIP_address, REMOTE_SIM_UDP_PORT, (unsigned char *)&RemoteSimUDP_Frame.tUDP_Header, (unsigned short)(6), 0); // send to the simulator
                    }
                    usThisLength += 2;
                }
                else if (usThisLength == 3) {
                    unsigned long ulThisData = ((*(data + 3) << 16) + (*(data + 4) << 8) + *(data + 5));
                    unsigned long ulResult = enc424j600ExecuteOp24(ucThisCommand, ulThisData);
                    if ((ucThisCommand & 0xc0) == 0) {                   // read command
                        RemoteSimUDP_Frame.ucUDP_Message[0] = REMOTE_ETH_INTERFACE;
                        RemoteSimUDP_Frame.ucUDP_Message[1] = REMOTE_ETH_RX_DATA;
                        RemoteSimUDP_Frame.ucUDP_Message[2] = 0;
                        RemoteSimUDP_Frame.ucUDP_Message[3] = 3;         // length
                        RemoteSimUDP_Frame.ucUDP_Message[4] = (unsigned char)(ulResult >> 16);
                        RemoteSimUDP_Frame.ucUDP_Message[5] = (unsigned char)(ulResult >> 8);
                        RemoteSimUDP_Frame.ucUDP_Message[6] = (unsigned char)(ulResult);
                        fnSendUDP(RemoteSimUDPSocket, ucRemoteSimIP_address, REMOTE_SIM_UDP_PORT, (unsigned char *)&RemoteSimUDP_Frame.tUDP_Header, (unsigned short)(7), 0); // send to the simulator
                    }
                    usThisLength += 2;
                }
                break;
            case REMOTE_ETH_DATA_BUFFER:
                usThisLength = ((*(data + 1) << 8) + *(data + 2));
                enc424j600WriteN(ucThisCommand, (data + 3), usThisLength); // write data - no read performed
                usThisLength += 2;
                break;
            case REMOTE_ETH_DATA_RX:
                usThisLength = ((*(data + 1) << 8) + *(data + 2));
                RemoteSimUDP_Frame.ucUDP_Message[0] = REMOTE_ETH_INTERFACE;
                RemoteSimUDP_Frame.ucUDP_Message[1] = REMOTE_ETH_RX_DATA;
                RemoteSimUDP_Frame.ucUDP_Message[2] = (unsigned char)(usThisLength << 8);
                RemoteSimUDP_Frame.ucUDP_Message[3] = (unsigned char)usThisLength;
                enc424j600ReadN(ucThisCommand, &RemoteSimUDP_Frame.ucUDP_Message[4], usThisLength);
                fnSendUDP(RemoteSimUDPSocket, ucRemoteSimIP_address, REMOTE_SIM_UDP_PORT, (unsigned char *)&RemoteSimUDP_Frame.tUDP_Header, (unsigned short)(usThisLength + 4), 0); // send to the simulator
                usThisLength += 2;
                break;
    #endif
            default:
                break;
            }
            if (usThisLength >= usLength) {
                break;
            }
            data += usThisLength;
            usLength -= usThisLength;
        }
        break;

    case UDP_EVENT_PORT_UNREACHABLE:                                     // we have received information that this port is not available at the destination so quit
        break;
    }
    return 0;
}
#endif
#endif