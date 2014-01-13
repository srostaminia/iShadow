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

void SleepMode_Measure();

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
  
//  if (disk_initialize(0) != SD_OK)
//    while(1);
  
  config_ms_timer();
  
  SleepMode_Measure();
  
  while(1) {
//    start = TIM4->CNT;
    stony_image_subsample();
//    total = TIM4->CNT - start;
  }
  
//  for (int i = 0; i < 50; i++) {
////      while (1);
//    stony_image_test();
//    if (stony_image_single() != 0)
//      while (1);
////    if (stony_image_dual() != 0)
////      while(1);
//  }
  
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

/**
  * @brief  This function configures the system to enter Sleep mode for
  *         current consumption measurement purpose.
  *         Sleep Mode
  *         ==========  
  *            - System Running at HSI (16MHz)
  *            - Flash 1 wait state  
  *            - Voltage Range 2
  *            - Code running from Internal FLASH
  *            - Current Consumption ~1mA 
  *            - Wakeup using EXTI Line (Key Button PA.00)   
  * @param  None
  * @retval None
  */
void SleepMode_Measure(void)
{
  /* Configure System Clock to HSI (16MHz) */
  __IO uint32_t StartUpCounter = 0, HSIStatus = 0;
    
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSI */
  RCC_HSICmd(ENABLE);

  /* Wait till HSI is ready and if Time out is reached exit */
  do
  {
    HSIStatus = RCC_GetFlagStatus(RCC_FLAG_HSIRDY);
    StartUpCounter++;  
  } while((HSIStatus == 0) && (StartUpCounter != HSI_STARTUP_TIMEOUT));


  if (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) != RESET)
  {
    HSIStatus = (uint32_t)0x01;
  }
  else
  {
    HSIStatus = (uint32_t)0x00;
  } 

  if (HSIStatus == 0x01)
  {
    /* Enable 64-bit access */
    FLASH_ReadAccess64Cmd(ENABLE);
   
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(ENABLE);
  
    /* Flash 1 wait state */
    FLASH_SetLatency(FLASH_Latency_1);    

    /* Enable the PWR APB1 Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  
    /* Select the Voltage Range 2 (1.5V) */
    PWR_VoltageScalingConfig(PWR_VoltageScaling_Range2);
  
    /* Wait Until the Voltage Regulator is ready */
    while(PWR_GetFlagStatus(PWR_FLAG_VOS) != RESET)
    {
    } 

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK */
    RCC_PCLK1Config(RCC_HCLK_Div1);

    /* Select HSI as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

    /* Wait till HSI is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x04)
    {}
  }
  else
  { 
    /* If HSI fails to start-up, the application will have wrong clock configuration.
    User can add here some code to deal with this error */

    /* Go to infinite loop */
    while (1)
    {}
  }

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_Init(GPIOG, &GPIO_InitStructure);  
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_Init(GPIOB, &GPIO_InitStructure);   

  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, DISABLE);

  /*  Configure Key Button*/
//  STM_EVAL_PBInit(BUTTON_KEY,BUTTON_MODE_GPIO);

//  /* Wait Until Key button pressed */
//  while(STM_EVAL_PBGetState(BUTTON_KEY) == RESET)
//  {
//  }
//  /* Wait Until Key button pressed */
//  while(STM_EVAL_PBGetState(BUTTON_KEY) != RESET)
//  {
//  }

  /*  Configure Key Button*/
//  STM_EVAL_PBInit(BUTTON_KEY,BUTTON_MODE_EXTI);

  /* Request to enter SLEEP mode with regulator ON */
  PWR_EnterSleepMode(PWR_Regulator_ON, PWR_SLEEPEntry_WFI);

  /* Initialize LED1 on STM32L152-EVAL board */
//  STM_EVAL_LEDInit(LED1);

  /* Infinite loop */
  while (1)
  {
    /* Toggle The LED1 */
//    STM_EVAL_LEDToggle(LED1);

    /* Inserted Delay */
    for(int index = 0; index < 0x5FFFF; index++);
  }
}