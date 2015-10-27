#include "stonyman.h"
#include "stm32l1xx.h"
#include "math.h"
#include "predict_gaze.h"

// TODO: FIXME! This is just to make the stupid delay_us warnings go away...
#include "main.h"

#ifdef SD_SEND
#include "diskio.h"

extern uint32_t sd_ptr;
#endif

#ifdef USB_SEND
#include "hw_config.h"

extern volatile uint8_t packet_sending;
extern uint8_t param_packet[PACKET_SIZE];
#endif

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
  if (cam == OUT_CAM) {
    OCAM_RESV_BANK->ODR |= OCAM_RESV_PIN;
    OCAM_RESV_BANK->ODR &= ~OCAM_RESV_PIN;
  }
  else {
    ECAM_RESV_BANK->ODR |= ECAM_RESV_PIN;
    ECAM_RESV_BANK->ODR &= ~ECAM_RESV_PIN;
  }
}

inline static void pulse_incv(uint8_t cam)
{
  if (cam == OUT_CAM) {
    OCAM_INCV_BANK->ODR |= OCAM_INCV_PIN;
    OCAM_INCV_BANK->ODR &= ~OCAM_INCV_PIN;
  }
  else {
    ECAM_INCV_BANK->ODR |= ECAM_INCV_PIN;
    ECAM_INCV_BANK->ODR &= ~ECAM_INCV_PIN;
  }
}

inline static void pulse_resp(uint8_t cam)
{
  if (cam == OUT_CAM) {
    OCAM_RESP_BANK->ODR |= OCAM_RESP_PIN;
    OCAM_RESP_BANK->ODR &= ~OCAM_RESP_PIN;
  }
  else {
    ECAM_RESP_BANK->ODR |= ECAM_RESP_PIN;
    ECAM_RESP_BANK->ODR &= ~ECAM_RESP_PIN;
  }
}

inline static void pulse_incp(uint8_t cam)
{
  if (cam == OUT_CAM) {
    OCAM_INCP_BANK->ODR |= OCAM_INCP_PIN;
    OCAM_INCP_BANK->ODR &= ~OCAM_INCP_PIN;
  }
  else {
    ECAM_INCP_BANK->ODR |= ECAM_INCP_PIN;
    ECAM_INCP_BANK->ODR &= ~ECAM_INCP_PIN;
  }
}

inline static void pulse_inph(unsigned short time, uint8_t cam)
{
  if (cam == OUT_CAM) {
    OCAM_INPH_BANK->ODR |= OCAM_INPH_PIN;
    delay_us(time);
    OCAM_INPH_BANK->ODR &= ~OCAM_INPH_PIN;
  }
  else {
    ECAM_INPH_BANK->ODR |= ECAM_INPH_PIN;
    delay_us(time);
    ECAM_INPH_BANK->ODR &= ~ECAM_INPH_PIN;
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

// inc_pointer_value
// Sets the pointer to a register and increments by the given amount
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

// inc_value
// Increments the current register the given number of times
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

// ----------------Initialization functions------------------------------------

// stony_init_default()
// Master init function - sets up MCU pins and configures Stonyman
void stony_init_default()
{
	stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3, 
		SMH_GAIN_3V3, SMH_SELAMP_3V3);
}


void stony_init(short vref, short nbias, short aobias, char gain, char selamp)
{
  short config;
  char flagUseAmplifier;

#ifdef LOAD_MODEL
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
  
  // Set MCU pins
  stony_pin_config();

  // Initialize ADC + DMA
  adc_dma_init();
  
  // Set all pins low
  set_pin(OCAM_RESV_BANK, OCAM_RESV_PIN, 0);
  set_pin(OCAM_INCV_BANK, OCAM_INCV_PIN, 0);
  set_pin(OCAM_RESP_BANK, OCAM_RESP_PIN, 0);
  set_pin(OCAM_INCP_BANK, OCAM_INCP_PIN, 0);
  set_pin(OCAM_INPH_BANK, OCAM_INPH_PIN, 0);
  set_pin(ECAM_RESV_BANK, ECAM_RESV_PIN, 0);
  set_pin(ECAM_INCV_BANK, ECAM_INCV_PIN, 0);
  set_pin(ECAM_RESP_BANK, ECAM_RESP_PIN, 0);
  set_pin(ECAM_INCP_BANK, ECAM_INCP_PIN, 0);
  set_pin(ECAM_INPH_BANK, ECAM_INPH_PIN, 0);

  //clear all chip register values
  clear_values(OUT_CAM);
  clear_values(EYE_CAM);

  //set up biases
  // TODO russ: haven't looked at what this function does
  set_biases(vref,nbias,aobias,OUT_CAM);
  set_biases(vref,nbias,aobias,EYE_CAM);

  // sanitize this input before use
  flagUseAmplifier=selamp ? 1:0;

  config = gain + (flagUseAmplifier * 8) + 16;

  //turn chip on with config value
  set_pointer_value(REG_CONFIG,config,OUT_CAM);
  set_pointer_value(REG_CONFIG,config,EYE_CAM);
  
  dac_init();
}

static void stony_pin_config()
{  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(CAM_AHB, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  
  GPIO_InitStructure.GPIO_Pin = OCAM_RESV_PIN;
  GPIO_Init(OCAM_RESV_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = OCAM_RESP_PIN;
  GPIO_Init(OCAM_RESP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = OCAM_INCV_PIN;
  GPIO_Init(OCAM_INCV_BANK, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = OCAM_INCP_PIN;
  GPIO_Init(OCAM_INCP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = OCAM_INPH_PIN;
  GPIO_Init(OCAM_INPH_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ECAM_RESV_PIN;
  GPIO_Init(ECAM_RESV_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ECAM_RESP_PIN;
  GPIO_Init(ECAM_RESP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ECAM_INCV_PIN;
  GPIO_Init(ECAM_INCV_BANK, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = ECAM_INCP_PIN;
  GPIO_Init(ECAM_INCP_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ECAM_INPH_PIN;
  GPIO_Init(ECAM_INPH_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  OCAM_AN_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(OCAM_AN_BANK, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ECAM_AN_PIN;
  GPIO_Init(ECAM_AN_BANK, &GPIO_InitStructure);
}

static void adc_dma_init() {
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  // Enable HSI oscillator (required for ADC operation)
  RCC_HSICmd(ENABLE);

  /* Check that HSI oscillator is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
  
  /*------------------------ DMA1 configuration ------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* DMA1 channel1 configuration */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_values;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
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
  ADC_Init(ADC1, &ADC_InitStructure);
  
  ADC_RegularChannelConfig(ADC1, OCAM_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, ECAM_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
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

static void dac_init() {
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

// ----------------Driver / image capture functions----------------------------

// stony_single()
// Capture an image from one camera (selected by PRIMARY_CAM in stonyman.h)
int stony_single()
{
  // TODO: LED code

  // Double-buffered (2-dim array), two bytes per pixel
  uint8_t buf8[2][TX_PIXELS * 2];

#ifdef DO_8BIT_CONV
    uint8_t *buf8_active = (uint8_t *)buf8[0];

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < USB_PIXELS * 2; j++) {
        buf8[i][j] = 0;
      }
    }
#else
    uint16_t *buf16 = (uint16_t *)buf8[0];
#endif

#ifdef COLUMN_COLLECT
  uint8_t outer_reg = REG_COLSEL, inner_reg = REG_ROWSEL;
#else
  uint8_t outer_reg = REG_ROWSEL, inner_reg = REG_COLSEL;
#endif
  
  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(outer_reg, 0, PRIMARY_CAM);

  int data_cycle = 0;
  for (int i_outer = 0; i_outer < 112; i_outer++) {
    set_pointer_value(inner_reg, 0, PRIMARY_CAM);

    delay_us(1);
    
    for (int j_inner = 0; j_inner < 112; j_inner++, data_cycle++) {      
      SINGLE_PARAM(INPH_BANK)->ODR |= SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      SINGLE_PARAM(INPH_BANK)->ODR &= ~SINGLE_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do ADC conversion
      ADC_SoftwareStartConv(ADC1);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
#ifdef DO_8BIT_CONV
      buf8_active[data_cycle] = CONV_8BIT(adc_values[adc_idx]);
#else
      buf16[data_cycle] = adc_values[adc_idx];
#endif

#ifdef USB_SEND
      if (data_cycle == USB_PIXELS - 1) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(buf8[buf_idx], PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;

#ifdef DO_8BIT_CONV
        buf8_active = (uint8_t *)buf8[buf_idx];
#else
        buf16 = (uint16_t *)buf8[buf_idx];
#endif
      }
#endif // USB_SEND
      
      adc_idx = !adc_idx;

      SINGLE_PARAM(INCV_BANK)->ODR |= SINGLE_PARAM(INCV_PIN);
      SINGLE_PARAM(INCV_BANK)->ODR &= ~SINGLE_PARAM(INCV_PIN);
    } // for (j_inner)
    
    inc_pointer_value(outer_reg, 1, PRIMARY_CAM);

#ifdef SD_SEND
    if (data_cycle / 112 == SD_ROWS) {
      if (i_outer > SD_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, SD_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;

#ifdef DO_8BIT_CONV
      buf8_active = (uint8_t *)buf8[buf_idx];
#else
      buf16 = (uint16_t *)buf8[buf_idx];
#endif
      
      sd_ptr += SD_BLOCKS / 2;
      data_cycle = 0;
    }
#endif // SD_SEND
  } // for (i_outer)

#ifdef USB_SEND

  if (data_cycle != -1) {

#ifdef USB_16BIT
    for (int i = data_cycle; i < USB_PIXELS; i++)
      buf16[i] = 0;
#endif
    
    while (packet_sending == 1);
    send_packet(buf8[buf_idx], PACKET_SIZE);
  }

  param_packet[0] = 0;

#ifdef USB_16BIT
  for (int i = 1; i < 12; i++) param_packet[i] = 1;
#else
  for (int i = 1; i < 6; i++)   param_packet[i] = 1;
#endif

  send_packet(param_packet, PACKET_SIZE);
  while(packet_sending == 1);
  
  send_empty_packet();
  while(packet_sending == 1);
  
  send_empty_packet();
  while(packet_sending == 1);

#endif // USB_SEND

#ifdef SD_SEND

#if (112 % SD_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)buf8[buf_idx], sd_ptr, SD_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += SD_MOD_BLOCKS / 2;
#endif // (112 % SD_ROWS != 0)
  
  f_finish_write();
#endif // SD_SEND
  
  return 0;
}