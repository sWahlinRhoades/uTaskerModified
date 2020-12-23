/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      glcd_fonts.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2019
    *********************************************************************
    This file is not linked directly in the project but is included by the GLCD task file

    Acknowledgements are made to Martin Honeywill of Kinesys Projects Limited who supplied the fonts,
    based on previous work by Michael J. Karas

    The project defines which of the fonts are actually available as consts

    10.08.2009 Font sets 7, 10, 15, 18 Dot corrected to respect bit map format requirement
    10.08.2009 Remove store_width                                        {1}
    10.08.2009 Extended dot width tables to unsigned short               {2}
    11.08.2009 Add graphical LCD style font (EN_CHAR_LCD_DOT)            {3}
    28.08.2010 Add user definable font extension                         {4}

*/ 

#if !defined USER_SPECIAL_SEVEN_DOT
    #define USER_SPECIAL_SEVEN_DOT 0
#endif
#if !defined USER_SPECIAL_NINE_DOT
    #define USER_SPECIAL_NINE_DOT 0
#endif
#if !defined USER_SPECIAL_TEN_DOT
    #define USER_SPECIAL_TEN_DOT 0
#endif
#if !defined USER_SPECIAL_FIFTEEN_DOT
    #define USER_SPECIAL_FIFTEEN_DOT 0
#endif
#if !defined USER_SPECIAL_EIGHTEEN_DOT
    #define USER_SPECIAL_EIGHTEEN_DOT 0
#endif

// Definition tables for fonts - fonts usually include 96 glyphs from 0x20 to 0x7F that are in ASCII order
//
struct FONT_DEF 
{
  //unsigned char store_width;                                           // {1} glyph storage width in bytes 
    unsigned char glyph_height;  		                                 // glyph height for storage 
    const unsigned char *glyph_table;                                    // font table start address in memory 
    unsigned char fixed_width;                                           // fixed width of glyphs - if zero then use the width table
    const unsigned short  *width_table; 	                             // {2} variable width table start adress 
    unsigned char glyph_beg;			 	                             // start ascii offset in table 
    unsigned char glyph_end;				                             // end ascii offset in table 
    unsigned char glyph_def;				                             // code for undefined glyph code 
};

#if defined FONTS_AVAILABLE
// Font definition tables for the three fonts 
//
extern const struct FONT_DEF fonts[FONT_COUNT];
#endif

// Glyph bitmap and width tables for the fonts
//
#if defined EN_FIVE_DOT
    extern const unsigned char  five_dot_glyph_table[];
    extern const unsigned short five_dot_width_table[];
#endif
#if defined EN_SIX_DOT
    extern const unsigned char  six_dot_glyph_table[];
    extern const unsigned short six_dot_width_table[];
#endif
#if defined EN_USER_DOT
    extern const unsigned char  user_dot_glyph_table[];
#endif
#if defined EN_USER_DOT2
    extern const unsigned char  user_dot2_glyph_table[];
#endif
#if defined EN_SEVEN_DOT
    extern const unsigned char  seven_dot_glyph_table[];
    extern const unsigned short seven_dot_width_table[];
#endif
#if defined EN_CHAR_LCD_DOT
    extern const unsigned char  seven_five_lcd_glyph_table[];    
#endif
#if defined EN_NINE_DOT
    extern const unsigned char  nine_dot_glyph_table[];
#endif
#if defined EN_TEN_DOT
    extern const unsigned char  ten_dot_glyph_table[];
#endif
#if defined EN_FIFTEEN_DOT
    extern const unsigned char  fifteen_dot_glyph_table[];
    extern const unsigned short fifteen_dot_width_table[];
#endif
#if defined EN_EIGHTEEN_DOT
    extern const unsigned char  eighteen_dot_glyph_table[];
    extern const unsigned short eighteen_dot_width_table[];
#endif

#if defined FONTS_AVAILABLE
    #if !defined USER_DOT_HEIGHT
        #define USER_DOT_HEIGHT   8
    #endif
    #if !defined USER_DOT_WIDTH
        #define USER_DOT_WIDTH    5
    #endif
const struct FONT_DEF fonts[FONT_COUNT] = {
    #if defined EN_FIVE_DOT
    {7, five_dot_glyph_table, 0, five_dot_width_table,' ','~','.'},
    #endif
    #if defined EN_FIVE_DOT_FIX
    {7, five_dot_glyph_table, 5, 0,' ','~','.'},
    #endif
    #if defined EN_SIX_DOT
    {8, six_dot_glyph_table, 0, six_dot_width_table,' ','~','.'},
    #endif
    #if defined EN_SIX_DOT_FIX
    {8, six_dot_glyph_table, 6, six_dot_width_table,' ','~','.'},        // share width table for exceptions
    #endif
    #if defined EN_SEVEN_DOT
    {8, seven_dot_glyph_table, 0, seven_dot_width_table,' ', ('~' + USER_SPECIAL_SEVEN_DOT) ,'.'}, // {4}
    #endif
    #if defined EN_CHAR_LCD_DOT
    {8, seven_five_lcd_glyph_table, 6, 0,' ',0xff,0xff},                 // fixed 5 pixel width with one pixel space to right
    #endif
    #if defined EN_NINE_DOT
    {12, nine_dot_glyph_table, 8, 0,' ', ('~' + USER_SPECIAL_NINE_DOT), '.'}, // {4}
    #endif
    #if defined EN_TEN_DOT
    {12, ten_dot_glyph_table,  9, 0,' ', ('~' + USER_SPECIAL_TEN_DOT), '.'}, // {4}
    #endif
    #if defined EN_FIFTEEN_DOT
    {18, fifteen_dot_glyph_table, 0, fifteen_dot_width_table,' ', ('~' + USER_SPECIAL_FIFTEEN_DOT), '.'}, // {4}
    #endif
    #if defined EN_EIGHTEEN_DOT
    {18, eighteen_dot_glyph_table, 0, eighteen_dot_width_table,' ', ('9' + USER_SPECIAL_EIGHTEEN_DOT), '.'}, // {4}
    #endif
    #if defined EN_USER_DOT
    {USER_DOT_HEIGHT, user_dot_glyph_table, USER_DOT_WIDTH, 0, USER_DOT_START, USER_DOT_END,'.'},
    #endif
    #if defined EN_USER_DOT
    {USER_DOT2_HEIGHT, user_dot2_glyph_table, USER_DOT2_WIDTH, 0, USER_DOT2_START, USER_DOT2_END,'.'},
    #endif
};
#endif


#if defined EN_CHAR_LCD_DOT
const unsigned char seven_five_lcd_glyph_table[] = {                     // {3}
    // ' ' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '!' charwidth: 5
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,
    0x04 << 3,
    0x00 << 3,
    // '"' charwidth: 5
    0x0a << 3,
    0x0a << 3,
    0x0a << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '#' charwidth: 5
    0x0a << 3,
    0x0a << 3,
    0x1f << 3,
    0x0a << 3,
    0x1f << 3,
    0x0a << 3,
    0x0a << 3,
    0x00 << 3,
    // '$' charwidth: 5 
    0x04 << 3,
    0x0f << 3,
    0x14 << 3,
    0x0e << 3,
    0x05 << 3,
    0x1e << 3,
    0x04 << 3,
    0x00 << 3,
    // '%' charwidth: 5
    0x18 << 3,
    0x19 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x13 << 3,
    0x03 << 3,
    0x00 << 3,
    // '&' charwidth: 5
    0x0c << 3,
    0x12 << 3,
    0x14 << 3,
    0x08 << 3,
    0x15 << 3,
    0x12 << 3,
    0x0d << 3,
    0x00 << 3,
    // ''' charwidth: 5
    0x0c << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '(' charwidth: 5
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x04 << 3,
    0x02 << 3,
    0x00 << 3,
    // ')' charwidth: 5
    0x08 << 3,
    0x04 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    // '*' charwidth: 5
    0x00 << 3,
    0x04 << 3,
    0x15 << 3,
    0x0e << 3,
    0x15 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,
    // '+' charwidth: 5
    0x00 << 3,
    0x04 << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,
    // ',' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x0c << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    // '-' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '.' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x0c << 3,
    0x0c << 3,
    0x00 << 3,
    // '/' charwidth: 5
    0x00 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x00 << 3,
    0x00 << 3,
    // '0' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x13 << 3,
    0x15 << 3,
    0x19 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // '1' charwidth: 5
    0x04 << 3,
    0x0c << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x0e << 3,
    0x00 << 3,
    // '2' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x1f << 3,
    0x00 << 3,
    // '3' charwidth: 5 
    0x1f << 3,
    0x02 << 3,
    0x04 << 3,
    0x02 << 3,
    0x01 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // '4' charwidth: 5
    0x02 << 3,
    0x06 << 3,
    0x0a << 3,
    0x12 << 3,
    0x1f << 3,
    0x02 << 3,
    0x02 << 3,
    0x00 << 3,
    // '5' charwidth: 5
    0x1f << 3,
    0x10 << 3,
    0x1e << 3,
    0x01 << 3,
    0x01 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // '6' charwidth: 5
    0x06 << 3,
    0x08 << 3,
    0x10 << 3,
    0x1e << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // '7' charwidth: 5
    0x1f << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x00 << 3,
    // '8' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // '9' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,
    0x02 << 3,
    0x0c << 3,
    0x00 << 3,
    // ':' charwidth: 5
    0x00 << 3,
    0x0c << 3,
    0x0c << 3,
    0x00 << 3,
    0x0c << 3,
    0x0c << 3,
    0x00 << 3,
    0x00 << 3,
    // ':' charwidth: 5
    0x00 << 3,
    0x0c << 3,
    0x0c << 3,
    0x00 << 3,
    0x0c << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    // '<' charwidth: 5
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x08 << 3,
    0x04 << 3,
    0x02 << 3,
    0x00 << 3,
    // '=' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '>' charwidth: 5
    0x08 << 3,
    0x04 << 3,
    0x02 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    // '?' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,
    0x04 << 3,
    0x00 << 3,
    // '@' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x01 << 3,
    0x0d << 3,
    0x15 << 3,
    0x15 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'A' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'B' charwidth: 5
    0x1e << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x00 << 3,
    // 'C' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'D' charwidth: 5
    0x1c << 3,
    0x12 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x12 << 3,
    0x1c << 3,
    0x00 << 3,
    // 'E' charwidth: 5     
    0x1f << 3,
    0x10 << 3,
    0x10 << 3,
    0x1e << 3,
    0x10 << 3,
    0x10 << 3,
    0x1f << 3,
    0x00 << 3,
    // 'F' charwidth: 5
    0x1f << 3,
    0x10 << 3,
    0x10 << 3,
    0x1e << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x00 << 3,
    // 'G' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x10 << 3,
    0x17 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x00 << 3,
    // 'H' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'I' charwidth: 5
    0x0e << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'J' charwidth: 5
    0x07 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x12 << 3,
    0x0c << 3,
    0x00 << 3,
    // 'K' charwidth: 5
    0x11 << 3,
    0x12 << 3,
    0x14 << 3,
    0x18 << 3,
    0x14 << 3,
    0x12 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'L' charwidth: 5
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x1f << 3,
    0x00 << 3,
    // 'M' charwidth: 5
    0x11 << 3,
    0x1b << 3,
    0x15 << 3,
    0x15 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'N' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x19 << 3,
    0x15 << 3,
    0x13 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'O' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'P' charwidth: 5    
    0x1e << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x00 << 3,
    // 'Q' charwidth: 5
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x15 << 3,
    0x12 << 3,
    0x0d << 3,
    0x00 << 3,
    // 'R' charwidth: 5     
    0x1e << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x14 << 3,
    0x12 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'S' charwidth: 5
    0x0f << 3,
    0x10 << 3,
    0x10 << 3,
    0x0e << 3,
    0x01 << 3,
    0x01 << 3,
    0x1e << 3,
    0x00 << 3,
    // 'T' charwidth: 5
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    // 'U' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'V' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x00 << 3,
    // 'W' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x15 << 3,
    0x15 << 3,
    0x15 << 3,
    0x0a << 3,
    0x00 << 3,
    // 'X' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x0a << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'Y' charwidth: 5
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    // 'Z' charwidth: 5
    0x1f << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x1f << 3,
    0x00 << 3,
    // '[' charwidth: 5
    0x0e << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x0e << 3,
    0x00 << 3,
    // '�' charwidth: 5
    0x11 << 3,
    0x0a << 3,
    0x1f << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    // ']' charwidth: 5
    0x0e << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x0e << 3,
    0x00 << 3,
    // '^' charwidth: 5
    0x04 << 3,
    0x0a << 3,
    0x11 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // '_' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    // ''' charwidth: 5
    0x08 << 3,
    0x04 << 3,
    0x02 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    // 'a' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x01 << 3,
    0x0f << 3,
    0x11 << 3,
    0x0f << 3,
    0x00 << 3,
    // 'b' charwidth: 5     
    0x10 << 3,
    0x10 << 3,
    0x16 << 3,
    0x19 << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x00 << 3,
    // 'c' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'd' charwidth: 5
    0x01 << 3,
    0x01 << 3,
    0x0d << 3,
    0x13 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x00 << 3,
    // 'e' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x1f << 3,
    0x10 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'f' charwidth: 5
    0x06 << 3,
    0x09 << 3,
    0x08 << 3,
    0x1c << 3,
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x00 << 3,
    // 'g' charwidth: 5
    0x00 << 3,
    0x0f << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'h' charwidth: 5
    0x10 << 3,
    0x10 << 3,
    0x16 << 3,
    0x19 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'i' charwidth: 5
    0x04 << 3,
    0x00 << 3,
    0x0c << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'j' charwidth: 5
    0x02 << 3,
    0x00 << 3,
    0x06 << 3,
    0x02 << 3,
    0x02 << 3,
    0x12 << 3,
    0x0c << 3,
    0x00 << 3,
    // 'k' charwidth: 5
    0x10 << 3,
    0x10 << 3,
    0x12 << 3,
    0x14 << 3,
    0x18 << 3,
    0x14 << 3,
    0x12 << 3,
    0x00 << 3,
    // 'l' charwidth: 5
    0x0c << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'm' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x1a << 3,
    0x15 << 3,
    0x15 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'n' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x16 << 3,
    0x19 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,
    // 'o' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'p' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x1e << 3,
    0x11 << 3,
    0x1e << 3,
    0x10 << 3,
    0x10 << 3,
    0x00 << 3,
    // 'q' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0d << 3,
    0x13 << 3,
    0x0f << 3,
    0x01 << 3,
    0x01 << 3,
    0x00 << 3,
    // 'r' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x16 << 3,
    0x19 << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x00 << 3,
    // 's' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x10 << 3,
    0x0e << 3,
    0x01 << 3,
    0x1e << 3,
    0x00 << 3,
    // 't' charwidth: 5
    0x08 << 3,
    0x08 << 3,
    0x1c << 3,
    0x08 << 3,
    0x08 << 3,
    0x09 << 3,
    0x06 << 3,
    0x00 << 3,
    // 'u' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x13 << 3,
    0x0d << 3,
    0x00 << 3,
    // 'v' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x00 << 3,
    // 'w' charwidth: 5     
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x15 << 3,
    0x15 << 3,
    0x0a << 3,
    0x00 << 3,
    // 'x' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x0a << 3,
    0x11 << 3,
    0x00 << 3,
    // 'y' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,
    0x0e << 3,
    0x00 << 3,
    // 'z' charwidth: 5
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x1f << 3,
    0x00 << 3,
    // '{' charwidth: 5
    0x02 << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x04 << 3,
    0x04 << 3,
    0x02 << 3,
    0x00 << 3,
    // '|' charwidth: 5
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    // '}' charwidth: 5
    0x08 << 3,
    0x04 << 3,
    0x04 << 3,
    0x02 << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,
    // '->' charwidth: 5     
    0x00 << 3,
    0x04 << 3,
    0x02 << 3,
    0x1f << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,
    // '<-' charwidth: 5
    0x00 << 3,
    0x04 << 3,
    0x08 << 3,
    0x1f << 3,
    0x08 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,


    // ' ' charwidth: 5 - start of 33 empty characters
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,


    
    0x00 << 3,                                                           // start of special characters
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1c << 3,
    0x14 << 3,
    0x1c << 3,
    0x00 << 3,

     
    0x07 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x1c << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x10 << 3,
    0x08 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x0c << 3,
    0x0c << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x1f << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x06 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x02 << 3,
    0x04 << 3,
    0x0c << 3,
    0x14 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x04 << 3,
    0x0f << 3,
    0x11 << 3,
    0x01 << 3,
    0x06 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x02 << 3,
    0x1f << 3,
    0x06 << 3,
    0x0a << 3,
    0x12 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x08 << 3,
    0x1f << 3,
    0x09 << 3,
    0x0a << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x02 << 3,
    0x02 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x1e << 3,
    0x02 << 3,
    0x1e << 3,
    0x02 << 3,
    0x1e << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x15 << 3,
    0x15 << 3,
    0x01 << 3,
    0x06 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x1f << 3,
    0x01 << 3,
    0x05 << 3,
    0x06 << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x0c << 3,
    0x14 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x02 << 3,
    0x1f << 3,
    0x02 << 3,
    0x06 << 3,
    0x0a << 3,
    0x12 << 3,
    0x02 << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x1f << 3,
    0x09 << 3,
    0x09 << 3,
    0x09 << 3,
    0x09 << 3,
    0x12 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x0f << 3,
    0x09 << 3,
    0x11 << 3,
    0x01 << 3,
    0x02 << 3,
    0x0c << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x0f << 3,
    0x12 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x01 << 3,
    0x01 << 3,
    0x01 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x0a << 3,
    0x1f << 3,
    0x0a << 3,
    0x0a << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x18 << 3,
    0x01 << 3,
    0x19 << 3,
    0x01 << 3,
    0x02 << 3,
    0x1c << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x0a << 3,
    0x11 << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x1f << 3,
    0x09 << 3,
    0x0a << 3,
    0x08 << 3,
    0x08 << 3,
    0x07 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x09 << 3,
    0x01 << 3,
    0x02 << 3,
    0x0c << 3,
    0x00 << 3,


    0x00 << 3,
    0x0f << 3,
    0x09 << 3,
    0x15 << 3,
    0x03 << 3,
    0x02 << 3,
    0x0c << 3,
    0x00 << 3,

     
    0x02 << 3,
    0x1c << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x15 << 3,
    0x15 << 3,
    0x15 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x0e << 3,
    0x00 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x08 << 3,
    0x08 << 3,
    0x0c << 3,
    0x0a << 3,
    0x08 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x0e << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x0a << 3,
    0x04 << 3,
    0x0a << 3,
    0x10 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x1f << 3,
    0x02 << 3,
    0x04 << 3,
    0x0e << 3,
    0x15 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x04 << 3,
    0x02 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,

     
    0x10 << 3,
    0x10 << 3,
    0x1f << 3,
    0x10 << 3,
    0x10 << 3,
    0x10 << 3,
    0x0f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x01 << 3,
    0x01 << 3,
    0x02 << 3,
    0x0c << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x08 << 3,
    0x14 << 3,
    0x02 << 3,
    0x01 << 3,
    0x01 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x15 << 3,
    0x15 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x01 << 3,
    0x0a << 3,
    0x04 << 3,
    0x02 << 3,
    0x00 << 3,



    0x00 << 3,
    0x0e << 3,
    0x00 << 3,
    0x0e << 3,
    0x00 << 3,
    0x0e << 3,
    0x01 << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x10 << 3,
    0x11 << 3,
    0x1f << 3,
    0x01 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x01 << 3,
    0x01 << 3,
    0x0a << 3,
    0x04 << 3,
    0x0a << 3,
    0x10 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x08 << 3,
    0x1f << 3,
    0x08 << 3,
    0x08 << 3,
    0x07 << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x08 << 3,
    0x1f << 3,
    0x09 << 3,
    0x0a << 3,
    0x08 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x0e << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x1f << 3,
    0x01 << 3,
    0x01 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x0e << 3,
    0x00 << 3,
    0x1f << 3,
    0x01 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x12 << 3,
    0x12 << 3,
    0x12 << 3,
    0x12 << 3,
    0x02 << 3,
    0x04 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x04 << 3,
    0x14 << 3,
    0x14 << 3,
    0x15 << 3,
    0x15 << 3,
    0x16 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x10 << 3,
    0x10 << 3,
    0x11 << 3,
    0x12 << 3,
    0x14 << 3,
    0x18 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x1f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x01 << 3,
    0x02 << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x18 << 3,
    0x00 << 3,
    0x01 << 3,
    0x01 << 3,
    0x02 << 3,
    0x1c << 3,
    0x00 << 3,

     
    0x04 << 3,
    0x12 << 3,
    0x08 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x1c << 3,
    0x14 << 3,
    0x1c << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x09 << 3,
    0x15 << 3,
    0x12 << 3,
    0x12 << 3,
    0x0d << 3,
    0x00 << 3,

     
    0x0a << 3,
    0x00 << 3,
    0x0e << 3,
    0x01 << 3,
    0x0f << 3,
    0x11 << 3,
    0x0f << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x1e << 3,
    0x11 << 3,
    0x1e << 3,
    0x10 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x10 << 3,
    0x0c << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x13 << 3,
    0x1d << 3,
    0x10 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x0f << 3,
    0x14 << 3,
    0x12 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x06 << 3,
    0x09 << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x10 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x0f << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,

     
    0x00 << 3,
    0x00 << 3,
    0x07 << 3,
    0x04 << 3,
    0x04 << 3,
    0x14 << 3,
    0x08 << 3,
    0x00 << 3,

     
    0x02 << 3,
    0x1a << 3,
    0x02 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x02 << 3,
    0x00 << 3,
    0x06 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,
    0x02 << 3,

     
    0x00 << 3,
    0x14 << 3,
    0x08 << 3,
    0x14 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,

     
    0x00 << 3,
    0x04 << 3,
    0x0e << 3,
    0x14 << 3,
    0x15 << 3,
    0x0e << 3,
    0x04 << 3,
    0x00 << 3,

     
    0x08 << 3,
    0x08 << 3,
    0x1c << 3,
    0x08 << 3,
    0x1c << 3,
    0x08 << 3,
    0x0f << 3,
    0x00 << 3,

     
    0x0e << 3,
    0x00 << 3,
    0x16 << 3,
    0x19 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,

     
    0x0a << 3,
    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x16 << 3,
    0x19 << 3,
    0x11 << 3,
    0x11 << 3,
    0x1e << 3,
    0x10 << 3,


    0x00 << 3,
    0x00 << 3,
    0x0d << 3,
    0x13 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,


    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x0e << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x0b << 3,
    0x15 << 3,
    0x1a << 3,
    0x00 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x0e << 3,
    0x11 << 3,
    0x11 << 3,
    0x0a << 3,
    0x1b << 3,
    0x00 << 3,


    0x0a << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x13 << 3,
    0x0d << 3,
    0x00 << 3,


    0x1f << 3,
    0x10 << 3,
    0x08 << 3,
    0x04 << 3,
    0x08 << 3,
    0x10 << 3,
    0x1f << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x0a << 3,
    0x0a << 3,
    0x0a << 3,
    0x13 << 3,
    0x00 << 3,


    0x1f << 3,
    0x00 << 3,
    0x11 << 3,
    0x0a << 3,
    0x04 << 3,
    0x0a << 3,
    0x11 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x11 << 3,
    0x0f << 3,
    0x01 << 3,


    0x00 << 3,
    0x01 << 3,
    0x1e << 3,
    0x04 << 3,
    0x1f << 3,
    0x04 << 3,
    0x04 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x08 << 3,
    0x0f << 3,
    0x09 << 3,
    0x11 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x1f << 3,
    0x15 << 3,
    0x1f << 3,
    0x11 << 3,
    0x11 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x04 << 3,
    0x00 << 3,
    0x1f << 3,
    0x00 << 3,
    0x04 << 3,
    0x00 << 3,


    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,
    0x00 << 3,


    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
    0x1f << 3,
};
#endif


#if defined EN_FIVE_DOT
const unsigned char five_dot_glyph_table[] = {
    // ' ' charwidth: 2 
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  

    // '!' charwidth: 2 
    0x00, 	//  [  ]  
    0x80, 	//  [* ]  
    0x80, 	//  [* ]  
    0x80, 	//  [* ]  
    0x00, 	//  [  ]  
    0x80, 	//  [* ]  
    0x00, 	//  [  ]  

    // '"' charwidth: 4 
    0x00, 	//  [    ]  
    0xA0, 	//  [* * ]  
    0xA0, 	//  [* * ]  
    0x00, 	//  [    ]  
    0x00, 	//  [    ]  
    0x00, 	//  [    ]  
    0x00, 	//  [    ]  

    // '#' charwidth: 8 
    0x00, 	//  [        ]  
    0x14, 	//  [   * *  ]  
    0x7E, 	//  [ ****** ]  
    0x28, 	//  [  * *   ]  
    0xFC, 	//  [******  ]  
    0x50, 	//  [ * *    ]  
    0x00, 	//  [        ]  

    // '$' charwidth: 4 
    0x40, 	//  [ *  ]  
    0x60, 	//  [ ** ]  
    0x80, 	//  [*   ]  
    0x40, 	//  [ *  ]  
    0x20, 	//  [  * ]  
    0xC0, 	//  [**  ]  
    0x40, 	//  [ *  ]  

    // '%' charwidth: 8 
    0x00, 	//  [        ]  
    0x64, 	//  [ **  *  ]  
    0xA8, 	//  [* * *   ]  
    0xD6, 	//  [** * ** ]  
    0x2A, 	//  [  * * * ]  
    0x4C, 	//  [ *  **  ]  
    0x00, 	//  [        ]  

    // '&' charwidth: 6 
    0x00, 	//  [      ]  
    0x60, 	//  [ **   ]  
    0x90, 	//  [*  *  ]  
    0x40, 	//  [ *    ]  
    0x98, 	//  [*  ** ]  
    0x60, 	//  [ **   ]  
    0x00, 	//  [      ]  

    // ''' charwidth: 2 
    0x00, 	//  [  ]  
    0x80, 	//  [* ]  
    0x80, 	//  [* ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  
    0x00, 	//  [  ]  

    // '(' charwidth: 3 
    0x40, 	//  [ * ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x40, 	//  [ * ]  

    // ')' charwidth: 3 
    0x80, 	//  [*  ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x80, 	//  [*  ]  

    // '*' charwidth: 6
    0x00, 	//  [      ]
    0x50, 	//  [ * *  ]
    0x20, 	//  [  *   ]
    0xF8, 	//  [***** ]
    0x20, 	//  [  *   ]
    0x50, 	//  [ * *  ]
    0x00, 	//  [      ]

    // '+' charwidth: 6 
    0x00, 	//  [      ]
    0x20, 	//  [  *   ]
    0x20, 	//  [  *   ]
    0xF8, 	//  [***** ]
    0x20, 	//  [  *   ]
    0x20, 	//  [  *   ]
    0x00, 	//  [      ]

    // ',' charwidth: 3
    0x00, 	//  [   ]
    0x00, 	//  [   ]
    0x00, 	//  [   ]
    0x00, 	//  [   ]
    0x40, 	//  [ * ]
    0x40, 	//  [ * ]
    0x80, 	//  [*  ]

    // '-' charwidth: 4
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 

    // '.' charwidth: 2
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]

    // '/' charwidth: 4
    0x20, 	//  [  * ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x00, 	//  [    ] 

    // '0' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // '1' charwidth: 3
    0x00, 	//  [   ]  
    0x40, 	//  [ * ]  
    0xC0, 	//  [** ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x00, 	//  [   ]  

    // '2' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x20, 	//  [  *  ]
    0x40, 	//  [ *   ]
    0xF0, 	//  [**** ]
    0x00, 	//  [     ]

    // '3' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x20, 	//  [  *  ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // '4' charwidth: 5
    0x00, 	//  [     ]
    0x20, 	//  [  *  ]
    0x60, 	//  [ **  ]
    0xA0, 	//  [* *  ]
    0xF0, 	//  [**** ]
    0x20, 	//  [  *  ]
    0x00, 	//  [     ]

    // '5' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x80, 	//  [*   ] 
    0xE0, 	//  [*** ] 
    0x20, 	//  [  * ] 
    0xC0, 	//  [**  ] 
    0x00, 	//  [    ] 

    // '6' charwidth: 5
    0x00, 	//  [     ]
    0x20, 	//  [  *  ]
    0x40, 	//  [ *   ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // '7' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x00, 	//  [    ] 

    // '8' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // '9' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x20, 	//  [  *  ]
    0x40, 	//  [ *   ]
    0x00, 	//  [     ]

    // ':' charwidth: 2
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]

    // ';' charwidth: 3
    0x00, 	//  [   ]  
    0x00, 	//  [   ]  
    0x40, 	//  [ * ]  
    0x00, 	//  [   ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x80, 	//  [*  ]  

    // '<' charwidth: 4
    0x00, 	//  [    ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x80, 	//  [*   ] 
    0x40, 	//  [ *  ] 
    0x20, 	//  [  * ] 
    0x00, 	//  [    ] 

    // '=' charwidth: 5
    0x00, 	//  [     ]
    0x00, 	//  [     ]
    0xF0, 	//  [**** ]
    0x00, 	//  [     ]
    0xF0, 	//  [**** ]
    0x00, 	//  [     ]
    0x00, 	//  [     ]

    // '>' charwidth: 4
    0x00, 	//  [    ] 
    0x80, 	//  [*   ] 
    0x40, 	//  [ *  ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x80, 	//  [*   ] 
    0x00, 	//  [    ] 

    // '?' charwidth: 4
    0x00, 	//  [    ] 
    0xC0, 	//  [**  ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x00, 	//  [    ] 
    0x40, 	//  [ *  ] 
    0x00, 	//  [    ] 

    // '@' charwidth: 8
    0x38, 	//  [  ***   ]
    0x44, 	//  [ *   *  ]
    0x9A, 	//  [*  ** * ]
    0xAA, 	//  [* * * * ]
    0xB4, 	//  [* ** *  ]
    0x40, 	//  [ *      ]
    0x30, 	//  [  **    ]

    // 'A' charwidth: 6 
    0x00, 	//  [      ]
    0x20, 	//  [  *   ]
    0x20, 	//  [  *   ]
    0x50, 	//  [ * *  ]
    0x70, 	//  [ ***  ]
    0x88, 	//  [*   * ]
    0x00, 	//  [      ]

    // 'B' charwidth: 5
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x00, 	//  [     ]

    // 'C' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x80, 	//  [*    ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // 'D' charwidth: 5
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x00, 	//  [     ]

    // 'E' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x80, 	//  [*   ] 
    0xE0, 	//  [*** ] 
    0x80, 	//  [*   ] 
    0xE0, 	//  [*** ] 
    0x00, 	//  [    ] 

    // 'F' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x80, 	//  [*   ] 
    0xE0, 	//  [*** ] 
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x00, 	//  [    ] 

    // 'G' charwidth: 5
    0x00, 	//  [     ]
    0x70, 	//  [ *** ]
    0x80, 	//  [*    ]
    0xB0, 	//  [* ** ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x00, 	//  [     ]

    // 'H' charwidth: 5
    0x00, 	//  [     ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0xF0, 	//  [**** ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x00, 	//  [     ]

    // 'I' charwidth: 2
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]

    // 'J' charwidth: 5 
    0x00, 	//  [     ] 
    0x10, 	//  [   * ] 
    0x10, 	//  [   * ] 
    0x10, 	//  [   * ] 
    0x90, 	//  [*  * ] 
    0x60, 	//  [ **  ] 
    0x00, 	//  [     ] 

    // 'K' charwidth: 5 
    0x00, 	//  [     ] 
    0x90, 	//  [*  * ] 
    0xA0, 	//  [* *  ] 
    0xC0, 	//  [**   ] 
    0xA0, 	//  [* *  ] 
    0x90, 	//  [*  * ] 
    0x00, 	//  [     ] 

    // 'L' charwidth: 4 
    0x00, 	//  [    ]  
    0x80, 	//  [*   ]  
    0x80, 	//  [*   ]  
    0x80, 	//  [*   ]  
    0x80, 	//  [*   ]  
    0xE0, 	//  [*** ]  
    0x00, 	//  [    ]  

    // 'M' charwidth: 6 
    0x00, 	//  [      ]
    0x88, 	//  [*   * ]
    0xD8, 	//  [** ** ]
    0xA8, 	//  [* * * ]
    0x88, 	//  [*   * ]
    0x88, 	//  [*   * ]
    0x00, 	//  [      ]

    // 'N' charwidth: 6 
    0x00, 	//  [      ]
    0x88, 	//  [*   * ]
    0xC8, 	//  [**  * ]
    0xA8, 	//  [* * * ]
    0x98, 	//  [*  ** ]
    0x88, 	//  [*   * ]
    0x00, 	//  [      ]

    // 'O' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // 'P' charwidth: 5
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x80, 	//  [*    ]
    0x80, 	//  [*    ]
    0x00, 	//  [     ]

    // 'Q' charwidth: 5
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x10, 	//  [   * ]

    // 'R' charwidth: 5
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0xA0, 	//  [* *  ]
    0x90, 	//  [*  * ]
    0x00, 	//  [     ]

    // 'S' charwidth: 4
    0x00, 	//  [    ] 
    0x60, 	//  [ ** ] 
    0x80, 	//  [*   ] 
    0x40, 	//  [ *  ] 
    0x20, 	//  [  * ] 
    0xC0, 	//  [**  ] 
    0x00, 	//  [    ] 

    // 'T' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x00, 	//  [    ] 

    // 'U' charwidth: 5
    0x00, 	//  [     ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // 'V' charwidth: 6
    0x00, 	//  [      ]  
    0x88, 	//  [*   * ]  
    0x50, 	//  [ * *  ]  
    0x50, 	//  [ * *  ]  
    0x20, 	//  [  *   ]  
    0x20, 	//  [  *   ]  
    0x00, 	//  [      ]  

    // 'W' charwidth: 8
    0x00, 	//  [        ]
    0x82, 	//  [*     * ]
    0x54, 	//  [ * * *  ]
    0x54, 	//  [ * * *  ]
    0x28, 	//  [  * *   ]
    0x28, 	//  [  * *   ]
    0x00, 	//  [        ]

    // 'X' charwidth: 6 
    0x00, 	//  [      ]
    0x88, 	//  [*   * ]
    0x50, 	//  [ * *  ]
    0x20, 	//  [  *   ]
    0x50, 	//  [ * *  ]
    0x88, 	//  [*   * ]
    0x00, 	//  [      ]

    // 'Y' charwidth: 6 
    0x00, 	//  [      ]
    0x88, 	//  [*   * ]
    0x50, 	//  [ * *  ]
    0x20, 	//  [  *   ]
    0x20, 	//  [  *   ]
    0x20, 	//  [  *   ]
    0x00, 	//  [      ]

    // 'Z' charwidth: 4
    0x00, 	//  [    ] 
    0xE0, 	//  [*** ] 
    0x20, 	//  [  * ] 
    0x40, 	//  [ *  ] 
    0x80, 	//  [*   ] 
    0xE0, 	//  [*** ] 
    0x00, 	//  [    ] 

    // '[' charwidth: 3
    0xC0, 	//  [** ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0xC0, 	//  [** ]  

    // '\' charwidth: 4
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x40, 	//  [ *  ] 
    0x40, 	//  [ *  ] 
    0x20, 	//  [  * ] 
    0x20, 	//  [  * ] 
    0x00, 	//  [    ] 

    // ']' charwidth: 3
    0xC0, 	//  [** ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0xC0, 	//  [** ]  

    // '^' charwidth: 6 
    0x00, 	//  [      ]
    0x20, 	//  [  *   ]
    0x50, 	//  [ * *  ]
    0x88, 	//  [*   * ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]

    // '_' charwidth: 6 
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0xF8, 	//  [***** ]

    // '`' charwidth: 3
    0x00, 	//  [   ]  
    0x80, 	//  [*  ]  
    0x40, 	//  [ * ]  
    0x00, 	//  [   ]  
    0x00, 	//  [   ]  
    0x00, 	//  [   ]  
    0x00, 	//  [   ]  

    // 'a' charwidth: 5
    0x00, 	//  [     ]
    0x00, 	//  [     ]
    0x70, 	//  [ *** ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x00, 	//  [     ]

    // 'b' charwidth: 5
    0x00, 	//  [     ]
    0x80, 	//  [*    ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x00, 	//  [     ]

    // 'c' charwidth: 4
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0x60, 	//  [ ** ] 
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x60, 	//  [ ** ] 
    0x00, 	//  [    ] 

    // 'd' charwidth: 5
    0x00, 	//  [     ]
    0x10, 	//  [   * ]
    0x70, 	//  [ *** ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x00, 	//  [     ]

    // 'e' charwidth: 4
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0x60, 	//  [ ** ] 
    0xA0, 	//  [* * ] 
    0xC0, 	//  [**  ] 
    0x60, 	//  [ ** ] 
    0x00, 	//  [    ] 

    // 'f' charwidth: 4
    0x00, 	//  [    ] 
    0x60, 	//  [ ** ] 
    0x80, 	//  [*   ] 
    0xC0, 	//  [**  ] 
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x00, 	//  [    ] 

    // 'g' charwidth: 5
    0x00, 	//  [     ]
    0x70, 	//  [ *** ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x10, 	//  [   * ]
    0x60, 	//  [ **  ]

    // 'h' charwidth: 5
    0x00, 	//  [     ]
    0x80, 	//  [*    ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x00, 	//  [     ]

    // 'i' charwidth: 2
    0x80, 	//  [* ]
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]

    // 'j' charwidth: 3
    0x40, 	//  [ * ]  
    0x00, 	//  [   ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x40, 	//  [ * ]  
    0x80, 	//  [*  ]  

    // 'k' charwidth: 5
    0x00, 	//  [     ]
    0x80, 	//  [*    ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0xA0, 	//  [* *  ]
    0x90, 	//  [*  * ]
    0x00, 	//  [     ]

    // 'l' charwidth: 2 
    0x00, 	//  [  ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x80, 	//  [* ]
    0x00, 	//  [  ]

    // 'm' charwidth: 6 
    0x00, 	//  [      ]
    0x00, 	//  [      ]
    0xF0, 	//  [****  ]
    0xA8, 	//  [* * * ]
    0xA8, 	//  [* * * ]
    0xA8, 	//  [* * * ]
    0x00, 	//  [      ]

    // 'n' charwidth: 5
    0x00, 	//  [     ]
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x00, 	//  [     ]

    // 'o' charwidth: 5
    0x00, 	//  [     ]
    0x00, 	//  [     ]
    0x60, 	//  [ **  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x60, 	//  [ **  ]
    0x00, 	//  [     ]

    // 'p' charwidth: 5
    0x00, 	//  [     ]
    0xE0, 	//  [***  ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0xE0, 	//  [***  ]
    0x80, 	//  [*    ]
    0x80, 	//  [*    ]

    // 'q' charwidth: 5
    0x00, 	//  [     ]
    0x70, 	//  [ *** ]
    0x90, 	//  [*  * ]
    0x90, 	//  [*  * ]
    0x70, 	//  [ *** ]
    0x10, 	//  [   * ]
    0x10, 	//  [   * ]

    // 'r' charwidth: 4
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0xA0, 	//  [* * ] 
    0xC0, 	//  [**  ] 
    0x80, 	//  [*   ] 
    0x80, 	//  [*   ] 
    0x00, 	//  [    ] 

    // 's' charwidth: 4
    0x00, 	//  [    ] 
    0x00, 	//  [    ] 
    0x60, 	//  [ ** ] 
    0xC0, 	//  [**  ] 
    0x20, 	//  [  * ] 
    0xC0, 	//  [**  ] 
    0x00, 	//  [    ] 

    // 't' charwidth: 3
    0x00, 	//  [   ]  
    0x80, 	//  [*  ]  
    0xC0, 	//  [** ]  
    0x80, 	//  [*  ]  
    0x80, 	//  [*  ]  
    0x40, 	//  [ * ]  
    0x00, 	//  [   ]  

    // 'u' charwidth: 5 
    0x00, 	//  [     ]    
    0x00, 	//  [     ]    
    0x90, 	//  [*  * ]    
    0x90, 	//  [*  * ]    
    0x90, 	//  [*  * ]    
    0x70, 	//  [ *** ]    
    0x00, 	//  [     ]    

    // 'v' charwidth: 4
    0x00, 	//  [    ]     
    0x00, 	//  [    ]     
    0xA0, 	//  [* * ]     
    0xA0, 	//  [* * ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0x00, 	//  [    ]     

    // 'w' charwidth: 6
    0x00, 	//  [      ]   
    0x00, 	//  [      ]   
    0xA8, 	//  [* * * ]   
    0xA8, 	//  [* * * ]   
    0x50, 	//  [ * *  ]   
    0x50, 	//  [ * *  ]   
    0x00, 	//  [      ]   

    // 'x' charwidth: 4
    0x00, 	//  [    ]     
    0x00, 	//  [    ]     
    0xA0, 	//  [* * ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0xA0, 	//  [* * ]     
    0x00, 	//  [    ]     

    // 'y' charwidth: 4
    0x00, 	//  [    ]     
    0x00, 	//  [    ]     
    0xA0, 	//  [* * ]     
    0xA0, 	//  [* * ]     
    0x40, 	//  [ *  ]     
    0x80, 	//  [*   ]     
    0x80, 	//  [*   ]     

    // 'z' charwidth: 4
    0x00, 	//  [    ]     
    0x00, 	//  [    ]     
    0xE0, 	//  [*** ]     
    0x40, 	//  [ *  ]     
    0x80, 	//  [*   ]     
    0xE0, 	//  [*** ]     
    0x00, 	//  [    ]     

    // '{' charwidth: 4
    0x20, 	//  [  * ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0x80, 	//  [*   ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0x20, 	//  [  * ]     

    // '|' charwidth: 2
    0x80, 	//  [* ]       
    0x80, 	//  [* ]       
    0x80, 	//  [* ]       
    0x80, 	//  [* ]       
    0x80, 	//  [* ]       
    0x80, 	//  [* ]       
    0x00, 	//  [  ]       

    // '}' charwidth: 4
    0x80, 	//  [*   ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0x20, 	//  [  * ]     
    0x40, 	//  [ *  ]     
    0x40, 	//  [ *  ]     
    0x80, 	//  [*   ]     

    // '~' charwidth: 7
    0x00, 	//  [       ]  
    0x64, 	//  [ **  * ]  
    0x98, 	//  [*  **  ]  
    0x00, 	//  [       ]  
    0x00, 	//  [       ]  
    0x00, 	//  [       ]  
    0x00, 	//  [       ]  
};

const unsigned short five_dot_width_table[] = {                          // {2}
		 2, // ' '
		 2, // '!'
		 4, // '"'
		 8, // '#'
		 4, // '$'
		 8, // '%'
		 6, // '&'
		 2, // '''
		 3, // '('
		 3, // ')'
		 6, // '*'
		 6, // '+'
		 3, // ','
		 4, // '-'
		 2, // '.'
		 4, // '/'
		 5, // '0'
		 3, // '1'
		 5, // '2'
		 5, // '3'
		 5, // '4'
		 4, // '5'
		 5, // '6'
		 4, // '7'
		 5, // '8'
		 5, // '9'
		 2, // ':'
		 3, // ';'
		 4, // '<'
		 5, // '='
		 4, // '>'
		 4, // '?'
		 8, // '@'
		 6, // 'A'
		 5, // 'B'
		 5, // 'C'
		 5, // 'D'
		 4, // 'E'
		 4, // 'F'
		 5, // 'G'
		 5, // 'H'
		 2, // 'I'
		 5, // 'J'
		 5, // 'K'
		 4, // 'L'
		 6, // 'M'
		 6, // 'N'
		 5, // 'O'
		 5, // 'P'
		 5, // 'Q'
		 5, // 'R'
		 4, // 'S'
		 4, // 'T'
		 5, // 'U'
		 6, // 'V'
		 8, // 'W'
		 6, // 'X'
		 6, // 'Y'
		 4, // 'Z'
		 3, // '['
		 4, // '\'
		 3, // ']'
		 6, // '^'
		 6, // '_'
		 3, // '`'
		 5, // 'a'
		 5, // 'b'
		 4, // 'c'
		 5, // 'd'
		 4, // 'e'
		 4, // 'f'
		 5, // 'g'
		 5, // 'h'
		 2, // 'i'
		 3, // 'j'
		 5, // 'k'
		 2, // 'l'
		 6, // 'm'
		 5, // 'n'
		 5, // 'o'
		 5, // 'p'
		 5, // 'q'
		 4, // 'r'
		 4, // 's'
		 3, // 't'
		 5, // 'u'
		 4, // 'v'
		 6, // 'w'
		 4, // 'x'
		 4, // 'y'
		 4, // 'z'
		 4, // '{'
		 2, // '|'
		 4, // '}'
		 7, // '~'
};
#endif


#if defined EN_SIX_DOT
const unsigned char six_dot_glyph_table[] = {
		// ' ' charwidth: 2
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		
		// '!' charwidth: 2
		0x80, 	//  [* ] 
		0x80, 	//  [* ] 
		0x80, 	//  [* ] 
		0x80, 	//  [* ] 
		0x00, 	//  [  ] 
		0x80, 	//  [* ] 
		0x00, 	//  [  ] 
		0x00, 	//  [  ] 
		
		// '"' charwidth: 4
		0xA0,  	//  [* * ] 
		0xA0,  	//  [* * ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// '#' charwidth: 8
		0x00,  	//  [        ] 
		0x14,  	//  [   * *  ] 
		0x7E,  	//  [ ****** ] 
		0x28,  	//  [  * *   ] 
		0xFC,  	//  [******  ] 
		0x50,  	//  [ * *    ] 
		0x00,  	//  [        ] 
		0x00,  	//  [        ] 
		
		// '$' charwidth: 5
		0x20,  	//  [  *  ] 
		0x70,  	//  [ *** ] 
		0x80,  	//  [*    ] 
		0x60,  	//  [ **  ] 
		0x10,  	//  [   * ] 
		0xE0,  	//  [***  ] 
		0x20,  	//  [  *  ] 
		0x00,  	//  [     ] 
		
		// '%' charwidth: 7
		0x64,  	//  [ **  * ] 
		0xA8,  	//  [* * *  ] 
		0xD0,  	//  [** *   ] 
		0x2C,  	//  [  * ** ] 
		0x54,  	//  [ * * * ] 
		0x98,  	//  [*  **  ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		
		// '&' charwidth: 6
		0x60,  	//  [ **   ] 
		0x90,  	//  [*  *  ] 
		0x40,  	//  [ *    ] 
		0x98,  	//  [*  ** ] 
		0x90,  	//  [*  *  ] 
		0x60,  	//  [ **   ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// ''' charwidth: 2
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// '(' charwidth: 4
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		
		// ')' charwidth: 4
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		0x20,  	//  [  * ] 
		0x20,  	//  [  * ] 
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		
		// '*' charwidth: 6
		0x50,  	//  [ * *  ] 
		0x20,  	//  [  *   ] 
		0xF8,  	//  [***** ] 
		0x20,  	//  [  *   ] 
		0x50,  	//  [ * *  ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// '+' charwidth: 6
		0x00, 	//  [      ] 
		0x20, 	//  [  *   ] 
		0x20, 	//  [  *   ] 
		0xF8, 	//  [***** ] 
		0x20, 	//  [  *   ] 
		0x20, 	//  [  *   ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		
		// ',' charwidth: 3
		0x00, 	//  [   ] 
		0x00, 	//  [   ] 
		0x00, 	//  [   ] 
		0x00, 	//  [   ] 
		0x00, 	//  [   ] 
		0x40, 	//  [ * ] 
		0x40, 	//  [ * ] 
		0x80, 	//  [*  ] 
		
		// '-' charwidth: 6
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0xF8,  	//  [***** ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// '.' charwidth: 2
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// '/' charwidth: 4
		0x20, 	//  [  * ] 
		0x20, 	//  [  * ] 
		0x40, 	//  [ *  ] 
		0x40, 	//  [ *  ] 
		0x80, 	//  [*   ] 
		0x80, 	//  [*   ] 
		0x00, 	//  [    ] 
		0x00, 	//  [    ] 
		
		// '0' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '1' charwidth: 3
		0x40,  	//  [ * ] 
		0xC0,  	//  [** ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		
		// '2' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x10,  	//  [   * ] 
		0x20,  	//  [  *  ] 
		0x40,  	//  [ *   ] 
		0xF0,  	//  [**** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '3' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x20,  	//  [  *  ] 
		0x10,  	//  [   * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '4' charwidth: 5
		0x20,  	//  [  *  ] 
		0x60,  	//  [ **  ] 
		0xA0,  	//  [* *  ] 
		0xF0,  	//  [**** ] 
		0x20,  	//  [  *  ] 
		0x20,  	//  [  *  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '5' charwidth: 5
		0xF0,  	//  [**** ] 
		0x80,  	//  [*    ] 
		0xE0,  	//  [***  ] 
		0x10,  	//  [   * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '6' charwidth: 5
		0x20,  	//  [  *  ] 
		0x40,  	//  [ *   ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '7' charwidth: 5
		0xF0, 	//  [**** ] 
		0x10, 	//  [   * ] 
		0x20, 	//  [  *  ] 
		0x40, 	//  [ *   ] 
		0x40, 	//  [ *   ] 
		0x40, 	//  [ *   ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		
		// '8' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '9' charwidth: 5
		0x60, 	//  [ **  ] 
		0x90, 	//  [*  * ] 
		0x90, 	//  [*  * ] 
		0x70, 	//  [ *** ] 
		0x20, 	//  [  *  ] 
		0x40, 	//  [ *   ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		
		// ':' charwidth: 2
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// ';' charwidth: 3
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x40,  	//  [ * ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x80,  	//  [*  ] 
		
		// '<' charwidth: 4
		0x00,  	//  [    ] 
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// '=' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0xF0,  	//  [**** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0xF0,  	//  [**** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '>' charwidth: 4
		0x00,  	//  [    ] 
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// '?' charwidth: 4
		0xC0,  	//  [**  ] 
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x00,  	//  [    ] 
		0x40,  	//  [ *  ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// '@' charwidth: 8
		0x3C,  	//  [  ****  ] 
		0x42,  	//  [ *    * ] 
		0x9A,  	//  [*  ** * ] 
		0xAA,  	//  [* * * * ] 
		0xB4,  	//  [* ** *  ] 
		0x40,  	//  [ *      ] 
		0x38,  	//  [  ***   ] 
		0x00,  	//  [        ] 
		
		// 'A' charwidth: 6
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x50,  	//  [ * *  ] 
		0x70,  	//  [ ***  ] 
		0x88,  	//  [*   * ] 
		0x88,  	//  [*   * ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'B' charwidth: 5
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'C' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'D' charwidth: 5
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'E' charwidth: 4
		0xE0,  	//  [*** ] 
		0x80,  	//  [*   ] 
		0xE0,  	//  [*** ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0xE0,  	//  [*** ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'F' charwidth: 4
		0xE0,  	//  [*** ] 
		0x80,  	//  [*   ] 
		0xE0,  	//  [*** ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'G' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x80,  	//  [*    ] 
		0xB0,  	//  [* ** ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'H' charwidth: 5
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xF0,  	//  [**** ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'I' charwidth: 2
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// 'J' charwidth: 5
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'K' charwidth: 5
		0x90,  	//  [*  * ] 
		0xA0,  	//  [* *  ] 
		0xC0,  	//  [**   ] 
		0xC0,  	//  [**   ] 
		0xA0,  	//  [* *  ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'L' charwidth: 4
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0xE0,  	//  [*** ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'M' charwidth: 6
		0x88,  	//  [*   * ] 
		0xD8,  	//  [** ** ] 
		0xA8,  	//  [* * * ] 
		0x88,  	//  [*   * ] 
		0x88,  	//  [*   * ] 
		0x88,  	//  [*   * ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'N' charwidth: 5
		0x90,  	//  [*  * ] 
		0xD0,  	//  [** * ] 
		0xD0,  	//  [** * ] 
		0xB0,  	//  [* ** ] 
		0xB0,  	//  [* ** ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'O' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'P' charwidth: 5
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'Q' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x10,  	//  [   * ] 
		0x00,  	//  [     ] 
		
		// 'R' charwidth: 5
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0xA0,  	//  [* *  ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'S' charwidth: 5
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x40,  	//  [ *   ] 
		0x20,  	//  [  *  ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'T' charwidth: 6
		0xF8,  	//  [***** ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'U' charwidth: 5
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'V' charwidth: 6
		0x88,  	//  [*   * ] 
		0x88,  	//  [*   * ] 
		0x50,  	//  [ * *  ] 
		0x50,  	//  [ * *  ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'W' charwidth: 10
		0x88, 0x80, 	//  [*   *   * ] 
		0x88, 0x80, 	//  [*   *   * ] 
		0x55, 0x00, 	//  [ * * * *  ] 
		0x55, 0x00, 	//  [ * * * *  ] 
		0x22, 0x00, 	//  [  *   *   ] 
		0x22, 0x00, 	//  [  *   *   ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		
		// 'X' charwidth: 6
		0x88,  	//  [*   * ] 
		0x50,  	//  [ * *  ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x50,  	//  [ * *  ] 
		0x88,  	//  [*   * ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'Y' charwidth: 6
		0x88,  	//  [*   * ] 
		0x50,  	//  [ * *  ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x20,  	//  [  *   ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'Z' charwidth: 5
		0xF0, 	//  [**** ] 
		0x10, 	//  [   * ] 
		0x20, 	//  [  *  ] 
		0x40, 	//  [ *   ] 
		0x80, 	//  [*    ] 
		0xF0, 	//  [**** ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		
		// '[' charwidth: 3
		0xC0,  	//  [** ] 
		0x80,  	//  [*  ] 
		0x80,  	//  [*  ] 
		0x80,  	//  [*  ] 
		0x80,  	//  [*  ] 
		0x80,  	//  [*  ] 
		0xC0,  	//  [** ] 
		0x00,  	//  [   ] 
		
		// '\' charwidth: 5
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0x40,  	//  [ *   ] 
		0x40,  	//  [ *   ] 
		0x20,  	//  [  *  ] 
		0x20,  	//  [  *  ] 
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		
		// ']' charwidth: 3
		0xC0,  	//  [** ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0xC0,  	//  [** ] 
		0x00,  	//  [   ] 
		
		// '^' charwidth: 4
		0x40,  	//  [ *  ] 
		0xA0,  	//  [* * ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// '_' charwidth: 7
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0xFC,  	//  [****** ] 
		0x00,  	//  [       ] 
		
		// '`' charwidth: 3
		0x80,  	//  [*  ] 
		0x40,  	//  [ * ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		
		// 'a' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x70,  	//  [ *** ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x70,  	//  [ *** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'b' charwidth: 5
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'c' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x60,  	//  [ ** ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x60,  	//  [ ** ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'd' charwidth: 5
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		0x70,  	//  [ *** ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x70,  	//  [ *** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'e' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x60,  	//  [ **  ] 
		0xB0,  	//  [* ** ] 
		0x80,  	//  [*    ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'f' charwidth: 4
		0x60,  	//  [ ** ] 
		0x80,  	//  [*   ] 
		0xC0,  	//  [**  ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'g' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x70,  	//  [ *** ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x70,  	//  [ *** ] 
		0x10,  	//  [   * ] 
		0x60,  	//  [ **  ] 
		
		// 'h' charwidth: 5
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'i' charwidth: 2
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// 'j' charwidth: 3
		0x40,  	//  [ * ] 
		0x00,  	//  [   ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x40,  	//  [ * ] 
		0x80,  	//  [*  ] 
		
		// 'k' charwidth: 5
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0xA0,  	//  [* *  ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'l' charwidth: 2
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x00,  	//  [  ] 
		0x00,  	//  [  ] 
		
		// 'm' charwidth: 6
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0xF0,  	//  [****  ] 
		0xA8,  	//  [* * * ] 
		0xA8,  	//  [* * * ] 
		0xA8,  	//  [* * * ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'n' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'o' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x60,  	//  [ **  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x60,  	//  [ **  ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'p' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0xE0,  	//  [***  ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0xE0,  	//  [***  ] 
		0x80,  	//  [*    ] 
		0x80,  	//  [*    ] 
		
		// 'q' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x70,  	//  [ *** ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x70,  	//  [ *** ] 
		0x10,  	//  [   * ] 
		0x10,  	//  [   * ] 
		
		// 'r' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0xA0,  	//  [* * ] 
		0xC0,  	//  [**  ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 's' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0x60,  	//  [ ** ] 
		0xC0,  	//  [**  ] 
		0x20,  	//  [  * ] 
		0xC0,  	//  [**  ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 't' charwidth: 3
		0x00,  	//  [   ] 
		0x80,  	//  [*  ] 
		0xC0,  	//  [** ] 
		0x80,  	//  [*  ] 
		0x80,  	//  [*  ] 
		0x40,  	//  [ * ] 
		0x00,  	//  [   ] 
		0x00,  	//  [   ] 
		
		// 'u' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x90,  	//  [*  * ] 
		0x70,  	//  [ *** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// 'v' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0xA0,  	//  [* * ] 
		0xA0,  	//  [* * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'w' charwidth: 6
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		0xA8,  	//  [* * * ] 
		0xA8,  	//  [* * * ] 
		0x50,  	//  [ * *  ] 
		0x50,  	//  [ * *  ] 
		0x00,  	//  [      ] 
		0x00,  	//  [      ] 
		
		// 'x' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0xA0,  	//  [* * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0xA0,  	//  [* * ] 
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		
		// 'y' charwidth: 4
		0x00,  	//  [    ] 
		0x00,  	//  [    ] 
		0xA0,  	//  [* * ] 
		0xA0,  	//  [* * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		
		// 'z' charwidth: 5
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		0xF0,  	//  [**** ] 
		0x20,  	//  [  *  ] 
		0x40,  	//  [ *   ] 
		0xF0,  	//  [**** ] 
		0x00,  	//  [     ] 
		0x00,  	//  [     ] 
		
		// '{' charwidth: 4
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		
		// '|' charwidth: 2
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		0x80,  	//  [* ] 
		
		// '}' charwidth: 4
		0x80,  	//  [*   ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x20,  	//  [  * ] 
		0x20,  	//  [  * ] 
		0x40,  	//  [ *  ] 
		0x40,  	//  [ *  ] 
		0x80,  	//  [*   ] 
		
		// '~' charwidth: 7
		0x64,  	//  [ **  * ] 
		0x98,  	//  [*  **  ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
		0x00,  	//  [       ] 
};


const unsigned short six_dot_width_table[] = {                           // {2}
		 2,            // ' '
		 2,            // '!'
		 4,            // '"'
		 8,            // '#'
		 5,            // '$'
		 7,            // '%'
		 6,            // '&'
		 2,            // '''
		 4,            // '('
		 4,            // ')'
		 6,            // '*'
		 6,            // '+'
		 3,            // ','
		 6,            // '-'
		 2,            // '.'
		 4,            // '/'
		 5,            // '0'
		 3,            // '1'
		 5,            // '2'
		 5,            // '3'
		 5,            // '4'
		 5,            // '5'
		 5,            // '6'
		 5,            // '7'
		 5,            // '8'
		 5,            // '9'
		 2,            // ':'
		 3,            // ';'
		 4,            // '<'
		 5,            // '='
		 4,            // '>'
		 4,            // '?'
		 8,            // '@'
		 6,            // 'A'
		 5,            // 'B'
		 5,            // 'C'
		 5,            // 'D'
		 4,            // 'E'
		 4,            // 'F'
		 5,            // 'G'
		 5,            // 'H'
		 2,            // 'I'
		 5,            // 'J'
		 5,            // 'K'
		 4,            // 'L'
		 6,            // 'M'
		 5,            // 'N'
		 5,            // 'O'
		 5,            // 'P'
		 5,            // 'Q'
		 5,            // 'R'
		 5,            // 'S'
		 6,            // 'T'
		 5,            // 'U'
		 6,            // 'V'
		10,            // 'W'
		 6 + (1 << 8), // 'X'
		 6 + (1 << 8), // 'Y'
		 5 + (1 << 8), // 'Z'
		 3 + (1 << 8), // '['
		 5 + (1 << 8), // '\'
		 3 + (1 << 8), // ']'
		 4 + (1 << 8), // '^'
		 7 + (1 << 8), // '_'
		 3 + (1 << 8), // '`'
		 5 + (1 << 8), // 'a'
		 5 + (1 << 8), // 'b'
		 4 + (1 << 8), // 'c'
		 5 + (1 << 8), // 'd'
		 5 + (1 << 8), // 'e'
		 4 + (1 << 8), // 'f'
		 5 + (1 << 8), // 'g'
		 5 + (1 << 8), // 'h'
		 2 + (1 << 8), // 'i'
		 3 + (1 << 8), // 'j'
		 5 + (1 << 8), // 'k'
		 2 + (1 << 8), // 'l'
		 6 + (1 << 8), // 'm'
		 5 + (1 << 8), // 'n'
		 5 + (1 << 8), // 'o'
		 5 + (1 << 8), // 'p'
		 5 + (1 << 8), // 'q'
		 4 + (1 << 8), // 'r'
		 4 + (1 << 8), // 's'
		 3 + (1 << 8), // 't'
		 5 + (1 << 8), // 'u'
		 4 + (1 << 8), // 'v'
		 6 + (1 << 8), // 'w'
		 4 + (1 << 8), // 'x'
		 4 + (1 << 8), // 'y'
		 5 + (1 << 8), // 'z'
		 4 + (1 << 8), // '{'
		 2 + (1 << 8), // '|'
		 4 + (1 << 8), // '}'
		 7 + (1 << 8), // '~'
};
#endif

#if defined EN_SEVEN_DOT
const unsigned char seven_dot_glyph_table[] = {
		// ' ' charwidth: 5 
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		
		// '!' charwidth: 4 
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		
		// '"' charwidth: 6 
		0x48, 	//  [ *  * ]  
		0x48, 	//  [ *  * ]  
		0x00, 	//  [      ]  
		0x00, 	//  [      ]  
		0x00, 	//  [      ]  
		0x00, 	//  [      ]  
		0x00, 	//  [      ]  
		0x00, 	//  [      ]  
		
		// '#' charwidth: 7 
		0x28, 	//  [  * *  ]  
		0x7C, 	//  [ ***** ]  
		0x7C, 	//  [ ***** ]  
		0x28, 	//  [  * *  ]  
		0x7C, 	//  [ ***** ]  
		0x7C, 	//  [ ***** ]  
		0x28, 	//  [  * *  ]  
		0x00, 	//  [       ]  
		
		// '$' charwidth: 7 
		0x3C, 	//  [  **** ]  
		0x68, 	//  [ ** *  ]  
		0x68, 	//  [ ** *  ]  
		0x38, 	//  [  ***  ]  
		0x2C, 	//  [  * ** ]  
		0x2C, 	//  [  * ** ]  
		0x78, 	//  [ ****  ]  
		0x28, 	//  [  * *  ]  
		
		// '%' charwidth: 8 
		0x64,  	//  [ **  *  ]  
		0x6C, 	//  [ ** **  ]  
		0x08, 	//  [    *   ]  
		0x18, 	//  [   **   ]  
		0x10, 	//  [   *    ]  
		0x36, 	//  [  ** ** ]  
		0x26, 	//  [  *  ** ]  
		0x00, 	//  [        ]  
		
		// '&' charwidth: 9 
		0x3C, 0x00, 	//  [  ****   ]  
		0x66, 0x00, 	//  [ **  **  ]  
		0x3C, 0x00, 	//  [  ****   ]  
		0x7D, 0x00, 	//  [ ***** * ]  
		0x67, 0x00, 	//  [ **  *** ]  
		0x67, 0x00, 	//  [ **  *** ]  
		0x3D, 0x00, 	//  [  **** * ]  
		0x00, 0x00, 	//  [         ]  
		
		// ''' charwidth: 3 
		0x40, 	//  [ * ]  
		0x40, 	//  [ * ]  
		0x00, 	//  [   ]  
		0x00, 	//  [   ]  
		0x00, 	//  [   ]  
		0x00, 	//  [   ]  
		0x00, 	//  [   ]  
		0x00, 	//  [   ]  
		
		// '(' charwidth: 5 
		0x10, 	//  [   * ]  
		0x20, 	//  [  *  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x20, 	//  [  *  ]  
		0x10, 	//  [   * ]  
		
		// ')' charwidth: 5 
		0x40,	//  [ *   ]  
		0x20,	//  [  *  ]  
		0x30,	//  [  ** ]  
		0x30,	//  [  ** ]  
		0x30,	//  [  ** ]  
		0x30,	//  [  ** ]  
		0x20,	//  [  *  ]  
		0x40,	//  [ *   ]  
		
		// '*' charwidth: 7 
		0x10, 	//  [   *   ]  
		0x54, 	//  [ * * * ]  
		0x7C, 	//  [ ***** ]  
		0x38, 	//  [  ***  ]  
		0x7C, 	//  [ ***** ]  
		0x54, 	//  [ * * * ]  
		0x10, 	//  [   *   ]  
		0x00, 	//  [       ]  
		
		// '+' charwidth: 7 
		0x00, 	//  [       ]  
		0x10, 	//  [   *   ]  
		0x10, 	//  [   *   ]  
		0x7C, 	//  [ ***** ]  
		0x10, 	//  [   *   ]  
		0x10, 	//  [   *   ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		
		// ',' charwidth: 4 
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x20, 	//  [  * ]  
		0x60, 	//  [ ** ]  
		0x40, 	//  [ *  ]  
		
		// '-' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		
		// '.' charwidth: 4 
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		
		// '/' charwidth: 5 
		0x10, 	//  [   * ]  
		0x10, 	//  [   * ]  
		0x30, 	//  [  ** ]  
		0x20, 	//  [  *  ]  
		0x60, 	//  [ **  ]  
		0x40, 	//  [ *   ]  
		0x40, 	//  [ *   ]  
		0x00, 	//  [     ]  
		
		// '0' charwidth: 7 
		0x38, 	//  [  ***  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// '1' charwidth: 5 
		0x30, 	//  [  ** ]  
		0x70, 	//  [ *** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x00, 	//  [     ]  
		
		// '2' charwidth: 7 
		0x78, 	//  [ ****  ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x38, 	//  [  ***  ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		
		// '3' charwidth: 7 
		0x78, 	//  [ ****  ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x38, 	//  [  ***  ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x78, 	//  [ ****  ]  
		0x00, 	//  [       ]  
		
		// '4' charwidth: 7 
		0x0C, 	//  [    ** ]  
		0x1C, 	//  [   *** ]  
		0x2C, 	//  [  * ** ]  
		0x4C, 	//  [ *  ** ]  
		0x7C, 	//  [ ***** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x00, 	//  [       ]  
		
		// '5' charwidth: 7 
		0x78, 	//  [ ****  ]  
		0x40, 	//  [ *     ]  
		0x78, 	//  [ ****  ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x78, 	//  [ ****  ]  
		0x00, 	//  [       ]  
		
		// '6' charwidth: 7 
		0x38, 	//  [  ***  ]  
		0x60, 	//  [ **    ]  
		0x78, 	//  [ ****  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// '7' charwidth: 7 
		0x7C, 	//  [ ***** ]  
		0x0C, 	//  [    ** ]  
		0x18, 	//  [   **  ]  
		0x18, 	//  [   **  ]  
		0x30, 	//  [  **   ]  
		0x30, 	//  [  **   ]  
		0x30, 	//  [  **   ]  
		0x00, 	//  [       ]  
		
		// '8' charwidth: 7 
		0x38, 	//  [  ***  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x38, 	//  [  ***  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// '9' charwidth: 7 
		0x38, 	//  [  ***  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x3C, 	//  [  **** ]  
		0x0C, 	//  [    ** ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// ':' charwidth: 4 
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		
		// ';' charwidth: 4 
		0x00, 	//  [    ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		0x20, 	//  [  * ]  
		0x60, 	//  [ ** ]  
		0x40, 	//  [ *  ]  
		
		// '<' charwidth: 6 
		0x08, 	//  [    * ]  
		0x18, 	//  [   ** ]  
		0x30, 	//  [  **  ]  
		0x60, 	//  [ **   ]  
		0x30, 	//  [  **  ]  
		0x18, 	//  [   ** ]  
		0x08, 	//  [    * ]  
		0x00, 	//  [      ]  
		
		// '=' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		
		// '>' charwidth: 6 
		0x40, 	//  [ *    ]  
		0x60, 	//  [ **   ]  
		0x30, 	//  [  **  ]  
		0x18, 	//  [   ** ]  
		0x30, 	//  [  **  ]  
		0x60, 	//  [ **   ]  
		0x40, 	//  [ *    ]  
		0x00, 	//  [      ]  
		
		// '?' charwidth: 7 
		0x78, 	//  [ ****  ]  
		0x0C, 	//  [    ** ]  
		0x18, 	//  [   **  ]  
		0x30, 	//  [  **   ]  
		0x30, 	//  [  **   ]  
		0x00, 	//  [       ]  
		0x30, 	//  [  **   ]  
		0x00, 	//  [       ]  
		
		// '@' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x6E, 	//  [ ** *** ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		
		// 'A' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		
		// 'B' charwidth: 8 
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x00, 	//  [        ]  
		
		// 'C' charwidth: 7 
		0x3C, 	//  [  **** ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x3C, 	//  [  **** ]  
		0x00, 	//  [       ]  
		
		// 'D' charwidth: 8 
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x00, 	//  [        ]  
		
		// 'E' charwidth: 7 
		0x7C, 	//  [ ***** ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		
		// 'F' charwidth: 7 
		0x7C, 	//  [ ***** ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x00, 	//  [       ]  
		
		// 'G' charwidth: 8 
		0x3E, 	//  [  ***** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		
		// 'H' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		
		// 'I' charwidth: 4 
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		
		// 'J' charwidth: 7 
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x0C, 	//  [    ** ]  
		0x78, 	//  [ ****  ]  
		0x00, 	//  [       ]  
		
		// 'K' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x70, 	//  [ ***    ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		
		// 'L' charwidth: 7 
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		
		// 'M' charwidth: 9 
		0x41, 0x00, 	//  [ *     * ]  
		0x63, 0x00, 	//  [ **   ** ]  
		0x77, 0x00, 	//  [ *** *** ]  
		0x7F, 0x00, 	//  [ ******* ]  
		0x6B, 0x00, 	//  [ ** * ** ]  
		0x63, 0x00, 	//  [ **   ** ]  
		0x63, 0x00, 	//  [ **   ** ]  
		0x00, 0x00, 	//  [         ]  
		
		// 'N' charwidth: 8 
		0x46, 	//  [ *   ** ]  
		0x66, 	//  [ **  ** ]  
		0x76, 	//  [ *** ** ]  
		0x7E, 	//  [ ****** ]  
		0x6E, 	//  [ ** *** ]  
		0x66, 	//  [ **  ** ]  
		0x62, 	//  [ **   * ]  
		0x00, 	//  [        ]  
		
		// 'O' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'P' charwidth: 8 
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		
		// 'Q' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x6E, 	//  [ ** *** ]  
		0x3C, 	//  [  ****  ]  
		0x06, 	//  [     ** ]  
		
		// 'R' charwidth: 8 
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		
		// 'S' charwidth: 6 
		0x38, 	//  [  *** ]  
		0x60, 	//  [ **   ]  
		0x60, 	//  [ **   ]  
		0x30, 	//  [  **  ]  
		0x18, 	//  [   ** ]  
		0x18, 	//  [   ** ]  
		0x70, 	//  [ ***  ]  
		0x00, 	//  [      ]  
		
		// 'T' charwidth: 8 
		0x7E, 	//  [ ****** ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		
		// 'U' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'V' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		
		// 'W' charwidth: 9 
		0x63, 0x00, 	//  [ **   ** ]  
		0x63, 0x00, 	//  [ **   ** ]  
		0x63, 0x00, 	//  [ **   ** ]  
		0x6B, 0x00, 	//  [ ** * ** ]  
		0x3E, 0x00, 	//  [  *****  ]  
		0x36, 0x00, 	//  [  ** **  ]  
		0x36, 0x00, 	//  [  ** **  ]  
		0x00, 0x00, 	//  [         ]  
		
		// 'X' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		
		// 'Y' charwidth: 8 
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		
		// 'Z' charwidth: 7 
		0x7C, 	//  [ ***** ]  
		0x0C, 	//  [    ** ]  
		0x1C, 	//  [   *** ]  
		0x38, 	//  [  ***  ]  
		0x70, 	//  [ ***   ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		
		// '[' charwidth: 5 
		0x70, 	//  [ *** ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x70, 	//  [ *** ]  
		
		// '\' charwidth: 7 
		0x40, 	//  [ *     ]  
		0x40, 	//  [ *     ]  
		0x60, 	//  [ **    ]  
		0x20, 	//  [  *    ]  
		0x30, 	//  [  **   ]  
		0x10, 	//  [   *   ]  
		0x10, 	//  [   *   ]  
		0x00, 	//  [       ]  
		
		// ']' charwidth: 5 
		0x70, 	//  [ *** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x30, 	//  [  ** ]  
		0x70, 	//  [ *** ]  
		
		// '^' charwidth: 7 
		0x20, 	//  [  *    ]  
		0x50, 	//  [ * *   ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		
		// '_' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0xF8, 	//  [*****   ]  
		
		// '`' charwidth: 5 
		0x20, 	//  [  *  ]  
		0x60, 	//  [ **  ]  
		0x40, 	//  [ *   ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		0x00, 	//  [     ]  
		
		// 'a' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x38, 	//  [  ***   ]  
		0x0C, 	//  [    **  ]  
		0x3C, 	//  [  ****  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'b' charwidth: 8 
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x00, 	//  [        ]  
		
		// 'c' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x38, 	//  [  ***  ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// 'd' charwidth: 8 
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x3C, 	//  [  ****  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'e' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x38, 	//  [  ***   ]  
		0x6C, 	//  [ ** **  ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'f' charwidth: 5 
		0x30, 	//  [  ** ]  
		0x60, 	//  [ **  ]  
		0x70, 	//  [ *** ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x00, 	//  [     ]  
		
		// 'g' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x38, 	//  [  ***   ]  
		
		// 'h' charwidth: 8 
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x00, 	//  [        ]  
		
		// 'i' charwidth: 4 
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x60, 	//  [ ** ]  
		0x00, 	//  [    ]  
		
		// 'j' charwidth: 4 
		0x30, 	//  [  **]  
		0x00, 	//  [    ]  
		0x30, 	//  [  **]  
		0x30, 	//  [  **]  
		0x30, 	//  [  **]  
		0x30, 	//  [  **]  
		0x30, 	//  [  **]  
		0x60, 	//  [ ** ]  
		
		// 'k' charwidth: 7 
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x60, 	//  [ **    ]  
		0x6C, 	//  [ ** ** ]  
		0x78, 	//  [ ****  ]  
		0x78, 	//  [ ****  ]  
		0x6C, 	//  [ ** ** ]  
		0x00, 	//  [       ]  
		
		// 'l' charwidth: 5 
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x00, 	//  [     ]  
		
		// 'm' charwidth: 9 
		0x00, 0x00, 	//  [         ]  
		0x00, 0x00, 	//  [         ]  
		0x7F, 0x00, 	//  [ ******* ]  
		0x6D, 0x80, 	//  [ ** ** **]  
		0x6D, 0x80, 	//  [ ** ** **]  
		0x6D, 0x80, 	//  [ ** ** **]  
		0x6D, 0x80, 	//  [ ** ** **]  
		0x00, 0x00, 	//  [         ]  
		
		// 'n' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x00, 	//  [        ]  
		
		// 'o' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x38, 	//  [  ***  ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x6C, 	//  [ ** ** ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// 'p' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		
		// 'q' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		
		// 'r' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x68, 	//  [ ** *   ]  
		0x78, 	//  [ ****   ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		
		// 's' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x38, 	//  [  ***  ]  
		0x60, 	//  [ **    ]  
		0x30, 	//  [  **   ]  
		0x18, 	//  [   **  ]  
		0x70, 	//  [ ***   ]  
		0x00, 	//  [       ]  
		
		// 't' charwidth: 5 
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x70, 	//  [ *** ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x60, 	//  [ **  ]  
		0x30, 	//  [  ** ]  
		0x00, 	//  [     ]  
		
		// 'u' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		
		// 'v' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x38, 	//  [  ***   ]  
		0x38, 	//  [  ***   ]  
		0x10, 	//  [   *    ]  
		0x00, 	//  [        ]  
		
		// 'w' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x63, 	//  [ **   **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x3E, 	//  [  ***** ]  
		0x36, 	//  [  ** ** ]  
		0x00, 	//  [        ]  
		
		// 'x' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x6C, 	//  [ ** **  ]  
		0x7C, 	//  [ *****  ]  
		0x38, 	//  [  ***   ]  
		0x7C, 	//  [ *****  ]  
		0x6C, 	//  [ ** **  ]  
		0x00, 	//  [        ]  
		
		// 'y' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x38, 	//  [  ***   ]  
		
		// 'z' charwidth: 7 
		0x00, 	//  [       ]  
		0x00, 	//  [       ]  
		0x7C, 	//  [ ***** ]  
		0x0C, 	//  [    ** ]  
		0x38, 	//  [  ***  ]  
		0x60, 	//  [ **    ]  
		0x7C, 	//  [ ***** ]  
		0x00, 	//  [       ]  
		
		// '{' charwidth: 7 
		0x38, 	//  [  ***  ]  
		0x20, 	//  [  *    ]  
		0x20, 	//  [  *    ]  
		0x40, 	//  [ *     ]  
		0x20, 	//  [  *    ]  
		0x20, 	//  [  *    ]  
		0x38, 	//  [  ***  ]  
		0x00, 	//  [       ]  
		
		// '|' charwidth: 4 
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		0x40, 	//  [ *  ]  
		
		// '}' charwidth: 7 
		0x70, 	//  [ ***   ]  
		0x10, 	//  [   *   ]  
		0x10, 	//  [   *   ]  
		0x08, 	//  [    *  ]  
		0x10, 	//  [   *   ]  
		0x10, 	//  [   *   ]  
		0x70, 	//  [ ***   ]  
		0x00, 	//  [       ]  
		
		// '~' charwidth: 8 
		0x28, 	//  [  * *   ]  
		0x50, 	//  [ * *    ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]

    #if USER_SPECIAL_SEVEN_DOT > 0                                       // {4}
        #define USER_SPECIAL_FONT_SEVEN_DOT
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_SEVEN_DOT
    #endif
};

const unsigned short seven_dot_width_table[] = {                         // {2}
		 5,            // ' ' 
		 4,            // '!' 
		 6,            // '"' 
		 7,            // '#' 
		 7,            // '$' 
		 8,            // '%' 
		 9,            // '&' 
		 3 + (1 << 8), // ''' 
		 5 + (1 << 8), // '(' 
		 5 + (1 << 8), // ')' 
		 7 + (1 << 8), // '*' 
		 7 + (1 << 8), // '+' 
		 4 + (1 << 8), // ','
		 7 + (1 << 8), // '-' 
		 4 + (1 << 8), // '.' 
		 5 + (1 << 8), // '/' 
		 7 + (1 << 8), // '0' 
		 5 + (1 << 8), // '1' 
		 7 + (1 << 8), // '2' 
		 7 + (1 << 8), // '3' 
		 7 + (1 << 8), // '4' 
		 7 + (1 << 8), // '5' 
		 7 + (1 << 8), // '6' 
		 7 + (1 << 8), // '7' 
		 7 + (1 << 8), // '8' 
		 7 + (1 << 8), // '9' 
		 4 + (1 << 8), // ':' 
		 4 + (1 << 8), // ';' 
		 6 + (1 << 8), // '<' 
		 7 + (1 << 8), // '=' 
		 6 + (1 << 8), // '>' 
		 7 + (1 << 8), // '?' 
		 8 + (1 << 8), // '@' 
		 8 + (1 << 8), // 'A' 
		 8 + (1 << 8), // 'B' 
		 7 + (1 << 8), // 'C' 
		 8 + (1 << 8), // 'D' 
		 7 + (1 << 8), // 'E' 
		 7 + (1 << 8), // 'F' 
		 8 + (1 << 8), // 'G' 
		 8 + (1 << 8), // 'H' 
		 4 + (1 << 8), // 'I' 
		 7 + (1 << 8), // 'J' 
		 8 + (1 << 8), // 'K' 
		 7 + (1 << 8), // 'L' 
		 9 + (1 << 8), // 'M' 
		 8 + (2 << 8), // 'N' 
		 8 + (2 << 8), // 'O' 
		 8 + (2 << 8), // 'P' 
		 8 + (2 << 8), // 'Q' 
		 8 + (2 << 8), // 'R' 
		 6 + (2 << 8), // 'S' 
		 8 + (2 << 8), // 'T' 
		 8 + (2 << 8), // 'U' 
		 8 + (2 << 8), // 'V' 
		 9 + (2 << 8), // 'W' 
		 8 + (3 << 8), // 'X' 
		 8 + (3 << 8), // 'Y' 
		 7 + (3 << 8), // 'Z' 
		 5 + (3 << 8), // '[' 
		 7 + (3 << 8), // '\' 
		 5 + (3 << 8), // ']' 
		 7 + (3 << 8), // '^' 
		 8 + (3 << 8), // '_' 
		 5 + (3 << 8), // '`' 
		 8 + (3 << 8), // 'a' 
		 8 + (3 << 8), // 'b' 
		 7 + (3 << 8), // 'c' 
		 8 + (3 << 8), // 'd' 
		 8 + (3 << 8), // 'e' 
		 5 + (3 << 8), // 'f' 
		 8 + (3 << 8), // 'g' 
		 8 + (3 << 8), // 'h' 
		 4 + (3 << 8), // 'i' 
		 4 + (3 << 8), // 'j' 
		 7 + (3 << 8), // 'k' 
		 5 + (3 << 8), // 'l' 
		 9 + (3 << 8), // 'm' 
		 8 + (4 << 8), // 'n' 
		 7 + (4 << 8), // 'o' 
		 8 + (4 << 8), // 'p' 
		 8 + (4 << 8), // 'q' 
		 8 + (4 << 8), // 'r' 
		 7 + (4 << 8), // 's' 
		 5 + (4 << 8), // 't' 
		 8 + (4 << 8), // 'u' 
		 8 + (4 << 8), // 'v' 
		 8 + (4 << 8), // 'w' 
		 8 + (4 << 8), // 'x' 
		 8 + (4 << 8), // 'y' 
		 7 + (4 << 8), // 'z' 
		 7 + (4 << 8), // '{' 
		 4 + (4 << 8), // '|' 
		 7 + (4 << 8), // '}' 
		 8 + (4 << 8), // '~'
    #if USER_SPECIAL_SEVEN_DOT > 0                                       // {4}
        #define USER_SPECIAL_FONT_SEVEN_DOT_WIDTH
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_SEVEN_DOT_WIDTH
    #endif
};
#endif

#if defined EN_NINE_DOT
const unsigned char nine_dot_glyph_table[] = {
		// ' ' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '!' charwidth: 8 
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x3C, 	//  [  ****  ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '"' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '#' charwidth: 8 
		0x00, 	//  [        ]  
		0x36, 	//  [  ** ** ]  
		0x36, 	//  [  ** ** ]  
		0x7F, 	//  [ *******]  
		0x36, 	//  [  ** ** ]  
		0x36, 	//  [  ** ** ]  
		0x36, 	//  [  ** ** ]  
		0x7F, 	//  [ *******]  
		0x36, 	//  [  ** ** ]  
		0x36, 	//  [  ** ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '$' charwidth: 8 
		0x08, 	//  [    *   ]  
		0x3E, 	//  [  ***** ]  
		0x6B, 	//  [ ** * **]  
		0x68, 	//  [ ** *   ]  
		0x68, 	//  [ ** *   ]  
		0x3E, 	//  [  ***** ]  
		0x0B, 	//  [    * **]  
		0x0B, 	//  [    * **]  
		0x6B, 	//  [ ** * **]  
		0x3E, 	//  [  ***** ]  
		0x08, 	//  [    *   ]  
		0x08, 	//  [        ]  
		
		// '%' charwidth: 8 
		0x70, 	//  [ ***    ]  
		0xD8, 	//  [** **   ]  
		0xDA, 	//  [** ** * ]  
		0x76, 	//  [ *** ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x6E, 	//  [ ** *** ]  
		0x5B, 	//  [ * ** **]  
		0x1B, 	//  [   ** **]  
		0x0E, 	//  [    *** ]  
		0x00, 	//  [        ]  
		
		// '&' charwidth: 8 
		0x00, 	//  [        ]  
		0x38, 	//  [  ***   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x38, 	//  [  ***   ]  
		0x6C, 	//  [ ** **  ]  
		0xC7, 	//  [**   ***]  
		0xC6, 	//  [**   ** ]  
		0x6E, 	//  [ ** *** ]  
		0x3B, 	//  [  *** **]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// ''' charwidth: 8 
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '(' charwidth: 8 
		0x00, 	//  [        ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		
		// ')' charwidth: 8 
		0x00, 	//  [        ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		
		// '*' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x36, 	//  [  ** ** ]  
		0x1C, 	//  [   ***  ]  
		0x7F, 	//  [ *******]  
		0x1C, 	//  [   ***  ]  
		0x36, 	//  [  ** ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '+' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x7E, 	//  [ ****** ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// ',' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x00, 	//  [        ]  
		
		// '-' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '.' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '/' charwidth: 8 
		0x00, 	//  [        ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		
		// '0' charwidth: 8 
		0x00, 	//  [        ]  
		0x1C, 	//  [   ***  ]  
		0x36, 	//  [  ** ** ]  
		0x63, 	//  [ **   **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x63, 	//  [ **   **]  
		0x36, 	//  [  ** ** ]  
		0x1C, 	//  [   ***  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '1' charwidth: 8 
		0x00, 	//  [        ]  
		0x0C, 	//  [    **  ]  
		0x1C, 	//  [   ***  ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x3F, 	//  [  ******]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '2' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '3' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x06, 	//  [     ** ]  
		0x1C, 	//  [   ***  ]  
		0x06, 	//  [     ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '4' charwidth: 8 
		0x00, 	//  [        ]  
		0x06, 	//  [     ** ]  
		0x0E, 	//  [    *** ]  
		0x1E, 	//  [   **** ]  
		0x36, 	//  [  ** ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7F, 	//  [ *******]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '5' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '6' charwidth: 8 
		0x00, 	//  [        ]  
		0x1C, 	//  [   ***  ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '7' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '8' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '9' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x38, 	//  [  ***   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// ':' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// ';' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x00, 	//  [        ]  
		
		// '<' charwidth: 8 
		0x00, 	//  [        ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x06, 	//  [     ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '=' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '>' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '?' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '@' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0xC3, 	//  [**    **]  
		0xC3, 	//  [**    **]  
		0xCF, 	//  [**  ****]  
		0xDB, 	//  [** ** **]  
		0xDB, 	//  [** ** **]  
		0xCF, 	//  [**  ****]  
		0xC0, 	//  [**      ]  
		0x7F, 	//  [ *******]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'A' charwidth: 8 
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'B' charwidth: 8 
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'C' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'D' charwidth: 8 
		0x00, 	//  [        ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'E' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'F' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'G' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x6E, 	//  [ ** *** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'H' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'I' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'J' charwidth: 8 
		0x00, 	//  [        ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'K' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x6C, 	//  [ ** **  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'L' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'M' charwidth: 8 
		0x00, 	//  [        ]  
		0x63, 	//  [ **   **]  
		0x77, 	//  [ *** ***]  
		0x7F, 	//  [ *******]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'N' charwidth: 8 
		0x00, 	//  [        ]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x73, 	//  [ ***  **]  
		0x7B, 	//  [ **** **]  
		0x6F, 	//  [ ** ****]  
		0x67, 	//  [ **  ***]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'O' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'P' charwidth: 8 
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'Q' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x06, 	//  [     ** ]  
		
		// 'R' charwidth: 8 
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x6C, 	//  [ ** **  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'S' charwidth: 8 
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x60, 	//  [ **     ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x06, 	//  [     ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'T' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'U' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'V' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'W' charwidth: 8 
		0x00, 	//  [        ]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x3E, 	//  [  ***** ]  
		0x36, 	//  [  ** ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'X' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'Y' charwidth: 8 
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'Z' charwidth: 8 
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '[' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x3C, 	//  [  ****  ]  
		
		// '\' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x00, 	//  [        ]  
		
		// ']' charwidth: 8 
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x3C, 	//  [  ****  ]  
		
		// '^' charwidth: 8 
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '_' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0xFF, 	//  [********]  
		
		// '`' charwidth: 8 
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'a' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x3E, 	//  [  ***** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'b' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'c' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'd' charwidth: 8 
		0x00, 	//  [        ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x3E, 	//  [  ***** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'e' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7E, 	//  [ ****** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'f' charwidth: 8 
		0x00, 	//  [        ]  
		0x1C, 	//  [   ***  ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x78, 	//  [ ****   ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'g' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3E, 	//  [  ***** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x06, 	//  [     ** ]  
		0x7C, 	//  [ *****  ]  
		
		// 'h' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'i' charwidth: 8 
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x78, 	//  [ ****   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'j' charwidth: 8 
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x0C, 	//  [    **  ]  
		0x78, 	//  [ ****   ]  
		
		// 'k' charwidth: 8 
		0x00, 	//  [        ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x6C, 	//  [ ** **  ]  
		0x78, 	//  [ ****   ]  
		0x6C, 	//  [ ** **  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'l' charwidth: 8 
		0x00, 	//  [        ]  
		0x78, 	//  [ ****   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'm' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x63, 	//  [ **   **]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'n' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'o' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'p' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x7C, 	//  [ *****  ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		
		// 'q' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3E, 	//  [  ***** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		
		// 'r' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7C, 	//  [ *****  ]  
		0x66, 	//  [ **  ** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 's' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x3E, 	//  [  ***** ]  
		0x60, 	//  [ **     ]  
		0x60, 	//  [ **     ]  
		0x3C, 	//  [  ****  ]  
		0x06, 	//  [     ** ]  
		0x06, 	//  [     ** ]  
		0x7C, 	//  [ *****  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 't' charwidth: 8 
		0x00, 	//  [        ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x78, 	//  [ ****   ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x30, 	//  [  **    ]  
		0x1C, 	//  [   ***  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'u' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'v' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'w' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x63, 	//  [ **   **]  
		0x6B, 	//  [ ** * **]  
		0x6B, 	//  [ ** * **]  
		0x3E, 	//  [  ***** ]  
		0x36, 	//  [  ** ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'x' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3C, 	//  [  ****  ]  
		0x18, 	//  [   **   ]  
		0x3C, 	//  [  ****  ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// 'y' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x66, 	//  [ **  ** ]  
		0x3E, 	//  [  ***** ]  
		0x06, 	//  [     ** ]  
		0x7C, 	//  [ *****  ]  
		
		// 'z' charwidth: 8 
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x7E, 	//  [ ****** ]  
		0x06, 	//  [     ** ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x60, 	//  [ **     ]  
		0x7E, 	//  [ ****** ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		
		// '{' charwidth: 8 
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x00, 	//  [        ]  
		
		// '|' charwidth: 8 
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x00, 	//  [        ]  
		
		// '}' charwidth: 8 
		0x30, 	//  [  **    ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x0C, 	//  [    **  ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x18, 	//  [   **   ]  
		0x30, 	//  [  **    ]  
		0x00, 	//  [        ]  
		
		// '~' charwidth: 8 
		0x00, 	//  [        ]  
		0x76, 	//  [ *** ** ]  
		0xDC, 	//  [** ***  ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]  
		0x00, 	//  [        ]

    #if USER_SPECIAL_NINE_DOT > 0                                        // {4}
        #define USER_SPECIAL_FONT_NINE_DOT
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_NINE_DOT
    #endif
};
#endif

#if defined EN_TEN_DOT
const unsigned char ten_dot_glyph_table[] = {
		// ' ' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '!' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '"' charwidth: 9,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x24, 0x00, 	//  [  *  *   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '#' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '$' charwidth: 9,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x3E, 0x00, 	//  [  *****  ] ,            
		0x6B, 0x00, 	//  [ ** * ** ] ,            
		0x69, 0x00, 	//  [ ** *  * ] ,            
		0x68, 0x00, 	//  [ ** *    ] ,            
		0x3E, 0x00, 	//  [  *****  ] ,            
		0x0B, 0x00, 	//  [    * ** ] ,            
		0x0B, 0x00, 	//  [    * ** ] ,            
		0x4B, 0x00, 	//  [ *  * ** ] ,            
		0x6B, 0x00, 	//  [ ** * ** ] ,            
		0x3E, 0x00, 	//  [  *****  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		
		// '%' charwidth: 9,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x51, 0x00, 	//  [ * *   * ] ,            
		0x73, 0x00, 	//  [ ***  ** ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x67, 0x00, 	//  [ **  *** ] ,            
		0xC5, 0x00, 	//  [**   * * ] ,            
		0x87, 0x00, 	//  [*    *** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '&' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0xCD, 0x00, 	//  [**  ** * ] ,            
		0xC7, 0x00, 	//  [**   *** ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7B, 0x00, 	//  [ **** ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ''' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '(' charwidth: 9,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ')' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '*' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '+' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ',' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		
		// '-' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '.' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '/' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x01, 0x00, 	//  [       * ] ,            
		0x03, 0x00, 	//  [      ** ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '0' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '1' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x7E, 0x00, 	//  [ ******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '2' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '3' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '4' charwidth: 9,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x1E, 0x00, 	//  [   ****  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '5' charwidth: 9,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '6' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xFC, 0x00, 	//  [******   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '7' charwidth: 9,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '8' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '9' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7E, 0x00, 	//  [ ******  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ':' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ';' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '<' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '=' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '>' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x03, 0x00, 	//  [      ** ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '?' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '@' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xCE, 0x00, 	//  [**  ***  ] ,            
		0xDE, 0x00, 	//  [** ****  ] ,            
		0xDC, 0x00, 	//  [** ***   ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'A' charwidth: 9,            
		0x10, 0x00, 	//  [   *     ] ,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'B' charwidth: 9,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'C' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xC2, 0x00, 	//  [**    *  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC2, 0x00, 	//  [**    *  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'D' charwidth: 9,            
		0xF8, 0x00, 	//  [*****    ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0xF8, 0x00, 	//  [*****    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'E' charwidth: 9,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x68, 0x00, 	//  [ ** *    ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x68, 0x00, 	//  [ ** *    ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'F' charwidth: 9,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x68, 0x00, 	//  [ ** *    ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x68, 0x00, 	//  [ ** *    ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xF0, 0x00, 	//  [****     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'G' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xC2, 0x00, 	//  [**    *  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xCE, 0x00, 	//  [**  ***  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3A, 0x00, 	//  [  *** *  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'H' charwidth: 9,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'I' charwidth: 9,            
		0x7E, 0x00, 	//  [ ******  ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x7E, 0x00, 	//  [ ******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'J' charwidth: 9,            
		0x1E, 0x00, 	//  [   ****  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'K' charwidth: 9,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'L' charwidth: 9,            
		0xF0, 0x00, 	//  [****     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'M' charwidth: 9,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xE7, 0x00, 	//  [***  *** ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'N' charwidth: 9,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0xF6, 0x00, 	//  [**** **  ] ,            
		0xDE, 0x00, 	//  [** ****  ] ,            
		0xCE, 0x00, 	//  [**  ***  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'O' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'P' charwidth: 9,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xF0, 0x00, 	//  [****     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'Q' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xD6, 0x00, 	//  [** * **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		
		// 'R' charwidth: 9,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'S' charwidth: 9,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'T' charwidth: 9,            
		0xFF, 0x00, 	//  [******** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'U' charwidth: 9,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'V' charwidth: 9,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'W' charwidth: 9,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'X' charwidth: 9,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'Y' charwidth: 9,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'Z' charwidth: 9,            
		0xFF, 0x00, 	//  [******** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '[' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '\' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x80, 0x00, 	//  [*        ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xE0, 0x00, 	//  [***      ] ,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x0E, 0x00, 	//  [    ***  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x02, 0x00, 	//  [      *  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// ']' charwidth: 9,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '^' charwidth: 9,            
		0x08, 0x00, 	//  [    *    ] ,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x36, 0x00, 	//  [  ** **  ] ,            
		0x63, 0x00, 	//  [ **   ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '_' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '`' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'a' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'b' charwidth: 9,            
		0xE0, 0x00, 	//  [***      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xDC, 0x00, 	//  [** ***   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'c' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'd' charwidth: 9,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'e' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'f' charwidth: 9,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x36, 0x00, 	//  [  ** **  ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'g' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		
		// 'h' charwidth: 9,            
		0xE0, 0x00, 	//  [***      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'i' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'j' charwidth: 9,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x0E, 0x00, 	//  [    ***  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		
		// 'k' charwidth: 9,            
		0xE0, 0x00, 	//  [***      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x78, 0x00, 	//  [ ****    ] ,            
		0x6C, 0x00, 	//  [ ** **   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'l' charwidth: 9,            
		0x38, 0x00, 	//  [  ***    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'm' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xE6, 0x00, 	//  [***  **  ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'n' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xDC, 0x00, 	//  [** ***   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'o' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'p' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xDC, 0x00, 	//  [** ***   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xF0, 0x00, 	//  [****     ] ,            
		
		// 'q' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x0C, 0x00, 	//  [    **   ] ,            
		0x1E, 0x00, 	//  [   ****  ] ,            
		
		// 'r' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xDC, 0x00, 	//  [** ***   ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xF0, 0x00, 	//  [****     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 's' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC0, 0x00, 	//  [**       ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7C, 0x00, 	//  [ *****   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 't' charwidth: 9,            
		0x10, 0x00, 	//  [   *     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0xFC, 0x00, 	//  [******   ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x36, 0x00, 	//  [  ** **  ] ,            
		0x1C, 0x00, 	//  [   ***   ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'u' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x76, 0x00, 	//  [ *** **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'v' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'w' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xDB, 0x00, 	//  [** ** ** ] ,            
		0xFF, 0x00, 	//  [******** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'x' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x3C, 0x00, 	//  [  ****   ] ,            
		0x66, 0x00, 	//  [ **  **  ] ,            
		0xC3, 0x00, 	//  [**    ** ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// 'y' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0x7E, 0x00, 	//  [ ******  ] ,            
		0x06, 0x00, 	//  [     **  ] ,            
		0xFC, 0x00, 	//  [******   ] ,            
		
		// 'z' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0xCC, 0x00, 	//  [**  **   ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x30, 0x00, 	//  [  **     ] ,            
		0x60, 0x00, 	//  [ **      ] ,            
		0xC6, 0x00, 	//  [**   **  ] ,            
		0xFE, 0x00, 	//  [*******  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '{' charwidth: 9,            
		0x0E, 0x00, 	//  [    ***  ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0E, 0x00, 	//  [    ***  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '|' charwidth: 9,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '}' charwidth: 9,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x0E, 0x00, 	//  [    ***  ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x18, 0x00, 	//  [   **    ] ,            
		0x70, 0x00, 	//  [ ***     ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		
		// '~' charwidth: 9,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x3B, 0x00, 	//  [  *** ** ] ,            
		0x6E, 0x00, 	//  [ ** ***  ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,            
		0x00, 0x00, 	//  [         ] ,

    #if USER_SPECIAL_TEN_DOT > 0                                         // {4}
        #define USER_SPECIAL_FONT_TEN_DOT
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_TEN_DOT
    #endif
};
#endif

#if defined EN_FIFTEEN_DOT
const unsigned char fifteen_dot_glyph_table[] = {
		// ' ' charwidth: 6
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		0x00,	//  [      ] 
		
		// '!' charwidth: 6
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		
		// '"' charwidth: 8
		0x6C, 	//  [ ** **  ] 
		0x6C, 	//  [ ** **  ] 
		0x6C, 	//  [ ** **  ] 
		0x6C, 	//  [ ** **  ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		
		// '#' charwidth: 13
		0x04, 0x40, //  [     *   *   ] 
		0x04, 0x40,	//  [     *   *   ] 
		0x04, 0x40,	//  [     *   *   ] 
		0x04, 0x40,	//  [     *   *   ] 
		0x7F, 0xF0,	//  [ *********** ] 
		0x08, 0x80,	//  [    *   *    ] 
		0x08, 0x80,	//  [    *   *    ] 
		0x08, 0x80,	//  [    *   *    ] 
		0x09, 0x00,	//  [    *  *     ] 
		0x11, 0x00,	//  [   *   *     ] 
		0xFF, 0xE0,	//  [***********  ] 
		0x11, 0x00,	//  [   *   *     ] 
		0x12, 0x00,	//  [   *  *      ] 
		0x22, 0x00,	//  [  *   *      ] 
		0x22, 0x00,	//  [  *   *      ] 
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		
		// '$' charwidth: 13
		0x02, 0x00,  	//  [      *      ] 
		0x0F, 0xC0,  	//  [    ******   ] 
		0x1A, 0x60,  	//  [   ** *  **  ] 
		0x32, 0x20,  	//  [  **  *   *  ] 
		0x32, 0x00,  	//  [  **  *      ] 
		0x3A, 0x00,  	//  [  *** *      ] 
		0x1E, 0x00,  	//  [   ****      ] 
		0x07, 0x00,  	//  [     ***     ] 
		0x03, 0xC0,  	//  [      ****   ] 
		0x02, 0xE0,  	//  [      * ***  ] 
		0x02, 0x60,  	//  [      *  **  ] 
		0x02, 0x60,  	//  [      *  **  ] 
		0x22, 0x60,  	//  [  *   *  **  ] 
		0x32, 0xC0,  	//  [  **  * **   ] 
		0x1F, 0x80,  	//  [   ******    ] 
		0x02, 0x00,  	//  [      *      ] 
		0x00, 0x00,  	//  [             ] 
		0x00, 0x00,  	//  [             ] 
		
		// '%' charwidth: 14
		0x78, 0x10, 	//  [ ****      *  ] 
		0xCC, 0x10, 	//  [**  **     *  ] 
		0xCC, 0x20, 	//  [**  **    *   ] 
		0xCC, 0x40, 	//  [**  **   *    ] 
		0xCC, 0x40, 	//  [**  **   *    ] 
		0xCC, 0x80, 	//  [**  **  *     ] 
		0x79, 0x00, 	//  [ ****  *      ] 
		0x02, 0x00, 	//  [      *       ] 
		0x02, 0xF0, 	//  [      * ****  ] 
		0x05, 0x98, 	//  [     * **  ** ] 
		0x09, 0x98, 	//  [    *  **  ** ] 
		0x11, 0x98, 	//  [   *   **  ** ] 
		0x11, 0x98, 	//  [   *   **  ** ] 
		0x21, 0x98, 	//  [  *    **  ** ] 
		0x40, 0xF0, 	//  [ *      ****  ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// '&' charwidth: 14
		0x07, 0x80, 	//  [     ****     ] 
		0x0C, 0xC0, 	//  [    **  **    ] 
		0x0C, 0xC0, 	//  [    **  **    ] 
		0x0C, 0xC0, 	//  [    **  **    ] 
		0x0C, 0xC0, 	//  [    **  **    ] 
		0x07, 0x80, 	//  [     ****     ] 
		0x0E, 0x00, 	//  [    ***       ] 
		0x33, 0x3C, 	//  [  **  **  ****] 
		0x63, 0x08, 	//  [ **   **    * ] 
		0x61, 0x88, 	//  [ **    **   * ] 
		0x61, 0xD0, 	//  [ **    *** *  ] 
		0x70, 0xF0, 	//  [ ***    ****  ] 
		0x78, 0x60, 	//  [ ****    **   ] 
		0x3F, 0xF0, 	//  [  **********  ] 
		0x0F, 0x38, 	//  [    ****  *** ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// ''' charwidth: 5
		0x60, 	//  [ **  ] 
		0x60, 	//  [ **  ] 
		0x60, 	//  [ **  ] 
		0x60, 	//  [ **  ] 
		0x60, 	//  [ **  ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		
		// '(' charwidth: 7
		0x04, 	//  [     * ] 
		0x08, 	//  [    *  ] 
		0x18, 	//  [   **  ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x20, 	//  [  *    ] 
		0x60, 	//  [ **    ] 
		0x60, 	//  [ **    ] 
		0x60, 	//  [ **    ] 
		0x60, 	//  [ **    ] 
		0x60, 	//  [ **    ] 
		0x60, 	//  [ **    ] 
		0x20, 	//  [  *    ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x18, 	//  [   **  ] 
		0x08, 	//  [    *  ] 
		0x04, 	//  [     * ] 
		
		// ')' charwidth: 7
		0x40, 	//  [ *     ] 
		0x20, 	//  [  *    ] 
		0x30, 	//  [  **   ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x08, 	//  [    *  ] 
		0x0C, 	//  [    ** ] 
		0x0C, 	//  [    ** ] 
		0x0C, 	//  [    ** ] 
		0x0C, 	//  [    ** ] 
		0x0C, 	//  [    ** ] 
		0x0C, 	//  [    ** ] 
		0x08, 	//  [    *  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x30, 	//  [  **   ] 
		0x20, 	//  [  *    ] 
		0x40, 	//  [ *     ] 
		
		// '*' charwidth: 10
		0x0C, 0x00, 	//  [    **    ] 
		0x0C, 0x00, 	//  [    **    ] 
		0x4C, 0x80, 	//  [ *  **  * ] 
		0x73, 0x80, 	//  [ ***  *** ] 
		0x12, 0x00, 	//  [   *  *   ] 
		0x1E, 0x00, 	//  [   ****   ] 
		0x33, 0x00, 	//  [  **  **  ] 
		0x33, 0x00, 	//  [  **  **  ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		
		// '+' charwidth: 13
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x7F, 0xE0,	//  [ **********  ] 
		0x7F, 0xE0,	//  [ **********  ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x06, 0x00,	//  [     **      ] 
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		0x00, 0x00,	//  [             ] 
		
		// ',' charwidth: 6
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x10, 	//  [   *  ] 
		0x10, 	//  [   *  ] 
		0x20, 	//  [  *   ] 
		
		// '-' charwidth: 7
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x7C, 	//  [ ***** ] 
		0x7C, 	//  [ ***** ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		
		// '.' charwidth: 6
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		
		// '/' charwidth: 11
		0x00, 0xC0, 	//  [        ** ] 
		0x00, 0xC0, 	//  [        ** ] 
		0x01, 0x80, 	//  [       **  ] 
		0x01, 0x80, 	//  [       **  ] 
		0x03, 0x80, 	//  [      ***  ] 
		0x03, 0x00, 	//  [      **   ] 
		0x03, 0x00, 	//  [      **   ] 
		0x06, 0x00, 	//  [     **    ] 
		0x06, 0x00, 	//  [     **    ] 
		0x0C, 0x00, 	//  [    **     ] 
		0x0C, 0x00, 	//  [    **     ] 
		0x18, 0x00, 	//  [   **      ] 
		0x18, 0x00, 	//  [   **      ] 
		0x38, 0x00, 	//  [  ***      ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		
		// '0' charwidth: 13
		0x0F, 0x00, 	//  [    ****     ] 
		0x19, 0x80, 	//  [   **  **    ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x20, 0x40, 	//  [  *      *   ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x20, 0x40, 	//  [  *      *   ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x19, 0x80, 	//  [   **  **    ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '1' charwidth: 13
		0x02, 0x00, 	//  [      *      ] 
		0x1E, 0x00, 	//  [   ****      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x1F, 0x80, 	//  [   ******    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '2' charwidth: 13
		0x1F, 0x80, 	//  [   ******    ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x20, 0x60, 	//  [  *      **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x01, 0x80, 	//  [       **    ] 
		0x03, 0x00, 	//  [      **     ] 
		0x06, 0x00, 	//  [     **      ] 
		0x0c, 0x00, 	//  [    **       ] 
		0x18, 0x00, 	//  [   **        ] 
		0x30, 0x00, 	//  [  **         ] 
		0x3F, 0xE0, 	//  [  *********  ] 
		0x3F, 0xE0, 	//  [  *********  ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '3' charwidth: 13
		0x1F, 0x80, 	//  [   ******    ] 
		0x20, 0xC0, 	//  [  *     **   ] 
		0x20, 0x60, 	//  [  *      **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x01, 0x80, 	//  [       **    ] 
		0x1F, 0x00, 	//  [   *****     ] 
		0x01, 0xC0, 	//  [       ***   ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x20, 0x60, 	//  [  *      **  ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x1F, 0x80, 	//  [   ******    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '4' charwidth: 13
		0x00, 0x40, 	//  [         *   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x01, 0xC0, 	//  [       ***   ] 
		0x02, 0xC0, 	//  [      * **   ] 
		0x04, 0xC0, 	//  [     *  **   ] 
		0x08, 0xC0, 	//  [    *   **   ] 
		0x10, 0xC0, 	//  [   *    **   ] 
		0x10, 0xC0, 	//  [   *    **   ] 
		0x20, 0xC0, 	//  [  *     **   ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x01, 0xE0, 	//  [       ****  ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '5' charwidth: 13
		0x1F, 0xC0, 	//  [   *******   ] 
		0x1F, 0xC0, 	//  [   *******   ] 
		0x10, 0x00, 	//  [   *         ] 
		0x10, 0x00, 	//  [   *         ] 
		0x10, 0x00, 	//  [   *         ] 
		0x10, 0x00, 	//  [   *         ] 
		0x1F, 0x00, 	//  [   *****     ] 
		0x01, 0xC0, 	//  [       ***   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x10, 0xE0, 	//  [   *    ***  ] 
		0x10, 0xC0, 	//  [   *    **   ] 
		0x0F, 0x80, 	//  [    *****    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '6' charwidth: 13
		0x0F, 0x80, 	//  [    *****    ] 
		0x18, 0x40, 	//  [   **    *   ] 
		0x30, 0x40, 	//  [  **     *   ] 
		0x30, 0x00, 	//  [  **         ] 
		0x60, 0x00, 	//  [ **          ] 
		0x6F, 0x80, 	//  [ ** *****    ] 
		0x70, 0xC0, 	//  [ ***    **   ] 
		0x60, 0xE0, 	//  [ **     ***  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x20, 0x60, 	//  [  *      **  ] 
		0x30, 0x40, 	//  [  **     *   ] 
		0x18, 0xC0, 	//  [   **   **   ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '7' charwidth: 13
		0x3F, 0xE0, 	//  [  *********  ] 
		0x3F, 0xE0, 	//  [  *********  ] 
		0x00, 0x40, 	//  [         *   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x00, 0x80, 	//  [        *    ] 
		0x01, 0x00, 	//  [       *     ] 
		0x01, 0x00, 	//  [       *     ] 
		0x02, 0x00, 	//  [      *      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x04, 0x00, 	//  [     *       ] 
		0x0C, 0x00, 	//  [    **       ] 
		0x0C, 0x00, 	//  [    **       ] 
		0x08, 0x00, 	//  [    *        ] 
		0x18, 0x00, 	//  [   **        ] 
		0x18, 0x00, 	//  [   **        ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '8' charwidth: 13
		0x0F, 0x80, 	//  [    *****    ] 
		0x18, 0xC0, 	//  [   **   **   ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x1C, 0xC0, 	//  [   ***  **   ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x1F, 0x80, 	//  [   ******    ] 
		0x31, 0xC0, 	//  [  **   ***   ] 
		0x60, 0xE0, 	//  [ **     ***  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x1F, 0x80, 	//  [   ******    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '9' charwidth: 13
		0x0F, 0x00, 	//  [    ****     ] 
		0x31, 0xC0, 	//  [  **   ***   ] 
		0x20, 0xC0, 	//  [  *     **   ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x1F, 0x60, 	//  [   ***** **  ] 
		0x00, 0x40, 	//  [         *   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x21, 0x80, 	//  [  *    **    ] 
		0x3E, 0x00, 	//  [  *****      ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// ':' charwidth: 6
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		
		// ';' charwidth: 6
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x00, 	//  [      ] 
		0x30, 	//  [  **  ] 
		0x30, 	//  [  **  ] 
		0x10, 	//  [   *  ] 
		0x10, 	//  [   *  ] 
		0x20, 	//  [  *   ] 
		
		// '<' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x10, 	//  [           * ] 
		0x00, 0x70, 	//  [         *** ] 
		0x01, 0xE0, 	//  [       ****  ] 
		0x07, 0x80, 	//  [     ****    ] 
		0x1E, 0x00, 	//  [   ****      ] 
		0x78, 0x00, 	//  [ ****        ] 
		0x1E, 0x00, 	//  [   ****      ] 
		0x07, 0x80, 	//  [     ****    ] 
		0x01, 0xE0, 	//  [       ****  ] 
		0x00, 0x70, 	//  [         *** ] 
		0x00, 0x10, 	//  [           * ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '=' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '>' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x40, 0x00, 	//  [ *           ] 
		0x70, 0x00, 	//  [ ***         ] 
		0x3C, 0x00, 	//  [  ****       ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x03, 0xC0, 	//  [      ****   ] 
		0x00, 0xF0, 	//  [        **** ] 
		0x03, 0xE0, 	//  [      *****  ] 
		0x0F, 0x80, 	//  [    *****    ] 
		0x3E, 0x00, 	//  [  *****      ] 
		0x70, 0x00, 	//  [ ***         ] 
		0x40, 0x00, 	//  [ *           ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '?' charwidth: 10
		0x3E, 0x00, 	//  [  *****   ] 
		0x63, 0x00, 	//  [ **   **  ] 
		0x41, 0x80, 	//  [ *     ** ] 
		0x01, 0x80, 	//  [       ** ] 
		0x01, 0x80, 	//  [       ** ] 
		0x03, 0x00, 	//  [      **  ] 
		0x02, 0x00, 	//  [      *   ] 
		0x0C, 0x00, 	//  [    **    ] 
		0x08, 0x00, 	//  [    *     ] 
		0x18, 0x00, 	//  [   **     ] 
		0x18, 0x00, 	//  [   **     ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x18, 0x00, 	//  [   **     ] 
		0x18, 0x00, 	//  [   **     ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		
		// '@' charwidth: 18
		0x01, 0xF8, 0x00, 	//  [       ******     ] 
		0x06, 0x06, 0x00, 	//  [     **      **   ] 
		0x0C, 0x03, 0x00, 	//  [    **        **  ] 
		0x10, 0xF9, 0x00, 	//  [   *    *****  *  ] 
		0x31, 0x98, 0x80, 	//  [  **   **  **   * ] 
		0x23, 0x18, 0x80, 	//  [  *   **   **   * ] 
		0x42, 0x18, 0x80, 	//  [ *    *    **   * ] 
		0x46, 0x30, 0x80, 	//  [ *   **   **    * ] 
		0x46, 0x30, 0x80, 	//  [ *   **   **    * ] 
		0x46, 0x71, 0x00, 	//  [ *   **  ***   *  ] 
		0x46, 0xB2, 0x00, 	//  [ *   ** * **  *   ] 
		0x23, 0x3C, 0x00, 	//  [  *   **  ****    ] 
		0x20, 0x00, 0x00, 	//  [  *               ] 
		0x18, 0x20, 0x00, 	//  [   **     *       ] 
		0x07, 0xC0, 0x00, 	//  [     *****        ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		
		// 'A' charwidth: 14
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x80, 	//  [      ***     ] 
		0x05, 0x80, 	//  [     * **     ] 
		0x05, 0x80, 	//  [     * **     ] 
		0x08, 0xC0, 	//  [    *   **    ] 
		0x08, 0xC0, 	//  [    *   **    ] 
		0x10, 0xE0, 	//  [   *    ***   ] 
		0x10, 0x60, 	//  [   *     **   ] 
		0x10, 0x60, 	//  [   *     **   ] 
		0x3F, 0xF0, 	//  [  **********  ] 
		0x20, 0x30, 	//  [  *       **  ] 
		0x40, 0x18, 	//  [ *         ** ] 
		0x40, 0x18, 	//  [ *         ** ] 
		0xE0, 0x3C, 	//  [***       ****] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'B' charwidth: 12
		0x7F, 0x00, 	//  [ *******    ] 
		0x31, 0xC0, 	//  [  **   ***  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0x80, 	//  [  **    *   ] 
		0x31, 0x80, 	//  [  **   **   ] 
		0x3E, 0x00, 	//  [  *****     ] 
		0x31, 0xC0, 	//  [  **   ***  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x7F, 0x80, 	//  [ ********   ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'C' charwidth: 14
		0x07, 0xF0, 	//  [     *******  ] 
		0x1C, 0x18, 	//  [   ***     ** ] 
		0x38, 0x08, 	//  [  ***       * ] 
		0x30, 0x00, 	//  [  **          ] 
		0x70, 0x00, 	//  [ ***          ] 
		0x60, 0x00, 	//  [ **           ] 
		0x60, 0x00, 	//  [ **           ] 
		0x60, 0x00, 	//  [ **           ] 
		0x60, 0x00, 	//  [ **           ] 
		0x60, 0x00, 	//  [ **           ] 
		0x70, 0x00, 	//  [ ***          ] 
		0x30, 0x00, 	//  [  **          ] 
		0x18, 0x00, 	//  [   **         ] 
		0x0C, 0x08, 	//  [    **      * ] 
		0x07, 0xF0, 	//  [     *******  ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'D' charwidth: 16
		0x7F, 0xC0, 	//  [ *********      ] 
		0x30, 0x70, 	//  [  **     ***    ] 
		0x30, 0x38, 	//  [  **      ***   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x0C, 	//  [  **        **  ] 
		0x30, 0x1C, 	//  [  **       ***  ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x38, 	//  [  **      ***   ] 
		0x30, 0x70, 	//  [  **     ***    ] 
		0x7F, 0xC0, 	//  [ *********      ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		
		// 'E' charwidth: 12
		0x7F, 0xC0, 	//  [ *********  ] 
		0x30, 0x40, 	//  [  **     *  ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x31, 0x00, 	//  [  **   *    ] 
		0x3F, 0x00, 	//  [  ******    ] 
		0x31, 0x00, 	//  [  **   *    ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x40, 	//  [  **     *  ] 
		0x30, 0x40, 	//  [  **     *  ] 
		0x7F, 0xC0, 	//  [ *********  ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'F' charwidth: 11
		0x7F, 0xC0, 	//  [ ********* ] 
		0x30, 0x40, 	//  [  **     * ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x80, 	//  [  **    *  ] 
		0x3F, 0x80, 	//  [  *******  ] 
		0x30, 0x80, 	//  [  **    *  ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x78, 0x00, 	//  [ ****      ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'G' charwidth: 15
		0x07, 0xF0, 	//  [     *******   ] 
		0x1C, 0x18, 	//  [   ***     **  ] 
		0x38, 0x08, 	//  [  ***       *  ] 
		0x30, 0x00, 	//  [  **           ] 
		0x70, 0x00, 	//  [ ***           ] 
		0x60, 0x00, 	//  [ **            ] 
		0x60, 0x00, 	//  [ **            ] 
		0x60, 0x00, 	//  [ **            ] 
		0x60, 0x7C, 	//  [ **      ***** ] 
		0x60, 0x18, 	//  [ **        **  ] 
		0x70, 0x18, 	//  [ ***       **  ] 
		0x30, 0x18, 	//  [  **       **  ] 
		0x18, 0x18, 	//  [   **      **  ] 
		0x0C, 0x18, 	//  [    **     **  ] 
		0x07, 0xF0, 	//  [     *******   ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		
		// 'H' charwidth: 16
		0x78, 0x3C, 	//  [ ****     ****  ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x78, 0x3C, 	//  [ ****     ****  ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		
		// 'I' charwidth: 7
		0x78, 	//  [ ****  ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x78, 	//  [ ****  ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		
		// 'J' charwidth: 9
		0x3E, 0x00, 	//  [  *****  ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x0C, 0x00, 	//  [    **   ] 
		0x8C, 0x00, 	//  [*   **   ] 
		0x98, 0x00, 	//  [*  **    ] 
		0xF0, 0x00, 	//  [****     ] 
		
		// 'K' charwidth: 14
		0x78, 0xF0, 	//  [ ****   ****  ] 
		0x30, 0x40, 	//  [  **     *    ] 
		0x30, 0x80, 	//  [  **    *     ] 
		0x30, 0x80, 	//  [  **    *     ] 
		0x31, 0x00, 	//  [  **   *      ] 
		0x32, 0x00, 	//  [  **  *       ] 
		0x34, 0x00, 	//  [  ** *        ] 
		0x3C, 0x00, 	//  [  ****        ] 
		0x36, 0x00, 	//  [  ** **       ] 
		0x33, 0x00, 	//  [  **  **      ] 
		0x31, 0x80, 	//  [  **   **     ] 
		0x30, 0xC0, 	//  [  **    **    ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x30, 0x30, 	//  [  **      **  ] 
		0x78, 0xF8, 	//  [ ****   ***** ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'L' charwidth: 11
		0x78, 0x00, 	//  [ ****      ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x40, 	//  [  **     * ] 
		0x30, 0x40, 	//  [  **     * ] 
		0x7F, 0xC0, 	//  [ ********* ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'M' charwidth: 17
		0x70, 0x07, 0x00, 	//  [ ***         *** ] 
		0x30, 0x06, 0x00, 	//  [  **         **  ] 
		0x38, 0x0E, 0x00, 	//  [  ***       ***  ] 
		0x38, 0x0E, 0x00, 	//  [  ***       ***  ] 
		0x3C, 0x16, 0x00, 	//  [  ****     * **  ] 
		0x2C, 0x16, 0x00, 	//  [  * **     * **  ] 
		0x2C, 0x36, 0x00, 	//  [  * **    ** **  ] 
		0x26, 0x26, 0x00, 	//  [  *  **   *  **  ] 
		0x26, 0x26, 0x00, 	//  [  *  **   *  **  ] 
		0x23, 0x46, 0x00, 	//  [  *   ** *   **  ] 
		0x23, 0x46, 0x00, 	//  [  *   ** *   **  ] 
		0x23, 0x86, 0x00, 	//  [  *   ***    **  ] 
		0x21, 0x86, 0x00, 	//  [  *    **    **  ] 
		0x21, 0x86, 0x00, 	//  [  *    **    **  ] 
		0x70, 0x0F, 0x00, 	//  [ ***        **** ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		
		// 'N' charwidth: 15
		0x70, 0x1C, 	//  [ ***       *** ] 
		0x30, 0x08, 	//  [  **        *  ] 
		0x38, 0x08, 	//  [  ***       *  ] 
		0x2C, 0x08, 	//  [  * **      *  ] 
		0x2E, 0x08, 	//  [  * ***     *  ] 
		0x26, 0x08, 	//  [  *  **     *  ] 
		0x23, 0x08, 	//  [  *   **    *  ] 
		0x21, 0x88, 	//  [  *    **   *  ] 
		0x21, 0x88, 	//  [  *    **   *  ] 
		0x20, 0xC8, 	//  [  *     **  *  ] 
		0x20, 0x68, 	//  [  *      ** *  ] 
		0x20, 0x78, 	//  [  *      ****  ] 
		0x20, 0x38, 	//  [  *       ***  ] 
		0x20, 0x18, 	//  [  *        **  ] 
		0x70, 0x08, 	//  [ ***        *  ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		
		// 'O' charwidth: 16
		0x07, 0xC0,	//  [     *****      ] 
		0x1C, 0x70,	//  [   ***   ***    ] 
		0x30, 0x18,	//  [  **       **   ] 
		0x30, 0x18,	//  [  **       **   ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x60, 0x0C,	//  [ **         **  ] 
		0x30, 0x18,	//  [  **       **   ] 
		0x30, 0x18,	//  [  **       **   ] 
		0x1C, 0x70,	//  [   ***   ***    ] 
		0x07, 0xC0,	//  [     *****      ] 
		0x00, 0x00,	//  [                ] 
		0x00, 0x00,	//  [                ] 
		0x00, 0x00,	//  [                ] 
		
		// 'P' charwidth: 12
		0x7F, 0x80, 	//  [ ********   ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0x60, 	//  [  **     ** ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x31, 0xC0, 	//  [  **   ***  ] 
		0x3F, 0x00, 	//  [  ******    ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x78, 0x00, 	//  [ ****       ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'Q' charwidth: 16
		0x07, 0xC0, 	//  [     *****      ] 
		0x1C, 0x70, 	//  [   ***   ***    ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x60, 0x0C, 	//  [ **         **  ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x30, 0x18, 	//  [  **       **   ] 
		0x1C, 0x70, 	//  [   ***   ***    ] 
		0x07, 0xC0, 	//  [     *****      ] 
		0x00, 0x30, 	//  [          **    ] 
		0x00, 0x1F, 	//  [           *****] 
		0x00, 0x07, 	//  [             ***] 
		
		// 'R' charwidth: 14
		0x7F, 0x80, 	//  [ ********     ] 
		0x30, 0xC0, 	//  [  **    **    ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x31, 0xC0, 	//  [  **   ***    ] 
		0x3F, 0x00, 	//  [  ******      ] 
		0x33, 0x00, 	//  [  **  **      ] 
		0x31, 0x80, 	//  [  **   **     ] 
		0x31, 0xC0, 	//  [  **   ***    ] 
		0x30, 0xC0, 	//  [  **    **    ] 
		0x30, 0x60, 	//  [  **     **   ] 
		0x30, 0x70, 	//  [  **     ***  ] 
		0x78, 0x38, 	//  [ ****     *** ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'S' charwidth: 11
		0x1F, 0x80, 	//  [   ******  ] 
		0x30, 0xC0, 	//  [  **    ** ] 
		0x60, 0x40, 	//  [ **      * ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x70, 0x00, 	//  [ ***       ] 
		0x3C, 0x00, 	//  [  ****     ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x03, 0x80, 	//  [      ***  ] 
		0x01, 0xC0, 	//  [       *** ] 
		0x00, 0xC0, 	//  [        ** ] 
		0x00, 0xC0, 	//  [        ** ] 
		0x40, 0xC0, 	//  [ *      ** ] 
		0x61, 0x80, 	//  [ **    **  ] 
		0x3F, 0x00, 	//  [  ******   ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'T' charwidth: 14
		0x7F, 0xF8, 	//  [ ************ ] 
		0x43, 0x08, 	//  [ *    **    * ] 
		0x43, 0x08, 	//  [ *    **    * ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x03, 0x00, 	//  [      **      ] 
		0x07, 0x80, 	//  [     ****     ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'U' charwidth: 15
		0x78, 0x38, 	//  [ ****     ***  ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x30, 0x10, 	//  [  **       *   ] 
		0x18, 0x20, 	//  [   **     *    ] 
		0x1C, 0x60, 	//  [   ***   **    ] 
		0x0F, 0xC0, 	//  [    ******     ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		0x00, 0x00, 	//  [               ] 
		
		// 'V' charwidth: 14
		0x78, 0x1C, 	//  [ ****      ***] 
		0x30, 0x08, 	//  [  **        * ] 
		0x30, 0x08, 	//  [  **        * ] 
		0x18, 0x10, 	//  [   **      *  ] 
		0x18, 0x10, 	//  [   **      *  ] 
		0x18, 0x10, 	//  [   **      *  ] 
		0x0C, 0x20, 	//  [    **    *   ] 
		0x0C, 0x20, 	//  [    **    *   ] 
		0x06, 0x40, 	//  [     **  *    ] 
		0x06, 0x40, 	//  [     **  *    ] 
		0x06, 0x40, 	//  [     **  *    ] 
		0x03, 0x80, 	//  [      ***     ] 
		0x03, 0x80, 	//  [      ***     ] 
		0x03, 0x80, 	//  [      ***     ] 
		0x01, 0x00, 	//  [       *      ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		0x00, 0x00, 	//  [              ] 
		
		// 'W' charwidth: 18
		0x78, 0x61, 0xC0, 	//  [ ****    **    ***] 
		0x30, 0x60, 0x80, 	//  [  **     **     * ] 
		0x30, 0x60, 0x80, 	//  [  **     **     * ] 
		0x30, 0xE0, 0x80, 	//  [  **    ***     * ] 
		0x38, 0xB1, 0x00, 	//  [  ***   * **   *  ] 
		0x18, 0xB1, 0x00, 	//  [   **   * **   *  ] 
		0x19, 0x31, 0x00, 	//  [   **  *  **   *  ] 
		0x19, 0x39, 0x00, 	//  [   **  *  ***  *  ] 
		0x19, 0x1A, 0x00, 	//  [   **  *   ** *   ] 
		0x0D, 0x1A, 0x00, 	//  [    ** *   ** *   ] 
		0x0E, 0x1A, 0x00, 	//  [    ***    ** *   ] 
		0x0E, 0x0E, 0x00, 	//  [    ***     ***   ] 
		0x0E, 0x0C, 0x00, 	//  [    ***     **    ] 
		0x04, 0x0C, 0x00, 	//  [     *      **    ] 
		0x04, 0x04, 0x00, 	//  [     *       *    ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		0x00, 0x00, 0x00, 	//  [                  ] 
		
		// 'X' charwidth: 13
		0xF8, 0x70, 	//  [*****    *** ] 
		0x60, 0x20, 	//  [ **       *  ] 
		0x30, 0x40, 	//  [  **     *   ] 
		0x38, 0x80, 	//  [  ***   *    ] 
		0x18, 0x80, 	//  [   **   *    ] 
		0x0D, 0x00, 	//  [    ** *     ] 
		0x0E, 0x00, 	//  [    ***      ] 
		0x06, 0x00, 	//  [     **      ] 
		0x07, 0x00, 	//  [     ***     ] 
		0x0B, 0x00, 	//  [    * **     ] 
		0x11, 0x80, 	//  [   *   **    ] 
		0x11, 0xC0, 	//  [   *   ***   ] 
		0x20, 0xC0, 	//  [  *     **   ] 
		0x40, 0x60, 	//  [ *       **  ] 
		0xE1, 0xF0, 	//  [***    ***** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'Y' charwidth: 12
		0xF0, 0x70, 	//  [****     ***] 
		0x60, 0x20, 	//  [ **       * ] 
		0x30, 0x40, 	//  [  **     *  ] 
		0x38, 0x40, 	//  [  ***    *  ] 
		0x18, 0x80, 	//  [   **   *   ] 
		0x1C, 0x80, 	//  [   ***  *   ] 
		0x0D, 0x00, 	//  [    ** *    ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x06, 0x00, 	//  [     **     ] 
		0x0F, 0x00, 	//  [    ****    ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'Z' charwidth: 13
		0x7F, 0xE0, 	//  [ **********  ] 
		0x40, 0xE0, 	//  [ *      ***  ] 
		0x00, 0xC0, 	//  [        **   ] 
		0x01, 0xC0, 	//  [       ***   ] 
		0x01, 0x80, 	//  [       **    ] 
		0x03, 0x00, 	//  [      **     ] 
		0x03, 0x00, 	//  [      **     ] 
		0x06, 0x00, 	//  [     **      ] 
		0x0E, 0x00, 	//  [    ***      ] 
		0x0C, 0x00, 	//  [    **       ] 
		0x18, 0x00, 	//  [   **        ] 
		0x18, 0x00, 	//  [   **        ] 
		0x30, 0x00, 	//  [  **         ] 
		0x70, 0x20, 	//  [ ***      *  ] 
		0x7F, 0xE0, 	//  [ **********  ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '[' charwidth: 7
		0x3C, 	//  [  **** ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x3C, 	//  [  **** ] 
		
		// '\' charwidth: 11
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x30, 0x00, 	//  [  **       ] 
		0x30, 0x00, 	//  [  **       ] 
		0x38, 0x00, 	//  [  ***      ] 
		0x18, 0x00, 	//  [   **      ] 
		0x18, 0x00, 	//  [   **      ] 
		0x0C, 0x00, 	//  [    **     ] 
		0x0C, 0x00, 	//  [    **     ] 
		0x06, 0x00, 	//  [     **    ] 
		0x06, 0x00, 	//  [     **    ] 
		0x03, 0x00, 	//  [      **   ] 
		0x03, 0x00, 	//  [      **   ] 
		0x03, 0x80, 	//  [      ***  ] 
		0x01, 0x80, 	//  [       **  ] 
		0x01, 0x80, 	//  [       **  ] 
		0x00, 0xC0, 	//  [        ** ] 
		0x00, 0xC0, 	//  [        ** ] 
		
		// ']' charwidth: 7
		0x78, 	//  [ ****  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x78, 	//  [ ****  ] 
		
		// '^' charwidth: 13
		0x03, 0x00, 	//  [      **     ] 
		0x03, 0x00, 	//  [      **     ] 
		0x07, 0x80, 	//  [     ****    ] 
		0x07, 0x80, 	//  [     ****    ] 
		0x0C, 0xC0, 	//  [    **  **   ] 
		0x0C, 0xC0, 	//  [    **  **   ] 
		0x18, 0x60, 	//  [   **    **  ] 
		0x18, 0x60, 	//  [   **    **  ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// '_' charwidth: 10
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		0x7F, 0x80, 	//  [ ******** ] 
		0x00, 0x00, 	//  [          ] 
		0x00, 0x00, 	//  [          ] 
		
		// '`' charwidth: 13
		0x06, 0x00, 	//  [     **      ] 
		0x03, 0x00, 	//  [      **     ] 
		0x01, 0x80, 	//  [       **    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'a' charwidth: 11
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x1F, 0x00, 	//  [   *****   ] 
		0x23, 0x80, 	//  [  *   ***  ] 
		0x21, 0x80, 	//  [  *    **  ] 
		0x01, 0x80, 	//  [       **  ] 
		0x01, 0x80, 	//  [       **  ] 
		0x1F, 0x80, 	//  [   ******  ] 
		0x31, 0x80, 	//  [  **   **  ] 
		0x61, 0x80, 	//  [ **    **  ] 
		0x61, 0x80, 	//  [ **    **  ] 
		0x73, 0x80, 	//  [ ***  ***  ] 
		0x3C, 0xE0, 	//  [  ****  ***] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'b' charwidth: 13
		0x70, 0x00, 	//  [ ***         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x37, 0xC0, 	//  [  ** *****   ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x37, 0x80, 	//  [  ** ****    ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'c' charwidth: 11
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x38, 0x80, 	//  [  ***   *  ] 
		0x30, 0x80, 	//  [  **    *  ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x30, 0x00, 	//  [  **       ] 
		0x38, 0x80, 	//  [  ***   *  ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'd' charwidth: 13
		0x00, 0xE0, 	//  [        ***  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x0F, 0x60, 	//  [    **** **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x1F, 0x70, 	//  [   ***** *** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'e' charwidth: 11
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x31, 0x80, 	//  [  **   **  ] 
		0x20, 0xC0, 	//  [  *     ** ] 
		0x60, 0xC0, 	//  [ **     ** ] 
		0x7F, 0xC0, 	//  [ ********* ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x60, 0x00, 	//  [ **        ] 
		0x30, 0x00, 	//  [  **       ] 
		0x18, 0x40, 	//  [   **    * ] 
		0x0F, 0x80, 	//  [    *****  ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 'f' charwidth: 8
		0x0F, 	//  [    ****] 
		0x19, 	//  [   **  *] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x7F, 	//  [ *******] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x3C, 	//  [  ****  ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		
		// 'g' charwidth: 12
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x0F, 0xF0, 	//  [    ********] 
		0x19, 0x80, 	//  [   **  **   ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x19, 0x80, 	//  [   **  **   ] 
		0x0F, 0x00, 	//  [    ****    ] 
		0x10, 0x00, 	//  [   *        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x3F, 0xC0, 	//  [  ********  ] 
		0x1F, 0xE0, 	//  [   ******** ] 
		0x20, 0x60, 	//  [  *      ** ] 
		0x60, 0xC0, 	//  [ **     **  ] 
		0x3F, 0x80, 	//  [  *******   ] 
		
		// 'h' charwidth: 13
		0x70, 0x00, 	//  [ ***         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x37, 0xC0, 	//  [  ** *****   ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x78, 0xF0, 	//  [ ****   **** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'i' charwidth: 7
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x00,	//  [       ] 
		0x00,	//  [       ] 
		0x70,	//  [ ***   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x30,	//  [  **   ] 
		0x78,	//  [ ****  ] 
		0x00,	//  [       ] 
		0x00,	//  [       ] 
		0x00,	//  [       ] 
		
		// 'j' charwidth: 7
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x38, 	//  [  ***  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x10, 	//  [   *   ] 
		0xE0, 	//  [***    ] 
		
		// 'k' charwidth: 12
		0x70, 0x00, 	//  [ ***        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0x00, 	//  [  **        ] 
		0x30, 0xE0, 	//  [  **    *** ] 
		0x30, 0x80, 	//  [  **    *   ] 
		0x31, 0x00, 	//  [  **   *    ] 
		0x32, 0x00, 	//  [  **  *     ] 
		0x34, 0x00, 	//  [  ** *      ] 
		0x3E, 0x00, 	//  [  *****     ] 
		0x36, 0x00, 	//  [  ** **     ] 
		0x33, 0x00, 	//  [  **  **    ] 
		0x31, 0x80, 	//  [  **   **   ] 
		0x30, 0xC0, 	//  [  **    **  ] 
		0x78, 0x70, 	//  [ ****    ***] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'l' charwidth: 7
		0x70, 	//  [ ***   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x78, 	//  [ ****  ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		0x00, 	//  [       ] 
		
		// 'm' charwidth: 19
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x77, 0xCF, 0x80, 	//  [ *** *****  *****  ] 
		0x38, 0x70, 0xC0, 	//  [  ***    ***    ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x30, 0x60, 0xC0, 	//  [  **     **     ** ] 
		0x78, 0xF1, 0xE0, 	//  [ ****   ****   ****] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		0x00, 0x00, 0x00, 	//  [                   ] 
		
		// 'n' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x77, 0xC0, 	//  [ *** *****   ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x78, 0xF0, 	//  [ ****   **** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'o' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x39, 0xC0, 	//  [  ***  ***   ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x30, 0xC0, 	//  [  **    **   ] 
		0x39, 0xC0, 	//  [  ***  ***   ] 
		0x0F, 0x00, 	//  [    ****     ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'p' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x77, 0xC0, 	//  [ *** *****   ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x30, 	//  [  **      ** ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x38, 0x60, 	//  [  ***    **  ] 
		0x37, 0x80, 	//  [  ** ****    ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x30, 0x00, 	//  [  **         ] 
		0x78, 0x00, 	//  [ ****        ] 
		
		// 'q' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x0F, 0x60, 	//  [    **** **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x60, 0x60, 	//  [ **      **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x1F, 0x60, 	//  [   ***** **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0x60, 	//  [         **  ] 
		0x00, 0xF0, 	//  [        **** ] 
		
		// 'r' charwidth: 9
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x73, 0x00, 	//  [ ***  ** ] 
		0x37, 0x00, 	//  [  ** *** ] 
		0x39, 0x00, 	//  [  ***  * ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x30, 0x00, 	//  [  **     ] 
		0x78, 0x00, 	//  [ ****    ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		
		// 's' charwidth: 11
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x30, 0x80, 	//  [  **    *  ] 
		0x30, 0x80, 	//  [  **    *  ] 
		0x30, 0x00, 	//  [  **       ] 
		0x1C, 0x00, 	//  [   ***     ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x03, 0xC0, 	//  [      **** ] 
		0x00, 0xC0, 	//  [        ** ] 
		0x20, 0xC0, 	//  [  *     ** ] 
		0x31, 0x80, 	//  [  **   **  ] 
		0x1F, 0x00, 	//  [   *****   ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// 't' charwidth: 8
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x7E, 	//  [ ****** ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x1E, 	//  [   **** ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		
		// 'u' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x70, 0xE0, 	//  [ ***    ***  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0x60, 	//  [  **     **  ] 
		0x30, 0xE0, 	//  [  **    ***  ] 
		0x1F, 0x70, 	//  [   ***** *** ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		
		// 'v' charwidth: 12
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x78, 0x70, 	//  [ ****    ***] 
		0x30, 0x20, 	//  [  **      * ] 
		0x30, 0x20, 	//  [  **      * ] 
		0x18, 0x40, 	//  [   **    *  ] 
		0x18, 0x40, 	//  [   **    *  ] 
		0x1C, 0x80, 	//  [   ***  *   ] 
		0x0C, 0x80, 	//  [    **  *   ] 
		0x0D, 0x00, 	//  [    ** *    ] 
		0x07, 0x00, 	//  [     ***    ] 
		0x06, 0x00, 	//  [     **     ] 
		0x02, 0x00, 	//  [      *     ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// 'w' charwidth: 17
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x78, 0xC7, 0x00, 	//  [ ****   **   *** ] 
		0x30, 0xC2, 0x00, 	//  [  **    **    *  ] 
		0x30, 0xC2, 0x00, 	//  [  **    **    *  ] 
		0x31, 0x62, 0x00, 	//  [  **   * **   *  ] 
		0x39, 0x62, 0x00, 	//  [  ***  * **   *  ] 
		0x1B, 0x74, 0x00, 	//  [   ** ** *** *   ] 
		0x1A, 0x34, 0x00, 	//  [   ** *   ** *   ] 
		0x1A, 0x34, 0x00, 	//  [   ** *   ** *   ] 
		0x0C, 0x18, 0x00, 	//  [    **     **    ] 
		0x0C, 0x18, 0x00, 	//  [    **     **    ] 
		0x0C, 0x18, 0x00, 	//  [    **     **    ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		0x00, 0x00, 0x00, 	//  [                 ] 
		
		// 'x' charwidth: 12
		0x00, 0x00,	//  [            ] 
		0x00, 0x00,	//  [            ] 
		0x00, 0x00,	//  [            ] 
		0x00, 0x00,	//  [            ] 
		0x78, 0xE0,	//  [ ****   *** ] 
		0x30, 0x40,	//  [  **     *  ] 
		0x18, 0x80,	//  [   **   *   ] 
		0x1D, 0x00,	//  [   *** *    ] 
		0x0E, 0x00,	//  [    ***     ] 
		0x06, 0x00,	//  [     **     ] 
		0x07, 0x00,	//  [     ***    ] 
		0x0B, 0x80,	//  [    * ***   ] 
		0x11, 0x80,	//  [   *   **   ] 
		0x20, 0xC0,	//  [  *     **  ] 
		0x71, 0xE0,	//  [ ***   **** ] 
		0x00, 0x00,	//  [            ] 
		0x00, 0x00,	//  [            ] 
		0x00, 0x00,	//  [            ] 
		
		// 'y' charwidth: 12
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x78, 0x70, 	//  [ ****    ***] 
		0x30, 0x20, 	//  [  **      * ] 
		0x30, 0x20, 	//  [  **      * ] 
		0x18, 0x40, 	//  [   **    *  ] 
		0x18, 0x40, 	//  [   **    *  ] 
		0x1C, 0x80, 	//  [   ***  *   ] 
		0x0C, 0x80, 	//  [    **  *   ] 
		0x0D, 0x00, 	//  [    ** *    ] 
		0x07, 0x00, 	//  [     ***    ] 
		0x06, 0x00, 	//  [     **     ] 
		0x02, 0x00, 	//  [      *     ] 
		0x04, 0x00, 	//  [     *      ] 
		0x0C, 0x00, 	//  [    **      ] 
		0x18, 0x00, 	//  [   **       ] 
		
		// 'z' charwidth: 12
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x3F, 0xE0, 	//  [  ********* ] 
		0x20, 0xE0, 	//  [  *     *** ] 
		0x00, 0xC0, 	//  [        **  ] 
		0x01, 0x80, 	//  [       **   ] 
		0x03, 0x00, 	//  [      **    ] 
		0x07, 0x00, 	//  [     ***    ] 
		0x06, 0x00, 	//  [     **     ] 
		0x0C, 0x00, 	//  [    **      ] 
		0x18, 0x00, 	//  [   **       ] 
		0x38, 0x20, 	//  [  ***     * ] 
		0x3F, 0xE0, 	//  [  ********* ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		0x00, 0x00, 	//  [            ] 
		
		// '{' charwidth: 7
		0x0C, 	//  [    ** ] 
		0x10, 	//  [   *   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x20, 	//  [  *    ] 
		0x40, 	//  [ *     ] 
		0x20, 	//  [  *    ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x30, 	//  [  **   ] 
		0x10, 	//  [   *   ] 
		0x0C, 	//  [    ** ] 
		0x00, 	//  [       ] 
		
		// '|' charwidth: 5
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		0x30, 	//  [  ** ] 
		
		// '}' charwidth: 7
		0x60, 	//  [ **    ] 
		0x10, 	//  [   *   ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x08, 	//  [    *  ] 
		0x04, 	//  [     * ] 
		0x08, 	//  [    *  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x18, 	//  [   **  ] 
		0x10, 	//  [   *   ] 
		0x60, 	//  [ **    ] 
		0x00, 	//  [       ] 
		
		// '~' charwidth: 13
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x3E, 0x10, 	//  [  *****    * ] 
		0x7F, 0xF0, 	//  [ *********** ] 
		0x03, 0xE0, 	//  [      *****  ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ] 
		0x00, 0x00, 	//  [             ]

    #if USER_SPECIAL_FIFTEEN_DOT > 0                                     // {4}
        #define USER_SPECIAL_FONT_FIFTEEN_DOT
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_FIFTEEN_DOT
    #endif
};

const unsigned short fifteen_dot_width_table[] = {                       // {2}
		 6,             // ' '
		 6,             // '!'
		 8,             // '"'
		13,             // '#'
		13 + (1 << 8),  // '$'
		14 + (2 << 8),  // '%'
		14 + (3 << 8),  // '&'
		 5 + (4 << 8),  // '''
		 7 + (4 << 8),  // '('
		 7 + (4 << 8),  // ')'
		10 + (4 << 8),  // '*'
		13 + (5 << 8),  // '+'
		 6 + (6 << 8),  // ','
		 7 + (6 << 8),  // '-'
		 6 + (6 << 8),  // '.'
		11 + (6 << 8),  // '/'
		13 + (7 << 8),  // '0'
		13 + (8 << 8),  // '1'
		13 + (9 << 8),  // '2'
		13 + (10 << 8), // '3'
		13 + (11 << 8), // '4'
		13 + (12 << 8), // '5'
		13 + (13 << 8), // '6'
		13 + (14 << 8), // '7'
		13 + (15 << 8), // '8'
		13 + (16 << 8), // '9'
		 6 + (17 << 8), // ':'
		 6 + (17 << 8), // ';'
		13 + (17 << 8), // '<'
		13 + (18 << 8), // '='
		13 + (19 << 8), // '>'
		10 + (20 << 8), // '?'
		18 + (21 << 8), // '@'
		14 + (23 << 8), // 'A'
		12 + (24 << 8), // 'B'
		14 + (25 << 8), // 'C'
		16 + (26 << 8), // 'D'
		12 + (27 << 8), // 'E'
		11 + (28 << 8), // 'F'
		15 + (29 << 8), // 'G'
		16 + (30 << 8), // 'H'
		 7 + (31 << 8), // 'I'
		 9 + (31 << 8), // 'J'
		14 + (32 << 8), // 'K'
		11 + (33 << 8), // 'L'
		17 + (34 << 8), // 'M'
		15 + (36 << 8), // 'N'
		16 + (37 << 8), // 'O'
		12 + (38 << 8), // 'P'
		16 + (39 << 8), // 'Q'
		14 + (40 << 8), // 'R'
		11 + (41 << 8), // 'S'
		14 + (42 << 8), // 'T'
		15 + (43 << 8), // 'U'
		14 + (44 << 8), // 'V'
		18 + (45 << 8), // 'W'
		13 + (47 << 8), // 'X'
		12 + (48 << 8), // 'Y'
		13 + (49 << 8), // 'Z'
		 7 + (50 << 8), // '['
		11 + (50 << 8), // '\'
		 7 + (51 << 8), // ']'
		13 + (51 << 8), // '^'
		10 + (52 << 8), // '_'
		13 + (53 << 8), // '`'
		11 + (54 << 8), // 'a'
		13 + (55 << 8), // 'b'
		11 + (56 << 8), // 'c'
		13 + (57 << 8), // 'd'
		11 + (58 << 8), // 'e'
		 8 + (59 << 8), // 'f'
		12 + (59 << 8), // 'g'
		13 + (60 << 8), // 'h'
		 7 + (61 << 8), // 'i'
		 7 + (61 << 8), // 'j'
		12 + (61 << 8), // 'k'
		 7 + (62 << 8), // 'l'
		19 + (62 << 8), // 'm'
		13 + (64 << 8), // 'n'
		13 + (65 << 8), // 'o'
		13 + (66 << 8), // 'p'
		13 + (67 << 8), // 'q'
		 9 + (68 << 8), // 'r'
		11 + (69 << 8), // 's'
		 8 + (70 << 8), // 't'
		13 + (70 << 8), // 'u'
		12 + (71 << 8), // 'v'
		17 + (72 << 8), // 'w'
		12 + (74 << 8), // 'x'
		12 + (75 << 8), // 'y'
		12 + (76 << 8), // 'z'
		 7 + (77 << 8), // '{'
		 5 + (77 << 8), // '|'
		 7 + (77 << 8), // '}'
		13 + (77 << 8), // '~'
    #if USER_SPECIAL_FIFTEEN_DOT > 0                                     // {4}
        #define USER_SPECIAL_FONT_FIFTEEN_DOT_WIDTH
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_FIFTEEN_DOT_WIDTH
    #endif
};
#endif

#if defined EN_EIGHTEEN_DOT
const unsigned char eighteen_dot_glyph_table[] = {
		// ' ' charwidth: 16
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		0x00, 0x00,	//  [      ] 
		
		// '!' charwidth: 8
		0x3F, 	//  [  ******] 
		0x3F, 	//  [  ******] 
		0x3F, 	//  [  ******] 
		0x3F, 	//  [  ******] 
		0x3F, 	//  [  ******] 
		0x3F, 	//  [  ******] 
		0x1E, 	//  [   **** ] 
		0x1E, 	//  [   **** ] 
		0x1E, 	//  [   **** ] 
		0x1E, 	//  [   **** ] 
		0x1E, 	//  [   **** ] 
		0x1E, 	//  [   **** ] 
		0x00, 	//  [        ] 
		0x1C, 	//  [   ***  ] 
		0x3E, 	//  [  ***** ] 
		0x3E, 	//  [  ***** ] 
		0x3E, 	//  [  ***** ] 
		0x1C, 	//  [   ***  ] 
		
		// '"' charwidth: 9
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x73, 0x80, 	//  [ ***  ***] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		0x00, 0x00, 	//  [         ] 
		
		// '#' charwidth: 21
		0x00, 0x38, 0xE0, 	//  [          ***   ***  ] 
		0x00, 0x70, 0xE0, 	//  [         ***    ***  ] 
		0x00, 0x71, 0xC0, 	//  [         ***   ***   ] 
		0x00, 0x71, 0xC0, 	//  [         ***   ***   ] 
		0x00, 0xE3, 0x80, 	//  [        ***   ***    ] 
		0x0F, 0xFF, 0xF8, 	//  [    *****************] 
		0x1F, 0xFF, 0xF8, 	//  [   ******************] 
		0x1F, 0xFF, 0xF0, 	//  [   ***************** ] 
		0x01, 0xC7, 0x00, 	//  [       ***   ***     ] 
		0x03, 0x87, 0x00, 	//  [      ***    ***     ] 
		0x03, 0x8E, 0x00, 	//  [      ***   ***      ] 
		0x3F, 0xFF, 0xE0, 	//  [  *****************  ] 
		0x3F, 0xFF, 0xE0, 	//  [  *****************  ] 
		0x7F, 0xFF, 0xC0, 	//  [ *****************   ] 
		0x07, 0x1C, 0x00, 	//  [     ***   ***       ] 
		0x0E, 0x1C, 0x00, 	//  [    ***    ***       ] 
		0x0E, 0x38, 0x00, 	//  [    ***   ***        ] 
		0x1C, 0x38, 0x00, 	//  [   ***    ***        ] 
		
		// '$' charwidth: 16
		0x01, 0x80, 	//  [       **       ] 
		0x01, 0x80, 	//  [       **       ] 
		0x0F, 0xE0, 	//  [    *******     ] 
		0x1F, 0xF0, 	//  [   *********    ] 
		0x3D, 0x90, 	//  [  **** **  *    ] 
		0x3D, 0x80, 	//  [  **** **       ] 
		0x3F, 0x80, 	//  [  *******       ] 
		0x1F, 0xE0, 	//  [   ********     ] 
		0x0F, 0xF0, 	//  [    ********    ] 
		0x03, 0xF8, 	//  [      *******   ] 
		0x01, 0xBC, 	//  [       ** ****  ] 
		0x01, 0xBC, 	//  [       ** ****  ] 
		0x21, 0xBC, 	//  [  *    ** ****  ] 
		0x31, 0xFC, 	//  [  **   *******  ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x1F, 0x80, 	//  [   ********     ] 
		0x01, 0xE0, 	//  [       **       ] 
		0x01, 0x80, 	//  [       **       ] 
		
		// '%' charwidth: 22
		0x1F, 0x01, 0xC0, 	//  [   *****       ***    ] 
		0x3F, 0x81, 0x80, 	//  [  *******      **     ] 
		0x71, 0xC3, 0x00, 	//  [ ***   ***    **      ] 
		0x71, 0xC7, 0x00, 	//  [ ***   ***   ***      ] 
		0x71, 0xC6, 0x00, 	//  [ ***   ***   **       ] 
		0x71, 0xCC, 0x00, 	//  [ ***   ***  **        ] 
		0x71, 0xDC, 0x00, 	//  [ ***   *** ***        ] 
		0x3F, 0x98, 0x00, 	//  [  *******  **         ] 
		0x1F, 0x30, 0x00, 	//  [   *****  **          ] 
		0x00, 0x33, 0xE0, 	//  [          **  *****   ] 
		0x00, 0x67, 0xF0, 	//  [         **  *******  ] 
		0x00, 0xCE, 0x38, 	//  [        **  ***   *** ] 
		0x00, 0xCE, 0x38, 	//  [        **  ***   *** ] 
		0x01, 0x8E, 0x38, 	//  [       **   ***   *** ] 
		0x03, 0x8E, 0x38, 	//  [      ***   ***   *** ] 
		0x03, 0x0E, 0x38, 	//  [      **    ***   *** ] 
		0x06, 0x07, 0xF0, 	//  [     **      *******  ] 
		0x0E, 0x03, 0xE0, 	//  [    ***       *****   ] 
		
		// '&' charwidth: 21
		0x07, 0xE0, 0x00, 	//  [     ******          ] 
		0x0F, 0xF0, 0x00, 	//  [    ********         ] 
		0x1F, 0xF8, 0x00, 	//  [   **********        ] 
		0x1F, 0x38, 0x00, 	//  [   *****  ***        ] 
		0x1F, 0x38, 0x00, 	//  [   *****  ***        ] 
		0x1F, 0xF8, 0x00, 	//  [   **********        ] 
		0x0F, 0xF0, 0x00, 	//  [    ********         ] 
		0x07, 0xE0, 0x00, 	//  [     ******          ] 
		0x0F, 0xF0, 0x00, 	//  [    ********         ] 
		0x1F, 0xF8, 0x00, 	//  [   **********        ] 
		0x3E, 0xFC, 0x00, 	//  [  ***** ******       ] 
		0x7C, 0x7F, 0x00, 	//  [ *****   *******     ] 
		0x7C, 0x3F, 0x80, 	//  [ *****    *******    ] 
		0x7C, 0x1F, 0xC0, 	//  [ *****     *******   ] 
		0x7E, 0x1F, 0xE0, 	//  [ ******    ********  ] 
		0x3F, 0xFF, 0xF8, 	//  [  *******************] 
		0x1F, 0xF3, 0xF8, 	//  [   *********  *******] 
		0x0F, 0xE0, 0xF8, 	//  [    *******     *****] 
		
		// ''' charwidth: 5
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x70, 	//  [ *** ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		0x00, 	//  [     ] 
		
		// '(' charwidth: 11
		0x01, 0x80, 	//  [       **  ] 
		0x03, 0x80, 	//  [      ***  ] 
		0x07, 0x00, 	//  [     ***   ] 
		0x07, 0x00, 	//  [     ***   ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x1E, 0x00, 	//  [   ****    ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x07, 0x00, 	//  [     ***   ] 
		0x07, 0x00, 	//  [     ***   ] 
		0x03, 0x80, 	//  [      ***  ] 
		0x01, 0x80, 	//  [       **  ] 
		
		// ')' charwidth: 11
		0x30, 0x00, 	//  [  **       ] 
		0x38, 0x00, 	//  [  ***      ] 
		0x1C, 0x00, 	//  [   ***     ] 
		0x1C, 0x00, 	//  [   ***     ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0F, 0x00, 	//  [    ****   ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x0E, 0x00, 	//  [    ***    ] 
		0x1C, 0x00, 	//  [   ***     ] 
		0x1C, 0x00, 	//  [   ***     ] 
		0x38, 0x00, 	//  [  ***      ] 
		0x30, 0x00, 	//  [  **       ] 
		
		// '*' charwidth: 16
		0x03, 0xC0, 	//  [      ****      ] 
		0x09, 0xA0, 	//  [    *  ** *     ] 
		0x0D, 0xB0, 	//  [    ** ** **    ] 
		0x1F, 0x78, 	//  [   ***** ****   ] 
		0x03, 0xC0, 	//  [      ****      ] 
		0x03, 0xC0, 	//  [      ****      ] 
		0x1F, 0x78, 	//  [   ***** ****   ] 
		0x0D, 0xB0, 	//  [    ** ** **    ] 
		0x09, 0xA0, 	//  [    *  ** *     ] 
		0x03, 0xC0, 	//  [      ****      ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		0x00, 0x00, 	//  [                ] 
		
		// '+' charwidth: 22
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x1F, 0xFF, 0xF0, 	//  [   *****************  ] 
		0x1F, 0xFF, 0xF0, 	//  [   *****************  ] 
		0x1F, 0xFF, 0xF0, 	//  [   *****************  ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x38, 0x00, 	//  [          ***         ] 
		0x00, 0x00, 0x00, 	//  [                      ] 
		
		// ',' charwidth: 8
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x38, 	//  [  ***   ] 
		0x7C, 	//  [ *****  ] 
		0x7C, 	//  [ *****  ] 
		0x7C, 	//  [ *****  ] 
		0x3C, 	//  [  ****  ] 
		0x18, 	//  [   **   ] 
		0x30, 	//  [  **    ] 
		0x60, 	//  [ **     ] 
		
		// '-' charwidth: 11
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x1F, 0xC0, 	//  [   ******* ] 
		0x3F, 0x80, 	//  [  *******  ] 
		0x7F, 0x00, 	//  [ *******   ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		0x00, 0x00, 	//  [           ] 
		
		// '.' charwidth: 8
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x00, 	//  [        ] 
		0x38, 	//  [  ***   ] 
		0x7C, 	//  [ *****  ] 
		0x7C, 	//  [ *****  ] 
		0x7C, 	//  [ *****  ] 
		0x38, 	//  [  ***   ] 
		
		// '/' charwidth: 8
		0x06, 	//  [     ** ] 
		0x06, 	//  [     ** ] 
		0x0E, 	//  [    *** ] 
		0x0C, 	//  [    **  ] 
		0x0C, 	//  [    **  ] 
		0x1C, 	//  [   ***  ] 
		0x18, 	//  [   **   ] 
		0x18, 	//  [   **   ] 
		0x38, 	//  [  ***   ] 
		0x30, 	//  [  **    ] 
		0x30, 	//  [  **    ] 
		0x70, 	//  [ ***    ] 
		0x60, 	//  [ **     ] 
		0x60, 	//  [ **     ] 
		0xE0, 	//  [***     ] 
		0xC0, 	//  [**      ] 
		0xC0, 	//  [**      ] 
		0xC0, 	//  [**      ] 
		
		// '0' charwidth: 16
		0x03, 0xE0, 	//  [      *****     ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x3E, 0x3E, 	//  [  *****   ***** ] 
		0x3E, 0x3E, 	//  [  *****   ***** ] 
		0x3C, 0x1E, 	//  [  ****     **** ] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x7C, 0x1F, 	//  [ *****     *****] 
		0x3C, 0x1F, 	//  [  ****     *****] 
		0x3E, 0x3E, 	//  [  *****   ***** ] 
		0x3E, 0x3E, 	//  [  *****   ***** ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		0x03, 0xE0, 	//  [      *****     ] 
		
		// '1' charwidth: 16
		0x00, 0x20, 	//  [          *     ] 
		0x00, 0x60, 	//  [         **     ] 
		0x00, 0xE0, 	//  [        ***     ] 
		0x07, 0xE0, 	//  [     ******     ] 
		0x1F, 0xE0, 	//  [   ********     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		
		// '2' charwidth: 16
		0x07, 0xE0, 	//  [     ******     ] 
		0x1F, 0xF8, 	//  [   **********   ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x38, 0xFC, 	//  [  ***   ******  ] 
		0x20, 0x7C, 	//  [  *      *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0xFC, 	//  [        ******  ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x01, 0xF8, 	//  [       ******   ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x07, 0xE0, 	//  [     ******     ] 
		0x07, 0xC0, 	//  [     *****      ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x1F, 0x82, 	//  [   ******     * ] 
		0x1F, 0xFE, 	//  [   ************ ] 
		0x3F, 0xFC, 	//  [  ************  ] 
		0x7F, 0xFC, 	//  [ *************  ] 
		
		// '3' charwidth: 16
		0x1F, 0xC0, 	//  [   *******      ] 
		0x3F, 0xF0, 	//  [  **********    ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x21, 0xF8, 	//  [  *    ******   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x1F, 0xE0, 	//  [   ********     ] 
		0x1F, 0xF0, 	//  [   *********    ] 
		0x1F, 0xF8, 	//  [   **********   ] 
		0x00, 0xFC, 	//  [        ******  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x7F, 0xF8, 	//  [ ************   ] 
		0x3F, 0xF0, 	//  [  **********    ] 
		0x3F, 0xC0, 	//  [  ********      ] 
		
		// '4' charwidth: 16
		0x01, 0xF8, 	//  [       ******   ] 
		0x01, 0xF8, 	//  [       ******   ] 
		0x03, 0xF8, 	//  [      *******   ] 
		0x03, 0xF8, 	//  [      *******   ] 
		0x06, 0xF8, 	//  [     ** *****   ] 
		0x06, 0xF8, 	//  [     ** *****   ] 
		0x0C, 0xF8, 	//  [    **  *****   ] 
		0x1C, 0xF8, 	//  [   ***  *****   ] 
		0x18, 0xF8, 	//  [   **   *****   ] 
		0x38, 0xF8, 	//  [  ***   *****   ] 
		0x30, 0xF9, 	//  [  **    *****  *] 
		0x7F, 0xFF, 	//  [ ***************] 
		0x7F, 0xFE, 	//  [ ************** ] 
		0xFF, 0xFE, 	//  [*************** ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x01, 0xFC, 	//  [       *******  ] 
		
		// '5' charwidth: 16
		0x1F, 0xF8, 	//  [   **********   ] 
		0x1F, 0xF8, 	//  [   **********   ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x3E, 0x08, 	//  [  *****     *   ] 
		0x3E, 0x00, 	//  [  *****         ] 
		0x3E, 0x00, 	//  [  *****         ] 
		0x3F, 0xE0, 	//  [  *********     ] 
		0x3F, 0xF0, 	//  [  **********    ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x21, 0xFC, 	//  [  *    *******  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x01, 0xF8, 	//  [       ******   ] 
		0x7F, 0xF8, 	//  [ ************   ] 
		0x3F, 0xF0, 	//  [  **********    ] 
		0x1F, 0xC0, 	//  [   *******      ] 
		
		// '6' charwidth: 16
		0x00, 0x78, 	//  [         ****   ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x07, 0xE0, 	//  [     ******     ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x1F, 0x00, 	//  [   *****        ] 
		0x3E, 0x00, 	//  [  *****         ] 
		0x3E, 0xF0, 	//  [  ***** ****    ] 
		0x3D, 0xFC, 	//  [  **** *******  ] 
		0x7D, 0xFC, 	//  [ ***** *******  ] 
		0x7C, 0x7E, 	//  [ *****   ****** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x3E, 0x7C, 	//  [  *****  *****  ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		0x07, 0xE0, 	//  [     ******     ] 
		
		// '7' charwidth: 16
		0x3F, 0xFE, 	//  [  ************* ] 
		0x3F, 0xFC, 	//  [  ************  ] 
		0x7F, 0xFC, 	//  [ *************  ] 
		0x40, 0x7C, 	//  [ *       *****  ] 
		0x00, 0x78, 	//  [         ****   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x03, 0xE0, 	//  [      *****     ] 
		0x07, 0xC0, 	//  [     *****      ] 
		0x07, 0xC0, 	//  [     *****      ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x1F, 0xC0, 	//  [   *******      ] 
		
		// '8' charwidth: 16
		0x07, 0xF0, 	//  [     *******    ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x3E, 0x3E, 	//  [  *****   ***** ] 
		0x3E, 0x1E, 	//  [  *****    **** ] 
		0x3E, 0x1E, 	//  [  *****    **** ] 
		0x3F, 0x9C, 	//  [  *******  ***  ] 
		0x1F, 0xF8, 	//  [   **********   ] 
		0x0F, 0xF8, 	//  [    *********   ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x3C, 0xFE, 	//  [  ****  ******* ] 
		0x78, 0x3F, 	//  [ ****     ******] 
		0x78, 0x1F, 	//  [ ****      *****] 
		0x78, 0x1F, 	//  [ ****      *****] 
		0x7C, 0x3F, 	//  [ *****    ******] 
		0x3F, 0xFE, 	//  [  ************* ] 
		0x1F, 0xFC, 	//  [   ***********  ] 
		0x07, 0xF0, 	//  [     *******    ] 
		
		// '9' charwidth: 16
		0x07, 0xE0, 	//  [     ******     ] 
		0x1F, 0xF0, 	//  [   *********    ] 
		0x3F, 0xF8, 	//  [  ***********   ] 
		0x3E, 0x7C, 	//  [  *****  *****  ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7C, 0x3E, 	//  [ *****    ***** ] 
		0x7E, 0x3E, 	//  [ ******   ***** ] 
		0x3F, 0xBE, 	//  [  ******* ***** ] 
		0x3F, 0xBE, 	//  [  ******* ***** ] 
		0x0F, 0x7C, 	//  [    **** *****  ] 
		0x00, 0x7C, 	//  [         *****  ] 
		0x00, 0xF8, 	//  [        *****   ] 
		0x01, 0xF0, 	//  [       *****    ] 
		0x07, 0xE0, 	//  [     ******     ] 
		0x0F, 0x80, 	//  [    *****       ] 
		0x1E, 0x00, 	//  [   ****         ]

    #if USER_SPECIAL_EIGHTEEN_DOT > 0                                     // {4}
        #define USER_SPECIAL_FONT_EIGHTEEN_DOT
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_EIGHTEEN_DOT
    #endif
};


const unsigned short eighteen_dot_width_table[] = {                      // {2}
    	16 + (0 << 8),             // ' '
		 8 + (1 << 8),  // '!'
		 9 + (1 << 8),  // '"'
		21 + (2 << 8),  // '#'
		16 + (4 << 8),  // '$'
		22 + (5 << 8),  // '%'
		21 + (7 << 8),  // '&'
		 5 + (9 << 8),  // '''
		11 + (9 << 8),  // '('
		11 + (10 << 8), // ')'
		16 + (11 << 8), // '*'
		22 + (12 << 8), // '+'
		 8 + (14 << 8), // ','
		11 + (14 << 8), // '-'
		 8 + (15 << 8), // '.'
		 8 + (15 << 8), // '/'
		16 + (15 << 8), // '0'
		16 + (16 << 8), // '1'
		16 + (17 << 8), // '2'
		16 + (18 << 8), // '3'
		16 + (19 << 8), // '4'
		16 + (20 << 8), // '5'
		16 + (21 << 8), // '6'
		16 + (22 << 8), // '7'
		16 + (23 << 8), // '8'
		16 + (24 << 8), // '9'
    #if USER_SPECIAL_EIGHTEEN_DOT > 0                                     // {4}
        #define USER_SPECIAL_FONT_EIGHTEEN_DOT_WIDTH
        #include "user_glcd_fonts.h"
        #undef USER_SPECIAL_FONT_EIGHTEEN_DOT_WIDTH
    #endif
};
#endif

