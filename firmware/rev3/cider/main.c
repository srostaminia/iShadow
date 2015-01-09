
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "libstony.h"

extern int fpn_offset;

static __IO uint32_t TimingDelay;
//extern uint16_t pred_img[112][112];
extern int8_t pred[2];
extern float pred_radius;
extern uint16_t min, max;

uint8_t packet_sending;

int main()
{ 
//  uint8_t tx_test[PACKET_SIZE];
//  
//  for (int i = 0; i < PACKET_SIZE; i++) {
//    tx_test[i] = i + 1;
//  }
  
  if (SysTick_Config(SystemCoreClock / 1000)) {
    while (1);
  }

  config_ms_timer();
  config_us_delay();
  
  volatile uint16_t start, total;
  
//  stony_init(41, 50, 41,
//            4, SMH_SELAMP_3V3);

  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  pred[0] = 255;
  pred[1] = 255;
  
//  uint16_t row[112] = {252,249,245,254,252,258,256,251,249,256,256,252,248,259,251,250,248,245,240,245,241,248,252,248,249,249,254,260,253,259,254,258,260,257,250,243,239,234,227,210,217,212,209,210,218,219,214,217,220,220,217,231,236,242,248,251,258,258,257,259,258,260,250,252,253,253,252,256,257,260,263,261,262,271,266,270,276,281,284,280,287,286,280,284,277,283,280,274,277,272,271,271,271,264,264,254,266,256,251,264,263,254,261,259,258,260,253,248,251,249,255};
//  uint8_t edges[6] = {0, 0, 0, 0, 0, 0};
  
  start = TIM5->CNT;
//  find_pupil_edge(44, edges, row);
  run_cider(0);
  total = TIM5->CNT - start;
    
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

#pragma inline=never
void delay_ms(int delayTime)
{
  TIM4->CNT = 0;
  while((uint16_t)(TIM4->CNT) <= delayTime);
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

void pulse_led(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Configure PA.04 (DAC_OUT1), PA.05 (DAC_OUT2) as analog */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  DAC_DeInit();
  
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  
  /* DAC Channel2 Init */
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_1, ENABLE);
  
  DAC_SetChannel1Data(DAC_Align_12b_R, 0x0); // 0x64D = 1.26V
  
  DAC_SetChannel1Data(DAC_Align_12b_R, 0x64D); // 0x64D = 1.26V
//  DAC_DualSoftwareTriggerCmd(ENABLE);
  
  while (1) {
    delay_ms(500);
    DAC_SetChannel1Data(DAC_Align_12b_R, 0x0); // 0x64D = 1.26V
    delay_ms(500);
    DAC_SetChannel1Data(DAC_Align_12b_R, 0x64D); // 0x64D = 1.26V
  }
}