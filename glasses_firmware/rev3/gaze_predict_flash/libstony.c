#include "libstony.h"
#include "stm32l1xx.h"
#include "main.h"
#include "diskio.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "predict_gaze.h"

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


extern __IO  uint32_t Receive_length ;
extern uint32_t sd_ptr;

extern unsigned short num_subsample;
extern unsigned short num_hidden;
extern unsigned int bh_offset;
extern unsigned int bo_offset;
extern unsigned int mask_offset;
extern unsigned int who_offset;
extern unsigned int wih_offset;
extern unsigned int fpn_offset;

extern unsigned short model_data[];

int adc_idx = 0;

//extern unsigned short mask[num_subsample][2];
//extern float bh[num_hidden];
//extern float bo[2];
//extern float wih[num_subsample][num_hidden];

__IO uint16_t adc_values[2];

// ----------------Small helper functions, not to be exported-------------------
inline static void set_pin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t val)
{
  if (val)
    GPIOx->ODR |= GPIO_Pin;
  else
    GPIOx->ODR &= ~GPIO_Pin;
}

// ----------------Stonyman low-level interface functions-----------------------
inline static void pulse_resv(uint8_t cam)
{
  if (cam == CAM1) {
    CAM1_RESV_BANK->ODR |= CAM1_RESV_PIN;
    CAM1_RESV_BANK->ODR &= ~CAM1_RESV_PIN;
  }
  else {
    CAM2_RESV_BANK->ODR |= CAM2_RESV_PIN;
    CAM2_RESV_BANK->ODR &= ~CAM2_RESV_PIN;
  }
}

inline static void pulse_incv(uint8_t cam)
{
  if (cam == CAM1) {
    CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
    CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
  }
  else {
    CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
    CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
  }
}

inline static void pulse_resp(uint8_t cam)
{
  if (cam == CAM1) {
    CAM1_RESP_BANK->ODR |= CAM1_RESP_PIN;
    CAM1_RESP_BANK->ODR &= ~CAM1_RESP_PIN;
  }
  else {
    CAM2_RESP_BANK->ODR |= CAM2_RESP_PIN;
    CAM2_RESP_BANK->ODR &= ~CAM2_RESP_PIN;
  }
}

inline static void pulse_incp(uint8_t cam)
{
  if (cam == CAM1) {
    CAM1_INCP_BANK->ODR |= CAM1_INCP_PIN;
    CAM1_INCP_BANK->ODR &= ~CAM1_INCP_PIN;
  }
  else {
    CAM2_INCP_BANK->ODR |= CAM2_INCP_PIN;
    CAM2_INCP_BANK->ODR &= ~CAM2_INCP_PIN;
  }
}

//inline static void pulse_inph(unsigned short time, uint8_t cam)
//{
//  if (cam == CAM1) {
//    CAM1_INPH_BANK->ODR |= CAM1_INPH_PIN;
//    delay_us(time);
//    CAM1_INPH_BANK->ODR &= ~CAM1_INPH_PIN;
//  }
//  else {
//    CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
//    delay_us(time);
//    CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
//  }
//}

// clear_values
// Resets the value of all registers to zero
void clear_values(uint8_t cam)
{
  short i;

  for (i = 0; i < 8; ++i)
  {
    set_pointer_value(i,0,cam);  //set each register to zero
  }
}

// set_pointer_value
// Sets the pointer to a register and sets the value of that register
void set_pointer_value(char ptr, short val, uint8_t cam)
{
  set_pointer(ptr, cam);  //set pointer to register
  set_value(val, cam);  //set value of that register
}

void inc_pointer_value(char ptr, short val, uint8_t cam)
{
  set_pointer(ptr, cam);  //set pointer to register
  inc_value(val, cam);  //set value of that register
}

// set_pointer
// Sets the pointer system register to the desired value
void set_pointer(char ptr, uint8_t cam)
{
  short ii;
  
  // clear pointer
  pulse_resp(cam);

  // increment pointer to desired value
  for (ii = 0; ii != ptr; ++ii)
  {
    pulse_incp(cam);
  }
}

// set_value
// Sets the value of the current register
void set_value(short val, uint8_t cam)
{
  short ii;
  
  // clear pointer
  pulse_resv(cam);

  // increment pointer
  for (ii = 0; ii != val; ++ii)
  {
    pulse_incv(cam);
  }
}

void inc_value(short val, uint8_t cam)
{
  short ii;

  // increment pointer
  for (ii = 0; ii != val; ++ii)
  {
    pulse_incv(cam);
  }
}

// set_biases
// Sets all three biases
void set_biases(short vref,short nbias,short aobias, uint8_t cam)
{
  set_pointer_value(REG_VREF,vref,cam);
  set_pointer_value(REG_NBIAS,nbias,cam);
  set_pointer_value(REG_AOBIAS,aobias,cam);
}

// ----------------Primary library functions------------------------------------

void stony_pin_config()
{
  // Enable HSI oscillator (required for ADC operation)
  RCC_HSICmd(ENABLE);
  
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHBPeriphClockCmd(CAM_AHB, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  
  GPIO_InitStructure.GPIO_Pin = CAM1_RESV_PIN;
  GPIO_Init(CAM1_RESV_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM1_RESP_PIN;
  GPIO_Init(CAM1_RESP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM1_INCV_PIN;
  GPIO_Init(CAM1_INCV_BANK, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = CAM1_INCP_PIN;
  GPIO_Init(CAM1_INCP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM1_INPH_PIN;
  GPIO_Init(CAM1_INPH_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM2_RESV_PIN;
  GPIO_Init(CAM2_RESV_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM2_RESP_PIN;
  GPIO_Init(CAM2_RESP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM2_INCV_PIN;
  GPIO_Init(CAM2_INCV_BANK, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = CAM2_INCP_PIN;
  GPIO_Init(CAM2_INCP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM2_INPH_PIN;
  GPIO_Init(CAM2_INPH_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  CAM1_AN_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CAM1_AN_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = CAM2_AN_PIN;
  GPIO_Init(CAM2_AN_BANK, &GPIO_InitStructure);

  /* Check that HSI oscillator is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
  
  /*------------------------ DMA1 configuration ------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  /* DMA1 channel1 configuration */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS; //(uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_values;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 2;
//  DMA_InitStructure.DMA_BufferSize = 1; // AMM
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_BankSelection(ADC1, ADC_Bank_A);
  
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
//  ADC_InitStructure.ADC_NbrOfConversion = 1;    // AMM
  ADC_Init(ADC1, &ADC_InitStructure);
  
  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles); // AMM
  
//  ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
  
  ADC_DiscModeChannelCountConfig(ADC1, 1);
  ADC_DiscModeCmd(ADC1, ENABLE);
  
  /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Wait until ADC1 ON status */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET);
}

///**
//  * @brief  Configure the ADC1 channel18 using DMA channel1.
//  * @param  None
//  * @retval None
//  */
//void ADC_DMA_Config(void)
//{
//  /*------------------------ DMA1 configuration ------------------------------*/
//  /* Enable DMA1 clock */
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//  /* DMA1 channel1 configuration */
//  DMA_DeInit(DMA1_Channel1);
//  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValue;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//  DMA_InitStructure.DMA_BufferSize = 1;
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
////  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
////  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;       // AMM
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;               // AMM
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
//  
//  /* Enable DMA1 channel1 */
//  DMA_Cmd(DMA1_Channel1, ENABLE);
//
//  /*----------------- ADC1 configuration with DMA enabled --------------------*/
//  /* Enable the HSI oscillator */
//  RCC_HSICmd(ENABLE);
//
//#if defined (USE_STM32L152_EVAL)
//  /* Enable GPIOB clock */
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
//  /* Configure PB.12 (ADC Channel18) in analog mode */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//#elif defined (USE_STM32L152D_EVAL)
//  /* Enable GPIOF clock */
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
//  /* Configure PF.10 (ADC Channel31) in analog mode */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);  
//#endif
//
//  /* Check that HSI oscillator is ready */
//  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
//
//  /* Enable ADC1 clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//  /* ADC1 configuration */
//  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
//  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStructure.ADC_NbrOfConversion = 1;
//  ADC_Init(ADC1, &ADC_InitStructure);
//
//#if defined (USE_STM32L152_EVAL)
//  /* ADC1 regular channel18 configuration */ 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_18, 1, ADC_SampleTime_4Cycles);
//#elif defined (USE_STM32L152D_EVAL)
//  /* ADC1 regular channel14 configuration */ 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_31, 1, ADC_SampleTime_4Cycles);
//#endif
//
//  /* Enable the request after last transfer for DMA Circular mode */
//  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
//  
//  /* Enable ADC1 DMA */
//  ADC_DMACmd(ADC1, ENABLE);
//  
//  /* Enable ADC1 */
//  ADC_Cmd(ADC1, ENABLE);
//
//  /* Wait until the ADC1 is ready */
//  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
//  {
//  }
//
//  /* Start ADC1 Software Conversion */ 
//  ADC_SoftwareStartConv(ADC1);
//}

void stony_init(short vref, short nbias, short aobias, char gain, char selamp)
{
  num_subsample = model_data[0];
  num_hidden = model_data[1];
  
  bh_offset = 2;
  bo_offset = bh_offset + num_hidden * 2;
  mask_offset = bo_offset + 4;
  who_offset = mask_offset + num_subsample * 2;
  wih_offset = who_offset + num_hidden * 2 * 2;
  fpn_offset = wih_offset + (num_hidden * num_subsample * 2);
  
  short config;
  char flagUseAmplifier;
  
  // Set MCU pins to interface with CPLD
  stony_pin_config();
  
  // Set all pins low
  set_pin(CAM1_RESV_BANK, CAM1_RESV_PIN, 0);
  set_pin(CAM1_INCV_BANK, CAM1_INCV_PIN, 0);
  set_pin(CAM1_RESP_BANK, CAM1_RESP_PIN, 0);
  set_pin(CAM1_INCP_BANK, CAM1_INCP_PIN, 0);
  set_pin(CAM1_INPH_BANK, CAM1_INPH_PIN, 0);
  set_pin(CAM2_RESV_BANK, CAM2_RESV_PIN, 0);
  set_pin(CAM2_INCV_BANK, CAM2_INCV_PIN, 0);
  set_pin(CAM2_RESP_BANK, CAM2_RESP_PIN, 0);
  set_pin(CAM2_INCP_BANK, CAM2_INCP_PIN, 0);
  set_pin(CAM2_INPH_BANK, CAM2_INPH_PIN, 0);

  //clear all chip register values
  clear_values(CAM1);
  clear_values(CAM2);

  //set up biases
  // TODO russ: haven't looked at what this function does
  set_biases(vref,nbias,aobias,CAM1);
  set_biases(vref,nbias,aobias,CAM2);

  // sanitize this input before use
  flagUseAmplifier=selamp ? 1:0;

  config = gain + (flagUseAmplifier * 8) + 16;

  //turn chip on with config value
  set_pointer_value(REG_CONFIG,config,CAM1);
  set_pointer_value(REG_CONFIG,config,CAM2);
}

int stony_read_pixel()
{
  uint16_t ADCdata;

  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);

  /* Wait until ADC Channel 5 or 1 end of conversion */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  
  ADCdata = ADC_GetConversionValue(ADC1);
  
  return ADCdata;
}

//uint16_t test_audio(void)
//{
//  uint16_t ADCdata;
//  ADC_InitTypeDef ADC_InitStructure;
//  GPIO_InitTypeDef GPIO_InitStructure;
//  
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
//  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
//  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);
//
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//
//  ADC_BankSelection(ADC1, ADC_Bank_A);
//  
//  ADC_StructInit(&ADC_InitStructure);
//  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
//  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
//  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStructure.ADC_NbrOfConversion = 1;
//  ADC_Init(ADC1, &ADC_InitStructure);
//  
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_4Cycles);
//  
//  /* Enable ADC1 */
//  ADC_Cmd(ADC1, ENABLE);
//
//  /* Wait until ADC1 ON status */
//  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
//  {
//  }
//  
//  /* Start ADC1 Software Conversion */
//  ADC_SoftwareStartConv(ADC1);
//
//  /* Wait until ADC Channel 5 or 1 end of conversion */
//  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
//  {
//  }
//  
//  ADCdata = ADC_GetConversionValue(ADC1);
//  
//  return ADCdata;
//}

//int stony_read_pixel()
//{
//  GPIO_InitTypeDef GPIO_InitStructure;
//  unsigned int pixel_val = 0;
//  
////  // Trigger read process
////  READ_EN_BANK->ODR |= READ_EN_PIN;
////  
////  // Wait for CAM_OUT to go high, indicating that the ADC is ready to transmit
////  while (!(CAM_OUT_BANK->IDR & CAM_OUT_PIN));
////  
////  // Stop the ADC clock so we can read the bits one at a time
////  TIM_Cmd(PWM_TIM, DISABLE);
//  
//  // Set the clock pin to GPIO so we can toggle it manually
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
//  GPIO_InitStructure.GPIO_Pin = PWM_PIN;
//  GPIO_Init(PWM_BANK, &GPIO_InitStructure);
//  
//  // Pull clock low to prepare for read
//  PWM_BANK->ODR &= ~PWM_PIN;
//  
//  __istate_t s = __get_interrupt_state();
//  __disable_interrupt();
//  
//  // Trigger read process
//  READ_EN_BANK->ODR |= READ_EN_PIN;
//  
//  pulse_clock();
//  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
//  
//  pulse_clock();
//  
//  // Read ADC bits
//  for (int i = 0; i < 12; i++) {
//    pixel_val <<= 1;
//    
//    int test = read_adc_bit();
//    
//    if (test)
//      pixel_val += 1;
//  }
//  
//  // Trigger end of read sequence
//  READ_EN_BANK->ODR &= ~READ_EN_PIN;
//  
//  __set_interrupt_state(s);
//  
//  pulse_clock();
//  
//  // Turn camera clock back on
////  stony_pwm();
//  
//  return pixel_val;
//}
//
//short read_adc_bit()
//{
//  short val = CAM_OUT_BANK->IDR & CAM_OUT_PIN;
//  
//  PWM_BANK->ODR |= PWM_PIN;
//  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
////  delay_us(1);
//  PWM_BANK->ODR &= ~PWM_PIN;
////  delay_us(1);
//  
//  return val;
//}

//void stony_test()
//{
//  stony_pin_config();
//  
//  while (1)
//    pulse_inph(1);
//}

//int stony_image()
//{
//  UINT num_written;
//  unsigned char buf8[112 * 2];
//  unsigned short *buf16 = (unsigned short*)buf8;
//  
//  set_pointer_value(REG_ROWSEL, 0);
//
//  for (int row = 0; row < 112; row++) {
//    set_pointer_value(REG_COLSEL, 0);
//    
//    delay_us(1);
//    
//    for (int col = 0; col < 112; col++) {
//      pulse_inph(1);
////      delay_us(1);
//      
////      short tmp = stony_read_pixel();
////      buf16[col] = tmp;
//      
//      buf16[col] = stony_read_pixel();
//      
//      pulse_incv();
//    } // for (col)
//    
//    inc_pointer_value(REG_ROWSEL, 1);
//    
////    if (f_write(file, buf8, 112 * 2, &num_written))      return -1;
//
//  } // for (row)
//  
//  return 0;
//}

//int stony_image_inline()
//{
//  UINT num_written;
//  uint8_t buf8[2][112 * 16 * 2];
//  uint16_t *buf16 = (uint16_t *)buf8[0];
//  uint16_t start, total;
//  uint8_t buf_idx = 0;
//  
//  set_pointer_value(REG_ROWSEL, 0, CAM1);
//
//  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
//    set_pointer_value(REG_COLSEL, 0, CAM1);
//    
//    delay_us(1);
//    
//    for (int col = 0; col < 112; col++) {
//      CAM1_INPH_BANK->ODR |= INPH_PIN;
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      CAM1_INPH_BANK->ODR &= ~INPH_PIN;
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      
//      /* Start ADC1 Software Conversion */
//      ADC_SoftwareStartConv(ADC1);
//
//      /* Wait until ADC Channel 5 or 1 end of conversion */
//      while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
//      
//      buf16[(data_cycle * 112) + col] = ADC_GetConversionValue(ADC1);
//      
////      buf16[(data_cycle * 112) + col] = 0xAAAA;
//      
//      INCV_BANK->ODR |= INCV_PIN;
//      INCV_BANK->ODR &= ~INCV_PIN;
//    } // for (col)
//    
//    inc_pointer_value(REG_ROWSEL, 1);
//
//    if (data_cycle == 15) {
//      if (row > 15) {
//        f_finish_write();
//      }
//      
//      if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, 7) != RES_OK)      return -1;
////      f_finish_write();
//      
//      buf_idx = !buf_idx;
//      buf16 = (uint16_t *)buf8[buf_idx];
//      
//      sd_ptr += 7;
//      data_cycle = -1;
//      
////      if (row == 31)    return 0;
//    }
//
//  } // for (row)
//  
//  f_finish_write();
//  
//  return 0;
//}

int stony_image_dual()
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, 2 cameras
  // Double-buffered (2-dim array)
  uint8_t buf8[2][112 * TX_ROWS * 2 * 2];
  uint16_t *buf16 = (uint16_t *)buf8[0];
  
  uint16_t cam2_offset = CAM2_OFFSET;
  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
  
  set_pointer_value(REG_ROWSEL, 0, CAM1);
  set_pointer_value(REG_ROWSEL, 0, CAM2);

  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
    set_pointer_value(REG_COLSEL, 0, CAM1);
    set_pointer_value(REG_COLSEL, 0, CAM2);
    
    delay_us(1);
    
    for (int col = 0; col < 112; col++) {      
      CAM1_INPH_BANK->ODR |= CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM1_INPH_BANK->ODR &= ~CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      if (col != 0) {
        buf16[(data_cycle * 112) + cam2_offset + (col - 1)] = adc_values[1];
      }
      
      // Do conversion for CAM1
      ADC_SoftwareStartConv(ADC1);
      
      if (col != 0) {
        CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
        CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
      }
      
      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      buf16[(data_cycle * 112) + col] = adc_values[0];
      
      // Do conversion for CAM2
      ADC_SoftwareStartConv(ADC1);

      CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
      CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
    } // for (col)
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    buf16[(data_cycle * 112) + cam2_offset + 111] = adc_values[1];
    
    inc_pointer_value(REG_ROWSEL, 1, CAM1);
    inc_pointer_value(REG_ROWSEL, 1, CAM2);

    if (data_cycle == TX_ROWS - 1) {
      if (row > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_BLOCKS) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      buf16 = (uint16_t *)buf8[buf_idx];
      
      sd_ptr += TX_BLOCKS;
      data_cycle = -1;
      
#if (112 % TX_ROWS != 0)
      if (row + TX_ROWS > 112) {
        cam2_offset = CAM2_MOD_OFFSET;
//        data_val = 0xCCCC;
      }
//      else
//        data_val = 0xBBBB;
#endif
      
//      if (row == 31)    return 0;
    }

  } // for (row)

#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_MOD_BLOCKS) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS;
#endif
  
  f_finish_write();
  
  return 0;
}

int stony_image_single()
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row
  // Double-buffered (2-dim array)
  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint16_t *buf16 = (uint16_t *)buf8[0];
  
  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(REG_ROWSEL, 0, CAM1);

  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
    set_pointer_value(REG_COLSEL, 0, CAM1);
    
    delay_us(1);
    
    for (int col = 0; col < 112; col++) {      
      CAM1_INPH_BANK->ODR |= CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM1_INPH_BANK->ODR &= ~CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do conversion for CAM2
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      buf16[(data_cycle * 112) + col] = adc_values[adc_idx];
      adc_idx = !adc_idx;

      CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
      CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
    } // for (col)
    
    inc_pointer_value(REG_ROWSEL, 1, CAM1);

    if (data_cycle == TX_ROWS - 1) {
      if (row > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      buf16 = (uint16_t *)buf8[buf_idx];
      
      sd_ptr += TX_BLOCKS / 2;
      data_cycle = -1;      
//      if (row == 31)    return 0;
    }
  } // for (row)

#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

int stony_image_test()
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row
  // Double-buffered (2-dim array)
  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint16_t *buf16 = (uint16_t *)buf8[0];
  uint16_t subsamples[NUM_SUBSAMPLE];
  uint16_t sub_idx = 0;
  
  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(REG_ROWSEL, 0, CAM2);

  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
    set_pointer_value(REG_COLSEL, 0, CAM2);
    
    delay_us(1);
    
    for (int col = 0; col < 112; col++) {      
      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do conversion for CAM2
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      buf16[(data_cycle * 112) + col] = adc_values[adc_idx];

      if (sub_idx < NUM_SUBSAMPLE && row == MASK(sub_idx, 0) && col == MASK(sub_idx, 1)) {
//        uint16_t temp = buf16[(data_cycle * 112) + col];
        subsamples[sub_idx] = buf16[(data_cycle * 112) + col] - FPN(sub_idx);
        sub_idx++;
      }
      adc_idx = !adc_idx;

      CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
      CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
    } // for (col)
    
    inc_pointer_value(REG_ROWSEL, 1, CAM2);

    if (data_cycle == TX_ROWS - 1) {
      if (row > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      buf16 = (uint16_t *)buf8[buf_idx];
      
      sd_ptr += TX_BLOCKS / 2;
      data_cycle = -1;      
//      if (row == 31)    return 0;
    }
    
  } // for (row)

#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  if (sub_idx < NUM_SUBSAMPLE)
    while(1);
    
  predict_gaze(subsamples);
  
  return 0;
}

int stony_image_subsample()
{
  uint16_t lastRow, lastCol;
  uint16_t subsamples[NUM_SUBSAMPLE];
//  uint16_t subsamples_raw[NUM_SUBSAMPLE];
  float x;
  float ah[NUM_HIDDEN];
  
  for (int i = 0; i < num_hidden; i++)  {
    ah[i] = BH(i) / (float)255;
  }
  
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(REG_ROWSEL, MASK(0, 0), CAM2);
  set_pointer_value(REG_COLSEL, 0, CAM2);
  
  lastRow = MASK(0, 0);
  lastCol = 0;

  for (int pixel = 0; pixel < num_subsample; pixel++) {
      if (MASK(pixel, 0) != lastRow)
      {
        char diff = MASK(pixel, 0) - lastRow;
        
        inc_pointer_value(REG_ROWSEL, diff, CAM2);
        
        lastRow = MASK(pixel, 0);

        set_pointer_value(REG_COLSEL, MASK(pixel, 1), CAM2);
        
        lastCol = MASK(pixel, 1);
        
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      } else {
        inc_value(MASK(pixel, 1) - lastCol, CAM2);
        
        lastCol = MASK(pixel, 1);
      }
      
      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      /* Start ADC1 Software Conversion */
      ADC_SoftwareStartConv(ADC1);
      
      if (pixel > 0) {
        x = (float)(subsamples[pixel - 1]) / 1000;

        for (int i = 0; i < num_hidden; i++) {
            ah[i] += x * WIH(pixel - 1, i);
        }
      }
      else {
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      }
      
//      subsamples_raw[pixel] = adc_values[adc_idx];
//      subsamples_raw[pixel] = FPN(pixel);
      subsamples[pixel] = adc_values[adc_idx] - FPN(pixel);
      adc_idx = !adc_idx;
  }
  
  x = (float)(subsamples[num_subsample - 1]) / 1000;

  for (int i = 0; i < num_hidden; i++) {
      ah[i] += x * WIH(num_subsample - 1, i);
  }
  
//  f_finish_write();
//  if (disk_write_fast(0, (uint8_t *)subsamples_raw, sd_ptr, 4) != RES_OK)      return -1;
//  sd_ptr += 4;
//  f_finish_write();
  
  if (finish_predict(ah) != 0)  return -1;
  
//  predict_gaze(subsamples);
  
  return 0;
}