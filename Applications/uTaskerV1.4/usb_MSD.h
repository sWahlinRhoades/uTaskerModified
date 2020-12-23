/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_MSD.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    30.08.2020 Extracted MSD device and host code from usb_application.c to this file

*/

#if defined MSD_DEFINES
    #if defined USE_USB_MSD
        #define NUMBER_OF_PARTITIONS        DISK_COUNT
    #endif

    #if defined _LITTLE_ENDIAN || defined _WINDOWS
        #define USBC_SIGNATURE   0x43425355                              // 'U' 'S' 'B' 'C'
        #define USBS_SIGNATURE   0x53425355                              // 'U' 'S' 'B' 'S'
    #else
        #define USBC_SIGNATURE   0x55534243                              // 'U' 'S' 'B' 'C'
        #define USBS_SIGNATURE   0x55534253                              // 'U' 'S' 'B' 'S'
    #endif

    #if defined USB_MSD_HOST
        #define STATUS_TRANSPORT            0x100
    #endif
#endif

#if defined MSD_HOST_STRUCT
#endif

#if defined MSD_HOST_CONST
    static const USB_HOST_DESCRIPTOR get_max_lum = {
        (STANDARD_DEVICE_TO_HOST | REQUEST_INTERFACE_CLASS),             // 0xa1 request class specific interface
        GET_MAX_LUN,                                                     // 0xfe
        {LITTLE_SHORT_WORD_BYTES(0)},
        {LITTLE_SHORT_WORD_BYTES(0)},
        {LITTLE_SHORT_WORD_BYTES(1)},
    };
#endif

#if defined MSD_LOCAL_VARIABLES
    #if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
    static unsigned char  ucDeviceLUN = 0;
    static int            iUSB_MSD_OpCode = 0;
    static unsigned long  ulTag =0 ;
    static unsigned long  ulLBA = 0;
    static unsigned long  ulBlockByteCount = 0;
    static unsigned short usBlockCount = 0;
    static unsigned char  ucRequestCount = 0;
    static unsigned char  ucMSDBulkInEndpoint = 0;                       // the endpoint that is to be used as MSD bulk IN endpoint
    static unsigned short usMSDBulkInEndpointSize = 0;
    static unsigned char  ucMSDBulkOutEndpoint = 0;                      // the endpoint that is to be used as MSD bulk OUT endpoint
    static unsigned short usMSDBulkOutEndpointSize = 0;
    static unsigned char  ucLocalMSDEvent[USB_CONTROLLERS] = {0};
    #endif
#endif


#if defined MSD_FUNCTIONS
#if defined USE_USB_MSD                                                  // device mode
static void fnResetMSD_DeviceVariables(int iInterface)
{
    ulWriteBlock = 0;
    ulReadBlock = 0;
    iContent = 0;
    iWriteInProgress = 0;                                                // {46}
}
#endif

#if defined USB_DEVICE_SUPPORT && defined USE_USB_MSD                    // usb-msd device mode
// Handle potential USB-MSD data reception from host
//
static void fnHandleMSD_host_reception(QUEUE_HANDLE USBPortID_MSD, unsigned char ucInputMessage[LARGE_MESSAGE])
{
    while (fnMsgs(USBPortID_MSD) != 0) {
        QUEUE_TRANSFER Length;
        if (ulWriteBlock != 0) {                                         // write data expected
            static unsigned char ucBuffer[512];                          // intermediate buffer to collect each sector content
            Length = fnRead(USBPortID_MSD, &ucBuffer[iContent], (QUEUE_TRANSFER)(512 - iContent)); // read the content directly to the intermediate buffer
            ulWriteBlock -= Length;
            iContent += Length;
            if (iContent >= 512) {                                       // input buffer is complete
    #if defined FAT_EMULATION
                if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & DISK_FAT_EMULATION) == 0) { // ignore writes to emulated FAT
    #endif
                    fnWriteSector(ucActiveLUN, ucBuffer, ulLogicalBlockAdr++); // commit the buffer content to the media
    #if defined FAT_EMULATION
                }
    #endif
                iContent = 0;                                            // reset intermediate buffer
                if (ulWriteBlock != 0) {                                 // block not complete
                    uTaskerStateChange(OWN_TASK, UTASKER_ACTIVATE);      // {17} yield after a write when further data is expected but schedule again immediately to read any remaining queue content
                    return;
                }
            }
            if (ulWriteBlock != 0) {                                     // more data expected
                continue;
            }                                                            // allow CSW to be sent after a complete transfer has completed
            iWriteInProgress = 0;                                        // {46} allow further USB-MSD commands to be interpreted
        }
        else {
          //unsigned long ulTransferLength;
            USB_MASS_STORAGE_CBW *ptrCBW;
            Length = fnRead(USBPortID_MSD, ucInputMessage, LARGE_MESSAGE); // read the content
            ptrCBW = (USB_MASS_STORAGE_CBW *)ucInputMessage;
          //ulTransferLength = (ptrCBW->dCBWDataTransferLength[0] | (ptrCBW->dCBWDataTransferLength[1] << 8) | (ptrCBW->dCBWDataTransferLength[2] << 16) | (ptrCBW->dCBWDataTransferLength[3] << 24));
            uMemcpy(&csw.dCBWTag, ptrCBW->dCBWTag, sizeof(csw.dCBWTag)); // save the tag
            csw.dCSWDataResidue[0] = 0;
            csw.dCSWDataResidue[1] = 0;
            csw.dCSWDataResidue[2] = 0;
            csw.dCSWDataResidue[3] = 0;
            csw.bCSWStatus = CSW_STATUS_COMMAND_PASSED;                  // set for success
            fnFlush(USBPortID_MSD, FLUSH_TX);                            // always flush the tx buffer to ensure message alignment in buffer before responding
            ucActiveLUN = ptrCBW->dCBWLUN;                               // the logical unit number to use
            if ((ptrCBW->dmCBWFlags & CBW_IN_FLAG) != 0) {
                switch (ptrCBW->CBWCB[CBW_OperationCode]) {
                case UFI_FORMAT_UNIT:
                    break;
                case UFI_INQUIRY:
                    {
                        CBW_INQUIRY_COMMAND *ptrInquiry = (CBW_INQUIRY_COMMAND *)ptrCBW->CBWCB;
                        unsigned char ucLogicalUnitNumber = (ptrInquiry->ucLogicalUnitNumber_EVPD >> 5); // the logical unit number being requested
                        QUEUE_TRANSFER inquiry_length;
                        unsigned char *ptrInquiryData = fnGetPartition(ucLogicalUnitNumber, &inquiry_length);
                        if (ptrInquiryData == 0) {                       // partition doesn't exist
                            csw.bCSWStatus = CSW_STATUS_COMMAND_FAILED;
                        }
                        else {
                            fnWrite(USBPortID_MSD, ptrInquiryData, inquiry_length); // respond to the request and then return 
                        }
                    }
                    break;
                case UFI_MODE_SENSE_6:                                   // {15}
                    {
                        MODE_PARAMETER_6 SelectDataWP;
                        uMemcpy(&SelectDataWP, &SelectData, sizeof(SelectData));
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & WRITE_PROTECTED_SD_CARD) != 0) {
                            SelectDataWP.ucWP_DPOFUA = PAR6_WRITE_PROTECTED; // the medium is write protected
                        }
                        fnWrite(USBPortID_MSD, (unsigned char *)&SelectDataWP, sizeof(SelectData)); // respond to the request and then return 
                    }
                    break;
                case UFI_START_STOP:
                case UFI_MODE_SELECT:
              //case UFI_MODE_SENSE:
                case UFI_PRE_ALLOW_REMOVAL:
                    break;
                case UFI_READ_10:
                case UFI_READ_12:
                    {                                
                        CBW_READ_10 *ptrRead = (CBW_READ_10 *)ptrCBW->CBWCB;
                      //ulLogicalBlockAdr = ((ptrRead->ucLogicalBlockAddress[0] << 24) | (ptrRead->ucLogicalBlockAddress[1] << 16) | (ptrRead->ucLogicalBlockAddress[2] << 8) | ptrRead->ucLogicalBlockAddress[3]);
                        if (ptrRead->ucOperationCode == UFI_READ_12) {
                            CBW_READ_12 *ptrRead = (CBW_READ_12 *)ptrCBW->CBWCB;
                            ulReadBlock = ((ptrRead->ucTransferLength[0] << 24) | (ptrRead->ucTransferLength[1] << 16) | (ptrRead->ucTransferLength[2] << 8) | ptrRead->ucTransferLength[3]);
                        }
                        else {
                            ulReadBlock = ((ptrRead->ucTransferLength[0] << 8) | ptrRead->ucTransferLength[1]); // the total number of blocks to be returned
                        }
                        fnContinueMedia();
                    }
                    continue;                                            // the transfer has not completed so don't send termination stage yet
                case UFI_READ_CAPACITY:
                    {
                        CBW_READ_CAPACITY_DATA formatData;
                        unsigned long ulLastSector = (ptrDiskInfo[ucActiveLUN]->ulSD_sectors - 1);
                        formatData.ucBlockLengthInBytes[0] = 0;
                        formatData.ucBlockLengthInBytes[1] = 0;
                        formatData.ucBlockLengthInBytes[2] = (unsigned char)(ptrDiskInfo[ucActiveLUN]->utFAT.usBytesPerSector >> 8);
                        formatData.ucBlockLengthInBytes[3] = (unsigned char)(ptrDiskInfo[ucActiveLUN]->utFAT.usBytesPerSector);
                        formatData.ucLastLogicalBlockAddress[0] = (unsigned char)(ulLastSector >> 24);
                        formatData.ucLastLogicalBlockAddress[1] = (unsigned char)(ulLastSector >> 16);
                        formatData.ucLastLogicalBlockAddress[2] = (unsigned char)(ulLastSector >> 8);
                        formatData.ucLastLogicalBlockAddress[3] = (unsigned char)(ulLastSector);
                        fnWrite(USBPortID_MSD, (unsigned char *)&formatData, sizeof(formatData));
                    }
                    break;
                case UFI_READ_FORMAT_CAPACITY:
                    {
                        CBW_FORMAT_CAPACITIES *ptrCapacities = (CBW_FORMAT_CAPACITIES *)ptrCBW->CBWCB;
                        unsigned short usLengthAccepted = ((ptrCapacities->ucAllocationLength[0] << 8) | (ptrCapacities->ucAllocationLength[1]));
                        CBW_CAPACITY_LIST mediaCapacity;
                        uMemcpy(&mediaCapacity, &formatCapacityNoMedia, sizeof(CBW_CAPACITY_LIST)); // assume no disk
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) != 0) { // {16}
                            if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & DISK_FORMATTED) != 0) {
                                mediaCapacity.capacityDescriptor.ucDescriptorCode = DESC_CODE_FORMATTED_MEDIA;
                            }
                            else {
                                mediaCapacity.capacityDescriptor.ucDescriptorCode = DESC_CODE_UNFORMATTED_MEDIA;
                            }
                            mediaCapacity.capacityDescriptor.ucNumberOfBlocks[3] = (unsigned char)ptrDiskInfo[ucActiveLUN]->ulSD_sectors;
                            mediaCapacity.capacityDescriptor.ucNumberOfBlocks[2] = (unsigned char)(ptrDiskInfo[ucActiveLUN]->ulSD_sectors >> 8);
                            mediaCapacity.capacityDescriptor.ucNumberOfBlocks[1] = (unsigned char)(ptrDiskInfo[ucActiveLUN]->ulSD_sectors >> 16);
                            mediaCapacity.capacityDescriptor.ucNumberOfBlocks[0] = (unsigned char)(ptrDiskInfo[ucActiveLUN]->ulSD_sectors >> 24);
                        }                                
                        if (usLengthAccepted > sizeof(CBW_CAPACITY_LIST)) {
                            usLengthAccepted = sizeof(CBW_CAPACITY_LIST);
                        }
                        fnWrite(USBPortID_MSD, (unsigned char *)&mediaCapacity, usLengthAccepted);
                    }
                    break;
                case UFI_REQUEST_SENSE:
                    {
                        CBW_RETURN_SENSE_DATA present_sense_data;
                        uMemcpy(&present_sense_data, &sense_data_OK, sizeof(sense_data_OK));
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) == 0) { // if the disk is not present
                            present_sense_data.ucValid_ErrorCode = (CURRENT_ERRORS); // set that the disk is presently in a non-usable state
                            present_sense_data.ucSenseKey = SENSE_NOT_READY;
                            present_sense_data.ucAdditionalSenseCode = DESC_MEDIUM_NOT_PRESENT;
                        }
                        fnWrite(USBPortID_MSD, (unsigned char *)&present_sense_data, sizeof(present_sense_data));
                    }
                    break;
                case UFI_REZERO_UNIT:
                case UFI_SEEK_10:
                case UFI_SEND_DIAGNOSTIC:
                case UFI_VERIFY:
                case UFI_WRITE_AND_VERIFY:
                default:
                    break;
                }
            }
            else {                                                       // OUT types
                switch (ptrCBW->CBWCB[CBW_OperationCode]) {
                case UFI_TEST_UNIT_READY:
                    if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) == 0) { // if the partition is not ready
                        csw.bCSWStatus = CSW_STATUS_COMMAND_FAILED;
                    }
                    break;
                case UFI_WRITE_10:
                case UFI_WRITE_12:
                    {
                        CBW_WRITE_10 *ptrWrite = (CBW_WRITE_10 *)ptrCBW->CBWCB;
                      //ulLogicalBlockAdr = ((ptrWrite->ucLogicalBlockAddress[0] << 24) | (ptrWrite->ucLogicalBlockAddress[1] << 16) | (ptrWrite->ucLogicalBlockAddress[2] << 8) | ptrWrite->ucLogicalBlockAddress[3]);
                        if (ptrWrite->ucOperationCode == UFI_WRITE_12) {
                            CBW_WRITE_12 *ptrWrite = (CBW_WRITE_12 *)ptrCBW->CBWCB;
                            ulWriteBlock = ((ptrWrite->ucTransferLength[0] << 24) | (ptrWrite->ucTransferLength[1] << 16) | (ptrWrite->ucTransferLength[2] << 8) | ptrWrite->ucTransferLength[3]);
                        }
                        else {
                            ulWriteBlock = ((ptrWrite->ucTransferLength[0] << 8) | ptrWrite->ucTransferLength[1]); // the total number of blocks to be returned
                        }
    #if defined UTFAT_MULTIPLE_BLOCK_WRITE
                        fnPrepareBlockWrite(ucActiveLUN, ulWriteBlock, 1); // {31} since we know that there will be a write to one or more blocks we can prepare the disk so that it can write faster
    #endif
                        ulWriteBlock *= 512;                             // convert to byte count
                    }
                    continue;                                            // data will follow so don't reply with CSW stage yet
                }
            }
        }
        fnWrite(USBPortID_MSD, (unsigned char *)&csw, sizeof(csw));      // close with CSW stage
    }
}
#endif

#if defined USE_USB_MSD
// The output buffer has space so continue with data transfer or terminate
//
static void fnContinueMedia(void)
{
    QUEUE_TRANSFER buffer_space = fnWrite(USBPortID_MSD, 0, 0);          // check whether there is space available in the USB output buffer to queue
    while ((ulReadBlock != 0) && (buffer_space >= 512)) {                // send as many blocks as possible as long as the output buffer has space
        if (UTFAT_SUCCESS == fnReadSector(ucActiveLUN, 0, ulLogicalBlockAdr)) { // read a sector from the disk
            fnWrite(USBPortID_MSD, ptrDiskInfo[ucActiveLUN]->ptrSectorData, 512); // send it to the USB output queue
            buffer_space -= 512;
            ulLogicalBlockAdr++;
        }
        else {
            // Error reading media
            //
        }
        ulReadBlock--;                                                   // one less block to be sent
    }
    if (ulReadBlock == 0) {                                              // all blocks have been read and put to the output queue
        if (buffer_space >= sizeof(csw)) {
            fnWrite(USBPortID_MSD, (unsigned char *)&csw, sizeof(csw));  // close with CSW stage
            return;
        }
    }
    if (buffer_space < 512) {                                            // if there is too little buffer space for a complete sector
        fnDriver(USBPortID_MSD, MODIFY_WAKEUP, (MODIFY_TX | TASK_USB | (USB_MSD_DEVICE_TX_FREE_REF << 8)));  // when there is room in the output buffer the task will be woken
    }
}

// MSC data endpoint callback (allows peeking at reception content directly in its rx interrupt)
//
static int mass_storage_callback(unsigned char *ptrData, unsigned short length, int iType, int iChannel)
{
    if (iWriteInProgress != 0) {                                         // {46} data expected
        return TRANSPARENT_CALLBACK;                                     // handle data in task
    }
    if (uMemcmp(cCBWSignature, ptrData, sizeof(cCBWSignature)) == 0) {   // check that the signature matches
        USB_MASS_STORAGE_CBW *ptrCBW = (USB_MASS_STORAGE_CBW *)ptrData;
        ucActiveLUN = ptrCBW->dCBWLUN;                                   // the logical unit number to use
        if (fnGetPartition(ucActiveLUN, 0) != 0) {                       // if valid storage device
            if ((ptrCBW->dCBWCBLength > 0) && (ptrCBW->dCBWCBLength <= 16)) { // check for valid length
                if ((ptrCBW->dmCBWFlags & CBW_IN_FLAG) != 0) {
                    switch (ptrCBW->CBWCB[CBW_OperationCode]) {
                    case UFI_MODE_SENSE_6:
                        {                                                // {15}
                            CBW_MODE_SENSE_6 *ptrSense = (CBW_MODE_SENSE_6 *)ptrCBW->CBWCB;
                            unsigned char ucLogicalUnitNumber = (ptrSense->ucLogicalUnitNumber_DBD >> 5);
                            if (fnGetPartition(ucLogicalUnitNumber, 0) != 0) {
                                if ((ptrSense->ucPC_PageCode & MODE_SENSE_6_PAGE_CODE_ALL_PAGES) == MODE_SENSE_6_PAGE_CODE_ALL_PAGES) { // only accept all page request
                                    return TRANSPARENT_CALLBACK;         // accept to be handled by task
                                }
                            }
                        }
                        break;                                           // stall
                    case UFI_REQUEST_SENSE:
                    case UFI_INQUIRY:
                        return TRANSPARENT_CALLBACK;                     // the call-back has done its work and the input buffer can now be used

                    case UFI_READ_CAPACITY:                              // {36}
                    case UFI_READ_FORMAT_CAPACITY:
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) != 0) { // {16} only respond when there is media inserted, else stall
                            return TRANSPARENT_CALLBACK;                 // the call-back has done its work and the input buffer can now be used
                        }
                        break;                                           // stall
                    case UFI_READ_10:
                    case UFI_READ_12:
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) != 0) { // {16} only respond when there is media inserted, else stall
                            CBW_READ_10 *ptrRead = (CBW_READ_10 *)ptrCBW->CBWCB;
                            ulLogicalBlockAdr = ((ptrRead->ucLogicalBlockAddress[0] << 24) | (ptrRead->ucLogicalBlockAddress[1] << 16) | (ptrRead->ucLogicalBlockAddress[2] << 8) | ptrRead->ucLogicalBlockAddress[3]);
                            if (ulLogicalBlockAdr < ptrDiskInfo[ucActiveLUN]->ulSD_sectors) { // check that the sector is valid
                                return TRANSPARENT_CALLBACK;             // the call-back has done its work and the input buffer can now be used
                            }
                        }
                        break;                                           // stall
                    default:
                        break;                                           // stall
                    }
                }
                else {
                    switch (ptrCBW->CBWCB[CBW_OperationCode]) {
                    case UFI_PRE_ALLOW_REMOVAL:
                    case UFI_TEST_UNIT_READY:
                        return TRANSPARENT_CALLBACK;                     // the call-back has done its work and the input buffer can now be used
                    case UFI_WRITE_10:
                    case UFI_WRITE_12:
                        if ((ptrDiskInfo[ucActiveLUN]->usDiskFlags & (DISK_MOUNTED | DISK_UNFORMATTED)) != 0) { // {16} only respond when there is media inserted, else stall
                            CBW_WRITE_10 *ptrWrite = (CBW_WRITE_10 *)ptrCBW->CBWCB;
                            ulLogicalBlockAdr = ((ptrWrite->ucLogicalBlockAddress[0] << 24) | (ptrWrite->ucLogicalBlockAddress[1] << 16) | (ptrWrite->ucLogicalBlockAddress[2] << 8) | ptrWrite->ucLogicalBlockAddress[3]);
                            if (ulLogicalBlockAdr < ptrDiskInfo[ucActiveLUN]->ulSD_sectors) { // check that the sector is valid
                                iWriteInProgress = 1;                    // {46} do not handle further commands until the data had been received
                                return TRANSPARENT_CALLBACK;             // the call-back has done its work and the input buffer can now be used
                            }                        
                        }
                        break;                                           // stall
                    default:
                        break;
                    }
                }
                uMemcpy(&csw.dCBWTag, ptrCBW->dCBWTag, sizeof(csw.dCBWTag)); // save the tag since we will halt
                uMemcpy(csw.dCSWDataResidue, &ptrCBW->dCBWDataTransferLength, sizeof(csw.dCSWDataResidue)); // since no data will be processes the residue is equal to the requested transfer length
                csw.bCSWStatus = CSW_STATUS_COMMAND_FAILED;              // set fail
            }
        }
    }
    return STALL_ENDPOINT;                                               // stall on any invalid formats
}

// Get partition details - used to ease support of multiple LUNs (Logical Units)
//
static unsigned char *fnGetPartition(unsigned char ucLUN, QUEUE_TRANSFER *length)
{
    if (ucLUN >= NUMBER_OF_PARTITIONS) {
        return 0;                                                        // invalid LUN
    }
    if (length != 0) {
        *length = sizeof(inquiryData);                                   // the inquiry data length
    }
    return (unsigned char *)&inquiryData;                                // the fixed inquiry data
}
#endif

#if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
// Handle potential USB-MSD data reception from device
//
static void fnHandleMSD_device_reception(QUEUE_HANDLE USBPortID_msd_host, unsigned char ucInputMessage[LARGE_MESSAGE])
{
    while (fnMsgs(USBPortID_msd_host) != 0) {                            // reception from host IN endpoint
        QUEUE_TRANSFER Length;
        Length = fnRead(USBPortID_msd_host, ucInputMessage, 64);         // read the content (up to 64 bytes content each time)
    #if 0
        {
            int i = 0;
            fnDebugMsg("Rx = ");
            fnDebugDec(Length, WITH_CR_LF);
            while (i < Length) {
                fnDebugHex(ucInputMessage[i], (sizeof(ucInputMessage[i]) | WITH_LEADIN | WITH_SPACE));
                if ((++i % 16) == 0) {
                    fnDebugMsg("\r\n");
                }
            }
        }
    #endif
        switch (iUSB_MSD_OpCode) {
        case UFI_READ_10:
            {
                static unsigned char ucBuffer[512];
                static int iOffset = 0;
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive next block or the status transport
                if (Length >= ulBlockByteCount) {                        // final data packet
                    uMemcpy(&ucBuffer[iOffset], ucInputMessage, ulBlockByteCount);
                    ulBlockByteCount = 0;
                    iOffset = 0;
                    iUSB_MSD_OpCode |= STATUS_TRANSPORT;
                }
                else {                                                   // more data expected
                    uMemcpy(&ucBuffer[iOffset], ucInputMessage, Length);
                    iOffset += Length;
                    ulBlockByteCount -= Length;
                }
            }
            break;
        case UFI_INQUIRY:                                                // expecting data transport from inquiry
            {
                //CBW_INQUIRY_DATA *ptrInquiry = (CBW_INQUIRY_DATA *)ucInputMessage;
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport
                iUSB_MSD_OpCode |= STATUS_TRANSPORT;
            }
            break;
        case UFI_READ_FORMAT_CAPACITY:
            {
                CBW_CAPACITY_LIST *ptrCapacityList = (CBW_CAPACITY_LIST *)ucInputMessage;
                unsigned char ucListLength = ptrCapacityList->ucCapacityListLength;
                CAPACITY_DESCRIPTOR *ptrCapacityDescriptor = &ptrCapacityList->capacityDescriptor;
                const UTDISK *ptrDiskInfo = fnGetDiskInfo(DISK_MEM_STICK);
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport
                iUSB_MSD_OpCode |= STATUS_TRANSPORT;
                while (ucListLength >= 8) {                              // for each capacity descriptor
                    unsigned long ulBlockLength = ((ptrCapacityDescriptor->ucBlockLength[0] << 16) | (ptrCapacityDescriptor->ucBlockLength[1] << 8) | ptrCapacityDescriptor->ucBlockLength[2]);
                    unsigned long ulNoOfBlocks  = ((ptrCapacityDescriptor->ucNumberOfBlocks[0] << 24) | (ptrCapacityDescriptor->ucNumberOfBlocks[1] << 16) | (ptrCapacityDescriptor->ucNumberOfBlocks[2] << 8) | ptrCapacityDescriptor->ucNumberOfBlocks[3]);
                    fnDebugMsg("(");
                    fnDebugDec(ptrCapacityDescriptor->ucDescriptorCode, 0);
                    fnDebugMsg(":");
                    fnDebugDec(ulBlockLength, 0);
                    fnDebugMsg(":");
                    fnDebugDec(ulNoOfBlocks, 0);
                    fnDebugMsg(") ");
                    ptrCapacityDescriptor++;
                    if (ucListLength >= 16) {
                        ucListLength -= 8;
                    }
                    else {
                        ((UTDISK *)ptrDiskInfo)->ulSD_sectors = ulNoOfBlocks; // if we are to format the memory stick we need to know how many sectors it has
                        break;
                    }
                }
            }
            break;
        case UFI_REQUEST_SENSE:
            {
                //CBW_RETURN_SENSE_DATA *ptrSenseData = (CBW_RETURN_SENSE_DATA *)ucInputMessage;
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport
                iUSB_MSD_OpCode |= STATUS_TRANSPORT;
            }
            break;
        case UFI_READ_CAPACITY:
            {
                CBW_READ_CAPACITY_DATA *ptrCapacity = (CBW_READ_CAPACITY_DATA *)ucInputMessage;
                unsigned long ulLastLogicalBlockAddress = ((ptrCapacity->ucLastLogicalBlockAddress[0] << 24) | (ptrCapacity->ucLastLogicalBlockAddress[1] << 16) | (ptrCapacity->ucLastLogicalBlockAddress[2] << 8) | ptrCapacity->ucLastLogicalBlockAddress[3]);
                unsigned long ulBlockLengthInBytes =  ((ptrCapacity->ucBlockLengthInBytes[0] << 24) | (ptrCapacity->ucBlockLengthInBytes[1] << 16) | (ptrCapacity->ucBlockLengthInBytes[2] << 8) | ptrCapacity->ucBlockLengthInBytes[3]);
                const UTDISK *ptrDiskInfo = fnGetDiskInfo(DISK_MEM_STICK);
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport
                iUSB_MSD_OpCode |= STATUS_TRANSPORT;
                fnDebugMsg("(");
                fnDebugDec(ulBlockLengthInBytes, 0);                     // block length
                fnDebugMsg(":");
                fnDebugDec(ulLastLogicalBlockAddress, 0);                // last logical block address
                fnDebugMsg(") ");
                ((UTDISK *)ptrDiskInfo)->ulSD_sectors = ulLastLogicalBlockAddress; // if we are to format the memory stick we need to know how many sectors it has
            }
            break;
        case (UFI_INQUIRY | STATUS_TRANSPORT):                           // expecting status transport after inquiry data
        case (UFI_READ_FORMAT_CAPACITY | STATUS_TRANSPORT):              // expecting status transport after reading format capacities
        case (UFI_REQUEST_SENSE | STATUS_TRANSPORT):                     // expecting status transport after reading requesting sense
        case (UFI_READ_CAPACITY | STATUS_TRANSPORT):                     // expecting status transport after reading capacity
        case (UFI_TEST_UNIT_READY | STATUS_TRANSPORT):                   // expecting status transport after sending unit ready
        case (UFI_READ_10 | STATUS_TRANSPORT):                           // expecting status transport after reading data
            {
                USB_MASS_STORAGE_CBW_LW *ptrStatus = (USB_MASS_STORAGE_CBW_LW *)ucInputMessage;
                fnDebugMsg("Status transport - ");
                if ((ptrStatus->dCBWSignatureL == USBS_SIGNATURE) && (ptrStatus->dCBWTagL == ulTag)) { // check the status transport's signature and tag
                    if (ptrStatus->dmCBWFlags == CSW_STATUS_COMMAND_PASSED) { // check that there are no errors reported
                        fnDebugMsg("Passed\r\n");
                        ucRequestCount = 0;
                        switch (iUSB_MSD_OpCode) {
                        case (UFI_INQUIRY | STATUS_TRANSPORT):           // inquiry was successful
                            fnSendMSD_host(UFI_REQUEST_SENSE);           // now request sense
                            break;
                        case (UFI_REQUEST_SENSE | STATUS_TRANSPORT):
                            fnSendMSD_host(UFI_READ_FORMAT_CAPACITY);    // now request format capacity
                            break;
                        case (UFI_READ_FORMAT_CAPACITY | STATUS_TRANSPORT):
                            fnSendMSD_host(UFI_READ_CAPACITY);           // now request capacity
                            break;
                        case (UFI_READ_CAPACITY | STATUS_TRANSPORT):
                            // At this point the memory stick details are known and it can be mounted
                            //
                            fnEventMessage(TASK_MASS_STORAGE, TASK_USB_HOST, MOUNT_USB_MSD); // initiate mounting the memory stick at the mass storage task
                            break;
                        case (UFI_TEST_UNIT_READY | STATUS_TRANSPORT):
                            fnSendMSD_host(UFI_READ_FORMAT_CAPACITY);    // now request format capacity
                            break;
                        case (UFI_READ_10 | STATUS_TRANSPORT):
                            iUSB_MSD_OpCode = 0;
                            fnDebugMsg("READ:");
                            break;
                        }
                    }
                    else {                                               // failed
                        iUSB_MSD_OpCode &= ~(STATUS_TRANSPORT);          // the origial request
                        if (UFI_TEST_UNIT_READY == iUSB_MSD_OpCode) {    // unit of not ready
                            fnDebugMsg("Not ready\r\n");                 // alway repeat when not ready
                            iUSB_MSD_OpCode = UFI_REQUEST_SENSE;
                        }
                        else if (ucRequestCount != 0) {                  // if the original request stalled
                            if (UFI_READ_FORMAT_CAPACITY == iUSB_MSD_OpCode) { // some sticks may not support this so we repeat just three times and then accept that there will be no ansswer
                                if (ucRequestCount >= 3) {
                                    iUSB_MSD_OpCode = UFI_READ_CAPACITY; // continue with next request
                                    ucRequestCount = 0;
                                }
                            }
                            else if (UFI_READ_CAPACITY == iUSB_MSD_OpCode) { // some sticks may be slow and stall on UFI_READ_FORMAT_CAPACITY and then never accept UFI_READ_CAPACITY
                                if (ucRequestCount >= 3) {
                                    iUSB_MSD_OpCode = UFI_TEST_UNIT_READY;
                                    ucRequestCount = 0;
                                }
                            }
                        }
                        uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), T_REPEAT_COMMAND); // repeat command after a short delay
                    }
                }
                else {
                    fnDebugMsg("ERROR STATUS\r\n");
                    iUSB_MSD_OpCode = 0;
                }
            }
        }
    }
}

static void fnRequestLUN(int iChannel)
{
    if (fnWrite(USB_control[USB_HOST_REF], (unsigned char *)&get_max_lum, sizeof(get_max_lum)) != 0) { // return directly (non-buffered) - the data must remain stable
        ucCollectingMode[iChannel] = GET_MAX_LUN;                        // expect data of this type
        usExpectedData[iChannel] = 1;                                    // the length of the expected response
    }
}

static int fnSendMSD_host(unsigned char ucOpcode)
{
    USB_MASS_STORAGE_CBW_LW command_transport;
    QUEUE_TRANSFER command_length;
    uMemset(&command_transport, 0x00, sizeof(command_transport));
    ulTag++;                                                             // increment the transaction ID for each transfer
    command_transport.dCBWSignatureL = USBC_SIGNATURE;
    command_transport.dCBWTagL = ulTag;                                  // transaction identifier
    command_transport.dCBWLUN = (ucDeviceLUN - 1);
    command_transport.CBWCB[0] = ucOpcode;
    command_length = 31;                                                 // physical size of USB_MASS_STORAGE_CBW_LW

    switch (ucOpcode)  {
    case UFI_INQUIRY:
        command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(36);// data transfer length
        command_transport.dmCBWFlags = CBW_IN_FLAG;
        command_transport.dCBWCBLength = 0x06;
        command_transport.CBWCB[4] = 36;
        fnDebugMsg("UFI INQUIRY -> ");
        break;
    case UFI_READ_FORMAT_CAPACITY:
        command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(64);// data transfer length
        command_transport.dmCBWFlags = CBW_IN_FLAG;
        command_transport.dCBWCBLength = 10;
        command_transport.CBWCB[8] = 64;
        fnDebugMsg("UFI FORMAT CAP. -> ");
        break;
    case UFI_REQUEST_SENSE:
        command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(18);// data transfer length
        command_transport.dmCBWFlags = CBW_IN_FLAG;
        command_transport.dCBWCBLength = 12;
        command_transport.CBWCB[4] = 18;
        fnDebugMsg("UFI REQUEST SENSE -> ");
        break;
    case UFI_READ_CAPACITY:
        command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(8); // data transfer length
        command_transport.dmCBWFlags = CBW_IN_FLAG;
        command_transport.dCBWCBLength = 10;
        fnDebugMsg("UFI READ CAP. -> ");
        break;
    case UFI_TEST_UNIT_READY:
      //command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(0); // data transfer length
      //command_transport.dmCBWFlags = 0;
        command_transport.dCBWCBLength = 6;
        fnDebugMsg("UFI RDY -> ");
        break;
    #if defined UTFAT_WRITE
    case UFI_WRITE_10:
    #endif
    case UFI_READ_10:
        command_transport.dCBWDataTransferLengthL = LITTLE_LONG_WORD(512); // data transfer length
    #if defined UTFAT_WRITE
        if (UFI_WRITE_10 != ucOpcode) {
            command_transport.dmCBWFlags = CBW_IN_FLAG;
        }
    #else
        command_transport.dmCBWFlags = CBW_IN_FLAG;
    #endif
        command_transport.dCBWCBLength = 10;
        command_transport.CBWCB[2] = (unsigned char)(ulLBA >> 24);
        command_transport.CBWCB[3] = (unsigned char)(ulLBA >> 16);
        command_transport.CBWCB[4] = (unsigned char)(ulLBA >> 8);
        command_transport.CBWCB[5] = (unsigned char)ulLBA;
        command_transport.CBWCB[7] = (unsigned char)(usBlockCount >> 8);
        command_transport.CBWCB[8] = (unsigned char)(usBlockCount);
        ulBlockByteCount = (usBlockCount * 512);                         // the amount of data in bytes
        break;
    default:
        return -1;
    }
    fnFlush(USBPortID_msd_host, FLUSH_TX);                               // always flush the tx buffer to ensure message alignment in buffer before sending
    if (fnWrite(USBPortID_msd_host, (unsigned char *)&command_transport, command_length) == command_length) { // write data (buffered on this bulk endpoint)
        if ((ucOpcode != UFI_WRITE_10) && (ucOpcode != UFI_READ_10)) {
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), T_GET_STATUS_TEST); // collect the status after a short delay (we should do it after the transmission has been acked!!!)
        }
        if (UFI_TEST_UNIT_READY == ucOpcode) {
            iUSB_MSD_OpCode = (ucOpcode | STATUS_TRANSPORT);             // this message has no data reception so we immediately request the status transport
        }
        else {
            iUSB_MSD_OpCode = ucOpcode;                                  // the operation in progress
        }
    }
    else {
        fnDebugMsg("Host write failure!\r\n");                           // the host could not send, due to device removal or buffer error
    }
    return 0;
}

static int utReadMSD(unsigned char *ptrBuffer, unsigned long ulSectorNumber)
{
    #define MAX_USB_MSD_READ_WAIT (50000)
    volatile int iWait;
    USB_MASS_STORAGE_CBW_LW status_stage;
    QUEUE_TRANSFER byte_length = 512;
    QUEUE_TRANSFER Length;
    ulLBA = ulSectorNumber;                                              // read a sector from this logical block address
    usBlockCount = 1;                                                    // read a single block
    if (USBPortID_msd_host == NO_ID_ALLOCATED) {                         // check that the USB device is ready
        return UTFAT_DISK_NOT_READY;
    }
    fnFlush(USBPortID_msd_host, FLUSH_TX);                               // always flush the tx buffer to ensure message alignment in buffer before commanding
    fnSendMSD_host(UFI_READ_10);                                         // request the read (and enable IN polling for the read data)
#if defined _iMX
    fnDelayLoop(1000); // temporary to ensure the command was sent before starting an IN
#endif
    fnDriver(USBPortID_msd_host, (RX_ON), 0);                            // enable IN polling so that we can receive data response
    iWait = 0;
    FOREVER_LOOP() {                                                     // wait until the data has been received
        Length = fnRead(USBPortID_msd_host, (ptrBuffer + (512 - byte_length)), byte_length); // read received data
        if (Length != 0) {                                               // all or part of the requested data is ready
            byte_length -= Length;                                       // the remaining length
            fnDriver(USBPortID_msd_host, (RX_ON), 0);                    // enable IN polling so that we can receive further data, or the status stage
            iWait = 0;
            if (byte_length == 0) {                                      // data has been completely received so we are waiting for the status stage
                FOREVER_LOOP() {                                         // wait until the status stage has been received
                    if (fnRead(USBPortID_msd_host, (unsigned char *)&status_stage, sizeof(status_stage)) != 0) { // read status stage
                        if ((status_stage.dCBWSignatureL == USBS_SIGNATURE) && (status_stage.dCBWTagL == ulTag)) {
                          //fnDebugMsg(" OK\r\n");
                            break;
                        }
                        else {
                            fnDebugMsg(" Status error\r\n");
                            return UTFAT_DISK_READ_ERROR;
                        }
                    }
                    else {
    #if defined _WINDOWS
                        fnSimInts(0);                                    // allow the USB interrupts to be simulated
    #endif
                        if (++iWait > MAX_USB_MSD_READ_WAIT) {          // limit the maximum wait in case the memory stick fails
                            fnDebugMsg(" TOS\r\n");
                            return UTFAT_DISK_READ_ERROR;
                        }
                    }
                }
                break;
            }
        }
        else {
    #if defined _WINDOWS
            fnSimInts(0);                                                // allow the USB interrupts to be simulated
    #endif
            if (++iWait > MAX_USB_MSD_READ_WAIT) {                       // limit the maximum wait in case the memory stick fails
                fnDebugMsg(" TOD\r\n");                                  // timeout
                return UTFAT_DISK_READ_ERROR;
            }
        }
    }
    return UTFAT_SUCCESS;
}

// Read a single sector from the disk (usb-msd drive) - the sector number is specified by ulSectorNumber
//
extern int utReadMSDsector(UTDISK *ptr_utDisk, unsigned long ulSectorNumber, void *ptrBuf)
{
  //fnDebugMsg("Sec");
    return (utReadMSD((unsigned char *)ptrBuf, ulSectorNumber));         // this blocks until the read completes (or fails)
}

// Read a single sector from the disk (usb-msd drive) - the sector number is specified by ulSectorNumber but return only demanded quantity
//
extern int utReadPartialMSDsector(UTDISK *ptr_utDisk, unsigned long ulSectorNumber, void *ptrBuf, unsigned short usOffset, unsigned short usLength)
{
    unsigned long ulTemp[512/sizeof(unsigned long)];
    int iRtn;
  /*fnDebugMsg("Part");
    fnDebugHex(usOffset, (sizeof(usOffset) | WITH_SPACE | WITH_LEADIN));
    fnDebugMsg("/");
    fnDebugDec(usLength, 0);*/
    iRtn = utReadMSD((unsigned char *)ulTemp, ulSectorNumber);           // read sector content to temporary buffer
    if (UTFAT_SUCCESS == iRtn) {
        uMemcpy(ptrBuf, (((unsigned char *)ulTemp) + usOffset), usLength); // return the requested content
    }
    return iRtn;
}

    #if defined UTFAT_WRITE
static int utWriteMSD(unsigned char *ptrBuffer, unsigned long ulSectorNumber)
{
    #define MAX_USB_MSD_WRITE_WAIT (500000)
    volatile int iWait;
    USB_MASS_STORAGE_CBW_LW status_stage;
    ulLBA = ulSectorNumber;                                              // write a sector from this logical block address
    usBlockCount = 1;                                                    // write a single block
  //fnDebugHex(ulSectorNumber, (sizeof(ulSectorNumber) | WITH_SPACE | WITH_LEADIN));
    fnSendMSD_host(UFI_WRITE_10);                                        // request the write
#if defined _iMX
    fnDelayLoop(1000); // temporary to ensure the command was sent before sending the data
#endif
    fnWrite(USBPortID_msd_host, ptrBuffer, 512);                         // write data (buffered on this bulk endpoint)
    iWait = 0;
    while (fnWrite(USBPortID_msd_host, 0, 512) < 512) {                  // wait until the buffer has been transmitted before enabling IN tokens
        if (++iWait > MAX_USB_MSD_WRITE_WAIT) {                          // limit the maximum wait in case the memory stick fails
            fnDebugMsg(" T1\r\n");
            iWait = 0;
          //return UTFAT_DISK_WRITE_ERROR;
        }
    #if defined _WINDOWS
        fnSimInts(0);                                                    // allow the USB interrupts to be simulated
    #endif
    }
    iWait = 0;
    fnDriver(USBPortID_msd_host, (RX_ON), 0);                            // enable IN polling so that we can receive data/status response (this will block until the trnsmission has completed)
    FOREVER_LOOP() {                                                     // wait until the status stage has been received
        if (fnRead(USBPortID_msd_host, (unsigned char *)&status_stage, sizeof(status_stage)) != 0) { // read status stage
            if ((status_stage.dCBWSignatureL == USBS_SIGNATURE) && (status_stage.dCBWTagL == ulTag)) {
              //fnDebugMsg(" OK\r\n");
                break;
            }
            else {
                fnDebugMsg(" Staus error\r\n");
                return UTFAT_DISK_READ_ERROR;
            }
        }
        else {
    #if defined _WINDOWS
            fnSimInts(0);                                                // allow the USB interrupts to be simulated
    #endif
            if (++iWait > MAX_USB_MSD_WRITE_WAIT) {                      // limit the maximum wait in case the memory stick fails
                fnDebugMsg(" TO\r\n");
                return UTFAT_DISK_READ_ERROR;
            }
        }
    }
    return UTFAT_SUCCESS;
}

extern int utCommitMSDSectorData(UTDISK *ptr_utDisk, void *ptrBuffer, unsigned long ulSectorNumber)
{
  //fnDebugMsg("W");
    return (utWriteMSD((unsigned char *)ptrBuffer, ulSectorNumber));
}

extern int utDeleteMSDSector(UTDISK *ptr_utDisk, unsigned long ulSectorNumber)
{
    unsigned long ulTemp[512/sizeof(unsigned long)];
  //fnDebugMsg("Del");
    uMemset(ulTemp, 0, sizeof(ulTemp));                                  // prepare a zeroed buffer to be written
    return (utWriteMSD((unsigned char *)ulTemp, ulSectorNumber));
}
    #endif

static void fnMSD_device_remove(QUEUE_HANDLE *ptr_USBPortID_msd_host, int iInterface)
{
    ucLocalMSDEvent[iInterface] = 0;
    fnDebugDec(iInterface, (WITH_SPACE | WITH_CR_LF));
    #if defined SUPPORT_QUEUE_DEALLOCATION
    fnClose(ptr_USBPortID_msd_host);                                     // free up device specific queue memory and invalidate the interface handle
    #endif
    fnEventMessage(TASK_MASS_STORAGE, TASK_USB_HOST, USB_MSD_REMOVED);   // {45} initiate de-mounting the memory stick at the mass storage task
}
#endif
#endif

/********************************** MSD interrupt event handling **********************************/
#if defined MSD_INTERRUPT_EVENTS
    #if defined USB_HOST_SUPPORT && defined USB_MSD_HOST
            case EVENT_LUN_READY:
                fnDebugMsg("LUN =");
                fnDebugDec(ucDeviceLUN, (WITH_CR_LF | WITH_SPACE));      // max LUN
                ucDeviceLUN = 1;                                         // communicate with first LUN
                fnSendMSD_host(UFI_INQUIRY);                             // continue by requesting information from the connected MSD
                break;
    #endif
#endif

/********************************** MSD timer event handling **********************************/
#if defined MSD_TIMER_EVENTS
            case T_GET_STATUS_AFTER_STALL:
                ucRequestCount++;                                        // count the number of stall that are cleared for this request
                iUSB_MSD_OpCode |= STATUS_TRANSPORT;                     // mark that the status transport is being read
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport after the endpoint was cleared
                break;
            case T_GET_STATUS_TEST:
                fnDriver(USBPortID_msd_host, (RX_ON), 0);                // enable IN polling so that we can receive the status transport after the endpoint was cleared
                break;
            case T_REPEAT_COMMAND:
                fnSendMSD_host(iUSB_MSD_OpCode);                         // repeat command since the stick reported an error when last sent (assume the stick is not yet ready)
                break;
            case T_REQUEST_LUN:
                fnRequestLUN(USB_HOST_REF);
                break;
#endif
