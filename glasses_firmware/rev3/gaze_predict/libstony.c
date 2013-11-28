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

//extern unsigned short mask[NUM_SUBSAMPLE][2];
//extern float bh[NUM_HIDDEN];
//extern float bo[2];
//extern float wih[NUM_SUBSAMPLE][NUM_HIDDEN];

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

inline static void pulse_inph(unsigned short time, uint8_t cam)
{
  if (cam == CAM1) {
    CAM1_INPH_BANK->ODR |= CAM1_INPH_PIN;
    delay_us(time);
    CAM1_INPH_BANK->ODR &= ~CAM1_INPH_PIN;
  }
  else {
    CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
    delay_us(time);
    CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
  }
}

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
  UINT num_written;
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
  UINT num_written;
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row
  // Double-buffered (2-dim array)
  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint16_t *buf16 = (uint16_t *)buf8[0];
  
  volatile uint16_t start, total;
  uint8_t buf_idx = 0, adc_idx = 0;
  
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
      
      // Do conversion for CAM1
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


int stony_image_subsample()
{
  float bh[6] = { 0.0020756887310791798453846, -0.0033394073146260079380798, 0.0080194558715114723501660, 0.0012560038182027179285566, 0.0073546648824121940019038, -0.0024356065647634021591184 };

unsigned short mask[1003][2] = {
	{ 2, 44 },
	{ 4, 33 },
	{ 4, 39 },
	{ 4, 41 },
	{ 4, 42 },
	{ 4, 43 },
	{ 4, 44 },
	{ 4, 45 },
	{ 4, 46 },
	{ 4, 47 },
	{ 4, 48 },
	{ 4, 49 },
	{ 4, 53 },
	{ 4, 55 },
	{ 5, 36 },
	{ 5, 38 },
	{ 5, 40 },
	{ 5, 42 },
	{ 5, 44 },
	{ 5, 46 },
	{ 5, 47 },
	{ 5, 48 },
	{ 5, 49 },
	{ 5, 52 },
	{ 5, 53 },
	{ 5, 54 },
	{ 5, 56 },
	{ 6, 39 },
	{ 6, 41 },
	{ 6, 43 },
	{ 6, 45 },
	{ 6, 46 },
	{ 6, 47 },
	{ 6, 48 },
	{ 6, 49 },
	{ 6, 50 },
	{ 6, 51 },
	{ 6, 52 },
	{ 6, 53 },
	{ 6, 54 },
	{ 6, 57 },
	{ 6, 59 },
	{ 6, 61 },
	{ 6, 63 },
	{ 7, 39 },
	{ 7, 41 },
	{ 7, 43 },
	{ 7, 44 },
	{ 7, 46 },
	{ 7, 47 },
	{ 7, 48 },
	{ 7, 49 },
	{ 7, 51 },
	{ 7, 52 },
	{ 7, 53 },
	{ 7, 54 },
	{ 7, 56 },
	{ 7, 57 },
	{ 7, 58 },
	{ 7, 59 },
	{ 7, 61 },
	{ 7, 63 },
	{ 8, 35 },
	{ 8, 36 },
	{ 8, 37 },
	{ 8, 43 },
	{ 8, 44 },
	{ 8, 45 },
	{ 8, 47 },
	{ 8, 48 },
	{ 8, 49 },
	{ 8, 50 },
	{ 8, 53 },
	{ 8, 54 },
	{ 8, 55 },
	{ 8, 56 },
	{ 8, 58 },
	{ 8, 59 },
	{ 8, 61 },
	{ 8, 63 },
	{ 8, 65 },
	{ 9, 40 },
	{ 9, 41 },
	{ 9, 43 },
	{ 9, 44 },
	{ 9, 45 },
	{ 9, 46 },
	{ 9, 47 },
	{ 9, 48 },
	{ 9, 49 },
	{ 9, 51 },
	{ 9, 52 },
	{ 9, 53 },
	{ 9, 54 },
	{ 9, 55 },
	{ 9, 56 },
	{ 9, 57 },
	{ 9, 58 },
	{ 9, 59 },
	{ 9, 60 },
	{ 9, 61 },
	{ 9, 68 },
	{ 10, 41 },
	{ 10, 43 },
	{ 10, 44 },
	{ 10, 47 },
	{ 10, 48 },
	{ 10, 49 },
	{ 10, 50 },
	{ 10, 51 },
	{ 10, 53 },
	{ 10, 54 },
	{ 10, 55 },
	{ 10, 57 },
	{ 10, 59 },
	{ 10, 61 },
	{ 10, 63 },
	{ 10, 67 },
	{ 10, 71 },
	{ 11, 34 },
	{ 11, 42 },
	{ 11, 43 },
	{ 11, 44 },
	{ 11, 45 },
	{ 11, 46 },
	{ 11, 48 },
	{ 11, 49 },
	{ 11, 50 },
	{ 11, 55 },
	{ 11, 59 },
	{ 11, 61 },
	{ 11, 67 },
	{ 11, 70 },
	{ 12, 29 },
	{ 12, 31 },
	{ 12, 33 },
	{ 12, 35 },
	{ 12, 39 },
	{ 12, 41 },
	{ 12, 43 },
	{ 12, 44 },
	{ 12, 46 },
	{ 12, 47 },
	{ 12, 48 },
	{ 12, 49 },
	{ 12, 50 },
	{ 12, 51 },
	{ 12, 53 },
	{ 12, 55 },
	{ 12, 56 },
	{ 12, 59 },
	{ 12, 63 },
	{ 12, 65 },
	{ 13, 32 },
	{ 13, 36 },
	{ 13, 38 },
	{ 13, 40 },
	{ 13, 42 },
	{ 13, 43 },
	{ 13, 44 },
	{ 13, 45 },
	{ 13, 47 },
	{ 13, 49 },
	{ 13, 51 },
	{ 13, 52 },
	{ 13, 54 },
	{ 13, 56 },
	{ 13, 57 },
	{ 13, 60 },
	{ 14, 36 },
	{ 14, 38 },
	{ 14, 39 },
	{ 14, 40 },
	{ 14, 41 },
	{ 14, 42 },
	{ 14, 43 },
	{ 14, 44 },
	{ 14, 45 },
	{ 14, 46 },
	{ 14, 47 },
	{ 14, 48 },
	{ 14, 49 },
	{ 14, 50 },
	{ 14, 52 },
	{ 14, 54 },
	{ 14, 56 },
	{ 14, 59 },
	{ 15, 33 },
	{ 15, 37 },
	{ 15, 39 },
	{ 15, 40 },
	{ 15, 41 },
	{ 15, 42 },
	{ 15, 43 },
	{ 15, 45 },
	{ 15, 46 },
	{ 15, 47 },
	{ 15, 48 },
	{ 15, 49 },
	{ 15, 51 },
	{ 15, 60 },
	{ 16, 34 },
	{ 16, 36 },
	{ 16, 37 },
	{ 16, 38 },
	{ 16, 39 },
	{ 16, 40 },
	{ 16, 41 },
	{ 16, 42 },
	{ 16, 43 },
	{ 16, 44 },
	{ 16, 46 },
	{ 16, 48 },
	{ 16, 49 },
	{ 16, 51 },
	{ 16, 52 },
	{ 17, 29 },
	{ 17, 31 },
	{ 17, 33 },
	{ 17, 35 },
	{ 17, 36 },
	{ 17, 37 },
	{ 17, 39 },
	{ 17, 40 },
	{ 17, 41 },
	{ 17, 42 },
	{ 17, 43 },
	{ 17, 44 },
	{ 17, 45 },
	{ 17, 46 },
	{ 17, 47 },
	{ 17, 48 },
	{ 17, 50 },
	{ 17, 51 },
	{ 17, 52 },
	{ 17, 56 },
	{ 17, 58 },
	{ 18, 33 },
	{ 18, 34 },
	{ 18, 35 },
	{ 18, 36 },
	{ 18, 38 },
	{ 18, 40 },
	{ 18, 41 },
	{ 18, 43 },
	{ 18, 44 },
	{ 18, 45 },
	{ 18, 47 },
	{ 18, 48 },
	{ 18, 50 },
	{ 18, 56 },
	{ 19, 28 },
	{ 19, 32 },
	{ 19, 34 },
	{ 19, 36 },
	{ 19, 38 },
	{ 19, 44 },
	{ 19, 46 },
	{ 19, 48 },
	{ 19, 49 },
	{ 19, 52 },
	{ 19, 56 },
	{ 20, 21 },
	{ 20, 25 },
	{ 20, 29 },
	{ 20, 40 },
	{ 20, 44 },
	{ 20, 46 },
	{ 20, 49 },
	{ 20, 51 },
	{ 20, 52 },
	{ 20, 56 },
	{ 21, 20 },
	{ 21, 23 },
	{ 21, 29 },
	{ 21, 39 },
	{ 21, 42 },
	{ 21, 45 },
	{ 21, 47 },
	{ 21, 48 },
	{ 21, 49 },
	{ 21, 52 },
	{ 21, 53 },
	{ 21, 54 },
	{ 21, 56 },
	{ 21, 58 },
	{ 22, 20 },
	{ 22, 21 },
	{ 22, 22 },
	{ 22, 39 },
	{ 22, 41 },
	{ 22, 42 },
	{ 22, 43 },
	{ 22, 45 },
	{ 22, 47 },
	{ 22, 48 },
	{ 22, 49 },
	{ 23, 18 },
	{ 23, 19 },
	{ 23, 23 },
	{ 23, 25 },
	{ 23, 40 },
	{ 23, 43 },
	{ 23, 47 },
	{ 23, 48 },
	{ 23, 51 },
	{ 24, 17 },
	{ 24, 19 },
	{ 24, 21 },
	{ 24, 23 },
	{ 24, 35 },
	{ 24, 39 },
	{ 24, 41 },
	{ 24, 43 },
	{ 25, 18 },
	{ 25, 19 },
	{ 25, 36 },
	{ 25, 41 },
	{ 26, 17 },
	{ 27, 16 },
	{ 27, 17 },
	{ 27, 18 },
	{ 27, 19 },
	{ 28, 15 },
	{ 28, 21 },
	{ 28, 23 },
	{ 28, 25 },
	{ 29, 14 },
	{ 29, 16 },
	{ 29, 18 },
	{ 29, 19 },
	{ 29, 20 },
	{ 29, 21 },
	{ 29, 22 },
	{ 29, 24 },
	{ 30, 13 },
	{ 30, 15 },
	{ 30, 16 },
	{ 30, 17 },
	{ 30, 18 },
	{ 30, 20 },
	{ 30, 22 },
	{ 30, 41 },
	{ 30, 46 },
	{ 30, 55 },
	{ 30, 69 },
	{ 31, 13 },
	{ 31, 15 },
	{ 31, 17 },
	{ 31, 18 },
	{ 31, 19 },
	{ 31, 20 },
	{ 31, 21 },
	{ 31, 42 },
	{ 31, 44 },
	{ 31, 48 },
	{ 31, 50 },
	{ 31, 52 },
	{ 31, 54 },
	{ 31, 56 },
	{ 31, 70 },
	{ 32, 13 },
	{ 32, 16 },
	{ 32, 17 },
	{ 32, 18 },
	{ 32, 19 },
	{ 32, 20 },
	{ 32, 21 },
	{ 32, 44 },
	{ 32, 52 },
	{ 32, 54 },
	{ 32, 70 },
	{ 33, 15 },
	{ 33, 17 },
	{ 33, 18 },
	{ 33, 19 },
	{ 33, 20 },
	{ 33, 21 },
	{ 33, 22 },
	{ 33, 48 },
	{ 33, 51 },
	{ 33, 52 },
	{ 33, 70 },
	{ 34, 13 },
	{ 34, 15 },
	{ 34, 16 },
	{ 34, 17 },
	{ 34, 18 },
	{ 34, 19 },
	{ 34, 20 },
	{ 34, 21 },
	{ 34, 51 },
	{ 34, 53 },
	{ 34, 69 },
	{ 34, 70 },
	{ 35, 13 },
	{ 35, 15 },
	{ 35, 16 },
	{ 35, 17 },
	{ 35, 18 },
	{ 35, 19 },
	{ 35, 20 },
	{ 35, 21 },
	{ 35, 54 },
	{ 35, 68 },
	{ 35, 70 },
	{ 36, 12 },
	{ 36, 16 },
	{ 36, 18 },
	{ 36, 20 },
	{ 36, 22 },
	{ 36, 53 },
	{ 36, 55 },
	{ 37, 13 },
	{ 37, 14 },
	{ 37, 15 },
	{ 37, 16 },
	{ 37, 17 },
	{ 37, 18 },
	{ 37, 19 },
	{ 37, 21 },
	{ 37, 53 },
	{ 38, 10 },
	{ 38, 14 },
	{ 38, 15 },
	{ 38, 16 },
	{ 38, 17 },
	{ 38, 18 },
	{ 38, 19 },
	{ 38, 20 },
	{ 38, 53 },
	{ 38, 54 },
	{ 39, 13 },
	{ 39, 15 },
	{ 39, 16 },
	{ 39, 17 },
	{ 39, 18 },
	{ 39, 19 },
	{ 39, 21 },
	{ 39, 52 },
	{ 39, 53 },
	{ 39, 54 },
	{ 40, 17 },
	{ 40, 18 },
	{ 40, 19 },
	{ 40, 20 },
	{ 40, 21 },
	{ 40, 53 },
	{ 41, 13 },
	{ 41, 17 },
	{ 41, 18 },
	{ 41, 19 },
	{ 41, 20 },
	{ 41, 21 },
	{ 41, 54 },
	{ 41, 56 },
	{ 41, 68 },
	{ 42, 11 },
	{ 42, 13 },
	{ 42, 15 },
	{ 42, 17 },
	{ 42, 18 },
	{ 42, 19 },
	{ 42, 20 },
	{ 42, 21 },
	{ 42, 53 },
	{ 42, 54 },
	{ 42, 55 },
	{ 42, 56 },
	{ 42, 57 },
	{ 42, 58 },
	{ 42, 59 },
	{ 42, 60 },
	{ 42, 61 },
	{ 43, 9 },
	{ 43, 16 },
	{ 43, 17 },
	{ 43, 18 },
	{ 43, 19 },
	{ 43, 20 },
	{ 43, 21 },
	{ 43, 22 },
	{ 43, 53 },
	{ 43, 54 },
	{ 43, 55 },
	{ 43, 56 },
	{ 43, 58 },
	{ 43, 59 },
	{ 43, 61 },
	{ 44, 12 },
	{ 44, 14 },
	{ 44, 15 },
	{ 44, 17 },
	{ 44, 18 },
	{ 44, 19 },
	{ 44, 20 },
	{ 44, 21 },
	{ 44, 53 },
	{ 44, 54 },
	{ 44, 55 },
	{ 44, 56 },
	{ 44, 57 },
	{ 44, 58 },
	{ 44, 59 },
	{ 44, 60 },
	{ 44, 61 },
	{ 44, 62 },
	{ 44, 63 },
	{ 44, 65 },
	{ 44, 67 },
	{ 44, 68 },
	{ 45, 10 },
	{ 45, 13 },
	{ 45, 15 },
	{ 45, 16 },
	{ 45, 17 },
	{ 45, 18 },
	{ 45, 19 },
	{ 45, 20 },
	{ 45, 21 },
	{ 45, 54 },
	{ 45, 56 },
	{ 45, 57 },
	{ 45, 58 },
	{ 45, 59 },
	{ 45, 60 },
	{ 45, 61 },
	{ 45, 68 },
	{ 46, 10 },
	{ 46, 12 },
	{ 46, 13 },
	{ 46, 14 },
	{ 46, 15 },
	{ 46, 16 },
	{ 46, 17 },
	{ 46, 18 },
	{ 46, 19 },
	{ 46, 20 },
	{ 46, 21 },
	{ 46, 22 },
	{ 46, 52 },
	{ 46, 54 },
	{ 46, 55 },
	{ 46, 56 },
	{ 46, 57 },
	{ 46, 58 },
	{ 46, 59 },
	{ 46, 60 },
	{ 46, 66 },
	{ 47, 9 },
	{ 47, 11 },
	{ 47, 13 },
	{ 47, 14 },
	{ 47, 15 },
	{ 47, 16 },
	{ 47, 17 },
	{ 47, 18 },
	{ 47, 19 },
	{ 47, 20 },
	{ 47, 21 },
	{ 47, 23 },
	{ 47, 51 },
	{ 47, 52 },
	{ 47, 53 },
	{ 47, 54 },
	{ 47, 55 },
	{ 47, 58 },
	{ 47, 59 },
	{ 47, 60 },
	{ 47, 61 },
	{ 47, 62 },
	{ 47, 63 },
	{ 47, 64 },
	{ 47, 65 },
	{ 47, 66 },
	{ 47, 67 },
	{ 47, 68 },
	{ 47, 70 },
	{ 48, 10 },
	{ 48, 11 },
	{ 48, 12 },
	{ 48, 13 },
	{ 48, 14 },
	{ 48, 15 },
	{ 48, 16 },
	{ 48, 17 },
	{ 48, 19 },
	{ 48, 20 },
	{ 48, 21 },
	{ 48, 23 },
	{ 48, 50 },
	{ 48, 53 },
	{ 48, 54 },
	{ 48, 55 },
	{ 48, 57 },
	{ 48, 59 },
	{ 48, 60 },
	{ 48, 61 },
	{ 48, 64 },
	{ 48, 66 },
	{ 48, 68 },
	{ 49, 8 },
	{ 49, 10 },
	{ 49, 11 },
	{ 49, 12 },
	{ 49, 13 },
	{ 49, 14 },
	{ 49, 15 },
	{ 49, 16 },
	{ 49, 17 },
	{ 49, 19 },
	{ 49, 20 },
	{ 49, 21 },
	{ 49, 22 },
	{ 49, 23 },
	{ 49, 24 },
	{ 49, 53 },
	{ 49, 54 },
	{ 49, 57 },
	{ 49, 58 },
	{ 49, 59 },
	{ 49, 60 },
	{ 49, 61 },
	{ 49, 62 },
	{ 49, 63 },
	{ 49, 64 },
	{ 49, 65 },
	{ 50, 11 },
	{ 50, 12 },
	{ 50, 13 },
	{ 50, 14 },
	{ 50, 15 },
	{ 50, 16 },
	{ 50, 17 },
	{ 50, 18 },
	{ 50, 19 },
	{ 50, 20 },
	{ 50, 21 },
	{ 50, 22 },
	{ 50, 23 },
	{ 50, 24 },
	{ 50, 49 },
	{ 50, 50 },
	{ 50, 51 },
	{ 50, 52 },
	{ 50, 53 },
	{ 50, 54 },
	{ 50, 56 },
	{ 50, 57 },
	{ 50, 58 },
	{ 50, 59 },
	{ 50, 60 },
	{ 50, 61 },
	{ 50, 62 },
	{ 50, 63 },
	{ 51, 9 },
	{ 51, 10 },
	{ 51, 11 },
	{ 51, 12 },
	{ 51, 13 },
	{ 51, 14 },
	{ 51, 15 },
	{ 51, 16 },
	{ 51, 17 },
	{ 51, 18 },
	{ 51, 19 },
	{ 51, 20 },
	{ 51, 21 },
	{ 51, 22 },
	{ 51, 23 },
	{ 51, 24 },
	{ 51, 54 },
	{ 51, 56 },
	{ 51, 57 },
	{ 51, 58 },
	{ 51, 59 },
	{ 51, 60 },
	{ 51, 61 },
	{ 51, 62 },
	{ 51, 64 },
	{ 51, 66 },
	{ 52, 9 },
	{ 52, 11 },
	{ 52, 12 },
	{ 52, 13 },
	{ 52, 14 },
	{ 52, 15 },
	{ 52, 16 },
	{ 52, 17 },
	{ 52, 18 },
	{ 52, 19 },
	{ 52, 20 },
	{ 52, 21 },
	{ 52, 22 },
	{ 52, 23 },
	{ 52, 24 },
	{ 52, 25 },
	{ 52, 48 },
	{ 52, 51 },
	{ 52, 52 },
	{ 52, 53 },
	{ 52, 54 },
	{ 52, 56 },
	{ 52, 57 },
	{ 52, 58 },
	{ 52, 59 },
	{ 52, 60 },
	{ 52, 61 },
	{ 52, 65 },
	{ 52, 69 },
	{ 53, 9 },
	{ 53, 11 },
	{ 53, 12 },
	{ 53, 13 },
	{ 53, 14 },
	{ 53, 15 },
	{ 53, 16 },
	{ 53, 17 },
	{ 53, 18 },
	{ 53, 19 },
	{ 53, 20 },
	{ 53, 21 },
	{ 53, 22 },
	{ 53, 23 },
	{ 53, 24 },
	{ 53, 26 },
	{ 53, 50 },
	{ 53, 51 },
	{ 53, 53 },
	{ 53, 54 },
	{ 53, 56 },
	{ 53, 57 },
	{ 53, 59 },
	{ 53, 60 },
	{ 53, 61 },
	{ 53, 64 },
	{ 54, 12 },
	{ 54, 13 },
	{ 54, 14 },
	{ 54, 15 },
	{ 54, 16 },
	{ 54, 17 },
	{ 54, 18 },
	{ 54, 19 },
	{ 54, 21 },
	{ 54, 22 },
	{ 54, 23 },
	{ 54, 24 },
	{ 54, 25 },
	{ 54, 26 },
	{ 54, 27 },
	{ 54, 48 },
	{ 54, 49 },
	{ 54, 53 },
	{ 54, 56 },
	{ 54, 61 },
	{ 54, 62 },
	{ 54, 63 },
	{ 55, 9 },
	{ 55, 11 },
	{ 55, 13 },
	{ 55, 15 },
	{ 55, 16 },
	{ 55, 17 },
	{ 55, 19 },
	{ 55, 21 },
	{ 55, 23 },
	{ 55, 24 },
	{ 55, 25 },
	{ 55, 27 },
	{ 55, 29 },
	{ 55, 35 },
	{ 55, 43 },
	{ 55, 44 },
	{ 55, 47 },
	{ 55, 48 },
	{ 55, 51 },
	{ 55, 54 },
	{ 55, 56 },
	{ 55, 59 },
	{ 55, 60 },
	{ 55, 64 },
	{ 56, 13 },
	{ 56, 14 },
	{ 56, 15 },
	{ 56, 17 },
	{ 56, 19 },
	{ 56, 21 },
	{ 56, 23 },
	{ 56, 24 },
	{ 56, 25 },
	{ 56, 26 },
	{ 56, 27 },
	{ 56, 29 },
	{ 56, 37 },
	{ 56, 43 },
	{ 56, 49 },
	{ 56, 52 },
	{ 56, 54 },
	{ 56, 56 },
	{ 56, 58 },
	{ 56, 62 },
	{ 57, 12 },
	{ 57, 14 },
	{ 57, 16 },
	{ 57, 22 },
	{ 57, 24 },
	{ 57, 25 },
	{ 57, 26 },
	{ 57, 27 },
	{ 57, 28 },
	{ 57, 30 },
	{ 57, 31 },
	{ 57, 34 },
	{ 57, 35 },
	{ 57, 36 },
	{ 57, 39 },
	{ 57, 43 },
	{ 57, 45 },
	{ 57, 47 },
	{ 57, 48 },
	{ 57, 49 },
	{ 57, 51 },
	{ 57, 54 },
	{ 57, 56 },
	{ 57, 57 },
	{ 57, 58 },
	{ 57, 59 },
	{ 57, 60 },
	{ 57, 61 },
	{ 58, 12 },
	{ 58, 16 },
	{ 58, 24 },
	{ 58, 26 },
	{ 58, 30 },
	{ 58, 31 },
	{ 58, 34 },
	{ 58, 35 },
	{ 58, 36 },
	{ 58, 38 },
	{ 58, 39 },
	{ 58, 41 },
	{ 58, 42 },
	{ 58, 43 },
	{ 58, 44 },
	{ 58, 45 },
	{ 58, 46 },
	{ 58, 47 },
	{ 58, 48 },
	{ 58, 49 },
	{ 58, 50 },
	{ 58, 51 },
	{ 58, 52 },
	{ 58, 53 },
	{ 58, 57 },
	{ 58, 58 },
	{ 58, 59 },
	{ 58, 61 },
	{ 59, 16 },
	{ 59, 17 },
	{ 59, 18 },
	{ 59, 19 },
	{ 59, 26 },
	{ 59, 27 },
	{ 59, 29 },
	{ 59, 31 },
	{ 59, 33 },
	{ 59, 34 },
	{ 59, 35 },
	{ 59, 36 },
	{ 59, 37 },
	{ 59, 39 },
	{ 59, 40 },
	{ 59, 41 },
	{ 59, 42 },
	{ 59, 43 },
	{ 59, 44 },
	{ 59, 45 },
	{ 59, 47 },
	{ 59, 48 },
	{ 59, 49 },
	{ 59, 51 },
	{ 59, 52 },
	{ 59, 53 },
	{ 59, 55 },
	{ 59, 57 },
	{ 59, 59 },
	{ 60, 16 },
	{ 60, 18 },
	{ 60, 24 },
	{ 60, 29 },
	{ 60, 30 },
	{ 60, 31 },
	{ 60, 33 },
	{ 60, 34 },
	{ 60, 35 },
	{ 60, 36 },
	{ 60, 37 },
	{ 60, 38 },
	{ 60, 39 },
	{ 60, 40 },
	{ 60, 42 },
	{ 60, 43 },
	{ 60, 44 },
	{ 60, 46 },
	{ 60, 47 },
	{ 60, 48 },
	{ 60, 49 },
	{ 60, 51 },
	{ 60, 53 },
	{ 61, 12 },
	{ 61, 16 },
	{ 61, 28 },
	{ 61, 30 },
	{ 61, 31 },
	{ 61, 33 },
	{ 61, 34 },
	{ 61, 35 },
	{ 61, 36 },
	{ 61, 38 },
	{ 61, 39 },
	{ 61, 40 },
	{ 61, 42 },
	{ 61, 43 },
	{ 61, 44 },
	{ 61, 45 },
	{ 61, 46 },
	{ 61, 47 },
	{ 61, 48 },
	{ 61, 51 },
	{ 61, 52 },
	{ 61, 54 },
	{ 61, 56 },
	{ 61, 66 },
	{ 62, 13 },
	{ 62, 19 },
	{ 62, 28 },
	{ 62, 29 },
	{ 62, 30 },
	{ 62, 31 },
	{ 62, 33 },
	{ 62, 34 },
	{ 62, 35 },
	{ 62, 36 },
	{ 62, 38 },
	{ 62, 40 },
	{ 62, 42 },
	{ 62, 43 },
	{ 62, 44 },
	{ 62, 45 },
	{ 62, 46 },
	{ 62, 48 },
	{ 63, 14 },
	{ 63, 16 },
	{ 63, 18 },
	{ 63, 20 },
	{ 63, 28 },
	{ 63, 29 },
	{ 63, 34 },
	{ 63, 36 },
	{ 63, 42 },
	{ 63, 43 },
	{ 63, 44 },
	{ 63, 48 },
	{ 64, 16 },
	{ 64, 18 },
	{ 64, 28 },
	{ 64, 30 },
	{ 64, 31 },
	{ 64, 32 },
	{ 64, 34 },
	{ 64, 36 },
	{ 64, 37 },
	{ 64, 40 },
	{ 64, 42 },
	{ 64, 43 },
	{ 64, 44 },
	{ 64, 46 },
	{ 64, 48 },
	{ 64, 61 },
	{ 65, 17 },
	{ 65, 19 },
	{ 65, 21 },
	{ 65, 32 },
	{ 65, 33 },
	{ 65, 34 },
	{ 65, 36 },
	{ 65, 43 },
	{ 65, 44 },
	{ 65, 49 },
	{ 65, 59 },
	{ 65, 61 },
	{ 66, 17 },
	{ 66, 19 },
	{ 66, 34 },
	{ 66, 35 },
	{ 66, 39 },
	{ 66, 41 },
	{ 66, 43 },
	{ 67, 18 },
	{ 67, 34 },
	{ 67, 40 },
	{ 67, 58 },
};

float wih[1003][6] = {
	{ -0.0044254880486562933952777, -0.0039368894800072412598624, -0.0038327043515611648738517, -0.0018289824988444770576673, -0.0016158855403112380895447, -0.0023020414417787510644176 },
	{ -0.0036847279375834349895991, -0.0040059454267626646087330, -0.0036438465458544129493834, 0.0044415796222602192330764, 0.0037387803813493962132308, 0.0040775555816203934086928 },
	{ -0.0030010764279636051830857, -0.0029011276913733130729256, -0.0027484904596421131338368, 0.0007491406193349330836645, 0.0006419216855139219359352, 0.0006182634362516201955925 },
	{ -0.0028233213976558038486864, -0.0026790582775345888472129, -0.0025554440026125091987852, 0.0002866717181215621139574, 0.0002656605424517327032792, 0.0001926953840722544933437 },
	{ -0.0039641862813405721183146, -0.0037713106726754170444638, -0.0035855917687024410622776, 0.0004014089422860637004255, 0.0002658298826946824079329, -0.0000494914405965077416807 },
	{ -0.0036086173726377832049472, -0.0035308608175096869302712, -0.0033343846034255011055725, 0.0011124639807093919009751, 0.0009374813483659689842056, 0.0008698575953438801274323 },
	{ -0.0037203786237801749620102, -0.0036149886015581699991173, -0.0034115159520304860762963, 0.0010584435272767149689710, 0.0008513881273536306939359, 0.0007218071948115032830315 },
	{ -0.0042088770113356023527817, -0.0040987312518718128917516, -0.0038704481505663058081568, 0.0011014841437155420845323, 0.0009251606744442867269126, 0.0008214297217362706137606 },
	{ -0.0038647484887217868794462, -0.0036072220581597181331135, -0.0034515763005885410734386, -0.0002412762350278535008943, -0.0002746746253701596077700, -0.0006401101391132049461472 },
	{ -0.0038426032665616581540657, -0.0037644721996540468113290, -0.0035498985453263510320143, 0.0012139389265822489883534, 0.0010190883957539990413432, 0.0009946670107433600548086 },
	{ -0.0041344303732496062719903, -0.0039372435242886793041461, -0.0037382438563329819895820, 0.0004264179012134340185196, 0.0002690984240588520023940, -0.0000619704194802902570043 },
	{ -0.0042400534858258549439358, -0.0040075140478125241202956, -0.0038302326444300668559229, 0.0000813522472770763816234, 0.0000378993859379100794696, -0.0002800452561492535884409 },
	{ -0.0042241717619625933702321, -0.0038786811408680170465413, -0.0037398025568914678058718, -0.0008057710897774058265985, -0.0007378194473035746235590, -0.0011577330027676420009380 },
	{ -0.0037293213984367721419710, -0.0034039447012471869477568, -0.0032875723757222617867069, -0.0009293068199185812406846, -0.0008272685186411589180686, -0.0012460159579875979430491 },
	{ -0.0032139415869609300538401, -0.0031802246109086601891236, -0.0029881775042625469303303, 0.0013403799995579150031860, 0.0011513250641292619314698, 0.0012232175226332339668112 },
	{ -0.0041904224537352248100563, -0.0041449743341991822712100, -0.0038919933296387271802474, 0.0015972204693975940344164, 0.0013338168823313219756022, 0.0012647319616768079066083 },
	{ -0.0051309192607871091998395, -0.0050242360157550592161813, -0.0047290681345011275835133, 0.0016476844113594969840758, 0.0013323620159595439289141, 0.0011822181488197020752390 },
	{ -0.0044452951322321724095255, -0.0042162222290618887529656, -0.0040148587405146613643625, 0.0003539171837720989175630, 0.0002453099594650943755109, -0.0000156182995453657000936 },
	{ -0.0043205621696438658088657, -0.0040686490432306479311797, -0.0038858893669586279905059, 0.0000428032270160072533089, 0.0000000000000000000000000, -0.0002005766095023346965148 },
	{ -0.0041051902991142428037130, -0.0037770709573743121989509, -0.0036360129935351041124969, -0.0006866686252267854709622, -0.0006032443649255969696579, -0.0009136703173285243501436 },
	{ -0.0032765328990051541431516, -0.0030506912598111100322351, -0.0029310468223624598985144, -0.0001446522633372764065664, -0.0001427196051557321930178, -0.0003337110961925798249469 },
	{ -0.0037862619201644190196354, -0.0034474017257680590435587, -0.0033287468578191690450618, -0.0008332531490970635250012, -0.0007504297026641347165277, -0.0011218102960129489563401 },
	{ -0.0031636721169481631446563, -0.0028484343509682119068016, -0.0027691821086919879424393, -0.0009488319057483534041744, -0.0008438755842692116865625, -0.0012040520660285189673971 },
	{ -0.0014848080548679309497578, -0.0012263751431154049781336, -0.0012325098587663760579120, -0.0013260970557273379314550, -0.0010383639240310249193810, -0.0010527450456831700335153 },
	{ -0.0045875939934946780712766, -0.0041777240260074894120934, -0.0040364828648281397480146, -0.0011417109264685429328295, -0.0010888400419917650899931, -0.0017573317243984708929672 },
	{ -0.0017258177564461339627971, -0.0012081693081494369965828, -0.0012963411625827110824904, -0.0033975287659400170599266, -0.0028624853896123199532842, -0.0033254000200973040865526 },
	{ -0.0025190602169282020131313, -0.0023282396232980969059956, -0.0022378277699540470926931, -0.0004993483722592621971981, -0.0003791976481453102174220, -0.0004435934953342855172238 },
	{ -0.0029681593672807970833716, -0.0026862655751790261160483, -0.0026056708965767330296592, -0.0006907073198775565570778, -0.0005467072109967215142248, -0.0006114505021012887507267 },
	{ -0.0017550897658695509793991, -0.0015684480848417530450545, -0.0015334460793447479438206, -0.0004847939709570473127732, -0.0003384174763081985938713, -0.0002151416788475199101931 },
	{ -0.0007842074588797325876074, -0.0006196866740938726698940, -0.0006491214372193343127779, -0.0007614645066725264166765, -0.0004965998934662183483466, -0.0002337008344421543054419 },
	{ -0.0015991114687803299407309, -0.0014110225238941880715471, -0.0013844057378590570676208, -0.0004851849622177617895116, -0.0003221402745146443083978, -0.0001596316906945616882416 },
	{ -0.0027739933792113920935085, -0.0025109149859756721423487, -0.0024332349971312198719242, -0.0006213144579459273621549, -0.0005185047544315209795338, -0.0007123928313000024194201 },
	{ -0.0012142033018523860389559, -0.0011417107749986210411819, -0.0010956905352290070235904, 0.0002674137576801409953674, 0.0003519435064191250199864, 0.0006885506941690925546487 },
	{ -0.0023782109632015250091919, -0.0021885739838472238229239, -0.0021102648782873848105901, -0.0002008728624927835976818, -0.0001654170415023519891216, -0.0003092303399639193938614 },
	{ -0.0042109680725372147791497, -0.0037121316371770919292916, -0.0036355249231358149723803, -0.0020047697235484628931756, -0.0017522671309335630975001, -0.0022606371294899138199574 },
	{ -0.0029682201359564441660310, -0.0027558368995132028617112, -0.0026431310943235680060992, -0.0001312826792966968086126, -0.0001207780213748473009325, -0.0003037545568058907748989 },
	{ -0.0012362010946536179829824, -0.0008856140200818512836153, -0.0009432190114643776478534, -0.0020723790411130418517849, -0.0016996695178858429401553, -0.0018481678371246880974998 },
	{ -0.0036858203581938741091872, -0.0035017919105411769373215, -0.0033341977524026520166012, 0.0003896457263025770957274, 0.0002810556538196232233620, 0.0000433388277799112101724 },
	{ -0.0036306586566256298861943, -0.0030246192497182149486190, -0.0030230119360345510115540, -0.0031667956952930319432238, -0.0027247183217288068954320, -0.0034030521436211160282714 },
	{ -0.0026621976344884928228351, -0.0026026203092671439456862, -0.0024537547312341568238325, 0.0008027337336598271448282, 0.0007083190810575251726913, 0.0007319474031610624133182 },
	{ -0.0028925096124560829499717, -0.0023360182724177780411168, -0.0023596314138243181836307, -0.0031379741927455820534043, -0.0026884674273885298886877, -0.0032671665184954498672987 },
	{ -0.0031740717714469198970639, -0.0027146281615689367948396, -0.0026828469425016938931794, -0.0022644509783841201312760, -0.0019492145380076170840811, -0.0024391121963331101503425 },
	{ -0.0042023753806202587823826, -0.0036798939525244859886921, -0.0036068083000164040570479, -0.0023615562376993900171873, -0.0020893980144740041497708, -0.0027892515412883621926665 },
	{ -0.0041718708550392870348933, -0.0037442641128101351172264, -0.0036325723796752861470549, -0.0015591356532804860352687, -0.0013997883786406178918682, -0.0019807668959053829904615 },
	{ -0.0014847787452100501010693, -0.0013746211460000470373777, -0.0013301379486231660297102, 0.0000870527498608526030836, 0.0001555696826520801961946, 0.0002805818794519526846593 },
	{ -0.0016094314542773140424420, -0.0016085007923078659206745, -0.0015087083531291640287053, 0.0009781567770421036121103, 0.0008945109469166101718099, 0.0011336395848566900752818 },
	{ -0.0015473451902743050930672, -0.0015114086579226639409090, -0.0014417004981152020497148, 0.0006570567341894744534114, 0.0006352434229849482447833, 0.0008506970523735428706819 },
	{ -0.0031573970544740682063478, -0.0030354338384032341323360, -0.0028875045913187729652860, 0.0007098923340833729984586, 0.0006242474256975915182857, 0.0006908732893207586997844 },
	{ -0.0025771743562569820323871, -0.0027687831370295660007863, -0.0025374894460141639469797, 0.0029592945358976148126362, 0.0025687541749131901844261, 0.0030689286549825029772054 },
	{ -0.0020038518360390548955774, -0.0019003398333281079288226, -0.0018196075825168270875126, 0.0001598187878217953970545, 0.0002126504395561223947908, 0.0003181231902257665791756 },
	{ -0.0024936083514687130968657, -0.0021950847025901708622908, -0.0021604949659669151797170, -0.0009963362501067584472286, -0.0008359141526412211618130, -0.0009788869382008160736908 },
	{ -0.0007657166841885782414268, -0.0005217458775093494717165, -0.0005789485929515154980646, -0.0014052728149349779512844, -0.0010850214138901160057515, -0.0010082225847009309259439 },
	{ -0.0040737812396575076578920, -0.0037652922922120570133064, -0.0036218287463582608982915, -0.0005245816937490743389616, -0.0004829719445294836056501, -0.0008470469261544728368021 },
	{ -0.0007148074306177515553629, -0.0005014940144387676794133, -0.0005511059615918933269826, -0.0009931908349608399664415, -0.0007446338549311194666622, -0.0005754672144950522243850 },
	{ -0.0025768206647563229698406, -0.0022783714985818367959602, -0.0022319972772705618663247, -0.0012180669557453679009013, -0.0010169172651832119724097, -0.0012638023000275490966698 },
	{ -0.0042933488348151785732321, -0.0041035305193497162087879, -0.0039075832263462022611589, 0.0006299257162989602880712, 0.0004984019137527033720819, 0.0003760772848446705804090 },
	{ -0.0032623779821703128602162, -0.0028053235233540790155571, -0.0027755035650024128222879, -0.0019666049111550999890785, -0.0017011918304972229766536, -0.0021244724403057719565313 },
	{ -0.0030488623405118819853499, -0.0027636268990630591738111, -0.0026745600476106969403678, -0.0009674512853624750007703, -0.0008128595676529896951745, -0.0010384927457152729694118 },
	{ -0.0020778244759076390686670, -0.0017432873719345220748372, -0.0017442884236169339372963, -0.0015366035876149069204499, -0.0012907766762858220374599, -0.0015006915209928909336889 },
	{ -0.0027164849895996269826814, -0.0021365150481657130265845, -0.0021887616731871809179488, -0.0035221295488818598132252, -0.0029997755478404679665982, -0.0036047074552807479609629 },
	{ -0.0029612871128537800874403, -0.0026562405175135409517162, -0.0025806336527200821225059, -0.0012380927027737860957740, -0.0010591291049276559902564, -0.0013740438852887020774324 },
	{ -0.0031294925466320258795883, -0.0029499640259355378035799, -0.0028118269595640127921088, -0.0001288721805515133107378, -0.0001194394956227827025953, -0.0002799429434000810746827 },
	{ -0.0023375956296921521909704, -0.0022495963230660782038173, -0.0021364259285630840187742, 0.0005107584397830868374116, 0.0004733562627564059065166, 0.0005751703999032523962134 },
	{ -0.0028286436785353299624979, -0.0027191214400482800576941, -0.0025877789648330608816840, 0.0004947450638609641451401, 0.0004089820615254643109901, 0.0002869351878556126197832 },
	{ -0.0016143333306717620213855, -0.0017510960716182030674076, -0.0015942762652861499749618, 0.0020816939799958829662718, 0.0018276436774561839217201, 0.0022166650617247798428500 },
	{ -0.0037734287722532579810453, -0.0037863748154609070360743, -0.0035417962724099459961846, 0.0020054521054497058407673, 0.0017227836502349699009107, 0.0018509122896380829619706 },
	{ -0.0015987240149497219509483, -0.0015351194901199120401020, -0.0014766439516406659315101, 0.0003213921512650313051102, 0.0003491237805635662139712, 0.0004685768675749170256509 },
	{ -0.0018044990566142028991931, -0.0018190185026760910102367, -0.0017059636212709619867284, 0.0011960789619923091042092, 0.0011297730606647240440138, 0.0015114392453456759052299 },
	{ -0.0010054666118503390558059, -0.0009375903753410213658376, -0.0009013918404220833558577, 0.0003121632595275921923800, 0.0003759602474249831155495, 0.0006983544567137965446146 },
	{ -0.0021233897388096248555478, -0.0021796694564192530506330, -0.0020430410224562050788832, 0.0016553115747154930996465, 0.0014547370136122380734206, 0.0017907457964875530490728 },
	{ -0.0036501003276384357855433, -0.0033578904554383429484354, -0.0032385550128787390401774, -0.0004330833555875601161532, -0.0003554755733014437063533, -0.0004723888526605994251338 },
	{ -0.0015418001846484749670779, -0.0016144962828092120164097, -0.0015021976901106310047646, 0.0013712229927790150953854, 0.0012254501572979770510075, 0.0014966506540431969655286 },
	{ -0.0004953478685985421986482, -0.0003870457097171252163446, -0.0004035866756574161867861, -0.0004720557862237958991411, -0.0002580195952328117811603, 0.0000594331607664736995744 },
	{ -0.0016769886896502691001065, -0.0013998641794197639980080, -0.0014081220907887889628313, -0.0013280962987479889847453, -0.0010756839629719079642900, -0.0011687588852035430547760 },
	{ -0.0027099827103259730570750, -0.0021652054638220110879820, -0.0021989873314455419227309, -0.0030615953654127138310048, -0.0025951697346508879014892, -0.0030884542851950910931358 },
	{ -0.0024043333533602779215899, -0.0021867797861351160118581, -0.0021302196906419820082723, -0.0005583975343787736262541, -0.0004408876682577127247877, -0.0004800645528565062247414 },
	{ -0.0022014961586199559805122, -0.0020670633756530200353385, -0.0019828102515421020327069, 0.0000000000000000000000000, 0.0000267229324202370589595, 0.0000000000000000000000000 },
	{ -0.0023954457246424839915455, -0.0019186081290593849559695, -0.0019512394972601100531090, -0.0026954231652479951515666, -0.0022542935112318720353164, -0.0025529262263676018133995 },
	{ -0.0030451171162670189786204, -0.0024411507630676368592326, -0.0024736162510198850955634, -0.0034204371946908659896103, -0.0028948089899391211445023, -0.0034329429770238550738426 },
	{ -0.0029079679459088608045603, -0.0026786582894197078115461, -0.0025753867442738029996940, -0.0004805465227557195067622, -0.0004026742571662574737834, -0.0005445117846399815872416 },
	{ -0.0029033369601650189581465, -0.0026125557460618497977733, -0.0025281721537444719859200, -0.0009940743824815960472119, -0.0008462087837322376229418, -0.0011085521322002690226827 },
	{ -0.0026671549741965000848709, -0.0026808202707059181763016, -0.0025137124940051518671991, 0.0012597777740369310237045, 0.0011019731761314969890886, 0.0012232563711455719974996 },
	{ -0.0004780969454894466209860, -0.0006658279381086041649740, -0.0005802777181802738510480, 0.0020521635709186791633740, 0.0018538923781566750241534, 0.0023539194740894021744271 },
	{ 0.0002551945244745635883558, 0.0002728632845941866835737, 0.0002174060132647000871068, 0.0000321583846145482985706, 0.0001716121062828968133352, 0.0005187582488455435864722 },
	{ 0.0005457790211040482305713, 0.0004378164740833395858488, 0.0004291705745652173837300, 0.0006975145428903550565841, 0.0007576061995678444243793, 0.0013297697327819299299712 },
	{ -0.0044134446352374658018025, -0.0045444892367178264108718, -0.0042242527219970883722855, 0.0032474921170874801630568, 0.0027185255785720100277225, 0.0028439769255527280050755 },
	{ -0.0015296123936291920308167, -0.0014977511467839919358236, -0.0014245432145765419846423, 0.0005738097522413999158550, 0.0005849459322654741461750, 0.0009113968723508870288866 },
	{ -0.0021164154097321340348981, -0.0020299829003188780973677, -0.0019492280612405940912191, 0.0004983105228336901513023, 0.0004713140641515484066497, 0.0005143961824118244412912 },
	{ -0.0041615547970882838663131, -0.0038233843297912129856331, -0.0037005883967603818082703, -0.0007399568786592921904727, -0.0006613760029194670547401, -0.0009724339820200824172081 },
	{ -0.0033167630664308299370813, -0.0030129617078697139981314, -0.0029319942215814240489558, -0.0007195579621873124734344, -0.0006110585321801063419037, -0.0008899687156279187816993 },
	{ -0.0002446601540574373854879, 0.0000836907316610647355895, -0.0000444627356665942071075, -0.0023623115501472651683867, -0.0019277782710457038936602, -0.0020401254528023739925890 },
	{ -0.0016766484091248149562725, -0.0014363430341698820661561, -0.0014296639719072869265798, -0.0010683499557639140973425, -0.0008522602525593704191759, -0.0008384116381147804775154 },
	{ -0.0018782118401290139562965, -0.0015673499177082290696045, -0.0015806122558846889911838, -0.0013546211654382690672133, -0.0011131533395361600108364, -0.0012724419801647839957381 },
	{ -0.0017125541159531010998451, -0.0013659513981087410827708, -0.0014006078722223560834720, -0.0019371899014864800512431, -0.0015853886424389629968695, -0.0016811648958236810429134 },
	{ -0.0003896366462406242268252, -0.0001160618162307342052285, -0.0002077453239969916001361, -0.0018884802082934130061653, -0.0015231711103754390704279, -0.0015683871159664650261206 },
	{ 0.0000602140351662617176183, 0.0002660703647917525018450, 0.0001761606463278583877344, -0.0015940823913407249374369, -0.0012310619322779109939547, -0.0010254402320276410996019 },
	{ -0.0002180825461911503897368, 0.0000970465256944322182371, -0.0000196543829903968787114, -0.0022653396310761591062044, -0.0018264337695876110053556, -0.0018866213367467309786063 },
	{ -0.0013950591121522669998262, -0.0011756619930681710354398, -0.0011724162869381760370552, -0.0010678461811084519195170, -0.0008347054429498985874541, -0.0007637719416928068520353 },
	{ -0.0012718304960042529135750, -0.0009272930742950487780982, -0.0009849254717018882630819, -0.0020388231789750608374812, -0.0016740419377955320125173, -0.0018589577535156509742464 },
	{ -0.0016290007129565950427796, -0.0011863930668181419444268, -0.0012538770200737560552018, -0.0028336117054845290710863, -0.0023460356254854609556193, -0.0025811398509122708555541 },
	{ -0.0018254379776868709817778, -0.0015587559677423929414830, -0.0015489549199428180592530, -0.0010949244107930629181513, -0.0008859568815009281591080, -0.0010258071464901001058900 },
	{ -0.0022016686223406980137118, -0.0018406085597442850814581, -0.0018363802714242930674837, -0.0018813172894812199998499, -0.0015922100634148781024557, -0.0018037360626790889268217 },
	{ -0.0014015268119034389546901, -0.0014831389040751360997028, -0.0013685430157795389766173, 0.0015267976064232730189218, 0.0013383194146287459742883, 0.0016074254593330850222899 },
	{ -0.0030311488714401188526715, -0.0029418815133033619779734, -0.0027980832975042879522121, 0.0009551702909175729949709, 0.0008182859010172774741343, 0.0008799276385898039407399 },
	{ -0.0017702197096477109965890, -0.0016826953418901170224375, -0.0016213901939023109884802, 0.0000757666272238235446795, 0.0001359202542421739036589, 0.0001628424485349697016685 },
	{ -0.0018673173881892880712069, -0.0017196628830156769109083, -0.0016782347411106119175472, -0.0000606597372901518874116, 0.0000000000000000000000000, 0.0001319562169344810074242 },
	{ 0.0002725491017627362810329, 0.0001685764507302292970325, 0.0001740257580779184880780, 0.0007057932789106666129214, 0.0007639576133386560179497, 0.0012396966462291840807564 },
	{ 0.0008476582262671600049436, 0.0010242451164695410593952, 0.0008862020326427798603761, -0.0015353748287771939486090, -0.0011761213294446039995256, -0.0009402026246049160823279 },
	{ -0.0006018772373847802465568, -0.0007307407106562797136431, -0.0006550730244960835954929, 0.0012919501070593651052137, 0.0012202335611965929237921, 0.0016143724140964149128818 },
	{ -0.0008889910155775463057939, -0.0006192024044459280406744, -0.0006802692673331619567512, -0.0013988354379983519628489, -0.0011103874519413331044965, -0.0010286570669973810459447 },
	{ 0.0009879510495999386729404, 0.0009768876192336095522911, 0.0008998876573679036403736, -0.0000859789179894622192712, 0.0000823805472559470413794, 0.0006528343103840524757134 },
	{ -0.0006121190396079031646390, -0.0004198919149894170060010, -0.0004720284769696404749036, -0.0012385466745504451063775, -0.0009451242047179547051128, -0.0008678616354607591445655 },
	{ -0.0012649695574713159737512, -0.0010798692969359500147541, -0.0010728691623041470681182, -0.0006355909491606523149115, -0.0004770317530830806983418, -0.0003882940499428005117627 },
	{ -0.0006617976607936016872347, -0.0004342606112382654121040, -0.0004882630796621596921091, -0.0012991130386831889909766, -0.0010172660392233690875019, -0.0008368341368566111718869 },
	{ -0.0021523023931824371637878, -0.0018368155161724009343666, -0.0018278950983855670082978, -0.0013901048336421460618201, -0.0011323927594583040479731, -0.0011566360472513609759099 },
	{ -0.0016223120661740999398637, -0.0011303278976460291033179, -0.0012221275034793269587824, -0.0030545951118786779897307, -0.0025648922276298767933267, -0.0028827331100315599886341 },
	{ -0.0017100274034798899382148, -0.0012505042244522540303975, -0.0013174562777142329560165, -0.0026980533212301849831094, -0.0022528004612823930401244, -0.0025139355040771460374216 },
	{ -0.0020193877263355451863458, -0.0016276983691116350422973, -0.0016466438600988959027605, -0.0020079721526702390931030, -0.0016781290410938429474158, -0.0018832075536762689187131 },
	{ -0.0040191257451861906596635, -0.0035598307438194408749976, -0.0034689688873391988713224, -0.0017877344691573449422806, -0.0015756649300836180361413, -0.0020930791457751249037833 },
	{ -0.0017025440629955129485246, -0.0017001222997623270313805, -0.0016113461871816898984822, 0.0006699737789922418371505, 0.0005900783248627845247944, 0.0006454680783971267427415 },
	{ -0.0027065032787508768417906, -0.0026071004583223221506838, -0.0024935826793426700553891, 0.0003964520000915815732025, 0.0003468617486336479779560, 0.0002910049731497382904596 },
	{ 0.0002178628507046966099152, 0.0000211293617152845596781, 0.0000571042982356847789345, 0.0015320518472623930426868, 0.0014134619392949529861903, 0.0018557026594287499756536 },
	{ 0.0008638458519361532630607, 0.0009901078273958038335201, 0.0008588137029499269374522, -0.0013563337765779268952182, -0.0010109641339145750051770, -0.0007885411310400446171023 },
	{ -0.0022097986427814008195125, -0.0020992081331066910106220, -0.0020148974370197240968405, 0.0003419137755513510087621, 0.0003179346060833330121775, 0.0003103238319519061190997 },
	{ -0.0000722832835845713383158, -0.0000306762630061838370262, -0.0000680350645452823218125, -0.0004222969448408961249189, -0.0002026589716272386102608, 0.0000835987809126305890777 },
	{ -0.0011958782839569020908094, -0.0011052245238978161082782, -0.0010849103698304740986402, -0.0001178810834914720962049, 0.0000000000000000000000000, 0.0001821414598886385944818 },
	{ -0.0004205592340076787227776, -0.0004897283720398564257903, -0.0004507404854847948732707, 0.0009405064312711557475422, 0.0008856826836695025698121, 0.0012942158552886899505058 },
	{ -0.0021816425419585649611576, -0.0019510926929155070642247, -0.0019101178938064930327989, -0.0008553279957890848892235, -0.0006978111181098062276718, -0.0008329361663419684608939 },
	{ 0.0003231860843009308112937, 0.0002346486476968506957631, 0.0002287143662659620101194, 0.0006797764129650481082848, 0.0006840023960045221035342, 0.0012323485731835180905652 },
	{ -0.0004619385297966315949598, 0.0000000000000000000000000, -0.0001632372557117910117069, -0.0035233706292152129797846, -0.0029330180737614530185609, -0.0032424338073244020688723 },
	{ -0.0002597549321374545785480, 0.0000365240781352153300043, -0.0000753875670747340416011, -0.0022174498460597038759778, -0.0018001755779719320286342, -0.0018523811453514599841691 },
	{ -0.0011206285961050689841539, -0.0008461028535015111635628, -0.0008835372295000519862751, -0.0015615249885265879582069, -0.0012599313208823239088052, -0.0012987622135545409847446 },
	{ -0.0027797862905818079837239, -0.0023819216490518738300886, -0.0023542887555657188852765, -0.0020266174841816839841124, -0.0017198171325993759275125, -0.0021232369772081600110825 },
	{ -0.0043611709063387333815132, -0.0046135184515192027149277, -0.0042519998401481812894120, 0.0040513933434714608297633, 0.0033593952216406739591514, 0.0034535785798255028318515 },
	{ -0.0038561727082300021468564, -0.0038494680733681561554671, -0.0036197015788269949233824, 0.0017874007097261339523098, 0.0014415095473906619841231, 0.0012505779604046649792354 },
	{ -0.0026744694530210929260539, -0.0024170811852457850348042, -0.0023544069412918490133424, -0.0007172493696075584162369, -0.0006579468140519526562007, -0.0010520390255072899123728 },
	{ -0.0005284788971225564497766, -0.0007589499417995438160234, -0.0006572873896608246542181, 0.0022802713699281999988611, 0.0020152775099028389163558, 0.0024671969946505711672458 },
	{ -0.0008879688967305482622319, -0.0008743185646227615362425, -0.0008516626839261140849424, 0.0004319565410981290811028, 0.0004400431131572135135324, 0.0005792186887224205077418 },
	{ -0.0015259938308758709372970, -0.0015835803877705040883933, -0.0014854753437499989415782, 0.0012077582292890959216741, 0.0010652395656484299479860, 0.0012192241077234539439744 },
	{ -0.0020590197640199627872326, -0.0022350056806809881146392, -0.0020551431702812319839146, 0.0023744636357577357868576, 0.0020540111804472301927638, 0.0023191717647051148486204 },
	{ 0.0001960505845382798893739, 0.0002346498130832742889940, 0.0001713808123541647060201, -0.0005502608448192758492518, -0.0003271246801271374777739, -0.0000337003847156160392493 },
	{ -0.0009921736663693861597857, -0.0007872395852969957816617, -0.0008224448182762911013430, -0.0013288944363896979840300, -0.0010974821590407260029493, -0.0012366625043808709711463 },
	{ 0.0011417314138655650784804, 0.0012362715132968689259879, 0.0011042525984865090606102, -0.0011325609550519219990983, -0.0008308265378106697001542, -0.0006024865828780577277704 },
	{ -0.0013036446236581870124910, -0.0011122649181407920880582, -0.0011269628366542589099908, -0.0009874923713061435712884, -0.0007746144940317736815594, -0.0007475459678321258157399 },
	{ 0.0005502632411091210040688, 0.0005256156128567230538876, 0.0004780935743776059884680, 0.0001676242015904384947706, 0.0002669514926041573069440, 0.0006591668293843749642721 },
	{ -0.0005984583283589877409150, -0.0006331173666341278864766, -0.0006053616328559832280784, 0.0004926880167555652747374, 0.0005330663822608927614535, 0.0009409127306489176905194 },
	{ 0.0005486775643797599591456, 0.0004891904461017856356117, 0.0004555385919177602099296, 0.0003403509947574951863790, 0.0004334305552084449222536, 0.0008993150152509361795403 },
	{ 0.0010600597704928020701548, 0.0009876250273084309118377, 0.0009265660755801075803076, 0.0001348784240157327950736, 0.0002522494329029045128689, 0.0006785742294294015144671 },
	{ 0.0003324864129144216228981, 0.0005470524610155173394349, 0.0004300561451475393907710, -0.0018773229658968939963237, -0.0015131232166744469462560, -0.0015533690281489969959444 },
	{ 0.0006942249740372881221251, 0.0010038701011224118991166, 0.0008234156285817836049348, -0.0029461023121293960363998, -0.0024086098584177578069143, -0.0025034589189744699440354 },
	{ 0.0006779654272638101523626, 0.0008895291802462834077764, 0.0007499246048098258225767, -0.0019046098468456819363670, -0.0015005114978863870120462, -0.0013878004447665339467916 },
	{ 0.0003953485566538625004505, 0.0007110439228786111665742, 0.0005570221996320627328575, -0.0026712803557602570261875, -0.0021801622881778520528229, -0.0022869879128759821457217 },
	{ -0.0001895848315414488092796, 0.0000675343338517137649389, -0.0000234708301903143485981, -0.0018145983483587530341041, -0.0014376327682415780944564, -0.0014211299619811300961464 },
	{ -0.0024393286711359629323481, -0.0024382489008187399788519, -0.0022914278283050321625980, 0.0011971507032272729782740, 0.0009856733771041127547402, 0.0009472731486299752930169 },
	{ -0.0004331719264410424153657, -0.0001822757947278062133981, -0.0002764445213045938236004, -0.0017341785660039479854966, -0.0014522088390463610066416, -0.0016336266014170860560312 },
	{ -0.0016092603065621159143250, -0.0013859631375317069817976, -0.0013897375644109800224557, -0.0010752488227705259745542, -0.0009287115745642986240974, -0.0011914370328989809001696 },
	{ -0.0012038106638467939782378, -0.0010184120720907019041340, -0.0010363221216949458983181, -0.0008611865188309402374331, -0.0007119755234569060753716, -0.0008600863079027367590415 },
	{ -0.0024074881028701929953917, -0.0023097376078631800047636, -0.0022172120228879351258588, 0.0002073293528447148106452, 0.0001549482229857272893268, 0.0000000000000000000000000 },
	{ 0.0003207402739571658945028, 0.0000000000000000000000000, 0.0000786384297938286751370, 0.0024227543385200371421628, 0.0021830711241563847957448, 0.0028307094903416481906921 },
	{ -0.0026359409412336940944921, -0.0025928706332391418358518, -0.0024696976597500968665655, 0.0007654606408921064541334, 0.0006482876384730757281585, 0.0006294200955142002828005 },
	{ 0.0013622514174538140081100, 0.0011896890813939070899452, 0.0011434233240816429975212, 0.0007764370740472008184177, 0.0007903827030047667684051, 0.0012686158943677470264250 },
	{ -0.0005801437426880360487269, -0.0005539833232708464777830, -0.0005502618919589378305643, 0.0000662263748204145190519, 0.0001505900773312353075478, 0.0003779077563902561733532 },
	{ 0.0000732143716824250524343, 0.0001139930515267903937205, 0.0000609712962561268721198, -0.0004343460722266220949347, -0.0002515526218860974073770, 0.0000171506943192214713787 },
	{ 0.0005219912361141610702975, 0.0003260515196318298060743, 0.0003514280937617274779526, 0.0012977233956010000674730, 0.0012298146031218730542950, 0.0017577276283185529464487 },
	{ 0.0008267428359841275109510, 0.0009206136930831424489488, 0.0008098926594827266650656, -0.0011743258976491519930685, -0.0008742205643963082148731, -0.0006810602930374387575796 },
	{ -0.0005956437768707300477525, -0.0002974592972673946998131, -0.0003942012725244914200448, -0.0022645452705987579970770, -0.0018573961407943849217378, -0.0019887996918313611584284 },
	{ 0.0029065786827685230822582, 0.0029101176225561739725756, 0.0026976568588063160915314, -0.0015603688878594609932421, -0.0011424934172336639187639, -0.0007678098310839160355046 },
	{ -0.0012274674897881200988370, -0.0010149376818553799278755, -0.0010274096593365219728944, -0.0012839833618224500821209, -0.0010699175920897640267027, -0.0012163041971087671083013 },
	{ 0.0017039705227469099588222, 0.0018580300603119599515967, 0.0016736417106560190753289, -0.0021152790171599278497283, -0.0016490132199796420954530, -0.0014787303294634009370040 },
	{ -0.0014051456639898409796408, -0.0012596415586535009926289, -0.0012402784614190939457834, -0.0002608197115700292153216, -0.0002415377756640047076861, -0.0003451742351298341809906 },
	{ -0.0014711520975180189269721, -0.0012239578943181060252665, -0.0012303856054084159627610, -0.0010868113251810500828609, -0.0009449003852529138085953, -0.0012159354047134580782685 },
	{ -0.0010684464084750669443658, -0.0009952808416644378774335, -0.0009817397867148598428894, -0.0001178820375243102994492, -0.0000297549691423057812356, 0.0000746676428580411141777 },
	{ -0.0002942922029366748233430, -0.0003498774297495220041328, -0.0003292373738073741794991, 0.0008213633316748875090596, 0.0007639505238232231895332, 0.0010607032781471080733898 },
	{ 0.0001182155421845312046145, -0.0000233959423654709005073, 0.0000000000000000000000000, 0.0010588367390837820879168, 0.0009944977955308627586700, 0.0013973652311399570227074 },
	{ -0.0031165917464223381536104, -0.0030550595430736760330093, -0.0028891741407851838537024, 0.0009736609634337160508813, 0.0007782658351616045765142, 0.0006157964717846155493913 },
	{ -0.0011532687408446109633003, -0.0009100824987634269268647, -0.0009704134338514684710031, -0.0015542355760904040615916, -0.0012918158667828299503155, -0.0014877106068707879413943 },
	{ -0.0024543402569798121633238, -0.0024440696016779900855143, -0.0023153824511074820417800, 0.0009924970245684461896823, 0.0008313037636626829278777, 0.0008468336404158982241852 },
	{ 0.0018651192717338668965177, 0.0018475852565752999643461, 0.0017050824107602930920780, -0.0008050460702313179568723, -0.0005456975795277003949899, -0.0002360115508805160091342 },
	{ -0.0012243954076010879483682, -0.0011888111208565169726187, -0.0011484404836113219165045, 0.0003342736243901439749555, 0.0003217809801688874867670, 0.0004332364187807073068262 },
	{ 0.0008503537041694468005121, 0.0006709565233489591176697, 0.0006539345377439266483385, 0.0008343503707110603521274, 0.0008249306837423621852762, 0.0012383746497118990666925 },
	{ -0.0004573803246052338209587, -0.0004448724888134455231377, -0.0004508567356667878174570, 0.0001620559108156496060799, 0.0002188824536946699093106, 0.0004355115456871599260991 },
	{ -0.0005247706980274256067728, -0.0003153241712870169782129, -0.0003968665925169134137823, -0.0016420093863127298988713, -0.0013105961809990549632454, -0.0013461141743262160404282 },
	{ 0.0002444493095662072937660, 0.0001338225835200793903054, 0.0001431436531495585892920, 0.0007866126057213582807234, 0.0007565425042685389482242, 0.0011403774725253499385674 },
	{ -0.0001382669528206170912483, -0.0000296022321120701399492, -0.0000860763556842556289486, -0.0009008778648649134242893, -0.0006701537415982230898315, -0.0005574271623278828939260 },
	{ 0.0014223075486634588916052, 0.0014620088527830560776871, 0.0013342934470816541006610, -0.0010973628365056060495614, -0.0008080406698332086431677, -0.0006169063550666253718802 },
	{ 0.0003530995278217576027746, 0.0006165637266990361096963, 0.0004790766683107716781415, -0.0023145286305633039868057, -0.0019034809854280470339111, -0.0020615241980623750031598 },
	{ 0.0019228406348528129937714, 0.0019080236642553200301919, 0.0017670711005910969167154, -0.0009815181311845690842466, -0.0006889351636451388925450, -0.0003497102113175668903003 },
	{ -0.0026340522389176499948316, -0.0024596425684633030708803, -0.0023688410528191110662743, -0.0001763933531539982086257, -0.0002309807328782634077650, -0.0005767246191819137791576 },
	{ -0.0013103394631231478958261, -0.0011859662243919380768958, -0.0011665883057443789030255, -0.0003988921464132047229982, -0.0003094283707200899268160, -0.0003885981146471386067122 },
	{ -0.0017269225748620220094870, -0.0015978468749002099937029, -0.0015595456346398110444862, -0.0002941784114114648200113, -0.0002326278863186396095742, -0.0003310080913664822910812 },
	{ -0.0006115808110698016213103, -0.0007091021662056683433698, -0.0006491475613847431458750, 0.0013024287673649949034771, 0.0011456549931505341077126, 0.0014644593808689090972536 },
	{ -0.0014124993170530611083513, -0.0012402128275628169704059, -0.0012391672024727709740255, -0.0008379848607694765890222, -0.0006874889377998452547405, -0.0008527500158528461549068 },
	{ -0.0002984503966527961836032, -0.0001029961249092984005606, -0.0001841154621235877909564, -0.0013114419542216120950079, -0.0011079994250562659290071, -0.0012337036714274850646850 },
	{ -0.0033860566010343057935517, -0.0031593798612737540587592, -0.0030558570937623138162598, -0.0005598051704963476421853, -0.0005345469634783683778728, -0.0009807949214571405182478 },
	{ 0.0000306257442503674428072, -0.0000926674280198986705321, -0.0000773015027304895728207, 0.0008594222801216641312652, 0.0008177598968921839361207, 0.0011523611042522300795116 },
	{ 0.0008429894114433296448130, 0.0009747621143749778073115, 0.0008417737771470847280356, -0.0013690260032530269379558, -0.0010831164429379429043482, -0.0009445648028191431459094 },
	{ -0.0033011684080786730076384, -0.0033136501538269841522777, -0.0031186030363295129223811, 0.0012611425834143290356654, 0.0010327689782666099604924, 0.0008679800255791340467953 },
	{ 0.0017418481048828060499950, 0.0015789016999702689889040, 0.0015086829591164029572281, 0.0006237001679282494666368, 0.0006610072311452597475928, 0.0011821342578133540481000 },
	{ -0.0031976413398518920526425, -0.0031899761739859729473001, -0.0030125182596960368638928, 0.0010919899812477419382556, 0.0009076106304116789060682, 0.0007867350520036901946608 },
	{ -0.0000980570081927630775006, -0.0002844336041317553154598, -0.0002275945867306282106943, 0.0013329207197441510224234, 0.0012199892402555651080981, 0.0015544667014360030385389 },
	{ 0.0007971665986665519643486, 0.0009297297725442964537110, 0.0008139443144081013725016, -0.0015511131724056329700412, -0.0012235869149089359693428, -0.0011111743873285380955485 },
	{ 0.0004291044148613663193961, 0.0006206281851364410300995, 0.0005052176862666422256856, -0.0012873118114108289024300, -0.0010524053194768119394442, -0.0010431302898217260487906 },
	{ -0.0015817084613048749703462, -0.0015206104378024060206492, -0.0014599966841370409353223, 0.0007854545987627110622056, 0.0006550122782547474322226, 0.0007110717240421961758690 },
	{ -0.0005436108531934181857526, -0.0004172159512238152873076, -0.0004515217394465114841146, -0.0005111865445246748700450, -0.0004094509986038933203704, -0.0003590604505960586965117 },
	{ -0.0017317572076122631028611, -0.0017445534835118109168867, -0.0016482480786152759084928, 0.0013034071405904588958347, 0.0010983517664965710899927, 0.0012022080500894000414797 },
	{ -0.0012598727832042919908689, -0.0009635682028622773653490, -0.0010069822113128760673884, -0.0017616831765342540108005, -0.0015132865497975230180405, -0.0018443337941787959436551 },
	{ -0.0019299441465096139631391, -0.0020225857309940911341051, -0.0018870975565222810731852, 0.0019223475674395109919629, 0.0016425512468999600192154, 0.0019096099371108609613762 },
	{ -0.0014957923217295420077744, -0.0012752855140671800170116, -0.0012765966670449779146734, -0.0010526887061863899382880, -0.0008858257227744338602182, -0.0010725428208953390139119 },
	{ -0.0014897626972549700679477, -0.0017118326579852849011337, -0.0015513444755826130781046, 0.0027048665593488998779015, 0.0023066132211060101295241, 0.0026774441831202470086504 },
	{ -0.0008826193117991451651122, -0.0008263525668577300590384, -0.0008189459654292026869610, -0.0002360808249240322980421, -0.0001790295975352914931589, -0.0001733820976154474917917 },
	{ -0.0010972471137194669821291, -0.0011197343525763490262870, -0.0010690636482269869327738, 0.0008328803260789950039383, 0.0007338411444818179448293, 0.0008827938244423404494030 },
	{ -0.0017059058537759669901129, -0.0018545250183770749154849, -0.0017105597381155289351401, 0.0020881142482035959864750, 0.0017873313178642540911822, 0.0020965201816624521626020 },
	{ -0.0030207622339226639779364, -0.0029896818435323569210227, -0.0028377893150922858800378, 0.0012367399964644319558527, 0.0010082860844462500707341, 0.0009622186927019188646196 },
	{ 0.0001546795934543260058434, 0.0003115392682397258956525, 0.0002185987186043977987572, -0.0014169727148589609383444, -0.0011384058611941870291229, -0.0011490641746168379043469 },
	{ -0.0011856138827652140839419, -0.0011818472066836170283888, -0.0011339111065632519626672, 0.0002269083239164481919509, 0.0002179491076709681935449, 0.0002333500919849812980673 },
	{ 0.0005476605691684773751307, 0.0004109511982011428924549, 0.0003996234751483727877172, 0.0007614047404865821296396, 0.0007466162899641956346083, 0.0011250904282461649062036 },
	{ 0.0008730355620688473898203, 0.0007076764555494004672181, 0.0007008991977819858292323, 0.0015145982537764040003897, 0.0013618197452185489861043, 0.0019003530864391540364244 },
	{ -0.0015862223639675550757727, -0.0015390083347347090474838, -0.0014611413214299800739787, 0.0010292888344625580280406, 0.0008413127593804404580694, 0.0009689048030852865636339 },
	{ 0.0007174709298185695787592, 0.0006447709812047599338067, 0.0006070177377474189877454, 0.0009635266219904522295645, 0.0008848421864619605056029, 0.0014030621741469449705736 },
	{ 0.0012303468073684490063230, 0.0013567238783342399754267, 0.0012168835474386050649054, -0.0011249026464836169632994, -0.0008882954853171100588136, -0.0007102173321930857225609 },
	{ -0.0011455267412076800947335, -0.0008187025779675543863539, -0.0008744633463017150798793, -0.0016086844859244120182346, -0.0013740687144204889041932, -0.0016578511341657769118801 },
	{ 0.0003130151283674555921094, 0.0002439103491387026908643, 0.0002377364919374548037599, 0.0009704397185094149665824, 0.0008629953275267411615460, 0.0012475682324134319302333 },
	{ 0.0004575716907058021104375, 0.0002303072428355311049297, 0.0002746655491660988262538, 0.0020769967439170718311037, 0.0018204514099012100367125, 0.0024522564945661379232078 },
	{ -0.0001598429570151217097955, -0.0000250161293748954393040, -0.0000756398765644545245570, -0.0006837313553847920461620, -0.0005431823952793275170001, -0.0005261461135388993248102 },
	{ -0.0010914012066732669149066, -0.0011028011967663979217558, -0.0010396157963907660774783, 0.0008794438033744002508954, 0.0007425308385417319625640, 0.0008949405447030036155412 },
	{ -0.0030108703536686899464303, -0.0027041939589042582152423, -0.0026312504880606660088993, -0.0010353833741604170221029, -0.0009736889041963444312022, -0.0015668724161201871055565 },
	{ 0.0017397711663288339348915, 0.0016649942697112220420241, 0.0015607007087060759641106, 0.0000000000000000000000000, 0.0000884861846725340663684, 0.0005005433227250701867794 },
	{ -0.0019294733277864699674820, -0.0020280058201837371581466, -0.0018761145042782310585205, 0.0016638016821012931081153, 0.0014212538776487960122397, 0.0014980737078507679570227 },
	{ 0.0010833922046142969389942, 0.0008679975921877687832645, 0.0008555040386544413965469, 0.0013148805684121760931765, 0.0012037886118439488931237, 0.0017427424073715449496375 },
	{ -0.0016040849330103159036537, -0.0016289922780683540465274, -0.0015293110581309930697674, 0.0009930958134245792580386, 0.0008540310215098643065873, 0.0009010285564139775080911 },
	{ -0.0000730081777051047982807, -0.0002853297620752905864976, -0.0002221483190537993865094, 0.0018589665177656900050401, 0.0016202276950170260445133, 0.0021033336012689480241933 },
	{ -0.0032519523407938981038323, -0.0031297244915311321633578, -0.0029856365059590900083719, 0.0004895366377352847707394, 0.0003595802528637010841289, 0.0000719031716208918338139 },
	{ -0.0016190727517044819095543, -0.0017271796298579640896453, -0.0015975695221955390525059, 0.0014840601410256650406067, 0.0012913216475557629230736, 0.0014118986047853589183010 },
	{ 0.0006045919507561148134936, 0.0004770961602917809748527, 0.0004639104006230356088900, 0.0006582452575488033667470, 0.0006419063745878681327001, 0.0010651925362116549007852 },
	{ 0.0001922166845824074897075, 0.0001157538284570081964925, 0.0001111822892363374974331, 0.0004240285355958888879706, 0.0004460761931450971952938, 0.0006184821756993455401513 },
	{ 0.0005233863074608946681920, 0.0006591146651141886921021, 0.0005529250438929073302799, -0.0017492607540415389382610, -0.0014083667706710520199642, -0.0015694609640983000608400 },
	{ 0.0000450935318551655775227, 0.0001644514947092064899043, 0.0000961409780717695688955, -0.0014535472863260129414847, -0.0011791586559200199334912, -0.0013826752430947850634746 },
	{ -0.0008205752101680824101698, -0.0008254792624073083954356, -0.0007781331062062447250297, 0.0010219855838075431013479, 0.0008651615785671458317210, 0.0010351714301950639112587 },
	{ 0.0008982177517891755998830, 0.0010542594426931539350334, 0.0009357207389488424566573, -0.0008888021773154860302041, -0.0007248356405924389657403, -0.0005875264238090387516888 },
	{ -0.0029912153306780007999688, -0.0025878665858653292046465, -0.0025509116828282460930077, -0.0013630649730190130038826, -0.0012714867441140759694862, -0.0018696862417224629701606 },
	{ -0.0008715750906511916069538, -0.0006948086088689806169225, -0.0007071174230514009305393, -0.0001843808580839778892176, -0.0001895011315432450064479, -0.0001726479890225452059043 },
	{ 0.0009315190694755509735053, 0.0008126535095474056663925, 0.0007937082856951519545946, 0.0011758628537512959624789, 0.0010639038407568850597845, 0.0015697932072793350719020 },
	{ 0.0002080367637398975897908, 0.0001095577134278171024199, 0.0001242664461458689931228, 0.0013152413429877900588605, 0.0011560883147193670977132, 0.0016216633155103660646895 },
	{ 0.0001836426302546013034252, 0.0000440900239595255321592, 0.0000739329037221354926903, 0.0012046433085375640205383, 0.0010827000177008629360625, 0.0014236801510413689495632 },
	{ -0.0002428406776426727956344, -0.0000655391691235291973814, -0.0001368771537349412108567, -0.0011261483925635599936260, -0.0009458470330678382751582, -0.0010883729195651059128841 },
	{ 0.0001116138475154741045627, -0.0000390847866448336574412, 0.0000000000000000000000000, 0.0015519719554246270802611, 0.0013739949085673388955164, 0.0017890351495577060808584 },
	{ -0.0003817822671504954084795, -0.0006579618139868499129483, -0.0005391594558286493411164, 0.0024704786307657971428098, 0.0021225942779689501577201, 0.0024808125308078449351712 },
	{ -0.0001852746212695032923101, -0.0003963515749293646146800, -0.0003204246367126138738193, 0.0018133649053543858965104, 0.0015771101165847380143337, 0.0018665748080011089135577 },
	{ 0.0009306893277715142008891, 0.0009941159673442106527175, 0.0008938181701861798184733, -0.0007017553778914343773354, -0.0005398142278885538448627, -0.0003873610527941550815169 },
	{ -0.0006105546389830629906009, -0.0003705343027970575014475, -0.0004334065597740511177269, -0.0015501792052463540371593, -0.0012875062460961919294511, -0.0014717839632058819700905 },
	{ 0.0013315824261448279668824, 0.0016257843302946010412280, 0.0014132583860236309480224, -0.0031839021092471848223271, -0.0026328635739514541155848, -0.0028616491282911779917009 },
	{ -0.0019239967435183608947258, -0.0017888810904099600954803, -0.0017133637862774499151935, 0.0004076758796838111757743, 0.0002525013756303549732063, 0.0000477660850566708093863 },
	{ -0.0011182278956358619823735, -0.0011560243986606889496360, -0.0010618910388586969961028, 0.0016552847886783089974977, 0.0013546473721209630058926, 0.0014960007423373698938474 },
	{ 0.0001698703320583311066234, 0.0002441597168928357083038, 0.0002046238543141905936659, 0.0002907641115283302957542, 0.0002256097049621461005271, 0.0003762620113254374243535 },
	{ 0.0009115763471165886071973, 0.0007811517537160333818011, 0.0007667268863810341268314, 0.0013021001233661879704917, 0.0011213039584968519102315, 0.0015285331933961639888642 },
	{ -0.0018319897418230799736316, -0.0018588814283360399759459, -0.0017235675129643780734146, 0.0016693393169582449592214, 0.0013227481208686389917845, 0.0013246647479442719749082 },
	{ 0.0000213035167079327602809, -0.0002159461222382827916191, -0.0001342843282405110016346, 0.0022074323041412552068308, 0.0018816819357915640954781, 0.0022310194370285378086260 },
	{ -0.0009715081231583510808572, -0.0011372215128734040109210, -0.0010163626393182080033528, 0.0020075321303166400975504, 0.0016809985703427740479782, 0.0018632920024329719093509 },
	{ -0.0004867180292480605954472, -0.0005944369844357003281055, -0.0005260014759441795830636, 0.0013576558812099250549144, 0.0011460867978613298919649, 0.0013200761088613590515939 },
	{ 0.0002709579476709462748756, 0.0002623579741301002151918, 0.0002342129749338615874420, 0.0003158455463459138045680, 0.0003536661782123434131370, 0.0006474892218974151632260 },
	{ -0.0012606450110511319227108, -0.0011820627528905741036175, -0.0011381951402716760507472, 0.0000441751816447000914315, 0.0000000000000000000000000, -0.0001661391988287807131974 },
	{ -0.0000374361340964863821773, -0.0002025302543977627032536, -0.0001504131507809350039653, 0.0010128185080860580002948, 0.0008909201374429257620433, 0.0010506218805583550281874 },
	{ -0.0000697314559847031755760, -0.0002048182778111016003574, -0.0001431456546450866874030, 0.0022396335543371607978980, 0.0019146631227470729645634, 0.0024076470836917069490157 },
	{ 0.0001134498819047597003521, 0.0003216546529334241184828, 0.0002361971510953769076962, -0.0005445843701118040268774, -0.0004684476685401185920045, -0.0003911024367736657889050 },
	{ 0.0002700124625233709242493, 0.0005479203424187355361483, 0.0004224277946134937238194, -0.0016460557219626829970199, -0.0014116523470710240495407, -0.0015861003534052499526485 },
	{ -0.0009375419585439371096894, -0.0008893052309778558305389, -0.0008475543813546454908342, 0.0008473946991599851270849, 0.0006684254406623439063312, 0.0007701464000471199781300 },
	{ 0.0010802701936692019144687, 0.0009018760595194315072415, 0.0008942586835948150743303, 0.0015315244605254359645113, 0.0013415473691331619796735, 0.0017833781748810199983130 },
	{ -0.0001096450048642154059559, -0.0002563625125084509847294, -0.0001957028984239817936009, 0.0016067490775625689787554, 0.0013730420587507818994816, 0.0016514159849377099947149 },
	{ -0.0022489860483368251213576, -0.0021622723058211511942828, -0.0020551413311335649247713, 0.0005565879365250650610136, 0.0004307635115717253250500, 0.0003071120087576801921565 },
	{ -0.0003810972130591178748567, -0.0003841657690042922165818, -0.0003635375006211518864041, 0.0003335557156171932977930, 0.0003338196199009294267042, 0.0004720095878422362191233 },
	{ -0.0017887546996614329952724, -0.0015535799276170120936102, -0.0015330887736654170462602, -0.0009152751931082718939853, -0.0008416400994071096585206, -0.0011502732996908060763425 },
	{ 0.0002592241596148464155384, 0.0003563852090537524149559, 0.0002902455309192138763023, -0.0011095777897185499314553, -0.0009102520419652287758314, -0.0009897028000172330888112 },
	{ 0.0008095808530511749024489, 0.0006640106475961012359385, 0.0006726823012541013847507, 0.0019010992365345251008807, 0.0016405868807242241034366, 0.0022634074195337131572836 },
	{ -0.0014886750198611590090964, -0.0011766564021064049132270, -0.0011937780206537620605878, -0.0007419522492702712405943, -0.0007425440660297228517850, -0.0010852383796434220391419 },
	{ -0.0020370645644948309760380, -0.0016824275846234549094960, -0.0016822379302275400174416, -0.0010823078445004229511395, -0.0010596127048644260817967, -0.0015820142337018669627391 },
	{ 0.0004362471126011754126578, 0.0004172256669388621735765, 0.0003910514902604838010508, 0.0009016144688144081022207, 0.0007658085758886469746401, 0.0010260128903237129491255 },
	{ 0.0004424279159712113797080, 0.0002274498197232477055604, 0.0002808289637572443137210, 0.0020385248361620878117251, 0.0017500259122323700428092, 0.0022030098297804510780462 },
	{ 0.0013573560979571650496334, 0.0012680948200267890858817, 0.0012050802034674779755763, 0.0007372810200549721184812, 0.0006641992135795534313997, 0.0009866934559985831524470 },
	{ 0.0005102948283059992058003, 0.0007339983497780041155761, 0.0006051120076261113202645, -0.0015066262464190089548355, -0.0012801857756424370876625, -0.0014756004122576760022312 },
	{ -0.0009098748030984850354574, -0.0010871396224865180479835, -0.0009594774619306130391053, 0.0022497525806366739861697, 0.0019004699949504650149934, 0.0022460065378160881492675 },
	{ -0.0006213009990882743284468, -0.0006001490205913725907091, -0.0005762351285626103472928, 0.0006584705255760267940748, 0.0005512068243731345828715, 0.0005907834359415296181586 },
	{ -0.0006052549427549557556696, -0.0005925593476165313608361, -0.0005528157922087424116717, 0.0003914127700510194223307, 0.0003527345079284236058735, 0.0005468647664807870303894 },
	{ 0.0007876992861716985289133, 0.0008643729573848930132149, 0.0007756105165313480036271, -0.0008649894657227848084741, -0.0006881816222935138683756, -0.0007195166971875966224759 },
	{ -0.0008083696712150427194504, -0.0005809451493366798785142, -0.0006101881161850140899983, -0.0014799274950568249830524, -0.0012631976409087769790823, -0.0014815590130897280003730 },
	{ -0.0011910595629617630984709, -0.0009155131361135923628858, -0.0009369305464182379030239, -0.0018598118827900479363424, -0.0016070415423044910521039, -0.0019573118903076381497330 },
	{ -0.0008045049592724764966206, -0.0006915821973546562107318, -0.0006820230549039472999276, -0.0010108579053582100477221, -0.0008355767108841949180817, -0.0009563194170018588955925 },
	{ -0.0009491792393914779682115, -0.0005840196880123602659468, -0.0006577900124697037657906, -0.0010386227271267410923739, -0.0009986121610291223953809, -0.0012238304182152510285464 },
	{ -0.0018041826303596901048126, -0.0016161383070755869998530, -0.0015577461936724630933715, 0.0002625370589122952053214, 0.0001455484436757623989498, 0.0000665046308628195701482 },
	{ -0.0014427768848714649443932, -0.0008697438048934385450631, -0.0009932287472997111243272, -0.0028141250491594128717299, -0.0025528622150169078507120, -0.0032430388791978848050945 },
	{ -0.0013853172786690299677498, -0.0012834914714706490822954, -0.0012256033185056029906346, 0.0005138908895868152639300, 0.0003637621003644311802258, 0.0003181911141959244213734 },
	{ 0.0002225246562174969938708, 0.0000396438694650141527638, 0.0000959857325223499574908, 0.0018625530963337290531190, 0.0016088040809361390040544, 0.0019950011650587220514430 },
	{ 0.0007491253346450648135588, 0.0009580391714325977659331, 0.0008233954204072097904712, -0.0012131811481735819361477, -0.0010478648839098879050336, -0.0011538763050823290512437 },
	{ -0.0002530548385849243266804, -0.0003530030185253433768801, -0.0002925510806284671171291, 0.0015729048385228119183443, 0.0013086831862175940174864, 0.0015682388725301009153174 },
	{ -0.0007656467525351600897365, -0.0008836201375013367055947, -0.0007818931122914662490767, 0.0017821579458938540535540, 0.0014893674917565769867667, 0.0016938478822912180106475 },
	{ -0.0012048178517591089445316, -0.0012211610985105819746771, -0.0011310360458851960105409, 0.0010510641903012190017525, 0.0008573867672939072511265, 0.0009381357957314912128510 },
	{ 0.0007127352782858642271310, 0.0010384840343334259676888, 0.0008607870325100607346649, -0.0023095755217508152044859, -0.0019866235727305828727418, -0.0022952008588924639395645 },
	{ -0.0022790455149729951789261, -0.0021229881010061220011909, -0.0020281336503338151175391, 0.0001121690848687382065446, 0.0000190235738404147990610, -0.0001834521625256206112716 },
	{ -0.0005946782926081860135362, -0.0004950514099964735533974, -0.0004816858543250602197293, 0.0010470576476713929717915, 0.0008181084149425868088865, 0.0010949980938056410109671 },
	{ -0.0026680049264579341027070, -0.0021383009931374269939341, -0.0021673453151323501114645, -0.0019769492377993370599287, -0.0018852341023006709275212, -0.0026007902573880990748645 },
	{ -0.0009391174918480935584694, -0.0005001863818945742438948, -0.0006100689587236341870219, -0.0020462357160396589701723, -0.0018707736008066919945053, -0.0024051158939487008586733 },
	{ -0.0002946182379858313087756, 0.0002146478816644940097388, 0.0000338639153683911430241, -0.0031157194630531070099477, -0.0027695105568368268492141, -0.0034002282195848962040308 },
	{ -0.0011473413537963119811919, -0.0011055476720712540273511, -0.0010366713056614879507472, 0.0011417227932082349515369, 0.0008811345309224682042712, 0.0009345012591774892740659 },
	{ 0.0017545248767711289338694, 0.0017854048045360829208988, 0.0016476305107818970015210, -0.0003910376039202410219410, -0.0003134469725235634104260, -0.0001759666906102430128465 },
	{ 0.0019477457326691840783922, 0.0020057507369279771285153, 0.0018479770734662549942773, -0.0008304667724282335447561, -0.0006651656160599514304443, -0.0005519612383907058399471 },
	{ 0.0006839548449621857956204, 0.0008537815712939605602982, 0.0007512148259989066909764, -0.0009282372242652858632464, -0.0007848135936068544081870, -0.0007265280035770176362761 },
	{ 0.0012191636563182780411685, 0.0013810563794011659465716, 0.0012328471563772349685328, -0.0015919326533360210300383, -0.0013099239624145340814276, -0.0013539872168801580024078 },
	{ -0.0017568316239994759362514, -0.0014166846817317950164156, -0.0014179947525571480037437, -0.0006576796080438116875316, -0.0006739682339913956451646, -0.0008859684399014159652749 },
	{ -0.0035786303278659268613249, -0.0033282299364148560695542, -0.0031825974581225179027566, 0.0013116013742222039188678, 0.0008971823123284548232720, 0.0007391934072405707692802 },
	{ 0.0005007429716416467906440, 0.0007808794039573298826143, 0.0006460790639804537801097, -0.0008167160960076430163343, -0.0007717415235169588919387, -0.0007044713388895623793623 },
	{ -0.0026260430946925279044324, -0.0023546180689844619683682, -0.0022761864564217128455159, 0.0002815692859841798195535, 0.0000457298480774646410631, -0.0002863175108839552743256 },
	{ -0.0015280879092446459591864, -0.0015054267047100750465810, -0.0014119928917652759241957, 0.0014835945581903209767455, 0.0011309284689691040132892, 0.0011685912637022630080480 },
	{ -0.0029445952241692390990990, -0.0027141414603778159404801, -0.0026089164389802637841753, 0.0004971815285402845325732, 0.0002228227988007042007293, -0.0001106090663071182951223 },
	{ -0.0012026221291612799579179, -0.0010552578488728489494036, -0.0010268179719255530243233, 0.0004130746575277921247074, 0.0002521257412622952137626, 0.0002127587143130512119422 },
	{ 0.0009108675022773024239700, 0.0007139416149967952977817, 0.0007332819009090007361662, 0.0020273737106514980003957, 0.0017356333103007029819431, 0.0022266816637823540550589 },
	{ -0.0020116558174672330081001, -0.0016506843111199940524886, -0.0016573665979802609717758, -0.0006750712409340252958406, -0.0007424190746794388697655, -0.0010113954740342170467854 },
	{ -0.0013618804097268500958445, -0.0009241035250102016661983, -0.0010006766150596670822415, -0.0018296931527618299530291, -0.0016994903569878439149321, -0.0021888105597133358098949 },
	{ 0.0007000869667320254235640, 0.0006294066462182024276548, 0.0006041936906839660142987, 0.0010307858072943500669927, 0.0008424592331716066351960, 0.0010994193479807350729421 },
	{ -0.0011138600035346549275289, -0.0005949825125839204890618, -0.0007175690623996686980102, -0.0030437369467554349113103, -0.0027009617133900432062177, -0.0033414524412961640713327 },
	{ 0.0015813132910809800553420, 0.0015446030281505850230428, 0.0014500812222404129044062, 0.0006017488150750933961763, 0.0005418202436628363673904, 0.0010529991764452930474993 },
	{ -0.0006309651477670098242145, -0.0002737530004930015877125, -0.0003704569739397593178439, -0.0012972008653356278951030, -0.0012182010446213729221165, -0.0012656574445291020065280 },
	{ -0.0015426083192796910103367, -0.0012607188821907749400053, -0.0012544464318083089343636, 0.0002835905084795556753766, 0.0000998888326605441417274, -0.0000145469708807764793888 },
	{ -0.0019075774749787229467995, -0.0014583437824042960479637, -0.0015118084936512699370526, -0.0015716640928237369384535, -0.0015282547095667111010375, -0.0019364860782940889437354 },
	{ -0.0018122830428439740645119, -0.0015233676821948250162259, -0.0015062019215721939718877, 0.0002339809403209100100406, 0.0000335168650754216721778, -0.0002523265226978650081074 },
	{ -0.0006276755234303359231099, -0.0002562515321953112950974, -0.0003567337134297149099475, -0.0015521938819111670299772, -0.0014182481099607169554994, -0.0016059297909405498977486 },
	{ -0.0016592668118195900203021, -0.0010993556682741159429006, -0.0012034378786402169167480, -0.0027117253921550951885344, -0.0024902000268447930207694, -0.0032115926606287141520379 },
	{ -0.0003666298847084047238427, 0.0000111208311845899600784, -0.0001088927312909188934538, -0.0019248682317693410672582, -0.0017482255375687759044179, -0.0021408235518549020848234 },
	{ -0.0019899892189565661533568, -0.0016859950332613329523901, -0.0016770003504584839529312, -0.0007076488305839389323162, -0.0007589993516504180961699, -0.0011830641955303329744542 },
	{ -0.0006178001030363142360990, -0.0004821482943602286891545, -0.0004874985348797025957839, 0.0008918700230818880827360, 0.0006734263829171832855050, 0.0009827596096081683482715 },
	{ -0.0004859812977409835822382, -0.0006336307610201815949252, -0.0005484012405515168667863, 0.0031400864309557869474832, 0.0025777730617373138290171, 0.0033099693727644081617245 },
	{ 0.0018986206757394990304183, 0.0018009476758447239431310, 0.0017096981486406038954623, 0.0013112996288860089520723, 0.0011090615685716630027524, 0.0017502210275975619777067 },
	{ -0.0021238172216733329673855, -0.0017741874354883129319416, -0.0017788350407157509612543, -0.0004020935750389998840013, -0.0005521435555509480747755, -0.0009235205530381978590582 },
	{ -0.0003713816518527947242896, -0.0002310898405663897091453, -0.0002609082977041852135711, 0.0007282938429235640389117, 0.0005228566146882873948848, 0.0007091582244476002513597 },
	{ -0.0024412056436566710419800, -0.0019479895945992090761090, -0.0019843577651482190507548, -0.0017778785488411651041080, -0.0017145206276966099435272, -0.0024571929039612317932639 },
	{ 0.0004753808229293167235928, 0.0003087061108088232259938, 0.0003385765467625560126187, 0.0024832261730448838138452, 0.0020740200603222101044532, 0.0026040736841392089773173 },
	{ 0.0009852506327237752978276, 0.0007454915030557894431787, 0.0007699936690447049033453, 0.0024909617818619110216072, 0.0021114353244193330254308, 0.0026507774416359860224224 },
	{ -0.0023865531707132959550466, -0.0021864939749232491922326, -0.0021003385596836720086711, 0.0005741229633157592365106, 0.0003476757645966598920563, 0.0003123888879749890895403 },
	{ -0.0024325609464292022136866, -0.0020976400028182638594909, -0.0020593740383311929208199, -0.0001592365935653621087514, -0.0003201819723011692137189, -0.0005192204326984960986030 },
	{ -0.0015510472835291329567892, -0.0015394159863473400994688, -0.0014547614939133650131114, 0.0020311194309942420983817, 0.0015754480442211229833382, 0.0017278734265286730537037 },
	{ -0.0024552573476186441252256, -0.0021862801785206219772062, -0.0021249297961848261952211, 0.0004690646017837567173688, 0.0002041219834181202995652, 0.0000667657725451416300027 },
	{ 0.0007143622572079917371762, 0.0005655668647502160809731, 0.0005534677889237814242193, 0.0023839529201335760491554, 0.0019564444384464908105459, 0.0025284151056995749634604 },
	{ -0.0002681204076221031188526, -0.0002483286050417672012440, -0.0002581821917367714005163, 0.0016502157207592879888808, 0.0012988585879326220318825, 0.0015266173347411129447571 },
	{ -0.0020161166434123308811854, -0.0020446166146496800794519, -0.0019228053522008889831413, 0.0023479633491046221667464, 0.0018294073834367789808258, 0.0018123222106573909449129 },
	{ 0.0000365923976180691578426, 0.0013093097576917398991436, 0.0008187187333879214273455, -0.0097789572142908103707937, -0.0084206359864102825474319, -0.0098540758470047670053438 },
	{ -0.0003096334232197745146321, 0.0005618345014997006958007, 0.0002432676209279321997807, -0.0064108273979269856518437, -0.0055430727187143037995587, -0.0065178153893624343750979 },
	{ 0.0015886521043367110052336, 0.0020808069691323318253784, 0.0018067482987010230650882, -0.0046947056128845993799859, -0.0039527751540999909660168, -0.0044604603399899332011325 },
	{ 0.0003845630169118222953240, 0.0000637266813853891716125, 0.0001813089680772335897906, 0.0020506293300077719707342, 0.0018373271393969960434828, 0.0023223935210277069432494 },
	{ -0.0014040572673615070702774, -0.0013467946875697460462629, -0.0012891899299368749474970, 0.0014429237034057869944886, 0.0011147992655922049978767, 0.0013573227218083290350481 },
	{ -0.0025589838056449499756329, -0.0024931285345798541788764, -0.0023703111166101420610730, 0.0020565578022868138963197, 0.0015794040679047859250528, 0.0017518498239356510275816 },
	{ -0.0003808429463518274112922, -0.0000785837120213117866540, -0.0001960395539192633877074, -0.0006794982214676358479172, -0.0007216359525594789794262, -0.0007062046932689569882918 },
	{ 0.0001784122127052661054263, -0.0000947648158955391889955, 0.0000000000000000000000000, 0.0032975940471505049006451, 0.0027329976523511449217063, 0.0032783565315517501637110 },
	{ -0.0033507673900023391723446, -0.0031281596977924489705325, -0.0030217032376112138401802, 0.0013271816205078549276430, 0.0008850796614717826088156, 0.0006122554028959784327285 },
	{ -0.0005891595973511394883013, -0.0008991434599041504957193, -0.0007478020519544399842549, 0.0037601774855954531347202, 0.0031098805621187532166605, 0.0036060945923666581219846 },
	{ -0.0024349584789954141668400, -0.0023786906325728210918891, -0.0022691642108127550031549, 0.0017374848675855230316356, 0.0013020336267530049304680, 0.0011666542985212609209039 },
	{ 0.0004676284773673564077422, 0.0015594783399731219127732, 0.0011061867201690510180423, -0.0084191162751336527658719, -0.0072684665808071102083288, -0.0084112901199653762607245 },
	{ -0.0006548734290150868457905, 0.0003226729247531248006599, 0.0000000000000000000000000, -0.0069986428747338298717029, -0.0060964339725920121959568, -0.0071518221347035906831913 },
	{ -0.0010943614487813619739337, -0.0007359535534324243653131, -0.0007908864842704413236976, -0.0018801087663512679442473, -0.0017335457396955810099176, -0.0022085703627724279606015 },
	{ 0.0001577290704574733047616, 0.0003077603132798277193495, 0.0002561921478334116121338, -0.0009585676125693316437792, -0.0008420542662867818621900, -0.0009216904295747789261500 },
	{ 0.0013111146425135130396539, 0.0014294689869007689338870, 0.0013129635755944669452122, -0.0012618168087457060609624, -0.0010823724500036979983414, -0.0010951237182985640045835 },
	{ -0.0030870459650259108111414, -0.0026635714517828619832263, -0.0025979123780817039296442, -0.0020893354905855208818177, -0.0019223954456361249266277, -0.0027014266705419911172859 },
	{ -0.0004318805837267865170050, -0.0000414367423442805503440, -0.0001332641306211793086854, -0.0029725990628112599137300, -0.0025679834103135871711809, -0.0031231721866911608086192 },
	{ -0.0011013480511063769833741, -0.0014371242184658770263406, -0.0012099563211037710555584, 0.0029265036051418840887250, 0.0025359694841251031842411, 0.0029877675660930050512143 },
	{ -0.0023618649366968949392109, -0.0022893456247078400669914, -0.0021592033123428960687817, 0.0015659858140432319009716, 0.0011988054929236240629675, 0.0012593406750085419688878 },
	{ -0.0017053513952728360791866, -0.0015033805030767340409764, -0.0014867561174040689903758, 0.0005313176483673376923958, 0.0002749246245461851260311, 0.0002180952725957057108507 },
	{ -0.0019736218916827871251718, -0.0017626468937219329224941, -0.0017218261131027119605758, 0.0006435060375103929730320, 0.0003836628357856497820166, 0.0003882307350980862851661 },
	{ -0.0004584309761153051890163, -0.0005734990501886180107663, -0.0005250848485371839742541, 0.0025556306686790640839146, 0.0020463782250203631923879, 0.0023802080212366278841318 },
	{ -0.0017994516681856030735492, -0.0017131879882915960425921, -0.0016306741939640669557254, 0.0016976671673990069086346, 0.0012780139819298899812872, 0.0013427656545337430648440 },
	{ -0.0020812755756314600848189, -0.0023183010810579008850740, -0.0021140133611187218211358, 0.0040235974135091378059426, 0.0032487769278422280565255, 0.0034947174816767180918764 },
	{ -0.0016502860293951030455639, -0.0012541308221572459308257, -0.0013021517339433870581733, -0.0012318197024573439117556, -0.0012312654476988489637979, -0.0017282132286552060254636 },
	{ -0.0002515746088883503983613, 0.0005603474007775169612317, 0.0002547350568113017767018, -0.0060687048996008151335868, -0.0052774525025628344146456, -0.0062359958415595965683376 },
	{ -0.0022988401246208621520051, -0.0019806839150989649298018, -0.0019404372737891139276412, -0.0012848836776706579893004, -0.0012514732269207180185427, -0.0018785116264185069360343 },
	{ -0.0003783845472361250139755, -0.0002554201816002749829762, -0.0002646027530290774739362, -0.0010494108037390310287501, -0.0009117334799214422882269, -0.0010794078452169249878606 },
	{ 0.0003399219527510174846188, -0.0000914358144281747969645, 0.0000692155858007202493725, 0.0030086086998288739599583, 0.0026696330918345979009432, 0.0033586406863198177939756 },
	{ -0.0014081673793807489205449, -0.0014296557239144539342562, -0.0013420541249896309182049, 0.0024895875050117101043301, 0.0019716246479401901628425, 0.0023101814471465911479675 },
	{ -0.0002921372881705638934781, -0.0005835927859749219384980, -0.0004677735291868678779194, 0.0041625215706740459689628, 0.0034440762945795590048437, 0.0041521416184640506236159 },
	{ -0.0005590972461393574214142, -0.0005584146977017768022702, -0.0005448017723241436885764, 0.0013689075445938870666945, 0.0010504054290390060603932, 0.0012343850775573741041652 },
	{ -0.0026719378786988750434928, -0.0026495961052645871335043, -0.0024979388866469900976064, 0.0025216575038225809424208, 0.0019155673781491679760064, 0.0018676924185740549806678 },
	{ -0.0002139148490026234925802, -0.0002756093839020705166948, -0.0002624256062474601062899, 0.0016363639923476279523090, 0.0012944820066632339947837, 0.0015040683411485799864798 },
	{ -0.0030145718695161480629729, -0.0028743289762984650277666, -0.0027461227903242879308521, 0.0015271393999162679103176, 0.0010615008205464649196359, 0.0007572491274283738287715 },
	{ -0.0011228415441616090755111, -0.0013424732983782620455993, -0.0012004381421622651020947, 0.0028409269330524210446509, 0.0023212427058909071951598, 0.0025309942340999091146825 },
	{ -0.0017219828231553270438819, -0.0015814569077836980025981, -0.0015241166874306189948224, -0.0004493559502069876068243, -0.0004841681538139194903789, -0.0008335258624689379555045 },
	{ -0.0013590041604333909924796, -0.0009380615623552050622372, -0.0010047995204403210606908, -0.0024382836385350888097623, -0.0021997469299230971127856, -0.0028414937418735049592944 },
	{ -0.0008676332942064641281371, -0.0008711715659867528107133, -0.0008002321371404204439909, 0.0001269791418468583068038, 0.0000719520233494083357503, 0.0000000000000000000000000 },
	{ 0.0011202965227937909057504, 0.0004274266019960370143436, 0.0006324544946366567879301, 0.0045819118104896685780836, 0.0040589942966967529000555, 0.0050959695199926423672676 },
	{ -0.0025717533213996299733262, -0.0027038551637547790680582, -0.0024908594941901208985602, 0.0031455503186190651547516, 0.0025295402887435141682515, 0.0027796599551870790345554 },
	{ -0.0021679027216922990006776, -0.0022979031964529611151149, -0.0021196029388560791960339, 0.0028445573043386260768395, 0.0022808409716000991110385, 0.0025069574079919459790033 },
	{ -0.0017397268289353570204503, -0.0016211668132974370488963, -0.0015783429268114220356961, 0.0011212738312200709600575, 0.0007446902392025206386325, 0.0007236390671549373064531 },
	{ -0.0029107832095982459977290, -0.0029487348719633870754164, -0.0027524714168534851477244, 0.0026247689249882899358013, 0.0020492934428704609982341, 0.0020768648097496868881739 },
	{ -0.0009767248380944388956582, -0.0009091957587697532035959, -0.0008978789066016926558381, 0.0013863613420200680999284, 0.0009963454604736930730891, 0.0010855774454822330297998 },
	{ -0.0036210361235980651622990, -0.0037120368417917588496624, -0.0034512056331063409482518, 0.0033954843457376349645516, 0.0026860869632734391682094, 0.0027127836630580228448328 },
	{ -0.0023706542977016738932283, -0.0022158068338177269555878, -0.0021419594991173010120611, 0.0006476796202395854575376, 0.0003524337123597033757071, 0.0000000000000000000000000 },
	{ -0.0035158840624430289824309, -0.0038852084479618670571910, -0.0035318674660319029044719, 0.0054064459678130130648732, 0.0044141938216661191080048, 0.0046700331002220074258502 },
	{ -0.0014155324147138909941790, -0.0010462252563956280556473, -0.0010983071725842849433036, -0.0024243573057892461690421, -0.0021803689288272688068249, -0.0028489420253913311058269 },
	{ -0.0018420523902757291075105, -0.0014200645491597720885407, -0.0014548786397755720904701, -0.0027006831565261728289595, -0.0024072048419801401510087, -0.0031721973258572911009978 },
	{ -0.0001531482716678027062446, -0.0006550169889203378897555, -0.0004361438541158987772066, 0.0037292502135358148243616, 0.0032799217278086790876290, 0.0039529569170674268632770 },
	{ 0.0015738770296931450798361, 0.0009104406377957379803770, 0.0010737540002648230004489, 0.0043024704913545821008403, 0.0038128270373485729886898, 0.0048117608640330907301963 },
	{ -0.0020441569697211511326407, -0.0022804812494454929083787, -0.0020606381948836570081052, 0.0038210920450088689190316, 0.0031246735335007970457699, 0.0035559978581444560800928 },
	{ 0.0002417513160343896132850, -0.0000764771055985642554285, 0.0000143887600273854602780, 0.0036441493895422801159445, 0.0030364824055550240951284, 0.0038069522690375888138159 },
	{ -0.0021500597489655191157931, -0.0020796611354744090442714, -0.0019900273061621569399560, 0.0012668974954288959432896, 0.0009078281345247020970093, 0.0008765691230919605636887 },
	{ -0.0015850725533906239936299, -0.0019335722632463510882450, -0.0017082107504879629443495, 0.0048483064843317623393570, 0.0039906886271234339597092, 0.0046142470635326020625411 },
	{ -0.0034736990270800610809687, -0.0036204157159253919778807, -0.0033735854229733361689392, 0.0035692006818647359182861, 0.0028140672216892138185473, 0.0028766338398886408082855 },
	{ -0.0022694386938514120137522, -0.0025856900718748570579264, -0.0023334730096862088370713, 0.0046591156130944087060408, 0.0037988954498625539420165, 0.0041673494303184135945095 },
	{ -0.0033987577262260698120200, -0.0034916851393936910487314, -0.0032568062210508300947931, 0.0029177147471054600338936, 0.0022729561921691191807293, 0.0021383918972247149999089 },
	{ -0.0046743084142346381926192, -0.0047663852751846768479593, -0.0044355141851265796407167, 0.0036926307370815599807889, 0.0028874134803350578852565, 0.0026891028217876059187852 },
	{ -0.0007431602557719899386154, -0.0005399103657225157474817, -0.0005670694261569608632736, -0.0018296701993817769682504, -0.0015939413714756640091247, -0.0020021069439728560700187 },
	{ 0.0012918773527457599881174, 0.0005868202050591512413183, 0.0007850256722617495906344, 0.0046049028882530406922569, 0.0040743069645148106625032, 0.0050655365579366021244745 },
	{ 0.0011412682697056590348100, 0.0006132877227651710753606, 0.0007582315385045224932770, 0.0032827224512035021487810, 0.0029331948194837378918187, 0.0037779356705869319640878 },
	{ -0.0008131119261110921533214, -0.0008968285662828798677354, -0.0008310909052141974770467, 0.0020191815492378991855893, 0.0016092734679453500439006, 0.0018945513315777910094001 },
	{ -0.0031826605338225728673851, -0.0030716347693670168796098, -0.0029261209760591058715029, 0.0015031343807721069243816, 0.0010576565087261030444210, 0.0008068460325291243115386 },
	{ -0.0031234629580637641999008, -0.0032152500319010328226621, -0.0030063024292507380119677, 0.0031347951862783630765097, 0.0024507135622226340967988, 0.0024224994480990490186567 },
	{ -0.0022194666931172309445874, -0.0022948873138707481973697, -0.0021491327029365971633401, 0.0025853168109850189863852, 0.0020135616251375199579898, 0.0019591432539994178482767 },
	{ -0.0026202264625522029423133, -0.0028984228402447881345338, -0.0026375451063136638556184, 0.0038277213624812117878649, 0.0031166072144843460535457, 0.0031650482292512258086781 },
	{ -0.0036371751554040787982047, -0.0031177578954040188058405, -0.0030782547199450938785192, -0.0030159896465786549361876, -0.0027564341803718310670857, -0.0038575476012718459134510 },
	{ -0.0023078797795802190491454, -0.0017740387014746209284888, -0.0018199961721526549188188, -0.0037655018402223149921737, -0.0033134910106546130482974, -0.0042418215344548091880195 },
	{ 0.0002584148129277231802081, 0.0000504363601346793780696, 0.0000969944451023916980829, 0.0024967998915958211116062, 0.0020913464427819109664464, 0.0026782370763835188463842 },
	{ -0.0018395615113646710675477, -0.0018421383352988129119415, -0.0017495804358822319296562, 0.0015485474900277310106311, 0.0011668827730615129699421, 0.0012538100831221410651900 },
	{ 0.0000537659390952370571238, 0.0000000000000000000000000, 0.0000000000000000000000000, 0.0017048253513663939699707, 0.0013718531567732529345693, 0.0018198168062338170459374 },
	{ -0.0033336724444753000791419, -0.0033659416030149881017552, -0.0031598668711776719923712, 0.0025152937813309240479287, 0.0019131585383727659453734, 0.0018797819271416569240907 },
	{ 0.0001722405167093660942682, 0.0001663469889517865880121, 0.0001387081332738089004160, 0.0012297361272772730760722, 0.0009598590786319756863049, 0.0013344778259707799818873 },
	{ -0.0036469382719265370194905, -0.0038650667548625378296123, -0.0035685543512905780472177, 0.0040721471399076183961263, 0.0032365100682410609736483, 0.0032693868528795820560529 },
	{ -0.0002200098314012205881051, -0.0006035481039698302246563, -0.0004601882902842264989537, 0.0043693596195326488834665, 0.0036377611390950729056137, 0.0043463735048463762386528 },
	{ -0.0011693411744305530746563, -0.0014563032136141100623267, -0.0012853458848434270653249, 0.0035231562656710519419734, 0.0029158655481365419216699, 0.0032432900158610609639420 },
	{ 0.0000906553035349058335081, 0.0005674194902927704635756, 0.0003794088273571780177765, -0.0043117785496885052687333, -0.0037112595660532318037061, -0.0044044557894874856313061 },
	{ -0.0024190702911834950118719, -0.0024926893709437390224404, -0.0023160975203645722125811, 0.0024305898873436581134055, 0.0019304549717134840500915, 0.0020765088780190319589503 },
	{ -0.0029452067329394939453058, -0.0028280168219310818263079, -0.0026926433362574240214238, 0.0012805156957430739120968, 0.0008887142765307666850139, 0.0007400928533514688351103 },
	{ -0.0012783021269053749212341, -0.0015158873632084579347740, -0.0013597506022454929064391, 0.0033410603459344671330167, 0.0027449146454625221805412, 0.0032350528818024569879808 },
	{ -0.0011215174749725290159752, -0.0015585618486223769881688, -0.0013367503666667150501179, 0.0051171049833945991824891, 0.0042492604636152090866186, 0.0050056350098275142687654 },
	{ -0.0014498017798581259042018, -0.0018305040213401429192996, -0.0016153446610770679815750, 0.0046066542525918224154635, 0.0038098117914103788242408, 0.0044049632401979591997399 },
	{ -0.0029665722247612241746495, -0.0031523994513650501099467, -0.0029029928839027158496422, 0.0038070128963751180377340, 0.0030370341596757349615643, 0.0032104703996270380489397 },
	{ -0.0020704364309577678870955, -0.0023283403963248018933574, -0.0021199811562083669368162, 0.0036533212572357719183369, 0.0029635762265940329175662, 0.0032024892949643179312913 },
	{ -0.0029584216101919609022419, -0.0030464686085447337943188, -0.0028373860132166360201178, 0.0027771832322236612161648, 0.0021759961571996130555440, 0.0021200431402710212015761 },
	{ -0.0012912890975055109788611, -0.0008481450428869597357381, -0.0009345356011979705397608, -0.0036914218980426561494967, -0.0032151952745675259522085, -0.0040473900910163174149403 },
	{ 0.0003361181279378313863787, 0.0009995815697627038511064, 0.0007219658750912044458889, -0.0060502771090579767973572, -0.0051769200990954894123219, -0.0060891857952184956503938 },
	{ -0.0026753282892497420343558, -0.0026941163105847769528023, -0.0025226907844210608340696, 0.0022783360508570110400384, 0.0017989402079226400003181, 0.0019165141438520299187187 },
	{ -0.0034937614551826599688933, -0.0035076433670668170049234, -0.0032853444282638219128834, 0.0027713394522403279190970, 0.0021684024624825629264102, 0.0022409196542358782167081 },
	{ -0.0017016436637593639578936, -0.0020065801370658300356586, -0.0018010398369086000856798, 0.0037743257597494870962673, 0.0030710592862560870917876, 0.0034591663127155041629335 },
	{ -0.0033486568606525949666486, -0.0031440683916315109378004, -0.0030214750395452069815783, 0.0007244604968926305699553, 0.0004247612512280357242520, 0.0001158414152120348027900 },
	{ -0.0019299295225535788994575, -0.0020871977596399401977401, -0.0019262449820069689303270, 0.0027143847786191060630911, 0.0021460322138473859540642, 0.0022846307675340230512739 },
	{ -0.0035919225621464608878830, -0.0036420241678083711475700, -0.0034036945107015301875686, 0.0028457501606200508691147, 0.0022446234586052829333702, 0.0022043060197668249561298 },
	{ -0.0027242562180263959689674, -0.0029276276114061378780251, -0.0026828801423583229467107, 0.0034848625479151048092707, 0.0028399913174172318648669, 0.0029619701602470729570604 },
	{ -0.0013734408186761349907751, -0.0009576678737534808543222, -0.0010372898197197270419440, -0.0035606198999132278142088, -0.0031233168116513339296336, -0.0040343767888054793760433 },
	{ 0.0012838724030456260920274, 0.0017539922753905120515056, 0.0014847670553772300463397, -0.0051317524522364859479451, -0.0043172343247749930122970, -0.0049670071738060032895401 },
	{ -0.0005196194264236931497150, -0.0000922861307830985337902, -0.0002286160817992469900580, -0.0041335305483209136903633, -0.0035536767099112439097230, -0.0043341272604543214230777 },
	{ -0.0049009838674991239998446, -0.0048457791270236994299325, -0.0045609201435717295833316, 0.0026019367210632768014356, 0.0019676248034072880283007, 0.0017412788742506429828794 },
	{ 0.0004055889621593086811872, 0.0001949254041766346873775, 0.0002304548465539712912356, 0.0023773958567055988519434, 0.0019834854594289229659521, 0.0024911461671869149919756 },
	{ -0.0049125465857738264199850, -0.0050160171933104433922290, -0.0046739758184330239612625, 0.0038412184743244260666517, 0.0030091282663297349783682, 0.0028676459555601299840355 },
	{ 0.0008547511268408425734802, 0.0004277091864871107140637, 0.0005169096510696848936414, 0.0037190811684841649370492, 0.0031736189854467749746347, 0.0039249410515440662575171 },
	{ -0.0035272731580276162066667, -0.0035873919968811211582416, -0.0033520301673714450289843, 0.0026209321602416279888004, 0.0020505246893845210064133, 0.0018865611762733580300533 },
	{ -0.0006091884943242398351565, -0.0002437021869926577052803, -0.0003457528538617650991360, -0.0036401054044653160905731, -0.0030774981475762959275133, -0.0037523906717216088703426 },
	{ -0.0015004843298650940696531, -0.0015730226962069489542867, -0.0014689946203429850388883, 0.0020485150680821981837576, 0.0016280300331330270664776, 0.0018679534282315800753499 },
	{ 0.0003766269566729421136402, 0.0000000000000000000000000, 0.0001103150653448519964792, 0.0040082103368080795879891, 0.0033756980460475840416512, 0.0042313696310499948269324 },
	{ -0.0036798489479534911977820, -0.0036516232733952340774874, -0.0034371460580368230416926, 0.0020490918333147859452248, 0.0015380279671715309849217, 0.0013651614441960620294214 },
	{ -0.0003699154416653849947183, -0.0007374887687998769609382, -0.0005891516024350986575550, 0.0039919848564650070588122, 0.0033343509727186779884045, 0.0039232599273006703993749 },
	{ -0.0030530000494704708467741, -0.0030449134743609669595532, -0.0028635364737340691931866, 0.0017138421383392950236857, 0.0012806150600902729570080, 0.0010728418155665140048399 },
	{ 0.0006956392008078019851061, 0.0001473892708225835916983, 0.0002975897385171329860912, 0.0048327034603671157816929, 0.0041228002329592190050578, 0.0049918523260748569672640 },
	{ -0.0001864801595324078089654, 0.0003646165758580110767377, 0.0001680358673049665873579, -0.0054541485005307433972521, -0.0046309297913587491382637, -0.0055359070987668346219834 },
	{ 0.0004601877218294253102208, 0.0007934945379517955796334, 0.0006418347080083470265061, -0.0039265607952430449725800, -0.0032670894656050808967096, -0.0037493860192172340922712 },
	{ 0.0010298522988569280747079, 0.0005387660241828826773736, 0.0006813789557844638971815, 0.0027816012397224648342031, 0.0025168808977086131600798, 0.0031971041305456551068209 },
	{ -0.0032279486321802062023800, -0.0032629938333919128283511, -0.0030506887148544249710458, 0.0022489705730812520334538, 0.0017412256254727869527604, 0.0016565182982436830582545 },
	{ -0.0010489253933226349094210, -0.0011595616146440139958285, -0.0010598277831017539635933, 0.0021777094894169521724359, 0.0017500212173904739366287, 0.0020286425726195769607407 },
	{ -0.0014998742743299910140653, -0.0017838987513670169059860, -0.0015894817049231959975308, 0.0038559381687031531703302, 0.0031479130987137559326283, 0.0036064984693663848167033 },
	{ -0.0021437356889130818743405, -0.0021161790181506707868297, -0.0020040381328854531620132, 0.0017871242800719491002764, 0.0013232679055357170899709, 0.0013022058549229879442083 },
	{ 0.0002856030062415768045435, 0.0000371861715889637198772, 0.0000881080878172362750439, 0.0028222593535424220397012, 0.0023693405991666981885713, 0.0030528081717337250677868 },
	{ -0.0004555015880545655745058, -0.0007401961688875493845102, -0.0006132753211885409727611, 0.0033449279218861001534979, 0.0027457507507009978192647, 0.0031590209348157228912146 },
	{ -0.0010660764590109249750488, -0.0012293102456639650333498, -0.0011149329383855959094834, 0.0025758521999832890460314, 0.0021200685836965530231546, 0.0024879500429256990769178 },
	{ 0.0004796935755871909769221, 0.0002321299440219682893357, 0.0002883236984469589032735, 0.0025532740042362839748280, 0.0021289371401789799857773, 0.0025648121436936299928633 },
	{ -0.0017247713347728450220681, -0.0012715614727734209348647, -0.0013252501872451760006610, -0.0040132996369961369473311, -0.0034524708473269308597886, -0.0044771823274093345765290 },
	{ -0.0020985014826166068467594, -0.0016966115066982170173082, -0.0017123295184626600214800, -0.0034430655177589059891263, -0.0029826437323950049859689, -0.0038415380262124439181282 },
	{ -0.0001042023767012595033589, 0.0005011097381157767404955, 0.0002839802777596599735188, -0.0057803825104216777219790, -0.0048793424729467512607206, -0.0057860838035366964071660 },
	{ -0.0009940862447058184704629, -0.0006251267437702030240962, -0.0006958165316756039680060, -0.0037460607711729859876926, -0.0031448239379948277928689, -0.0037500332633733659236375 },
	{ -0.0016291261759859370326919, -0.0008771558898420620279876, -0.0010407366546007590195344, -0.0063034955554683406997452, -0.0053682438198705733148342, -0.0065470641637541326582350 },
	{ -0.0016903840860604680673002, -0.0014076848366815289392839, -0.0013908119504129040199664, -0.0030003556570546699097668, -0.0024920337423749838040576, -0.0030374006276708671002440 },
	{ -0.0011696606843490109373107, -0.0008712390128762728539261, -0.0008886817391036171739221, -0.0032589571474585092049348, -0.0026994742071751649283018, -0.0033180406866579119189165 },
	{ -0.0016917166760168210904586, -0.0017089213986347889882161, -0.0015695860416905080183397, -0.0004519355008447335135882, -0.0003019542288546764232521, -0.0003764897249336207076496 },
	{ -0.0007758769411374011890428, -0.0005644965286053163630239, -0.0005688609492935706429123, -0.0026099926427468121518938, -0.0021188312207880799324655, -0.0025284852770047429489086 },
	{ -0.0018969348640021420435847, -0.0021203755144892011061386, -0.0019347986014112909369106, 0.0030053760409070079569205, 0.0024771914356087817998819, 0.0027905976308345019527846 },
	{ -0.0007834041982856090923296, -0.0009393553470510911064181, -0.0008348493486379803853181, 0.0025513247228017201656547, 0.0021154169843932459083025, 0.0026262568165657407953661 },
	{ -0.0019378238952202130399027, -0.0020349683192196349468261, -0.0018988052289885979859174, 0.0023439761809170779789657, 0.0018187574771075030836964, 0.0019546098913794450295522 },
	{ -0.0014615907952920529407764, -0.0016717935302313188979106, -0.0015008497834246659150687, 0.0033929211459776580442349, 0.0027859108030333559063996, 0.0032247144596988328718590 },
	{ -0.0008308164641492331086922, -0.0008758249709289266495341, -0.0008240259471657721771637, 0.0016616270736892560250936, 0.0012837661873768390751599, 0.0014902659475236120379532 },
	{ 0.0026375695112723831353696, 0.0024197914147287709861478, 0.0023246265927282998113579, 0.0013087360327814750310393, 0.0011838050078305749606561, 0.0018306769947823240546336 },
	{ -0.0012157789622715469909919, -0.0016980421130922650208017, -0.0014501295101430609121168, 0.0049888295358417053884659, 0.0041469415068222702583367, 0.0047715763030157743937454 },
	{ 0.0016914424289898599435694, 0.0011951239516877439940379, 0.0012635586641195899627182, 0.0036017425568559391889367, 0.0031247186015101418958551, 0.0039441389203632437610025 },
	{ -0.0016349219174574040024106, -0.0012617104536092969141459, -0.0013017013002870119961485, -0.0034144178949213057835954, -0.0029952147953405789358727, -0.0039535376478032364239157 },
	{ 0.0013068938338568199079848, 0.0014927892150108799853431, 0.0013502806384226430196083, -0.0031761051407004112050181, -0.0025578396868961429493072, -0.0028273526254020539191791 },
	{ -0.0020875195029827621311103, -0.0016904306206323629208077, -0.0017057992432415510636323, -0.0032737817951836198186732, -0.0028445010066942512036436, -0.0037072762451544351812449 },
	{ 0.0009169742584882494473505, 0.0016680493636780579450735, 0.0013425285889843889888806, -0.0076209393970213092431298, -0.0063656259538178248252871, -0.0073210300947609049243558 },
	{ 0.0014660626850556419519672, 0.0018074322314876210479606, 0.0016060434856205740589835, -0.0046237410675073553970038, -0.0037421065113624600063924, -0.0040789114914245869561449 },
	{ -0.0022610081945373078186889, -0.0020057983619101269097407, -0.0019391148603294979923389, -0.0026139906723209871408642, -0.0022149983065782251866194, -0.0028901305480250401476472 },
	{ -0.0015383225102028239526797, -0.0017080448345978029023318, -0.0015313212497867539820667, 0.0008198417918394476080687, 0.0007391116558356660261667, 0.0007168478149328521599470 },
	{ -0.0027507717341807079274030, -0.0030449654802747251093609, -0.0027634728127307679919344, 0.0042327501898009583375382, 0.0034880783114149939774795, 0.0038894071956203468513891 },
	{ -0.0033761770021759817880791, -0.0033121936554392078511100, -0.0031280330574869428389428, 0.0017023973828424409336646, 0.0012799329544626169164606, 0.0011788639316363850543384 },
	{ 0.0016082901112307338917229, 0.0015484219607235540083767, 0.0014461890762537000371113, 0.0004917299868790879924935, 0.0004403045615007707158130, 0.0009917052082017946239290 },
	{ 0.0014086050589093659403395, 0.0010422948659425629140685, 0.0010676870369951219161969, 0.0030700133470198788868122, 0.0026312647625688161644175, 0.0035380917324401729012751 },
	{ 0.0000844986268110692112163, 0.0000000000000000000000000, 0.0000142316321868064201599, 0.0021483620513069821150731, 0.0017407932200224289064766, 0.0022748178273256031492466 },
	{ 0.0031088358042317891342943, 0.0023950053836366280712866, 0.0024446510016235718787403, 0.0051200545860356763347432, 0.0044292555957600428587218, 0.0058554439073975330862232 },
	{ 0.0006327470223804417025562, 0.0004300088146107242060776, 0.0004576557712858571845549, 0.0025662000940708481512564, 0.0021155982544801021395797, 0.0026775277220422049641313 },
	{ 0.0010487719075311899459557, 0.0005395005613293724642046, 0.0006486214084339448097613, 0.0041657310929467412236038, 0.0035491118137021028522415, 0.0044174814588526121680867 },
	{ 0.0000811839495878629164834, 0.0003143867023065150759914, 0.0002187621440692821969069, -0.0031640451151341178297194, -0.0026719055923361710244701, -0.0033395003569034169971230 },
	{ 0.0013382681561736979852090, 0.0015033056364097119577544, 0.0013639581871150549714777, -0.0030590111142573721617066, -0.0024801713170024127994018, -0.0027630518998249141079948 },
	{ -0.0008489322504916736776906, -0.0001417484737274970075637, -0.0003525811242161375159629, -0.0066051392961621211916134, -0.0056101968975682996523813, -0.0068184580630463132930319 },
	{ -0.0014725029345185448956951, -0.0007937011242971140061869, -0.0009389526828925952870245, -0.0061965646141833828630663, -0.0052546581334758082557079, -0.0063355663744656303196234 },
	{ -0.0002119937496596321967277, 0.0003756415231491555778258, 0.0001798608996559653960129, -0.0060139557130231714038304, -0.0050659407627343710014967, -0.0060158514149820062344443 },
	{ -0.0002174889999248134973917, 0.0000571245463942631785865, 0.0000000000000000000000000, -0.0035361649873847291608575, -0.0028706412070344010725731, -0.0032797469711799260259910 },
	{ 0.0011251161480622438957472, 0.0016921677907819089852620, 0.0014217758662900300669490, -0.0067161375297806894316133, -0.0055822100467079480493049, -0.0064662607561328582991833 },
	{ -0.0003559450672887121266616, -0.0004073720361443691024769, -0.0003305429452704979108660, -0.0008996140951838351752790, -0.0006040273020587609608192, -0.0005540252854645339375059 },
	{ 0.0021355168863331579730269, 0.0024119500249143188165546, 0.0021881775464002608914960, -0.0045953368667191168278618, -0.0037218534359533652147722, -0.0040307815560273090385590 },
	{ 0.0014388418708494660550046, 0.0013084012329738770041843, 0.0012940466787587469928850, -0.0008178338288078487554322, -0.0004773065374242314059909, -0.0001235536737914996058014 },
	{ 0.0006548671626614467611294, 0.0004503265148751068214401, 0.0005102012956858504276469, 0.0000306503647797050718122, 0.0001796205105179787125946, 0.0004239044289906568124468 },
	{ 0.0004277917107777498193465, 0.0000288338669308719012781, 0.0001807552619841041967579, 0.0019686686362376968825738, 0.0018162582543131209743964, 0.0023179973672660409561574 },
	{ 0.0005812032625087637212682, 0.0000866115676246139527730, 0.0002557487998696181090430, 0.0030178839470424091880052, 0.0026965496417803670389790, 0.0033224903406403349416520 },
	{ 0.0004649901470458074745004, 0.0000682808291268630768167, 0.0002172153720819736978914, 0.0020660675316848471812226, 0.0019191130542279700565284, 0.0025039382358410387958980 },
	{ -0.0006029637076416161827266, -0.0008409767532764280024982, -0.0007309328776757109723608, 0.0025885943351022741056033, 0.0021908363359253398873483, 0.0026389209602728650252979 },
	{ -0.0027637836151960240224645, -0.0027578103515037218247674, -0.0025949215723445519937718, 0.0019645786404170200063757, 0.0015272407148082429995117, 0.0016466449217733280813852 },
	{ -0.0005120173205335923176384, -0.0007046231570000071036536, -0.0006118932611063730318476, 0.0027431399982986628266479, 0.0022557152314444249736891, 0.0028276412659855821793431 },
	{ 0.0005842986967530159822526, 0.0003773532769443695947427, 0.0004042392364118413058960, 0.0022251416139156920785680, 0.0018771360018761909949775, 0.0024961123061939551569244 },
	{ 0.0001338522796818031128895, 0.0000000000000000000000000, 0.0000380833749941440074822, 0.0019510108078911059942273, 0.0016030610468571969678270, 0.0021129503589606191994721 },
	{ 0.0005469573063022281980694, 0.0001842012593139140039494, 0.0002734974224490361888878, 0.0035094493221094012064265, 0.0029512829004446090169766, 0.0037127072326209381233908 },
	{ 0.0002197160556011907878594, 0.0002981411005040003896378, 0.0002386043130588020070530, 0.0004771514778345649246231, 0.0002934034075359851791685, 0.0005656909157783153625776 },
	{ 0.0041356403098601734330542, 0.0033991998400654250926667, 0.0033947072351525340097589, 0.0048700777302079856570516, 0.0042662149830002887504476, 0.0057168805799498257150248 },
	{ 0.0021189307090041542037950, 0.0021108177753143692045867, 0.0019704379266683429844376, -0.0001758470714167136060915, -0.0001371216809105709052638, 0.0002105007621779729097469 },
	{ -0.0007350664123505563007574, -0.0003544411182587637741855, -0.0004345695931225236900881, -0.0040008080823480387730462, -0.0033773124546353340894034, -0.0042158005568033720886212 },
	{ -0.0032528827391935318143201, -0.0026086802502896201325089, -0.0026205928301939011848543, -0.0051347550716005435980649, -0.0044203338679418627266138, -0.0057422863027102293703718 },
	{ -0.0003662881454149698201052, -0.0001116385966305519065762, -0.0001743579741998205971231, -0.0031773056894103140525287, -0.0026285789056318988043959, -0.0030386873348820751805643 },
	{ -0.0017903785921815210720881, -0.0012962096021181290003005, -0.0013353786205738730173687, -0.0046594855692339478409059, -0.0039342831563000182915069, -0.0048839860529879515829710 },
	{ 0.0006219543104568376213487, 0.0009527656380535186059905, 0.0008054357744969275990574, -0.0046851898318352831404598, -0.0038419744574653600342751, -0.0042634710178654387432995 },
	{ 0.0005017655858057351288617, 0.0004389127245537674222933, 0.0004685336498841698974101, -0.0011631075115294410137617, -0.0008016607310551993872613, -0.0007847324104550575368106 },
	{ -0.0010753366006979829787998, -0.0003919086457592065812078, -0.0005704301530862152793885, -0.0067835858124878902972177, -0.0057331061778313919391481, -0.0068418531737982502677342 },
	{ 0.0023693503565958991784191, 0.0020176244182192751633331, 0.0020297843591047569702934, 0.0009185000926876125135548, 0.0010271158689816430389929, 0.0017012935810700280207797 },
	{ 0.0010345732420946900011888, 0.0008167757135210287726401, 0.0008116609277651867966619, 0.0018768623566171049616563, 0.0016506481361284359998809, 0.0023499635651101748526348 },
	{ 0.0008275328130436208906584, 0.0005188515360031085120462, 0.0005656117321105508287740, 0.0028733528998932831785562, 0.0024690291769812371461268, 0.0032634236188812228499079 },
	{ -0.0008426474404574494552270, -0.0009671390506393841767982, -0.0008738668142713739046823, 0.0020436912982410731544347, 0.0017106368747619100142282, 0.0021513596342903830611892 },
	{ -0.0024666537395095131622591, -0.0024697957043269391091966, -0.0023190779103650061530872, 0.0016631728637762290164137, 0.0012872026162469170883218, 0.0013245141188024630359704 },
	{ -0.0004109333374220130096974, -0.0006337919289305524650144, -0.0005282368692968693671408, 0.0026663972814816860879006, 0.0022639183605328268636403, 0.0028449437852567549707417 },
	{ 0.0021462783825144399890705, 0.0018378439382986540805154, 0.0018010087594808629578302, 0.0025284829768686591794369, 0.0021998645305740570712882, 0.0031437767951859431694861 },
	{ 0.0008476110539700745707264, 0.0006457624134654121102889, 0.0006616831029014202817176, 0.0021208380463637729537729, 0.0017984884643520780526937, 0.0024891344127905648021815 },
	{ 0.0016344670126094669806993, 0.0010638263738664520408062, 0.0011599183750913889551920, 0.0050349977974815541409415, 0.0042699720407843580124996, 0.0054926586481501768477465 },
	{ 0.0000703835036090404630657, -0.0003162356269605032048874, -0.0001770919919947439057691, 0.0040575349479555749415693, 0.0034123190469303719442318, 0.0042679381439148086560542 },
	{ 0.0010838301313967260014470, 0.0006563296706408921956516, 0.0007424628271541983733778, 0.0040820678563250533885109, 0.0034331006835857350197039, 0.0043690361259889950312774 },
	{ -0.0004677324996257986870936, -0.0007856626145043722059258, -0.0006420412357995566258323, 0.0035166646555207878993565, 0.0029187695923072469635084, 0.0034828758147620732070482 },
	{ 0.0010131314223145019594308, 0.0007275265435370465751108, 0.0007614558974439534274475, 0.0025258403746389059517374, 0.0021173643404287767998129, 0.0026879787733733282069359 },
	{ 0.0018241853538509180537863, 0.0019547604608743511002411, 0.0017927010286337469093659, -0.0027908186250051600710287, -0.0022918883645183540410939, -0.0026207802884844178491208 },
	{ 0.0007172835606085271398583, 0.0007589091889946108625969, 0.0007162816253379945298507, -0.0018255221331746999957613, -0.0014746539469122810391000, -0.0017448843361055330937470 },
	{ -0.0009970334361061961112732, -0.0007358209907511323788140, -0.0007588944224802842385136, -0.0030580041595814218789717, -0.0025510812070118692000686, -0.0032389311083827888697317 },
	{ -0.0003831861622386217207728, 0.0003512760312107546050407, 0.0001133718992001001040675, -0.0072806872697262926125150, -0.0061396895692687114207131, -0.0072848749007750320319521 },
	{ -0.0002939023590093789938503, 0.0000000000000000000000000, -0.0000724261970954701944604, -0.0038980962471736328896210, -0.0031983319273065561987035, -0.0037983477204548330395661 },
	{ -0.0000725809734038858155427, 0.0004596484815369005242844, 0.0002875886948226960950284, -0.0056353427603684528787720, -0.0047095943981845514100804, -0.0055044875501488949379181 },
	{ -0.0015127035255062531039078, -0.0010378757362341560575164, -0.0010861874756532970912742, -0.0051177884086421807086076, -0.0042613215670085939287826, -0.0052386460536510042265701 },
	{ 0.0003596375069065828973189, 0.0004893173266874216529448, 0.0004464573739544146734060, -0.0024928748775361378973991, -0.0019765240509458130088727, -0.0020948502781966259894686 },
	{ 0.0027114424484524419535492, 0.0023185186309931508398530, 0.0023118247114902678272796, 0.0012588296551870360728831, 0.0012969815288740560964287, 0.0020496679977353559481967 },
	{ 0.0026220201874428221942848, 0.0022295730387647100766146, 0.0022025960238834128498975, 0.0027462062000619029758508, 0.0024810546638018649359680, 0.0035618218494005279492476 },
	{ 0.0005220440421455877378493, 0.0001451986859423915116919, 0.0002415903422595448929791, 0.0034801490298816480269128, 0.0030053299389168849388432, 0.0039036221754672507933126 },
	{ -0.0006881148988113567005823, -0.0010642250391524970230817, -0.0008847053969536182579589, 0.0041023666765898731886852, 0.0034860742745189202129730, 0.0042985098548158845802036 },
	{ -0.0023933008731994139320354, -0.0025105771996319038372181, -0.0023218116522671339817252, 0.0025588625754688161181849, 0.0020776355937614380056666, 0.0022993580362896358371128 },
	{ 0.0016511777137553130829356, 0.0012519991764018529277480, 0.0012908308615558240512594, 0.0033831184147665280433082, 0.0029562494447862470992039, 0.0039838258312741335956386 },
	{ 0.0007001767045083905896194, 0.0006567145400602764212725, 0.0006132166846628539502717, 0.0010597905756640759941667, 0.0008618225389406467115347, 0.0013438202217590409452003 },
	{ 0.0010604158488160429427150, 0.0005487579508772108234996, 0.0006748977782737290318990, 0.0048718448444222432411332, 0.0041632824158431168057315, 0.0053473775520033175676082 },
	{ 0.0020207658154375129737346, 0.0017385301028936179547346, 0.0017031533884987589896887, 0.0022704412295657689507822, 0.0019527535646839209310111, 0.0027571576020068849777189 },
	{ 0.0031587381934845590140959, 0.0025963088257399038333295, 0.0025984809356778210479022, 0.0044635440651855555829752, 0.0038410074637589739464383, 0.0052102279749367437500229 },
	{ 0.0020778407271513869153390, 0.0019592613371134351592240, 0.0018652666276095669368323, 0.0008243071649591378552771, 0.0007074232252964235455675, 0.0012369056484506299464776 },
	{ 0.0027493073313836469649674, 0.0021278474562478741013982, 0.0021824166393109769440661, 0.0049399651238516036066972, 0.0042344034604389103881861, 0.0055645875420165429384167 },
	{ 0.0003452110706579140887371, -0.0000244960998596127703083, 0.0000890687125563627317892, 0.0035505821378459009700357, 0.0029476771084708539363839, 0.0035220984059889728912118 },
	{ 0.0012809279944789950939077, 0.0013333503887291090882899, 0.0012502838716006720986423, -0.0021672044997501430096953, -0.0017399884372323100745900, -0.0020695302053835109602764 },
	{ 0.0018953053618630799886241, 0.0020464900054092010663054, 0.0018784549846166459161068, -0.0032808918513365118926894, -0.0026717954078022180218477, -0.0030563042333678681373998 },
	{ -0.0001817491720078690052142, -0.0000670041632121607435560, -0.0000818134830846287749041, -0.0020968627634215140573271, -0.0017434370485458700397730, -0.0022663014391264739780252 },
	{ 0.0017036866137373650118003, 0.0018626339140626320487609, 0.0017078580462692519170709, -0.0035013475068163307964142, -0.0028375276491733008969731, -0.0032590359190919340401371 },
	{ 0.0001938175654536187066460, 0.0004082855454918843949100, 0.0003301792728505648934732, -0.0033172373775808101578622, -0.0027104419647125169677560, -0.0032771014210898458639265 },
	{ -0.0013332253558701490526311, -0.0005280961392867402704257, -0.0007160017228209176673448, -0.0076593901821302556170989, -0.0064729176371799848968558, -0.0078335825273529117312998 },
	{ 0.0003202109475403687845543, 0.0009533821378012500256938, 0.0007194029259419723555322, -0.0071164831263338711522803, -0.0059389449475043300241683, -0.0069563375670146842741359 },
	{ -0.0008593482399274034785192, -0.0009650026168499912940582, -0.0008356942585274059701744, -0.0004775342854219868254408, -0.0002709650910215049136702, -0.0002899843945940630118158 },
	{ -0.0034841768108032390716122, -0.0030522386332851639788311, -0.0029689257697614150621757, -0.0039428155681143330427152, -0.0033657039949457939927158, -0.0044289181004730400481595 },
	{ 0.0001996898139565788060472, 0.0001914230732338260943491, 0.0002072128395257322923482, -0.0014524742206053070647798, -0.0010870801949251450695816, -0.0011302903522631040436619 },
	{ -0.0019709789735387161883906, -0.0021066808875143468345048, -0.0019081080515031690735678, 0.0008693522916159267125216, 0.0007643674568822345820676, 0.0006944303252002277529811 },
	{ -0.0016275033949917439360378, -0.0016015511727686809750892, -0.0014762138281605300445948, -0.0010819348349422610536946, -0.0008280088819947781881778, -0.0010727531679224360135205 },
	{ -0.0011661511658620760632787, -0.0012766494763201009093878, -0.0011257768601276010780182, -0.0000305057765503036207246, 0.0000140208768926100004279, -0.0002224709981666111904396 },
	{ 0.0063207601456463291186849, 0.0064434528814648036124102, 0.0059729508071301902483174, -0.0053247955747830590447767, -0.0041108168130393179379167, -0.0037480932919891538600277 },
	{ 0.0017635096064672489430541, 0.0016544921117763410328000, 0.0016127101879486840148997, -0.0010474007441460529178878, -0.0006740871808991394983218, -0.0004286527324369225969147 },
	{ 0.0024065848150290739849988, 0.0022483959306297662122942, 0.0021711916550384751170566, -0.0007579214575968553149754, -0.0004175157639822385047604, 0.0000122037570040667498394 },
	{ 0.0010831918818484090241000, 0.0011765667560150289919912, 0.0010849535906609470651413, -0.0021549354504101100003799, -0.0016676100898814899802652, -0.0017155580980827279773127 },
	{ -0.0003488804071278547923145, -0.0004850012752743761970317, -0.0004201713031604076926603, 0.0018709836236278178968340, 0.0015815088069786860467542, 0.0020301765236596588937168 },
	{ 0.0008105484685675082724005, 0.0001682889045249250927708, 0.0003507167880468138880697, 0.0055787185368343346694719, 0.0047877143149215378378769, 0.0060337356259080945908302 },
	{ -0.0001805896827183238115277, -0.0004360516841677377795558, -0.0003436676469155391781297, 0.0028047118036308689784097, 0.0023742379367018189365024, 0.0030049958436075990299285 },
	{ -0.0009650370031088533135169, -0.0010815685996372480411065, -0.0009801547516715995375658, 0.0020597418103225489938168, 0.0017269652563253639395574, 0.0021974000031503380242492 },
	{ -0.0001886936027415246912471, -0.0002804911560627077957153, -0.0002361638337038006929449, 0.0016456987402509010674778, 0.0013800239445773490338382, 0.0018377551131445088970773 },
	{ 0.0019611889439272189340768, 0.0018634925158882730857057, 0.0017645796396613760113892, 0.0008108116483857672092406, 0.0007664071253655079960995, 0.0014282428566020149377641 },
	{ 0.0015466086632062690797706, 0.0012066891172659089444957, 0.0012317663459345839539749, 0.0030839069856764310742503, 0.0026817297227615150174984, 0.0036536715738365089546635 },
	{ 0.0024865298799631131362875, 0.0024086602527732298324736, 0.0022782560303533411390220, 0.0005947842366341208296399, 0.0005650136758137020440498, 0.0012020373393803239579075 },
	{ 0.0007550130327774923254605, 0.0007758172359763340023703, 0.0007179897347258176612886, 0.0007511229533054479372278, 0.0005919551417217615166613, 0.0009924210709424351444657 },
	{ 0.0031901778177916570791717, 0.0031292959318798677960383, 0.0029377636197808650042607, 0.0002255658094421402132604, 0.0002124225141856378950254, 0.0008545582575762293879237 },
	{ 0.0001688392400034362066368, 0.0001297117736728393001637, 0.0001387644078408227879019, 0.0014515500862161199828460, 0.0011230550000792620911338, 0.0014979746402743569777538 },
	{ 0.0000500196917067999481275, -0.0001551075538935902048757, -0.0000801711317613449330312, 0.0024270244838079180552115, 0.0019998706740724280297994, 0.0023406567301031242119003 },
	{ 0.0019498658564030229949082, 0.0020079913092072849793712, 0.0018792660823379069880146, -0.0022716473246419161165222, -0.0018226844958327650166735, -0.0020087927301965960262542 },
	{ -0.0011561319800103009042963, -0.0008610479338377723937045, -0.0008802244606436224021173, -0.0033846063663157759625333, -0.0028399497404100050405862, -0.0036618118038451350458595 },
	{ -0.0028314301757880839735537, -0.0028127370955147519733519, -0.0026224453042390178172938, -0.0002835351600276192094927, -0.0002763383250939927915195, -0.0008394667312159801017199 },
	{ 0.0000918170411859229771648, 0.0009322651718729668603863, 0.0006103221662731224969947, -0.0082511254239458764470072, -0.0069781597359252003909269, -0.0083486915919249744116870 },
	{ -0.0025150616590886090695456, -0.0020252120185766239388037, -0.0020245647229716100604091, -0.0041833335766278956224795, -0.0035573986376358688286936, -0.0045403902254680774189244 },
	{ -0.0015615143856815979443026, -0.0008851821686188151823774, -0.0010100917149894819702105, -0.0069495552542076302615648, -0.0058529502073724447297476, -0.0071416835510393229108361 },
	{ 0.0009484423710310038507440, 0.0013188771443621921056893, 0.0011500057038827029426420, -0.0050072309045387543291472, -0.0040882255865967477570511, -0.0045831853777241029004874 },
	{ 0.0006726376948353911732967, 0.0008940940494347574029124, 0.0007956043472528499501187, -0.0037320645842896628743834, -0.0030004757838413120135668, -0.0032948022251341971960992 },
	{ 0.0000498072341454591175497, -0.0001288918809611686077179, -0.0000329506871484550877840, 0.0001509363447987852057969, 0.0003060767951580796951937, 0.0004957227120570011254241 },
	{ -0.0017876580830389920441503, -0.0018007298849709130021679, -0.0016442358699032739478840, -0.0000979866925828622831966, 0.0000442012086676509471385, -0.0000104563553309948299421 },
	{ 0.0048352582077776044139350, 0.0049837158117685392139107, 0.0046284220339830508400558, -0.0042634180391943532137389, -0.0033209761691425161884983, -0.0031153916181898399367922 },
	{ -0.0033516547408708480954020, -0.0040205581610407966144938, -0.0035214438004960340114491, 0.0078084427266606262763027, 0.0064483324468683446070716, 0.0073179580143657206939278 },
	{ -0.0005068740502211663495960, -0.0007571548501561492635833, -0.0006291407095910030730992, 0.0028275440996817328975776, 0.0024090418727705439905229, 0.0030205223452937340206104 },
	{ 0.0016644633412169400430575, 0.0015719790888237389868409, 0.0014897035374852659207062, 0.0007300088448009004393410, 0.0006894835394038399114733, 0.0012458782277914709751138 },
	{ 0.0010933766642355730817210, 0.0008870492931248674748496, 0.0008860109650829318733364, 0.0019785047883203910230876, 0.0017356927184306620171733, 0.0024579150168189058296708 },
	{ 0.0005823968587305143216823, 0.0003397604915541113114984, 0.0003808306561266727122833, 0.0025542069653536488525736, 0.0021899510462616679842829, 0.0029294397866473430129541 },
	{ 0.0011116512647211440525247, 0.0005846380904665577022197, 0.0007090823332861794555532, 0.0046109086603013799132711, 0.0039841022709089981418407, 0.0051376218731374080628260 },
	{ 0.0018351308946885859420955, 0.0016771911411941739424719, 0.0016076190748660539129450, 0.0014159277065327730265459, 0.0012838406778681629282629, 0.0021057265461958758930028 },
	{ -0.0003895826900264439947123, -0.0003719780050481262187456, -0.0003584512751130083029957, 0.0009985818743279740745261, 0.0008203117526140168980220, 0.0012136861062144309753724 },
	{ 0.0016512619528914070608783, 0.0013346067871629150453083, 0.0013587365681471800550606, 0.0028827151362037649502679, 0.0024837917134173620285764, 0.0032954254156011331990117 },
	{ 0.0040679496933554394264010, 0.0038010981019792151533454, 0.0036312304130464451851212, 0.0014952938485330549873986, 0.0013878444181659559120146, 0.0024694930277466371412731 },
	{ 0.0041995247733999123071480, 0.0037414053348650048134993, 0.0036384096367752228294579, 0.0029478544928179910289412, 0.0026054040083758321123819, 0.0038800670872979900818955 },
	{ 0.0045801991572068266253548, 0.0038201687720487600800900, 0.0038086563586821908268532, 0.0052408424226828217995511, 0.0045750066346919486884315, 0.0062061672500867111693101 },
	{ -0.0003416101652697140964553, -0.0004823859995132542236203, -0.0004115803779360342881319, 0.0023019908874095519439595, 0.0018564611357458089130334, 0.0022954601712240080974015 },
	{ 0.0018780908038830001084013, 0.0016613145644082880081349, 0.0016201459822473009923099, 0.0017620315739289898939596, 0.0014766484138089019679391, 0.0020297589319194951282266 },
	{ 0.0029123733888608118250407, 0.0026499627410581428656189, 0.0025488473475685450436723, 0.0015236066675032109568599, 0.0013241787283964140051196, 0.0019750427464561748532035 },
	{ 0.0024552005099315100423996, 0.0028398441243513189609360, 0.0025487708404357970262688, -0.0056332229763242057385408, -0.0046410007645308415727414, -0.0052858269638674524118493 },
	{ 0.0000934415741921543831260, 0.0000731438746759595104794, 0.0000965162350647936345682, -0.0015982558498150568979806, -0.0012448795386116339768690, -0.0015592748637599850112384 },
	{ -0.0019639904718583901278850, -0.0017818836845171189334214, -0.0016959987149793350619253, -0.0021796726580924699612218, -0.0017955736905514100686804, -0.0023840680827439558221725 },
	{ -0.0010995265640661019621027, -0.0007976211806920579811681, -0.0008154138738858358404285, -0.0037755373401384489072397, -0.0031171131474750399074913, -0.0038277627250896959290594 },
	{ 0.0003925273647110639118826, 0.0009632779443101428980267, 0.0007511719469349462758798, -0.0064216394455456288561934, -0.0053202583048381404257077, -0.0061828771794823177143652 },
	{ -0.0003120425658142502215418, -0.0000663772094759974390062, -0.0001112243397478326006151, -0.0036059536850834021833034, -0.0029374736470238510706310, -0.0034355541919326498465082 },
	{ -0.0018305791656987829667258, -0.0012361621937552040063901, -0.0013146794210037290786403, -0.0054440664556137880261688, -0.0045974711652897910785232, -0.0056151081040319303708253 },
	{ -0.0022452307582811852071369, -0.0023253040980046350491572, -0.0021194429313247491186711, 0.0005131639009554492085463, 0.0004647486317387447747086, 0.0002960351497986445112355 },
	{ -0.0022090850079009558860899, -0.0019645977653757560471570, -0.0018909735398581169803917, -0.0022664413963687990091755, -0.0019204579571671640848829, -0.0025340517171055349475439 },
	{ 0.0003385320263131451888096, 0.0003819770138356349815749, 0.0003734627279167838931340, -0.0018339739372872519620333, -0.0014199320635321700280473, -0.0015524845193810789690386 },
	{ 0.0038079687326259079732660, 0.0037586043257414290807983, 0.0035204914588475078908447, -0.0023990002377214832141039, -0.0017467358274269530367395, -0.0013933299459416479369744 },
	{ -0.0000467304050139168218299, -0.0000140862252818927401502, -0.0000353162556038556118458, 0.0006106117777665563987541, 0.0005216200493395944590147, 0.0009742128301469803620255 },
	{ 0.0014301526224447191036615, 0.0010883194456722750445565, 0.0011142416703601970128651, 0.0027555845331244939282189, 0.0024249086966291770114945, 0.0033019591193385699860741 },
	{ 0.0017309079340634709459623, 0.0013514623413181060460064, 0.0013804673326683820130251, 0.0031541236191350578883796, 0.0027783864342640859391997, 0.0038262650572943139593018 },
	{ 0.0006332628175754133987782, 0.0003479992865403152787519, 0.0004122665544807175251607, 0.0030106773983392611133969, 0.0026119199250273141682532, 0.0035299662731975698683196 },
	{ 0.0026707679253165517857527, 0.0021138964456869830700614, 0.0021422971401702391690269, 0.0041276161905790869841026, 0.0036232657918879138288226, 0.0049333190121936716654449 },
	{ 0.0023657102017015010225753, 0.0023982628042806420493038, 0.0022302436747945481726463, -0.0003274946725976791094513, -0.0001937204014812164991257, 0.0003990297952110312103627 },
	{ -0.0021432556117855559689456, -0.0023032776218465528250690, -0.0020722018433786339348945, 0.0031852321946254759724160, 0.0025377582330852591942227, 0.0026743266898829969567541 },
	{ -0.0004664478820921854154181, -0.0005792454711534385713642, -0.0005049394536073359765482, 0.0020992681023662930137053, 0.0017765960699403819059855, 0.0023628437544821211981072 },
	{ 0.0039838334831798103904954, 0.0036734035490401661938620, 0.0035356542297737348148901, 0.0019131982704925110539557, 0.0017433334399118421021574, 0.0028462351548661068284529 },
	{ -0.0004230745308319187945705, -0.0004085763466277947853107, -0.0003765285906272933269014, 0.0014113972145300089904713, 0.0011378007086979460404236, 0.0015220041755649950290102 },
	{ 0.0009365435459301029580415, 0.0008752963996203767371024, 0.0008393813145457653486156, 0.0013124821301892129407629, 0.0010812715382044989480964, 0.0015884222657395960509596 },
	{ 0.0036191201886682008298513, 0.0033187503717229342031780, 0.0031966634671580030149785, 0.0019958494319197669399080, 0.0017466544494252390397337, 0.0027621074548469298423625 },
	{ -0.0001116036536730171042139, -0.0001963294618800379023969, -0.0001527963371134122099165, 0.0018374509447626060591047, 0.0014534009525897200864469, 0.0017912121085518870229614 },
	{ -0.0003229826063732756796787, -0.0005801905283730619507579, -0.0004584181618686037945794, 0.0031521567440723091170496, 0.0025659031539639761375360, 0.0029934407153204209417474 },
	{ 0.0008532444581334731794481, 0.0013759300448875359974832, 0.0011360447918235580099661, -0.0053415442749698447896800, -0.0045508284093081323440355, -0.0054443638868861060284288 },
	{ 0.0012733938235099239322939, 0.0014762416232922490749724, 0.0013297412998515090263446, -0.0030037034535727461306842, -0.0025031301633147268713431, -0.0028754158592414137858573 },
	{ 0.0024688321609416378522861, 0.0024637151284282250880697, 0.0023175197797215378592295, -0.0021655953621652619493776, -0.0016961346257211499903572, -0.0017312795001755579369501 },
	{ 0.0013476156166050770341541, 0.0013327952909444700218405, 0.0012675089777660119086389, -0.0017935227790025960216413, -0.0014224400265923160986692, -0.0015835052671766219976185 },
	{ -0.0011754871487798771063915, -0.0009050099872748690842117, -0.0009016784734403610751530, -0.0030470862616244600087900, -0.0025531356157816910423808, -0.0032840159525343737972891 },
	{ 0.0030620479653459221555667, 0.0033360676737007478181996, 0.0030462261620981798994345, -0.0049335963478776279078630, -0.0039953575688057366144257, -0.0042950283737789151511377 },
	{ -0.0010812374805454559416329, -0.0010011258895121419570245, -0.0009265985615998802550977, -0.0016871995297469909690091, -0.0013418934071501779740909, -0.0017388626173936510502932 },
	{ -0.0020312752236926021227881, -0.0018846002333426661000393, -0.0017761382990766131067389, -0.0021300664043658280519256, -0.0017130293029718969468472, -0.0023159716101670141386037 },
	{ 0.0018323260749566810598465, 0.0018684385889452149055040, 0.0017618894545532669780596, -0.0029634366246336298240938, -0.0022950450775450141442657, -0.0024344735164646721763126 },
	{ 0.0000771254115079420248663, 0.0007486910256567269649278, 0.0005188879619413699776156, -0.0071763845099562751858602, -0.0059757046319551584273588, -0.0070333119660488108149110 },
	{ -0.0002179307981468609086394, -0.0000954586416828154103341, -0.0000922800189390072676723, -0.0024968733965786641532114, -0.0019866319426072908196912, -0.0022669392949079669036250 },
	{ -0.0017145753980668209087018, -0.0014056152950934020038481, -0.0013834264055646000157046, -0.0034321266042779408067531, -0.0028476844273918181828231, -0.0035864187223482511639172 },
	{ -0.0013140858051883320175429, -0.0014754397637973069162820, -0.0013028846755818259718629, 0.0004717776934723679014422, 0.0004911855505017621165154, 0.0004585127793477951786000 },
	{ -0.0004977766660531447251783, -0.0000935478622681599808388, -0.0001872879227989555873599, -0.0043985031032278722185058, -0.0036651239310004930099796, -0.0043615069685445304048521 },
	{ 0.0031464065101478870707663, 0.0029190891375567669038804, 0.0027959018919888980504729, 0.0009612725634394182717621, 0.0009606431713658085446220, 0.0018022372244497120077206 },
	{ 0.0002860046399374714166421, 0.0001112327082307795999812, 0.0001538235477609820067192, 0.0021110752774473160071900, 0.0018216826611674400565599, 0.0024387104857404779713337 },
	{ 0.0007325767222411440349195, 0.0000440006599298805366823, 0.0002541922808375132736280, 0.0058854320480976963017583, 0.0050797678338511276924239, 0.0063863783027761945876311 },
	{ -0.0000284785301546779508414, -0.0002406016024052946020509, -0.0001631740767422581984102, 0.0026671117221673478446531, 0.0022660940728810828603823, 0.0029278363302594240420718 },
	{ 0.0007883206660047754324103, 0.0006416951201035521728383, 0.0006410940577015842179592, 0.0015975724988407311033534, 0.0014241518632435129325386, 0.0021401099342403160329107 },
	{ 0.0017004367985432151076652, 0.0015900573559276599301671, 0.0015156906310107849493451, 0.0012510778044741599924344, 0.0011168446969719650341268, 0.0018224398057348999407229 },
	{ 0.0028229788110581718711678, 0.0023563135334607851652244, 0.0023461609056897440341027, 0.0034083613123366959460347, 0.0030483448496654518521110, 0.0043337303163473979775167 },
	{ 0.0004761674654246825146714, 0.0004114061254515420152679, 0.0004060240821011402859511, 0.0016056344069113189735459, 0.0013839954880356859258372, 0.0019940830790413140855000 },
	{ 0.0014514794765669539623049, 0.0009603610181362688159867, 0.0010498201338245219133710, 0.0041112051799531016196831, 0.0035840607822692580747281, 0.0047078847300735093134305 },
	{ 0.0018606579187869969407854, 0.0016734231457819969887679, 0.0016225393962070600983177, 0.0020779183733254350148478, 0.0018025731788800219285163, 0.0026505958245825501917725 },
	{ 0.0016564452251345569266244, 0.0012525459070578559995363, 0.0013061418531412839934280, 0.0035172099948926789647619, 0.0030666885941585749104921, 0.0041523595111007853900031 },
	{ 0.0028545354265133689522504, 0.0027083452344019947992393, 0.0025832866825042178141048, 0.0012279267876595520089800, 0.0011093506092085259062685, 0.0019366015803947249362960 },
	{ 0.0012898903025480980364531, 0.0009244390552830110147770, 0.0009905420325865619927963, 0.0034437071668444361148553, 0.0029522768337265190181917, 0.0038985762851690771053914 },
	{ 0.0047707327498082065936336, 0.0043753279026940419468850, 0.0042211923446353561903410, 0.0023496460973319050619113, 0.0021322041643663889459726, 0.0033806760783335579496900 },
	{ 0.0028031289431540359902151, 0.0023269006156120190624559, 0.0023289547828258040131078, 0.0035091535887215338611334, 0.0030626249483290991781903, 0.0042099685935018350477210 },
	{ 0.0023282151877701360437323, 0.0024070020029794358083775, 0.0022299066811872479167145, -0.0006925398781521498918837, -0.0005849071731807899826583, -0.0003305600212775720771891 },
	{ -0.0024809820608682380330134, -0.0022741881892459070435564, -0.0021634936849904418121326, -0.0016198103770409429114085, -0.0013773040539345109221220, -0.0020630481092694121468545 },
	{ -0.0014455457576715319712585, -0.0011720644179991830148568, -0.0011485142945982299272217, -0.0030144763148902331204027, -0.0024817491359585648266761, -0.0031952865703906009435531 },
	{ 0.0011155413240345678996951, 0.0013426657726293529052791, 0.0012202466291473070859885, -0.0037879452769824148959665, -0.0030458117916776898150000, -0.0034229267185495390601879 },
	{ 0.0004536727094736485747448, 0.0007111895618522004903356, 0.0006166408608972251490649, -0.0038232877320866069138783, -0.0030821702488783670956241, -0.0035984200097579689160698 },
	{ 0.0017341806997229929684229, 0.0015142365250655079669806, 0.0015194397646359769638025, -0.0006963847363013155782399, -0.0003454829275058058117229, -0.0000786531561085488382153 },
	{ -0.0017784184672110759598690, -0.0016138343233300590957857, -0.0015392948544808299216058, -0.0018109292489064889204731, -0.0014859077948251829537352, -0.0020253538320578528750260 },
	{ 0.0013942854354601099103189, 0.0012706378652246220085226, 0.0012526668801306169772486, -0.0009784039442440961922842, -0.0006302310602383845699465, -0.0004264330924349134981435 },
	{ -0.0019693409077811020041482, -0.0018565716093984559621061, -0.0017423240661219819777616, -0.0012048006644024070177501, -0.0009956666489753962945991, -0.0014936145348102649633992 },
	{ 0.0013583614547100950845776, 0.0013826228605464639664913, 0.0013090651574058289208918, -0.0017291203428016969738390, -0.0013084794078156449480266, -0.0013217786470069319872123 },
	{ 0.0028789607968121178754872, 0.0032482356894492610613345, 0.0029376029940104959707614, -0.0052138290311175908287034, -0.0042278005755658132000518, -0.0046288249816240729581085 },
	{ 0.0017487871843802249745281, 0.0014501589856013689678466, 0.0014472914426567479173613, 0.0024647086457802490702673, 0.0021645734831367540590419, 0.0029938955123066579230851 },
	{ 0.0023715364259051809579792, 0.0018975661354481958981166, 0.0019202054198190019601578, 0.0036967186747784339627410, 0.0032660345501408180081404, 0.0044787612983133284066950 },
	{ 0.0013982201638546940675178, 0.0014515588645767319667806, 0.0013412946045164770604574, -0.0001494829007930696012415, -0.0000580736603190592896601, 0.0004360546852356429227415 },
	{ 0.0004742856111967327817185, 0.0006734171158633778589420, 0.0005566957165644877136845, -0.0009163200352353980228728, -0.0007286993479195211795632, -0.0005528171476383293399606 },
	{ 0.0024024977566110311717751, 0.0020512757168593439942306, 0.0020244685657592330525756, 0.0025548566378479470838359, 0.0022963347347799149995862, 0.0033359019564290341187951 },
	{ 0.0004068619854687597161592, 0.0000951044136531555684257, 0.0001877252240448473133601, 0.0034465877251135409141403, 0.0029739548944857090335969, 0.0038429290191304399902650 },
	{ 0.0021590051130417329573741, 0.0021920625706558722126105, 0.0020380314319873990211440, -0.0005037588650990821974307, -0.0003365653387490082147464, 0.0001275376432254823129593 },
	{ 0.0001259253115263391062557, 0.0002288310335656003910530, 0.0001920771246133029096659, 0.0003171913264704843060313, 0.0002770303768312940831051, 0.0006121706885024931791769 },
	{ 0.0006981277812431576149099, 0.0008581669459131440868410, 0.0007511218053199298760186, -0.0005585085237493481310697, -0.0004607913563538598863099, -0.0001383774771952633051222 },
	{ 0.0006890870619478862849419, 0.0008083453056926227179441, 0.0007256600403375765284686, -0.0000213540921720720810312, 0.0000000000000000000000000, 0.0003051347944865986168590 },
	{ 0.0020354741190395280250880, 0.0018734711569257419660345, 0.0018043939247109899107896, 0.0014650449668528050476551, 0.0013135107462148860445889, 0.0021099700281869320298012 },
	{ 0.0020691231062441579717504, 0.0020183675608355399790084, 0.0019212728139934580513792, 0.0008701610818020931404845, 0.0008045129632248669039607, 0.0014551932962198930402581 },
	{ 0.0021347899463010000173369, 0.0023670210196016778884942, 0.0021391317218892318303969, -0.0021060410462910811139892, -0.0017409870409551000351028, -0.0016187160870394950475115 },
	{ 0.0028728888118929010904445, 0.0026625684531072398020246, 0.0025674671370316030048764, 0.0015813478871112940198074, 0.0014150342075660430393030, 0.0021652259957998971906523 },
	{ 0.0033478277531600758067098, 0.0032135647656915109805775, 0.0030491499583775350142034, 0.0005258096417549369726643, 0.0005139027048061190665676, 0.0012374969582401829874124 },
	{ 0.0006891285884315316969187, 0.0005362406544149885640957, 0.0005632128670601547950486, 0.0019114897766718400625580, 0.0016100643936375789392473, 0.0020662921008820140690576 },
	{ 0.0016122005205824040553059, 0.0020821883465689428223400, 0.0018042554508649840706858, -0.0051104728452095113389042, -0.0043400541879403805797799, -0.0050388446737886592036171 },
	{ 0.0005921599350968353384778, 0.0008721812123374645363272, 0.0007530410795424153811914, -0.0032173499585807950260086, -0.0026663694198809269331196, -0.0031825001101877521507422 },
	{ 0.0006088733738097487895730, 0.0006646108166835423478047, 0.0006310091766697818608037, -0.0017501138095826719240367, -0.0014093373810973839916488, -0.0015922610642169459770762 },
	{ -0.0013307642877638950831831, -0.0012965089637467059561354, -0.0011856360367308560986588, -0.0005657017010562142168145, -0.0004387692159492094013708, -0.0008225719527291781244666 },
	{ 0.0000348267932039637625866, 0.0003380264383630919252689, 0.0002511242643250253853311, -0.0037741019229658619160706, -0.0031374930831492950540629, -0.0037108348320297251596300 },
	{ 0.0019682147215051371338945, 0.0019989052841702138751945, 0.0018894305082517240130385, -0.0027410948813159918824311, -0.0021228498473756661664436, -0.0022486123558028070154591 },
	{ 0.0005625224334335180050026, 0.0005587988276136904687921, 0.0005681754142830208395565, -0.0014653467728185768943067, -0.0010491228417125050224185, -0.0012027699556503980253541 },
	{ 0.0011627671805604449482840, 0.0008704654656883557784994, 0.0009318539084920842525614, 0.0001597829240297223083855, 0.0003470063443409731793708, 0.0005730017036619514721879 },
	{ -0.0000761523938131694920007, -0.0001050561366369919956594, -0.0000404853691080175027658, -0.0010922980992207559705620, -0.0007531386364424178490873, -0.0009246654667445726493227 },
	{ 0.0020126197242514290083881, 0.0025695743035279100877077, 0.0022444295882275309625409, -0.0069002260510172370652771, -0.0056893140495371573653327, -0.0064577560853220023276422 },
	{ -0.0007640742134245244758170, -0.0005604779631406445845540, -0.0005457710740030560867680, -0.0027286353805904138732086, -0.0021946353543456560176994, -0.0027399118926720107854156 },
	{ 0.0027541509732547249898749, 0.0027490175882255688151468, 0.0026016957266797782061252, -0.0018602981545618839376299, -0.0013589020576363530952435, -0.0012209998928431789032983 },
	{ 0.0039865849070938388920560, 0.0044063497387924598774900, 0.0040094358362924758035217, -0.0054457265546094769589902, -0.0043773169632664666109378, -0.0045764851472235240983921 },
	{ 0.0030647076400330099457681, 0.0028519322127537900107841, 0.0027295126954694688171943, 0.0011288730452211409235885, 0.0010864531356831839004756, 0.0019671030265430038998231 },
	{ 0.0004709760294924443058476, -0.0000426901211144761611707, 0.0001162340448657263943400, 0.0047920972817773709040434, 0.0041377081299512246184702, 0.0053524036508857241595249 },
	{ 0.0007210427386651162414907, 0.0005558072636125660883136, 0.0005723145377739778921072, 0.0019078824505465101068341, 0.0016698017240753639574974, 0.0023260629480094229676324 },
	{ -0.0017205770092336590201820, -0.0017522191339994250096346, -0.0016322022673475489618761, 0.0019020275472242228944481, 0.0015729680015309629656506, 0.0019170519532234269691889 },
	{ 0.0006010153821109688338648, 0.0005176675094419909547647, 0.0005155089809299991457370, 0.0015551063145072970515803, 0.0013539145325101109920696, 0.0019611293690282517905576 },
	{ 0.0000000000000000000000000, -0.0001817276137457724015985, -0.0001188433946905109061436, 0.0024602914797640369586051, 0.0021286528485609649587185, 0.0028685968410709370216005 },
	{ 0.0009306843293452601384788, 0.0010695203143362759946183, 0.0009542338782438050150125, -0.0005528136897596734036439, -0.0004271135213935639183234, -0.0001020220056992552021616 },
	{ -0.0007906970905655445747140, -0.0006968504861852629992530, -0.0006749539088492284609824, 0.0006887146313314661564761, 0.0005577427935292099663717, 0.0008239458526444976222206 },
	{ 0.0013711720410667989789727, 0.0011854472795838999314710, 0.0011749677221118599596689, 0.0020328042888437020292713, 0.0017801323806430069396356, 0.0025767520774008689961321 },
	{ 0.0001562378091960524924449, -0.0000211496875694423403359, 0.0000447699956386152521349, 0.0024047592275409511615447, 0.0020728720930303988316534, 0.0027388171256905878732379 },
	{ 0.0018715024183686380584990, 0.0017510377508306539367372, 0.0016894620377477340567723, 0.0014717997896694669495571, 0.0013112819424112430889001, 0.0020927552366438329639609 },
	{ 0.0029028249784453200206535, 0.0029067903502230708380483, 0.0027219218944917649219417, -0.0002857866556067541110447, -0.0001511314425143319133821, 0.0004562479982606252150792 },
	{ 0.0037438156904229902104120, 0.0036846457686493649247295, 0.0034779700809155380918092, -0.0000379956839716955968545, 0.0001004382209223914947105, 0.0008521090099601833769624 },
	{ 0.0036250901165744758500309, 0.0034498320256725169621892, 0.0032900347207613601582810, 0.0009111352592934080169354, 0.0008887867320255194358500, 0.0017904548905899779834283 },
	{ 0.0036349616784405848826656, 0.0031857564688882789895319, 0.0031360565449420688133308, 0.0032244663905819140846398, 0.0028575249208465489277053, 0.0041172551249683405835689 },
	{ 0.0017088302349587539998022, 0.0018269694686247339721064, 0.0016764292334257779523710, -0.0009885917369954907128748, -0.0008218762621415208685996, -0.0006635380440480727814653 },
	{ 0.0014777037926353810744912, 0.0016208225172119599538645, 0.0014849723946821278934560, -0.0024212814486818559460424, -0.0019948222043679429693608, -0.0022692081847212291871585 },
	{ 0.0003736963557951698236721, 0.0005745942850755403168150, 0.0005105748551419564537288, -0.0024873278810579710525197, -0.0020408263324169940294839, -0.0024747912545094400554868 },
	{ -0.0001402622290486755000320, -0.0000730548995500479846641, -0.0000390374427853922606100, -0.0015861445081242079094275, -0.0012160199901902831072509, -0.0015335451604835670640775 },
	{ 0.0021638891049406490559026, 0.0020681171365603341801009, 0.0019999395189908980574600, -0.0011467370489603970380721, -0.0008091186915721674264082, -0.0006871648989371163503484 },
	{ -0.0008186566342486966026126, -0.0006051321214062122108221, -0.0005935901806730675038873, -0.0026025595119688049727813, -0.0021538394133141960766487, -0.0027474901230998588670673 },
	{ -0.0012835560172941499861021, -0.0011962510925034200263972, -0.0010922656171020140528777, -0.0014342892917411020296120, -0.0011004871811958550145705, -0.0015067152456130310494292 },
	{ -0.0000921793197444755520739, -0.0004263350639665726155708, -0.0002503198919202281988623, 0.0013592764184760259410512, 0.0013309622926220689240889, 0.0015145805803382570136478 },
	{ -0.0008121744368751747421634, -0.0010683334827770840213396, -0.0008861055794156584520735, 0.0011553214067585300221797, 0.0010743010660830228965301, 0.0011506737478477909158298 },
	{ 0.0000781059946163000992647, 0.0003991233123273875112516, 0.0003037334486558479203049, -0.0039594357350762506009278, -0.0032191957454494948831503, -0.0038505824542615489126940 },
	{ 0.0039255019938327371034625, 0.0039338158556494013870508, 0.0036895605005226300016075, -0.0025949407535295911618467, -0.0019708070854011030571817, -0.0018201592711369670579691 },
	{ 0.0015229337629675929362272, 0.0012613437915188899837021, 0.0012747823772802510059604, 0.0023425386121136010375565, 0.0020785166275383922147857, 0.0029014008889405940375128 },
	{ 0.0007431512641487749565306, 0.0007544189941350960479372, 0.0006862239156451618166122, 0.0005504189843411249954616, 0.0004644941931438027922507, 0.0009331298382765889753293 },
	{ 0.0004169915387906681877341, 0.0002930684651007638987473, 0.0003220686615897515227974, 0.0016816256951986368987917, 0.0014980448232790239716794, 0.0020851607727032698234859 },
	{ 0.0008983059640004378105724, 0.0006438407781390381376396, 0.0006770777215122259374419, 0.0025606786501813768794356, 0.0022134895287938527973226, 0.0030261774670272191013753 },
	{ 0.0013631888637276150244571, 0.0012589656540809620460797, 0.0012244128376188529935820, 0.0014490424310960759324979, 0.0013146564117623869751700, 0.0020675853523710969669724 },
	{ 0.0016306132625217130763251, 0.0016313884287194439521446, 0.0015187405671013458948598, 0.0002005719111195545059124, 0.0002216334305794222117938, 0.0007375736583495792189277 },
	{ -0.0004762579194955567931345, -0.0004043772542576157019885, -0.0003802369895483123738572, 0.0007310271978743953418325, 0.0006376524073594827882183, 0.0009562085496343120799567 },
	{ -0.0011726032770085870020338, -0.0012830970031890069713715, -0.0011601159518769899644391, 0.0023765545020447108315320, 0.0019684063200466660539800, 0.0023889392222155589676214 },
	{ 0.0029940537459776381604137, 0.0023457537091212529375972, 0.0024089232894219329145225, 0.0050829895521118367146540, 0.0044613967595827494147520, 0.0059908119842700330651142 },
	{ 0.0026495751348570208832944, 0.0026915676794119157942686, 0.0025259431039310210193038, -0.0005017569314001727115784, -0.0003156596898086164041178, 0.0002487034078140069231003 },
	{ 0.0045056297802550018122281, 0.0038945646538586929825965, 0.0038444249427998290237485, 0.0041220195664443803201804, 0.0036647755840780310565563, 0.0052264694837134166235937 },
	{ 0.0051098831706921591516712, 0.0047436038365776551223307, 0.0045753577722605727126637, 0.0017732125640636249401527, 0.0017045316830352869822557, 0.0029796155318675041599741 },
	{ 0.0017620846485117949622307, 0.0015849310900081659946731, 0.0015539368224835330058869, 0.0018063675429684710946332, 0.0015405971270381899670271, 0.0022482619795065189594041 },
	{ 0.0022245711391263669511054, 0.0022433220591665599570608, 0.0021101269938066029123458, -0.0003710356869751507843079, -0.0002646132774255838845771, 0.0000798105359764022951122 },
	{ 0.0009077738825324856326848, 0.0008148187713605078335080, 0.0008003368385666575351989, 0.0010815123496062039806787, 0.0009044370236921155581031, 0.0012910280500678479251309 },
	{ 0.0037422696280707810380373, 0.0035362965204609360381216, 0.0034019274415455101265304, -0.0003990224033994342069630, -0.0001713558106859859944682, 0.0002068405480826002109003 },
	{ 0.0016277823316031220582389, 0.0017745945331972400472315, 0.0016305698121234369745575, -0.0023043025892758909299596, -0.0019087832407122789363241, -0.0021578658049994478736189 },
	{ 0.0004945362462371977966408, 0.0006859861042559793663578, 0.0006115271399744206734517, -0.0025769663978828859056402, -0.0020972660196208249040739, -0.0024661129302127309664838 },
	{ 0.0015641673061337559198780, 0.0012403570120206239540267, 0.0013238923626144260235760, 0.0008073978627659050052306, 0.0009127314226676419475076, 0.0012742710908440698942129 },
	{ 0.0006241779197714926495991, 0.0006523622861902227511657, 0.0006356693062130315497946, -0.0018052998528580220614220, -0.0013696309144460249145742, -0.0016169699794902308957745 },
	{ 0.0009775777588489221580498, 0.0010531749668849850910118, 0.0009996071056893626564921, -0.0020274711649334621610397, -0.0015646437220383660558071, -0.0017391986019966269860770 },
	{ 0.0039129295044498723851745, 0.0040041777042399756200708, 0.0037286978820327159635240, -0.0033074937825011119750529, -0.0025776767212999699384335, -0.0025664154898902519046089 },
	{ 0.0031059434956515019447820, 0.0028516628502049728641210, 0.0027432075949425150686312, 0.0014625497259412229868192, 0.0013428537801165219174593, 0.0022389185643432538835862 },
	{ 0.0016966084745186230196140, 0.0013556309656438890985652, 0.0013717961819957450765595, 0.0029478305747302349995587, 0.0025440481138835888998140, 0.0034997089275232077902611 },
	{ 0.0032627992019719199122407, 0.0031911862161573319868202, 0.0029966305966281732084400, 0.0001719566390961245916803, 0.0002223420911066080094768, 0.0009933420467267419732421 },
	{ 0.0007874267018161178405614, 0.0008037590495346445340977, 0.0007430731801706376513789, 0.0005771785992298056637215, 0.0004671853029820776932561, 0.0009097621114398959886113 },
	{ -0.0001833127681207533999676, -0.0000825734213012231936697, -0.0001023931201581829034206, 0.0003233275915453206919396, 0.0002889686047472718065030, 0.0005723445974894281803452 },
	{ 0.0007580213258703524022847, 0.0007797703938800719467711, 0.0007251641944225916715239, 0.0005059045899607422054275, 0.0004220548099973670799420, 0.0008570400261664435715217 },
	{ -0.0008842087243474064254048, -0.0005101865228452498804423, -0.0005909897110409819418267, -0.0017859990958037069014308, -0.0015955729815628539626504, -0.0018168505466210499183638 },
	{ 0.0025065412753414490105996, 0.0021490674617732109796808, 0.0021370319330154151013512, 0.0027475544371308589959579, 0.0024533159535425051213375, 0.0035161590411317178613004 },
	{ 0.0024281608332414059484017, 0.0021460191869474469841816, 0.0021193205919892519478731, 0.0024415465888184642063996, 0.0021349324164946451715119, 0.0031062664211463980254435 },
	{ 0.0029027452139702520847453, 0.0029153091180342078844734, 0.0027444165301512401383688, -0.0004413904329522239859511, -0.0002530106686167118198311, 0.0002883054309641151823421 },
	{ 0.0023742697286293391878420, 0.0024045677980303009450835, 0.0022551283314849908390176, -0.0001716127459117247980053, -0.0001485582991004530872478, 0.0003231922112128671068301 },
	{ 0.0011003612455281039286120, 0.0012380859533910109834820, 0.0011267686529144069988023, -0.0008230261376108779104027, -0.0007372192098198183159846, -0.0006626408475472551819901 },
	{ 0.0002950899679835856004480, 0.0005505021506758772881565, 0.0004377160158673179795458, -0.0017465628161076340602725, -0.0015673017730260380402790, -0.0018662897421331040275633 },
	{ 0.0041223045303978855802285, 0.0038240848889960540185617, 0.0036751805757392549955931, 0.0006754070239229998046451, 0.0006820248579712719777524, 0.0012251116791535399119795 },
	{ 0.0043753247314852949587105, 0.0043055568639988935769614, 0.0040614845390876484071407, -0.0013604951913347440323881, -0.0010886479439396589546929, -0.0008950383112130301912004 },
	{ 0.0028295159048473679345992, 0.0028741709489560080675874, 0.0026953284110753489809242, -0.0016979019061544549980963, -0.0013672619110699870716152, -0.0013901624012361750524353 },
	{ 0.0028011126524490849096227, 0.0027458753443351919219795, 0.0026093956216375418198650, -0.0010433457761303010797455, -0.0008197133163775106867721, -0.0007504129721649350310761 },
	{ 0.0030468284136481518786788, 0.0032412935165588622164756, 0.0029984875343480629598936, -0.0032322041865662089026034, -0.0025901421958971159419105, -0.0027393937286325211952209 },
	{ 0.0009720929489265834992640, 0.0006639273292884719521526, 0.0007502126132644252373000, 0.0013078617486598730339925, 0.0012126098734255470745769, 0.0015076889620906090867802 },
	{ 0.0020841298183116750350474, 0.0021128328176788428553745, 0.0020177676458769120207415, -0.0019263631104187060264638, -0.0014372676276950770071078, -0.0014766546067556639287854 },
	{ -0.0002145497334849377941979, -0.0004836988855527031235838, -0.0003066315137312426849711, 0.0011012857637830478999691, 0.0010961970717179730667318, 0.0011763084117688349990866 },
	{ 0.0008540093156800240880092, 0.0007453535150950929121655, 0.0007699409309546304328031, -0.0009570586726803115154918, -0.0006586360063037052662085, -0.0007490396823672304917399 },
	{ 0.0023456064065162309136003, 0.0021971715713656699876799, 0.0021379433433065891077829, -0.0009611962492222996890315, -0.0005903976818879570070964, -0.0004298306505021498905537 },
	{ 0.0039761994345864542024782, 0.0040218181859415962423054, 0.0037717493283213839906642, -0.0027897601173278640737663, -0.0021130297402568451806126, -0.0019612429925578828658617 },
	{ -0.0009544358670567240588919, -0.0011920673390259808963748, -0.0010351013826420999396072, 0.0032320800681092370226777, 0.0027113462890258829829371, 0.0033261928662276819823607 },
	{ 0.0014766077711020799811265, 0.0014108026436408670125505, 0.0013312940327238310337515, 0.0007175672652317787152815, 0.0006544497668379146548129, 0.0012260197798003079436985 },
	{ 0.0002064320322719473941881, 0.0001755329679898111089765, 0.0001849681805727724007420, 0.0013013875408509269649554, 0.0011034966060438689862411, 0.0015673476930117540700804 },
	{ -0.0007397335392838708285901, -0.0006991170842939916762332, -0.0006509882883659640507440, 0.0010564061964639819869716, 0.0008725042550234204247195, 0.0011696969335795489707691 },
	{ -0.0001927560299532808045075, 0.0001550654010075904083981, 0.0000496314309934834692458, -0.0016869449801161741017697, -0.0014570868762870670121146, -0.0015296402306592220537301 },
	{ 0.0012074239989608769128143, 0.0010909636864083350343835, 0.0010902200668272100329143, 0.0016473274494174659950130, 0.0014621560880606369534074, 0.0021894852294141601070643 },
	{ 0.0028429308343491228004696, 0.0025433195126815131767439, 0.0024925466831716260387997, 0.0019350234897489949556504, 0.0017921580582455039817175, 0.0027158124557536151427728 },
	{ 0.0024523312622957648375388, 0.0025095263852735970842855, 0.0023496685271528158257814, -0.0006147405171837930920598, -0.0004434988909505349890389, 0.0000187378846224709792278 },
	{ 0.0021296017052915288021719, 0.0021530914186438320466477, 0.0020394040460101439150853, -0.0000186893790043324009434, 0.0000472333603938330625748, 0.0005490687590799106856942 },
	{ 0.0063155363074037596318733, 0.0060568264636719076898785, 0.0057666328966075588655094, 0.0001413607085881684894552, 0.0003451908047045426858671, 0.0014805136714276028939152 },
	{ 0.0027784790227418478328336, 0.0027593513754160300249785, 0.0026147798261352589617446, -0.0001680516372159719987699, -0.0000733801144190540031694, 0.0004024639382540838952219 },
	{ 0.0014588144433861770666444, 0.0017830243719110920076754, 0.0015751951936929379279734, -0.0029318074996245208248269, -0.0025021683340363779120274, -0.0027966099515340067833968 },
	{ 0.0034847071148739290277763, 0.0031628492101288380698720, 0.0030693312423470931436797, 0.0010540416741915970000321, 0.0010025610943436530191064, 0.0015450449195337689813762 },
	{ 0.0020891538546307960970416, 0.0020565032903713947876057, 0.0019595630944991408207978, -0.0006475054243407869433713, -0.0005301325134116192071276, -0.0005671669724458237599879 },
	{ 0.0008962806416642894692323, 0.0006204448471436093282688, 0.0007062392385034638305971, 0.0012822534171578979771827, 0.0011951522717952259722979, 0.0014157866465911680183692 },
	{ 0.0040194260580804696755819, 0.0039697837830515916163199, 0.0037784438909743119430762, -0.0021524325499134868387718, -0.0015858181806361251048321, -0.0014002186718392560516033 },
	{ 0.0016244818736125159804734, 0.0013346808190836340155677, 0.0013846987748282180413079, 0.0009390301136885842491278, 0.0009710875515800753777010, 0.0013238293235950469167728 },
	{ 0.0017797849490982799690320, 0.0018783729589605070373071, 0.0017798258226555539913971, -0.0027125916974555987909445, -0.0021336059072605828568214, -0.0023001712635564381184694 },
	{ 0.0021175175887310601803570, 0.0022306683350666908598192, 0.0020930223170922178724251, -0.0029792995949179381862115, -0.0023432268971443630418972, -0.0025037389320556728035394 },
	{ 0.0030787342427219858108367, 0.0032248446052134029787828, 0.0029974563692096939729326, -0.0032445071820327020528651, -0.0025548574493005119477596, -0.0025958563669827958327041 },
	{ 0.0027888707091226747937507, 0.0026644795953371181243541, 0.0025262900487025871480717, 0.0006078212759960977549026, 0.0006070787106870113468068, 0.0012726440402288539607834 },
	{ 0.0022481410626187759912997, 0.0020672437323857339487587, 0.0019900728361572739104346, 0.0014809213350746839883004, 0.0013347054728300019807441, 0.0020810274160999410117401 },
	{ 0.0004370729176162525060796, 0.0003389509770310148752126, 0.0003529345373954924239626, 0.0016750046384183580119864, 0.0014027493350773300416490, 0.0019189046913764579594536 },
	{ 0.0023776294268862238796836, 0.0025372025837077372134587, 0.0023406070070512781551575, -0.0015169744980240780669878, -0.0012081516668442720489202, -0.0009282678101938818000100 },
	{ 0.0026848373640161000627802, 0.0026424703989191659914137, 0.0025132874495170840455083, 0.0001076868231321551950836, 0.0001875135767593190069293, 0.0008009389758053582367495 },
	{ 0.0036815850606041738285468, 0.0035622805522209960611690, 0.0034014678771526469895636, 0.0002800598893733213909547, 0.0003660434272149096939820, 0.0011620538693513169769406 },
	{ 0.0014882276004810689011559, 0.0016195832540673419947669, 0.0015081539520154008983399, -0.0006671932523878126419098, -0.0005478578886350553072901, -0.0002696632421588227891684 },
	{ 0.0011097225662067119935500, 0.0010524306866475179565723, 0.0010438210140671280029556, 0.0008318501494807669293871, 0.0007319551859326612567452, 0.0011919729305895439188528 },
	{ 0.0015628803611388590501935, 0.0017614195076491069580360, 0.0016093012125507229464344, -0.0015025867865827559431008, -0.0012806242901444910703423, -0.0012448431888585989330692 },
	{ 0.0022348346075910970054412, 0.0024689461874273458535933, 0.0022496801246908440172212, -0.0024117162158444711492211, -0.0020344860338473301671069, -0.0021198757195978199310837 },
	{ 0.0025092188022396108050549, 0.0026853603023580508400714, 0.0024731819317532401786286, -0.0022066722769281230283334, -0.0018417560999336760833162, -0.0018669024030943390853737 },
	{ 0.0037618841255433028113175, 0.0036524472921375429612656, 0.0034701182154193120632524, -0.0004169477359030896910196, -0.0002832943656960701752680, 0.0000167271634262108008897 },
	{ 0.0025055473532139328242396, 0.0023698676134330321288446, 0.0022817073490552191965264, 0.0000969857321662000848974, 0.0001230711561646959085158, 0.0003894448688999793857513 },
	{ 0.0029532215605745319036979, 0.0029686232620575749539815, 0.0027895322999199980741614, -0.0011823309687110780327735, -0.0009645520883623315885197, -0.0009029054727051752035627 },
	{ 0.0025021746311070808960197, 0.0023087199163697390801031, 0.0022432557641965588865474, 0.0004935556126593804897665, 0.0004668085313904554896347, 0.0007786671892287710299366 },
	{ 0.0048423948874727522698191, 0.0048022543665037472140900, 0.0045366897348890168270574, -0.0018111395157221368921313, -0.0014028419845559950487890, -0.0011524421921127680178143 },
	{ 0.0038147205368339108348674, 0.0037941205804257679967684, 0.0035920298381940068792317, -0.0016761348198969338967046, -0.0012753364299771120359689, -0.0010714831208912820384449 },
	{ 0.0015879088288955629359317, 0.0014063900871856568983859, 0.0014206718991192060410483, 0.0002990182613802130082506, 0.0003649817726976042051891, 0.0005786421535562803015670 },
	{ 0.0021458736051518738884025, 0.0020068092755419408498851, 0.0019594936302788491626348, -0.0001362519978551107084977, 0.0000000000000000000000000, 0.0001473109851323017133965 },
	{ 0.0009016319492690914611660, 0.0007338149771348816390731, 0.0008004947188789248227470, 0.0003908460915541388139109, 0.0004445193341873053148247, 0.0005083368590650306660314 },
	{ -0.0008664094826794397708261, -0.0010650878603652889301673, -0.0008806246031798867788046, 0.0012449257237559490773399, 0.0011264284417356849876679, 0.0011357726706864249704987 },
	{ 0.0017848793532966220586056, 0.0016209420830860649370003, 0.0016271917618905600271678, -0.0003259071777668726827004, -0.0000944009808640583686964, 0.0000427263779419449288570 },
	{ 0.0021829796569514960634384, 0.0020164544724777149278805, 0.0019967971691875470267863, -0.0006168855561764941420472, -0.0003274670569126609045996, -0.0001708520800799939970242 },
	{ 0.0019033677371175679315929, 0.0018108988106449420366767, 0.0017839177748762370580987, -0.0013791933976206869390951, -0.0009586163601922270899661, -0.0009011027860409874870595 },
	{ 0.0012831373395923780390132, 0.0012339953478456989598849, 0.0012158546075071289573055, -0.0011154383453062589100918, -0.0007994955874605758463017, -0.0008553460941240653052711 },
	{ 0.0020974344673357139812175, 0.0022719902936897719281095, 0.0021187632070476252084401, -0.0034866089394673099172151, -0.0027712526464263279209199, -0.0030428226701442089366378 },
	{ 0.0016125597633845569107980, 0.0017803944374058470329070, 0.0016406665752296779522967, -0.0028025516738223160001509, -0.0022542240615365258596392, -0.0025004014514109218492166 },
	{ 0.0027564058850476839783450, 0.0028426831122556730016560, 0.0026658128414365758404592, -0.0027474318235511520185088, -0.0021277537234324771404925, -0.0021311096595521580691901 },
	{ 0.0029936757501508870493723, 0.0027736916269797190146007, 0.0026603500470305219659239, 0.0011176980844351599043290, 0.0010671041185671211078134, 0.0018914256647686899289601 },
	{ 0.0025124968209938630843425, 0.0024227264914003969822898, 0.0022927164954066560006996, 0.0003299112952845341894224, 0.0003657334498253381953152, 0.0010182386060185030438041 },
	{ 0.0023422003445036348318053, 0.0022819524575112531267207, 0.0021803432910369631436698, 0.0002868068559172310977995, 0.0003279526539244202081383, 0.0009344548155162619778799 },
	{ 0.0027203592218694090144882, 0.0027334945314929559243011, 0.0025800451669245119679097, -0.0006237096409265450872375, -0.0004356699934823387871524, 0.0000506625292719772082070 },
	{ 0.0010918678601211809307131, 0.0012983822977077419738579, 0.0011795383533027749972666, -0.0015889943514027279197753, -0.0013500482664130430618399, -0.0013665152705049830005046 },
	{ 0.0013529710346696279671397, 0.0015873498762255019295386, 0.0014329487742327450504332, -0.0020518199335078841395730, -0.0017696614926780080464391, -0.0019750984262355600860894 },
	{ 0.0011629210497647959367262, 0.0013034408666006819504241, 0.0012023150937282079449980, -0.0012992314928822370611455, -0.0011269526184653759926652, -0.0012149734690286939729070 },
	{ 0.0023612337375369151296067, 0.0023252335407883848936161, 0.0022150441640697219984613, -0.0004902673909456417046526, -0.0004078702433027688923663, -0.0003344326616053955899925 },
	{ 0.0021389250904754399280894, 0.0020994475808444150455312, 0.0020068478458521378689217, -0.0004403778978182516985231, -0.0003589724579640227067161, -0.0002259995053044835879204 },
	{ 0.0011734171088467020005464, 0.0012505500653380459668423, 0.0011797115125263719914717, -0.0009672187088637502995134, -0.0008306041827815333761231, -0.0009245120444270732949776 },
	{ 0.0036613210791856150294610, 0.0036210945348908070429150, 0.0034290366167231381595848, -0.0011817622698046630329988, -0.0009406951432030438969151, -0.0008395047595581417833668 },
	{ 0.0017529765721342499724955, 0.0017293122719911090991174, 0.0016646590955154039366548, -0.0006020033309390022653942, -0.0004844411461307467775539, -0.0005038334371484616070261 },
	{ 0.0019048778830500249451690, 0.0017765149279252709976079, 0.0017485825843092639951537, 0.0001872616917922972023212, 0.0001954872223245570869200, 0.0003761344539585093744126 },
	{ 0.0022453000364908151306742, 0.0023627216407049161234211, 0.0022184939850846759241965, -0.0019931360341758370263721, -0.0016930509407969700393093, -0.0019971140047042221753182 },
	{ 0.0017668461140228799814700, 0.0016352043552977839433599, 0.0016240461186107810025170, 0.0000936265481113410106065, 0.0001477102652532166966216, 0.0003036535040026405946403 },
	{ 0.0007396079992217538231006, 0.0006305053937615021997579, 0.0006754160943793497936041, 0.0002856482173624291082646, 0.0002544459810075444116013, 0.0001905295229581546907760 },
	{ 0.0016440606387078419634018, 0.0013642950216629899391069, 0.0014093577589404560242159, 0.0012587324401125840004495, 0.0011853272433576520807075, 0.0015756796901597090818786 },
	{ 0.0008900637681217453195046, 0.0006185498704119731048362, 0.0007319307689602907269696, 0.0013825488737403590806807, 0.0012678956175973120530653, 0.0014864518735865979871119 },
	{ 0.0020232973473198068853351, 0.0017488975068661941070225, 0.0017844943831109450621542, 0.0008392239316114295452631, 0.0008562328493389892870163, 0.0012142728065292569730044 },
	{ 0.0031067951306145520293245, 0.0029007090460470290500805, 0.0028347808300159000466367, -0.0003837931878478978843343, -0.0001231370222730341084001, 0.0001294963977677360072137 },
	{ -0.0007203061846395575340551, -0.0007813302205833687385031, -0.0006496540030173764395521, 0.0000964979311431598993061, 0.0001692184745280820895759, 0.0000710785675336021987181 },
	{ 0.0015855273509046499420122, 0.0014270168398394329420387, 0.0014516640417011109596979, -0.0003242686642892234072377, -0.0000980491672163410300572, -0.0000580479137382211831590 },
	{ 0.0017290208121829120195556, 0.0016331150712151070326233, 0.0016201498728297980024549, -0.0008535740001293892938172, -0.0005252278805681590435200, -0.0003915647530426407839019 },
	{ 0.0032436223214844790534162, 0.0030445041459569079783654, 0.0029684229250311680581298, -0.0006204649081148151581327, -0.0002932858753473277253071, 0.0000161314716302496200266 },
	{ 0.0012127273626806680823648, 0.0012525193078664119492732, 0.0012085622070287470580102, -0.0017033090693237319408065, -0.0013075369641882968966412, -0.0014671839293526429612302 },
	{ 0.0027588399505184160558080, 0.0025955999629835549598333, 0.0025125032043373401492714, -0.0007428111744552577623210, -0.0004113613170859154745916, -0.0000872315871275665004119 },
	{ 0.0035256751340424540355767, 0.0036104293184904359161058, 0.0033904346025428518801581, -0.0031916117483425520025275, -0.0024789397520912680519811, -0.0025307930289083470018485 },
	{ 0.0031778372559790268002400, 0.0033965211453842081464061, 0.0031379912482616630943488, -0.0038112143428847829011719, -0.0030478357889216511379671, -0.0032138921397304211213797 },
	{ 0.0025363406577781828256379, 0.0024903734048620988598965, 0.0023465949152156580573281, 0.0000975509878328383549500, 0.0001377955857028484898570, 0.0006251706828194343286767 },
	{ 0.0007483806512179109636171, 0.0006095456281237889495025, 0.0006290714486613326224262, 0.0017789718841155280348865, 0.0015191856785150559261410, 0.0021108800908352090205944 },
	{ 0.0024644250657482689309541, 0.0026650629421207218741618, 0.0024314699942631721324426, -0.0020742356939944688910826, -0.0017301123288976020912766, -0.0016255035476672691045763 },
	{ -0.0004573842287831006154890, -0.0000835296122009838016946, -0.0001842897504215927010750, -0.0021206727836978588735972, -0.0018674677546652910208047, -0.0021307617488644458279834 },
	{ 0.0035816053140259240458732, 0.0037406148707468771241746, 0.0034713401690829900729518, -0.0021454731981709828335381, -0.0017427680619839730838394, -0.0014355490440884819368372 },
	{ 0.0028948288629508578048699, 0.0029559076963859231244880, 0.0027860994293755769746213, -0.0008488181300136093520442, -0.0006376585476590839211483, -0.0001260105865189444093125 },
	{ -0.0003151458565148599244750, 0.0000336798557415235479931, -0.0000432019963245829370754, -0.0021138091082506401362229, -0.0018689475794980770547576, -0.0021944810640556092033726 },
	{ 0.0028836635600806509796556, 0.0025800696337754201042936, 0.0025583753793388531831698, 0.0020977101198942950388560, 0.0018587388109510679864989, 0.0026966646475971140471672 },
	{ -0.0000728737970281485771418, 0.0001646223991878965886831, 0.0001201559050747003976924, -0.0012602690560703430893569, -0.0011601764069583281064474, -0.0014194249313538511055033 },
	{ 0.0029455743353433371488348, 0.0029615646796361909387496, 0.0027939385897469770983492, -0.0011126852726515859635242, -0.0009136788021586876888322, -0.0007585667004805396896466 },
	{ 0.0029664750551697720640232, 0.0032029005551879388691283, 0.0029547264379139520293349, -0.0027353242451126748638335, -0.0022757139419920940914821, -0.0023151161037374958992507 },
	{ 0.0019727822690803749165755, 0.0021922066911803841232631, 0.0020140277262721642016763, -0.0023469215233074079002273, -0.0020342703221051840201117, -0.0022963787577264141263889 },
	{ 0.0016690790777556360931222, 0.0016473682975222609173316, 0.0015858204682941839675642, -0.0002933926829138879953558, -0.0002268446405429066104614, -0.0001216856174073231949655 },
	{ -0.0008239748072947118695541, -0.0007689319685366445902841, -0.0006854246153346978164336, 0.0002311976799028696977472, 0.0001140352079920627971239, -0.0001539703808309106889980 },
	{ 0.0004605029953018407854783, 0.0005646951127034865580007, 0.0005400251419233723267077, -0.0010318811612405659385722, -0.0009308333395950329179624, -0.0011664613110823880779560 },
	{ 0.0009843912575082085919531, 0.0011583811697399440995143, 0.0010837084140719569110012, -0.0015512661260150260346402, -0.0013305295093218180105948, -0.0015994341519944969050998 },
	{ -0.0007989306167405314709068, -0.0007301822419961745197145, -0.0006524382927906643026700, -0.0003031494072662331150554, -0.0003378357353356305822073, -0.0007006686976471967337465 },
	{ 0.0000231945498120209287840, 0.0000687416372551902379396, 0.0001154580675324709938909, -0.0003512884952753829903745, -0.0003217854147654371070077, -0.0005653321443630987194751 },
	{ 0.0027333942597942881801154, 0.0024763317775627368723390, 0.0024461409114613251111758, 0.0007018446064298519432995, 0.0006725007298878856202770, 0.0010116694815447519698731 },
	{ -0.0015561118728359660430549, -0.0015023508319062461001392, -0.0013689926880081099841435, 0.0000946264287727427595969, 0.0000627540035889385700452, -0.0002243020826553524871890 },
	{ 0.0023309698441479329879267, 0.0018734768987869099190341, 0.0019659157230142222140545, 0.0022681028588566981389618, 0.0021176216297021559567537, 0.0027675874684305787863658 },
	{ -0.0002026632856392443884138, -0.0004740017504635496924624, -0.0002992626330264818823416, 0.0017216609302040510130066, 0.0015155674090734480748133, 0.0016800450152895389684116 },
	{ 0.0013533984948918549411079, 0.0011387388632392750169309, 0.0012081492934430050796568, 0.0006487532512674559119839, 0.0007073702299015435747062, 0.0008907034663748575288242 },
	{ 0.0018413123558964641056368, 0.0015821323372555309936388, 0.0016378290823426569326748, 0.0006178339838034621221349, 0.0007310696863486777514335, 0.0010370259015168279987523 },
	{ -0.0006016666300790758778114, -0.0007389787867332399135673, -0.0005767601998900073788715, 0.0003957879319058999791070, 0.0004318067488064700827806, 0.0003790762951040913919347 },
	{ -0.0002002147620021364022788, -0.0004663427196567678954461, -0.0002779812291762388115623, 0.0012712577397143199915003, 0.0012236668410235121071117, 0.0012761233077909391042670 },
	{ 0.0006407906612945867127693, 0.0007129775602151144789712, 0.0007065995531657933651642, -0.0016409726000638029906609, -0.0012564358195789439645584, -0.0014807382767857669384620 },
	{ 0.0031291670733987900568374, 0.0030473371569509771772388, 0.0029250256448981530425779, -0.0015028147996619370310029, -0.0010362135024822309754822, -0.0008229935663536105472718 },
	{ 0.0023393855098193098475401, 0.0023070218345532860979019, 0.0022073007281105368540541, -0.0015053552966290749230704, -0.0010825217142873149768489, -0.0010229626974173479207753 },
	{ 0.0025507549700468021280919, 0.0023589146317682819359307, 0.0022742222223578211805217, 0.0012215821432399319879797, 0.0011002074859166629909601, 0.0017904743697990440493006 },
	{ 0.0015737034374582190357494, 0.0016414737128490459149022, 0.0015258088348872849825816, -0.0003791502976715471068259, -0.0003156762700646182154274, -0.0000238125830416788692454 },
	{ 0.0012331590833886559542554, 0.0015759056349649860299250, 0.0013849603370368369914534, -0.0027406780249032819916921, -0.0023270111004305609261888, -0.0025356896190133078848594 },
	{ 0.0031186121879060568068409, 0.0033861113993652299306025, 0.0031210117365454031514160, -0.0025922374812550421675772, -0.0021615009489886669556691, -0.0019645328916799891308398 },
	{ 0.0012268901105063959294100, 0.0010619431074195529417226, 0.0010917824875430759152400, 0.0014426114954557039179311, 0.0012622413815401899508939, 0.0017573721591663329921751 },
	{ 0.0032074709303337519024357, 0.0032214218845019819935371, 0.0030529207541684700612239, -0.0005769884951608773198187, -0.0004332087871889367869035, 0.0000617754807155490023385 },
	{ 0.0008204084432902874080701, 0.0007231325649009822482613, 0.0007593332933704400851752, 0.0013830041147601429725456, 0.0011417730359064249753831, 0.0015660985586439079685767 },
	{ 0.0013044385521008880057109, 0.0014267961562930600149385, 0.0013453453209147920970057, -0.0010432299596228509185047, -0.0009003106685032483949213, -0.0009765125664075881674678 },
	{ -0.0005748264961353750403250, -0.0006559033737282596006682, -0.0005443893144493645993784, 0.0015243868688684639092074, 0.0012457461929101870060782, 0.0014542579155931809487684 },
	{ 0.0021253085862955960715082, 0.0020260818803439569098235, 0.0019745603201054141677406, 0.0000298186022793179013692, 0.0000580329911322512319630, 0.0002360862911301539089019 },
	{ 0.0017493146534268910145121, 0.0019414824381972330732266, 0.0017946238729610950856513, -0.0018722907703909599883701, -0.0015624751974465920806995, -0.0015934837034282760531018 },
	{ -0.0004035512553141673086979, -0.0003064376957915475947931, -0.0002626714264878639792065, -0.0004403990215865278765039, -0.0004179482662053272761481, -0.0006397597577879301144685 },
	{ 0.0029889048145682679986923, 0.0026853545153121879257574, 0.0026582257402995228094678, 0.0013829840246542789150563, 0.0012710104921417530662864, 0.0019250666625197458952518 },
	{ 0.0007348275205093360079242, 0.0007438245777413221484359, 0.0007476396215562382567207, -0.0004766809315123848021269, -0.0003853047299993468829092, -0.0004786590925356970808655 },
	{ 0.0004620277166298595119577, 0.0006104528518846678852849, 0.0005867939357311883390514, -0.0015709868646308960873220, -0.0013325917021598729372833, -0.0016591128377243099731309 },
	{ -0.0009798254419355648895534, -0.0010610050042845879398473, -0.0008954731866442059871544, 0.0011488148050372139295089, 0.0009475280600444831646464, 0.0009007333163720346053821 },
	{ 0.0020046399742177228864293, 0.0022302734875184150251104, 0.0020730021200389379953943, -0.0030124328084106121068697, -0.0024492722913740288051554, -0.0027240344798293548121693 },
	{ -0.0003870971960440726150794, -0.0003373859136443047935031, -0.0002622329361830582055565, -0.0007815532788369408852269, -0.0006089582190162867074770, -0.0008164968065984990353706 },
	{ 0.0021823987571998760409686, 0.0020511816478946520020399, 0.0020340116902619379511685, -0.0001732871182218286986061, 0.0000000000000000000000000, 0.0002740688264682986162273 },
	{ 0.0003046678627523015142793, -0.0001094043898520695971702, 0.0000910280539404340657394, 0.0026153107985541637972582, 0.0023264046833655410359554, 0.0026706784080083368661251 },
	{ 0.0034788783832906400306229, 0.0031939517002769019990427, 0.0031466660003809389899165, 0.0003383042629986949168149, 0.0005387530418578653692435, 0.0011864452997842900045661 },
	{ 0.0001115851319019888935680, -0.0000883246834377320836230, 0.0000514697274923127200521, 0.0009470586883684206126743, 0.0009153992361189105652491, 0.0011065907581445000480713 },
	{ -0.0000677769844562231343291, -0.0000744410519665587698144, 0.0000000000000000000000000, -0.0007298358165556458748702, -0.0005080400340977331763090, -0.0005810723814096307353674 },
	{ 0.0034660260935176988826567, 0.0029668988909759071333216, 0.0029361883008308608221382, 0.0032679657287175141282265, 0.0029231802025177291981528, 0.0040963101446630642218927 },
	{ 0.0015294996766996330969657, 0.0013804911535035980368891, 0.0013507261357052070642470, 0.0015691703495601329494680, 0.0013421706622706529830891, 0.0019127147285763590033730 },
	{ 0.0024495915276183128500564, 0.0028057517978807228545279, 0.0025529079122406290493219, -0.0030955150424680929165111, -0.0026187197636612482079532, -0.0026618645096379220382765 },
	{ 0.0000697264270248890713835, 0.0003643480739223890047494, 0.0003024053605845772817705, -0.0015149378515304089565935, -0.0013445837584019789106432, -0.0015021239248113650317090 },
	{ 0.0008031287445404274244823, 0.0010559501598801500788644, 0.0009507140279501314315824, -0.0017610897854962190376554, -0.0015235797539920878981151, -0.0015749527661354930600096 },
	{ 0.0012660472689578210377104, 0.0013924494815822409127942, 0.0013205165726759640360682, -0.0007844887207778066494368, -0.0006740153409431197959259, -0.0005131391439321678548877 },
	{ 0.0009216476719402058896333, 0.0009911117579095530746858, 0.0009652485035395613234555, -0.0002847940549794049781666, -0.0002716479013424486182830, -0.0001933338778940948087231 },
	{ 0.0000885810346175073040891, 0.0002382006483329925875664, 0.0002379802476689612080515, -0.0005105492328002327658693, -0.0004605068310548696814376, -0.0004221617648331961784001 },
	{ 0.0042100885323088652403833, 0.0041295157246198721870400, 0.0039430114293749901621666, -0.0006328579945669820638193, -0.0004351248361243827871350, 0.0000357948790220612112975 },
	{ 0.0002803926851261606873790, 0.0001313616636667700907862, 0.0002262210803528276914943, 0.0014788299791035910361187, 0.0012205229689110249063216, 0.0014206670092928080830624 },
	{ 0.0010642312641421540139741, 0.0009667532860466778723771, 0.0010001351040679190822907, 0.0006244489521895338129606, 0.0005723984916086186178902, 0.0008616485283088536019022 },
	{ 0.0010005392041938089391084, 0.0009290111208950942688067, 0.0009497048461154755346139, 0.0003707757704737876889439, 0.0003153458680577800863666, 0.0004663853137991527198283 },
	{ 0.0011733092256733389409373, 0.0009990897987157036988504, 0.0010566165975932800043940, 0.0009351219088298318334040, 0.0008290156479564633361032, 0.0009964075122546403335405 },
	{ 0.0020293915325651349680636, 0.0019071179480345439374855, 0.0018937957988547629226139, -0.0000981036024826206158166, 0.0000644736525526525293189, 0.0004306821609662648880565 },
	{ 0.0026926869710865572113412, 0.0024988246490741401056868, 0.0024647467365463398419889, 0.0001872024161515711034986, 0.0002837299514683703258004, 0.0006243230916251122069455 },
	{ -0.0007905055001677703139726, -0.0009355845669930569355813, -0.0007697005390940889289070, 0.0012895300096620290926375, 0.0011441282050416010009769, 0.0012787813756021399868440 },
	{ 0.0016053648592664899585197, 0.0015494722948076571078513, 0.0015441688052060110851460, -0.0005834066823523258648806, -0.0003587142082216816991493, -0.0002684635132002554931512 },
	{ 0.0017727095719745819806662, 0.0014748060405263519693358, 0.0015546576006778380692891, 0.0011984786603960210712994, 0.0012053866603611749445252, 0.0016942707388501339598602 },
	{ 0.0012140591086912859777880, 0.0008325643985343338365074, 0.0009675102585206747955390, 0.0018521842612664659189631, 0.0017368407377646799885640, 0.0021700368521211238492796 },
	{ 0.0008007943858867417946321, 0.0005797850843821981481688, 0.0006877422301546165864239, 0.0006011921388975620308276, 0.0006792921177785856724773, 0.0009288825537327306900576 },
	{ 0.0001816812229188043983051, 0.0004434668974171131141000, 0.0003876661043659339160594, -0.0029199781533084567981817, -0.0023921620641751089979266, -0.0028427454330909739284305 },
	{ 0.0000213116344910481784732, 0.0001490753149065841040143, 0.0001511179696160880054755, -0.0018311941411703371029862, -0.0014739412174583060161082, -0.0017746924993150999232822 },
	{ 0.0020400482279142738602229, 0.0020663460079161010893301, 0.0019722399252988850187895, -0.0018303430234861090084714, -0.0014062030391074390421602, -0.0014235362277887520388725 },
	{ 0.0044596133081487949675892, 0.0047159429052251100253357, 0.0043452809187762859305382, -0.0044931873040991452197823, -0.0035927747834416609053199, -0.0036686479055434641390054 },
	{ 0.0027618979914087798607780, 0.0022013501569809099632502, 0.0022412156726251391408722, 0.0040399157306611824247433, 0.0035191737392320831048775, 0.0047017492332420878289878 },
	{ 0.0018645938435884099569628, 0.0015261756842097919627255, 0.0015416908375411070720312, 0.0027753154204663470563941, 0.0023746161975382509140564, 0.0032449566069598359194237 },
	{ 0.0029717394166038288427778, 0.0029870565542640540104835, 0.0028263615306745861963100, -0.0007145620001132694945958, -0.0005070864473685083575680, -0.0000502374918629612968808 },
	{ -0.0003620444494360935982526, 0.0000487261138297661973635, -0.0000492932133694490729543, -0.0025580471164716378149051, -0.0022679551032484209817375, -0.0026322842965290178460225 },
	{ 0.0023829936070315070086034, 0.0024011416695996950013137, 0.0022915875194552989650709, -0.0003548882171269965930989, -0.0002182721062345079120291, 0.0002333099060999873979819 },
	{ 0.0003548830178458081208506, 0.0004038304850464487943430, 0.0004144526071335549060638, 0.0001797541710313895044118, 0.0001200995559606956019369, 0.0003835095845657601949276 },
	{ 0.0020361030822204357997041, 0.0018896940571472330486213, 0.0018684591850481021071961, 0.0009431459434877719014839, 0.0008415099420167789770128, 0.0014969451965125390568295 },
	{ 0.0020057253378930679950376, 0.0020963723393490701039921, 0.0020024577958883240172416, -0.0009490970192497018590935, -0.0007510790704651691641544, -0.0004992687653775103708356 },
	{ -0.0002354849881614618065020, -0.0002125042727614330929226, -0.0001493037775964337120303, 0.0006463039409304306660495, 0.0004721828585015557877905, 0.0006174112363322341659466 },
	{ 0.0023680715132771510550436, 0.0025745096855247619552221, 0.0024146306470949918145108, -0.0021014645808125351558737, -0.0017088596474846499528172, -0.0016310442277678240292904 },
	{ 0.0011247022972637320693129, 0.0012435447633922049916200, 0.0011972544026157589010240, -0.0009467324547273433870895, -0.0007416515636080516860398, -0.0007015701196147100570169 },
	{ 0.0027278860139003020292281, 0.0025108836568757527882290, 0.0024828410067168618299838, 0.0007500641077549849954076, 0.0007994483182191811695277, 0.0013859201509695739853306 },
	{ 0.0014356827428556140184568, 0.0013260064326264670020450, 0.0013527167249752941081964, 0.0001989821544866722882478, 0.0003063996666912963042634, 0.0005730718058821735773345 },
	{ -0.0008344916814520519357176, -0.0009547590267963007581997, -0.0007888210190744407998911, 0.0012487822854572040286864, 0.0010385046754844040779731, 0.0011061492057957819559050 },
	{ 0.0020347554462569888977796, 0.0020822911786249971750895, 0.0020202332772798307913842, -0.0013763976464001550972099, -0.0009968360281456100507880, -0.0008847902524244682541041 },
	{ -0.0006840625752163777143280, -0.0005479990198255897063812, -0.0005003066920762046709478, -0.0012205428500289359355913, -0.0010192137358283799315056, -0.0012443116244042400162645 },
	{ 0.0037211208367260830212619, 0.0035600456689823148197227, 0.0034510925615005021323267, -0.0005779116730646919426403, -0.0002297144399660369130933, 0.0003149443249143022219294 },
	{ -0.0001836967733043168055258, -0.0003805413974580069101650, -0.0002252402523613695995656, 0.0012349649568742840886892, 0.0011725057107398440502111, 0.0013910255573962209199590 },
	{ 0.0019802764140652768555528, 0.0014673339512656609958524, 0.0015374127496791940936799, 0.0039480461294860999879597, 0.0034361940337546549410763, 0.0045514618153627050059495 },
	{ 0.0023094818528929779780090, 0.0018427881119870859888410, 0.0018757064574625680313663, 0.0035231798001869141061249, 0.0030749268047159362092502, 0.0041736650169985111194504 },
	{ -0.0002090268937112795021634, -0.0004864268937043485165235, -0.0003623149275203342198869, 0.0031408541010298938675471, 0.0026180419291228188179155, 0.0032621778863996349004384 },
	{ -0.0007180993273445446288886, -0.0007587024553346799959561, -0.0006872927066518523140856, 0.0015555161653878020380087, 0.0012320163710721520125557, 0.0015539103333671320031351 },
	{ 0.0013450640814803209530798, 0.0015522673116324580896985, 0.0014135091960032770529859, -0.0016286128296966019893266, -0.0013728964306474748975628, -0.0012785633962471060123128 },
	{ 0.0003239284773280550855205, 0.0007455200532570787101336, 0.0006106344266038481486888, -0.0031068794819010931394787, -0.0026643923518631339175666, -0.0030273888595353419875278 },
	{ 0.0003970276183227822901570, 0.0005068832851126209770518, 0.0005018344674932383956550, -0.0001522453302557943001332, -0.0001843058770757942128188, -0.0000322021071213782292958 },
	{ 0.0007359526819472104021760, 0.0004246008638231623809507, 0.0005384658574363736542254, 0.0026660773145091988360622, 0.0023087749022270608392771, 0.0029725795120393461862252 },
	{ 0.0009751343529079377955154, 0.0010492055817238460629220, 0.0010209123475599750623805, -0.0008659331856243624976968, -0.0006873152052438385008551, -0.0006138065377951318303193 },
	{ -0.0003403608069703901962619, -0.0003554644900984246227646, -0.0002491155134490326966690, 0.0000884767061741419601987, 0.0001396085465256011106262, 0.0001361692295772242865173 },
	{ 0.0003181629711399116861199, 0.0005295761929275632747330, 0.0004920280530147492061357, -0.0018793064785894420237050, -0.0015348108021875890918939, -0.0016589763284248499729950 },
	{ 0.0004367727285794842178278, 0.0001178484968890020041310, 0.0002551878712795777092229, 0.0021437555293675409143206, 0.0019372339210267321054498, 0.0024325183017327119995576 },
	{ 0.0012057006521248589750450, 0.0007879675372043771055044, 0.0008816915105695535262095, 0.0034146220601146350277777, 0.0029627021184993850011491, 0.0038460141797692108320272 },
	{ 0.0010860778983414890504905, 0.0009766559706477259249907, 0.0009693079466466173041081, 0.0012364182690503720211256, 0.0010611659389029310784219, 0.0015995973647405529929205 },
	{ 0.0012095524148944309002218, 0.0015996433369780660958825, 0.0013967210263945289971943, -0.0034047318457833051878747, -0.0028896707631872560126385, -0.0031648455004358159482758 },
	{ 0.0026389723848216700349822, 0.0025687994918570751165521, 0.0024672407678364280045336, -0.0001525188451202090088744, -0.0000303640127923736287451, 0.0005243206931766527904848 },
	{ 0.0018651570857431110615765, 0.0023094175855547629042097, 0.0020555781204312059858019, -0.0038071113248426382072442, -0.0032091614806387569643065, -0.0033924822736490201600945 },
	{ 0.0014400713094644850865989, 0.0014443952789496229596050, 0.0013974844951892850287989, -0.0002253196750704867092150, -0.0001426407693766709098952, 0.0002215199269472988030140 },
	{ -0.0011942344437551859898106, -0.0011502999403271479874483, -0.0010375198138877729167606, 0.0004577703245562355954985, 0.0003181478996971223737779, 0.0003111305975329959130904 },
	{ -0.0009176014594056470814049, -0.0007745658180804833011293, -0.0007041448295650789288361, -0.0004415610456609904877108, -0.0004221595011345677839372, -0.0005458471883081795084786 },
	{ 0.0010848543432604919756895, 0.0011874339000597809376525, 0.0011344899634130900002715, -0.0008694156406658119697756, -0.0006778577825875257327828, -0.0005362793033739174245259 },
	{ 0.0014039448367115870200394, 0.0010185846987863849885669, 0.0011257749597574709388148, 0.0024351976079214901135805, 0.0021709280176012810299746, 0.0027866457223065538989504 },
	{ 0.0011409363581790770564295, 0.0008402439473827049039312, 0.0009371018098305600044565, 0.0017658003739963680413932, 0.0016168741370231230151250, 0.0021310427284823379133483 },
	{ -0.0016747801696734129685978, -0.0014738408561876679882946, -0.0014041323574970920352867, -0.0010078703518819620565949, -0.0009118407877945530325831, -0.0013446619149397550686820 },
	{ 0.0010759616521528489289927, 0.0007619670138152006156479, 0.0008862006934989686115015, 0.0017716750970323479844742, 0.0016616642076719000554325, 0.0022288930529388581783246 },
	{ -0.0003962573071610094206652, -0.0007025998703228573342289, -0.0005128975820582054589991, 0.0022610468355991268467331, 0.0020304031653886711432377, 0.0024424331460163820756160 },
	{ -0.0015123167830031389834156, -0.0015695882614504919148196, -0.0013982486476371589010498, 0.0006755187027954840869870, 0.0006180442797446006331086, 0.0005620382247068475859916 },
	{ 0.0033261776753688208040083, 0.0035766298110091309520042, 0.0032856751595885598689339, -0.0040701768638194716840029, -0.0032728977297306269256261, -0.0034024284669892899957144 },
	{ 0.0018646443786071930596299, 0.0012619678900883740061550, 0.0013865427818552579931205, 0.0046177270691359316445723, 0.0040356301284398187964597, 0.0051807139095328439901134 },
	{ 0.0009736282969345117378063, 0.0007437133923326399130571, 0.0007847244785948350852961, 0.0019007869485368909714257, 0.0016710789981386339145969, 0.0022361028162993551622606 },
	{ -0.0003367570775849846893141, -0.0005113956135480949172917, -0.0004055868890637428082584, 0.0019821234103755977842831, 0.0016861636208523689359901, 0.0020742314640671130274163 },
	{ 0.0016238213495167509465428, 0.0016249001869066909478634, 0.0015637201695462888979132, -0.0003137165705103545239839, -0.0001895319374264862067464, 0.0002381001402774257029015 },
	{ 0.0018412839456077700200920, 0.0019635397230954180006202, 0.0018376322174972279776400, -0.0013943243333181518962827, -0.0010927030682533380844673, -0.0008636497994413490277704 },
	{ 0.0004883439363731364958252, 0.0007046721733139896162029, 0.0006318951290983876268800, -0.0017383577430109390694890, -0.0014695368418765990519564, -0.0015177908261647670047634 },
	{ -0.0000342742935739115430730, 0.0000799908080257986535194, 0.0001005025479081464055350, -0.0005699791613249457784135, -0.0004833445710603219945545, -0.0003702107136054805821938 },
	{ -0.0012020427319328120564779, -0.0012588487504980160752721, -0.0011115051047489909932103, 0.0008249441313588411033020, 0.0007237476979218754108078, 0.0007112071631332183554650 },
	{ 0.0008111454638008642374983, 0.0005159559577679896083188, 0.0006175178555793522016878, 0.0017430534632754250169706, 0.0015951683038523589621555, 0.0021150831306674020525216 },
	{ -0.0009296614466215847964736, -0.0008451153223452837273677, -0.0007651677030471453226299, -0.0008556116183956875669014, -0.0006742442935784611875508, -0.0008756461608712527320761 },
	{ 0.0028209909924886348957240, 0.0032402707570423862001863, 0.0029219517919356380493412, -0.0050101678982639530693377, -0.0040913852254852965673049, -0.0044283484236274399045707 },
	{ 0.0036696518638977778693777, 0.0042565728614509064017812, 0.0038209853404052572002636, -0.0066794487458273999955227, -0.0054773862958042499049505, -0.0059319297534479861408463 },
	{ 0.0009433059662546770416866, 0.0004677226505037865842730, 0.0006052150807902983726400, 0.0040448582502417831582853, 0.0035314661531770509365902, 0.0044670789209457001031622 },
	{ -0.0011971972548577619523819, -0.0015747325629938649805412, -0.0013407969435640810368371, 0.0039559580136653846663131, 0.0033451980112451009524710, 0.0038705216070275549888891 },
	{ 0.0007643355400297291522144, 0.0009106143720667754124995, 0.0008486909466491425729079, -0.0013801419397149611040937, -0.0011113667492406241032316, -0.0009601469088597266738488 },
	{ 0.0014056489838133560954514, 0.0012854173877343050193950, 0.0012845304684010949106826, 0.0005530760572424156702731, 0.0006106082923800948587811, 0.0010900627269172930011720 },
	{ 0.0029159920797984760498789, 0.0027540828971048540892153, 0.0026705400255899918240954, 0.0000330485854215356900944, 0.0002074840095696482093063, 0.0008140138515531017285656 },
	{ 0.0014444928799980229303795, 0.0011190376756648869136651, 0.0011834036452983709170744, 0.0019072246942151449035363, 0.0017797546185282819061063, 0.0024417828715568678892012 },
	{ 0.0020339918047425180336329, 0.0016282521752227470737684, 0.0016970850711784510236185, 0.0020864693296620872089986, 0.0019609714713644441595741, 0.0027070061319480350457700 },
	{ 0.0032909924604606418004071, 0.0025819698953125917902096, 0.0026504712444989529315786, 0.0045328869571675690225421, 0.0040766591571012667297169, 0.0054605693588597806198415 },
	{ 0.0024874710431952561789926, 0.0023457846597697358699552, 0.0022697651404475428955909, -0.0000484895007095566916641, 0.0001369849530786475125602, 0.0006654380544996555150658 },
	{ 0.0028192213642440509466236, 0.0023118835147130198555410, 0.0023444574707988511709511, 0.0026291387248563889353359, 0.0024654040339014901550563, 0.0034770174883679331144515 },
	{ 0.0051371533587632293993419, 0.0055319962795393915963582, 0.0050664318118455731188021, -0.0058587808663106378734287, -0.0047052598034550583913638, -0.0047732969629555596982606 },
};
  uint16_t lastRow, lastCol;
  uint16_t subsamples[NUM_SUBSAMPLE];
  float x;
  float ah[NUM_HIDDEN];
  int adc_idx = 0;
  
  for (int i = 0; i < NUM_HIDDEN; i++)  {
    ah[i] = bh[i] / 255;
  }
  
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(REG_ROWSEL, mask[0][0], CAM1);
  set_pointer_value(REG_COLSEL, 0, CAM1);
  
  lastRow = mask[0][0];
  lastCol = 0;

  for (int pixel = 0; pixel < NUM_SUBSAMPLE; pixel++) {
      if (mask[pixel][0] != lastRow)
      {
        char diff = mask[pixel][0] - lastRow;
        
        inc_pointer_value(REG_ROWSEL, diff, CAM1);
        
        lastRow = mask[pixel][0];

        set_pointer_value(REG_COLSEL, mask[pixel][1], CAM1);
        
        lastCol = mask[pixel][1];
        
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      } else {
        inc_value(mask[pixel][1] - lastCol, CAM1);
        
        lastCol = mask[pixel][1];
      }
      
      CAM1_INPH_BANK->ODR |= CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM1_INPH_BANK->ODR &= ~CAM1_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      /* Start ADC1 Software Conversion */
      ADC_SoftwareStartConv(ADC1);
      
      if (pixel > 0) {
        x = (float)(subsamples[pixel - 1]) / 255;

        for (int i = 0; i < NUM_HIDDEN; i++) {
            ah[i] += x * wih[pixel - 1][i];
        }
      }
      else {
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      }
      
      subsamples[pixel] = adc_values[adc_idx];
      adc_idx = !adc_idx;
  }
  
  x = (float)(subsamples[NUM_SUBSAMPLE - 1]) / 255;

  for (int i = 0; i < NUM_HIDDEN; i++) {
      ah[i] += x * wih[NUM_SUBSAMPLE - 1][i];
  }
  
  finish_predict(ah);
  
  return 0;
}

