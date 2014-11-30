
#include <intrinsics.h>
//#include <ST/iostm32L151xx.h>
#include "stm32l1xx.h"
#include "main.h"
#include "libstony.h"

extern int fpn_offset;

static __IO uint32_t TimingDelay;
//extern uint16_t pred_img[112][112];
extern int8_t pred[3];
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
  
//  stony_init(41, 50, 41,
//            4, SMH_SELAMP_3V3);

  stony_init(SMH_VREF_3V3, SMH_NBIAS_3V3, SMH_AOBIAS_3V3,
            SMH_GAIN_3V3, SMH_SELAMP_3V3);
  
  pred[0] = 255;
  pred[1] = 255;
  
  uint16_t medfilt_check[112] = {251,255,255,252,252,252,249,248,244,243,244,243,239,237,236,229,229,239,239,237,237,238,238,234,233,233,240,242,243,243,243,242,242,246,246,244,244,244,242,242,243,244,251,251,254,256,261,261,262,263,263,267,265,271,265,271,264,272,266,266,263,253,242,242,241,226,222,222,221,221,220,223,226,228,232,237,245,251,257,259,262,262,262,265,261,265,261,262,257,258,257,258,255,260,256,259,257,259,257,259,258,259,261,263,261,259,256,255,252,245,245,245};
  uint16_t edge_check[106] = {8,13,18,21,22,19,15,16,19,28,32,22,5,13,19,15,6,5,8,12,7,5,20,28,23,13,2,2,5,8,7,2,4,8,7,3,8,18,29,32,33,32,28,25,17,15,11,17,13,14,5,4,1,3,5,25,44,67,70,73,69,67,60,45,27,6,4,13,24,33,45,56,67,70,64,50,33,22,10,8,1,1,8,14,15,15,10,4,1,2,2,2,2,0,2,1,5,8,11,7,2,13,22,31,34,35};
  uint16_t row[112] = {255,242,244,239,239,236,239,239,242,239,238,241,241,237,241,239,241,240,244,241,248,241,249,247,247,245,248,244,248,242,249,243,246,243,247,240,243,239,246,248,252,248,255,254,260,258,270,264,263,257,273,265,270,264,269,264,275,268,271,267,262,253,249,234,223,216,219,212,214,210,213,210,217,213,218,214,224,224,237,242,252,258,270,289,327,350,353,339,333,345,322,295,268,269,254,260,255,260,251,258,253,261,258,263,260,261,258,265,258,260,255,264};
  uint8_t edges[6] = {0, 0, 0, 0, 0, 0};
  
  find_pupil_edge(72, edges, row, medfilt_check, edge_check);
    
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