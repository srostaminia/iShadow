#include "libstony.h"
#include "stm32l1xx.h"
#include "main.h"
#include "diskio.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "hw_config.h"
#include "predict_gaze.h"
#include "math.h"

//#define TX_ROWS         48
//
//#define CAM2_OFFSET     TX_ROWS * 112
////#define CAM2_OFFSET     5376
//#define TX_BLOCKS       (TX_ROWS * 112 * 4) / 512
//
//#if (112 % TX_ROWS != 0)
//#define CAM2_MOD_OFFSET (112 % TX_ROWS) * 112
//#define TX_MOD_BLOCKS   ((112 % TX_ROWS) * 112 * 4 ) / 512
//
//#if ((112 % TX_ROWS) * 112 * 4 ) % 512 != 0
//#error TX_ROWS invalid, does not align to 512B boundary
//#endif

//#endif

//#define USB_PIXELS      92

extern int8_t pred[2];
extern float pred_radius;
float last_r = 0;
//uint16_t pred_img[112][112];

extern uint8_t param_packet[];

extern __IO  uint32_t Receive_length ;
extern uint32_t sd_ptr;
extern volatile uint8_t packet_sending;

extern unsigned short num_subsample;
extern unsigned short num_hidden;
extern unsigned int bh_offset;
extern unsigned int bo_offset;
extern unsigned int mask_offset;
extern unsigned int who_offset;
extern unsigned int wih_offset;
extern unsigned int fpn_offset;
extern unsigned int col_fpn_offset;

extern unsigned short model_data[];

int adc_idx = 0;
//uint16_t last_min = 0, last_max = 1000;
uint16_t min = 1000, max = 0;

#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
uint16_t last_avg = 150;
#endif

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
  col_fpn_offset = fpn_offset + 112 * 112;
  
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
  
  dac_init();
  
  // Get initial min / max pixel values from eye-facing camera
//  stony_image_minmax();
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

int stony_image_single()
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row
  // Double-buffered (2-dim array)
//  uint8_t buf8[2][112 * TX_ROWS * 2];
  uint8_t buf8[2][USB_PIXELS * 2];
  uint16_t *buf16 = (uint16_t *)buf8[0];
  
  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
//  uint16_t min = 65535, max = 0;
  
  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, CAM1_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(REG_ROWSEL, 0, CAM1);
  
  int data_cycle = 0;
  for (int row = 0; row < 112; row++) {
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
      
      // Do conversion
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      buf16[data_cycle] = adc_values[adc_idx];
      
      if (data_cycle == (USB_PIXELS - 1)) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(buf8[buf_idx], PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;
        buf16 = (uint16_t *)buf8[buf_idx];
      }
      
      data_cycle++;
      adc_idx = !adc_idx;

      CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
      CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
    } // for (col)
    
    inc_pointer_value(REG_ROWSEL, 1, CAM1);
  } // for (row)
  
  if (data_cycle != -1) {
    for (int i = data_cycle; i < USB_PIXELS; i++)
      buf16[i] = 0;
    
    while (packet_sending == 1);
    send_packet(buf8[buf_idx], PACKET_SIZE);
  }
  
//  last_max = max;
//  last_min = min;
  
  return 0;
}

int stony_image_dual_subsample()
{
  __IO uint32_t led1 = 0, led2 = 0;
  uint32_t DAC_Align = DAC_Align_12b_R;
  
  led1 = led2 = (uint32_t)DAC_BASE;
  led1 += DHR12R1_OFFSET + DAC_Align;
  led2 += DHR12R2_OFFSET + DAC_Align;
  
//  *(__IO uint32_t *) led1 = LED_LOW;
//  *(__IO uint32_t *) led2 = LED_LOW;
  
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, 2 cameras
  // Double-buffered (2-dim array)
  uint8_t buf8[2][USB_PIXELS * 2];
  
  uint32_t pixel_sum = 0;
  float M = 0.0, S = 0.0;
  float value, tmpM;
  int k = 1;
  
  uint16_t pred_img[112][112];
  uint8_t buf_idx = 0;
  
  uint16_t this_pixel = 0;
  uint32_t eye_pixels_collected = -1;
  uint32_t current_subsample = 0;

#ifdef SEND_8BIT
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < USB_PIXELS * 2; j++) {
      buf8[i][j] = 0;
    }
  }
#endif

#ifdef SEND_16BIT
  uint16_t *buf16 = (uint16_t *)buf8[0];
#else
  uint8_t *buf8_active = (uint8_t *)buf8[0];
#endif
  
#ifdef COLUMN_COLLECT
  set_pointer_value(REG_COLSEL, 0, CAM1);
  set_pointer_value(REG_COLSEL, 0, CAM2);
#else
  set_pointer_value(REG_ROWSEL, 0, CAM1);
  set_pointer_value(REG_ROWSEL, 0, CAM2);
#endif
  
//  max = 0;
//  min = 1000;

  int data_cycle = 0;
  
//  int data_cycle = 6; // Start at 2 b/c first two "pixels" transmitted are prediction values from previous cycle
//#ifdef SEND_16BIT  
////  buf16[0] = pred[0];
////  buf16[1] = pred[1];
//  buf8[0][0] = 2;
//  buf8[0][2] = pred[0];
//  buf8[0][4] = pred[1];
//  for (int i = 6; i < 11; i += 2) buf8[0][i] = 1;
//  for (int i = 1; i < 12; i += 2) buf8[0][i] = 0;
//#else
//  buf8_active[0] = 2;
//  buf8_active[1] = pred[0];
//  buf8_active[2] = pred[1];
//  for (int i = 3; i < 6; i++)   buf8_active[i] = 1;
//#endif
  
  uint16_t packets_sent = 0;  // For debug purposes only
//  uint16_t start = 0, total = 0;
  for (int row = 0; row < 112; row++) {    
#ifdef COLUMN_COLLECT
    set_pointer_value(REG_ROWSEL, 0, CAM1);
    set_pointer_value(REG_ROWSEL, 0, CAM2);
#else
    set_pointer_value(REG_COLSEL, 0, CAM1);
    set_pointer_value(REG_COLSEL, 0, CAM2);
#endif
    
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
        
#ifdef USE_FPN_EYE
        this_pixel = adc_values[1] - FPN_INORDER((row * 112) + (col - 1));
#else
        this_pixel = adc_values[1];
#endif

#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
      // These pixels don't work in outmode for some reason
      // Replace them with the last image's average value
#ifdef COLUMN_COLLECT
      if (col == 48 && row < 6)
        this_pixel = last_avg;
#else
      if (row == 48 && col < 6)
        this_pixel = last_avg;
#endif
#endif
        
        //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
//        *(__IO uint32_t *) led1 = LED_LOW;
//        *(__IO uint32_t *) led2 = LED_LOW;
        
        pred_img[row][col - 1] = this_pixel;
        eye_pixels_collected++;
//        min = (this_pixel < min) ? (this_pixel) : (min);
//        max = (this_pixel > max) ? (this_pixel) : (max);
        
        if (MASK(current_subsample, 0) == row && 
            MASK(current_subsample, 1) == col - 1)
        {
          pixel_sum += this_pixel;
          
          // Standard deviation computation
          value = (float)this_pixel;
          tmpM = M;
          M += (value - tmpM) / k;
          S += (value - tmpM) * (value - M);
          k++;
          current_subsample++;
        }

#ifdef SEND_EYE

#ifdef SEND_16BIT
//        buf16[data_cycle] = adc_values[1];
        buf16[data_cycle] = this_pixel;
#else
        buf8_active[data_cycle] = CONV_8BIT(this_pixel);
#endif
        
        if (data_cycle == (USB_PIXELS - 1)) {
          while (packet_sending == 1);
          
          data_cycle = -1;
          send_packet(buf8[buf_idx], PACKET_SIZE);
          packet_sending = 1;
          
          buf_idx = !buf_idx;
          
#ifdef SEND_16BIT
          buf16 = (uint16_t *)buf8[buf_idx];
#else
          buf8_active = (uint8_t *)buf8[buf_idx];
#endif
          
          packets_sent += 1;
        }
        data_cycle++;
#endif
        
      }
      
      // Do conversion for CAM1
      ADC_SoftwareStartConv(ADC1);
      
      if (col != 0) {
        CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
        CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
      }
      
      //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_HIGH);
//      *(__IO uint32_t *) led1 = LED_HIGH;
//      *(__IO uint32_t *) led2 = LED_HIGH;
      
      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
      // These pixels don't work in outmode for some reason
      // Replace them with the last image's average value
#ifdef COLUMN_COLLECT
      if (col == 48 && row < 6)
        this_pixel = last_avg;
#else
      if (row == 48 && col < 6)
        this_pixel = last_avg;
#endif
#endif
      
#if !defined(SEND_EYE)
      
#ifdef SEND_16BIT
      buf16[data_cycle] = adc_values[0];
#else
      buf8_active[data_cycle] = CONV_8BIT(adc_values[0]);
#endif
      
      if (data_cycle == (USB_PIXELS - 1)) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(buf8[buf_idx], PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;
        
#ifdef SEND_16BIT
        buf16 = (uint16_t *)buf8[buf_idx];
#else
        buf8_active = (uint8_t *)buf8[buf_idx];
#endif
        
        packets_sent += 1;
      }
    
      data_cycle++;
#endif  // !defined(SEND_EYE)
      
      // Do conversion for CAM2
      ADC_SoftwareStartConv(ADC1);

      CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
      CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
    } // for (col)
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
#ifdef USE_FPN_EYE
    // TODO: Get subsampled pixels...
    this_pixel = adc_values[1] - FPN_INORDER((row * 112) + 111);
#else
    this_pixel = adc_values[1];
#endif
    
    //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
//    *(__IO uint32_t *) led1 = LED_LOW;
//    *(__IO uint32_t *) led2 = LED_LOW;
    
    pred_img[row][111] = this_pixel;
    eye_pixels_collected++;
//    min = (pred_img[row][111] < min) ? (pred_img[row][111]) : (min);
//    max = (pred_img[row][111] > max) ? (pred_img[row][111]) : (max);
    
        if (MASK(current_subsample, 0) == row && 
            MASK(current_subsample, 1) == 111)
        {
          pixel_sum += this_pixel;
          
          // Standard deviation computation
          value = (float)this_pixel;
          tmpM = M;
          M += (value - tmpM) / k;
          S += (value - tmpM) * (value - M);
          k++;
          current_subsample++;
        }
    
#ifdef SEND_EYE

#ifdef SEND_16BIT
//    buf16[data_cycle] = adc_values[1];
    buf16[data_cycle] = pred_img[row][111];
#else
    buf8_active[data_cycle] = CONV_8BIT(pred_img[row][111]);
#endif
    
    if (data_cycle == (USB_PIXELS - 1)) {
      while (packet_sending == 1);
      
      data_cycle = -1;
      send_packet(buf8[buf_idx], PACKET_SIZE);
      packet_sending = 1;
      
      buf_idx = !buf_idx;
      
#ifdef SEND_16BIT
      buf16 = (uint16_t *)buf8[buf_idx];
#else
      buf8_active = (uint8_t *)buf8[buf_idx];
#endif
      
      packets_sent += 1;
    }
    data_cycle++;
#endif
    
#ifdef COLUMN_COLLECT
    inc_pointer_value(REG_COLSEL, 1, CAM1);
    inc_pointer_value(REG_COLSEL, 1, CAM2);
#else
    inc_pointer_value(REG_ROWSEL, 1, CAM1);
    inc_pointer_value(REG_ROWSEL, 1, CAM2);
#endif
  } // for (row)
  
  if (data_cycle != -1) {
    for (int i = data_cycle; i < USB_PIXELS; i++) {
#ifdef SEND_16BIT
      buf16[i] = 0;
#else
      buf8_active[i] = 0;
#endif
    }
    
    while (packet_sending == 1);
    send_packet(buf8[buf_idx], PACKET_SIZE);
    
    packet_sending = 1;
    while (packet_sending == 1);
    
    packets_sent += 1;
  }
  
//  float mean = (float)pixel_sum / (112 * 112);
  float mean = (float)pixel_sum / (NUM_SUBSAMPLE);
  float std = sqrt(S / (k-1));
  
#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
  last_avg = (uint16_t)mean;
#endif
  
  // Predict gaze, store results in global variable pred[]
//  predict_gaze_fullimg((uint16_t*)pred_img, min, max);
  predict_gaze_fullmean((uint16_t*)pred_img, mean, std);
  
  data_cycle = 6;
#ifdef SEND_16BIT  
//  buf16[0] = pred[0];
//  buf16[1] = pred[1];
  param_packet[0] = 3;
  param_packet[2] = pred[0];
  param_packet[4] = pred[1];
  for (int i = 6; i < 11; i += 2) param_packet[i] = 1;
  for (int i = 1; i < 12; i += 2) param_packet[i] = 0;
#else
  param_packet[0] = 3;
  param_packet[1] = pred[0];
  param_packet[2] = pred[1];
  for (int i = 3; i < 6; i++)   param_packet[i] = 1;
#endif

  send_packet(param_packet, PACKET_SIZE);

  while(packet_sending == 1);
  
//  clear_ENDP1_packet_buffers();
  send_empty_packet();
  while(packet_sending == 1);
  
//  packet_sending = 1;
  send_empty_packet();
  while(packet_sending == 1);
  
  return 0;
}

int stony_cider_line(uint8_t rowcol_num, uint16_t *line_buf, uint8_t rowcol_sel)
{
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
  // Double-buffered (2-dim array)
//  uint8_t buf8[2][112 * TX_ROWS * 2];  
  volatile uint16_t start, total;
  
  if (rowcol_sel == SEL_COL) {
    set_pointer_value(REG_COLSEL, rowcol_num, CAM2);
    set_pointer_value(REG_ROWSEL, 0, CAM2);
  } else {
    set_pointer_value(REG_ROWSEL, rowcol_num, CAM2);
    set_pointer_value(REG_COLSEL, 0, CAM2);
  }
  
  delay_us(1);
  
  for (int i = 0; i < 112; i++) {      
    CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
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
    
    if (rowcol_sel == SEL_COL)
      line_buf[i] = adc_values[adc_idx] - COL_FPN((i * 112) + rowcol_num);
    else
      line_buf[i] = adc_values[adc_idx] - ROW_FPN((rowcol_num * 112) + i);
    
    adc_idx = !adc_idx;
    
    CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
    CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
  } // for (col)
  
  return 0;
}

int run_cider(uint8_t *cider_xy)
{
  uint8_t row_edges[6] = {0, 0, 0, 0, 0, 0}, col_edges[6] = {0, 0, 0, 0, 0, 0};
  int pupil_found = -1;
  
  cider_xy[1] = (uint8_t)((pred[1] < 0 ? 0 : pred[1]) > 111 ? 111 : pred[1]);
  cider_xy[0] = (uint8_t)((pred[0] < 0 ? 0 : pred[0]) > 111 ? 111 : pred[0]);
  
  uint8_t col_start = cider_xy[1], row_start = cider_xy[0];
  
  float best_ratio = 0, best_r = 0;
  uint8_t best_center[2] = {0, 0};
  
  uint16_t row[112], col[112];
  stony_cider_line(col_start, row, SEL_ROW);
  stony_cider_line(row_start, col, SEL_COL);
  
  find_pupil_edge(row_start, row_edges, row);
  find_pupil_edge(col_start, col_edges, col);

  for (uint8_t i = 0; (row_edges[i] != 0 || i==0) && i < 6; i += 2) {
    // Pupil can't be smaller than 4 pixels across
    if ((row_edges[i] - row_edges[i + 1]) < 4 && (row_edges[i] - row_edges[i + 1]) > -4)
      continue;
    
    for (uint8_t j = 0; (col_edges[j] != 0 || j==0) && j < 6; j += 2) {
      // Pupil can't be smaller than 4 pixels across
      if ((col_edges[j] - col_edges[j + 1]) < 4 && (col_edges[j] - col_edges[j + 1]) > -4)
        continue;
      
      float x_mid, y_mid;
      x_mid = (row_edges[i] + row_edges[i + 1]) / (float)2;
      y_mid = (col_edges[j] + col_edges[j + 1]) / (float)2;
      
      float r1, r2;
      r1 = sqrt(((x_mid - row_edges[i]) * (x_mid - row_edges[i])) + ((y_mid - col_start) * (y_mid - col_start)));
      r2 = sqrt(((x_mid - row_start) * (x_mid - row_start)) + ((y_mid - col_edges[j]) * (y_mid - col_edges[j])));
      
      float ratio = r1 / r2;
      if ((ratio < 0.6) || (ratio > (1/0.6)) || fabs(ratio - 1) > fabs(best_ratio - 1))
          continue;
      
      float r = (r1 + r2) / 2;
      if (last_r != 0 && (r / last_r < 0.75 || r / last_r > 1/0.75))
          continue;
      
      pupil_found = 1;
      best_ratio = ratio; best_r = r;
      best_center[0] = x_mid >= 0 ? (uint8_t)(x_mid+0.5) : (uint8_t)(x_mid-0.5);
      best_center[1] = y_mid >= 0 ? (uint8_t)(y_mid+0.5) : (uint8_t)(y_mid-0.5);
    }
  }
  
  pred[0] = best_center[0];
  pred[1] = best_center[1];
  pred_radius = best_r;
  last_r = best_r;
  
  return pupil_found;
}

void find_pupil_edge(uint8_t start_point, uint8_t* edges, uint16_t* pixels)
{
  uint16_t med_buf[2], next_pixel;
  uint8_t med_idx, small_val, reg_size, edge_idx;
  uint8_t peak_after, local_regions[3], lr_idx, lr_min;
  uint8_t peaks[53], peak_idx, spec_regions[53], spec_idx;
  int16_t conv_sum, conv_abs, reg_sum, edge_mean, region_means[53];
  int16_t edge_detect[106];
  
  uint8_t in_specular = 0, new_peak = 0; 
  
  // First do median filtering
  med_buf[0] = pixels[0];
  med_buf[1] = pixels[1];
  med_idx = 0;
  small_val = (pixels[0] < pixels[1]) ? 0 : 1;
  
  for (uint8_t i = 2; i < 112; i++) {
    next_pixel = pixels[i];
    
    if (next_pixel < med_buf[small_val]) {
      pixels[i - 1] = med_buf[small_val];
      small_val = med_idx;
    } else if (next_pixel > med_buf[!small_val]) {
      pixels[i - 1] = med_buf[!small_val];
      small_val = !med_idx;
    } else {
      pixels[i - 1] = next_pixel;
    }
    
    med_buf[med_idx] = next_pixel;
    med_idx = !med_idx;
  }
  
  // Next, do convolution
  conv_sum = -pixels[0] - pixels[1] - pixels[2] + pixels[4] + pixels[5] + pixels[6];
  conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
  edge_detect[0] = conv_abs;
  edge_mean = 0;
  for (uint8_t i = 4; i < 108; i++) {
    conv_sum += pixels[i - 4];
    conv_sum -= pixels[i - 1];
    conv_sum -= pixels[i];
    conv_sum += pixels[i + 3];
    
    conv_abs = (conv_sum < 0) ? (-conv_sum) : (conv_sum);
    
    edge_detect[i - 3] = conv_abs;
    edge_mean += conv_abs;
  }
  edge_mean /= 106;
  
  // Then peak identification (+ weeding out peaks resulting from specular reflection)
  // and calculating region means (+ identifying specular regions)
  peaks[0] = 0; reg_sum = pixels[1] + pixels[2] + pixels[3];
  peak_idx = 1; in_specular = 0; new_peak = 0; peak_after = 0;
  for (uint8_t i = 1; i < 105; i++) {
    if (edge_detect[i] > SPEC_THRESH) {
      if (in_specular == 0) {
        in_specular = 1;
        if (i > 1 && peaks[peak_idx - 1] != (i - 1)) {
          peaks[peak_idx] = i - 1;
          spec_regions[spec_idx] = peak_idx;
          peak_idx++; new_peak = 1;
        } else {
          spec_regions[spec_idx] = peak_idx - 1;
        }
        spec_idx++;
      } else if (in_specular == 2) {
        in_specular++;
      }
    } else if (edge_detect[i] < SPEC_THRESH && in_specular != 0) {
      if (in_specular == 1)
        in_specular++;
      else if (in_specular == 3) {
        in_specular = 0;
        peaks[peak_idx] = i;
        peak_idx++; new_peak = 1;
      }
    } else {
      if (edge_detect[i] >= edge_detect[i - 1] && edge_detect[i] > edge_detect[i + 1] && edge_detect[i] > edge_mean) {
        peaks[peak_idx] = i;
        peak_idx++; new_peak = 1;
      }
    }
    
    reg_sum += pixels[i + 3];
    
    if (new_peak == 1) {
      reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2] + (peak_idx == 2 ? 3 : 0);
      
      // If we retroactively made the previous point a peak, need to adjust the mean calculation
      if (in_specular == 1) {
        region_means[peak_idx - 2] = (reg_sum - pixels[i + 3]) / reg_size;
        reg_sum = pixels[i + 3];
      } else {
        region_means[peak_idx - 2] = reg_sum / reg_size;
        reg_sum = 0;
      }
      
      new_peak = 0;
      if (peak_after == 0 && peaks[peak_idx - 1] > start_point)
        peak_after = peak_idx - 1;
    }
  }
  
  // Set last peak as last pixel
  peaks[peak_idx] = 111;
  peak_idx++;
  reg_sum += pixels[108] + pixels[109] + pixels[110] + pixels[111];
  reg_size = peaks[peak_idx - 1] - peaks[peak_idx - 2];
  region_means[peak_idx - 2] = reg_sum / reg_size;
  
  if (peak_after == 0)
    peak_after = peak_idx - 1;
  
  // Identify the local regions around the start point
  lr_idx = 0;
  if (peak_after > 2) {
    local_regions[lr_idx] = peak_after - 2;
    lr_idx++;
  }
  
  local_regions[lr_idx] = peak_after - 1;
  lr_idx++;
  
  if (peak_after < peak_idx - 1) {
    local_regions[lr_idx] = peak_after;
    lr_idx++;
  }
  
  // Select the local region with the lowest mean
  lr_min = 0;
  for (uint8_t i = 1; i < lr_idx; i++) {
    if (region_means[local_regions[i]] < region_means[local_regions[lr_min]])
      lr_min = i;
  }
  
  edges[0] = peaks[local_regions[lr_min]];
  edges[0] += (edges[0] == 0 ? 0 : CONV_OFFSET);
  edges[1] = peaks[local_regions[lr_min]+1];
  edges[1] += (edges[1] == 111 ? 0 : CONV_OFFSET);
  edge_idx = 2;
  
//  // Check if the region has a specular point on either end
//  for (uint8_t i = 0; i < spec_idx; i++) {
//    if (spec_regions[i] == local_regions[lr_min] - 1) {
//      edges[edge_idx] = peaks[local_regions[lr_min] - 1] + CONV_OFFSET;
//      edges[edge_idx+1] = peaks[local_regions[lr_min] + 1] + CONV_OFFSET;
//      edge_idx += 2;
//    } else if (spec_regions[i] == local_regions[lr_min] + 1) {
//      edges[edge_idx] = peaks[local_regions[lr_min]] + CONV_OFFSET;
//      edges[edge_idx+1] = peaks[local_regions[lr_min] + 2] + CONV_OFFSET;
//      edge_idx += 2;
//    }
//  }
  
  return;       // Edge data is stored in argument array
}

int stony_send_cider_image(uint8_t *cider_rowcol, uint8_t cider_failed)
{
  __IO uint32_t led1 = 0, led2 = 0;
  uint32_t DAC_Align = DAC_Align_12b_R;
  
  led1 = led2 = (uint32_t)DAC_BASE;
  led1 += DHR12R1_OFFSET + DAC_Align;
  led2 += DHR12R2_OFFSET + DAC_Align;
  
//  *(__IO uint32_t *) led1 = LED_LOW;
//  *(__IO uint32_t *) led2 = LED_LOW;
  
  // 112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, 2 cameras
  // Double-buffered (2-dim array)
  uint8_t buf8[2][USB_PIXELS * 2];
  
  uint32_t pixel_sum = 0;
  float M = 0.0, S = 0.0;
  float value, tmpM;
  int k = 1;
  
  uint16_t pred_img[112][112];
  uint8_t buf_idx = 0;
  
  uint16_t this_pixel = 0;
  uint32_t eye_pixels_collected = -1;
  uint32_t current_subsample = 0;

#ifdef SEND_8BIT
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < USB_PIXELS * 2; j++) {
      buf8[i][j] = 0;
    }
  }
#endif

#ifdef SEND_16BIT
  uint16_t *buf16 = (uint16_t *)buf8[0];
#else
  uint8_t *buf8_active = (uint8_t *)buf8[0];
#endif
  
#ifdef COLUMN_COLLECT
  set_pointer_value(REG_COLSEL, 0, CAM1);
  set_pointer_value(REG_COLSEL, 0, CAM2);
#else
  set_pointer_value(REG_ROWSEL, 0, CAM1);
  set_pointer_value(REG_ROWSEL, 0, CAM2);
#endif
  
//  max = 0;
//  min = 1000;

  int data_cycle = 0;
  
  uint16_t packets_sent = 0;  // For debug purposes only
//  uint16_t start = 0, total = 0;
  for (int row = 0; row < 112; row++) {    
#ifdef COLUMN_COLLECT
    set_pointer_value(REG_ROWSEL, 0, CAM1);
    set_pointer_value(REG_ROWSEL, 0, CAM2);
#else
    set_pointer_value(REG_COLSEL, 0, CAM1);
    set_pointer_value(REG_COLSEL, 0, CAM2);
#endif
    
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
        
#ifdef USE_FPN_EYE
        this_pixel = adc_values[1] - FPN_INORDER((row * 112) + (col - 1));
#else
        this_pixel = adc_values[1];
#endif

#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
      // These pixels don't work in outmode for some reason
      // Replace them with the last image's average value
#ifdef COLUMN_COLLECT
      if (col == 48 && row < 6)
        this_pixel = last_avg;
#else
      if (row == 48 && col < 6)
        this_pixel = last_avg;
#endif
#endif
        
        //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
//        *(__IO uint32_t *) led1 = LED_LOW;
//        *(__IO uint32_t *) led2 = LED_LOW;
        
        pred_img[row][col - 1] = this_pixel;
        eye_pixels_collected++;
//        min = (this_pixel < min) ? (this_pixel) : (min);
//        max = (this_pixel > max) ? (this_pixel) : (max);
        
        if (MASK(current_subsample, 0) == row && 
            MASK(current_subsample, 1) == col - 1)
        {
          pixel_sum += this_pixel;
          
          // Standard deviation computation
          value = (float)this_pixel;
          tmpM = M;
          M += (value - tmpM) / k;
          S += (value - tmpM) * (value - M);
          k++;
          current_subsample++;
        }

#ifdef SEND_EYE

#ifdef SEND_16BIT
//        buf16[data_cycle] = adc_values[1];
        buf16[data_cycle] = this_pixel;
#else
        buf8_active[data_cycle] = CONV_8BIT(this_pixel);
#endif
        
        if (data_cycle == (USB_PIXELS - 1)) {
          while (packet_sending == 1);
          
          data_cycle = -1;
          send_packet(buf8[buf_idx], PACKET_SIZE);
          packet_sending = 1;
          
          buf_idx = !buf_idx;
          
#ifdef SEND_16BIT
          buf16 = (uint16_t *)buf8[buf_idx];
#else
          buf8_active = (uint8_t *)buf8[buf_idx];
#endif
          
          packets_sent += 1;
        }
        data_cycle++;
#endif
        
      }
      
      // Do conversion for CAM1
      ADC_SoftwareStartConv(ADC1);
      
      if (col != 0) {
        CAM2_INCV_BANK->ODR |= CAM2_INCV_PIN;
        CAM2_INCV_BANK->ODR &= ~CAM2_INCV_PIN;
      }
      
      //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_HIGH);
//      *(__IO uint32_t *) led1 = LED_HIGH;
//      *(__IO uint32_t *) led2 = LED_HIGH;
      
      CAM2_INPH_BANK->ODR |= CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      CAM2_INPH_BANK->ODR &= ~CAM2_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
      // These pixels don't work in outmode for some reason
      // Replace them with the last image's average value
#ifdef COLUMN_COLLECT
      if (col == 48 && row < 6)
        this_pixel = last_avg;
#else
      if (row == 48 && col < 6)
        this_pixel = last_avg;
#endif
#endif
      
#if !defined(SEND_EYE)
      
#ifdef SEND_16BIT
      buf16[data_cycle] = adc_values[0];
#else
      buf8_active[data_cycle] = CONV_8BIT(adc_values[0]);
#endif
      
      if (data_cycle == (USB_PIXELS - 1)) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(buf8[buf_idx], PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;
        
#ifdef SEND_16BIT
        buf16 = (uint16_t *)buf8[buf_idx];
#else
        buf8_active = (uint8_t *)buf8[buf_idx];
#endif
        
        packets_sent += 1;
      }
    
      data_cycle++;
#endif  // !defined(SEND_EYE)
      
      // Do conversion for CAM2
      ADC_SoftwareStartConv(ADC1);

      CAM1_INCV_BANK->ODR |= CAM1_INCV_PIN;
      CAM1_INCV_BANK->ODR &= ~CAM1_INCV_PIN;
    } // for (col)
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
#ifdef USE_FPN_EYE
    // TODO: Get subsampled pixels...
    this_pixel = adc_values[1] - FPN_INORDER((row * 112) + 111);
#else
    this_pixel = adc_values[1];
#endif
    
    //      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
//    *(__IO uint32_t *) led1 = LED_LOW;
//    *(__IO uint32_t *) led2 = LED_LOW;
    
    pred_img[row][111] = this_pixel;
    eye_pixels_collected++;
//    min = (pred_img[row][111] < min) ? (pred_img[row][111]) : (min);
//    max = (pred_img[row][111] > max) ? (pred_img[row][111]) : (max);
    
        if (MASK(current_subsample, 0) == row && 
            MASK(current_subsample, 1) == 111)
        {
          pixel_sum += this_pixel;
          
          // Standard deviation computation
          value = (float)this_pixel;
          tmpM = M;
          M += (value - tmpM) / k;
          S += (value - tmpM) * (value - M);
          k++;
          current_subsample++;
        }
    
#ifdef SEND_EYE
        
#ifdef SEND_16BIT
//    buf16[data_cycle] = adc_values[1];
    buf16[data_cycle] = pred_img[row][111];
#else
    buf8_active[data_cycle] = CONV_8BIT(pred_img[row][111]);
#endif
    
    if (data_cycle == (USB_PIXELS - 1)) {
      while (packet_sending == 1);
      
      data_cycle = -1;
      send_packet(buf8[buf_idx], PACKET_SIZE);
      packet_sending = 1;
      
      buf_idx = !buf_idx;
      
#ifdef SEND_16BIT
      buf16 = (uint16_t *)buf8[buf_idx];
#else
      buf8_active = (uint8_t *)buf8[buf_idx];
#endif
      
      packets_sent += 1;
    }
    data_cycle++;
#endif
    
#ifdef COLUMN_COLLECT
    inc_pointer_value(REG_COLSEL, 1, CAM1);
    inc_pointer_value(REG_COLSEL, 1, CAM2);
#else
    inc_pointer_value(REG_ROWSEL, 1, CAM1);
    inc_pointer_value(REG_ROWSEL, 1, CAM2);
#endif
  } // for (row)
  
  if (data_cycle != -1) {
    for (int i = data_cycle; i < USB_PIXELS; i++) {
#ifdef SEND_16BIT
      buf16[i] = 0;
#else
      buf8_active[i] = 0;
#endif
    }
    
    while (packet_sending == 1);
    send_packet(buf8[buf_idx], PACKET_SIZE);
    
    packet_sending = 1;
    while (packet_sending == 1);
    
    packets_sent += 1;
  }
  
//  float mean = (float)pixel_sum / (112 * 112);
#ifdef OUTMODE
  float mean = (float)pixel_sum / (NUM_SUBSAMPLE);
#endif
  
  float std = sqrt(S / (k-1));
  
#if defined(OUTMODE) && !defined(COLUMN_COLLECT)
  last_avg = (uint16_t)mean;
#endif
  
  data_cycle = 6;
#ifdef SEND_16BIT  
//  buf16[0] = pred[0];
//  buf16[1] = pred[1];
  param_packet[0] = cider_failed + 1;
  param_packet[2] = pred[0];
  param_packet[4] = pred[1];
  param_packet[6] = cider_rowcol[1];
  param_packet[8] = cider_rowcol[0];
  param_packet[10] = (uint8_t)pred_radius;
  for (int i = 1; i < 12; i += 2) param_packet[i] = 0;
#else
  param_packet[0] = cider_failed + 1;
  param_packet[1] = pred[0];
  param_packet[2] = pred[1];
  param_packet[3] = cider_rowcol[0];
  param_packet[4] = cider_rowcol[1];
  param_packet[5] = (uint8_t)pred_radius;
#endif

  send_packet(param_packet, PACKET_SIZE);

  while(packet_sending == 1);
  
//  clear_ENDP1_packet_buffers();
  send_empty_packet();
  while(packet_sending == 1);
  
//  packet_sending = 1;
  send_empty_packet();
  while(packet_sending == 1);
  
  return 0;
}
