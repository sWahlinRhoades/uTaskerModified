/*
This is the default Startup for STR91x devices for the GNU toolchain
It has been designed by ST Microelectronics and modified by Raisonance.
You can use it, modify it, distribute it freely but without any waranty.

Modified by Mark Butcher for uTasker use. Added FLASH initialisation and __str_disable_interrupt/__str_enable_interrupt  

*/


.extern main


;/* the following are useful for initializing the .data section */
.extern _sidata ;/* start address for the initialization values of the .data section. defined in linker script */
.extern _sdata ;/* start address for the .data section. defined in linker script */
.extern _edata ;/* end address for the .data section. defined in linker script */

;/* the following are useful for initializing the .bss section */
.extern _sbss ;/* start address for the .bss section. defined in linker script */
.extern _ebss ;/* end address for the .bss section. defined in linker script */

;/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */
.set  Mode_USR, 0x10            ;/* User Mode */
.set  Mode_FIQ, 0x11            ;/* FIQ Mode */
.set  Mode_IRQ, 0x12            ;/* IRQ Mode */
.set  Mode_SVC, 0x13            ;/* Supervisor Mode */
.set  Mode_ABT, 0x17            ;/* Abort Mode */
.set  Mode_UNDEF, 0x1B          ;/* Undefined Mode */
.set  Mode_SYS, 0x1F            ;/* System Mode */

.extern VIC0_BASE
.extern VIC1_BASE


.equ  I_Bit, 0x80               ;/* when I bit is set, IRQ is disabled */
.equ  F_Bit, 0x40               ;/* when F bit is set, FIQ is disabled */

/*; --- System memory locations */

;/* init value for the stack pointer. defined in linker script */
.extern _estack

;/* Stack Sizes. The default values are in the linker script, but they can be overriden. */
.extern __stack_und_end__
.extern __stack_svc_end__
.extern __stack_abt_end__
.extern __stack_fiq_end__
.extern __stack_irq_end__
.extern __stack_end__




/*; STR9X register specific definition*/

.equ SCRO_AHB_UNB,     0x5C002034

/***************************************************************************************/

        
       
/*;*******************************************************************************
;* Macro Name     : SaveContext
;* Description    : This macro used to save the context before entering
;                   an exception handler.
;* Input          : The range of registers to store.
;* Output         : none
;*******************************************************************************/
  
       .macro Savecontext $r0,$r12         
        STMFD sp!,{r0-r12,lr}
        MRS    r1,spsr        
        STMFD  sp!,{r1} 
        .endm      
/*;*******************************************************************************
;* Macro Name     : RestoreContext
;* Description    : This macro used to restore the context to return from
;                   an exception handler and continue the program execution.
;* Input          : The range of registers to restore.
;* Output         : none
;*******************************************************************************/
         .macro RestoreContext $r0,$r12  
         LDMFD   sp!,{r1}           /*; Restore the saved spsr_mode into r1.*/
         msr     spsr_cxsf,r1      /*; Restore spsr_mode.*/
         LDMFD   sp!,{r0-r12,pc}^   /*; Return to the instruction following...*/
        .endm                      /*; ...the exception interrupt.*/
                 

.globl _start
.globl _startup
.globl reset_handler
.section .vectors,"ax",%progbits
/*.text*/
reset_handler:
_startup:
_start:

        ldr     PC, Reset_Addr
        ldr     PC, Undefined_Addr
        ldr     PC, SWI_Addr
        ldr     PC, Prefetch_Addr
        ldr     PC, Abort_Addr
        nop                          /*; Reserved vector*/
        ldr     PC, IRQ_Addr
        /*ldr     PC, FIQ_Addr*/
        
      
/*;*******************************************************************************
;* Function Name  : FIQHandler
;* Description    : This function is called when FIQ
;                   exception is entered.
;* Input          : none
;* Output         : none
;******************************************************************************* */

FIQHandler:
        SUB    lr,lr,#4       /*; Update the link register.*/
        SaveContext r0,r7   /* ; Save the workspace plus the current*/
                              /*; return address lr_ fiq and spsr_fiq.*/
        BL      FIQ_Handler   /*; Branch to FIQ_Handler.*/
        RestoreContext r0,r7  /*; Restore the context and return to the...*/
                              /*; ...program execution.*/


       

Reset_Addr      : .long     Reset_Handler
Undefined_Addr  : .long     UndefinedHandler
SWI_Addr        : .long     SWIHandler
Prefetch_Addr   : .long     PrefetchAbortHandler
Abort_Addr      : .long     DataAbortHandler
                  .long 0      /*; Reserved vector*/
IRQ_Addr        : .long     IRQHandler
/*FIQ_Addr        : .long     FIQHandler*/

    
/*;*******************************************************************************
;* Function Name  : UndefinedHandler
;* Description    : This function called when undefined instruction
;                   exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************/

UndefinedHandler:
        SaveContext r0,r12      /*; Save the workspace plus the current*/
         BL      Undefined_Handler/*; Branch to Undefined_Handler.*/
        RestoreContext r0,r12 /*; Return to the instruction following..*/
                              /*; ...the undefined instruction.*/

/*;*******************************************************************************
;* Function Name  : SWIHandler
;* Description    : This function called when SWI instruction executed.
;* Input          : none
;* Output         : none
;*******************************************************************************/

SWIHandler:
        SaveContext r0,r12     /*; Save the workspace plus the current*/
                              /*; return address lr_ svc and spsr_svc.*/
        BL       SWI_Handler  /*; Branch to SWI_Handler.*/
        RestoreContext r0,r12 /*; Return to the instruction following...*/
                              /*; ...the SWI instruction.*/

/*;*******************************************************************************
;* Function Name  : IRQHandler
;* Description    : This function called when IRQ exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************/
 
IRQHandler:
       SUB    lr,lr,#4                ; /*Update the link register*/
       SaveContext r0,r12             ; /*Save the workspace plus the current*/
                                      ; /*return address lr_irq and spsr_irq*/
       LDR    r0, = VIC0_BASE
       LDR    r0, [r0]                ; /*Read the routine address*/
       LDR    r1, = VIC1_BASE
       LDR    r1, [r1]
       ; /*Padding between the acknowledge and re-enable of interrupts*/
       ; /*For more details, please refer to the following URL*/
       ; /*http://www.arm.com/support/faqip/3682.html*/
/*     NOP                            ; remove NOPs since interrupt remains disabled */
/*     NOP                            */
/*     MSR    cpsr_c,#0x1F            ; Switch to SYS mode and enable IRQ*/
       MSR    cpsr_c,#0x9F            ; /*Switch to SYS mode with disabled IRQ*/       
       STMFD  sp!,{lr}                ; /*Save the link register.*/
       LDR    lr, = IRQ_ReturnAddress ; /*Read the return address.*/
/*     MOV    pc, r0                  ; Branch to the IRQ handler.*/
       BX     r0                      ; /*Branch to the IRQ handler - switching mode accordingly */
IRQ_ReturnAddress:
       LDMFD  sp!,{lr}                ; /*Restore the link register.*/
       MSR    cpsr_c,#0xD2 | I_Bit    ; /*Switch to IRQ mode and disable IRQ*/
       LDR    r0, = VIC0_BASE         ; /*Write to the VectorAddress to clear the*/
       STR    r0, [r0]                ; /*respective interrupt in the internal interrupt*/
       LDR    r1, = VIC1_BASE         ; /*Write to the VectorAddressDaisy to clear the*/
       STR    r1,[r1]                 ; /*respective interrupt in the internal interrupt*/
       RestoreContext r0,r12          ; /*Restore the context and return to the...*/
                                      ; /*...program execution.*/
                                                                             
/*;*******************************************************************************
;* Function Name  : PrefetchAbortHandler
;* Description    : This function called when Prefetch Abort
;                   exception is entered.
;* Input          : none
;* Output         : none
;*******************************************************************************/

PrefetchAbortHandler:
        SUB    lr,lr,#4       /* ; Update the link register.*/
        SaveContext r0,r12    /*; Save the workspace plus the current*/
                              /*; return address lr_abt and spsr_abt.*/
        BL     Prefetch_Handler/*; Branch to Prefetch_Handler. */
        RestoreContext r0,r12 /*; Return to the instruction following that... */
                              /*; ...has generated the prefetch abort exception.*/

/*;*******************************************************************************
;* Function Name  : DataAbortHandler
;* Description    : This function is called when Data Abort
;                   exception is entered.
;* Input          : none
;* Output         : none
;********************************************************************************/

DataAbortHandler:
        SUB    lr,lr,#8       /*; Update the link register.*/
        SaveContext r0,r12    /*; Save the workspace plus the current*/
                              /*; return address lr_ abt and spsr_abt.*/
        BL     Abort_Handler  /*; Branch to Abort_Handler.*/
        RestoreContext r0,r12 /*; Return to the instruction following that...*/
                              /*; ...has generated the data abort exception. */

/*;*******************************************************************************
;* Macro Name     : IRQ_to_SYS
;* Description    : This macro used to switch form IRQ mode to SYS mode
;* Input          : none.
;* Output         : none
;*******************************************************************************/
       .macro IRQ_to_SYS

        MSR    cpsr_c,#0x1F
        STMFD  sp!,{lr}
       .endm
       
/*;*******************************************************************************
;* Macro Name     : SYS_to_IRQ
;* Description    : This macro used to switch from SYS mode to IRQ mode
;                   then to return to IRQHnadler routine.
;* Input          : none.
;* Output         : none.
;*******************************************************************************/
       .macro SYS_to_IRQ
        LDMFD  sp!,{lr}      /*; Restore the link register. */
        MSR    cpsr_c,#0xD2  /*; Switch to IRQ mode.*/
        MOV    pc,lr         /*; Return to IRQHandler routine to clear the*/
                             /*; pending bit.*/
       .endm

/***********************************************************************************************/
Reset_Handler:
        ldr    PC, =NextInst
NextInst:
        nop 	                        /* ;execute some instructions after reset while OSC stabilises */
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
                                        /*; enable buffered mode (this is faster but needs to be used carefully when initialising registers). Ensure that BUFFERED_PERIPHERALS is enabled in app_hw_str91xf.h, or remove this initialisation */
        MRC     p15, 0, r0, c1, c0, 0   /*; read CP15 register 1 into r0 */
	    ORR     r0, r0, #0x8            /*; enable Write Buffer on AHB   */
        MCR     p15, 0, r0, c1, c0, 0   /*; write CP15 register 1        */
        
        
/*; --- ARM9 core configuration*/

Enable_SRAM_96K:
   ldr R0, =SCRO_AHB_UNB
   ldr R1, =0x0196                       /*; 96k RAM */
   str R1, [R0]

/* Select the boot bank appropriately for the project */

Enable_FLASH_Boot0:
        LDR R6, =0x54000000
        LDR R7, =0x4
        STR R7, [R6]

        LDR R6, =0x54000004
        LDR R7, =0x3
        STR R7, [R6]

/*
Enable_FLASH_Boot1:
        LDR R6, =0x54000000
        LDR R7, =0x0                 / * ; 32k * /
        STR R7, [R6]

        LDR R6, =0x54000004
        LDR R7, =0x6                 / * ; 512k * /
        STR R7, [R6]
*/        
        
        
        
        
        
        

        LDR R6, =0x5400000C
        LDR R7, =0x0                 /* ; start address 0 */
        STR R7, [R6]

        LDR R6, =0x54000010
        LDR R7, =0x20000             /* ; start at 0x80000 */
        STR R7, [R6]

        LDR R6, =0x54000018
        LDR R7, =0x18                /* ; enable both banks */
        STR R7, [R6]       
        
      /*MRC P15,0,r0,c1,c0,0*/
  
/*; Initialize Stack pointer for various CPU Modes.*/
        msr     CPSR_c, #Mode_FIQ|F_Bit|I_Bit  /*; Change to FIQ mode*/
        ldr     SP, = __stack_fiq_end__    /*; Initialize FIQ stack pointer */
        
        msr     CPSR_c, #Mode_IRQ|F_Bit|I_Bit /*; Change to IRQ mode  */
        ldr     SP, = __stack_irq_end__    /*; Initialize IRQ stack pointer */ 
        
        MSR     CPSR_c, #Mode_ABT|F_Bit|I_Bit /*; Change to ABT mode  */
        LDR     SP, = __stack_abt_end__    /*; Initialize ABT stack pointer */
        
        MSR     CPSR_c, #Mode_UNDEF|F_Bit|I_Bit /*; Change to UNDEF mode  */
        LDR     SP, = __stack_und_end__    /*; Initialize UNDEF stack pointer */
        
        MSR     CPSR_c, #Mode_UNDEF|F_Bit|I_Bit /*; Change to Supervisor mode  */
        LDR     SP, = __stack_svc_end__    /*; Initialize Supervisor stack pointer */                                      
        
        MSR     CPSR_c, #Mode_SYS        /*; Change to System mode, Enable IRQ and FIQ */
        ldr     SP, = __stack_end__    /*; Initialize user stack pointer*/                
        
/* Set bits 17-18 of the Core Configuration Control Register (ensure FLASH programming instruction order) */
        MOV     r0, #0x60000
        MCR     p15,0x1,r0,c15,c1,0        

        /*IMPORT  __main */

	;/* copy the initial values for .data section from FLASH to RAM */
	/*ldr	R1, =_sidata
	ldr	R2, =_sdata
	ldr	R3, =_edata
_reset_inidata_loop:
	cmp	R2, R3
	ldrlO	R0, [R1], #4
	strlO	R0, [R2], #4
	blO	_reset_inidata_loop*/

	;/* Clear the .bss section */
	/*mov   r0,#0						;/ * get a zero * /
	ldr   r1,=_sbss				;/ * point to bss start * /
	ldr   r2,=_ebss				;/ * point to bss end * /
_reset_inibss_loop:
	cmp   r1,r2						;/ * check if some data remains to clear * /
	strlo r0,[r1],#4				;/ * clear 4 bytes * /
	blo   _reset_inibss_loop	;/ * loop until done */
  
        
/************************************************************************************************/
            
                      
/*; --- Now enter the C code */        
        ldr    PC, ___main
___main      : .long     main
     
             
    .code 32
    .global __str_disable_interrupt
__str_disable_interrupt:
_retry:
    mrs r12, CPSR                                                        /* get present SR value      */
    orr r12, r12, #0xc0                                                  /* set IRQ and FIQ mask bits */
    msr CPSR_c, r12                                                      /* set new value to SR       */
    mrs r12, CPSR                                                        /* get new SR value          */
    ands r12, r12, #0xc0                                                 /* mask the IRQ bits         */
    beq _retry                                                           /* loop if not set (can happen if there is an interrupt in last machine cycle) */
    bx lr                                                                /* return with interrupts disabled */
    

    .code 32
    .global __str_enable_interrupt
__str_enable_interrupt:
    mrs r12, CPSR                                                        /* get present SR value      */
    bic r12, r12, #0xC0                                                  /* remove IRQ and FIQ mask bits */
    msr CPSR_c, r12                                                      /* set new value to SR       */
    bx lr                                                                /* return with interrupts enabled */

 
        /*LTORG


        END
;*************
****** (C) COPYRIGHT 2003 STMicroelectronics *****
****** (C) COPYRIGHT 2005 RAISONANCE *****

END OF FILE****/












