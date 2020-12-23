/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland
    
    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_Loader.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */


static void jump_to_application(volatile unsigned long app_link_location) SUBROUTINE_OPTIMISATION_LEVEL1;
#if defined _COMPILE_KEIL
__asm void jump_to_application(volatile unsigned long app_link_location)
{
    mov r1, #0
    msr control, r1
    ldr sp, [r0, #0]
    ldr pc, [r0, #4]
}
#else
static void jump_to_application(volatile unsigned long app_link_location)
{
    #if !defined _WINDOWS                                                // cortex-M3/M4/M7 assembler code
    asm(" mov r1, #0");
    asm(" msr control, r1");                                             // disable potential FPU access flag so that subsequent exceptions do not save FPU registers
    asm(" ldr sp, [r0,#0]");                                             // load the stack pointer value from the program's reset vector
    asm(" ldr pc, [r0,#4]");                                             // load the program counter value from the program's reset vector to cause operation to continue from there
    #endif
}
#endif

// Only the bare-minimum loader calls this function to check the reset cause and maintain reset counters
//
static void fnSaveResetCause(void)
{
    if ((SRC_SRSR & SRC_SRSR_IPP_RESET_B) != 0) {                        // power on reset
        *BOOT_RESET_COUNTER = 0;                                         // delete persistent memory
        *BOOT_WDOG_COUNTER = 0;
        *SECRET_KEY_0 = 0;                                               // delete trace of secret key (tzhe key is written to persistant memory once when set so that the application can monitor it just once for test or tracking purposes)
        *SECRET_KEY_1 = 0;
        *SECRET_KEY_2 = 0;
        *SECRET_KEY_3 = 0;
        *BOOT_MAIL_BOX = DO_FULL_RESET;                                  // initiate full boot cycle
    }
    else {                                                               // not power on reset
#if defined iMX_RT105X || defined iMX_RT106X
        if ((SRC_SRSR & (SRC_SRSR_LOCKUP_SYSRESETREQ | SRC_SRSR_WDOG3_RST_B | SRC_SRSR_WDG_RST_B)) == SRC_SRSR_LOCKUP_SYSRESETREQ) // commanded reset (which retains the boot mailbox content)
#else
        if ((SRC_SRSR & (SRC_SRSR_JTAG_SW_RST | SRC_SRSR_WDOG3_RST_B | SRC_SRSR_WDG_RST_B)) == SRC_SRSR_JTAG_SW_RST) // commanded reset (which retains the boot mailbox content)
#endif
        {
            if ((*BOOT_MAIL_BOX & RESET_NO_COUNT_FLAG) != 0) {           // commanded reset that should not be counted, nor should the last reset cause be updated
                *BOOT_MAIL_BOX = (*BOOT_MAIL_BOX & ~RESET_NO_COUNT_FLAG);// remove the no count flag
                WRITE_ONE_TO_CLEAR(SRC_SRSR, (SRC_SRSR_IPP_RESET_B | SRC_SRSR_LOCKUP_SYSRESETREQ | SRC_SRSR_CSU_RESET_B | SRC_SRSR_IPP_USER_RESET_B | SRC_SRSR_WDG_RST_B | SRC_SRSR_JTAG_RST_B
                    | SRC_SRSR_JTAG_SW_RST | SRC_SRSR_WDOG3_RST_B | SRC_SRSR_TEMPSENSE_RST_B)); // clear reset cause flags
                return;                                                  // return in order to avoid saving reset cause and incrementing reset counters
            }
            else {
                *BOOT_RESET_COUNTER = (*BOOT_RESET_COUNTER + 1);         // count the number of other resets
            }
        }
        else {                                                           // not a commanded reset
            if ((SRC_SRSR & (SRC_SRSR_WDOG3_RST_B | SRC_SRSR_WDG_RST_B)) != 0) { // watchdog reset detected
                unsigned short usWatchdogCount = *BOOT_WDOG_COUNTER;     // original watchdog reset count
                ++usWatchdogCount;                                       // count the number of watchdog resets
                if ((usWatchdogCount % 16) == 0) {                       // every 16th watchdog reset
                    *BOOT_MAIL_BOX = RESET_TO_FALLBACK_LOADER;           // offer the fall-back loader so that potentially bad code can be updated
                }
                else {
                    *BOOT_MAIL_BOX = DO_FULL_RESET;                      // initiate full boot cycle on watchdog reset
                }
                *BOOT_WDOG_COUNTER = usWatchdogCount;                    // save the incremented watchdog reset counter
            }
            else {
                *BOOT_RESET_COUNTER = (*BOOT_RESET_COUNTER + 1);         // count the number of other resets
                *BOOT_MAIL_BOX = DO_FULL_RESET;                          // initiate full boot cycle on non-commanded resets
            }
        }
    }
    *BOOT_RESET_CAUSE = (unsigned short)SRC_SRSR;                        // save the reset cause
    WRITE_ONE_TO_CLEAR(SRC_SRSR, (SRC_SRSR_IPP_RESET_B | SRC_SRSR_LOCKUP_SYSRESETREQ | SRC_SRSR_CSU_RESET_B | SRC_SRSR_IPP_USER_RESET_B | SRC_SRSR_WDG_RST_B | SRC_SRSR_JTAG_RST_B
        | SRC_SRSR_JTAG_SW_RST | SRC_SRSR_WDOG3_RST_B | SRC_SRSR_TEMPSENSE_RST_B)); // clear reset cause flags
}


// This subroutine manipulates the stack pointer and changes the FlexRAM mapping in a way that preserves the stack content that is presently stored
// To ensure that the assembler code operates correctly the routine must never be in-lined
//
    #if defined _COMPILE_IAR
        #pragma inline=never
    #elif defined _GNU
        static void fnAdjustStackPointer(volatile unsigned long *ptrRemap, volatile unsigned long ulFlexRamConfig, volatile signed long slOffset) NEVER_INLINE_SUBROUTINE SUBROUTINE_OPTIMISATION_LEVEL1;
    #endif
    #if defined _COMPILE_KEIL
#pragma no_inline
__asm void fnAdjustStackPointer(volatile unsigned long *ptrRemap, volatile unsigned long ulFlexRamConfig, volatile signed long slOffset)
{
    mrs r11, msp
    add r11, r11, r2
    msr msp, r11
    str r1, [r0]
    dsb
}
    #else
static void fnAdjustStackPointer(volatile unsigned long *ptrRemap, volatile unsigned long ulFlexRamConfig, volatile signed long slOffset)
{
        #if defined _WINDOWS
    *ptrRemap = ulFlexRamConfig;                                         // enable the remapped FlexRAM configuration
        #else
    __asm volatile (
        "    mrs r11, msp                \n"                             // load the present stack pointer
        "    add r11, r11, r2            \n"                             // move to new location after the FlexRAM reconfiguration ( - 0x00200000)
        "    msr msp, r11                \n"                             // set the new stack pointer
        "    str r1, [r0]                \n"                             // enable the remapped FlexRAM configuration
        );
        #endif
    _SYNCHRONISATION_DATA_BARRIER();                                     // ensure that the remap write has terminated before returning
}
    #endif



// This code runs in QSPI flash and configures the flexRAM for subsequent operation
// - only stack variables are used which remain valid after the FlexRAM configuration
//
static unsigned char *fnConfigureFlexRAM(unsigned long ulCodeSize)
{
    unsigned char *ptrTopOfRAM;
    int i = 0;
    unsigned long ulBankConfig = 0;
    register unsigned long ulFlexRamConfig = (IOMUXC_GPR_GPR16_FLEXRAM_BANK_CFG_SEL_CFG | IOMUXC_GPR_GPR16_RESERVED); // select the configuration (rather than from fuse values)
    volatile unsigned long ulRemapAddress = (unsigned long)IOMUXC_GPR_GPR16_ADD;
    volatile register signed long slOffset;
    int iOCRAM_banks = 0;
    int iDTC_banks;
    int iITC_banks;
    #if !defined ENABLE_DATA_CACHE
  //int iOCRAM_banks_needed;
    #endif
    #if defined iMX_RT1015
  //ulCodeSize = (128 * 1024);                                           // the i.MX RT 1015 has 256k internal RAM but the maximum size of each area is limited to 128k - the optimal setup is therefore always 128k ITC and 128k DTC
    #endif
    iITC_banks = ((ulCodeSize + (FLEX_RAM_GRANULARITY  - 1))/FLEX_RAM_GRANULARITY); // the number of RAM banks required for code
    iDTC_banks = (FLEX_RAM_BANKS - iITC_banks);                          // reset for data
    ptrTopOfRAM = (unsigned char *)(RAM_START_ADDRESS_DTC + (iDTC_banks * FLEX_RAM_GRANULARITY));

    // Strategy is to put code into ITCM and allocate remaining banks for DTCM - without using OCRAM
    // - larger programs will need to change the strategy to run partly in QSPI Flash or use other external memory
    // 

    // Configure FlexRAM
    //
    #if SIZE_OF_FLEX_RAM >= (512*1024)                                   // set the highest power of two size for the flexram that is available
    IOMUXC_GPR_GPR14 = (IOMUXC_GPR_GPR14_CM7_CFGITCMSZ_512K | IOMUXC_GPR_GPR14_CM7_CFGDTCMSZ_512K); // define the memory type sizes
    #elif SIZE_OF_FLEX_RAM >= (256*1024)
    IOMUXC_GPR_GPR14 = (IOMUXC_GPR_GPR14_CM7_CFGITCMSZ_256K | IOMUXC_GPR_GPR14_CM7_CFGDTCMSZ_256K); // define the memory type sizes
    #else
    IOMUXC_GPR_GPR14 = (IOMUXC_GPR_GPR14_CM7_CFGITCMSZ_128K | IOMUXC_GPR_GPR14_CM7_CFGDTCMSZ_128K); // define the memory type sizes
    #endif
    if ((iOCRAM_banks + iITC_banks + iDTC_banks) > FLEX_RAM_BANKS) {
        _EXCEPTION("Too many RAM banks defined!");
    }
    #if defined iMX_RT1011
    slOffset = -((1 - iDTC_banks) * FLEX_RAM_GRANULARITY);
    #else
    slOffset = -((FLEX_RAM_BANKS/2 - iDTC_banks) * FLEX_RAM_GRANULARITY);
    #endif
    #if !defined ENABLE_DATA_CACHE
  //iOCRAM_banks_needed = iOCRAM_banks;
    #endif
    // The banks are ordered starting with instruction use, followed by (if used) OCRAM use, and always ending with data use
    //
    if (iDTC_banks != 0) {
        ulFlexRamConfig |= IOMUXC_GPR_GPR16_INIT_DTCM_EN;
    }
    if (iITC_banks != 0) {
        ulFlexRamConfig |= IOMUXC_GPR_GPR16_INIT_ITCM_EN;
    }
    while (iITC_banks-- != 0) {
        ulBankConfig |= (IOMUXC_GPR_GPR17_FLEXRAM_BANK_CFG_BANK0_ITCM << i);
        i += 2;
    }
    while (iOCRAM_banks-- != 0) {
        ulBankConfig |= (IOMUXC_GPR_GPR17_FLEXRAM_BANK_CFG_BANK0_OCRAM << i);
        i += 2;
    }
    while (iDTC_banks-- != 0) {
        ulBankConfig |= (IOMUXC_GPR_GPR17_FLEXRAM_BANK_CFG_BANK0_DTCM << i);
        i += 2;
    }
    POWER_UP_ATOMIC(3, FLEXRAM1_CLOCK);                                  // ensure FlexRAM is clocked
    IOMUXC_GPR_GPR17 = ulBankConfig;                                     // define the bank usage
    _SYNCHRONISATION_DATA_BARRIER();                                     // ensure that the bank configuration write has terminated before continuing

    // The stack pointer is adjusted so that it will have the same content after the FlexRAM remap
    //
    slOffset -= ((RAM_START_ADDRESS - RAM_START_ADDRESS_DTC));           // stack moves from top of original OCRAM/ITC to top of DTC 
    fnAdjustStackPointer((volatile unsigned long *)ulRemapAddress, ulFlexRamConfig, slOffset); // adjust the stack pointer and remap FlexRAM (stack pointer is adjusted so that it is in the same FlexRAM bank and thus has the same content before and after the remap)

    #if !defined ENABLE_DATA_CACHE                                       // only possible when data cache is not used
  //if (iOCRAM_banks_needed == 0) {
  //    POWER_DOWN_ATOMIC(3, OCRAM_CLOCK);                               // no OCRAM used so power down
  //}
    #endif

    // Now the new FlexRAM configuration is valid and the stack content restored so we can continue
    //
    return ptrTopOfRAM;
}

#if defined CRYPTOGRAPHY
typedef struct stALIGNED_KEY
{
    unsigned long  ulLength;                                             // long word to ensure following key alignment
    unsigned char  ucKey[32];                                            // key aligned
} ALIGNED_KEY;


#if defined _COMPILE_IAR
__root const CHAR _decrypt_key[] @ ".boot_hdr.key" // __root forces the function to be linked in IAR project
#elif defined _GNU
const CHAR __attribute__((section(".boot_hdr.key"))) _decrypt_key[]
#elif defined _COMPILE_KEIL
__attribute__((section("_key"))) const CHAR _decrypt_key[]
#else
const CHAR _decrypt_key[]
#endif
= APPLICATION_AES256_SECRET_KEY;

//static const CHAR decrypt_key[] 
    #define PRIME_AES256_INITIAL_VECTOR
    #if defined PRIME_AES256_INITIAL_VECTOR
//static const CHAR initial_vector[] = APPLICATION_AES256_INITIAL_VECTOR;
        #if defined _COMPILE_IAR
__root const CHAR _initial_vector[] @ ".boot_hdr.init" // __root forces the function to be linked in IAR project
        #elif defined _GNU
const CHAR __attribute__((section(".boot_hdr.init"))) _initial_vector[]
        #elif defined _COMPILE_KEIL
__attribute__((section("_init"))) const CHAR _initial_vector[]
        #else
const CHAR _initial_vector[]
        #endif
= APPLICATION_AES256_INITIAL_VECTOR;
    #endif


extern void fnGetKeys(const CHAR **decrypt_key, const CHAR **initial_vector)
{
    static unsigned long ulKeys[iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long)] = {0};
    unsigned long ulCodingKey[iMX_LOADER_KEY_STORAGE_AREA_SIZE/sizeof(unsigned long)];
    uMemset(ulCodingKey, 0, sizeof(ulCodingKey));                        // initialise decoding key array to zero
    uMemcpy(ulKeys, fnGetFlashAdd(ENCRYPTED_KEYS_ADDRESS), sizeof(ulKeys)); // copy encrypted key area content locally
    fnGetDecryptionKey(ulCodingKey);                                     // get a copy of the coding key
    fnDecryptKey(ulKeys, ulCodingKey);                                   // decrypt the key, ready for subsequent use
    *decrypt_key = (CHAR *)ulKeys;                                       // point to the decrypted copy of the AES256 secret key in RAM
    *initial_vector = (CHAR *)&ulKeys[iMX_LOADER_AES256_STORAGE_AREA_SIZE /sizeof(unsigned long)]; // point to the decrypted copy of the initial vector in RAM
}

extern void fnPrepareDecrypt(void)
{
    ALIGNED_KEY decryptKey = {0, {0}};
    const CHAR *decrypt_key = _decrypt_key;
    #if defined PRIME_AES256_INITIAL_VECTOR
    unsigned char initVector[32];
    int iVectorLength;
    const CHAR *initial_vector = _initial_vector;
    #endif
    #if defined _WINDOWS
    decrypt_key = (const CHAR *)fnGetFlashAdd(ORIGINAL_KEYS_ADDRESS);
    #endif
    decryptKey.ulLength = uStrlen(decrypt_key);
    if (decryptKey.ulLength == 0) {                                      // if the secret keys have been encrypted and relocated we need to decrypt them before use
        fnGetKeys(&decrypt_key, &initial_vector);
        decryptKey.ulLength = uStrlen(decrypt_key);
    }
    if (decryptKey.ulLength > 32) {
        decryptKey.ulLength = 32;
    }
    uMemcpy(decryptKey.ucKey, (const void *)decrypt_key, decryptKey.ulLength);
    while (decryptKey.ulLength < 32) {
        decryptKey.ucKey[decryptKey.ulLength++] = 'X';
    }
    fnAES_Init((AES_COMMAND_AES_SET_KEY_DECRYPT | AES_COMMAND_AES_RESET_IV | AES_COMMAND_AES_RE_INIT), decryptKey.ucKey, 256); // register the key
    #if defined PRIME_AES256_INITIAL_VECTOR
    iVectorLength = uStrlen(initial_vector);
    if (iVectorLength > 16) {
        iVectorLength = 16;
    }
    uMemcpy(initVector, initial_vector, iVectorLength);
    while (iVectorLength < 16) {
        initVector[iVectorLength++] = 'X';
    }
    fnAES_Init(AES_COMMAND_AES_PRIME_IV, initVector, 0);                 // prime the initial vector
    #endif
}
#endif

static void fnCopyApplicationToRAM(iMX_BOOT_HEADER *ptrHeader, const unsigned char *ptrCode, unsigned long ulCodeLength)
{
    #if defined CRYPTOGRAPHY
    if (((ptrHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK) == BOOT_LOADER_TYPE_AES256_RAM_EXECUTION) // if AES256 encrypted - for execution in RAM
        #if defined BOOT_LOADER_SUPPORTS_SDRAM
    || ((ptrHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK) == BOOT_LOADER_TYPE_AES256_SDRAM_EXECUTION) // if AES256 encrypted - for execution in SDRAM
        #endif
    ) { 
        unsigned long ulBuffer[4 * 1024];                                // long word aligned buffer
        unsigned long ulCopyLength = sizeof(ulBuffer);
        unsigned char *ptrDestination = (unsigned char *)(RAM_START_ADDRESS_ITC + INTERRUPT_RAM_AREA_SIZE);
        #if defined BOOT_LOADER_SUPPORTS_SDRAM
        if ((ptrHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK) == BOOT_LOADER_TYPE_AES256_SDRAM_EXECUTION) {
            ptrDestination = (unsigned char *)(SDRAM_ADDR);
        }
        #endif
        fnPrepareDecrypt();                                              // prepare the decrypt key and prime the initial vector
        while (ulCodeLength != 0) {
            if (ulCodeLength < sizeof(ulBuffer)) {
                ulCopyLength = ulCodeLength;                             // final block that may be smaller that the buffer size
            }
            fnRetriggerWatchdog();
            uMemcpy(ulBuffer, ptrCode, ulCopyLength);                    // copy the encrypted input block into the decryption buffer
            fnAES_Cipher(AES_COMMAND_AES_DECRYPT, (const unsigned char *)ulBuffer, (unsigned char *)ulBuffer, ulCopyLength); // decrypt the buffer content
        #if !defined _WINDOWS
            uMemcpy(ptrDestination, (const void *)ulBuffer, ulCopyLength); // copy the decrypted code to program RAM
        #endif
            ulCodeLength -= ulCopyLength;
            ptrDestination += ulCopyLength;
            ptrCode += ulCopyLength;
        }
    }
    else {                                                               // un-encrypted application to be executed in RAM
    #endif
    #if !defined _WINDOWS
        #if defined BOOT_LOADER_SUPPORTS_SDRAM
        if ((ptrHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK) == BOOT_LOADER_TYPE_PLAIN_SDRAM_EXECUTION) {
            uMemcpy((void *)(SDRAM_ADDR), ptrCode, ulCodeLength);        // copy the unencrypted code
        }
        else {
        #endif
            uMemcpy((void *)(RAM_START_ADDRESS_ITC + INTERRUPT_RAM_AREA_SIZE), ptrCode, ulCodeLength); // copy the unencrypted code
        #if defined BOOT_LOADER_SUPPORTS_SDRAM
        }
        #endif
    #endif
    #if defined CRYPTOGRAPHY
    }
    #endif
}

static void fnEnableOnTheFlyDecryption(unsigned long app_link_location, unsigned long ulCodeLength)
{
    CHAR *decrypt_key = 0;
    CHAR *initial_vector = 0;
    unsigned long ulKeyLength;
    #if !defined iMX_RT1011
    int i;
    unsigned long *ptrReg;
    #endif
    unsigned long *ptrIn;
    fnGetKeys((const CHAR **)&decrypt_key, (const CHAR **)&initial_vector); // the application will never be started before the secret keys have been encrypted and relocated so this will always return a pointer to the aligned decrypted version in RAM
    ulKeyLength = uStrlen(decrypt_key);
    if (ulKeyLength > 16) {
        ulKeyLength = 16;
    }
    while (ulKeyLength < 16) {
        decrypt_key[ulKeyLength++] = 'X';
    }
    ulKeyLength = uStrlen(initial_vector);
    if (ulKeyLength > 16) {
        ulKeyLength = 16;
    }
    while (ulKeyLength < 16) {
        initial_vector[ulKeyLength++] = 'X';
    }
#if defined iMX_RT1011
    OTFAD_CR = 0;                                                        // decryption initially disabled
    OTFAD_CTX1_RGD_W0 = (app_link_location & OTFAD_RGD_W0_SRTADDR_MASK); // set start address of region (1k aligned)
    ptrIn = (unsigned long *)decrypt_key;                                // use decrypted AES256 key
    OTFAD_CTX1_KEY0 = *ptrIn++;                                          // set encryption key (this is to finally be taken from secret eFUSE value)
    OTFAD_CTX1_KEY1 = *ptrIn++;
    OTFAD_CTX1_KEY2 = *ptrIn++;
    OTFAD_CTX1_KEY3 = *ptrIn;
    ptrIn = (unsigned long *)initial_vector;                             // use decrypted AES256 initial vector
    OTFAD_CTX1_CTR0 = *ptrIn++;                                          // set encryption counters
    OTFAD_CTX1_CTR1 = *ptrIn;
    OTFAD_CTX1_RGD_W1 = (((app_link_location + ulCodeLength + 16 + 1023 + iMX_FILE_OBJECT_SIZE) & OTFAD_RGD_W1_ENDADDR_MASK) | OTFAD_RGD_W1_ADE | OTFAD_RGD_W1_VLD); // set end address of region and validate its operation
    OTFAD_CR = OTFAD_CR_GE;                                              // enable decryption
#else
    POWER_UP_ATOMIC(4, BEE_CLOCK);                                       // power up the bus encryption engine
    IOMUXC_GPR_GPR18 = (app_link_location);                              // region 0 starting at application location
    IOMUXC_GPR_GPR19 = ((app_link_location + ulCodeLength + iMX_FILE_OBJECT_SIZE + 16 + 1023) & ~1023); // region 0 ending at end of application
    IOMUXC_GPR_GPR11 = (IOMUXC_GPR_GPR11_BEE_DE_RX_EN_0);                // enable decryption (don't lock changes)
    BEE_CTRL = (BEE_CTRL_CTRL_CLK_EN | BEE_CTRL_CTRL_SFTRST_N);
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    BEE_CTRL = (BEE_CTRL_SECURITY_LEVEL_R0 | BEE_CTRL_CTRL_AES_MODE_R0 | BEE_CTRL_SECURITY_LEVEL_R1 | BEE_CTRL_CTRL_AES_MODE_R1 | BEE_CTRL_CTRL_CLK_EN | BEE_CTRL_CTRL_SFTRST_N);
    BEE_ADDR_OFFSET0 = 0;
    BEE_ADDR_OFFSET1 = 0;
    BEE_REGION1_TOP = 0;
    BEE_REGION1_BOT = 0;
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    HW_OCOTP_CFG5 &= ~(uFUSE_BOOT_CFG2_BEE_KEY0_SEL_SW_GP2);             // take the AES key selection for BEE_KEY0 from registers and not from SW-GP2
    ptrReg = BEE_AES_KEY0_W0_ADD;
    ptrIn = (unsigned long *)decrypt_key;                                // use decrypted AES256 key
    for (i = 0; i < 4; i++) {
        *ptrReg++ = *ptrIn++;                                            // load the AES128 key to the bus encryption engine (4 aligned long words)
    }
    BEE_CTRL |= BEE_CTRL_BEE_ENABLE;
    BEE_CTRL |= BEE_CTRL_KEY_VALID;
    _WAIT_REGISTER_FALSE(BEE_CTRL, BEE_CTRL_KEY_VALID);
    BEE_CTRL &= ~BEE_CTRL_BEE_ENABLE;
    _WAIT_REGISTER_FALSE(BEE_STATUS, BEE_STATUS_IDLE);                   // wait until BEE is idle
    ptrReg = BEE_CTR_NONCE0_ADD;
    ptrIn = (unsigned long *)initial_vector;                             // use decrypted AES256 initial vector as nonce
    *ptrReg++ = 0;                                                       // zeroed first register
    ptrIn++;
    for (i = 0; i < 3; i++) {
        *ptrReg++ = *ptrIn++;                                            // load the the initial vector to the bus encryption engine (3 aligned long words)
    }
    BEE_CTRL |= BEE_CTRL_BEE_ENABLE;
#endif
#if defined _WINDOWS                                                     // check decryption of the encrypted code
    {
        unsigned char ucCheckBuffer[0x1000];
        unsigned char *ptrCode = (unsigned char *)(app_link_location);
        int i = 0;
        while (i < sizeof(ucCheckBuffer)) {
            ucCheckBuffer[i++] = *fnGetFlashAdd(ptrCode++);
        }
        i = 0;                                                           // set a breakpoint here and view the decrypted content in ucCheckBuffer
    }
#endif
    // QSPI reads from the application area will be decrypted on-the-fly from this point
    //
}

static void _jump_to_application(unsigned long ulResetVectorAddress, unsigned long ulCodeLength, unsigned short usType)
{
#if defined iMX_BOOTLOADER_DEMO_MODE
    unsigned long ulMaxSize = 0;
    switch (usType) {
    case BOOT_LOADER_TYPE_AES128_XiP_RESET_VECTOR:
    case BOOT_LOADER_TYPE_AES128_XiP_CONFIG_TABLE:
    #if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
    case BOOT_LOADER_TYPE_PLAIN_XiP_RESET_VECTOR:
    case BOOT_LOADER_TYPE_PLAIN_XiP_CONFIG_TABLE:
    #endif
        ulMaxSize = (256 * 1024);                                        // limit XiP application to this size
        break;
    #if defined BOOT_LOADER_SUPPORTS_SDRAM
        #if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
    case BOOT_LOADER_TYPE_PLAIN_SDRAM_EXECUTION:
        #endif
    case BOOT_LOADER_TYPE_AES256_SDRAM_EXECUTION:
        ulMaxSize = (256 * 1024);                                        // limit SDRAM application to this size
        break;
    #endif
    #if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
    case BOOT_LOADER_TYPE_PLAIN_RAM_EXECUTION:
    #endif
    case BOOT_LOADER_TYPE_AES256_RAM_EXECUTION:
        ulMaxSize = ((SIZE_OF_FLEX_RAM * 3) / 4);                        // limit internal RAM application to this size
        break;
    }
    if (ulCodeLength > ulMaxSize) {
        return;                                                          // don't allow over-sized code to be started in demo mode
    }
#endif
    if (*BOOT_MAIL_BOX == RESET_TO_APPLICATION_PAUSE) {
        volatile unsigned long ulStartDelay = 30000000;
        while (ulStartDelay-- != 0) {                                    // wait in a delay loop to allow debugger connection before the jump
            fnRetriggerWatchdog();
        }
    }
    jump_to_application(ulResetVectorAddress);
}

static unsigned long fnPrepareStackPointer(unsigned char *ptrTopOfRAM)
{
    unsigned long *ptrResetVector = (unsigned long *)(RAM_START_ADDRESS_ITC + INTERRUPT_RAM_AREA_SIZE);
    unsigned long ulStackPointer;
#if defined _WINDOWS
    ulStackPointer = (RAM_START_ADDRESS_DTC + SIZE_OF_FLEX_RAM - NON_INITIALISED_RAM_SIZE);
#else
    ulStackPointer = *ptrResetVector;                                    // the stack pointer according to the code's reset vector
#endif
    if ((ulStackPointer > RAM_START_ADDRESS_DTC) && (ulStackPointer <= (RAM_START_ADDRESS_DTC + SIZE_OF_FLEX_RAM))) { // don't change the stack pointer used by the application if the reset vector declares it outside of the possible DTC region
        ulStackPointer -= RAM_START_ADDRESS_DTC;
        ulStackPointer %= FLEX_RAM_GRANULARITY;
        ulStackPointer = (FLEX_RAM_GRANULARITY - ulStackPointer);
        if (ulStackPointer > _NON_INITIALISED_RAM_SIZE) {
            ulStackPointer = _NON_INITIALISED_RAM_SIZE;
        }
        ulStackPointer = (unsigned long)(ptrTopOfRAM - ulStackPointer); // set the initial stack pointer value that the loaded application will use to the end of DTC - leaving space for preserved variables
#if !defined _WINDOWS
        *ptrResetVector = ulStackPointer;
#endif
    }
    return ((unsigned long)(RAM_START_ADDRESS_ITC + INTERRUPT_RAM_AREA_SIZE));
}

// Load and start an application stored in QSPI
//
extern void start_application(unsigned long app_link_location)
{
    iMX_BOOT_HEADER *ptrFlashHeader;
    const unsigned char *ptrCode;
    unsigned long ulCodeLength;
    unsigned long _app_link_location = app_link_location;
#if defined (_WINDOWS) || defined (_LITTLE_ENDIAN)                       // the header format is big-endian so we need to convert to little-endian for processor compatibility
    iMX_BOOT_HEADER file_header;
    ptrFlashHeader = (iMX_BOOT_HEADER *)fnGetFlashAdd((unsigned char *)app_link_location);
    ptrCode = (const unsigned char *)(ptrFlashHeader + 1);
    uMemcpy(&file_header, ptrFlashHeader, sizeof(iMX_BOOT_HEADER));      // copy the header locally
    ptrFlashHeader = &file_header;
    fniMXHeaderToLittleEndian(ptrFlashHeader);                           // convert content from big- to little-endian format
    #if  (defined MIMXRT1024 || defined MIMXRT1064) && defined USE_EXTERNAL_QSPI_FLASH
    ptrFlashHeader->ulCodeLength &= 0x0fffffff;                          // temporary workaround - the first bit read from the spi flash is always a '1' (no other such errors occur later and the bit can't be seen when the application later reads)
    #endif
#endif
    ulCodeLength = fnCheckValidApplication(ptrFlashHeader, ptrCode);     // check the validity of the application
    if (ulCodeLength != 0) {                                             // if there is a valid, authenticated application
        unsigned short usType = (ptrFlashHeader->usMagicNumber & BOOT_LOADER_TYPE_MASK);
        switch (usType) {
        case BOOT_LOADER_TYPE_AES128_XiP_RESET_VECTOR:                   // runs encrypted in QSPI flash - no configuration table so starts directly with reset vector
        case BOOT_LOADER_TYPE_AES128_XiP_CONFIG_TABLE:                   // runs encrypted in QSPI flash - image contains redundant configuration table
#if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
        case BOOT_LOADER_TYPE_PLAIN_XiP_RESET_VECTOR:                    // the code is to be executed directly from QSPI flash and begins with the application's reset vector
        case BOOT_LOADER_TYPE_PLAIN_XiP_CONFIG_TABLE:                    // the code is to be executed directly from QSPI flash and begins with a configuration table that needs to be interpreted in order to locate the reset vector
#endif
            app_link_location += sizeof(iMX_BOOT_HEADER);                // move over the header
            app_link_location += *(unsigned char *)fnGetFlashAdd((unsigned char *)app_link_location); // move over the boot configuration (padding)
            if ((usType == BOOT_LOADER_TYPE_AES128_XiP_CONFIG_TABLE) || (usType == BOOT_LOADER_TYPE_AES128_XiP_RESET_VECTOR)) { // if the code runs in encrypted form (on-the-fly)
                fnEnableOnTheFlyDecryption((_app_link_location - iMX_FILE_OBJECT_SIZE), (ulCodeLength + iMX_FILE_OBJECT_SIZE + sizeof(iMX_BOOT_HEADER))); // enable on-the-fly decryption
            }
            if ((usType == BOOT_LOADER_TYPE_AES128_XiP_CONFIG_TABLE) || (usType == BOOT_LOADER_TYPE_PLAIN_XiP_CONFIG_TABLE)) { // if the code contains a configuration table
                app_link_location += BOOT_CONFIG_AREA_SIZE;              // move over the flash configuration to the image vector table
                if (*(unsigned char *)fnGetFlashAdd((unsigned char *)app_link_location) != (unsigned char)IVT_TAG_HEADER) { // check that it is has a valid IVT header
                    break;                                               // invalid
                }
                if (*(unsigned char *)(fnGetFlashAdd((unsigned char *)app_link_location) + 1) != (unsigned char)(IVT_SIZE)) {
                    break;                                               // invalid
                }
                if (*(unsigned char *)(fnGetFlashAdd((unsigned char *)app_link_location) + 2) != (unsigned char)(IVT_SIZE >> 8)) {
                    break;                                               // invalid
                }
                app_link_location = *(unsigned long *)(fnGetFlashAdd((unsigned char *)(app_link_location + 4))); // get the vector table location
            }
            _jump_to_application(app_link_location, ulCodeLength, usType); // jump to the application in QSPI flash - XiP (execute in place - this will not return)
            break;
#if defined BOOT_LOADER_SUPPORTS_SDRAM
    #if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
        case BOOT_LOADER_TYPE_PLAIN_SDRAM_EXECUTION:                     // the plain code is copied to SDRAM and executed from there
    #endif
        case BOOT_LOADER_TYPE_AES256_SDRAM_EXECUTION:                    // the encrypted code is decrypted and copied to SDRAM and executed from there
            fnCopyApplicationToRAM(ptrFlashHeader, ptrCode, ulCodeLength); // copy the application image to the memory where it will be executed in
            _jump_to_application(SDRAM_ADDR, ulCodeLength, usType);      // and finally jump to the new code (this will not return)
            break;
#endif
#if !defined iMX_BOOTLOADER_NO_PLAIN_CODE
        case BOOT_LOADER_TYPE_PLAIN_RAM_EXECUTION:                       // the plain code is copied to instruction RAM (ITC) and executed from there
#endif
        case BOOT_LOADER_TYPE_AES256_RAM_EXECUTION:                      // the encrypted code is decrypted and copied to instruction RAM (ITC) and executed from there
        {
            unsigned char *ptrTopOfRAM = fnConfigureFlexRAM(ulCodeLength + INTERRUPT_RAM_AREA_SIZE); // adjust the FlexRAM dimensioning to best suit the application to be excecuted
            // Beware that RAM banks have usually been reorganised and so global/static variables may have been moved around - all following code should NOT rely on global/static variable content to ensure correct operation!!
            //
            fnCopyApplicationToRAM(ptrFlashHeader, ptrCode, ulCodeLength); // copy the application image to the memory where it will be executed
            _jump_to_application(fnPrepareStackPointer(ptrTopOfRAM), ulCodeLength, usType); // and finally jump to the new code (this will not return)
            break;
        }
        default:                                                         // ignore invalid type
            break;
        }
    }
}
