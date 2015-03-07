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
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }
  
  config_ms_timer();
  config_us_delay();
  
  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
             SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  uint16_t model_time[] = {0, 0};
  get_model_time(model_time);
  
  uint16_t model_sum = model_time[0] + model_time[1];
  
  if (model_sum >= 250) {
    while(1);
  }
  
  const uint16_t SLEEP_TIME = 250 - model_sum;
  
  uint8_t cycle = 1;
  
  while (1) {
    if (SysTick_Config(SystemCoreClock / 1000)) {
      while (1);
    }
    
    dac_init();
    
//    config_us_delay();
    
    if (cycle == 1 || cycle == 2) {      
      DAC_SetChannel2Data(DAC_Align_12b_R, LED_HIGH);
      DAC_SetChannel1Data(DAC_Align_12b_R, LED_HIGH);
    } else {
      DAC_SetChannel2Data(DAC_Align_12b_R, LED_LOW);
      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);
    }
    
    if (cycle == 1 || cycle == 3) {
      stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
                 SMH_GAIN_3V3, SMH_SELAMP_3V3);
    }
    
//    config_ms_timer();
    stony_image_subsample();
    
    if (cycle == 1 || cycle == 2) {
      DAC_SetChannel2Data(DAC_Align_12b_R, LED_LOW);
      DAC_SetChannel1Data(DAC_Align_12b_R, LED_LOW);
    }
    
    if (cycle == 1 || cycle == 3)
      stony_sleep();
    
    if (cycle == 4) {
      cycle = 0;
      StopRTCLSIMode_Measure(SLEEP_TIME * 2);
    } else {
      StopRTCLSIMode_Measure(SLEEP_TIME);
    }
    
    SystemInit();
    
    cycle += 1;
  }
  
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

void get_model_time(uint16_t *model_time) {
  volatile uint16_t full_time, acq_time, start;
  
  stony_image_subsample();
  
  start = MS_TIME;
  stony_image_subsample();
  full_time = MS_TIME - start;
  
  start = MS_TIME;
  stony_image_subsample_nopred();
  acq_time = MS_TIME - start;
  
  model_time[0] = acq_time;                     // Acquisiton time
  model_time[1] = full_time - acq_time;         // Prediction time        
}

/**
  * @brief  This function configures the system to enter Stop mode with RTC 
  *         clocked by LSI for current consumption measurement purpose.
  *         STOP Mode with RTC clocked by LSI
  *         =================================   
  *           - RTC Clocked by LSI
  *           - Regulator in LP mode
  *           - HSI and HSE OFF
  *           - No IWDG
  *           - Current Consumption ~1.3uA
  *           - Automatic Wakeup using RTC clocked by LSI (after ~4s)  
  * @param  None
  * @retval None
  */
void StopRTCLSIMode_Measure(uint16_t sleep_time)
{
  // 37 kHz with a clock divider of 16
  uint32_t sleep_regval = (37 * sleep_time) / 16;
  
  NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable PWR APB1 Clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

  /* Reset RTC Domain */
  RCC_RTCResetCmd(ENABLE);
  RCC_RTCResetCmd(DISABLE);
  
  /* Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

  /*!< LSI Enable */
  RCC_LSICmd(ENABLE);

  /*!< Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}

  /*!< RTC Clock Source Selection */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  GPIO_Init(GPIOG, &GPIO_InitStructure); 
  GPIO_Init(GPIOF, &GPIO_InitStructure);  
//  GPIO_Init(GPIOA, &GPIO_InitStructure); 
//  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC |//RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | 
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, DISABLE);

//  /*  Configure Key Button*/
//  STM_EVAL_PBInit(BUTTON_KEY,BUTTON_MODE_GPIO);
//
//  /* Wait Until Key button pressed */
//  while(STM_EVAL_PBGetState(BUTTON_KEY) == RESET)
//  {
//  }
//  /* Wait Until Key button pressed */
//  while(STM_EVAL_PBGetState(BUTTON_KEY) != RESET)
//  {
//  }
    
  /* EXTI configuration *******************************************************/
  EXTI_ClearITPendingBit(EXTI_Line20);
  EXTI_InitStructure.EXTI_Line = EXTI_Line20;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* Enable the RTC Wakeup Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* RTC Wakeup Interrupt Generation: Clock Source: RTCCLK_Div16, Wakeup Time Base: ~4s 
     RTC Clock Source LSI ~37KHz  
  */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
  RTC_SetWakeUpCounter(sleep_regval);

  /* Enable the Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
      
  /* Enable Ultra low power mode */
  PWR_UltraLowPowerCmd(ENABLE);

  /* Enable Wakeup Counter */
  RTC_WakeUpCmd(ENABLE);
    
  /* Enter Stop Mode */
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

  /* Infinite loop */
//  while (1)
//  {
//    /* Toggle The LED1 */
//    STM_EVAL_LEDToggle(LED1);

//    /* Inserted Delay */
//    for(index = 0; index < 0x5FF; index++);
//  }
}