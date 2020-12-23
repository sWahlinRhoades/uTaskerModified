/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      disk_loader.c
    Project:   uTasker disk loader
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    03.03.2012 Add TWR_K53N512                                           {1}
    19.09.2012 Close Flash buffer when device has flash with row writing {2}
    25.11.2013 Limit wait when no SD card detected                       {3}
    18.12.2013 Allow another loader method to stop application from starting {4}
    18.12.2013 Add peripheral resets when jumping to the application     {5}
    18.12.2013 Add file system entry after SD card loading if used together with USB-MSD {6}
    17.01.2014 Pass directory pointer to utFAT2.0 version of utOpenFile(){7}
    15.05.2014 Add optional SD card wildcard file name matching          {8}
    15.05.2014 Add decryption of SD card content                         {9}
    11.12.2014 Add check of oversize SD card file                        {10}
    21.01.2015 Add file object whenever USB-MSD is enabled               {11}
    02.07.2015 Allow existing application to start if no matching file is found and using wildcard matching {12}
    21.07.2015 Add option to start the application by pressing a button (to restart update check and to jump to the [new] application) {13}
    23.10.2015 Renamed from SDLoader.c to disk_loader.c
    07.01.2016 Use fnJumpToValidApplication() to start application       {14}
    20.09.2017 Correct decryption when offset is larger than the file length {15}
    04.02.2018 Allow loading to data flash area (flexflash)              {16}
    04.02.2018 Allow limiting internal flash chunk erase sizes to avoid blocking operation when large flash is used {17}
    04.02.2018 Allow limiting flash erasure to the program size to be loaded {18}
    09.11.2018 Add AES-256 decryption option to disk loaders (SDCARD_SECURE_LOADER)
    14.04.2019 Correct disk pointer initialisation (after SDCARD_SECURE_LOADER option added) {19}
    14.09.2020 Add multiple file loading support

*/

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"

#if ((defined SDCARD_SUPPORT || defined SPI_FLASH_FAT) && !defined SDCARD_ACCESS_WITHOUT_UTFAT) || defined USB_MSD_HOST_LOADER

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#define OWN_TASK                   TASK_SD_LOADER

#define STATE_INIT                 0                                     // task states
#define STATE_ACTIVE               1
#define STATE_START_CHECKING       2
#define STATE_CHECKING             3
#define STATE_CHECK_SECRET_KEY     4
#define STATE_DELETING_FLASH       5
#define STATE_PROGRAMMING          6
#define STATE_VERIFYING            7


#define T_JUMP_TO_APP              1
#define T_CHECK_CARD               2
#define T_DELAYED_JUMP_TO_APP      3

#define E_DO_NEXT                  1
#define E_DO_SEARCH                2
#define E_RECHECK_SW               3

#define COPY_BUFFER_SIZE           256                                   // file processed in blocks of this size from SD card
#if defined FLEXFLASH_DATA || defined SPI_FILE_SYSTEM || defined _iMX || (defined SDCARD_SUPPORT && defined SDCARD_SECURE_LOADER)
    #define CRC_BLOCK_SIZE         COPY_BUFFER_SIZE                      // use the same (size) buffer for both copy and CRC check
#else
    #define CRC_BLOCK_SIZE         1024                                  // CRC check in internal flash made in blocks of this size
#endif

#if defined USB_MSD_HOST_LOADER
    #define LOADING_DISK           DISK_E
#else
    #define LOADING_DISK           DISK_D
#endif

#if defined ENCRYPTED_CARD_CONTENT                                       // ensure length is compiler independent
    #define SIZE_OF_UPLOAD_HEADER (sizeof(unsigned long) + (3 * sizeof(unsigned short)))
#else
    #define SIZE_OF_UPLOAD_HEADER (sizeof(unsigned long) + (2 * sizeof(unsigned short)))
#endif

#if !defined SDCARD_FILE_COUNT
    #define SDCARD_FILE_COUNT      1
#endif

#define SEARCH_ACTIVE              0x1
#define UPDATE_ACTIVE              0x2

/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

typedef struct _PACK stUPLOAD_HEADER
{
    unsigned long  ulCodeLength;
    unsigned short usMagicNumber;
    unsigned short usCRC;
#if defined ENCRYPTED_CARD_CONTENT                                       // {9}
    unsigned short usRAWCRC;
#endif
} UPLOAD_HEADER;

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

#if defined ENCRYPTED_CARD_CONTENT                                       // {9}
    static unsigned short fnDecrypt(unsigned char *ptrData, unsigned short usBlockSize);
#endif
static int fnUpdateSoftware(int iRef);
#if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)
    extern void fnHeaderToLittleEndian(UPLOAD_HEADER *file_header);
#endif
#if defined WILDCARD_FILES                                               // {8}
    static int fnWildcardMatch(CHAR *cString, CHAR *cPattern);
#endif
#if defined START_ON_INTERRUPT
    static void fnEnableRepeatInterrupt(void);                           // {13}
#endif
static int fnPrepareFileSearch(void);
#if defined WILDCARD_FILES
    static int fnNextSearch(void);
#endif
static void fnJumpToApplication(int iGo);                                // {3}

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

static const unsigned char ucSecretKey[] = _SECRET_KEY;

#if defined VARIABLE_FW_DIRECTORY
static const CHAR *firmwareDiskLocation[SDCARD_FILE_COUNT] = {           // specify in which directory the firmware file(s) is/are located
    FIRMWARE_DISK_LOCATION,
    #if SDCARD_FILE_COUNT > 1
    FIRMWARE2_DISK_LOCATION,
    #endif
    #if SDCARD_FILE_COUNT > 2
    FIRMWARE3_DISK_LOCATION,
    #endif
};

static const CHAR *firmware_name[SDCARD_FILE_COUNT] = {                  // specify the name of the file(s) to be loaded
    NEW_SOFTWARE_FILE,
    #if SDCARD_FILE_COUNT > 1
    NEW_SOFTWARE2_FILE,
    #endif
    #if SDCARD_FILE_COUNT > 2
    NEW_SOFTWARE3_FILE,
    #endif
};
#endif

#if SDCARD_FILE_COUNT > 1
static unsigned char *firmwareDestination[SDCARD_FILE_COUNT] = {
    (unsigned char *)_UTASKER_APP_START_,
    (unsigned char *)(FIRMWARE2_DESTINATION_ADDRESS),
    #if SDCARD_FILE_COUNT > 2
    (unsigned char *)(FIRMWARE3_DESTINATION_ADDRESS)
    #endif
};

static unsigned char *firmwareEnd[SDCARD_FILE_COUNT] = {
    UTASKER_APP_END,
    (unsigned char *)(FIRMWARE2_DESTINATION_ADDRESS + FIRMWARE2_MAX_SIZE),
    #if SDCARD_FILE_COUNT > 2
    (unsigned char *)((FIRMWARE3_DESTINATION_ADDRESS) + FIRMWARE3_MAX_SIZE)
    #endif
};
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if defined SDCARD_SECURE_LOADER
    static unsigned char ucCodeStart[SDCARD_FILE_COUNT][32];             // AES-256 block size
#else
    #if defined _iMX
    static unsigned char ucCodeStart[SDCARD_FILE_COUNT][SIZE_OF_UPLOAD_HEADER];
    #elif defined FLASH_ROW_SIZE && (FLASH_ROW_SIZE > 0)
    static unsigned char ucCodeStart[SDCARD_FILE_COUNT][FLASH_ROW_SIZE];
    #else
    static unsigned char ucCodeStart[SDCARD_FILE_COUNT][4];
    #endif
#endif
#if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD
    static FILE_OBJECT_INFO fileObjInfo = {0};                           // {6}
#endif
#if defined WILDCARD_FILES && defined DELETE_SDCARD_FILE_AFTER_UPDATE
    static CHAR cSWfile[SDCARD_FILE_COUNT][COPY_BUFFER_SIZE] = {{0}};
#endif
#if defined START_ON_INTERRUPT
    static int iStartRequested = 0;                                      // {13}
#endif
static UTDIRECTORY  *ptr_utDirectory[SDCARD_FILE_COUNT] = {0};           // pointer to directory objects
static int iAppState[SDCARD_FILE_COUNT] = {STATE_INIT};                  // task state
static UTFILE        utFile[SDCARD_FILE_COUNT] = {{0}};                  // local file object
static UPLOAD_HEADER file_header[SDCARD_FILE_COUNT] = {{0}};             // copy of the file header
#if defined WILDCARD_FILES                                               // {8}
    static UTLISTDIRECTORY utListDirectory[SDCARD_FILE_COUNT] = {{0}};   // list directory object for directory search
    static FILE_LISTING fileList[SDCARD_FILE_COUNT] = {{0}};
    static CHAR cBuffer[MAX_UTFAT_FILE_NAME + DOS_STYLE_LIST_ENTRY_LENGTH]; // buffer for listing string
#endif


// Application task
//
extern void fnSD_loader(TTASKTABLE *ptrTaskTable)
{
    #if defined WILDCARD_FILES                                           // {8}
    static int iResult = 0;
    #endif
    static unsigned char ucInputMessage[HEADER_LENGTH];                  // reserve space for receiving messages
    QUEUE_HANDLE         PortIDInternal = ptrTaskTable->TaskID;          // queue ID for task input

    if (STATE_INIT == iAppState[0]) {
        int i;
        iAppState[0] = STATE_ACTIVE;
        uMemset(ucCodeStart, 0xff, sizeof(ucCodeStart));
    #if (defined KWIKSTIK || defined TWR_K40X256 || defined TWR_K53N512 || defined TWR_KL43Z48M || defined FRDM_KL43Z || defined TWR_K40D100M || defined FRDM_K32L2B3) && !defined SERIAL_INTERFACE && !defined USB_INTERFACE // {1}
        CONFIGURE_SLCD();
    #endif
    #if defined USB_MSD_HOST_LOADER
        uTaskerStateChange(TASK_USB_HOST, UTASKER_ACTIVATE);             // allow the USB host task to start
        #if defined MEMORY_STICK_TASK
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 5), T_JUMP_TO_APP); // maximum wait for memory stick to mount
        #else
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_CHECK_CARD); // delay to allow memory stick to be mounted
        #endif
    #else
        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_CHECK_CARD);// delay to allow SD card to be mounted
    #endif
        for (i = 0; i < SDCARD_FILE_COUNT; i++) {
            ptr_utDirectory[i] = utAllocateDirectory(LOADING_DISK, 0);   // allocate a directory for use by this module associated with D: without path name string length
        }
    }
    else {
        while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH) != 0) { // check input queue
            if (ucInputMessage[MSG_SOURCE_TASK] == TIMER_EVENT) {        // timer event
                if (T_CHECK_CARD == ucInputMessage[MSG_TIMER_EVENT]) {   // check whether the disk is ready
                    const UTDISK *ptrDiskInfo = fnGetDiskInfo(LOADING_DISK);
    #if defined IGNORE_SD_CARD
                    if ((IGNORE_SD_CARD()) || (ptrDiskInfo->usDiskFlags == 0) || ((ptrDiskInfo->usDiskFlags & DISK_NOT_PRESENT) != 0))
    #else
                    if ((ptrDiskInfo->usDiskFlags == 0) || ((ptrDiskInfo->usDiskFlags & DISK_NOT_PRESENT) != 0)) // no card detected
    #endif
                    {
                        _DISPLAY_SD_CARD_NOT_PRESENT();                  // optionally display that the card is not present
                        fnJumpToApplication(0);                          // {3} jump to application if retries expire (restart timer when not yet true)
                    }
                    else if ((ptrDiskInfo->usDiskFlags & DISK_UNFORMATTED) != 0) {
                        _DISPLAY_SD_CARD_NOT_FORMATTED();                // optionally display that SD card is not formatted
                        fnJumpToApplication(0);                          // {3} jump to application if retries expire (restart timer when not yet true)
                    }
                    else {                                               // formatted disk detected
                        _DISPLAY_SD_CARD_PRESENT();                      // optionally display that card is ready
                        if (fnPrepareFileSearch() != 0) {                // set the firmware directory and start a file search if it exists
                            fnInterruptMessage(OWN_TASK, E_DO_SEARCH);   // schedule search
                        }
                        else {
                            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_JUMP_TO_APP); // {3} allow an existing application to start after short delay to display the state
                        }
                    }
                }
                else if (T_JUMP_TO_APP == ucInputMessage[MSG_TIMER_EVENT]) { // go to application
    #if SDCARD_FILE_COUNT > 1
                    int i;
                    int iActive = 0;
                    for (i = 0; i < SDCARD_FILE_COUNT; i++) {
                        if (iAppState[i] > STATE_ACTIVE) {               // ignore jumps to the application if there is still update activity in progress
                            iActive = 1;
                            break;
                        }
                    }
                    if (iActive != 0) {
                        continue;
                    }
    #endif
    #if defined SPECIAL_VERSION_SDCARD
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 60 * 3), T_DELAYED_JUMP_TO_APP);
                    uTaskerStateChange(TASK_USB, UTASKER_ACTIVATE);      // allow the USB task to start
    #else
                    fnJumpToApplication(1);                              // {3} unconditional jump to application code
    #endif
                }
    #if defined SPECIAL_VERSION_SDCARD
                else if (T_RECHECK_CARD == ucInputMessage[MSG_TIMER_EVENT]) { // USB write activity has taken place so the card needs to be rechecked for new firmware
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 0.1), T_CHECK_CARD);
                }
                else if (T_DELAYED_JUMP_TO_APP == ucInputMessage[MSG_TIMER_EVENT]) {
                    fnJumpToApplication(1);                              // unconditional jump to application code after an additional delay
                }
    #endif
            }
            else {                                                       // interrupt event assumed
                switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {
    #if defined WILDCARD_FILES                                           // {8}
                case E_DO_SEARCH:
                    switch (fnNextSearch()) {
                    case 0:                                              // if no valid firmware has been found and no update is in operation
                        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_JUMP_TO_APP); // {12} allow an existing application to start after short delay to display the state
                        return;
                    case SEARCH_ACTIVE:                                  // still scanning directories
                    case (SEARCH_ACTIVE | UPDATE_ACTIVE):                // still scanning directories (and actively updating a firmware)
                        return;
                    default:                                             // moved to updating phase
                        break;
                    }
                    break;
    #endif
    #if defined MEMORY_STICK_TASK
                case MEM_STICK_MOUNTED:
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0), T_CHECK_CARD);
                    break;
    #endif
    #if defined START_ON_INTERRUPT
                case E_RECHECK_SW:
                    if (iAppState[0] == STATE_ACTIVE) {                  // only accept when idle
                        iStartRequested = 1;                             // always start application after the next check
                        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.1 * SEC), T_CHECK_CARD); // start the check for new SW and start the application
                    }
                    return;
    #endif
                case E_DO_NEXT:
                    {
                        int i;
                        int iContinue = 0;
                        for (i = 0; i < SDCARD_FILE_COUNT; i++) {
                            if (fnUpdateSoftware(i) != 0) {
                                iContinue = 1;                           // updating process not yet complete
                            }
                        }
                        if (iContinue != 0) {
                            fnInterruptMessage(OWN_TASK, E_DO_NEXT);     // schedule next
                            return;
                        }
                    }
                    break;
                }
            }
        }
    }
}

static int fnPrepareFileSearch(void)
{
#if defined WILDCARD_FILES                                               // {8}
    int i;
    int iStartSearch = 0;
    const CHAR *directory = "/";                                         // default location is in the root directory
    for (i = 0; i < SDCARD_FILE_COUNT; i++) {
    #if defined VARIABLE_FW_DIRECTORY
        directory = firmwareDiskLocation[i];
    #endif
        if (utOpenDirectory(directory, ptr_utDirectory[i]) >= UTFAT_SUCCESS) { // open the directory where we expect to find software
            utFile[i].ownerTask = OWN_TASK;
            fileList[i].usMaxItems = 1;                                  // one item at a time
            fileList[i].ptrBuffer = cBuffer;                             // the string buffer to create the listing in
            fileList[i].usBufferLength = sizeof(cBuffer);                // buffer size
            fileList[i].ucStyle = (DOS_TYPE_LISTING | NO_CR_LF_LISTING | NULL_TERMINATE_LISTING);

            utListDirectory[i].ptr_utDirObject = ptr_utDirectory[i];     // reference the list directory to the main directory object
            utLocateDirectory("/", &utListDirectory[i]);
            iStartSearch = 1;                                            // a search should be started
        }
    }
    #if defined FRDM_K64F && defined DEV10
    // It is possible that not all destination devices are mounted so we adjust the destination control to match the first identified one
    // - during firmware updates t0 this destination the first foudn device is checked and updated to
    // - at the end of the procedure all available device are synchonised so that they finally have the same content
    //
    for (i = 0; i < SPI_FLASH_DEVICE_COUNT; i++) {                       // search for first existing flash device
        if (fnSPI_FlashExt_available(i) != 0) {                          // check whether the device exists
            firmwareEnd[1] = (firmwareDestination[1] + FIRMWARE2_MAX_SIZE); // set the end address (which will move if devices were not present)
            break;
        }
        else {                                                           // else if the device is not present
            firmwareDestination[1] += SPI_DATA_FLASH_0_SIZE;             // move to the next device's address
        }
    }
    #endif
    return iStartSearch;
#else
    if (utOpenFile(NEW_SOFTWARE_FILE, &utFile[0], ptr_utDirectory[0], UTFAT_OPEN_FOR_READ) == UTFAT_PATH_IS_FILE) { // {7}
        if (utFile[0].ulFileSize > (MAX_UTASKER_APP_SIZE - SIZE_OF_UPLOAD_HEADER)) { // {10} if the file is too large we ignore it
            _DISPLAY_OVERSIZE_CONTENT();
        }
        else {
            utReadFile(&utFile[0], (unsigned char *)&file_header[0], SIZE_OF_UPLOAD_HEADER); // read the file header
    #if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)
            fnHeaderToLittleEndian(&file_header[0]);
    #endif
            if (utFile[0].ulFileSize == (file_header[0].ulCodeLength + SIZE_OF_UPLOAD_HEADER)) { // content length matches
    #if defined _iMX
                if (((file_header[0].usMagicNumber & ~BOOT_LOADER_TYPE_MASK)) == VALID_VERSION_MAGIC_NUMBER))
    #else
                if ((file_header[0].usMagicNumber == VALID_VERSION_MAGIC_NUMBER))
    #endif
                {                                                        // check test that the header version (magic number) is correct
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER
                    fileObjInfo.usCreationDate = utFile[0].usCreationDate;
                    fileObjInfo.usCreationTime = utFile[0].usCreationTime;
    #endif
                    iAppState[0] = STATE_START_CHECKING;
                    if (fnUpdateSoftware(0) != 0) {
                        fnInterruptMessage(OWN_TASK, E_DO_NEXT);         // schedule next
                    }
                    return 1;
                }
                else {
                    _DISPLAY_INVALID_CONTENT();                          // optionally display that the software file has been found but has invalid content
                }
            }
            else {
                _DISPLAY_INVALID_CONTENT();                              // optionally display that the software file has been found but has invalid content
            }
        }
    }
    else {
        _DISPLAY_NO_FILE();                                              // optionally display that the software file is not present on the SD card
    }
    return 0;                                                            // no search started
#endif
}

#if defined WILDCARD_FILES                                               // {8}
static int fnNextSearch(void)
{
    #if SDCARD_FILE_COUNT > 1
        #define MAX_FIRWARE_FILE_SIZE   ((firmwareEnd[i] - firmwareDestination[i]) + SIZE_OF_UPLOAD_HEADER)
    #else
        #define MAX_FIRWARE_FILE_SIZE   (MAX_UTASKER_APP_SIZE + SIZE_OF_UPLOAD_HEADER)
    #endif
    int i;
    int iContinue = 0;
    int iSearching = 0;
    int iActive = 0;
    #if defined VARIABLE_FW_DIRECTORY
    const CHAR *firmwareName;
    #else
    const CHAR *firmwareName = NEW_SOFTWARE_FILE;
    #endif
    for (i = 0; i < SDCARD_FILE_COUNT; i++) {
        if (iAppState[i] >= STATE_START_CHECKING) {
            iActive = UPDATE_ACTIVE;
            continue;
        }
        if (utListDir(&utListDirectory[i], &fileList[i]) != UTFAT_NO_MORE_LISTING_ITEMS_FOUND) { // search through all files in the firmware directory
            if ((fileList[i].ucFileAttributes & DIR_ATTR_ARCHIVE) != 0) { // if a file
                CHAR *ptrFileName = &cBuffer[fileList[i].usStringLength - fileList[i].ucNameLength]; // set pointer to null terminated file name
    #if defined VARIABLE_FW_DIRECTORY
                firmwareName = firmware_name[i];
    #endif
                if (fnWildcardMatch(ptrFileName, (CHAR *)firmwareName) == 0) { // if the name matches
                    if (utOpenFile(ptrFileName, &utFile[i], ptr_utDirectory[i], UTFAT_OPEN_FOR_READ) == UTFAT_PATH_IS_FILE) { // open the file
                        if (utFile[i].ulFileSize > MAX_FIRWARE_FILE_SIZE) { // {10} if the file is too large we ignore it
                            _DISPLAY_OVERSIZE_CONTENT();
                        }
                        else {
                            utReadFile(&utFile[i], (unsigned char *)&file_header[i], SIZE_OF_UPLOAD_HEADER); // read the file header
    #if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)
                            fnHeaderToLittleEndian(&file_header[i]);
    #endif
                            if (utFile[i].ulFileSize == (file_header[i].ulCodeLength + SIZE_OF_UPLOAD_HEADER)) { // content length matches
    #if defined _iMX
                                if (((file_header[i].usMagicNumber & ~(BOOT_LOADER_TYPE_MASK)) == VALID_VERSION_MAGIC_NUMBER)) // check test that the header version (magic number) is correct (irrespective of type)
    #else
                                if ((file_header[i].usMagicNumber == VALID_VERSION_MAGIC_NUMBER)) // check test that the header version (magic number) is correct
    #endif
                                {
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD
                                    if (i == 0) {                        // the first file is expected to be the processor firmware file, who's name is set to the file object
                                        fileObjInfo.usCreationDate = utFile[0].usCreationDate;
                                        fileObjInfo.usCreationTime = utFile[0].usCreationTime;
                                    }
    #endif
    #if defined DELETE_SDCARD_FILE_AFTER_UPDATE
                                    uStrcpy(cSWfile[i], ptrFileName);    // make a copy of the file name so that it can be deleted later
    #endif
                                    iAppState[i] = STATE_START_CHECKING; // the updating phase can be started
                                    if (iContinue == 0) {                // only perform once
                                        iContinue = UPDATE_ACTIVE;
                                        fnInterruptMessage(OWN_TASK, E_DO_NEXT); // schedule next
                                    }
                                    continue;
                                }
                            }
                            _DISPLAY_INVALID_CONTENT();                  // optionally display that a software file was found but has invalid content
                        }
                    }
                    else {
                        _DISPLAY_NO_FILE();                              // optionally display that no matching software file is not present on the SD card
                    }
                }
            }
            if (iSearching == 0) {                                       // only perform once
                iSearching = SEARCH_ACTIVE;
                fnInterruptMessage(OWN_TASK, E_DO_SEARCH);               // schedule next
            }
        }
    }
    return (iSearching | iContinue |  iActive);                          // returns true if either the search continues or else firmware uploading is in operation
}

// Perform a single character match, allowing small and capitals to be equal
//
static int fnMatchCharacters(CHAR cChar1, CHAR cChar2)
{
    if (cChar1 != cChar2) {                                              // check for character match
        if ((cChar1 >= 'A') && (cChar1 <= 'Z')) {                        // capital letter
            if (cChar1 != (cChar2 - ('a' - 'A'))) {                      // allow capital letters and small letters to be matched
                return -1;                                               // failed
            }
        }
        else if ((cChar1 >= 'a') && (cChar1 <= 'z')) {                   // capital letter
            if (cChar1 != (cChar2 + ('a' - 'A'))) {                      // allow capital letters and small letters to be matched
                return -1;                                               // failed
            }
        }
        else {
            return -1;                                                   // failed
        }
    }
    return 0;                                                            // match
}

// Perform a wildcard string match
//
static int fnWildcardMatch(CHAR *cString, CHAR *cPattern)
{
    while (*cPattern != 0) {                                             // until the end of the pattern is reached
        switch (*cPattern) {                                             // letter of fixed string to be matched with
        case '?':                                                        // single wildcard - always accept and move over single wildcard
            if (*cString == 0) {                                         // don't match if the string is shorter
                return -1;
            }
            break;
        case '*':                                                        // wildcard (zero, one or multiple characters could be replaced)
            do {
                if (fnWildcardMatch(cString, (cPattern + 1)) == 0) {
                    return 0;                                            // match after wildcard is successful
                }
            } while (*cString++ != 0);
            return -1;                                                   // failed
        default:
            if (fnMatchCharacters(*cPattern, *cString) != 0) {           // case insensitive match
                return -1;                                               // difference found and so failed
            }
            break;
        }
        cString++;
        cPattern++;                                                      // move to next letter
    }
    if (*cString != 0) {                                                 // string too long
        return -1;                                                       // failed
    }
    return 0;                                                            // successfully matched
}
#endif

static void fnJumpToApplication(int iGo)                                 // {3}
{
    static int iMaxWait = 0;

    if ((iGo != 0) || (++iMaxWait >= MAX_WAIT_SD_CARD)) {                // if unconditional or if maximum attempts has been reached
#if defined SERIAL_INTERFACE || defined USB_INTERFACE                    // {4} if the SD card loader is operating in parallel with UART or USB-MSD loader, which wants to retain the mode
        if (RETAIN_LOADER_MODE()) {                                      // if the retain loader input is true (this will usually be unconditionally true when working with START_ON_INTERRUPT) 
    #if defined START_ON_INTERRUPT                                       // {13}
            if (iStartRequested == 0) {
                fnEnableRepeatInterrupt();                               // the user can now press the button to repeat the loading process and unconditionally restart the board
                return;                                                  // remain in the loader mode
            }
    #else
            return;                                                      // remain in loader mode if another loader method is forcing it
    #endif
        }
#endif
        fnJumpToValidApplication(1);                                     // {14}
        return;
    }
    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_CHECK_CARD);    // try again later
}

#if defined FRDM_K64F && defined DEV10
static void fnSynchronisedDelete(unsigned char *ptrDeletedFlash)
{
    if (ptrDeletedFlash == (unsigned char *)FIRMWARE2_DESTINATION_ADDRESS) { // if the first device has been deleted
        ptrDeletedFlash += SPI_DATA_FLASH_0_SIZE;                        // move to the second device and delete it if present
        if (fnSPI_FlashExt_available(1) != 0) {
            fnEraseFlashSector(ptrDeletedFlash, (MAX_FILE_LENGTH)FIRMWARE2_MAX_SIZE); // delete application space
        }
    }
    if (ptrDeletedFlash == (unsigned char *)FIRMWARE2_DESTINATION_ADDRESS + SPI_DATA_FLASH_0_SIZE) { // if the second device has been deleted
        ptrDeletedFlash += SPI_DATA_FLASH_1_SIZE;                        // move to the third device and program it if present
        if (fnSPI_FlashExt_available(2) != 0) {
            fnEraseFlashSector(ptrDeletedFlash, (MAX_FILE_LENGTH)FIRMWARE2_MAX_SIZE); // delete application space
        }
    }
}

static void fnSynchronisedWrite(unsigned char *ptrProgrammedFlash, unsigned char *ptr_ucBuffer, unsigned short usLength)
{
    if (ptrProgrammedFlash < (unsigned char *)(FIRMWARE2_DESTINATION_ADDRESS + SPI_DATA_FLASH_0_SIZE)) { // if the first device has been written to
        ptrProgrammedFlash += SPI_DATA_FLASH_0_SIZE;                     // move to the second device and delete it if present
        if (fnSPI_FlashExt_available(1) != 0) {
            fnWriteBytesFlash(ptrProgrammedFlash, ptr_ucBuffer, usLength); // program the buffer to the destination
        }
    }
    if (ptrProgrammedFlash < (unsigned char *)(FIRMWARE2_DESTINATION_ADDRESS + SPI_DATA_FLASH_0_SIZE + SPI_DATA_FLASH_1_SIZE)) { // if the second device has been written to
        ptrProgrammedFlash += SPI_DATA_FLASH_1_SIZE;                     // move to the second device and program it if present
        if (fnSPI_FlashExt_available(2) != 0) {
            fnWriteBytesFlash(ptrProgrammedFlash, ptr_ucBuffer, usLength); // program the buffer to the destination
        }
    }
}
#endif

// SD card update state-event machine
//
static int fnUpdateSoftware(int iRef)
{
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
    static unsigned long  ulCodeOffset = 0;
    static unsigned long  ulProgrammed = 0;
    static UTFILE         utFile_decrypt = {0};
    #endif
    #if defined MAX_FLASH_ERASE_SIZE                                     // {17}
    static unsigned long  ulDeleteSize = 0;
    #endif
    static unsigned short usCRC[SDCARD_FILE_COUNT] = {0};
    static int            iFlashMismatch[SDCARD_FILE_COUNT] = {0};
    static unsigned char  *ptrInternalFlash[SDCARD_FILE_COUNT] = {0};
    static unsigned long  ulFileLength[SDCARD_FILE_COUNT] = {0};

    UTFILE                *ptr_utFile = &utFile[iRef];
    UPLOAD_HEADER         *ptrFile_header = &file_header[iRef];
    unsigned long         ulBuffer[COPY_BUFFER_SIZE / sizeof(unsigned long)]; // temporary aligned buffer for reading SD card content to
    int                   iNextState = iAppState[iRef];
    int iEvent = 0;
    #if defined ENCRYPTED_CARD_CONTENT
    MAX_FILE_LENGTH       toProgram;
    #endif
    #if defined FLEXFLASH_DATA || defined SPI_FILE_SYSTEM || defined _iMX
    unsigned char         ucCheckBuffer[COPY_BUFFER_SIZE];               // buffer for reading internal flash content to
    #endif

    switch (iNextState) {                                                // present state
    case STATE_START_CHECKING:
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD
        if (iRef == 0) {
            fileObjInfo.ptrLastAddress = 0;                              // {6}
        }
    #endif
        iFlashMismatch[iRef] = 0;
        usCRC[iRef] = 0;
    #if SDCARD_FILE_COUNT > 1
        ptrInternalFlash[iRef] = firmwareDestination[iRef];              // location where new firmware is to be copied to
    #else
        ptrInternalFlash[iRef] = (unsigned char *)_UTASKER_APP_START_;   // {19} location where new firmware is to be copied to
    #endif
    #if defined _iMX
        ptrInternalFlash[iRef] += SIZE_OF_UPLOAD_HEADER;
    #endif
    #if defined SDCARD_SECURE_LOADER
        fnPrepareDecrypt(0);                                             // prepare the decrypt key and prime the initial vector
    #endif
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
        uMemcpy(&utFile_decrypt, ptr_utFile, sizeof(utFile_decrypt));    // copy the file object for decryption use
        ulCodeOffset = CODE_OFFSET;
        while (ulCodeOffset >= ptrFile_header->ulCodeLength) {
            ulCodeOffset -= ptrFile_header->ulCodeLength;
        }
        utSeek(&utFile_decrypt, (SIZE_OF_UPLOAD_HEADER + ulCodeOffset), UTFAT_SEEK_SET); // {15} move to the start of the encrypted code
        fnDecrypt(0, 0);                                                 // reset decryptor
    #endif
        iNextState = STATE_CHECKING;
        // Fall-through intentional
        //
    case STATE_CHECKING:                                                 // check the CRC of the file on the SD card
        utReadFile(ptr_utFile, ulBuffer, sizeof(ulBuffer));              // read a single buffer from the file on the SD card
        usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // calculate the CRC of complete file content
        if (iFlashMismatch[iRef] == 0) {                                 // if the code still matches
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
            utReadFile(&utFile_decrypt, ulBuffer, sizeof(ulBuffer));     // read a single buffer from the file on the SD card
            if (utFile_decrypt.ulFilePosition >= utFile_decrypt.ulFileSize) { // end of file reached
                unsigned char *ptrBuffer = (unsigned char *)ulBuffer;
                unsigned short usRemaining = (ptr_utFile->usLastReadWriteLength - utFile_decrypt.usLastReadWriteLength);
                utSeek(&utFile_decrypt, SIZE_OF_UPLOAD_HEADER, UTFAT_SEEK_SET); // move back to start of code in file
                utReadFile(&utFile_decrypt, &ptrBuffer[utFile_decrypt.usLastReadWriteLength], usRemaining); // read a single buffer from the file on the SD card
            }
            fnDecrypt((unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // decrypt the buffer before comparing
    #endif
    #if defined SDCARD_SECURE_LOADER && !defined _iMX
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)ulBuffer, (unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // decrypt the buffer content
    #endif
    #if defined FLEXFLASH_DATA || defined SPI_FILE_SYSTEM || defined _iMX// {16} if there is a possibility of working in data flash we need to collect a buffer first
            fnGetParsFile(ptrInternalFlash[iRef], ucCheckBuffer, ptr_utFile->usLastReadWriteLength);
            if (uMemcmp(ucCheckBuffer, ulBuffer, ptr_utFile->usLastReadWriteLength) != 0) { // check whether the code is different
                iFlashMismatch[iRef] = 1;                                // the code is different so needs to be updated
            }
            else {
                ptrInternalFlash[iRef] += ptr_utFile->usLastReadWriteLength;
            }
    #else
            if (uMemcmp(fnGetFlashAdd(ptrInternalFlash[iRef]), ulBuffer, ptr_utFile->usLastReadWriteLength) != 0) { // check whether the code is different
                iFlashMismatch[iRef] = 1;                                // the code is different so needs to be updated
            }
            else {
                ptrInternalFlash[iRef] += ptr_utFile->usLastReadWriteLength;
            }
    #endif
        }
        if (ptr_utFile->usLastReadWriteLength != sizeof(ulBuffer)) {     // end of file reached
            iNextState = STATE_CHECK_SECRET_KEY;                         // move to the next state
        }
        iEvent = 1;                                                      // schedule next
        break;

    case STATE_CHECK_SECRET_KEY:
        usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ucSecretKey, sizeof(ucSecretKey)); // add the secret key
        if (usCRC[iRef] == ptrFile_header->usCRC) {                      // content is valid
            _DISPLAY_VALID_CONTENT();                                    // optionally display that the content is valid
            if (iFlashMismatch[iRef] != 0) {                             // valid new code which needs to be programmed
    #if SDCARD_FILE_COUNT > 1
                ptrInternalFlash[iRef] = firmwareDestination[iRef];      // location where new firmware is to be copied to
        #if UTASKER_APP_START != _UTASKER_APP_START_
                if (iRef == 0) {
                    ptrInternalFlash[iRef] -= (_UTASKER_APP_START_ - UTASKER_APP_START);
                }
        #endif
    #else
                ptrInternalFlash[iRef] = (unsigned char *)UTASKER_APP_START;
    #endif
    #if defined MAX_FLASH_ERASE_SIZE                                     // {17}
                unsigned long ulThisDeleteSize;
        #if defined ERASE_NEEDED_FLASH_ONLY                              // {18}
                ulDeleteSize = ptrFile_header->ulCodeLength;             // delete only the size required to accept the new code 
        #else
            #if SDCARD_FILE_COUNT > 1
                ulDeleteSize = (firmwareEnd[iRef] - ptrInternalFlash[iRef]);
            #else
                ulDeleteSize = (UTASKER_APP_END - ptrInternalFlash[iRef]);
            #endif
        #endif
                ulThisDeleteSize = ulDeleteSize;
                if (ulThisDeleteSize > MAX_FLASH_ERASE_SIZE) {
                    ulThisDeleteSize = MAX_FLASH_ERASE_SIZE;
                }
                fnEraseFlashSector(ptrInternalFlash[iRef], ulThisDeleteSize);  // delete application space
                ulDeleteSize -= ulThisDeleteSize;
                ptrInternalFlash[iRef] += ulThisDeleteSize;
    #else
        #if SDCARD_FILE_COUNT > 1
                fnEraseFlashSector(ptrInternalFlash[iRef], (MAX_FILE_LENGTH)(firmwareEnd[iRef] - ptrInternalFlash[iRef])); // delete application space
        #else
                fnEraseFlashSector(ptrInternalFlash[iRef], (MAX_FILE_LENGTH)(UTASKER_APP_END - ptrInternalFlash[iRef])); // delete application space
        #endif
    #endif
                iNextState = STATE_DELETING_FLASH;                       // move to next state
                iEvent = 1;                                              // schedule next
            }
            else {
                _DISPLAY_SW_OK();                                        // optionally display that the programmed application is already up-to-date
    #if defined DELETE_SDCARD_FILE_AFTER_UPDATE
        #if defined WILDCARD_FILES
                utDeleteFile((const CHAR *)cSWfile[iRef], ptr_utDirectory[iRef]); // delete the file from the disk since it is the same as the loaded software
        #else
                utDeleteFile(NEW_SOFTWARE_FILE, ptr_utDirectory[0]);     // delete the file from the disk since it is the same as the loaded software
        #endif
    #endif
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1.5), T_JUMP_TO_APP); // start the application after a delay
                iNextState = STATE_ACTIVE;
            }
        }
        else {
            _DISPLAY_INVALID_CONTENT();                                  // optionally display that the content is not valid
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1), T_JUMP_TO_APP); // {3} start the existing application after a delay
            iNextState = STATE_ACTIVE;
        }
        break;

    case STATE_DELETING_FLASH:                                           // flash deleted
    #if defined MAX_FLASH_ERASE_SIZE                                     // {17}
        if (ulDeleteSize != 0) {
            unsigned long ulThisDeleteSize = ulDeleteSize;
            if (ulThisDeleteSize > MAX_FLASH_ERASE_SIZE) {
                ulThisDeleteSize = MAX_FLASH_ERASE_SIZE;
            }
            fnEraseFlashSector(ptrInternalFlash[iRef], ulThisDeleteSize);// delete application space
            ulDeleteSize -= ulThisDeleteSize;
            ptrInternalFlash[iRef] += ulThisDeleteSize;
            iEvent = 1;                                                  // schedule next
            break;
        }
    #endif
    #if SDCARD_FILE_COUNT > 1
        #if defined FRDM_K64F && defined DEV10
        if (iRef == 1) {
            // When the first device has been deleted we delete additional device before continuing
            //
            fnSynchronisedDelete(ptrInternalFlash[iRef]);
        }
        #endif
        ptrInternalFlash[iRef] = firmwareDestination[iRef];
    #else
        ptrInternalFlash[iRef] = (unsigned char *)_UTASKER_APP_START_;
    #endif
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
        ulProgrammed = sizeof(ucCodeStart[iRef]);
        utSeek(ptr_utFile, (SIZE_OF_UPLOAD_HEADER + ulCodeOffset), UTFAT_SEEK_SET); // {15} return to the start of the encrypted code
        utReadFile(ptr_utFile, ucCodeStart[iRef], sizeof(ucCodeStart[iRef])); // store code start for programming as final step
        fnDecrypt(0, 0);                                                 // reset decryptor
        fnDecrypt(ucCodeStart[iRef], sizeof(ucCodeStart[iRef]));
    #else
        #if defined _iMX
        utSeek(ptr_utFile, 0, UTFAT_SEEK_SET);                           // return to the start of the file (including header)
        #else
        utSeek(ptr_utFile, SIZE_OF_UPLOAD_HEADER, UTFAT_SEEK_SET);       // return to the start of the file after the header
        #endif
        #if defined SDCARD_SECURE_LOADER
        utReadFile(ptr_utFile, ulBuffer, sizeof(ucCodeStart[iRef]));     // store code start for programming as final step
        #else
        utReadFile(ptr_utFile, ucCodeStart[iRef], sizeof(ucCodeStart[iRef])); // store code start for programming as final step
        #endif
    #endif
        ptrInternalFlash[iRef] += sizeof(ucCodeStart[iRef]);
        iNextState = STATE_PROGRAMMING;                                  // move to next state
    #if defined SDCARD_SECURE_LOADER
        fnPrepareDecrypt(0);                                             // prepare the decrypt key and prime the initial vector
        fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)ulBuffer, (unsigned char *)ulBuffer, sizeof(ucCodeStart[iRef])); // decrypt the buffer content
        uMemcpy(ucCodeStart[iRef], ulBuffer, sizeof(ucCodeStart[iRef]));
    #endif
        // Fall-through intentionally
        //
    case STATE_PROGRAMMING:                                              // programming from SD card file to destination application space
        utReadFile(ptr_utFile, ulBuffer, sizeof(ulBuffer));              // read buffer from file
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
        fnDecrypt((unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // decrypt the buffer before programming
        ulProgrammed += ptr_utFile->usLastReadWriteLength;
        if (ulProgrammed > ptrFile_header->ulCodeLength) {
            toProgram = (ptr_utFile->usLastReadWriteLength - (ulProgrammed - ptrFile_header->ulCodeLength)); // final content length
        }
        else {
            toProgram = ptr_utFile->usLastReadWriteLength;
        }
        fnWriteBytesFlash(ptrInternalFlash[iRef], (unsigned char *)ulBuffer, toProgram); // program to flash
        ptrInternalFlash[iRef] += ptr_utFile->usLastReadWriteLength;
        if (ptr_utFile->ulFilePosition >= ptr_utFile->ulFileSize) {      // end of file reached
            utSeek(ptr_utFile, SIZE_OF_UPLOAD_HEADER, UTFAT_SEEK_SET);   // move back to start of code in file
            ptr_utFile->usLastReadWriteLength = sizeof(ulBuffer);        // avoid terminating
        }
        if (ulProgrammed >= ptrFile_header->ulCodeLength) {
            ptr_utFile->usLastReadWriteLength = 0;
        }
    #else
        #if defined SDCARD_SECURE_LOADER
        fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)ulBuffer, (unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // decrypt the buffer content
        #endif
        fnWriteBytesFlash(ptrInternalFlash[iRef], (unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength); // program the buffer to the destination
        #if defined FRDM_K64F && defined DEV10
        if (iRef == 1) {
            // When the first device has been programmed we write the same to other existing device
            //
            fnSynchronisedWrite(ptrInternalFlash[iRef], (unsigned char *)ulBuffer, ptr_utFile->usLastReadWriteLength);
        }
        #endif
        ptrInternalFlash[iRef] += ptr_utFile->usLastReadWriteLength;
    #endif
        if (ptr_utFile->usLastReadWriteLength != sizeof(ulBuffer)) {     // end of file reached
    #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0                     // {2}
            fnWriteBytesFlash(ptrInternalFlash[iRef], 0, 0);             // close any outstanding FLASH buffer from end of the file
    #endif
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD
            if (iRef == 0) {
                fileObjInfo.ptrLastAddress = ptrInternalFlash[iRef];     // save information about the length of data written
            }
    #endif
    #if SDCARD_FILE_COUNT > 1
            ptrInternalFlash[iRef] = firmwareDestination[iRef];
    #else
            ptrInternalFlash[iRef] = (unsigned char *)_UTASKER_APP_START_;
    #endif
            fnWriteBytesFlash(ptrInternalFlash[iRef], ucCodeStart[iRef], sizeof(ucCodeStart[iRef])); // program start of code to Flash
    #if defined FRDM_K64F && defined DEV10
            if (iRef == 1) {
                // When the first device has been programmed we write the same to other existing device
                //
                fnSynchronisedWrite(ptrInternalFlash[iRef], ucCodeStart[iRef], sizeof(ucCodeStart[iRef]));
                // Note that all devices have now been programed but we only check that the first one was programemd without errors
                // an the backup will only be deleted once the first device is accurate
                // - this means that the content of the first device is "guaranteed" to be correct but the synchronised devices not
                // - the application can add its own check that synchronised devices exactly match the first device if required
                //
            }
    #endif
    #if defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0                     // {2}
            fnWriteBytesFlash(ptrInternalFlash[iRef], 0, 0);             // close any outstanding FLASH buffer from write to start of code
    #endif
            usCRC[iRef] = 0;
            iNextState = STATE_VERIFYING;                                // move to next state
            ulFileLength[iRef] = (ptr_utFile->ulFileSize - SIZE_OF_UPLOAD_HEADER);
    #if defined _iMX
            ptrInternalFlash[iRef] += SIZE_OF_UPLOAD_HEADER;
    #endif
    #if defined SDCARD_SECURE_LOADER
            fnPrepareDecrypt(1);                                         // prepare the encrypt key and prime the initial vector
    #endif
        }
        iEvent = 1;                                                      // schedule next
        break;

    case STATE_VERIFYING:                                                // verify the CRC of the new application in flash
        if (ulFileLength[iRef] >= CRC_BLOCK_SIZE) {
    #if defined FLEXFLASH_DATA || defined SPI_FILE_SYSTEM || defined _iMX// {16} if there is a possibility of working in data flash we need to collect a buffer first
        #if defined SDCARD_SECURE_LOADER
            fnGetParsFile(ptrInternalFlash[iRef], ulBuffer, CRC_BLOCK_SIZE);
            fnAES_Cipher(AES_COMMAND_AES_ENCRYPT, (const unsigned char *)fnGetFlashAdd(ptrInternalFlash[iRef]), (unsigned char *)ulBuffer, CRC_BLOCK_SIZE); // encrypt the flash content
            usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ulBuffer, CRC_BLOCK_SIZE);
        #else
            fnGetParsFile(ptrInternalFlash[iRef], ucCheckBuffer, CRC_BLOCK_SIZE);
            usCRC[iRef] = fnCRC16(usCRC[iRef], ucCheckBuffer, CRC_BLOCK_SIZE);
        #endif
    #else
        #if defined SDCARD_SECURE_LOADER
            fnAES_Cipher(AES_COMMAND_AES_ENCRYPT, (const unsigned char *)fnGetFlashAdd(ptrInternalFlash[iRef]), (unsigned char *)ulBuffer, CRC_BLOCK_SIZE); // encrypt the flash content
            usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ulBuffer, CRC_BLOCK_SIZE);
        #else
            usCRC[iRef] = fnCRC16(usCRC[iRef], fnGetFlashAdd(ptrInternalFlash[iRef]), CRC_BLOCK_SIZE);
        #endif
    #endif
            ulFileLength[iRef] -= CRC_BLOCK_SIZE;
            ptrInternalFlash[iRef] += CRC_BLOCK_SIZE;
            iEvent = 1;                                                  // schedule next
        }
        else {
    #if defined FLEXFLASH_DATA || defined SPI_FILE_SYSTEM || defined _iMX// {16} if there is a possibility of working in data flash we need to collect a buffer first
        #if defined SDCARD_SECURE_LOADER
            fnGetParsFile(ptrInternalFlash[iRef], ulBuffer, CRC_BLOCK_SIZE);
            fnAES_Cipher(AES_COMMAND_AES_ENCRYPT, (const unsigned char *)fnGetFlashAdd(ptrInternalFlash[iRef]), (unsigned char *)ulBuffer, ulFileLength[iRef]); // encrypt the flash content
            usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ulBuffer, ulFileLength[iRef]);
        #else
            fnGetParsFile(ptrInternalFlash[iRef], ucCheckBuffer, ulFileLength[iRef]);
            usCRC[iRef] = fnCRC16(usCRC[iRef], ucCheckBuffer, ulFileLength[iRef]);
        #endif
    #else
        #if defined SDCARD_SECURE_LOADER
            fnAES_Cipher(AES_COMMAND_AES_ENCRYPT, (const unsigned char *)fnGetFlashAdd(ptrInternalFlash[iRef]), (unsigned char *)ulBuffer, ulFileLength[iRef]); // encrypt the flash content
            usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ulBuffer, ulFileLength[iRef]);
        #else
            usCRC[iRef] = fnCRC16(usCRC[iRef], fnGetFlashAdd(ptrInternalFlash[iRef]), ulFileLength[iRef]); // last block
        #endif
    #endif
            usCRC[iRef] = fnCRC16(usCRC[iRef], (unsigned char *)ucSecretKey, sizeof(ucSecretKey)); // add the secret key
    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
            if (usCRC[iRef] == ptrFile_header->usRAWCRC)
    #else
            if (usCRC[iRef] == ptrFile_header->usCRC)
    #endif
            {                                                            // new code has been verified as being correct
    #if defined USB_INTERFACE && defined USB_MSD_DEVICE_LOADER && !defined USE_USB_MSD // {6}{11}
                if (iRef == 0) {
                    fnAddSREC_file(&fileObjInfo);                        // set file entry compatible with USB-MSD
                }
    #endif
    #if defined DELETE_SDCARD_FILE_AFTER_UPDATE
        #if defined WILDCARD_FILES
                utDeleteFile((const CHAR *)cSWfile[iRef], ptr_utDirectory[iRef]); // delete the file from the disk since it has been successfully loaded
        #else
                utDeleteFile(NEW_SOFTWARE_FILE, ptr_utDirectory[0]);     // delete the file from the disk since it has been successfully loaded
        #endif
    #endif
    #if defined START_ON_INTERRUPT
                iStartRequested = 1;                                     // {13} allow the application to start after an update
    #endif
                _DISPLAY_SW_UPDATED();                                   // optionally display that the code was successfully programmed
    #if defined SPECIAL_VERSION_SDCARD
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1.5), T_DELAYED_JUMP_TO_APP); // start the application after a short delay
    #else
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 1.5), T_JUMP_TO_APP); // start the application after a short delay
   #endif
            }
            else {
                _DISPLAY_ERROR();                                        // optionally display that there was an error with the image loaded to flash
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(SEC * 4), T_CHECK_CARD); // try again or allow application to start after a further check
            }
            iNextState = STATE_ACTIVE;
        }
        break;
    }
    iAppState[iRef] = iNextState;
    return iEvent;
}

#if (defined (_WINDOWS) || defined (_LITTLE_ENDIAN)) && !defined _iMX
extern void fnHeaderToLittleEndian(UPLOAD_HEADER *file_header)
{
    unsigned short usShort;
    unsigned long  ulLong;

    #if defined ENCRYPTED_CARD_CONTENT                                   // {9}
    usShort = (file_header->usRAWCRC >> 8);
    file_header->usRAWCRC <<= 8;
    file_header->usRAWCRC |= usShort;
    #endif

    usShort = (file_header->usCRC >> 8);
    file_header->usCRC <<= 8;
    file_header->usCRC |= usShort;

    usShort = (file_header->usMagicNumber >> 8);
    file_header->usMagicNumber <<= 8;
    file_header->usMagicNumber |= usShort;

    ulLong = (file_header->ulCodeLength >> 24);
    ulLong |= (((file_header->ulCodeLength >> 16) & 0xff) << 8);
    ulLong |= (((file_header->ulCodeLength >> 8) & 0xff) << 16);
    ulLong |= (((file_header->ulCodeLength) & 0xff) << 24);
    file_header->ulCodeLength = ulLong;
}
#endif

#if defined ENCRYPTED_CARD_CONTENT                                       // {9}
// Decode a block of data
//
static unsigned short fnDecrypt(unsigned char *ptrData, unsigned short usBlockSize)
{
    static int iCycleLength = 0;
    static unsigned short usPRNG = 0;
    if (ptrData == 0) {
        iCycleLength = 0;
        usPRNG = KEY_PRIME;
        return 0;
    }

    while (usBlockSize != 0) {
        while (iCycleLength < sizeof(ucDecrypt)) {                       // cycle through second key
            usPRNG = fnCRC16(usPRNG, (unsigned char *)&ucDecrypt[iCycleLength++], 1); // generate next pseudo random pattern from decrypt pattern
            *ptrData = *ptrData ^ usPRNG;                                // decrypt each byte
            ptrData++;
            usBlockSize--;
            if (!usBlockSize) {
                break;
            }
        }
        if (iCycleLength >= sizeof(ucDecrypt)) {
            iCycleLength = 0;
        }
    }
    return usPRNG;
}
#endif

#if defined START_ON_INTERRUPT                                           // {13}
// Interrupt handler
//
static void fnRepeatCheck(void)                                          // button has been pressed
{
    fnInterruptMessage(OWN_TASK, E_RECHECK_SW);                          // cause the SD card checking process to be repeated
}

// Configure an interrupt on an input to detect a button being pressed
//
static void fnEnableRepeatInterrupt(void)
{
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    interrupt_setup.int_type       = PORT_INTERRUPT;                     // identifier to configure port interrupt
    interrupt_setup.int_handler    = fnRepeatCheck;                      // handling function
    interrupt_setup.int_priority   = 1;                                  // interrupt priority level (low priority since not critical)
    interrupt_setup.int_port       = BUTTON_PORT;                        // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = BUTTON_INPUT;                       // the interrupt input
    interrupt_setup.int_port_sense = BUTTON_INTERRUPT_EDGE;              // interrupt edge sensitivity
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
}
#endif
#endif

