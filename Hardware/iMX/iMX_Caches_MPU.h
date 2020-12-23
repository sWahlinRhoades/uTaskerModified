/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland
    
    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_Caches_MPU.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************

*/

/* =================================================================== */
/*                          local definitions                          */
/* =================================================================== */

extern void fnCommandCache(unsigned char ucCommands)
{
    register unsigned long ulSets;
    register unsigned long ulWays;
    // Instruction cache
    //
    if ((ucCommands & INSTRUCTION_CACHE_ENABLE) != 0) {
        if ((CONFIGURATION_CONTROL_REGISTER & CONFIGURATION_CONTROL_REGISTER_IC) == 0) { // if the instruction cache is presently disabled
            _SYNCHRONISATION_BARRIER();
            ICIALLU = 0;                                                 // invalidate instruction cache
            _SYNCHRONISATION_BARRIER();
            CONFIGURATION_CONTROL_REGISTER |= CONFIGURATION_CONTROL_REGISTER_IC; // enable instruction cache
            _SYNCHRONISATION_BARRIER();
        }
    }
    else if ((ucCommands & INSTRUCTION_CACHE_DISABLE) != 0) {
        _SYNCHRONISATION_BARRIER();
        CONFIGURATION_CONTROL_REGISTER &= ~(CONFIGURATION_CONTROL_REGISTER_IC); // disable instruction cache
        ICIALLU = 0;                                                     // invalidate instruction cache
        _SYNCHRONISATION_BARRIER();
    }
    if ((ucCommands & INSTRUCTION_CACHE_INVALIDATE) != 0) {
        _SYNCHRONISATION_BARRIER();
        ICIALLU = 0;                                                     // invalidate instruction cache
        _SYNCHRONISATION_BARRIER();
    }
    // Data cache
    //
    if ((ucCommands & (DATA_CACHE_ENABLE | DATA_CACHE_DISABLE | DATA_CACHE_INVALIDATE)) != 0) {
        CSSELR = (CSSELR_LEVEL1_CACHE | CSSELR_DATA_CACHE);              // select L1 data cache ID
        _SYNCHRONISATION_DATA_BARRIER();

        if ((ucCommands & DATA_CACHE_ENABLE) != 0) {
            if ((CONFIGURATION_CONTROL_REGISTER & CONFIGURATION_CONTROL_REGISTER_DC) != 0) { // if the data cache is already enabled
                return;
            }
            else {
                ucCommands = (DATA_CACHE_INVALIDATE | DATA_CACHE_ENABLE);
            }
        }
        else if ((ucCommands & DATA_CACHE_DISABLE) != 0) {
            CONFIGURATION_CONTROL_REGISTER &= ~(CONFIGURATION_CONTROL_REGISTER_DC); // disable data cache
            _SYNCHRONISATION_DATA_BARRIER();
        }

        if ((ucCommands & DATA_CACHE_INVALIDATE) != 0) {
            // Invalidate data cache
            //
            ulSets = ((CSSIDR & CSSIDR_SETS_MASK) >> CSSIDR_SETS_SHIFT);
            do {
                ulWays = ((CSSIDR & CSSIDR_WAYS_MASK) >> CSSIDR_WAYS_SHIFT);
                do {
                    DCISW = (((ulSets << DCISW_SET_SHIFT) | (ulWays << DCISW_WAY_SHIFT)) & (DCISW_SET_MASK | DCISW_SET_MASK)); // invalidate each data cache by set/way
                } while (ulWays-- != 0);
            } while (ulSets-- != 0);
            _SYNCHRONISATION_DATA_BARRIER();
        }

        if ((ucCommands & DATA_CACHE_ENABLE) != 0) {
            CONFIGURATION_CONTROL_REGISTER |= CONFIGURATION_CONTROL_REGISTER_DC; // enable data cache
            _SYNCHRONISATION_BARRIER();
        }
    }
}

// Beware that this routine may not access global variables since they haven't been initialised at this point
//
static void fnInitialise_iMX_caches_mpu(void)
{
    // Configure instruction cache (and initially disable/invalidate the data cache)
    //
    #if defined ENABLE_INSTRUCTION_CACHE
    fnCommandCache(INSTRUCTION_CACHE_ENABLE | DATA_CACHE_DISABLE | DATA_CACHE_INVALIDATE);
    #else
    fnCommandCache(INSTRUCTION_CACHE_DISABLE | DATA_CACHE_DISABLE | DATA_CACHE_INVALIDATE);
    #endif
#if defined MIMXRT1010 || defined MIMXRT1015 || defined MIMXRT1020 || defined MIMXRT1024 || defined MIMXRT1050 || defined MIMXRT1060 || defined MIMXRT1064 || defined iMX_RT1062_EMB_ART || defined MIMXRT1170 // initial development with these boards/processors
    // Configure the MPU
    //
    _DATA_MEMORY_BARRIER();
    #if defined SHCSR
    SHCSR = 0;                                                           // disable faults
    #endif
    MPU_CTRL = 0;                                                        // disable MPU

    // Region 0 setting
    //
    MPU_RBAR = (0x80000000 | MPU_RBAR_REGION_0 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NOT_SHAREABLE_NOT_CACHEABLE_DEVICE | MPU_RASR_SIZE_512M | MPU_RASR_ENABLE);

    // Region 1 setting
    //
    #if defined MIMXRT1024 || defined MIMXRT1064
    MPU_RBAR = (FLEXSPI1_FLASH_BASE | MPU_RBAR_REGION_1 | MPU_RBAR_VALID); // internal QSPI flash
    #else
    MPU_RBAR = (FLEXSPI_FLASH_BASE | MPU_RBAR_REGION_1 | MPU_RBAR_VALID);// external QSPI flash
    #endif
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NOT_SHAREABLE_NOT_CACHEABLE_DEVICE | MPU_RASR_SIZE_512M | MPU_RASR_ENABLE);

    // Region 2 setting (XIP flash)
    //
    MPU_RBAR = (FLEXSPI_FLASH_BASE | MPU_RBAR_REGION_2 | MPU_RBAR_VALID);// external QSPI flash
    #if defined MIMXRT1015 || defined MIMXRT1010
    MPU_RASR = (MPU_RASR_AP_READ_ONLY | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_16M | MPU_RASR_ENABLE);
    #elif defined MIMXRT1020 || defined MIMXRT1060 || defined iMX_RT1062_EMB_ART || defined MIMXRT1170
    MPU_RASR = (MPU_RASR_AP_READ_ONLY | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_8M | MPU_RASR_ENABLE);
    #elif defined MIMXRT1024 || defined MIMXRT1064
    MPU_RASR = (MPU_RASR_AP_READ_ONLY | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_4M | MPU_RASR_ENABLE);
    #else
    MPU_RASR = (MPU_RASR_AP_READ_ONLY | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_64M | MPU_RASR_ENABLE);
    #endif

    // Region 3 setting
    //
    MPU_RBAR = (RAM_START_ADDRESS_ITC | MPU_RBAR_REGION_3 | MPU_RBAR_VALID); // tightly coupled instruction RAM
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NOT_SHAREABLE_NOT_CACHEABLE_DEVICE | MPU_RASR_SIZE_1G | MPU_RASR_ENABLE);

    // Region 4 setting
    //
    MPU_RBAR = (RAM_START_ADDRESS_ITC | MPU_RBAR_REGION_4 | MPU_RBAR_VALID); // tightly coupled instruction RAM
    #if SIZE_OF_RAM_ITC == (32 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_32k | MPU_RASR_ENABLE);
    #elif SIZE_OF_RAM_ITC == (128 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_128k | MPU_RASR_ENABLE);
    #else
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_64k | MPU_RASR_ENABLE);
    #endif

    // Region 5 setting
    //
    MPU_RBAR = (RAM_START_ADDRESS_DTC | MPU_RBAR_REGION_5 | MPU_RBAR_VALID); // tightly coupled data RAM
    #if SIZE_OF_RAM_DTC == (32 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_32k | MPU_RASR_ENABLE);
    #elif SIZE_OF_RAM_DTC == (128 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_128k | MPU_RASR_ENABLE);
    #else
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_64k | MPU_RASR_ENABLE);
    #endif

    // Region 6 setting
    //
    #if defined RAM_START_ADDRESS_OCR2
    MPU_RBAR = (RAM_START_ADDRESS_OCR2 | MPU_RBAR_REGION_6 | MPU_RBAR_VALID); // non FlexRAM OCR
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_512k | MPU_RASR_ENABLE);
    #else
    MPU_RBAR = (RAM_START_ADDRESS_OCR | MPU_RBAR_REGION_6 | MPU_RBAR_VALID); // FlexRAM OCR
        #if SIZE_OF_RAM_OCR == (64 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_64k | MPU_RASR_ENABLE);
        #elif SIZE_OF_RAM_OCR == (256 * 1024)
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_256k | MPU_RASR_ENABLE);
        #else
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_128k | MPU_RASR_ENABLE);
        #endif
    #endif

    #if defined MIMXRT1015
    // Region 7 setting
    //
    MPU_RBAR = (RAM_START_ADDRESS_OCR | MPU_RBAR_REGION_7 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_NON_CACHEABLE_MEMORY | MPU_RASR_SIZE_64k | MPU_RASR_ENABLE);
    #elif defined RAM_START_ADDRESS_OCR2
    // Region 7 setting
    //
    MPU_RBAR = (RAM_START_ADDRESS_OCR | MPU_RBAR_REGION_7 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_256k | MPU_RASR_ENABLE);

    // Region 8 setting
    //
    MPU_RBAR = (SDRAM_ADDR | MPU_RBAR_REGION_8 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_32M | MPU_RASR_ENABLE);

    // Region 9 setting (non-cacheable)
    //
    MPU_RBAR = ((SDRAM_ADDR + 0x01e00000) | MPU_RBAR_REGION_9 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_NON_CACHEABLE_MEMORY | MPU_RASR_SIZE_2M | MPU_RASR_ENABLE);
    #elif defined SEMC_AVAILABLE
    // Region 7 setting
    //
    MPU_RBAR = (SDRAM_ADDR | MPU_RBAR_REGION_7 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_OUTER_INNER_WRITEBACK | MPU_RASR_SIZE_32M | MPU_RASR_ENABLE);

    // Region 8 setting (non-cacheable)
    //
    MPU_RBAR = ((SDRAM_ADDR + 0x01e00000) | MPU_RBAR_REGION_8 | MPU_RBAR_VALID);
    MPU_RASR = (MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL_NOT_SHAREABLE_NON_CACHEABLE_MEMORY | MPU_RASR_SIZE_2M | MPU_RASR_ENABLE);
    #endif

    MPU_CTRL = (MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE);                  // enable the MPU with privileged software access to the default memory map
    #if defined SHCSR
    SHCSR = SHCSR_MEMFAULTENA;                                           // enable memory faults
    #endif
    _SYNCHRONISATION_BARRIER();
#else
    _EXCEPTION("To do for this board");
#endif

    // Enabled data cache
    //
#if defined ENABLE_DATA_CACHE
    fnCommandCache(DATA_CACHE_ENABLE);
#endif
}
