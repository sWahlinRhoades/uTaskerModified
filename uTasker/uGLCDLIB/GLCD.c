/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      GLCD.c [Graphic LCD - mono-color with parallel interface]
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    10.08.2009 Rework fnWriteTextBuffer() to handle larger fonts         {1}
    09.08.2009 Add extra graphic functions                               {2}
    26.09.2009 Move to uGLCDLIB folder with Toshiba/Samsung plus OLED, CGLCD and TFT modes {3}
    27.09.2009 Add fnDoLCD_scroll                                        {4}
    27.12.2009 Adjust the alignment of the input buffer to ensure long word alignment {5}
    21.02.2010 Remove old code setting the input buffer (see {5})        {6}
    29.03.2010 Add optional adjustable pixel colour                      {7}
    13.05.2010 Add KITRONIX_GLCD_MODE                                    {8}
    26.06.2010 Add MB785 TFT module                                      {9}
    08.08.2010 Add CGLCD_PIXEL_SIZE to control the GLCD display pixel size {10}
    22.09.2010 Avoid setting invalid window size                         {11}
    23.01.2011 Limit dimension of rectangle to array size                {12}
    05.03.2011 Add windowed picture support                              {13}
    06.03.2011 Change TASK_APPLICATION to LCD_PARTNER_TASK               {14}
    08.03.2011 Increase input buffer size to respect boundary requirements{15}
    21.05.2011 Enable fnDisplayBitmap() generally when SD card active    {16}
    05.09.2012 Enabe fnDisplayBitmap() for SUPPORT_TFT                   {17}
    06.10.2012 Add TFT compatibility with IAR_LPC1788_SK                 {18}
    12.08.2013 Add ST7565S_GLCD_MODE                                     {19}
    12.08.2013 Add inverse mode to bit maps with samsung type display    {20}
    14.08.2013 Protect LCD image uploads to size of the display          {21}
    20.12.2014 Add TWR_LCD_RGB_GLCD_MODE support
    08.03.2017 Add FT800 support                                         {22}
    24.10.2020 Include touch.h header for various touch screen methods   {23}
    28.10.2020 Add character LCD emulation                               {24}

*/        

#include "config.h"


#if defined SUPPORT_GLCD && !defined GLCD_COLOR                          // activate this define to support monochrome GLCD

/* =================================================================== */
/*                           include files                             */
/* =================================================================== */

#include "glcd_fonts.h"                                                  // link in the const fonts as used by the project

#include "glcd_samsung.h"                                                // import GLCD (or emulated GLCD) defines
#include "glcd_toshiba.h"
#include "glcd_st7565s.h"                                                // {19}
#include "glcd_oled.h"
#include "glcd_tft.h"
#include "glcd_nokia.h"
#include "glcd_cglcd.h"

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */


#define MAX_GLCD_READY              100                                  // this many maximum tries will be made to detect the GLCD on power up. If hasn't been detected after this many it will be assumed that no GLCD is connected and the interface doesn't disturb the processor

#define OWN_TASK                    TASK_LCD

#define STATE_INIT                  0                                    // GLCD task states
#define STATE_INITIALISING          1
#define STATE_POWER_LCD_0           2
#define STATE_POWER_LCD_1           3
#define STATE_POWER_LCD_2           4
#define STATE_INITIALISING_1        5
#define STATE_INITIALISING_2        6
#define STATE_INITIALISING_3        7
#define STATE_INITIALISING_4        8
#define STATE_INITIALISING_5        9
#define STATE_INITIALISING_6        10
#define STATE_LCD_WRITING           11
#define STATE_LCD_CLEARING_DISPLAY  12
#define STATE_LCD_READY             13

#define E_INIT_DELAY                1

#define T_BLINK                     2
#define E_STABILISE_DELAY           3
#define T_FADE                      4

#define BLINK_NOTHING               0
#define BLINK_NEXT                  1
#define BLINK_RECT                  2
#define BLINK_LINE                  3
#define BLINK_DELETE_ALL            4

#define PEN_STATE_CHANGE            1                                    // interrupt events

#if defined VARIABLE_PIXEL_COLOUR                                        // {7}
    #define _LCD_PIXEL_COLOUR       LCD_pixel_colour                     // active pixel colour
    #define _LCD_ON_COLOUR          LCD_on_colour                        // disabled pixel colour when LCD is enabled (not turned off)
#else
    #define _LCD_PIXEL_COLOUR       LCD_PIXEL_COLOUR
    #define _LCD_ON_COLOUR          LCD_ON_COLOUR
#endif

#if defined SUPPORT_TOUCH_SCREEN
    #define TOUCH_SCREEN_DEFINES
        #include "touch.h"                                               // {23}
    #undef TOUCH_SCREEN_DEFINES
#endif

/* =================================================================== */
/*                      local structure definitions                    */
/* =================================================================== */

typedef struct stLCD_OBJECTS {
    DELAY_LIMIT      blink_interval;
    DELAY_LIMIT      blink_count_down;
    RECT_COORDINATES blink_coordinates;
    unsigned char    ucObjectType;
} LCD_OBJECTS;


/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static int  fnSmartUpdate(int iStart);
static int  fnWritePic(GLCD_PIC *pic);
static int  fnWriteLine(GLCD_LINE_BLINK *line);
#if defined E_LCD_RECT
    static int  fnWriteRect(GLCD_RECT_BLINK *rect, unsigned short usRadius, unsigned char ucPixelArray[Y_BYTES][X_BYTES]);
#endif
#if defined E_LCD_CIRCLE
    static int  fnWriteCircle(GLCD_CIRCLE *circle, unsigned char ucPixelArray[Y_BYTES][X_BYTES]);
#endif
static int  fnWriteScroll(GLCD_SCROLL *scroll);
#if defined FONTS_AVAILABLE
    static int  fnWriteText(GLCD_TEXT_POSITION *text_position, unsigned char ucLen, QUEUE_HANDLE PortIDInternal);
#endif
#if defined E_LCD_POLY
    static int fnDrawPolygon(GLCD_POLY *poly, unsigned char ucPoints);
#endif
#if defined E_LCD_PIC_RECOLOR
    static int fnRecolorPic(IMAGE_RECOLOR *recolor);
#endif
#if defined MAX_BLINKING_OBJECTS
    static int  fnConfigureBlink(unsigned char ucObjectType, void *object);
#endif
#if defined E_LCD_CANVAS
    static void fnHandleCanvas(RECT_COORDINATES *ptrRect, unsigned char ucCopyPaste);
#endif
#if defined SUPPORT_TOUCH_SCREEN
    static void fnWriteTouchPoint(GLCD_TOUCH_POINT *ptrTouchPoint);
    static void  fnFadeObject(void);
#endif
#if !defined FT800_GLCD_MODE
    static void fnClearScreen(int iMask);
#endif
#if !defined TFT_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined MB785_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE
    static int fnCheckLCD(void);
#endif
#if defined SUPPORT_TOUCH_SCREEN
    static int fnHandleTouchKey(unsigned short usX, unsigned short usY, unsigned char ucKeyID, int iPressedReleased);
        #define PEN_DOWN_EVENT   1
        #define PEN_MOVE_EVENT   -1
        #define PEN_UP_EVENT     0
#elif !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT && (defined SUPPORT_TOUCH_SCREEN && (defined MB785_GLCD_MODE || defined TOUCH_FT6206 || defined TOUCH_FT5406))
    static void fnHandleTouch(unsigned short usX, unsigned short usY, int iPenDown);
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

#if defined SUPPORT_TOUCH_SCREEN
    #define TOUCH_SCREEN_CONSTS
        #include "touch.h"                                               // {23}
    #undef TOUCH_SCREEN_CONSTS
#endif

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static int iLCD_State = STATE_INIT;
static int iSendAck = 0;

static unsigned char ucPixelArray[Y_BYTES][X_BYTES];
static unsigned char ucByteUpdateArray[UPDATE_HEIGHT][UPDATE_WIDTH];

#if defined SUPPORT_GLCD && defined TWR_LCD_RGB_GLCD_MODE && defined TFT_GLCD_MODE
    static COLORREF LCD_transparent_colour = RGB(0, 0, 0);
    static unsigned long ulPixelStyle = 0;
#endif
#if defined VARIABLE_PIXEL_COLOUR                                         // {7}
    static COLORREF LCD_pixel_colour = LCD_PIXEL_COLOUR;
    static COLORREF LCD_on_colour = LCD_ON_COLOUR;
#endif
#if defined GLCD_LCD_EMULATION                                           // {24}
    static GLCD_TEXT_POSITION lcdEmulationTextPosition = {0};
#endif
#if defined GLCD_INVERT_MASK
    static unsigned char ucByteUpdateArrayInvert[Y_BYTES][X_BYTES];      // inversion mask 
    static unsigned char ucByteUpdateArrayBlink[Y_BYTES][X_BYTES];       // blink mask
    static int iBlinkCycle = 0;
#endif
#if defined FT800_GLCD_MODE                                              // {22}
    static Ft_Gpu_Hal_Context_t ft800_host = {0};
    ft_uint32_t Ft_CmdBuffer_Index = 0;
    ft_uint32_t Ft_DlBuffer_Index;
    ft_uint8_t  Ft_DlBuffer[FT_DL_SIZE];
    ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif
#if defined SUPPORT_TOUCH_SCREEN && (defined MB785_GLCD_MODE || defined TOUCH_FT6206 || defined TOUCH_FT5406) // {9}
    #define TOUCH_SCREEN_VARIABLES
        #include "touch.h"                                               // {23}
    #undef TOUCH_SCREEN_VARIABLES
    #define TOUCH_SCREEN_FUNCTIONS
        #include "touch.h"                                               // {23}
    #undef TOUCH_SCREEN_FUNCTIONS
#endif



#if defined SUPPORT_GLCD && defined TWR_LCD_RGB_GLCD_MODE && defined TFT_GLCD_MODE
static unsigned short usRepeatX = 1;
static unsigned short usRepeatY = 1;

static void fnSetRep(unsigned short usRepX, unsigned short usRepY) // temp
{
    usRepeatX = usRepX;
    usRepeatY = usRepY;
}
#endif


// The GLCD task
//
extern void fnLCD(TTASKTABLE *ptrTaskTable)                              // LCD task
{
#if defined E_LCD_POLY || (defined SUPPORT_TOUCH_SCREEN && defined MB785_GLCD_MODE)
    static unsigned char _ucInputMessage[HEADER_LENGTH + 3 + 8 + 7 + 20];// {15}
#else
    static unsigned char _ucInputMessage[HEADER_LENGTH + 3 + 8 + 7];     // {5}{15} reserve space for receiving message headers and extension data
#endif
    static unsigned char *ucInputMessage;
    static unsigned char  ucPictureBoundary = 1;
    QUEUE_HANDLE PortIDInternal = ptrTaskTable->TaskID;                  // queue ID for task input

    if (iLCD_State != STATE_LCD_READY) {
        if ((iLCD_State > STATE_INITIALISING) && (GLCD_BUSY())) {
            return;                                                      // LCD is busy so continue waiting
        }
        switch (iLCD_State) {
        case STATE_INIT:                                                 // initialise the LCD hardware once on startup - reset is assumed to be held low up to this point
            {
            GLCD_PIC *pic;                                               // {6}
            ucInputMessage = _ucInputMessage;
            if (((CAST_POINTER_ARITHMETIC)ucInputMessage & 0x1) == 0) {
                ucInputMessage++;                                        // the input buffer must start on an uneven boundary to ensure struct alignment of read messages
            }
            pic = (GLCD_PIC *)&ucInputMessage[1];                        // since pictures use pointers an additional picture alignment may be necessary too
            while (((CAST_POINTER_ARITHMETIC)&pic->ptrPic & 0x3) != 0) { // {5}{6}
                pic = (GLCD_PIC *)&ucInputMessage[++ucPictureBoundary];  // try next
            }
#if defined TFT_GLCD_MODE                                                // only LPC24xx/LPC17xx/TWR_K70F120M
            #define GLCD_INIT
            #include "glcd_tft.h"                                        // include initialisation code
            #undef GLCD_INIT
#elif (defined CGLCD_GLCD_MODE && (defined AVR32_EVK1105 || defined EK_LM3S3748)) || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE
            #define GLCD_INIT
            #include "glcd_cglcd.h"                                      // include initialisation code
            #undef GLCD_INIT
#else
            GLCD_RST_H();                                                // release the reset line
            if (fnCheckLCD() != 0) {
                fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_ERROR); // inform the application that the GLCD has not been detected
                return;
            }
    #if defined OLED_GLCD_MODE
            #define GLCD_INIT
            #include "glcd_oled.h"                                       // include initialisation code
            #undef GLCD_INIT
    #elif defined NOKIA_GLCD_MODE
            #define GLCD_INIT
            #include "glcd_nokia.h"                                      // include initialisation code
            #undef GLCD_INIT
    #elif defined _GLCD_SAMSUNG                                          // samsung controller
          //while (GLCD_BUSY()) {};
            fnWriteGLCD_cmd(GLCD_ON, GLCD_CHIP0);                        // power on first chip
        #if SAMSUNG_CHIPS > 1
            while (GLCD_BUSY()) {};
            fnWriteGLCD_cmd(GLCD_ON, GLCD_CHIP1);                        // power on second chip
        #endif
            while (GLCD_BUSY()) {};
            fnWriteGLCD_cmd(GLCD_DISP_START, GLCD_CHIP0);
        #if SAMSUNG_CHIPS > 1
            while (GLCD_BUSY()) {};
            fnWriteGLCD_cmd(GLCD_DISP_START, GLCD_CHIP1);
        #endif  
            fnClearScreen(0);                                            // clear display               
    #elif defined ST7565S_GLCD_MODE                                      // {19}
        #if defined MGCF16404B
            fnWriteGLCD_cmd(0xa2);	                                     // bias 1/6 -> 1/9
            fnWriteGLCD_cmd(0xa1);	                                     // normal display -> SEG direction reverse
            fnWriteGLCD_cmd(0xa6);	                                     // normal display -> normal display (not inversed)
            fnWriteGLCD_cmd(0xc8);	                                     // com direction -> COM direction reversed
            fnWriteGLCD_cmd(0xa0);	                                     // SEG direction -> SEG direction normal

            fnWriteGLCD_cmd(0xf8); fnWriteGLCD_cmd(0x00);                // booster ratio 4x

            fnWriteGLCD_cmd(0x2c);	                                     // power control -> booster only on
            fnWriteGLCD_cmd(0x2e);                                       // -> booster + VR on
            fnWriteGLCD_cmd(0x2f);                                       // -> booster + VR + VF on

            fnWriteGLCD_cmd(0x22);	                                     // ra/rb=4

            fnWriteGLCD_cmd(0x81); fnWriteGLCD_cmd(0x1a);                // set voltage -> set electronic volume to 0x1a

            fnWriteGLCD_cmd(0x40);	                                     // start line
            fnWriteGLCD_cmd(0xaf);	                                     // display on
        #else
            fnWriteGLCD_cmd(GLCD_RESET);                                 // reset	            (0xe2)
            fnWriteGLCD_cmd(GLCD_ON);                                    // display on          (0xaf)
            fnWriteGLCD_cmd(GLCD_POWER_ON);                              // power on            (0x2f)
            fnWriteGLCD_cmd(GLCD_VERT_INV);                              //                     (0xc8)
            fnWriteGLCD_cmd(GLCD_DEFAULT_CONTRAST);                      // default contrast    (0x26)                                                                                                  
            fnWriteGLCD_cmd(GLCD_DISP_START);                            // set display start line to 0 (0x40)
        #endif
        #if defined GLCD_INVERT_MASK
            fnClearScreen(1);                                            // clear display and inversion mask
        #else
            fnClearScreen(0);                                            // clear display
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
    #else                                                                // toshiba controller
            fnCommandGlcd_2(SET_GRAPHICS_HOME_ADDRESS, 0x0000);          // set graphics start at zero
            fnCommandGlcd_2(SET_GRAPHICS_HORIZ_BYTES, X_BYTES);          // set the number of horizontal bytes
            fnCommandGlcd_2(SET_TEXT_HOME_ADDRESS, X_BYTES*GLCD_Y);      // set text start after graphics memory
            fnCommandGlcd_2(SET_TEXT_HORIZ_BYTES, X_BYTES);              // set number of text characters on a row
            fnClearScreen(0);
            while (GLCD_BUSY()) {}                                       // wait until the GLCD can accept the next command
            fnWriteGLCD_cmd(GRAPHIC_MODE_XOR);                           // set graphic mode
            while (GLCD_BUSY()) {}                                       // wait until the GLCD can accept the next command
            fnWriteGLCD_cmd(DISABLE_TEXT_DISABLE_CURSOR);                // set graphic mode
            fnCommandGlcd_2(SET_ADDRESS_POINTER, 0x0000);                // set the address pointer to the start of graphic memory
    #endif
#endif                                                                   
            fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_INITIALISED); // inform the application that the initialisation has completed
#if defined GLCD_LCD_EMULATION                                           // {24}
            lcdEmulationTextPosition.ucFont = FONT_CHAR_LCD;
            lcdEmulationTextPosition.ucMode = (REDRAW | PAINT_LIGHT);
#endif
            iLCD_State = STATE_LCD_READY;                                // we are ready to work....
            }
            break; 

        case STATE_LCD_WRITING:
#if defined FT800_GLCD_MODE                                              // {22}
            switch (ft800_host.iCoProcessorWait) {
            case FIFO_WRITE_YIELD:
                ft800_host.iCoProcessorWait = NO_YIELD;
                ft800_host.ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(0, REG_CMD_WRITE);
                if (ft800_host.count != 0) {
                    ft800_host.buffer += ft800_host.length;
                    ft800_host.count -= ft800_host.length;
                    if (ft800_host.count != 0) {
                        if (Ft_Gpu_Hal_WrCmdBuf(0, ft800_host.buffer, ft800_host.count) != 0) { // continue
                            return;
                        }
                    }
                }
                break;
            case SWAP_YIELD:                                             // after yielding for the swap the operation can continue without further actions
                ft800_host.iCoProcessorWait = NO_YIELD;
                break;
            }
            iSendAck = 1;
            iLCD_State = STATE_LCD_READY;
            Ft_CmdBuffer_Index = 0;
            uTaskerStateChange(OWN_TASK, UTASKER_STOP);                  // switch to event mode of operation since write has completed
#else
            iLCD_State = fnSmartUpdate(0);                               // continue with display updating
#endif
            if ((iLCD_State == STATE_LCD_READY) && (iSendAck != 0)) {    // if the write has completed
                fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_READY);
                iSendAck = 0;
            }
            break;

#if defined NOKIA_GLCD_MODE || defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined MB785_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
        case STATE_LCD_CLEARING_DISPLAY:
            iLCD_State = fnSmartUpdate(0);                               // continue with display updating
            if (iLCD_State == STATE_LCD_READY) {                         // if the write has completed
    #if defined NOKIA_GLCD_MODE
                WriteSpiCommand(DISON);                                  // display on
    #elif defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
                fnCommandGlcd_1(ST7637_DISPLAY_ON);                      // display on (0x29)
                fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));           // set a window equal to the complete display size
    #elif defined EK_LM3S3748
                fnWriteGLCD_cmd(ST7637_NORMAL_ON);                       // set the display for normal operation 
    #elif defined MB785_GLCD_MODE
                fnCommandGlcd_2(DISPLAY_CONTROL_1, 0x0173);              // 262k color and display ON
    #endif
                ENABLE_BACKLIGHT();                                      // and enable backlight
                fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_INITIALISED); // inform the application that the initialisation has completed
    #if defined SUPPORT_TOUCH_SCREEN
                fnStartTouch();
    #endif
    #if defined GLCD_BACKLIGHT_CONTROL
                fnSetBacklight();
    #endif
              //iLCD_State = STATE_LCD_READY;                            // we are ready to work....
            }
            else {
                iLCD_State = STATE_LCD_CLEARING_DISPLAY;                 // still in the process or clearing display
            }
            break;
#endif
        }

        if (iLCD_State != STATE_LCD_READY) {
            return;
        }
    }

#if defined SUPPORT_TOUCH_SCREEN && (defined MB785_GLCD_MODE || defined TOUCH_FT6206 || defined TOUCH_FT5406) // touch screen via I2C
    #define TOUCH_SCREEN_I2C_RECEPTION
        #include "touch.h"                                               // {23} handle touch screen I2C reception
    #undef TOUCH_SCREEN_I2C_RECEPTION
#endif
    while (fnRead(PortIDInternal, ucInputMessage, HEADER_LENGTH) != 0) { // check task input queue - this only occurs when all previous operation has completed
        switch (ucInputMessage[MSG_SOURCE_TASK]) {
        case TIMER_EVENT:
#if defined MAX_BLINKING_OBJECTS
            if (T_BLINK == ucInputMessage[MSG_TIMER_EVENT]) {            // blink timer fired
                fnConfigureBlink(BLINK_NEXT, 0);                         // next blink
                break;
            }
#endif
#if defined SUPPORT_TOUCH_SCREEN
            if (T_FADE == ucInputMessage[MSG_TIMER_EVENT]) {             // fade timer fired
                fnFadeObject();
            }
#endif
            break;
        case INTERRUPT_EVENT:
#if defined SUPPORT_TOUCH_SCREEN && (defined MB785_GLCD_MODE || defined TOUCH_FT6206 || defined TOUCH_FT5406) // touch screen via I2C
    #define TOUCH_SCREEN_INTERRUPT_EVENT
        #include "touch.h"                                               // {23} handle touch screen interrupt events
    #undef TOUCH_SCREEN_INTERRUPT_EVENT
#endif
            break;
        case LCD_PARTNER_TASK:                                           // {14}
            {
                unsigned char ucLen = (ucInputMessage[MSG_CONTENT_LENGTH] - 2);
#if defined GLCD_LCD_EMULATION
                fnRead(PortIDInternal, ucInputMessage, 1);               // read the message type
                if (ucInputMessage[0] != E_LCD_TEXT) {
                    fnRead(PortIDInternal, &ucInputMessage[1], 1);       // read the mode
                }
#else
                fnRead(PortIDInternal, ucInputMessage, 2);               // read the message type and mode
#endif
                switch (ucInputMessage[0]) {
#if defined FONTS_AVAILABLE
                case E_GLCD_TEXT:
                    {
                        GLCD_TEXT_POSITION *text_position = (GLCD_TEXT_POSITION *)&ucInputMessage[1];
                        if ((text_position->ucMode & GIVE_ACK) != 0) {   // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[2], (sizeof(GLCD_TEXT_POSITION) - 1));
                        if ((iLCD_State = fnWriteText(text_position, (unsigned char)(ucLen - (sizeof(GLCD_TEXT_POSITION) - 1)), PortIDInternal)) != STATE_LCD_READY) { // start text write
                            return;
                        }
                    }
                    break;
#endif
#if defined E_LCD_POLY
                case E_LCD_POLY:
                    {
                        GLCD_POLY *poly = (GLCD_POLY *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        if ((iLCD_State = fnDrawPolygon(poly, (unsigned char)((ucLen - 1) - ((sizeof(GLCD_POLY) - sizeof(poly->points)))))) != STATE_LCD_READY) { // start drawing poly
                            return;
                        }
                    }
                    break;
#endif
#if defined GLCD_PIC_WINDOWING
                case E_LCD_WINDOWED_PIC:                                 // {13}
#endif
                case E_LCD_PIC:
                    {
                        GLCD_PIC *pic = (GLCD_PIC *)&ucInputMessage[ucPictureBoundary];
                        ucInputMessage[ucPictureBoundary] = ucInputMessage[1];
                        if ((pic->ucMode & GIVE_ACK) != 0) {             // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[ucPictureBoundary + 1], ucLen);
                        if ((iLCD_State = fnWritePic(pic)) != STATE_LCD_READY) { // start image
                            return;
                        }
                    }
                    break;
#if defined E_LCD_PIC_REP
                case E_LCD_PIC_REP:
                    {
                        GLCD_PIC *pic = (GLCD_PIC *)&ucInputMessage[ucPictureBoundary];
                        unsigned short usX_repetitions;
                        unsigned short usY_repetitions;
                        ucInputMessage[ucPictureBoundary] = ucInputMessage[1];
                        if ((pic->ucMode & GIVE_ACK) != 0) {             // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[ucPictureBoundary + 1], ucLen);
                        usX_repetitions = ucInputMessage[ucPictureBoundary + sizeof(GLCD_PIC)];
                        usX_repetitions <<= 8;
                        usX_repetitions |= ucInputMessage[ucPictureBoundary + sizeof(GLCD_PIC) + 1];
                        usY_repetitions = ucInputMessage[ucPictureBoundary + sizeof(GLCD_PIC) + 2];
                        usY_repetitions <<= 8;
                        usY_repetitions |= ucInputMessage[ucPictureBoundary + sizeof(GLCD_PIC) + 3];
                        fnSetRep(usX_repetitions, usY_repetitions); // temp
                        if ((iLCD_State = fnWritePic(pic)) != STATE_LCD_READY) { // start image
                            return;
                        }
                        fnSetRep(1, 1); // temp
                    }
                    break;
#endif
#if defined E_LCD_PIC_RECOLOR
                case E_LCD_PIC_RECOLOR:
                    {
                        IMAGE_RECOLOR *recolor = (IMAGE_RECOLOR *)&ucInputMessage[ucPictureBoundary];
                        GLCD_PIC *pic = &recolor->glcd_pic;
                        ucInputMessage[ucPictureBoundary] = ucInputMessage[1];
                        if ((pic->ucMode & GIVE_ACK) != 0) {             // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[ucPictureBoundary + 1], ucLen);
                        if ((iLCD_State = fnRecolorPic(recolor)) != STATE_LCD_READY) { // start recolor image
                            return;
                        }
                    }
                    break;
#endif
                case E_LCD_LINE:                                         // {1}
                    {
                        GLCD_LINE_BLINK *line = (GLCD_LINE_BLINK *)&ucInputMessage[1];
                        if ((line->ucMode & GIVE_ACK) != 0) {            // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
#if defined MAX_BLINKING_OBJECTS
                        if ((line->ucMode & (BLINKING_OBJECT)) != 0) {
                            fnConfigureBlink(BLINK_LINE, line);
                        }
#endif
                        if ((iLCD_State = fnWriteLine(line)) != STATE_LCD_READY) { // start drawing line
                            return;
                        }
                    }
                    break;
#if defined SUPPORT_TOUCH_SCREEN
                case E_LCD_TOUCH_POINT:
                    {
                        GLCD_TOUCH_POINT *touchPoint = (GLCD_TOUCH_POINT *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        fnWriteTouchPoint(touchPoint);
                    }
                    break;
#endif
#if defined E_LCD_CANVAS
                case E_LCD_CANVAS:
                    {
                        RECT_COORDINATES *rect_coordinates = (RECT_COORDINATES *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        fnHandleCanvas(rect_coordinates, ucInputMessage[ucLen + 1]);
                    }
                    break;
#endif
#if defined E_LCD_RECT
                case E_LCD_RECT:                                         // {1}
                    {
                        GLCD_RECT_BLINK *rect = (GLCD_RECT_BLINK *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
    #if defined MAX_BLINKING_OBJECTS
                        if ((rect->ucMode & (BLINKING_OBJECT)) != 0) {
                            if (fnConfigureBlink(BLINK_RECT, rect) < 0) {
                                break;
                            }
                        }
    #endif
                        if ((rect->ucMode & GIVE_ACK) != 0) {            // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        if ((iLCD_State = fnWriteRect(rect, 0, ucPixelArray)) != STATE_LCD_READY) { // start drawing rectangle
                            return;
                        }
                    }
                    break;
#endif
#if defined E_LCD_ROUND_RECT
                case E_LCD_ROUND_RECT:
                    {
                        GLCD_ROUND_RECT *rect = (GLCD_ROUND_RECT *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        if ((rect->ucMode & GIVE_ACK) != 0) {            // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        if ((iLCD_State = fnWriteRect((GLCD_RECT_BLINK *)rect, rect->usRadius, ucPixelArray)) != STATE_LCD_READY) { // start drawing round rectangle
                            return;
                        }
                    }
                    break;
#endif
#if defined E_LCD_CIRCLE
                case E_LCD_CIRCLE:
                    {
                        GLCD_CIRCLE *circle = (GLCD_CIRCLE *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        if ((circle->ucMode & GIVE_ACK) != 0) {          // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        if ((iLCD_State = fnWriteCircle(circle, ucPixelArray)) != STATE_LCD_READY) { // start drawing rectangle
                            return;
                        }
                    }
                    break;
#endif
#if defined E_LCD_SCROLL
                case E_LCD_SCROLL:
                    {
                        GLCD_SCROLL *scroll = (GLCD_SCROLL *)&ucInputMessage[1];
                        if ((scroll->ucMode & GIVE_ACK) != 0) {          // does the application expect an ack when this operation has completed?
                            iSendAck = 1;
                        }
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        if ((iLCD_State = fnWriteScroll(scroll)) != STATE_LCD_READY) { // start drawing scrolled display
                            return;
                        }
                    }
                    break;
#endif
#if defined VARIABLE_PIXEL_COLOUR
                case E_LCD_STYLE:
                    {
                        GLCD_STYLE *style = (GLCD_STYLE *)&ucInputMessage[1];
                        fnRead(PortIDInternal, &ucInputMessage[2], ucLen);
                        switch (style->ucMode) {
                        case STYLE_TRANSPARENT_COLOR:
                            uMemcpy(&LCD_transparent_colour, &style->color, sizeof(LCD_transparent_colour));
                            ulPixelStyle = 1;                            // transparent colour active
                            break;
                        case STYLE_TRANSPARENT_OFF:
                            ulPixelStyle = 0;                            // transparent colour inactive
                            break;
                        case STYLE_PIXEL_COLOR:
                            uMemcpy(&LCD_pixel_colour, &style->color, sizeof(LCD_pixel_colour));
                            break;
                        case STYLE_BACKGROUND_COLOR:
                            uMemcpy(&LCD_on_colour, &style->color, sizeof(LCD_pixel_colour));
                        }
                    }
                    break;
#endif
#if defined GLCD_LCD_EMULATION                                           // {24}
                    case E_LCD_COMMAND:                                  // character LCD command emulation
                        if ((ucInputMessage[1] & 0x80) != 0) {           // write to DDR RAM address
                            unsigned char ucLocation = (ucInputMessage[1] & 0x3f);
                            lcdEmulationTextPosition.usX = (EMULATED_LCD_START_X + (ucLocation * EMULATED_LCD_CHARACTER_WIDTH));
                            if ((ucInputMessage[1] & 0x40) != 0) {       // second line
                                lcdEmulationTextPosition.usY = (EMULATED_LCD_START_Y + EMULATED_LCD_LINE_HEIGHT);
                            }
                            else {                                       // first line
                                lcdEmulationTextPosition.usY = EMULATED_LCD_START_Y;
                            }
                        }
                        else if ((ucInputMessage[1] & 0x40) != 0) {      // write to CG RAM address
                        }
                        else if ((ucInputMessage[1] & 0x20) != 0) {      // function set
                        }
                        else if (ucInputMessage[1] == 0x01) {            // clear LCD display
                            GLCD_RECT_BLINK rect;
                            rect.ucMode = (REDRAW | PAINT_DARK);
                            rect.rect_corners.usX_start = EMULATED_LCD_START_X;
                            rect.rect_corners.usY_start = EMULATED_LCD_START_Y;
                            rect.rect_corners.usX_end = (EMULATED_LCD_START_X + (EMULATED_LCD_CHARACTERS * EMULATED_LCD_CHARACTER_WIDTH));
                            rect.rect_corners.usY_end = (EMULATED_LCD_START_Y + (EMULATED_LCD_LINES * EMULATED_LCD_LINE_HEIGHT));

                            if ((iLCD_State = fnWriteRect(&rect, 0, ucPixelArray)) != STATE_LCD_READY) { // start drawing rectangle
                                return;
                            }
                        }
                        else {
                            _EXCEPTION("Use new method!");
                        }
                        iSendAck = 1;                                    // return an ack after the command has been processed
                        break;
                    case E_LCD_TEXT:                                     // character LCD text
                        iLCD_State = fnWriteText(&lcdEmulationTextPosition, (ucLen + 1), PortIDInternal); // start text write
                        lcdEmulationTextPosition.usX += ((ucLen + 1) * EMULATED_LCD_CHARACTER_WIDTH); // advance the cursor
                        if (iLCD_State != STATE_LCD_READY) {
                            return;
                        }
                        iSendAck = 1;                                    // return an ack after the command has been processed
                        break;
#endif
                }
                if (iSendAck != 0) {
                    iSendAck = 0;
                    fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_READY); // last operation has been completed
                }
            }
            break;
        default:
            while (fnRead(PortIDInternal, ucInputMessage, 1) != 0) {};  // flush unexpected messages
            break;
        }
    }
}

#define _GLCD_COMMANDS                                                   // hardware interface commands depending on display type
    #include "glcd_samsung.h"
    #include "glcd_toshiba.h"
    #include "glcd_oled.h"
    #include "glcd_tft.h"
    #include "glcd_nokia.h"
    #include "glcd_cglcd.h"
#undef _GLCD_COMMANDS

#if defined SUPPORT_GLCD && defined TWR_LCD_RGB_GLCD_MODE && defined TFT_GLCD_MODE

#if defined _WINDOWS
    #if defined _iMX && defined eLCD_IN_OCR2
        #define fnRedrawSimDisplay() CollectCommand(0, (unsigned long)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2))
    #else
        #define fnRedrawSimDisplay() CollectCommand(0, (unsigned long)fnGetSDRAM((unsigned char *)SDRAM_ADDR))
    #endif
#else
    #define fnRedrawSimDisplay()
#endif

#if defined E_LCD_CANVAS
    static COLORREF *ptrCanvas = 0;
    static unsigned short usCanvasWidth = 0;
    static POINT_COORDINATE canvas_location = {0};
#endif

static COLORREF *fnGetDesinationMemory(RECT_COORDINATES *ptrRect, unsigned short *ptrWidth)
{
    COLORREF *ptDst;
#if defined _iMX && defined eLCD_IN_OCR2
    ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
#else
    ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
#endif
    if (ptrRect != 0) {
#if defined E_LCD_CANVAS
        if (ptrCanvas != 0) {
            ptrRect->usX_start -= canvas_location.usX;
            ptrRect->usX_end -= canvas_location.usX;
            ptrRect->usY_start -= canvas_location.usY;
            ptrRect->usY_end -= canvas_location.usY;
            *ptrWidth = usCanvasWidth;
            return ptrCanvas;
        }
#endif
        *ptrWidth = GLCD_X;
    }
    return ptDst;
}

// Presently assumes TFT with frame buffer
//
static void fnPaintBitMap(unsigned short usX, unsigned short usY, unsigned short usWidth, unsigned short usHeight, unsigned char *ucPixelData, unsigned char ucMode, RECT_COORDINATES *window)
{
    int iIncrementX;
    int iIncrementY;
    int iOffsetX;
    int iOffsetY;
    unsigned char *ucThisPixelData;
    unsigned short usRepX = usRepeatX;
    unsigned short usRepY = usRepeatY;
    unsigned short usThisX;
    unsigned short usThisY;
    unsigned short usThisHeight;
    int iTransparentColourActive = ((ulPixelStyle & 1) != 0);
    unsigned short ucLineWidth;
    volatile register unsigned short usColor;                            // set volatile to stop GCC from optimising byte reads combined to unsigned short with half-word accesses
    COLORREF *ptStart;
    COLORREF *ptDst;
    COLORREF *ptDstRef;
    RECT_COORDINATES DrawRect;
    unsigned short usDestinationWidth = 0;
    if ((ucMode & 0x01)  != 1) {                                         // expect RGB565 format only
        return;
    }
    if ((ucMode & WINDOWED_PIC) != 0) { // temp flip x
        iIncrementX = -1;
        iOffsetX = usWidth;
    }
    else {
        iIncrementX = 1;
        iOffsetX = 0;
    }
    if ((ucMode & WINDOWED_PIC) != 0) { // temp flip y
        iIncrementY = -1;
        iOffsetY = (usHeight - 1);
    }
    else {
        iIncrementY = 1;
        iOffsetY = 0;
    }
    DrawRect.usX_start = usX;
    DrawRect.usY_start = usY;
    ptDstRef = fnGetDesinationMemory(&DrawRect, &usDestinationWidth);
    do {
        ptDst = ptDstRef;
        ucThisPixelData = ucPixelData;
        usThisX = DrawRect.usX_start;
        usThisY = (DrawRect.usY_start + iOffsetY);
        usThisHeight = usHeight;
        ptStart = (ptDst + usThisX);
        while (usThisHeight-- != 0) {                                    // for each horizontal line
            ptDst = (ptStart + iOffsetX + (usThisY * usDestinationWidth));
            ucLineWidth = usWidth;
            while (ucLineWidth-- != 0) {                                 // for each vertical line
                usColor = *ucThisPixelData++;
                usColor <<= 8;
                usColor |= *ucThisPixelData++;
                if ((iTransparentColourActive == 0) || (usColor != LCD_transparent_colour)) {
                    *ptDst = usColor;
                }
                ptDst += iIncrementX;
            }
            usThisY += iIncrementY;
        }
        DrawRect.usX_start += usWidth;
    } while (usRepX-- > 1);
    fnRedrawSimDisplay();
}

static void fnPaintBitMapAntiAlias(unsigned char ucUpsample, unsigned short usX, unsigned short usY, unsigned short usWidth, unsigned short usHeight, unsigned char *ucPixelData, unsigned char ucMode, RECT_COORDINATES *window)
{
    int iTransparentColourActive = 1;// ((ulPixelStyle & 1) != 0);
    unsigned short ucLineWidth;
    volatile register unsigned short usColor;                            // set volatile to stop GCC from optimising byte reads combined to unsigned short with half-word accesses
    unsigned char *ucNextPixelData;
    COLORREF *ptStart;
    COLORREF *ptDst;
    if (ucMode != 1) {                                                   // expect RGB565 format only
        return;
    }
    #if defined _iMX && defined eLCD_IN_OCR2
    ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
    #else
    ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
    #endif

    ptStart = (ptDst + usX);

    while (usHeight != 0) {                                              // for each horizontal line
        ptDst = (ptStart + (usY * GLCD_X));
        ucLineWidth = usWidth;
        while (ucLineWidth != 0) {                                       // for the line content
            if (ucUpsample == 1) {
                usColor = *ucPixelData++;
              //usColor <<= 8;
              //usColor |= *ucPixelData++;
                usColor |= (*ucPixelData++ << 8);
                ucLineWidth--;
            }
            else {
                unsigned long ulAccumulatedRed = 0;
                unsigned long ulAccumulatedGreen = 0;
                unsigned long ulAccumulatedBlue = 0;
                int ixSample = 0;
                int iySample = 0;
                ucNextPixelData = (ucPixelData + (ucUpsample * sizeof(COLORREF)));
                while (iySample < ucUpsample) {
                    while (ixSample < ucUpsample) {
                        usColor = *ucPixelData++;
                        //usColor <<= 8;
                        //usColor |= *ucPixelData++;
                        usColor |= (*ucPixelData++ << 8);
                        if (usColor == 0xf800) {
                            usColor = usColor;
                        }
                        usColor = ~usColor;
                        ulAccumulatedRed += (usColor >> 11);
                        ulAccumulatedGreen += ((usColor >> 5) & 0x3f);
                        ulAccumulatedBlue += (usColor & 0x1f);
                        ixSample++;
                    }
                    ixSample = 0;
                    iySample++;
                    ucPixelData += (usWidth * sizeof(COLORREF));
                    ucPixelData -= (ucUpsample * sizeof(COLORREF));
                }
                ucLineWidth -= ucUpsample;
                ucPixelData = ucNextPixelData;
                ulAccumulatedRed /= (ucUpsample * ucUpsample);
                ulAccumulatedGreen /= (ucUpsample * ucUpsample);
                ulAccumulatedBlue /= (ucUpsample * ucUpsample);
                usColor = (unsigned short)((ulAccumulatedRed << 11) | ((ulAccumulatedGreen & 0x3f) << 5) | (ulAccumulatedBlue & 0x1f));
                usColor = ~usColor;
            }
            if ((iTransparentColourActive == 0) || (usColor != 0xffff)) {
                *ptDst = usColor;
            }
            ptDst++;
        }
        ucPixelData += ((usWidth * sizeof(COLORREF) * (ucUpsample - 1)));
        usHeight -= ucUpsample;
        usY++;
    }
    fnRedrawSimDisplay();
}
#endif

// Bit map writing routine, used for application bit maps and also for font based characters
//
static void fnWriteBitMap(unsigned short x, unsigned short y,
               unsigned short usWidth, unsigned short usHeight,
               const unsigned char *glyph_ptr, unsigned char ucMode,
               RECT_COORDINATES *window)
{
#if (defined _GLCD_SAMSUNG || defined ST7565S_GLCD_MODE) && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE // {19}
    register unsigned char ucMemoryContent;
    register unsigned char ucOriginal_content;
    int iX = x;
    int iY = y/8;
    int iDotY = (0x01 << (y%8));
    int iDotX = 0x80;
    int iWidthScan;
    while (usHeight-- != 0) {
        int iProcessed = 0;
        iWidthScan = usWidth;
        iX = x;
        while (iWidthScan-- != 0) {
            if (iX < X_BYTES) {                                          // clip to right
                ucMemoryContent = ucOriginal_content = ucPixelArray[iY][iX];
                if ((window == 0) || (((window->usX_start <= iX) && (window->usX_end >= iX)) && ((window->usY_start <= y) && (window->usY_end >= y)))) { // {13} if a window is defined write only within its bounds
                    if ((*glyph_ptr & iDotX) != 0) {
                        if ((ucMode & PAINT_DARK) != 0) {                // {20}
                            ucMemoryContent &= (unsigned char)~iDotY;    // clear the pixel
                        }
                        else {
                            ucMemoryContent |= (unsigned char)iDotY;     // set the pixel
                        }
                    }
                    else {
                        if ((ucMode & PAINT_DARK) != 0) {                // {20}
                            ucMemoryContent |= (unsigned char)iDotY;     // set the pixel
                        }
                        else {
                            ucMemoryContent &= (unsigned char)~iDotY;    // clear the pixel
                        }
                    }
                    if (ucMemoryContent != ucOriginal_content) {         // if change to the memory
                        ucPixelArray[iY][iX] = ucMemoryContent;          // write the change to the pixel array
                        ucByteUpdateArray[iY/8][iX] |= (0x01 << iY);     // mark that this byte needs to be updated
                    }
                }
                iX++;
            }
            iDotX >>= 1;
            if (iDotX == 0) {
                iDotX = 0x80;
            }
            if (++iProcessed >= 8) {
                glyph_ptr++;
                iDotX = 0x80;
                iProcessed = 0;
            }
        }
        y++;
        iDotY <<= 1;
        if (iDotY > 0x80) {
            iDotY = 0x01;
            iY++;
            if (iY >= Y_BYTES) {
                return;                                                  // clipping at bottom
            }
        }
        if (iProcessed != 0) {
            glyph_ptr++;
            iDotX = 0x80;
            iProcessed = 0;
        }
    }
#else
    register unsigned char ucMemoryContent;
    register unsigned char ucOriginal_content;
    int iX_array;
    int iX_bytewidth;
    int iX_offset;

    iX_array = (x/8);
    iX_offset = (x%8);
    iX_bytewidth = ((iX_offset + usWidth + 7)/8);

    while (usHeight-- != 0) {                                            // insert the text, overwriting existing content
        int i = 0;
        int iCharacterOffset = iX_offset;
        int iRemaining = usWidth;
        int iProcessed;
        unsigned char ucMask;
        if (y >= Y_BYTES) {
            break;
        }
        while ((i < iX_bytewidth) && ((iX_array + i) < X_BYTES)) {
            ucMask = 0xff;
            ucOriginal_content = ucMemoryContent = ucPixelArray[y][iX_array + i];
            if (iCharacterOffset >= 0) {
                ucMask <<= (8 - iCharacterOffset);
                if ((iRemaining + iCharacterOffset) < 8) {
                    ucMask |= (0xff >> (iRemaining + iCharacterOffset));
                    iProcessed = 8;
                }
                else {
                    iRemaining -= (8 - iCharacterOffset);                // the remaining pixels in the x-direction
                    if (iRemaining == 0) {
                        iProcessed = 8;
                    }
                    else {
                        iProcessed = (8 - iCharacterOffset);
                    }
                }
                ucMemoryContent &= ucMask;                               // clear space for the new text pixels
                if ((ucMode & PAINT_DARK) != 0) {
                    ucMemoryContent |= ((~(*glyph_ptr) >> iCharacterOffset) & ~ucMask); // insert new pixel content
                }
                else {
                    ucMemoryContent |= ((*glyph_ptr >> iCharacterOffset) & ~ucMask); // insert new pixel content
                }
                ucPixelArray[y][iX_array + i] = ucMemoryContent;
                if (ucMemoryContent != ucOriginal_content) {
                    ucByteUpdateArray[y][(iX_array + i)/8] |= (0x80 >> ((iX_array + i)%8)); // mark that this byte needs to be updated
                }
            }
            else {
                if (iRemaining < (-iCharacterOffset)) {
                    ucMask >>= iRemaining;
                }
                else {
                    ucMask >>= (-iCharacterOffset);
                }
                ucMemoryContent &= ucMask;                               // clear space for the new text pixels
                if ((ucMode & PAINT_DARK) != 0) {
                    ucMemoryContent |= ((~(*glyph_ptr++) << (8 + iCharacterOffset)) & ~ucMask); // insert new pixel content
                }
                else {
                    ucMemoryContent |= ((*glyph_ptr++ << (8 + iCharacterOffset)) & ~ucMask); // insert new pixel content
                }
                iCharacterOffset = -iCharacterOffset;
                ucPixelArray[y][iX_array + i] = ucMemoryContent;
                if (ucMemoryContent != ucOriginal_content) {
                    ucByteUpdateArray[y][(iX_array + i)/8] |= (0x80 >> ((iX_array + i)%8)); // mark that this byte needs to be updated
                }
                if (iCharacterOffset >= iRemaining) {
                    break;                                               // work done for this row
                }
                iRemaining -= iCharacterOffset;                          // the remaining pixels in the x-direction
                continue;
            }
            if (iProcessed >= 8) {
                glyph_ptr++;
            }
            i++;
            iCharacterOffset = -iCharacterOffset;
        }
        y++;
        if ((iX_array + i) >= X_BYTES) {                                 // check for bitmap clipping to right
            while (i < iX_bytewidth) { 
                glyph_ptr++;                                             // skip non-visible input
                i++;
            }
        }
    }
#endif
}


#if defined FONTS_AVAILABLE
// Write text to the backup buffer at the coordinate using defined font
//
static void fnWriteTextBuffer(GLCD_TEXT_POSITION *text_position, CHAR *str, unsigned char ucStrLength)
{
    unsigned char font = text_position->ucFont;
    unsigned short x = text_position->usX;
    unsigned char ucGlyph;
    unsigned char ucWidth;
    unsigned char ucHeight;
    const unsigned char *glyph_ptr;

    while (ucStrLength-- != 0) {                                         // for each string character
        ucGlyph = (unsigned char)*str++;
        if ((ucGlyph < fonts[font].glyph_beg) || (ucGlyph > fonts[font].glyph_end)) { // check to make sure the symbol is a legal one if not then just replace it with the default character 
            ucGlyph = fonts[font].glyph_def;                             // replace by default
        }
        ucGlyph -= fonts[font].glyph_beg;                                // array reference             
        ucHeight = fonts[font].glyph_height;
        glyph_ptr = fonts[font].glyph_table + (ucGlyph * ucHeight);
        if ((ucWidth = fonts[font].fixed_width) == 0) {                  // check if it is a variable width font
            ucWidth = (fonts[font].width_table[ucGlyph] & 0xff);         // get the variable width instead 
            glyph_ptr += ((fonts[font].width_table[ucGlyph] >> 8) * ucHeight); // compensate for font widths of more than one byte
        }
        else if (fonts[font].width_table != 0) {                         // fixed width
            glyph_ptr += ((fonts[font].width_table[ucGlyph] >> 8) * ucHeight); // compensate for font widths of more than one byte
            if ((fonts[font].width_table[ucGlyph] & 0xff) > ucWidth) {   // exception - the fixed with is less than real character width so widen
                ucWidth = (fonts[font].width_table[ucGlyph] & 0xff);     // get the variable width instead 
            }
        }
        else if (fonts[font].fixed_width > 8) {                          // if fixed width is more that 8 bits
            glyph_ptr += (ucGlyph * (ucHeight * ((fonts[font].fixed_width)/8)));
        }
    #if defined ST7565S_GLCD_MODE && defined MGCF16404B
        ucWidth--;                                                       // no space between characters added because the display is divided into character groups
    #endif
        fnWriteBitMap(x, text_position->usY, ucWidth, ucHeight, glyph_ptr, text_position->ucMode, 0);
        x += ucWidth;                                                    // move right for next character
    }
}

// Read text from the input queue to an intermediate buffer.
//
static int fnWriteText(GLCD_TEXT_POSITION *text_position, unsigned char ucLen, QUEUE_HANDLE PortIDInternal)
{
    CHAR cNewText[MAX_TEXT_LENGTH];                                      // space for the maximum expected text string plus a null terminator
    unsigned char ucLength = ucLen;
    if (ucLength > MAX_TEXT_LENGTH) {                                    // ensure text cannot overwrite buffer end
        ucLength = MAX_TEXT_LENGTH;
    }
    fnRead(PortIDInternal, (unsigned char *)cNewText, ucLength);
    while (ucLen > ucLength) {
        unsigned char ucDummy;
        fnRead(PortIDInternal, &ucDummy, 1);                             // dummy read of any excessive text
        ucLen--;
    }
    fnWriteTextBuffer(text_position, cNewText, ucLength);                // convert the ASCII input buffer to characters defined by the font. These are written to the backup graphics buffer
    if ((text_position->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD if commanded to do so
    }
    return STATE_LCD_READY;                                              // operation complete
}
#endif



// Write a bit map as commanded by the application
//
static int fnWritePic(GLCD_PIC *pic)
{
    RECT_COORDINATES *rect_corners = 0;                                  // the window in the picture
    unsigned short usWidth = fnGetGbitWidth(pic->ptrPic); // ((pic->ptrPic->ucWidth[0] << 8) | pic->ptrPic->ucWidth[1]);
    unsigned short usHeight = fnGetGbitHeight(pic->ptrPic); // ((pic->ptrPic->ucHeight[0] << 8) | pic->ptrPic->ucHeight[1]);
#if defined GLCD_PIC_WINDOWING
    int iIgnore = 0;
    if ((pic->ucMode & WINDOWED_PIC) != 0) {                                    // {13} only a windows from the picture is to be displayed
        rect_corners = &((GLCD_PIC_WINDOW *)pic)->rect_corners;          // the window in the picture
        if ((rect_corners->usX_start >= rect_corners->usX_end) || (rect_corners->usY_start >= rect_corners->usY_end)) {
            iIgnore = 1;
        }
        else {
            rect_corners->usX_start += pic->usX;                             // set relative to picture content
            rect_corners->usX_end += pic->usX;
            rect_corners->usY_start += pic->usY;
            rect_corners->usY_end += pic->usY;
        }
    }
    if (iIgnore == 0) {                                                  // don't write if the window is invalid
        fnWriteBitMap(pic->usX, pic->usY, usWidth, usHeight, pic->ptrPic->ucData, pic->ucMode, rect_corners); // copy bit map to backup graphics buffer
    }
#else
    switch (pic->ucMode & 0x01) {
    case 0:                                                              // monochrome bit map is projected into the smart buffer
        fnWriteBitMap(pic->usX, pic->usY, usWidth, usHeight, pic->ptrPic->ucData, pic->ucMode, rect_corners); // copy bit map to backup graphics buffer
        break;
    #if defined SUPPORT_GLCD && defined TWR_LCD_RGB_GLCD_MODE && defined TFT_GLCD_MODE
    case 1:                                                              // RGB565 16 bit color bit map is painted directly to the display at the projected location
        fnPaintBitMap(pic->usX, pic->usY, usWidth, usHeight, pic->ptrPic->ucData, pic->ucMode, rect_corners); // paint bitmap in raw format
        break;
    default:
        _EXCEPTION("Unknown mode!");
    #endif
    }
#endif
    if ((pic->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD is commanded to do so
    }
    return STATE_LCD_READY;                                              // operation complete
}


#if defined E_LCD_PIC_RECOLOR
static int fnRecolorPic(IMAGE_RECOLOR *recolor)
{
    GLCD_PIC *pic = &recolor->glcd_pic;
    unsigned short usRepX = recolor->usXreps;
    unsigned short usRepY = recolor->usYreps;
    unsigned short usWidth = fnGetGbitWidth(pic->ptrPic);
    unsigned short usHeight;
    unsigned char ucNewRedPart = (((recolor->newColor >> 11) << 1) << 3);
    unsigned char ucNewGreenPart = (((recolor->newColor >> 5) & 0x3f) << 2);
    unsigned char ucNewBluePart = (((recolor->newColor & 0x1f) << 1) << 3);
    RECT_COORDINATES DrawRect;
    register unsigned short usX;
    register unsigned short usY;
    unsigned char *ucPixelData;
    unsigned short ucLineWidth;
    unsigned short usDestinationWidth = 0;
    volatile register unsigned short usColor;                            // set volatile to stop GCC from optimising byte reads combined to unsigned short with half-word accesses
    COLORREF *ptStart;
    COLORREF *ptDstRef;
    COLORREF *ptDst;
    int iFillBackground = ((recolor->glcd_pic.ucMode & 0x02) != 0);
    if ((pic->ucMode & 0x01)  != 1) {                                    // expect RGB565 format only
        return STATE_LCD_READY;
    }
    DrawRect.usX_start = pic->usX;
    DrawRect.usY_start = pic->usY;
    ptDstRef = fnGetDesinationMemory(&DrawRect, &usDestinationWidth);
    usX = DrawRect.usX_start;
    do {
        ptDst = ptDstRef;
        ucPixelData = pic->ptrPic->ucData;
        usHeight = fnGetGbitHeight(pic->ptrPic);
        ptStart = (ptDst + usX);
        usY = DrawRect.usY_start;
        while (usHeight-- != 0) {                                            // for each horizontal line
            ptDst = (ptStart + (usY * usDestinationWidth));
            ucLineWidth = usWidth;
            while (ucLineWidth-- != 0) {                                     // for each vertical line
                usColor = *ucPixelData++;
                usColor <<= 8;
                usColor |= *ucPixelData++;
                if (usColor != RGB(255, 255, 255)) {
                    //unsigned char ucRed = (unsigned char)((usColor >> 11) << 1); 
                    unsigned char ucGreen = (unsigned char)(((usColor >> 5) & 0x3f) << 2); // assume reference image is always black so we handle only the green component as representing the itensity
                  //unsigned char ucBlue = (unsigned char)((usColor & 0x1f) << 1);
                    unsigned char ucNewRed = ucNewRedPart;
                    unsigned char ucNewGreen = ucNewGreenPart;
                    unsigned char ucNewBlue = ucNewBluePart;
                    if ((ucGreen + ucNewRed) >= 255) {
                        ucNewRed = 255;
                    }
                    else {
                        ucNewRed += ucGreen;
                    }
                    if ((ucGreen + ucNewGreen) >= 255) {
                        ucNewGreen = 255;
                    }
                    else {
                        ucNewGreen += ucGreen;
                    }
                    if ((ucGreen + ucNewBlue) >= 255) {
                        ucNewBlue = 255;
                    }
                    else {
                        ucNewBlue += ucGreen;
                    }
                    *ptDst = RGB(ucNewRed, ucNewGreen, ucNewBlue);
                }
                else if (iFillBackground != 0) {
                    *ptDst = RGB(255, 255, 255);
                }
                ptDst++;
            }
            usY++;
        }
        usX += usWidth;
    } while (usRepX-- > 1);
    fnRedrawSimDisplay();
    return STATE_LCD_READY;                                              // operation complete
}
#endif

// Draw a line as commanded by the application
//
static int fnWriteLine(GLCD_LINE_BLINK *line)                            // {1}
{
    int iHorizontalDirection;
    int iX_steps = 0x100000;
    int iY_steps = 0x100000;
    int iYpixels, iXpixels;
    unsigned short usX_Start, usY_Start, usX_End, usY_End;
    unsigned char ucPixel;
    unsigned char ucOriginalMemoryContent;
    if (line->line_start_end.usY_start <= line->line_start_end.usY_end) {// always draw from top to bottom
        usY_Start = line->line_start_end.usY_start;
        usY_End   = line->line_start_end.usY_end;
        usX_Start = line->line_start_end.usX_start;
        usX_End   = line->line_start_end.usX_end;
    }
    else {
        usY_Start = line->line_start_end.usY_end;
        usY_End   = line->line_start_end.usY_start;
        usX_Start = line->line_start_end.usX_end;
        usX_End   = line->line_start_end.usX_start;
    }
#if (defined _GLCD_SAMSUNG || defined ST7565S_GLCD_MODE) && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE // {19}    
    iY_steps = (usY_End - usY_Start);                                    // height
    if (usX_Start <= usX_End) {
        iHorizontalDirection = 1;                                        // draw from left to right        
        if (usX_End == usX_Start) {
            iY_steps = 0x7fffffff;                                       // vertical line
        }
        else {
            if ((usX_End - usX_Start) <= iY_steps) {
                iY_steps *= 0x100000;
                iY_steps /= (usX_End - usX_Start);
            }
            else {
                if (usY_End == usY_Start) {
                    iX_steps = 0x7fffffff;                               // horizontal line
                }
                else {
                    iX_steps = (usX_End - usX_Start);                    // width
                    iX_steps *= 0x100000;
                    iX_steps /= iY_steps;
                }
                iY_steps = 0x100000;
            }
        }
    }
    else {
        iHorizontalDirection = -1;                                       // draw from right to left
        if ((usX_Start - usX_End) <= iY_steps) {
            if (usX_Start == usX_End) {                                  // vertical line
                iY_steps = 0x7fffffff;
            }
            else {
                iY_steps *= 0x100000;
                iY_steps /= (usX_Start - usX_End);
            }
            iX_steps = 0x100000;
        }
        else {
            if (iY_steps == 0) {                                         // horizontal line
                iX_steps = 0x7fffffff;
            }
            else {
                iX_steps = (usX_Start - usX_End);                        // width
                iX_steps *= 0x100000;
                iX_steps /= iY_steps;
            }
            iY_steps = 0x100000;
        }
    }
    iYpixels = iY_steps;
    iXpixels = iX_steps;                                                 // the number of pixels to draw in the horizontal direction (left or right)
    do {
        if (usY_Start >= (GLCD_Y/ CGLCD_PIXEL_SIZE)) {
            break;
        }
        do {
            if (usX_Start >= (GLCD_X/ CGLCD_PIXEL_SIZE)) {
                break;
            }
            if (iHorizontalDirection > 0) {
                if (usX_Start > usX_End) {
                    break;
                }
            }
            else {
                if (usX_Start < usX_End) {
                    break;
                }
            }
            ucPixel = (0x01 << (usY_Start%8));                           // position of the pixel to be drawn
            ucOriginalMemoryContent = ucPixelArray[usY_Start/8][usX_Start]; // get the original content
            if ((line->ucMode & (PAINT_INVERTED | PAINT_DARK)) != 0) {
                if ((line->ucMode & PAINT_DARK) != 0) {
                    ucPixelArray[usY_Start/8][usX_Start] &= ~ucPixel;    // clear the pixel
                }
                else {
                    ucPixelArray[usY_Start/8][usX_Start] = (ucOriginalMemoryContent ^ ucPixel); // set the pixel to the inverted value of background
                }
            }
            else {                                                       // default use
                ucPixelArray[usY_Start/8][usX_Start] |= ucPixel;         // set the pixel
            }
            if (ucPixelArray[usY_Start/8][usX_Start] != ucOriginalMemoryContent) { // check whether it was indeed modified
                ucByteUpdateArray[usY_Start/64][usX_Start] |= (0x01 << (usY_Start/8));// mark that this pixel byte needs to be updated
            }
            iYpixels -= 0x100000;
            if (iYpixels > 0) {
                break;
            }
            iYpixels += iY_steps;
            usX_Start += iHorizontalDirection;
            iXpixels -= 0x100000;
        } while (iXpixels > 0);
        if (iXpixels <= 0) {
            iXpixels += iX_steps;
        }
    } while (++usY_Start <= usY_End);                                    // follow the line from top to bottom
#else
    iY_steps = (usY_End - usY_Start);                                    // height
    if (usX_Start <= usX_End) {
        iHorizontalDirection = 1;                                        // draw from left to right        
        if (usX_End == usX_Start) {
            iY_steps = 0x7fffffff;                                       // vertical line
        }
        else {
            if ((usX_End - usX_Start) <= iY_steps) {
                iY_steps *= 0x100000;
                iY_steps /= (usX_End - usX_Start);
            }
            else {
                if (usY_End == usY_Start) {
                    iX_steps = 0x7fffffff;                               // horizontal line
                }
                else {
                    iX_steps = (usX_End - usX_Start);                    // width
                    iX_steps *= 0x100000;
                    iX_steps /= iY_steps;
                }
                iY_steps = 0x100000;
            }
        }
    }
    else {
        iHorizontalDirection = -1;                                       // draw from right to left
        if ((usX_Start - usX_End) <= iY_steps) {
            if (usX_Start == usX_End) {                                  // vertical line
                iY_steps = 0x7fffffff;
            }
            else {
                iY_steps *= 0x100000;
                iY_steps /= (usX_Start - usX_End);
            }
            iX_steps = 0x100000;
        }
        else {
            if (iY_steps == 0) {                                         // horizontal line
                iX_steps = 0x7fffffff;
            }
            else {
                iX_steps = (usX_Start - usX_End);                        // width
                iX_steps *= 0x100000;
                iX_steps /= iY_steps;
            }
            iY_steps = 0x100000;
        }
    }
    ucPixel = (0x80 >> (usX_Start%8));                                   // starting position of the pixel to be drawn
    iYpixels = iY_steps;
    iXpixels = iX_steps;                                                 // the number of pixels to draw in the horizontal direction (left or right)
    do {
        if (usY_Start >= (GLCD_Y/ CGLCD_PIXEL_SIZE)) {
            break;
        }
        do {
            if (usX_Start >= (GLCD_X/ CGLCD_PIXEL_SIZE)) {
                break;
            }
            if (iHorizontalDirection > 0) {
                if (usX_Start > usX_End) {
                    break;
                }
            }
            else {
                if (usX_Start < usX_End) {
                    break;
                }
            }
            ucOriginalMemoryContent = ucPixelArray[usY_Start][usX_Start/8];// get the original content
            if ((line->ucMode & (PAINT_INVERTED | PAINT_DARK)) != 0) {
                if ((line->ucMode & PAINT_DARK) != 0) {
                    ucPixelArray[usY_Start][usX_Start/8] &= ~ucPixel;    // clear the pixel
                }
                else {
                    ucPixelArray[usY_Start][usX_Start/8] = (ucOriginalMemoryContent ^ ucPixel); // set the pixel to the inverted value of background
                }
            }
            else {                                                       // default use
                ucPixelArray[usY_Start][usX_Start/8] |= ucPixel;         // set the pixel
            }
            if (ucPixelArray[usY_Start][usX_Start/8] != ucOriginalMemoryContent) { // check whether it was indeed modified
                ucByteUpdateArray[usY_Start][usX_Start/64] |= (0x80 >> ((usX_Start%64)/8));// mark that this pixel byte needs to be updated
            }
            iYpixels -= 0x100000;
            if (iYpixels > 0) {
                break;
            }
            iYpixels += iY_steps;
            if (iHorizontalDirection > 0) {
                ucPixel >>= 1;
                if (ucPixel == 0) {
                    ucPixel = 0x80;
                }
            }
            else {
                ucPixel <<= 1;
                if (ucPixel == 0) {
                    ucPixel = 0x01;
                }
            }
            usX_Start += iHorizontalDirection;
            iXpixels -= 0x100000;
        } while (iXpixels > 0);
        if (iXpixels <= 0) {
            iXpixels += iX_steps;
        }
    } while (++usY_Start <= usY_End);                                    // follow the line from top to bottom
#endif
    if ((line->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD if commanded to do so
    }
    return STATE_LCD_READY;                                              // operation complete
}

#if defined TFT_GLCD_MODE

//#include <stdio.h>
//#include <math.h>

// Calculate the quare root of a 32 bit floating point number using the Babylonian (or Heron's) method
// - series Xn+1 = 1/2 x (Xn + S/Xn) where S is the value whose square root is sought
// - sqrt(S) is thus lim Xn when n -> infinity
//
static float uSqrtf32(float x)
{
    #define SQRTF32_ACCURACY   0.001                                     // accuracy of the result
    float fQuess;
    float fDelta = 1;
    if (x == 0) {
        return (float)0.0;
    }
    if (x > (float)1.0) {
        fQuess = (x / 2);
    }
    else {
        fQuess = (x * 2);
    }
    while (fDelta > (float)(SQRTF32_ACCURACY)) {
        fDelta = fQuess;
        fQuess = ((fQuess + (x/fQuess)) / 2);
        if (fDelta >= fQuess) {
            fDelta = (fDelta - fQuess);
        }
        else {
            fDelta = (fQuess- fDelta);
        }
    }
    return fQuess;
}


// This routine creates an array corresponding to the number of pixels from the edge of a square in which a quarter of the circle occupies
// - the length is equal to cos(arcsin x), which can also be represented by the square root of (1 - x^2)
//
static void fnCreateCircleArray(unsigned char *ptrCircleArray, int iY_size)
{
    int i = 0;
    float value;
    while (i < iY_size) {
        value = (float)i;
        value /= (iY_size);                                              // scale input to 0..1
#if 1
        value *= value;                                                  // square input
        value = (1 - value);
        value = uSqrtf32(value);
#else
        value = asin(value);
        value = cos(value);
#endif
        value *= iY_size;                                                // scale the output
        *ptrCircleArray++ = (unsigned char)(value);                      // assume 256 pixels are adequate
        i++;
    }
}
#endif

#if defined SUPPORT_TOUCH_SCREEN
static COLORREF *ptBackup[TOUCH_SCREEN_PENS] = {0};

static RECT_COORDINATES fadeCoord[TOUCH_SCREEN_PENS] = {0};
static unsigned char ucFadeCnt[TOUCH_SCREEN_PENS] = {0};

static void fnRegisterFadeObject(RECT_COORDINATES *ptrSquareCoord, unsigned char ucRef)
{
    uMemcpy(&fadeCoord[ucRef], ptrSquareCoord, sizeof(RECT_COORDINATES));
    ucFadeCnt[ucRef] = 3;                                                // fade is performed in maximum 3 phases
    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), T_FADE);       // start delay to fade out the object
}

static void fnReturnBackup(COLORREF *ptDst, COLORREF *ptReturn, int iY_size)
{
    int iScan;
    int iX_size = iY_size;
    while (iY_size-- != 0) {                                             // each column
        iScan = iX_size;                                                 // line width
        while (iScan-- != 0) {                                           // each line
            *ptDst = *ptReturn;                                          // return the content from the backup
            ptDst++;
            ptReturn++;
        }
        ptDst -= iX_size;
        ptDst += GLCD_X;
    }
    fnRedrawSimDisplay();
}

static void fnFadeObject(void)
{
    int iRef = 0;
    while (iRef < TOUCH_SCREEN_PENS) {
        if (ucFadeCnt[iRef] != 0) {
    #if defined _iMX && defined eLCD_IN_OCR2
            COLORREF *ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
    #else
            COLORREF *ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
    #endif
            unsigned short usRandomPoint = 0;
            unsigned short usRandom;
            int iY_size = (fadeCoord[iRef].usY_end - fadeCoord[iRef].usY_start);
            int iX_size = iY_size;
            int iScan;
            COLORREF *ptReturn = ptBackup[iRef];
            ptDst += fadeCoord[iRef].usX_start;
            ptDst += (fadeCoord[iRef].usY_start * GLCD_X);
            if (--ucFadeCnt[iRef] == 0) {
                fnReturnBackup(ptDst, ptReturn, iY_size);
            }
            else {
                while (iY_size-- != 0) {                                         // each column
                    iScan = iX_size;                                             // line width
                    while (iScan-- != 0) {                                       // each line
                        if (usRandomPoint == 0) {
                            usRandom = fnRandom();
                            usRandomPoint = 0x8000;
                        }
                        if ((usRandom & usRandomPoint) == 0) {
                            *ptDst = *ptReturn;                                  // return the content from the backup
                        }
                        usRandomPoint >>= 1;
                        ptDst++;
                        ptReturn++;                                              // return the content from the backup
                    }
                    ptDst -= iX_size;
                    ptDst += GLCD_X;
                }
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), T_FADE);    // start delay to fade out the object
                fnRedrawSimDisplay();
            }
        }
        iRef++;
    }
}

static void fnWriteTouchPoint(GLCD_TOUCH_POINT *ptrTouchPoint)
{
    if ((ptrTouchPoint->ucMode & ~(REDRAW | EXTENDED_MODE | WINDOWED_PIC)) == PAINT_RAW) {
        static unsigned char *ptrCircleArray = 0;                        // one circle array shared by each touch point
        int iX_size = (ptrTouchPoint->usRadius * 2);
        int iY_size = iX_size;
        int iScan;
        unsigned char ucRef = ptrTouchPoint->ucRef;
        unsigned long ulRed;
        unsigned long ulGreen;
        unsigned long ulBlue;
        COLORREF *ptReturn;
    #if defined _iMX && defined eLCD_IN_OCR2
        COLORREF *ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
    #else
        COLORREF *ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
    #endif
        RECT_COORDINATES squareCoord;
        squareCoord.usX_start = (ptrTouchPoint->usCenterX - ptrTouchPoint->usRadius);
        squareCoord.usX_end = (ptrTouchPoint->usCenterX + ptrTouchPoint->usRadius);
        squareCoord.usY_start = (ptrTouchPoint->usCenterY - ptrTouchPoint->usRadius);
        squareCoord.usY_end = (ptrTouchPoint->usCenterY + ptrTouchPoint->usRadius);
        if (ptrCircleArray == 0) {
            ptrCircleArray = uMalloc(ptrTouchPoint->usRadius);           // allocate memory to hold the circle array
            fnCreateCircleArray(ptrCircleArray, ptrTouchPoint->usRadius);// fill the circle array
            {
                int i;
                fnDebugMsg("Circle Array =");
                for (i = 0; i < ptrTouchPoint->usRadius; i++) {
                    fnDebugHex((unsigned char)*(ptrCircleArray + i), (WITH_LEADIN | WITH_SPACE | sizeof(unsigned char)));
                }
                fnDebugMsg("\r\n");
            }
        }
        if (ptBackup[ucRef] == 0) {
            ptBackup[ucRef] = uMalloc(iX_size * iY_size * sizeof(COLORREF)); // size of the backup buffer
        }
        else if (ucFadeCnt[ucRef] != 0) {                                // if this touch point is presently fading return its background
            COLORREF *ptDstRestore = ptDst;
            ucFadeCnt[ucRef] = 0;
            ptDstRestore += fadeCoord[ucRef].usX_start;
            ptDstRestore += (fadeCoord[ucRef].usY_start * GLCD_X);
            fnReturnBackup(ptDstRestore, ptBackup[ucRef], iY_size);
        }
        ptDst += squareCoord.usX_start;
        ptDst += (squareCoord.usY_start * GLCD_X);
        ptReturn = ptBackup[ucRef];
        if ((ptrTouchPoint->ucMode & REDRAW) != 0) {
            if ((ptrTouchPoint->ucMode & WINDOWED_PIC) != 0) {           // fade out
                fnRegisterFadeObject(&squareCoord, ucRef);
            }
            else {
                while (iY_size-- != 0) {                                 // each column
                    iScan = iX_size;                                     // line width
                    while (iScan-- != 0) {                               // each line
                        *ptDst++ = *ptReturn++;                          // return the content from the backup
                    }
                    ptDst -= iX_size;
                    ptDst += GLCD_X;
                }
            }
        }
        else {
            while (iY_size != 0) {                                       // for each line
                iScan = iX_size;                                         // line width
                while (iScan-- != 0) {                                   // each line
                    unsigned char ucCircleArc;
                    ulRed = *ptDst;
                    *ptReturn++ = (COLORREF)ulRed;                       // backup the original frame buffer content
                    if (iY_size > (iX_size/2)) {
                        ucCircleArc = ptrCircleArray[iY_size - 1 - (iX_size/2)];
                    }
                    else {
                        ucCircleArc = ptrCircleArray[(iX_size/2) - iY_size];
                    }
                    if (iScan < (iX_size / 2)) {
                        ucCircleArc = ((iX_size / 2) - ucCircleArc);
                        if (iScan > ucCircleArc) {
                            ucCircleArc = 0xff;                          // force inversion
                        }
                        else {
                            ucCircleArc = 0;                             // no inversion
                        }
                    }
                    else {
                        ucCircleArc += (iX_size / 2);
                    }
                    if (iScan < ucCircleArc) {                           // invert inside of circle
                        ulBlue = ulGreen = ulRed;
                        ulRed >>= 11;
                        ulRed = ~ulRed;
                        ulRed &= 0x1f;
                        ulGreen >>= 5;
                        ulGreen = ~ulGreen;
                        ulGreen &= 0x3f;
                        ulBlue = ~ulBlue;
                        ulBlue &= 0x1f;
                        ulRed <<= 11;
                        ulRed |= (ulGreen << 5);
                        ulRed |= (ulBlue);
                        *ptDst = (COLORREF)ulRed;                        // invert the colours
                    }
                    ptDst++;
                }
                ptDst -= iX_size;
                ptDst += GLCD_X;
                iY_size--;
            }
        }
        fnRedrawSimDisplay();
    }
}
#endif

#if defined E_LCD_POLY
static COLORREF *fnGetWorkingRectangle(POINT_COORDINATE *ptrPoints, RECT_COORDINATES *ptrWorkingRect, unsigned char *ptr_ucUpsampling, unsigned char ucPoints)
{
    COLORREF *memory;
    unsigned char ucUpsampling = *ptr_ucUpsampling;
    unsigned short usWidth;
    unsigned short usHeight;
    int i;
    ptrWorkingRect->usX_end = ptrPoints->usX;
    ptrWorkingRect->usX_start = ptrPoints->usX;
    ptrWorkingRect->usY_end = ptrPoints->usY;
    ptrWorkingRect->usY_start = ptrPoints->usY;
    ptrPoints++;
    for (i = 0; i < ucPoints; i++) {
        if (ptrPoints->usX < ptrWorkingRect->usX_start) {
            ptrWorkingRect->usX_start = ptrPoints->usX;
        }
        if (ptrPoints->usX > ptrWorkingRect->usX_end) {
            ptrWorkingRect->usX_end = ptrPoints->usX;
        }
        if (ptrPoints->usY < ptrWorkingRect->usY_start) {
            ptrWorkingRect->usY_start = ptrPoints->usY;
        }
        if (ptrPoints->usY > ptrWorkingRect->usY_end) {
            ptrWorkingRect->usY_end = ptrPoints->usY;
        }
        ptrPoints++;
    }
    usWidth = ((ptrWorkingRect->usX_end - ptrWorkingRect->usX_start) + 1);
    usHeight = ((ptrWorkingRect->usY_end - ptrWorkingRect->usY_start) + 1);
    i = (usWidth * usHeight);                                            // the number of pixels
    i *= sizeof(COLORREF);
    if (ucUpsampling == 0) {                                             // dyamic anti-aliasing
        #define ANTI_ALIASING_BUFFER_MEMORY   (UCALLOC_HEAP_SIZE/2)
        #define MAX_ANTI_ALIASING_UPSAMPLING   7
        unsigned char ucMaxUpSampling = MAX_ANTI_ALIASING_UPSAMPLING;
        int j;
        FOREVER_LOOP() {
            j = i * (ucMaxUpSampling * ucMaxUpSampling);
            if (j < ANTI_ALIASING_BUFFER_MEMORY) {
                ucUpsampling = ucMaxUpSampling;
                break;
            }
            ucMaxUpSampling--;
        }
        *ptr_ucUpsampling = ucUpsampling;
    }
    i *= (ucUpsampling * ucUpsampling);
    ptrWorkingRect->usX_end += (usWidth * (ucUpsampling - 1));
    ptrWorkingRect->usY_end += (usHeight * (ucUpsampling - 1));
    memory = (COLORREF *)_uCalloc(i);
    uMemset(memory, 0xff, i);
    return memory;                                                       // return memory of suitable size
}


static void fnDrawPixelLine(COLORREF *ptrWorkingMemory, RECT_COORDINATES *ptrDiagonal, unsigned short usWidthCanvas, COLORREF Color)
{
    int iType = 0;
    int iWidthCanvasSense;
    unsigned short usX;
    unsigned short usY;
    unsigned short usX_start;
    unsigned short usY_start;
    unsigned short usWidth;
    unsigned short usHeight;
    float fXstep;
    float fYstep;
    float fRef;
    COLORREF *ptrStartMemory = ptrWorkingMemory;
    COLORREF *ptrDotMemory;
    POINT_COORDINATE destination;

    if (ptrDiagonal->usX_end > ptrDiagonal->usX_start) {
        destination.usX = (ptrDiagonal->usX_end - ptrDiagonal->usX_start);
        usX_start = ptrDiagonal->usX_start;
        iType = 1;
    }
    else if (ptrDiagonal->usX_end == ptrDiagonal->usX_start) {
        destination.usX = 0;
        usX_start = ptrDiagonal->usX_start;
    }
    else {
        destination.usX = (ptrDiagonal->usX_start - ptrDiagonal->usX_end);
        usX_start = ptrDiagonal->usX_end;
        iType = 2;
    }
    usWidth = (destination.usX + 1);

    if (ptrDiagonal->usY_end > ptrDiagonal->usY_start) {
        destination.usY = (ptrDiagonal->usY_end - ptrDiagonal->usY_start);
        usY_start = ptrDiagonal->usY_start;
        iType |= 4;
    }
    else if (ptrDiagonal->usY_end == ptrDiagonal->usY_start) {
        destination.usY = 0;
        usY_start = ptrDiagonal->usY_start;
    }
    else {
        destination.usY = (ptrDiagonal->usY_start - ptrDiagonal->usY_end);
        usY_start = ptrDiagonal->usY_end;
        iType |= 8;
    }
    usHeight = (destination.usY + 1);

    fYstep = (float)usHeight;
    fYstep /= usWidth;
    fXstep = (float)usWidth;
    fXstep /= usHeight;

    ptrStartMemory += usX_start;                                         // set to x start offset
    if ((iType == 9) || (iType == 6)) {
        ptrStartMemory += ((usHeight + usY_start - 1) * usWidthCanvas);  // set to y start offset
        iWidthCanvasSense = -usWidthCanvas;
    }
    else {
        ptrStartMemory += (usY_start * usWidthCanvas);                   // set to y start offset
        iWidthCanvasSense = usWidthCanvas;
    }

    fRef = (float)0.0;
    usX = 0;
    while (usX <= destination.usX) {
        usY = (unsigned short)fRef;
        ptrDotMemory = ptrStartMemory;
        ptrDotMemory += usX;
        ptrDotMemory += (usY * iWidthCanvasSense);
        *ptrDotMemory = Color;                                           // draw dot
        fRef += fYstep;
        usX++;
    }

    fRef = (float)0.0;
    usY = 0;
    while (usY <= destination.usY) {
        usX = (unsigned short)fRef;
        ptrDotMemory = ptrStartMemory;
        ptrDotMemory += usX;
        ptrDotMemory += (usY * iWidthCanvasSense);
        *ptrDotMemory = Color;                                           // draw dot
        fRef += fXstep;
        usY++;
    }
}

static void fnFillPolyBorder(COLORREF *ptrWorkingMemory, RECT_COORDINATES *ptrWorkingRect, COLORREF Color)
{
    unsigned short usX = ptrWorkingRect->usX_start;
    unsigned short usY = ptrWorkingRect->usY_start;
    COLORREF *ptrBorderStart = 0;
    COLORREF *ptrBorderEnd;
    int iState;
    while (usY <= ptrWorkingRect->usY_end) {                             // for each line
        iState = 0;
        ptrBorderEnd = 0;
        usX = ptrWorkingRect->usX_start;
        while (usX <= ptrWorkingRect->usX_end) {                         // for pixel in the line
            switch (iState) {
            case 0:
                if (*ptrWorkingMemory++ != RGB(255, 255, 255)) {         // border found
                    iState = 1;                                          // search for after the first border
                }
                break;
            case 1:
                if (*ptrWorkingMemory == RGB(255, 255, 255)) {           // the other side of the first border has been found
                    ptrBorderStart = ptrWorkingMemory;                   // the pixel after the first border has been found
                    iState = 2;                                          // search for the second boarder
                }
                ptrWorkingMemory++;
                break;
            case 2:
                if (*ptrWorkingMemory != RGB(255, 255, 255)) {           // second border has been found
                    ptrBorderEnd = ptrWorkingMemory;                     // the pixel where the second border has been found
                    while (ptrBorderStart <= ptrBorderEnd) {             // fill between the first border and the second border
                        *ptrBorderStart++ = Color;
                    }
                    iState = 1;                                          // repeat, in case there are multiple borders found
                  //ptrWorkingMemory += (ptrWorkingRect->usX_end - usX); // move to the end of the line
                  //usX = ptrWorkingRect->usX_end;
                  //break;
                }
                ptrWorkingMemory++;
                break;
            }
            usX++;
        }
      //if (ptrBorderEnd != 0) {
      //    while (ptrBorderStart <= ptrBorderEnd) {
      //        *ptrBorderStart ++ = Color;
          //}
          //uMemset(ptrBorderStart, 0x00, ((unsigned char *)ptrBorderEnd - (unsigned char *)ptrBorderStart)); // fill the poly
      //}
        usY++;
    }
}

static int fnDrawPolygon(GLCD_POLY *poly, unsigned char ucPoints)
{
    unsigned char ucUpsample = poly->ucAntiAliasing;
    int iClosePoly;
    POINT_COORDINATE *ptrPoint = &poly->points[0];
    RECT_COORDINATES diagonal;
    RECT_COORDINATES WorkingRect;
    unsigned short usWidthCanvas;
    COLORREF *ptrWorkingMemory;
    ucPoints /= (sizeof(poly->points[0]));
    if (ucPoints > 1) {
        iClosePoly = 1;
    }
    else {
        iClosePoly = 0;
    }
    ptrWorkingMemory = fnGetWorkingRectangle(ptrPoint, &WorkingRect, &ucUpsample, ucPoints); // get a working rectangle and memory to work in
    usWidthCanvas = (WorkingRect.usX_end - WorkingRect.usX_start);
    usWidthCanvas++;
    while (ucPoints-- != 0) {
        diagonal.usX_start = ((ptrPoint->usX - WorkingRect.usX_start) * ucUpsample);
        diagonal.usY_start = ((ptrPoint->usY - WorkingRect.usY_start) * ucUpsample);
        ptrPoint++;
        diagonal.usX_end = ((ptrPoint->usX - WorkingRect.usX_start) * ucUpsample);
        diagonal.usY_end = ((ptrPoint->usY - WorkingRect.usY_start) * ucUpsample);
        fnDrawPixelLine(ptrWorkingMemory, &diagonal, usWidthCanvas, poly->lineColor); // draw a line in the working memory
    }
    if (iClosePoly != 0) {                                               // if the polygon is to be closed
        diagonal.usX_start = ((ptrPoint->usX - WorkingRect.usX_start) * ucUpsample);
        diagonal.usY_start = ((ptrPoint->usY - WorkingRect.usY_start) * ucUpsample);
        ptrPoint = &poly->points[0];
        diagonal.usX_end = ((ptrPoint->usX - WorkingRect.usX_start) * ucUpsample); // the starting point
        diagonal.usY_end = ((ptrPoint->usY - WorkingRect.usY_start) * ucUpsample);
        fnDrawPixelLine(ptrWorkingMemory, &diagonal, usWidthCanvas, poly->lineColor); // draw a line in the working memory

        if (poly->ucMode & 2) {                                          // if the area is to be shaded
            fnFillPolyBorder(ptrWorkingMemory, &WorkingRect, poly->fillColor);
        }
    }

  //fnPaintBitMap(WorkingRect.usX_start, WorkingRect.usY_start, ((WorkingRect.usX_end - WorkingRect.usX_start) + 1), ((WorkingRect.usY_end - WorkingRect.usY_start) + 1), (unsigned char *)ptrWorkingMemory, 0x01, 0); // paint bitmap in raw format
    fnPaintBitMapAntiAlias(ucUpsample, WorkingRect.usX_start, WorkingRect.usY_start, ((WorkingRect.usX_end - WorkingRect.usX_start) + 1), ((WorkingRect.usY_end - WorkingRect.usY_start) + 1), (unsigned char *)ptrWorkingMemory, 0x01, 0); // paint bitmap in raw format

    uCFree(ptrWorkingMemory);
    return STATE_LCD_READY;
}
#endif

#if defined E_LCD_CANVAS

static void fnHandleCanvas(RECT_COORDINATES *ptrRect, unsigned char ucCopyPaste)
{
    COLORREF *ptrWorkingCanvas;
    unsigned short usThisWidth;
    COLORREF *ptDst = fnGetDesinationMemory(0, 0);
    unsigned short usCanvasHeight;
    usCanvasHeight = ((ptrRect->usY_end - ptrRect->usY_start) + 1);
    ptDst += ptrRect->usX_start;
    ptDst += (ptrRect->usY_start * GLCD_X);
    if (ucCopyPaste != 0) {
        int i;
        usCanvasWidth = ((ptrRect->usX_end - ptrRect->usX_start) + 1);
        i = (usCanvasWidth * usCanvasHeight);
        i *= sizeof(COLORREF);
        ptrCanvas = (COLORREF *)_uCalloc(i);
        ptrWorkingCanvas = ptrCanvas;
        canvas_location.usX = ptrRect->usX_start;
        canvas_location.usY = ptrRect->usY_start;
        while (usCanvasHeight-- != 0) {
            usThisWidth = usCanvasWidth;
            while (usThisWidth-- != 0) {
                *ptrWorkingCanvas++ = *ptDst++;
            }
            ptDst -= usCanvasWidth;
            ptDst += GLCD_X;
        }
    }
    else {
        ptrWorkingCanvas = ptrCanvas;
        while (usCanvasHeight-- != 0) {
            usThisWidth = usCanvasWidth;
            while (usThisWidth-- != 0) {
                *ptDst++ = *ptrWorkingCanvas++;
            }
            ptDst -= usCanvasWidth;
            ptDst += GLCD_X;
        }
        uCFree(ptrCanvas);
        ptrCanvas = 0;
        fnRedrawSimDisplay();
    }
}
#endif


#if defined E_LCD_CIRCLE
// Paint a filled circle as commanded by the application
//
static int fnWriteCircle(GLCD_CIRCLE *circle, unsigned char ucPixelArray[Y_BYTES][X_BYTES])
{
#if defined TFT_GLCD_MODE
    if ((circle->ucMode & ~(REDRAW | EXTENDED_MODE | WINDOWED_PIC)) == PAINT_RAW) {
        COLORREF *ptDst;
        unsigned short usX_Start = (circle->usCenterX - circle->usRadius);
        unsigned short usY_Start = (circle->usCenterY - circle->usRadius);
        unsigned short usHeight = (2 * circle->usRadius);
        unsigned short usWidth;
        unsigned short usWidthBackup = usHeight;
        unsigned short usArc;
        int i = 0;
    #if defined _iMX && defined eLCD_IN_OCR2
        ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
    #else
        ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
    #endif
        ptDst += usX_Start;
        ptDst += (usY_Start * GLCD_X);
        while (usHeight != 0) {                                          // for each horizontal line
            float value = (float)(circle->usRadius - i);
            value /= circle->usRadius;                                   // scale input to 0..1
            value *= value;                                              // square input
            value = (1 - value);
            value = uSqrtf32(value);
            value *= circle->usRadius;                                   // scale the output
            usArc = (unsigned short)(value);
            if (usHeight-- > circle->usRadius) {
                i++;
            }
            else {
                i--;
            }
            usWidth = usWidthBackup;
            while (usWidth-- != 0) {                                     // paint the line
                if (usWidth >= circle->usRadius) {                       // left half of circle
                    if ((usWidth - circle->usRadius) <= usArc) {
                        *ptDst = _LCD_PIXEL_COLOUR;                      // draw circle interior
                    }
                }
                else {                                                   // right half of circle
                    if ((circle->usRadius - usWidth) < (usArc + 1)) {
                        *ptDst = _LCD_PIXEL_COLOUR;
                    }
                }
                ptDst++;
            }
            ptDst -= usWidthBackup;
            ptDst += GLCD_X;
        }
        fnRedrawSimDisplay();
        return STATE_LCD_READY;                                          // operation complete
    }
#endif
    _EXCEPTION("To do!!");
    if ((circle->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD if commanded to do so
    }
    return STATE_LCD_READY;                                              // operation complete
}
#endif
#if defined E_LCD_RECT
// Paint a filled rectangle as commanded by the application
//
static int fnWriteRect(GLCD_RECT_BLINK *rect, unsigned short usRadius, unsigned char ucPixelArray[Y_BYTES][X_BYTES]) // {1}
{
    int iHorizontalDirection;
    int i = 0;
    RECT_COORDINATES draw_rect;
#if (!defined _GLCD_SAMSUNG && !defined ST7565S_GLCD_MODE) || defined OLED_GLCD_MODE || defined TFT_GLCD_MODE // {19}
    unsigned char ucPixelStart;
#endif
    unsigned short usRadiusY0;
    unsigned short usRadiusY1;
    unsigned short usRadiusX0;
    unsigned short usRadiusX1;
    unsigned char ucOriginalMemoryContent;
    if (rect->rect_corners.usY_start <= rect->rect_corners.usY_end) {    // always draw from top to bottom
        draw_rect.usY_start = rect->rect_corners.usY_start;
        draw_rect.usY_end   = rect->rect_corners.usY_end;
        draw_rect.usX_start = rect->rect_corners.usX_start;
        draw_rect.usX_end   = rect->rect_corners.usX_end;
    }
    else {
        draw_rect.usY_start = rect->rect_corners.usY_end;
        draw_rect.usY_end   = rect->rect_corners.usY_start;
        draw_rect.usX_start = rect->rect_corners.usX_end;
        draw_rect.usX_end   = rect->rect_corners.usX_start;
    }
    usRadiusY0 = (draw_rect.usY_start + usRadius);
    usRadiusY1 = (draw_rect.usY_end - usRadius);
    usRadiusX0 = (draw_rect.usX_start + usRadius);
    usRadiusX1 = (draw_rect.usX_end - usRadius);
#if defined TFT_GLCD_MODE
    if ((rect->ucMode & ~(REDRAW | EXTENDED_MODE | WINDOWED_PIC)) == PAINT_RAW) {
        unsigned short usDestinationWidth = 0;
        COLORREF *ptDst = fnGetDesinationMemory(&draw_rect, &usDestinationWidth);
        unsigned short usHeight = (draw_rect.usY_end - draw_rect.usY_start);
        unsigned short usWidth;
        unsigned short usWidthBackup = (draw_rect.usX_end - draw_rect.usX_start);
        ptDst += draw_rect.usX_start;
        ptDst += (draw_rect.usY_start * usDestinationWidth);
        while (usHeight-- != 0) {                                        // for each horizontal line
            usWidth = usWidthBackup;
            while (usWidth-- != 0) {                                     // for each pixel in the line
                *ptDst++ = _LCD_PIXEL_COLOUR;
            }
            ptDst -= usWidthBackup;
            ptDst += usDestinationWidth;
        }
        fnRedrawSimDisplay();
        return STATE_LCD_READY;                                          // operation complete
    }
#endif
#if (defined _GLCD_SAMSUNG || defined ST7565S_GLCD_MODE) && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE // {19}
    if (draw_rect.usX_start <= draw_rect.usX_end) {
        iHorizontalDirection = 1;                                        // draw from left to right
    }
    else {
        iHorizontalDirection = -1;                                       // draw from right to left
    }

    do {
        int iNextY = 8;
        unsigned short usX = draw_rect.usX_start;                        // start of new line
        unsigned char ucThisPixel = (0xff << (draw_rect.usY_start%8));
        iNextY -= (draw_rect.usY_start%8);
        if ((draw_rect.usY_end - draw_rect.usY_start + 1) < iNextY) {
            ucThisPixel &= (0xff >> (iNextY - (draw_rect.usY_end - draw_rect.usY_start + 1)));
            iNextY = (draw_rect.usY_end - draw_rect.usY_start + 1);
        }
        do {
            if (((draw_rect.usY_start/8) >= Y_BYTES) || (usX >= X_BYTES)) { // {12} ensure the rectangle is not too large
                break;
            }
            ucOriginalMemoryContent = ucPixelArray[draw_rect.usY_start /8][usX]; // get the original content
            if ((rect->ucMode & (PAINT_INVERTED | PAINT_DARK)) != 0) {
                if ((rect->ucMode & PAINT_DARK) != 0) {
                    ucPixelArray[draw_rect.usY_start /8][usX] &= ~ucThisPixel; // clear the pixel(s)
                }
                else {
                    ucPixelArray[draw_rect.usY_start /8][usX] = (ucOriginalMemoryContent ^ ucThisPixel); // set the pixel(s) to the inverted value of background
                }
            }
            else {                                                       // default use
                ucPixelArray[draw_rect.usY_start /8][usX] |= ucThisPixel; // set the pixel(s)
            }
            if (ucPixelArray[draw_rect.usY_start/8][usX] != ucOriginalMemoryContent) { // check whether it was indeed modified
                ucByteUpdateArray[draw_rect.usY_start/64][usX] |= (0x01 << (draw_rect.usY_start /8)); // mark that this pixel byte needs to be updated
            }
            if (usX == draw_rect.usX_end) {
                break;
            }
            usX += iHorizontalDirection;
        } FOREVER_LOOP();
        draw_rect.usY_start += iNextY;
    }  while (draw_rect.usY_start <= draw_rect.usY_end);
#else                                                                    // Toshiba type controller
    ucPixelStart = (0x80 >> (draw_rect.usX_start%8));                    // starting position of the pixel to be drawn
    if (draw_rect.usX_start <= draw_rect.usX_end) {
        iHorizontalDirection = 1;                                        // draw from left to right
    }
    else {
        iHorizontalDirection = -1;                                       // draw from right to left
    }

    do {                                                                 // for each line
        unsigned short usX = draw_rect.usX_start;                        // start of new line
        unsigned char ucPixel = ucPixelStart;
        unsigned short usArc1 = 0;
        unsigned short usArc2 = 0;
        int j = (draw_rect.usY_start - usRadiusY1);
        if (usRadius > i) {
            float value = (float)(usRadius - i);
            value /= usRadius;                                           // scale input to 0..1
            value *= value;                                              // square input
            value = (1 - value);
            value = uSqrtf32(value);
            value *= usRadius;                                           // scale the output
            usArc1 = (unsigned short)(value);
            i++;
        }
        if ((j >= 0) && (usRadius > j)) {
            float value = (float)(j);
            value /= usRadius;                                           // scale input to 0..1
            value *= value;                                              // square input
            value = (1 - value);
            value = uSqrtf32(value);
            value *= usRadius;                                           // scale the output
            usArc2 = (unsigned short)(value);
            j++;
        }

        FOREVER_LOOP() {                                                 // for each pixel in the line
            unsigned char ucThisPixel = ucPixel;
            int iNextX = iHorizontalDirection;
            if ((draw_rect.usY_start >= Y_BYTES) || (usX/8 >= X_BYTES)) {// {12} ensure the rectangle is not too large
                break;
            }
            ucOriginalMemoryContent = ucPixelArray[draw_rect.usY_start][usX/8]; // get the original content
            if (draw_rect.usY_start < usRadiusY0) {                      // handle rounded corners at top
                if (usX < usRadiusX0) {                                  // top left corner
                    if ((usRadiusX0 - usX) > usArc1) {
                        ucThisPixel &= ~ucPixel;
                    }
                }
                if (usX > usRadiusX1) {                                  // top right corner
                    if ((usX - usRadiusX1) > usArc1) {
                        ucThisPixel &= ~ucPixel;
                    }
                }
            }
            else if (draw_rect.usY_start > usRadiusY1) {                 // handle rounded corners at bottom
                if (usX < usRadiusX0) {                                  // bottom left corner
                    if ((usRadiusX0 - usX) > usArc2) {
                        ucThisPixel &= ~ucPixel;
                    }
                }
                if (usX > usRadiusX1) {                                  // bottom right corner
                    if ((usX - usRadiusX1) > usArc2) {
                        ucThisPixel &= ~ucPixel;
                    }
                }
            }
            if (iHorizontalDirection == 1) {
                if ((usRadius == 0) && ((ucPixel & 0x80) != 0) && ((draw_rect.usX_end - usX) > 8)) { // drawing from left to right and a complete byte can be written
                    ucThisPixel = 0xff;                                  // complete 8 pixels
                    iNextX = 8;
                }
                else {
                    ucPixel >>= 1;                                       // shift the pixel mask
                    if (ucPixel == 0) {                                  // if all 8 bits of a byte have been handled
                        ucPixel = 0x80;                                  // reload to MSB position
                    }
                }
            }
            else {
                if ((usRadius == 0) && (ucPixel & 0x01) && ((usX - draw_rect.usX_end) > 8)) { // drawing from right to left and a complete byte can be written
                    ucThisPixel = 0xff;                                  // complete 8 pixels
                    iNextX = -8;
                }
                else {
                    ucPixel <<= 1;
                    if (ucPixel == 0) {
                        ucPixel = 0x01;
                    }
                }
            }
            if ((rect->ucMode & (PAINT_INVERTED | PAINT_DARK)) != 0) {
                if ((rect->ucMode & PAINT_DARK) != 0) {
                    ucPixelArray[draw_rect.usY_start][usX/8] &= ~ucThisPixel; // clear the pixel(s)
                }
                else {
                    ucPixelArray[draw_rect.usY_start][usX/8] = (ucOriginalMemoryContent ^ ucThisPixel); // set the pixel(s) to the inverted value of background
                }
            }
            else {                                                       // default use
                ucPixelArray[draw_rect.usY_start][usX/8] |= ucThisPixel; // set the pixel(s)
            }
            if (ucPixelArray[draw_rect.usY_start][usX/8] != ucOriginalMemoryContent) { // check whether it was indeed modified
                ucByteUpdateArray[draw_rect.usY_start][usX/64] |= (0x80 >> ((usX%64)/8)); // mark that this pixel byte needs to be updated
            }
            if (usX == draw_rect.usX_end) {
                break;
            }
            usX += iNextX;
        }
    } while (draw_rect.usY_start++ < draw_rect.usY_end);
#endif
    if ((rect->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD if commanded to do so
    }
    return STATE_LCD_READY;                                              // operation complete
}
#endif

// Scroll the display content in x and/or y direction
//
static int fnWriteScroll(GLCD_SCROLL *scroll)
{
#if (defined _GLCD_SAMSUNG || defined ST7565S_GLCD_MODE) && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE // {19}
    int iX = 0;
    int iY;
    int iByteShift = (scroll->sY_scroll/8);
    int iBitShift  = (scroll->sY_scroll%8);
    unsigned char ucOriginalMemoryContent;
    #if defined MAX_BLINKING_OBJECTS
    if ((KILL_BLINK_OBJECTS & scroll->ucMode) != 0) {                    // often it is desired to stop blinking when starting scrolling
        fnConfigureBlink(BLINK_DELETE_ALL, 0);
    }
    #endif
    while (iX < X_BYTES) {
        for (iY = 0; iY < Y_BYTES; iY++) {
            ucOriginalMemoryContent = ucPixelArray[iY][iX];              // get the original content
            if ((iY + iByteShift) >= Y_BYTES) {
                if ((scroll->ucMode & PAINT_DARK) != 0) {
                    ucPixelArray[iY][iX] = 0xff;
                }
                else {
                    ucPixelArray[iY][iX] = 0;
                }
            }
            else {
                ucPixelArray[iY][iX] = ucPixelArray[iY + iByteShift][iX];
            }
            if (iBitShift != 0) {
                ucPixelArray[iY][iX] >>= iBitShift;
                if ((iY + iByteShift + 1) >= Y_BYTES) {
                    if ((scroll->ucMode & PAINT_DARK) != 0) {
                        ucPixelArray[iY][iX] |= (0xff << (8 - iBitShift));
                    }
                }
                else {
                    ucPixelArray[iY][iX] |= (ucPixelArray[iY + iByteShift + 1][iX] << (8 - iBitShift));
                }
            }
            if (ucOriginalMemoryContent != ucPixelArray[iY][iX]) {       // change in the byte
                ucByteUpdateArray[iY/8][iX] |= (0x01 << iY);             // mark that this pixel byte needs to be updated
            }
        }
        iX++;
    }
#else
    int iTop = 0;
    int iStart = (signed short)(scroll->sY_scroll);
    int iBottom = ((GLCD_Y/CGLCD_PIXEL_SIZE) - scroll->sY_scroll);
    int iX;
    unsigned char ucOriginalMemoryContent;
    #if defined MAX_BLINKING_OBJECTS
    if ((KILL_BLINK_OBJECTS & scroll->ucMode) != 0) {                    // often it is desired to stop blinking when starting scrolling
        fnConfigureBlink(BLINK_DELETE_ALL, 0);
    }
    #endif
    while (iTop < iBottom) {
        for (iX = 0; iX < X_BYTES; iX++) {
            ucOriginalMemoryContent = ucPixelArray[iTop][iX];            // get the original content
            ucPixelArray[iTop][iX] = ucPixelArray[iStart][iX];
            if (ucOriginalMemoryContent != ucPixelArray[iTop][iX]) {     // change in the byte
                ucByteUpdateArray[iTop][iX/8] |= (0x80 >> (iX%8));       // mark that this pixel byte needs to be updated
            }
        }
        iStart++;
        iTop++;
    }
    while (iTop < (GLCD_Y/CGLCD_PIXEL_SIZE)) {
        for (iX = 0; iX < X_BYTES; iX++) {
            ucOriginalMemoryContent = ucPixelArray[iTop][iX];            // get the original content
            if ((scroll->ucMode & PAINT_DARK) != 0) {
                ucPixelArray[iTop][iX] = 0xff;
            }
            else {
                ucPixelArray[iTop][iX] = 0;
            }
            if (ucOriginalMemoryContent != ucPixelArray[iTop][iX]) {     // change in the byte
                ucByteUpdateArray[iTop][iX/8] |= (0x80 >> (iX%8));       // mark that this pixel byte needs to be updated
            }
        }
        iTop++;
    }
#endif
    if ((scroll->ucMode & REDRAW) != 0) {
        return (fnSmartUpdate(1));                                       // redraw GLCD if commanded to do so
    }
    return STATE_LCD_READY;
}

#if defined MAX_BLINKING_OBJECTS
    #if !defined GLCD_INVERT_MASK
static int fnFutherBlinkObjects(LCD_OBJECTS blinking_objects[MAX_BLINKING_OBJECTS])
{
    int i = 0;
    while (i < MAX_BLINKING_OBJECTS) {
        if ((blinking_objects[i].ucObjectType != BLINK_NOTHING) && (blinking_objects[i].blink_count_down == 0)) { // this object's blink timer has also fired
            blinking_objects[i].blink_count_down = blinking_objects[i].blink_interval; // set next blink time
            return i;
        }
        i++;
    }
    return -1;                                                           // no further timed out object forund
}
    #endif

static int fnStartBlinkTimer(LCD_OBJECTS blinking_objects[MAX_BLINKING_OBJECTS], int iNewObject)
{
    UTASK_TICK remaining_tick;
    int i = 0;
    int iLowest = -1;
    int iActiveTimer = 0;
    DELAY_LIMIT next_timer = (DELAY_LIMIT)-1;
    while (i < MAX_BLINKING_OBJECTS) {
        if (blinking_objects[i].ucObjectType != BLINK_NOTHING) {         // active timer
            if (blinking_objects[i].blink_count_down < next_timer) {
                next_timer = blinking_objects[i].blink_count_down;       // lowest value found
                if (iNewObject != i) {
                    iActiveTimer = 1;                                    // timers(s) already active
                }
                iLowest = i;
            }
        }
        i++;
    }
    if (iNewObject < 0) {                                                // a timeout has just occurred and so the lowest next delay must always be set
        i = 0;
        while (i < MAX_BLINKING_OBJECTS) {                               // compensate all others
            if (i != iLowest) {
                blinking_objects[i].blink_count_down -= next_timer;
            }
            i++;
        }
    }
    else {
        if (iLowest == iNewObject) {                                     // the new entry has the shortest next delay value
            if (iActiveTimer != 0) {                                     // a timer is presently active
                remaining_tick = uTaskerRemainingTime(OWN_TASK);         // see how much of the present timer is remaining
                i = 0;
                if (remaining_tick <= next_timer) {
                    while (i < MAX_BLINKING_OBJECTS) {
                        if (i != iNewObject) {
                            blinking_objects[i].blink_count_down -= (DELAY_LIMIT)remaining_tick;
                        }
                        i++;
                    }
                }
                else {                                                   // the new delay is shorter than the existing one's remaining time
                    while (i < MAX_BLINKING_OBJECTS) {
                        if (i != iNewObject) {
                            blinking_objects[i].blink_count_down -= next_timer;
                        }
                        i++;
                    }
                }
            }
        }
        else if (iActiveTimer != 0) {
            blinking_objects[iNewObject].blink_count_down -= next_timer; // adjust first delay of new entry
            return iLowest;                                              // don't disturb present timer
        }
    }
    uTaskerMonoTimer(OWN_TASK, next_timer, T_BLINK);                     // start delay to next blink interval
    return iLowest;
}

#if defined GLCD_INVERT_MASK
static void fnInvalidateInversion(void)
{
    #if defined ST7565S_GLCD_MODE
    int x;
    int y;
    unsigned char ucFlag = 0x01;
    for (y = 0; y < (GLCD_Y / 8); y++) {                                 // for each row
        for (x = 0; x < GLCD_X; x++) {                                   // for each column
            if ((ucByteUpdateArrayBlink[y][x] != 0) || (ucByteUpdateArrayInvert[y][x])) { // if any bit in the inversion fields are set
                ucByteUpdateArray[y / 8][x] |= ucFlag;                   // mark that an update of the bits is required
            }
        }
        ucFlag <<= 1;
    }
    #else
    _EXCEPTION("To do!!");
    #endif
}
#endif

static int fnConfigureBlink(unsigned char ucObjectType, void *object)
{
    static LCD_OBJECTS blinking_objects[MAX_BLINKING_OBJECTS] = {{0}};
    static int iLastObject = 0;
    RECT_COORDINATES *object_coordinates = 0;
    DELAY_LIMIT blink_half_period;
    int i = 0;
    switch (ucObjectType) {
    case BLINK_RECT:
        {
            GLCD_RECT_BLINK *rect = (GLCD_RECT_BLINK *)object;
            object_coordinates = &(rect->rect_corners);
            blink_half_period = rect->blink_half_period;
#if defined GLCD_INVERT_MASK
            if ((rect->ucMode == ADD_INVERSION_MASK) != 0) {
                rect->ucMode = PAINT_LIGHT;
                fnWriteRect(rect, 0, ucByteUpdateArrayBlink);
                if (blinking_objects[0].blink_interval == 0) {
                    blinking_objects[0].blink_interval = blink_half_period;
                    uTaskerMonoTimer(OWN_TASK, blink_half_period, T_BLINK);
                }
                return -1;
            }
            else if (rect->ucMode == REMOVE_INVERSION_MASK) {
                rect->ucMode = PAINT_DARK;
                fnWriteRect(rect, 0, ucByteUpdateArrayBlink);
                return -1;
            }
            else if (rect->ucMode == ADD_CURSOR_MASK) {
                rect->ucMode = PAINT_LIGHT;
                fnWriteRect(rect, 0, ucByteUpdateArrayInvert);
                if (blinking_objects[0].blink_interval == 0) {
                    blinking_objects[0].blink_interval = blink_half_period;
                    uTaskerMonoTimer(OWN_TASK, blink_half_period, T_BLINK);
                }
                return -1;
            }
            else if (rect->ucMode == REMOVE_CURSOR_MASK) {
                rect->ucMode = PAINT_DARK;
                fnWriteRect(rect, 0, ucByteUpdateArrayInvert);
                return -1;
            }            
#endif
        }
        break;
    case BLINK_LINE:
        {
            GLCD_LINE_BLINK *line = (GLCD_LINE_BLINK *)object;
            object_coordinates = &(line->line_start_end);
            blink_half_period = line->blink_half_period;
        }
        break;
    case BLINK_NEXT:                                                     // blink timer has fired
#if defined GLCD_INVERT_MASK
        if (iBlinkCycle >= 3) {
            iBlinkCycle = 0;
        }
        else {
            ++iBlinkCycle;
        }
        fnInvalidateInversion();                                         // set inversion areas as invalid so that they will be redrawn
        uTaskerMonoTimer(OWN_TASK, blinking_objects[0].blink_interval, T_BLINK);
        iLCD_State = fnSmartUpdate(1);
#else
        blinking_objects[iLastObject].blink_count_down = blinking_objects[iLastObject].blink_interval;
        do {
            switch (blinking_objects[iLastObject].ucObjectType) {
            case BLINK_RECT:
                {
                    GLCD_RECT rect;
                    rect.ucMode = (PAINT_INVERTED | REDRAW);
                    uMemcpy(&rect.rect_corners, &blinking_objects[iLastObject].blink_coordinates, sizeof(RECT_COORDINATES));
                    fnDoLCD_rect((GLCD_RECT_BLINK *)&rect);
                }
                break;
            case BLINK_LINE:
                {
                    GLCD_LINE line;
                    line.ucMode = (PAINT_INVERTED | REDRAW);
                    uMemcpy(&line.line_start_end, &blinking_objects[iLastObject].blink_coordinates, sizeof(RECT_COORDINATES));
                    fnDoLCD_line((GLCD_LINE *)&line);
                }
                break;
            }
        } while ((iLastObject = fnFutherBlinkObjects(blinking_objects)) >= 0);
        iLastObject = fnStartBlinkTimer(blinking_objects, iLastObject);  // start next delay
#endif
        return 0;
    case BLINK_DELETE_ALL:
        uMemset(&blinking_objects, 0, sizeof(blinking_objects));
        uTaskerStopTimer(OWN_TASK);                                      // no more blink timers needed
        return 0;
    default:
        return 1;
    }
    while (i < MAX_BLINKING_OBJECTS) {
        if ((blink_half_period != 0) && (blinking_objects[i].ucObjectType == BLINK_NOTHING)) { // empty entry found
            blinking_objects[i].ucObjectType = ucObjectType;
            blinking_objects[i].blink_interval = blinking_objects[i].blink_count_down = blink_half_period; // blink frequency
            uMemcpy(&blinking_objects[i].blink_coordinates, object_coordinates, sizeof(RECT_COORDINATES));
            iLastObject = fnStartBlinkTimer(blinking_objects, i);
            return 0;
        }
        else if ((blinking_objects[i].ucObjectType == ucObjectType) && (uMemcmp(&blinking_objects[i].blink_coordinates, object_coordinates, sizeof(RECT_COORDINATES)) == 0)) {
            if (blink_half_period != 0) {                                // change blink rate on next timeout
                blinking_objects[i].blink_interval = blink_half_period;
                return 0;
            }
            blinking_objects[i].ucObjectType = BLINK_NOTHING;            // delete entry
            if (iLastObject == i) {                                      // timer presently being managed by this object
                i = 0;
                while (i < MAX_BLINKING_OBJECTS) {                       // check whether it can be killed
                    if (blinking_objects[i].ucObjectType != BLINK_NOTHING) { // adapted timer required
                        UTASK_TICK elapsed_tick = (blinking_objects[iLastObject].blink_count_down - uTaskerRemainingTime(OWN_TASK));
                        while (i < MAX_BLINKING_OBJECTS) {
                            blinking_objects[i].blink_count_down += (DELAY_LIMIT)elapsed_tick;
                            i++;
                        }
                        iLastObject = fnStartBlinkTimer(blinking_objects, -1);
                        return 0;
                    }
                    i++;
                }
                uTaskerStopTimer(OWN_TASK);                              // no more blink timers needed
            }
            return 0;
        }
        i++;
    }
    return 1;
}
#endif

#if defined _GLCD_SAMSUNG && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE
// Position XY Address Pointer within the (64 x X) x 64 Display Area
//
static void fnGLCDGotoXY(unsigned char ucXPos, unsigned char ucYPos) 
{
    unsigned char ucChip = GLCD_CHIP0;                                   // 'left' Chip  

    #if SAMSUNG_CHIPS > 1
    if (ucXPos >= SINGLE_CHIP_WIDTH) {                                   // select GLCD_CHIP1, the 'right' Chip
        ucXPos -= SINGLE_CHIP_WIDTH;
        ucChip = GLCD_CHIP1;
    }
    #endif

    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_ADD | ucXPos), ucChip);     // set X Coordinate
    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_PAGE | ucYPos/8), GLCD_CHIP0); // set Y Coordinate
    #if SAMSUNG_CHIPS > 1
    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_PAGE | ucYPos/8), GLCD_CHIP1);
    #endif
}
#elif defined ST7565S_GLCD_MODE                                          // {19}
static void fnGLCDGotoXY(unsigned char ucXPos, unsigned char ucYPos)
{
    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_ADDH | ucXPos/16));  		 // set X Coordinate - high nibble
    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_ADDL | ucXPos%16));   		 // set X Coordinate - low nibble
    while (GLCD_BUSY()) {}                                               // wait until ready
    fnWriteGLCD_cmd((unsigned char)(GLCD_SET_PAGE | ucYPos/8));			 // set Y Coordinate
}

#endif

// This routine is called when the content of the backup buffer is to be written to the GLCD.
// It only updates changed bytes and only sets the address pointer when the auto-inc value doesn't automatically match
//
static int fnSmartUpdate(int iStart)
{
#if defined OLED_GLCD_MODE || defined NOKIA_GLCD_MODE
    #if defined NOKIA_GLCD_MODE
    #define WINDOW_WIDTH DISPLAY_RIGHT_PIXEL
    static unsigned char pucWindowX[] = {CASET, 0, DISPLAY_RIGHT_PIXEL};
    static unsigned char pucWindowY[] = {PASET, 0, DISPLAY_BOTTOM_PIXEL};
    #else
    #define WINDOW_WIDTH ((GLCD_X - 2)/2)
    static unsigned char pucWindowX[] = {0x15, 0, WINDOW_WIDTH};
    static unsigned char pucWindowY[] = {0x75, 0, (GLCD_Y - 1)};
    #endif
    static unsigned char ucLastX_WindowStart = 0;
    static unsigned char x_stall = 0;
    static unsigned char y_stall = 0;
    static unsigned char ucLastX = 0xff;
    static unsigned char ucLastY = 0xff;
    int iMaxText = MAX_GLCD_WRITE_BURST;                                 // allow this many writes before pausing
    unsigned char x;
    unsigned char y;
    unsigned char g_pucBuffer[12];
    if (iStart != 0) {
        x_stall = DISPLAY_LEFT_PIXEL;
        y_stall = DISPLAY_TOP_PIXEL;
    }
    for (y = y_stall; y <= DISPLAY_BOTTOM_PIXEL; y++) {                  // for each row
        for (x = x_stall; x < (UPDATE_WIDTH); x++) {                     // for each column
            if (ucByteUpdateArray[y][x] != 0) {                          // check if a pixel has changed in this byte
                int iAdd = 0;
                unsigned char ucBit = 0x80;
    #if defined NOKIA_GLCD_MODE                                          // display pixel width is not divisable by 8 so handle last byte differently
                if (x == (UPDATE_WIDTH - 1)) {
                    ucByteUpdateArray[y][x] &= (0xff << (8 - (X_BYTES%8)));
                }
    #endif
                while (ucByteUpdateArray[y][x] != 0) {                   // for each changed pixel in the changed byte
                    if (ucByteUpdateArray[y][x] & ucBit) {               // this byte pixel has changed
                        unsigned char ucNewPixelByte = ucPixelArray[y][(x * 8 + iAdd)];
    #if defined NOKIA_GLCD_MODE
                        pucWindowY[1] = (y + 2);                         // avoid invisible lines
                        pucWindowX[1] = ((x * 64) + iAdd*8);
    #else                                                                // OLED
                        pucWindowY[1] = y;
                        pucWindowX[1] = ((x * 32) + (iAdd * 4));
    #endif
                        if ((ucLastX != pucWindowX[1]) || (ucLastY != pucWindowY[1])) { // if internal address pointer needs to be changed
                            ucLastX_WindowStart = pucWindowX[1];
                            fnSendSPI_Command(pucWindowX, sizeof(pucWindowX));
                            fnSendSPI_Command(pucWindowY, sizeof(pucWindowY));
    #if defined NOKIA_GLCD_MODE
                            WriteSpiCommand(RAMWR);                      // data follows
    #endif
                        }
    #if defined NOKIA_GLCD_MODE
                        if (ucNewPixelByte & 0x80) {                     // set pixel colour for each pixel in the on state
                            g_pucBuffer[0] = (unsigned char)(_LCD_PIXEL_COLOUR >> 4);
                            g_pucBuffer[1] = (unsigned char)(_LCD_PIXEL_COLOUR << 4);
                        }
                        else {
                            g_pucBuffer[0] = (unsigned char)(_LCD_ON_COLOUR >> 4);
                            g_pucBuffer[1] = (unsigned char)(_LCD_ON_COLOUR << 4);
                        }
                        if (ucNewPixelByte & 0x40) {
                            g_pucBuffer[1] |= (unsigned char)(_LCD_PIXEL_COLOUR >> 8);
                            g_pucBuffer[2] = (unsigned char)(_LCD_PIXEL_COLOUR);
                        }
                        else {
                            g_pucBuffer[1] |= (unsigned char)(_LCD_ON_COLOUR >> 8);
                            g_pucBuffer[2] = (unsigned char)(_LCD_ON_COLOUR);
                        }
                        if (pucWindowX[1] == 0x80) {
                            fnSend_SPI_data(g_pucBuffer, 3);             // write 2 pixels to display (involving 3 bytes to be sent)
                            pucWindowX[1] = (WINDOW_WIDTH - 8);
                        }
                        else {
                            if (ucNewPixelByte & 0x20) {
                                g_pucBuffer[3] = (unsigned char)(_LCD_PIXEL_COLOUR >> 4);
                                g_pucBuffer[4] = (unsigned char)(_LCD_PIXEL_COLOUR << 4);
                            }
                            else {
                                g_pucBuffer[3] = (unsigned char)(_LCD_ON_COLOUR >> 4);
                                g_pucBuffer[4] = (unsigned char)(_LCD_ON_COLOUR << 4);
                            }
                            if (ucNewPixelByte & 0x10) {
                                g_pucBuffer[4] |= (unsigned char)(_LCD_PIXEL_COLOUR >> 8);
                                g_pucBuffer[5] = (unsigned char)(_LCD_PIXEL_COLOUR);
                            }
                            else {
                                g_pucBuffer[4] |= (unsigned char)(_LCD_ON_COLOUR >> 8);
                                g_pucBuffer[5] = (unsigned char)(_LCD_ON_COLOUR);
                            }
                            if (ucNewPixelByte & 0x08) {
                                g_pucBuffer[6] = (unsigned char)(_LCD_PIXEL_COLOUR >> 4);
                                g_pucBuffer[7] = (unsigned char)(_LCD_PIXEL_COLOUR << 4);
                            }
                            else {
                                g_pucBuffer[6] = (unsigned char)(_LCD_ON_COLOUR >> 4);
                                g_pucBuffer[7] = (unsigned char)(_LCD_ON_COLOUR << 4);
                            }
                            if (ucNewPixelByte & 0x04) {
                                g_pucBuffer[7] |= (unsigned char)(_LCD_PIXEL_COLOUR >> 8);
                                g_pucBuffer[8] = (unsigned char)(_LCD_PIXEL_COLOUR);
                            }
                            else {
                                g_pucBuffer[7] |= (unsigned char)(_LCD_ON_COLOUR >> 8);
                                g_pucBuffer[8] = (unsigned char)(_LCD_ON_COLOUR);
                            }
                            if (ucNewPixelByte & 0x02) {
                                g_pucBuffer[9] = (unsigned char)(_LCD_PIXEL_COLOUR >> 4);
                                g_pucBuffer[10] = (unsigned char)(_LCD_PIXEL_COLOUR << 4);
                            }
                            else {
                                g_pucBuffer[9] = (unsigned char)(_LCD_ON_COLOUR >> 4);
                                g_pucBuffer[10] = (unsigned char)(_LCD_ON_COLOUR << 4);
                            }
                            if ((ucNewPixelByte & 0x01) != 0) {
                                g_pucBuffer[10] |= (unsigned char)(_LCD_PIXEL_COLOUR >> 8);
                                g_pucBuffer[11] = (unsigned char)(_LCD_PIXEL_COLOUR);
                            }
                            else {
                                g_pucBuffer[10] |= (unsigned char)(_LCD_ON_COLOUR >> 8);
                                g_pucBuffer[11] = (unsigned char)(_LCD_ON_COLOUR);
                            }
                            fnSend_SPI_data(g_pucBuffer, 12);            // write 8 pixels to display (involving 12 bytes to be sent)
                        }
    #else
                        uMemset(g_pucBuffer, 0, sizeof(g_pucBuffer));    // clear buffer to backgound colour
                        if ((ucNewPixelByte & 0x80) != 0) {
                            g_pucBuffer[0] = 0xf0;
                        }
                        if ((ucNewPixelByte & 0x40) != 0) {
                            g_pucBuffer[0] |= 0x0f;
                        }
                        if ((ucNewPixelByte & 0x20) != 0) {
                            g_pucBuffer[1] = 0xf0;
                        }
                        if ((ucNewPixelByte & 0x10) != 0) {
                            g_pucBuffer[1] |= 0x0f;
                        }
                        if ((ucNewPixelByte & 0x08) != 0) {
                            g_pucBuffer[2] = 0xf0;
                        }
                        if ((ucNewPixelByte & 0x04) != 0) {
                            g_pucBuffer[2] |= 0x0f;
                        }
                        if ((ucNewPixelByte & 0x02) != 0) {
                            g_pucBuffer[3] = 0xf0;
                        }
                        if ((ucNewPixelByte & 0x01) != 0) {
                            g_pucBuffer[3] |= 0x0f;
                        }
                        fnSend_SPI_data(g_pucBuffer, 4);                 // write 8 pixels to display (involving 4 bytes to be sent)
        #endif
                        ucLastY = pucWindowY[1];
        #if defined NOKIA_GLCD_MODE
                        ucLastX = (pucWindowX[1] + 8);
        #else
                        ucLastX = (pucWindowX[1] + 4);
        #endif
                        if (ucLastX >= WINDOW_WIDTH) {
                            ucLastX = ucLastX_WindowStart;
                            ucLastY++;
                        }
                        ucByteUpdateArray[y][x] &= ~ucBit;
                        iMaxText--;
                    }
                    iAdd++;
                    ucBit >>= 1;
                    if (ucBit == 0) {
                        ucBit = 0x80;
                    }
                }
                if (iMaxText <= 0) {                                     // maximum bust write has been performed
                    x_stall = (x + 1);                                   // save the update context
                    y_stall = y;
                    uTaskerStateChange(OWN_TASK, UTASKER_GO);            // switch to polling mode of operation
                    return STATE_LCD_WRITING;                            // busy state
                }
            }
        }
        x_stall = 0;
    }
#else
#if defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
    static unsigned short usLastX_pixel = 0xffff;                        // force the initial window to be set
    static unsigned short usLastY_pixel = 0xffff;
#endif
    static unsigned short x_stall = 0;
    static unsigned short y_stall = 0;
#if defined _GLCD_SAMSUNG && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined MB785_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE
    static unsigned short usNextAddress[SAMSUNG_CHIPS] = {0};
#elif defined ST7565S_GLCD_MODE                                          // {19}
    static unsigned short usNextAddress = 0;
#elif defined TFT_GLCD_MODE
    COLORREF *ptDst;
#elif !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE
    static unsigned short usLastAddress = 0xffff;
#endif
    int iMaxText = MAX_GLCD_WRITE_BURST;                                 // allow this many writes before yielding
    unsigned short x;
    unsigned short y;
    if (iStart != 0) {
        x_stall = DISPLAY_LEFT_PIXEL;
        y_stall = DISPLAY_TOP_PIXEL;
    }
#if defined _GLCD_SAMSUNG && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE
    for (y = y_stall; y < GLCD_Y;) {                                     // for each row
        unsigned char ucPixelByteBit = (0x01 << (y/8));
        while (ucPixelByteBit != 0) {
            for (x = x_stall; x < X_BYTES; x++) {                        // for each column
                if (ucByteUpdateArray[y/64][x] & ucPixelByteBit) {       // check if a pixel has changed in this byte
                    ucByteUpdateArray[y/64][x] &= ~ucPixelByteBit;
                    if (x < SINGLE_CHIP_WIDTH) {                         // chip 0
                        if (usNextAddress[0] != ((y * 8) + x)) {
                            fnGLCDGotoXY(x, y);                          // ensure coordinate is correct
                        }
                        while (GLCD_BUSY()) {}                    
                        fnWriteGLCD_data(ucPixelArray[y/8][x], GLCD_CHIP0);
                        usNextAddress[0] = ((y * 8) + x + 1);
                        if ((usNextAddress[0] % SINGLE_CHIP_WIDTH) == 0) {
                            usNextAddress[0] -= SINGLE_CHIP_WIDTH;
                        }
                    }
                    else {                                               // chip 1
                        if (usNextAddress[1] != ((y * 8) + x)) {
                            fnGLCDGotoXY(x, y);                          // ensure coordinate is correct
                        }
                        while (GLCD_BUSY()) {}                    
                        fnWriteGLCD_data(ucPixelArray[y/8][x], GLCD_CHIP1);
                        usNextAddress[1] = ((y * 8) + x + 1);
                        if ((usNextAddress[1] % SINGLE_CHIP_WIDTH) == 0) {
                            usNextAddress[1] -= SINGLE_CHIP_WIDTH;
                        }
                    }
                    if (--iMaxText <= 0) {                               // maximum bust write has been performed
                        y_stall = y;
                        x_stall = (x + 1);                               // save the update context
                        if (x_stall >= X_BYTES) {
                            x_stall = 0;
                        }
                        uTaskerStateChange(OWN_TASK, UTASKER_GO);        // switch to polling mode of operation
                        return STATE_LCD_WRITING;                        // busy state
                    }
                }
            }
            y += 8;;
            ucPixelByteBit <<= 1;
            x_stall = 0;
        }
    }
#elif defined ST7565S_GLCD_MODE											 // {19}	
    for (y = y_stall; y < GLCD_Y;) {                                     // for each row
        unsigned char ucPixelByteBit = (0x01 << (y/8));
        while (ucPixelByteBit != 0) {
            for (x = x_stall; x < X_BYTES; x++) {                        // for each column
                if ((ucByteUpdateArray[y / 64][x] & ucPixelByteBit) != 0) { // check if a pixel has changed in this byte
                    ucByteUpdateArray[y / 64][x] &= ~ucPixelByteBit;
                    if (usNextAddress != ((y * 16) + x)) {
                        fnGLCDGotoXY((unsigned char)x, (unsigned char)y);// ensure coordinate is correct
                    }
                    while (GLCD_BUSY()) {}
    #if defined GLCD_INVERT_MASK
                    {
                        unsigned char ucValue = ucPixelArray[y / 8][x];
                        switch (iBlinkCycle) {
                        case 3:
        #if defined GLCD_INVERT_MASK_TOGGLE
                            ucValue ^= ucByteUpdateArrayBlink[y / 8][x];
        #else
                            ucValue &= ~ucByteUpdateArrayBlink[y / 8][x];
        #endif
                            ucValue |= ucByteUpdateArrayInvert[y / 8][x];
                            break;
                        case 2:
        #if defined GLCD_INVERT_MASK_TOGGLE
                            ucValue ^= ucByteUpdateArrayBlink[y / 8][x];
        #else
                            ucValue &= ~ucByteUpdateArrayBlink[y / 8][x];
        #endif
                            ucValue &= ~ucByteUpdateArrayInvert[y / 8][x];
                            break;
                        case 1:
                            ucValue |= ucByteUpdateArrayInvert[y / 8][x];
                            break;
                        default:                                         // 0
                            ucValue &= ~ucByteUpdateArrayInvert[y / 8][x];
                            break;
                        }
                        fnWriteGLCD_data(ucValue);
                    }
    #else
                    fnWriteGLCD_data(ucPixelArray[y/8][x]);
    #endif
                    usNextAddress = ((y * 16) + x + 1);
                    if ((usNextAddress % 128) == 0) {
                        usNextAddress -= 128;
                    }
                    if (--iMaxText  <= 0) {                              // maximum bust write has been performed
                        y_stall = y;
                        x_stall = (x + 1);                               // save the update context
                        if (x_stall >= X_BYTES) {
                            x_stall = 0;
                        }
                        uTaskerStateChange(OWN_TASK, UTASKER_GO);        // switch to polling mode of operation
                        return STATE_LCD_WRITING;                        // busy state
                    }
                }
            }
            x_stall = 0;
            y += 8;
            if (y >= GLCD_Y) {                                           // quit if the end of the display has been reached
                break;
            }
            usNextAddress = 0xffff;                                      // for the address to be sent at the start of teh next line to ensure synchronised with the display's internal pointer
            ucPixelByteBit <<= 1;
        }
    }
#else
    for (y = y_stall; y <= (DISPLAY_BOTTOM_PIXEL/CGLCD_PIXEL_SIZE); y++) { // for each row
        for (x = x_stall; x < (UPDATE_WIDTH); x++) {                     // for each column
            if (ucByteUpdateArray[y][x] != 0) {                          // check if a pixel has changed in this byte
    #if !defined TFT_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined FT800_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE
                int iOffset = ((y * X_BYTES) + (x * 8)); 
    #endif
    #if defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE
                int iRepeatY = 0;
    #endif
                int iAdd = 0;
                unsigned char ucBit = 0x80;
                while (ucByteUpdateArray[y][x] != 0) {                   // for each changed bit in the changed byte
                    if ((ucByteUpdateArray[y][x] & ucBit) != 0) {        // this pixel byte has changed
    #if defined TFT_GLCD_MODE
                        unsigned char ucTFTBit = 0x80;
        #if defined _iMX && defined eLCD_IN_OCR2
                        ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
        #else
                        ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
        #endif
                        ptDst += (((GLCD_X * CGLCD_PIXEL_SIZE) * y) + ((((x * 8) + iAdd) * 8) * CGLCD_PIXEL_SIZE)); // location in LCD display memory
                        while (ucTFTBit != 0) {                          // update 8 pixels
                            if ((ucPixelArray[y][(x * 8 + iAdd)] & ucTFTBit) != 0) { // on color to be set
        #if CGLCD_PIXEL_SIZE > 1
                                *ptDst = _LCD_PIXEL_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_PIXEL_COLOUR;
            #if CGLCD_PIXEL_SIZE > 2
                                *(ptDst + (2 * GLCD_X)) = _LCD_PIXEL_COLOUR;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_PIXEL_COLOUR;
            #endif
                                ptDst++;
                                *ptDst = _LCD_PIXEL_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_PIXEL_COLOUR;
            #if CGLCD_PIXEL_SIZE > 2
                                *(ptDst + (2 * GLCD_X)) = _LCD_PIXEL_COLOUR;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_PIXEL_COLOUR;
            #endif
                                ptDst++;
            #if CGLCD_PIXEL_SIZE > 2
                                *ptDst = _LCD_PIXEL_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_PIXEL_COLOUR;
                                *(ptDst + (2 * GLCD_X)) = _LCD_PIXEL_COLOUR;
                #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_PIXEL_COLOUR;
                #endif
                                ptDst++;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *ptDst = _LCD_PIXEL_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_PIXEL_COLOUR;
                                *(ptDst + (2 * GLCD_X)) = _LCD_PIXEL_COLOUR;
                                *(ptDst + (3 * GLCD_X)) = _LCD_PIXEL_COLOUR;
                                ptDst++;
            #endif
        #else
                                *ptDst++ = _LCD_PIXEL_COLOUR;
        #endif
                            }
                            else {                                       // off color to be set (background when backlight is on)
        #if CGLCD_PIXEL_SIZE > 1
                                *ptDst = _LCD_ON_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_ON_COLOUR;
            #if CGLCD_PIXEL_SIZE > 2
                                *(ptDst + (2 * GLCD_X)) = _LCD_ON_COLOUR;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_ON_COLOUR;
            #endif
                                ptDst++;
                                *ptDst = _LCD_ON_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_ON_COLOUR;
            #if CGLCD_PIXEL_SIZE > 2
                                *(ptDst + (2 * GLCD_X)) = _LCD_ON_COLOUR;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_ON_COLOUR;
            #endif
                                ptDst++;
            #if CGLCD_PIXEL_SIZE > 2
                                *ptDst = _LCD_ON_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_ON_COLOUR;
                                *(ptDst + (2 * GLCD_X)) = _LCD_ON_COLOUR;
                #if CGLCD_PIXEL_SIZE > 3
                                *(ptDst + (3 * GLCD_X)) = _LCD_ON_COLOUR;
                #endif
                                ptDst++;
            #endif
            #if CGLCD_PIXEL_SIZE > 3
                                *ptDst = _LCD_ON_COLOUR;
                                *(ptDst + GLCD_X) = _LCD_ON_COLOUR;
                                *(ptDst + (2 * GLCD_X)) = _LCD_ON_COLOUR;
                                *(ptDst + (3 * GLCD_X)) = _LCD_ON_COLOUR;
                                ptDst++;
            #endif
        #else
                                *ptDst++ = _LCD_ON_COLOUR;
        #endif
                            }
                            ucTFTBit >>= 1;
                        }
                        iMaxText--;
                        ucByteUpdateArray[y][x] &= ~ucBit;               // reset since the changed content has been updated
    #elif defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
                        static unsigned short usLastX_window = 0;
                        static unsigned short usLastY_window = 0;
                        unsigned short usNextPixel = (unsigned short)((((x * 8) + iAdd) * 8) * CGLCD_PIXEL_SIZE);
                        unsigned char ucCGLCDBit = 0x80;
                        if (usNextPixel >= GLCD_X) {                     // {11} if there are no more valid screen pixels stop now
                            break;
                        }

                        if ((usNextPixel != usLastX_pixel) || (((y * CGLCD_PIXEL_SIZE) + iRepeatY) != usLastY_pixel)) { // only change the draw window when necessary
                            usLastX_pixel = usLastX_window = usNextPixel;
                            usLastY_pixel = usLastY_window = ((y * CGLCD_PIXEL_SIZE) + iRepeatY);
                            fnSetWindow(usLastX_window, usLastY_window, DISPLAY_RIGHT_PIXEL, DISPLAY_BOTTOM_PIXEL);
        #if defined AVR32_EVK1105
                            SELECT_REG(HIMAX_SRAMWRITE);
        #elif defined EK_LM3S3748
                            fnWriteGLCD_cmd(ST7637_MEMORY_WRITE);
        #endif
                        }
                        while (ucCGLCDBit != 0) {                        // for each of the 8 bits in the pixel byte
                            if ((ucCGLCDBit & ucPixelArray[y][((x * 8) + iAdd)]) != 0) { // pixel is on
        #if defined AVR32_EVK1105
                                WRITE_DATA(_LCD_PIXEL_COLOUR);
            #if CGLCD_PIXEL_SIZE > 1
                                WRITE_DATA(_LCD_PIXEL_COLOUR);
            #endif
        #elif defined EK_LM3S3748 || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE
                                fnWriteGLCD_data_pair(_LCD_PIXEL_COLOUR);
            #if CGLCD_PIXEL_SIZE > 1
                                fnWriteGLCD_data_pair(_LCD_PIXEL_COLOUR);
            #endif
        #endif
                            }
                            else {                                       // pixel off
        #if defined AVR32_EVK1105
                                WRITE_DATA(_LCD_ON_COLOUR);
            #if CGLCD_PIXEL_SIZE > 1
                                WRITE_DATA(_LCD_ON_COLOUR);
            #endif
        #elif defined EK_LM3S3748 || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE
                                fnWriteGLCD_data_pair(_LCD_ON_COLOUR);   // write the background color
            #if CGLCD_PIXEL_SIZE > 1
                                fnWriteGLCD_data_pair(_LCD_ON_COLOUR);   // repeat for next physical pixel
            #endif
        #endif
                            }
                            ucCGLCDBit >>= 1;
                        }
                        usLastX_pixel += (8 * CGLCD_PIXEL_SIZE);
                        if (usLastX_pixel > DISPLAY_RIGHT_PIXEL) {
        #if defined MB785_GLCD_MODE
                            usLastX_pixel = 0;
        #else
                            usLastX_pixel = usLastX_window;
        #endif
                            usLastY_pixel++;
                            if (usLastY_pixel > DISPLAY_BOTTOM_PIXEL) {
        #if defined MB785_GLCD_MODE
                                usLastY_pixel = 0;
        #else
                                usLastY_pixel = usLastY_window;
        #endif
                            }
                        }                        
                        iMaxText--;
        #if CGLCD_PIXEL_SIZE > 1
                        if (++iRepeatY >= CGLCD_PIXEL_SIZE) {
                            ucByteUpdateArray[y][x] &= ~ucBit;           // reset since the changed content has been updated
                            iRepeatY = 0;
                        }
                        else {
                            ucCGLCDBit = 0x80;                           // repeat the line byte
                            continue;                                    // repeat for second line of larger pixel
                        }
        #else
                        ucByteUpdateArray[y][x] &= ~ucBit;               // reset since the changed content has been updated
        #endif
    #else
                        if ((iOffset + iAdd) != usLastAddress) {         // if internal address pointer needs to be changed
                            fnCommandGlcd_2(SET_ADDRESS_POINTER, (unsigned short)(iOffset + iAdd));
                        }
                        fnCommandGlcd_1(WRITE_DATA_INC, ucPixelArray[y][(x * 8 + iAdd)]); // write the GLCD display value
                        usLastAddress = ((iOffset + iAdd) + 1);
                        iMaxText--;
                        ucByteUpdateArray[y][x] &= ~ucBit;               // reset since the changed content has been updated
    #endif
                    }
                    iAdd++;
                    ucBit >>= 1;
                    if (ucBit == 0) {
                        ucBit = 0x80;
                    }
                }
                if (iMaxText <= 0) {                                     // maximum bust write has been performed
                    x_stall = (x + 1);                                   // save the update context
                    y_stall = y;
    #if defined MB785_GLCD_MODE && defined SHARE_SPI
                    usLastX_pixel = usLastY_pixel = 0xffff;              // ensure next GRAM write starts with a new cursor coordinate being set and the next GRAM write being initiated correctly
                    fnConcludeGRAMwrite();                               // close present GRAM write phase to ensure final pixel is displayed
    #endif
                    uTaskerStateChange(OWN_TASK, UTASKER_GO);            // switch to polling mode of operation
                    return STATE_LCD_WRITING;                            // busy state
                }
            }
        }
        x_stall = 0;
    }
#endif
#endif
#if defined TFT_GLCD_MODE
    fnRedrawSimDisplay();
#else
    #if defined MB785_GLCD_MODE
    usLastX_pixel = usLastY_pixel = 0xffff;                              // ensure next GRAM write starts with a new cursor coordinate being set and the next GRAM write being initiated correctly
    fnConcludeGRAMwrite();                                               // close present GRAM write phase to ensure final pixel is displayed
    #endif
    if (iStart == 0) {
        uTaskerStateChange(OWN_TASK, UTASKER_STOP);                      // switch to event mode of operation since write has completed
    }
#endif
    return STATE_LCD_READY;
}

#if !defined FT800_GLCD_MODE
// Clear backup memory and and GLCD memory content. Used only once after reset
//
static void fnClearScreen(int iMaskClear)
{
#if !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE
    #if defined NOKIA_GLCD_MODE
    int iWrites = ((GLCD_X + 2) * (GLCD_Y + 2));
    unsigned char g_pucBuffer[3];
    #elif defined _GLCD_SAMSUNG
    unsigned char ucPagesY, ucColX;
    unsigned char ucChip = GLCD_CHIP0;
    #elif defined ST7565S_GLCD_MODE                                      // {19}
    unsigned char ucPagesY, ucColX;
    #elif !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined MB785_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE
    int iWrites = (X_BYTES * Y_BYTES) + (X_BYTES * Y_BYTES);             // size of graphic and text areas
    #endif
#endif
    uMemset(ucPixelArray, 0x00, sizeof(ucPixelArray));                   // initialise screen backup memory
#if defined GLCD_INVERT_MASK
    uMemset(ucByteUpdateArrayBlink, 0x00, sizeof(ucByteUpdateArrayBlink)); // initialise blink mask memory
    uMemset(ucByteUpdateArrayInvert, 0x00, sizeof(ucByteUpdateArrayInvert)); // initialise inversion mask memory
#endif
#if defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE
    uMemset(ucByteUpdateArray, 0xff, sizeof(ucByteUpdateArray));         // cause a complete display refresh
    if (fnSmartUpdate(1) == STATE_LCD_WRITING) {                         // we must yield but aren't complete with the initialisation
        iLCD_State = STATE_LCD_CLEARING_DISPLAY;
    }
#else
    uMemset(ucByteUpdateArray, 0x00, sizeof(ucByteUpdateArray));         // initialise update array
    #if defined NOKIA_GLCD_MODE
    WriteSpiCommand(CASET);
    WriteSpiData(0);                  
    WriteSpiData(DISPLAY_RIGHT_PIXEL + 2);
    WriteSpiCommand(PASET);
    WriteSpiData(0);                  
    WriteSpiData(DISPLAY_BOTTOM_PIXEL + 2);
    WriteSpiCommand(RAMWR);                                              // data follows
    g_pucBuffer[0] = (unsigned char)(_LCD_ON_COLOUR >> 4);
    g_pucBuffer[1] = (unsigned char)((_LCD_ON_COLOUR << 4) | (_LCD_ON_COLOUR >> 8));
    g_pucBuffer[2] = (unsigned char)(_LCD_ON_COLOUR);
    while (iWrites) {
        fnSend_SPI_data(g_pucBuffer, 3);                                 // write 2 pixels to display (involving 3 bytes to be sent)
        iWrites -= 2;
    }
    #elif !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE
        #if defined _GLCD_SAMSUNG                                        // Samsung controller
    for (ucPagesY = 0; ucPagesY < 8; ucPagesY++) {                       // clear each GLCD display location
        fnGLCDGotoXY(0, (unsigned char)(ucPagesY * 8));                  // pixel position
        for (ucColX = 0; ucColX < GLCD_X; ucColX++) {                    // for each column            
            if (0 == ucColX) {                                           // select appropriate chip
                ucChip = GLCD_CHIP0;
            }
            else if (SINGLE_CHIP_WIDTH == ucColX) {              
                fnGLCDGotoXY(SINGLE_CHIP_WIDTH, (unsigned char)(ucPagesY * 8));
                ucChip = GLCD_CHIP1;
            }            
            while (GLCD_BUSY()) {}
            fnWriteGLCD_data(0, ucChip);
        }
    }
    fnGLCDGotoXY(0, 0);
        #elif defined  ST7565S_GLCD_MODE                                 // {19}
    for (ucPagesY = 0; ucPagesY < (GLCD_Y / 8); ucPagesY++) {            // clear each GLCD display location
        fnGLCDGotoXY(0, (unsigned char)(ucPagesY * 8));                  // pixel position
        for (ucColX = 0; ucColX < GLCD_X; ucColX++) {                    // for each column            
            while (GLCD_BUSY()) {}
            fnWriteGLCD_data(0x00);
        }
    }
    fnGLCDGotoXY(0, 0);
        #else                                                            // Toshiba controller
    fnCommandGlcd_2(SET_ADDRESS_POINTER, 0x0000);                        // set the address pointer to the start of graphic memory
    while (iWrites-- != 0) {
        fnCommandGlcd_1(WRITE_DATA_INC, 0x00);                           // clear the GLDC memory location
    }
        #endif
    #endif
#endif
}
#endif


#if !defined TFT_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined KITRONIX_GLCD_MODE && !defined MB785_GLCD_MODE && !defined TFT2N0369_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE
// Check that the GLCD can be detected after a reset
//
static int fnCheckLCD(void)
{
    int iMaxWait = MAX_GLCD_READY;
    while (GLCD_BUSY() != 0) {                                           // it is assumed that pull downs are set to ensure that the GLCD is not detected when not connected
        if (--iMaxWait == 0) {
            GLCD_RST_L();                                                // activate reset again in case a new attempt should be made later
            SET_PULL_DOWNS();
            return 1;                                                    // no GLCD detected
        }
    }
    REMOVE_PULL_DOWNS();                                                 // disable pull downs during normal operation
    return 0;                                                            // GLCD has been detected
}
#endif

#if defined SUPPORT_TOUCH_SCREEN
#define MAX_TOUCH_AREAS 10
static GLCD_TOUCH TouchAreas[MAX_TOUCH_AREAS] = { 0 };
static UTASK_TICK TouchDownStamp[MAX_TOUCH_AREAS] = { 0 };
static unsigned char TouchTapCount[MAX_TOUCH_AREAS] = { 0 };
static unsigned short TouchPoints[MAX_TOUCH_AREAS][2] = { {0} };
static unsigned char ucKeyPressState[MAX_TOUCH_AREAS] = { 0 };
static unsigned char ucTouchEventFlags[MAX_TOUCH_AREAS] = { 0 };

extern int fnCreateLCD_touch(GLCD_TOUCH *ptrTouch)
{
    int iTouch = 0;
    while (iTouch < MAX_TOUCH_AREAS) {
        if (ucTouchEventFlags[iTouch] == 0) {                            // it is assumed that at least one event flag is sent when an entry is valid
            uMemcpy(&TouchAreas[iTouch], ptrTouch, sizeof(GLCD_TOUCH));  // fill the empty touch entry
            ucTouchEventFlags[iTouch] = 1;                               // entry is valid
            return 0;
        }
        iTouch++;
    }
    return -1;
}

extern int fnGetTouchLocation(unsigned char ucEvent, unsigned short *ptr_usX, unsigned short *ptr_usY)
{
    int iTouch = 0;
    while (iTouch < MAX_TOUCH_AREAS) {
        if (ucTouchEventFlags[iTouch] != 0) {
            if ((TouchAreas[iTouch].touch_events.ucPressEvent == ucEvent) || (TouchAreas[iTouch].touch_events.ucMoveEvent == ucEvent)) { // location requests are expected only for pressed or moved events
                if (ptr_usX != 0) {
                    *ptr_usX = TouchPoints[iTouch][0];
                }
                if (ptr_usY != 0) {
                    *ptr_usY = TouchPoints[iTouch][1];
                }
                return 0;                                                // found
            }
        }
        iTouch++;
    }
    return -1;                                                           // not found
}

extern int fnRemoveLCD_touch(GLCD_TOUCH *ptrTouch)
{
    int iTouch = 0;
    while (iTouch < MAX_TOUCH_AREAS) {
        if (ucTouchEventFlags[iTouch] != 0) {
            unsigned char *ptrEvent = &TouchAreas[iTouch].touch_events.ucPressEvent;
            unsigned char *ptrCheckEvent = &ptrTouch->touch_events.ucPressEvent;
            int i = 0;
            while (i++ < sizeof(TOUCH_MOUSE_EVENTS)) {
                if (*ptrEvent != *ptrCheckEvent) {                       // if any events don't match it is not the entry we are searching for
                    break;;
                }
                ptrEvent++;
                ptrCheckEvent++;
            }
            if (i >= sizeof(TOUCH_MOUSE_EVENTS)) {
                ucTouchEventFlags[iTouch] = 0;                           // no longer valid
                return 0;                                                // found
            }
        }
        iTouch++;
    }
    return -1;
}

#define TAP_TIME_LIMIT       (DELAY_LIMIT)(0.08 * SEC)
#define MULTI_TAP_TIME_LIMIT (DELAY_LIMIT)(0.40 * SEC)

static void fnDoKeyRelease(int iTouch, unsigned char ucKeyID)
{
    ucKeyPressState[iTouch] &= ~(1 << ucKeyID);                          // this pen is no longer pressing in the area
    if ((uTaskerSystemTick - TouchDownStamp[iTouch]) <= TAP_TIME_LIMIT) { // if the press was short (tap)
        ++TouchTapCount[iTouch];
        switch (TouchTapCount[iTouch]) {
        case 1:                                                          // first tap
            if (TouchAreas[iTouch].touch_events.ucTappedEvent != 0) {    // if a tap event is assigned
                fnInterruptMessage(TOUCH_KEY_PARTNER_TASK, TouchAreas[iTouch].touch_events.ucTappedEvent); // send a tapped interrupt event
            }
            break;
        case 2:                                                          // second tap (double-click)
            if (TouchAreas[iTouch].touch_events.ucDoubleClickedEvent != 0) { // if a double-click event is assigned
                fnInterruptMessage(TOUCH_KEY_PARTNER_TASK, TouchAreas[iTouch].touch_events.ucDoubleClickedEvent); // send a double-click interrupt event
            }
            break;
        default:
            break;
        }
    }
    if ((ucKeyPressState[iTouch] == 0) && (TouchAreas[iTouch].touch_events.ucReleaseEvent != 0)) { // if the key is no longer pressed by any pens
        fnInterruptMessage(TOUCH_KEY_PARTNER_TASK, TouchAreas[iTouch].touch_events.ucReleaseEvent); // send as released pressed interrupt event
    }
}

static int fnHandleTouchKey(unsigned short usX, unsigned short usY, unsigned char ucKeyID, int iPressedReleased)
{
    int iReturn = 0;
    int iNotLeaveKey;
    int iTouch = 0;
    while (iTouch < MAX_TOUCH_AREAS) {                                   // for each possible key area
        iNotLeaveKey = 0;
        if ((usX >= TouchAreas[iTouch].rect_corners.usX_start) && (usX <= TouchAreas[iTouch].rect_corners.usX_end)) { // the x coordinate is within the key's x-bounds
            if ((usY >= TouchAreas[iTouch].rect_corners.usY_start) && (usY <= TouchAreas[iTouch].rect_corners.usY_end)) { // also the y coordinate is within the key's y-bounds
                if (iPressedReleased != PEN_UP_EVENT) {                  // handle key press/move
                    TouchPoints[iTouch][0] = (usX - TouchAreas[iTouch].rect_corners.usX_start); // save last detected coordinates referenced to the area
                    TouchPoints[iTouch][1] = (usY - TouchAreas[iTouch].rect_corners.usY_start);
                    if (ucKeyPressState[iTouch] == 0) {                  // if the key wasn't previously pressed by any pens
                        if (TouchTapCount[iTouch] != 0) {                // if a previous tap has been counted
                            if ((uTaskerSystemTick - TouchDownStamp[iTouch]) > MULTI_TAP_TIME_LIMIT) { // if the space between taps is lomg it can't be a double click
                                TouchTapCount[iTouch] = 0;
                            }
                        }
                        TouchDownStamp[iTouch] = uTaskerSystemTick;      // note the time that pen down was detected
                        if (TouchAreas[iTouch].touch_events.ucPressEvent != 0) { // if there is a pressed event associated with the key
                            fnInterruptMessage(TOUCH_KEY_PARTNER_TASK, TouchAreas[iTouch].touch_events.ucPressEvent); // send as pressed interrupt event
                        }
                    }
                    else if (TouchAreas[iTouch].touch_events.ucMoveEvent != 0) {
                        fnInterruptMessage(TOUCH_KEY_PARTNER_TASK, TouchAreas[iTouch].touch_events.ucMoveEvent); // send as moved interrupt event
                    }
                    ucKeyPressState[iTouch] |= (1 << ucKeyID);
                }
                else if (iPressedReleased == PEN_UP_EVENT) {             // handle key release
                    fnDoKeyRelease(iTouch, ucKeyID);
                }
                iNotLeaveKey = 1;
                if (TouchAreas[iTouch].ucCharacteristics != 0) {
                    iReturn = 1;                                         // don't display touch points in this area
                }
            }
        }
        if ((iNotLeaveKey == 0) && (iPressedReleased < PEN_UP_EVENT)) {  // handle movement outside of a key area
            if ((ucKeyPressState[iTouch] & (1 << ucKeyID)) != 0) {       // this pen was previously pressing in this area
                fnDoKeyRelease(iTouch, ucKeyID);
                if (TouchAreas[iTouch].ucCharacteristics != 0) {
                    iReturn = 1;                                         // don't display touch points in this area
                }
            }
        }
        iTouch++;                                                        // handle next possible key area
    }
    return iReturn;
}
#endif


// Text message causing the GLCD to display the text at the defined position, using the defined font
//
extern void fnDoLCD_text(GLCD_TEXT_POSITION *text_pos, const CHAR *cText)
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_TEXT_MESSAGE)];
    unsigned char ucLength;

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_GLCD_TEXT;                       // message type
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], text_pos, sizeof(GLCD_TEXT_POSITION));
    ucLength = (unsigned char)((unsigned char *)uStrcpy((CHAR *)&ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(GLCD_TEXT_POSITION)], cText) - &ucMessage[MSG_CONTENT_COMMAND + sizeof(GLCD_TEXT_POSITION)]);
    ucLength += sizeof(GLCD_TEXT_POSITION);
    ucMessage[MSG_CONTENT_LENGTH]   = ucLength;                          // message content length
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(ucLength + HEADER_LENGTH)); // send message to defined task
}

#if defined E_LCD_POLY
extern void fnDoLCD_poly(GLCD_POLY *poly, unsigned char ucPoints)
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_POLY)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task

    ucMessage[MSG_CONTENT_LENGTH] = (1 + (sizeof(GLCD_POLY) - sizeof(poly->points)) + (sizeof(poly->points[0]) *ucPoints)); // message content length
    ucMessage[MSG_CONTENT_COMMAND] = E_LCD_POLY;                         // message type
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], poly, (ucMessage[MSG_CONTENT_LENGTH] - 1));
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + ucMessage[MSG_CONTENT_LENGTH])); // send message to defined task
}
#endif

// Image message causing the GLCD to display the image (pointer to image used) at the defined position
//
extern void fnDoLCD_pic(void *pic)                                       // {13}
{
#if defined GLCD_PIC_WINDOWING
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_PIC_WINDOW)];
#else
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_PIC)];
#endif

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
#if defined GLCD_PIC_WINDOWING
    if ((((GLCD_PIC_WINDOW *)pic)->glcd_pic.ucMode & WINDOWED_PIC) != 0) { // {13}
        ucMessage[MSG_CONTENT_LENGTH] = (1 + sizeof(GLCD_PIC_WINDOW));   // message content length
        ucMessage[MSG_CONTENT_COMMAND] = E_LCD_WINDOWED_PIC;             // message type
        uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], pic, sizeof(GLCD_PIC_WINDOW));
    }
    else {
        ucMessage[MSG_CONTENT_LENGTH] = (1 + sizeof(GLCD_PIC));          // message content length
        ucMessage[MSG_CONTENT_COMMAND] = E_LCD_PIC;                      // message type
        uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], pic, sizeof(GLCD_PIC));
    }
#else
    ucMessage[MSG_CONTENT_LENGTH] = (1 + sizeof(GLCD_PIC));              // message content length
    ucMessage[MSG_CONTENT_COMMAND] = E_LCD_PIC;                          // message type
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], pic, sizeof(GLCD_PIC));
#endif
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + ucMessage[MSG_CONTENT_LENGTH])); // send message to defined task
}

extern unsigned short fnGetGbitWidth(GBITMAP *gbit)
{
    volatile unsigned short usWidth = gbit->ucWidth[0];
    usWidth <<= 8;
    usWidth |= gbit->ucWidth[1];
    return (unsigned short)usWidth;
}

extern unsigned short fnGetGbitHeight(GBITMAP *gbit)
{
    volatile unsigned short usHeight = gbit->ucHeight[0];
    usHeight <<= 8;
    usHeight |= gbit->ucHeight[1];
    return (unsigned short)usHeight;
}

#if defined E_LCD_PIC_REP
extern void fnDoLCD_pic_rep(void *pic, unsigned short usXReps, unsigned short usYReps)
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_PIC) + 4];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task

    ucMessage[MSG_CONTENT_LENGTH] = (1 + sizeof(GLCD_PIC) + 4);          // message content length
    ucMessage[MSG_CONTENT_COMMAND] = E_LCD_PIC_REP;                      // message type
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], pic, sizeof(GLCD_PIC));

    ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(GLCD_PIC)] = (unsigned char)(usXReps >> 8);
    ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(GLCD_PIC) + 1] = (unsigned char)(usXReps);
    ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(GLCD_PIC) + 2] = (unsigned char)(usYReps >> 8);
    ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(GLCD_PIC) + 3] = (unsigned char)(usYReps);

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + ucMessage[MSG_CONTENT_LENGTH])); // send message to defined task
}
#endif

#if defined E_LCD_PIC_RECOLOR
extern void fnDoLCD_recolor(IMAGE_RECOLOR *recolor)
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(IMAGE_RECOLOR)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task

    ucMessage[MSG_CONTENT_LENGTH] = (1 + sizeof(IMAGE_RECOLOR));         // message content length
    ucMessage[MSG_CONTENT_COMMAND] = E_LCD_PIC_RECOLOR;                  // message type
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], recolor, sizeof(IMAGE_RECOLOR));

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + ucMessage[MSG_CONTENT_LENGTH])); // send message to defined task
}
#endif


// Message causing the GLCD to draw a line
//
extern void fnDoLCD_line(void *line)
{
    int iLength = (sizeof(GLCD_LINE) + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_LINE_BLINK)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_LINE;                        // message type
    if ((((GLCD_LINE *)line)->ucMode & (BLINKING_OBJECT)) != 0) {
        iLength += (sizeof(GLCD_LINE_BLINK) - sizeof(GLCD_LINE));        // extended command
    }
    ucMessage[MSG_CONTENT_LENGTH]   = iLength;                           // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], line, (iLength - 1));
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}

#if defined E_LCD_RECT
// Message causing the GLCD to draw a filled rectangle
//
extern void fnDoLCD_rect(void *rect)
{
    int iLength = (sizeof(GLCD_RECT) + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_RECT_BLINK)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_RECT;                        // message type
    if ((((GLCD_RECT *)rect)->ucMode & (BLINKING_OBJECT)) != 0) {        // extended command
        iLength += (sizeof(GLCD_RECT_BLINK) - sizeof(GLCD_RECT));
    }
    ucMessage[MSG_CONTENT_LENGTH ]   = (unsigned char)iLength;           // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], rect, (iLength - 1));   // add the rectangle object

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}
#endif

#if defined E_LCD_CANVAS
extern void fnDoLCD_canvas(RECT_COORDINATES *ptrRect, int iCopyPaste)
{
    int iLength = (sizeof(RECT_COORDINATES) + 1 + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(RECT_COORDINATES) + 1];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_CANVAS;                      // message type
    ucMessage[MSG_CONTENT_LENGTH]   = (unsigned char)iLength;            // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], ptrRect, sizeof(RECT_COORDINATES)); // add the rectangle coordinates
    ucMessage[MSG_CONTENT_COMMAND + 1 + sizeof(RECT_COORDINATES)] = (unsigned char)iCopyPaste;

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}
#endif


#if defined E_LCD_ROUND_RECT
// Message causing the GLCD to draw a filled rounded rectangle
//
extern void fnDoLCD_round_rect(void *rect)
{
    int iLength = (sizeof(GLCD_ROUND_RECT) + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_ROUND_RECT)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_ROUND_RECT;                  // message type
    ucMessage[MSG_CONTENT_LENGTH]    = (unsigned char)iLength;           // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], rect, (iLength - 1));   // add the rectangle object

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}
#endif

#if defined E_LCD_CIRCLE
// Message causing the GLCD to draw a filled circle
//
extern void fnDoLCD_circle(void *circle)
{
    int iLength = (sizeof(GLCD_CIRCLE) + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_CIRCLE)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE] = INTERNAL_ROUTE;                         // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK] = LCD_PARTNER_TASK;                       // partner task
    ucMessage[MSG_CONTENT_COMMAND] = E_LCD_CIRCLE;                       // message type
    ucMessage[MSG_CONTENT_LENGTH] = (unsigned char)iLength;              // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], circle, (iLength - 1)); // add the circle object

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}
#endif

#if defined SUPPORT_TOUCH_SCREEN
// Message causing the GLCD to draw a touch point
//
extern void fnDoLCD_touch(void *touchPoint)
{
    int iLength = (sizeof(GLCD_TOUCH_POINT) + 1);
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_TOUCH_POINT)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_TOUCH_POINT;                 // message type
    ucMessage[MSG_CONTENT_LENGTH] = (unsigned char)iLength;              // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], touchPoint, (iLength - 1)); // add the touch point object

    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + iLength)); // send message to defined task
}
#endif

// Message causing the display to scroll
//
extern void fnDoLCD_scroll(GLCD_SCROLL *scroll)                          // {4}
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_SCROLL)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_SCROLL;                      // message type
    ucMessage[MSG_CONTENT_LENGTH]   = (sizeof(GLCD_SCROLL) + 1);         // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], scroll, sizeof(GLCD_SCROLL));
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + (sizeof(GLCD_SCROLL) + 1))); // send message to defined task
}

extern void fnSetContrast(unsigned char ucContrast)
{
#if defined ST7565S_GLCD_MODE
    fnWriteGLCD_cmd(0x20 | (ucContrast & 0x07));                         // 0..7
#endif
}

#if defined MGCF16404B
extern void fnSetVolume(unsigned char ucVolume)
{
    fnWriteGLCD_cmd(0x81);
    fnWriteGLCD_cmd(ucVolume & 0x03f);                                   // 0..63
}
#endif

#if defined GLCD_BACKLIGHT_CONTROL
extern void fnSetBacklight(void)
{
    #if defined FIXED_BACKLIGHT_INTENSITY
        BACK_LIGHT_INTENSITY();
    #else 
    if (temp_pars->temp_parameters.ucGLCDBacklightPWM >= 96) {           // consider as maximum brightness and apply constant '1'
        BACK_LIGHT_MAX_INTENSITY();
        #if defined _WINDOWS && defined LCD_SIMULATE_BACKLIGHT
        fnSimBackLights(1, 100);
        #endif
    }
    else if (temp_pars->temp_parameters.ucGLCDBacklightPWM == 0) {       // consider as off and apply constant '0'
        BACK_LIGHT_MIN_INTENSITY();
        #if defined _WINDOWS && defined LCD_SIMULATE_BACKLIGHT
        fnSimBackLights(0, 0);
        #endif
    }
    else {
        #if defined _M5223X || defined _KINETIS || defined _iMX
            #if defined K70F150M_12M                                     // use LCD controller's contrast PWM to control the backlight intensity
        LCDC_LPCCR = ((LCDC_LPCCR_SCR_LINE_CLK | LCDC_LPCCR_SCR_PIXEL_CLK | LCDC_LPCCR_CC_EN) | ((temp_pars->temp_parameters.ucGLCDBacklightPWM * MAX_LCD_INTENSITY_VALUE)/100));
        _CONFIG_PERIPHERAL(D, 11, PD_11_GLCD_GLCD_CONTRAST);
            #else
        PWM_INTERRUPT_SETUP timer_setup;
        timer_setup.int_type = PWM_INTERRUPT;                            // PWM setup type
        timer_setup.int_handler = 0;
        timer_setup.pwm_reference = _GLCD_BACKLIGHT_TIMER;               // the timer to generate the control signal with
        timer_setup.pwm_mode  = _GLCD_TIMER_MODE_OF_OPERATION;           // generate PWM signal on this timer output port
        timer_setup.pwm_frequency = _GLCD_BACKLIGHT_PWM_FREQUENCY;       // backlight frequency
        timer_setup.pwm_value  = _PWM_PERCENT(temp_pars->temp_parameters.ucGLCDBacklightPWM, timer_setup.pwm_frequency); // PWM value
        fnConfigureInterrupt((void *)&timer_setup);                      // configure PWM output for backlight control
            #endif
        #else
        TIMER_INTERRUPT_SETUP timer_setup;                               // PWM Timer Init Struct
        timer_setup.int_type = TIMER_INTERRUPT;                          // timer setup type
        timer_setup.int_priority = 0;
        timer_setup.int_handler = 0;                                     // no interrupts used
        timer_setup.timer_reference = _GLCD_BACKLIGHT_TIMER;             // the timer to generate the control signal with
        timer_setup.timer_mode  = _GLCD_TIMER_MODE_OF_OPERATION;         // generate PWM signal on this timer output port
        timer_setup.timer_value = _GLCD_BACKLIGHT_PWM_FREQUENCY;         // backlight frequency
        timer_setup.pwm_value   = (unsigned short)_PWM_PERCENT(temp_pars->temp_parameters.ucGLCDBacklightPWM, timer_setup.timer_value); // contrast as PWM value
        fnConfigureInterrupt((void *)&timer_setup);                      // configure PWM output for contrast control
        #endif
        #if defined _WINDOWS && defined LCD_SIMULATE_BACKLIGHT
        fnSimBackLights(1, temp_pars->temp_parameters.ucGLCDBacklightPWM);
        #endif
    }
    #endif
}        
#endif

#if !defined SUPPORT_TOUCH_SCREEN && (defined MB785_GLCD_MODE || defined TOUCH_FT6206 || defined TOUCH_FT5406) && !defined DONT_HANDLE_TOUCH_SCREEN_MOVEMENT
static void fnHandleTouch(unsigned short usX, unsigned short usY, int iPenDown)
{
    static unsigned short usLastPixelX, usLastPixelY;
    unsigned char ucMemoryContent, ucNewContent;

    if (usX < MIN_X_TOUCH) {
        usX = MIN_X_TOUCH;
    }
    else if (usX > MAX_X_TOUCH) {
        usX = MAX_X_TOUCH;
    }
    if (usY < MIN_Y_TOUCH) {
        usY = MIN_Y_TOUCH;
    }
    else if (usY > MAX_Y_TOUCH) {
        usY = MAX_Y_TOUCH;
    }

    usX = (unsigned short)(((unsigned long)((usX - MIN_X_TOUCH) * (GLCD_X - 1)))/(MAX_X_TOUCH - MIN_X_TOUCH)); // convert to pixel location
    usY = (unsigned short)(((unsigned long)((usY - MIN_Y_TOUCH) * (GLCD_Y - 1)))/(MAX_Y_TOUCH - MIN_Y_TOUCH));

    if (iPenDown == 0) {
    #if !defined _WINDOWS
        if (usX > usLastPixelX) {                                        // movement to right
            if ((usX - usLastPixelX) > MAX_MOVEMENT_X) {
                return;                                                  // step too large so ignore
            }
        }
        else {
            if ((usLastPixelX - usX) > MAX_MOVEMENT_X) {
                return;                                                  // step too large so ignore
            }
        }
        if (usY > usLastPixelY) {                                        // movement down
            if ((usY - usLastPixelY) > MAX_MOVEMENT_Y) {
                return;                                                  // step too large so ignore
            }
        }
        else {
            if ((usLastPixelY - usY) > MAX_MOVEMENT_Y) {
                return;                                                  // step too large so ignore
            }
        }
    #endif
    }
    usLastPixelX = usX;                                                  // save last valid pixel location
    usLastPixelY = usY;
    #if CGLCD_PIXEL_SIZE > 1
    usY /= CGLCD_PIXEL_SIZE;
    usX /= CGLCD_PIXEL_SIZE;
    #endif
    ucNewContent = ucMemoryContent = ucPixelArray[usY][usX/8];           // original content
    ucNewContent |= (0x80 >> usX%8);                                     // set the pixel
    if (ucNewContent != ucMemoryContent) {
        ucPixelArray[usY][usX/8] = ucNewContent;
        ucByteUpdateArray[usY][usX/64] |= (0x80 >> (usX/8)%8);           // mark the need for an update
    }
}
#endif

#if defined VARIABLE_PIXEL_COLOUR
// Message causing a change to graphic styles
//
extern void fnDoLCD_style(GLCD_STYLE *style)                             // {7}
{
    unsigned char ucMessage[HEADER_LENGTH + 1 + sizeof(GLCD_STYLE)];

    ucMessage[MSG_DESTINATION_NODE] = INTERNAL_ROUTE;                    // destination node
    ucMessage[MSG_SOURCE_NODE]      = INTERNAL_ROUTE;                    // own node
    ucMessage[MSG_DESTINATION_TASK] = TASK_LCD;                          // destination task
    ucMessage[MSG_SOURCE_TASK]      = LCD_PARTNER_TASK;                  // partner task
    ucMessage[MSG_CONTENT_COMMAND]  = E_LCD_STYLE;                       // message type
    ucMessage[MSG_CONTENT_LENGTH]   = (sizeof(GLCD_STYLE) + 1);          // message content length
    uMemcpy(&ucMessage[MSG_CONTENT_COMMAND + 1], style, sizeof(GLCD_STYLE));
    fnWrite(INTERNAL_ROUTE, ucMessage, (QUEUE_TRANSFER)(HEADER_LENGTH + (sizeof(GLCD_STYLE) + 1))); // send message to defined task
}
#endif

    #if defined USE_HTTP || defined SDCARD_SUPPORT                       // {16}
extern int iGetPixelState(unsigned long ulPixelNumber)
{
    int x = ((ulPixelNumber%GLCD_X)/CGLCD_PIXEL_SIZE);
    int y = ((ulPixelNumber/GLCD_X)/CGLCD_PIXEL_SIZE);
    unsigned char ucBit;
        #if defined ST7565S_GLCD_MODE                                    // {19} ST7565S is a samsung type
    ucBit = (0x01 << y%8);
    y /= 8;
        #else
    ucBit = (0x80 >> x%8);
    x /= 8;
        #endif
    return ((ucPixelArray[y][x] & ucBit) != 0);                          // return the state of the defined pixel
}
    #endif
#endif

#if defined USE_HTTP || defined SDCARD_SUPPORT || defined SUPPORT_TFT    // {17}
    #if !defined SUPPORT_GLCD || defined GLCD_COLOR
        #define _GLCD_COMMANDS
            #include "glcd_tft.h"
        #undef _GLCD_COMMANDS
    #endif

#if (defined MB785_GLCD_MODE || defined AVR32_EVK1105 || defined IDM_L35_B || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined OLIMEX_LPC1766_STK || defined OLIMEX_LPC2478_STK || defined IAR_LPC1788_SK || defined TWR_LCD_RGB_GLCD_MODE) && defined GLCD_X // {17}{18} accept color images
    #if defined AVR32_EVK1105
extern void et024006_SetLimits(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    WRITE_REG(HIMAX_COL_ADDR_START2, (x1 >> 8)  );
    WRITE_REG(HIMAX_COL_ADDR_START1, (x1 & 0xff));
    WRITE_REG(HIMAX_COL_ADDR_END2,   (x2 >> 8)  );
    WRITE_REG(HIMAX_COL_ADDR_END1,   (x2 & 0xff));
    WRITE_REG(HIMAX_ROW_ADDR_START2, (y1 >> 8)  );
    WRITE_REG(HIMAX_ROW_ADDR_START1, (y1 & 0xff));
    WRITE_REG(HIMAX_ROW_ADDR_END2,   (y2 >> 8)  );
    WRITE_REG(HIMAX_ROW_ADDR_END1,   (y2 & 0xff));
}
    #endif


// Receive a bit map from an input - like HTTP port, where the data arrives in multiple frames
//
extern int fnDisplayBitmap(unsigned char *ptrData, unsigned short usLength)
{
#define BMP_IDLE         0
#define BMP_HEADER_READY 1
#define BMP_BAD_FILE     2
#if defined OLIMEX_LPC2478_STK || defined IAR_LPC1788_SK || defined TWR_LCD_RGB_GLCD_MODE // {18}
    static COLORREF *ptDst;
#endif
#if defined AVR32_EVK1105 || defined IDM_L35_B || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
    static unsigned short usBitmapWidth;
#endif
    static int iState = BMP_IDLE;    
    static unsigned long ulImageHeight;
    static unsigned long ulImageWidthBytes;
    static unsigned long BMP_Length;
    static unsigned long usValidData = 0;
    static unsigned char ucCorrection;
    static unsigned char ucBuffer[3 * GLCD_X];
    if (ptrData == 0) {
        iState = BMP_IDLE;                                               // reset ready to accept another bit map
        usValidData = 0;
    }
    while (usLength != 0) {
        switch (iState) {
        case BMP_IDLE:                                                   // bit map header needs to be interpreted
            {
                unsigned short usCopyLength = (unsigned short)((sizeof(W_BITMAPHEADER) + sizeof(W_BITMAPINFO) - usValidData));
                if (usCopyLength > usLength) {                           // part header
                    usCopyLength = usLength;                             // limit to available data
                }
                else {
                    iState = BMP_HEADER_READY;
                }
                uMemcpy(&ucBuffer[usValidData], ptrData, usCopyLength);  // first fill the intermediate buffer with the bitmap header
                ptrData += usCopyLength;
                usValidData += usCopyLength;
                usLength -= usCopyLength;
                if (iState == BMP_HEADER_READY) {                        // header complete
                    W_BITMAPHEADER *ptrBMP = (W_BITMAPHEADER *)ucBuffer;
                    W_BITMAPINFO   *ptrBMP_info = (W_BITMAPINFO *)(ucBuffer + sizeof(W_BITMAPHEADER));
                    if (ptrBMP->bmType[0] != 'B') {
                        iState = BMP_BAD_FILE;
                        return -1;                                       // not a bit map
                    }
                    if (ptrBMP->bmType[1] != 'M') {
                        iState = BMP_BAD_FILE;
                        return -1;                                       // not a bit map
                    }
                    if (ptrBMP_info->biBitCount[0] != 24) {
                        iState = BMP_BAD_FILE;
                        return -2;                                       // 24 bit color expected
                    }
                    if (ptrBMP_info->biCompression[0] != 0) {
                        iState = BMP_BAD_FILE;
                        return -3;                                       // compressed image not supported
                    }
                    BMP_Length = (ptrBMP_info->biSizeImage[3] << 24);    // bit map content length
                    BMP_Length |= (ptrBMP_info->biSizeImage[2] << 16);
                    BMP_Length |= (ptrBMP_info->biSizeImage[1] << 8);
                    BMP_Length |= (ptrBMP_info->biSizeImage[0]);
                    if (BMP_Length == 0) {
                        BMP_Length = (ptrBMP->bmLength[3] << 24);        // if the content length is zero, take it from the header
                        BMP_Length |= (ptrBMP->bmLength[2] << 16);
                        BMP_Length |= (ptrBMP->bmLength[1] << 8);
                        BMP_Length |= (ptrBMP->bmLength[0]);
                        BMP_Length -= ptrBMP->bmOffBits[0];
                    }
                    ulImageWidthBytes = (ptrBMP_info->biWidth[1] << 8);
                    ulImageWidthBytes |= (ptrBMP_info->biWidth[0]);
                    ulImageHeight = (ptrBMP_info->biHeight[1] << 8);
                    ulImageHeight |= (ptrBMP_info->biHeight[0]);
                    if ((ulImageWidthBytes > GLCD_X) || (ulImageHeight > GLCD_Y)) { // {21}
                        iState = BMP_BAD_FILE;
                        return -4;                                       // image size too large for the LCD
                    }
                    ucCorrection = (unsigned char)(ulImageWidthBytes%4);
#if defined AVR32_EVK1105 || defined IDM_L35_B || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
                    usBitmapWidth = (unsigned short)ulImageWidthBytes;
#endif
                    ulImageWidthBytes *= 3;                              // three color bytes per pixel
                    usValidData = 0;
#if defined OLIMEX_LPC2478_STK || defined IAR_LPC1788_SK || defined TWR_LCD_RGB_GLCD_MODE // {18}
    #if defined _iMX && defined eLCD_IN_OCR2
                    ptDst = (COLORREF *)fnGetOCR2((unsigned char *)RAM_START_ADDRESS_OCR2);
    #else
                    ptDst = (COLORREF *)fnGetSDRAM((unsigned char *)SDRAM_ADDR);
    #endif
                    ptDst += ((ulImageHeight - 1) * GLCD_X);             // inverted bit maps, so move to bottom of image ready to draw upwards
#endif
                }
            }
            break;

        case BMP_HEADER_READY:
            {
                unsigned short usCopyLength = (unsigned short)(ulImageWidthBytes + ucCorrection - usValidData);
                unsigned long x;
                unsigned short usBufferCopy;
                if (usLength < usCopyLength) {
                    usCopyLength = usLength;
                }
                if (BMP_Length < usCopyLength) {
                    usCopyLength = (unsigned short)BMP_Length;
                }
                if ((usCopyLength + usValidData) > sizeof(ucBuffer)) {   // limit to buffer length
                    usBufferCopy = (unsigned short)(sizeof(ucBuffer) - usValidData);
                }
                else {
                    usBufferCopy = usCopyLength;
                }
                uMemcpy(&ucBuffer[usValidData], ptrData, usBufferCopy);  // first fill the intermediate buffer with the bitmap content
                ptrData += usCopyLength;
                usValidData += usCopyLength;
                usLength -= usCopyLength;
#if defined AVR32_EVK1105
                if (usValidData >= (ulImageWidthBytes + ucCorrection)) { // a complete line has been collected so write it to the display
                    unsigned long ulPixel;
                    ulImageHeight--;
                    et024006_SetLimits(0, (unsigned short)(ulImageHeight), usBitmapWidth, (unsigned short)(ulImageHeight));
                    SELECT_REG(HIMAX_SRAMWRITE);                         // prepare write
                    for (x = 0; x < ulImageWidthBytes; x += 3) {
                        ulPixel = ((ucBuffer[x] >> 3) | ((ucBuffer[x + 1] & 0xfc) << 3) | ((ucBuffer[x + 2] & 0xf8) << 8));
                        WRITE_DATA((unsigned short)ulPixel);
                    }
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData = 0;                                     // complete buffer emptied
                }
#elif defined IDM_L35_B || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
                if (usValidData >= (ulImageWidthBytes + ucCorrection)) { // a complete line has been collected so write it to the display
                    unsigned long ulPixel;
                    ulImageHeight--;
                    fnSetWindow(0, (unsigned short)(ulImageHeight), usBitmapWidth, (unsigned short)(ulImageHeight));
                    for (x = 0; x < ulImageWidthBytes; x += 3) {
                        ulPixel = ((ucBuffer[x] >> 3) | ((ucBuffer[x + 1] & 0xfc) << 3) | ((ucBuffer[x + 2] & 0xf8) << 8));
                        fnWriteGLCD_data_pair((unsigned short)ulPixel);
                    }
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData = 0;                                     // complete buffer emptied
                }
#elif defined OLIMEX_LPC2478_STK || defined IAR_LPC1788_SK || defined TWR_LCD_RGB_GLCD_MODE // {18}
                if (usValidData >= (ulImageWidthBytes + ucCorrection)) { // a complete line has been collected so write it to the display
                    COLORREF *ptNextLine = (ptDst - GLCD_X);             // position of next line, moving upwards
                    COLORREF Pixel;
                    for (x = 0; x < ulImageWidthBytes; x += 3) {
                        Pixel = ((ucBuffer[x] << 16) | (ucBuffer[x + 1] << 8) | ucBuffer[x + 2]);
                        *ptDst++ = Pixel;
                    }
                    ptDst = ptNextLine;
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData = 0;
                    fnRedrawSimDisplay();
                }
#elif defined OLIMEX_LPC1766_STK
                while (usValidData >= /*(ulImageWidthBytes + ucCorrection)*/sizeof(ucBuffer)) { // a complete line has been collected so write it to the display
//                  unsigned long ulPixel;
                  //#define WINDOW_WIDTH  DISPLAY_RIGHT_PIXEL
                    static unsigned char pucWindowX[] = {CASET, 0, DISPLAY_RIGHT_PIXEL};
                    static unsigned char pucWindowY[] = {PASET, 0, DISPLAY_BOTTOM_PIXEL};
                    ulImageHeight--;
                    if (ulImageHeight < DISPLAY_BOTTOM_PIXEL) {          // don't write any lines outside of the display area
                        int i = 0;
                        pucWindowY[1] = (unsigned char)ulImageHeight;
                        fnSendSPI_Command(pucWindowX, sizeof(pucWindowX));
                        fnSendSPI_Command(pucWindowY, sizeof(pucWindowY));
                        WriteSpiCommand(RAMWR);                              // data follows
                        for (x = 0; x < GLCD_X;) {
                            ucBuffer[x] = (ucBuffer[i++] >> 4);
                            ucBuffer[x] <<= 4;
                            ucBuffer[x++] |= (ucBuffer[i++] >> 4);
                            ucBuffer[x] <<= 4;
                            ucBuffer[x++] |= (ucBuffer[i++] >> 4);
                        }
                        fnSend_SPI_data(ucBuffer, GLCD_X);
                    }
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData -= sizeof(ucBuffer);                     // complete buffer emptied
                }
#else
                if (usValidData >= (ulImageWidthBytes + ucCorrection)) { // a complete line has been collected so write it to the display
                    unsigned long ulPixel;                  //unsigned char ucBlue = 0, ucGreen = 0, ucRed = 0;
                //  et024006_SetLimits(0, (unsigned short)(ulImageHeight), usBitmapWidth, (unsigned short)(ulImageHeight));
                    fnSetWindow(0, (unsigned short)(--ulImageHeight), DISPLAY_RIGHT_PIXEL, DISPLAY_BOTTOM_PIXEL); // set to start of line and open write
                    for (x = 0; x < ulImageWidthBytes; x += 3) {
                       /* if (ucBlue < 0x20) {
                            ulPixel = ucBlue++;
                        }
                        else if (ucGreen < 0x40) {
                            ulPixel = (ucGreen++ << 5);
                        }
                        else if (ucRed < 0x20) {
                            ulPixel = (ucRed++ << 11);
                        }
                        else {
                            ulPixel = ((ucBuffer[x] >> 3) | ((ucBuffer[x + 1] & 0xfc) << 3) | ((ucBuffer[x + 2] & 0xf8) << 8)); // convert color format
                        }*/
                        ulPixel = ((ucBuffer[x] >> 3) | ((ucBuffer[x + 1] & 0xfc) << 3) | ((ucBuffer[x + 2] & 0xf8) << 8)); // convert color format
                        fnWriteGLCD_data_pair((unsigned short)ulPixel);  // write to display
                    }
                    fnConcludeGRAMwrite();                               // close present GRAM write phase to ensure final pixel is displayed
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData = 0;                                     // complete buffer emptied
                }
#endif
                if (BMP_Length == 0) {
                    iState = BMP_IDLE;
                    return 1;                                            // bit map has been completely written to the display
                }
            }
            break;

        case BMP_BAD_FILE:                                               // data is not valid so ignore
            return -1;
        }
    }
    return 0;
}
#elif defined GLCD_X && !defined SUPPORT_SLCD                            // {17}
// Receive a bit map from an input - like HTTP port, where the data arrives in multiple frames
//
extern int fnDisplayBitmap(unsigned char *ptrData, unsigned short usLength)
{
#define BMP_IDLE         0
#define BMP_HEADER_READY 1
#define BMP_BAD_FILE     2
    static int iBMPState = BMP_IDLE;
    static unsigned char *ptDst;
    static unsigned char *ptrChanged;
    static unsigned long ulImageHeight;
    static unsigned long ulImageWidthBytes;
    static unsigned long BMP_Length;
    static unsigned char ucBuffer[3 * GLCD_X];
    static unsigned long usValidData = 0;
    static unsigned char ucCorrection;
    #if defined ST7565S_GLCD_MODE                                        // {19} Samsung type 
    static unsigned char ucPixel = 0x80;
    static unsigned char ucChangedBit = 0x80;
    #endif
    if (ptrData == 0) {
        iBMPState = BMP_IDLE;                                            // reset ready to accept another bit map
        usValidData = 0;
        uTaskerStopTimer(LCD_PARTNER_TASK);                              // stop further display updates by the application
        iSendAck = 0;                                                    // ensure no acknowledgement sent to application on draw completion
    }
    while (usLength != 0) {
        switch (iBMPState) {
        case BMP_IDLE:                                                   // bit map header needs to be interpreted
            {
                unsigned short usCopyLength = (unsigned short)((sizeof(W_BITMAPHEADER) + sizeof(W_BITMAPINFO) - usValidData));
                if (usCopyLength > usLength) {                           // part header
                    usCopyLength = usLength;                             // limit to available data
                }
                else {
                    iBMPState = BMP_HEADER_READY;
                }
                uMemcpy(&ucBuffer[usValidData], ptrData, usCopyLength);  // first fill the intermediate buffer with the bitmap header
                ptrData += usCopyLength;
                usValidData += usCopyLength;
                usLength -= usCopyLength;
                if (iBMPState == BMP_HEADER_READY) {                     // header complete
                    W_BITMAPHEADER *ptrBMP = (W_BITMAPHEADER *)ucBuffer;
                    W_BITMAPINFO   *ptrBMP_info = (W_BITMAPINFO *)(ucBuffer + sizeof(W_BITMAPHEADER));
                    if (ptrBMP->bmType[0] != 'B') {
                        iBMPState = BMP_BAD_FILE;
                        return -1;                                       // not a bit map
                    }
                    if (ptrBMP->bmType[1] != 'M') {
                        iBMPState = BMP_BAD_FILE;
                        return -1;                                       // not a bit map
                    }
                    if (ptrBMP_info->biBitCount[0] != 24) {
                        iBMPState = BMP_BAD_FILE;
                        return -2;                                       // 24 bit color expected
                    }
                    if (ptrBMP_info->biCompression[0] != 0) {
                        iBMPState = BMP_BAD_FILE;
                        return -3;                                       // compressed image not supported
                    }
                    BMP_Length = (ptrBMP_info->biSizeImage[3] << 24);    // bit map content length
                    BMP_Length |= (ptrBMP_info->biSizeImage[2] << 16);
                    BMP_Length |= (ptrBMP_info->biSizeImage[1] << 8);
                    BMP_Length |= (ptrBMP_info->biSizeImage[0]);
                    if (BMP_Length == 0) {
                        BMP_Length = (ptrBMP->bmLength[3] << 24);        // if the content length is zero, take it from the header
                        BMP_Length |= (ptrBMP->bmLength[2] << 16);
                        BMP_Length |= (ptrBMP->bmLength[1] << 8);
                        BMP_Length |= (ptrBMP->bmLength[0]);
                        BMP_Length -= ptrBMP->bmOffBits[0];
                    }
                    ulImageWidthBytes = (ptrBMP_info->biWidth[1] << 8);
                    ulImageWidthBytes |= (ptrBMP_info->biWidth[0]);
                    ulImageHeight = (ptrBMP_info->biHeight[1] << 8);
                    ulImageHeight |= (ptrBMP_info->biHeight[0]);
                    if ((ulImageWidthBytes > GLCD_X) || (ulImageHeight > GLCD_Y)) { // {21}
                        iBMPState = BMP_BAD_FILE;
                        return -4;                                       // image size too large for the LCD
                    }
                    ucCorrection = (unsigned char)(ulImageWidthBytes%4);
                    ulImageWidthBytes *= 3;                              // three color bytes per pixel
                    usValidData = 0;
                    ptDst = &ucPixelArray[0][0];
                    ptrChanged = &ucByteUpdateArray[0][0];
    #if defined ST7565S_GLCD_MODE                                        // {19}
                    ptDst += (((ulImageHeight/8) - 1) * GLCD_X);         // inverted bit maps, so move to bottom of image ready to draw upwards
                    ptrChanged += (((ulImageHeight - 1) / 64) * UPDATE_WIDTH);                   
    #else
                    ptDst += ((ulImageHeight - 1) * (GLCD_X/8));         // inverted bit maps, so move to bottom of image ready to draw upwards
                    ptrChanged += ((ulImageHeight - 1) * UPDATE_WIDTH);                   
    #endif
                }
            }
            break;

        case BMP_HEADER_READY:
            {
                unsigned short usCopyLength = (unsigned short)(ulImageWidthBytes + ucCorrection - usValidData);
                unsigned long x;
                if (usLength < usCopyLength) {
                    usCopyLength = usLength;
                }
                if (BMP_Length < usCopyLength) {
                    usCopyLength = (unsigned short)BMP_Length;
                }
                uMemcpy(&ucBuffer[usValidData], ptrData, usCopyLength);  // first fill the intermediate buffer with the bitmap content
                ptrData += usCopyLength;
                usValidData += usCopyLength;
                usLength -= usCopyLength;
                if (usValidData >= (ulImageWidthBytes + ucCorrection)) { // a complete line has been collected so write it to the display
                    unsigned long ulPixel;
                    unsigned char ucOriginal = *ptDst;
    #if defined ST7565S_GLCD_MODE                                        //  {19}
                    unsigned char *ptNextLine = ptDst;
                    unsigned char *ptNextChange = ptrChanged;
    #else
                    unsigned char ucPixel = 0x80;
                    unsigned char *ptNextLine = ptDst - (GLCD_X/8);      // position of next line, moving upwards
                    unsigned char *ptNextChange = ptrChanged - UPDATE_WIDTH; // position of next change, moving upwards
                    unsigned char ucChangedBit = 0x80;
    #endif
    #if defined ST7565S_GLCD_MODE                                        //  {19}
                    ulImageHeight--;
                    for (x = 0; x < ulImageWidthBytes; x += 3) {         // write a complete pixel line 
                        ulPixel = ucBuffer[x] + ucBuffer[x + 1] + ucBuffer[x + 2];
                        if (ulPixel >= (3 * 0xf0)) {                     // cut-off for white
                            *ptDst |= ucPixel;
                        }
                        else {
                            *ptDst &= ~ucPixel;
                        }
                        if (ucOriginal != *ptDst) {                      // check whether a byte has changed
                            *ptrChanged |= ucChangedBit;                 // mark the byte to be updated
                        }
                        ptDst++;
                        ptrChanged++;
                        ucOriginal = *ptDst;
                    }
                    ptrChanged = ptNextChange;
                    ucPixel >>= 1;                                       // next line upwards
                    if (ucPixel == 0)  {
                        ucPixel = 0x80;                                  // start with the last
                        ptDst = (ptNextLine - GLCD_X);                   // position of next line in memory, moving upwards
                        ucChangedBit >>= 1;                              // next change line, upwards 
                        if (ucChangedBit == 0) {
                            ucChangedBit = 0x80;                         // start with the last
                            ptrChanged = ptNextChange - UPDATE_WIDTH;    // position of next change line in memory, moving upwards
                        }
                    }
                    else {
                        ptDst = ptNextLine;
                    }
    #else
                    ulImageHeight--;
                    for (x = 0; x < ulImageWidthBytes; x += 3) {
                        ulPixel = ucBuffer[x] + ucBuffer[x + 1] + ucBuffer[x + 2];
                        if (ulPixel >= (3 * 0xf0)) {                     // cut-off for white
                            *ptDst |= ucPixel;
                        }
                        else {
                            *ptDst &= ~ucPixel;
                        }
                        ucPixel >>= 1;
                        if (ucPixel == 0) {
                            ucPixel = 0x80;
                            if (ucOriginal != *ptDst) {                  // check whether a byte has changed
                                *ptrChanged |= ucChangedBit;             // mark the byte to be updated
                            }
                            ucChangedBit >>= 1;
                            if (ucChangedBit == 0) {
                                ucChangedBit = 0x80;
                                ptrChanged++;
                            }
                            ptDst++;
                            ucOriginal = *ptDst;
                        }
                    }
                    ptDst = ptNextLine;
                    ptrChanged = ptNextChange;
    #endif
                    BMP_Length -= (ulImageWidthBytes + ucCorrection);
                    usValidData = 0;                                     // complete buffer emptied
                }
                if (BMP_Length == 0) {
                    iBMPState = BMP_IDLE;
                    iLCD_State = fnSmartUpdate(1);                       // command an update of the display
                    return 1;                                            // bit map has been completely written to the display
                }
            }
            break;

        case BMP_BAD_FILE:                                               // data is not valid so ignore
            return -1;
        }
    }
    return 0;
}
#endif
#endif
