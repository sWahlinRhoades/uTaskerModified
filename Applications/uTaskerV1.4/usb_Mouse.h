/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      usb_Mouse.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************


*/

#if defined MOUSE_DEFINES
    #define MOUSE_INTERVAL                      (DELAY_LIMIT)(0.05 * SEC)
#endif

#if defined MOUSE_HOST_STRUCT

#endif

#if defined MOUSE_HOST_CONST

#endif

#if defined MOUSE_LOCAL_VARIABLES
    static int iUSB_mouse_state = 0;
#endif

#if defined MOUSE_FUNCTIONS
static void fnMouseEnumerated(int iInterface)
{
    iUSB_mouse_state = 1;                                                // mark that the mouse function is active
    uTaskerGlobalMonoTimer(OWN_TASK, MOUSE_INTERVAL, T_MOUSE_ACTION);    // start polling the mouse state
}

static void fnResetMouseDevice(int iInterface)
{
    iUSB_mouse_state = 0;                                                // mark that the mouse function is not active
    uTaskerGlobalStopTimer(OWN_TASK, T_MOUSE_ACTION);                    // stop the mouse sampling timer
}
#endif

/********************************** keyboard timer event handling **********************************/
#if defined MOUSE_DEVICE_TIMER_EVENTS
            case T_MOUSE_ACTION:
                if (iUSB_mouse_state != 0) {
                    static unsigned char ucMouseLastState[4] = {0, 0, 0, 0}; // {24}
                    static unsigned char ucMouseState[4] = {0, 0, 0, 0}; // this must be static since the interrupt IN endpoint uses non-buffered transmission
                    if (MOUSE_LEFT_CLICK()) {
                        ucMouseState[0] |= 0x01;                         // mouse left key held down
                    }
                    else {
                        ucMouseState[0] &= ~0x01;                        // mouse left key releases
                    }
                    if (MOUSE_RIGHT()) {
                        ucMouseState[1]++;                               // increase x coordinate
                    }
                    else if (MOUSE_LEFT()) {
                        ucMouseState[1]--;                               // decrease x coordinate
                    }
                    else {
                        ucMouseState[1] = 0;
                    }
                    if (MOUSE_UP()) {
                        ucMouseState[2]++;                               // increase y coordinate
                    }
                    else if (MOUSE_DOWN()) {
                        ucMouseState[2]--;                               // decrease y coordinate
                    }
                    else {
                        ucMouseState[2] = 0;
                    }
                    if (uMemcmp(ucMouseLastState, ucMouseState, sizeof(ucMouseState)) != 0) { // {24} if change to the mouse state
                        fnWrite(USBPortID_HID_mouse, (unsigned char *)&ucMouseState, sizeof(ucMouseState)); // prepare interrupt IN (non-buffered)
                        uMemcpy(ucMouseLastState, ucMouseState, sizeof(ucMouseState));
                    }
                    uTaskerGlobalMonoTimer(OWN_TASK, MOUSE_INTERVAL, T_MOUSE_ACTION); // next mouse state poll
                }
                break;
#endif

#if defined MOUSE_INTERRUPT_EVENTS

#endif
