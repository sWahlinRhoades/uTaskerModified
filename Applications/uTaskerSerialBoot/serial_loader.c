/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      serial_loader.c
    Project:   uTasker serial loader
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    07.01.2010 Provisional optional GCC workaround                       {1}
    25.03.2010 Use fnEraseFlashSector() rather than uFileErase()         {2}
    26.03.2010 Change queue buffer size to RX_QUEUE_SIZE                 {3}
    21.04.2010 Correct return value after successful SREC line           {4}
    11.09.2010 Add SPI_SW_UPLOAD support                                 {5}
    05.10.2010 Add optional SHOW_APP_DETAILS                             {6}
    31.10.2010 Correct input buffer limit                                {7}
    31.10.2010 Add encrypted SPI upload support                          {8}
    07.05.2011 Allow USB operation without serial interface              {9}
    18.07.2011 Use SLCD together with K40                                {10}
    03.03.2012 Add TWR_K53N512                                           {11}
    03.03.2013 Add file system entry after SREC download if used together with USB-MSD {12}
    03.03.2013 Add _DELETE_BOOT_MAILBOX() to reset boot mailbox commands after checking {13}
    18.12.2013 Add peripheral resets when jumping to the application     {14}
    20.12.2013 Avoid reading UART input queue once pause commanded       {15}
    15.04.2014 Configure web server when ethernet is enabled             {16}
    03.05.2014 Add option to disable SREC loading but keep other UART functions {17}
    04.06.2014 Add optional PHY port initialisation when required by web server loader {18}
    02.07.2014 Don't check programmed internal flash when the device can't program individual bytes {19}
    12.07.2014 Add KBOOT UART loader option                              {20}
    21.01.2015 Add file object whenever USB-MSD is enabled               {21}
    21.01.2015 Add file object to KBOOT firmware loading                 {22}
    27.05.2015 Add DEVELOPERS_LOADER loader option                       {23}
    23.06.2015 Handle small holes in SREC by committing to flash when necessary {24}
    20.10.2015 Use fnJumpToValidApplication() for checking and jumping to a valid application {25}
    23.10.2015 Renamed from Loader.c to serial_loader.c
    19.11.2015 Allow writing to final byte in the application area       {26}
    20.11.2015 Allow operation on USB-CDC virtual COM                    {27}
    07.01.2016 Remove power from memory stick when jumping to the application {28}
    02.07.2016 Add Intel Hex mode                                        {29}
    03.08.2017 Add USB-MSD iHex/SREC content support                     {30}
    05.10.2017 Add modbus loading support (takes precedence over serial) {31}
    17.01.2018 Add I2C slave mode                                        {32}
    09.06.2018 Add FREE_RUNNING_RX_DMA_RECEPTION mode for SREC/iHEX loader in order to avoid need for an intermediate programming buffer and XON/XOFF pause operation {33}
    11.06.2018 Update KBOOT to V2.0.0
    09.11.2018 Add AES-256 decryption option to KBOOT UART/USB loaders   {34}
    25.04.2019 Allow iHEX/SREC to be terminated by CR or LF, or CR+LF or LF+CR {35}
    07.02.2020 Add AES-256 decryption option to SREC/iHEX loaders        {36}
    09.02.2020 Add iMX support                                           {37}
    16.03.2020 Support saving first line of code at end of loading when using encrypted SREC/iHEX loader and add secure loading operation with intermediate buffer {38}
    07.06.2020 Increase efficiency of blank check from SPI flash and add watchdog retrigger to ensure that large application area checks can't lead to the watchdog firing {39}
    09.06.2020 Add Kboot PROPERTY_MAX_PACKET_SIZE response               {40}

*/


/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"


#if defined SERIAL_INTERFACE || defined I2C_INTERFACE || defined USE_USB_CDC || defined SUPPORT_GLCD || (defined USB_INTERFACE && defined HID_LOADER && defined KBOOT_HID_LOADER) || defined ETH_INTERFACE // {9}{16} remove srec/kboot loader when no serial interface but keep parts for Ethernet operation
    #define KEEP_SERIAL_TASK

#if defined _COMPILE_KEIL || ((defined _iMX) && defined _GNU)
    extern int uTaskerBoot(void);
#else
    extern void uTaskerBoot(void);
#endif

#if defined BLAZE_K22
    #include "widgets.h"                                                 // widgets and images used by the project
#endif
/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#define OWN_TASK                    TASK_APPLICATION

#if defined START_APPLICATION_VIA_RESET
    #undef  RESET_PERIPHERALS
    #define RESET_PERIPHERALS()    fnResetBoard()
#endif

#if defined iMX_FALLBACK_SERIAL_LOADER
    #define iMX_SERIAL_LOADER_TIMEOUT  (10.0 * SEC)
#else
    #define iMX_SERIAL_LOADER_TIMEOUT  (60.0 * SEC)
#endif

#define STATE_INIT                  0x0000                               // task states
#define STATE_ACTIVE                0x0001
#define STATE_QUESTION_DELETE       0x0002
#define STATE_LOADING               0x0004
#define STATE_RESETTING             0x0008
#define STATE_ERROR                 0x0010
#define STATE_QUESTION_MASS_DELETE  0x0020
#define STATE_FINAL_MASS_DELETE     0x0040
#define STATE_PAUSING               0x0080
#define STATE_DEVELOPERS_OPERATION  0x0100
#define STATE_DEVELOPERS_ERASE      0x0200
#define STATE_DEVELOPERS_WRITE      0x0400
#define STATE_DEVELOPERS_WRITE_DATA 0x0800
#define STATE_DEVELOPERS_READ       0x1000
#define STATE_DEVELOPERS_QUIT       0x2000
#define STATE_QUESTION_SWAP         0x4000
#define STATE_ENTER_KEY             0x8000


#define INTERMEDIATE_BUFFER_RESERVE 200

#if (defined SERIAL_INTERFACE && (defined KBOOT_LOADER || defined DEVELOPERS_LOADER || !defined REMOVE_SREC_LOADING)) || defined USE_USB_CDC
    #define RX_QUEUE_SIZE           256
#else
    #define RX_QUEUE_SIZE           HEADER_LENGTH
#endif

#define DEVELOPERS_ACK              0xfc
#define DEVELOPERS_NAK              0x03


// HID KBOOT defines
//
#define KBOOT_REPORT_ID_COMMAND_OUT                  1
#define KBOOT_REPORT_ID_DATA_OUT                     2
#define KBOOT_REPORT_ID_COMMAND_IN                   3
#define KBOOT_REPORT_ID_DATA_IN                      4

#define KBOOT_COMMAND_TAG_ERASE_ALL                  1
#define KBOOT_COMMAND_TAG_ERASE_REGION               2
#define KBOOT_COMMAND_TAG_READ_MEMORY                3
#define KBOOT_COMMAND_TAG_WRITE_MEMORY               4
#define KBOOT_COMMAND_TAG_FILL_MEMORY                5
#define KBOOT_COMMAND_TAG_FLASH_SECURITY_DISABLE     6
#define KBOOT_COMMAND_TAG_GET_PROPERTY               7
#define KBOOT_COMMAND_TAG_RECEIVE_SBFILE             8
#define KBOOT_COMMAND_TAG_EXECUTE                    9
#define KBOOT_COMMAND_TAG_CALL                       10
#define KBOOT_COMMAND_TAG_RESET                      11
#define KBOOT_COMMAND_TAG_SET_PROPERTY               12
#define KBOOT_COMMAND_TAG_MASS_ERASE                 13


#define PROPERTY_VERSION                             1
#define PROPERTY_PERIPHERALS                         2
#define PROPERTY_FLASH_START_ADD                     3
#define PROPERTY_FLASH_SIZE_BYTES                    4
#define PROPERTY_FLASH_SECTOR_SIZE                   5
#define PROPERTY_FLASH_BLOCK_CNT                     6
#define PROPERTY_AVAILABLE_CMDS                      7
#define PROPERTY_CRC_CHECK_STATUS                    8
#define PROPERTY_VERIFY_WRITES                       10
#define PROPERTY_MAX_PACKET_SIZE                     11
#define PROPERTY_RESERVED_REGIONS                    12
#define PROPERTY_VALID_REGIONS                       13
#define PROPERTY_RAM_START_ADD                       14
#define PROPERTY_RAM_SIZE_BYTES                      15
#define PROPERTY_SYS_DEVICE_ID                       16
#define PROPERTY_FLASH_SEC_STATE                     17
#define PROPERTY_UNIQUE_DEVICE_ID                    18


#define KBOOT_SERIAL_START_BYTE                      0x5a                // 'Z'
#define KBOOT_SERIAL_ACK                             0xa1
#define KBOOT_SERIAL_NAK                             0xa2
#define KBOOT_SERIAL_ACK_ABORT                       0xa3
#define KBOOT_SERIAL_COMMAND                         0xa4
#define KBOOT_SERIAL_DATA                            0xa5
#define KBOOT_SERIAL_PING                            0xa6
#define KBOOT_SERIAL_PING_RESPONSE                   0xa7

// Local controls
//
#define KBOOT_NO_ACTION                  0
#define KBOOT_RESET_COMMANDED            1
#define KBOOT_MASS_ERASE_COMMANDED       2
#endif

#if (defined USB_MSD_DEVICE_LOADER && (defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES)) || (((defined SERIAL_INTERFACE && !defined USE_MODBUS) || defined USE_USB_CDC) && !defined REMOVE_SREC_LOADING && !defined KBOOT_LOADER)
    #define SREC_IHEX_REQUIRED
    #if defined USB_MSD_ACCEPTS_HEX_FILES && !defined USB_MSD_ACCEPTS_SREC_FILES && !defined KEEP_SERIAL_TASK
        #define EXCLUSIVE_INTEL_HEX_MODE
    #endif
#endif
/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

typedef struct
{
    unsigned long addr;
    int           len;
    unsigned char typ;
    unsigned char csum;
    unsigned char data[100];
} SREC_TYP;

#if (defined USB_INTERFACE && (defined USB_MSD_DEVICE_LOADER || defined USE_USB_CDC)) || defined I2C_INTERFACE || (defined SERIAL_INTERFACE && !defined KBOOT_LOADER && !(defined USE_MODBUS && (defined MODBUS_RTU || defined MODBUS_ASCII)))
    #define NEEDS_BLANK_CHECK
    static unsigned char *fnBlankCheck(void);
#endif

#if defined KEEP_SERIAL_TASK
/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

#if defined SERIAL_INTERFACE || defined USE_USB_CDC
    #if defined DEVELOPERS_LOADER                                        // {23}
        static void fnSendDevelopers(QUEUE_HANDLE SerialPortID, unsigned char ucByte);
        static void fnSendDevelopersAckNak(QUEUE_HANDLE SerialPortID, unsigned char ucByte);
        static void fnSendDevelopersLongWord(QUEUE_HANDLE SerialPortID, unsigned long ulLongWord);
        #if defined DEVELOPERS_LOADER_CRC
            static void fnSendCRC(QUEUE_HANDLE SerialPortID);
        #endif
    #elif !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER && !defined REMOVE_SREC_LOADING
        #if !defined REMOVE_SREC_LOADING                                 // {17}
            #if defined USE_USB_CDC && !defined SERIAL_INTERFACE || defined FREE_RUNNING_RX_DMA_RECEPTION // {33}
                #undef INTERMEDIATE_PROG_BUFFER
            #else
                #if defined INTERMEDIATE_PROG_BUFFER
            static int fnIntermediateWrite(unsigned char *ptrAddr, unsigned char *ucPtrData, int iDataCnt);
            static int fnFlashIntermediate(void);
                #endif
            #endif
        #endif
    #endif
    #if defined NEEDS_BLANK_CHECK && !defined KBOOT_LOADER
        static int fnPerformBlankCheck(void);
    #endif
    #if !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER && !(defined USE_MODBUS && (defined MODBUS_RTU || defined MODBUS_ASCII))
        static void fnPrintScreen(void);
    #endif
#elif defined I2C_INTERFACE
    static int fnPerformBlankCheck(void);
#endif
#if ((defined SERIAL_INTERFACE || defined USE_USB_CDC) && defined DEVELOPERS_LOADER) || (defined I2C_INTERFACE && defined I2C_SLAVE_MODE && !defined BLAZE_K22)
    static void fnCommittProgramStart(void);
#endif
#if defined BLAZE_K22
    static void fnLoaderForced(int iNoFirmware);
#endif
#if defined I2C_INTERFACE && defined I2C_SLAVE_MODE && !defined BLAZE_K22// {32}
    static void fnOpenI2C_slave(void);
    static void fnCheckFlashState(void);
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined SERIAL_INTERFACE && defined KBOOT_LOADER                     // {20}
    const unsigned char ucAck[] = {KBOOT_SERIAL_START_BYTE, KBOOT_SERIAL_ACK};
    const unsigned char ucNak[] = {KBOOT_SERIAL_START_BYTE, KBOOT_SERIAL_NAK};
    const unsigned char ucPingResponse[] = {KBOOT_SERIAL_START_BYTE, KBOOT_SERIAL_PING_RESPONSE, 0x00, 0x01, 0x01, 'P', 0x00, 0x00, 0x78, 0x04}; // with serial protocol version (P1.1.0)
#endif

#if defined DEVELOPERS_LOADER
    static const CHAR cProcessorName[] = TARGET_HW;                      // processor/board name for reporting/display use
#endif
#endif

#if defined KBOOT_SECURE_LOADER || defined USB_MSD_DEVICE_SECURE_LOADER || defined SDCARD_SECURE_LOADER || defined SREC_SECURE_LOADER
    static const CHAR decrypt_key[] = "aes256 secret key";
    #define PRIME_AES256_INITIAL_VECTOR
    #if defined PRIME_AES256_INITIAL_VECTOR
    static const CHAR initial_vector[] = "initial vector";
    #endif
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if defined SREC_IHEX_REQUIRED || defined I2C_INTERFACE || (defined DEVELOPERS_LOADER && defined SERIAL_INTERFACE)
    #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
        static unsigned char ucCodeStart[FLASH_ROW_SIZE] = {0};
    #else
        static unsigned char ucCodeStart[4] = {0};
    #endif
    #if defined SREC_SECURE_LOADER
        #define CODE_START_SIZE   0
    #else
        #define CODE_START_SIZE   sizeof(ucCodeStart)
    #endif
#endif

#if defined KEEP_SERIAL_TASK
#if defined SERIAL_INTERFACE || defined USE_USB_CDC
    #if (defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD && !defined USB_MSD_HOST_LOADER) && ((defined HID_LOADER && defined KBOOT_HID_LOADER) || (!defined REMOVE_SREC_LOADING || defined KBOOT_LOADER || defined DEVELOPERS_LOADER)) // {9}{17}
        #define ADD_FILE_OBJECT_AFTER_LOADING                            // {21}
    #endif
    #if defined INTERMEDIATE_PROG_BUFFER && !defined REMOVE_SREC_LOADING && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER && !defined USE_USB_CDC // {17}
        static unsigned long ulOffset = 0;
        static unsigned char *ptrIntermediateFlash = 0;
        static unsigned char ucIntermediateBuffer[INTERMEDIATE_PROG_BUFFER] = {0};
    #endif
#elif defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && defined HID_LOADER && defined KBOOT_HID_LOADER
    #define ADD_FILE_OBJECT_AFTER_LOADING                                // {21}
#endif

#if defined ADD_FILE_OBJECT_AFTER_LOADING                                // {9}{17}
    static FILE_OBJECT_INFO fileObjInfo = {0};                           // {12}
#endif

#if (defined SERIAL_INTERFACE && defined KBOOT_LOADER) || (defined USB_INTERFACE && defined HID_LOADER && defined KBOOT_HID_LOADER)
    static unsigned char *ptrFlashAddress = 0;
    static unsigned long  ulProg_length = 0;
#endif

#if defined DEVELOPERS_LOADER && defined DEVELOPERS_LOADER_CRC
    static unsigned short usCRC_Tx = 0;                                  // present transmit message CRC calculation value
    static unsigned short usCRC_Rx = 0;                                  // present receive message CRC calculation value
#endif
#if defined I2C_INTERFACE && defined I2C_SLAVE_MODE && !defined BLAZE_K22// {32}
    static QUEUE_HANDLE I2CPortID = NO_ID_ALLOCATED;                     // handle of I2C interface
#endif
static int iAppState = STATE_INIT;                                       // task state


// Application task
//
extern void fnApplication(TTASKTABLE *ptrTaskTable)
{
#if (defined SERIAL_INTERFACE && !defined USE_MODBUS) || defined USE_USB_CDC
    static int iInputLength = 0;
    static QUEUE_HANDLE SerialPortID = NO_ID_ALLOCATED;                  // serial port handle
    static unsigned char ucSerialInputMessage[RX_QUEUE_SIZE] = {0};
#endif
#if defined DEVELOPERS_LOADER                                            // {23}
    static unsigned char ucDevelopersDataLength = 0;
#endif
    static unsigned char ucInputMessage[HEADER_LENGTH/*RX_QUEUE_SIZE*/] = {0}; // {3} reserve space for receiving messages (header length used since the serial interface queue is now independent)
    QUEUE_HANDLE PortIDInternal = ptrTaskTable->TaskID;                  // queue ID for task input

    if (STATE_INIT == iAppState) {
#if defined USE_USB_CDC
        SerialPortID = USBPortID_comms;                                  // use the USB-CDC handle
        DebugHandle = USBPortID_comms;
#elif defined USE_MODBUS                                                 // {31}
        fnInitModbus();                                                  // initialise MODBUS
#elif defined SERIAL_INTERFACE
        TTYTABLE tInterfaceParameters;                                   // table for passing information to driver
        tInterfaceParameters.Channel = LOADER_UART;                      // set UART channel for serial use
    #if defined KBOOT_LOADER                                             // {20}
        tInterfaceParameters.ucSpeed = SERIAL_BAUD_57600;                // fixed baud rate for kboot compatibility
        tInterfaceParameters.Config = (CHAR_8 | NO_PARITY | ONE_STOP | CHAR_MODE); // ensure no XON/XOFF mode used since the transfer is binary
    #elif defined DEVELOPERS_LOADER                                      // {23}
        tInterfaceParameters.ucSpeed = SERIAL_BAUD_115200;               // fixed baud rate for developer's loader
        tInterfaceParameters.Config = (CHAR_8 | NO_PARITY | ONE_STOP | CHAR_MODE); // ensure no XON/XOFF mode used since the transfer is binary
    #else
        tInterfaceParameters.ucSpeed = SERIAL_SPEED;                     // baud rate
        tInterfaceParameters.Config = (CHAR_8 | NO_PARITY | ONE_STOP | USE_XON_OFF | CHAR_MODE);
    #endif
    #if defined SERIAL_SUPPORT_DMA
        #if defined FREE_RUNNING_RX_DMA_RECEPTION                        // {33}
            #if (defined KINETIS_KL || defined KINETIS_KM) && !defined DEVICE_WITH_eDMA
        tInterfaceParameters.ucDMAConfig = (UART_RX_DMA | UART_RX_MODULO | UART_TX_DMA); // modulo aligned reception memory is required by kinetis KL parts without eDMA in free-running DMA mode
            #else
        tInterfaceParameters.ucDMAConfig = (UART_RX_DMA | UART_TX_DMA);
            #endif
        tInterfaceParameters.Config &= ~(USE_XON_OFF);                   // disable XON/XOFF flow control
        uTaskerStateChange(OWN_TASK, UTASKER_POLLING);                   // set the task to polling mode to regularly check the receive buffer
        #else
        tInterfaceParameters.ucDMAConfig = 0; // (UART_TX_DMA);
        #endif
    #endif
    #if defined USER_DEFINED_UART_RX_HANDLER
        tInterfaceParameters.receptionHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_RX_BREAK_DETECTION
        tInterfaceParameters.receiveBreakHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_TX_FRAME_COMPLETE
        tInterfaceParameters.txFrameCompleteHandler = 0;
    #endif
        tInterfaceParameters.Rx_tx_sizes.RxQueueSize = RX_BUFFER_SIZE;   // input buffer size
        tInterfaceParameters.Rx_tx_sizes.TxQueueSize = TX_BUFFER_SIZE;   // output buffer size
        tInterfaceParameters.Task_to_wake = OWN_TASK;                    // wake self when messages have been received
        if ((SerialPortID = fnOpen(TYPE_TTY, FOR_I_O, &tInterfaceParameters)) != NO_ID_ALLOCATED) { // open or change the channel with defined configurations (initially inactive)
            fnDriver(SerialPortID, (TX_ON | RX_ON), 0);                  // enable rx and tx
    #if defined KBOOT_RS485 || defined SREC_RS485
            fnDriver(SerialPortID, (MODIFY_CONTROL | CONFIG_RTS_PIN | SET_RS485_MODE), 0); // configure RTS pin for control use
    #endif
        }
    #if !defined KBOOT_LOADER
        DebugHandle = SerialPortID;                                      // assign our serial interface as debug port
    #endif
    #if defined MEMORY_SWAP
        fnHandleSwap(1);                                                 // check that no previous swap had aborted before completion
    #endif
    #if defined _iMX
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(iMX_SERIAL_LOADER_TIMEOUT), T_HOOKUP_TIMEOUT); // wait up to a defined amount of time for a response from the PC
    #endif
    #if defined RX_UART_TO_HOLD_LOADER
        #if defined _WINDOWS
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(3.0* SEC), T_HOOKUP_TIMEOUT); // wait up to 3s for a response from the PC
        #else
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.5 * SEC), T_HOOKUP_TIMEOUT); // wait up to 500ms for a response from the PC
        #endif
        fnDebugMsg("!");                                                 // signal that the loader is starting (enter '?' to hold the mode or else it moves to the application after 500ms)
    #elif !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER            // {20}
        fnPrintScreen();
    #endif
    #if defined SREC_IHEX_REQUIRED
        uMemset(ucCodeStart, 0xff, sizeof(ucCodeStart));                 // reset the contents of a flash buffer that will store the start of the code so that it can be programmed as final step
    #endif
    #if defined INTERMEDIATE_PROG_BUFFER && !defined REMOVE_SREC_LOADING && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER // {17}
        uMemset(ucIntermediateBuffer, 0xff, sizeof(ucIntermediateBuffer)); // start with flushed buffer
    #endif
    #if (defined KWIKSTIK || defined TWR_K40X256 || defined TWR_K40D100M | defined TWR_K53N512 || defined FRDM_KL46Z || defined FRDM_KL43Z || defined TWR_KL43Z48M || defined TWR_KL46Z48M || defined TWR_K40D100M || defined TWR_KM34Z50M || defined FRDM_K32L2B3) && !defined USB_INTERFACE // {10}{11}
        CONFIGURE_SLCD();
    #endif
    #if defined DEVELOPERS_LOADER                                        // {23}
        fnSendDevelopers(SerialPortID, DEVELOPERS_ACK);                  // send initial hook-up character
        #if defined _WINDOWS
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(5 * SEC), T_HOOKUP_TIMEOUT); // allow time for testing
        #else
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.2 * SEC), T_HOOKUP_TIMEOUT); // wait up to 200ms for a response from the PC
        #endif
    #endif
#endif
#if defined I2C_INTERFACE && defined I2C_SLAVE_MODE&& !defined BLAZE_K22 // {32}
        fnOpenI2C_slave();
    #if !defined SREC_IHEX_REQUIRED
        uMemset(ucCodeStart, 0xff, sizeof(ucCodeStart));                 // reset the contents of a flash buffer that will store the start of the code so that it can be programmed as final step
    #endif
#endif
#if defined ETH_INTERFACE                                                // {16}
    #if defined USE_HTTP
        fnConfigureAndStartWebServer();
    #elif defined USE_TFTP
        fnStartDHCP((UTASK_TASK)(FORCE_INIT | OWN_TASK), (DHCP_CLIENT_OPERATION)); // activate DHCP
    #endif
    #if defined _WINDOWS
        fnSimulateLinkUp();                                              // Ethernet link up simulation
    #endif
#endif
#if defined SUPPORT_GLCD
        uTaskerStateChange(TASK_LCD, UTASKER_ACTIVATE);                  // start LCD task
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(2 * SEC), T_HOOKUP_TIMEOUT); // allow time for the user to force loader mode
#endif
        iAppState = STATE_ACTIVE;                                        // move to the active state since initialisation has completed
    }
  //else if ((STATE_RESETTING | STATE_PAUSING) & iAppState) {            // {16} allow reset and application commands to be commanded from web server interface
        while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH) != 0) { // check task input queue
            if (TIMER_EVENT == ucInputMessage[MSG_SOURCE_TASK]) {        // timer event
                if (T_RESET == ucInputMessage[MSG_TIMER_EVENT]) {
                    fnResetBoard();                                      // command hardware reset
                }
#if defined SERIAL_INTERFACE && defined KBOOT_LOADER                     // {20}
                else if (T_MESSAGE_TIMEOUT == ucInputMessage[MSG_TIMER_EVENT]) { // the start of a message was received but it didn't complete
                    fnWrite(SerialPortID, (unsigned char *)ucNak, sizeof(ucNak)); // nak the reception
                    iInputLength = 0;                                   // reset the message
                }
#endif
#if defined KBOOT_LOADER && defined KBOOT_LOADER_MASS_ERASE_TO_UNLOCK
                else if (T_MASS_ERASE == ucInputMessage[MSG_TIMER_EVENT]) {
                    fnMassEraseFlash();                                  // this will completely erase FLASH content, unlocking any security. The code will no longer run after this command
                }
#endif
#if defined SERIAL_INTERFACE && defined INTERMEDIATE_PROG_BUFFER && !defined REMOVE_SREC_LOADING && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER // {17}
                else if (ucInputMessage[MSG_TIMER_EVENT] == T_COMMIT_BUFFER) {
                    if (fnFlashIntermediate() != 0) {                    // download has been temporarily suspended so write block to FLASH (without risk of rx overrun while programming)
                        iAppState = STATE_ERROR;
                    }
                    else {
                        iAppState = STATE_LOADING;
                    }
                    fnDebugMsg("\x11*");                                 // start flow control again since block programming has completed
                }
#endif
                else {                                                   // assumed - go to application - T_GO_TO_APP (or T_HOOKUP_TIMEOUT timeout)
                    fnJumpToValidApplication(1);                         // {25} if there is a valid application jump to it after disabling used peripherals
                    iAppState = STATE_ACTIVE;                            // we return if there is no application
#if defined BLAZE_K22
                    fnLoaderForced(1);                                   // no firmware present so start loader
#endif
#if defined RX_UART_TO_HOLD_LOADER
                    fnPrintScreen();                                     // print screen to show that there is no application available
#endif
                }
            }
#if defined USE_TFTP || defined BLAZE_K22 || defined I2C_INTERFACE
            else if (INTERRUPT_EVENT == ucInputMessage[MSG_SOURCE_TASK]) {
                switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {
    #if defined USE_TFTP
                case DHCP_SUCCESSFUL:                                    // we can now use the network connection
                    fnTransferTFTP();
                    break;

                case DHCP_MISSING_SERVER:
                    fnStopDHCP(ucInputMessage[1]);                       // DHCP server is missing so stop and continue with backup address (if available)
                    break;
    #endif
    #if defined BLAZE_K22
                case USER_FORCE_LOADER:
                    fnLoaderForced(0);                                   // start loader since the user wants to use it
                    break;
    #endif
    #if defined I2C_INTERFACE && defined I2C_SLAVE_MODE && !defined BLAZE_K22 // {32}
                case DELETE_APPLICATION_FLASH:
                    fnEraseFlashSector((unsigned char *)UTASKER_APP_START, (UTASKER_APP_END - (unsigned char *)UTASKER_APP_START)); // delete application space
                    fnCheckFlashState();
                    break;
                case TERMINATE_PROGRAMMING:
        #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
                    fnWriteBytesFlash(0, 0, 0);                          // close any outstanding FLASH buffer
        #endif
                    fnCommittProgramStart();
                    fnResetBoard();                                      // command hardware reset
                    break;
    #endif
                }
            }
#endif
#if defined SUPPORT_GLCD
            else if (TASK_LCD == ucInputMessage[MSG_SOURCE_TASK]) {
                fnRead(PortIDInternal, ucInputMessage, ucInputMessage[MSG_CONTENT_LENGTH]); // read the complete message
                if (E_LCD_INITIALISED == ucInputMessage[0]) {
                    #define SPLASH_SCREEN_X   70
                    #define SPLASH_SCREEN_Y   15
    #if defined VARIABLE_PIXEL_COLOUR
                    GLCD_STYLE graphic_style;
    #endif
                    GLCD_PIC test_pic;
                    GLCD_TEXT_POSITION text_pos;
                    test_pic.usX = (SPLASH_SCREEN_X + 13);
                    test_pic.usY = (SPLASH_SCREEN_Y - 2);
                    test_pic.ucMode = (PAINT_LIGHT);
                    test_pic.ptrPic = (GBITMAP *)flame;
                    fnDoLCD_pic(&test_pic);                              // draw blaze's flame
                    test_pic.usX = (SPLASH_SCREEN_X + 2);
                    test_pic.usY = (SPLASH_SCREEN_Y + 120);
                    test_pic.ptrPic = (GBITMAP *)Blaze;
                    test_pic.ucMode = (PAINT_LIGHT | REDRAW);
                    fnDoLCD_pic(&test_pic);                              // draw blaze's name
                    text_pos.ucFont = FONT_NINE_DOT;
                    text_pos.usX = (SPLASH_SCREEN_X + 12);
                    text_pos.usY = (SPLASH_SCREEN_Y + 158);
                    text_pos.ucMode = (PAINT_LIGHT);
    #if defined VARIABLE_PIXEL_COLOUR
                    graphic_style.ucMode = STYLE_PIXEL_COLOR;
                    graphic_style.color = (COLORREF)RGB(0, 0, 0);        // black pixels
                    fnDoLCD_style(&graphic_style);
    #endif
                    fnDoLCD_text(&text_pos, "POWERED BY");
                    test_pic.usX = (SPLASH_SCREEN_X + 23);
                    test_pic.usY = (SPLASH_SCREEN_Y + 175);
                    test_pic.ptrPic = (GBITMAP *)Tasker;
                    fnDoLCD_pic(&test_pic);                              // draw uTasker name
                    test_pic.usX = (SPLASH_SCREEN_X - 4);
                    test_pic.usY = (SPLASH_SCREEN_Y + 180);
                    test_pic.ptrPic = (GBITMAP *)u_symbol;
    #if defined VARIABLE_PIXEL_COLOUR
                    graphic_style.ucMode = STYLE_PIXEL_COLOR;
                    graphic_style.color = (COLORREF)RGB(255, 0, 0);      // red pixels
                    fnDoLCD_style(&graphic_style);
    #endif
                    fnDoLCD_pic(&test_pic);                              // draw uTasker start symbol
    #if defined VARIABLE_PIXEL_COLOUR
                    graphic_style.ucMode = STYLE_PIXEL_COLOR;
                    graphic_style.color = (COLORREF)RGB(255, 255, 255);  // white pixels
                    fnDoLCD_style(&graphic_style);
    #endif
                }
            }
#endif
        }
#if !defined USE_MODBUS && defined SERIAL_INTERFACE && defined INTERMEDIATE_PROG_BUFFER
        if ((iAppState & STATE_PAUSING) != 0) {                         // never read further input if pausing
            return;
        }
#endif
  //}
#if (defined SERIAL_INTERFACE && !defined USE_MODBUS) || defined USE_USB_CDC
    while (fnRead(SerialPortID, &ucSerialInputMessage[iInputLength], 1) != 0) { // check UART input for reception (one byte at a time)
    #if defined _iMX
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)iMX_SERIAL_LOADER_TIMEOUT, T_HOOKUP_TIMEOUT); // retrigger the timeout whenever there is UART activity
    #endif
    #if defined KBOOT_LOADER                                             // {20} each byte is received here
        if (iInputLength == 0) {                                         // waiting for the start of a message
            if (ucSerialInputMessage[0] == KBOOT_SERIAL_START_BYTE) {    // all packets begin with the start byte ('Z')
                iInputLength = 1;                                        // start receiving the message
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.1 * SEC), T_MESSAGE_TIMEOUT);
            }
        }
        else {                                                           // in reception
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.1 * SEC), T_MESSAGE_TIMEOUT);
            if (iInputLength == 1) {                                     // command byte
                switch (ucSerialInputMessage[1]) {
                case KBOOT_SERIAL_PING:                                  // 0xa6:
                    fnWrite(SerialPortID, (unsigned char *)ucPingResponse, sizeof(ucPingResponse)); // send a ping response back
                    break;
                case KBOOT_SERIAL_DATA:                                  // 0xa5
                case KBOOT_SERIAL_COMMAND:                               // 0xa4:
                    iInputLength = 2;
                    continue;
                case KBOOT_SERIAL_ACK:                                   // 0xa1:
                    break;
                }
                uTaskerStopTimer(OWN_TASK);
                iInputLength = 0;                                        // command completed
            }
            else if (iInputLength == 2) {                                // expecting command length
                // The command length is now in ucSerialInputMessage[2] so the expected length is known
                //
                iInputLength++;
            }
            else {                                                       // collecting message content
                iInputLength++;
                if (iInputLength == (ucSerialInputMessage[2] + 6)) {     // expected reception complete
                    unsigned short usCRC = fnCRC16(0, ucSerialInputMessage, 4);
                    usCRC = fnCRC16(usCRC, &ucSerialInputMessage[6], ucSerialInputMessage[2]);
                    uTaskerStopTimer(OWN_TASK);
                    if (usCRC != ((ucSerialInputMessage[5] << 8) | (ucSerialInputMessage[4]))) { // check the CRC of the reception frame
                        fnWrite(SerialPortID, (unsigned char *)ucNak, sizeof(ucNak)); // nak the reception
                    }
                    else {
                        fnWrite(SerialPortID, (unsigned char *)ucAck, sizeof(ucAck)); // acknowledge the reception
                        if (ucSerialInputMessage[1] == KBOOT_SERIAL_COMMAND) {
                            ucSerialInputMessage[2] = KBOOT_REPORT_ID_COMMAND_OUT; // for compatibility
                        }
                        else {
                            ucSerialInputMessage[4] = ucSerialInputMessage[2]; // shift the packet length to where the checksum is (for compatibility)
                            ucSerialInputMessage[5] = ucSerialInputMessage[3];
                            ucSerialInputMessage[2] = KBOOT_REPORT_ID_DATA_OUT; // for compatibility
                        }
                        switch (fnHandleKboot(SerialPortID, KBOOT_UART, (KBOOT_PACKET *)&ucSerialInputMessage[2])) { // generic kboot handling
                        case KBOOT_RESET_COMMANDED:
                            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1 * SEC), T_RESET);
                            break;
                        case KBOOT_MASS_ERASE_COMMANDED:
                            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.1 * SEC), T_MASS_ERASE);
                            break;
                        }
                    }
                    iInputLength = 0;
                }
            }
        }
    #elif defined DEVELOPERS_LOADER                                      // {23}
        switch (iAppState) {
        case STATE_ACTIVE:                                               // hook-up phase
            if (ucSerialInputMessage[0] == DEVELOPERS_ACK) {             // the PC is synchronised
                fnSendDevelopers(SerialPortID, DEVELOPERS_ACK);          // confirm synchronisation
                iAppState = STATE_DEVELOPERS_OPERATION;                  // operating state
                uTaskerStopTimer(OWN_TASK);                              // stop initial hook-up timer
            }
            break;
        case STATE_DEVELOPERS_OPERATION:
        #if defined DEVELOPERS_LOADER_CRC
            usCRC_Rx = usCRC_Tx = fnCRC16(0xffff, ucSerialInputMessage, 1); // reset CRC and process the command
        #endif
            switch (ucSerialInputMessage[0]) {
            case 'I':                                                    // ident command
                {
                    unsigned char *ptrBoardName = (unsigned char *)cProcessorName;
        #if defined DEVELOPERS_LOADER_READ
            #if defined DEVELOPERS_LOADER_PROTOCOL_VERSION_9
                #define PROTOCOL_VERSION_KINETIS   0x09
            #else
                #define PROTOCOL_VERSION_KINETIS   0x08
            #endif
            #if defined DEVELOPERS_LOADER_CRC
                    fnSendDevelopers(SerialPortID, (0xc0 | PROTOCOL_VERSION_KINETIS)); // return Kinetis version (with read support and CRC)
            #else
                    fnSendDevelopers(SerialPortID, (0x80 | PROTOCOL_VERSION_KINETIS)); // return Kinetis version (with read support but no CRC)
            #endif
        #else
            #if defined DEVELOPERS_LOADER_CRC
                    fnSendDevelopers(SerialPortID, (0x40 | PROTOCOL_VERSION_KINETIS)); // return Kinetis version (no read support but with CRC)
            #else
                    fnSendDevelopers(SerialPortID, (PROTOCOL_VERSION_KINETIS)); // return Kinetis version (no read support and no CRC)
            #endif
        #endif
                    #define FLASH_BLOCK_COUNT 1
        #if defined KINETIS_KE || defined KINETIS_KL
                    fnSendDevelopers(SerialPortID, (unsigned char)(0x014a >> 8)); // send ID from a K60
                    fnSendDevelopers(SerialPortID, (unsigned char)(0x014a));
        #else
                    fnSendDevelopers(SerialPortID, (unsigned char)(SIM_SDID >> 8));
                    fnSendDevelopers(SerialPortID, (unsigned char)(SIM_SDID));
        #endif
                    fnSendDevelopersLongWord(SerialPortID, FLASH_BLOCK_COUNT);
                    fnSendDevelopersLongWord(SerialPortID, _UTASKER_APP_START_);
                    fnSendDevelopersLongWord(SerialPortID, (unsigned long)UTASKER_APP_END);
                    fnSendDevelopersLongWord(SerialPortID, _UTASKER_APP_START_); // original vectors
                    fnSendDevelopersLongWord(SerialPortID, _UTASKER_APP_START_); // new vectors
                    fnSendDevelopersLongWord(SerialPortID, FLASH_GRANULARITY); // erase block size
                    fnSendDevelopersLongWord(SerialPortID, (RX_QUEUE_SIZE / 2)); // write block size
                    do {
                        fnSendDevelopers(SerialPortID, *ptrBoardName);   // send each character of board's name, including null terminator
                    } while (*ptrBoardName++ != 0);
        #if defined DEVELOPERS_LOADER_CRC
                    fnSendCRC(SerialPortID);                             // send the CRC
        #endif
                }
                break;
            case 'E':                                                    // erase
                iAppState = STATE_DEVELOPERS_ERASE;
        #if defined ADD_FILE_OBJECT_AFTER_LOADING
                fileObjInfo.ptrLastAddress = 0;
        #endif
                break;
            case 'W':                                                    // write
                iAppState = STATE_DEVELOPERS_WRITE;
                break;
        #if defined DEVELOPERS_LOADER_READ
            case 'R':
                iAppState = STATE_DEVELOPERS_READ;
                break;
        #endif
            case 'Q':                                                    // quit - immediately start the application
        #if defined DEVELOPERS_LOADER_CRC
                iAppState = STATE_DEVELOPERS_QUIT;                       // we check the CRC before accepting the quit command
        #else
                fnCommittProgramStart();
                RESET_PERIPHERALS();                                     // reset peripherals and disable interrupts ready for jumping to the application
                start_application(_UTASKER_APP_START_);                  // unconditional jump to application code
        #endif
                break;
            }
            break;
        case STATE_DEVELOPERS_ERASE:
        #if defined DEVELOPERS_LOADER_CRC
            if (++iInputLength >= 6)                                     // collect length of address plus CRC
        #else
            if (++iInputLength >= 4)                                     // collect length of address
        #endif
            {                                                            // if address has been collected
                unsigned long ulAddress = ((ucSerialInputMessage[0] << 24) | (ucSerialInputMessage[1] << 16) | (ucSerialInputMessage[2] << 8) | (ucSerialInputMessage[3]));
                iAppState = STATE_DEVELOPERS_OPERATION;                  // next operating state
                iInputLength = 0;
        #if defined DEVELOPERS_LOADER_CRC
                if (fnCRC16(usCRC_Rx, &ucSerialInputMessage[0], 6) != 0) { // process the address and CRC
                    fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_NAK);// return error
                    break;
                }
        #endif
                if (fnEraseFlashSector((unsigned char *)ulAddress, 1) != 0) { // erase the flash sector
                    fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_NAK);// return error
                    break;
                }
                fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_ACK);    // confirm erase
            }
            break;
        case STATE_DEVELOPERS_WRITE:
            if (++iInputLength >= 5) {                                   // if address has been collected
                ucDevelopersDataLength = ucSerialInputMessage[4];
        #if defined DEVELOPERS_LOADER_CRC
                ucDevelopersDataLength += 7;                             // expected length with CRC
        #else
                ucDevelopersDataLength += 5;                             // expected length
        #endif
                iAppState = STATE_DEVELOPERS_WRITE_DATA;
            }
            break;
        case STATE_DEVELOPERS_WRITE_DATA:
            if (++iInputLength >= ucDevelopersDataLength) {              // if all data has been collected
                unsigned long ulAddress = ((ucSerialInputMessage[0] << 24) | (ucSerialInputMessage[1] << 16) | (ucSerialInputMessage[2] << 8) | (ucSerialInputMessage[3]));
                iAppState = STATE_DEVELOPERS_OPERATION;                  // operating state
                iInputLength = 0;
        #if defined DEVELOPERS_LOADER_CRC
                if (fnCRC16(usCRC_Rx, &ucSerialInputMessage[0], ucDevelopersDataLength) != 0) { // process the CRC of the content
                    fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_NAK);// return error
                    break;
                }
                ucDevelopersDataLength -= 7;
        #else
                ucDevelopersDataLength -= 5;
        #endif
                if (ulAddress == _UTASKER_APP_START_) {
                    uMemcpy(ucCodeStart, &ucSerialInputMessage[5], CODE_START_SIZE); // backup start of code until complete code received
                    fnWriteBytesFlash((unsigned char *)(ulAddress + CODE_START_SIZE), (&ucSerialInputMessage[5] + CODE_START_SIZE), (MAX_FILE_LENGTH)(ucDevelopersDataLength - CODE_START_SIZE));
                }
                else {
                    fnWriteBytesFlash((unsigned char *)ulAddress, &ucSerialInputMessage[5], ucDevelopersDataLength); // save the CODE_START_SIZE block
                }
                fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_ACK);    // confirm programming
        #if defined ADD_FILE_OBJECT_AFTER_LOADING
                if ((unsigned char *)(ulAddress + ucDevelopersDataLength) > fileObjInfo.ptrLastAddress) { //monitor the size of the software data
                    fileObjInfo.ptrLastAddress = (unsigned char *)(ulAddress + ucDevelopersDataLength);
                }
        #endif
            }
            break;
        #if defined DEVELOPERS_LOADER_CRC
        case STATE_DEVELOPERS_QUIT:
            if (++iInputLength >= 2) {                                   // CRC collected
                iAppState = STATE_DEVELOPERS_OPERATION;                  // operating state
                iInputLength = 0;
                if (fnCRC16(usCRC_Rx, &ucSerialInputMessage[0], 2) != 0) { // process the CRC of the content
                    fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_NAK);// return error (note that the PC program doesn't actually wait for a response to the quit command)
                    break;
                }
              //fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_ACK);    // return OK (not required)
                fnCommittProgramStart();
                RESET_PERIPHERALS();                                     // reset peripherals and disable interrupts ready for jumping to the application
                start_application(_UTASKER_APP_START_);                  // unconditional jump to application code
            }
            break;
        #endif
        #if defined DEVELOPERS_LOADER_READ
        case STATE_DEVELOPERS_READ:                                      // the read is called after each write has been made in order to check that the data was correctly programmed
            #if defined DEVELOPERS_LOADER_CRC
            if (++iInputLength >= 7)                                     // length including CRC
            #else
            if (++iInputLength >= 5)
            #endif
            {                                                            // if address and length have been collected
                unsigned char ucProgramContent;
                unsigned long ulAddress = ((ucSerialInputMessage[0] << 24) | (ucSerialInputMessage[1] << 16) | (ucSerialInputMessage[2] << 8) | (ucSerialInputMessage[3]));
                int iLength = ucSerialInputMessage[4];
                iAppState = STATE_DEVELOPERS_OPERATION;                  // operating state
                iInputLength = 0;
            #if defined DEVELOPERS_LOADER_CRC
                if (fnCRC16(usCRC_Rx, &ucSerialInputMessage[0], 7) != 0) { // process the CRC of the content
                    fnSendDevelopersAckNak(SerialPortID, DEVELOPERS_NAK);// return error
                    break;
                }
                usCRC_Tx = 0xffff;                                       // the tx CRC of the returned read data is an exception because its calculation doesn't include priming with the command - re-seed the CRC for the data content calculation
            #endif
                if (ulAddress == _UTASKER_APP_START_) {                  // a read is being made from the start of the application code
                    int iStartCodeLength;
                    for (iStartCodeLength = 0; iStartCodeLength < CODE_START_SIZE; iStartCodeLength++) {
                        fnSendDevelopers(SerialPortID, ucCodeStart[iStartCodeLength]); // return the data from program start buffer
                    }
                    iLength -= CODE_START_SIZE;
                    ulAddress += CODE_START_SIZE;
                }
            #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
                else if (iLength != (RX_QUEUE_SIZE / 2)) {               // if a read is requested that is not the standard block length it means that it is the final read (complete program has been received)
                    fnWriteBytesFlash(0, 0, 0);                          // close any outstanding FLASH buffer
                }
            #endif
                while (iLength-- != 0) {
                    fnGetParsFile((unsigned char *)ulAddress++, &ucProgramContent, 1); // collect the program content (supports also SPI Flash based content)
                    fnSendDevelopers(SerialPortID, ucProgramContent);    // return the data
                }
            #if defined DEVELOPERS_LOADER_CRC
                fnSendCRC(SerialPortID);                                 // send the CRC
            #endif
            }
            break;
        #endif
        }
    #else
        switch (iAppState) {
        case STATE_ACTIVE:
            if (ucSerialInputMessage[0] == '\n') {
                continue;
            }
            if (ucSerialInputMessage[iInputLength] == '\r') {
                int iLastLength = iInputLength;
                iInputLength = 0;
                if (iLastLength == 2) {
                    if ((ucSerialInputMessage[0] == 'd') || (ucSerialInputMessage[0] == 'D')) {
        #if defined SPI_SW_UPLOAD  
                        if ((ucSerialInputMessage[1] == 'u') || (ucSerialInputMessage[1] == 'U'))
        #else
                        if ((ucSerialInputMessage[1] == 'c') || (ucSerialInputMessage[1] == 'C'))
        #endif
                        {
        #if defined SPI_SW_UPLOAD  
                            fnDebugMsg("\r\nDelete area [y/n] ? > ");
        #else
                            fnDebugMsg("\r\nDelete code [y/n] ? > ");
        #endif
                            iAppState = STATE_QUESTION_DELETE;
                            break;
                        }
                    }
        #if !defined REMOVE_SREC_LOADING                                 // {17}
                    else if ((ucSerialInputMessage[0] == 'l') || (ucSerialInputMessage[0] == 'L')) {
                        if ((ucSerialInputMessage[1] == 'd') || (ucSerialInputMessage[1] == 'D')) {
            #if defined SREC_SECURE_LOADER                               // {36}
                            fnPrepareDecrypt(0);                         // prepare AES-256 decryption key and initial vector
            #endif
            #if defined EXCLUSIVE_INTEL_HEX_MODE                         // {29}
                            fnDebugMsg("\r\nPlease start iHEX download: ");
            #elif defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
                            fnDebugMsg("\r\nPlease start S-REC/iHex download: ");
            #else
                            fnDebugMsg("\r\nPlease start S-REC download: ");
            #endif
            #if defined _STR91XF
                            fnUnprotectFile();                           // unprotect all blocks to be deleted
            #endif
                            iAppState = STATE_LOADING;
                            break;
                        }
                    }
        #endif
        #if defined MEMORY_SWAP
                    if ((ucSerialInputMessage[0] == 's') || (ucSerialInputMessage[0] == 'S')) {
                        if ((ucSerialInputMessage[1] == 'w') || (ucSerialInputMessage[1] == 'W'))
                        {
                            fnDebugMsg("\r\nSwap memory [y/n] ? > ");
                            iAppState = STATE_QUESTION_SWAP;
                            break;
                        }
                    }
        #else
                    else if ((ucSerialInputMessage[0] == 'g') || (ucSerialInputMessage[0] == 'G')) {
                        if ((ucSerialInputMessage[1] == 'o') || (ucSerialInputMessage[1] == 'O')) {
            #if defined iMX_FALLBACK_SERIAL_LOADER
                            fnDebugMsg("\r\nStarting Serial Loader\r\n\n");
            #else
                            fnDebugMsg("\r\nStarting Application\r\n\n");
            #endif
                            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.2 * SEC), T_GO_TO_APP);
                            iAppState = STATE_RESETTING;
                            break;
                        }
            #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
                        else if ((ucSerialInputMessage[1] == 'd') || (ucSerialInputMessage[1] == 'D')) {
                            fnDebugMsg("\r\nStarting delayed\r\n\n");
                            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.2 * SEC), T_GO_TO_APP);
                            iAppState = (STATE_RESETTING | STATE_PAUSING);
                        }
            #endif
                    }
        #endif
        #if defined MASS_ERASE
                    else if ((ucSerialInputMessage[0] == 'm') || (ucSerialInputMessage[0] == 'M')) {
                        if ((ucSerialInputMessage[1] == 'e') || (ucSerialInputMessage[1] == 'E')) {
                            fnDebugMsg("\r\nMass erase [y/n] ? > ");
                            iAppState = STATE_QUESTION_MASS_DELETE;
                            break;
                        }
                    }
        #endif
                    else if ((ucSerialInputMessage[0] == 'b') || (ucSerialInputMessage[0] == 'B')) {
        #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
                        if ((ucSerialInputMessage[1] == 't') || (ucSerialInputMessage[1] == 'T')) {
                            *BOOT_MAIL_BOX = (RESET_TO_FALLBACK_LOADER | RESET_NO_COUNT_FLAG); // put a message in the boot mailbox and reset the board (if the serial loader is installed and uses the mailbox it will cause the loader to start)
                            fnResetBoard();
                        }
        #endif
                        if ((ucSerialInputMessage[1] == 'c') || (ucSerialInputMessage[1] == 'C')) {
                            fnPerformBlankCheck();
                            break;
                        }
        #if defined UNLOCK_BACKDOOR_KEY
                        else if ((ucSerialInputMessage[1] == 'k') || (ucSerialInputMessage[1] == 'K')) {
                            fnDebugMsg("\r\nEnter 8 byte hex key");
                            iAppState = STATE_ENTER_KEY;
                            break;
                        }
        #endif
                    }
                }
                fnDebugMsg(" ??\r\n> ");
            }
            else if (ucSerialInputMessage[iInputLength] == '?') {        // interpret question mark as help request
        #if defined RX_UART_TO_HOLD_LOADER
                uTaskerStopTimer(OWN_TASK);                              // stop the hook-up timer
        #endif
                iInputLength = 0;
                fnPrintScreen();
            }
            else {
                fnWrite(SerialPortID, &ucSerialInputMessage[iInputLength], 1); // echo input back
                if (ucSerialInputMessage[iInputLength++] == DELETE_KEY) {
                    if (iInputLength < 2) {
                        iInputLength = 0;
                    }
                    else {
                        iInputLength -= 2;
                    }
                }
            }
            break;
        #if defined UNLOCK_BACKDOOR_KEY
        case STATE_ENTER_KEY:
            _EXCEPTION("To complete"); // collect key for comparison and execute fnBackdoorUnlock()
            break;
        #endif
        #if defined MASS_ERASE
        case STATE_FINAL_MASS_DELETE:
        case STATE_QUESTION_MASS_DELETE:
        #endif
        #if defined MEMORY_SWAP
        case STATE_QUESTION_SWAP:
        #endif
        case STATE_QUESTION_DELETE:
            if ((ucSerialInputMessage[0] == 'y') || (ucSerialInputMessage[0] == 'Y')) {
        #if defined MASS_ERASE
                if (STATE_QUESTION_MASS_DELETE == iAppState) {
                    fnDebugMsg("\r\n!! Destroy FLASH and Loader - confirm [y/n] ? > ");
                    iAppState = STATE_FINAL_MASS_DELETE;
                    break;
                }
                else if(STATE_FINAL_MASS_DELETE == iAppState) {
                    fnMassEraseFlash();                                  // this will completely erase FLASH content, unlocking any security. The code will no longer run after this command
                    break;
                }
        #endif
        #if defined MEMORY_SWAP
                if (STATE_QUESTION_SWAP == iAppState) {                  // swapping the memory causes a watchdog reset and then the new code starts in the alternative memory
                    fnHandleSwap(0);                                     // command a swap of the flash blocks
                    break;
                }
        #endif
        #if defined SPI_SW_UPLOAD
                fnDebugMsg("\r\n\nDeleting area...");
                if (fnEraseFlashSector((unsigned char *)(SIZE_OF_FLASH + UPLOAD_OFFSET), (MAX_FILE_LENGTH)(UTASKER_APP_END - (unsigned char *)UTASKER_APP_START)) == 0) // delete space in SPI FLASH
        #else
                fnDebugMsg("\r\n\nDeleting code...");
            #if defined MEMORY_SWAP && defined ADD_FILE_OBJECT_AFTER_LOADING
                fnEraseFlashSector((unsigned char *)(UTASKER_APP_START + (SIZE_OF_FLASH/2) - (2 * FLASH_GRANULARITY)), 0); // delete the firmware's file object
            #endif
                if (fnEraseFlashSector((unsigned char *)UTASKER_APP_START, (MAX_FILE_LENGTH)(UTASKER_APP_END - (unsigned char *)UTASKER_APP_START)) == 0) // delete application space {2}
        #endif
                {
                    fnDebugMsg("successful\r\n> ");
                }
                else {
                    fnDebugMsg("failed!!\r\n> ");
                }
            }
            else if ((ucSerialInputMessage[0] != '\n') && (ucSerialInputMessage[0] != '\r')) {
                fnDebugMsg("\r\nAborted!!\r\n> ");
            }
            iAppState = STATE_ACTIVE;
            break;
        #if !defined REMOVE_SREC_LOADING                                 // {17}
        case STATE_LOADING:
            if ((0 == iInputLength) && ((ucSerialInputMessage[0] == '\n') || (ucSerialInputMessage[0] == '\r'))) { // {35} ignore CR or LF at the start of a line
                continue;
            }
            if (ucSerialInputMessage[iInputLength] == ASCII_ETX) {       // ctrl + C to abort before, or during load (0x03)
                fnDebugMsg(" Aborted!\r\n");
                iInputLength = 0;
                fnPrintScreen();
                iAppState = STATE_ACTIVE;
                break;
            }
            if ((ucSerialInputMessage[iInputLength] == '\r') || (ucSerialInputMessage[iInputLength] == '\n')) { // {35} line received (LF or CR)
                iInputLength++;
            #if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
                switch (fnHandleRecord(ucSerialInputMessage, (ucSerialInputMessage + iInputLength), SERIAL_LOADING_IN_OPERATION))
            #else
                switch (fnHandleRecord(ucSerialInputMessage, (ucSerialInputMessage + iInputLength)))
            #endif
                {
                case CORRUPTED_SREC:
                    fnDebugMsg("\r\n\nRecord-error!! (Ctrl+r to reset)"); // this happens if a file is received that is not in the expetced SREC format
                    iAppState = STATE_ERROR;
                    break;
                case SREC_CS_ERROR:
                    fnDebugMsg("\r\n\nCS-error!! (Ctrl+r to reset)");    // this will occur if the CRC of a single SREC line is incorrect. This could happen if a character were not received (lost) or if a bit were incorrect in a character. Therefore it could happen in case of either a problem with the receiver keeping up with the data or when there is a transmission/reception error (such as due to noise or other interference)
                    iAppState = STATE_ERROR;
                    break;
                case PROGRAMMING_ERROR:
                    fnDebugMsg("\r\n\nProgramming-error!! (Ctrl+r to reset)"); // this occurs when it was not possible to program the data to flash - this error could occur if there were a problem with the flash driver or the memory to be written were protected, or in case of a HW problem with the flash in the chip stopping the programming operation from working
                    iAppState = STATE_ERROR;
                    break;
                case PROGRAMMING_COMPLETE:
            #if defined MEMORY_SWAP
                    fnDebugMsg("\r\n\nTerminated - swap if required\r\n");
                    iAppState = STATE_ACTIVE;
            #else
                    fnDebugMsg("\r\n\nTerminated - restarting...");
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1 * SEC), T_RESET);
                    iAppState = STATE_RESETTING;
            #endif
                    break;
                case INVALID_APPLICATION_LOCATION:
                    fnDebugMsg("!");                                     // this occurs when the valid SREC line received has a destination address outside the area defined for the application
                    break;
            #if defined INTERMEDIATE_PROG_BUFFER
                case STOP_FLOW_CONTROL:
                    fnDebugMsg("\x13*");                                 // send an XOFF to stop the download process for a short time
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.2 * SEC), T_COMMIT_BUFFER); // allow short time for flow control to take effect, after which the buffer will be programmed to flash
                    iInputLength = 0;
                    iAppState = STATE_PAUSING;
                    return;                                              // {15} don't extract any further input data until programming has completed
                case INVALID_SREC_HOLE:
                    fnDebugMsg("\r\n\nCode hole!! (Ctrl+r to reset)");
                    iAppState = STATE_ERROR;
                    break;
            #endif
                default:
                    fnDebugMsg(".");                                     // display REC line
                    break;
                }
                iInputLength = 0;
            }
            else {                                                       // {35}
                iInputLength++;
            }
            break;
        case STATE_ERROR:
            if (ucSerialInputMessage[iInputLength] == ASCII_DC2) {       // CTRL + R (0x12)
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.2 * SEC), T_RESET);
                iAppState = STATE_RESETTING;
            }
            break;
        #endif
        }
    #endif
        if (iInputLength >= RX_QUEUE_SIZE) {                             // {7} protect against buffer overflow
            iInputLength = 0;
        }
    }
#endif
}

#if defined BLAZE_K22
static void fnLoaderForced(int iNoFirmware)
{
    GLCD_TEXT_POSITION text_pos;
    uTaskerStopTimer(OWN_TASK);
    uTaskerStateChange(TASK_USB, UTASKER_ACTIVATE);                      // start the USB task to allow loading new code
    text_pos.ucFont = FONT_NINE_DOT;
    text_pos.usX = 5;
    if (iNoFirmware != 0) {
        text_pos.ucMode = (PAINT_LIGHT);
        text_pos.usY = 19;
        fnDoLCD_text(&text_pos, "No Firmware");
    }
    text_pos.ucMode = (PAINT_LIGHT | REDRAW);
    text_pos.usY = 5;
    fnDoLCD_text(&text_pos, "USB enabled");
    BACK_LIGHT_MAX_INTENSITY();
}
#endif

#if defined MEMORY_SWAP
extern void fnHandleSwap(int iCheck)
{
    int iResut = fnSwapMemory(iCheck);
    fnDebugMsg("\r\nSwap ");
    switch (iResut) {
    case SWAP_STATE_UNINITIALISED:
        fnDebugMsg("uninitialised");
        break;
    case SWAP_STATE_SWAPPED:                                             // expected only after commanding a swap
        fnDebugMsg("OK");
        if (iCheck == 0) {
            fnDebugMsg(" - resetting...\r\n");
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1 * SEC), T_RESET); // reset after a short delay
        }
        break;
    case SWAP_STATE_USING_1:
        fnDebugMsg("using 1");
        break;
    case SWAP_STATE_USING_0:
        fnDebugMsg("using 0");
        break;
    case SWAP_COMMAND_FAILURE:
        fnDebugMsg("cmd error!");
        break;
    case SWAP_ERASE_FAILURE:
        fnDebugMsg("erase error!");
        break;
    default:
        fnDebugMsg("??");
        break;
    }
}
#endif

#if defined SERIAL_INTERFACE && defined DEVELOPERS_LOADER                // {23}
static void fnSendDevelopers(QUEUE_HANDLE SerialPortID, unsigned char ucByte)
{
    fnWrite(SerialPortID, &ucByte, 1);
    #if defined DEVELOPERS_LOADER_CRC
    usCRC_Tx = fnCRC16(usCRC_Tx, &ucByte, 1);                            // process the transmitted byte
    #endif
}

static void fnSendDevelopersAckNak(QUEUE_HANDLE SerialPortID, unsigned char ucByte)
{
    #if defined DEVELOPERS_LOADER_CRC
    static const unsigned char ucAckCRC[] = {DEVELOPERS_ACK, 0xcf, 0x63}; // ACK with its fixed CRC value
    static const unsigned char ucNakCRC[] = {DEVELOPERS_NAK, 0xd1, 0x93}; // NAK with its fixed CRC value
    if (ucByte == DEVELOPERS_ACK) {
        fnWrite(SerialPortID, (unsigned char *)ucAckCRC, 3);
    }
    else {                                                               // assume NAK
        fnWrite(SerialPortID, (unsigned char *)ucNakCRC, 3);
    }
    #else
    fnWrite(SerialPortID, &ucByte, 1);                                   // send ACK/NAK
    #endif
}

    #if defined DEVELOPERS_LOADER_CRC
static void fnSendCRC(QUEUE_HANDLE SerialPortID)
{
    unsigned short usTxCRC = usCRC_Tx;                                   // the CRC value to be transmitted
    fnSendDevelopers(SerialPortID, (unsigned char)(usTxCRC >> 8));
    fnSendDevelopers(SerialPortID, (unsigned char)(usTxCRC));
}
    #endif

static void fnSendDevelopersLongWord(QUEUE_HANDLE SerialPortID, unsigned long ulLongWord)
{
    fnSendDevelopers(SerialPortID, (unsigned char)(ulLongWord >> 24));
    fnSendDevelopers(SerialPortID, (unsigned char)(ulLongWord >> 16));
    fnSendDevelopers(SerialPortID, (unsigned char)(ulLongWord >> 8));
    fnSendDevelopers(SerialPortID, (unsigned char)(ulLongWord));
}
#endif

#if ((defined SERIAL_INTERFACE || defined USE_USB_CDC) && defined DEVELOPERS_LOADER) || defined I2C_INTERFACE

// This routine allows a buffers content to be checked against a single byte value (often for all 0x00 or all 0xff)
//
static void *fnMemValue(void *ptrCheck, unsigned char ucValue, size_t Size)
{
    unsigned char *ptr = (unsigned char *)ptrCheck;
    while (Size-- != 0) {                                                // for each byte
        if (*ptr != ucValue) {
            return (void *)ptr;                                          // return pointer to the value that was not equal
        }
        ptr++;
    }
    return 0;                                                            // content is completely equal to the check value
}

static void fnCommittProgramStart(void)
{
    if (fnMemValue(ucCodeStart, 0xff, CODE_START_SIZE) == 0) {           // if the start of code is not waiting to be committed we ignore this call
        return;
    }
    fnWriteBytesFlash((unsigned char *)_UTASKER_APP_START_, ucCodeStart, CODE_START_SIZE); // complete programming
    uMemset(ucCodeStart, 0xff, CODE_START_SIZE);                         // invalidate code start after committing to flash
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
    fileObjInfo.ptrShortFileName = "DEVELOPES19";                        // define a name for the file (with default time/date)
    fnAddSREC_file(&fileObjInfo);
    #endif
}
#endif

#if ((defined SERIAL_INTERFACE && !defined USE_MODBUS) || defined USE_USB_CDC) && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER
static void fnPrintScreen(void)
{
    #if defined SHOW_APP_DETAILS                                         // {6}
        #if defined iMX_FALLBACK_SERIAL_LOADER
    fnDebugMsg("\r\n\n uTasker Fall-back Loader ");
        #else
    fnDebugMsg("\r\n\n uTasker Serial Loader ");
        #endif
    fnDebugMsg(SOFTWARE_VERSION);
    #if defined iMX_FALLBACK_SERIAL_LOADER
    fnDebugMsg("\r\n==============================\r\n");
    #else
    fnDebugMsg("\r\n===========================\r\n");
    #endif
        #if defined MEMORY_SWAP
    fnDebugMsg("[Swap size ");
    fnDebugHex((unsigned long)(SIZE_OF_FLASH/2), (WITH_LEADIN | sizeof(unsigned long)));
        #else
    fnDebugMsg("[");                                                     // display the application area
    fnDebugHex((unsigned long)_UTASKER_APP_START_, (WITH_LEADIN | sizeof(unsigned long)));
    fnDebugMsg("/");
    fnDebugHex((unsigned long)(UTASKER_APP_END - 1), (WITH_LEADIN | sizeof(unsigned long)));
        #endif
    fnDebugMsg("]\r\n");
        #if defined SPI_SW_UPLOAD
    fnDebugMsg("[");                                                     // display the area where the uploaded data will be stored
    fnDebugHex((unsigned long)(SIZE_OF_FLASH + UPLOAD_OFFSET), (WITH_LEADIN | sizeof(unsigned long)));
    fnDebugMsg("/");
    fnDebugHex((unsigned long)(((SIZE_OF_FLASH + UPLOAD_OFFSET) + (UTASKER_APP_END - UTASKER_APP_START)) - 1), (WITH_LEADIN | sizeof(unsigned long)));
    fnDebugMsg("]\r\n");
        #endif
    #else
    fnDebugMsg("\r\n\nuTasker Serial Loader\r\n");
    fnDebugMsg(      "=====================\r\n");
    #endif
    #if defined MASS_ERASE
    fnDebugMsg("me = mass erase\r\n");
    #endif
    fnDebugMsg("bc = blank check\r\n");
    #if defined SPI_SW_UPLOAD
    fnDebugMsg("du = delete upload area\r\n");
    #else
    fnDebugMsg("dc = delete code\r\n");
    #endif
    #if !defined REMOVE_SREC_LOADING                                     // {17}
    fnDebugMsg("ld = start load\r\n");
    #endif
    #if defined UNLOCK_BACKDOOR_KEY
    fnDebugMsg("bk = backdoor key\r\n");
    #endif
    #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
    fnDebugMsg("bt = start boot loader\r\n");
    #endif
    #if defined MEMORY_SWAP
    fnDebugMsg("sw = swap memory\r\n");
    #else
        #if defined iMX_FALLBACK_SERIAL_LOADER
    fnDebugMsg("go = start serial loader\r\n");
        #else
    fnDebugMsg("go = start application\r\n> ");
        #endif
    #endif
}

#if defined SHOW_APP_DETAILS && defined _REMOVE_FORMATTED_OUTPUT && !defined _WINDOWS // {6} copy of library code which is otherwise not linked

// Write hex value to buffer
//
extern CHAR *fnBufferHex(unsigned long ulValue, unsigned char ucLen, CHAR *pBuf)
{
    unsigned char ucTemp;
    int iTerminate = 0;
    int iCapitals = 0;
    int iCRLF = 0;

    if (ucLen & WITH_SPACE) {
        *pBuf++ = ' ';
    }

    if (ucLen & WITH_LEADIN) {
        *pBuf++ = '0';
        *pBuf++ = 'x';
    }
    if (!(ucLen & NO_TERMINATOR)) {
       iTerminate = 1;
    }
    if (ucLen & CODE_CAPITALS) {
        iCapitals = 1;
    }
    if (ucLen & WITH_CR_LF) {
        iCRLF = 1;
    }
    ucLen &= 0x7;                                                        // mask out possible sizes

    if (ucLen > 4) {                                                     // limit size
        ucLen = 4;                                                       // to length of a long
    }

    ucLen *= 2;                                                          // 2 bytes per byte

    while (ucLen--) {
        ucTemp = (unsigned char)((ulValue >> (ucLen * 4)) & 0x0f);
        ucTemp += '0';
        if (ucTemp > '9') {
            if (iCapitals != 0) {
                ucTemp += ('A' - '9' - 1);
            }
            else {
                ucTemp += ('a' - '9' - 1);
            }
        }
        *pBuf++ = (char)ucTemp;
    }
    if (iCRLF != 0) {
        *pBuf++ = '\r';
        *pBuf++ = '\n';
    }
    if (iTerminate != 0) {
    #if defined STRING_OPTIMISATION
        *pBuf = 0;                                                       // return pointer to null termination to simplify concatination
    #else
        *pBuf++ = 0;
    #endif
    }
    return pBuf;
}

// This converts a value to an ascii hex string and sends it to the debug output
// If the length is called with WITH_LEADIN set, it will add 0x to the beginning
//
extern void fnDebugHex(unsigned long ulValue, unsigned char uLen)        // display hex value
{
    #define MAX_LONG (5 + 8 + 1)                                         // 8 bytes plus terminator as well as optional spaces and line feeds
    CHAR cVal[MAX_LONG];                                                 // max display is a long type

    fnBufferHex(ulValue, uLen, cVal);

    fnDebugMsg(cVal);
}
#endif
#endif
#endif

#if defined KBOOT_SECURE_LOADER || defined USB_MSD_DEVICE_SECURE_LOADER || defined SDCARD_SECURE_LOADER || defined SREC_SECURE_LOADER // {34}
typedef struct stALIGNED_KEY
{
    unsigned long  ulLength;                                             // long word to ensure following key alignment
    unsigned char  ucKey[32];                                            // key aligned
} ALIGNED_KEY;

#if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
extern void fnPrepareNonce(unsigned char initVector[32])
{
    // The nonce is constructed from the start address concatonated with the XOR of the first and second 32 bits and the concatonated with the first two 32 bit values
    //
    static unsigned char ucNonce[16] = {0};
    static unsigned long ulAddress = 0;
    if (initVector != 0) {
        unsigned long ulCounter0;
        unsigned long ulCounter1;
        unsigned long ulCounterXOR;
    #if defined BEE_BLOCK
        ulCounterXOR = initVector[15];
        ulCounterXOR <<= 8;
        ulCounterXOR |= initVector[14];
        ulCounterXOR <<= 8;
        ulCounterXOR |= initVector[13];
        ulCounterXOR <<= 8;
        ulCounterXOR |= initVector[12];
        ulCounter0 = initVector[7];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[6];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[5];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[4];
        ulCounter1 = initVector[11];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[10];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[9];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[8];
        ucNonce[11] = (unsigned char)ulCounter0;                          // concatonated with the two counter registers
        ucNonce[10] = (unsigned char)(ulCounter0 >> 8);
        ucNonce[9] = (unsigned char)(ulCounter0 >> 16);
        ucNonce[8] = (unsigned char)(ulCounter0 >> 24);

        ucNonce[7] = (unsigned char)ulCounter1;                          // concatonated with the two counter registers
        ucNonce[6] = (unsigned char)(ulCounter1 >> 8);
        ucNonce[5] = (unsigned char)(ulCounter1 >> 16);
        ucNonce[4] = (unsigned char)(ulCounter1 >> 24);

        ucNonce[3] = (unsigned char)ulCounterXOR;                        // concatonated with XOR of the two counters
        ucNonce[2] = (unsigned char)(ulCounterXOR >> 8);
        ucNonce[1] = (unsigned char)(ulCounterXOR >> 16);
        ucNonce[0] = (unsigned char)(ulCounterXOR >> 24);
    #else
        ulCounter0 = initVector[0];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[1];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[2];
        ulCounter0 <<= 8;
        ulCounter0 |= initVector[3];
        ulCounter1 = initVector[4];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[5];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[6];
        ulCounter1 <<= 8;
        ulCounter1 |= initVector[7];
        ulCounterXOR = (ulCounter0 ^ ulCounter1);

        ucNonce[3] = (unsigned char)ulCounter0;                          // concatonated with the two counter registers
        ucNonce[2] = (unsigned char)(ulCounter0 >> 8);
        ucNonce[1] = (unsigned char)(ulCounter0 >> 16);
        ucNonce[0] = (unsigned char)(ulCounter0 >> 24);

        ucNonce[7] = (unsigned char)ulCounter1;                          // concatonated with the two counter registers
        ucNonce[6] = (unsigned char)(ulCounter1 >> 8);
        ucNonce[5] = (unsigned char)(ulCounter1 >> 16);
        ucNonce[4] = (unsigned char)(ulCounter1 >> 24);

        ucNonce[11] = (unsigned char)ulCounterXOR;                       // concatonated with XOR of the two counters
        ucNonce[10] = (unsigned char)(ulCounterXOR >> 8);
        ucNonce[9] = (unsigned char)(ulCounterXOR >> 16);
        ucNonce[8] = (unsigned char)(ulCounterXOR >> 24);
    #endif

        ulAddress = (iMX_APPLICATION_LOCATION + iMX_FILE_OBJECT_SIZE + 16); // start address
    }
    #if defined BEE_BLOCK
    ucNonce[15] = (unsigned char)(ulAddress >> 4);                       // concatonated with the QSPI address of 16 bytes to be decrypted
    ucNonce[14] = (unsigned char)(ulAddress >> 12);
    ucNonce[13] = (unsigned char)(ulAddress >> 20);
    ucNonce[12] = (unsigned char)(ulAddress >> 28);
    #else
    ucNonce[15] = (unsigned char)ulAddress;                              // concatonated with the QSPI address of 16 bytes to be decrypted
    ucNonce[14] = (unsigned char)(ulAddress >> 8);
    ucNonce[13] = (unsigned char)(ulAddress >> 16);
    ucNonce[12] = (unsigned char)(ulAddress >> 24);
    #endif

    ulAddress += 16;                                                     // increment for next use

    fnAES_Init(AES_COMMAND_AES_PRIME_IV, ucNonce, 0);                    // prime the initial vector
}
#endif


extern void fnPrepareDecrypt(int iEncrypt)
{
    ALIGNED_KEY decryptKey = {0, {0}};
    int iInitKey = (AES_COMMAND_AES_SET_KEY_DECRYPT | AES_COMMAND_AES_RESET_IV); // assume decrypt key initialisation with IV set to zero
    #if defined PRIME_AES256_INITIAL_VECTOR
    unsigned char initVector[32];
    int iVectorLength = (sizeof(initial_vector) - 1);
    #endif
    int iKeyLength = 256;
    decryptKey.ulLength = (sizeof(decrypt_key) - 1);
    uMemcpy(decryptKey.ucKey, decrypt_key, (sizeof(decrypt_key) - 1));
    while (decryptKey.ulLength < 32) {
        decryptKey.ucKey[decryptKey.ulLength++] = 'X';
    }
    #if defined SDCARD_SUPPORT && defined SDCARD_SECURE_LOADER           // the SD card loader also encrypts flash content to see whether it matches the encrypted card content
    if (iEncrypt == 1) {                                                 // set encyrpt key instead of decrypt key
        iInitKey = (AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_RESET_IV);
    }
    #endif
    #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
    if (iEncrypt == 2) {
        #if defined BEE_BLOCK
        int x;
        unsigned char ucTemp;
        for (x = 0; x < 8; x++) {
            ucTemp = decryptKey.ucKey[x];
            decryptKey.ucKey[x] = decryptKey.ucKey[15 - x];
            decryptKey.ucKey[15 - x] = ucTemp;
        }
        #endif
        iKeyLength = 128;                                                // AES128 instead of AES256
        iInitKey = (AES_COMMAND_AES_SET_KEY_ENCRYPT | AES_COMMAND_AES_RESET_IV); // CTR mode uses the same key initialisation for encryption and decryption
    }
    #endif
    fnAES_Init(iInitKey, decryptKey.ucKey, iKeyLength);                  // register the key
    #if defined PRIME_AES256_INITIAL_VECTOR
    uMemcpy(initVector, initial_vector, (sizeof(initial_vector) - 1));
    while (iVectorLength < 16) {
        initVector[iVectorLength++] = 'X';
    }
        #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
    if (iEncrypt == 2) {
        fnPrepareNonce(initVector);
        return;
    }
        #endif
    fnAES_Init(AES_COMMAND_AES_PRIME_IV, initVector, 0);                 // prime the initial vector
    #endif
}
#endif

#if (defined KBOOT_SECURE_LOADER && ((defined HID_LOADER && defined USB_INTERFACE) || (defined KBOOT_LOADER && defined SERIAL_INTERFACE))) || defined SREC_SECURE_LOADER // {34}
#define SECURE_BLOCK_LENGTH 512
typedef struct stALIGNED_DATA_BUFFER
{
    unsigned char *ptrDestinationAddress;
    unsigned long  ulLength;                                             // long word to ensure following data alignment
    unsigned char  ucDataInput[SECURE_BLOCK_LENGTH];                     // data aligned
} ALIGNED_DATA_BUFFER;


// It is assumed that the content is received in a linear fashion (addresses incrementing with each buffer)
//
static int fnWriteBytesSecure(unsigned char *ptrFlashAddress, unsigned char *ptrData, unsigned long ulBuff_length, int iCollectFirstLine)
{
    static ALIGNED_DATA_BUFFER encryptBuffer = { 0,0,{0} };
    unsigned long usThisLength;
    if (ptrData == 0) {                                                  // final block
        if (encryptBuffer.ulLength != 0) {
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)encryptBuffer.ucDataInput, encryptBuffer.ucDataInput, encryptBuffer.ulLength); // decrypt the buffer content
            fnWriteBytesFlash(encryptBuffer.ptrDestinationAddress, encryptBuffer.ucDataInput, encryptBuffer.ulLength); // program flash
        }
    #if  defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
        fnWriteBytesFlash(0, 0, 0);                                      // close any outstanding FLASH buffer
    #endif
    }
    if (encryptBuffer.ulLength == 0) {                                   // block starting
        encryptBuffer.ptrDestinationAddress = ptrFlashAddress;           // the address that the block is destined for
    }
    while (ulBuff_length != 0) {
        if (ulBuff_length <= (SECURE_BLOCK_LENGTH - encryptBuffer.ulLength)) {
            usThisLength = (unsigned short)ulBuff_length;
        }
        else {
            usThisLength = (unsigned short)(SECURE_BLOCK_LENGTH - encryptBuffer.ulLength);
        }
        uMemcpy(&encryptBuffer.ucDataInput[encryptBuffer.ulLength], ptrData, usThisLength);
        ptrData += usThisLength;
        ulBuff_length -= usThisLength;
        encryptBuffer.ulLength += usThisLength;
        if (encryptBuffer.ulLength >= SECURE_BLOCK_LENGTH) {             // if a complete buffer has been prepared
            MAX_FILE_LENGTH save_length = SECURE_BLOCK_LENGTH;
            unsigned char *ptrDestination = encryptBuffer.ptrDestinationAddress;
            unsigned char *ptrSource = encryptBuffer.ucDataInput;
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)encryptBuffer.ucDataInput, encryptBuffer.ucDataInput, SECURE_BLOCK_LENGTH); // decrypt the buffer content
            if ((iCollectFirstLine != 0) && (encryptBuffer.ptrDestinationAddress == (unsigned char *)_UTASKER_APP_START_)) { // {38} catch first bytes of application
                uMemcpy(ucCodeStart, ptrSource, sizeof(ucCodeStart));    // save first code location contents
                save_length -= sizeof(ucCodeStart);
                ptrDestination += sizeof(ucCodeStart);
                ptrSource += sizeof(ucCodeStart);
            }
            fnWriteBytesFlash(ptrDestination, ptrSource, save_length);   // program flash
            encryptBuffer.ptrDestinationAddress += SECURE_BLOCK_LENGTH;
            encryptBuffer.ulLength = 0;
        }
    }
    return 0;
}
#endif

#if defined INTERMEDIATE_PROG_BUFFER && !defined REMOVE_SREC_LOADING && !defined KBOOT_LOADER // {17}
static int fnIntermediateWrite(unsigned char *ptrAddr, unsigned char *ucPtrData, int iDataCnt)
{
    static unsigned char *ptrInput = 0;
    if (ptrIntermediateFlash == 0) {
        ptrIntermediateFlash = ptrAddr;                                  // the programming address of this buffer
        ptrInput = ptrIntermediateFlash;
    }
    else if (ptrAddr != ptrInput) {                                      // a hole in the srec has been identified - this must be jumped (left with 0xff content)
        ulOffset += (ptrAddr - ptrInput);                                // offset the hole size
        ptrInput = ptrAddr;                                              // synchronise
        if ((ulOffset + iDataCnt) >= ((sizeof(ucIntermediateBuffer) - (INTERMEDIATE_BUFFER_RESERVE/2)))) {
            return INVALID_SREC_HOLE;                                    // the hole is too large to be accepted - error to inform
        }
    }
    uMemcpy(&ucIntermediateBuffer[ulOffset], ucPtrData, iDataCnt);       // save the new line to the intermediate buffer
    ulOffset += iDataCnt;                                                // the amount of data collected
    ptrInput += iDataCnt;
    if (ulOffset > ((sizeof(ucIntermediateBuffer) - INTERMEDIATE_BUFFER_RESERVE))) { // if the buffer is almost full
        return STOP_FLOW_CONTROL;                                        // request that the flow is stopped so that programming can start
    }
    return LINE_ACCEPTED;                                                // submitted to buffer
}

static int fnFlashIntermediate(void)
{
    #if defined SREC_SECURE_LOADER                                       // {38}
    if (fnWriteBytesSecure(ptrIntermediateFlash, ucIntermediateBuffer, ulOffset, 1) != 0)
    #else
    if (fnWriteBytesFlash(ptrIntermediateFlash, ucIntermediateBuffer, ulOffset) != 0)
    #endif
    {
        fnDebugMsg("\r\n\nWrite-error!! (Ctrl+r to reset)");
        return 1;
    }
    #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
  //fnWriteBytesFlash(0, 0, 0);                                          // close any outstanding FLASH buffer
    #endif
    uMemset(ucIntermediateBuffer, 0xff, sizeof(ucIntermediateBuffer));   // flush the buffer ready for next use
    ulOffset = 0;
    ptrIntermediateFlash = 0;
    return 0;
}
#endif

#if (defined SERIAL_INTERFACE && defined KBOOT_LOADER) || (defined USB_INTERFACE && defined HID_LOADER && defined KBOOT_HID_LOADER && !defined USE_USB_MSD) // {20}
static void fnHandlePropertyGet(unsigned long ulPropertyTag, unsigned long ulMemoryID, KBOOT_PACKET *ptrKBOOT_response)
{
    #define BOOT_LOADER_NAME                    'K'
    #define BOOT_LOADER_MAJOR_VERSION            2
    #define BOOT_LOADER_MINOR_VERSION            0
    #define BOOT_LOADER_BUGFIX                   0
    #define kStatus_Success                      0
    #define kStatus_Fail                         1
    #define kStatus_ReadOnly                     2
    #define kStatus_OutOfRange                   3
    #define kStatus_UnknownProperty              10300
    #define kStatus_ReadOnlyProperty             10301
    #define kStatus_InvalidPropertyValue         10302
    #define KB_SUPPORTS_FLASH_ERASE_ALL          0x00000001
    #define KB_SUPPORTS_FLASH_ERASE_REGION       0x00000002
    #define KB_SUPPORTS_READ_MEMORY              0x00000004
    #define KB_SUPPORTS_WRITE_MEMORY             0x00000008
    #define KB_SUPPORTS_FILL_MEMORY              0x00000010
    #define KB_SUPPORTS_FLASH_SECURITY_DISABLE   0x00000020
    #define KB_SUPPORTS_GET_PROPERTY             0x00000040
    #define KB_SUPPORTS_RECEIVE_SB_FILE          0x00000080
    #define KB_SUPPORTS_EXECUTE                  0x00000100
    #define KB_SUPPORTS_CALL                     0x00000200
    #define KB_SUPPORTS_RESET                    0x00000400
    #define KB_SUPPORTS_SET_PROPERTY             0x00000800
    #define KB_SUPPORTS_FLASH_ERASE_ALL_UNSECURE 0x00001000
    #define KB_SUPPORTS_FLASH_PROGRAM_ONCE       0x00002000
    #define KB_SUPPORTS_FLASH_READ_ONCE          0x00004000
    #define KB_SUPPORTS_FLASH_READ_RESOURCE      0x00008000
    #define KB_SUPPORTS_CONFIGURE_QUAD_SPI       0x00010000
    #define KB_SUPPORTS_RELIABLE_UPDATE          0x00100000
    #define SUPPORTED_KBOOT_COMMANDS            (KB_SUPPORTS_FLASH_ERASE_REGION | KB_SUPPORTS_WRITE_MEMORY | KB_SUPPORTS_GET_PROPERTY | KB_SUPPORTS_RESET)
    const unsigned long version_info = ((BOOT_LOADER_NAME << 24) | (BOOT_LOADER_MAJOR_VERSION << 16) | (BOOT_LOADER_MINOR_VERSION << 8) | (BOOT_LOADER_BUGFIX));
    unsigned long ulStatus = kStatus_Success;
    unsigned long ulResponseData[4];
    int iDataLength = 1;                                                 // most responses return one data value
    int iNoStatus = 0;
    int iDataIndex = 4;
    int iDataRef = 0;
    int iDataPairs = 0;
    switch (ulPropertyTag) {
    case PROPERTY_VERSION:                                               // 1
        ulResponseData[0] = (unsigned long)version_info;
        break;
    case PROPERTY_FLASH_START_ADD:                                       // 3
        ulResponseData[0] = FLASH_START_ADDRESS;
        break;
    case PROPERTY_FLASH_SIZE_BYTES:                                      // 4
    #if SIZE_OF_FLASH == 0                                               // i.MX RT
        ulResponseData[0] = FLEXSPI_FLASH_SIZE;
    #else
        ulResponseData[0] = SIZE_OF_FLASH;
    #endif
        break;
    case PROPERTY_FLASH_SECTOR_SIZE:                                     // 5
        ulResponseData[0] = FLASH_GRANULARITY;
        break;
    case PROPERTY_AVAILABLE_CMDS:                                        // 7
        ulResponseData[0] = SUPPORTED_KBOOT_COMMANDS;
    #if defined KBOOT_LOADER_MASS_ERASE_TO_UNLOCK 
        ulResponseData[0] |= KB_SUPPORTS_FLASH_ERASE_ALL_UNSECURE;
    #endif
    #if defined KBOOT_LOADER_BACKDOOR_KEY_TO_UNLOCK
        ulResponseData[0] |= KB_SUPPORTS_FLASH_SECURITY_DISABLE;
    #endif
        break;
    case PROPERTY_MAX_PACKET_SIZE:                                       // {40} 11 (0x0b)
        ulResponseData[0] = 32;
        break;
    case PROPERTY_RESERVED_REGIONS:                                      // 12 (0x0c)
        iNoStatus = 0;
        iDataLength = 4;                                                 // the number of data values
        iDataPairs = 0;                                                  // the number of data pairs
        ulResponseData[0] = FLASH_START_ADDRESS;                         // reserved region
        ulResponseData[1] = (_UTASKER_APP_START_ - 1);
        ulResponseData[2] = RAM_START_ADDRESS;
        ulResponseData[3] = (RAM_START_ADDRESS + (SIZE_OF_RAM/2));
        break;
    case PROPERTY_RAM_SIZE_BYTES:                                        // 15 (0x0f)
        ulResponseData[0] = SIZE_OF_RAM;
        break;
    case PROPERTY_FLASH_SEC_STATE:                                       // 17 (0x11)
    #if defined FLASH_CONTROLLER_FTMRE
        ulResponseData[0] = ((FTMRH_FSEC & FTMRH_FSEC_SEC_UNSECURE_FLAG) == 0);
    #elif !defined _KINETIS
        ulResponseData[0] = 0;
    #else
        ulResponseData[0] = ((FTFL_FSEC & FTFL_FSEC_SEC_UNSECURE) == 0);
        #if defined _WINDOWS
        ulResponseData[0] = 1;                                           // test secured state behavior
        #endif
    #endif
        break;
    default:
        iDataLength = 0;
        ulStatus = kStatus_UnknownProperty;                              // returning this causes an error message to be displayed at the host but further operation is OK
        break;
    }
    if (iNoStatus == 0) {                                                // insert the status if not explicitly not required
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)ulStatus;
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulStatus >> 8);
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulStatus >> 16);
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulStatus >> 24);
        ptrKBOOT_response->ucData[3]++;                                  // parameter count
    }
    while (iDataLength-- != 0) {                                         // insert data
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)ulResponseData[iDataRef];
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulResponseData[iDataRef] >> 8);
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulResponseData[iDataRef] >> 16);
        ptrKBOOT_response->ucData[iDataIndex++] = (unsigned char)(ulResponseData[iDataRef] >> 24);
        ptrKBOOT_response->ucData[3]++;                                  // parameter count
        iDataRef++;
    }
    while (iDataPairs-- != 0) {
        ptrKBOOT_response->ucData[3]--;
    }
    ptrKBOOT_response->ucLength[0] = (unsigned char)iDataIndex;
}

static void fnReturnResponse(QUEUE_HANDLE hInterface, int iInterfaceType, KBOOT_PACKET *ptrKBOOT_response)
{
    #if (defined SERIAL_INTERFACE && defined KBOOT_LOADER)
    if (iInterfaceType == KBOOT_UART) {
        unsigned char ucSerialHeader[6];
        unsigned short usCRC;
        ucSerialHeader[0] = KBOOT_SERIAL_START_BYTE;
        ucSerialHeader[1] = KBOOT_SERIAL_COMMAND;
        ucSerialHeader[2] = ptrKBOOT_response->ucLength[0];
        ucSerialHeader[3] = 0;
        usCRC = fnCRC16(fnCRC16(0, ucSerialHeader, (sizeof(ucSerialHeader) - 2)), ptrKBOOT_response->ucData, ptrKBOOT_response->ucLength[0]); // calculate CRC
        ucSerialHeader[4] = (unsigned char)usCRC;
        ucSerialHeader[5] = (unsigned char)(usCRC >> 8);
        fnWrite(hInterface, ucSerialHeader, sizeof(ucSerialHeader));     // send serial header
        fnWrite(hInterface, ptrKBOOT_response->ucData, ptrKBOOT_response->ucLength[0]); // send the response body
    }
    else {                                                               // HID
    #endif
    #if (defined USB_INTERFACE && defined HID_LOADER && defined KBOOT_HID_LOADER)
        fnWrite(hInterface, (unsigned char *)ptrKBOOT_response, sizeof(KBOOT_PACKET));
        #if defined KBOOT_COMMAND_LIMIT
        uTaskerStopTimer(TASK_USB);                                      // stop timer as soon as a valid KBOOT message has been responded to
        #endif
    #endif
    #if (defined SERIAL_INTERFACE && defined KBOOT_LOADER)
    }
    #endif
}

static void fnPrepareGenericResponse(KBOOT_PACKET *ptrKBOOT_response, int iInterfaceType)
{
    ptrKBOOT_response->ucLength[0] = 12;
    ptrKBOOT_response->ucData[0] = 0xa0;
    if (iInterfaceType == KBOOT_UART) {
        ptrKBOOT_response->ucData[3] = 2;
    }
    else {
        ptrKBOOT_response->ucData[2] = 2;
    }
}

extern int fnHandleKboot(QUEUE_HANDLE hInterface, int iInterfaceType, KBOOT_PACKET *ptrKBOOT_packet)
{
    int iReturn = KBOOT_NO_ACTION;
    KBOOT_PACKET KBOOT_response;
    uMemset(&KBOOT_response, 0, sizeof(KBOOT_response));                 // zero the packet content
    switch (ptrKBOOT_packet->ucCommandType) {                            // the command
    case KBOOT_REPORT_ID_COMMAND_OUT:
        KBOOT_response.ucCommandType = KBOOT_REPORT_ID_COMMAND_IN;
        switch (ptrKBOOT_packet->ucData[0]) {
        case KBOOT_COMMAND_TAG_GET_PROPERTY:                             // 0x07
            {
    #if defined _WINDOWS
                unsigned char ucParameterCount = ptrKBOOT_packet->ucData[3]; // parameter count
    #endif
                unsigned long ulPropertyTag = (ptrKBOOT_packet->ucData[4] | (ptrKBOOT_packet->ucData[5] << 8) | (ptrKBOOT_packet->ucData[6] << 16) | (ptrKBOOT_packet->ucData[7] << 24));
                unsigned long ulMemoryID = (ptrKBOOT_packet->ucData[8] | (ptrKBOOT_packet->ucData[9] << 8) | (ptrKBOOT_packet->ucData[10] << 16) | (ptrKBOOT_packet->ucData[11] << 24));
    #if defined _WINDOWS
                if (ucParameterCount != 2) {
                    _EXCEPTION("Expecting the parameter count to be always 2 - investigate if this exception occurs");
                }
    #endif
                KBOOT_response.ucData[0] = (ptrKBOOT_packet->ucData[0] | 0xa0); // response tag
                fnHandlePropertyGet(ulPropertyTag, ulMemoryID, &KBOOT_response);
            }
            break;
        case KBOOT_COMMAND_TAG_WRITE_MEMORY:                             // 0x04
        case KBOOT_COMMAND_TAG_ERASE_REGION:                             // 0x02
            {
                unsigned long ulStartAddress;
                unsigned long ulLength;
                ulStartAddress = (ptrKBOOT_packet->ucData[4] | (ptrKBOOT_packet->ucData[5] << 8) | (ptrKBOOT_packet->ucData[6] << 16) | (ptrKBOOT_packet->ucData[7] << 24));
    #if defined _iMX && defined ADD_FILE_OBJECT_AFTER_LOADING
                ulStartAddress += _UTASKER_APP_START_;
    #endif
                ulLength = (ptrKBOOT_packet->ucData[8] | (ptrKBOOT_packet->ucData[9] << 8) | (ptrKBOOT_packet->ucData[10] << 16) | (ptrKBOOT_packet->ucData[11] << 24));
                if (KBOOT_COMMAND_TAG_ERASE_REGION == ptrKBOOT_packet->ucData[0]) { // command to delete an area in flash
                    if ((ulStartAddress >= UTASKER_APP_START) && (ulStartAddress < (unsigned long)UTASKER_APP_END)) {
                        if ((ulStartAddress + ulLength) > (unsigned long)UTASKER_APP_END) {
                            ulLength = ((unsigned long)UTASKER_APP_END - ulStartAddress);
                        }
                        fnEraseFlashSector((unsigned char *)ulStartAddress, (MAX_FILE_LENGTH)ulLength);
    #if defined KBOOT_SECURE_LOADER                                      // {34}
                        fnPrepareDecrypt(0);                             // prepare AES-256 decryption key and initial vector
    #endif
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
                        fileObjInfo.ptrLastAddress = 0;
    #endif
    #if defined DEBUG_CODE
                        fnDebugMsg("ERASING: ");
                        fnDebugHex(ulStartAddress, (sizeof(ulStartAddress) | WITH_LEADIN));
                        fnDebugHex(ulLength, (sizeof(ulLength) | WITH_SPACE | WITH_LEADIN | WITH_CR_LF));
    #endif
                    }
                }
                else {                                                   // command to write following data to flash
                    ptrFlashAddress = (unsigned char *)ulStartAddress;
                    ulProg_length = ulLength;
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
                    if ((ptrFlashAddress + ulLength) > fileObjInfo.ptrLastAddress) {
                        fileObjInfo.ptrLastAddress = (ptrFlashAddress + ulLength); // keep a track of the highest program address
                    }
    #endif
    #if defined DEBUG_CODE
                    fnDebugMsg("PROGRAMMING: ");
                    fnDebugHex(ulProg_length, (sizeof(ulLength) | WITH_LEADIN | WITH_CR_LF));
    #endif
                }
                fnPrepareGenericResponse(&KBOOT_response, iInterfaceType); // generic response
              //KBOOT_response.ucLength[0] = 12;                         
              //KBOOT_response.ucData[0] = 0xa0;
              //if (iInterfaceType == KBOOT_UART) {
              //    KBOOT_response.ucData[3] = 2;
              //}
              //else {
              //    KBOOT_response.ucData[2] = 2;
              //}
                KBOOT_response.ucData[8] = ptrKBOOT_packet->ucData[0];
            }
            break;
        case KBOOT_COMMAND_TAG_RESET:                                    // 0x0b
            iReturn = KBOOT_RESET_COMMANDED;                             // reset is required
            fnPrepareGenericResponse(&KBOOT_response, iInterfaceType);   // generic response
          //KBOOT_response.ucLength[0] = 12;                             // generic response
          //KBOOT_response.ucData[0] = 0xa0;
            if (iInterfaceType == KBOOT_UART) {
          //    KBOOT_response.ucData[3] = 2;
                fnDriver(hInterface, (RX_OFF), 0);                       // disable rx since we are resetting (to stop the host's ack triggering receive timer and losing reset timer)
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
                fileObjInfo.ptrShortFileName = "KBOOTSERBIN";            // define a name for the file (with default time/date)
    #endif
            }
            else {
          //    KBOOT_response.ucData[2] = 2;
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
                fileObjInfo.ptrShortFileName = "KBOOTUSBBIN";            // define a name for the file (with default time/date)
    #endif
            }
            KBOOT_response.ucData[8] = ptrKBOOT_packet->ucData[0];
    #if defined ADD_FILE_OBJECT_AFTER_LOADING
            if (fileObjInfo.ptrLastAddress != 0) {
                fnAddSREC_file(&fileObjInfo);                            // add a file object wit name and size
            }
    #endif
    #if defined DEBUG_CODE
            fnDebugMsg("RESETTING\r\n");
    #endif
            break;
    #if defined KBOOT_LOADER_MASS_ERASE_TO_UNLOCK
        case KBOOT_COMMAND_TAG_MASS_ERASE:                               // 13 (0x0d)
            iReturn = KBOOT_MASS_ERASE_COMMANDED;                        // respond with success and the perfrom a mass erase after a short delay
            fnPrepareGenericResponse(&KBOOT_response, iInterfaceType);   // generic response
            KBOOT_response.ucData[2] = 0x04;
            KBOOT_response.ucData[8] = 0x0d;
            break;
    #endif
    #if defined KBOOT_LOADER_BACKDOOR_KEY_TO_UNLOCK
        case KBOOT_COMMAND_TAG_FLASH_SECURITY_DISABLE:                   // 6
            {
                unsigned long ulKey[2];
                fnPrepareGenericResponse(&KBOOT_response, iInterfaceType); // generic response
                uMemcpy(ulKey, &ptrKBOOT_packet->ucData[4], sizeof(ulKey));
                fnBackdoorUnlock(ulKey);
                KBOOT_response.ucData[2] = 0x0c;
                KBOOT_response.ucData[8] = 0x06;
            }
            break;
    #endif

        case KBOOT_COMMAND_TAG_READ_MEMORY:                              // the following are not yet used by KBOOT resp. supported
        case KBOOT_COMMAND_TAG_FILL_MEMORY:
        case KBOOT_COMMAND_TAG_RECEIVE_SBFILE:
        case KBOOT_COMMAND_TAG_EXECUTE:
        case KBOOT_COMMAND_TAG_CALL:
        case KBOOT_COMMAND_TAG_SET_PROPERTY:
        case KBOOT_COMMAND_TAG_ERASE_ALL:
        default:
            _EXCEPTION("Use detected - investigate....");
            break;
        }
        fnReturnResponse(hInterface, iInterfaceType, &KBOOT_response);
        break;

    case KBOOT_REPORT_ID_DATA_OUT:                                       // data to be written
        {
            unsigned short usBuff_length = ptrKBOOT_packet->ucLength[0];
            if ((ptrFlashAddress >= (unsigned char *)UTASKER_APP_START) && (ptrFlashAddress < UTASKER_APP_END)) { // if the sector belongs to the application space
                if ((ptrFlashAddress + usBuff_length) > UTASKER_APP_END) { // if the write would be past the end of the application space
                    usBuff_length = (unsigned short)(ptrFlashAddress - UTASKER_APP_END);
                }
                if (usBuff_length > ulProg_length) {
                    usBuff_length = (unsigned short)ulProg_length;
                }
    #if defined KBOOT_SECURE_LOADER                                      // {34} decrypt buffer and write to flash
                fnWriteBytesSecure(ptrFlashAddress, ptrKBOOT_packet->ucData, usBuff_length, 0);
    #else
                fnWriteBytesFlash(ptrFlashAddress, ptrKBOOT_packet->ucData, usBuff_length); // program flash
    #endif
                ptrFlashAddress += usBuff_length;
                ulProg_length -= usBuff_length;
                if (ulProg_length == 0) {                                // complete code has been received
                    KBOOT_response.ucCommandType = KBOOT_REPORT_ID_COMMAND_IN;
                    fnPrepareGenericResponse(&KBOOT_response, iInterfaceType); // generic response
                  //KBOOT_response.ucLength[0] = 12;                     // generic response
                  //KBOOT_response.ucData[0] = 0xa0;
                  //if (iInterfaceType == KBOOT_UART) {
                  //    KBOOT_response.ucData[3] = 2;
                  //}
                  //else {
                  //    KBOOT_response.ucData[2] = 2;
                  //}
                    KBOOT_response.ucData[8] = KBOOT_COMMAND_TAG_WRITE_MEMORY;
    #if defined KBOOT_SECURE_LOADER                                      // {34}
                    fnWriteBytesSecure(0, 0, 0, 0);                      // decrypt and write any open block to flash
    #elif  defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0
                    fnWriteBytesFlash(0, 0, 0);                          // close any outstanding FLASH buffer
    #endif
                    fnReturnResponse(hInterface, iInterfaceType, &KBOOT_response); // send response to inform that all data has been programmed
    #if defined DEBUG_CODE
                    fnDebugMsg("*");
    #endif
                }
    #if defined DEBUG_CODE
                fnDebugMsg(".");
    #endif
            }
    #if defined DEBUG_CODE
            else {
                 fnDebugMsg("x");
            }
    #endif
        }
        break;
    }
    return iReturn;
}
#endif

#if defined SREC_IHEX_REQUIRED
    #if !defined KBOOT_LOADER || (defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && (defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES))
static unsigned char fnConvertByte(unsigned char ucASCII)
{
    unsigned char ucResult = (ucASCII - 0x30);
    if (ucResult > 9) {
        ucResult -= ('A' - '9' - 1);
        if (ucResult > 15) {
            ucResult -= ('a' - 'A');
        }
    }
    return ucResult;
}

#if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
static int fnLoadTerminate(int iType)
#else
static int fnLoadTerminate(void)
#endif
{
        #if (defined SERIAL_INTERFACE && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER) && defined INTERMEDIATE_PROG_BUFFER
    if (fnFlashIntermediate() != 0) {                                    // flash final intermediate buffer
        return PROGRAMMING_ERROR;
    }
        #endif
        #if defined SREC_SECURE_LOADER
    fnWriteBytesSecure(0, 0, 0, 0);                                      // close any outstanding decryp buffer
        #endif
        #if !defined SREC_SECURE_LOADER && (defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0)
    fnWriteBytesFlash(0, 0, 0);                                          // close any outstanding FLASH buffer
        #endif
        #if defined SPI_SW_UPLOAD
            #if defined SPI_SW_UPLOAD_ENCRYPTED                          // {8}
    ulHighestLocation -= FILE_HEADER;
    if (fnWriteBytesFlash((unsigned char *)(SIZE_OF_FLASH + UPLOAD_OFFSET), (unsigned char *)&ulHighestLocation, sizeof(ulHighestLocation)) != 0) // write file length for compatibilty
            #else
    if (fnWriteBytesFlash((unsigned char *)(SIZE_OF_FLASH + UPLOAD_OFFSET), ucCodeStart, sizeof(ucCodeStart)) != 0) // write first bytes of code to validate it
            #endif
        #else
            #if defined MEMORY_SWAP
    if (fnWriteBytesFlash((unsigned char *)UTASKER_APP_START, ucCodeStart, sizeof(ucCodeStart)) != 0) // write first bytes of code to validate it
            #else
    if (fnWriteBytesFlash((unsigned char *)_UTASKER_APP_START_, ucCodeStart, sizeof(ucCodeStart)) != 0) // write first bytes of code to validate it
            #endif
        #endif
    {
        return PROGRAMMING_ERROR;
    }
        #if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
    if (iType == USB_LOADING_IN_OPERATION) {
        return PROGRAMMING_COMPLETE;                                     // don't add a file name if programmed via USB since the usb device handling will do it
    }
        #endif
        #if defined ADD_FILE_OBJECT_AFTER_LOADING                        // {9}
    fileObjInfo.ptrShortFileName = "SOFTWARES19";                        // define a name for the file (with default time/date)
    if (fnAddSREC_file(&fileObjInfo) != 0) {                             // {12}
        return PROGRAMMING_ERROR;
    }
        #endif
    return PROGRAMMING_COMPLETE;                                         // end of file
}
    #endif

// SREC and/or Intel HEX input record handler
//
#if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
    extern int fnHandleRecord(unsigned char *ptrLine, unsigned char *ptrEnd, int iType)
#else
    extern int fnHandleRecord(unsigned char *ptrLine, unsigned char *ptrEnd)
#endif
{
    #define S_TYPE       0                                               // S-record states
    #define S_LENGTH_1   1
    #define S_LENGTH_2   2
    #define S_ADDR_1     3
    #define S_ADDR_2     4
    #define S_ADDR_3     5
    #define S_ADDR_4     6
    #define S_ADDR_5     7
    #define S_ADDR_6     8
    #define S_ADDR_7     9
    #define S_ADDR_8     10
    #define S_DATA_1     11
    #define S_CHECKSUM_1 200
    #define S_CHECKSUM_2 201
    #define S_LINE_END   202

    #define S_IHEX       1000
    #define S_I_ADDRESS  1001
    #define S_I_TYPE     1002
    #define S_I_DATA     1003

    #define I_DATA_RECORD               0x00
    #define I_END_OF_FILE               0x01
    #define I_EXTENDED_SEGMENT_ADDRESS  0x02
    #define I_START_SEGMENT_ADDRESS     0x03
    #define I_EXTENDED_LINEAR_ADDRESS   0x04
    #define I_START_LINEAR_ADDRESS      0x05

    #if !((defined SERIAL_INTERFACE && !defined KBOOT_LOADER) && defined INTERMEDIATE_PROG_BUFFER) && (defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0) // {24}
    static unsigned long ulNextExpectedSREC = 0;
    #endif
    #if defined SPI_SW_UPLOAD_ENCRYPTED                                  // {8}
    static unsigned long ulHighestLocation = 0;
    #endif
    #if defined EXCLUSIVE_INTEL_HEX_MODE || defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
    static unsigned long ulExtendedAddress = 0;
    static unsigned long ulIntelOffset = 0;
    #endif
    SREC_TYP srec;
    int iSREC_state;
    int iDataCnt = 0;
    unsigned char ucCheckSum = 0;
    srec.csum = 0;                                                       // quieten compiler warning by initialising
    #if defined EXCLUSIVE_INTEL_HEX_MODE                                 // {29}
    if (*ptrLine != ':') {                                               // each Intel Hex line must start with ':'
        #if defined _GNU_TEMP_WORKAROUND                                 // {1}
            ptrLine--;                                                   // after each restart after a pause the first 'S' is lost when compiled by AVR32 GCC - this simply accepts a line without the first letter and then works - reason however needs to be understood and solved correctly !!
        #else
            return CORRUPTED_SREC;
        #endif
    }
    iSREC_state = S_IHEX;                                                // handle as Intel Hex
    #else
    if ((*ptrLine != 'S') && (*ptrLine != 's')) {                        // each S-record line must start with 'S'
        #if defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
        if (*ptrLine != ':') {                                           // each Intel Hex line must start with ':'
        #endif
        #if defined _GNU_TEMP_WORKAROUND                                 // {1}
            ptrLine--;                                                   // after each restart after a pause the first 'S' is lost when compiled by AVR32 GCC - this simply accepts a line without the first letter and then works - reason however needs to be understood and solved correctly !!
        #else
            return CORRUPTED_SREC;
        #endif
        #if defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
        }
        else {
            iSREC_state = S_IHEX;                                        // handle as Intel Hex
        }
        #endif
    }
    else {
        iSREC_state = S_TYPE;                                            // handle as an S-record
    }
    #endif
    while (++ptrLine < ptrEnd) {                                         // for each input character
        switch (iSREC_state++) {
    #if defined SUPPORT_INTEL_HEX_MODE || defined EXCLUSIVE_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
        case S_IHEX:                                                     // the byte count 
            srec.csum = 0;
            srec.len = fnConvertByte(*ptrLine++);
            srec.len <<= 4;
            srec.len |= fnConvertByte(*ptrLine);
            ucCheckSum = (unsigned char)srec.len;
            if ((ptrLine + (srec.len * 2) + 10) != ptrEnd) {             // check the validity of the length
                return CORRUPTED_SREC;                                   // incorrect record length
            }
            srec.len *= 2;
            break;
        case S_I_ADDRESS:                                                // the address
            srec.addr = fnConvertByte(*ptrLine++); 
            srec.addr <<= 4;
            srec.addr |= fnConvertByte(*ptrLine++);
            srec.addr <<= 4;
            srec.addr |= fnConvertByte(*ptrLine++); 
            srec.addr <<= 4;
            srec.addr |= fnConvertByte(*ptrLine);
            ucCheckSum += (unsigned char)(srec.addr >> 8);
            ucCheckSum += (unsigned char)srec.addr;
            break;
        case S_I_TYPE:                                                   // the type
            srec.typ = fnConvertByte(*ptrLine++); 
            srec.typ <<= 4;
            srec.typ = fnConvertByte(*ptrLine);
            if (srec.typ > I_START_LINEAR_ADDRESS) {
                return CORRUPTED_SREC;
            }
            ucCheckSum += (unsigned char)srec.typ;
            if (srec.typ == I_END_OF_FILE) {
                iSREC_state = S_CHECKSUM_1;                              // this record has no data content to be collected
            }
            srec.typ |= 0x80;                                            // mark that the record is Intel Hex
            break;
    #endif
    #if !defined EXCLUSIVE_INTEL_HEX_MODE                                // {29}
        case S_TYPE:
            srec.typ = *ptrLine;
            if ((srec.typ == '0') || (srec.typ == '5')) {
                return LINE_ACCEPTED;                                    // ignore S0 and S5
            }
            if ((srec.typ == '7') || (srec.typ == '8') || (srec.typ == '9')) { // end of file
        #if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
                return (fnLoadTerminate(iType));
        #else
                return (fnLoadTerminate());
        #endif
            }
            srec.addr = 0;
            srec.csum = 0;
            srec.len = 0;
            break;
        case S_LENGTH_1:
        case S_LENGTH_2:
            srec.len |= (fnConvertByte(*ptrLine) << ((S_ADDR_1 - iSREC_state) * 4));
            break;
        case S_ADDR_1:
            ucCheckSum = (unsigned char)srec.len;
            srec.len *= 2;
            if (srec.typ == '1') {                                       // S1 record is composed of 2 byte data
                iSREC_state += 4;
                srec.len -= 6;                                           // length minus 2 bytes address and check sum
            }
            else if (srec.typ == '2') {                                  // S2 record is composed of 3 byte data
                iSREC_state += 2;
                srec.len -= 8;                                           // length minus 3 bytes address and check sum
            }
            else {
                srec.len -= 10;                                          // length minus 4 bytes address and check sum
            }
        case S_ADDR_2:
        case S_ADDR_3:
        case S_ADDR_4:
        case S_ADDR_5:
        case S_ADDR_6:
        case S_ADDR_7:
        case S_ADDR_8:
            srec.addr |= (fnConvertByte(*ptrLine) << ((S_DATA_1 - iSREC_state) * 4));
            break;
    #endif
        case S_CHECKSUM_1:
        case S_CHECKSUM_2:
            srec.csum |= (fnConvertByte(*ptrLine) << ((S_LINE_END - iSREC_state) * 4));
            break;
        case S_LINE_END:
            {
                unsigned char *ptrData = srec.data;
    #if !defined EXCLUSIVE_INTEL_HEX_MODE                                // {29}
        #if defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES
                if ((srec.typ & 0x80) == 0) {
        #endif
                    ucCheckSum += (unsigned char)(srec.addr >> 24);
                    ucCheckSum += (unsigned char)(srec.addr >> 16);
                    ucCheckSum += (unsigned char)(srec.addr >> 8);
                    ucCheckSum += (unsigned char)(srec.addr);
        #if defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES
                }
        #endif
    #endif
                ucCheckSum ^= 0xff;
    #if defined EXCLUSIVE_INTEL_HEX_MODE                                 // {29}
                ucCheckSum++;
    #elif defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
                if ((srec.typ & 0x80) != 0) {
                    ucCheckSum++;
                }
    #endif
                if (ucCheckSum != srec.csum) {
                    return SREC_CS_ERROR;                                // check sum error
                }
                iDataCnt /= 2;
        #if defined SUPPORT_INTEL_HEX_MODE || defined USB_MSD_ACCEPTS_HEX_FILES // {29}
                switch (srec.typ) {
                case (0x80 + I_END_OF_FILE):
            #if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
                    return (fnLoadTerminate(iType));
            #else
                    return (fnLoadTerminate());
            #endif
                case (0x80 + I_EXTENDED_SEGMENT_ADDRESS):
                    ulExtendedAddress = ((srec.data[0] << 12) | (srec.data[1] << 4)); // set the extended address
                    ulIntelOffset = 0;
                    return LINE_ACCEPTED;
                case (0x80 + I_START_SEGMENT_ADDRESS):
                    ulExtendedAddress = ((srec.data[0] << 12) | (srec.data[1] << 4)); // set the segment address
                    ulIntelOffset += ((srec.data[2] << 8) | srec.data[3]); // set the offset
                    return LINE_ACCEPTED;
                case (0x80 + I_EXTENDED_LINEAR_ADDRESS):
                    ulExtendedAddress = ((srec.data[0] << 24) | (srec.data[1] << 16)); // set the extended 
                    ulIntelOffset = 0;
                    return LINE_ACCEPTED;
                case (0x80 + I_START_LINEAR_ADDRESS):
                    return LINE_ACCEPTED;                                // the start address is not relevant for the loading operation
                case (0x80 + I_DATA_RECORD):
                    srec.addr += (ulExtendedAddress + ulIntelOffset);    // add the extended segment and offset to the data
                default:                                                 // default to data record
                    if (iDataCnt <= 0) {
                        return CORRUPTED_SREC;
                    }
                    break;
                }
        #else
                if (iDataCnt <= 0) {
                    return CORRUPTED_SREC;
                }
        #endif
        #if defined _iMX
                srec.addr += _UTASKER_APP_START_;
        #endif
        #if defined SPI_SW_UPLOAD_ENCRYPTED                              // {8} encrypted content has SREC address always starting at 0x00000000
                srec.addr += FILE_HEADER;                                // save space for a file header compatible with boot loader
                if ((unsigned char*)(srec.addr + iDataCnt) > (UTASKER_APP_END - UTASKER_APP_START)) { // ensure line doesn't write outside of intermediate space
                    return INVALID_APPLICATION_LOCATION;
                }
        #else
                if (srec.addr < (_UTASKER_APP_START_ + CODE_START_SIZE)) { // catch first bytes of application
            #if (_UTASKER_APP_START_ != 0)
                    if (srec.addr < _UTASKER_APP_START_){                // ensure line doesn't overwrite boot loader space
                        return INVALID_APPLICATION_LOCATION;
                    }
            #endif
            #if !defined SREC_SECURE_LOADER
                    while (srec.addr < (_UTASKER_APP_START_ + CODE_START_SIZE)) {
                #if defined _RX6XX                                       // workaround for compiler problem
                        volatile unsigned char *ptrBuffer = ucCodeStart;
                        volatile unsigned long ulOffset = (srec.addr - _UTASKER_APP_START_);
                        ptrBuffer += ulOffset;
                        *ptrBuffer = *ptrData++;
               #else
                        ucCodeStart[srec.addr - _UTASKER_APP_START_] = *ptrData++; // save first code location contents
               #endif
                        srec.addr++;
                        if (--iDataCnt == 0) {
                            return LINE_ACCEPTED;                        // line was valid
                        }
                    }
            #endif
                }
                if ((unsigned char *)(srec.addr + iDataCnt) > UTASKER_APP_END) { // {26} ensure line doesn't write outside of application space
                    return INVALID_APPLICATION_LOCATION;
                }
        #endif
        #if defined ADD_FILE_OBJECT_AFTER_LOADING
                if ((unsigned char *)srec.addr + iDataCnt > fileObjInfo.ptrLastAddress) { // {12} monitor the size of the software data
                    fileObjInfo.ptrLastAddress = (unsigned char *)srec.addr + iDataCnt;
                }
        #endif
        #if defined USB_MSD_ACCEPTS_SREC_FILES || defined USB_MSD_ACCEPTS_HEX_FILES // {30}
                if (iType == 0) {                                        // if the content is being tested for validity don't program anything
                    return LINE_ACCEPTED;                                // content is valid
                }
        #endif
        #if defined SERIAL_INTERFACE && !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER && defined INTERMEDIATE_PROG_BUFFER
            #if defined MEMORY_SWAP
                return (fnIntermediateWrite((unsigned char *)(srec.addr + UTASKER_APP_START), ptrData, iDataCnt));
            #else
                return (fnIntermediateWrite((unsigned char *)srec.addr, ptrData, iDataCnt));
            #endif
        #elif !defined KBOOT_LOADER && !defined DEVELOPERS_LOADER
            #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0             // {24}
                if (ulNextExpectedSREC != 0) {                           // if not first line to be programmed
                    if (ulNextExpectedSREC != srec.addr) {               // if the SREC address has jumped we ensure that any flash buffers are comitted if necessary
                        if ((ulNextExpectedSREC & ~(FLASH_ROW_SIZE - 1)) != (srec.addr & ~(FLASH_ROW_SIZE - 1))) { // address moves to a new flash buffer
                            fnWriteBytesFlash(0, 0, 0);                  // close any outstanding FLASH buffer
                        }
                    }
                }
            #endif
            #if defined SPI_SW_UPLOAD
                if (fnWriteBytesFlash((unsigned char *)(srec.addr + (SIZE_OF_FLASH + UPLOAD_OFFSET)), ptrData, iDataCnt) != 0) 
            #else
                #if defined MEMORY_SWAP
                if (fnWriteBytesFlash((unsigned char *)(srec.addr + UTASKER_APP_START), ptrData, iDataCnt) != 0)
                #else
                    #if defined SREC_SECURE_LOADER                       // {36} decrypt buffer and write to flash
                if (fnWriteBytesSecure((unsigned char *)srec.addr, ptrData, iDataCnt, 1) != 0)
                    #else
                if (fnWriteBytesFlash((unsigned char *)srec.addr, ptrData, iDataCnt) != 0)
                    #endif
                #endif
            #endif
                {
                    return PROGRAMMING_ERROR;
                }
            #if defined SPI_SW_UPLOAD || defined _iMX
                else {
                    unsigned char ucCheckMemory[256];                    // 256 bytes temporary space - assuming that SREC lines are never longer than this
                    fnGetParsFile((unsigned char *)(srec.addr + (SIZE_OF_FLASH + UPLOAD_OFFSET)), ucCheckMemory, (MAX_FILE_LENGTH)iDataCnt); // read back
                    if (uMemcmp(ucCheckMemory, ptrData, iDataCnt) != 0) { // check that the written code matches
                        return PROGRAMMING_ERROR;
                    }
                #if defined SPI_SW_UPLOAD_ENCRYPTED                      // {8}
                    if ((srec.addr + iDataCnt) > ulHighestLocation) {    // monitor the highest address written to
                        ulHighestLocation = (srec.addr + iDataCnt);
                    }
                #endif
                }
            #elif !(defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0)        // {19} don't check when the flash driver may be holding off committing rows since uneven length lines can otherwise detect errors since values are not yet in flash
                if (uMemcmp(fnGetFlashAdd((unsigned char*)srec.addr), ptrData, iDataCnt) != 0) { // check that the written code matches
                    return PROGRAMMING_ERROR;
                }
            #elif FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0                   // {24}
                ulNextExpectedSREC = (srec.addr + iDataCnt);             // store the next expected address so that holes in the SREC can be detected
            #endif
        #endif
            }
            return LINE_ACCEPTED;                                        // {4} line programmed successfully
        default:                                                         // collecting data
            if ((iDataCnt & 0x1) != 0) {
                srec.data[iDataCnt/2] |= fnConvertByte(*ptrLine);
                ucCheckSum += srec.data[iDataCnt/2];
            }
            else {
                srec.data[iDataCnt/2] = (fnConvertByte(*ptrLine) << 4);
            }
            if (++iDataCnt >= srec.len) {
                iSREC_state = S_CHECKSUM_1;
            }
            break;
        }
    }
    return CORRUPTED_SREC;                                               // line too short
}
#endif

#if defined NEEDS_BLANK_CHECK
static unsigned char *fnBlankCheck(void)
{
    #if defined SPI_SW_UPLOAD                                            // {5}
    unsigned char *ptrFlash = (unsigned char *)(SIZE_OF_FLASH + UPLOAD_OFFSET); // start of image area in SPI FLASH
    unsigned char *ptrEnd = (SIZE_OF_FLASH + UPLOAD_OFFSET) + (UTASKER_APP_END - UTASKER_APP_START); // size of upload area in SPI FLASH
    #else
    unsigned char *ptrFlash = (unsigned char *)UTASKER_APP_START;
    unsigned char *ptrEnd = UTASKER_APP_END;
    #endif
    fnDebugMsg("\r\nChecking Flash...");
    while (ptrFlash < ptrEnd) {
    #if defined SPI_SW_UPLOAD || (defined _iMX)                          // {39}
        int iCheck = 0;
        unsigned long ulLength = (ptrEnd - ptrFlash);                    // remaining length to check
        unsigned char ucCheck[1024];
        if (ulLength > sizeof(ucCheck)) {                                // perform chunk checks of maximum buffer size
            ulLength = sizeof(ucCheck);
        }
        fnGetParsFile(ptrFlash, ucCheck, ulLength);                      // read a block for checking
        while (ulLength-- != 0) {
            if (ucCheck[iCheck++] != 0xff) {
                return ptrFlash;                                         // non-blank location found
            }
            ptrFlash++;
        }
        fnRetriggerWatchdog();                                           // retrigger the watchdog to ensure that it can't fire when checking large program areas
    #else
        if (*fnGetFlashAdd(ptrFlash) != 0xff) {
            return ptrFlash;                                             // non-blank location found
        }
        ptrFlash++;
    #endif
    }
    return 0;                                                            // application Flash is blank
}
#endif


#if defined I2C_INTERFACE || (defined NEEDS_BLANK_CHECK && ((defined SERIAL_INTERFACE && !defined USE_MODBUS) || defined USE_USB_CDC))
static int fnPerformBlankCheck(void)
{
    unsigned char *ptrFlash = fnBlankCheck();                            // subroutine for blank check added
    if (ptrFlash == 0) {                                                 // if the application area is blank
        fnDebugMsg(" EMPTY!\r\n> ");
        return 0;
    }
    else {
    #if defined SHOW_APP_DETAILS
        fnDebugHex((CAST_POINTER_ARITHMETIC)ptrFlash, (WITH_SPACE | WITH_LEADIN | sizeof(CAST_POINTER_ARITHMETIC)));
        fnDebugMsg(" NOT BLANK!\r\n> ");
    #else
        fnDebugMsg(" NOT BLANK!\r\n> ");
    #endif
        return 1;
    }
}
#endif

#if defined I2C_INTERFACE && defined I2C_SLAVE_MODE&& !defined BLAZE_K22 // {32}
#define VERSION_MAJOR 1                                                  // I2C slave loader version
#define VERSION_MINOR 0

static unsigned char ucFlashState = 0;

static void fnFlashByte(unsigned char *ptrAddress, unsigned char *ptrByte)
{
    if (ptrAddress < (unsigned char *)(_UTASKER_APP_START_ + CODE_START_SIZE)) {
        ucCodeStart[(unsigned long)ptrAddress - _UTASKER_APP_START_] = *ptrByte; // backup start of code until complete code received
    }
    else {
        fnWriteBytesFlash(ptrAddress, ptrByte, 1);                       // save the received byte
    }

}

// The function is called during I2C slave interrupt handling so that the application can immediately respond in order to realise an I2C slave device
//
static int fnI2C_SlaveCallback(int iChannel, unsigned char *ptrDataByte, int iType)
{
    #define READ_VERSION_MAJOR         0
    #define READ_VERSION_MINOR         1
    #define READ_VERSION_FLASH_STATE   2

    #define TOTAL_RX_REGISTERS         3

    #define UPLOAD_MODE_READ           3

    #define WRITE_UPLOAD_DATA          0                                 // writes upload data and sets upload status read state
    #define WRITE_DELETE_REQUEST       1
    #define WRITE_RESET_REQUEST        2                                 // resets the device

    #define WRITE_DUMMY                3

    #define I2C_IDLE                   0
    #define I2C_SET_COMMAND            1
    #define I2C_SET_DATA               2
    static const unsigned char cucVersion[] = {VERSION_MAJOR, VERSION_MINOR};
    static int iRxCount = 0;
    static unsigned char ucResetKey = 0;
    static unsigned char ucUploadState[5] = {0};
    static unsigned char ucState = I2C_IDLE;
    static unsigned char ucReadMode = 0;
    static unsigned char ucWriteMode = 0;
    switch (iType) {                                                     // the interrupt callback type
    case I2C_SLAVE_ADDRESSED_FOR_READ:                                   // the slave is being addressed for reading from (first byte is requested)
        // Fall through intentionally
        //
    case I2C_SLAVE_READ:                                                 // further reads requested
        switch (ucReadMode++) {
        case READ_VERSION_MAJOR:
            *ptrDataByte = (unsigned char)cucVersion[0];                 // return the application version (major)
            break;
        case READ_VERSION_MINOR:
            *ptrDataByte = (unsigned char)cucVersion[1];                 // return the application version (minor)
            break;
        case READ_VERSION_FLASH_STATE:
            *ptrDataByte = ucFlashState;                                 // return the state of program flash
            ucReadMode = 0;                                              // circle back to the version
            break;
        case UPLOAD_MODE_READ:
            *ptrDataByte = ucUploadState[iRxCount++];                    // return the upload state
            if (iRxCount >= sizeof(ucUploadState)) {
                iRxCount = 0;                                            // circle in upload mode area
            }
            ucReadMode = UPLOAD_MODE_READ;                               // stay in this mode when uploading
            break;
        }
        return I2C_SLAVE_TX_PREPARED;                                    // the prepared byte is to to be sent

    case I2C_SLAVE_ADDRESSED_FOR_WRITE:                                  // the slave is being addressed to write to (*ptrDataByte is our address)
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1 * 60 * SEC), T_RESET);// reset if there is no master activity for this length of time
        ucState = I2C_SET_COMMAND;                                       // a write is being received and we expect the command to be written next
        return I2C_SLAVE_RX_CONSUMED;                                    // the byte has been consumed and nothing is to be put in the queue buffer

    case I2C_SLAVE_WRITE:                                                // data byte received (*ptrDataByte is the data received)
        if (ucState == I2C_SET_COMMAND) {                                // we are expecting the command to be received
            ucWriteMode = *ptrDataByte;                                  // the register that we will be writing to
            if (ucWriteMode > WRITE_RESET_REQUEST) {
                ucWriteMode = WRITE_DUMMY;                               // ignore invalid addresses
            }
            else if (ucWriteMode == WRITE_UPLOAD_DATA) {
                ucReadMode = UPLOAD_MODE_READ;                           // reads will return upload status from now on
            }
            ucResetKey = 0;
            ucState = I2C_SET_DATA;                                      // we may receive data next
        }
        else {                                                           // writing data
            switch (ucWriteMode++) {
            case WRITE_DUMMY:
                ucWriteMode = WRITE_DUMMY;                               // stay at dummy address in order to ignore all that arrives before a new write
                break;
            case WRITE_UPLOAD_DATA:
                {
                    unsigned long ulUploadAddress = ((ucUploadState[1] << 24) | (ucUploadState[2] << 16) | (ucUploadState[3] << 8) | ucUploadState[4]);
                    fnFlashByte((unsigned char *)(ulUploadAddress + _UTASKER_APP_START_), ptrDataByte); // save each byte to flash as it is received (the slave will hold the bus during the programming duration)
                    ucUploadState[0] |= 0x01;                            // upload has started
                    ulUploadAddress++;                                   // increment the address
                    ucUploadState[1] = (unsigned char)(ulUploadAddress >> 24); // put the updated address back to the status buffer for possible reading
                    ucUploadState[2] = (unsigned char)(ulUploadAddress >> 16);
                    ucUploadState[3] = (unsigned char)(ulUploadAddress >> 8);
                    ucUploadState[4] = (unsigned char)(ulUploadAddress);
                    ucWriteMode = WRITE_UPLOAD_DATA;
                }
                break;
            case WRITE_DELETE_REQUEST:                                   // delete key is being checked
                if ((ucResetKey == 0) && (*ptrDataByte == 0x52)) {
                    ucResetKey++;
                }
                else if ((ucResetKey == 1) && (*ptrDataByte == 0x84)) {
                    uMemset(ucUploadState, 0, sizeof(ucUploadState));
                    if (ucFlashState == 0) {                             // if the application area is not erased
                        fnInterruptMessage(OWN_TASK, DELETE_APPLICATION_FLASH); // send an interrupt event to the application so that it can delete the flash outside the interrupt routine
                    }
                }
                else {
                    ucResetKey = 0xff;                                   // block the reset key until next try
                }
                ucWriteMode = WRITE_DELETE_REQUEST;
                break;
            case WRITE_RESET_REQUEST:                                    // reset key is checked
                if ((ucResetKey == 0) && (*ptrDataByte == 0x55)) {
                    ucResetKey++;
                }
                else if ((ucResetKey == 1) && (*ptrDataByte == 0xaa)) {
                    ucUploadState[0] |= 0x80;                            // mark that a reset will occur shortly
                    if (ucUploadState[0] & 0x01) {                       // if reset command following loading
                        fnInterruptMessage(OWN_TASK, TERMINATE_PROGRAMMING);
                    }
                    else {
                        fnResetBoard();
                    }
                }
                else {
                    ucResetKey = 0xff;                                   // block the reset key until next try
                }
                ucWriteMode = WRITE_RESET_REQUEST;
                break;
            default:
                ucState = I2C_IDLE;                                      // return to data reception
                break;
            }
        }
        return I2C_SLAVE_RX_CONSUMED;                                    // the byte has been consumed and nothing is to be put in the queue buffer

    case I2C_SLAVE_READ_COMPLETE:
        // Fall through intentionally
        //
    case I2C_SLAVE_WRITE_COMPLETE:                                       // the write has terminated
        // ptrDataByte is 0 and so should not be used
        //
        ucState = I2C_IDLE;                                              // return to data reception
        return I2C_SLAVE_RX_CONSUMED;
    }
    return I2C_SLAVE_BUFFER;                                             // use the buffer for this transaction
}

static void fnOpenI2C_slave(void)
{
    I2CTABLE tI2CParameters;

    tI2CParameters.Channel = 0;                                          // I2C0
    tI2CParameters.ucSlaveAddress = OUR_SLAVE_ADDRESS;                   // slave address
    tI2CParameters.usSpeed = 0;                                          // select slave mode of operation
    tI2CParameters.fnI2C_SlaveCallback = fnI2C_SlaveCallback;            // the I2C slave interrupt callback on reception
    tI2CParameters.Rx_tx_sizes.TxQueueSize = 2;                          // transmit queue size (not used)
    tI2CParameters.Rx_tx_sizes.RxQueueSize = 2;                          // receive queue size (not used)
    tI2CParameters.Task_to_wake = 0;                                     // do not wake task on transmission termination

    I2CPortID = fnOpen(TYPE_I2C, FOR_I_O, &tI2CParameters);              // open the I2C channel with defined configurations
    fnCheckFlashState();
    if (ucFlashState == 0) {                                             // not empty
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(3 * 60 * SEC), T_RESET);// reset if there is no activity for this length of time
    }
}

static void fnCheckFlashState(void)
{
    if (fnPerformBlankCheck() == 0) {                                    // check whether flash is empty
        ucFlashState = 1;                                                // flash is empty
    }
}
#endif

#if defined _iMX || defined SDCARD_SUPPORT || defined SPI_FLASH_FAT || defined USB_MSD_HOST_LOADER || (defined SERIAL_INTERFACE && (defined KBOOT_LOADER || (defined DEVELOPERS_LOADER && defined DEVELOPERS_LOADER_CRC))) // {20}
// CRC-16 routine
//
extern unsigned short fnCRC16(unsigned short usCRC, unsigned char *ptrInput, unsigned long ulBlockSize)
{
    while (ulBlockSize-- != 0) {
        usCRC = (unsigned char)(usCRC >> 8) | (usCRC << 8);
        usCRC ^= *ptrInput++;
        usCRC ^= (unsigned char)(usCRC & 0xff) >> 4;
        usCRC ^= (usCRC << 8) << 4;
        usCRC ^= ((usCRC & 0xff) << 4) << 1;
    }
    return usCRC;
}
#endif

extern void fnJumpToValidApplication(int iResetPeripherals)              // {25}
{
#if defined _iMX                                                         // {37}
    iMX_BOOT_HEADER *ptrFlashHeader = (iMX_BOOT_HEADER *)fnGetFlashAdd((unsigned char *)_UTASKER_APP_START_);
    const unsigned char *ptrCode = (const unsigned char *)(ptrFlashHeader + 1);
    #if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)                   // the header format is big-endian so we need to convert to little-endian for processor compatibility
    iMX_BOOT_HEADER file_header;
  //uMemcpy(&file_header, ptrFlashHeader, sizeof(iMX_BOOT_HEADER));      // copy the header locally
    fnGetParsFile((unsigned char *)ptrFlashHeader, (unsigned char *)&file_header, sizeof(iMX_BOOT_HEADER)); // copy to the buffer
    ptrFlashHeader = &file_header;
    fniMXHeaderToLittleEndian(ptrFlashHeader);                           // convert content from big- to little-endian format
    #endif
    if (fnCheckValidApplication(ptrFlashHeader, ptrCode) != 0) {
    #if defined iMX_FALLBACK_SERIAL_LOADER
        *BOOT_MAIL_BOX = (RESET_TO_SERIAL_LOADER | RESET_NO_COUNT_FLAG); // inform the loader that the serial loader should be started
    #else
        *BOOT_MAIL_BOX = (RESET_TO_APPLICATION | RESET_NO_COUNT_FLAG);   // inform the loader that the application should be started
    #endif
    #if defined _iMX && !defined iMX_FALLBACK_SERIAL_LOADER
        if (iAppState == (STATE_RESETTING | STATE_PAUSING)) {
            *BOOT_MAIL_BOX = (RESET_TO_APPLICATION_PAUSE | RESET_NO_COUNT_FLAG);
        }
    #endif
        fnResetBoard();                                                  // reset to boot loader, which will then load and start the application
    }
    else {
    #if defined iMX_FALLBACK_SERIAL_LOADER
        fnDebugMsg("No Serial Loader!!\r\n");
    #else
        fnDebugMsg("No Application!!\r\n");
    #endif
    }
#else
    if (((*(unsigned long *)fnGetFlashAdd((unsigned char *)_UTASKER_APP_START_) != 0xffffffff) && (*(unsigned long *)(fnGetFlashAdd((unsigned char *)_UTASKER_APP_START_) + 4) != 0xffffffff))
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER           // {9}
        && (*(unsigned long *)fnGetFlashAdd((unsigned char *)UTASKER_APP_START) != 0xffffffff)
    #endif
    ) {
        if (iResetPeripherals != 0) {                                    // if peripherals have been in use
            uDisable_Interrupt();                                        // ensure interrupts are disabled when jumping to the application
    #if  (defined USB_INTERFACE && defined USB_MSD_HOST_LOADER && defined USB_HOST_SUPPORT)
            USB_HOST_POWER_OFF();                                        // {28} remove power to memory stick
    #endif
            RESET_PERIPHERALS();                                         // {14} reset peripherals and disable interrupts ready for jumping to the application
        }
        start_application(_UTASKER_APP_START_);                          // jump to the application
    }
    else {
        fnDebugMsg("No Application!!\r\n");
    }
#endif
}

#if defined _iMX
    #if defined ENABLE_PRODUCT_SECURITY
static int fnSetUnknownKey(void)
{
    POWER_UP_ATOMIC(2, OCOTP_CTRL_CLOCK);
    if ((HW_OCOTP_LOCK & (eFUSE_LOCK_SW_GP2_LOCK | eFUSE_LOCK_SW_GP2_RLOCK)) != (eFUSE_LOCK_SW_GP2_LOCK | eFUSE_LOCK_SW_GP2_RLOCK)) { // check whether the SW-GP2 fuse is protected again reading and writing
        unsigned long ulLock;
        unsigned long ulRandomKey[4];
        if ((HW_OCOTP_SW_GP20 == 0) && (HW_OCOTP_SW_GP21 == 0) && (HW_OCOTP_SW_GP22 == 0) && (HW_OCOTP_SW_GP23 == 0)) { // if it can be read as zero it hasn't been programmed yet
            int i;
            fnInitialiseRND(0);                                          // initialise the random number generator
            for (i = 0; i < 4; i++) {                                    // fill the key with a random number
        #if defined _WINDOWS
            #if defined MIMXRT1050
                static const unsigned long decrypt_key_reference[] = {   // irreversibly programmed and protected key in reference iMXRT1050-EVB
                    0xc44e21e2,
                    0x55d399a7,
                    0x0a70fce8,
                    0x8ec33d20,
                };
                ulRandomKey[i] = decrypt_key_reference[i];
                continue;
            #elif defined iMX_RT1062_EMB_ART
                #if 0
                static const unsigned long decrypt_key_reference[] = {   // irreversibly programmed and protected key in reference iMX_RT1062_EMB_ART - OEM reference 1
                    0xc44e21e2,
                    0x55d399a7,
                    0x0a70fce8,
                    0x8ec33d20,
                };
                #endif
                #if 1
                static const unsigned long decrypt_key_reference[] = {   // irreversibly programmed and protected key in reference iMX_RT1062_EMB_ART - OEM reference 2
                    0x61e1ae51,
                    0xa666559b,
                    0x4457e6fb,
                    0x76ce2e26,
                };
                #endif
                ulRandomKey[i] = decrypt_key_reference[i];
                continue;
            #endif
        #endif
                ulRandomKey[i] = fnGetRndHW();
                ulRandomKey[i] <<= 16;
                ulRandomKey[i] |= fnGetRndHW();
            }
            *SECRET_KEY_0 = ulRandomKey[0];                              // make a copy of the random key in the persistent memory which will be retained until the next power cycle (for test or monitoring purposes)
            *SECRET_KEY_1 = ulRandomKey[1];
            *SECRET_KEY_2 = ulRandomKey[2];
            *SECRET_KEY_3 = ulRandomKey[3];
            OCOTP_WriteFuseShadowRegister(eFUSE_SW_GP2_0, ulRandomKey, 4); // program the random key to SW_GP2
            if ((HW_OCOTP_SW_GP20 != ulRandomKey[0]) || (HW_OCOTP_SW_GP21 != ulRandomKey[1]) || (HW_OCOTP_SW_GP22 != ulRandomKey[2]) || (HW_OCOTP_SW_GP23 != ulRandomKey[3])) {
                return -1;
            }
        }
        ulLock = (eFUSE_LOCK_SW_GP2_RLOCK);                              // set lock of read of SW_GP2
        OCOTP_WriteFuseShadowRegister(eFUSE_LOCK, &ulLock, 1);           // blow SW_GP2 lock uFUSES so that the random key value can never be read out
        ulLock = (eFUSE_LOCK_SW_GP2_LOCK);                               // set lock of change of SW_GP2
        OCOTP_WriteFuseShadowRegister(eFUSE_LOCK, &ulLock, 1);           // blow SW_GP2 lock uFUSES so that the random key value can never be changed
        if ((HW_OCOTP_LOCK & (eFUSE_LOCK_SW_GP2_LOCK | eFUSE_LOCK_SW_GP2_RLOCK)) != (eFUSE_LOCK_SW_GP2_LOCK | eFUSE_LOCK_SW_GP2_RLOCK)) {
            return -1;
        }
    }
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2) != uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2) { // check whether the BEE module is using the KEY0 from SW-GP2
        // program it to use SW-GP2 as BEE0 key - this is not programmed because it can be controlled at run time
  //}
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_SJC_DISABLE) == 0) {            // if jtag is not disabled
          // need to disable jtag
  //}
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_JTAG_SMODE_NO_DEBUG) == uFUSE_BOOT_CFG2_JTAG_SMODE_JTAG) { // if jtag is not secured
        // need to secure jtag
        // write a response reference value and lock it from being modified or read
  //}
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_KTE) == 0) {                    // if trace is enabled
        // need to disable it
  //}
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_JTAG_HEO) == 0) {               // if HAB can override the JTAG security mode
        // need to disable it
  //}
  //if ((HW_OCOTP_CFG5 & uFUSE_BOOT_CFG2_FORCE_COLD_BOOT) == 0) {        // if HAB can override the JTAG security mode
        // need to disable it
  //}
    return 0;
}
    #endif

static void fnEncryptKeys(void)
{
    unsigned long ulKeys[iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long)];
    unsigned char *ptrKeys;
    unsigned char *ptrOriginalKeys;
    int i;
    #if defined iMX_SERIAL_LOADER && defined ENABLE_PRODUCT_SECURITY
    if (fnSetUnknownKey() < 0) {
        // The eFUSE programming has failed so we can't be sure that the processor has been correctly secured - we reset to avoid continuing rather than risking an incorrectly secured state
        //
        *BOOT_MAIL_BOX = DO_FULL_RESET;
        fnResetBoard();
    }
    #endif
    fnGetParsFile(ORIGINAL_KEYS_ADDRESS, (unsigned char *)ulKeys, sizeof(ulKeys)); // copy key area locally
    if (ulKeys[0] != 0) {                                                // if the keys have not yet been encrypted and re-located
        unsigned long ulCodingKey[iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long)];
        uMemset(ulCodingKey, 0, sizeof(ulCodingKey));                    // initialise decoding key array to zero
        fnGetDecryptionKey(ulCodingKey);                                 // get a copy of the coding key
        fnDecryptKey(ulKeys, ulCodingKey);                               // apply the key to have the effect of encryption
        fnWriteBytesFlash(ENCRYPTED_KEYS_ADDRESS, (unsigned char *)ulKeys, sizeof(ulKeys)); // relocate the encrypted keys
        uMemset(ulKeys, 0, sizeof(ulKeys));
        fnWriteBytesFlash(ORIGINAL_KEYS_ADDRESS, (unsigned char *)ulKeys, sizeof(ulKeys)); // clear original keys (this utilises the fact that SPI flash allows clearing individual bits even when locations have been previously programmed)
    }
    // At this point it is expected that the orignal key area have been cleared and we check that there was no previous failure and destroy any remaining information found
    // - any non-zero byte in the area is written to zero individually to ensure that devices that cannot overwrite multiple bytes result in a blank area
    //
    FOREVER_LOOP() {
        int iDirty = 0;
        fnGetParsFile(ORIGINAL_KEYS_ADDRESS, (unsigned char *)ulKeys, sizeof(ulKeys)); // copy key area locally again
        ptrKeys = (unsigned char *)ulKeys;
        ptrOriginalKeys = ORIGINAL_KEYS_ADDRESS;
        i = 0;
        while (i++ < iMX_LOADER_KEY_STORAGE_AREA_SIZE) {
            if (*ptrKeys++ != 0) {
                unsigned char ucEmpty = 0;
                fnWriteBytesFlash(ptrOriginalKeys, (unsigned char *)&ucEmpty, 1);
                iDirty = 1;
            }
            ptrOriginalKeys++;
        }
        if (iDirty == 0) {
            break;
        }
    }
}
#endif

// This is called very early on in the board initialisation and can be used to configure hardware.
// - beware that there is no heap available at call time
//
extern void fnUserHWInit(void)
{
#if defined USB_INTERFACE                                                 // {9}
    #if defined USB_MSD_DEVICE_LOADER && !defined MEMORY_SWAP
    int iForcedToBootMode = 0;
    #endif
    #if defined USB_HOST_SUPPORT
    USB_HOST_POWER_CONFIG();                                             // configure USB host power supply to default (off) state
    #endif
#endif
#if defined _iMX
    fnEncryptKeys();                                                     // if either the fall-back serial loader or the serial loader finds that the decryption keys are still in the boot loader code they are encrypted to a special area for subsequent use (this will therefore be performed just one after the boot loader has been programmed)
    uTaskerBoot();                                                       // perform the bare-minimum boot loader function before starting the serial loader (if code is updated this doesn't return but instead the board is reset)
#endif
#if !defined MEMORY_SWAP                                                 // swap memory remains always in the loader
    #if defined _WINDOWS && (defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER)
    iForcedToBootMode = 1;
    #endif
    #if defined MIMXRT1010 || defined MIMXRT1015
    fnDelayLoop(5000);                                                   // allow the force loader input to stabilise on these boards
    #endif
    if ((FORCE_BOOT() == 0)) {                                           // if the serial loader is not forced (eg. with an input held in a certain state or an appropriate mail-box command)
        _DELETE_BOOT_MAILBOX();                                          // {13}
        fnJumpToValidApplication(0);                                     // {25} check for forced boot mode or no application code
    }
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER           // {9}
    else {
        #if !defined _WINDOWS
        _DELETE_BOOT_MAILBOX();                                          // {13}
        #endif
        iForcedToBootMode = 1;
    }
    if (fnBlankCheck() != 0) {                                           // since the USB loader is more automated, clean up the application Flash if it is not blank
        if (iForcedToBootMode == 0) {                                    // if the boot mode has not been forced it means that the application is not detected
        #if defined SPI_SW_UPLOAD
            fnEraseFlashSector((unsigned char *)(SIZE_OF_FLASH + UPLOAD_OFFSET), (UTASKER_APP_END - (unsigned char *)UTASKER_APP_START)); // delete space in SPI FLASH
        #else
            fnEraseFlashSector((unsigned char *)UTASKER_APP_START, (MAX_FILE_LENGTH)(UTASKER_APP_END - (unsigned char *)UTASKER_APP_START)); // delete application space
        #endif
        }
    }
    // A non-forced boot mode start is guaranteed to have empty application Flash from here
    //
    #endif
    _DELETE_BOOT_MAILBOX();                                              // {13}
#endif
#if defined ETH_INTERFACE && defined RESET_PHY                           // {18}
    ASSERT_PHY_RST();                                                    // immediately set PHY to reset state
    CONFIG_PHY_STRAPS();                                                 // configure the required strap options - the reset line will be de-asserted during the Ethernet configuration
#endif
#if defined SUPPORT_GLCD
    CONFIGURE_GLCD();
#endif
}
