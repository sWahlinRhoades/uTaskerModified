/* iMX_RT_1020.ld                                                         */
/* GCC memory file for the iMX RT 1020 - for use with the uTasker project */
/* Use when running from FlexSPI Flash (total 256k CPU SRAM)              */



MEMORY
{
    SPI_FLASH (rx)    : ORIGIN = 0x60000000, LENGTH = 0x00800000            /* 8M FlexSPI Flash Flash */
    SRAM_DTC (rwx)    : ORIGIN = 0x20000000, LENGTH = 0x80000               /* 64K bytes (alias RAM) */  
    SRAM_ITC (rwx)    : ORIGIN = 0x300, LENGTH = 0x80000-0x300              /* 64K bytes (alias RAM2) - interrupt vectors located here */  
    SRAM_OC (rwx)     : ORIGIN = 0x20200000, LENGTH = 0x80000               /* 128K bytes (alias RAM3) */  
    BOARD_SDRAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0x2000000             /* 32M bytes (alias RAM4) */ 
}

SECTIONS
{
  __SRAM_segment_start__   = 0x20000000;
  __SRAM_segment_end__     = 0x2000ffff;
  __FLASH_segment_start__  = 0x60000000;
  __FLASH_segment_end__    = 0x600fffff;

  __STACKSIZE__ = 0;
  __HEAPSIZE__ = 0;                                                         /* set to non-zero when using library malloc() type functions */

    /* Image Vector Table and Boot Data for booting from external flash - located at the start of QSPI flash */
	.text : ALIGN(4)
    {
        FILL(0xff)
        __boot_hdr_start__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.conf))
        . = 0x1000 ;
        KEEP(*(.boot_hdr.ivt))
        . = 0x1020 ;
        KEEP(*(.boot_hdr.boot_data))
        . = 0x1030 ;
        KEEP(*(.boot_hdr.dcd_data))
        __boot_hdr_end__ = ABSOLUTE(.) ;
        . = 0x2000 ;
    } > SPI_FLASH

	.text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.vectors))
        *(.after_vectors*)
    } > SPI_FLASH

  __ARM_start__ = ALIGN(__data_load_end__ , 4);
  .ARM.exidx ALIGN(__data_load_end__ , 4) :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > SPI_FLASH
  __ARM_end__ = __exidx_start + SIZEOF(.ARM.exidx);

  __init_load_start__ = __FLASH_segment_start__;
  .init ALIGN(__FLASH_segment_start__ , 4) :
  {
    __init_start__ = .;
    *(.init .init.*)
  } > SRAM_DTC
  __init_end__ = __init_start__ + SIZEOF(.init);


  __text_load_start__ = ALIGN(__init_end__ , 4);
  .text ALIGN(__init_end__ , 4) :
  {
    __text_start__ = .;
    *(.text .text.* .glue_7t .glue_7 .gnu.linkonce.t.*)
  } > SPI_FLASH
  __text_end__ = __text_start__ + SIZEOF(.text) - 0x2000;

 
  __dtors_load_start__ = ALIGN(__text_end__ , 4);
  .dtors ALIGN(__text_end__ , 4) :
  {
    __dtors_start__ = .;
    KEEP (*(SORT(.dtors.*))) KEEP (*(.dtors))
  } > SRAM_DTC
  __dtors_end__ = __dtors_start__ + SIZEOF(.dtors);

 
  __ctors_load_start__ = ALIGN(__dtors_end__ , 4);
  .ctors ALIGN(__dtors_end__ , 4) :
  {
    __ctors_start__ = .;
    KEEP (*(SORT(.ctors.*))) KEEP (*(.ctors))
  } > SRAM_DTC
  __ctors_end__ = __ctors_start__ + SIZEOF(.ctors);

 
   __rodata_load_start__ = ALIGN(__ctors_end__ , 4);
  .rodata ALIGN(__ctors_end__ , 4) :
  {
    __rodata_start__ = .;
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  } > SPI_FLASH
  __rodata_end__ = __rodata_start__ + SIZEOF(.rodata);


  __fast_load_start__ = ALIGN(__rodata_end__ , 4);
  .fast __SRAM_segment_start__ : AT(ALIGN(__rodata_end__ , 4))
  {
    __fast_start__ = .;
    *(.fast .fast.*)
  } > SRAM_DTC
  __fast_end__ = __fast_start__ + SIZEOF(.fast);

  __fast_load_end__ = __fast_load_start__ + SIZEOF(.fast);

 
  .fast_run ALIGN(__fast_end__ , 4) (NOLOAD) :
  {
    __fast_run_start__ = .;
    . = MAX(__fast_run_start__ + SIZEOF(.fast), .);
  } > SRAM_DTC
  __fast_run_end__ = __fast_run_start__ + SIZEOF(.fast_run);

 
  __data_load_start__ = ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4);
  .data __fast_run_end__ : AT(ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4))
  {
    __data_start__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
  } > SRAM_DTC
  __data_end__ = __data_start__ + SIZEOF(.data);

  __data_load_end__ = __data_load_start__ + SIZEOF(.data);

  __FLASH_segment_used_end__ = ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4) + SIZEOF(.data);

  . = ASSERT((__data_load_start__ + SIZEOF(.data)) >= __FLASH_segment_start__ && (__data_load_start__ + SIZEOF(.data)) <= (__FLASH_segment_end__) , "error: .data is too large to fit in FLASH memory segment");

  .data_run ALIGN(__fast_run_end__ , 4) (NOLOAD) :
  {
    __data_run_start__ = .;
    . = MAX(__data_run_start__ + SIZEOF(.data), .);
  } > SRAM_DTC
  __data_run_end__ = __data_run_start__ + SIZEOF(.data_run);

 
  __bss_load_start__ = ALIGN(__data_run_end__ , 4);
  .bss ALIGN(__data_run_end__ , 4) (NOLOAD) :
  {
    __bss_start__ = .;
    *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON)
  } > SRAM_DTC
  __bss_end__ = __bss_start__ + SIZEOF(.bss);

  
  __non_init_load_start__ = ALIGN(__bss_end__ , 4);
  .non_init ALIGN(__bss_end__ , 4) (NOLOAD) :
  {
    __non_init_start__ = .;
    *(.non_init .non_init.*)
  } > SRAM_DTC
  __non_init_end__ = __non_init_start__ + SIZEOF(.non_init);

 
  __heap_load_start__ = ALIGN(__non_init_end__ , 4);
  .heap ALIGN(__non_init_end__ , 4) (NOLOAD) :
  {
    __heap_start__ = .;
    *(.heap)
    . = MAX(__heap_start__ + __HEAPSIZE__ , .);
  } > SRAM_DTC
  __heap_end__ = __heap_start__ + SIZEOF(.heap);

  . = ASSERT(__heap_end__ >= __SRAM_segment_start__ && __heap_end__ <= (__SRAM_segment_end__) , "error: .heap is too large to fit in SRAM memory segment");

  __stack_load_start__ = ALIGN(__heap_end__ , 4);
  .stack ALIGN(__heap_end__ , 4) (NOLOAD) :
  {
    __stack_start__ = .;
    *(.stack)
    . = MAX(__stack_start__ + __STACKSIZE__ , .);
  } > SRAM_DTC
 
  __stack_end__ = __SRAM_segment_end__;
}

