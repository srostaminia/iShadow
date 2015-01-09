/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* TODO Define system operating frequency and system functions */
#define GetSystemClock() (80000000ul)
#define	GetPeripheralClock()		(GetSystemClock()/(1 << OSCCONbits.PBDIV))
#define	GetInstructionClock()		(GetSystemClock())

#define CPU_CORE_TIMER_HZ (GetSystemClock()/8)    // CPU CoreTimer in Hz
#define US_TO_CT_TICKS  (CPU_CORE_TIMER_HZ/1000000ul)    // uS to CoreTimer Ticks, used in void ShortDelay(UINT32 DelayCount);

#define TOGGLES_PER_SEC			1000
#define CORE_TICK_RATE	       (GetSystemClock()/2/TOGGLES_PER_SEC)
#define TESTWRITE

//*********************stonyman operatinal mode*********************************
//#define FAST_MODE
#define MEDIUM_MODE
//#define QUALITY_MODE

//*********************serial output for data*********************************
#define SERIAL_OUT


//***********************AD parameters******************************************
// define setup parameters for OpenADC10
#define ADC_PARAM1 ADC_MODULE_ON | ADC_FORMAT_INTG16 | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_OFF
//when we have two stonyman, we can set ADC_ALT_INPUT_ON to make adc convert alternating between two camera
#define ADC_PARAM2 ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_1 | ADC_BUF_16 | ADC_ALT_INPUT_OFF

#ifdef FAST_MODE
//2 Tad sampling time and Tad get from peripheral clock divided by 4
#define ADC_PARAM3 ADC_SAMPLE_TIME_2 | ADC_CONV_CLK_PB | ADC_CONV_CLK_Tcy //ADC_CONV_CLK_3Tcy2//
#endif

#ifdef MEDIUM_MODE
#define ADC_PARAM3 ADC_SAMPLE_TIME_2 | ADC_CONV_CLK_PB | ADC_CONV_CLK_3Tcy2//ADC_CONV_CLK_Tcy //ADC_SAMPLE_TIME_6
//#define ADC_PARAM3 ADC_SAMPLE_TIME_1 | ADC_CONV_CLK_PB | ADC_CONV_CLK_My//ADC_CONV_CLK_Tcy //ADC_SAMPLE_TIME_6
#endif

#ifdef QUALITY_MODE
//31 Tad sampling time and Tad get from peripheral clock divided by 6
#define ADC_PARAM3 ADC_SAMPLE_TIME_31 | ADC_CONV_CLK_PB | ADC_CONV_CLK_3Tcy2 //ADC_CONV_CLK_Tcy//
#endif


//ADC input chanel AN0
#define ADC_PARAM4 ENABLE_AN0_ANA
#define ADC_PARAM5 SKIP_SCAN_ALL
//ADC interrupt config
#define ADC_PARAM_INT ADC_INT_ON | ADC_INT_PRI_7
//ADC analog input chanel
#define ADC_PARAM_INPUT ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN0
