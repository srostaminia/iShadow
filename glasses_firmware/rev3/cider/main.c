
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
  
  uint16_t medfilt_check[112] = {260,260,261,258,261,258,261,258,255,253,248,247,246,239,246,239,244,238,244,242,242,240,238,235,238,233,238,236,238,236,240,236,240,238,241,238,243,237,242,235,237,237,237,240,241,246,250,254,255,255,258,258,262,258,264,262,264,262,261,256,248,248,236,228,222,219,222,216,220,216,220,219,222,219,222,220,222,230,230,240,242,246,246,246,252,248,252,249,252,250,253,250,250,249,249,241,244,238,243,240,243,246,246,248,252,255,257,261,266,268,272,272};
  uint16_t edge_check[106] = {1,2,6,11,24,29,33,34,25,24,12,11,5,0,1,3,6,11,17,18,11,6,1,4,5,5,4,4,5,5,6,4,3,3,8,9,11,0,4,18,26,36,41,37,31,21,19,14,16,13,12,10,3,5,25,36,55,67,79,83,69,55,28,17,7,2,3,8,7,6,3,12,19,39,48,56,52,38,32,18,18,11,9,5,3,4,0,2,7,14,19,26,23,18,8,6,10,19,20,26,29,33,38,40,42,39};
  uint16_t row[112] = {260,264,256,261,258,265,258,261,253,255,247,248,236,246,239,246,238,244,236,245,242,240,235,238,233,242,231,238,236,241,235,240,236,241,238,243,237,244,235,242,231,237,237,241,240,246,250,254,255,255,258,262,258,264,258,264,262,266,256,261,248,248,236,228,219,222,215,224,216,220,215,222,219,223,219,222,220,230,230,242,240,246,246,252,242,252,248,252,249,253,250,254,249,250,241,249,238,244,234,243,240,246,246,252,248,257,255,266,261,268,272,289};
  uint8_t edges[6] = {0, 0, 0, 0, 0, 0};
  
  find_pupil_edge(70, edges, row, medfilt_check, edge_check);
    
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