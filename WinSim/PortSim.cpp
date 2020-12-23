/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      PortSim.cpp
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    18.07.2008 Parser correction                                         {1}
    19.07.2008 Add UART CTS and break, plus USB frame injection          {2}
    18.10.2008 Add USB setup frame injection                             {3}
    08.12.2008 Pass file string to fnDoPortSim() and fnPlayPortScript()  {4}
    15.02.2009 Add port simulation to M5223X                             {5}
    17.03.2009 Add port simulation to LM3Sxxxx                           {6}
    27.03.2009 Add M521X configuration                                   {7}
    10.04.2010 Add M5214, M5216, M5280, M5281 and M5282                  {8}
    21.05.2010 Add M520X                                                 {9}
    14.01.2011 Add Kinetis                                               {10}
    18.01.2016 Increase buffer to accept full sizes LAN frames           {11}
    24.12.2016 Add I2C data injection                                    {12}
    02.02.2017 Adapt for us tick resolution                              {13}
    18.02.2019 Add Komodo CAN recording input                            {14}
    24.08.2020 Add USB0/USB1

*/

#include "config.h"
#include "WinSim.h"
#include "Fcntl.h"
#include "io.h"
#include <sys/stat.h>
#if _VC80_UPGRADE>=0x0600
    #include <share.h>
#endif

static int iSimActive = 0;

static unsigned long ulOldTimeStamp = 0;
static unsigned long ulNewTimeStamp;
static unsigned long ulTimeDifference = 0;

#define ABSOLUTE_TIME 0
#define RELATIVE_TIME 1

// Extract ascii decimal input and update local time stamps
//
static void fnExtractTime(unsigned char *ucInput, int iStampType)
{
    unsigned long ulInputValue = 0;

    while (*ucInput != 0) {
        ulInputValue *= 10;
        ulInputValue += (*ucInput++ - '0');
    }
    ulNewTimeStamp = ulInputValue;
    
    if (iStampType == ABSOLUTE_TIME) {
        if (ulNewTimeStamp < ulOldTimeStamp) {
            ulNewTimeStamp = ulOldTimeStamp;
            ulTimeDifference = 0;
        }
        else {
            ulTimeDifference = (ulNewTimeStamp - ulOldTimeStamp);
            ulOldTimeStamp = ulNewTimeStamp;
        }
    }
    else {                                                               // assume relative time stamp
        ulNewTimeStamp = ulOldTimeStamp + ulNewTimeStamp;
        ulTimeDifference = (ulNewTimeStamp - ulOldTimeStamp);
        ulOldTimeStamp = ulNewTimeStamp;
    }
}

// Set next timer stamp position based on the file contents
//
static int fnNextTimeStamp(int iFile, int iStampType)
{
    unsigned char ucCharacter;
    unsigned char ucInput[20];
    int iTimeStampLength = 0;
    int iReturn = -1;

    while (_read(iFile, &ucCharacter, 1) > 0) {
        if (ucCharacter < '0') {                                         // white space
            if (iTimeStampLength == 0) {
                continue;
            }
            ucInput[iTimeStampLength] = 0;
            fnExtractTime(ucInput, iStampType);
            return 1;                                                    // new time stamp ready
        }
        if (ucCharacter > '9') {
            break;                                                       // invalid time stamp
        }
        ucInput[iTimeStampLength] = ucCharacter;
        if (iTimeStampLength++ > sizeof(ucInput)) {
            break;                                                       // too long so assume invalid
        }
    }
    return iReturn;
}

static unsigned long fnExtractDec(char **ptr_cCanTime)
{
    unsigned long ulValue = 0;
    while (**ptr_cCanTime != 0) {
        ulValue *= 10;
        ulValue += (**ptr_cCanTime - '0');
        (*ptr_cCanTime)++;
    }
    return ulValue;
}

static int fnExtractCAN_time(char *ptr_cCanTime)                         // {14}
{
    unsigned long ulTime;
    unsigned long ulNewTimeStamp;
    // Minutes
    //
    ulTime = fnExtractDec(&ptr_cCanTime);
    ulNewTimeStamp = (ulTime * 60 * 1000);
    ptr_cCanTime++;
    // Seconds
    //
    ulTime = fnExtractDec(&ptr_cCanTime);
    ulNewTimeStamp += (ulTime * 1000);
    ptr_cCanTime++;
    // ms
    //
    ulTime = fnExtractDec(&ptr_cCanTime);
    ulNewTimeStamp += ulTime;
    ptr_cCanTime++;
    // us ignored
    //
    if (ulNewTimeStamp < ulOldTimeStamp) {
        ulNewTimeStamp = ulOldTimeStamp;
        ulTimeDifference = 0;
    }
    else {
        ulTimeDifference = (ulNewTimeStamp - ulOldTimeStamp);
        ulOldTimeStamp = ulNewTimeStamp;
    }
    return 1;
}

static int fnGetTime(int iFile, int iCAN)
{
    unsigned char ucCharacter;
    int iComment = 0;
    int iCanTime = 0;
    char cCanTime[32] = {0};

    while (_read(iFile, &ucCharacter, 1) > 0) {                          // for each character - we assume we are always at a start of a line
        switch (ucCharacter) {
        case '.':
            if ((iCAN != 0) && (iCanTime != 0)) {
                cCanTime[iCanTime++] = 0;                                // seconds and milliseconds
                break;
            }
            break;
        case ',':
            if ((iCAN != 0) && (iCanTime != 0)) {
                cCanTime[iCanTime++] = 0;                                // microseconds
                return (fnExtractCAN_time(cCanTime));
            }
            break;
        case ':':                                                        // absolute time
            if ((iCAN != 0) && (iCanTime != 0)) {
                cCanTime[iCanTime++] = 0;                                // minutes
                break;
            }
            if (iComment == 0) {
                return (fnNextTimeStamp(iFile, ABSOLUTE_TIME));
            }
            break;
        case '+':                                                        // relative time
            if (iComment == 0) {
                return (fnNextTimeStamp(iFile, RELATIVE_TIME));
            }
            break;
        case '\r':
        case '\n':
            iComment = 0;                                                // start next line
            break;
        case '/':                                                        // comment line
            iComment = 1;
            break;
        default:                                                         // other characters or while space
            if (iCAN != 0) {
                cCanTime[iCanTime++] = ucCharacter;
            }
            break;
        }
    }
    return 0;                                                            // end of file
}

// Get the number of Tick intervals to wait for next ms delay
//
static unsigned long fnGetTimeDelay(int iFile, int iCAN) 
{
    if (fnGetTime(iFile, iCAN) <= 0) {
        return 0;                                                        // error or complete - quit
    }
    return (((ulTimeDifference * 1000) + (TICK_RESOLUTION))/(TICK_RESOLUTION)); // {13} time to wait in TICK intervals
}

static int fnEquivalent(char *input, char *command)
{
    while (*input != 0) {
        if (*input != *command) {
            if (*input != (*command + ('a' - 'A'))) {                    // accept small/capital mix
                return 0;                                                // no match
            }
        }
        input++;
        command++;
    }
    return 1;                                                            // successful match
}

static int fnMatchCommand(unsigned char ucCharacter)
{
    static char *cCommands[] = {
        "UART",                                                          // inject UART data
        "SPI",                                                           // inject SPI data
        "ETH",                                                           // inject Ethernet data
        "PORT",                                                          // inject port input changes
        "CAN",                                                           // inject CAN data
        "USB",                                                           // inject USB data
        "SUSB",                                                          // inject USB setup frames
        "CTS",                                                           // inject CTS changes
        "BREAK",                                                         // inject UART breaks
        "I2C",                                                           // inject I2C data
        "I2CR",                                                          // inject I2C data followed by a repeated start
        "USB0",                                                          // inject USB0 data
        "USB1",                                                          // inject USB1 data
        "SUSB0",                                                         // inject USB0 setup frames
        "SUSB1",                                                         // inject USB1 setup frames
        0                                                                // end of list
    };
    #define SIM_TYPE_UART       1                                        // keep in strict order with string table entries
    #define SIM_TYPE_SPI        2
    #define SIM_TYPE_ETH        3
    #define SIM_TYPE_PORT       4
    #define SIM_TYPE_CAN        5
    #define SIM_TYPE_USB        6
    #define SIM_TYPE_USB_SETUP  7
    #define SIM_TYPE_UART_CTS   8
    #define SIM_TYPE_UART_BREAK 9
    #define SIM_TYPE_I2C_DATA   10
    #define SIM_TYPE_I2C_DATA_R 11
    #define SIM_TYPE_USB0       12                                       // equivalent to SIM_TYPE_USB
    #define SIM_TYPE_USB1       13
    #define SIM_TYPE_USB0_SETUP 14                                       // equivalent to SIM_TYPE_USB_SETUP
    #define SIM_TYPE_USB1_SETUP 15
    
    static char ucMatchInput[31];
    static int iMatchCount = 0;

    if (ucCharacter == 0) {                                              // reset match counter ready for new start
        iMatchCount = 0;
    }
    else {
        if (ucCharacter == '-') {                                        // do a compare since the type name has been received
            char **ptrCommands = cCommands;
            int iType = 1;
            ucMatchInput[iMatchCount] = 0;
            while (*ptrCommands != 0) {
                if (fnEquivalent(ucMatchInput, *ptrCommands++) != 0) {
                    return iType;
                }
                iType++;
            }
        }
        else {
            if (iMatchCount >= (sizeof(ucMatchInput) - 1)) {             // limit size
                iMatchCount = 0;
            }
            else {
                ucMatchInput[iMatchCount++] = ucCharacter;
            }
        }
    }
    return 0;                                                            // no match (yet)
}

// Read an ASCII HEX value from a file and convert it to a binary number
//
static unsigned long fnGetHexValueFromFile(int iFile, unsigned long *ulFinalValue)
{
    static int iString = 0;                                              // static variable to allow string of characters to be handled
    unsigned long ulValue = 0;
    unsigned char ucCharacter;
    int iOperating = 0;
    int iAscii = 0;

    while (_read(iFile, &ucCharacter, 1) > 0) {                          // for each character - we assume we are always at a start of a line
        if ((iString != 0) && (0x22 != ucCharacter)) {                   // " in a string not supported
            *ulFinalValue = ucCharacter;                                 // return the value collected up to now
            return 1;
        }
        if (iAscii == 1) {
            ulValue = ucCharacter;
            iAscii |= 0x02;                                              // mark that the ascii character has been handled
            continue;
        }
        switch (ucCharacter) {
        case '/':                                                        // comment
        case '\r':                                                       // end of line equals end of data
        case '\n':
            if (iOperating != 0) {
                *ulFinalValue = ulValue;
                return 2;
            }
            else {
                return 0;                                                // no more values found
            }

        default:                                                         // other characters or while space
            if ((ucCharacter >= '0') && (ucCharacter <= '9')) {
                ulValue <<= 4;
                ulValue |= (ucCharacter - '0');
                iOperating++;
            }
            else if ((ucCharacter >= 'a') && (ucCharacter <= 'f')) {
                ulValue <<= 4;
                ulValue |= ((ucCharacter - 'a') + 10);
                iOperating++;
            }
            else if ((ucCharacter >= 'A') && (ucCharacter <= 'F')) {
                ulValue <<= 4;
                ulValue |= ((ucCharacter - 'A') + 10);
                iOperating++;
            }
            else if (0x27 == ucCharacter) {                              // ascii character
                if (iAscii & 0x02) {
                    *ulFinalValue = ulValue;                             // return the value collected up to now
                    return 1;  
                }
                iAscii ^= 1;
            }
            else if (0x22 == ucCharacter) {                              // string
                iString ^= 1;                                            // handling a string of ascii charcters
            }
            else {                                                       // space assumed
                if (iOperating != 0) {
                    *ulFinalValue = ulValue;                             // return the value collected up to now
                    return 1;                                      
                }
            }
            if (iOperating >= 8) {                                       // if 8 characters have been received return the longest unit
                *ulFinalValue = ulValue;                                 // return the value collected up to now
                return 1;
            }
            break;
        }
    }
    return 0;
}

// Read an ASCII Decimal value from a file and convert it to a binary number
//
static unsigned long fnGetDecValueFromFile(int iFile)
{
    unsigned long ulValue = 0;
    unsigned char ucCharacter;
    int iOperating = 0;

    while (_read(iFile, &ucCharacter, 1) > 0) {                          // for each character - we assume we are always at a start of a line
        switch (ucCharacter) {
        case '/':                                                        // comment
        case '\r':                                                       // end of line equals end of data
        case '\n':
            return ulValue;

        default:                                                         // other characters or while space
            if ((ucCharacter >= '0') && (ucCharacter <= '9')) {
                ulValue *= 10;
                ulValue += (ucCharacter - '0');
                iOperating++;
            }
            else {                                                       // space assumed
                if (iOperating != 0) {
                    return ulValue;                                      // return the value collected up to now
                }
            }
            if (ulValue > 1000) {                                        // an unexpectedly high decimal value has been received
                return ulValue;                                          // return the value collected up to now
            }
            break;
        }
    }
    return ulValue;
}

// The port interpretation can be hardware dependent
//
static void fnGetPortInfo(int iFile, unsigned char ucPortNumber)
{
#if defined _M5223X
    unsigned char ucCharacter;
    unsigned char ucPort = 0;
    unsigned long ulValue;
    switch (ucPortNumber) {
        case 'a':                                                        // possibly port AN or AS
        case 'A':
            ucPort = 'A';
            break;
        case 'd':                                                        // possibly port DD
        case 'D':
            ucPort = 'D';
            break;
        case 'g':                                                        // possibly port GP
        case 'G':
            ucPort = 'G';
            break;
        case 'l':                                                        // possibly port LD
        case 'L':
            ucPort = 'L';
            break;
        case 'n':                                                        // possibly port NQ
        case 'N':
            ucPort = 'N';
            break;
        case 'q':                                                        // possibly port QS
        case 'Q':
            ucPort = 'Q';
            break;
        case 't':                                                        // possibly port TA, TC, TD, TE, TF, TG, TH, TI or TJ
        case 'T':
            ucPort = 'T';
            break;
        case 'u':                                                        // possibly port UA, UB or UC
        case 'U':
            ucPort = 'U';
            break;
        default:
            return;                                                      // invalid port
    }
    _read(iFile, &ucCharacter, 1);
    #if !defined _M520X
    switch (ucPort) {
    #if !defined _M52XX_SDRAM && !defined _M520X                         // {8}{9}
        case 'A':
            if ((ucCharacter == 'N') || (ucCharacter == 'n')) {
                ucPort = _PORT_AN;
            }
            else if ((ucCharacter == 'S') || (ucCharacter == 's')) {
                ucPort = _PORT_AS;
            }
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M520X                                                  // {9}
        case 'D':
            if ((ucCharacter == 'D') || (ucCharacter == 'd')) {
                ucPort = _PORT_DD;
            }
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M5225X && !defined _M521X && !defined _M52XX_SDRAM && !defined _M520X // {7}{8}{9}
        case 'G':
            if ((ucCharacter == 'P') || (ucCharacter == 'p')) {
                ucPort = _PORT_GP;
            }
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M5225X && !defined _M521X && !defined _M52XX_SDRAM && !defined _M520X // {7}{8}{9}
        case 'L':
            if ((ucCharacter == 'D') || (ucCharacter == 'd')) {
                ucPort = _PORT_LD;
            }
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M5225X && !defined _M520X                              // {9}
        case 'N':
            if ((ucCharacter == 'Q') || (ucCharacter == 'q')) {
                ucPort = _PORT_NQ;
            }
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M520X                                                  // {9}
        case 'Q':
            if ((ucCharacter == 'S') || (ucCharacter == 's')) {
                ucPort = _PORT_QS;
            }
            else {
                return;                                                  // invalid port
            }
            break;
        case 'T':
            if ((ucCharacter == 'A') || (ucCharacter == 'a')) {
                ucPort = _PORT_TA;
            }
            else if ((ucCharacter == 'C') || (ucCharacter == 'c')) {
                ucPort = _PORT_TC;
            }
        #if !defined _M5225X
            else if ((ucCharacter == 'D') || (ucCharacter == 'd')) {
                ucPort = _PORT_TD;
            }
        #endif
        #if defined _M5225X
            else if ((ucCharacter == 'E') || (ucCharacter == 'e')) {
                ucPort = _PORT_TE;
            }
            else if ((ucCharacter == 'F') || (ucCharacter == 'f')) {
                ucPort = _PORT_TF;
            }
            else if ((ucCharacter == 'G') || (ucCharacter == 'g')) {
                ucPort = _PORT_TG;
            }
            else if ((ucCharacter == 'H') || (ucCharacter == 'h')) {
                ucPort = _PORT_TH;
            }
            else if ((ucCharacter == 'I') || (ucCharacter == 'i')) {
                ucPort = _PORT_TI;
            }
            else if ((ucCharacter == 'J') || (ucCharacter == 'j')) {
                ucPort = _PORT_TJ;
            }
        #endif
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
    #if !defined _M520X                                                  // {9}
        case 'U':
            if ((ucCharacter == 'A') || (ucCharacter == 'a')) {
                ucPort = _PORT_UA;
            }
        #if !defined _M52XX_SDRAM                                        // {8}
            else if ((ucCharacter == 'B') || (ucCharacter == 'b')) {
                ucPort = _PORT_UB;
            }
            else if ((ucCharacter == 'C') || (ucCharacter == 'c')) {
                ucPort = _PORT_UC;
            }
        #endif
            else {
                return;                                                  // invalid port
            }
            break;
    #endif
        default:
            return;                                                      // invalid port
    }
    #endif
    _read(iFile, &ucCharacter, 1);
    if (ucCharacter == '-') {                                            // a bit position follows
        unsigned long ulBitMask = (0x00000001 << fnGetDecValueFromFile(iFile));
        if (!fnGetHexValueFromFile(iFile, &ulValue)) {                   // get the bit state
            return;                                                      // missing data - ignore
        }
        if (ulValue >= 1) {               
            fnInjectPortValue(ucPort, ulBitMask, ulBitMask);             // set input high
        }
        else {
            fnInjectPortValue(ucPort, ulBitMask, 0);                     // set input low
        }
    }
    else {                                   
        if (!fnGetHexValueFromFile(iFile, &ulValue)) {
            return;                                                      // missing data - ignore
        }
        fnInjectPortValue(ucPort, 0xff, ulValue);                        // complete port being defined
    }
#elif defined _KINETIS                                                   // {10}
    unsigned char ucCharacter;
    int iPort = 0;
    unsigned long ulValue;

    _read(iFile, &ucCharacter, 1);
    if (ucPortNumber >= 'A') {
        iPort = (ucPortNumber - 'A');
        if (iPort < PORTS_AVAILABLE) {
            if (ucCharacter == '-') {                                    // a bit position follows
                unsigned long ulBitMask = (0x00000001 << fnGetDecValueFromFile(iFile));
                if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {       // get the bit state
                    return;                                              // missing data - ignore
                }
                if (ulValue >= 1) {               
                    fnInjectPortValue(iPort, ulBitMask, ulBitMask);      // set input high
                }
                else {
                    fnInjectPortValue(iPort, ulBitMask, 0);              // set input low
                }
            }
            else {                                   
                if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {
                    return;                                              // missing data - ignore
                }
                fnInjectPortValue(iPort, 0xffffffff, ulValue);           // complete port being defined
            }
        }
    }
#elif defined _HW_SAM7X
    unsigned char ucCharacter;
    int iPort = 0;
    unsigned long ulValue;
                                                                          // the SAM7X has 2 x 31 bit ports - PORT-A / PORT-B 
    if ((ucPortNumber == 'B') || (ucPortNumber == 'b')) {                 // otherwise A assumed
        iPort = 1;
    }
    _read(iFile, &ucCharacter, 1);
    if (ucCharacter == '-') {                                            // a bit position follows
        unsigned long ulBitMask = (0x00000001 << fnGetDecValueFromFile(iFile));
        if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {               // get the bit state
            return;                                                      // missing data - ignore
        }
        if (ulValue >= 1) {               
            fnInjectPortValue(iPort, ulBitMask, ulBitMask);              // set input high
        }
        else {
            fnInjectPortValue(iPort, ulBitMask, 0);                      // set input low
        }
    }
    else {                                   
        if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {
            return;                                                      // missing data - ignore
        }
        fnInjectPortValue(iPort, 0x7fffffff, ulValue);                   // complete port being defined
    }
#elif defined _LM3SXXXX                                                  // {6}
    unsigned char ucCharacter;
    int iPort = 0;
    unsigned long ulValue;
    
    if ((ucPortNumber >= 'a') && (ucPortNumber <= 'j')) {                // the LM3Sxxxx has several 8 bit ports - PORT-A, PORT-B ... PORT-J
        ucPortNumber -= ('a' - 'A');
    }
    if ((ucPortNumber >= 'A') && (ucPortNumber <= 'J')) {
        if (ucPortNumber == 'I') {
            return;
        }
        if (ucPortNumber == 'J') {
            iPort = _PORTJ;
        }
        else {
            iPort = (ucPortNumber - 'A');
        }
    }

    _read(iFile, &ucCharacter, 1);
    if (ucCharacter == '-') {                                            // a bit position follows
        unsigned long ulBitMask = (0x00000001 << fnGetDecValueFromFile(iFile));
        if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {               // get the bit state
            return;                                                      // missing data - ignore
        }
        if (ulValue >= 1) {               
            fnInjectPortValue(iPort, ulBitMask, ulBitMask);              // set input high
        }
        else {
            fnInjectPortValue(iPort, ulBitMask, 0);                      // set input low
        }
    }
    else {                                   
        if (fnGetHexValueFromFile(iFile, &ulValue) == 0) {
            return;                                                      // missing data - ignore
        }
        fnInjectPortValue(iPort, 0xff, ulValue);                         // complete port being defined
    }
#endif
}

static void fnExecute(int iFile, int iType)
{
    int iBreak;                                                          // {1}
    unsigned char ucSimBuffer[2048];                                     // {11}
    unsigned long ulDataLength = 0;
    unsigned long ulDataValue;
    int iPortNumber = 0;
    unsigned char ucCharacter;
    _read(iFile, &ucCharacter, 1);                                       // get the port number (assume 0..9)
    iPortNumber = (ucCharacter - '0');

    if (iType == SIM_TYPE_PORT) {                                        // get port reference and possibly bit
        fnGetPortInfo(iFile, ucCharacter);
        return;
    }

    while ((iBreak = fnGetHexValueFromFile(iFile, &ulDataValue)) != 0) { // extract the data to a binary input buffer ready for treatment
        if (ulDataLength >= sizeof(ucSimBuffer)) {
            break;
        }
        ucSimBuffer[ulDataLength++] = (unsigned char)ulDataValue;
        if (iBreak == 2) {                                               // {1} last value
            break;
        }
    }

    switch (iType) {
#if defined SERIAL_INTERFACE
    case SIM_TYPE_UART:
        fnInjectSerial(ucSimBuffer, (unsigned short)ulDataLength, iPortNumber);
        break;
    case SIM_TYPE_UART_CTS:                                              // {2}
        if (ucSimBuffer[0] == 0) {
            fnInjectSerial(0, UART_CTS_NEGATED, iPortNumber);
        }
        else {
            fnInjectSerial(0, UART_CTS_ACTIVATED, iPortNumber);
        }
        break;
    case SIM_TYPE_UART_BREAK:
        fnInjectSerial(0, UART_BREAK_CONDITION, iPortNumber);            // {2}
        break;
#endif
    case SIM_TYPE_SPI:
        fnInjectSPI(ucSimBuffer, (unsigned short)ulDataLength, iPortNumber);
        break;
#if defined ETH_INTERFACE
    case SIM_TYPE_ETH:
        fnInjectFrame(ucSimBuffer, (unsigned short)(ulDataLength));
        break;
#endif
#if defined CAN_INTERFACE
    case SIM_TYPE_CAN:
        break;
#endif
#if defined USB_INTERFACE
    case SIM_TYPE_USB0:
    case SIM_TYPE_USB:
        fnInjectUSB(ucSimBuffer, (unsigned short)ulDataLength, iPortNumber); // {2}
        break;

    case SIM_TYPE_USB0_SETUP:
    case SIM_TYPE_USB_SETUP:                                             // {3}
        fnInjectUSB(ucSimBuffer, (unsigned short)ulDataLength, (iPortNumber + USB_SETUP_FLAG));
        break;

    case SIM_TYPE_USB1:
        fnInjectUSB(ucSimBuffer, (unsigned short)ulDataLength, (iPortNumber + USB1_FLAG));
        break;

    case SIM_TYPE_USB1_SETUP:
        fnInjectUSB(ucSimBuffer, (unsigned short)ulDataLength, (iPortNumber + USB_SETUP_FLAG + USB1_FLAG));
        break;
#endif
#if defined I2C_INTERFACE                                                // {12}
    case SIM_TYPE_I2C_DATA_R:
    case SIM_TYPE_I2C_DATA:
        fnInjectI2C(ucSimBuffer, (unsigned short)ulDataLength, iPortNumber, (iType == SIM_TYPE_I2C_DATA_R));
        break;
#endif
    }
}

static int fnExtractHexPair(unsigned char *ptrHexPair, unsigned char *ucValue)
{
    unsigned char ucNibble1 = (*ptrHexPair++ - '0');
    unsigned char ucNibble2 = (*ptrHexPair - '0');
    if (ucNibble1 > 9) {
        ucNibble1 -= ('A' - '9' - 1);
    }
    if (*ptrHexPair == 0) {
        *ucValue = ucNibble1;
        return 1;
    }
    if (ucNibble2 > 9) {
        ucNibble2 -= ('A' - '9' - 1);
    }
    *ucValue = ((ucNibble1 << 4) | ucNibble2);
    return 0;
}


// Get next command from script file and execute it
//
static void fnInjectHardware_change(int iFile, int iCANmode)
{
    unsigned char ucCharacter;
    int iType = 0;
    int iCanField = 0;
    int iCanFieldInput = 0;
    unsigned char ucCAN_field[128] = {0};
    unsigned long ulCanID = 0;
    int iRTR = 0;
    unsigned char ucDLC = 0;

    fnMatchCommand(0);                                                   // reset command matching function on entry

    while (_read(iFile, &ucCharacter, 1) > 0) {                          // for each character - we assume we are always at a start of a line
        switch (ucCharacter) {
        case ',':
            if (iCANmode != 0) {                                         // {14}separator
                switch (++iCanField) {
                case 1:                                                  // ID
                    {
                        unsigned char ucValue;
                        if (fnExtractHexPair(&ucCAN_field[2], &ucValue) != 0) {
                        }
                        else {
                            ulCanID = (ucValue << 24);
                            if (fnExtractHexPair(&ucCAN_field[4], &ucValue) != 0) {
                                ulCanID >>= 20;                          // standard ID such as 0x3fa
                                ulCanID |= (ucValue & 0xf);
                            }
                            else {
                                ulCanID |= (ucValue << 16);
                                if (fnExtractHexPair(&ucCAN_field[6], &ucValue) != 0) {
                                }
                                else {
                                    ulCanID |= (ucValue << 8);
                                    if (fnExtractHexPair(&ucCAN_field[8], &ucValue) != 0) {
                                    }
                                    else {
                                        ulCanID |= (ucValue);
                                        ulCanID |= 0x80000000;           // mark that it is an extended address
                                    }
                                }
                            }
                        }
                    }
                    break;
                case 2:                                                  // RTR
                    if (ucCAN_field[0] == '1') {
                        iRTR = 1;
                    }
                    break;
                case 3:                                                  // DLC
                    if (ucCAN_field[0] != 0) {
                        ucDLC = (ucCAN_field[0] - '0');
                    }
                    break;

                }
                iCanFieldInput = 0;
            }
            break;
        case '\r':
        case '\n':
            if (iCANmode != 0) {                                         // data
    #if defined CAN_INTERFACE                                            // {14}
                if (ucDLC != 0) {
                    int iDataIndex = 0;
                    unsigned char *ptrData = ucCAN_field;
                    int iLen = ucDLC;
                    while (iLen-- != 0) {
                        fnExtractHexPair(ptrData, &ucCAN_field[iDataIndex++]);
                        ptrData += 3;
                    }
                    fnInjectCAN(ucCAN_field, ulCanID, iRTR, ucDLC, 0);
                }
    #endif
            }
            return;                                                      // empty line - ignore
        default:                                                         // other characters or while space
            if (iCANmode != 0) {
                ucCAN_field[iCanFieldInput++] = ucCharacter;
                if (iCanFieldInput >= sizeof(ucCAN_field)) {
                    iCanFieldInput = 0;
                }
                break;
            }
            if ((ucCharacter < '0') && (ucCharacter != '-')) {           // ignore white space
                break;
            }
            if ((iType = fnMatchCommand(ucCharacter)) != 0) {            // try to match a command
                fnExecute(iFile, iType);                                 // execute the rest of the line
                return;
            }
            break;
        }
    }
    return;                                                              // end of file
}

static int fnDetectCAN_mode(int iFile)
{
//  # Total Phase Komodo CAN GUI v1.00
//  # (c) 2011 Total Phase, Inc.
//  # www.totalphase.com
//  #
//  m:s.ms.us, ID, RTR, DLC, Data
    const char *ptrMatch = "m:s.ms.us,ID,RTR,DLC,Data";                  // length of 25
    unsigned char ucCharacter;
    int iMatch = 0;
    while (_read(iFile, &ucCharacter, 1) > 0) {
        switch (ucCharacter) {
        case '/':
        case '+':
        case '-':
            return 0;
        default:
            if (ptrMatch[iMatch] == ucCharacter) {
                iMatch++;
                if (iMatch == 25) {
                    return 1;
                }
            }
            else {
                iMatch = 0;
            }
            break;
        }
    }
    return 0;
}

// We open a recording and play the data in
//
static int fnPlayPortScript(CHAR *file)                                  // {4}
{   
    static int iFilePort = 0;
    static unsigned long ulNextDelay = 0;
    static int iCANmode = 0;

    if (iFilePort <= 0) {
#if _VC80_UPGRADE<0x0600
        iFilePort = _open(file, (_O_RDONLY));                            // {4}
#else
        _sopen_s(&iFilePort, file, (_O_RDONLY), _SH_DENYWR, _S_IREAD);   // {4}
#endif
        iCANmode = fnDetectCAN_mode(iFilePort);
        if (iCANmode == 0) {
            _lseek(iFilePort, 0, 0);
        }
    }

    if (iFilePort < 0) {
        return 0;                                                        // no file found
    }

    if (ulNextDelay != 0) {
        if (--ulNextDelay == 0) {
            fnInjectHardware_change(iFilePort, iCANmode);                // do next simulation job
        }
        else {
            return 1;
        }
    }

    while (ulNextDelay == 0) {
        ulNextDelay = fnGetTimeDelay(iFilePort, iCANmode);               // get the next delay interval
        if (ulNextDelay == 0) {
            break;                                                       // end of file or error
        }
        if (--ulNextDelay == 0) {                                        // immediate input
            fnInjectHardware_change(iFilePort, iCANmode);                // do next simulation job
        }
        else {
            return 1;                                                    // active
        }
    }

    _close(iFilePort);                                                   // script terminated
    iFilePort = 0;
    ulNextDelay = 0;
    ulOldTimeStamp = 0;
    return 0;
}

extern void fnDoPortSim(int iStart, CHAR *file)                          // {4}
{
    if ((iSimActive == 1) || (iStart != 0)) {
        if ((iStart != 0) && (iSimActive != 0)) {
            return;                                                      // ignore start when already active
        }
        iSimActive = fnPlayPortScript(file);                             // {4}
    }
    else {
        if (iSimActive != 0) {
            iSimActive--;
        }
    }
}