#include "stonyman.h"
#include "stm32l1xx.h"
#include "math.h"
#include "stdlib.h"

// TODO: FIXME! This is just to make the stupid delay_us warnings go away...
#include "main.h"

#ifdef EYE_TRACKING_ON
#include "eye_models.h"

extern int8_t pred[2];
extern unsigned short num_subsample;
extern unsigned int mask_offset;
#endif

#ifdef SD_SEND
#include "diskio.h"

extern uint32_t sd_ptr;
static uint8_t frame_data[512];
#endif // SD_SEND

#ifdef USB_SEND
#include "hw_config.h"

extern volatile uint8_t packet_sending;
static uint8_t frame_data[USB_PACKET_SIZE];
#endif // USB_SEND

#ifdef USE_PARAM_FILE
// This is declared extern because it is referencing an external binary file
// that must be provided in the project settings (see readme)
extern uint16_t model_data[];
#endif

extern uint32_t time_elapsed;

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

// Commented out because I am *sick* of getting the unused function warning
// Uncommend if we bring it back into the code at any point
// inline static void pulse_inph(unsigned short time, uint8_t cam) 
// {
//   if (cam == OUT_CAM) {
//     OCAM_INPH_BANK->ODR |= OCAM_INPH_PIN;
//     delay_us(time);
//     OCAM_INPH_BANK->ODR &= ~OCAM_INPH_PIN;
//   }
//   else {
//     ECAM_INPH_BANK->ODR |= ECAM_INPH_PIN;
//     delay_us(time);
//     ECAM_INPH_BANK->ODR &= ~ECAM_INPH_PIN;
//   }
// }

// clear_values
// Resets the value of all registers to zero
inline static void clear_values(uint8_t cam)
{
  short i;

  for (i = 0; i < 8; ++i)
  {
    set_pointer_value(i,0,cam);  //set each register to zero
  }
}

// set_pointer_value
// Sets the pointer to a register and sets the value of that register
inline static void set_pointer_value(char ptr, short val, uint8_t cam)
{
  set_pointer(ptr, cam);  //set pointer to register
  set_value(val, cam);  //set value of that register
}

// inc_pointer_value
// Sets the pointer to a register and increments by the given amount
inline static void inc_pointer_value(char ptr, short val, uint8_t cam)
{
  set_pointer(ptr, cam);  //set pointer to register
  inc_value(val, cam);  //set value of that register
}

// set_pointer
// Sets the pointer system register to the desired value
inline static void set_pointer(char ptr, uint8_t cam)
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
inline static void set_value(short val, uint8_t cam)
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
inline static void inc_value(short val, uint8_t cam)
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
inline static void set_biases(short vref,short nbias,short aobias, uint8_t cam)
{
  set_pointer_value(REG_VREF,vref,cam);
  set_pointer_value(REG_NBIAS,nbias,cam);
  set_pointer_value(REG_AOBIAS,aobias,cam);
}

// ----------------Data TX helper functions------------------------------------
#ifdef USB_SEND
inline static void usb_finish_tx(uint8_t *frame_data, uint8_t packet_length)
{
  for (int i = packet_length + 1; i < FRAME_DATA_LENGTH; i++)  
    CAST_PIXEL_BUFFER(frame_data)[i] = 1;

  while(packet_sending == 1);
  send_packet(frame_data, USB_PACKET_SIZE);
  while(packet_sending == 1);
  
  send_empty_packet();
  while(packet_sending == 1);
  
  send_empty_packet();
  while(packet_sending == 1);
}
#endif // USB_SEND

// ----------------Initialization functions------------------------------------

// stony_init_default()
// Master init function - sets up MCU pins and configures Stonyman
inline void stony_init_default()
{
	stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3, 
		SMH_GAIN_3V3, SMH_SELAMP_3V3);
}


void stony_init(short vref, short nbias, short aobias, char gain, char selamp)
{
  short config;
  char flagUseAmplifier;

#ifdef IMPLICIT_EYE_TRACKING
  read_cider_params();
#endif

#ifdef USB_SEND
  for (int i = 0; i < USB_PACKET_SIZE; i++)
    frame_data[i] = 0;
#endif // USB_SEND

#ifdef SD_SEND
  for (int i = 0; i < 512; i++)
    frame_data[i] = 0;
#endif // SD_SEND
  
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

// TODO: Test this
// void stony_reset(uint16_t wait_time, char gain, char selamp)
// {
//   short config;
//   char flagUseAmplifier;

//   //turn chip OFF
//   set_pointer_value(REG_CONFIG,0,OUT_CAM);
//   set_pointer_value(REG_CONFIG,0,EYE_CAM);

//   delay_ms(wait_time);

//   // sanitize this input before use
//   flagUseAmplifier=selamp ? 1:0;

//   config = gain + (flagUseAmplifier * 8) + 16;

//   //turn chip ON with config value
//   set_pointer_value(REG_CONFIG,config,OUT_CAM);
//   set_pointer_value(REG_CONFIG,config,EYE_CAM);
// }

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




// -----------------------------------------------------------------------------
// ----------------Standard image capture functions-----------------------------
// -----------------------------------------------------------------------------




// stony_single()
// Capture an image from one camera (selected by PRIMARY_CAM in stonyman.h)
int stony_single()
{
  // TODO: LED code

  // Double-buffered (2-dim array), two bytes per pixel
  uint8_t base_buffers[2][TX_PIXELS * 2];

  volatile uint16_t start, total;
  uint8_t buf_idx = 0;
  uint16_t this_pixel;

#ifdef USB_SEND
  #ifdef IMPLICIT_EYE_TRACKING
  uint8_t num_params = 7;
  #else
  uint8_t num_params = 4;
  #endif // IMPLICIT_EYE_TRACKING
#endif // USB_SEND

#ifdef IMPLICIT_EYE_TRACKING
  uint16_t *subsampled = (uint16_t*)malloc(num_subsample * sizeof(uint16_t));

  StreamStats stream_stats;
  init_streamstats(&stream_stats);
#endif

#ifdef DO_8BIT_CONV
    uint8_t *active_buffer = (uint8_t *)base_buffers[0];

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < USB_PIXELS * 2; j++) {
        base_buffers[i][j] = 0;
      }
    }
#else
    uint16_t *active_buffer = (uint16_t *)base_buffers[0];
#endif
  
  ADC_RegularChannelConfig(ADC1, PRIMARY_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, PRIMARY_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(MAJOR_REG, 0, PRIMARY_CAM);

  int data_cycle = 0;
  for (int i_major = 0; i_major < 112; i_major++) {
    set_pointer_value(MINOR_REG, 0, PRIMARY_CAM);

    delay_us(1);
    
    for (int j_minor = 0; j_minor < 112; j_minor++, data_cycle++) {      
      PRIMARY_PARAM(INPH_BANK)->ODR |= PRIMARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      PRIMARY_PARAM(INPH_BANK)->ODR &= ~PRIMARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      // Do ADC conversion
      ADC_SoftwareStartConv(ADC1);
      
      // TODO: there has *got* to be a way to test for a flag or something to check for ADC completion
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      this_pixel = adc_values[adc_idx] - FPN_PRI(i_major, j_minor);
      active_buffer[data_cycle] = RESIZE_PIXEL(this_pixel);

#ifdef IMPLICIT_EYE_TRACKING
      update_streamstats(&stream_stats, subsampled, this_pixel, i_major, j_minor);
#endif

#ifdef USB_SEND
      if (data_cycle == USB_PIXELS - 1) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(base_buffers[buf_idx], USB_PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;

        active_buffer = CAST_PIXEL_BUFFER(base_buffers[buf_idx]);
      }
#endif // USB_SEND
      
      adc_idx = !adc_idx;

      if (j_minor < 111) {
        PRIMARY_PARAM(INCV_BANK)->ODR |= PRIMARY_PARAM(INCV_PIN);
        PRIMARY_PARAM(INCV_BANK)->ODR &= ~PRIMARY_PARAM(INCV_PIN);
      }
    } // for (j_minor)

    if (i_major < 111)
      inc_pointer_value(MAJOR_REG, 1, PRIMARY_CAM);

#ifdef SD_SEND
    if (data_cycle / 112 == SD_ROWS) {
      if (i_major > SD_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)base_buffers[buf_idx], sd_ptr, SD_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;

      active_buffer = CAST_PIXEL_BUFFER(base_buffers[buf_idx]);
      
      sd_ptr += SD_BLOCKS / 2;
      data_cycle = 0;
    }
#endif // SD_SEND
  } // for (i_major)

#ifdef IMPLICIT_EYE_TRACKING
  ann_predict(subsampled, &stream_stats);
  free(subsampled);

  frame_data[FD_MODEL_OFFSET] = PARAM_ANN;
  frame_data[FD_PREDX_OFFSET] = pred[PRED_X];
  frame_data[FD_PREDY_OFFSET] = pred[PRED_Y];
#else
  frame_data[FD_MODEL_OFFSET] = PARAM_NOMODEL;
#endif // IMPLICIT_EYE_TRACKING
  
  time_elapsed = TIM5->CNT;
  TIM5->CNT = 0;
  *((uint32_t*)(frame_data + FD_TIMER_OFFSET)) = time_elapsed;

#ifdef USB_SEND

  if (data_cycle != 0) {

#ifdef USB_16BIT
    for (int i = data_cycle; i < USB_PIXELS; i++)
      active_buffer[i] = 0;
#endif
    
    while (packet_sending == 1);
    send_packet(base_buffers[buf_idx], USB_PACKET_SIZE);
  }

  usb_finish_tx(frame_data, num_params);
#endif // USB_SEND

#ifdef SD_SEND

#if (112 % SD_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)base_buffers[buf_idx], sd_ptr, SD_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += SD_MOD_BLOCKS / 2;
#endif // (112 % SD_ROWS != 0)

  f_finish_write();
  if (disk_write_fast(0, frame_data, sd_ptr, 1) != RES_OK)      return -1;
  sd_ptr += 1;

  f_finish_write();
#endif // SD_SEND

  return 0;
}



// stony_dual()
// Capture images simultaneously from both cameras
// (note: only one camera can be streamed when in USB mode - selected by
// PRIMARY_CAM in stonyman.h - though both cameras will still be read from)
int stony_dual()
{
  __IO uint32_t led1 = 0, led2 = 0;
  uint32_t DAC_Align = DAC_Align_12b_R;
  
  led1 = led2 = (uint32_t)DAC_BASE;
  led1 += DHR12R1_OFFSET + DAC_Align;
  led2 += DHR12R2_OFFSET + DAC_Align;
  
  // TODO: LED code

  // TODO: is this size any kind of correct?
  // Double-buffered (2-dim array), two bytes per pixel, two cameras
  uint8_t base_buffers[2][TX_PIXELS * 2 * 2];

  uint16_t secondary_offset = BUFFER_HALF;
  uint8_t buf_idx = 0;
  uint16_t this_pixel;

#ifdef USB_SEND
  #ifdef IMPLICIT_EYE_TRACKING
  uint8_t num_params = 3;
  #else
  uint8_t num_params = 1;
  #endif // IMPLICIT_EYE_TRACKING
#endif // USB_SEND

#ifdef IMPLICIT_EYE_TRACKING
  uint16_t *subsampled = (uint16_t*)malloc(num_subsample * sizeof(uint16_t));

  StreamStats stream_stats;
  init_streamstats(&stream_stats);
#endif

#ifdef DO_8BIT_CONV
    uint8_t *active_buffer = (uint8_t *)base_buffers[0];

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < USB_PIXELS * 2; j++) {
        base_buffers[i][j] = 0;
      }
    }
#else
    uint16_t *active_buffer = (uint16_t *)base_buffers[0];
#endif
  
  ADC_RegularChannelConfig(ADC1, PRIMARY_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SECONDARY_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  set_pointer_value(MAJOR_REG, 0, OUT_CAM);
  set_pointer_value(MAJOR_REG, 0, EYE_CAM);

  int data_cycle = 0;

for (int i_major = 0; i_major < 112; i_major++) {    
    set_pointer_value(MINOR_REG, 0, OUT_CAM);
    set_pointer_value(MINOR_REG, 0, EYE_CAM);
    
    delay_us(1);
    
    for (int j_minor = 0; j_minor < 112; j_minor++, data_cycle++) {        
      PRIMARY_PARAM(INPH_BANK)->ODR |= PRIMARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      PRIMARY_PARAM(INPH_BANK)->ODR &= ~PRIMARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");

      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      if (j_minor != 0) {        
        // DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
        // *(__IO uint32_t *) led1 = LED_LOW;
        // *(__IO uint32_t *) led2 = LED_LOW;

        active_buffer[(data_cycle - 1) + secondary_offset] = RESIZE_PIXEL(adc_values[1] - FPN_SEC(i_major, (j_minor - 1)));
      }
      
      // Do conversion for PRIMARY_CAM
      ADC_SoftwareStartConv(ADC1);
      
      if (j_minor != 0) {
        SECONDARY_PARAM(INCV_BANK)->ODR |= SECONDARY_PARAM(INCV_PIN);
        SECONDARY_PARAM(INCV_BANK)->ODR &= ~SECONDARY_PARAM(INCV_PIN);
      }
      
      SECONDARY_PARAM(INPH_BANK)->ODR |= SECONDARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      SECONDARY_PARAM(INPH_BANK)->ODR &= ~SECONDARY_PARAM(INPH_PIN);
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");

      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      this_pixel = adc_values[0] - FPN_PRI(i_major, j_minor);
      active_buffer[data_cycle] = RESIZE_PIXEL(this_pixel);

#ifdef IMPLICIT_EYE_TRACKING
      update_streamstats(&stream_stats, subsampled, this_pixel, i_major, j_minor);
#endif

#ifdef USB_SEND
      if (data_cycle == USB_PIXELS - 1) {
        while (packet_sending == 1);
        
        data_cycle = -1;
        send_packet(base_buffers[buf_idx], USB_PACKET_SIZE);
        packet_sending = 1;
        
        buf_idx = !buf_idx;

        active_buffer = CAST_PIXEL_BUFFER(base_buffers[buf_idx]);
      }
#endif // USB_SEND
      
      // Do conversion for SECONDARY_CAM
      ADC_SoftwareStartConv(ADC1);

      if (j_minor < 111) {
        PRIMARY_PARAM(INCV_BANK)->ODR |= PRIMARY_PARAM(INCV_PIN);
        PRIMARY_PARAM(INCV_BANK)->ODR &= ~PRIMARY_PARAM(INCV_PIN);
      }
    } // for (j_minor)
    
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
    
    // DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);      
    // *(__IO uint32_t *) led1 = LED_LOW;
    // *(__IO uint32_t *) led2 = LED_LOW;
    
    active_buffer[(data_cycle - 1) + secondary_offset] = RESIZE_PIXEL(adc_values[1] - FPN_SEC(i_major, 111));
    
    if (i_major < 111) {
      inc_pointer_value(MAJOR_REG, 1, OUT_CAM);
      inc_pointer_value(MAJOR_REG, 1, EYE_CAM);
    }

#ifdef SD_SEND
    if (data_cycle / 112 == SD_ROWS) {
      if (i_major > SD_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)base_buffers[buf_idx], sd_ptr, SD_BLOCKS) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;

      active_buffer = CAST_PIXEL_BUFFER(base_buffers[buf_idx]);
      
      sd_ptr += SD_BLOCKS;
      data_cycle = 0;

#if (112 % SD_ROWS != 0)
      if (i_major + SD_ROWS > 112) {
        secondary_offset = SD_MOD_OFFSET;

        for (int i = 0; i < TX_PIXELS * 2; i++) {
          active_buffer[i] = 0;
      }
    }
#endif
    }
#endif // SD_SEND
  } // for (i_major)

#ifdef IMPLICIT_EYE_TRACKING
  ann_predict(subsampled, &stream_stats);
  free(subsampled);

  frame_data[FD_MODEL_OFFSET] = PARAM_ANN;
  frame_data[FD_PREDX_OFFSET] = pred[PRED_X];
  frame_data[FD_PREDY_OFFSET] = pred[PRED_Y];
#else
  frame_data[FD_MODEL_OFFSET] = PARAM_NOMODEL;
#endif // IMPLICIT_EYE_TRACKING

  time_elapsed = TIM5->CNT;
  TIM5->CNT = 0;
  *((uint32_t*)(frame_data + FD_TIMER_OFFSET)) = time_elapsed;

#ifdef USB_SEND

  if (data_cycle != 0) {

#ifdef USB_16BIT
    for (int i = data_cycle; i < USB_PIXELS; i++)
      active_buffer[i] = 0;
#endif
    
    while (packet_sending == 1);
    send_packet(base_buffers[buf_idx], USB_PACKET_SIZE);
  }

  usb_finish_tx(frame_data, num_params);
#endif // USB_SEND

#ifdef SD_SEND

#if (112 % SD_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)base_buffers[buf_idx], sd_ptr, SD_MOD_BLOCKS) != RES_OK)      return -1;
  sd_ptr += SD_MOD_BLOCKS;
#endif // (112 % SD_ROWS != 0)

  f_finish_write();
  if (disk_write_fast(0, frame_data, sd_ptr, 1) != RES_OK)      return -1;
  sd_ptr += 1;
  
  f_finish_write();
#endif // SD_SEND
  
  return 0;
}



#if defined(EYE_TRACKING_ON) && defined(SD_SEND)
// stony_ann()
// Efficient ANN-based eye tracking - reads only the pixels needed for the ANN model
// IMPORTANT: As of right now, this function is only included in SD mode, since USB is not implemented for it
// SUPER IMPORTANT: Correct ANN predictions tested and confirmed, have not yet tested SD output code in this function
int stony_ann()
{
  uint8_t last_major, last_minor;
  uint16_t this_pixel;

  uint16_t *subsampled = (uint16_t*)malloc(num_subsample * sizeof(uint16_t));

  StreamStats stream_stats;
  init_streamstats(&stream_stats);
  
  ADC_RegularChannelConfig(ADC1, ECAM_ADC_CHAN, 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, ECAM_ADC_CHAN, 2, ADC_SampleTime_4Cycles);
  
  last_minor = MASK(0, MASK_MINOR);
  last_major = MASK(0, MASK_MAJOR);

  set_pointer_value(MAJOR_REG, last_major, EYE_CAM);
  set_pointer_value(MINOR_REG, last_minor, EYE_CAM);

  asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
  asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");

  for (int pixel = 0; pixel < num_subsample; pixel++) {   
      if (MASK(pixel, MASK_MAJOR) != last_major)
      {
        char diff = MASK(pixel, MASK_MAJOR) - last_major;
        inc_pointer_value(MAJOR_REG, diff, EYE_CAM);
        last_major = MASK(pixel, MASK_MAJOR);

        set_pointer_value(MINOR_REG, MASK(pixel, MASK_MINOR), EYE_CAM);
        last_minor = MASK(pixel, MASK_MINOR);
      } else {
         inc_value(MASK(pixel, MASK_MINOR) - last_minor, EYE_CAM);

         last_minor = MASK(pixel, MASK_MINOR);
      }

      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      ECAM_INPH_BANK->ODR |= ECAM_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      ECAM_INPH_BANK->ODR &= ~ECAM_INPH_PIN;
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      /* Start ADC1 Software Conversion */
      ADC_SoftwareStartConv(ADC1);
      
      if (pixel > 0)
        update_streamstats2(&stream_stats, subsampled, this_pixel);
      
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
      
      this_pixel = adc_values[adc_idx] - FPN_EYE(MASK(pixel, MASK_MAJOR), MASK(pixel, MASK_MINOR));
      adc_idx = !adc_idx;
  }
  
  update_streamstats2(&stream_stats, subsampled, this_pixel);
  
  ann_predict(subsampled, &stream_stats);
  free(subsampled);

  frame_data[FD_MODEL_OFFSET] = PARAM_ANN;
  frame_data[FD_PREDX_OFFSET] = pred[PRED_X];
  frame_data[FD_PREDY_OFFSET] = pred[PRED_Y];
  
  time_elapsed = TIM5->CNT;
  TIM5->CNT = 0;
  *((uint32_t*)(frame_data + FD_TIMER_OFFSET)) = time_elapsed;

 if (disk_write_fast(0, frame_data, sd_ptr, 1) != RES_OK)      return -1;
 sd_ptr += 1;

 f_finish_write();

  return 0;
}
#endif // defined(EYE_TRACKING_ON) && defined(SD_SEND)