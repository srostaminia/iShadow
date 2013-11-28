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

int main()
{  
  uint16_t start;
  volatile uint16_t total;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();

  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  if (disk_initialize(0) != SD_OK)
    while(1);
  
  config_ms_timer();
  
  while(1) {
    // TODO: Try taking a full image on the eye-facing, then doing prediction on it here and offline to verify that we're pulling the right pixel data
    if (stony_image_single() != 0)
      while (1);
    if (stony_image_subsample() != 0)
      while (1);
  }
  
  return total;
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