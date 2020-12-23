/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland
    
    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      Driver.c (containing queue driver and some library replacement routines)
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    24.03.2007 Cast to quieten GNU compiler                              {1}
    25.08.2007 uStrlen() const type to match prototype                   {2}
    25.08.2007 Use static RAM for code from RAM since uMalloc may not always be ready {3}
    12.10.2007 Add USB driver interface                                  {4}
    26.01.2008 Corrected variable name                                   {5}
    30.01.2008 Make use of fnNetworkTx() conditional on Ethernet         {6}
    12.10.2008 Conditional compile of library-replacement routines       {7}
    28.03.2009 Add optional write to input queue                         {8}
    16.04.2009 Add optional string utility return pointer optimisation   {9}
    16.04.2009 Add CR + LF to fnBufferHex() and fnDebugDec()             {10}
    08.05.2009 Add _REMOVE_FORMATTED_OUTPUT to aid code size optimisation{11}
    08.05.2009 Add _NO_CHECK_QUEUE_INPUT to aid code size optimisation   {12}
    08.06.2009 Add optional long memory set and copies                   {13}
    01.07.2009 Standardise string routines when STRING_OPTIMISATION is defined {14}
    17.08.2009 Add SSC support                                           {15}
    19.12.2009 Ignore calls to fnDriver() with the network handle        {16}
    02.02.2010 Add IPV6_DELIMITER                                        {17}
    18.04.2010 Add USB DMA support                                       {18}
    24.09.2010 Modify uStrcmp() for full compatibility with strcmp()     {19}
    19.11.2010 Adapt fnDecStrHex() to only accept '0'..'9' so that it behaves more like atoi() {20}
    29.11.2010 Add capability to request remaining size of a destination task's input queue {21}
    05.02.2010 Jump white space and handle negative numbers in fnDecStrHex() {22}
    21.10.2011 Add fnGetQueueHandle()                                    {23}
    06.12.2011 Add fnPeekInput() and fnGetBufPeek()                      {24}
    09.12.2012 Add reserved count value to queue read/writes to ensure that buffer limit check remains accurate "during" nested interrupt operations to same queue {25}
    10.11.2012 Add USER_WRITE_HANDLERS support                           {26}
    10.11.2012 Add IPV6_STRING_ROUTINES option                           {27}
    10.11.2012 Allow uStrcmp() to recognise two empty strings as being equal {28}
    04.06.2013 Added QUEUE_MALLOC() default                              {29}
    12.09.2013 Added uReverseMemcpy()                                    {30}
    01.03.2015 Add FIFO queue                                            {31}
    01.03.2015 Control flush of input or output and return flushed content size {32}
    17.12.2017 Change uMemset() to match memset() parameters             {32}
    23.03.2018 Optimise /= 10 used by fnBufferDec() using look-up table  {33}
    10.08.2018 Move uStrEquiv() from ip_utils.c to here and add uStrstr() and uStrstrCaseInsensitive() {34}
    24.12.2018 Add uStrlenSafe()                                         {35}
    17.08.2010 Add SUPPORT_QUEUE_DEALLOCATION optional support
    07.09.2020 Add fnDebugStyle()                                        {36}

*/


/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "config.h"


/* =================================================================== */
/*                          Global driver variables                    */
/* =================================================================== */

QUEUE_HANDLE DebugHandle = NETWORK_HANDLE;                               // if this is left with this value, debug data can be sent to a network (if supported by application)
IDINFO *que_ids = 0;                                                     // pointer to queue list

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined QUEUE_MALLOC                                                // {29}
    #define QUEUE_MALLOC(x)      uMalloc((MAX_MALLOC)(x))
#endif

/* =================================================================== */
/*                       local structure definitions                   */
/* =================================================================== */


/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static unsigned short fnWriteInternal(unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes);
static QUEUE_TRANSFER entry_que(QUEUE_HANDLE channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID);


/* =================================================================== */
/*                             constants                               */
/* =================================================================== */


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static QUEUE_HANDLE NrQueues = 0;                                        // the number of queues available (determined during initialisation)

/* =================================================================== */
/*                      local function definitions                     */
/* =================================================================== */


/* =================================================================== */
/*                      global function definitions                    */
/* =================================================================== */


// Allocate space for managing the desired amount of queues
//
extern void fnNeedQueues(QUEUE_LIMIT queues_needed)
{
    NrQueues = queues_needed;                                            // the amount of queues needed in system
    que_ids = (IDINFO *)QUEUE_MALLOC(NrQueues * sizeof(IDINFO));         // get exact required zeroed queue space
}


// Allocate memory for a queue buffer and fill out its control structure
//
#if defined SUPPORT_QUEUE_DEALLOCATION
static unsigned char *fnAllocateBuffer(QUEQUE *ptQUEQue, unsigned short usLength, void *(*_malloc_type)(MAX_MALLOC __size))
#else
static unsigned char *fnAllocateBuffer(QUEQUE *ptQUEQue, unsigned short usLength)
#endif
{
    unsigned char *new_memory;

#if defined SUPPORT_QUEUE_DEALLOCATION
    if (NO_MEMORY == (new_memory = (unsigned char *)_malloc_type(usLength))) {
        return (NO_MEMORY);
    }
#else
    if (NO_MEMORY == (new_memory = (unsigned char *)QUEUE_MALLOC(usLength))) {
        return (NO_MEMORY);
    }
#endif

    ptQUEQue->buffer_end = ptQUEQue->QUEbuffer = (unsigned char *)new_memory;
    ptQUEQue->get = (unsigned char *)new_memory;
    ptQUEQue->put = (unsigned char *)new_memory;
  //ptQUEQue->chars = 0;
    ptQUEQue->buffer_end += usLength;
    ptQUEQue->buf_length = usLength;

    return (new_memory);
}

// This routine is general to all drivers requiring queues. It creates queue structures on heap after checking that it has not already been allocated.
//
extern QUEUE_HANDLE fnAllocateQueue(QUEUE_DIMENSIONS *ptrQueueDimensions, QUEUE_HANDLE channel_handle, QUEUE_TRANSFER (*entry_add)(QUEUE_HANDLE Channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID), QUEUE_TRANSFER FullCntLength)
{
    QUEUE_HANDLE NewHandle;

    if (NO_ID_ALLOCATED != fnSearchID(entry_add, channel_handle)) {      // check that this combination doesn't already exist
        return NO_ID_ALLOCATED;                                          // the combination exists so we refuse to recreate it
    }

    if (NO_ID_ALLOCATED != (NewHandle = fnSearchID (0, 0))) {            // get the next free queue handle
        IDINFO *ptQueID = &que_ids[NewHandle - 1];                       // set pointer to the new queue entry
    #if defined SUPPORT_QUEUE_DEALLOCATION
        void *(*_malloc_type)(MAX_MALLOC __size);
        #if defined _WINDOWS
        if (ptrQueueDimensions->ucHeapType == 0xcc) {
            _EXCEPTION("type needs to be correctly defined in calling code!!");
        }
        #endif
        if (ptrQueueDimensions->ucHeapType == 1) {
            _malloc_type = _uCalloc;
        }
        else {
            _malloc_type = uMalloc;
        }
        #define _QUEUE_MALLOC     _malloc_type
        #define MALLOC_CALL_TYPE , _malloc_type
    #else
        #define _QUEUE_MALLOC     QUEUE_MALLOC
        #define MALLOC_CALL_TYPE
    #endif
        if ((ptrQueueDimensions->RxQueueSize) != 0) {                    // if a reception buffer is required by the queue
            if (NO_MEMORY != (ptQueID->input_buffer_control = (void *)_QUEUE_MALLOC(FullCntLength))) { // get input queue control block memory
    #if defined USB_INTERFACE && defined USB_DMA_RX && defined USB_RAM_START // {18}
                if ((entry_add != entry_usb) && (NO_MEMORY == fnAllocateBuffer(ptQueID->input_buffer_control, ptrQueueDimensions->RxQueueSize MALLOC_CALL_TYPE))) { //  allocated actual rx buffer in memory
                    return NO_ID_ALLOCATED;
                }
    #else
                if (NO_MEMORY == fnAllocateBuffer(ptQueID->input_buffer_control, ptrQueueDimensions->RxQueueSize MALLOC_CALL_TYPE)) { // now allocate actual rx buffer in memory
                    return NO_ID_ALLOCATED;
                }
    #endif
            }
        }

        if ((ptrQueueDimensions->TxQueueSize) != 0) {                    // if a transmission buffer is required by the queue
            if (NO_MEMORY != (ptQueID->output_buffer_control = (void *)_QUEUE_MALLOC(FullCntLength))) { // get outout queue control block memory
    #if defined USB_INTERFACE && defined USB_DMA_TX && defined USB_RAM_START
                if (entry_add == entry_usb) {
                  //if (NO_MEMORY == fnAllocateUSBBuffer(1, channel_handle, ptQueID->output_buffer_control, ptrQueueDimensions->TxQueueSize)) { //  allocated actual rx buffer in USB memory
                  //    return NO_ID_ALLOCATED;
                  //}
                }
                else if (NO_MEMORY == fnAllocateBuffer(ptQueID->output_buffer_control, ptrQueueDimensions->TxQueueSize MALLOC_CALL_TYPE)) { // now allocated actual tx buffer in memory
                    return NO_ID_ALLOCATED;
                }
    #else
                if (NO_MEMORY == fnAllocateBuffer(ptQueID->output_buffer_control, ptrQueueDimensions->TxQueueSize MALLOC_CALL_TYPE)) { // now allocate actual tx buffer in memory
                    return NO_ID_ALLOCATED;
                }
    #endif
            }
        }

        ptQueID->CallAddress = entry_add;                                // enter the queue's handler
        ptQueID->qHandle = channel_handle;                               // enter the queue's channel number
    }
    return NewHandle;                                                    // return the new queue's handle
}


// Generic open call for all software and interface queues
//
extern QUEUE_HANDLE fnOpen(unsigned char type_of_driver, unsigned char driver_mode, void *pars)
{
    switch (type_of_driver)  {
    case TYPE_QUEUE:
        {
            // Queues are always created at start up from the task table information so we
            // create a rx queue only for each defined task
            //
            QUEUE_DIMENSIONS QueueDimensions;
            QueueDimensions.RxQueueSize = (((TTASKTABLE*)pars)->QueLength); // set Rx length with no Tx length
            QueueDimensions.TxQueueSize = 0;
#if defined SUPPORT_QUEUE_DEALLOCATION
            QueueDimensions.ucHeapType = 0;
#endif
            return (fnAllocateQueue(&QueueDimensions, (unsigned char)*(((TTASKTABLE*)pars)->pcTaskName), entry_que, sizeof(QUEQUE)));
        }

#if defined SERIAL_INTERFACE
    case TYPE_TTY:
        return (fnOpenTTY((TTYTABLE*)pars, driver_mode));
#endif

#if defined PRINTER_INTERFACE
    case TYPE_PRN:
        return (fnOpenPRN((PRNTABLE*)pars, driver_mode));
#endif

#if defined CAN_INTERFACE
    case TYPE_CAN:
        return (fnOpenCAN((CANTABLE *)pars, driver_mode));
#endif

#if defined ETH_INTERFACE || defined WIFI_INTERFACE || ((defined USB_CDC_RNDIS || defined USB_RNDIS_HOST) && defined USB_TO_TCP_IP) || defined USE_PPP
    case TYPE_ETHERNET:
        return (fnOpenETHERNET((ETHTABLE *)pars, driver_mode));
#endif

#if defined SPI_INTERFACE
    case TYPE_SPI:
        return (fnOpenSPI((SPITABLE *)pars, driver_mode));
#endif

#if defined I2C_INTERFACE
    case TYPE_I2C:
        return (fnOpenI2C((I2CTABLE *)pars));
#endif

#if defined USB_INTERFACE
    case TYPE_USB:
        return (fnOpenUSB((USBTABLE *)pars, driver_mode));
#endif

#if defined SSC_INTERFACE                                                // {15}
    case TYPE_SSC:
        return (fnOpenSSC((SSCTABLE *)pars, driver_mode));
#endif

#if defined SUPPORT_FIFO_QUEUES
    case TYPE_FIFO:                                                      // {31}
        {
            static CHAR queue_name = 0;
            TTASKTABLE table;                                            // temporary task table object
            do {
                queue_name--;                                            // first FIFO queue name will be 0xff which is unlikely to collide with a task name
                if (queue_name == 0) {
                    return NO_ID_ALLOCATED;                              // all possible entries tried without success
                }
            } while (NO_ID_ALLOCATED != fnSearchID(entry_que, queue_name)); // check that the FIFO queue is unique and doesn't match with a task input queue
            table.pcTaskName = &queue_name;
            table.QueLength = (QUEUE_TRANSFER)((CAST_POINTER_ARITHMETIC)(pars)); // the requested length of the FIFO
            return (fnOpen(TYPE_QUEUE, FOR_READ, &table));               // open a queue for FIFO use (a FIFO uses the same queue as a task input but is not assigned to an actual task - a write to the FIFO queue doesn't cause any task to be scheduled, and the FIFO queue can be written and read by any software knowing its queue ID)
        }
#endif
    default:
        return NO_ID_ALLOCATED;                                          // driver type is not supported...
    }
}

#if defined SUPPORT_QUEUE_DEALLOCATION
static void fnFreeQueue(QUEQUE *ptr_buffer_control)
{
    if (ptr_buffer_control != 0) {                                       // if there is a queue
        if (ptr_buffer_control->QUEbuffer != 0) {                        // if the queueu has a buffer
            uCFree(ptr_buffer_control->QUEbuffer);                       // free the queue buffer memory
        }
        uCFree(ptr_buffer_control);                                      // free the queue control memory
    }
}

extern int fnClose(QUEUE_HANDLE *ptrHandle)
{
    if (*ptrHandle != NO_ID_ALLOCATED) {
        QUEUE_HANDLE driver_id = *ptrHandle;
        IDINFO *ptQueID = &que_ids[driver_id - 1];
        fnFreeQueue(ptQueID->input_buffer_control);
        fnFreeQueue(ptQueID->output_buffer_control);
        uMemset(ptQueID, 0x00, sizeof(IDINFO));                          // queue entry is now free
        *ptrHandle = NO_ID_ALLOCATED;                                    // the calling handle is no longer valid
        return 0;
    }
    else {
        return -1;                                                       // queue handle doesn't exist
    }
}
#endif


// Allow driver mode setting - returns present driver state
//
extern QUEUE_TRANSFER fnDriver(QUEUE_HANDLE driver_id, unsigned short state, unsigned short rx_or_tx)
{
    if (NETWORK_HANDLE == driver_id) {                                   // {16} ignore calls with the network handle
        return 0;
    }
    driver_id -= 1;
    return (que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, (unsigned char *)(CAST_POINTER_ARITHMETIC)rx_or_tx, state, CALL_DRIVER, driver_id); // {1}
}

// Write to queue or interface. Returns the number of bytes successfully sent
//
extern QUEUE_TRANSFER fnWrite(QUEUE_HANDLE driver_id, unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    if (INTERNAL_ROUTE == driver_id) {                                   // write to internal network
        return (fnWriteInternal(output_buffer, nr_of_bytes));            // routing info contained in message itself
    }
    else if (driver_id == NETWORK_HANDLE) {                              // debug output to network
#if defined ETH_INTERFACE || defined USB_CDC_RNDIS || defined USB_RNDIS_HOST || defined USER_NETWORK_WRITE // {6}
        return (fnNetworkTx(output_buffer, nr_of_bytes));
#else
        return 0;
#endif
    }
#if defined USER_WRITE_HANDLERS && (USER_WRITE_HANDLERS > 0)             // {26}
    else if (driver_id >= (NETWORK_HANDLE - USER_WRITE_HANDLERS)) {      // if the write is to a handle in the user write range allow the user to handle
        return (fnUserWrite(driver_id, output_buffer, nr_of_bytes));     // the user must supply this routine and handle it according to user requirements
    }
#endif
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, output_buffer, nr_of_bytes, CALL_WRITE, driver_id));
}


// Read from any queue. Returns the number of bytes successfully read
//
extern QUEUE_TRANSFER fnRead(QUEUE_HANDLE driver_id, unsigned char *input_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, input_buffer, nr_of_bytes, CALL_READ, driver_id));
}

#if !defined _NO_CHECK_QUEUE_INPUT                                       // {12}
// Get the number of waiting messages (can also be bytes) from any queue
//
extern QUEUE_TRANSFER fnMsgs(QUEUE_HANDLE driver_id)
{
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, 0, 0, CALL_INPUT, driver_id));
}
#endif

// Get a pointer to a data block of a linear queue buffer
//
extern QUEUE_TRANSFER fnGetDataBlock(QUEUE_HANDLE driver_id, unsigned char ucChannel, unsigned char **ptrBuf)
{
    driver_id -= 1;
    return (que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, (unsigned char*)ptrBuf, ucChannel, CALL_READ_LINEAR, driver_id);
}

#if defined SUPPORT_FLUSH
// Flush a queue
//
extern QUEUE_TRANSFER fnFlush(QUEUE_HANDLE driver_id, unsigned char ucTxRx)
{
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, 0, ucTxRx, CALL_FLUSH, driver_id));
}
#endif

#if defined ETH_INTERFACE || defined WIFI_INTERFACE
// Free a linear queue buffer after it has been read
//
extern QUEUE_TRANSFER fnFreeBuffer(QUEUE_HANDLE driver_id, signed char cChannel)
{
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, 0, cChannel, CALL_FREE, driver_id));
}
#endif

#if defined SERIAL_SUPPORT_SCAN
extern QUEUE_TRANSFER fnScan(QUEUE_HANDLE driver_id, unsigned char *ucSequence, unsigned char ucSeqLength)
{
    driver_id -= 1;
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, ucSequence, ucSeqLength, CALL_SCAN, driver_id));
}
#endif

#if defined SERIAL_SUPPORT_PEEK                                          // {24}
extern QUEUE_TRANSFER fnPeekInput(QUEUE_HANDLE driver_id, unsigned char *input_buffer, QUEUE_TRANSFER nr_of_bytes, int iOldestNewest)
{
    unsigned char ucPeekType;
    driver_id -= 1;
    if (iOldestNewest == PEEK_NEWEST_INPUT) {
        ucPeekType = CALL_PEEK_NEWEST;
    }
    else {
        ucPeekType = CALL_PEEK_OLDEST;
    }
    return ((que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, input_buffer, nr_of_bytes, ucPeekType, driver_id));
}
#endif

#if defined SERIAL_STATS
extern void fnStats(QUEUE_HANDLE driver_id, QUEUE_TRANSFER CounterNumber, unsigned long *ulCountValue)
{
    driver_id -= 1;
    (que_ids[driver_id].CallAddress)(que_ids[driver_id].qHandle, (unsigned char*)ulCountValue, CounterNumber, CALL_STATS, driver_id);
}
#endif




/********************************* Software queue support here *************************************************/

// This routine enters always with disabled interrupts. Interrupts are reactivated as soon as possible so that copies do not block longer than necessary
//
extern QUEUE_TRANSFER fnGetBuf(QUEQUE *ptQUEQue, unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    unsigned char *ptrGet = ptQUEQue->get;

    if (nr_of_bytes > ptQUEQue->chars) {
        nr_of_bytes = ptQUEQue->chars;
    }

    if (nr_of_bytes != 0) {
        ptQUEQue->get += nr_of_bytes;

        if (ptQUEQue->get >= ptQUEQue->buffer_end) {                     // we have a circular buffer and can either copy once or must copy twice
            QUEUE_TRANSFER FirstCopy = (QUEUE_TRANSFER)(ptQUEQue->buffer_end - ptrGet); // we need double copy
            ptQUEQue->get = ptQUEQue->QUEbuffer + (nr_of_bytes - FirstCopy); // correct get pointer since it has passed the end of the buffer

            uEnable_Interrupt();                                         // enable interrupts during copy

            if (output_buffer != 0) {
                uMemcpy(output_buffer, ptrGet, FirstCopy);
                output_buffer += FirstCopy;
                uMemcpy(output_buffer, ptQUEQue->QUEbuffer, (nr_of_bytes - FirstCopy));
            }
        }
        else {
            uEnable_Interrupt();                                         // enable interrupts during copy
            if (output_buffer != 0) {                                    // we can do it with a single copy
                uMemcpy(output_buffer, ptrGet, nr_of_bytes);
            }
        }
        uDisable_Interrupt();                                            // protect the counter adjustment
        ptQUEQue->chars -= nr_of_bytes;
    }
    uEnable_Interrupt();
    return nr_of_bytes;                                                  // return the number of bytes copied
}

#if defined SERIAL_SUPPORT_PEEK                                          // {24}
// This routine doesn't need protected interrupts. It returns content without removing it from the buffer
//
extern QUEUE_TRANSFER fnGetBufPeek(QUEQUE *ptQUEQue, unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes, int iNewest)
{
    unsigned char *ptrGet = ptQUEQue->get;

    if (nr_of_bytes > ptQUEQue->chars) {
        nr_of_bytes = ptQUEQue->chars;
    }
    else if (iNewest != 0) {
        ptrGet += (ptQUEQue->chars - nr_of_bytes);                       // if the newest data is to be peeked at move to the position of the newest nr_of_bytes bytes in the circular input buffer
        if (ptrGet >= ptQUEQue->buffer_end) {
             ptrGet -= ptQUEQue->buf_length;
        }
    }

    if ((nr_of_bytes != 0) && (output_buffer != 0)) {
        if ((ptrGet + nr_of_bytes) >= ptQUEQue->buffer_end) {            // we have a circular buffer and can either copy once or must copy twice
            QUEUE_TRANSFER FirstCopy = (QUEUE_TRANSFER)(ptQUEQue->buffer_end - ptrGet); // we need double copy
            uMemcpy(output_buffer, ptrGet, FirstCopy);
            output_buffer += FirstCopy;
            uMemcpy(output_buffer, ptQUEQue->QUEbuffer, (nr_of_bytes - FirstCopy));
        }
        else {                                                           // we can do it with a single copy
            uMemcpy(output_buffer, ptrGet, nr_of_bytes);
        }
    }
    return nr_of_bytes;                                                  // return the number of bytes copied
}
#endif

// This routine always enters and leaves with enabled interrupts.
//
extern QUEUE_TRANSFER fnFillBuf(QUEQUE *ptQUEQue, unsigned char *input_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    QUEUE_TRANSFER FirstCopy;
    unsigned char *ptrTo;

    uDisable_Interrupt();                                                // protect entry into function, interrupts are reactivated in the function
    if ((ptQUEQue->chars + ptQUEQue->reserved_chars + nr_of_bytes) > ptQUEQue->buf_length) { // {25} check that there is space (including any reserved count)
        nr_of_bytes = (ptQUEQue->buf_length - ptQUEQue->chars - ptQUEQue->reserved_chars); // {25} if not, cut message - user must ensure that this doesn't happen!
        _EXCEPTION("Message cut in size due to lack of queue space - is this expected??");
    }
    ptQUEQue->reserved_chars += nr_of_bytes;                             // {25} the additional count that will be added during the copy
    FirstCopy = nr_of_bytes;
    ptrTo = ptQUEQue->put;
    ptQUEQue->put += nr_of_bytes;                                        // the position of the put pointer after completion
    if (ptQUEQue->put >= ptQUEQue->buffer_end) {
        ptQUEQue->put -= ptQUEQue->buf_length;                           // handle overflow
        FirstCopy -= (QUEUE_TRANSFER)(ptQUEQue->put - ptQUEQue->QUEbuffer);
    }
#if defined USB_INTERFACE && defined USB_DMA_RX && defined USB_RAM_START // {18}
    if (input_buffer != 0) {                                             // don't perform any copy function since the DMA has done this
#endif
        uEnable_Interrupt();                                             // ensure copies do not block interrupts
        // Nested write to the queue may happen here from interrupts
        //
        uMemcpy(ptrTo, input_buffer, FirstCopy);
        if (nr_of_bytes != FirstCopy) {
            uMemcpy(ptQUEQue->QUEbuffer, (input_buffer + FirstCopy), (nr_of_bytes - FirstCopy));
        }

        uDisable_Interrupt();
#if defined USB_INTERFACE && defined USB_DMA_RX && defined USB_RAM_START // {18}
    }
#endif
    if (ptQUEQue->reserved_chars == nr_of_bytes) {                       // {25} if no nested write to the same queue took place or final nested write
        ptQUEQue->chars += (nr_of_bytes + ptQUEQue->new_chars);          // add the character count so that reads can now use new length
        ptQUEQue->reserved_chars = ptQUEQue->new_chars = 0;              // reset reserved counters
    }
    else {
        ptQUEQue->new_chars += nr_of_bytes;                              // a nested write is in progress so sum the new characters
        ptQUEQue->reserved_chars -= nr_of_bytes;                         // reserved characters passed to new characters
    }
    uEnable_Interrupt();
    return nr_of_bytes;                                                  // the number of bytes actually copied
}

#if defined FIFO_TO_QUEUE
// This routine always enters and leaves with enabled interrupts.
// It copies a FIFO input to a circular buffer
//
extern QUEUE_TRANSFER fnFillBuf_FIFO(QUEQUE *ptQUEQue, unsigned char *fifo_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    QUEUE_TRANSFER FirstCopy;
    QUEUE_TRANSFER CopyCount;
    unsigned char *ptrTo;

    uDisable_Interrupt();                                                // protect entry into function, interrupts are reactivated in the function

    if ((ptQUEQue->chars + ptQUEQue->reserved_chars + nr_of_bytes) > ptQUEQue->buf_length) { // {25} check that there is space (including any reserved count)
        nr_of_bytes = (ptQUEQue->buf_length - ptQUEQue->chars - ptQUEQue->reserved_chars); // {25} if not, cut message - user must ensure that this doesn't happen
    }
    ptQUEQue->reserved_chars += nr_of_bytes;                             // {25} the additional count that will be added during the copy
    FirstCopy = nr_of_bytes;
    ptrTo = ptQUEQue->put;
    ptQUEQue->put += nr_of_bytes;                                        // the position of the put pointer after completion
    if (ptQUEQue->put >= ptQUEQue->buffer_end) {
        ptQUEQue->put -= ptQUEQue->buf_length;                           // handle overflow
        FirstCopy -= (QUEUE_TRANSFER)(ptQUEQue->put - ptQUEQue->QUEbuffer);
    }

    uEnable_Interrupt();                                                 // ensure copies do not block interrupts
    // Nested write to the queue may happen here from interrupts
    //
    CopyCount = FirstCopy;
    while (CopyCount-- != 0) {
        *ptrTo++ = *fifo_buffer;
    #if defined _WINDOWS
        fifo_buffer++;
    #endif
    }

    if (nr_of_bytes != FirstCopy) {                                      // complete copy was not possible to one linear region
        ptrTo = ptQUEQue->QUEbuffer;                                     // start of the circular buffer
        CopyCount = (nr_of_bytes - FirstCopy);                           // remaining length
        while (CopyCount--) {
            *ptrTo++ = *fifo_buffer;
    #if defined _WINDOWS
            fifo_buffer++;
    #endif
        }
    }

    uDisable_Interrupt();
        if (ptQUEQue->reserved_chars == nr_of_bytes) {                   // {25} if no nested write to the same queue took place or final nested write
            ptQUEQue->chars += (nr_of_bytes + ptQUEQue->new_chars);      // add the character count so that reads can now use new length
            ptQUEQue->reserved_chars = ptQUEQue->new_chars = 0;          // reset reserved counters
        }
        else {
            ptQUEQue->new_chars += nr_of_bytes;                          // a nested write is in progress so sum the new characters
            ptQUEQue->reserved_chars -= nr_of_bytes;                     // reserved characters passed to new characters
        }
    uEnable_Interrupt();
    return nr_of_bytes;                                                  // the number of bytes actually copied
}
#endif

#if defined SUPPORT_DOUBLE_QUEUE_WRITES
// Write a buffer to an internal queue. This enters and leaves with enabled interrupts except for double writes which are preserved between calls
//
static QUEUE_TRANSFER fnWriteInternal(unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    static QUEUE_HANDLE DestID = NO_ID_ALLOCATED;                        // these variables are preserved over double queue header and data write
    static UTASK_TASK WakeTask;

    QUEUE_TRANSFER (*entry_add)(QUEUE_HANDLE channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID) = entry_que;
    QUEQUE *ptQUEQue;

    if (nr_of_bytes == 0) {                                              // zero length means head of a double queue write
        int iLen;
        if (output_buffer[MSG_DESTINATION_NODE] != output_buffer[MSG_SOURCE_NODE]) { // this is the head of a double queue write
            return 0;                                                    // we do not support distributed nodes at the moment
        }
        uDisable_Interrupt();                                            // protect from this point to ensure no interrupt routine writes to our queue
        WakeTask = (UTASK_TASK)output_buffer[MSG_DESTINATION_TASK];      // save the task to be worken
        if (NO_ID_ALLOCATED == (DestID = fnSearchID (entry_add, (unsigned char)WakeTask))) {
            uEnable_Interrupt();
            return 0;                                                    // no destination found - ignore
        }

        ptQUEQue = (struct stQUEQue *)(que_ids[DestID - 1].input_buffer_control); // set to input control block - write directly to input
        if ((ptQUEQue->chars + HEADER_LENGTH + output_buffer[MSG_CONTENT_LENGTH]) > ptQUEQue->buf_length) { // check that there is adequate space for the write
            DestID = NO_ID_ALLOCATED;                                    // abort double queue write
            uEnable_Interrupt();
            return 0;                                                    // we do not have space for the advertised message...and must also ignore following
        }

        iLen = (HEADER_LENGTH+1);                                        // copy the head but don't wake the task since it is not yet complete...The sending task must complete the job or else there may be problems...
        ptQUEQue->chars += (HEADER_LENGTH + 1);
        while (iLen-- != 0) {                                            // copy the header to the buffer
            *ptQUEQue->put++ = *output_buffer++;
            if (ptQUEQue->put >= ptQUEQue->buffer_end) {                 // handle wrap in circular buffer
                ptQUEQue->put = ptQUEQue->QUEbuffer;
            }
        }
        return (HEADER_LENGTH + 1);                                      // note that we are returning with disabled interrupts
    }
    else {                                                               // this is either a normal write or the second part of a double write
        QUEUE_HANDLE newDestID;
        UTASK_TASK NewWakeTask;
        if (DestID == NO_ID_ALLOCATED) {                                 // normal queue write
            if (output_buffer[MSG_DESTINATION_NODE] != output_buffer[MSG_SOURCE_NODE]) {
    #if defined SUPPORT_DISTRIBUTED_NODES
                return fnDistributedTx(output_buffer, nr_of_bytes);      // send the message across the network to destination task
    #else
                return 0;                                                // we do not support distributed nodes
    #endif
            }
            NewWakeTask = (UTASK_TASK)output_buffer[MSG_DESTINATION_TASK];
            if (NO_ID_ALLOCATED == (newDestID = fnSearchID (entry_add, (unsigned char)NewWakeTask))) {
                return 0;                                                // no destination found - ignore
            }
        }
        else {                                                           // double write being completed
            newDestID = DestID;                                          // the destination ID
            NewWakeTask = WakeTask;                                      // the destination task
            DestID = NO_ID_ALLOCATED;                                    // clear destination ID used during double write
            uEnable_Interrupt();                                         // second phase of double write can be performed with normal protection level
        }
        ptQUEQue = (struct stQUEQue *)(que_ids[newDestID - 1].input_buffer_control); // set to input control block - write directly to input
        if (output_buffer[MSG_SOURCE_TASK] == CHECK_QUEUE) {             // check on destination's queue {21}
            return (ptQUEQue->buf_length - ptQUEQue->chars);             // remaining space in queue (before adding new message)
        }
        nr_of_bytes = fnFillBuf(ptQUEQue, output_buffer, nr_of_bytes);
        uTaskerStateChange(NewWakeTask, UTASKER_ACTIVATE);               // schedule destination task to run once
        return (nr_of_bytes);
    }
}
#else
// Write a buffer to an internal queue. This enters and leaves with enabled interrupts
//
extern QUEUE_TRANSFER fnWriteInternal(unsigned char *output_buffer, QUEUE_TRANSFER nr_of_bytes)
{
    QUEUE_TRANSFER (*entry_add)(unsigned char channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID) = entry_que;
    QUEUE_TRANSFER DataSent = 0;
    QUEUE_HANDLE DestID;
    QUEQUE *ptQUEQue;

    if (output_buffer[MSG_DESTINATION_NODE] == output_buffer[MSG_SOURCE_NODE]) { // first find the destintion information
        UTASK_TASK cWakeTask = (UTASK_TASK)output_buffer[MSG_DESTINATION_TASK]; // local message {5} (WakeTask corrected to cWakeTask)
        if (NO_ID_ALLOCATED == (DestID = fnSearchID (entry_add, (unsigned char)cWakeTask))) { // search for ID with destination address
            return (0);                                                  // no data transfered because no such destination
        }
                                                                         // ID has been found, copy the data
        ptQUEQue = (struct stQUEQue *)(que_ids[DestID-1].input_buffer_control); // set to input control block - write directly to input
        if (output_buffer[MSG_SOURCE_TASK] == CHECK_QUEUE) {             // check on destination's queue {21}
            return (ptQUEQue->buf_length - ptQUEQue->chars);             // remaining space in queue (before adding new message)
        }
        DataSent = fnFillBuf(ptQUEQue, output_buffer, nr_of_bytes);      // protected copy performed here
    #if !defined APPLICATION_WITHOUT_OS
        uTaskerStateChange(cWakeTask, UTASKER_ACTIVATE);                 // wake the task written to so that it can handle the message
    #endif
    }
    else {
    #if defined SUPPORT_DISTRIBUTED_NODES
        return fnDistributedTx(output_buffer, nr_of_bytes);              // send the message across the network to destination task
    #else
        return 0;                                                        // we do not support distributed nodes
    #endif
    }
    return (DataSent);
}
#endif


extern QUEUE_TRANSFER entry_que(QUEUE_HANDLE channel, unsigned char *ptBuffer, QUEUE_TRANSFER Counter, unsigned char ucCallType, QUEUE_HANDLE DriverID)
{
    QUEUE_TRANSFER rtn_val = 0;
    QUEQUE *ptQUEQue = (struct stQUEQue *)(que_ids[DriverID].input_buffer_control); // set to input control block by default

    uDisable_Interrupt();                                                // disable all interrupts
    switch (ucCallType) {
#if !defined _NO_CHECK_QUEUE_INPUT                                       // {12}
    case CALL_INPUT:                                                     // request the number or input characters waiting
      //ptQUEQue = (struct stQUEQue *)(que_ids[DriverID].input_buffer_control); // set to input control block
        rtn_val = ptQUEQue->chars;
        break;
#endif
    case CALL_READ:                                                      // read data from the queue
      //ptQUEQue = (struct stQUEQue *)(que_ids[DriverID].input_buffer_control); // set to input control block
        return (fnGetBuf(ptQUEQue, ptBuffer, Counter));                  // interrupts are reenabled as soon as no longer critical
#if defined SUPPORT_FIFO_QUEUES                                          // {8}
    case CALL_WRITE:
      //ptQUEQue = (struct stQUEQue *)(que_ids[DriverID].input_buffer_control); // set to input control block
        if ((ptQUEQue->buf_length - ptQUEQue->chars) >= Counter) {       // only copy when there is enough room for a complete message
            uEnable_Interrupt();                                         // fnFillBuffer disables and then re-enables interrupts - be sure we are compatible
            if (ptBuffer == 0) {                                         // caller checking buffer space available
                return ((ptQUEQue->buf_length - ptQUEQue->chars - Counter) + 1);
            }
            rtn_val = fnFillBuf(ptQUEQue, ptBuffer, Counter);
            return (rtn_val);
        }
        break;
#endif
#if defined SUPPORT_FLUSH
    case CALL_FLUSH:                                                     // flush input or output completely
        if (FLUSH_TX == Counter) {                                       // {32} flush output, rather than input
            ptQUEQue = (struct stQUEQue *)(que_ids[DriverID].output_buffer_control); // set to output control block
        }
        if (ptQUEQue != 0) {                                             // ignore if there is no queue
            ptQUEQue->get = ptQUEQue->put = ptQUEQue->QUEbuffer;
            rtn_val = ptQUEQue->chars;                                   // {32} the content size that was flushed
            ptQUEQue->chars = 0;
        }
        break;
#endif
    default:
        break;
    }
    uEnable_Interrupt();                                                 // enable interrupts
    return (rtn_val);
}


// Search through the id table for already existing IDs
//
extern QUEUE_HANDLE fnSearchID(QUEUE_TRANSFER (*SearchAddress)(QUEUE_HANDLE, unsigned char *, QUEUE_TRANSFER, unsigned char, QUEUE_HANDLE), QUEUE_HANDLE channel_mask)
{
    IDINFO *pIDtables = que_ids;                                         // initialise pointer to start of ID table
    QUEUE_HANDLE TheID = 0;                                              // index

    while (TheID < NrQueues) {                                           // check through the queue list
        if ((pIDtables->CallAddress == SearchAddress) && (pIDtables->qHandle == channel_mask)) { // if the matching queue is found
            return (TheID + 1);                                          // return the matching queue's handle
        }
        TheID++;
        ++pIDtables;
    }
    return (NO_ID_ALLOCATED);                                            // queue list has been searched but no matching entry was found
}

// Return the physical channel number from a handle
//
extern QUEUE_HANDLE fnGetQueueHandle(QUEUE_HANDLE HandleId)              // {23}
{
    if (NETWORK_HANDLE == HandleId) {                                    // calls with the network handle
        return NETWORK_HANDLE;                                           // return an invalid channel number
    }
	return (que_ids[HandleId - 1].qHandle);
}

// Global routine for sending simple local internal interrupt messages
//
extern QUEUE_TRANSFER fnInterruptMessage(UTASK_TASK Task, unsigned char ucIntEvent)
{
    unsigned char int_message[HEADER_LENGTH];                            // define standard interrupt event

    if (Task == 0) {
        return 0;
    }

    int_message[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;
    int_message[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;
    int_message[MSG_DESTINATION_TASK] = (unsigned char)Task;
    int_message[MSG_SOURCE_TASK]      = INTERRUPT_EVENT;
    int_message[MSG_INTERRUPT_EVENT]  = ucIntEvent;

    return (fnWrite(INTERNAL_ROUTE, int_message, HEADER_LENGTH));
}

// Global routine for sending simple internal event messages (without data content)
//
extern QUEUE_TRANSFER fnEventMessage(UTASK_TASK DestTask, UTASK_TASK SourceTask, unsigned char ucEvent)
{
    unsigned char int_message[HEADER_LENGTH + 1];                        // define standard event

    int_message[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;
    int_message[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;
    int_message[MSG_DESTINATION_TASK] = (unsigned char)DestTask;
    int_message[MSG_SOURCE_TASK]      = (unsigned char)SourceTask;
    int_message[MSG_CONTENT_LENGTH]   = 1;
    int_message[MSG_CONTENT_COMMAND]  = ucEvent;

    return (fnWrite(INTERNAL_ROUTE, int_message, (HEADER_LENGTH + 1)));
}


// Used for sending null terminated text messages to a specific port
//
extern QUEUE_TRANSFER fnPrint(unsigned char *ucToSend, QUEUE_HANDLE portID)
{
    QUEUE_TRANSFER nr_chars = 0;
    unsigned char *ucCount = ucToSend;

    while (0 != *ucCount++) {                                            // first count the number of characters to send
        nr_chars++;
    }

    if (nr_chars == 0) {
        return(0);                                                       // if none, return
    }

    return (fnWrite(portID, ucToSend, nr_chars));                        // send message over port
}

extern QUEUE_TRANSFER fnDebugStyle(unsigned long ulStyle)                // {36}
{    
    if ((ulStyle & OUTPUT_STYLE_COLOR_MASK) != OUTPUT_COLOR_DEFAULT) {
        static const unsigned char ucStyleRef[] = { ESCAPE_SEQUENCE_START, '[', '0', ';', '3', '0', 'm' };
        unsigned char ucStyle[7];
        uMemcpy(ucStyle, ucStyleRef, 7);
        if ((ulStyle & OUTPUT_STYLE_OFF) == OUTPUT_STYLE_OFF) {
            ucStyle[3] = 'm';
            return (fnWrite(DebugHandle, ucStyle, 4));                   // send style over port
        }
        else {
            if ((ulStyle & OUTPUT_STYLE_SECOND_SET_MASK) != 0) {
                ucStyle[2] = '1';
            }
            ucStyle[5] += (ulStyle & OUTPUT_STYLE_OFF);
            return (fnWrite(DebugHandle, ucStyle, 7));                   // send style over port
        }
    }
    return 0;
}

// Used for sending text messages (with null terminator) to debug output
//
extern QUEUE_TRANSFER fnDebugMsg (CHAR *cToSend)
{
    return (fnPrint((unsigned char *)cToSend, DebugHandle));             // send to debug port
}


// Fast divide by 10 routine from https://hackaday.com/2020/06/12/binary-math-tricks-shifting-to-divide-by-ten-aint-easy/#more-415128
//
extern unsigned long fnDivideBy10(unsigned long n)
{
    unsigned long q;
    unsigned long r;
    q = (n >> 1) + (n >> 2);                                             // q=n/2+n/4 = 3n/4
    q = q + (q >> 4);                                                    // q=3n/4+(3n/4)/16 = 3n/4+3n/64 = 51n/644
    q = q + (q >> 8);                                                    // q=51n/64+(51n/64)/256 = 51n/64 + 51n/16384 = 13107n/16384 q = q + (q >> 16); // q= 13107n/16384+(13107n/16384)/65536=13107n/16348+13107n/1073741824=858993458n/1073741824
    // Note: q is now roughly 0.8n
    //
    q = q >> 3;                                                          // q=n/8 = (about 0.1n or n/10)
    r = n - (((q << 2) + q) << 1);                                       // rounding: r= n-2*(n/10*4+n/10)=n-2*5n/10=n-10n/10
    return (q + (r > 9));                                                // adjust answer by error term
}


#if !defined _REMOVE_FORMATTED_OUTPUT || defined _WINDOWS                // {11} (not removed when simulating since simulator uses some functions)
    #if defined STRING_OPTIMISATION
// This puts the converted string in a buffer
//
extern CHAR *fnBufferDec(signed long slNumberToConvert, unsigned char ucStyle, CHAR *ptrBuf)
    #else
// This either sends the converted string directly over the debug interface or puts it in a buffer
//
extern CHAR *fnDebugDec(signed long slNumberToConvert, unsigned char ucStyle, CHAR *ptrBuf)
    #endif
{
    // Converts the number to ASCII - the result is right aligned in the width given - an additional 0 is added at the end to aid string output
    // range -2^31 .. +2^31
    // length 1 .. 5                                                     // max possible space, inkl. neg
    //
    static const unsigned long ulNextDivide[] = {100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1, 0}; // {33} progressive divide by 10 values
    int iNext = 0;
    register unsigned long ulDiv = 1000000000;                           // largest possible divisor
    unsigned long ulNumberToConvert;
    unsigned char ucResult;
    unsigned int iFirstFound = 0;
    #if defined STRING_OPTIMISATION                                      // used only with a buffer
    CHAR *cPtr = ptrBuf;
    #else
    CHAR *cPtr;
    CHAR cTemp[13];
    if (ptrBuf) {
        cPtr = ptrBuf;
    }
    else {
        cPtr = cTemp;
    }
    #endif

    if ((WITH_SPACE & ucStyle) != 0) {                                   // add a space before the string
        *cPtr++ = ' ';
        ucStyle &= ~LEADING_SPACE;
    }

    if (((ucStyle & DISPLAY_NEGATIVE) != 0) && (slNumberToConvert < 0)) {// if the number is to be interpreted as a signed number and it is negative
        ulNumberToConvert = (unsigned long)-slNumberToConvert;
        *cPtr++ = '-';                                                   // add sign
    }
    else {
        ulNumberToConvert = (unsigned long)slNumberToConvert;
    }

    while (ulNumberToConvert != 0) {                                     // while there is a remainder
        while (ulDiv > ulNumberToConvert) {                              // identify when the number to convert is larger than the divide value
            ulDiv = ulNextDivide[iNext++];                               // ulDiv /= 10;
            if (iFirstFound != 0) {                                      // if this is not the leading digit
                *cPtr++ = '0';                                           // insert a zero mid-string
            }
            else {
                switch (ulDiv) {
                case 10000:
                case 1000:
                case 100:
                    if (((ucStyle & 3) > 1) && ((ucStyle & LEADING_ZERO) != 0)) { // if a leading zero is required
                        *cPtr++ = '0';                                   // insert a leading zero
                    }
                    break;
                case 10:
                    if (((ucStyle & 3) > 0) && ((ucStyle & LEADING_ZERO) != 0)) { // if a leading zero is required
                        *cPtr++ = '0';                                   // insert a leading zero
                    }
                    break;
                case 1:                                                  // smallest division unit reached without being able to divide (value < 10)
                    if ((ucStyle & LEADING_ZERO) != 0) {                 // if a leading zero is required
                        *cPtr++ = '0';                                   // insert a leading zero
                    }
                    else if ((ucStyle & LEADING_SPACE) != 0) {           // of if a leading space is required
                        *cPtr++ = ' ';                                   // insert a leading space
                    }
                    break;
                }
            }
        }
        if (ulDiv != 0) {
            iFirstFound = 1;
        }

        ucResult = (unsigned char)(ulNumberToConvert / ulDiv);
        *cPtr++ = ucResult + '0';
        ulNumberToConvert -= (ucResult * ulDiv);
        ulDiv = ulNextDivide[iNext++];                                   // ulDiv /= 10;
    }
    while ((ulDiv != 0) && (iFirstFound != 0)) {
        ulDiv = ulNextDivide[iNext++];                                   // ulDiv /= 10;
        *cPtr++ = '0';                                                   // add trailing zeros
    }

    if (iFirstFound == 0) {                                              // special case for zero
        if ((ucStyle & LEADING_SPACE) != 0) {
            *cPtr++ = ' ';
        }
        else {
            if ((ucStyle & LEADING_ZERO) != 0) {
                switch (ucStyle & 0x03) {
                case 2:
                    *cPtr++ = '0';
                    *cPtr++ = '0';
                    *cPtr++ = '0';
                    // Fall through intentionally
                    //
                case 1:
                    *cPtr++ = '0';
                    // Fall through intentionally
                    //
                default:
                    *cPtr++ = '0';
                    break;
                }
            }
            *cPtr++ = '0';
        }
    }

    if ((ucStyle & WITH_CR_LF) != 0) {                                   // {10}
        *cPtr++ = '\r';
        *cPtr++ = '\n';
    }
    if ((ucStyle & NO_TERMINATOR) == 0) {
        *cPtr = 0;                                                       // terminate string if not de-activated
    }
    #if !defined STRING_OPTIMISATION
    if (ptrBuf == 0) {
        fnDebugMsg(cTemp);
        return 0;
    }
    #endif
    return (cPtr);
}

#if defined STRING_OPTIMISATION                                           // {14} standardisation of string routines
// This sends the decimal ASCII converted string over the debug interface
//
extern void fnDebugDec(signed long slNumberToConvert, unsigned char ucStyle)
{
    #define MAX_LONG_DEC (11 + 4 + 1)                                    // 11 bytes plus terminator as well as optional spaces and line feeds, for longest positive and negative numbers
    CHAR cVal[MAX_LONG_DEC];                                             // temporary string buffer

    fnBufferDec(slNumberToConvert, ucStyle, cVal);                       // convert the number into an ASCII decinal string

    fnDebugMsg(cVal);                                                    // send to the debug output
}

extern void fnDebugFloat(float floatToConvert, unsigned char ucStyle)   // in development!!
{
    unsigned long ulFraction;
    unsigned long ulWhole;
    signed long slWhole;
    int iFractionLength = (ucStyle & 0x07);                              // number of digits after the decimal point (1..7 - 0 means 8)
    unsigned char ucFirstStyle = ucStyle;
    if (iFractionLength == 0) {
        iFractionLength = 8;
    }
    if (floatToConvert < 0) {                                            // test for negative
        if (floatToConvert <= (signed long)0x80000000) {
            fnDebugDec((signed long)0x80000000, (ucStyle | DISPLAY_NEGATIVE)); // display maximum negative
            return;
        }
        slWhole = (signed long)floatToConvert;
        floatToConvert += (float)slWhole;                                // the fraction remaining
        ucFirstStyle |= DISPLAY_NEGATIVE;                                // display for negative integer part
        while (iFractionLength-- != 0) {
            floatToConvert *= 10;
        }
        ulFraction = (unsigned long)floatToConvert;
        if (ulFraction != 0) {
            ucFirstStyle &= ~WITH_CR_LF;
        }
        fnDebugDec(slWhole, ucFirstStyle);                               // display negative integer part
    }
    else {
        if (floatToConvert >= (unsigned long)0xffffffff) {
            fnDebugDec((signed long)0xffffffff, (ucStyle & ~DISPLAY_NEGATIVE)); // display maximum positive
            return;
        }
        ucFirstStyle &= ~DISPLAY_NEGATIVE;                               // display for positive integer part
        ulWhole = (unsigned long)floatToConvert;
        floatToConvert -= (float)ulWhole;                                // the fraction remaining
        while (iFractionLength-- != 0) {
            floatToConvert *= 10;
        }
        ulFraction = (unsigned long)floatToConvert;
        if (ulFraction != 0) {
            ucFirstStyle &= ~WITH_CR_LF;
        }
        fnDebugDec((signed long)ulWhole, ucFirstStyle);                  // display positive integer part
    }
    if (ulFraction != 0) {
        fnDebugMsg(".");                                                 // decimal point
        fnDebugDec((signed long)ulFraction, (ucStyle & ~(LEADING_ZERO | DISPLAY_NEGATIVE | WITH_SPACE | LEADING_SPACE))); // display positive integer part
    }
}
#endif


// Write hex value to buffer
//
extern CHAR *fnBufferHex(unsigned long ulValue, unsigned char ucLen, CHAR *pBuf)
{
    unsigned char ucTemp;
    int iTerminate = 0;
    int iCapitals = 0;
    int iCRLF = 0;                                                       // {10}

    if ((ucLen & WITH_SPACE) != 0) {
        *pBuf++ = ' ';
    }
    if ((ucLen & WITH_LEADIN) != 0) {
        *pBuf++ = '0';
        *pBuf++ = 'x';
    }
    if ((ucLen & NO_TERMINATOR) == 0) {
       iTerminate = 1;
    }
    if ((ucLen & CODE_CAPITALS) != 0) {
        iCapitals = 1;
    }
    if( (ucLen & WITH_CR_LF) != 0) {                                     // {10}
        iCRLF = 1;
    }
    ucLen &= 0x7;                                                        // mask out possible sizes

    if (ucLen > 4) {                                                     // limit size
        ucLen = 4;                                                       // to length of a long
    }

    ucLen *= 2;                                                          // 2 bytes per byte

    while (ucLen-- != 0) {
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
    if (iCRLF != 0) {                                                    // {10}
        *pBuf++ = '\r';
        *pBuf++ = '\n';
    }
    if (iTerminate != 0) {
    #if defined STRING_OPTIMISATION                                      // {9}
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


// Converts an ascii hex byte to its hex value (the short string input can be terminated with any characters below '0').
// This is for example useful for converting a mac address input with dashes between each byte
//
extern unsigned long fnHexStrHex(CHAR *cNewAdd)
{
    unsigned long ulRet = 0;
    unsigned char ucTemp;

    while (*cNewAdd == ' ') {                                            // jump leading  white space
        cNewAdd++;
    }

    while ((*cNewAdd >= '0')
    #if defined MAC_DELIMITER && MAC_DELIMITER >= '0'
        && (*cNewAdd != MAC_DELIMITER)
    #endif
    #if (defined USE_IPV6 || defined IPV6_STRING_ROUTINES) && IPV6_DELIMITER >= '0' // {17}{27}
        && (*cNewAdd != IPV6_DELIMITER)
    #endif
        ) {
        ulRet <<= 4;
        ucTemp = (*cNewAdd++ - '0');
        if (ucTemp > 9) {
            ucTemp -= ('a' - '9' - 1);
            ucTemp &= 0x0f;                                              // handle upper case
        }
        ulRet |= ucTemp;
    }

    return ulRet;		
}

// Extract a decimal number from a string
//																						
extern unsigned long fnDecStrHex(CHAR *cNewAdd)
{
    int iNegative = 0;                                                   // {22} a caller requiring negative must cast the result to signed long
    unsigned long ulRet = 0;

    while ((*cNewAdd < '0') || (*cNewAdd > '9')) {                       // jump leading white space and handle negative numbers
        if (*cNewAdd == 0) {
            return 0;                                                    // no valid content found so return the value 0
        }
        if (*cNewAdd == '-') {
            iNegative = 1;                                               // the number is to be handled as a negative result
        }
        cNewAdd++;
    }

    while ((*cNewAdd >= '0') && (*cNewAdd <= '9')) {                     // {20}
        ulRet = (ulRet + ulRet + (ulRet * 8));                           // * 10 - (hopefully) without multiplications
        ulRet += (*cNewAdd++ - '0');
    }
    if (iNegative != 0) {                                                // the result is to be returned as a negative value
        return ((~ulRet) + 1);                                           // it is up to the caller to handle the result as a signed number if negative entry is expected
    }
    return ulRet;
}

// Extract a float input from a string and return its float value
//	
extern float fnFloatStrFloat(CHAR *cNewAdd)
{
    int iNegative = 0;
    unsigned int uiDiv = 1;
    signed long slInteger = (signed long)fnDecStrHex(cNewAdd);           // convert the integer
    float fFraction = 0;
    if ((slInteger == 0) && (*cNewAdd == '-')) {                         // catch case of negative umber with zero integer part
        iNegative = 1;
    }
    while (*cNewAdd != 0) {
        if (*cNewAdd++ == '.') {
            while (*cNewAdd != 0) {
                if (*cNewAdd == '0') {                                   // coun the number of leading zeros in the mantissa
                    cNewAdd++;
                    uiDiv *= 10;
                }
                else {
                    break;
                }
            }
            fFraction = (float)fnDecStrHex(cNewAdd);                     // convert the mantissa to an integer value
            while (*cNewAdd++ != 0) {                                    // count the length of the mantissa
                uiDiv *= 10;
            }
            fFraction /= uiDiv;                                          // convert the mantissa integer to a fraction
            if (iNegative != 0) {                                        // special case of negative number with zero integer part
                fFraction = -fFraction;
                return fFraction;
            }
            break;
        }
    }
    if (slInteger < 0) {
        fFraction -= (float)slInteger;                                   // subtract the integer part to the fraction to obtain the comple floating point value
    }
    else {
        fFraction += (float)slInteger;                                   // add the integer part to the fraction to obtain the comple floating point value
    }
    return fFraction;
}
#endif

#if defined LONG_UMEMSET                                                 // {13}
extern void uMemset_long(unsigned long *ptrTo, unsigned long ulValue, size_t Size)
{
    Size /= sizeof(unsigned long);
    while (Size-- != 0) {
        *ptrTo++ = ulValue;
    }
}
#endif

#if defined LONG_UMEMCPY                                                 // {13}
extern void uMemcpy_long(unsigned long *ptrTo, const unsigned long *ptrFrom, size_t Size)
{
    Size /= sizeof(unsigned long);
    while (Size-- != 0) {
        *ptrTo++ = *ptrFrom++;
    }
}
#endif


#if defined UREVERSEMEMCPY && (!defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA || defined DMA_REVMEMCPY_NOT_POSSIBLE) // {30}
// uMemcpy() executed in a reverse order to allow shifting buffers to the right
//
extern void *uReverseMemcpy(void *ptrTo, const void *ptrFrom, size_t Size)
{
    void *buffer = ptrTo;
    unsigned char *ptr1 = (unsigned char *)ptrTo;
    unsigned char *ptr2 = (unsigned char *)ptrFrom;
    ptr1 += Size;                                                        // move to the end of the buffers
    ptr2 += Size;

    while (Size-- != 0) {
        *(--ptr1) = *(--ptr2);                                           // copy backwards
    }

    return buffer;
}
#endif
#if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA

/********************************* Keep the order of next routines since they can be run from SRAM ************/
// memset implementation
//
    #if !defined uMemset                                                 // {7}
        #if defined RUN_LOOPS_IN_RAM                                     // execute this routine from RAM to optimise its speed
void (*uMemset)(void *ptrTo, unsigned char ucValue, size_t Size);
static void *_uMemset(void *ptrTo, int iValue, size_t Size)
        #else
extern void *uMemset(void *ptrTo, int iValue, size_t Size)
        #endif
{
    register unsigned char ucValue = (unsigned char)iValue;              // {32} convert the int parameter to unsigned char
    void *buffer = ptrTo;
    unsigned char *ptr = (unsigned char *)ptrTo;

    while (Size-- != 0) {
        *ptr++ = ucValue;
    }
    return buffer;
}
    #endif

// memcpy implementation
//
    #if !defined uMemcpy                                                 // {7}
        #if defined RUN_LOOPS_IN_RAM                                     // execute this routine from RAM to optimise its speed
void (*uMemcpy)(void *ptrTo, const void *ptrFrom, size_t Size);
static void *_uMemcpy(void *ptrTo, const void *ptrFrom, size_t Size)
        #else
extern void *uMemcpy(void *ptrTo, const void *ptrFrom, size_t Size)
        #endif
{
    void *buffer = ptrTo;
    unsigned char *ptr1 = (unsigned char *)ptrTo;
    unsigned char *ptr2 = (unsigned char *)ptrFrom;

    while (Size-- != 0) {
        *ptr1++ = *ptr2++;
    }

    return buffer;
}
    #endif
#endif



// memcmp implementation
//
#if !defined uMemcmp                                                     // {7}
    #if defined RUN_LOOPS_IN_RAM
int (*uMemcmp)(const void *ptrTo, const void *ptrFrom, size_t Size);
static int _uMemcmp(const unsigned char *ptrTo, const unsigned char *ptrFrom, size_t Size)
    #else
extern int uMemcmp(const void *ptrTo, const void *ptrFrom, size_t Size)
    #endif
{
    unsigned char *_ptrTo = (unsigned char *)ptrTo;
    unsigned char *_ptrFrom = (unsigned char *)ptrFrom;

    while (Size-- != 0) {
        if (*_ptrTo++ != *_ptrFrom++) {
            return 1;                                                    // match failed
        }
    }
    return 0;                                                            // compared memory is identical
}
#endif

// Check a null terminated string for equality
//
#if !defined uStrcmp                                                     // {7}
    #if defined RUN_LOOPS_IN_RAM
int (*uStrcmp)(const CHAR *ptrStr1, const CHAR *ptrStr2);
static int _uStrcmp(const CHAR *ptrStr1, const CHAR *ptrStr2)
    #else
extern int uStrcmp(const CHAR *ptrStr1, const CHAR *ptrStr2)             // {28} change from do {} while to while() so that empty strinsg are equal
    #endif
{
    while (*ptrStr1 != 0) {
        if (*ptrStr1 != *ptrStr2) {                                      // {19}
            if (*ptrStr1 > *ptrStr2) {                                   // check value of non-matching character
                return 1;                                                // string 1 has higher value that string 2
            }
            return -1;                                                   // string 1 has lower value that string 2
        }
        ptrStr1++;                                                       // {19}
        ptrStr2++;
    }
    if (*ptrStr2 != 0) {
        return -1;                                                       // {19} we have found a partial match - but the second string is longer
    }
    return 0;                                                            // both strings are identical
}
#endif

// strcpy implementation (but return next address that would have been written to)
//
#if !defined uStrcpy                                                     // {7}
    #if defined RUN_LOOPS_IN_RAM
CHAR *(*uStrcpy)(CHAR *ptrTo, const CHAR *ptrFrom);
static CHAR *_uStrcpy(CHAR *ptrTo, const CHAR *ptrFrom)
    #else
extern CHAR *uStrcpy(CHAR *ptrTo, const CHAR *ptrFrom)
    #endif
{
    do {
        *ptrTo++ = *ptrFrom;
    } while (*ptrFrom++ != 0);
    #if defined STRING_OPTIMISATION                                      // {9}
    return (ptrTo - 1);                                                  // return pointer to null termination to simplify concatination
    #else
    return ptrTo;                                                        // return the pointer to the address which would have been written to next
    #endif
}
#endif

// strlen implementation
//
#if !defined uStrlen                                                     // {7}
    #if defined RUN_LOOPS_IN_RAM
size_t(*uStrlen)(const CHAR *ptrStr);
static int _uStrlen(const CHAR *ptrStr)
    #else
extern size_t uStrlen(const CHAR *ptrStr)
    #endif
{
    size_t length = 0;

    while (*ptrStr++ != 0) {                                             // search for the null-terminator at the end of the string
        length++;
    }
    return length;                                                       // return the number of characters found before the null-terminator
}
#endif

extern size_t uStrlenSafe(const CHAR *ptrStr, size_t MaxStrLength)       // {35}
{
    size_t length = 0;
    while (length < MaxStrLength) {                                      // limit the search to the specified length
        if (*ptrStr++ == 0) {                                            // search for the null-terminator at the end of the string
            break;
        }
        length++;
    }
    return length;
}

// Tries to match a string, where lower and upper case are treated as equal
//
extern unsigned short uStrEquiv(const CHAR *cInput, const CHAR *cMatch)  // {34}
{
    unsigned short usMatch = 0;
    CHAR cReference;

    while ((cReference = *cMatch) != 0) {
        if (*cInput != cReference) {
            if (cReference >= 'a') {                                     // verify that it is not the case which doesn't match
                cReference -= ('a' - 'A');                               // try capital match
            }
            else if (cReference >= 'A') {
                cReference += ('a' - 'A');                               // try small match
            }
            if (*cInput != cReference) {                                 // last chance
                return 0;
            }
        }
        cMatch++;
        cInput++;
        usMatch++;
    }
    return usMatch;                                                      // return the length of match
}

// strstr implementation
//
extern CHAR *uStrstr(const CHAR *ptrStringToScan, const CHAR *ptrStringToMatch) // {34}
{
    size_t MatchLength = uStrlen(ptrStringToMatch);                      // the length of the match required
    while (*ptrStringToScan != 0) {                                      // scan the string
        if (uMemcmp(ptrStringToScan, ptrStringToMatch, MatchLength) == 0) { // try to match the string
            return (CHAR *)ptrStringToScan;                              // match found so return a pointer to its start location
        }
        ptrStringToScan++;
    }
    return 0;                                                            // string not found
}

// strstr implementation that is case insensitive
//
extern CHAR *uStrstrCaseInsensitive(const CHAR *ptrStringToScan, const CHAR *ptrStringToMatch) // {34}
{
    unsigned short usMatchLength;
    while (*ptrStringToScan != 0) {                                      // scan the string
        usMatchLength = uStrEquiv(ptrStringToScan, ptrStringToMatch);
        if (usMatchLength != 0) {
            return (CHAR *)ptrStringToScan;                              // match found so return a pointer to its start location
        }
        ptrStringToScan++;
    }
    return 0;                                                            // string not found
}


#if defined RUN_LOOPS_IN_RAM
extern void fnInitDriver(void)                                           // {3}
{
    int i = 0;
    #if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA
    #define ProgLength  ((CAST_POINTER_ARITHMETIC)fnInitDriver - (CAST_POINTER_ARITHMETIC)_uMemset)
    unsigned char *ptrProg = (unsigned char *)_uMemset;
    #else
    #define ProgLength  ((CAST_POINTER_ARITHMETIC)fnInitDriver - (CAST_POINTER_ARITHMETIC)_uMemcmp)
    unsigned char *ptrProg = (unsigned char *)_uMemcmp;
    #endif
    #if defined _WINDOWS
    #undef ProgLength
        #if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA
    #define ProgLength (6 * 500)
        #else
    #define ProgLength (4 * 500)
        #endif
    #endif
    static unsigned long ulCodeSpace[ProgLength/sizeof(unsigned long)];  // long word aligned code space
    unsigned char *ptrNewMem = (unsigned char *)ulCodeSpace;

    while (i++ < ProgLength) {                                           // copy the routines to heap
        *ptrNewMem++ = *ptrProg++;
    }

    ptrNewMem -= ProgLength;                                             // now set each start address

    #if defined _WINDOWS
        #if defined DMA_MEMCPY_SET && !defined DEVICE_WITHOUT_DMA
    memcpy(ptrNewMem, _uMemset, 500);
    uMemset = (void (*)(void *, unsigned char, size_t ))ptrNewMem;
    ptrNewMem += 500;
    memcpy(ptrNewMem, _uMemcpy, 500);
    uMemcpy = (int (*)(void *, const void *, size_t ))ptrNewMem;
    ptrNewMem += 500;
        #endif
    memcpy(ptrNewMem, _uMemcmp, 500);
    uMemcmp = (int (*)(const void *, const void *, size_t ))ptrNewMem;
    ptrNewMem += 500;
    memcpy(ptrNewMem, _uStrcmp, 500);
    uStrcmp = (int (*)(const CHAR *, const CHAR *))ptrNewMem;
    ptrNewMem += 500;
    memcpy(ptrNewMem, _uStrcpy, 500);
    uStrcpy = (CHAR * (*)(CHAR *, const CHAR *))ptrNewMem;
    ptrNewMem += 500;
    memcpy(ptrNewMem, _uStrlen, 500);
    uStrlen = (size_t(*)(const CHAR *))ptrNewMem;                        // {2}
    #else
        #if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA
    uMemset = (void (*)(void *, unsigned char, size_t ))ptrNewMem;
    ptrNewMem += (MAX_MALLOC)((CAST_POINTER_ARITHMETIC)_uMemcpy - (CAST_POINTER_ARITHMETIC)_uMemset);
    uMemcpy = (void (*)(void *, const void *, size_t ))ptrNewMem;
    ptrNewMem += (MAX_MALLOC)((CAST_POINTER_ARITHMETIC)_uMemcmp - (CAST_POINTER_ARITHMETIC)_uMemcpy);
        #endif
    uMemcmp = (int (*)(const void *, const void *, size_t ))ptrNewMem;
    ptrNewMem += (MAX_MALLOC)((CAST_POINTER_ARITHMETIC)_uStrcmp - (CAST_POINTER_ARITHMETIC)_uMemcmp);
    uStrcmp = (int (*)(const CHAR *, const CHAR *))ptrNewMem;
    ptrNewMem += (MAX_MALLOC)((CAST_POINTER_ARITHMETIC)_uStrcpy - (CAST_POINTER_ARITHMETIC)_uStrcmp);
    uStrcpy = (CHAR * (*)(CHAR *, const CHAR *))ptrNewMem;
    ptrNewMem += (MAX_MALLOC)((CAST_POINTER_ARITHMETIC)_uStrlen  - (CAST_POINTER_ARITHMETIC)_uStrcpy);
    uStrlen = (size_t(*)(const CHAR *))ptrNewMem;                        // {2}
    #endif
}
#endif
