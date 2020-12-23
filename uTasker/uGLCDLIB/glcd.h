/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      glcd.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    11.08.2009 Add FONT_CHAR_LCD                                         {1}     
    11.08.2009 Add GLCD_LINE and GLCD_RECT                               {2}
    15.08.2009 Add GLCD_LINE_BLINK and GLCD_RECT_BLINK                   {3}
    26.09.2009 Add GLCD message routines prototypes                      {4}
    23.11.2009 Struct packing control removed to driver.h
    13.05.2010 Add KITRONIX_GLCD_MODE
    26.06.2010 Add MB785_GLCD_MODE
    05.03.2011 Add windowed picture                                      {5}
    15.03.2015 Add TOUCH_MOUSE_EVENT_MESSAGE struct                      {6}
    25.11.2017 Add fnSetLCDContrast()                                    {7}
    18.11.2020 Add circle and round rectangle drawing, plus touch points
       
*/

#if !defined _EXCLUDE_WINDOWS_

#if !defined MAX_TEXT_LENGTH
    #define MAX_TEXT_LENGTH             64                               // default text length
#endif

#if defined TFT_GLCD_MODE && !defined CGLCD_GLCD_MODE && !defined eLCD_IN_16_BIT_MODE
    #define COLORREF unsigned long
    #define RGB(r,g,b)                  (COLORREF)((b << 16) | (g << 8) | r)
#else
    #define COLORREF unsigned short
    #if defined _HX8347 || defined KITRONIX_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined MB785_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined _iMX
        #define BLACK 0x0000
        #define BLUE  0x001f
        #define RED   0xf800
        #define GREEN 0x07e0
        #define WHITE 0xffff
        #define RGB(r,g,b)              (COLORREF)(((r << 8) & RED) | ((g << 3) & GREEN) | ((b >> 3) & BLUE))
    #else
        #define RGB(r,g,b)              (COLORREF)(((b << 4) & 0x0f00) | (g & 0x00f0) | (r >> 4))
    #endif
#endif

typedef struct stGLCD_TEXT_POSITION {
    unsigned char  ucMode;
    unsigned short usX;
    unsigned short usY;
    unsigned char  ucFont;
} GLCD_TEXT_POSITION;

__PACK_ON                                                                // compilers using pragmas to control packing will start struct packing from here
typedef struct _PACK stGLCD_TEXT_MESSAGE {
    GLCD_TEXT_POSITION text_position;
	CHAR text[MAX_TEXT_LENGTH];
} GLCD_TEXT_MESSAGE;

typedef struct _PACK stGLCD_SCROLL {
    unsigned char ucMode;
    signed short  sX_scroll;
    signed short  sY_scroll;
} GLCD_SCROLL;

typedef struct _PACK stGLCD_STYLE {
    unsigned char ucMode;
    COLORREF      color;
} GLCD_STYLE;

typedef struct _PACK stPOINT_COORDINATE {
    unsigned short usX;
    unsigned short usY;
} POINT_COORDINATE;

typedef struct _PACK stRECT_COORDINATES {
    unsigned short usX_start;
    unsigned short usY_start;
    unsigned short usX_end;
    unsigned short usY_end;
} RECT_COORDINATES;

typedef struct _PACK stGLCD_LINE {                                       // {2}
    unsigned char ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy bytes to align the following to 4 byte boundary
#endif
    RECT_COORDINATES line_start_end;
} GLCD_LINE;

typedef struct _PACK stGLCD_LINE_BLINK {                                 // {3}
    unsigned char ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy bytes to align the following to 4 byte boundary
#endif
    RECT_COORDINATES line_start_end;
    DELAY_LIMIT   blink_half_period;
} GLCD_LINE_BLINK;

typedef struct _PACK stGLCD_ROUND_RECT {
    unsigned char ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy bytes to align the following to 4 byte boundary
#endif
    RECT_COORDINATES rect_corners;
    unsigned short usRadius;
} GLCD_ROUND_RECT;

typedef struct _PACK stGLCD_RECT {                                       // {2}
    unsigned char ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy bytes to align the following to 4 byte boundary
#endif
    RECT_COORDINATES rect_corners;
} GLCD_RECT;

typedef struct _PACK stGLCD_CIRCLE {
    unsigned char ucMode;
    unsigned short usCenterX;
    unsigned short usCenterY;
    unsigned short usRadius;
} GLCD_CIRCLE;

typedef struct _PACK stGLCD_TOUCH_POINT {
    unsigned char ucMode;
    unsigned char ucRef;
    unsigned short usCenterX;
    unsigned short usCenterY;
    unsigned short usRadius;
} GLCD_TOUCH_POINT;

typedef struct _PACK stGLCD_RECT_BLINK {                                 // {3}
    unsigned char ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy byte1 to align the following to 4 byte boundary
#endif
    RECT_COORDINATES rect_corners;
    DELAY_LIMIT   blink_half_period;
} GLCD_RECT_BLINK;

typedef struct _PACK stGBITMAP {
    unsigned char ucType;
    unsigned char ucWidth[2];
    unsigned char ucHeight[2];
    unsigned char ucData[1];
} GBITMAP;

extern unsigned short fnGetGbitWidth(GBITMAP *gbit);
extern unsigned short fnGetGbitHeight(GBITMAP *gbit);

#define GBITMAP_

typedef struct _PACK stGLCD_PIC {
    unsigned char  ucMode;
#if defined _iMX
    unsigned char  ucRes[3];                                             // pack with dummy bytes to align the following to 4 byte boundaries
#endif
    unsigned short usX;
    unsigned short usY;
    GBITMAP       *ptrPic;                                               // this is guaranteed to be on a long word boundary
#if defined GLCD_EXTENSION_1
    unsigned short usOffsetY;
    unsigned short usHeight;
#endif
} GLCD_PIC;

typedef struct _PACK stIMAGE_RECOLOR {
    GLCD_PIC glcd_pic;
    unsigned short usXreps;
    unsigned short usYreps;
    COLORREF newColor;
} IMAGE_RECOLOR;

typedef struct _PACK stGLCD_PIC_WINDOW {                                 // {5}
    GLCD_PIC      glcd_pic;                                              // GLCD_PIC element
    RECT_COORDINATES rect_corners;                                       // rectangle of the part of the picture to be written
} GLCD_PIC_WINDOW;

typedef struct stTOUCH_MOUSE_EVENT_MESSAGE                               // {6}
{
    unsigned short usX;                                                  // present X coordinate
    unsigned short usY;                                                  // present Y coordinate
    unsigned char  ucEvent;                                              // event type
} TOUCH_MOUSE_EVENT_MESSAGE;

typedef struct stTOUCH_MOUSE_EVENTS
{
    unsigned char ucPressEvent;
    unsigned char ucReleaseEvent;
    unsigned char ucMoveEvent;
    unsigned char ucTappedEvent;
    unsigned char ucDoubleClickedEvent;
} TOUCH_MOUSE_EVENTS;

typedef struct _PACK stGLCD_TOUCH {
    RECT_COORDINATES rect_corners;
    TOUCH_MOUSE_EVENTS touch_events;
    unsigned char ucCharacteristics;
} GLCD_TOUCH;

#define MAX_POLY_POINTS   6
typedef struct _PACK stGLCD_POLY {
    unsigned char  ucMode;
    unsigned char  ucAntiAliasing;
#if defined _iMX
    unsigned char  ucRes[2];                                             // pack with dummy bytes to align the following to 4 byte boundaries
#endif
    COLORREF       lineColor;
    COLORREF       fillColor;
    POINT_COORDINATE points[MAX_POLY_POINTS];                            // coordinates
} GLCD_POLY;

#define TOUCH_MOUSE_PRESS          1
#define TOUCH_MOUSE_RELEASE        2
#define TOUCH_MOUSE_MOVE           3
#define TOUCH_MOUSE_CALIBRATION    4

// Define number labels for the font selections 
//
typedef enum
{
#if defined EN_FIVE_DOT
	FONT_FIVE_DOT,
#endif
#if defined EN_FIVE_DOT_FIX
	FONT_FIVE_DOT_FIX,
#endif
#if defined EN_SIX_DOT
	FONT_SIX_DOT,
#endif
#if defined EN_SIX_DOT_FIX
	FONT_SIX_DOT_FIX,
#endif
#if defined EN_SEVEN_DOT
	FONT_SEVEN_DOT,
#endif
#if defined EN_CHAR_LCD_DOT                                              // {1}
    FONT_CHAR_LCD,
#endif
#if defined EN_NINE_DOT
	FONT_NINE_DOT,
#endif
#if defined EN_TEN_DOT
	FONT_TEN_DOT,
#endif
#if defined EN_FIFTEEN_DOT
	FONT_FIFTEEN_DOT,
#endif
#if defined EN_EIGHTEEN_DOT
	FONT_EIGHTEEN_DOT,
#endif
#if defined EN_USER_DOT
    FONT_USER_DOT,
#endif
#if defined EN_USER_DOT2
    FONT_USER_DOT2,
#endif
    FONT_COUNT                                                           // this enum is equal to the number of active fonts
} FONT_BASE;

#if defined EN_FIVE_DOT || defined EN_FIVE_DOT_FIX || defined EN_SIX_DOT || defined EN_SIX_DOT_FIX || defined EN_SEVEN_DOT || defined EN_CHAR_LCD_DOT || defined EN_NINE_DOT || defined EN_TEN_DOT || defined EN_FIFTEEN_DOT || defined EN_EIGHTEEN_DOT
    #define FONTS_AVAILABLE
#endif

// Bit map structures as used by Windows
//
typedef struct _PACK stBITMAPHEADER {
    unsigned char  bmType[2];
    unsigned char  bmLength[4]; 
    unsigned char  bmReserved[4];
    unsigned char  bmOffBits[4];
} W_BITMAPHEADER;

typedef struct _PACK stBITMAPINFO { 
    unsigned char biSize[4]; 
    unsigned char biWidth[4]; 
    unsigned char biHeight[4]; 
    unsigned char biPlanes[2];
    unsigned char biBitCount[2];
    unsigned char biCompression[4];
    unsigned char biSizeImage[4];
    unsigned char biXPelsPerMeter[4];
    unsigned char biYPelsPerMeter[4];
    unsigned char biClrUsed[4];
    unsigned char biClrImportant[4]; 
} W_BITMAPINFO;
__PACK_OFF

#define PAINT_LIGHT             0x00                                     // object drawn with the light color
#define PAINT_DARK              0x01                                     // object drawn with the dark color (background)
#define PAINT_INVERTED          0x02                                     // object drawn with inverted color to the present image content
#define PAINT_RAW               (PAINT_DARK | PAINT_INVERTED)            // raw object keeping backup of original content
#define BLINKING_OBJECT         0x04                                     // the object to be drawn should use blink mode
#define KILL_BLINK_OBJECTS      0x04                                     // used together with scrolling command
#define WINDOWED_PIC            0x08                                     // {5} GLCD_PIC_WINDOW type instead of GLCD_PIC
#if defined GLCD_EXTENSION_1
#define SHOW_EXTRA          0x10
#endif
#define EXTENDED_MODE           0x20                                     // additional control bytes
#define GIVE_ACK                0x40                                     // return an ACK event when the operation has completed
#define REDRAW                  0x80                                     // redraw the display after updating the image buffer

#define ADD_INVERSION_MASK      (REDRAW | WINDOWED_PIC | BLINKING_OBJECT)
#define REMOVE_INVERSION_MASK   (GIVE_ACK | WINDOWED_PIC | BLINKING_OBJECT)
#define ADD_CURSOR_MASK         (REDRAW | EXTENDED_MODE | BLINKING_OBJECT)
#define REMOVE_CURSOR_MASK      (GIVE_ACK | EXTENDED_MODE | BLINKING_OBJECT)

#define STYLE_MASK              0x0f
#define STYLE_BACKGROUND_COLOR  0x00
#define STYLE_PIXEL_COLOR       0x01
#define STYLE_TRANSPARENT_COLOR 0x02
#define STYLE_TRANSPARENT_OFF   0x03

#define ROUND_TO_PROJECTED_PIXEL(x)   ((x + (CGLCD_PIXEL_SIZE -1))/CGLCD_PIXEL_SIZE)


extern void fnDoLCD_text(GLCD_TEXT_POSITION *text_pos, const CHAR *cText); // {4}
extern void fnDoLCD_pic(void *pic);                                      // {5}
extern void fnDoLCD_pic(void *pic);
extern void fnDoLCD_pic_rep(void *pic, unsigned short usXReps, unsigned short usYReps);
extern void fnDoLCD_line(void *line);
extern void fnDoLCD_rect(void *rect);
extern void fnDoLCD_round_rect(void *rect);
extern void fnDoLCD_circle(void *circle);
extern void fnDoLCD_scroll(GLCD_SCROLL *scroll);
extern void fnDoLCD_style(GLCD_STYLE *style);
extern void fnDoLCD_poly(GLCD_POLY *poly, unsigned char ucPoints);
extern void fnDoLCD_recolor(IMAGE_RECOLOR *recolor);
extern void fnDoLCD_canvas(RECT_COORDINATES *ptrRect, int iCopyPaste);
#if defined SUPPORT_TOUCH_SCREEN
    extern void fnDoLCD_touch(void *touchPoint);
    extern int fnCreateLCD_touch(GLCD_TOUCH *ptrTouch);
    extern int fnRemoveLCD_touch(GLCD_TOUCH *ptrTouch);
    extern int fnGetTouchLocation(unsigned char ucEvent, unsigned short *ptr_usX, unsigned short *ptr_usY);
#endif

extern void fnSetLCDContrast(unsigned char ucContrast);                  // {7}

#if defined FT800_GLCD_MODE
typedef char ft_char8_t;
typedef signed char ft_schar8_t;
typedef unsigned char ft_uchar8_t;
typedef ft_uchar8_t ft_uint8_t;
typedef short  ft_int16_t;
typedef unsigned short ft_uint16_t;
typedef unsigned int ft_uint32_t;
typedef int ft_int32_t;
typedef void ft_void_t;
typedef long long ft_int64_t;
typedef unsigned long long ft_uint64_t;
typedef float ft_float_t;
typedef double ft_double_t;
typedef char ft_bool_t;

#define ft_prog_uchar8_t  ft_uchar8_t
#define ft_prog_char8_t   ft_char8_t
#define ft_prog_uint16_t  ft_uint16_t

typedef struct {
    unsigned short   ft_cmd_fifo_wp;                                     // coprocessor fifo write pointer
    int iCoProcessorWait;
    unsigned char *buffer;
    unsigned long count;
    unsigned long length;
} Ft_Gpu_Hal_Context_t;


extern void Ft_Gpu_HostCommand(void *host, unsigned char ucCommand);
extern unsigned char Ft_Gpu_Hal_Rd8(void *host, unsigned long ulReg);
extern void Ft_Gpu_Hal_WrMem(void *host, unsigned long ulReg, const unsigned char *ptrData, unsigned long ulDataLength);
extern void Ft_Gpu_Hal_Wr8(void *host, unsigned long ulReg, unsigned char ucData);
extern void Ft_Gpu_Hal_Wr16(void *host, unsigned long ulReg, unsigned short usData);
extern unsigned short Ft_Gpu_Hal_Rd16(void *host, unsigned long ulReg);
extern int  Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host, unsigned char *buffer, unsigned long count);
extern int  Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host);
extern void Ft_App_WrCoCmd_Buffer(Ft_Gpu_Hal_Context_t *phost, unsigned long cmd);
extern void Ft_App_WrCoStr_Buffer(Ft_Gpu_Hal_Context_t *phost, const unsigned char *s);
extern int  Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost);
extern void Ft_Gpu_Hal_RdMem(void *host, unsigned long ulReg, unsigned char *buffer, unsigned long length);

#include "FT_Gpu.h"
#include "FT_CoPro_Cmds.h"

#if defined BRIDGETEK_GLIB
    #include "Bridgetek_GLIB/EVE_Hal.h"

    ft_void_t Ft_Gpu_CoCmd_DlStart(EVE_HalContext *phost);
    bool Ft_Gpu_CoCmd_SendCmd(EVE_HalContext *phost, uint32_t value);
    ft_uint32_t Ft_Bridgetek_Gpu_CoCmd_Calibrate(EVE_HalContext *phost);
#else
    #define FT_PROGMEM

    typedef struct {
	    ft_uint8_t reserved;
    } Ft_Gpu_App_Context_t;

    typedef struct {
	    union {
		    ft_uint8_t spi_cs_pin_no;		//spi chip select number of ft8xx chip
		    ft_uint8_t i2c_addr;			//i2c address of ft8xx chip
	    };
	    union {
		    ft_uint16_t spi_clockrate_khz;  //In KHz
		    ft_uint16_t i2c_clockrate_khz;  //In KHz
	    };
	    ft_uint8_t channel_no;				//mpsse channel number
	    ft_uint8_t pdn_pin_no;				//ft8xx power down pin number
    } Ft_Gpu_Hal_Config_t;

    typedef enum {
	    FT_GPU_HAL_OPENED,
	    FT_GPU_HAL_READING,
	    FT_GPU_HAL_WRITING,
	    FT_GPU_HAL_CLOSED,

	    FT_GPU_HAL_STATUS_COUNT,
	    FT_GPU_HAL_STATUS_ERROR = FT_GPU_HAL_STATUS_COUNT
    } FT_GPU_HAL_STATUS_E;


    #define Ft_Gpu_Hal_WrMemFromFlash(a,b,c,d)   Ft_Gpu_Hal_WrMem(a,b,c,d)

    extern void Ft_Gpu_Hal_Wr32(void *host, unsigned long ulReg, unsigned long ulValue);
    extern unsigned long Ft_Gpu_Hal_Rd32(void *host, unsigned long ulReg);

    extern void Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host, unsigned long count);
    extern unsigned short Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host);
    extern void Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host, unsigned long count);

    extern int  SAMAPP_GPU_DLSwap(ft_uint8_t DL_Swap_Type);
    extern void Ft_App_WrDlCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,unsigned long cmd);
    extern void Ft_App_Flush_DL_Buffer(Ft_Gpu_Hal_Context_t *phost);
#endif
#endif

#endif
