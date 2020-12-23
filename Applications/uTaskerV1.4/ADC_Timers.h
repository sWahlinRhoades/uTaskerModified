/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, R�tihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      ADC_Timers.h
    Project:   uTasker project
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    29.08.2009 Add timer frequency and PWM output tests                  {1}
    08.09.2009 Add INTERNAL_TEMP option for Luminary                     {2}
    07.11.2009 Add LPC2XXX PWM test and adjust PWM value to utilise macro{3}
    04.12.2009 Corrected syntax error                                    {4}
    14.01.2010 Add SAM7X PWM test                                        {5}
    03.03.2010 Add SAM7X capture mode test                               {6}
    13.04.2010 Add LPC17XX RIT test                                      {7}
    27.12.2010 Add SUPPORT_PWM_CONTROLLER mode for Luminary devices      {8}
    01.02.2012 Add PWM support for Kinetis and M522xx                    {9}
    24.02.2012 Modify PIT periodic test for Coldfire and Kinetis         {10}
    12.03.2012 Add Kinetis ADC                                           {11}
    30.09.2013 Add ADC_SELECT_INPUTS_A and ADC_SELECT_INPUTS_B input multiplexer setting for kinetis {12}
    09.10.2013 Added PGA gain setting for kinetis                        {13}
    24.12.2013 Add ADC/DAC delay line configuration [Kinetis]            {14}
    28.03.2014 Add voice recorder                                        {15}
    05.04.2014 Add Kinetis FlexTimer/TPM                                 {16}
    07.06.2014 Free directory object after each recording session        {17}
    15.06.2014 Add Kinetis LPTMR                                         {18}
    17.06.2014 Add fnSetColor() to allow PWM control of multicoloured LEDs based on inputs (such as accelerometer) {19}
    23.12.2015 Add TEST_DMA_DAC                                          {20}
    04.01.2016 Add KL support for ping-pong buffer operation             {21}
    05.01.2016 Clear PWM interrupt entry                                 {22}
    08.01.2017 Add TIMED_UART_TX_TEST                                    {23}
    22.04.2017 Add ADC_TRIGGER_TPM and TEST_STEPPER options
    20.05.2017 Add Kinetis timer capture test                            {24}
    09.02.2018 Add ADC polling reference (rather than using end of conversion interrupt) {25}
    03.05.2018 Corrected Kinetis internal temperature equation           {26}
    02.06.2018 Zero optional user UART callback handlers                 {27}
    05.09.2018 Add FFT_SAMPLED_INPUT option                              {28}
    05.10.2018 Add PHASE_SHIFTED_COMBINED_OUTPUTS option for PWM         {29}
    08.10.2018 Add MULTIPLE_CHANNEL_INTERRUPTS option for PWM            {30}
    13.10.2018 Add comparator reference                                  {31}
    29.05.2019 Add DMA driven PWM sine wave generation                   {32}
    03.07.2019 Add STM32 ADC reference (single input and scan mode)      {33}

    The file is otherwise not specifically linked in to the project since it is included by application.c when needed.
    The reason for ADC and timer configurations in a single file is that a HW timer is very often used togther with an ADC.

    For details of hardware timers (and PWM) see the document http://www.utasker.com/docs/uTasker/uTaskerHWTimers.PDF
    For details of ADC/DAC see the document https://www.utasker.com/docs/uTasker/uTaskerADC.pdf

*/

#if !defined _ADC_TIMER_CONFIG
    #define _ADC_TIMER_CONFIG

    #if defined SUPPORT_ADC                                              // if HW support is enabled
      //#define TEST_ADC                                                 // enable test of ADC operation
          //#define ADC_INTERNAL_TEMPERATURE                             // force internal temperature channel to be used, when available
          //#define TEST_POLL_ADC                                        // {25} poll ADC conversion complete rather than use end of conversion interrupt
            #define STM32_SCAN_MODE                                      // {33} STM32 scan mode operation rather than single input
      //#define TEST_AD_DA                                               // {14} enable test of reading ADC and writing (after delay) to DAC
          //#define ADC_TRIGGER_TPM                                      // use TPM module rather than PIT for ADC trigger (valid for KL parts)
          //#define VOICE_RECORDER                                       // {15} needs TEST_AD_DA and mass-storage and saves sampled input to SD card
          //#define HANDLE_PDB_INTERRUPT                                 // when the ADC is triggered by PDB handle also a PDB interrupt
          //#define FFT_SAMPLED_INPUT                                    // perform half-buffer interrupt and analyse the data with FFT (requires CMSIS_DSP_CFFT and CMSIS_DSP_FFT_4096)
                #define FFT_FLOATING_POINT                               // perform floating point FFT with result in a floating point array, else q15
                #if defined USE_CORTEX_CYCLE_COUNTER                     // only possible with Cortex-M4/M7
                    #define MEASURE_FFT_TIME
                #endif
      //#define INTERNAL_TEMP                                            // {2} read also internal temperature (Luminary Micro)

        #if defined TEST_ADC && (defined _HW_SAM7X || defined _HW_AVR32) // SAM7X and AVR32 specific tests
          //#define _SAM7X_ADC_TEST1                                     // software controlled ADC triggering with an interrupt on each channel
          //#define _SAM7X_ADC_TEST2                                     // software controlled ADC triggering with a single interrupt on last channel completion
            #define _SAM7X_ADC_TEST3                                     // ADTRG input triggered conversion with a single interrupt on last channel completion
          //#define _SAM7X_ADC_TEST4                                     // TIOA2 output triggered conversion with a single interrupt on last channel completion
          //#define _SAM7X_ADC_TEST5                                     // TIOA2 output triggered conversion with DMA support saving to a sample buffer
          //#define _SAM7X_ADC_TEST6                                     // test zero crossing, high and low triggers
        #endif
    #endif

    #if defined SUPPORT_DAC
      //#define TEST_DMA_DAC                                             // test generating a signal using DMA to DAC (based on timer trigger)
            #define GENERATE_SINE
            #if defined GENERATE_SINE
                #include <math.h>                                        // this may need libm.a explicitly linked (depending on IDE and compiler used)
            #endif
    #endif

    #if defined SUPPORT_COMPARATOR
      //#define TEST_COMPARATOR                                          // test a comparator interrupt
    #endif

    #if (defined SUPPORT_PIT1 || defined SUPPORT_PITS) && !defined KINETIS_WITHOUT_PIT // M522xx and Kinetis periodic interrupt timer
      //#define TEST_PIT                                                 // test a user defined periodic interrupt (choose one of the following if enabled)
          //#define TEST_PIT_SINGLE_SHOT                                 // test single-shot PIT
            #define TEST_PIT_PERIODIC                                    // test periodic PIT
              //#define TEST_PIT_DMA_GPIO                                // use PIT to toggle port output (requires SUPPORT_PIT_DMA_PORT_TOGGLE to be enabled)
        #if defined SERIAL_INTERFACE && defined UART_TIMED_TRANSMISSION
          //#define TIMED_UART_TX_TEST                                   // {23} demonstrate sending timed UART transmissions from a message
                #define TIMED_UART  1                                    // the UART to use
        #endif
    #endif
    #if defined SUPPORT_LPTMR && (LPTMR_AVAILABLE > 0)                   // Kinetis low power timer {18}
      //#define TEST_LPTMR_PERIODIC                                      // test a user defined periodic interrupt
      //#define TEST_LPTMR_SINGLE_SHOT                                   // test a user defined single-shot interrupt
    #endif
    #if defined SUPPORT_DMA_TIMER                                        // M522XX DMA timers
      //#define TEST_DMA_TIMER      ulCnt                                // test a user defined periodic interrupt
    #endif
    #if defined SUPPORT_GENERAL_PURPOSE_TIMER                            // M522XX general purpose timers
      //#define TEST_GPT                                                 // test general purpose timer operation
        #define GPT_CAPTURES     5                                       // when testing captures, collect this many values
    #endif
    #if defined SUPPORT_TIMER || defined SUPPORT_PWM_MODULE              // standard timers
      //#define TEST_TIMER                                               // enable timer test(s)
        #if defined TEST_TIMER
            #if defined SUPPORT_PWM_MODULE                               // {9}
                #define TEST_PWM                                         // {1} test generating PWM output from timer
                  //#define PHASE_SHIFTED_COMBINED_OUTPUTS               // {29} generate phase shifted outputs
                  //#define MULTIPLE_CHANNEL_INTERRUPTS                  // {30} generate multiple channel interrupts across multiple timers
              //#define TEST_STEPPER                                     // test generating stepper motor frequency patterns (use together with PWM)
              //#define TEST_SPWM                                        // {32} use together with TEST_STEPPER, TEST_DMA_DAC and GENERATE_SINE to generate a PWM signal using DMA that represents a sine wave 
            #endif
            #if defined SUPPORT_TIMER
              //#define TEST_SINGLE_SHOT_TIMER                           // test single-shot mode
              //#define TEST_PERIODIC_TIMER                              // test periodic interrupt mode
              //#define TEST_ADC_TIMER                                   // test periodic ADC trigger mode (Luminary)
              //#define TEST_CAPTURE                                     // {6} test timer capture mode
            #endif
        #endif
    #endif
    #define LENGTH_OF_TEST_BUFFER    32
    #if defined SUPPORT_RIT                                              // {7} LPC17XX repetitive interrupt timer
        #define RIT_TEST
    #endif
    #if defined VOICE_RECORDER                                           // {15}
        // RIFF WAVE header
        //
        __PACK_ON                                                        // compilers using pragmas to control packing will start struct packing from here
        typedef struct _PACK stRIFF_WAVE_HEADER
        {
            CHAR           cRiffTag[4];                                  // "RIFF" - riff header is 12 bytes in length
            unsigned long  ulRiffLength;                                 // data length - 8
            CHAR           cWaveTag[4];                                  // "WAVE"

            CHAR           cFormatTag[4];                                // "fmt " format paragraph is 24 bytes in length
            unsigned long  ulFormatLength;                               // always 16
            unsigned short usAudioFormat;                                // audio formal PCM = 1
            unsigned short usNumberOfChannels;                           // mono = 1, stereo = 2
            unsigned long  ulSampleRate;                                 // sample per second for each channel
            unsigned long  ulByteRate;                                   // sample rate x number of channels
            unsigned short usBlockAlign;                                 // frame size for sample in bytes
            unsigned short usBitsPerSample;                              // bits per sample

            CHAR           cDataTag[4];                                  // "data" data paragraph contains the sample values
            unsigned long  ulDataLength;                                 // data block length; 44 bytes less than the complete file size
        } RIFF_WAVE_HEADER;
        __PACK_OFF

        #define FORMAT_PCM 1
    #endif
    #if defined TEST_PIT_SINE_TO_USB
        extern void fnUSB_TimeBase(void);                                // routine in USB application that is called by the PIT interrupt
    #endif

/* =================================================================== */
/*                 local function prototype declarations               */
/* =================================================================== */

    #if defined TEST_ADC || defined TEST_AD_DA
        static void fnConfigureADC(void);
        #if defined _M5223X
            static void adc_level_change_low(ADC_INTERRUPT_RESULT *adc_result);
            static void adc_level_change_high(ADC_INTERRUPT_RESULT *adc_result);
        #endif
        #if defined VOICE_RECORDER && defined SDCARD_SUPPORT             // {15}
            static void fnSaveWaveToDisk(signed short *ptrInput, unsigned short usBufferLength);
            static void fnStartWaveDisk(void);
        #endif
    #endif
    #if defined TEST_COMPARATOR
        static void fnConfigureComparator(void);
    #endif
    #if defined SUPPORT_ADC && defined TEST_ADC && defined TEST_POLL_ADC // {25}
        static int fnCheckADC(int iChannel);
    #endif
    #if defined TEST_PIT || defined TEST_DMA_DAC
        static void fnConfigurePIT(void);
    #endif
    #if defined TIMED_UART_TX_TEST
        static void fnConfigureTimedUART(void);                          // {23}
    #endif
    #if defined SUPPORT_LPTMR && (defined TEST_LPTMR_PERIODIC || defined TEST_LPTMR_SINGLE_SHOT) // {18}
        static void fnConfigureLPTMR(void);
    #endif
    #if defined TEST_DMA_TIMER
        static void fnConfigure_DMA_Timer(void);
    #endif
    #if defined TEST_GPT
        static void fnConfigure_GPT(void);
    #endif
    #if defined TEST_TIMER
        static void fnConfigure_Timer(void);
        #if defined PWM_LED_CONTROL && defined TEST_PWM
            extern void fnSetColor(signed char x, signed char y);        // {19}
        #endif
    #endif
    #if defined TEST_ADC && defined _SAM7X_ADC_TEST5
        static void fnRestartADC(ADC_SETUP *adc_setup);
    #endif
    #if defined RIT_TEST
        static void fnConfigureRIT(void);
    #endif

/* =================================================================== */
/*                      local variable definitions                     */
/* =================================================================== */

    #if defined TEST_ADC
        #if defined _SAM7X_ADC_TEST5
            unsigned short *ptrADCValues = 0;
        #elif defined _LM3SXXXX
            #if defined INTERNAL_TEMP
                #define ADC_CHANNELS_LM3S   (ADC_CHANNELS + 1)
            #else
                #define ADC_CHANNELS_LM3S   ADC_CHANNELS
            #endif
            #define ADC_SEQUENCES  2
            #define ADC_SAMPLES_LM3SXXXX (ADC_CHANNELS_LM3S * ADC_SEQUENCES)  // 2 x all channel sample sequences (3 with temperature)
            static unsigned short usADC_samples[ADC_SAMPLES_LM3SXXXX];
        #endif
        #if defined DEV1
            int iBlockLed = 0;
            static const unsigned char controller[3] = { 0, 0, 1 };
            static const CHAR *adc_ref[3] = { "ADC0_DP0","ADC0_DM0","ADC1_DP0" };
            static int iAdcCnt = 0;
        #endif
        #if defined TEST_ADC && defined TEST_POLL_ADC                    // {25}
            static unsigned char ucADC_converting[ADC_CONTROLLERS] = {0};
        #endif
    #elif defined TEST_AD_DA                                             // {14}
        #if defined KINETIS_KL
            #define AD_DA_BUFFER_LENGTH    (256)                         // buffer for 31.25ms at 8k bytes/s
        #elif defined FFT_SAMPLED_INPUT
          //#define AD_DA_BUFFER_LENGTH    (4 * 1024)                    // buffer for 5ms at 400k bytes/s
            #define AD_DA_BUFFER_LENGTH    (8 * 1024)                    // buffer for 10.1ms at 400k bytes/s
          //#define AD_DA_BUFFER_LENGTH    (16 * 1024)                   // buffer for 20.23ms at 400k bytes/s
        #else
            #define AD_DA_BUFFER_LENGTH    (8 * 1024)                    // buffer for 1s at 8k bytes/s
        #endif
        static signed short sADC_buffer[AD_DA_BUFFER_LENGTH] = {0};      // 16 bit samples
        #if defined FFT_SAMPLED_INPUT
        static float windowing_buffer[AD_DA_BUFFER_LENGTH/2] = {0};      // windowing coefficients
        static float window_conversionFactor = 1.0;                      // scaling factor due to windowing
        static float fft_magnitude_buffer[AD_DA_BUFFER_LENGTH/4];        // fft frequency amplitude output
        #endif
    #endif
    #if defined TEST_AD_DA && (defined KINETIS_KL || defined KINETIS_KE)
        static signed short *ptrADC_buffer = 0;                          // pointer to aligned buffer - 16 bit samples 
    #endif
    #if defined TEST_GPT
        static unsigned long ulCaptureList[GPT_CAPTURES];                // make space for capture values
    #endif
    #if defined TIMED_UART_TX_TEST
        static QUEUE_HANDLE TimedUART_ID = NO_ID_ALLOCATED;              // UART handle for UART timed transmission demonstration
    #endif
    #if defined TEST_SPWM
        static unsigned short PWMTestBuffer[LENGTH_OF_TEST_BUFFER];
    #endif
#endif


/* =================================================================== */
/*                      Initialisation code                            */
/* =================================================================== */

#if defined _ADC_TIMER_INIT
    #if defined TEST_ADC || defined TEST_AD_DA
        fnConfigureADC();
        #if defined VOICE_RECORDER && defined SDCARD_SUPPORT             // {15}
        fnStartWaveDisk();                                               // configure an input interrupt to control starting and stopping voice recording
        #endif
    #endif
    #if defined TEST_COMPARATOR
        fnConfigureComparator();
    #endif
    #if defined TEST_PIT || defined TEST_DMA_DAC
        fnConfigurePIT();
    #endif
    #if defined TIMED_UART_TX_TEST
        fnConfigureTimedUART();
    #endif
    #if defined SUPPORT_LPTMR && (defined TEST_LPTMR_PERIODIC || defined TEST_LPTMR_SINGLE_SHOT) // {18}
        fnConfigureLPTMR();
    #endif
    #if defined TEST_DMA_TIMER
        fnConfigure_DMA_Timer();
    #endif
    #if defined TEST_GPT
        fnConfigure_GPT();
    #endif
    #if defined TEST_TIMER
        fnConfigure_Timer();
    #endif
    #if defined RIT_TEST
        fnConfigureRIT();
    #endif
#endif

#if defined _ADC_POLL_CODE                                               // {25} ADC polling code
    #if defined SUPPORT_ADC && defined TEST_ADC && defined TEST_POLL_ADC
        if (fnCheckADC(0) == 0) {                                        // if the ADC channel 0 conversion has completed
            fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER));  // generate trigger event to display the result
        }
    #endif
#endif

#if defined _ADC_TIMER_TIMER_EVENTS                                      // monostable timer event handling
    #if defined TEST_ADC && (defined _HW_SAM7X || defined _HW_AVR32)
        case E_NEXT_SAMPLE:
            {
                ADC_SETUP adc_setup;                                     // interrupt configuration parameters
                adc_setup.int_type = ADC_INTERRUPT;                      // identifier when configuring
        #if defined _SAM7X_ADC_TEST5
                fnRestartADC(&adc_setup);
        #else
                adc_setup.int_adc_mode = (ADC_START_OPERATION);          // start next software conversion
        #endif
                fnConfigureInterrupt((void *)&adc_setup);                // configure test interrupt on high level trigger
            }
            break;
    #endif
    #if defined TEST_ADC && (defined _LM3SXXXX || defined _KINETIS || defined _iMX || defined _STM32) // {11}
        case E_NEXT_SAMPLE:
            fnConfigureADC();                                            // repeat the ADC test
            break;
    #endif
    #if defined TIMED_UART_TX_TEST                                       // {23}
        case E_TIMER_TX_NOW:                                             // start next message transmission
            fnFlush(TimedUART_ID, FLUSH_TX);                             // the tx buffer is flushed so that there is only one message in the buffer at a time and its content starts at the first location of the buffer
            fnWrite(TimedUART_ID, (unsigned char *)"TIMED UART Transmission\r\n", 25);    // test message
            uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(2.0 * SEC), E_TIMER_TX_NOW); // repeat message transmission
            break;
    #endif
#endif
#if defined _ADC_TIMER_INT_EVENTS_1 && defined TEST_GPT                  // specific interrupt event handling
            case CAPTURE_COMPLETE_EVENT:
                {
                    int i = 0;
                    fnDebugMsg("Capture complete\r\n");
                    fnDebugMsg("================\r\n");
                    while (i < GPT_CAPTURES) {                           // list the capture samples
                        fnDebugHex(ulCaptureList[i], (WITH_LEADIN | WITH_CR_LF | sizeof(ulCaptureList[i])));
                        i++;
                    }
                }
                break;
#endif
#if defined _ADC_TIMER_INT_EVENTS_2  && defined SUPPORT_COMPARATOR && defined TEST_COMPARATOR // specific interrupt event handling
                if ((ucInputMessage[MSG_INTERRUPT_EVENT] >= CMP_EVENT_0_FALLING) && (ucInputMessage[MSG_INTERRUPT_EVENT] <= CMP_EVENT_3_FALLING_RISING)) {
                    unsigned long ulComparator = (ucInputMessage[MSG_INTERRUPT_EVENT] - CMP_EVENT_0_FALLING);
                    int iEvent = (ulComparator % 3);
                    ulComparator /= 3;
                    fnDebugMsg("CMP-");
                    fnDebugDec(ulComparator, 0);
                    fnDebugMsg(" = ");
                    switch (iEvent) {
                    case COMPARATOR_INTERRUPT_EVENT_FALLING:
                        fnDebugMsg("Falling");
                        break;
                    case COMPARATOR_INTERRUPT_EVENT_RISING:
                        fnDebugMsg("Rising");
                        break;
                    case COMPARATOR_INTERRUPT_EVENT_RISING_AND_FALLING:
                        fnDebugMsg("Rise/Fall");
                        break;
                    }
                    fnDebugMsg("\r\n");
                }
#endif
#if defined _ADC_TIMER_INT_EVENTS_2 && (defined TEST_ADC || defined TEST_AD_DA) // interrupt range event handling
    #if defined _M5223X || defined _KINETIS || defined _iMX              // {11}
                if ((ADC_LOW_7 >= ucInputMessage[MSG_INTERRUPT_EVENT]) && (ADC_HIGH_0 <= ucInputMessage[MSG_INTERRUPT_EVENT])) {
        #if !defined _KINETIS && !defined _iMX
                    ADC_SETUP adc_setup;                                 // interrupt configuration parameters
                    ADC_RESULTS results;
                    adc_setup.int_type = ADC_INTERRUPT;                  // identifier
                    adc_setup.int_priority = ADC_ERR_PRIORITY;           // port interrupt priority
                    adc_setup.int_adc_result = &results;
                    if (ucInputMessage[MSG_INTERRUPT_EVENT] < ADC_LOW_0) {
                        adc_setup.int_adc_bit = (ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_HIGH_0);                           // ADC bit 0
                        fnDebugMsg("ADC change high: ");
                        adc_setup.int_handler = adc_level_change_low;
                        adc_setup.int_adc_int_type = (ADC_LOW_LIMIT_INT);// interrupt types
                        adc_setup.int_low_level_trigger = (unsigned short)(ADC_VOLT * 1.5);
                    }
                    else  {
                        adc_setup.int_adc_bit = (ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_LOW_0);
                        fnDebugMsg("ADC change low: ");
                        adc_setup.int_handler = adc_level_change_high;
                        adc_setup.int_high_level_trigger = (unsigned short)(ADC_VOLT * 2);
                        adc_setup.int_adc_int_type = (ADC_HIGH_LIMIT_INT); // interrupt types
                    }
                    adc_setup.int_adc_mode = (ADC_START_OPERATION | ADC_GET_RESULT);// start operation now and return the present result
                    fnConfigureInterrupt((void *)&adc_setup);            // enter interrupt for low level trigger
                    fnDebugDec(adc_setup.int_adc_bit, 0);
                    fnDebugHex(results.sADC_value[adc_setup.int_adc_bit], (WITH_SPACE | WITH_LEADIN | WITH_CR_LF | 2));
        #endif
                }
                else if ((ADC_ZERO_CROSS_7 >= ucInputMessage[MSG_INTERRUPT_EVENT]) && (ADC_ZERO_CROSS_0 <= ucInputMessage[MSG_INTERRUPT_EVENT])) {
                    fnDebugMsg("Zero Crossing - channel ");
                    fnDebugDec((ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_ZERO_CROSS_0), WITH_CR_LF);
                }
        #if defined TEST_AD_DA                                           // {14}
                else if ((ADC_TRIGGER_1 == ucInputMessage[MSG_INTERRUPT_EVENT]) || (ADC_TRIGGER_2 == ucInputMessage[MSG_INTERRUPT_EVENT])) {
            #if defined KINETIS_KE
                    break;
            #else
                #if (defined VOICE_RECORDER && defined SDCARD_SUPPORT) || defined FFT_SAMPLED_INPUT || defined KWIKSTIK
                    signed short *ptrSample;
                    if ((ADC_TRIGGER_2 == ucInputMessage[MSG_INTERRUPT_EVENT])) {
                        ptrSample = &sADC_buffer[AD_DA_BUFFER_LENGTH/2]; // set sample pointer to second half of the buffer
                    }
                    else {
                        ptrSample = &sADC_buffer[0];                     // set sample pointer to first half of the buffer
                    }
                #endif
                #if defined VOICE_RECORDER && defined SDCARD_SUPPORT     // {15}
                    fnSaveWaveToDisk(ptrSample, (unsigned short)(AD_DA_BUFFER_LENGTH)); // if there is a disk ready, save the data to a file
                #elif defined FFT_SAMPLED_INPUT                          // preform an FFT analysis of the samples in the buffer
                    {
                    #if defined MEASURE_FFT_TIME
                        unsigned long ulTimeStamp = DWT_CYCCNT;          // time stamp before starting the FFT calculation
                    #endif
                        TOGGLE_TEST_OUTPUT();
                    #if defined FFT_FLOATING_POINT
                        fnFFT((void *)ptrSample, (void *)fft_magnitude_buffer, (AD_DA_BUFFER_LENGTH / 2), 0, ((AD_DA_BUFFER_LENGTH / 2) * sizeof(signed short)), windowing_buffer, window_conversionFactor, (FFT_INPUT_HALF_WORDS_UNSIGNED | FFT_CALCULATION_FLOAT | FFT_OUTPUT_FLOATS | FFT_MAGNITUDE_RESULT/* | FFT_RAM_COEFFICIENTS*/)); // perform complex fast-fourier transform (the result is in the input buffer)
                    #else
                        fnFFT((void *)ptrSample, (void *)fft_magnitude_buffer, (AD_DA_BUFFER_LENGTH / 2), 0, ((AD_DA_BUFFER_LENGTH / 2) * sizeof(signed short)), windowing_buffer, window_conversionFactor, (FFT_INPUT_HALF_WORDS_UNSIGNED | FFT_CALCULATION_Q15 | FFT_OUTPUT_FLOATS | FFT_MAGNITUDE_RESULT)); // perform complex fast-fourier transform (the result is in the input buffer)
                    #endif
                        TOGGLE_TEST_OUTPUT();
                    #if defined MEASURE_FFT_TIME
                        ulTimeStamp = ((DWT_CYCCNT - ulTimeStamp) / (SYSTEM_CLOCK / 1000000)); // calculate the time the calculation took
                        fnDebugMsg("FFT = ");
                        fnDebugDec(ulTimeStamp, 0);                      // us
                        fnDebugMsg("us]\r\n");
                    #endif
                    }
                #elif defined KWIKSTIK                                   // remove input's DC bias, amplify the signal and then add the bias again
                    {
                        int i;
                        for (i = 0; i < (AD_DA_BUFFER_LENGTH/2); i++) {  // for each sample in the buffer half
                            *ptrSample -= 0x0800;                        // remove DC bias
                            if (*ptrSample > (2047/16)) {                // limit positive input value if needed (positive saturation)
                                *ptrSample = (2047/16);                  // maximum positive input value
                            }
                            else if (*ptrSample < (-2048/16)) {          // limit negative input value if needed (negative saturation)
                                *ptrSample = (-2048/16);                 // maximum negative input value
                            }
                            *ptrSample *= 16;                            // amplify the AC value
                            *ptrSample++ += 0x0800;                      // add the DC bias again
                        }
                    }
                #endif
            #endif
                }
        #else
                else if ((ADC_TRIGGER == ucInputMessage[MSG_INTERRUPT_EVENT])
            #if defined _KINETIS || defined _iMX
                    || (ADC_TRIGGER_1 == ucInputMessage[MSG_INTERRUPT_EVENT])
            #endif
                  ) {
                    ADC_SETUP adc_setup;                                 // interrupt configuration parameters
                    ADC_RESULTS results;
                    adc_setup.int_type = ADC_INTERRUPT;                  // identifier
                    adc_setup.int_adc_mode = (ADC_READ_ONLY | ADC_GET_RESULT);
            #if defined _KINETIS || defined _iMX                         // {11}
                #if defined DEV1
                    adc_setup.int_adc_controller = controller[iAdcCnt];
                #else
                    if (ADC_TRIGGER_1 == ucInputMessage[MSG_INTERRUPT_EVENT]) {
                        adc_setup.int_adc_controller = 1;
                    }
                    else {
                        adc_setup.int_adc_controller = 0;
                    }
                    #if defined TWR_K20D50M || defined TWR_K20D72M || defined FRDM_K20D50M || defined TWR_K21D50M || defined TEENSY_3_1
                    adc_setup.int_adc_controller = 0;
                    #endif
                #endif
            #else
                    adc_setup.int_adc_bit = 0;
            #endif
                    adc_setup.int_adc_result = &results;
                    fnConfigureInterrupt((void *)&adc_setup);
                    fnDebugMsg("ADC triggered:");
            #if defined DEV1
                    fnDebugMsg((CHAR *)adc_ref[iAdcCnt]);
                    if (++iAdcCnt >= 3) {
                        iAdcCnt = 0;
                    }
            #endif
                    fnDebugHex(results.sADC_value[0], (WITH_SPACE | WITH_LEADIN | WITH_CR_LF | sizeof(results.sADC_value[0]))); // {4}
            #if defined _KINETIS || defined _iMX                         // {11}
                #if defined ADC_INTERNAL_TEMPERATURE
                    #define VTEMP_25_16BIT               ((VTEMP_25_MV * 0xffff) / ADC_REFERENCE_VOLTAGE)
                    #define VTEMP_25_12BIT               (VTEMP_25_16BIT >> 4)
                    #define VTEMP_25_10BIT               (VTEMP_25_16BIT >> 6)
                    #define TEMP_SENSOR_SLOPE_100_16BIT  ((TEMP_SENSOR_SLOPE_UV * 0xffff)/(10 * ADC_REFERENCE_VOLTAGE))
                    #define TEMP_SENSOR_SLOPE_100_12BIT  (TEMP_SENSOR_SLOPE_100_16BIT >> 4)
                    #define TEMP_SENSOR_SLOPE_100_10BIT  (TEMP_SENSOR_SLOPE_100_16BIT >> 6)
                    if (ADC_TRIGGER == ucInputMessage[MSG_INTERRUPT_EVENT]) { // convert the ADC reading to a temperature
                        signed short ssDifference = (results.sADC_value[0] - VTEMP_25_12BIT); // convert references to 12 bit values
                        signed long slTemperature100 = (2550 - ((ssDifference * 10000) / TEMP_SENSOR_SLOPE_100_12BIT)); // {26} the approximate temperature *100, rounded up/down
                        slTemperature100 /= 100;                         // the approximate temperature rounded up/down to 1�C
                        fnDebugDec(slTemperature100, DISPLAY_NEGATIVE);
                        fnDebugMsg(" degC\r\n");
                    }
                #endif
                #if defined GLOBAL_TIMER_TASK
                    uTaskerGlobalMonoTimer(OWN_TASK, (DELAY_LIMIT)(3.0 * SEC), E_NEXT_SAMPLE);
                #else
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(3.0 * SEC), E_NEXT_SAMPLE);
                #endif
            #endif
                }
        #endif
    #elif defined _HW_SAM7X || defined _HW_AVR32
                if (ADC_TRIGGER == ucInputMessage[MSG_INTERRUPT_EVENT]) {
        #if defined _SAM7X_ADC_TEST5
                    int iSamples;
                    unsigned short *ptrSamples = ptrADCValues;
                    fnDebugMsg("ADC results ready:\r\n");
                    fnDebugMsg("First 128 samples -\r\n");
                    for (iSamples = 0; iSamples < 128; iSamples++) {
                        fnDebugHex(*ptrSamples++, (WITH_SPACE | WITH_LEADIN | 2));
                        if (iSamples%8 == 7) {
                            fnDebugMsg("\r\n");
                        }
                    }
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(3.0 * SEC), E_NEXT_SAMPLE);
        #else
                    ADC_SETUP   adc_setup;                               // interrupt configuration parameters
                    ADC_RESULTS results;
                    int iChannel;
                    adc_setup.int_type = ADC_INTERRUPT;                  // identifier
                    adc_setup.int_adc_mode = (ADC_READ_ONLY | ADC_ALL_RESULTS);
                    adc_setup.int_adc_result = &results;
                    fnConfigureInterrupt((void *)&adc_setup);
                    fnDebugMsg("ADC results:\r\n");
                    for (iChannel = 0; iChannel < ADC_CHANNELS; iChannel++) {
                        fnDebugDec(iChannel, 0);
                        fnDebugMsg(": ");
                        fnDebugHex(results.sADC_value[iChannel], (WITH_SPACE | WITH_LEADIN | WITH_CR_LF | 2));
                    }
            #if defined _SAM7X_ADC_TEST1 || defined _SAM7X_ADC_TEST2
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1.0 * SEC), E_NEXT_SAMPLE);
            #endif
                    fnDebugMsg("\n");
        #endif
                }
                else if ((ADC_TRIGGER_0 <= ucInputMessage[MSG_INTERRUPT_EVENT]) && (ADC_TRIGGER_7 >= ucInputMessage[MSG_INTERRUPT_EVENT])) {
                    ADC_SETUP   adc_setup;                               // interrupt configuration parameters
                    ADC_RESULTS results;
                    adc_setup.int_type = ADC_INTERRUPT;                  // identifier
                    adc_setup.int_adc_mode = (ADC_READ_ONLY | ADC_GET_RESULT);
                    adc_setup.int_adc_bits = (0x01 << (ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_TRIGGER_0));
                    adc_setup.int_adc_result = &results;
                    fnConfigureInterrupt((void *)&adc_setup);
                    fnDebugMsg("ADC-");
                    fnDebugDec((ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_TRIGGER_0), 0);
                    fnDebugMsg(":");
                    fnDebugHex(results.sADC_value[ucInputMessage[MSG_INTERRUPT_EVENT] - ADC_TRIGGER_0], (WITH_SPACE | WITH_LEADIN | WITH_CR_LF | 2));
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(1.0 * SEC), E_NEXT_SAMPLE);
                }
        #if defined _SAM7X_ADC_TEST6
                else if ((ADC_TRIGGER_5_ZERO <= ucInputMessage[MSG_INTERRUPT_EVENT]) && (ADC_TRIGGER_6_HIGH >= ucInputMessage[MSG_INTERRUPT_EVENT])) {
                    fnDebugMsg("ADC trigger channel ");
                    if ((ADC_TRIGGER_5_HIGH == ucInputMessage[MSG_INTERRUPT_EVENT]) || (ADC_TRIGGER_5_ZERO == ucInputMessage[MSG_INTERRUPT_EVENT]) || (ADC_TRIGGER_5_LOW == ucInputMessage[MSG_INTERRUPT_EVENT])) {
                        fnDebugMsg("5:");
                    }
                    else {
                        fnDebugMsg("6: ");
                        ucInputMessage[MSG_INTERRUPT_EVENT] -= 1;
                    }
                    switch (ucInputMessage[MSG_INTERRUPT_EVENT]) {
                    case ADC_TRIGGER_5_LOW:
                        fnDebugMsg("low trigger\r\n");
                        break;
                    case ADC_TRIGGER_5_HIGH:
                        fnDebugMsg("high trigger\r\n");
                        break;
                    case ADC_TRIGGER_5_ZERO:
                        fnDebugMsg("zero crossing trigger\r\n");
                        break;
                    }
                }
        #endif
    #elif defined _STM32
                if (ADC_TRIGGER == ucInputMessage[MSG_INTERRUPT_EVENT]) {                // {33}
                    int x;
                    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
                    ADC_RESULTS results;
                    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
                    adc_setup.int_adc_mode = (ADC_GET_RESULT);
                    adc_setup.int_adc_result = 0;
                    adc_setup.int_adc_controller = STM32_ADC_1;
                    adc_setup.int_adc_result = &results;
                    fnConfigureInterrupt((void *)&adc_setup);                            // get the results

                    fnDebugMsg("ADC Value:\r\n");
                    fnDebugMsg("==========");
                    for (x = 0; x < ADC_CHANNELS; x++) {
                        if (results.ucADC_status[x] == ADC_RESULT_VALID) {
                            fnDebugMsg("\r\nSample-");
                            fnDebugDec(x, 0);
                            fnDebugMsg(" = ");
                            fnDebugHex(results.usADC_value[x], (WITH_LEADIN | sizeof(results.usADC_value[x])));
                        }
                    }
                    fnDebugMsg("\r\n");
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(5 * SEC), E_NEXT_SAMPLE);
                }
    #elif defined _LM3SXXXX
                if (ADC_TRIGGER == ucInputMessage[MSG_INTERRUPT_EVENT]) {
                    int x;
                    fnDebugMsg("ADC Values:\r\n");
                    fnDebugMsg("===========");
                    for (x = 0; x < ADC_SAMPLES_LM3SXXXX; x++) {
                        fnDebugMsg("\r\nSample-");
                        fnDebugDec(x, 0);
                        fnDebugMsg(" = ");
                        fnDebugHex(usADC_samples[x], (WITH_LEADIN | sizeof(usADC_samples[x])));
                    }
                    fnDebugMsg("\r\n");
                    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(5 * SEC), E_NEXT_SAMPLE);
                }
    #endif
#endif



#if defined _ADC_TIMER_ROUTINES && (defined TEST_ADC || defined TEST_AD_DA)
    #if defined _HW_SAM7X || defined _HW_AVR32
        #if defined _SAM7X_ADC_TEST1
static void adc_sample_complete(ADC_INTERRUPT_RESULT *adc_result)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_0 + adc_result->ucADC_channel));
}
        #elif !defined _SAM7X_ADC_TEST6
static void adc_all_samples_complete(ADC_INTERRUPT_RESULT *adc_result)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)ADC_TRIGGER);
}
        #endif
        #if defined _SAM7X_ADC_TEST6
static void adc_trigger(ADC_INTERRUPT_RESULT *adc_result)
{
    switch (adc_result->ucADC_channel) {
    case 5:
    case 6:
        if (adc_result->ucADC_flags & ADC_INT_ZERO_CROSSING) {
            fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_5_ZERO + (adc_result->ucADC_channel - 5)));
        }
        if (adc_result->ucADC_flags & ADC_INT_LOW_LEVEL) {
            fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_5_LOW  + (adc_result->ucADC_channel - 5)));
        }
        if (adc_result->ucADC_flags & ADC_INT_HIGH_LEVEL) {
            fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_5_HIGH + (adc_result->ucADC_channel - 5)));
        }
        break;
    }
}
        #endif

// ADC configuration for SAM7X / AVR32
//
static void fnConfigureADC(void)
{
        #if defined _WINDOWS
            #define ADC_SAMPLES 10
        #else
            #define ADC_SAMPLES 1024
        #endif
    #define ADC_INPUTS  3
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_priority = ADC_ERR_PRIORITY;                           // ADC interrupt priority
    adc_setup.int_adc_bits = (ADC_CHANNEL_0 | ADC_CHANNEL_5 | ADC_CHANNEL_6); // enable ADC bits 0, 5 and 6
    adc_setup.int_adc_result = 0;                                        // no result is requested
    adc_setup.int_adc_speed = (ADC_SAMPLING_SPEED(5000000));             // 5MHz sampling (5MHz is maximum for 10 bit mode and 8MHz maximum for 8 bit mode)

        #if defined _SAM7X_ADC_TEST1
    adc_setup.int_adc_int_type = (ADC_END_OF_SCAN_INT);                  // interrupt type - interrupt for each channel
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_SHOT_MODE | ADC_START_OPERATION); // single shot conversion, triggered immediately
    adc_setup.int_handler = adc_sample_complete;                         // handling function
        #elif defined _SAM7X_ADC_TEST2
    adc_setup.int_adc_int_type = (ADC_END_OF_SCAN_INT | ADC_SINGLE_SHOT_TRIGGER_INT); // interrupt type - single interrupt for last channel completed
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_SHOT_MODE | ADC_START_OPERATION); // single shot conversion, triggered immediately
    adc_setup.int_handler = adc_all_samples_complete;                    // handling function
        #elif defined _SAM7X_ADC_TEST3
    adc_setup.int_adc_int_type = (ADC_END_OF_SCAN_INT | ADC_SINGLE_SHOT_TRIGGER_INT); // interrupt type - single interrupt for last channel completed
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_SHOT_MODE | ADC_TRIGGERED_EXT); // single shot conversion, triggered immediately
    adc_setup.int_handler = adc_all_samples_complete;                    // handling function
        #elif defined _SAM7X_ADC_TEST4
    adc_setup.int_adc_int_type = (ADC_END_OF_SCAN_INT | ADC_SINGLE_SHOT_TRIGGER_INT); // interrupt type - single interrupt for last channel completed
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_LOOP_MODE | ADC_TRIGGERED_TIM2); // single shot conversion, triggered immediately
    adc_setup.int_handler = adc_all_samples_complete;                    // handling function
    adc_setup.int_adc_sample_rate = 1;                                   // 1Hz sampling rate - controlled by timer 2
        #elif defined _SAM7X_ADC_TEST5
    ptrADCValues = uMalloc(ADC_SAMPLES * 2 * ADC_INPUTS);                // space for sample values (in 10 bit mode each sample occupies 2 bytes)
    adc_setup.int_adc_int_type = (ADC_ENABLE_INTS);                      // interrupt type - interrupt on sample buffer full
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_LOOP_MODE | ADC_TRIGGERED_TIM2); // single shot conversion, triggered immediately
    adc_setup.int_handler = adc_all_samples_complete;                    // handling function
    adc_setup.int_adc_sample_rate = 1000;                                // 1kHz sampling rate - controlled by timer 2
    adc_setup.int_samples = (ADC_SAMPLES * ADC_INPUTS);                  // sample buffer size to be filled before interrupting
    adc_setup.int_adc_result = (void *)ptrADCValues;
        #elif defined _SAM7X_ADC_TEST6
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_LOOP_MODE | ADC_TRIGGERED_TIM2); // configure automatic trigger mode of operation
    adc_setup.int_zero_crossing_trigger = (unsigned short)(ADC_VOLT_10BIT * 1.65); // zero crossing value
    adc_setup.int_high_level_trigger = (unsigned short)(ADC_VOLT_10BIT * 2.5);// high trigger level
    adc_setup.int_low_level_trigger = (unsigned short)(ADC_VOLT_10BIT * 0.7); // low trigger level
    adc_setup.int_adc_sample_rate = 1;                                   // 1Hz sampling rate - controlled by timer 2
    adc_setup.int_adc_bits = (ADC_CHANNEL_5);                            // enable ADC bit 5
    adc_setup.int_handler = adc_trigger;                                 // handling function
    adc_setup.int_adc_int_type = (ADC_ZERO_CROSSING_INT_POSITIVE | ADC_ZERO_CROSSING_INT_NEGATIVE | ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT | ADC_SINGLE_SHOT_TRIGGER_INT | ADC_SINGLE_SHOT_CROSSING_INT); // interrupt type - interrupt on and triggers but only once on each type
    fnConfigureInterrupt((void *)&adc_setup);                            // configure
    adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_LOOP_MODE);    // add a trigger channel
    adc_setup.int_zero_crossing_trigger = (unsigned short)(ADC_VOLT_10BIT * 1.65); // zero crossing value
    adc_setup.int_high_level_trigger = (unsigned short)(ADC_VOLT_10BIT * 3.1);// high trigger level
    adc_setup.int_low_level_trigger = (unsigned short)(ADC_VOLT_10BIT * 0.35);// low trigger level
    adc_setup.int_adc_bits = (ADC_CHANNEL_6);                            // enable ADC bit 6
    adc_setup.int_adc_int_type = (ADC_ZERO_CROSSING_INT_POSITIVE | ADC_ZERO_CROSSING_INT_NEGATIVE | ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT); // interrupt type - interrupt continuously
        #endif
    fnConfigureInterrupt((void *)&adc_setup);                            // configure
}
        #if defined _SAM7X_ADC_TEST5
static void fnRestartADC(ADC_SETUP *adc_setup)
{
    adc_setup->int_adc_int_type = (ADC_ENABLE_INTS);                     // interrupt type - interrupt on sample buffer full
    adc_setup->int_priority = ADC_ERR_PRIORITY;                          // ADC interrupt priority
    adc_setup->int_adc_bits = (ADC_CHANNEL_0 | ADC_CHANNEL_5 | ADC_CHANNEL_6);// enable ADC bits 0, 5 and 6
    adc_setup->int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_LOOP_MODE | ADC_TRIGGERED_TIM2); // single shot conversion, triggered immediately
    adc_setup->int_handler = adc_all_samples_complete;                   // handling function
    adc_setup->int_adc_sample_rate = 1000;                               // 1kHz sampling rate - controlled by timer 2
    adc_setup->int_samples = (ADC_SAMPLES * ADC_INPUTS);                 // sample buffer
    adc_setup->int_adc_result = (void *)ptrADCValues;
}
        #endif
    #elif defined _LM3SXXXX
static void adc_samples_ready(void)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)ADC_TRIGGER);
}

static void fnConfigureADC(void)
{
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = adc_samples_ready;                           // handling function
    adc_setup.int_priority = PRIORITY_ADC;                               // ADC interrupt priority
        #if ADC_CHANNELS >= 8                                            // Luminary devices with 8 or more ADC inputs
  //adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_0 | ADC_CHANNEL_1 | ADC_CHANNEL_2 | ADC_CHANNEL_3); // ADC channels 0..3 as single ended inputs 
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_8 | ADC_CHANNEL_9 | ADC_CHANNEL_10 | ADC_CHANNEL_11); // ADC channels 0..3 as single ended inputs 
  //adc_setup.int_adc_differential_inputs = (ADC_CHANNEL_4 | ADC_CHANNEL_5 | ADC_CHANNEL_6 | ADC_CHANNEL_7); // ADC channels 4-5 and 6-7 as differential inputs
    adc_setup.int_adc_differential_inputs = (ADC_CHANNEL_12 | ADC_CHANNEL_13 | ADC_CHANNEL_14 | ADC_CHANNEL_15); // ADC channels 12-13 and 14-15 as differential inputs
        #else
            #if defined INTERNAL_TEMP
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_0 | ADC_CHANNEL_1 | ADC_CHANNEL_4_INTERNAL_TEMP);// ADC channels 0..1 as single ended inputs plus internal temperature
            #else
    adc_setup.int_adc_single_ended_inputs = (ADC_CHANNEL_0 | ADC_CHANNEL_1); // ADC channels 0..1 as single ended inputs
            #endif
    adc_setup.int_adc_differential_inputs = (ADC_CHANNEL_2 | ADC_CHANNEL_3); // ADC channels 2..3 as differential inputs
        #endif
        #if defined TEST_ADC_TIMER
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_TRIGGER_TIMER);    // triggered from timer
        #else
  //adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_TRIGGER_GPIO_PB4_FALLING); // triggered by Port B-4 on a falling edge
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_START_OPERATION);  // immediate start under SW control
        #endif
    adc_setup.int_adc_averaging = HW_AVERAGING_64;                       // basic sampling speed is 1MHz but can be averaged to improve accuracy or reduce speed
  //adc_setup.int_adc_averaging = NO_HW_AVERAGING;                       // disable hardware averaging for highest speed
    adc_setup.int_adc_result = usADC_samples;                            // location to save the samples to
    adc_setup.int_sequence_count = ADC_SEQUENCES;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
}
    #elif defined _STM32                                                 // {33}
// This interrupt is called when the ADC conversion has completed
//
static void __callback_interrupt adc_ready(ADC_INTERRUPT_RESULT *adc_result)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER));
}

static void fnConfigureADC(void)
{
#if !defined STM32_SCAN_MODE
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_handler = adc_ready;                                   // handling function
    adc_setup.int_priority = PRIORITY_ADC;                               // ADC interrupt priority
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_CLOCK_PCLK_DIV_8 | ADC_RESULT_RIGHT_ALIGNED | ADC_12_BIT_MODE | ADC_SINGLE_SHOT_MODE | ADC_CHANNEL_CYCLES_480 | ADC_START_OPERATION); // immediate start under SW control
    adc_setup.int_adc_int_type = 0;
    adc_setup.int_adc_result = 0;
    adc_setup.int_adc_controller = STM32_ADC_1;
    adc_setup.int_adc_bit = ADC123_IN10;
    adc_setup.int_watchdog_bit = ADC123_IN10;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
#else
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    static int init = 0;
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
    adc_setup.int_adc_controller = STM32_ADC_1;
    if (init != 0) {
        adc_setup.int_adc_mode = (ADC_START_OPERATION);                  // avoid adding additional scan channels after first call
    }
    else {
        adc_setup.int_handler = adc_ready;                               // handling function
        adc_setup.int_priority = PRIORITY_ADC;                           // ADC interrupt priority
        adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_CLOCK_PCLK_DIV_8 | ADC_RESULT_RIGHT_ALIGNED | ADC_12_BIT_MODE | ADC_SINGLE_SHOT_MODE | ADC_CHANNEL_CYCLES_480 | ADC_SEQUENTIAL_MODE); // configure ADC and channel but don't start yet
        adc_setup.int_adc_int_type = 0;
        adc_setup.int_adc_result = 0;
        adc_setup.int_adc_bit = ADC123_IN10;
        fnConfigureInterrupt((void *)&adc_setup);                        // configure ADC and first scan channel
        adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_SEQUENTIAL_MODE);
        adc_setup.int_adc_bit = ADC123_IN11;
        fnConfigureInterrupt((void *)&adc_setup);                        // configure next scan channel
        adc_setup.int_adc_bit = ADC123_IN12;
        fnConfigureInterrupt((void *)&adc_setup);                        // configure next scan channel
        adc_setup.int_adc_bit = ADC123_IN13;
        adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_SEQUENTIAL_MODE | ADC_START_OPERATION); // configure next sequence channel and immediate start under SW control
        init = 1;
    }
    fnConfigureInterrupt((void *)&adc_setup);                            // configure and start sequence
#endif
}

    #else                                                                // M5223X / Kinetis
        #if defined TEST_ADC && defined TEST_POLL_ADC                    // {25}
static int fnCheckADC(int iChannel)
{
    if (ucADC_converting[iChannel] != 0) {                               // if an ADC conversion has been started
        ADC_RESULTS adc_results;
        ADC_SETUP adc_setup;                                             // interrupt configuration parameters
        adc_setup.int_type = ADC_INTERRUPT;                              // identifier
        adc_setup.int_adc_mode = (ADC_READ_ONLY | ADC_CHECK_CONVERSION); // check whether the conversion has completed and don't wait for it to become ready
        adc_setup.int_adc_controller = 0;
        adc_setup.int_adc_result = &adc_results;
        fnConfigureInterrupt((void *)&adc_setup);
        if (adc_results.ucADC_status[0] == ADC_RESULT_VALID) {           // if the conversion is ready
    #if !defined FREE_RUNNING_RX_DMA_RECEPTION
            uTaskerStateChange(OWN_TASK, UTASKER_STOP);                  // stop polling operation
    #endif
            ucADC_converting[iChannel] = 0;                              // conversion has completed
            return 0;
        }
        else {
            return 1;                                                    // ADC is still busy
        }
    }
    return 2;                                                            // not active
}
        #else
// This interrupt is called when the ADC level changes above programmed threshold (on one of the enabled channels)
//
static void __callback_interrupt adc_level_change_high(ADC_INTERRUPT_RESULT *adc_result)
{
        #if defined _KINETIS || defined _iMX
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER));
        #else
    if (adc_result == 0) {
        fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER));
        return;
    }
    if (adc_result->ucADC_flags == ADC_INT_ZERO_CROSSING) {
        fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_ZERO_CROSS_0 + adc_result->ucADC_channel));
    }
    else {
        fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_HIGH_0 + adc_result->ucADC_channel));
    }
        #endif
}
        #endif

#if (defined _KINETIS || defined _iMX) && (ADC_CONTROLLERS > 1)
static void __callback_interrupt adc_range(ADC_INTERRUPT_RESULT *adc_result)
{
    #if defined _KINETIS || defined _iMX
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_1));
    #else
    if (0 == adc_result) {
        fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_1));
        return;
    }
    #endif
}
#endif

#if defined _M5223X
// This interrupt is called when the ADC level changes below programmed threshold (on one of the enabled channels)
//
static void __callback_interrupt adc_level_change_low(ADC_INTERRUPT_RESULT *adc_result)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_LOW_0 + adc_result->ucADC_channel));
}
#endif

#if defined TEST_AD_DA                                                   // {14}
    #if (defined VOICE_RECORDER && defined SDCARD_SUPPORT) || defined KWIKSTIK || defined FFT_SAMPLED_INPUT
// Half buffer interrupt occurs each time the ADC DMA has filled half of the input buffer
// - the ADC DMA operation will continue with the second half of the buffer so that the user has time to clear the existing data
//
static void __callback_interrupt half_buffer_interrupt(void)
{
    static int iPingPong = 0;
    TOGGLE_TEST_OUTPUT();
    fnInterruptMessage(OWN_TASK, (unsigned char)(ADC_TRIGGER_1 + iPingPong)); // send alternating event to follow the half-buffer that is ready to be processed
    TOGGLE_TEST_OUTPUT();
    iPingPong ^= 1;
}
    #endif
    #if PDB_AVAILABLE > 0 && defined HANDLE_PDB_INTERRUPT
// If enabled, the PDB interrupt occurs at 8kHz
//
static void __callback_interrupt _pdb_interrupt(void)
{
    _CONFIG_PORT_OUTPUT(A, (DEMO_LED_3), PORT_SRE_SLOW);                 // configure LED outputs for test purpose
    _TOGGLE_PORT(A, DEMO_LED_3);                                         // toggle LED for measurement purpose
}
    #endif

// Configure the PDB/LPTMR/PIT/PTM to generate ADC triggers (ADC channels must already be configured previous to starting the timer)
//
static void fnStart_ADC_Trigger(void)
{
    #if PDB_AVAILABLE == 0                                               // for devices that do not have a PDB a PIT or FlexTimer is used instead to trigger the ADC/DAC
        #if defined ADC_TRIGGER_TPM                                      // ADC triggering from TPM 1 - channel 0 and 1 (these channels are the default ADC triggers for ADC 0 inputs A and B)
    PWM_INTERRUPT_SETUP pwm_setup;
    pwm_setup.int_type = PWM_INTERRUPT;
    pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_16 | PWM_CENTER_ALIGNED | PWM_DMA_PERIOD_ENABLE | PWM_NO_OUTPUT); // clock PWM timer from the system clock with /16 pre-scaler (don't use an output)
    pwm_setup.int_handler = 0;                                           // no user interrupt call-back on PWM cycle
    pwm_setup.pwm_frequency = PWM_FREQUENCY(1000, 16);                   // generate 1kHz on PWM output
    pwm_setup.pwm_value = _PWM_PERCENT(1, pwm_setup.pwm_frequency);      // 1% PWM (high/low)
    pwm_setup.pwm_reference = (_TIMER_1 | 0);                            // timer module 1, channel 0 (triggers ADC0 input A)
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_value = _PWM_PERCENT(99, pwm_setup.pwm_frequency);     // 99% PWM (high/low)
    pwm_setup.pwm_reference = (_TIMER_1 | 1);                            // timer module 1, channel 1 (triggers ADC0 input B)
    fnConfigureInterrupt((void *)&pwm_setup);
        #else
    PIT_SETUP pit_setup;                                                 // interrupt configuration parameters
    pit_setup.int_type = PIT_INTERRUPT;
    pit_setup.int_handler = 0;                                           // no interrupt used since the PIT triggers ADC/DAC only
    pit_setup.int_priority = PIT0_INTERRUPT_PRIORITY;
    pit_setup.count_delay = PIT_US_DELAY(125);                           // 8kHz period
    pit_setup.ucPIT = 0;                                                 // use PIT0 since it is the only one that can trigger DAC conversions in Kinetis
    pit_setup.mode = (PIT_PERIODIC | PIT_RETRIGGER | PIT_TRIGGER_ADC0_A);// periodically trigger ADC0 channel A (PIT0 trigger was defined in ADC configuration) - uses retrigger in case the PIT was running previously
    fnConfigureInterrupt((void *)&pit_setup);                            // configure PIT0
        #endif
    #else
    PDB_SETUP pdb_setup;                                                 // interrupt configuration parameters
    pdb_setup.int_type = PDB_INTERRUPT;
        #if defined HANDLE_PDB_INTERRUPT
    pdb_setup.int_handler = _pdb_interrupt;                              // interrupt on each PDB cycle match
        #else
    pdb_setup.int_handler = 0;                                           // no interrupt
        #endif
    pdb_setup.int_priority = PRIORITY_PDB;    
    //pdb_setup.pdb_mode = (PDB_PERIODIC_DMA | PDB_TRIGGER_ADC1_A);      // periodic DMA and trigger ADC1 - channel A
    #if defined KWIKSTIK || defined TEENSY_3_1
    pdb_setup.pdb_mode = (PDB_PERIODIC_INTERRUPT | PDB_TRIGGER_ADC0_A);  // periodic interrupt and trigger ADC0 - channel A
    #else
    pdb_setup.pdb_mode = (PDB_PERIODIC_INTERRUPT | PDB_TRIGGER_ADC1_A);  // periodic interrupt and trigger ADC1 - channel A
    #endif
    //pdb_setup.pdb_mode = PDB_MONO_TIMER_INTERRUPT;                     // single-shot timer interrupt
    pdb_setup.prescaler = (PDB_PRESCALER_4 | PDB_MUL_1);                 // pre-scaler values of 1, 2, 4, 8, 16, 32, 64 and 128 are possible (with multipliers of 1, 10, 20 or 40)
    #if defined FFT_SAMPLED_INPUT
    pdb_setup.period = PDB_FREQUENCY(4, 1, 400000);                      // frequency of PDB cycle is 400kHz
    #else
    pdb_setup.period = PDB_FREQUENCY(4, 1, 8000);                        // frequency of PDB cycle is 8kHz
    #endif
    pdb_setup.int_match = 0;                                             // PDB interrupt/DMA at the start of the period so that it uses the old ADC value
    pdb_setup.ch0_delay_0 = pdb_setup.period;                            // ADC0 channel A trigger occurs at end of the PDB period
    pdb_setup.ch0_delay_1 = 0;
    pdb_setup.ch1_delay_0 = pdb_setup.period;                            // ADC1 channel A trigger occurs at end of the PDB period
    pdb_setup.ch1_delay_1 = 0;
    #if defined SUPPORT_DAC
    pdb_setup.pdb_mode &= ~PDB_PERIODIC_INTERRUPT;
    pdb_setup.pdb_mode |= PDB_PERIODIC_DMA;                              // use DMA to trigger DAC data writes
    pdb_setup.dac0_delay_0 = 0;
    #endif
    pdb_setup.pdb_trigger = PDB_TRIGGER_SW;                              // triggered by software (started immediately)
    fnConfigureInterrupt((void *)&pdb_setup);                            // configure PDB interrupt
    #endif
}
#endif


static void fnConfigureADC(void)
{
#if defined DEV1
    static const unsigned char input[3] = { ADC_DP0_SINGLE, ADC_DM0_SINGLE, ADC_DP0_SINGLE };
    static unsigned long ulCalibrate[3] = { ADC_CALIBRATE, ADC_CALIBRATE, ADC_CALIBRATE };
#elif defined DEV5
    extern void setup(void);
    setup();
    return;
#endif
#if (defined _KINETIS || defined _iMX) && (!defined KINETIS_KE || defined KINETIS_KE15 || defined KINETIS_KE18) && !defined DEV1   // {11}
    static unsigned long ulCalibrate = ADC_CALIBRATE;
#endif
    ADC_SETUP adc_setup;                                                 // interrupt configuration parameters
    adc_setup.int_type = ADC_INTERRUPT;                                  // identifier when configuring
#if defined _KINETIS || defined _iMX                                     // {11}
    #if !defined DEVICE_WITHOUT_DMA
    adc_setup.dma_int_priority = 3;                                      // priority of DMA interrupt the user wants to set
    adc_setup.dma_int_handler = 0;                                       // no interrupt so that free-running circular buffer is used (when ADC_FULL_BUFFER_DMA_AUTO_REPEAT is not defined)
        #if defined KINETIS_KL && defined TEST_AD_DA && defined ADC_TRIGGER_TPM
    adc_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_TPM1_OVERFLOW;   // trigger DMA TPM overflows
        #else
    adc_setup.usDmaTriggerSource = 0;                                    // default trigger is the ADC conversion completion of the channel in question
        #endif
    #endif
    #if !defined KINETIS_KE
    adc_setup.pga_gain = PGA_GAIN_OFF;                                   // {13} PGA gain can be specified for certain inputs
    #endif
    adc_setup.int_priority = PRIORITY_ADC;                               // ADC interrupt priority
    adc_setup.int_adc_controller = 0;                                    // ADC controller 0
    #if defined TEST_POLL_ADC                                            // {25}
    adc_setup.int_handler = 0;                                           // no interrupt used
    ucADC_converting[adc_setup.int_adc_controller] = 1;                  // conversion is in progress
    uTaskerStateChange(OWN_TASK, UTASKER_POLLING);                       // set the task to polling mode to regularly check the receive buffer
    #else
    adc_setup.int_handler = adc_level_change_high;                       // handling function
    #endif
    adc_setup.int_adc_int_type = (ADC_SINGLE_SHOT_TRIGGER_INT);          // interrupt type
    adc_setup.int_adc_offset = 0;                                        // no offset
    #if defined ADC_INTERNAL_TEMPERATURE
    adc_setup.int_adc_bit = ADC_TEMP_SENSOR;                             // ADC internal temperature
    #else
        #if defined TWR_K20D50M || defined TWR_K20D72M || defined TWR_K21D50M
    adc_setup.int_adc_bit = ADC_DM3_SINGLE;                              // ADC DM3 single-ended
    adc_setup.int_handler = adc_range;                                   // handling function
        #elif defined TEENSY_3_1
    adc_setup.int_adc_bit = ADC_DP3_SINGLE;                              // ADC DM3 single-ended - pad A12
    adc_setup.int_handler = adc_range;                                   // handling function
        #elif defined FRDM_KE02Z40M || defined FRDM_KE06Z
    adc_setup.int_adc_bit = ADC_SE12_SINGLE;                             // thermistor positive terminal
        #elif defined FRDM_KL43Z || defined TEST_COMPARATOR
    adc_setup.int_adc_bit = ADC_SE23_SINGLE;
        #elif defined FRDM_KL03Z
    adc_setup.int_adc_bit = ADC_SE0_SINGLE;
        #elif defined FRDM_KL27Z
    adc_setup.int_adc_bit = ADC_SE6_SINGLE;
        #else
    adc_setup.int_adc_bit = ADC_TEMP_SENSOR;                             // ADC internal temperature
        #endif
    #endif
    #if (defined TWR_K20D50M || defined TWR_K20D72M || defined TWR_K21D50M) && !defined DEV1
    adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_4 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED | ADC_SINGLE_SHOT_MODE | ADC_16_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion {12}
    adc_setup.int_adc_sample = (ADC_SAMPLE_LONG_PLUS_12 | ADC_SAMPLE_AVERAGING_32); // additional sampling clocks
    #else
        #if defined TEST_AD_DA                                           // {14}
            #if defined VOICE_RECORDER && defined SDCARD_SUPPORT
    adc_setup.dma_int_handler = half_buffer_interrupt;                   // half-buffer interrupt so that the signal can be saved to disk
                #if !(defined TWR_K20D50M || defined TWR_K20D72M || defined TWR_K21D50M || TEENSY_3_1) && (ADC_CONTROLLERS > 1)
    adc_setup.int_adc_controller = 1;                                    // ADC controller 1
    adc_setup.int_adc_bit = ADC_DM1_SINGLE;                              // potentiometer on K60 board
                #endif
            #elif defined KWIKSTIK
    adc_setup.int_adc_bit = ADC_SE10_SINGLE;                             // microphone on kwikstik board
    adc_setup.dma_int_handler = half_buffer_interrupt;                   // half-buffer interrupt so that the signal can be amplified
            #elif defined FRDM_KL26Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27
    adc_setup.int_adc_bit = ADC_SE8_SINGLE;                              // A0 [J4-2]
            #else
                #if (ADC_CONTROLLERS > 1)
    adc_setup.int_adc_controller = 1;                                    // ADC controller 1
                #endif
                #if defined FRDM_K66F || defined TEENSY_3_6
    adc_setup.int_adc_bit = ADC_DM0_SINGLE;
                #else
    adc_setup.int_adc_bit = ADC_DM1_SINGLE;                              // potentiometer on K60/K70 tower boards
                #endif
                #if defined FFT_SAMPLED_INPUT
    adc_setup.dma_int_handler = half_buffer_interrupt;                    // interrupt called when the buffer is half full
    window_conversionFactor = fnGenerateWindowFloat(windowing_buffer, (AD_DA_BUFFER_LENGTH/2), BLACKMANN_HARRIS_WINDOW); // calculate a window for use by the FFT
                #endif
    adc_setup.int_handler = 0;                                           // no ADC conversion interrupt
            #endif
            #if !defined DEVICE_WITHOUT_DMA
                #if defined KINETIS_KL                                   // {21}
    adc_setup.ucDmaChannel = 2;                                          // DMA channel 2 used
                #else
    adc_setup.ucDmaChannel = 6;                                          // DMA channel 6 used
                #endif
    adc_setup.ptrADC_Buffer = sADC_buffer;                               // ADC sample buffer to be used
    adc_setup.ulADC_buffer_length = sizeof(sADC_buffer);                 // physical length of the buffer
            #endif
            #if defined VOICE_RECORDER && defined SDCARD_SUPPORT         // {15}
    adc_setup.int_adc_mode = (ulCalibrate | /*ADC_LOOP_MODE |*/ ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_HW_TRIGGERED); // hardware triggering example (DMA to buffer with interrupt on half-buffer completion) - requires PDB set up afterwards
    adc_setup.int_adc_sample = (ADC_SAMPLE_LONG_PLUS_12 | ADC_SAMPLE_AVERAGING_8); // additional sampling clocks
                #if defined DEVICE_WITH_eDMA                             // {21}
    adc_setup.int_adc_mode |= ADC_FULL_BUFFER_DMA_AUTO_REPEAT;           // automated DMA (using interrupt) restart when not using modulo repetitions
                #endif
            #else
                #if defined KINETIS_KL                                   // {21}
                    #if defined ADC_CLOCK_BUS_DIV_2
  //adc_setup.int_adc_mode = (ulCalibrate | ADC_LOOP_MODE | ADC_FULL_BUFFER_DMA | ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_4 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE); // continuous conversion (DMA to buffer)
    adc_setup.int_adc_mode = (ulCalibrate | ADC_FULL_BUFFER_DMA | ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_HW_TRIGGERED); // hardware triggering (DMA to buffer)
                    #else                                                // not all KL parts (eg. KL82) have the bus/2 option
  //adc_setup.int_adc_mode = (ulCalibrate | ADC_LOOP_MODE | ADC_FULL_BUFFER_DMA | ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS | ADC_CLOCK_DIVIDE_4 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE); // continuous conversion (DMA to buffer)
    adc_setup.int_adc_mode = (ulCalibrate | ADC_FULL_BUFFER_DMA | ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_HW_TRIGGERED); // hardware triggering (DMA to buffer)
                    #endif
                    #if !defined DEVICE_WITH_eDMA
    adc_setup.int_adc_mode |= ADC_FULL_BUFFER_DMA_AUTO_REPEAT;           // automated DMA (using interrupt) restart when not using modulo repetitions
                    #endif
                    #if !defined DEVICE_WITHOUT_DMA
    adc_setup.dma_int_handler = 0;                                       // no user DMA interrupt call-back
                    #endif
                #else
                    #if defined FFT_SAMPLED_INPUT
    adc_setup.int_adc_mode = (ulCalibrate | ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_2/* | ADC_SAMPLE_ACTIVATE_LONG*/ | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_HW_TRIGGERED); // hardware triggering example (DMA to buffer with interrupt on half-buffer completion) - requires PDB set up afterwards
                    #else
    adc_setup.int_adc_mode = (ulCalibrate | /*ADC_LOOP_MODE |*/ ADC_HALF_BUFFER_DMA | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_HW_TRIGGERED); // hardware triggering example (DMA to buffer with interrupt on half-buffer completion) - requires PDB set up afterwards
                    #endif
                #endif
                #if defined FFT_SAMPLED_INPUT
    adc_setup.int_adc_sample = (ADC_SAMPLE_HIGH_SPEED_CONFIG | ADC_SAMPLE_AVERAGING_OFF); // additional sampling clocks and hardware averaging
                #else
    adc_setup.int_adc_sample = (ADC_SAMPLE_LONG_PLUS_12 | ADC_SAMPLE_AVERAGING_8); // additional sampling clocks and hardware averaging
                #endif
            #endif
    adc_setup.int_adc_bit_b = ADC_TEMP_SENSOR;                           // input B is only valid when using HW triggered mode
        #elif defined KINETIS_KE15
        // KE15's ADC uses a fixed clock source from the PCC
        //
    adc_setup.int_adc_mode = (ulCalibrate | ADC_CLOCK_DIVIDE_4 | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion
    adc_setup.int_adc_sample = (12); // sampling time (2..256 clocks)
        #elif defined KINETIS_KE
    adc_setup.int_adc_mode = (ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED | ADC_LOW_POWER_CONFIG); // single shot with interrupt on completion {12}
        #else
            #if defined KINETIS_KL82
    adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion
  //adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_IRC48MCLK | ADC_CLOCK_DIVIDE_4 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion
            #elif defined KINETIS_KL28
        // Warning - ADC_CLOCK_BUS_DIV_2 will not work on KL28
        // - bus, alternative clock or asynchronous clocks can be used instead
        //  
  //adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_ASYNCHRONOUS | ADC_CLOCK_DIVIDE_2 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion
  //adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_ALTERNATE_SOURCE | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion
    adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A /*| ADC_CLOCK_BUS_DIV_2*/ | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion {12}
    #elif defined DEV1
    adc_setup.int_adc_mode = (ulCalibrate[iAdcCnt] | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion {12}
            #else
    adc_setup.int_adc_mode = (ulCalibrate | ADC_SELECT_INPUTS_A | ADC_CLOCK_BUS_DIV_2 | ADC_CLOCK_DIVIDE_8 | ADC_SAMPLE_ACTIVATE_LONG | ADC_CONFIGURE_ADC | ADC_REFERENCE_VREF | ADC_CONFIGURE_CHANNEL | ADC_SINGLE_ENDED_INPUT | ADC_SINGLE_SHOT_MODE | ADC_12_BIT_MODE | ADC_SW_TRIGGERED); // note that the first configuration should calibrate the ADC - single shot with interrupt on completion {12}
             #endif
    adc_setup.int_adc_sample = (ADC_SAMPLE_LONG_PLUS_12 | ADC_SAMPLE_AVERAGING_32); // additional sampling clocks
        #endif
    #endif
    #if defined FRDM_KL27Z
    adc_setup.int_adc_mode |= ADC_SELECT_INPUTS_B;
    #endif
    adc_setup.int_adc_result = 0;                                        // no result is requested
#else
    adc_setup.int_handler = adc_level_change_high;                       // handling function
    adc_setup.int_priority = ADC_ERR_PRIORITY;                           // ADC interrupt priority
    adc_setup.int_adc_bit = 0;                                           // ADC bit 0
    adc_setup.int_adc_int_type = (ADC_HIGH_LIMIT_INT);                   // interrupt types
  //adc_setup.int_adc_int_type = (ADC_END_OF_SCAN_INT | ADC_SINGLE_SHOT_TRIGGER_INT); // use to test SYNCA trigger
    adc_setup.int_adc_offset = 0;                                        // no offset
    adc_setup.int_high_level_trigger = (unsigned short)(ADC_VOLT * 2);
    adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SEQUENTIAL_MODE | ADC_SINGLE_ENDED_INPUT | ADC_LOOP_MODE); // use to test single ended
  //adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SEQUENTIAL_MODE | ADC_DIFFERENTIAL_INPUT | ADC_LOOP_MODE); // use to test differential
  //adc_setup.int_adc_mode = (ADC_CONFIGURE_ADC | ADC_CONFIGURE_CHANNEL | ADC_SEQUENTIAL_MODE | ADC_TRIGGERED_MODE); // use to test SYNCA trigger
    adc_setup.int_adc_speed = (unsigned char)(ADC_SAMPLING_SPEED(5000000)); // 5MHz sampling (must be between 100kHz and 5MHz)
    adc_setup.int_adc_result = 0;                                        // no result is requested
#endif
#if defined DEV1
    adc_setup.int_adc_bit = input[iAdcCnt];
    adc_setup.int_adc_controller = controller[iAdcCnt];
    ulCalibrate[iAdcCnt] = 0;
#endif
    fnConfigureInterrupt((void *)&adc_setup);                            // configure ADC
#if defined TEST_AD_DA                                                   // {14}
    #if defined SUPPORT_DAC && (DAC_CONTROLLERS > 0)                     // configure DAC for delay-line output 
    {
        DAC_SETUP dac_setup;
        dac_setup.int_type = DAC_INTERRUPT;
        dac_setup.int_handler = 0;                                       // no interrupt used
        dac_setup.int_priority = INTERRUPT_LOWEST_PRIORITY;              // lowest priority (not used in this case)
        dac_setup.dac_mode = (DAC_CONFIGURE | DAC_REF_VDDA | DAC_NON_BUFFERED_MODE | DAC_FULL_BUFFER_DMA | DAC_ENABLE | DAC_BUFFER_DMA_START/* | DAC_OUTPUT_VALUE*/); // configure the DAC to use VDDA as reference voltage in non-buffered mode (using DMA)
      //dac_setup.usOutputValue = 0;                                     // prime first output value (when DAC_OUTPUT_VALUE is enabled)
        #if defined KWIKSTIK
        // Enable audio amplifier output on DAC1
        //
        _CONFIG_DRIVE_PORT_OUTPUT_VALUE_FAST_HIGH(E, (AUDIO_AMPLIFIER_OUTPUT), (AUDIO_AMPLIFIER_OUTPUT), PORT_SRE_SLOW);
        dac_setup.int_dac_controller = 1;                                // DAC 1
        #else
        dac_setup.int_dac_controller = 0;                                // DAC 0
        #endif
        #if defined KINETIS_KL
        dac_setup.ptrDAC_Buffer = (unsigned short *)&sADC_buffer[0];
        dac_setup.ulDAC_buffer_length = (AD_DA_BUFFER_LENGTH * sizeof(unsigned short));
        dac_setup.ucDmaChannel = 1;                                      // DMA channel 1 used
            #if defined KINETIS_KL && defined ADC_TRIGGER_TPM
        dac_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_TPM1_OVERFLOW; // trigger DMA to DAC when TPM overflows
            #else
        dac_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_ADC0;        // trigger DMA to DAC when ADC0 sample completes
            #endif
        dac_setup.dac_mode |= DAC_HW_TRIGGER_MODE;                       // use HW trigger mode rather than SW triggered mode
        dac_setup.dac_mode |= DAC_FULL_BUFFER_DMA_AUTO_REPEAT;           // automated DMA restart (using interrupt) when not using modulo repetitions
        #else
        dac_setup.ucDmaChannel = 7;                                      // use DMA channel 7
        dac_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_PDB0;
        dac_setup.ptrDAC_Buffer = (unsigned short *)sADC_buffer;         // DAC transmit buffer to be used (use the ADC buffer to create a digital delay line)
        dac_setup.ulDAC_buffer_length = sizeof(sADC_buffer);             // physical length of the buffer
        #endif
        fnConfigureInterrupt((void *)&dac_setup);                        // configure DAC
        fnStart_ADC_Trigger();                                           // configure the used timer to trigger ADC and DAC
    }
    return;
    #else                                                                // configure PWM for delay-line output
    {
        PWM_INTERRUPT_SETUP pwm_setup;
        pwm_setup.int_type = PWM_INTERRUPT;
        pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_1 | PWM_FULL_BUFFER_DMA); // clock PWM timer from the system clock with /1 pre-scaler
        #if defined KINETIS_KL && !defined DEVICE_WITHOUT_DMA
        pwm_setup.pwm_mode |= PWM_FULL_BUFFER_DMA_AUTO_REPEAT;           // automated DMA (using interrupt) restart when not using modulo repetitions
        #endif
        pwm_setup.pwm_frequency = PWM_TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(2000), 2); // generate 2000Hz on PWM output
        pwm_setup.pwm_value   = _PWM_PERCENT(50, pwm_setup.pwm_frequency); // 50% PWM (high/low)
        pwm_setup.pwm_reference = (_TIMER_2 | 0);                        // timer module 2, channel 0 (red LED in RGB LED of KL26 and KL27 boards)
        pwm_setup.int_priority = 0;
        pwm_setup.int_handler = 0;                                       // no user interrupt call-back on PWM cycle
        #if !defined DEVICE_WITHOUT_DMA
        pwm_setup.dma_int_priority = 0;
        pwm_setup.dma_int_handler = 0;                                   // no user interrupt call-back on DMA transfer completion
        pwm_setup.ucDmaChannel = 0;                                      // DMA channel 0 used (highest priority)
        pwm_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_ADC0;
        pwm_setup.ptrPWM_Buffer = (unsigned short *)sADC_buffer;         // PWM buffer to be used (use the ADC buffer to create a digital delay line)
        pwm_setup.ulPWM_buffer_length = (AD_DA_BUFFER_LENGTH * sizeof(unsigned short)); // physical length of the buffer
        #endif
        fnConfigureInterrupt((void *)&pwm_setup);
        fnStart_ADC_Trigger();                                           // configure the used timer to trigger ADC
    }
    #endif
#endif
#if defined _KINETIS || defined _iMX                                     // {11}
    #if (ADC_CONTROLLERS > 1) && !defined DEV1 && !defined TEST_COMPARATOR
    adc_setup.int_adc_controller = 1;                                    // ADC controller 1
    adc_setup.int_adc_bit = ADC_DM1_SINGLE;                              // ADC DM1 single-ended
  //adc_setup.int_adc_int_type = (ADC_LOW_LIMIT_INT);                    // interrupt type (trigger only when lower than the defined level)
  //adc_setup.int_adc_int_type = (ADC_HIGH_LIMIT_INT);                   // interrupt type (trigger only when higher than the defined level)
    adc_setup.int_adc_int_type = (ADC_LOW_LIMIT_INT | ADC_HIGH_LIMIT_INT); // interrupt type (trigger only when lower or higher than defined levels)
    adc_setup.int_low_level_trigger = (unsigned short)(ADC_VOLT * 1.3);  // the low level trigger threshold represented as input voltage (note: setting low threshold higher than the high threshold causes a trigger inside the range rather than outside of it)
    adc_setup.int_high_level_trigger = (unsigned short)(ADC_VOLT * 2.6); // the high level trigger threshold represented as input voltage
    adc_setup.int_handler = adc_range;                                   // handling function
    fnConfigureInterrupt((void *)&adc_setup);                            // start operation now
    #endif
#else
  //return;                                                              // used when testing SYNCA trigger

/*  example of configuring all ADC inputs with same parameters (M522xx)
    adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL);
    adc_setup.int_adc_bit = 1;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 2;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 3;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 4;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 5;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 6;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
    adc_setup.int_adc_bit = 7;
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on high level trigger
*/
    adc_setup.int_adc_mode = (ADC_CONFIGURE_CHANNEL | ADC_SET_CHANNEL_OFFSET);
    adc_setup.int_adc_bit = 1;                                           // channel 1 used to test zero crossing with offset
    adc_setup.int_adc_offset = (unsigned short)(ADC_VOLT * 2.5);         // offset adjustment - for zero crossing
    adc_setup.int_adc_int_type = (ADC_ZERO_CROSSING_INT_POSITIVE | ADC_SINGLE_SHOT_CROSSING_INT); // interrupt types
    fnConfigureInterrupt((void *)&adc_setup);                            // configure test interrupt on zero crossing (2.5V)

    adc_setup.int_adc_int_type = 0;                                      // no interrupt configuration to perform
    adc_setup.int_adc_mode = (ADC_START_OPERATION);
    fnConfigureInterrupt((void *)&adc_setup);                            // start operation now
#endif
#if (defined _KINETIS || defined _iMX) && !defined KINETIS_KE && !defined DEV1
    ulCalibrate = 0;                                                     // calibrate ADC only once
#endif
}
    #endif                                                               // end M5223X / Kinetis



#if defined VOICE_RECORDER && defined SDCARD_SUPPORT                     // {15}

static int iStartRecording = 0;                                          // recording state and debouncing
static int iStopRecording = 0;

// Interrupt routine to start/stop recording
//
static void __callback_interrupt fnStartStopRecording(void)
{
    if (iStartRecording == 0) {                                          // if idle immediately start recording
        iStartRecording = 1;
    }
    else if (iStartRecording >= 4) {                                     // following interrupt (after debounce time) stops the recording
        iStopRecording = 4;
    }
}

// Configure an interrupt used to start and stop recording (port interrupt support is required)
//
static void fnStartWaveDisk(void)
{
    INTERRUPT_SETUP interrupt_setup;                                     // interrupt configuration parameters
    interrupt_setup.int_type       = PORT_INTERRUPT;                     // identifier to configure port interrupt
    interrupt_setup.int_handler    = fnStartStopRecording;               // handling function
    interrupt_setup.int_priority   = WAVE_DISK_START_STOP_INT_PRIORITY;  // interrupt priority level
    interrupt_setup.int_port       = WAVE_DISK_START_STOP_INT_PORT;      // the port that the interrupt input is on
    interrupt_setup.int_port_bits  = WAVE_DISK_START_STOP_INT_BIT;       // the port bit used
    interrupt_setup.int_port_sense = (IRQ_FALLING_EDGE | PULLUP_ON);     // interrupt is to be falling edge sensitive
    fnConfigureInterrupt((void *)&interrupt_setup);                      // configure interrupt
}

// This is called when there is an ADC buffer ready to be committed to disk
//
static void fnSaveWaveToDisk(signed short *ptrInput, unsigned short usBufferLength)
{
    static UTDIRECTORY *ptr_utWavDirectory = 0;                         // pointer to a directory object
    static UTFILE utWavFile = {0};                                      // local file object
    static unsigned long ulWaveLength = 0;
    static RIFF_WAVE_HEADER wav_header = {
        {'R', 'I', 'F', 'F'},
        0,
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16,                                                              // this must be 16
        FORMAT_PCM,
        1,                                                               // mono
        8000,                                                            // 8000 samples per second
        (8000 * 2),                                                      // bytes per second
        2,                                                               // 2 bytes for each sample
        16,                                                              // 16 bits samples
        {'d', 'a', 't', 'a'},
        0,                                                               // initially the data rate is zero
    };

    if (iStopRecording != 0) {                                           // close the file
        if (ptr_utWavDirectory != 0) {                                   // valid disk present
            wav_header.ulRiffLength = (ulWaveLength - 8);                // the header length field values
            wav_header.ulDataLength = (ulWaveLength - sizeof(wav_header));
            utSeek(&utWavFile, 0, UTFAT_SEEK_SET);                       // move back to start of the file
            utWriteFile(&utWavFile, (unsigned char *)&wav_header, sizeof(wav_header)); // write new header
            utCloseFile(&utWavFile);                                     // work complete
            ulWaveLength = 0;                                            // reset ready for next recording session
            ptr_utWavDirectory = utFreeDirectory(ptr_utWavDirectory);    // {17} free the directory object
            uTaskerMonoTimer(TASK_WATCHDOG, (DELAY_LIMIT)(0.20 * SEC), 0); // set standard LED blinking rate again
        }
        if (--iStopRecording == 0) {                                     // debounce the stop interrupt
            iStartRecording = 0;
        }
    }
    else if (iStartRecording != 0) {                                     // save buffer to disk
        if (ptr_utWavDirectory == 0) {                                   // directory has not yet been opened
            ptr_utWavDirectory = utAllocateDirectory(DISK_D, 0);         // allocate a directory for use by this module associated with D: use root
        }
        if ((ptr_utWavDirectory->usDirectoryFlags & UTDIR_VALID) == 0) { // directory not valid
            if (utOpenDirectory(0, ptr_utWavDirectory) != UTFAT_SUCCESS) { // open the root directory
                return;                                                  // do nothing until a card is detected
            }
            if (UTFAT_PATH_IS_FILE != utOpenFile(RECORDER_WAVE_FILE_NAME, &utWavFile, ptr_utWavDirectory, (UTFAT_CREATE | UTFAT_OPEN_FOR_WRITE | UTFAT_OPEN_FOR_READ | UTFAT_APPEND | UTFAT_COMMIT_FILE_ON_CLOSE))) {
                return;
            }
            uTaskerMonoTimer(TASK_WATCHDOG, (DELAY_LIMIT)(0.05 * SEC), 0); // speed up LED during recording
            utSeek(&utWavFile, 0, UTFAT_SEEK_SET);                       // set to start of the file
            if (utWavFile.ulFileSize >= sizeof(wav_header)) {            // see whether there is already a header in the file
                utReadFile(&utWavFile, (unsigned char *)&wav_header, sizeof(wav_header)); // load existing header
                ulWaveLength = (wav_header.ulRiffLength + 8);            // valid length
                utSeek(&utWavFile, ulWaveLength, UTFAT_SEEK_SET);        // move to the end of the existing valid wave data
            }
        }
        if (utWavFile.ulFileSize == 0) {                                 // empty file so set the initial header
            if ((utWavFile.ulFileMode & UTFAT_OPEN_FOR_WRITE) == 0) {    // file has been closed so ignore further data
                return;
            }
            utWriteFile(&utWavFile, (unsigned char *)&wav_header, sizeof(wav_header)); // write an inital header with zero lengths
            ulWaveLength = sizeof(wav_header);                           // initial file content length
        }
        if (utWriteFile(&utWavFile, (unsigned char *)ptrInput, usBufferLength) == UTFAT_SUCCESS) { // save the data block
            ulWaveLength += usBufferLength;                              // new complete valid file length
        }
        if (iStartRecording < 4) {                                       // debounce start interrupt
            iStartRecording++;
        }
    }
}
#endif
#endif                                                                   // endif ADC configuration and interrupt handling routines

#if defined _ADC_TIMER_ROUTINES && defined SUPPORT_COMPARATOR && defined TEST_COMPARATOR
static void __callback_interrupt comparator_trigger(int iCMP_reference, int iEvent)
{
    fnInterruptMessage(OWN_TASK, (unsigned char)(CMP_EVENT_0_FALLING + iCMP_reference + iCMP_reference +iCMP_reference + iEvent));
}

static void fnConfigureComparator(void)
{
    COMPARATOR_SETUP comparator_setup;                                   // interrupt configuration parameters
    comparator_setup.int_type = COMPARATOR_INTERRUPT;                    // identifier when configuring
    comparator_setup.int_priority = PRIORITY_COMPARATOR;                 // comparator interrupt priority
    comparator_setup.int_handler = comparator_trigger;                   // interrupt call-back
    comparator_setup.comparator = 1;                                     // comparator 1
    comparator_setup.comparator_mode = (COMPARATOR_BUS_CLOCK | COMPARATOR_HYSTERISIS_LEVEL_0 | COMPARATOR_HIGH_SPEED_MODE |
                                        COMPARATOR_OUTPUT_ENABLE | COMPARATOR_OUTPUT_UNFILTERED |
                                        COMPARATOR_INTERRUPT_RISING_AND_FALLING |
                                        COMPARATOR_DAC_ENABLE | COMPARATOR_DAC_VOLTAGE_VDD |
                                        COMPARATOR_ENABLE);
    comparator_setup.filter = 0;                                         // filter sample count(1..7) in external sampling mode or 0..255 in bus clock mode
    comparator_setup.DAC_level = 31;                                     // half-voltage reference generated
    comparator_setup.positive_input = COMPARATOR_IN3;                    // positive input
    comparator_setup.negative_input = COMPARATOR_IN_DAC_OUT;             // negative input
    fnConfigureInterrupt((void *)&comparator_setup);                     // start operation
}
#endif

#if defined _ADC_TIMER_ROUTINES && defined SUPPORT_LPTMR && (defined TEST_LPTMR_PERIODIC || defined TEST_LPTMR_SINGLE_SHOT) // {18}
static void __callback_interrupt low_power_timer_int(void)
{
    TOGGLE_TEST_OUTPUT();
}

static void fnConfigureLPTMR(void)
{
    LPTMR_SETUP lptmr_setup;                                             // interrupt configuration parameters
    lptmr_setup.int_type = LPTMR_INTERRUPT;
    lptmr_setup.int_handler = low_power_timer_int;                       // test a single shot timer
    lptmr_setup.count_delay = LPTMR_US_DELAY(3245);                      // 3245us delay
  //lptmr_setup.count_delay = LPTMR_S_DELAY(5);                          // 5s delay
    #if defined TEST_LPTMR_PERIODIC
    lptmr_setup.mode = LPTMR_PERIODIC;                                   // periodic interrupt
    #else
    lptmr_setup.mode = LPTMR_SINGLE_SHOT;                                // one-shot interrupt
    #endif
    #if LPTMR_AVAILABLE > 1
    lptmr_setup.int_priority = LPTMR1_INTERRUPT_PRIORITY;
    lptmr_setup.ucTimer = 1;                                             // user low power timer 1
    #else
    lptmr_setup.int_priority = LPTMR0_INTERRUPT_PRIORITY;
    lptmr_setup.ucTimer = 0;                                             // user low power timer 0
    #endif
    fnConfigureInterrupt((void *)&lptmr_setup);                          // enter interrupt for LPTMR test
}
#endif


#if defined _ADC_TIMER_ROUTINES && (defined TEST_PIT || defined TEST_DMA_DAC)
    #if defined TEST_PIT_SINGLE_SHOT
static void test_timer_int(void)
{
    static int iTimerTest = 0;
    PIT_SETUP pit_setup;                                                 // interrupt configuration parameters
    TOGGLE_TEST_OUTPUT();
    switch (iTimerTest++) {
    case 0:
        pit_setup.count_delay = PIT_MS_DELAY(1877);                      // 1.877s delay
        pit_setup.mode = PIT_SINGLE_SHOT;                                // one-shot interrupt
        break;
    case 1:
        pit_setup.count_delay = PIT_S_DELAY(5);                          // 5s periodic
        pit_setup.mode = PIT_PERIODIC;                                   // periodic interrupt
        break;
    case 6:
        iTimerTest = 0;                                                  // reset for next test
        pit_setup.mode = PIT_STOP;                                       // stop timer
        break;
    default:
        return;
    }
    pit_setup.int_type = PIT_INTERRUPT;
    pit_setup.int_handler = test_timer_int;
    pit_setup.int_priority = PIT1_INTERRUPT_PRIORITY;
    #if defined SUPPORT_PITS
    pit_setup.ucPIT = 0;                                                 // use PIT0
    #endif
    fnConfigureInterrupt((void *)&pit_setup);                            // enter interrupt for PIT1 test
}
    #elif defined TEST_PIT_PERIODIC && !defined TEST_DMA_DAC
static __callback_interrupt void test_nmi(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle to show periodic interrupt
}
    #endif

// PIT configuration
//
static void fnConfigurePIT(void)
{
    #if defined TEST_DMA_DAC                                             // {20}
    static unsigned short *ptrTestBuffer = 0;
    int i;
    DAC_SETUP dac_setup;
    #endif
    PIT_SETUP pit_setup;                                                 // PIT interrupt configuration parameters
    pit_setup.int_type = PIT_INTERRUPT;
    #if defined TEST_DMA_DAC                                             // {20}
    pit_setup.int_priority = PIT0_INTERRUPT_PRIORITY;                    // not used
    pit_setup.count_delay = PIT_US_DELAY(100);                           // 10kHz
    pit_setup.mode = (PIT_PERIODIC);                                     // periodic DMA trigger
    pit_setup.int_handler = 0;                                           // no interrupt since the PIT will be used for triggering DMA
        #if defined KINETIS_KL && !defined DEVICE_WITH_eDMA
    ptrTestBuffer = uMallocAlign((LENGTH_OF_TEST_BUFFER * sizeof(unsigned short)), (LENGTH_OF_TEST_BUFFER * sizeof(unsigned short))); // modulo aligned buffer
        #else
    ptrTestBuffer = uMalloc(LENGTH_OF_TEST_BUFFER * sizeof(unsigned short)); // non-modulo-aligned memory is possible
        #endif
    // Define the waveform to be generated
    //
        #if defined GENERATE_SINE
    {
        #if !defined PI
            #define PI           (double)3.14159265
        #endif
        #if defined USE_CMSIS_SIN_COS
            float fSineValue;
            #define ANGLE_STEP   (float)((double)(2 * PI)/(double)LENGTH_OF_TEST_BUFFER);
            float angle = 0;
        #else
            #define ANGLE_STEP   (double)((double)(2 * PI)/(double)LENGTH_OF_TEST_BUFFER);
            double angle = 0;
        #endif
        for (i = 0; i < LENGTH_OF_TEST_BUFFER; i++) {                    // prepare a sine wave
        #if defined USE_CMSIS_SIN_COS
            arm_sin_cos_f32(angle, &fSineValue, 0);
            #if defined TEST_SPWM
            PWMTestBuffer[i] = (unsigned short)((signed short)(fSineValue * (float)0x7fff) + (0xffff/2)); // 16 bit scaling and half-scale DC offset added
            if (PWMTestBuffer[i] == 0) {
                PWMTestBuffer[i] = 1; // avoid 0 since it will not cause a match!!!!
            }
            #endif
            fSineValue *= (float)/*0x7ff*/0x700;                                  // 12 bit scaling
            ptrTestBuffer[i] = (unsigned short)((signed short)fSineValue + (0xfff/2)); // half-scale DC offset added
        #else
            ptrTestBuffer[i] = (unsigned short)((sin(angle) * 0x7ff) + (0xfff/2));
        #endif
            angle += ANGLE_STEP;
        }
    }
    pit_setup.count_delay = PIT_FREERUN_FREQ(1000 * LENGTH_OF_TEST_BUFFER); // 1kHz sine wave output
        #else
    for (i = 0; i < (LENGTH_OF_TEST_BUFFER/2); i++) {                    // prepare a test pattern
        ptrTestBuffer[i] = (i * (LENGTH_OF_TEST_BUFFER/2));              // ramp up
    }
    ptrTestBuffer[i++] = 0x0fff;                                         // max. 12 bit value
    for (; i < LENGTH_OF_TEST_BUFFER; i++) {
        ptrTestBuffer[i] = ((LENGTH_OF_TEST_BUFFER - i) * (LENGTH_OF_TEST_BUFFER/2)); // ramp down
    }
        #endif
    #elif defined TEST_PIT_SINGLE_SHOT
    pit_setup.int_handler = test_timer_int;                              // test a single shot timer
    pit_setup.count_delay = PIT_US_DELAY(3245);                          // 3245us delay
    pit_setup.mode = PIT_SINGLE_SHOT;                                    // one-shot interrupt
    pit_setup.int_priority = PIT0_INTERRUPT_PRIORITY;
    #elif defined TEST_PIT_PERIODIC && (defined _KINETIS || defined _iMX)// {10}
        #if defined _iMX
    pit_setup.mode = (PIT_PERIODIC  | PIT_TRIGGER_OUT);
        #else
    pit_setup.mode = PIT_PERIODIC;
        #endif
        #if defined TEST_PIT_DMA_GPIO
    pit_setup.int_handler = 0;                                           // no interrupt due to DMA
        #else
    pit_setup.int_handler = test_nmi;                                    // not NMI when used by kinetis reference
        #endif
    pit_setup.int_priority = PIT0_INTERRUPT_PRIORITY;
        #if defined TEST_PIT_SINE_TO_USB
    pit_setup.count_delay = PIT_US_DELAY(43);                            // 43us period (23000 times a second to send 512 bytes - resulting in 11.7MBytes of sample data)
        #else
    pit_setup.count_delay = PIT_US_DELAY(500);                           // 500us period
        #endif
    #elif defined TEST_PIT_PERIODIC                                      // coldfire
    PORTTC |= (PORT_TC_BIT1 | PORT_TC_BIT2);
    DDRTC |= (PORT_TC_BIT1 | PORT_TC_BIT2);
    pit_setup.int_handler = test_nmi;
    pit_setup.int_priority = (INTERRUPT_LEVEL_7 | INTERRUPT_PRIORITY_5); // NMI to give a high resolution 50us interrupt (the interrupt may not use operating system calls!)
    pit_setup.count_delay = PIT_US_DELAY(50);                            // 50us period
    pit_setup.mode = PIT_PERIODIC;
    #endif
    #if defined SUPPORT_PITS                                             // multiple PITs
    pit_setup.ucPIT = 0;                                                 // use PIT0
    #endif
    #if defined TEST_PIT_DMA_GPIO && !defined DEVICE_WITHOUT_DMA
    pit_setup.mode = (PIT_PERIODIC | PIT_OUTPUT_DMA_TRIG);               // periodic with DMA trigger to control a port toggle
        #if defined KINETIS_KL
    pit_setup.ulPortBits = PORTB_BIT18;                                  // toggle PTB18 on each PIT trigger
    pit_setup.ucPortRef = PORTB;
        #elif defined FRDM_KE15Z
    pit_setup.ulPortBits = (PORTD_BIT0 | PORTD_BIT7);                    // toggle PTD0 and PTD7 on each PIT trigger
    pit_setup.ucPortRef = PORTD;
        #elif defined _iMX
    pit_setup.ulPortBits = PORT1_BIT31;                                  // toggle GPIO1_IO31 on each PIT trigger
    pit_setup.ucPortRef = PORT1;
        #else
    pit_setup.ulPortBits = PORTA_BIT1;                                   // toggle PTA1 on each PIT trigger
    pit_setup.ucPortRef = PORTA;
        #endif
    pit_setup.ucPIT = 1;                                                 // use PIT1 (this will use DMA channel 1, 0 would use DMA channel 0, etc.)
    pit_setup.count_delay = PIT_US_DELAY(1);                             // 1us period (for 500kHz square wave)
    #endif
    fnConfigureInterrupt((void *)&pit_setup);                            // configure PIT and start the PIT

    #if (defined _KINETIS || defined _iMX) && defined TEST_PIT_PERIODIC && defined SUPPORT_PITS // multiple PITs
    pit_setup.ucPIT = 1;                                                 // use PIT1
    pit_setup.count_delay = PIT_MS_DELAY(20);                            // 20ms period
    fnConfigureInterrupt((void *)&pit_setup);                            // configure PIT
    #endif
    #if defined TEST_DMA_DAC                                             // {20}
    dac_setup.int_type = DAC_INTERRUPT;
    dac_setup.int_dac_controller = 0;                                    // DAC 0
    dac_setup.int_handler = 0;                                           // no interrupt used
    dac_setup.int_priority = INTERRUPT_LOWEST_PRIORITY;                  // lowest priority (not used in this case)
    dac_setup.dac_mode = (DAC_CONFIGURE | DAC_REF_VDDA | DAC_NON_BUFFERED_MODE | DAC_FULL_BUFFER_DMA | DAC_ENABLE | DAC_BUFFER_DMA_START /* | DAC_OUTPUT_VALUE*/); // configure the DAC to use VDDA as reference voltage in non-buffered mode (using DMA)
        #if defined KINETIS_KL && !defined DEVICE_WITH_eDMA
    dac_setup.dac_mode |= DAC_FULL_BUFFER_DMA_AUTO_REPEAT;
        #endif
    dac_setup.ptrDAC_Buffer = ptrTestBuffer;
    dac_setup.ulDAC_buffer_length = (LENGTH_OF_TEST_BUFFER * sizeof(unsigned short)); // the number of bytes in the buffer
    dac_setup.ucDmaChannel = 0;                                          // DMA channel 0 used
    dac_setup.usDmaTriggerSource = DMAMUX0_DMA0_CHCFG_SOURCE_PIT0;       // PIT0 triggers the channel mux (generally PIT0 can trigger DMA0, PIT1 can trigger DMA channel 1, etc.)
    dac_setup.dac_mode |= DAC_HW_TRIGGER_MODE;                           // use HW trigger mode rather than SW triggered mode (this requires PIT to trigger it)
    fnConfigureInterrupt((void *)&dac_setup);                            // configure DAC
    #endif
}

    #if defined TEST_PIT_SINE_TO_USB
//float buf[1024] = { 0.0 };
// This routine generates a two sine waves in the USB output buffer
// Since the byte rate is 11.7MBytes/s and 4 bytes are used per sample the sine waves represent 92kHz
// The second sine wave is 90� shifted to the first (quadrature) and the ordering of the bytes in the buffer is
// - sine 1 MSB
// - sine 1 LSB
// - sine 2 MSB
// - sine 2 LSB
//
extern void fnPrepareSignal(unsigned char *ptrBuffer, QUEUE_TRANSFER buf_length)
{
        #if !defined PI
            #define PI           (double)3.14159265
        #endif
    int i;
    unsigned short usValue1;
    unsigned short usValue2;
        #if defined USE_CMSIS_SIN_COS
    float fSineValue;
    #define ANGLE_STEP   (float)((double)(2 * PI)/(double)128);          // one sine wave in each 512 bytes in the buffer [4 bytes - 2 for reference sine and 2 for quadrature sine]
    float angle = 0;
        #else
    #define ANGLE_STEP   (double)((double)(2 * PI)/(double)128);
    double angle = 0;
        #endif
    for (i = 0; i < (buf_length/4); i++) {                               // prepare a sine wave, filling the available buffer
        #if defined USE_CMSIS_SIN_COS
        arm_sin_cos_f32(angle, &fSineValue, 0);                          // reference sine wave
        fSineValue *= (float)0x7fff;                                     // 16 bit scaling
        usValue1 = (unsigned short)((signed short)fSineValue + (0xffff/2)); // half-scale DC offset added
        #else
        usValue1 = (unsigned short)((sin(angle) * 0x7fff) + (0xffff/2));
        #endif
        #if defined USE_CMSIS_SIN_COS
        arm_sin_cos_f32((float)(angle + (PI/2)), &fSineValue, 0);               // quadrature sine wave
        fSineValue *= (float)0x7fff;                                     // 16 bit scaling
        usValue2 = (unsigned short)((signed short)fSineValue + (0xffff/2)); // half-scale DC offset added
        #else
        usValue2 = (unsigned short)((sin(angle + (PI / 2)) * 0x7fff) + (0xffff/2));
        #endif
      //buf[i] = (float)usValue2;
        *ptrBuffer++ = (unsigned char)(usValue1 >> 8);                   // pack in the two 16 bit quadrature sample to 4 bytes
        *ptrBuffer++ = (unsigned char)(usValue1);
        *ptrBuffer++ = (unsigned char)(usValue2 >> 8);
        *ptrBuffer++ = (unsigned char)(usValue2);
        angle += ANGLE_STEP;
    }
}
    #endif
#endif

#if defined _ADC_TIMER_ROUTINES && defined TIMED_UART_TX_TEST            // {23}
static void fnConfigureTimedUART(void)
{
    TTYTABLE tInterfaceParameters;                                       // table for passing information to driver
    tInterfaceParameters.Channel = TIMED_UART;                           // set UART channel for serial use
    tInterfaceParameters.ucSpeed = SERIAL_BAUD_19200;                    // baud rate
    tInterfaceParameters.Rx_tx_sizes.RxQueueSize = 0;                    // input buffer not needed since only tx used
    tInterfaceParameters.Rx_tx_sizes.TxQueueSize = 64;                   // output buffer size
    tInterfaceParameters.Task_to_wake = 0;                               // no task to be woken on receptions
    #if defined SUPPORT_FLOW_HIGH_LOW
    tInterfaceParameters.ucFlowHighWater = 80;                           // set the flow control high and low water levels in %
    tInterfaceParameters.ucFlowLowWater = 20;
    #endif
    #if defined USER_DEFINED_UART_RX_HANDLER                             // {27}
    tInterfaceParameters.receptionHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_RX_BREAK_DETECTION
    tInterfaceParameters.receiveBreakHandler = 0;
    #endif
    #if defined USER_DEFINED_UART_TX_FRAME_COMPLETE
    tInterfaceParameters.txFrameCompleteHandler = 0;
    #endif
    tInterfaceParameters.Config = (CHAR_MODE | CHAR_8 | NO_PARITY | ONE_STOP | NO_HANDSHAKE | UART_TIMED_TRANSMISSION_MODE); // the output will be used in timer transmission mode
    #if defined SERIAL_SUPPORT_DMA
    tInterfaceParameters.ucDMAConfig = (UART_TX_DMA);                    // dma is used as base of timer transmissions
        #if defined UART_TIMED_TRANSMISSION
    tInterfaceParameters.usMicroDelay = 2330;                            // 2330us time base between each transmitted character
        #endif
    #endif
    if ((TimedUART_ID = fnOpen(TYPE_TTY, FOR_WRITE, &tInterfaceParameters)) != NO_ID_ALLOCATED) { // open for write only
        fnDriver(TimedUART_ID, (TX_ON), 0);                              // enable tx
    }
    uTaskerMonoTimer(OWN_TASK, (DELAY_LIMIT)(3.0 * SEC), E_TIMER_TX_NOW);// first message transmission
}
#endif


#if defined _ADC_TIMER_ROUTINES && defined RIT_TEST                      // {7}

static void __callback_interrupt test_timer_int(void)
{
    static int iTimerTest = 0;
    RIT_SETUP rit_setup;                                                 // interrupt configuration parameters
    TOGGLE_TEST_OUTPUT();
    switch (iTimerTest++) {
    case 0:
        rit_setup.count_delay = RIT_MS_DELAY(1877);                      // 1.877s delay
        rit_setup.mode = RIT_SINGLE_SHOT;                                // one-shot interrupt
        break;
    case 1:
        rit_setup.count_delay = RIT_S_DELAY(5);                          // 5s periodic
        rit_setup.mode = RIT_PERIODIC;                                   // periodic interrupt
        break;
    case 6:
        iTimerTest = 0;                                                  // reset for next test
        rit_setup.mode = RIT_STOP;                                       // stop timer
        break;
    default:
        return;
    }
    rit_setup.int_type = RIT_INTERRUPT;
    rit_setup.int_handler = test_timer_int;
    rit_setup.int_priority = RIT_INTERRUPT_PRIORITY;
    fnConfigureInterrupt((void *)&rit_setup);                            // enter interrupt for RIT test
}

// RIT configuration
//
static void fnConfigureRIT(void)
{
    RIT_SETUP rit_setup;                                                 // interrupt configuration parameters
    rit_setup.int_type = RIT_INTERRUPT;
    rit_setup.int_handler = test_timer_int;                              // test a single shot timer
    rit_setup.int_priority = RIT_INTERRUPT_PRIORITY;
    rit_setup.count_delay = RIT_US_DELAY(3245);                          // 3245us delay
    rit_setup.mode = RIT_SINGLE_SHOT;                                    // one-shot interrupt
    fnConfigureInterrupt((void *)&rit_setup);                            // enter interrupt for RIT test
}
#endif

#if defined _ADC_TIMER_ROUTINES && defined  TEST_DMA_TIMER
static void __callback_interrupt DMA_timer_int(void)
{
    static int iTimerTest = 0;
    DMA_TIMER_SETUP dma_timer_setup;                                     // interrupt configuration parameters
    TOGGLE_TEST_OUTPUT();
    switch (iTimerTest++) {
    case 0:
        dma_timer_setup.mode = (DMA_TIMER_INTERNAL_CLOCK | DMA_TIMER_SINGLE_SHOT_INTERRUPT); // one-shot interrupt
        dma_timer_setup.count_delay = DMA_TIMER_MS_DELAY(1,1,2345);      // 2.345s delay
        break;
    case 1:
        dma_timer_setup.mode = (DMA_TIMER_INTERNAL_CLOCK | DMA_TIMER_RESTART_ON_MATCH | DMA_TIMER_PERIODIC_INTERRUPT | DMA_TIMER_INTERNAL_CLOCK_DIV_16); // periodic interrupt
        dma_timer_setup.count_delay = DMA_TIMER_S_DELAY(16,1,2);         // 2s periodic
        break;
    case 6:
        iTimerTest = 0;                                                  // reset for next test
        dma_timer_setup.mode = DMA_TIMER_STOP;                           // stop timer
        break;
    default:
        return;
    }
    dma_timer_setup.channel = 1;
    dma_timer_setup.int_type = DMA_TIMER_INTERRUPT;
    dma_timer_setup.int_handler = DMA_timer_int;
    dma_timer_setup.int_priority = DMA_TIMER1_INTERRUPT_PRIORITY;
    fnConfigureInterrupt((void *)&dma_timer_setup);                      // enter interrupt for DMA timer test
    if (iTimerTest == 2) {                                               // on second test
        dma_timer_setup.channel = 2;
        dma_timer_setup.mode = (DMA_TIMER_RESTART_ON_MATCH | DMA_TIMER_TOGGLE_OUTPUT);
        dma_timer_setup.count_delay = DMA_TIMER_US_DELAY(1,1,(1000000/2/1500)); // 1500Hz signal
        fnConfigureInterrupt((void *)&dma_timer_setup);                  // generate a frequency on TOUT2
    }
}

static void fnConfigure_DMA_Timer(void)
{
    DMA_TIMER_SETUP dma_timer_setup;                                     // interrupt configuration parameters
    dma_timer_setup.int_type = DMA_TIMER_INTERRUPT;
    dma_timer_setup.int_handler = DMA_timer_int;
    dma_timer_setup.channel = 1;                                         // DMA timer channel 1
    dma_timer_setup.int_priority = DMA_TIMER1_INTERRUPT_PRIORITY;        // define interrupt priority
    dma_timer_setup.mode = (DMA_TIMER_INTERNAL_CLOCK | DMA_TIMER_SINGLE_SHOT_INTERRUPT);
    dma_timer_setup.count_delay = DMA_TIMER_US_DELAY(1,1,6345);          // 6345us delay using no dividers
    fnConfigureInterrupt((void *)&dma_timer_setup);                      // enter interrupt for DMA timer test
}
#endif




#if defined _ADC_TIMER_ROUTINES && defined  TEST_GPT
// Interrupt call back from general purpose timer test
//
static void __callback_interrupt gptimer_int(void)
{
    fnInterruptMessage(OWN_TASK, CAPTURE_COMPLETE_EVENT);
}

static void fnConfigure_GPT(void)
{
    GPTIMER_SETUP gptimer_setup;                                         // interrupt configuration parameters
    gptimer_setup.int_type = GPT_TIMER_INTERRUPT;
    gptimer_setup.int_handler = gptimer_int;
    gptimer_setup.channel = 3;                                           // general purpose timer channel 3
    gptimer_setup.int_priority = GPTIMER0_INTERRUPT_PRIORITY;            // define interrupt priority
    gptimer_setup.mode = (GPT_CAPTURE_FALLING_EDGE | ENABLE_INPUT_PULLUP | GPT_INTERNAL_CLOCK | GPT_PRESCALE_16); // set up capture mode and define the timer clock
                                                                         // (in this example system clock / 2 / 16 = 1.875MHz with 60MHz: GTP timer overflows at 34.95ms but longer periods can be recorded using the extended option GPT_EXTENDED_COUNTER)
    gptimer_setup.usCaptureCount = GPT_CAPTURES;                         // request this many capture values to be recorded before calling our interrupt
    gptimer_setup.capture_list = ulCaptureList;                          // the capture list for saving to
    fnConfigureInterrupt((void *)&gptimer_setup);                        // enter interrupt for DMA timer test
}
#endif



#if defined _ADC_TIMER_ROUTINES && defined TEST_TIMER
    #if !((defined _KINETIS || defined _M5223X || defined _iMX) && (defined TEST_PWM && !defined TEST_PERIODIC_TIMER))
        #if defined _KINETIS && defined SUPPORT_CAPTURE && defined CAPTURE_CHANNEL_USER_DISPATCHER && !defined CAPTURE_CHANNEL_CALLBACK
static __callback_interrupt void timer_int(unsigned char ucCaptureChannels) // flags indicate which capture channel(s) caused the event
        #else
static __callback_interrupt void timer_int(void)
        #endif
{
        #if defined TEST_CAPTURE && (defined _KINETIS || defined _iMX)   // {24}
    static volatile unsigned long ulLastCapture = 0;
    ulLastCapture = CAPTURE_VALUE(0, 1);                                 // update the last capture value
        #endif
    TOGGLE_TEST_OUTPUT();
        #if defined TEST_SINGLE_SHOT_TIMER
    fnConfigure_Timer();
        #endif
}
    #endif

    #if defined PWM_LED_CONTROL && defined TEST_PWM
extern void fnSetColor(signed char x, signed char y)                     // {19}
{
        #if defined FRDM_KL02Z || defined FRDM_KL25Z || defined FRDM_KL26Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27 || defined FRDM_KE02Z40M
            #if defined FRDM_KL02Z
    #define PWM_CHANNEL_RED   1
    #define PWM_CHANNEL_GREEN 0
    #define PWM_CHANNEL_BLUE  1
    FLEX_TIMER_MODULE *ptrFlexTimerRed = (FLEX_TIMER_MODULE *)FTM_BLOCK_1;
    FLEX_TIMER_MODULE *ptrFlexTimerGreen = (FLEX_TIMER_MODULE *)FTM_BLOCK_1;
    FLEX_TIMER_MODULE *ptrFlexTimerBlue = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;
            #elif defined FRDM_KE02Z40M
    #define PWM_CHANNEL_RED   1
    #define PWM_CHANNEL_GREEN 0
    #define PWM_CHANNEL_BLUE  1
    FLEX_TIMER_MODULE *ptrFlexTimerRed = (FLEX_TIMER_MODULE *)FTM_BLOCK_2;
    FLEX_TIMER_MODULE *ptrFlexTimerGreen = (FLEX_TIMER_MODULE *)FTM_BLOCK_2;
    FLEX_TIMER_MODULE *ptrFlexTimerBlue = (FLEX_TIMER_MODULE *)FTM_BLOCK_1;
            #elif defined FRDM_KL25Z
    #define PWM_CHANNEL_RED   0
    #define PWM_CHANNEL_GREEN 1
    #define PWM_CHANNEL_BLUE  1
    FLEX_TIMER_MODULE *ptrFlexTimerRed = (FLEX_TIMER_MODULE *)FTM_BLOCK_2;
    FLEX_TIMER_MODULE *ptrFlexTimerGreen = (FLEX_TIMER_MODULE *)FTM_BLOCK_2;
    FLEX_TIMER_MODULE *ptrFlexTimerBlue = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;
            #elif defined FRDM_KL26Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27
    #define PWM_CHANNEL_RED   2
    #define PWM_CHANNEL_GREEN 4
    #define PWM_CHANNEL_BLUE  5
    FLEX_TIMER_MODULE *ptrFlexTimerRed = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;
    FLEX_TIMER_MODULE *ptrFlexTimerGreen = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;
    FLEX_TIMER_MODULE *ptrFlexTimerBlue = (FLEX_TIMER_MODULE *)FTM_BLOCK_0;
            #endif
    unsigned short usRed;
    unsigned short usGreen;
    unsigned short usBlue;
    unsigned short usDeviation;

    usBlue = (unsigned short)(ptrFlexTimerBlue->FTM_MOD/2);              // 50% PWM levels
    usRed = (unsigned short)(ptrFlexTimerRed->FTM_MOD/2);
    usGreen = (unsigned short)(ptrFlexTimerGreen->FTM_MOD/2);

    if (y < -64) {
        y = -64;
    }
    else if (y > 64) {
        y = 64;
    }
    if (x < -64) {
        x = -64;
    }
    else if (x > 64) {
        x = 64;
    }

    usDeviation = ((y * usBlue)/64);
    if (y >= 0) {
        usBlue += usDeviation;
    }
    else {
        usBlue -= usDeviation;
    }

    usDeviation = ((x * usRed)/64);
    if (x >= 0) {
        usRed -= usDeviation;
        usGreen += usDeviation;
    }
    else {
        usRed += usDeviation;
        usGreen -= usDeviation;
    }

    ptrFlexTimerRed->FTM_channel[PWM_CHANNEL_RED].FTM_CV = usRed;
    ptrFlexTimerGreen->FTM_channel[PWM_CHANNEL_GREEN].FTM_CV = usGreen;
    ptrFlexTimerBlue->FTM_channel[PWM_CHANNEL_BLUE].FTM_CV = usBlue;
        #endif
}
    #endif


#if defined TEST_STEPPER
static const unsigned short rampFrequencyValue[] = {
    PWM_FREQUENCY(4800, 16),                                             // 4800Hz
    PWM_FREQUENCY(4200, 16),                                             // 4200Hz
    PWM_FREQUENCY(3200, 16),                                             // 3200Hz
    PWM_FREQUENCY(1000, 16),                                             // 1000Hz
    PWM_FREQUENCY(100, 16),                                              // 100Hz
    PWM_FREQUENCY(800, 16),                                              // 800Hz
    PWM_FREQUENCY(1800, 16),                                             // 1800Hz
    PWM_FREQUENCY(20000, 16),                                            // 20kHz
};

#if defined FLEX_TIMERS_ONLY
static const unsigned short rampCountValue[] = {                         // accumulated
    10,                                                                  // 10 pulses at first frequency
    18,                                                                  // 8 pulses at next frequency
    22,
    24,
    25,
    27,
    33,
    53,
    0,                                                                   // end (dummy due to buffer delay)
};
#else
static const unsigned short rampCountValue[] = {
    (10 - 1),                                                            // 10 pulses at first frequency
    (8 - 1),                                                             // 8 pulses at next frequency
    (4 - 1),
    (2 - 1), 
    (1 - 1),
    (2 - 1),
    (6 - 1),
    (20 - 1), 
    0,                                                                   // end (dummy due to buffer delay)
};
#endif


static void __callback_interrupt fnEndOfRamp(void)
{
    FTM0_SC = 0;                                                         // stop the PWM output when the sequence has completed
}
#endif  

//static void __callback_interrupt PWM_IRQ(void)
//{
//    TOGGLE_TEST_OUTPUT();
//}

#if defined MULTIPLE_CHANNEL_INTERRUPTS                                  // {30}
static void __callback_interrupt _timer0_irq(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle on timer 0 period interrupt
}

static void __callback_interrupt _channel0_0_irq(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle on timer 0 channel 0 match interrupt
}

static void __callback_interrupt _channel0_1_irq(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle on timer 0 channel 1 match interrupt
}


static void __callback_interrupt _timer1_irq(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle on timer 1 period interrupt
}

static void __callback_interrupt _channel1_0_irq(void)
{
    TOGGLE_TEST_OUTPUT();                                                // toggle on timer 1 channel 0 match interrupt
}
#endif



static void fnConfigure_Timer(void)
{
#if (defined _KINETIS || defined _M5223X || defined _iMX) && defined TEST_PWM // {9} Kinetis, iMX and Coldfire PWM
    #if (defined _KINETIS || defined _iMX) && defined TEST_PERIODIC_TIMER// allow periodic timer together with PWM
    static TIMER_INTERRUPT_SETUP timer_setup = {0};                      // interrupt configuration parameters
    #endif
    PWM_INTERRUPT_SETUP pwm_setup;
    pwm_setup.int_type = PWM_INTERRUPT;
    pwm_setup.int_handler = 0;
    #if defined PHASE_SHIFTED_COMBINED_OUTPUTS                           // {29}
    // Set up two complimentary output pairs which are phase shifted from each other
    //
    pwm_setup.pwm_frequency = PWM_FREQUENCY(1000, 16);                   // generate 1000Hz on PWM output
    pwm_setup.pwm_value = _PWM_PERCENT(50, pwm_setup.pwm_frequency);     // 50% PWM (high/low)
    pwm_setup.pwm_mode = (PWM_EDGE_ALIGNED | PWM_COMBINED_PHASE_SHIFT);  // phase shifted mode (only possible in edge aligned mode)
    pwm_setup.pwm_phase_shift = _PWM_PERCENT(0, pwm_setup.pwm_frequency);// 0� phase shift
    pwm_setup.pwm_reference = (_TIMER_0 | 0);                            // timer module 0, channel 0
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for PWM test

    pwm_setup.pwm_mode |= (PWM_POLARITY);                                // complimentary output
    pwm_setup.pwm_reference = (_TIMER_0 | 1);                            // timer module 0, channel 1
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for PWM test

    pwm_setup.pwm_mode &= ~(PWM_POLARITY);
    pwm_setup.pwm_phase_shift = _PWM_PERCENT(25, pwm_setup.pwm_frequency); // 90� phase shift
    pwm_setup.pwm_reference = (_TIMER_0 | 2);                            // timer module 0, channel 2
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for PWM test

    pwm_setup.pwm_mode |= (PWM_POLARITY | PWM_SYS_CLK | PWM_PRESCALER_128);// clock PWM timer from the system clock with /128 pre-scaler [clock is enabled here so that all prepared values are synchronised]
    pwm_setup.pwm_reference = (_TIMER_0 | 3);                            // timer module 0, channel 3
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for PWM test
    #elif defined MULTIPLE_CHANNEL_INTERRUPTS                            // {30}
    pwm_setup.pwm_reference = (_TIMER_0 | 0);                            // timer module 0, channel 0
    pwm_setup.pwm_mode = (PWM_PRESCALER_128 | PWM_EDGE_ALIGNED | PWM_POLARITY | PWM_CHANNEL_INTERRUPT); // clock PWM timer from the system clock with /16 pre-scaler (don't configure the clock until all channels are set up)
    pwm_setup.pwm_frequency = PWM_FREQUENCY(1000, 16);                   // generate 1000Hz on PWM output
    pwm_setup.pwm_value = _PWM_PERCENT(50, pwm_setup.pwm_frequency);     // 50%
    pwm_setup.int_priority = PRIORITY_HW_TIMER;                          // interrupt priority
    pwm_setup.channel_int_handler = _channel0_0_irq;                     // interrupt called on channel 0 match
    fnConfigureInterrupt((void *)&pwm_setup);                            // configure channel 0
    pwm_setup.pwm_reference = (_TIMER_0 | 1);                            // timer module 0, channel 1
    pwm_setup.pwm_value = _PWM_PERCENT(60, pwm_setup.pwm_frequency);     // 60%
    pwm_setup.channel_int_handler = _channel0_1_irq;                     // interrupt called on channel 1 match
    pwm_setup.pwm_mode |= (PWM_SYS_CLK);                                 // start
    pwm_setup.int_handler = _timer0_irq;                                 // interrupt called on each cycle
    fnConfigureInterrupt((void *)&pwm_setup);                            // configure channel 1 and start

    pwm_setup.pwm_reference = (_TIMER_1 | 0);                            // timer module 1, channel 0
    pwm_setup.pwm_value = _PWM_PERCENT(70, pwm_setup.pwm_frequency);     // 70%
    pwm_setup.int_handler = _timer1_irq;                                 // interrupt called on each cycle
    pwm_setup.channel_int_handler = _channel1_0_irq;                     // interrupt called on channel 0 match
    fnConfigureInterrupt((void *)&pwm_setup);                            // configure channel 0 and start
    #else
    pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_16 | PWM_EDGE_ALIGNED); // clock PWM timer from the system clock with /16 pre-scaler
    pwm_setup.int_handler = 0;                                           // {22} no user interrupt call-back on PWM cycle
  //pwm_setup.int_handler = PWM_IRQ;                                     // enable to generate an interrupt on each PWM cycle
    #if defined FRDM_KL02Z || defined FRDM_KL03Z || defined FRDM_KE02Z || defined FRDM_KE04Z || defined FRDM_KE06Z || defined FRDM_K22F
    pwm_setup.pwm_reference = (_TIMER_0 | 1);                            // timer module 0, channel 1
    #elif defined FRDM_KL05Z
    pwm_setup.pwm_reference = (_TIMER_0 | 0);                            // timer module 0, channel 0
    #elif defined FRDM_KE02Z40M
    pwm_setup.pwm_reference = (_TIMER_2 | 1);                            // timer module 2, channel 1 (red LED in RGB LED)
    #elif defined FRDM_KL25Z || defined FRDM_KEAZN32Q64 || defined TRK_KEA64
    pwm_setup.pwm_reference = (_TIMER_2 | 0);                            // timer module 2, channel 0 (red LED in RGB LED)
    #elif defined FRDM_KL26Z || defined FRDM_KL27Z || defined CAPUCCINO_KL27 || defined FRDM_KE15Z
    pwm_setup.pwm_reference = (_TIMER_0 | 2);                            // timer module 0, channel 2 (red LED in RGB LED)
    #elif defined MIMXRT1050
    pwm_setup.pwm_reference = (_FLEX_PWM_4 | _FLEX_PWM_A_3);             // flexPWM module 4 (counts 1..4), channel 3, A output (equivalent to (_TIMER_3 | 3))
    #else
    pwm_setup.pwm_reference = (_TIMER_0 | 3);                            // timer module 0, channel 3
    #endif
    #if defined TEST_STEPPER
    // Configure DMA trigger generator
    //
    #if defined FLEX_TIMERS_ONLY
    pwm_setup.pwm_mode = (PWM_EXTERNAL_CLK | PWM_PRESCALER_0 | PWM_NO_OUTPUT | PWM_FULL_BUFFER_DMA | PWM_DMA_CHANNEL_ENABLE | PWM_DMA_CONTROL_PWM); // clock PWM timer from external clock (use channel DMA)
    #else
    pwm_setup.pwm_mode = (PWM_EXTERNAL_CLK | PWM_PRESCALER_0 | PWM_NO_OUTPUT | PWM_FULL_BUFFER_DMA | PWM_DMA_PERIOD_ENABLE | PWM_DMA_CONTROL_FREQUENCY); // clock PWM timer from external clock (use overflow DMA)
    #endif
    #if defined FRDM_K66F
    pwm_setup.pwm_reference = (_TPM_TIMER_2 | 0);                        // TPM module 2, channel 0
    #else
    pwm_setup.pwm_reference = (_TIMER_1 | 0);                            // timer module 1, channel 0
    #endif
    #if defined FLEX_TIMERS_ONLY
    pwm_setup.pwm_frequency = 0xffff;                                    // cause maximum timer count to be used
    #else
    pwm_setup.pwm_frequency = rampCountValue[0];                         // set the input pulse count value until overflow
    #endif
    pwm_setup.pwm_value = rampCountValue[0];                             // first match value
    pwm_setup.ucDmaChannel = 2;                                          // use DMA channel 2
    pwm_setup.dma_int_priority = 0;
    #if defined FLEX_TIMERS_ONLY
    pwm_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_FTM1_C0;         // load next value on own timer match
    #elif defined FRDM_K66F
    pwm_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_TPM2_OVERFLOW;   // load next value on own timer overflow
    #else
    pwm_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_TPM1_OVERFLOW;   // load next value on own timer overflow
    #endif
    #if defined FLEX_TIMERS_ONLY
    pwm_setup.ptrPWM_Buffer = (unsigned short *)&rampCountValue[1];      // buffer controlling the DMA trigger points
    #else
    pwm_setup.ptrPWM_Buffer = (unsigned short *)&rampCountValue[2];      // buffer controlling the DMA trigger points
    #endif
    pwm_setup.ulPWM_buffer_length = (sizeof(rampCountValue) - sizeof(unsigned short));
    pwm_setup.dma_int_handler = fnEndOfRamp;                             // call back on DMA termination
    #if !defined TEST_SPWM
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for DMA trigger generation
    #if !defined FLEX_TIMERS_ONLY
        #if defined FRDM_K66F
    FTM5_MOD = rampCountValue[1];                                        // prepare second value (buffered)
        #else
    FTM1_MOD = rampCountValue[1];                                        // prepare second value (buffered)
        #endif
    #endif
    #endif
    // Configure PWM output signal (this needs to be fed back to the clock input for stepper motor control)
    //
    #if defined FLEX_TIMERS_ONLY
    pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_16 | PWM_FULL_BUFFER_DMA | /*PWM_DMA_CHANNEL_ENABLE |*/ PWM_DMA_CONTROL_FREQUENCY);
    #else
    pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_16 | PWM_FULL_BUFFER_DMA | /*PWM_DMA_PERIOD_ENABLE |*/ PWM_DMA_CONTROL_FREQUENCY);
    #endif
    pwm_setup.pwm_reference = (_TIMER_0 | 2);                            // timer module 0, channel 2 (red LED in RGB LED)
    pwm_setup.ucDmaChannel = 1;                                          // use DMA channel 1
    #if defined TEST_SPWM
    pwm_setup.pwm_frequency = 0xffff;                                    // set base frequency
    pwm_setup.pwm_value = PWMTestBuffer[0];                              // initial PWM value
    pwm_setup.ptrPWM_Buffer = (unsigned short *)PWMTestBuffer;           // buffer controlling the PWM values
    pwm_setup.ulPWM_buffer_length = sizeof(PWMTestBuffer);
    pwm_setup.usDmaTriggerSource = DMAMUX0_CHCFG_SOURCE_FTM0_C2;
    pwm_setup.pwm_mode = (PWM_SYS_CLK | PWM_PRESCALER_1 | PWM_FULL_BUFFER_DMA | PWM_DMA_CHANNEL_ENABLE | PWM_DMA_CONTROL_PWM | PWM_FULL_BUFFER_DMA_AUTO_REPEAT);
    #else
    pwm_setup.pwm_frequency = rampFrequencyValue[0];                     // set start frequency
    pwm_setup.pwm_value = _PWM_PERCENT(50, PWM_FREQUENCY(20000, 16));    // 50% PWM (high/low) when the frequency is 20kHz
    pwm_setup.ptrPWM_Buffer = (unsigned short *)&rampFrequencyValue[1];  // buffer controlling the frequencies
    pwm_setup.ulPWM_buffer_length = (sizeof(rampFrequencyValue) - sizeof(unsigned short));
    #endif
    pwm_setup.dma_int_handler = 0;                                       // no callback on DMA buffer transfer termination
    fnConfigureInterrupt((void *)&pwm_setup);                            // configure and start the PWM output
    return;
    #else
        #if defined FTM_FLEXIBLE_CLOCKING
    pwm_setup.pwm_mode = (PWM_FIXED_CLK | PWM_PRESCALER_1 | PWM_EDGE_ALIGNED);
    pwm_setup.pwm_frequency = PWM_FIXED_CLOCK_FREQUENCY(1000, 1);        // generate 1000Hz on PWM output
        #else
    pwm_setup.pwm_frequency = PWM_FREQUENCY(1000, 16);                   // generate 1000Hz on PWM output
        #endif
    pwm_setup.pwm_value   = _PWM_PERCENT(20, pwm_setup.pwm_frequency);   // 20% PWM (high/low)
    fnConfigureInterrupt((void *)&pwm_setup);                            // enter configuration for PWM test
    #endif
    #if defined FRDM_KL02Z || defined FRDM_KE02Z40M
    pwm_setup.pwm_reference = (_TIMER_1 | 0);                            // timer module 1, channel 0
    #elif defined FRDM_KL03Z || defined FRDM_KEAZN32Q64 || defined TRK_KEA64
    pwm_setup.pwm_reference = (_TIMER_1 | 1);                            // timer module 1, channel 1
    #elif defined FRDM_KL05Z
    pwm_setup.pwm_reference = (_TIMER_0 | 1);                            // timer module 0, channel 1
    pwm_setup.pwm_mode |= PWM_POLARITY;                                  // change polarity of second channel
    #elif defined FRDM_KL25Z || defined FRDM_KE02Z || defined FRDM_KE04Z || defined FRDM_KE06Z
    pwm_setup.pwm_reference = (_TIMER_2 | 1);                            // timer module 2, channel 1 (green LED in RGB LED)
    #elif defined FRDM_KL26Z || defined CAPUCCINO_KL27 || defined FRDM_KE15Z
    pwm_setup.pwm_reference = (_TIMER_0 | 4);                            // timer module 0, channel 4 (green LED in RGB LED)
    #elif defined FRDM_KL27Z
    pwm_setup.pwm_frequency = PWM_FREQUENCY(1000000, 16);                 // generate 1MHz on PWM output
    pwm_setup.pwm_reference = (_TIMER_2 | 0);
    pwm_setup.pwm_mode |= (PWM_OPTION_MODULATE_LPUART0);                  // modulate LPUART0 output with the PWM signal
    #elif defined FRDM_KL28Z
    pwm_setup.pwm_mode = (PWM_TRIGGER_CLK | PWM_PRESCALER_1 | PWM_EDGE_ALIGNED); // clock from trigger source
    pwm_setup.pwm_reference = (_TIMER_1 | 1);                            // timer module 1, channel 1
    pwm_setup.ucTriggerSource = TRGMUX_SEL_TPM0_OVERFLOW;                // use TPM0's time base as clock to TPM1
    #else
    pwm_setup.pwm_reference = (_TIMER_0 | 2);                            // timer module 0, channel 2
    pwm_setup.pwm_mode |= PWM_POLARITY;                                  // change polarity of second channel
    #endif
    #if defined FRDM_KL28Z
    pwm_setup.pwm_frequency = 64;                                        // PWM1 period is defined by 64 PWM0 periods
    pwm_setup.pwm_value = 1;                                             // mark is defined by a single PWM0 clock cycle
    #else
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(706, pwm_setup.pwm_frequency); // 70.6% PWM (low/high) on different channel
    #endif
    fnConfigureInterrupt((void *)&pwm_setup);
    #if defined FRDM_K64F                                                // generate 6 different PWM signals on flex timer 0
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(553, pwm_setup.pwm_frequency); // 55.3% PWM (low/high) on different channel
    pwm_setup.pwm_reference = (_TIMER_0 | 1);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(249, pwm_setup.pwm_frequency); // 24.9% PWM (low/high) on different channel
    pwm_setup.pwm_reference = (_TIMER_0 | 0);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(129, pwm_setup.pwm_frequency); // 12.9% PWM (low/high) on different channel
    pwm_setup.pwm_reference = (_TIMER_0 | 4);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(20, pwm_setup.pwm_frequency); // 2.0% PWM (low/high) on different channel
    pwm_setup.pwm_reference = (_TIMER_0 | 5);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_mode = (PWM_FIXED_CLK | PWM_PRESCALER_1 | PWM_EDGE_ALIGNED); // clock from fixed clock (MCGFFCLK)
    pwm_setup.pwm_reference = (_TIMER_2 | 0);                            // timer module 2, channel 1
    pwm_setup.pwm_frequency = PWM_FIXED_CLOCK_FREQUENCY(1, 1);           // generate 1Hz on PWM output
    pwm_setup.pwm_value = _PWM_TENTH_PERCENT(200, pwm_setup.pwm_frequency); // 20.0% PWM (low/high)
    fnConfigureInterrupt((void *)&pwm_setup);
    #elif defined FRDM_KL02Z || defined FRDM_KE02Z40M
    pwm_setup.pwm_reference = (_TIMER_1 | 1);                            // timer module 1, channel 1 (red LED for FRDM-KL02Z and blue for FRDM-KE02Z40M)
    fnConfigureInterrupt((void *)&pwm_setup);
    #elif defined FRDM_KL25Z
    pwm_setup.pwm_reference = (_TIMER_0 | 1);                            // timer module 0, channel 1 (blue LED in RGB LED)
    fnConfigureInterrupt((void *)&pwm_setup);
    #elif defined FRDM_KL27Z_
    pwm_setup.pwm_mode = (PWM_EXTERNAL_CLK | PWM_PRESCALER_1 | PWM_EDGE_ALIGNED); // clock from TPM_CLKIN0
  //pwm_setup.pwm_mode = (PWM_EXTERNAL_CLK_1 | PWM_PRESCALER_1 | PWM_EDGE_ALIGNED); // clock from TPM_CLKIN1
    pwm_setup.pwm_reference = (_TIMER_2 | 1);                            // timer module 2, channel 1
    pwm_setup.pwm_frequency = 64;                                        // PWM2 period is defined by 64 PWM0 periods when the PWM0 output is connected to the clock input
    pwm_setup.pwm_value = 1;                                             // mark is defined by a single PWM0 clock cycle
    fnConfigureInterrupt((void *)&pwm_setup);
    #elif defined FRDM_KL26Z || defined CAPUCCINO_KL27
    pwm_setup.pwm_reference = (_TIMER_0 | 5);                            // timer module 0, channel 5 (blue LED in RGB LED)
    fnConfigureInterrupt((void *)&pwm_setup);
    #elif defined TWR_KE18F
    pwm_setup.pwm_reference = (_TIMER_0 | 4);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_0 | 5);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_0 | 6);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_1 | 5);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_1 | 6);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_1 | 7);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_2 | 4);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_2 | 5);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_3 | 4);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_3 | 6);
    fnConfigureInterrupt((void *)&pwm_setup);
    pwm_setup.pwm_reference = (_TIMER_3 | 7);
    fnConfigureInterrupt((void *)&pwm_setup);
    #endif
        #if (defined _KINETIS || defined _iMX) && defined TEST_PERIODIC_TIMER// allow periodic timer together with PWM
    timer_setup.int_type = TIMER_INTERRUPT;
    timer_setup.int_priority = PRIORITY_TIMERS;
    timer_setup.int_handler = timer_int;
    timer_setup.timer_reference = 2;                                     // FlexTimer/TPM 2
  //timer_setup.timer_mode = (TIMER_PERIODIC | TIMER_EXT_CLK_1);         // period timer interrupt using external clocksource 1
    timer_setup.timer_mode = (TIMER_PERIODIC);                           // period timer interrupt
    timer_setup.timer_value = TIMER_MS_DELAY(100);                       // 100ms periodic interrupt
    fnConfigureInterrupt((void *)&timer_setup);                          // enter interrupt for timer test
        #endif
    #endif
#else
    static TIMER_INTERRUPT_SETUP timer_setup = {0};                      // interrupt configuration parameters
    timer_setup.int_type = TIMER_INTERRUPT;
    timer_setup.int_priority = PRIORITY_TIMERS;                          // interrupt priority
    timer_setup.int_handler = timer_int;                                 // user interrupt callback
    timer_setup.timer_reference = 2;                                     // timer 2
    #if defined _STM32 && defined TEST_PWM
    timer_setup.timer_reference = 3;                                     // timer 3
    timer_setup.timer_mode = (TIMER_FREQUENCY | TIMER_PWM_CH1);          // PWM output on timer 3 channel 1 (50% MSR)
    timer_setup.prescaler_value = 1;
    timer_setup.timer_value = TIMER_nS_DELAY(1333);                      // 750kHz
    timer_setup.int_handler = 0;                                         // no interrupt
    #elif defined _LM3SXXXX
        #if defined TEST_SINGLE_SHOT_TIMER
    timer_setup.timer_mode = (TIMER_SINGLE_SHOT | TIMER_US_VALUE/* | TIMER_16BIT_CHANNEL_B*/); // single shot timer
    timer_setup.timer_value += 100;                                      // each subsequent delay increased by 100us
        #elif defined TEST_ADC_TIMER
    timer_setup.timer_mode = (TIMER_TRIGGER_ADC | TIMER_PERIODIC | TIMER_MS_VALUE); // period timer
    timer_setup.int_handler = 0;                                         // no interrupts - used together with ADC triggering
    timer_setup.timer_value += 100;                                      // each subsequent delay increased by 100ms
        #elif defined TEST_PERIODIC_TIMER
    timer_setup.timer_mode = (TIMER_PERIODIC | TIMER_MS_VALUE);          // period timer interrupt
    timer_setup.timer_value += 100;                                      // each subsequent delay increased by 100us
        #elif defined TEST_PWM                                           // {1}
            #if defined SUPPORT_PWM_CONTROLLER                           // {8}
    timer_setup.int_type    = PWM_CONFIGURATION;
    timer_setup.timer_mode  = PWM_DIV_1;                                 // don't start yet
    timer_setup.timer_value = PWM_FREQUENCY_VALUE(1000, 1);              // generate 1000Hz on timer output using PWM clock without divide
    timer_setup.pwm_value   = _PWM_PERCENT(20, PWM_FREQUENCY_VALUE(1000, 1)); // 20% PWM (high/low)
    fnConfigureInterrupt((void *)&timer_setup);                          // enter configuration for PWM test
    timer_setup.timer_reference = 5;
    timer_setup.timer_mode  = (TIMER_PWM_START_2 | TIMER_PWM_START_5 | PWM_DIV_1); // generate PWM signal on these outputs
    timer_setup.timer_value = PWM_FREQUENCY_VALUE(1500, 1);
    timer_setup.pwm_value   = _PWM_TENTH_PERCENT(706, PWM_FREQUENCY_VALUE(1500, 1)); // 70.6% PWM (high/low) on different channel
            #else
    timer_setup.timer_reference = 0;
    timer_setup.timer_mode  = (TIMER_PWM_B);                             // generate PWM signal on timer output port
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000));// generate 1000Hz on timer output
    timer_setup.pwm_value   = _PWM_PERCENT(20, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000)));// {3} 20% PWM (high/low)
    timer_setup.int_handler = 0;                                         // no interrupts
    fnConfigureInterrupt((void *)&timer_setup);                          // enter interrupt for timer test
    timer_setup.timer_mode  = (TIMER_PWM_A | TIMER_DONT_DISTURB);        // now set output A but don't disturb (reset) output B
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500));// generate 1500Hz on timer output
    timer_setup.pwm_value   = _PWM_TENTH_PERCENT(352, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500))); // 35.2% PWM (high/low)

//  example of stopping PWM outputs (first one and then complete timer module power down)
//  fnConfigureInterrupt((void *)&timer_setup);                          // enter interrupt for timer test
//  timer_setup.timer_mode  = (TIMER_STOP_PWM_B | TIMER_DONT_DISTURB);   // stop B but don't disturb A
//  fnConfigureInterrupt((void *)&timer_setup);                          // enter interrupt for timer test
//  timer_setup.timer_mode  = (TIMER_STOP_PWM_A);                        // stop A and power down timer module
            #endif
        #endif
    #elif defined _LPC23XX && defined TEST_PWM                           // {3}
    timer_setup.timer_mode  = (TIMER_PWM_MAT0);                          // generate PWM signal on MAT 2.0 output
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000)); // generate 1000Hz on timer output
    timer_setup.pwm_value   = _PWM_PERCENT(20, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000))); // 20% PWM (high/low)
    fnConfigureInterrupt((void *)&timer_setup);                          // enter configuration for PWM test
    timer_setup.timer_mode  = (TIMER_PWM_MAT1 | TIMER_DONT_DISTURB);     // generate PWM signal on MAT 2.1 output without disturbing present signal
    timer_setup.pwm_value   = _PWM_TENTH_PERCENT(706, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000))); // 70.6% PWM (high/low) using same timer but different output
    fnConfigureInterrupt((void *)&timer_setup);                          // enter configuration for PWM test
    timer_setup.timer_reference = 1;
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500));// generate 1500Hz on timer output
    timer_setup.pwm_value   = _PWM_PERCENT(50, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500))); // 50% PWM (high/low)
    #elif defined _KINETIS || defined _iMX                               // {16} Kinetis FlexTimer / i.MX RT GPT
    timer_setup.timer_reference = 0;                                     // FlexTimer/TPM 0/GPT 1 (GPTs count from 1)
        #if defined TEST_PERIODIC_TIMER
            #if defined FTM_CLOCKED_FROM_MCGFFLCLK
    timer_setup.timer_mode = (TIMER_PERIODIC | TIMER_FIXED_CLK);         // period timer interrupt clocked from fixed source
    timer_setup.timer_value = TIMER_FIXED_CLOCK_MS_DELAY(6000);          // 6s periodic interrupt
            #else
  //timer_setup.timer_mode = (TIMER_PERIODIC | TIMER_EXT_CLK_1);         // period timer interrupt using external clock source 1
                #if defined _iMX
    timer_setup.timer_mode = (TIMER_PERIODIC | TIMER_USES_PRESCALER | TIMER_SQUARE_WAVE)); // period timer interrupt with square wave output too (requires SUPPORT_TIMER_SQUARE_WAVE)
    timer_setup.prescaler = 1;                                           // 1..4096 or 1..16 for 24MHz clock source (if TIMER_USES_PRESCALER is not set a prescaler value of 1 is defaulted to)
    timer_setup.timer_value = TIMER_MS_PRESCALED_DELAY(150, 1);          // 150ms periodic interrupt
                #else
    timer_setup.timer_mode = (TIMER_PERIODIC/* | TIMER_SQUARE_WAVE*/);   // period timer interrupt with square wave output too (requires SUPPORT_TIMER_SQUARE_WAVE)
    timer_setup.timer_value = TIMER_MS_DELAY(150);                       // 150ms periodic interrupt
                #endif
            #endif
        #elif defined TEST_CAPTURE                                       // {24}
    timer_setup.capture_channel = 1;                                     // channel 1
            #if defined _KINETIS
    timer_setup.capture_prescaler = 128;                                 // 1, 2, ..128 possible
            #endif
    timer_setup.timer_mode = (TIMER_CAPTURE_FALLING);                    // capture interrupt on falling edge
    fnConfigureInterrupt((void *)&timer_setup);                          // configure timer interrupt for timer test
    timer_setup.capture_channel = 0;                                     // channel 0
        #else                                                            // single-shot
    timer_setup.timer_value += TIMER_US_DELAY(100);                      // each subsequent delay increased by 100us
        #endif
    #elif (defined _LPC23XX && defined _HW_TIMER_MODE) || defined _LPC17XX || defined _STM32
        #if defined TEST_PERIODIC_TIMER
    timer_setup.timer_mode = (TIMER_PERIODIC);                           // period timer interrupt
    timer_setup.timer_value = TIMER_MS_DELAY(1000);                      // 1s periodic interrupt
        #else
    timer_setup.timer_value += TIMER_US_DELAY(100);                      // each subsequent delay increased by 100us
        #endif
        #if defined _LPC214X
    timer_setup.timer_reference = 1;                                     // timer channel 1 since timers are limited
        #endif
    #elif defined _HW_SAM7X && defined TEST_PWM                          // {5}
    timer_setup.int_type    = PWM_CONFIGURATION;
    timer_setup.timer_mode  = (TIMER_PWM_ALT);                           // configure PWM signal on alternative PWM2 output
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000));// generate 1000Hz on timer output
    timer_setup.pwm_value   = _PWM_PERCENT(20, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1000))); // 20% PWM (high/low)
    fnConfigureInterrupt((void *)&timer_setup);                          // enter configuration for PWM test
    timer_setup.timer_reference = 3;
    timer_setup.timer_mode  = (TIMER_PWM | TIMER_PWM_START_2 | TIMER_PWM_START_3); // generate PWM signal on PWM3 output and synchronise all PWM outputs
    timer_setup.timer_value = TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500));
    timer_setup.pwm_value   = _PWM_TENTH_PERCENT(706, TIMER_US_DELAY(TIMER_FREQUENCY_VALUE(1500))); // 70.6% PWM (high/low) on different channel
    #elif defined _HW_SAM7X && defined TEST_CAPTURE
    timer_setup.timer_mode  = (TIMER_SOURCE_TIOA0);                      // configure clock source on 
    #else
        #if defined TEST_PERIODIC_TIMER
    timer_setup.timer_mode = (TIMER_PERIODIC);                           // period timer interrupt (activate HW timer mode if not accepted)
    timer_setup.timer_value = 1000;                                      // 1s periodic interrupt
        #else
    timer_setup.timer_value = 0;                                         // disable ms time delay
    timer_setup.timer_us_value += 100;                                   // each subsequent delay increased by 100us
        #endif
    #endif
    fnConfigureInterrupt((void *)&timer_setup);                          // configure timer interrupt for timer test
#endif
}
#endif
