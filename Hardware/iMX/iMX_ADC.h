/***********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher
    
    ---------------------------------------------------------------------
    File:      iMX_ADC.h
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************


*/

#if defined _ADC_INTERRUPT_CODE


/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

static __interrupt void _ADC_Interrupt_0(void);
    #if ADC_CONTROLLERS > 1
static __interrupt void _ADC_Interrupt_1(void);
    #endif
    #if ADC_CONTROLLERS > 2
static __interrupt void _ADC_Interrupt_2(void);
    #endif
    #if ADC_CONTROLLERS > 3
static __interrupt void _ADC_Interrupt_3(void);
    #endif

/* =================================================================== */
/*                             constants                               */
/* =================================================================== */

static const unsigned char *_ADC_Interrupt[ADC_CONTROLLERS] = {
    (unsigned char *)_ADC_Interrupt_0,
    #if ADC_CONTROLLERS > 1
    (unsigned char *)_ADC_Interrupt_1,
    #endif
    #if ADC_CONTROLLERS > 2
    (unsigned char *)_ADC_Interrupt_2,
    #endif
    #if ADC_CONTROLLERS > 3
    (unsigned char *)_ADC_Interrupt_3
    #endif
};


/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

static void (*_ADC_Interrupt_handler[ADC_CONTROLLERS])(ADC_INTERRUPT_RESULT *) = {0}; // user ADC interrupt handlers


/* =================================================================== */
/*                        ADC Interrupt Handlers                       */
/* =================================================================== */

// ADC 0 interrupt
//
static __interrupt void _ADC_Interrupt_0(void)
{
  //ADC0_SC1A = ADC_SC1A_ADCH_OFF;                                       // disable further operation and interrupt
    if (_ADC_Interrupt_handler[0] != 0) {
        ADC_INTERRUPT_RESULT adc_result;
  //    adc_result.sADC_value = (signed short)ADC0_RA;
        adc_result.ucADC_channel = 0;                                    // channel is not added
  //    adc_result.ucADC_flags = ADC_RESULT_VALID;                       // assumed that the result is valid
        uDisable_Interrupt();
            _ADC_Interrupt_handler[0](&adc_result);                      // call handling function to inform that the result is ready
        uEnable_Interrupt();
    }
}

    #if ADC_CONTROLLERS > 1
// ADC 1 interrupt
//
static __interrupt void _ADC_Interrupt_1(void)
{
  //ADC1_SC1A = ADC_SC1A_ADCH_OFF;                                       // disable further operation and interrupt
    if (_ADC_Interrupt_handler[1] != 0) {
        ADC_INTERRUPT_RESULT adc_result;
  //    adc_result.sADC_value = (signed short)ADC1_RA;
        adc_result.ucADC_channel = 0;
  //    adc_result.ucADC_flags = ADC_RESULT_VALID;
        uDisable_Interrupt();
            _ADC_Interrupt_handler[1](&adc_result);                      // call handling function to inform that the result is ready
        uEnable_Interrupt();
    }
}
    #endif



// Convert a standard 16 bit value to the present ADC mode format
//
static unsigned long fnConvertADCvalue(iMX_ADC_REGS *ptrADC, unsigned short usStandardValue, int iPlusOne)
{
    switch (ptrADC->ADC_CFG & ADC_CFG_MODE_MASK) {
    case ADC_CFG_MODE_12:                                                // conversion mode - single-ended 12 bit
        usStandardValue >>= 4;
        if (iPlusOne != 0) {                                             // increase by 1 so that a value is one LSB above the match threshold
            if (usStandardValue != 0x0fff) {
                usStandardValue++;
            }
        }
        break;
    case ADC_CFG_MODE_10:                                                // conversion mode - single-ended 10 bit
        usStandardValue >>= 6;
        if (iPlusOne != 0) {                                             // increase by 1 so that a value is one LSB above the match threshold
            if (usStandardValue != 0x03ff) {
                usStandardValue++;
            }
        }
        break;
    case ADC_CFG_MODE_8:                                                 // conversion mode - single-ended 8 bit
        usStandardValue >>= 8;
        if (iPlusOne != 0) {                                             // increase by 1 so that a value is one LSB above the match threshold
            if (usStandardValue != 0x00ff) {
                usStandardValue++;
            }
        }
        break;
    }
    return usStandardValue;
}
#endif


/* =================================================================== */
/*                         ADC Configuration                           */
/* =================================================================== */

#if defined _ADC_CONFIG_CODE
        {
            ADC_SETUP *ptrADC_settings = (ADC_SETUP *)ptrSettings; 
            iMX_ADC_REGS *ptrADC;
            int irq_ADC_ID;
            unsigned char ucADC_channel = ptrADC_settings->int_adc_bit;  // the channel to be configured
            if ((ADC_DISABLE_ADC & ptrADC_settings->int_adc_mode) != 0) {
                if (ptrADC_settings->int_adc_controller == 0) {
                    POWER_DOWN_ATOMIC(1, ADC1_CLOCK);                    // disable clocks to module
                }
    #if ADC_CONTROLLERS > 1
                else if (ptrADC_settings->int_adc_controller == 1) {
                    POWER_DOWN_ATOMIC(1, ADC2_CLOCK);                    // disable clocks to module
                }
    #endif
    #if defined _WINDOWS
                else {
                    _EXCEPTION("ADC controller doesn't exist!!");
                }
    #endif
                return;
            }
            if (ptrADC_settings->int_adc_controller == 0) {
                irq_ADC_ID = irq_ADC1_ID;
                ptrADC = (iMX_ADC_REGS *)ADC1_BLOCK;
            }
    #if ADC_CONTROLLERS > 1
            else if (ptrADC_settings->int_adc_controller == 1) {
                irq_ADC_ID = irq_ADC2_ID;
                ptrADC = (iMX_ADC_REGS *)ADC2_BLOCK;
            }
    #endif
            else {
    #if defined _WINDOWS
                _EXCEPTION("ADC controller doesn't exist!!");
    #endif
                return;
            }
            if ((ptrADC_settings->int_adc_mode & ADC_READ_ONLY) == 0) {  // if not being called only for read
                unsigned char ucChannelConfig = 0;
                unsigned short usDMA_source;
                if ((ptrADC_settings->int_adc_mode & ADC_CONFIGURE_ADC) != 0) { // main configuration is to be performed
                    if (ptrADC_settings->int_adc_controller == 0) {      // ADC1
                        POWER_UP_ATOMIC(1, ADC1_CLOCK);                  // enable clocks to module
                        usDMA_source = DMAMUX0_CHCFG_SOURCE_ADC1;
                    }
    #if ADC_CONTROLLERS > 1
                    else if (ptrADC_settings->int_adc_controller == 1) {
                        POWER_UP_ATOMIC(1, ADC2_CLOCK);                  // enable clocks to module
                        usDMA_source = DMAMUX0_CHCFG_SOURCE_ADC2;
                    }
    #endif
                    ptrADC->ADC_CFG = ((ADC_CFG_ADICLK_ASY | ADC_CFG_MODE_MASK | ADC_CFG_ADIV_8 | ADC_CFG_ADLPC | ADC_CFG_ADSTS_24 | ADC_CFG_ADHSC | ADC_CFG_AVGS_32 | ADC_CFG_OVWREN) & ptrADC_settings->int_adc_mode);
    #if defined _WINDOWS                                                 // simuation check of ADC clock speed range
                    // Check that the ADC frequency is set to a valid range
                    //
                    {
                        unsigned long ulADC_clockMax = 0;
                        unsigned long ulADC_clockMin = 0;
                        switch (ptrADC->ADC_CFG & ADC_CFG_ADICLK_ASY) {
                        case ADC_CFG_ADICLK_IPG:                         // ipg clock root
                            ulADC_clockMax = ulADC_clockMin = IPG_CLK_ROOT_FREQUENCY;
                            break;
                        case ADC_CFG_ADICLK_IPG2:                        // ipg clock root divided by 2
                            ulADC_clockMax = ulADC_clockMin = (IPG_CLK_ROOT_FREQUENCY /2);
                            break;
                        case ADC_CFG_ADICLK_ASY:
                            _EXCEPTION("To do");
                            /*
                            if ((ptrADC->ADC_CFG1 & ADC_CFG1_ADLPC) != 0) { // low power configuration is active (reduced power at the expense of the clock speed)
                                if ((ptrADC->ADC_CFG1 & ADC_CFG1_ADLPC) != 0) { // high speed configuration
                                    ulADC_clockMax = 6100000;            // 4.0MHz typical ADC clock speed (range 2.4MHz..6.1MHz)
                                    ulADC_clockMin = 2400000;
                                }
                                else {                                   // normal conversion sequence
                                    ulADC_clockMax = 3900000;            // 2.4MHz typical ADC clock speed (range 1.2MHz..3.9MHz)
                                    ulADC_clockMin = 1200000;
                                }
                            }
                            else {                                       // low power configuration is not active
                                if ((ptrADC->ADC_CFG1 & ADC_CFG1_ADLPC) != 0) { // high speed configuration
                                    ulADC_clockMax = 9500000;            // 6.2MHz typical ADC clock speed (range 4.4MHz..9.5MHz)
                                    ulADC_clockMin = 4400000;
                                }
                                else {                                   // normal conversion sequence
                                    ulADC_clockMax = 7300000;            // 5.2MHz typical ADC clock speed (range 3.0MHz..7.3MHz)
                                    ulADC_clockMin = 3000000;
                                }
                            }*/
                            break;
                        default:
                            _EXCEPTION("Invalid ADC clock");
                            break;
                        }
                        ulADC_clockMax >>= ((ADC_CFG_ADIV_8 & ptrADC->ADC_CFG) >> 5);
                        ulADC_clockMin >>= ((ADC_CFG_ADIV_8 & ptrADC->ADC_CFG) >> 5);
                        switch (ptrADC->ADC_CFG & ADC_CFG_MODE_MASK) {
                        case ADC_CFG_MODE_8:
                        case ADC_CFG_MODE_10:
                        case ADC_CFG_MODE_12:
                            if ((ulADC_clockMin < 1000000) || (ulADC_clockMax > 18000000)) { // check valid ADC clock rate
                                _EXCEPTION("ADC clock rate outside valid range 1MHz..18MHz for modes <= 13 bits");
                            }
                            break;
                        default:
                            _EXCEPTION("Invalid ADC conversion mode");
                            break;
                        }
                    }
    #endif
                    ptrADC->ADC_GC = ((ptrADC_settings->int_adc_mode >> 24) & (ADC_GC_AVGE | ADC_GC_CAL));
                    if ((ptrADC->ADC_GC & ADC_GC_CAL) != 0) {            // if calibration set (this should be perfomred once after every reset)
                        _WAIT_REGISTER_TRUE(ptrADC->ADC_GC, ADC_GC_CAL); // wait for calibration to complete (this can take up to 14000 ADCLK cycles and 100 bus cycles)
                        (void)ptrADC->ADC_R0;                            // read the conversion value in order to clear the conversion done flag
                    }
                    ptrADC->ADC_CFG |= (ADC_CFG_ADTRG_HW & ptrADC_settings->int_adc_mode); // enable hardware trigger mode after calibration, if set
                    if ((ptrADC_settings->int_adc_mode & ADC_LOOP_MODE) != 0) { // if continuous conversion is required
                        ptrADC->ADC_GC |= ADC_GC_ADCO;                   // enable continuous conversion, if set
                    }
                }
                if ((ptrADC_settings->int_adc_mode & ADC_CONFIGURE_CHANNEL) != 0) { // if a channel is to be configured
                    ptrADC->ADC_HC0 = ucADC_channel;                     // enter the channel to be used
                    if ((ADC_DISABLE_ADC & ptrADC_settings->int_adc_mode) != 0) {
                        if (ptrADC_settings->int_adc_controller == 0) {
                            switch (ucADC_channel) {                             // ensure that ADC inputs are configured with keeper disabled
                            case 0:
                                _CONFIG_PORT_INPUT(1, ADC1_IN0_GPIO_AD_B1_11_GPIO1_IO27, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 1:
                                _CONFIG_PORT_INPUT(1, ADC1_IN1_GPIO_AD_B0_12_GPIO1_IO12, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 2:
                                _CONFIG_PORT_INPUT(1, ADC1_IN2_GPIO_AD_B0_13_GPIO1_IO13, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 3:
                                _CONFIG_PORT_INPUT(1, ADC1_IN3_GPIO_AD_B0_14_GPIO1_IO14, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 4:
                                _CONFIG_PORT_INPUT(1, ADC1_IN4_GPIO_AD_B0_15_GPIO1_IO15, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 5:
                                _CONFIG_PORT_INPUT(1, ADC1_IN5_GPIO_AD_B1_00_GPIO1_IO16, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 6:
                                _CONFIG_PORT_INPUT(1, ADC1_IN6_GPIO_AD_B1_01_GPIO1_IO17, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 7:
                                _CONFIG_PORT_INPUT(1, ADC1_IN7_GPIO_AD_B1_02_GPIO1_IO18, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 8:
                                _CONFIG_PORT_INPUT(1, ADC1_IN8_GPIO_AD_B1_03_GPIO1_IO19, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 9:
                                _CONFIG_PORT_INPUT(1, ADC1_IN9_GPIO_AD_B1_04_GPIO1_IO20, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 10:
                                _CONFIG_PORT_INPUT(1, ADC1_IN10_GPIO_AD_B1_05_GPIO1_IO21, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 11:
                                _CONFIG_PORT_INPUT(1, ADC1_IN11_GPIO_AD_B1_06_GPIO1_IO22, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 12:
                                _CONFIG_PORT_INPUT(1, ADC1_IN12_GPIO_AD_B1_07_GPIO1_IO23, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 13:
                                _CONFIG_PORT_INPUT(1, ADC1_IN13_GPIO_AD_B1_08_GPIO1_IO24, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 14:
                                _CONFIG_PORT_INPUT(1, ADC1_IN14_GPIO_AD_B1_09_GPIO1_IO25, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 15:
                                _CONFIG_PORT_INPUT(1, ADC1_IN15_GPIO_AD_B1_10_GPIO1_IO26, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            }
                        }
    #if ADC_CONTROLLERS > 1
                        else if (ptrADC_settings->int_adc_controller == 1) {
                            switch (ucADC_channel) {                             // ensure that ADC inputs are configured with keeper disabled
                            case 0:
                                _CONFIG_PORT_INPUT(1, ADC2_IN0_GPIO_AD_B1_11_GPIO1_IO27, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 1:
                                _CONFIG_PORT_INPUT(1, ADC2_IN1_GPIO_AD_B1_12_GPIO1_IO28, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 2:
                                _CONFIG_PORT_INPUT(1, ADC2_IN2_GPIO_AD_B1_13_GPIO1_IO29, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 3:
                                _CONFIG_PORT_INPUT(1, ADC2_IN3_GPIO_AD_B1_14_GPIO1_IO30, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 4:
                                _CONFIG_PORT_INPUT(1, ADC2_IN4_GPIO_AD_B1_15_GPIO1_IO31, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 5:
                                _CONFIG_PORT_INPUT(1, ADC2_IN5_GPIO_AD_B1_00_GPIO1_IO16, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 6:
                                _CONFIG_PORT_INPUT(1, ADC2_IN6_GPIO_AD_B1_01_GPIO1_IO17, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 7:
                                _CONFIG_PORT_INPUT(1, ADC2_IN7_GPIO_AD_B1_02_GPIO1_IO18, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 8:
                                _CONFIG_PORT_INPUT(1, ADC2_IN8_GPIO_AD_B1_03_GPIO1_IO19, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 9:
                                _CONFIG_PORT_INPUT(1, ADC2_IN9_GPIO_AD_B1_04_GPIO1_IO20, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 10:
                                _CONFIG_PORT_INPUT(1, ADC2_IN10_GPIO_AD_B1_05_GPIO1_IO21, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 11:
                                _CONFIG_PORT_INPUT(1, ADC2_IN11_GPIO_AD_B1_06_GPIO1_IO22, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 12:
                                _CONFIG_PORT_INPUT(1, ADC2_IN12_GPIO_AD_B1_07_GPIO1_IO23, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 13:
                                _CONFIG_PORT_INPUT(1, ADC2_IN13_GPIO_AD_B1_08_GPIO1_IO24, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 14:
                                _CONFIG_PORT_INPUT(1, ADC2_IN14_GPIO_AD_B1_09_GPIO1_IO25, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            case 15:
                                _CONFIG_PORT_INPUT(1, ADC2_IN15_GPIO_AD_B1_10_GPIO1_IO26, IOMUXC_SW_PAD_CTL_PAD_SPEED_MEDIUM);
                                break;
                            }
                        }
    #endif
                    }
                    if ((ptrADC_settings->int_adc_mode & (ADC_FULL_BUFFER_DMA | ADC_HALF_BUFFER_DMA)) != 0) { // if DMA operation is being specified
                        unsigned long *ptrADC_result = (unsigned long *)((unsigned long)ptrADC + 0x010); // ADC channel as result register
                        unsigned long ulDMA_rules = (DMA_DIRECTION_INPUT | DMA_HALF_WORDS);
                        unsigned short usTriggerSource = ptrADC_settings->usDmaTriggerSource;
                        ptrADC->ADC_GC |= ADC_GC_DMAEN;                  // enable DMA trigger on ADC conversion end
                        if ((ptrADC_settings->int_adc_mode & ADC_FULL_BUFFER_DMA_AUTO_REPEAT) != 0) {
                            ulDMA_rules |= DMA_AUTOREPEAT;
                        }
                        if ((ptrADC_settings->int_adc_mode & ADC_HALF_BUFFER_DMA) != 0) {
                            ulDMA_rules |= DMA_HALF_BUFFER_INTERRUPT;
                        }
                        if (usTriggerSource == 0) {                      // if the default is defined
                            usTriggerSource = usDMA_source;
                        }
                        fnConfigDMA_buffer(ptrADC_settings->ucDmaChannel, usTriggerSource, ptrADC_settings->ulADC_buffer_length, ptrADC_result, ptrADC_settings->ptrADC_Buffer, ulDMA_rules, ptrADC_settings->dma_int_handler, ptrADC_settings->dma_int_priority); // source is the ADC result register and destination is the ADC buffer
                        fnDMA_BufferReset(ptrADC_settings->ucDmaChannel, DMA_BUFFER_START);
                    }
                    else if ((ptrADC_settings->int_adc_mode & ADC_LOOP_MODE) == 0) { // single shot mode
                        if (ptrADC_settings->int_handler != 0) {         // if single-shot conversion handler entered
                            _ADC_Interrupt_handler[ptrADC_settings->int_adc_controller] = ptrADC_settings->int_handler; // enter the interrupt handler function
                            fnEnterInterrupt(irq_ADC_ID, ptrADC_settings->int_priority, (void (*)(void))_ADC_Interrupt[ptrADC_settings->int_adc_controller]);
                            ptrADC->ADC_HC0 |= ADC_HC_AIEN;              // enable interrupt on end of conversion
                            if ((ptrADC_settings->int_adc_int_type & (ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT)) != 0) { // if a level is defined
                                ptrADC->ADC_GC |= ADC_GC_ADCO;           // enable continuous conversion
                                switch (ptrADC_settings->int_adc_int_type & (ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT)) {
                                case ADC_LOW_LIMIT_INT:
                                    ptrADC->ADC_CV = fnConvertADCvalue(ptrADC, ptrADC_settings->int_low_level_trigger, 0); // set low level threshold according to mode
                                    ptrADC->ADC_GC |= (ADC_GC_ACFE);     // enable compare function for lower than threshold
                                    break;
                                case ADC_HIGH_LIMIT_INT:
                                    ptrADC->ADC_CV = fnConvertADCvalue(ptrADC, ptrADC_settings->int_high_level_trigger, 1); // set high level threshold according to mode
                                    ptrADC->ADC_GC |= (ADC_GC_ACFE | ADC_GC_ACFGT); // enable compare function for greater or equal to threshold
                                    break;
                                case (ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT): // if the low value is less that the high value it results in a trigger when the value is higher or lower than the thresholds - if the low value is higher than th high value it rsults in a trigger if the value is between the thresholds
                                    ptrADC->ADC_CV = fnConvertADCvalue(ptrADC, ptrADC_settings->int_low_level_trigger, 0); // set low range threshold
                                    ptrADC->ADC_CV |= (fnConvertADCvalue(ptrADC, ptrADC_settings->int_high_level_trigger, 1) << 16); // set high range threshold
                                    ptrADC->ADC_GC |= (ADC_GC_ACFE | ADC_GC_ACREN); // enable compare function on range
                                    break;
                                }
                            }
                        }
                    }
                  // ??ptrADC->ADC_SC1A = ucChannelConfig;                  // start conversion if software mode or enable triggered conversion
                }
            }
            if (((ptrADC_settings->int_adc_mode & (ADC_GET_RESULT | ADC_CHECK_CONVERSION)) != 0) && (ptrADC_settings->int_adc_result != 0)) { // if there is a result structure
    #if defined _WINDOWS
                switch (ptrADC_settings->int_adc_controller) {
                case 0:
                    if (IS_POWERED_UP(1, ADC1_CLOCK) == 0) {
                        _EXCEPTION("Trying to read from ADC1 that is not powered up!");
                    }
                    break;
        #if ADC_CONTROLLERS > 1
                case 1:
                    if (IS_POWERED_UP(1, ADC2_CLOCK) == 0) {
                        _EXCEPTION("Trying to read from ADC2 that is not powered up!");
                    }
                    break;
        #endif
                }
    #endif
                if (((ptrADC_settings->int_adc_mode & ADC_CHECK_CONVERSION) != 0) || ((ptrADC->ADC_HC0 & ADC_HC_AIEN) == 0)) { // check the conversion state or a read interrupt, or read without interrupt operation
                    while ((ptrADC->ADC_HS & ADC_HS_COCO) == 0) {        // wait for conversion to complete
    #if defined _WINDOWS
                        if ((ptrADC->ADC_HC0 & ADC_HC_ADCH_MASK) != ADC_HC_ADCH_OFF) { // if the converter hasn't been disabled
                            ptrADC->ADC_HS |= ADC_HS_COCO;               // set conversion complete flag
                        }
    #endif
                        if ((ptrADC_settings->int_adc_mode & ADC_CHECK_CONVERSION) != 0) { // if we are not to wait
                            ptrADC_settings->int_adc_result->ucADC_status[0] = ADC_RESULT_NOT_READY; // result is presently not ready
                            return;
                        }
                        if ((ptrADC->ADC_HC0 & ADC_HC_ADCH_MASK) == ADC_HC_ADCH_OFF) { // if the converter sub-system has been disabled we return the read value without waiting for conversion completion (this happens after single-shot interrupts disable the converter but the last result is in the result register)
                            break;
                        }
                    }
                }
                ptrADC_settings->int_adc_result->ucADC_status[0] = ADC_RESULT_VALID; // mark that the result is valid
                ptrADC_settings->int_adc_result->sADC_value[0] = (signed short)ptrADC->ADC_R0; // return the read value
            }
        }
#endif
