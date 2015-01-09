#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"

uint16_t test_audio(void);

static __IO uint32_t TimingDelay;

int main()
{  
  uint16_t start;
  volatile uint16_t total;
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_us_delay();
  
  config_ms_timer();
  
  test_audio();
  
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

uint16_t test_audio(void)
{
  uint16_t ADCdata;
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_BankSelection(ADC1, ADC_Bank_A);
  
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_4Cycles);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Wait until ADC1 ON status */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
  {
  }
  
  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);

  /* Wait until ADC Channel 5 or 1 end of conversion */
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
  {
  }
  
  ADCdata = ADC_GetConversionValue(ADC1);
  
  return ADCdata;
}