/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      kinetis_FLASH.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    07.04.2012 Adapt Flash interface for FPU devices                     {10}
    22.07.2012 Add interface for OTP area                                {20}
    23.01.2013 Correct long word programming when simulating non-FPU devices (simulation error introduced with {10}) {31}
    21.03.2013 Add PARAMETER_NO_ALIGNMENT option to parameter flash      {35}
    24.03.2013 Add check of flash write validity when simulating         {36}
    09.07.2013 Implement fnFlashRoutine() as assemble code to avoid possibility of compilers in-lining it {46}
    26.09.2013 Correct flash address when simulating internal flash erase together with other memory types {53}
    28.04.2014 Make some flash drivers conditional on FLASH_ROUTINES so that they can be used without any file system support {79}
    11.05.2014 Add FlexNVM data flash mode                               {82}
    07.11.2014 Add section programming where possible (faster programming of larger Flash blocks) {105}
    12.12.2014 Add power of 2s ATMEL flash support                       {108}
    29.12.2014 Add KE02 EEPROM support                                   {109}
    10.03.2015 Correct KE/KEA flash configuration layout                 {122}
    12.01.2015 Perform write enable for SPI_FLASH_S25FL1_K (for KL/KE driver compatibility) {200}
    19.08.2017 Correct calculation of start address of preceding multiple memory device {201}
    13.10.2017 Replace KINETIS_KE dependancy by FLASH_CONTROLLER_FTMRE dependancy
    07.03.2018 Add 25AA160 SPI EEPROM support                            {202}
    20.08.2018 Add use of block erase command when possible              {203}
    09.11.2018 Add option to not disable interrupts when flash operations are performed [when it is know that this will always be in a different plane to the code] {204}
    09.03.2019 Include generic SPI interface                             {205}
    23.09.2020 Add device offset option                                  {206}
    24.11.2020 Add automatic switch from HSRUN to RUN mode option        {207}

*/


/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

#if !defined SIZE_OF_NVRAM
    #define SIZE_OF_NVRAM 0
#endif

#if defined SPI_SW_UPLOAD || defined SPI_FLASH_FAT || (defined SPI_FILE_SYSTEM && defined FLASH_FILE_SYSTEM)
    #define SPI_FLASH_ENABLED 
#endif
#if (defined SPI_EEPROM_FILE_SYSTEM && defined FLASH_FILE_SYSTEM)
    #define SPI_EEPROM_ENABLED 
#endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

/* =================================================================== */
/*                     global variable definitions                     */
/* =================================================================== */

#if (defined FLASH_ROUTINES || defined ACTIVE_FILE_SYSTEM  || defined USE_PARAMETER_BLOCK || defined INTERNAL_USER_FILES) && defined _WINDOWS
    int iFetchingInternalMemory = _ACCESS_NOT_IN_CODE;
#endif



#if defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK || defined SUPPORT_PROGRAM_ONCE
// This routine runs from SRAM - the reason why the pointer is passed is to avoid the routine taking it from a const value in FLASH, which is then not code location independent
//
    #if defined _WINDOWS || defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS // {204}
        #if defined _WINDOWS && defined USE_SECTION_PROGRAMMING          // {105}
            static unsigned char ucFlexRam[FLEXRAM_SIZE] = {0};
            #undef FLEXRAM_START_ADDRESS
            #define FLEXRAM_START_ADDRESS  ucFlexRam
        #endif
static void fnFlashRoutine(volatile unsigned char *ptrFTFL_BLOCK)
{
#if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined _WINDOWS
    if ((SMC_PMCTRL & SMC_PMCTRL_RUNM_HSRUN) == SMC_PMCTRL_RUNM_HSRUN) {
        _EXCEPTION("Flash writes are not possible in HS RUN mode!!!");
    }
#endif

    *ptrFTFL_BLOCK = FTFL_STAT_CCIF;                                     // launch the command - this clears the FTFL_STAT_CCIF flag (register is FTFL_FSTAT)
    while ((*ptrFTFL_BLOCK & FTFL_STAT_CCIF) == 0) {}                    // wait for the command to terminate
}
    #else                                                                // {46}
static unsigned short fnFlashRoutine[] = {                               // to avoid potential compiler in-lining of the routine (removing position independency) the machine code is used directly
    0x2180,    // MOVS   r1,#0x80                                           load the value 0x80 (command complete interrupt flag) to register r1
    0x7001,    // STRB   r1,[r0,#0x00]                                      write r1 (0x80) to the passed pointer location (r0)
    0x7801,    // LDRB   r1,[r0,#0x00]                                      read back from the same location to r1
    0x0609,    // LSLS   r1,r1,#24                                          shift the register content by 24 bits to the left so that the command complete interrupt flag is at bit 31
    0xd5fc,    // BPL    -4                                                 if the command complete interrupt flag bit is '0' (register content is not negative value) branch back to read its value again
    0x4770     // BX     lr                                                 return from sub-routine
};
    #endif


    #if defined _WINDOWS
static int fnFlashProtected(unsigned long *ptrWord)
{
        #if !defined FLASH_CONTROLLER_FTMRE
    unsigned long ulProtectionBlock = (1 << (((CAST_POINTER_ARITHMETIC)ptrWord) % (SIZE_OF_FLASH / 32)));
    if ((FTFL_FPROT3_0 & ulProtectionBlock) == 0) {
        return 1;                                                        // the area is protected
    }
        #endif
    return 0;                                                            // the area is not protected
}
    #endif

    #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined USE_HIGH_SPEED_RUN_MODE && defined AUTO_RUN_MODE_ON_FLASH
static void fnSetHSRUN_mode(int iHSRUN_mode)                             // {207}
{
    if (iHSRUN_mode != 0) {                                              // move to HSRUN mode
        SMC_PMCTRL |= (SMC_PMCTRL_RUNM_HSRUN);                           // set HSRUN mode
        while (SMC_PMSTAT != SMC_PMSTAT_HSRUN) {                         // ensure that HSRUN mode has been entered before increasing the system frequency
            #if defined _WINDOWS
            SMC_PMSTAT = SMC_PMSTAT_HSRUN;
             #endif
        }
        SIM_CLKDIV1 &= ~SIM_CLKDIV1_SYSTEM_2;                            // HSRUN will be at 120MHz (increased from 60MHz)
    }
    else {
        if ((SMC_PMCTRL & SMC_PMCTRL_RUNM_HSRUN) == SMC_PMCTRL_RUNM_HSRUN) { // if operating in HSRUN mode we set RUN mode so that flashing works
            SIM_CLKDIV1 |= SIM_CLKDIV1_SYSTEM_2;                         // HSRUN will be at 120MHz so reduce to 60MHz
            SMC_PMCTRL &= ~(SMC_PMCTRL_RUNM_HSRUN);                      // set to run mode
            while (SMC_PMSTAT != SMC_PMSTAT_RUN) {                       // ensure that RUN mode has been entered before erasing flash
        #if defined _WINDOWS
                SMC_PMSTAT = SMC_PMSTAT_RUN;
        #endif
            }
        }
    }
}
    #endif

static int fnFlashNow(unsigned char ucCommand, unsigned long *ptrWord, unsigned long *ptr_ulWord)
{
    #if !defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS
    static void (*fnRAM_code)(volatile unsigned char *) = 0;
    #endif
    #if defined FLEXFLASH_DATA || (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {82}{109}
    int iNoInterruptDisable = 0;                                         // the default is to protect programming from interrupts
    #endif
    #if !defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS            // {204}
    if (fnRAM_code == 0) {                                               // the first time this is used it will load the program to SRAM
        #define PROG_WORD_SIZE 30                                        // adequate space for the small program
        int i = 0;
        unsigned char *ptrThumb2 = (unsigned char *)fnFlashRoutine;
        static unsigned short usProgSpace[PROG_WORD_SIZE] = {0};         // make space for the routine in static memory (this will have an even boundary)

        ptrThumb2 =  (unsigned char *)(((CAST_POINTER_ARITHMETIC)ptrThumb2) & ~0x1); // thumb 2 address
        while (i < PROG_WORD_SIZE) {                                     // copy program to SRAM
            usProgSpace[i++] = *(unsigned short *)ptrThumb2;
            ptrThumb2 += sizeof (unsigned short);
        }
        ptrThumb2 = (unsigned char *)usProgSpace;
        ptrThumb2++;                                                     // create a thumb 2 call
        fnRAM_code = (void(*)(volatile unsigned char *))(ptrThumb2);
    #endif
    #if defined FLASH_CONTROLLER_FTMRE                                   // set the flash programming clock to 1MHz once after a reset [most KE and KEA parts]
        #if BUS_CLOCK < 8000000
        _EXCEPTION("Bus clock must be at least 800kHz to program flash!");
        #elif BUS_CLOCK <= 1600000                                       // recommended values depending on bus clock
        FTMRH_FCLKDIV = 0x00;
        #elif BUS_CLOCK <= 2600000
        FTMRH_FCLKDIV = 0x01
        #elif BUS_CLOCK <= 3600000
        FTMRH_FCLKDIV = 0x02;
        #elif BUS_CLOCK <= 4600000
        FTMRH_FCLKDIV = 0x03;
        #elif BUS_CLOCK <= 5600000
        FTMRH_FCLKDIV = 0x04;
        #elif BUS_CLOCK <= 6600000
        FTMRH_FCLKDIV = 0x05;
        #elif BUS_CLOCK <= 7600000
        FTMRH_FCLKDIV = 0x06;
        #elif BUS_CLOCK <= 8600000
        FTMRH_FCLKDIV = 0x07;
        #elif BUS_CLOCK <= 9600000
        FTMRH_FCLKDIV = 0x08;
        #elif BUS_CLOCK <= 10600000
        FTMRH_FCLKDIV = 0x09;
        #elif BUS_CLOCK <= 11600000
        FTMRH_FCLKDIV = 0x0a;
        #elif BUS_CLOCK <= 12600000
        FTMRH_FCLKDIV = 0x0b;
        #elif BUS_CLOCK <= 13600000
        FTMRH_FCLKDIV = 0x0c;
        #elif BUS_CLOCK <= 14600000
        FTMRH_FCLKDIV = 0x0d;
        #elif BUS_CLOCK <= 15600000
        FTMRH_FCLKDIV = 0x0e;
        #elif BUS_CLOCK <= 16600000
        FTMRH_FCLKDIV = 0x0f;
        #elif BUS_CLOCK <= 17600000
        FTMRH_FCLKDIV = 0x10;
        #elif BUS_CLOCK <= 18600000
        FTMRH_FCLKDIV = 0x11;
        #elif BUS_CLOCK <= 19600000
        FTMRH_FCLKDIV = 0x12;
        #elif BUS_CLOCK <= 20600000
        FTMRH_FCLKDIV = 0x13;
        #elif BUS_CLOCK <= 21600000
        FTMRH_FCLKDIV = 0x14;
        #elif BUS_CLOCK <= 22600000
        FTMRH_FCLKDIV = 0x15;
        #elif BUS_CLOCK <= 23600000
        FTMRH_FCLKDIV = 0x16;
        #elif BUS_CLOCK <= 24600000
        FTMRH_FCLKDIV = 0x17;
        #else
        FTMRH_FCLKDIV = 0x18;
        #endif
    #endif
    #if !defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS
    }
    #endif
    _WAIT_REGISTER_FALSE(FTFL_FSTAT, FTFL_STAT_CCIF);                    // wait for previous commands to complete

    #if defined FTFL_STAT_RDCOLERR
    if ((FTFL_FSTAT & (FTFL_STAT_ACCERR | FTFL_STAT_FPVIOL | FTFL_STAT_RDCOLERR)) != 0) { // check for errors in previous command 
        FTFL_FSTAT = (FTFL_STAT_ACCERR | FTFL_STAT_FPVIOL | FTFL_STAT_RDCOLERR); // clear old errors
    }
    #else
    if ((FTFL_FSTAT & (FTFL_STAT_ACCERR | FTFL_STAT_FPVIOL)) != 0) {     // check for errors in previous command 
        FTFL_FSTAT = (FTFL_STAT_ACCERR | FTFL_STAT_FPVIOL);              // clear old errors
    }
    #endif

    #if defined FLASH_CONTROLLER_FTMRE                                   // most KE and KEA parts
    FTMRH_FCCOBIX = 0;                                                   // reset the command index
        #if defined KINETIS_KE_EEPROM && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0) // {109}
    if (ptrWord >= (unsigned long *)(SIZE_OF_FLASH)) {                   // is the access a write/erase in EEPROM
        ptrWord += ((KE_EEPROM_START_ADDRESS - (SIZE_OF_FLASH))/sizeof(unsigned long)); // convert to EEPROM physical address
        if (FCMD_ERASE_FLASH_SECTOR == ucCommand) {
            ucCommand = FCMD_ERASE_EEPROM_SECTOR;                        // modify the command to the EEPROM sector erase command
        }
        iNoInterruptDisable = 1;                                         // this command may execute without interrupts being disabled
    }
        #endif
    #endif

    switch (FTFL_FCCOB0 = ucCommand) {                                   // enter the command sequence
    case FCMD_ERASE_ALL_BLOCKS:                                          // single command word required with the erase all blocks command
    #if defined _WINDOWS
        {
            unsigned char *ptrFlash = 0;
            do {
                fnDeleteFlashSector(fnGetFlashAdd(ptrFlash));            // erase all sectors
                ptrFlash += FLASH_GRANULARITY;
            } while (ptrFlash < (unsigned char *)SIZE_OF_FLASH);
        }
    #endif
        break;
    #if defined USE_SECTION_PROGRAMMING                                  // {105}
    case FCMD_PROGRAM_SECTOR:
    #endif
    #if (defined FLASH_CONTROLLER_FTMRE && defined KINETIS_KE_EEPROM && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109} (some KE and KEA parts)
    case FCMD_PROGRAM_EEPROM:
    case FCMD_ERASE_EEPROM_SECTOR:
    #endif
    #if FLASH_BLOCK_COUNT > 1
    case FCMD_ERASE_FLASH_BLOCK:                                         // {203} block erase
    #endif
    case FCMD_ERASE_FLASH_SECTOR:                                        // sector erase
    case FCMD_PROGRAM:
    #if defined FLEXFLASH_DATA                                           // {82} if working with FlashNMV in data flash mode
        if (ptrWord >= (unsigned long *)(SIZE_OF_FLASH - SIZE_OF_FLEXFLASH)) { // is the access a write/erase in FlexNVM
            ptrWord += ((FLEXNVM_START_ADDRESS - (SIZE_OF_FLASH - SIZE_OF_FLEXFLASH))/sizeof(unsigned long)); // convert to FlexNVM physical address
        }
    #endif
    #if defined SUPPORT_SWAP_BLOCK
    case FCMD_SWAP:
    #endif
        FTFL_FCCOB1 = (unsigned char)(((CAST_POINTER_ARITHMETIC)ptrWord) >> 16); // set address in flash
    #if defined FLEXFLASH_DATA                                           // {82} if working with FlashNMV in data flash mode
        if (((CAST_POINTER_ARITHMETIC)ptrWord & FLEXNVM_START_ADDRESS) != 0) {
            FTFL_FCCOB1 |= 0x80;                                         // set address A23 so that the write/erase is in FlexNVM
            iNoInterruptDisable = 1;                                     // this command may execute without interrupts being disabled
        }
    #endif
    #if defined FLASH_CONTROLLER_FTMRE
        FTMRH_FCCOBIX = 1;                                               // set the command index
    #endif
        FTFL_FCCOB2 = (unsigned char)(((CAST_POINTER_ARITHMETIC)ptrWord) >> 8);
        FTFL_FCCOB3 = (unsigned char)((CAST_POINTER_ARITHMETIC)ptrWord);
        if (FCMD_PROGRAM == ucCommand) {                                 // program long-word aligned value (or phrase)
    #if defined FLASH_CONTROLLER_FTMRE
            FTMRH_FCCOBIX = 2;                                           // set the command index
            FTMRH_FCCOBHI = (unsigned char)(*ptr_ulWord >> 8);
            FTMRH_FCCOBLO = (unsigned char)(*ptr_ulWord);
            FTMRH_FCCOBIX = 3;                                           // set the command index
            FTMRH_FCCOBHI = (unsigned char)(*ptr_ulWord >> 24);
            FTMRH_FCCOBLO = (unsigned char)(*ptr_ulWord >> 16);
    #else
            FTFL_FCCOB7_4 = *ptr_ulWord;                                 // enter the long word to be programmed
    #endif
    #if defined _WINDOWS
            if (*(unsigned long *)fnGetFlashAdd((unsigned char *)ptrWord) != 0xffffffff) { // {36}
                _EXCEPTION("Attempting to write to non-blank flash unit!!");
            }
            if (fnFlashProtected(ptrWord) != 0) {
                FTFL_FSTAT |= FTFL_STAT_FPVIOL;
            }
            else {
                *(unsigned long *)fnGetFlashAdd((unsigned char *)ptrWord) = *ptr_ulWord; // change the value in flash
            }
    #endif
    #if (FLASH_ROW_SIZE == 8)                                            // {10} write second long word
            ptr_ulWord++;
        #if defined FLASH_CONTROLLER_FTMRE
            FTMRH_FCCOBIX = 4;                                           // set the command index
            FTMRH_FCCOBHI = (unsigned char)(*ptr_ulWord >> 8);
            FTMRH_FCCOBLO = (unsigned char)(*ptr_ulWord);
            FTMRH_FCCOBIX = 5;                                           // set the command index
            FTMRH_FCCOBHI = (unsigned char)(*ptr_ulWord >> 24);
            FTMRH_FCCOBLO = (unsigned char)(*ptr_ulWord >> 16);
        #else
            FTFL_FCCOBB_8 = *ptr_ulWord;                                 // enter the second long word to be programmed
        #endif
        #if defined _WINDOWS                                             // {31}
            if (*(unsigned long *)fnGetFlashAdd((unsigned char *)(ptrWord + 1)) != 0xffffffff) { // {36}
                _EXCEPTION("Attempting to write to non-blank flash unit!!");
            }
            *(unsigned long *)fnGetFlashAdd((unsigned char *)(ptrWord + 1)) = *ptr_ulWord;;
        #endif
    #endif
        }
    #if (defined FLASH_CONTROLLER_FTMRE && defined KINETIS_KE_EEPROM && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
        else if (FCMD_PROGRAM_EEPROM == ucCommand) {                     // if programming to EEPROM write a single byte
            FTMRH_FCCOBIX = 2;                                           // set the command index
            FTMRH_FCCOBLO = *(unsigned char *)ptr_ulWord;
        #if defined _WINDOWS
            if (*(unsigned char *)fnGetFlashAdd((unsigned char *)ptrWord) != 0xff) {
                _EXCEPTION("Attempting to write to non-blank EEPROM byte!!");
            }
            *(unsigned char *)fnGetFlashAdd((unsigned char *)ptrWord) = *(unsigned char *)ptr_ulWord;
        #endif
            break;
        }
    #endif
    #if defined USE_SECTION_PROGRAMMING                                  // {105}
        else if (ucCommand == FCMD_PROGRAM_SECTOR) {
            FTFL_FCCOB4 = (unsigned char)(*ptr_ulWord >> 8);             // enter the number of long words/phrases to be programmed
            FTFL_FCCOB5 = (unsigned char)(*ptr_ulWord);
        #if defined _WINDOWS
            {
                unsigned long ulProgrammingCycles = ((FTFL_FCCOB4 << 8) | FTFL_FCCOB5);
                unsigned long ulStartAddress = (FTFL_FCCOB1 << 16);
                ulStartAddress |= (FTFL_FCCOB2 << 8);
                ulStartAddress |= (FTFL_FCCOB3);
                ulProgrammingCycles *= FLASH_ROW_SIZE;                   // number of bytes to be programmed
                if (ulProgrammingCycles > FLEXRAM_MAX_SECTION_COPY_SIZE) {
                    _EXCEPTION("Sector write size is too long");
                }
                memcpy(fnGetFlashAdd((unsigned char *)ulStartAddress), (void *)FLEXRAM_START_ADDRESS, ulProgrammingCycles);
            }
        #endif
        }
    #endif
    #if defined SUPPORT_SWAP_BLOCK
        else if (FCMD_SWAP == ucCommand) {
            FTFL_FCCOB7_4 = *ptr_ulWord;
        }
    #endif
        else {
    #if defined _WINDOWS
        #if FLASH_BLOCK_COUNT > 1                                        // {203}
            if (FCMD_ERASE_FLASH_BLOCK == ucCommand) {
                unsigned char *ptrFlash = fnGetFlashAdd((unsigned char *)ptrWord);
                unsigned long ulSize = 0;
                do {
                    fnDeleteFlashSector(ptrFlash);                       // the sector erase must be phrase aligned ([2:0] = 0) or 128 bit aligned ([3:0] = 0) for some devices
                    ulSize += FLASH_GRANULARITY;
                    ptrFlash += FLASH_GRANULARITY;
                } while (ulSize < FLASH_BLOCK_SIZE);
            }
            else {
                fnDeleteFlashSector(fnGetFlashAdd((unsigned char *)ptrWord)); // the sector erase must be phrase aligned ([2:0] = 0) or 128 bit aligned ([3:0] = 0) for some devices
            }
        #else
            fnDeleteFlashSector(fnGetFlashAdd((unsigned char *)ptrWord));// the sector erase must be phrase aligned ([2:0] = 0) or 128 bit aligned ([3:0] = 0) for some devices
        #endif
    #endif
        }
        break;
    #if defined SUPPORT_BACKDOOR_ACCESS_KEY
    case FCMD_VERIFY_BACKDOOR_ACCESS_KEY:
        FTFL_FCCOB7_4 = *ptr_ulWord++;                                   // key bytes 0..3
        FTFL_FCCOBB_8 = *ptr_ulWord;                                     // key bytes 4..7
        #if defined _WINDOWS
        if ((KINETIS_FLASH_CONFIGURATION_SECURITY & FTFL_FSEC_KEYEN_DISABLED) == FTFL_FSEC_KEYEN_ENABLED) { // if the backdoor access is enabled
            if ((FTFL_FCCOB7_4 == ((BACKDOOR_KEY_0 << 24) | (BACKDOOR_KEY_1 << 16) | (BACKDOOR_KEY_2 << 8) | BACKDOOR_KEY_3)) && // and if the backdoor key matches
                (FTFL_FCCOBB_8 == ((BACKDOOR_KEY_4 << 24) | (BACKDOOR_KEY_5 << 16) | (BACKDOOR_KEY_6 << 8) | BACKDOOR_KEY_7))) {
                FTFL_FSEC |= (FTFL_FSEC_SEC_UNSECURE);                   // unsecured
            }
            else {
                FTFL_FSTAT |= (FTFL_STAT_ACCERR);                        // set access error flag
            }
        }
        #endif
        break;
    #endif
    #if defined SUPPORT_PROGRAM_ONCE                                     // {20}
    case FCMD_READ_ONCE:
        FTFL_FCCOB1 = (unsigned char)(*ptrWord);                         // record index (0..7) for FPU types or (0..15)
        #if defined _WINDOWS
        {
            unsigned char *ptrFlash = fnGetFileSystemStart(0);
            ptrFlash += SIZE_OF_FLASH + ((unsigned char)FTFL_FCCOB1 * FLASH_ROW_SIZE); // move to the location containg this value in simulated flash
            FTFL_FCCOB4 = *ptrFlash++;                                   // load the result registers with the content
            FTFL_FCCOB5 = *ptrFlash++;
            FTFL_FCCOB6 = *ptrFlash++;
            FTFL_FCCOB7 = *ptrFlash++;
            #if (FLASH_ROW_SIZE == 8)
            FTFL_FCCOB8 = *ptrFlash++;
            FTFL_FCCOB9 = *ptrFlash++;
            FTFL_FCCOBA = *ptrFlash++;
            FTFL_FCCOBB = *ptrFlash;
            #endif
        }
        #endif
        break;
    case FCMD_PROGRAM_ONCE:
        FTFL_FCCOB1 = (unsigned char)(*ptrWord);                         // record index (0..7) for FPU types or (0..15)
        FTFL_FCCOB7_4 = *ptr_ulWord;
        #if (FLASH_ROW_SIZE == 8)
        ptr_ulWord++;
        FTFL_FCCOBB_8 = *ptr_ulWord;
        #endif
        #if defined _WINDOWS
        {
            unsigned char cBlank[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
            unsigned char *ptrFlash = fnGetFileSystemStart(0);
            ptrFlash += SIZE_OF_FLASH + ((unsigned char)FTFL_FCCOB1 * FLASH_ROW_SIZE);// move to the location containing this value in simulated flash
            if (uMemcmp(ptrFlash, cBlank, FLASH_ROW_SIZE) == 0) {        // if not yet programmed
                *ptrFlash++ = FTFL_FCCOB4;                               // program the content - this can only be done once
                *ptrFlash++ = FTFL_FCCOB5;
                *ptrFlash++ = FTFL_FCCOB6;
                *ptrFlash++ = FTFL_FCCOB7;
            #if (FLASH_ROW_SIZE == 8)
                *ptrFlash++ = FTFL_FCCOB8;
                *ptrFlash++ = FTFL_FCCOB9;
                *ptrFlash++ = FTFL_FCCOBA;
                *ptrFlash   = FTFL_FCCOBB;
            #endif
            }
            else {
                _EXCEPTION("OTP location already programmed and cannot be modified!!!");
                return FTFL_STAT_FPVIOL;                                 // signal programming error
            }
        }
        #endif
        break;
    #endif
    }
    #if defined FLEXFLASH_DATA || (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {82}{109} if working with FlashNMV in data flash mode or EEPROM
    if (iNoInterruptDisable == 0) {
        uDisable_Interrupt();                                            // protect this region from interrupts
    }
    #elif !defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS          // {204}
    uDisable_Interrupt();                                                // protect this region from interrupts
    #endif
    #if defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS || defined _WINDOWS // {204}
    fnFlashRoutine((volatile unsigned char *)FLASH_STATUS_REGISTER);     // execute the command in flash
    #else
    fnRAM_code((volatile unsigned char *)FLASH_STATUS_REGISTER);         // execute the command from SRAM
    #endif
    #if defined FMC_PFB0CR_                                              // prepared but needs testing
    FMC_PFB0CR |= (FMC_PFBCR_CINV_WAY_0 | FMC_PFBCR_CINV_WAY_1 | FMC_PFBCR_CINV_WAY_2 | FMC_PFBCR_CINV_WAY_3); // invalidate cache to ensure that it has no stale flash content after a flash modification
    #elif defined FMC_PFB01CR_
    FMC_PFB1CR |= (FMC_PFBCR_CINV_WAY_0 | FMC_PFBCR_CINV_WAY_1 | FMC_PFBCR_CINV_WAY_2 | FMC_PFBCR_CINV_WAY_3); // invalidate cache to ensure that it has no stale flash content after a flash modification
    #endif
    #if defined FLEXFLASH_DATA || (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {82}{109} if working with FlashNMV in data flash mode or EEPROM
    if (iNoInterruptDisable == 0) {
        uEnable_Interrupt();                                             // safe to accept interrupts again
    }
    #elif !defined NO_INTERRUPT_DISABLE_DURING_FLASH_OPERATIONS          // {204}
    uEnable_Interrupt();                                                 // safe to accept interrupts again
    #endif

    #if defined SUPPORT_PROGRAM_ONCE                                     // {20}
    if (FCMD_READ_ONCE == ucCommand) {                                   // the result (8 bytes) is in FTFL_FCCOB2/FTFL_FCCOB3
        *ptr_ulWord++ = ((FTFL_FCCOB4 << 24) | (FTFL_FCCOB5 << 16) | (FTFL_FCCOB6 << 8) | FTFL_FCCOB7);
        #if (FLASH_ROW_SIZE == 8)
        *ptr_ulWord   = ((FTFL_FCCOB8 << 24) | (FTFL_FCCOB9 << 16) | (FTFL_FCCOBA << 8) | FTFL_FCCOBB);
        #endif
    }
    #endif
    #if defined SUPPORT_SWAP_BLOCK
    if (FCMD_SWAP == ucCommand) {
        #if defined _WINDOWS
        switch (FTFL_FCCOB4) {
        case SWAP_CONTROL_CODE_REPORT_SWAP_STATUS:
            FTFL_FCCOB5 = CURRENT_SWAP_MODE_UNINITIALISED;               // initial state is usually uninitialised (except when previous operation was interrupted)
            break;
        case SWAP_CONTROL_CODE_INITIALISE_SWAP_SYSTEM:
            {
                unsigned char *ptrFlash = fnGetFileSystemStart(0);
                unsigned long ulOffset = ((FTFL_FCCOB1 << 16) | (FTFL_FCCOB2 << 8) | FTFL_FCCOB3);
                ptrFlash += ulOffset;
                if (*ptrFlash != 0xff) {
                    _EXCEPTION("Swap indicator not in erased state!!");
                }
                *ptrFlash = 0;
                FTFL_FCCOB5 = CURRENT_SWAP_MODE_READY;
            }
            break;
        }
        #endif
        *ptr_ulWord = FTFL_FCCOB7_4;                                     // update with the result
    }
    #endif
    return ((FTFL_FSTAT & (FTFL_STAT_ACCERR | FTFL_STAT_FPVIOL | FTFL_STAT_MGSTAT0)) != 0); // if there was an error this will be non-zero
}
#endif

#if defined SUPPORT_BACKDOOR_ACCESS_KEY
extern int fnBackdoorUnlock(unsigned long Key[2])
{
    return (fnFlashNow(FCMD_VERIFY_BACKDOOR_ACCESS_KEY, 0, Key) != 0);
}
#endif

#if defined SUPPORT_PROGRAM_ONCE                                         // {20}
// Read and write data from the program once area (64 bytes available which can only be programmed once in 4/8 byte blocks)
// The length is the number of long words to be copied to/from the buffer - FPU devices write 8 bytes at a time other 4 bytes at a time
//
extern int fnProgramOnce(int iCommand, unsigned long *ptrBuffer, unsigned char ucBlockNumber, unsigned char ucLength)
{
    #if (FLASH_ROW_SIZE == 8)
    unsigned long ulBuffer[2];                                           // the devices with phrase programming always read/write 8 bytes at a time
    unsigned char ucIndex8;
    #endif
    unsigned char ucMax = (ucLength + ucBlockNumber);                    // finishing block
    unsigned char ucIndex = ucBlockNumber;                               // starting block
    unsigned char ucCommandType;
    if (iCommand == PROGRAM_ONCE_READ) {                                 // read data from the program once area
        ucCommandType = FCMD_READ_ONCE;
    }
    else {                                                               // program (one time programmable)
        ucCommandType = FCMD_PROGRAM_ONCE;
    }
    for (; ucIndex < ucMax; ) {
    #if (FLASH_ROW_SIZE == 8)
        ucIndex8 = (ucIndex/2);
        if (ucCommandType == FCMD_PROGRAM_ONCE) {                        // writing
            if (ucIndex & 1) {                                           // uneven
                ulBuffer[0] = 0xffffffff;
                ulBuffer[1] = *ptrBuffer++;
                ucIndex++;
            }
            else {
                ulBuffer[0] = *ptrBuffer++;
                if (++ucIndex >= ucMax) {
                    ulBuffer[1] = 0xffffffff;
                }
                else {
                    ulBuffer[1] = *ptrBuffer++;
                }
                ucIndex++;
            }
        }
        if (fnFlashNow(ucCommandType, (unsigned long *)&ucIndex8, ulBuffer) != 0) { // read/write 2 long words at a time
            return -1;                                                   // error
        }
        if (ucCommandType == FCMD_READ_ONCE) {                           // reading
            if (ucIndex & 1) {                                           // uneven
                *ptrBuffer++ = ulBuffer[1];
            }
            else {
                *ptrBuffer++ = ulBuffer[0];
                ucIndex++;
                if (ucIndex >= ucMax) {
                    break;
                }
                *ptrBuffer++ = ulBuffer[1];
            }
            ucIndex++;
        }
    #else
        if (fnFlashNow(ucCommandType, (unsigned long *)&ucIndex, ptrBuffer) != 0) { // read/write 1 long word at a time
            return -1;                                                   // error
        }
        ucIndex++;
        ptrBuffer++;
    #endif
    }
    return 0;                                                            // success
}
#endif

#if defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK
// This command is used to erase the complete FLASH contents, including the security area
// Since the complete FLASH will be empty the code will no longer run afterwards
//
extern int fnMassEraseFlash(void)
{
    int iRtn = fnFlashNow(FCMD_ERASE_ALL_BLOCKS, 0, 0);
#if defined _WINDOWS
    if (iRtn == 0) {
        fnResetBoard();
    }
#endif
    return iRtn;
}
#endif


#if defined FLASH_ROUTINES || defined ACTIVE_FILE_SYSTEM || defined USE_PARAMETER_BLOCK
static int fnWriteInternalFlash(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ucData)
{
    #if (defined FLASH_ROUTINES || defined FLASH_FILE_SYSTEM || defined USE_PARAMETER_BLOCK) && (defined FLASH_ROW_SIZE && FLASH_ROW_SIZE > 0)
    static unsigned char *ptrOpenBuffer = 0;
    unsigned char *ptrFlashBuffer;
    unsigned long ulBufferOffset;
    MAX_FILE_LENGTH BufferCopyLength;
    MAX_FILE_LENGTH Length = ptrAccessDetails->BlockLength;
    if (ucData == 0) {                                                   // close an open buffer
        ulBufferOffset = ((CAST_POINTER_ARITHMETIC)ptrOpenBuffer & (FLASH_ROW_SIZE - 1));
        if (ulBufferOffset == 0) {
            return 0;                                                    // no open buffer so nothing to do
        }
        ulBufferOffset = FLASH_ROW_SIZE;                                 // cause the open buffer to be saved without copying any input data
        ptrOpenBuffer = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrOpenBuffer & ~(FLASH_ROW_SIZE - 1));
    }
    else {
        #if (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
        if ((unsigned char *)ptrAccessDetails->ulOffset >= (unsigned char *)(SIZE_OF_FLASH)) { // is the address in EEPROM
            ulBufferOffset = ptrAccessDetails->ulOffset;
        }
        else {
            ptrOpenBuffer = (unsigned char *)(ptrAccessDetails->ulOffset & ~(FLASH_ROW_SIZE - 1)); // set to start of long word or phrase that the address is in
            ulBufferOffset = (ptrAccessDetails->ulOffset & (FLASH_ROW_SIZE - 1)); // offset in the long word or phrase
        }
        #else
        ptrOpenBuffer = (unsigned char *)(ptrAccessDetails->ulOffset & ~(FLASH_ROW_SIZE - 1)); // set to start of long word or phrase that the address is in
        ulBufferOffset = (ptrAccessDetails->ulOffset & (FLASH_ROW_SIZE - 1)); // offset in the long word or phrase
        #endif
    }
        #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined USE_HIGH_SPEED_RUN_MODE && defined AUTO_RUN_MODE_ON_FLASH
    fnSetHSRUN_mode(0);                                                  // if operating in HSRUN mode we set RUN mode so that flashing works
        #endif
    do {                                                                 // handle each byte to be programmed
        #if (defined FLASH_CONTROLLER_FTMRE && defined KINETIS_KE_EEPROM && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
        if ((unsigned char *)ulBufferOffset >= (unsigned char *)(SIZE_OF_FLASH)) { // is the address in EEPROM
            if ((fnFlashNow(FCMD_PROGRAM_EEPROM, (unsigned long *)ulBufferOffset, (unsigned long *)ucData)) != 0) { // program single bytes
                return -1;                                               // error
            }
            ucData++;
            ulBufferOffset++;
            Length--;
            continue;
        }
        #endif
        #if defined USE_SECTION_PROGRAMMING                              // {105}
        if (ulBufferOffset == 0) {                                       // if the data start is aligned there is a possibility of using accelerated section programming
            MAX_FILE_LENGTH SectionLength = (Length & ~(FLASH_ROW_SIZE - 1)); // round down to full long words/phrases
            if (SectionLength > (FLASH_ROW_SIZE * 2)) {                  // from 2 long words or phrases
                unsigned char *ptrEnd = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrOpenBuffer & ~(FLASH_GRANULARITY - 1));
                ptrEnd += FLASH_GRANULARITY;                             // pointer to the next flash sector
                if ((ptrOpenBuffer + SectionLength) >= ptrEnd) {         // end of write is past a sector boundary
                    SectionLength = (ptrEnd - ptrOpenBuffer);            // limit
                }
                if (SectionLength > (FLASH_ROW_SIZE * 2)) {              // if still at least 2 long words or phrases to make section write worthwhile
                    if (SectionLength > FLEXRAM_MAX_SECTION_COPY_SIZE) {
                        SectionLength = FLEXRAM_MAX_SECTION_COPY_SIZE;
                    }
                    uMemcpy((void *)FLEXRAM_START_ADDRESS, ucData, SectionLength); // copy the data to the accelerator RAM
                    ulFlashRow[0] = (SectionLength/FLASH_ROW_SIZE);      // the number of long words/phrases to be written to the section
                    if ((fnFlashNow(FCMD_PROGRAM_SECTOR, (unsigned long *)ptrOpenBuffer, &ulFlashRow[0])) != 0) { // write section
                        return -1;                                       // error
                    }
                    ptrOpenBuffer += SectionLength;
                    Length -= SectionLength;
                    ucData += SectionLength;
                    if (Length == 0) {
                        return 0;
                    }
                    continue;
                }
            }
        }
        #endif
        BufferCopyLength = (MAX_FILE_LENGTH)(FLASH_ROW_SIZE - ulBufferOffset); // remaining buffer space to end of present backup buffer
        if (BufferCopyLength > Length) {                                 // limit in case the amount of bytes to be programmed is less than the long word or phrase involved
            BufferCopyLength = Length;
        }
        ptrFlashBuffer = (unsigned char *)ulFlashRow + ulBufferOffset;   // pointer set in FLASH row backup buffer
        uMemcpy(ptrFlashBuffer, ucData, BufferCopyLength);               // copy the input data to the FLASH row backup buffer
        ucData += BufferCopyLength;
        Length -= BufferCopyLength;                                      // remaining data length
        ptrFlashBuffer += BufferCopyLength;                              // next copy location
        if (ptrFlashBuffer >= ((unsigned char *)ulFlashRow + FLASH_ROW_SIZE)) { // a complete backup buffer is ready to be copied to FLASH
            ptrFlashBuffer = (unsigned char *)ulFlashRow;                // set pointer to start of FLASH row backup buffer
            ulBufferOffset = 0;
            if ((fnFlashNow(FCMD_PROGRAM, (unsigned long *)ptrOpenBuffer, &ulFlashRow[0])) != 0) { // {10} write long word/phrase
                return -1;                                               // error
            }
            ptrOpenBuffer += FLASH_ROW_SIZE;
            uMemset(ulFlashRow, 0xff, FLASH_ROW_SIZE);                   // flush the intermediate buffer
        }
        else {                                                           // incomplete buffer collected
            ptrOpenBuffer += BufferCopyLength;
        }
    } while (Length != 0);
        #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined USE_HIGH_SPEED_RUN_MODE && defined AUTO_RUN_MODE_ON_FLASH
    fnSetHSRUN_mode(1);                                                  // return HSRUN mode after flashing has completed
        #endif
    #endif
    return 0;
}

    #if defined SPI_FLASH_ENABLED                                        // if SPI memory driver required
// Include generic SPI flash routines
// static void fnReadSPI(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static void fnWriteSPI(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static MAX_FILE_LENGTH fnDeleteSPI(ACCESS_DETAILS *ptrAccessDetails);
// static void fnReadSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
// static void fnWriteSPI_EEPROM(ACCESS_DETAILS *ptrAccessDetails, unsigned char *ptrBuffer);
//
        #include "../SPI_Memory/spi_flash_interface.h"                   // {205}
    #endif

    #if !defined ONLY_INTERNAL_FLASH_STORAGE
// Search for the memory type that the starting address is in, return the type and restrict the largest length that can be read,written, erased from that location
//
extern unsigned char fnGetStorageType(unsigned char *memory_pointer, ACCESS_DETAILS *ptrAccessDetails)
{
    unsigned long ulMaximumLength;
    STORAGE_AREA_ENTRY *ptrStorageList = UserStorageListPtr;
    #if defined _WINDOWS                                                 // this is only used when reading with the simulator
    if (iFetchingInternalMemory != _ACCESS_NOT_IN_CODE) {                // if the access is to program memory when simulating
        iFetchingInternalMemory = _ACCESS_NOT_IN_CODE;                   // reset single-shot program access
        return _STORAGE_PROGRAM;
    }
    #endif
    while (ptrStorageList != 0) {                                        // while there are storage areas defined
        if ((memory_pointer >= ptrStorageList->ptrMemoryStart) && (memory_pointer <= ptrStorageList->ptrMemoryEnd)) { // determine in which memory the access starts in
            if (ptrAccessDetails == 0) {
                return ptrStorageList->ucStorageType;
            }
            ptrAccessDetails->ucDeviceNumber = 0;
            if (ptrStorageList->ucDeviceCount > 1) {                     // if storage area is made up of multiple devices (assumed to be all the same size)
                unsigned long ulDeviceRangeLength = ((unsigned long)((ptrStorageList->ptrMemoryEnd - ptrStorageList->ptrMemoryStart) + 1)/ptrStorageList->ucDeviceCount);
                unsigned char *ptrStart = ptrStorageList->ptrMemoryStart;
                unsigned char *ptrEnd = (ptrStorageList->ptrMemoryStart + ulDeviceRangeLength);                
                while (ptrEnd <= memory_pointer) {                       // {201}
                    ptrStart += ulDeviceRangeLength;
                    ptrEnd += ulDeviceRangeLength;
                    ptrAccessDetails->ucDeviceNumber++;                  // the device number that the access is in
                }
                ulMaximumLength = (unsigned long)(ptrEnd - memory_pointer);
                ptrAccessDetails->ulOffset = (unsigned long)(memory_pointer - ptrStart); // offset from start of memory
            }
            else {
                ulMaximumLength = (unsigned long)(ptrStorageList->ptrMemoryEnd - memory_pointer);
                ulMaximumLength++;                                       // compensate the fact that the end pointer is to the end of the last used address (see reason in STORAGE_AREA_ENTRY definition)
                ptrAccessDetails->ulOffset = (unsigned long)(memory_pointer - ptrStorageList->ptrMemoryStart); // offset from start of memory
            }
            if (ulMaximumLength < ptrAccessDetails->BlockLength) {
                ptrAccessDetails->BlockLength = (MAX_FILE_LENGTH)ulMaximumLength; // limit the length to the present storage device
            }
        #if defined STORAGE_DEVICE_OFFSET                                // {206}
            ptrAccessDetails->ucDeviceNumber += ptrStorageList->ucDeviceOffset;
        #endif
            return ptrStorageList->ucStorageType;                        // the storage area type
        }
        ptrStorageList = (STORAGE_AREA_ENTRY *)(ptrStorageList->ptrNext);// move to next storage area
    };
    _EXCEPTION("Attempted access to unsupported storage area!!!!");
    return _STORAGE_INVALID;                                             // attempted access outside of the defined storage areas
}
    #endif

// Erase FLASH sector(s). The pointer can be anywhere in the sector to be erased.
// If the length signifies multiple sectors, each one necessary is erased.
//
extern int fnEraseFlashSector(unsigned char *ptrSector, MAX_FILE_LENGTH Length)
{
#if (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
    unsigned short _FLASH_GRANULARITY = FLASH_GRANULARITY;               // assume program flash granularity
#else
    #define _FLASH_GRANULARITY FLASH_GRANULARITY
#endif
    #if defined MANAGED_FILES
    MAX_FILE_LENGTH OriginalLength = Length;
    #endif
    #if !defined ONLY_INTERNAL_FLASH_STORAGE
    ACCESS_DETAILS AccessDetails;
    do {
        AccessDetails.BlockLength = Length;
        switch (fnGetStorageType(ptrSector, &AccessDetails)) {           // get the storage type based on the memory location and also return the largest amount of data that can be read from a single device
        case _STORAGE_INTERNAL_FLASH:
            Length += (MAX_FILE_LENGTH)(((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(_FLASH_GRANULARITY - 1)));
            ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(_FLASH_GRANULARITY - 1)); // set to sector boundary
        #if FLASH_BLOCK_COUNT > 1                                        // {203} check whether a block erase can be performed
            if (((CAST_POINTER_ARITHMETIC)ptrSector & (FLASH_BLOCK_SIZE - 1)) == 0) { // if the start address is on a block boundary
                if (Length >= FLASH_BLOCK_SIZE) {                        // and the erase length encloses a block
                    if ((fnFlashNow(FCMD_ERASE_FLASH_BLOCK, (unsigned long *)ptrSector, (unsigned long)0)) != 0) { // erase a single block
                        return -1;                                       // error
                    }
                    if (Length <= FLASH_BLOCK_SIZE) {                    // if completed
                        break;
                    }
                    ptrSector += FLASH_BLOCK_SIZE;
                    Length -= (MAX_FILE_LENGTH)FLASH_BLOCK_SIZE;
                    continue;
                }
            }
        #endif
            if ((fnFlashNow(FCMD_ERASE_FLASH_SECTOR, (unsigned long *)(unsigned long)ptrSector, (unsigned long)0)) != 0) { // {53}
                return -1;                                               // error
            }
            AccessDetails.BlockLength = _FLASH_GRANULARITY;
            break;
        #if defined SPI_FLASH_ENABLED
        case _STORAGE_SPI_FLASH:
            {
                MAX_FILE_LENGTH PageBoundaryOffset = (MAX_FILE_LENGTH)(((CAST_POINTER_ARITHMETIC)(AccessDetails.ulOffset))%SPI_FLASH_PAGE_LENGTH); // {50}
                Length += PageBoundaryOffset;                            // include length back to start of page
                ptrSector -= PageBoundaryOffset;                         // set to page boundary
                AccessDetails.BlockLength = fnDeleteSPI(&AccessDetails); // delete page/block in SPI flash 
            }
            break;
        #endif
        #if defined SPI_EEPROM_ENABLED                                   // {202}
        case _STORAGE_SPI_EEPROM:
        {
            unsigned char ucErased[SPI_EEPROM_PAGE_LENGTH];
            MAX_FILE_LENGTH PageBoundaryOffset = (MAX_FILE_LENGTH)(((CAST_POINTER_ARITHMETIC)(AccessDetails.ulOffset))%SPI_EEPROM_PAGE_LENGTH);
            Length += PageBoundaryOffset;                                // include length back to start of page
            ptrSector -= PageBoundaryOffset;                             // set to page boundary
            uMemset(ucErased, 0xff, SPI_EEPROM_PAGE_LENGTH);             // buffer with 0xff
            fnWriteSPI(&AccessDetails, ucErased);                        // delete page in SPI eeprom (by programming to 0xff)
            AccessDetails.BlockLength = SPI_EEPROM_PAGE_LENGTH;
        }
            break;
        #endif
        #if defined I2C_EEPROM_FILE_SYSTEM
        case _STORAGE_I2C_EEPROM:
            {
                CAST_POINTER_ARITHMETIC align_length = (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXTENSION_FILE_SIZE - 1)));
                Length += align_length;
                AccessDetails.BlockLength += align_length;
                ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXTENSION_FILE_SIZE - 1)); // set to sector boundary
                if (fnDeleteI2C_eeprom(&AccessDetails) < 0) {            // delete data in I2C EEPROM
                    return -1;
                }
            }
            break;
        #endif
        #if defined EXT_FLASH_FILE_SYSTEM
        case _STORAGE_PARALLEL_FLASH:
            Length += (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXT_FLASH_PAGE_LENGTH - 1)));
            ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(EXT_FLASH_PAGE_LENGTH - 1)); // set to sector boundary
            AccessDetails.BlockLength = fnDeleteParallelFlash(&AccessDetails); // delete page/block in parallel flash 
            if (AccessDetails.BlockLength == 0) {
                return -1;                                               // error
            }
            break;
        #endif
        default:
            _EXCEPTION("Invalid storage type");
            return -1;                                                   // invalid
        }
        ptrSector += AccessDetails.BlockLength;
        if (Length <= AccessDetails.BlockLength) {                       // if completed
        #if defined MANAGED_FILES
            if (OriginalLength == 0) {                                   // if a single page erase was called, return the page size of the storage area
	            return AccessDetails.BlockLength;
	        }
        #endif
            break;
        }
        Length -= AccessDetails.BlockLength;
    } FOREVER_LOOP();
    #else                                                                // case when only internal Flash is available
        #if (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
    if (ptrSector >= (unsigned char *)(SIZE_OF_FLASH)) {                 // is the sector in EEPROM
        _FLASH_GRANULARITY = KE_EEPROM_GRANULARITY;                      // set EEPROM granularity
    }
        #endif
    Length += (((CAST_POINTER_ARITHMETIC)ptrSector) - ((CAST_POINTER_ARITHMETIC)ptrSector & ~(_FLASH_GRANULARITY - 1))); // increase length to compensate if the address is not on a sector boundary
    ptrSector = (unsigned char *)((CAST_POINTER_ARITHMETIC)ptrSector & ~(_FLASH_GRANULARITY - 1)); // set to sector boundary
        #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined USE_HIGH_SPEED_RUN_MODE && defined AUTO_RUN_MODE_ON_FLASH
    fnSetHSRUN_mode(0);                                                  // if operating in HSRUN mode we set RUN mode so that flashing works
        #endif
    do {
        #if FLASH_BLOCK_COUNT > 1                                        // {203} check whether a block erase can be performed
        if (((CAST_POINTER_ARITHMETIC)ptrSector & (FLASH_BLOCK_SIZE - 1)) == 0) { // if the start address is on a block boundary
            if (Length >= FLASH_BLOCK_SIZE) {                            // and the erase length encloses a block
                if ((fnFlashNow(FCMD_ERASE_FLASH_BLOCK, (unsigned long *)ptrSector, (unsigned long)0)) != 0) { // erase a single block
                    return -1;                                           // error
                }
                if (Length <= FLASH_BLOCK_SIZE) {                        // check whether entire deletion has completed
                    break;
                }
                ptrSector += FLASH_BLOCK_SIZE;                           // advance sector point to next internal flash sector
                Length -= FLASH_BLOCK_SIZE;
                continue;
            }
        }
        #endif
        if ((fnFlashNow(FCMD_ERASE_FLASH_SECTOR, (unsigned long *)ptrSector, (unsigned long)0)) != 0) { // erase a single sector
            return -1;                                                   // error
        }
        if (Length <= _FLASH_GRANULARITY) {                              // check whether entire deletion has completed
        #if defined MANAGED_FILES
            if (OriginalLength == 0) {                                   // if a single page erase was called, return the page size of the storage area
	            return _FLASH_GRANULARITY;
	        }
        #endif
            break;
        }
        ptrSector += _FLASH_GRANULARITY;                                 // advance sector point to next internal flash sector
        Length -= _FLASH_GRANULARITY;
    } FOREVER_LOOP();
    #endif
    #if defined HIGH_SPEED_RUN_MODE_AVAILABLE && defined USE_HIGH_SPEED_RUN_MODE && defined AUTO_RUN_MODE_ON_FLASH
    fnSetHSRUN_mode(1);                                                  // return HSRUN mode after flashing has completed
    #endif
    return 0;                                                            // success
}


extern int fnWriteBytesFlash(unsigned char *ucDestination, unsigned char *ucData, MAX_FILE_LENGTH Length)
{
    ACCESS_DETAILS AccessDetails;
    AccessDetails.BlockLength = Length;
    #if !defined ONLY_INTERNAL_FLASH_STORAGE
    FOREVER_LOOP() {                                                     // {24} allow zero length write to ensure that open flash buffer can be closed
        switch (fnGetStorageType(ucDestination, &AccessDetails)) {       // get the storage type based on the memory location and also return the largest amount of data that can be read from a single device
        case _STORAGE_INTERNAL_FLASH:
            if (fnWriteInternalFlash(&AccessDetails, ucData) != 0) {
                return 1;
            }
            break;
        #if defined SPI_FLASH_ENABLED
        case _STORAGE_SPI_FLASH:
            fnWriteSPI(&AccessDetails, ucData);
            break;
        #endif
        #if defined SPI_EEPROM_ENABLED                                   // {202}
        case _STORAGE_SPI_EEPROM:
            fnWriteSPI_EEPROM(&AccessDetails, ucData);
            break;
        #endif
        #if defined I2C_EEPROM_FILE_SYSTEM
        case _STORAGE_I2C_EEPROM:
            fnWriteI2C_eeprom(&AccessDetails, ucData);
            break;
        #endif
        #if defined EXT_FLASH_FILE_SYSTEM
        case _STORAGE_PARALLEL_FLASH:
            if (fnWriteParallelFlash(&AccessDetails, ucData) != 0) {
                return 1;                                                // error
            }
            break;
        #endif
        default:
            _EXCEPTION("Invalid storage type");
            return 1;                                                    // invalid
        }
        if (Length == AccessDetails.BlockLength) {                       // if write is complete
            return 0;
        }
        Length -= AccessDetails.BlockLength;                             // decrease the amount of data remaining
        ucDestination += AccessDetails.BlockLength;                      // increase the destination address accordingly
        ucData += AccessDetails.BlockLength;                             // increase the buffer address accordingly
        AccessDetails.BlockLength = Length;                              // synchronise individual accesses to remaining length
    }
    return 0;
    #else
    AccessDetails.ulOffset = (unsigned long)ucDestination;
    return (fnWriteInternalFlash(&AccessDetails, ucData));
    #endif
}

extern void fnGetParsFile(unsigned char *ParLocation, unsigned char *ptrValue, MAX_FILE_LENGTH Size)
{
    #if defined _WINDOWS
    if (iFetchingInternalMemory != _ACCESS_NOT_IN_CODE) {
        uMemcpy(ptrValue, ParLocation, Size);
        return;
    }
    #endif
    #if !defined NO_FLASH_SUPPORT
        #if !defined ONLY_INTERNAL_FLASH_STORAGE
    {
        ACCESS_DETAILS AccessDetails;
        while (Size != 0) {
            AccessDetails.BlockLength = Size;                            // synchronise individual accesses to remaining length
            switch (fnGetStorageType(ParLocation, &AccessDetails)) {     // get the storage type based on the memory location and also return the largest amount of data that can be read from a single device
        #if !defined _WINDOWS
            case _STORAGE_INVALID:                                       // read from unknown area is allowed on the HW so that SRAM (for example) could be read without explicity declaring it
        #endif
            case _STORAGE_INTERNAL_FLASH:
                uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), AccessDetails.BlockLength); // read from internal FLASH with no access restrictions so we can simply copy the data
                break;
                #if defined SPI_FLASH_ENABLED
            case _STORAGE_SPI_FLASH:
                fnReadSPI(&AccessDetails, ptrValue);                     // read from the SPI device
                break;
                #endif
                #if defined SPI_EEPROM_ENABLED                           // {202}
            case _STORAGE_SPI_EEPROM:
                fnReadSPI_EEPROM(&AccessDetails, ptrValue);              // read from the SPI eeprom device
                break;
                #endif
                #if defined I2C_EEPROM_FILE_SYSTEM
            case _STORAGE_I2C_EEPROM:
                fnReadI2C_eeprom(&AccessDetails, ptrValue);              // read the data from the external device
                break;
                #endif
                #if defined EXT_FLASH_FILE_SYSTEM
            case _STORAGE_PARALLEL_FLASH:
                uMemcpy(ptrValue, fnGetExtFlashAdd(ParLocation), AccessDetails.BlockLength); // read from external parallel FLASH with no access restrictions so we can simply copy the data
                break;
                #endif
                #if defined _WINDOWS                                     // only when simulating
            case _STORAGE_PROGRAM:
                uMemcpy(ptrValue, ParLocation, Size);                            
                return;
                #endif
            default:
                _EXCEPTION("Invalid storage type");
                return;                                                  // invalid
            }
            if (Size == AccessDetails.BlockLength) {                     // if read is complete
                return;
            }
            Size -= AccessDetails.BlockLength;                           // decrease the amount of data remaining
            ParLocation += AccessDetails.BlockLength;                    // increase the destination address accordingly
            ptrValue += AccessDetails.BlockLength;                       // increase the buffer address accordingly
        }
    }
        #else
            #if defined FLEXFLASH_DATA                                   // {82} if working with FlashNMV in data flash mode
    if (ParLocation >= (unsigned char *)(SIZE_OF_FLASH - SIZE_OF_FLEXFLASH)) { // is the access a read from FlexNVM
        ParLocation += (FLEXNVM_START_ADDRESS - (SIZE_OF_FLASH - SIZE_OF_FLEXFLASH)); // convert to FlexNVM physical address
    }
            #elif (defined FLASH_CONTROLLER_FTMRE && (defined SIZE_OF_EEPROM && SIZE_OF_EEPROM > 0)) // {109}
    if (ParLocation >= (unsigned char *)(SIZE_OF_FLASH)) { // is the access a read from EEPROM
        ParLocation += (KE_EEPROM_START_ADDRESS - (SIZE_OF_FLASH)); // convert to EEPROM physical address
    }
            #endif
    uMemcpy(ptrValue, fnGetFlashAdd(ParLocation), Size);                 // directly copy memory since this must be a pointer to code (embedded file)
        #endif
    #endif
}
#endif

#if defined SUPPORT_SWAP_BLOCK
extern int fnSwapMemory(int iCheck)
{
    #define FLASH_SWAP_INDICATOR_ADDRESS    ((FLASH_START_ADDRESS + (SIZE_OF_FLASH/2)) - 32) // final sector in the first half of flash memory used as flash swap indicator
    unsigned long ulCommand;
    FOREVER_LOOP() {
        ulCommand = ((SWAP_CONTROL_CODE_REPORT_SWAP_STATUS << 24) | 0x00ffffff); // note that the unused bytes in the command are set to 0xff so that it is clear whether they are changed by the operation (failed operations may otherwise not be detectable)
        if (fnFlashNow(FCMD_SWAP, (unsigned long *)FLASH_SWAP_INDICATOR_ADDRESS, &ulCommand) != 0) { // get the swap state
            return SWAP_COMMAND_FAILURE;                                 // error
        }
      //fnDebugHex(ulCommand, (WITH_LEADIN | WITH_CR_LF | sizeof(ulCommand)));
        switch ((unsigned char)(ulCommand >> 16)) {                      // switch on current swap mode
        case CURRENT_SWAP_MODE_UNINITIALISED:
            {
                int iPhrases = 0;
                unsigned long *ptrSwapIndicator = (unsigned long *)FLASH_SWAP_INDICATOR_ADDRESS;
                if (iCheck != 0) {                                       // if checking present state
                    return SWAP_STATE_UNINITIALISED;                     // uninitialised
                }
                while (iPhrases++ < ((FLASH_ROW_SIZE/sizeof(unsigned long)) * 2)) { // for each of the two phrases of each block containing the swap indicator
                    if (*(unsigned long *)fnGetFlashAdd((unsigned char *)ptrSwapIndicator) != 0xffffffff) {
                        fnDebugMsg("Cleaning swap indicator sector\r\n");
                        if ((fnFlashNow(FCMD_ERASE_FLASH_SECTOR, ptrSwapIndicator, (unsigned long)0)) != 0) { // erase the sector
                            return SWAP_ERASE_FAILURE;
                        }
                    }
                    if (iPhrases == (FLASH_ROW_SIZE/sizeof(unsigned long))) { // after checking the two phrases in the first block move to the second block
                        ptrSwapIndicator = (unsigned long *)(FLASH_SWAP_INDICATOR_ADDRESS + (SIZE_OF_FLASH/2)); // location in the second block
                    }
                    else {
                        ptrSwapIndicator++;                              // move to the next long word
                    }
                }
                ulCommand = ((SWAP_CONTROL_CODE_INITIALISE_SWAP_SYSTEM << 24) | 0x00ffffff); // set the flash swap indicator address to initialise the swap mechanism (this address is programmed to IFR swap memory and 0xff00 programmed to the swap indicator address in the first block)
                if (fnFlashNow(FCMD_SWAP, (unsigned long *)FLASH_SWAP_INDICATOR_ADDRESS, &ulCommand) != 0) { // start by initialising the swap process
                    return SWAP_COMMAND_FAILURE;
                }
                while (FTFL_FCCOB5 == CURRENT_SWAP_MODE_UNINITIALISED) {}// the current swap mode may take a little time to complete so wait until the new state is indicated
            }
            break;
        case CURRENT_SWAP_MODE_READY:
            ulCommand = ((SWAP_CONTROL_CODE_SET_SWAP_IN_UPDATE_STATE << 24) | 0x00ffffff);
            if (fnFlashNow(FCMD_SWAP, (unsigned long *)FLASH_SWAP_INDICATOR_ADDRESS, &ulCommand) != 0) {
                return SWAP_COMMAND_FAILURE;                             // error
            }
            while (FTFL_FCCOB5 == CURRENT_SWAP_MODE_READY) {}            // the current swap mode may take a little time to complete so wait until the new state is indicated
            break;
        case CURRENT_SWAP_MODE_UPDATE:
            if (iCheck != 0) {                                           // checking and not swapping
                if ((unsigned char)(ulCommand >> 8) != 0) {              // check the active block
                    return SWAP_STATE_USING_1;                           // block 1 is being used
                }
                else {
                    return SWAP_STATE_USING_0;                           // block 0 is being used
                }
            }
            else {
                // We are performing a swap from one active block to another
                // - this requires erasing the swap indicator of the non-active block
                //
                if ((fnFlashNow(FCMD_ERASE_FLASH_SECTOR, (unsigned long *)(FLASH_SWAP_INDICATOR_ADDRESS + (SIZE_OF_FLASH/2)), (unsigned long)0)) != 0) { // erase the sector
                    return SWAP_ERASE_FAILURE;
                }
            }
            // Fall through intentionally
            //
        case CURRENT_SWAP_MODE_UPDATE_ERASED:
            ulCommand = ((SWAP_CONTROL_CODE_SET_SWAP_IN_COMPLETE_STATE << 24) | 0x00ffffff);
            if (fnFlashNow(FCMD_SWAP, (unsigned long *)FLASH_SWAP_INDICATOR_ADDRESS, &ulCommand) != 0) {
                    return SWAP_COMMAND_FAILURE;
            }
            while (FTFL_FCCOB5 != CURRENT_SWAP_MODE_COMPLETE) {}         // the current swap mode may take a little time to complete so wait until the new state is indicated
            // Fall through intentionally
            //
        case CURRENT_SWAP_MODE_COMPLETE:
            return SWAP_STATE_SWAPPED;                                   // swap has completed - a reset is required to effect the new operation
        default:
            break;                                                       // unexpected - continue until a valid state is returned
        }
    }
    return 0;
}
#endif

#if defined USE_PARAMETER_BLOCK

#define PARAMETER_BLOCK_1 (unsigned long *)(PARAMETER_BLOCK_START)
#define PARAMETER_BLOCK_2 (unsigned long *)(PARAMETER_BLOCK_START + PARAMETER_BLOCK_SIZE)

#if (FLASH_ROW_SIZE == 8)                                                // {10}
    #define PARAMETER_STATUS_SIZE   4                                    // status is 8 bytes long to match the  phrase programming requirement of these parts
#else
    #define PARAMETER_STATUS_SIZE   2                                    // status is 4 bytes long to match the long word programming requirement of these parts
#endif

// Kinetis has 2k (4k for FPU parts) byte blocks in program Flash which can be individually modified so we use one of these, and a backup if desired
// Kinetis KL has 1k byte blocks
//
extern unsigned char fnGetValidPars(unsigned char ucValid)
{
    unsigned long ulValidUse[PARAMETER_STATUS_SIZE];
    unsigned long ulCompare;

    fnGetParsFile((unsigned char *)PARAMETER_BLOCK_1, (unsigned char *)ulValidUse, sizeof(ulValidUse)); // the first 2 long word locations (4 with fast devices) define the validity of the block. 0x55555555 0x55555555 means it is fully valid. 0x55555555 0xffffffff means it contains temporary unvalidated data

    if (PRESENT_TEMP_BLOCK != ucValid) {
        ulCompare = 0x55555555;                                          // we are looking for validated pattern
    }
    else {
        ulCompare = 0xffffffff;                                          // we are looking for temporary pattern
    }

    if ((ulValidUse[0] == 0x55555555) && (ulValidUse[PARAMETER_STATUS_SIZE/2] == ulCompare)) {
        return 1;                                                        // first block matched request
    }
    #if defined USE_PAR_SWAP_BLOCK
    else {
        fnGetParsFile((unsigned char *)PARAMETER_BLOCK_2, (unsigned char *)ulValidUse, sizeof(ulValidUse));
        if ((ulValidUse[0] == 0x55555555) && (ulValidUse[PARAMETER_STATUS_SIZE/2] == ulCompare)) {
            return 2;                                                    // second block matched request
        }
    }
    #endif
    return 0;                                                            // all other cases indicate the searched block doesn't exist
}

// Retrieve parameter from the defined block
//
extern int fnGetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned long *ptrPar = PARAMETER_BLOCK_1;
    #if defined USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar += (PARAMETER_BLOCK_2 - PARAMETER_BLOCK_1);
    }
    #endif

    ptrPar += PARAMETER_STATUS_SIZE;                                     // first parameter starts after validation information
    #if defined PARAMETER_NO_ALIGNMENT                                   // {35}
    ptrPar = (unsigned long *)((unsigned char *)ptrPar + usParameterReference); // move to the location of the parameter using a pointer to bytes
    fnGetParsFile((unsigned char *)ptrPar, ucValue, usLength);
    #else
    ptrPar += usParameterReference;                                      // move to the long word location where the parameter value is stored
        #if (FLASH_ROW_SIZE == 8)
    ptrPar += usParameterReference;                                      // {10} move to double-long word location
        #endif

    while (usLength-- != 0) {
        fnGetParsFile((unsigned char *)ptrPar, ucValue++, 1);
        ptrPar += (PARAMETER_STATUS_SIZE/2);
    }
    #endif
    return 0;
}


// Delete all sectors contained in a parameter block
//
static int fnDeleteParBlock(unsigned char *ptrAddInPar)
{
    return (fnEraseFlashSector(ptrAddInPar, PARAMETER_BLOCK_SIZE));
}


// Write parameters to the defined block
//
extern int fnSetParameters(unsigned char ucValidBlock, unsigned short usParameterReference, unsigned char *ucValue, unsigned short usLength)
{
    unsigned long *ptrPar = PARAMETER_BLOCK_1, *ptrStart;
    unsigned long ulValid = 0x55555555;
    int iBlockUse = 0;
    unsigned long ulValidCheck[PARAMETER_STATUS_SIZE/2];

    if ((TEMP_PARS & ucValidBlock) != 0) {
        ucValidBlock &= ~TEMP_PARS;
        iBlockUse = 1;
    }

    if ((ucValidBlock & BLOCK_INVALID) != 0) {                           // no valid parameter blocks have been found so we can use the first for saving the data.
        fnDeleteParBlock((unsigned char *)PARAMETER_BLOCK_1);            // we delete it to be sure it is fresh
    #if defined USE_PAR_SWAP_BLOCK
        fnDeleteParBlock((unsigned char *)PARAMETER_BLOCK_2);
    #endif
        ucValidBlock = 1;
    }

    #if defined USE_PAR_SWAP_BLOCK
    if (ucValidBlock == 2) {
        ptrPar = PARAMETER_BLOCK_2;
    }
    #endif
    ptrStart = ptrPar;
    ptrPar += PARAMETER_STATUS_SIZE;                                     // first parameter starts after validation information
    #if defined PARAMETER_NO_ALIGNMENT                                   // {35}
    ptrPar = (unsigned long *)((unsigned char *)ptrPar + usParameterReference); // move to the location of the parameter using a pointer to bytes
    #else
    ptrPar += usParameterReference;                                      // move to the long word location where the parameter value is stored
        #if (FLASH_ROW_SIZE == 8)
    ptrPar += usParameterReference;                                      // {10} move to double-long word (phrase) location
        #endif
    #endif

    if (usLength == 0) {                                                 // command rather than normal write
        if (iBlockUse != 0) {                                            // if we have a new set waiting, set it to use state
            iBlockUse = 0;
    #if defined USE_PAR_SWAP_BLOCK
            if (ptrStart == PARAMETER_BLOCK_1) {
                ptrPar = PARAMETER_BLOCK_2;
            }
            else {
                ptrPar = PARAMETER_BLOCK_1;
            }
            fnDeleteParBlock((unsigned char *)ptrPar);
    #endif
        }
        else {                                                           // we must delete the block
            fnDeleteParBlock((unsigned char *)ptrStart);
            if (usParameterReference != 0) {
                return 0;                                                // we don't validate it again
            }
        }
    }
    else {
    #if defined PARAMETER_NO_ALIGNMENT                                   // {35}
        #if defined _WINDOWS                                             // when simulating the write size to internal flash is checked since each write is expected to be a multiple of the flash write size
        if (ptrPar < (unsigned long *)SIZE_OF_FLASH) {                   // parameter memory is in the internal flash
            if ((usLength % FLASH_ROW_SIZE) != 0) {                      // check write block size which must always be a multiple of the flash write size
                _EXCEPTION("When using the option PARAMETER_NO_ALIGNMENT all parameter block writes must be of the correct modularity size");
            }
            if (((CAST_POINTER_ARITHMETIC)ptrPar & (FLASH_ROW_SIZE - 1)) != 0) { // check the alignment of the write
                _EXCEPTION("When using the option PARAMETER_NO_ALIGNMENT all parameter block writes must be aligned to a flash long word/phrase");
            }
        }
        #endif
        fnWriteBytesFlash((unsigned char *)ptrPar, ucValue, usLength);   // it is expected that the user writes blocks of parameter data that is aligned and packed to respect the characteristics of the flash memory being used
    #else
        #if (FLASH_ROW_SIZE == 8)                                        // {10}
        ulValidCheck[1] = 0xffffffff;
        #endif
        while (usLength-- != 0) {
        #if defined _WINDOWS || defined _LITTLE_ENDIAN
            ulValidCheck[0] = (0xffffff00 | *ucValue++);
        #else
            ulValidCheck[0] = (0x00ffffff | (*ucValue++ << 24));
        #endif
            fnWriteBytesFlash((unsigned char *)ptrPar, (unsigned char *)&ulValidCheck[0], sizeof(ulValidCheck)); // write each parameter to FLASH (each byte occupies a long word - 2 long words for fast parts)
            ptrPar += (PARAMETER_STATUS_SIZE/2);
        }
    #endif
    }
    fnGetParsFile((unsigned char *)ptrStart, (unsigned char *)&ulValidCheck, sizeof(ulValidCheck));
    if (ulValidCheck[0] != ulValid) {
        ulValidCheck[0] = ulValid;
        fnWriteBytesFlash((unsigned char *)ptrStart, (unsigned char *)&ulValidCheck[0], sizeof(ulValidCheck)); // validate data
    }
    if (iBlockUse == 0) {
        fnGetParsFile((unsigned char *)(ptrStart + (PARAMETER_STATUS_SIZE/2)), (unsigned char *)&ulValidCheck, sizeof(ulValidCheck));
        if (ulValidCheck[0] != ulValid) {
            ulValidCheck[0] = ulValid;
            fnWriteBytesFlash((unsigned char *)(ptrStart + (PARAMETER_STATUS_SIZE/2)), (unsigned char *)&ulValidCheck[0], sizeof(ulValidCheck)); // use data
        }
    }
    return 0;
}
#endif
