/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      touch.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

#if defined TOUCH_SCREEN_DEFINES
/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

    #define TOUCH_IDLE                  0x00
    #define TOUCH_GET_PEN               0x01
    #define TOUCH_POLLING               0x02
    #define TOUCH_INITIALISING          0x04

    #if defined MB785_GLCD_MODE
        // Touch screen controller for STM321C-EVAL
        //
        #define ADD_TOUCH_CONTROLLER_READ          0x83
        #define ADD_TOUCH_CONTROLLER_WRITE         0x82
                                               
        #define STMPE811_CHIP_ID_OFFSET            0x00
        #define STMPE811_ID_VER_OFFSET             0x02                  // address 2
        #define STMPE811_SYS_CTRL1_OFFSET          0x03                  // address 3 - reset control
            #define STMPE811_HIBERNATE               0x01
            #define STMPE811_SOFT_RESET              0x02
        #define STMPE811_SYS_CTRL2_OFFSET          0x04                  // address 4 - clock control
            #define STMPE811_ADC_OFF                 0x01
            #define STMPE811_TOUCH_OFF               0x02
            #define STMPE811_GPIO_OFF                0x04
            #define STMPE811_TEMP_SENSOR_OFF         0x08
        #define STMPE811_SPI_CFG_OFFSET            0x08                  // address 8 - SPI configuration
        #define STMPE811_INT_CTRL_OFFSET           0x09                  // address 9 - interrupt control register
            #define STMPE811_GLOBAL_INT              0x01
            #define STMPE811_TYPE_EDGE               0x02
            #define STMPE811_TYPE_LEVEL              0x00
            #define STMPE811_ACTIVE_HIGH             0x04
            #define STMPE811_ACTIVE_LOW              0x00
        #define STMPE811_INT_EN_OFFSET             0x0a                  // address 10 - interrupt enable register
            #define STMPE811_INT_TOUCH_DET           0x01
            #define STMPE811_INT_FIFO_TH             0x02
            #define STMPE811_INT_FIFO_OFLOW          0x04
            #define STMPE811_INT_FIFO_FULL           0x08
            #define STMPE811_INT_FIFO_EMPTY          0x10
            #define STMPE811_INT_TEMP_SENS           0x20
            #define STMPE811_INT_ADC                 0x40
            #define STMPE811_INT_GPIO                0x80
        #define STMPE811_INT_STA_OFFSET            0x0b                  // address 11 - interrupt status register (read-only/write 1 to reset)
        #define STMPE811_GPIO_EN_OFFSET            0x0c                  // address 12 - GPIO interrupt enable register
        #define STMPE811_GPIO_INT_STA_OFFSET       0x0d                  // address 13 - GPIO interrupt status register (read-only)
        #define STMPE811_ADC_INT_EN_OFFSET         0x0e                  // address 14 - ADC interrupt enable register
        #define STMPE811_ADC_INT_STA_OFFSET        0x0f                  // address 15 - ADC interrupt status register (read-only)
        #define STMPE811_GPIO_SET_PIN_OFFSET       0x10                  // address 16 - GPIO set pin register
        #define STMPE811_GPIO_CLR_PIN_OFFSET       0x11                  // address 17 - GPIO clear pin register
        #define STMPE811_GPIO_MPA_STA_OFFSET       0x12                  // address 18 - GPIO monitor pin state register
        #define STMPE811_GPIO_DIR_OFFSET           0x13                  // address 19 - GPIO direction register
        #define STMPE811_GPIO_ED_OFFSET            0x14                  // address 20 - GPIO edge detect register
        #define STMPE811_GPIO_RE_OFFSET            0x15                  // address 21 - GPIO rising edge register
        #define STMPE811_GPIO_FE_OFFSET            0x16                  // address 22 - GPIO falling edge register
        #define STMPE811_GPIO_AF_OFFSET            0x17                  // address 23 - GPIO alternate function register
        #define STMPE811_ADC_CTRL1_OFFSET          0x20                  // address 32 - ADC control
            #define STMPE811_ADC_CTRL1_REF_SEL       0x02
            #define STMPE811_ADC_CTRL1_MOD12B        0x08
            #define STMPE811_ADC_CTRL1_CONV36        0x00
            #define STMPE811_ADC_CTRL1_CONV44        0x10
            #define STMPE811_ADC_CTRL1_CONV56        0x20
            #define STMPE811_ADC_CTRL1_CONV64        0x30
            #define STMPE811_ADC_CTRL1_CONV80        0x40
            #define STMPE811_ADC_CTRL1_CONV96        0x50
            #define STMPE811_ADC_CTRL1_CONV124       0x60
        #define STMPE811_ADC_CTRL2_OFFSET          0x21                  // address 33 - ADC control
            #define STMPE811_ADC_CTRL2_1_625M        0x00
            #define STMPE811_ADC_CTRL2_3_25M         0x01
            #define STMPE811_ADC_CTRL2_6_5M          0x02
        #define STMPE811_ADC_CAPT_OFFSET           0x22                  // address 34 - ADC acquisition control
        #define STMPE811_ADC_DATA_CH0_OFFSET       0x30                  // address 48 - ADC channel 0
        #define STMPE811_ADC_DATA_CH1_OFFSET       0x32                  // address 50 - ADC channel 1
        #define STMPE811_ADC_DATA_CH2_OFFSET       0x34                  // address 52 - ADC channel 2
        #define STMPE811_ADC_DATA_CH3_OFFSET       0x36                  // address 54 - ADC channel 3
        #define STMPE811_ADC_DATA_CH4_OFFSET       0x38                  // address 56 - ADC channel 4
        #define STMPE811_ADC_DATA_CH5_OFFSET       0x3a                  // address 58 - ADC channel 5
        #define STMPE811_ADC_DATA_CH6_OFFSET       0x3c                  // address 60 - ADC channel 6
        #define STMPE811_ADC_DATA_CH7_OFFSET       0x3e                  // address 62 - ADC channel 7
        #define STMPE811_TSC_CTRL_OFFSET           0x40                  // address 64 - 4-wire touch screen controller setup
            #define TCS_CTRL_EN                      0x01
            #define TCS_CTRL_X_Y_Z                   0x00
            #define TCS_CTRL_X_Y                     0x02
            #define TCS_CTRL_X                       0x04
            #define TCS_CTRL_Y                       0x06
            #define TCS_CTRL_Z                       0x08
            #define TCS_CTRL_TRACK_OFF               0x00
            #define TCS_CTRL_TRACK_4                 0x10
            #define TCS_CTRL_TRACK_8                 0x20
            #define TCS_CTRL_TRACK_16                0x30
            #define TCS_CTRL_TRACK_32                0x40
            #define TCS_CTRL_TRACK_64                0x50
            #define TCS_CTRL_TRACK_92                0x60
            #define TCS_CTRL_TRACK_127               0x70
            #define TCS_CTRL_TOUCH_DETECT            0x80
        #define STMPE811_TSC_CFG_OFFSET            0x41                  // address 65 - touch screen controller configuration
            #define TSC_CFG_SETTLING_10us            0x00
            #define TSC_CFG_SETTLING_100us           0x01
            #define TSC_CFG_SETTLING_500us           0x02
            #define TSC_CFG_SETTLING_1ms             0x03
            #define TSC_CFG_SETTLING_5ms             0x04
            #define TSC_CFG_SETTLING_10ms            0x05
            #define TSC_CFG_SETTLING_50ms            0x06
            #define TSC_CFG_SETTLING_100ms           0x07
            #define TSC_CFG_DET_DELAY_10us           0x00
            #define TSC_CFG_DET_DELAY_50us           0x08
            #define TSC_CFG_DET_DELAY_100us          0x10
            #define TSC_CFG_DET_DELAY_500us          0x18
            #define TSC_CFG_DET_DELAY_1ms            0x20
            #define TSC_CFG_DET_DELAY_5ms            0x28
            #define TSC_CFG_DET_DELAY_10ms           0x30
            #define TSC_CFG_DET_DELAY_50ms           0x38
            #define TSC_CFG_AVE_CTRL_1_SAMPLE        0x00
            #define TSC_CFG_AVE_CTRL_2_SAMPLE        0x40
            #define TSC_CFG_AVE_CTRL_4_SAMPLE        0x80
            #define TSC_CFG_AVE_CTRL_8_SAMPLE        0xc0
        #define STMPE811_WDW_TR_X_OFFSET           0x42                  // address 66 - window setup for top right X
        #define STMPE811_WDW_TR_Y_OFFSET           0x44                  // address 68 - window setup for top right Y
        #define STMPE811_WDW_BL_X_OFFSET           0x46                  // address 70 - window setup for bottom left X
        #define STMPE811_WDW_BL_Y_OFFSET           0x48                  // address 72 - window setup for bottom left Y
        #define STMPE811_FIFO_TH_OFFSET            0x4a                  // address 74 - FIFO level to generate interrupt
        #define STMPE811_FIFO_STA_OFFSET           0x4b                  // address 75 - FIFO status
            #define FIFO_STA_FIFO_RESET              0x01
            #define FIFO_STA_FIFO_TH_TRIG            0x10
            #define FIFO_STA_FIFO_EMPTY              0x20
            #define FIFO_STA_FIFO_FULL               0x40
            #define FIFO_STA_FIFO_OFLOW              0x80
        #define STMPE811_FIFO_SIZE_OFFSET          0x4c                  // address 76 - FIFO size (read-only)
        #define STMPE811_TSC_DATA_X_OFFSET         0x4d                  // address 77 - touch screen controller data access (read-only)
        #define STMPE811_TSC_DATA_Y_OFFSET         0x4f                  // address 79 - touch screen controller data access (read-only)
        #define STMPE811_TSC_DATA_Z_OFFSET         0x51                  // address 81 - touch screen controller data access (read-only)
        #define STMPE811_TSC_DATA_XYZ_OFFSET       0x52                  // address 82 - touch screen controller data access (read-only)
        #define STMPE811_TSC_FRACT_Z_OFFSET        0x56                  // address 86 - touch screen controller FRACTION_Z
            #define TSC_FRACT_0_8                    0x00
            #define TSC_FRACT_1_7                    0x01
            #define TSC_FRACT_2_6                    0x02
            #define TSC_FRACT_3_5                    0x03
            #define TSC_FRACT_4_4                    0x04
            #define TSC_FRACT_5_3                    0x05
            #define TSC_FRACT_6_2                    0x06
            #define TSC_FRACT_7_1                    0x07
        #define STMPE811_TSC_DATA_OFFSET           0x57                  // address 87 - touch screen controller data access (read-only)
        #define STMPE811_TSC_I_DRIVE_OFFSET        0x58                  // address 88 - touch screen controller drive I
            #define TSC_I_DRIVE_20mA                 0x00
            #define TSC_I_DRIVE_50mA                 0x01
        #define STMPE811_TSC_SHIELD_OFFSET         0x59                  // address 89 - touch screen controller shield
                                               
        #define STMPE811_TEMP_CTRL_OFFSET          0x60                  // address 96 - temperature sensor setup
        #define STMPE811_TEMP_DATA_OFFSET          0x61                  // address 97 - temperature data (read-only)
        #define STMPE811_TEMP_TH_OFFSET            0x62                  // address 98 - threshold for temperature controlled interrupt

        #define MIN_X_TOUCH        0x00f0                                // tuned values - for calibration these should be taken from parameters
        #define MAX_X_TOUCH        0x0f26
        #define MIN_Y_TOUCH        0x0110
        #define MAX_Y_TOUCH        0x0f10

        #define MAX_MOVEMENT_Y     12
        #define MAX_MOVEMENT_X     12
    #elif defined TOUCH_FT5406
        #define ADD_TOUCH_CONTROLLER_WRITE         0x70
        #define ADD_TOUCH_CONTROLLER_READ          0x71

        #define FT5X06_DEVICE_MODE_ADDRESS         0
            #define FT5X06_DEVICE_MODE_NORMAL          0x00              // normal operating mode
            #define FT5X06_DEVICE_MODE_SYSTEM_INFO     0x10              // (reserved)
            #define FT5X06_DEVICE_MODE_TES             0x20              // test mode - read raw data (reserved)

        #define FT5X06_GEST_ID_ADDRESS             1
            #define FT5X06_GEST_ID_NO_GESTURE          0x00
            #define FT5X06_GEST_ID_MOVE_FLAG           0x10
            #define FT5X06_GEST_ID_MOVE_UP             0x10
            #define FT5X06_GEST_ID_MOVE_LEFT           0x14
            #define FT5X06_GEST_ID_MOVE_DOWN           0x18
            #define FT5X06_GEST_ID_MOVE_RIGHT          0x1c
            #define FT5X06_GEST_ID_ZOOM_IN             0x48
            #define FT5X06_GEST_ID_ZOOM_OUT            0x49

        #define FT5X06_TD_STATUS_ADDRESS           2
            #define FT5X06_TD_STATUS_DETECTED_0        0x00
            #define FT5X06_TD_STATUS_DETECTED_1        0x01
            #define FT5X06_TD_STATUS_DETECTED_2        0x02
            #define FT5X06_TD_STATUS_DETECTED_3        0x03
            #define FT5X06_TD_STATUS_DETECTED_4        0x04
            #define FT5X06_TD_STATUS_DETECTED_5        0x05

        #define MIN_X_TOUCH        1                                     // tuned values - for calibration these should be taken from parameters
        #define MAX_X_TOUCH        476
        #define MIN_Y_TOUCH        1
        #define MAX_Y_TOUCH        270

        #define MAX_MOVEMENT_Y     5
        #define MAX_MOVEMENT_X     5
    #elif defined TOUCH_FT6206
        #define FT6206_WRITE_ADDRESS               0x70
        #define FT6206_READ_ADDRESS                0x71
        #define FT6206_GEST_OFFSET                 1
        #define FT6206_TD_STATUS_OFFSET            2
            #define TCS_CTRL_TOUCH_DETECT              0x01              // first touch detected
            #define TCS_CTRL_2nd_TOUCH_DETECT          0x02              // second touch detected
        #define TOUCH_DATA_LENGTH                  5
        #define FT6206_XH1_OFFSET                  3
        #define FT6206_XL1_OFFSET                  4
        #define EVENT_LIFT_UP                          0x40
        #define FT6206_YH1_OFFSET                  5
        #define FT6206_YL1_OFFSET                  6

        #define MIN_X_TOUCH        0x0000                                // tuned values - for calibration these should be taken from parameters
        #define MAX_X_TOUCH        0x00ef
        #define MIN_Y_TOUCH        0x0000
        #define MAX_Y_TOUCH        0x0ef

        #define MAX_MOVEMENT_Y     5
        #define MAX_MOVEMENT_X     5
    #endif
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined TOUCH_SCREEN_CONSTS
    #if defined MB785_GLCD_MODE
        #define TOUCH_DATA_LENGTH
        static const unsigned char ucResetInt[]      = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_INT_STA_OFFSET, (STMPE811_INT_TOUCH_DET)};
        static const unsigned char ucGetTouchData[]  = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_FIFO_STA_OFFSET}; // read a block of touch data values
        static const unsigned char ucGetData[]       = {TOUCH_DATA_LENGTH, ADD_TOUCH_CONTROLLER_READ, OWN_TASK}; // read 7 bytes from this address and wake the task when the data is available
        static const unsigned char ucGetTouchState[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_TSC_CTRL_OFFSET}; // first read the touch screen controller state to ensure that a detection exists
        static const unsigned char ucGetValue[]      = {1, ADD_TOUCH_CONTROLLER_READ, OWN_TASK}; // read 1 byte from this address to ensure that a touch has been detected
        static const unsigned char ucResetFIFO1[]    = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_FIFO_STA_OFFSET, (FIFO_STA_FIFO_RESET)}; // reset FIFO buffer
        static const unsigned char ucResetFIFO2[]    = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_FIFO_STA_OFFSET, (0)}; // take FIFO buffer out of reset
    #elif defined TOUCH_FT5406
        static const unsigned char ucSetNormalMode[] = {ADD_TOUCH_CONTROLLER_WRITE, FT5X06_DEVICE_MODE_ADDRESS, (FT5X06_DEVICE_MODE_NORMAL)}; // set the touch controller to its normal mode of operation
        static const unsigned char ucGetTouchState[] = {ADD_TOUCH_CONTROLLER_WRITE, FT5X06_DEVICE_MODE_ADDRESS};
        static const unsigned char ucGetValue[]      = {0x21, ADD_TOUCH_CONTROLLER_READ, OWN_TASK}; // read 32 byte from this address to obtain full touch data
    #elif defined TOUCH_FT6206
        static const unsigned char ucGetTouchState[] = {FT6206_WRITE_ADDRESS, FT6206_TD_STATUS_OFFSET}; // first read the touch screen controller state to ensure that a detection exists
        static const unsigned char ucGetValue[]      = {1, FT6206_READ_ADDRESS, OWN_TASK}; // read 1 byte from this address to ensure that a touch has been detected
        static const unsigned char ucGetTouchData[]  = {FT6206_WRITE_ADDRESS, FT6206_TD_STATUS_OFFSET }; // read a block of touch data values
        static const unsigned char ucGetData[]       = {TOUCH_DATA_LENGTH, FT6206_READ_ADDRESS, OWN_TASK}; // read 5 bytes from this address and wake the task when the data is available
    #endif
#endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

#if defined TOUCH_SCREEN_VARIABLES
    static QUEUE_HANDLE TouchPortID = 0;
    static int iTouchState = TOUCH_IDLE;
    #if !defined TOUCH_FT5406
    static int iPenDown = 0;
    #endif
#endif


#if defined TOUCH_SCREEN_FUNCTIONS
    #if defined TOUCH_FT6206 || defined TOUCH_FT5406
// Touch screen informs that touch data is ready to be read
//
static void touch_screen_interrupt(void)
{
    fnInterruptMessage(OWN_TASK, PEN_STATE_CHANGE);                      // inform that the pen state has changed
}
    #endif

// The function is presently dedicated to Luminary Micro, Freescale Tower LCD, FT6206 and FT5406
//
static void fnStartTouch(void)
{
    #if defined TOUCH_FT6206 || defined TOUCH_FT5406                     // connected via I2C
    I2CTABLE tI2CParameters;
    tI2CParameters.Channel = 0;                                          // I2C interface 0
    tI2CParameters.usSpeed = 100;                                        // 100k
    tI2CParameters.Rx_tx_sizes.TxQueueSize = 64;                         // transmit queue size
    tI2CParameters.Rx_tx_sizes.RxQueueSize = 128;                        // receive queue size
        #if defined SUPPORT_QUEUE_DEALLOCATION
    tI2CParameters.Rx_tx_sizes.ucHeapType = 0;                           // the driver queues created here use non de-allocatable heap memory
        #endif
    tI2CParameters.Task_to_wake = 0;                                     // no wake on transmission

    if ((TouchPortID = fnOpen(TYPE_I2C, FOR_I_O, &tI2CParameters)) != NO_ID_ALLOCATED) { // open the channel with defined configurations
        INTERRUPT_SETUP interrupt_setup;                                 // interrupt configuration parameters
        interrupt_setup.int_type      = PORT_INTERRUPT;                  // identifier to configure port interrupt
        interrupt_setup.int_handler   = touch_screen_interrupt;          // interrupt handling callback
        interrupt_setup.int_priority  = TC_INT_PRIORTY;                  // interrupt priority level
        interrupt_setup.int_port      = TC_INT_PORT;                     // the port that the interrupt input is on
        interrupt_setup.int_port_bits = TC_INT_PORT_BIT;                 // the interrupt input
        interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON); // falling edge generates interrupt
        fnConfigureInterrupt((void *)&interrupt_setup);                  // configure touch screen interrupt
        TOUCHRESET_H();                                                  // negate the reset line to the touch screen controller (it will report via the interrupt when it is ready (about 300ms later)
        #if defined TOUCH_FT5406
        iTouchState = TOUCH_INITIALISING;
        if (TOUCH_INTERRUPT_STATE() == 0) {                              // if the touch screen is already indicating ready
            fnInterruptMessage(OWN_TASK, PEN_STATE_CHANGE);              // kick off the initialisation
        }
        #endif
    }
    #elif defined TFT2N0369_GLCD_MODE                                    // Freescale/NXP Tower LCD
    ADC_SETUP adc_setup; 
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnY_ready;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_bit = 4;
    adc_setup.int_adc_int_type = ADC_END_OF_SCAN_INT;
    adc_setup.int_adc_offset = 0;                                        // no offset
    adc_setup.int_high_level_trigger = 0;
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_AUTO_POWER_DOWN_MODE | ADC_SEQUENTIAL_MODE | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE);
    adc_setup.int_adc_speed = (unsigned char)(ADC_SAMPLING_SPEED(650000));// 650kHz sampling - slowest possible at 80MHz (must be between 100kHz and 5MHz)
    fnConfigureInterrupt((void *)&adc_setup);                            // configure ADC and channel 4
    adc_setup.int_adc_bit = 5;
    adc_setup.int_adc_int_type = 0;                                      // don't configure interrupts again - channel 4 setting is relevant
    adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL);
    fnConfigureInterrupt((void *)&adc_setup);                            // configure channel 5
    adc_setup.int_adc_bit = 6;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure channel 6
    adc_setup.int_adc_bit = 7;
    adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_START_OPERATION);
    fnConfigureInterrupt((void *)&adc_setup);                            // configure channel 7
    PANPAR &= ~(TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS | TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS); // set all pins back to GPIO mode
    ADC_POWER |= 0x3f0;                                                  // maximum start up delay
    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(AN, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS | TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS), 0);
    #else                                                                // Luminary Micro IDM_L35_B
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    TIMER_INTERRUPT_SETUP timer_setup;                                   // interrupt configuration parameters
    timer_setup.int_type = TIMER_INTERRUPT;
    timer_setup.int_priority = PRIORITY_TIMERS;
    timer_setup.int_handler = 0;
    timer_setup.timer_reference = 2;                                     // timer channel 2
    timer_setup.timer_mode = (TIMER_TRIGGER_ADC | TIMER_PERIODIC | TIMER_MS_VALUE); // period timer with ADC trigger
    timer_setup.timer_value = 1;                                         // 1ms interrupt rate

    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnY_stabilise;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_5 | ADC_CHANNEL_7); // ADC channels 5 and 7 as single ended inputs
    adc_setup.int_adc_differential_inputs = 0;
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_TRIGGER_TIMER);    // configure and triggered by timer
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
    adc_setup.int_adc_result = usResults;
    adc_setup.int_sequence_count = 1;

    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(C, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS), 0);
    _CONFIG_DRIVE_PORT_OUTPUT_VALUE(D, (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS), 0);

    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
    fnConfigureInterrupt((void *)&timer_setup);                          // enter interrupt for timer test
    #endif
}

// Display pen touch point
//
static void fnDisplayTouchPoint(unsigned short usX, short usY, unsigned char ucPenID, int iEvent)
{
    #define TOUCH_POINT_RADIUS     18
    static GLCD_TOUCH_POINT touchLocation[TOUCH_SCREEN_PENS] = { 0 };
    if (usX < TOUCH_POINT_RADIUS) {
        usX = TOUCH_POINT_RADIUS;
    }
    else if (usX > (GLCD_X - TOUCH_POINT_RADIUS)) {
        usX = (GLCD_X - TOUCH_POINT_RADIUS);
    }
    if (usY < TOUCH_POINT_RADIUS) {
        usY = TOUCH_POINT_RADIUS;
    }
    else if (usY > (GLCD_Y - TOUCH_POINT_RADIUS)) {
        usY = (GLCD_Y - TOUCH_POINT_RADIUS);
    }
    touchLocation[ucPenID].ucRef = ucPenID;
    switch (iEvent) {
    case -1:                                                             // move the existing touch point
        touchLocation[ucPenID].ucMode = (PAINT_RAW | REDRAW);
        fnDoLCD_touch(&touchLocation[ucPenID]);
        // Fall-through intentionally
        //
    case 1:                                                              // create the initial touch point
        touchLocation[ucPenID].ucMode = (PAINT_RAW);
        break;

    case 0:                                                              // fade out the initial touch point
        touchLocation[ucPenID].ucMode = (PAINT_RAW | REDRAW | WINDOWED_PIC);
        break;
    default:
        _EXCEPTION("Invalid!");
        break;
    }
    touchLocation[ucPenID].usCenterX = usX;                              // specify the center of the touch point
    touchLocation[ucPenID].usCenterY = usY;
    touchLocation[ucPenID].usRadius = TOUCH_POINT_RADIUS;                // touch radius
    fnDoLCD_touch(&touchLocation[ucPenID]);
}
#endif

// Touch screen interrupt events
//
#if defined TOUCH_SCREEN_INTERRUPT_EVENT
            if (PEN_STATE_CHANGE == ucInputMessage[MSG_INTERRUPT_EVENT]) {
    #if defined TOUCH_FT5406
                switch (iTouchState) {
                case TOUCH_INITIALISING:
                    fnWrite(TouchPortID, (unsigned char *)ucSetNormalMode, sizeof(ucSetNormalMode)); // ensure that the touch controller is in normal operating mode
                    // Fall-though intentionally
                    //
                case TOUCH_IDLE:
                    iTouchState = TOUCH_GET_PEN;
                    fnWrite(TouchPortID, (unsigned char *)ucGetTouchState, sizeof(ucGetTouchState)); // set the read address (note that the actual address entered doesn't change the fact that it always read  fom the first address)]
                    fnRead(TouchPortID, (unsigned char *)ucGetValue, 0); // start the read process of the touch state
                    break;
                }
    #else
        #if defined TOUCH_FT6206
                if (iTouchState == TOUCH_IDLE)                           // if not already in the process of requesting the state (otherwise ignore)
        #else
                if ((iTouchState & TOUCH_GET_PEN) == 0)
        #endif
                {
        #if defined MB785_GLCD_MODE
                    if (iTouchState == TOUCH_POLLING) {                  // if pen is down and the movement is being polled
                        iTouchState = (TOUCH_POLLING | TOUCH_GET_PEN);   // mark that the pen state is to be requested
                    }
                    else {
        #endif
                        fnWrite(TouchPortID, (unsigned char *)ucGetTouchState, sizeof(ucGetTouchState)); // set the read address
                        fnRead(TouchPortID,  (unsigned char *)ucGetValue, 0); // start the read process of the touch state
                        iTouchState = TOUCH_GET_PEN;                     // expecting the pen state
        #if defined MB785_GLCD_MODE
                    }
        #endif
                }
    #endif
                break;
            }
#endif

// Touch screen I2C reception
//
#if defined TOUCH_SCREEN_I2C_RECEPTION
    if (iTouchState != TOUCH_IDLE) {                                     // ignore if not idle
        while (fnMsgs(TouchPortID) != 0) {                               // while touch screen message(s) available
    #if defined TOUCH_FT5406
            unsigned char ucTempData[0x21];
            if (fnRead(TouchPortID, ucTempData, 0x21) != 0x21) {         // each message is of a fixed length
                fnDebugMsg(" Bad length\r\n");                           // this should never happen
            }
            else {
                static unsigned short usPenX[TOUCH_SCREEN_PENS] = {0};
                static unsigned short usPenY[TOUCH_SCREEN_PENS] = {0};
                static unsigned char ucLastPensDown = 0;                 // the number of pens down in previous message

                unsigned char ucPensNow = ucTempData[3];                 // the number of pens down according to this message
                unsigned char ucThisPen = 0;
                unsigned char ucPenID;
                int iDisplay = 0;
                if (ucPensNow != ucLastPensDown) {                       // if a different number of pens are down in comparison to last message
                    if (ucPensNow < ucLastPensDown) {                    // if there are less pens down this time
                        unsigned char ucLastValue = ucLastPensDown;
                        ucLastPensDown = ucPensNow;                      // save the new number of pens for comparison with next message
                        ucPensNow = ucLastValue;                         // use previous pen count in order to detect all pen up events
                    }
                    else {
                        ucLastPensDown = ucPensNow;                      // save the new number
                    }
                }
                while (ucPensNow-- != 0) {                               // for each pen event in this message
                    unsigned short usX;
                    unsigned short usY;
                    ucPenID = (ucTempData[6 + (ucThisPen * 6)] >> 4);    // the pen ID that the coordinates belong to
                    usY = (ucTempData[4 + (ucThisPen * 6)] & 0xf);       // extract each pen coordinates
                    usY <<= 8;
                    usY |= ucTempData[5 + (ucThisPen * 6)];
                    usX = (ucTempData[6 + (ucThisPen * 6)] & 0xf);
                    usX <<= 8;
                    usX |= ucTempData[7 + (ucThisPen * 6)];
                    switch (ucTempData[4 + (ucThisPen * 6)] & 0xc0) {    // the event assiciated with this coordinate
                    case 0x00:                                           // pen down
                        fnDebugMsg("Pen");
                        fnDebugDec(ucPenID, WITH_SPACE);
                        fnDebugMsg(" down");
                        if (fnHandleTouchKey(usX, usY, ucPenID, PEN_DOWN_EVENT) == 0) { // handle pressing keys
                            fnDisplayTouchPoint(usX, usY, ucPenID, 1);   // display the touch point
                        }
    #if !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
                      //fnHandleTouch(usY, usX, 1);
    #endif
                        break;
                    case 0x40:                                           // pen just moved up
                        fnDebugMsg("Pen");
                        fnDebugDec(ucPenID, WITH_SPACE);
                        fnDebugMsg("   up");
                        if (fnHandleTouchKey(usX, usY, ucPenID, PEN_UP_EVENT) == 0) { // handle releasing pressed keys
                            fnDisplayTouchPoint(usX, usY, ucPenID, 0);   // fade the released touch point
                        }
    #if !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
                      //fnHandleTouch(usY, usX, 0);
    #endif
                        break;
                    case 0x80:                                           // pen movement
                        if ((usPenX[ucPenID] == usX) && (usPenY[ucPenID] == usY)) { // don't display when there is no movement
                            ucThisPen++;
                            continue;
                        }
                        fnDebugMsg("Pen");
                        fnDebugDec(ucPenID, WITH_SPACE);
                        fnDebugMsg(" move");
                        if (fnHandleTouchKey(usX, usY, ucPenID, PEN_MOVE_EVENT) == 0) { // handle moving away from pressed keys
                            fnDisplayTouchPoint(usX, usY, ucPenID, -1);  // fade the potental released touch point
                        }
    #if !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
                      //fnHandleTouch(usY, usX, 1);
    #endif
                        break;
                    }
                    usPenX[ucPenID] = usX;                               // save the coordinates belonging to each pen
                    usPenY[ucPenID] = usY;
                    fnDebugDec(usX, WITH_SPACE);
                    fnDebugMsg(",");
                    fnDebugDec(usY, WITH_SPACE);
                    ucThisPen++;
                    iDisplay = 1;
                }
                if (iDisplay != 0) {
                    fnDebugMsg("\r\n");
                }
            }
            iTouchState = TOUCH_IDLE;
    #else
            switch (iTouchState) {
            case TOUCH_GET_PEN:                                          // requesting first pen state after interrupt
        #if defined MB785_GLCD_MODE
                fnWrite(TouchPortID, (unsigned char *)ucResetInt, sizeof(ucResetInt)); // reset the  pending interrupt bit
        #endif
                fnRead(TouchPortID, ucInputMessage, 1);                  // read the state
                if ((ucInputMessage[0] & TCS_CTRL_TOUCH_DETECT) != 0) {  // if pen is down
                    fnWrite(TouchPortID, (unsigned char *)ucGetTouchData, sizeof(ucGetTouchData)); // set the read address
                    fnRead(TouchPortID,  (unsigned char *)ucGetData, 0); // start the read process of the block of data
         #if defined PEN_DOWN_ACTION
                    PEN_DOWN_ACTION();                                   // user definable pen down action
         #endif
                    fnDebugMsg("PEN DOWN\n\r");
                    iTouchState = TOUCH_POLLING;                         // we now poll to monitor movement
                    iPenDown = 1;                                        // pen state is down
                }
                else {                                                   // pen is up
                    iTouchState = TOUCH_IDLE;
        #if defined MB785_GLCD_MODE
                    fnWrite(TouchPortID, (unsigned char *)ucResetFIFO1, sizeof(ucResetFIFO1)); // flush FIFO
                    fnWrite(TouchPortID, (unsigned char *)ucResetFIFO2, sizeof(ucResetFIFO2)); // release flush FIFO
        #endif
                    fnDebugMsg("PEN UP\n\r");
                }
                break;
        #if defined MB785_GLCD_MODE
            case (TOUCH_GET_PEN | TOUCH_POLLING):                        // requesting touch location with queued state
                iTouchState = TOUCH_GET_PEN;
                fnWrite(TouchPortID, (unsigned char *)ucGetTouchState, sizeof(ucGetTouchState)); // set the read address
                fnRead(TouchPortID,  (unsigned char *)ucGetValue,      0);   // start the read process of 1 byte
                // Fall-through intentionally
                //
        #endif
            case TOUCH_POLLING:                                          // request touch location
                {
                    fnRead(TouchPortID, ucInputMessage, TOUCH_DATA_LENGTH);
        #if defined MB785_GLCD_MODE
                    if ((ucInputMessage[0] & FIFO_STA_FIFO_EMPTY) == 0) {// only when valid touch position
            #if !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
                        unsigned short usY = ((ucInputMessage[2] << 8) + ucInputMessage[3]);
                        unsigned short usX = ((ucInputMessage[4] << 8) + ucInputMessage[5]);
                        fnHandleTouch(usX, usY, iPenDown);
            #endif
                        iPenDown = 0;                                    // reset reporting of new pen-down
                    }
        #elif defined TOUCH_FT6206 || defined TOUCH_FT5406
                    if ((ucInputMessage[0] & TCS_CTRL_TOUCH_DETECT) != 0) { // if pen is down
            #if !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
                        unsigned short usY = ((ucInputMessage[3] << 8) + ucInputMessage[4]);
                        unsigned short usX = ((ucInputMessage[1] << 8) + ucInputMessage[2]);
                        fnHandleTouch((usX & 0x0fff), (usY & 0x0fff), iPenDown);
            #endif
                        iPenDown = 0;                                    // reset reporting of new pen-down
                    }
                    else {
                        iTouchState = TOUCH_IDLE;
                        fnDebugMsg("PEN UP\n\r");
                        break;
                    }
        #endif
                    if (iTouchState == TOUCH_POLLING) {
                        fnWrite(TouchPortID, (unsigned char *)ucGetTouchData, sizeof(ucGetTouchData)); // set the read address
                        fnRead(TouchPortID,  (unsigned char *)ucGetData, 0); // start the read process of the next block of data
                    }
                }
                break;
            }
    #endif
        }
    }
#endif
