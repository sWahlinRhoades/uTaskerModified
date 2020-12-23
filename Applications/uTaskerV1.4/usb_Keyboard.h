/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_Keyboard.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************


*/

#if defined KEYBOARD_DEFINES
    #define KEYBOARD_INTERVAL                   (DELAY_LIMIT)(0.02 * SEC)
#endif

#if defined KEYBOARD_HOST_STRUCT

#endif

#if defined KEYBOARD_HOST_CONST

#endif

#if defined KEYBOARD_LOCAL_VARIABLES
    static CHAR cLastInput = 0;
    static int iUSB_keyboard_state = 0;
#endif

#if defined KEYBOARD_FUNCTIONS
static void fnKeyboardEnumerated(int iInterface)
{
    iUSB_keyboard_state = KEYBOARD_ENUMERATED;                           // mark that the keyboard function is active
    #if !defined IN_COMPLETE_CALLBACK
    uTaskerGlobalMonoTimer(OWN_TASK, KEYBOARD_INTERVAL, T_KEYBOARD_ACTION); // start polling the keyboard state
    #endif
}

static void fnResetKeyboardDevice(int iInterface)
{
    iUSB_keyboard_state = 0;                                             // mark that the keyboard function is not active
    cLastInput = 0;
    #if !defined IN_COMPLETE_CALLBACK
    uTaskerGlobalStopTimer(OWN_TASK, T_KEYBOARD_ACTION);
    #endif
    #if defined USE_MAINTENANCE
    usUSB_state &= ~(ES_USB_KEYBOARD_MODE);                              // keyboard no longer connected
    #endif
    #if defined SUPPORT_FIFO_QUEUES
    if (keyboardQueue != NO_ID_ALLOCATED) {                              // flush the keyboard fifo
        #if !defined SUPPORT_FLUSH
        unsigned char ucFlush;
        do {
        } while (fnRead(keyboardQueue, &ucFlush, 1) != 0);
        #else
        fnFlush(keyboardQueue, FLUSH_RX);
        #endif
    }
    #endif
}

static unsigned char ucLockKeys = 0;
#define NUM_LOCK_FLAG    0x01
#define CAPS_LOCK_FLAG   0x02
#define SCROLL_LOCK_FLAG 0x04
#define LOCK_KEYS        (NUM_LOCK_FLAG | CAPS_LOCK_FLAG | SCROLL_LOCK_FLAG)
static void fnSetKeyboardOutput(unsigned char ucOutputs)                 // assumed to be set report with a single byte content
{
    // Directly change the state of outputs
    //
    ucLockKeys = (ucOutputs & LOCK_KEYS);                                // present lock key states
}

// ucKeyboardState[8] is assumed to be zeroed when called
//
static void fnInsertUSBKeyCode(CHAR cInputCharacter, unsigned char ucKeyboardState[8])
{
    if ((cInputCharacter >= 'A') && (cInputCharacter <= 'Z')) {          // if capital
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter += ('a' - 'A');                                  // convert to miniscule equivalent
    }
    if ((cInputCharacter >= 'a') && (cInputCharacter <= 'z')) {          // if a character key
        cInputCharacter -= ('a' - 4);                                    // convert 'a' to the code 4
    }
    else if ((cInputCharacter >= '1') && (cInputCharacter <= '9')) {     // number (not including '0')
        cInputCharacter -= ('1' - 30);                                   // convert '1' to code 30
    }
    else if (cInputCharacter == '0') {
        cInputCharacter = 39;                                            // code of '0' is 39
    }
    else if (cInputCharacter == ' ') {
        cInputCharacter = 44;                                            // space bar
    }
    else if (TAB_KEY == cInputCharacter) {
        cInputCharacter = 43;                                            // tab key
    }
    else if (CARRIAGE_RETURN == cInputCharacter) {
        cInputCharacter = 40;                                            // return key
    }
    else if (DELETE_KEY == cInputCharacter) {
        cInputCharacter = 42;                                            // backspace
    }
    else if ('!' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 30;
    }
    else if ('@' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 31;
    }
    else if ('#' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 32;
    }
    else if ('$' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 33;
    }
    else if ('%' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 34;
    }
    else if ('^' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 35;
    }
    else if ('&' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 36;
    }
    else if ('*' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 37;
    }
    else if ('(' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 38;
    }
    else if (')' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 39;
    }
    else if ('=' == cInputCharacter) {
        cInputCharacter = 46;
    }
    else if ('+' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 46;
    }
    else if ('[' == cInputCharacter) {
        cInputCharacter = 47;
    }
    else if ('{' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 47;
    }
    else if (']' == cInputCharacter) {
        cInputCharacter = 48;
    }
    else if ('}' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 48;
    }
    else if (0x5c == cInputCharacter) {                                  // '\'
        cInputCharacter = 49;
    }
    else if ('|' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 49;
    }
    else if (',' == cInputCharacter) {
        cInputCharacter = 54;
    }
    else if ('<' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 54;
    }
    else if ('.' == cInputCharacter) {
        cInputCharacter = 55;
    }
    else if ('>' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 55;
    }
    else if ('/' == cInputCharacter) {
        cInputCharacter = 56;
    }
    else if ('?' == cInputCharacter) {
        ucKeyboardState[0] = 0x02;                                       // shift
        cInputCharacter = 56;
    }
    else {
        return;                                                          // others are not yet supported
    }
    ucKeyboardState[2] = (unsigned char)cInputCharacter;                 // key code
}

static void fnKeyboardInput(void)
{
    if (iUSB_keyboard_state == KEYBOARD_ENUMERATED) {                    // if enumerated and not waiting for the previous IN to complete
        static unsigned char ucKeyboardState[8] = {0};                   // this must be static since the interrupt IN endpoint uses non-buffered transmission
        static CHAR cDoubleKey = 0;
        CHAR cSingleInput = 0;
        if (cDoubleKey != 0) {                                           // if a key is being queued (the second in a row with the same value) we insert it now
            cSingleInput = cDoubleKey;
            cDoubleKey = 0;
        }
    #if defined SUPPORT_FIFO_QUEUES
        else if ((keyboardQueue != NO_ID_ALLOCATED) && (fnRead(keyboardQueue, (unsigned char *)&cSingleInput, 1) != 0)) { // extract a single input from the FIFO queue and send a key code if one was found
            if (cSingleInput == cLastInput) {                            // if the same value is received twice in a row we need to send a key release before sending the second so that the host recognises it
                cDoubleKey = cSingleInput;                               // save the key that still needs to be sent
                cSingleInput = 0;                                        // cause a the key relase to be sent
            }
        }
    #endif
    #if !defined IN_COMPLETE_CALLBACK
        else {                                                           // handle mechanical keys if required
        #if defined TEENSY_3_1 || defined TEENSY_LC
            _CONFIG_PORT_INPUT_FAST_LOW(D, (SWITCH_1 | SWITCH_2), PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  (_READ_PORT_MASK(D, SWITCH_2) == 0)      // pin 21 [A6] acts as shift key
            #define KEY_T()     (_READ_PORT_MASK(D, SWITCH_1) == 0)      // pin 20 [A7] acts as 't' key
        #elif defined FRDM_KL25Z
            _CONFIG_PORT_INPUT_FAST_LOW(D, (SWITCH_1), PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  0
            #define KEY_T()     (_READ_PORT_MASK(D, SWITCH_1) == 0)      // switch 1 acts as 't' key
        #elif defined TWR_K24F120M || defined FRDM_K20D50M
            _CONFIG_PORT_INPUT_FAST_LOW(C, (SWITCH_1), PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  0
            #define KEY_T()     (_READ_PORT_MASK(C, SWITCH_1) == 0)      // switch 1 acts as 't' key
        #elif defined TWR_KL25Z48M
            _CONFIG_PORT_INPUT_FAST_LOW(C, (SWITCH_4), PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  0
            #define KEY_T()     (_READ_PORT_MASK(C, SWITCH_4) == 0)      // switch 4 acts as 't' key
        #elif defined FRDM_KL43Z
            _CONFIG_PORT_INPUT_FAST_LOW(C, SWITCH_3, PORT_PS_UP_ENABLE);
            _CONFIG_PORT_INPUT_FAST_LOW(A, SWITCH_1, PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  (_READ_PORT_MASK(A, SWITCH_1) == 0)      // FRDM-KL43Z SW1 acts as shift key
            #define KEY_T()     (_READ_PORT_MASK(C, SWITCH_3) == 0)      // FRDM-KL43Z SW3 acts as 't' key
        #elif defined FRDM_K64F
            _CONFIG_PORT_INPUT_FAST_LOW(C, SWITCH_2, PORT_PS_UP_ENABLE);
            _CONFIG_PORT_INPUT_FAST_LOW(A, SWITCH_3, PORT_PS_UP_ENABLE);
            #define SHIFTKEY()  (_READ_PORT_MASK(C, SWITCH_2) == 0)      // FRDM-K64F SW2 acts as shift key
            #define KEY_T()     (_READ_PORT_MASK(A, SWITCH_3) == 0)      // FRDM-K64F SW3 acts as 't' key
        #endif
        #if defined KEY_T
            if (KEY_T() != 0) {                                          // simple key press
                if (SHIFTKEY() != 0) {                                   // shift also pressed
                    cSingleInput = 'T';
                    fnInsertUSBKeyCode('T', ucKeyboardState);
                }
                else {
                    cSingleInput = 't';
                    fnInsertUSBKeyCode('t', ucKeyboardState);
                }
            }
        #endif
        }
    #endif
        // cSingleInput will be 0 if there was no key being pressed or will have an ASCII value representing a key that has just been pressed or is being held down
        //
        if (cLastInput != cSingleInput) {                                // if there has been a change the new state is sent
            ucKeyboardState[0] = 0;                                      // reset the relevant bytes in the frame
            ucKeyboardState[2] = 0;
            fnInsertUSBKeyCode(cSingleInput, ucKeyboardState);           // convert the ASCII input to a key code
    #if defined IN_COMPLETE_CALLBACK
            iUSB_keyboard_state = (KEYBOARD_ENUMERATED | KEYBOARD_IN_ACTIVE); // mark that there will be a new IN frame prepared
    #endif
            fnWrite(USBPortID_HID_kb, ucKeyboardState, sizeof(ucKeyboardState)); // place the message into the output buffer so that it is sent on next IN poll
            cLastInput = cSingleInput;
        }
    #if !defined IN_COMPLETE_CALLBACK
        uTaskerGlobalMonoTimer(OWN_TASK, KEYBOARD_INTERVAL, T_KEYBOARD_ACTION); // next keyboard state poll (slower than the USB IN polling rate to ensure that the frame has been sent before next input check)
    #endif
    }
}

    #if defined IN_COMPLETE_CALLBACK
        #if defined USB_KEYBOARD_DELAY
// Inter-key-stroke delay has expired
//
static void delayExpired(void)
{
    if (iUSB_keyboard_state == 0) {                                      // if the USB connection has been removed in the meantime
        return;
    }
    iUSB_keyboard_state = KEYBOARD_ENUMERATED;                           // keyboard is no longer waiting for the previous IN to complete and inter-character delay to expire
    fnInterruptMessage(TASK_USB, EVENT_USB_KEYBOARD_INPUT);              // check to see whether there are more keyboard strokes to be sent
}
        #endif
// The previous IN data was successfully transferred (this is an interrupt callback)
//
static void fnKeyboardPolled(unsigned char ucEndpoint)
{
        #if defined USB_KEYBOARD_DELAY
    if (temp_pars->temp_parameters.ucKeyboardInterCharacterDelay > 1) {  // if a minumum inter-character space is defined trigger the next event after the delay
        PIT_SETUP pit_setup;                                             // interrupt configuration parameters
        pit_setup.int_type = PIT_INTERRUPT;
        pit_setup.int_handler = delayExpired;
        pit_setup.int_priority = PIT0_INTERRUPT_PRIORITY;
        pit_setup.count_delay = PIT_MS_DELAY(temp_pars->temp_parameters.ucKeyboardInterCharacterDelay); // set the required single-shot interrupt delay value
        pit_setup.mode = PIT_SINGLE_SHOT;                                // one-shot interrupt
        #if defined SUPPORT_PITS
        pit_setup.ucPIT = 0;                                             // use PIT0
        #endif
        fnConfigureInterrupt((void *)&pit_setup);                        // enter interrupt for PIT0
        return;                                                          
    }
        #endif
    iUSB_keyboard_state = KEYBOARD_ENUMERATED;                           // keyboard is no longer waiting for the previous IN to complete
    fnInterruptMessage(TASK_USB, EVENT_USB_KEYBOARD_INPUT);              // check to see whether there are more keyboard strokes to be sent
}
    #endif
#endif

/********************************** keyboard timer event handling **********************************/
#if defined KEYBOARD_DEVICE_TIMER_EVENTS
            case T_KEYBOARD_ACTION:
                fnKeyboardInput();                                       // handle keyboard input
                break;
#endif

#if defined KEYBOARD_INTERRUPT_EVENTS
    #if defined USE_USB_HID_KEYBOARD && defined IN_COMPLETE_CALLBACK
            case EVENT_USB_KEYBOARD_INPUT:                               // the user has entered new keyboard input
                fnKeyboardInput();                                       // handle the keyboard input
                break;
    #endif
#endif
