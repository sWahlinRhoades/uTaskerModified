/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    --------------------------------------------------------------------
    File:      ppp.c
    Project:   Single Chip Embedded Internet
    --------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2018
    ********************************************************************
    09.05.2017 SLIP [RFC1055] implemented (PPP is not presently used but shows an initial test framework)
    02.06.2018 Zero optional user UART callback handlers                 {1}
    
 */


#include "config.h"


#if defined USE_PPP

#define TEMP_LWIPP_PPP                                                   // temporary using lwip PPP
//#define UTASKER_PPP_LOW_LAYER                                          // replace PPP lowest layer by uTasker implementation
extern void start_ppp_now(unsigned char uart_handle);
extern void pass_raw_input(unsigned char newByte);
extern int fnSendPPP(unsigned char *buffer, int iLength);
void (*ppp_callback)(void *) = 0;
void *ppp_parameter = 0;

 /* =================================================================== */
 /*                          local definitions                          */
 /* =================================================================== */

#define OWN_TASK TASK_PPP

#define PPP_RX_BUFFER_SIZE   128                                         // space in driver rx buffer
#define PPP_TX_BUFFER_SIZE   2048                                        // space in driver tx buffer
#define PPP_RX_BUFFER_SPACE  2028                                        // local buffer space

#define PPP_STATE_INIT       0
#define PPP_STATE_IDLE       1
#define PPP_STATE_CLIENT     2
#define PPP_STATE_IN_FRAME   3

#define PPP_INIT_CRC         0xffff
#define PPP_GOOD_CRC         0xf0b8

#define SLIP_END_CHARACTER           192                                 // octal 300 or hex 0xc0
#define SLIP_ESCAPE_CHARACTER        219                                 // octal 333 or hex 0xdb
#define SLIP_ESCAPE_END_CHARACTER    220                                 // octal 334 or hex 0xdc
#define SLIP_ESCAPE_ESCAPE_CHARACTER 221                                 // octal 335 or hex 0xdd

#if !defined PPP_NETWORK
    #define PPP_NETWORK              0
#endif

#define E_TIMER_PPP                  1

#define SLIP_MAX_DATAGRAM            1006


 /* =================================================================== */
 /*                      local structure definitions                    */
 /* =================================================================== */

 /* =================================================================== */
 /*                 local function prototype declarations               */
 /* =================================================================== */


static int fnPPP_ConfigEthernet(ETHTABLE *pars);
static int fnPPP_GetQuantityRxBuf(void);
static unsigned char *fnPPP_GetTxBufferAdd(int iBufNr);
static int fnPPP_WaitTxFree(void);
static void fnPPP_PutInBuffer(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes);
static QUEUE_TRANSFER fnPPP_StartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put);
static void fnPPP_FreeEthernetBuffer(int iBufNr);
#if defined USE_IGMP
    static void fnPPP_ModifyMulticastFilter(QUEUE_TRANSFER action, unsigned char *ptrIP);
#endif

#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
    static int fnHandlePPP_frame(unsigned char *ptrFrame, QUEUE_TRANSFER ppp_frame_length);
    static unsigned short fnPPP_crc(unsigned short crc_value, unsigned char ucData);
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined USE_SLIP
static const unsigned char cEscapeEscapeEnd[] = { SLIP_ESCAPE_CHARACTER, SLIP_ESCAPE_ESCAPE_CHARACTER, SLIP_ESCAPE_END_CHARACTER };
#endif

static const ETHERNET_FUNCTIONS PPP_EthernetFunctions = {
    fnPPP_ConfigEthernet,                                                // configuration function for this interface
    fnPPP_GetQuantityRxBuf,                                              // call-back used to get the number of available receive buffers
    fnPPP_GetTxBufferAdd,                                                // call-back used to get a memory-mapped buffer address
    fnPPP_WaitTxFree,                                                    // call-back used to allow waiting on transmit buffer availability
    fnPPP_PutInBuffer,                                                   // call-back used to prepare transmit data to the output buffer
    fnPPP_StartEthTx,                                                    // call-back used to release a prepared transmit buffer
    fnPPP_FreeEthernetBuffer,                                            // call-back used to free a used reception buffer
#if defined USE_IGMP
    fnPPP_ModifyMulticastFilter,                                         // call-back used to setup the multicast filter
#endif
};

#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
    static const unsigned char cClient[] = { 'C', 'L', 'I', 'E', 'N', 'T', 'S', 'E', 'R', 'V', 'E', 'R' };
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static int iPPP_State = PPP_STATE_INIT;
static QUEUE_HANDLE PPP_PortID = 0;
static QUEUE_HANDLE PPP_Handle = 0;

#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
    static unsigned long ulACCM_bits = 0xffffffff;                       // default: escape all characters between 0x00 and 0x1f
    static unsigned char ucMagicNumber[4];
#endif

    
#if defined ETH_INTERFACE
    #define PPP_INTERFACE_CHANNEL  1                                     // the Ethernet controller has channel 0 so PPP uses Ethernet channel 1
#else
    #define PPP_INTERFACE_CHANNEL  0                                     // default channel number
#endif

// PPP task
//
extern void fnPPP(TTASKTABLE *ptrTaskTable)
{
    QUEUE_HANDLE           PortIDInternal = ptrTaskTable->TaskID;        // queue ID for task input
    static unsigned char   ucInputMessage[PPP_RX_BUFFER_SPACE];          // reserve space for receiving messages
    static QUEUE_TRANSFER  ppp_frame_length = 0;                         // collected frame size
    unsigned char          ucThisChar;
#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
    static int             iRxEscape = 0;
    static unsigned short  crc_value;
#endif

    if (iPPP_State == PPP_STATE_INIT) {
        ETHTABLE ppp_ethernet;                                           // configuration structure to be passed to the Ethernet configuration
        ppp_ethernet.ptrEthernetFunctions = (void *)&PPP_EthernetFunctions; // enter the Ethernet function list for the defult internal controller
        ppp_ethernet.Task_to_wake = 0;
        ppp_ethernet.Channel = PPP_INTERFACE_CHANNEL;
        ppp_ethernet.usMode = network[PPP_NETWORK].usNetworkOptions;     // options to be used by the interface
#if defined USE_IPV6                                                     // generate an IPv6 link-local address from the MAC address
        ucLinkLocalIPv6Address[PPP_NETWORK][0] = 0xfe;                   // link-local unicast
        ucLinkLocalIPv6Address[PPP_NETWORK][1] = 0x80;                   // link-local unicast
                                                                         // intermediate values left at 0x00
        ucLinkLocalIPv6Address[PPP_NETWORK][8] = (network[PPP_NETWORK].ucOurMAC[0] | 0x2); // invert the universal/local bit (since it is always '0' it means setting to '1')
        ucLinkLocalIPv6Address[PPP_NETWORK][9] = network[PPP_NETWORK].ucOurMAC[1];
        ucLinkLocalIPv6Address[PPP_NETWORK][10] = network[PPP_NETWORK].ucOurMAC[2];
        ucLinkLocalIPv6Address[PPP_NETWORK][11] = 0xff;                  // insert standard 16 bit value to extend MAC-48 to EUI-64
        ucLinkLocalIPv6Address[PPP_NETWORK][12] = 0xfe;
        ucLinkLocalIPv6Address[PPP_NETWORK][13] = network[PPP_NETWORK].ucOurMAC[3];
        ucLinkLocalIPv6Address[v][14] = network[PPP_NETWORK].ucOurMAC[4];
        ucLinkLocalIPv6Address[v][15] = network[v].ucOurMAC[5];
        ppp_ethernet.usMode |= CON_MULTICAST;                            // enable multicast when using IPV6
#else
        uMemcpy(ppp_ethernet.ucMAC, &network[PPP_NETWORK].ucOurMAC[0], MAC_LENGTH); // the MAC address to be used by the interface
#endif
#if defined ETHERNET_BRIDGING
        ppp_ethernet.usMode |= (PROMISCUOUS);                            // Ethernet bridging requires promiscuous operation
#endif
        ppp_ethernet.usSizeTx = SLIP_MAX_DATAGRAM;                       // transmit buffer size
        ppp_ethernet.ucEthTypes = (ARP | IPV4);                          // enable reception of these protocols (used only by NE64 controller)
        ppp_ethernet.usExtEthTypes = 0;                                  // specify extended frame types (only used by NE64 controller)
        PPP_Handle = fnOpen(TYPE_ETHERNET, FOR_I_O, &ppp_ethernet);
#if IP_INTERFACE_COUNT > 1
        fnEnterInterfaceHandle(PPP_IP_INTERFACE, PPP_Handle, ((INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING))); // enter the queue as RNDIS interface handler
#else
        Ethernet_handle[0] = PPP_Handle;
#endif
#if defined USE_SLIP_DIAL_OUT
        fnWrite(PPP_PortID, (unsigned char *)"*99***1#", 8);             // establish a connection to the modem
#endif
#if !defined USE_SLIP
        start_ppp_now(PPP_PortID);                                       // temporary - using lwip PPP
#endif
        iPPP_State = PPP_STATE_IDLE;
    }

    while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH)) {      // check task input queue
        switch (ucInputMessage[MSG_SOURCE_TASK]) {                       // switch depending on message source
        case TIMER_EVENT:
            switch (ucInputMessage[MSG_TIMER_EVENT]) {
            case E_TIMER_PPP:
                ppp_callback(ppp_parameter);                             // timeout callback
                break;
            default:
                break;
            }
            break;
        }
    }

#if defined USE_SLIP
    // SLIP character reception
    //
    while (fnRead(PPP_PortID, &ucInputMessage[ppp_frame_length], 1) != 0) { // while serial input waiting
        if (iPPP_State == PPP_STATE_IN_FRAME) {
            if (SLIP_ESCAPE_END_CHARACTER == ucInputMessage[ppp_frame_length]) {
                ucInputMessage[ppp_frame_length] = SLIP_END_CHARACTER;
            }
            else if (SLIP_ESCAPE_ESCAPE_CHARACTER == ucInputMessage[ppp_frame_length]) {
                ucInputMessage[ppp_frame_length] = SLIP_ESCAPE_CHARACTER;
            }
            else {                                                       // this is a protocol violation but it is recommended to just put the received byto to the buffer
                ucInputMessage[ppp_frame_length] = ucInputMessage[ppp_frame_length];
            }
            iPPP_State = PPP_STATE_IDLE;
        }
        else {
            switch (ucInputMessage[ppp_frame_length]) {
            case SLIP_END_CHARACTER:                                     // end of a datagram (pass datagram to the stack)
                if (ppp_frame_length != 0) {
                    ETHERNET_FRAME rx_frame;
                    rx_frame.frame_size = (unsigned short)ppp_frame_length;
                    rx_frame.ptEth = (ETHERNET_FRAME_CONTENT *)ucInputMessage;
                    rx_frame.usDataLength = 0;
                    rx_frame.usIPLength = 0;
    #if (IP_INTERFACE_COUNT > 1)
                    rx_frame.ucInterface = (PPP_INTERFACE >> INTERFACE_SHIFT); // reception is on the RNDIS interface
                    rx_frame.ucInterfaceHandling = (DEFAULT_INTERFACE_CHARACTERISTICS | INTERFACE_NO_MAC_FILTERING); // default interface handling
    #endif
    #if defined IPV4_SUPPORT_RX_DEFRAGMENTATION && defined IP_RX_CHECKSUM_OFFLOAD
                    rx_frame.ucSpecialHandling = (INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING | INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_MAC_FILTERING);
    #endif
                    fnHandleEthernetFrame(&rx_frame, PPP_Handle);        // handle the reception
                    ppp_frame_length = 0;
    #if defined ETH_ERROR_FLAGS
                    if ((rx_frame.ucErrorFlags & (ETH_ERROR_INVALID_IPv4 | ETH_ERROR_INVALID_IPv4_CHECKSUM | ETH_ERROR_INVALID_ARP_RARP)) != 0) {
                        // Check for other coding?
                        //
                    }
    #endif
                }
                continue;
            case SLIP_ESCAPE_CHARACTER:                                  // escape sequence in progress
                iPPP_State = PPP_STATE_IN_FRAME;                         // next character determines the meaning
                continue;
            default:
                break;
            }
        }
        if (++ppp_frame_length >= sizeof(ucInputMessage)) {              // protect input buffer form overflows in case of invalid input
            ppp_frame_length = 0;
        }
    }
#else
    // PPP character reception
    //
    while (fnRead(PPP_PortID, &ucInputMessage[ppp_frame_length], 1) != 0) { // while serial input waiting
    #if defined UTASKER_PPP_LOW_LAYER
        static unsigned char rx_accm[32] = { 0 };                        // asychronous control character map as used by receiver
    #endif
        ucThisChar = ucInputMessage[ppp_frame_length];
    #if defined TEMP_LWIPP_PPP
        pass_raw_input(ucThisChar);                                      // pass each received UART input character to LWIP PPP stack
    #endif
    #if defined UTASKER_PPP_LOW_LAYER
        switch (iPPP_State) {
        case PPP_STATE_IDLE:                                             // waiting for dial in
        #if defined PPP_DIAL_IN
            if (ucThisChar == cClient[ppp_frame_length]) {
                if (++ppp_frame_length >= 6)  {
                    iPPP_State = PPP_STATE_CLIENT;
                    fnWrite(PPP_PortID, (unsigned char *)cClient, sizeof(cClient)); // answer with CLIENTSERVER
                }
                else {
                    break;
                }
            }
        #endif
            rx_accm[0] = 0xff;                                           // 0x00..0x07 are control characters by default
            rx_accm[1] = 0xff;                                           // 0x08..0x0f are control characters by default
            rx_accm[2] = 0xff;                                           // 0x10..0x17 are control characters by default
            rx_accm[3] = 0xff;                                           // 0x18..0x1f are control characters by default
            uMemset(&rx_accm[4], 0, (sizeof(rx_accm) - 4));              // rest are not control characters
            ppp_frame_length = 0;
        #if defined PPP_DIAL_IN
            break;
        #endif

        case PPP_STATE_CLIENT:
            if (ucThisChar == 0x7e) {                                    // start of frame
                iPPP_State = PPP_STATE_IN_FRAME;
                crc_value = PPP_INIT_CRC;                                // prime crc value for frame checking
                break;
            }
            else if (ucThisChar != 0xff) {                               // we also accept an all stations broadcast address without a start sync
                break;
            }
            iPPP_State = PPP_STATE_IN_FRAME;
            // Fall through intentionally on 0xff (all stations broadcast address)
            //
        case PPP_STATE_IN_FRAME:
            if (ucThisChar == 0x7e) {                                    // end of frame
                if (ppp_frame_length == 0) {                             // ignore multiple 0x7e
                    break;
                }
                else {
                    static int iFrameCnt = 0;
                    if (++iFrameCnt == 0x0e) {
                        iFrameCnt = 0;
                    }
                    if (crc_value == PPP_GOOD_CRC) {                     // drop all frames with bad crc value
                        /*iPPP_State = */fnHandlePPP_frame(ucInputMessage, (ppp_frame_length - 2));
                    }
        #if defined _WINDOWS
                    else {
                        crc_value = crc_value;
                    }
        #endif
                    iRxEscape = 0;
                    crc_value = PPP_INIT_CRC;                            // prime crc value for frame checking
                    ppp_frame_length = 0;
                    break;
                }
            }
            else if ((rx_accm[ucThisChar/8] & (1 << (ucThisChar & 0x7))) != 0) { // �f this is a non-escaped control character we ignore it
                break;
            }
            if (iRxEscape != 0) {                                        // escaping character
                ucInputMessage[ppp_frame_length] ^= 0x20;
                iRxEscape = 0;
            }
            else if (ucThisChar == 0x7d) {                               // receiving an escape character
                iRxEscape = 1;
                break;
            }
            crc_value = fnPPP_crc(crc_value, ucInputMessage[ppp_frame_length]); // calculate CRC over each data byte
            if (++ppp_frame_length >= PPP_RX_BUFFER_SPACE) {             // check for over length frames
                iPPP_State = PPP_STATE_CLIENT;
                ppp_frame_length = 0;
            }
            break;
        }
    #endif
    }
#endif
}

static int fnPPP_ConfigEthernet(ETHTABLE *pars)
{
    TTYTABLE tInterfaceParameters;                                       // table for passing information to driver
    tInterfaceParameters.Channel = PPP_UART;                             // set UART channel for serial use
    tInterfaceParameters.ucSpeed = SERIAL_BAUD_115200;                   // baud rate
    tInterfaceParameters.Rx_tx_sizes.RxQueueSize = PPP_RX_BUFFER_SIZE;   // input buffer size
    tInterfaceParameters.Rx_tx_sizes.TxQueueSize = PPP_TX_BUFFER_SIZE;   // output buffer size
    tInterfaceParameters.Task_to_wake = OWN_TASK;                        // wake self when messages have been received
    #if defined SUPPORT_FLOW_HIGH_LOW
    tInterfaceParameters.ucFlowHighWater = 80;                           // set the flow control high and low water levels in %
    tInterfaceParameters.ucFlowLowWater = 20;
    #endif
    #if defined USER_DEFINED_UART_RX_HANDLER                             // {1}
    tInterfaceParameters.receptionHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_RX_BREAK_DETECTION
    tInterfaceParameters.receiveBreakHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_TX_FRAME_COMPLETE
    tInterfaceParameters.txFrameCompleteHandler = 0;
    #endif
    tInterfaceParameters.Config = (CHAR_8 | NO_PARITY | ONE_STOP /*| USE_XON_OFF*/ | CHAR_MODE);
    #if defined SERIAL_SUPPORT_DMA
    tInterfaceParameters.ucDMAConfig = 0;
    #endif
    if ((PPP_PortID = fnOpen(TYPE_TTY, FOR_I_O, &tInterfaceParameters)) != NO_ID_ALLOCATED) { // open serial port with defined parameters
        fnDriver(PPP_PortID, (TX_ON | RX_ON), 0);                        // enable rx and tx
    }
    return 0;
}

static int fnPPP_GetQuantityRxBuf(void)
{
    return 0;                                                            // no reception buffer
}

static unsigned char *fnPPP_GetTxBufferAdd(int iBufNr)
{
    return 0;                                                            // return a pointer to the payload area of the message (buffer not used)
}

static int fnPPP_WaitTxFree(void)
{
    return 0;
}

static void fnPPP_FreeEthernetBuffer(int iBufNr)                          // dummy since not used
{
}

#if defined TEMP_LWIPP_PPP
extern void fnSetIPStuff(unsigned long ipaddr, unsigned long netmask, unsigned long gw)
{
    network[PPP_NETWORK].ucOurIP[3] = (unsigned char)(ipaddr >> 24);
    network[PPP_NETWORK].ucOurIP[2] = (unsigned char)(ipaddr >> 16);
    network[PPP_NETWORK].ucOurIP[1] = (unsigned char)(ipaddr >> 8);
    network[PPP_NETWORK].ucOurIP[0] = (unsigned char)(ipaddr);

    network[PPP_NETWORK].ucNetMask[3] = (unsigned char)(netmask >> 24);
    network[PPP_NETWORK].ucNetMask[2] = (unsigned char)(netmask >> 16);
    network[PPP_NETWORK].ucNetMask[1] = (unsigned char)(netmask >> 8);
    network[PPP_NETWORK].ucNetMask[0] = (unsigned char)(netmask);

    network[PPP_NETWORK].ucDefGW[3] = (unsigned char)(gw >> 24);
    network[PPP_NETWORK].ucDefGW[2] = (unsigned char)(gw >> 16);
    network[PPP_NETWORK].ucDefGW[1] = (unsigned char)(gw >> 8);
    network[PPP_NETWORK].ucDefGW[0] = (unsigned char)(gw);
#if defined _WINDOWS
    fnSimulateLinkUp();
#endif
}

struct pbuf {
    /** next pbuf in singly linked pbuf chain */
    struct pbuf *next;

    /** pointer to the actual data in the buffer */
    unsigned char *payload;

    /**
    * total length of this buffer and all next buffers in chain
    * belonging to the same packet.
    *
    * For non-queue packet chains this is the invariant:
    * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
    */
    unsigned short tot_len;

    /** length of this buffer */
    unsigned short len;

    /** pbuf_type as u8_t instead of enum to save space */
    unsigned char /*pbuf_type*/ type;

    /** misc flags */
    unsigned char flags;

    /**
    * the reference count always equals the number of pointers
    * that refer to this pbuf. This can be pointers from an application,
    * the stack itself, or pbuf->next pointers from a chain.
    */
    unsigned short ref;
};

extern unsigned char IP_input(struct pbuf *p, void *inp)
{
    ETHERNET_FRAME rx_frame;                                             // temporary frame strcuture
    rx_frame.frame_size = (p->len + ((2 * MAC_LENGTH) + 2));             // add Ethernet II header for compatibility
    rx_frame.ptEth = (ETHERNET_FRAME_CONTENT *)(p->payload - ((2 * MAC_LENGTH) + 2));
    rx_frame.ptEth->ethernet_frame_type[0] = (unsigned char)(PROTOCOL_IPv4 >> 8); // assume always IPv4 content
    rx_frame.ptEth->ethernet_frame_type[1] = (unsigned char)(PROTOCOL_IPv4);
    #if IP_INTERFACE_COUNT > 1
    rx_frame.ucInterfaceHandling = (INTERFACE_NO_TX_CS_OFFLOADING | INTERFACE_NO_RX_CS_OFFLOADING | INTERFACE_NO_TX_PAYLOAD_CS_OFFLOADING | INTERFACE_NO_MAC_ETHERNET_II);
    rx_frame.ucInterface = (PPP_INTERFACE >> INTERFACE_SHIFT);           // reception is on the PPP interface
    #endif
    fnHandleEthernetFrame(&rx_frame, PPP_Handle);                        // pass to the TCP/IP for handling the received IP datagram
    return 0;
}
#endif

#if defined USE_SLIP
static int iTxCount = 0;

// The Ethernet transmission is passed in increasing protocol layers and we send it directly to the serial interface after inserting any escape sequences necessary
//
static void fnPPP_PutInBuffer(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes)
{
    QUEUE_TRANSFER ChunkLen = 0;
    unsigned char *ptrToSend = ptrIn;
    if (iTxCount == 0) {
        fnWrite(PPP_PortID, (unsigned char *)&cEscapeEscapeEnd[2], 1);   // send preceeding end character
        iTxCount += nr_of_bytes;
        return;
    }
    iTxCount += nr_of_bytes;
    while (nr_of_bytes-- != 0) {                                         // scan for escape requirements
        if (*ptrToSend == SLIP_END_CHARACTER) {                          // end character found in the data
            if (ChunkLen != 0) {
                fnWrite(PPP_PortID, ptrIn, ChunkLen);                    // copy preceding raw data
            }
            fnWrite(PPP_PortID, (unsigned char *)&cEscapeEscapeEnd[1], 2); // insert an escape sequence
            ptrIn += (ChunkLen + 1);
            ChunkLen = 0;
        }
        else if (*ptrToSend == SLIP_ESCAPE_CHARACTER) {                  // escape character found in the data
            if (ChunkLen != 0) {
                fnWrite(PPP_PortID, ptrIn, ChunkLen);                    // copy preceding raw data
            }
            fnWrite(PPP_PortID, (unsigned char *)&cEscapeEscapeEnd[0], 2); // insert an escape sequence
            ptrIn += (ChunkLen + 1);
            ChunkLen = 0;
        }
        else {
            ChunkLen++;                                                  // count raw input
        }
        ptrToSend++;
    }
    if (ChunkLen != 0) {
        fnWrite(PPP_PortID, ptrIn, ChunkLen);                            // copy remaining raw data
    }
}

// Initiate transmission of prepared Ethernet content to PPP interface
//
static QUEUE_TRANSFER fnPPP_StartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    fnWrite(PPP_PortID, (unsigned char *)&cEscapeEscapeEnd[2], 1);       // send end character
    iTxCount = 0;
    return (DataLen);
}
#else

#if defined TEMP_LWIPP_PPP
extern void start_timer(void(*fsm_timeout)(void *), int msecs, void *f)
{
    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)((msecs * SEC)/1000), E_TIMER_PPP);
    ppp_callback = fsm_timeout;
    ppp_parameter = f;
}

extern void stop_timer(void(*fsm_timeout)(void *))
{
    uTaskerStopTimer(OWN_TASK);
    fsm_timeout = 0;
}
#endif

static unsigned char ucTxBuffer[2048] = { 0 };
static QUEUE_TRANSFER txLength = 0;

// The Ethernet transmission is passed in increasing protocol layers and we send it directly to the serial interface after inserting any escape sequences necessary
//
static void fnPPP_PutInBuffer(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes)
{
    uMemcpy(&ucTxBuffer[txLength], ptrIn, nr_of_bytes);
    txLength += nr_of_bytes;
}

// Initiate transmission of prepared Ethernet content to PPP interface
//
static QUEUE_TRANSFER fnPPP_StartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    fnSendPPP(&ucTxBuffer[((2 * MAC_LENGTH) + 2)], (DataLen - ((2 * MAC_LENGTH) + 2)));
    txLength = 0;
    return (DataLen);
}
#endif


#if defined USE_IGMP
static void fnPPP_ModifyMulticastFilter(QUEUE_TRANSFER action, unsigned char *ptrIP)
{
    _EXCEPTION("Not implemented");
}
#endif


#if !defined USE_SLIP
#define LCP_PROTOCOL       0xc021
#define PAP_PROTOCOL       0xc023
#define CHAP_PROTOCOL      0xc22305
#define MSCHAPV1_PROTOCOL  0xc22380
#define MSCHAPV2_PROTOCOL  0xc22381
#define EAP_PROTOCOL       0xc227
#define SPAP_PROTOCOL      0xc027
#define OLD_SPAP_PROTOCOL  0xc123
#define CCP_PROTOCOL       0x80fd
#define IPCP_PROTOCOL      0x8021
#define IPV4_PROTOCOL      0x0021

#define VENDOR_EXTENSION   0x00
#define CONFIG_REQUEST     0x01
#define CONFIG_ACK         0x02
#define CONFIG_NACK        0x03
#define CONFIG_REJECT      0x04
#define TERMINATE_REQUEST  0x05
#define TERMINATE_ACK      0x06
#define CODE_REJECT        0x07
#define PROTOCOL_REJECT    0x08
#define ECHO_REQUEST       0x09
#define ECHO_REPLY         0x0a
#define DISCARD_REQUEST    0x0b
#define IDENTIFICATION     0x0c
#define TIME_REMAINING     0x0d
#define RESET_REQUEST      0x0e
#define RESET_ACK          0x0f

// Options
//
#define ACCM               0x02                                          // Asynchronous Control Character Map
#define AUTH_PROT          0x03
#define QUALITY_PROT       0x04
#define MAGIC_NUMBER       0x05
#define PFC                0x07                                          // Protocol Field Compression
#define ACFC               0x08                                          // Address and Control Field Compression
#define CALLBACK           0x0d

#define IPCP               0x02                                          // IP compression protocol
#define IPIP               0x03                                          // IP address (0.0.0.0 if not known)
#define IPDNS              0x81                                          // Primary DNS address (0.0.0.0 if not known)
#define IPNBNS             0x82                                          // Primary NBNS address (0.0.0.0 if not known)
#define IPSDNS             0x83                                          // Secondary DNS address (0.0.0.0 if not known)
#define IPSNBNS            0x84                                          // Secondary NBNS address (0.0.0.0 if not known)

#define AUTHENTICATE_REQUEST 0x01
#define AUTHENTICATE_ACK     0x02
#define AUTHENTICATE_NAK     0x03

typedef struct st_ppp_info {
    unsigned char ucPPP_Code;
    unsigned char ucPPP_ID;
    unsigned char ucPPP_Length[2];
} PPP_INFO;

typedef struct st_ppp_option {
    unsigned char ucPPP_option;
    unsigned char ucPPP_option_length;
} PPP_OPTION;

#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
static const unsigned char ucHDLC_Flag = 0x7e;
static const unsigned char ucHDLC_Add[] = {0xff, 0x03};

static unsigned short usTxCRC;

// Check whether this data byte has to be escaped
//
static int fnIsEscape(const unsigned char ucData)
{
    if ((ucData == 0x7e) || (ucData == 0x7d)) {
        return 1;
    }

    if (ucData <= 0x1f) {                                                // if the data byte is in the range 0..0x1f check whether it has to be escaped
        if ((ulACCM_bits >> ucData) & 0x01) {
            return 1;
        }
    }
    return 0;
}

// Write PPP data to the UART, calculating its check sum and escaping characters if necessary
//
static void fnWritePPP(const unsigned char *ptrData, QUEUE_TRANSFER length)
{
    const unsigned char *ptrTx = ptrData;
    QUEUE_TRANSFER txLength = 0;

    while (length != 0) {
        usTxCRC = fnPPP_crc(usTxCRC, *ptrData);                          // calculate the crc of the tx frame over each byte
        if (fnIsEscape(*ptrData)) {
            unsigned char ucStuffed[2];
            if (txLength != 0) {
                fnWrite(PPP_PortID, (unsigned char *)ptrTx, txLength);   // send waiting non-stuffed segments
            }
            ucStuffed[0] = 0x7d;
            ucStuffed[1] = (*ptrData ^ 0x20);
            fnWrite(PPP_PortID, ucStuffed, sizeof(ucStuffed));           // send stuffed character
            ptrData++;
            ptrTx = ptrData;
            txLength = 0;
        }
        else {
            ptrData++;
            txLength++;                                                  // collect the length without byte stuffing
        }
        length--;
    }
    if (txLength != 0) {
        fnWrite(PPP_PortID, (unsigned char *)ptrTx, txLength);           // send waiting non-stuffed segments
    }
}

static unsigned char ucID = 0x0;

static void fnReject(unsigned short usProtocol, unsigned char ucOption, unsigned char *option_data, unsigned char ucOptionLength)
{
    unsigned char ucProtocolCode[6 + 2]; // fixed test message
    unsigned char ucCRC_16[2];

    usTxCRC = PPP_INIT_CRC;                                              // prime tx crc value
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send start flag
    fnWritePPP(ucHDLC_Add, sizeof(ucHDLC_Add));                          // send address and control

    ucProtocolCode[0] = (unsigned char)(usProtocol>>8);
    ucProtocolCode[1] = (unsigned char)(usProtocol);
    ucProtocolCode[2] = CONFIG_REJECT;
    ucProtocolCode[3] = ucID++;
    ucProtocolCode[4] = 0;
    ucProtocolCode[5] = ucOptionLength + 4;

    ucProtocolCode[6] = ucOption;
    ucProtocolCode[7] = ucOptionLength;

    fnWritePPP(ucProtocolCode, sizeof(ucProtocolCode));                  // send protocol, code, ID and length
    ucOptionLength -= 2;
    if (ucOptionLength != 0) {
        fnWritePPP(option_data, ucOptionLength);
    }

    usTxCRC ^= 0xffff;
    ucCRC_16[0] = (unsigned char)(usTxCRC);
    ucCRC_16[1] = (unsigned char)(usTxCRC >> 8);
    fnWritePPP(ucCRC_16, sizeof(ucCRC_16));                              // add the check sum
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send end flag
}

// Test
//
static void fnNAK(unsigned short usProtocol, PPP_INFO *ppp_inf)
{
    unsigned short usOptionsLength = ppp_inf->ucPPP_Length[0];
    unsigned char ucCRC_16[2];
    unsigned char ucProtocolCode[2];

    ppp_inf->ucPPP_Code = CONFIG_NACK;                                    // convert into ack frame

    usTxCRC = PPP_INIT_CRC;                                              // prime tx crc value
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send start flag
    fnWritePPP(ucHDLC_Add, sizeof(ucHDLC_Add));                          // send address and control

    ucProtocolCode[0] = (unsigned char)(usProtocol>>8);
    ucProtocolCode[1] = (unsigned char)(usProtocol);
    fnWritePPP(ucProtocolCode, sizeof(ucProtocolCode));

    usOptionsLength <<= 8;
    usOptionsLength |= ppp_inf->ucPPP_Length[1];

    fnWritePPP((unsigned char *)ppp_inf, usOptionsLength);

    usTxCRC ^= 0xffff;
    ucCRC_16[0] = (unsigned char)(usTxCRC);
    ucCRC_16[1] = (unsigned char)(usTxCRC >> 8);
    fnWritePPP(ucCRC_16, sizeof(ucCRC_16));                              // add the check sum
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send end flag
}

// Send back a configuration acknowledge since we accept the negotiation parameters
//
static void fnAccept(unsigned short usProtocol, PPP_INFO *ppp_inf)
{
    unsigned short usOptionsLength = ppp_inf->ucPPP_Length[0];
    unsigned char ucCRC_16[2];
    unsigned char ucProtocolCode[2];

    ppp_inf->ucPPP_Code = CONFIG_ACK;                                    // convert into ack frame

    usTxCRC = PPP_INIT_CRC;                                              // prime tx crc value
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send start flag
    fnWritePPP(ucHDLC_Add, sizeof(ucHDLC_Add));                          // send address and control

    ucProtocolCode[0] = (unsigned char)(usProtocol>>8);
    ucProtocolCode[1] = (unsigned char)(usProtocol);
    fnWritePPP(ucProtocolCode, sizeof(ucProtocolCode));

    usOptionsLength <<= 8;
    usOptionsLength |= ppp_inf->ucPPP_Length[1];

    fnWritePPP((unsigned char *)ppp_inf, usOptionsLength);

    usTxCRC ^= 0xffff;
    ucCRC_16[0] = (unsigned char)(usTxCRC);
    ucCRC_16[1] = (unsigned char)(usTxCRC >> 8);
    fnWritePPP(ucCRC_16, sizeof(ucCRC_16));                              // add the check sum
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send end flag
}

static void fnStartAuthentication(void)
{
    static const unsigned char ucPAP_authentication[] = {AUTH_PROT, 4, (unsigned char)(PAP_PROTOCOL>>8),(unsigned char)(PAP_PROTOCOL)};
    unsigned char ucProtocolCode[2];
    unsigned char ucCRC_16[2];
    PPP_INFO ppp_inf;

    usTxCRC = PPP_INIT_CRC;                                              // prime tx crc value
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send start flag
    fnWritePPP(ucHDLC_Add, sizeof(ucHDLC_Add));                          // send address and control

    ucProtocolCode[0] = (unsigned char)(LCP_PROTOCOL >> 8);
    ucProtocolCode[1] = (unsigned char)(LCP_PROTOCOL);
    fnWritePPP(ucProtocolCode, sizeof(ucProtocolCode));

    ppp_inf.ucPPP_Code = CONFIG_REQUEST;
    ppp_inf.ucPPP_ID   = ucID++;
    ppp_inf.ucPPP_Length[0] = 0;
    ppp_inf.ucPPP_Length[1] = (sizeof(ppp_inf) + sizeof(ucPAP_authentication));
    fnWritePPP((unsigned char *)&ppp_inf, sizeof(ppp_inf));
    fnWritePPP((unsigned char *)&ucPAP_authentication, sizeof(ucPAP_authentication));

    usTxCRC ^= 0xffff;
    ucCRC_16[0] = (unsigned char)(usTxCRC);
    ucCRC_16[1] = (unsigned char)(usTxCRC >> 8);
    fnWritePPP(ucCRC_16, sizeof(ucCRC_16));                              // add the check sum
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send end flag
}

static void fnSendPAP_ok(void)
{
    unsigned char ucProtocolCode[2];
    unsigned char ucCRC_16[2];
    PPP_INFO ppp_inf;
    unsigned char no_data = 0;

    usTxCRC = PPP_INIT_CRC;                                              // prime tx crc value
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send start flag
    fnWritePPP(ucHDLC_Add, sizeof(ucHDLC_Add));                          // send address and control

    ucProtocolCode[0] = (unsigned char)(PAP_PROTOCOL>>8);
    ucProtocolCode[1] = (unsigned char)(PAP_PROTOCOL);
    fnWritePPP(ucProtocolCode, sizeof(ucProtocolCode));

    ppp_inf.ucPPP_Code = AUTHENTICATE_ACK;
    ppp_inf.ucPPP_ID   = ucID++;
    ppp_inf.ucPPP_Length[0] = 0;
    ppp_inf.ucPPP_Length[1] = (sizeof(ppp_inf) + 1);
    fnWritePPP((unsigned char *)&ppp_inf, sizeof(ppp_inf));
    fnWritePPP(&no_data, 1);

    usTxCRC ^= 0xffff;
    ucCRC_16[0] = (unsigned char)(usTxCRC);
    ucCRC_16[1] = (unsigned char)(usTxCRC >> 8);
    fnWritePPP(ucCRC_16, sizeof(ucCRC_16));                              // add the check sum
    fnWrite(PPP_PortID, (unsigned char *)&ucHDLC_Flag, sizeof(ucHDLC_Flag)); // send end flag
}

static unsigned char ucTestIP[] = {192,168,0,151};
static unsigned char ucTestDNS[] = {192,168,0,1};
static unsigned char ucTestN[] = {192,168,0,1};

static void fnNegotiateOptions(unsigned short usProtocol, PPP_INFO *ppp_inf, unsigned char *ptrFrame)
{
    int iFound = 0;
    unsigned short usOptionsLength = ppp_inf->ucPPP_Length[0];
    unsigned char ucSingleOptLength;
    PPP_OPTION *ppp_opt = (PPP_OPTION *)ptrFrame;
    usOptionsLength <<= 8;
    usOptionsLength |= ppp_inf->ucPPP_Length[1];
    usOptionsLength -= sizeof(PPP_INFO);

    while (usOptionsLength != 0) {
        ptrFrame += sizeof(PPP_OPTION);
        if (LCP_PROTOCOL == usProtocol) {
            switch (ppp_opt->ucPPP_option) {
            case ACCM:
                ulACCM_bits = *ptrFrame;                                 // set the requested character map
                ulACCM_bits <<= 8;                                       // validate only when moving to open state
                ulACCM_bits = *(ptrFrame+1);
                ulACCM_bits <<= 8;
                ulACCM_bits = *(ptrFrame+2);
                ulACCM_bits <<= 8;
                ulACCM_bits = *(ptrFrame+3);
                break;
            case MAGIC_NUMBER:
                uMemcpy(ucMagicNumber, ptrFrame, 4);                     // set the advertised
                break;
            case PFC:                                                    // protocol field compression
            case ACFC:                                                   // address and control field compression
            case CALLBACK:
                fnReject(usProtocol, ppp_opt->ucPPP_option, ptrFrame, ppp_opt->ucPPP_option_length); // reject the option
                break;
            }
        }
        else if (CCP_PROTOCOL == usProtocol) {
            fnReject(usProtocol, ppp_opt->ucPPP_option, ptrFrame, ppp_opt->ucPPP_option_length);
            return;
        }
        else if (IPCP_PROTOCOL == usProtocol) {
            switch (ppp_opt->ucPPP_option) {
            case IPCP:                                                   // IP compression protocol
            //    fnReject(usProtocol, ppp_opt->ucPPP_option, ptrFrame, ppp_opt->ucPPP_option_length); // reject the option
            //    return;
                break;

            case IPIP:
                if (!(uMemcmp(ptrFrame, ucTestIP, IPV4_LENGTH))) {
                    iFound = 1;
                }
                else  {
                    uMemcpy(ptrFrame, ucTestIP, IPV4_LENGTH);
                }
                break;
            case IPDNS:
                if (!iFound) {
                    uMemcpy(ptrFrame, ucTestDNS, IPV4_LENGTH);
                }
                break;
            case IPNBNS:
                if (!iFound) {
                    uMemcpy(ptrFrame, ucTestN, IPV4_LENGTH);
                }
                break;
            case IPSDNS:
                if (!iFound) {
                    uMemcpy(ptrFrame, ucTestDNS, IPV4_LENGTH);
                }
                break;
            case IPSNBNS:
                if (!iFound) {
                    uMemcpy(ptrFrame, ucTestN, IPV4_LENGTH);
                }
                break;
            }
        }
        ucSingleOptLength = ppp_opt->ucPPP_option_length;
        if (ucSingleOptLength < sizeof(PPP_OPTION)) {
            ucSingleOptLength = sizeof(PPP_OPTION);                      // ensure length is not less that the opton field can really be
        }
        if (ppp_opt->ucPPP_option_length >= usOptionsLength) {           // protect against bad option length value
            usOptionsLength = 0;
        }
        else {
            usOptionsLength -= (ppp_opt->ucPPP_option_length);
        }
        ptrFrame += (ucSingleOptLength - sizeof(PPP_OPTION));
        ppp_opt = (PPP_OPTION *)ptrFrame;
    }
    if ((IPCP_PROTOCOL == usProtocol) && (!iFound)) {
        fnNAK(usProtocol, ppp_inf);
    }
    else {
        fnAccept(usProtocol, ppp_inf);                                   // if we haven't rejected a negotiation option accept now
    }
    if (LCP_PROTOCOL == usProtocol) {
        fnStartAuthentication();
    }
}
#endif

#if !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
static int fnHandlePPP_frame(unsigned char *ptrFrame, QUEUE_TRANSFER ppp_frame_length)
{
    if (*ptrFrame++ == 0xff) {                                           // address always 0xff
        if (*ptrFrame++ == 0x03) {                                       // control always unnumbered info
            PPP_INFO *ppp_inf;
            unsigned short usProtocol = *ptrFrame++;
            usProtocol <<= 8;
            usProtocol |= *ptrFrame++;
            ppp_inf = (PPP_INFO *)ptrFrame;
            ptrFrame += sizeof(PPP_INFO);
            switch (usProtocol) {
            case LCP_PROTOCOL:
                switch (ppp_inf->ucPPP_Code) {                           // code
                case CONFIG_REQUEST:
                    fnNegotiateOptions(usProtocol, ppp_inf, ptrFrame);
                    break;
                case CONFIG_ACK:
                    break;                                               // peer has accepted out option request
                case CONFIG_NACK:
                    break;
                case CONFIG_REJECT:
                    break;
                case TERMINATE_REQUEST:                                  // terminate connection request
                    break;
                case IDENTIFICATION:                                     // peer moved to LCP opened state (often sent twice - no need to acknowledge it)
                    break;
                }
                break;

            case PAP_PROTOCOL:                                           // peer performing authentication
                switch (ppp_inf->ucPPP_Code) {
                case AUTHENTICATE_REQUEST:
                    fnSendPAP_ok();
                    break;
                }
                break;

            case CCP_PROTOCOL:                                           // compression control protocol
                switch (ppp_inf->ucPPP_Code) {                           // code
                case CONFIG_REQUEST:
                    fnNegotiateOptions(usProtocol, ppp_inf, ptrFrame);
                    break;
                }
                break;

            case IPCP_PROTOCOL:                                          // IP Control protocol
                switch (ppp_inf->ucPPP_Code) {                           // code
                case CONFIG_REQUEST:
                    fnNegotiateOptions(usProtocol, ppp_inf, ptrFrame);
                    break;
                }
                break;

            case IPV4_PROTOCOL:                                          // IPV4 data
                switch (ppp_inf->ucPPP_Code) {                           // code
                case CONFIG_REQUEST:
                    fnNegotiateOptions(usProtocol, ppp_inf, ptrFrame);
                    break;
                }
                break;
            }
        }
    }
    return 0;
}
#endif

#if defined _PPP_CRC_LOOKUP
// PPP CRC look up table as defined in RFC 1171
//
static const unsigned short fcstab[256] = {
      0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
      0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
      0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
      0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
      0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
      0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
      0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
      0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
      0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
      0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
      0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
      0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
      0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
      0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
      0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
      0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
      0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
      0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
      0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
      0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
      0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
      0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
      0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
      0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
      0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
      0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
      0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
      0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
      0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
      0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
      0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
      0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static unsigned short fnPPP_crc(unsigned short usCRC, unsigned char ucData)
{
    usCRC = (usCRC >> 8) ^ fcstab[(usCRC ^ ucData) & 0xff];
    return usCRC;
}
#elif !defined USE_SLIP && defined UTASKER_PPP_LOW_LAYER
// Optimised code implementation of CCITT CRC-16
//
static unsigned short fnPPP_crc(unsigned short usCRC, unsigned char ucData)
{
    ucData ^= (unsigned char)usCRC;
    ucData ^= (ucData << 4);

    return ((((unsigned short)ucData << 8) | (usCRC >> 8)) ^ (ucData >> 4) ^ ((unsigned short)ucData << 3));
}
#endif
#endif


#endif
