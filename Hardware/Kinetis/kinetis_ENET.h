/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_ENET.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    21.04.2012 Use link-local IPv6 address for multicast configuration   {11}
    29.04.2012 Add LAN_TX_FPU_WORKAROUND to workaround a problem with the Ethernet controller missing a queued buffer when caching is enabled {12}
    18.06.2012 Add Ethernet parameter to enable passing received CRC-32 to application {13}
    23.06.2012 Enable RMII clock from external pin                       {14}
    14.07.2012 Add _fnMIIread() and _fnMIIwrite()                        {18}
    14.07.2012 Add Micrel SMI mode                                       {19}
    04.08.2012 Configure EMAC buffer descriptor operation for little-endian when this is possible in the device used {21}
    13.08.2012 Add support for magic reset Ethernet reception frame      {22}
    27.09.2012 Correct LINK UP event                                     {27}
    19.10.2012 Exception only on multicast MAC address                   {29}
    17.12.2012 Add interface number to Ethernet when working with multiple IP interfaces {30}
    07.04.2013 Add PHY_POLL_LINK support                                 {37}
    23.06.2013 Add _PHY_KSZ8863 mode                                     {43}
    23.06.2013 Add PHY tail tagging support                              {44}
    08.07.2013 Add full duplex event type                                {45}
    28.07.2013 Allow the PHY interrupt handler to be interrupted by higher priority interrupts {49}
    17.08.2013 Adjust PHY interface when _DP83849I is used               {52}
    16.04.2014 Add fnModifyMulticastFilter() for IGMP use                {74}
    02.06.2014 Add _KSZ8081RNA status register settings                  {84}
    11.06.2015 Modify the handling of link-down with _KSZ8081RNA         {100}
    17.01.2016 Add fnResetENET() when restarting Ethernet (for simulator){101}
    27.01.2016 Add _KSZ8051RNL                                           {102}
    08.03.2017 Add option to not initialise MII_RXER pin (NO_MII_RXER)   {103}
    07.08.2017 Add option for MDIO on port A                             {104}
    07.09.2017 Optionally manage MDIO clock in tail-tagging routine      {105}
    20.01.2019 Share Ethernet driver with iMX project
    30.07.2020 Support second ethernet interface
    07.09.2020 Add fnShowEthernet()                                      {106}

*/

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined ETHERNET_COUNT
    #define ETHERNET_COUNT   1
#endif
#if (ETHERNET_COUNT > 1)
    #define PHY_INTERFACE   ucPhyInterface
#else
    #define PHY_INTERFACE   0
#endif

#if !defined ETH_BD_MALLOC_ALIGN
    #define ETH_BD_MALLOC_ALIGN(x, y) uMallocAlign((MAX_MALLOC)(x), (y))
#endif
#if !defined ETH_RX_BUF_MALLOC_ALIGN
    #define ETH_RX_BUF_MALLOC_ALIGN(x, y) uMallocAlign((MAX_MALLOC)(x), (y))
#endif
#if !defined ETH_TX_BUF_MALLOC_ALIGN
    #define ETH_TX_BUF_MALLOC_ALIGN(x, y) uMallocAlign((MAX_MALLOC)(x), (y))
#endif

#if defined _WINDOWS || defined ETHER_DBSWP                              // {21} device with ETHER_DBSWP control in the ECR register can configure the EMAC buffer descriptors to use little-endian mode
    #define fnLE_ENET_add(x)  x
    #define fnLE_ENET_word(x) x
#else                                                                    // when EMAC operates with buffer descripts in big-endian mode addresses and lengths are converted to little-endian using these macros
    // Convert between little and big-endian address
    //
    #define fnLE_ENET_add(x) (unsigned char *)(((unsigned long)(x) >> 24) | (((unsigned long)(x) >> 8) & 0x0000ff00) | (((unsigned long)(x) << 8) & 0x00ff0000) | (((unsigned long)(x) << 24) & 0xff000000))

    // Convert between little and big-endian short word
    //
    #define fnLE_ENET_word(x) (((unsigned short)(x) >> 8) | ((unsigned short)(x) << 8))
#endif

//#define TEST_CRC_32_IEEE                                               // use Ethernet receptions to verify CRC module CRC-32 IEEE settings (test only)

#if defined EMAC_ENHANCED
    #if defined ETHER_DBSWP                                              // {21} if the EMAC supports buffer descriptors in little-endian format enable this mode
        #define ENABLE_ETHERNET_OPERATION (ETHER_EN | ETHER_EN1588 | ETHER_DBSWP)
    #else
        #define ENABLE_ETHERNET_OPERATION (ETHER_EN | ETHER_EN1588)
    #endif
#else
    #if defined ETHER_DBSWP                                              // {21} if the EMAC supports buffer descriptors in little-endian format enable this mode
        #define ENABLE_ETHERNET_OPERATION (ETHER_EN | ETHER_DBSWP)
    #else
        #define ENABLE_ETHERNET_OPERATION (ETHER_EN)
    #endif
#endif

#if defined _iMX
    #define ETHERNET_CONTROLLER_CLOCK   (IPG_CLK_ROOT_FREQUENCY)
#elif defined ETHERNET_RMII && defined ETHERNET_RMII_CLOCK_INPUT         // RMII mode with external clock inut
    #define ETHERNET_CONTROLLER_CLOCK   50000000                         // 50MHz external clock supplied as Ethernet controller clock
#else
    #define ETHERNET_CONTROLLER_CLOCK   SYSTEM_CLOCK                     // Ethernet controller clocked by the system clock
#endif

#if !defined MSCR_HOLDTIME
    #define MSCR_HOLDTIME   0
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if (defined ETH_INTERFACE && !defined NO_INTERNAL_ETHERNET) && !defined ETHERNET_RELEASE_AFTER_EVERY_FRAME
    static const unsigned char EMAC_RXA_int_message[HEADER_LENGTH] = {0, 0 , TASK_ETHERNET, INTERRUPT_EVENT, EMAC_RX_INTERRUPT}; // define fixed interrupt event
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if (defined ETH_INTERFACE && !defined NO_INTERNAL_ETHERNET)
    #if defined USE_MULTIPLE_BUFFERS
        static unsigned char ucMultiBuf = 0;
    #elif defined _MAGIC_RESET_FRAME
        static volatile KINETIS_FEC_BD *ptrMagicRxBd = 0;                // initially disabled
    #endif
    static KINETIS_FEC_BD *ptrRxBd_start, *ptrTxBd_start;
    static volatile KINETIS_FEC_BD *ptrRxBd, *ptrTxBd;
    static unsigned short usEthernetLength[ETHERNET_COUNT] = {0};
    #if defined PHY_TAIL_TAGGING && !defined NO_INTERNAL_ETHERNET        // {44}
        static int iTailTagging = 0;                                     // tail tagging is disabled by default
        static unsigned char ucTailTagPort = ETH_TX_PORT_SWITCH;         // port for transmission - default is for switch operation
    #endif
    #if defined EMAC_ENHANCED && defined IP_TX_CHECKSUM_OFFLOAD && defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD && defined IPV4_SUPPORT_TX_FRAGMENTATION
        static int iFragmentTx = 0;
    #endif
    #if defined USE_IGMP && !defined NO_INTERNAL_ETHERNET                // {74}
        #if defined USE_IPV6
            #define MAX_MULTICAST_FILTERS (IGMP_MAX_HOSTS + 2)
        #else
            #define MAX_MULTICAST_FILTERS (IGMP_MAX_HOSTS + 1)
        #endif
        static unsigned char ucGroupHashes[MAX_MULTICAST_FILTERS] = {0};
    #endif
    #if defined SCAN_PHY_ADD
        #undef PHY_ADDRESS
        static unsigned char ucPhyAddress = 1;                           // first address to test with - increments until a response is found and then is ued for all further communication
        #define PHY_ADDRESS ucPhyAddress
    #endif
    #if defined _WINDOWS
        static unsigned short usPhyMode = 0;
    #endif
#endif


/* =================================================================== */
/*                          Ethernet Controller                        */
/* =================================================================== */



#if defined PHY_TAIL_TAGGING                                             // {44}
extern void fnSetTailTagMode(int iPort)
{
    if (iPort != iTailTagging) {                                         // only react to changes
    #if defined STOP_MII_CLOCK                                           // {105}
        MSCR = ((((ETHERNET_CONTROLLER_CLOCK / (2 * MII_MANAGEMENT_CLOCK_SPEED)) + 1) << 1) | (MSCR_HOLDTIME << 8)); // enable PHY clock for reads
    #endif
        iTailTagging = iPort;                                            // enable/disable tail tagging (the enabled value controls frame reception when simulating)
    #if defined USE_IP
        fnDeleteArp();                                                   // delete ARP table when changing mode
    #endif
    #if defined USE_IPV6
        fnDeleteNeighbors();                                             // delete the IPV6 neighbor cache when changing mode
    #endif
        if (iTailTagging != 0) {                                         // if enabled
            _fnMIIwrite(0, 3,    0x74);                                  // enable tail-tagging operation at Micrel switch
            _fnMIIwrite(0, 0x11, 0x47);                                  // set both ports to pass all reception to sniffer port
            _fnMIIwrite(0, 0x21, 0x47);
            _fnMIIwrite(0, 0x31, 0x87);                                  // define the processor as sniffer port (promiscuous)
        }
        else {
            _fnMIIwrite(0, 3,    0x34);                                  // disable operation at Micrel switch
            _fnMIIwrite(0, 0x11, 0x07);                                  // don't pass reception to sniffer port
            _fnMIIwrite(0, 0x21, 0x07);
            _fnMIIwrite(0, 0x31, 0x07);                                  // disable the processor as sniffer port (non-promiscuous) since we return to normal switch mode
        }
    #if defined STOP_MII_CLOCK                                           // {105}
        MSCR = 0;                                                        // disable PHY clock after use
    #endif
    }
}

extern int fnGetTailTagMode(void)
{
    return (iTailTagging);                                               // return present mode
}

extern void fnSetTailTagPort(unsigned char ucPort)
{
    ucTailTagPort = ucPort;                                              // set the port for frame transmission
}

extern unsigned char fnGetTailTagPort(void)
{
    return (ucTailTagPort);                                              // return present transmission port
}
#endif

#if defined EMAC_ENHANCED && defined IP_TX_CHECKSUM_OFFLOAD && defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD && defined IPV4_SUPPORT_TX_FRAGMENTATION
extern void fnSetFragmentMode(int iMode)
{
    iFragmentTx = iMode;
}
#endif


// This is called when an Ethernet event has been received. Generally it is due to a reception and returns channel and message details.
//
extern signed char fnEthernetEvent(unsigned char *ucEvent, ETHERNET_FRAME *rx_frame)
{
#if defined LAN_REPORT_ACTIVITY
    static const unsigned char EMAC_int_message[HEADER_LENGTH] = {0, 0, INTERRUPT_TASK_LAN_EXCEPTIONS, INTERRUPT_EVENT, EMAC_RX_INTERRUPT};
#endif
    if (EMAC_RX_INTERRUPT == *ucEvent) {
#if defined USE_MULTIPLE_BUFFERS
        if ((rx_frame->frame_size = fnExtractNextRx()) == 0) {           // get frame details from present buffer descriptor
            return -1;                                                   // nothing (else) waiting
        }
        rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)ucEthernetInput;     // return pointer to the fixed linear input buffer
    #if IP_INTERFACE_COUNT > 1
        rx_frame->ucInterface = (ETHERNET_INTERFACE >> INTERFACE_SHIFT); // {30}
        rx_frame->ucInterfaceHandling = DEFAULT_INTERFACE_CHARACTERISTICS; // handling that this interface needs
    #endif
#else
        if ((ptrRxBd->usBDControl & EMPTY_BUFFER) != 0) {
            return -1;                                                   // nothing waiting
        }
        else if (ptrRxBd->usBDLength == 0) {                             // zero length is invalid
            return -1;
        }
    #if IP_INTERFACE_COUNT > 1
        rx_frame->ucInterface = (ETHERNET_INTERFACE >> INTERFACE_SHIFT); // {30}
        rx_frame->ucInterfaceHandling = DEFAULT_INTERFACE_CHARACTERISTICS; // handling that this interface needs
    #endif
        rx_frame->ptEth = (ETHERNET_FRAME_CONTENT *)fnLE_ENET_add(ptrRxBd->ptrBD_Data); // set pointer to reception content in the buffer descriptor
        if ((ptrRxBd->usBDControl & (TRUNCATED_FRAME | OVERRUN_FRAME | RECEIVE_CRC_ERROR | RECEIVE_NON_OCTET_AL | RECEIVE_LENGTH_VIOL)) != 0) { // corrupted reception
            if ((ptrRxBd->usBDControl & OVERRUN_FRAME) != 0) {
    #if defined USE_IP_STATS
                fnIncrementEthernetStats(TOTAL_LOST_RX_FRAMES, DEFAULT_NETWORK); // we lost a frame due to RX overrun so count the event
    #endif
            }
if ((RECEIVE_CRC_ERROR & ptrRxBd->usBDControl) != 0) { // temp test!
    fnDebugMsg("\r\n\n\nCRC!!!!!!\r\n\n");
}
            rx_frame->frame_size = 0;                                    // reject further frame processing
            ptrRxBd->usBDControl &= ~(TRUNCATED_FRAME | OVERRUN_FRAME | RECEIVE_CRC_ERROR | RECEIVE_NON_OCTET_AL | RECEIVE_LENGTH_VIOL); // reset error flags that could otherwise block further use of the buffer descriptor
        }
        else {                                                           // reception without errors
            rx_frame->frame_size = fnLE_ENET_word(ptrRxBd->usBDLength);  // length in the buffer
            if ((RCR & CRCFWD) == 0) {                                   // if the CRC-32 is not being removed reduce the reception length (the user knows that it will be in the buffer after the data)
                rx_frame->frame_size -= 4;
            }
    #if defined IPV4_SUPPORT_RX_DEFRAGMENTATION && defined IP_RX_CHECKSUM_OFFLOAD
            rx_frame->usFragment_CS = fnLE_ENET_word(ptrRxBd->usPayloadCS); // save the payload checksum of the possible fragment
            rx_frame->ucSpecialHandling = 0;
    #endif
    #if defined PHY_TAIL_TAGGING                                         // {44}
            if (iTailTagging != 0) {                                     // if tail tagging is enabled
        #if defined _WINDOWS
                if (iTailTagging == 2) {
                    rx_frame->ucRxPort = 2;                              // simulate frames arrive over this particular port
                    rx_frame->ucInterface = (PHY2_INTERFACE >> INTERFACE_SHIFT); // simulate frames arrive over this particular port
                }
                else {
                    rx_frame->ucRxPort = 1;                              // simulate frames arrive over this particular port
                    rx_frame->ucInterface = (PHY1_INTERFACE >> INTERFACE_SHIFT); // simulate frames arrive over this particular port
                }
        #else                                                            // in tail tagging mode the source port is read from the received trame
                rx_frame->frame_size--;                                  // remove tail from the length
                if ((rx_frame->ptEth->ucData[rx_frame->frame_size - 14] & 0x01) != 0) {
                    rx_frame->ucRxPort = 2;                              // this Ethernet frame arrived over port 2
                    rx_frame->ucInterface = (PHY2_INTERFACE >> INTERFACE_SHIFT);
                }
                else {
                    rx_frame->ucRxPort = 1;                              // this Ethernet frame arrived over port 1
                    rx_frame->ucInterface = (PHY1_INTERFACE >> INTERFACE_SHIFT);
                }
        #endif
            }
            else {
                rx_frame->ucRxPort = 0;                                  // don't care when tail tagging is not enabled
            }
    #endif
        }
    #if defined TEST_CRC_32_IEEE                                         // test option checking
        if (rx_frame->frame_size != 0) {
            unsigned long ulLength = rx_frame->frame_size;
            unsigned long *ptr = (unsigned long *)rx_frame->ptEth;
            POWER_UP_ATOMIC(6, CRC);                                     // power up the CRC module
            CRC_CTRL = (CRC_CTRL_TCRC_32 | CRC_CTRL_TOTR_BITS_BYTES | CRC_CTRL_TOT_BITS_BYTES | CRC_CTRL_FXOR); // mode required for CRC-32 IEEE compatibility
            CRC_GPOLY = 0x04c11db7;                                      // set the polynomial
            CRC_CTRL = (CRC_CTRL_TCRC_32 | CRC_CTRL_WAS | CRC_CTRL_TOTR_BITS_BYTES | CRC_CTRL_TOT_BITS_BYTES | CRC_CTRL_FXOR); // enable write of seed
            CRC_CRC = 0xffffffff;                                        // seed is 0xffffffff
            CRC_CTRL = (CRC_CTRL_TCRC_32 | CRC_CTRL_TOTR_BITS_BYTES | CRC_CTRL_TOT_BITS_BYTES | CRC_CTRL_FXOR); // switch back to data input mode
            while (ulLength >= 4) {                                      // main part using long words
                CRC_CRC = *ptr++;                                        // process long world
                ulLength -= 4;                                           // by bytes at a time - the CRC calculation requires 2 clock cycles
            }
        #if !defined ERRATA_ID_2776 || defined _WINDOWS
            if (ulLength != 0) {                                         // remaining bytes that didn't fit into a long word
                unsigned char *ptrByte = (unsigned char *)ptr;
                CRC_CRC_HU = *ptrByte++;                                 // write bytes starting with MSB
                ulLength--;
                if (ulLength != 0) {
                    CRC_CRC_HL = *ptrByte++;
                    ulLength--;
                    if (ulLength != 0) {
                        CRC_CRC_LU = *ptrByte++;
                        ulLength--;
                    }
                }
                ptr = (unsigned long *)ptrByte;
            }
        #else                                                            // errata 2776 workaround
            if (ulLength != 0) {                                         // remaining bytes that didn't fit into a long word
            #if !defined _COMPILE_KEIL
                register unsigned long ull = 0;
                register int i;
            #else
                register unsigned long ull;
            #endif
                CRC_CTRL = (CRC_CTRL_TCRC_32 | CRC_CTRL_TOTR_BITS_BYTES | CRC_CTRL_FXOR); // switch to mode without swapping
            #if defined _COMPILE_KEIL
                ull = __rbit(*ptr);                                      // swap all bits of long word
            #else
                // To do - insert asm code for other compilers if required
                //
                for (i = 0; i < 32; i++) {
                    if (*ptr & (0x80000000 >> i)) {
                        ull |= (0x00000001 << i);
                    }
                }
            #endif
                CRC_CRC_LL = (unsigned char)(ull >> 24);                 // swap byte
                ulLength--;
                if (ulLength != 0) {
                    CRC_CRC_LU = (unsigned char)(ull >> 16)              // swap byte
                    ulLength--;
                    if (ulLength != 0) {
                        CRC_CRC_HL = (unsigned char)(ull >> 8);          // swap byte
                        ulLength--;
                    }
                }
            }
        #endif
            if (CRC_CRC == *ptr) {                                       // check the CRC that was calculated with that received at the end of the Ethernet frame
                 // CRC-32 matches
            }
            else {
                // CRC-32 mismatch
            }
        }
    #endif
#endif
#if defined LAN_REPORT_ACTIVITY
        fnWrite(INTERNAL_ROUTE, (unsigned char *)EMAC_int_message, HEADER_LENGTH); // inform the task of event
#endif
        return 0;                                                        // channel received from 0 (fixed value for compatibility with other devices)
    }
#if defined USE_IP_STATS && !defined ETHERNET_RELEASE_AFTER_EVERY_FRAME
    else {
        fnIncrementEthernetStats(TOTAL_OTHER_EVENTS, DEFAULT_NETWORK);   // count other unexpected events
    }
#endif
    return -1;                                                           // channel invalid
}

#if defined LAN_REPORT_ACTIVITY || defined LAN_TX_FPU_WORKAROUND         // {12}
// Ethernet transmission interrupt
//
static __interrupt void _fec_txf_isr(void)
{
    #if defined LAN_REPORT_ACTIVITY
    static const unsigned char EMAC_int_message[HEADER_LENGTH] = {0, 0, INTERRUPT_TASK_LAN_EXCEPTIONS, INTERRUPT_EVENT, EMAC_TX_INTERRUPT};
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when putting message to queue
        fnWrite(INTERNAL_ROUTE, (unsigned char *)EMAC_int_message, HEADER_LENGTH); // inform the task of event
    uEnable_Interrupt();                                                 // release     
    #endif
    WRITE_ONE_TO_CLEAR(EIR, (TXF));                                      // reset interrupt

    #if defined LAN_TX_FPU_WORKAROUND
    TDAR = TDAR_TDAR;                                                    // {12} retrigger buffer polling to ensure that no waiting buffer is missed (using when the errata has been solved can have detremental effects)
    #endif
}
#endif

#if defined _MAGIC_RESET_FRAME
extern void fnEnableMagicFrames(int iOnOff)
{
    uDisable_Interrupt();                                                // protect from ethernet reception interrupts
        if (iOnOff == 0) {
            ptrMagicRxBd = 0;                                            // disable
        }
        else {
            ptrMagicRxBd = ptrRxBd;                                      // enabled by synchronising the magic frame buffer with the receiver
        }
    uEnable_Interrupt();                                                 // allow further interrupt processing
}
#endif

// Re-install flushed buffer descriptors and restart operation
//
static void fnRestartEthernet(int iInterface)
{
#if defined _WINDOWS
    extern void fnResetENET(void);
#endif
    int i;
    KINETIS_FEC_BD *ptrBD = ptrRxBd_start;
#if defined _WINDOWS
    fnResetENET();                                                       // {101} reset simulation
#endif
    ptrRxBd = (volatile KINETIS_FEC_BD *)ptrRxBd_start;
#if defined _MAGIC_RESET_FRAME && !defined USE_MULTIPLE_BUFFERS          // {22}
    if (ptrMagicRxBd != 0) {
        ptrMagicRxBd = ptrRxBd;
    }
#endif
    ERDSR = (unsigned long)ptrRxBd_start;
    EMRBR = usEthernetLength[iInterface];
#if defined USE_MULTIPLE_BUFFERS
    for (i = 0; i < (NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE * ucMultiBuf); i++) { // initialise each Rx buffer descriptor
        if (i == ((NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE * ucMultiBuf) - 1)) {
            ptrBD->usBDControl = (EMPTY_BUFFER | WRAP_BIT_RX);
        }
        else {
            ptrBD->usBDControl = EMPTY_BUFFER;
        }
        ptrBD++;
    }
#else
    for (i = 0; i < NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE; i++) {      // initialise each Rx buffer descriptor
        if (i == (NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE - 1)) {
            ptrBD->usBDControl = (EMPTY_BUFFER | WRAP_BIT_RX);
        }
        else {
            ptrBD->usBDControl = EMPTY_BUFFER;
        }
        ptrBD++;
    }
#endif
    ptrBD = ptrTxBd_start;
    ptrTxBd = ptrTxBd_start;
    ETDSR = (unsigned long)ptrTxBd_start;
    for (i = 0; i < NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE; i++) {
        if (i == (NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE - 1)) {
            ptrBD->usBDControl = (WRAP_BIT_TX);
        }
        else {
            ptrBD->usBDControl = 0;
        }
        ptrBD++;
    }
    ECR = ENABLE_ETHERNET_OPERATION;                                     // re-enable FEC
    RDAR = 0;                                                            // enable RX BD polling
}

// Ethernet error and miscellaneous interrupt
//
#if defined _iMX
static void _fec_misc(KINETIS_FAST_ENET *ptrEnet, int iInterface)
{
    unsigned long ulSources = ptrEnet->ENET_EIR;                         // check which interrupt occurred
    WRITE_ONE_TO_CLEAR(ptrEnet->ENET_EIR, (BABR | BABT | GRA | EBERR | LC | RL | UN)); // reset the interrupt event (put break point here to analyse what happened)
    if ((ulSources & EBERR) != 0) {                                      // the operation will be stopped if there is a DMA error
        fnRestartEthernet(iInterface);                                   // restart the ethernet controller in order to continue
    }
}
#else
static __interrupt void _fec_misc(void)
{
    unsigned long ulSources = EIR;                                       // check which interrupt occurred
    WRITE_ONE_TO_CLEAR(EIR, (BABR | BABT | GRA | EBERR | LC | RL | UN)); // reset the interrupt event (put break point here to analyse what happened)
    if ((ulSources & EBERR) != 0) {                                      // the operation will be stopped if there is a DMA error
        fnRestartEthernet(0);                                            // restart the ethernet controller in order to continue
    }
}
#endif

// Ethernet reception interrupt
//
#if (ETHERNET_COUNT > 1)
static void _fec_rx_frame_isr(int iInterface)
#else
static __interrupt void _fec_rx_frame_isr(void)
#endif
{
    KINETIS_FAST_ENET *ptrEnet;
#if (ETHERNET_COUNT > 1)
    if (iInterface > 0) {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC2_BASE_ADD;
    }
    else {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    }
#else
    ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    #if defined _iMX
    int iInterface = 0;
    #endif
#endif
#if defined _MAGIC_RESET_FRAME  && !defined USE_MULTIPLE_BUFFERS         // {22}
    if (ptrMagicRxBd != 0) {
        if ((ptrMagicRxBd->usBDControl & EMPTY_BUFFER) == 0) {           // ignore if buffer is empty
            if ((ptrMagicRxBd->usBDControl & (TRUNCATED_FRAME | OVERRUN_FRAME)) == 0) { // only handle valid frames
                unsigned short usLength = fnLE_ENET_word(ptrMagicRxBd->usBDLength); // length in the buffer
                if ((ptrEnet->ENET_RCR & CRCFWD) == 0) {                 // if the CRC-32 is not being removed reduce the reception length (the user knows that it will be in the buffer after the data)
                    usLength -= 4;
                }
                if (usLength == 93) {                                    // payload length must be 93 bytes in length
                    unsigned char *ptrData = &((ETHERNET_FRAME_CONTENT *)fnLE_ENET_add(ptrMagicRxBd->ptrBD_Data))->ethernet_frame_type[0];
                    int i;
                    for (i = 0; i < 24; i++) {                           // remaining header must be (unsigned short)_MAGIC_RESET_FRAME repeated 24 times
                        if (*ptrData++ != (unsigned char)(_MAGIC_RESET_FRAME >> 8)) {
                            goto _not_magic_frame;
                        }
                        if (*ptrData++ != (unsigned char)(_MAGIC_RESET_FRAME)) {
                            goto _not_magic_frame;
                        }
                    }
                    // A magic frame has been received - the 61st byte is used to signal the type and 32 bytes of optional data are passed
                    //
                    fnMagicFrame(*ptrData, (ptrData + 1));               // allow the user to handle the magic frame
                }
            }
_not_magic_frame:
            if ((ptrMagicRxBd->usBDControl & WRAP_BIT_RX) != 0) {
                ptrMagicRxBd = (KINETIS_FEC_BD *)ptrEnet->ENET_ERDSR;    // set to first BD
            }
            else {
                ptrMagicRxBd++;                                          // increment to next buffer
            }
        }
    }
#endif
#if defined ETHERNET_RELEASE_AFTER_EVERY_FRAME
    uTaskerStateChange(TASK_ETHERNET, UTASKER_ACTIVATE);                 // schedule the Ethernet task
#else
    uDisable_Interrupt();                                                // ensure interrupts remain blocked when putting message to queue
        fnWrite(INTERNAL_ROUTE, (unsigned char *)EMAC_RXA_int_message, HEADER_LENGTH); // inform the Ethernet task that a new frame is waiting
    uEnable_Interrupt();                                                 // release  
#endif
    WRITE_ONE_TO_CLEAR(ptrEnet->ENET_EIR, (RXF));                        // reset interrupt flag
#if defined _iMX                                                         // i.MX RT has one interrupt vector so we check for miscellaneous interrupts
    if ((ptrEnet->ENET_EIR & (BABR | BABT | GRA | EBERR | LC | RL | UN)) != 0) { // check for error or miscellaneous interrupts
fnDebugMsg("MISC ETHERNET!!! ");
fnDebugHex(ptrEnet->ENET_EIR, (sizeof(ptrEnet->ENET_EIR) | WITH_LEADIN | WITH_CR_LF));
        _fec_misc(ptrEnet, iInterface);
    }
#endif
}

#if (ETHERNET_COUNT > 1)
static __interrupt void _fec_rx_frame_isr_2(void)
{
    _fec_rx_frame_isr(1);
}

static __interrupt void _fec_rx_frame_isr_1(void)
{
    _fec_rx_frame_isr(0);
}
#endif


#if defined _DP83849I || defined _DP83825I                               // {52} adjust registers and flags to suit the PHY
    #define PHY_INTERRUPT_REGISTER 0x12
    #define PHY_LINK_STATE_CHANGE  0x2000
    #define PHY_LINK_STATUS_REG    0x10
    #define PHY_LINK_MASK          0x0007
    #define PHY_LINK_10BASE_T_HALF_DUPLEX   0x0003
    #define PHY_LINK_100BASE_TX_HALF_DUPLEX 0x0001
    #define PHY_LINK_10BASE_T_FULL_DUPLEX   0x0007
    #define PHY_LINK_100BASE_TX_FULL_DUPLEX 0x0005
#elif defined _LAN8720 || defined _LAN8740 || defined _LAN8742
    #define PHY_INTERRUPT_REGISTER (0x1d)
    #define PHY_LINK_STATE_CHANGE  (0x0050)                              // auto-negotiation complete or link-down
    #define PHY_LINK_STATUS_REG    (0x1f)
    #define PHY_LINK_MASK          (0x0014)
    #define PHY_LINK_10BASE_T_FULL_DUPLEX   0x0014
    #define PHY_LINK_100BASE_TX_FULL_DUPLEX 0x0018
    #define PHY_LINK_100BASE_TX_HALF_DUPLEX 0x0008
    #define PHY_LINK_10BASE_T_HALF_DUPLEX   0x0004
#elif (defined _KSZ8081RNA || defined _KSZ8081RNB) || defined _KSZ8051RNL// {84}{102}
    #define PHY_INTERRUPT_REGISTER KS8041_INTERRUPT_CONTROL_STATUS_REGISTER // 0x1b
    #define PHY_LINK_STATE_CHANGE  (PHY_LINK_UP_INT)                     // {100}
    #define PHY_LINK_STATUS_REG    0x1e                                  // PHY control 1
    #define PHY_LINK_MASK          0x0007
    #define PHY_LINK_10BASE_T_FULL_DUPLEX   0x0005
    #define PHY_LINK_100BASE_TX_FULL_DUPLEX 0x0006
    #define PHY_LINK_100BASE_TX_HALF_DUPLEX 0x0002
    #define PHY_LINK_10BASE_T_HALF_DUPLEX   0x0001
#else
    #define PHY_INTERRUPT_REGISTER KS8041_INTERRUPT_CONTROL_STATUS_REGISTER
    #define PHY_LINK_STATE_CHANGE  PHY_LINK_UP_INT
    #define PHY_LINK_STATUS_REG    KS8041_100BASETX_PHY_CONTROL_REGISTER
    #define PHY_LINK_MASK          PHY_MODE_MASK
    #define PHY_LINK_10BASE_T_HALF_DUPLEX   PHY_MODE_10BASE_T_HALF_DUPLEX
    #define PHY_LINK_100BASE_TX_HALF_DUPLEX PHY_MODE_100BASE_TX_HALF_DUPLEX
    #define PHY_LINK_10BASE_T_FULL_DUPLEX   PHY_MODE_10BASE_T_FULL_DUPLEX
    #define PHY_LINK_100BASE_TX_FULL_DUPLEX PHY_MODE_100BASE_TX_FULL_DUPLEX
#endif

// Function to read from the MII interface
//
static unsigned short fnMIIread(unsigned char _mpadr, unsigned char _mradr)
{
    KINETIS_FAST_ENET *ptrEnet;
#if (ETHERNET_COUNT > 1)
    int iInterface = (_mpadr >> 6);                                      // extract the PHY interface
    _mpadr &= 0x3f;
    if (iInterface > 0) {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC2_BASE_ADD;
    }
    else {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    }
#else
    ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
#endif
#if defined PHY_MICREL_SMI
    if (_mpadr == 0) {                                                   // {19}
    	int i;
        #define MDIO_LINE      PORTB_BIT0
        #define MDC_LINE       PORTB_BIT1
        #define SET_MDC_HIGH()  _SETBITS(B, MDC_LINE);_SETBITS(B, MDC_LINE);_SETBITS(B, MDC_LINE);_SETBITS(B, MDC_LINE)
        #define SET_MDC_LOW()   _CLEARBITS(B, MDC_LINE);_CLEARBITS(B, MDC_LINE);_CLEARBITS(B, MDC_LINE);_CLEARBITS(B, MDC_LINE)
        // Bit bang to achieve the operation since the read doesn't work with the Kinetis in SMI mode
        //
        _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, (MDIO_LINE | MDC_LINE), (MDIO_LINE), (PORT_SRE_FAST)); // GPIO driving with initial value
    	for (i = 0; i < 32; i++) {
            SET_MDC_HIGH();                                              // preamble with 32 clocks
            SET_MDC_LOW();
        }
        _CLEARBITS(B, MDIO_LINE);                                        // '0'
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
        _SETBITS(B, MDIO_LINE);                                          // '1'
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
        _CLEARBITS(B, MDIO_LINE);                                        // '0'
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
        _SETBITS(B, MDIO_LINE);                                          // '1'
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
        SET_MDC_HIGH();                                                  // clock
        SET_MDC_LOW();
    	for (i = 0; i < 8; i++) {                                        // register value
        	if (_mradr & 0x80) {
                _SETBITS(B, MDIO_LINE);                                  // '1'
            }
        	else {
                _CLEARBITS(B, MDIO_LINE);                                // '0'
            }
            SET_MDC_HIGH();                                              // clock
        	_mradr <<= 1;
            SET_MDC_LOW();
        }
        SET_MDC_HIGH();                                                  // clock
        _FLOAT_PORT(B, MDIO_LINE);                                       // set data line as input
        SET_MDC_LOW();

    	for (i = 0; i < 9; i++) {                                        // 9 clocks reading in '0'
            SET_MDC_HIGH();                                              // clock
            SET_MDC_LOW();
        }
    	_mradr = 0;
    	for (i = 0; i < 8; i++) {                                        // 8 clocks reading in data
            SET_MDC_HIGH();                                              // clock
        	_mradr <<= 1;
            if (_READ_PORT_MASK(B, MDIO_LINE)) {                         // if '1'
            	_mradr |= 1;
            }
            SET_MDC_LOW();
        }
        // Return pin mode when completed
        //
        _CONFIG_PERIPHERAL(B, 0, PB_0_MII0_MDIO);                        // MII0_MDIO on PB.0 (alt. function 4)
        _CONFIG_PERIPHERAL(B, 1, PB_1_MII0_MDC);                         // MII0_MDC on PB.1 (alt. function 4)
    	return _mradr;
    }
	else {
        ptrEnet->ENET_MMFR = (MII_READ | (_mpadr << 23) | (_mradr << 18)); // command read from given address
    }
#else
    ptrEnet->ENET_MMFR = (MII_READ | (_mpadr << 23) | (_mradr << 18));
#endif

    while ((ptrEnet->ENET_EIR & MII) == 0) {                             // wait until the read has completed
#if defined _WINDOWS
    #if defined STOP_MII_CLOCK
        if (ptrEnet->ENET_MSCR == 0) {
            _EXCEPTION("Trying to read from PHY without clock enabled!");
        }        
    #endif
        ptrEnet->ENET_EIR |= MII;
#endif
    };
    ptrEnet->ENET_EIR = MII;                                             // reset the interrupt event
#if defined _WINDOWS
    ptrEnet->ENET_EIR &= ~MII;
    if (PHY_INTERRUPT_REGISTER == _mradr) {      
        ptrEnet->ENET_MMFR = PHY_LINK_STATE_CHANGE;                      // link state change
    }
    else if (PHY_REG_CR == _mradr) {
        ptrEnet->ENET_MMFR = (PHY_R0_DR | PHY_R0_ANE);                   // default is typically auto-negotiation with 100M speed set
    }
    else if (PHY_LINK_STATUS_REG == _mradr) {
        ptrEnet->ENET_MMFR = PHY_LINK_10BASE_T_FULL_DUPLEX;              // state link up 100M
    }
    else if (PHY_REG_ANAR == _mradr) {
        ptrEnet->ENET_MMFR = (PHY_R4_NP | PHY_R4_100F | PHY_R4_100H | PHY_R4_10F | PHY_R4_10H | 1); // capabilities
    }
    #if defined PHY_IDENTIFIER
    else if (PHY_REG_ID1 == _mradr) {
        ptrEnet->ENET_MMFR = (unsigned short)(PHY_IDENTIFIER >> 16);     // ID 1
    }
    else if (PHY_REG_ID2 == _mradr) {
        ptrEnet->ENET_MMFR = (unsigned short)(PHY_IDENTIFIER);           // ID 2
    }
    #endif
    #if defined PHY_INTERRUPT
    else if (KS8041_INTERRUPT_CONTROL_STATUS_REGISTER == _mradr) {
        unsigned long ulBit = PHY_INTERRUPT;
        unsigned char ucPortBit = 0;
        while ((ulBit & 0x80000000) == 0) {
            ucPortBit++;
            ulBit <<= 1;
        }
        ptrEnet->ENET_MMFR = PHY_LINK_UP_INT;
        fnSimulateInputChange(PHY_INTERRUPT_PORT, ucPortBit, SET_INPUT); // clear level sensitive interrupt input
    }
    else if (KS8041_BASIC_STATUS_REGISTER == _mradr) {
        ptrEnet->ENET_MMFR = PHY_100MEG_HALF_DUPLEX;
    }
    #endif
#endif
    return (unsigned short)ptrEnet->ENET_MMFR;                           // return the data read
}


#if defined PHY_POLL_LINK || defined PHY_INTERRUPT                       // {37}
extern void fnCheckEthLinkState(void)
{
    volatile unsigned short usInterrupt;
    #if defined INTERRUPT_TASK_PHY
    unsigned char int_phy_message[HEADER_LENGTH];
    #endif
    KINETIS_FAST_ENET *ptrEnet;
    #if (ETHERNET_COUNT > 1)                                             // only i.MX RT
    unsigned char ucPhyInterface = 0;
    int iInterface = 0;
    if (iInterface > 0) {
        if (IS_POWERED_UP(7, ENET2_CLOCK) == 0) {
            return;                                                      // don't check the link state if the ethernet module hasn't been powered up
        }
        ptrEnet = (KINETIS_FAST_ENET *)EMAC2_BASE_ADD;
    }
    else {
        if (IS_POWERED_UP(1, ENET_CLOCK) == 0) {
            return;
        }
        ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    }
    #else
        #if defined _iMX
    if (IS_POWERED_UP(1, ENET_CLOCK) == 0)
        #else
    if (IS_POWERED_UP(2, ENET) == 0)
        #endif
    {
        return;                                                          // ignore if the ethernet controller is not clocked
    }
    ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    #endif
    #if defined STOP_MII_CLOCK                                           // if the MDIO clock is stopped between accesses
    ptrEnet->ENET_MSCR = ((((ETHERNET_CONTROLLER_CLOCK / (2 * MII_MANAGEMENT_CLOCK_SPEED)) - 1) << 1) | (MSCR_HOLDTIME << 8)); // enable PHY clock for the read
    #endif
    usInterrupt = fnMIIread(PHY_ADDRESS, PHY_INTERRUPT_REGISTER);        // read the cause(s) of the interrupt, which resets the bits
    if ((PHY_LINK_STATE_CHANGE & usInterrupt) != 0) {                    // a link state change has taken place
        int iFullDuplex = 0;
        usInterrupt = fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_LINK_STATUS_REG); // check the details of link
        switch (usInterrupt & PHY_LINK_MASK) {        
        case PHY_LINK_100BASE_TX_FULL_DUPLEX:
            iFullDuplex = 1;
        case PHY_LINK_100BASE_TX_HALF_DUPLEX:
    #if defined INTERRUPT_TASK_PHY
            int_phy_message[MSG_INTERRUPT_EVENT] = (unsigned char)(LAN_LINK_UP_100 + iFullDuplex); // {45}
    #endif
    #if defined ETHERNET_RMII
            ptrEnet->ENET_RCR &= ~RMII_10T;                              // ensure that RMII is operating in 100M mode
    #endif
            break;
        case PHY_LINK_10BASE_T_FULL_DUPLEX:
            iFullDuplex = 1;
        case PHY_LINK_10BASE_T_HALF_DUPLEX:
    #if defined INTERRUPT_TASK_PHY
            int_phy_message[MSG_INTERRUPT_EVENT] = (unsigned char)(LAN_LINK_UP_10 + iFullDuplex); // {45}
    #endif
    #if defined ETHERNET_RMII
            ptrEnet->ENET_RCR |= RMII_10T;                               // ensure that RMII is operating in 10M mode
    #endif
            break;
    #if defined _DP83849I && defined INTERRUPT_TASK_PHY                  // {52}{84}{100}
        default:                                                         // link has gone down
            int_phy_message[MSG_INTERRUPT_EVENT] = LAN_LINK_DOWN;
            break;
    #endif
        }
        if (((iFullDuplex != 0) && ((ptrEnet->ENET_RCR & DRT) != 0)) || ((iFullDuplex == 0) && ((ptrEnet->ENET_TCR & FDEN) != 0))) { // only restart if duplex mode has changed
            ptrEnet->ENET_ECR = 0;                                       // disable FEC in order to modify duplex mode
            if (iFullDuplex != 0) {                                      // when using RMII it is important to synchronise the mode
                ptrEnet->ENET_TCR |= FDEN;
                ptrEnet->ENET_RCR &= ~DRT;                               // full duplex
            }
            else {
                ptrEnet->ENET_TCR &= ~FDEN;
                ptrEnet->ENET_RCR |= DRT;                                // half duplex
            }
            fnRestartEthernet(0);                                        // a change in mode require a restart of the Ethernet interface
        }
    }
    #if defined INTERRUPT_TASK_PHY
        #if defined _DP83849I                                            // {52}
    else {
            #if defined STOP_MII_CLOCK                                   // moved down to allow else before the LINK DOWN check
        ptrEnet->ENET_MSCR = 0;                                          // disable clock
            #endif
        return;                                                          // link state has not changed so ignore
    }
        #else
    else if (PHY_LINK_DOWN_INT & usInterrupt) {                          // {27} add else so that the LINK UP event is not overwritten to UNKNOWN
        int_phy_message[MSG_INTERRUPT_EVENT] = LAN_LINK_DOWN;
    }
    else {
        int_phy_message[MSG_INTERRUPT_EVENT] = UNKNOWN_INTERRUPT;
    }
        #endif    
    int_phy_message[MSG_DESTINATION_NODE] = int_phy_message[MSG_SOURCE_NODE] = INTERNAL_ROUTE;
    int_phy_message[MSG_DESTINATION_TASK] = INTERRUPT_TASK_PHY;
    int_phy_message[MSG_SOURCE_TASK] = INTERRUPT_EVENT;
    uDisable_Interrupt();                                                // {49} ensure interrupts remain blocked when putting message to queue
        fnWrite(INTERNAL_ROUTE, (unsigned char *)int_phy_message, HEADER_LENGTH); // inform the corresponding task
    uEnable_Interrupt();
    #endif
    #if defined STOP_MII_CLOCK                                           // {27} moved down to allow else before the LINK DOWN check
    ptrEnet->ENET_MSCR = 0;                                              // disable clock
    #endif
}
#endif

#if defined PHY_INTERRUPT
static void fnPhyInterrupt(void)                                         // port interrupt call-back
{
    uEnable_Interrupt();                                                 // {49} don't block higher priority interrupts while handling
        fnCheckEthLinkState();                                           // {37}
    uDisable_Interrupt();                                                // block interrupts again for compatibility
}
#endif

// Function to write to the MII interface
//
static void fnMIIwrite(unsigned char _mpadr, unsigned char _mradr, unsigned short _mwdata)
{
    KINETIS_FAST_ENET *ptrEnet;
#if (ETHERNET_COUNT > 1)
    int iInterface = (_mpadr >> 6);                                      // extract the PHY interface
    _mpadr &= 0x3f;
    if (iInterface > 0) {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC2_BASE_ADD;
    }
    else {
        ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
    }
#else
    ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
#endif
#if defined PHY_MICREL_SMI
    if (_mpadr == 0) {                                                   // {19}
        ptrEnet->ENET_MMFR = ((MII_TA | MII_ST) | (_mradr << 18) | _mwdata); // command SMI write to given SMI register
    }
    else {
        ptrEnet->ENET_MMFR = (MII_WRITE | (_mpadr << 23) | (_mradr << 18) | _mwdata); // command write to given address
    }
#else
    ptrEnet->ENET_MMFR = (MII_WRITE | (_mpadr << 23) | (_mradr << 18) | _mwdata);
#endif
    while ((ptrEnet->ENET_EIR & MII) == 0) {                             // wait until the write has completed
#if defined _WINDOWS
    #if defined STOP_MII_CLOCK
        if (ptrEnet->ENET_MSCR == 0) {
            _EXCEPTION("Trying to write to PHY without clock enabled!");
        }
    #endif
        ptrEnet->ENET_EIR |= MII;
#endif
    };
    WRITE_ONE_TO_CLEAR(ptrEnet->ENET_EIR, (MII));                        // reset the interrupt event
}

#if defined PHY_MULTI_PORT                                               // {18}
extern unsigned short _fnMIIread(unsigned char ucPort, unsigned char _mradr)
{
    return (fnMIIread(ucPort, _mradr));
}

extern void _fnMIIwrite(unsigned char ucPort, unsigned char _mradr, unsigned short _mwdata)
{
    fnMIIwrite(ucPort, _mradr, _mwdata);
}
#else
extern unsigned short _fnMIIread(unsigned char _mradr)
{
    unsigned short usRegisterValue;
    #if defined STOP_MII_CLOCK
    MSCR = ((((ETHERNET_CONTROLLER_CLOCK / (2 * MII_MANAGEMENT_CLOCK_SPEED)) + 1) << 1) | (MSCR_HOLDTIME << 8)); // enable PHY clock for accesses
    #endif
    usRegisterValue = fnMIIread(PHY_ADDRESS, _mradr);
    #if defined STOP_MII_CLOCK
    MSCR = 0;                                                            // disable the clock again
    #endif
    return usRegisterValue;
}

extern void _fnMIIwrite(unsigned char _mradr, unsigned short _mwdata)
{
    #if defined STOP_MII_CLOCK
    MSCR = ((((ETHERNET_CONTROLLER_CLOCK / (2 * MII_MANAGEMENT_CLOCK_SPEED)) + 1) << 1) | (MSCR_HOLDTIME << 8)); // enable PHY clock for accesses
    #endif
    fnMIIwrite(PHY_ADDRESS, _mradr, _mwdata);
    #if defined STOP_MII_CLOCK
    MSCR = 0;                                                            // disable the clock again
    #endif
}
#endif

#if defined USE_IGMP || defined USE_IPV6
static unsigned char fnCalculateHash(unsigned char *ptrMac)
{
    #define CRC_POLY 0xedb88320                                          // CRC-32 polynomial in reverse direction
    unsigned long ulCRC32 = 0xffffffff;
    int i, iBit;
    unsigned char ucByte;
	for (i = 0; i < MAC_LENGTH; i++) {
        ucByte = *ptrMac++;
        for (iBit = 0; iBit < 8; iBit++) {
            if ((ulCRC32 ^ ucByte) & 1) {
                ulCRC32 = (ulCRC32 >> 1) ^ CRC_POLY;
            }
            else {
                ulCRC32 >>= 1;
            }
            ucByte >>= 1;
        }
    }
    ulCRC32 >>= 26;                                                      // the upper 6 bits of the CRC32 determine the hash entry location
    return (unsigned char)ulCRC32;
}

static void fnEnableMulticastHash(unsigned char ucHashRef)
{
    if (ucHashRef < 32) {
        GALR |= (0x00000001 << ucHashRef);                               // enter single group hash entry
    }
    else {
        ucHashRef -= 32;
        GAUR |= (0x00000001 << ucHashRef);
    }
}
#endif

#if defined USE_IPV6
// Configure a multicast address for IPV6 use
//
static void fnConfigureMulticastIPV6(void)
{
    unsigned char ucHashRef;
    unsigned char ucMac[MAC_LENGTH];                                     // set hash according to the multicast address 0x33 0x33 0xff XX XX XX (where XX XX XX are the last three bytes of the IPV6 IP address) as used by ND (neighbor discovery)
    ucMac[0] = 0x33;
    ucMac[1] = 0x33;
    ucMac[2] = 0xff;
    ucMac[3] = ucLinkLocalIPv6Address[DEFAULT_NETWORK][3 + (IPV6_LENGTH - MAC_LENGTH)];
    ucMac[4] = ucLinkLocalIPv6Address[DEFAULT_NETWORK][4 + (IPV6_LENGTH - MAC_LENGTH)];
    ucMac[5] = ucLinkLocalIPv6Address[DEFAULT_NETWORK][5 + (IPV6_LENGTH - MAC_LENGTH)];
    ucHashRef = fnCalculateHash(ucMac);
    fnEnableMulticastHash(ucHashRef);
    #if defined USE_IGMP
    ucGroupHashes[0] = ucHashRef;                                        // mark that this hash is required by IPv6
    #endif
}
#endif

#if defined USE_IGMP                                                     // {74}
static void fnDisableMulticastHash(unsigned char ucHashRef)
{
    if (ucHashRef < 32) {
        GALR &= ~(0x00000001 << ucHashRef);                              // enter single group hash entry
    }
    else {
        ucHashRef -= 32;
        GAUR &= ~(0x00000001 << ucHashRef);
    }
}

// IGMP adds and removes multicast host groups and requires the receiver's multicast filtering to be modified in the process
//
extern void fnModifyMulticastFilter(QUEUE_TRANSFER action, unsigned char *ptrIP)
{
    int iFree = -1;
    int iMatch = -1;
    int i;
    int iHashCount = 0;
    unsigned char ucMac[MAC_LENGTH];
    unsigned char ucHashRef;
    switch (action) {
    case REMOVE_IPV4_FILTER:
    case ADD_IPV4_FILTER:                                                // a filter has to be enabled for this IPv4 address
        ucMac[0] = 0x01;                                                 // IANA owned multicast ethernet address block
        ucMac[1] = 0x00;
        ucMac[2] = 0x5e;                                                 // enter the multicast MAC address belonging to IANA for this purpose
        ucMac[3] = (*(ptrIP + 1) & 0x7f);                                // add 23 bits of the destination IP address
        ucMac[4] = *(ptrIP + 2);
        ucMac[5] = *(ptrIP + 3);
        ucHashRef = fnCalculateHash(ucMac);
        break;
    default:                                                             // ignore any other calls
        return;
    }
    for (i = 0; i < MAX_MULTICAST_FILTERS; i++) {                        // check whether the entry exists
        if (ucGroupHashes[i] == ucHashRef) {                             // existing hash entry found
            iMatch = i;
            iHashCount++;                                                // count the number of groups requiring this hash
        }
        else if (ucGroupHashes[i] == 0) {                                // free location
            iFree = i;                                                   // remember the free entry
        }
    }
    if (action == ADD_IPV4_FILTER) {                                     // the entry doesn't exist so we add it
        if (iFree < 0) {
            _EXCEPTION("No more hash entry space");
        }
        else {
            ucGroupHashes[iFree] = ucHashRef;                            // note that this hash is required
            fnEnableMulticastHash(ucHashRef);                            // enable the hash entry
        }
    }
    else if (action == REMOVE_IPV4_FILTER) {
        if (iMatch >= 0) {                                               // remove an entry for this hash
            ucGroupHashes[iMatch] = 0;                                   // entry free
            if (iHashCount == 1) {                                       // single entry so it can be disabled
                fnDisableMulticastHash(ucHashRef);
            }
        }
    }
}
#endif

// Ethernet configuration routine
//
extern int fnConfigEthernet(ETHTABLE *pars)
{
    unsigned long ulPhyIdentifier;
    unsigned short usRxSize = pars->usSizeRx;
    #if defined PHY_INTERRUPT
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    #endif
    unsigned short usMIIData;
    unsigned char *ptrBuffer;
    KINETIS_FEC_BD *ptrBD;
    int i;
    KINETIS_FAST_ENET *ptrEnet;
    #if defined _iMX
        #if (ETHERNET_COUNT > 1)
    unsigned char ucPhyInterface = 0;
    if (pars->Channel > 0) {
        POWER_UP_ATOMIC(7, ENET2_CLOCK);                                 // power up the second Ethernet controller
        ptrEnet = (KINETIS_FAST_ENET *)EMAC2_BASE_ADD;
        ucPhyInterface = (1 << 6);
    }
    else {
        #endif
        POWER_UP_ATOMIC(1, ENET_CLOCK);                                  // power up the first Ethernet controller
        ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
        #if !defined PERIPH_CLK_SOURCE_PLL6_500M
        _WAIT_REGISTER_FALSE(CCM_ANALOG_PLL_ENET, CCM_ANALOG_PLL_ENET_LOCK); // wait for PLL6 to lock at 500MHz (enabled in start-up code)
        #endif
        #if defined ETHERNET_RMII
            #if defined iMX_RT102X
        IOMUXC_ENET_RMII_SELECT_INPUT = IOMUXC_ENET_RMII_SELECT_INPUT_GPIO_AD_B0_08_ALT4; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_08, ENET0_REF_CLK1, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_REF_CLK1 on GPIO1-08 (alt. function 4)
            #else
        IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT_GPIO_B1_10_ALT6; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL_LOOPBACK(GPIO_B1_10, ENET0_REF_CLK, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_REF_CLK on GPIO2-26 (alt. function 6)
            #endif
        CCM_ANALOG_PLL_ENET = ((CCM_ANALOG_PLL_ENET & ~(CCM_ANALOG_PLL_ENET_BYPASS | CCM_ANALOG_PLL_ENET_DIV_SELECT_125M)) | (CCM_ANALOG_PLL_ENET_DIV_SELECT_50M | CCM_ANALOG_PLL_ENET_25M_REF_EN)); // enable 50MHz
        IOMUXC_GPR_GPR1 = ((IOMUXC_GPR_GPR1 & ~IOMUXC_GPR_GPR1_ENET1_CLK_SEL) | (IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR)); // select the clock direction as output
        #else
        CCM_ANALOG_PLL_ENET = ((CCM_ANALOG_PLL_ENET & ~(CCM_ANALOG_PLL_ENET_BYPASS | CCM_ANALOG_PLL_ENET_DIV_SELECT_125M))); // enable 25MHz
        #endif
        #if (ETHERNET_COUNT > 1)
    }
        #endif
    #else                                                                // kinetis
        #if defined ETHERNET_RMII && !defined ETHERNET_RMII_CLOCK_INPUT  // {14} don't do this check if configured to use external reference clock on dedicated input
            #if OSCERCLK != 50000000
                #error Ethernet RMII operation requires a 50MHz external clock signal!!
            #endif
        #endif
    POWER_UP_ATOMIC(2, ENET);                                            // power up the Ethernet controller
    ptrEnet = (KINETIS_FAST_ENET *)EMAC_BASE_ADD;
        #if defined MPU_AVAILABLE
    MPU_CESR = 0;                                                        // allow concurrent access to MPU controller
        #endif
    #endif
    #if defined FORCE_PHY_CONFIG
    FNFORCE_PHY_CONFIG();                                                // drive configuration lines and reset
    #endif
    #if defined _iMX
        #if (ETHERNET_COUNT > 1)
    if (pars->Channel > 0) {
        _CONFIG_PERIPHERAL_LOOPBACK(GPIO_B0_15, ENET2_REF_CLK2, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET2_REF_CLK2 on GPIO2-15 (alt. function 6)
        _CONFIG_PERIPHERAL(GPIO_B0_00, ENET2_MDC, (PORT_DSE_MID));       // ENET2_MDC on GPIO2-00 (alt. function 8)
        _CONFIG_PERIPHERAL(GPIO_B0_01, ENET2_MDIO, (PORT_PS_UP_ENABLE)); // ENET2_MDIO on GPIO2-01 (alt. function 8) with pullup enabled
        _CONFIG_PERIPHERAL(GPIO_B0_10, ENET2_CRS, 0);                    // ENET2_RX_EN on GPIO2-10 (alt. function 8) - ENET_CRS_DV
        _CONFIG_PERIPHERAL(GPIO_B0_12, ENET2_TDATA00, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET2_TDATA00 on GPIO2-12 (alt. function 8)
        _CONFIG_PERIPHERAL(GPIO_B0_13, ENET2_TDATA01, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET2_TDATA01 on GPIO2-13 (alt. function 8)
        _CONFIG_PERIPHERAL(GPIO_B0_14, ENET2_TX_EN, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET2_TX_EN on GPIO2-14 (alt. function 8)
        _CONFIG_PERIPHERAL(GPIO_B1_01, ENET2_RDATA00, 0);                // ENET2_RX_DATA00 on GPIO2-20 (alt. function 8)
        _CONFIG_PERIPHERAL(GPIO_B1_02, ENET2_RDATA01, 0);                // ENET2_RX_DATA01 on GPIO2-21 (alt. function 8)
    }
    else {
        #endif
        #if defined MDIO_ON_PORT1
        _CONFIG_PERIPHERAL(GPIO_AD_B0_05, ENET0_MDC, (PORT_DSE_MID));    // ENET0_MDC on GPIO1-05 (alt. function 4)
        _CONFIG_PERIPHERAL(GPIO_AD_B0_04, ENET0_MDIO, (PORT_PS_UP_ENABLE)); // ENET0_MDIO on GPIO1-04 (alt. function 4) with pullup enabled
        #elif defined MDIO_ON_PORT2
        _CONFIG_PERIPHERAL(GPIO_B1_14, ENET_MDC, (PORT_DSE_MID));        // ENET_MDC on GPIO2-30 (alt. function 0)
        IOMUXC_ENET_MDIO_SELECT_INPUT = IOMUXC_ENET_MDIO_SELECT_INPUT_GPIO_B1_15_ALT0;
        _CONFIG_PERIPHERAL(GPIO_B1_15, ENET_MDIO, (PORT_PS_UP_ENABLE));  // ENET0_MDIO on GPIO2-30 (alt. function 0) with pullup enabled
        #elif defined MDIO_ON_PORT3_HIGH
        _CONFIG_PERIPHERAL(GPIO_SD_B0_03, ENET0_MDC, (PORT_DSE_MID));    // ENET0_MDC on GPIO3-16 (alt. function 6)
        _CONFIG_PERIPHERAL(GPIO_SD_B0_02, ENET0_MDIO, (PORT_PS_UP_ENABLE)); // ENET0_MDIO on GPIO3-15 (alt. function 6) with pullup enabled
        #else
            #if defined iMX_RT102X
                #if defined ENET0_MDIO_ON_SD
        _CONFIG_PERIPHERAL(GPIO_SD_B0_03, ENET0_MDC, (PORT_DSE_MID));    // ENET0_MDC on GPIO3-16 (alt. function 6)
        IOMUXC_ENET_MDIO_SELECT_INPUT = IOMUXC_ENET_MDIO_SELECT_INPUT_GPIO_SD_AD_B0_02_ALT6; // special case to select the pad involved in daisy chain
                    #if defined ETHERNET_MDIO_WITH_PULLUPS
        _CONFIG_PERIPHERAL(GPIO_SD_B0_02, ENET0_MDIO, (PORT_PS_UP_ENABLE | PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-15 (alt. function 6) with pullup enabled
                    #else
        _CONFIG_PERIPHERAL(GPIO_SD_B0_02, ENET0_MDIO, (PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-15 (alt. function 6)
                    #endif
                #else
        IOMUXC_ENET_MDIO_SELECT_INPUT = IOMUXC_ENET_MDIO_SELECT_INPUT_GPIO_EMC_40_ALT4; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_EMC_41, ENET0_MDC, (PORT_DSE_MID));      // ENET0_MDC on GPIO3-09 (alt. function 4)
                    #if defined ETHERNET_MDIO_WITH_PULLUPS
        _CONFIG_PERIPHERAL(GPIO_EMC_40, ENET0_MDIO, (PORT_PS_UP_ENABLE | PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-08 (alt. function 4) with pullup enabled
                    #else
        _CONFIG_PERIPHERAL(GPIO_EMC_40, ENET0_MDIO, (PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-08 (alt. function 4)
                    #endif
                #endif
            #else
        _CONFIG_PERIPHERAL(GPIO_EMC_40, ENET0_MDC, (PORT_DSE_MID));      // ENET0_MDC on GPIO3-26 (alt. function 4)
        IOMUXC_ENET_MDIO_SELECT_INPUT = IOMUXC_ENET_MDIO_SELECT_INPUT_GPIO_EMC_41_ALT4; // special case to select the pad involved in daisy chain
                #if defined ETHERNET_MDIO_WITH_PULLUPS
        _CONFIG_PERIPHERAL(GPIO_EMC_41, ENET0_MDIO, (PORT_PS_UP_ENABLE | PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-27 (alt. function 4) with pullup enabled
                #else
        _CONFIG_PERIPHERAL(GPIO_EMC_41, ENET0_MDIO, (PORT_DSE_MID | PORT_SRE_FAST | PORT_SPEED_MID)); // ENET0_MDIO on GPIO3-27 (alt. function 4)
                #endif
            #endif
        #endif

        #if defined iMX_RT102X
        _CONFIG_PERIPHERAL(GPIO_AD_B0_14, ENET0_TDATA00, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA00 on GPIO1-14 (alt. function 0)
        _CONFIG_PERIPHERAL(GPIO_AD_B0_15, ENET0_TDATA01, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA01 on GPIO1-15 (alt. function 0)
        _CONFIG_PERIPHERAL(GPIO_AD_B0_13, ENET0_TX_EN, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TX_EN on GPIO1-13 (alt. function 0)

        IOMUXC_ENET_RX_DATA0_SELECT_INPUT = IOMUXC_ENET_RX_DATA0_SELECT_INPUT_GPIO_AD_B0_10_ALT0; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_10, ENET0_RDATA00, 0);             // ENET0_RDATA00 on GPIO1-10 (alt. function 0)
        IOMUXC_ENET_RX_DATA1_SELECT_INPUT = IOMUXC_ENET_RX_DATA1_SELECT_INPUT_GPIO_AD_B0_09_ALT0; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_09, ENET0_RDATA01, 0);             // ENET0_RDATA01 on GPIO1-09 (alt. function 0)

        IOMUXC_ENET_RX_EN_SELECT_INPUT = IOMUXC_ENET_RX_EN_SELECT_INPUT_GPIO_AD_B0_11_ALT0; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_11, ENET0_RX_EN, 0);               // ENET0_RX_EN on GPIO1-11 (alt. function 0) - ENET_CRS_DV
        IOMUXC_ENET_RX_ERR_SELECT_INPUT = IOMUXC_ENET_RX_ERR_SELECT_INPUT_GPIO_AD_B0_12_ALT0; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_AD_B0_12, ENET0_RX_ER, 0);               // ENET0_RX_ER on GPIO1-12 (alt. function 0)
        #else
        _CONFIG_PERIPHERAL(GPIO_B1_07, ENET0_TX_DATA00, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA00 on GPIO2-23 (alt. function 3)
        _CONFIG_PERIPHERAL(GPIO_B1_08, ENET0_TX_DATA01, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA01 on GPIO2-24 (alt. function 3)
        _CONFIG_PERIPHERAL(GPIO_B1_09, ENET0_TX_EN, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TX_EN on GPIO2-25 (alt. function 3)

        IOMUXC_ENET0_RXDATA_SELECT_INPUT = IOMUXC_ENET0_RXDATA_SELECT_INPUT_GPIO_B1_04_ALT3; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_B1_04, ENET0_RX_DATA00, 0);              // ENET0_RX_DATA00 on GPIO2-20 (alt. function 3)
        IOMUXC_ENET1_RXDATA_SELECT_INPUT = IOMUXC_ENET1_RXDATA_SELECT_INPUT_GPIO_B1_05_ALT3; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_B1_05, ENET0_RX_DATA01, 0);              // ENET0_RX_DATA01 on GPIO2-21 (alt. function 3)
        IOMUXC_ENET_RXEN_SELECT_INPUT = IOMUXC_ENET_RXEN_SELECT_INPUT_GPIO_B1_06_ALT3; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_B1_06, ENET0_RX_EN, 0);                  // ENET0_RX_EN on GPIO2-22 (alt. function 3) - ENET_CRS_DV
        IOMUXC_ENET_RXERR_SELECT_INPUT = IOMUXC_ENET_RXEN_SELECT_INPUT_GPIO_B1_11_ALT3; // special case to select the pad involved in daisy chain
        _CONFIG_PERIPHERAL(GPIO_B1_11, ENET0_RX_ER, 0);                  // ENET0_RX_ER on GPIO2-27 (alt. function 3)
        #endif
        #if (ETHERNET_COUNT > 1)
    }
        #endif
    #else
        #if defined MDIO_ON_PORTA                                        // {104}
            #if defined ETHERNET_MDIO_WITH_PULLUPS
    _CONFIG_PERIPHERAL(A, 7, (PA_7_MII0_MDIO | PORT_PS_UP_ENABLE));      // MII0_MDIO on PA.7 (alt. function 5) with pullup enabled
            #else
    _CONFIG_PERIPHERAL(A, 7, PA_7_MII0_MDIO);                            // MII0_MDIO on PA.7 (alt. function 5)
            #endif
    _CONFIG_PERIPHERAL(A, 8, PA_8_MII0_MDC);                             // MII0_MDC on PA.8 (alt. function 5)
        #else
            #if defined ETHERNET_MDIO_WITH_PULLUPS
    _CONFIG_PERIPHERAL(B, 0, (PB_0_MII0_MDIO | PORT_PS_UP_ENABLE));      // MII0_MDIO on PB.0 (alt. function 4/8) with pullup enabled
            #else
    _CONFIG_PERIPHERAL(B, 0, PB_0_MII0_MDIO);                            // MII0_MDIO on PB.0 (alt. function 4/8)
            #endif
    _CONFIG_PERIPHERAL(B, 1, PB_1_MII0_MDC);                             // MII0_MDC on PB.1 (alt. function 4/8)
        #endif

        #if defined JTAG_DEBUG_IN_USE_ERRATA_2541
    _CONFIG_PERIPHERAL(A, 5, (PORT_PS_DOWN_ENABLE));                     // pull the optional line down to 0V to avoid disturbing JTAG_TRST - not needed when using SWD for debugging 
        #elif !defined NO_MII_RXER                                       // {103}
    _CONFIG_PERIPHERAL(A, 5, PA_5_MII0_RXER);                            // MII0_RXER on PA.5 (alt. function 4)
        #endif
    _CONFIG_PERIPHERAL(A, 12, PA_12_MII0_RXD1);                          // MII0_RXD1 on PA.12 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 13, PA_13_MII0_RXD0);                          // MII0_RXD0 on PA.13 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 14, PA_14_RMII0_CRS_DV);                       // RMII0_CRS_DV/ MII0_RXDV on PA.14 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 15, PA_15_MII0_TXEN);                          // MII0_TXEN on PA.15 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 16, PA_16_MII0_TXD0);                          // MII0_TXD0 on PA.16 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 17, PA_17_MII0_TXD1);                          // MII0_TXD1 on PA.17 (alt. function 4/5)
        #if defined ETHERNET_RMII && defined ETHERNET_RMII_CLOCK_INPUT       // {14}
            #if defined KINETIS_K64 || defined KINETIS_K65 || defined KINETIS_K66
    _CONFIG_PERIPHERAL(E, 26,  PE_26_ENET_1588_CLKIN);                   // select the pin function for external 50MHz clock input
    SIM_SOPT2 |= (SIM_SOPT2_RMIISRC_ENET_1588_CLKIN);                    // select the ENET_1588_CLKIN as clock source (rather than EXTAL)
            #endif
        #endif
    #endif
    #if !defined ETHERNET_RMII                                           // additional signals used in MII mode
        #if defined _iMX
            #if defined iMX_RT102X
    _CONFIG_PERIPHERAL(GPIO_EMC_41, ENET0_TDATA02, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA02 on GPIO3-09 (alt. function 6)
    _CONFIG_PERIPHERAL(GPIO_EMC_40, ENET0_TDATA03, (PORT_DSE_MID | PORT_SRE_FAST)); // ENET0_TDATA03 on GPIO3-08 (alt. function 6)

    _CONFIG_PERIPHERAL(GPIO_EMC_38, ENET0_RDATA02, 0);                   // ENET0_RDATA02 on GPIO3-06 (alt. function 6)
    _CONFIG_PERIPHERAL(GPIO_EMC_37, ENET0_RDATA03, 0);                   // ENET0_RDATA03 on GPIO3-05 (alt. function 6)

    IOMUXC_ENET_TX_CLK_SELECT_INPUT = IOMUXC_ENET_TX_CLK_SELECT_INPUT_GPIO_AD_B0_08_ALT0;
    _CONFIG_PERIPHERAL(GPIO_AD_B0_08, ENET0_TX_CLK, (0));                // ENET0_TX_CLK on GPIO1-08 (alt. function 0)
    _CONFIG_PERIPHERAL(GPIO_EMC_36, ENET0_RX_CLK, (0));                  // ENET0_RX_CLK on GPIO3-04 (alt. function 6)

    _CONFIG_PERIPHERAL(GPIO_EMC_35, ENET0_COL, (0));                     // ENET0_RX_COL on GPIO3-03 (alt. function 6)
    _CONFIG_PERIPHERAL(GPIO_EMC_34, ENET0_CRS, (0));                     // ENET0_RX_COL on GPIO3-02 (alt. function 6)
            #else
    _EXCEPTION("To do!");
            #endif
        #else
    _CONFIG_PERIPHERAL(A, 9,  PA_9_MII0_RXD3);                           // MII0_RXD3 on PA.9  (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 10, PA_10_MII0_RXD2);                          // MII0_RXD2 on PA.10 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 11, PA_11_MII0_RXCLK);                         // MII0_RXCLK on PA.11 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 24, PA_24_MII0_TXD2);                          // MII0_TXD2 on PA.24 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 25, PA_25_MII0_TXCLK);                         // MII0_TXCLK on PA.25 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 26, PA_26_MII0_TXD3);                          // MII0_TXD3 on PA.26 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 27, PA_27_MII0_CRS);                           // MII0_CRS  on PA.27 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 28, PA_28_MII0_TXER);                          // MII0_TXER on PA.28 (alt. function 4/5)
    _CONFIG_PERIPHERAL(A, 29, PA_29_MII0_COL);                           // MII0_COL  on PA.29 (alt. function 4/5)
        #endif
    #endif

    #if defined FNRESETPHY
    FNRESETPHY();                                                        // optional control of the reset line to the PHY to respect any power up delays
    #endif
                                                                         // EMAC initialisation
    ptrEnet->ENET_ECR = RESET_FEC;                                       // EMAC software reset
    _WAIT_REGISTER_TRUE(ptrEnet->ENET_ECR, RESET_FEC);                   // short wait until reset is complete
    #if defined LAN_REPORT_ACTIVITY || defined LAN_TX_FPU_WORKAROUND     // {12}
    ptrEnet->ENET_EIMR = (BABR | BABT | GRA | TXF | RXF | EBERR | LC | RL | UN); // interrupt sources
    #else
    ptrEnet->ENET_EIMR = (BABR | BABT | GRA | RXF | EBERR | LC | RL | UN); // interrupt sources
    #endif
    ptrEnet->ENET_EIR = 0xffffffff;                                      // reset all pending interrupts
    #if (((ETHERNET_CONTROLLER_CLOCK/(2 * MII_MANAGEMENT_CLOCK_SPEED)) - 1) > 0x3f) // ensure that there is no overflow when a low speed can't be obtained
    ptrEnet->ENET_MSCR = (0x7e | (MSCR_HOLDTIME << 8));                  // maximum value
    #else
    ptrEnet->ENET_MSCR = ((((ETHERNET_CONTROLLER_CLOCK/(2 * MII_MANAGEMENT_CLOCK_SPEED)) - 1) << 1) | (MSCR_HOLDTIME << 8)); // generate the communication channel clock
    #endif
    ptrEnet->ENET_GAUR = 0;                                              // reset group hash entries
    ptrEnet->ENET_GALR = 0;
    ptrEnet->ENET_IALR = 0;                                              // reset individual hash entries
    ptrEnet->ENET_IAUR = 0;
    #if defined USE_IPV6
    ptrEnet->ENET_PALR = ((network[DEFAULT_NETWORK].ucOurMAC[0] << 24) | (network[DEFAULT_NETWORK].ucOurMAC[1] << 16) | (network[DEFAULT_NETWORK].ucOurMAC[2] << 8) | (network[DEFAULT_NETWORK].ucOurMAC[3])); // set the MAC address
    ptrEnet->ENET_PAUR = ((network[DEFAULT_NETWORK].ucOurMAC[4] << 24) | (network[DEFAULT_NETWORK].ucOurMAC[5] << 16));
    #else
        #if defined _GNU                                                 // work-around for GCC optimisation error which caused an unaligned long word access to a non-aligned byte array
    {
        volatile unsigned long ulPALR;
        ulPALR = pars->ucMAC[0];
        ulPALR <<= 8;
        ulPALR |= pars->ucMAC[1];
        ulPALR <<= 8;
        ulPALR |= pars->ucMAC[2];
        ulPALR <<= 8;
        ulPALR |= pars->ucMAC[3];
        ptrEnet->ENET_PALR = (unsigned long)ulPALR;
    }
        #else
    ptrEnet->ENET_PALR = ((pars->ucMAC[0] << 24) | (pars->ucMAC[1] << 16) | (pars->ucMAC[2] << 8) | (pars->ucMAC[3])); // set the MAC address
        #endif
    ptrEnet->ENET_PAUR = ((pars->ucMAC[4] << 24) | (pars->ucMAC[5] << 16));
    #endif
    #if defined _WINDOWS
    ptrEnet->ENET_EIR = 0;
    if ((ptrEnet->ENET_PALR & 0x01000000) != 0) {                        // {29}
        _EXCEPTION("Check that the own MAC address doesn't set the multicast bit!!"); // delete the file FLASH_KINETIS.ini (in simulation folder) after changing processor types to start with default parameters again
    }
    #endif
    ptrEnet->ENET_OPD = PAUSE_DURATION;
    if (usRxSize > 1514) {
        usRxSize = 1514;                                                 // maximum frame size supported (without CRC-32)
    }
    #if defined SUPPORT_VLAN
    usRxSize += 4;                                                       // increase by 4 to allow for reception of a VLAN tag
    #endif
    #if defined ETHERNET_RMII    
    ptrEnet->ENET_RCR = ((usRxSize << 16) | FEC_MII_MODE | CRCFWD | RMII_MODE); // set max frame size to be received and RMII mode - don't pass received CRC to use
    #else
    ptrEnet->ENET_RCR = ((usRxSize << 16) | FEC_MII_MODE | CRCFWD);      // set max frame size to be received and MII mode - don't pass received CRC to use
    #endif
    #if defined TEST_CRC_32_IEEE                                         // test requires CRC_32 to be copied to the input buffer
    ptrEnet->ENET_RCR &= ~(CRCFWD);
    usRxSize += 4;                                                       // ensure space for the CRC-32 in the buffer
    #else
    if ((pars->usMode & PASS_CRC32_RX) != 0) {                           // {13}
        ptrEnet->ENET_RCR &= ~(CRCFWD);                                  // enable CRC-32 value to be stored in the reception buffer
        usRxSize += 4;                                                   // ensure space for the CRC-32 in the buffer
    }
    #endif

    if ((pars->usMode & PROMISCUOUS) != 0) {
        ptrEnet->ENET_RCR |= PROM;                                       // no destination address checking
    }
    else {
    #if defined USE_IPV6
        if ((pars->usMode & CON_MULTICAST) != 0) {                       // if multicast reception is to be configured
            fnConfigureMulticastIPV6();                                  // set IPV6 multicast address
        }
    #endif
        if ((pars->usMode & NO_BROADCAST) != 0) {
            ptrEnet->ENET_RCR |= BC_REJ;                                 // reject broadcast frames
        }
    }

    if ((pars->usMode & FULL_DUPLEX) != 0) {                             // define network control
        ptrEnet->ENET_TCR = FDEN;
        if ((pars->usMode & RX_FLOW_CONTROL) != 0) {
            ptrEnet->ENET_RCR |= FCE;                                    // uses pause flow control
        }
    }
    else {
        ptrEnet->ENET_TCR = 0;
        ptrEnet->ENET_RCR |= DRT;                                        // half duplex
    }

    if ((pars->usMode & LOOPBACK) != 0) {                                // optional Loopback for testing
        ptrEnet->ENET_RCR |= LOOP;
    }

    ptrEnet->ENET_MIBC = (MIB_DISABLE | MIB_CLEAR);                      // ensure MIB is disabled while resetting and command clear to reset all counters in the maintenance block RAM
    ptrEnet->ENET_MIBC = 0;

    #if defined IP_TX_CHECKSUM_OFFLOAD && defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
    ptrEnet->_ENET_TACC = (TACC_IPCHK | TACC_PROCHK);
    ptrEnet->_ENET_TFWR = (TFWR_STRFWD);                                 // enable store and forward so that the check sum accelerator can be used on transmission
    #elif defined IP_TX_CHECKSUM_OFFLOAD
    ptrEnet->_ENET_TACC = (TACC_IPCHK);
    ptrEnet->_ENET_TFWR = (TFWR_STRFWD);                                 // enable store and forward so that the check sum accelerator can be used on transmission
    #elif defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
    ptrEnet->_ENET_TACC = (TACC_PROCHK);
    ptrEnet->_ENET_TFWR = TFWR_STRFWD;                                   // enable store and forward so that the check sum accelerator can be used on transmission
    #else
    ptrEnet->_ENET_TFWR = (TFWR_TFWR_4032);                              // don't start sending Ethernet frame until FIFO has been filled to ensure no Tx under-run is possible
    #endif
    #if defined IP_RX_CHECKSUM_OFFLOAD
    ptrEnet->_ENET_RACC = (RACC_IPDIS | RACC_PRODIS/* | RACC_LINEDIS*/); // automatically discard frames with MAC, IP header or IP protocol errors
    #else
  //ptrEnet->_ENET_RACC = (RACC_LINEDIS);                                // automatically discard reception frames with MAC errors
    #endif

    #if defined SCAN_PHY_ADD || defined POLL_PHY
    i = 0;
    do {                                                                 // check that the external PHY is responding correctly
    #endif
        ulPhyIdentifier = fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_ID1); // check that the PHY is working correctly by reading its identifier - part 1
        ulPhyIdentifier <<= 16;
        ulPhyIdentifier |= fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_ID2); // check that the PHY is working correctly by reading its identifier - part 2

        if ((ulPhyIdentifier & PHY_MASK) != (PHY_IDENTIFIER & PHY_MASK)) {
    #if defined SCAN_PHY_ADD || defined POLL_PHY
        #if defined SCAN_PHY_ADD
            PHY_ADDRESS++;
            if (PHY_ADDRESS > 31)                                        // all possible PHY addresses have been checked but no match found
        #else
            if (++i > POLL_PHY)
        #endif
            {
    #endif
    #if defined ALLOW_PHY_ERROR
        #if defined SCAN_PHY_ADD || defined POLL_PHY
                break;
        #endif
    #else
                ptrEnet->ENET_MSCR = 0;                                  // disable clock
        #if defined _iMX
            #if (ETHERNET_COUNT > 1)
                if (pars->Channel > 0) {
                    POWER_DOWN_ATOMIC(7, ENET2_CLOCK);                   // power down the second Ethernet controller
                }
                else {
                    POWER_DOWN_ATOMIC(1, ENET_CLOCK);                    // power down the first Ethernet controller
                }
            #else
                POWER_DOWN_ATOMIC(1, ENET_CLOCK);
            #endif
        #else
                POWER_DOWN_ATOMIC(2, ENET);
        #endif
                return -1;                                               // if the identifier is incorrect the phy isn't responding correctly - return error
    #endif
    #if defined SCAN_PHY_ADD || defined POLL_PHY
            }
    #endif
        }
    #if defined SCAN_PHY_ADD || defined POLL_PHY
        else {
            break;                                                       // the present PHY address has resulted in a match so use this address
        }
    } FOREVER_LOOP();
    #endif

    if ((usRxSize & 0x0f) != 0) {                                        // ensure size is suitable (divisible by 16)
        usRxSize &= ~0x0f;
        usRxSize += 0x10;
    }
    #if defined USE_MULTIPLE_BUFFERS
    ptrEnet->ENET_EMRBR = usEthernetLength[pars->Channel] = 256;         // use small multiple buffers for reception
    ucMultiBuf = ((usRxSize + 128)/256);
    ptrRxBd = ptrRxBd_start = ptrBD = (KINETIS_FEC_BD *)ETH_BD_MALLOC_ALIGN((sizeof(KINETIS_FEC_BD) * NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE * ucMultiBuf), 16); // define memory for rx buffer descriptors (aligned on 16 byte boundary)
    #else
    ptrEnet->ENET_EMRBR = usEthernetLength[pars->Channel] = usRxSize;    // set receive buffer size
    ptrRxBd = ptrRxBd_start = ptrBD = (KINETIS_FEC_BD *)ETH_BD_MALLOC_ALIGN((sizeof(KINETIS_FEC_BD) * NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE), 16); // define memory for rx buffer descriptors (aligned on 16 byte boundary)
    #endif
    ptrEnet->ENET_ERDSR = (unsigned long)ptrBD;                          // set pointer to receive descriptor ring
    #if defined USE_MULTIPLE_BUFFERS
    ptrBuffer = ETH_RX_BUF_MALLOC_ALIGN((ucMultiBuf * 256 * NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE), 16); // get rx buffer memory
    for (i = 0; i < (NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE * ucMultiBuf); i++) { // initialise each Rx buffer descriptor
        if (i == ((NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE * ucMultiBuf) - 1)) {
            ptrBD->usBDControl = (EMPTY_BUFFER | WRAP_BIT_RX);
        }
        else {
            ptrBD->usBDControl = EMPTY_BUFFER;
        }
        #if defined EMAC_ENHANCED
        ptrBD->ulBDControlEnhanced = RX_GEN_INTERRUPT;
        #endif
        ptrBD->ptrBD_Data = fnLE_ENET_add(ptrBuffer);
        ptrBuffer += 256;
        ptrBD++;
    }
    #else
    ptrBuffer = (unsigned char *)ETH_RX_BUF_MALLOC_ALIGN((usRxSize * NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE), 16); // get rx buffer memory
    for (i = 0; i < NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE; i++) {      // initialise each Rx buffer descriptor
        if (i == (NUMBER_OF_RX_BUFFERS_IN_ETHERNET_DEVICE - 1)) {
            ptrBD->usBDControl = (EMPTY_BUFFER | WRAP_BIT_RX);
        }
        else {
            ptrBD->usBDControl = EMPTY_BUFFER;
        }
        #if defined EMAC_ENHANCED
        ptrBD->ulBDControlEnhanced = RX_GEN_INTERRUPT;
        #endif
        ptrBD->ptrBD_Data = fnLE_ENET_add(ptrBuffer);
        ptrBuffer += usRxSize;
        ptrBD++;
    }
    #endif

    if ((pars->usSizeTx & 0x07) != 0) {                                  // ensure size is suitable (divisible by 8)
        pars->usSizeTx &= ~0x07;
        pars->usSizeTx += 0x08;                                          // when not, round-up to next length divisible by 8
    }
    ptrTxBd = ptrTxBd_start = ptrBD = (KINETIS_FEC_BD *)ETH_BD_MALLOC_ALIGN((sizeof(KINETIS_FEC_BD) * NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE), 16);
    ptrEnet->ENET_ETDSR = (unsigned long)ptrBD;
    ptrBuffer = (unsigned char *)ETH_TX_BUF_MALLOC_ALIGN((pars->usSizeTx * NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE), 8); // get tx buffer memory
    for (i = 0; i < NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE; i++) {
        if (i == (NUMBER_OF_TX_BUFFERS_IN_ETHERNET_DEVICE - 1)) {
            ptrBD->usBDControl = (WRAP_BIT_TX);
        }
        else {
            ptrBD->usBDControl = 0;
        }
    #if defined EMAC_ENHANCED
        #if defined IP_TX_CHECKSUM_OFFLOAD && defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
        ptrBD->ulBDControlEnhanced = (TX_GENERATE_INT | TX_INSERT_PROT_CS | TX_INSERT_IP_CS);
        #elif defined IP_TX_CHECKSUM_OFFLOAD
        ptrBD->ulBDControlEnhanced = (TX_GENERATE_INT | TX_INSERT_IP_CS);
        #elif defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD
        ptrBD->ulBDControlEnhanced = (TX_GENERATE_INT | TX_INSERT_PROT_CS);
        #else
        ptrBD->ulBDControlEnhanced = (TX_GENERATE_INT);
        #endif
        #if defined EMAC_IEEE1588
        ptrBD->ulBDControlEnhanced |= TX_ADD_TIMESTAMP;                  // enable time stamping of transmitted frames
        #endif
    #endif
        ptrBD->ptrBD_Data = fnLE_ENET_add(ptrBuffer);
        ptrBuffer += pars->usSizeTx;
        ptrBD++;
    }
    #if defined _iMX                                                     // iMX has a single interrupt vector
        #if (ETHERNET_COUNT > 1)
    if (pars->Channel > 0) {
        fnEnterInterrupt(irq_ETH1_ID, PRIORITY_EMAC2, _fec_rx_frame_isr_2); // configure and enter the Ethernet rx handling interrupt routine in the vector table
    }
    else {
        fnEnterInterrupt(irq_ETH0_ID, PRIORITY_EMAC, _fec_rx_frame_isr_1); // configure and enter the Ethernet rx handling interrupt routine in the vector table
    }
        #else
    fnEnterInterrupt(irq_ETH0_ID, PRIORITY_EMAC, _fec_rx_frame_isr);     // configure and enter the Ethernet rx handling interrupt routine in the vector table
        #endif
    #else
    fnEnterInterrupt(irq_ETH_RX_ID, PRIORITY_EMAC, _fec_rx_frame_isr);   // configure and enter the Ethernet rx handling interrupt routine in the vector table
    #if defined LAN_REPORT_ACTIVITY || defined LAN_TX_FPU_WORKAROUND     // {12}
    fnEnterInterrupt(irq_ETH_TX_ID, PRIORITY_EMAC, _fec_txf_isr);        // configure and enter the Ethernet tx handling interrupt routine in the vector table
    #endif
    fnEnterInterrupt(irq_ETH_ERR_MISC_ID, PRIORITY_EMAC, _fec_misc);     // configure and enter the Ethernet error and miscellaneous handling interrupt routine in the vector table
    #endif

    ptrEnet->ENET_ECR = ENABLE_ETHERNET_OPERATION;                       // enable FEC for operation
    #if (defined _KSZ8051RNL || defined _KSZ8081RNB) && defined ETHERNET_RMII // {102}
    usMIIData = fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_CTRL2); // get KSZ8051RNL control 2 register value
    usMIIData |= KSZ8051_CTRL2_RMII_50MHZ_CLK;                           // select 50MHz output clock mode (rather than the default 25MHz)
    fnMIIwrite((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_CTRL2, usMIIData); // write back new value
    #endif
    #if !defined _PHY_KSZ8863                                            // {43}
    if ((pars->usMode & (LAN_10M | LAN_100M)) == 0) {                    // when in auto-negotiation mode
        usMIIData = fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_ANAR); // get auto neg. mode supported by PHY
        if ((pars->usMode & RX_FLOW_CONTROL) != 0) {
            usMIIData |= PHY_R4_FC;                                      // set flow control
        }
        fnMIIwrite((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_ANAR, usMIIData); // write PHY_REG_ANAR to configure PHY Link abilities advertisement
    }
    #endif
    #if defined PHY_INTERRUPT
    fnMIIwrite((PHY_ADDRESS + PHY_INTERFACE), KS8041_INTERRUPT_CONTROL_STATUS_REGISTER, // enable various PHY interrupts
//             PHY_REMOTE_FAULT_INT_ENABLE
//             PHY_LINK_PARTNER_ACK_INT_ENABLE
//             PHY_PARALLEL_DETECT_FAULT_INT_ENABLE
//             PHY_PAGE_RECEIVED_INT_ENABLE
//             PHY_RECEIVE_ERROR_INT_ENABLE
//             PHY_JABBER_INT_ENABLE
              (PHY_LINK_UP_INT_ENABLE | PHY_LINK_DOWN_INT_ENABLE)
               );
    #endif
    usMIIData = fnMIIread((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_CR);    // read PHY Control Register 0
    usMIIData &= ~(PHY_R0_ISOLATE);                                      // remove electrical isolation
    if ((pars->usMode & (LAN_10M | LAN_100M)) == 0) {
        usMIIData |= (PHY_R0_ANE | PHY_R0_RAN);                          // configure and start auto-negotiation
    }
    else {
        usMIIData &= ~(PHY_R0_ANE | PHY_R0_RAN);                         // ensure that the PHY doesn't perform auto-negotiation
        if ((pars->usMode & FULL_DUPLEX) != 0) {
            usMIIData |= PHY_R0_DPLX;                                    // set full duplex operation
        }
        else {
            usMIIData &= ~PHY_R0_DPLX;                                   // set half-duplex operation
        }

        if ((pars->usMode & LAN_100M) != 0) {
            usMIIData |= PHY_R0_DR;                                      // set manual 100Mb
        }
        else {
            usMIIData &= ~PHY_R0_DR;                                     // set manual 10Mb
    #if defined ETHERNET_RMII
            ptrEnet->ENET_RCR |= RMII_10T;
    #endif
        }
    }
    fnMIIwrite((PHY_ADDRESS + PHY_INTERFACE), PHY_REG_CR, usMIIData);    // command initial operating mode of PHY
    #if defined PHY_ADDRESS_2                                            // {43}
    fnMIIwrite((PHY_ADDRESS_2 + PHY_INTERFACE), PHY_REG_CR, usMIIData);  // command initial operating mode of PHY (second port)
    #endif
    #if defined PHY_INTERRUPT                                            // enable PHY interrupt
    fnMIIwrite((PHY_ADDRESS + PHY_INTERFACE), KS8041_INTERRUPT_CONTROL_STATUS_REGISTER, // enable various PHY interrupts
//             PHY_REMOTE_FAULT_INT_ENABLE
//             PHY_LINK_PARTNER_ACK_INT_ENABLE
//             PHY_PARALLEL_DETECT_FAULT_INT_ENABLE
//             PHY_PAGE_RECEIVED_INT_ENABLE
//             PHY_RECEIVE_ERROR_INT_ENABLE
//             PHY_JABBER_INT_ENABLE
              (PHY_LINK_UP_INT_ENABLE | PHY_LINK_DOWN_INT_ENABLE)
               );
    interrupt_setup.int_type       = PORT_INTERRUPT;                     // identifier to configure port interrupt
    interrupt_setup.int_handler    = fnPhyInterrupt;                     // handling function
    interrupt_setup.int_priority   = PRIORITY_PHY_PORT_INT;              // interrupt priority level
    interrupt_setup.int_port       = PHY_INTERRUPT_PORT;                 // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = PHY_INTERRUPT;                      // the IRQ input connected
    interrupt_setup.int_port_sense = (IRQ_LOW_LEVEL | PULLUP_ON);        // interrupt is to be low level sensitive
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
    #endif
    ptrEnet->ENET_RDAR = 0;                                              // enable RX BD polling
    #if defined STOP_MII_CLOCK
    ptrEnet->ENET_MSCR = 0;                                              // disable clock to save power and to avoid interfence when not used
    #endif
    return 0;                                                            // configuration successful
}


#if defined _WINDOWS
    #if defined USE_IPV6 || defined USE_IGMP
static int fnIsHashMulticast(unsigned char *ucData)
{
    unsigned long ulCRC32 = 0xffffffff;
    int i, bit;
    unsigned char ucByte;
    if (!(*ucData & 0x01)) {                                             // if not multicast, don't check
        return 0;
    }
	for (i = 0; i < MAC_LENGTH; i++) {
        ucByte = *ucData++;;
        for (bit = 0; bit < 8; bit++) {
            if ((ulCRC32 ^ ucByte) & 1) {
                ulCRC32 = (ulCRC32 >> 1) ^ CRC_POLY;
            }
            else {
                ulCRC32 >>= 1;
            }
            ucByte >>= 1;
        }
	}
    ulCRC32 >>= 26;                                                      // the upper 6 bits of the CRC32 detemine the hash entry location

    if (ulCRC32 < 32) {
        if ((GALR & (0x00000001 << ulCRC32)) != 0) {
            return 1;                                                    // multicast hash match
        }
    }
    else {
        ulCRC32 -= 32;
        if ((GAUR & (0x00000001 << ulCRC32)) != 0) {
            return 1;                                                    // multicast hash match
        }
    }
    return 0;                                                            // not a match
}
    #endif

    // Check whether a unicast address matches
    //
    static int fnIsOurMAC(unsigned char *ucData)
    {
        if ((*ucData == (unsigned char)(PALR >> 24)) && (*(ucData + 1) == (unsigned char)(PALR >> 16)) && (*(ucData + 2) == (unsigned char)(PALR >> 8)) && (*(ucData + 3) == (unsigned char)(PALR)) &&
            (*(ucData + 4) == (unsigned char)(PAUR >> 24)) && (*(ucData + 5) == (unsigned char)(PAUR >> 16))) {
            return 1;                                                    // MAC address matches the programmed unicast MAC address in the Ethernet controller
        }
        return 0;                                                        // not MAC address match
    }

    // This routine is supplied to aid the simulator. It checks whether Ethereal frames should be accepted based
    // on the programmed MAC address and the Ethernet operating mode
    //
    extern int fnCheckEthernetMode(unsigned char *ucData, unsigned short usLen)
    {
        // This function checks to see whether we can receive the simulated frame and can block it if wanted
        //
        if (usLen < ((2 * MAC_LENGTH) + 2)) {
            return 0;                                                    // too short to be a valid Ethernet frame
        }
    #if !defined _DEVELOP_SIM
        if (fnIsOurMAC(&ucData[MAC_LENGTH]) != 0) {                      // check source MAC address
            return 0;                                                    // we never accept echos of our own transmissions - check before broadcast
        }
    #endif
        if (uMemcmp(ucData, cucBroadcast, MAC_LENGTH) == 0) {
            return 1;                                                    // broadcast
        }
        if ((RCR & PROM) != 0) {
            return 1;                                                    // in promiscuous mode we accept everything
        }
        if (fnIsOurMAC(&ucData[0]) != 0) {                               // check destination MAC address
            return 1;                                                    // we accept our mac address
        }
    #if defined USE_IPV6 || defined USE_IGMP
        if (fnIsHashMulticast(&ucData[0]) != 0) {                        // check destination MAC address
            return 1;                                                    // multicast hash match
        }
    #endif
        return 0;                                                        // otherwise ignore
    }
#endif

// This can be called by the Ethernet driver to wait a short time for the transmit buffer to become free
//
extern int fnWaitTxFree(void)
{
    volatile int iWait = 5000;                                           // wait until the transmitter is free
    while ((ptrTxBd->usBDControl & READY_TX) != 0) {
        if (--iWait == 0) {
            return 1;                                                    // we have waited long enough - give up...
        }
    }
    return 0;
}

extern void fnPutInBuffer(unsigned char *ptrOut, unsigned char *ptrIn, QUEUE_TRANSFER nr_of_bytes)
{
    uMemcpy(ptrOut, ptrIn, nr_of_bytes);                                 // the Ethernet output buffer is linear so can be copied to using uMemcpy()
}

// This can be called by the ethernet driver to find out how many receive buffers the Ethernet Controller has
//
extern int fnGetQuantityRxBuf(void)
{
    return 1;
}

// This can be called by the ethernet driver to get the address of the corresponding Ethernet receive buffer
//
extern unsigned char *fnGetRxBufferAdd(int iBufNr)
{
    return 0;
}

// This can be called by the ethernet driver to get the address of the corresponding Ethernet transmit buffer
//
extern unsigned char *fnGetTxBufferAdd(int iBufNr)
{
    return fnLE_ENET_add(ptrTxBd->ptrBD_Data);
}

// This can be called by the ethernet driver to free an Ethernet receive buffer after it has been used
//
extern void fnFreeEthernetBuffer(int iBufNr)
{
    #if defined USE_MULTIPLE_BUFFERS
    if (iBufNr >= 0) {
        return;                                                          // when using multi-small buffers all processing is internal
    }
    #endif
    ptrRxBd->usBDLength = 0;                                             // the buffer has been completely used so free it for use by the Ethernet controller
    ptrRxBd->usBDControl |= EMPTY_BUFFER;                                // free the buffer
    #if defined _WINDOWS
    ptrRxBd->usBDControl &= ~(LAST_IN_FRAME_RX | TRUNCATED_FRAME | OVERRUN_FRAME);
    #endif
    if ((ptrRxBd->usBDControl & WRAP_BIT_RX) != 0) {                     // handle wrap
        ptrRxBd = (KINETIS_FEC_BD *)ERDSR;                               // set to first BD
    }
    else {
        ptrRxBd++;                                                       // increment to next buffer
    }
    RDAR = 0;                                                            // re-enable buffer operation
}

// Data has been prepared, now send it over the ETHERNET interface
//
extern QUEUE_TRANSFER fnStartEthTx(QUEUE_TRANSFER DataLen, unsigned char *ptr_put)
{
    #if defined PHY_TAIL_TAGGING                                         // {44}
    if (iTailTagging != 0) {
      //while (DataLen < 60) {                                           // when tail tagging we need to fill out short frames
      //    *ptr_put++ = 0x00;                                           // pad with zeros if smaller than 60 [chip must send at least 60]
      //    DataLen++;
      //}
        #if !defined _WINDOWS
        ptrTxBd->usBDLength = fnLE_ENET_word(DataLen + 1);               // add tag to tail to specify which port to transmit over
        #endif
      //*ptr_put = ucTailTagPort;                                        // specify port for the frame to be output to
    }
    else {
        ptrTxBd->usBDLength = fnLE_ENET_word(DataLen);                   // mark length of data to send (Kinetis automatically zero-pads transmission to minimum length)
    }
    #else
    ptrTxBd->usBDLength = fnLE_ENET_word(DataLen);                       // mark length of data to send (Kinetis automatically zero-pads transmission to minimum length)
    #endif
    #if defined EMAC_ENHANCED && defined IP_TX_CHECKSUM_OFFLOAD && defined IP_TX_PAYLOAD_CHECKSUM_OFFLOAD && defined IPV4_SUPPORT_TX_FRAGMENTATION
    if (iFragmentTx == 0) {
        ptrTxBd->ulBDControlEnhanced |= (TX_INSERT_PROT_CS);             // ensure that payload checkum offloading is enabled for this frame
    }
    else {
        ptrTxBd->ulBDControlEnhanced &= ~(TX_INSERT_PROT_CS);            // disable payload checkum offloading for this frame
    }
    #endif
    ptrTxBd->usBDControl |= (READY_TX | LAST_IN_FRAME_TX | TX_CRC);      // start transmission
    #if defined EMAC_ENHANCED
        #if defined LAN_REPORT_ACTIVITY || defined LAN_TX_FPU_WORKAROUND // {12}
    ptrTxBd->ulBDU = TX_GENERATE_INT;
        #else
    ptrTxBd->ulBDU = 0;
        #endif
    #endif
    TDAR = TDAR_TDAR;                                                    // enable TX BD operation to send the new content

    #if defined _WINDOWS
    fnSimulateEthTx(DataLen, fnGetSimTxBufferAdd());                     // send the Ethernet frame to the NIC
        #if defined PSEUDO_BROADCAST_LOOPBACK                            // broadcasts are looped back to any shared additional networks on the Ethernet interface
    if (uMemcmp(cucBroadcast, (ptr_put - DataLen), IPV4_LENGTH) == 0)  { // broadcast Ethernet frame
        fnSimulateEthernetIn((ptr_put - DataLen), DataLen, 1);
    }
        #endif
        #if defined PSEUDO_LOOPBACK                                      // if we detect an IP frame being sent to our own address we loop it back to the input
    if (((*(ptr_put - DataLen + 12)) == 0x08) && ((uMemcmp(&network[DEFAULT_NETWORK].ucOurIP[0], (ptr_put - DataLen + 26), IPV4_LENGTH)) == 0)) {
        fnSimulateEthernetIn((ptr_put - DataLen), DataLen, 1);
    }
        #endif
    fec_txf_isr();                                                       // simulate end of transmission - tx interrupt
    ptrTxBd->usBDControl &= ~READY_TX;                                   // free buffer for next transmission
    #endif

    if ((ptrTxBd->usBDControl & WRAP_BIT_TX) != 0) {                     // prepare local pointer for next write
        ptrTxBd = (KINETIS_FEC_BD *)ETDSR;
    }
    else {
        ptrTxBd++;
    }
    return (DataLen);
}


static unsigned long ulMIB[MIB_RAM_SIZE/sizeof(unsigned long)] = {0};    // local backup of last counter value displayed

#define TX_RIGHT_ALIGNED 27
#define RX_RIGHT_ALIGNED 12

static void fnDisplayMIBcounter(unsigned char ucOffset, CHAR cLine[TX_RIGHT_ALIGNED + 1], int iRx) // {106}
{
    volatile unsigned long *ptrMIBold = ulMIB;
    volatile unsigned long *ptrMIBnew = MIBRAM_ADD;
    int iShift;
    unsigned long ulLastValue = *(volatile unsigned long *)(((volatile unsigned char *)ptrMIBold) + ucOffset);
    unsigned long ulNewValue = *(volatile unsigned long *)(((volatile unsigned char *)ptrMIBnew) + ucOffset);
    CHAR *Value;
    int iOffset = (TX_RIGHT_ALIGNED - 10);
    int iRxOffset;
    if (iRx != 0) {                                                      // display rx counter
        iRxOffset = (TX_RIGHT_ALIGNED - 11);
    }
    else {                                                               // display tx counter
        iRxOffset = 0;
    }
    Value = &cLine[iOffset];
    Value = fnBufferDec(ulNewValue, 0, Value);                           // generate decimal string
    iShift = (&cLine[TX_RIGHT_ALIGNED] - Value);
    uReverseMemcpy((&cLine[iOffset] + iShift), &cLine[iOffset], (Value - &cLine[iOffset]));
    uMemset(&cLine[iOffset], ' ', iShift);
    if (ulLastValue != ulNewValue) {                                     // display changed values in red
        if (iRx == 0) {
            CHAR ucBackup = cLine[iOffset];
            cLine[iOffset] = 0;
            fnDebugMsg(cLine);
            cLine[iOffset] = ucBackup;
        }
        fnDebugStyle(OUTPUT_COLOR_RED);
        if (iRx != 0) {
            fnDebugMsg(&cLine[iRxOffset]);
        }
        else {
            fnDebugMsg(&cLine[iOffset]);
        }
        fnDebugStyle(OUTPUT_STYLE_OFF);
    }
    else {
        fnDebugMsg(&cLine[iRxOffset]);                                   // display line with no color
    }
}

static void fnENET_Display(const CHAR *ptrString, unsigned char ucTX_Offset, unsigned char ucRX_Offset) // {106}
{
    CHAR cLine[TX_RIGHT_ALIGNED + 1];
    CHAR *ptrEnd = uStrcpy(cLine, ptrString);
    while (ptrEnd < &cLine[TX_RIGHT_ALIGNED]) {
        *ptrEnd++ = ' ';
    }
    *ptrEnd = 0;
    if (ucTX_Offset != 0xff) {                                           // if tx counter is to be displayed
        fnDisplayMIBcounter(ucTX_Offset, cLine, 0);
    } 
    else {
        fnDebugMsg(cLine);
    }
    if (ucRX_Offset != 0xff) {                                           // if rx counter is to be displayed
        ptrEnd = &cLine[TX_RIGHT_ALIGNED - 11];
        while (ptrEnd < &cLine[TX_RIGHT_ALIGNED]) {
            *ptrEnd++ = ' ';
        }
        fnDisplayMIBcounter(ucRX_Offset, cLine, 1);
    }
    fnDebugMsg("\r\n");
}

extern void fnShowEthernet(int iInterface)                               // {106}
{
    int i;
    volatile unsigned long *ptrMIB = MIBRAM_ADD;
    fnDebugMsg("           Ethernet counters\r\n");
    fnDebugMsg("=======================================\r\n");
    fnDebugMsg("                         Tx         Rx\r\n");
    fnENET_Display("Packets", 0x04, 0x84);
    fnENET_Display("Broadcast", 0x08, 0x88);
    fnENET_Display("Multicast", 0x0c, 0x8c);
    fnENET_Display("CRC-Align", 0x10, 0x90);
    fnENET_Display("<64 bytes", 0x14, 0x94);
    fnENET_Display(">MAX_FL", 0x18, 0x98);
    fnENET_Display("<64 CRC", 0x1c, 0x9c);
    fnENET_Display(">MAX_FL CRC", 0x20, 0xa0);
    fnENET_Display("64 bytes", 0x28, 0xa8);
    fnENET_Display("65-127 bytes", 0x2c, 0xac);
    fnENET_Display("128-255 bytes", 0x30, 0xb0);
    fnENET_Display("256-511 bytes", 0x34, 0xb4);
    fnENET_Display("512-1023 bytes", 0x38, 0xb8);
    fnENET_Display("1024-2047 bytes", 0x3c, 0xbc);
    fnENET_Display(">2048 bytes", 0x40, 0xc0);
    fnENET_Display("Octets", 0x44, 0xc4);
    fnENET_Display("Dropped", 0x48, 0xc8);
    fnENET_Display("Frames OK", 0x4c, 0xcc);
    fnENET_Display("FIFO under/over", 0x64, 0xd8);
    fnENET_Display("Pause frames", 0x70, 0xdc);
    fnENET_Display("Good octets", 0x74, 0xe0);
    fnENET_Display("Collisions", 0x24, 0xff);
    fnENET_Display("Single collision", 0x50, 0xff);
    fnENET_Display("Multi-collision", 0x54, 0xff);
    fnENET_Display("Deferral delay", 0x58, 0xff);
    fnENET_Display("Late collision", 0x5c, 0xff);
    fnENET_Display("Ex. collisions", 0x60, 0xff);
    fnENET_Display("Carrier sense", 0x68, 0xff);
    fnENET_Display("SQE error", 0x6c, 0xff);
    fnENET_Display("CRC error", 0xff, 0xd0);
    fnENET_Display("Alignment error", 0xff, 0xd4);
    for (i = 0; i < (MIB_RAM_SIZE/sizeof(unsigned long)); i++) {
        if ((ptrMIB <= IEEE_T_OCTETS_OK_ADD) || (ptrMIB >= RMON_R_PACKETS_ADD)) {
            ulMIB[i] = *ptrMIB;
        }
        ptrMIB++;
    }
#if defined _WINDOWS
    ptrMIB = MIBRAM_ADD;
    for (i = 0; i < (MIB_RAM_SIZE / sizeof(unsigned long)); i++) {
        *ptrMIB++ = (i * 4);
    }
#endif
}
