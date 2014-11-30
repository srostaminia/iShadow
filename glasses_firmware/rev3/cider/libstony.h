#ifndef __LIBSTONY_H
#define __LIBSTONY_H

#include "stm32l1xx.h"

#define CAM1                 1
#define CAM2                 2

#define DHR12R1_OFFSET             ((uint32_t)0x00000008)
#define DHR12R2_OFFSET             ((uint32_t)0x00000014)

#define CAM_AHB         RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC

#define CAM1_RESP_BANK       GPIOB
#define CAM1_RESP_PIN        GPIO_Pin_0
#define CAM1_INCP_BANK       GPIOB
#define CAM1_INCP_PIN        GPIO_Pin_1
#define CAM1_RESV_BANK       GPIOB
#define CAM1_RESV_PIN        GPIO_Pin_2
#define CAM1_INCV_BANK       GPIOB
#define CAM1_INCV_PIN        GPIO_Pin_3
#define CAM1_INPH_BANK       GPIOB
#define CAM1_INPH_PIN        GPIO_Pin_4
#define CAM1_AN_BANK         GPIOA
#define CAM1_AN_PIN          GPIO_Pin_2
#define CAM1_ADC_CHAN        ADC_Channel_2

#define CAM2_RESP_BANK       GPIOC
#define CAM2_RESP_PIN        GPIO_Pin_0
#define CAM2_INCP_BANK       GPIOC
#define CAM2_INCP_PIN        GPIO_Pin_1
#define CAM2_RESV_BANK       GPIOC
#define CAM2_RESV_PIN        GPIO_Pin_2
#define CAM2_INCV_BANK       GPIOC
#define CAM2_INCV_PIN        GPIO_Pin_3
#define CAM2_INPH_BANK       GPIOC
#define CAM2_INPH_PIN        GPIO_Pin_4
#define CAM2_AN_BANK         GPIOA
#define CAM2_AN_PIN          GPIO_Pin_3
#define CAM2_ADC_CHAN        ADC_Channel_3

#define REG_COLSEL      0  //select column
#define REG_ROWSEL      1  //select row
#define REG_VSW         2  //vertical switching
#define REG_HSW         3  //horizontal switching
#define REG_VREF        4  //voltage reference
#define REG_CONFIG      5  //configuration register
#define REG_NBIAS       6  //nbias
#define REG_AOBIAS      7  //analog out bias
#define NUM_REG         8  

#define SMH_VREF_3V3    41     //vref for 3.3 volt
#define SMH_NBIAS_3V3   50     //nbias for 3.3 volts
#define SMH_AOBIAS_3V3  37     //aobias for 3.3 volts
#define SMH_GAIN_3V3    2//3   //amp gain for 3.3 volts 
#define SMH_SELAMP_3V3  1

#define SMH_VREF_OUT    48

#define ADC1_DR_ADDRESS                 ((uint32_t)0x40012458)
#define DMA_DIR_PeripheralToMemory      ((uint32_t)0x00000000)

// Uncomment to transmit eye-facing camera instead of out-facing
//#define SEND_EYE

// Comment out to use unmasked eye pixels
//#define USE_FPN_EYE

// Comment out to leave LEDs on at all times
//#define LED_DUTY_CYCLE

// Uncomment one or the other for pixel transmission rate
//#define SEND_16BIT
#define SEND_8BIT

// CIDER parameters
#define PEAK_THRESH     0.22
#define SPEC_THRESH     1

#if defined(SEND_16BIT) && defined(SEND_8BIT)
#error CANNOT DEFINE BOTH SEND_16BIT AND SEND_8BIT (LIBSTONY.H)
#elif !defined(SEND_16BIT) && !defined(SEND_8BIT)
#error MUST DEFINE ONE OF SEND_16BIT OR SEND_8BIT (LIBSTONY.H)
#endif

#ifdef SEND_16BIT
#define USB_PIXELS      92
#else
//#define USB_PIXELS      184
#define USB_PIXELS      112
#define CONV_8BIT(X)    (((X) >> 2) & 0xFF)
#endif

#define LED_LOW         0

//#define LED_HIGH        0        
#define LED_HIGH          0x59E       // 1.15V
//#define LED_HIGH        0x5D1         // 1.25V
//#define LED_HIGH        0x64D
//#define LED_HIGH        0x746

void stony_pin_config();
void stony_init(short vref, short nbias, short aobias, char gain, char selamp);
void dac_init();
int stony_read_pixel();
int stony_image_cider();
int stony_image_subsample();
int stony_image_minmax();
int stony_image_dual_subsample();

void pulse_resv(uint8_t cam);
void pulse_incv(uint8_t cam);
void pulse_resp(uint8_t cam);
void pulse_incp(uint8_t cam);
void pulse_inph(unsigned short time, uint8_t cam);
void pulse_clock();
void clear_values(uint8_t cam);
void set_pointer_value(char ptr, short val, uint8_t cam);
void inc_pointer_value(char ptr, short val, uint8_t cam);
void set_pointer(char ptr, uint8_t cam);
void set_value(short val, uint8_t cam);
void inc_value(short val, uint8_t cam);
void set_biases(short vref, short nbias, short aobias, uint8_t cam);
short read_adc_bit();

#endif // __LIBSTONY_H