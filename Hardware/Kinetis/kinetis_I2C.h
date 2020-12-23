/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_I2C.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    09.10.2012 Extend I2C speed settings                                 {1}
    25.04.2014 Add automatic I2C lockup detection and recovery           {2}
    24.09.2014 Add I2C slave mode
    07.01.2016 Added 4th I2C interface
    27.03.2016 Add arbitration lost check and recovery                   {3}
    24.12.2016 Add I2C_SLAVE_RX_MESSAGE_MODE and generate I2C_SLAVE_WRITE_COMPLETE on slave reception after repeated-start {4}
    21.01.2018 If the slave is already addressed when the stop condition handling is performed, allow the reception handling to continue {5}
    23.01.2018 If the slave is already addressed when the start condition handling is performed, allow the reception handling to continue {6}
    24.03.2019 Add DMA mode and I2C_2_BYTE_LENGTH option
    15.01.2020 Add K02F I2C0 option on PTE24 and PTE25
    16.11.2020 I2C simulation extended to support multiple I2C buses     {7}
*/

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static void fnConfigI2C_pins(QUEUE_HANDLE Channel,  int iMaster);
static void fnSendSlaveAddress(I2CQue *ptI2CQue, QUEUE_HANDLE Channel, KINETIS_I2C_CONTROL *ptrI2C);
static void fnI2C_Handler(KINETIS_I2C_CONTROL *ptrI2C, int iChannel);
#if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
    static void fnStartI2C_TxDMA(KINETIS_I2C_CONTROL *ptrI2C, I2CQue *ptI2CQue, QUEUE_HANDLE Channel);
    static void fnStartI2C_RxDMA(KINETIS_I2C_CONTROL *ptrI2C, I2CQue *ptI2CQue, QUEUE_HANDLE Channel);
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static unsigned char ucCheckTxI2C = 0;                                   // {2}
#if defined I2C_SLAVE_MODE
    static unsigned char *ptrMessage[NUMBER_I2C] = {0};
    static unsigned char ucMessageLength[NUMBER_I2C] = {0};
    static int (*fnI2C_SlaveCallback[NUMBER_I2C])(int iChannel, unsigned char *ptrDataByte, int iType) = {0};
#endif
#if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
    static unsigned short usDMATriggerSourceTx[NUMBER_I2C] = {0};
    static unsigned short usDMATriggerSourceRx[NUMBER_I2C] = {0};
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

extern I2CQue *I2C_rx_control[NUMBER_I2C];
extern I2CQue *I2C_tx_control[NUMBER_I2C];

#if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA               // DMA support on transmission
// DMA channel assignments for each I2C transmitter
//
static const unsigned char I2C_DMA_TX_CHANNEL[NUMBER_I2C] = {
    DMA_I2C0_TX_CHANNEL, 
    #if (NUMBER_I2C) > 1
    DMA_I2C1_TX_CHANNEL,
    #endif
    #if (NUMBER_I2C) > 2
    DMA_I2C2_TX_CHANNEL, 
    #endif
    #if (NUMBER_I2C) > 3
    DMA_I2C3_TX_CHANNEL,
    #endif
};

// DMA channel assignments for each I2C receiver
//
static const unsigned char I2C_DMA_RX_CHANNEL[I2C_AVAILABLE] = {
    DMA_I2C0_RX_CHANNEL,
    #if (NUMBER_I2C) > 1
    DMA_I2C1_RX_CHANNEL,
    #endif
    #if (NUMBER_I2C) > 2
    DMA_I2C2_RX_CHANNEL,
    #endif
    #if (NUMBER_I2C) > 3
    DMA_I2C3_RX_CHANNEL,
    #endif
};

// DMA channel interrupt priority assignments for each I2C transmitter
//
static const unsigned char I2C_DMA_TX_INT_PRIORITY[NUMBER_I2C] = {
    DMA_I2C0_TX_INT_PRIORITY, 
    #if (NUMBER_I2C) > 1
    DMA_I2C1_TX_INT_PRIORITY, 
    #endif
    #if (NUMBER_I2C) > 2
    DMA_I2C2_TX_INT_PRIORITY, 
    #endif
    #if (NUMBER_I2C) > 3
    DMA_I2C3_TX_INT_PRIORITY,
    #endif
};

// DMA channel interrupt priority assignments for each I2C receiver
//
static const unsigned char I2C_DMA_RX_INT_PRIORITY[NUMBER_I2C] = {
    DMA_I2C0_RX_INT_PRIORITY,
    #if (NUMBER_I2C) > 1
    DMA_I2C1_RX_INT_PRIORITY,
    #endif
    #if (NUMBER_I2C) > 2
    DMA_I2C2_RX_INT_PRIORITY,
    #endif
    #if (NUMBER_I2C) > 3
    DMA_I2C3_RX_INT_PRIORITY,
    #endif
};
#endif

/* =================================================================== */
/*                        I2C Interrupt Handlers                       */
/* =================================================================== */


//#define MAX_EVENTS 200                                                 // comment this in to enable event logging (mainly used to determine behaviour in double-buffered mode in order to find workarounds for silicon issues)
#if defined MAX_EVENTS
unsigned char ucTemp[MAX_EVENTS] = {0};
int iEventCounter = 0;

static void fnLogEvent(unsigned char ucLocation, unsigned char ucValue)
{
    if (iEventCounter < (MAX_EVENTS - 3)) {
        ucTemp[iEventCounter++] = ucLocation;
        ucTemp[iEventCounter++] = ucValue;
        ucTemp[iEventCounter++] = '-';
    }
}
#else
    #define fnLogEvent(x, y)                                             // dummy when logging is disabled
#endif

#if defined I2C_SLAVE_MODE
    #if defined I2C_SLAVE_RX_BUFFER
static int fnSaveRx(I2CQue *ptrRxControl, unsigned char ucRxByte)
{
    fnLogEvent('r', ucRxByte);
    if (ptrRxControl->I2C_queue.chars < ptrRxControl->I2C_queue.buf_length) { // ensure there is space in the input buffer
        *ptrRxControl->I2C_queue.put = ucRxByte;                         // save the received byte
        ptrRxControl->I2C_queue.chars++;                                 // increment the character count
        if (++(ptrRxControl->I2C_queue.put) >= ptrRxControl->I2C_queue.buffer_end) { // handle circular input buffer
            ptrRxControl->I2C_queue.put = ptrRxControl->I2C_queue.QUEbuffer;
        }
        return 0;                                                        // received byte was saved
    }
    else {
        return -1;                                                       // the received byte wasn't saved
    }
}
    #endif

static unsigned char fnGetTxByte(int iChannel, I2CQue *ptrTxControl, int iType)
{
    #if defined I2C_SLAVE_TX_BUFFER
    int iResult = I2C_SLAVE_BUFFER;
    #endif
    unsigned char ucByte = 0xff;                                         // default in case there is no defined data to return
    if (fnI2C_SlaveCallback[iChannel] != 0) {                            // if there is a callback
        #if defined I2C_SLAVE_TX_BUFFER
        iResult = fnI2C_SlaveCallback[iChannel](iChannel, &ucByte, iType);
        #else
        fnI2C_SlaveCallback[iChannel](iChannel, &ucByte, iType);
        #endif
    }
    #if defined I2C_SLAVE_TX_BUFFER
    if (I2C_SLAVE_BUFFER == iResult) {                                   // extract the data from the buffer
        if (ptrTxControl != 0) {
            if (ptrTxControl->I2C_queue.chars != 0) {                    // if there is data in the queue
                ucByte = *(ptrTxControl->I2C_queue.get);                 // the next byte to send is taken from the queue
                if (++(ptrTxControl->I2C_queue.get) >= ptrTxControl->I2C_queue.buffer_end) { // handle the circular buffer
                    ptrTxControl->I2C_queue.get = ptrTxControl->I2C_queue.QUEbuffer;
                }
                ptrTxControl->I2C_queue.chars--;
            }
        }
    }
    #endif
    return ucByte;                                                       // prepare the byte to be returned
}
#endif

#if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA               // DMA support
// Common DMA tx interrupt handler
//
static void _i2c_tx_dma_Interrupt(KINETIS_I2C_CONTROL *ptrI2C, int iChannel)
{
    I2CQue *ptrTxControl = I2C_tx_control[iChannel];
    #if defined _WINDOWS
    fnSimI2C_devices(iChannel, I2C_TX_DATA, (unsigned char)(ptrI2C->I2C_D)); // {7}
    #endif
    #if defined I2C_2_BYTE_LENGTH
    ptrTxControl->usPresentLen -= ptrTxControl->dma_length;
    #else
    ptrTxControl->ucPresentLen -= ptrTxControl->dma_length;
    #endif
    ptrTxControl->I2C_queue.get += ptrTxControl->dma_length;
    if (ptrTxControl->I2C_queue.get >= ptrTxControl->I2C_queue.buffer_end) {
        ptrTxControl->I2C_queue.get -= ptrTxControl->I2C_queue.buf_length;
    }
    #if defined I2C_2_BYTE_LENGTH
    if (ptrTxControl->usPresentLen == 0)
    #else
    if (ptrTxControl->ucPresentLen == 0)
    #endif
    {                                                                    // if all data transmitted
        fnI2C_Handler(ptrI2C, iChannel);
    }
    else {                                                               // transmit remaining (after circular buffer wrap-around)
        fnStartI2C_TxDMA(ptrI2C, ptrTxControl, iChannel);
    #if defined _WINDOWS
        fnSimI2C_devices(iChannel, I2C_TX_DATA, (unsigned char)(ptrI2C->I2C_D)); // {7}
    #endif
    }
}

static void _i2c0_tx_dma_Interrupt(void)
{
    _i2c_tx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C0_BLOCK, 0);
}

    #if NUMBER_I2C > 1
static void _i2c1_tx_dma_Interrupt(void)
{
    _i2c_tx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C1_BLOCK, 1);
}
    #endif
    #if NUMBER_I2C > 2
static void _i2c2_tx_dma_Interrupt(void)
{
    _i2c_tx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C2_BLOCK, 2);
}
    #endif
    #if NUMBER_I2C > 3
static void _i2c3_tx_dma_Interrupt(void)
{
    _i2c_tx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C3_BLOCK, 3);
}
    #endif

static void(*_i2c_tx_dma_Interrupts[NUMBER_I2C])(void) = {
    _i2c0_tx_dma_Interrupt,
    #if NUMBER_I2C > 1
    _i2c1_tx_dma_Interrupt,
    #endif
    #if NUMBER_I2C > 2
    _i2c2_tx_dma_Interrupt,
    #endif
    #if NUMBER_I2C > 3
    _i2c3_tx_dma_Interrupt,
    #endif
};

// Common DMA tx interrupt handler
//
static void _i2c_rx_dma_Interrupt(KINETIS_I2C_CONTROL *ptrI2C, int iChannel)
{
    I2CQue *ptrTxControl = I2C_tx_control[iChannel];
    I2CQue *ptrRxControl = I2C_rx_control[iChannel];
    #if defined I2C_2_BYTE_LENGTH
    ptrTxControl->usPresentLen -= ptrTxControl->dma_length;
    #else
    ptrTxControl->ucPresentLen -= ptrTxControl->dma_length;
    #endif
    ptrRxControl->I2C_queue.put += ptrTxControl->dma_length;
    if (ptrRxControl->I2C_queue.put >= ptrRxControl->I2C_queue.buffer_end) {
        ptrRxControl->I2C_queue.put -= ptrRxControl->I2C_queue.buf_length;
    }
    ptrRxControl->I2C_queue.chars += ptrTxControl->dma_length;
    #if defined I2C_2_BYTE_LENGTH
    if (ptrTxControl->usPresentLen <= 1)
    #else
    if (ptrTxControl->ucPresentLen <= 1)
    #endif
    {                                                                    // if all data received
    #if defined I2C_2_BYTE_LENGTH
        ptrTxControl->usPresentLen = 0;
    #else
        ptrTxControl->ucPresentLen = 0;
    #endif
        ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MSTA | I2C_TXAK);     // we don't acknowledge last byte
        fnLogEvent('J', ptrI2C->I2C_C1);
    #if defined _WINDOWS
        ptrI2C->I2C_D = fnSimI2C_devices(iChannel, I2C_RX_DATA, ptrI2C->I2C_D); // {7} simulate final read and interrupt
        ptrI2C->I2C_S |= I2C_IIF;
        iInts |= (I2C_INT0 << iChannel);
    #endif
    }
    else {                                                               // receive remaining (after circular buffer wrap-around)
    #if defined I2C_2_BYTE_LENGTH
        ptrTxControl->usPresentLen++;
    #else
        ptrTxControl->ucPresentLen++;
    #endif
        fnStartI2C_RxDMA(ptrI2C, ptrTxControl, iChannel);
    #if defined I2C_2_BYTE_LENGTH
        ptrTxControl->usPresentLen--;
    #else
        ptrTxControl->ucPresentLen--;
    #endif
    }
}

static void _i2c0_rx_dma_Interrupt(void)
{
    _i2c_rx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C0_BLOCK, 0);
}

#if NUMBER_I2C > 1
static void _i2c1_rx_dma_Interrupt(void)
{
    _i2c_rx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C1_BLOCK, 1);
}
#endif
#if NUMBER_I2C > 2
static void _i2c2_rx_dma_Interrupt(void)
{
    _i2c_rx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C2_BLOCK, 2);
}
#endif
#if NUMBER_I2C > 3
static void _i2c3_rx_dma_Interrupt(void)
{
    _i2c_rx_dma_Interrupt((KINETIS_I2C_CONTROL *)I2C3_BLOCK, 3);
}
#endif


static void(*_i2c_rx_dma_Interrupts[NUMBER_I2C])(void) = {
    _i2c0_rx_dma_Interrupt,
#if NUMBER_I2C > 1
    _i2c1_rx_dma_Interrupt,
#endif
#if NUMBER_I2C > 2
    _i2c2_rx_dma_Interrupt,
#endif
#if NUMBER_I2C > 3
    _i2c3_rx_dma_Interrupt,
#endif
};

static void fnStartI2C_TxDMA(KINETIS_I2C_CONTROL *ptrI2C, I2CQue *ptI2CQue, QUEUE_HANDLE Channel)
{
    QUEUE_TRANSFER max_linear_data = (ptI2CQue->I2C_queue.buffer_end - ptI2CQue->I2C_queue.get);
    #if defined I2C_2_BYTE_LENGTH
    ptI2CQue->dma_length = ptI2CQue->usPresentLen;
    #else
    ptI2CQue->dma_length = ptI2CQue->ucPresentLen;
    #endif
    if (ptI2CQue->dma_length > max_linear_data) {
    #if defined I2C_2_BYTE_LENGTH
        ptI2CQue->dma_length = max_linear_data;
    #else
        ptI2CQue->dma_length = (unsigned char)max_linear_data;
    #endif
    }
    #if defined SUPPORT_LOW_POWER
    PROTECTED_SET_VARIABLE(ulPeripheralNeedsClock, (I2C0_CLK_REQUIRED << Channel)); // mark that stop mode should be avoided until the I2C activity has completed
    #endif
    fnConfigDMA_buffer(I2C_DMA_TX_CHANNEL[Channel], usDMATriggerSourceTx[Channel], ptI2CQue->dma_length, ptI2CQue->I2C_queue.get, (void *)&(ptrI2C->I2C_D), (DMA_BYTES | DMA_DIRECTION_OUTPUT | DMA_SINGLE_CYCLE), _i2c_tx_dma_Interrupts[Channel], I2C_DMA_TX_INT_PRIORITY[Channel]);
    ptrI2C->I2C_C1 = (I2C_MTX | I2C_MSTA | I2C_IIEN | I2C_IEN | I2C_DMAEN); // enable DMA
    fnDMA_BufferReset(I2C_DMA_TX_CHANNEL[Channel], DMA_BUFFER_START);    // start
    #if defined _WINDOWS                                                 // simulation
    iDMA |= (DMA_CONTROLLER_0 << I2C_DMA_TX_CHANNEL[Channel]);           // activate first DMA request
    #endif
}

static void fnStartI2C_RxDMA(KINETIS_I2C_CONTROL *ptrI2C, I2CQue *ptI2CQue, QUEUE_HANDLE Channel)
{
    QUEQUE *ptrRxQueue = &I2C_rx_control[Channel]->I2C_queue;
    QUEUE_TRANSFER max_linear_data = (ptrRxQueue->buffer_end - ptrRxQueue->put);
    #if defined I2C_2_BYTE_LENGTH
    ptI2CQue->dma_length = (ptI2CQue->usPresentLen - 1);                 // the final read is not performed by DMA since the acknowledge need to be removed
    #else
    ptI2CQue->dma_length = (ptI2CQue->ucPresentLen - 1);
    #endif
    if (ptI2CQue->dma_length > max_linear_data) {
    #if defined I2C_2_BYTE_LENGTH
        ptI2CQue->dma_length = max_linear_data;
    #else
        ptI2CQue->dma_length = (unsigned char)max_linear_data;
    #endif
    }
    fnConfigDMA_buffer(I2C_DMA_RX_CHANNEL[Channel], usDMATriggerSourceRx[Channel], ptI2CQue->dma_length, (void *)&(ptrI2C->I2C_D), ptrRxQueue->put, (DMA_BYTES | DMA_DIRECTION_INPUT | DMA_SINGLE_CYCLE), _i2c_rx_dma_Interrupts[Channel], I2C_DMA_RX_INT_PRIORITY[Channel]);
    #if defined SUPPORT_LOW_POWER
    PROTECTED_SET_VARIABLE(ulPeripheralNeedsClock, (I2C0_CLK_REQUIRED << Channel)); // mark that stop mode should be avoided until the I2C activity has completed
    #endif
    ptrI2C->I2C_C1 = (I2C_MSTA | I2C_IIEN | I2C_IEN | I2C_DMAEN);        // enable DMA
    fnDMA_BufferReset(I2C_DMA_RX_CHANNEL[Channel], DMA_BUFFER_START);    // start
    #if defined _WINDOWS                                                 // simulation
    iDMA |= (DMA_CONTROLLER_0 << I2C_DMA_RX_CHANNEL[Channel]);           // activate first DMA request
    #endif
}
#endif

// General I2C interrupt handler
//
static void fnI2C_Handler(KINETIS_I2C_CONTROL *ptrI2C, int iChannel)
{
    I2CQue *ptrTxControl = I2C_tx_control[iChannel];
    #if defined _WINDOWS
    int int_id = irq_I2C0_ID;
        #if NUMBER_I2C > 1
    if (iChannel == 1) {
            #if defined irq_I2C1_EXTENDED_ID                             // I2C1 interrupt uses INTMUX
        int_id = irq_I2C1_EXTENDED_ID;
            #else
        int_id = irq_I2C1_ID;
            #endif
    }
        #endif
        #if NUMBER_I2C > 2
    else if (iChannel == 2) {
        int_id = irq_I2C2_ID;
    }
        #endif
        #if NUMBER_I2C > 3
    if (iChannel == 3) {
        int_id = irq_I2C3_ID;
    }
        #endif
    #endif

    #if defined I2C_SLAVE_MODE                                           // slave mode support
    if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_INT) != 0) {                      // if the slave has enabled start/stop condition interrupt(s)
        if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_STOPF) != 0) {                // if a stop condition has been detected
            fnLogEvent('e', ptrI2C->I2C_FLT);
            fnLogEvent('1', ptrI2C->I2C_S);
            ptrI2C->I2C_FLT |= I2C_FLT_FLT_STOPF;                        // clear the stop flag (write '1' to clear) [note that if the start flag is also set, it is also cleared by this action]
        #if defined _WINDOWS
            ptrI2C->I2C_FLT &= ~I2C_FLT_FLT_STOPF;
        #endif
            fnLogEvent('2', ptrI2C->I2C_FLT);
            WRITE_ONE_TO_CLEAR(ptrI2C->I2C_S, I2C_IIF);                  // clear the interrupt flag (write '1' to clear)
        #if defined _WINDOWS
            ptrI2C->I2C_S = 0;
        #endif
            if ((ptrTxControl->ucState & RX_ACTIVE) != 0) {              // end of a reception from master so terminate reception buffer
                int iResult = I2C_SLAVE_BUFFER;
                if (fnI2C_SlaveCallback[iChannel] != 0) {                // if there is a callback
                    iResult = fnI2C_SlaveCallback[iChannel](iChannel, 0, I2C_SLAVE_WRITE_COMPLETE);
                }
                if (iResult == I2C_SLAVE_BUFFER) {
                    I2CQue *ptrRxControl = I2C_rx_control[iChannel];
                    *ptrMessage[iChannel] = ucMessageLength[iChannel];   // enter the length of the message into the input buffer
                    fnLogEvent('3', ucMessageLength[iChannel]);
                    ptrRxControl->msgs++;                                // increment the message count
                    if (ptrRxControl->wake_task != 0) {                  // wake up the receiver task if desired
                        uTaskerStateChange(ptrRxControl->wake_task, UTASKER_ACTIVATE); // wake up owner task
                    }
                }
            }
            ptrTxControl->ucState &= ~(RX_ACTIVE);                       // idle again
            if ((ptrI2C->I2C_S & (I2C_IAAS | I2C_TCF)) != (I2C_IAAS | I2C_TCF)) { // {5} if the slave is already addressed we allow the reception handling to continue since we will have cleared the reception interrupt
                return;
            }
        }
            #if defined DOUBLE_BUFFERED_I2C || defined I2C_START_CONDITION_INTERRUPT
        else if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_STARTF) != 0) {          // the start condition interrupt must be handled by double-buffered devices
            fnLogEvent('!', ptrI2C->I2C_FLT);
            fnLogEvent('1', ptrI2C->I2C_S);
            ptrI2C->I2C_FLT |= I2C_FLT_FLT_STARTF;                       // clear the start flag (write '1' to clear)
            WRITE_ONE_TO_CLEAR(ptrI2C->I2C_S, I2C_IIF);                  // clear the interrupt flag (write '1' to clear)
            fnLogEvent('2', ptrI2C->I2C_FLT);
                #if defined _WINDOWS
            ptrI2C->I2C_S = 0;
            ptrI2C->I2C_FLT &= ~I2C_FLT_FLT_STARTF;
                #endif

            // Master only
            //
            if (ptrI2C->I2C_F != 0) {                                    // if master mode is in operation
                ptrI2C->I2C_FLT = 0;                                     // disable further start/stop interrupts
                fnSendSlaveAddress(ptrTxControl, iChannel, ptrI2C);      // a repeated start has just been sent so we now continue with the slave address (this cannot be prepared before the repeated start has been sent)
                return;
            }
            else if ((ptrI2C->I2C_S & (I2C_IAAS | I2C_TCF)) != (I2C_IAAS | I2C_TCF)) { // {6} if the slave is already addressed and the interrupt is pending we allow the reception handling to continue since we will have cleared the reception interru�t
                return;                                                  // otherwise return
            }
        }
            #endif
    }
    #elif defined DOUBLE_BUFFERED_I2C                                    // master mode only
    if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_INT) != 0) {                      // if the master has enabled start/stop interrupt
        if ((ptrI2C->I2C_FLT & I2C_FLT_FLT_STARTF) != 0) {               // the start condition interrupt must be handled by double-buffered master devices after sending a repeated start
            ptrI2C->I2C_FLT |= I2C_FLT_FLT_STARTF;                       // clear the start flag
            ptrI2C->I2C_S = I2C_IIF;                                     // clear the interrupt flag
            ptrI2C->I2C_FLT = 0;                                         // disable further start/stop interrupts
            fnSendSlaveAddress(ptrTxControl, iChannel, ptrI2C);          // a repeated start has just been sent so we now continue with the slave address (this cannot be prepared before the repeated start has been completed)
            return;
        }
    }
    #endif
    WRITE_ONE_TO_CLEAR_INTERRUPT(ptrI2C->I2C_S, I2C_IIF, int_id);        // clear the interrupt flag (write '1' to clear)
    #if defined I2C_SLAVE_MODE
    if (ptrI2C->I2C_F == 0) {                                            // if we are slave
        I2CQue *ptrRxControl = I2C_rx_control[iChannel];
        fnLogEvent('S', ptrI2C->I2C_S);
        if ((ptrI2C->I2C_S & I2C_IAAS) != 0) {                           // if being addressed as a slave
            fnLogEvent('A', ptrI2C->I2C_S);
            if ((ptrTxControl->ucState & RX_ACTIVE) != 0) {              // {4} if a read or write follows a write and repeated start
                if (fnI2C_SlaveCallback[iChannel] != 0) {                // if there is a callback
                    fnI2C_SlaveCallback[iChannel](iChannel, 0, I2C_SLAVE_WRITE_COMPLETE);
                }
            }
            if ((ptrI2C->I2C_S & I2C_SRW) != 0) {                        // if the master is addressing for a read
                ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MTX);         // set transmit mode and clear the IAAS flag (a write to I2C_C1 clears IAAS)
        #if defined _WINDOWS
                ptrI2C->I2C_S &= ~I2C_IAAS;
        #endif
                ptrTxControl->ucState |= TX_ACTIVE;                      // mark that we are transmitting because the master is reading
                fnLogEvent('R', ptrI2C->I2C_S);
                ptrI2C->I2C_D = fnGetTxByte(iChannel, ptrTxControl, I2C_SLAVE_ADDRESSED_FOR_READ); // prepare the byte to be read
            }
            else {                                                       // master is addressing for a write
        #if defined I2C_SLAVE_RX_BUFFER
                int iResult = I2C_SLAVE_BUFFER;
        #endif
                unsigned char ucAddress;
                ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN);                   // write to C1 in order to clear the IAAS flag - remain in receive mode (a write to I2C_C1 clears IAAS)
        #if defined _WINDOWS
                ptrI2C->I2C_S &= ~I2C_IAAS;
        #endif
                fnLogEvent('W', ptrI2C->I2C_S);
                ptrTxControl->ucState |= RX_ACTIVE;                      // mark that we are receiving
                ucAddress = ptrI2C->I2C_D;                               // dummy read (this reads out address as addressed)
                ptrMessage[iChannel] = ptrRxControl->I2C_queue.put;      // remember the location used to save the message length to
                if (fnI2C_SlaveCallback[iChannel] != 0) {                // if there is a callback
        #if defined I2C_SLAVE_RX_BUFFER
                    iResult = fnI2C_SlaveCallback[iChannel](iChannel, &ucAddress, I2C_SLAVE_ADDRESSED_FOR_WRITE);
        #else
                    fnI2C_SlaveCallback[iChannel](iChannel, &ucAddress, I2C_SLAVE_ADDRESSED_FOR_WRITE);
        #endif
                }
        #if defined I2C_SLAVE_RX_BUFFER
                if (iResult == I2C_SLAVE_BUFFER) {
                    fnSaveRx(ptrRxControl, 0);                           // put a dummy value into the inputs buffer, which is later used for the message length
                }
        #endif
                ucMessageLength[iChannel] = 0;                           // reset the present message length counter
            }
        }
        else {                                                           // not being addressed as slave
            if ((ptrTxControl->ucState & TX_ACTIVE) != 0) {              // if transmitting
                if ((ptrI2C->I2C_S & I2C_RXACK) != 0) {                  // no ack means that this is the final byte
                    fnLogEvent('a', ptrI2C->I2C_S);
                    ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN);               // switch to receive mode
                    fnLogEvent('0', ptrI2C->I2C_S);
                    fnLogEvent('1', ptrI2C->I2C_D);
                    (void)ptrI2C->I2C_D;                                 // dummy read
                    fnGetTxByte(iChannel, ptrTxControl, I2C_SLAVE_READ_COMPLETE); // final byte has been read
                    ptrTxControl->ucState &= ~(TX_ACTIVE);
                }
                else {
                    fnLogEvent('w', ptrI2C->I2C_S);
                    ptrI2C->I2C_D = fnGetTxByte(iChannel, ptrTxControl, I2C_SLAVE_READ); // prepare the byte to be read
                }
            }
            else {                                                       // read in progress
        #if defined I2C_SLAVE_RX_BUFFER
                int iResult = I2C_SLAVE_BUFFER;
        #endif
                unsigned char ucRxData = ptrI2C->I2C_D;
                if (fnI2C_SlaveCallback[iChannel] != 0) {                // if there is a callback
        #if defined I2C_SLAVE_RX_BUFFER
                    iResult = fnI2C_SlaveCallback[iChannel](iChannel, &ucRxData, I2C_SLAVE_WRITE);
        #else
                    fnI2C_SlaveCallback[iChannel](iChannel, &ucRxData, I2C_SLAVE_WRITE);
        #endif
                       
                }
        #if defined I2C_SLAVE_RX_BUFFER
                if (iResult == I2C_SLAVE_BUFFER) {
                    if (fnSaveRx(ptrRxControl, ucRxData) >= 0) {         // save the received byte to the input buffer
                        ucMessageLength[iChannel]++;                     // increment the present message length
                    }
                }
        #endif
            }
        }
        return;                                                          // slave mode interrupt handling completed
    }
    #endif
    fnLogEvent('M', ptrI2C->I2C_S);
    if ((ptrTxControl->ucState & RX_ACTIVE) != 0) {                      // if the processor is reading from a slave
        I2CQue *ptrRxControl = I2C_rx_control[iChannel];
        register int iFirstRead = ((ptrI2C->I2C_C1 & I2C_MTX) != 0);
        fnLogEvent('R', ptrI2C->I2C_C1);
    #if defined I2C_2_BYTE_LENGTH
        if (ptrTxControl->usPresentLen == 1)
    #else
        if (ptrTxControl->ucPresentLen == 1)
    #endif
        {                                                                // last byte to be read?
            ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MSTA | I2C_TXAK); // we don't acknowledge last byte
            fnLogEvent('L', ptrI2C->I2C_C1);
        }
        else if
    #if defined I2C_2_BYTE_LENGTH
            (ptrTxControl->usPresentLen == 0)
    #else
            (ptrTxControl->ucPresentLen == 0)
    #endif
        {                                                                // we have completed the read
            ptrI2C->I2C_C1 = (I2C_IEN | I2C_TXAK);                       // send stop condition and disable interrupts
            ptrTxControl->ucState &= ~(TX_WAIT | TX_ACTIVE | RX_ACTIVE);
            ptrRxControl->msgs++;                                        // increment complete message counter
            if (ptrRxControl->wake_task != 0) {                          // wake up the receiver task if desired
                uTaskerStateChange(ptrRxControl->wake_task, UTASKER_ACTIVATE); // wake up owner task
            }
            fnLogEvent('S', ptrI2C->I2C_C1);
        }
        else {
            ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MSTA);            // ensure we acknowledge multi-byte reads
            fnLogEvent('a', ptrI2C->I2C_C1);
        }
        
        if (iFirstRead != 0) {                                           // have we just sent the slave address?
            fnLogEvent('d', ptrI2C->I2C_D);
    #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
        #if defined I2C_2_BYTE_LENGTH
            if (ptrTxControl->usPresentLen != 0)
        #else
            if (ptrTxControl->ucPresentLen != 0)
        #endif
            {
                if (usDMATriggerSourceRx[iChannel] != 0) {               // if more than 1 byte of data is to be read and rx dma mode is enabled
                    fnStartI2C_RxDMA(ptrI2C, ptrTxControl, iChannel);
                    return;
                }
            }
    #endif
            (void)ptrI2C->I2C_D;                                         // dummy read
        }
        else {
            fnLogEvent('R', ptrI2C->I2C_C1);
            *ptrRxControl->I2C_queue.put++ = ptrI2C->I2C_D;              // read the byte
            ptrRxControl->I2C_queue.chars++;                             // and put it into the rx buffer
            if (ptrRxControl->I2C_queue.put >= ptrRxControl->I2C_queue.buffer_end) { // handle circular input buffer
                ptrRxControl->I2C_queue.put = ptrRxControl->I2C_queue.QUEbuffer;
            }
        }
    #if defined I2C_2_BYTE_LENGTH
        if (ptrTxControl->usPresentLen != 0)
    #else
        if (ptrTxControl->ucPresentLen != 0)
    #endif
        {
    #if defined I2C_2_BYTE_LENGTH
            ptrTxControl->usPresentLen--;
    #else
            ptrTxControl->ucPresentLen--;
    #endif
    #if defined _WINDOWS
            ptrI2C->I2C_D = fnSimI2C_devices(iChannel, I2C_RX_DATA, ptrI2C->I2C_D); // {7} simulate the interrupt directly
            ptrI2C->I2C_S |= I2C_IIF;
            iInts |= (I2C_INT0 << iChannel);
    #endif
        }
        else {                                                           // read sequence complete so continue with next write if something is waiting
            if (ptrTxControl->I2C_queue.chars != 0) {
                fnLogEvent('P', 0);
                fnTxI2C(ptrTxControl, iChannel);                         // we have another message to send so we can send a repeated start condition
            }
        }
        return;
    }
    else if 
    #if defined I2C_2_BYTE_LENGTH
        (ptrTxControl->usPresentLen != 0)
    #else
        (ptrTxControl->ucPresentLen != 0)
    #endif
    {                                                                    // TX_ACTIVE - send next byte if available
        fnLogEvent('X', *ptrTxControl->I2C_queue.get);
    #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
        #if defined I2C_2_BYTE_LENGTH
        if (ptrTxControl->usPresentLen != 0)
        #else
        if (ptrTxControl->ucPresentLen != 0)
        #endif
        {
            if (usDMATriggerSourceTx[iChannel] != 0) {                   // if more than 1 byte of data is to be sent and tx dma mode is enabled
                fnStartI2C_TxDMA(ptrI2C, ptrTxControl, iChannel);
                return;
            }
        }
    #endif
    #if defined I2C_2_BYTE_LENGTH
        ptrTxControl->usPresentLen--;
    #else
        ptrTxControl->ucPresentLen--;
    #endif
        ptrI2C->I2C_D = *ptrTxControl->I2C_queue.get++;                  // send the next byte
        if (ptrTxControl->I2C_queue.get >= ptrTxControl->I2C_queue.buffer_end) { // handle the circular transmit buffer
            ptrTxControl->I2C_queue.get = ptrTxControl->I2C_queue.QUEbuffer;
        }
        #if defined _WINDOWS
        ptrI2C->I2C_S |= I2C_IIF;                                        // simulate the interrupt directly 
        fnSimI2C_devices(iChannel, I2C_TX_DATA, ptrI2C->I2C_D);          // {7}
        iInts |= (I2C_INT0 << iChannel);                                 // signal that an interrupt is to be generated
        #endif
    }
    else {                                                               // last byte in TX_ACTIVE
        if (ptrTxControl->I2C_queue.chars == 0) {                        // transmission complete
            ptrI2C->I2C_C1 = (I2C_IEN | I2C_MTX);                        // send stop condition and disable interrupts
            fnLogEvent('E', ptrI2C->I2C_C1);
            ptrTxControl->ucState &= ~(TX_WAIT | TX_ACTIVE | RX_ACTIVE);
            if (ptrTxControl->wake_task != 0) {                          // if a wakeup task has been defined for transmission termination
               uTaskerStateChange(ptrTxControl->wake_task, UTASKER_ACTIVATE); // wake up owner task since the transmission has terminated
            }
        }
        else {                                                           // a further message is queued for transmission
            fnLogEvent('p', 0);
            fnTxI2C(ptrTxControl, iChannel);                             // we have another message to send so we can send a repeated start condition
        }
    }
}

// I2C0 interrupt handler
//
static __interrupt void _I2C_Interrupt_0(void)                           // I2C0 interrupt
{
    fnI2C_Handler((KINETIS_I2C_CONTROL *)I2C0_BLOCK, 0);
}

    #if NUMBER_I2C > 1
// I2C1 interrupt handler
//
static __interrupt void _I2C_Interrupt_1(void)                           // I2C1 interrupt
{
    fnI2C_Handler((KINETIS_I2C_CONTROL *)I2C1_BLOCK, 1);
}
    #endif

    #if NUMBER_I2C > 2
// I2C2 interrupt handler
//
static __interrupt void _I2C_Interrupt_2(void)                           // I2C2 interrupt
{
    fnI2C_Handler((KINETIS_I2C_CONTROL *)I2C2_BLOCK, 2);
}
    #endif

    #if NUMBER_I2C > 3
// I2C3 interrupt handler
//
static __interrupt void _I2C_Interrupt_3(void)                           // I2C3 interrupt
{
    fnI2C_Handler((KINETIS_I2C_CONTROL *)I2C3_BLOCK, 3);
}
    #endif



/* =================================================================== */
/*                   I2C Transmission Initiation                       */
/* =================================================================== */

// Send a first byte to I2C bus
//
extern void fnTxI2C(I2CQue *ptI2CQue, QUEUE_HANDLE Channel)
{
    KINETIS_I2C_CONTROL *ptrI2C;
    #if I2C_AVAILABLE > 1
    if (Channel == 1) {
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C1_BLOCK;
    }
        #if I2C_AVAILABLE > 2
    else if (Channel == 2) {
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C2_BLOCK;
    }
        #endif
        #if I2C_AVAILABLE > 3
    else if (Channel == 3) {
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C3_BLOCK;
    }
        #endif
    else {
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C0_BLOCK;
    }
    #else
    ptrI2C = (KINETIS_I2C_CONTROL *)I2C0_BLOCK;
    #endif
    #if defined I2C_2_BYTE_LENGTH
    ptI2CQue->usPresentLen = *ptI2CQue->I2C_queue.get++;                 // get the length of this transaction (MSB)
    ptI2CQue->usPresentLen <<= 8;
    if (ptI2CQue->I2C_queue.get >= ptI2CQue->I2C_queue.buffer_end) {     // handle circular buffer
        ptI2CQue->I2C_queue.get = ptI2CQue->I2C_queue.QUEbuffer;
    }
    ptI2CQue->usPresentLen |= *ptI2CQue->I2C_queue.get++;
    #else
    ptI2CQue->ucPresentLen = *ptI2CQue->I2C_queue.get++;                 // get the length of this transaction
    #endif
    if (ptI2CQue->I2C_queue.get >= ptI2CQue->I2C_queue.buffer_end) {     // handle circular buffer
        ptI2CQue->I2C_queue.get = ptI2CQue->I2C_queue.QUEbuffer;
    }

    if ((ptI2CQue->ucState & TX_ACTIVE) != 0) {                          // restart since we are hanging a second telegram on to previous one
        fnLogEvent('*', ptrI2C->I2C_C1);
        ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MSTA | I2C_MTX | I2C_RSTA); // repeated start
    #if defined DOUBLE_BUFFERED_I2C
        ptrI2C->I2C_FLT = (I2C_FLT_FLT_STARTF | I2C_FLT_FLT_STOPF);      // clear flags
        ptrI2C->I2C_FLT = I2C_FLT_FLT_SSIE;                              // enable start/stop condition interrupt
        #if defined _WINDOWS
        ptrI2C->I2C_FLT |= I2C_FLT_FLT_STARTF;
        ptrI2C->I2C_S |= I2C_IIF;                                        // simulate the interrupt directly
        iInts |= (I2C_INT0 << Channel);                                  // signal that an interrupt is to be generated
        #endif
        return;
    #endif
    }
    else {                                                               // address to be sent on an idle bus
        if ((ucCheckTxI2C & (1 << Channel)) == 0) {                      // {2} on first use we check that the bus is not held in a busy state (can happen when a reset took place during an acknowledge period and the slave is holding the bus)
            ucCheckTxI2C |= (1 << Channel);                              // checked only once
            uEnable_Interrupt();
                fnConfigI2C_pins(Channel, 1);                            // check and configure pins for I2C use
            uDisable_Interrupt();
        }
        fnLogEvent('B', ptrI2C->I2C_S);
        while ((ptrI2C->I2C_S & I2C_IBB) != 0) {                         // wait until the stop has actually been sent to avoid a further transmission being started in the mean time
            if ((ptrI2C->I2C_S & I2C_IAL) != 0) {                        // {3} arbitration lost flag set
                ptrI2C->I2C_S = I2C_IAL;                                 // clear arbitration lost flag
                ptrI2C->I2C_C1 &= ~(I2C_IEN);                            // disable I2S to clear busy
                fnDelayLoop(100);                                        // short delay before re-enabling the I2C controller (without delay it doesn't generate any further interrupts)
                ptrI2C->I2C_C1 |= (I2C_IEN);                             // re-enable
            }
        }
        fnLogEvent('b', ptrI2C->I2C_S);
        ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MTX);                 // set transmit mode with interrupt enabled
        ptrI2C->I2C_C1 = (I2C_IEN | I2C_IIEN | I2C_MSTA | I2C_MTX);      // set master mode to cause start condition to be sent
    }
    fnSendSlaveAddress(ptI2CQue, Channel, ptrI2C);
}

static void fnSendSlaveAddress(I2CQue *ptI2CQue, QUEUE_HANDLE Channel, KINETIS_I2C_CONTROL *ptrI2C)
{
    register unsigned char ucAddress = *ptI2CQue->I2C_queue.get++;       // the slave address
    ptrI2C->I2C_D = ucAddress;                                           // send the slave address (this includes the rd/wr bit) - I2Cx_D
    fnLogEvent('?', ucAddress);
    if (ptI2CQue->I2C_queue.get >= ptI2CQue->I2C_queue.buffer_end) {     // handle circular buffer
        ptI2CQue->I2C_queue.get = ptI2CQue->I2C_queue.QUEbuffer;
    }

    if ((ucAddress & 0x01) != 0) {                                       // reading from the slave
        I2C_tx_control[Channel]->ucState |= (RX_ACTIVE | TX_ACTIVE);
    #if defined I2C_2_BYTE_LENGTH
        ptI2CQue->I2C_queue.chars -= 4;
    #else
        ptI2CQue->I2C_queue.chars -= 3;
    #endif
        fnLogEvent('g', (unsigned char)(ptI2CQue->I2C_queue.chars));
        I2C_rx_control[Channel]->wake_task = *ptI2CQue->I2C_queue.get++; // enter task to be woken when reception has completed
        if (ptI2CQue->I2C_queue.get >= ptI2CQue->I2C_queue.buffer_end) {
            ptI2CQue->I2C_queue.get = ptI2CQue->I2C_queue.QUEbuffer;     // handle circular buffer
        }
    }
    else {
        I2C_tx_control[Channel]->ucState |= (TX_ACTIVE);                 // writing to the slave
    #if defined I2C_2_BYTE_LENGTH
        ptI2CQue->I2C_queue.chars -= (ptI2CQue->usPresentLen + 1);       // the remaining queue content
    #else
        ptI2CQue->I2C_queue.chars -= (ptI2CQue->ucPresentLen + 1);       // the remaining queue content
    #endif
        fnLogEvent('h', (unsigned char)(ptI2CQue->I2C_queue.chars));
    }

    #if defined _WINDOWS
    ptrI2C->I2C_S |= I2C_IIF;                                            // simulate the interrupt directly
        #if defined DOUBLE_BUFFERED_I2C
    ptrI2C->I2C_FLT |= I2C_FLT_FLT_STARTF;
        #endif
    fnSimI2C_devices(Channel, I2C_ADDRESS, ptrI2C->I2C_D);               // {7}
    iInts |= (I2C_INT0 << Channel);                                      // signal that an interrupt is to be generated
    #endif
}

/* =================================================================== */
/*                         I2C Configuration                           */
/* =================================================================== */

// Initially the I2C pins were configured as inputs to allow an I2C bus lockup state to be detected
// - this routine checks for this state and generates clocks if needed to recover from it before setting I2C pin functions
//
static void fnConfigI2C_pins(QUEUE_HANDLE Channel, int iMaster)          // {2}
{
    if (Channel == 0)  {
    #if defined KINETIS_KE && defined I2C0_B
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, KE_PORTA_BIT2) == 0) {             // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE(A, KE_PORTB_BIT6, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT(A, KE_PORTB_BIT6, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        SIM_PINSEL0 |= SIM_PINSEL_I2C0PS;                                // remap the I2C pins
        _CONFIG_PERIPHERAL(B, 6,  (PB_6_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PB6 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 7,  (PB_7_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PB7 (alt. function 2)
    #elif defined KINETIS_KE
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT2) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE(A, KE_PORTA_BIT3, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT(A, KE_PORTA_BIT3, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 2,  (PA_2_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PA2 (alt. function 3)
        _CONFIG_PERIPHERAL(A, 3,  (PA_3_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PA3 (alt. function 3)
    #elif defined KINETIS_KV10 || defined KINETIS_KV11
        if (iMaster != 0) {
            while (_READ_PORT_MASK(C, PORTC_BIT7) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(C, PORTC_BIT6, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(C, PORTC_BIT6, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(C, 7,  (PC_7_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PC7 (alt. function 7)
        _CONFIG_PERIPHERAL(C, 6,  (PC_6_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PC6 (alt. function 7)
    #elif defined KINETIS_KV30
        if (iMaster != 0) {
            while (_READ_PORT_MASK(D, PORTD_BIT3) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(D, PORTD_BIT2, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(D, PORTD_BIT2, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(D, 3,  (PD_3_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PD3 (alt. function 7)
        _CONFIG_PERIPHERAL(D, 2,  (PD_2_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PCD2 (alt. function 7)
    #elif ((defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05) && defined I2C0_A_0)
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT4) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT3, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT3, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 4,  (PA_4_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PA4 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 3,  (PA_3_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PA3 (alt. function 2)
    #elif ((defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05) && defined I2C0_A_1)
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT3) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT4, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT4, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 3,  (PA_3_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PA3 (alt. function 3)
        _CONFIG_PERIPHERAL(A, 4,  (PA_4_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PA4 (alt. function 3)
    #elif (defined KINETIS_KL03 && defined I2C0_B_0)
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, PORTB_BIT1) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, PORTB_BIT0, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(B, PORTB_BIT0, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(B, 1,  (PB_1_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PB1 (alt. function 4)
        _CONFIG_PERIPHERAL(B, 0,  (PB_0_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PB0 (alt. function 4)
    #elif (defined KINETIS_KL03 && defined I2C0_A_2)
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT9) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT8, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT8, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 9,  (PA_9_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PA9 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 8,  (PA_8_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PA8 (alt. function 2)
    #elif defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, PORTB_BIT4) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, PORTB_BIT3, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(B, PORTB_BIT3, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(B, 4,  (PB_4_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PB4 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 3,  (PB_3_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PB3 (alt. function 2)
    #elif defined I2C0_B_LOW
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, PORTB_BIT1) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, PORTB_BIT0, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(B, PORTB_BIT0, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(B, 1,  (PB_1_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PB1 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 0,  (PB_0_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PB0 (alt. function 2)
    #elif defined KINETIS_KL82 && defined I2C0_ON_D_LOW
        if (iMaster != 0) {
            while (_READ_PORT_MASK(D, PORTD_BIT3) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(D, PORTD_BIT2, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(D, PORTD_BIT2, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(D, 3,  (PD_3_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PD3 (alt. function 7)
        _CONFIG_PERIPHERAL(D, 2,  (PD_2_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PD2 (alt. function 7)
    #elif defined I2C0_ON_D
        if (iMaster != 0) {
            while (_READ_PORT_MASK(D, PORTD_BIT9) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(D, PORTD_BIT8, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(D, PORTD_BIT8, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(D, 9,  (PD_9_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PD9 (alt. function 2)
        _CONFIG_PERIPHERAL(D, 8,  (PD_8_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PD8 (alt. function 2)
    #elif (defined KINETIS_K64 || defined KINETIS_KL17 || defined KINETIS_K24 || defined KINETIS_KL25 || defined KINETIS_KL26 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL43 || defined KINETIS_KL46 || defined KINETIS_K02) && defined I2C0_ON_E
        if (iMaster != 0) {
            while (_READ_PORT_MASK(E, PORTE_BIT25) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_HIGH(E, PORTE_BIT24, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_HIGH(E, PORTE_BIT24, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(E, 25, (PE_25_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PE25 (alt. function 5)
        _CONFIG_PERIPHERAL(E, 24, (PE_24_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PE24 (alt. function 5)
    #elif (defined KINETIS_K26 || defined KINETIS_K65 || defined KINETIS_K66 ||  defined KINETIS_K70) && defined I2C0_ON_E
        if (iMaster != 0) {
            while (_READ_PORT_MASK(E, PORTE_BIT18) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_HIGH(E, PORTE_BIT19, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_HIGH(E, PORTE_BIT19, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(E, 18, (PE_18_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PE18 (alt. function 4)
        _CONFIG_PERIPHERAL(E, 19, (PE_19_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PE19 (alt. function 4)
    #else
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, PORTB_BIT3) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, PORTB_BIT2, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(B, PORTB_BIT2, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(B, 3,  (PB_3_I2C0_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SDA on PB3 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 2,  (PB_2_I2C0_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C0_SCL on PB2 (alt. function 2)
    #endif
    #if defined KINETIS_KE
        I2C0_C1 = (I2C_IEN);                                      _SIM_PER_CHANGE; // enable I2C controller after checking bus so that the peripheral pin function becomes valie
    #endif
    }
    #if I2C_AVAILABLE > 1
    else if (Channel == 1)  {
        #if defined KINETIS_KE
            #if defined I2C1_ON_H
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, KE_PORTH_BIT3) == 0) {             // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE(B, KE_PORTH_BIT4, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT(B, KE_PORTH_BIT4, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        SIM_PINSEL1 |= SIM_PINSEL1_I2C1PS;                               // remap port
        _CONFIG_PERIPHERAL(B, 3,  (PH_3_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PH3 (alt. function 2)
        _CONFIG_PERIPHERAL(B, 4,  (PH_4_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PH4 (alt. function 2)
            #else
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, KE_PORTE_BIT0) == 0) {             // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE(B, KE_PORTE_BIT1, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT(B, KE_PORTE_BIT1, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        SIM_PINSEL1 &= ~SIM_PINSEL1_I2C1PS;
        _CONFIG_PERIPHERAL(B, 0,  (PE_0_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PE0 (alt. function 4)
        _CONFIG_PERIPHERAL(B, 1,  (PE_1_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PE1 (alt. function 4)
            #endif
        #elif (defined KINETIS_KL17 || defined KINETIS_KL27) && defined I2C1_ON_D
        if (iMaster != 0) {
            while (_READ_PORT_MASK(D, PORTD_BIT6) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(D, PORTD_BIT7, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(D, PORTD_BIT7, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(D, 6, (PD_6_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PD6 (alt. function 4)
        _CONFIG_PERIPHERAL(D, 7, (PD_7_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PD7 (alt. function 4)
        #elif defined KINETIS_KL02 && defined I2C1_A_1
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT3) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT4, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT4, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 3,  (PA_3_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PA3 (alt. function 3)
        _CONFIG_PERIPHERAL(A, 4,  (PA_4_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PA4 (alt. function 3)
        #elif defined KINETIS_KL02
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT9) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT8, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT8, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 9,  (PA_9_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PA9 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 8,  (PA_8_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PA8 (alt. function 2)
        #elif defined I2C1_ON_E
        if (iMaster != 0) {
            while (_READ_PORT_MASK(E, PORTE_BIT1) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(E, PORTE_BIT0, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(E, PORTE_BIT0, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(E, 0,  (PE_0_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PE0 (alt. function 6)
        _CONFIG_PERIPHERAL(E, 1,  (PE_1_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PE1 (alt. function 6)
        #else
        if (iMaster != 0) {
            while (_READ_PORT_MASK(C, PORTC_BIT11) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(C, PORTC_BIT10, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(C, PORTC_BIT10, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(C, 11, (PC_11_I2C1_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SDA on PC11 (alt. function 2)
        _CONFIG_PERIPHERAL(C, 10, (PC_10_I2C1_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C1_SCL on PC10 (alt. function 2)
        #endif
    }
    #endif
    #if I2C_AVAILABLE > 2
    else if (Channel == 2) {
        #if defined KINETIS_K80 && defined I2C2_ON_B
        if (iMaster != 0) {
            while (_READ_PORT_MASK(B, PORTB_BIT10) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(B, PORTB_BIT11, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(B, PORTB_BIT11, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(B, 10, (PB_10_I2C2_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SDA on PB10 (alt. function 4)
        _CONFIG_PERIPHERAL(B, 11, (PB_11_I2C2_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SCL on PB11 (alt. function 4)
        #elif defined KINETIS_K80 && defined I2C2_A_LOW                    // initially configure as input with pull-up
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT7) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT6, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT6, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 7, (PA_7_I2C2_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SDA on PA7 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 6, (PA_6_I2C2_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SCL on PA6 (alt. function 2)
        #elif defined I2C2_A_HIGH                                          // initially configure as input with pull-up
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT13) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT14, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT14, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 13, (PA_13_I2C2_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SDA on PA13 (alt. function 5)
        _CONFIG_PERIPHERAL(A, 14, (PA_14_I2C2_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SCL on PA14 (alt. function 5)
        #else
            #if defined KINETIS_K80
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT10) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT11, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT11, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 10, (PA_10_I2C2_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SDA on PA10 (alt. function 2)
        _CONFIG_PERIPHERAL(A, 11, (PA_11_I2C2_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SCL on PA11 (alt. function 2)
            #else
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT11) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT12, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT12, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 11, (PA_11_I2C2_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SDA on PA11 (alt. function 5)
        _CONFIG_PERIPHERAL(A, 12, (PA_12_I2C2_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C2_SCL on PA12 (alt. function 5)
            #endif
        #endif
    }
    #endif
    #if I2C_AVAILABLE > 3
    else {
        #if defined I2C3_ON_E
        if (iMaster != 0) {
            while (_READ_PORT_MASK(E, PORTE_BIT10) == 0) {               // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(E, PORTE_BIT11, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(E, PORTE_BIT11, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(E, 10, (PE_10_I2C3_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C3_SDA on PE10 (alt. function 4)
        _CONFIG_PERIPHERAL(E, 11, (PE_11_I2C3_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C3_SCL on PE11 (alt. function 4)
        #else
        if (iMaster != 0) {
            while (_READ_PORT_MASK(A, PORTA_BIT1) == 0) {                // if the SDA line is low we clock the SCL line to free it
                _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_LOW(A, PORTA_BIT2, 0, (PORT_ODE | PORT_PS_UP_ENABLE)); // set output '0'
                fnDelayLoop(10);
                _CONFIG_PORT_INPUT_FAST_LOW(A, PORTA_BIT2, (PORT_ODE | PORT_PS_UP_ENABLE));
                fnDelayLoop(10);
            }
        }
        _CONFIG_PERIPHERAL(A, 1, (PA_1_I2C3_SDA | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C3_SDA on PA1 (alt. function 4)
        _CONFIG_PERIPHERAL(A, 2, (PA_2_I2C3_SCL | PORT_ODE | PORT_PS_UP_ENABLE)); // I2C3_SCL on PA2 (alt. function 4)
        #endif
    }
    #endif
}

// Configure the I2C hardware
//
extern void fnConfigI2C(I2CTABLE *pars)
{
    KINETIS_I2C_CONTROL *ptrI2C;
    #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
    unsigned short usDMA_Trigger;
    #endif
    unsigned char ucSpeed;
    if (pars->Channel == 0) {                                            // I2C channel 0
        POWER_UP_ATOMIC(4, I2C0);                                        // enable clock to module
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C0_BLOCK;
    #if defined irq_LPI2C0_ID
        fnEnterInterrupt(irq_LPI2C0_ID, PRIORITY_I2C0, _I2C_Interrupt_0); // enter I2C0 interrupt handler
    #else
        fnEnterInterrupt(irq_I2C0_ID, PRIORITY_I2C0, _I2C_Interrupt_0);  // enter I2C0 interrupt handler
    #endif
    #if defined KINETIS_KE && defined I2C0_B                             // initially configure as inputs with pull-up
        _CONFIG_PORT_INPUT(A, (KE_PORTB_BIT6 | KE_PORTB_BIT7), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined KINETIS_KE
        _CONFIG_PORT_INPUT(A, (KE_PORTA_BIT2 | KE_PORTA_BIT3), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined KINETIS_KV10 || defined KINETIS_KV11
        _CONFIG_PORT_INPUT_FAST_LOW(C, (PORTC_BIT6 | PORTC_BIT7), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined KINETIS_KV30
        _CONFIG_PORT_INPUT_FAST_LOW(D, (PORTD_BIT2 | PORTD_BIT3), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif ((defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05) && (defined I2C0_A_0 || defined I2C0_A_1))
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT3 | PORTA_BIT4), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_KL02 && defined I2C0_A_0)
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT3 | PORTA_BIT4), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_KL03 && defined I2C0_B_0)
        _CONFIG_PORT_INPUT_FAST_LOW(B, (PORTB_BIT1 | PORTB_BIT0), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_KL03 && defined I2C0_A_2)
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT8 | PORTA_BIT9), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_KL02 || defined KINETIS_KL03 || defined KINETIS_KL04 || defined KINETIS_KL05)
        _CONFIG_PORT_INPUT_FAST_LOW(B, (PORTB_BIT3 | PORTB_BIT4), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined I2C0_B_LOW
        _CONFIG_PORT_INPUT_FAST_LOW(B, (PORTB_BIT1 | PORTB_BIT0), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined KINETIS_KL82 && defined I2C0_ON_D_LOW
        _CONFIG_PORT_INPUT_FAST_LOW(D, (PORTD_BIT3 | PORTD_BIT2), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif defined I2C0_ON_D
        _CONFIG_PORT_INPUT_FAST_LOW(D, (PORTD_BIT9 | PORTD_BIT8), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_K64 || defined KINETIS_KL17 || defined KINETIS_K24 || defined KINETIS_KL25 || defined KINETIS_KL26 || defined KINETIS_KL27 || defined KINETIS_KL28 || defined KINETIS_KL43 || defined KINETIS_KL46 || defined KINETIS_K02) && defined I2C0_ON_E
        _CONFIG_PORT_INPUT_FAST_HIGH(E, (PORTE_BIT25 | PORTE_BIT24), (PORT_ODE | PORT_PS_UP_ENABLE));
    #elif (defined KINETIS_K26 || defined KINETIS_K65 || defined KINETIS_K66 || defined KINETIS_K70) && defined I2C0_ON_E
        _CONFIG_PORT_INPUT_FAST_HIGH(E, (PORTE_BIT19 | PORTE_BIT18), (PORT_ODE | PORT_PS_UP_ENABLE));
    #else
        _CONFIG_PORT_INPUT_FAST_LOW(B, (PORTB_BIT3 | PORTB_BIT2), (PORT_ODE | PORT_PS_UP_ENABLE));
    #endif
    #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
        #if defined DMAMUX0_CHCFG_SOURCE_I2C0
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C0;
        #elif defined DMAMUX0_CHCFG_SOURCE_I2C0_3
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C0_3;
        #endif
    #endif
    }
    #if I2C_AVAILABLE > 1
    else if (pars->Channel == 1) {                                       // I2C channel 1
        POWER_UP_ATOMIC(4, I2C1);                                        // enable clock to module
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C1_BLOCK;
        #if defined irq_LPI2C1_ID
        fnEnterInterrupt(irq_LPI2C1_ID, PRIORITY_I2C1, _I2C_Interrupt_1);  // enter LPI2C1 interrupt handler
        #elif !defined irq_I2C1_ID && defined INTMUX0_AVAILABLE
        fnEnterInterrupt((irq_INTMUX0_0_ID + INTMUX_I2C1), INTMUX0_PERIPHERAL_I2C1, _I2C_Interrupt_1); // enter I2C1 interrupt handler based on INTMUX
        #else
        fnEnterInterrupt(irq_I2C1_ID, PRIORITY_I2C1, _I2C_Interrupt_1);  // enter I2C1 interrupt handler
        #endif
        #if defined KINETIS_KE                                           // initially configure as inputs with pull-up
            #if defined I2C1_ON_H
        _CONFIG_PORT_INPUT(B, (KE_PORTH_BIT3 | KE_PORTH_BIT4), (PORT_ODE | PORT_PS_UP_ENABLE));
            #else
        _CONFIG_PORT_INPUT(B, (KE_PORTE_BIT0 | KE_PORTE_BIT1), (PORT_ODE | PORT_PS_UP_ENABLE));
            #endif
        #elif (defined KINETIS_KL17 || defined KINETIS_KL27) && defined I2C1_ON_D
        _CONFIG_PORT_INPUT_FAST_LOW(D, (PORTD_BIT7 | PORTD_BIT6), (PORT_ODE | PORT_PS_UP_ENABLE));
        #elif defined KINETIS_KL02 && defined I2C1_A_1
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT3 | PORTA_BIT4), (PORT_ODE | PORT_PS_UP_ENABLE));
        #elif defined KINETIS_KL02
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT8 | PORTA_BIT9), (PORT_ODE | PORT_PS_UP_ENABLE));
        #elif defined I2C1_ON_E
        _CONFIG_PORT_INPUT_FAST_LOW(E, (PORTE_BIT1 | PORTE_BIT0), (PORT_ODE | PORT_PS_UP_ENABLE));
        #else
        _CONFIG_PORT_INPUT_FAST_LOW(C, (PORTC_BIT11 | PORTC_BIT10), (PORT_ODE | PORT_PS_UP_ENABLE));
        #endif
        #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
            #if defined DMAMUX0_CHCFG_SOURCE_I2C1
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C1;
            #elif defined DMAMUX0_CHCFG_SOURCE_I2C1_2
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C1_2;
            #endif
        #endif
    }
    #endif
    #if I2C_AVAILABLE > 2
    else if (pars->Channel == 2) {                                       // I2C channel 2
        POWER_UP_ATOMIC(1, I2C2);                                        // enable clock to module
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C2_BLOCK;
        #if !defined irq_I2C2_ID && defined INTMUX0_AVAILABLE
        fnEnterInterrupt((irq_INTMUX0_0_ID + INTMUX_I2C2), INTMUX0_PERIPHERAL_I2C2, _I2C_Interrupt_2); // enter I2C2 interrupt handler
        #else
        fnEnterInterrupt(irq_I2C2_ID, PRIORITY_I2C2, _I2C_Interrupt_2);  // enter I2C2 interrupt handler
        #endif
        #if defined KINETIS_K80 && defined I2C2_ON_B                     // initially configure as inputs with pull-up
        _CONFIG_PORT_INPUT_FAST_LOW(B, (PORTB_BIT10 | PORTB_BIT11), (PORT_ODE | PORT_PS_UP_ENABLE));
        #elif defined KINETIS_K80 && defined I2C2_A_LOW
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT6 | PORTA_BIT7), (PORT_ODE | PORT_PS_UP_ENABLE));
        #elif defined I2C2_A_HIGH 
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT13 | PORTA_BIT14), (PORT_ODE | PORT_PS_UP_ENABLE));
        #else
            #if defined KINETIS_K80
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT10 | PORTA_BIT11), (PORT_ODE | PORT_PS_UP_ENABLE));
            #else
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT11 | PORTA_BIT12), (PORT_ODE | PORT_PS_UP_ENABLE));
            #endif
        #endif
        #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
            #if defined DMAMUX0_CHCFG_SOURCE_I2C2
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C2;
            #elif defined DMAMUX0_CHCFG_SOURCE_I2C1_2
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C1_2;
            #endif
        #endif
    }
    #endif
    #if I2C_AVAILABLE > 3
    else if (pars->Channel == 3) {                                       // I2C channel 3
        POWER_UP_ATOMIC(1, I2C3);                                        // enable clock to module
        ptrI2C = (KINETIS_I2C_CONTROL *)I2C3_BLOCK;
        fnEnterInterrupt(irq_I2C3_ID, PRIORITY_I2C3, _I2C_Interrupt_3);  // enter I2C3 interrupt handler
        #if defined I2C3_ON_E                                            // initially configure as inputs with pull-up
        _CONFIG_PORT_INPUT_FAST_LOW(E, (PORTE_BIT10 | PORTE_BIT11), (PORT_ODE | PORT_PS_UP_ENABLE));
        #else
        _CONFIG_PORT_INPUT_FAST_LOW(A, (PORTA_BIT1 | PORTA_BIT2), (PORT_ODE | PORT_PS_UP_ENABLE));
        #endif
        #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
            #if defined DMAMUX0_CHCFG_SOURCE_I2C3
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C3;
            #elif defined DMAMUX0_CHCFG_SOURCE_I2C0_3
        usDMA_Trigger = DMAMUX0_CHCFG_SOURCE_I2C0_3;
            #endif
        #endif
    }
    #endif
    else {
        return;
    }

    // The calculation of the correct divider ratio doesn't follow a formula so is best taken from a table.
    // The required divider value is ((BUS_CLOCK/1000)/pars->usSpeed). Various typical speeds are supported here.
    //                                                                   {1}
    // Note that some devices have a MULT field in the divider register which can be used as a prescaler - this is presently not used
    // due to errate e6070 which doesn't allow a repeat start to be generated when it is set to a non-zero value
    //
    switch (pars->usSpeed) {
    case 400:                                                            // high speed I2C (400kHz)
    #if BUS_CLOCK > 60000000                                             // 75MHz
        ucSpeed = 0x1e;                                                  // set about 400k with 75MHz bus frequency
    #elif BUS_CLOCK > 50000000                                           // 60MHz
        ucSpeed = 0x20;                                                  // set about 400k with 60MHz bus frequency
    #elif BUS_CLOCK > 40000000                                           // 50MHz
        ucSpeed = 0x17;                                                  // set about 400k with 50MHz bus frequency
    #elif BUS_CLOCK > 30000000                                           // 40MHz
        ucSpeed = 0x16;                                                  // set about 400k with 40MHz bus frequency
    #elif BUS_CLOCK > 20000000                                           // 30MHz
        ucSpeed = 0x14;                                                  // set about 400k with 30MHz bus frequency
    #else                                                                // assume 20MHz
        ucSpeed = 0x11;                                                  // set about 400k with 20MHz bus frequency
    #endif
        break;
    case 100:                                                            // 100kHz
    default:                                                             // default to 100kHz
    #if BUS_CLOCK > 60000000                                             // 75MHz
        ucSpeed = 0x2e;                                                  // set about 100k with 75MHz bus frequency
    #elif BUS_CLOCK > 50000000                                           // 60MHz
        ucSpeed = 0x2d;                                                  // set about 100k with 60MHz bus frequency
    #elif BUS_CLOCK > 40000000                                           // 50MHz
        ucSpeed = 0x2b;                                                  // set about 100k with 50MHz bus frequency
    #elif BUS_CLOCK > 30000000                                           // 40MHz
        ucSpeed = 0x2a;                                                  // set about 100k with 40MHz bus frequency
    #elif BUS_CLOCK > 20000000                                           // 30MHz
        ucSpeed = 0x28;                                                  // set about 100k with 30MHz bus frequency
    #elif BUS_CLOCK > 10000000                                           // 20MHz
        ucSpeed = 0x22;                                                  // set about 100k with 20MHz bus frequency
    #elif BUS_CLOCK > 3000000                                            // 4MHz
        ucSpeed = 0x07;                                                  // set about 100k with 4MHz bus frequency
    #else                                                                // 2MHz
        ucSpeed = 0x00;                                                  // set about 100k with 2MHz bus frequency
    #endif
        break;

    case 50:                                                             // 50kHz
    #if BUS_CLOCK > 60000000                                             // 75MHz
        ucSpeed = 0x36;                                                  // set about 50k with 75MHz bus frequency
    #elif BUS_CLOCK > 50000000                                           // 60MHz
        ucSpeed = 0x35;                                                  // set about 50k with 60MHz bus frequency
    #elif BUS_CLOCK > 40000000                                           // 50MHz
        ucSpeed = 0x33;                                                  // set about 50k with 50MHz bus frequency
    #elif BUS_CLOCK > 20000000                                           // 40MHz
        ucSpeed = 0x32;                                                  // set about 50k with 40MHz bus frequency
    #elif BUS_CLOCK > 10000000                                           // 30MHz
        ucSpeed = 0x2c;                                                  // set about 50k with 30MHz bus frequency
    #else                                                                // assume 20MHz
        ucSpeed = 0x29;                                                  // set about 50k with 20MHz bus frequency
    #endif
        break;
    #if defined I2C_SLAVE_MODE
    case 0:                                                              // I2C slave mode
        {
            ucSpeed = 0;                                                 // speed is determined by master
            ptrI2C->I2C_A1 = pars->ucSlaveAddress;                       // program the slave's address
            fnConfigI2C_pins(pars->Channel, 0);                          // configure pins for I2C use
            I2C_rx_control[pars->Channel]->ucState = I2C_SLAVE_RX_MESSAGE_MODE; //{4} the slave receiver operates in message mode
            I2C_tx_control[pars->Channel]->ucState = I2C_SLAVE_TX_BUFFER_MODE; // the slave transmitter operates in buffer mode
            fnI2C_SlaveCallback[pars->Channel] = pars->fnI2C_SlaveCallback; // optional callback to use on slave reception
        }
        break;
    #endif
    }
    ptrI2C->I2C_F = ucSpeed;                                             // set the operating speed
    #if !defined KINETIS_KE
    ptrI2C->I2C_C1 = (I2C_IEN);                                          // enable I2C controller
    #endif
    #if defined I2C_DMA_SUPPORT && !defined DEVICE_WITHOUT_DMA
    if ((pars->ucDMAConfig & I2C_TX_DMA) != 0) {
        usDMATriggerSourceTx[pars->Channel] = usDMA_Trigger;             // mark that this channel is operating in DMA tx mode
    }
    if ((pars->ucDMAConfig & I2C_RX_DMA) != 0) {
        usDMATriggerSourceRx[pars->Channel] = usDMA_Trigger;             // mark that this channel is operating in DMA rx mode
    }
    #endif
    #if defined I2C_SLAVE_MODE
    if (ucSpeed == 0) {
        ptrI2C->I2C_C1 |= I2C_IIEN;                                      // immediately enable interrupts if operating as a slave
        #if defined DOUBLE_BUFFERED_I2C || defined I2C_START_CONDITION_INTERRUPT
        ptrI2C->I2C_FLT = I2C_FLT_FLT_SSIE;                              // enable start/stop condition interrupt
        #else
        ptrI2C->I2C_FLT = I2C_FLT_FLT_STOPIE;                            // enable stop condition interrupt
        #endif
    }
    #endif
    #if defined _WINDOWS
    fnConfigSimI2C(pars->Channel, (pars->usSpeed * 1000));               // inform the simulator of the I2C speed that has been set
    #endif
}

