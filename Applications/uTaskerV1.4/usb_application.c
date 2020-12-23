/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_application.c - reference for USB communication device, audio, MSD and HID mouse/keyboards
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    19.09.2008 File content made conditional on HOST support             {1}
    19.09.2008 Allow use without USE_MAINTENANCE                         {2}
    14.12.2008 Add Luminary Micro Vendor ID                              {3}
    03.01.2009 Add brackets to quieten GCC warning                       {4}
    11.01.2009 Control callback returns BUFFER_CONSUMED_EXPECT_MORE when extra data is expected {5}
    08.02.2009 Add official uTasker PID for Luminary project             {6}
    11.02.2009 Add official uTasker VID/PID for ATMEL SAM7X project      {7}
    20.02.2009 Make fnSetNewSerialMode() call dependent on DEMO_UART     {8}
    12.05.2009 Set ucCollectingMode to zero after a control endpoint 0 command has been processed {9}
    01.07.2009 Adapt for STRING_OPTIMISATION configuration               {10}
    06.08.2009 MODBUS USB slave support added                            {11}
    07.01.2010 Adapt fnSetSerialNumberString to allow strings with zero content {12}
    21.03.2010 Add USB_TERMINATING_ENDPOINT to control the behavior of bulk endpoint when transmission has terminated {13}
    02.04.2010 Add LPCXXXX descriptor setup to respect endpoint capabilities {14}
    24.09.2010 Change ucDiskFlags to usDiskFlags
    06.03.2011 Accept on MODE_SENSE_6_PAGE_CODE_ALL_PAGES in UFI_MODE_SENSE_6 and report write protection {15}
    07.03.2011 Allow reading and writing MSD when not formatted          {16}
    07.03.2011 Yield after mass-storage writes                           {17}
    05.05.2011 Configure system clock as USB clock source for Kinetis tower kit {18}
    17.05.2011 Official USB VID/PID for Freescale MSD                    {19}
    05.06.2011 Adapt endpoint clear to suit LPC devices                  {20}
    25.01.2012 Add optional USB logo display control                     {21}
    25.01.2012 Activate USB-MSD with define USE_USB_MSD                  {22}
    25.01.2012 Add USE_USB_HID_MOUSE                                     {23}
    05.02.2012 Move USE_USB_CDC default to config.h
    10.02.2012 Only send HID mouse information on change and add mouse left key {24}
    18.02.2012 Add TWR_K60F120M support                                  {25}
    19.04.2012 Add KWIKSTIK, TWR_K70F120M and TWR_K20D50M support        {26}
    04.02.2013 Add HS USB                                                {27}
    06.06.2013 Add USB_SPEC_VERSION default                              {28}
    06.06.2013 Add KBED USB clock configuration                          {29}
    15.01.2014 Add KINETIS_K53 and FRDM_K20D50M USB clock configuration  {30}
    02.02.2014 Prepare disk for block writing to speed up operation      {31}
    04.02.2014 Add host mode                                             {32}
    14.08.2014 Add multiple USB-CDC interfaces as well as composite device capability {33}
    02.11.2014 Add HID keyboard                                          {34}
    07.12.2014 Add FREEMASTER_CDC support                                {35}
    11.12.2014 Return stall to UFI_READ_CAPACITY when not formatted      {36}
    20.07.2015 Descriptors included as header files to the application and USE_USB_HID_RAW added
    23.11.2015 Add USB-CDC host mode - USB_CDC_HOST                      {37}
    28.11.2015 Reset ucCollectingMode only after receiving all data and reset to 0xff {38}
    14.12.2015 Add audio endpoints                                       {39}
    13.04.2016 Change parameter of fnGetUSB_string_entry() to unsigned char {40}
    15.11.2016 Default FIRST_CDC_INTERFACE_HANDLE to 0 for USB host CDC  {41}
    05.01.2017 Add audio reception FFT analysis                          {42}
    06.02.2017 Automatically enable IN polling on CDC bulk IN endpoint   {43}
    12.02.2017 Allow USB-MSD or USB-CDC host operation, depending on which device is detected {44}
    07.03.2017 Add USB_MSD_REMOVED event on memory stick removal         {45}
    14.01.2018 Block USB-MSD further command handling until complete write data has been received {46}
    14.02.2018 Allow operation in both device and host modes (decided at initialisation) {47}
    20.02.2018 When both host and device modes can operate handle CDC reception appropriately {48}
    04.05.2018 Change interface to fnSetNewSerialMode()                  {49}
    02.06.2018 Zero optional user UART callback handlers                 {50}
    30.10.2018 Add optional USB-CDC interface to interface pass though reference {51}
    13.08.2019 Add RNDIS host operation
    06.06.2020 Updated to allow dual device or device/host on parts with 2 USB controller (including FS + HS on kinetis)
    06.06.2020 Add high speed dual-microphone audio reference            {52}
    28.06.2020 Extracted RNDIS device and host code from here to its own file: usb_RNDIS.h
    30.06.2020 Extracted MSD device and host code from here to its own file: usb_MSD.h
    21.08.2020 Add host hub support
    24.08.2020 Extracted HID mouse, HID keyboard, HID raw and Audio devices from here to their own files: usb_Keyboard.h, usb_Mouse.h, usb_RAW.h, usb_Audio.h

*/


/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"

#if defined USB_INTERFACE || defined USB_HOST_SUPPORT                    // {1}

#if defined FRDM_KL27Z && defined _DEV2
    #include "../uTaskerSerialBoot/_DEV2.h"
#endif


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#define OWN_TASK                            TASK_USB

#if !defined USB_CDC_COUNT
    #if defined USB_CDC_HOST_COUNT
        #define USB_CDC_COUNT               USB_CDC_HOST_COUNT
    #else
        #define USB_CDC_COUNT               1
    #endif
#endif
#if !defined MODBUS_USB_CDC_COUNT
    #define MODBUS_USB_CDC_COUNT            0
#endif
#define USB_DEVICE_REF                      0
#define USB_AUDIO_DEVICE_REF                (USB_DEVICE_REF)
#if defined _iMX && (USBS_AVAILABLE > 1)
    #define USB_CONTROLLERS                 2
    #define USB_HS_DEVICE_REF               1
    #define USB_HOST_REF                    1
    #if defined USE_HS_USB_AUDIO
        #undef USB_AUDIO_DEVICE_REF
        #define USB_AUDIO_DEVICE_REF        (USB_HS_DEVICE_REF)
        #define PARALLEL_USD_DEVICE_INTERFACE
    #endif
#elif defined USB_HS_INTERFACE && defined USB_FS_INTERFACE                // kinetis parts with both FS and HS interfaces in operation together
    #define PARALLEL_USD_DEVICE_INTERFACE
    #define USB_CONTROLLERS                 2
    #define USB_HS_HOST_REF                 1
    #define USB_HS_DEVICE_REF               1                             // the first is the FS device interface and the second is the HS device interface
    #undef USB_AUDIO_DEVICE_REF
    #define USB_AUDIO_DEVICE_REF            (USB_HS_DEVICE_REF)
    #define USB_HOST_REF                    (USB_HS_HOST_REF)
#else
    #define USB_CONTROLLERS                 1
    #define USB_HOST_REF                    0
#endif

#if defined USB_HOST_SUPPORT
    #if !defined NUMBER_OF_HOST_ENDPOINTS
        #define NUMBER_OF_HOST_ENDPOINTS    5                            // reserve 2 bulk endpoints for USB-MSD host as well as an interrupt endpoint (although not used, some memory sticks request this) - plus reserve
    #endif
#endif

#if defined TELIT_LE910C1
    #define RNDIS_HOST_INTERRUPT_INTERFACE  0                            // RNDIS interrupt interface
    #define FIRST_CDC_HOST_INTERFACE        1                            // RNDIS data interface (bulk)
    #define SECOND_CDC_HOST_INTERFACE       6                            // modem interface (bulk)
#else
    #define FIRST_CDC_HOST_INTERFACE        0
    #define SECOND_CDC_HOST_INTERFACE       1
    #define RNDIS_HOST_INTERRUPT_INTERFACE  (FIRST_CDC_HOST_INTERFACE)
#endif

#define RNDIS_DEFINES
    #include "usb_RNDIS.h"
#undef RNDIS_DEFINES
#define MSD_DEFINES
    #include "usb_MSD.h"
#undef MSD_DEFINES
#define HUB_DEFINES
    #include "usb_HUB.h"
#undef HUB_DEFINES
#define KEYBOARD_DEFINES
    #include "usb_Keyboard.h"
#undef KEYBOARD_DEFINES
#define MOUSE_DEFINES
    #include "usb_Mouse.h"
#undef MOUSE_DEFINES
#define HID_RAW_DEFINES
    #include "usb_RAW.h"
#undef HID_RAW_DEFINES

#define INCLUDE_USB_DEFINES
    #if defined USE_USB_CDC                                              // one or more CDC interfaces (with optional MSD, mouse and/or keyboard)
        #if USB_CDC_COUNT > 1
          //#define USB_CDC_INTERFACE_PASSTHROUGH                        // {51} if USB_CDC_COUNT is 2 or more reception on the first USB-CDC interface is passed to the second, and second to first
        #endif
      //#define USB_CDC_ECHO                                             // echo reception rather than sending to UART (when no command line interface) or to the command line interface
        #include "usb_cdc_descriptors.h"
    #elif defined USE_USB_MSD                                            // MSD (with optional mouse and/or keyboard)
        #include "usb_msd_descriptors.h"
    #elif defined USE_USB_HID_MOUSE                                      // HID mouse (with optional keyboard)
        #include "usb_mouse_descriptors.h"
    #elif defined USE_USB_HID_KEYBOARD                                   // HID keyboard
        #include "usb_keyboard_descriptors.h"
    #elif defined USE_USB_HID_RAW                                        // HID raw
        #include "usb_hid_raw_descriptors.h"
    #elif defined USE_USB_AUDIO                                          // audio
        #include "usb_audio_descriptors.h"
        #define AUDIO_DESCRIPTOR_INCLUDED
    #endif

    #if defined USE_HS_USB_AUDIO && !defined AUDIO_DESCRIPTOR_INCLUDED
        #define HS_AUDIO_VERSION
        #include "usb_audio_descriptors.h"                               // audio class on HS interface
    #endif
#undef INCLUDE_USB_DEFINES

// Interrupt events
//
#define EVENT_LUN_READY                     1
#define EVENT_MUTE_CHANGE                   2
#define EVENT_VOLUME_CHANGE                 3
#define EVENT_RETURN_PRESENT_UART_SETTING   10                           // 11, 12, etc. for additional USB-CDC interfaces
#define EVENT_COMMIT_NEW_UART_SETTING       30                           // 21, 22, etc. for additional USB-CDC interfaces
#define EVENT_HANDLE_ENCAPSULATED_COMMAND   50                           // 31, 32, etc. for additional USB-CDC-RDNIS interfaces
#define EVENT_FFT_HALF_PING                 70
#define EVENT_FFT_PING                      71
#define EVENT_FFT_HALF_PONG                 72
#define EVENT_FFT_PONG                      73
#define EVENT_RNDIS_DEVICE_CONTROL_OK       74                           // control transfer transmission has been successful (acknowledged by device)
#define EVENT_RNDIS_CONTROL_DATA_READY      75

#if defined TELIT_LE910C1
    #define MODEM_SET_LINE_CODING           80
    #define MODEM_SET_LINE_CODING_DATA      81
    #define MODEM_INITIALISATION_COMPLETE   82
#endif

#define EVENT_HUB_CONFIGURED                90
#define EVENT_HUB_CHANGED                   91
#define HUB_PORT1_CHANGED                   92
#define HUB_PORT2_CHANGED                   93
#define HUB_PORT3_CHANGED                   94
#define HUB_PORT4_CHANGED                   95
#define HUB_PORT5_CHANGED                   96
#define HUB_PORT6_CHANGED                   97
#define HUB_PORT7_CHANGED                   98

// Timer events
//
#define T_MOUSE_ACTION                      1
#define T_KEYBOARD_ACTION                   2
#define T_CHECK_DISK                        3
#define T_GET_STATUS_AFTER_STALL            4
#define T_GET_STATUS_TEST                   5
#define T_REPEAT_COMMAND                    6
#define T_REQUEST_LUN                       7
#define T_RNDIS_KEEPALIVE                   8
#define T_RNDIS_COMMAND_TIMEOUT             9
#define T_RNDIS_INTERRUPT_INTERVAL          10


#define KEYBOARD_ENUMERATED                 1
#define KEYBOARD_IN_ACTIVE                  2


#define USB_HOST_MSD_ACTIVE                 0x01
#define USB_HOST_CDC_ACTIVE                 0x02
#define USB_HOST_CDC_RNDIS_ACTIVE           0x04
#define USB_HOST_CDC_RNDIS_TELIT_LE910C1    0x08
#define USB_HOST_HUB_ACTIVE                 0x10

#define USB_RNDIS_HOST_TX_FREE_REF          0
#if defined CUSTOM_USB_TX_FREE
    #define USB_MSD_DEVICE_TX_FREE_REF      1
#else
    #define USB_MSD_DEVICE_TX_FREE_REF      0
    #if (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST) && (defined USB_DEVICE_SUPPORT && defined USE_USB_MSD)
        #error "CUSTOM_USB_TX_FREE needs to be defined for USB_RNDIS host and USB-MSD device to be used together in the same task"
    #endif
#endif


#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && !defined FIRST_CDC_INTERFACE_HANDLE && !(defined USB_DEVICE_SUPPORT && defined USE_USB_CDC)
    #define FIRST_CDC_INTERFACE_HANDLE      0                            // {41}
    #if defined USB_RNDIS_HOST && defined TELIT_LE910C1
        #define VCOM_CDC_INTERFACE_HANDLE   1
    #else
        #define VCOM_CDC_INTERFACE_HANDLE   FIRST_CDC_INTERFACE_HANDLE
    #endif
#else
    #define VCOM_CDC_INTERFACE_HANDLE       0
#endif
#if !defined RNDIS_NETWORK
    #define RNDIS_NETWORK                   0
#endif

#if defined USB_HOST_SUPPORT && defined USB_DEVICE_SUPPORT               // {47} decide the number of additional endpoints required for both host and device operations
    #if NUMBER_OF_HOST_ENDPOINTS > NUMBER_OF_ENDPOINTS
        #define NUMBER_OF_ENDPOINTS_REQUIRED (NUMBER_OF_HOST_ENDPOINTS)
    #else
        #define NUMBER_OF_ENDPOINTS_REQUIRED (NUMBER_OF_ENDPOINTS)
    #endif
#elif defined USB_HOST_SUPPORT
    #define NUMBER_OF_ENDPOINTS_REQUIRED (NUMBER_OF_HOST_ENDPOINTS)
#else
    #define NUMBER_OF_ENDPOINTS_REQUIRED (NUMBER_OF_ENDPOINTS)
#endif

#define USB_DEVICE_MODE_OF_OPERATION        0
#define USB_HOST_MODE_OF_OPERATION          1


/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */

__PACK_ON                                                                // compilers using pragmas to control packing will start struct packing from here
#define INCLUDE_USB_CONFIG_DESCRIPTOR
// We define the contents of the configuration descriptor used for our specific device and then set its contents
//
    #if defined USE_USB_CDC                                              // one or more CDC interfaces (with optional MSD, mouse and/or keyboard)
        #include "usb_cdc_descriptors.h"
    #elif defined USE_USB_MSD                                            // MSD (with optional mouse and/or keyboard)
        #include "usb_msd_descriptors.h"
    #elif defined USE_USB_HID_MOUSE                                      // HID mouse (with optional keyboard)
        #include "usb_mouse_descriptors.h"
    #elif defined USE_USB_HID_KEYBOARD                                   // HID keyboard
        #include "usb_keyboard_descriptors.h"
    #elif defined USE_USB_HID_RAW                                        // HID raw
        #include "usb_hid_raw_descriptors.h"
    #elif defined USE_USB_AUDIO                                          // audio
        #include "usb_audio_descriptors.h"
    #endif

    #if defined USE_HS_USB_AUDIO && !defined AUDIO_DESCRIPTOR_INCLUDED
        #include "usb_audio_descriptors.h"                               // audio class on HS interface
    #endif
#undef INCLUDE_USB_CONFIG_DESCRIPTOR

#if defined USB_HOST_SUPPORT
    #if defined USB_CDC_HOST && defined USB_RNDIS_HOST
        #define RNDIS_HOST_STRUCT                                        // RNDIS host structs
            #include "usb_RNDIS.h"
        #undef RNDIS_HOST_STRUCT
    #endif
    #if defined USB_HOST_HUB
        #define HUB_HOST_STRUCT                                          // hub host structs
            #include "usb_HUB.h"
        #undef HUB_HOST_STRUCT
    #endif
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */


#define INCLUDE_USB_CLASS_CONSTS
    #if defined USE_USB_CDC                                              // one or more CDC interfaces (with optional MSD, mouse and/or keyboard)
        #include "usb_cdc_descriptors.h"
    #endif
    #if defined USE_USB_MSD
        #include "usb_msd_descriptors.h"
    #endif
    #if defined USE_USB_HID_MOUSE
        #include "usb_mouse_descriptors.h"
    #endif
    #if defined USE_USB_HID_KEYBOARD
        #include "usb_keyboard_descriptors.h"
    #endif
    #if defined USE_USB_HID_RAW
        #include "usb_hid_raw_descriptors.h"
    #endif
    #if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
        #include "usb_audio_descriptors.h"
    #endif
#undef INCLUDE_USB_CLASS_CONSTS

#define INCLUDE_USB_DEVICE_DESCRIPTOR
    #if defined USE_USB_CDC                                              // one or more CDC interfaces (with optional MSD, mouse and/or keyboard)
        #include "usb_cdc_descriptors.h"
    #elif defined USE_USB_MSD                                            // MSD (with optional mouse and/or keyboard)
        #include "usb_msd_descriptors.h"
    #elif defined USE_USB_HID_MOUSE                                      // HID mouse (with optional keyboard)
        #include "usb_mouse_descriptors.h"
    #elif defined USE_USB_HID_KEYBOARD                                   // HID keyboard
        #include "usb_keyboard_descriptors.h"
    #elif defined USE_USB_HID_RAW                                        // HID raw
        #include "usb_hid_raw_descriptors.h"
    #elif defined USE_USB_AUDIO                                          // audio
        #include "usb_audio_descriptors.h"
    #endif

    #if defined USE_HS_USB_AUDIO && !defined AUDIO_DESCRIPTOR_INCLUDED
        #include "usb_audio_descriptors.h"
    #endif
#undef INCLUDE_USB_DEVICE_DESCRIPTOR

#if !defined FIRST_CDC_RNDIS_HANDLE
    #define FIRST_CDC_RNDIS_HANDLE                0
#endif
#if !defined FIRST_CDC_RNDIS_INTERFACE
    #define FIRST_CDC_RNDIS_INTERFACE             1
#endif
#if !defined FIRST_CDC_RNDIS_INTERRUPT_INTERFACE
    #define FIRST_CDC_RNDIS_INTERRUPT_INTERFACE   0
#endif
#if !defined RNDIS_DATA_ENDPOINT_SIZE
    #define RNDIS_DATA_ENDPOINT_SIZE              usCDCBulkInEndpointSize[FIRST_CDC_RNDIS_INTERFACE]
#endif

#if defined USE_USB_CDC && defined USB_CDC_RNDIS                         // RNDIS device
    #define RNDIS_DEVICE_CONSTS                                          // RNDIS device consts
        #include "usb_RNDIS.h"
    #undef RNDIS_DEVICE_CONSTS
#endif

#if defined USB_HOST_SUPPORT
    #if defined USB_MSD_HOST
        #define MSD_HOST_CONST                                           // MSD host consts
            #include "usb_MSD.h"
        #undef MSD_HOST_CONST
    #endif
    #if defined USB_RNDIS_HOST
        #define RNDIS_HOST_CONST                                         // RNDIS host consts
            #include "usb_RNDIS.h"
        #undef RNDIS_HOST_CONST
    #endif
    #if defined USB_HOST_HUB
        #define HUB_HOST_CONST                                           // hub host consts
            #include "usb_HUB.h"
        #undef HUB_HOST_CONST
    #endif
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

#if defined MODBUS_USB_SLAVE                                             // {11}
    QUEUE_HANDLE USBPortID_comms[USB_CDC_COUNT] = {NO_ID_ALLOCATED};     // USB port endpoint handle (exported for use by MODBUS module)
#endif
#if defined USE_USB_HID_KEYBOARD && defined SUPPORT_FIFO_QUEUES
    QUEUE_HANDLE keyboardQueue = NO_ID_ALLOCATED;
#endif
#if defined TEST_PIT_SINE_TO_USB
    USBQUE *ptrUsbCDC_TxQueue = 0;                                       // USB-CDC output queue
#endif
#if (defined USB_RNDIS_HOST || defined USB_CDC_RNDIS) && defined USB_TO_TCP_IP
    QUEUE_HANDLE RNDIS_Handle = NO_ID_ALLOCATED;                         // handle to RNDIS interface
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if defined USB_HOST_SUPPORT && (defined USB_DEVICE_SUPPORT || (defined _iMX && (USBS_AVAILABLE > 1))) // {47}
    static int iUSB_mode[USB_CONTROLLERS] = {USB_DEVICE_MODE_OF_OPERATION}; // default to device mode
#endif

static QUEUE_HANDLE USB_control[USB_CONTROLLERS] = {NO_ID_ALLOCATED};    // USB default control endpoint handle
static unsigned short usExpectedData[USB_CONTROLLERS] = {0};             // used by the control endpoint to collect data
static unsigned char  ucCollectingMode[USB_CONTROLLERS] = {
    0xff,
#if USB_CONTROLLERS > 1
    0xff,
#endif
};
#if defined USB_HOST_SUPPORT
    static unsigned long ulHostClassTypes = 0;                           // {44}
    static unsigned char ucHostSpeed = 0;
#endif
#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    static RNDIS_HOST_INSTANCE sRNDIS_Host = {0};
    static unsigned short usControlEndpointSize = 0;
#endif
#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
    static QUEUE_HANDLE USBPortID_msd_host = NO_ID_ALLOCATED;
#endif
#if (defined USE_USB_CDC || defined USB_CDC_HOST)
    #if !defined CDC_WITHOUT_INTERRUPT_ENDPOINT
    static QUEUE_HANDLE USBPortID_interrupt[USB_CDC_COUNT] = {NO_ID_ALLOCATED}; // interrupt endpoint handle(s)
    #endif
    #if !defined MODBUS_USB_SLAVE                                        // {11}
    static QUEUE_HANDLE USBPortID_comms[USB_CDC_COUNT] = {NO_ID_ALLOCATED}; // USB port endpoint handle(s)
    #endif
#endif
#if defined USE_USB_HID_MOUSE
    static QUEUE_HANDLE USBPortID_HID_mouse = NO_ID_ALLOCATED;           // USB handle for HID mouse communication
#endif
#if defined USE_USB_HID_RAW
    static QUEUE_HANDLE USBPortID_HID_raw = NO_ID_ALLOCATED;             // USB handle for HID raw communication
#endif
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    static QUEUE_HANDLE USBPortID_Audio = NO_ID_ALLOCATED;               // USB handle for audio data
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
    static QUEUE_HANDLE USBPortID_HID_kb = NO_ID_ALLOCATED;              // USB handle for HID keyboard communication
#endif
#if defined USE_USB_MSD                                                  // local variables required by USB-MSD
    static QUEUE_HANDLE USBPortID_MSD = NO_ID_ALLOCATED;                 // USB handle for MSD communication
    #if !defined SDCARD_SUPPORT && !defined SPI_FLASH_FAT && !defined FLASH_FAT && (!defined FAT_EMULATION && NUMBER_USB_MSD == 1)
        #error USB-MSD needs utFAT or FAT emulation enabled !!
    #endif
    static unsigned long ulLogicalBlockAdr = 0;                          // present logical block address (shared between read and write)
    static unsigned long ulReadBlock = 0;                                // the outstanding blocks to be read from the media
    static unsigned long ulWriteBlock = 0;                               // the outstanding blocks to be written to the media
    static int iWriteInProgress = 0;                                     // {46} flag indicating that reception is write data and not commands
    static int iContent = 0;
    static const UTDISK *ptrDiskInfo[DISK_COUNT] = {0};
    static USB_MASS_STORAGE_CSW csw = {{'U', 'S', 'B', 'S'}, {0}, {0}, CSW_STATUS_COMMAND_PASSED };
    static unsigned char ucActiveLUN = 0;
#endif
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    signed long slDelta = 0;                                             // deviation from optimal audio synchronisation relationship (USB host to audio time base)
#endif
#if defined USB_STRING_OPTION && defined USB_RUN_TIME_DEFINABLE_STRINGS
    static USB_STRING_DESCRIPTOR *SerialNumberDescriptor;                // string built up to contain a variable serial number
#endif
#define RNDIS_LOCAL_VARIABLES
    #include "usb_RNDIS.h"
#undef RNDIS_LOCAL_VARIABLES
#define MSD_LOCAL_VARIABLES
    #include "usb_MSD.h"
#undef MSD_LOCAL_VARIABLES
#if defined USE_USB_HID_KEYBOARD
    #define KEYBOARD_LOCAL_VARIABLES
        #include "usb_Keyboard.h"
    #undef KEYBOARD_LOCAL_VARIABLES
#endif
#if defined USE_USB_HID_MOUSE
    #define MOUSE_LOCAL_VARIABLES
        #include "usb_Mouse.h"
    #undef MOUSE_LOCAL_VARIABLES
#endif
#if defined USB_DEVICE_SUPPORT && defined USE_USB_HID_RAW
    #define HID_RAW_LOCAL_VARIABLES
        #include "usb_RAW.h"
    #undef HID_RAW_LOCAL_VARIABLES
#endif
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    #define AUDIO_LOCAL_VARIABLES
        #include "usb_Audio.h"
    #undef AUDIO_LOCAL_VARIABLES
#endif
#if defined USE_USB_CDC
    #define CDC_LOCAL_VARIABLES
        #include "usb_CDC.h"
    #undef CDC_LOCAL_VARIABLES
#endif
#if defined USB_HOST_SUPPORT
    #if defined USB_CDC_HOST                                             // {37}
    static unsigned char  ucCDCBulkOutEndpoint[USB_CDC_COUNT] = {0};
    static unsigned short usCDCBulkOutEndpointSize[USB_CDC_COUNT] = {0};
    static unsigned char  ucCDCBulkInEndpoint[USB_CDC_COUNT] = {0};
    static unsigned short usCDCBulkInEndpointSize[USB_CDC_COUNT] = {0};
        #if defined USB_RNDIS_HOST
    static unsigned char  ucCDC_InterruptEndpoint[USB_CDC_COUNT] = {0};
    static DELAY_LIMIT    CDC_InterruptPollingRate[USB_CDC_COUNT] = {0};
    static unsigned short usCDC_InterruptEndpointSize[USB_CDC_COUNT] = {0};
        #endif
    #endif

    #if defined TELIT_LE910C1
    static unsigned long  ulInterfaceFilter[USB_CONTROLLERS] = {0};
    #endif

    #if defined USB_HOST_HUB
    static unsigned char  ucHub_InterruptEndpoint = 0;
    static unsigned short usHub_InterruptEndpointSize = 0;
    static DELAY_LIMIT    HUB_InterruptPollingRate = 0;
    static QUEUE_HANDLE   USBPortID_hub_host = NO_ID_ALLOCATED;          // hub interrupt endpoint
    #endif
#endif


/* =================================================================== */
/*                      local function definitions                     */
/* =================================================================== */

static QUEUE_HANDLE fnConfigureUSB(int iChannel);                        // routine to open and configure the USB interface(s)

#if defined USB_STRING_OPTION && defined USB_RUN_TIME_DEFINABLE_STRINGS
    static void fnSetSerialNumberString(CHAR *ptrSerialNumber);
#endif
#if defined USE_USB_MSD
    static void fnContinueMedia(void);
    static unsigned char *fnGetPartition(unsigned char ucLUN, QUEUE_TRANSFER *length);
#endif
#if defined USE_USB_CDC
    #if defined USB_CDC_RNDIS
        static void fnResetRNDIS(void);
        static void fnHandleRNDIS_EncapsulatedCommand(int iInterface);
        static void fnRNDIS_report_link_state(void);
    #endif
    #if defined USB_DEVICE_SUPPORT && !defined USB_CDC_INTERFACE_PASSTHROUGH && !defined USB_CDC_ECHO
        static void fnHandleUSB_command_input(unsigned char ucInputMessage[LARGE_MESSAGE], QUEUE_TRANSFER Length);
    #endif
#endif
#if (defined USE_USB_CDC && defined USB_CDC_RNDIS && (defined USB_TO_ETHERNET || defined USB_TO_TCP_IP)) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST)
    static QUEUE_TRANSFER fnSendRNDIS_Data(QUEUE_HANDLE rndis_handle, unsigned char *ptrData, REMOTE_NDIS_ETHERNET_PACKET_MESSAGE *ptrRNDIS_message, unsigned short usEthernetLength);
#endif
#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    static void fnDisplayUSB_Interface(unsigned char ucEvent);
#endif
#if defined USB_HOST_SUPPORT
    static void fnConfigureHostApplicationEndpoints(unsigned char ucConfiguration);
    static unsigned char fnDisplayDeviceInfo(unsigned long *ptrDeviceType, int iChannel);
    #if defined USB_MSD_HOST
        static void fnRequestLUN(int iChannel);
        static int fnSendMSD_host(unsigned char ucOpcode);
    #endif
    #if defined USB_CDC_HOST && defined USB_RNDIS_HOST
        static int fnRNDIS_Control(unsigned long ulMessage);
        static void fnInterrupEndpointPoll(void);
        static void fnPollRNDIS_data(void);
        static int fnRNDIS_HostManagement(int iEvent);
        #if defined TELIT_LE910C1
        static int fnTelitModemStateEvent(int iEvent, unsigned char ucEndpoint);
            #define TELIT_ENUMERATED         0
            #define TELIT_ENDPOINT_CLEARED   1
            #define TELIT_STATUS_STAGE_OK    2
            #define TELIT_SETUP_ACKED        3
        #endif
    #endif
#endif
#define RNDIS_FUNCTIONS                                                  // RNDIS functions
    #include "usb_RNDIS.h"
#undef RNDIS_FUNCTIONS
#define CDC_FUNCTIONS                                                    // CDC functions
    #include "usb_CDC.h"
#undef CDC_FUNCTIONS
#define MSD_FUNCTIONS                                                    // MSD functions
    #include "usb_MSD.h"
#undef MSD_FUNCTIONS
#if defined USB_HOST_SUPPORT && defined USB_HOST_HUB
    #define HUB_FUNCTIONS                                                // HUB functions
        #include "usb_HUB.h"
    #undef HUB_FUNCTIONS
#endif
#if defined USE_USB_HID_KEYBOARD
    #define KEYBOARD_FUNCTIONS                                           // HID keyboard functions
        #include "usb_Keyboard.h"
    #undef KEYBOARD_FUNCTIONS
#endif
#if defined USE_USB_HID_MOUSE
    #define MOUSE_FUNCTIONS                                              // HID mouse functions
        #include "usb_Mouse.h"
    #undef MOUSE_FUNCTIONS
#endif
#if defined USB_DEVICE_SUPPORT && defined USE_USB_HID_RAW
    #define HID_RAW_FUNCTIONS                                            // HID raw functions
        #include "usb_RAW.h"
    #undef HID_RAW_FUNCTIONS
#endif
#if defined USB_DEVICE_SUPPORT
    #define AUDIO_FUNCTIONS                                               // Audio class functions
        #include "usb_Audio.h"
    #undef AUDIO_FUNCTIONS
#endif


/* =================================================================== */
/*                                task                                 */
/* =================================================================== */


// USB task
//
extern void fnTaskUSB(TTASKTABLE *ptrTaskTable)
{
    QUEUE_HANDLE PortIDInternal = ptrTaskTable->TaskID;                  // queue ID for task input
    unsigned char ucInputMessage[LARGE_MESSAGE];                         // reserve space for receiving messages
#if defined USB_DEVICE_SUPPORT && defined FREEMASTER_CDC && (USB_CDC_VCOM_COUNT == 1)
    QUEUE_TRANSFER Length;
#elif defined USB_DEVICE_SUPPORT && ((defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0)) && defined USE_MAINTENANCE) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && (!defined USB_RNDIS_HOST || defined TELIT_LE910C1))
    #if defined USB_CDC_VCOM_COUNT && (USB_CDC_VCOM_COUNT > (MODBUS_USB_CDC_COUNT + 1))
    QUEUE_TRANSFER Length;
    #endif
#elif defined USB_DEVICE_SUPPORT && defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0)
    QUEUE_TRANSFER Length;
#endif
#if defined USE_USB_CDC                                                  // {11}
    #if (USB_CDC_VCOM_COUNT > 1) && ((USB_CDC_VCOM_COUNT > (MODBUS_USB_CDC_COUNT + 1)) || !defined USE_MAINTENANCE)
    int iCDC_input;
    #endif
#endif
#if defined USB_HOST_SUPPORT && !defined USB_DEVICE_SUPPORT
    if (USB_control[USB_HOST_REF] == NO_ID_ALLOCATED)                    // initialisation is performd on first call only
#else
    if (USB_control[USB_DEVICE_REF] == NO_ID_ALLOCATED)                  // initialisation is performd on first call only
#endif
    {
#if defined USE_USB_HID_KEYBOARD && defined SUPPORT_FIFO_QUEUES
        keyboardQueue = fnOpen(TYPE_FIFO, FOR_I_O, (FIFO_LENGTH)64);     // open a FIFO queue (of 64 bytes length) for receiving input to be sent from the keyboard
#endif
#if defined FRDM_KL27Z && defined _DEV2
        fnInitialiseDEV2();
#elif defined USB_STRING_OPTION && defined USB_RUN_TIME_DEFINABLE_STRINGS // if dynamic strings are supported, prepare a specific serial number ready for enumeration
        fnSetSerialNumberString(temp_pars->temp_parameters.cDeviceIDName); // construct a serial number string for USB use
#endif
#if defined USE_USB_CDC && defined SERIAL_INTERFACE && defined USB_SERIAL_CONNECTIONS && (USB_CDC_VCOM_COUNT > 1)
        fnOpenUSB_UARTs(0);                                              // configure and open additional UARTs to be used with USB CDC connection
#endif
#if defined USB_DEVICE_SUPPORT && !defined USB_HOST_SUPPORT
        USB_control[USB_DEVICE_REF] = fnConfigureUSB(USB_DEVICE_REF);    // configure the USB interface for device mode of operation
    #if defined PARALLEL_USD_DEVICE_INTERFACE
        USB_control[USB_HS_DEVICE_REF] = fnConfigureUSB(USB_HS_DEVICE_REF); // configure the high speed USB interface
    #endif
#endif
#if defined USB_HOST_SUPPORT
    #if defined USB_DEVICE_SUPPORT
        USB_control[USB_DEVICE_REF] = fnConfigureUSB(USB_DEVICE_REF);    // configure the USB device interface
        #if defined _iMX && defined USB_HOST_SUPPORT && (USBS_AVAILABLE > 1) // host in parallel with device is only possible when dual USB is available
        iUSB_mode[USB_HOST_REF] = USB_HOST_MODE_OF_OPERATION;            // configure host interface
        USB_control[USB_HOST_REF] = fnConfigureUSB(USB_HOST_REF);        // configure the USB host interface
        #endif
    #else
        USB_control[USB_HOST_REF] = fnConfigureUSB(USB_HOST_REF);        // configure the USB interface for host mode of operaton
        #if defined _iMX && defined USB_HOST_SUPPORT && (USBS_AVAILABLE > 1)
        iUSB_mode[USB_HOST_REF] = USB_HOST_MODE_OF_OPERATION;            // host interface
        #endif
    #endif
#endif
#if defined MODBUS_USB_SLAVE                                             // {11}
        fnInitModbus();                                                  // initialise MODBUS since the USB handle is now ready
#endif
#if defined USE_USB_HID_MOUSE                                            // {23}
        CONFIGURE_MOUSE_INPUTS();
#endif
#if defined USE_USB_MSD
        do {
            ptrDiskInfo[ucActiveLUN] = fnGetDiskInfo(ucActiveLUN);       // get a pointer to each disk information for local use
    #if defined FAT_EMULATION && !defined SDCARD_SUPPORT
            fnPrepareRootDirectory(ptrDiskInfo[ucActiveLUN], -1);        // generate a root directory for subsequent FAT emulation
    #endif
        } while (++ucActiveLUN < DISK_COUNT);                            // repeat for each disk
#endif
    }

    // Handle the task's input queue
    //
    while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH) != 0) { // check task input queue
        switch (ucInputMessage[MSG_SOURCE_TASK]) {                       // switch depending on source
        case TIMER_EVENT:
            switch (ucInputMessage[MSG_TIMER_EVENT]) {
#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
    #define MSD_TIMER_EVENTS
        #include "usb_MSD.h"
    #undef MSD_TIMER_EVENTS
#endif
#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    #define RNDIS_HOST_TIMER_EVENTS
        #include "usb_RNDIS.h"
    #undef RNDIS_HOST_TIMER_EVENTS
#endif
#if defined USE_USB_HID_KEYBOARD  && !defined IN_COMPLETE_CALLBACK       // {34}
    #define KEYBOARD_DEVICE_TIMER_EVENTS
        #include "usb_Keyboard.h"
    #undef KEYBOARD_DEVICE_TIMER_EVENTS
#endif
#if defined USE_USB_HID_MOUSE
    #define MOUSE_DEVICE_TIMER_EVENTS
        #include "usb_Mouse.h"
    #undef MOUSE_DEVICE_TIMER_EVENTS
#endif
            }
            break;
        case INTERRUPT_EVENT:                                            // interrupt event without data
            switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {               // specific interrupt event type
#if (defined _iMX && (USBS_AVAILABLE > 1)) || defined PARALLEL_USD_DEVICE_INTERFACE
            case EVENT_USB_RESET_1:
#endif
            case EVENT_USB_RESET:                                        // active USB connection has been reset (device mode)
                fnDebugMsg("USB ");                                      // display that the USB bus has been reset
#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_INTERRUPT_EVENT]);
#endif
                fnDebugMsg("Reset\n\r");
                DEL_USB_SYMBOL();                                        // {21} optionally display the new USB state
#if defined USE_USB_HID_MOUSE                                            // {23}
                fnResetMouseDevice(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESET);
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                fnResetKeyboardDevice(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESET);
#endif
#if defined USE_USB_MSD
                fnResetMSD_DeviceVariables(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESET); // reset local variables on each reset
#endif
#if defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0) && defined USE_MAINTENANCE
                fnResetCDC_commandLine(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESET);
#endif
#if defined  TEST_PIT_SINE_TO_USB
                fnUSB_stream_stop(1);
#endif
                break;

#if (defined _iMX && (USBS_AVAILABLE > 1)) || defined PARALLEL_USD_DEVICE_INTERFACE
            case EVENT_USB_SUSPEND_1:
#endif
            case EVENT_USB_SUSPEND:                                      // a suspend condition has been detected. A bus powered device should reduce consumption to <= 500uA or <= 2.5mA (high power device)
                fnDebugMsg("USB ");
#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_INTERRUPT_EVENT]);
#endif
                fnDebugMsg("Suspended\n\r");
                DEL_USB_SYMBOL();                                        // {21} optionally display the new USB state
#if defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0) && defined USE_MAINTENANCE
                fnResetCDC_commandLine(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_SUSPEND);
#endif
#if defined USE_USB_HID_MOUSE
                fnResetMouseDevice(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_SUSPEND);
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                fnResetKeyboardDevice(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_SUSPEND);
#endif
#if defined  TEST_PIT_SINE_TO_USB
                fnUSB_stream_stop(1);
#endif
                fnSetUSBConfigState(USB_DEVICE_SUSPEND, 0, (ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_SUSPEND)); // set all endpoint states to suspended
                break;

#if (defined _iMX && (USBS_AVAILABLE > 1)) || defined PARALLEL_USD_DEVICE_INTERFACE
            case EVENT_USB_RESUME_1:
#endif
            case EVENT_USB_RESUME:                                       // a resume sequence has been detected so full power consumption can be resumed
                fnSetUSBConfigState(USB_DEVICE_RESUME, 0, (ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESUME)); // remove suspended state from all endpoints
                fnDebugMsg("USB ");
#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_INTERRUPT_EVENT]);
#endif
                fnDebugMsg("Resume\n\r");
                SET_USB_SYMBOL();                                        // {21} optionally display the new USB state
#if defined USE_USB_HID_MOUSE                                            // {23}
                fnMouseEnumerated(ucInputMessage[MSG_CONTENT_COMMAND] - EVENT_USB_RESUME);
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                fnKeyboardEnumerated(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_RESUME);
#endif
                break;
#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
            case (TX_FREE - USB_RNDIS_HOST_TX_FREE_REF):                 // RNDIS data packet transmission has completed
                fnRNDIS_Data_Tx_Complete();
                break;
#endif
#if defined USB_DEVICE_SUPPORT && defined USE_USB_MSD
            case (TX_FREE - USB_MSD_DEVICE_TX_FREE_REF):                 // the USB tx buffer has enough space to accept more data
                fnContinueMedia();                                       // the output buffer has space so continue with data transfer
                break;
#endif
#if defined USB_HOST_SUPPORT                                             // {32}
            case EVENT_USB_DETECT_HS_1:
            case EVENT_USB_DETECT_HS:
            case EVENT_USB_DETECT_FS_1:
            case EVENT_USB_DETECT_FS:
            case EVENT_USB_DETECT_LS_1:
            case EVENT_USB_DETECT_LS:
                fnDebugMsg("USB ");
                ucHostSpeed = ucInputMessage[MSG_INTERRUPT_EVENT];
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_INTERRUPT_EVENT]);
    #endif
                if (EVENT_USB_DETECT_HS == (ucInputMessage[MSG_INTERRUPT_EVENT] & USB_EVENT_INTERFACE_MASK)) {
                    fnDebugMsg("H");
                }
                else if (EVENT_USB_DETECT_FS == (ucInputMessage[MSG_INTERRUPT_EVENT] & USB_EVENT_INTERFACE_MASK)) {
                    fnDebugMsg("F");
                }
                else {
                    fnDebugMsg("L");
                }
                fnDebugMsg("S device detected\r\n");
                break;
            case EVENT_USB_REMOVAL_1:
            case EVENT_USB_REMOVAL:
    #if defined USB_CDC_HOST && defined USE_MAINTENANCE
                usUSB_state &= ~(ES_USB_RS232_MODE);
    #endif
                fnDebugMsg("USB ");
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_INTERRUPT_EVENT]);
    #endif
                fnDebugMsg("device removed\r\n");
    #if defined USB_CDC_HOST && defined USB_RNDIS_HOST
                fnRNDIS_device_remove((int)(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_REMOVAL)); // handle the removal to reset the RNDIS host instance
    #endif
    #if defined USB_MSD_HOST
                if ((ulHostClassTypes & USB_HOST_MSD_ACTIVE) != 0) {     // if the USB MSD host mode was active
                    fnMSD_device_remove(&USBPortID_msd_host, (int)(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_REMOVAL)); // handle the removal to reset the MSD host instance
                }
    #endif
    #if defined USB_HOST_HUB
                if ((ulHostClassTypes & USB_HOST_HUB_ACTIVE) != 0) {     // hub removed
                    fnHubStateMachine(HUB_DEVICE_DISCONNECTED, 0, 0);
                }
    #endif
                usExpectedData[(int)(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_REMOVAL)] = 0;
                ucCollectingMode[(int)(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_USB_REMOVAL)] = 0xff;
                ulHostClassTypes = 0;
                break;
    #if defined USB_CDC_HOST && defined USB_RNDIS_HOST
        #define RNDIS_HOST_INTERRUPT_EVENTS
            #include "usb_RNDIS.h"
        #undef RNDIS_HOST_INTERRUPT_EVENTS
    #endif
    #if defined USB_HOST_HUB
        #define HUB_INTERRUPT_EVENTS
            #include "usb_HUB.h"
        #undef HUB_INTERRUPT_EVENTS
    #endif
    #if defined USB_MSD_HOST
        #define MSD_INTERRUPT_EVENTS
            #include "usb_MSD.h"
        #undef MSD_INTERRUPT_EVENTS
    #endif
    #if defined USE_USB_HID_KEYBOARD && defined IN_COMPLETE_CALLBACK
        #define KEYBOARD_INTERRUPT_EVENTS
            #include "usb_Keyboard.h"
        #undef KEYBOARD_INTERRUPT_EVENTS
    #endif
#endif
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    #define AUDIO_INTERRUPT_EVENTS
        #include "usb_Audio.h"
    #undef AUDIO_INTERRUPT_EVENTS
#endif
#if defined USE_USB_CDC
            default:
    #if defined USB_CDC_RNDIS
                if (ucInputMessage[MSG_INTERRUPT_EVENT] >= EVENT_HANDLE_ENCAPSULATED_COMMAND) {
                    fnHandleRNDIS_EncapsulatedCommand(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_HANDLE_ENCAPSULATED_COMMAND); // handle the prepared encapsulated command
                    break;
                }
    #endif
    #if USB_CDC_VCOM_COUNT > 0
                if (ucInputMessage[MSG_INTERRUPT_EVENT] >= EVENT_COMMIT_NEW_UART_SETTING) {
                    fnCommit_UART(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_COMMIT_NEW_UART_SETTING); // set the serial interface characteristics
                }
                else if (ucInputMessage[MSG_INTERRUPT_EVENT] >= EVENT_RETURN_PRESENT_UART_SETTING) {
                    fnReturnUART_settings(ucInputMessage[MSG_INTERRUPT_EVENT] - EVENT_RETURN_PRESENT_UART_SETTING); // return the present serial interface settings
                }
    #endif
                break;
#endif
            }                                                            // end interrupt event switch
            break;

        case TASK_USB:                                                   // USB interrupt handler is requesting us to perform work offline
            fnRead(PortIDInternal, &ucInputMessage[MSG_CONTENT_COMMAND], ucInputMessage[MSG_CONTENT_LENGTH]); // get the content
            switch (ucInputMessage[MSG_CONTENT_COMMAND]) {
#if defined USB_HOST_SUPPORT                                             // {32}
                case E_USB_DEVICE_INFO_1:
                case E_USB_DEVICE_INFO:                                  // the USB host has collected information ready to decide how to work with a device
                    fnDebugMsg("USB device information ready:\r\n");
                    ucInputMessage[0] = fnDisplayDeviceInfo(&ulHostClassTypes, (ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_DEVICE_INFO)); // analyse and display information
                    if (ucInputMessage[0] != 0) {                        // if there is a supported configuration to be enabled
    #if defined USB_MSD_HOST                        
                        ulTag = fnRandom();                              // start with random tag number
                        ulTag <<= 16;
                        ulTag |= fnRandom();
    #endif
    #if defined USB_HOST_HUB
                        if ((ulHostClassTypes & USB_HOST_HUB_ACTIVE) != 0) { // hub connected
                            fnHubStateMachine(HUB_DEVICE_CONNECTED, ucInputMessage, 1);
                            break;
                        }
    #endif
                        fnDriver(USB_control[USB_HOST_REF], (TX_ON), ucInputMessage[0]); // enable configuration - this causes the generic USB layer to send SetConfiguration (acknowledged with the event E_USB_ACTIVATE_CONFIGURATION))
                    }
                    break;
                case E_USB_DEVICE_STALLED_1:
                case E_USB_DEVICE_STALLED:
                    fnDebugMsg("Stall ");
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                    fnDisplayUSB_Interface(ucInputMessage[MSG_CONTENT_COMMAND]);
    #endif
                    fnDebugMsg("on EP-");
                    fnDebugDec((ucInputMessage[MSG_CONTENT_COMMAND + 1] & ~IN_ENDPOINT), 0); // display the endpoint that stalled
                    fnDebugMsg("\n\r");
                    fnClearFeature(USB_control[USB_HOST_REF], ucInputMessage[MSG_CONTENT_COMMAND + 1], USB_HOST_REF); // clear the stalled endpoint
                    break;
                case E_USB_DEVICE_CLEARED_1:
                case E_USB_DEVICE_CLEARED:
                    if ((ucInputMessage[MSG_CONTENT_COMMAND + 1] & IN_ENDPOINT) != 0) {
                        fnDebugMsg("IN ");
                    }
                    else {
                        fnDebugMsg("OUT ");
                    }
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                    fnDisplayUSB_Interface(ucInputMessage[MSG_CONTENT_COMMAND]);
    #endif
                    fnDebugMsg("EP-");
                    fnDebugDec((ucInputMessage[MSG_CONTENT_COMMAND + 1] & ~IN_ENDPOINT), 0); // display the endpoint number that was cleared
                    fnDebugMsg(" cleared\n\r");
    #if defined USB_MSD_HOST
                    if ((ulHostClassTypes & USB_HOST_MSD_ACTIVE) != 0) {
                        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), T_GET_STATUS_AFTER_STALL); // collect the status after a short delay
                        break;
                    }
    #endif
    #if defined USB_RNDIS_HOST && defined TELIT_LE910C1
                    if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_TELIT_LE910C1) != 0) {
                        fnTelitModemStateEvent(TELIT_ENDPOINT_CLEARED, ucInputMessage[MSG_CONTENT_COMMAND + 1]);
                    }
    #endif
                    break;
                case E_USB_HOST_ERROR_REPORT_1:
                case E_USB_HOST_ERROR_REPORT:
    #if defined USB_RNDIS_HOST
                    if (ucInputMessage[MSG_CONTENT_COMMAND + 1] == (USB_HOST_NACK_PID_DETECTED | (ucCDC_InterruptEndpoint[FIRST_CDC_RNDIS_INTERFACE] << 4))) {
                        // The device returned a NAK when we tried polling its interrupt endpoint
                        //
                        fnDebugMsg("N");
                        break;
                    }
   #endif
                    fnDebugMsg("USB Host error: EP-");
                    fnDebugDec((ucInputMessage[MSG_CONTENT_COMMAND + 1] >> 4), 0);
                    fnDebugMsg(" Code: ");
                    fnDebugDec((ucInputMessage[MSG_CONTENT_COMMAND + 1] & 0x0f), WITH_CR_LF);
                    // Attempt a device reset in case it was a spurious error
                    //
                  //fnUSB_ResetCycle();
                    break;
#endif
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
    #define AUDIO_DEVICE_USB_EVENTS
        #include "usb_Audio.h"
    #undef AUDIO_DEVICE_USB_EVENTS
#endif
#if (defined _iMX && (USBS_AVAILABLE > 1)) || defined PARALLEL_USD_DEVICE_INTERFACE
            case E_USB_ACTIVATE_CONFIGURATION_1:                         // HS USB controller 1
#endif
            case E_USB_ACTIVATE_CONFIGURATION:                           // USB controller 0
                fnDebugMsg("Enumerated ");                               // the interface has been activated and enumeration has completed
#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
                fnDisplayUSB_Interface(ucInputMessage[MSG_CONTENT_COMMAND]);
#endif
                fnDebugMsg("[");
                fnDebugDec(ucInputMessage[MSG_CONTENT_DATA_START], 1);   // {10} the configuration
                fnDebugMsg("]\n\r");
                SET_USB_SYMBOL();                                        // {21} display connection in LCD, on LED etc.
#if defined _WINDOWS && defined TEST_PIT_SINE_TO_USB
                fnUSB_stream_start();
#endif
#if defined USB_HOST_SUPPORT                                             // {32}
    #if defined _iMX && (USBS_AVAILABLE > 1)
                if (iUSB_mode[ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_ACTIVATE_CONFIGURATION] == USB_HOST_MODE_OF_OPERATION) { // if the host interface has enumerated
    #endif
                    fnConfigureHostApplicationEndpoints(ucInputMessage[MSG_CONTENT_COMMAND + 1]); // configure endpoints according to configuration
    #if defined USB_MSD_HOST
                    iUSB_MSD_OpCode = 0;
                    if ((ulHostClassTypes & USB_HOST_MSD_ACTIVE) != 0) { // {44}
                        fnRequestLUN(ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_ACTIVATE_CONFIGURATION); // the first thing that the MSD host does is request the number of logical units that the disk drive has
                    }
    #endif
    #if defined USB_CDC_HOST
                    if ((ulHostClassTypes & USB_HOST_CDC_ACTIVE) != 0) { // {44} if the connected device is an RNDIS device
        #if defined USB_RNDIS_HOST
                        if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_ACTIVE) != 0) {
            #if defined TELIT_LE910C1
                            if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_TELIT_LE910C1) != 0) {
                                fnTelitModemStateEvent(TELIT_ENUMERATED, 0);
                                break;
                            }
            #endif
                            // After enumeration in RNDIS host mode the host starts the initialisation with REMOTE_NDIS_INITIALIZE_MSG
                            //
                            fnRNDIS_Control(REMOTE_NDIS_INITIALIZE_MSG);
            #if !defined _iMX && !defined USB_HS_INTERFACE               // only used for non-HS controllers
                            uTaskerGlobalMonoTimer(OWN_TASK, CDC_InterruptPollingRate[FIRST_CDC_RNDIS_INTERFACE], T_RNDIS_INTERRUPT_INTERVAL); // start the interrupt endpoint interval
            #endif
                            break;
                        }
                        fnPollRNDIS_data();
        #else
                        fnDriver(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], (RX_ON), 0); // {43} enable IN polling on first interface
        #endif
                    }
    #endif
    #if defined USB_HOST_HUB
                    if ((ulHostClassTypes & USB_HOST_HUB_ACTIVE) != 0) {
                        fnHubStateMachine(HUB_DEVICE_ACTIVATED, 0, 0);   // start configuring the hub
                    }
    #endif
    #if defined _iMX && (USBS_AVAILABLE > 1)
                }
    #endif
#elif defined USE_USB_CDC && !defined USB_CDC_INTERFACE_PASSTHROUGH && !defined USB_CDC_ECHO && !defined SERIAL_INTERFACE
                fnHandleUSB_command_input((unsigned char *)"\r", 1);     // when there is no serial interface we try to connect the new USB interface to the debug handle so that debug messages are immediately seen
#endif
#if (defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO) && defined MIC_ONLY && !defined SPEAKER_ONLY // {52}
                fnEnableAudio(ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_ACTIVATE_CONFIGURATION); // allow synchronised operation to restart after re-enumeration has taken place
#endif
#if defined USE_USB_HID_MOUSE
                fnMouseEnumerated(ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_ACTIVATE_CONFIGURATION);
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                fnKeyboardEnumerated(ucInputMessage[MSG_CONTENT_COMMAND] - E_USB_ACTIVATE_CONFIGURATION);
#endif
#if defined USE_USB_CDC && defined USB_CDC_RNDIS
                fnResetRNDIS();                                          // reset RNDIS variables
#endif
            }
            break;

        default:
            break;
        }
    }

    // Tasks performed each time the USB task is executed
    //
#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
    if (USBPortID_msd_host != NO_ID_ALLOCATED) {                         // only check bulk input queue when the host is configured
        fnHandleMSD_device_reception(USBPortID_msd_host, ucInputMessage);// handle potential MSD data reception from device
    }
#endif
#if defined USB_DEVICE_SUPPORT && defined USE_USB_MSD                    // usb-msd device mode
    if (USBPortID_MSD != NO_ID_ALLOCATED) {
        fnHandleMSD_host_reception(USBPortID_MSD, ucInputMessage);       // handle potential USB-MSD host receptions
    }
#endif
#if (defined USB_DEVICE_SUPPORT && defined USE_USB_CDC && defined USB_CDC_RNDIS) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST) // RNDIS device/host mode
    if (USBPortID_comms[FIRST_CDC_RNDIS_HANDLE] != NO_ID_ALLOCATED) {    // if RNDIS host data interface is configured
        fnRNDIS_USB_DataRx(USBPortID_comms[FIRST_CDC_RNDIS_HANDLE]);     // handle potential RNDIS data reception
    }
#endif
#if defined USB_DEVICE_SUPPORT && defined FREEMASTER_CDC && (USB_CDC_VCOM_COUNT == 1) // {35} FreeMaster run-time debugging on single USB-CDC connection rather than command-line interface
    if ((Length = fnRead(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, MEDIUM_MESSAGE)) != 0) { // read available data
        fnHandleFreeMaster(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, Length); // handle the received data
    }
#elif defined USB_DEVICE_SUPPORT && ((defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0)) && defined USE_MAINTENANCE) || (defined USB_HOST_SUPPORT && defined USB_CDC_HOST && (!defined USB_RNDIS_HOST || defined TELIT_LE910C1)) // USB-CDC with command line interface
    #if !defined FREE_RTOS_USB || (USB_CDC_COUNT > 1)
    if (USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE] != NO_ID_ALLOCATED) {
        fnHandleCDC_reception(USBPortID_comms[VCOM_CDC_INTERFACE_HANDLE], ucInputMessage); // handle potential CDC data on the VCOM interface
    }
    #endif
    #if defined USB_CDC_VCOM_COUNT && (USB_CDC_VCOM_COUNT > (MODBUS_USB_CDC_COUNT + 1))
    for (iCDC_input = (1 + MODBUS_USB_CDC_COUNT); iCDC_input < USB_CDC_VCOM_COUNT; iCDC_input++) { // for each additional CDC interface
        #if defined FREEMASTER_CDC && (USB_CDC_VCOM_COUNT > 1)           // {35} FreeMaster run-time debugging on final USB-CDC connection
        if (iCDC_input == (USB_CDC_VCOM_COUNT - 1)) {                    // final CDC instance if used for FreeMaster run-time debugging
            if ((Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, MEDIUM_MESSAGE)) != 0) { // read available data
                fnHandleFreeMaster(USBPortID_comms[iCDC_input], ucInputMessage, Length); // handle the received data
            }
            break;
        }
        #endif
        #if !defined FREEMASTER_CDC || (USB_CDC_VCOM_COUNT > 2)
            #if defined USB_CDC_INTERFACE_PASSTHROUGH
        while (fnMsgs(USBPortID_comms[iCDC_input]) != 0) {               // if there is data available in the USB input
            Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, LARGE_MESSAGE);
            fnWrite(USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage, Length); // send input to first USB-CDC interface
        }
            #elif defined SERIAL_INTERFACE && defined USB_SERIAL_CONNECTIONS
                #if defined  TEST_PIT_SINE_TO_USB
        Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, MEDIUM_MESSAGE); // read the content from the USB input
        if (Length != 0) {
            if (ucInputMessage[0] == 's') {                              // if 's' is received on the streaming OUT endpoint
                fnUSB_stream_start();                                    // start the streaming
            }
            else if (ucInputMessage[0] == 't') {                         // if 't' is received on the streaming OUT endpoint
                fnUSB_stream_stop(0);                                    // stop the streaming
            }
        }
                #else
        if (fnMsgs(USBPortID_comms[iCDC_input]) != 0) {                  // if there is data available in the USB input
            if (CDCSerialPortID[iCDC_input] == NO_ID_ALLOCATED) {        // no attached UART
                fnRead(USBPortID_comms[iCDC_input], ucInputMessage, LARGE_MESSAGE); // clear input
            }
            else {
                if (fnWrite(CDCSerialPortID[iCDC_input], 0, MEDIUM_MESSAGE) != 0) { // check that there is space for a block of data to be copied to the corresponding UART outout
                    Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, MEDIUM_MESSAGE); // read the content from the USB input
                    fnWrite(CDCSerialPortID[iCDC_input], ucInputMessage, Length); // send read content to serial port
                }
                else {
                    fnDriver(CDCSerialPortID[iCDC_input], MODIFY_WAKEUP, (MODIFY_TX | OWN_TASK)); // we want to be woken when the UART's queue is free again
                                                                         // leave present USB data in the input buffer until we have enough serial output buffer space
                                                                         // the TX_FREE event is not explicitly handled since it is used to wake a next check of the buffer progress
                }
            }
        }
        if (CDCSerialPortID[iCDC_input] != NO_ID_ALLOCATED) {            // if there is a UART ready
            if ((Length = fnRead(CDCSerialPortID[iCDC_input], ucInputMessage, LARGE_MESSAGE)) != 0) { // if there is serial reception
                fnWrite(USBPortID_comms[iCDC_input], ucInputMessage, Length); // send to USB-CDC interface
            }
        }
                #endif
            #elif !defined FREE_RTOS_USB
        Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, LARGE_MESSAGE); // read any content
        if (Length != 0) {
            fnWrite(USBPortID_comms[iCDC_input], ucInputMessage, Length); // echo the reception back
        }
            #endif
        #endif
    }
    #endif
#elif defined USB_DEVICE_SUPPORT && defined USE_USB_CDC && (USB_CDC_VCOM_COUNT > 0) // USB-CDC without command line menu (quick test case without full flow control handling)
    #if USB_CDC_VCOM_COUNT > 1                                           // multiple CDC interfacs
        #if defined SERIAL_INTERFACE && defined DEMO_UART
    CDCSerialPortID[0] = SerialPortID;                                   // automatically enter the debug serial port in the CDC UART list
        #endif
        #if defined USE_MAINTENANCE
    iCDC_input = 1;                                                      // first interface is handled in application.c
        #else
    iCDC_input = MODBUS_USB_CDC_COUNT;                                   // CDC-UART bridge on all subsequent interfaces
        #endif
    for ( ; iCDC_input < USB_CDC_VCOM_COUNT; iCDC_input++) {             // for each CDC interface
    #elif !defined USE_MAINTENANCE                                       // single CDC interface without command-line shell
        #define iCDC_input 0
    #endif
        Length = fnRead(USBPortID_comms[iCDC_input], ucInputMessage, LARGE_MESSAGE); // read any CDC input content
    #if defined SERIAL_INTERFACE && defined USB_SERIAL_CONNECTIONS       // {8} CDC-UART bridge
        {
        #if USB_CDC_VCOM_COUNT > 1
            QUEUE_HANDLE uartID = CDCSerialPortID[iCDC_input];           // the associated UART to bridge to/from
        #else
            QUEUE_HANDLE uartID = SerialPortID;                          // the associated UART to bridge to/from
        #endif
            if (uartID != NO_ID_ALLOCATED) {                             // if the UART interface has been opened
                if (Length != 0) {                                       // CDC reception available
        #if defined USB_CDC_ECHO
                    fnWrite(USBPortID_comms[iCDC_input], ucInputMessage, Length); // echo back to the USB-CDC connection
        #else
                    fnWrite(uartID, ucInputMessage, Length);             // send input to serial port (data loss not respected if the serial port has no more output buffer space)
        #endif
                }
                if ((Length = fnRead(uartID, ucInputMessage, LARGE_MESSAGE)) != 0) { // read any UART input data
                    fnWrite(USBPortID_comms[iCDC_input], ucInputMessage, Length); // pass it on to the USB-CDC connection
                }
            }
        }
    #else
        if (Length != 0) {
            fnWrite(USBPortID_comms[iCDC_input], ucInputMessage, Length);    // echo the reception back
        }
    #endif
    #if USB_CDC_VCOM_COUNT > 1
    }
    #endif
#endif
#if defined USB_DEVICE_SUPPORT && defined USE_USB_HID_RAW
    if (USBPortID_HID_raw != NO_ID_ALLOCATED) {
        fnHandleHID_RAW_reception(USBPortID_HID_raw);                    // handle potential raw HID data
    }
#endif
#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    if (USBPortID_interrupt[FIRST_CDC_INTERFACE_HANDLE] != NO_ID_ALLOCATED) {
        fnRNDIS_USB_InterruptRx(USBPortID_interrupt[FIRST_CDC_INTERFACE_HANDLE], ucInputMessage); // handle RNDIS interrupt endpoint reception
    }

    #if (USB_CDC_COUNT > 1) && defined TELIT_LE910C1
    if (USBPortID_interrupt[VCOM_CDC_INTERFACE_HANDLE] != NO_ID_ALLOCATED) {
        fnRNDIS_USB_InterruptModem(USBPortID_interrupt[VCOM_CDC_INTERFACE_HANDLE], ucInputMessage); // handle modem interrupt endpoint reception
    }
    #endif
#endif
}


#if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
// Display the USB interface that the device has been detected on
//
static void fnDisplayUSB_Interface(unsigned char ucEvent)
{
    if ((ucEvent & 1) != 0) {                                            // odd events are on USB interface 1
        fnDebugMsg("(1) ");
    }
    else {                                                               // USB interface 0
        fnDebugMsg("(0) ");
    }
}
#endif

#if defined USE_USB_HID_RAW && defined IN_COMPLETE_CALLBACK
static void fnRawHIDPolled(unsigned char ucEndpoint)
{
}
#endif


// The application must always supply this routine and return its device descriptor when requested
// This example shows a single fixed device configuration but multiple configurations could be selected (eg. for experimental use)
//
extern void *fnGetUSB_device_descriptor(unsigned short *usLength, int iChannel)
{
#if defined USB_HOST_SUPPORT && !defined USB_DEVICE_SUPPORT              // {47}
    return 0;                                                            // dummy for host
#else
    #if defined PARALLEL_USD_DEVICE_INTERFACE
    if (iChannel == USB_HS_DEVICE_REF) {                                 // if on the HS USB device interface
        *usLength = sizeof(HS_device_descriptor);                        // return this device's device descriptor
        return (void *)&HS_device_descriptor;
    }
    #endif
    *usLength = sizeof(device_descriptor);
    return (void *)&device_descriptor;
#endif
}

// The application must always supply this routine and return its configuration descriptor when requested
// This example shows a single fixed configuration but multiple configurations could be selected (eg. for programmable device types)
//
extern void *fnGetUSB_config_descriptor(unsigned short *usLength, int iChannel)
{
#if defined USB_HOST_SUPPORT && !defined USB_DEVICE_SUPPORT              // {47}
    return 0;                                                            // in host mode we return a zero so that the generic driver uses its own information
#else
    #if defined PARALLEL_USD_DEVICE_INTERFACE
    if (iChannel == USB_HS_DEVICE_REF) {                                 // if on the HS USB device interface
        *usLength = sizeof(HS_config_descriptor);                        // return this device's configuration descriptor
        return (void *)&HS_config_descriptor;
    }
    #elif defined _iMX && (USBS_AVAILABLE > 1)
    if (iChannel == USB_HOST_REF) {
        return 0;
    }
    #endif
    *usLength = sizeof(config_descriptor);
    return (void *)&config_descriptor;
#endif
}

#if defined USB_HOST_INTERFACE_FILTER
extern unsigned long fnGetUSB_InterfaceFilter(int iChannel)
{
    return ulInterfaceFilter[iChannel];
}
#endif

#if defined USB_STRING_OPTION && !defined _DEV2
    #if defined USB_RUN_TIME_DEFINABLE_STRINGS
// This routine constructs a USB string descriptor for use by the USB interface during emumeration
// - the new string has to respect the descriptor format (using UNICODE) and is built in preparation so that it can be passed in an interrupt
//
static void fnSetSerialNumberString(CHAR *ptrSerialNumber) {             // {12}
    unsigned char ucDescriptorLength = (sizeof(USB_STRING_DESCRIPTOR) - 2);
    unsigned char *ptrString;
    size_t StringLength = (uStrlen(ptrSerialNumber) * 2);
    if (StringLength == 0) {
        ucDescriptorLength += 2;                                         // space for a null-terminator
    }
    else {
        ucDescriptorLength += (unsigned char)StringLength;
    }
    if (SerialNumberDescriptor == 0) {
        SerialNumberDescriptor = (USB_STRING_DESCRIPTOR *)uMalloc(ucDescriptorLength); // get memory to store the string descriptor
        SerialNumberDescriptor->bLength = ucDescriptorLength;
        SerialNumberDescriptor->bDescriptorType = DESCRIPTOR_TYPE_STRING;
    }
    ptrString = &SerialNumberDescriptor->unicode_string_space[0];
    if (StringLength == 0) {
        *ptrString++ = 0;                                                // when no string add a null-terminator
        *ptrString++ = 0;
    }
    else {
        while (*ptrSerialNumber != 0) {
            *ptrString++ = *ptrSerialNumber++;                           // unicode - english string (requiring just zeros to be added)
            *ptrString++ = 0;
        }
    }
}
    #endif


// This routine must always be supplied by the user if usb strings are supported
//
extern unsigned char *fnGetUSB_string_entry(unsigned char ucStringRef, unsigned short *usLength, int iChannel) // {40}
{
    #if defined USB_HOST_SUPPORT && !defined USB_DEVICE_SUPPORT          // {47}
    return 0;                                                            // dummy for host
    #else
    const unsigned char *ptrString;
    if (ucStringRef > (unsigned char)LAST_STRING_INDEX) {
        return 0;                                                        // invalid string index
    }
    ptrString = ucStringTable[ucStringRef];
        #if defined PARALLEL_USD_DEVICE_INTERFACE
    if (iChannel == USB_HS_DEVICE_REF) {                                 // if on the HS USB device interface
        ptrString = HS_ucStringTable[ucStringRef];
    }
        #endif
        #if defined USB_RUN_TIME_DEFINABLE_STRINGS                       // if variable strings are supported
    if (*ptrString == 0) {                                               // no length defined, meaning it is a run-time definabled string
        switch (ucStringRef) {
        case SERIAL_NUMBER_STRING_INDEX:                                 // USB driver needs to know what string is used as serial number
            *usLength = (unsigned short)SerialNumberDescriptor->bLength; // return length and location of the user defined serial number
            return (unsigned char *)SerialNumberDescriptor;
        default:
            return 0;                                                    // invalid string index
        }
    }
    else {
        *usLength = *ptrString;                                          // the length of the string
    }
        #else
    *usLength = *ptrString;                                              // the length of the string
        #endif
    return ((unsigned char *)ptrString);                                 // return a pointer to the string
    #endif
}
#endif


// Endpoint 0 call-back for any non-supported control transfers.
// This can be called with either setup frame content (iType != 0) or with data belonging to following OUT frames.
// TERMINATE_ZERO_DATA must be returned to setup tokens with NO further data, when there is no response sent.
// BUFFER_CONSUMED_EXPECT_MORE is returned when extra data is to be received.
// STALL_ENDPOINT should be returned if the request in the setup frame is not expected.
// Return BUFFER_CONSUMED in all other cases.
//
// If further data is to be received, this may arrive in multiple frames and the call-back needs to manage this to be able to know when the data is complete
//
static int control_callback(unsigned char *ptrData, unsigned short usLength, int iType, int iChannel)
{
#if defined USE_USB_CDC
    static int iInterface = 0;
#endif
    int iRtn = BUFFER_CONSUMED;                                          // default return value if not overwritten
    switch (iType) {
#if defined USB_DEVICE_SUPPORT && defined USE_USB_MSD
    case ENDPOINT_CLEARED:                                               // halted endpoint has been freed
    #if defined _LPC23XX || defined _LPC17XX                             // {20}
        if (*ptrData == (IN_ENDPOINT | 0x05))                            // BULK IN 5 is the only endpoint that is expect to be halted
    #else
        if (*ptrData == (IN_ENDPOINT | USB_MSD_IN_ENDPOINT_NUMBER))      // this BULK IN is the only endpoint that is expect to be halted
    #endif
        {
            fnWrite(USBPortID_MSD, (unsigned char *)&csw, sizeof(csw));  // answer with the failed CSW on next endpoint 1 OUT
        }
        return TERMINATE_ZERO_DATA;                                      // send zero data to terminate the halt phase
#endif
#if defined USB_HOST_SUPPORT && defined USB_HOST_CALLBACK_SETUP_ACK
    case HOST_SETUP_ACKED:                                               // host has just successfully sent a SETUP (or following data)
    #if defined USB_HOST_HUB
        if (fnHubStateMachine(HUB_DEVICE_SETUP_ACKED, 0, 0) != 0) {
            return INITIATE_IN_TOKEN;                                    // if hub is connected but not yet enumerated don't allow any other class to handle thsi event
        }
    #endif
    #if defined USB_RNDIS_HOST
        #if defined TELIT_LE910C1
        if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_TELIT_LE910C1) != 0) {
            if ((iRtn = fnTelitModemStateEvent(TELIT_SETUP_ACKED, 0)) != USB_HOST_ERROR) {
                return iRtn;                                             // return according to the modem channel handling and don't handle RNDIS
            }                                                            // event not handle by the modem so allow it to be treated by RNDIS
        }
        #endif
        if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_ACTIVE) != 0) {
            if (fnRNDIS_ControlData() == 0) {                            // send control transaction data
                return BUFFER_CONSUMED;
            }
            else {                                                       // transaction data has been acknowledged
                ucLocalRNDISEvent[iChannel] = EVENT_RNDIS_DEVICE_CONTROL_OK; // event to be generated when the device returns a zero data frame 
            }
        }
    #endif
        return INITIATE_IN_TOKEN;                                        // default is to allow the control endpoint to send IN tokens so that the device can respond
#endif
    case STATUS_STAGE_RECEPTION:                                         // this is the status stage of a control transfer - it confirms that the exchange has completed and can be ignored if not of interest to us
#if defined USB_HOST_HUB
    case STATUS_STAGE_RECEPTION_ENUM:
        if (fnHubStateMachine(HUB_DEVICE_STATUS_STAGE_RECEPTION, 0, 0) != 0) {
            return USB_HOST_ERROR;                                       // used to inform that the event should be passed to the enumeration state event machine
        }
#endif
#if defined USB_HOST_SUPPORT && defined USB_CDC_HOST && defined USB_RNDIS_HOST
    #if defined TELIT_LE910C1
        if ((ulHostClassTypes & USB_HOST_CDC_RNDIS_TELIT_LE910C1) != 0) {
            if (fnTelitModemStateEvent(TELIT_STATUS_STAGE_OK, 0) != USB_HOST_ERROR) {
                return BUFFER_CONSUMED;                                  // handled by the modem CDC interface and so no RNDIS handling
            }                                                            // event not handle by the modem so allow it to be treated by RNDIS
        }
    #endif
        if (ucLocalRNDISEvent[iChannel] != 0) {                          // if an event is queued
            if (ucLocalRNDISEvent[iChannel] == EVENT_RNDIS_DEVICE_CONTROL_OK) {
                fnRNDIS_HostManagement(RNDIS_HOST_CTRL_TERMINATING);     // mark that the control transaction has completed
            }
            fnInterruptMessage(OWN_TASK, ucLocalRNDISEvent[iChannel]);   // send the defined event to the USB task
            ucLocalRNDISEvent[iChannel] = 0;                             // single shot event has been generated so clear
        }
        if (sRNDIS_Host.iControlDataLength != 0) {                       // zero data terminates data length which is a multiple of the endpoint size
            sRNDIS_Host.iWaitingControlDataLength = sRNDIS_Host.iControlDataLength;
            sRNDIS_Host.iControlDataLength = 0;
            fnInterruptMessage(OWN_TASK, EVENT_RNDIS_CONTROL_DATA_READY);// inform the USB task that we have a complete response from the device
        }
#endif
#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
        if (ucLocalMSDEvent[iChannel] != 0) {                            // if an event is queued
            fnInterruptMessage(OWN_TASK, ucLocalMSDEvent[iChannel]);     // send the defined event to the USB task
            ucLocalMSDEvent[iChannel] = 0;                               // single shot event has been generated so clear
        }
#endif
        return BUFFER_CONSUMED;
    case SETUP_DATA_RECEPTION:
        {
            USB_SETUP_HEADER *ptrSetup = (USB_SETUP_HEADER *)ptrData;    // interpret the received data as a setup header
#if !defined USE_USB_HID_MOUSE && !defined USE_USB_HID_KEYBOARD && !defined USE_USB_HID_RAW && (!defined USE_USB_AUDIO && !defined USE_HS_USB_AUDIO) // {23}{34}
            if ((ptrSetup->bmRequestType & ~STANDARD_DEVICE_TO_HOST) != REQUEST_INTERFACE_CLASS) { // 0x21
                return STALL_ENDPOINT;                                   // stall on any unsupported request types
            }
#endif
            usExpectedData[iChannel] = ptrSetup->wLength[0];             // the amount of additional data which is expected to arrive from the host belonging to this request
            usExpectedData[iChannel] |= (ptrSetup->wLength[1] << 8);
            if ((ptrSetup->bmRequestType & STANDARD_DEVICE_TO_HOST) != 0) { // request for information
#if defined USE_USB_CDC && defined USB_CDC_RNDIS
                unsigned short usMaxDataLength = usExpectedData[iChannel];
#endif
                usExpectedData[iChannel] = 0;                            // no data expected to be received by us
                switch (ptrSetup->bRequest) {
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
                case 0x81:
                    fnWrite(USB_control[USB_AUDIO_DEVICE_REF], (unsigned char *)&ucReport1, sizeof(ucReport1)); // return directly (non-buffered)
                    break;
                case 0x82:
                    fnWrite(USB_control[USB_AUDIO_DEVICE_REF], (unsigned char *)&ucReport2, sizeof(ucReport2)); // return directly (non-buffered)
                    break;
                case 0x83:
                    fnWrite(USB_control[USB_AUDIO_DEVICE_REF], (unsigned char *)&ucReport3, sizeof(ucReport3)); // return directly (non-buffered)
                    break;
                case 0x84:
                    fnWrite(USB_control[USB_AUDIO_DEVICE_REF], (unsigned char *)&ucReport4, sizeof(ucReport4)); // return directly (non-buffered)
                    break;
#endif
#if defined USE_USB_HID_MOUSE || defined USE_USB_HID_KEYBOARD || defined USE_USB_HID_RAW // {23}
                case USB_REQUEST_GET_DESCRIPTOR:                         // standard request
                    if (ptrSetup->wValue[1] == DESCRIPTOR_TYPE_REPORT) {
                        int iHID_Interface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // the interface being requested
    #if defined USE_USB_HID_MOUSE
                        if (iHID_Interface == USB_MOUSE_INTERFACE_NUMBER) { // mouse interface number
                            fnWrite(USB_control[USB_DEVICE_REF], (unsigned char *)&ucMouseReport, sizeof(ucMouseReport)); // return directly (non-buffered)
                            break;
                        }
    #endif
    #if defined USE_USB_HID_KEYBOARD                                     // {34}
                        if (iHID_Interface == USB_KEYBOARD_INTERFACE_NUMBER) { // keyboard interface number
                            fnWrite(USB_control[USB_DEVICE_REF], (unsigned char *)&ucKeyboardReport, sizeof(ucKeyboardReport)); // return directly (non-buffered)
                            break;
                        }
    #endif
    #if defined USE_USB_HID_RAW
                        if (iHID_Interface == USB_HID_RAW_INTERFACE_NUMBER) { // raw interface number
                            fnWrite(USB_control[USB_DEVICE_REF], (unsigned char *)&ucRawReport, sizeof(ucRawReport)); // return directly (non-buffered)
                            break;
                        }
    #endif
                    }
                    return STALL_ENDPOINT;                               // not supported
#endif
#if defined USE_USB_MSD
                case BULK_ONLY_MASS_STORAGE_RESET:
                    break;
                case GET_MAX_LUN:
                    {
                        static const unsigned char ucPartitions = (NUMBER_OF_PARTITIONS - 1);
                        if ((ptrSetup->wValue[0] != 0) || (ptrSetup->wValue[1] != 0) || (ptrSetup->wLength[0] != 1) || (ptrSetup->wLength[1] != 0)) {
                            return STALL_ENDPOINT;                       // stall if bad request format
                        }
                        else {
                            fnWrite(USB_control[USB_DEVICE_REF], (unsigned char *)&ucPartitions, sizeof(ucPartitions)); // return directly (non-buffered)
                        }
                    }
                    break;
#endif
#if defined USE_USB_CDC
    #if defined USB_CDC_RNDIS
                case GET_ENCAPSULATED_RESPONSE:                          // 0x01
                    iInterface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // remember the interface that this command belongs to
                    fnSendEncapsulatedResponse(((iInterface - FIRST_CDC_RNDIS_INTERFACE)/2), usMaxDataLength); // return the prepared response
                    break;
    #endif
                case GET_LINE_CODING:                                    // 0x21
                    iInterface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // remember the interface that this command belongs to
                    fnInterruptMessage(OWN_TASK, (unsigned char)(EVENT_RETURN_PRESENT_UART_SETTING + ((iInterface - FIRST_CDC_INTERFACE_HANDLE)/2)));
                    break;
#endif
                default:
                    return STALL_ENDPOINT;                               // stall on any unsupported requests
                }
            }
            else {                                                       // command (direction host to device)
                iRtn = TERMINATE_ZERO_DATA;                              // {5} acknowledge receipt of the request if we have no data to return (default)
                switch (ptrSetup->bRequest) {
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
                case 0x01:
                  //iInterface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // remember the interface that this command belongs to
                    usAudioCommand = (ptrSetup->wValue[0] | (ptrSetup->wValue[1] << 8));
                    ucCollectingMode[iChannel] = ptrSetup->bRequest;     // the next OUT frame will contain the settings
                    iRtn = BUFFER_CONSUMED_EXPECT_MORE;                  // {5} the present buffer has been consumed but extra data is subsequently expected
                    break;
#endif
#if defined USE_USB_CDC                                                  // {23}
    #if defined USB_CDC_RNDIS
                case SEND_ENCAPSULATED_COMMAND:                          // 0x00
    #endif
    #if USB_CDC_VCOM_COUNT > 0
                case SET_LINE_CODING:                                    // 0x20 - the host is informing us of parameters to use
    #endif
                    iInterface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // remember the interface that this command belongs to
                    ucCollectingMode[iChannel] = ptrSetup->bRequest;     // the next OUT frame will contain the settings
                    iRtn = BUFFER_CONSUMED_EXPECT_MORE;                  // {5} the present buffer has been consumed but extra data is subsequently expected
                    break;
    #if USB_CDC_VCOM_COUNT > 0
                case SET_CONTROL_LINE_STATE:                             // OUT - 0x22 (controls RTS and DTR)
                    iInterface = (ptrSetup->wIndex[0] | (ptrSetup->wIndex[1] << 8)); // remember the interface that this command belongs to
        #if defined SERIAL_INTERFACE && defined SUPPORT_HW_FLOW && defined DEMO_UART // {8}
                    {
                        QUEUE_HANDLE serialHandle = SerialPortID;
            #if USB_CDC_VCOM_COUNT > 1
                        iInterface -= FIRST_CDC_INTERFACE_HANDLE;
                        iInterface /= 2;
                        if (iInterface > 0) {
                            serialHandle = CDCSerialPortID[iInterface];
                        }
            #endif
                        if (serialHandle != NO_ID_ALLOCATED) {           // only set the control lines when the UART is available
                            if (ptrSetup->wValue[0] & CDC_RTS) {
                                fnDriver(serialHandle, (MODIFY_CONTROL | SET_RTS), 0);
                            }
                            else {
                                fnDriver(serialHandle, (MODIFY_CONTROL | CLEAR_RTS), 0);
                            }
                            if (ptrSetup->wValue[0] & CDC_DTR) {
                                fnDriver(serialHandle, (MODIFY_CONTROL | SET_DTR), 0);
                            }
                            else {
                                fnDriver(serialHandle, (MODIFY_CONTROL | CLEAR_DTR), 0);
                            }
                        }
                    }
        #endif
                    break;
    #endif
#endif
#if defined USE_USB_HID_MOUSE || defined USE_USB_HID_KEYBOARD || defined USE_USB_HID_RAW // {23}{34}
                case HID_SET_IDLE:                                       // 0x0a - this can silence a report
                    break;                                               // answer with zero data
#endif
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                case HID_SET_REPORT:                                     // 0x09 - set report
                    return BUFFER_CONSUMED_EXPECT_MORE;                  // the present buffer has been consumed but extra data is subsequently expected
#endif
                default:
                    return STALL_ENDPOINT;                               // stall on any unsupported requests
                }
            }
            if (usLength <= sizeof(USB_SETUP_HEADER)) {
                return iRtn;                                             // no extra data in this frame
            }
            usLength -= sizeof(USB_SETUP_HEADER);                        // header handled
            ptrData += sizeof(USB_SETUP_HEADER);
        }
        // Fall through intentionally
        //
    default:                                                             // OUT_DATA_RECEPTION
        if (usExpectedData[iChannel] != 0) {                             // complete data has been received
            // Handle and control commands here
            //
            switch (ucCollectingMode[iChannel]) {
#if defined USE_USB_AUDIO || defined USE_HS_USB_AUDIO
            case 0x01:
                if (usAudioCommand == 0x0100) {                          // mute
                    iMuteOff = (*ptrData == 0);
                    fnInterruptMessage(OWN_TASK, EVENT_MUTE_CHANGE);
                }
                else if (usAudioCommand == 0x0200) {                     // volume
                    usSpeakerVolume = *ptrData++;
                    usSpeakerVolume |= (*ptrData << 8);
                    fnInterruptMessage(OWN_TASK, EVENT_VOLUME_CHANGE);
                }
                break;
#endif
#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
            case GET_MAX_LUN:
                ucDeviceLUN = (*ptrData + 1);                            // increment the value by 1 to get the number of partitions
                ucLocalMSDEvent[iChannel] = EVENT_LUN_READY;             // when the returned zero data has been acknowledged we continue with this event
                break;
#endif
#if defined USE_USB_CDC
    #if defined USB_CDC_RNDIS
            case SEND_ENCAPSULATED_COMMAND:                              // 0x00
                fnRNDIS_control(ptrData, usLength, usExpectedData[iChannel], ((iInterface - FIRST_CDC_RNDIS_INTERFACE)/2));
                break;
    #endif
    #if USB_CDC_VCOM_COUNT > 0
            case SET_LINE_CODING:
                fnNewUART_settings(ptrData, usLength, usExpectedData[iChannel], ((iInterface - FIRST_CDC_INTERFACE_HANDLE)/2)); // set the new UART mode (the complete data will always be received here so we can always terminate now, otherwise BUFFER_CONSUMED_EXPECT_MORE would be returned until complete)
              //iRtn = TERMINATE_ZERO_DATA;                              // {5}
                break;
    #endif
#endif
            default:
#if defined USE_USB_HID_KEYBOARD                                         // {34}
                fnSetKeyboardOutput(*ptrData);                           // assumed to be set report with a single byte content
#endif
                break;
            }
            if (usLength >= usExpectedData[iChannel]) {
                ucCollectingMode[iChannel] = 0xff;                       // {9}{38} reset to avoid repeat of command when subsequent, invalid commands are received
                usExpectedData[iChannel] = 0;                            // all of the expected data belonging to this transfer has been received
                return TERMINATE_ZERO_DATA;
            }
            else {
                usExpectedData[iChannel] -= usLength;                    // remaining length to be received before transaction has completed
            }
            return BUFFER_CONSUMED_EXPECT_MORE;
        }
#if defined USB_RNDIS_HOST
        else {                                                           // general data reception on control endpoint
    #if defined USB_HOST_HUB
            int iHubState = fnHubStateMachine(HUB_DEVICE_DATA_RECEPTION, ptrData, usLength);
            if (iHubState == 0) {
    #endif
                uMemcpy(&sRNDIS_Host.ucControlDataReception[sRNDIS_Host.iControlDataLength], ptrData, usLength); // copy the data into the control reception buffer
                sRNDIS_Host.iControlDataLength += usLength;              // the available data length
    #if defined USB_HOST_HUB
            }
    #endif
            if (usLength != usControlEndpointSize) {                     // if the data is complete
    #if defined USB_HOST_HUB
                if (iHubState == -1) {
                    return TERMINATE_ZERO_DATA_NON_E;                    // terminate with a zero data packet
                }
    #endif
                return TERMINATE_ZERO_DATA;                              // terminate with a zero data packet
            }
            else {
                return INITIATE_IN_TOKEN;                                // continue collecting data with IN token
            }
        }
#endif
        break;
    }
    return iRtn;
}

#if defined USB_HOST_SUPPORT
// Display a device string - assuming english character set
//
static void fnDisplayUSB_string(unsigned char *ptr_string)
{
    unsigned char ucLength = *ptr_string;                                // reported length of unicode string
    CHAR cSingleCharacter[2];
    if (ucLength > (USB_MAX_STRING_LENGTH * 2)) {                        // in case the unicode string is reported longer than the part that we requested we cut the end
        ucLength = (USB_MAX_STRING_LENGTH * 2);
    }
    cSingleCharacter[1] = 0;                                             // terminator
    while (ucLength >= 4) {                                              // for each unicode character (ignore final 0x0000)
        ptr_string += 2;                                                 // set to second byte of unicode string
        cSingleCharacter[0] = *ptr_string;
        fnDebugMsg(cSingleCharacter);                                    // display
        ucLength -= 2;
    }
}

static unsigned char fnDisplayDeviceInfo(unsigned long *ptrDeviceType, int iChannel)
{
    #if defined USB_CDC_HOST
    int iCDC_reference = 0;
    #endif
    USB_DEVICE_DESCRIPTOR *ptr_device_descriptor;
    USB_CONFIGURATION_DESCRIPTOR *ptr_config_desc;
    USB_INTERFACE_DESCRIPTOR *ptr_interface_desc;
    unsigned char *ptr_string;
    unsigned short usVendor;
    unsigned short usProduct;
    int iTotalInterfaces;
    *ptrDeviceType = 0;                                                  // reset before determining the new device type
    ptr_device_descriptor = (USB_DEVICE_DESCRIPTOR *)fnGetDeviceInfo(REQUEST_USB_DEVICE_DESCRIPTOR, USB_HOST_REF);
    ptr_config_desc = (USB_CONFIGURATION_DESCRIPTOR *)fnGetDeviceInfo(REQUEST_USB_CONFIG_DESCRIPTOR, USB_HOST_REF);
    fnDebugMsg("USB");
    if (ptr_device_descriptor->bcdUSB[1] == 2) {
        fnDebugMsg("2.0");
    }
    else {
        fnDebugMsg("1.1");
    }
    fnDebugMsg(" device with ");
#if defined USB_RNDIS_HOST
    usControlEndpointSize = ptr_device_descriptor->bMaxPacketSize0;
#endif
    fnDebugDec(ptr_device_descriptor->bMaxPacketSize0, 0);
    fnDebugMsg(" byte pipe\r\nVendor/Product = ");
    usVendor = ((ptr_device_descriptor->idVendor[1] << 8) | ptr_device_descriptor->idVendor[0]);
    fnDebugHex(usVendor, (WITH_LEADIN | sizeof(usVendor)));
    fnDebugMsg("/");
    usProduct = ((ptr_device_descriptor->idProduct[1] << 8) | ptr_device_descriptor->idProduct[0]);
    fnDebugHex(usProduct, (WITH_LEADIN | WITH_CR_LF | sizeof(usProduct)));
    #if defined USB_STRING_OPTION
    ptr_string = (unsigned char *)fnGetDeviceInfo((REQUEST_USB_STRING_DESCRIPTOR + ptr_device_descriptor->iManufacturer), USB_HOST_REF);
    if (ptr_string != 0) {
        fnDebugMsg("Manufacturer = \x22");
        fnDisplayUSB_string(ptr_string);
        fnDebugMsg("\x22\r\n");
    }
    ptr_string = (unsigned char *)fnGetDeviceInfo((REQUEST_USB_STRING_DESCRIPTOR + ptr_device_descriptor->iProduct), USB_HOST_REF);
    if (ptr_string != 0) {
        fnDebugMsg("Product = \x22");
        fnDisplayUSB_string(ptr_string);
        fnDebugMsg("\x22\r\n");
    }
    ptr_string = (unsigned char *)fnGetDeviceInfo((REQUEST_USB_STRING_DESCRIPTOR + ptr_device_descriptor->iSerialNumber), USB_HOST_REF);
    if (ptr_string != 0) {
        fnDebugMsg("Serial Number = \x22");
        fnDisplayUSB_string(ptr_string);
        fnDebugMsg("\x22\r\n");
    }
    #endif

    if ((ptr_config_desc->bmAttributes & SELF_POWERED) != 0) {
        fnDebugMsg("\r\nSelf-powered device");
    }
    else {
        fnDebugMsg("\r\nBus-powered device (max. ");
        fnDebugDec((ptr_config_desc->bMaxPower * 2), 0);
        fnDebugMsg("mA)");
    }
    fnDebugMsg(" with ");
    iTotalInterfaces = ptr_config_desc->bNumInterface;
    fnDebugDec(iTotalInterfaces, 1);
    fnDebugMsg(" interface(s)\r\n");
    #if defined USB_CDC_HOST || defined USB_MSD_HOST                     // {37}
    ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_config_desc + 1);
    FOREVER_LOOP() {                                                     // scan through the reported interfaces
        if (ptr_interface_desc->bDescriptorType == DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION) { // handle an optional interface association descriptor
            USB_INTERFACE_ASSOCIATION_DESCRIPTOR *ptr_interface_association_desc = (USB_INTERFACE_ASSOCIATION_DESCRIPTOR *)ptr_interface_desc;
            fnDebugMsg("Associated Interfaces = ");
            fnDebugDec(ptr_interface_association_desc->bInterfaceCount, WITH_CR_LF);
            ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_interface_association_desc + 1);
        }
        if (ptr_interface_desc->bDescriptorType == DESCRIPTOR_TYPE_INTERFACE) { // interface descriptor
            unsigned long ulInterfaceType = 0;
            #define INTERFACE_TYPE_USB_MSD          0x00000001
            #define INTERFACE_TYPE_RNDIS            0x00000002
            #define INTERFACE_TYPE_CDC              0x00000004
            #define INTERFACE_TYPE_TELIT_MODEM      0x00000008
            #define INTERFACE_TYPE_MOTOROLA_MODEM   0x00000010
            #define INTERFACE_TYPE_HUB              0x00000020
            fnDebugMsg("Interface");
            switch (ptr_interface_desc->bInterfaceClass) {
        #if defined USB_MSD_HOST
            case INTERFACE_CLASS_MASS_STORAGE:
                ulInterfaceType = INTERFACE_TYPE_USB_MSD;
                break;
        #endif
        #if defined USB_RNDIS_HOST
            case INTERFACE_CLASS_WIRELESS_CONTROLLER:                    // Android RNDIS uses wireless controller class
                ulInterfaceType = INTERFACE_TYPE_RNDIS;
                break;
        #endif
        #if defined USB_CDC_HOST
            case USB_CLASS_COMMUNICATION_CONTROL:
            case INTERFACE_CLASS_COMMUNICATION_DATA:
                ulInterfaceType = INTERFACE_TYPE_CDC;
                break;
        #endif
        #if defined USB_HOST_HUB
            case INTERFACE_CLASS_HUB:
                ulInterfaceType = INTERFACE_TYPE_HUB;
                break;
        #endif
            default:
        #if defined TELIT_LE910C1
                if ((usVendor == 0x1bc7) && (/*(usProduct == 0x1201) || */(usProduct == 0x1203))) { // modem
                    ulInterfaceFilter[iChannel] = (ACCEPT_USB_INTERFACES(USB_INTERFACE_ACCEPTED(0) | USB_INTERFACE_ACCEPTED(1) | USB_INTERFACE_ACCEPTED(6))); // accept only interface 0, 1 and 6 when working with this device
                    ulInterfaceType = INTERFACE_TYPE_TELIT_MODEM;
                }
        #endif
        #if defined USB_RNDIS_HOST
                if ((usVendor == 0x0cad) && (usProduct == 0x1053)) {     // motorola modem
                    ulInterfaceType = INTERFACE_TYPE_MOTOROLA_MODEM;
                }
        #endif
                break;
            }
            fnDebugDec(ptr_interface_desc->bInterfaceNumber, (WITH_SPACE | WITH_CR_LF));
            iTotalInterfaces--;                                          // interfaces still remaining
            if ((ulInterfaceType & (INTERFACE_TYPE_RNDIS | INTERFACE_TYPE_TELIT_MODEM | INTERFACE_TYPE_MOTOROLA_MODEM | INTERFACE_TYPE_CDC)) != 0) {
                unsigned char ucInterfaceEndpoints = ptr_interface_desc->bNumEndpoints;
                ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_interface_desc + 1);
                while (ucInterfaceEndpoints-- != 0) {
                    while (ptr_interface_desc->bDescriptorType == CS_INTERFACE) { // move over class interfaces
                        USB_CDC_FUNCTIONAL_DESCRIPTOR_UNION *ptr_functional_des_union = (USB_CDC_FUNCTIONAL_DESCRIPTOR_UNION *)ptr_interface_desc;
                        ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(((unsigned char *)ptr_functional_des_union) + ptr_functional_des_union->bLength);
                    }
                    if (DESCRIPTOR_TYPE_ENDPOINT == ptr_interface_desc->bDescriptorType) {
                        USB_ENDPOINT_DESCRIPTOR *ptr_endpoint_desc = (USB_ENDPOINT_DESCRIPTOR *)ptr_interface_desc;
                        USB_ENDPOINT_INFO EndpointInfo;
                        fnDisplayEndpoint(ptr_endpoint_desc, &EndpointInfo, ucHostSpeed, 1);
                        switch (EndpointInfo.ucEndpointType) {
        #if defined USB_RNDIS_HOST
                        case ENDPOINT_INTERRUPT:
                            ucCDC_InterruptEndpoint[iCDC_reference] = EndpointInfo.ucEndpointNumber;
                            usCDC_InterruptEndpointSize[iCDC_reference] = EndpointInfo.usEndpointSize;
                            CDC_InterruptPollingRate[iCDC_reference] = EndpointInfo.pollingRate;
                            break;
        #endif
                        case ENDPOINT_BULK_IN:                           // it is expected that CDC has only one bulk IN endpoint
        #if defined USB_CDC_HOST
                            ucCDCBulkInEndpoint[iCDC_reference] = EndpointInfo.ucEndpointNumber; // the endpoint that is to be used as IN endpoint
                            usCDCBulkInEndpointSize[iCDC_reference] = EndpointInfo.usEndpointSize; // the length of the IN endpoint
        #endif
                            break;
                        case ENDPOINT_BULK_OUT:                          // it is expected that CDC has only one bulk OUT endpoint
        #if defined USB_CDC_HOST
                            ucCDCBulkOutEndpoint[iCDC_reference] = EndpointInfo.ucEndpointNumber; // the endpoint that is to be used as OUT endpoint
                            usCDCBulkOutEndpointSize[iCDC_reference] = EndpointInfo.usEndpointSize; // the length of the OUT endpoint
        #endif
                            break;
                        }
                        ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_endpoint_desc + 1); // move to the next descriptor
                    }
                }
                *ptrDeviceType |= USB_HOST_CDC_ACTIVE;                   // CDC device detected (not composite)
        #if defined USB_RNDIS_HOST
                fnDebugMsg("CDC");
                if (((*ptrDeviceType & USB_HOST_CDC_RNDIS_ACTIVE) != 0) || ((ulInterfaceType & (INTERFACE_TYPE_RNDIS | INTERFACE_TYPE_TELIT_MODEM | INTERFACE_TYPE_MOTOROLA_MODEM)) != 0)) {
                    fnDebugMsg(" (RNDIS)");
                    *ptrDeviceType = (USB_HOST_CDC_ACTIVE | USB_HOST_CDC_RNDIS_ACTIVE);
                    if ((ulInterfaceType & INTERFACE_TYPE_TELIT_MODEM) != 0) {
                        *ptrDeviceType |= USB_HOST_CDC_RNDIS_TELIT_LE910C1;
                    }
                }
                fnDebugMsg(" class\r\n");
        #endif
                if (iTotalInterfaces <= 0) {                             // if all interfaces have been accounted for
                    return (ptr_config_desc->bConfigurationValue);       // the valid configuration to be enabled
                }
        #if defined USB_CDC_HOST
                iCDC_reference++;                                        // move to next possible CDC interface
        #endif
            }
        #if defined USB_MSD_HOST
            else if ((ulInterfaceType & INTERFACE_TYPE_USB_MSD) != 0) {
                USB_ENDPOINT_DESCRIPTOR *ptr_endpoint_desc;
                int iEndpoints = ptr_interface_desc->bNumEndpoints;
                fnDebugMsg("Mass Storage Class : Sub-class =");
                fnDebugHex(ptr_interface_desc->bInterfaceSubClass, (WITH_SPACE | WITH_LEADIN | sizeof(ptr_interface_desc->bInterfaceSubClass)));
                fnDebugMsg(" interface protocol =");
                fnDebugHex(ptr_interface_desc->bInterfaceProtocol, (WITH_SPACE | WITH_LEADIN | WITH_CR_LF | sizeof(ptr_interface_desc->bInterfaceProtocol)));
                fnDebugMsg("Endpoints:\r\n");
                ptr_endpoint_desc = (USB_ENDPOINT_DESCRIPTOR *)(ptr_interface_desc + 1);
                while (iEndpoints-- != 0) {
                    USB_ENDPOINT_INFO EndpointInfo;
                    fnDisplayEndpoint(ptr_endpoint_desc, &EndpointInfo, ucHostSpeed, 1);
                    if (EndpointInfo.ucEndpointType == ENDPOINT_BULK_IN) { // it is expected that MSD has only one bulk IN endpoint
                        ucMSDBulkInEndpoint = EndpointInfo.ucEndpointNumber; // the endpoint that is to be used as IN endpoint
                        usMSDBulkInEndpointSize = EndpointInfo.usEndpointSize; // the length of the IN endpoint
                    }
                    else if (EndpointInfo.ucEndpointType == ENDPOINT_BULK_OUT) { // it is expected that MSD has only one bulk OUT endpoint
                        ucMSDBulkOutEndpoint = EndpointInfo.ucEndpointNumber; // the endpoint that is to be used as OUT endpoint
                        usMSDBulkOutEndpointSize = EndpointInfo.usEndpointSize; // the length of the OUT endpoint
                    }
                    ptr_endpoint_desc++;
                }
                *ptrDeviceType = USB_HOST_MSD_ACTIVE;                    // MSD device detected (not composite)
                return (ptr_config_desc->bConfigurationValue);           // the valid configuration to be enabled
            }
        #endif
        #if defined USB_HOST_HUB
            else if ((ulInterfaceType & INTERFACE_TYPE_HUB) != 0) {
                unsigned char ucInterfaceEndpoints = ptr_interface_desc->bNumEndpoints;
                ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_interface_desc + 1);
                fnDebugMsg("Hub class\r\n");
                while (ucInterfaceEndpoints-- != 0) {
                    if (DESCRIPTOR_TYPE_ENDPOINT == ptr_interface_desc->bDescriptorType) {
                        USB_ENDPOINT_DESCRIPTOR *ptr_endpoint_desc = (USB_ENDPOINT_DESCRIPTOR *)ptr_interface_desc;
                        USB_ENDPOINT_INFO EndpointInfo;
                        fnDisplayEndpoint(ptr_endpoint_desc, &EndpointInfo, ucHostSpeed, 1);
                        switch (EndpointInfo.ucEndpointType) {
                        case ENDPOINT_INTERRUPT:
                            ucHub_InterruptEndpoint = EndpointInfo.ucEndpointNumber;
                            usHub_InterruptEndpointSize = EndpointInfo.usEndpointSize;
                            HUB_InterruptPollingRate = EndpointInfo.pollingRate;
                            break;
                        }
                        ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(ptr_endpoint_desc + 1); // move to the next descriptor
                    }
                }
                *ptrDeviceType = USB_HOST_HUB_ACTIVE;                    // hub detected
                return (ptr_config_desc->bConfigurationValue);           // the valid configuration to be enabled

            }
        #endif
            else {
                if (iTotalInterfaces <= 0) {                             // if all interfaces have been accounted for
                    return (ptr_config_desc->bConfigurationValue);       // the valid configuration to be enabled
                }
                fnDebugMsg("Discarded class");
                fnDebugHex(ptr_interface_desc->bInterfaceClass, (sizeof(ptr_interface_desc->bInterfaceClass) | WITH_LEADIN | WITH_SPACE | WITH_CR_LF));
                // Discard unsupported class
                //
                FOREVER_LOOP() {
                    USB_CDC_FUNCTIONAL_DESCRIPTOR_UNION *ptr_functional_des_union = (USB_CDC_FUNCTIONAL_DESCRIPTOR_UNION *)ptr_interface_desc;
                    ptr_interface_desc = (USB_INTERFACE_DESCRIPTOR *)(((unsigned char *)ptr_functional_des_union) + ptr_functional_des_union->bLength);
                    if ((ptr_interface_desc->bDescriptorType == DESCRIPTOR_TYPE_INTERFACE) || (ptr_interface_desc->bDescriptorType == DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION)) {
                        break;
                    }
                }
            }
        }
    }
    #endif
    fnDebugMsg("NON-SUPPORTED CLASS -");
    fnDebugHex(ptr_interface_desc->bInterfaceClass, (WITH_LEADIN | WITH_SPACE | WITH_CR_LF | sizeof(ptr_interface_desc->bInterfaceClass)));
    return 0;                                                            // not supported device
}

// Host endpoint configuration (according to attached device requirements)
//
static void fnConfigureHostApplicationEndpoints(unsigned char ucActiveConfiguration)
{
    USBTABLE tInterfaceParameters;                                       // table for passing information to driver
    #if defined SUPPORT_QUEUE_DEALLOCATION
    tInterfaceParameters.queue_sizes.ucHeapType = 1;                     // the driver queues created here use de-allocatable heap memory
    #endif
    #if defined USB_MSD_HOST
    if ((ulHostClassTypes & USB_HOST_MSD_ACTIVE) != 0) {                 // {44}
        if (NO_ID_ALLOCATED == USBPortID_msd_host) {
        #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
            tInterfaceParameters.ucUSB_controller = USB_HOST_REF;        // the HS USB controller used
        #endif
            tInterfaceParameters.owner_task = OWN_TASK;                  // wake usb task on receptions
            tInterfaceParameters.Endpoint = ucMSDBulkOutEndpoint;        // set USB endpoints to act as an input/output pair - transmitter (OUT)
            tInterfaceParameters.Paired_RxEndpoint = ucMSDBulkInEndpoint;// receiver (IN)
            tInterfaceParameters.usEndpointSize = usMSDBulkOutEndpointSize; // endpoint queue size (2 buffers of this size will be created for reception)
            tInterfaceParameters.usb_callback = 0;                       // no call-back since we use rx buffer - the same task is owner
            tInterfaceParameters.usConfig = 0;
            tInterfaceParameters.queue_sizes.RxQueueSize = 1024;         // optional input queue (used only when no call-back defined)
            tInterfaceParameters.queue_sizes.TxQueueSize = (512 + 32);   // additional tx buffer - allow queueing a sector plus a command
            #if defined WAKE_BLOCKED_USB_TX
            tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
            #endif
            USBPortID_msd_host = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoints with defined configurations (initially inactive)
        }
    }
    #endif
    #if defined USB_CDC_HOST                                             // {37}
    if ((ulHostClassTypes & USB_HOST_CDC_ACTIVE) != 0) {                 // {44}
        #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
            tInterfaceParameters.ucUSB_controller = USB_HOST_REF;        // the HS USB controller used
        #endif
        if ((NO_ID_ALLOCATED == USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE]) && (ucCDCBulkOutEndpoint[FIRST_CDC_HOST_INTERFACE] != 0)) { // if the host endpoint is known but hasn't been opened
            tInterfaceParameters.owner_task = OWN_TASK;                  // wake usb task on receptions on this endpoint
            tInterfaceParameters.Endpoint = ucCDCBulkOutEndpoint[FIRST_CDC_HOST_INTERFACE]; // set USB endpoints to act as an input/output pair - transmitter (OUT)
            tInterfaceParameters.Paired_RxEndpoint = ucCDCBulkInEndpoint[FIRST_CDC_HOST_INTERFACE]; // receiver (IN)
            tInterfaceParameters.usEndpointSize = usCDCBulkOutEndpointSize[FIRST_CDC_HOST_INTERFACE]; // endpoint queue size (2 buffers of this size will be created for reception)
            tInterfaceParameters.usb_callback = 0;                       // no call-back since we use rx buffer - the same task is owner
            tInterfaceParameters.usConfig = USB_IN_AUTO_POLL;            // automatically restart IN polling after data reception (possible on HS USB and ignored on FS USB)
        #if defined USB_RNDIS_HOST
            tInterfaceParameters.queue_sizes.RxQueueSize = (sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)); // input queue
            tInterfaceParameters.queue_sizes.TxQueueSize = (sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)); // additional tx 
        #else
            tInterfaceParameters.queue_sizes.RxQueueSize = 512;          // optional input queue (used only when no call-back defined)
            tInterfaceParameters.queue_sizes.TxQueueSize = 1024;         // additional tx buffer
        #endif
        #if defined WAKE_BLOCKED_USB_TX
            #if defined USB_RNDIS_HOST
            tInterfaceParameters.low_water_level = 0;                    // TX_FREE event on empty transmit buffer
            #else
            tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
            #endif
        #endif
            USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoints with defined configurations (initially inactive)
        #if defined USB_RNDIS_HOST                                       // interrupt endpoint
            tInterfaceParameters.usConfig = 0;
            tInterfaceParameters.Endpoint = ucCDC_InterruptEndpoint[RNDIS_HOST_INTERRUPT_INTERFACE];
            tInterfaceParameters.Paired_RxEndpoint = 0;                  // no pairing
            tInterfaceParameters.queue_sizes.RxQueueSize = usCDC_InterruptEndpointSize[RNDIS_HOST_INTERRUPT_INTERFACE];
            tInterfaceParameters.queue_sizes.TxQueueSize = 0;            // no transmission on the interrupt endpoint
            USBPortID_interrupt[FIRST_CDC_INTERFACE_HANDLE] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoint with defined configurations (initially inactive)
        #endif
        }
        #if (USB_CDC_COUNT > 1)
        if ((NO_ID_ALLOCATED == USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE + 1]) && (ucCDCBulkOutEndpoint[SECOND_CDC_HOST_INTERFACE] != 0)) { // if the host endpoint is known but hasn't been opened
            tInterfaceParameters.owner_task = OWN_TASK;                  // wake usb task on receptions
            tInterfaceParameters.Endpoint = ucCDCBulkOutEndpoint[SECOND_CDC_HOST_INTERFACE]; // set USB endpoints to act as an input/output pair - transmitter (OUT)
            tInterfaceParameters.Paired_RxEndpoint = ucCDCBulkInEndpoint[SECOND_CDC_HOST_INTERFACE]; // receiver (IN)
            tInterfaceParameters.usEndpointSize = usCDCBulkOutEndpointSize[SECOND_CDC_HOST_INTERFACE]; // endpoint queue size (2 buffers of this size will be created for reception)
            tInterfaceParameters.usb_callback = 0;                       // no call-back since we use rx buffer - the same task is owner
            tInterfaceParameters.usConfig = USB_IN_AUTO_POLL;            // automatically restart IN polling after data reception (possible on HS USB and ignored on FS USB)
            tInterfaceParameters.queue_sizes.RxQueueSize = 512;          // optional input queue (used only when no call-back defined)
            tInterfaceParameters.queue_sizes.TxQueueSize = 1024;         // additional tx buffer
            #if defined WAKE_BLOCKED_USB_TX
            tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
            #endif            
            USBPortID_comms[FIRST_CDC_INTERFACE_HANDLE + 1] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoints with defined configurations (initially inactive)
            #if defined USB_RNDIS_HOST                                   // interrupt endpoint
            tInterfaceParameters.usConfig = 0;
            tInterfaceParameters.Endpoint = ucCDC_InterruptEndpoint[SECOND_CDC_HOST_INTERFACE];
            tInterfaceParameters.Paired_RxEndpoint = 0;                  // no pairing
            tInterfaceParameters.queue_sizes.RxQueueSize = usCDC_InterruptEndpointSize[SECOND_CDC_HOST_INTERFACE];
            tInterfaceParameters.queue_sizes.TxQueueSize = 0;            // no transmission on the interrupt endpoint
            USBPortID_interrupt[VCOM_CDC_INTERFACE_HANDLE] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoint with defined configurations (initially inactive)
            #endif
        }
        #endif
    }
    #endif
    #if defined USB_HOST_HUB
    if ((ulHostClassTypes & USB_HOST_HUB_ACTIVE) != 0) {
        if (NO_ID_ALLOCATED == USBPortID_hub_host) {
        #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
            tInterfaceParameters.ucUSB_controller = USB_HOST_REF;        // the HS USB controller used
        #endif
            tInterfaceParameters.owner_task = OWN_TASK;                  // wake usb task on receptions
            tInterfaceParameters.Endpoint = ucHub_InterruptEndpoint;     // set USB endpoints to act as an interrupt IN
            tInterfaceParameters.Paired_RxEndpoint = 0;                  // no pairing
            tInterfaceParameters.usEndpointSize = usHub_InterruptEndpointSize; // endpoint queue size (2 buffers of this size will be created for reception)
            tInterfaceParameters.usb_callback = hub_interrupt_callback;  // use interrupt callback on hub status messages
            tInterfaceParameters.usConfig = 0;
            tInterfaceParameters.queue_sizes.RxQueueSize = 0;
            tInterfaceParameters.queue_sizes.TxQueueSize = 0;
            #if defined WAKE_BLOCKED_USB_TX
            tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
            #endif
            USBPortID_hub_host = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoints with defined configurations (initially inactive)
        }
    }
    #endif
    fnSetUSBConfigState(USB_CONFIG_ACTIVATE, ucActiveConfiguration, USB_HOST_REF); // now activate the configuration
}
#endif

#if defined USB_DEVICE_SUPPORT
    #if (defined USB_CDC_RNDIS || defined USB_RNDIS_HOST) && defined USB_TO_TCP_IP && defined USE_IGMP
static void fnUSBModifyMulticastFilter(QUEUE_TRANSFER action, unsigned char *ptrIP)
{
    _EXCEPTION("Not implemented");
}
    #endif
#endif

#if defined USB_DEVICE_SUPPORT
static void fnConfigureDeviceApplicationEndpoints(int iChannel)
{
    USBTABLE tInterfaceParameters;                                       // table for passing information to driver
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    tInterfaceParameters.ucUSB_controller = (unsigned char)iChannel;
    #endif
    QUEUE_HANDLE endpointNumber = 1;
    #if defined USE_USB_CDC
        #if !defined CDC_WITHOUT_INTERRUPT_ENDPOINT
    QUEUE_HANDLE endpointNumberInt;
        #endif
        #if (USB_CDC_COUNT > 1)
    int iCDC_interface = 1;                                              // additional CDC interfaces
        #endif
    #endif

    #if defined SUPPORT_QUEUE_DEALLOCATION
    tInterfaceParameters.queue_sizes.ucHeapType = 0;                     // the driver queues created here use non-allocatable heap memory
    #endif
    tInterfaceParameters.owner_task = OWN_TASK;                          // USB task is woken on receptions
    tInterfaceParameters.usConfig = 0;
    #if defined IN_COMPLETE_CALLBACK
    tInterfaceParameters.INcallback = 0;
    #endif
    #if defined USE_USB_CDC
        #if defined _LPC23XX || defined _LPC17XX                         // {14}
    tInterfaceParameters.Endpoint = 5;                                   // set USB endpoints to act as an input/output pair - transmitter (IN)
    tInterfaceParameters.Paired_RxEndpoint = 2;                          // receiver (OUT)
        #else
    tInterfaceParameters.Paired_RxEndpoint = endpointNumber;             // receiver (OUT)
            #if !defined USB_SIMPLEX_ENDPOINTS
    endpointNumber++;
            #endif
    tInterfaceParameters.Endpoint = endpointNumber;                      // set USB endpoints to act as an input/output pair - transmitter (IN)
            #if !defined CDC_WITHOUT_INTERRUPT_ENDPOINT
    endpointNumberInt = (endpointNumber + 1);
    endpointNumber++;
            #endif
        #endif    
    tInterfaceParameters.usb_callback = 0;                               // no call-back since we use rx buffer - the same task is owner
        #if defined USB_CDC_RNDIS
    tInterfaceParameters.usEndpointSize = RNDIS_DATA_ENDPOINT_SIZE;      // endpoint queue size (2 buffers of this size will be created for reception)
    tInterfaceParameters.queue_sizes.RxQueueSize = (sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)); // optional input queue (used only when no call-back defined)
            #if defined USB_TX_MESSAGE_MODE
    tInterfaceParameters.queue_sizes.TxQueueSize = ((3 * sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)) + RNDIS_DATA_ENDPOINT_SIZE); // tx buffer size (allow 3 full size Ethernet frames to be queued)
            #else
    tInterfaceParameters.queue_sizes.TxQueueSize = (sizeof(REMOTE_NDIS_ETHERNET_PACKET_MESSAGE)); // single full-size tx buffer since there will be only one message queued at a time
            #endif
    tInterfaceParameters.usConfig = (USB_IN_MESSAGE_MODE | (5 << USB_IN_FIFO_SHIFT)); // configure the IN endpoint to message mode with support for 5 messages queued
        #else
    tInterfaceParameters.usEndpointSize = CDC_DATA_ENDPOINT_SIZE;        // endpoint queue size (2 buffers of this size will be created for reception)
            #if SIZE_OF_RAM <= (16 * 1024)                               // use smaller buffers when there is limited RAM available
    tInterfaceParameters.queue_sizes.RxQueueSize = 64;                   // optional input queue (used only when no call-back defined)
    tInterfaceParameters.queue_sizes.TxQueueSize = 256;                  // additional tx buffer
            #else
    tInterfaceParameters.queue_sizes.RxQueueSize = 256;                  // optional input queue (used only when no call-back defined)
    tInterfaceParameters.queue_sizes.TxQueueSize = 1024;                 // additional tx buffer
            #endif
    tInterfaceParameters.usConfig = USB_TERMINATING_ENDPOINT;            // {13} configure the IN endpoint to terminate messages with a zero length frame if a block transmission equals the endpoint size
        #endif
        #if defined WAKE_BLOCKED_USB_TX
    tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
        #endif
        #if defined MODBUS_USB_SLAVE && defined USE_USB_CDC && (MODBUS_USB_INTERFACE_BASE == 0) // {11}{23}
    tInterfaceParameters.owner_task = TASK_MODBUS;                       // receptions on this endpoint are to be handled by the MODBUS task
        #endif

    USBPortID_comms[0] = fnOpen(TYPE_USB, 0, &tInterfaceParameters);     // open the endpoints with defined configurations (initially inactive)
        #if defined USB_CDC_RNDIS && defined USB_TO_TCP_IP
    fnConfigRNDIS_interface();
        #endif
        #if defined MODBUS_USB_SLAVE && defined USE_USB_CDC && (MODBUS_USB_INTERFACE_BASE == 0)
    tInterfaceParameters.owner_task = OWN_TASK;
        #endif
        #if USB_CDC_COUNT > 1                                            // {33} composite with multiple CDC interfaces
    tInterfaceParameters.usEndpointSize = CDC_DATA_ENDPOINT_SIZE;        // endpoint queue size (2 buffers of this size will be created for reception)
            #if SIZE_OF_RAM < (32 * 1024) && (USB_CDC_VCOM_COUNT > 1)
    tInterfaceParameters.queue_sizes.RxQueueSize = 64;                   // reduce the queue sizes used for the extended CDC interfaces when there is little memory
    tInterfaceParameters.queue_sizes.TxQueueSize = 128;
            #elif defined USB_CDC_RNDIS
    tInterfaceParameters.queue_sizes.RxQueueSize = 256;                  // optional input queue (used only when no call-back defined)
    tInterfaceParameters.queue_sizes.TxQueueSize = 1024;                 // additional tx buffer
            #elif defined TEST_PIT_SINE_TO_USB
    tInterfaceParameters.queue_sizes.RxQueueSize = 256;                  // optional input queue (used only when no call-back defined)
    tInterfaceParameters.queue_sizes.TxQueueSize = (4 * 1024);           // additional tx buffer
            #endif
            #if !defined USB_SIMPLEX_ENDPOINTS
    endpointNumber++;
            #endif
    while (iCDC_interface < USB_CDC_COUNT) {                             // for each additional CDC interface
            #if defined USB_SIMPLEX_ENDPOINTS
        endpointNumber++;
        tInterfaceParameters.Paired_RxEndpoint = tInterfaceParameters.Endpoint = endpointNumber++; 
            #else
        tInterfaceParameters.Paired_RxEndpoint = endpointNumber++;       // receiver (OUT)
        tInterfaceParameters.Endpoint = endpointNumber;                  // set USB endpoints to act as an input/output pair - transmitter (IN)
        endpointNumber += 2;
            #endif
            #if defined CDC_WITHOUT_INTERRUPT_ENDPOINT
        endpointNumber--;
            #endif
            #if defined MODBUS_USB_SLAVE && defined USE_USB_CDC
        if ((iCDC_interface >= MODBUS_USB_INTERFACE_BASE) && (iCDC_interface < (MODBUS_USB_INTERFACE_BASE + MODBUS_USB_CDC_COUNT))) {
            tInterfaceParameters.owner_task = TASK_MODBUS;               // receptions on this endpoint are to be handled by the MODBUS task
        }
            #endif
        USBPortID_comms[iCDC_interface++] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoints with defined configurations (initially inactive)
            #if defined MODBUS_USB_SLAVE && defined USE_USB_CDC
        tInterfaceParameters.owner_task = OWN_TASK;                      // USB task to handle next interfaces if not overwritten
            #endif
            #if defined TEST_PIT_SINE_TO_USB
        ptrUsbCDC_TxQueue = (USBQUE *)que_ids[USBPortID_comms[1] - 1].output_buffer_control; // set to output control block
            #endif
    }
        #endif
        #if !defined CDC_WITHOUT_INTERRUPT_ENDPOINT
    tInterfaceParameters.usConfig = 0;                                   // {13}
            #if defined _LPC23XX || defined _LPC17XX                     // {14}
    tInterfaceParameters.Endpoint = 4;                                   // set interrupt endpoint
            #else
    tInterfaceParameters.Endpoint = endpointNumberInt;                   // set interrupt endpoint
            #endif
    tInterfaceParameters.Paired_RxEndpoint = 0;                          // no pairing
    tInterfaceParameters.owner_task = 0;                                 // don't wake task on reception
    tInterfaceParameters.usb_callback = 0;                               // no call back function
    tInterfaceParameters.queue_sizes.TxQueueSize = tInterfaceParameters.queue_sizes.RxQueueSize = 0; // no buffering
    USBPortID_interrupt[0] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoint with defined configurations (initially inactive)
            #if USB_CDC_COUNT > 1                                        // {33}
    iCDC_interface = 1;
    while (iCDC_interface < USB_CDC_COUNT) {                             // configure endpoints for each CDC interface
                #if defined USB_SIMPLEX_ENDPOINTS
        endpointNumberInt += 2;                                          // 2 endpoints for each additional CDC interface (1 interrupt, 1 bulk IN/OUT)
                #else
        endpointNumberInt += 3;                                          // 3 endpoints for each additional CDC interface (1 interrupt, 1 bulk IN and 1 bulk OUT)
                #endif
        tInterfaceParameters.Endpoint = endpointNumberInt;
        USBPortID_interrupt[iCDC_interface++] = fnOpen(TYPE_USB, 0, &tInterfaceParameters); // open the endpoint with defined configurations (initially inactive)
    }
            #endif
            #if defined USB_SIMPLEX_ENDPOINTS || (USB_CDC_COUNT == 1)
    endpointNumber++;
            #endif
        #endif
    #endif
    #if defined USE_USB_MSD
        #if defined USB_HS_INTERFACE && !defined USB_FS_INTERFACE
    tInterfaceParameters.usEndpointSize = 512;                           // endpoint queue size
        #else
    tInterfaceParameters.usEndpointSize = 64;                            // endpoint queue size (2 buffers of this size will be created for reception)
        #endif
        #if defined _LPC23XX || defined _LPC17XX                         // {14}
    tInterfaceParameters.Endpoint = 5;                                   // set USB endpoints to act as an input/output pair - transmitter (IN)
    tInterfaceParameters.Paired_RxEndpoint = 2;                          // receiver (OUT)
        #else
    tInterfaceParameters.Paired_RxEndpoint = endpointNumber;             // receiver (OUT)
            #if !defined USB_SIMPLEX_ENDPOINTS
    endpointNumber++;
            #endif
    tInterfaceParameters.Endpoint = endpointNumber++;                    // set USB endpoints to act as an input/output pair - transmitter (IN)
        #endif
    tInterfaceParameters.usb_callback = mass_storage_callback;           // allow receptions to be 'peeked' by call-back
    tInterfaceParameters.queue_sizes.RxQueueSize = 512;                  // optional input queue (used only when no call-back defined) and large enough to hold a full sector
    tInterfaceParameters.queue_sizes.TxQueueSize = 1024;                 // additional tx buffer
        #if defined WAKE_BLOCKED_USB_TX
    tInterfaceParameters.low_water_level = (tInterfaceParameters.queue_sizes.TxQueueSize/2); // TX_FREE event on half buffer empty
        #endif
    tInterfaceParameters.owner_task = OWN_TASK;                          // wake the USB task on receptions
    USBPortID_MSD = fnOpen(TYPE_USB, 0, &tInterfaceParameters);          // open the endpoints with defined configurations (initially inactive)
    #endif
    #if defined USE_USB_HID_MOUSE || defined USE_USB_HID_KEYBOARD        // {34}
    tInterfaceParameters.usEndpointSize = 8;                             // endpoint queue size (2 buffers of this size will be created for reception)
        #if defined _LPC23XX || defined _LPC17XX
    tInterfaceParameters.Endpoint = 5;
        #else
    tInterfaceParameters.Endpoint = endpointNumber++;                    // the endpoint used by HID device
        #endif
    tInterfaceParameters.usb_callback = 0;
    tInterfaceParameters.queue_sizes.TxQueueSize = 0;
    tInterfaceParameters.queue_sizes.RxQueueSize = 0;
    tInterfaceParameters.Paired_RxEndpoint = 0;                          // no pairing
        #if defined USE_USB_HID_MOUSE
    USBPortID_HID_mouse = fnOpen(TYPE_USB, 0, &tInterfaceParameters);    // open the endpoints with defined configurations (initially inactive)
        #endif
        #if defined USE_USB_HID_KEYBOARD
            #if defined USE_USB_HID_MOUSE
    tInterfaceParameters.Endpoint = endpointNumber++;                    // the endpoint used by HID keyboard
            #endif
            #if defined IN_COMPLETE_CALLBACK
    tInterfaceParameters.INcallback = fnKeyboardPolled;
            #endif
    USBPortID_HID_kb = fnOpen(TYPE_USB, 0, &tInterfaceParameters);       // open the endpoints with defined configurations (initially inactive)
        #endif
    #endif
    #if defined USE_USB_HID_RAW
    tInterfaceParameters.usEndpointSize = HID_RAW_RX_SIZE;               // endpoint queue size (2 buffers of this size will be created for reception)
    tInterfaceParameters.Endpoint = endpointNumber++;                    // the endpoint used by HID raw device for reception
    tInterfaceParameters.usb_callback = 0;
    tInterfaceParameters.queue_sizes.TxQueueSize = 0;
    tInterfaceParameters.queue_sizes.RxQueueSize = (2 * HID_RAW_RX_SIZE);// reception buffer with space for two complete HID raw messages
    tInterfaceParameters.Paired_RxEndpoint = endpointNumber++;
        #if defined IN_COMPLETE_CALLBACK
    tInterfaceParameters.INcallback = fnRawHIDPolled;
        #endif
    USBPortID_HID_raw = fnOpen(TYPE_USB, 0, &tInterfaceParameters);      // open the endpoints with defined configurations (initially inactive)
    #endif
    #if defined USE_USB_AUDIO
        #if defined AUDIO_FFT                                            // {42}
    window_conversionFactor = fnGenerateWindowFloat(windowing_buffer, FFT_INPUT_SAMPLES, BLACKMANN_HARRIS_WINDOW); // calculate a window for use by the FFT
        #endif
    tInterfaceParameters.usEndpointSize = ISOCHRONOUS_ENDPOINT_SIZE;     // isochronous endpoint size
    tInterfaceParameters.Paired_RxEndpoint = endpointNumber;             // the endpoint used by the audio device for reception
        #if !defined USB_SIMPLEX_ENDPOINTS
    endpointNumber++;
        #endif
    tInterfaceParameters.Endpoint = endpointNumber++;                    // paired endpoint for transmission (IN)
    #if !defined MIC_ONLY
    tInterfaceParameters.usb_callback = fnAudioPacket;                   // user callback when audio packet has been received
    #else
    tInterfaceParameters.usb_callback = 0;
    #endif
    #if defined MIC_ONLY
    tInterfaceParameters.queue_sizes.RxQueueSize = 0;
        #if defined MIC_STEREO
    tInterfaceParameters.queue_sizes.TxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT * 2);
        #else
    tInterfaceParameters.queue_sizes.TxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT);
        #endif
    #else
    tInterfaceParameters.queue_sizes.TxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT);
    tInterfaceParameters.queue_sizes.RxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT); // reception buffer with space for two complete HID raw messages
    #endif    
    tInterfaceParameters.usConfig = (USB_IN_ZERO_COPY_ISO | USB_IN_ZERO_COPY | USB_OUT_ZERO_COPY); // the buffers are accessed directly by the USB controller to avoid the need for the CPU to copy data 
        #if defined IN_COMPLETE_CALLBACK
            #if defined MIC_ONLY
    tInterfaceParameters.INcallback = mic_in_complete;
            #else
    tInterfaceParameters.INcallback = 0;
            #endif
        #endif
    USBPortID_Audio = fnOpen(TYPE_USB, 0, &tInterfaceParameters);        // open the endpoints with defined configurations (initially inactive)
        #if defined MIC_ONLY && !defined SPEAKER_ONLY
    fnAudioIn(USBPortID_Audio, tInterfaceParameters.queue_sizes.TxQueueSize); // initialise the audio input (microphone)
        #endif
        #if !defined MIC_ONLY
    fnAudioOut(USBPortID_Audio, tInterfaceParameters.queue_sizes.RxQueueSize); // initialise the audio output (speaker)
        #endif
    #endif
}

    #if defined PARALLEL_USD_DEVICE_INTERFACE
static void fnConfigureHighSpeedDeviceApplicationEndpoints(int iChannel)
{
    USBTABLE tInterfaceParameters;                                       // table for passing information to driver
        #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (HSUSB_CONTROLLERS > 1))
    tInterfaceParameters.ucUSB_controller = (unsigned char)iChannel;
        #endif
    QUEUE_HANDLE endpointNumber = 1;

    tInterfaceParameters.owner_task = OWN_TASK;                          // USB task is woken on receptions
    tInterfaceParameters.usConfig = 0;
        #if defined IN_COMPLETE_CALLBACK
    tInterfaceParameters.INcallback = 0;
        #endif
        #if defined USE_HS_USB_AUDIO                                    // audio class on HS interface
    tInterfaceParameters.usEndpointSize = ISOCHRONOUS_ENDPOINT_SIZE;     // isochronous endpoint size
    tInterfaceParameters.Paired_RxEndpoint = endpointNumber;             // the endpoint used by the audio device for reception
            #if !defined USB_SIMPLEX_ENDPOINTS_ // temp
    endpointNumber++;
            #endif
    tInterfaceParameters.Endpoint = endpointNumber++;                    // paired endpoint for transmission (IN)
            #if !defined MIC_ONLY
    tInterfaceParameters.usb_callback = fnAudioPacket;                   // user callback when audio packet has been received
            #else
    tInterfaceParameters.usb_callback = 0;
            #endif
    tInterfaceParameters.queue_sizes.TxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT);
    tInterfaceParameters.queue_sizes.RxQueueSize = (ISOCHRONOUS_ENDPOINT_SIZE * AUDIO_BUFFER_COUNT); // reception buffer with space for two complete HID raw messages
    tInterfaceParameters.usConfig = (USB_IN_ZERO_COPY_ISO | USB_IN_ZERO_COPY | USB_OUT_ZERO_COPY); // the buffers are accessed directly by the USB controller to avoid the need for the CPU to copy data 
            #if defined IN_COMPLETE_CALLBACK
                #if defined MIC_ONLY
    tInterfaceParameters.INcallback = mic_in_complete;;
                #else
    tInterfaceParameters.INcallback = 0;
                #endif
            #endif
    USBPortID_Audio = fnOpen(TYPE_USB, 0, &tInterfaceParameters);        // open the endpoints with defined configurations (initially inactive)
            #if defined MIC_ONLY && !defined SPEAKER_ONLY
    fnAudioIn(USBPortID_Audio, tInterfaceParameters.queue_sizes.TxQueueSize); // initialise the audio input (microphone)
            #endif
            #if !defined MIC_ONLY
    fnAudioOut(USBPortID_Audio, tInterfaceParameters.queue_sizes.RxQueueSize); // initialise the audio output (speaker)
           #endif
        #endif
}
    #endif
#endif


#if defined USB_INTERFACE || defined USB_HOST_SUPPORT
// The USB interface is configured by opening the USB interface once for the default control endpoint 0,
// followed by an open of each endpoint to be used (each endpoint has its own handle). Each endpoint can use an optional call-back
// or can define a task to be woken on OUT frames. Transmission can use direct memory method or else an output buffer (size defined by open),
// and receptions can use an optional input buffer (size defined by open).
//
static QUEUE_HANDLE fnConfigureUSB(int iChannel)
{
    USBTABLE tInterfaceParameters;                                       // table for passing information to driver
    QUEUE_HANDLE USB_Handle;
    #if defined PARALLEL_USD_DEVICE_INTERFACE || (defined _iMX && (USBS_AVAILABLE > 1))
    tInterfaceParameters.ucUSB_controller = (unsigned char)iChannel;
    #endif
    #if defined SUPPORT_QUEUE_DEALLOCATION
    tInterfaceParameters.queue_sizes.ucHeapType = 0;                     // the driver queues created here use non-allocatable heap memory
    #endif

    // Configure the control endpoint
    //
    tInterfaceParameters.Endpoint = 0;                                   // set USB default control endpoint for configuration
    #if defined USB_HOST_SUPPORT && !defined USB_DEVICE_SUPPORT          // {32}{47} host mode (only) rather than device mode
    tInterfaceParameters.usConfig = USB_HOST_MODE;                       // configure host mode of operation
    #else
        #if defined OLIMEX_LPC2378_STK                                   // {20} this board requires alternative USB pin mapping
    tInterfaceParameters.usConfig = (USB_FULL_SPEED | USB_ALT_PORT_MAPPING); // full-speed, rather than low-speed - use alternative pins
        #else
            #if defined PARALLEL_USD_DEVICE_INTERFACE
    if (iChannel == USB_HS_DEVICE_REF) {
        tInterfaceParameters.usConfig = USB_HIGH_SPEED;                  // use high speed interface
    }
    else {
        tInterfaceParameters.usConfig = USB_FULL_SPEED;                  // full-speed, rather than low-speed
    }
            #elif defined USB_HS_INTERFACE                               // {27}
    tInterfaceParameters.usConfig = USB_HIGH_SPEED;                      // use high speed interface
            #else
    tInterfaceParameters.usConfig = USB_FULL_SPEED;                      // full-speed, rather than low-speed
            #endif
        #endif
    #endif
    #if defined USB_HOST_SUPPORT && defined USB_DEVICE_SUPPORT           // {47}
    if (USB_HOST_MODE_OF_OPERATION == iUSB_mode[iChannel]) {             // if we are in host mode
        tInterfaceParameters.usConfig = USB_HOST_MODE;                   // configure host mode of operation
        tInterfaceParameters.ucEndPoints = NUMBER_OF_ENDPOINTS_REQUIRED; // number of endpoints, in addition to EP0, required by the configuration
    }
    else {
    #endif
    #if defined _LPC23XX || defined _LPC17XX                             // {14}
        tInterfaceParameters.ucEndPoints = 5;                            // due to fixed endpoint ordering in the LPC endpoints up to 5 are used
    #else
        tInterfaceParameters.ucEndPoints = NUMBER_OF_ENDPOINTS_REQUIRED; // number of endpoints, in addition to EP0, required by the configuration
    #endif
    #if defined USB_HOST_SUPPORT && defined USB_DEVICE_SUPPORT 
    }
    #endif
    tInterfaceParameters.usb_callback = control_callback;                // call-back for control endpoint to enable class exchanges to be handled
    tInterfaceParameters.queue_sizes.TxQueueSize = 0;                    // no tx buffering on control endpoint
    tInterfaceParameters.queue_sizes.RxQueueSize = 0;                    // no rx buffering on control endpoint
    #if defined _KINETIS | defined _iMX                                  // {18}{25}{26}{29}{30}
    tInterfaceParameters.ucClockSource = INTERNAL_USB_CLOCK;             // use internal clock source
    #else
    tInterfaceParameters.ucClockSource = EXTERNAL_USB_CLOCK;             // use 48MHz crystal directly as USB clock (recommended for lowest jitter)
    #endif
    tInterfaceParameters.owner_task = OWN_TASK;                          // local task receives USB state change events
    #if defined USE_USB_OTG_CHARGE_PUMP
    tInterfaceParameters.OTG_I2C_Channel = I2CPortID;                    // let the driver open its own I2C interface, if not yet open
    #endif
    USB_Handle = fnOpen(TYPE_USB, 0, &tInterfaceParameters);             // open the default control endpoint with defined configurations (reserves resources but only control is active)
    #if defined USB_DEVICE_SUPPORT && defined USB_HOST_SUPPORT           // {47}
    if (USB_HOST_MODE_OF_OPERATION == iUSB_mode[iChannel]) {             // if we are configuring host mode
        return USB_Handle;                                               // don't perform device endpoint configuration
    }
    #endif
    #if defined USB_DEVICE_SUPPORT
        #if defined PARALLEL_USD_DEVICE_INTERFACE
    if (iChannel == USB_HS_DEVICE_REF) {
        fnConfigureHighSpeedDeviceApplicationEndpoints(iChannel);        // configure the endpoints that will be used by the high speed device class/application
    }
    else {
        fnConfigureDeviceApplicationEndpoints(iChannel);                 // configure the endpoints that will be used by the device class/application
    }
        #else
    fnConfigureDeviceApplicationEndpoints(iChannel);                     // configure the endpoints that will be used by the device class/application
        #endif
    #endif
    #if defined USB_RNDIS_HOST && defined USB_TO_TCP_IP
    fnConfigRNDIS_interface();
    #endif
    return USB_Handle;                                                   // the USB endpoint 0 handle
}
__PACK_OFF
#endif
#endif