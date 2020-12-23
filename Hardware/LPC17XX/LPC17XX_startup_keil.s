;**********************************************************************
;  Mark Butcher    Bsc (Hons) MPhil MIET
;
;  M.J.Butcher Consulting
;  Birchstrasse 20f,    CH-5406, R�tihof
;  Switzerland
;
;  www.uTasker.com    Skype: M_J_Butcher
;
;  ---------------------------------------------------------------------
;  File:        LPC17XX_startup_keil.s [NXP]
;  Project:     Single Chip Embedded Internet
;  ---------------------------------------------------------------------
;  Copyright (C) M.J.Butcher Consulting 2004..2011
;  *********************************************************************
;
;  Normally there is no assembler startup code required when using the
;  LPC17XX. However Keil requires stack pointer and heap settings to be
;  assigned in a routine called	__user_initial_stackheap. This needs
;  to put some values into specific registers. The inline assembler
;  doesn't work in Thumb mode (which the Cortex M3 uses exclusively) and
;  so this has been left here in case someone wishes to use it.
;
;  In addition - due to the same reason - the command to move to sleep
;  mode is also here
;
;  Note that the heap is not used in the uTasker project (size 0)
;  but can be configured for the Keil library use here.
;  The stack values are not initialised since the uTasker
;  startup code sets it to the top of RAM and it can be 
;  left there (see const RESET_VECTOR reset_vect in 
;  LPC17XX.c)

Stack   EQU     0x00000100
Heap    EQU     0x00000000

; space for heap can be allocated here
;
        AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
HeapMem
        SPACE   Heap
__heap_limit

        PRESERVE8

        AREA    |.text|, CODE, READONLY

        EXPORT  __user_initial_stackheap
__user_initial_stackheap
        LDR     R0, = HeapMem
;       LDR     R1, = (StackMem + Stack)
        LDR     R2, = (HeapMem + Heap)
;       LDR     R3, = StackMem
        BX      LR

        ALIGN

        EXPORT __sleep_mode
__sleep_mode
        wfi                                                              ; switch CPU to sleep mode
        BX      LR

        ALIGN

        EXPORT start_application
start_application
        ldr sp, [r0,#0]
        ldr pc, [r0,#4]

        ALIGN

        END
