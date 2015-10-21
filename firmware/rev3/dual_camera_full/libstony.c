#include "libstony.h"
#include "stm32l1xx.h"
#include "main.h"
#include "diskio.h"
#include "math.h"
#include "predict_gaze.h"

extern __IO  uint32_t Receive_length ;
extern uint32_t sd_ptr;

extern volatile uint16_t time_start, time_total;

int adc_idx = 0;
__IO uint16_t adc_values[2];

#ifdef CIDER_MODE
extern unsigned short num_subsample;
extern unsigned short num_hidden;
extern unsigned int bh_offset;
extern unsigned int bo_offset;
extern unsigned int mask_offset;
extern unsigned int who_offset;
extern unsigned int wih_offset;
extern unsigned int fpn_offset;
extern unsigned int col_fpn_offset;

unsigned short model_data[];

extern int8_t pred[2];
extern float pred_radius;
float last_r = 0;
#endif

#ifdef OUTDOOR_SWITCH
extern uint8_t outdoor_mode;
#endif

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

#ifdef CIDER_MODE
  num_subsample = model_data[0];
  num_hidden = model_data[1];
  
  bh_offset = 2;
  bo_offset = bh_offset + num_hidden * 2;
  mask_offset = bo_offset + 4;
  who_offset = mask_offset + num_subsample * 2;
  wih_offset = who_offset + num_hidden * 2 * 2;
  fpn_offset = wih_offset + (num_hidden * num_subsample * 2);
  col_fpn_offset = fpn_offset + 112 * 112;
#endif
  
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
#if !defined(OUTDOOR_SWITCH)
        buf16[(data_cycle * 112) + cam2_offset + (col - 1)] = adc_values[1];
#else
        if (row == 0 && col == 1) {
          buf16[(data_cycle * 112) + cam2_offset + (col - 1)] = outdoor_mode;
        } else {
          buf16[(data_cycle * 112) + cam2_offset + (col - 1)] = adc_values[1];
        }
#endif
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
      
#if !defined(OUTDOOR_SWITCH)
      buf16[(data_cycle * 112) + col] = adc_values[0];
#else
      if (row == 0 && col == 0) {
        buf16[(data_cycle * 112) + col] = outdoor_mode;
      } else {
        buf16[(data_cycle * 112) + col] = adc_values[0];
      }
#endif
      
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

//int stony_cider_line(uint8_t rowcol_num, uint16_t *line_buf, uint8_t rowcol_sel)
//{
//  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//  // Double-buffered (2-dim array)
////  uint8_t buf8[2][112 * TX_ROWS * 2];  
//  volatile uint16_t start, total;
//  
//  if (rowcol_sel == SEL_COL) {
//    set_pointer_value(REG_COLSEL, rowcol_num, CAM2);
//    set_pointer_value(REG_ROWSEL, 0, CAM2);
//  } else {
//    set_pointer_value(REG_ROWSEL, rowcol_num, CAM2);
//    set_pointer_value(REG_COLSEL, 0, CAM2);
//  }
//  
//  delay_us(1);
//  
//  for (int i = 0; i < 112; i++) {      
//    CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    
//    // Do conversion for SINGLE_CAM
//    ADC_SoftwareStartConv(ADC1);
//    
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//    
//    if (rowcol_sel == SEL_COL)
////      line_buf[i] = adc_values[adc_idx] - COL_FPN((i * 112) + rowcol_num);
//      line_buf[i] = adc_values[adc_idx] - COL_FPN((rowcol_num * 112) + i);
//    else
//      line_buf[i] = adc_values[adc_idx] - ROW_FPN((rowcol_num * 112) + i);
//    
//    adc_idx = !adc_idx;
//    
//    CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
//    CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
//  } // for (col)
//  
//  return 0;
//}

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
  
//  int16_t last_row = -1, last_col = -1;
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
  
  return 0;
}

int stony_mask_fullsample(uint8_t *row, uint8_t *col, uint32_t num_pixels, uint8_t *sd_buf)
{
  uint16_t *buf16 = (uint16_t *)sd_buf;
  uint32_t pix_idx = 0;
  
  set_pointer_value(REG_COLSEL, 0, SINGLE_CAM);
  
//  int16_t last_row = -1, last_col = -1;
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
  
  return 0;
}
//
//int run_cider(uint8_t *cider_xy)
//{
//  uint8_t row_edges[6] = {0, 0, 0, 0, 0, 0}, col_edges[6] = {0, 0, 0, 0, 0, 0};
//  int pupil_found = -1;
//  
//  cider_xy[1] = (uint8_t)((pred[1] < 0 ? 0 : pred[1]) > 111 ? 111 : pred[1]);
//  cider_xy[0] = (uint8_t)((pred[0] < 0 ? 0 : pred[0]) > 111 ? 111 : pred[0]);
//  
//  uint8_t col_start = cider_xy[1], row_start = cider_xy[0];
//  
//  float best_ratio = 0, best_r = 0;
//  uint8_t best_center[2] = {0, 0};
//  
//  // Configure ADC to read only from eye camera
//  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
//  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
//  
//  uint16_t row[112], col[112];
//  stony_cider_line(col_start, row, SEL_ROW);
//  stony_cider_line(row_start, col, SEL_COL);
//  
//  // Reconfigure ADC for dual-camera reading
//  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
//  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles); // AMM
//  
//  find_pupil_edge(row_start, row_edges, row);
//  find_pupil_edge(col_start, col_edges, col);
//
//  for (uint8_t i = 0; (row_edges[i] != 0 || i==0) && i < 6; i += 2) {
//    // Pupil can't be smaller than 4 pixels across
//    if ((row_edges[i] - row_edges[i + 1]) < 4 && (row_edges[i] - row_edges[i + 1]) > -4)
//      continue;
//    
//    for (uint8_t j = 0; (col_edges[j] != 0 || j==0) && j < 6; j += 2) {
//      // Pupil can't be smaller than 4 pixels across
//      if ((col_edges[j] - col_edges[j + 1]) < 4 && (col_edges[j] - col_edges[j + 1]) > -4)
//        continue;
//      
//      float x_mid, y_mid;
//      x_mid = (row_edges[i] + row_edges[i + 1]) / (float)2;
//      y_mid = (col_edges[j] + col_edges[j + 1]) / (float)2;
//      
//      float r1, r2;
//      r1 = sqrt(((x_mid - row_edges[i]) * (x_mid - row_edges[i])) + ((y_mid - col_start) * (y_mid - col_start)));
//      r2 = sqrt(((x_mid - row_start) * (x_mid - row_start)) + ((y_mid - col_edges[j]) * (y_mid - col_edges[j])));
//      
//      float ratio = r1 / r2;
//      if ((ratio < 0.6) || (ratio > (1/0.6)) || fabs(ratio - 1) > fabs(best_ratio - 1))
//          continue;
//      
//      float r = (r1 + r2) / 2;
//      if (last_r != 0 && (r / last_r < 0.75 || r / last_r > 1/0.75))
//          continue;
//      
//      pupil_found = 1;
//      best_ratio = ratio; best_r = r;
//      best_center[0] = x_mid >= 0 ? (uint8_t)(x_mid+0.5) : (uint8_t)(x_mid-0.5);
//      best_center[1] = y_mid >= 0 ? (uint8_t)(y_mid+0.5) : (uint8_t)(y_mid-0.5);
//    }
//  }
//  
//  pred[0] = best_center[0];
//  pred[1] = best_center[1];
//  pred_radius = best_r;
//  last_r = best_r;
//
//  return pupil_found;
//}
//
//
//void find_pupil_edge(uint8_t start_point, uint8_t* edges, uint16_t* pixels)
//{
//  uint16_t med_buf[2], next_pixel;
//  uint8_t med_idx, small_val, reg_size, edge_idx;
//  uint8_t peak_after, local_regions[3], lr_idx, lr_min;
//  uint8_t peaks[53], peak_idx, spec_regions[53], spec_idx;
//  int16_t conv_sum, conv_abs, reg_sum, edge_mean, region_means[53];
//  int16_t edge_detect[106];
//  
//  uint8_t in_specular = 0, new_peak = 0; 
//  
//  // First do median filtering
//  med_buf[0] = pixels[0];
//  med_buf[1] = pixels[1];
//  med_idx = 0;
//  small_val = (pixels[0] < pixels[1]) ? 0 : 1;
//  
//  for (uint8_t i = 2; i < 112; i++) {
//    next_pixel = pixels[i];
//    
//    if (next_pixel < med_buf[small_val]) {
//      pixels[i - 1] = med_buf[small_val];
//      small_val = med_idx;
//    } else if (next_pixel > med_buf[!small_val]) {
//      pixels[i - 1] = med_buf[!small_val];
//      small_val = !med_idx;
//    } else {
//      pixels[i - 1] = next_pixel;
//    }
//    
//    med_buf[med_idx] = next_pixel;
//    med_idx = !med_idx;
//  }
//  
//  // Then percentile clamping
//  uint16_t perc_val = quick_percentile(pixels);
//  for (uint8_t i = 0; i < 112; i++) {
//    if (pixels[i] < perc_val)   pixels[i] = perc_val;
//  }
//  
//  // Next, do convolution
//  conv_sum = -pixels[0] - pixels[1] - pixels[2] + pixels[4] + pixels[5] + pixels[6];
//  conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
//  edge_detect[0] = conv_abs;
//  edge_mean = 0;
//  for (uint8_t i = 4; i < 108; i++) {
//    conv_sum += pixels[i - 4];
//    conv_sum -= pixels[i - 1];
//    conv_sum -= pixels[i];
//    conv_sum += pixels[i + 3];
//    
//    conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
//    
//    edge_detect[i - 3] = conv_abs;
//    edge_mean += conv_abs;
//  }
//  edge_mean /= 106;
//  
//  // Then peak identification (+ weeding out peaks resulting from specular reflection)
//  // and calculating region means (+ identifying specular regions)
//  peaks[0] = 0; reg_sum = pixels[1] + pixels[2] + pixels[3];
//  peak_idx = 1; in_specular = 0; new_peak = 0; peak_after = 0;
//  for (uint8_t i = 1; i < 105; i++) {
//    if (edge_detect[i] > SPEC_THRESH) {
//      if (in_specular == 0) {
//        in_specular = 1;
//        if (i > 1 && peaks[peak_idx - 1] != (i - 1)) {
//          peaks[peak_idx] = i - 1;
//          spec_regions[spec_idx] = peak_idx;
//          peak_idx++; new_peak = 1;
//        } else {
//          spec_regions[spec_idx] = peak_idx - 1;
//        }
//        spec_idx++;
//      } else if (in_specular == 2) {
//        in_specular++;
//      }
//    } else if (edge_detect[i] < SPEC_THRESH && in_specular != 0) {
//      if (in_specular == 1)
//        in_specular++;
//      else if (in_specular == 3) {
//        in_specular = 0;
//        peaks[peak_idx] = i;
//        peak_idx++; new_peak = 1;
//      }
//    } else {
//      if (edge_detect[i] >= edge_detect[i - 1] && edge_detect[i] > edge_detect[i + 1] && edge_detect[i] > edge_mean) {
//        peaks[peak_idx] = i;
//        peak_idx++; new_peak = 1;
//      }
//    }
//    
//    reg_sum += pixels[i + 3];
//    
//    if (new_peak == 1) {
//      reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2] + (peak_idx == 2 ? 3 : 0);
//      
//      // If we retroactively made the previous point a peak, need to adjust the mean calculation
//      if (in_specular == 1) {
//        region_means[peak_idx - 2] = (reg_sum - pixels[i + 3]) / reg_size;
//        reg_sum = pixels[i + 3];
//      } else {
//        region_means[peak_idx - 2] = reg_sum / reg_size;
//        reg_sum = 0;
//      }
//      
//      new_peak = 0;
//      if (peak_after == 0 && peaks[peak_idx - 1] > start_point)
//        peak_after = peak_idx - 1;
//    }
//  }
//  
//  // Set last peak as last pixel
//  peaks[peak_idx] = 111;
//  peak_idx++;
//  reg_sum += pixels[108] + pixels[109] + pixels[110] + pixels[111];
//  reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2];
//  region_means[peak_idx - 2] = reg_sum / reg_size;
//  
//  if (peak_after == 0)
//    peak_after = peak_idx - 1;
//  
//  // Identify the local regions around the start point
//  lr_idx = 0;
//  if (peak_after > 2) {
//    local_regions[lr_idx] = peak_after - 2;
//    lr_idx++;
//  }
//  
//  local_regions[lr_idx] = peak_after - 1;
//  lr_idx++;
//  
//  if (peak_after < peak_idx - 1) {
//    local_regions[lr_idx] = peak_after;
//    lr_idx++;
//  }
//  
//  // Select the local region with the lowest mean
//  lr_min = 0;
//  for (uint8_t i = 1; i < lr_idx; i++) {
//    if (region_means[local_regions[i]] < region_means[local_regions[lr_min]])
//      lr_min = i;
//  }
//  
//  edges[0] = peaks[local_regions[lr_min]];
//  edges[0] += (edges[0] == 0 ? 0 : CONV_OFFSET);
//  edges[1] = peaks[local_regions[lr_min]+1];
//  edges[1] += (edges[1] == 111 ? 0 : CONV_OFFSET);
//  edge_idx = 2;
//  
////  // Check if the region has a specular point on either end
////  for (uint8_t i = 0; i < spec_idx; i++) {
////    if (spec_regions[i] == local_regions[lr_min] - 1) {
////      edges[edge_idx] = peaks[local_regions[lr_min] - 1] + CONV_OFFSET;
////      edges[edge_idx+1] = peaks[local_regions[lr_min] + 1] + CONV_OFFSET;
////      edge_idx += 2;
////    } else if (spec_regions[i] == local_regions[lr_min] + 1) {
////      edges[edge_idx] = peaks[local_regions[lr_min]] + CONV_OFFSET;
////      edges[edge_idx+1] = peaks[local_regions[lr_min] + 2] + CONV_OFFSET;
////      edge_idx += 2;
////    }
////  }
//  
//  return;       // Edge data is stored in argument array
//}
//
//uint16_t quick_percentile(uint16_t *base_row)
//{
//  uint16_t row[112];
//  uint16_t r, w, mid;
//  
//  for (uint16_t i = 0; i < 112; i++) {
//    row[i] = base_row[i];
//  }
//  
//  uint8_t from = 0, to = 111;
//  
//  // Percentile value is hardcoded so it doesn't have to be computed at runtime
//  uint8_t k = (((112 * 100) * CIDER_PERCENTILE) / 10000);
//  
//  // if from == to we reached the kth element
//  while (from < to) {
//    r = from, w = to;
//    mid = row[(r + w) / 2];
//    
//    // stop if the reader and writer meets
//    while (r < w) {
//      
//      if (row[r] >= mid) { // put the large values at the end
//        uint16_t tmp = row[w];
//        row[w] = row[r];
//        row[r] = tmp;
//        w--;
//      } else { // the value is smaller than the pivot, skip
//        r++;
//      }
//    }
//    
//    // if we stepped up (r++) we need to step one down
//    if (row[r] > mid && r != 0)    
//      r--;
//    
//    // the r pointer is on the end of the first k elements
//    if (k <= r) {
//      to = r;
//    } else {
//      from = r + 1;
//    }
//  }
//  
//  return row[k];
//}
//
//int stony_image_ann()
//{
//  uint16_t lastRow, lastCol;
//  uint16_t pix_value;
//  
//  uint16_t pred_pixels[NUM_SUBSAMPLE];
//  
//  uint32_t pixel_sum = 0;
//  float M = 0.0, S = 0.0;
//  float value, tmpM;
//  int k = 1;
//  uint32_t current_subsample = 0;
//  
//  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
//  ADC_RegularChannelConfig(ADC1, CAM2_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
//  
//  set_pointer_value(REG_ROWSEL, 0, CAM2);
//  set_pointer_value(REG_COLSEL, MASK(0, 1), CAM2);
//  
//  lastRow = 0;
//  lastCol = MASK(0, 1);
//
//  for (int pixel = 0; pixel < NUM_SUBSAMPLE; pixel++) {   
//      if (MASK(pixel, 1) != lastCol)
//      {
//        // Set row to zero to avoid precharging any other rows
//        set_pointer_value(REG_ROWSEL, 0, CAM2);
//        
//        char diff = MASK(pixel, 1) - lastCol;
//        
//        inc_pointer_value(REG_COLSEL, diff, CAM2);
//        
//        lastCol = MASK(pixel, 1);
//
//        set_pointer_value(REG_ROWSEL, MASK(pixel, 0), CAM2);
//        
//        lastRow = MASK(pixel, 0);
//        
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      } else {
//        inc_value(MASK(pixel, 0) - lastRow, CAM2);
//        
//        lastRow = MASK(pixel, 0);
//      }
//      
//      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      
//      /* Start ADC1 Software Conversion */
//      ADC_SoftwareStartConv(ADC1);
//      
//      if (pixel > 0) {
//        pixel_sum += pix_value;
//        
//        // Standard deviation computation
//        value = (float)pix_value;
//        tmpM = M;
//        M += (value - tmpM) / k;
//        S += (value - tmpM) * (value - M);
//        k++;
//        current_subsample++;
//      }
//      else {
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//        asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
//      }
//
//      pix_value = adc_values[adc_idx] - COL_FPN(pixel);
//      adc_idx = !adc_idx;
//  }
//  
//  pixel_sum += pix_value;
//  
//  // Standard deviation computation
//  value = (float)pix_value;
//  tmpM = M;
//  M += (value - tmpM) / k;
//  S += (value - tmpM) * (value - M);
//  k++;
//  current_subsample++;
//  
//  if (current_subsample != NUM_SUBSAMPLE)
//    while(1);
//  
//  float mean = (float)pixel_sum / (NUM_SUBSAMPLE);
//  float std = sqrt(S / (k-1));
//  
//  // Predict gaze, store results in global variable pred[]
//  predict_gaze_mean(pred_pixels, mean, std);
//
//  return 0;
//}
//
