/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      glcd_cglcd.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    13.05.2010 Add KITRONIX_GLCD_MODE                                    {1}
    26.06.2010 Add MB785_GLCD_MODE                                       {2}
    09.08.2010 Add CGLCD_PIXEL_SIZE to control effective size            {3}
    29.08.2010 Add touch screen operation                                {4}
    09.11.2010 Change EIB to EBI (AVR32 external bus interface naming)
    19.02.2017 Add FT800                                                 {5}
    06.07.2019 Add ILI9341_GLCD_MODE mode                                {6}
    02.10.2019 Flush timeout events when waiting for display to become ready {7}
    29.10.2020 Correct X_BYTES and Y_BYTES dimensions to operate with various pixel sizes {9}

*/

#if (defined CGLCD_GLCD_MODE || defined KITRONIX_GLCD_MODE || defined MB785_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE) && !defined _GLCD_SAMSUNG && !defined OLED_GLCD_MODE && !defined TFT_GLCD_MODE && !defined NOKIA_GLCD_MODE
    #if !defined _GLCD_CGLCD_DEFINES

        #define MAX_GLCD_READY              100

        #define X_BYTES	                    ((((GLCD_X + (CGLCD_PIXEL_SIZE - 1))/CGLCD_PIXEL_SIZE) + 7)/8) // {3}{8} the number of bytes holding the X-pixels
        #define Y_BYTES	                    ((GLCD_Y + (CGLCD_PIXEL_SIZE - 1))/CGLCD_PIXEL_SIZE) // {3}{8} the number of rows of X_BYTES holding the Y rows
        #define DISPLAY_LEFT_PIXEL          0
        #define DISPLAY_TOP_PIXEL           0
        #define DISPLAY_RIGHT_PIXEL         (GLCD_X - 1)
        #define DISPLAY_BOTTOM_PIXEL        (GLCD_Y - 1)
        #define UPDATE_WIDTH                ((X_BYTES + 7)/8)
        #define UPDATE_HEIGHT               Y_BYTES

        #define SET_ADDRESS_POINTER         0x24
        #define SET_TEXT_HOME_ADDRESS       0x40
        #define SET_TEXT_HORIZ_BYTES        0x41
        #define SET_GRAPHICS_HOME_ADDRESS   0x42
        #define SET_GRAPHICS_HORIZ_BYTES    0x43
        #define GRAPHIC_MODE_XOR            0x81
        #define DISABLE_TEXT_DISABLE_CURSOR 0x98

        #define WRITE_DATA_INC              0xc0

        #if defined FT800_GLCD_MODE
            #if defined BRIDGETEK_GLIB_
                #define GLCD_BUSY()         (ft800_host.CmdWaiting != 0)
            #else
                #define NO_YIELD            0
                #define FIFO_WRITE_YIELD    1
                #define SWAP_YIELD          2
                #define GLCD_BUSY()         (((ft800_host.iCoProcessorWait == FIFO_WRITE_YIELD) && ((Ft_Gpu_Hal_Rd16(0, REG_CMD_READ) != Ft_Gpu_Hal_Rd16(0, REG_CMD_WRITE)))) || ((ft800_host.iCoProcessorWait == SWAP_YIELD) && ((Ft_Gpu_Hal_Rd8(0, REG_DLSWAP) != DLSWAP_DONE))))
            #endif
        #else
            #define GLCD_BUSY()             0
        #endif

        #define NWE_SETUP                   20                           // NWE setup time in ns
        #define NCS_WR_SETUP                0                            // NCS write setup time in ns
        #define NRD_SETUP                   30                           // NRD setup time in ns
        #define NCS_RD_SETUP                0                            // NCS read setup time in ns
        #define NWE_PULSE                   60                           // NWE pulse time in ns
        #define NCS_WR_PULSE                90                           // NCS write pulse time in ns
        #define NRD_PULSE                   210                          // NRD pulse time in ns
        #define NCS_RD_PULSE                240                          // NCS read pulse time in ns
        #define NCS_WR_HOLD                 10                           // NCS write hold time in ns
        #define NWE_HOLD                    20                           // NWE hold time in ns
        #define NCS_RD_HOLD                 30                           // NCS read hold time in ns
        #define NRD_HOLD                    30                           // NRDhold time in ns

        #define Max(a, b)                   (((a) > (b)) ?  (a) : (b))

        #define NWE_CYCLE                   Max((NCS_WR_SETUP + NCS_WR_PULSE + NCS_WR_HOLD),(NWE_SETUP + NWE_PULSE + NWE_HOLD)) // write cycle time in ns
        #define NRD_CYCLE                   Max((NCS_RD_SETUP + NCS_RD_PULSE + NCS_RD_HOLD),(NRD_SETUP + NRD_PULSE + NRD_HOLD)) // write cycle time in ns

        #define NCS_CONTROLLED_READ         0                            // read controlled by NRD and not NCS
        #define NCS_CONTROLLED_WRITE        0                            // write controlled by NWE and not NCS
        #define NWAIT_MODE                  NWAIT_MODE_DISABLED          // NWAIT pin not used
        #define SMC_8_BIT_CHIPS             0                            // not 8-bit SM chips on SMC
        #define SMC_DBW                     16                           // SMC data bus width
        #define TDF_CYCLES                  0                            // data float time
        #define TDF_OPTIM                   0
        #define PAGE_MODE                   0
        #define PAGE_SIZE                   0

        #if defined AVR32_EVK1105
            static void SMC_CS_SETUP(int ncs, unsigned long hsb_mhz_up);
        #elif defined EK_LM3S3748
            static void fnWriteGLCD_cmd(unsigned char ucByte);
            static void fnCommandGlcd_1(unsigned char ucCommand, unsigned char ucParameter);
            static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter);
            static void fnWriteGLCD_data_pair(unsigned short usBytePair);
        #elif defined KITRONIX_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined FT800_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
            #if defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
                static void fnCommandGlcd_1(unsigned char ucCommand);
                static void fnDataGlcd_1(unsigned char ucData);
                static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usData1, unsigned short usData2);
                static void fnWriteGLCD_data_pair(unsigned short usData);
            #elif defined TFT2N0369_GLCD_MODE
                #if defined _WINDOWS
	                    #define fnCommandGlcd_1(command) CollectCommand(1, (command))
                        #define fnCommandGlcd_2(command, data) CollectCommand(1, (command)); CollectCommand(0, (data))
                        #define fnWriteGLCD_data_pair(data) CollectCommand(0, (data));

                #else
                        #define fnCommandGlcd_1(command)       *((volatile unsigned short*)GLCD_COMMAND_ADDR) = (command)
                        #define fnCommandGlcd_2(command, data) *((volatile unsigned short*)GLCD_COMMAND_ADDR) = (command); *((volatile unsigned short*)GLCD_DATA_ADDR) = (data)
                        #define fnWriteGLCD_data_pair(data)    *((volatile unsigned short*)GLCD_DATA_ADDR) = (data)
                #endif
            #elif defined FT800_GLCD_MODE                                // {5}
    	        #define fnCommandGlcd_1(command)                         // dummy for compatibility
                #define fnCommandGlcd_2(command, data)
                #define fnWriteGLCD_data_pair(data)
            #else
                static void fnCommandGlcd_1(unsigned char ucCommand);
                static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter);
                static void fnWriteGLCD_data_pair(unsigned short usBytePair);
            #endif
            #if defined SUPPORT_TOUCH_SCREEN
                static void fnStartTouch(void);
            #endif
        #elif defined  MB785_GLCD_MODE
            static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter);
            #if defined SUPPORT_TOUCH_SCREEN
                static void fnStartTouch(void);
            #endif
        #endif
        static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
        #define START_OSCILLATION               0x00
        #define DRIVE_OUTPUT_CONTROL_1          0x01
        #define LCD_DRIVING_CONTROL             0x02
        #define ENTRY_MODE                      0x03
        #define RESIZE_CONTROL                  0x04
        #define DISPLAY_CONTROL_1               0x07
        #define DISPLAY_CONTROL_2               0x08
        #define DISPLAY_CONTROL_3               0x09
        #define DISPLAY_CONTROL_4               0x0a
        #define RGB_DISPLAY_INTERFACE_CONTROL_1 0x0c
        #define FRAME_MARKER_POSITION           0x0d
        #define RGB_DISPLAY_INTERFACE_CONTROL_2 0x0f
        #define POWER_CONTROL_1                 0x10
        #define POWER_CONTROL_2                 0x11
        #define POWER_CONTROL_3                 0x12
        #define POWER_CONTROL_4                 0x13
        #define GAMMA_CONTROL_1                 0x30
        #define GAMMA_CONTROL_2                 0x31
        #define GAMMA_CONTROL_3                 0x32
        #define GAMMA_CONTROL_4                 0x35
        #define GAMMA_CONTROL_5                 0x36
        #define GAMMA_CONTROL_6                 0x37
        #define GAMMA_CONTROL_7                 0x38
        #define GAMMA_CONTROL_8                 0x39
        #define GAMMA_CONTROL_9                 0x3c
        #define GAMMA_CONTROL_10                0x3d

        #define START_BYTE                      0x70
        #define SET_INDEX                       0x00
        #define READ_STATUS                     0x01
        #define LCD_WRITE_REG                   0x02
        #define LCD_READ_REG                    0x03

        #define HIMAX_DISPMODECTRL              0x01
          #define HIMAX_INVON                   0x04
        #define HIMAX_MEMACCESSCTRL             0x16
          #define HIMAX_MY                      0x80
          #define HIMAX_MX                      0x40
          #define HIMAX_MV                      0x20
          #define HIMAX_BGR                     0x08
        #define HIMAX_OSCCTRL1                  0x19
          #define HIMAX_OSC_EN                  0x01
        #define HIMAX_POWERCTRL1                0x1b
          #define HIMAX_GASENB                  0x80
          #define HIMAX_PON                     0x10
          #define HIMAX_DK                      0x08
          #define HIMAX_XDK                     0x04
          #define HIMAX_VTFT_TRI                0x02
          #define HIMAX_STB                     0x01
        #define HIMAX_POWERCTRL2                0x1c
          #define HIMAX_AP2                     0x04
          #define HIMAX_AP1                     0x02
          #define HIMAX_AP0                     0x01
        #define HIMAX_DISPCTRL1                 0x26
          #define HIMAX_PT1                     0x80
          #define HIMAX_PT0                     0x40
          #define HIMAX_GON                     0x20
          #define HIMAX_DTE                     0x10
          #define HIMAX_D1                      0x08
          #define HIMAX_D0                      0x04
        #define HIMAX_CYCLECTRL5                0x3e
        #define HIMAX_VCOMCTRL1                 0x43
          #define HIMAX_VCOMG                   0x80
        #define HIMAX_GAMMACTRL1                0x46
        #define HIMAX_GAMMACTRL2                0x47
        #define HIMAX_GAMMACTRL3                0x48
        #define HIMAX_GAMMACTRL4                0x49
        #define HIMAX_GAMMACTRL5                0x4a
        #define HIMAX_GAMMACTRL6                0x4b
        #define HIMAX_GAMMACTRL7                0x4c
        #define HIMAX_GAMMACTRL8                0x4d
        #define HIMAX_GAMMACTRL9                0x4e
        #define HIMAX_GAMMACTRL10               0x4f
        #define HIMAX_GAMMACTRL11               0x50
        #define HIMAX_GAMMACTRL12               0x51
        #define HIMAX_INTERNAL28                0x70
          #define HIMAX_TEON                    0x08
        #define HIMAX_SAP                       0x90

        #define HIMAX_COL_ADDR_START2           0x02
        #define HIMAX_COL_ADDR_START1           0x03
        #define HIMAX_COL_ADDR_END2             0x04
        #define HIMAX_COL_ADDR_END1             0x05
        #define HIMAX_ROW_ADDR_START2           0x06
        #define HIMAX_ROW_ADDR_START1           0x07
        #define HIMAX_ROW_ADDR_END2             0x08
        #define HIMAX_ROW_ADDR_END1             0x09
        #define HIMAX_PA_START_ROW_HB           0x0A
        #define HIMAX_PA_START_ROW_LB           0x0B
        #define HIMAX_PA_END_ROW_HB             0x0C
        #define HIMAX_PA_END_ROW_LB             0x0D
        #define HIMAX_VS_TFA_HB                 0x0E
        #define HIMAX_VS_TFA_LB                 0x0F
        #define HIMAX_VS_VSA_HB                 0x10
        #define HIMAX_VS_VSA_LB                 0x11
        #define HIMAX_VS_BFA_HB                 0x12
        #define HIMAX_VS_BFA_LB                 0x13
        #define HIMAX_VS_VSP_HB                 0x14
        #define HIMAX_VS_VSP_LB                 0x15
        #define HIMAX_SRAMWRITE                 0x22

        #if defined _WINDOWS
            unsigned short usLCD_interface[2];                                   // dummy locations for running with the simulator
            #define AVR32_EBI_CS0_ADDRESS       usLCD_interface
            #define ET024006_CMD_ADDR           ((volatile unsigned short *) AVR32_EBI_CS0_ADDRESS)
            #define ET024006_PARAM_ADDR         ((volatile unsigned short *) (AVR32_EBI_CS0_ADDRESS + 1))
            #define WRITE_REG(add, value)       *ET024006_CMD_ADDR = add; CollectCommand(1, *ET024006_CMD_ADDR); *ET024006_PARAM_ADDR = value; 	CollectCommand(0, *ET024006_PARAM_ADDR)
            #define SELECT_REG(add)             *ET024006_CMD_ADDR = add; CollectCommand(1, *ET024006_CMD_ADDR)
            #define WRITE_DATA(value)           *ET024006_PARAM_ADDR = value; CollectCommand(0, *ET024006_PARAM_ADDR);
        #else
            #define AVR32_EBI_CS0_ADDRESS       0xC0000000
            #define ET024006_CMD_ADDR           ((volatile unsigned short *) AVR32_EBI_CS0_ADDRESS)
            #define ET024006_PARAM_ADDR         ((volatile unsigned short *) (AVR32_EBI_CS0_ADDRESS + 0x00200000))
            #define WRITE_REG(add, value)       *ET024006_CMD_ADDR = add; *ET024006_PARAM_ADDR = value
            #define SELECT_REG(add)             *ET024006_CMD_ADDR = add
            #define WRITE_DATA(value)           *ET024006_PARAM_ADDR = value
        #endif
        #define ST7637_RESET                    0x01
        #define ST7637_SLPOUT                   0x11
        #define ST7637_NORMAL_ON                0x13
        #define ST7637_DISPLAY_OFF              0x28
        #define ST7637_COLMODE                  0x3a
        #define ST7637_TEON                     0x35
        #define ST7637_MADCTR                   0x36
        #define ST7637_DISPLAY_ON               0x29
        #define ST7637_COL_ADDR_SET             0x2a
        #define ST7637_ROW_ADDR_SET             0x2b
        #define ST7637_MEMORY_WRITE             0x2c
        #define ST7637_COMSCANDIR               0xb7
        #define ST7637_AUTOLOAD_SET             0xd7
        #define ST7637_READ_OTP                 0xe3

        #if defined ST7789S_GLCD_MODE
            #define ST7789_WRITE_MEM_CONTINUE   0x3c
            #define ST7789_PORCH_CONTROL        0xb2
            #define ST7789_FRAME_RATE_CONTROL   0xb3
            #define ST7789_GATE_CONTROL         0xb7
            #define ST7789_VCOM_SETTING         0xbb
            #define ST7789_LCM_CONTROL          0xc0
            #define ST7789_VDV_VRH_ENABLE       0xc2
            #define ST7789_VRH_SET              0xc3
            #define ST7789_VDV_SET              0xc4
            #define ST7789_FRAME_RATE_NORMAL_MODE 0xc6
            #define ST7789_POWER_CONTROL_1      0xd0
            #define ST7789_POSITIVE_GAMMA       0xe0
            #define ST7789_NEGATIVE_GAMMA       0xe1
        #elif defined ILI9341_GLCD_MODE
            #define ILI9341_POWER_CONTROL_1     0xc0
            #define ILI9341_POWER_CONTROL_2     0xc1
            #define ILI9341_VCOM_CONTROL_1      0xc5
            #define ILI9341_VCOM_CONTROL_2      0xc7
            #define ILI9341_MEMORY_ACCESS_CONTROL     0x36
            #define ILI9341_FRAME_RATE_CONTROL_NORMAL 0xb1
            #define ILI9341_DISPLAY_FUNCTION_CONTROL  0xb6
            #define ILI9341_GAMMA_SET           0x26
            #define ILI9341_PIXEL_FORMAT_SET    0x3a
            #define ILI9341_DISPLAY_INVERSION_ON      0x21
            #define ILI9341_POSITIVE_GAMMA_CORRECTION 0xe0
            #define ILI9341_NEGATIVE_GAMMA_CORRECTION 0xe1
            #define ILI9341_SLEEP_OUT           0x11
            #define ILI9341_COLUMN_ADDRESS_SET  0x2a
            #define ILI9341_PAGE_ADDRESS_SET    0x2b
            #define ILI9341_DISPLAY_ON          0x29
            #define ILI9341_MEMORY_WRITE        0x2c
        #else
            #define ST7637_VOP_SET              0xc0
            #define ST7637_BIAS_SET             0xc3
            #define ST7637_BOOSTER_SET          0xc4
            #define ST7637_OTP_READ_WRITE       0xe0
            #define ST7637_CONTROL_CANCEL       0xe1
        #endif

        #define SSD2119_DEVICE_CODE_READ_REG    0x00
        #define SSD2119_OSC_START_REG           0x00
            #define SSD2119_OSCEN               0x0001
        #define SSD2119_OUTPUT_CTRL_REG         0x01
            #define SSD2119_RL                  0x4000
            #define SSD2119_REVERSE_DISPLAY     0x2000
            #define SSD2119_FLIP_DISPLAY        0x1000
            #define SSD2119_BGR                 0x0800                   // color display BGR rather than RGB
            #define SSD2119_SM                  0x0400
            #define SSD2119_TB                  0x0200
            #define SSD2119_MUX_MASK            0x00ff
        #define SSD2119_LCD_DRIVE_AC_CTRL_REG   0x02
            #define SSD2119_FLD                 0x1000
            #define SSD2119_ENWS                0x0800
            #define SSD2119_B_C                 0x0400
            #define SSD2119_EOR                 0x0200
            #define SSD2119_WSMD                0x0100
            #define SSD2119_NWMASK              0x00ff
        #define SSD2119_PWR_CTRL_1_REG          0x03
            #define SSD2119_DCT_FLINE_4         0x6000
            #define SSD2119_BT_5_3              0x0a00
            #define SSD2119_DC_FLINE_4          0x0060
            #define SSD2119_AP_SMALL_TO_MED     0x0004
        #define SSD2119_DISPLAY_CTRL_REG        0x07
        #define SSD2119_FRAME_CYCLE_CTRL_REG    0x0b
        #define SSD2119_PWR_CTRL_2_REG          0x0c
            #define SSD2119_VCIX2_5_1           0x0000
            #define SSD2119_VCIX2_5_3           0x0001
            #define SSD2119_VCIX2_5_5           0x0002
            #define SSD2119_VCIX2_5_7           0x0003
            #define SSD2119_VCIX2_5_9           0x0004
            #define SSD2119_VCIX2_6_1           0x0005
        #define SSD2119_PWR_CTRL_3_REG          0x0d
            #define SSD2119_VLCD63_2_810        0x0000
            #define SSD2119_VLCD63_2_900        0x0001
            #define SSD2119_VLCD63_3_000        0x0002
            #define SSD2119_VLCD63_1_780        0x0003
            #define SSD2119_VLCD63_1_850        0x0004
            #define SSD2119_VLCD63_1_930        0x0005
            #define SSD2119_VLCD63_2_020        0x0006
            #define SSD2119_VLCD63_2_090        0x0007
            #define SSD2119_VLCD63_2_165        0x0008
            #define SSD2119_VLCD63_2_245        0x0009
            #define SSD2119_VLCD63_2_335        0x000a
            #define SSD2119_VLCD63_2_400        0x000b
            #define SSD2119_VLCD63_2_500        0x000c
            #define SSD2119_VLCD63_2_570        0x000d
            #define SSD2119_VLCD63_2_645        0x000e
            #define SSD2119_VLCD63_2_725        0x000f
        #define SSD2119_PWR_CTRL_4_REG          0x0e
            #define SSD2119_VCOMG               0x2000
            #define SSD2119_VCOM_0_60           0x0000
            #define SSD2119_VCOM_0_63           0x0100
            #define SSD2119_VCOM_0_66           0x0200
            #define SSD2119_VCOM_0_69           0x0300
            #define SSD2119_VCOM_0_72           0x0400
            #define SSD2119_VCOM_0_75           0x0500
            #define SSD2119_VCOM_0_78           0x0600
            #define SSD2119_VCOM_0_81           0x0700
            #define SSD2119_VCOM_0_84           0x0800
            #define SSD2119_VCOM_0_87           0x0900
            #define SSD2119_VCOM_0_90           0x0a00
            #define SSD2119_VCOM_0_93           0x0b00
            #define SSD2119_VCOM_0_96           0x0c00
            #define SSD2119_VCOM_0_99           0x0d00
            #define SSD2119_VCOM_1_02           0x0e00
            #define SSD2119_VCOM_EXT_RES        0x0f00
            #define SSD2119_VCOM_1_05           0x1000
            #define SSD2119_VCOM_1_08           0x1100
            #define SSD2119_VCOM_1_11           0x1200
            #define SSD2119_VCOM_1_14           0x1300
            #define SSD2119_VCOM_1_17           0x1400
            #define SSD2119_VCOM_1_20           0x1500
            #define SSD2119_VCOM_1_23           0x1600
        #define SSD2119_GATE_SCAN_START_REG     0x0f
        #define SSD2119_SLEEP_MODE_1_REG        0x10
            #define SSD2119_SL                  0x0001
        #define SSD2119_ENTRY_MODE_REG          0x11
            #define SSD2119_VSMODE              0x8000
            #define SSD2119_DFM_65K             0x6000
            #define SSD2119_DFM_262K            0x4000
            #define SSD2119_DENMODE             0x0800
            #define SSD2119_WMODE               0x0400
            #define SSD2119_NOSYNC              0x0200
            #define SSD2119_DMODE               0x0100
            #define SSD2119_TYPE_A              0x0000
            #define SSD2119_TYPE_B              0x0040
            #define SSD2119_TYPE_C              0x0080
            #define SSD2119_ID_00               0x0000
            #define SSD2119_ID_01               0x0010
            #define SSD2119_ID_10               0x0020
            #define SSD2119_ID_11               0x0030
            #define SSD2119_AM                  0x0008
        #define SSD2119_SLEEP_MODE_2_REG        0x12
            #define SSD2119_DSLP                0x2000
            #define SSD2119_CORE_VOLTAGE_1_3    0x0199
            #define SSD2119_CORE_VOLTAGE_1_4    0x0399
            #define SSD2119_CORE_VOLTAGE_1_5    0x0599
            #define SSD2119_CORE_VOLTAGE_1_6    0x0799
            #define SSD2119_CORE_VOLTAGE_1_7    0x0999
            #define SSD2119_CORE_VOLTAGE_1_8    0x0b99
            #define SSD2119_CORE_VOLTAGE_1_9    0x0d99
            #define SSD2119_CORE_VOLTAGE_2_0    0x0f99
        #define SSD2119_GEN_IF_CTRL_REG         0x15
        #define SSD2119_HORIZ_PORCH_REG         0x16
        #define SSD2119_VERT_PORCH_REG          0x17
        #define SSD2119_PWR_CTRL_5_REG          0x1e
            #define SSD2119_NOTP                0x0080
            #define SSD2119_VCM_MASK            0x003f
        #define SSD2119_UNIFORMITY_REG          0x20
            #define SSD2119_ENSVIN              0xb0eb
        #define SSD2119_RAM_DATA_REG            0x22                       // sets data mode
        #define SSD2119_FRAME_FREQ_REG          0x25
        #define SSD2119_ANALOG_SET_REG          0x26
            #define SSD2119_WR_T                0x4000
            #define SSD2119_VCB                 0x2000
            #define SSD2119_RTLM                0x1000
            #define SSD2119_ENN                 0x0800
        #define SSD2119_VCOM_OTP_1_REG          0x28
        #define SSD2119_VCOM_OTP_2_REG          0x29
        #define SSD2119_GAMMA_CTRL_1_REG        0x30
        #define SSD2119_GAMMA_CTRL_2_REG        0x31
        #define SSD2119_GAMMA_CTRL_3_REG        0x32
        #define SSD2119_GAMMA_CTRL_4_REG        0x33
        #define SSD2119_GAMMA_CTRL_5_REG        0x34
        #define SSD2119_GAMMA_CTRL_6_REG        0x35
        #define SSD2119_GAMMA_CTRL_7_REG        0x36
        #define SSD2119_GAMMA_CTRL_8_REG        0x37
        #define SSD2119_GAMMA_CTRL_9_REG        0x3a
        #define SSD2119_GAMMA_CTRL_10_REG       0x3b
        #define SSD2119_V_SCROLL_CTRL_REG_1     0x41
        #define SSD2119_V_SCROLL_CTRL_REG_2     0x42
        #define SSD2119_V_RAM_POS_REG           0x44
        #define SSD2119_H_RAM_START_REG         0x45
        #define SSD2119_H_RAM_END_REG           0x46
        #define SSD2119_FIRST_SCRN_X_POS_REG    0x48
        #define SSD2119_FIRST_SCRN_Y_POS_REG    0x49
        #define SSD2119_SECND_SCRN_X_POS_REG    0x4a
        #define SSD2119_SECND_SCRN_Y_POS_REG    0x4b
    #if defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
        #define SSD2119_X_RAM_ADDR_REG          0x4f
        #define SSD2119_Y_RAM_ADDR_REG          0x4e
    #else
        #define SSD2119_X_RAM_ADDR_REG          0x4e
        #define SSD2119_Y_RAM_ADDR_REG          0x4f
    #endif

    #if defined MB785_GLCD_MODE

    #endif

        #define _GLCD_CGLCD_DEFINES                                      // include only once
    #endif

    #if defined _GLCD_COMMANDS                                           // link in CGLCD specific interface commands

#if defined MB785_GLCD_MODE
// Write a 16 bit word to the graphic memory in the LCD. It is expected that the display has been commanded to graphic mode
//
static void fnWriteGLCD_data_pair(unsigned short usBytePair)
{
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = (usBytePair >> 8);                                         // high byte
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = usBytePair;                                                // low byte
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR &= ~SPISR_BSY;
    #endif
}

static void fnConcludeGRAMwrite(void)
{
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    while ((SPI3_SR & SPISR_BSY) != 0) {}
    _SETBITS(B, PORTB_BIT2);                                             // negate chip select
    #if defined _WINDOWS
    CollectCommand(1, 0);
    #endif
}

static void LCD_WriteRegIndex(unsigned char LCD_Reg)
{
    _CLEARBITS(B, PORTB_BIT2);                                           // assert chip select
    SPI3_DR = (START_BYTE | SET_INDEX);                                  // access type
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR |= SPISR_TXE;
    #endif
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = 0;                                                         // high byte is always 0
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = LCD_Reg;                                                   // register value
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR &= ~SPISR_BSY;
    #endif
    while ((SPI3_SR & SPISR_TXE) = 0) {}
    while ((SPI3_SR & SPISR_BSY) != 0) {}
    _SETBITS(B, PORTB_BIT2);                                             // negate chip select
}


// Write an 8 bit index command followed by 16 bit register value
//
static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter)
{
    LCD_WriteRegIndex(ucCommand);
    _CLEARBITS(B, PORTB_BIT2);                                           // assert chip select
    SPI3_DR = (START_BYTE | LCD_WRITE_REG);
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR |= SPISR_TXE;
    #endif
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = (usParameter >> 8);
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    SPI3_DR = usParameter;
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR &= ~SPISR_BSY;
    #endif
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    while ((SPI3_SR & SPISR_BSY) != 0) {}
    _SETBITS(B, PORTB_BIT2);                                             // negate chip select
}

// Configure a write window and set the present address to the start of the window
//
static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    while ((SPI3_SR & SPISR_TXE) == 0) {}
    while ((SPI3_SR & SPISR_BSY) != 0) {}                                // ensure any previous data was sent
    _SETBITS(B, PORTB_BIT2);                                             // negate chip select before staring
    #if defined _WINDOWS
    CollectCommand(1, 0);
    #endif
    fnCommandGlcd_2(0x20, y1);                                           // set cursor position
    fnCommandGlcd_2(0x21, (unsigned short)(0x013f - x1));

    LCD_WriteRegIndex(0x22);                                             // select GRAM Reg
    _CLEARBITS(B, PORTB_BIT2);                                           // assert chip select
    SPI3_DR = (START_BYTE | LCD_WRITE_REG);                              // low byte
    #if defined _WINDOWS
    CollectCommand(0, SPI3_DR); SPI3_SR |= SPISR_TXE;
    #endif
}


#if defined SUPPORT_TOUCH_SCREEN && defined MB785_GLCD_MODE              // {4}

// Open I2C interface to communicate with touch screen controller
//
static void fnConfigI2C_Interface(void)
{
    I2CTABLE tI2CParameters;

    tI2CParameters.Channel = 0;                                          // first I2C interface (I2C1 on STM32)
    tI2CParameters.usSpeed = 100;                                        // 100k
    tI2CParameters.Rx_tx_sizes.TxQueueSize = 64;                         // transmit queue size
    tI2CParameters.Rx_tx_sizes.RxQueueSize = 64;                         // receive queue size
    tI2CParameters.Task_to_wake = 0;                                     // no wake on transmission

    if ((TouchPortID = fnOpen(TYPE_I2C, FOR_I_O, &tI2CParameters)) != NO_ID_ALLOCATED) { // open the channel with defined configurations
        static const unsigned char ucResetTouch[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_SYS_CTRL1_OFFSET, STMPE811_SOFT_RESET};
        static const unsigned char ucEnableTouch[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_SYS_CTRL2_OFFSET, (STMPE811_TEMP_SENSOR_OFF | STMPE811_GPIO_OFF)};
        static const unsigned char ucEnableInt[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_INT_EN_OFFSET, (STMPE811_INT_TOUCH_DET/* | STMPE811_INT_FIFO_TH | STMPE811_INT_FIFO_OFLOW*/)};
        static const unsigned char ucConfigADC1[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_ADC_CTRL1_OFFSET, (STMPE811_ADC_CTRL1_CONV80 | STMPE811_ADC_CTRL1_MOD12B | 0x01)};
        static const unsigned char ucConfigPins[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_GPIO_AF_OFFSET, 0};
        static const unsigned char ucConfigDelay[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_TSC_CFG_OFFSET, ((TSC_CFG_AVE_CTRL_4_SAMPLE | TSC_CFG_DET_DELAY_500us | TSC_CFG_SETTLING_500us))};
        static const unsigned char ucConfigThreshold[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_FIFO_TH_OFFSET, 0x01};
        static const unsigned char ucConfigFraction[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_TSC_FRACT_Z_OFFSET, (TSC_FRACT_1_7)};
        static const unsigned char ucConfig50mA[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_TSC_I_DRIVE_OFFSET, TSC_I_DRIVE_50mA};
        static const unsigned char ucResetPending[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_INT_STA_OFFSET, 0xff};        
        static const unsigned char ucConfigIntOut[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_INT_CTRL_OFFSET, (STMPE811_GLOBAL_INT | STMPE811_TYPE_LEVEL | STMPE811_ACTIVE_LOW)};
        static const unsigned char ucConfigMode[] = {ADD_TOUCH_CONTROLLER_WRITE, STMPE811_TSC_CTRL_OFFSET, (TCS_CTRL_EN | TCS_CTRL_X_Y_Z | TCS_CTRL_TRACK_OFF)};

        fnWrite(TouchPortID, (unsigned char *)ucResetTouch, sizeof(ucResetTouch)); // reset the controller  
        fnWrite(TouchPortID, (unsigned char *)ucEnableTouch, sizeof(ucEnableTouch)); // enable touch screen and ADC
        fnWrite(TouchPortID, (unsigned char *)ucEnableInt, sizeof(ucEnableInt)); // enable touch screen detect interrupt
        fnWrite(TouchPortID, (unsigned char *)ucConfigADC1, sizeof(ucConfigADC1)); // configure ADC operation
        fnWrite(TouchPortID, (unsigned char *)ucConfigPins, sizeof(ucConfigPins)); // configure pins as touch function
        fnWrite(TouchPortID, (unsigned char *)ucConfigDelay, sizeof(ucConfigDelay)); // touch screen detection timing
        fnWrite(TouchPortID, (unsigned char *)ucConfigThreshold, sizeof(ucConfigThreshold)); // sampling threshold - interrupt on 1 sample
        fnWrite(TouchPortID, (unsigned char *)ucConfigFraction, sizeof(ucConfigFraction)); // result format - 1 whole part with 7 fraction part
        fnWrite(TouchPortID, (unsigned char *)ucConfig50mA, sizeof(ucConfig50mA)); // set 50mA current limit
        fnWrite(TouchPortID, (unsigned char *)ucResetPending, sizeof(ucResetPending)); // reset all pending interrupts
        fnWrite(TouchPortID, (unsigned char *)ucConfigIntOut, sizeof(ucConfigIntOut)); // configure low sensitive interrupt output
        fnWrite(TouchPortID, (unsigned char *)ucConfigMode, sizeof(ucConfigMode)); // set mode and enable
    }
}

// Interrupt handler when touch controller interrupt occurs
//
static void touch_interrupt(void)
{
    fnInterruptMessage(OWN_TASK, PEN_STATE_CHANGE);                      // inform that the pen state has changed
}

// Configure touch interrupt line to react to falling edges
//
static void fnConfigureTouchInterrupt(void)
{
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    interrupt_setup.int_type = PORT_INTERRUPT;                           // identifier when configuring
    interrupt_setup.int_handler = touch_interrupt;                       // handling function
    interrupt_setup.int_priority = PRIORITY_EXI10_15;                    // port interrupt priority
    interrupt_setup.int_port = PORTB;                                    // the port used
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE);                 // interrupt on this edge
    interrupt_setup.int_port_bit = PORTB_BIT14;                          // the port input connected
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure touch controller interrupt
}

static void fnStartTouch(void)
{
    fnConfigI2C_Interface();                                             // open I2C interface to communicate with touch screen controller
    fnConfigureTouchInterrupt();                                         // configure touch interrupt line
}
#endif
#endif

#if defined AVR32_EVK1105
// Configure the SM Controller with SM setup and timing information for all chip select
// This code is based on the ATMEL reference
//
static void SMC_CS_SETUP(int ncs, unsigned long hsb_mhz_up)
{
    unsigned long nwe_setup    = ((NWE_SETUP    * hsb_mhz_up + 999) / 1000);
    unsigned long ncs_wr_setup = ((NCS_WR_SETUP * hsb_mhz_up + 999) / 1000);
    unsigned long nrd_setup    = ((NRD_SETUP    * hsb_mhz_up + 999) / 1000);
    unsigned long ncs_rd_setup = ((NCS_RD_SETUP * hsb_mhz_up + 999) / 1000);
    unsigned long nwe_pulse    = ((NWE_PULSE    * hsb_mhz_up + 999) / 1000);
    unsigned long ncs_wr_pulse = ((NCS_WR_PULSE * hsb_mhz_up + 999) / 1000);
    unsigned long nrd_pulse    = ((NRD_PULSE    * hsb_mhz_up + 999) / 1000);
    unsigned long ncs_rd_pulse = ((NCS_RD_PULSE * hsb_mhz_up + 999) / 1000);
    unsigned long nwe_cycle    = ((NWE_CYCLE    * hsb_mhz_up + 999) / 1000);
    unsigned long nrd_cycle    = ((NRD_CYCLE    * hsb_mhz_up + 999) / 1000);

    if( ncs_rd_setup + ncs_rd_pulse < nrd_setup + nrd_pulse ) {          // some coherency checks to ensure that CS is active during Rd or Wr
        ncs_rd_pulse = nrd_setup + nrd_pulse - ncs_rd_setup;
    }
    if( ncs_wr_setup + ncs_wr_pulse < nwe_setup + nwe_pulse ) {
        ncs_wr_pulse = nwe_setup + nwe_pulse - ncs_wr_setup;
    }

    /* ncs_hold = n_cycle - ncs_setup - ncs_pulse           */
    /* n_hold   = n_cycle - n_setup - n_pulse               */
    /*                                                      */
    /* All holds parameters must be positive or null, so:   */
    /* nwe_cycle shall be >= ncs_wr_setup + ncs_wr_pulse    */
    if (nwe_cycle < ncs_wr_setup + ncs_wr_pulse) {
        nwe_cycle = ncs_wr_setup + ncs_wr_pulse;
    }

    if (nwe_cycle < nwe_setup + nwe_pulse) {                             // nwe_cycle shall be >= nwe_setup + nwe_pulse
        nwe_cycle = nwe_setup + nwe_pulse;
    }

    if (nrd_cycle < ncs_rd_setup + ncs_rd_pulse) {                       // nrd_cycle shall be >= ncs_rd_setup + ncs_rd_pulse
        nrd_cycle = ncs_rd_setup + ncs_rd_pulse;
    }

    if (nrd_cycle < nrd_setup + nrd_pulse) {                             // nrd_cycle shall be >= nrd_setup + nrd_pulse
        nrd_cycle = nrd_setup + nrd_pulse;
    }

    SMC_SETUP_0 = (nwe_setup | (ncs_wr_setup << NCS_WR_SETUP_SHIFT) | (nrd_setup << NRD_SETUP_SHIFT) | (ncs_rd_setup << NCS_RD_SETUP_SHIFT));
    SMC_PULSE_0 = (nwe_pulse | (ncs_wr_pulse << NCS_WR_PULSE_SHIFT) | (nrd_pulse << NRD_PULSE_SHIFT) | (ncs_rd_pulse << NCS_RD_PULSE_SHIFT));
    SMC_CYCLE_0 = (nwe_cycle | (nrd_cycle    << NCS_RD_CYCLE_SHIFT));
    SMC_MODE_0 = ((((NCS_CONTROLLED_READ) ? READ_MODE_NCS : READ_MODE_NRD)) |
                  (((NCS_CONTROLLED_WRITE) ? WRITE_MODE_NCS : WRITE_MODE_NWE)) | (NWAIT_MODE) |
                  (((SMC_8_BIT_CHIPS) ? BYTE_WRITE_ACCESS_TYPE : BYTE_WRITE_ACCESS_TYPE)) |
                  (((SMC_DBW <= 8 ) ? DBW_8_BIT_BUS : (SMC_DBW <= 16) ? DBW_16_BIT_BUS : DBW_32_BIT_BUS)) |
                    (TDF_CYCLES << TDF_CYCLES_SHIFT) | (TDF_OPTIM) | (PAGE_MODE) | (PAGE_SIZE));
    // SMC_SETUP_0 = 0x00020002
    // SMC_PULSE_0 = 0x100e0604
    // SMC_CYCLE_0 = 0x00120007
    // SMC_MODE_0  = 0x00001103
}


static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    WRITE_REG( HIMAX_COL_ADDR_START2, (x1 >> 8));
    WRITE_REG( HIMAX_COL_ADDR_START1, (x1 & 0xff));
    WRITE_REG( HIMAX_COL_ADDR_END2,   (x2 >> 8));
    WRITE_REG( HIMAX_COL_ADDR_END1,   (x2 & 0xff));
    WRITE_REG( HIMAX_ROW_ADDR_START2, (y1 >> 8));
    WRITE_REG( HIMAX_ROW_ADDR_START1, (y1 & 0xff));
    WRITE_REG( HIMAX_ROW_ADDR_END2,   (y2 >> 8));
    WRITE_REG( HIMAX_ROW_ADDR_END1,   (y2 & 0xff));
}
#endif

#if defined EK_LM3S3748
// Write an 8 bit command byte. It is assumed that the RD/WR lines are initially high and return them high after completion.
// The data bus is considered to be always driven.
//
static void fnWriteGLCD_cmd(unsigned char ucByte)
{
    GLCD_CD_H();                                                         // set C/D line high for command
    GLCD_WR_L();                                                         // set the WR line low
    GLCD_DATAOUT(ucByte);                                                // set the data value on the bus
    GLCD_DELAY_WRITE();                                                  // ensure tDS delay before removing the write line
    GLCD_WR_H();
    #if defined _WINDOWS
    CollectCommand(1, ucByte);
    #endif
}

// Write an 8 bit data byte. It is assumed that the RD/WR lines are initially high and return then high after completion
//
static void fnWriteGLCD_data(unsigned char ucByte)
{
    GLCD_CD_L();                                                         // set C/D line low for data
    GLCD_WR_L();                                                         // set the WR line low
    GLCD_DATAOUT(ucByte);                                                // set the data value on the bus
    GLCD_DELAY_WRITE();                                                  // ensure tDS delay before removing the write line
    GLCD_WR_H();
    #if defined _WINDOWS
	CollectCommand(0, ucByte);
    #endif
}

static void fnWriteGLCD_data_pair(unsigned short usBytePair)
{
    GLCD_CD_L();                                                         // set C/D line low for data
    GLCD_WR_L();                                                         // set the WR line low
    GLCD_DATAOUT((unsigned char)(usBytePair >> 8));                      // set the data value on the bus
    GLCD_DELAY_WRITE();                                                  // ensure tDS delay before removing the write line
    GLCD_WR_H();
    #if defined _WINDOWS
	CollectCommand(0, (unsigned char)(usBytePair >> 8));
    #endif
    GLCD_WR_L();                                                         // set the WR line low
    GLCD_DATAOUT((unsigned char)usBytePair);                             // set the data value on the bus
    GLCD_DELAY_WRITE();                                                  // ensure tDS delay before removing the write line
    GLCD_WR_H();
    #if defined _WINDOWS
	CollectCommand(0, (unsigned char)usBytePair);
    #endif
}

// This routine is used to write one parameter data bytes plus a command byte.
// The GLCD may be busy for some time after certain commands.
//
static void fnCommandGlcd_1(unsigned char ucCommand, unsigned char ucParameter)
{
    fnWriteGLCD_cmd(ucCommand);                                          // write the command
    fnWriteGLCD_data(ucParameter);                                       // write parameter byte
}

// This routine is used to write two parameter data bytes plus a command byte.
// The GLCD may be busy for some time after certain commands
//
static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter)
{
    fnWriteGLCD_cmd(ucCommand);                                          // write the command
    fnWriteGLCD_data((unsigned char)(usParameter >> 8));                 // write high data byte
    fnWriteGLCD_data((unsigned char)usParameter);                        // write low data byte
}

static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    y1++;                                                                // x = 0..127; y = 1..128, so adjust here
    y2++;
    fnCommandGlcd_2(ST7637_COL_ADDR_SET, (unsigned short)((x1 << 8) | ((unsigned char)x2)));
    fnCommandGlcd_2(ST7637_ROW_ADDR_SET, (unsigned short)((y1 << 8) | ((unsigned char)y2)));
}
#endif


#if defined KITRONIX_GLCD_MODE || defined TFT2N0369_GLCD_MODE || defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE || defined FT800_GLCD_MODE

#if defined FT800_GLCD_MODE
    #if defined _WINDOWS
        extern void _FT8XXEMU_cs(int cs);                                // FT800 emulation interface prototypes
        extern unsigned char _FT8XXEMU_transfer(unsigned char data);
    #endif

extern void Ft_Gpu_HostCommand(void *host, unsigned char ucCommand)
{
    WRITE_LCD_SPI_CMD0_FIRST(ucCommand);                                 // assert the CS line and send the command
    WRITE_LCD_SPI_CMD0(0);                                               // send three further dummy bytes
    WRITE_LCD_SPI_CMD0(0);
    WRITE_LCD_SPI_CMD0_LAST(0);                                          // remove the CS line after final byte
    FLUSH_LCD_SPI_RX(4);                                                 // ensure that the activity has terminated before quitting so that further reads/writes can't disturb
}

extern unsigned char Ft_Gpu_Hal_Rd8(void *host, unsigned long ulReg)
{
    unsigned char value;
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)(ulReg >> 16));              // assert the CS line and send the most significant byte of the address
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)ulReg);
    WRITE_LCD_SPI_CMD0(0);
    FLUSH_LCD_SPI_RX(4);                                                 // we are not interested in the data returned from the first 4 bytes
    READ_LCD_SPI_CMD0_LAST(0, value);                                    // read a byte and remove the CS line afterwards
    return value;
}

extern void Ft_Gpu_Hal_Wr8(void *host, unsigned long ulReg, unsigned char ucData)
{
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)((ulReg >> 16) | 0x80));     // assert the CS line and send the most significant byte of the address (for write)
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg));
    WRITE_LCD_SPI_CMD0_LAST((unsigned char)(ucData));                    // send the data byte and negate the chip select line
    FLUSH_LCD_SPI_RX(4);                                                 // ensure that the activity has terminated before quitting so that further reads/writes can't disturb
}

extern void Ft_Gpu_Hal_Wr16(void *host, unsigned long ulReg, unsigned short usData)
{
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)((ulReg >> 16) | 0x80));     // assert the CS line and send the most significant byte of the address (for write)
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg));
    WRITE_LCD_SPI_CMD0((unsigned char)(usData));                         // send the least significant data bytes first
    FLUSH_LCD_SPI_RX(3);                                                 // wait for 3 bytes to be sent/received
    WRITE_LCD_SPI_CMD0_LAST((unsigned char)(usData >> 8));               // send the most significant data byte and negate the chip select line
    FLUSH_LCD_SPI_RX(2);                                                 // ensure that the activity has terminated before quitting so that further reads/writes can't disturb
}

extern unsigned short Ft_Gpu_Hal_Rd16(void *host, unsigned long ulReg)
{
    unsigned char ucValue1, ucValue2;
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)(ulReg >> 16));              // assert the CS line and send the most significant byte of the address
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)ulReg);
    WRITE_LCD_SPI_CMD0(0);
    FLUSH_LCD_SPI_RX(4);                                                 // we are not interested in the data returned from the first 4 bytes
    READ_LCD_SPI_CMD0(0, ucValue1);                                      // read a byte
    READ_LCD_SPI_CMD0_LAST(0, ucValue2);                                 // read a byte and remove the CS line afterwards
    return (ucValue1 | (ucValue2 << 8));                                 // return the read 16 bit value
}

extern void Ft_Gpu_Hal_Wr32(void *host, unsigned long ulReg, unsigned long ulValue)
{
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)((ulReg >> 16) | 0x80));     // assert the CS line and send the most significant byte of the address
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)ulReg);
    WRITE_LCD_SPI_CMD0((unsigned char)ulValue);                          // least significant data byte first
    FLUSH_LCD_SPI_RX(3);                                                 // wait until 3 bytes have been sent/received
    WRITE_LCD_SPI_CMD0((unsigned char)(ulValue >> 8));
    FLUSH_LCD_SPI_RX(1);                                                 // wait for one further byte to be sent/received
    WRITE_LCD_SPI_CMD0((unsigned char)(ulValue >> 16));
    FLUSH_LCD_SPI_RX(1);                                                 // wait for one further byte to be sent/received
    WRITE_LCD_SPI_CMD0_LAST((unsigned char)(ulValue >> 24));             // send most significant byte and negate the chip select line
    FLUSH_LCD_SPI_RX(2);                                                 // ensure that the activity has terminated before quitting so that further reads/writes can't disturb
}

extern unsigned long Ft_Gpu_Hal_Rd32(void *host, unsigned long ulReg)
{
    unsigned char ucValue1, ucValue2, ucValue3, ucValue4;
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)(ulReg >> 16));              // assert the CS line and send the most significant byte of the address
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)ulReg);
    WRITE_LCD_SPI_CMD0(0);
    FLUSH_LCD_SPI_RX(4);                                                 // we are not interested in the data returned from the first 4 bytes
    READ_LCD_SPI_CMD0(0, ucValue1);                                      // read a byte
    READ_LCD_SPI_CMD0(0, ucValue2);                                      // read a byte
    READ_LCD_SPI_CMD0(0, ucValue3);                                      // read a byte
    READ_LCD_SPI_CMD0_LAST(0, ucValue4);                                 // read the final byte and remove the CS line afterwards
    return (ucValue1 | (ucValue2 << 8) | (ucValue3 << 16) | (ucValue4 << 24)); // return the read 32 bit value
}

extern void Ft_Gpu_Hal_RdMem(void *host, unsigned long ulReg, unsigned char *buffer, unsigned long length)
{
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)(ulReg >> 16));              // assert the CS line and send the most significant byte of the address
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)ulReg);
    WRITE_LCD_SPI_CMD0(0);
    FLUSH_LCD_SPI_RX(4);                                                 // we are not interested in the data returned from the first 4 bytes
    while (length > 1) {
        READ_LCD_SPI_CMD0(0, *buffer);                                   // read a byte
        buffer++;
        length--;
    }
    READ_LCD_SPI_CMD0_LAST(0, *buffer);                                  // read the final byte and remove the CS line afterwards
}

extern void Ft_Gpu_Hal_WrMem(void *host, unsigned long ulReg, const unsigned char *ptrData, unsigned long ulDataLength)
{
    WRITE_LCD_SPI_CMD0_FIRST((unsigned char)((ulReg >> 16) | 0x80));     // assert the CS line and send the most significant byte of the address (for write)
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg >> 8));
    WRITE_LCD_SPI_CMD0((unsigned char)(ulReg));
    FLUSH_LCD_SPI_RX(2);                                                 // wait for two bytes to be sent/received
    while (ulDataLength > 1) {
        WRITE_LCD_SPI_CMD0((unsigned char)(*ptrData));                   // send the data bytes
        FLUSH_LCD_SPI_RX(1);
        ptrData++;
        ulDataLength--;
    }
    WRITE_LCD_SPI_CMD0_LAST((unsigned char)(*ptrData));                  // send the final data byte and negate the chip select line
    FLUSH_LCD_SPI_RX(2);                                                 // ensure that the activity has terminated before quitting so that further reads/writes can't disturb
}

extern int SAMAPP_GPU_DLSwap(unsigned char DL_Swap_Type)
{
    #define SWAP_MAX   500
    volatile int iYield = 0;
    unsigned char Swap_Type = DLSWAP_FRAME;

    if (DL_Swap_Type == DLSWAP_LINE) {
        Swap_Type = DLSWAP_LINE;
    }

    Ft_Gpu_Hal_Wr8(0, REG_DLSWAP, Swap_Type);                            // perform a new DL swap

    while (Ft_Gpu_Hal_Rd8(0, REG_DLSWAP) != DLSWAP_DONE) {               // wait until the swap has completed
        if (++iYield > SWAP_MAX) {
            iLCD_State = STATE_LCD_WRITING;
            ft800_host.iCoProcessorWait = SWAP_YIELD;                    // mark that the co-processor is busy and we yield until it is ready again
            uTaskerStateChange(OWN_TASK, UTASKER_GO);                    // poll the co-processor state and inform on completion
            return 1;
        }
    }
    return 0;
}

extern void Ft_App_WrCoCmd_Buffer(Ft_Gpu_Hal_Context_t *phost, unsigned long cmd)
{
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd =(ft_uint32_t*)&Ft_CmdBuffer[Ft_CmdBuffer_Index];
   *pBuffcmd = cmd;
   /* Increment the command index */
   Ft_CmdBuffer_Index += FT_CMD_SIZE;
}

extern void Ft_App_WrCoStr_Buffer(Ft_Gpu_Hal_Context_t *phost, const unsigned char *s)
{
    unsigned short length = 0;
    length = (unsigned short)(uStrlen((const CHAR *)s) + 1);             // last for the null termination
    uStrcpy((CHAR *)&Ft_CmdBuffer[Ft_CmdBuffer_Index], (const CHAR *)s);
    Ft_CmdBuffer_Index += ((length + 3) & ~3);                           // increment the length and long-word align
}

extern int Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost)
{ 
    if (Ft_CmdBuffer_Index > 0) {
        if (Ft_Gpu_Hal_WrCmdBuf(0, Ft_CmdBuffer, Ft_CmdBuffer_Index) != 0) {
            return 1;
        }
    }
    Ft_CmdBuffer_Index = 0;
    return 0;
}

extern void Ft_App_WrDlCmd_Buffer(Ft_Gpu_Hal_Context_t *phost, unsigned long cmd)
{
   /* Copy the command instruction into buffer */
   unsigned long *pBuffcmd = (unsigned long *)&Ft_DlBuffer[Ft_DlBuffer_Index];
   *pBuffcmd = cmd;

   /* Increment the command index */
   Ft_DlBuffer_Index += FT_CMD_SIZE;
}

extern void Ft_App_Flush_DL_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
   if (Ft_DlBuffer_Index > 0) {
     Ft_Gpu_Hal_WrMem(0, RAM_DL, Ft_DlBuffer, Ft_DlBuffer_Index);
   }
   Ft_DlBuffer_Index = 0;
}

extern void Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host, unsigned long count)
{
    ft800_host.ft_cmd_fifo_wp  = (ft800_host.ft_cmd_fifo_wp + count) & 4095;

	//4 byte alignment
    ft800_host.ft_cmd_fifo_wp = (ft800_host.ft_cmd_fifo_wp + 3) & 0xffc;
	Ft_Gpu_Hal_Wr16(0,REG_CMD_WRITE, ft800_host.ft_cmd_fifo_wp);
}

extern unsigned short Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host)
{
    unsigned short fullness, retval;
    //ft800_host.ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
    fullness = ((ft800_host.ft_cmd_fifo_wp - Ft_Gpu_Hal_Rd16(0,REG_CMD_READ)) & 4095);
    retval = ((FT_CMD_FIFO_SIZE - 4) - fullness);
    return (retval);
}

extern void Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host, unsigned long count)
{
    unsigned short getfreespace;
    do {
        getfreespace = Ft_Gpu_Cmdfifo_Freespace(0);
    } while (getfreespace < count);
}


extern int Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host, unsigned char *buffer, unsigned long count)
{
    unsigned long length = 0;
    unsigned long availablefreesize;
    do {                
        availablefreesize = Ft_Gpu_Cmdfifo_Freespace(0);
        if (count > availablefreesize) {
            length = availablefreesize;
        }
        else {
            length = count;
        }
      //Ft_Gpu_Hal_CheckCmdBuffer(0, length);                            // this is not necessary since the free space has been checked already
        Ft_Gpu_Hal_WrMem(0, (ft800_host.ft_cmd_fifo_wp + RAM_CMD), buffer, length);
        Ft_Gpu_Hal_Updatecmdfifo(0, length);
        if (Ft_Gpu_Hal_WaitCmdfifo_empty(0) != 0) {
            ft800_host.buffer = buffer;                                  // the buffer being written
            ft800_host.count = count;                                    // length remaining
            ft800_host.length = length;                                  // stalled length
            return 1;
        }
        count -= length;
        buffer += length;
    } while (count > 0);
    return 0;
}

extern int Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host)
{
    if (ft800_host.iCoProcessorWait != NO_YIELD) {
        _EXCEPTION("Don't call if we have already yielded!!");
        return -1;                                                       // not yet completed
    }
    else {
        #define MAX_YIELD_TIME    500
        volatile int iYield = 0;
        while (Ft_Gpu_Hal_Rd16(0, REG_CMD_READ) != Ft_Gpu_Hal_Rd16(0, REG_CMD_WRITE)) { // wait until the display has completed its present work
            if (++iYield > MAX_YIELD_TIME) {                             // if the wait is longer than the yield time
                iLCD_State = STATE_LCD_WRITING;                          // mark that we are in a state writing to the display
                ft800_host.iCoProcessorWait = FIFO_WRITE_YIELD;          // mark that the co-processor is busy and we yield until it is ready again
                uTaskerStateChange(OWN_TASK, UTASKER_GO);                // poll the co-processor state and inform on completion
                return 1;                                                // not yet completed
            }
        }
        ft800_host.ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(0, REG_CMD_WRITE);
        return 0;                                                        // completed
    }
}

#if defined BRIDGETEK_GLIB

#define eve_assert(x)                                                    // dummy
#define eve_printf_debug(x) _EXCEPTION(x)

#define EVE_Hal_rd16(x, y)   Ft_Gpu_Hal_Rd16(x, y)

static inline void endFunc(EVE_HalContext *phost)
{
    if (phost->Status == EVE_STATUS_WRITING)
    {
        EVE_Hal_endTransfer(phost);
#if !defined(EVE_SUPPORT_CMDB)
        EVE_Hal_wr16(phost, REG_CMD_WRITE, phost->CmdWp);
#endif
    }
}

uint16_t EVE_Cmd_space(EVE_HalContext *phost)
{
    uint16_t space;
#if !defined(EVE_SUPPORT_CMDB)
    uint16_t wp, rp;
#endif
    endFunc(phost);
#if defined(EVE_SUPPORT_CMDB)
    space = EVE_Hal_rd16(phost, REG_CMDB_SPACE) & EVE_CMD_FIFO_MASK;
    if (EVE_CMD_FAULT(space))
        phost->CmdFault = true;
    phost->CmdSpace = space;
    return space;
#else
    wp = EVE_Cmd_wp(phost);
    rp = EVE_Cmd_rp(phost);
    space = (rp - wp - 4) & EVE_CMD_FIFO_MASK;
    phost->CmdSpace = space;
    return space;
#endif
}

static bool checkWait(EVE_HalContext *phost, uint16_t rpOrSpace)
{
    /* Check for coprocessor fault */
    if (EVE_CMD_FAULT(rpOrSpace))
    {
#if defined(_DEBUG) && (EVE_MODEL >= EVE_BT815)
        char err[128];
#endif
        /* Coprocessor fault */
        phost->CmdWaiting = false;
        eve_printf_debug("Coprocessor fault\n");
#if defined(_DEBUG) && (EVE_MODEL >= EVE_BT815)
        EVE_Hal_rdMem(phost, err, RAM_ERR_REPORT, 128);
        eve_printf_debug("%s\n", err);
        displayError(phost, err);
#endif
        /* eve_debug_break(); */
        return false;
    }

    return true;
}

/* Idle. Call regularly to update frequently changing internal state */
void EVE_HalImpl_idle(EVE_HalContext *phost)
{
    /* no-op */
}

void EVE_Hal_idle(EVE_HalContext *phost)
{
    EVE_HalImpl_idle(phost);
}

static bool handleWait(EVE_HalContext *phost, uint16_t rpOrSpace)
{
    /* Check for coprocessor fault */
    if (!checkWait(phost, rpOrSpace))
        return false;

    /* Process any idling */
    EVE_Hal_idle(phost); // not used by BT8XX

                         /* Process user idling */
    if (phost->Parameters.CbCmdWait)
    {
        if (!phost->Parameters.CbCmdWait(phost))
        {
            /* Wait aborted */
            phost->CmdWaiting = false;
            eve_printf_debug("Wait for coprocessor aborted\n");
            return false;
        }
    }
    return true;
}

bool EVE_Cmd_waitSpace(EVE_HalContext *phost, uint32_t size)
{
    uint16_t space;

    if (size > (EVE_CMD_FIFO_SIZE - 4))
    {
        eve_printf_debug("Requested free space exceeds coprocessor FIFO\n");
        return false;
    }

    eve_assert(!phost->CmdWaiting);
    phost->CmdWaiting = true;

    space = phost->CmdSpace;
    if (space < size)
        space = EVE_Cmd_space(phost);
    if (!checkWait(phost, space))
        return false;

    while (space < size)
    {
        if (!handleWait(phost, space))
            return false;
        space = EVE_Cmd_space(phost);
    }

    /* Sufficient space */
    phost->CmdWaiting = false;
    return true;
}

bool /*EVE_Cmd_wr32*/Ft_Gpu_CoCmd_SendCmd(EVE_HalContext *phost, uint32_t value)
{
#if 0
    Ft_Gpu_Hal_Wr32(phost, REG_CMDB_WRITE, value);
#else
    eve_assert(!phost->CmdWaiting);
    eve_assert(phost->CmdBufferIndex == 0);

    if (phost->CmdSpace < 4 && !EVE_Cmd_waitSpace(phost, 4))
        return false;

    if (phost->Status != EVE_STATUS_WRITING)
    {
#if defined(EVE_SUPPORT_CMDB)
        EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, REG_CMDB_WRITE);
#else
        EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, RAM_CMD + phost->CmdWp);
#endif
    }
    EVE_Hal_transfer32(phost, value);
    if (!phost->CmdFunc) /* Keep alive while writing function */
    {
        EVE_Hal_endTransfer(phost);
    }
    eve_assert(phost->CmdSpace >= 4);
    phost->CmdSpace -= 4;
#if !defined(EVE_SUPPORT_CMDB)
    phost->CmdWp += 4;
    phost->CmdWp &= EVE_CMD_FIFO_MASK;
    if (!phost->CmdFunc) /* Defer write pointer */
    {
        EVE_Hal_wr16(phost, REG_CMD_WRITE, phost->CmdWp);
    }
#endif
#endif
    return true;
}

ft_void_t Ft_Gpu_CoCmd_DlStart(EVE_HalContext *phost)
{
    Ft_Gpu_CoCmd_SendCmd(phost, CMD_DLSTART);
}


/* Begin writing a function, keeps the transfer open */
void EVE_Cmd_startFunc(EVE_HalContext *phost)
{
    eve_assert(!phost->CmdWaiting);
    eve_assert(phost->CmdBufferIndex == 0);
    phost->CmdFunc = true;
}




void EVE_Hal_startTransfer(EVE_HalContext *phost, EVE_TRANSFER_T rw, uint32_t addr)
{
    eve_assert(phost->Status == EVE_STATUS_OPENED);

    if (rw == EVE_TRANSFER_READ)
    {
#if 1
        WRITE_LCD_SPI_CMD0_FIRST((unsigned char)(addr >> 16));           // assert the CS line and send the most significant byte of the address
        WRITE_LCD_SPI_CMD0((unsigned char)(addr >> 8));
        WRITE_LCD_SPI_CMD0((unsigned char)addr);
        WRITE_LCD_SPI_CMD0(0);
        FLUSH_LCD_SPI_RX(4);                                             // we are not interested in the data returned from the first 4 bytes
#else
        BT8XXEMU_chipSelect(phost->Emulator, 1);
        BT8XXEMU_transfer(phost->Emulator, (addr >> 16) & 0xFF);
        BT8XXEMU_transfer(phost->Emulator, (addr >> 8) & 0xFF);
        BT8XXEMU_transfer(phost->Emulator, addr & 0xFF);
        BT8XXEMU_transfer(phost->Emulator, 0); // Dummy Read Byte
#endif
        phost->Status = EVE_STATUS_READING;
    }
    else
    {
#if 1
        WRITE_LCD_SPI_CMD0_FIRST((unsigned char)((addr >> 16) | 0x80));  // assert the CS line and send the most significant byte of the address
        WRITE_LCD_SPI_CMD0((unsigned char)(addr >> 8));
        WRITE_LCD_SPI_CMD0((unsigned char)addr);
        FLUSH_LCD_SPI_RX(3);                                             // we are not interested in the data returned from the first 3 bytes
#else
        BT8XXEMU_chipSelect(phost->Emulator, 1);
        BT8XXEMU_transfer(phost->Emulator, ((addr >> 16) & 0xFF) | 0x80);
        BT8XXEMU_transfer(phost->Emulator, (addr >> 8) & 0xFF);
        BT8XXEMU_transfer(phost->Emulator, addr & 0xFF);
#endif
        phost->Status = EVE_STATUS_WRITING;
    }
}

static inline uint8_t transfer8(EVE_HalContext *phost, uint8_t value)
{
#if 1
    unsigned char ucValue;
    READ_LCD_SPI_CMD0(value, ucValue);                                   // send and read a byte
    return ucValue;
#else
    return BT8XXEMU_transfer(phost->Emulator, value);
#endif
}

uint32_t EVE_Hal_transfer32(EVE_HalContext *phost, uint32_t value)
{
    uint32_t retVal = 0;
    retVal = transfer8(phost, value & 0xFF);
    retVal |= (uint32_t)transfer8(phost, (value >> 8) & 0xFF) << 8;
    retVal |= (uint32_t)transfer8(phost, (value >> 16) & 0xFF) << 16;
    retVal |= (uint32_t)transfer8(phost, value >> 24) << 24;
    return retVal;
}

// Note that this functon is not usable with the queued SPI in the Kinetsi - is needs to be integrated into the previous data transfer!!!!
//
void EVE_Hal_endTransfer(EVE_HalContext *phost)
{
    eve_assert(phost->Status == EVE_STATUS_READING || phost->Status == EVE_STATUS_WRITING);
#if 1
    _FT8XXEMU_cs(0);
#else
    BT8XXEMU_chipSelect(phost->Emulator, 0);
#endif
    phost->Status = EVE_STATUS_OPENED;
}

bool EVE_Cmd_wr32(EVE_HalContext *phost, uint32_t value)
{
    eve_assert(!phost->CmdWaiting);
    eve_assert(phost->CmdBufferIndex == 0);

    if (phost->CmdSpace < 4 && !EVE_Cmd_waitSpace(phost, 4))
        return false;

    if (phost->Status != EVE_STATUS_WRITING)
    {
#if defined(EVE_SUPPORT_CMDB)
        EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, REG_CMDB_WRITE);
#else
        EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, RAM_CMD + phost->CmdWp);
#endif
    }
    EVE_Hal_transfer32(phost, value);
    if (!phost->CmdFunc) /* Keep alive while writing function */
    {
        EVE_Hal_endTransfer(phost);
    }
    eve_assert(phost->CmdSpace >= 4);
    phost->CmdSpace -= 4;
#if !defined(EVE_SUPPORT_CMDB)
    phost->CmdWp += 4;
    phost->CmdWp &= EVE_CMD_FIFO_MASK;
    if (!phost->CmdFunc) /* Defer write pointer */
    {
        EVE_Hal_wr16(phost, REG_CMD_WRITE, phost->CmdWp);
    }
#endif

    return true;
}

uint16_t EVE_Cmd_wp(EVE_HalContext *phost)
{
    endFunc(phost);
#if defined(EVE_SUPPORT_CMDB)
    return EVE_Hal_rd16(phost, REG_CMD_WRITE) & EVE_CMD_FIFO_MASK;
#else
    phost->CmdWp = EVE_Hal_rd16(phost, REG_CMD_WRITE) & EVE_CMD_FIFO_MASK;
    return phost->CmdWp;
#endif
}
uint16_t EVE_Hal_transfer16(EVE_HalContext *phost, uint16_t value)
{
    uint16_t retVal = 0;
    retVal = transfer8(phost, value & 0xFF);
    retVal |= (uint16_t)transfer8(phost, (value >> 8) & 0xFF) << 8;
    return retVal;
}

void EVE_Hal_wr16(EVE_HalContext *phost, uint32_t addr, uint16_t v)
{
    EVE_Hal_startTransfer(phost, EVE_TRANSFER_WRITE, addr);
    EVE_Hal_transfer16(phost, v);
    EVE_Hal_endTransfer(phost);
}

/* Move the write pointer forward by the specified number of bytes. Returns the previous write pointer */
uint16_t EVE_Cmd_moveWp(EVE_HalContext *phost, uint16_t bytes)
{
    uint16_t wp, prevWp;
    eve_assert(!phost->CmdWaiting);
    eve_assert(phost->CmdBufferIndex == 0);

    if (!EVE_Cmd_waitSpace(phost, bytes))
        return -1;

    prevWp = EVE_Cmd_wp(phost);
    wp = (prevWp + bytes) & EVE_CMD_FIFO_MASK;
#if !defined(EVE_SUPPORT_CMDB)
    phost->CmdWp = wp;
#endif
    EVE_Hal_wr16(phost, REG_CMD_WRITE, wp);

    return prevWp;
}
/* End writing a function, closes the transfer */
void EVE_Cmd_endFunc(EVE_HalContext *phost)
{
    eve_assert(!phost->CmdWaiting);
    eve_assert(phost->CmdBufferIndex == 0);
    endFunc(phost);
    phost->CmdFunc = false;
}

uint16_t EVE_Cmd_rp(EVE_HalContext *phost)
{
    uint16_t rp;
    endFunc(phost);
    rp = EVE_Hal_rd16(phost, REG_CMD_READ) & EVE_CMD_FIFO_MASK;
    if (EVE_CMD_FAULT(rp))
        phost->CmdFault = true;
    return rp;
}

bool EVE_Cmd_waitFlush(EVE_HalContext *phost)
{
    uint16_t rp, wp;

    eve_assert(!phost->CmdWaiting);
    phost->CmdWaiting = true;
    while ((rp = EVE_Cmd_rp(phost)) != (wp = EVE_Cmd_wp(phost)))
    {
        // eve_printf_debug("Waiting for CoCmd FIFO... rp: %i, wp: %i\n", (int)rp, (int)wp);
        if (!handleWait(phost, rp))
            return false;
    }

    /* Command buffer empty */
    phost->CmdWaiting = false;
    return true;
}

uint32_t EVE_Hal_rd32(EVE_HalContext *phost, uint32_t addr)
{
    uint32_t value;
    EVE_Hal_startTransfer(phost, EVE_TRANSFER_READ, addr);
    value = EVE_Hal_transfer32(phost, 0);
    EVE_Hal_endTransfer(phost);
    return value;
}

ft_uint32_t Ft_Bridgetek_Gpu_CoCmd_Calibrate(EVE_HalContext *phost)
{
#if 0
    Ft_Gpu_CoCmd_SendCmd(phost, CMD_CALIBRATE);
    return 0;
#else
    uint16_t resAddr;

    EVE_Cmd_startFunc(phost);
    EVE_Cmd_wr32(phost, CMD_CALIBRATE);
    resAddr = EVE_Cmd_moveWp(phost, 4);
    EVE_Cmd_endFunc(phost);

#if 1
    return 0;
#else
    /* Wait for the result */
    if (!EVE_Cmd_waitFlush(phost))
        return 0;
    return EVE_Hal_rd32(phost, RAM_CMD + resAddr);
#endif
#endif
}
#endif

#if 0
extern void Ft_GpuEmu_SPII2C_csHigh(void)
{
    // To remove...
   // _EXCEPTION("Should never be called!!!");
    _FT8XXEMU_cs(0);
}

void  Ft_GpuEmu_SPII2C_StartRead(unsigned long ulAddress)
{
    // To remove...
   //  _EXCEPTION("Should never be called!!!");   
     _FT8XXEMU_cs(1);
    _FT8XXEMU_transfer((unsigned char)(ulAddress >> 16));
    _FT8XXEMU_transfer((unsigned char)(ulAddress >> 8));
    _FT8XXEMU_transfer((unsigned char)ulAddress);
    _FT8XXEMU_transfer((unsigned char)0);                                // dummy byte read
}

void  Ft_GpuEmu_SPII2C_StartWrite(unsigned long ulAddress)
{
//    _EXCEPTION("Should never be called!!!");
    // To remove...
    _FT8XXEMU_cs(1);
    _FT8XXEMU_transfer((unsigned char)(ulAddress >> 16) | 0x80);
    _FT8XXEMU_transfer((unsigned char)(ulAddress >> 8));
    _FT8XXEMU_transfer((unsigned char)ulAddress);
}

unsigned char Ft_GpuEmu_SPII2C_transfer(unsigned char data)
{
 //   _EXCEPTION("Should never be called!!!");
    // To remove
	return _FT8XXEMU_transfer(data);
}
#endif
#endif

#if defined ST7789S_GLCD_MODE || defined ILI9341_GLCD_MODE
    #if defined _WINDOWS
unsigned short _ucCommand;
unsigned short _ucData;
    #endif
// Single byte display command
//
static void fnCommandGlcd_1(unsigned char ucCommand)
{
#if defined GLCD_COMMAND_BYTE_ACCESS
    *((volatile unsigned char *)GLCD_COMMAND_ADDR) = ucCommand;          // write the command byte
    #if defined _WINDOWS
    CollectCommand(1, *((volatile unsigned char *)GLCD_COMMAND_ADDR));   // pass the command to the LCD simulator
    #endif
#else
    *((volatile unsigned short *)GLCD_COMMAND_ADDR) = ucCommand;         // write the command byte
    #if defined _WINDOWS
    CollectCommand(1, *((volatile unsigned short *)GLCD_COMMAND_ADDR));  // pass the command to the LCD simulator
    #endif
#endif
}

// Single byte display data
//
static void fnDataGlcd_1(unsigned char ucData)
{
    *((volatile unsigned short *)GLCD_DATA_ADDR) = ucData;               // write the data byte
#if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
#endif
}

// Send a command followed by 4 data bytes prepared as two short words
//
static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usData1, unsigned short usData2)
{
#if defined GLCD_COMMAND_BYTE_ACCESS
    *((volatile unsigned char *)GLCD_COMMAND_ADDR) = ucCommand;          // write the command byte
    #if defined _WINDOWS
    CollectCommand(1, *((volatile unsigned char *)GLCD_COMMAND_ADDR));   // pass the command to the LCD simulator
    #endif
#else
    *((volatile unsigned short *)GLCD_COMMAND_ADDR) = ucCommand;         // write the command byte
    #if defined _WINDOWS
    CollectCommand(1, *((volatile unsigned short *)GLCD_COMMAND_ADDR));  // pass the command to the LCD simulator
    #endif
#endif
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData1 >> 8); // write the command byte
#if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
#endif
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData1); // write the dat byte
#if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
#endif
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData2 >> 8); // write the data byte
#if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
#endif
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData2); // write the data byte
#if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
#endif
}

static void fnWriteGLCD_data_pair(unsigned short usData)
{
#if defined GLCD_DATA_BUS_WIDTH && (GLCD_DATA_BUS_WIDTH == 16)
    *((volatile unsigned short *)GLCD_DATA_ADDR) = usData;               // write the short word
    #if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
    #endif
#else
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData >> 8); // write the MSB of the word
    #if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
    #endif
    *((volatile unsigned short *)GLCD_DATA_ADDR) = (unsigned char)(usData); // write the LSB of the word
    #if defined _WINDOWS
    CollectCommand(0, *((volatile unsigned short *)GLCD_DATA_ADDR));     // pass the data to the LCD simulator
    #endif
#endif
}
#elif !defined TFT2N0369_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE
// Write a 16 bit word to the graphic memory in the LCD. It is expected that the display has been commanded to graphic mode
//
static void fnWriteGLCD_data_pair(unsigned short usBytePair)
{
    _WRITE_PORT(F, (usBytePair >> 8)); _WRITE_PORT(B, usBytePair);       // the data bus is driven by default so just set the value
    _CLEARBITS(H, (TFT_WRITE));_CLEARBITS(H, (TFT_WRITE));               // assert the write signal
    #if defined _WINDOWS
	CollectCommand(0, usBytePair);
    #endif
    _SETBITS(H, (TFT_WRITE));                                            // negate the write signal after at least 50ns
}

// Write an 8 bit command followed by 16 bit parameter
//
static void fnCommandGlcd_2(unsigned char ucCommand, unsigned short usParameter)
{
    _WRITE_PORT(F, 0); _WRITE_PORT(B, ucCommand);                        // the data bus is driven by default so just set the value
    _CLEARBITS(H, (TFT_DC | TFT_WRITE));_CLEARBITS(H, (TFT_DC | TFT_WRITE));_CLEARBITS(H, (TFT_DC | TFT_WRITE)); // drive DC and WR lines low to indicate a command write and slow to respect display timing
    #if defined _WINDOWS
	CollectCommand(1, ucCommand);
    #endif
    _SETBITS(H, (TFT_WRITE));                                            // negate the write line
    _SETBITS(H, (TFT_DC));                                               // negate the DC line to set back to data mode
    fnWriteGLCD_data_pair(usParameter);                                  // write the parameter
}

// Write an 8 bit command with no following parameter
//
static void fnCommandGlcd_1(unsigned char ucCommand)
{
    _WRITE_PORT(F, 0); _WRITE_PORT(B, ucCommand);                        // the data bus is driven by default so just set the value
    _CLEARBITS(H, (TFT_DC | TFT_WRITE));_CLEARBITS(H, (TFT_DC | TFT_WRITE));_CLEARBITS(H, (TFT_DC | TFT_WRITE)); // drive DC and WR lines low to indicate a command write and slow to respect display timing
    #if defined _WINDOWS
	CollectCommand(1, ucCommand);
    #endif
    _SETBITS(H, (TFT_WRITE));                                            // negate the write line
    _SETBITS(H, (TFT_DC));                                               // negate the DC line to set back to data mode
}
#endif

#if defined ST7789S_GLCD_MODE
// Configure a write window and set the present address to the start of the window
//
static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    if (y2 > y1) {
        fnCommandGlcd_2(ST7637_ROW_ADDR_SET, y1, y2);
    }
    else {
        fnCommandGlcd_2(ST7637_ROW_ADDR_SET, y2, y1);
    }
    if (x2 > x1) {
        fnCommandGlcd_2(ST7637_COL_ADDR_SET, x1, x2);
    }
    else {
        fnCommandGlcd_2(ST7637_COL_ADDR_SET, x2, x1);
    }
    
    fnCommandGlcd_1(ST7637_MEMORY_WRITE);                                // set to data mode following this configuration
}
#elif defined ILI9341_GLCD_MODE
// Configure a write window and set the present address to the start of the window
//
static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    if (y2 > y1) {
        fnCommandGlcd_2(ILI9341_PAGE_ADDRESS_SET, y1, y2);
    }
    else {
        fnCommandGlcd_2(ILI9341_PAGE_ADDRESS_SET, y2, y1);
    }
    if (x2 > x1) {
        fnCommandGlcd_2(ILI9341_COLUMN_ADDRESS_SET, x1, x2);
    }
    else {
        fnCommandGlcd_2(ILI9341_COLUMN_ADDRESS_SET, x2, x1);
    }
    
    fnCommandGlcd_1(ILI9341_MEMORY_WRITE);                               // set to data mode following this configuration
}
#else
// Configure a write window and set the present address to the start of the window
//
static void fnSetWindow(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    if (y2 > y1) {
        fnCommandGlcd_2(SSD2119_V_RAM_POS_REG, (unsigned short)((y2 << 8) | y1));
        fnCommandGlcd_2(SSD2119_Y_RAM_ADDR_REG, y1);
    }
    else {
        fnCommandGlcd_2(SSD2119_V_RAM_POS_REG, (unsigned short)((y1 << 8) | y2));
        fnCommandGlcd_2(SSD2119_Y_RAM_ADDR_REG, y2);
    }
    if (x2 > x1) {
        fnCommandGlcd_2(SSD2119_H_RAM_START_REG, x1);
        fnCommandGlcd_2(SSD2119_H_RAM_END_REG, x2);
        fnCommandGlcd_2(SSD2119_X_RAM_ADDR_REG, x1);
    }
    else {
        fnCommandGlcd_2(SSD2119_H_RAM_START_REG, x2);     
        fnCommandGlcd_2(SSD2119_H_RAM_END_REG, x1);
        fnCommandGlcd_2(SSD2119_X_RAM_ADDR_REG, x2);
    }
    fnCommandGlcd_1(SSD2119_RAM_DATA_REG);                               // set to data mode following this configuration
}
#endif

#if defined SUPPORT_TOUCH_SCREEN && !defined TOUCH_FT6206 && !defined TOUCH_FT5406 // {4}

#if defined TFT2N0369_GLCD_MODE                                          // Freescale/NXP Tower LCD
    static void fnY_ready(ADC_INTERRUPT_RESULT *res);
    #define TOUCH_X_DRIVE_PLUS   PORT_AN_BIT6
    #define TOUCH_X_DRIVE_MINUS  PORT_AN_BIT4
    #define TOUCH_Y_DRIVE_PLUS   PORT_AN_BIT5
    #define TOUCH_Y_DRIVE_MINUS  PORT_AN_BIT7
    #define MIN_X_TOUCH          0x0d50                                  // tuned values - for calibration these should be taken from parameters
    #define MAX_X_TOUCH          0x7186
    #define MIN_Y_TOUCH          0x0f40
    #define MAX_Y_TOUCH          0x7100
    #define PRIORITY_ADC         ADC_ERR_PRIORITY
    #define MAX_MOVEMENT_Y       6
    #define MAX_MOVEMENT_X       20

    #define PEN_DOWN_DEBOUNCE    (6 * 5)
#else                                                                    // Luminary Micro IDM_L35_B
    static void fnY_stabilise(void);
    #define TOUCH_X_DRIVE_PLUS   PORTC_BIT4
    #define TOUCH_X_DRIVE_MINUS  PORTC_BIT5
    #define TOUCH_Y_DRIVE_PLUS   PORTD_BIT3
    #define TOUCH_Y_DRIVE_MINUS  PORTD_BIT2
    #define MIN_X_TOUCH          0x00dd                                  // tuned values - for calibration these should be taken from parameters
    #define MAX_X_TOUCH          0x0374
    #define MIN_Y_TOUCH          0x0120
    #define MAX_Y_TOUCH          0x0318
    #define MAX_MOVEMENT_Y       6
    #define MAX_MOVEMENT_X       6

    #define PEN_DOWN_DEBOUNCE    6
#endif

#if defined TFT2N0369_GLCD_MODE                                          // Freescale/NXP Tower LCD
    static ADC_RESULTS    Results;                                       // buffer for results
#else
    static unsigned short usResults[4];                                  // buffer for results
#endif
#if !defined TOUCH_FT6206 && !defined TOUCH_FT5406
    static int iPenDown = 0;                                             // initially pen is up (not touching touch screen)
#endif


// 2..4ms touch screen interrupt (from ADC result)
//
#if defined TFT2N0369_GLCD_MODE                                          // Freescale/NXP Tower LCD
static void fnTouchInterrupt(ADC_INTERRUPT_RESULT *res)
#else
static void fnTouchInterrupt(void)                                       // new x and y samples ready
#endif
{
    #if defined _WINDOWS
    extern void fnGetPenSamples(unsigned short *ptrX, unsigned short *ptrY);
    #endif
#if !defined TOUCH_FT6206 && !defined TOUCH_FT5406
    static unsigned short usLastPixelX, usLastPixelY;
    int iMove = 0;
    unsigned short usX, usY;
    ADC_SETUP adc_setup;
    unsigned char ucMemoryContent, ucNewContent;
#endif
#if defined TOUCH_FT6206 || defined TOUCH_FT5406
#elif defined TFT2N0369_GLCD_MODE                                        // Freescale/NXP Tower LCD
    ADC_RESULTS  samples;
    _CLEARBITS(AN, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS | TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS));
    PANPAR &= ~(TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS);               // set as GPIO - momentarily driving 0
    _SETBITS(AN, TOUCH_X_DRIVE_PLUS);                                    // drive X outputs (X+ positive, X- negative)
    PANPAR |= (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS);                // prepare to read Y input(s)
    
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnY_ready;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_bit = 4;                                           // one of the X-inputs
    adc_setup.int_adc_int_type = ADC_END_OF_SCAN_INT;
    adc_setup.int_adc_mode = (ADC_START_OPERATION | ADC_GET_RESULT | ADC_ALL_RESULTS);
    adc_setup.int_adc_result = &samples;
    #if !defined _WINDOWS
    usY = (unsigned short)Results.sADC_value[7];                         // the x, y pair from the present touch sample
    #endif
    fnConfigureInterrupt((void *)&adc_setup);                            // start next sample sequence

    #if defined _WINDOWS
    fnGetPenSamples((unsigned short *)&samples.sADC_value[6], (unsigned short *)&Results.sADC_value[7]); // get results from simulator
    usY = (unsigned short)Results.sADC_value[7];                         // the x, y pair from the present touch sample
    #endif
    usX = (unsigned short)samples.sADC_value[6];
#else                                                                    // Luminary Micro IDM_L35_B
    _CLEARBITS(D, (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS));
    _DRIVE_PORT_OUTPUT_VALUE(C, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS), 0);

    #if defined _WINDOWS
    fnGetPenSamples(&usResults[1], &usResults[2]);                       // get results from simulator
    #endif
    usX = usResults[1];                                                  // the x, y pair from the present touch sample
    usY = usResults[2];

    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnY_stabilise;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_5 | ADC_CHANNEL_7); // ADC channels 5 and 7 as single ended inputs
    adc_setup.int_adc_differential_inputs = 0;
    adc_setup.int_adc_mode = (ADC_TRIGGER_TIMER | ADC_CONFIGURE_ADC);    // configure and triggered by timer
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
    adc_setup.int_adc_result = &usResults[0];
    adc_setup.int_sequence_count = 1;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start next sequence
#endif
#if !defined TOUCH_FT6206 && !defined TOUCH_FT5406
    if (iPenDown < PEN_DOWN_DEBOUNCE) {
        if (usX < MIN_X_TOUCH) {
            iPenDown = 0;
            return;
        }
        if (usY < MIN_Y_TOUCH) {
            iPenDown = 0;
            return;
        }
    #if defined _WINDOWS
        iPenDown = (PEN_DOWN_DEBOUNCE - 1);                              // no debouncing when simulating
    #endif
        if (++iPenDown < PEN_DOWN_DEBOUNCE) {
            return;
        }                                                                // just detected pen being applied
    }
    else {                                                               // pen presently down
        if (usX < MIN_X_TOUCH) {
            if (usY < MIN_Y_TOUCH) {
                iPenDown = 0;                                            // just detected pen being removed
            }
            return;
        }
        else if (usY < MIN_Y_TOUCH) {                                    // pen probably being lifted
            return;
        }
        iMove = 1;
    }
    if (usX >= MAX_X_TOUCH) {
        usX = (GLCD_X - 1);
    }
    else {
        usX = (unsigned short)(((unsigned long)((usX - MIN_X_TOUCH) * (GLCD_X - 1)))/(MAX_X_TOUCH - MIN_X_TOUCH)); // convert to pixel location
    }
    if (usY >= MAX_Y_TOUCH) {
        usY = (GLCD_Y - 1);
    }
    else {
        usY = (unsigned short)(((unsigned long)((usY - MIN_Y_TOUCH) * (GLCD_Y - 1)))/(MAX_Y_TOUCH - MIN_Y_TOUCH));
    }
    if (iMove != 0) {
    #if !defined _WINDOWS                                                // don't filter movement when simulating
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
    #if CGLCD_PIXEL_SIZE > 1                                             // {3}
    usY /= CGLCD_PIXEL_SIZE;
    usX /= CGLCD_PIXEL_SIZE;
    #endif
    ucNewContent = ucMemoryContent = ucPixelArray[usY][usX/8];           // original content
    ucNewContent |= (0x80 >> usX%8);                                     // set the pixel
    if (ucNewContent != ucMemoryContent) {
        ucPixelArray[usY][usX/8] = ucNewContent;
        ucByteUpdateArray[usY][usX/64] |= (0x80 >> (usX/8)%8);           // mark the need for an update
    }
#endif
}

#if !defined TFT2N0369_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE // not Freescale/NXP Tower LCD
static void fnX_stabilise(void)
{
    ADC_SETUP adc_setup; 
                                                                         // Luminary Micro IDM_L35_B
    _FLOAT_PORT(C, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS));
    _DRIVE_PORT_OUTPUT_VALUE(D, (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS), TOUCH_Y_DRIVE_PLUS);
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnTouchInterrupt;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_5 | ADC_CHANNEL_7); // ADC channels 5 and 7 as single ended inputs
    adc_setup.int_adc_differential_inputs = 0;
    adc_setup.int_adc_mode = (ADC_TRIGGER_TIMER | ADC_CONFIGURE_ADC);    // configure and triggered by timer
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
    adc_setup.int_adc_result = &usResults[2];
    adc_setup.int_sequence_count = 1;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
}
#endif

#if defined TFT2N0369_GLCD_MODE                                          // Freescale/NXP Tower LCD
static void fnY_ready(ADC_INTERRUPT_RESULT *res)
#else
static void fnY_ready(void)
#endif
{
#if !defined TOUCH_FT6206 && !defined TOUCH_FT5406
    ADC_SETUP adc_setup; 
#if defined TFT2N0369_GLCD_MODE                                          // Freescale/NXP Tower LCD
    #if defined M52259_TOWER
    static unsigned char ucIntensityInput = (PORT_TA_BIT0 | PORT_TA_BIT1);
    #endif
    _CLEARBITS(AN, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS | TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS));
    PANPAR &= ~(TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS);               // set as GPIO - momentarily driving 0
    _SETBITS(AN, TOUCH_Y_DRIVE_PLUS);                                    // drive Y outputs (Y+ positive, Y- negative)
    PANPAR |= (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS);                // prepare to read X input(s) by setting them as AIN

    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnTouchInterrupt;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_bit = 4;                                           // one of the X-inputs
    adc_setup.int_adc_int_type = ADC_END_OF_SCAN_INT;
    adc_setup.int_adc_mode = (ADC_START_OPERATION | ADC_GET_RESULT | ADC_ALL_RESULTS);
    adc_setup.int_adc_result = &Results;
    fnConfigureInterrupt((void *)&adc_setup);                            // start next sample sequence

    #if defined M52259_TOWER
    if (ucIntensityInput != _READ_PORT(TA)) {                            // change to port inputs detected
        unsigned char usChanges = ucIntensityInput ^ _READ_PORT(TA);
        ucIntensityInput = _READ_PORT(TA);
        if ((usChanges & PORT_TA_BIT0) && (!(ucIntensityInput & PORT_TA_BIT0))) { // switch SW1 has been pressed - increase brightness
            if (PWMDTY6 >= (PWMPER6 - 5)) {
                PWMDTY6 = PWMPER6;
            }
            else {
                PWMDTY6 += 5;
            }
        }
        if ((usChanges & PORT_TA_BIT1) && (!(ucIntensityInput & PORT_TA_BIT1))) {  // switch SW2 has been pressed - decrease brightness
            if (PWMDTY6 <= 5) {
                PWMDTY6 = 0;
            }
            else {
                PWMDTY6 -= 5;
            }
        }
    }
    #endif
#else                                                                    // Luminary Micro IDM_L35_B
    _CLEARBITS(C, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS));
    _DRIVE_PORT_OUTPUT_VALUE(D, (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS), 0);
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnX_stabilise;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_5 | ADC_CHANNEL_7); // ADC channels 5 and 7 as single ended inputs
    adc_setup.int_adc_differential_inputs = 0;
    adc_setup.int_adc_mode = (ADC_TRIGGER_TIMER | ADC_CONFIGURE_ADC);    // configure and triggered by timer
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
    adc_setup.int_adc_result = &usResults[2];
    adc_setup.int_sequence_count = 1;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
#endif
#endif
}

#if !defined TFT2N0369_GLCD_MODE && !defined ST7789S_GLCD_MODE && !defined ILI9341_GLCD_MODE && !defined FT800_GLCD_MODE // not Freescale/NXP Tower LCD
static void fnY_stabilise(void)
{
    ADC_SETUP adc_setup;                                                 // Luminary Micro IDM_L35_B
    _FLOAT_PORT(D, (TOUCH_Y_DRIVE_PLUS | TOUCH_Y_DRIVE_MINUS));
    _DRIVE_PORT_OUTPUT_VALUE(C, (TOUCH_X_DRIVE_PLUS | TOUCH_X_DRIVE_MINUS), TOUCH_X_DRIVE_PLUS);

    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = fnY_ready;
    adc_setup.int_priority = PRIORITY_ADC;
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_5 | ADC_CHANNEL_7); // ADC channels 5 and 7 as single ended inputs
    adc_setup.int_adc_differential_inputs = 0;
    adc_setup.int_adc_mode = (ADC_TRIGGER_TIMER | ADC_CONFIGURE_ADC);    // configure and triggered by timer
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
    adc_setup.int_adc_result = &usResults[0];
    adc_setup.int_sequence_count = 1;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
}
#endif
#endif

#if defined SUPPORT_TOUCH_SCREEN
    #if defined TOUCH_FT6206 || defined TOUCH_FT5406
// Touch screen informs that touch data is ready to read
//
static void touch_screen_interrupt(void)
{
    fnInterruptMessage(OWN_TASK, PEN_STATE_CHANGE);                      // inform that the pen state has changed
}
    #endif

// The function is presently dedicated to Luminary Micro, Freescale Tower LCD and FT6206
//
static void fnStartTouch(void)
{
    #if defined TOUCH_FT6206 || defined TOUCH_FT5406
    I2CTABLE tI2CParameters;
    tI2CParameters.Channel = 0;                                          // I2C interface 0
    tI2CParameters.usSpeed = 100;                                        // 100k
    tI2CParameters.Rx_tx_sizes.TxQueueSize = 64;                         // transmit queue size
    tI2CParameters.Rx_tx_sizes.RxQueueSize = 64;                         // receive queue size
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
        #endif
    #endif
#endif

    #if defined GLCD_INIT && defined AVR32_EVK1105
            POWER_UP(PBB, PBBMASK_SMC);                                  // power up the static memory controller
            POWER_UP(HSB, HSBMASK_EBI);                                  // power up the external bus interface
            SMC_CS_SETUP(0, ((PBB_CLOCK + 999999) / 1000000));           // configure the SMC to match the 16 bit LCD bus interface
            _CONFIG_PERIPHERAL_A(2, (EBI_DATA_0_A | EBI_DATA_1_A | EBI_DATA_2_A | EBI_DATA_3_A | EBI_DATA_4_A | EBI_DATA_5_A)); // configure EBI lines D0..D5
            _CONFIG_PERIPHERAL_A(3, (EBI_DATA_6_A | EBI_DATA_7_A | EBI_DATA_8_A | EBI_DATA_9_A | EBI_DATA_10_A | EBI_DATA_11_A | EBI_DATA_12_A | EBI_DATA_13_A | EBI_DATA_14_A | EBI_DATA_15_A));// configure EBI lines D6..D15
            _CONFIG_PERIPHERAL_A(3, (EBI_NWE_0_A | EBI_NRD_0_A));        // configure EBI NWE0 (WR), NRD line
            _CONFIG_PERIPHERAL_C(0, (EBI_NCS_0_C | EBI_ADD_21_C));       // configure EBI NCS0 as LCD chip select line and EBI A21 as DNC
            GLCD_RST_H();                                                // release the reset line - which takes place > 120ms after
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.005*SEC), E_INIT_DELAY); // 5ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING;
            return;

        case STATE_INITIALISING:
            {
                unsigned short usRegisterValue;
                *ET024006_CMD_ADDR = HIMAX_OSCCTRL1;                     // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= HIMAX_OSC_EN;                         // set the oscillator enable bit
                *ET024006_CMD_ADDR = HIMAX_OSCCTRL1;                     // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.010*SEC), E_INIT_DELAY); // 10ms delay
                iLCD_State = STATE_INITIALISING_1;
            }
            return;

        case STATE_INITIALISING_1:
            {
                unsigned short usRegisterValue;
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~HIMAX_STB;                           // clear the standby bit
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value
                WRITE_REG(HIMAX_GAMMACTRL1,  0x94);                      // write gamma correction values
                WRITE_REG(HIMAX_GAMMACTRL2,  0x41);
                WRITE_REG(HIMAX_GAMMACTRL3,  0x00);
                WRITE_REG(HIMAX_GAMMACTRL4,  0x33);
                WRITE_REG(HIMAX_GAMMACTRL5,  0x23);
                WRITE_REG(HIMAX_GAMMACTRL6,  0x45);
                WRITE_REG(HIMAX_GAMMACTRL7,  0x44);
                WRITE_REG(HIMAX_GAMMACTRL8,  0x77);
                WRITE_REG(HIMAX_GAMMACTRL9,  0x12);
                WRITE_REG(HIMAX_GAMMACTRL10, 0xCC);
                WRITE_REG(HIMAX_GAMMACTRL11, 0x46);
                WRITE_REG(HIMAX_GAMMACTRL12, 0x82);
                WRITE_REG(HIMAX_CYCLECTRL5,  0x38);                      // not actually gamma, but driving timing, required for noise removal for our TFT.
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050*SEC), E_INIT_DELAY); // 50ms delay
                iLCD_State = STATE_INITIALISING_2;
            }
            return;

        case STATE_INITIALISING_2:                                       // start the power up sequence
            {
                unsigned short usRegisterValue;
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~(HIMAX_DTE);                         // clear the bit
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~(HIMAX_PON);                         // clear the bit
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_DK);                           // set the bit
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_VCOMCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~(HIMAX_VCOMG);                       // clear the bit
                *ET024006_CMD_ADDR = HIMAX_VCOMCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050*SEC), E_INIT_DELAY); // 50ms delay
                iLCD_State = STATE_INITIALISING_3;
            }
            return;

        case STATE_INITIALISING_3:
            {
                unsigned short usRegisterValue;
                WRITE_REG(HIMAX_POWERCTRL2, HIMAX_AP2);                  // set op-amp current drive level

                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_PON);                          // set the bit
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~(HIMAX_DK);                          // clear the bit
                *ET024006_CMD_ADDR = HIMAX_POWERCTRL1;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_VCOMCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_VCOMG);                        // set the bit
                *ET024006_CMD_ADDR = HIMAX_VCOMCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050*SEC), E_INIT_DELAY); // 50ms delay
                iLCD_State = STATE_INITIALISING_4;
            }
            return;

        case STATE_INITIALISING_4:                                       // initiate turning on the display
            {
                unsigned short usRegisterValue;
                WRITE_REG(HIMAX_SAP, 0x0a);                              // set fixed current level in normal mode

                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue &= ~(HIMAX_GON | HIMAX_DTE | HIMAX_D1);  // clear the bits
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_D0);                           // set the bit
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050*SEC), E_INIT_DELAY); // 50ms delay
                iLCD_State = STATE_INITIALISING_5;
            }
            return;

        case STATE_INITIALISING_5:                                       // initiate turning on the display
            {
                unsigned short usRegisterValue;

                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_GON);                          // set the bit
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_D1);                           // set the bit
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050*SEC), E_INIT_DELAY); // 50ms delay
                iLCD_State = STATE_INITIALISING_6;
            }
            return;

        case STATE_INITIALISING_6:                                       // initiate turning on the display
            {
                unsigned short usRegisterValue;

                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_DTE);                          // set the bit
                *ET024006_CMD_ADDR = HIMAX_DISPCTRL1;                    // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                *ET024006_CMD_ADDR = HIMAX_DISPMODECTRL;                 // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_INVON);                        // set the bit
                *ET024006_CMD_ADDR = HIMAX_DISPMODECTRL;                 // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value

                WRITE_REG(HIMAX_MEMACCESSCTRL, (HIMAX_MX | HIMAX_MV | HIMAX_BGR));

                *ET024006_CMD_ADDR = HIMAX_INTERNAL28;                   // set the register address
                usRegisterValue = *ET024006_PARAM_ADDR;                  // read the content of the register
                usRegisterValue |= (HIMAX_TEON);                         // set the bit
                *ET024006_CMD_ADDR = HIMAX_INTERNAL28;                   // set the register address
                *ET024006_PARAM_ADDR = usRegisterValue;                  // set the new register value
                fnSetWindow(0, 0, DISPLAY_RIGHT_PIXEL, DISPLAY_BOTTOM_PIXEL); // set drawing windows to the entire screen size
                SELECT_REG(HIMAX_SRAMWRITE);                             // go to graphics write mode
                fnClearScreen(0);
            }
            break;
    #elif defined GLCD_INIT && defined EK_LM3S3748
            fnWriteGLCD_cmd(ST7637_RESET);                               // perform a software reset of the ST7637 and wait at least 120ms
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.15*SEC), E_INIT_DELAY);
            iLCD_State = STATE_INITIALISING;
            return;

        case STATE_INITIALISING:
            fnCommandGlcd_1(ST7637_AUTOLOAD_SET, 0xbf);                  // disable autoloading of mask ROM data
            fnCommandGlcd_1(ST7637_OTP_READ_WRITE, 0x00);                // set OTP control mode to read
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.01*SEC), E_INIT_DELAY); // wait at least 10ms
            iLCD_State = STATE_INITIALISING_1;
            return;

        case STATE_INITIALISING_1:
            fnWriteGLCD_cmd(ST7637_READ_OTP);                            // start OTP read
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.02*SEC), E_INIT_DELAY); // wait at least 20ms
            iLCD_State = STATE_INITIALISING_2;
            return;

        case STATE_INITIALISING_2:
            fnWriteGLCD_cmd(ST7637_CONTROL_CANCEL);                      // cancel the OTP read, which will have completed by now
            fnWriteGLCD_cmd(ST7637_DISPLAY_OFF);                         // turn off the display
            fnWriteGLCD_cmd(ST7637_SLPOUT);                              // exit from the sleep mode with boost on
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05*SEC), E_INIT_DELAY); // wait at least 50ms
            iLCD_State = STATE_INITIALISING_3;
            return;

        case STATE_INITIALISING_3:
            fnCommandGlcd_2(ST7637_VOP_SET, 0x0401);                     // program LCD supply V0 to 14V
            fnCommandGlcd_1(ST7637_BIAS_SET, 0x00);                      // setect an LCD bias voltage ration of 1:12
            fnCommandGlcd_1(ST7637_BOOSTER_SET, 0x07);                   // enable 8x booster
            fnCommandGlcd_1(ST7637_COMSCANDIR, 0xc0);                    // invert the column scan direction
            fnCommandGlcd_1(ST7637_COLMODE, 0x05);                       // select 16bpp, 5-6-5 format
            fnCommandGlcd_1(ST7637_MADCTR, 0x00);                        // select the memory scanning direction
            fnWriteGLCD_cmd(ST7637_DISPLAY_ON);                          // enable the display
            fnSetWindow(DISPLAY_LEFT_PIXEL, DISPLAY_TOP_PIXEL, DISPLAY_RIGHT_PIXEL, DISPLAY_BOTTOM_PIXEL);
            fnWriteGLCD_cmd(ST7637_MEMORY_WRITE);
            fnClearScreen(0);
            break;
    #elif defined GLCD_INIT && defined KITRONIX_GLCD_MODE                // {1}
            GLCD_RST_H();                                                // release the reset line
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.003*SEC), E_INIT_DELAY); // 3ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING;
            return;
        case STATE_INITIALISING:
            fnCommandGlcd_2(SSD2119_SLEEP_MODE_1_REG, SSD2119_SL);       // ensure start in sleep mode
            fnCommandGlcd_2(SSD2119_PWR_CTRL_5_REG,        (SSD2119_NOTP | 0x0032)); // set initial power parameters
            fnCommandGlcd_2(SSD2119_VCOM_OTP_1_REG,        0x0006);
            fnCommandGlcd_2(SSD2119_OSC_START_REG,         SSD2119_OSCEN); // start oscillator
            fnCommandGlcd_2(SSD2119_OUTPUT_CTRL_REG,       (SSD2119_REVERSE_DISPLAY | SSD2119_FLIP_DISPLAY | SSD2119_TB | SSD2119_RL/* | SSD2119_BGR | SSD2119_TB*/ | 0xef)); // set pixel format and display orientation
            fnCommandGlcd_2(SSD2119_LCD_DRIVE_AC_CTRL_REG, (SSD2119_B_C | SSD2119_EOR/* | SSD2119_ENWS*/));
            fnCommandGlcd_2(SSD2119_SLEEP_MODE_1_REG, 0x0000);           // exit sleep mode
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.035*SEC), E_INIT_DELAY); // wait at least 30ms
            iLCD_State = STATE_INITIALISING_1;
            break;
        case STATE_INITIALISING_1:
            fnCommandGlcd_2(SSD2119_ENTRY_MODE_REG, (SSD2119_DFM_65K | SSD2119_DENMODE | SSD2119_ID_11)); // configure pixel color format and MCU interface parameters
            fnCommandGlcd_2(SSD2119_SLEEP_MODE_2_REG, SSD2119_CORE_VOLTAGE_1_7); // set analogue parameters
            fnCommandGlcd_2(SSD2119_ANALOG_SET_REG,   (SSD2119_VCB | SSD2119_RTLM | SSD2119_ENN));
            fnCommandGlcd_2(SSD2119_DISPLAY_CTRL_REG, 0x0033);           // enable display
            fnCommandGlcd_2(SSD2119_PWR_CTRL_2_REG,   SSD2119_VCIX2_6_1);// set VCIX2 voltage to 6.1V
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_1_REG, 0x0000);           // configure gamma correction
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_2_REG, 0x0303);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_3_REG, 0x0407);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_4_REG, 0x0301);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_5_REG, 0x0301);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_6_REG, 0x0403);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_7_REG, 0x0707);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_8_REG, 0x0400);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_9_REG, 0x0a00);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_10_REG,0x1000);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_3_REG,   SSD2119_VLCD63_2_335);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_4_REG,   (SSD2119_VCOMG | SSD2119_VCOM_1_02));
            fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));
            fnClearScreen(0);                                            // clear display 
            if (iLCD_State == STATE_LCD_CLEARING_DISPLAY) {              // not yet complete
                return;
            }
            ENABLE_BACKLIGHT();                                          // finally enable backlight
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
    #elif (defined GLCD_INIT && defined ST7789S_GLCD_MODE)
            GLCD_RST_H();                                                // release the reset line
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.120 * SEC), E_INIT_DELAY); // 120ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING;
            return;
        case STATE_INITIALISING:
            fnCommandGlcd_1(ST7637_SLPOUT);                              // sleep out command (0x11)
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.05 * SEC), E_INIT_DELAY); // 50ms delay
            iLCD_State = STATE_INITIALISING_1;
            break;
        case STATE_INITIALISING_1:
            fnCommandGlcd_1(ST7637_MADCTR);                              // memory data access control (0x36)
            fnDataGlcd_1(0x00);                                          // select the memory scanning direction
            fnCommandGlcd_1(ST7637_TEON);                                // tearing effect line on (0x35)
            fnDataGlcd_1(0x00);                                          // the tearing effect output consists of v-blanking information only

          //fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));               // set a window equal to the complete display size

            fnCommandGlcd_1(ST7637_COLMODE);                             // interface pixel format (0x3a)
            fnDataGlcd_1(0x55);  	

            fnCommandGlcd_1(ST7789_PORCH_CONTROL);                       // porch control (0xb2)
            fnDataGlcd_1(0x46);
            fnDataGlcd_1(0x4a); 
            fnDataGlcd_1(0x01); 
            fnDataGlcd_1(0xde);
            fnDataGlcd_1(0x33);  

            fnCommandGlcd_1(ST7789_FRAME_RATE_CONTROL);                  // frame rate control (0xb3)
            fnDataGlcd_1(0x10); 
            fnDataGlcd_1(0x05); 
            fnDataGlcd_1(0x0f);  
	
            fnCommandGlcd_1(0xb4);                                       // unknow command
            fnDataGlcd_1(0x0b);  
	
            fnCommandGlcd_1(ST7789_GATE_CONTROL);                        // gate control (0xb7)
            fnDataGlcd_1(0x35);  
	
            fnCommandGlcd_1(ST7789_VCOM_SETTING);                        // VCOM setting (0xbb)
            fnDataGlcd_1(0x28);  
	
            fnCommandGlcd_1(0xbc);                                       // unknown command
            fnDataGlcd_1(0xec);  
	
            fnCommandGlcd_1(ST7789_LCM_CONTROL);                         // LCM control (0xc0)
            fnDataGlcd_1(0x2c);  
	
            fnCommandGlcd_1(ST7789_VDV_VRH_ENABLE);                      // VDV and VRH command enable (0xc2)
            fnDataGlcd_1(0x01); 
		
            fnCommandGlcd_1(ST7789_VRH_SET);                             // VRH set (0xc3)
            fnDataGlcd_1(0x1e);  
	
            fnCommandGlcd_1(ST7789_VDV_SET);                             // VDV set (0xc4)
            fnDataGlcd_1(0x20);  
	
            fnCommandGlcd_1(ST7789_FRAME_RATE_NORMAL_MODE);              // frame rate control in normal mode (0xc6)
            fnDataGlcd_1(0x0c);

            fnCommandGlcd_1(ST7789_POWER_CONTROL_1);                     // power control 1 (0xd0)
            fnDataGlcd_1(0xa4); 
            fnDataGlcd_1(0xa1);  
	
            fnCommandGlcd_1(ST7789_POSITIVE_GAMMA);                      // positive voltage gamma control (0xe0)
            fnDataGlcd_1(0xd0); 
            fnDataGlcd_1(0x00); 
            fnDataGlcd_1(0x00); 
            fnDataGlcd_1(0x08); 
            fnDataGlcd_1(0x07); 
            fnDataGlcd_1(0x05); 
            fnDataGlcd_1(0x29); 
            fnDataGlcd_1(0x54); 
            fnDataGlcd_1(0x41); 
            fnDataGlcd_1(0x3c); 
            fnDataGlcd_1(0x17); 
            fnDataGlcd_1(0x15); 
            fnDataGlcd_1(0x1a); 
            fnDataGlcd_1(0x20);  
	
            fnCommandGlcd_1(ST7789_NEGATIVE_GAMMA);                      // negative voltage gamma control (0xe1)
            fnDataGlcd_1(0xd0); 
            fnDataGlcd_1(0x00); 
            fnDataGlcd_1(0x00); 
            fnDataGlcd_1(0x08); 
            fnDataGlcd_1(0x07); 
            fnDataGlcd_1(0x04); 
            fnDataGlcd_1(0x29); 
            fnDataGlcd_1(0x44); 
            fnDataGlcd_1(0x42); 
            fnDataGlcd_1(0x3b); 
            fnDataGlcd_1(0x16); 
            fnDataGlcd_1(0x15); 
            fnDataGlcd_1(0x1b); 
            fnDataGlcd_1(0x1f);

            fnClearScreen(0);
            if (iLCD_State == STATE_LCD_CLEARING_DISPLAY) {
                break;
            }
            fnCommandGlcd_1(ST7637_DISPLAY_ON);                          // display on (0x29)
            fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));               // set a window equal to the complete display size
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
    #elif (defined GLCD_INIT && defined ILI9341_GLCD_MODE)
            GLCD_RST_H();                                                // release the reset line
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.100 * SEC), E_INIT_DELAY); // 100ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING;
            return;
        case STATE_INITIALISING:
            fnCommandGlcd_1(0xcb);                                       // ??
            fnDataGlcd_1(0x39);
            fnDataGlcd_1(0x2c);
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0x34);
            fnDataGlcd_1(0x02);

            fnCommandGlcd_1(0xcf);                                       // ??
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0xc1);
            fnDataGlcd_1(0x30);

            fnCommandGlcd_1(0xe8);                                       // ??
            fnDataGlcd_1(0x85);
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0x78);

            fnCommandGlcd_1(0xea);                                       // ??
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0x00);

            fnCommandGlcd_1(0xed);                                       // ??
            fnDataGlcd_1(0x64);
            fnDataGlcd_1(0x03);
            fnDataGlcd_1(0x12);
            fnDataGlcd_1(0x81);
            
            fnCommandGlcd_1(0xf7);                                       // ??
            fnDataGlcd_1(0x20);

            fnCommandGlcd_1(ILI9341_POWER_CONTROL_1);                    // 0xc0
            fnDataGlcd_1(0x23);
            
            fnCommandGlcd_1(ILI9341_POWER_CONTROL_2);                    // 0xc1
            fnDataGlcd_1(0x10);
            
            fnCommandGlcd_1(ILI9341_VCOM_CONTROL_1);                     // 0xc5
            fnDataGlcd_1(0x2b);
            fnDataGlcd_1(0x2b);
            
            fnCommandGlcd_1(ILI9341_VCOM_CONTROL_2);                     // 0xc7
            fnDataGlcd_1(0xc0);

            fnCommandGlcd_1(ILI9341_MEMORY_ACCESS_CONTROL);              // 0x36
            fnDataGlcd_1(0xe8);

            fnCommandGlcd_1(ILI9341_FRAME_RATE_CONTROL_NORMAL);          // 0xb1
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0x1b);
            
            fnCommandGlcd_1(ILI9341_DISPLAY_FUNCTION_CONTROL);           // 0xb6
            fnDataGlcd_1(0x0a);
            fnDataGlcd_1(0x02);
            
            fnCommandGlcd_1(0xf2);                                       // ??
            fnDataGlcd_1(0x00);

            fnCommandGlcd_1(ILI9341_GAMMA_SET);                          // 0x26
            fnDataGlcd_1(0x01);

            fnCommandGlcd_1(ILI9341_PIXEL_FORMAT_SET);                   // 0x3a
            fnDataGlcd_1(0x55);

            fnCommandGlcd_1(ILI9341_DISPLAY_INVERSION_ON);               // 0x21

            fnCommandGlcd_1(ILI9341_POSITIVE_GAMMA_CORRECTION);          // 0xe0
            fnDataGlcd_1(0x0f);
            fnDataGlcd_1(0x31);
            fnDataGlcd_1(0x2b);
            fnDataGlcd_1(0x0c);
            fnDataGlcd_1(0x0e);
            fnDataGlcd_1(0x08);
            fnDataGlcd_1(0x4e);
            fnDataGlcd_1(0xf1);
            fnDataGlcd_1(0x37);
            fnDataGlcd_1(0x07);
            fnDataGlcd_1(0x10);
            fnDataGlcd_1(0x03);
            fnDataGlcd_1(0x0e);
            fnDataGlcd_1(0x09);
            fnDataGlcd_1(0x00);

            fnCommandGlcd_1(ILI9341_NEGATIVE_GAMMA_CORRECTION);          // 0xe1
            fnDataGlcd_1(0x00);
            fnDataGlcd_1(0x0e);
            fnDataGlcd_1(0x14);
            fnDataGlcd_1(0x03);
            fnDataGlcd_1(0x11);
            fnDataGlcd_1(0x07);
            fnDataGlcd_1(0x31);
            fnDataGlcd_1(0xc1);
            fnDataGlcd_1(0x48);
            fnDataGlcd_1(0x08);
            fnDataGlcd_1(0x0f);
            fnDataGlcd_1(0x0c);
            fnDataGlcd_1(0x31);
            fnDataGlcd_1(0x36);
            fnDataGlcd_1(0x0f);

            fnCommandGlcd_1(ILI9341_SLEEP_OUT);                          // 0x11

            fnClearScreen(0);
            if (iLCD_State == STATE_LCD_CLEARING_DISPLAY) {
                break;
            }
            fnCommandGlcd_1(ILI9341_DISPLAY_ON);                         // display on (0x29)
            fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));               // set a window equal to the complete display size
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
    #elif defined GLCD_INIT && defined FT800_GLCD_MODE                   // {5}
            GLCD_RST_H();                                                // release the reset line (this is in fact the power down line on the FT800 - not needed by emulator)
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050 * SEC), E_INIT_DELAY); // 50ms delay to allow the display to become ready
            iLCD_State = STATE_INITIALISING;
            return;
        case STATE_INITIALISING:
            #define FT_GPU_ACTIVE_M      0x00
            #define FT_GPU_EXTERNAL_OSC  0x44
            #define FT_GPU_PLL_48M       0x62
            #define FT_GPU_CORE_RESET    0x68
            Ft_Gpu_HostCommand(0, FT_GPU_ACTIVE_M);                      // access address 0 to wake up the device
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.020 * SEC), E_INIT_DELAY); // 20ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING_1;
            return;
        case STATE_INITIALISING_1:
            Ft_Gpu_HostCommand(0, FT_GPU_EXTERNAL_OSC);                  // set the clock to external clock
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.010 * SEC), E_INIT_DELAY); // 10ms delay
            iLCD_State = STATE_INITIALISING_2;
            return;
        case STATE_INITIALISING_2:
            Ft_Gpu_HostCommand(0, FT_GPU_PLL_48M);                       // switch the PLL output to 48MHz
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.010 * SEC), E_INIT_DELAY); // 10ms delay
            iLCD_State = STATE_INITIALISING_3;
            return;
        case STATE_INITIALISING_3:
            Ft_Gpu_HostCommand(0, FT_GPU_CORE_RESET);                    // perform a core reset
            iLCD_State = STATE_INITIALISING_4;
            // Fall though intentionally
            //
        case STATE_INITIALISING_4:
        #if defined SUPPORT_FLUSH
            fnFlush(PortIDInternal, FLUSH_RX);                           // {7} flush the timeout events that have queued
        #endif
			if (Ft_Gpu_Hal_Rd8(0, REG_ID) != 0x7c) {                     // read register ID to check if FT800 is ready
                uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.020 * SEC), E_INIT_DELAY); // 20ms delay before trying again
                return;
            }
            #define DLSWAP_FRAME         2UL
        #if defined FT_812_ENABLE
            #define RESISTANCE_THRESHOLD (1800)
        #else
            #define RESISTANCE_THRESHOLD (1200)
        #endif
        #if defined FT_800_ENABLE
            Ft_Gpu_Hal_Wr16(0, REG_HCYCLE, 408);                         // 320 x 240
            Ft_Gpu_Hal_Wr16(0, REG_HOFFSET, 70);
            Ft_Gpu_Hal_Wr16(0, REG_HSYNC0, 0);
            Ft_Gpu_Hal_Wr16(0, REG_HSYNC1, 10);
            Ft_Gpu_Hal_Wr16(0, REG_VCYCLE, 263);
            Ft_Gpu_Hal_Wr16(0, REG_VOFFSET, 13);
            Ft_Gpu_Hal_Wr16(0, REG_VSYNC0, 0);
            Ft_Gpu_Hal_Wr16(0, REG_VSYNC1, 2);
            Ft_Gpu_Hal_Wr8(0,  REG_SWIZZLE, 2);
            Ft_Gpu_Hal_Wr8(0,  REG_PCLK_POL, 8);
            Ft_Gpu_Hal_Wr8(0,  REG_PCLK, 8);
            Ft_Gpu_Hal_Wr16(0, REG_HSIZE, GLCD_X);
            Ft_Gpu_Hal_Wr16(0, REG_VSIZE, GLCD_Y);
        #else
            Ft_Gpu_Hal_Wr16(0, REG_HCYCLE, 928);                         // original reference
            Ft_Gpu_Hal_Wr16(0, REG_HOFFSET, 88);
            Ft_Gpu_Hal_Wr16(0, REG_HSYNC0, 0);
            Ft_Gpu_Hal_Wr16(0, REG_HSYNC1, 48);
            Ft_Gpu_Hal_Wr16(0, REG_VCYCLE, 525);
            Ft_Gpu_Hal_Wr16(0, REG_VOFFSET, 32);
            Ft_Gpu_Hal_Wr16(0, REG_VSYNC0, 0);
            Ft_Gpu_Hal_Wr16(0, REG_VSYNC1, 3);
            Ft_Gpu_Hal_Wr8(0,  REG_SWIZZLE, 0);
            Ft_Gpu_Hal_Wr8(0,  REG_PCLK_POL, 1);
            #if !defined FT_812_ENABLE
            Ft_Gpu_Hal_Wr8(0,  REG_PCLK, 5);
            #endif
            Ft_Gpu_Hal_Wr16(0, REG_HSIZE, 800);
            Ft_Gpu_Hal_Wr16(0, REG_VSIZE, 480);
            Ft_Gpu_Hal_Wr16(0, REG_CSPREAD, 0);
            Ft_Gpu_Hal_Wr16(0, REG_DITHER, 1);
        #endif
        #if defined FT_800_ENABLE || defined FT_801_ENABLE
            Ft_Gpu_Hal_Wr8(0, REG_GPIO_DIR, (0x83 | Ft_Gpu_Hal_Rd8(0, REG_GPIO_DIR))); // set the display enable to '1'
            Ft_Gpu_Hal_Wr8(0, REG_GPIO, (0x083 | Ft_Gpu_Hal_Rd8(0, REG_GPIO)));
        #else
            Ft_Gpu_Hal_Wr16(0, REG_GPIOX_DIR, 0xffff);
            Ft_Gpu_Hal_Wr16(0, REG_GPIOX, 0xffff);
        #endif
        #if !defined EVE_SCREEN_CAPACITIVE
            Ft_Gpu_Hal_Wr16(0, REG_TOUCH_RZTHRESH, RESISTANCE_THRESHOLD);
        #endif
            // It is optional to clear the screen here
            //
            {
                // Boot up for FT800 followed by graphics primitive sample cases
                // Initial boot up DL - make the back ground green color
                const unsigned char FT_DLCODE_BOOTUP[12] = 
                {
                0,255,0,2,                                               // GPU instruction CLEAR_COLOR_RGB
                7,0,0,38,                                                // GPU instruction CLEAR
                0,0,0,0,                                                 // GPU instruction DISPLAY
                };
                Ft_Gpu_Hal_WrMem(0, RAM_DL, FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
                Ft_Gpu_Hal_Wr8(0, REG_DLSWAP, DLSWAP_FRAME);
            }
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.120 * SEC), E_INIT_DELAY); // 120ms delay
            iLCD_State = STATE_INITIALISING_5;
            break;
        case STATE_INITIALISING_5:
        #if defined FT_812_ENABLE
            Ft_Gpu_Hal_Wr8(0,  REG_PCLK, 2);                             // the display content is visible after this command
        #endif
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
            fnEventMessage(LCD_PARTNER_TASK, TASK_LCD, E_LCD_INITIALISED); // inform the application that the initialisation has completed
            iLCD_State = STATE_LCD_READY;
            break;
    #elif defined GLCD_INIT && defined TFT2N0369_GLCD_MODE
            GLCD_RST_H();                                                // release the reset line
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.003 * SEC), E_INIT_DELAY); // 3ms delay for display to complete initialisation
            iLCD_State = STATE_INITIALISING;
            return;
        case STATE_INITIALISING:                                         // controller SSD1289 but many registers compatible with SSD2119
            fnCommandGlcd_2(SSD2119_OSC_START_REG,   SSD2119_OSCEN);     // start oscillator
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.015 * SEC), E_INIT_DELAY); // 15ms delay
            iLCD_State = STATE_INITIALISING_1;
            break;
        case STATE_INITIALISING_1:
            fnCommandGlcd_2(SSD2119_PWR_CTRL_1_REG,  0xaeac);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_2_REG,  0x0007);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_3_REG,  0x000f);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_4_REG,  0x2900);
            fnCommandGlcd_2(SSD2119_PWR_CTRL_5_REG,  0x00b3);
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.015 * SEC), E_INIT_DELAY); // 15ms delay
            iLCD_State = STATE_INITIALISING_2;
            break;
        case STATE_INITIALISING_2:
            fnCommandGlcd_2(SSD2119_OUTPUT_CTRL_REG, (SSD2119_RL | SSD2119_REVERSE_DISPLAY | SSD2119_BGR | SSD2119_TB | 0x013f));
            fnCommandGlcd_2(SSD2119_LCD_DRIVE_AC_CTRL_REG, 0x0600);
            fnCommandGlcd_2(SSD2119_SLEEP_MODE_1_REG,0x0000);
            fnCommandGlcd_2(SSD2119_ENTRY_MODE_REG,  0x60b8);            // horizontal display
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.150 * SEC), E_INIT_DELAY); // 150ms delay
            iLCD_State = STATE_INITIALISING_3;
            break;
        case STATE_INITIALISING_3:
            fnCommandGlcd_2(0x0005,                  0x0000);            // compare register 1
            fnCommandGlcd_2(0x0006,                  0x0000);            // compare register 1
            fnCommandGlcd_2(SSD2119_HORIZ_PORCH_REG, 0xef1c);
            fnCommandGlcd_2(SSD2119_VERT_PORCH_REG,  0x0003);
            fnCommandGlcd_2(SSD2119_DISPLAY_CTRL_REG,0x0233);
            fnCommandGlcd_2(SSD2119_FRAME_CYCLE_CTRL_REG,0x5312);
            fnCommandGlcd_2(SSD2119_GATE_SCAN_START_REG,0x0000);
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.020 * SEC), E_INIT_DELAY); // 20ms delay
            iLCD_State = STATE_INITIALISING_4;
            break;
        case STATE_INITIALISING_4:
            fnCommandGlcd_2(SSD2119_V_SCROLL_CTRL_REG_1,0x0000);
            fnCommandGlcd_2(SSD2119_V_SCROLL_CTRL_REG_2,0x0000);
            fnCommandGlcd_2(SSD2119_FIRST_SCRN_X_POS_REG,0x0000);
            fnCommandGlcd_2(SSD2119_FIRST_SCRN_Y_POS_REG,(GLCD_X - 1));
            fnCommandGlcd_2(SSD2119_V_RAM_POS_REG,   (((GLCD_Y - 1) << 8)));
            fnCommandGlcd_2(SSD2119_H_RAM_START_REG, 0x0000);
            fnCommandGlcd_2(SSD2119_H_RAM_END_REG,   (GLCD_X - 1));
            fnCommandGlcd_2(SSD2119_SECND_SCRN_X_POS_REG,0x0000);
            fnCommandGlcd_2(SSD2119_SECND_SCRN_Y_POS_REG,0x0000);
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.020 * SEC), E_INIT_DELAY); // 20ms delay
            iLCD_State = STATE_INITIALISING_5;
            break;
        case STATE_INITIALISING_5:
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_1_REG,0x0707);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_2_REG,0x0704);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_3_REG,0x0204);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_4_REG,0x0201);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_5_REG,0x0203);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_6_REG,0x0204);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_7_REG,0x0204);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_8_REG,0x0502);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_9_REG,0x0302);
            fnCommandGlcd_2(SSD2119_GAMMA_CTRL_10_REG,0x0500);
            fnCommandGlcd_2(0x0023,         0x0000);                     // ram write data mask 1
            fnCommandGlcd_2(0x0024,         0x0000);                     // ram write data mask 2
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.031 * SEC), E_INIT_DELAY); // 31ms delay
            iLCD_State = STATE_INITIALISING_6;
            break;
        case STATE_INITIALISING_6:
            fnSetWindow(0, 0, (GLCD_X - 1), (GLCD_Y - 1));
            fnCommandGlcd_2(SSD2119_X_RAM_ADDR_REG, 0);                  // set address counter
            fnCommandGlcd_2(SSD2119_Y_RAM_ADDR_REG, 0);
            fnCommandGlcd_1(SSD2119_RAM_DATA_REG);                       // set data mode

            fnClearScreen(0);
            if (iLCD_State == STATE_LCD_CLEARING_DISPLAY) {
                break;
            }
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
    #elif defined GLCD_INIT && defined MB785_GLCD_MODE                   // {2} the first contact with the display takes place after at least 50ms after reset
            // Start initial sequence
            //
            fnCommandGlcd_2(0xe5, 0x8000);                               // set the internal vcore voltage - unknown command!
            fnCommandGlcd_2(START_OSCILLATION,  0x0001);                 // start internal OSC.
            fnCommandGlcd_2(DRIVE_OUTPUT_CONTROL_1,  0x0100);            // set SS and SM bit
            fnCommandGlcd_2(LCD_DRIVING_CONTROL,  0x0700);               // set 1 line inversion
            fnCommandGlcd_2(ENTRY_MODE,  0x1030);                        // set GRAM write direction and BGR = 1
            fnCommandGlcd_2(RESIZE_CONTROL,  0x0000);                    // resize register
            fnCommandGlcd_2(DISPLAY_CONTROL_2,  0x0202);                 // set the back porch and front porch
            fnCommandGlcd_2(DISPLAY_CONTROL_3,  0x0000);                 // set non-display area refresh cycle ISC[3:0]
            fnCommandGlcd_2(DISPLAY_CONTROL_4, 0x0000);                  // FMARK function
            fnCommandGlcd_2(RGB_DISPLAY_INTERFACE_CONTROL_1, 0x0000);    // RGB interface setting
            fnCommandGlcd_2(FRAME_MARKER_POSITION, 0x0000);              // frame marker Position
            fnCommandGlcd_2(RGB_DISPLAY_INTERFACE_CONTROL_2, 0x0000);    // RGB interface polarity
            // Continue with the power on sequence
            //
            fnCommandGlcd_2(POWER_CONTROL_1, 0x0000);                    // SAP, BT[3:0], AP, DSTB, SLP, STB
            fnCommandGlcd_2(POWER_CONTROL_2, 0x0000);                    // DC1[2:0], DC0[2:0], VC[2:0]
            fnCommandGlcd_2(POWER_CONTROL_3, 0x0000);                    // VREG1OUT voltage
            fnCommandGlcd_2(POWER_CONTROL_4, 0x0000);                    // VDV[4:0] for VCOM amplitude
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.200 * SEC), E_INIT_DELAY); // dis-charge capacitor power voltage (200ms)
            iLCD_State = STATE_INITIALISING_1;
            return;
        case STATE_INITIALISING_1:
            fnCommandGlcd_2(POWER_CONTROL_1, 0x17b0);                    // SAP, BT[3:0], AP, DSTB, SLP, STB
            fnCommandGlcd_2(POWER_CONTROL_2, 0x0137);                    // DC1[2:0], DC0[2:0], VC[2:0]
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050 * SEC), E_INIT_DELAY); // delay 50ms
            iLCD_State = STATE_INITIALISING_2;
            return;
        case STATE_INITIALISING_2:
            fnCommandGlcd_2(POWER_CONTROL_3, 0x0139);                    // VREG1OUT voltage
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050 * SEC), E_INIT_DELAY); // delay 50ms
            iLCD_State = STATE_INITIALISING_3;
            return;
        case STATE_INITIALISING_3:
            fnCommandGlcd_2(POWER_CONTROL_4, 0x1d00);                    // VDV[4:0] for VCOM amplitude
            fnCommandGlcd_2(0x29, 0x0013);                               // VCM[4:0] for VCOMH
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(0.050 * SEC), E_INIT_DELAY); // delay 50ms
            iLCD_State = STATE_INITIALISING_4;
            return;
        case STATE_INITIALISING_4:
            // Adjust the gamma curve to achieve acceptable image colors
            //
            fnCommandGlcd_2(GAMMA_CONTROL_1, 0x0000);
            fnCommandGlcd_2(GAMMA_CONTROL_2, 0x0506);
            fnCommandGlcd_2(GAMMA_CONTROL_3, 0x0104);
            fnCommandGlcd_2(GAMMA_CONTROL_4, 0x0207);
            fnCommandGlcd_2(GAMMA_CONTROL_5, 0x000f);
            fnCommandGlcd_2(GAMMA_CONTROL_6, 0x0306);
            fnCommandGlcd_2(GAMMA_CONTROL_7, 0x0102);
            fnCommandGlcd_2(GAMMA_CONTROL_8, 0x0707);
            fnCommandGlcd_2(GAMMA_CONTROL_9, 0x0702);
            fnCommandGlcd_2(GAMMA_CONTROL_10,0x1604);
            // Set GRAM area
            //
            fnCommandGlcd_2(0x50, 0x0000);                               // horizontal GRAM Start Address
            fnCommandGlcd_2(0x51, 0x00ef);                               // horizontal GRAM End Address
            fnCommandGlcd_2(0x52, 0x0000);                               // vertical GRAM Start Address
            fnCommandGlcd_2(0x53, 0x013f);                               // vertical GRAM End Address
            fnCommandGlcd_2(0x60, 0x2700);                               // gate scan line
            fnCommandGlcd_2(0x61, 0x0001);                               // NDL,VLE, REV
            fnCommandGlcd_2(0x6a, 0x0000);                               // set scrolling line            

            fnCommandGlcd_2(0x80, 0x0000);                               // partial display control
            fnCommandGlcd_2(0x81, 0x0000);
            fnCommandGlcd_2(0x82, 0x0000);
            fnCommandGlcd_2(0x83, 0x0000);
            fnCommandGlcd_2(0x84, 0x0000);
            fnCommandGlcd_2(0x85, 0x0000);

            fnCommandGlcd_2(0x90, 0x0010);                               // panel control
            fnCommandGlcd_2(0x92, 0x0000);
            fnCommandGlcd_2(0x93, 0x0003);
            fnCommandGlcd_2(0x95, 0x0110);
            fnCommandGlcd_2(0x97, 0x0000);
            fnCommandGlcd_2(0x98, 0x0000);

            fnCommandGlcd_2(ENTRY_MODE, 0x1018);
            fnCommandGlcd_2(DISPLAY_CONTROL_1, 0x0170);                  // 262k color

            fnClearScreen(0);
            if (iLCD_State == STATE_LCD_CLEARING_DISPLAY) {
                break;
            }
        #if defined SUPPORT_TOUCH_SCREEN
            fnStartTouch();
        #endif
        #if defined GLCD_BACKLIGHT_CONTROL
            fnSetBacklight();
        #endif
    #endif
#endif
