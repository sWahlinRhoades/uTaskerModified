/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      telnet.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2018
    *********************************************************************
    04.02.2007 SentLength initialised to zero since it is always checked {1}
    21.05.2009 SentLength used as echo result to avoid particular lengths being interpreted incorrectly by TCP {2}
    03.02.2010 Protect TELNET mode check when called by a non-TELNET socket {3}
    24.03.2011 Use TCP_WINDOW_UPDATE event to kick off stalled TCP flow  {4}
    24.03.2011 Interpret a regeneration with zero data length as a probe request {5}
    26.03.2011 Pass the destination port to the user listener on connection request {6}
    26.03.2011 Automatically accept a repeated connection request without disturbing the application {7}
    17.12.2012 Allow TELNET server to work on multiple networks          {8}
    01.03.2013 Retain RAW socket mode over socket resets                 {9}
    04.06.2013 Added TELNET_MALLOC() default                             {10}
    06.08.2013 Add IMMEDIATE_MEMORY_ALLOCATION option                    {11}
    31.10.2104 Add TELNET client                                         {12}
    16.02.2017 Add RFC 2217 (Telnet com port control option) mode        {13}

*/



#include "config.h"

#if defined USE_TELNET || defined USE_TELNET_CLIENT


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined TELNET_MALLOC                                                // {10}
    #define TELNET_MALLOC(x)      uMalloc((MAX_MALLOC)(x))
#endif

#define OWN_TASK   0


#define TELNET_STATE_FREE      0
#define TELNET_STATE_RESERVED  1
#define TELNET_STATE_CONNECTED 2

#if !defined RFC2217_INSTANCES
    #define RFC2217_INSTANCES     1
#endif

#define RFC2217_STATE_FREE    0
#define RFC2217_STATE_OFF     1
#define RFC2217_STATE_IDLE    2
#define RFC2217_STATE_ACTIVE  3

/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static int fnTELNETListener(USOCKET Socket, unsigned char ucEvent, unsigned char *ucIp_Data, unsigned short usPortLen);
static QUEUE_TRANSFER fnSendNeg(TELNET *TELNET_session, unsigned char ucAnswer, unsigned char ucOption);
static void fnResetTelnetSession(TELNET *TELNET_session);

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined USE_TELNET
    static const CHAR cDeleteInput[] = {DELETE_KEY, ' ', DELETE_KEY};
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static TELNET *ptrTELNET = 0;

#if defined TELNET_RFC2217_SUPPORT
    static RFC2217_INSTANCE rfc2217_instance[RFC2217_INSTANCES] = {{0}};
#endif


/* =================================================================== */
/*                               code                                  */
/* =================================================================== */


// Return a pointer to a TELNET session based on its socket reference
//
static TELNET *fnGetTelnetSession(USOCKET Socket)
{
    TELNET *TELNET_session = ptrTELNET;
    int i;

    if (TELNET_session == 0) {
        return 0;
    }
    for (i = 0; i < NO_OF_TELNET_SESSIONS; i++) {                        // for each possible Telnet socket
        if (Socket < 0) {                                                // {8} if searching for a free socket
            if (i >= (NO_OF_TELNET_SESSIONS - TELNET_CLIENT_COUNT)) {    // {12} don't include client sessions
                break;
            }
            if (TELNET_session->Telnet_socket == Socket) {               // free socket
                return TELNET_session;                                   // return pointer to the free session
            }
        }
        else if (_TCP_SOCKET_MASK(TELNET_session->Telnet_socket) == _TCP_SOCKET_MASK(Socket)) { // {8}
            return TELNET_session;
        }
        TELNET_session++;
    }
    return 0;                                                            // the socket is not a telnet socket
}

static void fnInitialiseTelnetSessions(void)
{
    if (ptrTELNET == 0) {                                                // when called, the user defined number of TELNET sockets will be created if they do not yet exist
        TELNET *TELNET_session;
        int i;
        ptrTELNET = TELNET_session = (TELNET *)TELNET_MALLOC(NO_OF_TELNET_SESSIONS * sizeof(TELNET)); // get memory for socket administration
        for (i = 0; i < NO_OF_TELNET_SESSIONS; i++) {                    // initialise sockets for each possible session
            TELNET_session->Telnet_socket = -1;                          // mark unused
            TELNET_session++;
        }
    }
}

#if defined USE_TELNET
    #if defined IMMEDIATE_MEMORY_ALLOCATION                              // {11}
extern USOCKET fnStartTelnet(unsigned short usTelnetPortNumber, unsigned short usIdleTimeout, unsigned short usMaxWindow, unsigned short usTxBufSize, UTASK_TASK wakeOnAck, int (*listener)(USOCKET, unsigned char, unsigned char *, unsigned short))
    #else
extern USOCKET fnStartTelnet(unsigned short usTelnetPortNumber, unsigned short usIdleTimeout, unsigned short usMaxWindow, UTASK_TASK wakeOnAck, int (*listener)(USOCKET, unsigned char, unsigned char *, unsigned short))
    #endif
{
    TELNET *TELNET_session;
    fnInitialiseTelnetSessions();                                        // initialise Telnet sessions if needed
    TELNET_session = fnGetTelnetSession(-1);                             // get a free telnet socket
    if (TELNET_session == 0) {
        return -1;                                                       // no free socket found
    }

    TELNET_session->Telnet_socket = fnGetTCP_Socket(TOS_MINIMISE_DELAY, usIdleTimeout, fnTELNETListener);
    #if defined IMMEDIATE_MEMORY_ALLOCATION                              // {11}
    if (usTxBufSize == 0) {
        usTxBufSize = TCP_BUFFER;
    }
    fnDefineTCPBufferSize(TELNET_session->Telnet_socket, usTxBufSize);   // dimension the buffered socket's output buffer and allocate its memory
    #endif
    TELNET_session->fnApp = listener;
    TELNET_session->usTelnetPortNumber = usTelnetPortNumber;
    TELNET_session->usMaxWindow = usMaxWindow;
    TELNET_session->wakeOnAck = wakeOnAck;
    fnTCP_Listen(TELNET_session->Telnet_socket, usTelnetPortNumber, usMaxWindow);
    return TELNET_session->Telnet_socket;
}


extern int fnTelnet(USOCKET Telnet_socket, int iCommand)
{
    TELNET *TELNET_session = fnGetTelnetSession(Telnet_socket);

    if (TELNET_session == 0) {
        return APP_ACCEPT;                                               // not found, simply return accepted by application
    }

    switch (iCommand) {
    case GOTO_ECHO_MODE:
        fnSendNeg(TELNET_session, TELNET_WILL, TELNET_ECHO);
        TELNET_session->usTelnetMode |= TELNET_WILL_ECHO;                // mark that we want to echo
        return APP_SENT_DATA;

    case LEAVE_ECHO_MODE:
        fnSendNeg(TELNET_session, TELNET_WONT, TELNET_ECHO);
        TELNET_session->usTelnetMode |= TELNET_WONT_ECHO;                // mark that we wont to echo
        return APP_SENT_DATA;

    case PASSWORD_ENTRY:
        TELNET_session->usTelnetMode |= TELNET_PASSWORD_ENTRY_MODE;
        break;

    case CLEAR_TEXT_ENTRY:
        TELNET_session->usTelnetMode &= ~TELNET_PASSWORD_ENTRY_MODE;
        break;

    case TELNET_ASCII_MODE:
        TELNET_session->usTelnetMode &= ~(TELNET_RAW_SOCKET | TELNET_BINARY_MODE);
        break;

    case TELNET_RAW_MODE:
        TELNET_session->usTelnetMode |= TELNET_RAW_SOCKET;
        break;

    case TELNET_RAW_RX_IAC_ON:
        TELNET_session->usTelnetMode |= TELNET_SEARCH_RX_IAC;
        break;

    case TELNET_RAW_RX_IAC_OFF:
        TELNET_session->usTelnetMode &= ~TELNET_SEARCH_RX_IAC;
        break;

    case TELNET_RAW_TX_IAC_ON:
        TELNET_session->usTelnetMode |= TELNET_STUFF_TX_IAC;
        break;

    case TELNET_RAW_TX_IAC_OFF:
        TELNET_session->usTelnetMode &= ~TELNET_STUFF_TX_IAC;
        break;

    case TELNET_RESET_MODE:
        TELNET_session->usTelnetMode = 0;
        break;

    #if defined TELNET_RFC2217_SUPPORT                                   // {13}
    case TELNET_RFC2217_ON:
        TELNET_session->usTelnetMode |= TELNET_RFC2217_ACTIVE;
        break;

    case TELNET_RFC2217_OFF:
        TELNET_session->usTelnetMode &= ~TELNET_RFC2217_ACTIVE;
        break;
    #endif
    }
    return APP_ACCEPT;
}


extern void fnStopTelnet(USOCKET TelnetSocket)
{
    TELNET *TELNET_session = fnGetTelnetSession(TelnetSocket);

    if (TELNET_session == 0) {
        return;
    }

    fnReleaseTCP_Socket(TelnetSocket);                                   // we close the sockets so that Telnet socket is effectively dead.
    fnResetTelnetSession(TELNET_session);
    TELNET_session->Telnet_socket = -1;
}


extern int fnCheckTelnetBinaryTx(USOCKET Socket)
{
    TELNET *TELNET_session = fnGetTelnetSession(Socket);
    if (TELNET_session == 0) {                                           // {3} not a TELNET socket
        return 0;
    }
    if (TELNET_session->usTelnetMode & (TELNET_STUFF_TX_IAC | TELNET_BINARY_MODE)) {
        return 1;
    }
    return 0;
}
#endif                                                                   // end USE_TELNET


static void fnResetTelnetSession(TELNET *TELNET_session)
{
    TELNET_session->ucState = TELNET_STATE_FREE;
    TELNET_session->usTelnetMode &= TELNET_RAW_SOCKET;                   // {9} reset the mode, but retain RAW socket mode if present
}

static QUEUE_TRANSFER fnSendNeg(TELNET *TELNET_session, unsigned char ucAnswer, unsigned char ucOption)
{
    unsigned char ucNegotiation[3];
    ucNegotiation[0] = TELNET_IAC;
    ucNegotiation[1] = ucAnswer;
    ucNegotiation[2] = ucOption;

    return (fnSendBufTCP(TELNET_session->Telnet_socket, (unsigned char *)ucNegotiation, sizeof(ucNegotiation), (TCP_BUF_SEND | TCP_CONTENT_NEGOTIATION)));
}

#if defined TELNET_RFC2217_SUPPORT                                       // {13}
static int fnRFC2217_listener(USOCKET Socket, unsigned char ucEvent, unsigned char *ptrData, unsigned short usLength)
{
    TELNET *TELNET_session = fnGetTelnetSession(Socket);
    switch (ucEvent) {
    case TCP_EVENT_CONNECTED:                                            // TCP connection has been established
        TELNET_session->ptrRFC2217_Instance->iInstanceState = RFC2217_STATE_ACTIVE;
        TELNET_session->ptrRFC2217_Instance->ucLineStateMask = 0;
        TELNET_session->ptrRFC2217_Instance->ucModemStateMast = 0;
        if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
            TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_CONNECTION_OPENED, 0); // request the application to change the interface settings back to their original state
        }
        break;
    case TCP_EVENT_DATA:
        if (TELNET_session->ptrRFC2217_Instance->iInstanceState == RFC2217_STATE_ACTIVE) {
            if (TELNET_session->ptrRFC2217_Instance->uart_settings.uartID != NO_ID_ALLOCATED) {
                fnWrite(TELNET_session->ptrRFC2217_Instance->uart_settings.uartID, ptrData, usLength); // pass the received data to the serial interface
            }
        }
        break;
    case TCP_EVENT_CLOSE:
    case TCP_EVENT_CLOSED:
        TELNET_session->ptrRFC2217_Instance->iInstanceState = RFC2217_STATE_IDLE;
        if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
            if (uMemcmp(&TELNET_session->ptrRFC2217_Instance->uart_settings_original, &TELNET_session->ptrRFC2217_Instance->uart_settings, sizeof(&TELNET_session->ptrRFC2217_Instance->uart_settings_original)) != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_CONNECTION_CLOSED, &TELNET_session->ptrRFC2217_Instance->uart_settings_original); // request the application to change the interface settings back to their original state
            }
        }
        break;
    }
    return APP_ACCEPT;                                                   // default is to accept a connection
}

extern USOCKET fnTelnetRF2217(RFC2217_SESSION_CONFIG *ptrConfig)
{
    int iInstanceRef;
    if (ptrConfig != 0) {                                                // is a listener is to be created/enabled
        for (iInstanceRef = 0; iInstanceRef < RFC2217_INSTANCES; iInstanceRef++) {
            if (rfc2217_instance[iInstanceRef].iInstanceState == RFC2217_STATE_FREE) { // free instance found
                break;
            }
        }
        if (iInstanceRef >= RFC2217_INSTANCES) {
            return -1;                                                   // no RFC2217 instance free
        }
    #if defined IMMEDIATE_MEMORY_ALLOCATION                              // extra parameter for tx buffer size to immediately allocate (uses default size)
        rfc2217_instance[iInstanceRef].Telnet_RFC2217_socket = fnStartTelnet(ptrConfig->usPortNumber, ptrConfig->usIdleTimeout, 0, 0, 0, fnRFC2217_listener); // 2 minute inactivity timeout
    #else
        rfc2217_instance[iInstanceRef].Telnet_RFC2217_socket = fnStartTelnet(ptrConfig->usPortNumber, ptrConfig->usIdleTimeout, 0, 0, fnRFC2217_listener); // 2 minute inactivity timeout
    #endif
        if (rfc2217_instance[iInstanceRef].Telnet_RFC2217_socket < 0) {
            return -1;
        }
        else {
            TELNET *TELNET_session = fnGetTelnetSession(rfc2217_instance[iInstanceRef].Telnet_RFC2217_socket);
            TELNET_session->usTelnetMode |= TELNET_RFC2217_ACTIVE;
            TELNET_session->ptrRFC2217_Instance = &rfc2217_instance[iInstanceRef];
            // create an instance with UART variables so that we can respond to requets for UART information and change settings, as well as returning the original settings when the session is terminated
            // when connected we autonomously bridge all UART/VCOM data to/from the Telnet TCP connection
            rfc2217_instance[iInstanceRef].iInstanceState = RFC2217_STATE_IDLE;
            uMemcpy(&rfc2217_instance[iInstanceRef].uart_settings_original, &(ptrConfig->uart_settings), sizeof(rfc2217_instance[iInstanceRef].uart_settings)); // backup the original settings so that they can be returned if changed
            uMemcpy(&rfc2217_instance[iInstanceRef].uart_settings, &(ptrConfig->uart_settings), sizeof(rfc2217_instance[iInstanceRef].uart_settings));
        }
    }
    else {
        fnTelnet(rfc2217_instance[0].Telnet_RFC2217_socket, TELNET_RFC2217_OFF);
        rfc2217_instance[0].iInstanceState = RFC2217_STATE_OFF;
    }
    return rfc2217_instance[0].Telnet_RFC2217_socket;
}

static int fnSetRFC2217_portConfig(TELNET *TELNET_session, unsigned char *ptrData)
{
    unsigned char ucNegotiation[9];
    unsigned char ucType = *ptrData++;
    unsigned long ulNewLong;
    int iLength = 1;
    switch (ucType) {
    case SET_BAUDRATE_SERVER:
        ulNewLong = *ptrData++;                                          // get the speed in network order
        ulNewLong <<= 8;
        ulNewLong |= *ptrData++;
        ulNewLong <<= 8;
        ulNewLong |= *ptrData++;
        ulNewLong <<= 8;
        ulNewLong |= *ptrData++;
        if ((ulNewLong != 0) && (TELNET_session->ptrRFC2217_Instance->uart_settings.ulBaudRate != ulNewLong)) { // request for baud rate modification
            if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_SETTINGS_CHANGED, &TELNET_session->ptrRFC2217_Instance->uart_settings); // request the application to change the interface settings - if it doesn't accept values it can overwrite them
            }
        }
        ucNegotiation[3] = (unsigned char)(TELNET_session->ptrRFC2217_Instance->uart_settings.ulBaudRate >> 24);
        ucNegotiation[4] = (unsigned char)(TELNET_session->ptrRFC2217_Instance->uart_settings.ulBaudRate >> 16);
        ucNegotiation[5] = (unsigned char)(TELNET_session->ptrRFC2217_Instance->uart_settings.ulBaudRate >> 8);
        ucNegotiation[6] = (unsigned char)(TELNET_session->ptrRFC2217_Instance->uart_settings.ulBaudRate);
        iLength = 4;
        break;
    case SET_DATASIZE_SERVER:
        if ((*ptrData != 0) && (TELNET_session->ptrRFC2217_Instance->uart_settings.ucDataSize != *ptrData)) { // request for data size modification
            if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_SETTINGS_CHANGED, &TELNET_session->ptrRFC2217_Instance->uart_settings); // request the application to change the interface settings - if it doesn't accept values it can overwrite them
            }
        }
        ucNegotiation[3] = TELNET_session->ptrRFC2217_Instance->uart_settings.ucDataSize;
        break;
    case SET_PARITY_SERVER:
        if ((*ptrData != 0) && (TELNET_session->ptrRFC2217_Instance->uart_settings.ucParity != *ptrData)) { // request for parity modification
            if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_SETTINGS_CHANGED, &TELNET_session->ptrRFC2217_Instance->uart_settings); // request the application to change the interface settings - if it doesn't accept values it can overwrite them
            }
        }
        ucNegotiation[3] = TELNET_session->ptrRFC2217_Instance->uart_settings.ucParity;
        break;
    case SET_STOPSIZE_SERVER:
        if ((*ptrData != 0) && (TELNET_session->ptrRFC2217_Instance->uart_settings.ucStopBits != *ptrData)) { // request for stop size modification
            if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_SETTINGS_CHANGED, &TELNET_session->ptrRFC2217_Instance->uart_settings); // request the application to change the interface settings - if it doesn't accept values it can overwrite them
            }
        }
        ucNegotiation[3] = TELNET_session->ptrRFC2217_Instance->uart_settings.ucStopBits;
        break;
    case SET_CONTROL_SERVER:
        if ((*ptrData != 0) && (TELNET_session->ptrRFC2217_Instance->uart_settings.ucFlowControl != *ptrData)) { // request for flow control modification
            if (TELNET_session->ptrRFC2217_Instance->RFC2217_callback != 0) {
                TELNET_session->ptrRFC2217_Instance->RFC2217_callback(RFC2217_SETTINGS_CHANGED, &TELNET_session->ptrRFC2217_Instance->uart_settings); // request the application to change the interface settings - if it doesn't accept values it can overwrite them
            }
        }
        ucNegotiation[3] = TELNET_session->ptrRFC2217_Instance->uart_settings.ucFlowControl;
        break;
    case SET_LINESTATE_MASK_SERVER:
        TELNET_session->ptrRFC2217_Instance->ucLineStateMask = *ptrData;
        ucNegotiation[3] = *ptrData;
        break;
    case SET_MODEMSTATE_MASK_SERVER:
        TELNET_session->ptrRFC2217_Instance->ucModemStateMast = *ptrData;
        ucNegotiation[3] = *ptrData;
        break;
    case PURGE_DATA_SERVER:
        switch (*ptrData) {
        case RFC2217_PURGE_ACCESS_SERVER_RECEIVE_DATA_BUFFER:
            fnFlush(TELNET_session->ptrRFC2217_Instance->uart_settings.uartID, FLUSH_RX);
            break;
        case RFC2217_PURGE_ACCESS_SERVER_TRANSMIT_DATA_BUFFER:
            fnFlush(TELNET_session->ptrRFC2217_Instance->uart_settings.uartID, FLUSH_TX);
            break;
        case RFC2217_PURGE_ACCESS_SERVER_TX_RX_DATA_BUFFERS:
            fnFlush(TELNET_session->ptrRFC2217_Instance->uart_settings.uartID, FLUSH_RX);
            fnFlush(TELNET_session->ptrRFC2217_Instance->uart_settings.uartID, FLUSH_TX);
            break;
        }
        ucNegotiation[3] = *ptrData;
        break;
    }
    ucNegotiation[0] = TELNET_IAC;
    ucNegotiation[1] = TELNET_SB;
    ucNegotiation[2] = (ucType + 100);                                   // return server response
    ucNegotiation[3 + iLength] = TELNET_IAC;
    ucNegotiation[4 + iLength] = TELNET_SE;
    return (fnSendBufTCP(TELNET_session->Telnet_socket, (unsigned char *)ucNegotiation, (iLength + 5), (TCP_BUF_SEND | TCP_CONTENT_NEGOTIATION)));
}
#endif

// We handle one negotiation parameter here. If we send something in response we signal this in the return value
//
static unsigned char fnTelnetNegotiate(TELNET *TELNET_session, unsigned char *ucIp_Data, unsigned short usLen, int *iReturn)
{
    if (usLen != 0) {                                                    // we assume a negotiation sequence is always completely contained in a single frame
        unsigned char ucInputCommand = *ucIp_Data++;
        unsigned char ucNegotiateLength = 2;                             // most negotiation parameter lengths are 2 more bytes long
        unsigned char ucNegotationToSend = 0;
        QUEUE_TRANSFER Negotiation_sent = 0;
        if (usLen > 1) {                                                 // handle commands that require at least 2 bytes of input
            switch (ucInputCommand) {                                    // which command / negotiation type is it?
            case TELNET_DO:
                ucInputCommand = 0;                                      // handled
                if (TELNET_ECHO == *ucIp_Data) {
                    if ((TELNET_WILL_ECHO & TELNET_session->usTelnetMode) != 0) { // have we just suggested we echo?
                        TELNET_session->usTelnetMode |= TELNET_DO_ECHO;  // mark we will be doing echos
                        TELNET_session->usTelnetMode &= ~TELNET_WILL_ECHO; // ensure we only answer once
                    }
                    else if ((TELNET_session->usTelnetMode & TELNET_DO_ECHO) == 0) { // if we are not changing, don't respond
                        TELNET_session->usTelnetMode |= TELNET_DO_ECHO;  // the other end has accepted that we perform echoing (or is requesting we do so)
                        ucNegotationToSend = TELNET_WILL;
                    }
                }
                else if (TELNET_BINARY == *ucIp_Data) {
                    ucNegotationToSend = TELNET_WILL;                    // we accept binary mode
                }
                else {
                    ucNegotationToSend = TELNET_WONT;                    // we respond to all other dos with wont
                }
                break;

            case TELNET_DONT:
                ucInputCommand = 0;
                if (TELNET_ECHO == *ucIp_Data) {                         // we should not perform echoing
                    TELNET_session->usTelnetMode &= ~TELNET_DO_ECHO;
                    if ((TELNET_session->usTelnetMode & TELNET_WONT_ECHO) != 0) {
                        TELNET_session->usTelnetMode &= ~TELNET_WONT_ECHO;
                        break;
                    }
                }
                ucNegotationToSend = TELNET_WONT;                        // we are obliged to respond to don't with won't
                break;

            case TELNET_WILL:
                ucInputCommand = 0;
                if (TELNET_BINARY == *ucIp_Data) {
                    Negotiation_sent |= fnSendNeg(TELNET_session, TELNET_DO, *ucIp_Data);
                    TELNET_session->usTelnetMode |= TELNET_BINARY_MODE;  // we have completed the handshake to binary mode
                    TELNET_session->usTelnetMode &= ~TELNET_DO_ECHO;
                }
    #if defined USE_TELNET_CLIENT
                else if ((TELNET_ECHO == *ucIp_Data) && ((TELNET_session->usTelnetMode & TELNET_CLIENT_MODE) != 0) && ((TELNET_session->usTelnetMode & TELNET_CLIENT_MODE_ECHO) != 0)) {
                    ucNegotationToSend = TELNET_DO;                      // we are a client and the server wants to echo - we accept this
                }
    #endif
    #if defined TELNET_RFC2217_SUPPORT                                   // {13}
                else if (TELNET_RFC2217_COM_PORT_OPTION == *ucIp_Data) {
                    if ((TELNET_session->usTelnetMode & TELNET_RFC2217_ACTIVE) != 0) {
                        Negotiation_sent |= fnSendNeg(TELNET_session, TELNET_DO, *ucIp_Data); // accept the mode
                    }
                    else {
                        ucNegotationToSend = TELNET_DONT;                // don't accept the mode
                    }
                }
    #endif
                else {
                    ucNegotationToSend = TELNET_DONT;                    // we respond to all other wills with don't
                }
                break;

            case TELNET_WONT:
                ucInputCommand = 0;
                ucNegotationToSend = TELNET_DONT;                        // we are obliged to respond to won't with don't
                break;
    #if defined TELNET_RFC2217_SUPPORT                                   // {13}
            case TELNET_SB:                                              // start sub-negotiation
                if (TELNET_RFC2217_COM_PORT_OPTION == *ucIp_Data) {
                    ++ucIp_Data;
                    switch (*(ucIp_Data)) {                              // handle server to cient commands (these are terminated by TELNET_IAC and TELNET_SE)
                    case SET_SIGNATURE_SERVER:                           // ?
                        _EXCEPTION("To do...");
                        break;
                    case SET_BAUDRATE_SERVER:                            // 4 bytes in network order specifying the required baud rate - a value or zero requests the baud rate
                        if (fnSetRFC2217_portConfig(TELNET_session, ucIp_Data) != 0) {
                            *iReturn = APP_SENT_DATA;                    // mark that we have sent TCP data
                        }
                        return 5;                                        // move over 5 bytes
                    case SET_DATASIZE_SERVER:                            // 1 byte specifying the number of bits in a character - a value of zero requests the present setting
                    case SET_PARITY_SERVER:                              // 1 byte specifying the parity setting - a value of zero requests the present setting
                    case SET_STOPSIZE_SERVER:                            // 1 byte specifying the stop bit setting - a value of zero requests the present setting
                    case SET_CONTROL_SERVER:                             // 1 byte specifying the flow control setting
                    case SET_LINESTATE_MASK_SERVER:                      // 1 byte specifying the line state mask - 0 removes all mask bits
                    case SET_MODEMSTATE_MASK_SERVER:                     // 1 byte specifying the line state mask - 0 removes all mask bits
                    case PURGE_DATA_SERVER:                              // 1 byte specifying the purge command
                        if (fnSetRFC2217_portConfig(TELNET_session, ucIp_Data) != 0) {
                            *iReturn = APP_SENT_DATA;                    // mark that we have sent TCP data
                        }
                        return 2;                                        // move over 2 bytes
                    // Notification of COM port and modem line changes
                    //
                    case NOTIFY_LINESTATE_SERVER:
                    case NOTIFY_MODEMSTATE_SERVER:
                        _EXCEPTION("To do...");
                        return 2;                                        // move over 2 bytes
                    // Flow control
                    //
                    case FLOWCONTROL_SUSPEND_SERVER:
                    case FLOWCONTROL_RESUME_SERVER:
                        _EXCEPTION("To do...");
                        return 1;                                        // move over 1 byte
                    }
                }
                break;
    #endif
            default:                                                     // not supported (but could be single byte command)
                break;
            }
        }
        switch (ucInputCommand) {                                        // handle single byte commands
        case 0:                                                          // handled two byte case
            break;
    #if defined TELNET_RFC2217_SUPPORT                                   // {13}
        case TELNET_SE:                                                  // end sub-negotiation
    #endif
        case TELNET_DATA_MARK:
        default:                                                         // presently assume all unknown commands are single byte command that can be ignored
            return 1;                                                    // skip 
        }

        if (ucNegotationToSend != 0) {                                   // if there is a response to be sent
            Negotiation_sent |= fnSendNeg(TELNET_session, ucNegotationToSend, *ucIp_Data); // send the response
        }
        if (Negotiation_sent != 0) {                                     // if the response transmission was successful
            *iReturn = APP_SENT_DATA;                                    // mark that we have sent TCP data
        }
        return ucNegotiateLength;                                        // length of found negotiation command that is to be skipped
    }
    return 0;                                                            // no data to be skipped
}

// This routine filters out double IAC occurances in the input data stream and interprets TELNET commands
//
static unsigned short fnHandleIAC(TELNET *TELNET_session, unsigned char *ucData, unsigned short usLength, int *iReturn)
{
    unsigned char *ptrInput = ucData;
    unsigned char *ptrOutput = ucData;
    unsigned short usFF_found = (TELNET_session->usTelnetMode & TELNET_BIN_RX_IAC); // get original state
    unsigned short usInputLength = usLength;

    TELNET_session->usTelnetMode &= ~TELNET_BIN_RX_IAC;

    while (usInputLength-- != 0) {                                       // for each input byte
        if ((usFF_found == 0) || (*ptrInput != TELNET_IAC)) {            // if not sequence of 2 IAC bytes
            if (*ptrInput == TELNET_IAC) {                               // either first of a double pair or a command sequence
                usFF_found = TELNET_BIN_RX_IAC;                          // we don't copy the IAC (yet), but mark that we have found a starting one
                ptrInput++;                                              // jump over
                usLength--;                                              // removed
            }
            else {
                if (usFF_found != 0) {                                   // probably an IAC
                    unsigned char ucNegLength;
                    usFF_found = 0;
                    if (((TELNET_session->usTelnetMode & TELNET_RAW_SOCKET) == 0) && ((ucNegLength = fnTelnetNegotiate(TELNET_session, ptrInput, usLength, iReturn)) != 0)) {
                        usLength -= ucNegLength;
                        ptrInput += ucNegLength;                         // jump over IAC command sequence
                        usInputLength -= (ucNegLength - 1);
                        continue;                                        // continue after treatment and removal of IAC
                    }
                    else {
                        usLength++;
                        *ptrOutput++ = TELNET_IAC;                       // insert the IAC, which doesn't belong to a command sequence
                    }
                }
                *ptrOutput++ = *ptrInput++;                              // transfer the input stream to output
            }
        }
        else {                                                           // sequence of 2 x IAC is a special case
            ptrInput++;
            *ptrOutput++ = TELNET_IAC;                                   // add only 1 x IAC to buffer
            usFF_found = 0;
        }
    }
    TELNET_session->usTelnetMode |= usFF_found;                         // updata reception state to flag if we have part of a IAC sequence
    return usLength;                                                    // length of input after any IAC (negotiation) removal
}

// Telnet client/server standard call back
//
static int fnTELNETListener(USOCKET Socket, unsigned char ucEvent, unsigned char *ucIp_Data, unsigned short usPortLen)
{
    TELNET *TELNET_session = fnGetTelnetSession(Socket);
    int iReturn;

    if (TELNET_session == 0) {                                           // if there is no Telnet session for this socket
        return APP_REJECT;
    }
    iReturn = APP_ACCEPT;                                                // prepare default response

    switch (ucEvent) {                                                   // the TCP event to be handled
    case TCP_EVENT_CONREQ:                                               // session request received on the TCP port
        if (TELNET_session->ucState == TELNET_STATE_CONNECTED) {         // a request for connection to a connected session is always rejected
            return APP_REJECT;
        }
        else if (TELNET_session->ucState != TELNET_STATE_RESERVED) {     // {7} if the connection is reserved it means that we already received a SYN and our SYN + ACK was probably lost, so accept without disturbing the application
            if (APP_REJECT == TELNET_session->fnApp(Socket, TCP_EVENT_CONREQ, ucIp_Data, usPortLen)) { // {6} if the application wants to refuse the connection it is done here
                return APP_REJECT;                                       // cause a RST to be sent and the TCP connection to be terminated
            }
            TELNET_session->Telnet_socket = Socket;                      // {8}
            TELNET_session->ucState = TELNET_STATE_RESERVED;             // reserve the session if the socket is available
        }
        break;                                                           // accept, which causes a SYN + ACK to be returned

    case TCP_EVENT_CONNECTED:                                            // TCP connection has been established
        TELNET_session->ucState = TELNET_STATE_CONNECTED;
    #if defined USE_TELNET_CLIENT
        if ((TELNET_session->usTelnetMode & (TELNET_CLIENT_MODE | TELNET_CLIENT_NO_NEGOTIATION)) == (TELNET_CLIENT_MODE)) { // acting as a client with negotiation not disabled
            int iReturnNeg;
            if ((TELNET_session->usTelnetMode & TELNET_CLIENT_MODE_ECHO) != 0) {// we need to negotiate echo
                iReturnNeg = (fnSendNeg(TELNET_session, TELNET_DO, TELNET_ECHO) > 0); // inform that we would like echo
            }
            else {
                iReturnNeg = (fnSendNeg(TELNET_session, TELNET_DONT, TELNET_ECHO) > 0); // inform that we would don't want echo
            }
            iReturn = TELNET_session->fnApp(Socket, TCP_EVENT_CONNECTED, ucIp_Data, usPortLen);
            if (iReturnNeg > iReturn) {
                iReturn = iReturnNeg;
            }
            break;
        }
        return (TELNET_session->fnApp(Socket, TCP_EVENT_CONNECTED, ucIp_Data, usPortLen));
    #else
        return (TELNET_session->fnApp(Socket, TCP_EVENT_CONNECTED, ucIp_Data, usPortLen));
    #endif

    case TCP_EVENT_ABORT:
        fnTCP_Listen(Socket, TELNET_session->usTelnetPortNumber, ptrTELNET->usMaxWindow); // set back to listening state
        fnResetTelnetSession(TELNET_session);
        TELNET_session->fnApp(Socket, TCP_EVENT_ABORT, ucIp_Data, usPortLen);
        // Fall through intentionally
        //
    case TCP_EVENT_CLOSE:
        return (TELNET_session->fnApp(Socket, TCP_EVENT_CLOSE, ucIp_Data, usPortLen));    

    case TCP_EVENT_CLOSED:
        TELNET_session->fnApp(Socket, TCP_EVENT_CLOSED, ucIp_Data, usPortLen);
        fnResetTelnetSession(TELNET_session);
        fnTCP_Listen(Socket, TELNET_session->usTelnetPortNumber, ptrTELNET->usMaxWindow); // set TCP port back to listening state
        break;

    #if defined SUPPORT_PEER_WINDOW
    case TCP_EVENT_PARTIAL_ACK:                                          // possible ack to a part of a transmission received
        if (TELNET_session->wakeOnAck != 0) {
            uTaskerStateChange(TELNET_session->wakeOnAck, UTASKER_ACTIVATE); // wake application so that it can continue with queued receive data
        }
        if (fnSendBufTCP(Socket, 0, usPortLen, TCP_BUF_NEXT)) {          // send next buffered (if waiting)
            return APP_SENT_DATA;
        }
        break;
    #endif

    case TCP_EVENT_ACK:
        if (TELNET_session->wakeOnAck != 0) {
            uTaskerStateChange(TELNET_session->wakeOnAck, UTASKER_ACTIVATE); // wake application so that it can continue with queued receive data
        }

        if (fnSendBufTCP(Socket, 0, 0, TCP_BUF_NEXT) != 0) {             // send next buffered (if waiting)
            iReturn = APP_SENT_DATA;                                     // mark that data has been transmitted
        }

        iReturn |= TELNET_session->fnApp(Socket, TCP_EVENT_ACK, 0, 0);   // let application also handle ack

        if ((iReturn & APP_REQUEST_CLOSE) != 0) {                        // if application request a close
            fnTCP_close(Socket);                                         // terminate the connection
            return APP_REQUEST_CLOSE;
        }
        break;

    case TCP_EVENT_REGENERATE:
        if (usPortLen == 0) {                                            // {5} if there is no data to be repeated interpret as probe request
            fnSendBufTCP(Socket, 0, 0, (TCP_BUF_NEXT | TCP_BUF_SEND));   // cause probe to be sent
            break;
        }
        if (fnSendBufTCP(Socket, 0, 0, TCP_BUF_REP) != 0) {              // repeat send buffered
    #if defined SUPPORT_PEER_WINDOW 
            fnSendBufTCP(Socket, 0, 0, (TCP_BUF_NEXT | TCP_BUF_KICK_NEXT)); // kick off any following data as long as windowing allows it
    #endif
            return APP_SENT_DATA;
        }
        break;

    case TCP_EVENT_DATA:
        if (((TELNET_session->usTelnetMode & TELNET_RAW_SOCKET) == 0) || ((TELNET_session->usTelnetMode & TELNET_SEARCH_RX_IAC) != 0)) { // in RAW socket mode we ignore Telnet contents unless specifically activated
            if (((usPortLen = fnHandleIAC(TELNET_session, ucIp_Data, usPortLen, &iReturn)) != 0) && ((TELNET_session->usTelnetMode & TELNET_DO_ECHO) != 0)) { // if we are doing echo
    #if defined USE_TELNET
        #if defined USE_TELNET_CLIENT
                if ((TELNET_session->usTelnetMode & TELNET_CLIENT_MODE) == 0) { // never echo in client mode
        #endif
                    QUEUE_TRANSFER SentLength = 0;                       // {1} initialise to zero since it is always checked
                    if ((TELNET_session->usTelnetMode & TELNET_PASSWORD_ENTRY_MODE) != 0) {
                        if (*ucIp_Data == DELETE_KEY) {
                            SentLength = fnSendBufTCP(Socket, (unsigned char *)cDeleteInput, 3, TCP_BUF_SEND); // allow our separation character or delete to pass
                        }
                        else if (*ucIp_Data == ':') {
                            SentLength = fnSendBufTCP(Socket, ucIp_Data, 1, TCP_BUF_SEND); // allow our separation character
                        }
                        else {
                            SentLength = fnSendBufTCP(Socket, (unsigned char *)"*", 1, TCP_BUF_SEND); // don't echo input but send a star (password entry)
                        }
                    }
                    else {
                        SentLength = fnSendBufTCP(Socket, ucIp_Data, usPortLen, TCP_BUF_SEND); // {2} perform echo of input 
                    } 
                    if (SentLength != 0) {
                        iReturn = APP_SENT_DATA;
                    }
        #if defined USE_TELNET_CLIENT
                }
        #endif
    #endif
            }
        }
        return (iReturn | TELNET_session->fnApp(Socket, TCP_EVENT_DATA, ucIp_Data, usPortLen)); // pass the received data on to the TELNET application

    #if defined SUPPORT_PEER_WINDOW 
    case TCP_WINDOW_UPDATE:                                              // {4} the peer is informing of a change in its receiver window size
        if (fnSendBufTCP(Socket, 0, 0, (TCP_BUF_NEXT | TCP_BUF_KICK_NEXT)) != 0) { // kick off any following data as long as windowing allows it
            return APP_SENT_DATA;
        }
    #endif
        break;
    }
    return iReturn;
}

#if defined USE_TELNET_CLIENT
// Establish a connection on a particular TELNET client interface
//
extern USOCKET fnConnectTELNET(TELNET_CLIENT_DETAILS *ptrTelnetClientDetails)
{
    TELNET *TELNET_session;
    if (ptrTelnetClientDetails->ucInterfaceReference >= TELNET_CLIENT_COUNT) {
        return -1;                                                       // invalid interface
    }
    fnInitialiseTelnetSessions();                                        // initialise Telnet sessions if needed
    TELNET_session = ptrTELNET;
    TELNET_session += (NO_OF_TELNET_SESSIONS - TELNET_CLIENT_COUNT);
    TELNET_session += ptrTelnetClientDetails->ucInterfaceReference;      // set to the session according to the interface being used
    if (TELNET_session->Telnet_socket < 0) {                             // socket not yet allocated
        TELNET_session->Telnet_socket = fnGetTCP_Socket(TOS_MINIMISE_DELAY, ptrTelnetClientDetails->usIdleTimeout, fnTELNETListener);
    }
    if (TELNET_session->Telnet_socket >= 0) {
        if (TELNET_session->ucState != TELNET_STATE_FREE) {
            return SOCKET_STATE_INVALID;
        }
        TELNET_session->fnApp = ptrTelnetClientDetails->listener;
        TELNET_session->usTelnetPortNumber = ptrTelnetClientDetails->usPortNumber;
        TELNET_session->usMaxWindow = ptrTelnetClientDetails->usMaxWindow;
        TELNET_session->wakeOnAck = ptrTelnetClientDetails->wakeOnAck;
        TELNET_session->ptrTelnetClientDetails = ptrTelnetClientDetails;
        TELNET_session->usTelnetMode = (ptrTelnetClientDetails->usTelnetMode | TELNET_CLIENT_MODE);
        if (fnTCP_Connect(TELNET_session->Telnet_socket, ptrTelnetClientDetails->ucDestinationIPv4, ptrTelnetClientDetails->usPortNumber, 0, 0) < 0) { // attempt to start a connection
            return SOCKET_STATE_INVALID;                                 // connection couldn't be started
        }
    }
    return TELNET_session->Telnet_socket;                                // return the socket reference
}
#endif
#endif
