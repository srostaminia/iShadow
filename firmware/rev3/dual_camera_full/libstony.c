#include "libstony.h"
#include "stm32l1xx.h"
#include "main.h"
#include "diskio.h"

extern __IO  uint32_t Receive_length ;
extern uint32_t sd_ptr;

extern volatile uint16_t time_start, time_total;

int adc_idx = 0;
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
//  config = 16;  // FIXME!!!

  //turn chip on with config value
  set_pointer_value(REG_CONFIG,config,CAM1);
  set_pointer_value(REG_CONFIG,config,CAM2);
  
  dac_init();
}

void dac_init() {
  GPIO_InitTypeDef GPIO_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Configure PA.04 (DAC_OUT1), PA.05 (DAC_OUT2) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  DAC_DeInit();
  
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  
  /* DAC Channel2 Init */
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);
  DAC_Cmd(DAC_Channel_1, ENABLE);
  
  DAC_SetChannel2Data(DAC_Align_12b_R, LED_HIGH);
  DAC_SetChannel1Data(DAC_Align_12b_R, LED_HIGH);
}

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
//  set_pointer_value(REG_COLSEL, 0, CAM1);
//  set_pointer_value(REG_COLSEL, 0, CAM2);

  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
    set_pointer_value(REG_COLSEL, 0, CAM1);
    set_pointer_value(REG_COLSEL, 0, CAM2);
//    set_pointer_value(REG_ROWSEL, 0, CAM1);
//    set_pointer_value(REG_ROWSEL, 0, CAM2);
    
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
//    inc_pointer_value(REG_COLSEL, 1, CAM1);
//    inc_pointer_value(REG_COLSEL, 1, CAM2);

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
//  uint16_t min = 65535, max = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
#ifdef COLUMN_COLLECT
  set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
#else
  set_pointer_value(REG_ROWSEL, 0, SINGLE_CAM);
#endif

  for (int row = 0, data_cycle = 0; row < 112; row++, data_cycle++) {
#ifdef COLUMN_COLLECT
    set_pointer_value(REG_ROWSEL, 0, SINGLE_CAM);
#else
    set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
#endif
    delay_us(1);
    
    for (int col = 0; col < 112; col++) {      
      SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do conversion for 
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      buf16[(data_cycle * 112) + col] = adc_values[adc_idx];
      
//      min = (adc_values[adc_idx] < min) ? (adc_values[adc_idx]) : (min);
//      max = (adc_values[adc_idx] > max) ? (adc_values[adc_idx]) : (max);
      
      adc_idx = !adc_idx;

      SINGLE_PARAM(INCV_BANK)->ODR |= SINGLE_PARAM(INCV_PIN);
      SINGLE_PARAM(INCV_BANK)->ODR &= ~SINGLE_PARAM(INCV_PIN);
    } // for (col)
    
#ifdef COLUMN_COLLECT
    inc_pointer_value(REG_COLSEL, 1, SINGLE_CAM);
#else
    inc_pointer_value(REG_ROWSEL, 1, SINGLE_CAM);
#endif

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
  
//  last_max = max;
//  last_min = min;
  
  return 0;
}

int stony_cider_line(uint8_t rowcol_num, uint8_t *sd_buf, uint8_t rowcol_sel)
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
  // Double-buffered (2-dim array)
//  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint16_t *buf16 = (uint16_t *)sd_buf;
  
  volatile uint16_t start, total;
  
  if (rowcol_sel == 0) {
    set_pointer_value(REG_COLSEL, rowcol_num, SINGLE_CAM);
    set_pointer_value(REG_ROWSEL, 0, SINGLE_CAM);
  } else {
    set_pointer_value(REG_ROWSEL, rowcol_num, SINGLE_CAM);
    set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
  }
  
  delay_us(1);
  
  for (int i = 0; i < 112; i++) {      
    SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    // Do conversion for SINGLE_CAM
    ADC_SoftwareStartConv(ADC1);
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    buf16[i] = adc_values[adc_idx];
    adc_idx = !adc_idx;
    
    SINGLE_PARAM(INCV_BANK)->ODR |= SINGLE_PARAM(INCV_PIN);
    SINGLE_PARAM(INCV_BANK)->ODR &= ~SINGLE_PARAM(INCV_PIN);
  } // for (col)
  
  return 0;
}

int stony_single_pixel(uint8_t loc[2], uint16_t reps, uint16_t delay, uint8_t *sd_buf)
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
  // Double-buffered (2-dim array)
//  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint16_t *buf16 = (uint16_t *)sd_buf;
  
  volatile uint16_t start, total;
  
  set_pointer_value(REG_ROWSEL, loc[0], SINGLE_CAM);
  set_pointer_value(REG_COLSEL, loc[1], SINGLE_CAM);
  
  delay_us(1);
  
  for (int i = 0; i < reps; i++) { 
//    set_pointer_value(REG_ROWSEL, loc[0], SINGLE_CAM);
//    set_pointer_value(REG_COLSEL, loc[1], SINGLE_CAM);
//    
//    delay_us(1);
    
    time_start = TIM5->CNT;
    SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    // Do conversion for SINGLE_CAM
    ADC_SoftwareStartConv(ADC1);
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    buf16[i] = adc_values[adc_idx];
    adc_idx = !adc_idx;
    
    time_total = TIM5->CNT - time_start;
    if (delay != 0)
      delay_us(delay);
    
//    if (i == 0) {
//      Delay(20);
  } // for (col)
  
  return 0;
}

int stony_mask_subsample(uint8_t *row, uint8_t *col, uint32_t num_pixels, uint8_t *sd_buf)
{
  uint16_t *buf16 = (uint16_t *)sd_buf;
  
  int16_t last_row = -1, last_col = -1;
  for (uint32_t i = 0; i < num_pixels; i++) {
//    if (last_col == -1 || last_col != col[i]) {
//      set_pointer_value(REG_COLSEL, col[i], SINGLE_CAM);
//      set_pointer_value(REG_ROWSEL, row[i], SINGLE_CAM);
//      
//      last_col = col[i];
//      last_row = row[i];
//    } else {
//      for (uint8_t j = 0; j < (row[i] - last_row); j++)
//        pulse_incv(SINGLE_CAM);
//      
//      last_row = row[i];
//    }
    
    set_pointer_value(REG_COLSEL, col[i], SINGLE_CAM);
    set_pointer_value(REG_ROWSEL, row[i], SINGLE_CAM);
  
    SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    // Do conversion for SINGLE_CAM
    ADC_SoftwareStartConv(ADC1);
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    buf16[i] = adc_values[adc_idx];
    adc_idx = !adc_idx;
  }
  
  set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
  set_pointer_value(REG_ROWSEL, 0, SINGLE_CAM);
}

int stony_mask_fullsample(uint8_t *row, uint8_t *col, uint32_t num_pixels, uint8_t *sd_buf)
{
  uint16_t *buf16 = (uint16_t *)sd_buf;
  uint32_t pix_idx = 0;
  
  set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
  
  int16_t last_row = -1, last_col = -1;
  for (uint8_t col_idx = 0; col_idx < 112; col_idx++) {
    set_pointer_value(REG_ROWSEL, 1, SINGLE_CAM);
    
//    delay_us(1);
    
    for (uint8_t row_idx = 1; row_idx < 112; row_idx++) {
      SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do conversion for SINGLE_CAM
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      if (row[pix_idx] == row_idx && col[pix_idx] == col_idx) {
        buf16[pix_idx] = adc_values[adc_idx];
        pix_idx++;
      }
      
      adc_idx = !adc_idx;
      
      if (pix_idx == num_pixels)
        return 0;
      
      SINGLE_PARAM(INCV_BANK)->ODR |= SINGLE_PARAM(INCV_PIN);
      SINGLE_PARAM(INCV_BANK)->ODR &= ~SINGLE_PARAM(INCV_PIN);
    }
    
    inc_pointer_value(REG_COLSEL, 1, SINGLE_CAM);
  }
  
  set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
  set_pointer_value(REG_ROWSEL, 0, SINGLE_CAM);
}