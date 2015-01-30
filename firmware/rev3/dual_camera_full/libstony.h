#ifndef __LIBSTONY_H
#define __LIBSTONY_H

#include "stm32l1xx.h"

// Camera to use for single-camera functions
#define SINGLE_CAM      CAM2

#define LED_LOW         0

//#define LED_HIGH        0        
#define LED_HIGH          0x59E       // 1.15V
//#define LED_HIGH        0x5D1         // 1.25V
//#define LED_HIGH        0x64D
//#define LED_HIGH        0x746

#define TX_ROWS         48

#define CAM2_OFFSET     TX_ROWS * 112
//#define CAM2_OFFSET     5376
#define TX_BLOCKS       (TX_ROWS * 112 * 4) / 512

#if (112 % TX_ROWS != 0)
#define CAM2_MOD_OFFSET (112 % TX_ROWS) * 112
#define TX_MOD_BLOCKS   ((112 % TX_ROWS) * 112 * 4 ) / 512

#if ((112 % TX_ROWS) * 112 * 4 ) % 512 != 0
#error TX_ROWS invalid, does not align to 512B boundary
#endif

#endif

#define CAM1                 0
#define CAM2                 1

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

#if SINGLE_CAM == CAM1
#define SINGLE_PARAM(PNAME)    CAM1 ## _ ## PNAME
#elif SINGLE_CAM == CAM2
#define SINGLE_PARAM(PNAME)    CAM2 ## _ ## PNAME
#endif

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

#define ADC1_DR_ADDRESS                 ((uint32_t)0x40012458)
#define DMA_DIR_PeripheralToMemory      ((uint32_t)0x00000000)

void stony_pin_config();
void stony_init(short vref, short nbias, short aobias, char gain, char selamp);
int stony_read_pixel();
int stony_image_dual();
int stony_image_single();
int stony_cider_line(uint8_t rowcol_num, uint8_t *sd_buf, uint8_t rowcol_sel);
void dac_init();

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