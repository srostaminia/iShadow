#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "TestSDCard.h"
//#include "ff.h"
#include "libstony.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "diskio.h"

static __IO uint32_t TimingDelay;
extern uint32_t sd_ptr;

volatile uint16_t time_start, time_total;

int main()
{  
  volatile uint16_t start, total;
  int result;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();
  
  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  if (disk_initialize(0) != SD_OK)
    while(1);
  
  config_ms_timer();
  
//  while(1) {
////    start = TIM4->CNT;
//    if (stony_image_single())
//      while(1);
//    
////    total = TIM4->CNT - start;
//  }
  
//  stony_image_single();
  uint8_t loc1[2] = {55, 55};
//  uint8_t loc2[2] = {100, 100};
  
//  pixel_mass_test(loc1);
  
//  single_pixel_test(loc1, 56, 0);
//  single_pixel_test(loc2, 56, 0);
  cider_line_test(55, 0);
  
//  stony_image_single();
  
  return total;
}

int pixel_mass_test(uint8_t loc[2])
{
//   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[112 * 112 * 2];
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  stony_single_pixel(loc, 112 * 112, 18, sd_buf);
  
//  uint8_t loc2[2] = {56, 0};
//  stony_single_pixel(loc2, 112 * 56, 18, (sd_buf + 112 * 56 * 2));

  for (int i = 0; i < 2; i++) {
    if (disk_write_fast(0, sd_buf + TX_BLOCKS / 2 * 512 * i, sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
    f_finish_write();
    sd_ptr += TX_BLOCKS / 2;  
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf + TX_BLOCKS * 512, sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

int single_pixel_test(uint8_t loc[2], uint16_t lines, uint16_t delay)
{
  //   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[2][TX_ROWS][112 * 2];
  uint8_t buf_idx = 0;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  for (int i = 0, buf_line = 1; i < lines; i++, buf_line++) {
    stony_single_pixel(loc, 112, delay, sd_buf[buf_idx][buf_line - 1]);
    
    if (buf_line == TX_ROWS) {
      buf_line = 0;
      if (i > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      
      sd_ptr += TX_BLOCKS / 2;
//      if (row == 31)    return 0;
    }
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

int cider_line_test(int8_t rowcol_num, int8_t rowcol_sel)
{
  //   112 pixels per row, TX_ROWS rows per data transfer, 2 bytes per row, only 1 camera
//   Double-buffered (2-dim array)
  uint8_t sd_buf[2][TX_ROWS][112 * 2];
  uint8_t buf_idx = 0;
  uint16_t this_rowcol;
  
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 1, ADC_SampleTime_4Cycles);
  ADC_RegularChannelConfig(ADC1, SINGLE_PARAM(ADC_CHAN), 2, ADC_SampleTime_4Cycles);
  
  for (int i = 0, buf_line = 1; i < 112; i++, buf_line++) {
    this_rowcol = (rowcol_num < 0) ? (i) : (rowcol_num);
    stony_cider_line(this_rowcol, sd_buf[buf_idx][buf_line - 1], rowcol_sel);
    
    if (buf_line == TX_ROWS) {
      buf_line = 0;
      if (i > TX_ROWS - 1) {
        f_finish_write();
      }
      
      if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_BLOCKS / 2) != RES_OK)      return -1;
      
      buf_idx = !buf_idx;
      
      sd_ptr += TX_BLOCKS / 2;
//      if (row == 31)    return 0;
    }
  }
  
#if (112 % TX_ROWS != 0)
  f_finish_write();
  
  if (disk_write_fast(0, (uint8_t *)sd_buf[buf_idx], sd_ptr, TX_MOD_BLOCKS / 2) != RES_OK)      return -1;
  sd_ptr += TX_MOD_BLOCKS / 2;
#endif
  
  f_finish_write();
  
  return 0;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

void config_ms_timer()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM4, ENABLE);
}

void config_us_delay()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
  TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_Cmd(TIM5, ENABLE);
}

#pragma inline=never
void delay_us(int delayTime)
{
//  uint16_t start = TIM5->CNT;
//  while((uint16_t)(TIM5->CNT - start) <= delayTime);
  
  for (int i = 0; i < delayTime; i++) {
    asm volatile ("nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n");
  }
}