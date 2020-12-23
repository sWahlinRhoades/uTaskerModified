/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      iMX_errata.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

// Mask/errata management
//
#if defined iMX_RT102X
    #define ERR011164                                                    // ADC : ADC_ETC fails to clear the ADC_ETC request signals automatically after receiving DMA ack (no fix scheduled)
    #define ERR006032                                                    // FlexCAN : a frame with wrong ID or payload is transmitted into the CAN bus when the Message Buffer under transmission is either aborted or deactivated while the CAN bus is in the Bus Idle state (no fix scheduled)
    #define ERR011207                                                    // FlexSPI : when FLEXSPI_AHBCR[PREFETCHEN] is set, incorrect data can be returned in rare conditions (no fix scheduled)
    #define ERR011096                                                    // SAI: the internal bit clock cannot be generated when BCI = 1 (no fix scheduled)
    #define ERR011150                                                    // SAI: internally generated receive or transmit BCLK cannot be re-enabled if it is first disabled when RCR2[DIV] or TCR2[DIV] > 0 (no fix scheduled)
    #define ERR011165                                                    // SNVS: invalid ECC check failure (no fix scheduled)
#elif defined iMX_RT105X || defined iMX_RT106X
    #define ERR011377                                                    // FlexSPI : FlexSPI DLL lock status bit not accurate due to timing issue (no fix scheduled)
    #define ERR050235                                                    // CCM: incorrect clock setting for CAN affects UART clock gating (no fix scheduled)
    #define ERR010661                                                    // USB: VBUS leakage occurs if USBOTG1 VBUS is on and USBOTG2 VBUS transitions from on to off (no fix scheduled)
#endif
// General errata with no fix scheduled
//
#define ERR006223                                                        // CCM : failure to resume from wait/stop mode with power gating (no fix scheduled)
#define ERR007265                                                        // CCM : when improper low-power sequence is used, the SoC enters low power mode before the Arm core executes WFI (no fix scheduled)
#define ERR011572                                                        // Cortex - M7: write-through stores and loads may return incorrect data (no fix scheduled)
#define ERR005829                                                        // FlexCAN: FlexCAN does not transmit a message that is enabled to be transmitted in a specific moment during the arbitration process (no fix scheduled)
#define ERR009527                                                        // FlexCAN : the transmission abort mechanism may not work properly (no fix scheduled)
#define ERR009595                                                        // FlexCAN: corrupt frame possible if the Freeze Mode or the low-power mode are entered during a bus-off state (no fix scheduled)
#define ERR011377                                                        // FlexSPI : FlexSPI DLL lock status bit not accurate due to timing issue (no fix scheduled)
#define ERR050130                                                        // PIT: temporary incorrect value reported in LMTR64H register in lifetimer mode (no fix scheduled)
#define ERR050194                                                        // QTMR: overflow flag and related interrupt cannot be generated when the timer is configured in upward count mode (no fix scheduled)
#define ERR050144                                                        // SAI: setting FCONT = 1 when TMR > 0 may not function correctly (no fix scheduled)
#define ERR011225                                                        // SEMC: CPU AXI writes to SEMC NAND memory may cause incorrect data programmed into NAND memory (no fix scheduled)
#define ERR006281                                                        // USB: incorrect DP/DN state when only VBUS is applied (no fix scheduled)

// General errata with fix in future silicon
//
#define ERR050101                                                        // USB: endpoint conflict issue in device mode when an isochronous IN endpoint exists (fix in future silicon)
